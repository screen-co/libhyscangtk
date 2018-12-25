/**
 * \file hyscan-gtk-gen-control.c
 *
 * \brief Исходный файл виджета управления генератором.
 * \author Vladimir Maximov (vmakxs@gmail.com)
 * \date 2018
 * \license Проприетарная лицензия ООО "Экран"
 *
 */

#include "hyscan-gtk-gen-control.h"

enum
{
  PROP_O,
  PROP_CAPABILITIES,
  PROP_SIGNALS,
  PROP_PRESETS
};

enum
{
  SIGNAL_MODE_CHANGED,
  SIGNAL_PRESET_CHANGED,
  SIGNAL_AUTO_CHANGED,
  SIGNAL_SIMPLE_CHANGED,
  SIGNAL_EXTENDED_CHANGED,
  SIGNAL_LAST
};

static guint hyscan_gtk_gen_control_signals[SIGNAL_LAST] = {0};

struct _HyScanGtkGenControlPrivate
{
  /* Widgets. */
  GtkWidget                   *mode_cbt;
  GtkWidget                   *control_stack;

  GtkWidget                   *preset_grid;
  GtkWidget                   *auto_grid;
  GtkWidget                   *simple_grid;
  GtkWidget                   *extended_grid;

  GtkWidget                   *preset_cbt;
  GtkWidget                   *auto_signal_cbt;
  GtkWidget                   *simple_signal_cbt;
  GtkWidget                   *simple_power_spinbtn;
  GtkWidget                   *extended_signal_cbt;
  GtkWidget                   *extended_power_spinbtn;
  GtkWidget                   *extended_duration_spinbtn;

  GtkAdjustment               *simple_power_adj;
  GtkAdjustment               *extended_power_adj;
  GtkAdjustment               *extended_duration_adj;

  /* Capabilities. */
  HyScanSonarGeneratorModeType      capabilities;
  HyScanSonarGeneratorSignalType    signals;
  HyScanDataSchemaEnumValue  **presets;

  /* Helpers. */
  GHashTable                  *presets_dict;
  GHashTable                  *durations_dict;

  /* Generator data. */
  HyScanSonarGeneratorModeType      mode;
  guint                        preset;
  HyScanSonarGeneratorSignalType    signal_type;
  gdouble                      power;
  gdouble                      duration;
};

static void           hyscan_gtk_gen_control_set_property                (GObject                     *object,
                                                                          guint                        prop_id,
                                                                          const GValue                *value,
                                                                          GParamSpec                  *pspec);
static void           hyscan_gtk_gen_control_constructed                 (GObject                     *object);
static void           hyscan_gtk_gen_control_finalize                    (GObject                     *object);

static void           hyscan_gtk_gen_control_mode_changed                (HyScanGtkGenControl         *genc,
                                                                          GtkComboBoxText             *cbt);
static void           hyscan_gtk_gen_control_preset_changed              (HyScanGtkGenControl         *genc,
                                                                          GtkComboBoxText             *cbt);
static void           hyscan_gtk_gen_control_auto_signal_changed         (HyScanGtkGenControl         *genc,
                                                                          GtkComboBoxText             *cbt);
static void           hyscan_gtk_gen_control_simple_signal_changed       (HyScanGtkGenControl         *genc,
                                                                          GtkComboBoxText             *cbt);
static void           hyscan_gtk_gen_control_extended_signal_changed     (HyScanGtkGenControl         *genc,
                                                                          GtkComboBoxText             *cbt);
static void           hyscan_gtk_gen_control_simple_power_changed        (HyScanGtkGenControl         *genc,
                                                                          GtkAdjustment               *adj);
static void           hyscan_gtk_gen_control_extended_power_changed      (HyScanGtkGenControl         *genc,
                                                                          GtkAdjustment               *adj);
static void           hyscan_gtk_gen_control_extended_duration_changed   (HyScanGtkGenControl         *genc,
                                                                          GtkAdjustment               *adj);

static void           hyscan_gtk_gen_control_switch_stack                (HyScanGtkGenControl         *genc,
                                                                          HyScanSonarGeneratorModeType      mode);

static HyScanSonarGeneratorModeType
                      hyscan_gtk_gen_control_mode_by_id                  (const gchar                 *id);
static const gchar*   hyscan_gtk_gen_control_id_by_mode                  (HyScanSonarGeneratorModeType      mode);

static HyScanSonarGeneratorSignalType
                      hyscan_gtk_gen_control_signal_by_id                (const gchar                 *id);
static const gchar*   hyscan_gtk_gen_control_id_by_signal                (HyScanSonarGeneratorSignalType    mode);

static void           hyscan_gtk_gen_control_make_signals_cbt            (HyScanGtkGenControl         *genc,
                                                                          GtkComboBoxText             *cbt);

static void           hyscan_gtk_gen_control_free_preset                 (HyScanDataSchemaEnumValue   *preset);


G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkGenControl, hyscan_gtk_gen_control, GTK_TYPE_GRID)

static void
hyscan_gtk_gen_control_class_init (HyScanGtkGenControlClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = hyscan_gtk_gen_control_set_property;
  object_class->constructed = hyscan_gtk_gen_control_constructed;
  object_class->finalize = hyscan_gtk_gen_control_finalize;

  hyscan_gtk_gen_control_signals[SIGNAL_MODE_CHANGED] =
    g_signal_new ("mode-changed", G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  hyscan_gtk_gen_control_signals[SIGNAL_PRESET_CHANGED] =
    g_signal_new ("preset-changed", G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  hyscan_gtk_gen_control_signals[SIGNAL_AUTO_CHANGED] =
    g_signal_new ("auto-changed", G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  hyscan_gtk_gen_control_signals[SIGNAL_SIMPLE_CHANGED] =
    g_signal_new ("simple-changed", G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  hyscan_gtk_gen_control_signals[SIGNAL_EXTENDED_CHANGED] =
    g_signal_new ("ext-changed", G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  g_object_class_install_property (object_class,
                                   PROP_CAPABILITIES,
                                   g_param_spec_int ("capabilities",
                                                     "Capabilities",
                                                     "Capabilities",
                                                      0, G_MAXINT, 0,
                                                      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class,
                                   PROP_SIGNALS,
                                   g_param_spec_int ("signals",
                                                     "Signals",
                                                     "Signals",
                                                      0, G_MAXINT, 0,
                                                      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class,
                                   PROP_PRESETS,
                                   g_param_spec_pointer ("presets",
                                                         "Presets",
                                                         "Presets",
                                                          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  gtk_widget_class_set_template_from_resource (widget_class, "/org/hyscan/gtk/hyscan-gtk-gen-control.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkGenControl, mode_cbt);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkGenControl, control_stack);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkGenControl, preset_grid);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkGenControl, auto_grid);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkGenControl, simple_grid);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkGenControl, extended_grid);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkGenControl, preset_cbt);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkGenControl, auto_signal_cbt);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkGenControl, simple_signal_cbt);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkGenControl, simple_power_spinbtn);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkGenControl, extended_signal_cbt);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkGenControl, extended_power_spinbtn);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkGenControl, extended_duration_spinbtn);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkGenControl, simple_power_adj);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkGenControl, extended_power_adj);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkGenControl, extended_duration_adj);
  gtk_widget_class_bind_template_callback_full (widget_class, "mode_changed",
                                                G_CALLBACK (hyscan_gtk_gen_control_mode_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "preset_changed",
                                                G_CALLBACK (hyscan_gtk_gen_control_preset_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "auto_signal_changed",
                                                G_CALLBACK (hyscan_gtk_gen_control_auto_signal_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "simple_signal_changed",
                                                G_CALLBACK (hyscan_gtk_gen_control_simple_signal_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "extended_signal_changed",
                                                G_CALLBACK (hyscan_gtk_gen_control_extended_signal_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "simple_power_changed",
                                                G_CALLBACK (hyscan_gtk_gen_control_simple_power_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "extended_power_changed",
                                                G_CALLBACK (hyscan_gtk_gen_control_extended_power_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "extended_duration_changed",
                                                G_CALLBACK (hyscan_gtk_gen_control_extended_duration_changed));
}

static void
hyscan_gtk_gen_control_init (HyScanGtkGenControl *genc)
{
  genc->priv = hyscan_gtk_gen_control_get_instance_private (genc);
  gtk_widget_init_template (GTK_WIDGET (genc));
}

static void
hyscan_gtk_gen_control_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  HyScanGtkGenControl *genc = HYSCAN_GTK_GEN_CONTROL (object);
  HyScanGtkGenControlPrivate *priv = genc->priv;

  switch (prop_id)
    {
    case PROP_CAPABILITIES:
      priv->capabilities = g_value_get_int (value);
      break;

    case PROP_SIGNALS:
      priv->signals = g_value_get_int (value);
      break;

    case PROP_PRESETS:
      priv->presets = (HyScanDataSchemaEnumValue **) g_value_get_pointer (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
hyscan_gtk_gen_control_constructed (GObject *object)
{
  HyScanGtkGenControl *genc = HYSCAN_GTK_GEN_CONTROL (object);
  HyScanGtkGenControlPrivate *priv = genc->priv;

  G_OBJECT_CLASS (hyscan_gtk_gen_control_parent_class)->constructed (object);

  if (!priv->capabilities)
    {
      gtk_widget_set_sensitive (GTK_WIDGET (genc), FALSE);
      return;
    }

  if (priv->capabilities & HYSCAN_SONAR_GENERATOR_MODE_PRESET && priv->presets != NULL)
    {
      HyScanDataSchemaEnumValue **presets = priv->presets;

      gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (priv->mode_cbt),
                                 hyscan_gtk_gen_control_id_by_mode (HYSCAN_SONAR_GENERATOR_MODE_PRESET),
                                 "Presets");

      priv->presets_dict = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                  g_free, (GDestroyNotify) hyscan_gtk_gen_control_free_preset);

      /* Наполнение выпадающего списка преднастроек. */
      while (*presets != NULL)
        {
          HyScanDataSchemaEnumValue *enum_value = *presets;
          HyScanDataSchemaEnumValue *preset = g_new0 (HyScanDataSchemaEnumValue, 1);

          preset->value = enum_value->value;
          preset->name = g_strdup (enum_value->name);
          preset->description = g_strdup (enum_value->description);

          /* Ключ преднастройки - строковое представление значения преднастройки. */
          gchar *id = g_strdup_printf ("%u", (guint) preset->value);

          /* Добавить ассоциацию ключа и значения преднастройки. */
          g_hash_table_insert (priv->presets_dict, id, preset);

          /* Добавить преднастройку в выпадающий список преднастроек. */
          gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (priv->preset_cbt), id, preset->name);

          /* Переход к следующей преднастройке. */
          ++presets;
        }
    }
  else
    {
      gtk_container_remove (GTK_CONTAINER (priv->control_stack), priv->preset_grid);
    }

  if (priv->capabilities & HYSCAN_SONAR_GENERATOR_MODE_AUTO && priv->signals)
    {
      gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (priv->mode_cbt),
                                 hyscan_gtk_gen_control_id_by_mode (HYSCAN_SONAR_GENERATOR_MODE_AUTO),
                                 "Auto");

      /* Наполнение выпадающего списка сигналов. */
      hyscan_gtk_gen_control_make_signals_cbt (genc, GTK_COMBO_BOX_TEXT (priv->auto_signal_cbt));
    }
  else
    {
      gtk_container_remove (GTK_CONTAINER (priv->control_stack), priv->auto_grid);
    }

  if (priv->capabilities & HYSCAN_SONAR_GENERATOR_MODE_SIMPLE && priv->signals)
    {
      gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (priv->mode_cbt),
                                 hyscan_gtk_gen_control_id_by_mode (HYSCAN_SONAR_GENERATOR_MODE_SIMPLE),
                                 "Simple");

      /* Наполнение выпадающего списка сигналов. */
      hyscan_gtk_gen_control_make_signals_cbt (genc, GTK_COMBO_BOX_TEXT (priv->simple_signal_cbt));
    }
  else
    {
      gtk_container_remove (GTK_CONTAINER (priv->control_stack), priv->simple_grid);
    }

  if (priv->capabilities & HYSCAN_SONAR_GENERATOR_MODE_EXTENDED && priv->signals)
    {
      gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (priv->mode_cbt),
                                 hyscan_gtk_gen_control_id_by_mode (HYSCAN_SONAR_GENERATOR_MODE_EXTENDED),
                                 "Extended");

      /* Наполнение выпадающего списка сигналов. */
      hyscan_gtk_gen_control_make_signals_cbt (genc, GTK_COMBO_BOX_TEXT (priv->extended_signal_cbt));
    }
  else
    {
      gtk_container_remove (GTK_CONTAINER (priv->control_stack), priv->extended_grid);
    }
}

static void
hyscan_gtk_gen_control_finalize (GObject *object)
{
  HyScanGtkGenControl *genc = HYSCAN_GTK_GEN_CONTROL (object);
  HyScanGtkGenControlPrivate *priv = genc->priv;

  if (priv->presets_dict != NULL)
    g_hash_table_unref (priv->presets_dict);

  G_OBJECT_CLASS (hyscan_gtk_gen_control_parent_class)->finalize (object);
}

static void
hyscan_gtk_gen_control_mode_changed (HyScanGtkGenControl *genc,
                                     GtkComboBoxText     *cbt)
{
  const gchar *id;
  HyScanSonarGeneratorModeType mode;

  id = gtk_combo_box_get_active_id (GTK_COMBO_BOX (cbt));
  mode = hyscan_gtk_gen_control_mode_by_id (id);

  /* Смена стека осуществляется в любом случае. */
  hyscan_gtk_gen_control_switch_stack (genc, mode);

  /* Изменение режима только при необходимости. */
  if (genc->priv->mode != mode)
    {
      genc->priv->mode = mode;
      g_signal_emit (genc, hyscan_gtk_gen_control_signals[SIGNAL_MODE_CHANGED], 0, NULL);
    }
}

static void
hyscan_gtk_gen_control_preset_changed (HyScanGtkGenControl *genc,
                                       GtkComboBoxText     *cbt)
{
  HyScanGtkGenControlPrivate *priv = genc->priv;
  const gchar *id;
  HyScanDataSchemaEnumValue *preset;

  if ((id = gtk_combo_box_get_active_id (GTK_COMBO_BOX (cbt))) == NULL)
    return;

  if ((preset = g_hash_table_lookup (priv->presets_dict, id)) == NULL)
    {
      g_warning ("HyScanGtkGenControl: preset not found.");
    }

  /* Изменение преднастройки только при необходимости. */
  if (genc->priv->preset != preset->value)
    {
      genc->priv->preset = preset->value;
      g_signal_emit (genc, hyscan_gtk_gen_control_signals[SIGNAL_PRESET_CHANGED], 0, NULL);
    }
}

static void
hyscan_gtk_gen_control_auto_signal_changed (HyScanGtkGenControl *genc,
                                            GtkComboBoxText     *cbt)
{
  const gchar *id;
  HyScanSonarGeneratorSignalType signal_type;

  id = gtk_combo_box_get_active_id (GTK_COMBO_BOX (cbt));
  signal_type = hyscan_gtk_gen_control_signal_by_id (id);

  /* Изменение режима только при необходимости. */
  if (genc->priv->signal_type != signal_type)
    {
      hyscan_gtk_gen_control_set_signal (genc, signal_type);
      g_signal_emit (genc, hyscan_gtk_gen_control_signals[SIGNAL_AUTO_CHANGED], 0, NULL);
    }
}

static void
hyscan_gtk_gen_control_simple_signal_changed (HyScanGtkGenControl *genc,
                                              GtkComboBoxText     *cbt)
{
  const gchar *id;
  HyScanSonarGeneratorSignalType signal_type;

  id = gtk_combo_box_get_active_id (GTK_COMBO_BOX (cbt));
  signal_type = hyscan_gtk_gen_control_signal_by_id (id);

  /* Изменение режима только при необходимости. */
  if (genc->priv->signal_type != signal_type)
    {
      hyscan_gtk_gen_control_set_signal (genc, signal_type);
      g_signal_emit (genc, hyscan_gtk_gen_control_signals[SIGNAL_SIMPLE_CHANGED], 0, NULL);
    }
}

static void
hyscan_gtk_gen_control_extended_signal_changed (HyScanGtkGenControl *genc,
                                                GtkComboBoxText     *cbt)
{
  const gchar *id;
  HyScanSonarGeneratorSignalType signal_type;

  id = gtk_combo_box_get_active_id (GTK_COMBO_BOX (cbt));
  signal_type = hyscan_gtk_gen_control_signal_by_id (id);

  /* Изменение режима только при необходимости. */
  if (genc->priv->signal_type != signal_type)
    {
      hyscan_gtk_gen_control_set_signal (genc, signal_type);
      g_signal_emit (genc, hyscan_gtk_gen_control_signals[SIGNAL_EXTENDED_CHANGED], 0, NULL);
    }
}

static void
hyscan_gtk_gen_control_simple_power_changed (HyScanGtkGenControl *genc,
                                             GtkAdjustment       *adj)
{
  g_warning ("Not implemented");
}

static void
hyscan_gtk_gen_control_extended_power_changed (HyScanGtkGenControl *genc,
                                               GtkAdjustment       *adj)
{
  g_warning ("Not implemented");
}

static void
hyscan_gtk_gen_control_extended_duration_changed (HyScanGtkGenControl *genc,
                                                  GtkAdjustment       *adj)
{
  g_warning ("Not implemented");
}

static void
hyscan_gtk_gen_control_switch_stack (HyScanGtkGenControl     *genc,
                                     HyScanSonarGeneratorModeType  mode)
{
  HyScanGtkGenControlPrivate *priv = genc->priv;

  switch (mode)
    {
    case HYSCAN_SONAR_GENERATOR_MODE_PRESET:
      gtk_stack_set_visible_child (GTK_STACK (priv->control_stack), priv->preset_grid);
      break;
    case HYSCAN_SONAR_GENERATOR_MODE_AUTO:
      gtk_stack_set_visible_child (GTK_STACK (priv->control_stack), priv->auto_grid);
      break;
    case HYSCAN_SONAR_GENERATOR_MODE_SIMPLE:
      gtk_stack_set_visible_child (GTK_STACK (priv->control_stack), priv->simple_grid);
      break;
    case HYSCAN_SONAR_GENERATOR_MODE_EXTENDED:
      gtk_stack_set_visible_child (GTK_STACK (priv->control_stack), priv->extended_grid);
      break;
    default:
      return;
    }
}

static HyScanSonarGeneratorModeType
hyscan_gtk_gen_control_mode_by_id (const gchar *id)
{
  HyScanSonarGeneratorModeType mode;

  if (id == NULL)
    return HYSCAN_SONAR_GENERATOR_MODE_NONE;

  if (g_str_equal (id, "preset"))
    mode = HYSCAN_SONAR_GENERATOR_MODE_PRESET;
  else if (g_str_equal (id, "auto"))
    mode = HYSCAN_SONAR_GENERATOR_MODE_AUTO;
  else if (g_str_equal (id, "simple"))
    mode = HYSCAN_SONAR_GENERATOR_MODE_SIMPLE;
  else if (g_str_equal (id, "extended"))
    mode = HYSCAN_SONAR_GENERATOR_MODE_EXTENDED;
  else
    mode = HYSCAN_SONAR_GENERATOR_MODE_NONE;

  return mode;
}

static const gchar*
hyscan_gtk_gen_control_id_by_mode (HyScanSonarGeneratorModeType mode)
{
  const gchar *id;

  switch (mode)
    {
    case HYSCAN_SONAR_GENERATOR_MODE_PRESET:
      id = "preset";
      break;
    case HYSCAN_SONAR_GENERATOR_MODE_AUTO:
      id = "auto";
      break;
    case HYSCAN_SONAR_GENERATOR_MODE_SIMPLE:
      id = "simple";
      break;
    case HYSCAN_SONAR_GENERATOR_MODE_EXTENDED:
      id = "extended";
      break;
    default:
      id = NULL;
      break;
    }

  return id;
}

static HyScanSonarGeneratorSignalType
hyscan_gtk_gen_control_signal_by_id (const gchar *id)
{
  HyScanSonarGeneratorSignalType signal_type;

  if (id == NULL)
    return HYSCAN_SONAR_GENERATOR_SIGNAL_NONE;

  if (g_str_equal (id, "auto"))
    signal_type = HYSCAN_SONAR_GENERATOR_SIGNAL_AUTO;
  else if (g_str_equal (id, "tone"))
    signal_type = HYSCAN_SONAR_GENERATOR_SIGNAL_TONE;
  else if (g_str_equal (id, "lfm"))
    signal_type = HYSCAN_SONAR_GENERATOR_SIGNAL_LFM;
  else
    signal_type = HYSCAN_SONAR_GENERATOR_SIGNAL_NONE;

  return signal_type;
}

static const gchar*
hyscan_gtk_gen_control_id_by_signal (HyScanSonarGeneratorSignalType signal_type)
{
  const gchar *id;

  switch (signal_type)
    {
    case HYSCAN_SONAR_GENERATOR_SIGNAL_AUTO:
      id = "auto";
      break;
    case HYSCAN_SONAR_GENERATOR_SIGNAL_TONE:
      id = "tone";
      break;
    case HYSCAN_SONAR_GENERATOR_SIGNAL_LFM:
      id = "lfm";
      break;
    default:
      id = NULL;
      break;
    }

  return id;
}

static void
hyscan_gtk_gen_control_make_signals_cbt (HyScanGtkGenControl *genc,
                                         GtkComboBoxText     *cbt)
{
  HyScanGtkGenControlPrivate *priv = genc->priv;

  if (priv->signals & HYSCAN_SONAR_GENERATOR_SIGNAL_AUTO)
    {
      gtk_combo_box_text_append (cbt,
                                 hyscan_gtk_gen_control_id_by_signal (HYSCAN_SONAR_GENERATOR_SIGNAL_AUTO),
                                 "Auto");
    }

  if (priv->signals & HYSCAN_SONAR_GENERATOR_SIGNAL_TONE)
    {
      gtk_combo_box_text_append (cbt,
                                 hyscan_gtk_gen_control_id_by_signal (HYSCAN_SONAR_GENERATOR_SIGNAL_TONE),
                                 "Tone");
    }

   if (priv->signals & HYSCAN_SONAR_GENERATOR_SIGNAL_LFM)
    {
      gtk_combo_box_text_append (cbt,
                                 hyscan_gtk_gen_control_id_by_signal (HYSCAN_SONAR_GENERATOR_SIGNAL_LFM),
                                 "LFM");
    }
}

static void
hyscan_gtk_gen_control_free_preset (HyScanDataSchemaEnumValue *preset)
{
  g_free (preset->name);
  g_free (preset->description);
  g_free (preset);
}

GtkWidget *
hyscan_gtk_gen_control_new (HyScanSonarGeneratorModeType     capabilities,
                            HyScanSonarGeneratorSignalType   signals,
                            HyScanDataSchemaEnumValue **presets)
{
  return g_object_new (HYSCAN_TYPE_GTK_GEN_CONTROL,
                       "capabilities", capabilities,
                       "signals", signals,
                       "presets", presets,
                       NULL);
}

void
hyscan_gtk_gen_control_set_durations (HyScanGtkGenControl *genc,
                                      GList               *durations)
{
  g_return_if_fail (HYSCAN_IS_GTK_GEN_CONTROL (genc));

  g_warning ("Not implemented");
}

void
hyscan_gtk_gen_control_set_mode (HyScanGtkGenControl     *genc,
                                 HyScanSonarGeneratorModeType  mode)
{
  HyScanGtkGenControlPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_GEN_CONTROL (genc));

  priv = genc->priv;

  if (priv->capabilities & mode)
    {
      if (priv->mode != mode)
        {
          priv->mode = mode;
          gtk_combo_box_set_active_id (GTK_COMBO_BOX (priv->mode_cbt), hyscan_gtk_gen_control_id_by_mode (priv->mode));
        }
    }
  else
    {
      g_warning ("HyScanGtkGenControl: wrong mode type.");
    }
}

void
hyscan_gtk_gen_control_set_preset (HyScanGtkGenControl *genc,
                                   guint                preset)
{
  HyScanGtkGenControlPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_GEN_CONTROL (genc));

  priv = genc->priv;

  if (priv->preset != preset)
    {
      guint cur_preset;
      gchar *preset_id;

      /* Нужно запомнить текущий пресет и установить пользовательский,
         так как изменение положения комбинированного списка приведет
         к испусканию сигнала changed, что недопустимо при внешней
         установке значения преднастройки. */
      cur_preset = priv->preset;
      priv->preset = preset;

      preset_id = g_strdup_printf ("%u", preset);

      if (!gtk_combo_box_set_active_id (GTK_COMBO_BOX (priv->preset_cbt), preset_id))
        {
          priv->preset = preset;
          g_warning ("HyScanGtkGenControl: wrong preset.");
        }

      g_free (preset_id);
    }
}

void
hyscan_gtk_gen_control_set_signal (HyScanGtkGenControl       *genc,
                                   HyScanSonarGeneratorSignalType  signal_type)
{
  HyScanGtkGenControlPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_GEN_CONTROL (genc));

  priv = genc->priv;

  if (priv->signals & signal_type)
    {
      if (priv->signal_type != signal_type)
        {
          priv->signal_type = signal_type;

          gtk_combo_box_set_active_id (GTK_COMBO_BOX (priv->auto_signal_cbt),
                                       hyscan_gtk_gen_control_id_by_signal (priv->signal_type));
          gtk_combo_box_set_active_id (GTK_COMBO_BOX (priv->simple_signal_cbt),
                                       hyscan_gtk_gen_control_id_by_signal (priv->signal_type));
          gtk_combo_box_set_active_id (GTK_COMBO_BOX (priv->extended_signal_cbt),
                                       hyscan_gtk_gen_control_id_by_signal (priv->signal_type));
        }
    }
  else
    {
      g_warning ("HyScanGtkGenControl: wrong signal type.");
    }
}

void
hyscan_gtk_gen_control_set_power (HyScanGtkGenControl *genc,
                                  gdouble              power)
{
  g_return_if_fail (HYSCAN_IS_GTK_GEN_CONTROL (genc));

  g_warning ("Not implemented");
}

void
hyscan_gtk_gen_control_set_duration (HyScanGtkGenControl *genc,
                                     gdouble              duration)
{
  g_return_if_fail (HYSCAN_IS_GTK_GEN_CONTROL (genc));

  g_warning ("Not implemented");
}

HyScanSonarGeneratorModeType
hyscan_gtk_gen_control_get_mode (HyScanGtkGenControl *genc)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_GEN_CONTROL (genc), HYSCAN_SONAR_GENERATOR_MODE_NONE);

  return genc->priv->mode;
}

gint64
hyscan_gtk_gen_control_get_preset (HyScanGtkGenControl *genc)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_GEN_CONTROL (genc), -1);

  return genc->priv->preset;
}

HyScanSonarGeneratorSignalType
hyscan_gtk_gen_control_get_signal (HyScanGtkGenControl *genc)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_GEN_CONTROL (genc), HYSCAN_SONAR_GENERATOR_SIGNAL_NONE);

  return genc->priv->signal_type;
}

gdouble
hyscan_gtk_gen_control_get_power (HyScanGtkGenControl *genc)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_GEN_CONTROL (genc), 0.0);

  g_warning ("Not implemented");

  return 0.0;
}

gdouble
hyscan_gtk_gen_control_get_duration (HyScanGtkGenControl *genc)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_GEN_CONTROL (genc), 0.0);

  g_warning ("Not implemented");

  return 0.0;
}

