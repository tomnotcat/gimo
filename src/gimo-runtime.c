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
#include "gimo-runtime.h"
#include "gimo-error.h"
#include "gimo-utils.h"

G_DEFINE_TYPE (GimoRuntime, gimo_runtime, G_TYPE_OBJECT)

struct _GimoRuntimePrivate {
    GTree *objects;
};

static void gimo_runtime_init (GimoRuntime *self)
{
    GimoRuntimePrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_RUNTIME,
                                              GimoRuntimePrivate);
    priv = self->priv;

    priv->objects = NULL;
}

static void gimo_runtime_finalize (GObject *gobject)
{
    GimoRuntime *self = GIMO_RUNTIME (gobject);
    GimoRuntimePrivate *priv = self->priv;

    if (priv->objects)
        g_tree_unref (priv->objects);

    G_OBJECT_CLASS (gimo_runtime_parent_class)->finalize (gobject);
}

static void gimo_runtime_class_init (GimoRuntimeClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_runtime_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoRuntimePrivate));
}

GimoRuntime* gimo_runtime_new (void)
{
    return g_object_new (GIMO_TYPE_RUNTIME, NULL);
}

gboolean gimo_runtime_define_object (GimoRuntime *self,
                                     const gchar *symbol,
                                     GObject *object)
{
    GimoRuntimePrivate *priv;

    g_return_val_if_fail (GIMO_IS_RUNTIME (self), FALSE);

    priv = self->priv;

    if (NULL == priv->objects) {
        priv->objects = g_tree_new_full (_gimo_gtree_string_compare,
                                         NULL, g_free, g_object_unref);
    }
    else if (g_tree_lookup (priv->objects, symbol))
        gimo_set_error_return_val (GIMO_ERROR_CONFLICT, FALSE);

    g_tree_insert (priv->objects,
                   g_strdup (symbol),
                   g_object_ref (object));
    return TRUE;
}

gboolean gimo_runtime_start (GimoRuntime *self)
{
    return FALSE;
}

gboolean gimo_runtime_stop (GimoRuntime *self)
{
    return FALSE;
}

/**
 * gimo_runtime_resolve:
 * @self: a #GimoRuntime
 * @symbol: the symbol name
 *
 * Resolve the plugin runtime information.
 *
 * Returns: (allow-none) (transfer full): a #GObject
 */
GObject* gimo_runtime_resolve (GimoRuntime *self,
                               const gchar *symbol)
{
    GimoRuntimePrivate *priv;
    GObject *object = NULL;

    g_return_val_if_fail (GIMO_IS_RUNTIME (self), NULL);

    priv = self->priv;

    if (priv->objects)
        object = g_tree_lookup (priv->objects, symbol);

    if (object)
        g_object_ref (object);

    return object;
}
