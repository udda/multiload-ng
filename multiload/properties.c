/* GNOME cpuload/memload panel applet
 * (C) 2002 The Free Software Foundation
 *
 * Authors: 
 *		  Todd Kulesza
 *
 *
 */

#include <config.h>
#include <string.h>
#include <libintl.h>
#include <gtk/gtk.h>

#include "properties.h"
#include "multiload.h"
#include "multiload-config.h"
#include "multiload-colors.h"
#include "util.h"


#define PREF_CONTENT_PADDING 6
#define PREF_LABEL_SPACING 3

static GtkWidget *checkbuttons[GRAPH_MAX];

/* Defined in panel-specific code. */
extern MultiloadPlugin *
multiload_configure_get_plugin (GtkWidget *widget);

static void
properties_set_checkboxes_sensitive(MultiloadPlugin *ma, gboolean sensitive)
{
	gint i;
	// Count the number of visible graphs
	gint visible_count = 0;
	gint last_graph = 0;

	if (!sensitive) {
		// Only set unsensitive if one checkbox remains checked
		for (i = 0; i < GRAPH_MAX; i++) {
			if (ma->graph_config[i].visible) {
				last_graph = i;
				visible_count ++;
			}
		}
	}

	if ( visible_count < 2 ) {
		if (sensitive) {
			// Enable all checkboxes
			for (i = 0; i < GRAPH_MAX; i++)
				gtk_widget_set_sensitive(checkbuttons[i], TRUE);
		} else {
			// Disable last remaining checkbox
			gtk_widget_set_sensitive(checkbuttons[last_graph], FALSE);
		}
	}

	return;
}

static void
dialog_response_cb(GtkWidget *dialog, gint response, gpointer id) {
	MultiloadPlugin *ma = g_object_get_data (G_OBJECT(dialog), "MultiloadPlugin");
	GtkWidget *pref_dialog = g_object_get_data (G_OBJECT(dialog), "PrefDialog");

	gint action_type = GPOINTER_TO_INT(id) & 0xFFFF0000;
//	gint action_data = GPOINTER_TO_INT(id) & 0x0000FFFF;

	guint i;

	switch(action_type) {
		case ACTION_DEFAULT_COLORS:
			if (response == GTK_RESPONSE_YES) {
				for ( i = 0; i < GRAPH_MAX; i++ )
					multiload_colors_default(ma, i);
				multiload_init_preferences(pref_dialog, ma);
				multiload_refresh(ma);
			}
			gtk_widget_destroy (dialog);
			break;
	}
}

static void
action_performed_cb(GtkWidget *widget, gpointer id)
{
	MultiloadPlugin *ma = multiload_configure_get_plugin(widget);
	gint action_type = GPOINTER_TO_INT(id) & 0xFFFF0000;
//	gint action_data = GPOINTER_TO_INT(id) & 0x0000FFFF;

	GtkWidget *pref_dialog = gtk_widget_get_ancestor(widget, GTK_TYPE_DIALOG);
	GtkWidget *dialog;

	gchar *filename;

	switch(action_type) {
		case ACTION_DEFAULT_COLORS:
			dialog = gtk_yesno_dialog(GTK_WINDOW(pref_dialog),
								_("Revert to default colors?\n"
								"You will lose any customization."),
								G_CALLBACK(dialog_response_cb), id);
			g_object_set_data (G_OBJECT(dialog), "MultiloadPlugin", ma);
			g_object_set_data (G_OBJECT(dialog), "PrefDialog", pref_dialog);
			gtk_widget_show_all (dialog);
			break;

		case ACTION_IMPORT_COLORS:
			filename = gtk_open_file_dialog(GTK_WINDOW(pref_dialog),
								_("Import color theme"));
			if (filename != NULL) {
				multiload_colors_from_file(filename, ma, GTK_WINDOW(pref_dialog));
				g_free (filename);
			}
		break;

		case ACTION_EXPORT_COLORS:
			filename = gtk_save_file_dialog(GTK_WINDOW(pref_dialog),
								_("Export color theme"), "multiload.colors");
			if (filename != NULL) {
				multiload_colors_to_file(filename, ma, GTK_WINDOW(pref_dialog));
				g_free (filename);
			}
		break;

		default:
			g_assert_not_reached();
	}
}


static void
property_changed_cb(GtkWidget *widget, gpointer id) {
	MultiloadPlugin *ma = multiload_configure_get_plugin(widget);
	// properties are all in the first 16 bits; this make room for another 16 bit value
	gint prop_type = GPOINTER_TO_INT(id) & 0xFFFF0000;
	gint prop_data = GPOINTER_TO_INT(id) & 0x0000FFFF;

	gint value;
	gint graph;
	gint i;


	switch(prop_type) {
		case PROP_SHOWGRAPH:
			g_assert(prop_data>=0 && prop_data<GRAPH_MAX);
			value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
			properties_set_checkboxes_sensitive(ma, value);
			ma->graph_config[prop_data].visible = value;
			if (value) {
				gtk_widget_show_all (ma->graphs[prop_data]->main_widget);
				load_graph_start(ma->graphs[prop_data]);
			} else {
				load_graph_stop(ma->graphs[prop_data]);
				gtk_widget_hide (ma->graphs[prop_data]->main_widget);
			}
			break;

		case PROP_SPEED:
			value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
			ma->speed = value;
			for (i = 0; i < GRAPH_MAX; i++) {
				load_graph_stop(ma->graphs[i]);
				if (ma->graph_config[i].visible)
					load_graph_start(ma->graphs[i]);
			}
			break;

		case PROP_SIZE:
			value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
			ma->size = value;
			for (i = 0; i < GRAPH_MAX; i++)
				load_graph_resize(ma->graphs[i]);
			break;

		case PROP_PADDING:
			value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
			ma->padding = value;
			multiload_refresh(ma);
			break;

		case PROP_SPACING:
			value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
			ma->spacing = value;
			multiload_refresh(ma);
			break;

		case PROP_ORIENTATION:
			value = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
			ma->orientation_policy = value;
			multiload_refresh(ma);
			break;

		case PROP_BORDERWIDTH:
			g_assert(prop_data>=0 && prop_data<GRAPH_MAX);
			value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
			ma->graph_config[prop_data].border_width = value;
			multiload_refresh(ma);
			break;

		case PROP_COLOR:
			graph = prop_data >> 8;
			i = prop_data & 0xFF;

			g_assert(graph>=0 && graph<GRAPH_MAX);
			g_assert(i>=0 && i<multiload_config_get_num_colors(graph));

			gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), &ma->graph_config[graph].colors[i]);
			ma->graph_config[graph].alpha[i] = gtk_color_button_get_alpha(GTK_COLOR_BUTTON(widget));

			if (i == multiload_config_get_num_colors(graph)-2) //border color, need refresh
				multiload_refresh(ma);
			break;

		case PROP_FILLBETWEEN:
			value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
			ma->fill_between = value;
			multiload_refresh(ma);
			break;

		default:
			g_assert_not_reached();
	}
}

/* create a new page in the notebook widget, add it, and return a pointer to it */
static GtkWidget *
add_page(GtkNotebook *notebook, const gchar *label, const gchar *description)
{
	GtkWidget *page;
	GtkWidget *page_label;

	page = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(page), PREF_CONTENT_PADDING);
	page_label = gtk_label_new_with_mnemonic(label);
	gtk_widget_set_tooltip_text(page_label, description);

	gtk_notebook_append_page(notebook, page, page_label);

	return page;
}


// create a color selector with optional custom label
static GtkWidget *
color_selector_new(guint graph, guint index, gboolean use_alpha, gboolean use_label, MultiloadPlugin *ma)
{
	GtkWidget *box;
	GtkWidget *label;
	GtkWidget *color_picker;

	const gchar *color_name = graph_types[graph].colors[index].label_interactive;
	const gchar *dialog_title = g_strdup_printf(_("Select color:  %s -> %s"),
					graph_types[graph].label_noninteractive,
					graph_types[graph].colors[index].label_noninteractive);

	box = gtk_hbox_new (FALSE, PREF_LABEL_SPACING);

	// color button
	color_picker = gtk_color_button_new_with_color(
					&ma->graph_config[graph].colors[index]);
	gtk_color_button_set_title (GTK_COLOR_BUTTON(color_picker),	dialog_title);
	gtk_box_pack_start (GTK_BOX(box), color_picker, FALSE, FALSE, 0);

	if (use_alpha) {
		gtk_color_button_set_use_alpha (GTK_COLOR_BUTTON(color_picker), TRUE);
		gtk_color_button_set_alpha (GTK_COLOR_BUTTON(color_picker),
					ma->graph_config[graph].alpha[index]);
	}

	if (use_label) {
		label = gtk_label_new_with_mnemonic(color_name);
		gtk_label_set_mnemonic_widget (GTK_LABEL (label), color_picker);
		gtk_box_pack_start (GTK_BOX(box), label, FALSE, FALSE, 0);
	}

	g_signal_connect(G_OBJECT(color_picker), "color_set",
					G_CALLBACK(property_changed_cb),
					GINT_TO_POINTER(PROP_COLOR | ((graph&0xFF)<<8) | (index & 0xFF)));

	return box;
}

// create the properties dialog and initialize it from current configuration
void
multiload_init_preferences(GtkWidget *dialog, MultiloadPlugin *ma)
{
	guint i, j, k;
	static GtkNotebook *tabs = NULL;
	GtkSizeGroup *sizegroup;
	GtkWidget *page;
	GtkWidget *frame, *frame2;
	GtkWidget *box, *box2, *box3;
	GtkTable *table;
	GtkWidget *label;
	GtkWidget *t;

	GtkWidget *contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	// Delete old container if present
	if (G_UNLIKELY(GTK_IS_WIDGET(tabs)))
		gtk_container_remove(GTK_CONTAINER(contentArea), GTK_WIDGET(tabs));

	// Create new container
	tabs = GTK_NOTEBOOK(gtk_notebook_new());
	gtk_box_pack_start(GTK_BOX(contentArea), GTK_WIDGET(tabs), TRUE, TRUE, 0);



	// COLORS PAGE
	page = add_page(tabs, _("_Resources"), _("Change colors and visibility of the graphs."));

	box = gtk_hbox_new(FALSE, PREF_CONTENT_PADDING);
	gtk_container_add(GTK_CONTAINER(page), box);

	sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_BOTH);
	for( i = 0; i < GRAPH_MAX; i++ ) {
		// -- -- checkbox
		t = gtk_check_button_new_with_mnemonic(graph_types[i].label_interactive);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(t),
							ma->graph_config[i].visible);
		g_signal_connect(G_OBJECT(t), "toggled",
							G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_SHOWGRAPH | i));
		checkbuttons[i] = t;

		// -- -- frame
		frame = gtk_frame_new(NULL);
		gtk_frame_set_label_widget(GTK_FRAME(frame), t);
		gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(frame), FALSE, FALSE, 0);

		box2 = gtk_vbox_new(FALSE, 0);
		gtk_container_set_border_width(GTK_CONTAINER(box2), PREF_CONTENT_PADDING);
		gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(box2));

		// -- -- colors
		k = multiload_config_get_num_data(i);
		for( j=0; j<k; j++ ) {
			t = color_selector_new(i, j, TRUE, TRUE, ma);
			gtk_size_group_add_widget(sizegroup, t);
			gtk_box_pack_start(GTK_BOX(box2), t, FALSE, FALSE, 0);
		}


		// background color
		t = color_selector_new(i, k+1, FALSE, TRUE, ma);
		gtk_size_group_add_widget(sizegroup, t);
		gtk_box_pack_end(GTK_BOX(box2), t, FALSE, FALSE, 0);


		// border
		frame2 = gtk_frame_new(graph_types[i].colors[k].label_noninteractive);
		gtk_box_pack_end(GTK_BOX(box2), frame2, FALSE, FALSE, PREF_LABEL_SPACING);

		box3 = gtk_hbox_new(FALSE, PREF_CONTENT_PADDING);
		gtk_container_set_border_width(GTK_CONTAINER(box3), PREF_CONTENT_PADDING);
		gtk_container_add(GTK_CONTAINER(frame2), GTK_WIDGET(box3));

		t = color_selector_new(i, k, FALSE, FALSE, ma);
		gtk_box_pack_start(GTK_BOX(box3), t, FALSE, FALSE, 0);

		t = gtk_spin_button_new_with_parameters(
						MIN_BORDER_WIDTH, MAX_BORDER_WIDTH, STEP_BORDER_WIDTH,
						ma->graph_config[i].border_width);
		g_signal_connect(G_OBJECT(t), "value_changed",
				G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_BORDERWIDTH | i));
		gtk_box_pack_start(GTK_BOX(box3), t, FALSE, FALSE, 0);


		// separator
		t = gtk_hseparator_new();
		gtk_size_group_add_widget(sizegroup, t);
		gtk_box_pack_end(GTK_BOX(box2), t, FALSE, FALSE, 0);

	}
	properties_set_checkboxes_sensitive(ma, FALSE);

	// -- bottom buttons
	box = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(page), box, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_button_new_with_label(_("Default colors"));
	gtk_button_set_image(GTK_BUTTON(t), gtk_image_new_from_icon_name("document-revert", GTK_ICON_SIZE_BUTTON));
	g_signal_connect(G_OBJECT(t), "clicked", G_CALLBACK(action_performed_cb),
							GINT_TO_POINTER(ACTION_DEFAULT_COLORS));
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_button_new_with_label(_("Import color theme"));
	gtk_button_set_image(GTK_BUTTON(t), gtk_image_new_from_icon_name("document-open", GTK_ICON_SIZE_BUTTON));
	g_signal_connect(G_OBJECT(t), "clicked", G_CALLBACK(action_performed_cb),
							GINT_TO_POINTER(ACTION_IMPORT_COLORS));
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_button_new_with_label(_("Export color theme"));
	gtk_button_set_image(GTK_BUTTON(t), gtk_image_new_from_icon_name("document-saveas", GTK_ICON_SIZE_BUTTON));
	g_signal_connect(G_OBJECT(t), "clicked", G_CALLBACK(action_performed_cb),
							GINT_TO_POINTER(ACTION_EXPORT_COLORS));
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);



	// OPTIONS PAGE
	page = add_page(tabs, _("_Options"), _("Select settings that fit your needs."));

	// -- table
	table = GTK_TABLE(gtk_table_new(4, 3, FALSE));
	gtk_container_set_border_width(GTK_CONTAINER(table), PREF_CONTENT_PADDING);
	gtk_table_set_col_spacings(table, 4);
	gtk_table_set_row_spacings(table, 4);
	gtk_box_pack_start (GTK_BOX (page), GTK_WIDGET(table), FALSE, FALSE, 0);

	// -- -- row: width/height
	if (multiload_get_orientation(ma) == GTK_ORIENTATION_HORIZONTAL)
		label = gtk_label_new_with_mnemonic(_("Wid_th: "));
	else
		label = gtk_label_new_with_mnemonic(_("Heigh_t: "));
	gtk_misc_set_alignment(GTK_MISC(label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 0, 1, 0, 1);

	t = gtk_spin_button_new_with_parameters(MIN_SIZE, MAX_SIZE, STEP_SIZE, ma->size);
	gtk_label_set_mnemonic_widget (GTK_LABEL(label), t);
	g_signal_connect(G_OBJECT(t), "value_changed",
			G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_SIZE));
	gtk_table_attach_defaults(table, GTK_WIDGET(t), 1, 2, 0, 1);

	label = gtk_label_new (_("pixels"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 2, 3, 0, 1);

	// -- -- row: padding
	label = gtk_label_new_with_mnemonic(_("Pa_dding: "));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 0, 1, 1, 2);

	t = gtk_spin_button_new_with_parameters(MIN_PADDING, MAX_PADDING, STEP_PADDING, ma->padding);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), t);
	g_signal_connect(G_OBJECT(t), "value_changed",
			G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_PADDING));
	gtk_table_attach_defaults(table, GTK_WIDGET(t), 1, 2, 1, 2);

	label = gtk_label_new(_("pixels"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 2, 3, 1, 2);

	// -- -- row: spacing
	label = gtk_label_new_with_mnemonic(_("S_pacing: "));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 0, 1, 2, 3);

	t = gtk_spin_button_new_with_parameters(MIN_SPACING, MAX_SPACING, STEP_SPACING, ma->spacing);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), t);
	g_signal_connect(G_OBJECT(t), "value_changed",
			G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_SPACING));
	gtk_table_attach_defaults(table, GTK_WIDGET(t), 1, 2, 2, 3);

	label = gtk_label_new(_("pixels"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 2, 3, 2, 3);

	// -- -- row: update interval
	label = gtk_label_new_with_mnemonic(_("Upd_ate interval: "));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 0, 1, 3, 4);

	t = gtk_spin_button_new_with_parameters(MIN_SPEED, MAX_SPEED, STEP_SPEED, ma->speed);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), t);
	g_signal_connect(G_OBJECT(t), "value_changed",
			G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_SPEED));
	gtk_table_attach_defaults(table, GTK_WIDGET(t), 1, 2, 3, 4);

	label = gtk_label_new(_("milliseconds"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 2, 3, 3, 4);

	// -- -- row: orientation
	label = gtk_label_new_with_mnemonic(_("_Orientation: "));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_table_attach_defaults(table, GTK_WIDGET(label), 0, 1, 4, 5);

	t = gtk_combo_box_text_new();
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(t), _("Automatic"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(t), _("Horizontal"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(t), _("Vertical"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(t), ma->orientation_policy);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), t);

	g_signal_connect(G_OBJECT(t), "changed",
			G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_ORIENTATION));
	gtk_table_attach_defaults(table, GTK_WIDGET(t), 1, 2, 4, 5);


	// -- checkbox
	t = gtk_check_button_new_with_mnemonic(_("Fill space between graphs"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(t),
						ma->fill_between);
	g_signal_connect(G_OBJECT(t), "toggled",
						G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_FILLBETWEEN));
	gtk_box_pack_start(GTK_BOX(page), t, FALSE, FALSE, PREF_CONTENT_PADDING);
	

	gtk_widget_show_all(GTK_WIDGET(contentArea));
}
