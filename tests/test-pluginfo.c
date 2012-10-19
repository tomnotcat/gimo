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
#include "gimo-pluginfo.h"
#include "gimo-require.h"
#include <string.h>

struct _Pluginfo {
    const gchar *id;
    const gchar *url;
    const gchar *klass;
    const gchar *name;
    const gchar *version;
    const gchar *provider;
    GSList *requires;
    GSList *extpoints;
    GSList *extensions;
};

static void _test_pluginfo (const struct _Pluginfo *p)
{
    GimoPluginfo *info;
    GSList *it1, *it2;

    info = gimo_pluginfo_new (p->id,
                              p->url,
                              p->klass,
                              p->name,
                              p->version,
                              p->provider,
                              p->requires,
                              p->extpoints,
                              p->extensions);

    g_assert (!strcmp (gimo_pluginfo_get_identifier (info), p->id));
    g_assert (!strcmp (gimo_pluginfo_get_url (info), p->url));
    g_assert (!strcmp (gimo_pluginfo_get_klass (info), p->klass));
    g_assert (!strcmp (gimo_pluginfo_get_name (info), p->name));
    g_assert (!strcmp (gimo_pluginfo_get_version (info), p->version));
    g_assert (!strcmp (gimo_pluginfo_get_provider (info), p->provider));

    if (p->requires) {
        it2 = gimo_pluginfo_get_requires (info);
        for (it1 = p->requires; it1 != NULL; it1 = it1->next) {
            g_assert (it1 != it2);
            g_assert (g_slist_find (it2, it1->data));
        }
    }
    else {
        g_assert (gimo_pluginfo_get_requires (info) == NULL);
    }

    if (p->extpoints) {
    }
    else {
        g_assert (gimo_pluginfo_get_extpoints (info) == NULL);
    }

    if (p->extensions) {
    }
    else {
        g_assert (gimo_pluginfo_get_extensions (info) == NULL);
    }

    g_object_unref (info);
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
    _test_pluginfo (&info);

    /* requires */
    info.requires = g_slist_prepend (
        info.requires, gimo_require_new ("plugin1", "1.0", FALSE));
    g_assert (strcmp (gimo_require_get_plugin_id (info.requires->data),
                      "plugin1") == 0);
    g_assert (strcmp (gimo_require_get_version (info.requires->data),
                      "1.0") == 0);
    g_assert (!gimo_require_is_optional (info.requires->data));

    info.requires = g_slist_prepend (
        info.requires, gimo_require_new ("plugin2", "2.0", TRUE));
    g_assert (strcmp (gimo_require_get_plugin_id (info.requires->data),
                      "plugin2") == 0);
    g_assert (strcmp (gimo_require_get_version (info.requires->data),
                      "2.0") == 0);
    g_assert (gimo_require_is_optional (info.requires->data));

    _test_pluginfo (&info);

    g_slist_free_full (info.requires, g_object_unref);
    info.requires = NULL;

    /* extpoints */
    return 0;
}
