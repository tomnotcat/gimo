/* GIMO - A plugin system based on GObject.
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
#include "gimo-loadable.h"

GType gimo_loadable_get_type (void)
{
    static GType loadable_type = 0;

    if (!loadable_type) {
        const GTypeInfo loadable_info = {
            sizeof (GimoLoadableInterface),  /* class_size */
            NULL,    /* base_init */
            NULL,    /* base_finalize */
        };

        loadable_type = g_type_register_static (G_TYPE_INTERFACE,
                                                "GimoLoadable",
                                                &loadable_info, 0);
    }

    return loadable_type;
}

gboolean gimo_loadable_load (GimoLoadable *self,
                             const gchar *file_name)
{
    g_return_val_if_fail (GIMO_IS_LOADABLE (self), FALSE);

    return GIMO_LOADABLE_GET_IFACE (self)->load (self, file_name);
}
