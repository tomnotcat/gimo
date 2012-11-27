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
#include "config.h"
#include "gimo-dlmodule.h"
#include "gimo-factory.h"
#include "gimo-loader.h"
#include "gimo-plugin.h"
#undef  _POSIX_C_SOURCE
#include </usr/include/python2.7/Python.h>

static void test_module_common (gboolean cached)
{
    GimoLoader *loader;
    GimoModule *module;
    GObject *plugin;
    GimoFactory *factory;
    GPtrArray *paths;

#ifdef HAVE_INTROSPECTION
    GModule *gmodule;
    GimoFactoryFunc new_module;
#endif

    if (cached)
        loader = gimo_loader_new_cached ();
    else
        loader = gimo_loader_new ();

    g_assert (gimo_loader_dup_paths (loader) == NULL);
    gimo_loader_add_paths (loader, g_getenv ("GIMO_PLUGIN_PATH"));
    paths = gimo_loader_dup_paths (loader);

    g_assert (paths && paths->len == 4);
    g_ptr_array_unref (paths);

    /* Dynamic library */
    g_assert (!gimo_loader_load (loader, "demo-plugin"));
    factory = gimo_factory_new ((GimoFactoryFunc) gimo_dlmodule_new, NULL);
    g_assert (gimo_loader_register (loader,
                                    NULL,
                                    factory));
    g_object_unref (factory);
    module = GIMO_MODULE (gimo_loader_load (loader, "demo-plugin"));

    if (cached) {
        g_assert (gimo_loader_load (loader, "demo-plugin") ==
                  GIMO_LOADABLE (module));
        g_object_unref (module);
    }
    else {
        GimoLoadable *m2 = gimo_loader_load (loader, "demo-plugin");
        g_assert (m2 != GIMO_LOADABLE (module));
        g_object_unref (m2);
    }

    g_assert (GIMO_IS_DLMODULE (module));
    plugin = gimo_module_resolve (module,
                                  "test_plugin_new",
                                  NULL,
                                  TRUE);
    g_assert (GIMO_IS_PLUGIN (plugin));
    g_object_unref (plugin);
    g_object_unref (module);

#ifdef HAVE_INTROSPECTION
    if (!cached) {
        gimo_loader_remove_paths (loader, g_getenv ("GIMO_PLUGIN_PATH"));
        g_assert (gimo_loader_dup_paths (loader) == NULL);
        g_assert (!gimo_loader_load (loader, "pymodule-1.0"));
        g_object_unref (loader);
        return;
    }

    /* Python module */
    g_assert (!gimo_loader_load (loader, "demo-plugin.py"));
    module = GIMO_MODULE (gimo_loader_load (loader, "pymodule-1.0"));
    g_assert (module);
    gmodule = _gimo_dlmodule_get_gmodule (GIMO_DLMODULE (module));
    g_assert (gmodule);
    g_assert (g_module_symbol (gmodule,
                               "gimo_pymodule_new",
                               (gpointer *) &new_module));
    factory = gimo_factory_new (new_module, NULL);
    g_assert (gimo_loader_register (loader, "py", factory));
    g_object_unref (factory);
    g_object_unref (module);
    module = GIMO_MODULE (gimo_loader_load (loader, "demo-plugin.py"));
    g_assert (module);
    plugin = gimo_module_resolve (module,
                                  "test_plugin_new",
                                  NULL,
                                  TRUE);
    g_assert (GIMO_IS_PLUGIN (plugin));
    g_object_unref (plugin);
    g_object_unref (module);

    /* JavaScript module */
    g_assert (!gimo_loader_load (loader, "demo-plugin.js"));
    module = GIMO_MODULE (gimo_loader_load (loader, "jsmodule-1.0"));
    g_assert (module);
    gmodule = _gimo_dlmodule_get_gmodule (GIMO_DLMODULE (module));
    g_assert (gmodule);
    g_assert (g_module_symbol (gmodule,
                               "gimo_jsmodule_new",
                               (gpointer *) &new_module));
    factory = gimo_factory_new (new_module, NULL);
    g_assert (gimo_loader_register (loader, "js", factory));
    g_object_unref (factory);
    g_object_unref (module);

    module = GIMO_MODULE (gimo_loader_load (loader, "demo-plugin.js"));
    g_assert (module);
    plugin = gimo_module_resolve (module,
                                  "test_plugin_new",
                                  NULL,
                                  TRUE);
    g_assert (GIMO_IS_PLUGIN (plugin));
    g_object_unref (plugin);
    g_object_unref (module);
#endif /* HAVE_INTROSPECTION */

    g_object_unref (loader);
}

int main (int argc, char *argv[])
{
    g_type_init ();
    g_thread_init (NULL);

    test_module_common (FALSE);
    test_module_common (TRUE);

    return 0;
}
