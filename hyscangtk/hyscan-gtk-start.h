/* hyscan-gtk-start.h
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

#ifndef __HYSCAN_GTK_START_H__
#define __HYSCAN_GTK_START_H__

#include <gtk/gtk.h>
#include <hyscan-api.h>
#include <hyscan-db.h>
#include <hyscan-control.h>

G_BEGIN_DECLS

#define HYSCAN_TYPE_GTK_START             (hyscan_gtk_start_get_type ())
#define HYSCAN_GTK_START(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_START, HyScanGtkStart))
#define HYSCAN_IS_GTK_START(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_START))
#define HYSCAN_GTK_START_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_START, HyScanGtkStartClass))
#define HYSCAN_IS_GTK_START_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_START))
#define HYSCAN_GTK_START_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_START, HyScanGtkStartClass))

typedef struct _HyScanGtkStart HyScanGtkStart;
typedef struct _HyScanGtkStartPrivate HyScanGtkStartPrivate;
typedef struct _HyScanGtkStartClass HyScanGtkStartClass;

struct _HyScanGtkStart
{
  GtkWindow parent_instance;

  HyScanGtkStartPrivate *priv;
};

struct _HyScanGtkStartClass
{
  GtkWindowClass parent_class;
};

HYSCAN_API
GType           hyscan_gtk_start_get_type         (void);

HYSCAN_API
GtkWidget *     hyscan_gtk_start_new              (void);

G_END_DECLS

#endif /* __HYSCAN_GTK_START_H__ */
