/* hyscan-gtk-device-auto.h
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

/**
 * SECTION: hyscan-gtk-device-auto
 * @Title HyScanGtkDeviceAuto
 * @Short_description
 *
 */

#include "hyscan-gtk-device-auto.h"
#include <hyscan-driver.h>
#include <glib/gi18n-lib.h>

#define WIDGET_RESOURCE_UI "/org/hyscan/gtk/hyscan-gtk-device-auto.ui"
#define HYSCAN_GTK_PROFILE_HW_DEVICE "hyscan-gtk-profile-hw-device"

enum
{
  PROP_0,
  PROP_DRIVERS,
};

typedef struct
{
  gchar    *name;
  gdouble   progress;
  gboolean  completed;
} HyScanGtkDeviceAutoDriver;

struct _HyScanGtkDeviceAutoPrivate
{
  gchar                **paths;

  GHashTable            *drivs;        /* {HyScanDriver*: HyScanGtkDeviceAutoDriver*} */
  GMutex                 lock;

  GtkListBox            *device_list;  /* Список устройств. */
  GtkButton             *refresh;
  GtkButton             *stop;

  // GList                 *devices;
  HyScanProfileHWDevice *selected_device;
};

static void    hyscan_gtk_device_auto_set_property       (GObject               *object,
                                                          guint                  prop_id,
                                                          const GValue          *value,
                                                          GParamSpec            *pspec);
static void    hyscan_gtk_device_auto_object_constructed (GObject               *object);
static void    hyscan_gtk_device_auto_object_finalize    (GObject               *object);

static void    hyscan_gtk_device_auto_driver_free        (gpointer data);

static void    hyscan_gtk_device_auto_driver_list        (HyScanGtkDeviceAuto  *self,
                                                          gchar               **paths);

static void    hyscan_gtk_device_auto_update_ui          (gpointer udata);

static void    hyscan_gtk_device_auto_selected           (HyScanGtkDeviceAuto *self,
                                                          GtkListBoxRow       *row,
                                                          GtkListBox          *listbox);

static void    hyscan_gtk_device_auto_refresh_helper     (gpointer             key,
                                                          gpointer             value,
                                                          gpointer             user_data);

static void    hyscan_gtk_device_auto_refresh            (HyScanGtkDeviceAuto *self);

static void    hyscan_gtk_device_auto_refresh_stop       (HyScanGtkDeviceAuto *self);

static void    hyscan_gtk_device_auto_progress_helper    (HyScanGtkDeviceAuto *self,
                                                          HyScanDiscover      *driver,
                                                          gdouble              progress,
                                                          gboolean             completed);

static void    hyscan_gtk_device_auto_progress           (HyScanDiscover      *driver,
                                                          gdouble              progress,
                                                          HyScanGtkDeviceAuto *self);

static void    hyscan_gtk_device_auto_completed          (HyScanDiscover      *driver,
                                                          HyScanGtkDeviceAuto *self);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkDeviceAuto, hyscan_gtk_device_auto, GTK_TYPE_DIALOG);

static void
hyscan_gtk_device_auto_class_init (HyScanGtkDeviceAutoClass *klass)
{
  GtkWidgetClass *wclass = GTK_WIDGET_CLASS (klass);
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  oclass->set_property = hyscan_gtk_device_auto_set_property;
  oclass->constructed = hyscan_gtk_device_auto_object_constructed;
  oclass->finalize = hyscan_gtk_device_auto_object_finalize;

  gtk_widget_class_set_template_from_resource (wclass, WIDGET_RESOURCE_UI);

  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkDeviceAuto, device_list);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkDeviceAuto, refresh);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkDeviceAuto, stop);

  gtk_widget_class_bind_template_callback_full (wclass,
    "selected", (GCallback)hyscan_gtk_device_auto_selected);
  gtk_widget_class_bind_template_callback_full (wclass,
    "refresh", (GCallback)hyscan_gtk_device_auto_refresh);
  gtk_widget_class_bind_template_callback_full (wclass,
    "stop", (GCallback)hyscan_gtk_device_auto_refresh_stop);

  g_object_class_install_property (oclass, PROP_DRIVERS,
    g_param_spec_pointer ("drivers", "Drivers", "Drivers search paths",
                          G_PARAM_CONSTRUCT | G_PARAM_WRITABLE));
}

static void
hyscan_gtk_device_auto_init (HyScanGtkDeviceAuto *self)
{
  HyScanGtkDeviceAutoPrivate *priv;

  self->priv = hyscan_gtk_device_auto_get_instance_private (self);
  gtk_widget_init_template (GTK_WIDGET (self));
  priv = self->priv;

  g_mutex_init (&priv->lock);
  priv->drivs = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                       g_object_unref,
                                       hyscan_gtk_device_auto_driver_free);
}

static void
hyscan_gtk_device_auto_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  HyScanGtkDeviceAuto *self = HYSCAN_GTK_DEVICE_AUTO (object);

  switch (prop_id)
    {
    case PROP_DRIVERS:
      self->priv->paths = g_strdupv ((gchar**)g_value_get_pointer (value));
      hyscan_gtk_device_auto_driver_list (self, self->priv->paths);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
hyscan_gtk_device_auto_object_constructed (GObject *object)
{
  // HyScanGtkDeviceAuto *self = HYSCAN_GTK_DEVICE_AUTO (object);
  // HyScanGtkDeviceAutoPrivate *priv = self->priv;

  G_OBJECT_CLASS (hyscan_gtk_device_auto_parent_class)->constructed (object);
}

static void
hyscan_gtk_device_auto_object_finalize (GObject *object)
{
  HyScanGtkDeviceAuto *self = HYSCAN_GTK_DEVICE_AUTO (object);
  HyScanGtkDeviceAutoPrivate *priv = self->priv;

  g_hash_table_unref (priv->drivs);
  g_strfreev (priv->paths);

  G_OBJECT_CLASS (hyscan_gtk_device_auto_parent_class)->finalize (object);
}

static void
hyscan_gtk_device_auto_driver_free (gpointer data)
{
  HyScanGtkDeviceAutoDriver *info;
  info = (HyScanGtkDeviceAutoDriver*)data;

  g_free (info->name);
  g_free (info);
}

static void
hyscan_gtk_device_auto_driver_list (HyScanGtkDeviceAuto  *self,
                                    gchar               **paths)
{
  HyScanGtkDeviceAutoDriver *info;
  HyScanDriver *driver;
  gchar **path, **drivers, **driv;

  /* Проходим по всем папкам с драйверами. */
  for (path = paths; path != NULL && *path != NULL; ++path)
    {
      /* Из каждого пути получаем список доступных драйверов. */
      drivers = hyscan_driver_list (*path);
      for (driv = drivers; driv != NULL && *driv != NULL; ++driv)
        {
          driver = hyscan_driver_new (*path, *driv);

          if (driver == NULL)
            continue;

          /* Подключение сигналов. */
          g_signal_connect (driver, "progress",
                            G_CALLBACK (hyscan_gtk_device_auto_progress),
                            self);
          g_signal_connect (driver, "completed",
                            G_CALLBACK (hyscan_gtk_device_auto_completed),
                            self);

          /* Это для автоматического обнаружения. */
          info = g_new0 (HyScanGtkDeviceAutoDriver, 1);
          info->name = g_strdup (*driv);
          g_hash_table_insert (self->priv->drivs, driver, info);
        }

      g_free (drivers);
    }
}


static void
hyscan_gtk_device_auto_update_ui (gpointer udata)
{
  HyScanGtkDeviceAuto *self = udata;
  HyScanGtkDeviceAutoPrivate *priv = self->priv;
  GHashTableIter iter;
  gpointer v;
  gdouble overall = 0, maximum = 0;
  gboolean completed = TRUE;

  g_mutex_lock (&self->priv->lock);
  g_hash_table_iter_init (&iter, self->priv->drivs);
  while (g_hash_table_iter_next (&iter, NULL, &v))
    {
      HyScanGtkDeviceAutoDriver *info = v;

      maximum += 100.;
      overall += info->progress;
      completed &= info->completed;
    }
  g_mutex_unlock (&self->priv->lock);

  gtk_widget_set_sensitive (GTK_WIDGET (priv->refresh), completed);
  gtk_widget_set_sensitive (GTK_WIDGET (priv->stop), !completed);

  if (completed)
    {
      gtk_button_set_label (priv->refresh, _("Refresh"));
    }
  else
    {
      gchar *percentage = g_strdup_printf ("%2.0f%%", 100. * overall / maximum);
      gtk_button_set_label (priv->refresh, percentage);
      g_free (percentage);
    }
}

static GtkWidget *
hyscan_gtk_device_auto_make_row (HyScanProfileHWDevice *device)
{
  GtkWidget *row, *label;

  row = gtk_list_box_row_new ();

  label = gtk_label_new (hyscan_profile_hw_device_get_name (device));
  gtk_widget_set_margin_top (label, 12);
  gtk_widget_set_margin_bottom (label, 12);

  g_object_set_data_full (G_OBJECT (row), HYSCAN_GTK_PROFILE_HW_DEVICE,
                          g_object_ref (device), g_object_unref);

  gtk_container_add (GTK_CONTAINER (row), label);
  gtk_widget_show_all (row);

  return row;
}

/* Вспомогательная функция выбора или очистки выбранного девайса. */
static void
hyscan_gtk_device_auto_select_helper (HyScanGtkDeviceAuto *self,
                                                   HyScanProfileHWDevice          *device)
{
  HyScanGtkDeviceAutoPrivate *priv = self->priv;

  g_clear_object (&priv->selected_device);

  if (device == NULL)
    return;

  priv->selected_device = g_object_ref (device);
  gtk_dialog_set_response_sensitive (GTK_DIALOG (self), GTK_RESPONSE_OK, device != NULL);
}

static void
hyscan_gtk_device_auto_selected (HyScanGtkDeviceAuto *self,
                                              GtkListBoxRow                  *row,
                                              GtkListBox                     *listbox)
{
  HyScanProfileHWDevice *device = NULL;

  if (row != NULL)
    device = g_object_get_data (G_OBJECT (row), HYSCAN_GTK_PROFILE_HW_DEVICE);

  hyscan_gtk_device_auto_select_helper (self, device);
}

static void
hyscan_gtk_device_auto_refresh_prepare (gpointer key,
                                                     gpointer value,
                                                     gpointer user_data)
{
  HyScanGtkDeviceAutoDriver *info = value;
  gboolean start = GPOINTER_TO_INT (user_data);

  info->completed = !start;
  info->progress = start ? 0.0 : 100.0;
}

static void
hyscan_gtk_device_auto_refresh_helper (gpointer key,
                                                    gpointer value,
                                                    gpointer user_data)
{
  HyScanDriver *driver = key;
  gboolean start = GPOINTER_TO_INT (user_data);

  if (start)
    hyscan_discover_start (HYSCAN_DISCOVER (driver));
  else
    hyscan_discover_stop (HYSCAN_DISCOVER (driver));
}

static void
hyscan_gtk_device_auto_refresh (HyScanGtkDeviceAuto *self)
{
  g_mutex_lock (&self->priv->lock);
  g_hash_table_foreach (self->priv->drivs,
                        hyscan_gtk_device_auto_refresh_prepare,
                        GINT_TO_POINTER (TRUE));
  g_mutex_unlock (&self->priv->lock);
  g_hash_table_foreach (self->priv->drivs,
                        hyscan_gtk_device_auto_refresh_helper,
                        GINT_TO_POINTER (TRUE));
}

static void
hyscan_gtk_device_auto_refresh_stop (HyScanGtkDeviceAuto *self)
{
  g_hash_table_foreach (self->priv->drivs,
                        hyscan_gtk_device_auto_refresh_helper,
                        GINT_TO_POINTER (FALSE));
  g_mutex_lock (&self->priv->lock);
  g_hash_table_foreach (self->priv->drivs,
                        hyscan_gtk_device_auto_refresh_prepare,
                        GINT_TO_POINTER (FALSE));
  g_mutex_unlock (&self->priv->lock);
}

static void
hyscan_gtk_device_auto_progress_helper (HyScanGtkDeviceAuto *self,
                                                     HyScanDiscover                 *driver,
                                                     gdouble                         progress,
                                                     gboolean                        completed)
{
  HyScanGtkDeviceAutoDriver *info;

  info = g_hash_table_lookup (self->priv->drivs, driver);

  g_mutex_lock (&self->priv->lock);

  info->progress = progress;
  info->completed = completed;

  g_mutex_unlock (&self->priv->lock);
  g_idle_add ((GSourceFunc)hyscan_gtk_device_auto_update_ui, self);
}

static void
hyscan_gtk_device_auto_progress (HyScanDiscover                 *driver,
                                              gdouble                         progress,
                                              HyScanGtkDeviceAuto *self)
{
  hyscan_gtk_device_auto_progress_helper (self, driver, progress, FALSE);
}

static void
hyscan_gtk_device_auto_completed (HyScanDiscover                 *driver,
                                               HyScanGtkDeviceAuto *self)
{
  HyScanGtkDeviceAutoPrivate *priv = self->priv;
  HyScanGtkDeviceAutoDriver *driver_info;
  HyScanDiscoverInfo *info;
  HyScanProfileHWDevice *device;
  const gchar *driver_name;
  GList *list, *link;

  hyscan_gtk_device_auto_progress_helper (self, driver, 100.0, TRUE);
  list = hyscan_discover_list (driver);

  /* Определяю название этого драйвера. */
  driver_info = g_hash_table_lookup (priv->drivs, driver);
  driver_name = driver_info->name;

  for (link = list; link != NULL; link = link->next)
    {
      GtkWidget *row;
      info = link->data;

      device = hyscan_profile_hw_device_new (priv->paths);
      hyscan_profile_hw_device_set_name (device, info->name);
      hyscan_profile_hw_device_set_uri (device, info->uri);
      hyscan_profile_hw_device_set_driver (device, driver_name);

      row = hyscan_gtk_device_auto_make_row (device);

      gtk_list_box_insert (priv->device_list, row, -1);
    }

  g_list_free_full (list, (GDestroyNotify)hyscan_discover_info_free);
}

GtkWidget *
hyscan_gtk_device_auto_new (gchar **drivers)
{
  return g_object_new (HYSCAN_TYPE_GTK_DEVICE_AUTO,
                       "drivers", drivers,
                       NULL);
}

HyScanProfileHWDevice *
hyscan_gtk_device_auto_get_device (HyScanGtkDeviceAuto *self)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_DEVICE_AUTO (self), NULL);

  if (self->priv->selected_device == NULL)
    return NULL;

  return g_object_ref (self->priv->selected_device);
}
