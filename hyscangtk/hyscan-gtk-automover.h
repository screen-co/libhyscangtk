#ifndef __HYSCAN_GTK_AUTOMOVER_H__
#define __HYSCAN_GTK_AUTOMOVER_H__

#include <gtk/gtk.h>
#include <hyscan-api.h>

G_BEGIN_DECLS

#define HYSCAN_TYPE_GTK_AUTOMOVER            \
        (hyscan_gtk_automover_get_type ())

#define HYSCAN_GTK_AUTOMOVER(obj)            \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_AUTOMOVER, HyScanGtkAutomover))

#define HYSCAN_IS_GTK_AUTOMOVER(obj)         \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_AUTOMOVER))

#define HYSCAN_GTK_AUTOMOVER_CLASS(klass)    \
        (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_AUTOMOVER, HyScanGtkAutomoverClass))

#define HYSCAN_IS_GTK_AUTOMOVER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_AUTOMOVER))

#define HYSCAN_GTK_AUTOMOVER_GET_CLASS(obj)  \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_AUTOMOVER, HyScanGtkAutomoverClass))

typedef struct _HyScanGtkAutomover HyScanGtkAutomover;
typedef struct _HyScanGtkAutomoverPrivate HyScanGtkAutomoverPrivate;
typedef struct _HyScanGtkAutomoverClass HyScanGtkAutomoverClass;

struct _HyScanGtkAutomover
{
  GtkGrid                      parent_instance;
  HyScanGtkAutomoverPrivate   *priv;
};

struct _HyScanGtkAutomoverClass
{
  GtkGridClass   parent_class;
};

HYSCAN_API
GType        hyscan_gtk_automover_get_type      (void);

HYSCAN_API
GtkWidget*   hyscan_gtk_automover_new           (void);
 
HYSCAN_API
void         hyscan_gtk_automover_set_enabled   (HyScanGtkAutomover   *automover,
                                                 gboolean              enabled);

HYSCAN_API
gboolean     hyscan_gtk_automover_is_enabled    (HyScanGtkAutomover   *automover);

G_END_DECLS

#endif /* __HYSCAN_GTK_AUTOMOVER_H__ */
