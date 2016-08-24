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
str_replace (const gchar *string, const gchar *needle, const gchar *replacement)
{
	gchar *tok = NULL;
	gchar *newstr = NULL;
	gchar *oldstr = NULL;

	if (needle == NULL || replacement == NULL)
		return g_strdup (string);

	newstr = g_strdup (string);
	while ( (tok = strstr(newstr, needle)) != NULL ) {
		oldstr = newstr;
		newstr = malloc ( strlen(oldstr) - strlen(needle) + strlen(replacement) + 1 );
		if ( newstr == NULL ) {
			g_free (oldstr);
			return NULL;
		}

		memcpy ( newstr, oldstr, tok - oldstr );
		memcpy ( newstr + (tok-oldstr), replacement, strlen(replacement) );
		memcpy ( newstr + (tok-oldstr) + strlen(replacement), tok + strlen(needle), strlen(oldstr) - strlen(needle) - (tok-oldstr) );
		memset ( newstr + strlen(oldstr) - strlen(needle) + strlen(replacement) , 0, 1 );
		g_free (oldstr);
	}
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


static void
close_dialog_cb(GtkWidget *dialog, gint response, gpointer id) {
	gtk_widget_destroy(dialog);
}

void gtk_error_dialog(GtkWindow *parent, const gchar *message)
{
	GtkWidget *dialog = gtk_message_dialog_new(parent,
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", message);
	g_signal_connect (G_OBJECT (dialog), "response", G_CALLBACK(close_dialog_cb), NULL);
	gtk_widget_show(dialog);
}

GtkWidget* gtk_yesno_dialog(GtkWindow *parent, const gchar *message, GCallback cb, gpointer data)
{
	GtkWidget *dialog = gtk_message_dialog_new(parent,
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s", message);
	g_signal_connect (G_OBJECT (dialog), "response", cb, data);
	return dialog;
}

gchar* gtk_open_file_dialog(GtkWindow *parent, const gchar *title)
{
	int response;
	char *filename;
	GtkWidget *dialog = gtk_file_chooser_dialog_new (title, parent,
										GTK_FILE_CHOOSER_ACTION_OPEN,
										_("_Cancel"), GTK_RESPONSE_CANCEL,
										_("_Open"), GTK_RESPONSE_ACCEPT,
										NULL);

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	else
		filename = NULL;
	gtk_widget_destroy (dialog);
	return filename;
}

gchar* gtk_save_file_dialog(GtkWindow *parent, const gchar *title, const gchar *current_name)
{
	int response;
	char *filename;
	GtkWidget *dialog = gtk_file_chooser_dialog_new (title, parent,
										GTK_FILE_CHOOSER_ACTION_SAVE,
										_("_Cancel"), GTK_RESPONSE_CANCEL,
										_("_Save"), GTK_RESPONSE_ACCEPT,
										NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
	gtk_file_chooser_set_create_folders (GTK_FILE_CHOOSER (dialog), TRUE);
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), current_name);

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	else
		filename = NULL;
	gtk_widget_destroy (dialog);
	return filename;
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
xdg_open_url(const gchar* url) {
	gchar *cmdline;
	gboolean result;

	g_debug("[util] Trying to open URL '%s' with xdg-open...", url);

	cmdline = g_strdup_printf("xdg-open %s", url);
	result = g_spawn_command_line_async (cmdline, NULL);
	g_free(cmdline);

	if (G_UNLIKELY (result == FALSE))
		g_warning (_("Unable to open the following url: %s"), url);
}
