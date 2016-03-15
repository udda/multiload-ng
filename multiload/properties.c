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
static GtkWidget *infobar_widgets_speed[2];
static GtkWidget *infobar_widgets_orientation[2];
static GtkWidget *infobar_widgets_padding[2];

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
show_hide_warnings(MultiloadPlugin *ma)
{
	int n;
	gboolean b;

	static int tooltip_timeout = -1;
	if (tooltip_timeout == -1)
		g_object_get(gtk_settings_get_default(), "gtk-tooltip-timeout", &tooltip_timeout, NULL);

	n = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(infobar_widgets_speed[0]));
	b = (n < tooltip_timeout);
	gtk_widget_set_visible(infobar_widgets_speed[1], b);

	n = gtk_combo_box_get_active(GTK_COMBO_BOX(infobar_widgets_orientation[0]));
	b = ((ma->panel_orientation == GTK_ORIENTATION_HORIZONTAL && n == 2) ||
		(ma->panel_orientation == GTK_ORIENTATION_VERTICAL && n == 1));
	gtk_widget_set_visible(infobar_widgets_orientation[1], b);

	n = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(infobar_widgets_padding[0]));
	b = (n > 10);
	gtk_widget_set_visible(infobar_widgets_padding[1], b);

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

			//border color update needs refresh
			if (i == multiload_colors_get_extra_index(graph, EXTRA_COLOR_BORDER))
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
	show_hide_warnings(ma);
}

/*
// create a new page in the notebook widget, add it, and return a pointer to it
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
*/

// create a color selector with optional custom label
static GtkWidget *
color_selector_new(guint graph, guint index, gboolean use_alpha, gboolean use_label, MultiloadPlugin *ma)
{
	GtkWidget *box;
	GtkWidget *label;
	GtkWidget *color_picker;

	const gchar *name = graph_types[graph].colors[index].label_interactive;
	const gchar *tooltip = graph_types[graph].colors[index].label_noninteractive;
	const gchar *dialog_title = g_strdup_printf(_("Select color:  %s -> %s"),
					graph_types[graph].label_noninteractive,
					graph_types[graph].colors[index].label_noninteractive);

	box = gtk_hbox_new (FALSE, PREF_LABEL_SPACING);

	// color button
	color_picker = gtk_color_button_new_with_color(
					&ma->graph_config[graph].colors[index]);
	gtk_color_button_set_title (GTK_COLOR_BUTTON(color_picker),	dialog_title);
	gtk_box_pack_start (GTK_BOX(box), color_picker, FALSE, FALSE, 0);
	gtk_widget_set_tooltip_text(color_picker, tooltip);

	if (use_alpha) {
		gtk_color_button_set_use_alpha (GTK_COLOR_BUTTON(color_picker), TRUE);
		gtk_color_button_set_alpha (GTK_COLOR_BUTTON(color_picker),
					ma->graph_config[graph].alpha[index]);
	}

	if (use_label) {
		label = gtk_label_new_with_mnemonic(name);
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
	static GtkVBox *container = NULL;
	GtkSizeGroup *sizegroup, *sizegroup2;
	GtkWidget *frame, *frame2;
	GtkWidget *box, *box2, *box3;
	GtkWidget *label;
	GtkWidget *t;

	GtkWidget *contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	// Delete old container if present
	if (G_UNLIKELY(GTK_IS_WIDGET(container)))
		gtk_container_remove(GTK_CONTAINER(contentArea), GTK_WIDGET(container));

	// Create new container
	container = GTK_VBOX(gtk_vbox_new(FALSE, PREF_CONTENT_PADDING));
	gtk_box_pack_start(GTK_BOX(contentArea), GTK_WIDGET(container), TRUE, TRUE, 0);



	// COLORS
	box = gtk_hbox_new(FALSE, PREF_CONTENT_PADDING);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, FALSE, 0);

	sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_BOTH);
	for( i = 0; i < GRAPH_MAX; i++ ) {
		// -- -- checkbox
		t = gtk_check_button_new_with_mnemonic(graph_types[i].label_interactive);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(t),
							ma->graph_config[i].visible);
		g_signal_connect(G_OBJECT(t), "toggled",
							G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_SHOWGRAPH | i));
		gtk_widget_set_tooltip_text(t, _("Show or hide this graph"));
		checkbuttons[i] = t;

		// -- -- frame
		frame = gtk_frame_new(NULL);
		gtk_frame_set_label_widget(GTK_FRAME(frame), t);
		gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(frame), FALSE, FALSE, PREF_CONTENT_PADDING);

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
		frame2 = gtk_frame_new(_("Background"));
		gtk_box_pack_end(GTK_BOX(box2), frame2, FALSE, FALSE, PREF_LABEL_SPACING);

		box3 = gtk_hbox_new(FALSE, PREF_CONTENT_PADDING);
		gtk_container_set_border_width(GTK_CONTAINER(box3), PREF_CONTENT_PADDING);
		gtk_container_add(GTK_CONTAINER(frame2), GTK_WIDGET(box3));

		k = multiload_colors_get_extra_index(i, EXTRA_COLOR_BACKGROUND_TOP);
		t = color_selector_new(i, k, FALSE, FALSE, ma);
		gtk_box_pack_start(GTK_BOX(box3), t, FALSE, FALSE, 0);

		k = multiload_colors_get_extra_index(i, EXTRA_COLOR_BACKGROUND_BOTTOM);
		t = color_selector_new(i, k, FALSE, FALSE, ma);
		gtk_box_pack_start(GTK_BOX(box3), t, FALSE, FALSE, 0);


		// border
		k = multiload_colors_get_extra_index(i, EXTRA_COLOR_BORDER);
		frame2 = gtk_frame_new(graph_types[i].colors[k].label_noninteractive);
		gtk_box_pack_end(GTK_BOX(box2), frame2, FALSE, FALSE, PREF_LABEL_SPACING);

		box3 = gtk_hbox_new(FALSE, PREF_CONTENT_PADDING);
		gtk_container_set_border_width(GTK_CONTAINER(box3), PREF_CONTENT_PADDING);
		gtk_container_add(GTK_CONTAINER(frame2), GTK_WIDGET(box3));

		t = color_selector_new(i, k, FALSE, FALSE, ma);
		gtk_box_pack_start(GTK_BOX(box3), t, FALSE, FALSE, 0);

		t = gtk_spin_button_new_with_parameters(
						MIN_BORDER_WIDTH, MAX_BORDER_WIDTH, STEP_BORDER_WIDTH,
						ma->graph_config[i].border_width, NULL);
		gtk_entry_set_width_chars(GTK_ENTRY(t), 2);
		g_signal_connect(G_OBJECT(t), "value_changed",
				G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_BORDERWIDTH | i));
		gtk_widget_set_tooltip_text(t, _("Border width"));
		gtk_box_pack_start(GTK_BOX(box3), t, FALSE, FALSE, 0);
	}
	properties_set_checkboxes_sensitive(ma, FALSE);

	// -- bottom buttons
	box = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_button_new_with_label(_("Default colors"));
	gtk_button_set_image(GTK_BUTTON(t), gtk_image_new_from_icon_name(GTK_STOCK_REVERT_TO_SAVED, GTK_ICON_SIZE_SMALL_TOOLBAR));
	g_signal_connect(G_OBJECT(t), "clicked", G_CALLBACK(action_performed_cb),
							GINT_TO_POINTER(ACTION_DEFAULT_COLORS));
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_button_new_with_label(_("Import color theme"));
	gtk_button_set_image(GTK_BUTTON(t), gtk_image_new_from_icon_name(GTK_STOCK_OPEN, GTK_ICON_SIZE_SMALL_TOOLBAR));
	g_signal_connect(G_OBJECT(t), "clicked", G_CALLBACK(action_performed_cb),
							GINT_TO_POINTER(ACTION_IMPORT_COLORS));
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_button_new_with_label(_("Export color theme"));
	gtk_button_set_image(GTK_BUTTON(t), gtk_image_new_from_icon_name(GTK_STOCK_SAVE_AS, GTK_ICON_SIZE_SMALL_TOOLBAR));
	g_signal_connect(G_OBJECT(t), "clicked", G_CALLBACK(action_performed_cb),
							GINT_TO_POINTER(ACTION_EXPORT_COLORS));
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);



	// OPTIONS

	sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	sizegroup2 = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	// Width / Height
	box = gtk_hbox_new(FALSE, PREF_LABEL_SPACING);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, FALSE, 0);

	if (multiload_get_orientation(ma) == GTK_ORIENTATION_HORIZONTAL)
		label = gtk_label_new_with_mnemonic(_("Wid_th: "));
	else
		label = gtk_label_new_with_mnemonic(_("Heigh_t: "));
	gtk_misc_set_alignment(GTK_MISC(label), 0.0f, 0.5f);
	gtk_size_group_add_widget(sizegroup, label);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_spin_button_new_with_parameters(MIN_SIZE, MAX_SIZE, STEP_SIZE, ma->size, _("%d pixel"));
	gtk_label_set_mnemonic_widget (GTK_LABEL(label), t);
	g_signal_connect(G_OBJECT(t), "value_changed",
			G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_SIZE));
	gtk_size_group_add_widget(sizegroup2, t);
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);

	// -- -- row: padding
	box = gtk_hbox_new(FALSE, PREF_LABEL_SPACING);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, FALSE, 0);

	label = gtk_label_new_with_mnemonic(_("Pa_dding: "));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_size_group_add_widget(sizegroup, label);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_spin_button_new_with_parameters(MIN_PADDING, MAX_PADDING, STEP_PADDING, ma->padding, _("%d pixel"));
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), t);
	g_signal_connect(G_OBJECT(t), "value_changed",
			G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_PADDING));
	gtk_size_group_add_widget(sizegroup2, t);
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);
	infobar_widgets_padding[0] = t;

	t = gtk_warning_bar_new(_("If padding is set too large, the graph won't show."));
	gtk_box_pack_start(GTK_BOX(box), t, TRUE, TRUE, PREF_CONTENT_PADDING);
	infobar_widgets_padding[1] = t;

	// -- -- row: spacing
	box = gtk_hbox_new(FALSE, PREF_LABEL_SPACING);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, FALSE, 0);

	label = gtk_label_new_with_mnemonic(_("S_pacing: "));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_size_group_add_widget(sizegroup, label);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_spin_button_new_with_parameters(MIN_SPACING, MAX_SPACING, STEP_SPACING, ma->spacing, _("%d pixel"));
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), t);
	g_signal_connect(G_OBJECT(t), "value_changed",
			G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_SPACING));
	gtk_size_group_add_widget(sizegroup2, t);
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);

	// -- -- row: update interval
	box = gtk_hbox_new(FALSE, PREF_LABEL_SPACING);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, FALSE, 0);

	label = gtk_label_new_with_mnemonic(_("Upd_ate interval: "));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_size_group_add_widget(sizegroup, label);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_spin_button_new_with_parameters(MIN_SPEED, MAX_SPEED, STEP_SPEED, ma->speed, _("%d milliseconds"));
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), t);
	g_signal_connect(G_OBJECT(t), "value_changed",
			G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_SPEED));
	gtk_size_group_add_widget(sizegroup2, t);
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);
	infobar_widgets_speed[0] = t;

	t = gtk_warning_bar_new(_("System settings could prevent the informative tooltip to show if the update interval is set too short."));
	gtk_box_pack_start(GTK_BOX(box), t, TRUE, TRUE, PREF_CONTENT_PADDING);
	infobar_widgets_speed[1] = t;

	// -- -- row: orientation
	box = gtk_hbox_new(FALSE, PREF_LABEL_SPACING);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, FALSE, 0);

	label = gtk_label_new_with_mnemonic(_("_Orientation: "));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_size_group_add_widget(sizegroup, label);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_combo_box_text_new();
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(t), _("Automatic"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(t), _("Horizontal"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(t), _("Vertical"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(t), ma->orientation_policy);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), t);
	g_signal_connect(G_OBJECT(t), "changed",
			G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_ORIENTATION));
	gtk_size_group_add_widget(sizegroup2, t);
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);
	infobar_widgets_orientation[0] = t;

	t = gtk_warning_bar_new(_("Selected orientation is not the same of the panel. Graphs could be very small."));
	gtk_box_pack_start(GTK_BOX(box), t, TRUE, TRUE, PREF_CONTENT_PADDING);
	infobar_widgets_orientation[1] = t;


	// -- checkbox
	t = gtk_check_button_new_with_mnemonic(_("Fill space between graphs"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(t),
						ma->fill_between);
	g_signal_connect(G_OBJECT(t), "toggled",
						G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_FILLBETWEEN));
	gtk_box_pack_start(GTK_BOX(container), t, FALSE, FALSE, PREF_CONTENT_PADDING);
	

	gtk_widget_show_all(GTK_WIDGET(contentArea));
	show_hide_warnings(ma);
}
