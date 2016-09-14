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

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gi18n-lib.h>

#include <lxpanel/plugin.h>

#include "common/about-data.h"
#include "common/multiload.h"
#include "common/ui.h"


typedef struct {
	LXPanel *panel;
	config_setting_t *settings;
} PanelData;


gpointer
multiload_ps_settings_open_for_read(MultiloadPlugin *ma)
{
	return ((PanelData*)ma->panel_data)->settings;
}
gpointer
multiload_ps_settings_open_for_save(MultiloadPlugin *ma)
{
	return ((PanelData*)ma->panel_data)->settings;
}

gboolean
multiload_ps_settings_save(gpointer settings)
{
	return TRUE;
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
	//force settings update
	LXPanel *panel = ((PanelData*)ma->panel_data)->panel;
	lxpanel_config_save(panel);
}



GtkWidget*
lxpanel_configure_cb(LXPanel *panel, GtkWidget *ebox)
{
	MultiloadPlugin *multiload = lxpanel_plugin_get_data(ebox);
	return multiload_ui_configure_dialog_new(multiload,
		GTK_WINDOW(gtk_widget_get_toplevel (GTK_WIDGET(ebox))));
}

void
lxpanel_reconfigure_cb(LXPanel *panel, GtkWidget *ebox)
{
	MultiloadPlugin *multiload = lxpanel_plugin_get_data(ebox);

	if ( panel_get_orientation(panel) == GTK_ORIENTATION_VERTICAL )
		multiload->panel_orientation = GTK_ORIENTATION_VERTICAL;
	else // lxpanel panel orientation can have values other than vert/horiz
		multiload->panel_orientation = GTK_ORIENTATION_HORIZONTAL;

	multiload_refresh_orientation(multiload);
}

void
lxpanel_destructor(gpointer user_data)
{
	MultiloadPlugin *multiload = lxpanel_plugin_get_data(user_data);
	gtk_widget_destroy (GTK_WIDGET(user_data));
	g_free(multiload->panel_data);
	g_free(multiload);
}

GtkWidget*
lxpanel_constructor(LXPanel *panel, config_setting_t *settings)
{
	MultiloadPlugin *multiload = multiload_new();

	multiload->panel_data = g_slice_new0(PanelData);
	((PanelData*)multiload->panel_data)->panel = panel;
	((PanelData*)multiload->panel_data)->settings = settings;

	lxpanel_plugin_set_data(multiload->container, multiload, lxpanel_destructor);

	multiload_ui_read (multiload);
	lxpanel_reconfigure_cb(panel, GTK_WIDGET(multiload->container));
	multiload_start(multiload);

	return GTK_WIDGET(multiload->container);
}


FM_DEFINE_MODULE(lxpanel_gtk, multiload-ng)

/* Plugin descriptor. */
LXPanelPluginInit fm_module_init_lxpanel_gtk = {
	.name				= about_data_progname,
	.description		= about_data_description_N,

	.new_instance		= lxpanel_constructor,
	.config				= lxpanel_configure_cb,
	.reconfigure		= lxpanel_reconfigure_cb,
	.one_per_system		= FALSE,
	.expand_available	= FALSE
};
