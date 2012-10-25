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
#include "gimo-error.h"

static gint error_code;
static gchar *error_message;

G_LOCK_DEFINE_STATIC (error_lock);

void gimo_set_error (gint code)
{
    gimo_set_error_full (code, NULL);
}

void gimo_set_error_string (gint code, const gchar *string)
{
    gimo_set_error_full (code, string);
}

void gimo_set_error_full (gint code, const gchar *format, ...)
{
    error_code = code;

    if (format) {
        va_list ap;

        va_start (ap, format);
        G_LOCK (error_lock);

        g_free (error_message);
        error_message = g_strdup_vprintf (format, ap);

        G_UNLOCK (error_lock);
        va_end (ap);
    }
}

gint gimo_get_error (void)
{
    return error_code;
}

void gimo_clear_error (void)
{
    error_code = 0;

    if (error_message) {
        G_LOCK (error_lock);
        g_free (error_message);
        error_message = NULL;
        G_UNLOCK (error_lock);
    }
}

const gchar* gimo_error_to_string (gint code)
{
    if (code == error_code && error_message)
        return error_message;

    /* TODO: Add error code to string conversion. */
    return NULL;
}
