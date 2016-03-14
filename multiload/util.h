#ifndef H_MULTILOAD_UTIL_
#define H_MULTILOAD_UTIL_

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>


gchar* format_rate_for_display (guint rate);
gchar* format_percent(guint value, guint total, guint ndigits);

void gdk_color_to_argb_string(GdkColor* color, guint16 alpha, gchar *out_str);
gboolean argb_string_to_gdk_color(const gchar *gspec, GdkColor *color, guint16 *alpha);

#endif /* H_MULTILOAD_UTIL_ */
