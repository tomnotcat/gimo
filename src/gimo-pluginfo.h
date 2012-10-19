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
#ifndef __GIMO_PLUGINFO_H__
#define __GIMO_PLUGINFO_H__

#include "gimo-types.h"

G_BEGIN_DECLS

#define GIMO_TYPE_PLUGINFO (gimo_pluginfo_get_type())
#define GIMO_PLUGINFO(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GIMO_TYPE_PLUGINFO, GimoPluginfo))
#define GIMO_IS_PLUGINFO(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GIMO_TYPE_PLUGINFO))
#define GIMO_PLUGINFO_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GIMO_TYPE_PLUGINFO, GimoPluginfoClass))
#define GIMO_IS_PLUGINFO_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GIMO_TYPE_PLUGINFO))
#define GIMO_PLUGINFO_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GIMO_TYPE_PLUGINFO, GimoPluginfoClass))

typedef struct _GimoPluginfoPrivate GimoPluginfoPrivate;
typedef struct _GimoPluginfoClass GimoPluginfoClass;

struct _GimoPluginfo {
    GObject parent_instance;
    GimoPluginfoPrivate *priv;
};

struct _GimoPluginfoClass {
    GObjectClass parent_class;
};

GType gimo_pluginfo_get_type (void) G_GNUC_CONST;

GimoPluginfo* gimo_pluginfo_new (const gchar *identifier,
                                 const gchar *url,
                                 const gchar *klass,
                                 const gchar *name,
                                 const gchar *version,
                                 const gchar *provider,
                                 GSList *requires,
                                 GSList *extpoints,
                                 GSList *extensions);

const gchar* gimo_pluginfo_get_identifier (GimoPluginfo *self);

const gchar* gimo_pluginfo_get_url (GimoPluginfo *self);

const gchar* gimo_pluginfo_get_klass (GimoPluginfo *self);

const gchar* gimo_pluginfo_get_name (GimoPluginfo *self);

const gchar* gimo_pluginfo_get_version (GimoPluginfo *self);

const gchar* gimo_pluginfo_get_provider (GimoPluginfo *self);

GSList* gimo_pluginfo_get_requires (GimoPluginfo *self);

GSList* gimo_pluginfo_get_extpoints (GimoPluginfo *self);

GSList* gimo_pluginfo_get_extensions (GimoPluginfo *self);

G_END_DECLS

#endif /* __GIMO_PLUGINFO_H__ */
