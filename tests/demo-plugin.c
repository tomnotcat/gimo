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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "gimo-datastore.h"
#include "gimo-plugin.h"
#include "gimo-signalbus.h"
#include <string.h>

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

enum {
    SIG_TESTBUS,
    LAST_SIGNAL
};

struct _TestPlugin {
    GimoPlugin parent_instance;
};

struct _TestPluginClass {
    GimoPluginClass parent_class;
    void (*test_bus) (TestPlugin *self);
};

G_DEFINE_TYPE (TestPlugin, test_plugin, GIMO_TYPE_PLUGIN)

static guint test_signals[LAST_SIGNAL] = { 0 };

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

    klass->test_bus = NULL;

    test_signals[SIG_TESTBUS] =
            g_signal_new ("test-bus",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET (TestPluginClass, test_bus),
                          NULL, NULL,
                          g_cclosure_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);
}

TestPlugin* test_plugin_new (void)
{
    return g_object_new (TEST_TYPE_PLUGIN, NULL);
}

GIMO_SIGNALBUS_BEGIN (TestPlugin, test_plugin, 1)
g_signal_new ("test-bus",
              G_OBJECT_CLASS_TYPE (gobject_class),
              G_SIGNAL_RUN_FIRST,
              GIMO_SIGNALBUS_CLASS_OFFSET,
              NULL, NULL,
              g_cclosure_marshal_VOID__VOID,
              G_TYPE_NONE, 0);
GIMO_SIGNALBUS_END

static void _test_plugin_test_run (GimoContext *context,
                                   GimoRunnable *run)
{
    gimo_runnable_run (run);
}

static void _test_plugin_test_bus (GimoSignalBus *bus,
                                   GimoContext *c)
{
    gimo_bind_string (G_OBJECT (c), "dl_bus", "dl_bus");
}

static gboolean _demo_plugin_start (GimoPlugin *p)
{
    GimoContext *c = gimo_plugin_query_context (p);
    TestPlugin *t = test_plugin_new ();
    GimoSignalBus *bus;
    gimo_bind_string (G_OBJECT (c), "dl_start", "dl_start");
    gimo_bind_object (G_OBJECT (c), "dl_object", G_OBJECT (t));
    g_assert (!test_plugin_get_bus (t, NULL));
    bus = test_plugin_get_bus (t, c);
    g_assert (test_plugin_get_bus (t, NULL) == bus);
    g_signal_connect (c,
                      "async-run",
                      G_CALLBACK (_test_plugin_test_run),
                      NULL);
    g_signal_connect (bus,
                      "test-bus",
                      G_CALLBACK (_test_plugin_test_bus),
                      c);
    g_signal_emit (t,
                   test_signals[SIG_TESTBUS],
                   0);
    g_object_unref (t);
    g_object_unref (c);
    return TRUE;
}

static void _demo_plugin_run (GimoPlugin *p)
{
    GimoContext *c = gimo_plugin_query_context (p);
    gimo_bind_string (G_OBJECT (c), "dl_run", "dl_run");
    g_object_unref (c);
}

static void _demo_plugin_stop (GimoPlugin *p)
{
    GimoContext *c = gimo_plugin_query_context (p);
    gimo_bind_string (G_OBJECT (c), "dl_stop", "dl_stop");
    g_object_unref (c);
}

static void _demo_plugin_save (GimoPlugin *p,
                               GimoDataStore *s)
{
    g_assert (!gimo_data_store_get_string (s, "demokey"));
    gimo_data_store_set_string (s, "demokey", "demodata");
}

static void _demo_plugin_restore (GimoPlugin *p,
                                  GimoDataStore *s)
{
    GimoContext *c = gimo_plugin_query_context (p);

    if (strcmp (gimo_data_store_get_string (s, "demokey"),
                "demodata") == 0)
    {
        gimo_bind_string (G_OBJECT (c), "dl_update", "dl_update");
    }

    g_object_unref (c);
}

GObject* demo_plugin (GimoPlugin *plugin)
{
    g_signal_connect (plugin,
                      "start",
                      G_CALLBACK (_demo_plugin_start),
                      NULL);

    g_signal_connect (plugin,
                      "run",
                      G_CALLBACK (_demo_plugin_run),
                      NULL);

    g_signal_connect (plugin,
                      "stop",
                      G_CALLBACK (_demo_plugin_stop),
                      NULL);

    g_signal_connect (plugin,
                      "save",
                      G_CALLBACK (_demo_plugin_save),
                      NULL);

    g_signal_connect (plugin,
                      "restore",
                      G_CALLBACK (_demo_plugin_restore),
                      NULL);

    return g_object_ref (plugin);
}
