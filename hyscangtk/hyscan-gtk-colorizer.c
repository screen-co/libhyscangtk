#include "hyscan-gtk-colorizer.h"

enum
{
  SIGNAL_COLORMAP_CHANGED,
  SIGNAL_LAST
};

static guint hyscan_gtk_colorizer_signals[SIGNAL_LAST] = {0};

struct _HyScanGtkColorizerPrivate
{
  GtkWidget     *colormap_cbt;
  const gchar   *colormap_id;
};

static void   hyscan_gtk_colorizer_constructed        (GObject              *object);
static void   hyscan_gtk_colorizer_finalize           (GObject              *object);
static void   hyscan_gtk_colorizer_colormap_changed   (HyScanGtkColorizer   *colorizer,
                                                       GtkComboBoxText      *cbt);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkColorizer, hyscan_gtk_colorizer, GTK_TYPE_GRID)

static void
hyscan_gtk_colorizer_class_init (HyScanGtkColorizerClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  G_OBJECT_CLASS (klass)->constructed = hyscan_gtk_colorizer_constructed;
  G_OBJECT_CLASS (klass)->finalize = hyscan_gtk_colorizer_finalize;

  hyscan_gtk_colorizer_signals[SIGNAL_COLORMAP_CHANGED] =
    g_signal_new ("colormap-changed", HYSCAN_TYPE_GTK_COLORIZER, G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/hyscan/gtk/hyscan-gtk-colorizer.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkColorizer, colormap_cbt);
  gtk_widget_class_bind_template_callback_full (widget_class, "colormap_changed",
                                                G_CALLBACK (hyscan_gtk_colorizer_colormap_changed));
}

static void
hyscan_gtk_colorizer_init (HyScanGtkColorizer *colorizer)
{
  colorizer->priv = hyscan_gtk_colorizer_get_instance_private (colorizer);
  gtk_widget_init_template (GTK_WIDGET (colorizer));
}

static void
hyscan_gtk_colorizer_constructed (GObject *object)
{
  G_OBJECT_CLASS (hyscan_gtk_colorizer_parent_class)->constructed (object);
}

static void
hyscan_gtk_colorizer_finalize (GObject *object)
{
  G_OBJECT_CLASS (hyscan_gtk_colorizer_parent_class)->finalize (object);
}

static void
hyscan_gtk_colorizer_colormap_changed (HyScanGtkColorizer *colorizer,
                                       GtkComboBoxText    *cbt)
{
  HyScanGtkColorizerPrivate *priv = colorizer->priv;
  const gchar *colormap_id = gtk_combo_box_get_active_id (GTK_COMBO_BOX (cbt));

  if (colormap_id != NULL && colormap_id != priv->colormap_id)
    {
      priv->colormap_id = colormap_id;
      g_signal_emit (colorizer, hyscan_gtk_colorizer_signals[SIGNAL_COLORMAP_CHANGED], 0, NULL);
    }
}

GtkWidget *
hyscan_gtk_colorizer_new (void)
{
  return g_object_new (HYSCAN_TYPE_GTK_COLORIZER, NULL);
}

const gchar *
hyscan_gtk_colorizer_get_colormap (HyScanGtkColorizer *colorizer)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_COLORIZER (colorizer), NULL);

  return colorizer->priv->colormap_id;
}

void
hyscan_gtk_colorizer_set_colormap (HyScanGtkColorizer *colorizer,
                                   const gchar        *colormap_id)
{
  g_return_if_fail (HYSCAN_IS_GTK_COLORIZER (colorizer));

  gtk_combo_box_set_active_id (GTK_COMBO_BOX (colorizer->priv->colormap_cbt), colormap_id);
}

void
hyscan_gtk_colorizer_set_colormaps (HyScanGtkColorizer *colorizer,
                                    HyScanColormapSpec *colormaps,
                                    gsize               length)
{
  HyScanGtkColorizerPrivate *priv = colorizer->priv;
  GtkComboBoxText *cbt;
  guint i;

  g_return_if_fail (HYSCAN_IS_GTK_COLORIZER (colorizer));

  priv = colorizer->priv;

  cbt = GTK_COMBO_BOX_TEXT (priv->colormap_cbt);

  gtk_combo_box_set_active_id (GTK_COMBO_BOX (cbt), NULL);
  gtk_combo_box_text_remove_all (cbt);
  
  for (i = 0; i < length; ++i)
    gtk_combo_box_text_append (cbt, colormaps[i].colormap_id, colormaps[i].colormap_name);

  if (!gtk_combo_box_set_active_id (GTK_COMBO_BOX (cbt), priv->colormap_id))
    {
      priv->colormap_id = NULL;
      gtk_combo_box_set_active (GTK_COMBO_BOX (cbt), 0);
    }
}
