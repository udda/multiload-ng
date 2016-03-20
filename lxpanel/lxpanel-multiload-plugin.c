#include <config.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gi18n-lib.h>

#include <lxpanel/plugin.h>

#include "multiload/multiload.h"
#include "multiload/multiload-colors.h"
#include "multiload/multiload-config.h"
#include "multiload/properties.h"


typedef struct  {
	LXPanel * panel;
	config_setting_t *settings;
	MultiloadPlugin ma;
	GtkWidget *dlg;
} MultiloadLxpanelPlugin;



void
multiload_read(config_setting_t *settings, MultiloadPlugin *ma)
{
	guint i;
	gchar *key;
	const char *tmp_str;

	multiload_defaults(ma);

	g_assert_nonnull(settings);
	g_assert_nonnull(ma);
	config_setting_lookup_int(settings, "speed", &ma->speed);
	config_setting_lookup_int(settings, "size", &ma->size);
	config_setting_lookup_int(settings, "padding", &ma->padding);
	config_setting_lookup_int(settings, "spacing", &ma->spacing);
	config_setting_lookup_int(settings, "orientation", &ma->orientation_policy);
	config_setting_lookup_int(settings, "fill-between", &ma->fill_between);
	config_setting_lookup_int(settings, "tooltip-details", &ma->tooltip_details);
	config_setting_lookup_int(settings, "dblclick-policy", &ma->dblclick_policy);
	if (config_setting_lookup_string(settings, "dblclick-cmdline", &tmp_str))
		strncpy(ma->dblclick_cmdline, tmp_str, sizeof(ma->dblclick_cmdline)/sizeof(gchar));

	for ( i=0; i<GRAPH_MAX; i++ ) {
		key = g_strdup_printf("%s_visible", graph_types[i].name);
		config_setting_lookup_int(settings, key, &ma->graph_config[i].visible);
		g_free (key);

		key = g_strdup_printf("%s_border-width", graph_types[i].name);
		config_setting_lookup_int(settings, key, &ma->graph_config[i].border_width);
		g_free (key);

		key = g_strdup_printf("%s_colors", graph_types[i].name);
		if (config_setting_lookup_string(settings, key, &tmp_str))
			multiload_colors_unstringify(ma, i, tmp_str);
		g_free (key);
	}

	multiload_sanitize(ma);
}


void
multiload_save(gpointer user_data)
{
	MultiloadLxpanelPlugin *multiload = (MultiloadLxpanelPlugin*)user_data;
	MultiloadPlugin *ma = &multiload->ma;
	config_setting_t *s = multiload->settings;

	guint i;
	char *key;
	char list[10*MAX_COLORS];

	g_assert_nonnull(s);

	config_group_set_int	(s, "speed",			ma->speed);
	config_group_set_int	(s, "size",				ma->size);
	config_group_set_int	(s, "padding",			ma->padding);
	config_group_set_int	(s, "spacing",			ma->spacing);
	config_group_set_int	(s, "orientation",		ma->orientation_policy);
	config_group_set_int	(s, "fill-between",		ma->fill_between);
	config_group_set_int	(s, "tooltip-details",	ma->tooltip_details);
	config_group_set_int	(s, "dblclick-policy",	ma->dblclick_policy);
	config_group_set_string	(s, "dblclick-cmdline",	ma->dblclick_cmdline);

	for ( i = 0; i < GRAPH_MAX; i++ ) {
		key = g_strdup_printf("%s_visible",			graph_types[i].name);
		config_group_set_int(s, key, ma->graph_config[i].visible);
		g_free (key);

		key = g_strdup_printf("%s_border-width",	graph_types[i].name);
		config_group_set_int(s, key, ma->graph_config[i].border_width);
		g_free (key);

		multiload_colors_stringify (ma, i, list);
		key = g_strdup_printf("%s_colors",			graph_types[i].name);
		config_group_set_string(s, key, list);
		g_free (key);
	}
}


void
multiload_configure_response (GtkWidget *dialog, gint response, MultiloadLxpanelPlugin *multiload)
{
	gboolean result;

	if (response == GTK_RESPONSE_HELP) {
		/* show help */
		result = g_spawn_command_line_async ("xdg-open --launch WebBrowser "
						PLUGIN_WEBSITE, NULL);

		if (G_UNLIKELY (result == FALSE))
			g_warning (_("Unable to open the following url: %s"), PLUGIN_WEBSITE);
	} else {
		/* destroy the properties dialog */
		multiload_save(multiload);
		gtk_widget_destroy (multiload->dlg);
		multiload->dlg = NULL;
	}
}

GtkWidget*
multiload_configure(LXPanel *panel, GtkWidget *ebox)
{
	MultiloadLxpanelPlugin *multiload = lxpanel_plugin_get_data(ebox);

	if ( multiload->dlg != NULL ) {
		gtk_widget_show_all (multiload->dlg);
		gtk_window_present (GTK_WINDOW (multiload->dlg));
		return GTK_WIDGET(multiload->dlg);
	}

	/* create the dialog */
	multiload->dlg = gtk_dialog_new_with_buttons(_("Multiload"),
					GTK_WINDOW(gtk_widget_get_toplevel(ebox)),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_STOCK_HELP, GTK_RESPONSE_HELP,
					GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
					NULL);

	/* center dialog on the screen */
	gtk_window_set_position (GTK_WINDOW (multiload->dlg), GTK_WIN_POS_CENTER);

	/* set dialog icon */
	gtk_window_set_icon_name (GTK_WINDOW (multiload->dlg), "utilities-system-monitor");

	/* link the dialog to the plugin, so we can destroy it when the plugin
	* is closed, but the dialog is still open */
	g_object_set_data (G_OBJECT (multiload->dlg), "MultiloadPlugin", &multiload->ma);

	/* Initialize dialog widgets */
	gtk_dialog_set_default_response (GTK_DIALOG (multiload->dlg), GTK_RESPONSE_OK);
	gtk_window_set_resizable (GTK_WINDOW (multiload->dlg), FALSE);
	multiload_init_preferences(multiload->dlg, &multiload->ma);

	/* connect the reponse signal to the dialog */
	g_signal_connect (G_OBJECT (multiload->dlg), "response",
					G_CALLBACK(multiload_configure_response), multiload);

	return GTK_WIDGET(multiload->dlg);
}

void
multiload_configuration_changed(LXPanel *panel, GtkWidget *ebox)
{
	MultiloadLxpanelPlugin *multiload = lxpanel_plugin_get_data(ebox);

	// despite the name, in vertical orientation this is panel width
	int size = panel_get_height(panel);

	/* Determine orientation and size */
	if ( panel_get_orientation(panel) == GTK_ORIENTATION_VERTICAL ) {
		multiload->ma.panel_orientation = GTK_ORIENTATION_VERTICAL;
		gtk_widget_set_size_request (ebox, size, -1);
	} else { /* ma->panel_orientation can have values other than vert/horiz */
		multiload->ma.panel_orientation = GTK_ORIENTATION_HORIZONTAL;
		gtk_widget_set_size_request (ebox, -1, size);
	}

	/* Refresh the panel applet */
	multiload_refresh(&(multiload->ma));
}


void
multiload_destructor(gpointer user_data)
{
	MultiloadLxpanelPlugin *multiload = lxpanel_plugin_get_data(user_data);
	gtk_widget_destroy (GTK_WIDGET(user_data));
	g_free(multiload);
}



GtkWidget*
multiload_constructor(LXPanel *panel, config_setting_t *settings)
{
	/* allocate our private structure instance */
	MultiloadLxpanelPlugin *multiload = g_new0(MultiloadLxpanelPlugin, 1);

	multiload->panel = panel;
	multiload->settings = settings;

	/* Initialize multiload */
	multiload_init ();
	/* read the user settings */
	multiload_read (settings, &multiload->ma);


	/* create a container widget */
	multiload->ma.container = GTK_CONTAINER(gtk_event_box_new ());
	lxpanel_plugin_set_data(multiload->ma.container, multiload, multiload_destructor);
	gtk_widget_show (GTK_WIDGET(multiload->ma.container));

	/* Set size request and update orientation */
	multiload_configuration_changed(panel, GTK_WIDGET(multiload->ma.container));

	return GTK_WIDGET(multiload->ma.container);
}


FM_DEFINE_MODULE(lxpanel_gtk, multiload)

/* Plugin descriptor. */
LXPanelPluginInit fm_module_init_lxpanel_gtk = {
	.name = N_("Multiload"),
	.description = N_("A system load monitor that graphs processor, memory, "
					"and swap space use, plus network and disk activity."),

	.new_instance = multiload_constructor,
	.config = multiload_configure,
	.reconfigure = multiload_configuration_changed,
	.one_per_system = FALSE,
	.expand_available = FALSE
};
