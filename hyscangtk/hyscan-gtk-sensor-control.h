/**
 * \file hyscan-gtk-sensor-control.h
 *
 * \brief Заголовочный файл виджета управления датчиками.
 * \author Vladimir Maximov (vmakxs@gmail.com)
 * \date 2018
 * \license Проприетарная лицензия ООО "Экран"
 *
 * \defgroup HyScanGtkSensorControl HyScanGtkSensorControl - виджет управление датчиками.
 *
 */

#ifndef __HYSCAN_GTK_SENSOR_CONTROL_H__
#define __HYSCAN_GTK_SENSOR_CONTROL_H__

#include <gtk/gtk.h>
#include <hyscan-api.h>
#include <hyscan-sensor.h>

G_BEGIN_DECLS

#define HYSCAN_TYPE_GTK_SENSOR_CONTROL            \
        (hyscan_gtk_sensor_control_get_type ())
#define HYSCAN_GTK_SENSOR_CONTROL(obj)            \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_SENSOR_CONTROL, HyScanGtkSensorControl))
#define HYSCAN_IS_GTK_SENSOR_CONTROL(obj)         \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_SENSOR_CONTROL))
#define HYSCAN_GTK_SENSOR_CONTROL_CLASS(klass)    \
        (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_SENSOR_CONTROL, HyScanGtkSensorControlClass))
#define HYSCAN_IS_GTK_SENSOR_CONTROL_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_SENSOR_CONTROL))
#define HYSCAN_GTK_SENSOR_CONTROL_GET_CLASS(obj)  \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_SENSOR_CONTROL, HyScanGtkSensorControlClass))

typedef struct _HyScanGtkSensorControl HyScanGtkSensorControl;
typedef struct _HyScanGtkSensorControlPrivate HyScanGtkSensorControlPrivate;
typedef struct _HyScanGtkSensorControlClass HyScanGtkSensorControlClass;

struct _HyScanGtkSensorControl
{
  GtkGrid                        parent_instance;
  HyScanGtkSensorControlPrivate *priv;
};

struct _HyScanGtkSensorControlClass
{
  GtkGridClass parent_class;
};

HYSCAN_API
GType         hyscan_gtk_sensor_control_get_type            (void);

/* Создаёт виджет управления датчиками. */
HYSCAN_API
GtkWidget*    hyscan_gtk_sensor_control_new                 (void);

/* Получает датчики. Работать с хэш-таблицей требуется через hyscan-sensor-data. */
HYSCAN_API
GHashTable*   hyscan_gtk_sensor_control_get_sensors         (HyScanGtkSensorControl    *gtk_sensor_control);

/* Обновляет состояние виджета — подтягивает внешние изменения из хэш-таблицы датчиков. */
HYSCAN_API
void          hyscan_gtk_sensor_control_update              (HyScanGtkSensorControl    *gtk_sensor_control);

G_END_DECLS

#endif /* __HYSCAN_GTK_SENSOR_CONTROL_H__ */
