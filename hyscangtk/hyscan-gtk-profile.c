/* hyscan-gtk-profile.c
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
 * SECTION: hyscan-gtk-profile
 * @Title: HyScanGtkProfile
 * @Short_description: Общий класс для виджетов профилей
 *
 * Класс позволяет относительно легко создавать новые профили и гарантировать
 * одинаковое поведение у виджетов, реализующих профили.
 *
 *              HyScanGtk                          HyScanCore
 * +-----------------------------------+  +-----------------------------+
 * |          GtkProfile               |  |         Profile             |
 * |      +------> ^ <---------+       |  |   +------> ^ <------+       |
 * |      |        |           |       |  |   |        |        |       |
 * |      +        |           |       |  |   +        |        |       |
 * | GtkProfileDB--|-----------|------------ProfileDB  |        |       |
 * |    +          +           |       |  |            +        |       |
 * |    |      GtkProfileHW----|-------------------+ProfileHW   |       |
 * |    |          +           +       |  |           +         +       |
 * |    +          | GtkProfileOffset-----------------|--+ProfileOffset |
 * | Gtk..DBEditor |              +    |  |           +                 |
 * |               +              |    |  |       ProfileHWDevice       |
 * |         Gtk..HWEditor        |    |  +-----------------------------+
 * |               +              |    |
 * |       Gtk..EditorHWDevice    |    |
 * |                              +    |
 * |                 Gtk..EditorOffset |
 * +-----------------------------------+
 *
 */

#include "hyscan-gtk-profile.h"
#include <glib/gi18n-lib.h>
#include <hyscan-cell-renderer-pixbuf.h>
#include <hyscan-config.h>
#include <hyscan-types.h>
#include <glib/gstdio.h>

enum
{
  RESPONSE_CANCEL,
  RESPONSE_APPLY,
  RESPONSE_DELETE
};

enum
{
  ROW_INVALID = -1,
  ROW_NOT_SELECTED = 0,
  ROW_PROFILE,
  ROW_NEW_PROFILE
};

enum
{
  SELECTOR_COL,
  SELECTOR_VISIBLE,
  PATH_COL,
  NAME_COL,
  STYLE_COL,
  OBJECT_COL,
  ROW_TYPE_COL,
  ICON_NAME_COL,
  N_COLUMNS
};

enum
{
  SIGNAL_SELECTED,
  SIGNAL_LAST
};

enum
{
  PROP_0,
  PROP_FOLDERS,
  PROP_READONLY,
};

struct _HyScanGtkProfilePrivate
{
  gchar        **folders;          /* Список папок. */
  gboolean       readonly;         /* Возможно ли редактирование/создание профилей. */

  GHashTable    *profiles;         /* {gchar* file_name : HyScanProfile*} */
  GtkTreeModel  *store;            /* Модель с профилями. */

  HyScanProfile *selected_profile; /* Выбранный профиль. */
};

static void    hyscan_gtk_profile_set_property             (GObject               *object,
                                                            guint                  prop_id,
                                                            const GValue          *value,
                                                            GParamSpec            *pspec);
static void    hyscan_gtk_profile_object_constructed       (GObject               *object);
static void    hyscan_gtk_profile_object_finalize          (GObject               *object);

static GHashTable * hyscan_gtk_profile_get_files           (const gchar           *folder,
                                                            gboolean               create);
static void    hyscan_gtk_profile_load                     (HyScanGtkProfile      *self,
                                                            const gchar           *dir);

static void    hyscan_gtk_profile_edit                     (HyScanGtkProfile      *self,
                                                            HyScanProfile         *profile);
static void    hyscan_gtk_profile_create                   (HyScanGtkProfile      *self);

static void    hyscan_gtk_profile_clicked                  (GtkCellRenderer       *cell_renderer,
                                                            const gchar           *path,
                                                            GdkEvent              *event,
                                                            gpointer               user_data);
static void    hyscan_gtk_profile_toggled                  (GtkCellRendererToggle *cell_renderer,
                                                            gchar                 *path,
                                                            gpointer               user_data);

static void    hyscan_gtk_profile_make_tree                (HyScanGtkProfile      *self);
static GtkTreeModel * hyscan_gtk_profile_make_model        (HyScanGtkProfile      *self);
static void    hyscan_gtk_profile_update_tree              (HyScanGtkProfile      *self);
static gint    hyscan_gtk_profile_compare_func             (GtkTreeModel          *model,
                                                            GtkTreeIter           *a,
                                                            GtkTreeIter           *b,
                                                            gpointer               user_data);
static void    hyscan_gtk_profile_restyle                  (GtkDialog             *dialog,
                                                            gint                   response_id,
                                                            const gchar           *style_class);
static void    hyscan_gtk_profile_sanity_check             (HyScanGtkProfileEditor *editor,
                                                            GtkDialog              *dialog);

static guint   hyscan_gtk_profile_signals[SIGNAL_LAST] = {0};

G_DEFINE_ABSTRACT_TYPE_WITH_CODE (HyScanGtkProfile, hyscan_gtk_profile, GTK_TYPE_TREE_VIEW,
                                  G_ADD_PRIVATE (HyScanGtkProfile));

static void
hyscan_gtk_profile_class_init (HyScanGtkProfileClass *klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  oclass->set_property = hyscan_gtk_profile_set_property;
  oclass->constructed = hyscan_gtk_profile_object_constructed;
  oclass->finalize = hyscan_gtk_profile_object_finalize;

  /* Есть кое-какие реализации по умолчанию. */
  klass->make_model = hyscan_gtk_profile_make_model;
  klass->make_tree = hyscan_gtk_profile_make_tree;
  klass->update_tree = hyscan_gtk_profile_update_tree;

  g_object_class_install_property (oclass, PROP_FOLDERS,
    g_param_spec_pointer ("folders", "Folders", "Folders to look for profiles",
                          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (oclass, PROP_READONLY,
    g_param_spec_boolean ("readonly", "Read-only", "Disable profile editing",
                          FALSE, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  hyscan_gtk_profile_signals[SIGNAL_SELECTED] =
    g_signal_new ("selected", G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST, 0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1, G_TYPE_OBJECT);
}

static void
hyscan_gtk_profile_init (HyScanGtkProfile *self)
{
  HyScanGtkProfileClass *klass = HYSCAN_GTK_PROFILE_GET_CLASS (self);
  HyScanGtkProfilePrivate *priv = hyscan_gtk_profile_get_instance_private (self);
  self->priv = priv;

  priv->profiles = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  priv->store = klass->make_model (self);
  klass->make_tree (self);
}

static void
hyscan_gtk_profile_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  HyScanGtkProfile *self = HYSCAN_GTK_PROFILE (object);
  HyScanGtkProfilePrivate *priv = self->priv;

  switch (prop_id)
    {
    case PROP_FOLDERS:
      priv->folders = g_strdupv ((gchar**) g_value_get_pointer (value));
      break;

    case PROP_READONLY:
      priv->readonly = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
hyscan_gtk_profile_object_constructed (GObject *object)
{
  HyScanGtkProfile *self = HYSCAN_GTK_PROFILE (object);
  HyScanGtkProfileClass *klass = HYSCAN_GTK_PROFILE_GET_CLASS (self);
  HyScanGtkProfilePrivate *priv = self->priv;
  gchar **iter;

  G_OBJECT_CLASS (hyscan_gtk_profile_parent_class)->constructed (object);

  /* Профили загружаются из каталогов, переданных на этапе конструирования. */
  for (iter = priv->folders; iter != NULL && *iter != NULL; ++iter)
    {
      gchar *folder = g_build_filename (*iter, klass->subfolder, NULL);
      hyscan_gtk_profile_load (self, folder);
      g_free (folder);
    }

  klass->update_tree (self);
}

static void
hyscan_gtk_profile_object_finalize (GObject *object)
{
  HyScanGtkProfile *self = HYSCAN_GTK_PROFILE (object);
  HyScanGtkProfilePrivate *priv = self->priv;

  g_clear_pointer (&priv->folders, g_strfreev);
  g_clear_pointer (&priv->profiles, g_hash_table_unref);

  G_OBJECT_CLASS (hyscan_gtk_profile_parent_class)->finalize (object);
}

/* Функция возвращает список файлов в папке. */
static GHashTable *
hyscan_gtk_profile_get_files (const gchar *folder,
                              gboolean     create)
{
  GHashTable *set;
  const gchar *filename;
  GError *error = NULL;
  GDir *dir;

  if (!g_file_test (folder, G_FILE_TEST_IS_DIR | G_FILE_TEST_EXISTS))
    {
    #if G_OS_UNIX
      gint flags = S_IRUSR | S_IWUSR | S_IXUSR |
                   S_IRGRP | S_IWGRP | S_IXGRP |
                   S_IROTH | S_IXOTH;
    #else
      gint flags = 0;
    #endif

      if (!create)
        {
          g_warning ("HyScanGtkProfile: directory %s doesn't exist", folder);
          return NULL;
        }
      else if (0 != g_mkdir (folder, flags))
        {
          g_warning ("HyScanGtkProfile: directory %s "
                     "doesn't exist and can't be created", folder);
          return NULL;
        }
    }

  dir = g_dir_open (folder, 0, &error);
  if (error != NULL)
    {
      g_warning ("HyScanGtkProfile: %s", error->message);
      g_clear_pointer (&error, g_error_free);
      return NULL;
    }

  set = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  while ((filename = g_dir_read_name (dir)) != NULL)
    {
      gchar *fullname = g_build_filename (folder, filename, NULL);
      g_hash_table_add (set, fullname);
    }

  g_dir_close (dir);

  return set;
}

/* Функция загружает профили. */
static void
hyscan_gtk_profile_load (HyScanGtkProfile *self,
                         const gchar      *folder)
{
  HyScanGtkProfileClass *klass = HYSCAN_GTK_PROFILE_GET_CLASS (self);
  HyScanGtkProfilePrivate *priv = self->priv;
  GHashTable *files;
  GHashTableIter iter;
  const gchar *filename;

  files = hyscan_gtk_profile_get_files (folder, FALSE);
  if (files == NULL)
    return;

  g_hash_table_iter_init (&iter, files);
  while (g_hash_table_iter_next (&iter, (gpointer*)&filename, NULL))
    {
      HyScanProfile *profile;

      profile = klass->new_profile (self, filename);
      hyscan_profile_read (profile);

      if (profile != NULL)
        g_hash_table_insert (priv->profiles, g_strdup (filename), profile);
    }

  g_hash_table_unref (files);
}

/* Функция редактирования существующего профиля. */
static void
hyscan_gtk_profile_edit (HyScanGtkProfile *self,
                         HyScanProfile    *profile)
{
  HyScanGtkProfileClass *klass = HYSCAN_GTK_PROFILE_GET_CLASS (self);
  HyScanGtkProfilePrivate *priv = self->priv;
  GtkWidget *dialog, *content, *creator;
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
  GtkWindow *window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (self)));

  dialog = gtk_dialog_new_with_buttons ("Edit profile",
                                        window,
                                        flags,
                                        _("Delete"), RESPONSE_DELETE,
                                        _("Cancel"), RESPONSE_CANCEL,
                                        _("OK"), RESPONSE_APPLY,
                                        NULL);
  hyscan_gtk_profile_restyle (GTK_DIALOG (dialog), RESPONSE_APPLY,
                              GTK_STYLE_CLASS_SUGGESTED_ACTION);
  hyscan_gtk_profile_restyle (GTK_DIALOG (dialog), RESPONSE_DELETE,
                              GTK_STYLE_CLASS_DESTRUCTIVE_ACTION);

  content = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

  creator = klass->make_editor (self, profile);
  hyscan_gtk_profile_sanity_check (HYSCAN_GTK_PROFILE_EDITOR (creator), GTK_DIALOG (dialog));
  g_signal_connect (creator, "changed", G_CALLBACK (hyscan_gtk_profile_sanity_check), dialog);

  gtk_container_add (GTK_CONTAINER (content), creator);
  gtk_widget_show_all (dialog);
  switch (gtk_dialog_run (GTK_DIALOG (dialog)))
    {
      const gchar *filename;

      case RESPONSE_APPLY:
        filename = hyscan_profile_get_file (profile);
        hyscan_profile_write (profile);
        g_hash_table_insert (priv->profiles, g_strdup (filename), profile);
        hyscan_gtk_profile_update_tree (self);
        break;

      case RESPONSE_DELETE:
        hyscan_profile_delete (profile);
        g_hash_table_remove (priv->profiles, hyscan_profile_get_file (profile));
        hyscan_gtk_profile_update_tree (self);
        break;

      case RESPONSE_CANCEL:
      default:
        hyscan_profile_read (profile);
    }

  gtk_widget_destroy (dialog);
}

/* Функция создания нового профиля. */
static void
hyscan_gtk_profile_create (HyScanGtkProfile *self)
{
  HyScanGtkProfileClass *klass = HYSCAN_GTK_PROFILE_GET_CLASS (self);
  HyScanProfile *profile;
  gchar *filename = NULL, *folder = NULL;
  gchar random_str[20];
  GHashTable *files;

  /* Сначала сгененрируем новое уникальное имя для профиля. */
  folder = g_build_filename (hyscan_config_get_user_files_dir(), klass->subfolder, NULL);
  files = hyscan_gtk_profile_get_files (folder, TRUE);

  do
    {
      g_clear_pointer (&filename, g_free);
      hyscan_rand_id (random_str, G_N_ELEMENTS (random_str));
      filename = g_build_filename (folder, random_str, NULL);
    }
  while (g_hash_table_contains (files, filename));
  g_hash_table_unref (files);

  /* Всё, можно создавать профиль. */
  profile = klass->new_profile (self, filename);
  hyscan_gtk_profile_edit (self, profile);
  g_object_unref (profile);
}

/* Обработчик нажатия на кнопку добавления/удаления. */
static void
hyscan_gtk_profile_clicked (GtkCellRenderer *cell_renderer,
                            const gchar     *path,
                            GdkEvent        *event,
                            gpointer         user_data)
{
  HyScanGtkProfile *self = user_data;
  HyScanGtkProfilePrivate *priv = self->priv;
  GtkTreeIter iter;
  HyScanProfile *profile;
  gint row_type;

  if (!gtk_tree_model_get_iter_from_string (priv->store, &iter, path))
    return;

  gtk_tree_model_get (priv->store, &iter,
                      OBJECT_COL, &profile,
                      ROW_TYPE_COL, &row_type, -1);

  if (row_type == ROW_NEW_PROFILE)
    hyscan_gtk_profile_create (HYSCAN_GTK_PROFILE (self));
  else if (row_type == ROW_PROFILE)
    hyscan_gtk_profile_edit (self, profile);

  g_clear_object (&profile);
}

/* Функция выбора профиля. */
static void
hyscan_gtk_profile_toggled (GtkCellRendererToggle *cell_renderer,
                            gchar                 *path,
                            gpointer               user_data)
{
  HyScanGtkProfile *self = user_data;
  HyScanGtkProfilePrivate *priv = self->priv;
  GtkTreeIter iter;
  HyScanProfile *profile;

  if (!gtk_tree_model_get_iter_from_string (priv->store, &iter, path))
    return;

  gtk_tree_model_get (priv->store, &iter, OBJECT_COL, &profile, -1);

  g_clear_object (&priv->selected_profile);
  if (profile != NULL)
    priv->selected_profile = profile;

  hyscan_gtk_profile_update_tree (self);

  /* Передаем, кто сейчас выбран. */
  g_signal_emit (self, hyscan_gtk_profile_signals[SIGNAL_SELECTED], 0, profile);
}

/* Функция создает дерево по умолчанию (одна колонка - название профиля). */
static void
hyscan_gtk_profile_make_tree (HyScanGtkProfile *self)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  /* Выбор профиля. */
  {
    renderer = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_toggle_set_radio (GTK_CELL_RENDERER_TOGGLE (renderer), TRUE);
    g_signal_connect (renderer, "toggled",
                      G_CALLBACK (hyscan_gtk_profile_toggled), self);
    column = gtk_tree_view_column_new_with_attributes (NULL,
                                                       renderer,
                                                       "active", SELECTOR_COL,
                                                       "visible", SELECTOR_VISIBLE,
                                                       NULL);
    gtk_tree_view_column_set_expand (column, FALSE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (self), column);
  }

  /* Колонка с названием профиля. */
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
  gtk_tree_view_append_column (GTK_TREE_VIEW (self), column);

  /* Колонка с иконкой правки. */
  if (!self->priv->readonly)
    {
      renderer = hyscan_cell_renderer_pixbuf_new ();
      column = gtk_tree_view_column_new_with_attributes (NULL,
                                                         renderer,
                                                         "icon-name",
                                                         ICON_NAME_COL,
                                                         NULL);
      g_signal_connect (renderer, "clicked",
                        G_CALLBACK (hyscan_gtk_profile_clicked), self);
      gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
      gtk_tree_view_append_column (GTK_TREE_VIEW (self), column);
    }
}

/* Функция создает модель по умолчанию. */
static GtkTreeModel *
hyscan_gtk_profile_make_model (HyScanGtkProfile *self)
{
  GtkTreeSelection *select = NULL;
  GtkListStore *store = NULL;

  store = gtk_list_store_new (N_COLUMNS,
                              G_TYPE_BOOLEAN,  /* SELECTOR_COL */
                              G_TYPE_BOOLEAN,  /* SELECTOR_VISIBLE */
                              G_TYPE_STRING,   /* PATH_COL */
                              G_TYPE_STRING,   /* NAME_COL */
                              G_TYPE_INT,      /* STYLE_COL */
                              G_TYPE_OBJECT,   /* OBJECT_COL */
                              G_TYPE_INT,      /* ROW_TYPE_COL */
                              G_TYPE_STRING);  /* ICON_NAME_COL */

  gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (store), NAME_COL,
                                   hyscan_gtk_profile_compare_func,
                                   NULL, NULL);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (store), NAME_COL, GTK_SORT_DESCENDING);

  gtk_tree_view_set_model (GTK_TREE_VIEW (self), GTK_TREE_MODEL (store));

  select = gtk_tree_view_get_selection (GTK_TREE_VIEW (self));
  gtk_tree_selection_set_mode (select, GTK_SELECTION_NONE);

  return GTK_TREE_MODEL (store);
}

/* Функция обновляет дерево. */
static void
hyscan_gtk_profile_update_tree (HyScanGtkProfile *self)
{
  HyScanGtkProfileClass *klass = HYSCAN_GTK_PROFILE_GET_CLASS (self);
  HyScanGtkProfilePrivate *priv = self->priv;
  GtkListStore *store = GTK_LIST_STORE (priv->store);
  GHashTableIter iter;
  GtkTreeIter ls_iter;
  gpointer k, v;

  gtk_list_store_clear (store);

  /* Специальные строки: "не выбрано" и "новый". */
  gtk_list_store_append (store, &ls_iter);
  gtk_list_store_set (store, &ls_iter,
                      SELECTOR_COL, NULL == priv->selected_profile,
                      SELECTOR_VISIBLE, TRUE,
                      PATH_COL, NULL,
                      NAME_COL, _("Not selected"),
                      STYLE_COL, PANGO_STYLE_ITALIC,
                      OBJECT_COL, NULL,
                      ROW_TYPE_COL, ROW_NOT_SELECTED,
                      ICON_NAME_COL, NULL,
                      -1);

  if (klass->make_editor != NULL && !priv->readonly)
    {
      gtk_list_store_append (store, &ls_iter);
      gtk_list_store_set (store, &ls_iter,
                          SELECTOR_VISIBLE, FALSE,
                          PATH_COL, NULL,
                          NAME_COL, _("New..."),
                          STYLE_COL, PANGO_STYLE_ITALIC,
                          OBJECT_COL, NULL,
                          ROW_TYPE_COL, ROW_NEW_PROFILE,
                          ICON_NAME_COL, "list-add-symbolic",
                          -1);
    }

  g_hash_table_iter_init (&iter, self->priv->profiles);
  while (g_hash_table_iter_next (&iter, &k, &v))
    {
      const gchar * file = (const gchar *)k;
      HyScanProfile * profile = (HyScanProfile *)v;

      /* Добавляем в деревце. */
      gtk_list_store_append (store, &ls_iter);
      gtk_list_store_set (store, &ls_iter,
                          SELECTOR_COL, profile == priv->selected_profile,
                          SELECTOR_VISIBLE, TRUE,
                          PATH_COL, file,
                          NAME_COL, hyscan_profile_get_name (profile),
                          STYLE_COL, PANGO_STYLE_NORMAL,
                          OBJECT_COL, g_object_ref (profile),
                          ROW_TYPE_COL, ROW_PROFILE,
                          ICON_NAME_COL, "emblem-system-symbolic",
                          -1);
    }
}

/* Функция сортировки. Она довольно хитрая, чтобы иметь возможность
 * фиксировать отдельные кнопки вверху или внизу.*/
static gint
hyscan_gtk_profile_compare_func (GtkTreeModel *model,
                                 GtkTreeIter  *a,
                                 GtkTreeIter  *b,
                                 gpointer      user_data)
{
  gchar *name_a, *name_b;
  gint type_a, type_b, result;

  gtk_tree_model_get (model, a, NAME_COL, &name_a, ROW_TYPE_COL, &type_a, -1);
  gtk_tree_model_get (model, b, NAME_COL, &name_b, ROW_TYPE_COL, &type_b, -1);

  /* Сортировка идет по 2 полям: тип строки (ROW_TYPE_COL) и названию.
   * Тип строки приоритетен. */
  if (type_a == type_b)
    {
      /* Деление необходимо, т.к. на выходе должна быть величина [-1, 0, 1],
       * a g_strcmp0 гарантирует возврат 0, положительного или отрицательного
       * значения. */
      result = g_strcmp0 (name_a, name_b);
      result = result == 0 ? 0 : result / ABS (result);
    }
  else
    {
      /* Здесь я домножаю на -1, чтобы при сортировке по убыванию специальные
       * строки оставались на своих местах. */
      GtkSortType order;
      gtk_tree_sortable_get_sort_column_id (GTK_TREE_SORTABLE (model), NULL, &order);

      result = type_a < type_b ? -1 : 1;
      if (order == GTK_SORT_DESCENDING)
        result *= -1;
    }

  g_free (name_a);
  g_free (name_b);

  return result;
}

/* Вспомогательная функция добавления стиля к диалоговым кнопкам. */
static void
hyscan_gtk_profile_restyle (GtkDialog   *dialog,
                            gint         response_id,
                            const gchar *style_class)
{
  GtkWidget *widget = gtk_dialog_get_widget_for_response (dialog, response_id);
  GtkStyleContext *context = gtk_widget_get_style_context (widget);

  gtk_style_context_add_class (context, style_class);
}

/* Функция активации/деактивации кнопок диалога. */
static void
hyscan_gtk_profile_sanity_check (HyScanGtkProfileEditor *editor,
                                 GtkDialog              *dialog)
{
  gboolean sane = hyscan_gtk_profile_editor_get_sanity (editor);
  gtk_dialog_set_response_sensitive (dialog, RESPONSE_APPLY, sane);
}
