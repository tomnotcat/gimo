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
#include "gimo-loader.h"
#include "gimo-dlmodule.h"
#include "gimo-error.h"
#include "gimo-factory.h"
#include "gimo-loadable.h"
#include "gimo-utils.h"
#include <string.h>

G_DEFINE_TYPE (GimoLoader, gimo_loader, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_CACHE
};

struct _GimoLoaderPrivate {
    GQueue *paths;
    GQueue *loaders;
    GTree *object_tree;
    GSList *object_list;
    GMutex mutex;
};

struct _FactoryInfo {
    gchar *suffix;
    GimoFactory *factory;
    gint ref_count;
};

struct _PathInfo {
    gchar *path;
    gint ref_count;
};

static void _factory_info_unref (gpointer p)
{
    struct _FactoryInfo *info = p;

    if (g_atomic_int_add (&info->ref_count, -1) == 1) {
        g_object_unref (info->factory);
        g_free (info->suffix);
        g_free (info);
    }
}

/* TODO: These _path_info_xxx duplicate with gimo-context.c */
static gint _path_info_compare (gconstpointer a,
                                gconstpointer b)
{
    const struct _PathInfo *info = a;
    return strcmp (info->path, b);
}

static void _path_info_ref (struct _PathInfo *info)
{
    g_atomic_int_add (&info->ref_count, 1);
}

static void _path_info_unref (gpointer p)
{
    struct _PathInfo *info = p;

    if (g_atomic_int_add (&info->ref_count, -1) == 1) {
        g_free (info->path);
        g_free (info);
    }
}

static GList* _gimo_loader_lookup (GimoLoader *self,
                                   const gchar *suffix)
{
    GimoLoaderPrivate *priv = self->priv;
    GList *it;
    struct _FactoryInfo *info;

    it = g_queue_peek_head_link (priv->loaders);
    while (it) {
        info = it->data;

        if (suffix == info->suffix)
            return it;

        if (suffix && info->suffix) {
            if (!strcmp (info->suffix, suffix))
                return it;
        }

        it = it->next;
    }

    return NULL;
}

static gboolean _gimo_loader_query_cached (gpointer key,
                                           gpointer value,
                                           gpointer data)
{
    GPtrArray *param = data;

    g_ptr_array_add (param, g_object_ref (value));

    return FALSE;
}

static GimoLoadable* _gimo_loader_load_file (GPtrArray *loaders,
                                             const gchar *suffix,
                                             const gchar *file_name)
{
    GimoLoadable *object = NULL;
    struct _FactoryInfo *info;
    guint i;

    if (file_name && !g_file_test (file_name, G_FILE_TEST_EXISTS)) {
        gimo_set_error_full (GIMO_ERROR_NO_FILE,
                             "GimoLoader file not exist: %s",
                             file_name);
        return NULL;
    }

    for (i = 0; i < loaders->len; ++i) {
        info = g_ptr_array_index (loaders, i);

        if (info->suffix) {
            if (NULL == suffix || strcmp (suffix, info->suffix))
                continue;
        }

        object = gimo_safe_cast (gimo_factory_make (info->factory),
                                 GIMO_TYPE_LOADABLE);
        if (object) {
            if (!gimo_loadable_load (object, file_name)) {
                g_object_unref (object);
                object = NULL;
            }

            break;
        }
    }

    return object;
}

static void gimo_loader_init (GimoLoader *self)
{
    GimoLoaderPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_LOADER,
                                              GimoLoaderPrivate);
    priv = self->priv;

    priv->paths = g_queue_new ();
    priv->loaders = g_queue_new ();
    priv->object_tree = NULL;
    priv->object_list = NULL;
    g_mutex_init (&priv->mutex);
}

static void gimo_loader_finalize (GObject *gobject)
{
    GimoLoader *self = GIMO_LOADER (gobject);
    GimoLoaderPrivate *priv = self->priv;

    if (priv->object_list)
        g_slist_free_full (priv->object_list, g_object_unref);

    if (priv->object_tree)
        g_tree_unref (priv->object_tree);

    g_queue_free_full (priv->loaders, _factory_info_unref);
    g_queue_free_full (priv->paths, _path_info_unref);
    g_mutex_clear (&priv->mutex);

    G_OBJECT_CLASS (gimo_loader_parent_class)->finalize (gobject);
}

static void gimo_loader_set_property (GObject *object,
                                      guint prop_id,
                                      const GValue *value,
                                      GParamSpec *pspec)
{
    GimoLoader *self = GIMO_LOADER (object);
    GimoLoaderPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_CACHE:
        if (g_value_get_boolean (value)) {
            priv->object_tree = g_tree_new_full (
                _gimo_gtree_string_compare, NULL, g_free, NULL);
        }
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_loader_get_property (GObject *object,
                                      guint prop_id,
                                      GValue *value,
                                      GParamSpec *pspec)
{
    GimoLoader *self = GIMO_LOADER (object);
    GimoLoaderPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_CACHE:
        g_value_set_boolean (value, priv->object_tree != NULL);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_loader_class_init (GimoLoaderClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_loader_finalize;
    gobject_class->set_property = gimo_loader_set_property;
    gobject_class->get_property = gimo_loader_get_property;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoLoaderPrivate));

    g_object_class_install_property (
        gobject_class, PROP_CACHE,
        g_param_spec_boolean ("cache",
                              "Cache object",
                              "Cache the loaded objects",
                              FALSE,
                              G_PARAM_READABLE |
                              G_PARAM_WRITABLE |
                              G_PARAM_CONSTRUCT_ONLY |
                              G_PARAM_STATIC_STRINGS));
}

GimoLoader* gimo_loader_new (void)
{
    return g_object_new (GIMO_TYPE_LOADER, NULL);
}

GimoLoader* gimo_loader_new_cached (void)
{
    return g_object_new (GIMO_TYPE_LOADER, "cache", TRUE, NULL);
}

/**
 * gimo_loader_add_paths:
 * @self: a #GimoLoader
 * @paths: (allow-none): path list
 *
 * Add file search paths to the loader.
 */
void gimo_loader_add_paths (GimoLoader *self,
                            const gchar *paths)
{
    GimoLoaderPrivate *priv;

    g_return_if_fail (GIMO_IS_LOADER (self));

    priv = self->priv;

    if (paths) {
        gchar **dirs;
        struct _PathInfo *info;
        guint i = 0;

        dirs = g_strsplit (paths, G_SEARCHPATH_SEPARATOR_S, 0);

        g_mutex_lock (&priv->mutex);
        while (dirs[i]) {
            info = g_malloc (sizeof *info);
            info->path = dirs[i];
            info->ref_count = 1;
            g_queue_push_head (priv->paths, info);
            ++i;
        }

        g_mutex_unlock (&priv->mutex);
        g_free (dirs);
    }
}

void gimo_loader_remove_paths (GimoLoader *self,
                               const gchar *paths)
{
    GimoLoaderPrivate *priv;

    g_return_if_fail (GIMO_IS_LOADER (self));

    priv = self->priv;

    if (paths) {
        gchar **dirs;
        GList *l;
        guint i = 0;

        dirs = g_strsplit (paths, G_SEARCHPATH_SEPARATOR_S, 0);

        g_mutex_lock (&priv->mutex);
        while (dirs[i]) {
            l = g_queue_find_custom (priv->paths,
                                     dirs[i],
                                     _path_info_compare);
            if (l) {
                _path_info_unref (l->data);
                g_queue_delete_link (priv->paths, l);
            }

            ++i;
        }

        g_mutex_unlock (&priv->mutex);
        g_strfreev (dirs);
    }
}

/**
 * gimo_loader_dup_paths:
 * @self: a #GimoLoader
 *
 * Duplicate a copy of load search paths.
 *
 * Returns: (allow-none) (element-type utf8) (transfer container):
 *          an array of search paths
 */
GPtrArray* gimo_loader_dup_paths (GimoLoader *self)
{
    GimoLoaderPrivate *priv;
    GPtrArray *result = NULL;

    g_return_val_if_fail (GIMO_IS_LOADER (self), NULL);

    priv = self->priv;

    g_mutex_lock (&priv->mutex);

    if (g_queue_get_length (priv->paths) > 0) {
        GList *it;
        struct _PathInfo *pi;

        result = g_ptr_array_new_with_free_func (g_free);
        it = g_queue_peek_head_link (priv->paths);
        while (it) {
            pi = it->data;
            g_ptr_array_add (result, g_strdup (pi->path));
            it = it->next;
        }
    }

    g_mutex_unlock (&priv->mutex);

    return result;
}

/**
 * gimo_loader_register:
 * @self: a #GimoLoader
 * @suffix: (allow-none): the file suffix
 * @factory: a #GimoLoadable factory
 *
 * Register a #GimoLoadable factor.
 *
 * Returns: %TRUE if register success.
 */
gboolean gimo_loader_register (GimoLoader *self,
                               const gchar *suffix,
                               GimoFactory *factory)
{
    GimoLoaderPrivate *priv;
    gboolean result = FALSE;

    g_return_val_if_fail (GIMO_IS_LOADER (self), FALSE);

    priv = self->priv;

    g_mutex_lock (&priv->mutex);

    if (!_gimo_loader_lookup (self, suffix)) {
        struct _FactoryInfo *info;

        info = g_malloc (sizeof *info);
        info->suffix = g_strdup (suffix);
        info->factory = g_object_ref (factory);
        info->ref_count = 1;

        if (suffix)
            g_queue_push_head (priv->loaders, info);
        else
            g_queue_push_tail (priv->loaders, info);

        result = TRUE;
    }

    g_mutex_unlock (&priv->mutex);

    return result;
}

void gimo_loader_unregister (GimoLoader *self,
                             const gchar *suffix)
{
    GimoLoaderPrivate *priv;
    GList *node;

    g_return_if_fail (GIMO_IS_LOADER (self));

    priv = self->priv;

    g_mutex_lock (&priv->mutex);

    node = _gimo_loader_lookup (self, suffix);
    if (node) {
        _factory_info_unref (node->data);
        g_queue_delete_link (priv->loaders, node);
    }

    g_mutex_unlock (&priv->mutex);
}

/**
 * gimo_loader_load:
 * @self: a #GimoLoader
 * @file_name: the file name
 *
 * Load a file.
 *
 * Returns: (allow-none) (transfer full):
 *     A #GimoLoadable if successful, %NULL on error.
 *     Free the returned object with g_object_unref().
 */
GimoLoadable* gimo_loader_load (GimoLoader *self,
                                const gchar *file_name)
{
    GimoLoaderPrivate *priv;
    GList *it;
    guint count;
    GPtrArray *arr;
    const gchar *suffix;
    struct _FactoryInfo *info;
    GimoLoadable *result = NULL;

    g_return_val_if_fail (GIMO_IS_LOADER (self), NULL);

    priv = self->priv;

    g_mutex_lock (&priv->mutex);

    if (file_name) {
        suffix = strrchr (file_name, '.');

        if (suffix)
            ++suffix;
    }
    else {
        suffix = NULL;
    }

    if (!_gimo_loader_lookup (self, suffix) &&
        !_gimo_loader_lookup (self, NULL))
    {
        g_mutex_unlock (&priv->mutex);
        return NULL;
    }

    if (priv->object_tree) {
        result = g_tree_lookup (priv->object_tree, file_name);

        if (result) {
            g_object_ref (result);
            g_mutex_unlock (&priv->mutex);
            return result;
        }
    }

    count = g_queue_get_length (priv->loaders);
    if (0 == count) {
        g_mutex_unlock (&priv->mutex);
        return NULL;
    }

    arr = g_ptr_array_new_full (count, _factory_info_unref);
    it = g_queue_peek_head_link (priv->loaders);
    while (it) {
        info = it->data;
        g_atomic_int_add (&info->ref_count, 1);
        g_ptr_array_add (arr, info);
        it = it->next;
    }

    g_mutex_unlock (&priv->mutex);

    result = _gimo_loader_load_file (arr, suffix, file_name);

    if (NULL == result && file_name &&
        !g_path_is_absolute (file_name) &&
        g_queue_get_length (priv->paths) > 0)
    {
        GQueue *paths;
        GList *it;
        struct _PathInfo *pi;
        gchar *full_path;

        g_mutex_lock (&priv->mutex);

        paths = g_queue_copy (priv->paths);
        g_queue_foreach (paths, (GFunc) _path_info_ref, NULL);

        g_mutex_unlock (&priv->mutex);

        it = g_queue_peek_head_link (paths);
        while (it) {
            pi = it->data;
            full_path = g_build_filename (pi->path, file_name, NULL);
            result = _gimo_loader_load_file (arr, suffix, full_path);
            g_free (full_path);

            if (result)
                break;

            it = it->next;
        }

        g_queue_free_full (paths, _path_info_unref);
    }

    g_ptr_array_unref (arr);

    if (result && priv->object_tree) {
        GimoLoadable *exist;

        g_mutex_lock (&priv->mutex);

        exist = g_tree_lookup (priv->object_tree, file_name);
        if (!exist) {
            g_tree_insert (priv->object_tree,
                           g_strdup (file_name),
                           result);

            priv->object_list = g_slist_prepend (priv->object_list,
                                                 result);
        }
        else {
            g_object_unref (result);
            result = exist;
        }

        g_object_ref (result);
        g_mutex_unlock (&priv->mutex);
    }

    return result;
}

/**
 * gimo_loader_query_cached:
 * @self: a #GimoLoader
 *
 * Query all the cached loaded objects.
 *
 * Returns: (element-type Gimo.Loadable) (transfer container):
 *          An #GPtrArray if successful or %NULL if there's none.
 *          Free the returned array with g_ptr_array_unref().
 */
GPtrArray* gimo_loader_query_cached (GimoLoader *self)
{
    GimoLoaderPrivate *priv;
    GPtrArray *result = NULL;

    g_return_val_if_fail (GIMO_IS_LOADER (self), NULL);

    priv = self->priv;

    if (priv->object_tree && g_tree_nnodes (priv->object_tree) > 0) {
        result = g_ptr_array_new_with_free_func (g_object_unref);

        g_mutex_lock (&priv->mutex);

        g_tree_foreach (priv->object_tree,
                        _gimo_loader_query_cached,
                        result);

        g_mutex_unlock (&priv->mutex);
    }

    return result;
}
