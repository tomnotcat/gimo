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
#include "gimo-utils.h"

#include <gi/object.h>
#include <gi/value.h>
#include <gjs/gjs.h>
#include <gjs/gjs-module.h>

enum {
    PROP_0,
    PROP_CONTEXT
};

struct _GimoJsmodulePrivate {
    GimoJsmoduleContext *context;
    GjsContext *gjsctx;
    gchar *name;
};

struct _GimoJsmoduleContext {
    GjsContext *gjsctx;
    gint ref_count;
    gint gjs_ref_count;
};

static void gimo_loadable_interface_init (GimoLoadableInterface *iface);
static void gimo_module_interface_init (GimoModuleInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GimoJsmodule, gimo_jsmodule, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_LOADABLE,
                                                gimo_loadable_interface_init);
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_MODULE,
                                                gimo_module_interface_init))

static GimoJsmoduleContext* _gimo_jsmodule_context_new (void)
{
    GimoJsmoduleContext *self;

    self = g_malloc (sizeof *self);
    self->gjsctx = NULL;
    self->ref_count = 1;
    self->gjs_ref_count = 0;

    return self;
}

static void _gimo_jsmodule_context_ref (GimoJsmoduleContext *self)
{
    g_atomic_int_add (&self->ref_count, 1);
}

static void _gimo_jsmodule_context_unref (gpointer p)
{
    GimoJsmoduleContext *self = p;

    if (g_atomic_int_add (&self->ref_count, -1) == 1) {
        g_assert (0 == self->gjs_ref_count);
        g_free (self);
    }
}

static GjsContext* _gimo_jsmodule_context_acquire (GimoJsmoduleContext *self)
{
    if (g_atomic_int_add (&self->gjs_ref_count, 1) == 0)
        self->gjsctx = gjs_context_new ();
    else
        g_object_ref (self->gjsctx);

    return self->gjsctx;
}

static void _gimo_jsmodule_context_release (GimoJsmoduleContext *self)
{
    g_object_unref (self->gjsctx);
    g_atomic_int_add (&self->gjs_ref_count, -1);
}

static gboolean _gimo_jsmodule_open (GimoModule *module,
                                     const gchar *file_name)
{
    GimoJsmodule *self = GIMO_JSMODULE (module);
    GimoJsmodulePrivate *priv = self->priv;
    GError *error = NULL;
    int status = 0;

    if (priv->gjsctx)
        gimo_set_error_return_val (GIMO_ERROR_CONFLICT, FALSE);

    priv->gjsctx = _gimo_jsmodule_context_acquire (priv->context);
    if (!gjs_context_eval_file (priv->gjsctx,
                                file_name,
                                &status,
                                &error))
    {
        gimo_set_error_full (GIMO_ERROR_LOAD,
                             "Eval JS failed: %s: %s",
                             file_name, error->message);
        g_error_free (error);
        _gimo_jsmodule_context_release (priv->context);
        priv->gjsctx = NULL;
    }

    if (priv->gjsctx)
        priv->name = g_strdup (file_name);

    return (priv->gjsctx != NULL);
}

static gboolean _gimo_jsmodule_close (GimoModule *module)
{
    GimoJsmodule *self = GIMO_JSMODULE (module);
    GimoJsmodulePrivate *priv = self->priv;

    if (priv->gjsctx) {
        _gimo_jsmodule_context_release (priv->context);
        priv->gjsctx = NULL;
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

    if (!priv->gjsctx)
        return NULL;

    js_ctx = gjs_context_get_native_context (priv->gjsctx);
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

    priv->name = NULL;
}

static void gimo_jsmodule_finalize (GObject *gobject)
{
    GimoJsmodule *self = GIMO_JSMODULE (gobject);
    GimoJsmodulePrivate *priv = self->priv;

    _gimo_jsmodule_close (GIMO_MODULE (gobject));
    _gimo_jsmodule_context_unref (priv->context);

    G_OBJECT_CLASS (gimo_jsmodule_parent_class)->finalize (gobject);
}

static void gimo_jsmodule_set_property (GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
    GimoJsmodule *self = GIMO_JSMODULE (object);
    GimoJsmodulePrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_CONTEXT:
        priv->context = g_value_get_pointer (value);

        if (priv->context)
            _gimo_jsmodule_context_ref (priv->context);
        else
            priv->context = _gimo_jsmodule_context_new ();
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_jsmodule_class_init (GimoJsmoduleClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_jsmodule_finalize;
    gobject_class->set_property = gimo_jsmodule_set_property;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoJsmodulePrivate));

    g_object_class_install_property (
        gobject_class, PROP_CONTEXT,
        g_param_spec_pointer ("context",
                              "GJS context",
                              "The GJS context",
                              G_PARAM_WRITABLE |
                              G_PARAM_CONSTRUCT_ONLY |
                              G_PARAM_STATIC_STRINGS));
}

GimoJsmodule* gimo_jsmodule_new (GimoJsmoduleContext *context)
{
    return g_object_new (GIMO_TYPE_JSMODULE,
                         "context", context, NULL);
}

static gboolean _gimo_jsmodule_plugin_start (GimoPlugin *self)
{
    static GQuark gjsctx_quark;
    GimoContext *context = NULL;
    GimoLoader *loader = NULL;
    GimoFactory *factory = NULL;
    GimoJsmoduleContext *gjsctx = NULL;
    gboolean result = FALSE;

    if (!gjsctx_quark)
        gjsctx_quark = g_quark_from_static_string ("gimo_jsmodule_context");

    do {
        context = gimo_plugin_query_context (self);
        if (NULL == context)
            break;

        loader = gimo_safe_cast (
            gimo_context_resolve_extpoint (
                context, "org.gimo.core.loader.module"),
            GIMO_TYPE_LOADER);

        if (NULL == loader)
            break;

        gjsctx = g_object_get_qdata (G_OBJECT (context), gjsctx_quark);
        if (!gjsctx) {
            gjsctx = _gimo_jsmodule_context_new ();

            g_object_set_qdata_full (G_OBJECT (context),
                                     gjsctx_quark,
                                     gjsctx,
                                     _gimo_jsmodule_context_unref);
        }

        factory = gimo_factory_new ((GimoFactoryFunc) gimo_jsmodule_new, gjsctx);
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
