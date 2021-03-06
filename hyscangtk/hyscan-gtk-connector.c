/* hyscan-gtk-connector.h
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
 * SECTION: hyscan-gtk-connector
 * @Title: HyScanGtkConnector
 * @Short_description: Виджет подключения к профилям.
 *
 * Имеет ряд недостатков. Увы.
 */

#include <hyscan-gtk-connector.h>
#include <hyscan-gtk-profile-db.h>
#include <hyscan-gtk-profile-hw.h>
#include <hyscan-gtk-profile-offset.h>
#include <hyscan-async.h>
#include <glib/gi18n-lib.h>

enum
{
  PROP_0,
  PROP_DRIVERS,
  PROP_SYSFOLDER,
};

struct _HyScanGtkConnectorPrivate
{
  gchar        **drivers;
  gchar        **folders;

  HyScanProfile *db_profile;
  HyScanProfile *hw_profile;
  HyScanProfile *of_profile;

  struct
    {
      GtkWidget *bar;
      GtkWidget *label;
    } progress;

  gchar         *db_profile_file;
  gchar         *hw_profile_file;
  gchar         *of_profile_file;

  gint           offset_page;
  gint           connect_page;
  gint           previous_page;

  HyScanAsync   *async;
  HyScanDB      *db;
  HyScanControl *control;

  gboolean       result;
};

static void    hyscan_gtk_connector_set_property             (GObject               *object,
                                                              guint                  prop_id,
                                                              const GValue          *value,
                                                              GParamSpec            *pspec);
static void    hyscan_gtk_connector_object_constructed       (GObject               *object);
static void    hyscan_gtk_connector_object_finalize          (GObject               *object);

static void    hyscan_gtk_connector_prepare                  (HyScanGtkConnector    *self,
                                                              GtkWidget             *page);

static void    hyscan_gtk_connector_make_intro_page          (HyScanGtkConnector    *self);
static void    hyscan_gtk_connector_make_db_page             (HyScanGtkConnector    *self);
static void    hyscan_gtk_connector_make_hw_page             (HyScanGtkConnector    *self);
static void    hyscan_gtk_connector_make_offset_page         (HyScanGtkConnector    *self);
static void    hyscan_gtk_connector_make_confirm_page        (HyScanGtkConnector    *self);
static void    hyscan_gtk_connector_make_connect_page        (HyScanGtkConnector    *self);

static void    hyscan_gtk_connector_selected_db              (HyScanGtkProfile      *page,
                                                              HyScanProfile         *profile,
                                                              HyScanGtkConnector    *self);
static void    hyscan_gtk_connector_selected_hw              (HyScanGtkProfile      *page,
                                                              HyScanProfile         *profile,
                                                              HyScanGtkConnector    *self);
static void    hyscan_gtk_connector_selected_offset          (HyScanGtkProfile      *page,
                                                              HyScanProfile         *profile,
                                                              HyScanGtkConnector    *self);

static void    hyscan_gtk_connector_apply                    (HyScanGtkConnector    *self);
static void    hyscan_gtk_connector_done_db                  (HyScanGtkConnector    *self,
                                                              gpointer               res);
static void    hyscan_gtk_connector_done_hw_check            (HyScanGtkConnector    *self,
                                                              gpointer               res);
static void    hyscan_gtk_connector_done_hw                  (HyScanGtkConnector    *self,
                                                              gpointer               res);
static void    hyscan_gtk_connector_done_bind                (HyScanGtkConnector    *self,
                                                              gpointer               res);
static void    hyscan_gtk_connector_done_of                  (HyScanGtkConnector    *self,
                                                              gpointer               res);
static void    hyscan_gtk_connector_finished                 (HyScanGtkConnector    *self,
                                                              gboolean               success);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkConnector, hyscan_gtk_connector, GTK_TYPE_ASSISTANT);

static void
hyscan_gtk_connector_class_init (HyScanGtkConnectorClass *klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  oclass->set_property = hyscan_gtk_connector_set_property;
  oclass->constructed = hyscan_gtk_connector_object_constructed;
  oclass->finalize = hyscan_gtk_connector_object_finalize;

  g_object_class_install_property (oclass, PROP_DRIVERS,
    g_param_spec_pointer ("drivers", "DriverPaths", "Where to look for drivers",
                          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (oclass, PROP_SYSFOLDER,
    g_param_spec_pointer ("folders", "Folders", "Folders with profiles",
                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void
hyscan_gtk_connector_init (HyScanGtkConnector *gtk_connector)
{
  gtk_connector->priv = hyscan_gtk_connector_get_instance_private (gtk_connector);
}

static void
hyscan_gtk_connector_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  HyScanGtkConnector *self = HYSCAN_GTK_CONNECTOR (object);

  switch (prop_id)
    {
    case PROP_DRIVERS:
      self->priv->drivers = g_strdupv (g_value_get_pointer (value));
      break;

    case PROP_SYSFOLDER:
      self->priv->folders = g_strdupv((gchar**) g_value_get_pointer (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
hyscan_gtk_connector_object_constructed (GObject *object)
{
  HyScanGtkConnector *self = HYSCAN_GTK_CONNECTOR (object);
  HyScanGtkConnectorPrivate *priv = self->priv;

  G_OBJECT_CLASS (hyscan_gtk_connector_parent_class)->constructed (object);

  priv->previous_page = -1;

  priv->async = hyscan_async_new ();
  hyscan_gtk_connector_make_intro_page (self);

  hyscan_gtk_connector_make_db_page (self);
  hyscan_gtk_connector_make_hw_page (self);
  hyscan_gtk_connector_make_offset_page (self);

  hyscan_gtk_connector_make_confirm_page (self);
  hyscan_gtk_connector_make_connect_page (self);

  g_signal_connect (self, "apply", G_CALLBACK (hyscan_gtk_connector_apply), NULL);
  g_signal_connect (self, "prepare", G_CALLBACK (hyscan_gtk_connector_prepare), NULL);
}

static void
hyscan_gtk_connector_object_finalize (GObject *object)
{
  HyScanGtkConnector *self = HYSCAN_GTK_CONNECTOR (object);
  HyScanGtkConnectorPrivate *priv = self->priv;

  g_clear_object (&priv->async);
  g_clear_object (&priv->db_profile);
  g_clear_object (&priv->hw_profile);
  g_clear_object (&priv->of_profile);

  g_clear_object (&priv->db);
  g_clear_object (&priv->control);

  g_clear_pointer (&priv->drivers, g_strfreev);
  g_clear_pointer (&priv->folders, g_strfreev);

  G_OBJECT_CLASS (hyscan_gtk_connector_parent_class)->finalize (object);
}

static void
hyscan_gtk_connector_prepare (HyScanGtkConnector *self,
                              GtkWidget          *page)
{
  GtkAssistant *assistant = GTK_ASSISTANT (self);
  HyScanGtkConnectorPrivate *priv = self->priv;
  gint cur_page, prev_page;

  cur_page = gtk_assistant_get_current_page (assistant);
  prev_page = priv->previous_page;
  priv->previous_page = cur_page;

  /* Пропуск страницы с оффсетами, если не выбран профиль оборудования. */
  if (cur_page == priv->offset_page && priv->hw_profile == NULL)
    {
      if (prev_page < cur_page)
        gtk_assistant_next_page (assistant);
      else
        gtk_assistant_previous_page (assistant);
    }
  if (cur_page == priv->connect_page)
    gtk_assistant_commit (assistant);
}

static void
hyscan_gtk_connector_make_intro_page (HyScanGtkConnector *self)
{
  GtkWidget *box, *label;

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);

  label = gtk_label_new (_("Welcome to Connection Manager"));

  gtk_box_pack_start (GTK_BOX (box), label, TRUE, FALSE, 0);
  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (box, GTK_ALIGN_CENTER);

  gtk_assistant_append_page (GTK_ASSISTANT (self), box);

  gtk_assistant_set_page_type (GTK_ASSISTANT (self), box, GTK_ASSISTANT_PAGE_INTRO);
  gtk_assistant_set_page_title (GTK_ASSISTANT (self), box, _("Intro"));
  gtk_assistant_set_page_complete (GTK_ASSISTANT (self), box, TRUE);
}

static void
hyscan_gtk_connector_make_db_page (HyScanGtkConnector *self)
{
  GtkWidget *page = hyscan_gtk_profile_db_new (self->priv->folders, FALSE);

  g_signal_connect (page, "selected",
                    G_CALLBACK (hyscan_gtk_connector_selected_db), self);
  gtk_assistant_append_page (GTK_ASSISTANT (self), page);
  gtk_assistant_set_page_title (GTK_ASSISTANT (self), page, _("Database"));
}

static void
hyscan_gtk_connector_make_hw_page (HyScanGtkConnector *self)
{
  GtkWidget *page = hyscan_gtk_profile_hw_new (self->priv->folders, self->priv->drivers, FALSE);

  g_signal_connect (page, "selected",
                    G_CALLBACK (hyscan_gtk_connector_selected_hw), self);

  gtk_assistant_append_page (GTK_ASSISTANT (self), page);
  gtk_assistant_set_page_title (GTK_ASSISTANT (self), page, _("Hardware"));
  gtk_assistant_set_page_complete (GTK_ASSISTANT (self), page, TRUE);
}

static void
hyscan_gtk_connector_make_offset_page (HyScanGtkConnector *self)
{
  GtkWidget *page = hyscan_gtk_profile_offset_new (self->priv->folders, FALSE);

  g_signal_connect (page, "selected",
                    G_CALLBACK (hyscan_gtk_connector_selected_offset), self);
  self->priv->offset_page = gtk_assistant_append_page (GTK_ASSISTANT (self), page);
  gtk_assistant_set_page_title (GTK_ASSISTANT (self), page, _("Antennas positions"));
  gtk_assistant_set_page_complete (GTK_ASSISTANT (self), page, TRUE);
}

static void
hyscan_gtk_connector_make_confirm_page (HyScanGtkConnector *self)
{
  GtkWidget *page = gtk_label_new (_("Review your selections"));

  gtk_assistant_append_page (GTK_ASSISTANT (self), page);
  gtk_assistant_set_page_type (GTK_ASSISTANT (self), page, GTK_ASSISTANT_PAGE_CONFIRM);
  gtk_assistant_set_page_title (GTK_ASSISTANT (self), page, _("Confirm"));
  gtk_assistant_set_page_complete (GTK_ASSISTANT (self), page, TRUE);
}

static void
hyscan_gtk_connector_make_connect_page (HyScanGtkConnector *self)
{
  GtkWidget *box;
  HyScanGtkConnectorPrivate *priv = self->priv;

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);

  priv->progress.label = gtk_label_new (NULL);
  priv->progress.bar = gtk_progress_bar_new ();

  gtk_box_pack_start (GTK_BOX (box), priv->progress.label, TRUE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box), priv->progress.bar, FALSE, FALSE, 0);
  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (box, GTK_ALIGN_CENTER);

  self->priv->connect_page = gtk_assistant_append_page (GTK_ASSISTANT (self), box);
  gtk_assistant_set_page_type (GTK_ASSISTANT (self), box, GTK_ASSISTANT_PAGE_SUMMARY);
  gtk_assistant_set_page_title (GTK_ASSISTANT (self), box, _("Connection"));
  gtk_assistant_set_page_complete (GTK_ASSISTANT (self), box, FALSE);
}

static void
hyscan_gtk_connector_selected_db (HyScanGtkProfile   *page,
                                  HyScanProfile      *profile,
                                  HyScanGtkConnector *self)
{
  g_clear_object (&self->priv->db_profile);
  g_clear_pointer (&self->priv->db_profile_file, g_free);

  if (profile != NULL)
    {
      self->priv->db_profile = g_object_ref (profile);
      self->priv->db_profile_file = g_strdup (hyscan_profile_get_file (profile));
    }

  gtk_assistant_set_page_complete (GTK_ASSISTANT (self), GTK_WIDGET (page), profile != NULL);
}

static void
hyscan_gtk_connector_selected_hw (HyScanGtkProfile   *page,
                                  HyScanProfile      *profile,
                                  HyScanGtkConnector *self)
{
  g_clear_object (&self->priv->hw_profile);
  g_clear_pointer (&self->priv->hw_profile_file, g_free);

  if (profile != NULL)
    {
      self->priv->hw_profile = g_object_ref (profile);
      self->priv->hw_profile_file = g_strdup (hyscan_profile_get_file (profile));
    }
}

static void
hyscan_gtk_connector_selected_offset (HyScanGtkProfile   *page,
                                      HyScanProfile      *profile,
                                      HyScanGtkConnector *self)
{
  g_clear_object (&self->priv->of_profile);
  g_clear_pointer (&self->priv->of_profile_file, g_free);

  if (profile != NULL)
    {
      self->priv->of_profile = g_object_ref (profile);
      self->priv->of_profile_file = g_strdup (hyscan_profile_get_file (profile));
    }
}

static void
hyscan_gtk_connector_apply (HyScanGtkConnector *self)
{
  HyScanGtkConnectorPrivate *priv = self->priv;

  gtk_label_set_text (GTK_LABEL (priv->progress.label), _("Connecting to database..."));
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (priv->progress.bar), 0.0);

  hyscan_async_append (priv->async,
                       (HyScanAsyncCommand)hyscan_profile_db_connect,
                       priv->db_profile, NULL,
                       (HyScanAsyncResult)hyscan_gtk_connector_done_db, self);
}

static void
hyscan_gtk_connector_done_db (HyScanGtkConnector *self,
                              gpointer            res)
{
  HyScanDB *db = HYSCAN_DB (res);
  HyScanGtkConnectorPrivate *priv = self->priv;

  if (db == NULL)
    {
      g_warning ("DBProfile connection failed");
      hyscan_gtk_connector_finished (self, FALSE);
      return;
    }

  priv->db = db;
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (priv->progress.bar), 0.3);

  if (priv->hw_profile == NULL)
    {
      hyscan_gtk_connector_finished (self, TRUE);
      return;
    }

  gtk_label_set_text (GTK_LABEL (priv->progress.label), _("Checking sonars..."));
  hyscan_async_append (priv->async,
                       (HyScanAsyncCommand)hyscan_profile_hw_check,
                       priv->hw_profile, NULL,
                       (HyScanAsyncResult)hyscan_gtk_connector_done_hw_check, self);
}

static void
hyscan_gtk_connector_done_hw_check (HyScanGtkConnector *self,
                                    gpointer            res)
{
  gboolean check = GPOINTER_TO_INT (res);
  HyScanGtkConnectorPrivate *priv = self->priv;

  if (!check)
    {
      g_warning ("HWProfile check failed");
      hyscan_gtk_connector_finished (self, FALSE);
      return;
    }

  gtk_label_set_text (GTK_LABEL (self->priv->progress.label), _("Connecting to sonars..."));
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (self->priv->progress.bar), 0.5);

  hyscan_async_append (priv->async,
                       (HyScanAsyncCommand)hyscan_profile_hw_connect,
                       self->priv->hw_profile, NULL,
                       (HyScanAsyncResult)hyscan_gtk_connector_done_hw, self);
}

static void
hyscan_gtk_connector_done_hw (HyScanGtkConnector *self,
                              gpointer            res)
{
  HyScanControl *control = HYSCAN_CONTROL (res);
  HyScanGtkConnectorPrivate *priv = self->priv;

  if (control == NULL)
    {
      g_warning ("HWProfile connection failed");
      hyscan_gtk_connector_finished (self, FALSE);
      return;
    }

  priv->control = control;

  gtk_label_set_text (GTK_LABEL (priv->progress.label), _("Connecting to sonars..."));
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (priv->progress.bar), 0.5);

  hyscan_async_append (priv->async,
                       (HyScanAsyncCommand)hyscan_control_device_bind,
                       priv->control, NULL,
                       (HyScanAsyncResult)hyscan_gtk_connector_done_bind, self);
}

static void
hyscan_gtk_connector_done_bind (HyScanGtkConnector *self,
                                gpointer            res)
{
  gboolean check = GPOINTER_TO_INT (res);
  HyScanGtkConnectorPrivate *priv = self->priv;

  if (!check)
    {
      g_warning ("HWProfile binding failed");
      hyscan_gtk_connector_finished (self, FALSE);
      return;
    }

  if (priv->of_profile == NULL)
    {
      hyscan_gtk_connector_finished (self, TRUE);
      return;
    }

  gtk_label_set_text (GTK_LABEL (priv->progress.label), _("Setting up offsets..."));
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (priv->progress.bar), 0.9);

  hyscan_async_append (priv->async,
                       (HyScanAsyncCommand)hyscan_profile_offset_apply,
                       priv->of_profile, priv->control,
                       (HyScanAsyncResult)hyscan_gtk_connector_done_of, self);
}

static void
hyscan_gtk_connector_done_of (HyScanGtkConnector *self,
                              gpointer            res)
{
  gboolean check = GPOINTER_TO_INT (res);

  if (!check)
    {
      g_warning ("ProfileOffset apply failed");
      hyscan_gtk_connector_finished (self, FALSE);
    }
  else
    {
      hyscan_gtk_connector_finished (self, TRUE);
    }
}

static void
hyscan_gtk_connector_finished (HyScanGtkConnector *self,
                               gboolean            success)
{
  GtkAssistant *wizard = GTK_ASSISTANT (self);
  GtkWidget *current_page;
  GtkLabel *label = GTK_LABEL (self->priv->progress.label);

  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (self->priv->progress.bar), 1.0);

  current_page = gtk_assistant_get_nth_page (wizard, gtk_assistant_get_current_page (wizard));
  gtk_assistant_set_page_complete (wizard, current_page, TRUE);

  if (success)
    gtk_label_set_text (label, _("Connection completed. HyScan is ready to launch. "));
  else
    gtk_label_set_text (label, _("Connection failed. "));

  self->priv->result = success;
}

GtkWidget *
hyscan_gtk_connector_new (gchar **folders,
                          gchar **drivers)
{
  return g_object_new (HYSCAN_TYPE_GTK_CONNECTOR,
                       "use-header-bar", TRUE,
                       "drivers", drivers,
                       "folders", folders,
                       NULL);
}

gboolean
hyscan_gtk_connector_get_result (HyScanGtkConnector *self)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_CONNECTOR (self), FALSE);

  return self->priv->result;
}

HyScanDB *
hyscan_gtk_connector_get_db (HyScanGtkConnector *self)
{
  HyScanGtkConnectorPrivate *priv;

  g_return_val_if_fail (HYSCAN_IS_GTK_CONNECTOR (self), NULL);
  priv = self->priv;

  return (priv->db != NULL) ? g_object_ref (priv->db) : NULL;
}

HyScanControl *
hyscan_gtk_connector_get_control (HyScanGtkConnector *self)
{
  HyScanGtkConnectorPrivate *priv;

  g_return_val_if_fail (HYSCAN_IS_GTK_CONNECTOR (self), NULL);
  priv = self->priv;

  return (priv->control != NULL) ? g_object_ref (priv->control) : NULL;
}

const gchar *
hyscan_gtk_connector_get_db_name (HyScanGtkConnector *self)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_CONNECTOR (self), NULL);

  return self->priv->db_profile_file;
}

const gchar *
hyscan_gtk_connector_get_hw_name (HyScanGtkConnector *self)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_CONNECTOR (self), NULL);

  return self->priv->hw_profile_file;
}
const gchar *
hyscan_gtk_connector_get_offset_name (HyScanGtkConnector *self)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_CONNECTOR (self), NULL);

  return self->priv->of_profile_file;
}
