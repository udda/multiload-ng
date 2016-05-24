/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of multiload-nandhp.
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

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gi18n-lib.h>

#include <mate-panel-applet.h>
#include <mate-panel-applet-gsettings.h>

#include "common/about-data.h"
#include "common/multiload.h"
#include "common/multiload-colors.h"
#include "common/multiload-config.h"

typedef struct
{
	MatePanelApplet   *applet;
	MultiloadPlugin   ma;
	GSettings         *settings;
} MultiloadApplet;


void
multiload_read(GSettings *settings, MultiloadPlugin *ma)
{
	guint i;
	gchar *key;
	const char *tmp_str;

	multiload_defaults(ma);

	g_assert_nonnull(settings);
	g_assert_nonnull(ma);
	ma->speed = g_settings_get_int(settings, "speed");
	ma->size = g_settings_get_int(settings, "size");
	ma->padding = g_settings_get_int(settings, "padding");
	ma->spacing = g_settings_get_int(settings, "spacing");
	ma->orientation_policy = g_settings_get_int(settings, "orientation");
	ma->fill_between = g_settings_get_boolean(settings, "fill-between");
	ma->tooltip_details = g_settings_get_boolean(settings, "tooltip-details");
	ma->dblclick_policy = g_settings_get_int(settings, "dblclick-policy");
	if ((tmp_str = g_settings_get_string(settings, "dblclick-cmdline")))
		strncpy(ma->dblclick_cmdline, tmp_str, sizeof(ma->dblclick_cmdline)/sizeof(gchar));

	for ( i=0; i<GRAPH_MAX; i++ ) {
		key = g_strdup_printf("%s-visible", graph_types[i].name);
		ma->graph_config[i].visible = g_settings_get_boolean(settings, key);
		g_free (key);

		key = g_strdup_printf("%s-border-width", graph_types[i].name);
		ma->graph_config[i].border_width = g_settings_get_int(settings, key);
		g_free (key);

		key = g_strdup_printf("%s-colors", graph_types[i].name);
		if ((tmp_str = g_settings_get_string(settings, key)))
			multiload_colors_unstringify(ma, i, tmp_str);
		g_free (key);
	}

	multiload_sanitize(ma);
}


void
multiload_save(MultiloadApplet *multiload)
{
	MultiloadPlugin *ma = &multiload->ma;
	GSettings *s = multiload->settings;

	guint i;
	char *key;
	char list[10*MAX_COLORS];

	g_assert_nonnull(s);

	g_settings_set_int		(s, "speed",			ma->speed);
	g_settings_set_int		(s, "size",				ma->size);
	g_settings_set_int		(s, "padding",			ma->padding);
	g_settings_set_int		(s, "spacing",			ma->spacing);
	g_settings_set_int		(s, "orientation",		ma->orientation_policy);
	g_settings_set_boolean	(s, "fill-between",		ma->fill_between);
	g_settings_set_boolean	(s, "tooltip-details",	ma->tooltip_details);
	g_settings_set_int		(s, "dblclick-policy",	ma->dblclick_policy);
	g_settings_set_string	(s, "dblclick-cmdline",	ma->dblclick_cmdline);

	for ( i = 0; i < GRAPH_MAX; i++ ) {
		key = g_strdup_printf("%s_visible",			graph_types[i].name);
		g_settings_set_boolean(s, key, ma->graph_config[i].visible);
		g_free (key);

		key = g_strdup_printf("%s_border-width",	graph_types[i].name);
		g_settings_set_int(s, key, ma->graph_config[i].border_width);
		g_free (key);

		multiload_colors_stringify (ma, i, list);
		key = g_strdup_printf("%s_colors",			graph_types[i].name);
		g_settings_set_string(s, key, list);
		g_free (key);
	}
}


static const GtkActionEntry multiload_menu_actions [] = {
	{ "MultiLoadProperties", GTK_STOCK_PROPERTIES, N_("_Preferences"),
	  NULL, NULL,
	  NULL },
//	  G_CALLBACK (multiload_properties_cb) },
	{ "MultiLoadRunProcman", GTK_STOCK_EXECUTE, N_("_Open System Monitor"),
	  NULL, NULL,
	  NULL },
//	  G_CALLBACK (start_procman_cb) },
	{ "MultiLoadHelp", GTK_STOCK_HELP, N_("_Help"),
	  NULL, NULL,
	  NULL },
//	  G_CALLBACK (help_cb) },
	{ "MultiLoadAbout", GTK_STOCK_ABOUT, N_("_About"),
	  NULL, NULL,
	  NULL },
//	  G_CALLBACK (about_cb) }
};

static gboolean
multiload_fill (MatePanelApplet* applet)
{
	MultiloadApplet *multiload;

	/* set mate-panel applet options */
	mate_panel_applet_set_flags (applet, MATE_PANEL_APPLET_EXPAND_MINOR | MATE_PANEL_APPLET_HAS_HANDLE);
	mate_panel_applet_set_background_widget(applet, GTK_WIDGET(applet));

	/* create the MultiloadApplet struct */
	multiload = g_malloc0(sizeof(MultiloadApplet));
	multiload->applet = applet;
	multiload->settings = mate_panel_applet_settings_new (applet, "org.mate.panel.applet.multiload-nandhp");

	GtkActionGroup *action_group = gtk_action_group_new ("Multiload Applet Actions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, multiload_menu_actions, G_N_ELEMENTS (multiload_menu_actions), multiload);

	gchar *ui_path = g_build_filename (MULTILOAD_MENU_UI_DIR, "multiload-applet-menu.xml", NULL);
	mate_panel_applet_setup_menu_from_file (applet, ui_path, action_group);
	g_free (ui_path);

	/* Initialize multiload */
	multiload_init ();
	/* read the user settings */
	multiload_read (multiload->settings, &multiload->ma);

	/* create a container widget */
	multiload->ma.container = GTK_CONTAINER(gtk_event_box_new ());
	gtk_container_add (GTK_CONTAINER(applet), GTK_WIDGET(multiload->ma.container));
	gtk_widget_show (GTK_WIDGET(applet));
	gtk_widget_show (GTK_WIDGET(multiload->ma.container));

//TODO do size/orientation changes require g_signal_connect?

	return TRUE;
}

/* This function, called by mate-panel, will create the applet */
static gboolean
multiload_constructor (MatePanelApplet* applet, const char* iid, gpointer data)
{
	if (!g_strcmp0(iid, "MultiloadNandhpApplet"))
		return multiload_fill (applet);
	else
		return FALSE;
}

MATE_PANEL_APPLET_OUT_PROCESS_FACTORY("MultiloadNandhpFactory",
									  PANEL_TYPE_APPLET,
									  about_data_description,
									  multiload_constructor,
									  NULL)
