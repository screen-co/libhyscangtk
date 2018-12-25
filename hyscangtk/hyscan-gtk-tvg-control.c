/**
 * \file hyscan-gtk-tvg-control.c
 *
 * \brief Виджет управления системой ВАРУ.
 * \author Vladimir Maximov (vmakxs@gmail.com)
 * \date 2018
 * \license Проприетарная лицензия ООО "Экран"
 *
 */

#include "hyscan-gtk-tvg-control.h"

enum
{
  PROP_0,
  PROP_CAPABITILIES
};

enum
{
  SIGNAL_CHANGED,
  SIGNAL_LAST
};

static guint hyscan_gtk_tvg_control_signals[SIGNAL_LAST] = {0};

struct _HyScanGtkTVGControlPrivate
{
  /* Widgets. */
  GtkAdjustment          *lin_gain_adj;
  GtkAdjustment          *lin_step_adj;

  GtkAdjustment          *log_gain_adj;
  GtkAdjustment          *log_alpha_adj;
  GtkAdjustment          *log_beta_adj;

  GtkAdjustment          *level_adj;
  GtkAdjustment          *sensitivity_adj;

  GtkWidget              *tvg_mode_cbt;
  GtkWidget              *tvg_control_stack;
  GtkWidget              *auto_tvg_grid;
  GtkWidget              *lin_tvg_grid;
  GtkWidget              *log_tvg_grid;

  /* TVG capabilities. */
  gint                    capabilities;

  /* TVG data. */
  gdouble                 log_gain;
  gdouble                 log_beta;
  gdouble                 log_alpha;

  gdouble                 lin_gain;
  gdouble                 lin_step;

  gdouble                 level;
  gdouble                 sensitivity;

  HyScanSonarTVGModeType       tvg_mode;
};

static void                hyscan_gtk_tvg_control_set_property          (GObject               *object,
                                                                         guint                  prop_id,
                                                                         const GValue          *value,
                                                                         GParamSpec            *pspec);
static void                hyscan_gtk_tvg_control_constructed           (GObject               *object);
static void                hyscan_gtk_tvg_control_finalize              (GObject               *object);

static void                hyscan_gtk_tvg_control_tvg_mode_changed      (HyScanGtkTVGControl   *tvgc,
                                                                         GtkComboBoxText       *cbt);
static void                hyscan_gtk_tvg_control_level_changed         (HyScanGtkTVGControl   *tvgc,
                                                                         GtkAdjustment         *adj);
static void                hyscan_gtk_tvg_control_sensitivity_changed   (HyScanGtkTVGControl   *tvgc,
                                                                         GtkAdjustment         *adj);
static void                hyscan_gtk_tvg_control_lin_gain_changed      (HyScanGtkTVGControl   *tvgc,
                                                                         GtkAdjustment         *adj);
static void                hyscan_gtk_tvg_control_lin_step_changed      (HyScanGtkTVGControl   *tvgc,
                                                                         GtkAdjustment         *adj);
static void                hyscan_gtk_tvg_control_log_gain_changed      (HyScanGtkTVGControl   *tvgc,
                                                                         GtkAdjustment         *adj);
static void                hyscan_gtk_tvg_control_log_beta_changed      (HyScanGtkTVGControl   *tvgc,
                                                                         GtkAdjustment         *adj);
static void                hyscan_gtk_tvg_control_log_alpha_changed     (HyScanGtkTVGControl   *tvgc,
                                                                         GtkAdjustment         *adj);

static void                hyscan_gtk_tvg_control_switch_stack          (HyScanGtkTVGControl   *tvgc,
                                                                         HyScanSonarTVGModeType      mode);

static HyScanSonarTVGModeType   hyscan_gtk_tvg_control_mode_by_id            (const gchar           *id);
static const gchar*        hyscan_gtk_tvg_control_id_by_mode            (HyScanSonarTVGModeType      mode);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkTVGControl, hyscan_gtk_tvg_control, GTK_TYPE_GRID)

static void
hyscan_gtk_tvg_control_class_init (HyScanGtkTVGControlClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = hyscan_gtk_tvg_control_set_property;
  object_class->constructed = hyscan_gtk_tvg_control_constructed;
  object_class->finalize = hyscan_gtk_tvg_control_finalize;

  hyscan_gtk_tvg_control_signals[SIGNAL_CHANGED] =
    g_signal_new ("changed", G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  g_object_class_install_property (object_class,
                                   PROP_CAPABITILIES,
                                   g_param_spec_int ("capabilities",
                                                     "Capabilities",
                                                     "Capabilities",
                                                      0, G_MAXINT, 0,
                                                      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  gtk_widget_class_set_template_from_resource (widget_class, "/org/hyscan/gtk/hyscan-gtk-tvg-control.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkTVGControl, lin_gain_adj);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkTVGControl, lin_step_adj);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkTVGControl, log_gain_adj);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkTVGControl, log_beta_adj);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkTVGControl, log_alpha_adj);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkTVGControl, level_adj);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkTVGControl, sensitivity_adj);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkTVGControl, tvg_mode_cbt);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkTVGControl, tvg_control_stack);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkTVGControl, auto_tvg_grid);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkTVGControl, lin_tvg_grid);
  gtk_widget_class_bind_template_child_private (widget_class, HyScanGtkTVGControl, log_tvg_grid);

  gtk_widget_class_bind_template_callback_full (widget_class, "tvg_mode_changed",
                                                G_CALLBACK (hyscan_gtk_tvg_control_tvg_mode_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "level_changed",
                                                G_CALLBACK (hyscan_gtk_tvg_control_level_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "sensitivity_changed",
                                                G_CALLBACK (hyscan_gtk_tvg_control_sensitivity_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "lin_gain_changed",
                                                G_CALLBACK (hyscan_gtk_tvg_control_lin_gain_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "lin_step_changed",
                                                G_CALLBACK (hyscan_gtk_tvg_control_lin_step_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "log_gain_changed",
                                                G_CALLBACK (hyscan_gtk_tvg_control_log_gain_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "log_beta_changed",
                                                G_CALLBACK (hyscan_gtk_tvg_control_log_beta_changed));
  gtk_widget_class_bind_template_callback_full (widget_class, "log_alpha_changed",
                                                G_CALLBACK (hyscan_gtk_tvg_control_log_alpha_changed));
}

static void
hyscan_gtk_tvg_control_init (HyScanGtkTVGControl *tvgc)
{
  tvgc->priv = hyscan_gtk_tvg_control_get_instance_private (tvgc);
  gtk_widget_init_template (GTK_WIDGET (tvgc));
}

static void
hyscan_gtk_tvg_control_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  HyScanGtkTVGControlPrivate *priv = HYSCAN_GTK_TVG_CONTROL (object)->priv;

  switch (prop_id)
    {
    case PROP_CAPABITILIES:
      priv->capabilities = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
hyscan_gtk_tvg_control_constructed (GObject *object)
{
  HyScanGtkTVGControl *tvgc = HYSCAN_GTK_TVG_CONTROL (object);
  HyScanGtkTVGControlPrivate *priv = tvgc->priv;

  G_OBJECT_CLASS (hyscan_gtk_tvg_control_parent_class)->constructed (object);

  if (priv->capabilities)
    {
      if (priv->capabilities & HYSCAN_SONAR_TVG_MODE_AUTO)
        {
          gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (priv->tvg_mode_cbt),
                                     hyscan_gtk_tvg_control_id_by_mode (HYSCAN_SONAR_TVG_MODE_AUTO),
                                     "Auto");
        }
      else
        {
          gtk_container_remove (GTK_CONTAINER (priv->tvg_control_stack), priv->auto_tvg_grid);
        }

      if (priv->capabilities & HYSCAN_SONAR_TVG_MODE_LINEAR_DB)
        {
          gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (priv->tvg_mode_cbt),
                                     hyscan_gtk_tvg_control_id_by_mode (HYSCAN_SONAR_TVG_MODE_LINEAR_DB),
                                     "Linear DB");
        }
      else
        {
          gtk_container_remove (GTK_CONTAINER (priv->tvg_control_stack), priv->lin_tvg_grid);
        }

      if (priv->capabilities & HYSCAN_SONAR_TVG_MODE_LOGARITHMIC)
        {
          gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (priv->tvg_mode_cbt),
                                     hyscan_gtk_tvg_control_id_by_mode (HYSCAN_SONAR_TVG_MODE_LOGARITHMIC),
                                     "Logarithmic");
        }
      else
        {
          gtk_container_remove (GTK_CONTAINER (priv->tvg_control_stack), priv->log_tvg_grid);
        }
    }
  else
    {
      gtk_widget_set_sensitive (GTK_WIDGET (tvgc), FALSE);
    }
}

static void
hyscan_gtk_tvg_control_finalize (GObject *object)
{
  G_OBJECT_CLASS (hyscan_gtk_tvg_control_parent_class)->finalize (object);
}

static void
hyscan_gtk_tvg_control_tvg_mode_changed (HyScanGtkTVGControl *tvgc,
                                         GtkComboBoxText     *cbt)
{
  const gchar *id;
  HyScanSonarTVGModeType mode;

  id = gtk_combo_box_get_active_id (GTK_COMBO_BOX (cbt));
  mode = hyscan_gtk_tvg_control_mode_by_id (id);

  hyscan_gtk_tvg_control_switch_stack (tvgc, mode);

  if (tvgc->priv->tvg_mode != mode)
    {
      tvgc->priv->tvg_mode = mode;
      g_signal_emit (tvgc, hyscan_gtk_tvg_control_signals[SIGNAL_CHANGED], 0, NULL);
    }
}

static void
hyscan_gtk_tvg_control_level_changed (HyScanGtkTVGControl *tvgc,
                                      GtkAdjustment       *adj)
{
  gdouble level = gtk_adjustment_get_value (adj);
  if (tvgc->priv->level != level)
    {
      tvgc->priv->level = level;
      g_signal_emit(tvgc, hyscan_gtk_tvg_control_signals[SIGNAL_CHANGED], 0, NULL);
    }
}

static void
hyscan_gtk_tvg_control_sensitivity_changed (HyScanGtkTVGControl *tvgc,
                                            GtkAdjustment       *adj)
{
  gdouble sensitivity = gtk_adjustment_get_value (adj);
  if (tvgc->priv->sensitivity != sensitivity)
    {
      tvgc->priv->sensitivity = sensitivity;
      g_signal_emit(tvgc, hyscan_gtk_tvg_control_signals[SIGNAL_CHANGED], 0, NULL);
    }
}

static void
hyscan_gtk_tvg_control_lin_gain_changed (HyScanGtkTVGControl *tvgc,
                                         GtkAdjustment       *adj)
{
  gdouble lin_gain = gtk_adjustment_get_value (adj);
  if (tvgc->priv->lin_gain != lin_gain)
    {
      tvgc->priv->lin_gain = lin_gain;
      g_signal_emit(tvgc, hyscan_gtk_tvg_control_signals[SIGNAL_CHANGED], 0, NULL);
    }
}

static void
hyscan_gtk_tvg_control_lin_step_changed (HyScanGtkTVGControl *tvgc,
                                         GtkAdjustment       *adj)
{
  gdouble lin_step = gtk_adjustment_get_value (adj);
  if (tvgc->priv->lin_step != lin_step)
    {
      tvgc->priv->lin_step = lin_step;
      g_signal_emit(tvgc, hyscan_gtk_tvg_control_signals[SIGNAL_CHANGED], 0, NULL);
    }
}

static void
hyscan_gtk_tvg_control_log_gain_changed (HyScanGtkTVGControl *tvgc,
                                         GtkAdjustment       *adj)
{
  gdouble log_gain = gtk_adjustment_get_value (adj);
  if (tvgc->priv->log_gain != log_gain)
    {
      tvgc->priv->log_gain = log_gain;
      g_signal_emit(tvgc, hyscan_gtk_tvg_control_signals[SIGNAL_CHANGED], 0, NULL);
    }
}

static void
hyscan_gtk_tvg_control_log_beta_changed (HyScanGtkTVGControl *tvgc,
                                         GtkAdjustment       *adj)
{
  gdouble log_beta = gtk_adjustment_get_value (adj);
  if (tvgc->priv->log_beta != log_beta)
    {
      tvgc->priv->log_beta = log_beta;
      g_signal_emit(tvgc, hyscan_gtk_tvg_control_signals[SIGNAL_CHANGED], 0, NULL);
    }
}

static void
hyscan_gtk_tvg_control_log_alpha_changed (HyScanGtkTVGControl *tvgc,
                                          GtkAdjustment       *adj)
{
  gdouble log_alpha = gtk_adjustment_get_value (adj);
  if (tvgc->priv->log_alpha != log_alpha)
    {
      tvgc->priv->log_alpha = log_alpha;
      g_signal_emit(tvgc, hyscan_gtk_tvg_control_signals[SIGNAL_CHANGED], 0, NULL);
    }
}

static void
hyscan_gtk_tvg_control_switch_stack (HyScanGtkTVGControl *tvgc,
                                     HyScanSonarTVGModeType    mode)
{
  HyScanGtkTVGControlPrivate *priv = tvgc->priv;

  switch (mode)
  {
  case HYSCAN_SONAR_TVG_MODE_AUTO:
    gtk_stack_set_visible_child (GTK_STACK (priv->tvg_control_stack), priv->auto_tvg_grid);
    break;
  case HYSCAN_SONAR_TVG_MODE_LINEAR_DB:
    gtk_stack_set_visible_child (GTK_STACK (priv->tvg_control_stack), priv->lin_tvg_grid);
    break;
  case HYSCAN_SONAR_TVG_MODE_LOGARITHMIC:
    gtk_stack_set_visible_child (GTK_STACK (priv->tvg_control_stack), priv->log_tvg_grid);
    break;
  default:
    g_warning ("HyScanGtkTVGControl: wrong TVG mode.");
  }
}


static HyScanSonarTVGModeType
hyscan_gtk_tvg_control_mode_by_id (const gchar *id)
{
  if (id == NULL)
    return HYSCAN_SONAR_TVG_MODE_NONE;

  if (g_str_equal (id, "auto"))
    return HYSCAN_SONAR_TVG_MODE_AUTO;
  else if (g_str_equal (id, "lin"))
    return HYSCAN_SONAR_TVG_MODE_LINEAR_DB;
  else if (g_str_equal (id, "log"))
    return HYSCAN_SONAR_TVG_MODE_LOGARITHMIC;

  return HYSCAN_SONAR_TVG_MODE_NONE;
}

static const gchar*
hyscan_gtk_tvg_control_id_by_mode (HyScanSonarTVGModeType mode)
{
  const gchar *id;

  switch (mode)
    {
    case HYSCAN_SONAR_TVG_MODE_AUTO:
      id = "auto";
      break;
    case HYSCAN_SONAR_TVG_MODE_LINEAR_DB:
      id = "lin";
      break;
    case HYSCAN_SONAR_TVG_MODE_LOGARITHMIC:
      id = "log";
      break;
    default:
      id = NULL;
    }

  return id;
}

GtkWidget *
hyscan_gtk_tvg_control_new (HyScanSonarTVGModeType capabilities)
{
  return g_object_new (HYSCAN_TYPE_GTK_TVG_CONTROL,
                       "capabilities", (int) capabilities,
                       NULL);
}

void
hyscan_gtk_tvg_control_set_mode (HyScanGtkTVGControl *tvgc,
                                 HyScanSonarTVGModeType    mode)
{
  HyScanGtkTVGControlPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc));

  priv = tvgc->priv;

  if (priv->capabilities & mode)
    {
      if (priv->tvg_mode != mode)
        {
          priv->tvg_mode = mode;
          gtk_combo_box_set_active_id (GTK_COMBO_BOX (priv->tvg_mode_cbt),
                                       hyscan_gtk_tvg_control_id_by_mode (mode));
        }
    }
  else
    {
      g_warning ("HyScanGtkTVGControl: wrong TVG mode type.");
    }
}

void
hyscan_gtk_tvg_control_set_gain_range (HyScanGtkTVGControl *tvgc,
                                       gdouble              gain_min,
                                       gdouble              gain_max)
{
  HyScanGtkTVGControlPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc));

  priv = tvgc->priv;

  /* Linear DB: lower and upper gains. */
  gtk_adjustment_set_lower (priv->lin_gain_adj, gain_min);
  gtk_adjustment_set_upper (priv->lin_gain_adj, gain_max);

  /* Logarithmic: lower and upper gains. */
  gtk_adjustment_set_lower (priv->log_gain_adj, gain_min);
  gtk_adjustment_set_upper (priv->log_gain_adj, gain_max);
}

void
hyscan_gtk_tvg_control_set_level (HyScanGtkTVGControl *tvgc,
                                  gdouble              level)
{
  HyScanGtkTVGControlPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc));

  priv = tvgc->priv;

  if (priv->level != level)
    {
      priv->level = level;
      gtk_adjustment_set_value (priv->level_adj, level);
    }
}

void
hyscan_gtk_tvg_control_set_sensitivity (HyScanGtkTVGControl *tvgc,
                                        gdouble              sensitivity)
{
  HyScanGtkTVGControlPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc));

  priv = tvgc->priv;

  if (priv->sensitivity != sensitivity)
    {
      priv->sensitivity = sensitivity;
      gtk_adjustment_set_value (priv->sensitivity_adj, sensitivity);
    }
}

void
hyscan_gtk_tvg_control_set_linear_db_gain (HyScanGtkTVGControl *tvgc,
                                           gdouble              gain)
{
  HyScanGtkTVGControlPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc));

  priv = tvgc->priv;

  if (priv->lin_gain != gain)
    {
      priv->lin_gain = gain;
      gtk_adjustment_set_value (priv->lin_gain_adj, gain);
    }
}

void
hyscan_gtk_tvg_control_set_linear_db_step (HyScanGtkTVGControl *tvgc,
                                           gdouble              step)
{
  HyScanGtkTVGControlPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc));

  priv = tvgc->priv;

  if (priv->lin_step != step)
    {
      priv->lin_step = step;
      gtk_adjustment_set_value (priv->lin_step_adj, step);
    }
}

void
hyscan_gtk_tvg_control_set_logarithmic_gain (HyScanGtkTVGControl *tvgc,
                                             gdouble              gain)
{
  HyScanGtkTVGControlPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc));

  priv = tvgc->priv;

  if (priv->log_gain != gain)
    {
      priv->log_gain = gain;
      gtk_adjustment_set_value (priv->log_gain_adj, gain);
    }
}

void
hyscan_gtk_tvg_control_set_logarithmic_beta (HyScanGtkTVGControl *tvgc,
                                             gdouble              beta)
{
  HyScanGtkTVGControlPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc));

  priv = tvgc->priv;

  if (priv->log_beta != beta)
    {
      priv->log_beta = beta;
      gtk_adjustment_set_value (priv->log_beta_adj, beta);
    }
}

void
hyscan_gtk_tvg_control_set_logarithmic_alpha (HyScanGtkTVGControl *tvgc,
                                              gdouble              alpha)
{
  HyScanGtkTVGControlPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc));

  priv = tvgc->priv;

  if (priv->log_alpha != alpha)
    {
      priv->log_alpha = alpha;
      gtk_adjustment_set_value (priv->log_alpha_adj, alpha);
    }
}


HyScanSonarTVGModeType
hyscan_gtk_tvg_control_get_mode (HyScanGtkTVGControl *tvgc)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc), HYSCAN_SONAR_TVG_MODE_NONE);

  return tvgc->priv->tvg_mode;
}

gdouble
hyscan_gtk_tvg_control_get_level (HyScanGtkTVGControl *tvgc)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc), -G_MAXDOUBLE);

  return tvgc->priv->level;
}

gdouble
hyscan_gtk_tvg_control_get_sensitivity (HyScanGtkTVGControl *tvgc)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc), -G_MAXDOUBLE);

  return tvgc->priv->sensitivity;
}

gdouble
hyscan_gtk_tvg_control_get_linear_db_gain (HyScanGtkTVGControl *tvgc)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc), -G_MAXDOUBLE);

  return tvgc->priv->lin_gain;
}

gdouble
hyscan_gtk_tvg_control_get_linear_db_step (HyScanGtkTVGControl *tvgc)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc), -G_MAXDOUBLE);

  return tvgc->priv->lin_step;
}

gdouble
hyscan_gtk_tvg_control_get_logarithmic_gain (HyScanGtkTVGControl *tvgc)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc), -G_MAXDOUBLE);

  return tvgc->priv->log_gain;
}

gdouble
hyscan_gtk_tvg_control_get_logarithmic_beta (HyScanGtkTVGControl *tvgc)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc), -G_MAXDOUBLE);

  return tvgc->priv->log_beta;
}

gdouble
hyscan_gtk_tvg_control_get_logarithmic_alpha (HyScanGtkTVGControl *tvgc)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_TVG_CONTROL (tvgc), -G_MAXDOUBLE);

  return tvgc->priv->log_alpha;
}

