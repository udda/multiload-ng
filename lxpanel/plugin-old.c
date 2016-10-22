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


#ifndef LXPANEL_NEW_API

#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gi18n-lib.h>

#include <lxpanel/plugin.h>

#include "common/about-data.h"
#include "common/multiload.h"
#include "common/ui.h"
#include "common/util.h"

typedef struct {
	Plugin *plugin;
	char **fp_read;
	FILE *fp_save;
} PanelData;


static gchar*
key_name_convert(const gchar *key)
{
	// LxPanel old API accepts only alphanumeric characters for key names
	return str_replace(key, "-", "00");
}

gpointer
multiload_ps_settings_open_for_read(MultiloadPlugin *ma)
{
	return ((PanelData*)ma->panel_data)->fp_read;
}
gpointer
multiload_ps_settings_open_for_save(MultiloadPlugin *ma)
{
	return ((PanelData*)ma->panel_data)->fp_save;
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
multiload_ps_settings_get_string(gpointer settings, const gchar *key, gchar *destination, size_t maxlen)
{
	line s;
	s.len = 1024;
	gchar *key_converted = key_name_convert(key);

	// make a new buffer everytime (expensive, but necessary to read settings out of order)
	char *fp = g_strdup(*((char**)settings));
	char *cur = fp;

	while ( lxpanel_get_line(&cur, &s) != LINE_BLOCK_END ) {
		if ( s.type == LINE_VAR ) {
			if ( g_ascii_strcasecmp(s.t[0], key_converted) == 0 ) {
				strncpy(destination, s.t[1], maxlen);
				break;
			}
		} else {
			break;
		}
	}

	g_free(fp);
	g_free(key_converted);
}
void
multiload_ps_settings_get_int(gpointer settings, const gchar *key, int *destination)
{
	char buf[1024];
	multiload_ps_settings_get_string(settings, key, buf, 1024);
	*destination = atoi(buf);
}
void
multiload_ps_settings_get_boolean(gpointer settings, const gchar *key, gboolean *destination)
{
	char buf[1024];
	multiload_ps_settings_get_string(settings, key, buf, 1024);
	*destination = (buf[0]=='1' ? TRUE:FALSE);
}

void
multiload_ps_settings_set_int(gpointer settings, const gchar *key, int value)
{
	gchar *key_converted = key_name_convert(key);
	lxpanel_put_int((FILE*)settings, key_converted, value);
	g_free(key_converted);
}
void
multiload_ps_settings_set_boolean(gpointer settings, const gchar *key, gboolean value)
{
	gchar *key_converted = key_name_convert(key);
	lxpanel_put_bool((FILE*)settings, key_converted, value);
	g_free(key_converted);
}
void
multiload_ps_settings_set_string(gpointer settings, const gchar *key, const gchar *value)
{
	gchar *key_converted = key_name_convert(key);
	lxpanel_put_str((FILE*)settings, key_converted, value);
	g_free(key_converted);
}

void
multiload_ps_preferences_closed_cb(MultiloadPlugin *ma)
{
	PanelData *pd = (PanelData*)ma->panel_data;

	//reset
	pd->fp_save = NULL;
}


static int
lxpanel_constructor(Plugin *p, char **fp)
{
	MultiloadPlugin *multiload = multiload_new();

	multiload->panel_data = g_slice_new0(PanelData);
	((PanelData*)multiload->panel_data)->plugin = p;
	((PanelData*)multiload->panel_data)->fp_read = fp;

	p->priv = multiload;
	p->pwid = GTK_WIDGET(multiload->container);

	multiload_ui_read (multiload);
	lxpanel_configuration_changed(p);
	multiload_start(multiload);

	return 1;
}

static void
lxpanel_destructor(Plugin * p) {
	MultiloadPlugin *multiload = (MultiloadPlugin*)p->priv;
	gtk_widget_destroy (GTK_WIDGET(multiload->container));
	g_free(multiload);
}

static void lxpanel_configure(Plugin * p, GtkWindow * parent)
{
	MultiloadPlugin *multiload = (MultiloadPlugin*)p->priv;
	((PanelData*)multiload->panel_data)->fp_save = NULL;
	GtkWidget *dialog = multiload_ui_configure_dialog_new(multiload, parent);
	gtk_window_present(GTK_WINDOW(dialog));
}

static void lxpanel_save(Plugin * p, FILE * fp)
{
	MultiloadPlugin *multiload = (MultiloadPlugin*)p->priv;
	((PanelData*)multiload->panel_data)->fp_save = fp;
	multiload_ui_save(multiload);
}

static void lxpanel_configuration_changed(Plugin *p)
{
	MultiloadPlugin *multiload = p->priv;
	Panel *panel = p->panel;

	if ( panel->orientation == ORIENT_HORIZ )
		multiload->panel_orientation = GTK_ORIENTATION_HORIZONTAL;
	else
		multiload->panel_orientation = GTK_ORIENTATION_VERTICAL;

	multiload_refresh_orientation(multiload);
}



/* Plugin descriptor. */
PluginClass multiload_ng_plugin_class = {
   PLUGINCLASS_VERSIONING,

   type : "multiload_ng",
   name : about_data_progname,
   version: PACKAGE_VERSION,
   description : about_data_description_N,
   one_per_system : FALSE,
   expand_available : FALSE,

   constructor : lxpanel_constructor,
   destructor  : lxpanel_destructor,
   config : lxpanel_configure,
   save : lxpanel_save,
   panel_configuration_changed : lxpanel_configuration_changed
};

#endif
