#ifndef __HYSCAN_GTK_LEVELLER_H__
#define __HYSCAN_GTK_LEVELLER_H__

#include <gtk/gtk.h>
#include <hyscan-api.h>

G_BEGIN_DECLS


#define HYSCAN_TYPE_GTK_LEVELLER            \
        (hyscan_gtk_leveller_get_type ())

#define HYSCAN_GTK_LEVELLER(obj)            \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_LEVELLER, HyScanGtkLeveller))

#define HYSCAN_IS_GTK_LEVELLER(obj)         \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_LEVELLER))

#define HYSCAN_GTK_LEVELLER_CLASS(klass)    \
        (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_LEVELLER, HyScanGtkLevellerClass))

#define HYSCAN_IS_GTK_LEVELLER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_LEVELLER))

#define HYSCAN_GTK_LEVELLER_GET_CLASS(obj)  \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_LEVELLER, HyScanGtkLevellerClass))

typedef struct _HyScanGtkLeveller HyScanGtkLeveller;
typedef struct _HyScanGtkLevellerPrivate HyScanGtkLevellerPrivate;
typedef struct _HyScanGtkLevellerClass HyScanGtkLevellerClass;

struct _HyScanGtkLeveller
{
  GtkGrid                     parent_instance;
  HyScanGtkLevellerPrivate   *priv;
};

struct _HyScanGtkLevellerClass
{
  GtkGridClass parent_class;
};

HYSCAN_API
GType        hyscan_gtk_leveller_get_type          (void);

HYSCAN_API
GtkWidget*   hyscan_gtk_leveller_new               (void);

HYSCAN_API
void         hyscan_gtk_leveller_set_white_range   (HyScanGtkLeveller   *leveller,
                                                    gdouble              lower,
                                                    gdouble              upper);

HYSCAN_API
void         hyscan_gtk_leveller_set_gamma_range   (HyScanGtkLeveller   *leveller,
                                                    gdouble              lower,
                                                    gdouble              upper);

HYSCAN_API
void         hyscan_gtk_leveller_set_black_range   (HyScanGtkLeveller   *leveller,
                                                    gdouble              lower,
                                                    gdouble              upper);

HYSCAN_API
void         hyscan_gtk_leveller_set_levels        (HyScanGtkLeveller   *leveller,
                                                    gdouble              black,
                                                    gdouble              gamma,
                                                    gdouble              white);

HYSCAN_API
void         hyscan_gtk_leveller_get_levels        (HyScanGtkLeveller   *leveller,
                                                    gdouble             *black,
                                                    gdouble             *gamma,
                                                    gdouble             *white);

G_END_DECLS

#endif /* __HYSCAN_GTK_LEVELLER_H__ */
