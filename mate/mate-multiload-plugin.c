/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of multiload-nandhp.
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

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gi18n-lib.h>

#include <mate-panel-applet.h>
#include <mate-panel-applet-gsettings.h>

#include "common/about-data.h"
#include "common/multiload.h"
#include "common/ui.h"



gpointer
multiload_ps_settings_open_for_read(MultiloadPlugin *ma)
{
	return ma->panel_data;
}
gpointer
multiload_ps_settings_open_for_save(MultiloadPlugin *ma)
{
	return ma->panel_data;
}

void
multiload_ps_settings_close(gpointer settings)
{
}

void
multiload_ps_settings_get_int(gpointer settings, const gchar *key, int *destination)
{
	*destination = g_settings_get_int((GSettings*)settings, key);
}
void
multiload_ps_settings_get_boolean(gpointer settings, const gchar *key, gboolean *destination)
{
	*destination = g_settings_get_boolean((GSettings*)settings, key);
}
void
multiload_ps_settings_get_string(gpointer settings, const gchar *key, gchar *destination, size_t maxlen)
{
	const gchar* temp = g_settings_get_string((GSettings*)settings, key);
	if (G_LIKELY(temp != NULL))
		strncpy(destination, temp, maxlen);
}

void
multiload_ps_settings_set_int(gpointer settings, const gchar *key, int value)
{
	g_settings_set_int((GSettings*)settings, key, value);
}
void
multiload_ps_settings_set_boolean(gpointer settings, const gchar *key, gboolean value)
{
	g_settings_set_boolean((GSettings*)settings, key, value);
}
void
multiload_ps_settings_set_string(gpointer settings, const gchar *key, const gchar *value)
{
	g_settings_set_string((GSettings*)settings, key, value);
}

void
multiload_ps_preferences_closed_cb(MultiloadPlugin *ma)
{
}



static void
mate_sysmon_cb (GtkAction *action, MultiloadPlugin *ma)
{
	multiload_ui_start_system_monitor(ma);
}

static void
mate_properties_cb (GtkAction *action, MultiloadPlugin *ma)
{
	GtkWidget *dialog = multiload_ui_configure_dialog_new(ma,
		GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (ma->container))));
	gtk_widget_show (dialog);
}

static void
mate_help_cb (GtkAction *action, gpointer data)
{
	multiload_ui_show_help();
}

static void
mate_about_cb (GtkAction *action, MultiloadPlugin *ma)
{
	multiload_ui_show_about(
		GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (ma->container))));
}

static void
mate_orientation_cb (MatePanelApplet *applet, guint orient, MultiloadPlugin *ma)
{
	if (orient == MATE_PANEL_APPLET_ORIENT_UP || orient == MATE_PANEL_APPLET_ORIENT_DOWN)
		ma->panel_orientation = GTK_ORIENTATION_HORIZONTAL;
	else // if (orient == MATE_PANEL_APPLET_ORIENT_LEFT || orient == MATE_PANEL_APPLET_ORIENT_RIGHT)
		ma->panel_orientation = GTK_ORIENTATION_VERTICAL;

	multiload_refresh(ma);
}
static void
mate_size_cb (MatePanelApplet *applet, guint size, MultiloadPlugin *ma)
{
	multiload_refresh(ma);
}

static const GtkActionEntry multiload_menu_actions [] = {
	{ "MultiLoadRunSysmon", "utilities-system-monitor", N_("Start task manager"),
			NULL, NULL, G_CALLBACK (mate_sysmon_cb) },
	{ "MultiLoadProperties", GTK_STOCK_PROPERTIES, N_("_Preferences"),
			NULL, NULL, G_CALLBACK (mate_properties_cb) },
	{ "MultiLoadHelp", GTK_STOCK_HELP, N_("_Help"),
			NULL, NULL, G_CALLBACK (mate_help_cb) },
	{ "MultiLoadAbout", GTK_STOCK_ABOUT, N_("_About"),
			NULL, NULL, G_CALLBACK (mate_about_cb) }
};


static gboolean
mate_constructor (MatePanelApplet* applet, const char* iid, gpointer data)
{
	if (g_strcmp0(iid, "MultiloadNandhpApplet"))
		return FALSE;


	MultiloadPlugin *multiload = g_slice_new0 (MultiloadPlugin);

	mate_panel_applet_set_flags (applet, MATE_PANEL_APPLET_EXPAND_MINOR | MATE_PANEL_APPLET_HAS_HANDLE);
	mate_panel_applet_set_background_widget(applet, GTK_WIDGET(applet));
	multiload_init ();

	multiload->panel_data = mate_panel_applet_settings_new (applet, "org.mate.panel.applet.multiload-nandhp");

	multiload->container = GTK_CONTAINER(gtk_event_box_new ());
	gtk_container_add (GTK_CONTAINER(applet), GTK_WIDGET(multiload->container));
	gtk_widget_show (GTK_WIDGET(applet));
	gtk_widget_show (GTK_WIDGET(multiload->container));

	multiload_ui_read (multiload);
	multiload_refresh(multiload);


	/* menu items */
	GtkActionGroup *action_group = gtk_action_group_new ("Multiload Applet Actions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, multiload_menu_actions, G_N_ELEMENTS (multiload_menu_actions), multiload);

	gchar *ui_path = g_build_filename (MULTILOAD_MENU_UI_DIR, "multiload-applet-menu.xml", NULL);
	mate_panel_applet_setup_menu_from_file (applet, ui_path, action_group);
	g_free (ui_path);


	/* plugin signals */
	g_signal_connect (G_OBJECT (applet), "change-orient",
						G_CALLBACK (mate_orientation_cb), multiload);
	g_signal_connect (G_OBJECT (applet), "change-size",
						G_CALLBACK (mate_size_cb), multiload);

	return TRUE;
}

MATE_PANEL_APPLET_OUT_PROCESS_FACTORY("MultiloadNandhpFactory",
									  PANEL_TYPE_APPLET,
									  about_data_description,
									  mate_constructor,
									  NULL)
