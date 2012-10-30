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

#include "gimo-require.h"

G_DEFINE_TYPE (GimoRequire, gimo_require, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_PLUGINID,
    PROP_VERSION,
    PROP_OPTIONAL
};

struct _GimoRequirePrivate {
    gchar *plugin_id;
    gchar *version;
    gboolean optional;
};

static void gimo_require_init (GimoRequire *self)
{
    GimoRequirePrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_REQUIRE,
                                              GimoRequirePrivate);
    priv = self->priv;

    priv->plugin_id = NULL;
    priv->version = NULL;
    priv->optional = FALSE;
}

static void gimo_require_finalize (GObject *gobject)
{
    GimoRequire *self = GIMO_REQUIRE (gobject);
    GimoRequirePrivate *priv = self->priv;

    g_free (priv->plugin_id);
    g_free (priv->version);

    G_OBJECT_CLASS (gimo_require_parent_class)->finalize (gobject);
}

static void gimo_require_set_property (GObject *object,
                                       guint prop_id,
                                       const GValue *value,
                                       GParamSpec *pspec)
{
    GimoRequire *self = GIMO_REQUIRE (object);
    GimoRequirePrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_PLUGINID:
        priv->plugin_id = g_value_dup_string (value);
        break;

    case PROP_VERSION:
        priv->version = g_value_dup_string (value);
        break;

    case PROP_OPTIONAL:
        priv->optional = g_value_get_boolean (value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_require_get_property (GObject *object,
                                       guint prop_id,
                                       GValue *value,
                                       GParamSpec *pspec)
{
    GimoRequire *self = GIMO_REQUIRE (object);
    GimoRequirePrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_PLUGINID:
        g_value_set_string (value, priv->plugin_id);
        break;

    case PROP_VERSION:
        g_value_set_string (value, priv->version);
        break;

    case PROP_OPTIONAL:
        g_value_set_boolean (value, priv->optional);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_require_class_init (GimoRequireClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_require_finalize;
    gobject_class->set_property = gimo_require_set_property;
    gobject_class->get_property = gimo_require_get_property;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoRequirePrivate));

    g_object_class_install_property (
        gobject_class, PROP_PLUGINID,
        g_param_spec_string ("plugin-id",
                             "Plugin identifier",
                             "The identifier of the plugin",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_VERSION,
        g_param_spec_string ("version",
                             "Plugin version",
                             "The required version of the plugin",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_OPTIONAL,
        g_param_spec_boolean ("optional",
                              "Optional",
                              "Whether or not the plugin is optional",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_CONSTRUCT_ONLY |
                              G_PARAM_STATIC_STRINGS));
}

GimoRequire* gimo_require_new (const gchar *plugin_id,
                               const gchar *version,
                               gboolean optional)
{
    return g_object_new (GIMO_TYPE_REQUIRE,
                         "plugin-id", plugin_id,
                         "version", version,
                         "optional", optional,
                         NULL);
}

const gchar* gimo_require_get_plugin_id (GimoRequire *self)
{
    g_return_val_if_fail (GIMO_IS_REQUIRE (self), NULL);

    return self->priv->plugin_id;
}

const gchar* gimo_require_get_version (GimoRequire *self)
{
    g_return_val_if_fail (GIMO_IS_REQUIRE (self), NULL);

    return self->priv->version;
}

gboolean gimo_require_is_optional (GimoRequire *self)
{
    g_return_val_if_fail (GIMO_IS_REQUIRE (self), FALSE);

    return self->priv->optional;
}
