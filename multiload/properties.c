/* GNOME cpuload/memload panel applet
 * (C) 2002 The Free Software Foundation
 *
 * Authors: 
 *		  Todd Kulesza
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <libintl.h>
#include <gtk/gtk.h>

#include "properties.h"
#include "multiload.h"

#define PROP_SPEED			6
#define PROP_SIZE			7
#define HIG_IDENTATION		"    "

/* Defined in panel-specific code. */
extern MultiloadPlugin *
multiload_configure_get_plugin (GtkWidget *widget);

#if 0
#define NEVER_SENSITIVE		"never_sensitive"
/* set sensitive and setup NEVER_SENSITIVE appropriately */
static void
hard_set_sensitive (GtkWidget *w, gboolean sensitivity)
{
	gtk_widget_set_sensitive (w, sensitivity);
	g_object_set_data (G_OBJECT (w), NEVER_SENSITIVE,
			   GINT_TO_POINTER ( ! sensitivity));
}

/* set sensitive, but always insensitive if NEVER_SENSITIVE is set */
static void
soft_set_sensitive (GtkWidget *w, gboolean sensitivity)
{
	if (g_object_get_data (G_OBJECT (w), NEVER_SENSITIVE))
		gtk_widget_set_sensitive (w, FALSE);
	else
		gtk_widget_set_sensitive (w, sensitivity);
}

static gboolean
key_writable (PanelApplet *applet, const char *key)
{
	gboolean writable;
	char *fullkey;
	static GConfClient *client = NULL;
	if (client == NULL)
		client = gconf_client_get_default ();

	fullkey = panel_applet_gconf_get_full_key (applet, key);

	writable = gconf_client_key_is_writable (client, fullkey, NULL);

	g_free (fullkey);

	return writable;
}
#else
#define soft_set_sensitive gtk_widget_set_sensitive
#endif

static void
properties_set_checkboxes_sensitive(MultiloadPlugin *ma, GtkWidget *checkbox,
									gboolean sensitive)
{
	/* CounT the number of visible graphs */
	gint i, total_graphs = 0, last_graph = 0;

	if ( !sensitive ) {
	/* Only set unsensitive if one checkbox remains checked */
		for (i = 0; i < NGRAPHS; i++) {
			if (ma->graph_config[i].visible) {
				last_graph = i;
				total_graphs++;
			}
		}
	}

	if ( total_graphs < 2 ) {
		/* Container widget that contains the checkboxes */
		GtkWidget *container = gtk_widget_get_ancestor(checkbox, GTK_TYPE_BOX);
		if (container && container != checkbox) {
				GList *list = gtk_container_get_children (GTK_CONTAINER(container));
			if ( sensitive ) {
				/* Enable all checkboxes */
				GList *item = list;
				while ( item && item->data ) {
					GtkWidget *nthbox = GTK_WIDGET (item->data);
					soft_set_sensitive(nthbox, TRUE);
					item = g_list_next (item);
				}
			} else {
				/* Disable last remaining checkbox */
				GtkWidget *nthbox = GTK_WIDGET(g_list_nth_data(list, last_graph));
				if ( nthbox )
					soft_set_sensitive(nthbox, FALSE);
				else
					g_assert_not_reached ();
			}
		} else {
			g_assert_not_reached ();
		}
	}

	return;
}

static void
property_toggled_cb(GtkWidget *widget, gpointer id)
{
	MultiloadPlugin *ma = multiload_configure_get_plugin(widget);
	gint prop_type = GPOINTER_TO_INT(id);
	gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

	if (active) {
		properties_set_checkboxes_sensitive(ma, widget, TRUE);
		gtk_widget_show_all (ma->graphs[prop_type]->main_widget);
		ma->graph_config[prop_type].visible = TRUE;
		load_graph_start(ma->graphs[prop_type]);
	} else {
		load_graph_stop(ma->graphs[prop_type]);
		gtk_widget_hide (ma->graphs[prop_type]->main_widget);
		ma->graph_config[prop_type].visible = FALSE;
		properties_set_checkboxes_sensitive(ma, widget, FALSE);
	}

	return;
}

static void
spin_button_changed_cb(GtkWidget *widget, gpointer id)
{
	MultiloadPlugin *ma = multiload_configure_get_plugin(widget);
	gint prop_type = GPOINTER_TO_INT(id);
	gint value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	gint i;

	switch(prop_type) {
		case PROP_SPEED:
			ma->speed = value;
			for (i = 0; i < NGRAPHS; i++) {
				load_graph_stop(ma->graphs[i]);
				if (ma->graph_config[i].visible)
					load_graph_start(ma->graphs[i]);
			}
			break;

		case PROP_SIZE:
			ma->size = value;
			for (i = 0; i < NGRAPHS; i++)
				load_graph_resize(ma->graphs[i]);			
			break;

		default:
			g_assert_not_reached();
	}

	return;
}

/* create a new page in the notebook widget, add it, and return a pointer to it */
static GtkWidget *
add_page(GtkNotebook *notebook, const gchar *label)
{
	GtkWidget *page;
	GtkWidget *page_label;
	
	page = gtk_hbox_new(TRUE, 0);
	page_label = gtk_label_new(label);
	gtk_container_set_border_width(GTK_CONTAINER(page), 6);
		
	gtk_notebook_append_page(notebook, page, page_label);
	
	return page;
}

/* apply the selected color to the applet */
static void
color_picker_set_cb(GtkColorButton *color_picker, gpointer data)
{
	/* Parse user data for graph and color slot */
	MultiloadPlugin *ma = multiload_configure_get_plugin(GTK_WIDGET (color_picker));
	guint color_slot = GPOINTER_TO_INT(data);
	guint graph = color_slot>>16, index = color_slot&0xFFFF; 

	g_assert(graph >= 0 && graph < NGRAPHS);
	g_assert(index >= 0 && index < graph_types[graph].num_colors);
		
	gtk_color_button_get_color(color_picker, &ma->graph_config[graph].colors[index]);

	return;
}

/* create a color selector */
static void
add_color_selector(GtkWidget *page, const gchar *name, guint graph, guint index,
				   MultiloadPlugin *ma)
{
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *color_picker;
	guint color_slot = ((graph&0xFFFF)<<16)|(index&0xFFFF);
		
	vbox = gtk_vbox_new (FALSE, 6);
	label = gtk_label_new_with_mnemonic(name);
	color_picker = gtk_color_button_new();
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), color_picker);

	gtk_box_pack_start(GTK_BOX(vbox), color_picker, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(page), vbox, FALSE, FALSE, 0);	

	gtk_color_button_set_color(GTK_COLOR_BUTTON(color_picker),
							 &ma->graph_config[graph].colors[index]);

	g_signal_connect(G_OBJECT(color_picker), "color_set",
					G_CALLBACK(color_picker_set_cb),
					GINT_TO_POINTER(color_slot));

	//if ( ! key_writable (ma->applet, gconf_path))
	//	hard_set_sensitive (vbox, FALSE);

	return;
}

/* creates the properties dialog and initialize it from the current
 * configuration */
void
multiload_init_preferences(GtkWidget *dialog, MultiloadPlugin *ma)
{
	GtkWidget *hbox, *vbox;
	GtkWidget *categories_vbox;
	GtkWidget *category_vbox;
	GtkWidget *control_vbox;
	GtkWidget *control_hbox;
	GtkWidget *indent;
	GtkWidget *spin_button;
	GtkWidget *label;
	GtkNotebook *notebook;
	GtkSizeGroup *label_size;
	GtkSizeGroup *spin_size;
	gchar *text;
	guint i;

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_widget_show (vbox);
	
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), vbox,
				TRUE, TRUE, 0);

	categories_vbox = gtk_vbox_new (FALSE, 18);
	gtk_box_pack_start (GTK_BOX (vbox), categories_vbox, TRUE, TRUE, 0);
	gtk_widget_show (categories_vbox);

	category_vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (categories_vbox), category_vbox, TRUE, TRUE, 0);
	gtk_widget_show (category_vbox);
	
	text = g_strconcat ("<span weight=\"bold\">", _("Monitored Resources"), "</span>", NULL);
	label = gtk_label_new (text);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (category_vbox), label, FALSE, FALSE, 0);
	g_free (text);
	
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (category_vbox), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox);
	
	indent = gtk_label_new (HIG_IDENTATION);
	gtk_label_set_justify (GTK_LABEL (indent), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX (hbox), indent, FALSE, FALSE, 0);
	gtk_widget_show (indent);
	
	control_vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), control_vbox, TRUE, TRUE, 0);
	gtk_widget_show (control_vbox);
	
	control_hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (control_vbox), control_hbox, TRUE, TRUE, 0);
	gtk_widget_show (control_hbox);
	

	for ( i = 0; i < NGRAPHS; i++ ) {
		GtkWidget *checkbox = gtk_check_button_new_with_mnemonic
			(graph_types[i].interactive_label);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox),
					ma->graph_config[i].visible);
		g_signal_connect(G_OBJECT(checkbox), "toggled",
				 G_CALLBACK(property_toggled_cb),
				 GINT_TO_POINTER(i));
		gtk_box_pack_start (GTK_BOX (control_hbox), checkbox,
							FALSE, FALSE, 0);

		//if ( ! key_writable (ma->applet, "view_cpuload"))
		//	hard_set_sensitive (check_box, FALSE); // FIXME

		/* If only one graph is visible, disable its checkbox. */
		if ( i == NGRAPHS-1 )
			properties_set_checkboxes_sensitive(ma, checkbox, FALSE);
	}

	category_vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (categories_vbox), category_vbox, TRUE, TRUE, 0);
	gtk_widget_show (category_vbox);

	text = g_strconcat ("<span weight=\"bold\">", _("Options"), "</span>", NULL);
	label = gtk_label_new (text);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (category_vbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);
	g_free (text);
	
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (category_vbox), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox);

	indent = gtk_label_new (HIG_IDENTATION);
	gtk_label_set_justify (GTK_LABEL (indent), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX (hbox), indent, FALSE, FALSE, 0);
	gtk_widget_show (indent);

	control_vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), control_vbox, TRUE, TRUE, 0);
	gtk_widget_show (control_vbox);
	
	control_hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (control_vbox), control_hbox, TRUE, TRUE, 0);
	gtk_widget_show (control_hbox);
	
	label_size = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	if ( ma->orientation == GTK_ORIENTATION_HORIZONTAL )
		text = g_strdup(_("Wid_th: "));
	else
		text = g_strdup(_("Heigh_t: "));
	
	label = gtk_label_new_with_mnemonic(text);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_size_group_add_widget (label_size, label);
	gtk_box_pack_start (GTK_BOX (control_hbox), label, FALSE, FALSE, 0);
	g_free(text);
	
	hbox = gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (control_hbox), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox);

	spin_size = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
			  
	spin_button = gtk_spin_button_new_with_range(MIN_SIZE, MAX_SIZE, 5);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), spin_button);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button),
				(gdouble)ma->size);
	g_signal_connect(G_OBJECT(spin_button), "value_changed",
				G_CALLBACK(spin_button_changed_cb),
				GINT_TO_POINTER(PROP_SIZE));
	
	//if ( ! key_writable (ma->applet, "size")) {
	//	hard_set_sensitive (label, FALSE);
	//	hard_set_sensitive (hbox, FALSE);
	//}

	gtk_size_group_add_widget (spin_size, spin_button);
	gtk_box_pack_start (GTK_BOX (hbox), spin_button, FALSE, FALSE, 0);
	
	label = gtk_label_new (_("pixels"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	
	control_hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (control_vbox), control_hbox, TRUE, TRUE, 0);
	gtk_widget_show (control_hbox);
	
	label = gtk_label_new_with_mnemonic(_("Upd_ate interval: "));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_size_group_add_widget (label_size, label);
	gtk_box_pack_start (GTK_BOX (control_hbox), label, FALSE, FALSE, 0);
	
	hbox = gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (control_hbox), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox);
	
	spin_button = gtk_spin_button_new_with_range(MIN_SPEED, MAX_SPEED, 50);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), spin_button);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button),
				(gdouble)ma->speed);
	g_signal_connect(G_OBJECT(spin_button), "value_changed",
				G_CALLBACK(spin_button_changed_cb),
				GINT_TO_POINTER(PROP_SPEED));
	gtk_size_group_add_widget (spin_size, spin_button);
	gtk_box_pack_start (GTK_BOX (hbox), spin_button, FALSE, FALSE, 0);

	//if ( ! key_writable (ma->applet, "speed")) {
	//	hard_set_sensitive (label, FALSE);
	//	hard_set_sensitive (hbox, FALSE);
	//}
	
	label = gtk_label_new(_("milliseconds"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	
	
	category_vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (categories_vbox), category_vbox, TRUE, TRUE, 0);
	gtk_widget_show (category_vbox);

	text = g_strconcat ("<span weight=\"bold\">", _("Colors"), "</span>", NULL);
	label = gtk_label_new (text);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (category_vbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);
	g_free (text);
	
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (category_vbox), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox);

	indent = gtk_label_new (HIG_IDENTATION);
	gtk_label_set_justify (GTK_LABEL (indent), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX (hbox), indent, FALSE, FALSE, 0);
	gtk_widget_show (indent);

	control_vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), control_vbox, TRUE, TRUE, 0);
	gtk_widget_show (control_vbox);

	notebook = GTK_NOTEBOOK(gtk_notebook_new());
	gtk_container_add (GTK_CONTAINER (control_vbox), GTK_WIDGET (notebook));
	
	for ( i = 0; i < NGRAPHS; i++ ) {
		guint j;
		GtkWidget *page = add_page(notebook,
								graph_types[i].noninteractive_label);
		gtk_container_set_border_width (GTK_CONTAINER (page), 12);
		for ( j = 0; j < graph_types[i].num_colors; j++ ) {
			add_color_selector(page, graph_types[i].colors[j].prefs_label,
								i, j, ma);
		}
	}
	gtk_notebook_set_current_page (notebook, 0);

	return;
}
