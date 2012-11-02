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

#include "gimo-archive.h"
#include "gimo-utils.h"

G_DEFINE_TYPE (GimoArchive, gimo_archive, G_TYPE_OBJECT)

struct _GimoArchivePrivate {
    GTree *objects;
    GMutex mutex;
};

static void gimo_archive_init (GimoArchive *self)
{
    GimoArchivePrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_ARCHIVE,
                                              GimoArchivePrivate);
    priv = self->priv;

    priv->objects = g_tree_new_full (_gimo_utils_string_compare,
                                     NULL, g_free, g_object_unref);
    g_mutex_init (&priv->mutex);
}

static void gimo_archive_finalize (GObject *gobject)
{
    GimoArchive *self = GIMO_ARCHIVE (gobject);
    GimoArchivePrivate *priv = self->priv;

    g_tree_unref (priv->objects);
    g_mutex_clear (&priv->mutex);

    G_OBJECT_CLASS (gimo_archive_parent_class)->finalize (gobject);
}

static void gimo_archive_class_init (GimoArchiveClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_archive_finalize;

    klass->read = NULL;
    klass->save = NULL;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoArchivePrivate));
}

GimoArchive* gimo_archive_new (void)
{
    return g_object_new (GIMO_TYPE_ARCHIVE, NULL);
}

gboolean gimo_archive_read (GimoArchive *self,
                            const gchar *file_name)
{
    g_return_val_if_fail (GIMO_IS_ARCHIVE (self), FALSE);

    return GIMO_ARCHIVE_GET_CLASS (self)->read (self, file_name);
}

gboolean gimo_archive_save (GimoArchive *self,
                            const gchar *file_name)
{
    g_return_val_if_fail (GIMO_IS_ARCHIVE (self), FALSE);

    return GIMO_ARCHIVE_GET_CLASS (self)->save (self, file_name);
}

gboolean gimo_archive_add_object (GimoArchive *self,
                                  const gchar *id,
                                  GObject *object)
{
    GimoArchivePrivate *priv;

    g_return_val_if_fail (GIMO_IS_ARCHIVE (self), FALSE);

    priv = self->priv;

    g_mutex_lock (&priv->mutex);

    if (g_tree_lookup (priv->objects, id)) {
        g_mutex_unlock (&priv->mutex);
        return FALSE;
    }

    g_tree_insert (priv->objects,
                   g_strdup (id),
                   g_object_ref (object));

    g_mutex_unlock (&priv->mutex);

    return TRUE;
}

/**
 * gimo_archive_query_object:
 * @self: a #GimoArchive
 * @id: the object identifier
 *
 * Query object by identifier.
 *
 * Returns: (allow-none) (transfer full): a #GObject
 */
GObject* gimo_archive_query_object (GimoArchive *self,
                                    const gchar *id)
{
    GimoArchivePrivate *priv;
    GObject *object;

    g_return_val_if_fail (GIMO_IS_ARCHIVE (self), NULL);

    priv = self->priv;

    g_mutex_lock (&priv->mutex);

    object = g_tree_lookup (priv->objects, id);

    if (object)
        g_object_ref (object);

    g_mutex_unlock (&priv->mutex);

    return object;
}

void gimo_archive_remove_object (GimoArchive *self,
                                 const gchar *id)
{
    GimoArchivePrivate *priv;

    g_return_if_fail (GIMO_IS_ARCHIVE (self));

    priv = self->priv;

    g_mutex_lock (&priv->mutex);

    g_tree_remove (priv->objects, id);

    g_mutex_unlock (&priv->mutex);
}
