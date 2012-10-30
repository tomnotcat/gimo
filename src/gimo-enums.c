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
#include "gimo-enums.h"

GType gimo_plugin_state_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile)) {
        static const GEnumValue values[] = {
            { GIMO_PLUGIN_UNINSTALLED, "GIMO_PLUGIN_UNINSTALLED", "UNINSTALLED" },
            { GIMO_PLUGIN_INSTALLED, "GIMO_PLUGIN_INSTALLED", "INSTALLED" },
            { GIMO_PLUGIN_RESOLVED, "GIMO_PLUGIN_RESOLVED", "RESOLVED" },
            { GIMO_PLUGIN_STARTING, "GIMO_PLUGIN_STARTING", "STARTING" },
            { GIMO_PLUGIN_STOPPING, "GIMO_PLUGIN_STOPPING", "STOPPING" },
            { GIMO_PLUGIN_ACTIVE, "GIMO_PLUGIN_ACTIVE", "ACTIVE" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id =
                g_enum_register_static (g_intern_static_string ("GimoPluginState"), values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}
