/* hyscan-gtk-profile-offset-editor.h
 *
 * Copyright 2020 Screen LLC, Alexander Dmitriev <m1n7@yandex.ru>
 *
 * This file is part of @library@.
 *
 * @library@ is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * @library@ is distributed in the hope that it will be useful,
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

/* @library@ имеет двойную лицензию.
 *
 * Во-первых, вы можете распространять @library@ на условиях Стандартной
 * Общественной Лицензии GNU версии 3, либо по любой более поздней версии
 * лицензии (по вашему выбору). Полные положения лицензии GNU приведены в
 * <http://www.gnu.org/licenses/>.
 *
 * Во-вторых, этот программный код можно использовать по коммерческой
 * лицензии. Для этого свяжитесь с ООО Экран - <info@screen-co.ru>.
 */

/**
 * SECTION: hyscan-gtk-profile-offset-editor
 * @Title HyScanGtkProfileOffsetEditor
 * @Short_description
 *
 */

#include "hyscan-gtk-profile-offset-editor.h"

enum
{
  PROP_0,
};

struct _HyScanGtkProfileOffsetEditorPrivate
{

};

static void    hyscan_gtk_profile_offset_editor_set_property             (GObject               *object,
                                                                          guint                  prop_id,
                                                                          const GValue          *value,
                                                                          GParamSpec            *pspec);
static void    hyscan_gtk_profile_offset_editor_object_constructed       (GObject               *object);
static void    hyscan_gtk_profile_offset_editor_object_finalize          (GObject               *object);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkProfileOffsetEditor, hyscan_gtk_profile_offset_editor, G_TYPE_OBJECT);

static void
hyscan_gtk_profile_offset_editor_class_init (HyScanGtkProfileOffsetEditorClass *klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  oclass->set_property = hyscan_gtk_profile_offset_editor_set_property;
  oclass->constructed = hyscan_gtk_profile_offset_editor_object_constructed;
  oclass->finalize = hyscan_gtk_profile_offset_editor_object_finalize;
}

static void
hyscan_gtk_profile_offset_editor_init (HyScanGtkProfileOffsetEditor *gtk_profile_offset_editor)
{
  gtk_profile_offset_editor->priv = hyscan_gtk_profile_offset_editor_get_instance_private (gtk_profile_offset_editor);
}

static void
hyscan_gtk_profile_offset_editor_set_property (GObject      *object,
                                               guint         prop_id,
                                               const GValue *value,
                                               GParamSpec   *pspec)
{
  HyScanGtkProfileOffsetEditor *self = HYSCAN_GTK_PROFILE_OFFSET_EDITOR (object);
  HyScanGtkProfileOffsetEditorPrivate *priv = self->priv;

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
hyscan_gtk_profile_offset_editor_object_constructed (GObject *object)
{
  HyScanGtkProfileOffsetEditor *self = HYSCAN_GTK_PROFILE_OFFSET_EDITOR (object);
  HyScanGtkProfileOffsetEditorPrivate *priv = self->priv;

  /* TODO: Remove this call only when class is derived from GObject. */
  G_OBJECT_CLASS (hyscan_gtk_profile_offset_editor_parent_class)->constructed (object);
}

static void
hyscan_gtk_profile_offset_editor_object_finalize (GObject *object)
{
  HyScanGtkProfileOffsetEditor *self = HYSCAN_GTK_PROFILE_OFFSET_EDITOR (object);
  HyScanGtkProfileOffsetEditorPrivate *priv = self->priv;

  G_OBJECT_CLASS (hyscan_gtk_profile_offset_editor_parent_class)->finalize (object);
}

HyScanGtkProfileOffsetEditor *
hyscan_gtk_profile_offset_editor_new (void)
{
  return g_object_new (HYSCAN_TYPE_GTK_PROFILE_OFFSET_EDITOR,
                       NULL);
}
