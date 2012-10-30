/* GIMO - A plugin system based on GObject.
 *
 * Copyright (C) 2012 TinySoft, Inc.
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
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "gimo-plugin.h"

G_DEFINE_TYPE (GimoPlugin, gimo_plugin, G_TYPE_OBJECT)

struct _GimoPluginPrivate {
    int n;
};

static void gimo_plugin_init (GimoPlugin *self)
{
    GimoPluginPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_PLUGIN,
                                              GimoPluginPrivate);
    priv = self->priv;

    priv->n = 0;
}

static void gimo_plugin_finalize (GObject *gobject)
{
    G_OBJECT_CLASS (gimo_plugin_parent_class)->finalize (gobject);
}

static void gimo_plugin_class_init (GimoPluginClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_plugin_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoPluginPrivate));
}
