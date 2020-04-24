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
#include <hyscan-gtk-profile-editor-hw-device.h>
#include <hyscan-cell-renderer-pixbuf.h>
#include <glib/gi18n-lib.h>

#define WIDGET_RESOURCE_UI "/org/hyscan/gtk/hyscan-gtk-profile-editor-hw.ui"

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
  HyScanProfileHW  *profile;     /* Редактируемый профиль. */
  gchar           **drivers;     /* Папки с драйверами. */

  GtkEntry         *name;        /* Поле ввода названия профиля. */
  GtkTreeView      *device_list; /* Список устройств. */
  GtkListStore     *store;       /* Модель для списка устр-в. */
  GtkStack         *stack;       /* Стек с редакторами уст-в. */

  GHashTable       *known;       /* Имеющиеся устройства {gchar *}. */
};

static void           hyscan_gtk_profile_editor_hw_set_property       (GObject                  *object,
                                                                       guint                     prop_id,
                                                                       const GValue             *value,
                                                                       GParamSpec               *pspec);
static void           hyscan_gtk_profile_editor_hw_object_constructed (GObject                  *object);
static void           hyscan_gtk_profile_editor_hw_object_finalize    (GObject                  *object);
static void           hyscan_gtk_profile_editor_hw_make_tree          (HyScanGtkProfileEditorHW *self);
static GtkListStore * hyscan_gtk_profile_editor_hw_make_model         (HyScanGtkProfileEditorHW *self);
static gint           hyscan_gtk_profile_editor_hw_compare_func       (GtkTreeModel             *model,
                                                                       GtkTreeIter              *a,
                                                                       GtkTreeIter              *b,
                                                                       gpointer                  user_data);
static void           hyscan_gtk_profile_editor_hw_ensure_page        (HyScanGtkProfileEditorHW *self,
                                                                       HyScanProfileHWDevice    *device);
static void           hyscan_gtk_profile_editor_hw_clicked            (GtkCellRenderer          *cell_renderer,
                                                                       const gchar              *path,
                                                                       GdkEvent                 *event,
                                                                       gpointer                  user_data);
static void           hyscan_gtk_profile_editor_hw_device_changed     (HyScanGtkProfileEditorHWDevice *device,
                                                                       HyScanGtkProfileEditorHW       *self);
static void           hyscan_gtk_profile_editor_hw_update_tree        (HyScanGtkProfileEditorHW *self);
static void           hyscan_gtk_profile_editor_hw_name_changed       (HyScanGtkProfileEditorHW *self);
static void           hyscan_gtk_profile_editor_hw_selected           (GtkTreeSelection         *selection,
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
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHW, name);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHW, stack);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorHW, device_list);

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

  priv->known = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  priv->store = hyscan_gtk_profile_editor_hw_make_model (self);
  hyscan_gtk_profile_editor_hw_make_tree (self);
  gtk_tree_view_set_model (priv->device_list, GTK_TREE_MODEL (priv->store));

  g_signal_connect_swapped (priv->name, "changed",
                            G_CALLBACK (hyscan_gtk_profile_editor_hw_name_changed),
                            self);
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
  HyScanProfile *profile;
  const gchar *name;

  G_OBJECT_CLASS (hyscan_gtk_profile_editor_hw_parent_class)->constructed (object);

  profile = hyscan_gtk_profile_editor_get_profile (HYSCAN_GTK_PROFILE_EDITOR (self));
  self->priv->profile = HYSCAN_PROFILE_HW (profile);

  name = hyscan_profile_get_name (profile);
  if (name != NULL)
    gtk_entry_set_text (self->priv->name, name);

  hyscan_gtk_profile_editor_hw_update_tree (self);
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

/* Функция создает виджет дерева. */
static void
hyscan_gtk_profile_editor_hw_make_tree (HyScanGtkProfileEditorHW *self)
{
  GtkTreeView * view = GTK_TREE_VIEW (self->priv->device_list);
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeSelection *selection;

  /* Колонка с названием устройства. */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Device name"), renderer,
                                                     "text",
                                                     NAME_COL,
                                                     "style",
                                                     STYLE_COL,
                                                     NULL);
  gtk_tree_view_column_set_sort_column_id (column, NAME_COL);
  gtk_tree_view_column_set_expand (column, TRUE);
  gtk_tree_view_append_column (view, column);

  /* Колонка с иконкой статуса. */
  renderer = gtk_cell_renderer_pixbuf_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL, renderer,
                                                     "icon-name",
                                                     STATUS_ICON_COL,
                                                     NULL);
  gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_append_column (view, column);

  /* Колонка с иконкой создания/удаления. */
  renderer = hyscan_cell_renderer_pixbuf_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL, renderer,
                                                     "icon-name",
                                                     ACTION_ICON_COL,
                                                     NULL);
  g_signal_connect (renderer, "clicked",
                    G_CALLBACK (hyscan_gtk_profile_editor_hw_clicked), self);
  gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_append_column (view, column);

  selection = gtk_tree_view_get_selection (view);
  g_signal_connect (selection, "changed",
                    G_CALLBACK (hyscan_gtk_profile_editor_hw_selected),
                    self);
}

/* Функция создает модель по умолчанию. */
static GtkListStore *
hyscan_gtk_profile_editor_hw_make_model (HyScanGtkProfileEditorHW *self)
{
  GtkListStore *store = NULL;

  store = gtk_list_store_new (N_COLUMNS,
                              G_TYPE_OBJECT,  /* DEVICE_COL */
                              G_TYPE_STRING,  /* NAME_COL */
                              G_TYPE_STRING,  /* STATUS_ICON_COL */
                              G_TYPE_INT,     /* ACTION_TYPE_COL */
                              G_TYPE_STRING,  /* ACTION_ICON_COL */
                              G_TYPE_INT);    /* STYLE_COL */

  gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (store), NAME_COL,
                                   hyscan_gtk_profile_editor_hw_compare_func,
                                   NULL, NULL);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (store), NAME_COL, GTK_SORT_DESCENDING);

  return store;
}

/* Функция проверяет наличие страницы и создает её при необходимости. */
static void
hyscan_gtk_profile_editor_hw_ensure_page (HyScanGtkProfileEditorHW *self,
                                          HyScanProfileHWDevice    *device)
{
  HyScanGtkProfileEditorHWPrivate *priv = self->priv;
  GtkWidget *page;
  const gchar *id;

  id = hyscan_profile_hw_device_get_group (device);
  if (g_hash_table_contains (priv->known, id))
    return;

  page = hyscan_gtk_profile_editor_hw_device_new (device);
  g_signal_connect (page, "changed",
                    G_CALLBACK (hyscan_gtk_profile_editor_hw_device_changed),
                    self);

  gtk_stack_add_named (priv->stack, page, id);
  g_hash_table_add (priv->known, g_strdup (id));
  gtk_widget_show_all (page);
}

/* Функция удаления устройства. */
static void
hyscan_gtk_profile_editor_hw_remove (HyScanGtkProfileEditorHW *self,
                                     HyScanProfileHWDevice    *device)
{
  HyScanGtkProfileEditorHWPrivate *priv = self->priv;
  const gchar *id;

  id = hyscan_profile_hw_device_get_group (device);

  hyscan_profile_hw_remove (priv->profile, id);
  gtk_widget_destroy (gtk_stack_get_child_by_name (priv->stack, id));
  g_hash_table_remove (priv->known, id);
}

/* Обработчик нажатия кнопок удаления/добавления устройств. */
static void
hyscan_gtk_profile_editor_hw_clicked (GtkCellRenderer *cell_renderer,
                                      const gchar     *path,
                                      GdkEvent        *event,
                                      gpointer         user_data)
{
  HyScanGtkProfileEditorHW *self = user_data;
  HyScanGtkProfileEditorHWPrivate *priv = self->priv;
  GtkTreeModel *model = GTK_TREE_MODEL (priv->store);
  HyScanProfileHWDevice *device;
  GtkTreeIter iter;
  gint action;

  if (!gtk_tree_model_get_iter_from_string (model, &iter, path))
    return;

  gtk_tree_model_get (model, &iter,
                      DEVICE_COL, &device,
                      ACTION_TYPE_COL, &action, -1);

  /* Если это кнопка добавления нового устройства... */
  if (action == ACTION_ADD)
    {
      device = hyscan_profile_hw_device_new (priv->drivers);
      hyscan_profile_hw_add (priv->profile, device);
    }
  else if (action == ACTION_DELETE)
    {
      hyscan_gtk_profile_editor_hw_remove (self, device);
    }

  g_clear_object (&device);
  hyscan_gtk_profile_editor_hw_update_tree (self);
}

/* Функция обновляет строку с устройством в дереве. */
static void
hyscan_gtk_profile_editor_hw_update_device (GtkListStore          *store,
                                            GtkTreeIter           *iter,
                                            HyScanProfileHWDevice *device)
{
  gint style;
  const gchar *name, *uri, *icon;
  const gchar *title;

  name = hyscan_profile_hw_device_get_name (device);
  uri = hyscan_profile_hw_device_get_uri (device);

  if (NULL != name)
    {
      style = PANGO_STYLE_NORMAL;
      title = _(name);
    }
  else if (NULL != uri)
    {
      style = PANGO_STYLE_ITALIC;
      title = _(uri);
    }
  else
    {
      style = PANGO_STYLE_ITALIC;
      title = _("Nameless Device");
    }

  if (!hyscan_profile_hw_device_sanity (device))
    icon = "dialog-warning-symbolic";
  else if (hyscan_profile_hw_device_check (device))
    icon = "network-transmit-receive-symbolic";
  else
    icon = "network-error-symbolic";

  gtk_list_store_set (store, iter,
                      DEVICE_COL, device,
                      NAME_COL, title,
                      STYLE_COL, style,
                      STATUS_ICON_COL, icon,
                      ACTION_TYPE_COL, ACTION_DELETE,
                      ACTION_ICON_COL, "list-remove-symbolic",
                      -1);
}

/* Обработчик сигнала "changed" от редактора уст-ва. */
static void
hyscan_gtk_profile_editor_hw_device_changed (HyScanGtkProfileEditorHWDevice *page,
                                             HyScanGtkProfileEditorHW       *self)
{
  GtkTreeModel *model = GTK_TREE_MODEL (self->priv->store);
  GtkTreeIter iter;
  HyScanProfileHWDevice *device = hyscan_gtk_profile_editor_hw_device_get_device (page);
  HyScanProfileHWDevice *device_iter;

  if (!gtk_tree_model_get_iter_first (model, &iter))
    return;

  do
    {
      gtk_tree_model_get (model, &iter, DEVICE_COL, &device_iter, -1);

      if (device_iter != device)
        {
          g_clear_object (&device_iter);
          continue;
        }

      hyscan_gtk_profile_editor_hw_update_device (GTK_LIST_STORE (model),
                                                  &iter, device);
      g_clear_object (&device);
      break;
    }
  while (gtk_tree_model_iter_next (model, &iter));

  g_clear_object (&device);
  hyscan_gtk_profile_editor_check_sanity (HYSCAN_GTK_PROFILE_EDITOR (self));
}

/* Функция обновляет список устройств. */
static void
hyscan_gtk_profile_editor_hw_update_tree (HyScanGtkProfileEditorHW *self)
{
  GtkListStore *store = GTK_LIST_STORE (self->priv->store);
  GList *devices, *link;
  GtkTreeIter ls_iter;

  /* Выглядит как грязный хак, но похоже это рабочий способ избежать
   * ворнинга при удалении строки. */
  {
    GtkTreeSelection *selection = gtk_tree_view_get_selection (self->priv->device_list);
    gtk_tree_selection_set_mode  (selection, GTK_SELECTION_NONE);
    gtk_list_store_clear (store);
    gtk_tree_selection_set_mode  (selection, GTK_SELECTION_SINGLE);
  }

  devices = hyscan_profile_hw_list (self->priv->profile);

  for (link = devices; link != NULL; link = link->next)
    {
      HyScanProfileHWDevice *device = (HyScanProfileHWDevice *)link->data;

      hyscan_gtk_profile_editor_hw_ensure_page (self, device);
      gtk_list_store_append (store, &ls_iter);
      hyscan_gtk_profile_editor_hw_update_device (store, &ls_iter, device);
    }

  /* Специальная строка: "новый профиль". */
  gtk_list_store_append (store, &ls_iter);
  gtk_list_store_set (store, &ls_iter,
                      DEVICE_COL, NULL,
                      NAME_COL, _("New..."),
                      STYLE_COL, PANGO_STYLE_ITALIC,
                      STATUS_ICON_COL, "blank",
                      ACTION_TYPE_COL, ACTION_ADD,
                      ACTION_ICON_COL, "list-add-symbolic",
                      -1);

  hyscan_gtk_profile_editor_check_sanity (HYSCAN_GTK_PROFILE_EDITOR (self));
}

/* Обработчик изменения названия профиля. */
static void
hyscan_gtk_profile_editor_hw_name_changed (HyScanGtkProfileEditorHW *self)
{
  HyScanGtkProfileEditorHWPrivate *priv = self->priv;
  const gchar *name;

  name = gtk_entry_get_text (priv->name);
  hyscan_profile_set_name (HYSCAN_PROFILE (priv->profile), name);

  hyscan_gtk_profile_editor_check_sanity (HYSCAN_GTK_PROFILE_EDITOR (self));
}

/* Выбор устройства. */
static void
hyscan_gtk_profile_editor_hw_selected (GtkTreeSelection         *selection,
                                       HyScanGtkProfileEditorHW *self)
{
  HyScanProfileHWDevice *device;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gint action;

  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    return;

  gtk_tree_model_get (model, &iter, ACTION_TYPE_COL, &action, DEVICE_COL, &device, -1);
  if (action == ACTION_ADD)
    {
      gtk_tree_selection_unselect_iter (selection, &iter);
    }
  else
    {
      const gchar *id = hyscan_profile_hw_device_get_group (device);
      gtk_stack_set_visible_child_name (self->priv->stack, id);
    }

  g_clear_object (&device);
}

/* Функция сортировки. Она довольно хитрая, чтобы иметь возможность
 * фиксировать отдельные кнопки вверху или внизу.*/
static gint
hyscan_gtk_profile_editor_hw_compare_func (GtkTreeModel *model,
                                           GtkTreeIter  *a,
                                           GtkTreeIter  *b,
                                           gpointer      user_data)
{
  gchar *name_a, *name_b;
  gint action_a, action_b, result;

  gtk_tree_model_get (model, a, NAME_COL, &name_a, ACTION_TYPE_COL, &action_a, -1);
  gtk_tree_model_get (model, b, NAME_COL, &name_b, ACTION_TYPE_COL, &action_b, -1);

  /* Сортировка идет по 2 полям: тип строки (ROW_TYPE_COL) и названию.
   * Тип строки приоритетен. */
  if (action_a == action_b)
    {
      /* Деление необходимо, т.к. на выходе должна быть величина [-1, 0, 1],
       * a g_strcmp0 гарантирует возврат 0, положительного или отрицательного
       * значения. */
      result = g_strcmp0 (name_a, name_b);
      result = (result == 0) ? 0 : result / ABS (result);
    }
  else
    {
      /* Здесь я домножаю на -1, чтобы при сортировке по убыванию специальные
       * строки оставались на своих местах. */
      GtkSortType order;
      gtk_tree_sortable_get_sort_column_id (GTK_TREE_SORTABLE (model), NULL, &order);

      result = action_a < action_b ? -1 : 1;
      if (order == GTK_SORT_DESCENDING)
        result *= -1;
    }

  g_free (name_a);
  g_free (name_b);

  return result;
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
