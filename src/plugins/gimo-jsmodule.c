/* GIMO - A plugin framework based on GObject.
 *
 * Copyright (C) 2012 TinySoft, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "gimo-jsmodule.h"
#include "gimo-context.h"
#include "gimo-error.h"
#include "gimo-factory.h"
#include "gimo-loader.h"
#include "gimo-plugin.h"

#include <gi/object.h>
#include <gi/value.h>
#include <gjs/gjs.h>
#include <gjs/gjs-module.h>

struct _GimoJsmodulePrivate {
    GjsContext *context;
    gchar *name;
};

static void gimo_loadable_interface_init (GimoLoadableInterface *iface);
static void gimo_module_interface_init (GimoModuleInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GimoJsmodule, gimo_jsmodule, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_LOADABLE,
                                                gimo_loadable_interface_init);
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_MODULE,
                                                gimo_module_interface_init))

static gboolean _gimo_jsmodule_open (GimoModule *module,
                                     const gchar *file_name)
{
    GimoJsmodule *self = GIMO_JSMODULE (module);
    GimoJsmodulePrivate *priv = self->priv;
    GError *error = NULL;
    int status = 0;

    if (priv->context)
        gimo_set_error_return_val (GIMO_ERROR_CONFLICT, FALSE);

    priv->context = gjs_context_new ();
    if (!gjs_context_eval_file (priv->context,
                                file_name,
                                &status,
                                &error))
    {
        gimo_set_error_full (GIMO_ERROR_LOAD,
                             "Eval JS failed: %s: %s",
                             file_name, error->message);
        g_error_free (error);
        g_object_unref (priv->context);
        priv->context = NULL;
    }

    if (priv->context)
        priv->name = g_strdup (file_name);

    return (priv->context != NULL);
}

static gboolean _gimo_jsmodule_close (GimoModule *module)
{
    GimoJsmodule *self = GIMO_JSMODULE (module);
    GimoJsmodulePrivate *priv = self->priv;

    if (priv->context) {
        g_object_unref (priv->context);
        priv->context = NULL;
    }

    if (priv->name) {
        g_free (priv->name);
        priv->name = NULL;
    }

    return TRUE;
}

static const gchar* _gimo_jsmodule_get_name (GimoModule *module)
{
    GimoJsmodule *self = GIMO_JSMODULE (module);

    return self->priv->name;
}

static GObject* _gimo_jsmodule_resolve (GimoModule *module,
                                        const gchar *symbol,
                                        GObject *param,
                                        gboolean has_return)
{
    GimoJsmodule *self = GIMO_JSMODULE (module);
    GimoJsmodulePrivate *priv = self->priv;
    JSContext *js_ctx = NULL;
    jsval function;
    jsval arg;
    jsval rval;
    GValue garg = G_VALUE_INIT;
    JSObject *object = NULL;
    JSObject *global = NULL;
    GObject *result = NULL;

    if (!priv->context)
        return NULL;

    js_ctx = gjs_context_get_native_context (priv->context);
    global = JS_GetGlobalObject (js_ctx);

    JS_GetProperty (js_ctx, global, symbol, &function);
    if (JSVAL_VOID == function) {
        gimo_set_error_full (GIMO_ERROR_NO_SYMBOL,
                             "Can't get JS symbol: %s: %s",
                             priv->name, symbol);
        return NULL;
    }

    g_value_init (&garg, G_TYPE_OBJECT);
    g_value_set_object (&garg, param);
    gjs_value_from_g_value (js_ctx, &arg, &garg);

    JS_BeginRequest (js_ctx);

    if (JS_IsExceptionPending(js_ctx)) {
        g_warning ("Exception was pending before invoking callback??? "
                   "Not expected");

        gjs_log_exception(js_ctx, NULL);
    }

    if (!gjs_call_function_value (
            js_ctx, global, function, 1, &arg, &rval))
    {
        if (!gjs_log_exception (js_ctx, NULL))
            g_warning ("Function invocation failed but no exception was set?");

        JS_EndRequest (js_ctx);
        g_value_unset (&garg);

        gimo_set_error_full (GIMO_ERROR_INVALID_SYMBOL,
                             "Call JS function failed: %s: %s",
                             priv->name, symbol);
        return NULL;
    }

    if (gjs_log_exception (js_ctx, NULL))
        g_warning ("Function invocation succeeded but an exception was set");

    JS_EndRequest (js_ctx);
    g_value_unset (&garg);

    if (!has_return)
        return NULL;

    if (JSVAL_VOID == rval) {
        gimo_set_error_full (GIMO_ERROR_INVALID_RETURN,
                             "JS function return NULL: %s: %s",
                             priv->name, symbol);
        return NULL;
    }

    object = JSVAL_TO_OBJECT (rval);
    if (NULL == object) {
        gimo_set_error_full (GIMO_ERROR_INVALID_RETURN,
                             "JS function return non-Object: %s: %s",
                             priv->name, symbol);
        return NULL;
    }

    result = gjs_g_object_from_object (js_ctx, object);
    if (result)
        g_object_ref (result);

    return result;
}

static void gimo_loadable_interface_init (GimoLoadableInterface *iface)
{
    iface->load = (GimoLoadableLoadFunc) _gimo_jsmodule_open;
    iface->unload = (GimoLoadableUnloadFunc) _gimo_jsmodule_close;
}

static void gimo_module_interface_init (GimoModuleInterface *iface)
{
    iface->open = _gimo_jsmodule_open;
    iface->close = _gimo_jsmodule_close;
    iface->get_name = _gimo_jsmodule_get_name;
    iface->resolve = _gimo_jsmodule_resolve;
}

static void gimo_jsmodule_init (GimoJsmodule *self)
{
    GimoJsmodulePrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_JSMODULE,
                                              GimoJsmodulePrivate);
    priv = self->priv;

    priv->context = NULL;
    priv->name = NULL;
}

static void gimo_jsmodule_finalize (GObject *gobject)
{
    _gimo_jsmodule_close (GIMO_MODULE (gobject));

    G_OBJECT_CLASS (gimo_jsmodule_parent_class)->finalize (gobject);
}

static void gimo_jsmodule_class_init (GimoJsmoduleClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_jsmodule_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoJsmodulePrivate));
}

GimoJsmodule* gimo_jsmodule_new (void)
{
    return g_object_new (GIMO_TYPE_JSMODULE, NULL);
}

static gboolean _gimo_jsmodule_plugin_start (GimoPlugin *self)
{
    GimoContext *context = NULL;
    GimoLoader *loader = NULL;
    GimoFactory *factory = NULL;
    gboolean result = FALSE;

    do {
        context = gimo_plugin_query_context (self);
        if (NULL == context)
            break;

        loader = gimo_context_resolve_extpoint (context,
                                                "org.gimo.core.loader.module",
                                                GIMO_TYPE_LOADER);
        if (NULL == loader)
            break;

        factory = gimo_factory_new ((GimoFactoryFunc) gimo_jsmodule_new,
                                    NULL);
        result = gimo_loader_register (loader, "js", factory);
    } while (0);

    if (factory)
        g_object_unref (factory);

    if (loader)
        g_object_unref (loader);

    if (context)
        g_object_unref (context);

    return result;
}

void gimo_jsmodule_plugin (GimoPlugin *plugin)
{
    g_signal_connect (plugin,
                      "start",
                      G_CALLBACK (_gimo_jsmodule_plugin_start),
                      NULL);
}
