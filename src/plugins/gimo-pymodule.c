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
#include "gimo-utils.h"

/* Redefined in Python.h */
#undef  _POSIX_C_SOURCE
#include <pygobject.h>

enum {
    PROP_0,
    PROP_THREADSTATE
};

struct _GimoPymodulePrivate {
    GimoPymoduleState *state;
    PyObject *module;
    gchar *name;
};

struct _GimoPymoduleState {
    PyThreadState *thread_state;
    gint ref_count;
};

static void gimo_loadable_interface_init (GimoLoadableInterface *iface);
static void gimo_module_interface_init (GimoModuleInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GimoPymodule, gimo_pymodule, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_LOADABLE,
                                                gimo_loadable_interface_init);
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_MODULE,
                                                gimo_module_interface_init))

static int thread_state_count;
static PyThreadState *main_thread_state;

static GimoPymoduleState* _gimo_pymodule_state_new (void)
{
    GimoPymoduleState *state;
    PyInterpreterState *main_state;

    state = g_malloc (sizeof *state);

    if (g_atomic_int_add (&thread_state_count, 1) == 0) {
        if (!main_thread_state) {
            Py_InitializeEx (0);
            PyEval_InitThreads ();
            main_thread_state = PyThreadState_Get ();
            PyEval_ReleaseLock ();
        }
    }

    PyEval_AcquireLock ();

    main_state = main_thread_state->interp;
    state->thread_state = PyThreadState_New (main_state);

    PyEval_ReleaseLock ();

    state->ref_count = 1;
    return state;
}

static void _gimo_pymodule_state_ref (GimoPymoduleState *self)
{
    g_atomic_int_add (&self->ref_count, 1);
}

static void _gimo_pymodule_state_unref (gpointer p)
{
    GimoPymoduleState *self = p;

    if (g_atomic_int_add (&self->ref_count, -1) == 1) {
        PyThreadState *old_state;

        /* grab the lock */
        PyEval_AcquireLock ();

        /* swap my thread state out of the interpreter */
        old_state = PyThreadState_Swap (NULL);

        if (old_state != self->thread_state)
            PyThreadState_Swap (old_state);

        /* clear out any cruft from thread state object */
        PyThreadState_Clear (self->thread_state);

        /* delete my thread state object */
        PyThreadState_Delete (self->thread_state);

        /* release the lock */
        PyEval_ReleaseLock ();

        g_free (self);

        if (g_atomic_int_add (&thread_state_count, -1) == 1) {
            if (0) {
                /* FIXME: Will crash when call multiple times. */
                PyEval_AcquireLock ();
                main_thread_state = NULL;
                Py_Finalize ();
            }

            PyEval_AcquireLock ();
            PyThreadState_Clear (main_thread_state);
            PyEval_ReleaseLock ();
        }
    }
}

static gboolean _gimo_pymodule_open (GimoModule *module,
                                     const gchar *file_name)
{
    GimoPymodule *self = GIMO_PYMODULE (module);
    GimoPymodulePrivate *priv = self->priv;
    PyThreadState *old_state;
    gchar *path;
    gchar *name;
    gchar *split;
    gchar *suffix;

    if (priv->module)
        gimo_set_error_return_val (GIMO_ERROR_CONFLICT, FALSE);

    path = g_strdup (file_name);
    name = path;

    suffix = strrchr (path, '.');
    if (suffix)
        *suffix = '\0';

    split = strrchr (path, '/');
#ifdef G_OS_WIN32
    if (NULL== split)
        split = strrchr (path, '\\');
#endif

    PyEval_AcquireLock ();

    old_state = PyThreadState_Swap (priv->state->thread_state);

    if (split) {
        gchar *code;

        *split = '\0';
        name = split + 1;

#ifdef G_OS_WIN32
        code = path;
        while (*code) {
            /* Python can only use '/' as path separator? */
            if (*code == '\\')
                *code = '/';
            ++code;
        }
#endif
        code = g_strdup_printf ("import sys; sys.path.append (\"%s\")", path);
        PyRun_SimpleString (code);
        g_free (code);
    }
    else {
        PyRun_SimpleString ("import sys; sys.path.append (\".\")");
    }

    priv->module = PyImport_ImportModule (name);
    if (NULL == priv->module) {
        gimo_set_error_full (GIMO_ERROR_LOAD,
                             "Import python module error: %s",
                             name);
        goto fail;
    }

fail:
    PyRun_SimpleString ("sys.path.pop ()");

    if (PyErr_Occurred ())
        PyErr_Print ();

    PyThreadState_Swap (old_state);
    PyEval_ReleaseLock ();

    if (priv->module)
        priv->name = g_strdup (file_name);

    g_free (path);

    return (priv->module != NULL);
}

static gboolean _gimo_pymodule_close (GimoModule *module)
{
    GimoPymodule *self = GIMO_PYMODULE (module);
    GimoPymodulePrivate *priv = self->priv;

    PyEval_AcquireLock ();

    if (priv->module) {
        Py_DECREF (priv->module);
        priv->module = NULL;
    }

    PyEval_ReleaseLock();

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
    static PyObject *gobject_module = NULL;
    GimoPymodule *self = GIMO_PYMODULE (module);
    GimoPymodulePrivate *priv = self->priv;
    PyObject *func = NULL;
    PyObject *args = NULL;
    PyObject *arg0 = NULL;
    PyObject *value = NULL;
    GObject *object = NULL;

    if (!priv->module)
        return NULL;

    PyEval_AcquireLock ();

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

    gobject_module = pygobject_init (3, 0, 0);
    args = PyTuple_New (1);
    arg0 = pygobject_new (param);
    PyTuple_SetItem (args, 0, arg0);
    value = PyObject_CallObject (func, args);
    Py_XDECREF (arg0);
    Py_XDECREF (args);

    if (NULL == value || value == Py_None) {
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

    if (value && value != Py_None)
        Py_DECREF (value);

    Py_XDECREF (gobject_module);

    if (PyErr_Occurred ())
        PyErr_Print ();

    PyEval_ReleaseLock ();

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

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_PYMODULE,
                                              GimoPymodulePrivate);
    priv = self->priv;

    priv->module = NULL;
    priv->name = NULL;
}

static void gimo_pymodule_finalize (GObject *gobject)
{
    GimoPymodule *self = GIMO_PYMODULE (gobject);
    GimoPymodulePrivate *priv = self->priv;

    _gimo_pymodule_close (GIMO_MODULE (gobject));
    _gimo_pymodule_state_unref (priv->state);

    G_OBJECT_CLASS (gimo_pymodule_parent_class)->finalize (gobject);
}

static void gimo_pymodule_set_property (GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
    GimoPymodule *self = GIMO_PYMODULE (object);
    GimoPymodulePrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_THREADSTATE:
        priv->state = g_value_get_pointer (value);

        if (priv->state)
            _gimo_pymodule_state_ref (priv->state);
        else
            priv->state = _gimo_pymodule_state_new ();
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_pymodule_class_init (GimoPymoduleClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_pymodule_finalize;
    gobject_class->set_property = gimo_pymodule_set_property;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoPymodulePrivate));

    g_object_class_install_property (
        gobject_class, PROP_THREADSTATE,
        g_param_spec_pointer ("thread-state",
                              "Thread state",
                              "The python thread state",
                              G_PARAM_WRITABLE |
                              G_PARAM_CONSTRUCT_ONLY |
                              G_PARAM_STATIC_STRINGS));
}

GimoPymodule* gimo_pymodule_new (GimoPymoduleState *state)
{
    return g_object_new (GIMO_TYPE_PYMODULE,
                         "thread-state", state, NULL);
}

static gboolean _gimo_pymodule_plugin_start (GimoPlugin *self)
{
    static GQuark state_quark;
    GimoContext *context = NULL;
    GimoLoader *loader = NULL;
    GimoFactory *factory = NULL;
    GimoPymoduleState *state = NULL;
    gboolean result = FALSE;

    if (!state_quark)
        state_quark = g_quark_from_static_string ("gimo_pymodule_thread_state");

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

        state = g_object_get_qdata (G_OBJECT (context), state_quark);
        if (!state) {
            state = _gimo_pymodule_state_new ();

            g_object_set_qdata_full (G_OBJECT (context),
                                     state_quark,
                                     state,
                                     _gimo_pymodule_state_unref);
        }

        factory = gimo_factory_new ((GimoFactoryFunc) gimo_pymodule_new, state);
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

GObject* gimo_pymodule_plugin (GimoPlugin *plugin)
{
    g_signal_connect (plugin,
                      "start",
                      G_CALLBACK (_gimo_pymodule_plugin_start),
                      NULL);

    return g_object_ref (plugin);
}
