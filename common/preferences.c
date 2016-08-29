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
#include "preferences.h"
#include "util.h"
#include "ui.h"

#ifdef MULTILOAD_EXPERIMENTAL
#include <errno.h>
#include "linux-proc.h"
#endif

static GtkBuilder *builder = NULL;

#define OB(name) (gtk_builder_get_object(builder, name))



static const gchar* checkbox_visibility_names[GRAPH_MAX] = {
	"cb_visible_cpu",
	"cb_visible_mem",
	"cb_visible_net",
	"cb_visible_swap",
	"cb_visible_load",
	"cb_visible_disk",
	"cb_visible_temp"
#ifdef MULTILOAD_EXPERIMENTAL
	,"cb_visible_parm"
#endif
};

static const gchar* button_advanced_names[GRAPH_MAX] = {
	"button_advanced_cpu",
	"button_advanced_mem",
	"button_advanced_net",
	"button_advanced_swap",
	"button_advanced_load",
	"button_advanced_disk",
	"button_advanced_temp"
#ifdef MULTILOAD_EXPERIMENTAL
	,"button_advanced_parm"
#endif
};

static const gchar* spinbutton_size_names[GRAPH_MAX] = {
	"sb_size_cpu",
	"sb_size_mem",
	"sb_size_net",
	"sb_size_swap",
	"sb_size_load",
	"sb_size_disk",
	"sb_size_temp"
#ifdef MULTILOAD_EXPERIMENTAL
	,"sb_size_parm"
#endif
};

static const gchar* spinbutton_interval_names[GRAPH_MAX] = {
	"sb_interval_cpu",
	"sb_interval_mem",
	"sb_interval_net",
	"sb_interval_swap",
	"sb_interval_load",
	"sb_interval_disk",
	"sb_interval_temp"
#ifdef MULTILOAD_EXPERIMENTAL
	,"sb_interval_parm"
#endif
};

static const gchar* label_timespan_names[GRAPH_MAX] = {
	"label_timespan_cpu",
	"label_timespan_mem",
	"label_timespan_net",
	"label_timespan_swap",
	"label_timespan_load",
	"label_timespan_disk",
	"label_timespan_temp"
#ifdef MULTILOAD_EXPERIMENTAL
	,"label_timespan_parm"
#endif
};

static const gchar* tooltip_style_names[GRAPH_MAX] = {
	"combo_tooltip_cpu",
	"combo_tooltip_mem",
	"combo_tooltip_net",
	"combo_tooltip_swap",
	"combo_tooltip_load",
	"combo_tooltip_disk",
	"combo_tooltip_temp"
#ifdef MULTILOAD_EXPERIMENTAL
	,"combo_tooltip_parm"
#endif
};

static const gchar* dblclick_policy_names[GRAPH_MAX] = {
	"combo_dblclick_cpu",
	"combo_dblclick_mem",
	"combo_dblclick_net",
	"combo_dblclick_swap",
	"combo_dblclick_load",
	"combo_dblclick_disk",
	"combo_dblclick_temp"
#ifdef MULTILOAD_EXPERIMENTAL
	,"combo_dblclick_parm"
#endif
};

static const gchar* dblclick_command_names[GRAPH_MAX] = {
	"entry_dblclick_command_cpu",
	"entry_dblclick_command_mem",
	"entry_dblclick_command_net",
	"entry_dblclick_command_swap",
	"entry_dblclick_command_load",
	"entry_dblclick_command_disk",
	"entry_dblclick_command_temp"
#ifdef MULTILOAD_EXPERIMENTAL
	,"entry_dblclick_command_parm"
#endif
};

static const gchar* info_dblclick_command_names[GRAPH_MAX] = {
	"image_info_dblclick_command_cpu",
	"image_info_dblclick_command_mem",
	"image_info_dblclick_command_net",
	"image_info_dblclick_command_swap",
	"image_info_dblclick_command_load",
	"image_info_dblclick_command_disk",
	"image_info_dblclick_command_temp"
#ifdef MULTILOAD_EXPERIMENTAL
	,"image_info_dblclick_command_parm"
#endif
};

static const gchar* spin_border_names[GRAPH_MAX] = {
	"sb_border_cpu",
	"sb_border_mem",
	"sb_border_net",
	"sb_border_swap",
	"sb_border_load",
	"sb_border_disk",
	"sb_border_temp"
#ifdef MULTILOAD_EXPERIMENTAL
	,"sb_border_parm"
#endif
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
#ifdef MULTILOAD_EXPERIMENTAL
	}, {
		"cb_color_parm1",
		"cb_color_parm_border",
		"cb_color_parm_bg1",
		"cb_color_parm_bg2",
		NULL
#endif
	}
};



static guint
multiload_preferences_get_graph_index (GtkWidget *widget, const gchar **list)
{
	guint i;
	const gchar *name = gtk_buildable_get_name(GTK_BUILDABLE(widget));

	for (i=0; i<GRAPH_MAX; i++) {
		if (strcmp(list[i], name) == 0)
			break;
	}
	g_assert(i < GRAPH_MAX);

	return i;
}

static void
multiload_preferences_color_scheme_select (gint index)
{
	GtkTreePath *path = gtk_tree_path_new_from_indices(index, -1);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(OB("treeview_colors")), path, NULL, FALSE);
	gtk_tree_path_free(path);
}

static void
multiload_preferences_color_scheme_select_custom ()
{
	// Custom color scheme is the last entry
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(OB("treeview_colors")));
	guint n = gtk_tree_model_iter_n_children(model, NULL);
	if (n > 0)
		multiload_preferences_color_scheme_select(n-1);
}

static void
multiload_preferences_update_dynamic_widgets(MultiloadPlugin *ma)
{
	guint i;
	GraphConfig *gc;

	for (i=0; i<GRAPH_MAX; i++) {
		gc = &ma->graph_config[i];

		// timespan
		gchar *timespan = format_time_duration(gc->size * gc->interval / 1000);
		gtk_label_set_text (GTK_LABEL(OB(label_timespan_names[i])), timespan);
		g_free(timespan);

		// cmdline enable
		gboolean cmdline_enabled = (gc->dblclick_policy == DBLCLICK_POLICY_CMDLINE);
		gtk_widget_set_sensitive (GTK_WIDGET(OB(dblclick_command_names[i])), cmdline_enabled);
		gtk_widget_set_visible (GTK_WIDGET(OB(info_dblclick_command_names[i])), cmdline_enabled);
	}

	// padding warning
	gtk_widget_set_visible(GTK_WIDGET(OB("image_warning_padding")), (ma->padding >= 10));

	// orientation warning
	gtk_widget_set_visible (GTK_WIDGET(OB("image_warning_orientation")),
		( ma->panel_orientation == GTK_ORIENTATION_HORIZONTAL && ma->orientation_policy == MULTILOAD_ORIENTATION_VERTICAL) ||
		( ma->panel_orientation == GTK_ORIENTATION_VERTICAL &&   ma->orientation_policy == MULTILOAD_ORIENTATION_HORIZONTAL)
	);
}



static void
multiload_preferences_checkboxes_sensitive_cb (GtkToggleButton *checkbox, gpointer user_data)
{
	guint i;

	// Count the number of visible graphs
	gint visible_count = 0;
	gint last_graph = 0;
	gboolean active = gtk_toggle_button_get_active(checkbox);

	if (!active) {
		for (i = 0; i < GRAPH_MAX; i++) {
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(OB(checkbox_visibility_names[i])))) {
				last_graph = i;
				visible_count ++;
			}
		}
	}

	if ( visible_count < 2 ) {
		if (active) {
			// Enable all checkboxes
			for (i = 0; i < GRAPH_MAX; i++)
				gtk_widget_set_sensitive(GTK_WIDGET(OB(checkbox_visibility_names[i])), TRUE);
		} else {
			// Disable last remaining checkbox
			gtk_widget_set_sensitive(GTK_WIDGET(OB(checkbox_visibility_names[last_graph])), FALSE);
		}
	}

	return;
}

static void
multiload_preferences_graph_visibility_cb (GtkToggleButton *checkbox, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_WIDGET(checkbox), checkbox_visibility_names);
	gboolean active = gtk_toggle_button_get_active(checkbox);

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
multiload_preferences_button_advanced_clicked_cb (GtkWidget *button, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_WIDGET(button), button_advanced_names);

	GtkWidget *dialog_config = GTK_WIDGET(OB("dialog_advanced"));
	gtk_window_set_transient_for(GTK_WINDOW(dialog_config), GTK_WINDOW(gtk_widget_get_toplevel(button)));

	GtkWidget *notebook = GTK_WIDGET(OB("advanced_notebook"));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), i);
	gtk_widget_show(dialog_config);
}

static gint
multiload_preferences_size_output_cb (GtkSpinButton *spin, gpointer p)
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
multiload_preferences_size_change_cb (GtkSpinButton *spin, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_WIDGET(spin), spinbutton_size_names);

	ma->graph_config[i].size = gtk_spin_button_get_value_as_int(spin);

	load_graph_resize(ma->graphs[i]);
	multiload_preferences_update_dynamic_widgets(ma);
}

static gint
multiload_preferences_interval_output_cb (GtkSpinButton *spin, gpointer p)
{
	const gchar *format = _("%d milliseconds");

	static int tooltip_timeout = -1;
	if (tooltip_timeout == -1)
		g_object_get(gtk_settings_get_default(), "gtk-tooltip-timeout", &tooltip_timeout, NULL);

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
multiload_preferences_interval_change_cb (GtkSpinButton *spin, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_WIDGET(spin), spinbutton_interval_names);

	ma->graph_config[i].interval = gtk_spin_button_get_value_as_int(spin);

	load_graph_stop(ma->graphs[i]);
	if (ma->graph_config[i].visible)
		load_graph_start(ma->graphs[i]);

	multiload_preferences_update_dynamic_widgets(ma);
}

static void
multiload_preferences_spacing_or_padding_changed_cb (GtkRange *scale, MultiloadPlugin *ma)
{
	guint value = (guint)gtk_range_get_value(scale);
	const gchar *name = gtk_buildable_get_name(GTK_BUILDABLE(scale));

	if (strcmp(name, "hscale_spacing") == 0)
		ma->spacing = value;
	else if (strcmp(name, "hscale_padding") == 0)
		ma->padding = value;
	else
		g_assert_not_reached();

	multiload_preferences_update_dynamic_widgets(ma);
	multiload_refresh(ma);
}

static void
multiload_preferences_orientation_changed_cb (GtkComboBox *combo, MultiloadPlugin *ma)
{
	ma->orientation_policy = gtk_combo_box_get_active (combo);
	multiload_preferences_update_dynamic_widgets(ma);
	multiload_refresh(ma);
}

static void
multiload_preferences_fill_between_toggled_cb (GtkToggleButton *toggle, MultiloadPlugin *ma)
{
	ma->fill_between = gtk_toggle_button_get_active(toggle);
	multiload_refresh(ma);
}

static void
multiload_preferences_tooltip_style_changed_cb (GtkComboBox *combo, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_WIDGET(combo), tooltip_style_names);

	ma->graph_config[i].tooltip_style = gtk_combo_box_get_active(combo);
}

static void
multiload_preferences_dblclick_policy_changed_cb (GtkComboBox *combo, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_WIDGET(combo), dblclick_policy_names);

	ma->graph_config[i].dblclick_policy = gtk_combo_box_get_active(combo);
	multiload_preferences_update_dynamic_widgets(ma);
}

static void
multiload_preferences_dblclick_command_changed_cb (GtkEntry *entry, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_WIDGET(entry), dblclick_command_names);

	strncpy(ma->graph_config[i].dblclick_cmdline, gtk_entry_get_text(entry), sizeof(ma->graph_config[i].dblclick_cmdline));
}

static void
multiload_preferences_border_changed_cb (GtkSpinButton *spin, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_WIDGET(spin), spin_border_names);

	ma->graph_config[i].border_width = gtk_spin_button_get_value_as_int(spin);
	multiload_refresh(ma);
}

static void
multiload_preferences_color_set_cb (GtkColorButton *col, MultiloadPlugin *ma)
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

	// border color update needs refresh
	if (i == multiload_colors_get_extra_index(graph_index, EXTRA_COLOR_BORDER))
		multiload_refresh(ma);

	// every color-set event changes the color scheme to (Custom)
	multiload_preferences_color_scheme_select_custom();
}

static void
multiload_preferences_colorscheme_import_clicked_cb (GtkWidget *tb, MultiloadPlugin *ma)
{
	int response;
	char *filename;
	MultiloadColorSchemeStatus result;

	GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(tb));
	GtkWidget *dialog = gtk_file_chooser_dialog_new (_("Import color scheme"), parent,
										GTK_FILE_CHOOSER_ACTION_OPEN,
										_("_Cancel"), GTK_RESPONSE_CANCEL,
										_("_Open"), GTK_RESPONSE_ACCEPT,
										NULL);

	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog), MULTILOAD_CONFIG_PATH);

	response = gtk_dialog_run (GTK_DIALOG (dialog));

	if (response == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		result = multiload_color_scheme_from_file(filename, ma);

		switch(result) {
			case MULTILOAD_COLOR_SCHEME_STATUS_VALID:
				multiload_preferences_update_color_buttons(ma);
				multiload_preferences_color_scheme_select_custom();
				multiload_refresh(ma);
				break;
			case MULTILOAD_COLOR_SCHEME_STATUS_WRONG_FORMAT:
				gtk_error_dialog(parent, _("Color scheme format is incorrect. Unable to import."));
				break;
			case MULTILOAD_COLOR_SCHEME_STATUS_WRONG_VERSION:
				gtk_error_dialog(parent, _("Color scheme was created by an incompatible version of Multiload-ng. Unable to import."));
				break;
		}
	}

	gtk_widget_destroy (dialog);
}

static void
multiload_preferences_colorscheme_export_clicked_cb (GtkWidget *tb, MultiloadPlugin *ma)
{
	int response;
	char *filename;
	gboolean result;

	GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(tb));
	GtkWidget *dialog = gtk_file_chooser_dialog_new (_("Export color scheme"), parent,
										GTK_FILE_CHOOSER_ACTION_SAVE,
										_("_Cancel"), GTK_RESPONSE_CANCEL,
										_("_Save"), GTK_RESPONSE_ACCEPT,
										NULL);

	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "multiload-ng.colors");
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog), MULTILOAD_CONFIG_PATH);

	response = gtk_dialog_run (GTK_DIALOG (dialog));

	if (response == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		result = multiload_color_scheme_to_file(filename, ma);
		if (!result)
			gtk_error_dialog(parent, _("Error exporting color scheme."));
	}

	gtk_widget_destroy (dialog);
}

static void
multiload_preferences_color_scheme_selected_cb (GtkTreeSelection *sel, MultiloadPlugin *ma)
{
	const MultiloadColorScheme *scheme;
	GList *rows;
	GtkTreePath *path;
	gint *ii;

	rows = gtk_tree_selection_get_selected_rows(sel, NULL);
	if (rows == NULL)
		return;

	path = rows->data;
	ii = gtk_tree_path_get_indices(path);

	if (ii != NULL) {
		scheme = &multiload_builtin_color_schemes[ii[0]];

		if (scheme->name[0] == '\0') {
			strncpy(ma->color_scheme, "-", sizeof(ma->color_scheme));
		} else {
			strncpy(ma->color_scheme, scheme->name, sizeof(ma->color_scheme));
			multiload_color_scheme_apply(scheme, ma);
			multiload_preferences_update_color_buttons(ma);
			multiload_refresh(ma);
		}
	}

	g_list_foreach (rows, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (rows);
}

#ifdef MULTILOAD_EXPERIMENTAL
static void
multiload_preferences_parm_command_changed_cb (GtkEntry *entry, MultiloadPlugin *ma)
{
	ParametricData *xd = (ParametricData*)ma->graphs[GRAPH_PARAMETRIC]->extra_data;
	strncpy(xd->command, gtk_entry_get_text(entry), sizeof(xd->command));
}

static void
multiload_preferences_parm_command_test_clicked_cb (GtkWidget *button, MultiloadPlugin *ma)
{
	gchar *stdout, *stderr;
	gint exit_status;

	ParametricData *xd = (ParametricData*)ma->graphs[GRAPH_PARAMETRIC]->extra_data;

	if (xd->command[0] == '\0') {
		//TODO better message
		gtk_error_dialog(GTK_WINDOW(gtk_widget_get_toplevel(button)), _("Command line is empty."));
		return;
	}

	gboolean spawn_success= g_spawn_command_line_sync (xd->command, &stdout, &stderr, &exit_status, NULL);
	if (!spawn_success) {
		//TODO better message
		gtk_error_dialog(GTK_WINDOW(gtk_widget_get_toplevel(button)), _("Unable to execute command."));
		return;
	} else if (exit_status != 0) {
		//TODO printf!
		gtk_error_dialog(GTK_WINDOW(gtk_widget_get_toplevel(button)), _("Command returned exit code %d.")/*, exit_status*/);
		return;
	}

	errno = 0;
	g_ascii_strtoull (stdout, NULL, 0);
	if (errno != 0) {
		//TODO better message
		gtk_error_dialog(GTK_WINDOW(gtk_widget_get_toplevel(button)), _("Command did not return a valid number."));
		return;
	} else {
		//TODO this is not an error!
		gtk_error_dialog(GTK_WINDOW(gtk_widget_get_toplevel(button)), _("Command line is valid."));
	}

}
#endif

static void
multiload_preferences_destroy ()
{
	if (builder == NULL)
		return;

	// top level windows must be explicitly destroyed
	gtk_widget_destroy(GTK_WIDGET(OB("dialog_advanced")));
	g_object_unref (G_OBJECT (builder));
	builder = NULL;
}

static void
multiload_preferences_init ()
{
	if (builder != NULL)
		multiload_preferences_destroy();

	builder = gtk_builder_new();
	gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);
	gtk_builder_add_from_string (builder, binary_data_preferences_ui, -1, NULL);
}

static void
multiload_preferences_connect_signals (MultiloadPlugin *ma)
{
	// cannot use gtk_builder_connect_signals because this fails in panel plugins
	guint i, j;

	for (i=0; i<GRAPH_MAX; i++) {
		g_signal_connect(G_OBJECT(OB(spinbutton_size_names[i])), "output", G_CALLBACK(multiload_preferences_size_output_cb), ma);
		g_signal_connect(G_OBJECT(OB(spinbutton_size_names[i])), "value-changed", G_CALLBACK(multiload_preferences_size_change_cb), ma);
		g_signal_connect(G_OBJECT(OB(spinbutton_interval_names[i])), "output", G_CALLBACK(multiload_preferences_interval_output_cb), ma);
		g_signal_connect(G_OBJECT(OB(spinbutton_interval_names[i])), "value-changed", G_CALLBACK(multiload_preferences_interval_change_cb), ma);
		g_signal_connect(G_OBJECT(OB(checkbox_visibility_names[i])), "toggled", G_CALLBACK(multiload_preferences_graph_visibility_cb), ma);
		g_signal_connect(G_OBJECT(OB(checkbox_visibility_names[i])), "toggled", G_CALLBACK(multiload_preferences_checkboxes_sensitive_cb), ma);
		g_signal_connect(G_OBJECT(OB(button_advanced_names[i])), "clicked", G_CALLBACK(multiload_preferences_button_advanced_clicked_cb), ma);
		g_signal_connect(G_OBJECT(OB(tooltip_style_names[i])), "changed", G_CALLBACK(multiload_preferences_tooltip_style_changed_cb), ma);
		g_signal_connect(G_OBJECT(OB(dblclick_policy_names[i])), "changed", G_CALLBACK(multiload_preferences_dblclick_policy_changed_cb), ma);
		g_signal_connect(G_OBJECT(OB(dblclick_command_names[i])), "changed", G_CALLBACK(multiload_preferences_dblclick_command_changed_cb), ma);
		g_signal_connect(G_OBJECT(OB(spin_border_names[i])), "value-changed", G_CALLBACK(multiload_preferences_border_changed_cb), ma);

		for (j=0; j < G_N_ELEMENTS(color_button_names[i]); j++) {
			if (NULL == color_button_names[i][j])
				break;
			g_signal_connect(G_OBJECT(OB(color_button_names[i][j])), "color-set", G_CALLBACK(multiload_preferences_color_set_cb), ma);
		}
	}
	g_signal_connect(G_OBJECT(OB("cb_fill_between")), "toggled", G_CALLBACK(multiload_preferences_fill_between_toggled_cb), ma);
	g_signal_connect(G_OBJECT(OB("hscale_spacing")), "value-changed", G_CALLBACK(multiload_preferences_spacing_or_padding_changed_cb), ma);
	g_signal_connect(G_OBJECT(OB("hscale_padding")), "value-changed", G_CALLBACK(multiload_preferences_spacing_or_padding_changed_cb), ma);
	g_signal_connect(G_OBJECT(OB("combo_orientation")), "changed", G_CALLBACK(multiload_preferences_orientation_changed_cb), ma);
	g_signal_connect(G_OBJECT(OB("tb_colorscheme_import")), "clicked", G_CALLBACK(multiload_preferences_colorscheme_import_clicked_cb), ma);
	g_signal_connect(G_OBJECT(OB("tb_colorscheme_export")), "clicked", G_CALLBACK(multiload_preferences_colorscheme_export_clicked_cb), ma);

#ifdef MULTILOAD_EXPERIMENTAL
	g_signal_connect(G_OBJECT(OB("entry_parm_command")), "changed", G_CALLBACK(multiload_preferences_parm_command_changed_cb), ma);
	g_signal_connect(G_OBJECT(OB("button_parm_command_test")), "clicked", G_CALLBACK(multiload_preferences_parm_command_test_clicked_cb), ma);
#endif
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(OB("treeview_colors")));
	g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(multiload_preferences_color_scheme_selected_cb), ma);

	g_signal_connect(G_OBJECT(OB("dialog_advanced")), "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), ma);
	g_signal_connect_swapped(G_OBJECT(OB("button_dialog_advanced_close")), "clicked", G_CALLBACK(gtk_widget_hide), G_OBJECT(OB("dialog_advanced")));

	g_debug("[preferences] Signals connected");
}

void
multiload_preferences_update_color_buttons (MultiloadPlugin *ma)
{
	guint i, c;
	for (i=0; i<GRAPH_MAX; i++)
		for (c=0; c<multiload_config_get_num_colors(i); c++)
			gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(OB(color_button_names[i][c])), &ma->graph_config[i].colors[c]);
}


void
multiload_preferences_fill_dialog (GtkWidget *dialog, MultiloadPlugin *ma)
{
	guint i;
	gboolean color_scheme_is_set = FALSE;

	multiload_preferences_init();

	// init values
	for (i=0; i<GRAPH_MAX; i++) {
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(OB(spinbutton_size_names[i])), ma->graph_config[i].size*1.00);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(OB(spinbutton_interval_names[i])), ma->graph_config[i].interval*1.00);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(OB(checkbox_visibility_names[i])), ma->graph_config[i].visible);
		gtk_entry_set_max_length(GTK_ENTRY(OB(dblclick_command_names[i])), sizeof(ma->graph_config[i].dblclick_cmdline));
		gtk_combo_box_set_active (GTK_COMBO_BOX(OB(tooltip_style_names[i])), ma->graph_config[i].tooltip_style);
		gtk_combo_box_set_active (GTK_COMBO_BOX(OB(dblclick_policy_names[i])), ma->graph_config[i].dblclick_policy);
		gtk_entry_set_text (GTK_ENTRY(OB(dblclick_command_names[i])), ma->graph_config[i].dblclick_cmdline);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(OB(spin_border_names[i])), ma->graph_config[i].border_width*1.00);
	}
	gtk_range_set_value(GTK_RANGE(OB("hscale_spacing")), (gdouble)ma->spacing);
	gtk_range_set_value(GTK_RANGE(OB("hscale_padding")), (gdouble)ma->padding);
	gtk_combo_box_set_active (GTK_COMBO_BOX(OB("combo_orientation")), ma->orientation_policy);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(OB("cb_fill_between")), ma->fill_between);

#ifdef MULTILOAD_EXPERIMENTAL
	gtk_entry_set_text(GTK_ENTRY(OB("entry_parm_command")), ((ParametricData*)ma->graphs[GRAPH_PARAMETRIC]->extra_data)->command);
#endif

	multiload_preferences_update_color_buttons(ma);

	// color schemes list
	GtkListStore *ls_colors = GTK_LIST_STORE(OB("liststore_colors"));
	for (i=0; multiload_builtin_color_schemes[i].name[0] != '\0'; i++) {
		const gchar *name = multiload_builtin_color_schemes[i].name;

		// insert color scheme
		gtk_list_store_insert_with_values(ls_colors, NULL, -1, 0, name, 1, i, -1 );

		// if it's the current color scheme, select it
		if (strcmp(ma->color_scheme, name) == 0) {
			multiload_preferences_color_scheme_select(i);
			color_scheme_is_set = TRUE;
		}
	}
	// insert (Custom) entry
	gtk_list_store_insert_with_values(ls_colors, NULL, -1, 0, _("(Custom)"), 1, i, -1 );

	// no current color scheme, select last entry (Custom)
	if (!color_scheme_is_set)
		multiload_preferences_color_scheme_select_custom();

	multiload_preferences_update_dynamic_widgets(ma);

	// main window
	GtkWidget *mainwnd_vbox = GTK_WIDGET(OB("mainwnd_vbox"));
	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), mainwnd_vbox);

	multiload_preferences_connect_signals(ma);

#ifndef MULTILOAD_EXPERIMENTAL
	gtk_widget_hide(GTK_WIDGET(OB("table_parm_command")));
#endif
	g_debug("[preferences] Initialized");
}

