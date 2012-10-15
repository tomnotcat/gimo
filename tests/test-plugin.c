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
#include "gimo-import.h"
#include "gimo-plugin.h"
#include <string.h>

struct _Pluginfo {
    const gchar *id;
    const gchar *url;
    const gchar *entry;
    const gchar *name;
    const gchar *version;
    const gchar *provider;
    GSList *imports;
    GSList *extpoints;
    GSList *extensions;
};

static void _test_plugin (const struct _Pluginfo *info)
{
    GimoPlugin *plugin;
    GSList *it1, *it2;

    plugin = gimo_plugin_new (info->id,
                              info->url,
                              info->entry,
                              info->name,
                              info->version,
                              info->provider,
                              info->imports,
                              info->extpoints,
                              info->extensions);

    g_assert (gimo_plugin_get_state (plugin) == GIMO_PLUGIN_UNINSTALLED);
    g_assert (!strcmp (gimo_plugin_get_id (plugin), info->id));
    g_assert (!strcmp (gimo_plugin_get_url (plugin), info->url));
    g_assert (!strcmp (gimo_plugin_get_entry (plugin), info->entry));
    g_assert (!strcmp (gimo_plugin_get_name (plugin), info->name));
    g_assert (!strcmp (gimo_plugin_get_version (plugin), info->version));
    g_assert (!strcmp (gimo_plugin_get_provider (plugin), info->provider));

    if (info->imports) {
        it2 = gimo_plugin_get_imports (plugin);
        for (it1 = info->imports; it1 != NULL; it1 = it1->next) {
            g_assert (it1 != it2);
            g_assert (g_slist_find (it2, it1->data));
        }
    }
    else {
        g_assert (gimo_plugin_get_imports (plugin) == NULL);
    }

    if (info->extpoints) {
    }
    else {
        g_assert (gimo_plugin_get_extpoints (plugin) == NULL);
    }

    if (info->extensions) {
    }
    else {
        g_assert (gimo_plugin_get_extensions (plugin) == NULL);
    }

    g_object_unref (plugin);
}

int main (int argc, char *argv[])
{
    struct _Pluginfo info = {"myplugin",
                             "/plugins/myplugin.so",
                             "make_myplugin",
                             "hello",
                             "1.0",
                             "tom",
                             NULL,
                             NULL,
                             NULL};
    g_type_init ();
    g_thread_init (NULL);

    /* basic */
    _test_plugin (&info);

    /* imports */
    info.imports = g_slist_prepend (
        info.imports, gimo_import_new ("plugin1", "1.0", FALSE));
    g_assert (strcmp (gimo_import_get_plugin_id (info.imports->data),
                      "plugin1") == 0);
    g_assert (strcmp (gimo_import_get_version (info.imports->data),
                      "1.0") == 0);
    g_assert (!gimo_import_is_optional (info.imports->data));

    info.imports = g_slist_prepend (
        info.imports, gimo_import_new ("plugin2", "2.0", TRUE));
    g_assert (strcmp (gimo_import_get_plugin_id (info.imports->data),
                      "plugin2") == 0);
    g_assert (strcmp (gimo_import_get_version (info.imports->data),
                      "2.0") == 0);
    g_assert (gimo_import_is_optional (info.imports->data));

    _test_plugin (&info);

    g_slist_free_full (info.imports, g_object_unref);
    info.imports = NULL;

    /* extpoints */
    return 0;
}
