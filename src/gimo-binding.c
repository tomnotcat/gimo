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
#include "gimo-binding.h"

struct _BindingData {
    gpointer data;
    gboolean is_object;
};

G_LOCK_DEFINE_STATIC (binding_lock);

static void _binding_data_destroy (gpointer p)
{
    struct _BindingData *d = p;

    if (d->is_object)
        g_object_unref (d->data);
    else
        g_free (d->data);

    g_free (d);
}

static void _binding_location_destroy (gpointer p)
{
    g_dataset_destroy (p);
    g_free (p);
}

static gconstpointer _gimo_binding_location (GObject *object,
                                             gboolean create)
{
    static GQuark binding_quark;
    gpointer result;

    if (!binding_quark)
        binding_quark = g_quark_from_static_string ("gimo_binding_location");

    result = g_object_get_qdata (object, binding_quark);
    if (!result && create) {
        result = g_malloc (1);

        G_LOCK (binding_lock);

        if (!g_object_get_qdata (object, binding_quark)) {
            g_object_set_qdata_full (object,
                                     binding_quark,
                                     result,
                                     _binding_location_destroy);
        }
        else {
            g_free (result);
            result = g_object_get_qdata (object, binding_quark);
        }

        G_UNLOCK (binding_lock);
    }

    return result;
}

/**
 * gimo_bind_object:
 * @object: a #GObject
 * @key: the binding key
 * @data: (allow-none): the binding data
 *
 * Binding an object to the object.
 */
void gimo_bind_object (GObject *object,
                       const gchar *key,
                       GObject *data)
{
    gconstpointer l = _gimo_binding_location (object, TRUE);
    struct _BindingData *d;

    G_LOCK (binding_lock);

    if (data) {
        d = g_malloc (sizeof *d);
        d->data = g_object_ref (data);
        d->is_object = TRUE;

        g_dataset_set_data_full (l, key, d, _binding_data_destroy);
    }
    else
        g_dataset_remove_data (l, key);

    G_UNLOCK (binding_lock);
}

/**
 * gimo_bind_string:
 * @object: a #GObject
 * @key: the binding key
 * @data: (allow-none): the binding data
 *
 * Binding a string to the object.
 */
void gimo_bind_string (GObject *object,
                       const gchar *key,
                       const gchar *data)
{
    gconstpointer l = _gimo_binding_location (object, TRUE);
    struct _BindingData *d;

    G_LOCK (binding_lock);

    if (data) {
        d = g_malloc (sizeof *d);
        d->data = g_strdup (data);
        d->is_object = FALSE;

        g_dataset_set_data_full (l, key, d, _binding_data_destroy);
    }
    else
        g_dataset_remove_data (l, key);

    G_UNLOCK (binding_lock);
}

void gimo_binding_lock (GObject *object)
{
    G_LOCK (binding_lock);
}

void gimo_binding_unlock (GObject *object)
{
    G_UNLOCK (binding_lock);
}

/**
 * gimo_lookup_object:
 * @object: a #GObject
 * @key: the binding key
 *
 * Lookup a binded object, must hold binding lock.
 *
 * Returns: (allow-none) (transfer none): a #GObject
 */
GObject* gimo_lookup_object (GObject *object,
                             const gchar *key)
{
    gconstpointer l = _gimo_binding_location (object, FALSE);
    struct _BindingData *d;

    if (NULL == l)
        return NULL;

    d = g_dataset_get_data (l, key);
    if (NULL == d)
        return NULL;

    if (!d->is_object)
        return NULL;

    return d->data;
}

const gchar* gimo_lookup_string (GObject *object,
                                 const gchar *key)
{
    gconstpointer l = _gimo_binding_location (object, FALSE);
    struct _BindingData *d;

    if (NULL == l)
        return NULL;

    d = g_dataset_get_data (l, key);
    if (NULL == d)
        return NULL;

    if (d->is_object)
        return NULL;

    return d->data;
}

/**
 * gimo_query_object:
 * @object: a #GObject
 * @key: the binding key
 *
 * Query a binded object.
 *
 * Returns: (allow-none) (transfer full): a #GObject
 */
GObject* gimo_query_object (GObject *object,
                            const gchar *key)
{
    GObject *result;

    G_LOCK (binding_lock);

    result = gimo_lookup_object (object, key);
    if (result)
        g_object_ref (result);

    G_UNLOCK (binding_lock);

    return result;
}

gchar* gimo_query_string (GObject *object,
                          const gchar *key)
{
    const gchar *result;

    G_LOCK (binding_lock);

    result = gimo_lookup_string (object, key);
    if (result)
        result = g_strdup (result);

    G_UNLOCK (binding_lock);

    return (gchar *) result;
}
