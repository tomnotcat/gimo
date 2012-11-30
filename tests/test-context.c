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
#include "gimo-binding.h"
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
	gimo_context_add_paths (context, TEST_PLUGIN_PATH);

    g_signal_connect (context,
                      "state-changed",
                      G_CALLBACK (_test_context_state_changed),
                      &param);

    array = g_ptr_array_new_with_free_func (g_object_unref);
    extpt = gimo_ext_point_new ("extpt1", "extptname1");
    g_ptr_array_add (array, extpt);
    plugin = gimo_plugin_new ("test.plugin1", NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, array, NULL);
    g_ptr_array_unref (array);
    g_assert (!gimo_context_query_plugin (context, "test.plugin1"));
    g_assert (gimo_context_install_plugin (context, NULL, plugin));
    g_assert (gimo_context_query_plugin (context, "test.plugin1") == plugin);
    g_object_unref (plugin);
    g_object_unref (plugin);
    g_assert (GIMO_PLUGIN_UNINSTALLED == param.old_state);
    g_assert (GIMO_PLUGIN_INSTALLED == param.new_state);
    g_assert (1 == param.count);

    plugin = gimo_plugin_new ("test.plugin1", NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL);
    g_assert (!gimo_context_install_plugin (context, NULL, plugin));
    g_assert (gimo_get_error () == GIMO_ERROR_CONFLICT);
    g_object_unref (plugin);
    plugin = gimo_plugin_new ("test.plugin2", NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL);
    g_assert (gimo_context_install_plugin (context, NULL, plugin));
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

static void _test_context_dlplugin (void)
{
    GimoContext *context;
    GPtrArray *exts;

    context = gimo_context_new ();
    gimo_context_add_paths (context, TEST_PLUGIN_PATH);

    g_assert (gimo_context_load_plugin (context,
                                        "demo-plugin.xml",
                                        NULL,
                                        FALSE) == 1);
    g_object_unref (context);

    context = gimo_context_new ();
    gimo_context_add_paths (context, TEST_PLUGIN_PATH);

    g_assert (!gimo_lookup_string (G_OBJECT (context), "dl_start"));

    g_assert (gimo_context_load_plugin (context,
                                        "demo-plugin.xml",
                                        NULL,
                                        TRUE) == 1);

    exts = gimo_context_query_extensions (context,
                                          "org.oren.test.extension1");
    g_assert (exts && 2 == exts->len);
    g_ptr_array_unref (exts);
    exts = gimo_context_query_extensions (context,
                                          "org.oren.test.extension2");
    g_assert (exts && 1 == exts->len);
    g_ptr_array_unref (exts);
    exts = gimo_context_query_extensions (context,
                                          "org.oren.test.extension3");
    g_assert (!exts);

    g_assert (gimo_lookup_string (G_OBJECT (context), "dl_start"));
    g_assert (!gimo_lookup_string (G_OBJECT (context), "dl_run"));
    g_assert (!gimo_lookup_string (G_OBJECT (context), "dl_stop"));

    gimo_context_run_plugins (context);
    g_assert (gimo_lookup_string (G_OBJECT (context), "dl_run"));

    g_assert (gimo_context_load_plugin (context,
                                        "demo-plugin.xml",
                                        NULL,
                                        FALSE) == 0);
    gimo_context_destroy (context);
    g_assert (gimo_lookup_string (G_OBJECT (context), "dl_stop"));
    g_object_unref (context);
}

static void _test_context_jsplugin (void)
{
    GimoContext *context;

    context = gimo_context_new ();
	gimo_context_add_paths (context, TEST_PLUGIN_PATH);
    g_assert (gimo_context_load_plugin (context,
                                        "plugins",
                                        NULL,
                                        FALSE) == 2);
    g_object_unref (context);

    context = gimo_context_new ();
	gimo_context_add_paths (context, TEST_PLUGIN_PATH);

#ifdef HAVE_INTROSPECTION
    g_assert (gimo_context_load_plugin (context,
                                        "jsmodule-1.0.xml",
                                        NULL,
                                        TRUE) == 1);

    g_assert (!gimo_lookup_string (G_OBJECT (context), "js_start"));

    g_assert (gimo_context_load_plugin (context,
                                        "plugins/plugin1.xml",
                                        NULL,
                                        TRUE) == 1);

    g_assert (gimo_lookup_string (G_OBJECT (context), "js_start"));
    g_assert (!gimo_lookup_string (G_OBJECT (context), "js_run"));
    g_assert (!gimo_lookup_string (G_OBJECT (context), "js_stop"));

    gimo_context_run_plugins (context);
    g_assert (gimo_lookup_string (G_OBJECT (context), "js_run"));

#else  /* !HAVE_INTROSPECTION */
    g_assert (gimo_context_load_plugin (context,
                                        "plugins/plugin1.xml",
                                        NULL,
                                        FALSE) == 1);
#endif /* !HAVE_INTROSPECTION */

    gimo_context_destroy (context);
    g_assert (gimo_lookup_string (G_OBJECT (context), "js_stop"));
    g_object_unref (context);
}

static void _test_context_pyplugin (void)
{
#ifdef HAVE_INTROSPECTION
    GimoContext *context;

    context = gimo_context_new ();
	gimo_context_add_paths (context, TEST_PLUGIN_PATH);

    g_assert (gimo_context_load_plugin (context,
                                        "pymodule-1.0.xml",
                                        NULL,
                                        TRUE) == 1);

    g_assert (!gimo_lookup_string (G_OBJECT (context), "py_start"));

    g_assert (gimo_context_load_plugin (context,
                                        "plugins/plugin2.xml",
                                        NULL,
                                        TRUE) == 1);

    g_assert (gimo_lookup_string (G_OBJECT (context), "py_start"));
    g_assert (!gimo_lookup_string (G_OBJECT (context), "py_run"));
    g_assert (!gimo_lookup_string (G_OBJECT (context), "py_stop"));

    gimo_context_run_plugins (context);
    g_assert (gimo_lookup_string (G_OBJECT (context), "py_run"));

#else  /* !HAVE_INTROSPECTION */
    g_assert (gimo_context_load_plugin (context,
                                        "plugins/plugin2.xml",
                                        NULL,
                                        FALSE) == 1);
#endif /* !HAVE_INTROSPECTION */

    g_assert (gimo_context_load_plugin (context,
                                        "plugins",
                                        NULL,
                                        FALSE) == 1);
    gimo_context_destroy (context);
    g_assert (gimo_lookup_string (G_OBJECT (context), "py_stop"));
    g_object_unref (context);
}

int main (int argc, char *argv[])
{
    g_type_init ();
    g_thread_init (NULL);

    _test_context_common ();
    _test_context_dlplugin ();
    _test_context_jsplugin ();

#ifndef G_OS_WIN32
    /* FIXME: Win32 seems to have a deadlock. */ 
    _test_context_pyplugin ();
#endif

    return 0;
}
