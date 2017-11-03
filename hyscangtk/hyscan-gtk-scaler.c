/**
 * \file hyscan-gtk-scaler.c
 *
 * \brief Исходный файл виджета управления масштабом.
 * \author Vladimir Maximov (vmakxs@gmail.com)
 * \date 2018
 * \license Проприетарная лицензия ООО "Экран"
 *
 */

#include "hyscan-gtk-scaler.h"

enum
{
  SIGNAL_ZOOMED_IN,
  SIGNAL_ZOOMED_OUT,
  SIGNAL_LAST
};

static guint hyscan_gtk_scaler_signals[SIGNAL_LAST] = {0};

struct _HyScanGtkScalerPrivate
{
  GtkWidget   *zoom_in_btn;
  GtkWidget   *zoom_out_btn;
  GtkWidget   *scale_label;
};

static void   hyscan_gtk_scaler_constructed        (GObject           *object);
static void   hyscan_gtk_scaler_finalize           (GObject           *object);

static void   hyscan_gtk_scaler_zoom_in_clicked    (HyScanGtkScaler   *scaler,
                                                    GtkButton         *btn);
static void   hyscan_gtk_scaler_zoom_out_clicked   (HyScanGtkScaler   *scaler,
                                                    GtkButton         *btn);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkScaler, hyscan_gtk_scaler, GTK_TYPE_GRID)

static void
hyscan_gtk_scaler_class_init (HyScanGtkScalerClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  G_OBJECT_CLASS (klass)->constructed = hyscan_gtk_scaler_constructed;
  G_OBJECT_CLASS (klass)->finalize = hyscan_gtk_scaler_finalize;

  hyscan_gtk_scaler_signals[SIGNAL_ZOOMED_IN] =
    g_signal_new ("zoomed-in", G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
  hyscan_gtk_scaler_signals[SIGNAL_ZOOMED_OUT] =
    g_signal_new ("zoomed-out", G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/hyscan/gtk/hyscan-gtk-scaler.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkScaler, scale_label);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkScaler, zoom_in_btn);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkScaler, zoom_out_btn);
  gtk_widget_class_bind_template_callback_full (widget_class, "zoom_in_clicked",
                                                G_CALLBACK (hyscan_gtk_scaler_zoom_in_clicked));
  gtk_widget_class_bind_template_callback_full (widget_class, "zoom_out_clicked",
                                                G_CALLBACK (hyscan_gtk_scaler_zoom_out_clicked));
}

static void
hyscan_gtk_scaler_init (HyScanGtkScaler *scaler)
{
  scaler->priv = hyscan_gtk_scaler_get_instance_private (scaler);
  gtk_widget_init_template (GTK_WIDGET (scaler));
}

static void
hyscan_gtk_scaler_constructed (GObject *object)
{
  G_OBJECT_CLASS (hyscan_gtk_scaler_parent_class)->constructed (object);
}

static void
hyscan_gtk_scaler_finalize (GObject *object)
{
  G_OBJECT_CLASS (hyscan_gtk_scaler_parent_class)->finalize (object);
}

static void
hyscan_gtk_scaler_zoom_in_clicked (HyScanGtkScaler *scaler,
                                   GtkButton       *btn)
{
  g_signal_emit (scaler, hyscan_gtk_scaler_signals[SIGNAL_ZOOMED_IN], 0, NULL);
}

static void
hyscan_gtk_scaler_zoom_out_clicked (HyScanGtkScaler *scaler,
                                    GtkButton       *btn)
{
  g_signal_emit (scaler, hyscan_gtk_scaler_signals[SIGNAL_ZOOMED_OUT], 0, NULL);
}

GtkWidget *
hyscan_gtk_scaler_new (void)
{
  return g_object_new (HYSCAN_TYPE_GTK_SCALER, NULL);
}

void
hyscan_gtk_scaler_set_zoom_in_sensitive (HyScanGtkScaler *scaler,
                                         gboolean         sensitive)
{
  g_return_if_fail (HYSCAN_IS_GTK_SCALER (scaler));

  gtk_widget_set_sensitive (scaler->priv->zoom_in_btn, sensitive);
}

void
hyscan_gtk_scaler_set_zoom_out_sensitive (HyScanGtkScaler *scaler,
                                          gboolean         sensitive)
{
  g_return_if_fail (HYSCAN_IS_GTK_SCALER (scaler));

  gtk_widget_set_sensitive (scaler->priv->zoom_out_btn, sensitive);
}

void
hyscan_gtk_scaler_set_scale_text (HyScanGtkScaler *scaler,
                                  const gchar     *text)
{
  g_return_if_fail (HYSCAN_IS_GTK_SCALER (scaler));

  gtk_label_set_text (GTK_LABEL (scaler->priv->scale_label), text);
}

