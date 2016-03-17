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
	int tmp_int;
	const char *tmp_str;

	multiload_defaults(ma);

	g_assert_nonnull(settings);
	g_assert_nonnull(ma);

	if (config_setting_lookup_int(settings, "speed", &tmp_int))
		ma->speed = tmp_int;
	if (config_setting_lookup_int(settings, "size", &tmp_int))
		ma->size = tmp_int;
	if (config_setting_lookup_int(settings, "padding", &tmp_int))
		ma->padding = tmp_int;
	if (config_setting_lookup_int(settings, "spacing", &tmp_int))
		ma->spacing = tmp_int;
	if (config_setting_lookup_int(settings, "orientation", &tmp_int))
		ma->orientation_policy = tmp_int;
	if (config_setting_lookup_int(settings, "fill-between", &tmp_int))
		ma->fill_between = tmp_int? TRUE:FALSE;

	for ( i = 0; i < GRAPH_MAX; i++ ) {
		key = g_strdup_printf("%s_visible", graph_types[i].name);
		if (config_setting_lookup_int(settings, key, &tmp_int))
			ma->graph_config[i].visible = tmp_int? TRUE:FALSE;
		g_free (key);

		key = g_strdup_printf("%s_border-width", graph_types[i].name);
		if (config_setting_lookup_int(settings, key, &tmp_int))
			ma->graph_config[i].border_width = tmp_int;
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
	guint i;

	config_group_set_int(multiload->settings, "speed", ma->speed);
	config_group_set_int(multiload->settings, "size", ma->size);
	config_group_set_int(multiload->settings, "padding", ma->padding);
	config_group_set_int(multiload->settings, "spacing", ma->spacing);
	config_group_set_int(multiload->settings, "orientation", ma->orientation_policy);
	config_group_set_int(multiload->settings, "fill-between", ma->fill_between);

	for ( i = 0; i < GRAPH_MAX; i++ ) {
		char *key, list[10*MAX_COLORS];

		key = g_strdup_printf("%s_visible", graph_types[i].name);
		config_group_set_int(multiload->settings, key, ma->graph_config[i].visible);
		g_free (key);

		key = g_strdup_printf("%s_border-width", graph_types[i].name);
		config_group_set_int(multiload->settings, key, ma->graph_config[i].border_width);
		g_free (key);

		key = g_strdup_printf("%s_colors", graph_types[i].name);
		multiload_colors_stringify (ma, i, list);
		config_group_set_string(multiload->settings, key, list);
		g_free (key);
	}
}



gboolean
multiload_press_event(GtkWidget *ebox, GdkEventButton *event, LXPanel *panel)
{
	return TRUE;
}

void
multiload_configure_response (GtkWidget *dialog, gint response, MultiloadLxpanelPlugin *multiload)
{
	gboolean result;

	if (response == GTK_RESPONSE_HELP) {
		/* show help */
		/* FIXME: Not all common versions of xdg-open support lxde -2012-06-25 */
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

GtkWidget* multiload_configure(LXPanel *panel, GtkWidget *ebox)
{
	MultiloadLxpanelPlugin *multiload = lxpanel_plugin_get_data(ebox);

	if ( multiload->dlg != NULL ) {
		gtk_widget_show_all (multiload->dlg);
		gtk_window_present (GTK_WINDOW (multiload->dlg));
		return GTK_WIDGET(multiload->dlg);
	}

	/* create the dialog */
	multiload->dlg = gtk_dialog_new_with_buttons(_("Multiload"),
					GTK_WINDOW(ebox),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_STOCK_HELP, GTK_RESPONSE_HELP,
					GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
					NULL);
	multiload->dlg = multiload->dlg;

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

//	/* show the entire dialog */
//	gtk_widget_show (multiload->dlg);
	return GTK_WIDGET(multiload->dlg);
}

void multiload_configuration_changed(LXPanel *panel, GtkWidget *ebox)
{
	MultiloadLxpanelPlugin *multiload = lxpanel_plugin_get_data(ebox);

	/* Determine orientation and size */
	if ( panel_get_orientation(panel) == GTK_ORIENTATION_VERTICAL ) {
		multiload->ma.panel_orientation = GTK_ORIENTATION_VERTICAL;
		gtk_widget_set_size_request (ebox, panel_get_height(panel), -1); //TODO è lo stesso di width?
	} else { //multiload->ma->panel_orientation can have values other than vert/horiz
		multiload->ma.panel_orientation = GTK_ORIENTATION_HORIZONTAL;
		gtk_widget_set_size_request (ebox, -1, panel_get_height(panel));
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
multiload_constructor(LXPanel *panel, config_setting_t *settings) //char **fp)
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

	/* Initialize the applet */
	/* Set size request and update orientation */
	multiload_panel_configuration_changed(panel, GTK_WIDGET(multiload->ma.container));

	return GTK_WIDGET(multiload->ma.container);
}


/* Lookup the MultiloadPlugin object from the preferences dialog. */
/* Called from multiload/properties.c */
MultiloadPlugin *
multiload_configure_get_plugin (GtkWidget *widget)
{
	GtkWidget *toplevel = gtk_widget_get_toplevel (widget);
	MultiloadPlugin *ma = NULL;
	if ( G_LIKELY (gtk_widget_is_toplevel (toplevel)) )
		ma = g_object_get_data(G_OBJECT(toplevel), "MultiloadPlugin");
	else
		g_assert_not_reached ();
	g_assert_nonnull(ma);
	return ma;
}

FM_DEFINE_MODULE(lxpanel_gtk, multiload)

/* Plugin descriptor. */
LXPanelPluginInit fm_module_init_lxpanel_gtk = {
	.name = N_("Multiload"),
	.description = N_("A system load monitor that graphs processor, memory, "
					"and swap space use, plus network and disk activity."),

	.new_instance = multiload_constructor,
	.config = multiload_configure,
	// these fields are not implemented in lxpanel>0.7
	//.type = "multiload",
	//.version = PACKAGE_VERSION,
	.reconfigure = multiload_configuration_changed,
	.one_per_system = FALSE,
	.expand_available = FALSE
//	.button_press_event = multiload_press_event
};

