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

#ifndef __HYSCAN_GTK_PROFILE_HW_EDITOR_H__
#define __HYSCAN_GTK_PROFILE_HW_EDITOR_H__

#include <gtk/gtk.h>
#include <hyscan-profile-hw.h>

G_BEGIN_DECLS

#define HYSCAN_TYPE_GTK_PROFILE_HW_EDITOR             (hyscan_gtk_profile_hw_editor_get_type ())
#define HYSCAN_GTK_PROFILE_HW_EDITOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_PROFILE_HW_EDITOR, HyScanGtkProfileHWEditor))
#define HYSCAN_IS_GTK_PROFILE_HW_EDITOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_PROFILE_HW_EDITOR))
#define HYSCAN_GTK_PROFILE_HW_EDITOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_PROFILE_HW_EDITOR, HyScanGtkProfileHWEditorClass))
#define HYSCAN_IS_GTK_PROFILE_HW_EDITOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_PROFILE_HW_EDITOR))
#define HYSCAN_GTK_PROFILE_HW_EDITOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_PROFILE_HW_EDITOR, HyScanGtkProfileHWEditorClass))

typedef struct _HyScanGtkProfileHWEditor HyScanGtkProfileHWEditor;
typedef struct _HyScanGtkProfileHWEditorPrivate HyScanGtkProfileHWEditorPrivate;
typedef struct _HyScanGtkProfileHWEditorClass HyScanGtkProfileHWEditorClass;

struct _HyScanGtkProfileHWEditor
{
  GtkGrid parent_instance;

  HyScanGtkProfileHWEditorPrivate *priv;
};

struct _HyScanGtkProfileHWEditorClass
{
  GtkGridClass parent_class;
};

HYSCAN_API
GType                      hyscan_gtk_profile_hw_editor_get_type         (void);

HYSCAN_API
GtkWidget *                hyscan_gtk_profile_hw_editor_new              (HyScanProfile *profile,
                                                                          gchar        **drivers);

G_END_DECLS

#endif /* __HYSCAN_GTK_PROFILE_HW_EDITOR_H__ */
