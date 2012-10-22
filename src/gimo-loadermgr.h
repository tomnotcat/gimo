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
#ifndef __GIMO_LOADERMGR_H__
#define __GIMO_LOADERMGR_H__

#include "gimo-types.h"

G_BEGIN_DECLS

#define GIMO_TYPE_LOADERMGR (gimo_loadermgr_get_type())
#define GIMO_LOADERMGR(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GIMO_TYPE_LOADERMGR, GimoLoaderMgr))
#define GIMO_IS_LOADERMGR(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GIMO_TYPE_LOADERMGR))
#define GIMO_LOADERMGR_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GIMO_TYPE_LOADERMGR, GimoLoaderMgrClass))
#define GIMO_IS_LOADERMGR_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GIMO_TYPE_LOADERMGR))
#define GIMO_LOADERMGR_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GIMO_TYPE_LOADERMGR, GimoLoaderMgrClass))

typedef struct _GimoLoaderMgrPrivate GimoLoaderMgrPrivate;
typedef struct _GimoLoaderMgrClass GimoLoaderMgrClass;

struct _GimoLoaderMgr {
    GObject parent_instance;
    GimoLoaderMgrPrivate *priv;
};

struct _GimoLoaderMgrClass {
    GObjectClass parent_class;
};

GType gimo_loadermgr_get_type (void) G_GNUC_CONST;

GimoLoaderMgr* gimo_loadermgr_new (GimoContext *ctx);

GimoPlugin* gimo_loadermgr_load (GimoLoaderMgr *self,
                                 GimoPluginfo *info);

G_END_DECLS

#endif /* __GIMO_LOADERMGR_H__ */
