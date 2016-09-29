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


static DesktopAgnosticConfigClient *_settings = NULL;

gpointer
multiload_ps_settings_open_for_read(MultiloadPlugin *ma)
{
	return _settings;
}
gpointer
multiload_ps_settings_open_for_save(MultiloadPlugin *ma)
{
	return _settings;
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
	GError *error = NULL;
	gint value = desktop_agnostic_config_client_get_int ((DesktopAgnosticConfigClient*)settings, DESKTOP_AGNOSTIC_CONFIG_GROUP_DEFAULT, key, &error);

	if (G_LIKELY(error != NULL))
		*destination = value;

	g_clear_error(&error);
}
void
multiload_ps_settings_get_boolean(gpointer settings, const gchar *key, gboolean *destination)
{
	GError *error = NULL;
	gboolean value = desktop_agnostic_config_client_get_bool ((DesktopAgnosticConfigClient*)settings, DESKTOP_AGNOSTIC_CONFIG_GROUP_DEFAULT, key, &error);

	if (G_LIKELY(error != NULL))
		*destination = value;

	g_clear_error(&error);
}
void
multiload_ps_settings_get_string(gpointer settings, const gchar *key, gchar *destination, size_t maxlen)
{
	GError *error = NULL;
	gchar *value = desktop_agnostic_config_client_get_string ((DesktopAgnosticConfigClient*)settings, DESKTOP_AGNOSTIC_CONFIG_GROUP_DEFAULT, key, &error);

	if (G_LIKELY(error != NULL && value != NULL))
		strncpy(destination, value, maxlen);

	g_free(value);
	g_clear_error(&error);
}

void
multiload_ps_settings_set_int(gpointer settings, const gchar *key, int value)
{
	GError *error = NULL;
	desktop_agnostic_config_client_set_int ((DesktopAgnosticConfigClient*)settings, DESKTOP_AGNOSTIC_CONFIG_GROUP_DEFAULT, key, value, &error);
	g_clear_error(&error);
}
void
multiload_ps_settings_set_boolean(gpointer settings, const gchar *key, gboolean value)
{
	GError *error = NULL;
	desktop_agnostic_config_client_set_bool ((DesktopAgnosticConfigClient*)settings, DESKTOP_AGNOSTIC_CONFIG_GROUP_DEFAULT, key, value, &error);
	g_clear_error(&error);
}
void
multiload_ps_settings_set_string(gpointer settings, const gchar *key, const gchar *value)
{
	GError *error = NULL;
	desktop_agnostic_config_client_set_string ((DesktopAgnosticConfigClient*)settings, DESKTOP_AGNOSTIC_CONFIG_GROUP_DEFAULT, key, value, &error);
	g_clear_error(&error);
}

void
multiload_ps_preferences_closed_cb(MultiloadPlugin *ma)
{
}



static void
awn_preferences_cb(AwnApplet *applet, MultiloadPlugin *multiload)
{
	GtkWidget *dialog = multiload_ui_configure_dialog_new(multiload,
		GTK_WINDOW(gtk_widget_get_toplevel (GTK_WIDGET(applet))));

	gtk_window_present(GTK_WINDOW(dialog));
}

static void
awn_applet_deleted_cb (AwnApplet *app, MultiloadPlugin *multiload)
{
	gtk_widget_destroy (GTK_WIDGET(multiload->container));
	g_free(multiload);
}

static gboolean
awn_button_press_event_cb(AwnApplet *applet, GdkEventButton *event, MultiloadPlugin *multiload)
{
	static GtkWidget *menu = NULL;
	GtkWidget *menuitem;

	if (event->button == 1) {
	} else if (event->button == 3) {
		if (menu == NULL) {
			menu = awn_applet_create_default_menu(applet);

			menuitem = gtk_image_menu_item_new_with_label (_("Start task manager"));
			gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_icon_name("utilities-system-monitor", GTK_ICON_SIZE_MENU));
			g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_start_system_monitor), multiload);
			gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

			gtk_menu_shell_append (GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

			menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_PREFERENCES, NULL);
			g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(awn_preferences_cb), multiload);
			gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

			menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_HELP, NULL);
			g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_show_help), NULL);
			gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

			gtk_menu_shell_append (GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

			menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
			g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_show_about), NULL);
			gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

			gtk_widget_show_all(menu);
		}

		if (menu != NULL) {
			awn_applet_popup_gtk_menu (AWN_APPLET(applet), menu, event->button, event->time);
		}
	}
	return FALSE;
}

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
	GError *error = NULL;
	AwnApplet *applet = awn_applet_new( name, uid, panel_id );
	MultiloadPlugin *multiload = multiload_new();

	_settings = awn_config_get_default_for_applet(applet, &error);
	if (error != NULL) printf("ERR: %s\n",error->message);
	//TODO check for errors
	g_clear_error(&error);

	multiload_ui_read (multiload);
	multiload_start(multiload);

	int size = awn_applet_get_size (applet);

//	gtk_widget_set_size_request (GTK_WIDGET (multiload->container), size, size );
	gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(multiload->container));

	g_signal_connect (G_OBJECT (applet), "position-changed",   G_CALLBACK (awn_position_changed_cb), multiload);
	g_signal_connect (G_OBJECT (applet), "applet-deleted",     G_CALLBACK (awn_applet_deleted_cb), multiload);
	g_signal_connect (G_OBJECT (applet), "button-press-event", G_CALLBACK (awn_button_press_event_cb), multiload);

	//TODO set correct size, taking in account orientation, size and offset - do that on startup and on change of any of these properties

	// Signals:
	//"offset-changed"
	//"size-changed"
	//"panel-configure-event"
	//"origin-changed"
	//"menu-creation"
	//"flags-changed"

	return applet;
}

#endif /* def MULTILOAD_EXPERIMENTAL */
