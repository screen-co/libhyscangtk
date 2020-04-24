/* hyscan-gtk-profile-editor.c
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
 * SECTION: hyscan-gtk-profile-editor
 * @Title HyScanGtkProfileEditor
 * @Short_description: Базовый класс виджетов редактирования профилей
 *
 */

#include "hyscan-gtk-profile-editor.h"

enum
{
  PROP_0,
  PROP_PROFILE
};

enum
{
  SIGNAL_SANE,
  SIGNAL_LAST
};

struct _HyScanGtkProfileEditorPrivate
{
  HyScanProfile *profile; /* Профиль. */
  gboolean       sane;    /* Значение валидности. */
};

static void    hyscan_gtk_profile_editor_set_property             (GObject               *object,
                                                                   guint                  prop_id,
                                                                   const GValue          *value,
                                                                   GParamSpec            *pspec);
static void    hyscan_gtk_profile_editor_object_finalize          (GObject               *object);
static guint   hyscan_gtk_profile_editor_signals[SIGNAL_LAST] = {0};

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (HyScanGtkProfileEditor, hyscan_gtk_profile_editor, GTK_TYPE_GRID);

static void
hyscan_gtk_profile_editor_class_init (HyScanGtkProfileEditorClass *klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  oclass->set_property = hyscan_gtk_profile_editor_set_property;
  oclass->finalize = hyscan_gtk_profile_editor_object_finalize;

  g_object_class_install_property (oclass, PROP_PROFILE,
    g_param_spec_object ("profile", "Profile", "HyScanProfile",
                         HYSCAN_TYPE_PROFILE,
                         G_PARAM_CONSTRUCT | G_PARAM_WRITABLE));

  /**
   * HyScanGtkProfileEditor::changed:
   * @editor: the object which received the signal
   *
   * The ::changed signal is emitted when the underlying profile has changed.
   * Normally you check profile's sanity with hyscan_gtk_profile_editor_get_sanity()
   * and update your widget to reflect current state.
   **/
  hyscan_gtk_profile_editor_signals[SIGNAL_SANE] =
    g_signal_new ("changed", HYSCAN_TYPE_GTK_PROFILE_EDITOR,
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
hyscan_gtk_profile_editor_init (HyScanGtkProfileEditor *gtk_profile_editor)
{
  gtk_profile_editor->priv = hyscan_gtk_profile_editor_get_instance_private (gtk_profile_editor);
}

static void
hyscan_gtk_profile_editor_set_property (GObject      *object,
                                        guint         prop_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  HyScanGtkProfileEditor *self = HYSCAN_GTK_PROFILE_EDITOR (object);
  HyScanGtkProfileEditorPrivate *priv = self->priv;

  switch (prop_id)
    {
    case PROP_PROFILE:
      priv->profile = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
hyscan_gtk_profile_editor_object_finalize (GObject *object)
{
  HyScanGtkProfileEditor *self = HYSCAN_GTK_PROFILE_EDITOR (object);

  g_clear_object (&self->priv->profile);

  G_OBJECT_CLASS (hyscan_gtk_profile_editor_parent_class)->finalize (object);
}

/**
 * hyscan_gtk_profile_editor_get_profile:
 * @self: #HyScanGtkProfileEditor
 *
 * Returns: (transfer full): указатель на HyScanProfile, редактируемый виджетом
 */
HyScanProfile *
hyscan_gtk_profile_editor_get_profile (HyScanGtkProfileEditor *self)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_PROFILE_EDITOR (self), NULL);

  return g_object_ref (self->priv->profile);
}

/**
 * hyscan_gtk_profile_editor_check_sanity:
 * @self: #HyScanGtkProfileEditor
 *
 * Функция инициирует валидацию профиля и эмитирует HyScanGtkProfileEditor::changed,
 * если состояние изменилось.
 */
void
hyscan_gtk_profile_editor_check_sanity (HyScanGtkProfileEditor *self)
{
  gboolean old_sanity, new_sanity;
  HyScanGtkProfileEditorPrivate *priv;

  g_return_if_fail (HYSCAN_IS_GTK_PROFILE_EDITOR (self));
  priv = self->priv;

  new_sanity = hyscan_profile_sanity (priv->profile);
  old_sanity = priv->sane;
  priv->sane = new_sanity;

  if (new_sanity != old_sanity)
    g_signal_emit (self, hyscan_gtk_profile_editor_signals[SIGNAL_SANE], 0, new_sanity);
}

/**
 * hyscan_gtk_profile_editor_get_sanity:
 * @self: #HyScanGtkProfileEditor
 *
 * Функция возвращает валидность профиля.
 *
 * Returns: %TRUE, если профиль валиден.
 */
gboolean
hyscan_gtk_profile_editor_get_sanity (HyScanGtkProfileEditor *self)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_PROFILE_EDITOR (self), FALSE);

  return self->priv->sane;
}
