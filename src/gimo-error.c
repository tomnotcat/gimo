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
#include "config.h"
#include "gimo-error.h"
#include "gimo-intl.h"

static gboolean trace_error;
static gint error_code;
static gchar *error_message;

G_LOCK_DEFINE_STATIC (error_lock);

void gimo_trace_error (gboolean trace)
{
    trace_error = trace;
}

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

        if (trace_error)
            g_warning ("GimoError: %d: %s", error_code, error_message);

        G_UNLOCK (error_lock);
        va_end (ap);
    }
    else if (trace_error) {
        g_warning ("GimoError: %d", error_code);
    }
}

gint gimo_get_error (void)
{
    return error_code;
}

gchar* gimo_dup_error_string (void)
{
    gchar *str = NULL;

    if (error_message) {
        G_LOCK (error_lock);
        str = g_strdup (error_message);
        G_UNLOCK (error_lock);
    }
    else {
        str = g_strdup (gimo_error_to_string (error_code));
    }

    return str;
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
    switch (code) {
        GIMO_ERROR_LIST (GIMO_LIST_TO_STRING,
                         GIMO_LIST_TO_STRING);
    default:
        break;
    }

    return NULL;
}
