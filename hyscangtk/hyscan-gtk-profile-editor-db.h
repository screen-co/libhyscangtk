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

#ifndef __HYSCAN_GTK_PROFILE_EDITOR_DB_H__
#define __HYSCAN_GTK_PROFILE_EDITOR_DB_H__

#include <hyscan-gtk-profile-editor.h>
#include <hyscan-profile-db.h>

G_BEGIN_DECLS

#define HYSCAN_TYPE_GTK_PROFILE_EDITOR_DB             (hyscan_gtk_profile_editor_db_get_type ())
#define HYSCAN_GTK_PROFILE_EDITOR_DB(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_PROFILE_EDITOR_DB, HyScanGtkProfileEditorDB))
#define HYSCAN_IS_GTK_PROFILE_EDITOR_DB(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_PROFILE_EDITOR_DB))
#define HYSCAN_GTK_PROFILE_EDITOR_DB_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_PROFILE_EDITOR_DB, HyScanGtkProfileEditorDBClass))
#define HYSCAN_IS_GTK_PROFILE_EDITOR_DB_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_PROFILE_EDITOR_DB))
#define HYSCAN_GTK_PROFILE_EDITOR_DB_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_PROFILE_EDITOR_DB, HyScanGtkProfileEditorDBClass))

typedef struct _HyScanGtkProfileEditorDB HyScanGtkProfileEditorDB;
typedef struct _HyScanGtkProfileEditorDBPrivate HyScanGtkProfileEditorDBPrivate;
typedef struct _HyScanGtkProfileEditorDBClass HyScanGtkProfileEditorDBClass;

struct _HyScanGtkProfileEditorDB
{
  HyScanGtkProfileEditor parent_instance;

  HyScanGtkProfileEditorDBPrivate *priv;
};

struct _HyScanGtkProfileEditorDBClass
{
  HyScanGtkProfileEditorClass parent_class;
};

HYSCAN_API
GType        hyscan_gtk_profile_editor_db_get_type   (void);

HYSCAN_API
GtkWidget *  hyscan_gtk_profile_editor_db_new        (HyScanProfile *profile);

G_END_DECLS

#endif /* __HYSCAN_GTK_PROFILE_EDITOR_DB_H__ */
