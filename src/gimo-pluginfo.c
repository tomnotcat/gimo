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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * MT safe
 */

#include "gimo-pluginfo.h"
#include "gimo-context.h"
#include "gimo-error.h"
#include "gimo-extension.h"
#include "gimo-extpoint.h"
#include "gimo-plugin.h"
#include "gimo-require.h"
#include "gimo-utils.h"
#include <stdlib.h>
#include <string.h>

extern void _gimo_extpoint_setup (gpointer data, gpointer user_data);
extern void _gimo_extpoint_teardown (gpointer data, gpointer user_data);
extern void _gimo_extension_setup (gpointer data, gpointer user_data);
extern void _gimo_extension_teardown (gpointer data, gpointer user_data);
extern GimoPlugin* _gimo_context_load_plugin (GimoContext *self,
                                              GimoPluginfo *info);
extern void _gimo_context_plugin_state_changed (GimoContext *self,
                                                GimoPluginfo *info,
                                                GimoPluginState old_state,
                                                GimoPluginState new_state);

G_DEFINE_TYPE (GimoPluginfo, gimo_pluginfo, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_IDENTIFIER,
    PROP_URL,
    PROP_SYMBOL,
    PROP_NAME,
    PROP_VERSION,
    PROP_PROVIDER,
    PROP_REQUIRES,
    PROP_EXTPOINTS,
    PROP_EXTENSIONS
};

struct _GimoPluginfoPrivate {
    GimoContext *context;
    gchar *identifier;
    gchar *url;
    gchar *symbol;
    gchar *name;
    gchar *version;
    gchar *provider;
    GPtrArray *requires;
    GPtrArray *extpoints;
    GPtrArray *extensions;
    GimoPlugin *plugin;
    GimoPluginState state;
};

G_LOCK_DEFINE_STATIC (pluginfo_lock);

static gint _gimo_pluginfo_extpoint_sort (gconstpointer a,
                                          gconstpointer b)
{
    return strcmp (gimo_extpoint_get_local_id (*(GimoExtpoint **) a),
                   gimo_extpoint_get_local_id (*(GimoExtpoint **) b));
}

static gint _gimo_pluginfo_extpoint_search (gconstpointer a,
                                            gconstpointer b)
{
    return strcmp (a, gimo_extpoint_get_local_id (*(GimoExtpoint **) b));
}

static gint _gimo_pluginfo_extension_sort (gconstpointer a,
                                           gconstpointer b)
{
    return strcmp (gimo_extension_get_local_id (*(GimoExtension **) a),
                   gimo_extension_get_local_id (*(GimoExtension **) b));
}

static gint _gimo_pluginfo_extension_search (gconstpointer a,
                                             gconstpointer b)
{
    return strcmp (a, gimo_extension_get_local_id (*(GimoExtension **) b));
}

static GPtrArray* _gimo_pluginfo_clone_array (GimoPluginfo *self,
                                              GPtrArray *arr,
                                              GType type,
                                              void (*func) (gpointer, gpointer))
{
    GPtrArray *result;
    GObject *object;
    guint i;

    if (NULL == arr)
        return NULL;

    result = g_ptr_array_new_full (arr->len, g_object_unref);
    for (i = 0; i < arr->len; ++i) {
        object = g_ptr_array_index (arr, i);

        g_assert (G_OBJECT_TYPE (object) == type);
        g_ptr_array_add (result, g_object_ref (object));

        if (func)
            func (object, self);
    }

    return result;
}

static GimoPlugin* _gimo_pluginfo_load_plugin (GimoPluginfo *self)
{
    GimoPluginfoPrivate *priv = self->priv;
    GimoContext *ctx;
    GimoPlugin *plugin;

    G_LOCK (pluginfo_lock);

    if (priv->plugin)
        return g_object_ref (priv->plugin);

    G_UNLOCK (pluginfo_lock);

    ctx = gimo_pluginfo_query_context (self);
    if (NULL == ctx)
        gimo_set_error_return_val (GIMO_ERROR_NOT_FOUND, NULL);

    plugin = _gimo_context_load_plugin (ctx, self);
    if (NULL == plugin)
        gimo_set_error_return_val (GIMO_ERROR_IMPORT, NULL);

    G_LOCK (pluginfo_lock);

    if (priv->plugin) {
        G_UNLOCK (pluginfo_lock);
        g_object_unref (plugin);
        return priv->plugin;
    }

    priv->plugin = g_object_ref (plugin);
    priv->state = GIMO_PLUGIN_RESOLVED;

    G_UNLOCK (pluginfo_lock);

    if (priv->requires) {
        GimoRequire *r;
        GimoPlugin *p;
        GimoPluginfo *pi;
        const gchar *id;
        guint i;

        for (i = 0; i < priv->requires->len; ++i) {
            r = g_ptr_array_index (priv->requires, i);
            id = gimo_require_get_plugin_id (r);
            pi = gimo_context_query_plugin (ctx, id);

            if (NULL == pi)
                break;

            p = _gimo_pluginfo_load_plugin (pi);
            g_object_unref (pi);

            if (NULL == p)
                break;

            g_object_unref (p);
        }

        if (i < priv->requires->len) {
            G_LOCK (pluginfo_lock);

            g_object_unref (plugin);
            plugin = NULL;

            g_object_unref (priv->plugin);
            priv->plugin = NULL;

            priv->state = GIMO_PLUGIN_INSTALLED;

            G_UNLOCK (pluginfo_lock);
        }
    }

    if (plugin) {
        _gimo_context_plugin_state_changed (ctx, self,
                                            GIMO_PLUGIN_INSTALLED,
                                            GIMO_PLUGIN_RESOLVED);
    }

    g_object_unref (ctx);

    return plugin;
}

static void gimo_pluginfo_init (GimoPluginfo *self)
{
    GimoPluginfoPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_PLUGINFO,
                                              GimoPluginfoPrivate);
    priv = self->priv;

    priv->context = NULL;
    priv->identifier = NULL;
    priv->url = NULL;
    priv->symbol = NULL;
    priv->name = NULL;
    priv->version = NULL;
    priv->provider = NULL;
    priv->requires = NULL;
    priv->extpoints = NULL;
    priv->extensions = NULL;
    priv->plugin = NULL;
    priv->state = GIMO_PLUGIN_UNINSTALLED;
}

static void gimo_pluginfo_finalize (GObject *gobject)
{
    GimoPluginfo *self = GIMO_PLUGINFO (gobject);
    GimoPluginfoPrivate *priv = self->priv;

    g_assert (!priv->context && !priv->plugin);

    g_free (priv->identifier);
    g_free (priv->url);
    g_free (priv->symbol);
    g_free (priv->name);
    g_free (priv->version);
    g_free (priv->provider);

    if (priv->requires)
        g_ptr_array_unref (priv->requires);

    if (priv->extpoints) {
        g_ptr_array_foreach (priv->extpoints,
                             _gimo_extpoint_teardown,
                             self);
        g_ptr_array_unref (priv->extpoints);
    }

    if (priv->extensions) {
        g_ptr_array_foreach (priv->extensions,
                             _gimo_extension_teardown,
                             self);
        g_ptr_array_unref (priv->extensions);
    }

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

    case PROP_SYMBOL:
        priv->symbol = g_value_dup_string (value);
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

                g_ptr_array_sort (priv->extpoints,
                                  _gimo_pluginfo_extpoint_sort);
            }
        }
        break;

    case PROP_EXTENSIONS:
        {
            GPtrArray *arr = g_value_get_boxed (value);
            if (arr) {
                priv->extensions = _gimo_pluginfo_clone_array (
                    self, arr, GIMO_TYPE_EXTENSION, _gimo_extension_setup);

                g_ptr_array_sort (priv->extensions,
                                  _gimo_pluginfo_extension_sort);
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

    case PROP_SYMBOL:
        g_value_set_string (value, priv->symbol);
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
        gobject_class, PROP_SYMBOL,
        g_param_spec_string ("symbol",
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
 * @url: (allow-none): url/path of the plugin
 * @symbol: (allow-none): the runtime symbol name
 * @name: (allow-none): the display name
 * @version: (allow-none): the release version
 * @provider: (allow-none): the provider name
 * @requires: (allow-none) (element-type Gimo.Require) (transfer none):
 *            the required plugins
 * @extpoints: (allow-none) (element-type Gimo.Extpoint) (transfer none):
 *             the extension points
 * @extensions: (allow-none) (element-type Gimo.Extension) (transfer none):
 *              the extensions
 *
 * Create a plugin descriptor of the provided parameters.
 */
GimoPluginfo* gimo_pluginfo_new (const gchar *identifier,
                                 const gchar *url,
                                 const gchar *symbol,
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
                         "symbol", symbol,
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

const gchar* gimo_pluginfo_get_symbol (GimoPluginfo *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGINFO (self), NULL);

    return self->priv->symbol;
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
 * gimo_pluginfo_get_extpoint:
 * @self: a #GimoPluginfo
 * @local_id: the local extension point ID
 *
 * Get an extension point with the specified local ID.
 *
 * Returns: (allow-none) (transfer none): a #GimoExtpoint
 */
GimoExtpoint* gimo_pluginfo_get_extpoint (GimoPluginfo *self,
                                          const gchar *local_id)
{
    GimoPluginfoPrivate *priv;
    GimoExtpoint **result;

    g_return_val_if_fail (GIMO_IS_PLUGINFO (self), NULL);

    priv = self->priv;

    if (NULL == priv->extpoints)
        return NULL;

    result = bsearch (local_id,
                      priv->extpoints->pdata,
                      priv->extpoints->len,
                      sizeof (gpointer),
                      _gimo_pluginfo_extpoint_search);

    return result ? *result : NULL;
}

/**
 * gimo_pluginfo_get_extension:
 * @self: a #GimoPluginfo
 * @local_id: the local extension ID
 *
 * Get an extension with the specified local ID.
 *
 * Returns: (allow-none) (transfer none): a #GimoExtension
 */
GimoExtension* gimo_pluginfo_get_extension (GimoPluginfo *self,
                                            const gchar *local_id)
{
    GimoPluginfoPrivate *priv;
    GimoExtension **result;

    g_return_val_if_fail (GIMO_IS_PLUGINFO (self), NULL);

    priv = self->priv;

    if (NULL == priv->extensions)
        return NULL;

    result = bsearch (local_id,
                      priv->extensions->pdata,
                      priv->extensions->len,
                      sizeof (gpointer),
                      _gimo_pluginfo_extension_search);

    return result ? *result : NULL;
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

GimoPluginState gimo_pluginfo_get_state (GimoPluginfo *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGINFO (self), GIMO_PLUGIN_UNINSTALLED);

    return self->priv->state;
}

/**
 * gimo_pluginfo_query_context:
 * @self: a #GimoPluginfo
 *
 * Query the context of the plugin descriptor.
 *
 * Returns: (allow-none) (transfer full): a #GimoContext
 */
GimoContext* gimo_pluginfo_query_context (GimoPluginfo *self)
{
    GimoPluginfoPrivate *priv;
    GimoContext *ctx = NULL;

    g_return_val_if_fail (GIMO_IS_PLUGINFO (self), NULL);

    priv = self->priv;

    G_LOCK (pluginfo_lock);

    if (priv->context)
        ctx = g_object_ref (priv->context);

    G_UNLOCK (pluginfo_lock);

    return ctx;
}

gboolean gimo_pluginfo_start (GimoPlugin *self)
{
    return FALSE;
}

gboolean gimo_pluginfo_stop (GimoPlugin *self)
{
    return FALSE;
}

/**
 * gimo_pluginfo_resolve:
 * @self: a #GimoPluginfo
 * @symbol: the symbol name
 *
 * Resolve the plugin runtime information.
 *
 * Returns: (allow-none) (transfer full): a #GObject
 */
GObject* gimo_pluginfo_resolve (GimoPluginfo *self,
                                const gchar *symbol)
{
    GimoPlugin *plugin;

    g_return_val_if_fail (GIMO_IS_PLUGINFO (self), NULL);

    plugin = _gimo_pluginfo_load_plugin (self);
    if (NULL == plugin)
        return NULL;

    g_object_unref (plugin);
    return NULL;
}

void _gimo_pluginfo_install (GimoPluginfo *self,
                             GimoContext *context)
{
    GimoPluginfoPrivate *priv = self->priv;

    g_assert (NULL == priv->context);

    G_LOCK (pluginfo_lock);
    priv->context = context;
    priv->state = GIMO_PLUGIN_INSTALLED;
    G_UNLOCK (pluginfo_lock);
}

void _gimo_pluginfo_uninstall (GimoPluginfo *self)
{
    GimoPluginfoPrivate *priv = self->priv;

    G_LOCK (pluginfo_lock);
    priv->context = NULL;
    priv->state = GIMO_PLUGIN_UNINSTALLED;
    G_UNLOCK (pluginfo_lock);
}
