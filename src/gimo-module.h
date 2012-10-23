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
#ifndef __GIMO_MODULE_H__
#define __GIMO_MODULE_H__

#include "gimo-types.h"

G_BEGIN_DECLS

#define GIMO_TYPE_MODULE (gimo_module_get_type())
#define GIMO_MODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GIMO_TYPE_MODULE, GimoModule))
#define GIMO_IS_MODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GIMO_TYPE_MODULE))
#define GIMO_MODULE_GET_IFACE(inst) \
    (G_TYPE_INSTANCE_GET_INTERFACE ((inst), GIMO_TYPE_MODULE, GimoModuleInterface))

typedef struct _GimoModuleInterface GimoModuleInterface;

struct _GimoModuleInterface {
    GTypeInterface base_iface;
    gboolean (*open) (GimoModule *self,
                      const gchar *file_name);
    gboolean (*close) (GimoModule *self);
    GObject* (*resolve) (GimoModule *self,
                         const gchar *symbol,
                         GObject *param);
};

GType gimo_module_get_type (void) G_GNUC_CONST;

gboolean gimo_module_open (GimoModule *self,
                           const gchar *file_name);

gboolean gimo_module_close (GimoModule *self);

GObject* gimo_module_resolve (GimoModule *self,
                              const gchar *symbol,
                              GObject *param);

G_END_DECLS

#endif /* __GIMO_MODULE_H__ */
