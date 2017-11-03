#include "hyscan-gtk-automover.h"

enum
{
  SIGNAL_AUTOMOVE_CHANGED,
  SIGNAL_LAST
};

static guint hyscan_gtk_automover_signals[SIGNAL_LAST] = {0};

struct _HyScanGtkAutomoverPrivate
{
  GtkWidget   *automove_cb;
  gboolean     enabled;
};

static void   hyscan_gtk_automover_constructed   (GObject              *object);
static void   hyscan_gtk_automover_finalize      (GObject              *object);
static void   hyscan_gtk_automover_toggled       (HyScanGtkAutomover   *automover,
                                                  GtkCheckButton       *cb);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkAutomover, hyscan_gtk_automover, GTK_TYPE_GRID)

static void
hyscan_gtk_automover_class_init (HyScanGtkAutomoverClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  G_OBJECT_CLASS (klass)->constructed = hyscan_gtk_automover_constructed;
  G_OBJECT_CLASS (klass)->finalize = hyscan_gtk_automover_finalize;

  hyscan_gtk_automover_signals[SIGNAL_AUTOMOVE_CHANGED] =
    g_signal_new ("automove-changed", G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, 
                  G_TYPE_NONE, 0);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/hyscan/gtk/hyscan-gtk-automover.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkAutomover, automove_cb);
  gtk_widget_class_bind_template_callback_full (widget_class, "automove_toggled",
                                                G_CALLBACK (hyscan_gtk_automover_toggled));
}

static void
hyscan_gtk_automover_init (HyScanGtkAutomover *automover)
{
  automover->priv = hyscan_gtk_automover_get_instance_private (automover);
  gtk_widget_init_template (GTK_WIDGET (automover));
}

static void
hyscan_gtk_automover_constructed (GObject *object)
{
  G_OBJECT_CLASS (hyscan_gtk_automover_parent_class)->constructed (object);
}

static void
hyscan_gtk_automover_finalize (GObject *object)
{
  G_OBJECT_CLASS (hyscan_gtk_automover_parent_class)->finalize (object);
}

static void
hyscan_gtk_automover_toggled (HyScanGtkAutomover *automover,
                              GtkCheckButton     *cb)
{
  gboolean enabled = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cb));
  if (enabled != automover->priv->enabled)
    {
      automover->priv->enabled = enabled;
      g_signal_emit (automover, hyscan_gtk_automover_signals[SIGNAL_AUTOMOVE_CHANGED], 0, NULL);
    }
}

GtkWidget *
hyscan_gtk_automover_new (void)
{
  return g_object_new (HYSCAN_TYPE_GTK_AUTOMOVER, NULL);
}

void
hyscan_gtk_automover_set_enabled (HyScanGtkAutomover *automover,
                                  gboolean            enabled)
{
  HyScanGtkAutomoverPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_AUTOMOVER (automover));

  priv = automover->priv;

  priv->enabled = enabled;
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->automove_cb), enabled);
}

gboolean
hyscan_gtk_automover_is_enabled (HyScanGtkAutomover *automover)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_AUTOMOVER (automover), FALSE);

  return automover->priv->enabled;
}