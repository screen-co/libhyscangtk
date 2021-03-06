/* hyscan-gtk-spin-button.h
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

#ifndef __HYSCAN_GTK_SPIN_BUTTON_H__
#define __HYSCAN_GTK_SPIN_BUTTON_H__

#include <hyscan-api.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HYSCAN_TYPE_GTK_SPIN_BUTTON             (hyscan_gtk_spin_button_get_type ())
#define HYSCAN_GTK_SPIN_BUTTON(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_SPIN_BUTTON, HyScanGtkSpinButton))
#define HYSCAN_IS_GTK_SPIN_BUTTON(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_SPIN_BUTTON))
#define HYSCAN_GTK_SPIN_BUTTON_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_SPIN_BUTTON, HyScanGtkSpinButtonClass))
#define HYSCAN_IS_GTK_SPIN_BUTTON_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_SPIN_BUTTON))
#define HYSCAN_GTK_SPIN_BUTTON_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_SPIN_BUTTON, HyScanGtkSpinButtonClass))

typedef struct _HyScanGtkSpinButton HyScanGtkSpinButton;
typedef struct _HyScanGtkSpinButtonClass HyScanGtkSpinButtonClass;

struct _HyScanGtkSpinButtonClass
{
  GtkSpinButtonClass parent_class;
};

HYSCAN_API
GType                  hyscan_gtk_spin_button_get_type         (void);

HYSCAN_API
GtkWidget            * hyscan_gtk_spin_button_new              (void);

HYSCAN_API
void                   hyscan_gtk_spin_button_set_base         (HyScanGtkSpinButton *spin,
                                                                guint                base);

HYSCAN_API
void                   hyscan_gtk_spin_button_set_show_prefix  (HyScanGtkSpinButton *spin,
                                                                gboolean             show);

G_END_DECLS

#endif /* __HYSCAN_GTK_SPIN_BUTTON_H__ */
