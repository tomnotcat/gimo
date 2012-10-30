/* GIMO - A plugin system based on GObject.
 *
 * Copyright (c) 2012 TinySoft, Inc.
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

#include "gimo-context.h"
#include "gimo-error.h"
#include "gimo-extpoint.h"
#include "gimo-loader.h"
#include "gimo-marshal.h"
#include "gimo-pluginfo.h"
#include "gimo-utils.h"
#include <string.h>

extern void _gimo_pluginfo_install (gpointer data, gpointer user_data);
extern void _gimo_pluginfo_uninstall (gpointer data);

G_DEFINE_TYPE (GimoContext, gimo_context, G_TYPE_OBJECT)

enum {
    SIG_STATECHANGED,
    LAST_SIGNAL
};

struct _GimoContextPrivate {
    GTree *plugins;
    GMutex mutex;
};

struct _QueryParam {
    GPtrArray *plugins;
    const gchar *namesps;
};

static guint context_signals[LAST_SIGNAL] = { 0 };

static gint _gimo_context_plugin_compare (gconstpointer a,
                                          gconstpointer b,
                                          gpointer user_data)
{
    return strcmp (a, b);
}

static void _gimo_context_plugin_destroy (gpointer p)
{
    _gimo_pluginfo_uninstall (p);
    g_object_unref (p);
}

static gboolean _gimo_context_query_plugins (gpointer key,
                                             gpointer value,
                                             gpointer data)
{
    GimoPluginfo *info = value;
    struct _QueryParam *p = data;

    if (p->namesps) {
        const gchar *it = p->namesps;
        const gchar *id = gimo_pluginfo_get_identifier (info);

        while (*it) {
            if (*it++ != *id++)
                return FALSE;
        }
    }

    if (NULL == p->plugins)
        p->plugins = g_ptr_array_new_with_free_func (g_object_unref);

    g_ptr_array_add (p->plugins, g_object_ref (info));

    return FALSE;
}

static void gimo_context_init (GimoContext *self)
{
    GimoContextPrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_CONTEXT,
                                              GimoContextPrivate);
    priv = self->priv;

    priv->plugins = g_tree_new_full (_gimo_context_plugin_compare,
                                     NULL, NULL,
                                     _gimo_context_plugin_destroy);
    g_mutex_init (&priv->mutex);
}

static void gimo_context_finalize (GObject *gobject)
{
    GimoContext *self = GIMO_CONTEXT (gobject);
    GimoContextPrivate *priv = self->priv;

    g_tree_unref (priv->plugins);
    g_mutex_clear (&priv->mutex);

    G_OBJECT_CLASS (gimo_context_parent_class)->finalize (gobject);
}

static void gimo_context_class_init (GimoContextClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_context_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoContextPrivate));

    context_signals[SIG_STATECHANGED] =
            g_signal_new ("state-changed",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET (GimoContextClass, state_changed),
                          NULL, NULL,
                          _gimo_marshal_VOID__OBJECT_ENUM_ENUM,
                          G_TYPE_NONE, 3,
                          GIMO_TYPE_PLUGINFO,
                          GIMO_TYPE_PLUGIN_STATE,
                          GIMO_TYPE_PLUGIN_STATE);
}

GimoContext* gimo_context_new (void)
{
    return g_object_new (GIMO_TYPE_CONTEXT, NULL);
}

gboolean gimo_context_install_plugin (GimoContext *self,
                                      GimoPluginfo *info)
{
    GimoContextPrivate *priv;
    const gchar *plugin_id;

    g_return_val_if_fail (GIMO_IS_CONTEXT (self), FALSE);

    priv = self->priv;

    plugin_id = gimo_pluginfo_get_identifier (info);
    if (NULL == plugin_id || !plugin_id[0])
        gimo_set_error_return_val (GIMO_ERROR_INVALID_ID, FALSE);

    g_mutex_lock (&priv->mutex);

    if (g_tree_lookup (priv->plugins, plugin_id)) {
        g_mutex_unlock (&priv->mutex);
        gimo_set_error_return_val (GIMO_ERROR_CONFLICT, FALSE);
    }

    g_tree_insert (priv->plugins,
                   (gpointer) plugin_id,
                   g_object_ref (info));

    _gimo_pluginfo_install (info, self);

    g_mutex_unlock (&priv->mutex);

    g_signal_emit (self,
                   context_signals[SIG_STATECHANGED],
                   0,
                   info,
                   GIMO_PLUGIN_UNINSTALLED,
                   GIMO_PLUGIN_INSTALLED);
    return TRUE;
}

void gimo_context_uninstall_plugin (GimoContext *self,
                                    const gchar *plugin_id)
{
    GimoContextPrivate *priv;
    GimoPluginfo *info;

    g_return_if_fail (GIMO_IS_CONTEXT (self));

    priv = self->priv;

    if (NULL == plugin_id || !plugin_id[0])
        return;

    g_mutex_lock (&priv->mutex);

    info = g_tree_lookup (priv->plugins, plugin_id);
    if (NULL == info) {
        g_mutex_unlock (&priv->mutex);
        return;
    }

    g_object_ref (info);
    g_tree_remove (priv->plugins, plugin_id);
    g_mutex_unlock (&priv->mutex);

    g_signal_emit (self,
                   context_signals[SIG_STATECHANGED],
                   0,
                   info,
                   GIMO_PLUGIN_INSTALLED,
                   GIMO_PLUGIN_UNINSTALLED);

    g_object_unref (info);
}

/**
 * gimo_context_query_plugin:
 * @self: a #GimoContext
 *
 * Query a plugin descriptor with the specified ID.
 *
 * Returns: (allow-none) (transfer full): a #GimoPluginfo
 */
GimoPluginfo* gimo_context_query_plugin (GimoContext *self,
                                         const gchar *plugin_id)
{
    GimoContextPrivate *priv;
    GimoPluginfo *info = NULL;

    g_return_val_if_fail (GIMO_IS_CONTEXT (self), NULL);

    priv = self->priv;

    if (NULL == plugin_id || !plugin_id[0])
        return NULL;

    g_mutex_lock (&priv->mutex);

    info = g_tree_lookup (priv->plugins, plugin_id);
    if (info)
        g_object_ref (info);

    g_mutex_unlock (&priv->mutex);

    return info;
}

/**
 * gimo_context_query_plugins:
 * @self: a #GimoContext
 * @namesps: (allow-none): namespace
 *
 * Query the plugins that inside namespace @namesps
 *
 * Returns: (element-type Gimo.Pluginfo) (transfer full):
 *          an array of plugin descriptors.
 */
GPtrArray* gimo_context_query_plugins (GimoContext *self,
                                       const gchar *namesps)
{
    GimoContextPrivate *priv;
    struct _QueryParam param;

    g_return_val_if_fail (GIMO_IS_CONTEXT (self), NULL);

    priv = self->priv;

    param.plugins = NULL;
    param.namesps = namesps;

    g_mutex_lock (&priv->mutex);

    g_tree_foreach (priv->plugins,
                    _gimo_context_query_plugins,
                    &param);

    g_mutex_unlock (&priv->mutex);

    return param.plugins;
}

/**
 * gimo_context_query_extpoint:
 * @self: a #GimoContext
 *
 * Query an extension point with the specified ID.
 *
 * Returns: (allow-none) (transfer full): a #GimoExtpoint
 */
GimoExtpoint* gimo_context_query_extpoint (GimoContext *self,
                                           const gchar *extpoint_id)
{
    GimoPluginfo *info = NULL;
    GimoExtpoint *extpt = NULL;
    gchar *plugin_id = NULL;
    gchar *local_id = NULL;

    plugin_id = _gimo_utils_parse_extension_id (extpoint_id, &local_id);
    if (NULL == plugin_id)
        goto done;

    info = gimo_context_query_plugin (self, plugin_id);
    if (NULL == info)
        goto done;

    extpt = gimo_pluginfo_get_extpoint (info, local_id);
    if (extpt)
        g_object_ref (extpt);

done:
    if (info)
        g_object_unref (info);

    g_free (plugin_id);

    return extpt;
}

GimoPlugin* _gimo_context_load_plugin (GimoContext *self,
                                       GimoPluginfo *info)
{
    /*
    GimoExtpoint *extpt;
    GimoLoader *loader;
    GimoPlugin *plugin;

    extpt = gimo_context_query_extpoint (self, "gimo.core.loader");
    if (NULL == extpt) {
        return NULL;
    }

    ldmgr = GIMO_LOADERMGR (gimo_extpoint_resolve (extpt));
    if (NULL == ldmgr) {
        g_object_unref (extpt);
        return NULL;
    }

    plugin = gimo_loadermgr_load (ldmgr, info);
    g_object_unref (extpt);
    return plugin;
    */
    return NULL;
}

void _gimo_context_plugin_state_changed (GimoContext *self,
                                         GimoPluginfo *info,
                                         GimoPluginState old_state,
                                         GimoPluginState new_state)
{
    g_signal_emit (self,
                   context_signals[SIG_STATECHANGED],
                   0,
                   info,
                   old_state,
                   new_state);
}
