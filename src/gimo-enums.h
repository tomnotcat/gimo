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
#ifndef __GIMO_ENUMS_H__
#define __GIMO_ENUMS_H__

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * GimoStatus:
 * @GIMO_STATUS_SUCCESS: no error has occurred
 * @GIMO_STATUS_NO_MEMORY: out of memory
 * @GIMO_STATUS_INVALID_ID: invalid identifier
 * @GIMO_STATUS_INVALID_OBJECT: invalid object
 * @GIMO_STATUS_INVALID_STATE: invalid state
 * @GIMO_STATUS_CONFLICT: object conflict
 * @GIMO_STATUS_NOT_FOUND: object not found
 */
typedef enum {
    GIMO_STATUS_SUCCESS,
    GIMO_STATUS_NO_MEMORY,
    GIMO_STATUS_INVALID_ID,
    GIMO_STATUS_INVALID_OBJECT,
    GIMO_STATUS_INVALID_STATE,
    GIMO_STATUS_CONFLICT,
    GIMO_STATUS_NOT_FOUND
} GimoStatus;

/**
 * GimoPluginState:
 * @GIMO_PLUGIN_UNINSTALLED: not installed
 * @GIMO_PLUGIN_INSTALLED: installed
 * @GIMO_PLUGIN_RESOLVED: dependencies have been resolved
 * @GIMO_PLUGIN_STARTING: starting
 * @GIMO_PLUGIN_STOPPING: stopping
 * @GIMO_PLUGIN_ACTIVE: has been successfully started
 */
typedef enum {
    GIMO_PLUGIN_UNINSTALLED,
    GIMO_PLUGIN_INSTALLED,
    GIMO_PLUGIN_RESOLVED,
    GIMO_PLUGIN_STARTING,
    GIMO_PLUGIN_STOPPING,
    GIMO_PLUGIN_ACTIVE
} GimoPluginState;

GType gimo_plugin_state_get_type (void) G_GNUC_CONST;
#define GIMO_TYPE_PLUGIN_STATE (gimo_plugin_state_get_type ())

G_END_DECLS

#endif /* __GIMO_ENUMS_H__ */
