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
	GtkWidget         *label;
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

	/* Initialize multiload */
	multiload_init ();
	/* read the user settings */
	multiload_read (multiload->settings, &multiload->ma);
	multiload_sanitize(&multiload->ma);

	/* create a container widget */
	multiload->ma.container = GTK_CONTAINER(gtk_event_box_new ());
	gtk_container_add (GTK_CONTAINER(applet), GTK_WIDGET(multiload->ma.container));
	gtk_widget_show (GTK_WIDGET(applet));
	gtk_widget_show (GTK_WIDGET(multiload->ma.container));

	multiload_refresh(&(multiload->ma));

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
