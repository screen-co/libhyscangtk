/* hyscan-gtk-datetime.c
 *
 * Copyright 2018 Screen LLC, Alexander Dmitriev <m1n7@yandex.ru>
 *
 * This file is part of HyScanGtk.
 *
 * HyScanGtk is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HyScanGtk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Alternatively, you can license this code under a commercial license.
 * Contact the Screen LLC in this case - <info@screen-co.ru>.
 */

/* HyScanGtk имеет двойную лицензию.
 *
 * Во-первых, вы можете распространять HyScanGtk на условиях Стандартной
 * Общественной Лицензии GNU версии 3, либо по любой более поздней версии
 * лицензии (по вашему выбору). Полные положения лицензии GNU приведены в
 * <http://www.gnu.org/licenses/>.
 *
 * Во-вторых, этот программный код можно использовать по коммерческой
 * лицензии. Для этого свяжитесь с ООО Экран - <info@screen-co.ru>.
 */

/**
 * SECTION: hyscan-gtk-datetime
 * @Title: HyScanGtkDatetime
 * @Short_description: виджет даты и времени
 *
 * Виджет отображает заданное время и дату или что-то одно.
 * Временная зона всегда UTC.
 */
#include "hyscan-gtk-datetime.h"
#include <glib/gprintf.h>

#define WIDGET_RESOURCE_UI "/org/hyscan/gtk/hyscan-gtk-datetime.ui"

enum
{
  PROP_0,
  PROP_MODE,
  PROP_TIME
};

struct _HyScanGtkDateTimePrivate
{
  HyScanGtkDateTimeMode mode;  /* Режим отображения. */
  GDateTime       *dt;         /* Дата и время. */

  GtkWidget       *popover;    /* Всплывающее окошко. */

  GtkCalendar     *date;       /* Календарь. */
  GtkWidget       *time;       /* Время. */

  GtkAdjustment   *hour;       /* Часы. */
  GtkAdjustment   *minute;     /* Минуты. */
  GtkAdjustment   *second;     /* Секунды. */
};

static void        hyscan_gtk_datetime_object_set_property  (GObject           *object,
                                                             guint              prop_id,
                                                             const GValue      *value,
                                                             GParamSpec        *pspec);
static void        hyscan_gtk_datetime_object_get_property  (GObject           *object,
                                                             guint              prop_id,
                                                             GValue            *value,
                                                             GParamSpec        *pspec);
static void        hyscan_gtk_datetime_object_finalize      (GObject           *object);

static void        hyscan_gtk_datetime_start_edit           (HyScanGtkDateTime *self);
static void        hyscan_gtk_datetime_accept               (HyScanGtkDateTime *self);
static void        hyscan_gtk_datetime_close                (HyScanGtkDateTime *self);

static void        hyscan_gtk_datetime_changed              (HyScanGtkDateTime *self,
                                                             GDateTime         *dt);
static gboolean    hyscan_gtk_datetime_output               (GtkSpinButton     *button);

static GParamSpec * hyscan_gtk_datetime_prop_time;

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkDateTime, hyscan_gtk_datetime, GTK_TYPE_TOGGLE_BUTTON);

static void
hyscan_gtk_datetime_class_init (HyScanGtkDateTimeClass *klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);
  GtkWidgetClass *wclass = GTK_WIDGET_CLASS (klass);

  oclass->set_property = hyscan_gtk_datetime_object_set_property;
  oclass->get_property = hyscan_gtk_datetime_object_get_property;
  oclass->finalize = hyscan_gtk_datetime_object_finalize;

  gtk_widget_class_set_template_from_resource (wclass, WIDGET_RESOURCE_UI);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkDateTime, popover);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkDateTime, date);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkDateTime, time);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkDateTime, hour);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkDateTime, minute);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkDateTime, second);

  gtk_widget_class_bind_template_callback (wclass, hyscan_gtk_datetime_output);
  gtk_widget_class_bind_template_callback (wclass, hyscan_gtk_datetime_start_edit);
  gtk_widget_class_bind_template_callback (wclass, hyscan_gtk_datetime_accept);
  gtk_widget_class_bind_template_callback (wclass, hyscan_gtk_datetime_close);
  gtk_widget_class_bind_template_callback (wclass, gtk_widget_hide);

  g_object_class_install_property (oclass, PROP_MODE,
    g_param_spec_int ("mode", "Mode", "Time, date or both",
                      HYSCAN_GTK_DATETIME_TIME,
                      HYSCAN_GTK_DATETIME_BOTH,
                      HYSCAN_GTK_DATETIME_BOTH,
                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  /* Это специально вынесено в отдельную переменную,
   * чтобы рассылать уведомления об изменении. */
  hyscan_gtk_datetime_prop_time =
    g_param_spec_int64 ("time", "Time", "Current time",
                        G_MININT64, G_MAXINT64, 0,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (oclass, PROP_TIME,
                                   hyscan_gtk_datetime_prop_time);
}

static void
hyscan_gtk_datetime_init (HyScanGtkDateTime *self)
{
  self->priv = hyscan_gtk_datetime_get_instance_private (self);
  gtk_widget_init_template (GTK_WIDGET (self));
  hyscan_gtk_datetime_set_time (self, 0);
}

static void
hyscan_gtk_datetime_object_set_property (GObject      *object,
                                         guint         prop_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
  HyScanGtkDateTime *self = HYSCAN_GTK_DATETIME (object);

  switch (prop_id)
    {
    case PROP_MODE:
      hyscan_gtk_datetime_set_mode (self, g_value_get_int (value));
      break;

    case PROP_TIME:
      hyscan_gtk_datetime_set_time (self, g_value_get_int64 (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
hyscan_gtk_datetime_object_get_property (GObject    *object,
                                         guint       prop_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
  HyScanGtkDateTime *self = HYSCAN_GTK_DATETIME (object);

  switch (prop_id)
    {
    case PROP_MODE:
      g_value_set_int (value, hyscan_gtk_datetime_get_mode (self));
      break;

    case PROP_TIME:
      g_value_set_int64 (value, hyscan_gtk_datetime_get_time (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
hyscan_gtk_datetime_object_finalize (GObject *object)
{
  HyScanGtkDateTime *self = HYSCAN_GTK_DATETIME (object);
  HyScanGtkDateTimePrivate *priv = self->priv;

  g_clear_pointer (&priv->dt, g_date_time_unref);

  G_OBJECT_CLASS (hyscan_gtk_datetime_parent_class)->finalize (object);
}

/* Функция преднастраивает виджеты popover'а. */
static void
hyscan_gtk_datetime_start_edit (HyScanGtkDateTime *self)
{
  HyScanGtkDateTimePrivate *priv = self->priv;

  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (self)))
    return;

  if (priv->mode & HYSCAN_GTK_DATETIME_DATE)
    {
      gint y, m, d;
      g_date_time_get_ymd (priv->dt, &y, &m, &d);
      gtk_calendar_select_month (priv->date, m - 1, y);
      gtk_calendar_select_day (priv->date, d);
    }

  if (priv->mode & HYSCAN_GTK_DATETIME_TIME)
    {
      gint h, m, s;
      h = g_date_time_get_hour (priv->dt);
      m = g_date_time_get_minute (priv->dt);
      s = g_date_time_get_second (priv->dt);
      gtk_adjustment_set_value (priv->hour, h);
      gtk_adjustment_set_value (priv->minute, m);
      gtk_adjustment_set_value (priv->second, s);
    }

  gtk_popover_popup (GTK_POPOVER (priv->popover));

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (self), TRUE);
}

/* Нажатие кнопки OK приводит к сохранению значений. */
static void
hyscan_gtk_datetime_accept (HyScanGtkDateTime *self)
{
  HyScanGtkDateTimePrivate *priv = self->priv;
  GDateTime *dt;
  guint hour = 0, min = 0, sec = 0;
  guint day = 1, month = 1, year = 1970;

  if (priv->mode & HYSCAN_GTK_DATETIME_TIME)
    {
      hour = gtk_adjustment_get_value (priv->hour);
      min = gtk_adjustment_get_value (priv->minute);
      sec = gtk_adjustment_get_value (priv->second);
    }
  if (priv->mode & HYSCAN_GTK_DATETIME_DATE)
    {
      gtk_calendar_get_date (priv->date, &year, &month, &day);
      month += 1;
    }

  dt = g_date_time_new_utc (year, month, day, hour, min, sec);
  hyscan_gtk_datetime_changed (self, dt);
  g_date_time_unref (dt);

  gtk_popover_popdown (GTK_POPOVER (priv->popover));
}

/* Закрытие поповера. */
static void
hyscan_gtk_datetime_close (HyScanGtkDateTime *self)
{
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (self), FALSE);
}

/* Функция устанавливает новый GDateTime. */
static void
hyscan_gtk_datetime_changed (HyScanGtkDateTime *self,
                             GDateTime         *dt)
{
  HyScanGtkDateTimePrivate *priv = self->priv;
  gchar *label = NULL;
  gchar *date = NULL;
  gchar *time = NULL;

  /* Обновляю datetime. */
  g_date_time_ref (dt);
  g_clear_pointer (&priv->dt, g_date_time_unref);
  priv->dt = dt;

  /* Обновляю текст. */
  if (priv->mode & HYSCAN_GTK_DATETIME_TIME)
    time = g_date_time_format (dt, "%T");
  if (priv->mode & HYSCAN_GTK_DATETIME_DATE)
    date = g_date_time_format (dt, "%d.%m.%Y");

  label = g_strdup_printf ("%s%s%s",
                           date != NULL ? date : "",
                           date != NULL ? " " : "",
                           time != NULL ? time : "");
  gtk_button_set_label (GTK_BUTTON (self), label);
  g_free (label);
  g_free (date);
  g_free (time);

  /* И рассылаю уведомление. */
  g_object_notify_by_pspec (G_OBJECT (self), hyscan_gtk_datetime_prop_time);
}

/* Функция форматирует вывод в SpinButtonы. */
static gboolean
hyscan_gtk_datetime_output (GtkSpinButton *spin)
{
  gint value = gtk_adjustment_get_value (gtk_spin_button_get_adjustment (spin));
  gchar text[3];

  g_snprintf (text, sizeof(text), "%02d", value);
  gtk_entry_set_text (GTK_ENTRY (spin), text);

  return TRUE;
}

/**
 * hyscan_gtk_datetime_new:
 * @mode: режим отображения
 *
 * Функция создаёт новый виджет #HyScanGtkDateTime, отображающий данные в
 * соответствии с заданным режимом
 *
 * Returns: #HyScanGtkDateTime.
 */
GtkWidget *
hyscan_gtk_datetime_new (HyScanGtkDateTimeMode mode)
{
  return g_object_new (HYSCAN_TYPE_GTK_DATETIME, "mode", mode, NULL);
}

/**
 * hyscan_gtk_datetime_set_time:
 * @self: #HyScanGtkDateTime
 * @unixtime: временная метка UNIX-time
 *
 * Функция задает текущую отображаемую метку времени.
 */
void
hyscan_gtk_datetime_set_time (HyScanGtkDateTime *self,
                              gint64             unixtime)
{
  GDateTime *dt;

  g_return_if_fail (HYSCAN_IS_GTK_DATETIME (self));

  dt = g_date_time_new_from_unix_utc (unixtime);
  hyscan_gtk_datetime_changed (self, dt);
  g_date_time_unref (dt);
}

/**
 * hyscan_gtk_datetime_get_time:
 * @self: #HyScanGtkDateTime
 *
 * Функция возвращает метку времени.
 * @Returns: временная метка UNIX-time
 */
gint64
hyscan_gtk_datetime_get_time (HyScanGtkDateTime *self)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_DATETIME (self), -1);

  return g_date_time_to_unix (self->priv->dt);
}

/**
 * hyscan_gtk_datetime_set_time:
 * @self: #HyScanGtkDateTime
 * @mode: режим отображения
 *
 * Функция задает режим отображения
 */
void
hyscan_gtk_datetime_set_mode (HyScanGtkDateTime    *self,
                              HyScanGtkDateTimeMode mode)
{
  gboolean time_visible = mode & HYSCAN_GTK_DATETIME_TIME;
  gboolean date_visible = mode & HYSCAN_GTK_DATETIME_DATE;

  g_return_if_fail (HYSCAN_IS_GTK_DATETIME (self));
  g_return_if_fail (time_visible || date_visible);
  self->priv->mode = mode;

  gtk_widget_set_visible (self->priv->time, time_visible);
  gtk_widget_set_visible (GTK_WIDGET (self->priv->date), date_visible);
  hyscan_gtk_datetime_changed (self, self->priv->dt);
}

/**
 * hyscan_gtk_datetime_get_mode:
 * @self: #HyScanGtkDateTime
 *
 * Функция возвращает режим отображения.
 * @Returns: режим отображения из #HyScanGtkDateTimeMode
 */
HyScanGtkDateTimeMode
hyscan_gtk_datetime_get_mode (HyScanGtkDateTime *self)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_DATETIME (self), -1);

  return self->priv->mode;
}
