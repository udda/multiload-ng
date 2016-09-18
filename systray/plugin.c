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
#include "common/ui.h"


// Panel Specific Settings Implementation
#define MULTILOAD_CONFIG_BASENAME "systray.conf"
#include "common/ps-settings-impl-gkeyfile.inc"


static GtkWindow *dummy_window;
static GtkStatusIcon *status_icons[GRAPH_MAX];
static GtkWidget *menu;

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

	if (!g->config->visible)
		return;
	if (g->surface == NULL)
		return;
	if (status_icons[g->id] == NULL)
		return;

	for (i=0, v=0; i<GRAPH_MAX; i++) {
		visible = g->multiload->graph_config[i].visible;
		gtk_status_icon_set_visible(status_icons[i], visible);
		if (visible)
			v++;
	}

	if (!gtk_status_icon_is_embedded(status_icons[g->id]))
		return;
	guint size = gtk_status_icon_get_size(status_icons[g->id]);
	gtk_widget_set_size_request(GTK_WIDGET(g->multiload->container), v*GRAPH_MAX, size);
	gtk_window_resize(dummy_window, v*GRAPH_MAX, size);

	gtk_status_icon_set_tooltip_text(status_icons[g->id], gtk_widget_get_tooltip_text(g->disp));

	// set graph size from icon size
	g->config->size = size;

	// update icon pixbuf
	cairo_surface_t *surface = g->surface;
	GdkPixbuf *pixbuf = gdk_pixbuf_get_from_surface (surface, 0, 0, g->draw_width, g->draw_height);
	gtk_status_icon_set_from_pixbuf(status_icons[g->id], pixbuf);
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

		// set title
		title = g_strdup_printf("Multiload-ng: %s", graph_types[i].label);
		gtk_status_icon_set_title(status_icons[i], title);
		g_free(title);

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

	// UGLY HACK: create dummy window just to realize plugin
	dummy_window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
	gtk_container_add(GTK_CONTAINER(dummy_window), GTK_WIDGET(multiload->container));
	gtk_window_set_decorated(dummy_window, FALSE);
	gtk_window_set_skip_taskbar_hint(dummy_window, TRUE);
	gtk_window_set_skip_pager_hint(dummy_window, TRUE);
	gtk_window_set_keep_below(dummy_window, TRUE);
	gtk_window_set_accept_focus(dummy_window, FALSE);
	gtk_widget_set_can_focus (GTK_WIDGET(dummy_window), FALSE);
	gtk_widget_set_can_focus (GTK_WIDGET(multiload->container), FALSE);

	gtk_widget_show(GTK_WIDGET(dummy_window));

	build_menu(multiload);
	build_icons(multiload);

	gtk_main ();

	return 0;
}

#endif /* def MULTILOAD_EXPERIMENTAL */
