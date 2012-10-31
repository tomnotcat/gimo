/* GIMO - A plugin system based on GObject.
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

#include "gimo-extpoint.h"
#include "gimo-pluginfo.h"

G_DEFINE_TYPE (GimoExtPoint, gimo_extpoint, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_ID,
    PROP_NAME
};

struct _GimoExtPointPrivate {
    GimoPluginfo *info;
    gchar *id;
    gchar *local_id;
    gchar *name;
};

G_LOCK_DEFINE_STATIC (extpoint_lock);

static void gimo_extpoint_init (GimoExtPoint *self)
{
    GimoExtPointPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_EXTPOINT,
                                              GimoExtPointPrivate);
    priv = self->priv;

    priv->info = NULL;
    priv->id = NULL;
    priv->local_id = NULL;
    priv->name = NULL;
}

static void gimo_extpoint_finalize (GObject *gobject)
{
    GimoExtPoint *self = GIMO_EXTPOINT (gobject);
    GimoExtPointPrivate *priv = self->priv;

    g_assert (NULL == priv->info);

    g_free (priv->local_id);
    g_free (priv->id);
    g_free (priv->name);

    G_OBJECT_CLASS (gimo_extpoint_parent_class)->finalize (gobject);
}

static void gimo_extpoint_set_property (GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
    GimoExtPoint *self = GIMO_EXTPOINT (object);
    GimoExtPointPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_ID:
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
    GimoExtPoint *self = GIMO_EXTPOINT (object);
    GimoExtPointPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_ID:
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

static void gimo_extpoint_class_init (GimoExtPointClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_extpoint_finalize;
    gobject_class->set_property = gimo_extpoint_set_property;
    gobject_class->get_property = gimo_extpoint_get_property;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoExtPointPrivate));

    g_object_class_install_property (
        gobject_class, PROP_ID,
        g_param_spec_string ("id",
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

GimoExtPoint* gimo_extpoint_new (const gchar *id,
                                 const gchar *name)
{
    return g_object_new (GIMO_TYPE_EXTPOINT,
                         "id", id,
                         "name", name,
                         NULL);
}

const gchar* gimo_extpoint_get_local_id (GimoExtPoint *self)
{
    g_return_val_if_fail (GIMO_IS_EXTPOINT (self), NULL);

    return self->priv->local_id;
}

const gchar* gimo_extpoint_get_name (GimoExtPoint *self)
{
    g_return_val_if_fail (GIMO_IS_EXTPOINT (self), NULL);

    return self->priv->name;
}

const gchar* gimo_extpoint_get_id (GimoExtPoint *self)
{
    g_return_val_if_fail (GIMO_IS_EXTPOINT (self), NULL);

    return self->priv->id;
}

/**
 * gimo_extpoint_query_pluginfo:
 * @self: a #GimoExtPoint
 *
 * Query the plugin descriptor of the extension point.
 *
 * Returns: (allow-none) (transfer full): a #GimoPluginfo
 */
GimoPluginfo* gimo_extpoint_query_pluginfo (GimoExtPoint *self)
{
    GimoExtPointPrivate *priv;
    GimoPluginfo *info = NULL;

    g_return_val_if_fail (GIMO_IS_EXTPOINT (self), NULL);

    priv = self->priv;

    G_LOCK (extpoint_lock);

    if (priv->info)
        info = g_object_ref (priv->info);

    G_UNLOCK (extpoint_lock);

    return info;
}

/**
 * gimo_extpoint_resolve:
 * @self: a #GimoExtPoint
 *
 * Resolve the extention point runtime information.
 *
 * Returns: (allow-none) (transfer full): a #GObject
 */
GObject* gimo_extpoint_resolve (GimoExtPoint *self)
{
    return NULL;
}

void _gimo_extpoint_setup (GimoExtPoint *self,
                           GimoPluginfo *info)
{
    GimoExtPointPrivate *priv = self->priv;

    g_assert (NULL == priv->id);

    G_LOCK (extpoint_lock);

    priv->info = info;

    priv->id = g_strdup_printf ("%s.%s",
                                gimo_pluginfo_get_id (info),
                                priv->local_id);
    G_UNLOCK (extpoint_lock);
}

void _gimo_extpoint_teardown (GimoExtPoint *self,
                              GimoPluginfo *info)
{
    GimoExtPointPrivate *priv = self->priv;

    g_assert (priv->info == info);

    G_LOCK (extpoint_lock);
    priv->info = NULL;
    G_UNLOCK (extpoint_lock);
}
