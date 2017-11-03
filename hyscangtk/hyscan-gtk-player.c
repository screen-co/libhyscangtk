/**
 * \file hyscan-gtk-player.h
 *
 * \brief Исходный файл виджета плеера.
 * \author Vladimir Maximov (vmakxs@gmail.com)
 * \date 2018
 * \license Проприетарная лицензия ООО "Экран"
 *
 */

#include "hyscan-gtk-player.h"

enum
{
  SIGNAL_CHANGED,
  SIGNAL_LAST
};

static guint hyscan_gtk_player_signals[SIGNAL_LAST] = { 0 };

struct _HyScanGtkPlayerPrivate
{
  gdouble        speed;

  GtkBuilder    *builder;
  GtkWidget     *content;
  GtkAdjustment *speed_adj;
};

static void  hyscan_gtk_player_constructed    (GObject          *object);
static void  hyscan_gtk_player_finalize       (GObject          *object);
static void  hyscan_gtk_player_speed_changed  (HyScanGtkPlayer  *player,
                                               GtkAdjustment    *adj);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkPlayer, hyscan_gtk_player, GTK_TYPE_GRID)

static void
hyscan_gtk_player_class_init (HyScanGtkPlayerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = hyscan_gtk_player_constructed;
  object_class->finalize = hyscan_gtk_player_finalize;

  hyscan_gtk_player_signals[SIGNAL_CHANGED] =
    g_signal_new ("changed", G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

}

static void
hyscan_gtk_player_init (HyScanGtkPlayer *player)
{
  player->priv = hyscan_gtk_player_get_instance_private (player);
}

static void
hyscan_gtk_player_constructed (GObject *object)
{
  HyScanGtkPlayer *player = HYSCAN_GTK_PLAYER (object);
  HyScanGtkPlayerPrivate *priv = player->priv;

  G_OBJECT_CLASS (hyscan_gtk_player_parent_class)->constructed (object);

  priv->builder = gtk_builder_new_from_resource ("/org/hyscan/gtk/hyscan-gtk-player.ui");
  priv->content = GTK_WIDGET (gtk_builder_get_object (priv->builder, "hyscan_gtk_player"));
  priv->speed_adj = GTK_ADJUSTMENT (gtk_builder_get_object (priv->builder, "speed_adj"));

  gtk_builder_add_callback_symbol (priv->builder, "speed_changed", G_CALLBACK (hyscan_gtk_player_speed_changed));
  gtk_builder_connect_signals (priv->builder, player);

  gtk_container_add (GTK_CONTAINER (player), priv->content);
}

static void
hyscan_gtk_player_finalize (GObject *object)
{
  HyScanGtkPlayer *player = HYSCAN_GTK_PLAYER (object);
  HyScanGtkPlayerPrivate *priv = player->priv;

  g_object_unref (priv->builder);

  G_OBJECT_CLASS (hyscan_gtk_player_parent_class)->finalize (object);
}

static void
hyscan_gtk_player_speed_changed (HyScanGtkPlayer *player,
                                 GtkAdjustment   *adj)
{
  gdouble speed;
  HyScanGtkPlayerPrivate *priv = player->priv;

  speed = gtk_adjustment_get_value (adj);
  
  if (speed == priv->speed)
    return;

  priv->speed = speed;

  g_signal_emit (player, hyscan_gtk_player_signals[SIGNAL_CHANGED], 0, NULL);
}

GtkWidget *
hyscan_gtk_player_new (void)
{
  return GTK_WIDGET (g_object_new (HYSCAN_TYPE_GTK_PLAYER, NULL));
}

void
hyscan_gtk_player_set_speed (HyScanGtkPlayer *player,
                             gdouble          speed)
{
  HyScanGtkPlayerPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_PLAYER (player));

  priv = player->priv;

  if (priv->speed == speed)
    return;

  gtk_adjustment_set_value (priv->speed_adj, speed);
}

gdouble
hyscan_gtk_player_get_speed (HyScanGtkPlayer *player)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_PLAYER (player), 0.0);

  return player->priv->speed;
}
