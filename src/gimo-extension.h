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
#ifndef __GIMO_EXTENSION_H__
#define __GIMO_EXTENSION_H__

#include "gimo-types.h"

G_BEGIN_DECLS

#define GIMO_TYPE_EXTENSION (gimo_extension_get_type())
#define GIMO_EXTENSION(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GIMO_TYPE_EXTENSION, GimoExtension))
#define GIMO_IS_EXTENSION(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GIMO_TYPE_EXTENSION))
#define GIMO_EXTENSION_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GIMO_TYPE_EXTENSION, GimoExtensionClass))
#define GIMO_IS_EXTENSION_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GIMO_TYPE_EXTENSION))
#define GIMO_EXTENSION_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GIMO_TYPE_EXTENSION, GimoExtensionClass))

typedef struct _GimoExtensionPrivate GimoExtensionPrivate;
typedef struct _GimoExtensionClass GimoExtensionClass;

struct _GimoExtension {
    GObject parent_instance;
    GimoExtensionPrivate *priv;
};

struct _GimoExtensionClass {
    GObjectClass parent_class;
};

GType gimo_extension_get_type (void) G_GNUC_CONST;

GimoExtension* gimo_extension_new (void);

G_END_DECLS

#endif /* __GIMO_EXTENSION_H__ */
