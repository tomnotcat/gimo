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
#ifndef __GIMO_RUNTIME_H__
#define __GIMO_RUNTIME_H__

#include "gimo-types.h"

G_BEGIN_DECLS

#define GIMO_TYPE_RUNTIME (gimo_runtime_get_type())
#define GIMO_RUNTIME(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GIMO_TYPE_RUNTIME, GimoRuntime))
#define GIMO_IS_RUNTIME(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GIMO_TYPE_RUNTIME))
#define GIMO_RUNTIME_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GIMO_TYPE_RUNTIME, GimoRuntimeClass))
#define GIMO_IS_RUNTIME_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GIMO_TYPE_RUNTIME))
#define GIMO_RUNTIME_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), GIMO_TYPE_RUNTIME, GimoRuntimeClass))

typedef struct _GimoRuntimePrivate GimoRuntimePrivate;
typedef struct _GimoRuntimeClass GimoRuntimeClass;

struct _GimoRuntime {
    GObject parent_instance;
    GimoRuntimePrivate *priv;
};

struct _GimoRuntimeClass {
    GObjectClass parent_class;
    gboolean (*start) (GimoRuntime *self);
    gboolean (*stop) (GimoRuntime *self);
};

GType gimo_runtime_get_type (void) G_GNUC_CONST;

GimoRuntime* gimo_runtime_new (void);

gboolean gimo_runtime_define_object (GimoRuntime *self,
                                     const gchar *symbol,
                                     GObject *object);

gboolean gimo_runtime_start (GimoRuntime *self);

gboolean gimo_runtime_stop (GimoRuntime *self);

GObject* gimo_runtime_resolve (GimoRuntime *self, const gchar *symbol);

GimoPlugin* gimo_runtime_query_plugin (GimoRuntime *self);

GimoContext* gimo_runtime_query_context (GimoRuntime *self);

#define GIMO_RUNTIME_DEFAULT_SYMBOL gimo_create_plugin
#define GIMO_RUNTIME_DEFAULT_SYMBOL_NAME "gimo_create_plugin"

#define GIMO_DEFINE_RUNTIME_SYMBOL(_SYMBOL_, _CODE_) \
    GimoRuntime* _SYMBOL_ (GObject *plugin) \
    { \
        GimoRuntime *runtime = gimo_runtime_new (); \
        _CODE_; \
        return runtime; \
    }

#define GIMO_DEFINE_RUNTIME_DEFAULT_SYMBOL(_CODE_) \
    GIMO_DEFINE_RUNTIME_SYMBOL (GIMO_RUNTIME_DEFAULT_SYMBOL, _CODE_)

G_END_DECLS

#endif /* __GIMO_RUNTIME_H__ */
