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
#include "gimo-xmlarchive.h"
#include "gimo-error.h"
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
};

static void gimo_loadable_interface_init (GimoLoadableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GimoXmlArchive, gimo_xmlarchive, GIMO_TYPE_ARCHIVE,
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_LOADABLE,
                                                gimo_loadable_interface_init))

static void _parse_param_destroy (gpointer p)
{
    GParameter *param = p;
    g_value_unset (&param->value);
}

static struct _ParseFrame* _parse_frame_create (const gchar *id,
                                                GType type,
                                                GParamSpec *prop)
{
    struct _ParseFrame *f;

    f = g_malloc (sizeof *f);

    f->id = g_strdup (id);
    f->klass = NULL;
    f->obj_array = NULL;
    f->prop = prop;
    f->params = NULL;
    f->type = type;

    if (prop) {
        g_assert (!id && !type);

        if (G_PARAM_SPEC_VALUE_TYPE (prop) == GIMO_TYPE_OBJECT_ARRAY)
            f->obj_array = g_ptr_array_new_with_free_func (g_object_unref);
    }
    else {
        g_assert (type);

        if (G_TYPE_IS_OBJECT (type))
            f->klass = g_type_class_ref (type);
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

static void _parse_frame_add_param (struct _ParseFrame *f,
                                    const gchar *name,
                                    const GValue *value)
{
    g_assert (f->klass);

    if (NULL == f->params) {
        f->params = g_array_new (FALSE, FALSE, sizeof (GParameter));
        g_array_set_clear_func (f->params, _parse_param_destroy);
    }

    g_array_append_val (f->params, value);
}

static void _parse_frame_set_property (struct _ParseFrame *f,
                                       const gchar *name,
                                       const gchar *value)
{
    GParamSpec *prop = f->prop;
    GValue src_val = G_VALUE_INIT;
    GValue dest_val = G_VALUE_INIT;

    if (NULL == prop) {
        g_assert (f->klass && name);

        prop = g_object_class_find_property (f->klass, name);
        if (NULL == prop) {
            g_warning ("XmlArchive invalid property: %s", name);
            return;
        }

        name = prop->name;
    }

    g_value_init (&src_val, G_TYPE_STRING);
    g_value_init (&dest_val, G_PARAM_SPEC_VALUE_TYPE (prop));

    g_value_set_static_string (&src_val, value);

    if (g_value_transform (&src_val, &dest_val)) {
        _parse_frame_add_param (f, prop->name, &dest_val);
    }
    else {
        g_warning ("XmlArchive transform property error: %s", name);
    }
}

static void _parse_frame_set_object (struct _ParseFrame *f,
                                     gpointer p)
{
    if (f->klass) {
    }
    else if (f->obj_array) {
        g_assert (G_IS_OBJECT (p));
        g_ptr_array_add (f->obj_array, g_object_ref (p));
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

static const char* _gimo_xml_find_attr (const char **attr,
                                        const char *key)
{
    int i;

    for (i = 0; attr[i]; i += 2) {
        if (strcmp (attr[i], key) == 0)
            return attr[i + 1];
    }

    return NULL;
}

static void _gimo_xml_start_element (void *data,
                                     const char *el,
                                     const char **attr)
{
    struct _ParseContext *c = data;

    ++c->depth;

    if (c->frames->len > 0) {
        struct _ParseFrame *p, *f;

        p = g_ptr_array_index (c->frames, c->frames->len - 1);
        if (p) {
            if (p->klass) {
                /* Object property. */
                GParamSpec *prop;

                prop = g_object_class_find_property (p->klass, el);
                if (prop) {
                    g_ptr_array_add (c->frames,
                                     _parse_frame_create (NULL, 0, prop));
                }
                else {
                    g_warning ("XmlArchive invalid property: %s", el);
                }
            }
            else if (p->obj_array) {
                /* Object array element. */
                GType el_type;

                el_type = g_type_from_name (el);
                if (!el_type) {
                    gimo_set_error_full (GIMO_ERROR_NO_TYPE,
                                         "XmlArchive type not exists: %s", el);
                    c->error = TRUE;
                    return;
                }

                if (!G_TYPE_IS_OBJECT (el_type)) {
                    gimo_set_error_full (GIMO_ERROR_INVALID_TYPE,
                                         "XmlArchive not object type: %s", el);
                    c->error = TRUE;
                    return;
                }

                g_ptr_array_add (c->frames,
                                 _parse_frame_create (NULL, el_type, NULL));

                f = g_ptr_array_index (c->frames, c->frames->len - 1);
                while (*attr) {
                    _parse_frame_set_property (f, attr[0], attr[1]);
                    attr += 2;
                }
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

            val = _gimo_xml_find_attr (attr, "type");
            if (NULL == val) {
                gimo_set_error_full (GIMO_ERROR_NO_ATTRIBUTE,
                                     "XmlArchive object type not found");
                c->error = TRUE;
                return;
            }

            type = g_type_from_name (val);
            if (!type) {
                gimo_set_error_full (GIMO_ERROR_NO_TYPE,
                                     "XmlArchive type not exists: %s", val);
                c->error = TRUE;
                return;
            }

            if (!G_TYPE_IS_OBJECT (type)) {
                gimo_set_error_full (GIMO_ERROR_INVALID_TYPE,
                                     "XmlArchive not object type: %s", val);
                c->error = TRUE;
                return;
            }

            val = _gimo_xml_find_attr (attr, "id");
            if (NULL == val) {
                gimo_set_error_full (GIMO_ERROR_NO_ATTRIBUTE,
                                     "XmlArchive object id not found");
                c->error = TRUE;
                return;
            }

            g_ptr_array_add (c->frames,
                             _parse_frame_create (val, type, NULL));
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

        version = _gimo_xml_find_attr (attr, "version");
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

        if (c->frames->len > 1) {
            p = g_ptr_array_index (c->frames, c->frames->len - 2);
            _parse_frame_set_object (p, obj);
        }
        else {
            if (!gimo_archive_add_object (c->archive, f->id, obj))
                c->error = TRUE;
        }

        g_object_unref (obj);
    }
    else if (f->obj_array) {
        if (c->frames->len > 1) {
            p = g_ptr_array_index (c->frames, c->frames->len - 2);
            _parse_frame_set_object (p, f->obj_array);
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
    struct _ParseFrame *f;

    f = g_ptr_array_index (c->frames, c->frames->len - 1);
    if (f && f->prop) {
        gchar *str = g_strndup (txt, len);
        _parse_frame_set_property (f, NULL, str);
        g_free (str);
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
            gimo_set_error_full (GIMO_ERROR_IMPORT,
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
}

GimoXmlArchive* gimo_xmlarchive_new (void)
{
    return g_object_new (GIMO_TYPE_XMLARCHIVE, NULL);
}
