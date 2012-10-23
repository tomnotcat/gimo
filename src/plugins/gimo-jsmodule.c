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
#include "gimo-jsmodule.h"
#include "gimo-pluginfo.h"
#include <gmodule.h>

static void gimo_loader_interface_init (GimoLoaderInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GimoJsmodule, gimo_jsmodule, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_LOADER,
                                                gimo_loader_interface_init))

static gboolean _gimo_jsmodule_check (GimoLoader *loader,
                                      GimoPluginfo *info)
{
    return FALSE;
}

static GimoPlugin* _gimo_jsmodule_load (GimoLoader *loader,
                                        GimoPluginfo *info)
{
    return NULL;
}

static void gimo_loader_interface_init (GimoLoaderInterface *iface)
{
    iface->check = _gimo_jsmodule_check;
    iface->load = _gimo_jsmodule_load;
}

static void gimo_jsmodule_init (GimoJsmodule *self)
{
}

static void gimo_jsmodule_class_init (GimoJsmoduleClass *klass)
{
}

GimoJsmodule* gimo_jsmodule_new (void)
{
    return g_object_new (GIMO_TYPE_JSMODULE, NULL);
}
