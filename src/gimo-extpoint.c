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

#include "gimo-extpoint.h"
#include "gimo-error.h"
#include "gimo-plugin.h"
#include <string.h>

G_DEFINE_TYPE (GimoExtPoint, gimo_ext_point, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_ID,
    PROP_NAME
};

struct _GimoExtPointPrivate {
    GimoPlugin *plugin;
    gchar *id;
    gchar *local_id;
    gchar *name;
};

G_LOCK_DEFINE_STATIC (extpoint_lock);

static void gimo_ext_point_init (GimoExtPoint *self)
{
    GimoExtPointPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_EXTPOINT,
                                              GimoExtPointPrivate);
    priv = self->priv;

    priv->plugin = NULL;
    priv->id = NULL;
    priv->local_id = NULL;
    priv->name = NULL;
}

static void gimo_ext_point_finalize (GObject *gobject)
{
    GimoExtPoint *self = GIMO_EXTPOINT (gobject);
    GimoExtPointPrivate *priv = self->priv;

    g_assert (NULL == priv->plugin);

    g_free (priv->local_id);
    g_free (priv->id);
    g_free (priv->name);

    G_OBJECT_CLASS (gimo_ext_point_parent_class)->finalize (gobject);
}

static void gimo_ext_point_set_property (GObject *object,
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

static void gimo_ext_point_get_property (GObject *object,
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

static void gimo_ext_point_class_init (GimoExtPointClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_ext_point_finalize;
    gobject_class->set_property = gimo_ext_point_set_property;
    gobject_class->get_property = gimo_ext_point_get_property;

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

GimoExtPoint* gimo_ext_point_new (const gchar *id,
                                  const gchar *name)
{
    return g_object_new (GIMO_TYPE_EXTPOINT,
                         "id", id,
                         "name", name,
                         NULL);
}

const gchar* gimo_ext_point_get_local_id (GimoExtPoint *self)
{
    g_return_val_if_fail (GIMO_IS_EXTPOINT (self), NULL);

    return self->priv->local_id;
}

const gchar* gimo_ext_point_get_name (GimoExtPoint *self)
{
    g_return_val_if_fail (GIMO_IS_EXTPOINT (self), NULL);

    return self->priv->name;
}

const gchar* gimo_ext_point_get_id (GimoExtPoint *self)
{
    g_return_val_if_fail (GIMO_IS_EXTPOINT (self), NULL);

    return self->priv->id;
}

/**
 * gimo_ext_point_query_plugin:
 * @self: a #GimoExtPoint
 *
 * Query the plugin descriptor of the extension point.
 *
 * Returns: (allow-none) (transfer full): a #GimoPlugin
 */
GimoPlugin* gimo_ext_point_query_plugin (GimoExtPoint *self)
{
    GimoExtPointPrivate *priv;
    GimoPlugin *plugin = NULL;

    g_return_val_if_fail (GIMO_IS_EXTPOINT (self), NULL);

    priv = self->priv;

    G_LOCK (extpoint_lock);

    if (priv->plugin)
        plugin = g_object_ref (priv->plugin);

    G_UNLOCK (extpoint_lock);

    return plugin;
}

void _gimo_ext_point_setup (GimoExtPoint *self,
                            GimoPlugin *plugin)
{
    GimoExtPointPrivate *priv = self->priv;

    g_assert (NULL == priv->id);

    G_LOCK (extpoint_lock);

    priv->plugin = plugin;

    priv->id = g_strdup_printf ("%s.%s",
                                gimo_plugin_get_id (plugin),
                                priv->local_id);
    G_UNLOCK (extpoint_lock);
}

void _gimo_ext_point_teardown (GimoExtPoint *self,
                               GimoPlugin *plugin)
{
    GimoExtPointPrivate *priv = self->priv;

    g_assert (priv->plugin == plugin);

    G_LOCK (extpoint_lock);

    priv->plugin = NULL;

    G_UNLOCK (extpoint_lock);
}

gint _gimo_ext_point_sort_by_id (gconstpointer a,
                                 gconstpointer b)
{
    GimoExtPoint *p1 = *(GimoExtPoint **) a;
    GimoExtPoint *p2 = *(GimoExtPoint **) b;

    return strcmp (p1->priv->local_id, p2->priv->local_id);
}

gint _gimo_ext_point_search_by_id (gconstpointer a,
                                   gconstpointer b)
{
    GimoExtPoint *p = *(GimoExtPoint **) b;

    return strcmp (a, p->priv->local_id);
}
