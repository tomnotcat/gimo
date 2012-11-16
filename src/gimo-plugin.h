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
#ifndef __GIMO_PLUGIN_H__
#define __GIMO_PLUGIN_H__

#include "gimo-types.h"

G_BEGIN_DECLS

#define GIMO_TYPE_PLUGIN (gimo_plugin_get_type())
#define GIMO_PLUGIN(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GIMO_TYPE_PLUGIN, GimoPlugin))
#define GIMO_IS_PLUGIN(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GIMO_TYPE_PLUGIN))
#define GIMO_PLUGIN_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GIMO_TYPE_PLUGIN, GimoPluginClass))
#define GIMO_IS_PLUGIN_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GIMO_TYPE_PLUGIN))
#define GIMO_PLUGIN_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GIMO_TYPE_PLUGIN, GimoPluginClass))

typedef struct _GimoPluginPrivate GimoPluginPrivate;
typedef struct _GimoPluginClass GimoPluginClass;

struct _GimoPlugin {
    GObject parent_instance;
    GimoPluginPrivate *priv;
};

struct _GimoPluginClass {
    GObjectClass parent_class;
};

GType gimo_plugin_get_type (void) G_GNUC_CONST;

GimoPlugin* gimo_plugin_new (const gchar *id,
                             const gchar *name,
                             const gchar *version,
                             const gchar *provider,
                             const gchar *module,
                             const gchar *symbol,
                             GPtrArray *requires,
                             GPtrArray *extpoints,
                             GPtrArray *extensions);

const gchar* gimo_plugin_get_id (GimoPlugin *self);

const gchar* gimo_plugin_get_name (GimoPlugin *self);

const gchar* gimo_plugin_get_version (GimoPlugin *self);

const gchar* gimo_plugin_get_provider (GimoPlugin *self);

const gchar* gimo_plugin_get_uri (GimoPlugin *self);

const gchar* gimo_plugin_get_module (GimoPlugin *self);

const gchar* gimo_plugin_get_symbol (GimoPlugin *self);

GimoExtPoint* gimo_plugin_get_extpoint (GimoPlugin *self,
                                        const gchar *local_id);

GimoExtension* gimo_plugin_get_extension (GimoPlugin *self,
                                          const gchar *local_id);

GPtrArray* gimo_plugin_get_requires (GimoPlugin *self);

GPtrArray* gimo_plugin_get_extpoints (GimoPlugin *self);

GPtrArray* gimo_plugin_get_extensions (GimoPlugin *self);

GimoPluginState gimo_plugin_get_state (GimoPlugin *self);

GimoContext* gimo_plugin_query_context (GimoPlugin *self);

gboolean gimo_plugin_start (GimoPlugin *self,
                            GimoLoader *loader);

gboolean gimo_plugin_stop (GimoPlugin *self);

GObject* gimo_plugin_resolve (GimoPlugin *self,
                              const gchar *symbol);

G_END_DECLS

#endif /* __GIMO_PLUGIN_H__ */
