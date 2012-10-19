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

enum {
    PROP_0,
    PROP_LOCALID,
    PROP_NAME,
    PROP_EXTPOINTID
};

struct _GimoExtensionPrivate {
    gchar *local_id;
    gchar *name;
    gchar *extpoint_id;
    gchar *identifier;
    GimoPluginfo *info;
};

static void gimo_extension_init (GimoExtension *self)
{
    GimoExtensionPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_EXTENSION,
                                              GimoExtensionPrivate);
    priv = self->priv;

    priv->local_id = NULL;
    priv->name = NULL;
    priv->extpoint_id = NULL;
    priv->identifier = NULL;
    priv->info = NULL;
}

static void gimo_extension_finalize (GObject *gobject)
{
    GimoExtension *self = GIMO_EXTENSION (gobject);
    GimoExtensionPrivate *priv = self->priv;

    g_free (priv->local_id);
    g_free (priv->name);
    g_free (priv->extpoint_id);
    g_free (priv->identifier);

    G_OBJECT_CLASS (gimo_extension_parent_class)->finalize (gobject);
}

static void gimo_extension_set_property (GObject *object,
                                         guint prop_id,
                                         const GValue *value,
                                         GParamSpec *pspec)
{
    GimoExtension *self = GIMO_EXTENSION (object);
    GimoExtensionPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_LOCALID:
        priv->local_id = g_value_dup_string (value);
        break;

    case PROP_NAME:
        priv->name = g_value_dup_string (value);
        break;

    case PROP_EXTPOINTID:
        priv->extpoint_id = g_value_dup_string (value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_extension_get_property (GObject *object,
                                         guint prop_id,
                                         GValue *value,
                                         GParamSpec *pspec)
{
    GimoExtension *self = GIMO_EXTENSION (object);
    GimoExtensionPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_LOCALID:
        g_value_set_string (value, priv->local_id);
        break;

    case PROP_NAME:
        g_value_set_string (value, priv->name);
        break;

    case PROP_EXTPOINTID:
        g_value_set_string (value, priv->extpoint_id);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_extension_class_init (GimoExtensionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_extension_finalize;
    gobject_class->set_property = gimo_extension_set_property;
    gobject_class->get_property = gimo_extension_get_property;

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

const gchar* gimo_extension_get_identifier (GimoExtension *self)
{
    g_return_val_if_fail (GIMO_IS_EXTENSION (self), NULL);

    return self->priv->identifier;
}

/**
 * gimo_extension_query_pluginfo:
 * @self: a #GimoExtpoint
 *
 * Query the plugin descriptor of the extension.
 *
 * Returns: (allow-none) (transfer full): a #GimoPluginfo
 */
GimoPluginfo* gimo_extension_query_pluginfo (GimoExtension *self)
{
    g_return_val_if_fail (GIMO_IS_EXTENSION (self), NULL);

    if (self->priv->info)
        return g_object_ref (self->priv->info);

    return NULL;
}
