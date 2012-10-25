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
#include "gimo-loader.h"
#include "gimo-module.h"
#include <string.h>

G_DEFINE_TYPE (GimoLoader, gimo_loader, G_TYPE_OBJECT)

struct _GimoLoaderPrivate {
    GSList *paths;
    GQueue *loaders;
    GTree *modules;
    GMutex mutex;
};

struct _LoaderInfo {
    gchar *suffix;
    GimoModuleCtorFunc ctor;
    gpointer user_data;
    gint ref_count;
};

static void _loader_info_destroy (gpointer p)
{
    struct _LoaderInfo *info = p;

    if (g_atomic_int_add (&info->ref_count, -1) == 1) {
        g_free (info->suffix);
        g_free (info);
    }
}

static gint _gimo_module_compare (gconstpointer a,
                                  gconstpointer b,
                                  gpointer user_data)
{
    return strcmp (a, b);
}

static GList* _gimo_loader_lookup (GimoLoader *self,
                                   const gchar *suffix)
{
    GimoLoaderPrivate *priv = self->priv;
    GList *it;
    struct _LoaderInfo *info;

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

static GimoModule* _gimo_loader_load_module (GPtrArray *loaders,
                                             const gchar *suffix,
                                             const gchar *file_name)
{
    GimoModule *module = NULL;
    struct _LoaderInfo *info;
    guint i;

    for (i = 0; i < loaders->len; ++i) {
        info = g_ptr_array_index (loaders, i);

        if (info->suffix) {
            if (NULL == suffix || strcmp (suffix, info->suffix))
                continue;
        }

        module = info->ctor (info->user_data);
        if (module) {
            if (!gimo_module_open (module, file_name)) {
                g_object_unref (module);
                module = NULL;
            }

            break;
        }
    }

    return module;
}

static void gimo_loader_init (GimoLoader *self)
{
    GimoLoaderPrivate *priv;
    const gchar *plugin_path = g_getenv ("GIMO_PLUGIN_PATH");

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_LOADER,
                                              GimoLoaderPrivate);
    priv = self->priv;

    priv->paths = NULL;
    priv->loaders = g_queue_new ();
    priv->modules = g_tree_new_full (_gimo_module_compare,
                                     NULL, NULL,
                                     g_object_unref);
    g_mutex_init (&priv->mutex);

    if (plugin_path) {
        gchar **dirs;
        guint i = 0;

        dirs = g_strsplit (plugin_path, G_SEARCHPATH_SEPARATOR_S, 0);
        while (dirs[i]) {
            priv->paths = g_slist_prepend (priv->paths, dirs[i]);
            ++i;
        }

        g_free (dirs);
    }
}

static void gimo_loader_finalize (GObject *gobject)
{
    GimoLoader *self = GIMO_LOADER (gobject);
    GimoLoaderPrivate *priv = self->priv;

    g_tree_unref (priv->modules);
    g_queue_free_full (priv->loaders, _loader_info_destroy);
    g_slist_free_full (priv->paths, g_free);
    g_mutex_clear (&priv->mutex);

    G_OBJECT_CLASS (gimo_loader_parent_class)->finalize (gobject);
}

static void gimo_loader_class_init (GimoLoaderClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_loader_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoLoaderPrivate));
}

GimoLoader* gimo_loader_new (void)
{
    return g_object_new (GIMO_TYPE_LOADER, NULL);
}

void gimo_loader_add_path (GimoLoader *self,
                           const gchar *path)
{
    GimoLoaderPrivate *priv;

    g_return_if_fail (GIMO_IS_LOADER (self));

    priv = self->priv;

    /* TODO: Make this function thread safe. */
    priv->paths = g_slist_prepend (priv->paths,
                                   g_strdup (path));
}

/**
 * gimo_loader_get_paths:
 * @self: a #GimoLoader
 *
 * Get the module search paths.
 *
 * Returns: (allow-none) (element-type utf8) (transfer none):
 *          the search paths list
 */
GSList* gimo_loader_get_paths (GimoLoader *self)
{
    g_return_val_if_fail (GIMO_IS_LOADER (self), NULL);

    /* TODO: Make this function thread safe. */
    return self->priv->paths;
}

/**
 * gimo_loader_register:
 * @self: a #GimoLoader
 * @suffix: (allow-none): the module file suffix
 * @func: (scope async): the constructor function
 * @user_data: user data for @func
 *
 * Load a module.
 *
 * Returns: (allow-none) (transfer full): a #GimoModule
 */
gboolean gimo_loader_register (GimoLoader *self,
                               const gchar *suffix,
                               GimoModuleCtorFunc func,
                               gpointer user_data)
{
    GimoLoaderPrivate *priv;
    gboolean result = FALSE;

    g_return_val_if_fail (GIMO_IS_LOADER (self), FALSE);

    priv = self->priv;

    g_mutex_lock (&priv->mutex);

    if (!_gimo_loader_lookup (self, suffix)) {
        struct _LoaderInfo *info;

        info = g_malloc (sizeof *info);
        info->suffix = g_strdup (suffix);
        info->ctor = func;
        info->user_data = user_data;
        info->ref_count = 1;

        g_queue_push_head (priv->loaders, info);
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
        _loader_info_destroy (node->data);
        g_queue_delete_link (priv->loaders, node);
    }

    g_mutex_unlock (&priv->mutex);
}

/**
 * gimo_loader_load:
 * @self: a #GimoLoader
 * @file_name: the module file name
 *
 * Load a module.
 *
 * Returns: (allow-none) (transfer full): a #GimoModule
 */
GimoModule* gimo_loader_load (GimoLoader *self,
                              const gchar *file_name)
{
    GimoLoaderPrivate *priv;
    GList *it;
    guint count;
    GPtrArray *arr;
    const gchar *suffix;
    struct _LoaderInfo *info;
    GimoModule *module = NULL;

    g_return_val_if_fail (GIMO_IS_LOADER (self), NULL);

    priv = self->priv;

    g_mutex_lock (&priv->mutex);

    module = g_tree_lookup (priv->modules, file_name);
    if (module) {
        g_object_ref (module);
        g_mutex_unlock (&priv->mutex);
        return module;
    }

    count = g_queue_get_length (priv->loaders);
    if (0 == count) {
        g_mutex_unlock (&priv->mutex);
        return NULL;
    }

    arr = g_ptr_array_new_full (count, _loader_info_destroy);
    it = g_queue_peek_head_link (priv->loaders);
    while (it) {
        info = it->data;
        g_atomic_int_add (&info->ref_count, 1);
        g_ptr_array_add (arr, info);
        it = it->next;
    }

    g_mutex_unlock (&priv->mutex);

    if (file_name) {
        suffix = strrchr (file_name, '.');

        if (suffix)
            ++suffix;
    }
    else {
        suffix = NULL;
    }

    module = _gimo_loader_load_module (arr, suffix, file_name);

    if (NULL == module && !g_path_is_absolute (file_name)) {
        GSList *it = priv->paths;
        gchar *full_path;

        /* TODO: Make paths thread safe. */
        while (it) {
            full_path = g_build_filename (it->data, file_name, NULL);
            module = _gimo_loader_load_module (arr, suffix, full_path);
            g_free (full_path);

            if (module)
                break;

            it = it->next;
        }
    }

    g_ptr_array_unref (arr);

    if (module) {
        GimoModule *exist;

        g_mutex_lock (&priv->mutex);

        exist = g_tree_lookup (priv->modules, file_name);
        if (!exist) {
            g_tree_insert (priv->modules,
                           (gpointer) gimo_module_get_name (module),
                           module);
        }
        else {
            g_object_unref (module);
            module = exist;
        }

        g_object_ref (module);
        g_mutex_unlock (&priv->mutex);
    }

    return module;
}
