#include <string.h>
#include "hyscan-gtk-pane.h"

GType
hyscan_gtk_pane_style_get_type (void)
{
    static GType etype = 0;
    if (G_UNLIKELY (etype == 0)) {
        static const GEnumValue values[] = {
            { HYSCAN_GTK_PANE_TEXT, "HYSCAN_GTK_PANE_TEXT", "text" },
            { HYSCAN_GTK_PANE_ARROW, "HYSCAN_GTK_PANE_ARROW", "arrow" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static (g_intern_static_string ("HyScanGtkPaneStyle"), values);
    }
    return etype;
}

#define DEFAULT_HEADER_STYLE HYSCAN_GTK_PANE_TEXT

enum
{
  PROP_0,
  PROP_BODY,
  PROP_TITLE,
  PROP_BOLD,
  PROP_HEADER_STYLE
};

struct _HyScanGtkPane
{
  GtkGrid               parent_instance;

  gchar                *title;
  GtkWidget            *body;
  gboolean              bold;
  HyScanGtkPaneStyle    header_style;

  GtkWidget            *ebox;
  GtkWidget            *expander;
  GtkWidget            *revealer;
};

static void         hyscan_gtk_pane_set_property        (GObject         *object,
                                                         guint            prop_id,
                                                         const GValue    *value,
                                                         GParamSpec      *pspec);
static void         hyscan_gtk_pane_constructed         (GObject         *object);
static void         hyscan_gtk_pane_finalize            (GObject         *object);
static void         hyscan_gtk_pane_setup               (HyScanGtkPane   *pane);
static GtkWidget*   hyscan_gtk_pane_make_text_header    (HyScanGtkPane   *pane);
static GtkWidget*   hyscan_gtk_pane_make_arrow_header   (HyScanGtkPane   *pane);
static gboolean     hyscan_gtk_pane_expand_text         (HyScanGtkPane   *pane,
                                                         GdkEvent        *event,
                                                         GtkWidget       *ebox);
static gboolean     hyscan_gtk_pane_expand_arrow        (HyScanGtkPane   *pane,
                                                         GdkEvent        *event,
                                                         GtkWidget       *ebox);
static GtkWidget   *hyscan_gtk_pane_make_right_arrow    (void);
static GtkWidget   *hyscan_gtk_pane_make_down_arrow     (void);

G_DEFINE_TYPE (HyScanGtkPane, hyscan_gtk_pane, GTK_TYPE_GRID)

static void
hyscan_gtk_pane_class_init (HyScanGtkPaneClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = hyscan_gtk_pane_set_property;
  object_class->constructed = hyscan_gtk_pane_constructed;
  object_class->finalize = hyscan_gtk_pane_finalize;

  g_object_class_install_property (object_class,
                                   PROP_BODY,
                                   g_param_spec_object ("body",
                                                        "Body",
                                                        "Body",
                                                        GTK_TYPE_WIDGET,
                                                        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class,
                                   PROP_TITLE,
                                   g_param_spec_string ("title",
                                                        "Title",
                                                        "Title",
                                                        NULL,
                                                        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class,
                                   PROP_BOLD,
                                   g_param_spec_boolean ("bold",
                                                         "bold",
                                                         "Bold",
                                                         FALSE,
                                                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class,
                                   PROP_HEADER_STYLE,
                                   g_param_spec_enum ("header-style",
                                                      "Header style",
                                                      "Header style",
                                                      HYSCAN_TYPE_GTK_PANE_STYLE,
                                                      DEFAULT_HEADER_STYLE,
                                                      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void
hyscan_gtk_pane_init (HyScanGtkPane *pane)
{
}

static void
hyscan_gtk_pane_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  HyScanGtkPane *pane = HYSCAN_GTK_PANE (object);

  switch (prop_id)
    {
    case PROP_BODY:
      pane->body = g_value_get_object (value);
      break;

    case PROP_TITLE:
      pane->title = g_value_dup_string (value);
      break;

    case PROP_BOLD:
      pane->bold = g_value_get_boolean (value);
      break;

    case PROP_HEADER_STYLE:
      pane->header_style = g_value_get_enum (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
hyscan_gtk_pane_constructed (GObject *object)
{
  G_OBJECT_CLASS (hyscan_gtk_pane_parent_class)->constructed (object);
  hyscan_gtk_pane_setup (HYSCAN_GTK_PANE (object));
}

static void
hyscan_gtk_pane_finalize (GObject *object)
{
  g_free (HYSCAN_GTK_PANE (object)->title);
  G_OBJECT_CLASS (hyscan_gtk_pane_parent_class)->finalize (object);
}

static void
hyscan_gtk_pane_setup (HyScanGtkPane *pane)
{
  GtkWidget *header = NULL;

  if (pane->body == NULL || !strlen (pane->title))
    return;

  if (pane->header_style == HYSCAN_GTK_PANE_TEXT)
    header = hyscan_gtk_pane_make_text_header (pane);
  else if (pane->header_style == HYSCAN_GTK_PANE_ARROW)
    header = hyscan_gtk_pane_make_arrow_header (pane);

  if (header == NULL)
    return;

  pane->revealer = gtk_revealer_new ();
  gtk_revealer_set_transition_type (GTK_REVEALER (pane->revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP);
  gtk_revealer_set_reveal_child (GTK_REVEALER (pane->revealer), TRUE);
  gtk_container_add (GTK_CONTAINER (pane->revealer), pane->body);

  gtk_grid_attach (GTK_GRID (pane), header, 0, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (pane), pane->revealer, 0, 1, 1, 1);
  /*gtk_grid_attach (GTK_GRID (pane), gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), 0, 2, 1, 1);*/
}

static GtkWidget *
hyscan_gtk_pane_make_text_header (HyScanGtkPane *pane)
{
  GtkWidget *label;
  GtkWidget *header;
  gchar *markup;

  pane->ebox = gtk_event_box_new ();
  gtk_widget_set_halign (pane->ebox, GTK_ALIGN_END);
  g_signal_connect_swapped (pane->ebox, "button-press-event",
                            G_CALLBACK(hyscan_gtk_pane_expand_text), pane);

  pane->expander = gtk_label_new ("hide");
  gtk_container_add (GTK_CONTAINER (pane->ebox), pane->expander);

  label = gtk_label_new (NULL);
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_hexpand (label, TRUE);
  markup = g_markup_printf_escaped (pane->bold ? "<b>%s</b>" : "%s", pane->title);
  gtk_label_set_markup (GTK_LABEL (label), markup);
  g_free (markup);

  header = gtk_grid_new ();
  gtk_grid_attach (GTK_GRID (header), label, 0, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (header), pane->ebox, 1, 0, 1, 1);
  gtk_widget_set_margin_start (header, 4);
  gtk_widget_set_margin_top (header, 4);
  gtk_widget_set_margin_end (header, 4);
  gtk_widget_set_margin_bottom (header, 4);

  return header;
}

static GtkWidget *
hyscan_gtk_pane_make_arrow_header (HyScanGtkPane *pane)
{
  GtkWidget *label;
  GtkWidget *header;
  gchar *markup;

  pane->ebox = gtk_event_box_new ();
  g_signal_connect_swapped (pane->ebox, "button-press-event",
                            G_CALLBACK(hyscan_gtk_pane_expand_arrow), pane);

  gtk_widget_set_halign (pane->ebox, GTK_ALIGN_START);
  pane->expander = hyscan_gtk_pane_make_down_arrow();
  gtk_container_add (GTK_CONTAINER (pane->ebox), pane->expander);

  label = gtk_label_new (NULL);
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_hexpand (label, TRUE);
  markup = g_markup_printf_escaped (pane->bold ? "<b>%s</b>" : "%s", pane->title);
  gtk_label_set_markup (GTK_LABEL (label), markup);
  g_free (markup);

  header = gtk_grid_new ();
  gtk_grid_attach (GTK_GRID (header), pane->ebox, 0, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (header), label, 1, 0, 1, 1);
  gtk_grid_set_column_spacing (GTK_GRID (header), 4);
  gtk_widget_set_margin_start (header, 4);
  gtk_widget_set_margin_top (header, 4);
  gtk_widget_set_margin_end (header, 4);
  gtk_widget_set_margin_bottom (header, 4);

  return header;
}

static gboolean
hyscan_gtk_pane_expand_text (HyScanGtkPane *pane,
                             GdkEvent      *event,
                             GtkWidget     *ebox)
{
  gboolean revealed = gtk_revealer_get_reveal_child (GTK_REVEALER (pane->revealer));
  gtk_revealer_set_reveal_child (GTK_REVEALER (pane->revealer), !revealed);

  gtk_label_set_text (GTK_LABEL (pane->expander), revealed ? "show" : "hide");
  gtk_widget_set_visible (pane->revealer, !revealed);

  return TRUE;
}

static gboolean
hyscan_gtk_pane_expand_arrow (HyScanGtkPane *pane,
                              GdkEvent      *event,
                              GtkWidget     *ebox)
{
  gboolean revealed = gtk_revealer_get_reveal_child (GTK_REVEALER (pane->revealer));
  gtk_revealer_set_reveal_child (GTK_REVEALER (pane->revealer), !revealed);

  gtk_container_remove (GTK_CONTAINER (pane->ebox),
                        gtk_bin_get_child (GTK_BIN (pane->ebox)));

  pane->expander = revealed ? hyscan_gtk_pane_make_right_arrow ()
                            : hyscan_gtk_pane_make_down_arrow ();
  gtk_container_add (GTK_CONTAINER (pane->ebox), pane->expander);
  gtk_widget_show_all (GTK_WIDGET (pane->ebox));

  return TRUE;
}

/* Возвращает изображение со стрелкой вправо. */
static GtkWidget *
hyscan_gtk_pane_make_right_arrow (void)
{
  return gtk_image_new_from_icon_name ("pan-end-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
}

/* Возвращает изображение со стрелкой вниз. */
static GtkWidget *
hyscan_gtk_pane_make_down_arrow (void)
{
  return gtk_image_new_from_icon_name ("pan-down-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
}

GtkWidget *
hyscan_gtk_pane_new (const gchar          *title,
                     GtkWidget            *body,
                     gboolean              bold,
                     HyScanGtkPaneStyle    style)
{
  return g_object_new (HYSCAN_TYPE_GTK_PANE,
                       "title", title,
                       "body", body,
                       "bold", bold,
                       "header-style", style,
                       NULL);
}

void
hyscan_gtk_pane_set_expanded (HyScanGtkPane *pane,
                              gboolean       expanded)
{
  g_return_if_fail (HYSCAN_IS_GTK_PANE (pane));

  if (expanded == gtk_revealer_get_reveal_child (GTK_REVEALER (pane->revealer)))
    return;

  switch (pane->header_style)
    {
      case HYSCAN_GTK_PANE_ARROW:
        hyscan_gtk_pane_expand_arrow (pane, NULL, pane->ebox);
        break;
      case HYSCAN_GTK_PANE_TEXT:
        hyscan_gtk_pane_expand_text (pane, NULL, pane->ebox);
        break;
    }
}
