/**
 * \file hyscan-gtk-project-creator.c
 *
 * \brief Исходный файл виджета создания проекта.
 * \author Vladimir Maximov (vmakxs@gmail.com)
 * \date 2018
 * \license Проприетарная лицензия ООО "Экран"
 *
 */

#include "hyscan-gtk-project-creator.h"

enum
{
  PROP_O,
  PROP_RELATIVE_TO
};

enum
{
  SIGNAL_CREATE,
  SIGNAL_LAST
};

static guint hyscan_gtk_project_creator_signals[SIGNAL_LAST] = {0};

struct _HyScanGtkProjectCreatorPrivate
{
  GtkWidget *entry;       /* Поле ввода. */
  GtkWidget *create_btn;  /* Кнопка создания. */
  GtkWidget *relative_to; /* Опорный виджет. */
};

static void   hyscan_gtk_project_creator_constructed     (GObject                  *object);
static void   hyscan_gtk_project_creator_finalize        (GObject                  *object);
static void   hyscan_gtk_project_creator_set_property    (GObject                  *object,
                                                          guint                     prop_id,
                                                          const GValue             *value,
                                                          GParamSpec               *pspec);
static void   hyscan_gtk_project_creator_create_clicked  (HyScanGtkProjectCreator  *project_creator,
                                                          GtkButton                *create_btn);

G_DEFINE_TYPE_WITH_PRIVATE (HyScanGtkProjectCreator, hyscan_gtk_project_creator, GTK_TYPE_POPOVER)

static void
hyscan_gtk_project_creator_class_init (HyScanGtkProjectCreatorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = hyscan_gtk_project_creator_constructed;
  object_class->finalize = hyscan_gtk_project_creator_finalize;
  object_class->set_property = hyscan_gtk_project_creator_set_property;

  /* Сигнал, испускаемый при нажатии на кнопку создания. */
  hyscan_gtk_project_creator_signals[SIGNAL_CREATE] =
    g_signal_new ("create", HYSCAN_TYPE_GTK_PROJECT_CREATOR, G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /* Свойство для задания опорного виджета. */
  g_object_class_install_property (object_class,
                                   PROP_RELATIVE_TO,
                                   g_param_spec_object ("relative-to",
                                                        "RelativeTo",
                                                        "RelativeTo",
                                                        GTK_TYPE_WIDGET,
                                                        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void
hyscan_gtk_project_creator_init (HyScanGtkProjectCreator *project_creator)
{
  project_creator->priv = hyscan_gtk_project_creator_get_instance_private (project_creator);
}

static void
hyscan_gtk_project_creator_constructed (GObject *object)
{
  HyScanGtkProjectCreator *project_creator = HYSCAN_GTK_PROJECT_CREATOR (object);
  HyScanGtkProjectCreatorPrivate *priv = project_creator->priv;
  GtkWidget *grid;
  GtkWidget *label;

  G_OBJECT_CLASS (hyscan_gtk_project_creator_parent_class)->constructed (object);

  label = gtk_label_new ("Project");
  gtk_widget_set_halign (label, GTK_ALIGN_END);

  priv->create_btn = gtk_button_new_with_label ("Create project");
  gtk_widget_set_hexpand (priv->create_btn, TRUE);
  g_signal_connect_swapped (priv->create_btn, "clicked",
                            G_CALLBACK (hyscan_gtk_project_creator_create_clicked), project_creator);

  priv->entry = gtk_entry_new ();
  gtk_widget_set_hexpand (priv->entry, TRUE);

  grid = gtk_grid_new ();
  gtk_grid_set_column_homogeneous (GTK_GRID (grid), TRUE);
  gtk_grid_set_row_spacing (GTK_GRID (grid), 4);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 8);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), priv->entry, 1, 0, 2, 1);
  gtk_grid_attach (GTK_GRID (grid), priv->create_btn, 0, 1, 3, 1);
  gtk_widget_set_margin_start (grid, 12);
  gtk_widget_set_margin_top (grid, 12);
  gtk_widget_set_margin_end (grid, 12);
  gtk_widget_set_margin_bottom (grid, 12);

  gtk_popover_set_relative_to (GTK_POPOVER (project_creator), priv->relative_to);
  gtk_popover_set_modal (GTK_POPOVER (project_creator), TRUE);
  gtk_popover_set_position (GTK_POPOVER (project_creator), GTK_POS_BOTTOM);
  gtk_container_add (GTK_CONTAINER (project_creator), grid);
}

static void
hyscan_gtk_project_creator_finalize (GObject *object)
{
  G_OBJECT_CLASS (hyscan_gtk_project_creator_parent_class)->finalize (object);
}

static void
hyscan_gtk_project_creator_set_property (GObject      *object,
                                         guint         prop_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
  HyScanGtkProjectCreator *project_creator = HYSCAN_GTK_PROJECT_CREATOR (object);

  switch (prop_id)
    {
    case PROP_RELATIVE_TO:
      project_creator->priv->relative_to = g_value_get_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (project_creator, prop_id, pspec);
      break;
    }
}

static void
hyscan_gtk_project_creator_create_clicked (HyScanGtkProjectCreator *project_creator,
                                           GtkButton               *create_btn)
{
  (void) create_btn;
  g_signal_emit (project_creator, hyscan_gtk_project_creator_signals[SIGNAL_CREATE], 0, NULL);
}

GtkWidget *
hyscan_gtk_project_creator_new (GtkWidget *relative_to)
{
  return g_object_new (HYSCAN_TYPE_GTK_PROJECT_CREATOR, "relative-to", relative_to, NULL);
}

const gchar *
hyscan_gtk_project_creator_get_project (HyScanGtkProjectCreator *project_creator)
{
  g_return_val_if_fail (HYSCAN_IS_GTK_PROJECT_CREATOR (project_creator), NULL);
  return gtk_entry_get_text (GTK_ENTRY (project_creator->priv->entry));
}

void
hyscan_gtk_project_creator_set_project (HyScanGtkProjectCreator *project_creator,
                                        const gchar             *project)
{
  g_return_if_fail (HYSCAN_IS_GTK_PROJECT_CREATOR (project_creator));
  return gtk_entry_set_text (GTK_ENTRY (project_creator->priv->entry), project);
}
