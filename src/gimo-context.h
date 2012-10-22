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
#ifndef __GIMO_CONTEXT_H__
#define __GIMO_CONTEXT_H__

#include "gimo-types.h"

G_BEGIN_DECLS

#define GIMO_TYPE_CONTEXT (gimo_context_get_type())
#define GIMO_CONTEXT(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GIMO_TYPE_CONTEXT, GimoContext))
#define GIMO_IS_CONTEXT(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GIMO_TYPE_CONTEXT))
#define GIMO_CONTEXT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GIMO_TYPE_CONTEXT, GimoContextClass))
#define GIMO_IS_CONTEXT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GIMO_TYPE_CONTEXT))
#define GIMO_CONTEXT_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GIMO_TYPE_CONTEXT, GimoContextClass))

typedef struct _GimoContextPrivate GimoContextPrivate;
typedef struct _GimoContextClass GimoContextClass;

struct _GimoContext {
    GObject parent_instance;
    GimoContextPrivate *priv;
};

struct _GimoContextClass {
    GObjectClass parent_class;
    void (*state_changed) (GimoContext *self,
                           GimoPluginfo *info,
                           GimoPluginState old_state,
                           GimoPluginState new_state);
};

GType gimo_context_get_type (void) G_GNUC_CONST;

GimoContext* gimo_context_new (void);

GimoStatus gimo_context_install_plugin (GimoContext *self,
                                        GimoPluginfo *info);

GimoStatus gimo_context_uninstall_plugin (GimoContext *self,
                                          const gchar *plugin_id);

GimoPluginfo* gimo_context_query_plugin (GimoContext *self,
                                         const gchar *plugin_id);

GPtrArray* gimo_context_query_plugins (GimoContext *self,
                                       const gchar *namesps);

GimoExtpoint* gimo_context_query_extpoint (GimoContext *self,
                                           const gchar *extpoint_id);

G_END_DECLS

#endif /* __GIMO_CONTEXT_H__ */
