/**
 * \file hyscan-gtk-pane.h
 *
 * \brief Заголовочный файл виджета панели.
 * \author Vladimir Maximov (vmakxs@gmail.com)
 * \date 2018
 * \license Проприетарная лицензия ООО "Экран"
 *
 * \defgroup HyScanGtkPane HyScanGtkPane - виджет панель
 *
 */

#ifndef __HYSCAN_GTK_PANE_H__
#define __HYSCAN_GTK_PANE_H__

#include <gtk/gtk.h>
#include <hyscan-api.h>

G_BEGIN_DECLS

typedef enum
{
  HYSCAN_GTK_PANE_TEXT = 1,
  HYSCAN_GTK_PANE_ARROW
}
HyScanGtkPaneStyle;

#define HYSCAN_TYPE_GTK_PANE_STYLE \
        (hyscan_gtk_pane_style_get_type ())

#define HYSCAN_TYPE_GTK_PANE       \
        hyscan_gtk_pane_get_type ()

#define HYSCAN_GTK_PANE(obj)       \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_PANE, HyScanGtkPane))

#define HYSCAN_IS_GTK_PANE(obj)    \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_PANE))

typedef struct _HyScanGtkPane HyScanGtkPane;
typedef struct _HyScanGtkPaneClass HyScanGtkPaneClass;

struct _HyScanGtkPaneClass
{
  GtkGridClass parent_class;
};

HYSCAN_API
GType       hyscan_gtk_pane_style_get_type  (void);

HYSCAN_API
GType       hyscan_gtk_pane_get_type        (void);

HYSCAN_API
GtkWidget*  hyscan_gtk_pane_new             (const gchar         *title,
                                             GtkWidget           *body,
                                             gboolean             bold,
                                             HyScanGtkPaneStyle   style);

HYSCAN_API
void        hyscan_gtk_pane_set_expanded    (HyScanGtkPane       *pane,
                                             gboolean             expanded);

G_END_DECLS

#endif /* __HYSCAN_GTK_PANE_H__ */
