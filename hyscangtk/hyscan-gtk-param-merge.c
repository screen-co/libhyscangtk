/* hyscan-gtk-param-merge.c
 *
 * Copyright 2018 Screen LLC, Alexander Dmitriev <m1n7@yandex.ru>
 * Copyright 2020 Screen LLC, Alexey Sakhnov <alexsakhnov@gmail.com>
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
 * SECTION: hyscan-gtk-param-merge
 * @Short_description: виджет, отображающий HyScanParamMerge одним списком
 * @Title: HyScanGtkParamMerge
 *
 * Простейший виджет, отображающий все ключи единым списком. Аналогичен HyScanGtkParamList,
 * с тем отличием, что позволяет напротив каждого параметра отображается галочка,
 * включающая запись этого параметра.
 *
 * Для работы виджета необходим бэкенд типа #HyScanParamMerge.
 */

#include "hyscan-gtk-param-merge.h"

struct _HyScanGtkParamMergePrivate
{
  GtkWidget *box;
};

static void       hyscan_gtk_param_merge_object_constructed (GObject                    *object);
static void       hyscan_gtk_param_merge_object_finalize    (GObject                    *object);
static void       hyscan_gtk_param_merge_keep_changed       (GtkToggleButton            *button,
                                                             GParamSpec                 *pspec,
                                                             HyScanGtkParamMerge        *self);
static void       hyscan_gtk_param_merge_add_widgets        (HyScanGtkParamMerge        *self,
                                                             const HyScanDataSchemaNode *node,
                                                             HyScanParamList            *plist,
                                                             GtkSizeGroup               *size);
static void       hyscan_gtk_param_merge_clear              (HyScanGtkParam     *gtk_param);
static void       hyscan_gtk_param_merge_update             (HyScanGtkParam             *gtk_param);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkParamMerge, hyscan_gtk_param_merge, HYSCAN_TYPE_GTK_PARAM);

static void
hyscan_gtk_param_merge_class_init (HyScanGtkParamMergeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  HyScanGtkParamClass *param_class = HYSCAN_GTK_PARAM_CLASS (klass);

  object_class->constructed = hyscan_gtk_param_merge_object_constructed;
  object_class->finalize = hyscan_gtk_param_merge_object_finalize;
  param_class->clear = hyscan_gtk_param_merge_clear;
  param_class->update = hyscan_gtk_param_merge_update;
}

static void
hyscan_gtk_param_merge_init (HyScanGtkParamMerge *self)
{
  self->priv = hyscan_gtk_param_merge_get_instance_private (self);
  self->priv->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
}

static void
hyscan_gtk_param_merge_add_widgets (HyScanGtkParamMerge       *self,
                                   const HyScanDataSchemaNode *node,
                                   HyScanParamList            *plist,
                                   GtkSizeGroup               *size)
{
  HyScanGtkParamMergePrivate *priv = self->priv;
  GtkWidget *widget;
  GList *link;
  gboolean show_hidden;
  GHashTable *widgets;
  HyScanParam *param;
  HyScanDataSchema *schema;

  /* Текущий узел. */
  if (node == NULL)
    {
      node = hyscan_gtk_param_get_nodes (HYSCAN_GTK_PARAM (self));
      if (node == NULL)
        {
          g_warning ("Node is NULL. Maybe you set wrong root.");
          return;
        }
    }

  show_hidden = hyscan_gtk_param_get_show_hidden (HYSCAN_GTK_PARAM (self));
  widgets = hyscan_gtk_param_get_widgets (HYSCAN_GTK_PARAM (self));

  /* Рекурсивно идем по всем узлам. */
  for (link = node->nodes; link != NULL; link = link->next)
    {
      HyScanDataSchemaNode *subnode = link->data;
      hyscan_gtk_param_merge_add_widgets (self, subnode, plist, size);
    }

  /* А теперь по всем ключам. */
  param = hyscan_gtk_param_get_param (HYSCAN_GTK_PARAM (self));
  if (param == NULL)
    return;

  schema = hyscan_param_schema (param);
  for (link = node->keys; link != NULL; link = link->next)
    {
      HyScanDataSchemaKey *key = link->data;
      GtkWidget *wrap;

      if ((key->access & HYSCAN_DATA_SCHEMA_ACCESS_HIDDEN) && !show_hidden)
        continue;

      /* Ищем виджет. */
      widget = g_hash_table_lookup (widgets, key->id);

      /* Путь - в список отслеживаемых. */
      hyscan_param_list_add (plist, key->id);

      /* Виджет (с твиками) в контейнер. */
      g_object_set (widget, "margin-start", 12, "margin-end", 12, "margin-bottom", 6, NULL);
      hyscan_gtk_param_key_add_to_size_group (HYSCAN_GTK_PARAM_KEY (widget), size);

      wrap = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
      gtk_box_pack_start (GTK_BOX (wrap), widget, TRUE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX (priv->box), wrap, FALSE, TRUE, 0);

      /* Галочка для активации виджета. */
      if (hyscan_data_schema_key_get_access (schema, key->id) & HYSCAN_DATA_SCHEMA_ACCESS_WRITE)
        {
          GtkWidget *check_box = gtk_check_button_new ();
          gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_box),
                                        !hyscan_param_merge_get_keep (HYSCAN_PARAM_MERGE (param), key->id));
          g_object_bind_property (check_box, "active", widget, "sensitive", G_BINDING_SYNC_CREATE);
          g_object_set_data_full (G_OBJECT (check_box), "key-id", g_strdup (key->id), g_free);
          g_signal_connect (check_box, "notify::active", G_CALLBACK (hyscan_gtk_param_merge_keep_changed), self);
          gtk_box_pack_start (GTK_BOX (wrap), check_box, FALSE, FALSE, 0);
        }
    }
  g_object_unref (param);
  g_object_unref (schema);
}

/* Функция удаляет старые виджеты. */
static void
hyscan_gtk_param_merge_clear (HyScanGtkParam *gtk_param)
{
  HyScanGtkParamMerge *self = HYSCAN_GTK_PARAM_MERGE (gtk_param);

  hyscan_gtk_param_clear_container (GTK_CONTAINER (self->priv->box));
}

static void
hyscan_gtk_param_merge_update (HyScanGtkParam *gtk_param)
{
  HyScanGtkParamMerge *self = HYSCAN_GTK_PARAM_MERGE (gtk_param);
  HyScanParamList *plist;
  GtkSizeGroup *size;

  plist = hyscan_param_list_new ();

  /* Очищаем старые виджеты. */
  hyscan_gtk_param_clear_container (GTK_CONTAINER (self->priv->box));

  /* Наполняем виджетами. */
  size = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
  hyscan_gtk_param_merge_add_widgets (self, NULL, plist, size);
  gtk_widget_show_all (GTK_WIDGET (self));

  /* Устанавливаем список отслеживаемых (то есть все пути). */
  hyscan_gtk_param_set_watch_list (HYSCAN_GTK_PARAM (self), plist);

  g_object_unref (plist);
  g_object_unref (size);
}

static void
hyscan_gtk_param_merge_object_constructed (GObject *object)
{
  GtkWidget *scrolled;

  HyScanGtkParamMerge *self = HYSCAN_GTK_PARAM_MERGE (object);
  HyScanGtkParamMergePrivate *priv = self->priv;

  G_OBJECT_CLASS (hyscan_gtk_param_merge_parent_class)->constructed (object);

  /* Прокрутка области параметров. */
  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                  GTK_POLICY_NEVER,
                                  GTK_POLICY_AUTOMATIC);

  gtk_container_add (GTK_CONTAINER (scrolled), priv->box);
  g_object_set (scrolled, "hexpand", TRUE, "vexpand", TRUE, NULL);
  gtk_grid_attach (GTK_GRID (self), scrolled, 0, 0, 1, 1);
}

static void
hyscan_gtk_param_merge_object_finalize (GObject *object)
{
  G_OBJECT_CLASS (hyscan_gtk_param_merge_parent_class)->finalize (object);
}

static void
hyscan_gtk_param_merge_keep_changed (GtkToggleButton     *button,
                                     GParamSpec          *pspec,
                                     HyScanGtkParamMerge *self)
{
  HyScanParam *param;
  gboolean active;
  const gchar *key_id;
  GHashTable *widgets;
  HyScanGtkParamKey *widget;
  GVariant *variant;

  param = hyscan_gtk_param_get_param (HYSCAN_GTK_PARAM (self));
  if (!HYSCAN_IS_PARAM_MERGE (param))
    goto exit;

  key_id = g_object_get_data (G_OBJECT (button), "key-id");

  active = gtk_toggle_button_get_active (button);
  hyscan_param_merge_set_keep (HYSCAN_PARAM_MERGE (param), key_id, !active);

  /* Оповещаем, что значение ключа изменилось. */
  widgets = hyscan_gtk_param_get_widgets (HYSCAN_GTK_PARAM (self));
  widget = g_hash_table_lookup (widgets, key_id);
  variant = hyscan_gtk_param_key_get (widget);
  g_signal_emit_by_name (widget, "changed", key_id, variant);
  g_variant_unref (variant);

exit:
  g_clear_object (&param);
}

/**
 * hyscan_gtk_param_merge_new_full:
 * @param: указатель на интерфейс #HyScanParamMerge
 * @root: корневой узел схемы
 * @show_hidden: показывать ли скрытые ключи
 *
 * Функция создает виджет #HyScanGtkParamMerge.
 *
 * Returns: #HyScanGtkParamMerge.
 */
GtkWidget *
hyscan_gtk_param_merge_new_full (HyScanParamMerge *param,
                                 const gchar      *root,
                                 gboolean          show_hidden)
{
  GtkWidget *object;

  g_return_val_if_fail (HYSCAN_IS_PARAM (param), NULL);

  object = g_object_new (HYSCAN_TYPE_GTK_PARAM_MERGE, NULL);

  hyscan_gtk_param_set_param (HYSCAN_GTK_PARAM (object), HYSCAN_PARAM (param), root, show_hidden);

  return object;
}
