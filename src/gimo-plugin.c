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
#include "gimo-context.h"
#include "gimo-error.h"
#include "gimo-extension.h"
#include "gimo-extpoint.h"
#include "gimo-loader.h"
#include "gimo-module.h"
#include "gimo-plugin.h"
#include "gimo-require.h"
#include "gimo-runtime.h"
#include "gimo-utils.h"
#include <stdlib.h>

extern void _gimo_extpoint_setup (gpointer data, gpointer user_data);
extern void _gimo_extpoint_teardown (gpointer data, gpointer user_data);
extern gint _gimo_extpoint_sort_by_id (gconstpointer a,
                                       gconstpointer b);
extern gint _gimo_extpoint_search_by_id (gconstpointer a,
                                         gconstpointer b);

extern void _gimo_extension_setup (gpointer data, gpointer user_data);
extern void _gimo_extension_teardown (gpointer data, gpointer user_data);
extern gint _gimo_extension_sort_by_id (gconstpointer a,
                                        gconstpointer b);
extern gint _gimo_extension_search_by_id (gconstpointer a,
                                          gconstpointer b);

extern GimoRuntime* _gimo_context_load_runtime (GimoContext *self,
                                                GimoPlugin *plugin);
extern void _gimo_context_plugin_state_changed (GimoContext *self,
                                                GimoPlugin *plugin,
                                                GimoPluginState old_state,
                                                GimoPluginState new_state);

G_DEFINE_TYPE (GimoPlugin, gimo_plugin, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_ID,
    PROP_NAME,
    PROP_VERSION,
    PROP_PROVIDER,
    PROP_MODULE,
    PROP_SYMBOL,
    PROP_REQUIRES,
    PROP_EXTPOINTS,
    PROP_EXTENSIONS,
    PROP_RUNTIME
};

struct _GimoPluginPrivate {
    GimoContext *context;
    gchar *id;
    gchar *name;
    gchar *version;
    gchar *provider;
    gchar *module;
    gchar *symbol;
    GPtrArray *requires;
    GPtrArray *extpoints;
    GPtrArray *extensions;
    GimoModule *runtime_module;
    GimoRuntime *runtime;
    GimoPluginState state;
};

G_LOCK_DEFINE_STATIC (plugin_lock);

static GimoRuntime* _gimo_plugin_load_runtime (GimoPlugin *self,
                                               GimoContext *context)
{
    GimoPluginPrivate *priv = self->priv;
    const gchar *symbol;

    if (NULL == priv->runtime_module) {
        GimoLoader *loader;
        GimoModule *module;

        loader = gimo_context_resolve_extpoint (context,
                                                "org.gimo.core.loader.module",
                                                GIMO_TYPE_LOADER);
        if (NULL == loader)
            return NULL;

        module = gimo_safe_cast (gimo_loader_load (loader, priv->module),
                                 GIMO_TYPE_MODULE);
        if (NULL == module) {
            g_object_unref (loader);
            return NULL;
        }

        G_LOCK (plugin_lock);

        if (NULL == priv->runtime_module)
            priv->runtime_module = module;
        else
            g_object_unref (module);

        G_UNLOCK (plugin_lock);

        g_object_unref (loader);
    }

    symbol = priv->symbol;

    if (NULL == symbol)
        symbol = GIMO_RUNTIME_SYMBOL_NAME;

    return gimo_safe_cast (gimo_module_resolve (priv->runtime_module,
                                                symbol,
                                                G_OBJECT (self)),
                           GIMO_TYPE_RUNTIME);
}

static GimoRuntime* _gimo_plugin_query_runtime (GimoPlugin *self)
{
    GimoPluginPrivate *priv = self->priv;
    GimoContext *context;
    GimoRuntime *runtime;

    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    G_LOCK (plugin_lock);

    if (priv->runtime) {
        runtime = g_object_ref (priv->runtime);
        G_UNLOCK (plugin_lock);
        return runtime;
    }

    G_UNLOCK (plugin_lock);

    context = gimo_plugin_query_context (self);
    if (NULL == context)
        gimo_set_error_return_val (GIMO_ERROR_NO_OBJECT, NULL);

    runtime = _gimo_plugin_load_runtime (self, context);
    g_assert (runtime);
    if (NULL == runtime)
        gimo_set_error_return_val (GIMO_ERROR_LOAD, NULL);

    G_LOCK (plugin_lock);

    if (priv->runtime) {
        g_object_unref (runtime);
        runtime = g_object_ref (priv->runtime);
        G_UNLOCK (plugin_lock);
        g_object_unref (context);
        return runtime;
    }

    priv->runtime = g_object_ref (runtime);
    priv->state = GIMO_PLUGIN_RESOLVED;

    G_UNLOCK (plugin_lock);

    if (priv->requires) {
        GimoRequire *r;
        GimoPlugin *p;
        GimoRuntime *rt;
        const gchar *id;
        guint i;

        for (i = 0; i < priv->requires->len; ++i) {
            r = g_ptr_array_index (priv->requires, i);
            id = gimo_require_get_plugin_id (r);
            p = gimo_context_query_plugin (context, id);

            if (NULL == p)
                break;

            rt = _gimo_plugin_query_runtime (p);
            g_object_unref (p);

            if (NULL == rt)
                break;

            g_object_unref (rt);
        }

        if (i < priv->requires->len) {
            G_LOCK (plugin_lock);

            g_object_unref (runtime);
            runtime = NULL;

            g_object_unref (priv->runtime);
            priv->runtime = NULL;

            priv->state = GIMO_PLUGIN_INSTALLED;

            G_UNLOCK (plugin_lock);
        }
    }

    if (runtime) {
        _gimo_context_plugin_state_changed (context,
                                            self,
                                            GIMO_PLUGIN_INSTALLED,
                                            GIMO_PLUGIN_RESOLVED);
    }

    g_object_unref (context);

    return runtime;
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
    priv->module = NULL;
    priv->symbol = NULL;
    priv->requires = NULL;
    priv->extpoints = NULL;
    priv->extensions = NULL;
    priv->runtime_module = NULL;
    priv->runtime = NULL;
    priv->state = GIMO_PLUGIN_UNINSTALLED;
}

static void gimo_plugin_finalize (GObject *gobject)
{
    GimoPlugin *self = GIMO_PLUGIN (gobject);
    GimoPluginPrivate *priv = self->priv;

    g_assert (!priv->context);

    g_free (priv->id);
    g_free (priv->name);
    g_free (priv->version);
    g_free (priv->provider);
    g_free (priv->module);
    g_free (priv->symbol);

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

    if (priv->runtime)
        g_object_unref (priv->runtime);

    if (priv->runtime_module)
        g_object_unref (priv->runtime_module);

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
                    arr, GIMO_TYPE_EXTPOINT, _gimo_extpoint_setup, self);

                g_ptr_array_sort (priv->extpoints,
                                  _gimo_extpoint_sort_by_id);
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

    case PROP_RUNTIME:
        priv->runtime = g_value_dup_object (value);
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

    case PROP_RUNTIME:
        g_value_set_object (value, priv->runtime);
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

    g_object_class_install_property (
        gobject_class, PROP_ID,
        g_param_spec_string ("id",
                             "Unique identifier",
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
        gobject_class, PROP_MODULE,
        g_param_spec_string ("module",
                             "Module name",
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

    g_object_class_install_property (
        gobject_class, PROP_RUNTIME,
        g_param_spec_object ("runtime",
                             "Runtime object",
                             "The runtime object of this plugin",
                             GIMO_TYPE_RUNTIME,
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
 * @module: (allow-none): the runtime module name
 * @symbol: (allow-none): the runtime symbol name
 * @requires: (allow-none) (element-type Gimo.Require) (transfer none):
 *            the required plugins
 * @extpoints: (allow-none) (element-type Gimo.ExtPoint) (transfer none):
 *             the extension points
 * @extensions: (allow-none) (element-type Gimo.Extension) (transfer none):
 *              the extensions
 *
 * Create a plugin descriptor of the provided parameters.
 */
GimoPlugin* gimo_plugin_new (const gchar *id,
                             const gchar *name,
                             const gchar *version,
                             const gchar *provider,
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
                      _gimo_extpoint_search_by_id);

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

GimoPluginState gimo_plugin_get_state (GimoPlugin *self)
{
    g_return_val_if_fail (GIMO_IS_PLUGIN (self), GIMO_PLUGIN_UNINSTALLED);

    return self->priv->state;
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

gboolean gimo_plugin_start (GimoPlugin *self)
{
    GimoRuntime *runtime;
    gboolean result;

    g_return_val_if_fail (GIMO_IS_PLUGIN (self), FALSE);

    runtime = _gimo_plugin_query_runtime (self);
    if (NULL == runtime)
        return FALSE;

    result = gimo_runtime_start (runtime);

    g_object_unref (runtime);

    return result;
}

gboolean gimo_plugin_stop (GimoPlugin *self)
{
    GimoRuntime *runtime;
    gboolean result;

    g_return_val_if_fail (GIMO_IS_PLUGIN (self), FALSE);

    runtime = _gimo_plugin_query_runtime (self);
    if (NULL == runtime)
        return FALSE;

    result = gimo_runtime_stop (runtime);

    g_object_unref (runtime);

    return result;
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
    GimoRuntime *runtime;
    GObject *object = NULL;

    g_return_val_if_fail (GIMO_IS_PLUGIN (self), NULL);

    runtime = _gimo_plugin_query_runtime (self);
    if (NULL == runtime)
        return NULL;

    object = gimo_runtime_resolve (runtime, symbol);

    g_object_unref (runtime);

    return object;
}

void _gimo_plugin_install (GimoPlugin *self,
                           GimoContext *context)
{
    GimoPluginPrivate *priv = self->priv;

    g_assert (NULL == priv->context);

    G_LOCK (plugin_lock);
    priv->context = context;
    priv->state = GIMO_PLUGIN_INSTALLED;
    G_UNLOCK (plugin_lock);
}

void _gimo_plugin_uninstall (GimoPlugin *self)
{
    GimoPluginPrivate *priv = self->priv;

    G_LOCK (plugin_lock);
    priv->context = NULL;
    priv->state = GIMO_PLUGIN_UNINSTALLED;
    G_UNLOCK (plugin_lock);
}
