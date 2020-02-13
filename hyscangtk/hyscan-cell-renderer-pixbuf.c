/* hyscan-cell-renderer-pixbuf.h
 *
 * Copyright 2020 Screen LLC, Alexander Dmitriev <m1n7@yandex.ru>
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
 * SECTION: hyscan-cell-renderer-pixbuf
 * @Title HyScanCellRendererPixbuf
 * @Short_description
 *
 */

#include "hyscan-cell-renderer-pixbuf.h"

enum
{
  SIGNAL_ACTIVATE,
  SIGNAL_LAST
};

static guint hyscan_cell_renderer_pixbuf_signals[SIGNAL_LAST] = {0};

G_DEFINE_TYPE (HyScanCellRendererPixbuf, hyscan_cell_renderer_pixbuf, GTK_TYPE_CELL_RENDERER_PIXBUF);

static gboolean hyscan_cell_renderer_pixbuf_activate  (GtkCellRenderer      *cell,
                                                       GdkEvent             *event,
                                                       GtkWidget            *widget,
                                                       const gchar          *path,
                                                       const GdkRectangle   *background_area,
                                                       const GdkRectangle   *cell_area,
                                                       GtkCellRendererState  flags);

static void
hyscan_cell_renderer_pixbuf_class_init (HyScanCellRendererPixbufClass *klass)
{
  GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (klass);
  cell_class->activate = hyscan_cell_renderer_pixbuf_activate;

  /**
   * GtkCellRendererToggle::clicked:
   * @cell_renderer: the object which received the signal
   * @path: string representation of #GtkTreePath describing the
   *        event location
   *
   * The ::clicked signal is emitted when the cell is clicked.
   *
   * It is the responsibility of the application to update the model
   * with the correct value to store at @path.  Often this is simply the
   * opposite of the value currently stored at @path.
   **/
  hyscan_cell_renderer_pixbuf_signals[SIGNAL_ACTIVATE] =
    g_signal_new ("clicked",
                  HYSCAN_TYPE_CELL_RENDERER_PIXBUF,
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);
}

static void
hyscan_cell_renderer_pixbuf_init (HyScanCellRendererPixbuf *renderer)
{
  g_object_set (renderer, "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE, NULL);
}

static gboolean
hyscan_cell_renderer_pixbuf_activate  (GtkCellRenderer      *cell,
                                       GdkEvent             *event,
                                       GtkWidget            *widget,
                                       const gchar          *path,
                                       const GdkRectangle   *background_area,
                                       const GdkRectangle   *cell_area,
                                       GtkCellRendererState  flags)
{
  g_signal_emit (cell, hyscan_cell_renderer_pixbuf_signals[SIGNAL_ACTIVATE], 0, path);
  return TRUE;
}


GtkCellRenderer *
hyscan_cell_renderer_pixbuf_new (void)
{
  return g_object_new (HYSCAN_TYPE_CELL_RENDERER_PIXBUF,
                       NULL);
}

