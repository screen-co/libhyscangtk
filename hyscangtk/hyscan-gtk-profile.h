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

#ifndef __HYSCAN_GTK_PROFILE_H__
#define __HYSCAN_GTK_PROFILE_H__

#include <gtk/gtk.h>
#include <hyscan-api.h>
#include <hyscan-profile.h>

G_BEGIN_DECLS

#define HYSCAN_TYPE_GTK_PROFILE             (hyscan_gtk_profile_get_type ())
#define HYSCAN_GTK_PROFILE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_PROFILE, HyScanGtkProfile))
#define HYSCAN_IS_GTK_PROFILE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_PROFILE))
#define HYSCAN_GTK_PROFILE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_PROFILE, HyScanGtkProfileClass))
#define HYSCAN_IS_GTK_PROFILE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_PROFILE))
#define HYSCAN_GTK_PROFILE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_PROFILE, HyScanGtkProfileClass))

typedef struct _HyScanGtkProfile HyScanGtkProfile;
typedef struct _HyScanGtkProfilePrivate HyScanGtkProfilePrivate;
typedef struct _HyScanGtkProfileClass HyScanGtkProfileClass;

struct _HyScanGtkProfile
{
  GtkTreeView parent_instance;

  HyScanGtkProfilePrivate *priv;
};

/**
 * HyScanGtkProfileClass:
 * @g_iface: Базовый интерфейс.
 * @subfolder: папка, в которой лежат профили.
 * @new_profile: фабричный метод создания новых объектов профилей.
 * @make_model: метод создания модели для виджета.
 * @make_tree: метод создания #GtkTreeView с профилями.
 */
struct _HyScanGtkProfileClass
{
  GtkTreeViewClass parent_class;

  const gchar      *subfolder;

  HyScanProfile * (*new_profile) (HyScanGtkProfile *parent,
                                  const gchar      *filename);

  void            (*make_model)  (HyScanGtkProfile *self,
                                  GHashTable       *profiles);
  void            (*make_tree)   (HyScanGtkProfile *self);
  void            (*update_tree) (HyScanGtkProfile *self);

  GtkWidget *     (*make_editor) (HyScanGtkProfile *self,
                                  HyScanProfile    *profile);
};

HYSCAN_API
GType                  hyscan_gtk_profile_get_type         (void);

G_END_DECLS

#endif /* __HYSCAN_GTK_PROFILE_H__ */
