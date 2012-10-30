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
#include "gimo-dlmodule.h"
#include "gimo-error.h"

struct _GimoDlmodulePrivate {
    GModule *module;
};

static void gimo_loadable_interface_init (GimoLoadableInterface *iface);
static void gimo_module_interface_init (GimoModuleInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GimoDlmodule, gimo_dlmodule, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_LOADABLE,
                                                gimo_loadable_interface_init);
                         G_IMPLEMENT_INTERFACE (GIMO_TYPE_MODULE,
                                                gimo_module_interface_init))

static gboolean _gimo_dlmodule_open (GimoModule *module,
                                     const gchar *file_name)
{
    GimoDlmodule *self = GIMO_DLMODULE (module);
    GimoDlmodulePrivate *priv = self->priv;

    if (priv->module)
        gimo_set_error_return_val (GIMO_ERROR_CONFLICT, FALSE);

    priv->module = g_module_open (file_name, G_MODULE_BIND_LAZY);
    if (NULL == priv->module) {
        gimo_set_error_full (GIMO_ERROR_UNLOAD,
                             "Dlmodule: open module error: %s: %s",
                             file_name,
                             g_module_error ());
        return FALSE;
    }

    return TRUE;
}

static gboolean _gimo_dlmodule_close (GimoModule *module)
{
    GimoDlmodule *self = GIMO_DLMODULE (module);
    GimoDlmodulePrivate *priv = self->priv;

    if (NULL == priv->module)
        return TRUE;

    if (!g_module_close (priv->module)) {
        gimo_set_error_full (GIMO_ERROR_UNLOAD,
                             "Dlmodule: close module error: %s",
                             g_module_error ());
        return FALSE;
    }

    priv->module = NULL;
    return TRUE;
}

static const gchar* _gimo_dlmodule_get_name (GimoModule *module)
{
    GimoDlmodule *self = GIMO_DLMODULE (module);
    GimoDlmodulePrivate *priv = self->priv;

    if (priv->module)
        return g_module_name (priv->module);

    return NULL;
}

static GObject* _gimo_dlmodule_resolve (GimoModule *module,
                                        const gchar *symbol,
                                        GObject *param)
{
    GimoDlmodule *self = GIMO_DLMODULE (module);
    GimoDlmodulePrivate *priv = self->priv;
    GObject* (*new_object) (GObject*) = NULL;

    if (!priv->module)
        return NULL;

    if (!g_module_symbol (priv->module,
                          symbol,
                          (gpointer *) &new_object))
    {
        gimo_set_error (GIMO_ERROR_NO_SYMBOL);
        return NULL;
    }

    if (NULL == new_object)
        return NULL;

    return new_object (param);
}

static void gimo_loadable_interface_init (GimoLoadableInterface *iface)
{
    iface->load = (GimoLoadableLoadFunc) _gimo_dlmodule_open;
}

static void gimo_module_interface_init (GimoModuleInterface *iface)
{
    iface->open = _gimo_dlmodule_open;
    iface->close = _gimo_dlmodule_close;
    iface->get_name = _gimo_dlmodule_get_name;
    iface->resolve = _gimo_dlmodule_resolve;
}

static void gimo_dlmodule_init (GimoDlmodule *self)
{
    GimoDlmodulePrivate *priv;

    self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                              GIMO_TYPE_DLMODULE,
                                              GimoDlmodulePrivate);
    priv = self->priv;

    priv->module = NULL;
}

static void gimo_dlmodule_finalize (GObject *gobject)
{
    /* NOTE: Don't close module, otherwise valgrind will
     *       report memory leak. */
    if (0)
        _gimo_dlmodule_close (GIMO_MODULE (gobject));

    G_OBJECT_CLASS (gimo_dlmodule_parent_class)->finalize (gobject);
}

static void gimo_dlmodule_class_init (GimoDlmoduleClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = gimo_dlmodule_finalize;

    g_type_class_add_private (gobject_class,
                              sizeof (GimoDlmodulePrivate));
}

GimoDlmodule* gimo_dlmodule_new (void)
{
    return g_object_new (GIMO_TYPE_DLMODULE, NULL);
}

GModule* _gimo_dlmodule_get_gmodule (GimoDlmodule *self)
{
    g_return_val_if_fail (GIMO_IS_DLMODULE (self), NULL);

    return self->priv->module;
}
