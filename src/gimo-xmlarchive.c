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
#include "config.h"
#include "gimo-xmlarchive.h"
#include "gimo-context.h"
#include "gimo-error.h"
#include "gimo-factory.h"
#include "gimo-loader.h"
#include "gimo-plugin.h"
#include "gimo-utils.h"
#include <ctype.h>
#include <expat.h>
#include <stdio.h>
#include <string.h>

#ifdef XML_LARGE_SIZE
# if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#  define XML_FMT_INT_MOD "I64"
# else
#  define XML_FMT_INT_MOD "ll"
# endif
#else
# define XML_FMT_INT_MOD "l"
#endif

struct _ParseContext {
    GimoArchive *archive;
    GPtrArray *frames;
    gint depth;
    gboolean error;
};

struct _ParseFrame {
    gchar *id;
    GObjectClass *klass;
    GPtrArray *obj_array;
    GParamSpec *prop;
    GArray *params;
    GType type;
    GType obj_array_type;
};

static void gimo_loadable_interface_init (GimoLoadableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GimoXmlArchive, gimo_xmlarchive, GIMO_TYPE_ARCHIVE,
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_LOADABLE,
                                                gimo_loadable_interface_init))

#define DEFINE_SSCANF(func_name, to_member, format) \
    static void _value_transform_##func_name (const GValue *src_value, \
                                              GValue *dest_value)      \
    { \
        sscanf (src_value->data[0].v_pointer, (format), \
                &dest_value->data[0].to_member); \
    } extern void glib_dummy_decl (void)

DEFINE_SSCANF (string_int,     v_int,    "%d");
DEFINE_SSCANF (string_uint,    v_uint,   "%u");
DEFINE_SSCANF (string_long,    v_long,   "%ld");
DEFINE_SSCANF (string_ulong,   v_ulong,  "%lu");
DEFINE_SSCANF (string_int64,   v_int64,  "%" G_GINT64_FORMAT);
DEFINE_SSCANF (string_uint64,  v_uint64, "%" G_GUINT64_FORMAT);
DEFINE_SSCANF (string_float,   v_float,  "%f");
DEFINE_SSCANF (string_double,  v_double, "%lf");

static void _value_transform_string_bool (const GValue *src_value,
                                          GValue *dest_value)
{
    if (strcmp (src_value->data[0].v_pointer, "TRUE") == 0)
        dest_value->data[0].v_int = TRUE;
    else
        dest_value->data[0].v_int = FALSE;
}

static void _value_transform_string_enum (const GValue *src_value,
                                          GValue *dest_value)
{
    GEnumClass *klass = g_type_class_ref (G_VALUE_TYPE (dest_value));
    GEnumValue *enum_value = NULL;
    gchar *str = g_strdup (src_value->data[0].v_pointer);
    gchar *name, *end;

    name = str;
    while (isblank (*name))
        ++name;

    if (*name) {
        end = name;
        while (*end && !isblank (*end))
            ++end;

        *end = '\0';

        enum_value = g_enum_get_value_by_name (klass, name);
    }

    if (enum_value) {
        dest_value->data[0].v_long = enum_value->value;
    }
    else {
        sscanf (src_value->data[0].v_pointer, "%ld",
                &dest_value->data[0].v_long);
    }

    g_free (str);
    g_type_class_unref (klass);
}

static void _value_transform_string_flags (const GValue *src_value,
                                           GValue *dest_value)
{
    GFlagsClass *klass = g_type_class_ref (G_VALUE_TYPE (dest_value));
    gchar *str = g_strdup (src_value->data[0].v_pointer);
    gchar *name, *end;
    GFlagsValue *flags_value;
    gboolean stop = FALSE;

    name = str;
    while (!stop) {
        while (isblank (*name) || *name == '|')
            ++name;

        if (!*name)
            break;

        end = name;
        while (*end && !isblank (*end) && *end != '|')
            ++end;

        if (*end)
            *end = '\0';
        else
            stop = TRUE;

        flags_value = g_flags_get_value_by_name (klass, name);
        if (flags_value) {
            dest_value->data[0].v_ulong |= flags_value->value;
        }
        else {
            glong value = 0;

            sscanf (name, "%ld", &value);
            dest_value->data[0].v_ulong |= value;
        }

        name = end + 1;
    }

    g_free (str);
    g_type_class_unref (klass);
}

static void _parse_param_destroy (gpointer p)
{
    GParameter *param = p;
    g_value_unset (&param->value);
}

static const char* _gimo_xml_find_attr (const char **attr,
                                        gint index,
                                        const char *key)
{
    gint i, j = index + index;

    for (i = 0; attr[i]; i += 2) {
        if (-1 == index || i == j) {
            if (strcmp (attr[i], key) == 0)
                return attr[i + 1];
        }
    }

    return NULL;
}

static GType _gimo_class_from_name (const gchar *name)
{
    GType type;

    type = g_type_from_name (name);

    if (!type)
        type = gimo_resolve_type_lazily (name);

    if (!type) {
        g_warning ("XmlArchive type not exists: %s", name);
        return 0;
    }

    if (!G_TYPE_IS_OBJECT (type)) {
        g_warning ("XmlArchive type not class: %s", name);
        return 0;
    }

    return type;
}

static void _parse_frame_add_param (struct _ParseFrame *f,
                                    const gchar *name,
                                    const GValue *value)
{
    GParameter param;

    g_assert (f->klass);

    if (NULL == f->params) {
        f->params = g_array_new (FALSE, FALSE, sizeof (GParameter));
        g_array_set_clear_func (f->params, _parse_param_destroy);
    }

    param.name = name;
    param.value = *value;
    g_array_append_val (f->params, param);
}

static void _parse_frame_set_property (struct _ParseFrame *f,
                                       GParamSpec *prop,
                                       gpointer object,
                                       const gchar *name,
                                       const gchar *value)
{
    GValue src_val = G_VALUE_INIT;
    GValue dest_val = G_VALUE_INIT;

    if (f->obj_array) {
        g_assert (!prop && G_IS_OBJECT (object) && !name && !value);
        g_ptr_array_add (f->obj_array, g_object_ref (object));
        return;
    }

    if (object) {
        GType type;

        g_assert (f->klass && !name && !value);

        type = G_PARAM_SPEC_VALUE_TYPE (prop);
        if (G_TYPE_IS_OBJECT (type)) {
            g_value_init (&dest_val, G_TYPE_OBJECT);
            g_value_set_object (&dest_val, object);
        }
        else if (G_TYPE_IS_BOXED (type)) {
            g_value_init (&dest_val, type);
            g_value_set_boxed (&dest_val, object);
        }
        else {
            g_warning ("XmlArchive invalid property type: %s",
                       g_type_name (type));
            return;
        }

        _parse_frame_add_param (f, prop->name, &dest_val);
        return;
    }

    if (NULL == prop) {
        g_assert (f->klass && name);

        prop = g_object_class_find_property (f->klass, name);
        if (NULL == prop) {
            g_warning ("XmlArchive invalid property: %s", name);
            return;
        }
    }

    g_value_init (&src_val, G_TYPE_STRING);
    g_value_init (&dest_val, G_PARAM_SPEC_VALUE_TYPE (prop));

    g_value_set_static_string (&src_val, value);

    if (g_value_transform (&src_val, &dest_val)) {
        _parse_frame_add_param (f, prop->name, &dest_val);
    }
    else {
        g_warning ("XmlArchive transform property error: %s", prop->name);
    }
}

static struct _ParseFrame* _parse_frame_create (const gchar *id,
                                                GType type,
                                                GParamSpec *prop,
                                                const gchar **attr)
{
    struct _ParseFrame *f;

    f = g_malloc (sizeof *f);

    f->id = g_strdup (id);
    f->klass = NULL;
    f->obj_array = NULL;
    f->obj_array_type = 0;
    f->prop = prop;
    f->params = NULL;
    f->type = type;

    if (prop) {
        g_assert (!id && !type);

        f->type = G_PARAM_SPEC_VALUE_TYPE (prop);
    }

    if (G_TYPE_IS_OBJECT (f->type)) {
        f->klass = g_type_class_ref (f->type);

        while (*attr) {
            _parse_frame_set_property (
                f, NULL, NULL, attr[0], attr[1]);

            attr += 2;
        }
    }
    else if (GIMO_TYPE_OBJECT_ARRAY == f->type) {
        const gchar *val;

        f->obj_array = g_ptr_array_new_with_free_func (g_object_unref);

        val = _gimo_xml_find_attr (attr, 0, "class");
        if (val)
            f->obj_array_type = _gimo_class_from_name (val);
    }

    return f;
}

static void _parse_frame_destroy (gpointer p)
{
    struct _ParseFrame *f = p;

    if (f) {
        if (f->obj_array)
            g_ptr_array_unref (f->obj_array);
        else if (f->klass)
            g_type_class_unref (f->klass);

        if (f->params)
            g_array_unref (f->params);

        g_free (f->id);
        g_free (f);
    }
}

static struct _ParseContext* _parse_context_create (GimoArchive *archive)
{
    struct _ParseContext *c;

    c = g_malloc (sizeof *c);
    c->archive = archive;
    c->frames = g_ptr_array_new_with_free_func (_parse_frame_destroy);
    c->depth = 0;
    c->error = FALSE;

    return c;
}

static void _parse_context_destroy (gpointer p)
{
    struct _ParseContext *c = p;

    g_ptr_array_unref (c->frames);
    g_free (c);
}

static void _gimo_xml_start_element (void *data,
                                     const char *el,
                                     const char **attr)
{
    struct _ParseContext *c = data;

    ++c->depth;

    if (c->error)
        return;

    if (c->frames->len > 0) {
        struct _ParseFrame *p, *f;

        p = g_ptr_array_index (c->frames, c->frames->len - 1);
        if (p) {
            if (p->klass) {
                /* Object property. */
                GParamSpec *prop;

                prop = g_object_class_find_property (p->klass, el);
                if (prop) {
                    f = _parse_frame_create (NULL, 0, prop, attr);
                    g_ptr_array_add (c->frames, f);
                }
                else {
                    g_warning ("XmlArchive invalid property: %s", el);
                }
            }
            else if (p->obj_array) {
                /* Object array element. */
                const gchar *val;
                GType el_type;

                val = _gimo_xml_find_attr (attr, 0, "class");
                if (val) {
                    el_type = _gimo_class_from_name (val);

                    if (!el_type)
                        return;

                    attr += 2;
                }
                else {
                    el_type = p->obj_array_type;
                    if (!el_type) {
                        g_warning ("XmlArchive element type unknown: %s", el);
                        return;
                    }
                }

                f = _parse_frame_create (NULL, el_type, NULL, attr);
                g_ptr_array_add (c->frames, f);
            }
            else {
                g_warning ("XmlArchive invalid element: %s", el);
            }
        }
        else {
            /* Object begin. */
            const gchar *val;
            GType type;

            if (strcmp (el, "object")) {
                gimo_set_error_full (GIMO_ERROR_INVALID_ATTRIBUTE,
                                     "XmlArchive invalid element: %s", el);
                c->error = TRUE;
                return;
            }

            val = _gimo_xml_find_attr (attr, 0, "class");
            if (NULL == val) {
                g_warning ("XmlArchive class attrubite not found");
                return;
            }

            type = _gimo_class_from_name (val);
            if (!type)
                return;

            attr += 2;

            val = _gimo_xml_find_attr (attr, 0, "id");
            if (val)
                attr += 2;

            f = _parse_frame_create (val, type, NULL, attr);
            g_ptr_array_add (c->frames, f);
        }
    }
    else {
        /* Root node. */
        const gchar *version;

        if (strcmp (el, "archive")) {
            gimo_set_error_full (GIMO_ERROR_INVALID_ATTRIBUTE,
                                 "XmlArchive invalid root name: %s", el);
            c->error = TRUE;
            return;
        }

        version = _gimo_xml_find_attr (attr, -1, "version");
        if (NULL == version) {
            gimo_set_error_full (GIMO_ERROR_NO_ATTRIBUTE,
                                 "XmlArchive version not found");
            c->error = TRUE;
            return;
        }

        if (strcmp (version, "1.0")) {
            gimo_set_error_full (GIMO_ERROR_INVALID_ATTRIBUTE,
                                 "XmlArchive invalid version: %s", version);
            c->error = TRUE;
            return;
        }

        g_ptr_array_add (c->frames, NULL);
    }
}

static void _gimo_xml_end_element (void *data,
                                   const char *el)
{
    struct _ParseContext *c = data;
    struct _ParseFrame *f, *p;

    if (c->frames->len != c->depth) {
        --c->depth;
        return;
    }

    f = g_ptr_array_index (c->frames, c->frames->len - 1);
    if (c->error || !f)
        goto done;

    if (f->klass) {
        GObject *obj;
        GParameter *params;
        guint nparam;

        params = f->params ? (GParameter *) f->params->data : NULL;
        nparam = f->params ? f->params->len : 0;

        obj = g_object_newv (f->type, nparam, params);
        if (NULL == obj) {
            g_warning ("XmlArchive new object failed: %s",
                       g_type_name (f->type));
            goto done;
        }

        if (c->frames->len > 2) {
            p = g_ptr_array_index (c->frames, c->frames->len - 2);
            _parse_frame_set_property (p, f->prop, obj, NULL, NULL);
        }
        else {
            if (!gimo_archive_add_object (c->archive, f->id, obj))
                c->error = TRUE;
        }

        g_object_unref (obj);
    }
    else if (f->obj_array) {
        if (c->frames->len > 2) {
            p = g_ptr_array_index (c->frames, c->frames->len - 2);
            _parse_frame_set_property (p, f->prop, f->obj_array, NULL, NULL);
        }
    }

done:
    g_ptr_array_remove_index (c->frames, c->frames->len - 1);

    --c->depth;
}

static void _gimo_xml_handle_char (void *data,
                                   const char *txt,
                                   int len)
{
    struct _ParseContext *c = data;

    if (c->error)
        return;

    if (c->frames->len > 2) {
        struct _ParseFrame *p, *f;

        p = g_ptr_array_index (c->frames, c->frames->len - 2);
        f = g_ptr_array_index (c->frames, c->frames->len - 1);
        if (f->prop && !f->klass && !f->obj_array) {
            gchar *str = g_strndup (txt, len);
            _parse_frame_set_property (p, f->prop, NULL, NULL, str);
            g_free (str);
        }
    }
}

static gboolean _gimo_xmlarchive_read (GimoArchive *self,
                                       const gchar *file_name)
{
    FILE *fp;
    char buf[1024];
    size_t len;
    int done;
    int status;
    XML_Parser parser;
    struct _ParseContext *context;
    gboolean error;

    fp = fopen (file_name, "rb");
    if (NULL == fp)
        gimo_set_error_return_val (GIMO_ERROR_OPEN_FILE, FALSE);

    parser = XML_ParserCreate (NULL);

    context = _parse_context_create (GIMO_ARCHIVE (self));
    XML_SetUserData (parser, context);

    XML_SetElementHandler (parser,
                           _gimo_xml_start_element,
                           _gimo_xml_end_element);

    XML_SetCharacterDataHandler (parser,
                                 _gimo_xml_handle_char);

    do {
        len = fread (buf, 1, sizeof (buf), fp);
        done = len < sizeof(buf);
        status = XML_Parse (parser, buf, len, done);

        if (XML_STATUS_ERROR == status) {
            gimo_set_error_full (GIMO_ERROR_LOAD,
                                 "XmlArchive parse error: %s at line %"
                                 XML_FMT_INT_MOD "u\n",
                                 XML_ErrorString (XML_GetErrorCode (parser)),
                                 XML_GetCurrentLineNumber (parser));
            break;
        }

        if (context->error)
            break;
    } while (!done);

    error = context->error;
    g_assert (context->frames->len == 0);
    XML_ParserFree (parser);
    _parse_context_destroy (context);
    fclose (fp);

    return (XML_STATUS_OK == status) && !error;
}

static gboolean _gimo_xmlarchive_save (GimoArchive *self,
                                       const gchar *file_name)
{
    return FALSE;
}

static void gimo_loadable_interface_init (GimoLoadableInterface *iface)
{
    iface->load = (GimoLoadableLoadFunc) _gimo_xmlarchive_read;
}

static void gimo_xmlarchive_init (GimoXmlArchive *self)
{
}

static void gimo_xmlarchive_finalize (GObject *gobject)
{
    G_OBJECT_CLASS (gimo_xmlarchive_parent_class)->finalize (gobject);
}

static void gimo_xmlarchive_class_init (GimoXmlArchiveClass *klass)
{
    GObjectClass *gobject_class;
    GimoArchiveClass *archive_class;

    gobject_class = G_OBJECT_CLASS (klass);
    archive_class = GIMO_ARCHIVE_CLASS (klass);

    gobject_class->finalize = gimo_xmlarchive_finalize;

    archive_class->read = _gimo_xmlarchive_read;
    archive_class->save = _gimo_xmlarchive_save;

    /* Register value transformation functions. */
    g_value_register_transform_func (G_TYPE_STRING, G_TYPE_CHAR,
                                     _value_transform_string_int);

    g_value_register_transform_func (G_TYPE_STRING, G_TYPE_UCHAR,
                                     _value_transform_string_uint);

    g_value_register_transform_func (G_TYPE_STRING, G_TYPE_BOOLEAN,
                                     _value_transform_string_bool);

    g_value_register_transform_func (G_TYPE_STRING, G_TYPE_INT,
                                     _value_transform_string_int);

    g_value_register_transform_func (G_TYPE_STRING, G_TYPE_UINT,
                                     _value_transform_string_uint);

    g_value_register_transform_func (G_TYPE_STRING, G_TYPE_LONG,
                                     _value_transform_string_long);

    g_value_register_transform_func (G_TYPE_STRING, G_TYPE_ULONG,
                                     _value_transform_string_ulong);

    g_value_register_transform_func (G_TYPE_STRING, G_TYPE_INT64,
                                     _value_transform_string_int64);

    g_value_register_transform_func (G_TYPE_STRING, G_TYPE_UINT64,
                                     _value_transform_string_uint64);

    g_value_register_transform_func (G_TYPE_STRING, G_TYPE_FLOAT,
                                     _value_transform_string_float);

    g_value_register_transform_func (G_TYPE_STRING, G_TYPE_DOUBLE,
                                     _value_transform_string_double);

    g_value_register_transform_func (G_TYPE_STRING, G_TYPE_ENUM,
                                     _value_transform_string_enum);

    g_value_register_transform_func (G_TYPE_STRING, G_TYPE_FLAGS,
                                     _value_transform_string_flags);

}

GimoXmlArchive* gimo_xmlarchive_new (void)
{
    return g_object_new (GIMO_TYPE_XMLARCHIVE, NULL);
}

static gboolean _gimo_xmlarchive_plugin_start (GimoPlugin *self)
{
    GimoContext *context = NULL;
    GimoLoader *loader = NULL;
    GimoFactory *factory = NULL;
    gboolean result = FALSE;

    do {
        context = gimo_plugin_query_context (self);
        if (NULL == context)
            break;

        loader = gimo_safe_cast (
            gimo_context_resolve_extpoint (
                context, "org.gimo.core.loader.archive"),
            GIMO_TYPE_LOADER);

        if (NULL == loader)
            break;

        factory = gimo_factory_new ((GimoFactoryFunc) gimo_xmlarchive_new,
                                    NULL);
        result = gimo_loader_register (loader, "xml", factory);
    } while (0);

    if (factory)
        g_object_unref (factory);

    if (loader)
        g_object_unref (loader);

    if (context)
        g_object_unref (context);

    return result;
}

GObject* gimo_xmlarchive_plugin (GimoPlugin *plugin)
{
    g_signal_connect (plugin,
                      "start",
                      G_CALLBACK (_gimo_xmlarchive_plugin_start),
                      NULL);

    return g_object_ref (plugin);
}
