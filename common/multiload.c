/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *               1997 The Free Software Foundation
 *                    (Authors: Tim P. Gerla, Martin Baulig, Todd Kulesza)
 *
 * With code from wmload.c, v0.9.2, apparently by Ryan Land, rland@bc1.com.
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

#include <glib/gi18n-lib.h>

#include "colors.h"
#include "gtk-compat.h"
#include "graph-data.h"
#include "load-graph.h"
#include "multiload.h"
#include "multiload-config.h"
#include "preferences.h"
#include "util.h"
#include "ui.h"


#ifdef ENABLE_MULTILOAD_DEBUG
// https://stackoverflow.com/a/1737675/106302
void _debug(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	FILE *log = fopen("/tmp/multiload-ng.log", "a");
	vfprintf(log, fmt, ap);
	fputs("\n", log);
	fclose(log);
	va_end(ap);
}
#else
void _debug(const char *fmt, ...) {
	(void) fmt;
}
#endif

const char* MULTILOAD_CONFIG_PATH;

void
multiload_tooltip_update (LoadGraph *g)
{
	gchar buf_title[100];
	gchar buf_text[800];
	gchar tooltip_markup[1024];

	g_assert(g != NULL);
	g_assert(g->multiload != NULL);
	g_assert(g->multiload->extra_data != NULL);
	g_assert_cmpuint(g->id, >=, 0);
	g_assert_cmpuint(g->id,  <, GRAPH_MAX);

	buf_title[0] = '\0';
	buf_text[0] = '\0';

	graph_types[g->id].tooltip_update(buf_title, sizeof(buf_title), buf_text, sizeof(buf_text), g, g->multiload->extra_data[g->id], g->config->tooltip_style);

	if (buf_text[0] == '\0')
		g_warning("[multiload] Empty text for tooltip #%d", g->id);

	const gchar *title = (*buf_title)? buf_title : graph_types[g->id].label;
	gchar *text = str_replace(buf_text, "&", "&amp;");

	if (g->config->tooltip_style == MULTILOAD_TOOLTIP_STYLE_DETAILED)
		g_snprintf(tooltip_markup, sizeof(tooltip_markup), "<span weight='bold' size='larger'>%s</span>\n%s", title, text);
	else
		g_snprintf(tooltip_markup, sizeof(tooltip_markup), "%s: %s", title, text);

	gtk_widget_set_tooltip_markup(g->disp, tooltip_markup);
	gtk_widget_trigger_tooltip_query (g->disp);
	g_free(text);
}

/* get current orientation */
GtkOrientation
multiload_get_orientation(MultiloadPlugin *ma) {
	if (ma->orientation_policy == MULTILOAD_ORIENTATION_HORIZONTAL)
		return GTK_ORIENTATION_HORIZONTAL;
	else if (ma->orientation_policy == MULTILOAD_ORIENTATION_VERTICAL)
		return GTK_ORIENTATION_VERTICAL;
	else // if (ma->orientation_policy == MULTILOAD_ORIENTATION_AUTO)
		return ma->panel_orientation;
}

void
multiload_start(MultiloadPlugin *ma)
{
	guint i, n;
	gboolean vis;

	ma->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

	multiload_set_spacing(ma, ma->spacing);
	multiload_set_padding(ma, ma->padding);
	multiload_refresh_orientation(ma);
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(ma->container), FALSE);

	gtk_widget_show (ma->box);
	gtk_container_add (ma->container, ma->box);

	// Children (graphs) are individually shown/hidden to control visibility
	gtk_widget_set_no_show_all (ma->box, TRUE);

	// Only start and display the graphs the user has turned on
	for (n=0, i=0; i < GRAPH_MAX; i++) {
		gtk_box_pack_start(GTK_BOX(ma->box), ma->graphs[i]->main_widget, TRUE, TRUE, 0);

		vis = ma->graph_config[i].visible;
		gtk_widget_set_visible (ma->graphs[i]->main_widget, vis);
		if (vis) {
			load_graph_start(ma->graphs[i]);
			n++;
		}
	}

	multiload_set_order (ma, ma->graph_order);

	g_debug("[multiload] Started %d of %d graphs", n, GRAPH_MAX);
	return;
}

void
multiload_set_spacing (MultiloadPlugin *ma, gint val)
{
	gtk_box_set_spacing(GTK_BOX(ma->box), val);
}

void
multiload_set_padding (MultiloadPlugin *ma, gint val)
{
	gtk_container_set_border_width(GTK_CONTAINER(ma->box), val);
}

void
multiload_set_max_value (MultiloadPlugin *ma, guint graph_id, int val)
{
	AutoScaler *scaler = multiload_get_scaler(ma, graph_id);
	if (scaler == NULL)
		return;
	if (val < 0) {
		autoscaler_set_enabled(scaler, TRUE);
	} else {
		autoscaler_set_enabled(scaler, FALSE);
		autoscaler_set_max(scaler, val);
	}
}

void
multiload_set_max_floor (MultiloadPlugin *ma, guint graph_id, int val)
{
	AutoScaler *scaler = multiload_get_scaler(ma, graph_id);
	if (scaler == NULL)
		return;

	if (val < 0)
		val = AUTOSCALER_MIN_DEFAULT;

	autoscaler_set_min(scaler, val);
}

int
multiload_get_max_value(MultiloadPlugin *ma, guint graph_id)
{
	AutoScaler *scaler = multiload_get_scaler(ma, graph_id);
	if (scaler == NULL)
		return -1;

	if (autoscaler_get_enabled(scaler))
		return -1;
	else
		return autoscaler_get_max(scaler, NULL, 0);
}

gint
multiload_calculate_size_request (MultiloadPlugin *ma)
{
	guint i;
	gint size = 2*ma->padding - ma->spacing;

	for ( i = 0; i < GRAPH_MAX; i++ ) {
		if (ma->graph_config[i].visible) {
			size += ma->graph_config[i].size;
			size += ma->spacing;
		}
	}

	return size;
}

void
multiload_refresh_orientation (MultiloadPlugin *ma)
{
	GtkAllocation alloc;
	guint i;
	LoadGraph *g;
	gint opposite_size = 120;

	gtk_orientable_set_orientation(GTK_ORIENTABLE(ma->box), multiload_get_orientation(ma));

	if ( ma->panel_orientation == GTK_ORIENTATION_HORIZONTAL && ma->orientation_policy == MULTILOAD_ORIENTATION_VERTICAL) {
		gtk_widget_set_size_request(GTK_WIDGET(ma->container), opposite_size, -1);
	} else if ( ma->panel_orientation == GTK_ORIENTATION_VERTICAL && ma->orientation_policy == MULTILOAD_ORIENTATION_HORIZONTAL) {
		gtk_widget_set_size_request(GTK_WIDGET(ma->container), -1, opposite_size);
	} else
		gtk_widget_set_size_request(GTK_WIDGET(ma->container), -1, -1);

	for (i=0; i<GRAPH_MAX; i++) {
		g = ma->graphs[i];
		if (g != NULL)
			load_graph_resize(g);
	}

	gtk_widget_get_allocation(GTK_WIDGET(ma->container), &alloc);
	g_debug("[multiload] New allocation for Multiload-ng container: %d,%d", alloc.width, alloc.height);
}

void
multiload_set_order (MultiloadPlugin *ma, gint new_order[GRAPH_MAX])
{
	gint i;

	// validate permutation
	gint check[GRAPH_MAX];
	memset(check, 0, sizeof(check));

	for (i=0; i<GRAPH_MAX; i++) {
		if (new_order[i] < 0 || new_order[i] >= GRAPH_MAX)
			g_error("multiload_set_order: permutation index out of bounds");
		else
			check[new_order[i]]++;
	}

	for (i=0; i<GRAPH_MAX; i++) {
		if (check[i] != 1)
			g_error("multiload_set_order: array is not a permutation");
	}

	// actual reordering
	for (i=0; i<GRAPH_MAX; i++)
		gtk_box_reorder_child (GTK_BOX(ma->box), GTK_WIDGET(ma->graphs[new_order[i]]->main_widget), -1);
}

void
multiload_init()
{
	static gboolean initialized = FALSE;
	if (initialized)
		return;
	initialized = TRUE;


	MULTILOAD_CONFIG_PATH = g_build_filename(
			g_get_home_dir (),
			".config",
			"multiload-ng",
			NULL);

	if (g_mkdir_with_parents (MULTILOAD_CONFIG_PATH, 0755) != 0)
		g_error("[multiload] Error creating directory '%s'", MULTILOAD_CONFIG_PATH);


	multiload_config_init();

	multiload_ui_print_notice();

	g_debug("[multiload] Initialization complete");
}

void multiload_defaults(MultiloadPlugin *ma)
{
	guint i;
	GraphConfig *conf;

	/* default settings */
	ma->padding = DEFAULT_PADDING;
	ma->spacing = DEFAULT_SPACING;
	ma->size_format_iec = DEFAULT_SIZE_FORMAT_IEC;
	strncpy(ma->color_scheme, DEFAULT_COLOR_SCHEME, sizeof(ma->color_scheme));
	for ( i = 0; i < GRAPH_MAX; i++ ) {
		conf = &ma->graph_config[i];
		conf->border_width = DEFAULT_BORDER_WIDTH;
		conf->visible = i == 0 ? TRUE : FALSE;
		conf->interval = DEFAULT_INTERVAL;
		conf->size = DEFAULT_SIZE;
		conf->tooltip_style = DEFAULT_TOOLTIP_STYLE;
		conf->dblclick_policy = DEFAULT_DBLCLICK_POLICY;
		conf->filter[0] = '\0';
		conf->filter_enable = FALSE;
		conf->bg_direction = DEFAULT_BACKGROUND_DIRECTION;
		ma->graph_order[i] = i;
		multiload_colors_default(ma, i);

		multiload_set_max_value(ma, i, graph_types[i].scaler_max);
		multiload_set_max_floor(ma, i, graph_types[i].scaler_max_floor);
	}

	((MemoryData*)ma->extra_data[GRAPH_MEMLOAD])->procps_compliant = TRUE;
}

void
multiload_sanitize(MultiloadPlugin *ma)
{
	guint i, visible_count = 0;

	/* Keep values between max and min */
	ma->padding = CLAMP(ma->padding, MIN_PADDING, MAX_PADDING);
	ma->spacing = CLAMP(ma->spacing, MIN_SPACING, MAX_SPACING);
	ma->orientation_policy = CLAMP(ma->orientation_policy, 0, MULTILOAD_ORIENTATION_N_VALUES);

	for ( i=0; i<GRAPH_MAX; i++ ) {
		ma->graph_config[i].border_width = CLAMP(ma->graph_config[i].border_width, MIN_BORDER_WIDTH, MAX_BORDER_WIDTH);

		ma->graph_config[i].interval = CLAMP(ma->graph_config[i].interval, MIN_INTERVAL, MAX_INTERVAL);
		ma->graph_config[i].size = CLAMP(ma->graph_config[i].size, MIN_SIZE, MAX_SIZE);
		ma->graph_config[i].tooltip_style = CLAMP(ma->graph_config[i].tooltip_style, 0, MULTILOAD_TOOLTIP_STYLE_N_VALUES);
		ma->graph_config[i].dblclick_policy = CLAMP(ma->graph_config[i].dblclick_policy, 0, DBLCLICK_POLICY_N_VALUES);

		if (ma->graph_config[i].visible) {
			ma->graph_config[i].visible = TRUE;
			visible_count++;
		}
	}

	/* Ensure at lease one graph is visible */
	if (visible_count == 0)
		ma->graph_config[0].visible = TRUE;
}

void
multiload_set_update_cb (MultiloadPlugin *ma, guint graph_id, GraphUpdateFunc callback, gpointer user_data) {
	g_assert(ma != NULL);
	ma->graphs[graph_id]->update_cb = callback;
	ma->graphs[graph_id]->update_cb_user_data = user_data;
}

MultiloadPlugin*
multiload_new()
{
	guint i;
	MultiloadPlugin *ma = g_slice_new0(MultiloadPlugin);
	multiload_init();

	ma->container = GTK_CONTAINER(gtk_event_box_new ());
	gtk_widget_show (GTK_WIDGET(ma->container));

	ma->extra_data[GRAPH_CPULOAD]		= (gpointer)g_new0(CpuData, 1);
	ma->extra_data[GRAPH_MEMLOAD]		= (gpointer)g_new0(MemoryData, 1);
	ma->extra_data[GRAPH_NETLOAD]		= (gpointer)g_new0(NetData, 1);
	ma->extra_data[GRAPH_SWAPLOAD]		= (gpointer)g_new0(SwapData, 1);
	ma->extra_data[GRAPH_LOADAVG]		= (gpointer)g_new0(LoadData, 1);
	ma->extra_data[GRAPH_DISKLOAD]		= (gpointer)g_new0(DiskData, 1);
	ma->extra_data[GRAPH_TEMPERATURE]	= (gpointer)g_new0(TemperatureData, 1);
	ma->extra_data[GRAPH_BATTERY]		= (gpointer)g_new0(BatteryData, 1);
	ma->extra_data[GRAPH_PARAMETRIC]	= (gpointer)g_new0(ParametricData, 1);

	for (i=0; i < GRAPH_MAX; i++) {
		ma->graphs[i] = load_graph_new (ma, i);
		if ( graph_types[i].init != NULL )
			graph_types[i].init(ma->graphs[i], ma->extra_data[i]);
	}

	return ma;
}

void
multiload_free(MultiloadPlugin *ma)
{
	gint i;

	for (i = 0; i < GRAPH_MAX; i++) {
		load_graph_stop (ma->graphs[i]);
		gtk_widget_destroy (ma->graphs[i]->main_widget);

		load_graph_unalloc (ma->graphs[i]);
		g_free (ma->graphs[i]);

		g_free (ma->extra_data[i]);
	}

	g_free (ma);

	g_debug("[multiload] Destroyed");
}


int
multiload_find_graph_by_name(char *str, char **suffix)
{
	guint i;
	for ( i = 0; i < GRAPH_MAX; i++ ) {
		int n = strlen(graph_types[i].name);
		if ( strncasecmp(str, graph_types[i].name, n) == 0 ) {
			if ( suffix )
				*suffix = str+n;
			return i;
		}
	}
	return -1;
}

