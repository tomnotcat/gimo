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

#include "gimo-datastore.h"
#include "gimo-utils.h"
#include <stdlib.h>
#include <string.h>

G_DEFINE_TYPE (GimoDataStore, gimo_data_store, G_TYPE_OBJECT)

struct _GimoDataStorePrivate {
    GTree *datas;
};

G_LOCK_DEFINE_STATIC (bind_lock);

static void _gimo_bind_location_destroy (gpointer p)
{
    g_dataset_destroy (p);
    g_free (p);
}

static gconstpointer _gimo_bind_location (GObject *object,
                                          gboolean create)
{
    static GQuark bind_quark;
    gpointer result;

    if (!bind_quark)
        bind_quark = g_quark_from_static_string ("gimo_bind_location");

    result = g_object_get_qdata (object, bind_quark);
    if (!result && create) {
        result = g_malloc (1);

        G_LOCK (bind_lock);

        if (!g_object_get_qdata (object, bind_quark)) {
            g_object_set_qdata_full (object,
                                     bind_quark,
                                     result,
                                     _gimo_bind_location_destroy);
        }
        else {
            g_free (result);
            result = g_object_get_qdata (object, bind_quark);
        }

        G_UNLOCK (bind_lock);
    }

    return result;
}

static void _gimo_data_value_destroy (gpointer x)
{
    GValue *p = x;
    g_value_unset (p);
    g_free (p);
}

static void gimo_data_store_init (GimoDataStore *self)
{
    GimoDataStorePrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_DATASTORE,
                                              GimoDataStorePrivate);
    priv = self->priv;

    priv->datas = g_tree_new_full (_gimo_gtree_string_compare,
                                   NULL, NULL,
                                   _gimo_data_value_destroy);
}

static void gimo_data_store_finalize (GObject *gobject)
{
    GimoDataStore *self = GIMO_DATASTORE (gobject);
    GimoDataStorePrivate *priv = self->priv;

    g_tree_unref (priv->datas);

    G_OBJECT_CLASS (gimo_data_store_parent_class)->finalize (gobject);
}

static void gimo_data_store_class_init (GimoDataStoreClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (GimoDataStorePrivate));

    gobject_class->finalize = gimo_data_store_finalize;
}

GimoDataStore* gimo_data_store_new (void)
{
    return g_object_new (GIMO_TYPE_DATASTORE, NULL);
}

/**
 * gimo_data_store_set:
 * @self: a #GimoDataStore
 * @key: the data key
 * @value: (allow-none): the data value
 *
 * Store a data to the data store.
 */
void gimo_data_store_set (GimoDataStore *self,
                          const gchar *key,
                          const GValue *value)
{
    GimoDataStorePrivate *priv;

    g_return_if_fail (GIMO_IS_DATASTORE (self));

    priv = self->priv;

    if (value) {
        GValue *v;
        gchar *k;
        gsize size;

        size = sizeof (GValue) + strlen (key) + 1;
        v = g_malloc (size);
        memset (v, 0, size);

        k = (gchar *)v + sizeof (GValue);
        g_value_init (v, G_VALUE_TYPE (value));
        g_value_copy (value, v);
        strcpy (k, key);

        g_tree_replace (priv->datas, k, v);
    }
    else {
        g_tree_remove (priv->datas, key);
    }
}

/**
 * gimo_data_store_get:
 * @self: a #GimoDataStore
 * @key: the data key
 *
 * Retrieve a data from the data store.
 *
 * Returns: (allow-none) (transfer none): a #GValue containing
 *          the value stored in the given key, or %NULL on error.
 */
const GValue* gimo_data_store_get (GimoDataStore *self,
                                   const gchar *key)
{
    GimoDataStorePrivate *priv;

    g_return_val_if_fail (GIMO_IS_DATASTORE (self), NULL);

    priv = self->priv;

    return g_tree_lookup (priv->datas, key);
}

/**
 * gimo_data_store_foreach: (skip)
 * @self: a #GimoDataStore
 * @func: the function to call for each data visited. If this function
 *   returns %TRUE, the traversal is stopped.
 * @user_data: user data to pass to the function.
 *
 * Calls the given function for each of the key/value pairs in the store.
 */
void gimo_data_store_foreach (GimoDataStore *self,
                              GTraverseFunc func,
                              gpointer user_data)
{
    GimoDataStorePrivate *priv;

    g_return_if_fail (GIMO_IS_DATASTORE (self));

    priv = self->priv;

    g_tree_foreach (priv->datas, func, user_data);
}

/**
 * gimo_data_store_set_string:
 * @self: a #GimoDataStore
 * @key: the data key
 * @value: (allow-none): the data value
 *
 * Store a string to the data store.
 */
void gimo_data_store_set_string (GimoDataStore *self,
                                 const gchar *key,
                                 const gchar *value)
{
    if (value) {
        GValue v = G_VALUE_INIT;

        g_value_init (&v, G_TYPE_STRING);
        g_value_set_static_string (&v, value);
        gimo_data_store_set (self, key, &v);
        g_value_unset (&v);
    }
    else {
        gimo_data_store_set (self, key, NULL);
    }
}

/**
 * gimo_data_store_get_string:
 * @self: a #GimoDataStore
 * @key: the string key
 *
 * Retrieve a string from the data store.
 *
 * Returns: (allow-none) (transfer none): the string stored
 *          in the given key, or %NULL on error.
 */
const gchar* gimo_data_store_get_string (GimoDataStore *self,
                                         const gchar *key)
{
    const GValue *v = gimo_data_store_get (self, key);

    if (v && G_VALUE_HOLDS_STRING (v))
        return g_value_get_string (v);

    return NULL;
}

/**
 * gimo_data_store_set_object:
 * @self: a #GimoDataStore
 * @key: the data key
 * @value: (allow-none): the data value
 *
 * Store a object to the data store.
 */
void gimo_data_store_set_object (GimoDataStore *self,
                                 const gchar *key,
                                 GObject *value)
{
    if (value) {
        GValue v = G_VALUE_INIT;

        g_value_init (&v, G_TYPE_OBJECT);
        g_value_set_object (&v, value);
        gimo_data_store_set (self, key, &v);
        g_value_unset (&v);
    }
    else {
        gimo_data_store_set (self, key, NULL);
    }
}

/**
 * gimo_data_store_get_object:
 * @self: a #GimoDataStore
 * @key: the object key
 *
 * Retrieve an object from the data store.
 *
 * Returns: (allow-none) (transfer none): the object stored
 *          in the given key, or %NULL on error.
 */
GObject* gimo_data_store_get_object (GimoDataStore *self,
                                     const gchar *key)
{
    const GValue *v = gimo_data_store_get (self, key);

    if (v && G_VALUE_HOLDS_OBJECT (v))
        return g_value_get_object (v);

    return NULL;
}

/**
 * gimo_bind:
 * @object: a #GObject
 * @key: the data key
 * @value: (allow-none): the data value
 *
 * Bind a data to the specified object.
 */
void gimo_bind (GObject *object,
                const gchar *key,
                const GValue *value)
{
    gconstpointer l = _gimo_bind_location (object, TRUE);

    G_LOCK (bind_lock);

    if (value) {
        GValue *v;

        v = g_malloc (sizeof (GValue));
        memset (v, 0, sizeof (GValue));
        g_value_init (v, G_VALUE_TYPE (value));
        g_value_copy (value, v);

        g_dataset_set_data_full (l, key, v, _gimo_data_value_destroy);
    }
    else
        g_dataset_remove_data (l, key);

    G_UNLOCK (bind_lock);
}

/**
 * gimo_lookup:
 * @object: a #GObject
 * @key: the data key
 *
 * Lookup a previous bind data from the specified object.
 *
 * Returns: (allow-none) (transfer none): a #GValue containing
 *          the value bind to the given key, or %NULL on error.
 */
const GValue* gimo_lookup (GObject *object,
                           const gchar *key)
{
    gconstpointer l = _gimo_bind_location (object, FALSE);

    if (NULL == l)
        return NULL;

    return g_dataset_get_data (l, key);
}

/**
 * gimo_bind_string:
 * @object: a #GObject
 * @key: the data key
 * @value: (allow-none): the data value
 *
 * Bind a string to the specified object.
 */
void gimo_bind_string (GObject *object,
                       const gchar *key,
                       const gchar *value)
{
    if (value) {
        GValue v = G_VALUE_INIT;

        g_value_init (&v, G_TYPE_STRING);
        g_value_set_static_string (&v, value);
        gimo_bind (object, key, &v);
        g_value_unset (&v);
    }
    else {
        gimo_bind (object, key, NULL);
    }
}

/**
 * gimo_lookup_string:
 * @object: a #GObject
 * @key: the data key
 *
 * Lookup a previous bind string from the specified object.
 *
 * Returns: (allow-none) (transfer none): the string bind
 *          to the given key, or %NULL on error.
 */
const gchar* gimo_lookup_string (GObject *object,
                                 const gchar *key)
{
    const GValue *v = gimo_lookup (object, key);

    if (v && G_VALUE_HOLDS_STRING (v))
        return g_value_get_string (v);

    return NULL;
}

/**
 * gimo_bind_object:
 * @object: a #GObject
 * @key: the data key
 * @value: (allow-none): the data value
 *
 * Bind an object to the specified object.
 */
void gimo_bind_object (GObject *object,
                       const gchar *key,
                       GObject *value)
{
    if (value) {
        GValue v = G_VALUE_INIT;

        g_value_init (&v, G_TYPE_OBJECT);
        g_value_set_object (&v, value);
        gimo_bind (object, key, &v);
        g_value_unset (&v);
    }
    else {
        gimo_bind (object, key, NULL);
    }
}

/**
 * gimo_lookup_object:
 * @object: a #GObject
 * @key: the data key
 *
 * Lookup a previous bind object from the specified object.
 *
 * Returns: (allow-none) (transfer none): the object bind
 *          to the given key, or %NULL on error.
 */
GObject* gimo_lookup_object (GObject *object,
                             const gchar *key)
{
    const GValue *v = gimo_lookup (object, key);

    if (v && G_VALUE_HOLDS_OBJECT (v))
        return g_value_get_object (v);

    return NULL;
}
