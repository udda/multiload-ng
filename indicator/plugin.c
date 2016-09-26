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

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>
#include <libappindicator/app-indicator.h>

#include "common/about-data.h"
#include "common/multiload.h"
#include "common/ui.h"


// Panel Specific Settings Implementation
#define MULTILOAD_CONFIG_BASENAME "indicator.conf"
#include "common/ps-settings-impl-gkeyfile.inc"


static GtkWidget *offscr;
static gchar *icon_filename;

static void
indicator_destroy_cb(GtkWidget *widget, MultiloadPlugin *ma)
{
    gtk_main_quit ();
    g_free(ma);
}

static void
indicator_preferences_cb(GtkWidget *widget, MultiloadPlugin *ma)
{
	GtkWidget *window = gtk_widget_get_toplevel (widget);
	GtkWidget *dialog = multiload_ui_configure_dialog_new(ma, GTK_WINDOW(window));
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(dialog));
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_present(GTK_WINDOW(dialog));
}

static void
indicator_graph_update_cb(LoadGraph *g, gpointer user_data)
{
	static guint64 last_msec = 0;
	guint64 now_msec;
	struct timeval tv;
	gboolean too_early;

	GtkAllocation allocation;
	GdkPixbuf *pixbuf;
	GError *error = NULL;
	guint height = 24; //TODO retrieve actual panel height

	// ignore updates if too close each other
	gettimeofday(&tv, NULL);
	now_msec = tv.tv_sec*1000 + tv.tv_usec/1000;
	if ( (now_msec-last_msec) < 200 )
		return;
	last_msec = now_msec;

	// resize widget and offscreen window to fit into panel
	gtk_widget_set_size_request(GTK_WIDGET(g->multiload->container), -1, height);
	gtk_widget_get_allocation (GTK_WIDGET(g->multiload->container), &allocation);
	gtk_window_resize(GTK_WINDOW(offscr), allocation.width, allocation.height);

	// update icon pixbuf
	pixbuf = gtk_offscreen_window_get_pixbuf (GTK_OFFSCREEN_WINDOW(offscr));
	gdk_pixbuf_save (pixbuf, icon_filename, "png", &error, "compression", "0", NULL);
	if (error != NULL)
		printf("ERR: %s\n",error->message); // TODO better error handling
	else if (gdk_pixbuf_get_width(pixbuf) == allocation.width && gdk_pixbuf_get_height(pixbuf) == allocation.height)
		app_indicator_set_icon(APP_INDICATOR(user_data), icon_filename);

	g_clear_error(&error);
	g_object_unref(pixbuf);
}

static void
create_buffer_file()
{
	gchar *template = g_build_filename(g_get_tmp_dir(), "multiload-ng.XXXXXX", NULL);
	char *dirname = mkdtemp(template);

	if (dirname != NULL) {
		icon_filename = g_build_filename(dirname, "mapped_icon.png", NULL);
		// TODO error message (check errno) and exit
		// TODO memory mapped file? (or other means to avoid repetitive read/write to disk)
	}

	g_free(template);
}

static void
set_graph_update_cb (AppIndicator *indicator, MultiloadPlugin *ma)
{
	int i;

	for (i=GRAPH_MAX-1; i>=0; i--)
		multiload_set_update_cb(ma, i, indicator_graph_update_cb, indicator);
}

static void
build_menu(AppIndicator *indicator, MultiloadPlugin *ma)
{
	GtkWidget *menuitem;
	GtkWidget *menu = gtk_menu_new();

	menuitem = gtk_image_menu_item_new_with_label (_("Start task manager"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_icon_name("utilities-system-monitor", GTK_ICON_SIZE_MENU));
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_start_system_monitor), ma);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_PREFERENCES, NULL);
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(indicator_preferences_cb), ma);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_HELP, NULL);
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_show_help), NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_show_about), NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, NULL);
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(indicator_destroy_cb), ma);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_widget_show_all(menu);
	app_indicator_set_menu (indicator, GTK_MENU (menu));
}


int main (int argc, char **argv)
{
	AppIndicator *indicator;

	gtk_init (&argc, &argv);

	MultiloadPlugin *multiload = multiload_new();

	multiload_ui_read (multiload);
	multiload_start(multiload);

	// create offscreen window to keep widget drawing
	offscr = gtk_offscreen_window_new ();
	gtk_widget_set_size_request(GTK_WIDGET(multiload->container), -1, -1);
	gtk_container_add(GTK_CONTAINER(offscr), GTK_WIDGET(multiload->container));
	gtk_widget_show(offscr);

	// create indicator
	indicator = app_indicator_new ("indicator-multiload-ng", about_data_icon, APP_INDICATOR_CATEGORY_HARDWARE);
	app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);

	create_buffer_file();
	build_menu(indicator, multiload);
	set_graph_update_cb(indicator, multiload);

	gtk_main ();

	return 0;
}

#endif /* def MULTILOAD_EXPERIMENTAL */
