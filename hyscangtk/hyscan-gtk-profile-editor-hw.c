/* hyscan-gtk-profile-editor-hw.c
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
 * SECTION: hyscan-gtk-profile-editor-hw
 * @Title HyScanGtkProfileEditorHW
 * @Short_description : Виджет редактирования профилей оборудования
 *
 */

#include "hyscan-gtk-profile-editor-hw.h"
#include <hyscan-driver.h>
#include <hyscan-gtk-param-tree.h>
#include <hyscan-gtk-param-list.h>
#include <glib/gi18n-lib.h>

#define WIDGET_RESOURCE_UI "/org/hyscan/gtk/hyscan-gtk-profile-editor-hw.ui"
#define HYSCAN_GTK_PROFILE_HW_DEVICE "hyscan-gtk-profile-hw-device"

enum
{
  ACTION_INVALID = -1,
  ACTION_DELETE,
  ACTION_ADD
};

enum
{
  DEVICE_COL,
  NAME_COL,
  STATUS_ICON_COL,
  ACTION_TYPE_COL,
  ACTION_ICON_COL,
  STYLE_COL,
  N_COLUMNS
};

enum
{
  PROP_0,
  PROP_DRIVERS
};

struct _HyScanGtkProfileEditorHWPrivate
{
  HyScanProfileHW       *profile;      /* Редактируемый профиль. */
  gchar                **drivers;      /* Папки с драйверами. */

  GtkEntry              *profile_name; /* Поле ввода названия профиля. */
  GtkListBox            *device_list;  /* Список устройств. */
  GtkStack              *stack;        /* Стек с редакторами уст-в. */
  GtkEntry              *add;          /* Кнопка добавления устройства. */
  GtkEntry              *remove;       /* Кнопка удаления устройства. */

  GtkPopover            *popover;      /* Всплывающее окно добавления. */
  GtkEntry              *new_name;     /* Название нового устройства. */
  GtkComboBoxText       *new_driver;   /* Драйвер нового устройства. */
  GtkEntry              *new_uri;      /* Адрес нового устройства. */
  GtkButton             *new_add;      /* Кнопка подтверждения выбора. */

  GHashTable            *known;       /* Имеющиеся устройства {gchar *: HyScanProfileHWDevice *}. */
  HyScanProfileHWDevice *selected_device;
};

static void           hyscan_gtk_profile_editor_hw_set_property       (GObject                  *object,
                                                                       guint                     prop_id,
                                                                       const GValue             *value,
                                                                       GParamSpec               *pspec);
static void           hyscan_gtk_profile_editor_hw_object_constructed (GObject                  *object);
static void           hyscan_gtk_profile_editor_hw_object_finalize    (GObject                  *object);

static GtkWidget *    hyscan_gtk_profile_editor_hw_make_row           (HyScanProfileHWDevice    *device);
static void           hyscan_gtk_profile_editor_hw_select_helper      (HyScanGtkProfileEditorHW *self,
                                                                       HyScanProfileHWDevice    *device);
static void           hyscan_gtk_profile_editor_hw_add_helper         (HyScanGtkProfileEditorHW *self,
                                                                       HyScanProfileHWDevice    *device);
static void           hyscan_gtk_profile_editor_hw_name_changed       (HyScanGtkProfileEditorHW *self);
static void           hyscan_gtk_profile_editor_hw_new_device_sanity  (HyScanGtkProfileEditorHW *self);

static void           hyscan_gtk_profile_editor_hw_remove             (HyScanGtkProfileEditorHW *self);
static void           hyscan_gtk_profile_editor_hw_add_start          (HyScanGtkProfileEditorHW *self);
static void           hyscan_gtk_profile_editor_hw_add_ok             (HyScanGtkProfileEditorHW *self);

static void           hyscan_gtk_profile_editor_hw_update_list        (HyScanGtkProfileEditorHW *self);
static void           hyscan_gtk_profile_editor_hw_selected           (GtkListBox               *box,
                                                                       GtkListBoxRow            *row,
                                                                       HyScanGtkProfileEditorHW *self);


G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkProfileEditorHW, hyscan_gtk_profile_editor_hw, HYSCAN_TYPE_GTK_PROFILE_EDITOR);

static void
hyscan_gtk_profile_editor_hw_class_init (HyScanGtkProfileEditorHWClass *klass)
{
  GtkWidgetClass *wclass = GTK_WIDGET_CLASS (klass);
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  oclass->set_property = hyscan_gtk_profile_editor_hw_set_property;
  oclass->constructed = hyscan_gtk_profile_editor_hw_object_constructed;
  oclass->finalize = hyscan_gtk_profile_editor_hw_object_finalize;

  gtk_widget_class_set_template_from_resource (wclass, WIDGET_RESOURCE_UI);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHW, profile_name);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHW, device_list);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHW, stack);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHW, add);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHW, remove);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHW, popover);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHW, new_name);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHW, new_driver);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHW, new_uri);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHW, new_add);

  gtk_widget_class_bind_template_callback_full (wclass,
    "row_selected", (GCallback)hyscan_gtk_profile_editor_hw_selected);
  gtk_widget_class_bind_template_callback_full (wclass,
    "profile_name_changed", (GCallback)hyscan_gtk_profile_editor_hw_name_changed);
  gtk_widget_class_bind_template_callback_full (wclass,
    "add_device_clicked", (GCallback)hyscan_gtk_profile_editor_hw_add_start);
  gtk_widget_class_bind_template_callback_full (wclass,
    "remove_device_clicked", (GCallback)hyscan_gtk_profile_editor_hw_remove);

  gtk_widget_class_bind_template_callback_full (wclass,
    "new_device_name_changed", (GCallback)hyscan_gtk_profile_editor_hw_new_device_sanity);
  gtk_widget_class_bind_template_callback_full (wclass,
    "new_device_driver_changed", (GCallback)hyscan_gtk_profile_editor_hw_new_device_sanity);
  gtk_widget_class_bind_template_callback_full (wclass,
    "new_device_uri_changed", (GCallback)hyscan_gtk_profile_editor_hw_new_device_sanity);
  gtk_widget_class_bind_template_callback_full (wclass,
    "new_device_add_clicked", (GCallback)hyscan_gtk_profile_editor_hw_add_ok);


  g_object_class_install_property (oclass, PROP_DRIVERS,
    g_param_spec_pointer ("drivers", "Drivers", "Drivers search paths",
                          G_PARAM_CONSTRUCT | G_PARAM_WRITABLE));
}

static void
hyscan_gtk_profile_editor_hw_init (HyScanGtkProfileEditorHW *self)
{
  HyScanGtkProfileEditorHWPrivate *priv;

  self->priv = hyscan_gtk_profile_editor_hw_get_instance_private (self);
  gtk_widget_init_template (GTK_WIDGET (self));
  priv = self->priv;

  priv->known = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
}

static void
hyscan_gtk_profile_editor_hw_set_property (GObject      *object,
                                           guint         prop_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
  HyScanGtkProfileEditorHW *self = HYSCAN_GTK_PROFILE_EDITOR_HW (object);
  HyScanGtkProfileEditorHWPrivate *priv = self->priv;

  switch (prop_id)
    {
    case PROP_DRIVERS:
      priv->drivers = g_strdupv (g_value_get_pointer (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
hyscan_gtk_profile_editor_hw_object_constructed (GObject *object)
{
  HyScanGtkProfileEditorHW *self = HYSCAN_GTK_PROFILE_EDITOR_HW (object);
  HyScanGtkProfileEditorHWPrivate *priv = self->priv;
  HyScanProfile *profile;
  const gchar *name;

  G_OBJECT_CLASS (hyscan_gtk_profile_editor_hw_parent_class)->constructed (object);

  profile = hyscan_gtk_profile_editor_get_profile (HYSCAN_GTK_PROFILE_EDITOR (self));
  priv->profile = HYSCAN_PROFILE_HW (profile);

  name = hyscan_profile_get_name (profile);
  if (name != NULL)
    gtk_entry_set_text (priv->profile_name, name);

  /* Заполняем комбобокс с драйверами. */
  {
    GArray *driver_list = g_array_new (TRUE, FALSE, sizeof(gchar*));
    gchar **path, **drivers, **driv;

    /* Из каждого пути получаем список доступных драйверов. */
    for (path = priv->drivers; path != NULL && *path != NULL; ++path)
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
      gtk_combo_box_text_insert (priv->new_driver, -1, *driv, *driv);
  }

  /* */
  {
    GList *devices, *link;

    devices = hyscan_profile_hw_list (priv->profile);
    for (link = devices; link != NULL; link = link->next)
      {
        HyScanProfileHWDevice *device = (HyScanProfileHWDevice*)link->data;
        hyscan_gtk_profile_editor_hw_add_helper (self, device);
      }

    g_list_free (devices);
  }

  hyscan_gtk_profile_editor_hw_select_helper (self, NULL);
  hyscan_gtk_profile_editor_hw_update_list (self);
}

static void
hyscan_gtk_profile_editor_hw_object_finalize (GObject *object)
{
  HyScanGtkProfileEditorHW *self = HYSCAN_GTK_PROFILE_EDITOR_HW (object);
  HyScanGtkProfileEditorHWPrivate *priv = self->priv;

  g_clear_object (&priv->profile);
  g_clear_pointer (&priv->drivers, g_strfreev);
  g_clear_pointer (&priv->known, g_hash_table_unref);

  G_OBJECT_CLASS (hyscan_gtk_profile_editor_hw_parent_class)->finalize (object);
}

/* Функция создает виджет устройства. */
static GtkWidget *
hyscan_gtk_profile_editor_hw_make_row (HyScanProfileHWDevice *device)
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
hyscan_gtk_profile_editor_hw_select_helper (HyScanGtkProfileEditorHW *self,
                                            HyScanProfileHWDevice    *device)
{
  HyScanGtkProfileEditorHWPrivate *priv = self->priv;

  g_clear_object (&priv->selected_device);

  gtk_widget_set_sensitive (GTK_WIDGET (priv->remove), device != NULL);

  if (device == NULL)
    return;

  priv->selected_device = g_object_ref (device);
  gtk_stack_set_visible_child_name (priv->stack, hyscan_profile_hw_device_get_group (device));
}

static void
hyscan_gtk_profile_editor_hw_add_helper (HyScanGtkProfileEditorHW *self,
                                         HyScanProfileHWDevice    *device)
{
  HyScanGtkProfileEditorHWPrivate *priv = self->priv;
  GtkWidget *param;
  const gchar *id;

  id = hyscan_profile_hw_device_get_group (device);
  param = hyscan_gtk_param_list_new_full (HYSCAN_PARAM (device), NULL, FALSE);
  gtk_widget_show_all (param);

  g_hash_table_insert (priv->known, g_strdup (id), g_object_ref (device));
  gtk_stack_add_named (priv->stack, param, id);
}

/* Обработчик изменения названия профиля. */
static void
hyscan_gtk_profile_editor_hw_name_changed (HyScanGtkProfileEditorHW *self)
{
  HyScanGtkProfileEditorHWPrivate *priv = self->priv;
  const gchar *name;

  name = gtk_entry_get_text (priv->profile_name);
  hyscan_profile_set_name (HYSCAN_PROFILE (priv->profile), name);

  hyscan_gtk_profile_editor_check_sanity (HYSCAN_GTK_PROFILE_EDITOR (self));
}


/* Функция настривает активность кнопки добавления устройства. */
static void
hyscan_gtk_profile_editor_hw_new_device_sanity (HyScanGtkProfileEditorHW *self)
{
  const gchar *name, *uri, *driver;
  gboolean sane = TRUE;

  name = gtk_entry_get_text (self->priv->new_name);
  uri = gtk_entry_get_text (self->priv->new_uri);
  driver = gtk_combo_box_get_active_id (GTK_COMBO_BOX (self->priv->new_driver));

  if (name == NULL || name[0] == '\0' ||
      uri == NULL || uri[0] == '\0' ||
      driver == NULL || driver[0] == '\0')
    {
      sane = FALSE;
    }

  gtk_widget_set_sensitive (GTK_WIDGET (self->priv->new_add), sane);
}

/* Функция удаления устройства. */
static void
hyscan_gtk_profile_editor_hw_remove (HyScanGtkProfileEditorHW *self)
{
  HyScanGtkProfileEditorHWPrivate *priv = self->priv;
  const gchar *id;

  id = hyscan_profile_hw_device_get_group (priv->selected_device);

  hyscan_profile_hw_remove (priv->profile, id);
  gtk_widget_destroy (gtk_stack_get_child_by_name (priv->stack, id));
  g_hash_table_remove (priv->known, id);

  hyscan_gtk_profile_editor_hw_select_helper (self, NULL);
  hyscan_gtk_profile_editor_hw_update_list (self);
}

/* Показ поповера добавления устройства. */
static void
hyscan_gtk_profile_editor_hw_add_start (HyScanGtkProfileEditorHW *self)
{
  HyScanGtkProfileEditorHWPrivate *priv = self->priv;

  hyscan_gtk_profile_editor_hw_new_device_sanity (self);

  gtk_popover_popup (priv->popover);
}

/* Обработчик добавления устройства. */
static void
hyscan_gtk_profile_editor_hw_add_ok (HyScanGtkProfileEditorHW *self)
{
  HyScanGtkProfileEditorHWPrivate *priv = self->priv;
  HyScanProfileHWDevice *device;
  const gchar *name, *uri, *driver;

  name = gtk_entry_get_text (priv->new_name);
  uri = gtk_entry_get_text (priv->new_uri);
  driver = gtk_combo_box_get_active_id (GTK_COMBO_BOX (priv->new_driver));

  /* Создаем и наполняем профиль. */
  device = hyscan_profile_hw_device_new (priv->drivers);
  hyscan_profile_hw_device_set_name (device, name);
  hyscan_profile_hw_device_set_uri (device, uri);
  hyscan_profile_hw_device_set_driver (device, driver);
  hyscan_profile_hw_device_update (device);

  /* Добавляем в профиль.*/
  hyscan_profile_hw_add (priv->profile, device);

  /* Добавляем во внутренние структуры.*/
  hyscan_gtk_profile_editor_hw_add_helper (self, device);
  hyscan_gtk_profile_editor_hw_update_list (self);

  /* Очищаем поля. */
  gtk_entry_set_text (priv->new_name, "");
  gtk_entry_set_text (priv->new_uri, "");
  gtk_combo_box_set_active_id (GTK_COMBO_BOX (priv->new_driver), NULL);
  hyscan_gtk_profile_editor_hw_new_device_sanity (self);

  /* Прячем поповер. */
  gtk_popover_popdown (priv->popover);
}

/* Функция обновляет список. */
static void
hyscan_gtk_profile_editor_hw_update_list (HyScanGtkProfileEditorHW *self)
{
  HyScanGtkProfileEditorHWPrivate *priv = self->priv;
  GHashTableIter iter;
  gpointer v;
  GtkWidget *row;

  gtk_container_foreach (GTK_CONTAINER (priv->device_list),
                         (GtkCallback)gtk_widget_destroy, NULL);

  g_hash_table_iter_init (&iter, priv->known);
  while (g_hash_table_iter_next (&iter, NULL, &v))
    {
      row = hyscan_gtk_profile_editor_hw_make_row (HYSCAN_PROFILE_HW_DEVICE (v));
      gtk_list_box_insert (GTK_LIST_BOX (priv->device_list), row, -1);
    }

  hyscan_gtk_profile_editor_check_sanity (HYSCAN_GTK_PROFILE_EDITOR (self));
}

/* Выбор устройства. */
static void
hyscan_gtk_profile_editor_hw_selected (GtkListBox               *box,
                                       GtkListBoxRow            *row,
                                       HyScanGtkProfileEditorHW *self)
{
  HyScanProfileHWDevice *device = NULL;

  if (row != NULL)
    device = g_object_get_data (G_OBJECT (row), HYSCAN_GTK_PROFILE_HW_DEVICE);

  hyscan_gtk_profile_editor_hw_select_helper (self, device);
}

/*
 * hyscan_gtk_profile_editor_hw_new:
 * @profile: редактируемый профиль
 * @drivers: список папок с драйверами устройств
 *
 * Returns: (transfer full) виджет профилей БД.
 */
GtkWidget *
hyscan_gtk_profile_editor_hw_new (HyScanProfile *profile,
                                  gchar        **drivers)
{
  return g_object_new (HYSCAN_TYPE_GTK_PROFILE_EDITOR_HW,
                       "profile", profile,
                       "drivers", drivers,
                       NULL);
}
