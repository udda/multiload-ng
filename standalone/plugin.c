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

#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gi18n-lib.h>

#include "common/about-data.h"
#include "common/gtk-compat.h"
#include "common/multiload.h"
#include "common/preferences.h"
#include "common/ui.h"


// Panel Specific Settings Implementation
#define MULTILOAD_CONFIG_BASENAME "standalone.conf"
#include "common/ps-settings-impl-gkeyfile.inc"


static void
standalone_destroy_cb (GtkWidget *widget, MultiloadPlugin *multiload)
{
    gtk_main_quit ();
    g_free(multiload);
}

static void
standalone_settingsmenu_cb (GtkWidget *widget, GtkMenu *menu)
{
	#if GTK_CHECK_VERSION(3,22,0)
		gtk_menu_popup_at_widget (menu, widget, GDK_GRAVITY_SOUTH_EAST, GDK_GRAVITY_NORTH_EAST, NULL);
	#else
		gtk_menu_popup(menu, NULL, NULL, NULL, NULL, 1, 0);
	#endif
}

static void
standalone_preferences_cb (GtkWidget *widget, MultiloadPlugin *multiload)
{
	GtkWindow *window = multiload->panel_data;
	GtkWidget *dialog = multiload_ui_configure_dialog_new(multiload, window);
	gtk_window_set_transient_for(window, GTK_WINDOW(dialog));
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_present(GTK_WINDOW(dialog));

	multiload_preferences_disable_settings(MULTILOAD_SETTINGS_ORIENT_WARNING);
	multiload_preferences_add_infobar (GTK_MESSAGE_INFO, _("If the window is larger than its minimum size, graphs will be scaled in proportion to the dimensions set."));
}

static gboolean
standalone_resize_cb (GtkWidget *dialog, GdkEventConfigure *event, MultiloadPlugin *ma)
{
	if (event->type == GDK_CONFIGURE) {
		if (event->width >= event->height)
			ma->panel_orientation = GTK_ORIENTATION_HORIZONTAL;
		else
			ma->panel_orientation = GTK_ORIENTATION_VERTICAL;

		multiload_refresh_orientation(ma);
	}

	return FALSE;
}

#ifdef MULTILOAD_EXPERIMENTAL

static gboolean window_decorated = TRUE; //TODO preferences
static gboolean window_above = TRUE; //TODO preferences
static gboolean window_show_button = TRUE; //TODO preferences
static gboolean window_allow_move = TRUE; //TODO preferences
static gboolean window_allow_resize = TRUE; //TODO preferences

static void
standalone_above_cb (GtkCheckMenuItem *item, GtkWindow *window)
{
	gboolean v = gtk_check_menu_item_get_active(item);
	gtk_window_set_keep_above(window, v);
}

static void
standalone_decorated_cb (GtkCheckMenuItem *item, GtkWindow *window)
{
	gboolean v = gtk_check_menu_item_get_active(item);
	gtk_window_set_decorated(window, v);
}

static void
standalone_show_button_cb (GtkCheckMenuItem *item, GtkWidget *button_config)
{
	gboolean v = gtk_check_menu_item_get_active(item);
	printf("TODO [%c]\n", v?'X':' ');
//	gtk_widget_set_visible(button_config, v)
	//TODO notice user to use right click when disabled
}

static void
standalone_allow_move_cb (GtkCheckMenuItem *item, GtkWindow *window)
{
	window_allow_move = gtk_check_menu_item_get_active(item);
	printf("TODO [%c]\n", window_allow_move?'X':' ');
}

static void
standalone_allow_resize_cb (GtkCheckMenuItem *item, GtkWindow *window)
{
	window_allow_resize = gtk_check_menu_item_get_active(item);
	printf("TODO [%c]\n", window_allow_resize?'X':' ');
}

static void
standalone_handle_press_cb (GtkWidget *widget, GdkEventButton *event, GtkWindow *window)
{
	if (window_allow_move)
		gtk_window_begin_move_drag(window, event->button, event->x_root, event->y_root, event->time);
}

#endif

static void
build_menu(MultiloadPlugin *ma, GtkWidget *button_config)
{
	GtkMenu *menu = GTK_MENU(gtk_menu_new());
	GtkWidget *menuitem;

	menuitem = gtk_menu_item_new_with_label (_("Start task manager"));
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_start_system_monitor), ma);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_menu_shell_append (GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	menuitem = gtk_menu_item_new_with_mnemonic (_("_Preferences"));
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(standalone_preferences_cb), ma);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	#ifdef MULTILOAD_EXPERIMENTAL

	GtkMenu *submenu = GTK_MENU(gtk_menu_new());
	menuitem = gtk_menu_item_new_with_mnemonic (_("Extra settings"));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), GTK_WIDGET(submenu));
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	#endif

	menuitem = gtk_menu_item_new_with_mnemonic (_("_Help"));
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_show_help), NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_menu_shell_append (GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	menuitem = gtk_menu_item_new_with_mnemonic (_("_About"));
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_show_about), ma->panel_data);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_mnemonic (_("_Quit"));
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(standalone_destroy_cb), ma);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	#ifdef MULTILOAD_EXPERIMENTAL

	menuitem = gtk_check_menu_item_new_with_mnemonic (_("_Always on top"));
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menuitem), window_above); //TODO preferences
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(standalone_above_cb), ma->panel_data);
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_check_menu_item_new_with_mnemonic (_("Show window _borders"));
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menuitem), window_decorated); //TODO preferences
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(standalone_decorated_cb), ma->panel_data);
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_check_menu_item_new_with_mnemonic (_("Show _settings button"));
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menuitem), window_show_button); //TODO preferences
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(standalone_show_button_cb), button_config);
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_check_menu_item_new_with_mnemonic (_("Allow _move"));
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menuitem), window_allow_move); //TODO preferences
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(standalone_allow_move_cb), ma->panel_data);
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	menuitem = gtk_check_menu_item_new_with_mnemonic (_("Allow _resize"));
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menuitem), window_allow_resize); //TODO preferences
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(standalone_allow_resize_cb), ma->panel_data);
	gtk_menu_shell_append (GTK_MENU_SHELL(submenu), menuitem);

	#endif

	gtk_widget_show_all(GTK_WIDGET(menu));

	g_signal_connect (G_OBJECT(button_config), "clicked", G_CALLBACK(standalone_settingsmenu_cb), menu);
}

int main(int argc, char *argv[]) {
	MultiloadOptions *options = multiload_ui_parse_cmdline (&argc, &argv, NULL);
	MultiloadPlugin *multiload = multiload_new();

	if (options->reset_settings)
		multiload_defaults (multiload);
	else
		multiload_ui_read (multiload);

	multiload_start(multiload);

	GtkWindow *w = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_title (w, about_data_progname);
	gtk_window_set_decorated (w, TRUE);
	gtk_window_set_keep_above (w, TRUE);
	gtk_window_set_icon_name (w, about_data_icon);
	multiload->panel_data = w;

	GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_widget_show (hbox);

	GtkWidget *btnConfig = gtk_button_new_from_icon_name("preferences-system", GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_widget_show (btnConfig);

	#ifdef MULTILOAD_EXPERIMENTAL
	GtkWidget *handle = gtk_event_box_new();
	gtk_container_add (GTK_CONTAINER(handle), gtk_label_new("(move)"));
	g_signal_connect (G_OBJECT(handle), "button-press-event", G_CALLBACK(standalone_handle_press_cb), w);
	gtk_box_pack_start(GTK_BOX(hbox), handle, FALSE, FALSE, 0);
	#endif

	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(multiload->container), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), btnConfig, FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(w), hbox);

	g_signal_connect (G_OBJECT(w), "destroy", G_CALLBACK(standalone_destroy_cb), multiload);
	g_signal_connect (G_OBJECT(w), "configure-event", G_CALLBACK(standalone_resize_cb), multiload);
	gtk_container_set_border_width (GTK_CONTAINER (w), 0);

	build_menu(multiload, btnConfig);

	gtk_window_present (w);

	if (options->show_preferences)
		standalone_preferences_cb(GTK_WIDGET(w), multiload);

	gtk_main();

	return 0;
}
