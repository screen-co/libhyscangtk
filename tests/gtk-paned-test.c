#include <hyscan-gtk-paned.h>

int
main (int    argc,
      char **argv)
{
  GtkWidget *window;
  GtkWidget *paned;
  GtkWidget *central, *pane3, *pane2, *pane1;

  gtk_init (&argc, &argv);

  {
    gchar **args;

    #ifdef G_OS_WIN32
      args = g_win32_get_command_line ();
    #else
      args = g_strdupv (argv);
    #endif

    g_strfreev (args);
  }

  central = gtk_image_new_from_icon_name ("mail-unread", GTK_ICON_SIZE_DIALOG);
  pane3 = gtk_image_new_from_icon_name ("document-open-recent", GTK_ICON_SIZE_DIALOG);
  pane2 = gtk_image_new_from_icon_name ("document-page-setup", GTK_ICON_SIZE_DIALOG);
  pane1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (pane1), gtk_text_view_new ());

  paned = hyscan_gtk_paned_new ();
  hyscan_gtk_paned_set_center_widget (HYSCAN_GTK_PANED (paned), central);
  hyscan_gtk_paned_add (HYSCAN_GTK_PANED (paned), pane1, "pane1", "Text Editor", "accessories-text-editor-symbolic");
  hyscan_gtk_paned_add (HYSCAN_GTK_PANED (paned), pane2, "pane2", "Page Setup", "document-page-setup-symbolic");
  hyscan_gtk_paned_add (HYSCAN_GTK_PANED (paned), pane3, "pane3", "Open Recent", "document-open-recent-symbolic");

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (window), 600, 300);
  gtk_container_add (GTK_CONTAINER (window), paned);
  g_signal_connect_swapped (window, "destroy", G_CALLBACK (gtk_main_quit), window);

  gtk_widget_show_all (window);

  gtk_main ();
}
