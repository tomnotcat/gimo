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

GType gimo_loader_get_type (void)
{
    static GType loader_type = 0;

    if (!loader_type) {
        const GTypeInfo loader_info = {
            sizeof (GimoLoaderInterface),  /* class_size */
            NULL,    /* base_init */
            NULL,    /* base_finalize */
        };

        loader_type = g_type_register_static (G_TYPE_INTERFACE,
                                              "GimoLoader",
                                              &loader_info, 0);
    }

    return loader_type;
}

/**
 * gimo_loader_load:
 * @self: a #GimoLoader
 *
 * Load the plugin runtime object.
 *
 * Returns: (allow-none) (transfer full): a #GimoPlugin
 */
GimoPlugin* gimo_loader_load (GimoLoader *self,
                              GimoPluginfo *info)
{
    g_return_val_if_fail (GIMO_IS_LOADER (self), NULL);

    return GIMO_LOADER_GET_IFACE (self)->load (self, info);
}
