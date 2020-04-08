/* hyscan-gtk-profile-hw-device-editor.h
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
 * SECTION: hyscan-gtk-profile-hw-device-editor
 * @Title HyScanGtkProfileHWDeviceEditor
 * @Short_description
 *
 */

#include "hyscan-gtk-profile-hw-device-editor.h"
#include <hyscan-driver.h>
#include <hyscan-data-box.h>
#include <hyscan-gtk-param-tree.h>

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

struct _HyScanGtkProfileHWDeviceEditorPrivate
{
  HyScanProfileHWDevice *device;
  GtkEntry              *name;
  GtkEntry              *uri;
  GtkComboBoxText       *driver;
  HyScanGtkParam        *param;
};

static void    hyscan_gtk_profile_hw_device_editor_set_property       (GObject               *object,
                                                                       guint                  prop_id,
                                                                       const GValue          *value,
                                                                       GParamSpec            *pspec);
static void    hyscan_gtk_profile_hw_device_editor_object_constructed (GObject               *object);
static void    hyscan_gtk_profile_hw_device_editor_object_finalize    (GObject               *object);
static void    hyscan_gtk_profile_hw_device_editor_name_changed       (HyScanGtkProfileHWDeviceEditor *self);
static void    hyscan_gtk_profile_hw_device_editor_uri_changed        (HyScanGtkProfileHWDeviceEditor *self);
static void    hyscan_gtk_profile_hw_device_editor_driver_changed     (HyScanGtkProfileHWDeviceEditor *self);
static void    hyscan_gtk_profile_hw_device_editor_update             (HyScanGtkProfileHWDeviceEditor *self);

static guint   hyscan_gtk_profile_hw_device_editor_signals[SIGNAL_LAST] = {0};

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkProfileHWDeviceEditor, hyscan_gtk_profile_hw_device_editor, GTK_TYPE_GRID);

static void
hyscan_gtk_profile_hw_device_editor_class_init (HyScanGtkProfileHWDeviceEditorClass *klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  oclass->set_property = hyscan_gtk_profile_hw_device_editor_set_property;
  oclass->constructed = hyscan_gtk_profile_hw_device_editor_object_constructed;
  oclass->finalize = hyscan_gtk_profile_hw_device_editor_object_finalize;

  g_object_class_install_property (oclass, PROP_PROFILE,
    g_param_spec_object ("device", "Device", "HW Device Profile",
                         HYSCAN_TYPE_PROFILE_HW_DEVICE,
                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  hyscan_gtk_profile_hw_device_editor_signals[SIGNAL_CHANGED] =
    g_signal_new ("changed", HYSCAN_TYPE_GTK_PROFILE_HW_DEVICE_EDITOR,
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
hyscan_gtk_profile_hw_device_editor_init (HyScanGtkProfileHWDeviceEditor *self)
{
  HyScanGtkProfileHWDeviceEditorPrivate *priv;
  GtkWidget *name_label, *uri_label, *driver_label;
  GtkGrid *grid = GTK_GRID (self);
  gint row = 0;

  priv = hyscan_gtk_profile_hw_device_editor_get_instance_private (self);
  self->priv = priv;

  /* Создаю виджеты. */
  name_label = gtk_label_new ("Name");
  driver_label = gtk_label_new ("Driver");
  uri_label = gtk_label_new ("URI");

  priv->name = GTK_ENTRY (gtk_entry_new ());
  priv->uri = GTK_ENTRY (gtk_entry_new ());
  priv->driver = GTK_COMBO_BOX_TEXT (gtk_combo_box_text_new ());
  priv->param = HYSCAN_GTK_PARAM (hyscan_gtk_param_tree_new (NULL, NULL, FALSE));
  hyscan_gtk_param_set_immidiate (HYSCAN_GTK_PARAM (priv->param), TRUE);

  /* Left, Top, Width, Height. */
  gtk_grid_attach (grid, GTK_WIDGET (name_label), 0, row, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (priv->name), 1, row, 1, 1);
  ++row;

  gtk_grid_attach (grid, GTK_WIDGET (driver_label), 0, row, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (priv->driver), 1, row, 1, 1);
  ++row;

  gtk_grid_attach (grid, GTK_WIDGET (uri_label), 0, row, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (priv->uri), 1, row, 1, 1);
  ++row;

  gtk_grid_attach (grid, GTK_WIDGET (priv->param), 0, row, 2, 1);

  gtk_widget_set_hexpand (GTK_WIDGET (priv->name), TRUE);
  gtk_widget_set_hexpand (GTK_WIDGET (priv->uri), TRUE);
  gtk_widget_set_hexpand (GTK_WIDGET (priv->driver), TRUE);
  gtk_grid_set_row_spacing (grid, 6);
  gtk_grid_set_column_spacing (grid, 6);
  gtk_widget_set_margin_start (GTK_WIDGET (self), 6);
  gtk_widget_set_margin_end (GTK_WIDGET (self), 6);
  gtk_widget_set_margin_top (GTK_WIDGET (self), 6);
  gtk_widget_set_margin_bottom (GTK_WIDGET (self), 6);
  gtk_widget_set_hexpand (GTK_WIDGET (self), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET (self), TRUE);

}

static void
hyscan_gtk_profile_hw_device_editor_set_property (GObject      *object,
                                                  guint         prop_id,
                                                  const GValue *value,
                                                  GParamSpec   *pspec)
{
  HyScanGtkProfileHWDeviceEditor *self = HYSCAN_GTK_PROFILE_HW_DEVICE_EDITOR (object);
  HyScanGtkProfileHWDeviceEditorPrivate *priv = self->priv;

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
hyscan_gtk_profile_hw_device_editor_object_constructed (GObject *object)
{
  HyScanGtkProfileHWDeviceEditor *self = HYSCAN_GTK_PROFILE_HW_DEVICE_EDITOR (object);
  HyScanGtkProfileHWDeviceEditorPrivate *priv = self->priv;
  const gchar *name, *uri;

  G_OBJECT_CLASS (hyscan_gtk_profile_hw_device_editor_parent_class)->constructed (object);

  /* Сигналы виджетов. */
  g_signal_connect_swapped (priv->name, "changed",
                            G_CALLBACK (hyscan_gtk_profile_hw_device_editor_name_changed),
                            self);
  g_signal_connect_swapped (priv->uri, "changed",
                            G_CALLBACK (hyscan_gtk_profile_hw_device_editor_uri_changed),
                            self);
  g_signal_connect_swapped (priv->driver, "changed",
                            G_CALLBACK (hyscan_gtk_profile_hw_device_editor_driver_changed),
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

  hyscan_gtk_profile_hw_device_editor_update (self);
}

static void
hyscan_gtk_profile_hw_device_editor_name_changed (HyScanGtkProfileHWDeviceEditor *self)
{
  const gchar *name, *text;

  name = hyscan_profile_hw_device_get_name (self->priv->device);
  text = gtk_entry_get_text (self->priv->name);

  if (0 == g_strcmp0 (name, text))
    return;

  hyscan_profile_hw_device_set_name (self->priv->device, text);

  g_signal_emit (self, hyscan_gtk_profile_hw_device_editor_signals[SIGNAL_CHANGED], 0);

}

static void
hyscan_gtk_profile_hw_device_editor_uri_changed (HyScanGtkProfileHWDeviceEditor *self)
{
  const gchar *uri, *text;

  uri = hyscan_profile_hw_device_get_uri (self->priv->device);
  text = gtk_entry_get_text (self->priv->uri);

  if (0 == g_strcmp0 (uri, text))
    return;

  hyscan_profile_hw_device_set_uri (self->priv->device, text);
  hyscan_gtk_profile_hw_device_editor_update (self);
}

static void
hyscan_gtk_profile_hw_device_editor_driver_changed (HyScanGtkProfileHWDeviceEditor *self)
{
  const gchar *driver, *text;

  driver = hyscan_profile_hw_device_get_driver (self->priv->device);
  text = gtk_combo_box_get_active_id (GTK_COMBO_BOX (self->priv->driver));

  if (0 == g_strcmp0 (driver, text))
    return;

  hyscan_profile_hw_device_set_driver (self->priv->device, text);
  hyscan_gtk_profile_hw_device_editor_update (self);
}

static void
hyscan_gtk_profile_hw_device_editor_update (HyScanGtkProfileHWDeviceEditor *self)
{
  HyScanGtkProfileHWDeviceEditorPrivate *priv = self->priv;
  gboolean update_state;

  update_state = hyscan_profile_hw_device_update (priv->device);
  if (update_state)
    hyscan_gtk_param_set_param (priv->param, HYSCAN_PARAM (priv->device), "/", FALSE);
  else
    hyscan_gtk_param_set_param (priv->param, NULL, "/", FALSE);

  g_signal_emit (self, hyscan_gtk_profile_hw_device_editor_signals[SIGNAL_CHANGED], 0);
}

static void
hyscan_gtk_profile_hw_device_editor_object_finalize (GObject *object)
{
  HyScanGtkProfileHWDeviceEditor *self = HYSCAN_GTK_PROFILE_HW_DEVICE_EDITOR (object);
  HyScanGtkProfileHWDeviceEditorPrivate *priv = self->priv;

  g_clear_object (&priv->device);

  G_OBJECT_CLASS (hyscan_gtk_profile_hw_device_editor_parent_class)->finalize (object);
}

GtkWidget *
hyscan_gtk_profile_hw_device_editor_new (HyScanProfileHWDevice *device)
{
  return g_object_new (HYSCAN_TYPE_GTK_PROFILE_HW_DEVICE_EDITOR,
                       "device", device,
                       NULL);
}

HyScanProfileHWDevice *
hyscan_gtk_profile_hw_device_editor_get_device (HyScanGtkProfileHWDeviceEditor *self)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_PROFILE_HW_DEVICE_EDITOR (self), NULL);

  return g_object_ref (self->priv->device);
}
