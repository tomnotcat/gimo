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
#include "gimo-archive.h"

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
}

int main (int argc, char *argv[])
{
    g_type_init ();
    g_thread_init (NULL);

    _test_archive_common ();
    _test_archive_xml ();

    return 0;
}
