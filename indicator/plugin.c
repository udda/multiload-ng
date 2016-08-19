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

#ifdef MULTILOAD_EXPERIMENTAL_ENABLE

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>




int main (int argc, char **argv)
{
	AppIndicator *indicator;
	GtkWidget *menu;
	GtkWidget *menu_item;

	gtk_init (&argc, &argv);

	indicator = app_indicator_new ("example-simple-client", "utilities-system-monitor", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	menu = gtk_menu_new();
	menu_item = gtk_menu_item_new_with_label("Test1");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	menu_item = gtk_menu_item_new_with_label("Test2");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	menu_item = gtk_menu_item_new_with_label("Test3");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	menu_item = gtk_menu_item_new_with_label("Test4");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	app_indicator_set_label(indicator, "TestLabel", "TestGuide");
	app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);

	app_indicator_set_menu (indicator, GTK_MENU (menu));

	gtk_main ();

	return 0;
}

#endif /* def MULTILOAD_EXPERIMENTAL_ENABLE */
