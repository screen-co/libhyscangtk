/* hyscan-gtk-profile-hw.h
 *
 * Copyright 2019 Screen LLC, Alexander Dmitriev <m1n7@yandex.ru>
 *
 * This file is part of HyScanGtk.
 *
 * HyScanGtk is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HyScanGtk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Alternatively, you can license this code under a commercial license.
 * Contact the Screen LLC in this case - <info@screen-co.ru>.
 */

/* HyScanGtk имеет двойную лицензию.
 *
 * Во-первых, вы можете распространять HyScanGtk на условиях Стандартной
 * Общественной Лицензии GNU версии 3, либо по любой более поздней версии
 * лицензии (по вашему выбору). Полные положения лицензии GNU приведены в
 * <http://www.gnu.org/licenses/>.
 *
 * Во-вторых, этот программный код можно использовать по коммерческой
 * лицензии. Для этого свяжитесь с ООО Экран - <info@screen-co.ru>.
 */

#include "hyscan-gtk-profile-hw.h"

#define HYSCAN_HW_PROFILES_PATH "hw-profiles"

enum
{
  PROP_0,
  PROP_DRIVERS
};

struct _HyScanGtkProfileHWPrivate
{
  gchar **drivers;
};

static void hyscan_gtk_profile_hw_set_property           (GObject      *object,
                                                          guint         prop_id,
                                                          const GValue *value,
                                                          GParamSpec   *pspec);
static void hyscan_gtk_profile_hw_object_finalize        (GObject      *object);

static HyScanProfile * hyscan_gtk_profile_hw_new_profile (HyScanGtkProfile *parent,
                                                          const gchar      *filename);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkProfileHW, hyscan_gtk_profile_hw, HYSCAN_TYPE_GTK_PROFILE);

static void
hyscan_gtk_profile_hw_class_init (HyScanGtkProfileHWClass *klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);
  HyScanGtkProfileClass *pklass = HYSCAN_GTK_PROFILE_CLASS (klass);

  oclass->set_property = hyscan_gtk_profile_hw_set_property;
  oclass->finalize = hyscan_gtk_profile_hw_object_finalize;

  pklass->subfolder = HYSCAN_HW_PROFILES_PATH;
  pklass->new_profile = hyscan_gtk_profile_hw_new_profile;

  g_object_class_install_property (oclass, PROP_DRIVERS,
    g_param_spec_pointer ("drivers", "DriverPaths", "Where to look for drivert",
                          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void
hyscan_gtk_profile_hw_init (HyScanGtkProfileHW *gtk_profile_hw)
{
  gtk_profile_hw->priv = hyscan_gtk_profile_hw_get_instance_private (gtk_profile_hw);
}

static void
hyscan_gtk_profile_hw_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  HyScanGtkProfileHW *profile = HYSCAN_GTK_PROFILE_HW (object);

  switch (prop_id)
    {
    case PROP_DRIVERS:
      profile->priv->drivers = g_strdupv (g_value_get_pointer (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
hyscan_gtk_profile_hw_object_finalize (GObject *object)
{
  HyScanGtkProfileHW *self = HYSCAN_GTK_PROFILE_HW (object);

  g_clear_pointer (&self->priv->drivers, g_strfreev);

  G_OBJECT_CLASS (hyscan_gtk_profile_hw_parent_class)->finalize (object);
}

static HyScanProfile *
hyscan_gtk_profile_hw_new_profile (HyScanGtkProfile *parent,
                                   const gchar      *filename)
{
  HyScanGtkProfileHW *self = HYSCAN_GTK_PROFILE_HW (parent);
  HyScanHWProfile *profile;

  profile = hyscan_hw_profile_new (filename);
  hyscan_hw_profile_set_driver_paths (profile, self->priv->drivers);

  return HYSCAN_PROFILE (profile);
}

GtkWidget *
hyscan_gtk_profile_hw_new (const gchar  *sys,
                           gchar       **drivers)
{
  return g_object_new (HYSCAN_TYPE_GTK_PROFILE_HW,
                       "sysfolder", sys,
                       "drivers", drivers,
                       NULL);
}