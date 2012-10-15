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
#include "gimo-plugin.h"

G_DEFINE_TYPE (GimoPlugin, gimo_plugin, G_TYPE_OBJECT)

struct _GimoPluginPrivate {
    gchar *id;
    gchar *url;
    gchar *entry;
    gchar *name;
    gchar *version;
    gchar *provider;
    GSList *imports;
    GSList *extpoints;
    GSList *extensions;
    GimoPluginState state;
};

static void gimo_plugin_init (GimoPlugin *self)
{
    GimoPluginPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_PLUGIN,
                                              GimoPluginPrivate);
    priv = self->priv;

    priv->id = NULL;
    priv->url = NULL;
    priv->entry = NULL;
    priv->name = NULL;
    priv->version = NULL;
    priv->provider = NULL;
    priv->imports = NULL;
    priv->extpoints = NULL;
    priv->extensions = NULL;
    priv->state = GIMO_PLUGIN_UNINSTALLED;
}

static void gimo_plugin_finalize (GObject *gobject)
{
    GimoPlugin *self = GIMO_PLUGIN (gobject);
    GimoPluginPrivate *priv = self->priv;

    g_free (priv->id);
    g_free (priv->url);
    g_free (priv->entry);
    g_free (priv->name);
    g_free (priv->version);
    g_free (priv->provider);

    g_slist_free_full (priv->imports, g_object_unref);
    g_slist_free_full (priv->extpoints, g_object_unref);
    g_slist_free_full (priv->extensions, g_object_unref);

    G_OBJECT_CLASS (gimo_plugin_parent_class)->finalize (gobject);
}

static void gimo_plugin_class_init (GimoPluginClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (GimoPluginPrivate));

    gobject_class->finalize = gimo_plugin_finalize;
}

GimoPlugin* gimo_plugin_new (const gchar *id,
                             const gchar *url,
                             const gchar *entry,
                             const gchar *name,
                             const gchar *version,
                             const gchar *provider,
                             GSList *imports,
                             GSList *extpoints,
                             GSList *extensions)
{
    GimoPlugin *self = g_object_new (GIMO_TYPE_PLUGIN, NULL);
    GimoPluginPrivate *priv = self->priv;
    GSList *it;

    priv->id = g_strdup (id);
    priv->url = g_strdup (url);
    priv->entry = g_strdup (entry);
    priv->name = g_strdup (name);
    priv->version = g_strdup (version);
    priv->provider = g_strdup (provider);

    for (it = imports; it != NULL; it = it->next) {
        priv->imports = g_slist_prepend (priv->imports,
                                         g_object_ref (it->data));
    }

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

const gchar* gimo_plugin_get_id (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    return self->priv->id;
}

const gchar* gimo_plugin_get_url (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    return self->priv->url;
}

const gchar* gimo_plugin_get_entry (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    return self->priv->entry;
}

const gchar* gimo_plugin_get_name (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    return self->priv->name;
}

const gchar* gimo_plugin_get_version (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    return self->priv->version;
}

const gchar* gimo_plugin_get_provider (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    return self->priv->provider;
}

GSList* gimo_plugin_get_imports (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    return self->priv->imports;
}

GSList* gimo_plugin_get_extpoints (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    return self->priv->extpoints;
}

GSList* gimo_plugin_get_extensions (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    return self->priv->extensions;
}

GimoPluginState gimo_plugin_get_state (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), GIMO_PLUGIN_UNINSTALLED);

    return self->priv->state;
}

GimoStatus gimo_plugin_start (GimoPlugin *self)
{
    GimoPluginPrivate *priv;

    g_return_val_if_fail (GIMO_IS_PLUGIN (self), GIMO_STATUS_INVALID_OBJECT);

    priv = self->priv;
    (void) priv;

    return GIMO_STATUS_SUCCESS;
}

GimoStatus gimo_plugin_stop (GimoPlugin *self)
{
    GimoPluginPrivate *priv;

    g_return_val_if_fail (GIMO_IS_PLUGIN (self), GIMO_STATUS_INVALID_OBJECT);

    priv = self->priv;
    (void) priv;

    return GIMO_STATUS_SUCCESS;
}
