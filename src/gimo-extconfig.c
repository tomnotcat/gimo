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

/*
 * MT safe
 */

#include "gimo-extconfig.h"
#include <string.h>

G_DEFINE_TYPE (GimoExtConfig, gimo_extconfig, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_NAME,
    PROP_VALUE
};

struct _GimoExtConfigPrivate {
    gchar *name;
    gchar *value;
};

static void gimo_extconfig_init (GimoExtConfig *self)
{
    GimoExtConfigPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_EXTCONFIG,
                                              GimoExtConfigPrivate);
    priv = self->priv;

    priv->name = NULL;
    priv->value = NULL;
}

static void gimo_extconfig_finalize (GObject *gobject)
{
    GimoExtConfig *self = GIMO_EXTCONFIG (gobject);
    GimoExtConfigPrivate *priv = self->priv;

    g_free (priv->name);
    g_free (priv->value);

    G_OBJECT_CLASS (gimo_extconfig_parent_class)->finalize (gobject);
}

static void gimo_extconfig_set_property (GObject *object,
                                         guint prop_id,
                                         const GValue *value,
                                         GParamSpec *pspec)
{
    GimoExtConfig *self = GIMO_EXTCONFIG (object);
    GimoExtConfigPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_NAME:
        priv->name = g_value_dup_string (value);
        break;

    case PROP_VALUE:
        priv->value = g_value_dup_string (value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_extconfig_get_property (GObject *object,
                                         guint prop_id,
                                         GValue *value,
                                         GParamSpec *pspec)
{
    GimoExtConfig *self = GIMO_EXTCONFIG (object);
    GimoExtConfigPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_NAME:
        g_value_set_string (value, priv->name);
        break;

    case PROP_VALUE:
        g_value_set_string (value, priv->value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_extconfig_class_init (GimoExtConfigClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (GimoExtConfigPrivate));

    gobject_class->finalize = gimo_extconfig_finalize;
    gobject_class->set_property = gimo_extconfig_set_property;
    gobject_class->get_property = gimo_extconfig_get_property;

    g_object_class_install_property (
        gobject_class, PROP_NAME,
        g_param_spec_string ("name",
                             "Configuration name",
                             "The configuration name",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_VALUE,
        g_param_spec_string ("value",
                             "Configuration value",
                             "The configuration value",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));
}

GimoExtConfig* gimo_extconfig_new (const gchar *name,
                                   const gchar *value)
{
    return g_object_new (GIMO_TYPE_EXTCONFIG,
                         "name", name,
                         "value", value,
                         NULL);
}

const gchar* gimo_extconfig_get_name (GimoExtConfig *self)
{
    g_return_val_if_fail (GIMO_IS_EXTCONFIG (self), NULL);

    return self->priv->name;
}

const gchar* gimo_extconfig_get_value (GimoExtConfig *self)
{
    g_return_val_if_fail (GIMO_IS_EXTCONFIG (self), NULL);

    return self->priv->value;
}

gint _gimo_extconfig_sort_by_name (gconstpointer a,
                                   gconstpointer b)
{
    GimoExtConfig *p1 = *(GimoExtConfig **) a;
    GimoExtConfig *p2 = *(GimoExtConfig **) b;

    return strcmp (p1->priv->name, p2->priv->name);
}

gint _gimo_extconfig_search_by_name (gconstpointer a,
                                     gconstpointer b)
{
    GimoExtConfig *p = *(GimoExtConfig **) b;

    return strcmp (a, p->priv->name);
}
