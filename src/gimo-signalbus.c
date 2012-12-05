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

/*
 * MT safe
 */

#include "gimo-signalbus.h"
#include "gimo-context.h"

#define GIMO_SIGNAL_BUS_CAPACITY 2048

G_DEFINE_TYPE (GimoSignalBus, gimo_signal_bus, GIMO_TYPE_RUNNABLE)

enum {
    PROP_0,
    PROP_CONTEXT,
    PROP_OBJECT
};

struct _GimoSignalBusPrivate {
    GObject *object;
    GimoContext *context;
    GAsyncQueue *signals;
};

struct _GimoBusClosure {
    GClosure closure;
    GimoSignalBus *bus;
    guint signal_id;
};

struct _GimoBusSignal {
    guint signal_id;
    guint n_param_values;
    GValue *param_values;
};

G_LOCK_DEFINE_STATIC (sigbus_lock);

static void _gimo_signal_bus_reset (GimoContext *context,
                                    GimoSignalBus *self)
{
    GimoSignalBusPrivate *priv = self->priv;

    g_assert (context == priv->context);

    G_LOCK (sigbus_lock);

    context = priv->context;
    priv->context = NULL;

    G_UNLOCK (sigbus_lock);

    g_object_unref (context);
}

static GimoContext* _gimo_signal_bus_context (GimoSignalBus *self)
{
    GimoSignalBusPrivate *priv = self->priv;
    GimoContext *context;

    G_LOCK (sigbus_lock);

    context = priv->context;
    if (context)
        g_object_ref (context);

    G_UNLOCK (sigbus_lock);

    return context;
}

static struct _GimoBusSignal* _signal_bus_signal_create (GimoSignalBus *self,
                                                         guint signal_id,
                                                         guint n_param_values,
                                                         const GValue *param_values)
{
    struct _GimoBusSignal *signal = g_malloc (sizeof *signal);
    guint i;

    signal->signal_id = signal_id;
    signal->n_param_values = n_param_values;
    signal->param_values = g_malloc0 (n_param_values * sizeof (GValue));

    g_value_init (&signal->param_values[0], GIMO_TYPE_SIGNALBUS);
    g_value_set_object (&signal->param_values[0], self);

    for (i = 1; i < n_param_values; ++i) {
        g_value_init (&signal->param_values[i], param_values[i].g_type);
        g_value_copy (&param_values[i], &signal->param_values[i]);
    }

    return signal;
}

static void _signal_bus_signal_destroy (gpointer p)
{
    struct _GimoBusSignal *self = p;
    guint i;

    for (i = 0; i < self->n_param_values; ++i)
        g_value_unset (&self->param_values[i]);

    g_free (self->param_values);
    g_free (p);
}

static void _gimo_signal_bus_marshal (GClosure *closure,
                                      GValue *return_value G_GNUC_UNUSED,
                                      guint n_param_values,
                                      const GValue *param_values,
                                      gpointer invocation_hint G_GNUC_UNUSED,
                                      gpointer marshal_data)
{
    struct _GimoBusClosure *bus_closure = (struct _GimoBusClosure *) closure;
    struct _GimoBusSignal *signal;
    GimoSignalBus *self = bus_closure->bus;
    GimoSignalBusPrivate *priv = self->priv;
    GimoContext *context;

    g_assert (NULL == marshal_data && NULL == return_value);

    if (g_async_queue_length (priv->signals) > GIMO_SIGNAL_BUS_CAPACITY) {
        g_warning ("GimoSignalBus full");
        return;
    }

    context = _gimo_signal_bus_context (self);
    if (NULL == context)
        return;

    signal = _signal_bus_signal_create (self,
                                        bus_closure->signal_id,
                                        n_param_values,
                                        param_values);

    g_async_queue_push (priv->signals, signal);

    gimo_context_async_run (context, GIMO_RUNNABLE (self));
    g_object_unref (context);
}

static void gimo_signal_bus_init (GimoSignalBus *self)
{
    GimoSignalBusPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_SIGNALBUS,
                                              GimoSignalBusPrivate);
    priv = self->priv;

    priv->context = NULL;
    priv->object = NULL;
    priv->signals = g_async_queue_new_full (_signal_bus_signal_destroy);
}

static void gimo_signal_bus_finalize (GObject *gobject)
{
    GimoSignalBus *self = GIMO_SIGNALBUS (gobject);
    GimoSignalBusPrivate *priv = self->priv;
    GimoContext *context;

    context = _gimo_signal_bus_context (self);
    if (context) {
        g_signal_handlers_disconnect_by_data (context, self);
        g_object_unref (context);
    }

    g_async_queue_unref (priv->signals);

    G_OBJECT_CLASS (gimo_signal_bus_parent_class)->finalize (gobject);
}

static void gimo_signal_bus_set_property (GObject *object,
                                          guint prop_id,
                                          const GValue *value,
                                          GParamSpec *pspec)
{
    GimoSignalBus *self = GIMO_SIGNALBUS (object);
    GimoSignalBusPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_CONTEXT:
        priv->context = g_value_dup_object (value);

        if (priv->context) {
            g_signal_connect (priv->context,
                              "destroy",
                              G_CALLBACK (_gimo_signal_bus_reset),
                              self);
        }
        break;

    case PROP_OBJECT:
        priv->object = g_value_get_object (value);

        if (priv->object) {
            GObjectClass *klass;
            GClosure *closure;
            struct _GimoBusClosure *bus_closure;
            const gchar *name;
            guint *ids, i, n_ids;

            klass = G_OBJECT_GET_CLASS (self);
            ids = g_signal_list_ids (G_TYPE_FROM_CLASS (klass), &n_ids);

            for (i = 0; i < n_ids; ++i) {
                name = g_signal_name (ids[i]);

                closure = g_closure_new_simple (
                    sizeof (struct _GimoBusClosure), NULL);

                bus_closure = (struct _GimoBusClosure *) closure;
                bus_closure->bus = self;
                bus_closure->signal_id = ids[i];

                g_closure_set_marshal (closure, _gimo_signal_bus_marshal);

                g_signal_connect_closure (priv->object,
                                          name,
                                          closure,
                                          TRUE);
            }

            g_free (ids);
        }
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_signal_bus_get_property (GObject *object,
                                          guint prop_id,
                                          GValue *value,
                                          GParamSpec *pspec)
{
    GimoSignalBus *self = GIMO_SIGNALBUS (object);
    GimoSignalBusPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_CONTEXT:
        g_value_set_object (value, priv->context);
        break;

    case PROP_OBJECT:
        g_value_set_object (value, priv->object);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void _gimo_signal_bus_run (GimoRunnable *runnable)
{
    GimoSignalBus *self = GIMO_SIGNALBUS (runnable);
    GimoSignalBusPrivate *priv = self->priv;
    struct _GimoBusSignal *signal;

    while (g_async_queue_length (priv->signals) > 0) {
        signal = g_async_queue_pop (priv->signals);

        g_signal_emitv (signal->param_values,
                        signal->signal_id,
                        0,
                        NULL);

        _signal_bus_signal_destroy (signal);
    }
}

static void gimo_signal_bus_class_init (GimoSignalBusClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GimoRunnableClass *run_class = GIMO_RUNNABLE_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (GimoSignalBusPrivate));

    gobject_class->finalize = gimo_signal_bus_finalize;
    gobject_class->set_property = gimo_signal_bus_set_property;
    gobject_class->get_property = gimo_signal_bus_get_property;

    run_class->run = _gimo_signal_bus_run;

    g_object_class_install_property (
        gobject_class, PROP_CONTEXT,
        g_param_spec_object ("context",
                             "Context",
                             "The plugin context",
                             GIMO_TYPE_CONTEXT,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_OBJECT,
        g_param_spec_object ("object",
                             "Object",
                             "The source object",
                             G_TYPE_OBJECT,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));
}
