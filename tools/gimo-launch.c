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
#include "gimo-pluginfo.h"
#include <locale.h>

#define GIMO_LAUNCH_DEFAULT_DIR "plugins"

static void _install_plugins (GimoContext *context,
                              const gchar *file_name)
{
}

int main (int argc, char *argv[])
{
    static gchar **starts = NULL;
    static gchar **files = NULL;

    static GOptionEntry entries[] = {
        { "start", 's', 0, G_OPTION_ARG_STRING_ARRAY, &starts, "Startup plugins", NULL },
        { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &files, NULL, "FILE" },
        { NULL }
    };

    GError *error = NULL;
    GOptionContext *optctx;

    GimoContext *context = NULL;
    GimoPluginfo *plugin;

    g_type_init();

#if !GLIB_CHECK_VERSION (2, 31, 0)
    g_thread_init (NULL);
#endif

    setlocale (LC_ALL, "");

    optctx = g_option_context_new ("gimo-launch");
    g_option_context_add_main_entries (optctx, entries, "");

    if (!g_option_context_parse (optctx, &argc, &argv, &error)) {
        g_warning ("option parsing failed: %s", error->message);
        g_clear_error (&error);
    }

    g_option_context_free (optctx);

    context = gimo_context_new ();

    if (files) {
        gchar **it = files;

        while (*it) {
            _install_plugins (context, *it);
            ++it;
        }
    }
    else {
        _install_plugins (context, GIMO_LAUNCH_DEFAULT_DIR);
    }

    if (starts) {
        gchar **it = starts;

        while (*it) {
            plugin = gimo_context_query_plugin (context, *it);
            if (plugin) {
                if (!gimo_pluginfo_start (plugin)) {
                    gchar *err_str = gimo_dup_error_string ();

                    g_warning ("Start plugin error: %s: %d: %s",
                               *it, gimo_get_error (), err_str);

                    g_free (err_str);
                }
            }
            else {
                g_warning ("Plugin not exist: %s", *it);
            }

            ++it;
        }
    }
    else {
    }

    g_object_unref (context);

    if (files)
        g_strfreev (files);

    if (starts)
        g_strfreev (starts);

    return 0;
}
