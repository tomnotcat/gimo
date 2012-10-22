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
#include "gimo-loadermgr.h"

G_DEFINE_TYPE (GimoLoaderMgr, gimo_loadermgr, G_TYPE_OBJECT)

struct _GimoLoaderMgrPrivate {
    int n;
};

static void gimo_loadermgr_init (GimoLoaderMgr *self)
{
    GimoLoaderMgrPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_LOADERMGR,
                                              GimoLoaderMgrPrivate);
    priv = self->priv;

    priv->n = 0;
}

static void gimo_loadermgr_finalize (GObject *gobject)
{
    G_OBJECT_CLASS (gimo_loadermgr_parent_class)->finalize (gobject);
}

static void gimo_loadermgr_class_init (GimoLoaderMgrClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_loadermgr_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoLoaderMgrPrivate));
}

/**
 * gimo_loadermgr_load:
 * @self: a #GimoLoaderMgr
 * @info: the plugin descriptor
 *
 * Load a plugin based on the plugin descriptor.
 *
 * Returns: (allow-none) (transfer full): a #GimoPlugin
 */
GimoPlugin* gimo_loadermgr_load (GimoLoaderMgr *self,
                                 GimoPluginfo *info)
{
    return NULL;
}
