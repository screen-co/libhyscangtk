/* hyscan-gtk-paned.c
 *
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
 * SECTION: hyscan-gtk-paned
 * @Title HyScanGtkPaned
 * @Short_description виджет скрывающихся панелей
 *
 * Виджет HyScanGtkPaned содержит две области, размеры которых можно менять мышью,
 * как в #GtkPaned.
 *
 * Верхняя область содержит центральный виджет, его можно установить с помощью функции
 * hyscan_gtk_paned_set_central_widget().
 *
 * Нижняя область показывает одну из нескольких панелей, добавленных через
 * hyscan_gtk_paned_add(). Пользователь может переключать панели или скрывать
 * область панелей вообще.
 */

#include "hyscan-gtk-paned.h"

typedef struct
{
  HyScanGtkPaned   *paned;         /* Указатель на объект виджета. */
  gchar            *name;          /* Уникальное имя панели. */
  GtkWidget        *widget;        /* Виджет панели. */
  GtkWidget        *button;        /* Кнопка выбора этой панели. */
  gulong            handler_id;    /* Ид обработчика сигнала активации кнопки. */
} HyScanGtkPanedChild;

struct _HyScanGtkPanedPrivate
{
  GList                *children;  /* Список панелей. */
  GtkWidget            *buttons;   /* Кнопки переключения панелей. */
  GtkWidget            *stack;     /* Стек с панелями. */
  GtkWidget            *revealer;  /* Контейнер, скрывающий панели. */
  GtkWidget            *paned;     /* Контейнер для главного виджета и панелей, с возможностью изменения их размеров. */
  gint                  position;  /* Положение paned, когда панель открыта. */
};

static void     hyscan_gtk_paned_object_constructed       (GObject               *object);
static void     hyscan_gtk_paned_object_finalize          (GObject               *object);
static void     hyscan_gtk_paned_position                 (HyScanGtkPaned        *self);
static void     hyscan_gtk_paned_revealer_closed          (HyScanGtkPaned        *self);
static void     hyscan_gtk_paned_child_free               (HyScanGtkPanedChild   *child);
static void     hyscan_gtk_paned_btn_active               (HyScanGtkPanedChild   *clicked_child);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkPaned, hyscan_gtk_paned, GTK_TYPE_BIN)

static void
hyscan_gtk_paned_class_init (HyScanGtkPanedClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = hyscan_gtk_paned_object_constructed;
  object_class->finalize = hyscan_gtk_paned_object_finalize;
}

static void
hyscan_gtk_paned_init (HyScanGtkPaned *gtk_paned)
{
  gtk_paned->priv = hyscan_gtk_paned_get_instance_private (gtk_paned);
}

static void
hyscan_gtk_paned_object_constructed (GObject *object)
{
  HyScanGtkPaned *gtk_paned = HYSCAN_GTK_PANED (object);
  HyScanGtkPanedPrivate *priv = gtk_paned->priv;
  GtkWidget *box;

  G_OBJECT_CLASS (hyscan_gtk_paned_parent_class)->constructed (object);

  /* Кнопки переключения панелей. */
  priv->buttons = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  g_object_set (priv->buttons, "margin", 6, NULL);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (priv->buttons), GTK_BUTTONBOX_EXPAND);
  gtk_widget_set_halign (priv->buttons, GTK_ALIGN_CENTER);

  /* Стек с панелями внутри GtkRevealer. */
  priv->stack = gtk_stack_new ();
  priv->revealer = gtk_revealer_new ();
  gtk_revealer_set_transition_type (GTK_REVEALER (priv->revealer), GTK_REVEALER_TRANSITION_TYPE_NONE);
  gtk_container_add (GTK_CONTAINER (priv->revealer), priv->stack);

  priv->paned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
  gtk_paned_pack2 (GTK_PANED (priv->paned), priv->revealer, FALSE, FALSE);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (box), priv->paned, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box), priv->buttons, FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (gtk_paned), box);

  g_signal_connect_swapped (priv->revealer, "notify::child-revealed",
                            G_CALLBACK (hyscan_gtk_paned_revealer_closed), gtk_paned);
  g_signal_connect_swapped (priv->paned, "notify::position",
                            G_CALLBACK (hyscan_gtk_paned_position), gtk_paned);
}

static void
hyscan_gtk_paned_object_finalize (GObject *object)
{
  HyScanGtkPaned *gtk_paned = HYSCAN_GTK_PANED (object);
  HyScanGtkPanedPrivate *priv = gtk_paned->priv;

  g_list_free_full (priv->children, (GDestroyNotify) hyscan_gtk_paned_child_free);

  G_OBJECT_CLASS (hyscan_gtk_paned_parent_class)->finalize (object);
}

static void
hyscan_gtk_paned_child_free (HyScanGtkPanedChild *child)
{
  g_free (child->name);
  g_free (child);
}

/* Управляет размером области панелей. */
static void
hyscan_gtk_paned_revealer_closed (HyScanGtkPaned *self)
{
  HyScanGtkPanedPrivate *priv = self->priv;
  GtkRevealer *revealer = GTK_REVEALER (priv->revealer);
  gboolean revealed;
  gint height;

  revealed = gtk_revealer_get_child_revealed (revealer);
  height = gtk_widget_get_allocated_height (GTK_WIDGET (priv->paned));
  if (revealed)
    {
      gtk_paned_set_position (GTK_PANED (priv->paned), height - priv->position);
    }
  else
    {
      priv->position = height - gtk_paned_get_position (GTK_PANED (priv->paned));
      gtk_paned_set_position (GTK_PANED (priv->paned), height);
    }
}

/* Запрещает изменять размер GtkPaned, если область панелей закрыта. */
static void
hyscan_gtk_paned_position (HyScanGtkPaned *self)
{
  HyScanGtkPanedPrivate *priv = self->priv;
  GtkRevealer *revealer = GTK_REVEALER (priv->revealer);
  gboolean revealed;
  gint height;

  revealed = gtk_revealer_get_child_revealed (revealer);
  if (revealed)
    return;

  /* Если панели скрыты, то в GtkPaned показываем только центральный виджет. */
  height = gtk_widget_get_allocated_height (GTK_WIDGET (priv->paned));
  gtk_paned_set_position (GTK_PANED (priv->paned), height);
}

/* Переключает панели и скрывает или открывает область панелей при нажатии на кнопку. */
static void
hyscan_gtk_paned_btn_active (HyScanGtkPanedChild *clicked_child)
{
  HyScanGtkPanedPrivate *priv = clicked_child->paned->priv;
  gboolean active;
  GList *link;

  for (link = priv->children; link != NULL; link = link->next)
    {
      HyScanGtkPanedChild *child = link->data;

      if (child == clicked_child || !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (child->button)))
        continue;

      g_signal_handler_block (child->button, child->handler_id);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (child->button), FALSE);
      g_signal_handler_unblock (child->button, child->handler_id);
    }

  active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (clicked_child->button));
  gtk_stack_set_visible_child (GTK_STACK (priv->stack), clicked_child->widget);

  gtk_revealer_set_reveal_child (GTK_REVEALER (priv->revealer), active);
}

/**
 * hyscan_gtk_paned_new:
 *
 * Создаёт новый виджет #HyScanGtkPaned.
 *
 * Returns: виджет #HyScanGtkPaned.
 */
GtkWidget *
hyscan_gtk_paned_new (void)
{
  return g_object_new (HYSCAN_TYPE_GTK_PANED, NULL);
}

/**
 * hyscan_gtk_paned_set_center_widget:
 * @paned: указатель на #HyScanGtkPaned
 * @widget: указатель на центральный виджет
 *
 * Устанавливает основной центральный виджет.
 */
void
hyscan_gtk_paned_set_center_widget (HyScanGtkPaned *paned,
                                    GtkWidget      *widget)
{
  HyScanGtkPanedPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_PANED (paned));

  priv = paned->priv;

  gtk_paned_pack1 (GTK_PANED (priv->paned), widget, TRUE, FALSE);
}

/**
 * hyscan_gtk_paned_add:
 * @paned: указатель на #HyScanGtkPaned
 * @pane: указатель на виджет панели
 * @name: уникальный идентификатор панели
 * @title: (nullable): заголовок панели
 * @icon_name: (nullable): имя иконки панели
 *
 * Добавляет панель в стек панелей.
 */
void
hyscan_gtk_paned_add (HyScanGtkPaned *paned,
                      GtkWidget      *pane,
                      const gchar    *name,
                      const gchar    *title,
                      const gchar    *icon_name)
{
  HyScanGtkPanedPrivate *priv;
  HyScanGtkPanedChild *child;

  g_return_if_fail (HYSCAN_IS_GTK_PANED (paned));

  priv = paned->priv;

  child = g_new0 (HyScanGtkPanedChild, 1);
  child->paned = paned;
  child->name = g_strdup (name);
  child->button = gtk_toggle_button_new_with_label (title);
  child->widget = pane;
  child->handler_id = g_signal_connect_swapped (child->button,
                                                "notify::active",
                                                G_CALLBACK (hyscan_gtk_paned_btn_active),
                                                child);

  if (icon_name != NULL)
    gtk_button_set_image (GTK_BUTTON (child->button), gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_BUTTON));

  gtk_stack_add_named (GTK_STACK (priv->stack), pane, name);
  gtk_box_pack_start (GTK_BOX (priv->buttons), child->button, FALSE, TRUE, 0);

  priv->children = g_list_append (priv->children, child);
}
