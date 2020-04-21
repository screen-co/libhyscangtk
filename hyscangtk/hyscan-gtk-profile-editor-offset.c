/* hyscan-gtk-profile-editor-offset.c
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
 * SECTION: hyscan-gtk-profile-editor-offset
 * @Title HyScanGtkProfileEditorOffset
 * @Short_description: Виджет редактирования профилей местоположений антенн
 *
 */

#include "hyscan-gtk-profile-editor-offset.h"
#include <hyscan-cell-renderer-pixbuf.h>
#include <glib/gi18n-lib.h>

#define PAGE_RESOURCE_UI "/org/hyscan/gtk/hyscan-gtk-profile-editor-offset-page.ui"
#define WIDGET_RESOURCE_UI "/org/hyscan/gtk/hyscan-gtk-profile-editor-offset.ui"
#define SOURCE_LIST_USER_DATA "hyscan-source-list"
#define SOURCE_LIST_DELIMITER ","

#define N_TYPES 16

typedef struct
{
  gchar            *title;
  HyScanSourceType  types[N_TYPES];
} HyScanGtkProfileEditorOffsetMenuItem;

typedef struct
{
  GtkAdjustment *forward;
  GtkAdjustment *pitch;
  GtkAdjustment *roll;
  GtkAdjustment *starboard;
  GtkAdjustment *vertical;
  GtkAdjustment *yaw;
} HyScanGtkProfileEditorOffsetPage;

static HyScanGtkProfileEditorOffsetMenuItem sonar_presets[] =
{
  {N_("Sidescan sonar"),
    {HYSCAN_SOURCE_SIDE_SCAN_STARBOARD,
     HYSCAN_SOURCE_SIDE_SCAN_PORT,
     HYSCAN_SOURCE_INVALID}
  },
  {N_("Sidescan with Echosounder"),
    {HYSCAN_SOURCE_SIDE_SCAN_STARBOARD,
     HYSCAN_SOURCE_SIDE_SCAN_PORT,
     HYSCAN_SOURCE_ECHOSOUNDER,
     HYSCAN_SOURCE_INVALID}
  },
  {N_("High Frequency Sidescan sonar"),
    {HYSCAN_SOURCE_SIDE_SCAN_STARBOARD_HI,
     HYSCAN_SOURCE_SIDE_SCAN_PORT_HI,
     HYSCAN_SOURCE_INVALID}
  },
};

enum
{
  ID_COL,
  NAME_COL,
  ICON_NAME_COL,
  STYLE_COL,
  ACTION_TYPE_COL,
  N_COLUMNS
};

enum
{
  ACTION_INVALID = -1,
  ACTION_DELETE,
  ACTION_ADD
};

struct _HyScanGtkProfileEditorOffsetPrivate
{
  HyScanProfileOffset *profile;

  GtkEntry            *name;
  GtkStack            *stack;
  GtkTreeView         *device_list;

  GtkListStore        *store;

  GHashTable          *known; /* Список устройств {gchar* name/id : HyScanGtkProfileEditorOffsetPage}*/

  GtkMenu             *add_menu;
};

static void    hyscan_gtk_profile_editor_offset_object_constructed (GObject                      *object);
static void    hyscan_gtk_profile_editor_offset_object_finalize    (GObject                      *object);

static GtkMenu * hyscan_gtk_profile_editor_offset_make_menu        (HyScanGtkProfileEditorOffset *self);
static GtkListStore * hyscan_gtk_profile_editor_offset_make_model  (HyScanGtkProfileEditorOffset *self);
static void    hyscan_gtk_profile_editor_offset_make_tree          (HyScanGtkProfileEditorOffset *self);

static void    hyscan_gtk_profile_editor_offset_clicked            (HyScanCellRendererPixbuf     *rend,
                                                                    const gchar                  *path,
                                                                    GdkEvent                     *event,
                                                                    HyScanGtkProfileEditorOffset *self);
static void    hyscan_gtk_profile_editor_offset_ensure_page        (HyScanGtkProfileEditorOffset *self,
                                                                    const gchar                  *id,
                                                                    HyScanAntennaOffset          *offt);
static void    hyscan_gtk_profile_editor_offset_add_sonars         (GtkMenuItem                  *item,
                                                                    HyScanGtkProfileEditorOffset *self);
static void    hyscan_gtk_profile_editor_offset_add_sensor         (GtkMenuItem                  *item,
                                                                    HyScanGtkProfileEditorOffset *self);
static void    hyscan_gtk_profile_editor_offset_remove             (HyScanGtkProfileEditorOffset *self,
                                                                    const gchar                  *id);
static void    hyscan_gtk_profile_editor_offset_update             (HyScanGtkProfileEditorOffset *self);
static void    hyscan_gtk_profile_editor_offset_changed            (GtkSpinButton                *button,
                                                                    HyScanGtkProfileEditorOffset *self);
static void    hyscan_gtk_profile_editor_offset_selected           (GtkTreeSelection             *selection,
                                                                    HyScanGtkProfileEditorOffset *self);
static void    hyscan_gtk_profile_editor_offset_name_changed       (HyScanGtkProfileEditorOffset *self);
static gint    hyscan_gtk_profile_editor_offset_compare_func       (GtkTreeModel                 *model,
                                                                    GtkTreeIter                  *a,
                                                                    GtkTreeIter                  *b,
                                                                    gpointer                      user_data);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkProfileEditorOffset, hyscan_gtk_profile_editor_offset, HYSCAN_TYPE_GTK_PROFILE_EDITOR);

static void
hyscan_gtk_profile_editor_offset_class_init (HyScanGtkProfileEditorOffsetClass *klass)
{
  GtkWidgetClass *wclass = GTK_WIDGET_CLASS (klass);
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  oclass->constructed = hyscan_gtk_profile_editor_offset_object_constructed;
  oclass->finalize = hyscan_gtk_profile_editor_offset_object_finalize;

  gtk_widget_class_set_template_from_resource (wclass, WIDGET_RESOURCE_UI);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorOffset, name);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorOffset, stack);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkProfileEditorOffset, device_list);
}

static void
hyscan_gtk_profile_editor_offset_init (HyScanGtkProfileEditorOffset *self)
{
  HyScanGtkProfileEditorOffsetPrivate * priv;

  priv = hyscan_gtk_profile_editor_offset_get_instance_private (self);
  self->priv = priv;

  gtk_widget_init_template (GTK_WIDGET (self));

  priv->store = hyscan_gtk_profile_editor_offset_make_model (self);
  hyscan_gtk_profile_editor_offset_make_tree (self);
  gtk_tree_view_set_model (GTK_TREE_VIEW (priv->device_list), GTK_TREE_MODEL (priv->store));

  priv->known = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  priv->add_menu = hyscan_gtk_profile_editor_offset_make_menu (self);

  g_signal_connect_swapped (priv->name, "changed",
                            G_CALLBACK (hyscan_gtk_profile_editor_offset_name_changed),
                            self);
}

static GtkMenu *
hyscan_gtk_profile_editor_offset_make_menu (HyScanGtkProfileEditorOffset *self)
{
  GtkWidget *menu, *submenu;
  GtkWidget *mitem;
  gint j, t, subt;
  guint i;

  t = 0;
  menu = gtk_menu_new ();

  /* Преднастроенные локаторы. */
  submenu = gtk_menu_new ();
  for (i = 0, subt = 0; i < G_N_ELEMENTS (sonar_presets); ++i)
    {
      HyScanGtkProfileEditorOffsetMenuItem *item = &sonar_presets[i];
      GString *item_data;
      gchar *str;

      /* Список источников данных хранится в строковом виде. */
      item_data = g_string_new (NULL);
      for (j = 0; j < N_TYPES && item->types[j] != HYSCAN_SOURCE_INVALID; ++j)
        g_string_append_printf (item_data, "%s%s", hyscan_source_get_id_by_type (item->types[j]), SOURCE_LIST_DELIMITER);

      str = g_string_free (item_data, FALSE);

      /* Пункт меню. */
      mitem = gtk_menu_item_new_with_label (g_dgettext (GETTEXT_PACKAGE, item->title));
      g_object_set_data_full (G_OBJECT (mitem), SOURCE_LIST_USER_DATA, str, g_free);
      g_signal_connect (mitem, "activate",
                        G_CALLBACK (hyscan_gtk_profile_editor_offset_add_sonars), self);
      gtk_menu_attach (GTK_MENU (submenu), mitem, 0, 1, subt, subt+1);
      ++subt;
    }
  mitem = gtk_menu_item_new_with_label (_("Sonar presets"));
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (mitem), submenu);
  gtk_menu_attach (GTK_MENU (menu), mitem, 0, 1, t, t+1);
  ++t;

  /* Одиночные HyScanSourceType. */
  submenu = gtk_menu_new ();
  for (i = HYSCAN_SOURCE_INVALID, subt = 0; i < HYSCAN_SOURCE_LAST; ++i)
    {
      if (!hyscan_source_is_sonar (i))
        continue;

      mitem = gtk_menu_item_new_with_label (hyscan_source_get_name_by_type (i));
      g_object_set_data (G_OBJECT (mitem), SOURCE_LIST_USER_DATA,
                         (gpointer)hyscan_source_get_id_by_type (i));

      g_signal_connect (mitem, "activate",
                        G_CALLBACK (hyscan_gtk_profile_editor_offset_add_sonars), self);

      gtk_menu_attach (GTK_MENU (submenu), mitem, 0, 1, subt, subt+1);
      ++subt;
    }
  mitem = gtk_menu_item_new_with_label (_("Sonars"));
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (mitem), submenu);
  gtk_menu_attach (GTK_MENU (menu), mitem, 0, 1, t, t+1);
  ++t;

  /* Кнопка ручного ввода названия. */
  mitem = gtk_menu_item_new_with_label (_("Other device..."));
  g_signal_connect (mitem, "activate",
                    G_CALLBACK (hyscan_gtk_profile_editor_offset_add_sensor), self);
  gtk_menu_attach (GTK_MENU (menu), mitem, 0, 1, t, t+1);
  ++t;

  gtk_widget_show_all (menu);
  return GTK_MENU (menu);
}

static void
hyscan_gtk_profile_editor_offset_object_constructed (GObject *object)
{
  HyScanGtkProfileEditorOffset *self = HYSCAN_GTK_PROFILE_EDITOR_OFFSET (object);
  HyScanGtkProfileEditorOffsetPrivate *priv = self->priv;;
  HyScanProfile *profile;
  const gchar *name;

  profile = hyscan_gtk_profile_editor_get_profile (HYSCAN_GTK_PROFILE_EDITOR (self));
  priv->profile = HYSCAN_PROFILE_OFFSET (profile);

  name = hyscan_profile_get_name (profile);
  if (name != NULL)
    gtk_entry_set_text (priv->name, name);

  hyscan_gtk_profile_editor_offset_update (self);

  G_OBJECT_CLASS (hyscan_gtk_profile_editor_offset_parent_class)->constructed (object);
}

static void
hyscan_gtk_profile_editor_offset_object_finalize (GObject *object)
{
  HyScanGtkProfileEditorOffset *self = HYSCAN_GTK_PROFILE_EDITOR_OFFSET (object);
  HyScanGtkProfileEditorOffsetPrivate *priv = self->priv;

  g_clear_object (&priv->profile);
  g_clear_pointer (&priv->known, g_hash_table_unref);

  G_OBJECT_CLASS (hyscan_gtk_profile_editor_offset_parent_class)->finalize (object);
}


GtkWidget *
hyscan_gtk_profile_editor_offset_new (HyScanProfile *profile)
{
  return g_object_new (HYSCAN_TYPE_GTK_PROFILE_EDITOR_OFFSET,
                       "profile", profile,
                       NULL);
}

static GtkListStore *
hyscan_gtk_profile_editor_offset_make_model (HyScanGtkProfileEditorOffset *self)
{
  GtkListStore *store;

  store = gtk_list_store_new (N_COLUMNS,
                              G_TYPE_STRING, /* ID_COL */
                              G_TYPE_STRING, /* NAME_COL */
                              G_TYPE_STRING, /* ICON_NAME_COL */
                              G_TYPE_INT,    /* STYLE_COL */
                              G_TYPE_INT     /* ACTION_TYPE_COL */
                              );
  gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (store), NAME_COL,
                                   hyscan_gtk_profile_editor_offset_compare_func,
                                   NULL, NULL);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (store), NAME_COL, GTK_SORT_DESCENDING);

  return store;
}

static void
hyscan_gtk_profile_editor_offset_make_tree (HyScanGtkProfileEditorOffset *self)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeSelection *selection;
  GtkTreeView *view = GTK_TREE_VIEW (self->priv->device_list);

  /* Колонка с названием устройства. */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Name"),
                                                     renderer,
                                                     "text",
                                                     NAME_COL,
                                                     "style",
                                                     STYLE_COL,
                                                     NULL);
  gtk_tree_view_column_set_sort_column_id (column, NAME_COL);
  gtk_tree_view_column_set_expand (column, TRUE);
  gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_append_column (view, column);

  /* Колонка с иконкой. */
  renderer = hyscan_cell_renderer_pixbuf_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL,
                                                     renderer,
                                                     "icon-name",
                                                     ICON_NAME_COL,
                                                     NULL);
  g_signal_connect (renderer, "clicked",
                    G_CALLBACK (hyscan_gtk_profile_editor_offset_clicked), self);
  gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_append_column (view, column);

  selection = gtk_tree_view_get_selection (view);
  g_signal_connect (selection, "changed",
                    G_CALLBACK (hyscan_gtk_profile_editor_offset_selected),
                    self);
}

static void
hyscan_gtk_profile_editor_offset_clicked (HyScanCellRendererPixbuf     *rend,
                                          const gchar                  *path,
                                          GdkEvent                     *event,
                                          HyScanGtkProfileEditorOffset *self)
{
  HyScanGtkProfileEditorOffsetPrivate *priv = self->priv;
  GtkTreeIter iter;
  gchar *id = NULL;
  gint action;

  if (!gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (priv->store), &iter, path))
    return;

  gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter, ID_COL, &id, ACTION_TYPE_COL, &action, -1);

  if (action == ACTION_DELETE)
    hyscan_gtk_profile_editor_offset_remove (self, id);
  else if (action == ACTION_ADD)
    gtk_menu_popup_at_pointer (priv->add_menu, event);

  g_free (id);
}

static void
hyscan_gtk_profile_editor_offset_ensure_page (HyScanGtkProfileEditorOffset *self,
                                              const gchar                  *id,
                                              HyScanAntennaOffset          *offt)
{
  GtkBuilder *builder;
  GtkWidget *widget;
  HyScanGtkProfileEditorOffsetPage *page;

  if (g_hash_table_contains (self->priv->known, id))
    return;

  /* Создаем новую страницу. */
  page = g_new0 (HyScanGtkProfileEditorOffsetPage, 1);

  builder = gtk_builder_new_from_resource (PAGE_RESOURCE_UI);
  widget = GTK_WIDGET (g_object_ref (gtk_builder_get_object (builder, "page")));
  page->starboard = GTK_ADJUSTMENT (g_object_ref (gtk_builder_get_object (builder, "starboard")));
  page->forward = GTK_ADJUSTMENT (g_object_ref (gtk_builder_get_object (builder, "forward")));
  page->vertical = GTK_ADJUSTMENT (g_object_ref (gtk_builder_get_object (builder, "vertical")));
  page->roll = GTK_ADJUSTMENT (g_object_ref (gtk_builder_get_object (builder, "roll")));
  page->pitch = GTK_ADJUSTMENT (g_object_ref (gtk_builder_get_object (builder, "pitch")));
  page->yaw = GTK_ADJUSTMENT (g_object_ref (gtk_builder_get_object (builder, "yaw")));

  gtk_adjustment_set_value (page->starboard, offt->starboard);
  gtk_adjustment_set_value (page->forward, offt->forward);
  gtk_adjustment_set_value (page->vertical, offt->vertical);
  gtk_adjustment_set_value (page->roll, offt->roll);
  gtk_adjustment_set_value (page->pitch, offt->pitch);
  gtk_adjustment_set_value (page->yaw, offt->yaw);

  gtk_builder_add_callback_symbol (builder, "offset_changed",
                                   G_CALLBACK (hyscan_gtk_profile_editor_offset_changed));
  gtk_builder_connect_signals (builder, self);

  gtk_stack_add_named (self->priv->stack, widget, id);
  g_hash_table_insert (self->priv->known, g_strdup (id), page);

  g_object_unref (builder);
}

static void
hyscan_gtk_profile_editor_offset_add_sonars (GtkMenuItem                  *item,
                                             HyScanGtkProfileEditorOffset *self)
{
  HyScanGtkProfileEditorOffsetPrivate *priv = self->priv;
  HyScanAntennaOffset offt = {0, 0, 0, 0, 0, 0};
  HyScanSourceType source;
  const gchar *ids;
  gchar **split, **id;

  ids = g_object_get_data (G_OBJECT (item), SOURCE_LIST_USER_DATA);
  split = g_strsplit (ids, SOURCE_LIST_DELIMITER, -1);

  for (id = split; id != NULL && *id != NULL; ++id)
    {
      if (0 == g_strcmp0 (*id, ""))
        continue;

      source = hyscan_source_get_type_by_id (*id);
      hyscan_profile_offset_add_source (priv->profile, source, &offt);
    }

  g_strfreev (split);
  hyscan_gtk_profile_editor_offset_update (self);
}

static void
hyscan_gtk_profile_editor_offset_entry_sanity (GtkEntry  *entry,
                                               GtkDialog *dialog)
{
  const gchar *text = gtk_entry_get_text (entry);
  gboolean sane = (text != NULL && !g_str_equal (text, ""));

  gtk_dialog_set_response_sensitive (dialog, GTK_RESPONSE_OK, sane);
}

static void
hyscan_gtk_profile_editor_offset_add_sensor (GtkMenuItem                  *item,
                                             HyScanGtkProfileEditorOffset *self)
{
  HyScanAntennaOffset offt = {0, 0, 0, 0, 0, 0};
  GtkWidget *dialog, *content, *entry;
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
  GtkWindow *window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (self)));

  dialog = gtk_dialog_new_with_buttons (_("New device"),
                                        window, flags,
                                        _("_Cancel"), GTK_RESPONSE_CANCEL,
                                        _("_OK"), GTK_RESPONSE_OK,
                                        NULL);

  content = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  entry = gtk_entry_new ();
  gtk_entry_set_placeholder_text (GTK_ENTRY (entry), _("Enter sensor name"));
  g_signal_connect (entry, "changed",
                    G_CALLBACK (hyscan_gtk_profile_editor_offset_entry_sanity),
                    dialog);
  hyscan_gtk_profile_editor_offset_entry_sanity (GTK_ENTRY (entry), GTK_DIALOG (dialog));

  gtk_container_add (GTK_CONTAINER (content), entry);
  gtk_widget_show_all (dialog);

  if (GTK_RESPONSE_OK != gtk_dialog_run (GTK_DIALOG (dialog)))
    return;

  hyscan_profile_offset_add_sensor (self->priv->profile,
                                    gtk_entry_get_text (GTK_ENTRY (entry)),
                                    &offt);

  hyscan_gtk_profile_editor_offset_update (self);

  gtk_widget_destroy (dialog);
}

static void
hyscan_gtk_profile_editor_offset_remove (HyScanGtkProfileEditorOffset *self,
                                         const gchar                  *id)
{
  HyScanGtkProfileEditorOffsetPrivate *priv = self->priv;
  HyScanSourceType source;

  source = hyscan_source_get_type_by_id (id);
  if (source != HYSCAN_SOURCE_INVALID)
    hyscan_profile_offset_remove_source (priv->profile, source);
  else
    hyscan_profile_offset_remove_sensor (priv->profile, id);

  g_hash_table_remove (priv->known, id);
  gtk_widget_destroy (gtk_stack_get_child_by_name (priv->stack, id));

  hyscan_gtk_profile_editor_offset_update (self);
}

static void
hyscan_gtk_profile_editor_offset_update (HyScanGtkProfileEditorOffset *self)
{
  HyScanGtkProfileEditorOffsetPrivate *priv = self->priv;
  GHashTable *sources, *sensors;
  GHashTableIter iter;
  GtkTreeIter ls_iter;
  HyScanAntennaOffset *offt;
  HyScanSourceType source;
  const gchar *name;
  const gchar *id;

  /* Выглядит как грязный хак, но похоже это рабочий способ избежать
   * ворнинга при удалении строки. */
  {
    GtkTreeSelection *selection = gtk_tree_view_get_selection (self->priv->device_list);
    gtk_tree_selection_set_mode  (selection, GTK_SELECTION_NONE);
    gtk_list_store_clear (priv->store);
    gtk_tree_selection_set_mode  (selection, GTK_SELECTION_SINGLE);
  }

  sources = hyscan_profile_offset_list_sources (priv->profile);
  g_hash_table_iter_init (&iter, sources);
  while (g_hash_table_iter_next (&iter, (gpointer)&source, (gpointer)&offt))
    {
      id = hyscan_source_get_id_by_type (source);
      name = hyscan_source_get_name_by_type (source);

      hyscan_gtk_profile_editor_offset_ensure_page (self, id, offt);

      gtk_list_store_append (priv->store, &ls_iter);
      gtk_list_store_set (priv->store, &ls_iter,
                          ID_COL, /*g_strdup*/ (id),
                          NAME_COL, /*g_strdup*/ (name),
                          ICON_NAME_COL, "list-remove-symbolic",
                          STYLE_COL, PANGO_STYLE_NORMAL,
                          ACTION_TYPE_COL, ACTION_DELETE,
                          -1);
    }

  sensors = hyscan_profile_offset_list_sensors (priv->profile);
  g_hash_table_iter_init (&iter, sensors);
  while (g_hash_table_iter_next (&iter, (gpointer)&id, (gpointer)&offt))
    {
      hyscan_gtk_profile_editor_offset_ensure_page (self, id, offt);

      gtk_list_store_append (priv->store, &ls_iter);
      gtk_list_store_set (priv->store, &ls_iter,
                          ID_COL, /*g_strdup*/ (id),
                          NAME_COL, /*g_strdup*/ (id),
                          ICON_NAME_COL, "list-remove-symbolic",
                          STYLE_COL, PANGO_STYLE_NORMAL,
                          ACTION_TYPE_COL, ACTION_DELETE,
                          -1);
    }

  gtk_list_store_append (priv->store, &ls_iter);
  gtk_list_store_set (priv->store, &ls_iter,
                      NAME_COL, /*g_strdup*/(_("New...")),
                      ICON_NAME_COL, "list-add-symbolic",
                      STYLE_COL, PANGO_STYLE_ITALIC,
                      ACTION_TYPE_COL, ACTION_ADD,
                      -1);

  g_hash_table_unref (sources);
  g_hash_table_unref (sensors);

  hyscan_gtk_profile_editor_check_sanity (HYSCAN_GTK_PROFILE_EDITOR (self));
}

static void
hyscan_gtk_profile_editor_offset_changed (GtkSpinButton                *button,
                                          HyScanGtkProfileEditorOffset *self)
{
  HyScanGtkProfileEditorOffsetPrivate *priv = self->priv;
  GHashTableIter iter;
  HyScanGtkProfileEditorOffsetPage *page;
  const gchar *id;

  g_hash_table_iter_init (&iter, priv->known);
  while (g_hash_table_iter_next (&iter, (gpointer)&id, (gpointer)&page))
    {
      HyScanSourceType source;
      HyScanAntennaOffset offt;

      offt.starboard = gtk_adjustment_get_value (page->starboard);
      offt.forward = gtk_adjustment_get_value (page->forward);
      offt.vertical = gtk_adjustment_get_value (page->vertical);
      offt.yaw = gtk_adjustment_get_value (page->yaw);
      offt.pitch = gtk_adjustment_get_value (page->pitch);
      offt.roll = gtk_adjustment_get_value (page->roll);

      source = hyscan_source_get_type_by_id (id);

      if (source != HYSCAN_SOURCE_INVALID)
        hyscan_profile_offset_add_source (priv->profile, source, &offt);
      else
        hyscan_profile_offset_add_sensor (priv->profile, id, &offt);
    }
}

static void
hyscan_gtk_profile_editor_offset_selected (GtkTreeSelection             *selection,
                                           HyScanGtkProfileEditorOffset *self)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *id;

  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    return;

  gtk_tree_model_get (model, &iter, ID_COL, &id, -1);

  if (id == NULL)
    gtk_tree_selection_unselect_iter (selection, &iter);

  gtk_stack_set_visible_child_name (self->priv->stack, id);
  g_free (id);
}

static void
hyscan_gtk_profile_editor_offset_name_changed (HyScanGtkProfileEditorOffset *self)
{
  const gchar *name;

  name = gtk_entry_get_text (self->priv->name);
  hyscan_profile_set_name (HYSCAN_PROFILE (self->priv->profile), name);
  hyscan_gtk_profile_editor_check_sanity (HYSCAN_GTK_PROFILE_EDITOR (self));
}

/* Функция сортировки. Она довольно хитрая, чтобы иметь возможность
 * фиксировать отдельные кнопки вверху или внизу.*/
static gint
hyscan_gtk_profile_editor_offset_compare_func (GtkTreeModel *model,
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
