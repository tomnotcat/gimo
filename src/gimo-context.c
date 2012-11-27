/* GIMO - A plugin framework based on GObject.
 *
 * Copyright (c) 2012 TinySoft, Inc.
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

#include "gimo-context.h"
#include "gimo-archive.h"
#include "gimo-dlmodule.h"
#include "gimo-error.h"
#include "gimo-extconfig.h"
#include "gimo-extension.h"
#include "gimo-extpoint.h"
#include "gimo-factory.h"
#include "gimo-loader.h"
#include "gimo-marshal.h"
#include "gimo-plugin.h"
#include "gimo-require.h"
#include "gimo-utils.h"

extern void _gimo_plugin_install (GimoPlugin *self,
                                  GimoContext *context,
                                  const gchar *cur_path);
extern void _gimo_plugin_uninstall (GimoPlugin *self);

G_DEFINE_TYPE (GimoContext, gimo_context, G_TYPE_OBJECT)

enum {
    SIG_STATECHANGED,
    LAST_SIGNAL
};

struct _GimoContextPrivate {
    GTree *plugins;
    gchar **paths;
    GMutex mutex;
};

struct _QueryExtension {
    const gchar *extpt_id;
    GPtrArray *array;
};

static guint context_signals[LAST_SIGNAL] = { 0 };

static void _gimo_context_plugin_destroy (gpointer p)
{
    _gimo_plugin_uninstall (p);
    g_object_unref (p);
}

static guint _gimo_context_load_plugin (GimoContext *self,
                                        GimoLoader *aloader,
                                        GimoLoader *mloader,
                                        const gchar *cur_path,
                                        const gchar *file_name,
                                        gboolean start)
{
    GimoArchive *archive;
    GPtrArray *objects;
    GObject *object;
    guint result = 0;

    archive = gimo_safe_cast (gimo_loader_load (aloader, file_name),
                              GIMO_TYPE_ARCHIVE);
    if (NULL == archive) {
        gimo_set_error_full (GIMO_ERROR_LOAD,
                             "GimoContext load archive error: %s",
                             file_name);
        return FALSE;
    }

    objects = gimo_archive_query_objects (archive);
    if (objects) {
        guint i;

        for (i = 0; i < objects->len; ++i) {
            object = g_ptr_array_index (objects, i);

            if (!GIMO_IS_PLUGIN (object))
                continue;

            if (!gimo_context_install_plugin (self,
                                              cur_path,
                                              GIMO_PLUGIN (object)))
            {
                continue;
            }

            if (start) {
                if (gimo_plugin_start (GIMO_PLUGIN (object), mloader))
                    ++result;
            }
            else {
                ++result;
            }
        }

        g_ptr_array_unref (objects);
    }

    g_object_unref (archive);

    return result;
}

static gboolean _gimo_context_query_plugins (gpointer key,
                                             gpointer value,
                                             gpointer data)
{
    GPtrArray *param = data;

    g_ptr_array_add (param, g_object_ref (value));

    return FALSE;
}

static gboolean _gimo_context_query_extensions (gpointer key,
                                                gpointer value,
                                                gpointer data)
{
    GimoPlugin *plugin = value;
    struct _QueryExtension *param = data;
    GPtrArray *exts;
    GimoExtension *ext;
    guint i;

    exts = gimo_plugin_query_extensions (plugin,
                                         param->extpt_id);
    if (NULL == exts)
        return FALSE;

    if (NULL == param->array)
        param->array = g_ptr_array_new_with_free_func (g_object_unref);

    for (i = 0; i < exts->len; ++i) {
        ext = g_ptr_array_index (exts, i);
        g_ptr_array_add (param->array, g_object_ref (ext));
    }

    g_ptr_array_unref (exts);
    return FALSE;
}

static void gimo_context_init (GimoContext *self)
{
    GimoContextPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_CONTEXT,
                                              GimoContextPrivate);
    priv = self->priv;

    priv->plugins = g_tree_new_full (_gimo_gtree_string_compare,
                                     NULL, NULL,
                                     _gimo_context_plugin_destroy);
    priv->paths = NULL;
    g_mutex_init (&priv->mutex);
}

static void gimo_context_constructed (GObject *gobject)
{
    GimoContext *self = GIMO_CONTEXT (gobject);
    GimoContextPrivate *priv = self->priv;
    GimoPlugin *plugin;
    GimoLoader *loader;
    GimoFactory *factory;
    GimoExtPoint *extpt;
    GimoExtension *ext;
    GPtrArray *array;
    const gchar *plugin_path;

    plugin_path = g_getenv ("GIMO_PLUGIN_PATH");
    if (plugin_path) {
        priv->paths = g_strsplit (plugin_path,
                                  G_SEARCHPATH_SEPARATOR_S,
                                  0);
    }

    /* org.gimo.core.loader */
    array = g_ptr_array_new_with_free_func (g_object_unref);
    extpt = gimo_ext_point_new ("archive", "Object Archive Loader");
    g_ptr_array_add (array, extpt);
    extpt = gimo_ext_point_new ("module", "Dynamic Module Loader");
    g_ptr_array_add (array, extpt);

    plugin = g_object_new (GIMO_TYPE_PLUGIN,
                           "id", "org.gimo.core.loader",
                           "name", "File Loader",
                           "version", "1.0",
                           "provider", "gimoapp.com",
                           "extpoints", array,
                           NULL);

    g_ptr_array_unref (array);

    /* Archive loader. */
    loader = gimo_loader_new ();
    gimo_loader_add_paths (loader, plugin_path);

    gimo_plugin_define (plugin, "archive", G_OBJECT (loader));
    g_object_unref (loader);

    /* Module loader. */
    loader = gimo_loader_new_cached ();
    gimo_loader_add_paths (loader, plugin_path);

    factory = gimo_factory_new ((GimoFactoryFunc) gimo_dlmodule_new, NULL);

    gimo_loader_register (loader, NULL, factory);
    gimo_plugin_define (plugin, "module", G_OBJECT (loader));

    g_object_unref (factory);

    gimo_context_install_plugin (self, NULL, plugin);
    gimo_plugin_start (plugin, loader);
    g_object_unref (plugin);

    /* org.gimo.core.xml.archive */
    array = g_ptr_array_new_with_free_func (g_object_unref);
    ext = gimo_extension_new ("loader",
                              "XML Archive Loader",
                              "org.gimo.core.loader.archive",
                              NULL);
    g_ptr_array_add (array, ext);

    plugin = gimo_plugin_new ("org.gimo.core.xml.archive",
                              "XML Archive Plugin",
                              "1.0",
                              "gimoapp.com",
                              NULL,
                              NULL,
                              "gimo_xmlarchive_plugin",
                              NULL,
                              NULL,
                              array);

    gimo_context_install_plugin (self, NULL, plugin);
    gimo_plugin_start (plugin, loader);

    g_ptr_array_unref (array);
    g_object_unref (plugin);
    g_object_unref (loader);
}

static void gimo_context_finalize (GObject *gobject)
{
    GimoContext *self = GIMO_CONTEXT (gobject);
    GimoContextPrivate *priv = self->priv;
    GObject *loader;

    /* Hold a reference to the module loader, so it will
     * be destroyed after all other plugins. */
    loader = gimo_context_resolve_extpoint (self,
                                            "org.gimo.core.loader.module");
    g_tree_unref (priv->plugins);
    g_strfreev (priv->paths);
    g_mutex_clear (&priv->mutex);
    g_object_unref (loader);

    G_OBJECT_CLASS (gimo_context_parent_class)->finalize (gobject);
}

static void gimo_context_class_init (GimoContextClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->constructed = gimo_context_constructed;
    gobject_class->finalize = gimo_context_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoContextPrivate));

    klass->state_changed = NULL;

    context_signals[SIG_STATECHANGED] =
            g_signal_new ("state-changed",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET (GimoContextClass, state_changed),
                          NULL, NULL,
                          _gimo_marshal_VOID__OBJECT_ENUM_ENUM,
                          G_TYPE_NONE, 3,
                          GIMO_TYPE_PLUGIN,
                          GIMO_TYPE_PLUGIN_STATE,
                          GIMO_TYPE_PLUGIN_STATE);
}

GimoContext* gimo_context_new (void)
{
    return g_object_new (GIMO_TYPE_CONTEXT, NULL);
}

/**
 * gimo_context_install_plugin:
 * @self: a #GimoContext
 * @path: (allow-none): plugin root path
 * @plugin: the plugin
 *
 * Install a plugin to the context.
 *
 * Returns: whether install success
 */
gboolean gimo_context_install_plugin (GimoContext *self,
                                      const gchar *path,
                                      GimoPlugin *plugin)
{
    GimoContextPrivate *priv;
    const gchar *plugin_id;

    g_return_val_if_fail (GIMO_IS_CONTEXT (self), FALSE);

    priv = self->priv;

    plugin_id = gimo_plugin_get_id (plugin);
    if (NULL == plugin_id || !plugin_id[0])
        gimo_set_error_return_val (GIMO_ERROR_INVALID_ID, FALSE);

    g_mutex_lock (&priv->mutex);

    if (g_tree_lookup (priv->plugins, plugin_id)) {
        g_mutex_unlock (&priv->mutex);
        gimo_set_error_return_val (GIMO_ERROR_CONFLICT, FALSE);
    }

    g_tree_insert (priv->plugins,
                   (gpointer) plugin_id,
                   g_object_ref (plugin));

    _gimo_plugin_install (plugin, self, path);

    g_mutex_unlock (&priv->mutex);

    g_signal_emit (self,
                   context_signals[SIG_STATECHANGED],
                   0,
                   plugin,
                   GIMO_PLUGIN_UNINSTALLED,
                   GIMO_PLUGIN_INSTALLED);
    return TRUE;
}

void gimo_context_uninstall_plugin (GimoContext *self,
                                    const gchar *plugin_id)
{
    GimoContextPrivate *priv;
    GimoPlugin *plugin;

    g_return_if_fail (GIMO_IS_CONTEXT (self));

    priv = self->priv;

    if (NULL == plugin_id || !plugin_id[0])
        return;

    g_mutex_lock (&priv->mutex);

    plugin = g_tree_lookup (priv->plugins, plugin_id);
    if (NULL == plugin) {
        g_mutex_unlock (&priv->mutex);
        return;
    }

    g_object_ref (plugin);
    g_tree_remove (priv->plugins, plugin_id);
    g_mutex_unlock (&priv->mutex);

    g_signal_emit (self,
                   context_signals[SIG_STATECHANGED],
                   0,
                   plugin,
                   GIMO_PLUGIN_INSTALLED,
                   GIMO_PLUGIN_UNINSTALLED);

    g_object_unref (plugin);
}

guint gimo_context_load_plugin (GimoContext *self,
                                const gchar *file_path,
                                GCancellable *cancellable,
                                gboolean start)
{
    GimoContextPrivate *priv;
    guint result = 0;
    GimoLoader *aloader = NULL;
    GimoLoader *mloader = NULL;
    gchar *real_path = (gchar *) file_path;

    g_return_val_if_fail (GIMO_IS_CONTEXT (self), 0);

    priv = self->priv;

    if (!g_file_test (file_path, G_FILE_TEST_EXISTS)) {
        if (priv->paths && !g_path_is_absolute (file_path)) {
            guint i = 0;

            while (priv->paths[i]) {
                real_path = g_build_filename (priv->paths[i],
                                              file_path,
                                              NULL);
                if (g_file_test (real_path, G_FILE_TEST_EXISTS))
                    break;

                g_free (real_path);
                ++i;
            }
        }
    }

    if (!g_file_test (real_path, G_FILE_TEST_EXISTS)) {
        gimo_set_error (GIMO_ERROR_NO_FILE);
        goto done;
    }

    aloader = gimo_safe_cast (
        gimo_context_resolve_extpoint (
            self, "org.gimo.core.loader.archive"),
        GIMO_TYPE_LOADER);

    if (NULL == aloader)
        goto done;

    mloader = gimo_safe_cast (
        gimo_context_resolve_extpoint (
            self, "org.gimo.core.loader.module"),
        GIMO_TYPE_LOADER);

    if (NULL == mloader)
        goto done;

    if (g_file_test (real_path, G_FILE_TEST_IS_REGULAR)) {
        gchar *dirname = g_path_get_dirname (real_path);

        result += _gimo_context_load_plugin (self,
                                             aloader,
                                             mloader,
                                             dirname,
                                             real_path,
                                             start);
        g_free (dirname);

        goto done;
    }

    if (g_file_test (real_path, G_FILE_TEST_IS_DIR)) {
        GFile *file;
        GFileEnumerator *enumerator;
        GError *error = NULL;

        file = g_file_new_for_path (real_path);
        enumerator = g_file_enumerate_children (file,
                                                "standard::*",
                                                G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                cancellable,
                                                &error);
        if (enumerator && !g_cancellable_is_cancelled (cancellable)) {
            GFileInfo *child_info;
            GFile *child;
            gchar *child_path;

            while (!g_cancellable_is_cancelled (cancellable)) {
                child_info = g_file_enumerator_next_file (enumerator,
                                                          cancellable,
                                                          NULL);
                if (NULL == child_info)
                    break;

                if (!g_cancellable_is_cancelled (cancellable)) {
                    child = g_file_resolve_relative_path (
                        file, g_file_info_get_name (child_info));

                    child_path = g_file_get_path (child);

                    result += _gimo_context_load_plugin (self,
                                                         aloader,
                                                         mloader,
                                                         real_path,
                                                         child_path,
                                                         start);
                    g_free (child_path);
                    g_object_unref (child);
                }

                g_object_unref (child_info);
            }
        }

        if (enumerator)
            g_object_unref (enumerator);

        g_object_unref (file);

        if (error) {
            gimo_set_error_full (GIMO_ERROR_INVALID_FILE,
                                 "GimoContext enumerate dir error: %s: %s",
                                 real_path,
                                 error ? error->message : NULL);
            g_clear_error (&error);
        }
    }
    else {
        gimo_set_error (GIMO_ERROR_INVALID_FILE);
    }

done:
    if (aloader)
        g_object_unref (aloader);

    if (mloader)
        g_object_unref (mloader);

    if (real_path != file_path)
        g_free (real_path);

    return result;
}

/**
 * gimo_context_query_plugin:
 * @self: a #GimoContext
 *
 * Query a plugin descriptor with the specified ID.
 *
 * Returns: (allow-none) (transfer full): a #GimoPlugin
 */
GimoPlugin* gimo_context_query_plugin (GimoContext *self,
                                       const gchar *plugin_id)
{
    GimoContextPrivate *priv;
    GimoPlugin *plugin = NULL;

    g_return_val_if_fail (GIMO_IS_CONTEXT (self), NULL);

    priv = self->priv;

    if (NULL == plugin_id || !plugin_id[0])
        return NULL;

    g_mutex_lock (&priv->mutex);

    plugin = g_tree_lookup (priv->plugins, plugin_id);
    if (plugin) {
        g_object_ref (plugin);
    }
    else {
        gimo_set_error_full (GIMO_ERROR_NO_PLUGIN,
                             "GimoContext query plugin failed: %s",
                             plugin_id);
    }

    g_mutex_unlock (&priv->mutex);

    return plugin;
}

/**
 * gimo_context_query_plugins:
 * @self: a #GimoContext
 *
 * Query all the installed plugins.
 *
 * Returns: (element-type Gimo.Plugin) (transfer container):
 *          An #GPtrArray of plugins if successful, %NULL on error.
 *          Free the returned array with g_object_unref().
 */
GPtrArray* gimo_context_query_plugins (GimoContext *self)
{
    GimoContextPrivate *priv;
    GPtrArray *param = NULL;

    g_return_val_if_fail (GIMO_IS_CONTEXT (self), NULL);

    priv = self->priv;

    if (g_tree_nnodes (priv->plugins) > 0) {
        param = g_ptr_array_new_with_free_func (g_object_unref);

        g_mutex_lock (&priv->mutex);

        g_tree_foreach (priv->plugins,
                        _gimo_context_query_plugins,
                        param);

        g_mutex_unlock (&priv->mutex);
    }

    return param;
}

/**
 * gimo_context_query_extpoint:
 * @self: a #GimoContext
 * @extpt_id: the extension point ID
 *
 * Query an extension point with the specified ID.
 *
 * Returns: (allow-none) (transfer full): A #GimoExtPoint
 *          if successful, %NULL on error. Free the returned
 *          object with g_object_unref().
 */
GimoExtPoint* gimo_context_query_extpoint (GimoContext *self,
                                           const gchar *extpt_id)
{
    GimoPlugin *plugin = NULL;
    GimoExtPoint *extpt = NULL;
    gchar *plugin_id = NULL;
    gchar *local_id = NULL;

    plugin_id = _gimo_parse_extension_id (extpt_id, &local_id);
    if (NULL == plugin_id)
        goto done;

    plugin = gimo_context_query_plugin (self, plugin_id);
    if (NULL == plugin)
        goto done;

    extpt = gimo_plugin_get_extpoint (plugin, local_id);
    if (extpt) {
        g_object_ref (extpt);
    }
    else {
        gimo_set_error_full (GIMO_ERROR_NO_EXTPOINT,
                             "GimoContext query extpoint failed: %s: %s",
                             plugin_id, local_id);
    }

done:
    if (plugin)
        g_object_unref (plugin);

    g_free (plugin_id);

    return extpt;
}

/**
 * gimo_context_query_extensions:
 * @self: a #GimoContext
 * @extpt_id: the extension point ID
 *
 * Query the specified extensions in this context.
 *
 * Returns: (element-type Gimo.Extension) (transfer container):
 *          An #GPtrArray of extensions if successful, %NULL on error.
 *          Free the returned array with g_ptr_array_unref().
 */
GPtrArray* gimo_context_query_extensions (GimoContext *self,
                                          const gchar *extpt_id)
{
    GimoContextPrivate *priv;
    struct _QueryExtension param;

    g_return_val_if_fail (GIMO_IS_CONTEXT (self), NULL);

    priv = self->priv;

    param.extpt_id = extpt_id;
    param.array = NULL;

    g_mutex_lock (&priv->mutex);

    g_tree_foreach (priv->plugins,
                    _gimo_context_query_extensions,
                    &param);

    g_mutex_unlock (&priv->mutex);

    return param.array;
}

/**
 * gimo_context_resolve_extpoint:
 * @self: a #GimoContext
 * @extpt_id: the extension point ID
 *
 * Resolve an extension point.
 *
 * Returns: (allow-none) (transfer full):
 *          A #GObject if successful, %NULL on error. Free the
 *          returned object with g_object_unref().
 */
GObject* gimo_context_resolve_extpoint (GimoContext *self,
                                        const gchar *extpt_id)
{
    GimoExtPoint *extpt = NULL;
    GimoPlugin *plugin = NULL;
    const gchar *symbol;
    GObject *object = NULL;

    extpt = gimo_context_query_extpoint (self, extpt_id);
    if (NULL == extpt)
        goto done;

    plugin = gimo_ext_point_query_plugin (extpt);
    if (NULL == plugin)
        goto done;

    symbol = gimo_ext_point_get_local_id (extpt);
    object = gimo_plugin_resolve (plugin, symbol);

done:
    if (extpt)
        g_object_unref (extpt);

    if (plugin)
        g_object_unref (plugin);

    return object;
}

void gimo_context_destroy (GimoContext *self)
{
    GPtrArray *array;
    GimoLoader *loader;
    guint i;

    array = gimo_context_query_plugins (self);
    if (array) {
        for (i = 0; i < array->len; ++i)
            gimo_plugin_stop (g_ptr_array_index (array, i));

        g_ptr_array_unref (array);
    }

    loader = gimo_safe_cast (
        gimo_context_resolve_extpoint (
            self, "org.gimo.core.loader.module"),
        GIMO_TYPE_LOADER);

    array = gimo_loader_query_cached (loader);

    if (array) {
        for (i = 0; i < array->len; ++i)
            gimo_loadable_unload (g_ptr_array_index (array, i));

        g_ptr_array_unref (array);
    }

    g_object_unref (loader);
}

void _gimo_context_plugin_state_changed (GimoContext *self,
                                         GimoPlugin *plugin,
                                         GimoPluginState old_state,
                                         GimoPluginState new_state)
{
    g_signal_emit (self,
                   context_signals[SIG_STATECHANGED],
                   0,
                   plugin,
                   old_state,
                   new_state);
}
