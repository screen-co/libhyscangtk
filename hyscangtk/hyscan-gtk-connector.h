/* hyscan-gtk-connector.h
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

#ifndef __HYSCAN_GTK_CONNECTOR_H__
#define __HYSCAN_GTK_CONNECTOR_H__

#include <gtk/gtk.h>
#include <hyscan-api.h>
#include <hyscan-db.h>
#include <hyscan-control.h>

G_BEGIN_DECLS

#define HYSCAN_TYPE_GTK_CONNECTOR             (hyscan_gtk_connector_get_type ())
#define HYSCAN_GTK_CONNECTOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_CONNECTOR, HyScanGtkConnector))
#define HYSCAN_IS_GTK_CONNECTOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_CONNECTOR))
#define HYSCAN_GTK_CONNECTOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_CONNECTOR, HyScanGtkConnectorClass))
#define HYSCAN_IS_GTK_CONNECTOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_CONNECTOR))
#define HYSCAN_GTK_CONNECTOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_CONNECTOR, HyScanGtkConnectorClass))

typedef struct _HyScanGtkConnector HyScanGtkConnector;
typedef struct _HyScanGtkConnectorPrivate HyScanGtkConnectorPrivate;
typedef struct _HyScanGtkConnectorClass HyScanGtkConnectorClass;

struct _HyScanGtkConnector
{
  GtkAssistant parent_instance;

  HyScanGtkConnectorPrivate *priv;
};

struct _HyScanGtkConnectorClass
{
  GtkAssistantClass parent_class;
};

HYSCAN_API
GType                  hyscan_gtk_connector_get_type         (void);

HYSCAN_API
GtkWidget *            hyscan_gtk_connector_new              (const gchar **folders,
                                                              gchar       **drivers);

HYSCAN_API
gboolean               hyscan_gtk_connector_get_result       (HyScanGtkConnector *self);

HYSCAN_API
HyScanDB *             hyscan_gtk_connector_get_db           (HyScanGtkConnector *self);

HYSCAN_API
HyScanControl *        hyscan_gtk_connector_get_control      (HyScanGtkConnector *self);

HYSCAN_API
const gchar *          hyscan_gtk_connector_get_db_name      (HyScanGtkConnector *self);

HYSCAN_API
const gchar *          hyscan_gtk_connector_get_hw_name      (HyScanGtkConnector *self);

HYSCAN_API
const gchar *          hyscan_gtk_connector_get_offset_name  (HyScanGtkConnector *self);

G_END_DECLS

#endif /* __HYSCAN_GTK_CONNECTOR_H__ */
