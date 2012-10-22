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
#ifndef __GIMO_LOADERSET_H__
#define __GIMO_LOADERSET_H__

#include "gimo-types.h"

G_BEGIN_DECLS

#define GIMO_TYPE_LOADERSET (gimo_loaderset_get_type())
#define GIMO_LOADERSET(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GIMO_TYPE_LOADERSET, GimoLoaderSet))
#define GIMO_IS_LOADERSET(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GIMO_TYPE_LOADERSET))
#define GIMO_LOADERSET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GIMO_TYPE_LOADERSET, GimoLoaderSetClass))
#define GIMO_IS_LOADERSET_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GIMO_TYPE_LOADERSET))
#define GIMO_LOADERSET_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GIMO_TYPE_LOADERSET, GimoLoaderSetClass))

typedef struct _GimoLoaderSetPrivate GimoLoaderSetPrivate;
typedef struct _GimoLoaderSetClass GimoLoaderSetClass;

struct _GimoLoaderSet {
    GObject parent_instance;
    GimoLoaderSetPrivate *priv;
};

struct _GimoLoaderSetClass {
    GObjectClass parent_class;
};

GType gimo_loaderset_get_type (void) G_GNUC_CONST;

GimoPlugin* gimo_loaderset_load (GimoLoaderSet *self,
                                 GimoPluginfo *info);

G_END_DECLS

#endif /* __GIMO_LOADERSET_H__ */
