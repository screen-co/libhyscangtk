/* hyscan-gtk-profile-db.h
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
 * SECTION: hyscan-gtk-profile-db
 * @Title: HyScanGtkProfileDB
 * @Short_description: Виджет профиля баз данных.
 */

#include "hyscan-gtk-profile-db.h"
#include <hyscan-gtk-profile-db-editor.h>


#define HYSCAN_GTK_PROFILE_DB_PATH "db-profiles"

static HyScanProfile * hyscan_gtk_profile_db_new_profile (HyScanGtkProfile *parent,
                                                          const gchar      *filename);
static GtkWidget *     hyscan_gtk_profile_db_editor      (HyScanGtkProfile *parent,
                                                          HyScanProfile    *profile);


G_DEFINE_TYPE (HyScanGtkProfileDB, hyscan_gtk_profile_db, HYSCAN_TYPE_GTK_PROFILE);

static void
hyscan_gtk_profile_db_class_init (HyScanGtkProfileDBClass *klass)
{
  HyScanGtkProfileClass *pklass = HYSCAN_GTK_PROFILE_CLASS (klass);

  pklass->subfolder = HYSCAN_GTK_PROFILE_DB_PATH;
  pklass->new_profile = hyscan_gtk_profile_db_new_profile;
  pklass->make_editor = hyscan_gtk_profile_db_editor;
}

static void
hyscan_gtk_profile_db_init (HyScanGtkProfileDB *gtk_profile_db)
{
}

static HyScanProfile *
hyscan_gtk_profile_db_new_profile (HyScanGtkProfile *parent,
                                   const gchar      *filename)
{
  return HYSCAN_PROFILE (hyscan_profile_db_new (filename));
}

static GtkWidget *
hyscan_gtk_profile_db_editor (HyScanGtkProfile *parent,
                              HyScanProfile    *profile)
{
  return hyscan_gtk_profile_db_editor_new (profile);
}
/*
 * hyscan_gtk_profile_db_new:
 * @sys: путь к системной папке профилей
 *
 * Returns: (transfer full) виджет профилей БД.
 */
GtkWidget *
hyscan_gtk_profile_db_new (gchar    **folders,
                           gboolean   readonly)
{
  return g_object_new (HYSCAN_TYPE_GTK_PROFILE_DB,
                       "folders", folders,
                       "readonly", readonly,
                       NULL);
}
