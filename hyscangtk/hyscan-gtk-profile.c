/* hyscan-gtk-profile-viewer.h
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

#include <glib/gi18n.h>
#include "hyscan-gtk-profile.h"

/* По умолчанию я буду искать в папке hyscan. Это можно переопределить на
 * этапе компиляции для standalone сборок. */
#ifndef HYSCAN_GTK_PROFILE_PATH
# define HYSCAN_GTK_PROFILE_PATH "hyscan"
#endif

enum
{
  PATH_COL,
  NAME_COL,
  STYLE_COL,
  OBJECT_COL,
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
  PROP_SYSFOLDER,
};

struct _HyScanGtkProfilePrivate
{
  gchar      **folders;

  GHashTable  *profiles; /* {gchar* file_name : HyScanProfile*} */
};

static void    hyscan_gtk_profile_set_property             (GObject               *object,
                                                            guint                  prop_id,
                                                            const GValue          *value,
                                                            GParamSpec            *pspec);
static void    hyscan_gtk_profile_object_constructed       (GObject               *object);
static void    hyscan_gtk_profile_object_finalize          (GObject               *object);
static void    hyscan_gtk_profile_load                     (HyScanGtkProfile      *self,
                                                            const gchar           *dir);

static void    hyscan_gtk_profile_make_model               (HyScanGtkProfile      *self,
                                                            GHashTable            *profiles);
static void    hyscan_gtk_profile_make_tree                (HyScanGtkProfile      *self);
static void    hyscan_gtk_profile_selection_changed        (GtkTreeSelection      *select,
                                                            GtkTreeView           *self);

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

  g_object_class_install_property (oclass, PROP_SYSFOLDER,
    g_param_spec_pointer ("folders", "Folders", "Folders to look for profiles",
                          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  hyscan_gtk_profile_signals[SIGNAL_SELECTED] =
    g_signal_new ("selected", G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST, 0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1, G_TYPE_OBJECT);
}

static void
hyscan_gtk_profile_init (HyScanGtkProfile *gtk_profile)
{
  gtk_profile->priv = hyscan_gtk_profile_get_instance_private (gtk_profile);
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
    case PROP_SYSFOLDER:
      priv->folders = g_strdupv ((gchar**) g_value_get_pointer (value));
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

  priv->profiles = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  /* Профили загружаются из двух каталогов. */
  for (iter = priv->folders; iter != NULL && *iter != NULL; ++iter)
    {
      gchar *folder = g_build_filename (*iter,
                                        HYSCAN_GTK_PROFILE_PATH,
                                        klass->subfolder, NULL);
      hyscan_gtk_profile_load (self, folder);
      g_free (folder);
    }

  klass->make_model (self, priv->profiles);
  klass->make_tree (self);
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

static void
hyscan_gtk_profile_load (HyScanGtkProfile *self,
                         const gchar      *folder)
{
  HyScanGtkProfileClass *klass = HYSCAN_GTK_PROFILE_GET_CLASS (self);
  HyScanGtkProfilePrivate *priv = self->priv;
  const gchar *filename;
  GError *error = NULL;
  GDir *dir;

  if (!g_file_test (folder, G_FILE_TEST_IS_DIR | G_FILE_TEST_EXISTS))
    {
      g_warning ("HyScanGtkProfile: directory %s doesn't exist", folder);
      return;
    }

  dir = g_dir_open (folder, 0, &error);
  if (error != NULL)
    {
      g_warning ("HyScanGtkProfile: %s", error->message);
      g_clear_pointer (&error, g_error_free);
      return;
    }

  while ((filename = g_dir_read_name (dir)) != NULL)
    {
      gchar *fullname;
      HyScanProfile *profile;

      fullname = g_build_filename (folder, filename, NULL);
      profile = klass->new_profile (self, fullname);
      hyscan_profile_read (profile);

      if (profile != NULL)
        g_hash_table_insert (priv->profiles, g_strdup (fullname), profile);

      g_free (fullname);
    }

  g_dir_close (dir);
}

static void
hyscan_gtk_profile_make_model (HyScanGtkProfile *self,
                               GHashTable       *profiles)
{
  GtkTreeSelection *select = NULL;
  GtkListStore *store = NULL;
  GtkTreeIter ls_iter;
  GHashTableIter ht_iter;
  gpointer k, v;

  store = gtk_list_store_new (N_COLUMNS,
                              G_TYPE_STRING,
                              G_TYPE_STRING,
                              G_TYPE_INT,
                              G_TYPE_OBJECT);

  /* Заполняем список параметров. */
  gtk_list_store_append (store, &ls_iter);
  gtk_list_store_set (store, &ls_iter,
                      PATH_COL, NULL,
                      NAME_COL, g_strdup ("Not selected"),
                      STYLE_COL, PANGO_STYLE_ITALIC,
                      OBJECT_COL, NULL,
                      -1);

  g_hash_table_iter_init (&ht_iter, profiles);
  while (g_hash_table_iter_next (&ht_iter, &k, &v))
    {
      const gchar * file = (const gchar *)k;
      HyScanProfile * profile = (HyScanProfile *)v;

      /* Добавляем в деревце. */
      gtk_list_store_append (store, &ls_iter);
      gtk_list_store_set (store, &ls_iter,
                          PATH_COL, g_strdup (file),
                          NAME_COL, g_strdup (hyscan_profile_get_name (profile)),
                          STYLE_COL, PANGO_STYLE_NORMAL,
                          OBJECT_COL, g_object_ref (profile),
                          -1);
    }

  gtk_tree_view_set_model (GTK_TREE_VIEW (self), GTK_TREE_MODEL (store));

  select = gtk_tree_view_get_selection (GTK_TREE_VIEW (self));
  gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
  g_signal_connect (select, "changed", G_CALLBACK (hyscan_gtk_profile_selection_changed), self);
}

static void
hyscan_gtk_profile_make_tree (HyScanGtkProfile *self)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

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
  gtk_tree_view_append_column (GTK_TREE_VIEW (self), column);
}

/* Функция смены страницы. */
static void
hyscan_gtk_profile_selection_changed (GtkTreeSelection *select,
                                      GtkTreeView      *self)
{
  HyScanProfile *profile = NULL;
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (!gtk_tree_selection_get_selected (select, &model, &iter))
    return;

  gtk_tree_model_get (model, &iter, OBJECT_COL, &profile, -1);

  /* Передаем, кто сейчас выбран. */
  g_signal_emit (self, hyscan_gtk_profile_signals[SIGNAL_SELECTED], 0,
                 profile);

  g_clear_object (&profile);
}

GtkWidget *
hyscan_gtk_profile_new (const gchar *subfolder,
                        const gchar *systemfolder,
                        GType        profile_type)
{
  return g_object_new (HYSCAN_TYPE_GTK_PROFILE,
                       "subfolder", subfolder,
                       "sysfolder", systemfolder,
                       "type", profile_type,
                       NULL);
}
