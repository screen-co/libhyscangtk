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

#define HYSCAN_GTK_PROFILE_PROFILE "hyscan-gtk-profile"
enum
{
  RESPONSE_CANCEL,
  RESPONSE_APPLY,
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

  GtkListBox    *list_box;         /* Виджет со списком профилей. */
  GtkWidget     *action_bar;       /* Контейнер с действиями над профилями. */

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

static gboolean hyscan_gtk_profile_edit                     (HyScanGtkProfile      *self,
                                                            HyScanProfile         *profile);
static void    hyscan_gtk_profile_add                      (HyScanGtkProfile      *self);
static void    hyscan_gtk_profile_remove                   (HyScanGtkProfile      *self);
static void    hyscan_gtk_profile_config                   (HyScanGtkProfile      *self);
static void    hyscan_gtk_profile_row_selected             (GtkListBox            *box,
                                                            GtkListBoxRow         *row,
                                                            gpointer               user_data);

static GtkWidget * hyscan_gtk_profile_make_row             (HyScanGtkProfile      *self,
                                                            HyScanProfile         *profile);
static void    hyscan_gtk_profile_update_list              (HyScanGtkProfile      *self);
static void    hyscan_gtk_profile_restyle                  (GtkDialog             *dialog,
                                                            gint                   response_id,
                                                            const gchar           *style_class);
static void    hyscan_gtk_profile_sanity_check             (HyScanGtkProfileEditor *editor,
                                                            GtkDialog              *dialog);

static guint   hyscan_gtk_profile_signals[SIGNAL_LAST] = {0};

G_DEFINE_ABSTRACT_TYPE_WITH_CODE (HyScanGtkProfile, hyscan_gtk_profile, GTK_TYPE_GRID,
                                  G_ADD_PRIVATE (HyScanGtkProfile));

static void
hyscan_gtk_profile_class_init (HyScanGtkProfileClass *klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  oclass->set_property = hyscan_gtk_profile_set_property;
  oclass->constructed = hyscan_gtk_profile_object_constructed;
  oclass->finalize = hyscan_gtk_profile_object_finalize;

  /* Есть кое-какие реализации по умолчанию. */
  klass->update_list = hyscan_gtk_profile_update_list;

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
  GtkWidget *add, *remove, *config;
  HyScanGtkProfilePrivate *priv = hyscan_gtk_profile_get_instance_private (self);
  self->priv = priv;

  priv->profiles = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  priv->list_box = GTK_LIST_BOX (gtk_list_box_new ());
  gtk_list_box_set_selection_mode (priv->list_box, GTK_SELECTION_SINGLE);
  g_signal_connect (priv->list_box, "row-selected", G_CALLBACK (hyscan_gtk_profile_row_selected), self);
  gtk_widget_set_hexpand (GTK_WIDGET (priv->list_box), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET (priv->list_box), TRUE);

  add = gtk_button_new_from_icon_name ("list-add-symbolic", GTK_ICON_SIZE_BUTTON);
  remove = gtk_button_new_from_icon_name ("list-remove-symbolic", GTK_ICON_SIZE_BUTTON);
  config = gtk_button_new_from_icon_name ("emblem-system-symbolic", GTK_ICON_SIZE_BUTTON);
  g_signal_connect_swapped (add, "clicked", G_CALLBACK (hyscan_gtk_profile_add), self);
  g_signal_connect_swapped (remove, "clicked", G_CALLBACK (hyscan_gtk_profile_remove), self);
  g_signal_connect_swapped (config, "clicked", G_CALLBACK (hyscan_gtk_profile_config), self);

  priv->action_bar = gtk_action_bar_new ();
  gtk_action_bar_pack_start (GTK_ACTION_BAR (priv->action_bar), config);
  gtk_action_bar_pack_start (GTK_ACTION_BAR (priv->action_bar), add);
  gtk_action_bar_pack_start (GTK_ACTION_BAR (priv->action_bar), remove);

  gtk_grid_attach (GTK_GRID (self), GTK_WIDGET (priv->list_box), 0, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (self), GTK_WIDGET (priv->action_bar), 0, 1, 1, 1);
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

  if (priv->readonly)
    {
      gtk_widget_set_no_show_all (priv->action_bar, TRUE);
      gtk_widget_hide (priv->action_bar);
    }

  klass->update_list (self);
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
    #ifdef G_OS_UNIX
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
static gboolean
hyscan_gtk_profile_edit (HyScanGtkProfile *self,
                         HyScanProfile    *profile)
{
  gboolean response;
  HyScanGtkProfileClass *klass = HYSCAN_GTK_PROFILE_GET_CLASS (self);
  GtkWidget *dialog, *content, *creator;
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
  GtkWindow *window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (self)));

  dialog = gtk_dialog_new_with_buttons ("Edit profile",
                                        window,
                                        flags,
                                        _("Cancel"), RESPONSE_CANCEL,
                                        _("OK"), RESPONSE_APPLY,
                                        NULL);
  hyscan_gtk_profile_restyle (GTK_DIALOG (dialog), RESPONSE_APPLY,
                              GTK_STYLE_CLASS_SUGGESTED_ACTION);

  content = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

  creator = klass->make_editor (self, profile);
  hyscan_gtk_profile_sanity_check (HYSCAN_GTK_PROFILE_EDITOR (creator), GTK_DIALOG (dialog));
  g_signal_connect (creator, "changed", G_CALLBACK (hyscan_gtk_profile_sanity_check), dialog);

  gtk_container_add (GTK_CONTAINER (content), creator);
  gtk_widget_show_all (dialog);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  if (RESPONSE_APPLY == response)
    {
      hyscan_profile_write (profile);
      return TRUE;
    }
  else /* if (RESPONSE_CANCEL == response) */
    {
      hyscan_profile_read (profile);
      return FALSE;
    }
}

/* Функция создания нового профиля. */
static void
hyscan_gtk_profile_add (HyScanGtkProfile *self)
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

  /* Всё, можно создавать профиль и открывать диалог редактирования. */
  profile = klass->new_profile (self, filename);
  if (hyscan_gtk_profile_edit (self, profile))
    {
      g_hash_table_insert (self->priv->profiles, g_strdup (filename), g_object_ref (profile));
      hyscan_gtk_profile_update_list (self);
    }

  g_object_unref (profile);
}

/* Обработчик нажатия на кнопку добавления/удаления. */
static void
hyscan_gtk_profile_remove (HyScanGtkProfile *self)
{
  HyScanGtkProfilePrivate *priv = self->priv;

  g_hash_table_remove (priv->profiles, hyscan_profile_get_file (priv->selected_profile));
  hyscan_profile_delete (priv->selected_profile);
  g_clear_object (&priv->selected_profile);
  hyscan_gtk_profile_update_list (self);
}

/* Обработчик нажатия на кнопку добавления/удаления. */
static void
hyscan_gtk_profile_config (HyScanGtkProfile *self)
{
  if (hyscan_gtk_profile_edit (self, self->priv->selected_profile))
    hyscan_gtk_profile_update_list (self);
}

static void
hyscan_gtk_profile_row_selected (GtkListBox    *box,
                                 GtkListBoxRow *row,
                                 gpointer       user_data)
{
  HyScanGtkProfile *self = HYSCAN_GTK_PROFILE (user_data);
  HyScanProfile *profile = NULL;

  if (row != NULL)
    profile = g_object_get_data (G_OBJECT (row), HYSCAN_GTK_PROFILE_PROFILE);

  g_clear_object (&self->priv->selected_profile);
  if (profile != NULL)
    self->priv->selected_profile = g_object_ref (profile);

  g_signal_emit (self, hyscan_gtk_profile_signals[SIGNAL_SELECTED], 0, profile);
}

/* Функция создает виджет для отображения по-умолчанию. */
static GtkWidget *
hyscan_gtk_profile_make_row (HyScanGtkProfile *self,
                             HyScanProfile    *profile)
{
  HyScanGtkProfileClass *klass = HYSCAN_GTK_PROFILE_GET_CLASS (self);
  GtkWidget *row, *label;

  row = gtk_list_box_row_new ();

  if (klass->make_row != NULL)
    {
      label = klass->make_row (self, profile);
    }
  else
    {
      label = gtk_label_new (hyscan_profile_get_name (profile));
      gtk_widget_set_margin_top (label, 12);
      gtk_widget_set_margin_bottom (label, 12);
    }

  g_object_set_data_full (G_OBJECT (row), HYSCAN_GTK_PROFILE_PROFILE,
                          g_object_ref (profile), g_object_unref);



  gtk_container_add (GTK_CONTAINER (row), label);
  gtk_widget_show_all (row);

  return row;
}

/* Функция обновляет список. */
static void
hyscan_gtk_profile_update_list (HyScanGtkProfile *self)
{
  HyScanGtkProfilePrivate *priv = self->priv;
  GHashTableIter iter;
  gpointer k, v;
  GtkWidget *row;

  gtk_container_foreach (GTK_CONTAINER (priv->list_box), (GtkCallback)gtk_widget_destroy, NULL);

  g_hash_table_iter_init (&iter, self->priv->profiles);
  while (g_hash_table_iter_next (&iter, &k, &v))
    {
      row = hyscan_gtk_profile_make_row (self, HYSCAN_PROFILE (v));
      gtk_list_box_insert (GTK_LIST_BOX (priv->list_box), row, -1);
    }
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
