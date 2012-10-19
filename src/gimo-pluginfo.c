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
#include "gimo-pluginfo.h"

G_DEFINE_TYPE (GimoPluginfo, gimo_pluginfo, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_REQUIRES
};

struct _GimoPluginfoPrivate {
    gchar *identifier;
    gchar *url;
    gchar *klass;
    gchar *name;
    gchar *version;
    gchar *provider;
    GSList *requires;
    GSList *extpoints;
    GSList *extensions;
};

static void gimo_pluginfo_init (GimoPluginfo *self)
{
    GimoPluginfoPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_PLUGINFO,
                                              GimoPluginfoPrivate);
    priv = self->priv;

    priv->identifier = NULL;
    priv->url = NULL;
    priv->klass = NULL;
    priv->name = NULL;
    priv->version = NULL;
    priv->provider = NULL;
    priv->requires = NULL;
    priv->extpoints = NULL;
    priv->extensions = NULL;
}

static void gimo_pluginfo_finalize (GObject *gobject)
{
    GimoPluginfo *self = GIMO_PLUGINFO (gobject);
    GimoPluginfoPrivate *priv = self->priv;

    g_free (priv->identifier);
    g_free (priv->url);
    g_free (priv->klass);
    g_free (priv->name);
    g_free (priv->version);
    g_free (priv->provider);

    g_slist_free_full (priv->requires, g_object_unref);
    g_slist_free_full (priv->extpoints, g_object_unref);
    g_slist_free_full (priv->extensions, g_object_unref);

    G_OBJECT_CLASS (gimo_pluginfo_parent_class)->finalize (gobject);
}

static void gimo_pluginfo_set_property (GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
    GimoPluginfo *self = GIMO_PLUGINFO (object);
    GimoPluginfoPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_REQUIRES:
        {
            GSList *it = g_value_get_pointer (value);

            while (it) {
                priv->requires = g_slist_prepend (priv->requires,
                                                  g_object_ref (it->data));
                it = it->next;
            }
        }
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_pluginfo_get_property (GObject *object,
                                        guint prop_id,
                                        GValue *value,
                                        GParamSpec *pspec)
{
    GimoPluginfo *self = GIMO_PLUGINFO (object);
    GimoPluginfoPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_REQUIRES:
        g_value_set_pointer (value, priv->requires);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_pluginfo_class_init (GimoPluginfoClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (GimoPluginfoPrivate));

    gobject_class->finalize = gimo_pluginfo_finalize;
    gobject_class->set_property = gimo_pluginfo_set_property;
    gobject_class->get_property = gimo_pluginfo_get_property;

    /**
     * GimoPluginfo:requires:
     * @pspec: (type Gimo.Pluginfo):
     *
     * The required plugins.
     */
    g_object_class_install_property (
        gobject_class, PROP_REQUIRES,
        g_param_spec_pointer ("requires",
                              "Required plugins",
                              "The plugins required by this plugin",
                              G_PARAM_READABLE |
                              G_PARAM_WRITABLE |
                              G_PARAM_CONSTRUCT_ONLY |
                              G_PARAM_STATIC_STRINGS));
}

/**
 * gimo_pluginfo_new:
 * @identifier: the unique identifier
 * @url: url/path of the plugin
 * @klass: the runtime class name
 * @name: the display name
 * @version: the release version
 * @provider: the provider name
 * @requires: (element-type Gimo.Extension) (transfer none):
 *            the required plugins
 * @extpoints: (element-type Gimo.Extension) (transfer none):
 *             the extension points
 * @extensions: (element-type Gimo.Extension) (transfer none):
 *              the extensions
 *
 * Create a plugin descriptor of the provided parameters.
 */
GimoPluginfo* gimo_pluginfo_new (const gchar *identifier,
                                 const gchar *url,
                                 const gchar *klass,
                                 const gchar *name,
                                 const gchar *version,
                                 const gchar *provider,
                                 GSList *requires,
                                 GSList *extpoints,
                                 GSList *extensions)
{
    GimoPluginfo *self = g_object_new (GIMO_TYPE_PLUGINFO,
                                       "requires", requires,
                                       NULL);
    GimoPluginfoPrivate *priv = self->priv;
    GSList *it;

    priv->identifier = g_strdup (identifier);
    priv->url = g_strdup (url);
    priv->klass = g_strdup (klass);
    priv->name = g_strdup (name);
    priv->version = g_strdup (version);
    priv->provider = g_strdup (provider);

    for (it = extpoints; it != NULL; it = it->next) {
        priv->extpoints = g_slist_prepend (priv->extpoints,
                                           g_object_ref (it->data));
    }

    for (it = extensions; it != NULL; it = it->next) {
        priv->extensions = g_slist_prepend (priv->extensions,
                                            g_object_ref (it->data));
    }

    return self;
}

const gchar* gimo_pluginfo_get_identifier (GimoPluginfo *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGINFO (self), NULL);

    return self->priv->identifier;
}

const gchar* gimo_pluginfo_get_url (GimoPluginfo *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGINFO (self), NULL);

    return self->priv->url;
}

const gchar* gimo_pluginfo_get_klass (GimoPluginfo *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGINFO (self), NULL);

    return self->priv->klass;
}

const gchar* gimo_pluginfo_get_name (GimoPluginfo *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGINFO (self), NULL);

    return self->priv->name;
}

const gchar* gimo_pluginfo_get_version (GimoPluginfo *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGINFO (self), NULL);

    return self->priv->version;
}

const gchar* gimo_pluginfo_get_provider (GimoPluginfo *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGINFO (self), NULL);

    return self->priv->provider;
}

/**
 * gimo_pluginfo_get_requires:
 * @self: a #GimoPluginfo
 *
 * Get the required plugins of the plugin.
 *
 * Returns: (element-type Gimo.Require) (transfer none):
 *          the required plugins list.
 */
GSList* gimo_pluginfo_get_requires (GimoPluginfo *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGINFO (self), NULL);

    return self->priv->requires;
}

/**
 * gimo_pluginfo_get_extpoints:
 * @self: a #GimoPluginfo
 *
 * Get the extension points provided by the plugin.
 *
 * Returns: (element-type Gimo.Extpoint) (transfer none):
 *          the extension points list.
 */
GSList* gimo_pluginfo_get_extpoints (GimoPluginfo *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGINFO (self), NULL);

    return self->priv->extpoints;
}

/**
 * gimo_pluginfo_get_extensions:
 * @self: a #GimoPluginfo
 *
 * Get the extensions provided by the plugin.
 *
 * Returns: (element-type Gimo.Extension) (transfer none):
 *          the extensions list.
 */
GSList* gimo_pluginfo_get_extensions (GimoPluginfo *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGINFO (self), NULL);

    return self->priv->extensions;
}
