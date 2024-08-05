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
#include <errno.h>
#include <math.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>

#include "binary-data.h"
#include "colors.h"
#include "gtk-compat.h"
#include "graph-data.h"
#include "load-graph.h"
#include "multiload.h"
#include "multiload-config.h"
#include "preferences.h"
#include "util.h"
#include "ui.h"


static GtkBuilder *builder = NULL;
static gboolean _orientation_warning_disable = FALSE;

#define OB(name) (gtk_builder_get_object(builder, name))
#define EMBED_GRAPH_INDEX(ob,i) g_object_set_data(G_OBJECT(ob), "graph-index", GUINT_TO_POINTER(i))
#define EXTRACT_GRAPH_INDEX(ob) GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(ob), "graph-index"))
#define DEFINE_OB_NAMES_FULL(p) static const gchar* (p##_names)[GRAPH_MAX] = { #p "_cpu", #p "_mem", #p "_net", #p "_swap", #p "_load", #p "_disk", #p "_temp", #p "_bat", #p "_parm" }


DEFINE_OB_NAMES_FULL(cb_visible);
DEFINE_OB_NAMES_FULL(button_advanced);
DEFINE_OB_NAMES_FULL(sb_size);
DEFINE_OB_NAMES_FULL(sb_interval);
DEFINE_OB_NAMES_FULL(combo_tooltip);
DEFINE_OB_NAMES_FULL(combo_dblclick);
DEFINE_OB_NAMES_FULL(entry_dblclick_command);
DEFINE_OB_NAMES_FULL(image_info_dblclick_command);
DEFINE_OB_NAMES_FULL(sb_border);
DEFINE_OB_NAMES_FULL(advanced_box);
DEFINE_OB_NAMES_FULL(draw_color_bgpreview);
DEFINE_OB_NAMES_FULL(button_gradient);

static const gchar* spin_ceil_names[GRAPH_MAX] = {
	"",
	"",
	"sb_ceil_net",
	"",
	"sb_ceil_load",
	"sb_ceil_disk",
	"sb_ceil_temp",
	"",
	"sb_ceil_parm"
};

static const gchar* cb_autoscaler_names[GRAPH_MAX] = {
	"",
	"",
	"cb_autoscaler_net",
	"",
	"cb_autoscaler_load",
	"cb_autoscaler_disk",
	"cb_autoscaler_temp",
	"",
	"cb_autoscaler_parm"
};

static const gchar* cb_source_auto_names[GRAPH_MAX] = {
	"",
	"",
	"cb_source_auto_net",
	"",
	"",
	"cb_source_auto_disk",
	"cb_source_auto_temp",
	"",
	""
};

static const gchar* treeview_source_names[GRAPH_MAX] = {
	"",
	"",
	"treeview_source_net",
	"",
	"",
	"treeview_source_disk",
	"treeview_source_temp",
	"",
	""
};

static const gchar* cellrenderertoggle_source_names[GRAPH_MAX] = {
	"",
	"",
	"cellrenderertoggle_source_net",
	"",
	"",
	"cellrenderertoggle_source_disk",
	"cellrenderertoggle_source_temp",
	"",
	""
};

static const gchar* liststore_source_names[GRAPH_MAX] = {
	"",
	"",
	"liststore_source_net",
	"",
	"",
	"liststore_source_disk",
	"liststore_source_temp",
	"",
	""
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
		"cb_color_temp2",
		"cb_color_temp_border",
		"cb_color_temp_bg1",
		"cb_color_temp_bg2",
		NULL
	}, {
		"cb_color_bat1",
		"cb_color_bat2",
		"cb_color_bat3",
		"cb_color_bat_border",
		"cb_color_bat_bg1",
		"cb_color_bat_bg2",
		NULL
	}, {
		"cb_color_parm1",
		"cb_color_parm2",
		"cb_color_parm3",
		"cb_color_parm4",
		"cb_color_parm_border",
		"cb_color_parm_bg1",
		"cb_color_parm_bg2"
	}
};

enum {
	LS_SOURCE_COLUMN_SELECTED	= 0,
	LS_SOURCE_COLUMN_LABEL		= 1,
	LS_SOURCE_COLUMN_ABSENT		= 2,
	LS_SOURCE_COLUMN_DATA		= 3
};

enum {
	LS_COLORS_COLUMN_NAME		= 0,
	LS_COLORS_COLUMN_ICON		= 1,
	LS_COLORS_COLUMN_SEPARATOR	= 2
};

static guint
multiload_preferences_get_graph_index (GtkBuildable *ob, const gchar **list)
{
	guint i;
	const gchar *name = gtk_buildable_get_name(ob);

	for (i=0; i<GRAPH_MAX; i++) {
		if (strcmp(list[i], name) == 0)
			break;
	}
	g_assert_cmpuint(i, <, GRAPH_MAX);

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
	GraphConfig *conf;

	for (i=0; i<GRAPH_MAX; i++) {
		conf = &ma->graph_config[i];

		// cmdline enable
		gboolean cmdline_enabled = (conf->dblclick_policy == DBLCLICK_POLICY_CMDLINE);
		gtk_widget_set_sensitive (GTK_WIDGET(OB(entry_dblclick_command_names[i])), cmdline_enabled);
		gtk_widget_set_visible (GTK_WIDGET(OB(image_info_dblclick_command_names[i])), cmdline_enabled);

		// autoscaler
		if (strcmp(cb_autoscaler_names[i], "") != 0) {
			gtk_widget_set_sensitive(GTK_WIDGET(OB(spin_ceil_names[i])),
					!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(OB(cb_autoscaler_names[i]))));
		}

		// filter
		if (strcmp(cb_source_auto_names[i], "") != 0) {
			gtk_widget_set_sensitive(GTK_WIDGET(OB(treeview_source_names[i])),
					!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(OB(cb_source_auto_names[i]))));
		}
	}

	// padding warning
	gtk_widget_set_visible(GTK_WIDGET(OB("image_warning_padding")), (ma->padding >= 10));

	// orientation warning
	gtk_widget_set_visible (GTK_WIDGET(OB("image_warning_orientation")),
		!_orientation_warning_disable &&
		(( ma->panel_orientation == GTK_ORIENTATION_HORIZONTAL && ma->orientation_policy == MULTILOAD_ORIENTATION_VERTICAL) ||
		( ma->panel_orientation == GTK_ORIENTATION_VERTICAL &&   ma->orientation_policy == MULTILOAD_ORIENTATION_HORIZONTAL))
	);
}



static void
multiload_preferences_checkboxes_sensitive_cb (GtkToggleButton *checkbox, gpointer user_data)
{
	guint i;

	// Count the number of visible graphs
	gint visible_count = 0;
	gint last_graph = 0;
	gboolean active = FALSE;
	if (checkbox != NULL)
		active = gtk_toggle_button_get_active(checkbox);

	if (!active) {
		for (i = 0; i < GRAPH_MAX; i++) {
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(OB(cb_visible_names[i])))) {
				last_graph = i;
				visible_count ++;
			}
		}
	}

	if ( visible_count < 2 ) {
		if (active) {
			// Enable all checkboxes
			for (i = 0; i < GRAPH_MAX; i++)
				gtk_widget_set_sensitive(GTK_WIDGET(OB(cb_visible_names[i])), TRUE);
		} else {
			// Disable last remaining checkbox
			gtk_widget_set_sensitive(GTK_WIDGET(OB(cb_visible_names[last_graph])), FALSE);
		}
	}

	return;
}

static void
multiload_preferences_graph_visibility_cb (GtkToggleButton *checkbox, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_BUILDABLE(checkbox), cb_visible_names);
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
	guint i = multiload_preferences_get_graph_index(GTK_BUILDABLE(button), button_advanced_names);

	GtkWidget *dialog_config = GTK_WIDGET(OB("dialog_advanced"));
	gtk_window_set_transient_for(GTK_WINDOW(dialog_config), GTK_WINDOW(gtk_widget_get_toplevel(button)));

	GtkNotebook *notebook = GTK_NOTEBOOK(OB("advanced_notebook"));

	gtk_widget_show(dialog_config);

	/* From GTK+ 2/3 Reference Manual:
	Note that due to historical reasons, GtkNotebook refuses to switch to a
	page unless the child widget is visible. Therefore, it is recommended
	to show child widgets before adding them to a notebook. */
	gtk_notebook_set_current_page(notebook, i);
}

static gint
multiload_preferences_size_input_cb (GtkSpinButton *spin, double *new_value, gpointer user_data)
{
	const gchar *format = _("%d pixel");
	int value;

	const gchar *text = gtk_entry_get_text(GTK_ENTRY(spin));

	if (sscanf(text, format, &value) == 1) {
		*new_value = (double)value;
		return TRUE;
	} else if (sscanf(text, "%d", &value) == 1) {
		*new_value = (double)value;
		return TRUE;
	} else {
		return GTK_INPUT_ERROR;
	}
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
	guint i = multiload_preferences_get_graph_index(GTK_BUILDABLE(spin), sb_size_names);

	ma->graph_config[i].size = gtk_spin_button_get_value_as_int(spin);

	load_graph_resize(ma->graphs[i]);
	multiload_preferences_update_dynamic_widgets(ma);
}

static gint
multiload_preferences_interval_input_cb (GtkSpinButton *spin, double *new_value, gpointer user_data)
{
	const gchar *format = _("%d milliseconds");
	int value;

	const gchar *text = gtk_entry_get_text(GTK_ENTRY(spin));

	if (sscanf(text, format, &value) == 1) {
		*new_value = (double)value;
		return TRUE;
	} else if (sscanf(text, "%d", &value) == 1) {
		*new_value = (double)value;
		return TRUE;
	} else {
		return GTK_INPUT_ERROR;
	}
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

	if (n <= tooltip_timeout)
		gtk_entry_set_icon_from_icon_name (GTK_ENTRY(spin), GTK_ENTRY_ICON_SECONDARY, "dialog-warning");
	else
		gtk_entry_set_icon_from_icon_name (GTK_ENTRY(spin), GTK_ENTRY_ICON_SECONDARY, NULL);

	// block the default output
	return TRUE;
}

static void
multiload_preferences_interval_change_cb (GtkSpinButton *spin, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_BUILDABLE(spin), sb_interval_names);

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

	if (strcmp(name, "hscale_spacing") == 0) {
		ma->spacing = value;
		multiload_set_spacing(ma, value);
	} else if (strcmp(name, "hscale_padding") == 0) {
		ma->padding = value;
		multiload_set_padding(ma, value);
	} else
		g_assert_not_reached();

	multiload_preferences_update_dynamic_widgets(ma);
}

static void
multiload_preferences_orientation_changed_cb (GtkComboBox *combo, MultiloadPlugin *ma)
{
	ma->orientation_policy = gtk_combo_box_get_active (combo);
	multiload_preferences_update_dynamic_widgets(ma);
	multiload_refresh_orientation(ma);
}

static void
multiload_preferences_iec_units_toggled_cb (GtkToggleButton *toggle, MultiloadPlugin *ma)
{
	ma->size_format_iec = gtk_toggle_button_get_active(toggle);
}

static void
multiload_preferences_tooltip_style_changed_cb (GtkComboBox *combo, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_BUILDABLE(combo), combo_tooltip_names);

	ma->graph_config[i].tooltip_style = gtk_combo_box_get_active(combo);
}

static void
multiload_preferences_dblclick_policy_changed_cb (GtkComboBox *combo, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_BUILDABLE(combo), combo_dblclick_names);

	ma->graph_config[i].dblclick_policy = gtk_combo_box_get_active(combo);
	multiload_preferences_update_dynamic_widgets(ma);
}

static void
multiload_preferences_dblclick_command_changed_cb (GtkEntry *entry, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_BUILDABLE(entry), entry_dblclick_command_names);

	strncpy(ma->graph_config[i].dblclick_cmdline, gtk_entry_get_text(entry), sizeof(ma->graph_config[i].dblclick_cmdline));
}

static void
multiload_preferences_border_changed_cb (GtkSpinButton *spin, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_BUILDABLE(spin), sb_border_names);
	guint value = gtk_spin_button_get_value_as_int(spin);

	ma->graph_config[i].border_width = value;
	gtk_widget_queue_draw(GTK_WIDGET(OB(draw_color_bgpreview_names[i])));
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
	g_assert_cmpuint(graph_index, <, GRAPH_MAX);
	g_assert(found == TRUE);

	gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(col), &ma->graph_config[graph_index].colors[i]);

	// every color-set event changes the color scheme to (Custom)
	multiload_preferences_color_scheme_select_custom();

	gtk_widget_queue_draw(GTK_WIDGET(OB(draw_color_bgpreview_names[graph_index])));
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
				break;
			case MULTILOAD_COLOR_SCHEME_STATUS_WRONG_FORMAT:
				show_modal_info_dialog(parent, GTK_MESSAGE_ERROR, _("Color scheme format is incorrect. Unable to import."));
				break;
			case MULTILOAD_COLOR_SCHEME_STATUS_WRONG_VERSION:
				show_modal_info_dialog(parent, GTK_MESSAGE_ERROR, _("Color scheme was created by an incompatible version of Multiload-ng. Unable to import."));
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
			show_modal_info_dialog(parent, GTK_MESSAGE_ERROR, _("Error exporting color scheme."));
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
		}
	}

	g_list_foreach (rows, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (rows);
}

static void
multiload_preferences_mem_slab_changed_cb (GtkComboBox *combo, MultiloadPlugin *ma)
{
	MemoryData *xd = (MemoryData*)ma->extra_data[GRAPH_MEMLOAD];
	xd->procps_compliant = (gtk_combo_box_get_active (combo) == 1);
}

static void
multiload_preferences_parm_command_changed_cb (GtkEntry *entry, MultiloadPlugin *ma)
{
	ParametricData *xd = (ParametricData*)ma->extra_data[GRAPH_PARAMETRIC];
	strncpy(xd->command, gtk_entry_get_text(entry), sizeof(xd->command));
}

static void
multiload_preferences_parm_command_test_clicked_cb (GtkWidget *button, MultiloadPlugin *ma)
{
	ParametricData *xd = (ParametricData*)ma->extra_data[GRAPH_PARAMETRIC];
	// test command line and fill structure with result
	multiload_graph_parm_get_data(0, NULL, NULL, xd, FALSE);

	GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(button));

	if (xd->error == TRUE) {
		show_modal_info_dialog(parent, GTK_MESSAGE_ERROR, xd->message);
	} else {
		g_snprintf(xd->message, sizeof(xd->message), _("Command line is valid. Retrieved %d numbers."), xd->nvalues);
		show_modal_info_dialog(parent, GTK_MESSAGE_INFO, xd->message);
	}
}

static void
multiload_preferences_autoscaler_toggled_cb (GtkToggleButton *toggle, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_BUILDABLE(toggle), cb_autoscaler_names);
	AutoScaler *scaler = multiload_get_scaler(ma, i);
	if (scaler == NULL)
		return;

	gboolean enable = gtk_toggle_button_get_active(toggle);
	autoscaler_set_enabled(scaler, enable);

	if (!enable) {
		// "Automatic" disabled; copy last automatic max to spin button
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(OB(spin_ceil_names[i])),
			autoscaler_get_max(scaler, ma->graphs[i], 0));
	}

	multiload_preferences_update_dynamic_widgets(ma);
}

static void
multiload_preferences_ceil_changed_cb (GtkSpinButton *spin, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_BUILDABLE(spin), spin_ceil_names);
	AutoScaler *scaler = multiload_get_scaler(ma, i);
	if (scaler == NULL)
		return;

	int value = gtk_spin_button_get_value_as_int(spin);
	autoscaler_set_max(scaler, value);
}

static gint
multiload_preferences_ceil_input_cb (GtkSpinButton *spin, double *new_value, LoadGraph *g)
{
	gchar *format = g_strdup_printf("%%d %s", graph_types[g->id].output_unit);
	int value;

	const gchar *text = gtk_entry_get_text(GTK_ENTRY(spin));

	if (sscanf(text, format, &value) == 1) {
		*new_value = (double)value;
		return TRUE;
	} else if (sscanf(text, "%d", &value) == 1) {
		*new_value = (double)value;
		return TRUE;
	} else {
		return GTK_INPUT_ERROR;
	}
}

static gint
multiload_preferences_ceil_output_cb (GtkSpinButton *spin, LoadGraph *g)
{
	gint n = gtk_spin_button_get_value_as_int(spin);
	gchar *s = g_strdup_printf("%d %s", n, graph_types[g->id].output_unit);
	gtk_entry_set_text(GTK_ENTRY(spin), s);
	g_free(s);

	// block the default output
	return TRUE;
}

static void
multiload_preferences_source_toggled_cb (GtkCellRendererToggle *cell, gchar *path_string, MultiloadPlugin *ma)
{
	GtkListStore *ls;
	GtkTreePath *path;
	GtkTreeIter iter;
	gboolean active;
	gboolean b;
	gchar *s;
	MultiloadFilter *filter;

	guint graph_index = EXTRACT_GRAPH_INDEX(cell);
	if (liststore_source_names[graph_index] == NULL)
		return; // filter not implemented for this graph

	ls = GTK_LIST_STORE(OB(liststore_source_names[graph_index]));
	active = gtk_cell_renderer_toggle_get_active(cell);

	// radio button behavior
	if (gtk_cell_renderer_toggle_get_radio(cell)) {
		if (active)
			return; // selecting already selected radio buttons - do nothing

		// deselect every radio button
		b = gtk_tree_model_get_iter_first (GTK_TREE_MODEL(ls), &iter);
		while (b) {
			gtk_list_store_set (ls, &iter, 0, FALSE, -1);
			b = gtk_tree_model_iter_next (GTK_TREE_MODEL(ls), &iter);
		}
	}

	// select current entry
	path = gtk_tree_path_new_from_string(path_string);
	gtk_tree_model_get_iter (GTK_TREE_MODEL(ls), &iter, path);
	gtk_list_store_set (ls, &iter, 0, !active, -1);
	gtk_tree_path_free (path);

	// update filter
	filter = multiload_filter_new();

	b = gtk_tree_model_get_iter_first (GTK_TREE_MODEL(ls), &iter);
	while (b) {
		gtk_tree_model_get (GTK_TREE_MODEL(ls), &iter,
			LS_SOURCE_COLUMN_SELECTED,	&b,
			LS_SOURCE_COLUMN_DATA,		&s,
		-1);

		if (b)
			multiload_filter_append(filter, s);

		g_free(s);
		b = gtk_tree_model_iter_next (GTK_TREE_MODEL(ls), &iter);
	}
	multiload_filter_export(filter, ma->graph_config[graph_index].filter, sizeof(ma->graph_config[graph_index].filter));
	g_debug ("[preferences] set filter for graph #%d: %s\n", graph_index, ma->graph_config[graph_index].filter);

	// trigger data refresh for graphs that require it
	ma->graphs[graph_index]->filter_changed = TRUE;
}

static void
multiload_preferences_source_auto_toggled_cb (GtkToggleButton *toggle, MultiloadPlugin *ma)
{
	guint i = multiload_preferences_get_graph_index(GTK_BUILDABLE(toggle), cb_source_auto_names);
	gboolean b = gtk_toggle_button_get_active(toggle);

	ma->graph_config[i].filter_enable = !b;

	// trigger data refresh for graphs that require it
	ma->graphs[i]->filter_changed = TRUE;

	multiload_preferences_update_dynamic_widgets(ma);
}


#ifdef MULTILOAD_DEVELOPER_MODE

#define _CPRINT(c, buf) sprintf(buf, "HEX_TO_RGBA(%02X%02X%02X, %02X)", (guint8)(255*c.red), (guint8)(255*c.green), (guint8)(255*c.blue), (guint8)(255*c.alpha));

static void
multiload_preferences_dev_colorscheme_generate_clicked_cb (GtkToolButton *btn, MultiloadPlugin *ma)
{
	char buf[30];
	printf("\t{ \"COLORSCHEME_NAME\", NULL /*color_scheme_icon_xpm*/,\n");

	printf("\t\t\t{  { // CPU\n");
	_CPRINT(ma->graph_config[GRAPH_CPULOAD].colors[0], buf);
	printf("\t\t\t\t%s,\t\t// User\n", buf);
	_CPRINT(ma->graph_config[GRAPH_CPULOAD].colors[1], buf);
	printf("\t\t\t\t%s,\t\t// System\n", buf);
	_CPRINT(ma->graph_config[GRAPH_CPULOAD].colors[2], buf);
	printf("\t\t\t\t%s,\t\t// Nice\n", buf);
	_CPRINT(ma->graph_config[GRAPH_CPULOAD].colors[3], buf);
	printf("\t\t\t\t%s,\t\t// IOWait\n", buf);
	_CPRINT(ma->graph_config[GRAPH_CPULOAD].colors[4], buf);
	printf("\t\t\t\t%s,\t\t// Border\n", buf);
	_CPRINT(ma->graph_config[GRAPH_CPULOAD].colors[5], buf);
	printf("\t\t\t\t%s,\t\t// Background (top)\n", buf);
	_CPRINT(ma->graph_config[GRAPH_CPULOAD].colors[6], buf);
	printf("\t\t\t\t%s\t\t\t// Background (bottom)\n", buf);

	printf("\t\t\t}, { // MEM\n");
	_CPRINT(ma->graph_config[GRAPH_MEMLOAD].colors[0], buf);
	printf("\t\t\t\t%s,\t\t// User\n", buf);
	_CPRINT(ma->graph_config[GRAPH_MEMLOAD].colors[1], buf);
	printf("\t\t\t\t%s,\t\t// Shared\n", buf);
	_CPRINT(ma->graph_config[GRAPH_MEMLOAD].colors[2], buf);
	printf("\t\t\t\t%s,\t\t// Buffers\n", buf);
	_CPRINT(ma->graph_config[GRAPH_MEMLOAD].colors[3], buf);
	printf("\t\t\t\t%s,\t\t// Cached\n", buf);
	_CPRINT(ma->graph_config[GRAPH_MEMLOAD].colors[4], buf);
	printf("\t\t\t\t%s,\t\t// Border\n", buf);
	_CPRINT(ma->graph_config[GRAPH_MEMLOAD].colors[5], buf);
	printf("\t\t\t\t%s,\t\t// Background (top)\n", buf);
	_CPRINT(ma->graph_config[GRAPH_MEMLOAD].colors[6], buf);
	printf("\t\t\t\t%s\t\t\t// Background (bottom)\n", buf);

	printf("\t\t\t}, { // NET\n");
	_CPRINT(ma->graph_config[GRAPH_NETLOAD].colors[0], buf);
	printf("\t\t\t\t%s,\t\t// In\n", buf);
	_CPRINT(ma->graph_config[GRAPH_NETLOAD].colors[1], buf);
	printf("\t\t\t\t%s,\t\t// Out\n", buf);
	_CPRINT(ma->graph_config[GRAPH_NETLOAD].colors[2], buf);
	printf("\t\t\t\t%s,\t\t// Local\n", buf);
	_CPRINT(ma->graph_config[GRAPH_NETLOAD].colors[3], buf);
	printf("\t\t\t\t%s,\t\t// Border\n", buf);
	_CPRINT(ma->graph_config[GRAPH_NETLOAD].colors[4], buf);
	printf("\t\t\t\t%s,\t\t// Background (top)\n", buf);
	_CPRINT(ma->graph_config[GRAPH_NETLOAD].colors[5], buf);
	printf("\t\t\t\t%s\t\t\t// Background (bottom)\n", buf);

	printf("\t\t\t}, { // SWAP\n");
	_CPRINT(ma->graph_config[GRAPH_SWAPLOAD].colors[0], buf);
	printf("\t\t\t\t%s,\t\t// Used\n", buf);
	_CPRINT(ma->graph_config[GRAPH_SWAPLOAD].colors[1], buf);
	printf("\t\t\t\t%s,\t\t// Border\n", buf);
	_CPRINT(ma->graph_config[GRAPH_SWAPLOAD].colors[2], buf);
	printf("\t\t\t\t%s,\t\t// Background (top)\n", buf);
	_CPRINT(ma->graph_config[GRAPH_SWAPLOAD].colors[3], buf);
	printf("\t\t\t\t%s\t\t\t// Background (bottom)\n", buf);

	printf("\t\t\t}, { // LOAD\n");
	_CPRINT(ma->graph_config[GRAPH_LOADAVG].colors[0], buf);
	printf("\t\t\t\t%s,\t\t// Average\n", buf);
	_CPRINT(ma->graph_config[GRAPH_LOADAVG].colors[1], buf);
	printf("\t\t\t\t%s,\t\t// Border\n", buf);
	_CPRINT(ma->graph_config[GRAPH_LOADAVG].colors[2], buf);
	printf("\t\t\t\t%s,\t\t// Background (top)\n", buf);
	_CPRINT(ma->graph_config[GRAPH_LOADAVG].colors[3], buf);
	printf("\t\t\t\t%s\t\t\t// Background (bottom)\n", buf);

	printf("\t\t\t}, { // DISK\n");
	_CPRINT(ma->graph_config[GRAPH_DISKLOAD].colors[0], buf);
	printf("\t\t\t\t%s,\t\t// Read\n", buf);
	_CPRINT(ma->graph_config[GRAPH_DISKLOAD].colors[1], buf);
	printf("\t\t\t\t%s,\t\t// Write\n", buf);
	_CPRINT(ma->graph_config[GRAPH_DISKLOAD].colors[2], buf);
	printf("\t\t\t\t%s,\t\t// Border\n", buf);
	_CPRINT(ma->graph_config[GRAPH_DISKLOAD].colors[3], buf);
	printf("\t\t\t\t%s,\t\t// Background (top)\n", buf);
	_CPRINT(ma->graph_config[GRAPH_DISKLOAD].colors[4], buf);
	printf("\t\t\t\t%s\t\t\t// Background (bottom)\n", buf);

	printf("\t\t\t}, { // TEMP\n");
	_CPRINT(ma->graph_config[GRAPH_TEMPERATURE].colors[0], buf);
	printf("\t\t\t\t%s,\t\t// Value\n", buf);
	_CPRINT(ma->graph_config[GRAPH_TEMPERATURE].colors[1], buf);
	printf("\t\t\t\t%s,\t\t// Critical\n", buf);
	_CPRINT(ma->graph_config[GRAPH_TEMPERATURE].colors[2], buf);
	printf("\t\t\t\t%s,\t\t// Border\n", buf);
	_CPRINT(ma->graph_config[GRAPH_TEMPERATURE].colors[3], buf);
	printf("\t\t\t\t%s,\t\t// Background (top)\n", buf);
	_CPRINT(ma->graph_config[GRAPH_TEMPERATURE].colors[4], buf);
	printf("\t\t\t\t%s\t\t\t// Background (bottom)\n", buf);

	printf("\t\t\t}, { // BAT\n");
	_CPRINT(ma->graph_config[GRAPH_BATTERY].colors[0], buf);
	printf("\t\t\t\t%s,\t\t// Charging\n", buf);
	_CPRINT(ma->graph_config[GRAPH_BATTERY].colors[1], buf);
	printf("\t\t\t\t%s,\t\t// Discharging\n", buf);
	_CPRINT(ma->graph_config[GRAPH_BATTERY].colors[2], buf);
	printf("\t\t\t\t%s,\t\t// Critical level\n", buf);
	_CPRINT(ma->graph_config[GRAPH_BATTERY].colors[3], buf);
	printf("\t\t\t\t%s,\t\t// Border\n", buf);
	_CPRINT(ma->graph_config[GRAPH_BATTERY].colors[4], buf);
	printf("\t\t\t\t%s,\t\t// Background (top)\n", buf);
	_CPRINT(ma->graph_config[GRAPH_BATTERY].colors[5], buf);
	printf("\t\t\t\t%s\t\t\t// Background (bottom)\n", buf);

	printf("\t\t\t}, { // PARM\n");
	_CPRINT(ma->graph_config[GRAPH_PARAMETRIC].colors[0], buf);
	printf("\t\t\t\t%s,\t\t// Result 1\n", buf);
	_CPRINT(ma->graph_config[GRAPH_PARAMETRIC].colors[1], buf);
	printf("\t\t\t\t%s,\t\t// Result 2\n", buf);
	_CPRINT(ma->graph_config[GRAPH_PARAMETRIC].colors[2], buf);
	printf("\t\t\t\t%s,\t\t// Result 3\n", buf);
	_CPRINT(ma->graph_config[GRAPH_PARAMETRIC].colors[3], buf);
	printf("\t\t\t\t%s,\t\t// Result 4\n", buf);
	_CPRINT(ma->graph_config[GRAPH_PARAMETRIC].colors[4], buf);
	printf("\t\t\t\t%s,\t\t// Border\n", buf);
	_CPRINT(ma->graph_config[GRAPH_PARAMETRIC].colors[5], buf);
	printf("\t\t\t\t%s,\t\t// Background (top)\n", buf);
	_CPRINT(ma->graph_config[GRAPH_PARAMETRIC].colors[6], buf);
	printf("\t\t\t\t%s\t\t\t// Background (bottom)\n", buf);

	printf("\t\t\t}\n");
	printf("\t\t}\n");
	printf("\t},\n");
}


static void
multiload_developer_buttons(MultiloadPlugin *ma)
{
	GtkToolbar *toolbar = GTK_TOOLBAR(OB("toolbar_colors"));
	GtkToolItem *button;

	// separator
	button = gtk_separator_tool_item_new();
	gtk_separator_tool_item_set_draw (GTK_SEPARATOR_TOOL_ITEM(button), FALSE);
	gtk_tool_item_set_expand (button, TRUE);
	gtk_widget_show(GTK_WIDGET(button));
	gtk_toolbar_insert(toolbar, button, -1);

	// Generate C code for current color scheme
	button = gtk_tool_button_new(NULL, NULL);
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button), "insert-object");
	gtk_tool_item_set_tooltip_text (button, "MULTILOAD_DEVELOPER_MODE: generate C code for current color scheme");
	gtk_widget_show(GTK_WIDGET(button));
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(multiload_preferences_dev_colorscheme_generate_clicked_cb), ma);
	gtk_toolbar_insert(toolbar, button, -1);

}
#else

static void multiload_developer_buttons() {}

#endif

static gboolean
multiload_preferences_bgpreview_draw_cb(GtkWidget *widget, cairo_t *cr, LoadGraph *g)
{
	guint i, total;
	gdouble step, remainder;
	GdkRGBA *c_top, *c_bottom, *c_border;
	GdkRGBA *colors = g->config->colors;
	GtkAllocation alloc;

	gtk_widget_get_allocation(widget, &alloc);

	guint x = 0;
	guint y = 0;
	guint W = alloc.width;
	guint H = alloc.height;

	c_top = &colors[multiload_colors_get_extra_index(g->id, EXTRA_COLOR_BACKGROUND_TOP)];
	c_bottom = &colors[multiload_colors_get_extra_index(g->id, EXTRA_COLOR_BACKGROUND_BOTTOM)];
	c_border = &colors[multiload_colors_get_extra_index(g->id, EXTRA_COLOR_BORDER)];

	// border
	if (g->config->border_width > 0) {
		cairo_set_source_rgba(cr, c_border->red, c_border->green, c_border->blue, c_border->alpha);
		cairo_rectangle(cr, 0, 0, W, H);
		cairo_fill(cr);

		if ((guint)(2*g->config->border_width) < W)
			W -= 2*g->config->border_width;
		else
			W=0;

		if ((guint)(2*g->config->border_width) < H)
			H -= 2*g->config->border_width;
		else
			H=0;

		x = g->config->border_width;
		y = g->config->border_width;
	}

	if (W > 0 && H > 0) {
		// background
		load_graph_cairo_set_gradient(cr, W, H, g->config->bg_direction, c_top, c_bottom);
		cairo_rectangle(cr, x, y, W, H);
		cairo_fill(cr);

		// data example
		total = multiload_config_get_num_data(g->id);
		step = floor(H/total);
		remainder = H - step*total;
		for (i = 0; i < total; i++) {
			cairo_set_source_rgba(cr, colors[i].red, colors[i].green, colors[i].blue, colors[i].alpha);
			cairo_rectangle(cr,
				x + ((W%2)? 1+(W-1)/2:W/2),
				y + ((i==total-1) ? 0 : H - (i+1)*step),
				W/2,
				step + ((i==total-1) ? remainder : 0)
			);
			cairo_fill(cr);
		}
	}
	return FALSE;
}

#if GTK_API == 2
static gboolean
multiload_preferences_bgpreview_expose(GtkWidget *widget, GdkEventExpose *event, LoadGraph *g)
{
	cairo_t *cr = gdk_cairo_create (event->window);
	multiload_preferences_bgpreview_draw_cb(widget, cr, g);
	cairo_destroy (cr);
	return FALSE;
}
#endif

static void multiload_preferences_gradient_toggled_cb (GtkToggleButton *button, MultiloadPlugin *ma)
{
	if (gtk_toggle_button_get_active(button) == FALSE)
		return;
	guint graph_index = EXTRACT_GRAPH_INDEX(button);
	ma->graph_config[graph_index].bg_direction = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "bg-direction"));
	gtk_widget_queue_draw(GTK_WIDGET(OB(draw_color_bgpreview_names[graph_index])));
	gtk_dialog_response(GTK_DIALOG(g_object_get_data(G_OBJECT(button), "bg-dialog")), 0);
}

static void
multiload_preferences_button_gradient_clicked_cb (GtkWidget *button, MultiloadPlugin *ma)
{
	static const gchar *labels[MULTILOAD_GRADIENT_MAX] = { "↓", "↙", "←", "↖", "↑", "↗", "→", "↘", "◎" }; //TODO C-escape these
	GtkWidget *buttons[MULTILOAD_GRADIENT_MAX];

	guint graph_index = multiload_preferences_get_graph_index(GTK_BUILDABLE(button), button_gradient_names);
	guint i;

	GtkWidget *dialog = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog), _("Direction of gradient"));

	for (i=0; i<MULTILOAD_GRADIENT_MAX; i++) {
		if (i==0)
			buttons[i] = gtk_radio_button_new(NULL);
		else
			buttons[i] = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(buttons[0]));

		g_object_set(G_OBJECT(buttons[i]), "draw-indicator", FALSE, NULL);
		g_signal_connect(G_OBJECT(buttons[i]), "toggled", G_CALLBACK(multiload_preferences_gradient_toggled_cb), ma);
		g_object_set_data(G_OBJECT(buttons[i]), "bg-direction", GINT_TO_POINTER(i));
		g_object_set_data(G_OBJECT(buttons[i]), "bg-dialog", dialog);
		EMBED_GRAPH_INDEX(buttons[i], graph_index);

		gtk_container_add(GTK_CONTAINER(buttons[i]), gtk_label_new(labels[i]));
	}
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(buttons[ma->graph_config[graph_index].bg_direction]), TRUE);

	GtkWidget *content_area = gtk_dialog_get_content_area (GTK_DIALOG(dialog));
	gtk_container_add(GTK_CONTAINER(content_area), gtk_label_new(graph_types[graph_index].label));

#if GTK_API == 2
	GtkWidget *table = gtk_table_new (3, 3, 1);

	gtk_table_attach_defaults (GTK_TABLE(table), buttons[MULTILOAD_GRADIENT_LINEAR_N_TO_S],		1, 2, 2, 3);
	gtk_table_attach_defaults (GTK_TABLE(table), buttons[MULTILOAD_GRADIENT_LINEAR_NE_TO_SW],	0, 1, 2, 3);
	gtk_table_attach_defaults (GTK_TABLE(table), buttons[MULTILOAD_GRADIENT_LINEAR_E_TO_W],		0, 1, 1, 2);
	gtk_table_attach_defaults (GTK_TABLE(table), buttons[MULTILOAD_GRADIENT_LINEAR_SE_TO_NW],	0, 1, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE(table), buttons[MULTILOAD_GRADIENT_LINEAR_S_TO_N],		1, 2, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE(table), buttons[MULTILOAD_GRADIENT_LINEAR_SW_TO_NE],	2, 3, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE(table), buttons[MULTILOAD_GRADIENT_LINEAR_W_TO_E],		2, 3, 1, 2);
	gtk_table_attach_defaults (GTK_TABLE(table), buttons[MULTILOAD_GRADIENT_LINEAR_NW_TO_SE],	2, 3, 2, 3);
	gtk_table_attach_defaults (GTK_TABLE(table), buttons[MULTILOAD_GRADIENT_RADIAL],			1, 2, 1, 2);

	gtk_container_add(GTK_CONTAINER(content_area), table);
#else
	GtkWidget *grid = gtk_grid_new();

	gtk_grid_attach (GTK_GRID(grid), buttons[MULTILOAD_GRADIENT_LINEAR_N_TO_S],		1, 2, 1, 1);
	gtk_grid_attach (GTK_GRID(grid), buttons[MULTILOAD_GRADIENT_LINEAR_NE_TO_SW],	0, 2, 1, 1);
	gtk_grid_attach (GTK_GRID(grid), buttons[MULTILOAD_GRADIENT_LINEAR_E_TO_W],		0, 1, 1, 1);
	gtk_grid_attach (GTK_GRID(grid), buttons[MULTILOAD_GRADIENT_LINEAR_SE_TO_NW],	0, 0, 1, 1);
	gtk_grid_attach (GTK_GRID(grid), buttons[MULTILOAD_GRADIENT_LINEAR_S_TO_N],		1, 0, 1, 1);
	gtk_grid_attach (GTK_GRID(grid), buttons[MULTILOAD_GRADIENT_LINEAR_SW_TO_NE],	2, 0, 1, 1);
	gtk_grid_attach (GTK_GRID(grid), buttons[MULTILOAD_GRADIENT_LINEAR_W_TO_E],		2, 1, 1, 1);
	gtk_grid_attach (GTK_GRID(grid), buttons[MULTILOAD_GRADIENT_LINEAR_NW_TO_SE],	2, 2, 1, 1);
	gtk_grid_attach (GTK_GRID(grid), buttons[MULTILOAD_GRADIENT_RADIAL],			1, 1, 1, 1);

	gtk_container_add(GTK_CONTAINER(content_area), grid);
#endif

	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gtk_widget_get_toplevel(button)));
	gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_UTILITY);
	gtk_widget_show_all(dialog);

/*	int x, y, x_root, y_root;
	gdk_window_get_root_origin(gtk_widget_get_window(button), &x_root, &y_root);
	gtk_widget_translate_coordinates(button, gtk_widget_get_toplevel(button), 0, 0, &x, &y);
	gtk_window_move(GTK_WINDOW(dialog), x_root+x, y_root+y);
*/

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

static void
multiload_preferences_reorder_set_order_from_tree_view(MultiloadPlugin *ma)
{
	GtkTreeModel *tree_model = GTK_TREE_MODEL(OB("liststore_reorder"));
	GtkTreeIter iter;
	gboolean valid;
	gint i = 0;

	for (valid = gtk_tree_model_get_iter_first(tree_model, &iter); valid == TRUE; valid = gtk_tree_model_iter_next(tree_model, &iter)) {
		if (i >= GRAPH_MAX)
			g_error("Array out of bounds during graph reordering");
		gtk_tree_model_get(tree_model, &iter, 1, &ma->graph_order[i++], -1);
	}

	multiload_set_order (ma, ma->graph_order);
}

static void
multiload_preferences_reorder_row_deleted_cb (GtkTreeModel *tree_model, GtkTreePath *path, MultiloadPlugin *ma)
{
	multiload_preferences_reorder_set_order_from_tree_view (ma);
}

static void
multiload_preferences_reorder_up_clicked_cb (GtkToolButton *button, MultiloadPlugin *ma)
{
	gint i, s;
	gint new_order[GRAPH_MAX];

	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(OB("treeview_reorder")));
	GList *list = gtk_tree_selection_get_selected_rows (sel, NULL);

	if (list == NULL)
		return;

	GtkTreePath *path = list->data;
	gint *indices = gtk_tree_path_get_indices (path);
	s = indices[0];
	gtk_tree_path_free(path);

	if (s == 0)
		return;

	for (i=0; i<GRAPH_MAX; i++) {
		if (i == s)
			new_order[i] = i-1;
		else if (i == s-1)
			new_order[i] = i+1;
		else
			new_order[i] = i;
	}

	gtk_list_store_reorder (GTK_LIST_STORE(OB("liststore_reorder")), new_order);
	multiload_preferences_reorder_set_order_from_tree_view (ma);
}

static void
multiload_preferences_reorder_down_clicked_cb (GtkToolButton *button, MultiloadPlugin *ma)
{
	gint i, s;
	gint new_order[GRAPH_MAX];

	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(OB("treeview_reorder")));
	GList *list = gtk_tree_selection_get_selected_rows (sel, NULL);

	if (list == NULL)
		return;

	GtkTreePath *path = list->data;
	gint *indices = gtk_tree_path_get_indices (path);
	s = indices[0];
	gtk_tree_path_free(path);

	if (s == GRAPH_MAX-1)
		return;

	for (i=0; i<GRAPH_MAX; i++) {
		if (i == s)
			new_order[i] = i+1;
		else if (i == s+1)
			new_order[i] = i-1;
		else
			new_order[i] = i;
	}

	gtk_list_store_reorder (GTK_LIST_STORE(OB("liststore_reorder")), new_order);
	multiload_preferences_reorder_set_order_from_tree_view (ma);
}

static void
multiload_preferences_reorder_reset_clicked_cb (GtkToolButton *button, MultiloadPlugin *ma)
{
	GtkTreeModel *tree_model = GTK_TREE_MODEL(OB("liststore_reorder"));
	GtkTreeIter iter;
	gboolean valid;
	gint n;

	gint i;
	gint new_order[GRAPH_MAX];

	for (i=0, valid = gtk_tree_model_get_iter_first(tree_model, &iter); valid == TRUE; i++, valid = gtk_tree_model_iter_next(tree_model, &iter)) {
		gtk_tree_model_get(tree_model, &iter, 1, &n, -1);
		new_order[n] = i;
	}

	gtk_list_store_reorder (GTK_LIST_STORE(OB("liststore_reorder")), new_order);

	multiload_preferences_reorder_set_order_from_tree_view (ma);
}

gboolean
multiload_preferences_color_scheme_row_separator_func (GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	gboolean sep;
	gtk_tree_model_get(model, iter, LS_COLORS_COLUMN_SEPARATOR, &sep, -1);
	return sep;
}


static void
multiload_preferences_destroy ()
{
	if (builder == NULL)
		return;

	// top level windows must be explicitly destroyed
	gtk_widget_destroy(GTK_WIDGET(OB("dialog_advanced")));
	g_clear_object (&builder);
}

static void
multiload_preferences_init ()
{
	GError *error = NULL;

	if (builder != NULL)
		multiload_preferences_destroy();

	builder = gtk_builder_new();
	gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);
	gtk_builder_add_from_string (builder, binary_data_preferences_ui, -1, &error);

	if (error != NULL) {
		g_error ("Unable load Preferences UI: %s", error->message);
		g_error_free (error);
	}
}

static void
multiload_preferences_connect_signals (MultiloadPlugin *ma)
{
	// cannot use gtk_builder_connect_signals because this fails in panel plugins
	guint i, j;

	for (i=0; i<GRAPH_MAX; i++) {
		g_signal_connect(G_OBJECT(OB(sb_size_names[i])), "input", G_CALLBACK(multiload_preferences_size_input_cb), ma);
		g_signal_connect(G_OBJECT(OB(sb_size_names[i])), "output", G_CALLBACK(multiload_preferences_size_output_cb), ma);
		g_signal_connect(G_OBJECT(OB(sb_size_names[i])), "value-changed", G_CALLBACK(multiload_preferences_size_change_cb), ma);
		g_signal_connect(G_OBJECT(OB(sb_interval_names[i])), "input", G_CALLBACK(multiload_preferences_interval_input_cb), ma);
		g_signal_connect(G_OBJECT(OB(sb_interval_names[i])), "output", G_CALLBACK(multiload_preferences_interval_output_cb), ma);
		g_signal_connect(G_OBJECT(OB(sb_interval_names[i])), "value-changed", G_CALLBACK(multiload_preferences_interval_change_cb), ma);
		g_signal_connect(G_OBJECT(OB(cb_visible_names[i])), "toggled", G_CALLBACK(multiload_preferences_graph_visibility_cb), ma);
		g_signal_connect(G_OBJECT(OB(cb_visible_names[i])), "toggled", G_CALLBACK(multiload_preferences_checkboxes_sensitive_cb), ma);
		g_signal_connect(G_OBJECT(OB(button_advanced_names[i])), "clicked", G_CALLBACK(multiload_preferences_button_advanced_clicked_cb), ma);
		g_signal_connect(G_OBJECT(OB(combo_tooltip_names[i])), "changed", G_CALLBACK(multiload_preferences_tooltip_style_changed_cb), ma);
		g_signal_connect(G_OBJECT(OB(combo_dblclick_names[i])), "changed", G_CALLBACK(multiload_preferences_dblclick_policy_changed_cb), ma);
		g_signal_connect(G_OBJECT(OB(entry_dblclick_command_names[i])), "changed", G_CALLBACK(multiload_preferences_dblclick_command_changed_cb), ma);
		g_signal_connect(G_OBJECT(OB(sb_border_names[i])), "value-changed", G_CALLBACK(multiload_preferences_border_changed_cb), ma);

		// autoscaler
		if (cb_autoscaler_names[i][0] != '\0') {
			g_signal_connect(G_OBJECT(OB(cb_autoscaler_names[i])), "toggled", G_CALLBACK(multiload_preferences_autoscaler_toggled_cb), ma);
			g_signal_connect(G_OBJECT(OB(spin_ceil_names[i])), "input", G_CALLBACK(multiload_preferences_ceil_input_cb), ma->graphs[i]);
			g_signal_connect(G_OBJECT(OB(spin_ceil_names[i])), "output", G_CALLBACK(multiload_preferences_ceil_output_cb), ma->graphs[i]);
			g_signal_connect(G_OBJECT(OB(spin_ceil_names[i])), "value-changed", G_CALLBACK(multiload_preferences_ceil_changed_cb), ma);
		}

		// filter
		if (cellrenderertoggle_source_names[i][0] != '\0') {
			g_signal_connect(G_OBJECT(OB(cb_source_auto_names[i])), "toggled", G_CALLBACK(multiload_preferences_source_auto_toggled_cb), ma);
			g_signal_connect(G_OBJECT(OB(cellrenderertoggle_source_names[i])), "toggled", G_CALLBACK(multiload_preferences_source_toggled_cb), ma);
			EMBED_GRAPH_INDEX(OB(cellrenderertoggle_source_names[i]), i);
		}

		// color buttons
		for (j=0; j < G_N_ELEMENTS(color_button_names[i]); j++) {
			if (NULL == color_button_names[i][j])
				break;
			g_signal_connect(G_OBJECT(OB(color_button_names[i][j])), "color-set", G_CALLBACK(multiload_preferences_color_set_cb), ma);
		}

		// background preview
		#if GTK_API == 2
			g_signal_connect (OB(draw_color_bgpreview_names[i]), "expose_event", G_CALLBACK (multiload_preferences_bgpreview_expose), ma->graphs[i]);
		#elif GTK_API == 3
			g_signal_connect (OB(draw_color_bgpreview_names[i]), "draw", G_CALLBACK (multiload_preferences_bgpreview_draw_cb), ma->graphs[i]);
		#endif

		// background gradient button
		g_signal_connect(G_OBJECT(OB(button_gradient_names[i])), "clicked", G_CALLBACK(multiload_preferences_button_gradient_clicked_cb), ma);
	}

	g_signal_connect(G_OBJECT(OB("cb_iec_units")), "toggled", G_CALLBACK(multiload_preferences_iec_units_toggled_cb), ma);
	g_signal_connect(G_OBJECT(OB("hscale_spacing")), "value-changed", G_CALLBACK(multiload_preferences_spacing_or_padding_changed_cb), ma);
	g_signal_connect(G_OBJECT(OB("hscale_padding")), "value-changed", G_CALLBACK(multiload_preferences_spacing_or_padding_changed_cb), ma);
	g_signal_connect(G_OBJECT(OB("combo_orientation")), "changed", G_CALLBACK(multiload_preferences_orientation_changed_cb), ma);

	// Memory graph
	g_signal_connect(G_OBJECT(OB("combo_mem_slab")), "changed", G_CALLBACK(multiload_preferences_mem_slab_changed_cb), ma);

	// Parametric graph
	g_signal_connect(G_OBJECT(OB("entry_parm_command")), "changed", G_CALLBACK(multiload_preferences_parm_command_changed_cb), ma);
	g_signal_connect(G_OBJECT(OB("button_parm_command_test")), "clicked", G_CALLBACK(multiload_preferences_parm_command_test_clicked_cb), ma);

	// Color schemes
	g_signal_connect(G_OBJECT(OB("tb_colorscheme_import")), "clicked", G_CALLBACK(multiload_preferences_colorscheme_import_clicked_cb), ma);
	g_signal_connect(G_OBJECT(OB("tb_colorscheme_export")), "clicked", G_CALLBACK(multiload_preferences_colorscheme_export_clicked_cb), ma);
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(OB("treeview_colors")));
	g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(multiload_preferences_color_scheme_selected_cb), ma);

	// Graph order
	g_signal_connect(G_OBJECT(OB("liststore_reorder")), "row-deleted", G_CALLBACK(multiload_preferences_reorder_row_deleted_cb), ma);
	g_signal_connect(G_OBJECT(OB("toolbar_reorder_btn_up")), "clicked", G_CALLBACK(multiload_preferences_reorder_up_clicked_cb), ma);
	g_signal_connect(G_OBJECT(OB("toolbar_reorder_btn_down")), "clicked", G_CALLBACK(multiload_preferences_reorder_down_clicked_cb), ma);
	g_signal_connect(G_OBJECT(OB("toolbar_reorder_btn_reset")), "clicked", G_CALLBACK(multiload_preferences_reorder_reset_clicked_cb), ma);

	// Dialog
	g_signal_connect(G_OBJECT(OB("dialog_advanced")), "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), ma);
	g_signal_connect_swapped(G_OBJECT(OB("button_dialog_advanced_close")), "clicked", G_CALLBACK(gtk_widget_hide), G_OBJECT(OB("dialog_advanced")));

	g_debug("[preferences] Signals connected");
}

void
multiload_preferences_update_color_buttons (MultiloadPlugin *ma)
{
	guint i, c;
	for (i=0; i<GRAPH_MAX; i++) {
		for (c=0; c<multiload_config_get_num_colors(i); c++)
			gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(OB(color_button_names[i][c])), &ma->graph_config[i].colors[c]);
		gtk_widget_queue_draw(GTK_WIDGET(OB(draw_color_bgpreview_names[i])));
	}
}

void
multiload_preferences_add_infobar (GtkMessageType message_type, const gchar *text)
{
	GtkWidget *info_bar = gtk_info_bar_new();
	gtk_info_bar_set_message_type (GTK_INFO_BAR(info_bar), message_type);

	GtkWidget *content_area = gtk_info_bar_get_content_area (GTK_INFO_BAR(info_bar));

	GtkWidget *label = gtk_label_new(text);
	gtk_label_set_xalign (GTK_LABEL(label), 0);
	gtk_container_add(GTK_CONTAINER(content_area), label);

	gtk_info_bar_add_button (GTK_INFO_BAR(info_bar), _("_Close"), GTK_RESPONSE_OK);

	g_signal_connect_swapped(G_OBJECT(info_bar), "response", G_CALLBACK (gtk_widget_hide), OB("infobar_graphs_container"));

	gtk_container_add(GTK_CONTAINER(OB("infobar_graphs_container")), info_bar);
	gtk_widget_show_all(GTK_WIDGET(OB("infobar_graphs_container")));
}

void
multiload_preferences_disable_settings(guint mask)
{
	guint i;

	if (builder == NULL)
		multiload_preferences_init();

	if (mask & MULTILOAD_SETTINGS_SIZE) {
		for (i=0; i<GRAPH_MAX; i++)
			gtk_widget_set_sensitive(GTK_WIDGET(OB(sb_size_names[i])), FALSE);
	}

	if (mask & MULTILOAD_SETTINGS_PADDING)
		gtk_widget_set_sensitive(GTK_WIDGET(OB("hscale_padding")), FALSE);

	if (mask & MULTILOAD_SETTINGS_SPACING)
		gtk_widget_set_sensitive(GTK_WIDGET(OB("hscale_spacing")), FALSE);

	if (mask & MULTILOAD_SETTINGS_ORIENTATION)
		gtk_widget_set_sensitive(GTK_WIDGET(OB("combo_orientation")), FALSE);

	if (mask & MULTILOAD_SETTINGS_DBLCLICK_POLICY) {
		for (i=0; i<GRAPH_MAX; i++)
			gtk_widget_set_sensitive(GTK_WIDGET(OB(combo_dblclick_names[i])), FALSE);
	}

	if (mask & MULTILOAD_SETTINGS_TOOLTIPS) {
		for (i=0; i<GRAPH_MAX; i++)
			gtk_widget_set_sensitive(GTK_WIDGET(OB(combo_tooltip_names[i])), FALSE);
	}

	if (mask & MULTILOAD_SETTINGS_ORIENT_WARNING)
		_orientation_warning_disable = TRUE;

	if (mask & MULTILOAD_SETTINGS_REORDER) {
		gtk_widget_set_sensitive(GTK_WIDGET(OB("treeview_reorder")), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(OB("toolbar_reorder_btn_up")), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(OB("toolbar_reorder_btn_down")), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(OB("toolbar_reorder_btn_reset")), FALSE);
	}
}


void
multiload_preferences_fill_dialog (GtkWidget *dialog, MultiloadPlugin *ma)
{
	guint i,j;
	gint tmp;
	gboolean color_scheme_is_set = FALSE;
	GraphConfig *conf;

	GdkPixbuf *pix;

	multiload_preferences_init();

	// init values
	for (i=0; i<GRAPH_MAX; i++) {
		conf = &ma->graph_config[i];

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(OB(sb_size_names[i])), conf->size*1.00);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(OB(sb_interval_names[i])), conf->interval*1.00);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(OB(cb_visible_names[i])), conf->visible);
		gtk_entry_set_max_length(GTK_ENTRY(OB(entry_dblclick_command_names[i])), sizeof(conf->dblclick_cmdline));
		gtk_combo_box_set_active (GTK_COMBO_BOX(OB(combo_tooltip_names[i])), conf->tooltip_style);
		gtk_combo_box_set_active (GTK_COMBO_BOX(OB(combo_dblclick_names[i])), conf->dblclick_policy);
		gtk_entry_set_text (GTK_ENTRY(OB(entry_dblclick_command_names[i])), conf->dblclick_cmdline);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(OB(sb_border_names[i])), conf->border_width*1.00);

		// autoscaler
		if (cb_autoscaler_names[i][0] != '\0') {
			tmp = multiload_get_max_value(ma, i);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(OB(spin_ceil_names[i])), tmp*1.00);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(OB(cb_autoscaler_names[i])), (tmp<0));
		}

		// filter
		if (cb_source_auto_names[i][0] != '\0') {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(OB(cb_source_auto_names[i])), !conf->filter_enable);

			MultiloadFilter *filter = graph_types[i].get_filter(ma->graphs[i], ma->extra_data[i]);
			for (j=0; j<multiload_filter_get_length(filter); j++) {
				gtk_list_store_insert_with_values (GTK_LIST_STORE(OB(liststore_source_names[i])), NULL, -1,
					LS_SOURCE_COLUMN_SELECTED,	multiload_filter_get_element_selected	(filter, j),
					LS_SOURCE_COLUMN_LABEL,		multiload_filter_get_element_label		(filter, j),
					LS_SOURCE_COLUMN_ABSENT,	multiload_filter_get_element_absent		(filter, j),
					LS_SOURCE_COLUMN_DATA,		multiload_filter_get_element_data		(filter, j),
				-1);
			}
			multiload_filter_free(filter);
		}

		// advanced preferences - tab menu names
		gtk_notebook_set_menu_label_text(GTK_NOTEBOOK(OB("advanced_notebook")), GTK_WIDGET(OB(advanced_box_names[i])), graph_types[i].label);
	}
	gtk_range_set_value(GTK_RANGE(OB("hscale_spacing")), (gdouble)ma->spacing);
	gtk_range_set_value(GTK_RANGE(OB("hscale_padding")), (gdouble)ma->padding);
	gtk_combo_box_set_active (GTK_COMBO_BOX(OB("combo_orientation")), ma->orientation_policy);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(OB("cb_iec_units")), ma->size_format_iec);

	// Memory
	gtk_combo_box_set_active (GTK_COMBO_BOX(OB("combo_mem_slab")), ((MemoryData*)ma->extra_data[GRAPH_MEMLOAD])->procps_compliant?1:0);

	// Parametric
	gtk_entry_set_text(GTK_ENTRY(OB("entry_parm_command")), ((ParametricData*)ma->extra_data[GRAPH_PARAMETRIC])->command);

	// Color schemes
	GtkListStore *ls_colors = GTK_LIST_STORE(OB("liststore_colors"));
	gtk_tree_view_set_row_separator_func (GTK_TREE_VIEW(OB("treeview_colors")), multiload_preferences_color_scheme_row_separator_func, NULL, NULL);
	for (i=0; multiload_builtin_color_schemes[i].name[0] != '\0'; i++) {
		const gchar *name = multiload_builtin_color_schemes[i].name;
		gboolean sep = (name[0]=='=');

		if (multiload_builtin_color_schemes[i].xpm_data != NULL)
			pix = gdk_pixbuf_new_from_xpm_data((const char**)multiload_builtin_color_schemes[i].xpm_data);
		else
			pix = NULL;

		// insert color scheme
		gtk_list_store_insert_with_values( ls_colors, NULL, -1,
			LS_COLORS_COLUMN_NAME,		name,
			LS_COLORS_COLUMN_ICON,		pix,
			LS_COLORS_COLUMN_SEPARATOR,	sep,
		-1 );

		// if it's the current color scheme, select it
		if (strcmp(ma->color_scheme, name) == 0) {
			multiload_preferences_color_scheme_select(i);
			color_scheme_is_set = TRUE;
		}
	}
	// insert (Custom) entry
	gtk_list_store_insert_with_values( ls_colors, NULL, -1,
		LS_COLORS_COLUMN_NAME,		_("(Custom)"),
		LS_COLORS_COLUMN_SEPARATOR,	FALSE,
	-1 );
	// no current color scheme, select last entry (Custom)
	if (!color_scheme_is_set)
		multiload_preferences_color_scheme_select_custom();

	// Graph order
	GtkListStore *ls_reorder = GTK_LIST_STORE(OB("liststore_reorder"));
	for (i=0; i<GRAPH_MAX; i++) {
		gtk_list_store_insert_with_values( ls_reorder, NULL, -1, 0, graph_types[ma->graph_order[i]].label, 1, ma->graph_order[i], -1 );
	}

	// refresh widget status
	multiload_preferences_update_color_buttons(ma);
	multiload_preferences_checkboxes_sensitive_cb(NULL, NULL);
	multiload_preferences_update_dynamic_widgets(ma);

	// other stuff
	multiload_developer_buttons(ma);

	#if GTK_API == 3 && GTK_CHECK_VERSION(3,16,0)
	if (gtk_check_version(3,16,0) == NULL)
		gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(OB("scrolledwindow_color_scheme")), FALSE);
	#endif

	// main window
	GtkWidget *mainwnd_vbox = GTK_WIDGET(OB("mainwnd_vbox"));
	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), mainwnd_vbox);

	multiload_preferences_connect_signals(ma);

	g_debug("[preferences] Initialized");
}

