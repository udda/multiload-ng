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
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>

#include "binary-data.h"
#include "colors.h"
#include "gtk-compat.h"
#include "load-graph.h"
#include "multiload.h"
#include "multiload-colors.h"
#include "multiload-config.h"
#include "properties.h"
#include "util.h"


static GtkBuilder *builder = NULL;



static const gchar* checkbox_visibility_names[GRAPH_MAX] = {
	"cb_visible_cpu",
	"cb_visible_mem",
	"cb_visible_net",
	"cb_visible_swap",
	"cb_visible_load",
	"cb_visible_disk",
	"cb_visible_temp"
};

static const gchar* button_advanced_names[GRAPH_MAX] = {
	"button_advanced_cpu",
	"button_advanced_mem",
	"button_advanced_net",
	"button_advanced_swap",
	"button_advanced_load",
	"button_advanced_disk",
	"button_advanced_temp"
};

static const gchar* spinbutton_size_names[GRAPH_MAX] = {
	"sb_size_cpu",
	"sb_size_mem",
	"sb_size_net",
	"sb_size_swap",
	"sb_size_load",
	"sb_size_disk",
	"sb_size_temp"
};

static const gchar* spinbutton_interval_names[GRAPH_MAX] = {
	"sb_interval_cpu",
	"sb_interval_mem",
	"sb_interval_net",
	"sb_interval_swap",
	"sb_interval_load",
	"sb_interval_disk",
	"sb_interval_temp"
};

static const gchar* label_timespan_names[GRAPH_MAX] = {
	"label_timespan_cpu",
	"label_timespan_mem",
	"label_timespan_net",
	"label_timespan_swap",
	"label_timespan_load",
	"label_timespan_disk",
	"label_timespan_temp"
};

static const gchar* tooltip_style_names[GRAPH_MAX] = {
	"combo_tooltip_cpu",
	"combo_tooltip_mem",
	"combo_tooltip_net",
	"combo_tooltip_swap",
	"combo_tooltip_load",
	"combo_tooltip_disk",
	"combo_tooltip_temp"
};

static const gchar* dblclick_policy_names[GRAPH_MAX] = {
	"combo_dblclick_cpu",
	"combo_dblclick_mem",
	"combo_dblclick_net",
	"combo_dblclick_swap",
	"combo_dblclick_load",
	"combo_dblclick_disk",
	"combo_dblclick_temp"
};

static const gchar* dblclick_command_names[GRAPH_MAX] = {
	"entry_dblclick_command_cpu",
	"entry_dblclick_command_mem",
	"entry_dblclick_command_net",
	"entry_dblclick_command_swap",
	"entry_dblclick_command_load",
	"entry_dblclick_command_disk",
	"entry_dblclick_command_temp"
};

static const gchar* info_dblclick_command_names[GRAPH_MAX] = {
	"image_info_dblclick_command_cpu",
	"image_info_dblclick_command_mem",
	"image_info_dblclick_command_net",
	"image_info_dblclick_command_swap",
	"image_info_dblclick_command_load",
	"image_info_dblclick_command_disk",
	"image_info_dblclick_command_temp"
};

static const gchar* spin_border_names[GRAPH_MAX] = {
	"sb_border_cpu",
	"sb_border_mem",
	"sb_border_net",
	"sb_border_swap",
	"sb_border_load",
	"sb_border_disk",
	"sb_border_temp"
};

static const gchar* color_button_names[GRAPH_MAX][MAX_COLORS] = {
	{
		"cb_color_cpu1",
		"cb_color_cpu2",
		"cb_color_cpu3",
		"cb_color_cpu4",
		"cb_color_cpu_border",
		"cb_color_cpu_bg1",
		"cb_color_cpu_bg2"
	}, {
		"cb_color_mem1",
		"cb_color_mem2",
		"cb_color_mem3",
		"cb_color_mem4",
		"cb_color_mem_border",
		"cb_color_mem_bg1",
		"cb_color_mem_bg2"
	}, {
		"cb_color_net1",
		"cb_color_net2",
		"cb_color_net3",
		"cb_color_net_border",
		"cb_color_net_bg1",
		"cb_color_net_bg2",
		NULL
	}, {
		"cb_color_swap1",
		"cb_color_swap_border",
		"cb_color_swap_bg1",
		"cb_color_swap_bg2",
		NULL
	}, {
		"cb_color_load1",
		"cb_color_load_border",
		"cb_color_load_bg1",
		"cb_color_load_bg2",
		NULL
	}, {
		"cb_color_disk1",
		"cb_color_disk2",
		"cb_color_disk_border",
		"cb_color_disk_bg1",
		"cb_color_disk_bg2",
		NULL
	}, {
		"cb_color_temp1",
		"cb_color_temp_border",
		"cb_color_temp_bg1",
		"cb_color_temp_bg2",
		NULL
	}
};

static void
update_dynamic_widgets(MultiloadPlugin *ma)
{
	guint i;
	GraphConfig *gc;

	for (i=0; i<GRAPH_MAX; i++) {
		gc = &ma->graph_config[i];

		// timespan
		gchar *timespan = format_time_duration(gc->size * gc->interval / 1000);
		gtk_label_set_text (GTK_LABEL(gtk_builder_get_object(builder, label_timespan_names[i])), timespan);
		g_free(timespan);

		// cmdline enable
		gboolean cmdline_enabled = (gc->dblclick_policy == DBLCLICK_POLICY_CMDLINE);
		gtk_widget_set_sensitive (GTK_WIDGET(gtk_builder_get_object(builder, dblclick_command_names[i])), cmdline_enabled);
		gtk_widget_set_visible (GTK_WIDGET(gtk_builder_get_object(builder, info_dblclick_command_names[i])), cmdline_enabled);
	}

	// padding warning
	gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(builder, "image_warning_padding")), (ma->padding >= 10));

	// orientation warning
	gtk_widget_set_visible (GTK_WIDGET(gtk_builder_get_object(builder, "image_warning_orientation")),
		( ma->panel_orientation == GTK_ORIENTATION_HORIZONTAL && ma->orientation_policy == MULTILOAD_ORIENTATION_VERTICAL) ||
		( ma->panel_orientation == GTK_ORIENTATION_VERTICAL &&   ma->orientation_policy == MULTILOAD_ORIENTATION_HORIZONTAL)
	);
}


static void
prop_checkboxes_sensitive_cb (GtkToggleButton *checkbox, gpointer user_data)
{
	guint i;

	// Count the number of visible graphs
	gint visible_count = 0;
	gint last_graph = 0;
	gboolean active = gtk_toggle_button_get_active(checkbox);

	if (!active) {
		for (i = 0; i < GRAPH_MAX; i++) {
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, checkbox_visibility_names[i])))) {
				last_graph = i;
				visible_count ++;
			}
		}
	}

	if ( visible_count < 2 ) {
		if (active) {
			// Enable all checkboxes
			for (i = 0; i < GRAPH_MAX; i++)
				gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(builder, checkbox_visibility_names[i])), TRUE);
		} else {
			// Disable last remaining checkbox
			gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(builder, checkbox_visibility_names[last_graph])), FALSE);
		}
	}

	return;
}


static void
prop_graph_visibility_cb (GtkToggleButton *checkbox, MultiloadPlugin *ma)
{
	guint i;
	gboolean active;

	// find graph index
	const gchar *name = gtk_buildable_get_name(GTK_BUILDABLE(checkbox));
	for (i=0; i<GRAPH_MAX; i++) {
		if (strcmp(checkbox_visibility_names[i], name) == 0)
			break;
	}
	g_assert(i < GRAPH_MAX);

	active = gtk_toggle_button_get_active(checkbox);
	ma->graph_config[i].visible = active;
	if (active) {
		gtk_widget_show_all (ma->graphs[i]->main_widget);
		load_graph_start(ma->graphs[i]);
	} else {
		load_graph_stop(ma->graphs[i]);
		gtk_widget_hide (ma->graphs[i]->main_widget);
	}
}


static void
button_advanced_clicked_cb (GtkWidget *button, MultiloadPlugin *ma)
{
	guint i;

	// find graph index
	const gchar *name = gtk_buildable_get_name(GTK_BUILDABLE(button));
	for (i=0; i<GRAPH_MAX; i++) {
		if (strcmp(button_advanced_names[i], name) == 0)
			break;
	}
	g_assert(i < GRAPH_MAX);

	GtkWidget *dialog_config = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_advanced"));
	gtk_window_set_transient_for(GTK_WINDOW(dialog_config), GTK_WINDOW(gtk_widget_get_toplevel(button)));

	GtkWidget *notebook = GTK_WIDGET(gtk_builder_get_object(builder, "advanced_notebook"));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), i);
	gtk_widget_show(dialog_config);
}


static gint
spinbutton_size_output_cb (GtkSpinButton *spin, gpointer p)
{
	const gchar *format = _("%d pixel");

	gint n = gtk_spin_button_get_value_as_int(spin);

	gchar *s = g_strdup_printf(format, n);
	gtk_entry_set_text(GTK_ENTRY(spin), s);
	g_free(s);

	// block the default output
	return TRUE;
}


static void
spinbutton_size_change_cb (GtkSpinButton *spin, MultiloadPlugin *ma)
{
	guint i;

	// find graph index
	const gchar *name = gtk_buildable_get_name(GTK_BUILDABLE(spin));
	for (i=0; i<GRAPH_MAX; i++) {
		if (strcmp(spinbutton_size_names[i], name) == 0)
			break;
	}
	g_assert(i < GRAPH_MAX);

	ma->graph_config[i].size = gtk_spin_button_get_value_as_int(spin);
	load_graph_resize(ma->graphs[i]);

	update_dynamic_widgets(ma);
}


static gint
spinbutton_interval_output_cb (GtkSpinButton *spin, gpointer p)
{
	static int tooltip_timeout = -1;
	if (tooltip_timeout == -1)
		g_object_get(gtk_settings_get_default(), "gtk-tooltip-timeout", &tooltip_timeout, NULL);


	const gchar *format = _("%d milliseconds");

	gint n = gtk_spin_button_get_value_as_int(spin);

	gchar *s = g_strdup_printf(format, n);
	gtk_entry_set_text(GTK_ENTRY(spin), s);
	g_free(s);

	if (n < tooltip_timeout)
		gtk_entry_set_icon_from_icon_name (GTK_ENTRY(spin), GTK_ENTRY_ICON_SECONDARY, "dialog-warning");
	else
		gtk_entry_set_icon_from_icon_name (GTK_ENTRY(spin), GTK_ENTRY_ICON_SECONDARY, NULL);

	// block the default output
	return TRUE;
}


static void
spinbutton_interval_change_cb (GtkSpinButton *spin, MultiloadPlugin *ma)
{
	guint i;

	// find graph index
	const gchar *name = gtk_buildable_get_name(GTK_BUILDABLE(spin));
	for (i=0; i<GRAPH_MAX; i++) {
		if (strcmp(spinbutton_interval_names[i], name) == 0)
			break;
	}
	g_assert(i < GRAPH_MAX);

	ma->graph_config[i].interval = gtk_spin_button_get_value_as_int(spin);

	load_graph_stop(ma->graphs[i]);
	if (ma->graph_config[i].visible)
		load_graph_start(ma->graphs[i]);

	update_dynamic_widgets(ma);
}


static void
spacing_or_padding_changed_cb (GtkRange *scale, MultiloadPlugin *ma)
{
	guint value = (guint)gtk_range_get_value(scale);
	const gchar *name = gtk_buildable_get_name(GTK_BUILDABLE(scale));

	if (strcmp(name, "hscale_spacing") == 0)
		ma->spacing = value;
	else if (strcmp(name, "hscale_padding") == 0)
		ma->padding = value;
	else
		g_assert_not_reached();

	update_dynamic_widgets(ma);
	multiload_refresh(ma);
}


static void
combo_orientation_changed_cb (GtkComboBox *combo, MultiloadPlugin *ma)
{
	ma->orientation_policy = gtk_combo_box_get_active (combo);
	update_dynamic_widgets(ma);
	multiload_refresh(ma);
}


static void
fill_between_toggled_cb (GtkToggleButton *toggle, MultiloadPlugin *ma)
{
	ma->fill_between = gtk_toggle_button_get_active(toggle);
	multiload_refresh(ma);
}


static void
tooltip_style_changed_cb (GtkComboBox *combo, MultiloadPlugin *ma)
{
	guint i;

	// find graph index
	const gchar *name = gtk_buildable_get_name(GTK_BUILDABLE(combo));
	for (i=0; i<GRAPH_MAX; i++) {
		if (strcmp(tooltip_style_names[i], name) == 0)
			break;
	}
	g_assert(i < GRAPH_MAX);

	ma->graph_config[i].tooltip_style = gtk_combo_box_get_active(combo);
}


static void
dblclick_policy_changed_cb (GtkComboBox *combo, MultiloadPlugin *ma)
{
	guint i;

	// find graph index
	const gchar *name = gtk_buildable_get_name(GTK_BUILDABLE(combo));
	for (i=0; i<GRAPH_MAX; i++) {
		if (strcmp(dblclick_policy_names[i], name) == 0)
			break;
	}
	g_assert(i < GRAPH_MAX);

	ma->graph_config[i].dblclick_policy = gtk_combo_box_get_active(combo);

	update_dynamic_widgets(ma);
}


static void
dblclick_command_changed_cb (GtkEntry *entry, MultiloadPlugin *ma)
{
	guint i;

	// find graph index
	const gchar *name = gtk_buildable_get_name(GTK_BUILDABLE(entry));
	for (i=0; i<GRAPH_MAX; i++) {
		if (strcmp(dblclick_command_names[i], name) == 0)
			break;
	}
	g_assert(i < GRAPH_MAX);

	strncpy(ma->graph_config[i].dblclick_cmdline, gtk_entry_get_text(entry), sizeof(ma->graph_config[i].dblclick_cmdline));
}


static void
border_changed_cb (GtkSpinButton *spin, MultiloadPlugin *ma)
{
	guint i;

	// find graph index
	const gchar *name = gtk_buildable_get_name(GTK_BUILDABLE(spin));
	for (i=0; i<GRAPH_MAX; i++) {
		if (strcmp(spin_border_names[i], name) == 0)
			break;
	}
	g_assert(i < GRAPH_MAX);

	ma->graph_config[i].border_width = gtk_spin_button_get_value_as_int(spin);
	multiload_refresh(ma);
}

static void
color_set_cb (GtkColorButton *col, MultiloadPlugin *ma)
{
	guint i;
	guint graph_index;
	gboolean found = FALSE;

	// find graph index
	const gchar *name = gtk_buildable_get_name(GTK_BUILDABLE(col));
	for (graph_index=0; graph_index<GRAPH_MAX; graph_index++) {
		for (i=0; i<multiload_config_get_num_colors(graph_index); i++) {
			if (NULL == color_button_names[graph_index][i])
				break;
			if (strcmp(color_button_names[graph_index][i], name) == 0) {
				found = TRUE;
				break;
			}
		}
		if (found)
			break;
	}
	g_assert(graph_index < GRAPH_MAX);
	g_assert(found == TRUE);

	gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(col), &ma->graph_config[graph_index].colors[i]);

	//border color update needs refresh
	if (i == multiload_colors_get_extra_index(graph_index, EXTRA_COLOR_BORDER))
		multiload_refresh(ma);
}

static void
colorscheme_new_clicked_cb (GtkToolButton *tb, MultiloadPlugin *ma)
{
	printf("NEW (not yet implemented)\n");
	multiload_color_scheme_apply(&multiload_builtin_color_schemes[0], ma);
	multiload_fill_color_buttons(ma);
}

static void
colorscheme_delete_clicked_cb (GtkToolButton *tb, MultiloadPlugin *ma)
{
	printf("DELETE (not yet implemented)\n");
}

static void
colorscheme_save_clicked_cb (GtkToolButton *tb, MultiloadPlugin *ma)
{
	printf("SAVE (not yet implemented)\n");
}

static void
colorscheme_import_clicked_cb (GtkWidget *tb, MultiloadPlugin *ma)
{
	GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(tb));
	gchar *filename = gtk_open_file_dialog(parent, _("Import color scheme"));
	MultiloadColorSchemeStatus result = multiload_color_scheme_from_file(filename, ma);
	multiload_fill_color_buttons(ma);
	multiload_refresh(ma);

	printf("IMPORT = %d\n", result);
}

static void
colorscheme_export_clicked_cb (GtkWidget *tb, MultiloadPlugin *ma)
{
	GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(tb));
	gchar *filename = gtk_save_file_dialog(parent, _("Export color scheme"), "multiload-ng.colors");
	gboolean result = multiload_color_scheme_to_file(filename, ma);

	printf("EXPORT = %d\n", result);
}

static void
color_scheme_selected_cb (GtkTreeSelection *sel, MultiloadPlugin *ma)
{
	GList *rows = gtk_tree_selection_get_selected_rows(sel, NULL);
	if (rows == NULL)
		return;

	GtkTreePath *path = rows->data;
	gint *indices = gtk_tree_path_get_indices(path);

	if (indices != NULL) {
		printf("Selected color scheme #%d\n", indices[0]);
		multiload_color_scheme_apply(&multiload_builtin_color_schemes[indices[0]], ma);
		multiload_fill_color_buttons(ma);
		multiload_refresh(ma);
	}

	g_list_foreach (rows, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (rows);
}


static void
multiload_preferences_connect_signals (MultiloadPlugin *ma)
{
	// cannot use gtk_builder_connect_signals because this fails in panel plugins
	guint i, j;

	for (i=0; i<GRAPH_MAX; i++) {
		g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, spinbutton_size_names[i])), "output", G_CALLBACK(spinbutton_size_output_cb), ma);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, spinbutton_size_names[i])), "value-changed", G_CALLBACK(spinbutton_size_change_cb), ma);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, spinbutton_interval_names[i])), "output", G_CALLBACK(spinbutton_interval_output_cb), ma);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, spinbutton_interval_names[i])), "value-changed", G_CALLBACK(spinbutton_interval_change_cb), ma);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, checkbox_visibility_names[i])), "toggled", G_CALLBACK(prop_graph_visibility_cb), ma);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, checkbox_visibility_names[i])), "toggled", G_CALLBACK(prop_checkboxes_sensitive_cb), ma);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, button_advanced_names[i])), "clicked", G_CALLBACK(button_advanced_clicked_cb), ma);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, tooltip_style_names[i])), "changed", G_CALLBACK(tooltip_style_changed_cb), ma);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, dblclick_policy_names[i])), "changed", G_CALLBACK(dblclick_policy_changed_cb), ma);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, dblclick_command_names[i])), "changed", G_CALLBACK(dblclick_command_changed_cb), ma);
		g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, spin_border_names[i])), "value-changed", G_CALLBACK(border_changed_cb), ma);

		for (j=0; j < G_N_ELEMENTS(color_button_names[i]); j++) {
			if (NULL == color_button_names[i][j])
				break;
			g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, color_button_names[i][j])), "color-set", G_CALLBACK(color_set_cb), ma);
		}
	}
	g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, "cb_fill_between")), "toggled", G_CALLBACK(fill_between_toggled_cb), ma);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, "hscale_spacing")), "value-changed", G_CALLBACK(spacing_or_padding_changed_cb), ma);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, "hscale_padding")), "value-changed", G_CALLBACK(spacing_or_padding_changed_cb), ma);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, "combo_orientation")), "changed", G_CALLBACK(combo_orientation_changed_cb), ma);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, "tb_colorscheme_new")), "clicked", G_CALLBACK(colorscheme_new_clicked_cb), ma);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, "tb_colorscheme_delete")), "clicked", G_CALLBACK(colorscheme_delete_clicked_cb), ma);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, "tb_colorscheme_save")), "clicked", G_CALLBACK(colorscheme_save_clicked_cb), ma);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, "tb_colorscheme_import")), "clicked", G_CALLBACK(colorscheme_import_clicked_cb), ma);
	g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, "tb_colorscheme_export")), "clicked", G_CALLBACK(colorscheme_export_clicked_cb), ma);

	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(gtk_builder_get_object(builder, "treeview_colors")));
	g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(color_scheme_selected_cb), ma);

	g_signal_connect(G_OBJECT(gtk_builder_get_object(builder, "dialog_advanced")), "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), ma);
	g_signal_connect_swapped(G_OBJECT(gtk_builder_get_object(builder, "button_dialog_advanced_close")), "clicked", G_CALLBACK(gtk_widget_hide), G_OBJECT(gtk_builder_get_object(builder, "dialog_advanced")));

	g_debug("[preferences] Signals connected");
}


void
multiload_fill_color_buttons (MultiloadPlugin *ma)
{
	guint i, c;
	for (i=0; i<GRAPH_MAX; i++)
		for (c=0; c<multiload_config_get_num_colors(i); c++)
			gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(gtk_builder_get_object(builder, color_button_names[i][c])), &ma->graph_config[i].colors[c]);
}


// create the properties dialog and initialize it from current configuration
void
multiload_init_preferences (GtkWidget *dialog, MultiloadPlugin *ma)
{
	guint i;

	builder = gtk_builder_new();
	gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);
	gtk_builder_add_from_string (builder, binary_data_preferences_ui, -1, NULL);

	// init values
	for (i=0; i<GRAPH_MAX; i++) {
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, spinbutton_size_names[i])), ma->graph_config[i].size*1.00);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, spinbutton_interval_names[i])), ma->graph_config[i].interval*1.00);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, checkbox_visibility_names[i])), ma->graph_config[i].visible);
		gtk_entry_set_max_length(GTK_ENTRY(gtk_builder_get_object(builder, dblclick_command_names[i])), sizeof(ma->graph_config[i].dblclick_cmdline));
		gtk_combo_box_set_active (GTK_COMBO_BOX(gtk_builder_get_object(builder, tooltip_style_names[i])), ma->graph_config[i].tooltip_style);
		gtk_combo_box_set_active (GTK_COMBO_BOX(gtk_builder_get_object(builder, dblclick_policy_names[i])), ma->graph_config[i].dblclick_policy);
		gtk_entry_set_text (GTK_ENTRY(gtk_builder_get_object(builder, dblclick_command_names[i])), ma->graph_config[i].dblclick_cmdline);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, spin_border_names[i])), ma->graph_config[i].border_width*1.00);
	}
	gtk_range_set_value(GTK_RANGE(gtk_builder_get_object(builder, "hscale_spacing")), (gdouble)ma->spacing);
	gtk_range_set_value(GTK_RANGE(gtk_builder_get_object(builder, "hscale_padding")), (gdouble)ma->padding);
	gtk_combo_box_set_active (GTK_COMBO_BOX(gtk_builder_get_object(builder, "combo_orientation")), ma->orientation_policy);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "cb_fill_between")), ma->fill_between);

	multiload_fill_color_buttons(ma);

	GtkListStore *ls_colors = GTK_LIST_STORE(gtk_builder_get_object(builder, "liststore_colors"));
	for (i=0; multiload_builtin_color_schemes[i].name[0] != '\0'; i++) {
		gtk_list_store_insert_with_values(ls_colors, NULL, -1,
			0, multiload_builtin_color_schemes[i].name,
			1, i,
			-1
		);
	}

	update_dynamic_widgets(ma);


	// main window
	GtkWidget *mainwnd_vbox = GTK_WIDGET(gtk_builder_get_object(builder, "mainwnd_vbox"));
	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), mainwnd_vbox);

	multiload_preferences_connect_signals(ma);

	g_debug("[preferences] Initialized");
}

