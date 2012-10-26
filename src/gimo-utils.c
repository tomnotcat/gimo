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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "gimo-utils.h"
#include <string.h>

void gimo_context_install_core_plugins (GimoContext *self)
{
}

gchar* _gimo_utils_parse_extension_id (const gchar *ext_id,
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

gint _gimo_utils_string_compare (gconstpointer a,
                                 gconstpointer b,
                                 gpointer user_data)
{
    return strcmp (a, b);
}
