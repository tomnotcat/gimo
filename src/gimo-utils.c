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
#include "gimo-utils.h"
#include "gimo-error.h"
#include <string.h>

gchar* _gimo_parse_extension_id (const gchar *ext_id,
                                 gchar **local_id)
{
    gchar *dot = strrchr (ext_id, '.');
    gchar *result;

    if (NULL == dot) {
        *local_id = (gchar *) ext_id;
        return NULL;
    }

    result = g_strndup (ext_id, dot - ext_id);
    *local_id = dot + 1;

    return result;
}

gint _gimo_gtree_string_compare (gconstpointer a,
                                 gconstpointer b,
                                 gpointer user_data)
{
    return strcmp (a, b);
}

GPtrArray* _gimo_clone_object_array (GPtrArray *arr,
                                     GType type,
                                     void (*func) (gpointer, gpointer),
                                     gpointer user_data)
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
            func (object, user_data);
    }

    return result;
}

/**
 * gimo_safe_cast:
 * @object: (type GObject.Object) (allow-none): a #GObject
 * @type: the target type to cast
 *
 * Cast a object to the specified type, unref the object if failed.
 *
 * Returns: (type GObject.Object) (allow-none) (transfer full):
 *     A #GObject if successful, %NULL on error.
 */
gpointer gimo_safe_cast (gpointer object, GType type)
{
    gpointer result;

    if (object) {
        result = G_TYPE_CHECK_INSTANCE_CAST (object, type, gpointer);

        if (!result) {
            g_object_unref (object);
            gimo_set_error (GIMO_ERROR_INVALID_OBJECT);
        }

        return result;
    }

    return NULL;
}
