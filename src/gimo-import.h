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
#ifndef __GIMO_IMPORT_H__
#define __GIMO_IMPORT_H__

#include "gimo-types.h"

G_BEGIN_DECLS

#define GIMO_TYPE_IMPORT (gimo_import_get_type())
#define GIMO_IMPORT(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GIMO_TYPE_IMPORT, GimoImport))
#define GIMO_IS_IMPORT(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GIMO_TYPE_IMPORT))
#define GIMO_IMPORT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GIMO_TYPE_IMPORT, GimoImportClass))
#define GIMO_IS_IMPORT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GIMO_TYPE_IMPORT))
#define GIMO_IMPORT_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GIMO_TYPE_IMPORT, GimoImportClass))

typedef struct _GimoImportPrivate GimoImportPrivate;
typedef struct _GimoImportClass GimoImportClass;

struct _GimoImport {
    GObject parent_instance;
    GimoImportPrivate *priv;
};

struct _GimoImportClass {
    GObjectClass parent_class;
};

GType gimo_import_get_type (void) G_GNUC_CONST;

GimoImport* gimo_import_new (const gchar *plugin_id,
                             const gchar *version,
                             gboolean optional);

const gchar* gimo_import_get_plugin_id (GimoImport *self);

const gchar* gimo_import_get_version (GimoImport *self);

gboolean gimo_import_is_optional (GimoImport *self);

G_END_DECLS

#endif /* __GIMO_IMPORT_H__ */
