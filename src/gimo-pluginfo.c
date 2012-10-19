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
#include "gimo-extension.h"
#include "gimo-extpoint.h"
#include "gimo-require.h"
#include "gimo-utils.h"

extern void _gimo_extpoint_setup (gpointer self,
                                  gpointer info);

G_DEFINE_TYPE (GimoPluginfo, gimo_pluginfo, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_IDENTIFIER,
    PROP_URL,
    PROP_KLASS,
    PROP_NAME,
    PROP_VERSION,
    PROP_PROVIDER,
    PROP_REQUIRES,
    PROP_EXTPOINTS,
    PROP_EXTENSIONS
};

struct _GimoPluginfoPrivate {
    gchar *identifier;
    gchar *url;
    gchar *klass;
    gchar *name;
    gchar *version;
    gchar *provider;
    GPtrArray *requires;
    GPtrArray *extpoints;
    GPtrArray *extensions;
};

static GPtrArray* _gimo_pluginfo_clone_array (GimoPluginfo *self,
                                              GPtrArray *arr,
                                              GType type,
                                              void (*func) (gpointer, gpointer))
{
    GPtrArray *result;
    guint i;

    if (NULL == arr)
        return NULL;

    result = g_ptr_array_new_full (arr->len, g_object_unref);
    for (i = 0; i < arr->len; ++i) {
        g_ptr_array_add (result,
                         g_object_ref (g_ptr_array_index (arr, i)));
        if (func)
            func (g_ptr_array_index (arr, i), self);
    }

    return result;
}

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

    if (priv->requires)
        g_ptr_array_unref (priv->requires);

    if (priv->extpoints)
        g_ptr_array_unref (priv->extpoints);

    if (priv->extensions)
        g_ptr_array_unref (priv->extensions);

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
    case PROP_IDENTIFIER:
        priv->identifier = g_value_dup_string (value);
        break;

    case PROP_URL:
        priv->url = g_value_dup_string (value);
        break;

    case PROP_KLASS:
        priv->klass = g_value_dup_string (value);
        break;

    case PROP_NAME:
        priv->name = g_value_dup_string (value);
        break;

    case PROP_VERSION:
        priv->version = g_value_dup_string (value);
        break;

    case PROP_PROVIDER:
        priv->provider = g_value_dup_string (value);
        break;

    case PROP_REQUIRES:
        {
            GPtrArray *arr = g_value_get_boxed (value);
            if (arr) {
                priv->requires = _gimo_pluginfo_clone_array (
                    self, arr, GIMO_TYPE_REQUIRE, NULL);
            }
        }
        break;

    case PROP_EXTPOINTS:
        {
            GPtrArray *arr = g_value_get_boxed (value);
            if (arr) {
                priv->extpoints = _gimo_pluginfo_clone_array (
                    self, arr, GIMO_TYPE_EXTPOINT, _gimo_extpoint_setup);
            }
        }
        break;

    case PROP_EXTENSIONS:
        {
            GPtrArray *arr = g_value_get_boxed (value);
            if (arr) {
                priv->extensions = _gimo_pluginfo_clone_array (
                    self, arr, GIMO_TYPE_EXTENSION, NULL);
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
    case PROP_IDENTIFIER:
        g_value_set_string (value, priv->identifier);
        break;

    case PROP_URL:
        g_value_set_string (value, priv->url);
        break;

    case PROP_KLASS:
        g_value_set_string (value, priv->klass);
        break;

    case PROP_NAME:
        g_value_set_string (value, priv->name);
        break;

    case PROP_VERSION:
        g_value_set_string (value, priv->version);
        break;

    case PROP_PROVIDER:
        g_value_set_string (value, priv->provider);
        break;

    case PROP_REQUIRES:
        g_value_set_boxed (value, priv->requires);
        break;

    case PROP_EXTPOINTS:
        g_value_set_boxed (value, priv->extpoints);
        break;

    case PROP_EXTENSIONS:
        g_value_set_boxed (value, priv->extensions);
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

    g_object_class_install_property (
        gobject_class, PROP_IDENTIFIER,
        g_param_spec_string ("identifier",
                             "Unique identifier",
                             "The unique identifier of the plugin",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_URL,
        g_param_spec_string ("url",
                             "URL/path",
                             "The URL/path of the plugin",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_KLASS,
        g_param_spec_string ("klass",
                             "Class name",
                             "The runtime class name of the plugin",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_NAME,
        g_param_spec_string ("name",
                             "Plugin name",
                             "The display name of the plugin",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_VERSION,
        g_param_spec_string ("version",
                             "Plugin version",
                             "The version of the plugin",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_PROVIDER,
        g_param_spec_string ("provider",
                             "Provider name",
                             "The provider of the plugin",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_REQUIRES,
        g_param_spec_boxed ("requires",
                            "Required plugins",
                            "The plugins required by this plugin",
                            G_TYPE_PTR_ARRAY,
                            G_PARAM_READABLE |
                            G_PARAM_WRITABLE |
                            G_PARAM_CONSTRUCT_ONLY |
                            G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_EXTPOINTS,
        g_param_spec_boxed ("extpoints",
                            "Extension points",
                            "The extension points provided by this plugin",
                            G_TYPE_PTR_ARRAY,
                            G_PARAM_READABLE |
                            G_PARAM_WRITABLE |
                            G_PARAM_CONSTRUCT_ONLY |
                            G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_EXTENSIONS,
        g_param_spec_boxed ("extensions",
                            "Extensions",
                            "The extensions provided by this plugin",
                            G_TYPE_PTR_ARRAY,
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
 * @requires: (element-type Gimo.Require) (transfer none):
 *            the required plugins
 * @extpoints: (element-type Gimo.Extpoint) (transfer none):
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
                                 GPtrArray *requires,
                                 GPtrArray *extpoints,
                                 GPtrArray *extensions)
{
    return g_object_new (GIMO_TYPE_PLUGINFO,
                         "identifier", identifier,
                         "url", url,
                         "klass", klass,
                         "name", name,
                         "version", version,
                         "provider", provider,
                         "requires", requires,
                         "extpoints", extpoints,
                         "extensions", extensions,
                         NULL);
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
GPtrArray* gimo_pluginfo_get_requires (GimoPluginfo *self)
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
GPtrArray* gimo_pluginfo_get_extpoints (GimoPluginfo *self)
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
GPtrArray* gimo_pluginfo_get_extensions (GimoPluginfo *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGINFO (self), NULL);

    return self->priv->extensions;
}
