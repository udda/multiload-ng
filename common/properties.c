/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
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
#include <glib/gi18n-lib.h>

#include "gtk-compat.h"
#include "load-graph.h"
#include "multiload.h"
#include "multiload-config.h"
#include "multiload-colors.h"
#include "properties.h"
#include "util.h"


#define PREF_CONTENT_PADDING 5
#define PREF_LABEL_SPACING 3

static GList *dynamic_widgets = NULL;
static GtkWidget *checkbuttons[GRAPH_MAX];

static MultiloadPlugin *
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
								_("Import color scheme"));
			if (filename != NULL) {
				multiload_colors_from_file(filename, ma, GTK_WINDOW(pref_dialog));
				g_free (filename);
			}
		break;

		case ACTION_EXPORT_COLORS:
			filename = gtk_save_file_dialog(GTK_WINDOW(pref_dialog),
								_("Export color scheme"), "multiload.colors");
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
manage_dynamic_widgets(MultiloadPlugin *ma)
{
	GList* l;
	gchar *str1, *str2;

	static int tooltip_timeout = -1;
	if (tooltip_timeout == -1)
		g_object_get(gtk_settings_get_default(), "gtk-tooltip-timeout", &tooltip_timeout, NULL);

	// skip first element (NULL)
	for ( l=dynamic_widgets->next; ; l = l->next ) {
		if (l == NULL)
			break;
		GtkWidget *w = GTK_WIDGET(l->data);
		GtkWidget *target = GTK_WIDGET(g_object_get_data (G_OBJECT(w), "dynamic_target"));
		gint prop_type = GPOINTER_TO_INT(g_object_get_data (G_OBJECT(w), "dynamic_prop"));
		switch (prop_type) {
			case PROP_SIZE:
				str1 = format_time_duration(ma->size * ma->interval / 1000);
				str2 = g_strdup_printf(_("Graph timespan is %s"), str1);
				gtk_label_set_text (GTK_LABEL(g_object_get_data (G_OBJECT(target), "label")), str2);
				g_free(str1);
				g_free(str2);
				break;

			case PROP_INTERVAL:
				gtk_widget_set_visible (target, ma->interval < tooltip_timeout);
				break;

			case PROP_ORIENTATION:
				gtk_widget_set_visible (target, (
						ma->panel_orientation == GTK_ORIENTATION_HORIZONTAL &&
						ma->orientation_policy == MULTILOAD_ORIENTATION_VERTICAL
					) || (
						ma->panel_orientation == GTK_ORIENTATION_VERTICAL &&
						ma->orientation_policy == MULTILOAD_ORIENTATION_HORIZONTAL
					));
				break;

			case PROP_PADDING:
				gtk_widget_set_visible (target, ma->padding >= 10);
				break;

			case PROP_DBLCLICK_POLICY:
				gtk_widget_set_visible (target,
						ma->dblclick_policy == DBLCLICK_POLICY_CMDLINE);
				break;

			default:
				g_assert_not_reached();
		}
	}
}


static void
property_changed_cb(GtkWidget *widget, gpointer id) {
	MultiloadPlugin *ma = multiload_configure_get_plugin(widget);
	// properties are all in the first 16 bits; this make room for another 16 bit value
	gint prop_type = GPOINTER_TO_INT(id) & 0xFFFF0000;
	gint prop_data = GPOINTER_TO_INT(id) & 0x0000FFFF;

	gint graph;
	guint i;

	gint val_int;
	const gchar *val_str;

	g_debug("[properties] Property changed (type=%d, data=%d)", prop_type, prop_data);

	switch(prop_type) {
		case PROP_SHOWGRAPH:
			g_assert(prop_data>=0 && prop_data<GRAPH_MAX);
			val_int = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
			properties_set_checkboxes_sensitive(ma, val_int);
			ma->graph_config[prop_data].visible = val_int;
			if (val_int != 0) {
				gtk_widget_show_all (ma->graphs[prop_data]->main_widget);
				load_graph_start(ma->graphs[prop_data]);
			} else {
				load_graph_stop(ma->graphs[prop_data]);
				gtk_widget_hide (ma->graphs[prop_data]->main_widget);
			}
			break;

		case PROP_INTERVAL:
			val_int = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
			ma->interval = val_int;
			for (i = 0; i < GRAPH_MAX; i++) {
				load_graph_stop(ma->graphs[i]);
				if (ma->graph_config[i].visible)
					load_graph_start(ma->graphs[i]);
			}
			break;

		case PROP_SIZE:
			val_int = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
			ma->size = val_int;
			for (i = 0; i < GRAPH_MAX; i++)
				load_graph_resize(ma->graphs[i]);
			break;

		case PROP_PADDING:
			val_int = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
			ma->padding = val_int;
			multiload_refresh(ma);
			break;

		case PROP_SPACING:
			val_int = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
			ma->spacing = val_int;
			multiload_refresh(ma);
			break;

		case PROP_ORIENTATION:
			val_int = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
			ma->orientation_policy = val_int;
			multiload_refresh(ma);
			break;

		case PROP_BORDERWIDTH:
			g_assert(prop_data>=0 && prop_data<GRAPH_MAX);
			val_int = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
			ma->graph_config[prop_data].border_width = val_int;
			multiload_refresh(ma);
			break;

		case PROP_COLOR:
			graph = prop_data >> 8;
			i = prop_data & 0xFF;

			g_assert(graph>=0 && graph<GRAPH_MAX);
			g_assert(i>=0 && i<multiload_config_get_num_colors(graph));

			gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(widget), &ma->graph_config[graph].colors[i]);

			//border color update needs refresh
			if (i == multiload_colors_get_extra_index(graph, EXTRA_COLOR_BORDER))
				multiload_refresh(ma);
			break;

		case PROP_FILLBETWEEN:
			val_int = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
			ma->fill_between = val_int;
			multiload_refresh(ma);
			break;

		case PROP_TOOLTIP_DETAILS:
			val_int = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
			ma->tooltip_details = val_int;
			break;

		case PROP_DBLCLICK_POLICY:
			val_int = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
			ma->dblclick_policy = val_int;
			break;

		case PROP_DBLCLICK_CMDLINE:
			val_str = gtk_entry_get_text(GTK_ENTRY(widget));
			i = sizeof(ma->dblclick_cmdline)/sizeof(gchar);
			strncpy(ma->dblclick_cmdline, val_str, i);
			break;

		default:
			g_assert_not_reached();
	}
	manage_dynamic_widgets(ma);
}

/*
// create a new page in the notebook widget, add it, and return a pointer to it
static GtkWidget *
add_page(GtkNotebook *notebook, const gchar *label, const gchar *description)
{
	GtkWidget *page;
	GtkWidget *page_label;

	page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
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

	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, PREF_LABEL_SPACING);

	// color button
	color_picker = gtk_color_button_new_with_rgba(&ma->graph_config[graph].colors[index]);
	gtk_color_chooser_set_use_alpha (GTK_COLOR_CHOOSER(color_picker), use_alpha);
	gtk_color_button_set_title (GTK_COLOR_BUTTON(color_picker), dialog_title);
	gtk_box_pack_start (GTK_BOX(box), color_picker, FALSE, FALSE, 0);
	gtk_widget_set_tooltip_text(color_picker, tooltip);

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
	GtkWidget *container;
	GtkSizeGroup *sizegroup, *sizegroup2;
	GtkWidget *frame;
	GtkWidget *box, *box2, *box3, *box4;
	GtkWidget *label;
	GtkWidget *t, *p;

	GtkWidget *contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	// Init warning bar widget list
	if (dynamic_widgets != NULL)
		g_list_free(dynamic_widgets);
	dynamic_widgets = g_list_append(NULL, NULL);

	// Delete existing dialog contents, if present
	container = GTK_WIDGET(g_object_get_data (G_OBJECT(dialog), "ContentVBox"));
	if (G_UNLIKELY(container != NULL && GTK_IS_WIDGET(container)))
		gtk_widget_destroy(container);

	// Create new container
	container = gtk_box_new(GTK_ORIENTATION_VERTICAL, PREF_CONTENT_PADDING);
	gtk_box_pack_start(GTK_BOX(contentArea), container, TRUE, TRUE, 0);
	g_object_set_data (G_OBJECT(dialog), "ContentVBox", container);


	// COLORS
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, PREF_CONTENT_PADDING);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, FALSE, 0);

	sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_BOTH);
	for( i = 0; i < GRAPH_MAX; i++ ) {
		// graph enable
		t = gtk_check_button_new_with_mnemonic(graph_types[i].label_interactive);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(t),
							ma->graph_config[i].visible);
		g_signal_connect(G_OBJECT(t), "toggled",
							G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_SHOWGRAPH | i));
		gtk_widget_set_tooltip_text(t, _("Show or hide this graph"));
		checkbuttons[i] = t;

		// frame
		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
		gtk_frame_set_label_widget(GTK_FRAME(frame), t);
		gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(frame), FALSE, FALSE, 0);

		box2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_container_set_border_width(GTK_CONTAINER(box2), PREF_CONTENT_PADDING);
		gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(box2));

		// colors
		k = multiload_config_get_num_data(i);
		for( j=0; j<k; j++ ) {
			t = color_selector_new(i, j, TRUE, TRUE, ma);
			gtk_size_group_add_widget(sizegroup, t);
			gtk_box_pack_start(GTK_BOX(box2), t, FALSE, FALSE, 0);
		}


		// background color
		box4 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_box_pack_end(GTK_BOX(box2), box4, FALSE, FALSE, 0);

		t = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
		gtk_widget_set_size_request(t, -1, 3);
		gtk_box_pack_start(GTK_BOX(box4), t, FALSE, FALSE, 2);

		label = gtk_label_new(_("Background"));
		g_object_set(G_OBJECT(label), "yalign", 1.0, NULL, NULL);
		gtk_box_pack_start(GTK_BOX(box4), label, FALSE, FALSE, 0);

		box3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, PREF_CONTENT_PADDING);
		gtk_container_set_border_width(GTK_CONTAINER(box3), PREF_CONTENT_PADDING);
		gtk_box_pack_start(GTK_BOX(box4), GTK_WIDGET(box3), FALSE, FALSE, 0);

		k = multiload_colors_get_extra_index(i, EXTRA_COLOR_BACKGROUND_TOP);
		t = color_selector_new(i, k, FALSE, FALSE, ma);
		gtk_box_pack_start(GTK_BOX(box3), t, FALSE, FALSE, 0);

		k = multiload_colors_get_extra_index(i, EXTRA_COLOR_BACKGROUND_BOTTOM);
		t = color_selector_new(i, k, FALSE, FALSE, ma);
		gtk_box_pack_start(GTK_BOX(box3), t, FALSE, FALSE, 0);

		// border
		box4 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_box_pack_end(GTK_BOX(box2), box4, FALSE, FALSE, PREF_CONTENT_PADDING);

		label = gtk_label_new(NULL);
		gtk_box_pack_start(GTK_BOX(box4), label, FALSE, FALSE, PREF_CONTENT_PADDING);

		t = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
		gtk_widget_set_size_request(t, -1, 3);
		gtk_box_pack_start(GTK_BOX(box4), t, FALSE, FALSE, 2);

		label = gtk_label_new(_("Border"));
		g_object_set(G_OBJECT(label), "yalign", 1.0, NULL);
		gtk_box_pack_start(GTK_BOX(box4), label, FALSE, FALSE, 0);

		box3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, PREF_CONTENT_PADDING);
		gtk_container_set_border_width(GTK_CONTAINER(box3), PREF_CONTENT_PADDING);
		gtk_box_pack_start(GTK_BOX(box4), GTK_WIDGET(box3), FALSE, FALSE, 0);

		k = multiload_colors_get_extra_index(i, EXTRA_COLOR_BORDER);
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

	// color scheme buttons
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_button_new_with_label(_("Default colors"));
	gtk_button_set_image(GTK_BUTTON(t), gtk_image_new_from_icon_name("document-revert", GTK_ICON_SIZE_SMALL_TOOLBAR));
	g_signal_connect(G_OBJECT(t), "clicked", G_CALLBACK(action_performed_cb),
							GINT_TO_POINTER(ACTION_DEFAULT_COLORS));
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_button_new_with_label(_("Import color scheme"));
	gtk_button_set_image(GTK_BUTTON(t), gtk_image_new_from_icon_name("document-open", GTK_ICON_SIZE_SMALL_TOOLBAR));
	g_signal_connect(G_OBJECT(t), "clicked", G_CALLBACK(action_performed_cb),
							GINT_TO_POINTER(ACTION_IMPORT_COLORS));
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_button_new_with_label(_("Export color scheme"));
	gtk_button_set_image(GTK_BUTTON(t), gtk_image_new_from_icon_name("document-save-as", GTK_ICON_SIZE_SMALL_TOOLBAR));
	g_signal_connect(G_OBJECT(t), "clicked", G_CALLBACK(action_performed_cb),
							GINT_TO_POINTER(ACTION_EXPORT_COLORS));
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);



	// OPTIONS

	sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	sizegroup2 = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	// Width / Height
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, PREF_LABEL_SPACING);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, FALSE, 0);

	if (ma->panel_orientation == GTK_ORIENTATION_HORIZONTAL)
		label = gtk_label_new_with_mnemonic(_("Wid_th:"));
	else
		label = gtk_label_new_with_mnemonic(_("Heigh_t:"));
	g_object_set(G_OBJECT(label), "xalign", 0.0, NULL);
	gtk_size_group_add_widget(sizegroup, label);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_spin_button_new_with_parameters(MIN_SIZE, MAX_SIZE, STEP_SIZE, ma->size, _("%d pixel"));
	gtk_label_set_mnemonic_widget (GTK_LABEL(label), t);
	g_signal_connect(G_OBJECT(t), "value_changed",
			G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_SIZE));
	gtk_size_group_add_widget(sizegroup2, t);
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);

	label = gtk_icon_label_new("dialog-information", NULL);
	gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, PREF_CONTENT_PADDING);
	g_object_set_data(G_OBJECT(t), "dynamic_target", label);
	g_object_set_data(G_OBJECT(t), "dynamic_prop", GINT_TO_POINTER(PROP_SIZE));
	g_assert_nonnull(g_list_append(dynamic_widgets, t));

	// Spacing
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, PREF_LABEL_SPACING);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, FALSE, 0);

	label = gtk_label_new_with_mnemonic(_("S_pacing:"));
	g_object_set(G_OBJECT(label), "xalign", 0.0, NULL);
	gtk_size_group_add_widget(sizegroup, label);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_spin_button_new_with_parameters(MIN_SPACING, MAX_SPACING, STEP_SPACING, ma->spacing, _("%d pixel"));
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), t);
	g_signal_connect(G_OBJECT(t), "value_changed",
			G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_SPACING));
	gtk_size_group_add_widget(sizegroup2, t);
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);

	// Padding
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, PREF_LABEL_SPACING);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, FALSE, 0);

	label = gtk_label_new_with_mnemonic(_("Pa_dding:"));
	g_object_set(G_OBJECT(label), "xalign", 0.0, NULL);
	gtk_size_group_add_widget(sizegroup, label);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_spin_button_new_with_parameters(MIN_PADDING, MAX_PADDING, STEP_PADDING, ma->padding, _("%d pixel"));
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), t);
	g_signal_connect(G_OBJECT(t), "value_changed",
			G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_PADDING));
	gtk_size_group_add_widget(sizegroup2, t);
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);

	label = gtk_icon_label_new("dialog-warning", _("If padding is set too large, the graph won't show."));
	gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, PREF_CONTENT_PADDING);
	g_object_set_data(G_OBJECT(t), "dynamic_target", label);
	g_object_set_data(G_OBJECT(t), "dynamic_prop", GINT_TO_POINTER(PROP_PADDING));
	g_assert_nonnull(g_list_append(dynamic_widgets, t));

	// Update interval
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, PREF_LABEL_SPACING);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, FALSE, 0);

	label = gtk_label_new_with_mnemonic(_("Upd_ate interval:"));
	g_object_set(G_OBJECT(label), "xalign", 0.0, NULL);
	gtk_size_group_add_widget(sizegroup, label);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_spin_button_new_with_parameters(MIN_INTERVAL, MAX_INTERVAL, STEP_INTERVAL, ma->interval, _("%d milliseconds"));
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), t);
	g_signal_connect(G_OBJECT(t), "value_changed",
			G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_INTERVAL));
	gtk_size_group_add_widget(sizegroup2, t);
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);

	label = gtk_icon_label_new("dialog-warning", _("Tooltip may not show if update interval is too short."));
	gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, PREF_CONTENT_PADDING);
	g_object_set_data(G_OBJECT(t), "dynamic_target", label);
	g_object_set_data(G_OBJECT(t), "dynamic_prop", GINT_TO_POINTER(PROP_INTERVAL));
	g_assert_nonnull(g_list_append(dynamic_widgets, t));

	// Orientation
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, PREF_LABEL_SPACING);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, FALSE, 0);

	label = gtk_label_new_with_mnemonic(_("_Orientation:"));
	g_object_set(G_OBJECT(label), "xalign", 0.0, NULL);
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

	label = gtk_icon_label_new("dialog-warning", _("Selected orientation is not the same of the panel. Graphs may be very small."));
	gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, PREF_CONTENT_PADDING);
	g_object_set_data(G_OBJECT(t), "dynamic_target", label);
	g_object_set_data(G_OBJECT(t), "dynamic_prop", GINT_TO_POINTER(PROP_ORIENTATION));
	g_assert_nonnull(g_list_append(dynamic_widgets, t));

	// Execute application on double click
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, PREF_LABEL_SPACING);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, FALSE, 0);

	label = gtk_label_new_with_mnemonic(_("On double clic_k:"));
	g_object_set(G_OBJECT(label), "xalign", 0.0, NULL);
	gtk_size_group_add_widget(sizegroup, label);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, PREF_CONTENT_PADDING);

	t = gtk_combo_box_text_new();
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(t), _("Do nothing"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(t), _("Start task manager"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(t), _("Execute custom command"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(t), ma->dblclick_policy);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), t);
	g_signal_connect(G_OBJECT(t), "changed",
			G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_DBLCLICK_POLICY));
	gtk_size_group_add_widget(sizegroup2, t);
	gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, PREF_CONTENT_PADDING);

	p = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(p), sizeof(ma->dblclick_cmdline)/sizeof(gchar) - 1);
	gtk_box_pack_start(GTK_BOX(box), p, FALSE, FALSE, PREF_CONTENT_PADDING);
	gtk_entry_set_text(GTK_ENTRY(p), ma->dblclick_cmdline);
	gtk_entry_set_width_chars(GTK_ENTRY(p), 36);
	g_signal_connect(G_OBJECT(p), "changed",
			G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_DBLCLICK_CMDLINE));
	g_object_set_data(G_OBJECT(t), "dynamic_target", p);
	g_object_set_data(G_OBJECT(t), "dynamic_prop", GINT_TO_POINTER(PROP_DBLCLICK_POLICY));
	g_assert_nonnull(g_list_append(dynamic_widgets, t));


	// Fill space between graphs
	t = gtk_check_button_new_with_mnemonic(_("_Fill space between graphs"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(t), ma->fill_between);
	g_signal_connect(G_OBJECT(t), "toggled",
						G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_FILLBETWEEN));
	gtk_box_pack_start(GTK_BOX(container), t, FALSE, FALSE, 0);
	
	// Detailed information in tooltips
	t = gtk_check_button_new_with_mnemonic(_("_Detailed information in tooltips"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(t), ma->tooltip_details);
	g_signal_connect(G_OBJECT(t), "toggled",
						G_CALLBACK(property_changed_cb), GINT_TO_POINTER(PROP_TOOLTIP_DETAILS));
	gtk_box_pack_start(GTK_BOX(container), t, FALSE, FALSE, 0);


	gtk_widget_show_all(GTK_WIDGET(contentArea));
	manage_dynamic_widgets(ma);
	g_debug("[preferences] Initialized");
}
