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
#include "gimo-extconfig.h"
#include "gimo-extension.h"
#include "gimo-extpoint.h"
#include "gimo-plugin.h"
#include "gimo-require.h"
#include <string.h>

struct _PluginInfo {
    const gchar *id;
    const gchar *name;
    const gchar *version;
    const gchar *provider;
    const gchar *path;
    const gchar *module;
    const gchar *symbol;
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

static void _test_plugin_info (const struct _PluginInfo *p)
{
    GimoPlugin *plugin;

    plugin = gimo_plugin_new (p->id,
                              p->name,
                              p->version,
                              p->provider,
                              p->path,
                              p->module,
                              p->symbol,
                              p->requires,
                              p->extpoints,
                              p->extensions);
    g_assert (!strcmp (gimo_plugin_get_id (plugin), p->id));
    g_assert (!strcmp (gimo_plugin_get_name (plugin), p->name));
    g_assert (!strcmp (gimo_plugin_get_version (plugin), p->version));
    g_assert (!strcmp (gimo_plugin_get_provider (plugin), p->provider));
    g_assert (!strcmp (gimo_plugin_get_path (plugin), p->path));
    g_assert (!strcmp (gimo_plugin_get_module (plugin), p->module));
    g_assert (!strcmp (gimo_plugin_get_symbol (plugin), p->symbol));
    g_assert (_ptr_array_equal (p->requires,
                                gimo_plugin_get_requires (plugin)));
    g_assert (_ptr_array_equal (p->extpoints,
                                gimo_plugin_get_extpoints (plugin)));
    g_assert (_ptr_array_equal (p->extensions,
                                gimo_plugin_get_extensions (plugin)));
    g_assert (gimo_plugin_get_state (plugin) == GIMO_PLUGIN_UNINSTALLED);
    g_object_unref (plugin);
}

int main (int argc, char *argv[])
{
    struct _PluginInfo info = {"test.myplugin",
                               "hello",
                               "1.0",
                               "tom",
                               ".",
                               "mymodule",
                               "myplugin_new",
                               NULL,
                               NULL,
                               NULL};
    GimoRequire *req;
    GimoExtPoint *extpt;
    GimoExtension *ext;
    GimoExtConfig *cfg;
    GPtrArray *cfgs;

    g_type_init ();

    /* basic */
    _test_plugin_info (&info);

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

    _test_plugin_info (&info);

    g_ptr_array_unref (info.requires);
    info.requires = NULL;

    /* extension points */
    info.extpoints = g_ptr_array_new_with_free_func (g_object_unref);

    extpt = gimo_ext_point_new ("extpt1", "name1");
    g_ptr_array_add (info.extpoints, extpt);
    g_assert (!strcmp (gimo_ext_point_get_local_id (extpt), "extpt1"));
    g_assert (!strcmp (gimo_ext_point_get_name (extpt), "name1"));
    g_assert (gimo_ext_point_get_id (extpt) == NULL);
    g_assert (gimo_ext_point_query_plugin (extpt) == NULL);

    _test_plugin_info (&info);

    g_assert (!strcmp (gimo_ext_point_get_id (extpt), "test.myplugin.extpt1"));
    g_assert (gimo_ext_point_query_plugin (extpt) == NULL);
    g_ptr_array_unref (info.extpoints);
    info.extpoints = NULL;

    /* extensions */
    info.extensions = g_ptr_array_new_with_free_func (g_object_unref);

    cfgs = g_ptr_array_new_with_free_func (g_object_unref);
    cfg = gimo_ext_config_new ("sub1", "subval1", NULL);
    g_ptr_array_add (cfgs, cfg);
    cfg = gimo_ext_config_new ("sub2", "subval2", NULL);
    g_ptr_array_add (cfgs, cfg);
    cfg = gimo_ext_config_new ("cfg1", "cfgval1", cfgs);
    g_ptr_array_unref (cfgs);
    cfgs = g_ptr_array_new_with_free_func (g_object_unref);
    g_ptr_array_add (cfgs, cfg);
    cfg = gimo_ext_config_new ("cfg2", "cfgval2", NULL);
    g_ptr_array_add (cfgs, cfg);

    ext = gimo_extension_new ("ext1", "name2", "extp2", cfgs);
    g_ptr_array_unref (cfgs);

    g_assert (gimo_extension_get_config (ext, "cfg1"));
    g_assert (gimo_extension_get_config (ext, "cfg1.sub1"));
    g_assert (gimo_extension_get_config (ext, "cfg1.sub2"));
    g_assert (gimo_extension_get_config (ext, "cfg2"));
    g_assert (!gimo_extension_get_config (ext, "cfg2.sub1"));
    g_assert (!gimo_extension_get_config (ext, "cfg3"));
    cfgs = gimo_extension_get_configs (ext, NULL);
    g_assert (cfgs && 2 == cfgs->len);
    cfgs = gimo_extension_get_configs (ext, "cfg1");
    g_assert (cfgs && 2 == cfgs->len);
    g_assert (!gimo_extension_get_configs (ext, "cfg2"));
    g_assert (!strcmp (gimo_extension_get_config_value (ext, "cfg1"),
                       "cfgval1"));
    g_assert (!strcmp (gimo_extension_get_config_value (ext, "cfg2"),
                       "cfgval2"));
    g_assert (!strcmp (gimo_extension_get_config_value (ext, "cfg1.sub1"),
                       "subval1"));
    g_assert (!strcmp (gimo_extension_get_config_value (ext, "cfg1.sub2"),
                       "subval2"));
    g_assert (!gimo_extension_get_config_value (ext, "cfg3"));

    g_ptr_array_add (info.extensions, ext);
    g_assert (!strcmp (gimo_extension_get_local_id (ext), "ext1"));
    g_assert (!strcmp (gimo_extension_get_name (ext), "name2"));
    g_assert (!strcmp (gimo_extension_get_extpoint_id (ext), "extp2"));
    g_assert (gimo_extension_get_id (ext) == NULL);
    g_assert (gimo_extension_query_plugin (ext) == NULL);

    _test_plugin_info (&info);

    g_assert (!strcmp (gimo_extension_get_id (ext), "test.myplugin.ext1"));
    g_assert (gimo_extension_query_plugin (ext) == NULL);
    g_ptr_array_unref (info.extensions);
    info.extensions = NULL;

    return 0;
}
