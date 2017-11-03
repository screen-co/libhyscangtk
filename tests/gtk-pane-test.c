#include <stdlib.h>
#include <hyscan-gtk-pane.h>
#include <hyscan-gtk-colorizer.h>
#include <hyscan-gtk-scaler.h>
#include <hyscan-gtk-leveller.h>
#include <hyscan-gtk-automover.h>
#include <hyscan-gtk-recorder.h>
#include <hyscan-gtk-project-viewer.h>
#include <hyscan-gtk-tvg-control.h>
#include <hyscan-gtk-gen-control.h>
#include <hyscan-gtk-mark-editor.h>

static GtkWidget *
create_content(void)
{
  GtkWidget *colorizer;
  GtkWidget *scaler;
  GtkWidget *leveller;
  GtkWidget *automover;
  GtkGrid *body;
  gint row = 0;

  colorizer = hyscan_gtk_colorizer_new ();
  scaler = hyscan_gtk_scaler_new ();
  leveller = hyscan_gtk_leveller_new ();
  automover = hyscan_gtk_automover_new ();

  body = GTK_GRID (gtk_grid_new ());
  gtk_grid_set_row_spacing (body, 4);
  gtk_grid_attach (body, colorizer, 0, row, 1, 1);
  gtk_grid_attach (body, gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), 0, ++row, 1, 1);
  gtk_grid_attach (body, scaler, 0, ++row, 1, 1);
  gtk_grid_attach (body, gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), 0, ++row, 1, 1);
  gtk_grid_attach (body, leveller, 0, ++row, 1, 1);
  gtk_grid_attach (body, gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), 0, ++row, 1, 1);
  gtk_grid_attach (body, automover, 0, ++row, 1, 1);

  gtk_widget_set_margin_start (GTK_WIDGET (body), 24);
  gtk_widget_set_margin_end (GTK_WIDGET (body), 4);
  gtk_widget_set_margin_top (GTK_WIDGET (body), 4);
  gtk_widget_set_margin_bottom (GTK_WIDGET (body), 4);

  return GTK_WIDGET (body);
}

static void
init_project_viewer (HyScanGtkProjectViewer *pv,
                     const gchar            *prefix,
                     guint                   nprojects)
{
  GtkTreeIter tree_iter;
  GtkListStore *ls;
  GDateTime *dt;

  guint n = nprojects;

  dt = g_date_time_new_now_local();
  ls = hyscan_gtk_project_viewer_get_liststore (pv);

  while (--n)
    {
      GDateTime *dt_buf;
      gchar *name_str;
      gchar *dt_str;

      name_str = g_strdup_printf ("%s %d", prefix, nprojects - n);

      dt_buf = dt;
      dt = g_date_time_add_hours (dt, n % 2);
      g_date_time_unref (dt_buf);

      dt_buf = dt;
      dt = g_date_time_add_minutes (dt, (n + 1) % 2);
      g_date_time_unref (dt_buf);

      dt_str =  g_date_time_format (dt, "%d/%m/%y %H:%M");

      gtk_list_store_append (ls, &tree_iter);
      gtk_list_store_set (ls, &tree_iter,
                          0, name_str,
                          1, name_str,
                          2, dt_str,
                          3, g_date_time_to_unix (dt),
                          -1);

      g_free (name_str);
      g_free (dt_str);
    }

  g_date_time_unref (dt);
}

int
main (int argc, char **argv)
{
  GtkWidget *window;
  GtkWidget *grid;
  GtkWidget *wf_pane, *tracks_pane;

  HyScanDataSchemaEnumValue **presets;

  GtkWidget *tvgc_pane;
  GtkWidget *genc_pane;
  GtkWidget *recorder_pane;

  GtkWidget *genc;
  GtkWidget *tvgc;
  GtkWidget *recorder;

  GtkWidget *tracks_viewer;

  GtkWidget *mark_editor;
  GtkWidget *mark_editor_pane;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  tvgc = hyscan_gtk_tvg_control_new (HYSCAN_TVG_MODE_AUTO | HYSCAN_TVG_MODE_LINEAR_DB);
  gtk_widget_set_margin_start (tvgc, 24);
  gtk_widget_set_margin_end (tvgc, 4);
  gtk_widget_set_margin_top (tvgc, 4);
  gtk_widget_set_margin_bottom (tvgc, 4);

  presets = g_malloc0 (sizeof (HyScanDataSchemaEnumValue *) * 4);
  
  presets[0] = g_malloc (sizeof (HyScanDataSchemaEnumValue));
  presets[0]->value = 0;
  presets[0]->name = g_strdup ("TONE-100");
  presets[0]->description = g_strdup ("TONE-100");
  
  presets[1] = g_malloc (sizeof (HyScanDataSchemaEnumValue));
  presets[1]->value = 1;
  presets[1]->name = g_strdup ("LMF-1");
  presets[1]->description = g_strdup ("LFM-1");
  
  presets[2] = g_malloc (sizeof (HyScanDataSchemaEnumValue));
  presets[2]->value = 2;
  presets[2]->name = g_strdup ("LFM-16");
  presets[2]->description = g_strdup ("LFM-16");
  
  genc = hyscan_gtk_gen_control_new (
    HYSCAN_GENERATOR_MODE_PRESET | HYSCAN_GENERATOR_MODE_AUTO | HYSCAN_GENERATOR_MODE_SIMPLE,
    HYSCAN_GENERATOR_SIGNAL_TONE | HYSCAN_GENERATOR_SIGNAL_LFMD,
    presets
  );
  gtk_widget_set_margin_start (genc, 24);
  gtk_widget_set_margin_end (genc, 4);
  gtk_widget_set_margin_top (genc, 4);
  gtk_widget_set_margin_bottom (genc, 4);

  recorder = hyscan_gtk_recorder_new ();
  gtk_widget_set_margin_start (recorder, 24);
  gtk_widget_set_margin_end (recorder, 4);
  gtk_widget_set_margin_top (recorder, 4);
  gtk_widget_set_margin_bottom (recorder, 4);

  tvgc_pane = hyscan_gtk_pane_new ("TVG", tvgc, TRUE, HYSCAN_GTK_PANE_ARROW);
  genc_pane = hyscan_gtk_pane_new ("Generator", genc, TRUE, HYSCAN_GTK_PANE_ARROW);
  recorder_pane = hyscan_gtk_pane_new ("Record", recorder, TRUE, HYSCAN_GTK_PANE_ARROW);

  tracks_viewer = hyscan_gtk_project_viewer_new ();
  init_project_viewer (HYSCAN_GTK_PROJECT_VIEWER (tracks_viewer), "Track", 30);
  gtk_widget_set_size_request (tracks_viewer, -1, 150);
  gtk_widget_set_hexpand (tracks_viewer, TRUE);
  gtk_widget_set_margin_start (tracks_viewer, 24);
  gtk_widget_set_margin_end (tracks_viewer, 4);
  gtk_widget_set_margin_top (tracks_viewer, 4);
  gtk_widget_set_margin_bottom (tracks_viewer, 4);

  tracks_pane = hyscan_gtk_pane_new ("Tracks", tracks_viewer, FALSE, HYSCAN_GTK_PANE_ARROW);
  wf_pane = hyscan_gtk_pane_new ("Waterfall settings", create_content (), FALSE, HYSCAN_GTK_PANE_ARROW);

  mark_editor = hyscan_gtk_mark_editor_new ();
  gtk_widget_set_margin_start (GTK_WIDGET (mark_editor), 4);
  gtk_widget_set_margin_end (GTK_WIDGET (mark_editor), 4);
  gtk_widget_set_margin_top (GTK_WIDGET (mark_editor), 4);
  gtk_widget_set_margin_bottom (GTK_WIDGET (mark_editor), 4);
  mark_editor_pane = hyscan_gtk_pane_new ("Mark editor", mark_editor, FALSE, HYSCAN_GTK_PANE_ARROW);  

  grid = gtk_grid_new ();
  gtk_grid_attach (GTK_GRID (grid), tracks_pane, 0, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), wf_pane, 0, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), genc_pane, 0, 2, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), tvgc_pane, 0, 3, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), recorder_pane, 0, 4, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), mark_editor_pane, 0, 5, 1, 1);

  gtk_container_add (GTK_CONTAINER (window), grid);
  g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit), NULL);

  gtk_widget_show_all (window);

  gtk_main ();

  return EXIT_SUCCESS;
}
