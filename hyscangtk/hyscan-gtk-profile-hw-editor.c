/* hyscan-gtk-profile-hw-editor.h
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
 * SECTION: hyscan-gtk-profile-hw-editor
 * @Title HyScanGtkProfileHWEditor
 * @Short_description
 *
 */

#include "hyscan-gtk-profile-hw-editor.h"
#include <hyscan-gtk-profile-hw-device-editor.h>
#include <hyscan-cell-renderer-pixbuf.h>
#include <glib/gi18n.h>

enum
{
  RESPONSE_CANCEL,
  RESPONSE_APPLY,
  RESPONSE_DELETE
};

enum
{
  ACTION_INVALID = -1,
  ACTION_EDIT = 0,
  ACTION_NEW
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
  PROP_PROFILE,
  PROP_DRIVERS
};

struct _HyScanGtkProfileHWEditorPrivate
{
  HyScanProfileHW  *profile;
  gchar           **drivers;

  GtkEntry         *name;
  GtkTreeView      *tree;
  GtkListStore     *store;
};

static void           hyscan_gtk_profile_hw_editor_set_property       (GObject                  *object,
                                                                       guint                     prop_id,
                                                                       const GValue             *value,
                                                                       GParamSpec               *pspec);
static void           hyscan_gtk_profile_hw_editor_object_constructed (GObject                  *object);
static void           hyscan_gtk_profile_hw_editor_object_finalize    (GObject                  *object);
static GtkTreeView *  hyscan_gtk_profile_hw_editor_make_tree          (HyScanGtkProfileHWEditor *self);
static GtkListStore * hyscan_gtk_profile_hw_editor_make_model         (HyScanGtkProfileHWEditor *self);
static gint           hyscan_gtk_profile_hw_editor_compare_func       (GtkTreeModel             *model,
                                                                       GtkTreeIter              *a,
                                                                       GtkTreeIter              *b,
                                                                       gpointer                  user_data);
static gboolean       hyscan_gtk_profile_hw_editor_edit               (HyScanGtkProfileHWEditor *self,
                                                                       HyScanProfileHWDevice    *device);
static void           hyscan_gtk_profile_hw_editor_clicked            (GtkCellRenderer          *cell_renderer,
                                                                       const gchar              *path,
                                                                       gpointer                  user_data);
static void           hyscan_gtk_profile_hw_editor_update_tree        (HyScanGtkProfileHWEditor *self);
static void           hyscan_gtk_profile_hw_editor_name_changed       (HyScanGtkProfileHWEditor *self);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkProfileHWEditor, hyscan_gtk_profile_hw_editor, GTK_TYPE_GRID);

static void
hyscan_gtk_profile_hw_editor_class_init (HyScanGtkProfileHWEditorClass *klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  oclass->set_property = hyscan_gtk_profile_hw_editor_set_property;
  oclass->constructed = hyscan_gtk_profile_hw_editor_object_constructed;
  oclass->finalize = hyscan_gtk_profile_hw_editor_object_finalize;

  g_object_class_install_property (oclass, PROP_PROFILE,
    g_param_spec_object ("profile", "Profile", "HyScanProfile",
                         HYSCAN_TYPE_PROFILE_HW,
                         G_PARAM_CONSTRUCT | G_PARAM_WRITABLE));
  g_object_class_install_property (oclass, PROP_DRIVERS,
    g_param_spec_pointer ("drivers", "Drivers", "Drivers search paths",
                          G_PARAM_CONSTRUCT | G_PARAM_WRITABLE));
}

static void
hyscan_gtk_profile_hw_editor_init (HyScanGtkProfileHWEditor *self)
{
  HyScanGtkProfileHWEditorPrivate *priv;
  GtkWidget *name_label;
  GtkGrid *grid = GTK_GRID (self);

  priv = hyscan_gtk_profile_hw_editor_get_instance_private (self);
  self->priv = priv;

  /* Создаю виджеты. */
  name_label = gtk_label_new ("Name");
  priv->name = GTK_ENTRY (gtk_entry_new ());

  priv->store = hyscan_gtk_profile_hw_editor_make_model (self);
  priv->tree = hyscan_gtk_profile_hw_editor_make_tree (self);
  gtk_tree_view_set_model (priv->tree, GTK_TREE_MODEL (priv->store));

                                               /* L, T, W, H. */
  gtk_grid_attach (grid, GTK_WIDGET (name_label), 0, 0, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (priv->name), 1, 0, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (priv->tree), 0, 1, 2, 1);

  gtk_widget_set_hexpand (GTK_WIDGET (priv->name), TRUE);
  gtk_widget_set_hexpand (GTK_WIDGET (priv->tree), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET (priv->tree), TRUE);

  gtk_grid_set_row_spacing (grid, 6);
  gtk_grid_set_column_spacing (grid, 6);
  gtk_widget_set_margin_start (GTK_WIDGET (self), 6);
  gtk_widget_set_margin_end (GTK_WIDGET (self), 6);
  gtk_widget_set_margin_top (GTK_WIDGET (self), 6);
  gtk_widget_set_margin_bottom (GTK_WIDGET (self), 6);
  gtk_widget_set_hexpand (GTK_WIDGET (self), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET (self), TRUE);

  g_signal_connect_swapped (priv->name, "changed",
                            G_CALLBACK (hyscan_gtk_profile_hw_editor_name_changed),
                            self);
}

static void
hyscan_gtk_profile_hw_editor_set_property (GObject      *object,
                                           guint         prop_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
  HyScanGtkProfileHWEditor *self = HYSCAN_GTK_PROFILE_HW_EDITOR (object);
  HyScanGtkProfileHWEditorPrivate *priv = self->priv;

  switch (prop_id)
    {
    case PROP_PROFILE:
      priv->profile = g_value_dup_object (value);
      break;

    case PROP_DRIVERS:
      priv->drivers = g_strdupv (g_value_get_pointer (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
hyscan_gtk_profile_hw_editor_object_constructed (GObject *object)
{
  HyScanGtkProfileHWEditor *self = HYSCAN_GTK_PROFILE_HW_EDITOR (object);

  G_OBJECT_CLASS (hyscan_gtk_profile_hw_editor_parent_class)->constructed (object);

  gtk_entry_set_text (self->priv->name, hyscan_profile_get_name (HYSCAN_PROFILE (self->priv->profile)));

  hyscan_gtk_profile_hw_editor_update_tree (self);
}

static void
hyscan_gtk_profile_hw_editor_object_finalize (GObject *object)
{
  HyScanGtkProfileHWEditor *self = HYSCAN_GTK_PROFILE_HW_EDITOR (object);
  HyScanGtkProfileHWEditorPrivate *priv = self->priv;

  g_clear_object (&priv->profile);
  g_clear_pointer (&priv->drivers, g_strfreev);

  G_OBJECT_CLASS (hyscan_gtk_profile_hw_editor_parent_class)->finalize (object);
}

/* Функция создает виджет дерева. */
static GtkTreeView *
hyscan_gtk_profile_hw_editor_make_tree (HyScanGtkProfileHWEditor *self)
{
  GtkTreeView * view;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  view = GTK_TREE_VIEW (gtk_tree_view_new ());

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

  /* Колонка с иконкой правки. */
  renderer = hyscan_cell_renderer_pixbuf_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL, renderer,
                                                     "icon-name",
                                                     ACTION_ICON_COL,
                                                     NULL);
  g_signal_connect (renderer, "clicked",
                    G_CALLBACK (hyscan_gtk_profile_hw_editor_clicked), self);
  gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_append_column (view, column);

  return view;
}

/* Функция создает модель по умолчанию. */
static GtkListStore *
hyscan_gtk_profile_hw_editor_make_model (HyScanGtkProfileHWEditor *self)
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
                                   hyscan_gtk_profile_hw_editor_compare_func,
                                   NULL, NULL);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (store), NAME_COL, GTK_SORT_DESCENDING);

  return store;
}

/* Функция сортировки. Она довольно хитрая, чтобы иметь возможность
 * фиксировать отдельные кнопки вверху или внизу.*/
static gint
hyscan_gtk_profile_hw_editor_compare_func (GtkTreeModel *model,
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

static void
hyscan_gtk_profile_hw_editor_sanity (GtkDialog *dialog,
                                     gboolean   sane)
{
  gtk_dialog_set_response_sensitive (dialog, RESPONSE_APPLY, sane);
}

static void
hyscan_gtk_profile_hw_editor_restyle (GtkDialog   *dialog,
                                      gint         response_id,
                                      const gchar *style_class)
{
  GtkWidget *widget = gtk_dialog_get_widget_for_response (dialog, response_id);
  GtkStyleContext *context = gtk_widget_get_style_context (widget);

  gtk_style_context_add_class (context, style_class);
}

static gboolean
hyscan_gtk_profile_hw_editor_edit (HyScanGtkProfileHWEditor *self,
                                   HyScanProfileHWDevice    *device)
{
  gint response;
  GtkWidget *dialog, *content, *creator;
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
  GtkWindow *window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (self)));

  dialog = gtk_dialog_new_with_buttons (_("Edit device"),
                                        window, flags,
                                        _("_Delete"), RESPONSE_DELETE,
                                        _("_Cancel"), RESPONSE_CANCEL,
                                        _("_OK"), RESPONSE_APPLY,
                                        NULL);

  hyscan_gtk_profile_hw_editor_restyle (GTK_DIALOG (dialog), RESPONSE_APPLY,
                                        GTK_STYLE_CLASS_SUGGESTED_ACTION);
  hyscan_gtk_profile_hw_editor_restyle (GTK_DIALOG (dialog), RESPONSE_DELETE,
                                        GTK_STYLE_CLASS_DESTRUCTIVE_ACTION);

  creator = hyscan_gtk_profile_hw_device_editor_new (device);
  content = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gtk_container_add (GTK_CONTAINER (content), creator);
  gtk_widget_show_all (dialog);

  g_signal_connect_swapped (creator, "sane",
                            G_CALLBACK (hyscan_gtk_profile_hw_editor_sanity), dialog);

  response = gtk_dialog_run (GTK_DIALOG (dialog));

  gtk_widget_destroy (dialog);
  return response;
}

static void
hyscan_gtk_profile_hw_editor_clicked (GtkCellRenderer *cell_renderer,
                                      const gchar     *path,
                                      gpointer         user_data)
{
  HyScanGtkProfileHWEditor *self = user_data;
  HyScanGtkProfileHWEditorPrivate *priv = self->priv;
  GtkTreeModel *model = GTK_TREE_MODEL (priv->store);
  HyScanProfileHWDevice *device;
  GtkTreeIter iter;
  gint action, response;

  if (!gtk_tree_model_get_iter_from_string (model, &iter, path))
    return;

  gtk_tree_model_get (model, &iter,
                      DEVICE_COL, &device,
                      ACTION_TYPE_COL, &action, -1);

  /* Если это кнопка добавления нового устройства... */
  if (action == ACTION_NEW)
    {
      device = hyscan_profile_hw_device_new ();
      hyscan_profile_hw_device_set_paths (device, priv->drivers);
      hyscan_profile_hw_add (priv->profile, device);
    }

  /* Правим устройство. */
  response = hyscan_gtk_profile_hw_editor_edit (self, device);

  /* Профиль следует удалить, если нажата кнопка удалить ЛИБО
   * нажата кнопка отмена и это был свежесозданный профиль. */
  if (RESPONSE_DELETE == response ||
      (RESPONSE_CANCEL == response && ACTION_NEW == action))
    {
      const gchar *id = hyscan_profile_hw_device_get_group (device);
      hyscan_profile_hw_remove (priv->profile, id);
    }

  g_clear_object (&device);

  hyscan_gtk_profile_hw_editor_update_tree (self);
}

static void
hyscan_gtk_profile_hw_editor_update_tree (HyScanGtkProfileHWEditor *self)
{
  GtkListStore *store = GTK_LIST_STORE (self->priv->store);
  GList *devices, *link;
  GtkTreeIter ls_iter;
  gint device_counter = 0;

  gtk_list_store_clear (store);

  /* Специальная строка: "новый профиль". */
  gtk_list_store_append (store, &ls_iter);
  gtk_list_store_set (store, &ls_iter,
                      DEVICE_COL, NULL,
                      NAME_COL, g_strdup ("New..."),
                      STYLE_COL, PANGO_STYLE_ITALIC,
                      ACTION_TYPE_COL, ACTION_NEW,
                      ACTION_ICON_COL, "list-add-symbolic",
                      -1);

  devices = hyscan_profile_hw_list (self->priv->profile);

  for (link = devices; link != NULL; link = link->next)
    {
      HyScanProfileHWDevice *device = (HyScanProfileHWDevice *)link->data;
      gboolean check = hyscan_profile_hw_device_check (device);
      gint style = PANGO_STYLE_NORMAL;
      const gchar *text, *icon;
      gchar *title;

      /* Определяем, что будет в поле название. */
      text = hyscan_profile_hw_device_get_name (device);
      if (text == NULL)
        {
          style = PANGO_STYLE_ITALIC;
          text = hyscan_profile_hw_device_get_uri (device);
        }

      title = text != NULL ? g_strdup (text) : g_strdup_printf ("Device %i", ++device_counter);
      icon = check ? "network-transmit-receive-symbolic" : "network-error-symbolic";

      /* Добавляем в деревце. */
      gtk_list_store_append (store, &ls_iter);
      gtk_list_store_set (store, &ls_iter,
                          DEVICE_COL, g_object_ref (device),
                          NAME_COL, title,
                          STYLE_COL, style,
                          STATUS_ICON_COL, g_strdup (icon),
                          ACTION_TYPE_COL, ACTION_EDIT,
                          ACTION_ICON_COL, g_strdup ("emblem-system-symbolic"),
                          -1);
    }
}

static void
hyscan_gtk_profile_hw_editor_name_changed (HyScanGtkProfileHWEditor *self)
{
  HyScanGtkProfileHWEditorPrivate *priv = self->priv;
  const gchar *name;

  name = gtk_entry_get_text (priv->name);
  hyscan_profile_set_name (HYSCAN_PROFILE (priv->profile), name);
}

GtkWidget *
hyscan_gtk_profile_hw_editor_new (HyScanProfile *profile,
                                  gchar        **drivers)
{
  return g_object_new (HYSCAN_TYPE_GTK_PROFILE_HW_EDITOR,
                       "profile", profile,
                       "drivers", drivers,
                       NULL);
}
