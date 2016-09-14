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

#include <string.h>
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>

#include "common/about-data.h"
#include "common/multiload.h"
#include "common/ui.h"


#define G_KEY_GROUP_NAME "Multiload"

gchar* systray_build_config_filename() {
	return g_build_filename(MULTILOAD_CONFIG_PATH, "systray.conf", NULL);
}


gpointer
multiload_ps_settings_open_for_read(MultiloadPlugin *ma)
{
	GKeyFile *gkf;
	GError *err = NULL;
	gboolean res;
	gchar* fname = systray_build_config_filename();

	gkf = g_key_file_new();
	if (g_file_test(fname, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
		res = g_key_file_load_from_file(gkf, fname, G_KEY_FILE_NONE, &err);
		if (!res) {
			g_warning("multiload_ps_settings_open_for_read: %s", err->message);
			g_key_file_free(gkf);
			gkf = NULL;
		}
	}

	g_clear_error(&err);
	return gkf;
}
gpointer
multiload_ps_settings_open_for_save(MultiloadPlugin *ma)
{
	return g_key_file_new();
}

gboolean
multiload_ps_settings_save(gpointer settings)
{
	GError *err = NULL;
	gboolean res;
	gchar* fname = systray_build_config_filename();

	res = g_key_file_save_to_file((GKeyFile*)settings, fname, &err);

	if (!res)
		g_warning("multiload_ps_settings_save: %s", err->message);

	g_free(fname);
	g_clear_error(&err);
	return res;
}

void
multiload_ps_settings_close(gpointer settings)
{
	g_key_file_free((GKeyFile*)settings);
}

void
multiload_ps_settings_get_int(gpointer settings, const gchar *key, int *destination)
{
	GError *err = NULL;
	int temp;

	temp = g_key_file_get_integer ((GKeyFile*)settings, G_KEY_GROUP_NAME, key, &err);
	if (G_LIKELY(err == NULL || err->code == 0))
		*destination = temp;

	g_clear_error(&err);
}
void
multiload_ps_settings_get_boolean(gpointer settings, const gchar *key, gboolean *destination)
{
	GError *err = NULL;
	gboolean temp;

	temp = g_key_file_get_boolean ((GKeyFile*)settings, G_KEY_GROUP_NAME, key, &err);
	if (G_LIKELY(err == NULL || err->code == 0))
		*destination = temp;

	g_clear_error(&err);
}
void
multiload_ps_settings_get_string(gpointer settings, const gchar *key, gchar *destination, size_t maxlen)
{
	GError *err = NULL;
	gchar *temp;

	temp = g_key_file_get_string ((GKeyFile*)settings, G_KEY_GROUP_NAME, key, &err);
	if (G_LIKELY(err == NULL || err->code == 0)) {
		if (temp == NULL)
			destination[0] = 0;
		else
			strncpy(destination, temp, maxlen);
	}

	g_clear_error(&err);
	g_free(temp);
}

void
multiload_ps_settings_set_int(gpointer settings, const gchar *key, int value)
{
	g_key_file_set_integer ((GKeyFile*)settings, G_KEY_GROUP_NAME, key, value);
}
void
multiload_ps_settings_set_boolean(gpointer settings, const gchar *key, gboolean value)
{
	g_key_file_set_boolean ((GKeyFile*)settings, G_KEY_GROUP_NAME, key, value);
}
void
multiload_ps_settings_set_string(gpointer settings, const gchar *key, const gchar *value)
{
	g_key_file_set_string ((GKeyFile*)settings, G_KEY_GROUP_NAME, key, value);
}

void
multiload_ps_preferences_closed_cb(MultiloadPlugin *ma)
{
}



static void
systray_destroy_cb(GtkWidget *widget, MultiloadPlugin *ma)
{
    gtk_main_quit ();
    g_free(ma);
}

static void
systray_preferences_cb(GtkWidget *widget, MultiloadPlugin *ma)
{
	GtkWidget *window = gtk_widget_get_toplevel (widget);
	GtkWidget *dialog = multiload_ui_configure_dialog_new(ma, GTK_WINDOW(window));
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(dialog));
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_widget_show(dialog);
}


static void
systray_popup_menu (GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data)
{
	GtkMenu *menu = GTK_MENU(g_object_get_data(G_OBJECT(status_icon), "menu"));
	gtk_menu_popup (menu, NULL, NULL, NULL, NULL, button, activate_time);
}


int main (int argc, char **argv)
{
	GtkStatusIcon *status_icon;
	GtkWidget *menu;
	GtkWidget *menuitem;

	gtk_init (&argc, &argv);

	MultiloadPlugin *multiload = multiload_new();
	gtk_widget_set_size_request(GTK_WIDGET(multiload->container), -1, 30);

	multiload_ui_read (multiload);
	multiload_start(multiload);


	GtkWidget *menuitem_container = gtk_menu_item_new();
	gtk_container_add(GTK_CONTAINER(menuitem_container), GTK_WIDGET(multiload->container));
	gtk_widget_set_sensitive(menuitem_container, FALSE);


	menu = gtk_menu_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem_container);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	menuitem = gtk_image_menu_item_new_with_label (_("Start task manager"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_icon_name("utilities-system-monitor", GTK_ICON_SIZE_MENU));
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_start_system_monitor), multiload);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_PREFERENCES, NULL);
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(systray_preferences_cb), multiload);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_HELP, NULL);
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_show_help), NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_show_about), NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, NULL);
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(systray_destroy_cb), multiload);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_widget_show_all(menu);

	status_icon = gtk_status_icon_new_from_icon_name (about_data_icon);
	gtk_status_icon_set_title(status_icon, "Multiload-ng");
	g_object_set_data(G_OBJECT(status_icon), "menu", menu);
	g_signal_connect (G_OBJECT(status_icon), "popup-menu", G_CALLBACK(systray_popup_menu), multiload);

	gtk_main ();

	return 0;
}

#endif /* def MULTILOAD_EXPERIMENTAL */
