/* hyscan-gtk-profile-editor-hw-device.c
 *
 * Copyright 2020 Screen LLC, Alexander Dmitriev <m1n7@yandex.ru>
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
 * SECTION: hyscan-gtk-profile-editor-hw-device
 * @Title HyScanGtkProfileEditorHWDevice
 * @Short_description: Виджет редактирования устройств в составе профиля оборудования
 *
 */

#include "hyscan-gtk-profile-editor-hw-device.h"
#include <hyscan-driver.h>
#include <hyscan-data-box.h>
#include <hyscan-gtk-param-tree.h>

#define WIDGET_RESOURCE_UI "/org/hyscan/gtk/hyscan-gtk-profile-editor-hw-device.ui"

enum
{
  SIGNAL_CHANGED,
  SIGNAL_LAST
};

enum
{
  PROP_0,
  PROP_PROFILE
};

struct _HyScanGtkProfileEditorHWDevicePrivate
{
  HyScanProfileHWDevice *device;

  GtkEntry              *name;
  GtkEntry              *uri;
  GtkComboBoxText       *driver;
  GtkExpander           *expander;

  HyScanGtkParam        *param;
};

static void    hyscan_gtk_profile_editor_hw_device_set_property       (GObject               *object,
                                                                       guint                  prop_id,
                                                                       const GValue          *value,
                                                                       GParamSpec            *pspec);
static void    hyscan_gtk_profile_editor_hw_device_object_constructed (GObject               *object);
static void    hyscan_gtk_profile_editor_hw_device_object_finalize    (GObject               *object);
static void    hyscan_gtk_profile_editor_hw_device_name_changed       (HyScanGtkProfileEditorHWDevice *self);
static void    hyscan_gtk_profile_editor_hw_device_uri_changed        (HyScanGtkProfileEditorHWDevice *self);
static void    hyscan_gtk_profile_editor_hw_device_driver_changed     (HyScanGtkProfileEditorHWDevice *self);
static void    hyscan_gtk_profile_editor_hw_device_update             (HyScanGtkProfileEditorHWDevice *self);

static guint   hyscan_gtk_profile_editor_hw_device_signals[SIGNAL_LAST] = {0};

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkProfileEditorHWDevice, hyscan_gtk_profile_editor_hw_device, GTK_TYPE_GRID);

static void
hyscan_gtk_profile_editor_hw_device_class_init (HyScanGtkProfileEditorHWDeviceClass *klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);
  GtkWidgetClass *wclass = GTK_WIDGET_CLASS (klass);

  oclass->set_property = hyscan_gtk_profile_editor_hw_device_set_property;
  oclass->constructed = hyscan_gtk_profile_editor_hw_device_object_constructed;
  oclass->finalize = hyscan_gtk_profile_editor_hw_device_object_finalize;

  gtk_widget_class_set_template_from_resource (wclass, WIDGET_RESOURCE_UI);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHWDevice, name);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHWDevice, uri);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHWDevice, driver);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHWDevice, expander);

  g_object_class_install_property (oclass, PROP_PROFILE,
    g_param_spec_object ("device", "Device", "HW Device Profile",
                         HYSCAN_TYPE_PROFILE_HW_DEVICE,
                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  hyscan_gtk_profile_editor_hw_device_signals[SIGNAL_CHANGED] =
    g_signal_new ("changed", HYSCAN_TYPE_GTK_PROFILE_EDITOR_HW_DEVICE,
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
hyscan_gtk_profile_editor_hw_device_init (HyScanGtkProfileEditorHWDevice *self)
{
  HyScanGtkProfileEditorHWDevicePrivate *priv;

  self->priv = hyscan_gtk_profile_editor_hw_device_get_instance_private (self);
  gtk_widget_init_template (GTK_WIDGET (self));
  priv = self->priv;

  priv->param = HYSCAN_GTK_PARAM (hyscan_gtk_param_tree_new (NULL, NULL, FALSE));
  hyscan_gtk_param_set_immidiate (HYSCAN_GTK_PARAM (priv->param), TRUE);

  gtk_container_add (GTK_CONTAINER (priv->expander), GTK_WIDGET (priv->param));
}

static void
hyscan_gtk_profile_editor_hw_device_set_property (GObject      *object,
                                                  guint         prop_id,
                                                  const GValue *value,
                                                  GParamSpec   *pspec)
{
  HyScanGtkProfileEditorHWDevice *self = HYSCAN_GTK_PROFILE_EDITOR_HW_DEVICE (object);
  HyScanGtkProfileEditorHWDevicePrivate *priv = self->priv;

  switch (prop_id)
    {
    case PROP_PROFILE:
      priv->device = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
hyscan_gtk_profile_editor_hw_device_object_constructed (GObject *object)
{
  HyScanGtkProfileEditorHWDevice *self = HYSCAN_GTK_PROFILE_EDITOR_HW_DEVICE (object);
  HyScanGtkProfileEditorHWDevicePrivate *priv = self->priv;
  const gchar *name, *uri;

  G_OBJECT_CLASS (hyscan_gtk_profile_editor_hw_device_parent_class)->constructed (object);

  /* Сигналы виджетов. */
  g_signal_connect_swapped (priv->name, "changed",
                            G_CALLBACK (hyscan_gtk_profile_editor_hw_device_name_changed),
                            self);
  g_signal_connect_swapped (priv->uri, "changed",
                            G_CALLBACK (hyscan_gtk_profile_editor_hw_device_uri_changed),
                            self);
  g_signal_connect_swapped (priv->driver, "changed",
                            G_CALLBACK (hyscan_gtk_profile_editor_hw_device_driver_changed),
                            self);

  /* Считываем название профиля, uri. */
  name = hyscan_profile_hw_device_get_name (priv->device);
  name != NULL ? gtk_entry_set_text (priv->name, name) : 0;
  uri = hyscan_profile_hw_device_get_uri (priv->device);
  uri != NULL ? gtk_entry_set_text (priv->uri, uri) : 0;

  /* Заполняем комбобокс с драйверами. */
  {
    GArray *driver_list = g_array_new (TRUE, FALSE, sizeof(gchar*));
    const gchar **paths, **path;
    gchar **drivers, **driv;

    /* Из каждого пути получаем список доступных драйверов. */
    paths = hyscan_profile_hw_device_get_paths (priv->device);
    for (path = paths; path != NULL && *path != NULL; ++path)
      {
        drivers = hyscan_driver_list (*path);
        for (driv = drivers; driv != NULL && *driv != NULL; ++driv)
          g_array_append_val (driver_list, *driv);

        g_free (drivers);
      }

    /* Для красоты сортируем список драйверов, запихиваем его в комбобокс. */
    g_array_sort (driver_list, (GCompareFunc)g_strcmp0);
    drivers = (gchar**)g_array_free (driver_list, FALSE);
    for (driv = drivers; driv != NULL && *driv != NULL; ++driv)
      gtk_combo_box_text_insert (priv->driver, -1, *driv, *driv);

    /* Выбираем выбранный драйвер (тьфу). */
    gtk_combo_box_set_active_id (GTK_COMBO_BOX (priv->driver),
                                 hyscan_profile_hw_device_get_driver (priv->device));
  }

  hyscan_gtk_profile_editor_hw_device_update (self);
}

static void
hyscan_gtk_profile_editor_hw_device_object_finalize (GObject *object)
{
  HyScanGtkProfileEditorHWDevice *self = HYSCAN_GTK_PROFILE_EDITOR_HW_DEVICE (object);
  HyScanGtkProfileEditorHWDevicePrivate *priv = self->priv;

  g_clear_object (&priv->device);

  G_OBJECT_CLASS (hyscan_gtk_profile_editor_hw_device_parent_class)->finalize (object);
}

/* Обработчик смены названия устройства. */
static void
hyscan_gtk_profile_editor_hw_device_name_changed (HyScanGtkProfileEditorHWDevice *self)
{
  const gchar *name, *text;

  name = hyscan_profile_hw_device_get_name (self->priv->device);
  text = gtk_entry_get_text (self->priv->name);

  if (0 == g_strcmp0 (name, text))
    return;

  hyscan_profile_hw_device_set_name (self->priv->device, text);

  g_signal_emit (self, hyscan_gtk_profile_editor_hw_device_signals[SIGNAL_CHANGED], 0);
}

/* Обработчик смены адреса. */
static void
hyscan_gtk_profile_editor_hw_device_uri_changed (HyScanGtkProfileEditorHWDevice *self)
{
  const gchar *uri, *text;

  uri = hyscan_profile_hw_device_get_uri (self->priv->device);
  text = gtk_entry_get_text (self->priv->uri);

  if (0 == g_strcmp0 (uri, text))
    return;

  hyscan_profile_hw_device_set_uri (self->priv->device, text);
  hyscan_gtk_profile_editor_hw_device_update (self);
}

/* Обработчик смены драйвера. */
static void
hyscan_gtk_profile_editor_hw_device_driver_changed (HyScanGtkProfileEditorHWDevice *self)
{
  const gchar *driver, *text;

  driver = hyscan_profile_hw_device_get_driver (self->priv->device);
  text = gtk_combo_box_get_active_id (GTK_COMBO_BOX (self->priv->driver));

  if (0 == g_strcmp0 (driver, text))
    return;

  hyscan_profile_hw_device_set_driver (self->priv->device, text);
  hyscan_gtk_profile_editor_hw_device_update (self);
}

/* Функция обновляет схему устройства. */
static void
hyscan_gtk_profile_editor_hw_device_update (HyScanGtkProfileEditorHWDevice *self)
{
  HyScanGtkProfileEditorHWDevicePrivate *priv = self->priv;
  gboolean update_state;

  update_state = hyscan_profile_hw_device_update (priv->device);
  if (update_state)
    hyscan_gtk_param_set_param (priv->param, HYSCAN_PARAM (priv->device), "/", FALSE);
  else
    hyscan_gtk_param_set_param (priv->param, NULL, "/", FALSE);

  // gtk_revealer_set_reveal_child (priv->revealer, update_state);
  gtk_expander_set_expanded (priv->expander, update_state);
  gtk_widget_set_sensitive (GTK_WIDGET (priv->expander), update_state);

  g_signal_emit (self, hyscan_gtk_profile_editor_hw_device_signals[SIGNAL_CHANGED], 0);
}

/*
 * hyscan_gtk_profile_editor_hw_device_new:
 * @device: редактируемый HyScanProfileHWDevice
 *
 * Returns: (transfer full): виджет редактирования устройства.
 */
GtkWidget *
hyscan_gtk_profile_editor_hw_device_new (HyScanProfileHWDevice *device)
{
  return g_object_new (HYSCAN_TYPE_GTK_PROFILE_EDITOR_HW_DEVICE,
                       "device", device,
                       NULL);
}

/*
 * hyscan_gtk_profile_editor_hw_device_get_device:
 * @self: редактор устройства
 *
 * Returns: (transfer full): HyScanProfileHWDevice. При этом нельзя что-либо менять
 * в устройстве, т.к. редактор об этом не узнает.
 */
HyScanProfileHWDevice *
hyscan_gtk_profile_editor_hw_device_get_device (HyScanGtkProfileEditorHWDevice *self)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_PROFILE_EDITOR_HW_DEVICE (self), NULL);

  return g_object_ref (self->priv->device);
}
