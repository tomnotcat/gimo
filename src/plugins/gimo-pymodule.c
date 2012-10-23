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
#include "gimo-pymodule.h"
#include "gimo-pluginfo.h"
#include <gmodule.h>

static void gimo_loader_interface_init (GimoLoaderInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GimoPymodule, gimo_pymodule, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_LOADER,
                                                gimo_loader_interface_init))

static gboolean _gimo_pymodule_check (GimoLoader *loader,
                                      GimoPluginfo *info)
{
    return FALSE;
}

static GimoPlugin* _gimo_pymodule_load (GimoLoader *loader,
                                        GimoPluginfo *info)
{
    return NULL;
}

static void gimo_loader_interface_init (GimoLoaderInterface *iface)
{
    iface->check = _gimo_pymodule_check;
    iface->load = _gimo_pymodule_load;
}

static void gimo_pymodule_init (GimoPymodule *self)
{
}

static void gimo_pymodule_class_init (GimoPymoduleClass *klass)
{
}

GimoPymodule* gimo_pymodule_new (void)
{
    return g_object_new (GIMO_TYPE_PYMODULE, NULL);
}
