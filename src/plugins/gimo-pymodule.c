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
#include "gimo-pymodule.h"
#include "gimo-context.h"
#include "gimo-error.h"
#include "gimo-factory.h"
#include "gimo-loader.h"
#include "gimo-plugin.h"

/* Redefined in Python.h */
#undef  _POSIX_C_SOURCE
#include <pygobject.h>

struct _GimoPymodulePrivate {
    PyObject *module;
    gchar *name;
};

static void gimo_loadable_interface_init (GimoLoadableInterface *iface);
static void gimo_module_interface_init (GimoModuleInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GimoPymodule, gimo_pymodule, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_LOADABLE,
                                                gimo_loadable_interface_init);
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_MODULE,
                                                gimo_module_interface_init))

static gboolean _gimo_pymodule_open (GimoModule *module,
                                     const gchar *file_name)
{
    GimoPymodule *self = GIMO_PYMODULE (module);
    GimoPymodulePrivate *priv = self->priv;
    gchar *path;
    gchar *suffix;

    if (priv->module)
        gimo_set_error_return_val (GIMO_ERROR_CONFLICT, FALSE);

    path = g_strdup (file_name);
    suffix = strrchr (path, '.');
    if (suffix)
        *suffix = '\0';

    priv->module = PyImport_ImportModule (path);
    if (NULL == priv->module) {
        gimo_set_error_full (GIMO_ERROR_LOAD,
                             "Import python module error: %s",
                             path);
        goto fail;
    }

fail:
    if (priv->module)
        priv->name = path;
    else
        g_free (path);

    return (priv->module != NULL);
}

static gboolean _gimo_pymodule_close (GimoModule *module)
{
    GimoPymodule *self = GIMO_PYMODULE (module);
    GimoPymodulePrivate *priv = self->priv;

    if (priv->module) {
        Py_DECREF (priv->module);
        priv->module = NULL;
    }

    if (priv->name) {
        g_free (priv->name);
        priv->name = NULL;
    }

    return TRUE;
}

static const gchar* _gimo_pymodule_get_name (GimoModule *module)
{
    GimoPymodule *self = GIMO_PYMODULE (module);

    return self->priv->name;
}

static GObject* _gimo_pymodule_resolve (GimoModule *module,
                                        const gchar *symbol,
                                        GObject *param,
                                        gboolean has_return)
{
    GimoPymodule *self = GIMO_PYMODULE (module);
    GimoPymodulePrivate *priv = self->priv;
    PyObject *gobject_module = NULL;
    PyObject *func = NULL;
    PyObject *arg0 = NULL;
    PyObject *value = NULL;
    GObject *object = NULL;

    if (!priv->module)
        return NULL;

    func = PyObject_GetAttrString (priv->module, symbol);
    if (NULL == func) {
        gimo_set_error_full (GIMO_ERROR_NO_SYMBOL,
                             "Can't get python attribute: %s: %s",
                             priv->name, symbol);
        goto done;
    }

    if (!PyCallable_Check (func)) {
        gimo_set_error_full (GIMO_ERROR_INVALID_SYMBOL,
                             "Attribute not callable: %s: %s",
                             priv->name, symbol);
        goto done;
    }

    gobject_module = pygobject_init (0, 0, 0);
    arg0 = pygobject_new (param);
    value = PyObject_CallFunctionObjArgs (func, arg0, NULL);
    Py_DECREF (arg0);

    if (!has_return)
        goto done;

    if (NULL == value) {
        gimo_set_error_full (GIMO_ERROR_INVALID_RETURN,
                             "Function return NULL: %s: %s",
                             priv->name, symbol);
        goto done;
    }

    object = pygobject_get (value);
    if (object)
        g_object_ref (object);

done:
    Py_XDECREF (func);
    Py_XDECREF (value);
    Py_XDECREF (gobject_module);

    if (PyErr_Occurred ())
        PyErr_Print ();

    return object;
}

static void gimo_loadable_interface_init (GimoLoadableInterface *iface)
{
    iface->load = (GimoLoadableLoadFunc) _gimo_pymodule_open;
    iface->unload = (GimoLoadableUnloadFunc) _gimo_pymodule_close;
}

static void gimo_module_interface_init (GimoModuleInterface *iface)
{
    iface->open = _gimo_pymodule_open;
    iface->close = _gimo_pymodule_close;
    iface->get_name = _gimo_pymodule_get_name;
    iface->resolve = _gimo_pymodule_resolve;
}

static void gimo_pymodule_init (GimoPymodule *self)
{
    GimoPymodulePrivate *priv;
    GimoPymoduleClass *klass;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_PYMODULE,
                                              GimoPymodulePrivate);
    priv = self->priv;

    priv->module = NULL;
    priv->name = NULL;

    klass = GIMO_PYMODULE_GET_CLASS (self);

    if (g_atomic_int_add (&klass->instance_count, 1) == 0) {
        g_setenv ("PYTHONPATH", ".", 0);
        Py_Initialize ();
    }
}

static void gimo_pymodule_finalize (GObject *gobject)
{
    GimoPymoduleClass *klass;

    _gimo_pymodule_close (GIMO_MODULE (gobject));

    klass = GIMO_PYMODULE_GET_CLASS (gobject);

    if (g_atomic_int_add (&klass->instance_count, -1) == 1) {
        Py_Finalize ();
    }

    G_OBJECT_CLASS (gimo_pymodule_parent_class)->finalize (gobject);
}

static void gimo_pymodule_class_init (GimoPymoduleClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_pymodule_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoPymodulePrivate));
    klass->instance_count = 0;
}

GimoPymodule* gimo_pymodule_new (void)
{
    return g_object_new (GIMO_TYPE_PYMODULE, NULL);
}

static gboolean _gimo_pymodule_plugin_start (GimoPlugin *self)
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

        factory = gimo_factory_new ((GimoFactoryFunc) gimo_pymodule_new,
                                    NULL);
        result = gimo_loader_register (loader, "py", factory);
    } while (0);

    if (factory)
        g_object_unref (factory);

    if (loader)
        g_object_unref (loader);

    if (context)
        g_object_unref (context);

    return result;
}

void gimo_pymodule_plugin (GimoPlugin *plugin)
{
    g_signal_connect (plugin,
                      "start",
                      G_CALLBACK (_gimo_pymodule_plugin_start),
                      NULL);
}
