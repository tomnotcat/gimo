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
#include "gimo-extension.h"
#include "gimo-extpoint.h"
#include "gimo-pluginfo.h"
#include "gimo-require.h"
#include <string.h>

struct _Pluginfo {
    const gchar *id;
    const gchar *name;
    const gchar *version;
    const gchar *provider;
    const gchar *uri;
    const gchar *module;
    GPtrArray *requires;
    GPtrArray *extpoints;
    GPtrArray *extensions;
};

static gboolean _ptr_array_equal (GPtrArray *a, GPtrArray *b)
{
    if (a && b) {
        guint i;

        if (a->len != b->len)
            return FALSE;

        for (i = 0; i < a->len; ++i) {
            if (g_ptr_array_index (a, i) != g_ptr_array_index (b, i))
                return FALSE;
        }

        return TRUE;
    }

    return a == b;
}

static void _test_pluginfo (const struct _Pluginfo *p)
{
    GimoPluginfo *info;

    info = gimo_pluginfo_new (p->id,
                              p->name,
                              p->version,
                              p->provider,
                              p->uri,
                              p->module,
                              p->requires,
                              p->extpoints,
                              p->extensions);
    g_assert (!strcmp (gimo_pluginfo_get_id (info), p->id));
    g_assert (!strcmp (gimo_pluginfo_get_name (info), p->name));
    g_assert (!strcmp (gimo_pluginfo_get_version (info), p->version));
    g_assert (!strcmp (gimo_pluginfo_get_provider (info), p->provider));
    g_assert (!strcmp (gimo_pluginfo_get_uri (info), p->uri));
    g_assert (!strcmp (gimo_pluginfo_get_module (info), p->module));
    g_assert (_ptr_array_equal (p->requires,
                                gimo_pluginfo_get_requires (info)));
    g_assert (_ptr_array_equal (p->extpoints,
                                gimo_pluginfo_get_extpoints (info)));
    g_assert (_ptr_array_equal (p->extensions,
                                gimo_pluginfo_get_extensions (info)));
    g_assert (gimo_pluginfo_get_state (info) == GIMO_PLUGIN_UNINSTALLED);
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
    GimoRequire *req;
    GimoExtPoint *extpt;
    GimoExtension *ext;

    g_type_init ();
    g_thread_init (NULL);

    /* basic */
    _test_pluginfo (&info);

    /* requires */
    info.requires = g_ptr_array_new_with_free_func (g_object_unref);
    req = gimo_require_new ("plugin1", "1.0", FALSE);
    g_ptr_array_add (info.requires, req);
    g_assert (!strcmp (gimo_require_get_plugin_id (req), "plugin1"));
    g_assert (!strcmp (gimo_require_get_version (req), "1.0"));
    g_assert (!gimo_require_is_optional (req));

    req = gimo_require_new ("plugin2", "2.0", TRUE);
    g_ptr_array_add (info.requires, req);
    g_assert (!strcmp (gimo_require_get_plugin_id (req), "plugin2"));
    g_assert (!strcmp (gimo_require_get_version (req), "2.0"));
    g_assert (gimo_require_is_optional (req));

    _test_pluginfo (&info);

    g_ptr_array_unref (info.requires);
    info.requires = NULL;

    /* extension points */
    info.extpoints = g_ptr_array_new_with_free_func (g_object_unref);

    extpt = gimo_extpoint_new ("extpt1", "name1");
    g_ptr_array_add (info.extpoints, extpt);
    g_assert (!strcmp (gimo_extpoint_get_local_id (extpt), "extpt1"));
    g_assert (!strcmp (gimo_extpoint_get_name (extpt), "name1"));
    g_assert (gimo_extpoint_get_id (extpt) == NULL);
    g_assert (gimo_extpoint_query_pluginfo (extpt) == NULL);

    _test_pluginfo (&info);

    g_assert (!strcmp (gimo_extpoint_get_id (extpt), "myplugin.extpt1"));
    g_assert (gimo_extpoint_query_pluginfo (extpt) == NULL);
    g_ptr_array_unref (info.extpoints);
    info.extpoints = NULL;

    /* extensions */
    info.extensions = g_ptr_array_new_with_free_func (g_object_unref);

    ext = gimo_extension_new ("ext1", "name2", "extp2", NULL);
    g_ptr_array_add (info.extensions, ext);
    g_assert (!strcmp (gimo_extension_get_local_id (ext), "ext1"));
    g_assert (!strcmp (gimo_extension_get_name (ext), "name2"));
    g_assert (!strcmp (gimo_extension_get_extpoint_id (ext), "extp2"));
    g_assert (gimo_extension_get_id (ext) == NULL);
    g_assert (gimo_extension_query_pluginfo (ext) == NULL);

    _test_pluginfo (&info);

    g_assert (!strcmp (gimo_extension_get_id (ext), "myplugin.ext1"));
    g_assert (gimo_extension_query_pluginfo (ext) == NULL);
    g_ptr_array_unref (info.extensions);
    info.extensions = NULL;

    return 0;
}
