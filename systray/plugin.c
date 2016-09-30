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
#include "common/multiload-config.h"
#include "common/preferences.h"
#include "common/ui.h"


// Panel Specific Settings Implementation
#define MULTILOAD_CONFIG_BASENAME "systray.conf"
#include "common/ps-settings-impl-gkeyfile.inc"


static GtkWidget *offscr;
static GtkStatusIcon *status_icons[GRAPH_MAX];
static GtkWidget *menu;
static guint timer_indexes[GRAPH_MAX];

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
	gtk_window_present(GTK_WINDOW(dialog));

	multiload_preferences_disable_settings(
		MULTILOAD_SETTINGS_SIZE |
		MULTILOAD_SETTINGS_PADDING |
		MULTILOAD_SETTINGS_SPACING |
		MULTILOAD_SETTINGS_ORIENTATION |
		MULTILOAD_SETTINGS_FILL_BETWEEN
	);
}


static void
systray_popup_menu (GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data)
{
	gtk_menu_popup (GTK_MENU(menu), NULL, NULL, NULL, NULL, button, activate_time);
}


static void
systray_graph_update_cb(LoadGraph *g, gpointer user_data)
{
	guint i, v;
	gboolean visible;
	GdkPixbuf *pixbuf;

	if (!g->config->visible)
		return;

	g_return_if_fail (g->surface != NULL);
	g_return_if_fail (status_icons[g->id] != NULL);

	for (i=0, v=0; i<GRAPH_MAX; i++) {
		visible = g->multiload->graph_config[i].visible;
		gtk_status_icon_set_visible(status_icons[i], visible);
		if (visible)
			v++;
	}

	g_return_if_fail (gtk_status_icon_is_embedded(status_icons[g->id]));

	guint icon_size = gtk_status_icon_get_size(status_icons[g->id]);

	gtk_window_resize(GTK_WINDOW(offscr), v*icon_size, icon_size);
	gtk_widget_set_size_request(GTK_WIDGET(g->multiload->container), v*icon_size, icon_size);

	// set graph size from icon size
	g->config->size = icon_size;

	// update icon pixbuf
#if GTK_API == 3
	pixbuf = gdk_pixbuf_get_from_surface (g->surface, 0, 0, g->draw_width, g->draw_height);
#elif GTK_API == 2
	pixbuf = gdk_pixbuf_get_from_drawable (NULL, GDK_DRAWABLE(gtk_widget_get_window(g->disp)), NULL, 0, 0, 0, 0, g->draw_width, g->draw_height);
#else
#error "Wrong GTK+ version."
#endif

	if (gdk_pixbuf_get_width(pixbuf) != icon_size || gdk_pixbuf_get_height(pixbuf) != icon_size)
		return; // incorrect pixbuf size - could be first drawing

	gtk_status_icon_set_from_pixbuf(status_icons[g->id], pixbuf);
}

gboolean
systray_tooltip_disable (GtkStatusIcon *status_icon)
{
	LoadGraph *g = g_object_get_data(G_OBJECT(status_icon), "graph");

	timer_indexes[g->id] = 0;

	g->tooltip_update = FALSE;
	gtk_status_icon_set_tooltip_markup(status_icon, gtk_status_icon_get_title(status_icon));

	return FALSE; // single shot timer
}

gboolean
systray_query_tooltip_cb (GtkStatusIcon *status_icon, gint x, gint y, gboolean keyboard_mode, GtkTooltip *tooltip, LoadGraph *g)
{
	g->tooltip_update = TRUE;

	if (timer_indexes[g->id] > 0)
		g_source_remove(timer_indexes[g->id]);
	timer_indexes[g->id] = g_timeout_add (g->config->interval+100, (GSourceFunc) systray_tooltip_disable, status_icon);

	gtk_tooltip_set_markup(tooltip, gtk_widget_get_tooltip_markup(g->disp));

	return TRUE;
}

static void
build_icons (MultiloadPlugin *ma)
{
	int i;
	gchar *title;

	// set proper settings
	multiload_set_padding(ma, 0);
	multiload_set_spacing(ma, 0);

	// create new icons
	for (i=GRAPH_MAX-1; i>=0; i--) {
		// create status icon
		status_icons[i] = gtk_status_icon_new_from_icon_name (about_data_icon);
		g_signal_connect (G_OBJECT(status_icons[i]), "popup-menu", G_CALLBACK(systray_popup_menu), ma);
		g_signal_connect (G_OBJECT(status_icons[i]), "query-tooltip", G_CALLBACK(systray_query_tooltip_cb), ma->graphs[i]);
		g_object_set_data(G_OBJECT(status_icons[i]), "graph", ma->graphs[i]);

		// set title
		title = g_strdup_printf("Multiload-ng: %s", graph_types[i].label);
		gtk_status_icon_set_title(status_icons[i], title);
		g_free(title);

		// enable "query-tooltip" signal
		gtk_status_icon_set_has_tooltip (status_icons[i], TRUE);

		multiload_set_update_cb(ma, i, systray_graph_update_cb, status_icons[i]);
	}

	multiload_ui_save(ma);
}


static void
build_menu(MultiloadPlugin *ma)
{
	GtkWidget *menuitem;
	menu = gtk_menu_new();

	menuitem = gtk_image_menu_item_new_with_label (_("Start task manager"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menuitem), gtk_image_new_from_icon_name("utilities-system-monitor", GTK_ICON_SIZE_MENU));
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_start_system_monitor), ma);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_PREFERENCES, NULL);
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(systray_preferences_cb), ma);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_HELP, NULL);
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_show_help), NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_show_about), NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, NULL);
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(systray_destroy_cb), ma);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_widget_show_all(menu);
}


int main (int argc, char **argv)
{

	gtk_init (&argc, &argv);

	MultiloadPlugin *multiload = multiload_new();

	multiload_ui_read (multiload);
	multiload_start(multiload);

	memset(status_icons, 0, sizeof(status_icons));
	memset(timer_indexes, 0, sizeof(timer_indexes));

	// create offscreen window to keep widget drawing
	offscr = gtk_offscreen_window_new ();
	gtk_widget_set_size_request(GTK_WIDGET(multiload->container), -1, -1);
	gtk_container_add(GTK_CONTAINER(offscr), GTK_WIDGET(multiload->container));
	gtk_widget_show(offscr);

	build_menu(multiload);
	build_icons(multiload);

	gtk_main ();

	return 0;
}

#endif /* def MULTILOAD_EXPERIMENTAL */
