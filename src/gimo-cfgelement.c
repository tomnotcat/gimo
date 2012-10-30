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
#include "gimo-cfgelement.h"

G_DEFINE_TYPE (GimoCfgElement, gimo_cfgelement, G_TYPE_OBJECT)

struct _GimoCfgElementPrivate {
    gchar *name;
    gchar *value;
};

static void gimo_cfgelement_init (GimoCfgElement *self)
{
    GimoCfgElementPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_CFGELEMENT,
                                              GimoCfgElementPrivate);
    priv = self->priv;

    priv->name = NULL;
    priv->value = NULL;
}

static void gimo_cfgelement_finalize (GObject *gobject)
{
    GimoCfgElement *self = GIMO_CFGELEMENT (gobject);
    GimoCfgElementPrivate *priv = self->priv;

    g_free (priv->name);
    g_free (priv->value);

    G_OBJECT_CLASS (gimo_cfgelement_parent_class)->finalize (gobject);
}

static void gimo_cfgelement_class_init (GimoCfgElementClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_cfgelement_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoCfgElementPrivate));
}

GimoCfgElement* gimo_cfgelement_new (const gchar *name,
                                     const gchar *value)
{
    return g_object_new (GIMO_TYPE_CFGELEMENT,
                         "name", name,
                         "value", value,
                         NULL);
}

const gchar* gimo_cfgelement_get_name (GimoCfgElement *self)
{
    g_return_val_if_fail (GIMO_IS_CFGELEMENT (self), NULL);

    return self->priv->name;
}

const gchar* gimo_cfgelement_get_value (GimoCfgElement *self)
{
    g_return_val_if_fail (GIMO_IS_CFGELEMENT (self), NULL);

    return self->priv->value;
}
