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
#ifndef __GIMO_CFGELEMENT_H__
#define __GIMO_CFGELEMENT_H__

#include "gimo-types.h"

G_BEGIN_DECLS

#define GIMO_TYPE_CFGELEMENT (gimo_cfgelement_get_type())
#define GIMO_CFGELEMENT(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GIMO_TYPE_CFGELEMENT, GimoCfgElement))
#define GIMO_IS_CFGELEMENT(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GIMO_TYPE_CFGELEMENT))
#define GIMO_CFGELEMENT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GIMO_TYPE_CFGELEMENT, GimoCfgElementClass))
#define GIMO_IS_CFGELEMENT_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GIMO_TYPE_CFGELEMENT))
#define GIMO_CFGELEMENT_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GIMO_TYPE_CFGELEMENT, GimoCfgElementClass))

typedef struct _GimoCfgElementPrivate GimoCfgElementPrivate;
typedef struct _GimoCfgElementClass GimoCfgElementClass;

struct _GimoCfgElement {
    GObject parent_instance;
    GimoCfgElementPrivate *priv;
};

struct _GimoCfgElementClass {
    GObjectClass parent_class;
};

GType gimo_cfgelement_get_type (void) G_GNUC_CONST;

GimoCfgElement* gimo_cfgelement_new (const gchar *name,
                                     const gchar *value);

const gchar* gimo_cfgelement_get_name (GimoCfgElement *self);

const gchar* gimo_cfgelement_get_value (GimoCfgElement *self);

G_END_DECLS

#endif /* __GIMO_CFGELEMENT_H__ */
