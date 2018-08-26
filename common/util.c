/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of multiload-ng.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include <config.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gtk-compat.h"
#include "util.h"


guint64
calculate_speed(guint64 delta, guint period_ms)
{
	return ( delta * 1000 ) / period_ms;
}

gchar *
str_replace(const char *string , const char *needle , const char *replacement)
{
	char  *p = NULL;
	const char *oldstr = NULL;
	char *newstr = NULL;
	size_t c = 0;
	size_t final;

	if (string == NULL)
		return NULL;

	if (needle == NULL || replacement == NULL || string[0] == '\0')
		return g_strdup(string);

	size_t s_string = strlen(string);
	size_t s_needle = strlen(needle);
	size_t s_replacement = strlen(replacement);

	// count how many occurences
	for(p = strstr(string, needle); p != NULL; p = strstr(p+s_needle, needle))
		c++;

	if (c == 0)
		return g_strdup(string);

	// final size (add 1 for the ending NULL byte)
	c = s_string + (s_replacement-s_needle)*c + 1;

	// new string with new size
	newstr = g_malloc0( c );

	oldstr = string;
	for(p = strstr(string, needle); p != NULL; p = strstr(p+s_needle, needle)) {
		strncat(newstr, oldstr , p-oldstr);
		g_strlcat(newstr, replacement, c);

		oldstr = p + s_needle;
	}

	// copy the part after the last search match
	final = g_strlcat(newstr, oldstr, c);

	if (final+1 != c)
		g_warning("[util] Failed prediction of replaced string length (allocated %zu, used %zu)", c, final+1);

	return newstr;
}



gchar*
format_size_for_display(guint64 size, gboolean iec_units)
{
	GFormatSizeFlags flags = iec_units ? G_FORMAT_SIZE_IEC_UNITS : G_FORMAT_SIZE_DEFAULT;
	return g_format_size_full(size, flags);
}

gchar*
format_size_for_display_short(guint64 rate, gboolean iec_units)
{
	// Transform e.g. "2.1 MB" into shorter "2.1M"
	gchar *bytes = format_size_for_display(rate, iec_units);
	gchar *ret = g_strdup(bytes);
	guint i;
	guint j=0;
	for (i=0; bytes[i] != '\0'; i++) {
		if (bytes[i] == ' ') {
			if (bytes[i+1] != '\0') { // should always be the case
				ret[j++] = bytes[i+1];
			}
			ret[j++] = '\0';
			break;
		} else if (i >= 3 && bytes[i] == '.' && isdigit(bytes[i+1])) {
			// Strip decimals from "123.4" (three or more leading digits)
			// Not worrying about rounding up for now, as this requires parsing
			// the number to correctly handle all cases (e.g. "999.5").
			i++;
		} else {
			ret[j++] = bytes[i];
		}
	}

	g_free(bytes);
	return ret;
}

gchar*
format_rate_for_display(guint64 rate, gboolean iec_units)
{
	gchar *bytes = format_size_for_display(rate, iec_units);
	// xgettext: this is a rate format (eg. 2 MiB/s)
	gchar *ret = g_strdup_printf(_("%s/s"), bytes);

	g_free(bytes);
	return ret;
}

gchar*
format_percent(guint64 value, guint64 total, guint ndigits)
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

gchar*
format_time_duration(gdouble seconds) {
	gint d, h, m, s;
	gchar *format = g_new0(gchar, 24);
	gchar *unit = NULL;

	int n;
	gchar *pos = format;

	guint64 t = (guint64)seconds;

	s = t%60;

	t = (t-s)/60;
	m = t%60;

	t = (t-m)/60;
	h = t%24;

	d = (t-h)/24;

	if (d) {
		// xgettext: single-letter form for "days" - e.g. 3 days -> 3d
		unit=_("d");
		n = sprintf(pos, "%d%s ", d, unit);
		pos += n;
	}

	if (h) {
		// xgettext: single-letter form for "hours" - e.g. 3 hours -> 3h
		unit=_("h");
		n = sprintf(pos, "%d%s ", h, unit);
		pos += n;
	}

	if (m) {
		// xgettext: single-letter form for "minutes" - e.g. 3 minutes -> 3m
		unit=_("m");
		n = sprintf(pos, "%d%s ", m, unit);
		pos += n;
	}

	if (s || pos==format) {
		// xgettext: single-letter form for "seconds" - e.g. 3 seconds -> 3s
		unit=_("s");
		n = sprintf(pos, "%d%s ", s, unit);
		pos += n;
	}

	// remove last space
	*(pos-1) = 0;

	return format;
}


void show_modal_info_dialog(GtkWindow *parent, GtkMessageType type, const gchar *message)
{
	GtkWidget *dialog = gtk_message_dialog_new(parent,
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						type, GTK_BUTTONS_CLOSE, "%s", message);
	g_signal_connect (G_OBJECT (dialog), "response", (GCallback)gtk_widget_destroy, NULL);
	gtk_widget_show(dialog);
}


gchar* get_system_monitor_executable()
{
	static const gchar* sysmon[] = {
		"ksysguard",
		"gnome-system-monitor",
		"xfce4-taskmanager",
		"lxtask",
		"mate-system-monitor",
		"procexp",
		NULL
	};

	guint i=0;
	gchar* path;
	for (;;) {
		if (sysmon[i] == NULL)
			return NULL;

		g_debug("[util] Looking for system monitor: '%s'", sysmon[i]);

		path = g_find_program_in_path (sysmon[i]);
		if (path != NULL) {
			g_debug("[util] Found system monitor '%s' in '%s'", sysmon[i], path);
			return path;
		}

		i++;
	}
}

void
xdg_open_url(const gchar* url)
{
	gchar *cmdline;
	gboolean result;

	g_debug("[util] Trying to open URL '%s' with xdg-open...", url);

	cmdline = g_strdup_printf("xdg-open %s", url);
	result = g_spawn_command_line_async (cmdline, NULL);
	g_free(cmdline);

	if (G_UNLIKELY (result == FALSE))
		g_warning (_("Unable to open the following url: '%s'"), url);
}

static void
gdk_pixbuf_image_data_free (guchar *pixels, gpointer data)
{
	g_free(pixels);
}

GdkPixbuf*
cairo_surface_to_gdk_pixbuf(cairo_surface_t *surface, guint width, guint height)
{
	guint i, j;
	guchar *img_data, *img_data_converted;

	img_data = cairo_image_surface_get_data (surface);
	img_data_converted = g_new(guchar, width * height * 3);
	for (i=0,j=0; i<width*height; i++) {
		img_data_converted[j++] = img_data[i*4+2];
		img_data_converted[j++] = img_data[i*4+1];
		img_data_converted[j++] = img_data[i*4];
	}

	// create pixbuf from converted data
	return gdk_pixbuf_new_from_data(img_data_converted, GDK_COLORSPACE_RGB, FALSE, 8, width, height, width*3, gdk_pixbuf_image_data_free, NULL);
}

gchar*
int_array_to_string(const int *data, const int length)
{
	int i;
	int ret_len = 12*length; // enough to fit every integer number (including sign) and separators
	gchar tmp[12];

	if (data == NULL)
		return NULL;

	gchar *ret = g_new0(gchar, ret_len);

	for (i=0; i<length; i++) {
		if (i < length - 1)
			snprintf(tmp, sizeof(tmp), "%d,", data[i]);
		else
			snprintf(tmp, sizeof(tmp), "%d", data[i]);

		g_strlcat(ret, tmp, ret_len);
	}

	return ret;
}

void
string_to_int_array(const gchar *array_str, int *data, const int length)
{
	int i;

	if (array_str == NULL || array_str[0] == '\0')
		return;

	char **tokens = g_strsplit(array_str, ",", length);

	for (i=0; tokens[i]!=NULL; i++)
		data[i] = (int)g_ascii_strtoll(tokens[i], NULL, 10);

	g_strfreev(tokens);
}
