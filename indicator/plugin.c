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

#include "common/about-data.h"

/*
README

This is an empty indicator. At the moment it does nothing. The libappindicator
structure allows only an icon and a label, so in order to draw the plugin we will
redraw over the indicator icon. As this requires large modifications in the code,
indicator plugin is postponed.
*/

int main (int argc, char **argv)
{
	AppIndicator *indicator;
	GtkWidget *menu;

	gtk_init (&argc, &argv);

	indicator = app_indicator_new ("indicator-multiload-ng", about_data_icon, APP_INDICATOR_CATEGORY_HARDWARE);

	menu = gtk_menu_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_menu_item_new_with_label("This"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_menu_item_new_with_label("is"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_menu_item_new_with_label("not"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_menu_item_new_with_label("yet"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_menu_item_new_with_label("implemented"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_menu_item_new_with_label("sorry!"));
	gtk_widget_show_all(menu);

	app_indicator_set_label(indicator, "Multiload-ng", "Multiload-ng");
	app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);

	app_indicator_set_menu (indicator, GTK_MENU (menu));

	gtk_main ();

	return 0;
}

#endif /* def MULTILOAD_EXPERIMENTAL_ENABLE */
