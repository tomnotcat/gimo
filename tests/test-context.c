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
#include "gimo-context.h"
#include "gimo-error.h"
#include "gimo-extpoint.h"
#include "gimo-loader.h"
#include "gimo-plugin.h"
#include <string.h>

struct _StateChange {
    GimoPluginState old_state;
    GimoPluginState new_state;
    gint count;
};

static void _test_context_state_changed (GimoContext *context,
                                         GimoPlugin *plugin,
                                         GimoPluginState old_state,
                                         GimoPluginState new_state,
                                         struct _StateChange *data)
{
    g_assert (gimo_plugin_get_state (plugin) == new_state);

    data->old_state = old_state;
    data->new_state = new_state;
    data->count++;
}

static void _test_context_common (void)
{
    GimoContext *context;
    GimoPlugin *plugin;
    GimoExtPoint *extpt;
    GPtrArray *array;

    struct _StateChange param = {
        GIMO_PLUGIN_UNINSTALLED,
        GIMO_PLUGIN_UNINSTALLED,
        0,
    };

    context = gimo_context_new ();

    g_signal_connect (context,
                      "state-changed",
                      G_CALLBACK (_test_context_state_changed),
                      &param);

    array = g_ptr_array_new_with_free_func (g_object_unref);
    extpt = gimo_ext_point_new ("extpt1", "extptname1");
    g_ptr_array_add (array, extpt);
    plugin = gimo_plugin_new ("test.plugin1", NULL, NULL, NULL,
                              NULL, NULL, NULL, array, NULL);
    g_ptr_array_unref (array);
    g_assert (!gimo_context_query_plugin (context, "test.plugin1"));
    g_assert (gimo_context_install_plugin (context, plugin));
    g_assert (gimo_context_query_plugin (context, "test.plugin1") == plugin);
    g_object_unref (plugin);
    g_object_unref (plugin);
    g_assert (GIMO_PLUGIN_UNINSTALLED == param.old_state);
    g_assert (GIMO_PLUGIN_INSTALLED == param.new_state);
    g_assert (1 == param.count);

    plugin = gimo_plugin_new ("test.plugin1", NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL);
    g_assert (!gimo_context_install_plugin (context, plugin));
    g_assert (gimo_get_error () == GIMO_ERROR_CONFLICT);
    g_object_unref (plugin);
    plugin = gimo_plugin_new ("test.plugin2", NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL);
    g_assert (gimo_context_install_plugin (context, plugin));
    g_assert (GIMO_PLUGIN_UNINSTALLED == param.old_state);
    g_assert (GIMO_PLUGIN_INSTALLED == param.new_state);
    g_assert (2 == param.count);
    g_object_unref (plugin);

    g_assert (gimo_context_query_plugin (context, "test.plugin2") == plugin);
    g_object_unref (plugin);

    array = gimo_context_query_plugins (context);
    /* With core plugins. */
    g_assert (array->len > 2);
    g_ptr_array_unref (array);

    extpt = gimo_context_query_extpoint (context, "test.plugin1.extpt1");
    g_assert (extpt);
    plugin = gimo_ext_point_query_plugin (extpt);
    g_assert (plugin);
    g_object_unref (plugin);

    gimo_context_uninstall_plugin (context, "test.plugin1");
    g_assert (!gimo_context_query_plugin (context, "test.plugin1"));
    g_assert (!gimo_context_query_extpoint (context, "test.plugin1.extpt1"));
    g_assert (GIMO_PLUGIN_INSTALLED == param.old_state);
    g_assert (GIMO_PLUGIN_UNINSTALLED == param.new_state);
    g_assert (3 == param.count);

    g_assert (!gimo_ext_point_query_plugin (extpt));
    g_object_unref (extpt);
    g_object_unref (context);
    g_assert (3 == param.count);
}

static void _test_context_load_plugin (void)
{
    GimoContext *context;
    GimoLoader *loader;

    context = gimo_context_new ();
    g_assert (gimo_context_load_plugin (context,
                                        "plugins/plugin1.xml",
                                        NULL,
                                        FALSE) == 1);
    g_object_unref (context);

    context = gimo_context_new ();
    g_assert (gimo_context_load_plugin (context,
                                        "plugins",
                                        NULL,
                                        FALSE) == 2);

    loader = gimo_context_resolve_extpoint (context,
                                            "org.gimo.core.loader.module",
                                            GIMO_TYPE_LOADER);
    g_assert (loader);

    g_assert (!gimo_loader_load (loader, "demo-plugin.py"));
    g_assert (!gimo_loader_load (loader, "demo-plugin.js"));

#ifdef HAVE_INTROSPECTION
    {
        GimoLoadable *module;

        g_assert (gimo_context_load_plugin (context,
                                            TEST_MODULE_PATH,
                                            NULL,
                                            TRUE) == 2);

        module = gimo_loader_load (loader, "demo-plugin.py");
        g_assert (module);
        g_object_unref (module);

        module = gimo_loader_load (loader, "demo-plugin.js");
        g_assert (module);
        g_object_unref (module);
    }
#endif /* HAVE_INTROSPECTION */

    g_object_unref (loader);
    g_object_unref (context);
}

int main (int argc, char *argv[])
{
    g_type_init ();
    g_thread_init (NULL);

    _test_context_common ();
    _test_context_load_plugin ();

    return 0;
}
