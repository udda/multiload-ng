/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *               2012 nandhp <nandhp@gmail.com>
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
#include <string.h>

#ifdef HAVE_XFCE4UI
#include <libxfce4ui/libxfce4ui.h>
#elif HAVE_XFCEGUI4
#include <libxfcegui4/libxfcegui4.h>
#else
#error Must have one of libxfce4ui or xfcegui4
#endif
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include "common/about-data.h"

#include "common/multiload.h"
#include "common/multiload-config.h"
#include "common/multiload-colors.h"
#include "common/properties.h"


typedef struct {
	MultiloadPlugin ma;
	GtkWidget       *ebox;
} MultiloadXfcePlugin; //TODO remove this struct, and other in panel-specific files

gpointer multiload_settings_open_for_read(MultiloadPlugin *ma) {
	gchar *file;
	XfceRc *rc;
	file = xfce_panel_plugin_lookup_rc_file((XfcePanelPlugin*)(ma->panel_plugin));
	if (G_UNLIKELY(file == NULL))
		return NULL;

	rc = xfce_rc_simple_open (file, TRUE);
	g_free(file);

	return (gpointer)rc;
}
gpointer multiload_settings_open_for_save(MultiloadPlugin *ma) {
	gchar *file;
	XfceRc *rc;

	file = xfce_panel_plugin_save_location((XfcePanelPlugin*)(ma->panel_plugin), TRUE);
	if (G_UNLIKELY(file == NULL))
		return NULL;

	rc = xfce_rc_simple_open (file, FALSE);
	g_free(file);

	return (gpointer)rc;
}

void multiload_settings_close(gpointer settings) {
	xfce_rc_close((XfceRc*)settings);
}

void multiload_settings_get_int(gpointer settings, const gchar *key, int *destination) {
	*destination = xfce_rc_read_int_entry((XfceRc*)settings, key, *destination);
}
void multiload_settings_get_boolean(gpointer settings, const gchar *key, gboolean *destination) {
	*destination = xfce_rc_read_bool_entry((XfceRc*)settings, key, *destination);
}
void multiload_settings_get_string(gpointer settings, const gchar *key, gchar *destination, size_t maxlen) {
	const gchar* temp = xfce_rc_read_entry((XfceRc*)settings, key, NULL);
	if (G_LIKELY(temp != NULL))
		strncpy(destination, temp, maxlen);
}

void multiload_settings_set_int(gpointer settings, const gchar *key, int value) {
	xfce_rc_write_int_entry((XfceRc*)settings, key, value);
}
void multiload_settings_set_boolean(gpointer settings, const gchar *key, gboolean value) {
	xfce_rc_write_bool_entry((XfceRc*)settings, key, value);
}
void multiload_settings_set_string(gpointer settings, const gchar *key, const gchar *value) {
	xfce_rc_write_entry((XfceRc*)settings, key, value);
}



void
multiload_read (MultiloadPlugin *ma)
{
	gpointer *settings;
	gchar *key;
	gchar colors_list[10*MAX_COLORS];
	int i;

	multiload_defaults(ma);

	settings = multiload_settings_open_for_read(ma);
	if (G_LIKELY (settings != NULL)) {
		multiload_settings_get_int		(settings, "speed",				&ma->speed);
		multiload_settings_get_int		(settings, "size",				&ma->size);
		multiload_settings_get_int		(settings, "padding",			&ma->padding);
		multiload_settings_get_int		(settings, "spacing",			&ma->spacing);
		multiload_settings_get_int		(settings, "orientation",		&ma->orientation_policy);
		multiload_settings_get_boolean	(settings, "fill-between",		&ma->fill_between);
		multiload_settings_get_boolean	(settings, "tooltip-details",	&ma->tooltip_details);

		multiload_settings_get_int		(settings, "dblclick-policy",	&ma->dblclick_policy);
		multiload_settings_get_string	(settings, "dblclick-cmdline",	ma->dblclick_cmdline, sizeof(ma->dblclick_cmdline)/sizeof(gchar));

		for ( i = 0; i < GRAPH_MAX; i++ ) {
			/* Visibility */
			key = g_strdup_printf("graph-%s-visible", graph_types[i].name);
			multiload_settings_get_boolean (settings, key, &ma->graph_config[i].visible);
			g_free (key);

			/* Border width */
			key = g_strdup_printf("graph-%s-border-width", graph_types[i].name);
			multiload_settings_get_int (settings, key, &ma->graph_config[i].border_width);
			g_free (key);

			/* Colors */
			key = g_strdup_printf("graph-%s-colors", graph_types[i].name);
			colors_list[0] = 0;
			multiload_settings_get_string (settings, key, colors_list, sizeof(colors_list)/sizeof(gchar));
			multiload_colors_unstringify(ma, i, colors_list);
			g_free (key);
		}
		multiload_settings_close(settings);

		multiload_sanitize(ma);
		return;
	}
}

void
multiload_save (MultiloadPlugin *ma)
{
	gpointer *settings;
	char *key;
	gchar colors_list[10*MAX_COLORS];
	int i;

	settings = multiload_settings_open_for_save(ma);
	if (G_LIKELY (settings != NULL)) {
		multiload_settings_set_int		(settings, "speed",				ma->speed);
		multiload_settings_set_int		(settings, "size",				ma->size);
		multiload_settings_set_int		(settings, "padding",			ma->padding);
		multiload_settings_set_int		(settings, "spacing",			ma->spacing);
		multiload_settings_set_int		(settings, "orientation",		ma->orientation_policy);
		multiload_settings_set_boolean	(settings, "fill-between",		ma->fill_between);
		multiload_settings_set_boolean	(settings, "tooltip-details",	ma->tooltip_details);

		multiload_settings_set_int		(settings, "dblclick-policy",	ma->dblclick_policy);
		multiload_settings_set_string	(settings, "dblclick-cmdline",	ma->dblclick_cmdline);

		for ( i = 0; i < GRAPH_MAX; i++ ) {
			/* Visibility */
			key = g_strdup_printf("graph-%s-visible", graph_types[i].name);
			multiload_settings_set_boolean (settings, key, ma->graph_config[i].visible);
			g_free (key);

			/* Border width */
			key = g_strdup_printf("graph-%s-border-width", graph_types[i].name);
			multiload_settings_set_int (settings, key, ma->graph_config[i].border_width);
			g_free (key);

			/* Colors */
			key = g_strdup_printf("graph-%s-colors", graph_types[i].name);
			multiload_colors_stringify (ma, i, colors_list);
			multiload_settings_set_string (settings, key, colors_list);
			g_free (key);
		}
		multiload_settings_close(settings);
	}
}

static void
multiload_configure_response (GtkWidget *dialog, gint response, MultiloadXfcePlugin *multiload)
{
	gboolean result;
	gchar *cmdline;

	if (response == GTK_RESPONSE_HELP) {
		cmdline = g_strdup_printf("xdg-open --launch WebBrowser %s", about_data_website);
		result = g_spawn_command_line_async (cmdline, NULL);
		g_free(cmdline);

		if (G_UNLIKELY (result == FALSE))
			g_warning (_("Unable to open the following url: %s"), about_data_website);
	} else {
		/* remove the dialog data from the plugin */
		g_object_set_data (G_OBJECT (multiload->ma.panel_plugin), "dialog", NULL);

		/* unlock the panel menu */
		xfce_panel_plugin_unblock_menu ((XfcePanelPlugin*)(multiload->ma.panel_plugin));

		/* save the plugin */
		multiload_save (&multiload->ma);

		/* destroy the properties dialog */
		gtk_widget_destroy (dialog);
	}
}

void
multiload_configure (XfcePanelPlugin *plugin, MultiloadXfcePlugin *multiload)
{
	GtkWidget *dialog;

	/* block the plugin menu */
	xfce_panel_plugin_block_menu (plugin);

	/* create the dialog */
	dialog = xfce_titled_dialog_new_with_buttons(_("Multiload"),
					GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_STOCK_HELP, GTK_RESPONSE_HELP,
					GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
					NULL);

	/* center dialog on the screen */
	gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

	/* set dialog icon */
	gtk_window_set_icon_name (GTK_WINDOW (dialog), "utilities-system-monitor");

	/* link the dialog to the plugin, so we can destroy it when the plugin
	* is closed, but the dialog is still open */
	g_object_set_data (G_OBJECT (plugin), "dialog", dialog);
	g_object_set_data (G_OBJECT (dialog), "MultiloadPlugin", &multiload->ma);

	/* Initialize dialog widgets */
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	multiload_init_preferences(dialog, &multiload->ma);

	/* connect the reponse signal to the dialog */
	g_signal_connect (G_OBJECT (dialog), "response",
					G_CALLBACK(multiload_configure_response), multiload);

	/* show the entire dialog */
	gtk_widget_show (dialog);
}

static void
multiload_orientation_changed (XfcePanelPlugin *plugin, GtkOrientation orientation, MultiloadPlugin *ma)
{
	/* Get the plugin's current size request */
	gint size = -1, size_alt = -1;
	gtk_widget_get_size_request (GTK_WIDGET (plugin), &size, &size_alt);
	if ( size < 0 )
		size = size_alt;

	ma->panel_orientation = orientation;

	/* Rotate the plugin size to the new orientation */
	if ( orientation == GTK_ORIENTATION_HORIZONTAL)
		gtk_widget_set_size_request (GTK_WIDGET (plugin), -1, size);
	else
		gtk_widget_set_size_request (GTK_WIDGET (plugin), size, -1);

	multiload_refresh(ma);
}

static gboolean
multiload_size_changed (XfcePanelPlugin *plugin, gint size, MultiloadPlugin *ma)
{
	/* set the widget size */
	if ( ma->panel_orientation == GTK_ORIENTATION_HORIZONTAL)
		gtk_widget_set_size_request (GTK_WIDGET (plugin), -1, size);
	else
		gtk_widget_set_size_request (GTK_WIDGET (plugin), size, -1);

	multiload_refresh(ma);

	return TRUE;
}


void
multiload_about (XfcePanelPlugin *plugin)
{
	gtk_show_about_dialog(NULL,
	"logo-icon-name",	"utilities-system-monitor",
	"program-name",		about_data_progname,
	"version",			PACKAGE_VERSION,
	"comments",			about_data_description,
	"website",			about_data_website,
	"copyright",		about_data_copyright,
	"license",			about_data_license,
	"authors",			about_data_authors,
	"translator-credits", _("translator-credits"),
	NULL);
}


static MultiloadXfcePlugin *
multiload_new (XfcePanelPlugin *plugin)
{
	MultiloadXfcePlugin *multiload = panel_slice_new0 (MultiloadXfcePlugin);

	/* pointer to plugin */
	multiload->ma.panel_plugin = plugin;

	/* read the user settings */
	multiload_read (&multiload->ma);

	/* create a container widget */
	multiload->ebox = gtk_event_box_new ();
	gtk_widget_show (multiload->ebox);

	/* get the current orientation */
	multiload->ma.panel_orientation = xfce_panel_plugin_get_orientation (plugin);

	/* Initialize the applet */
	multiload->ma.container = GTK_CONTAINER(multiload->ebox);
	multiload_refresh(&(multiload->ma));

	return multiload;
}

static void
multiload_save_cb (XfcePanelPlugin *plugin, MultiloadPlugin *multiload)
{
	multiload_save(multiload);
}

static void
multiload_free (XfcePanelPlugin *plugin, MultiloadXfcePlugin *multiload)
{
	GtkWidget *dialog;

	/* check if the dialog is still open. if so, destroy it */
	dialog = g_object_get_data (G_OBJECT (plugin), "dialog");
	if (G_UNLIKELY (dialog != NULL))
		gtk_widget_destroy (dialog);

	multiload_destroy (&multiload->ma);

	/* destroy the panel widgets */
	gtk_widget_destroy (multiload->ebox);

	/* free the plugin structure */
	panel_slice_free (MultiloadXfcePlugin, multiload);
}


static void
multiload_construct (XfcePanelPlugin *plugin)
{
	MultiloadXfcePlugin *multiload;

	/* Initialize multiload */
	multiload_init ();

	/* setup transation domain */
	xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

	/* create the plugin */
	multiload = multiload_new (plugin);

	/* add the ebox to the panel */
	gtk_container_add (GTK_CONTAINER (plugin), multiload->ebox);

	/* show the panel's right-click menu on this ebox */
	xfce_panel_plugin_add_action_widget (plugin, multiload->ebox);

	/* connect plugin signals */
	g_signal_connect (G_OBJECT (plugin), "free-data",
						G_CALLBACK (multiload_free), multiload);

	g_signal_connect (G_OBJECT (plugin), "save",
						G_CALLBACK (multiload_save_cb), &multiload->ma);

	g_signal_connect (G_OBJECT (plugin), "size-changed",
						G_CALLBACK (multiload_size_changed), &multiload->ma);

	g_signal_connect (G_OBJECT (plugin), "orientation-changed",
						G_CALLBACK (multiload_orientation_changed), &multiload->ma);

	/* show the configure menu item and connect signal */
	xfce_panel_plugin_menu_show_configure (plugin);
	g_signal_connect (G_OBJECT (plugin), "configure-plugin",
						G_CALLBACK (multiload_configure), multiload);

	/* show the about menu item and connect signal */
	xfce_panel_plugin_menu_show_about (plugin);
	g_signal_connect (G_OBJECT (plugin), "about",
						G_CALLBACK (multiload_about), NULL);
}

/* register the plugin */
#ifdef XFCE_PANEL_PLUGIN_REGISTER
XFCE_PANEL_PLUGIN_REGISTER (multiload_construct);           /* Xfce 4.8 */
#else
XFCE_PANEL_PLUGIN_REGISTER_INTERNAL (multiload_construct);  /* Xfce 4.6 */
#endif
