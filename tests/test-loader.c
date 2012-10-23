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
#include "gimo-dlmodule.h"
#include "gimo-loader.h"
#include "gimo-plugin.h"

static void _test_loader_dlmodule (void)
{
    GimoModule *module;
    GObject *plugin;

    module = GIMO_MODULE (gimo_dlmodule_new ());
    g_assert (gimo_module_open (module, "testplugin"));
    plugin = gimo_module_resolve (module,
                                  "test_plugin_new",
                                  NULL);
    g_assert (GIMO_IS_PLUGIN (plugin));
    g_object_unref (plugin);
    g_assert (gimo_module_close (module));
    g_assert (!gimo_module_resolve (module,
                                    "test_plugin_new",
                                    NULL));
    g_object_unref (module);
}

static void _test_loader_pymodule (void)
{
}

static void _test_loader_jsmodule (void)
{
}

static void _test_loader_loader (void)
{
    GimoLoader *loader;
    GimoModule *module;
    GObject *plugin;

    loader = gimo_loader_new ();
    g_assert (gimo_loader_register (loader,
                                    NULL,
                                    (GimoModuleCtorFunc) gimo_dlmodule_new,
                                    NULL));
    module = gimo_loader_load (loader, "testplugin");
    g_assert (GIMO_IS_DLMODULE (module));
    plugin = gimo_module_resolve (module,
                                  "test_plugin_new",
                                  NULL);
    g_assert (GIMO_IS_PLUGIN (plugin));
    g_object_unref (plugin);
    g_object_unref (module);
    g_object_unref (loader);
}

int main (int argc, char *argv[])
{
    g_type_init ();
    g_thread_init (NULL);

    _test_loader_dlmodule ();
    _test_loader_pymodule ();
    _test_loader_jsmodule ();
    _test_loader_loader ();

    return 0;
}
