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

	size_t s_string = strlen(string);
	size_t s_needle = strlen(needle);
	size_t s_replacement = strlen(replacement);

	if (needle == NULL || replacement == NULL)
		return g_strdup(string);

	// count how many occurences
	for(p = strstr(string, needle); p != NULL; p = strstr(p+s_needle, needle))
		c++;

	if (c == 0)
		return g_strdup(string);

	// final size
	c = s_string + (s_replacement-s_needle)*c;

	// new string with new size
	newstr = g_malloc0( c );

	oldstr = string;
	for(p = strstr(string, needle); p != NULL; p = strstr(p+s_needle, needle)) {
		// move ahead and copy some text from original string, from a certain position
		strncpy(newstr + strlen(newstr) , oldstr , p - oldstr);

		// move ahead and copy the replacement text
		strcpy(newstr + strlen(newstr) , replacement);

		// the new start position after this search match
		oldstr = p + s_needle;
	}

	// copy the part after the last search match
	strcpy(newstr + strlen(newstr) , oldstr);

	return newstr;
}


gboolean
file_check_contents(FILE *f, const gchar *string)
{
	size_t n;
	size_t s;
	gchar *buf;

	n = strlen(string);
	buf = (gchar*)malloc(n);

	s = fread(buf, 1, n, f);

	if (s != n)
		return FALSE;

	if (strncmp(buf, string, n) != 0)
		return FALSE;

	return TRUE;
}

gint64
read_int_from_file(const gchar *path)
{
	FILE *f;
	size_t s;
	//30 chars should contain every possible number
	gchar buf[30];


	f = fopen(path, "r");
	if (!f)
		return 0;

	s = fread(buf, 1, sizeof(buf), f);
	fclose(f);

	if (s < 1)
		return 0;

	return atol(buf);
}


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
	gchar* format = g_new0(gchar, 24);

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
		n = sprintf(pos, "%dd ", d);
		pos += n;
	}

	if (h) {
		n = sprintf(pos, "%dh ", h);
		pos += n;
	}

	if (m) {
		n = sprintf(pos, "%dm ", m);
		pos += n;
	}

	if (s || pos==format) {
		n = sprintf(pos, "%ds ", s);
		pos += n;
	}

	// remove last space
	*(pos-1) = 0;

	return format;
}


void gtk_error_dialog(GtkWindow *parent, const gchar *message)
{
	GtkWidget *dialog = gtk_message_dialog_new(parent,
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", message);
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
		"gnome-system-monitor",
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

// return a file pointer that does not need to be closed
FILE*
cached_fopen_r(gchar* path, gboolean reopen)
{
	static GHashTable *table = NULL;
	FILE *f;

	if (table == NULL)
		table = g_hash_table_new (g_str_hash, g_str_equal);

	f = (FILE*)g_hash_table_lookup(table, path);
	if (f != NULL && reopen) {
		fclose(f);
		f = NULL;
	}
	if (f == NULL) {
		f = fopen(path, "r");
		g_assert(f != NULL);
		g_hash_table_insert(table, path, f);
	}

	rewind(f);
	return f;
}
