/**
 * \file hyscan-gtk-scaler.h
 *
 * \brief Заголовочный файл виджета управления масштабом.
 * \author Vladimir Maximov (vmakxs@gmail.com)
 * \date 2018
 * \license Проприетарная лицензия ООО "Экран"
 *
 * \defgroup HyScanGtkScaler HyScanGtkScaler - виджет управления масштабом.
 *
 */

#ifndef __HYSCAN_GTK_SCALER_H__
#define __HYSCAN_GTK_SCALER_H__

#include <gtk/gtk.h>
#include <hyscan-api.h>

G_BEGIN_DECLS

#define HYSCAN_TYPE_GTK_SCALER              \
        (hyscan_gtk_scaler_get_type ())

#define HYSCAN_GTK_SCALER(obj)              \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_SCALER, HyScanGtkScaler))

#define HYSCAN_IS_GTK_SCALER(obj)           \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_SCALER))

#define HYSCAN_GTK_SCALER_CLASS(klass)    \
        (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_SCALER, HyScanGtkScalerClass))

#define HYSCAN_IS_GTK_SCALER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_SCALER))

#define HYSCAN_GTK_SCALER_GET_CLASS(obj)  \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_SCALER, HyScanGtkScalerClass))

typedef struct _HyScanGtkScaler HyScanGtkScaler;
typedef struct _HyScanGtkScalerPrivate HyScanGtkScalerPrivate;
typedef struct _HyScanGtkScalerClass HyScanGtkScalerClass;

struct _HyScanGtkScaler
{
  GtkGrid                   parent_instance;
  HyScanGtkScalerPrivate   *priv;
};

struct _HyScanGtkScalerClass
{
  GtkGridClass   parent_class;
};

HYSCAN_API
GType        hyscan_gtk_scaler_get_type                 (void);

HYSCAN_API
GtkWidget*   hyscan_gtk_scaler_new                      (void);

HYSCAN_API
void         hyscan_gtk_scaler_set_zoom_in_sensitive    (HyScanGtkScaler   *scaler,
                                                         gboolean           sensitive);

HYSCAN_API
void         hyscan_gtk_scaler_set_zoom_out_sensitive   (HyScanGtkScaler   *scaler,
                                                         gboolean           sensitive);

HYSCAN_API
void         hyscan_gtk_scaler_set_scale_text           (HyScanGtkScaler   *scaler,
                                                         const gchar       *text);

G_END_DECLS

#endif /* __HYSCAN_GTK_SCALER_H__ */
