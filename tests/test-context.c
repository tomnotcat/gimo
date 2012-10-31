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
#include "gimo-context.h"
#include "gimo-error.h"
#include "gimo-extpoint.h"
#include "gimo-pluginfo.h"
#include <string.h>

struct _StateChange {
    GimoPluginState old_state;
    GimoPluginState new_state;
    gint count;
};

static void _test_context_state_changed (GimoContext *ctx,
                                         GimoPluginfo *info,
                                         GimoPluginState old_state,
                                         GimoPluginState new_state,
                                         struct _StateChange *data)
{
    g_assert (gimo_pluginfo_get_state (info) == new_state);

    data->old_state = old_state;
    data->new_state = new_state;
    data->count++;
}

static void _test_context_common (void)
{
    GimoContext *ctx;
    GimoPluginfo *info;
    GimoExtPoint *extpt;
    GPtrArray *array;

    struct _StateChange param = {
        GIMO_PLUGIN_UNINSTALLED,
        GIMO_PLUGIN_UNINSTALLED,
        0,
    };

    ctx = gimo_context_new ();

    g_signal_connect (ctx,
                      "state-changed",
                      G_CALLBACK (_test_context_state_changed),
                      &param);

    array = g_ptr_array_new_with_free_func (g_object_unref);
    extpt = gimo_extpoint_new ("extpt1", "extptname1");
    g_ptr_array_add (array, extpt);
    info = gimo_pluginfo_new ("test.plugin1", NULL, NULL, NULL,
                              NULL, NULL, NULL, array, NULL);
    g_ptr_array_unref (array);
    g_assert (!gimo_context_query_plugin (ctx, "test.plugin1"));
    g_assert (gimo_context_install_plugin (ctx, info));
    g_assert (gimo_context_query_plugin (ctx, "test.plugin1") == info);
    g_object_unref (info);
    g_object_unref (info);
    g_assert (GIMO_PLUGIN_UNINSTALLED == param.old_state);
    g_assert (GIMO_PLUGIN_INSTALLED == param.new_state);
    g_assert (1 == param.count);

    info = gimo_pluginfo_new ("test.plugin1", NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL);
    g_assert (!gimo_context_install_plugin (ctx, info));
    g_assert (gimo_get_error () == GIMO_ERROR_CONFLICT);
    g_object_unref (info);
    info = gimo_pluginfo_new ("test.plugin2", NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL);
    g_assert (gimo_context_install_plugin (ctx, info));
    g_assert (GIMO_PLUGIN_UNINSTALLED == param.old_state);
    g_assert (GIMO_PLUGIN_INSTALLED == param.new_state);
    g_assert (2 == param.count);
    g_object_unref (info);

    g_assert (gimo_context_query_plugin (ctx, "test.plugin2") == info);
    g_object_unref (info);

    array = gimo_context_query_plugins (ctx, "hello");
    g_assert (NULL == array);

    array = gimo_context_query_plugins (ctx, NULL);
    g_assert (2 == array->len);
    g_ptr_array_unref (array);

    if (1) {
        guint i;
        const gchar *plugin_id = "test.plugin";
        gchar id_prefix[32] = { 0 };

        for (i = 0; i < strlen (plugin_id); ++i) {
            id_prefix[i] = plugin_id[i];
            array = gimo_context_query_plugins (ctx, id_prefix);
            g_assert (2 == array->len);
            g_ptr_array_unref (array);
        }
    }

    array = gimo_context_query_plugins (ctx, "test.plugin2");
    g_assert (1 == array->len);
    g_ptr_array_unref (array);

    extpt = gimo_context_query_extpoint (ctx, "test.plugin1.extpt1");
    g_assert (extpt);
    info = gimo_extpoint_query_pluginfo (extpt);
    g_assert (info);
    g_object_unref (info);

    gimo_context_uninstall_plugin (ctx, "test.plugin1");
    g_assert (!gimo_context_query_plugin (ctx, "test.plugin1"));
    g_assert (!gimo_context_query_extpoint (ctx, "test.plugin1.extpt1"));
    g_assert (GIMO_PLUGIN_INSTALLED == param.old_state);
    g_assert (GIMO_PLUGIN_UNINSTALLED == param.new_state);
    g_assert (3 == param.count);

    g_assert (!gimo_extpoint_query_pluginfo (extpt));
    g_object_unref (extpt);
    g_object_unref (ctx);
    g_assert (3 == param.count);
}

int main (int argc, char *argv[])
{
    g_type_init ();
    g_thread_init (NULL);

    _test_context_common ();

    return 0;
}
