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
#include "gimo-archive.h"
#include "gimo-dlmodule.h"
#include "gimo-loader.h"

#define TEST_TYPE_CONFIG (test_config_get_type())
#define TEST_CONFIG(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TEST_TYPE_CONFIG, TestConfig))

GType test_enum_get_type (void) G_GNUC_CONST;
#define TEST_TYPE_ENUM (test_enum_get_type ())

GType test_flags_get_type (void) G_GNUC_CONST;
#define TEST_TYPE_FLAGS (test_flags_get_type ())

typedef struct _TestConfig {
    GObject parent_instance;
    gint8 i8;
    guint8 u8;
    gboolean b;
    gint i32;
    guint u32;
    glong l32;
    gulong ul32;
    gint64 i64;
    guint64 u64;
    gint venum;
    guint vflags;
    gfloat f;
    gdouble d;
    gchar *s;
    GPtrArray *a;
    struct _TestConfig *o;
} TestConfig;

typedef struct _TestConfigClass {
    GObjectClass parent_class;
} TestConfigClass;

G_DEFINE_TYPE (TestConfig, test_config, G_TYPE_OBJECT)

enum {
    TEST_ENUM_0,
    TEST_ENUM_1,
    TEST_ENUM_2,
    TEST_ENUM_3
};

enum {
    TEST_FLAG_0 = 0,
    TEST_FLAG_1 = 1 << 0,
    TEST_FLAG_2 = 1 << 1,
    TEST_FLAG_3 = 1 << 2
};

enum {
    PROP_0,
    PROP_CHAR,
    PROP_UCHAR,
    PROP_BOOLEAN,
    PROP_INT,
    PROP_UINT,
    PROP_LONG,
    PROP_ULONG,
    PROP_INT64,
    PROP_UINT64,
    PROP_ENUM,
    PROP_FLAGS,
    PROP_FLOAT,
    PROP_DOUBLE,
    PROP_STRING,
    PROP_OBJECT,
    PROP_ARRAY
};

GType test_enum_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile)) {
        static const GEnumValue values[] = {
            { TEST_ENUM_0, "TEST_ENUM_0", "ENUM0" },
            { TEST_ENUM_1, "TEST_ENUM_1", "ENUM1" },
            { TEST_ENUM_2, "TEST_ENUM_2", "ENUM2" },
            { TEST_ENUM_3, "TEST_ENUM_3", "ENUM3" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id =
                g_enum_register_static (g_intern_static_string ("TestEnum"), values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

GType test_flags_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile)) {
        static const GFlagsValue values[] = {
            { TEST_FLAG_0, "TEST_FLAG_0", "FLAG0" },
            { TEST_FLAG_1, "TEST_FLAG_1", "FLAG1" },
            { TEST_FLAG_2, "TEST_FLAG_2", "FLAG2" },
            { TEST_FLAG_3, "TEST_FLAG_3", "FLAG3" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id =
                g_flags_register_static (g_intern_static_string ("TestFlags"), values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

static void test_config_init (TestConfig *self)
{
}

static void test_config_finalize (GObject *gobject)
{
    TestConfig *self = TEST_CONFIG (gobject);

    g_free (self->s);

    if (self->a)
        g_ptr_array_unref (self->a);

    if (self->o)
        g_object_unref (self->o);

    G_OBJECT_CLASS (test_config_parent_class)->finalize (gobject);
}

static void test_config_set_property (GObject *object,
                                      guint prop_id,
                                      const GValue *value,
                                      GParamSpec *pspec)
{
    TestConfig *self = TEST_CONFIG (object);

    switch (prop_id) {
    case PROP_CHAR:
        self->i8 = g_value_get_char (value);
        break;

    case PROP_UCHAR:
        self->u8 = g_value_get_uchar (value);
        break;

    case PROP_BOOLEAN:
        self->b = g_value_get_boolean (value);
        break;

    case PROP_INT:
        self->i32 = g_value_get_int (value);
        break;

    case PROP_UINT:
        self->u32 = g_value_get_uint (value);
        break;

    case PROP_LONG:
        self->l32 = g_value_get_long (value);
        break;

    case PROP_ULONG:
        self->ul32 = g_value_get_ulong (value);
        break;

    case PROP_INT64:
        self->i64 = g_value_get_int64 (value);
        break;

    case PROP_UINT64:
        self->u64 = g_value_get_uint64 (value);
        break;

    case PROP_ENUM:
        self->venum = g_value_get_enum (value);
        break;

    case PROP_FLAGS:
        self->vflags = g_value_get_flags (value);
        break;

    case PROP_FLOAT:
        self->f = g_value_get_float (value);
        break;

    case PROP_DOUBLE:
        self->d = g_value_get_double (value);
        break;

    case PROP_STRING:
        self->s = g_value_dup_string (value);
        break;

    case PROP_OBJECT:
        self->o = g_value_dup_object (value);
        break;

    case PROP_ARRAY:
        g_assert (0);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void test_config_get_property (GObject *object,
                                      guint prop_id,
                                      GValue *value,
                                      GParamSpec *pspec)
{
    TestConfig *self = TEST_CONFIG (object);

    switch (prop_id) {
    case PROP_CHAR:
        g_value_set_char (value, self->i8);
        break;

    case PROP_UCHAR:
        g_value_set_uchar (value, self->u8);
        break;

    case PROP_BOOLEAN:
        g_value_set_boolean (value, self->b);
        break;

    case PROP_INT:
        g_value_set_int (value, self->i32);
        break;

    case PROP_UINT:
        g_value_set_uint (value, self->u32);
        break;

    case PROP_LONG:
        g_value_set_long (value, self->l32);
        break;

    case PROP_ULONG:
        g_value_set_ulong (value, self->ul32);
        break;

    case PROP_INT64:
        g_value_set_int64 (value, self->i64);
        break;

    case PROP_UINT64:
        g_value_set_uint64 (value, self->u64);
        break;

    case PROP_ENUM:
        g_value_set_enum (value, self->venum);
        break;

    case PROP_FLAGS:
        g_value_set_flags (value, self->vflags);
        break;

    case PROP_FLOAT:
        g_value_set_float (value, self->f);
        break;

    case PROP_DOUBLE:
        g_value_set_double (value, self->d);
        break;

    case PROP_STRING:
        g_value_set_string (value, self->s);
        break;

    case PROP_OBJECT:
        g_value_set_object (value, self->o);
        break;

    case PROP_ARRAY:
        g_assert (0);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void test_config_class_init (TestConfigClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = test_config_finalize;
    gobject_class->set_property = test_config_set_property;
    gobject_class->get_property = test_config_get_property;

    g_object_class_install_property (
        gobject_class, PROP_CHAR,
        g_param_spec_char ("char", "char", "char",
                           G_MININT8, G_MAXINT8, 0,
                           G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_UCHAR,
        g_param_spec_uchar ("uchar", "uchar", "uchar",
                            0, G_MAXUINT8, 0,
                            G_PARAM_READWRITE |
                            G_PARAM_CONSTRUCT_ONLY |
                            G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_BOOLEAN,
        g_param_spec_boolean ("boolean", "boolean", "boolean",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_CONSTRUCT_ONLY |
                              G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_INT,
        g_param_spec_int ("int", "int", "int",
                          G_MININT32, G_MAXINT32, 0,
                          G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_UINT,
        g_param_spec_uint ("uint", "uint", "uint",
                           0, G_MAXUINT32, 0,
                           G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_LONG,
        g_param_spec_long ("long", "long", "long",
                           G_MININT32, G_MAXINT32, 0,
                           G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_ULONG,
        g_param_spec_ulong ("ulong", "ulong", "ulong",
                            0, G_MAXUINT32, 0,
                            G_PARAM_READWRITE |
                            G_PARAM_CONSTRUCT_ONLY |
                            G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_INT64,
        g_param_spec_int64 ("int64", "int64", "int64",
                            G_MININT64, G_MAXINT64, 0,
                            G_PARAM_READWRITE |
                            G_PARAM_CONSTRUCT_ONLY |
                            G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_UINT64,
        g_param_spec_uint64 ("uint64", "uint64", "uint64",
                             0, G_MAXUINT64, 0,
                             G_PARAM_READWRITE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_ENUM,
        g_param_spec_enum ("enum", "enum", "enum",
                           TEST_TYPE_ENUM,
                           0,
                           G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_FLAGS,
        g_param_spec_flags ("flags", "flags", "flags",
                            TEST_TYPE_FLAGS,
                            0,
                            G_PARAM_READWRITE |
                            G_PARAM_CONSTRUCT_ONLY |
                            G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_FLOAT,
        g_param_spec_float ("float", "float", "float",
                            0.0, G_MAXFLOAT, 0.0,
                            G_PARAM_READWRITE |
                            G_PARAM_CONSTRUCT_ONLY |
                            G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_DOUBLE,
        g_param_spec_double ("double", "double", "double",
                             0.0, G_MAXDOUBLE, 0.0,
                             G_PARAM_READWRITE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_STRING,
        g_param_spec_string ("string", "string", "string",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_OBJECT,
        g_param_spec_object ("object", "object", "object",
                             TEST_TYPE_CONFIG,
                             G_PARAM_READWRITE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_ARRAY,
        g_param_spec_boxed ("array", "array", "array",
                            GIMO_TYPE_OBJECT_ARRAY,
                            G_PARAM_READWRITE |
                            G_PARAM_CONSTRUCT_ONLY |
                            G_PARAM_STATIC_STRINGS));

}

static void _test_archive_common (void)
{
    GimoArchive *archive;
    GObject *object;

    archive = gimo_archive_new ();
    object = G_OBJECT (gimo_archive_new ());
    g_assert (gimo_archive_add_object (archive, "1", object));
    g_assert (!gimo_archive_add_object (archive, "1", object));
    g_assert (!gimo_archive_query_object (archive, "2"));
    g_assert (gimo_archive_add_object (archive, "2", object));
    g_object_unref (object);
    object = gimo_archive_query_object (archive, "1");
    g_assert (object);
    g_object_unref (object);
    gimo_archive_remove_object (archive, "1");
    g_assert (!gimo_archive_query_object (archive, "1"));
    g_object_unref (archive);
}

static void _test_archive_xml (void)
{
    GimoLoader *loader;
    GimoModule *module;
    GObject *archive;
    TestConfig *config;

    /* Register types. */
    g_type_name (TEST_TYPE_CONFIG);

    loader = gimo_loader_new_cached ();
    g_assert (gimo_loader_register (loader,
                                    NULL,
                                    (GimoLoadableCtorFunc) gimo_dlmodule_new,
                                    NULL));
    module = GIMO_MODULE (gimo_loader_load (loader, "xmlarchive-1.0"));
    archive = gimo_module_resolve (module,
                                   "gimo_xmlarchive_new",
                                   NULL);
    g_assert (archive);
    g_assert (gimo_archive_read (GIMO_ARCHIVE (archive),
                                 "test-archive.xml"));
    config = TEST_CONFIG (gimo_archive_query_object (GIMO_ARCHIVE (archive),
                                                     "config1"));
    g_assert (config);
    g_object_unref (config);
    g_object_unref (archive);
    g_object_unref (module);
    g_object_unref (loader);
}

int main (int argc, char *argv[])
{
    g_type_init ();
    g_thread_init (NULL);

    _test_archive_common ();
    _test_archive_xml ();

    return 0;
}
