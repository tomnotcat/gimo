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
#include "gimo-extpoint.h"
#include "gimo-pluginfo.h"

G_DEFINE_TYPE (GimoExtpoint, gimo_extpoint, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_LOCALID,
    PROP_NAME
};

struct _GimoExtpointPrivate {
    GimoPluginfo *info;
    gchar *identifier;
    gchar *local_id;
    gchar *name;
};

static void gimo_extpoint_init (GimoExtpoint *self)
{
    GimoExtpointPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_EXTPOINT,
                                              GimoExtpointPrivate);
    priv = self->priv;

    priv->info = NULL;
    priv->identifier = NULL;
    priv->local_id = NULL;
    priv->name = NULL;
}

static void gimo_extpoint_finalize (GObject *gobject)
{
    GimoExtpoint *self = GIMO_EXTPOINT (gobject);
    GimoExtpointPrivate *priv = self->priv;

    g_free (priv->local_id);
    g_free (priv->identifier);
    g_free (priv->name);

    G_OBJECT_CLASS (gimo_extpoint_parent_class)->finalize (gobject);
}

static void gimo_extpoint_set_property (GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
    GimoExtpoint *self = GIMO_EXTPOINT (object);
    GimoExtpointPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_LOCALID:
        priv->local_id = g_value_dup_string (value);
        break;

    case PROP_NAME:
        priv->name = g_value_dup_string (value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_extpoint_get_property (GObject *object,
                                        guint prop_id,
                                        GValue *value,
                                        GParamSpec *pspec)
{
    GimoExtpoint *self = GIMO_EXTPOINT (object);
    GimoExtpointPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_LOCALID:
        g_value_set_string (value, priv->local_id);
        break;

    case PROP_NAME:
        g_value_set_string (value, priv->name);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_extpoint_class_init (GimoExtpointClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_extpoint_finalize;
    gobject_class->set_property = gimo_extpoint_set_property;
    gobject_class->get_property = gimo_extpoint_get_property;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoExtpointPrivate));

    g_object_class_install_property (
        gobject_class, PROP_LOCALID,
        g_param_spec_string ("local-id",
                             "Local identifier",
                             "The local identifier of the extension point",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_NAME,
        g_param_spec_string ("name",
                             "Extension point name",
                             "The display name of the extension point",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));
}

GimoExtpoint* gimo_extpoint_new (const gchar *local_id,
                                 const gchar *name)
{
    return g_object_new (GIMO_TYPE_EXTPOINT,
                         "local-id", local_id,
                         "name", name,
                         NULL);
}

const gchar* gimo_extpoint_get_local_id (GimoExtpoint *self)
{
    g_return_val_if_fail (GIMO_IS_EXTPOINT (self), NULL);

    return self->priv->local_id;
}

const gchar* gimo_extpoint_get_name (GimoExtpoint *self)
{
    g_return_val_if_fail (GIMO_IS_EXTPOINT (self), NULL);

    return self->priv->name;
}

const gchar* gimo_extpoint_get_identifier (GimoExtpoint *self)
{
    g_return_val_if_fail (GIMO_IS_EXTPOINT (self), NULL);

    return self->priv->identifier;
}

/**
 * gimo_extpoint_query_pluginfo:
 * @self: a #GimoExtpoint
 *
 * Query the plugin descriptor of the extension point.
 *
 * Returns: (allow-none) (transfer full): a #GimoPluginfo
 */
GimoPluginfo* gimo_extpoint_query_pluginfo (GimoExtpoint *self)
{
    g_return_val_if_fail (GIMO_IS_EXTPOINT (self), NULL);

    if (self->priv->info)
        return g_object_ref (self->priv->info);

    return NULL;
}

void _gimo_extpoint_setup (GimoExtpoint *self,
                           GimoPluginfo *info)
{
    GimoExtpointPrivate *priv = self->priv;

    g_assert (NULL == priv->identifier);

    priv->info = g_object_ref (info);
    priv->identifier = g_strdup_printf ("%s.%s",
                                        gimo_pluginfo_get_identifier (info),
                                        priv->local_id);
}

void _gimo_extpoint_teardown (GimoExtpoint *self)
{
    g_object_unref (self->priv->info);
}
