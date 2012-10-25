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
#include "gimo-module.h"

GType gimo_module_get_type (void)
{
    static GType module_type = 0;

    if (!module_type) {
        const GTypeInfo module_info = {
            sizeof (GimoModuleInterface),  /* class_size */
            NULL,    /* base_init */
            NULL,    /* base_finalize */
        };

        module_type = g_type_register_static (G_TYPE_INTERFACE,
                                              "GimoModule",
                                              &module_info, 0);
    }

    return module_type;
}

gboolean gimo_module_open (GimoModule *self,
                           const gchar *file_name)
{
    g_return_val_if_fail (GIMO_IS_MODULE (self), FALSE);

    return GIMO_MODULE_GET_IFACE (self)->open (self, file_name);
}

gboolean gimo_module_close (GimoModule *self)
{
    g_return_val_if_fail (GIMO_IS_MODULE (self), FALSE);

    return GIMO_MODULE_GET_IFACE (self)->close (self);
}

const gchar* gimo_module_get_name (GimoModule *self)
{
    g_return_val_if_fail (GIMO_IS_MODULE (self), NULL);

    return GIMO_MODULE_GET_IFACE (self)->get_name (self);
}

/**
 * gimo_module_resolve:
 * @self: a #GimoModule
 * @symbol: the constructor symbol
 * @param: (allow-none): the parameter for constructor
 *
 * Resolve the symbol as an object constructor and new an object.
 *
 * Returns: (allow-none) (transfer full): a #GObject
 */
GObject* gimo_module_resolve (GimoModule *self,
                              const gchar *symbol,
                              GObject *param)
{
    g_return_val_if_fail (GIMO_IS_MODULE (self), NULL);

    return GIMO_MODULE_GET_IFACE (self)->resolve (self, symbol, param);
}
