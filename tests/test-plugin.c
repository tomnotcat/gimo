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
#include "gimo-plugin.h"

#define TEST_TYPE_PLUGIN (test_plugin_get_type())
#define TEST_PLUGIN(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TEST_TYPE_PLUGIN, TestPlugin))
#define TEST_IS_PLUGIN(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), TEST_TYPE_PLUGIN))
#define TEST_PLUGIN_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), TEST_TYPE_PLUGIN, TestPluginClass))
#define TEST_IS_PLUGIN_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), TEST_TYPE_PLUGIN))
#define TEST_PLUGIN_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TEST_TYPE_PLUGIN, TestPluginClass))

typedef struct _TestPlugin TestPlugin;
typedef struct _TestPluginClass TestPluginClass;

struct _TestPlugin {
    GimoPlugin parent_instance;
};

struct _TestPluginClass {
    GimoPluginClass parent_class;
};

G_DEFINE_TYPE (TestPlugin, test_plugin, GIMO_TYPE_PLUGIN)

static void test_plugin_init (TestPlugin *self)
{
}

static void test_plugin_finalize (GObject *gobject)
{
    G_OBJECT_CLASS (test_plugin_parent_class)->finalize (gobject);
}

static void test_plugin_class_init (TestPluginClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = test_plugin_finalize;
}

GimoPlugin* test_plugin_new (void)
{
    return g_object_new (TEST_TYPE_PLUGIN, NULL);
}
