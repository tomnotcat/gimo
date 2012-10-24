/* GIMO - A plugin system based on GObject.
 *
 * Copyright © 2012 SoftFlag, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "gimo-jsmodule.h"

#include <gi/object.h>
#include <gjs/gjs.h>
#include <gjs/gjs-module.h>

struct _GimoJsmodulePrivate {
    GjsContext *context;
    gchar *name;
};

static void gimo_module_interface_init (GimoModuleInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GimoJsmodule, gimo_jsmodule, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_MODULE,
                                                gimo_module_interface_init))

static gboolean _gimo_jsmodule_open (GimoModule *module,
                                     const gchar *file_name)
{
    GimoJsmodule *self = GIMO_JSMODULE (module);
    GimoJsmodulePrivate *priv = self->priv;
    GError *error = NULL;
    int status = 0;

    if (priv->context) {
        g_warning ("Jsmodule: module already opened: %s", file_name);
        return FALSE;
    }

    priv->context = gjs_context_new ();
    if (!gjs_context_eval_file (priv->context,
                                file_name,
                                &status,
                                &error))
    {
        g_warning ("Eval JS failed: %s: %s", file_name, error->message);

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
                                        GObject *param)
{
    GimoJsmodule *self = GIMO_JSMODULE (module);
    GimoJsmodulePrivate *priv = self->priv;
    JSContext *js_ctx = NULL;
    jsval function;
    jsval rval;
    JSObject *object = NULL;
    JSObject *global = NULL;
    GObject *result = NULL;

    if (!priv->context)
        return NULL;

    js_ctx = gjs_context_get_native_context (priv->context);
    global = JS_GetGlobalObject (js_ctx);

    JS_GetProperty (js_ctx, global, symbol, &function);
    if (JSVAL_VOID == function) {
        g_warning ("Can't get JS symbol: %s: %s",
                   priv->name, symbol);
        return NULL;
    }

    if (!gjs_call_function_value (
            js_ctx, global, function, 0, NULL, &rval))
    {
        g_warning ("Call JS function failed: %s: %s",
                   priv->name, symbol);
        return NULL;
    }

    if (JSVAL_VOID == rval) {
        g_warning ("JS function return NULL: %s: %s",
                   priv->name, symbol);
        return NULL;
    }

    object = JSVAL_TO_OBJECT (rval);
    if (NULL == object) {
        g_warning ("JS function return non-Object: %s: %s",
                   priv->name, symbol);
        return NULL;
    }

    result = gjs_g_object_from_object (js_ctx, object);
    if (result)
        g_object_ref (result);

    return result;
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