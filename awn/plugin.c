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

#ifdef MULTILOAD_EXPERIMENTAL

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gi18n-lib.h>

#include <libawn/libawn.h>

#include "common/about-data.h"
#include "common/multiload.h"
#include "common/ui.h"


/*
gpointer
multiload_ps_settings_open_for_read(MultiloadPlugin *ma)
{
}
gpointer
multiload_ps_settings_open_for_save(MultiloadPlugin *ma)
{
}

gboolean
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
}
void
multiload_ps_settings_get_boolean(gpointer settings, const gchar *key, gboolean *destination)
{
}
void
multiload_ps_settings_get_string(gpointer settings, const gchar *key, gchar *destination, size_t maxlen)
{
}

void
multiload_ps_settings_set_int(gpointer settings, const gchar *key, int value)
{
}
void
multiload_ps_settings_set_boolean(gpointer settings, const gchar *key, gboolean value)
{
}
void
multiload_ps_settings_set_string(gpointer settings, const gchar *key, const gchar *value)
{
}

void
multiload_ps_preferences_closed_cb(MultiloadPlugin *ma)
{
}
*/

/*
GtkWidget*
lxpanel_configure_cb(LXPanel *panel, GtkWidget *ebox)
{
	MultiloadPlugin *multiload = lxpanel_plugin_get_data(ebox);
	return multiload_ui_configure_dialog_new(multiload,
		GTK_WINDOW(gtk_widget_get_toplevel (GTK_WIDGET(ebox))));
}
*/

static void
awn_position_changed_cb (AwnApplet *app, GtkPositionType orient, MultiloadPlugin *multiload)
{
	if ( orient == GTK_POS_LEFT || orient == GTK_POS_RIGHT )
		multiload->panel_orientation = GTK_ORIENTATION_VERTICAL;
	else // if ( orient == GTK_POS_TOP || orient == GTK_POS_BOTTOM )
		multiload->panel_orientation = GTK_ORIENTATION_HORIZONTAL;

	multiload_refresh_orientation(multiload);
}


AwnApplet*
awn_applet_factory_initp (const gchar *name, const gchar *uid, gint panel_id)
{
	AwnApplet *applet = awn_applet_new( name, uid, panel_id );
	MultiloadPlugin *multiload = multiload_new();

	multiload_ui_read (multiload);
	multiload_start(multiload);

	int size = awn_applet_get_size (applet);

	gtk_widget_set_size_request (GTK_WIDGET (applet),  size*2, size );
	gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(multiload->container));

	// connect to height and orientation changes
	g_signal_connect (G_OBJECT (applet), "position-changed", G_CALLBACK (awn_position_changed_cb), multiload);
//"offset-changed"
//"size-changed"
//"panel-configure-event"
//"origin-changed"
//"applet-deleted"
//"menu-creation"
//"flags-changed"
	
	return applet;
}

#endif /* def MULTILOAD_EXPERIMENTAL */
