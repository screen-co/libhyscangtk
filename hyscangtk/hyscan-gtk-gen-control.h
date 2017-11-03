/**
 * \file hyscan-gtk-gen-control.h
 *
 * \brief Заголовочный файл виджета управления генератором.
 * \author Vladimir Maximov (vmakxs@gmail.com)
 * \date 2018
 * \license Проприетарная лицензия ООО "Экран"
 *
 * \defgroup HyScanGtkGenControl HyScanGtkGenControl - виджет управления генератором.
 *
 */

#ifndef __HYSCAN_GTK_GEN_CONTROL_H__
#define __HYSCAN_GTK_GEN_CONTROL_H__

#include <gtk/gtk.h>
#include <hyscan-generator-control.h>

G_BEGIN_DECLS

typedef struct {
  HyScanGeneratorSignalType  signal;
  gdouble                    duration_min;
  gdouble                    duration_max;
} HyScanSignalDurationSpec;

#define HYSCAN_TYPE_GTK_GEN_CONTROL            \
        (hyscan_gtk_gen_control_get_type ())

#define HYSCAN_GTK_GEN_CONTROL(obj)            \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_GEN_CONTROL, HyScanGtkGenControl))

#define HYSCAN_IS_GTK_GEN_CONTROL(obj)         \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_GEN_CONTROL))

#define HYSCAN_GTK_GEN_CONTROL_CLASS(klass)    \
        (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_GEN_CONTROL, HyScanGtkGenControlClass))

#define HYSCAN_IS_GTK_GEN_CONTROL_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_GEN_CONTROL))

#define HYSCAN_GTK_GEN_CONTROL_GET_CLASS(obj)  \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_GEN_CONTROL, HyScanGtkGenControlClass))

typedef struct _HyScanGtkGenControl HyScanGtkGenControl;
typedef struct _HyScanGtkGenControlPrivate HyScanGtkGenControlPrivate;
typedef struct _HyScanGtkGenControlClass HyScanGtkGenControlClass;

struct _HyScanGtkGenControl
{
  GtkGrid                       parent_instance;

  HyScanGtkGenControlPrivate   *priv;
};

struct _HyScanGtkGenControlClass
{
  GtkGridClass    parent_class;
};

HYSCAN_API
GType        hyscan_gtk_gen_control_get_type             (void);

HYSCAN_API
GtkWidget*   hyscan_gtk_gen_control_new                  (HyScanGeneratorModeType       capabilities,
                                                          HyScanGeneratorSignalType     singals,
                                                          HyScanDataSchemaEnumValue   **presets);

HYSCAN_API
void         hyscan_gtk_gen_control_set_durations        (HyScanGtkGenControl          *genc,
                                                          GList                        *durations);

HYSCAN_API
void         hyscan_gtk_gen_control_set_mode             (HyScanGtkGenControl          *genc,
                                                          HyScanGeneratorModeType       mode);

HYSCAN_API
void         hyscan_gtk_gen_control_set_preset           (HyScanGtkGenControl          *genc,
                                                          guint                         preset);

HYSCAN_API
void         hyscan_gtk_gen_control_set_signal           (HyScanGtkGenControl          *genc,
                                                          HyScanGeneratorSignalType     signal_type);

HYSCAN_API
void         hyscan_gtk_gen_control_set_power            (HyScanGtkGenControl          *genc,
                                                          gdouble                       power);

HYSCAN_API
void         hyscan_gtk_gen_control_set_duration         (HyScanGtkGenControl          *genc,
                                                          gdouble                       duration);

HYSCAN_API
HyScanGeneratorModeType
             hyscan_gtk_gen_control_get_mode             (HyScanGtkGenControl          *genc);

HYSCAN_API
gint64       hyscan_gtk_gen_control_get_preset           (HyScanGtkGenControl          *genc);

HYSCAN_API
HyScanGeneratorSignalType
             hyscan_gtk_gen_control_get_signal           (HyScanGtkGenControl          *genc);

HYSCAN_API
gdouble      hyscan_gtk_gen_control_get_power            (HyScanGtkGenControl          *genc);

HYSCAN_API
gdouble      hyscan_gtk_gen_control_get_duration         (HyScanGtkGenControl          *genc);

G_END_DECLS

#endif /* __HYSCAN_GTK_GEN_CONTROL_H__ */
