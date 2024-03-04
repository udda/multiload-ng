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
#include <signal.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>
#include <libappindicator/app-indicator.h>

#include "common/about-data.h"
#include "common/multiload.h"
#include "common/multiload-config.h"
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
static GtkWidget *graphs_menu_items[GRAPH_MAX];

static int lasttick;

static void
indicator_cleanup(int sig)
{
	char signame[12];
	if (sig == SIGTERM)
		strcpy(signame, "SIGTERM");
	else if (sig == SIGINT)
		strcpy(signame, "SIGINT");
	else
		sprintf(signame, "Signal %d", sig);

	printf("Received %s, cleaning up...\n", signame);
	remove(icon_filename[0]);
	remove(icon_filename[1]);
	remove(icon_directory);

	exit(0);
}

static void
indicator_destroy_cb(GtkWidget *widget, MultiloadPlugin *ma)
{
	gtk_main_quit ();
	multiload_free (ma);
	indicator_cleanup (0);
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
indicator_update_menu(MultiloadPlugin *ma)
{
	guint i;
	gchar title[98], text[800], label[900];

	for (i=0; i<GRAPH_MAX; i++) {
		ma->graphs[i]->tooltip_update = ma->graph_config[i].visible;
		gtk_widget_set_visible(graphs_menu_items[i], ma->graph_config[i].visible);

		if (ma->graph_config[i].visible) {
			graph_types[i].tooltip_update(title, sizeof(title), text, sizeof(text), ma->graphs[i], ma->extra_data[i], ma->graph_config[i].tooltip_style);
			g_snprintf(label, sizeof(label), "%s: %s", graph_types[i].label, text);
			gtk_menu_item_set_label(GTK_MENU_ITEM(graphs_menu_items[i]), label);
		}

	}
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
	} else {
		app_indicator_set_icon(indicator, icon_filename[icon_current_index]);
	}
	icon_current_index = 1-icon_current_index;
	cairo_surface_destroy (surface);
	cairo_destroy (cr);
}

static void
indicator_graph_update_cb(LoadGraph *g, gpointer user_data)
{
	GtkAllocation allocation;

	// Don't tick too often!
	int now = time(NULL);
	if (now - lasttick < 1) {
		return;
	}
	lasttick = now;

	indicator_update_menu(g->multiload);

	if (!g->config->visible)
		return;

	g_return_if_fail (g->surface != NULL);

	if (indicator_connected == FALSE) {
		g_warning ("Indicator is not connected to panel, thus it cannot be displayed.");
		return;
	}

	// resize widget and offscreen window to fit into panel
	int icon_height = indicator_get_icon_height();
	if (multiload_get_orientation(g->multiload) == GTK_ORIENTATION_HORIZONTAL)
		gtk_widget_set_size_request(GTK_WIDGET(g->multiload->container), multiload_calculate_size_request(g->multiload), icon_height);
	else
		gtk_widget_set_size_request(GTK_WIDGET(g->multiload->container), 120, icon_height); //TODO 120 should not be hardcoded

	gtk_widget_get_allocation (GTK_WIDGET(g->multiload->container), &allocation);
	if (allocation.height != icon_height) {
		// If we render now then we don't recover, so skip it
		return;
	}

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

	for (i=0; i<GRAPH_MAX; i++) {
		ma->graph_config[i].tooltip_style = MULTILOAD_TOOLTIP_STYLE_SIMPLE;
		ma->graph_config[i].dblclick_policy = DBLCLICK_POLICY_DONOTHING;
	}

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
	guint i;
	GtkWidget *menuitem;
	GtkWidget *menu = gtk_menu_new();

	menuitem = gtk_menu_item_new_with_label (_("Start task manager"));
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_start_system_monitor), ma);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	for (i=0; i<GRAPH_MAX; i++) {
		graphs_menu_items[i] = gtk_menu_item_new_with_label (graph_types[i].label);
		gtk_widget_set_sensitive(graphs_menu_items[i], FALSE);
		gtk_menu_shell_append (GTK_MENU_SHELL(menu), graphs_menu_items[i]);
	}

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	menuitem = gtk_menu_item_new_with_mnemonic (_("_Preferences"));
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(indicator_preferences_cb), ma);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_mnemonic (_("_Help"));
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_show_help), NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	menuitem = gtk_menu_item_new_with_mnemonic (_("_About"));
	g_signal_connect (G_OBJECT(menuitem), "activate", G_CALLBACK(multiload_ui_show_about), NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_mnemonic (_("_Quit"));
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

	// cleanup on SIGTERM
	signal(SIGINT, indicator_cleanup);
	signal(SIGTERM, indicator_cleanup);

	if (options->show_about)
		multiload_ui_show_about(NULL);
	if (options->show_preferences)
		indicator_preferences_cb(GTK_WIDGET(indicator), multiload);

	gtk_main ();

	return 0;
}
