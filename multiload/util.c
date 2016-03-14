#include <config.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"


gchar*
format_rate_for_display(guint rate)
{
	gchar *bytes = g_format_size_full(rate, G_FORMAT_SIZE_IEC_UNITS);
	// xgettext: this is a rate format (eg. 2 MiB/s)
	gchar *ret = g_strdup_printf(_("%s/s"), bytes);

	g_free(bytes);
	return ret;
}

gchar*
format_percent(guint value, guint total, guint ndigits)
{
	gchar *ret;
	gchar *format;

	double percent = 100.0 * (double)value / (double)total;
	if (percent < 0)
		percent = 0;
	if (percent > 100)
		percent = 100;

	if (ndigits == 0) {
		ret = g_strdup_printf("%u%%", (guint)percent);
	} else {
		format = g_strdup_printf("%%.%uf%%%%", ndigits);
		ret = g_strdup_printf(format, percent);
		g_free(format);
	}

	return ret;
}


void
gdk_color_to_argb_string(GdkColor* color, guint16 alpha, gchar *out_str)
{
	// note: out_str must be at least 10 characters long
	int rc = snprintf(out_str, 10, "#%02X%02X%02X%02X",
					alpha / 256,
					color->red / 256,
					color->green / 256,
					color->blue / 256);
	g_assert(rc == 9);
}

gboolean
argb_string_to_gdk_color(const gchar *gspec, GdkColor *color, guint16 *alpha)
{
	gchar buf[8];
	if (strlen(gspec) == 7) {
		// may be a standard RGB hex string, fallback to gdk_color_parse
		return gdk_color_parse(gspec, color);
	} else if (G_UNLIKELY (strlen(gspec) != 9) ) {
		return FALSE;
	}

	// alpha part
	buf[0] = gspec[1];
	buf[1] = gspec[2];
	buf[2] = 0;
	errno = 0;
	*alpha = (guint16)strtol(buf, NULL, 16);
	if (errno) {
		// error in strtol, set alpha=max
		*alpha = 0xFFFF;
	} else {
		/* alpha is in the form '0x00jk'. Transform it in the form
		  '0xjkjk', so the conversion of 8 to 16 bits is proportional. */
		*alpha |= (*alpha << 8);
	}

	// color part
	buf[0] = '#';
	strncpy(buf+1, gspec+3, 6);
	buf[7] = 0;
	return gdk_color_parse(buf, color);
}


GtkWidget* gtk_spin_button_new_with_parameters(gint min, gint max, gint step, gint start_value)
{
	guint maxdigits;
	guint mindigits;
	guint maxlen;
	gchar *str;
	GtkWidget *w = gtk_spin_button_new_with_range(min, max, step);

	// calculate max length
	str = g_strdup_printf("%u", max);
	maxdigits = strlen(str);
	g_free(str);
	str = g_strdup_printf("%u", min);
	mindigits = strlen(str);
	g_free(str);

	maxlen = MAX(maxdigits, mindigits);
	gtk_entry_set_max_length (GTK_ENTRY(w), maxlen);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), (gdouble)start_value);

	return w;
}
