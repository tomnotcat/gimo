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
#include "gimo-extpoint.h"

G_DEFINE_TYPE (GimoExtpoint, gimo_extpoint, G_TYPE_OBJECT)

struct _GimoExtpointPrivate {
    int n;
};

static void gimo_extpoint_init (GimoExtpoint *self)
{
    GimoExtpointPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_EXTPOINT,
                                              GimoExtpointPrivate);
    priv = self->priv;

    priv->n = 0;
}

static void gimo_extpoint_finalize (GObject *gobject)
{
    G_OBJECT_CLASS (gimo_extpoint_parent_class)->finalize (gobject);
}

static void gimo_extpoint_class_init (GimoExtpointClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_extpoint_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoExtpointPrivate));
}

GimoExtpoint* gimo_extpoint_new (void)
{
    return g_object_new (GIMO_TYPE_EXTPOINT, NULL);
}
