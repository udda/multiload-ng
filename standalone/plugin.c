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
#include "common/ui.h"


// Panel Specific Settings Implementation
#define MULTILOAD_CONFIG_BASENAME "standalone.conf"
#include "common/ps-settings-impl-gkeyfile.inc"


static void
standalone_destroy_cb(GtkWidget *widget, MultiloadPlugin *multiload)
{
    gtk_main_quit ();
    g_free(multiload);
}

void
standalone_settingsmenu_cb(GtkWidget *widget, GtkMenu *menu)
{
	gtk_menu_popup(menu, NULL, NULL, NULL, NULL, 1, 0);
}

void
standalone_preferences_cb(GtkWidget *widget, MultiloadPlugin *multiload)
{
	GtkWindow *window = multiload->panel_data;
	GtkWidget *dialog = multiload_ui_configure_dialog_new(multiload, window);
	gtk_window_set_transient_for(window, GTK_WINDOW(dialog));
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_present(GTK_WINDOW(dialog));
}


int main(int argc, char *argv[]) {
	gtk_init (&argc, &argv);

	MultiloadPlugin *multiload = multiload_new();

	multiload_ui_read (multiload);
	multiload_start(multiload);

	GtkWindow *w = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_title (w, about_data_progname);
	gtk_window_set_decorated (w, TRUE);
	gtk_window_set_keep_above (w, TRUE);
	gtk_window_set_icon_name (w, about_data_icon);
	multiload->panel_data = w;

	GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	GtkWidget *btnConfig = gtk_button_new_from_icon_name("preferences-system", GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_widget_show (btnConfig);
	gtk_widget_show (hbox);

	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(multiload->container), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), btnConfig, FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(w), hbox);


	GtkMenu *menu = GTK_MENU(gtk_menu_new());
	GtkWidget *menuitem;

	menuitem = gtk_image_menu_item_new_with_label (_("Start task manager"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_icon_name("utilities-system-monitor", GTK_ICON_SIZE_MENU));
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_start_system_monitor), multiload);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_menu_shell_append (GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_PREFERENCES, NULL);
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(standalone_preferences_cb), multiload);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_HELP, NULL);
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_show_help), NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_menu_shell_append (GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_show_about), w);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, NULL);
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(standalone_destroy_cb), multiload);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_widget_show_all(GTK_WIDGET(menu));


	g_signal_connect (G_OBJECT(w), "destroy", G_CALLBACK(standalone_destroy_cb), multiload);
	g_signal_connect (G_OBJECT(btnConfig), "clicked", G_CALLBACK(standalone_settingsmenu_cb), menu);
	gtk_container_set_border_width (GTK_CONTAINER (w), 0);


	gtk_window_present (w);

	gtk_main();

	return 0;
}
