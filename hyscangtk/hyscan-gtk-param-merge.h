/* hyscan-gtk-param-merge.h
 *
 * Copyright 2018 Screen LLC, Alexander Dmitriev <m1n7@yandex.ru>
 * Copyright 2020 Screen LLC, Alexey Sakhnov <alexsakhnov@gmail.com>
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

#ifndef __HYSCAN_GTK_PARAM_MERGE_H__
#define __HYSCAN_GTK_PARAM_MERGE_H__

#include <hyscan-gtk-param.h>
#include <hyscan-param-merge.h>

G_BEGIN_DECLS

#define HYSCAN_TYPE_GTK_PARAM_MERGE             (hyscan_gtk_param_merge_get_type ())
#define HYSCAN_GTK_PARAM_MERGE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_PARAM_MERGE, HyScanGtkParamMerge))
#define HYSCAN_IS_GTK_PARAM_MERGE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_PARAM_MERGE))
#define HYSCAN_GTK_PARAM_MERGE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_PARAM_MERGE, HyScanGtkParamMergeClass))
#define HYSCAN_IS_GTK_PARAM_MERGE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_PARAM_MERGE))
#define HYSCAN_GTK_PARAM_MERGE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_PARAM_MERGE, HyScanGtkParamMergeClass))

typedef struct _HyScanGtkParamMerge HyScanGtkParamMerge;
typedef struct _HyScanGtkParamMergePrivate HyScanGtkParamMergePrivate;
typedef struct _HyScanGtkParamMergeClass HyScanGtkParamMergeClass;

struct _HyScanGtkParamMerge
{
  HyScanGtkParam parent_instance;

  HyScanGtkParamMergePrivate *priv;
};

struct _HyScanGtkParamMergeClass
{
  HyScanGtkParamClass parent_class;
};

HYSCAN_API
GType                  hyscan_gtk_param_merge_get_type         (void);

HYSCAN_API
GtkWidget *            hyscan_gtk_param_merge_new_full         (HyScanParamMerge *param,
                                                                const gchar      *root,
                                                                gboolean          show_hidden);


G_END_DECLS

#endif /* __HYSCAN_GTK_PARAM_MERGE_H__ */
