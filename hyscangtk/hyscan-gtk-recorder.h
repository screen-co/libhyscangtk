#ifndef __HYSCAN_GTK_RECORDER_H__
#define __HYSCAN_GTK_RECORDER_H__

#include <gtk/gtk.h>
#include <hyscan-types.h>

G_BEGIN_DECLS

#define HYSCAN_TYPE_GTK_RECORDER            \
        (hyscan_gtk_recorder_get_type ())

#define HYSCAN_GTK_RECORDER(obj)            \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_RECORDER, HyScanGtkRecorder))

#define HYSCAN_IS_GTK_RECORDER(obj)         \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_RECORDER))

#define HYSCAN_GTK_RECORDER_CLASS(klass)    \
        (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_RECORDER, HyScanGtkRecorderClass))

#define HYSCAN_IS_GTK_RECORDER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_RECORDER))

#define HYSCAN_GTK_RECORDER_GET_CLASS(obj)  \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_RECORDER, HyScanGtkRecorderClass))

typedef struct _HyScanGtkRecorder HyScanGtkRecorder;
typedef struct _HyScanGtkRecorderPrivate HyScanGtkRecorderPrivate;
typedef struct _HyScanGtkRecorderClass HyScanGtkRecorderClass;

struct _HyScanGtkRecorder
{
  GtkGrid                     parent_instance;
  HyScanGtkRecorderPrivate   *priv;
};

struct _HyScanGtkRecorderClass
{
  GtkGridClass   parent_class;
};

HYSCAN_API
GType             hyscan_gtk_recorder_get_type           (void);

HYSCAN_API
GtkWidget*        hyscan_gtk_recorder_new                (void);

HYSCAN_API
void              hyscan_gtk_recorder_set_track_type     (HyScanGtkRecorder   *recorder,
                                                          HyScanTrackType      track_type);

HYSCAN_API
void              hyscan_gtk_recorder_set_record_state   (HyScanGtkRecorder   *recorder,
                                                          gboolean             enabled);

HYSCAN_API
HyScanTrackType   hyscan_gtk_recorder_get_track_type     (HyScanGtkRecorder   *recorder);

HYSCAN_API
gboolean          hyscan_gtk_recorder_get_record_state   (HyScanGtkRecorder   *recorder);

G_END_DECLS

#endif /* __HYSCAN_GTK_RECORDER_H__ */
