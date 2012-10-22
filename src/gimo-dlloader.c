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
#include "gimo-dlloader.h"
#include "gimo-pluginfo.h"
#include <gmodule.h>

static void gimo_loader_interface_init (GimoLoaderInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GimoDlloader, gimo_dlloader, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_LOADER,
                                                gimo_loader_interface_init))

static GimoPlugin* _gimo_dlloader_load (GimoLoader *loader,
                                        GimoPluginfo *info)
{
    const gchar *path = NULL;
    const gchar *symbol = NULL;
    GModule *module = NULL;
    GimoPlugin* (*new_plugin) (GimoPluginfo*) = NULL;
    GimoPlugin *plugin = NULL;

    path = gimo_pluginfo_get_url (info);
    symbol = gimo_pluginfo_get_symbol (info);
    if (NULL == symbol)
        goto done;

    module = g_module_open (path, G_MODULE_BIND_LAZY);
    if (NULL == module) {
        g_warning ("Dlloader: open module error: %s: %s",
                   path, g_module_error ());
        goto done;;
    }

    if (!g_module_symbol (module,
                          symbol,
                          (gpointer *) &new_plugin))
    {
        g_warning ("Dlloader: resolve symbol error: %s: %s",
                   path, symbol);
        goto done;
    }

    if (NULL == new_plugin)
        goto done;

    plugin = new_plugin (info);

done:
    if (NULL == plugin) {
        if (module) {
            if (!g_module_close (module)) {
                g_warning ("Dlloader: close module error: %s",
                           g_module_error ());
            }
        }
    }

    return plugin;
}

static void gimo_loader_interface_init (GimoLoaderInterface *iface)
{
    iface->load = _gimo_dlloader_load;
}

static void gimo_dlloader_init (GimoDlloader *self)
{
}

static void gimo_dlloader_class_init (GimoDlloaderClass *klass)
{
}

GimoDlloader* gimo_dlloader_new (void)
{
    return g_object_new (GIMO_TYPE_DLLOADER, NULL);
}
