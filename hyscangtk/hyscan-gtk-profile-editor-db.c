/* hyscan-gtk-profile-db-editor.h
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
 * SECTION: hyscan-gtk-profile-editor
 * @Title HyScanGtkProfileEditorDB
 * @Short_description
 *
 */

#include "hyscan-gtk-profile-editor-db.h"
#include <string.h>

struct _HyScanGtkProfileEditorDBPrivate
{
  HyScanProfile *profile;

  GtkEntry      *name_entry;

  GtkEntry      *file_chooser;
};

static void    hyscan_gtk_profile_editor_db_object_constructed       (GObject                  *object);
static void    hyscan_gtk_profile_editor_db_object_finalize          (GObject                  *object);
static void    hyscan_gtk_profile_editor_db_name_changed             (HyScanGtkProfileEditorDB *editor);
static void    hyscan_gtk_profile_editor_db_file_path_changed        (HyScanGtkProfileEditorDB *editor);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkProfileEditorDB, hyscan_gtk_profile_editor_db, HYSCAN_TYPE_GTK_PROFILE_EDITOR);

static void
hyscan_gtk_profile_editor_db_class_init (HyScanGtkProfileEditorDBClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  oclass->constructed = hyscan_gtk_profile_editor_db_object_constructed;
  oclass->finalize = hyscan_gtk_profile_editor_db_object_finalize;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/hyscan/gtk/hyscan-gtk-profile-editor-db.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkProfileEditorDB, name_entry);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkProfileEditorDB, file_chooser);
  gtk_widget_class_bind_template_callback_full (widget_class, "file_path_changed",
                                                G_CALLBACK (hyscan_gtk_profile_editor_db_file_path_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "name_changed",
                                                G_CALLBACK (hyscan_gtk_profile_editor_db_name_changed));
}

static void
hyscan_gtk_profile_editor_db_init (HyScanGtkProfileEditorDB *self)
{
  self->priv = hyscan_gtk_profile_editor_db_get_instance_private (self);
  gtk_widget_init_template (GTK_WIDGET (self));
}

static void
hyscan_gtk_profile_editor_db_object_constructed (GObject *object)
{
  HyScanGtkProfileEditorDB *self = HYSCAN_GTK_PROFILE_EDITOR_DB (object);
  HyScanGtkProfileEditorDBPrivate *priv = self->priv;
  const gchar *name, *uri;

  G_OBJECT_CLASS (hyscan_gtk_profile_editor_db_parent_class)->constructed (object);

  priv->profile = hyscan_gtk_profile_editor_get_profile (HYSCAN_GTK_PROFILE_EDITOR (self));

  name = hyscan_profile_get_name (priv->profile);
  uri = hyscan_profile_db_get_uri (HYSCAN_PROFILE_DB (priv->profile));

  if (name != NULL)
    gtk_entry_set_text (priv->name_entry, name);

  if (uri != NULL)
    {
      gchar **split = g_strsplit (uri, "://", 2);

      /* TODO: прочие протоколы подключения к БД.
      if (0 == g_strcmp0 (split[0], "file"))
        ; // id = "file";
      else if (0 == g_strcmp0 (split[0], "tcp"))
        ; // id = "tcp";
      else if (0 == g_strcmp0 (split[0], "shm"))
        ; // id = "shm";
      */

      gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (priv->file_chooser),
                                     split[1]);

      g_strfreev (split);
    }
}

static void
hyscan_gtk_profile_editor_db_object_finalize (GObject *object)
{
  HyScanGtkProfileEditorDB *self = HYSCAN_GTK_PROFILE_EDITOR_DB (object);
  HyScanGtkProfileEditorDBPrivate *priv = self->priv;

  g_clear_object (&priv->profile);

  G_OBJECT_CLASS (hyscan_gtk_profile_editor_db_parent_class)->finalize (object);
}

static void
hyscan_gtk_profile_editor_db_name_changed (HyScanGtkProfileEditorDB *self)
{
  HyScanGtkProfileEditorDBPrivate *priv = self->priv;
  const gchar *name;

  name = gtk_entry_get_text (priv->name_entry);
  hyscan_profile_set_name (priv->profile, name);

  hyscan_gtk_profile_editor_check_sanity (HYSCAN_GTK_PROFILE_EDITOR (self));
}

static void
hyscan_gtk_profile_editor_db_file_path_changed (HyScanGtkProfileEditorDB *self)
{
  HyScanGtkProfileEditorDBPrivate *priv = self->priv;
  gchar *selected, *uri;

  selected = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (priv->file_chooser));
  uri = g_strdup_printf ("file://%s", selected);
  hyscan_profile_db_set_uri (HYSCAN_PROFILE_DB (priv->profile), uri);

  hyscan_gtk_profile_editor_check_sanity (HYSCAN_GTK_PROFILE_EDITOR (self));

  g_free (selected);
  g_free (uri);
}


GtkWidget *
hyscan_gtk_profile_editor_db_new (HyScanProfile *profile)
{
  return g_object_new (HYSCAN_TYPE_GTK_PROFILE_EDITOR_DB,
                       "profile", profile,
                       NULL);
}
