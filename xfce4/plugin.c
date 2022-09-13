/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *               2012 nandhp <nandhp@gmail.com>
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

#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4panel/xfce-panel-macros.h>

#include "common/multiload.h"
#include "common/ui.h"


gpointer
multiload_ps_settings_open_for_read(MultiloadPlugin *ma)
{
	gchar *file;
	XfceRc *rc;
	file = xfce_panel_plugin_lookup_rc_file((XfcePanelPlugin*)(ma->panel_data));
	if (G_UNLIKELY(file == NULL))
		return NULL;

	rc = xfce_rc_simple_open (file, TRUE);
	g_free(file);

	return (gpointer)rc;
}
gpointer
multiload_ps_settings_open_for_save(MultiloadPlugin *ma)
{
	gchar *file;
	XfceRc *rc;

	file = xfce_panel_plugin_save_location((XfcePanelPlugin*)(ma->panel_data), TRUE);
	if (G_UNLIKELY(file == NULL))
		return NULL;

	rc = xfce_rc_simple_open (file, FALSE);
	g_free(file);

	return (gpointer)rc;
}

gboolean
multiload_ps_settings_save(gpointer settings)
{
	return TRUE;
}

void
multiload_ps_settings_close(gpointer settings)
{
	xfce_rc_close((XfceRc*)settings);
}

void
multiload_ps_settings_get_int(gpointer settings, const gchar *key, int *destination)
{
	*destination = xfce_rc_read_int_entry((XfceRc*)settings, key, *destination);
}
void
multiload_ps_settings_get_boolean(gpointer settings, const gchar *key, gboolean *destination)
{
	*destination = xfce_rc_read_bool_entry((XfceRc*)settings, key, *destination);
}
void
multiload_ps_settings_get_string(gpointer settings, const gchar *key, gchar *destination, size_t maxlen)
{
	const gchar* temp = xfce_rc_read_entry((XfceRc*)settings, key, NULL);
	if (G_LIKELY(temp != NULL))
		strncpy(destination, temp, maxlen);
}

void
multiload_ps_settings_set_int(gpointer settings, const gchar *key, int value)
{
	xfce_rc_write_int_entry((XfceRc*)settings, key, value);
}
void
multiload_ps_settings_set_boolean(gpointer settings, const gchar *key, gboolean value)
{
	xfce_rc_write_bool_entry((XfceRc*)settings, key, value);
}
void
multiload_ps_settings_set_string(gpointer settings, const gchar *key, const gchar *value)
{
	xfce_rc_write_entry((XfceRc*)settings, key, value);
}

void
multiload_ps_preferences_closed_cb(MultiloadPlugin *ma)
{
	xfce_panel_plugin_unblock_menu ((XfcePanelPlugin*)(ma->panel_data));
}



void
xfce_configure_cb (XfcePanelPlugin *plugin, MultiloadPlugin *ma)
{
	xfce_panel_plugin_block_menu (plugin);
	GtkWidget *dialog = multiload_ui_configure_dialog_new(ma,
		GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))));
	gtk_window_present(GTK_WINDOW(dialog));
}

void
xfce_about_cb (XfcePanelPlugin *plugin)
{
	multiload_ui_show_about(GTK_WINDOW(gtk_widget_get_toplevel (GTK_WIDGET(plugin))));
}

void
xfce_save_cb (XfcePanelPlugin *plugin, MultiloadPlugin *multiload)
{
	multiload_ui_save(multiload);
}

void
xfce_free_cb (XfcePanelPlugin *plugin, MultiloadPlugin *multiload)
{
	if (G_UNLIKELY (multiload->pref_dialog != NULL))
		gtk_widget_destroy (multiload->pref_dialog);

	multiload_free (multiload);
	gtk_widget_destroy (GTK_WIDGET(multiload->container));

	g_slice_free (MultiloadPlugin, multiload);
}

static gboolean
xfce_size_changed_cb (XfcePanelPlugin *plugin, int size, MultiloadPlugin *ma)
{
	return TRUE; // this is necessary
}
static void
xfce_orientation_changed_cb (XfcePanelPlugin *plugin, GtkOrientation orientation, MultiloadPlugin *ma)
{
	ma->panel_orientation = orientation;
	multiload_refresh_orientation(ma);
}

static void
xfce_help_cb(GtkMenuItem *mi, gpointer data)
{
	multiload_ui_show_help();
}
static void
xfce_sysmon_cb(GtkMenuItem *mi, MultiloadPlugin *ma)
{
	multiload_ui_start_system_monitor(ma);
}

static void
xfce_constructor (XfcePanelPlugin *plugin)
{
	MultiloadPlugin *multiload = multiload_new();

	multiload->panel_data = plugin;
	multiload->panel_orientation = xfce_panel_plugin_get_orientation (plugin);
	gtk_container_add (GTK_CONTAINER (plugin), GTK_WIDGET(multiload->container));

	multiload_ui_read (multiload);
	multiload_start(multiload);


	/* show the panel's right-click menu on this ebox */
	xfce_panel_plugin_add_action_widget (plugin, GTK_WIDGET(multiload->container));


	/* plugin signals */
	g_signal_connect(G_OBJECT(plugin), "free-data",           G_CALLBACK (xfce_free_cb),                multiload);
	g_signal_connect(G_OBJECT(plugin), "save",                G_CALLBACK (xfce_save_cb),                multiload);
	g_signal_connect(G_OBJECT(plugin), "size-changed",        G_CALLBACK (xfce_size_changed_cb),        multiload);
	g_signal_connect(G_OBJECT(plugin), "orientation-changed", G_CALLBACK (xfce_orientation_changed_cb), multiload);

	/* menu items */
	GtkMenuItem *mi_separator = GTK_MENU_ITEM(gtk_separator_menu_item_new ());
	GtkMenuItem *mi_help = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic (_("_Help")));
	GtkMenuItem *mi_sysmon = GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Start task manager")));
	xfce_panel_plugin_menu_show_configure (plugin);
	xfce_panel_plugin_menu_show_about (plugin);
	xfce_panel_plugin_menu_insert_item (plugin, mi_separator);
	xfce_panel_plugin_menu_insert_item (plugin, mi_help);
	xfce_panel_plugin_menu_insert_item (plugin, mi_sysmon);
	g_signal_connect (G_OBJECT (plugin), "configure-plugin",	G_CALLBACK (xfce_configure_cb), multiload);
	g_signal_connect (G_OBJECT (plugin), "about",				G_CALLBACK (xfce_about_cb), NULL);
	g_signal_connect (G_OBJECT (mi_help), "activate",			G_CALLBACK (xfce_help_cb), NULL);
	g_signal_connect (G_OBJECT (mi_sysmon), "activate",			G_CALLBACK (xfce_sysmon_cb), multiload);
	gtk_widget_show(GTK_WIDGET(mi_separator));
	gtk_widget_show(GTK_WIDGET(mi_help));
	gtk_widget_show(GTK_WIDGET(mi_sysmon));
}

/* register the plugin */
#ifdef XFCE_PANEL_PLUGIN_REGISTER
XFCE_PANEL_PLUGIN_REGISTER (xfce_constructor);           /* Xfce 4.8 */
#else
XFCE_PANEL_PLUGIN_REGISTER_INTERNAL (xfce_constructor);  /* Xfce 4.6 */
#endif
