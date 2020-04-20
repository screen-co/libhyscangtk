#include <hyscan-profile-hw.h>
#include <hyscan-profile-db.h>
#include <hyscan-profile-offset.h>
#include <hyscan-gtk-profile-db.h>
#include <hyscan-gtk-profile-hw.h>
#include <hyscan-gtk-profile-offset.h>
#include <hyscan-gtk-connector.h>
#include <hyscan-config.h>

void cancel_close (HyScanGtkConnector *con,
                   const gchar        *text);

GtkWidget *window;
const gchar *paths[2]={NULL, NULL};


int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);

  paths[0] = "./";

  window = hyscan_gtk_connector_new ((gchar **)hyscan_config_get_profile_dirs (),
                                     (gchar**)paths);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  g_signal_connect (window, "cancel", G_CALLBACK (cancel_close), "Cancel");
  g_signal_connect (window, "close", G_CALLBACK (cancel_close), "Close");
  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}

void
cancel_close (HyScanGtkConnector *con,
              const gchar        *text)
{
  HyScanDB *db;
  HyScanControl *control;

  db = hyscan_gtk_connector_get_db (con);
  control = hyscan_gtk_connector_get_control (con);

  g_message ("DB: %s (%s)", hyscan_gtk_connector_get_db_name (con), db != NULL ? "ok" : "fail");
  g_message ("Hardware: %s (%s)", hyscan_gtk_connector_get_hw_name (con), control != NULL ? "ok" : "fail");
  g_message ("Offsets: %s", hyscan_gtk_connector_get_offset_name (con));

  gtk_widget_destroy (GTK_WIDGET (con));

  g_clear_object (&db);
  g_clear_object (&control);
}
