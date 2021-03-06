/* hyscan-gtk-param-key.c
 *
 * Copyright 2018 Screen LLC, Alexander Dmitriev <m1n7@yandex.ru>
 *
 * This file is part of HyScanGui.
 *
 * HyScanGui is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HyScanGui is distributed in the hope that it will be useful,
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

/* HyScanGui имеет двойную лицензию.
 *
 * Во-первых, вы можете распространять HyScanGui на условиях Стандартной
 * Общественной Лицензии GNU версии 3, либо по любой более поздней версии
 * лицензии (по вашему выбору). Полные положения лицензии GNU приведены в
 * <http://www.gnu.org/licenses/>.
 *
 * Во-вторых, этот программный код можно использовать по коммерческой
 * лицензии. Для этого свяжитесь с ООО Экран - <info@screen-co.ru>.
 */

/**
 * SECTION: hyscan-gtk-param-key
 * @Short_description: Виджет ключа и значения в схеме данных
 * @Title: HyScanGtkParamKey
 *
 * Данный класс прячет внутри себя всю работу по установке, отображению и
 * смене значения. Виджет состоит из двух подвиджетов: виджет названия параметра
 * и виджет значения. С помощью функций #hyscan_gtk_param_key_get_label и
 * #hyscan_gtk_param_key_get_value можно получить эти виджеты и изменить способ
 * выравнивания или как-либо иначе настроить внешний вид виджета.
 *
 * В зависимости от типа ключа и предлагаемого типа отображения автоматически
 * выбирается, как этот ключ будет отображен: строка, переключатель, число,
 * календарь, список. Пользователю не требуется подключаться к разным сигналам
 * в зависимости от типа ключа: существует единственный сигнал "changed",
 * передающий #GVariant
 */
#include "hyscan-gtk-param-key.h"
#include <hyscan-gtk-spin-button.h>
#include <hyscan-gtk-datetime.h>
#include <hyscan-gtk-marshallers.h>
#include <math.h>

#define DESCRIPTION_MARKUP "<span style=\"italic\">\%s</span>"
#define INVALID "HyScanGtkParam: invalid key"

#define time_view(view) ((view) == HYSCAN_DATA_SCHEMA_VIEW_DATE || \
                        (view) == HYSCAN_DATA_SCHEMA_VIEW_TIME || \
                        (view) == HYSCAN_DATA_SCHEMA_VIEW_DATE_TIME)
#define value_view(view) ((view) == HYSCAN_DATA_SCHEMA_VIEW_DEFAULT || \
                          (view) == HYSCAN_DATA_SCHEMA_VIEW_HEX || \
                          (view) == HYSCAN_DATA_SCHEMA_VIEW_BIN || \
                          (view) == HYSCAN_DATA_SCHEMA_VIEW_DEC)
#define color_view(view) ((view) == HYSCAN_DATA_SCHEMA_VIEW_RGB || \
                          (view) == HYSCAN_DATA_SCHEMA_VIEW_RGBA)

enum
{
  VALUE_COL,
  NAME_COL,
  N_COL
};

enum
{
  SIGNAL_CHANGED,
  SIGNAL_LAST
};

enum
{
  PROP_0,
  PROP_DATA_SCHEMA,
  PROP_KEY
};

struct _HyScanGtkParamKeyPrivate
{
  HyScanDataSchema    *schema;
  HyScanDataSchemaKey *key;

  GtkWidget           *label;
  GtkWidget           *value;

  guint                handler;

  GtkSizeGroup        *hsize;
};

static void       hyscan_gtk_param_key_set_property          (GObject               *object,
                                                              guint                  prop_id,
                                                              const GValue          *value,
                                                              GParamSpec            *pspec);
static void       hyscan_gtk_param_key_object_constructed    (GObject               *object);
static void       hyscan_gtk_param_key_object_finalize       (GObject               *object);
static void       hyscan_gtk_param_key_emit_variant          (HyScanGtkParamKey     *self,
                                                              GVariant              *value);

static gchar *    hyscan_gtk_param_key_enum_id               (gint64                 val);
static gint64     hyscan_gtk_param_key_enum_val              (const gchar           *id);

static GtkWidget* hyscan_gtk_param_key_make_editor           (HyScanGtkParamKey     *self);
static GtkWidget* hyscan_gtk_param_key_make_editor_boolean   (HyScanDataSchema      *schema,
                                                              HyScanDataSchemaKey   *key);
static GtkWidget* hyscan_gtk_param_key_make_editor_btn       (HyScanDataSchema      *schema,
                                                              HyScanDataSchemaKey   *key);
static GtkWidget* hyscan_gtk_param_key_make_editor_integer   (HyScanDataSchema      *schema,
                                                              HyScanDataSchemaKey   *key);
static GtkWidget* hyscan_gtk_param_key_make_editor_time      (HyScanDataSchema      *schema,
                                                              HyScanDataSchemaKey   *key);
static GtkWidget* hyscan_gtk_param_key_make_editor_double    (HyScanDataSchema      *schema,
                                                              HyScanDataSchemaKey   *key);
static GtkWidget* hyscan_gtk_param_key_make_editor_color     (HyScanDataSchema      *schema,
                                                              HyScanDataSchemaKey   *key);
static GtkWidget* hyscan_gtk_param_key_make_editor_string    (HyScanDataSchema      *schema,
                                                              HyScanDataSchemaKey   *key);
static GtkWidget* hyscan_gtk_param_key_make_editor_enum      (HyScanDataSchema      *schema,
                                                              HyScanDataSchemaKey   *key);
static void       hyscan_gtk_param_key_notify_boolean        (GObject               *object,
                                                              GParamSpec            *pspec,
                                                              gpointer               udata);
static void       hyscan_gtk_param_key_notify_btn            (GObject               *object,
                                                              gpointer               udata);
static void       hyscan_gtk_param_key_notify_integer        (GObject               *object,
                                                              GParamSpec            *pspec,
                                                              gpointer               udata);
static void       hyscan_gtk_param_key_notify_time           (GObject               *object,
                                                              GParamSpec            *pspec,
                                                              gpointer               udata);
static void       hyscan_gtk_param_key_notify_double         (GObject               *object,
                                                              GParamSpec            *pspec,
                                                              gpointer               udata);
static void       hyscan_gtk_param_key_notify_string         (GObject               *object,
                                                              GParamSpec            *pspec,
                                                              gpointer               udata);
static void       hyscan_gtk_param_key_notify_color          (GObject               *object,
                                                              GParamSpec            *pspec,
                                                              gpointer               udata);
static void       hyscan_gtk_param_key_notify_enum           (GObject               *object,
                                                              GParamSpec            *pspec,
                                                              gpointer               udata);

static guint      hyscan_gtk_param_key_signals[SIGNAL_LAST] = {0};

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkParamKey, hyscan_gtk_param_key, GTK_TYPE_GRID);

static void
hyscan_gtk_param_key_class_init (HyScanGtkParamKeyClass *klass)
{
  GParamFlags flags;
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = hyscan_gtk_param_key_set_property;
  object_class->constructed = hyscan_gtk_param_key_object_constructed;
  object_class->finalize = hyscan_gtk_param_key_object_finalize;

  flags = G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY;

  g_object_class_install_property (object_class, PROP_DATA_SCHEMA,
    g_param_spec_object ("schema", "DataSchema", "HyScanDataSchema",
                         HYSCAN_TYPE_DATA_SCHEMA, flags));
  g_object_class_install_property (object_class, PROP_KEY,
    g_param_spec_boxed ("key", "Key", "Key identificator",
                        hyscan_data_schema_key_get_type (), flags));

  /**
   * HyScanGtkParamKey::changed:
   * @paramkey: пославший сигнал #HyScanGtkParamKey
   * @id: идентификатор ключа (см #HyScanParamKey)
   * @value: (type GVariant): текущее значение
   *
   * Сигнал посылается, когда пользователь изменяет значение (кнопками,
   * с клавиатуры, мышью).
   */
  hyscan_gtk_param_key_signals[SIGNAL_CHANGED] =
    g_signal_new ("changed", HYSCAN_TYPE_GTK_PARAM_KEY,
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  hyscan_gtk_marshal_VOID__STRING_VARIANT,
                  G_TYPE_NONE,
                  2, G_TYPE_STRING, G_TYPE_VARIANT);
}

static void
hyscan_gtk_param_key_init (HyScanGtkParamKey *self)
{
  self->priv = hyscan_gtk_param_key_get_instance_private (self);
}

static void
hyscan_gtk_param_key_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  HyScanGtkParamKey *self = HYSCAN_GTK_PARAM_KEY (object);
  HyScanGtkParamKeyPrivate *priv = self->priv;

  if (prop_id == PROP_DATA_SCHEMA)
    priv->schema = g_value_dup_object (value);
  else if (prop_id == PROP_KEY)
    priv->key = g_value_dup_boxed (value);
  else
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
}

static void
hyscan_gtk_param_key_object_constructed (GObject *object)
{
  HyScanGtkParamKey *self = HYSCAN_GTK_PARAM_KEY (object);
  HyScanGtkParamKeyPrivate *priv = self->priv;
  gboolean sensitive = (priv->key->access & HYSCAN_DATA_SCHEMA_ACCESS_WRITE);
  gboolean has_label = (priv->key->view != HYSCAN_DATA_SCHEMA_VIEW_BUTTON);

  G_OBJECT_CLASS (hyscan_gtk_param_key_parent_class)->constructed (object);

  priv->value = hyscan_gtk_param_key_make_editor (self);
  gtk_widget_set_sensitive (priv->value, sensitive);

  if (priv->key->type == HYSCAN_DATA_SCHEMA_KEY_BOOLEAN && has_label)
    gtk_widget_set_halign (priv->value, GTK_ALIGN_START);
  else
    gtk_widget_set_halign (priv->value, GTK_ALIGN_FILL);
  gtk_widget_set_hexpand (priv->value, TRUE);

  gtk_grid_set_column_spacing (GTK_GRID (self), 12);
  gtk_grid_attach (GTK_GRID (self), priv->value, 1, 0, 1, 1);

  if (has_label)
    {
      priv->label = gtk_label_new (priv->key->name);
      gtk_widget_set_sensitive (priv->label, sensitive);

      gtk_label_set_xalign (GTK_LABEL (priv->label), 1.0);
      gtk_widget_set_halign (priv->label, GTK_ALIGN_END);
      gtk_widget_set_hexpand (priv->label, FALSE);

      gtk_grid_attach (GTK_GRID (self), priv->label, 0, 0, 1, 1);
    }

  gtk_widget_set_tooltip_text (has_label ? priv->label : priv->value, priv->key->description);

  gtk_widget_set_name (GTK_WIDGET (self), priv->key->id);
}

static void
hyscan_gtk_param_key_object_finalize (GObject *object)
{
  HyScanGtkParamKey *self = HYSCAN_GTK_PARAM_KEY (object);
  HyScanGtkParamKeyPrivate *priv = self->priv;

  g_clear_object (&priv->schema);
  g_clear_pointer (&priv->key, hyscan_data_schema_key_free);

  g_clear_object (&priv->hsize);

  G_OBJECT_CLASS (hyscan_gtk_param_key_parent_class)->finalize (object);
}

/* Функция эмиттирует сигнал и очищает гвариант. */
static void
hyscan_gtk_param_key_emit_variant (HyScanGtkParamKey     *self,
                                   GVariant              *variant)
{
  g_variant_ref_sink (variant);

  g_signal_emit (self, hyscan_gtk_param_key_signals[SIGNAL_CHANGED], 0,
                 self->priv->key->id, variant);

  g_variant_unref (variant);
}

/* Генератор идентификаторов для перечислений. */
static gchar *
hyscan_gtk_param_key_enum_id (gint64 val)
{
  return g_strdup_printf ("%li", val);
}

/* Функция возвращает численное значение перечисления по его идентификатору. */
static gint64
hyscan_gtk_param_key_enum_val (const gchar *id)
{
  return g_ascii_strtoll (id, NULL, 10);
}

/* Обертка для создания редактора. */
static GtkWidget *
hyscan_gtk_param_key_make_editor (HyScanGtkParamKey *self)
{
  HyScanGtkParamKeyPrivate *priv = self->priv;
  HyScanDataSchema *schema = priv->schema;
  HyScanDataSchemaKey *key = priv->key;

  void (*cbk) (GObject *, GParamSpec *, gpointer) = NULL;

  GtkWidget *editor = NULL;
  const gchar *signal = NULL;

  switch (key->type)
    {
    case HYSCAN_DATA_SCHEMA_KEY_BOOLEAN:
      if (key->view == HYSCAN_DATA_SCHEMA_VIEW_BUTTON && key->access == HYSCAN_DATA_SCHEMA_ACCESS_WRITE)
        {
          editor = hyscan_gtk_param_key_make_editor_btn (schema, key);
          signal = "clicked";
          cbk = hyscan_gtk_param_key_notify_btn;
        }
      else
        {
          editor = hyscan_gtk_param_key_make_editor_boolean (schema, key);
          signal = "notify::active";
          cbk = hyscan_gtk_param_key_notify_boolean;
        }
      break;

    case HYSCAN_DATA_SCHEMA_KEY_INTEGER:

      if (time_view (key->view))
        {
          editor = hyscan_gtk_param_key_make_editor_time (schema, key);
          signal = "notify::time";
          cbk = hyscan_gtk_param_key_notify_time;
        }
      else if (value_view (key->view))
        {
          editor = hyscan_gtk_param_key_make_editor_integer (schema, key);
          signal = "notify::value";
          cbk = hyscan_gtk_param_key_notify_integer;
        }
      break;

    case HYSCAN_DATA_SCHEMA_KEY_DOUBLE:
      editor = hyscan_gtk_param_key_make_editor_double (schema, key);
      signal = "notify::value";
      cbk = hyscan_gtk_param_key_notify_double;
      break;

    case HYSCAN_DATA_SCHEMA_KEY_STRING:
      if (color_view (key->view))
        {
          editor = hyscan_gtk_param_key_make_editor_color (schema, key);
          signal = "notify::color";
          cbk = hyscan_gtk_param_key_notify_color;
        }
      else
        {
          editor = hyscan_gtk_param_key_make_editor_string (schema, key);
          signal = "notify::text";
          cbk = hyscan_gtk_param_key_notify_string;
        }
      break;

    case HYSCAN_DATA_SCHEMA_KEY_ENUM:
      editor = hyscan_gtk_param_key_make_editor_enum (schema, key);
      signal = "notify::active-id";
      cbk = hyscan_gtk_param_key_notify_enum;
      break;

    default:
      break;
    }

  if (editor == NULL)
    editor = gtk_label_new (INVALID);

  if (signal != NULL && cbk != NULL)
    {
      priv->handler = g_signal_connect (G_OBJECT (editor), signal,
                                        G_CALLBACK (cbk), self);
    }

  // TODO: Access_writeonly
  return editor;
}

/* Функция создает редактор для булевых значений. */
static GtkWidget *
hyscan_gtk_param_key_make_editor_boolean (HyScanDataSchema    *schema,
                                          HyScanDataSchemaKey *key)
{
  GtkWidget *editor;
  GVariant *def;
  gboolean active = FALSE;

  def = hyscan_data_schema_key_get_default (schema, key->id);
  if (def != NULL)
    active = g_variant_get_boolean (def);

  if (key->view == HYSCAN_DATA_SCHEMA_VIEW_BUTTON)
    {
      editor = gtk_toggle_button_new_with_label (key->name);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor), active);
    }
  else
    {
      editor = gtk_switch_new ();
      gtk_switch_set_active (GTK_SWITCH (editor), active);
    }

  g_clear_pointer (&def, g_variant_unref);

  return editor;
}

/* Функция создает редактор для булевых write-only значений. */
static GtkWidget *
hyscan_gtk_param_key_make_editor_btn (HyScanDataSchema    *schema,
                                      HyScanDataSchemaKey *key)
{
  return gtk_button_new_with_label (key->name);
}

/* Функция создает редактор для целых значений. */
static GtkWidget *
hyscan_gtk_param_key_make_editor_integer (HyScanDataSchema    *schema,
                                          HyScanDataSchemaKey *key)
{
  GtkWidget *editor = NULL;
  GtkAdjustment *adjustment;
  GVariant *_min, *_max, *_step, *_def;
  gint64 def, min, max, step;
  guint base;

  def = 0;
  min = G_MININT64;
  max = G_MAXINT64;
  step = 1;

  _def = hyscan_data_schema_key_get_default (schema, key->id);
  _min = hyscan_data_schema_key_get_minimum (schema, key->id);
  _max = hyscan_data_schema_key_get_maximum (schema, key->id);
  _step = hyscan_data_schema_key_get_step (schema, key->id);

  if (_def != NULL)
    def = g_variant_get_int64 (_def);
  if (_min != NULL)
    min = g_variant_get_int64 (_min);
  if (_max != NULL)
    max = g_variant_get_int64 (_max);
  if (_step != NULL)
    step = g_variant_get_int64 (_step);

  if (key->view == HYSCAN_DATA_SCHEMA_VIEW_BIN)
    base = 2;
  else if (key->view == HYSCAN_DATA_SCHEMA_VIEW_HEX)
    base = 16;
  else /* if (key->view == HYSCAN_DATA_SCHEMA_VIEW_DEC || key->view == HYSCAN_DATA_SCHEMA_VIEW_DEFAULT) */
    base = 10;

  adjustment = gtk_adjustment_new (def, min, max, step, 10.0 * step, 0);
  editor = g_object_new (HYSCAN_TYPE_GTK_SPIN_BUTTON,
                         "base", base,                  /* HyScanGtkSpinButton */
                         "adjustment", adjustment,      /* GtkSpinButton */
                         "climb-rate", 1.0,             /* GtkSpinButton */
                         NULL);

  g_clear_pointer (&_def, g_variant_unref);
  g_clear_pointer (&_min, g_variant_unref);
  g_clear_pointer (&_max, g_variant_unref);
  g_clear_pointer (&_step, g_variant_unref);

  return editor;
}

/* Функция создает редактор для значений времени. */
static GtkWidget *
hyscan_gtk_param_key_make_editor_time (HyScanDataSchema    *schema,
                                       HyScanDataSchemaKey *key)
{
  GtkWidget *editor = NULL;
  HyScanGtkDateTimeMode mode;
  GVariant *_def;
  gint64 def = 1;

  _def = hyscan_data_schema_key_get_default (schema, key->id);
  if (_def != NULL)
    def = g_variant_get_int64 (_def);

  if (key->view == HYSCAN_DATA_SCHEMA_VIEW_DATE)
    mode = HYSCAN_GTK_DATETIME_DATE;
  else if (key->view == HYSCAN_DATA_SCHEMA_VIEW_TIME)
    mode = HYSCAN_GTK_DATETIME_TIME;
  else /* if (key->view == HYSCAN_DATA_SCHEMA_VIEW_DATE_TIME) */
    mode = HYSCAN_GTK_DATETIME_BOTH;

  editor = hyscan_gtk_datetime_new (mode);
  hyscan_gtk_datetime_set_time (HYSCAN_GTK_DATETIME (editor), def);

  g_clear_pointer (&_def, g_variant_unref);

  return editor;
}

/* Функция создает редактор для значений double. */
static GtkWidget *
hyscan_gtk_param_key_make_editor_double (HyScanDataSchema    *schema,
                                         HyScanDataSchemaKey *key)
{
  GtkWidget *editor;
  GtkAdjustment *adjustment;
  GVariant *_min, *_max, *_step, *_def;
  gdouble def, min, max, step;
  guint digits;

  def = 0;
  min = -G_MAXDOUBLE;
  max = G_MAXDOUBLE;
  step = 0.1;

  _def = hyscan_data_schema_key_get_default (schema, key->id);
  _min = hyscan_data_schema_key_get_minimum (schema, key->id);
  _max = hyscan_data_schema_key_get_maximum (schema, key->id);
  _step = hyscan_data_schema_key_get_step (schema, key->id);

  if (_def != NULL)
    def = g_variant_get_double (_def);
  if (_min != NULL)
    min = g_variant_get_double (_min);
  if (_max != NULL)
    max = g_variant_get_double (_max);
  if (_step != NULL)
    step = g_variant_get_double (_step);

  /* Это сделано потому, что adjustment принимает значения [start, end). */
  if (max < G_MAXDOUBLE - step)
    max += step;
  else
    max = G_MAXDOUBLE;

  /* Количество знаков после запятой определяется шагом. */
  digits = ABS(log10 (step)) + 1;

  adjustment = gtk_adjustment_new (def, min, max, step, step, step);
  editor = gtk_spin_button_new (adjustment, step, digits);

  g_clear_pointer (&_def, g_variant_unref);
  g_clear_pointer (&_min, g_variant_unref);
  g_clear_pointer (&_max, g_variant_unref);
  g_clear_pointer (&_step, g_variant_unref);

  return editor;
}

/* Функция создает редактор для цветовых значений. */
static GtkWidget *
hyscan_gtk_param_key_make_editor_color (HyScanDataSchema    *schema,
                                        HyScanDataSchemaKey *key)
{
  GtkWidget *editor;
  GVariant *_def;
  const gchar *def = "";
  GdkRGBA color;

  editor = gtk_color_button_new ();
  if (key->view == HYSCAN_DATA_SCHEMA_VIEW_RGBA)
    gtk_color_chooser_set_use_alpha (GTK_COLOR_CHOOSER (editor), TRUE);

  _def = hyscan_data_schema_key_get_default (schema, key->id);
  if (_def != NULL)
    def = g_variant_get_string (_def, NULL);

  if (gdk_rgba_parse (&color, def))
    gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (editor), &color);

  g_clear_pointer (&_def, g_variant_unref);

  return editor;
}

/* Функция создает редактор для строковых значений. */
static GtkWidget *
hyscan_gtk_param_key_make_editor_string (HyScanDataSchema    *schema,
                                         HyScanDataSchemaKey *key)
{
  GtkWidget *editor;
  GVariant *_def;
  const gchar *def = "";

  editor = gtk_entry_new ();

  _def = hyscan_data_schema_key_get_default (schema, key->id);
  if (_def != NULL)
    def = g_variant_get_string (_def, NULL);

  gtk_entry_set_text (GTK_ENTRY (editor), def);

  g_clear_pointer(&_def, g_variant_unref);

  return editor;
}

/* Функция создает редактор для перечислений. */
static GtkWidget *
hyscan_gtk_param_key_make_editor_enum (HyScanDataSchema    *schema,
                                       HyScanDataSchemaKey *key)
{
  GtkWidget *editor;
  const gchar *enum_id;
  GList *values, *link;
  GVariant *_def;

  /* Список значений. */
  enum_id = hyscan_data_schema_key_get_enum_id (schema, key->id);
  values = hyscan_data_schema_enum_get_values (schema, enum_id);

  editor = gtk_combo_box_text_new ();

  for (link = values; link != NULL; link = link->next)
    {
      HyScanDataSchemaEnumValue *data = link->data;

      gchar *id = hyscan_gtk_param_key_enum_id (data->value);

      gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (editor), id, data->name);

      g_free (id);
    }

  _def = hyscan_data_schema_key_get_default (schema, key->id);
  if (_def != NULL)
    {
      gint64 def;
      gchar *def_id;

      def = g_variant_get_int64 (_def);
      g_variant_unref (_def);

      def_id = hyscan_gtk_param_key_enum_id (def);
      gtk_combo_box_set_active_id (GTK_COMBO_BOX (editor), def_id);
      g_free (def_id);
    }

  g_list_free (values);

  return editor;
}

/* Функция уведомления о смене булева значения. */
static void
hyscan_gtk_param_key_notify_boolean (GObject    *object,
                                     GParamSpec *pspec,
                                     gpointer    udata)
{
  HyScanGtkParamKey *self = udata;
  gboolean val;
  GVariant *variant;

  g_object_get (object, pspec->name, &val, NULL);
  variant = g_variant_new_boolean (val);
  hyscan_gtk_param_key_emit_variant (self, variant);
}

/* Функция уведомления о смене булева значения кнопки. */
static void
hyscan_gtk_param_key_notify_btn (GObject    *object,
                                 gpointer    udata)
{
  HyScanGtkParamKey *self = udata;
  GVariant *variant;

  variant = g_variant_new_boolean (TRUE);
  hyscan_gtk_param_key_emit_variant (self, variant);
}

/* Функция уведомления о смене целочисленного значения. */
static void
hyscan_gtk_param_key_notify_integer (GObject    *object,
                                     GParamSpec *pspec,
                                     gpointer    udata)
{
  HyScanGtkParamKey *self = udata;
  gdouble _val;
  gint64 val;
  GVariant *variant;

  g_object_get (object, pspec->name, &_val, NULL);
  val = (gint64) _val;
  variant = g_variant_new_int64 (val);
  hyscan_gtk_param_key_emit_variant (self, variant);
}

/* Функция уведомления о смене целочисленного значения. */
static void
hyscan_gtk_param_key_notify_time (GObject    *object,
                                  GParamSpec *pspec,
                                  gpointer    udata)
{
  HyScanGtkParamKey *self = udata;
  gint64 val;
  GVariant *variant;

  g_object_get (object, pspec->name, &val, NULL);
  variant = g_variant_new_int64 (val);
  hyscan_gtk_param_key_emit_variant (self, variant);
}

/* Функция уведомления о смене double значения. */
static void
hyscan_gtk_param_key_notify_double (GObject    *object,
                                    GParamSpec *pspec,
                                    gpointer    udata)
{
  HyScanGtkParamKey *self = udata;
  gdouble val;
  GVariant *variant;

  g_object_get (object, pspec->name, &val, NULL);
  variant = g_variant_new_double (val);
  hyscan_gtk_param_key_emit_variant (self, variant);
}

/* Функция уведомления о смене строкового значения. */
static void
hyscan_gtk_param_key_notify_string (GObject    *object,
                                    GParamSpec *pspec,
                                    gpointer    udata)
{
  HyScanGtkParamKey *self = udata;
  gchar * val = NULL;
  GVariant *variant;

  g_object_get (object, pspec->name, &val, NULL);
  variant = g_variant_new_take_string (val);
  hyscan_gtk_param_key_emit_variant (self, variant);
}

/* Функция уведомления о смене значения цвета. */
static void
hyscan_gtk_param_key_notify_color (GObject    *object,
                                   GParamSpec *pspec,
                                   gpointer    udata)
{
  HyScanGtkParamKey *self = udata;
  gchar * val = NULL;
  GVariant *variant;
  GdkRGBA rgba;

  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (object), &rgba);
  val = gdk_rgba_to_string (&rgba);
  variant = g_variant_new_take_string (val);
  hyscan_gtk_param_key_emit_variant (self, variant);
}

/* Функция уведомления о смене перечисления. */
static void
hyscan_gtk_param_key_notify_enum (GObject    *object,
                                  GParamSpec *pspec,
                                  gpointer    udata)
{
  HyScanGtkParamKey *self = udata;
  gchar * _val, *check;
  gint64 val;
  GVariant *variant;

  g_object_get (object, pspec->name, &_val, NULL);

  val = g_ascii_strtoll (_val, &check, 10);
  if (check == NULL)
    g_warning ("HyScanGtkParamKey: enum id not recognised");
  g_free (_val);

  variant = g_variant_new_int64 (val);
  hyscan_gtk_param_key_emit_variant (self, variant);
}

/**
 * hyscan_gtk_param_key_new:
 * @schema указатель на #HyScanDataSchema
 * @key указатель на #HyScanDataSchemaKey
 *
 * Функция создает новый виджет HyScanGtkParamKey.
 *
 * Returns: HyScanGtkParamKey.
 */
GtkWidget *
hyscan_gtk_param_key_new (HyScanDataSchema    *schema,
                          HyScanDataSchemaKey *key)
{
  HyScanGtkParamKey *object;

  object = g_object_new (HYSCAN_TYPE_GTK_PARAM_KEY,
                         "schema", schema,
                         "key", key,
                         NULL);

  return GTK_WIDGET (object);
}

/**
 * hyscan_gtk_param_key_get_key:
 * @self: указатель на #HyScanGtkParamKey
 *
 * Функция возвращает идентификатор ключа.
 *
 * Returns: идентификатор ключа.
 */
const gchar *
hyscan_gtk_param_key_get_key (HyScanGtkParamKey *self)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_PARAM_KEY (self), NULL);

  return self->priv->key->id;
}

/**
 * hyscan_gtk_param_key_set_from_param_list:
 * @self: указатель на #HyScanGtkParamKey
 * @plist: указатель на #HyScanParamList
 *
 * Функция позволяет задать значение из #HyScanParamList
 */
void
hyscan_gtk_param_key_set_from_param_list (HyScanGtkParamKey *self,
                                          HyScanParamList   *plist)
{
  GVariant *value;

  g_return_if_fail (HYSCAN_IS_GTK_PARAM_KEY (self));
  if (!HYSCAN_IS_PARAM_LIST (plist))
    return;

  value = hyscan_param_list_get (plist, self->priv->key->id);

  hyscan_gtk_param_key_set (self, value);

  g_variant_unref (value);
}

/**
 * hyscan_gtk_param_key_set:
 * @self: указатель на #HyScanGtkParamKey
 * @value: значение
 *
 * Функция позволяет задать значение из #GVariant.
 */
void
hyscan_gtk_param_key_set (HyScanGtkParamKey *self,
                          GVariant          *value)
{
  HyScanGtkParamKeyPrivate *priv;
  GVariantClass variant_class;

  g_return_if_fail (HYSCAN_IS_GTK_PARAM_KEY (self));
  priv = self->priv;

  variant_class = g_variant_classify (value);

  g_signal_handler_block (priv->value, priv->handler);

  switch (priv->key->type)
    {
    case HYSCAN_DATA_SCHEMA_KEY_BOOLEAN:
      {
        gboolean val;
        if (variant_class != G_VARIANT_CLASS_BOOLEAN)
          break;

        val = g_variant_get_boolean (value);
        if (priv->key->view == HYSCAN_DATA_SCHEMA_VIEW_BUTTON)
          gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->value), val);
        else
          gtk_switch_set_active (GTK_SWITCH (priv->value), val);
        break;
      }

    case HYSCAN_DATA_SCHEMA_KEY_INTEGER:
      {
        gint64 val;

        if (variant_class != G_VARIANT_CLASS_INT64)
          break;

        val = g_variant_get_int64 (value);
        if (time_view (priv->key->view))
          hyscan_gtk_datetime_set_time (HYSCAN_GTK_DATETIME (priv->value), val);
        else
          gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->value), val);
        break;
      }

    case HYSCAN_DATA_SCHEMA_KEY_DOUBLE:
      {
        GtkAdjustment *adj;
        gdouble val;

        if (variant_class != G_VARIANT_CLASS_DOUBLE)
          break;

        val = g_variant_get_double (value);
        adj = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (priv->value));
        gtk_adjustment_set_value (adj, val);
        break;
      }

    case HYSCAN_DATA_SCHEMA_KEY_STRING:
      {
        const gchar *val;

        if (variant_class != G_VARIANT_CLASS_STRING)
          break;

        val = g_variant_get_string (value, NULL);
        if (color_view (priv->key->view))
          {
            GdkRGBA rgba;

            gdk_rgba_parse (&rgba, val);
            gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (priv->value), &rgba);
          }
        else
          {
            gtk_entry_set_text (GTK_ENTRY (priv->value), val);
          }

        break;
      }

    case HYSCAN_DATA_SCHEMA_KEY_ENUM:
      {
        gchar *id;
        gint64 val;

        if (variant_class != G_VARIANT_CLASS_INT64)
          break;

        val = g_variant_get_int64 (value);
        id = hyscan_gtk_param_key_enum_id (val);
        gtk_combo_box_set_active_id (GTK_COMBO_BOX (priv->value), id);
        g_free (id);
        break;
      }

    default:
      g_warning (INVALID);
    }

  g_signal_handler_unblock (priv->value, priv->handler);
}

/**
 * hyscan_gtk_param_key_get:
 * @self: указатель на #HyScanGtkParamKey
 *
 * Функция считывает значение в #GVariant.
 *
 * Returns: (transfer full): указатель на #GVariant, для удаления g_variant_unref()
 */
GVariant *
hyscan_gtk_param_key_get (HyScanGtkParamKey *self)
{
  HyScanGtkParamKeyPrivate *priv;
  GVariant *value = NULL;

  g_return_val_if_fail (HYSCAN_IS_GTK_PARAM_KEY (self), NULL);

  priv = self->priv;

  switch (priv->key->type)
    {
    case HYSCAN_DATA_SCHEMA_KEY_BOOLEAN:
      {
        gboolean val;

        val = gtk_switch_get_active (GTK_SWITCH (priv->value));

        value = g_variant_new_boolean (val);
        break;
      }

    case HYSCAN_DATA_SCHEMA_KEY_INTEGER:
      {
        gint64 val;

        if (time_view (priv->key->view))
          val = hyscan_gtk_datetime_get_time (HYSCAN_GTK_DATETIME (priv->value));
        else
          val = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (priv->value));

        value = g_variant_new_int64 (val);
        break;
      }

    case HYSCAN_DATA_SCHEMA_KEY_DOUBLE:
      {
        gdouble val;

        val = gtk_spin_button_get_value(GTK_SPIN_BUTTON (priv->value));
        value = g_variant_new_double (val);
        break;
      }

    case HYSCAN_DATA_SCHEMA_KEY_STRING:
      {
        if (color_view (priv->key->view))
          {
            GdkRGBA rgba;
            gchar *string;

            gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (priv->value), &rgba);
            string = gdk_rgba_to_string (&rgba);
            value = g_variant_new_take_string (string);
          }
        else
          {
            const gchar *string;

            string = gtk_entry_get_text (GTK_ENTRY (priv->value));
            value = g_variant_new_string (string);
          }

        break;
      }

    case HYSCAN_DATA_SCHEMA_KEY_ENUM:
      {
        const gchar *id;
        gint64 val;

        id = gtk_combo_box_get_active_id (GTK_COMBO_BOX (priv->value));
        val = hyscan_gtk_param_key_enum_val (id);

        value = g_variant_new_int64 (val);
        break;
      }

    default:
      g_warning (INVALID);

      return NULL;
    }

  return g_variant_ref_sink (value);
}

/**
 * hyscan_gtk_param_key_add_to_size_group:
 * @self: указатель на #HyScanGtkParamKey
 * @group #GtkSizeGroup

 * Функция добавляет виджет названия к #GtkSizeGroup.
 */
void
hyscan_gtk_param_key_add_to_size_group (HyScanGtkParamKey *self,
                                        GtkSizeGroup      *group)
{
  HyScanGtkParamKeyPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_PARAM_KEY (self));

  priv = self->priv;

  if (priv->hsize != NULL)
    {
      gtk_size_group_remove_widget (priv->hsize, GTK_WIDGET (self));
      g_clear_object (&priv->hsize);
    }

  if (group != NULL && priv->label != NULL)
    {
      gtk_size_group_add_widget (group, priv->label);
      priv->hsize = g_object_ref (group);
    }
}

/**
 * hyscan_gtk_param_key_get_label:
 * @self: указатель на #HyScanGtkParamKey
 *
 * Функция возвращает виджет названия.
 *
 * Returns: (transfer none): указатель на виджет названия. Освобождать эту память нельзя!
 */
const GtkWidget *
hyscan_gtk_param_key_get_label (HyScanGtkParamKey *self)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_PARAM_KEY (self), NULL);

  return self->priv->label;
}

/**
 * hyscan_gtk_param_key_get_value:
 * @self: указатель на #HyScanGtkParamKey
 *
 * Функция возвращает виджет значения
 *
 * Returns: (transfer none): указатель на виджет значения. Освобождать эту память нельзя!
 */
const GtkWidget *
hyscan_gtk_param_key_get_value (HyScanGtkParamKey *self)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_PARAM_KEY (self), NULL);

  return self->priv->value;
}
