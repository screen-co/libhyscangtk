/**
 * \file hyscan-colormap-spec.h
 *
 * \brief Заголовочный файл для работы со спецификациями цветовых схем.
 * \author Vladimir Maximov (vmakxs@gmail.com)
 * \date 2018
 * \license Проприетарная лицензия ООО "Экран"
 *
 */

#ifndef __HYSCAN_COLORMAP_SPEC_H__
#define __HYSCAN_COLORMAP_SPEC_H__

#include <glib.h>
#include <hyscan-api.h>

G_BEGIN_DECLS

typedef struct
{
  gchar *colormap_id;
  gchar *colormap_name;
} HyScanColormapSpec;

HYSCAN_API
HyScanColormapSpec*   hyscan_colormap_spec_new                 (const gchar          *colormap_id,
                                                                const gchar          *colormap_name);

HYSCAN_API
void                  hyscan_colormap_spec_free                (HyScanColormapSpec   *spec);

HYSCAN_API
const gchar*          hyscan_colormap_spec_get_colormap_id     (HyScanColormapSpec   *spec);

HYSCAN_API
const gchar*          hyscan_colormap_spec_get_colormap_name   (HyScanColormapSpec   *spec);

G_END_DECLS

#endif /* __HYSCAN_COLORMAP_SPEC_H__ */