/**
 * \file hyscan-gtk-sensor-control.c
 *
 * \brief Исходный файл виджета управления датчиками.
 * \author Vladimir Maximov (vmakxs@gmail.com)
 * \date 2018
 * \license Проприетарная лицензия ООО "Экран"
 *
 */

#include "hyscan-gtk-sensor-control.h"

enum
{
  PROP_O,
  PROP_SENSORS
};

enum
{
  SIGNAL_CHANGED,
  SIGNAL_LAST
};

enum
{
  SAS_SENSOR = 0,
  NMEA_SENSOR = 1
};

static guint hyscan_gtk_sensor_control_signals[SIGNAL_LAST] = {0};

struct _HyScanGtkSensorControlPrivate
{
  GtkBuilder    *builder;
  GtkWidget     *sensor_control_view;
  GtkWidget     *ports_cbt;
  GtkWidget     *port_type_name_label;
  GtkWidget     *enable_checkbtn;
  GtkWidget     *params_stack;
  GtkWidget     *virtual_port_grid;
  GtkAdjustment *virtual_channel_adjustment;
  GtkAdjustment *virtual_time_offset_adjustment;
  GtkWidget     *uart_port_grid;
  GtkAdjustment *uart_channel_adjustment;
  GtkAdjustment *uart_time_offset_adjustment;
  GtkWidget     *uart_protocol_cbt;
  GtkWidget     *uart_device_cbt;
  GtkWidget     *uart_mode_cbt;
  GtkWidget     *udp_port_grid;
  GtkAdjustment *udp_channel_adjustment;
  GtkAdjustment *udp_time_offset_adjustment;
  GtkWidget     *udp_protocol_cbt;
  GtkWidget     *udp_addr_cbt;
  GtkAdjustment *udp_port_adjustment;

  gboolean       update;         /* Состояние обновления - для предотвращения испускания сигналов. */
  gchar         *current_sensor; /* Текущий датчик. */
  GHashTable    *sensors;        /* Состояние датчиков. */
};

static void  hyscan_gtk_sensor_control_constructed                  (GObject                 *object);
static void  hyscan_gtk_sensor_control_finalize                     (GObject                 *object);

static void  hyscan_gtk_sensor_control_begin_update                 (HyScanGtkSensorControl  *gtk_sensor_control);
static void  hyscan_gtk_sensor_control_end_update                   (HyScanGtkSensorControl  *gtk_sensor_control);
static void  hyscan_gtk_sensor_control_emit_changed                 (HyScanGtkSensorControl  *gtk_sensor_control);
static void  hyscan_gtk_sensor_control_set_sensor                   (HyScanGtkSensorControl  *gtk_sensor_control,
                                                                     const gchar             *sensor);

static void  hyscan_gtk_sensor_control_virtual_channel_changed      (HyScanGtkSensorControl  *gtk_sensor_control,
                                                                     GtkAdjustment           *adj);
static void  hyscan_gtk_sensor_control_virtual_time_offset_changed  (HyScanGtkSensorControl  *gtk_sensor_control,
                                                                     GtkAdjustment           *adj);
static void  hyscan_gtk_sensor_control_uart_channel_changed         (HyScanGtkSensorControl  *gtk_sensor_control,
                                                                     GtkAdjustment           *adj);
static void  hyscan_gtk_sensor_control_uart_time_offset_changed     (HyScanGtkSensorControl  *gtk_sensor_control,
                                                                     GtkAdjustment           *adj);
static void  hyscan_gtk_sensor_control_udp_channel_changed          (HyScanGtkSensorControl  *gtk_sensor_control,
                                                                     GtkAdjustment           *adj);
static void  hyscan_gtk_sensor_control_udp_time_offset_changed      (HyScanGtkSensorControl  *gtk_sensor_control,
                                                                     GtkAdjustment           *adj);
static void  hyscan_gtk_sensor_control_udp_port_changed             (HyScanGtkSensorControl  *gtk_sensor_control,
                                                                     GtkAdjustment           *adj);
static void  hyscan_gtk_sensor_control_port_changed                 (HyScanGtkSensorControl  *gtk_sensor_control,
                                                                     GtkComboBoxText         *cbt);
static void  hyscan_gtk_sensor_control_uart_protocol_changed        (HyScanGtkSensorControl  *gtk_sensor_control,
                                                                     GtkComboBoxText         *cbt);
static void  hyscan_gtk_sensor_control_uart_device_changed          (HyScanGtkSensorControl  *gtk_sensor_control,
                                                                     GtkComboBoxText         *cbt);
static void  hyscan_gtk_sensor_control_uart_mode_changed            (HyScanGtkSensorControl  *gtk_sensor_control,
                                                                     GtkComboBoxText         *cbt);
static void  hyscan_gtk_sensor_control_udp_protocol_changed         (HyScanGtkSensorControl  *gtk_sensor_control,
                                                                     GtkComboBoxText         *cbt);
static void  hyscan_gtk_sensor_control_udp_addr_changed             (HyScanGtkSensorControl  *gtk_sensor_control,
                                                                     GtkComboBoxText         *cbt);
static void  hyscan_gtk_sensor_control_enable_toggled               (HyScanGtkSensorControl  *gtk_sensor_control,
                                                                     GtkCheckButton          *checkbtn);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkSensorControl, hyscan_gtk_sensor_control, GTK_TYPE_GRID)

static void
hyscan_gtk_sensor_control_class_init (HyScanGtkSensorControlClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = hyscan_gtk_sensor_control_constructed;
  object_class->finalize = hyscan_gtk_sensor_control_finalize;

  /* Сигнал изменения в датчиках. */
  hyscan_gtk_sensor_control_signals[SIGNAL_CHANGED] =
    g_signal_new ("changed", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
}

static void
hyscan_gtk_sensor_control_init (HyScanGtkSensorControl *sensor_control)
{
  sensor_control->priv = hyscan_gtk_sensor_control_get_instance_private (sensor_control);
}

static void
hyscan_gtk_sensor_control_constructed (GObject *object)
{
  HyScanGtkSensorControl *gtk_sensor_control = HYSCAN_GTK_SENSOR_CONTROL (object);
  HyScanGtkSensorControlPrivate *priv = gtk_sensor_control->priv;

  G_OBJECT_CLASS (hyscan_gtk_sensor_control_parent_class)->constructed (object);

  priv->builder = gtk_builder_new_from_resource ("/org/hyscan/gtk/hyscan-gtk-sensor-control.ui");
  priv->sensor_control_view = GTK_WIDGET (gtk_builder_get_object (priv->builder, "sensor_control_view"));
  priv->ports_cbt = GTK_WIDGET (gtk_builder_get_object (priv->builder, "ports_cbt"));
  priv->port_type_name_label = GTK_WIDGET (gtk_builder_get_object (priv->builder, "port_type_name_label"));
  priv->enable_checkbtn = GTK_WIDGET (gtk_builder_get_object (priv->builder, "enable_checkbtn"));

  priv->params_stack = GTK_WIDGET (gtk_builder_get_object (priv->builder, "params_stack"));

  priv->virtual_port_grid = GTK_WIDGET (gtk_builder_get_object (priv->builder, "virtual_port_grid"));
  priv->virtual_channel_adjustment = GTK_ADJUSTMENT (
    gtk_builder_get_object (priv->builder, "virtual_channel_adjustment")
  );
  priv->virtual_time_offset_adjustment = GTK_ADJUSTMENT (
    gtk_builder_get_object (priv->builder, "virtual_time_offset_adjustment")
  );

  priv->uart_port_grid = GTK_WIDGET (gtk_builder_get_object (priv->builder, "uart_port_grid"));
  priv->uart_channel_adjustment = GTK_ADJUSTMENT (gtk_builder_get_object (priv->builder, "uart_channel_adjustment"));
  priv->uart_time_offset_adjustment = GTK_ADJUSTMENT (
    gtk_builder_get_object (priv->builder, "uart_time_offset_adjustment")
  );
  priv->uart_protocol_cbt = GTK_WIDGET (gtk_builder_get_object (priv->builder, "uart_protocol_cbt"));
  priv->uart_device_cbt = GTK_WIDGET (gtk_builder_get_object (priv->builder, "uart_device_cbt"));
  priv->uart_mode_cbt = GTK_WIDGET (gtk_builder_get_object (priv->builder, "uart_mode_cbt"));

  priv->udp_port_grid = GTK_WIDGET (gtk_builder_get_object (priv->builder, "udp_port_grid"));
  priv->udp_channel_adjustment = GTK_ADJUSTMENT (gtk_builder_get_object (priv->builder, "udp_channel_adjustment"));
  priv->udp_time_offset_adjustment = GTK_ADJUSTMENT (
    gtk_builder_get_object (priv->builder, "udp_time_offset_adjustment")
  );
  priv->udp_protocol_cbt = GTK_WIDGET (gtk_builder_get_object (priv->builder, "udp_protocol_cbt"));
  priv->udp_addr_cbt = GTK_WIDGET (gtk_builder_get_object (priv->builder, "udp_addr_cbt"));
  priv->udp_port_adjustment = GTK_ADJUSTMENT (gtk_builder_get_object (priv->builder, "udp_port_adjustment"));

  gtk_builder_add_callback_symbol (priv->builder, "uart_channel_changed",
                                   G_CALLBACK (hyscan_gtk_sensor_control_uart_channel_changed));
  gtk_builder_add_callback_symbol (priv->builder, "uart_time_offset_changed",
                                   G_CALLBACK (hyscan_gtk_sensor_control_uart_time_offset_changed));
  gtk_builder_add_callback_symbol (priv->builder, "udp_channel_changed",
                                   G_CALLBACK (hyscan_gtk_sensor_control_udp_channel_changed));
  gtk_builder_add_callback_symbol (priv->builder, "udp_port_changed",
                                   G_CALLBACK (hyscan_gtk_sensor_control_udp_port_changed));
  gtk_builder_add_callback_symbol (priv->builder, "udp_time_offset_changed",
                                   G_CALLBACK (hyscan_gtk_sensor_control_udp_time_offset_changed));
  gtk_builder_add_callback_symbol (priv->builder, "virtual_channel_changed",
                                   G_CALLBACK (hyscan_gtk_sensor_control_virtual_channel_changed));
  gtk_builder_add_callback_symbol (priv->builder, "virtual_time_offset_changed",
                                   G_CALLBACK (hyscan_gtk_sensor_control_virtual_time_offset_changed));

  gtk_builder_add_callback_symbol (priv->builder, "port_changed",
                                   G_CALLBACK (hyscan_gtk_sensor_control_port_changed));
  gtk_builder_add_callback_symbol (priv->builder, "uart_protocol_changed",
                                   G_CALLBACK (hyscan_gtk_sensor_control_uart_protocol_changed));
  gtk_builder_add_callback_symbol (priv->builder, "uart_device_changed",
                                   G_CALLBACK (hyscan_gtk_sensor_control_uart_device_changed));
  gtk_builder_add_callback_symbol (priv->builder, "uart_mode_changed",
                                   G_CALLBACK (hyscan_gtk_sensor_control_uart_mode_changed));
  gtk_builder_add_callback_symbol (priv->builder, "udp_protocol_changed",
                                   G_CALLBACK (hyscan_gtk_sensor_control_udp_protocol_changed));
  gtk_builder_add_callback_symbol (priv->builder, "udp_addr_changed",
                                   G_CALLBACK (hyscan_gtk_sensor_control_udp_addr_changed));

  gtk_builder_add_callback_symbol (priv->builder, "enable_toggled",
                                   G_CALLBACK (hyscan_gtk_sensor_control_enable_toggled));

  gtk_builder_connect_signals (priv->builder, gtk_sensor_control);

  gtk_grid_attach(GTK_GRID (gtk_sensor_control), priv->sensor_control_view, 0, 0, 1, 1);
  gtk_widget_set_hexpand (GTK_WIDGET (priv->sensor_control_view), TRUE);
  gtk_widget_set_halign (GTK_WIDGET (priv->sensor_control_view), GTK_ALIGN_FILL);

  priv->sensors = hyscan_sensors_data_new ();
}

static void
hyscan_gtk_sensor_control_finalize (GObject *object)
{
  HyScanGtkSensorControl *gtk_sensor_control = HYSCAN_GTK_SENSOR_CONTROL (object);
  HyScanGtkSensorControlPrivate *priv = gtk_sensor_control->priv;

  g_object_unref (priv->builder);
  g_hash_table_unref (priv->sensors);
  g_free (priv->current_sensor);

  G_OBJECT_CLASS (hyscan_gtk_sensor_control_parent_class)->finalize (object);
}

static void
hyscan_gtk_sensor_control_begin_update (HyScanGtkSensorControl *gtk_sensor_control)
{
  gtk_sensor_control->priv->update = TRUE;
}

static void
hyscan_gtk_sensor_control_end_update (HyScanGtkSensorControl *gtk_sensor_control)
{
  gtk_sensor_control->priv->update = FALSE;
}

static void
hyscan_gtk_sensor_control_emit_changed (HyScanGtkSensorControl *gtk_sensor_control)
{
  HyScanGtkSensorControlPrivate *priv = gtk_sensor_control->priv;
  if (!priv->update)
    g_signal_emit (gtk_sensor_control, hyscan_gtk_sensor_control_signals[SIGNAL_CHANGED], 0, priv->current_sensor);
}

static void
hyscan_gtk_sensor_control_set_sensor (HyScanGtkSensorControl *gtk_sensor_control,
                                      const gchar            *sensor)
{
  HyScanGtkSensorControlPrivate *priv;
  HyScanSensorPrm *sensor_prm;

  priv = gtk_sensor_control->priv;

  if (sensor == NULL)
    return;

  sensor_prm = g_hash_table_lookup (priv->sensors, sensor);
  if (sensor_prm == NULL)
    return;

  hyscan_gtk_sensor_control_begin_update (gtk_sensor_control);

  /* Задание текущего датчика - этот датчик будет отправлен в качестве параметра сигнала changed. */
  g_free (priv->current_sensor);
  priv->current_sensor = g_strdup (sensor);

  gtk_combo_box_set_active_id (GTK_COMBO_BOX (priv->ports_cbt), priv->current_sensor);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->enable_checkbtn), sensor_prm->state);

  if (sensor_prm->port_type == HYSCAN_SENSOR_PORT_VIRTUAL)
    {
      HyScanVirtualPortPrm *prm = sensor_prm->port_prm;

      gtk_label_set_text (GTK_LABEL (priv->port_type_name_label), "Virtual");
      gtk_stack_set_visible_child (GTK_STACK (priv->params_stack), priv->virtual_port_grid);
      gtk_widget_set_sensitive (priv->virtual_port_grid, sensor_prm->state);

      gtk_adjustment_set_value (priv->virtual_channel_adjustment, prm->channel);
      gtk_adjustment_set_value (priv->virtual_time_offset_adjustment, prm->time_offset);
    }
  else if (sensor_prm->port_type == HYSCAN_SENSOR_PORT_UART)
    {
      HyScanUARTPortPrm *prm;
      HyScanDataSchemaEnumValue **values;
      guint i, active_id;

      prm = sensor_prm->port_prm;

      gtk_label_set_text (GTK_LABEL (priv->port_type_name_label), "UART");
      gtk_stack_set_visible_child (GTK_STACK (priv->params_stack), priv->uart_port_grid);
      gtk_widget_set_sensitive (priv->uart_port_grid, sensor_prm->state);

      gtk_adjustment_set_value (priv->uart_channel_adjustment, prm->channel);
      gtk_adjustment_set_value (priv->uart_time_offset_adjustment, prm->time_offset);

      if (prm->protocol == HYSCAN_SENSOR_PROTOCOL_SAS)
        gtk_combo_box_set_active (GTK_COMBO_BOX (priv->uart_protocol_cbt), SAS_SENSOR);
      else
        gtk_combo_box_set_active (GTK_COMBO_BOX (priv->uart_protocol_cbt), NMEA_SENSOR);

      gtk_combo_box_set_active (GTK_COMBO_BOX (priv->uart_device_cbt), -1);
      gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT (priv->uart_device_cbt));
      values = prm->devices;
      for (i = 0; values[i] != NULL; ++i)
        {
          gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (priv->uart_device_cbt),values[i]->name, values[i]->name);
          if (prm->device == values[i]->value)
            active_id = i;
        }
      gtk_combo_box_set_active (GTK_COMBO_BOX (priv->uart_device_cbt), active_id);

      gtk_combo_box_set_active (GTK_COMBO_BOX (priv->uart_mode_cbt), -1);
      gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT (priv->uart_mode_cbt));
      values = prm->modes;
      for (i = 0; values[i] != NULL; ++i)
        {
          gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (priv->uart_mode_cbt),values[i]->name, values[i]->name);
          if (prm->mode == values[i]->value)
            active_id = i;
        }
      gtk_combo_box_set_active (GTK_COMBO_BOX (priv->uart_mode_cbt), active_id);
    }
  else if (sensor_prm->port_type == HYSCAN_SENSOR_PORT_UDP_IP)
    {
      HyScanUDPPortPrm *prm;
      HyScanDataSchemaEnumValue **values;
      guint i, active_id;

      prm = sensor_prm->port_prm;

      gtk_label_set_text (GTK_LABEL (priv->port_type_name_label), "UDP/IP");
      gtk_stack_set_visible_child (GTK_STACK (priv->params_stack), priv->udp_port_grid);
      gtk_widget_set_sensitive (priv->udp_port_grid, sensor_prm->state);

      gtk_adjustment_set_value (priv->udp_channel_adjustment, prm->channel);
      gtk_adjustment_set_value (priv->udp_time_offset_adjustment, prm->time_offset);
      gtk_adjustment_set_value (priv->udp_port_adjustment, prm->port);

      if (prm->protocol == HYSCAN_SENSOR_PROTOCOL_SAS)
        gtk_combo_box_set_active (GTK_COMBO_BOX (priv->udp_protocol_cbt), SAS_SENSOR);
      else
        gtk_combo_box_set_active (GTK_COMBO_BOX (priv->udp_protocol_cbt), NMEA_SENSOR);

      gtk_combo_box_set_active (GTK_COMBO_BOX (priv->udp_addr_cbt), -1);
      gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT (priv->udp_addr_cbt));
      values = prm->addresses;
      for (i = 0; values[i] != NULL; ++i)
        {
          gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (priv->udp_addr_cbt),values[i]->name, values[i]->name);
          if (prm->addr == values[i]->value)
            active_id = i;
        }
      gtk_combo_box_set_active (GTK_COMBO_BOX (priv->udp_addr_cbt), active_id);
    }

  hyscan_gtk_sensor_control_end_update (gtk_sensor_control);
}

static void
hyscan_gtk_sensor_control_virtual_channel_changed (HyScanGtkSensorControl *gtk_sensor_control,
                                                   GtkAdjustment          *adj)
{
  HyScanSensorPrm *sensor_prm;
  HyScanGtkSensorControlPrivate *priv;

  priv = gtk_sensor_control->priv;

  if (priv->update)
    return;

  if (priv->current_sensor == NULL)
    return;

  sensor_prm = g_hash_table_lookup (priv->sensors, priv->current_sensor);
  if (sensor_prm == NULL)
    return;

  ((HyScanVirtualPortPrm *) sensor_prm->port_prm)->channel = (guint) gtk_adjustment_get_value (adj);

  hyscan_gtk_sensor_control_emit_changed (gtk_sensor_control);
}

static void
hyscan_gtk_sensor_control_virtual_time_offset_changed (HyScanGtkSensorControl *gtk_sensor_control,
                                                       GtkAdjustment          *adj)
{
  HyScanSensorPrm *sensor_prm;
  HyScanGtkSensorControlPrivate *priv;

  priv = gtk_sensor_control->priv;

  if (priv->update)
    return;

  if (priv->current_sensor == NULL)
    return;

  sensor_prm = g_hash_table_lookup (priv->sensors, priv->current_sensor);
  if (sensor_prm == NULL)
    return;

  ((HyScanVirtualPortPrm *) sensor_prm->port_prm)->time_offset = (guint) gtk_adjustment_get_value (adj);

  hyscan_gtk_sensor_control_emit_changed (gtk_sensor_control);
}

static void
hyscan_gtk_sensor_control_uart_channel_changed (HyScanGtkSensorControl *gtk_sensor_control,
                                                GtkAdjustment          *adj)
{
  HyScanSensorPrm *sensor_prm;
  HyScanGtkSensorControlPrivate *priv;

  priv = gtk_sensor_control->priv;

  if (priv->update)
    return;

  if (priv->current_sensor == NULL)
    return;

  sensor_prm = g_hash_table_lookup (priv->sensors, priv->current_sensor);
  if (sensor_prm == NULL)
    return;

  ((HyScanUARTPortPrm *) sensor_prm->port_prm)->channel = (guint) gtk_adjustment_get_value (adj);

  hyscan_gtk_sensor_control_emit_changed (gtk_sensor_control);
}

static void
hyscan_gtk_sensor_control_uart_time_offset_changed (HyScanGtkSensorControl *gtk_sensor_control,
                                                    GtkAdjustment          *adj)
{
  HyScanSensorPrm *sensor_prm;
  HyScanGtkSensorControlPrivate *priv;

  priv = gtk_sensor_control->priv;

  if (priv->update)
    return;

  if (priv->current_sensor == NULL)
    return;

  sensor_prm = g_hash_table_lookup (priv->sensors, priv->current_sensor);
  if (sensor_prm == NULL)
    return;

  ((HyScanUARTPortPrm *) sensor_prm->port_prm)->time_offset = (guint) gtk_adjustment_get_value (adj);

  hyscan_gtk_sensor_control_emit_changed (gtk_sensor_control);
}

static void
hyscan_gtk_sensor_control_udp_channel_changed (HyScanGtkSensorControl *gtk_sensor_control,
                                               GtkAdjustment          *adj)
{
  HyScanSensorPrm *sensor_prm;
  HyScanGtkSensorControlPrivate *priv;

  priv = gtk_sensor_control->priv;

  if (priv->update)
    return;

  if (priv->current_sensor == NULL)
    return;

  sensor_prm = g_hash_table_lookup (priv->sensors, priv->current_sensor);
  if (sensor_prm == NULL)
    return;

  ((HyScanUDPPortPrm *) sensor_prm->port_prm)->channel = (guint) gtk_adjustment_get_value (adj);

  hyscan_gtk_sensor_control_emit_changed (gtk_sensor_control);
}

static void
hyscan_gtk_sensor_control_udp_time_offset_changed (HyScanGtkSensorControl *gtk_sensor_control,
                                                   GtkAdjustment          *adj)
{
  HyScanSensorPrm *sensor_prm;
  HyScanGtkSensorControlPrivate *priv;

  priv = gtk_sensor_control->priv;

  if (priv->update)
    return;

  if (priv->current_sensor == NULL)
    return;

  sensor_prm = g_hash_table_lookup (priv->sensors, priv->current_sensor);
  if (sensor_prm == NULL)
    return;

  ((HyScanUDPPortPrm *) sensor_prm->port_prm)->time_offset = (guint) gtk_adjustment_get_value (adj);

  hyscan_gtk_sensor_control_emit_changed (gtk_sensor_control);
}

static void
hyscan_gtk_sensor_control_udp_port_changed (HyScanGtkSensorControl *gtk_sensor_control,
                                            GtkAdjustment          *adj)
{
  HyScanSensorPrm *sensor_prm;
  HyScanGtkSensorControlPrivate *priv;

  priv = gtk_sensor_control->priv;

  if (priv->update)
    return;

  if (priv->current_sensor == NULL)
    return;

  sensor_prm = g_hash_table_lookup (priv->sensors, priv->current_sensor);
  if (sensor_prm == NULL)
    return;

  ((HyScanUDPPortPrm *) sensor_prm->port_prm)->port = (guint16) gtk_adjustment_get_value (adj);

  hyscan_gtk_sensor_control_emit_changed (gtk_sensor_control);
}

static void
hyscan_gtk_sensor_control_port_changed (HyScanGtkSensorControl *gtk_sensor_control,
                                        GtkComboBoxText        *cbt)
{
  const gchar *sensor = gtk_combo_box_get_active_id (GTK_COMBO_BOX (cbt));
  if (!gtk_sensor_control->priv->update)
    hyscan_gtk_sensor_control_set_sensor (gtk_sensor_control, sensor);
}

static void
hyscan_gtk_sensor_control_uart_protocol_changed (HyScanGtkSensorControl *gtk_sensor_control,
                                                 GtkComboBoxText        *cbt)
{
  HyScanSensorPrm *sensor_prm;
  HyScanUARTPortPrm *prm;
  HyScanGtkSensorControlPrivate *priv;
  gint active;

  priv = gtk_sensor_control->priv;

  if (priv->update)
    return;

  sensor_prm = g_hash_table_lookup (priv->sensors, priv->current_sensor);
  if (sensor_prm == NULL)
    return;

  prm = sensor_prm->port_prm;

  active = gtk_combo_box_get_active (GTK_COMBO_BOX (cbt));
  if (active == SAS_SENSOR)
    prm->protocol = HYSCAN_SENSOR_PROTOCOL_SAS;
  else
    prm->protocol = HYSCAN_SENSOR_PROTOCOL_NMEA_0183;

  hyscan_gtk_sensor_control_emit_changed (gtk_sensor_control);
}

static void
hyscan_gtk_sensor_control_uart_device_changed (HyScanGtkSensorControl *gtk_sensor_control,
                                               GtkComboBoxText        *cbt)
{
  HyScanSensorPrm *sensor_prm;
  HyScanUARTPortPrm *prm;
  HyScanGtkSensorControlPrivate *priv;
  const gchar *active_id;
  HyScanDataSchemaEnumValue **values;

  priv = gtk_sensor_control->priv;

  if (priv->update)
    return;

  active_id = gtk_combo_box_get_active_id (GTK_COMBO_BOX (cbt));
  if (active_id == NULL)
    return;

  sensor_prm = g_hash_table_lookup (priv->sensors, priv->current_sensor);
  if (sensor_prm == NULL)
    return;

  prm = sensor_prm->port_prm;
  values = prm->devices;

  while (*values != NULL)
    {
      HyScanDataSchemaEnumValue *ev = *values;
      if (g_str_equal (ev->name, active_id))
        {
          prm->device = ev->value;
          break;
        }
      ++values;
    }

  hyscan_gtk_sensor_control_emit_changed (gtk_sensor_control);
}

static void
hyscan_gtk_sensor_control_uart_mode_changed (HyScanGtkSensorControl *gtk_sensor_control,
                                             GtkComboBoxText        *cbt)
{
  HyScanSensorPrm *sensor_prm;
  HyScanUARTPortPrm *prm;
  HyScanGtkSensorControlPrivate *priv;
  const gchar *active_id;
  HyScanDataSchemaEnumValue **values;

  priv = gtk_sensor_control->priv;

  if (priv->update)
    return;

  active_id = gtk_combo_box_get_active_id (GTK_COMBO_BOX (cbt));
  if (active_id == NULL)
    return;

  sensor_prm = g_hash_table_lookup (priv->sensors, priv->current_sensor);
  if (sensor_prm == NULL)
    return;

  prm = sensor_prm->port_prm;
  values = prm->modes;

  while (*values != NULL)
    {
      HyScanDataSchemaEnumValue *ev = *values;
      if (g_str_equal (ev->name, active_id))
        {
          prm->mode = ev->value;
          break;
        }
      ++values;
    }

  hyscan_gtk_sensor_control_emit_changed (gtk_sensor_control);
}

static void
hyscan_gtk_sensor_control_udp_protocol_changed (HyScanGtkSensorControl *gtk_sensor_control,
                                                GtkComboBoxText        *cbt)
{
  HyScanSensorPrm *sensor_prm;
  HyScanUDPPortPrm *prm;
  HyScanGtkSensorControlPrivate *priv;
  gint active;

  priv = gtk_sensor_control->priv;

  if (priv->update)
    return;

  sensor_prm = g_hash_table_lookup (priv->sensors, priv->current_sensor);
  if (sensor_prm == NULL)
    return;

  prm = sensor_prm->port_prm;

  active = gtk_combo_box_get_active (GTK_COMBO_BOX (cbt));
  if (active == SAS_SENSOR)
    prm->protocol = HYSCAN_SENSOR_PROTOCOL_SAS;
  else
    prm->protocol = HYSCAN_SENSOR_PROTOCOL_NMEA_0183;

  hyscan_gtk_sensor_control_emit_changed (gtk_sensor_control);
}

static void
hyscan_gtk_sensor_control_udp_addr_changed (HyScanGtkSensorControl *gtk_sensor_control,
                                            GtkComboBoxText        *cbt)
{
  HyScanSensorPrm *sensor_prm;
  HyScanUDPPortPrm *prm;
  HyScanGtkSensorControlPrivate *priv;
  const gchar *active_id;
  HyScanDataSchemaEnumValue **values;

  priv = gtk_sensor_control->priv;

  if (priv->update)
    return;

  active_id = gtk_combo_box_get_active_id (GTK_COMBO_BOX (cbt));
  if (active_id == NULL)
    return;

  sensor_prm = g_hash_table_lookup (priv->sensors, priv->current_sensor);
  if (sensor_prm == NULL)
    return;

  prm = sensor_prm->port_prm;
  values = prm->addresses;

  while (*values != NULL)
    {
      HyScanDataSchemaEnumValue *ev = *values;
      if (g_str_equal (ev->name, active_id))
        {
          prm->addr = ev->value;
          break;
        }
      ++values;
    }

  hyscan_gtk_sensor_control_emit_changed (gtk_sensor_control);
}

static void
hyscan_gtk_sensor_control_enable_toggled (HyScanGtkSensorControl *gtk_sensor_control,
                                          GtkCheckButton         *checkbtn)
{
  HyScanSensorPrm *sensor_prm;
  HyScanGtkSensorControlPrivate *priv;
  gchar *sensor;

  priv = gtk_sensor_control->priv;

  if (priv->update)
    return;

  sensor_prm = g_hash_table_lookup (priv->sensors, priv->current_sensor);
  if (sensor_prm == NULL)
    return;

  sensor_prm->state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbtn));

  sensor = g_strdup (priv->current_sensor);
  hyscan_gtk_sensor_control_set_sensor (gtk_sensor_control, sensor);
  g_free (sensor);

  hyscan_gtk_sensor_control_emit_changed (gtk_sensor_control);
}

GtkWidget *
hyscan_gtk_sensor_control_new (void)
{
  return g_object_new (HYSCAN_TYPE_GTK_SENSOR_CONTROL, NULL);
}

/* Получает датчики. */
GHashTable *
hyscan_gtk_sensor_control_get_sensors (HyScanGtkSensorControl *gtk_sensor_control)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_SENSOR_CONTROL (gtk_sensor_control), NULL);
  return g_hash_table_ref (gtk_sensor_control->priv->sensors);
}

/* Обновляет состояние виджета — подтягивает внешние изменения из хэш-таблицы датчиков. */
void
hyscan_gtk_sensor_control_update (HyScanGtkSensorControl *gtk_sensor_control)
{
  HyScanGtkSensorControlPrivate *priv;
  GHashTableIter iter;
  gchar *sensor;
  gchar *active;
  GList *sensors;

  g_return_if_fail (HYSCAN_IS_GTK_SENSOR_CONTROL (gtk_sensor_control));

  priv = gtk_sensor_control->priv;

  if (priv->sensors == NULL)
    return;

  /* А есть ли датчики? */
  sensors = g_hash_table_get_keys (priv->sensors);
  if (sensors == NULL || !g_list_length (sensors))
    {
      gtk_widget_set_sensitive (priv->ports_cbt, FALSE);
      gtk_widget_set_sensitive (priv->enable_checkbtn, FALSE);
      gtk_widget_set_sensitive (priv->virtual_port_grid, FALSE);
      gtk_widget_set_sensitive (priv->udp_port_grid, FALSE);
      gtk_widget_set_sensitive (priv->uart_port_grid, FALSE);
      g_list_free (sensors);
      return;
    }
  gtk_widget_set_sensitive (priv->ports_cbt, TRUE);
  gtk_widget_set_sensitive (priv->enable_checkbtn, TRUE);
  gtk_widget_set_sensitive (priv->virtual_port_grid, TRUE);
  gtk_widget_set_sensitive (priv->udp_port_grid, TRUE);
  gtk_widget_set_sensitive (priv->uart_port_grid, TRUE);
  g_list_free (sensors);

  /* Текущий редактируемый датчик. */
  active = g_strdup (gtk_combo_box_get_active_id (GTK_COMBO_BOX (priv->ports_cbt)));

  gtk_label_set_text (GTK_LABEL (priv->port_type_name_label), "n/a");

  /* Очистка выпадающего списка датчиков. */
  gtk_combo_box_set_active (GTK_COMBO_BOX (priv->ports_cbt), -1);
  gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT (priv->ports_cbt));

  g_hash_table_iter_init (&iter, priv->sensors);
  while (g_hash_table_iter_next (&iter, (gpointer *) &sensor, NULL))
    {
      /* Заполнить список датчиков. */
      gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (priv->ports_cbt), sensor, sensor);
    }

  if (active != NULL)
    {
      hyscan_gtk_sensor_control_set_sensor (gtk_sensor_control, active);
      g_free (active);
    }
  else
    {
      gtk_combo_box_set_active (GTK_COMBO_BOX (priv->ports_cbt), 0);
    }
}
