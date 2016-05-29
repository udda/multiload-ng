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

#include "common/about-data.h"
#include "common/multiload.h"
#include "common/ui.h"

/*
gpointer
multiload_ps_settings_open_for_read(MultiloadPlugin *ma)
{
	return ma->panel_data;
}
gpointer
multiload_ps_settings_open_for_save(MultiloadPlugin *ma)
{
	return ma->panel_data;
}

void
multiload_ps_settings_save(gpointer settings)
{
}

void
multiload_ps_settings_close(gpointer settings)
{
}

void
multiload_ps_settings_get_int(gpointer settings, const gchar *key, int *destination)
{
	config_setting_lookup_int((config_setting_t*)settings, key, destination);
}
void
multiload_ps_settings_get_boolean(gpointer settings, const gchar *key, gboolean *destination)
{
	config_setting_lookup_int((config_setting_t*)settings, key, destination);
	*destination = (*destination)? TRUE:FALSE;
}
void
multiload_ps_settings_get_string(gpointer settings, const gchar *key, gchar *destination, size_t maxlen)
{
	const gchar* temp = NULL;
	config_setting_lookup_string((config_setting_t*)settings, key, &temp);
	if (G_LIKELY(temp != NULL))
		strncpy(destination, temp, maxlen);
}

void
multiload_ps_settings_set_int(gpointer settings, const gchar *key, int value)
{
	config_group_set_int((config_setting_t*)settings, key, value);
}
void
multiload_ps_settings_set_boolean(gpointer settings, const gchar *key, gboolean value)
{
	config_group_set_int((config_setting_t*)settings, key, value?TRUE:FALSE);
}
void
multiload_ps_settings_set_string(gpointer settings, const gchar *key, const gchar *value)
{
	config_group_set_string((config_setting_t*)settings, key, value);
}

void
multiload_ps_preferences_closed_cb(MultiloadPlugin *ma)
{
}



GtkWidget*
standalone_configure_cb(LXPanel *panel, GtkWidget *ebox)
{
	MultiloadPlugin *multiload = lxpanel_plugin_get_data(ebox);
	return multiload_ui_configure_dialog_new(multiload,
		GTK_WINDOW(gtk_widget_get_toplevel (GTK_WIDGET(ebox))));
}

void
standalone_reconfigure_cb(LXPanel *panel, GtkWidget *ebox)
{
	MultiloadPlugin *multiload = lxpanel_plugin_get_data(ebox);

	if ( panel_get_orientation(panel) == GTK_ORIENTATION_VERTICAL )
		multiload->panel_orientation = GTK_ORIENTATION_VERTICAL;
	else // lxpanel panel orientation can have values other than vert/horiz
		multiload->panel_orientation = GTK_ORIENTATION_HORIZONTAL;

	multiload_refresh(multiload);
}

void
standalone_destructor(gpointer user_data)
{
	MultiloadPlugin *multiload = lxpanel_plugin_get_data(user_data);
	gtk_widget_destroy (GTK_WIDGET(user_data));
	g_free(multiload);
}

*/


int main(int argc, char *argv[]) {
	MultiloadPlugin *multiload = g_slice_new0(MultiloadPlugin);

	gtk_init (&argc, &argv);
	multiload_init ();

	multiload->container = GTK_CONTAINER(gtk_event_box_new ());
	gtk_widget_show (GTK_WIDGET(multiload->container));

//	multiload_ui_read (multiload);

	GtkWindow *w = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_title (w, about_data_progname);
	gtk_window_set_decorated (w, TRUE);
	gtk_window_set_keep_above (w, TRUE);
	gtk_window_set_icon_name (w, "utilities-system-monitor");

	gtk_container_add(GTK_CONTAINER(w), GTK_WIDGET(multiload->container));

	gtk_window_present (w);

	gtk_main();

	return 0;
}
