/**
 * \file hyscan-gtk-project-creator.h
 *
 * \brief Заголовочный файл виджета создания проекта.
 * \author Vladimir Maximov (vmakxs@gmail.com)
 * \date 2018
 * \license Проприетарная лицензия ООО "Экран"
 *
 * \defgroup HyScanGtkProjectCreator HyScanGtkProjectCreator - виджет создания проекта
 *
 */

#ifndef __HYSCAN_GTK_PROJECT_CREATOR_H__
#define __HYSCAN_GTK_PROJECT_CREATOR_H__

#include <gtk/gtk.h>
#include <hyscan-api.h>

G_BEGIN_DECLS

#define HYSCAN_TYPE_GTK_PROJECT_CREATOR            \
        (hyscan_gtk_project_creator_get_type ())
#define HYSCAN_GTK_PROJECT_CREATOR(obj)            \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), HYSCAN_TYPE_GTK_PROJECT_CREATOR, HyScanGtkProjectCreator))
#define HYSCAN_IS_GTK_PROJECT_CREATOR(obj)         \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HYSCAN_TYPE_GTK_PROJECT_CREATOR))
#define HYSCAN_GTK_PROJECT_CREATOR_CLASS(klass)    \
        (G_TYPE_CHECK_CLASS_CAST ((klass), HYSCAN_TYPE_GTK_PROJECT_CREATOR, HyScanGtkProjectCreatorClass))
#define HYSCAN_IS_GTK_PROJECT_CREATOR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), HYSCAN_TYPE_GTK_PROJECT_CREATOR))
#define HYSCAN_GTK_PROJECT_CREATOR_GET_CLASS(obj)  \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), HYSCAN_TYPE_GTK_PROJECT_CREATOR, HyScanGtkProjectCreatorClass))

typedef struct _HyScanGtkProjectCreator HyScanGtkProjectCreator;
typedef struct _HyScanGtkProjectCreatorPrivate HyScanGtkProjectCreatorPrivate;
typedef struct _HyScanGtkProjectCreatorClass HyScanGtkProjectCreatorClass;

struct _HyScanGtkProjectCreator
{
  GtkPopover                       parent_instance;
  HyScanGtkProjectCreatorPrivate  *priv;
};

struct _HyScanGtkProjectCreatorClass
{
  GtkPopoverClass  parent_class;
};

HYSCAN_API
GType         hyscan_gtk_project_creator_get_type     (void);

HYSCAN_API
GtkWidget*    hyscan_gtk_project_creator_new          (GtkWidget                *relative_to);

HYSCAN_API
const gchar*  hyscan_gtk_project_creator_get_project  (HyScanGtkProjectCreator  *project_creator);

HYSCAN_API
void          hyscan_gtk_project_creator_set_project  (HyScanGtkProjectCreator  *project_creator,
                                                       const gchar              *project);

G_END_DECLS

#endif /* __HYSCAN_GTK_PROJECT_CREATOR_H__ */
