#include "hyscan-gtk-recorder.h"

#define HYSCAN_GTK_RECORDER_TRACK_ID_SURVEY        "survey"
#define HYSCAN_GTK_RECORDER_TRACK_ID_TACK          "tack"
#define HYSCAN_GTK_RECORDER_TRACK_ID_CALIBRATION   "calibration"

enum
{
  SIGNAL_RECORD_STATE_CHANGED,
  SIGNAL_TRACK_TYPE_CHANGED,
  SIGNAL_NEW_TRACK,
  SIGNAL_LAST
};

static guint hyscan_gtk_recorder_signals[SIGNAL_LAST] = {0};

struct _HyScanGtkRecorderPrivate
{
  HyScanTrackType    track_type;
  gboolean           record_state;

  GtkWidget         *track_type_cbt;
  GtkWidget         *record_switch;
  GtkWidget         *new_track_btn;
};

static void              hyscan_gtk_recorder_constructed            (GObject             *object);
static void              hyscan_gtk_recorder_finalize               (GObject             *object);

static void              hyscan_gtk_recorder_new_track_clicked      (HyScanGtkRecorder   *recorder,
                                                                     GtkButton           *btn);
static void              hyscan_gtk_recorder_record_switched        (HyScanGtkRecorder   *recorder,
                                                                     gboolean             state,
                                                                     GtkWidget           *widget);
static void              hyscan_gtk_recorder_track_type_changed     (HyScanGtkRecorder   *recorder,
                                                                     GtkComboBox         *widget);
static gboolean          hyscan_gtk_recorder_track_type_scrolled    (HyScanGtkRecorder   *recorder,
                                                                     GdkEvent            *event,
                                                                     GtkWidget           *combobox);

static const gchar*      hyscan_gtk_recorder_get_track_id_by_type   (HyScanTrackType      track_type);

static HyScanTrackType   hyscan_gtk_recorder_get_track_type_by_id   (const gchar         *track_id);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkRecorder, hyscan_gtk_recorder, GTK_TYPE_GRID)

static void
hyscan_gtk_recorder_class_init (HyScanGtkRecorderClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  G_OBJECT_CLASS (klass)->constructed = hyscan_gtk_recorder_constructed;
  G_OBJECT_CLASS (klass)->finalize = hyscan_gtk_recorder_finalize;

  hyscan_gtk_recorder_signals[SIGNAL_TRACK_TYPE_CHANGED] =
    g_signal_new ("track-type-changed", G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
  hyscan_gtk_recorder_signals[SIGNAL_RECORD_STATE_CHANGED] =
    g_signal_new ("record-state-changed", G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
  hyscan_gtk_recorder_signals[SIGNAL_NEW_TRACK] =
    g_signal_new ("new-track", G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/hyscan/gtk/hyscan-gtk-recorder.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkRecorder, track_type_cbt);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkRecorder, record_switch);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkRecorder, new_track_btn);
  gtk_widget_class_bind_template_callback_full (widget_class, "track_type_changed",
                                                G_CALLBACK (hyscan_gtk_recorder_track_type_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "track_type_scrolled",
                                                G_CALLBACK (hyscan_gtk_recorder_track_type_scrolled));
  gtk_widget_class_bind_template_callback_full (widget_class, "record_switched",
                                                G_CALLBACK (hyscan_gtk_recorder_record_switched));
  gtk_widget_class_bind_template_callback_full (widget_class, "new_track_clicked",
                                                G_CALLBACK (hyscan_gtk_recorder_new_track_clicked));
}

static void
hyscan_gtk_recorder_init (HyScanGtkRecorder *recorder)
{
  recorder->priv = hyscan_gtk_recorder_get_instance_private (recorder);

  gtk_widget_init_template (GTK_WIDGET (recorder));
}

static void
hyscan_gtk_recorder_constructed (GObject *object)
{
  G_OBJECT_CLASS (hyscan_gtk_recorder_parent_class)->constructed (object);

  gtk_widget_set_sensitive (HYSCAN_GTK_RECORDER (object)->priv->new_track_btn, FALSE);
}

static void
hyscan_gtk_recorder_finalize (GObject *object)
{
  G_OBJECT_CLASS (hyscan_gtk_recorder_parent_class)->finalize (object);
}

static void
hyscan_gtk_recorder_new_track_clicked (HyScanGtkRecorder *recorder,
                                       GtkButton         *btn)
{
  if (recorder->priv->record_state)
    g_signal_emit (recorder, hyscan_gtk_recorder_signals[SIGNAL_NEW_TRACK], 0, NULL);
}

static void
hyscan_gtk_recorder_record_switched (HyScanGtkRecorder *recorder,
                                     gboolean           state,
                                     GtkWidget         *widget)
{
  HyScanGtkRecorderPrivate *priv = recorder->priv;

  if (priv->record_state != state)
    {
      priv->record_state = state;
      gtk_widget_set_sensitive (priv->new_track_btn, state);

      g_signal_emit (recorder, hyscan_gtk_recorder_signals[SIGNAL_RECORD_STATE_CHANGED], 0, NULL);
    }
}

static void
hyscan_gtk_recorder_track_type_changed (HyScanGtkRecorder *recorder,
                                        GtkComboBox       *widget)
{
  HyScanGtkRecorderPrivate *priv = recorder->priv;
  const gchar *track_id = gtk_combo_box_get_active_id(GTK_COMBO_BOX (priv->track_type_cbt));
  HyScanTrackType track_type = hyscan_gtk_recorder_get_track_type_by_id (track_id);

  if (priv->track_type != track_type)
  {
    priv->track_type = track_type;

    g_signal_emit (recorder, hyscan_gtk_recorder_signals[SIGNAL_TRACK_TYPE_CHANGED], 0, NULL);
  }
}

static gboolean
hyscan_gtk_recorder_track_type_scrolled (HyScanGtkRecorder *recorder,
                                         GdkEvent          *event,
                                         GtkWidget         *combobox)
{
  return TRUE;
}

static const gchar *
hyscan_gtk_recorder_get_track_id_by_type (HyScanTrackType track_type)
{
  switch (track_type)
   {
   case HYSCAN_TRACK_SURVEY:
     return HYSCAN_GTK_RECORDER_TRACK_ID_SURVEY;
   case HYSCAN_TRACK_TACK:
     return HYSCAN_GTK_RECORDER_TRACK_ID_TACK;
   case HYSCAN_TRACK_CALIBRATION:
     return HYSCAN_GTK_RECORDER_TRACK_ID_CALIBRATION;
   default:
     return NULL;
   }
}

static HyScanTrackType
hyscan_gtk_recorder_get_track_type_by_id (const gchar *track_id)
{
  if (g_str_equal (track_id, HYSCAN_GTK_RECORDER_TRACK_ID_SURVEY))
    return HYSCAN_TRACK_SURVEY;
  if (g_str_equal (track_id, HYSCAN_GTK_RECORDER_TRACK_ID_TACK))
    return HYSCAN_TRACK_TACK;
  if (g_str_equal (track_id, HYSCAN_GTK_RECORDER_TRACK_ID_CALIBRATION))
    return HYSCAN_TRACK_CALIBRATION;
  return HYSCAN_TRACK_UNSPECIFIED;
}

GtkWidget *
hyscan_gtk_recorder_new (void)
{
  return g_object_new (HYSCAN_TYPE_GTK_RECORDER, NULL);
}

void
hyscan_gtk_recorder_set_track_type (HyScanGtkRecorder *recorder,
                                    HyScanTrackType    track_type)
{
  HyScanGtkRecorderPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_RECORDER (recorder));

  if (track_type == HYSCAN_TRACK_UNSPECIFIED)
    return;

  priv = recorder->priv;

  if (priv->track_type != track_type)
    {
      const gchar *track_id;
      priv->track_type = track_type;

      track_id = hyscan_gtk_recorder_get_track_id_by_type (track_type);
      gtk_combo_box_set_active_id(GTK_COMBO_BOX (priv->track_type_cbt), track_id);

      g_signal_emit (recorder, hyscan_gtk_recorder_signals[SIGNAL_TRACK_TYPE_CHANGED], 0, NULL);
    }
}

void
hyscan_gtk_recorder_set_record_state (HyScanGtkRecorder *recorder,
                                      gboolean           enabled)
{
  HyScanGtkRecorderPrivate *priv;
  g_return_if_fail (HYSCAN_IS_GTK_RECORDER (recorder));

  priv = recorder->priv;

  if (priv->record_state != enabled)
    {
      priv->record_state = enabled;

      gtk_switch_set_state (GTK_SWITCH (priv->record_switch), enabled);
      gtk_switch_set_active (GTK_SWITCH (priv->record_switch), enabled);
      gtk_widget_set_sensitive (priv->new_track_btn, enabled);

      g_signal_emit (recorder, hyscan_gtk_recorder_signals[SIGNAL_RECORD_STATE_CHANGED], 0, NULL);
    }
}

HyScanTrackType
hyscan_gtk_recorder_get_track_type (HyScanGtkRecorder *recorder)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_RECORDER (recorder), HYSCAN_TRACK_UNSPECIFIED);

  return recorder->priv->track_type;
}

gboolean
hyscan_gtk_recorder_get_record_state (HyScanGtkRecorder *recorder)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_RECORDER (recorder), FALSE);

  return recorder->priv->record_state;
}
