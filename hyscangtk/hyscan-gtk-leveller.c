/**
 * \file hyscan-gtk-leveller.c
 *
 * \brief Исходный файл виджета управления уровнями.
 * \author Vladimir Maximov (vmakxs@gmail.com)
 * \date 2018
 * \license Проприетарная лицензия ООО "Экран"
 *
 */

#include "hyscan-gtk-leveller.h"

enum
{
  SIGNAL_LEVELS_CHANGED,
  SIGNAL_LAST
};

static guint hyscan_gtk_leveller_signals[SIGNAL_LAST] = {0};

struct _HyScanGtkLevellerPrivate
{
  GtkAdjustment   *black_adj;
  GtkAdjustment   *gamma_adj;
  GtkAdjustment   *white_adj;

  gdouble          black;
  gdouble          gamma;
  gdouble          white;
};

static void   hyscan_gtk_leveller_constructed     (GObject             *object);
static void   hyscan_gtk_leveller_finalize        (GObject             *object);

static void   hyscan_gtk_leveller_black_changed   (HyScanGtkLeveller   *leveller,
                                                   GtkAdjustment       *adj);
static void   hyscan_gtk_leveller_gamma_changed   (HyScanGtkLeveller   *leveller,
                                                   GtkAdjustment       *adj);
static void   hyscan_gtk_leveller_white_changed   (HyScanGtkLeveller   *leveller,
                                                   GtkAdjustment       *adj);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkLeveller, hyscan_gtk_leveller, GTK_TYPE_GRID)

static void
hyscan_gtk_leveller_class_init (HyScanGtkLevellerClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  G_OBJECT_CLASS (klass)->constructed = hyscan_gtk_leveller_constructed;
  G_OBJECT_CLASS (klass)->finalize = hyscan_gtk_leveller_finalize;

  hyscan_gtk_leveller_signals[SIGNAL_LEVELS_CHANGED] =
    g_signal_new ("levels-changed", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/hyscan/gtk/hyscan-gtk-leveller.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkLeveller, white_adj);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkLeveller, gamma_adj);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkLeveller, black_adj);
  gtk_widget_class_bind_template_callback_full (widget_class, "black_changed",
                                                G_CALLBACK (hyscan_gtk_leveller_black_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "gamma_changed",
                                                G_CALLBACK (hyscan_gtk_leveller_gamma_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "white_changed",
                                                G_CALLBACK (hyscan_gtk_leveller_white_changed));
}

static void
hyscan_gtk_leveller_init (HyScanGtkLeveller *leveller)
{
  leveller->priv = hyscan_gtk_leveller_get_instance_private (leveller);
  gtk_widget_init_template (GTK_WIDGET (leveller));
}

static void
hyscan_gtk_leveller_constructed (GObject *object)
{
  G_OBJECT_CLASS (hyscan_gtk_leveller_parent_class)->constructed (object);
}

static void
hyscan_gtk_leveller_finalize (GObject *object)
{
  G_OBJECT_CLASS (hyscan_gtk_leveller_parent_class)->finalize (object);
}

static void
hyscan_gtk_leveller_black_changed (HyScanGtkLeveller *leveller,
                                   GtkAdjustment     *adj)
{
  HyScanGtkLevellerPrivate *priv = leveller->priv;
  gdouble black = gtk_adjustment_get_value (adj);
  if (priv->black != black)
    {
      priv->black = black;
      g_signal_emit (leveller, hyscan_gtk_leveller_signals[SIGNAL_LEVELS_CHANGED], 0, NULL);
    }
}

static void
hyscan_gtk_leveller_gamma_changed (HyScanGtkLeveller *leveller,
                                   GtkAdjustment     *adj)
{
  HyScanGtkLevellerPrivate *priv = leveller->priv;
  gdouble gamma = gtk_adjustment_get_value (adj);
  if (priv->gamma != gamma)
    {
      priv->gamma = gamma;
      g_signal_emit (leveller, hyscan_gtk_leveller_signals[SIGNAL_LEVELS_CHANGED], 0, NULL);
    }
}

static void
hyscan_gtk_leveller_white_changed (HyScanGtkLeveller *leveller,
                                   GtkAdjustment     *adj)
{
  HyScanGtkLevellerPrivate *priv = leveller->priv;
  gdouble white = gtk_adjustment_get_value (adj);
  if (priv->white != white)
    {
      priv->white = white;
      g_signal_emit (leveller, hyscan_gtk_leveller_signals[SIGNAL_LEVELS_CHANGED], 0, NULL);
    }
}

GtkWidget *
hyscan_gtk_leveller_new (void)
{
  return g_object_new (HYSCAN_TYPE_GTK_LEVELLER, NULL);
}

void
hyscan_gtk_leveller_set_white_range (HyScanGtkLeveller *leveller,
                                     gdouble            lower,
                                     gdouble            upper)
{
  g_return_if_fail (HYSCAN_IS_GTK_LEVELLER (leveller));

  gtk_adjustment_set_lower (leveller->priv->white_adj, lower);
  gtk_adjustment_set_upper (leveller->priv->white_adj, upper);
}

void
hyscan_gtk_leveller_set_gamma_range (HyScanGtkLeveller *leveller,
                                     gdouble            lower,
                                     gdouble            upper)
{
  g_return_if_fail (HYSCAN_IS_GTK_LEVELLER (leveller));

  gtk_adjustment_set_lower (leveller->priv->gamma_adj, lower);
  gtk_adjustment_set_upper (leveller->priv->gamma_adj, upper);
}

void
hyscan_gtk_leveller_set_black_range (HyScanGtkLeveller *leveller,
                                     gdouble            lower,
                                     gdouble            upper)
{
  g_return_if_fail (HYSCAN_IS_GTK_LEVELLER (leveller));

  gtk_adjustment_set_lower (leveller->priv->black_adj, lower);
  gtk_adjustment_set_upper (leveller->priv->black_adj, upper);
}

void
hyscan_gtk_leveller_set_levels (HyScanGtkLeveller *leveller,
                                gdouble            black,
                                gdouble            gamma,
                                gdouble            white)
{
  HyScanGtkLevellerPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_LEVELLER (leveller));

  priv = leveller->priv;

  priv->black = black;
  priv->gamma = gamma;
  priv->white = white;

  gtk_adjustment_set_value (priv->black_adj, black);
  gtk_adjustment_set_value (priv->gamma_adj, gamma);
  gtk_adjustment_set_value (priv->white_adj, white);
}

void
hyscan_gtk_leveller_get_levels (HyScanGtkLeveller *leveller,
                                gdouble           *black,
                                gdouble           *gamma,
                                gdouble           *white)
{
  HyScanGtkLevellerPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_LEVELLER (leveller));

  priv = leveller->priv;

  if (black != NULL)
    *black = priv->black;
  if (gamma != NULL)
    *gamma = priv->gamma;
  if (white != NULL)
    *white = priv->white;
}
