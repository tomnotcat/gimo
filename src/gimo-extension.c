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
#include "gimo-extension.h"

G_DEFINE_TYPE (GimoExtension, gimo_extension, G_TYPE_OBJECT)

struct _GimoExtensionPrivate {
    int n;
};

static void gimo_extension_init (GimoExtension *self)
{
    GimoExtensionPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_EXTENSION,
                                              GimoExtensionPrivate);
    priv = self->priv;

    priv->n = 0;
}

static void gimo_extension_finalize (GObject *gobject)
{
    G_OBJECT_CLASS (gimo_extension_parent_class)->finalize (gobject);
}

static void gimo_extension_class_init (GimoExtensionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_extension_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoExtensionPrivate));
}

GimoExtension* gimo_extension_new (const gchar *local_id,
                                   const gchar *name,
                                   const gchar *extpoint_id)
{
    return g_object_new (GIMO_TYPE_EXTENSION,
                         "local-id", local_id,
                         "name", name,
                         "extpoint-id", extpoint_id,
                         NULL);
}

const gchar* gimo_extension_get_local_id (GimoExtension *self)
{
    g_return_val_if_fail (GIMO_IS_EXTENSION (self), NULL);

    return self->priv->local_id;
}

const gchar* gimo_extension_get_name (GimoExtension *self)
{
    g_return_val_if_fail (GIMO_IS_EXTENSION (self), NULL);

    return self->priv->name;
}

const gchar* gimo_extension_get_extpoint_id (GimoExtension *self)
{
    g_return_val_if_fail (GIMO_IS_EXTENSION (self), NULL);

    return self->priv->extpoint_id;
}

/**
 * gimo_extension_get_pluginfo:
 * @self: a #GimoExtpoint
 *
 * Get the plugin descriptor of the extension.
 *
 * Returns: (transfer none): a #GimoPluginfo
 */
GimoPluginfo* gimo_extension_get_pluginfo (GimoExtension *self)
{
    g_return_val_if_fail (GIMO_IS_EXTENSION (self), NULL);

    return self->priv->info;
}

const gchar* gimo_extension_get_identifier (GimoExtension *self)
{
    g_return_val_if_fail (GIMO_IS_EXTENSION (self), NULL);

    return self->priv->identifier;
}
