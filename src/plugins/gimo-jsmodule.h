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
#ifndef __GIMO_JSMODULE_H__
#define __GIMO_JSMODULE_H__

#include "gimo-loadable.h"
#include "gimo-module.h"

G_BEGIN_DECLS

#define GIMO_TYPE_JSMODULE (gimo_jsmodule_get_type())
#define GIMO_JSMODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GIMO_TYPE_JSMODULE, GimoJsmodule))
#define GIMO_IS_JSMODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GIMO_TYPE_JSMODULE))
#define GIMO_JSMODULE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GIMO_TYPE_JSMODULE, GimoJsmoduleClass))
#define GIMO_IS_JSMODULE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GIMO_TYPE_JSMODULE))
#define GIMO_JSMODULE_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GIMO_TYPE_JSMODULE, GimoJsmoduleClass))

typedef struct _GimoJsmodule GimoJsmodule;
typedef struct _GimoJsmodulePrivate GimoJsmodulePrivate;
typedef struct _GimoJsmoduleClass GimoJsmoduleClass;

struct _GimoJsmodule {
    GObject parent_instance;
    GimoJsmodulePrivate *priv;
};

struct _GimoJsmoduleClass {
    GObjectClass parent_class;
};

GType gimo_jsmodule_get_type (void) G_GNUC_CONST;

GimoJsmodule* gimo_jsmodule_new (void);

G_END_DECLS

#endif /* __GIMO_JSMODULE_H__ */
