/* hyscan-gtk-device-manual.h
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
 * SECTION: hyscan-gtk-device-manual
 * @Title HyScanGtkDeviceManual
 * @Short_description ручной ввод устройства.
 *
 * Виджет позволяет вручную выбрать драйвер, название устройства и uri.
 */

#include "hyscan-gtk-device-manual.h"
#include <hyscan-driver.h>

#define WIDGET_RESOURCE_UI "/org/hyscan/gtk/hyscan-gtk-device-manual.ui"

enum
{
  PROP_0,
  PROP_DRIVERS,
};

struct _HyScanGtkDeviceManualPrivate
{
  gchar           **paths;  /* Пути к драйверам. */
  GtkEntry         *name;   /* Название. */
  GtkComboBoxText  *driver; /* Драйвер. */
  GtkEntry         *uri;    /* УРИ. */
};

static void    hyscan_gtk_device_manual_set_property             (GObject               *object,
                                                                  guint                  prop_id,
                                                                  const GValue          *value,
                                                                  GParamSpec            *pspec);
static void    hyscan_gtk_device_manual_object_constructed       (GObject               *object);
static void    hyscan_gtk_device_manual_object_finalize          (GObject               *object);
static void    hyscan_gtk_device_manual_changed (HyScanGtkDeviceManual *self);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkDeviceManual, hyscan_gtk_device_manual, GTK_TYPE_DIALOG);

static void
hyscan_gtk_device_manual_class_init (HyScanGtkDeviceManualClass *klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);
  GtkWidgetClass *wclass = GTK_WIDGET_CLASS (klass);

  oclass->set_property = hyscan_gtk_device_manual_set_property;
  oclass->constructed = hyscan_gtk_device_manual_object_constructed;
  oclass->finalize = hyscan_gtk_device_manual_object_finalize;

  gtk_widget_class_set_template_from_resource (wclass, WIDGET_RESOURCE_UI);

  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkDeviceManual, name);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkDeviceManual, driver);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkDeviceManual, uri);

  gtk_widget_class_bind_template_callback_full (wclass,
    "name_changed", (GCallback)hyscan_gtk_device_manual_changed);
  gtk_widget_class_bind_template_callback_full (wclass,
    "driver_changed", (GCallback)hyscan_gtk_device_manual_changed);
  gtk_widget_class_bind_template_callback_full (wclass,
    "uri_changed", (GCallback)hyscan_gtk_device_manual_changed);

  g_object_class_install_property (oclass, PROP_DRIVERS,
    g_param_spec_pointer ("drivers", "Drivers", "Drivers search paths",
                          G_PARAM_CONSTRUCT | G_PARAM_WRITABLE));
}

static void
hyscan_gtk_device_manual_init (HyScanGtkDeviceManual *self)
{
  self->priv = hyscan_gtk_device_manual_get_instance_private (self);
  gtk_widget_init_template (GTK_WIDGET (self));
}

static void
hyscan_gtk_device_manual_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  HyScanGtkDeviceManual *self = HYSCAN_GTK_DEVICE_MANUAL (object);

  switch (prop_id)
    {
    case PROP_DRIVERS:
      self->priv->paths = g_strdupv ((gchar**)g_value_get_pointer (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
hyscan_gtk_device_manual_object_constructed (GObject *object)
{
  HyScanGtkDeviceManual *self = HYSCAN_GTK_DEVICE_MANUAL (object);
  HyScanGtkDeviceManualPrivate *priv = self->priv;
  gchar **path, **drivers, **driv;
  GArray *dlist;

  G_OBJECT_CLASS (hyscan_gtk_device_manual_parent_class)->constructed (object);

  /* Список названий, из него заполним комбобокс. */
  dlist = g_array_new (TRUE, FALSE, sizeof (gchar*));

  /* Проходим по всем папкам с драйверами. */
  for (path = priv->paths; path != NULL && *path != NULL; ++path)
    {
      /* Из каждого пути получаем список доступных драйверов. */
      drivers = hyscan_driver_list (*path);
      for (driv = drivers; driv != NULL && *driv != NULL; ++driv)
        g_array_append_val (dlist, *driv);

      g_free (drivers);
    }

  /* Для красоты сортируем список драйверов, запихиваем его в комбобокс. */
  g_array_sort (dlist, (GCompareFunc)g_strcmp0);
  drivers = (gchar**)g_array_free (dlist, FALSE);
  for (driv = drivers; driv != NULL && *driv != NULL; ++driv)
    gtk_combo_box_text_insert (priv->driver, -1, *driv, *driv);

  g_strfreev (drivers);
}

static void
hyscan_gtk_device_manual_object_finalize (GObject *object)
{
  HyScanGtkDeviceManual *self = HYSCAN_GTK_DEVICE_MANUAL (object);

  g_strfreev (self->priv->paths);

  G_OBJECT_CLASS (hyscan_gtk_device_manual_parent_class)->finalize (object);
}

/* Функция настривает активность кнопки добавления устройства. */
static gboolean
hyscan_gtk_device_manual_sane (HyScanGtkDeviceManual *self)
{
  HyScanGtkDeviceManualPrivate *priv = self->priv;
  const gchar *name, *uri, *driver;

  name = gtk_entry_get_text (priv->name);
  uri = gtk_entry_get_text (priv->uri);
  driver = gtk_combo_box_get_active_id (GTK_COMBO_BOX (priv->driver));

  if (name == NULL || name[0] == '\0' ||
      uri == NULL || uri[0] == '\0' ||
      driver == NULL || driver[0] == '\0')
    {
      return FALSE;
    }

  return TRUE;
}

/* Обработчик пользовательского ввода. */
static void
hyscan_gtk_device_manual_changed (HyScanGtkDeviceManual *self)
{
  gboolean sane = hyscan_gtk_device_manual_sane (self);
  gtk_dialog_set_response_sensitive (GTK_DIALOG (self), GTK_RESPONSE_OK, sane);
}

/**
 * hyscan_gtk_device_manual_new:
 * @paths: пути к драйверам.
 *
 * Функция создает виджет.
 * @Returns: (transfer full) виджет.
 */
GtkWidget *
hyscan_gtk_device_manual_new (gchar **paths)
{
  return g_object_new (HYSCAN_TYPE_GTK_DEVICE_MANUAL,
                       "drivers", paths,
                       NULL);
}

/**
 * hyscan_gtk_device_manual_get_device:
 * @self: #HyScanGtkDeviceManual
 *
 * Функция возвращает устройство. У него будут заданы пути к драйверам,
 * название, драйвер и uri.
 * @Returns: (transfer full) свежесозданное устройство.
 */
HyScanProfileHWDevice *
hyscan_gtk_device_manual_get_device (HyScanGtkDeviceManual *self)
{
  HyScanGtkDeviceManualPrivate *priv;
  HyScanProfileHWDevice *device;
  const gchar *name, *uri, *driver;

  g_return_val_if_fail (HYSCAN_IS_GTK_DEVICE_MANUAL (self), NULL);
  g_return_val_if_fail (hyscan_gtk_device_manual_sane (self), NULL);
  priv = self->priv;

  name = gtk_entry_get_text (priv->name);
  uri = gtk_entry_get_text (priv->uri);
  driver = gtk_combo_box_get_active_id (GTK_COMBO_BOX (priv->driver));

  device = hyscan_profile_hw_device_new (priv->paths);
  hyscan_profile_hw_device_set_name (device, name);
  hyscan_profile_hw_device_set_uri (device, uri);
  hyscan_profile_hw_device_set_driver (device, driver);

  hyscan_profile_hw_device_update (device);
  return device;
}
