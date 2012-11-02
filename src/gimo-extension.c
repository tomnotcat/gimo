/* GIMO - A plugin framework based on GObject.
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

#include "gimo-extension.h"
#include "gimo-extconfig.h"
#include "gimo-pluginfo.h"
#include "gimo-utils.h"
#include <stdlib.h>
#include <string.h>

extern gint _gimo_extconfig_sort_by_name (gconstpointer a,
                                          gconstpointer b);
extern gint _gimo_extconfig_search_by_name (gconstpointer a,
                                            gconstpointer b);

G_DEFINE_TYPE (GimoExtension, gimo_extension, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_ID,
    PROP_NAME,
    PROP_POINT,
    PROP_CONFIGS
};

struct _GimoExtensionPrivate {
    gchar *local_id;
    gchar *id;
    gchar *name;
    gchar *extpoint_id;
    GimoPluginfo *info;
    GPtrArray *configs;
};

G_LOCK_DEFINE_STATIC (extension_lock);

static void gimo_extension_init (GimoExtension *self)
{
    GimoExtensionPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_EXTENSION,
                                              GimoExtensionPrivate);
    priv = self->priv;

    priv->local_id = NULL;
    priv->id = NULL;
    priv->name = NULL;
    priv->extpoint_id = NULL;
    priv->info = NULL;
    priv->configs = NULL;
}

static void gimo_extension_finalize (GObject *gobject)
{
    GimoExtension *self = GIMO_EXTENSION (gobject);
    GimoExtensionPrivate *priv = self->priv;

    g_assert (NULL == priv->info);

    g_free (priv->local_id);
    g_free (priv->id);
    g_free (priv->name);
    g_free (priv->extpoint_id);

    if (priv->configs)
        g_ptr_array_unref (priv->configs);

    G_OBJECT_CLASS (gimo_extension_parent_class)->finalize (gobject);
}

static void gimo_extension_set_property (GObject *object,
                                         guint prop_id,
                                         const GValue *value,
                                         GParamSpec *pspec)
{
    GimoExtension *self = GIMO_EXTENSION (object);
    GimoExtensionPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_ID:
        priv->local_id = g_value_dup_string (value);
        break;

    case PROP_NAME:
        priv->name = g_value_dup_string (value);
        break;

    case PROP_POINT:
        priv->extpoint_id = g_value_dup_string (value);
        break;

    case PROP_CONFIGS:
        {
            GPtrArray *arr = g_value_get_boxed (value);
            if (arr) {
                priv->configs = _gimo_utils_clone_object_array (
                    arr, GIMO_TYPE_EXTCONFIG, NULL, NULL);

                g_ptr_array_sort (priv->configs,
                                  _gimo_extconfig_sort_by_name);
            }
        }
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_extension_get_property (GObject *object,
                                         guint prop_id,
                                         GValue *value,
                                         GParamSpec *pspec)
{
    GimoExtension *self = GIMO_EXTENSION (object);
    GimoExtensionPrivate *priv = self->priv;

    switch (prop_id) {
    case PROP_ID:
        g_value_set_string (value, priv->local_id);
        break;

    case PROP_NAME:
        g_value_set_string (value, priv->name);
        break;

    case PROP_POINT:
        g_value_set_string (value, priv->extpoint_id);
        break;

    case PROP_CONFIGS:
        g_value_set_boxed (value, priv->configs);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gimo_extension_class_init (GimoExtensionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_extension_finalize;
    gobject_class->set_property = gimo_extension_set_property;
    gobject_class->get_property = gimo_extension_get_property;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoExtensionPrivate));

    g_object_class_install_property (
        gobject_class, PROP_ID,
        g_param_spec_string ("id",
                             "Local identifier",
                             "The local identifier of the extension",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_NAME,
        g_param_spec_string ("name",
                             "Extension name",
                             "The display name of the extension",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_POINT,
        g_param_spec_string ("point",
                             "Extension point identifier",
                             "The identifier of the host extension poit",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_WRITABLE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
        gobject_class, PROP_CONFIGS,
        g_param_spec_boxed ("configs",
                            "Extension configurations",
                            "The configurations of the extension",
                            GIMO_TYPE_OBJECT_ARRAY,
                            G_PARAM_READABLE |
                            G_PARAM_WRITABLE |
                            G_PARAM_CONSTRUCT_ONLY |
                            G_PARAM_STATIC_STRINGS));
}

GimoExtension* gimo_extension_new (const gchar *id,
                                   const gchar *name,
                                   const gchar *point,
                                   GPtrArray *configs)
{
    return g_object_new (GIMO_TYPE_EXTENSION,
                         "id", id,
                         "name", name,
                         "point", point,
                         "configs", configs,
                         NULL);
}

const gchar* gimo_extension_get_local_id (GimoExtension *self)
{
    g_return_val_if_fail (GIMO_IS_EXTENSION (self), NULL);

    return self->priv->local_id;
}

const gchar* gimo_extension_get_id (GimoExtension *self)
{
    g_return_val_if_fail (GIMO_IS_EXTENSION (self), NULL);

    return self->priv->id;
}

const gchar* gimo_extension_get_name (GimoExtension *self)
{
    g_return_val_if_fail (GIMO_IS_EXTENSION (self), NULL);

    return self->priv->name;
}

const gchar* gimo_extension_get_extpoint_id (GimoExtension *self)
{
    g_return_val_if_fail (GIMO_IS_EXTENSION (self), NULL);

    return self->priv->extpoint_id;
}

/**
 * gimo_extension_get_config:
 * @self: a #GimoExtension
 * @name: the configuration name
 *
 * Get a configuration with the specified name.
 *
 * Returns: (allow-none) (transfer none): a #GimoExtConfig
 */
GimoExtConfig* gimo_extension_get_config (GimoExtension *self,
                                          const gchar *name)
{
    GimoExtensionPrivate *priv;
    GimoExtConfig **result;

    g_return_val_if_fail (GIMO_IS_EXTENSION (self), NULL);

    priv = self->priv;

    if (NULL == priv->configs)
        return NULL;

    result = bsearch (name,
                      priv->configs->pdata,
                      priv->configs->len,
                      sizeof (gpointer),
                      _gimo_extconfig_search_by_name);

    return result ? *result : NULL;
}

/**
 * gimo_extension_get_configs:
 * @self: a #GimoExtension
 *
 * Get the configurations of the extension.
 *
 * Returns: (element-type Gimo.ExtConfig) (transfer none):
 *          the configurations list.
 */
GPtrArray* gimo_extension_get_configs (GimoExtension *self)
{
    g_return_val_if_fail (GIMO_IS_EXTENSION (self), NULL);

    return self->priv->configs;
}

/**
 * gimo_extension_query_pluginfo:
 * @self: a #GimoExtpoint
 *
 * Query the plugin descriptor of the extension.
 *
 * Returns: (allow-none) (transfer full): a #GimoPluginfo
 */
GimoPluginfo* gimo_extension_query_pluginfo (GimoExtension *self)
{
    GimoExtensionPrivate *priv;
    GimoPluginfo *info = NULL;

    g_return_val_if_fail (GIMO_IS_EXTENSION (self), NULL);

    priv = self->priv;

    G_LOCK (extension_lock);

    if (priv->info)
        info = g_object_ref (priv->info);

    G_UNLOCK (extension_lock);

    return info;
}

void _gimo_extension_setup (GimoExtension *self,
                            GimoPluginfo *info)
{
    GimoExtensionPrivate *priv = self->priv;

    g_assert (NULL == priv->id);

    G_LOCK (extension_lock);

    priv->info = info;

    priv->id = g_strdup_printf ("%s.%s",
                                gimo_pluginfo_get_id (info),
                                priv->local_id);
    G_UNLOCK (extension_lock);
}

void _gimo_extension_teardown (GimoExtension *self,
                               GimoPluginfo *info)
{
    GimoExtensionPrivate *priv = self->priv;

    g_assert (priv->info == info);

    G_LOCK (extension_lock);

    priv->info = NULL;

    G_UNLOCK (extension_lock);
}

gint _gimo_extension_sort_by_id (gconstpointer a,
                                 gconstpointer b)
{
    GimoExtension *p1 = *(GimoExtension **) a;
    GimoExtension *p2 = *(GimoExtension **) b;

    return strcmp (p1->priv->local_id, p2->priv->local_id);
}

gint _gimo_extension_search_by_id (gconstpointer a,
                                   gconstpointer b)
{
    GimoExtension *p = *(GimoExtension **) b;

    return strcmp (a, p->priv->local_id);
}
