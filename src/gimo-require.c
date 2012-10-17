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
#include "gimo-require.h"

G_DEFINE_TYPE (GimoRequire, gimo_require, G_TYPE_OBJECT)

struct _GimoRequirePrivate {
    gchar *plugin_id;
    gchar *version;
    gboolean optional;
};

static void gimo_require_init (GimoRequire *self)
{
    GimoRequirePrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_REQUIRE,
                                              GimoRequirePrivate);
    priv = self->priv;

    priv->plugin_id = NULL;
    priv->version = NULL;
    priv->optional = FALSE;
}

static void gimo_require_finalize (GObject *gobject)
{
    GimoRequire *self = GIMO_REQUIRE (gobject);
    GimoRequirePrivate *priv = self->priv;

    g_free (priv->plugin_id);
    g_free (priv->version);

    G_OBJECT_CLASS (gimo_require_parent_class)->finalize (gobject);
}

static void gimo_require_class_init (GimoRequireClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_require_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoRequirePrivate));
}

GimoRequire* gimo_require_new (const gchar *plugin_id,
                               const gchar *version,
                               gboolean optional)
{
    GimoRequire *self = g_object_new (GIMO_TYPE_REQUIRE, NULL);
    GimoRequirePrivate *priv = self->priv;

    priv->plugin_id = g_strdup (plugin_id);
    priv->version = g_strdup (version);
    priv->optional = optional;

    return self;
}

const gchar* gimo_require_get_plugin_id (GimoRequire *self)
{
    g_return_val_if_fail (GIMO_IS_REQUIRE (self), NULL);

    return self->priv->plugin_id;
}

const gchar* gimo_require_get_version (GimoRequire *self)
{
    g_return_val_if_fail (GIMO_IS_REQUIRE (self), NULL);

    return self->priv->version;
}

gboolean gimo_require_is_optional (GimoRequire *self)
{
    g_return_val_if_fail (GIMO_IS_REQUIRE (self), FALSE);

    return self->priv->optional;
}
