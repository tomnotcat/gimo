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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef __GIMO_ERROR_H__
#define __GIMO_ERROR_H__

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * GimoErrors:
 * @GIMO_ERROR_NONE: no error
 * @GIMO_ERROR_NO_MEMORY: out of memory
 * @GIMO_ERROR_NEW_OBJECT: new object failed
 * @GIMO_ERROR_INVALID_ID: invalid identifier
 * @GIMO_ERROR_INVALID_OBJECT: invalid object
 * @GIMO_ERROR_INVALID_STATE: invalid state
 * @GIMO_ERROR_CONFLICT: object conflict
 * @GIMO_ERROR_NOT_FOUND: object not found
 * @GIMO_ERROR_IMPORT: import module error
 * @GIMO_ERROR_UNLOAD: unload module error
 * @GIMO_ERROR_NO_SYMBOL: symbol not found
 * @GIMO_ERROR_INVALID_SYMBOL: invalid symbol
 */
typedef enum {
    GIMO_ERROR_NONE,
    GIMO_ERROR_NO_MEMORY,
    GIMO_ERROR_NEW_OBJECT,
    GIMO_ERROR_INVALID_ID,
    GIMO_ERROR_INVALID_OBJECT,
    GIMO_ERROR_INVALID_STATE,
    GIMO_ERROR_CONFLICT,
    GIMO_ERROR_NOT_FOUND,
    GIMO_ERROR_IMPORT,
    GIMO_ERROR_UNLOAD,
    GIMO_ERROR_NO_SYMBOL,
    GIMO_ERROR_INVALID_SYMBOL
} GimoErrors;

void gimo_set_error (gint code);

void gimo_set_error_string (gint code, const gchar *string);

void gimo_set_error_full (gint code, const gchar *format, ...);

gint gimo_get_error (void);

void gimo_clear_error (void);

const gchar* gimo_error_to_string (gint code);

#define gimo_set_error_return(_code) \
    G_STMT_START { \
        gimo_set_error (_code); \
        return; \
    } G_STMT_END

#define gimo_set_error_return_val(_code, _val) \
    G_STMT_START { \
        gimo_set_error (_code); \
        return _val; \
    } G_STMT_END

G_END_DECLS

#endif /* __GIMO_ERROR_H__ */