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
#include "gimo-runnable.h"

G_DEFINE_TYPE (GimoRunnable, gimo_runnable, G_TYPE_OBJECT)

enum {
    SIG_RUN,
    LAST_SIGNAL
};

static guint runnable_signals[LAST_SIGNAL] = { 0 };

static void gimo_runnable_init (GimoRunnable *self)
{
}

static void gimo_runnable_class_init (GimoRunnableClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    klass->run = NULL;

    runnable_signals[SIG_RUN] =
            g_signal_new ("run",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (GimoRunnableClass, run),
                          NULL, NULL,
                          g_cclosure_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);
}

GimoRunnable* gimo_runnable_new (void)
{
    return g_object_new (GIMO_TYPE_RUNNABLE, NULL);
}

void gimo_runnable_run (GimoRunnable *self)
{
    g_return_if_fail (GIMO_IS_RUNNABLE (self));

    g_signal_emit (self,
                   runnable_signals[SIG_RUN],
                   0);
}
