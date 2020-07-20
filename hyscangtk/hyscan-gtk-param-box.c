/* hyscan-gtk-param-box.c
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
 * SECTION: hyscan-gtk-param-box
 * @Short_description: виджет, отображающий HyScanParam одним списком
 * @Title: HyScanGtkParamBox
 *
 * Простейший виджет, отображающий все ключи единым списком. Маловероятно, что
 * кто-то захочет его использовать, зато он хорош образовательных целях.
 */

#include "hyscan-gtk-param-box.h"

struct _HyScanGtkParamBoxPrivate
{
  GtkWidget *box;
};

static void       hyscan_gtk_param_box_object_constructed (GObject                    *object);
static void       hyscan_gtk_param_box_object_finalize    (GObject                    *object);
static void       hyscan_gtk_param_box_add_widgets        (HyScanGtkParamBox          *self,
                                                           const HyScanDataSchemaNode *node,
                                                           HyScanParamList            *plist,
                                                           GtkSizeGroup               *size);
static void       hyscan_gtk_param_box_clear              (HyScanGtkParam             *gtk_param);
static void       hyscan_gtk_param_box_update             (HyScanGtkParam             *gtk_param);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkParamBox, hyscan_gtk_param_box, HYSCAN_TYPE_GTK_PARAM);

static void
hyscan_gtk_param_box_class_init (HyScanGtkParamBoxClass *klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);
  HyScanGtkParamClass *wclass = HYSCAN_GTK_PARAM_CLASS (klass);

  oclass->constructed = hyscan_gtk_param_box_object_constructed;
  oclass->finalize = hyscan_gtk_param_box_object_finalize;
  wclass->clear = hyscan_gtk_param_box_clear;
  wclass->update = hyscan_gtk_param_box_update;
}


static void
hyscan_gtk_param_box_init (HyScanGtkParamBox *self)
{
  self->priv = hyscan_gtk_param_box_get_instance_private (self);
  self->priv->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
}

static void
hyscan_gtk_param_box_add_widgets (HyScanGtkParamBox   *self,
                                   const HyScanDataSchemaNode *node,
                                   HyScanParamList      *plist,
                                   GtkSizeGroup         *size)
{
  GtkWidget *widget, *box;
  GList *link;
  gboolean show_hidden;
  GHashTable *widgets;
  const gchar *title;

  /* Текущий узел. */
  if (node == NULL)
    return;

  show_hidden = hyscan_gtk_param_get_show_hidden (HYSCAN_GTK_PARAM (self));
  widgets = hyscan_gtk_param_get_widgets (HYSCAN_GTK_PARAM (self));

  /* Рекурсивно идем по всем узлам. */
  for (link = node->nodes; link != NULL; link = link->next)
    {
      HyScanDataSchemaNode *subnode = link->data;
      hyscan_gtk_param_box_add_widgets (self, subnode, plist, size);
    }

  /* А теперь по всем ключам. */
  if (!hyscan_gtk_param_node_has_visible_keys (node, show_hidden))
    return;

  title = node->name;
  if (title == NULL)
    title = node->name;
  if (title == NULL)
    title = node->path;

  box = gtk_list_box_new ();
  gtk_list_box_set_selection_mode (GTK_LIST_BOX (box), GTK_SELECTION_NONE);

  for (link = node->keys; link != NULL; link = link->next)
    {
      HyScanDataSchemaKey *key = link->data;

      if ((key->access & HYSCAN_DATA_SCHEMA_ACCESS_HIDDEN) && !show_hidden)
        continue;

      /* Ищем виджет. */
      widget = g_hash_table_lookup (widgets, key->id);
      // gtk_widget_show_all (widget);

      /* Путь - в список отслеживаемых. */
      hyscan_param_list_add (plist, key->id);

      /* Виджет (с твиками) в контейнер. */
      g_object_set (widget, "margin-start", 12, "margin-end", 12, "margin-bottom", 6, NULL);
      hyscan_gtk_param_key_add_to_size_group (HYSCAN_GTK_PARAM_KEY (widget), size);

      gtk_list_box_insert (GTK_LIST_BOX (box), widget, -1);
    }

  gtk_box_pack_start (GTK_BOX (self->priv->box), box, FALSE, TRUE, 0);
}

static void
hyscan_gtk_param_box_object_constructed (GObject *object)
{
  HyScanGtkParamBox *self = HYSCAN_GTK_PARAM_BOX (object);
  HyScanGtkParamBoxPrivate *priv = self->priv;

  G_OBJECT_CLASS (hyscan_gtk_param_box_parent_class)->constructed (object);

  gtk_grid_attach (GTK_GRID (self), priv->box, 0, 0, 1, 1);
}

static void
hyscan_gtk_param_box_object_finalize (GObject *object)
{
  G_OBJECT_CLASS (hyscan_gtk_param_box_parent_class)->finalize (object);
}

/* Функция удаляет старые виджеты. */
static void
hyscan_gtk_param_box_clear (HyScanGtkParam *gtk_param)
{
  HyScanGtkParamBox *self = HYSCAN_GTK_PARAM_BOX (gtk_param);

  hyscan_gtk_param_clear_container (GTK_CONTAINER (self->priv->box));
}

/* Функция размещает виджеты где следует. */
static void
hyscan_gtk_param_box_update (HyScanGtkParam *gtk_param)
{
  HyScanGtkParamBox *self = HYSCAN_GTK_PARAM_BOX (gtk_param);
  const HyScanDataSchemaNode *node;
  HyScanParamList *plist;
  GtkSizeGroup *size;

  /* Проверяем, задана ли вообще схема. Если да, то пересоздаем все виджеты. */

  node = hyscan_gtk_param_get_nodes (HYSCAN_GTK_PARAM (self));
  if (node == NULL)
    {
      g_warning ("Node is NULL. Maybe you set wrong root.");
      return;
    }

  /* Наполняем виджетами. */
  size = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
  plist = hyscan_param_list_new ();

  hyscan_gtk_param_box_add_widgets (self, node, plist, size);
  gtk_widget_show_all (GTK_WIDGET (self));

  /* Устанавливаем список отслеживаемых (то есть все пути). */
  hyscan_gtk_param_set_watch_list (HYSCAN_GTK_PARAM (self), plist);

  g_object_unref (plist);
  g_object_unref (size);
}

/**
 * hyscan_gtk_param_box_new_full:
 * @param param указатель на интерфейс HyScanParam
 * @root: корневой узел схемы
 * @show_hidden: показывать ли скрытые ключи
 *
 * Функция создает виджет #HyScanGtkParamBox.
 *
 * Returns: #HyScanGtkParamBox.
 */
GtkWidget *
hyscan_gtk_param_box_new_full (HyScanParam *param,
                                const gchar *root,
                                gboolean     show_hidden)
{
  HyScanGtkParamBox *object = g_object_new (HYSCAN_TYPE_GTK_PARAM_BOX, NULL);

  hyscan_gtk_param_set_param (HYSCAN_GTK_PARAM (object), param, root, show_hidden);

  return GTK_WIDGET (object);
}

/**
 * hyscan_gtk_param_box_new_default:
 *
 * Функция создает виджет #HyScanGtkParamBox.
 *
 * Returns: #HyScanGtkParamBox.
 */
GtkWidget *
hyscan_gtk_param_box_new_default (void)
{
  return g_object_new (HYSCAN_TYPE_GTK_PARAM_BOX, NULL);
}
