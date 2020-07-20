#ifndef __HYSCAN_GTK_PARAM_BOX_H__
#define __HYSCAN_GTK_PARAM_BOX_H__

#include <hyscan-gtk-param-list.h>

G_BEGIN_DECLS

#define HYSCAN_TYPE_GTK_PARAM_BOX             (hyscan_gtk_param_box_get_type ())
#define HYSCAN_GTK_PARAM_BOX(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_PARAM_BOX, HyScanGtkParamBox))
#define HYSCAN_IS_GTK_PARAM_BOX(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_PARAM_BOX))
#define HYSCAN_GTK_PARAM_BOX_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_PARAM_BOX, HyScanGtkParamBoxClass))
#define HYSCAN_IS_GTK_PARAM_BOX_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_PARAM_BOX))
#define HYSCAN_GTK_PARAM_BOX_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_PARAM_BOX, HyScanGtkParamBoxClass))

typedef struct _HyScanGtkParamBox HyScanGtkParamBox;
typedef struct _HyScanGtkParamBoxPrivate HyScanGtkParamBoxPrivate;
typedef struct _HyScanGtkParamBoxClass HyScanGtkParamBoxClass;

struct _HyScanGtkParamBox
{
  HyScanGtkParamList parent_instance;

  HyScanGtkParamBoxPrivate *priv;
};

struct _HyScanGtkParamBoxClass
{
  HyScanGtkParamListClass parent_class;
};

HYSCAN_API
GType                  hyscan_gtk_param_box_get_type            (void);

HYSCAN_API
GtkWidget *            hyscan_gtk_param_box_new_default         (void);

HYSCAN_API
GtkWidget *            hyscan_gtk_param_box_new_full            (HyScanParam    *param,
                                                                 const gchar    *root,
                                                                 gboolean        show_hidden);

G_END_DECLS

#endif /* __HYSCAN_GTK_PARAM_BOX_H__ */
