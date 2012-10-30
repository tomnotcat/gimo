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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "gimo-xmlarchive.h"
#include "gimo-error.h"
#include <expat.h>
#include <stdio.h>

#ifdef XML_LARGE_SIZE
# if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#  define XML_FMT_INT_MOD "I64"
# else
#  define XML_FMT_INT_MOD "ll"
# endif
#else
# define XML_FMT_INT_MOD "l"
#endif

struct _GimoXmlArchivePrivate {
    int n;
};

static void gimo_loadable_interface_init (GimoLoadableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GimoXmlArchive, gimo_xmlarchive, GIMO_TYPE_ARCHIVE,
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_LOADABLE,
                                                gimo_loadable_interface_init))

static void _gimo_xml_start_element (void *data,
                                     const char *el,
                                     const char **attr)
{
}

static void _gimo_xml_end_element (void *data,
                                   const char *el)
{
}

static void _gimo_xml_handle_char (void *data,
                                   const char *txt,
                                   int len)
{
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

    fp = fopen (file_name, "rb");
    if (NULL == fp)
        gimo_set_error_return_val (GIMO_ERROR_OPEN_FILE, FALSE);

    parser = XML_ParserCreate (NULL);
    XML_SetUserData (parser, NULL);

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
                                 "Parse xml error: %s at line %"
                                 XML_FMT_INT_MOD "u\n",
                                 XML_ErrorString (XML_GetErrorCode (parser)),
                                 XML_GetCurrentLineNumber (parser));
            break;
        }
    } while (!done);

    XML_ParserFree (parser);
    fclose (fp);

    return (XML_STATUS_OK == status);
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
    GimoXmlArchivePrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_XMLARCHIVE,
                                              GimoXmlArchivePrivate);
    priv = self->priv;

    priv->n = 0;
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

    g_type_class_add_private (gobject_class,
                              sizeof (GimoXmlArchivePrivate));
}

GimoXmlArchive* gimo_xmlarchive_new (void)
{
    return g_object_new (GIMO_TYPE_XMLARCHIVE, NULL);
}
