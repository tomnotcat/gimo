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
#ifndef __GIMO_LOADER_H__
#define __GIMO_LOADER_H__

#include "gimo-types.h"

G_BEGIN_DECLS

#define GIMO_TYPE_LOADER (gimo_loader_get_type())
#define GIMO_LOADER(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GIMO_TYPE_LOADER, GimoLoader))
#define GIMO_IS_LOADER(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GIMO_TYPE_LOADER))
#define GIMO_LOADER_GET_IFACE(inst) \
    (G_TYPE_INSTANCE_GET_INTERFACE ((inst), GIMO_TYPE_LOADER, GimoLoaderInterface))

typedef struct _GimoLoaderInterface GimoLoaderInterface;

struct _GimoLoaderInterface {
    GTypeInterface base_iface;
    GimoPlugin* (*load) (GimoLoader *self, GimoPluginfo *info);
};

GType gimo_loader_get_type (void) G_GNUC_CONST;

GimoPlugin* gimo_loader_load (GimoLoader *self,
                              GimoPluginfo *info);

G_END_DECLS

#endif /* __GIMO_LOADER_H__ */
