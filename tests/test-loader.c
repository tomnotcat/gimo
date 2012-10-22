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
#include "gimo-loader.h"
#include "gimo-pluginfo.h"

static void _test_loader_dlloader (void)
{
    GimoPluginfo *info;
    GimoDlloader *dlld;
    GimoLoader *loader;
    GimoPlugin *plugin;

    info = gimo_pluginfo_new ("test.plugin1",
                              "libtestplugin",
                              "test_plugin_new",
                              NULL, NULL, NULL,
                              NULL, NULL, NULL);
    dlld = gimo_dlloader_new ();
    loader = GIMO_LOADER (dlld);
    plugin = gimo_loader_load (loader, info);
    g_assert (plugin);
    g_object_unref (plugin);
    g_object_unref (dlld);
    g_object_unref (info);
}

static void _test_loader_manager (void)
{
    GimoPluginfo *info;
    GimoLoaderMgr *ldmgr;
    GimoDlloader *dlld;
    GimoPlugin *plugin;

    info = gimo_pluginfo_new ("test.plugin1",
                              "libtestplugin",
                              "test_plugin_new",
                              NULL, NULL, NULL,
                              NULL, NULL, NULL);
    ldmgr = gimo_loadermgr_new (NULL);

    dlld = gimo_dlloader_new ();
    gimo_loadermgr_register_loader (ldmgr, dlld);
    g_object_unref (dlld);

    plugin = gimo_loadermgr_load (ldmgr, info);
    g_assert (plugin);

    g_object_unref (plugin);
    g_object_unref (info);
    g_object_unref (ldmgr);
}

int main (int argc, char *argv[])
{
    g_type_init ();
    g_thread_init (NULL);

    _test_loader_dlloader ();
    _test_loader_manager ();

    return 0;
}
