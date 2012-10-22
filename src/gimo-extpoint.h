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
#ifndef __GIMO_EXTPOINT_H__
#define __GIMO_EXTPOINT_H__

#include "gimo-types.h"

G_BEGIN_DECLS

#define GIMO_TYPE_EXTPOINT (gimo_extpoint_get_type())
#define GIMO_EXTPOINT(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GIMO_TYPE_EXTPOINT, GimoExtpoint))
#define GIMO_IS_EXTPOINT(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GIMO_TYPE_EXTPOINT))
#define GIMO_EXTPOINT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GIMO_TYPE_EXTPOINT, GimoExtpointClass))
#define GIMO_IS_EXTPOINT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GIMO_TYPE_EXTPOINT))
#define GIMO_EXTPOINT_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GIMO_TYPE_EXTPOINT, GimoExtpointClass))

typedef struct _GimoExtpointPrivate GimoExtpointPrivate;
typedef struct _GimoExtpointClass GimoExtpointClass;

struct _GimoExtpoint {
    GObject parent_instance;
    GimoExtpointPrivate *priv;
};

struct _GimoExtpointClass {
    GObjectClass parent_class;
};

GType gimo_extpoint_get_type (void) G_GNUC_CONST;

GimoExtpoint* gimo_extpoint_new (const gchar *local_id,
                                 const gchar *name);

const gchar* gimo_extpoint_get_local_id (GimoExtpoint *self);

const gchar* gimo_extpoint_get_name (GimoExtpoint *self);

const gchar* gimo_extpoint_get_identifier (GimoExtpoint *self);

GimoPluginfo* gimo_extpoint_query_pluginfo (GimoExtpoint *self);

GObject* gimo_extpoint_resolve (GimoExtpoint *self);

G_END_DECLS

#endif /* __GIMO_EXTPOINT_H__ */
