/**
 * \file hyscan-gtk-colorizer.h
 *
 * \brief Заголовочный файл виджета управления цветовыми схемами.
 * \author Vladimir Maximov (vmakxs@gmail.com)
 * \date 2018
 * \license Проприетарная лицензия ООО "Экран"
 *
 * \defgroup HyScanGtkColorizer HyScanGtkColorizer - управление цветовыми схемами.
 *
 */

#ifndef __HYSCAN_GTK_COLORIZER_H__
#define __HYSCAN_GTK_COLORIZER_H__

#include <gtk/gtk.h>
#include <hyscan-api.h>
#include "hyscan-colormap-spec.h"

G_BEGIN_DECLS

#define HYSCAN_TYPE_GTK_COLORIZER            \
        (hyscan_gtk_colorizer_get_type ())

#define HYSCAN_GTK_COLORIZER(obj)            \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_COLORIZER, HyScanGtkColorizer))

#define HYSCAN_IS_GTK_COLORIZER(obj)         \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_COLORIZER))

#define HYSCAN_GTK_COLORIZER_CLASS(klass)    \
        (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_COLORIZER, HyScanGtkColorizerClass))

#define HYSCAN_IS_GTK_COLORIZER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_COLORIZER))

#define HYSCAN_GTK_COLORIZER_GET_CLASS(obj)  \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_COLORIZER, HyScanGtkColorizerClass))

typedef struct _HyScanGtkColorizer HyScanGtkColorizer;
typedef struct _HyScanGtkColorizerPrivate HyScanGtkColorizerPrivate;
typedef struct _HyScanGtkColorizerClass HyScanGtkColorizerClass;

struct _HyScanGtkColorizer
{
  GtkGrid                      parent_instance;
  HyScanGtkColorizerPrivate   *priv;
};

struct _HyScanGtkColorizerClass
{
  GtkGridClass   parent_class;
};

HYSCAN_API
GType          hyscan_gtk_colorizer_get_type        (void);

HYSCAN_API
GtkWidget*     hyscan_gtk_colorizer_new             (void);

HYSCAN_API
const gchar*   hyscan_gtk_colorizer_get_colormap    (HyScanGtkColorizer   *colorizer);

HYSCAN_API
void           hyscan_gtk_colorizer_set_colormap    (HyScanGtkColorizer   *colorize,
                                                     const gchar          *colormap_id);

HYSCAN_API
void           hyscan_gtk_colorizer_set_colormaps   (HyScanGtkColorizer   *colorize,
                                                     HyScanColormapSpec   *colormaps,
                                                     gsize                 length);

G_END_DECLS

#endif /* __HYSCAN_GTK_COLORIZER_H__ */
