#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gi18n-lib.h>

#include <lxpanel/plugin.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "multiload/multiload.h"
#include "multiload/properties.h"

/* FIXME: DIRTY HACKS! The following lxpanel functions are NOT in
  the /usr/include/lxpanel/plugin.h header file! For a usage example,
  see lxpanel/src/plugins/launchbar.c
 */
/* Handle-right-click events. Defined in lxpanel/src/panel.h. */
extern gboolean plugin_button_press_event(GtkWidget *widget, GdkEventButton *event, Plugin *plugin);
/* Trigger a save event. Defined only in lxpanel/src/configurator.c! */ 
void panel_config_save(Panel * panel);  /* defined in  */

/** BEGIN H **/
typedef struct _MultiloadLxpanelPlugin MultiloadLxpanelPlugin;

/* an instance of this struct is what will be assigned to 'priv' */
struct _MultiloadLxpanelPlugin {
	MultiloadPlugin ma;
	GtkWidget *dlg;
};
/** END H **/

static void
multiload_read(char **fp, MultiloadPlugin *ma)
{
	guint i, found = 0;

	/* Initial settings */
	ma->speed = 0;
	ma->size = 0;
	for ( i = 0; i < NGRAPHS; i++ ) {
		/* Default visibility and colors */
		ma->graph_config[i].visible = FALSE;
		multiload_colorconfig_default(ma, i);
	}

	if ( fp != NULL ) {
		line s;
		s.len = 1024;
		while ( lxpanel_get_line(fp, &s) != LINE_BLOCK_END ) {
			if ( s.type == LINE_VAR ) {
				if ( g_ascii_strcasecmp(s.t[0], "speed") == 0 ) {
					ma->speed = atoi(s.t[1]);
				} else if ( g_ascii_strcasecmp(s.t[0], "size") == 0 ) {
					ma->size = atoi(s.t[1]);
				} else if ( g_ascii_strcasecmp(s.t[0], "padding") == 0 ) {
					ma->padding = atoi(s.t[1]);
				} else if ( g_ascii_strcasecmp(s.t[0], "spacing") == 0 ) {
					ma->spacing = atoi(s.t[1]);
				} else if ( g_ascii_strcasecmp(s.t[0], "show-frame") == 0 ) {
					ma->show_frame = atoi(s.t[1]);
				} else if ( g_ascii_strcasecmp(s.t[0], "orientation") == 0 ) {
					ma->orientation_policy = atoi(s.t[1]);
				} else {
					const char *suffix; /* Set by multiload_find_graph_by_name */
					int i = multiload_find_graph_by_name(s.t[0], &suffix);

					if ( suffix == NULL || i < 0 || i >= NGRAPHS )
						continue;
					else if ( g_ascii_strcasecmp(suffix, "Visible") == 0 )
						ma->graph_config[i].visible = atoi(s.t[1]) ? TRUE : FALSE;
					else if ( g_ascii_strcasecmp(suffix, "Colors") == 0 )
						multiload_colorconfig_unstringify(ma, i, s.t[1]);
				}
			} else {
				ERR ("Failed to parse config token %s\n", s.str);
				break;
			}
		}
	}

	/* Handle errors from atoi */
	if ( ma->speed == 0 )
		ma->speed = DEFAULT_SPEED;
	if ( ma->size == 0 )
		ma->size = DEFAULT_SIZE;
	if ( ma->padding == 0 )
		ma->padding = DEFAULT_PADDING;
	if ( ma->spacing == 0 )
		ma->padding = DEFAULT_SPACING;
	/* Ensure at lease one graph is visible */
	for ( i = 0; i < NGRAPHS; i++ ) {
		if ( ma->graph_config[i].visible == TRUE )
			found++;
	}
	if ( found == 0 )
		ma->graph_config[0].visible = TRUE;
}

static void multiload_save_configuration(Plugin * p, FILE * fp)
{
	MultiloadLxpanelPlugin *multiload = p->priv;
	MultiloadPlugin *ma = &multiload->ma;
	guint i;

	/* Write size and speed */
	lxpanel_put_int (fp, "speed", ma->speed);
	lxpanel_put_int (fp, "size", ma->size);
	lxpanel_put_int (fp, "padding", ma->padding);
	lxpanel_put_int (fp, "spacing", ma->spacing);
	lxpanel_put_int (fp, "show-frame", ma->show_frame);
	lxpanel_put_int (fp, "orientation", ma->orientation_policy);

	for ( i = 0; i < NGRAPHS; i++ ) {
		char *key, list[10*MAX_COLORS];

		/* Visibility */
		key = g_strdup_printf("%sVisible", graph_types[i].name);
		lxpanel_put_int (fp, key, ma->graph_config[i].visible);
		g_free (key);

		/* Save colors */
		multiload_colorconfig_stringify (ma, i, list);
		key = g_strdup_printf("%sColors", graph_types[i].name);
		/* Don't use lxpanel_put_str (fp, key, list), in order to avoid a compiler
		 warning "the address of 'list' will always evaluate as 'true'".
		*/
		lxpanel_put_line(fp, "%s=%s", key, list);
		g_free (key);
	}
}

static void multiload_panel_configuration_changed(Plugin *p)
{
	MultiloadLxpanelPlugin *multiload = p->priv;

	/* Determine orientation and size */
	if ( p->panel->orientation == GTK_ORIENTATION_VERTICAL ) {
		multiload->ma->panel_orientation = GTK_ORIENTATION_VERTICAL;
		gtk_widget_set_size_request (p->pwid, p->panel->width, -1);
	} else { // p->panel->orientation can have values other than vert/horiz
		multiload->ma->panel_orientation = GTK_ORIENTATION_HORIZONTAL;
		gtk_widget_set_size_request (p->pwid, -1, p->panel->height);
	}

	/* Refresh the panel applet */
	multiload_refresh(&(multiload->ma));
}

static gboolean
multiload_press_event(GtkWidget *pwid, GdkEventButton *event, Plugin *p)
{
	/* Standard right-click handling. */
	if (plugin_button_press_event(pwid, event, p))
		return TRUE;

	if (event->button == 1) { /* left button */
	/* Launch system monitor */
	}
	return TRUE;
}

static int
multiload_constructor(Plugin *p, char **fp)
{
	/* allocate our private structure instance */
	MultiloadLxpanelPlugin *multiload = g_new0(MultiloadLxpanelPlugin, 1);
	p->priv = multiload;

	/* Initialize multiload */
	multiload_init ();
	multiload->dlg = NULL;

	/* read the user settings */
	multiload_read (fp, &multiload->ma);

	/* create a container widget */
	p->pwid = gtk_event_box_new ();
	gtk_widget_show (p->pwid);

	/* Initialize the applet */
	multiload->ma.container = GTK_CONTAINER(p->pwid);
	/* Set size request and update orientation */
	multiload_panel_configuration_changed(p);

	g_signal_connect(p->pwid, "button-press-event",
						G_CALLBACK(multiload_press_event), p);
	/* FIXME: No way to add system monitor item to menu? */

	return 1;
}

static void
multiload_destructor(Plugin * p)
{
	/* find our private structure instance */
	MultiloadLxpanelPlugin *multiload = (MultiloadLxpanelPlugin *)p->priv;

	/* Destroy dialog */
	if ( multiload->dlg ) {
		gtk_widget_destroy (multiload->dlg);
		multiload->dlg = NULL;
	}

	/* free private data. Panel will free pwid for us. */
	g_free(multiload);
}

static void
multiload_configure_response (GtkWidget              *dialog,
							  gint                    response,
							  MultiloadLxpanelPlugin *multiload)
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
		gtk_widget_destroy (multiload->dlg);
		multiload->dlg = NULL;
	}
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
	g_assert( ma != NULL);
return ma;
}

static void multiload_configure(Plugin * p, GtkWindow * parent)
{
	GtkWidget *dialog;
	MultiloadLxpanelPlugin *multiload = (MultiloadLxpanelPlugin *)p->priv;
	if ( multiload->dlg != NULL ) {
		gtk_widget_show_all (multiload->dlg);
		gtk_window_present (GTK_WINDOW (multiload->dlg));
		return;
	}

	/* create the dialog */
	multiload->dlg = gtk_dialog_new_with_buttons(_("Multiload preferences"),
					parent,
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_STOCK_HELP, GTK_RESPONSE_HELP,
					GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
					NULL);
	dialog = multiload->dlg;

	/* center dialog on the screen */
	gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

	/* set dialog icon */
	gtk_window_set_icon_name (GTK_WINDOW (dialog), "utilities-system-monitor");

	/* link the dialog to the plugin, so we can destroy it when the plugin
	* is closed, but the dialog is still open */
	g_object_set_data (G_OBJECT (dialog), "MultiloadPlugin", &multiload->ma);

	/* Initialize dialog widgets */
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	multiload_init_preferences(dialog, &multiload->ma);

	/* connect the reponse signal to the dialog */
	g_signal_connect (G_OBJECT (dialog), "response",
					G_CALLBACK(multiload_configure_response), multiload);

	/* Magic incantation from lxpanel/src/plugins/launchbar.c */
	/* Establish a callback when the dialog completes. */
	g_object_weak_ref(G_OBJECT(dialog), (GWeakNotify) panel_config_save, p->panel);

	/* show the entire dialog */
	gtk_widget_show_all (dialog);
}

/* Plugin descriptor. */
PluginClass multiload_plugin_class = {

	// this is a #define taking care of the size/version variables
	PLUGINCLASS_VERSIONING,

	// type of this plugin
	type : "multiload",
	name : N_("Multiload"),
	version: PACKAGE_VERSION,
	description : N_("A system load monitor that graphs processor, memory, "
					"and swap space use, plus network and disk activity."),

	// we can have many running at the same time
	one_per_system : FALSE,

	// can't expand this plugin
	expand_available : FALSE,

	// assigning our functions to provided pointers.
	constructor : multiload_constructor,
	destructor  : multiload_destructor,
	config : multiload_configure,
	save : multiload_save_configuration,
panel_configuration_changed : multiload_panel_configuration_changed
};

