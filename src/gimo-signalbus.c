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

G_DEFINE_TYPE (GimoSignalBus, gimo_signal_bus, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_CONTEXT
};

struct _GimoSignalBusPrivate {
    GimoContext *context;
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

static void gimo_signal_bus_init (GimoSignalBus *self)
{
    GimoSignalBusPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_SIGNALBUS,
                                              GimoSignalBusPrivate);
    priv = self->priv;

    priv->context = NULL;
}

static void gimo_signal_bus_finalize (GObject *gobject)
{
    GimoSignalBus *self = GIMO_SIGNALBUS (gobject);
    GimoContext *context;

    context = _gimo_signal_bus_context (self);
    if (context) {
        g_signal_handlers_disconnect_by_data (context, self);
        g_object_unref (context);
    }

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

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_signal_bus_class_init (GimoSignalBusClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (GimoSignalBusPrivate));

    gobject_class->finalize = gimo_signal_bus_finalize;
    gobject_class->set_property = gimo_signal_bus_set_property;
    gobject_class->get_property = gimo_signal_bus_get_property;

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
}
