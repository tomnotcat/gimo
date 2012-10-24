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
#ifndef __GIMO_PYMODULE_H__
#define __GIMO_PYMODULE_H__

#include "gimo-module.h"

G_BEGIN_DECLS

#define GIMO_TYPE_PYMODULE (gimo_pymodule_get_type())
#define GIMO_PYMODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GIMO_TYPE_PYMODULE, GimoPymodule))
#define GIMO_IS_PYMODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GIMO_TYPE_PYMODULE))
#define GIMO_PYMODULE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GIMO_TYPE_PYMODULE, GimoPymoduleClass))
#define GIMO_IS_PYMODULE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GIMO_TYPE_PYMODULE))
#define GIMO_PYMODULE_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GIMO_TYPE_PYMODULE, GimoPymoduleClass))

typedef struct _GimoPymodule GimoPymodule;
typedef struct _GimoPymodulePrivate GimoPymodulePrivate;
typedef struct _GimoPymoduleClass GimoPymoduleClass;

struct _GimoPymodule {
    GObject parent_instance;
    GimoPymodulePrivate *priv;
};

struct _GimoPymoduleClass {
    GObjectClass parent_class;
};

GType gimo_pymodule_get_type (void) G_GNUC_CONST;

GimoPymodule* gimo_pymodule_new (void);

G_END_DECLS

#endif /* __GIMO_PYMODULE_H__ */
