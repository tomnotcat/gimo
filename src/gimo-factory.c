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
#include "gimo-factory.h"
#include "gimo-marshal.h"

G_DEFINE_TYPE (GimoFactory, gimo_factory, G_TYPE_OBJECT)

enum {
    SIG_MAKE,
    LAST_SIGNAL
};

struct _GimoFactoryPrivate {
    GimoFactoryFunc func;
    gpointer user_data;
};

static guint factory_signals[LAST_SIGNAL] = { 0 };

static void gimo_factory_init (GimoFactory *self)
{
    GimoFactoryPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_FACTORY,
                                              GimoFactoryPrivate);
    priv = self->priv;

    priv->func = NULL;
    priv->user_data = NULL;
}

static void gimo_factory_class_init (GimoFactoryClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (GimoFactoryPrivate));

    klass->make = NULL;

    /**
     * GimoFactory::make:
     * @widget: the object which received the signal
     * @drag_context: the drag context
     * @result: the result of the drag operation
     *
     * The ::make signal is emitted to make a new object.
     *
     * Returns: (allow-none) (transfer full):
     *          A #GObject if successful, %NULL on error.
     *          Free the returned object with g_object_unref().
     */
    factory_signals[SIG_MAKE] =
            g_signal_new ("make",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (GimoFactoryClass, make),
                          NULL, NULL,
                          _gimo_marshal_OBJECT__VOID,
                          G_TYPE_OBJECT, 0);
}

/**
 * gimo_factory_new:
 * @func: (scope async) (allow-none): the factory function
 * @user_data: (allow-none): user data for @func
 *
 * Create a factory with the specified callback.
 */
GimoFactory* gimo_factory_new (GimoFactoryFunc func,
                               gpointer user_data)
{
    GimoFactory *self = g_object_new (GIMO_TYPE_FACTORY, NULL);
    GimoFactoryPrivate *priv = self->priv;

    priv->func = func;
    priv->user_data = user_data;

    return self;
}

/**
 * gimo_factory_make:
 * @self: a #GimoFactory
 *
 * Make an object from the factory.
 *
 * Returns: (allow-none) (transfer full): A #GObject if
 *          successful, %NULL on error. Free the returned
 *          object with g_object_unref().
 */
GObject* gimo_factory_make (GimoFactory *self)
{
    GimoFactoryPrivate *priv;
    GObject *result = NULL;

    g_return_val_if_fail (GIMO_IS_FACTORY (self), NULL);

    priv = self->priv;

    g_signal_emit (self,
                   factory_signals[SIG_MAKE],
                   0,
                   &result);

    if (NULL == result && priv->func)
        result = priv->func (priv->user_data);

    return result;
}
