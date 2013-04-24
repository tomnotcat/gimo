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
#include "gimo-context.h"
#include "gimo-error.h"
#include "gimo-plugin.h"
#include <locale.h>

#define GIMO_LAUNCH_DEFAULT_DIR "plugins"

static void _load_plugin (GimoContext *context,
                          const gchar *file_path,
                          gboolean start)
{
    GPtrArray *plugins = NULL;

    if (!gimo_context_load_plugin (context, file_path,
                                   TRUE, NULL, &plugins))
    {
        gchar *err_str = gimo_dup_error_string ();

        g_warning ("Load plugin error: %s: %s",
                   file_path, err_str);

        g_free (err_str);
    }

    if (plugins) {
        if (start) {
            guint i;
            GimoPlugin *plugin;

            for (i = 0; i < plugins->len; ++i) {
                plugin = g_ptr_array_index (plugins, i);
                gimo_plugin_start (plugin, NULL);
            }
        }

        g_ptr_array_unref (plugins);
    }
}

int main (int argc, char *argv[])
{
    static gchar **starts = NULL;
    static gchar **files = NULL;
    static gint silent = 0;

    static GOptionEntry entries[] = {
        { "start", 's', 0, G_OPTION_ARG_STRING_ARRAY, &starts, "Startup plugins", NULL },
        { "silent", 'l', 0, G_OPTION_ARG_INT, &silent, "Run silently", NULL },
        { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &files, NULL, "FILE" },
        { NULL }
    };

    GError *error = NULL;
    GOptionContext *optctx;

    GimoContext *context = NULL;
    GimoPlugin *plugin;
    gchar *app_path;

    g_type_init();

#if !GLIB_CHECK_VERSION (2, 31, 0)
    g_thread_init (NULL);
#endif

    setlocale (LC_ALL, "");

    if (!silent)
        gimo_trace_error (TRUE);

    optctx = g_option_context_new ("gimo-launch");
    g_option_context_add_main_entries (optctx, entries, "");

    if (!g_option_context_parse (optctx, &argc, &argv, &error)) {
        g_warning ("option parsing failed: %s", error->message);
        g_clear_error (&error);
    }

    g_option_context_free (optctx);

    context = gimo_context_new ();
    app_path = g_path_get_dirname (argv[0]);
    gimo_context_add_paths (context, app_path);
    g_free (app_path);

    if (files) {
        gchar **it = files;

        while (*it) {
            _load_plugin (context, *it, !starts);
            ++it;
        }
    }
    else {
        _load_plugin (context, GIMO_LAUNCH_DEFAULT_DIR, !starts);
    }

    if (starts) {
        gchar **it = starts;

        while (*it) {
            plugin = gimo_context_query_plugin (context, *it);
            if (plugin) {
                if (!gimo_plugin_start (plugin, NULL)) {
                    gchar *err_str = gimo_dup_error_string ();

                    g_warning ("Start plugin error: %s: %s",
                               gimo_plugin_get_id (plugin),
                               err_str);

                    g_free (err_str);
                }
            }
            else {
                g_warning ("Plugin not exist: %s", *it);
            }

            ++it;
        }
    }

    gimo_context_run_plugins (context);

    gimo_context_destroy (context);
    g_object_unref (context);

    if (files)
        g_strfreev (files);

    if (starts)
        g_strfreev (starts);

    return 0;
}
