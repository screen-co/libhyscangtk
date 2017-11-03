#include "hyscan-colormap-spec.h"
#include <string.h>

HyScanColormapSpec * 
hyscan_colormap_spec_new (const gchar *colormap_id,
                          const gchar *colormap_name)
{
  if (strlen (colormap_id) && strlen (colormap_name))
    {
      HyScanColormapSpec *spec = g_new0 (HyScanColormapSpec, 1);
      spec->colormap_id = g_strdup (colormap_id);
      spec->colormap_name = g_strdup (colormap_name);
    }
  return NULL;
}

void
hyscan_colormap_spec_free (HyScanColormapSpec *spec)
{
  if (spec != NULL)
    {
      g_free (spec->colormap_id);
      g_free (spec->colormap_name);
      g_free (spec);
    }
}

const gchar *
hyscan_colormap_spec_get_colormap_id (HyScanColormapSpec *spec)
{
  return spec != NULL ? spec->colormap_id : NULL;
}

const gchar *
hyscan_colormap_spec_get_colormap_name (HyScanColormapSpec *spec)
{
  return spec != NULL ? spec->colormap_name : NULL;
}
