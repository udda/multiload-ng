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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>
#include <libappindicator/app-indicator.h>

#include "common/about-data.h"
#include "common/multiload.h"
#include "common/preferences.h"
#include "common/ui.h"


// Panel Specific Settings Implementation
#define MULTILOAD_CONFIG_BASENAME "indicator.conf"
#include "common/ps-settings-impl-gkeyfile.inc"

static AppIndicator *indicator;
static GtkWidget *offscr;
static gchar icon_directory[PATH_MAX];
static gchar icon_filename[2][PATH_MAX];
static int icon_current_index=0;
static gboolean indicator_connected;

static void
indicator_destroy_cb(GtkWidget *widget, MultiloadPlugin *ma)
{
    gtk_main_quit ();
    g_free(ma);

    // cleanup buffer files
    remove(icon_filename[0]);
    remove(icon_filename[1]);
    remove(icon_directory);
}

static void
indicator_preferences_cb(GtkWidget *widget, MultiloadPlugin *ma)
{
	GtkWidget *window = gtk_widget_get_toplevel (widget);
	GtkWidget *dialog = multiload_ui_configure_dialog_new(ma, GTK_WINDOW(window));
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(dialog));
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_present(GTK_WINDOW(dialog));

	multiload_preferences_disable_settings(
		MULTILOAD_SETTINGS_FILL_BETWEEN |
		MULTILOAD_SETTINGS_TOOLTIPS |
		MULTILOAD_SETTINGS_DBLCLICK_POLICY
	);
}

static int
indicator_get_icon_height()
{
	return 22; // seems to be a fixed size (it's hardcoded in libappindicator source)
}

static void
indicator_update_pixbuf(MultiloadPlugin *ma)
{
	GtkAllocation allocation;
	GError *error = NULL;

	cairo_status_t status;
	cairo_surface_t *surface;
	cairo_t *cr;

	int i;
	double x=ma->padding, y=ma->padding;

	gtk_widget_get_allocation (GTK_WIDGET(ma->container), &allocation);

	surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, allocation.width, allocation.height);
	g_assert (surface != NULL);
	cr = cairo_create (surface);
	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);

	for (i=0; i<GRAPH_MAX; i++) {
		if (ma->graph_config[i].visible) {
			cairo_set_source_surface (cr, ma->graphs[i]->surface, x, y);
			cairo_paint (cr);

			if (multiload_get_orientation(ma) == GTK_ORIENTATION_HORIZONTAL)
				x += ma->graphs[i]->draw_width + ma->spacing;
			else
				y += ma->graphs[i]->draw_height + ma->spacing;
		}
	}

	status = cairo_surface_write_to_png (surface, icon_filename[icon_current_index]);
	g_assert(status == CAIRO_STATUS_SUCCESS);
	if (error != NULL) {
		g_error("Cannot save Multiload-ng window to temporary buffer: %s\n", error->message);
		g_clear_error(&error);
	} else
		app_indicator_set_icon(indicator, icon_filename[icon_current_index]);

	icon_current_index = 1-icon_current_index;
	cairo_surface_destroy (surface);
	cairo_destroy (cr);
}

static void
indicator_graph_update_cb(LoadGraph *g, gpointer user_data)
{
	GtkAllocation allocation;
	int width;
	guint height;

	if (!g->config->visible)
		return;

	g_return_if_fail (g->surface != NULL);

	if (indicator_connected == FALSE) {
		g_warning ("Indicator is not connected to panel, thus it cannot be displayed.");
		return;
	}

	if (multiload_get_orientation(g->multiload) == GTK_ORIENTATION_HORIZONTAL)
		width = -1;
	else
		width = 100;
	height = indicator_get_icon_height();

	// resize widget and offscreen window to fit into panel
	gtk_widget_set_size_request(GTK_WIDGET(g->multiload->container), width, height);
	gtk_widget_get_allocation (GTK_WIDGET(g->multiload->container), &allocation);
	gtk_window_resize(GTK_WINDOW(offscr), allocation.width, allocation.height);

	indicator_update_pixbuf(g->multiload);
}

static void
indicator_connection_changed_cb (AppIndicator *indicator, gboolean connected, MultiloadPlugin *ma)
{
	indicator_connected = connected;
}

static void
set_defaults(MultiloadPlugin *ma)
{
	guint i;

	ma->fill_between = FALSE;
	for (i=0; i<GRAPH_MAX; i++) {
		ma->graph_config[i].tooltip_style = MULTILOAD_TOOLTIP_STYLE_SIMPLE;
		ma->graph_config[i].dblclick_policy = DBLCLICK_POLICY_DONOTHING;
	}

	multiload_set_fill_between (ma, ma->fill_between);

	multiload_ui_save(ma);
}

static void
create_buffer_files()
{
	gchar *tmp;

	gchar *template = g_build_filename(g_get_tmp_dir(), "multiload-ng.XXXXXX", NULL);
	char *dirname = mkdtemp(template);

	if (dirname == NULL) {
		g_error("Unable to create buffer directory.");
	} else {
		strncpy(icon_directory, dirname, PATH_MAX);
		g_debug("Using this directory for buffers: %s", icon_directory);

		tmp = g_build_filename(icon_directory, "mapped_icon0.png", NULL);
		strncpy(icon_filename[0], tmp, PATH_MAX);
		g_free(tmp);

		tmp = g_build_filename(icon_directory, "mapped_icon1.png", NULL);
		strncpy(icon_filename[1], tmp, PATH_MAX);
		g_free(tmp);
	}

	g_free(template); // apparently mkdtemp uses template buffer, so free it after its use
}


static void
build_menu(MultiloadPlugin *ma)
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
	guint i;

	MultiloadOptions *options = multiload_ui_parse_cmdline (&argc, &argv, NULL); //TODO put this in Porting Wiki page
	MultiloadPlugin *multiload = multiload_new();

	if (options->reset_settings)
		multiload_defaults (multiload);
	else
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

	// "connected" property means a visible indicator, otherwise could be hidden. or fall back to GtkStatusIcon
	g_signal_connect (G_OBJECT(indicator), "connection-changed", G_CALLBACK(indicator_connection_changed_cb), multiload);
	g_object_get(G_OBJECT(indicator), "connected", &indicator_connected, NULL);

	// set update callback
	for (i=0; i<GRAPH_MAX; i++)
		multiload_set_update_cb(multiload, i, indicator_graph_update_cb, indicator);

	set_defaults(multiload);
	create_buffer_files();
	build_menu(multiload);

	if (options->show_preferences)
		indicator_preferences_cb(GTK_WIDGET(indicator), multiload);

	gtk_main ();

	return 0;
}
