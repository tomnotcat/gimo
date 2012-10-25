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
#include "gimo-pymodule.h"
#include "gimo-error.h"

/* Redefined in Python.h */
#undef  _POSIX_C_SOURCE
#include <pygobject.h>

struct _GimoPymodulePrivate {
    PyObject *module;
    gchar *name;
};

static void gimo_module_interface_init (GimoModuleInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GimoPymodule, gimo_pymodule, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_MODULE,
                                                gimo_module_interface_init))

static gboolean _gimo_pymodule_open (GimoModule *module,
                                     const gchar *file_name)
{
    GimoPymodule *self = GIMO_PYMODULE (module);
    GimoPymodulePrivate *priv = self->priv;
    gchar *path;
    gchar *suffix;
    PyObject *name = NULL;

    if (priv->module)
        gimo_set_error_return_val (GIMO_ERROR_CONFLICT, FALSE);

    path = g_strdup (file_name);
    suffix = strrchr (path, '.');
    if (suffix)
        *suffix = '\0';

    name = PyString_FromString (path);
    if (NULL == name)
        goto fail;

    priv->module = PyImport_Import (name);
    if (NULL == priv->module) {
        gimo_set_error_full (GIMO_ERROR_IMPORT,
                             "Import python module error: %s",
                             path);
        goto fail;
    }

fail:
    if (name)
        Py_DECREF (name);

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
                                        GObject *param)
{
    GimoPymodule *self = GIMO_PYMODULE (module);
    GimoPymodulePrivate *priv = self->priv;
    PyObject *func = NULL;
    PyObject *value = NULL;
    GObject *object = NULL;

    if (!priv->module)
        return NULL;

    func = PyObject_GetAttrString (priv->module, symbol);
    if (NULL == func) {
        gimo_set_error_full (GIMO_ERROR_NO_SYMBOL,
                             "Can't get python attribute: %s: %s",
                             priv->name, symbol);
        goto fail;
    }

    if (!PyCallable_Check (func)) {
        gimo_set_error_full (GIMO_ERROR_INVALID_SYMBOL,
                             "Attribute not callable: %s: %s",
                             priv->name, symbol);
        goto fail;
    }

    value = PyObject_CallObject (func, NULL);
    if (NULL == value) {
        gimo_set_error_full (GIMO_ERROR_NEW_OBJECT,
                             "Function return NULL: %s: %s",
                             priv->name, symbol);
        goto fail;
    }

    object = pygobject_get (value);
    if (object)
        g_object_ref (object);

fail:
    if (PyErr_Occurred ())
        PyErr_Print ();

    if (func)
        Py_DECREF (func);

    if (value)
        Py_DECREF (value);

    return object;
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

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_PYMODULE,
                                              GimoPymodulePrivate);
    priv = self->priv;

    priv->module = NULL;
    priv->name = NULL;
}

static void gimo_pymodule_finalize (GObject *gobject)
{
    _gimo_pymodule_close (GIMO_MODULE (gobject));

    G_OBJECT_CLASS (gimo_pymodule_parent_class)->finalize (gobject);
}

static void gimo_pymodule_class_init (GimoPymoduleClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_pymodule_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoPymodulePrivate));
}

GimoPymodule* gimo_pymodule_new (void)
{
    return g_object_new (GIMO_TYPE_PYMODULE, NULL);
}
