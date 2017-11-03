#ifndef __HYSCAN_GTK_PLAYER_H__
#define __HYSCAN_GTK_PLAYER_H__

#include <gtk/gtk.h>
#include <hyscan-api.h>

G_BEGIN_DECLS

#define HYSCAN_TYPE_GTK_PLAYER            \
        (hyscan_gtk_player_get_type ())

#define HYSCAN_GTK_PLAYER(obj)            \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_PLAYER, HyScanGtkPlayer))

#define HYSCAN_IS_GTK_PLAYER(obj)         \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_PLAYER))

#define HYSCAN_GTK_PLAYER_CLASS(klass)    \
        (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_PLAYER, HyScanGtkPlayerClass))

#define HYSCAN_IS_GTK_PLAYER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_PLAYER))

#define HYSCAN_GTK_PLAYER_GET_CLASS(obj)  \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_PLAYER, HyScanGtkPlayerClass))

typedef struct _HyScanGtkPlayer HyScanGtkPlayer;
typedef struct _HyScanGtkPlayerPrivate HyScanGtkPlayerPrivate;
typedef struct _HyScanGtkPlayerClass HyScanGtkPlayerClass;

struct _HyScanGtkPlayer
{
  GtkGrid parent_instance;
  HyScanGtkPlayerPrivate *priv;
};

struct _HyScanGtkPlayerClass
{
  GtkGridClass parent_class;
};

HYSCAN_API
GType       hyscan_gtk_player_get_type    (void);

HYSCAN_API
GtkWidget*  hyscan_gtk_player_new         (void);

HYSCAN_API
void        hyscan_gtk_player_set_playing (HyScanGtkPlayer  *player,
                                           gboolean          playing);

HYSCAN_API
void        hyscan_gtk_player_set_speed   (HyScanGtkPlayer  *player,
                                           gdouble           speed);

HYSCAN_API
gboolean    hyscan_gtk_player_is_playing  (HyScanGtkPlayer  *player);

HYSCAN_API
gdouble     hyscan_gtk_player_get_speed   (HyScanGtkPlayer  *player);

G_END_DECLS

#endif /* __HYSCAN_GTK_PLAYER_H__ */
