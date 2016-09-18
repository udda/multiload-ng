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


const char* MULTILOAD_CONFIG_PATH;

void
multiload_tooltip_update (LoadGraph *g)
{
	gchar *title = NULL;
	gchar *text = NULL;
	gchar *text_sanitized = NULL;
	gchar *tooltip_markup;

	g_assert_nonnull(g);
	g_assert_nonnull(g->multiload);
	g_assert_nonnull(g->multiload->extra_data);
	g_assert_cmpuint(g->id, >=, 0);
	g_assert_cmpuint(g->id,  <, GRAPH_MAX);

	graph_types[g->id].tooltip_update(&title, &text, g, g->multiload->extra_data[g->id]);

	if (title == NULL)
		title = g_strdup(graph_types[g->id].label);

	if (text == NULL) {
		text_sanitized = g_strdup("");
		g_warning("[multiload] Empty text for tooltip #%d", g->id);
	} else {
		text_sanitized = str_replace(text, "&", "&amp;");
		g_free(text);
	}

	if (g->config->tooltip_style == MULTILOAD_TOOLTIP_STYLE_DETAILED) {
		tooltip_markup = g_strdup_printf("<span weight='bold' size='larger'>%s</span>\n%s", title, text_sanitized);
	} else {
		tooltip_markup = g_strdup_printf("%s: %s", title, text_sanitized);
	}

	gtk_widget_set_tooltip_markup(g->disp, tooltip_markup);
	gtk_widget_trigger_tooltip_query (g->disp);
	g_free(title);
	g_free(text_sanitized);
	g_free(tooltip_markup);

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
	multiload_set_fill_between(ma, ma->fill_between);
	multiload_refresh_orientation(ma);

	gtk_widget_show (ma->box);
	gtk_container_add (ma->container, ma->box);

	// Children (graphs) are individually shown/hidden to control visibility
	gtk_widget_set_no_show_all (ma->box, TRUE);

	// Create the GRAPH_MAX graphs, with user properties from ma->graph_config.
	// Only start and display the graphs the user has turned on
	for (n=0, i=0; i < GRAPH_MAX; i++) {
		ma->graphs[i] = load_graph_new (ma, i);
		gtk_box_pack_start(GTK_BOX(ma->box), ma->graphs[i]->main_widget, TRUE, TRUE, 0);

		vis = ma->graph_config[i].visible;
		gtk_widget_set_visible (ma->graphs[i]->main_widget, vis);
		if (vis) {
			load_graph_start(ma->graphs[i]);
			n++;
		}
	}

	g_debug("[multiload] Started %d of %d graphs", n, GRAPH_MAX);
	return;
}

void
multiload_set_fill_between (MultiloadPlugin *ma, gboolean fill)
{
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(ma->container), fill);
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

void
multiload_refresh_orientation (MultiloadPlugin *ma)
{
	guint i;
	LoadGraph *g;

	gtk_orientable_set_orientation(GTK_ORIENTABLE(ma->box), multiload_get_orientation(ma));

	for (i=0; i<GRAPH_MAX; i++) {
		g = ma->graphs[i];
		if (g != NULL)
			load_graph_resize(g);
	}
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

	g_debug("[multiload] Initialization complete");
}

void multiload_defaults(MultiloadPlugin *ma)
{
	guint i;
	GraphConfig *conf;

	/* default settings */
	ma->padding = DEFAULT_PADDING;
	ma->spacing = DEFAULT_SPACING;
	ma->fill_between = DEFAULT_FILL_BETWEEN;
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
		multiload_colors_default(ma, i);

		multiload_set_max_value(ma, i, graph_types[i].scaler_max);
		multiload_set_max_floor(ma, i, graph_types[i].scaler_max_floor);
	}
}

void
multiload_sanitize(MultiloadPlugin *ma)
{
	guint i, visible_count = 0;

	/* Keep values between max and min */
	ma->padding = CLAMP(ma->padding, MIN_PADDING, MAX_PADDING);
	ma->spacing = CLAMP(ma->spacing, MIN_SPACING, MAX_SPACING);
	ma->fill_between = ma->fill_between? TRUE:FALSE;
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
	g_assert_nonnull(ma);
	ma->graphs[graph_id]->update_cb = callback;
	ma->graphs[graph_id]->update_cb_user_data = user_data;
}

MultiloadPlugin*
multiload_new()
{
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
	ma->extra_data[GRAPH_PARAMETRIC]	= (gpointer)g_new0(ParametricData, 1);

	return ma;
}

void
multiload_destroy(MultiloadPlugin *ma)
{
	gint i;

	for (i = 0; i < GRAPH_MAX; i++) {
		load_graph_stop(ma->graphs[i]);
		gtk_widget_destroy(ma->graphs[i]->main_widget);

		load_graph_unalloc(ma->graphs[i]);
		g_free(ma->graphs[i]);

		g_free(ma->extra_data[i]);
	}

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

