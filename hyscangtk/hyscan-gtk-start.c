/* hyscan-gtk-start.h
 *
 * Copyright 2019 Screen LLC, Alexander Dmitriev <m1n7@yandex.ru>
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
 * SECTION: hyscan-gtk-start
 * @Title HyScanGtkStart
 * @Short_description
 *
 */

#include "hyscan-gtk-start.h"
#include <hyscan-gtk-profile-db.h>
#include <hyscan-gtk-profile-hw.h>
#include <hyscan-async.h>
#include <glib/gi18n-lib.h>

#define WIDGET_RESOURCE_UI "/org/hyscan/gtk/hyscan-gtk-start.ui"

enum
{
  PROP_0,
};

struct _HyScanGtkStartPrivate
{

  GtkWidget *content_area;
  GtkWidget *action_area;

  GtkWidget *project_name;
  GtkWidget *hardware_name;

  GtkWidget *project_last;
  GtkWidget *hardware_last;

  GtkWidget *buttons;
  GtkWidget *progress;

  GtkWidget *view;
  GtkWidget *scan;

  GtkWidget *status;
  GtkWidget *percentage;
  GtkWidget *cancel;

  HyScanProfileHW *hw;
  // HyScanProfileHW *hw;

};

static void    hyscan_gtk_start_set_property       (GObject        *object,
                                                    guint           prop_id,
                                                    const GValue   *value,
                                                    GParamSpec     *pspec);
static void    hyscan_gtk_start_object_constructed (GObject        *object);
static void    hyscan_gtk_start_object_finalize    (GObject        *object);

static void    hyscan_gtk_start_settings           (HyScanGtkStart *self);
static void    hyscan_gtk_start_select_project     (HyScanGtkStart *self);
static void    hyscan_gtk_start_select_hardware    (HyScanGtkStart *self);
static void    hyscan_gtk_start_view               (HyScanGtkStart *self);
static void    hyscan_gtk_start_scan               (HyScanGtkStart *self);
static void    hyscan_gtk_start_cancel             (HyScanGtkStart *self);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkStart, hyscan_gtk_start, GTK_TYPE_WINDOW);

static void
hyscan_gtk_start_class_init (HyScanGtkStartClass *klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);
  GtkWidgetClass *wclass = GTK_WIDGET_CLASS (klass);

  oclass->set_property = hyscan_gtk_start_set_property;
  oclass->constructed = hyscan_gtk_start_object_constructed;
  oclass->finalize = hyscan_gtk_start_object_finalize;

  gtk_widget_class_set_template_from_resource (wclass, WIDGET_RESOURCE_UI);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkStart, content_area);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkStart, action_area);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkStart, project_name);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkStart, hardware_name);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkStart, project_last);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkStart, hardware_last);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkStart, buttons);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkStart, progress);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkStart, view);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkStart, scan);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkStart, status);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkStart, percentage);
  gtk_widget_class_bind_template_child_private (wclass, HyScanGtkStart, cancel);

  gtk_widget_class_bind_template_callback (wclass, hyscan_gtk_start_settings);
  gtk_widget_class_bind_template_callback (wclass, hyscan_gtk_start_select_project);
  gtk_widget_class_bind_template_callback (wclass, hyscan_gtk_start_select_hardware);
  gtk_widget_class_bind_template_callback (wclass, hyscan_gtk_start_view);
  gtk_widget_class_bind_template_callback (wclass, hyscan_gtk_start_scan);
  gtk_widget_class_bind_template_callback (wclass, hyscan_gtk_start_cancel);

  g_object_class_install_property (oclass, PROP_FOLDERS,
    g_param_spec_pointer ("drivers", "Drivers", "Drivers search paths",
                          G_PARAM_CONSTRUCT | G_PARAM_WRITABLE));

  g_object_class_install_property (oclass, PROP_DRIVERS,
    g_param_spec_pointer ("drivers", "Drivers", "Drivers search paths",
                          G_PARAM_CONSTRUCT | G_PARAM_WRITABLE));
}

static void
hyscan_gtk_start_init (HyScanGtkStart *self)
{
  self->priv = hyscan_gtk_start_get_instance_private (self);
  gtk_widget_init_template (GTK_WIDGET (self));
}

static void
hyscan_gtk_start_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  HyScanGtkStart *self = HYSCAN_GTK_START (object);
  HyScanGtkStartPrivate *priv = self->priv;

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
hyscan_gtk_start_object_constructed (GObject *object)
{
  HyScanGtkStart *self = HYSCAN_GTK_START (object);
  HyScanGtkStartPrivate *priv = self->priv;

  G_OBJECT_CLASS (hyscan_gtk_start_parent_class)->constructed (object);
}

static void
hyscan_gtk_start_object_finalize (GObject *object)
{
  HyScanGtkStart *self = HYSCAN_GTK_START (object);
  HyScanGtkStartPrivate *priv = self->priv;

  G_OBJECT_CLASS (hyscan_gtk_start_parent_class)->finalize (object);
}

static void
hyscan_gtk_start_settings (HyScanGtkStart *self)
{
  g_message("not implemented");
}

static void
hyscan_gtk_start_select_project (HyScanGtkStart *self)
{
  g_message("not implemented");
}

static void
hyscan_gtk_start_select_hardware (HyScanGtkStart *self)
{
  HyScanGtkStartPrivate *priv = self->priv;
  GtkWidget *dialog, *content, *selector;
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
  gint res;

  dialog = gtk_dialog_new_with_buttons (_("Select Hardware"),
                                        GTK_WINDOW (self), flags,
                                        _("_OK"), GTK_RESPONSE_OK,
                                        _("_Cancel"), GTK_RESPONSE_CANCEL,
                                        NULL);

  content = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  selector = hyscan_gtk_profile_hw_new (priv->folders, priv->drivers, FALSE);
  gtk_container_add (GTK_CONTAINER (content), selector);

  res = gtk_dialog_run (GTK_DIALOG (dialog));
  /* Если пользователь нажмёт Ок, я просто заменю профиль. Если отмена,
   * то я попробую повторно считать профиль, на тот случай если пользователь
   * все профили удалил, а потом нажал отмену. */
  if (GTK_RESPONSE_CANCEL == res)
    {
      if (!hyscan_profile_read (HYSCAN_PROFILE (priv->hw)))
        g_clear_object (&priv->hw);
    }
  else
    {
      g_clear_object (&priv->hw);
      priv->hw = hyscan_gtk_profile_get_profile (HYSCAN_GTK_PROFILE (selector));
    }

  // TODO: update ui!
  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
hyscan_gtk_start_view (HyScanGtkStart *self)
{

}

static void
hyscan_gtk_start_scan (HyScanGtkStart *self)
{

}

static void
hyscan_gtk_start_cancel (HyScanGtkStart *self)
{

}


GtkWidget *
hyscan_gtk_start_new (void)
{
  return g_object_new (HYSCAN_TYPE_GTK_START,
                       NULL);
}
