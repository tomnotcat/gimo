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
#include "gimo-datastore.h"
#include <string.h>

int main (int argc, char *argv[])
{
    GimoDataStore *store;
    GObject *object;
    const GValue *value;

    g_type_init ();

    store = gimo_data_store_new ();
    g_assert (!gimo_data_store_get_string (store, "hello"));
    g_assert (!gimo_data_store_get_object (store, "hello"));
    gimo_data_store_set_string (store, "hello", "world");
    g_assert (strcmp (gimo_data_store_get_string (store, "hello"),
                      "world") == 0);
    value = gimo_data_store_get (store, "hello");
    g_assert (value && G_VALUE_TYPE (value) == G_TYPE_STRING);

    object = G_OBJECT (gimo_data_store_new ());
    gimo_data_store_set_object (store, "hello", G_OBJECT (object));
    g_assert (gimo_data_store_get_object (store, "hello") == object);
    value = gimo_data_store_get (store, "hello");
    g_assert (value && G_VALUE_TYPE (value) == G_TYPE_OBJECT);

    g_assert (!gimo_lookup_string (object, "bind"));
    gimo_bind_string (object, "bind", "string");
    g_assert (strcmp (gimo_lookup_string (object, "bind"),
                      "string") == 0);
    g_assert (!gimo_lookup_object (G_OBJECT (store), "bind"));
    gimo_bind_object (G_OBJECT (store), "bind", object);
    g_assert (gimo_lookup_object (G_OBJECT (store), "bind") == object);

    g_object_unref (object);
    g_object_unref (store);

    return 0;
}
