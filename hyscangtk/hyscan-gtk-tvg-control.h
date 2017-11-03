#ifndef __HYSCAN_GTK_TVG_CONTROL_H__
#define __HYSCAN_GTK_TVG_CONTROL_H__

#include <gtk/gtk.h>
#include <hyscan-tvg-control.h>

G_BEGIN_DECLS

#define HYSCAN_TYPE_GTK_TVG_CONTROL            \
        (hyscan_gtk_tvg_control_get_type ())

#define HYSCAN_GTK_TVG_CONTROL(obj)            \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_TVG_CONTROL, HyScanGtkTVGControl))

#define HYSCAN_IS_GTK_TVG_CONTROL(obj)         \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_TVG_CONTROL))

#define HYSCAN_GTK_TVG_CONTROL_CLASS(klass)    \
        (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_TVG_CONTROL, HyScanGtkTVGControlClass))

#define HYSCAN_IS_GTK_TVG_CONTROL_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_TVG_CONTROL))

#define HYSCAN_GTK_TVG_CONTROL_GET_CLASS(obj)  \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_TVG_CONTROL, HyScanGtkTVGControlClass))

typedef struct _HyScanGtkTVGControl HyScanGtkTVGControl;
typedef struct _HyScanGtkTVGControlPrivate HyScanGtkTVGControlPrivate;
typedef struct _HyScanGtkTVGControlClass HyScanGtkTVGControlClass;

struct _HyScanGtkTVGControl
{
  GtkGrid                       parent_instance;

  HyScanGtkTVGControlPrivate   *priv;
};

struct _HyScanGtkTVGControlClass
{
  GtkGridClass   parent_class;
};

HYSCAN_API
GType               hyscan_gtk_tvg_control_get_type                (void);

HYSCAN_API
GtkWidget*          hyscan_gtk_tvg_control_new                     (HyScanTVGModeType      capabilities);

HYSCAN_API
void                hyscan_gtk_tvg_control_set_mode                (HyScanGtkTVGControl   *tvgc,
                                                                    HyScanTVGModeType      mode);

HYSCAN_API
void                hyscan_gtk_tvg_control_set_gain_range          (HyScanGtkTVGControl   *tvgc,
                                                                    gdouble                gain_min,
                                                                    gdouble                gain_max);

HYSCAN_API
void                hyscan_gtk_tvg_control_set_level               (HyScanGtkTVGControl   *tvgc,
                                                                    gdouble                level);

HYSCAN_API
void                hyscan_gtk_tvg_control_set_sensitivity         (HyScanGtkTVGControl   *tvgc,
                                                                    gdouble                sensitivity);

HYSCAN_API
void                hyscan_gtk_tvg_control_set_linear_db_gain      (HyScanGtkTVGControl   *tvgc,
                                                                    gdouble                gain);

HYSCAN_API
void                hyscan_gtk_tvg_control_set_linear_db_step      (HyScanGtkTVGControl   *tvgc,
                                                                    gdouble                step);

HYSCAN_API
void                hyscan_gtk_tvg_control_set_logarithmic_gain    (HyScanGtkTVGControl   *tvgc,
                                                                    gdouble                gain);

HYSCAN_API
void                hyscan_gtk_tvg_control_set_logarithmic_beta    (HyScanGtkTVGControl   *tvgc,
                                                                    gdouble                beta);

HYSCAN_API
void                hyscan_gtk_tvg_control_set_logarithmic_alpha   (HyScanGtkTVGControl   *tvgc,
                                                                    gdouble                alpha);

HYSCAN_API

HyScanTVGModeType   hyscan_gtk_tvg_control_get_mode                (HyScanGtkTVGControl   *tvgc);

HYSCAN_API
gdouble             hyscan_gtk_tvg_control_get_level               (HyScanGtkTVGControl   *tvgc);

HYSCAN_API
gdouble             hyscan_gtk_tvg_control_get_sensitivity         (HyScanGtkTVGControl   *tvgc);

HYSCAN_API
gdouble             hyscan_gtk_tvg_control_get_linear_db_gain      (HyScanGtkTVGControl   *tvgc);

HYSCAN_API
gdouble             hyscan_gtk_tvg_control_get_linear_db_step      (HyScanGtkTVGControl   *tvgc);

HYSCAN_API
gdouble             hyscan_gtk_tvg_control_get_logarithmic_gain    (HyScanGtkTVGControl   *tvgc);

HYSCAN_API
gdouble             hyscan_gtk_tvg_control_get_logarithmic_beta    (HyScanGtkTVGControl   *tvgc);

HYSCAN_API
gdouble             hyscan_gtk_tvg_control_get_logarithmic_alpha   (HyScanGtkTVGControl   *tvgc);

G_END_DECLS

#endif /* __HYSCAN_GTK_TVG_CONTROL_H__ */