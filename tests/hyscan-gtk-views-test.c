/**
 * \file hyscan-gtk-views-test.c
 *
 * \brief Тест виджетов.
 * \author Vladimir Maximov (vmakxs@gmail.com)
 * \date 2018
 * \license Проприетарная лицензия ООО "Экран"
 *
 */

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
#include <hyscan-gtk-project-creator.h>
#include <hyscan-gtk-sensor-control.h>
// #include <hyscan-sensors-data.h>

static HyScanDataSchemaEnumValue *
make_data_schema_enum_value (gint64  value,
                             gchar  *name,
                             gchar  *description)
{
  HyScanDataSchemaEnumValue *ev = g_new (HyScanDataSchemaEnumValue, 1);
  ev->value = value;
  ev->description = g_strdup(description);
  ev->name = g_strdup (name);
  return ev;
}

static HyScanDataSchemaEnumValue **
make_devices (void)
{
  HyScanDataSchemaEnumValue **devices = g_malloc0 (sizeof (HyScanDataSchemaEnumValue *) * 4);
  devices[0] = make_data_schema_enum_value (0, "device1", "device1");
  devices[1] = make_data_schema_enum_value (1, "device2", "device2");
  devices[2] = make_data_schema_enum_value (2, "device3", "device3");

  return devices;
}

static HyScanDataSchemaEnumValue **
make_modes (void)
{
  HyScanDataSchemaEnumValue **modes = g_malloc0 (sizeof (HyScanDataSchemaEnumValue *) * 4);
  modes[0] = make_data_schema_enum_value (0, "mode1", "mode1");
  modes[1] = make_data_schema_enum_value (1, "mode2", "mode2");
  modes[2] = make_data_schema_enum_value (2, "mode3", "mode3");

  return modes;
}

static HyScanDataSchemaEnumValue **
make_addresses (void)
{
  HyScanDataSchemaEnumValue **addresses = g_malloc0 (sizeof (HyScanDataSchemaEnumValue *) * 4);
  addresses[0] = make_data_schema_enum_value (0, "address1", "address1");
  addresses[1] = make_data_schema_enum_value (1, "address2", "address2");
  addresses[2] = make_data_schema_enum_value (2, "address3", "address3");

  return addresses;
}

static void
show_project_creator (GtkButton               *btn,
                      HyScanGtkProjectCreator *project_creator)
{
  hyscan_gtk_project_creator_set_project (project_creator, "");
  gtk_widget_show_all (GTK_WIDGET (project_creator));
}

static void
project_creator_create (HyScanGtkProjectCreator *project_creator,
                        gpointer                 unused)
{
  (void) unused;
  g_message ("Project: %s", hyscan_gtk_project_creator_get_project (project_creator));
  gtk_widget_hide (GTK_WIDGET (project_creator));
}

static GtkWidget *
create_content (void)
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
init_sensors (GHashTable *sensors)
{
  HyScanDataSchemaEnumValue **addresses;
  HyScanDataSchemaEnumValue **modes;
  HyScanDataSchemaEnumValue **devices;

  addresses = make_addresses ();
  modes = make_modes ();
  devices = make_devices ();
  /*
  hyscan_sensors_data_set_virtual_sensor (sensors, "virtual", 1, 200);
  hyscan_sensors_data_set_state (sensors, "virtual", FALSE);

  hyscan_sensors_data_set_udp_sensor (sensors, "udp", 2, 400, HYSCAN_SENSOR_PROTOCOL_SAS, 0, 2345, addresses);
  hyscan_sensors_data_set_state (sensors, "udp", TRUE);

  hyscan_sensors_data_set_uart_sensor (sensors, "uart", 4, 800, HYSCAN_SENSOR_PROTOCOL_NMEA_0183, 1, 2, devices, modes);
  hyscan_sensors_data_set_state (sensors, "uart", TRUE);
  */
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
  GtkWidget *sensorc_pane;

  GtkWidget *genc;
  GtkWidget *tvgc;
  GtkWidget *recorder;
  GtkWidget *sensorc;
  GHashTable *sensors = NULL;

  GtkWidget *tracks_viewer;

  GtkWidget *mark_editor;
  GtkWidget *mark_editor_pane;

  GtkWidget *project_creator;
  GtkWidget *project_creator_btn;
  int row = 0;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  tvgc = hyscan_gtk_tvg_control_new (HYSCAN_SONAR_TVG_MODE_AUTO | HYSCAN_SONAR_TVG_MODE_LINEAR_DB);
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
    HYSCAN_SONAR_GENERATOR_MODE_PRESET | HYSCAN_SONAR_GENERATOR_MODE_AUTO | HYSCAN_SONAR_GENERATOR_MODE_SIMPLE,
    HYSCAN_SONAR_GENERATOR_SIGNAL_TONE,
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

  /*
  sensorc = hyscan_gtk_sensor_control_new ();
  sensors = hyscan_gtk_sensor_control_get_sensors (HYSCAN_GTK_SENSOR_CONTROL (sensorc));
  init_sensors (sensors);
  g_hash_table_unref (sensors);
  hyscan_gtk_sensor_control_update (HYSCAN_GTK_SENSOR_CONTROL (sensorc));

  gtk_widget_set_margin_start (sensorc, 24);
  gtk_widget_set_margin_end (sensorc, 4);
  gtk_widget_set_margin_top (sensorc, 4);
  gtk_widget_set_margin_bottom (sensorc, 4);
  */
  sensorc = gtk_label_new ("Not implemented");

  tvgc_pane = hyscan_gtk_pane_new ("TVG", tvgc, TRUE, HYSCAN_GTK_PANE_ARROW);
  hyscan_gtk_pane_set_expanded (HYSCAN_GTK_PANE (tvgc_pane), FALSE);

  genc_pane = hyscan_gtk_pane_new ("Generator", genc, TRUE, HYSCAN_GTK_PANE_ARROW);
  hyscan_gtk_pane_set_expanded (HYSCAN_GTK_PANE (genc_pane), FALSE);

  recorder_pane = hyscan_gtk_pane_new ("Record", recorder, TRUE, HYSCAN_GTK_PANE_ARROW);
  hyscan_gtk_pane_set_expanded (HYSCAN_GTK_PANE (recorder_pane), TRUE);

  sensorc_pane = hyscan_gtk_pane_new ("Sensors", sensorc, TRUE, HYSCAN_GTK_PANE_ARROW);
  hyscan_gtk_pane_set_expanded (HYSCAN_GTK_PANE (sensorc_pane), TRUE);

  tracks_viewer = hyscan_gtk_project_viewer_new ();
  init_project_viewer (HYSCAN_GTK_PROJECT_VIEWER (tracks_viewer), "Track", 30);
  gtk_widget_set_size_request (tracks_viewer, -1, 150);
  gtk_widget_set_hexpand (tracks_viewer, TRUE);
  gtk_widget_set_margin_start (tracks_viewer, 24);
  gtk_widget_set_margin_end (tracks_viewer, 4);
  gtk_widget_set_margin_top (tracks_viewer, 4);
  gtk_widget_set_margin_bottom (tracks_viewer, 4);

  tracks_pane = hyscan_gtk_pane_new ("Tracks", tracks_viewer, FALSE, HYSCAN_GTK_PANE_TEXT);
  hyscan_gtk_pane_set_expanded (HYSCAN_GTK_PANE (tracks_pane), FALSE);
  wf_pane = hyscan_gtk_pane_new ("Image", create_content (), FALSE, HYSCAN_GTK_PANE_TEXT);
  hyscan_gtk_pane_set_expanded (HYSCAN_GTK_PANE (wf_pane), FALSE);

  mark_editor = hyscan_gtk_mark_editor_new ();
  gtk_widget_set_margin_start (GTK_WIDGET (mark_editor), 4);
  gtk_widget_set_margin_end (GTK_WIDGET (mark_editor), 4);
  gtk_widget_set_margin_top (GTK_WIDGET (mark_editor), 4);
  gtk_widget_set_margin_bottom (GTK_WIDGET (mark_editor), 4);
  mark_editor_pane = hyscan_gtk_pane_new ("Mark editor", mark_editor, FALSE, HYSCAN_GTK_PANE_ARROW);

  project_creator_btn = gtk_button_new_with_label ("New project");
  project_creator = hyscan_gtk_project_creator_new (project_creator_btn);
  g_signal_connect (project_creator_btn, "clicked", G_CALLBACK (show_project_creator), project_creator);
  g_signal_connect (project_creator, "create", G_CALLBACK (project_creator_create), project_creator);

  grid = gtk_grid_new ();
  gtk_grid_attach (GTK_GRID (grid), project_creator_btn, 0, row++, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), tracks_pane, 0, row++, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), wf_pane, 0, row++, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), genc_pane, 0, row++, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), tvgc_pane, 0, row++, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), recorder_pane, 0, row++, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), sensorc_pane, 0, row++, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), mark_editor_pane, 0, row++, 1, 1);

  gtk_container_add (GTK_CONTAINER (window), grid);
  g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit), NULL);

  gtk_widget_show_all (window);

  gtk_main ();

  return EXIT_SUCCESS;
}
