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
#include "common/ui.h"

#include "common/about-data.h"
#include "common/multiload.h"
#include "common/multiload-config.h"
#include "common/multiload-colors.h"
#include "common/properties.h"
#include "common/util.h"


// ps = Panel Specific - implement these for every panel
extern gpointer multiload_ps_settings_open_for_read(MultiloadPlugin *ma);
extern gpointer multiload_ps_settings_open_for_save(MultiloadPlugin *ma);
extern gboolean multiload_ps_settings_save(gpointer settings);
extern void multiload_ps_settings_close(gpointer settings);
extern void multiload_ps_settings_get_int(gpointer settings, const gchar *key, int *destination);
extern void multiload_ps_settings_get_boolean(gpointer settings, const gchar *key, gboolean *destination);
extern void multiload_ps_settings_get_string(gpointer settings, const gchar *key, gchar *destination, size_t maxlen);
extern void multiload_ps_settings_set_int(gpointer settings, const gchar *key, int value);
extern void multiload_ps_settings_set_boolean(gpointer settings, const gchar *key, gboolean value);
extern void multiload_ps_settings_set_string(gpointer settings, const gchar *key, const gchar *value);
extern void multiload_ps_preferences_closed_cb(MultiloadPlugin *ma);



void
multiload_ui_read (MultiloadPlugin *ma)
{
	gpointer *settings;
	gchar *key;
	gchar colors_list[10*MAX_COLORS];
	int i;

	multiload_defaults(ma);

	settings = multiload_ps_settings_open_for_read(ma);
	g_debug("[ui] Reading settings from object %p", settings);
	if (G_LIKELY (settings != NULL)) {
		multiload_ps_settings_get_int		(settings, "padding",			&ma->padding);
		multiload_ps_settings_get_int		(settings, "spacing",			&ma->spacing);
		multiload_ps_settings_get_int		(settings, "orientation",		&ma->orientation_policy);
		multiload_ps_settings_get_boolean	(settings, "fill-between",		&ma->fill_between);

		for ( i = 0; i < GRAPH_MAX; i++ ) {
			/* Visibility */
			key = g_strdup_printf("graph-%s-visible", graph_types[i].name);
			multiload_ps_settings_get_boolean (settings, key, &ma->graph_config[i].visible);
			g_free (key);

			/* Border width */
			key = g_strdup_printf("graph-%s-border-width", graph_types[i].name);
			multiload_ps_settings_get_int (settings, key, &ma->graph_config[i].border_width);
			g_free (key);

			/* Interval */
			key = g_strdup_printf("graph-%s-interval", graph_types[i].name);
			multiload_ps_settings_get_int (settings, key, &ma->graph_config[i].interval);
			g_free (key);

			/* Size */
			key = g_strdup_printf("graph-%s-size", graph_types[i].name);
			multiload_ps_settings_get_int (settings, key, &ma->graph_config[i].size);
			g_free (key);

			/* Tooltip style */
			key = g_strdup_printf("graph-%s-tooltip-style", graph_types[i].name);
			multiload_ps_settings_get_int (settings, key, &ma->graph_config[i].tooltip_style);
			g_free (key);

			/* Double click policy */
			key = g_strdup_printf("graph-%s-dblclick-policy", graph_types[i].name);
			multiload_ps_settings_get_int (settings, key, &ma->graph_config[i].dblclick_policy);
			g_free (key);

			/* Double click command line */
			key = g_strdup_printf("graph-%s-dblclick-cmdline", graph_types[i].name);
			multiload_ps_settings_get_string (settings, key, ma->graph_config[i].dblclick_cmdline, sizeof(ma->graph_config[i].dblclick_cmdline));
			g_free (key);

			/* Colors */
			key = g_strdup_printf("graph-%s-colors", graph_types[i].name);
			colors_list[0] = 0;
			multiload_ps_settings_get_string (settings, key, colors_list, sizeof(colors_list)/sizeof(gchar));
			multiload_colors_unstringify(ma, i, colors_list);
			g_free (key);
		}
		g_debug("[ui] Done reading settings. Closing object %p", settings);
		multiload_ps_settings_close(settings);

		multiload_sanitize(ma);
	} else {
		g_warning("multiload_ui_read: settings = NULL");
	}
}

void
multiload_ui_save (MultiloadPlugin *ma)
{
	gpointer *settings;
	char *key;
	gchar colors_list[10*MAX_COLORS];
	int i;

	settings = multiload_ps_settings_open_for_save(ma);
	g_debug("[ui] Writing settings to object %p", settings);
	if (G_LIKELY (settings != NULL)) {
		multiload_ps_settings_set_int		(settings, "padding",			ma->padding);
		multiload_ps_settings_set_int		(settings, "spacing",			ma->spacing);
		multiload_ps_settings_set_int		(settings, "orientation",		ma->orientation_policy);
		multiload_ps_settings_set_boolean	(settings, "fill-between",		ma->fill_between);

		for ( i = 0; i < GRAPH_MAX; i++ ) {
			/* Visibility */
			key = g_strdup_printf("graph-%s-visible", graph_types[i].name);
			multiload_ps_settings_set_boolean (settings, key, ma->graph_config[i].visible);
			g_free (key);

			/* Border width */
			key = g_strdup_printf("graph-%s-border-width", graph_types[i].name);
			multiload_ps_settings_set_int (settings, key, ma->graph_config[i].border_width);
			g_free (key);

			/* Interval */
			key = g_strdup_printf("graph-%s-interval", graph_types[i].name);
			multiload_ps_settings_set_int (settings, key, ma->graph_config[i].interval);
			g_free (key);

			/* Size */
			key = g_strdup_printf("graph-%s-size", graph_types[i].name);
			multiload_ps_settings_set_int (settings, key, ma->graph_config[i].size);
			g_free (key);

			/* Tooltip style */
			key = g_strdup_printf("graph-%s-tooltip-style", graph_types[i].name);
			multiload_ps_settings_set_int (settings, key, ma->graph_config[i].tooltip_style);
			g_free (key);

			/* Double click policy */
			key = g_strdup_printf("graph-%s-dblclick-policy", graph_types[i].name);
			multiload_ps_settings_set_int (settings, key, ma->graph_config[i].dblclick_policy);
			g_free (key);

			/* Double click command line */
			key = g_strdup_printf("graph-%s-dblclick-cmdline", graph_types[i].name);
			multiload_ps_settings_set_string (settings, key, ma->graph_config[i].dblclick_cmdline);
			g_free (key);

			/* Colors */
			key = g_strdup_printf("graph-%s-colors", graph_types[i].name);
			multiload_colors_stringify (ma, i, colors_list);
			multiload_ps_settings_set_string (settings, key, colors_list);
			g_free (key);
		}

		if (!multiload_ps_settings_save(settings)) {
			g_warning("multiload_ui_save: PS save failed");
		}
		g_debug("[ui] Done writing settings. Closing object %p", settings);
		multiload_ps_settings_close(settings);
	} else {
		g_warning("multiload_ui_save: settings = NULL");
	}
}

void
multiload_ui_show_help() {
	g_debug("[ui] Help command triggered");
	xdg_open_url(about_data_website);
}

void
multiload_ui_show_about (GtkWindow* parent)
{
	g_debug("[ui] About command triggered");
	gtk_show_about_dialog(parent,
		"logo-icon-name",		about_data_icon,
		"program-name",			about_data_progname,
		"version",				PACKAGE_VERSION,
		"comments",				about_data_description,
		"website",				about_data_website,
		"copyright",			about_data_copyright,
		"license",				about_data_license,
	#if GTK_API == 3
		"license-type",			GTK_LICENSE_GPL_2_0,
	#endif
		"authors",				about_data_authors,
		"translator-credits",	_("translator-credits"),
		NULL);
}

void
multiload_ui_configure_response_cb (GtkWidget *dialog, gint response, MultiloadPlugin *ma)
{
	if (response == GTK_RESPONSE_HELP) {
		g_debug("[ui] Help command triggered from preferences");
		xdg_open_url(about_data_preferences_website);
	} else {
		ma->pref_dialog = NULL;
		multiload_ui_save (ma);
		multiload_ps_preferences_closed_cb(ma);
		gtk_widget_destroy (dialog);
	}
}

GtkWidget*
multiload_ui_configure_dialog_new(MultiloadPlugin *ma, GtkWindow* parent)
{
	if (G_UNLIKELY(ma->pref_dialog != NULL)) {
		g_debug("[ui] Configure: using existing dialog");
		return ma->pref_dialog;
	}

	g_debug("[ui] Configure: creating new dialog");
	GtkWidget *dialog = gtk_dialog_new_with_buttons(about_data_progname,
					parent,
					GTK_DIALOG_DESTROY_WITH_PARENT,
					_("_Help"), GTK_RESPONSE_HELP,
					_("_Close"), GTK_RESPONSE_OK,
					NULL);

	gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_icon_name (GTK_WINDOW (dialog), about_data_icon);

	// link back the dialog to the plugin
	g_object_set_data (G_OBJECT (dialog), "MultiloadPlugin", ma);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	multiload_init_preferences(dialog, ma);

	g_signal_connect (G_OBJECT (dialog), "response",
					G_CALLBACK(multiload_ui_configure_response_cb), ma);

	ma->pref_dialog = dialog;
	return dialog;
}

void
multiload_ui_start_system_monitor(MultiloadPlugin *ma)
{
	gboolean result;
	gchar *cmdline = get_system_monitor_executable();

	if (cmdline == NULL || cmdline[0] == '\0') {
		g_debug("[ui] NULL or empty cmdline for multiload_ui_start_system_monitor()");
	} else {
		g_debug("[ui] Executing command line: '%s'", cmdline);
		result = g_spawn_command_line_async (cmdline, NULL);

		if (G_UNLIKELY (result == FALSE))
			g_warning (_("Unable to execute the following command line: '%s'"), cmdline);
	}

	g_free(cmdline);
}
