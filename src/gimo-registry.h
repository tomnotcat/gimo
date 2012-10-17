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
#ifndef __GIMO_REGISTRY_H__
#define __GIMO_REGISTRY_H__

#include "gimo-types.h"

G_BEGIN_DECLS

#define GIMO_TYPE_REGISTRY (gimo_registry_get_type())
#define GIMO_REGISTRY(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GIMO_TYPE_REGISTRY, GimoRegistry))
#define GIMO_IS_REGISTRY(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GIMO_TYPE_REGISTRY))
#define GIMO_REGISTRY_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GIMO_TYPE_REGISTRY, GimoRegistryClass))
#define GIMO_IS_REGISTRY_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GIMO_TYPE_REGISTRY))
#define GIMO_REGISTRY_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GIMO_TYPE_REGISTRY, GimoRegistryClass))

typedef struct _GimoRegistryPrivate GimoRegistryPrivate;
typedef struct _GimoRegistryClass GimoRegistryClass;

typedef gboolean (*GimoPluginTraverseFunc) (GimoPlugin *plugin, gpointer data);

struct _GimoRegistry {
    GObject parent_instance;
    GimoRegistryPrivate *priv;
};

struct _GimoRegistryClass {
    GObjectClass parent_class;
};

GType gimo_registry_get_type (void) G_GNUC_CONST;

GimoRegistry* gimo_registry_new (void);

GimoStatus gimo_registry_install_plugin (GimoRegistry *self,
                                         GimoPlugin *plugin);

GimoStatus gimo_registry_uninstall_plugin (GimoRegistry *self,
                                           const gchar *id);

GimoPlugin* gimo_registry_query_plugin (GimoRegistry *self,
                                        const gchar *id);

void gimo_registry_foreach_plugins (GimoRegistry *self,
                                    GimoPluginTraverseFunc func,
                                    gpointer user_data);
G_END_DECLS

#endif /* __GIMO_REGISTRY_H__ */
