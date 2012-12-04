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

#include "gimo-plugin.h"
#include "gimo-binding.h"
#include "gimo-context.h"
#include "gimo-error.h"
#include "gimo-extension.h"
#include "gimo-extpoint.h"
#include "gimo-loader.h"
#include "gimo-marshal.h"
#include "gimo-module.h"
#include "gimo-plugin.h"
#include "gimo-require.h"
#include "gimo-utils.h"
#include <stdlib.h>
#include <string.h>

extern void _gimo_ext_point_setup (gpointer data, gpointer user_data);
extern void _gimo_ext_point_teardown (gpointer data, gpointer user_data);
extern gint _gimo_ext_point_sort_by_id (gconstpointer a,
                                        gconstpointer b);
extern gint _gimo_ext_point_search_by_id (gconstpointer a,
                                          gconstpointer b);

extern void _gimo_extension_setup (gpointer data, gpointer user_data);
extern void _gimo_extension_teardown (gpointer data, gpointer user_data);
extern gint _gimo_extension_sort_by_id (gconstpointer a,
                                        gconstpointer b);
extern gint _gimo_extension_search_by_id (gconstpointer a,
                                          gconstpointer b);

extern void _gimo_context_plugin_state_changed (GimoContext *self,
                                                GimoPlugin *plugin,
                                                GimoPluginState old_state,
                                                GimoPluginState new_state);

G_DEFINE_TYPE (GimoPlugin, gimo_plugin, GIMO_TYPE_RUNNABLE)

enum {
    SIG_START,
    SIG_STOP,
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_ID,
    PROP_NAME,
    PROP_VERSION,
    PROP_PROVIDER,
    PROP_PATH,
    PROP_MODULE,
    PROP_SYMBOL,
    PROP_REQUIRES,
    PROP_EXTPOINTS,
    PROP_EXTENSIONS
};

struct _GimoPluginPrivate {
    GimoContext *context;
    gchar *id;
    gchar *name;
    gchar *version;
    gchar *provider;
    gchar *path;
    gchar *module;
    gchar *symbol;
    GPtrArray *requires;
    GPtrArray *extpoints;
    GPtrArray *extensions;
    GimoModule *runtime;
    GimoPluginState state;
};

G_LOCK_DEFINE_STATIC (plugin_lock);

static guint plugin_signals[LAST_SIGNAL] = { 0 };

static gboolean _gimo_plugin_load_module (GimoPlugin *self,
                                          GimoContext *context,
                                          GimoLoader *loader)
{
    GimoPluginPrivate *priv = self->priv;
    GimoModule *module = NULL;
    GimoLoadable *loadable = NULL;
    GObject *result;

    if (loader) {
        g_object_ref (loader);
    }
    else {
        loader = gimo_safe_cast (
            gimo_context_resolve_extpoint (
                context, "org.gimo.core.loader.module"),
            GIMO_TYPE_LOADER);

        if (NULL == loader)
            return FALSE;
    }

    if (priv->path && priv->module) {
        gchar *full_path;

        full_path = g_build_filename (priv->path, priv->module, NULL);
        loadable = gimo_loader_load (loader, full_path);
        g_free (full_path);
    }

    if (NULL == loadable)
        loadable = gimo_loader_load (loader, priv->module);

    g_object_unref (loader);

    if (NULL == loadable)
        return FALSE;

    module = GIMO_MODULE (loadable);
    if (NULL == module) {
        g_object_unref (loadable);

        gimo_set_error (GIMO_ERROR_INVALID_TYPE);
        return FALSE;
    }

    G_LOCK (plugin_lock);

    if (priv->runtime) {
        G_UNLOCK (plugin_lock);
        g_object_unref (module);
        return TRUE;
    }

    priv->runtime = module;

    G_UNLOCK (plugin_lock);

    if (priv->symbol) {
        result = gimo_module_resolve (priv->runtime,
                                      priv->symbol,
                                      G_OBJECT (self));
        if (result)
            g_object_unref (result);
        else
            return FALSE;
    }

    return TRUE;
}

static GimoModule* _gimo_plugin_query_module (GimoPlugin *self,
                                              GimoLoader *loader,
                                              gboolean load)
{
    GimoPluginPrivate *priv = self->priv;
    GimoContext *context;
    GimoModule *module;

    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    G_LOCK (plugin_lock);

    if (priv->runtime) {
        module = g_object_ref (priv->runtime);
        G_UNLOCK (plugin_lock);
        return module;
    }

    G_UNLOCK (plugin_lock);

    if (!load)
        return NULL;

    context = gimo_plugin_query_context (self);
    if (NULL == context)
        gimo_set_error_return_val (GIMO_ERROR_NO_OBJECT, NULL);

    if (!_gimo_plugin_load_module (self, context, loader)) {
        g_object_unref (context);

        gimo_set_error_full (GIMO_ERROR_LOAD,
                             "GimoPlugin load module error: %s: %s",
                             priv->module,
                             priv->symbol);
        return NULL;
    }

    G_LOCK (plugin_lock);

    if (!priv->runtime) {
        G_UNLOCK (plugin_lock);
        g_object_unref (context);
        return NULL;
    }

    module = g_object_ref (priv->runtime);
    priv->state = GIMO_PLUGIN_RESOLVED;

    G_UNLOCK (plugin_lock);

    if (priv->requires) {
        GimoRequire *r;
        GimoPlugin *p;
        GimoModule *m;
        const gchar *id;
        guint i;

        for (i = 0; i < priv->requires->len; ++i) {
            r = g_ptr_array_index (priv->requires, i);
            id = gimo_require_get_plugin_id (r);
            p = gimo_context_query_plugin (context, id);

            if (NULL == p)
                break;

            m = _gimo_plugin_query_module (p, loader, load);
            g_object_unref (p);

            if (NULL == m)
                break;

            g_object_unref (m);
        }

        if (i < priv->requires->len) {
            G_LOCK (plugin_lock);

            g_object_unref (module);
            module = NULL;

            g_object_unref (priv->runtime);
            priv->runtime = NULL;

            priv->state = GIMO_PLUGIN_INSTALLED;

            G_UNLOCK (plugin_lock);
        }
    }

    if (module) {
        _gimo_context_plugin_state_changed (context,
                                            self,
                                            GIMO_PLUGIN_INSTALLED,
                                            GIMO_PLUGIN_RESOLVED);
    }

    g_object_unref (context);

    return module;
}

static gboolean _gimo_plugin_emit_signal (GimoPlugin *self,
                                          guint signal_id)
{
    GValue param = { 0, };
    GValue return_val = { 0, };
    gboolean result;

    g_value_init (&param, G_TYPE_OBJECT);
    g_value_init (&return_val, G_TYPE_BOOLEAN);

    g_value_set_object (&param, self);
    g_value_set_boolean (&return_val, TRUE);
    g_signal_emitv (&param, signal_id, 0, &return_val);

    result = g_value_get_boolean (&return_val);

    g_value_unset (&param);
    g_value_unset (&return_val);
    return result;
}

static void gimo_plugin_init (GimoPlugin *self)
{
    GimoPluginPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_PLUGIN,
                                              GimoPluginPrivate);
    priv = self->priv;

    priv->context = NULL;
    priv->id = NULL;
    priv->name = NULL;
    priv->version = NULL;
    priv->provider = NULL;
    priv->path = NULL;
    priv->module = NULL;
    priv->symbol = NULL;
    priv->requires = NULL;
    priv->extpoints = NULL;
    priv->extensions = NULL;
    priv->runtime = NULL;
    priv->state = GIMO_PLUGIN_UNINSTALLED;
}

static void gimo_plugin_finalize (GObject *gobject)
{
    GimoPlugin *self = GIMO_PLUGIN (gobject);
    GimoPluginPrivate *priv = self->priv;

    g_assert (!priv->context && !priv->runtime);

    if (priv->requires)
        g_ptr_array_unref (priv->requires);

    if (priv->extpoints) {
        g_ptr_array_foreach (priv->extpoints,
                             _gimo_ext_point_teardown,
                             self);
        g_ptr_array_unref (priv->extpoints);
    }

    if (priv->extensions) {
        g_ptr_array_foreach (priv->extensions,
                             _gimo_extension_teardown,
                             self);
        g_ptr_array_unref (priv->extensions);
    }

    g_free (priv->id);
    g_free (priv->name);
    g_free (priv->version);
    g_free (priv->provider);
    g_free (priv->path);
    g_free (priv->module);
    g_free (priv->symbol);

    G_OBJECT_CLASS (gimo_plugin_parent_class)->finalize (gobject);
}

static void gimo_plugin_set_property (GObject *object,
                                      guint prop_id,
                                      const GValue *value,
                                      GParamSpec *pspec)
{
    GimoPlugin *self = GIMO_PLUGIN (object);
    GimoPluginPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_ID:
        priv->id = g_value_dup_string (value);
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

    case PROP_PATH:
        priv->path = g_value_dup_string (value);
        break;

    case PROP_MODULE:
        priv->module = g_value_dup_string (value);
        break;

    case PROP_SYMBOL:
        priv->symbol = g_value_dup_string (value);
        break;

    case PROP_REQUIRES:
        {
            GPtrArray *arr = g_value_get_boxed (value);
            if (arr) {
                priv->requires = _gimo_clone_object_array (
                    arr, GIMO_TYPE_REQUIRE, NULL, NULL);
            }
        }
        break;

    case PROP_EXTPOINTS:
        {
            GPtrArray *arr = g_value_get_boxed (value);
            if (arr) {
                priv->extpoints = _gimo_clone_object_array (
                    arr, GIMO_TYPE_EXTPOINT, _gimo_ext_point_setup, self);

                g_ptr_array_sort (priv->extpoints,
                                  _gimo_ext_point_sort_by_id);
            }
        }
        break;

    case PROP_EXTENSIONS:
        {
            GPtrArray *arr = g_value_get_boxed (value);
            if (arr) {
                priv->extensions = _gimo_clone_object_array (
                    arr, GIMO_TYPE_EXTENSION, _gimo_extension_setup, self);

                g_ptr_array_sort (priv->extensions,
                                  _gimo_extension_sort_by_id);
            }
        }
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_plugin_get_property (GObject *object,
                                      guint prop_id,
                                      GValue *value,
                                      GParamSpec *pspec)
{
    GimoPlugin *self = GIMO_PLUGIN (object);
    GimoPluginPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_ID:
        g_value_set_string (value, priv->id);
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

    case PROP_PATH:
        g_value_set_string (value, priv->path);
        break;

    case PROP_MODULE:
        g_value_set_string (value, priv->module);
        break;

    case PROP_SYMBOL:
        g_value_set_string (value, priv->symbol);
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

static void gimo_plugin_class_init (GimoPluginClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (GimoPluginPrivate));

    gobject_class->finalize = gimo_plugin_finalize;
    gobject_class->set_property = gimo_plugin_set_property;
    gobject_class->get_property = gimo_plugin_get_property;

    klass->start = NULL;
    klass->stop = NULL;

    plugin_signals[SIG_START] =
            g_signal_new ("start",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (GimoPluginClass, start),
                          NULL, NULL,
                          _gimo_marshal_BOOLEAN__VOID,
                          G_TYPE_BOOLEAN, 0);

    plugin_signals[SIG_STOP] =
            g_signal_new ("stop",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (GimoPluginClass, stop),
                          NULL, NULL,
                          g_cclosure_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    g_object_class_install_property (
        gobject_class, PROP_ID,
        g_param_spec_string ("id",
                             "Unique Identifier",
                             "The unique identifier of the plugin",
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
                             "Plugin Version",
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
        gobject_class, PROP_PATH,
        g_param_spec_string ("path",
                             "Module Path",
                             "The runtime module path",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_MODULE,
        g_param_spec_string ("module",
                             "Module Name",
                             "The runtime module name",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_SYMBOL,
        g_param_spec_string ("symbol",
                             "Runtime symbol name",
                             "The runtime symbol name",
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
                            GIMO_TYPE_OBJECT_ARRAY,
                            G_PARAM_READABLE |
                            G_PARAM_WRITABLE |
                            G_PARAM_CONSTRUCT_ONLY |
                            G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_EXTPOINTS,
        g_param_spec_boxed ("extpoints",
                            "Extension points",
                            "The extension points provided by this plugin",
                            GIMO_TYPE_OBJECT_ARRAY,
                            G_PARAM_READABLE |
                            G_PARAM_WRITABLE |
                            G_PARAM_CONSTRUCT_ONLY |
                            G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_EXTENSIONS,
        g_param_spec_boxed ("extensions",
                            "Extensions",
                            "The extensions provided by this plugin",
                            GIMO_TYPE_OBJECT_ARRAY,
                            G_PARAM_READABLE |
                            G_PARAM_WRITABLE |
                            G_PARAM_CONSTRUCT_ONLY |
                            G_PARAM_STATIC_STRINGS));
}

/**
 * gimo_plugin_new:
 * @id: the unique identifier
 * @name: (allow-none): the display name
 * @version: (allow-none): the release version
 * @provider: (allow-none): the provider name
 * @path: (allow-none): the runtime module path
 * @module: (allow-none): the runtime module name
 * @symbol: (allow-none): the runtime symbol name
 * @requires: (allow-none) (element-type Gimo.Require) (transfer none):
 *            the required plugins
 * @extpoints: (allow-none) (element-type Gimo.ExtPoint) (transfer none):
 *             the extension points
 * @extensions: (allow-none) (element-type Gimo.Extension) (transfer none):
 *              the extensions
 *
 * Create a plugin descriptor with the provided parameters.
 */
GimoPlugin* gimo_plugin_new (const gchar *id,
                             const gchar *name,
                             const gchar *version,
                             const gchar *provider,
                             const gchar *path,
                             const gchar *module,
                             const gchar *symbol,
                             GPtrArray *requires,
                             GPtrArray *extpoints,
                             GPtrArray *extensions)
{
    return g_object_new (GIMO_TYPE_PLUGIN,
                         "id", id,
                         "name", name,
                         "version", version,
                         "provider", provider,
                         "path", path,
                         "module", module,
                         "symbol", symbol,
                         "requires", requires,
                         "extpoints", extpoints,
                         "extensions", extensions,
                         NULL);
}

const gchar* gimo_plugin_get_id (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    return self->priv->id;
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

const gchar* gimo_plugin_get_path (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    return self->priv->path;
}

const gchar* gimo_plugin_get_module (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    return self->priv->module;
}

const gchar* gimo_plugin_get_symbol (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    return self->priv->symbol;
}

/**
 * gimo_plugin_get_extpoint:
 * @self: a #GimoPlugin
 * @local_id: the local extension point ID
 *
 * Get an extension point with the specified local ID.
 *
 * Returns: (allow-none) (transfer none): a #GimoExtPoint
 */
GimoExtPoint* gimo_plugin_get_extpoint (GimoPlugin *self,
                                        const gchar *local_id)
{
    GimoPluginPrivate *priv;
    GimoExtPoint **result;

    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    priv = self->priv;

    if (NULL == priv->extpoints)
        return NULL;

    result = bsearch (local_id,
                      priv->extpoints->pdata,
                      priv->extpoints->len,
                      sizeof (gpointer),
                      _gimo_ext_point_search_by_id);

    return result ? *result : NULL;
}

/**
 * gimo_plugin_get_extension:
 * @self: a #GimoPlugin
 * @local_id: the local extension ID
 *
 * Get an extension with the specified local ID.
 *
 * Returns: (allow-none) (transfer none): a #GimoExtension
 */
GimoExtension* gimo_plugin_get_extension (GimoPlugin *self,
                                          const gchar *local_id)
{
    GimoPluginPrivate *priv;
    GimoExtension **result;

    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    priv = self->priv;

    if (NULL == priv->extensions)
        return NULL;

    result = bsearch (local_id,
                      priv->extensions->pdata,
                      priv->extensions->len,
                      sizeof (gpointer),
                      _gimo_extension_search_by_id);

    return result ? *result : NULL;
}

/**
 * gimo_plugin_get_requires:
 * @self: a #GimoPlugin
 *
 * Get the required plugins of the plugin.
 *
 * Returns: (element-type Gimo.Require) (transfer none):
 *          the required plugins list.
 */
GPtrArray* gimo_plugin_get_requires (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    return self->priv->requires;
}

/**
 * gimo_plugin_get_extpoints:
 * @self: a #GimoPlugin
 *
 * Get the extension points provided by the plugin.
 *
 * Returns: (element-type Gimo.ExtPoint) (transfer none):
 *          the extension points list.
 */
GPtrArray* gimo_plugin_get_extpoints (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    return self->priv->extpoints;
}

/**
 * gimo_plugin_get_extensions:
 * @self: a #GimoPlugin
 *
 * Get the extensions provided by the plugin.
 *
 * Returns: (element-type Gimo.Extension) (transfer none):
 *          the extensions list.
 */
GPtrArray* gimo_plugin_get_extensions (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    return self->priv->extensions;
}

/**
 * gimo_plugin_query_extensions:
 * @self: a #GimoPlugin
 * @extpt_id: the extension point ID
 *
 * Query the specified extensions in this plugin.
 *
 * Returns: (element-type Gimo.Extension) (transfer container):
 *          An #GPtrArray of extensions if successful, %NULL on error.
 *          Free the returned array with g_ptr_array_unref().
 */
GPtrArray* gimo_plugin_query_extensions (GimoPlugin *self,
                                         const gchar *extpt_id)
{
    GimoPluginPrivate *priv;
    GimoExtension *ext;
    GPtrArray *result = NULL;
    guint i;

    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    priv = self->priv;

    if (NULL == priv->extensions)
        return NULL;

    for (i = 0; i < priv->extensions->len; ++i) {
        ext = g_ptr_array_index (priv->extensions, i);

        if (!strcmp (gimo_extension_get_extpoint_id (ext), extpt_id)) {
            if (NULL == result)
                result = g_ptr_array_new_with_free_func (g_object_unref);

            g_ptr_array_add (result, g_object_ref (ext));
        }
    }

    return result;
}

/**
 * gimo_plugin_query_context:
 * @self: a #GimoPlugin
 *
 * Query the context of the plugin descriptor.
 *
 * Returns: (allow-none) (transfer full): a #GimoContext
 */
GimoContext* gimo_plugin_query_context (GimoPlugin *self)
{
    GimoPluginPrivate *priv;
    GimoContext *ctx = NULL;

    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    priv = self->priv;

    G_LOCK (plugin_lock);

    if (priv->context)
        ctx = g_object_ref (priv->context);

    G_UNLOCK (plugin_lock);

    return ctx;
}

GimoPluginState gimo_plugin_get_state (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), GIMO_PLUGIN_UNINSTALLED);

    return self->priv->state;
}

gboolean gimo_plugin_define (GimoPlugin *self,
                             const gchar *symbol,
                             GObject *object)
{
    if (gimo_lookup_object (G_OBJECT (self), symbol))
        return FALSE;

    gimo_bind_object (G_OBJECT (self), symbol, object);
    return TRUE;
}

/**
 * gimo_plugin_resolve:
 * @self: a #GimoPlugin
 * @symbol: the symbol name
 *
 * Resolve the plugin runtime information.
 *
 * Returns: (allow-none) (transfer full): a #GObject
 */
GObject* gimo_plugin_resolve (GimoPlugin *self,
                              const gchar *symbol)
{
    GimoModule *module;
    GObject *object = NULL;

    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    object = gimo_query_object (G_OBJECT (self), symbol);
    if (object)
        return object;

    module = _gimo_plugin_query_module (self, NULL, TRUE);
    if (NULL == module)
        return NULL;

    object = gimo_module_resolve (module,
                                  symbol,
                                  G_OBJECT (self));
    g_object_unref (module);

    return object;
}

gboolean gimo_plugin_start (GimoPlugin *self,
                            GimoLoader *loader)
{
    GimoPluginPrivate *priv;
    GimoModule *module;
    gboolean result;

    g_return_val_if_fail (GIMO_IS_PLUGIN (self), FALSE);

    priv = self->priv;

    if (GIMO_PLUGIN_ACTIVE == priv->state)
        return TRUE;

    module = _gimo_plugin_query_module (self, loader, TRUE);
    if (NULL == module)
        return FALSE;

    result = _gimo_plugin_emit_signal (self,
                                       plugin_signals [SIG_START]);

    g_object_unref (module);

    if (result)
        priv->state = GIMO_PLUGIN_ACTIVE;

    return result;
}

void gimo_plugin_stop (GimoPlugin *self)
{
    GimoPluginPrivate *priv;
    GimoModule *module;

    g_return_if_fail (GIMO_IS_PLUGIN (self));

    priv = self->priv;

    if (priv->state != GIMO_PLUGIN_ACTIVE &&
        priv->state != GIMO_PLUGIN_STARTING)
    {
        return;
    }

    module = _gimo_plugin_query_module (self, NULL, FALSE);
    if (NULL == module)
        return;

    g_signal_emit (self,
                   plugin_signals[SIG_STOP],
                   0);

    g_object_unref (module);

    priv->state = GIMO_PLUGIN_RESOLVED;
}

void _gimo_plugin_install (GimoPlugin *self,
                           GimoContext *context,
                           const gchar *cur_path)
{
    GimoPluginPrivate *priv = self->priv;

    g_assert (NULL == priv->context);

    G_LOCK (plugin_lock);

    priv->context = context;
    priv->state = GIMO_PLUGIN_INSTALLED;

    if (cur_path) {
        gchar *full_path;

        if (priv->path) {
            full_path = g_build_path (G_DIR_SEPARATOR_S,
                                      cur_path,
                                      priv->path,
                                      NULL);
            g_free (priv->path);
        }
        else {
            full_path = g_strdup (cur_path);
        }

        priv->path = full_path;
    }

    G_UNLOCK (plugin_lock);
}

void _gimo_plugin_uninstall (GimoPlugin *self)
{
    GimoPluginPrivate *priv = self->priv;

    G_LOCK (plugin_lock);

    if (priv->runtime) {
        g_object_unref (priv->runtime);
        priv->runtime = NULL;
    }

    priv->context = NULL;
    priv->state = GIMO_PLUGIN_UNINSTALLED;

    G_UNLOCK (plugin_lock);
}
