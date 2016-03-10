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

#define HIG_IDENTATION		"    "

#define PREF_CONTENT_PADDING 6

static GtkWidget *checkbuttons[NGRAPHS];

/* Defined in panel-specific code. */
extern MultiloadPlugin *
multiload_configure_get_plugin (GtkWidget *widget);

static void
properties_set_checkboxes_sensitive(MultiloadPlugin *ma, gboolean sensitive)
{
	gint i;
	// CounT the number of visible graphs
	gint total_graphs = 0;
	gint last_graph = 0;

	if (!sensitive) {
		// Only set unsensitive if one checkbox remains checked
		for (i = 0; i < NGRAPHS; i++) {
			if (ma->graph_config[i].visible) {
				last_graph = i;
				total_graphs++;
			}
		}
	}

	if ( total_graphs < 2 ) {
		if (sensitive) {
			// Enable all checkboxes
			for (i = 0; i < NGRAPHS; i++)
				gtk_widget_set_sensitive(checkbuttons[i], TRUE);
		} else {
			// Disable last remaining checkbox
			gtk_widget_set_sensitive(checkbuttons[last_graph], FALSE);
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
		properties_set_checkboxes_sensitive(ma, TRUE);
		gtk_widget_show_all (ma->graphs[prop_type]->main_widget);
		ma->graph_config[prop_type].visible = TRUE;
		load_graph_start(ma->graphs[prop_type]);
	} else {
		load_graph_stop(ma->graphs[prop_type]);
		gtk_widget_hide (ma->graphs[prop_type]->main_widget);
		ma->graph_config[prop_type].visible = FALSE;
		properties_set_checkboxes_sensitive(ma, FALSE);
	}

	return;
}

static void
preference_toggled_cb(GtkWidget *widget, gpointer id)
{
	MultiloadPlugin *ma = multiload_configure_get_plugin(widget);
	gint prop_type = GPOINTER_TO_INT(id);
	gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	if (prop_type == PROP_SHOWFRAME) {
		ma->show_frame = active;
		multiload_refresh(ma, ma->orientation);
	}
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

		case PROP_PADDING:
			ma->padding = value;
			multiload_refresh(ma, ma->orientation);
			break;

		case PROP_SPACING:
			ma->spacing = value;
			multiload_refresh(ma, ma->orientation);
			break;

		default:
			g_assert_not_reached();
	}

	return;
}

/* create a new page in the notebook widget, add it, and return a pointer to it */
static GtkWidget *
add_page(GtkNotebook *notebook, const gchar *label, const gchar *description)
{
	GtkWidget *page;
	GtkWidget *page_label;

	page = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(page), PREF_CONTENT_PADDING);
	page_label = gtk_label_new(label);
	gtk_widget_set_tooltip_text(page_label, description);

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
	guint graph = color_slot >> 16;
	guint index = color_slot & 0xFFFF; 

	g_assert(graph >= 0 && graph < NGRAPHS);
	g_assert(index >= 0 && index < graph_types[graph].num_colors);

	gtk_color_button_get_color(color_picker, &ma->graph_config[graph].colors[index]);
	ma->graph_config[graph].alpha[index] = gtk_color_button_get_alpha(color_picker);

	return;
}

/* create a color selector */
static GtkWidget *
new_color_selector(guint graph, guint index, gboolean use_alpha, MultiloadPlugin *ma)
{
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *color_picker;
	guint color_slot = ( (graph&0xFFFF) << 16 ) | (index&0xFFFF);

	const gchar *color_name = graph_types[graph].colors[index].interactive_label;
	const gchar *dialog_title = g_strdup_printf(_("Select color:  %s -> %s"),
					graph_types[graph].noninteractive_label,
					graph_types[graph].colors[index].noninteractive_label);

	vbox = gtk_vbox_new (FALSE, 0);
	label = gtk_label_new_with_mnemonic(color_name);
	color_picker = gtk_color_button_new_with_color(
					&ma->graph_config[graph].colors[index]);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), color_picker);

	gtk_color_button_set_title (GTK_COLOR_BUTTON(color_picker),	dialog_title);
	if (use_alpha) {
		gtk_color_button_set_use_alpha (GTK_COLOR_BUTTON(color_picker), TRUE);
		gtk_color_button_set_alpha (GTK_COLOR_BUTTON(color_picker),
					ma->graph_config[graph].alpha[index]);
	}

	gtk_box_pack_start (GTK_BOX(vbox), color_picker, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), label, FALSE, FALSE, 0);

	g_signal_connect(G_OBJECT(color_picker), "color_set",
				G_CALLBACK(color_picker_set_cb), GINT_TO_POINTER(color_slot));

	return vbox;
}

/* creates the properties dialog and initialize it from the current
 * configuration */
void
multiload_init_preferences(GtkWidget *dialog, MultiloadPlugin *ma)
{
	guint i, j, k;
	GtkNotebook *tabs;
	GtkSizeGroup *sizegroup;
	GtkWidget *page;
	GtkWidget *frame;
	GtkWidget *box;
	GtkTable *table;
	GtkWidget *label;
	GtkWidget *t;

	tabs = GTK_NOTEBOOK(gtk_notebook_new());
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
				GTK_WIDGET(tabs), TRUE, TRUE, 0);



	// COLORS PAGE
	page = add_page(tabs, _("_Resources"), _("Change colors and visibility of the graphs."));

	sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	for( i = 0; i < NGRAPHS; i++ ) {
		// -- -- checkbox
		t = gtk_check_button_new_with_mnemonic(graph_types[i].interactive_label);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(t),
							ma->graph_config[i].visible);
		g_signal_connect(G_OBJECT(t), "toggled",
							G_CALLBACK(property_toggled_cb), GINT_TO_POINTER(i));
		checkbuttons[i] = t;

		// -- -- frame
		frame = gtk_frame_new(NULL);
		gtk_frame_set_label_widget(GTK_FRAME(frame), t);
		gtk_box_pack_start(GTK_BOX(page), GTK_WIDGET(frame), FALSE, FALSE, 0);

		box = gtk_hbox_new(FALSE, 0);
		gtk_container_set_border_width(GTK_CONTAINER(box), 2);
		gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(box));

		// -- -- colors
		k = graph_types[i].num_colors;
		for( j = 0; j < k; j++ ) {
			if (j == k-1) {
				label = gtk_label_new(NULL); // actually a spacer
				gtk_size_group_add_widget(sizegroup, label);
				gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, PREF_CONTENT_PADDING);

				t = new_color_selector(i, j, FALSE, ma);
				gtk_box_pack_end(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);
			} else {
				t = new_color_selector(i, j, TRUE, ma);
				gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);
			}
			gtk_size_group_add_widget(sizegroup, t);
		}
	}
	properties_set_checkboxes_sensitive(ma, FALSE);



	// OPTIONS PAGE
	page = add_page(tabs, _("Options"), _("Select settings that fit your needs."));

	// -- table
	table = GTK_TABLE(gtk_table_new(4, 3, FALSE));
	gtk_container_set_border_width(GTK_CONTAINER(table), PREF_CONTENT_PADDING);
	gtk_table_set_col_spacings(table, 4);
	gtk_table_set_row_spacings(table, 4);
	gtk_box_pack_start (GTK_BOX (page), GTK_WIDGET(table), FALSE, FALSE, 0);

	// -- -- row: width/height
	if (ma->orientation == GTK_ORIENTATION_HORIZONTAL)
		label = gtk_label_new_with_mnemonic(_("Wid_th: "));
	else
		label = gtk_label_new_with_mnemonic(_("Heigh_t: "));
	gtk_misc_set_alignment(GTK_MISC(label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 0, 1, 0, 1);

	t = gtk_spin_button_new_with_range(MIN_SIZE, MAX_SIZE, STEP_SIZE);
	gtk_label_set_mnemonic_widget (GTK_LABEL(label), t);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(t), (gdouble)ma->size);
	g_signal_connect(G_OBJECT(t), "value_changed",
			G_CALLBACK(spin_button_changed_cb), GINT_TO_POINTER(PROP_SIZE));
	gtk_table_attach_defaults(table, GTK_WIDGET(t), 1, 2, 0, 1);

	label = gtk_label_new (_("pixels"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 2, 3, 0, 1);

	// -- -- row: padding
	label = gtk_label_new_with_mnemonic(_("Pa_dding: "));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 0, 1, 1, 2);

	t = gtk_spin_button_new_with_range(MIN_PADDING, MAX_PADDING, STEP_PADDING);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), t);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(t), (gdouble)ma->padding);
	g_signal_connect(G_OBJECT(t), "value_changed",
			G_CALLBACK(spin_button_changed_cb), GINT_TO_POINTER(PROP_PADDING));
	gtk_table_attach_defaults(table, GTK_WIDGET(t), 1, 2, 1, 2);

	label = gtk_label_new(_("pixels"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 2, 3, 1, 2);

	// -- -- row: spacing
	label = gtk_label_new_with_mnemonic(_("S_pacing: "));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 0, 1, 2, 3);

	t = gtk_spin_button_new_with_range(MIN_SPACING, MAX_SPACING, STEP_SPACING);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), t);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(t), (gdouble)ma->spacing);
	g_signal_connect(G_OBJECT(t), "value_changed",
			G_CALLBACK(spin_button_changed_cb), GINT_TO_POINTER(PROP_SPACING));
	gtk_table_attach_defaults(table, GTK_WIDGET(t), 1, 2, 2, 3);

	label = gtk_label_new(_("pixels"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 2, 3, 2, 3);

	// -- -- row: update interval
	label = gtk_label_new_with_mnemonic(_("Upd_ate interval: "));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 0, 1, 3, 4);

	t = gtk_spin_button_new_with_range(MIN_SPEED, MAX_SPEED, STEP_SPEED);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), t);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(t), (gdouble)ma->speed);
	g_signal_connect(G_OBJECT(t), "value_changed",
			G_CALLBACK(spin_button_changed_cb), GINT_TO_POINTER(PROP_SPEED));
	gtk_table_attach_defaults(table, GTK_WIDGET(t), 1, 2, 3, 4);

	label = gtk_label_new(_("milliseconds"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 2, 3, 3, 4);

	// -- checkbox: show frame
	t = gtk_check_button_new_with_mnemonic(_("Frames around graphs"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(t), ma->show_frame);
	g_signal_connect(G_OBJECT(t), "toggled", G_CALLBACK(preference_toggled_cb),
			GINT_TO_POINTER(PROP_SHOWFRAME));
	gtk_box_pack_start(GTK_BOX(page), t, FALSE, FALSE, 0);

}
