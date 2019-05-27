#include <hyscan-hw-profile.h>
#include <hyscan-db-profile.h>
#include <hyscan-offset-profile.h>
#include <hyscan-gtk-profile-db.h>
#include <hyscan-gtk-profile-hw.h>
#include <hyscan-gtk-profile-offset.h>
#include <hyscan-gtk-connector.h>

void cancel_close (HyScanGtkConnector *con,
                   const gchar        *text);

GtkWidget *window;

int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);

  window = hyscan_gtk_connector_new ();
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
  GtkWidget *dialog;

  db = hyscan_gtk_connector_get_db (con);
  control = hyscan_gtk_connector_get_control (con);

  dialog = gtk_message_dialog_new (GTK_WINDOW (window), GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                   "%s clicked.", text);
  gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                            "DB: %s; HW: %s",
                                            db != NULL ? "ok" : "fail",
                                            control != NULL ? "ok" : "fail");

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
  gtk_widget_destroy (GTK_WIDGET (con));

  g_clear_object (&db);
  g_clear_object (&control);
}
