/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *                    The Free Software Foundation
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

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <glib.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "gtk-compat.h"
#include "graph-data.h"
#include "load-graph.h"
#include "multiload.h"
#include "multiload-config.h"
#include "multiload-colors.h"
#include "preferences.h"
#include "util.h"


/* Wrapper for cairo_set_source_rgba */
static void
cairo_set_source_rgba_from_config(cairo_t *cr, GraphConfig *config, guint color_index)
{
	GdkRGBA *c = &(config->colors[color_index]);
	cairo_set_source_rgba(cr, c->red, c->green, c->blue, c->alpha);
}

static void
cairo_set_vertical_gradient(cairo_t *cr, double height, GdkRGBA *a, GdkRGBA *b)
{
	cairo_pattern_t *pat = cairo_pattern_create_linear (0.0, 0.0, 0.0, height);
	cairo_pattern_add_color_stop_rgb (pat, 0, a->red, a->green, a->blue);
	cairo_pattern_add_color_stop_rgb (pat, 1, b->red, b->green, b->blue);
	cairo_set_source(cr, pat);
}

/* Redraws the backing pixmap for the load graph and updates the window */
static void
load_graph_draw (LoadGraph *g)
{
	guint i, j;
	guint c_top, c_bottom;
	cairo_t *cr;
	GraphConfig *config = &(g->multiload->graph_config[g->id]);
	GdkRGBA *colors = config->colors;

	const guint W = g->draw_width;
	const guint H = g->draw_height;

	/* we might get called before the configure event so that
	 * g->disp->allocation may not have the correct size
	 * (after the user resized the applet in the prop dialog). */

	if (!g->surface)
		g->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, W, H);

	cr = cairo_create (g->surface);
	cairo_set_line_width (cr, 1.0);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);

	for (i = 0; i < W; i++)
		g->pos[i] = H - 1;

	c_top = multiload_colors_get_extra_index(g->id, EXTRA_COLOR_BACKGROUND_TOP);
	c_bottom = multiload_colors_get_extra_index(g->id, EXTRA_COLOR_BACKGROUND_BOTTOM);
	cairo_set_vertical_gradient(cr, g->draw_height, &(colors[c_top]), &(colors[c_bottom]));
	cairo_rectangle(cr, 0, 0, g->draw_width, g->draw_height);
	cairo_fill(cr);

	for (j = 0; j < multiload_config_get_num_data(g->id); j++) {
		cairo_set_source_rgba_from_config(cr, config, j);

		for (i = 0; i < W; i++) {
			if (g->data[i][j] == 0)
				continue;
			cairo_move_to (cr, W - i - 0.5, g->pos[i] + 0.5);
			cairo_line_to (cr, W - i - 0.5, g->pos[i] + 0.5 - g->data[i][j]);

			g->pos[i] -= g->data[i][j];
		}

		cairo_stroke (cr);
	}

	cairo_destroy (cr);

	cr = gdk_cairo_create (gtk_widget_get_window (g->disp));
	cairo_set_source_surface (cr, g->surface, 0, 0);
	cairo_paint (cr);
	cairo_destroy (cr);
}

/* Rotates graph data to the right */
static void
load_graph_rotate (LoadGraph *g)
{
	guint i;
	gint* tmp;

	tmp = g->data[g->draw_width - 1];
	for(i = g->draw_width - 1; i > 0; --i)
		g->data[i] = g->data[i-1];
	g->data[0] = tmp;
}


/* Updates the load graph when the timeout expires */
static gboolean
load_graph_update (LoadGraph *g)
{

	if (g->data == NULL)
		return TRUE;

	load_graph_rotate(g);
	graph_types[g->id].get_data(g->draw_height, g->data [0], g);

	if (g->tooltip_update)
		multiload_tooltip_update(g);

	load_graph_draw(g);
	return TRUE;
}

void
load_graph_unalloc (LoadGraph *g)
{
	guint i;

	if (!g->allocated)
		return;

	for (i = 0; i < g->draw_width; i++)
		g_free (g->data [i]);

	g_free (g->data);
	g_free (g->pos);

	g->pos = NULL;
	g->data = NULL;

	if (g->surface) {
		cairo_surface_destroy (g->surface);
		g->surface = NULL;
	}

	g->allocated = FALSE;
	g_debug("[load-graph] Graph '%s' unallocated", graph_types[g->id].name);
}

static void
load_graph_alloc (LoadGraph *g)
{
	guint i;

	if (g->allocated)
		return;

	g->data = g_new0 (gint *, g->draw_width);
	g->pos = g_new0 (guint, g->draw_width);

	guint data_size = sizeof (guint) * multiload_config_get_num_data(g->id);

	for (i = 0; i < g->draw_width; i++)
		g->data [i] = g_malloc0 (data_size);

	g->allocated = TRUE;
	g_debug("[load-graph] Graph '%s' allocated", graph_types[g->id].name);
}

static gint
load_graph_configure (GtkWidget *widget, GdkEventConfigure *event, LoadGraph *g)
{
	GtkAllocation allocation;

	load_graph_unalloc (g);

	gtk_widget_get_allocation (g->disp, &allocation);

	g->draw_width = allocation.width;
	g->draw_height = allocation.height;
	g->draw_width = MAX (g->draw_width, 1);
	g->draw_height = MAX (g->draw_height, 1);

	g_debug("[load-graph] widget allocation for graph '%s': [%d,%d] resulting draw size: [%d,%d]", graph_types[g->id].name, allocation.width, allocation.height, g->draw_width, g->draw_height);

	load_graph_alloc (g);

	if (!g->surface)
		g->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
												g->draw_width, g->draw_height);

	gtk_widget_queue_draw (widget);

	return TRUE;
}
/*
static gboolean
load_graph_draw_cb (GtkWidget *widget, cairo_t *cr, LoadGraph *g)
{
	cairo_set_source_surface (cr, g->surface, 0, 0);
	cairo_paint (cr);

	return FALSE;
}

#if GTK_API == 2
static gboolean
load_graph_expose (GtkWidget *widget, GdkEventExpose *event, LoadGraph *g)
{
	cairo_t *cr = gdk_cairo_create (event->window);
	load_graph_draw_cb(widget, cr, g);
	cairo_destroy (cr);
	return FALSE;
}
#endif
*/

static gboolean
load_graph_border_draw_cb (GtkWidget *widget, cairo_t *cr, GdkRGBA *color)
{
	GtkAllocation allocation;
	gtk_widget_get_allocation (widget, &allocation);

	cairo_set_source_rgba(cr, color->red, color->green, color->blue, 1.0);
	cairo_rectangle(cr, 0, 0, allocation.width, allocation.height);
	cairo_fill (cr);

	cairo_paint (cr);

	return TRUE;
}

#if GTK_API == 2
static gboolean
load_graph_border_expose (GtkWidget *widget, GdkEventExpose *event, GdkRGBA *color)
{
	cairo_t *cr = gdk_cairo_create (event->window);
	load_graph_border_draw_cb(widget, cr, color);
	cairo_destroy (cr);
	return TRUE;
}
#endif

static void
load_graph_destroy (GtkWidget *widget, LoadGraph *g)
{
	load_graph_stop (g);

	gtk_widget_destroy(widget);
}

static gchar* parse_cmdline(const gchar* cmdline, guint n_graph) {
	g_assert(n_graph <= GRAPH_MAX);

	guint i;
	gchar *ret, *tmp;

	gchar number[2] = {'1'+n_graph, '\0'};

	const gchar *subst_table[][2] = {
		{ "%n", number },
		{ "%x", graph_types[n_graph].name }
	};

	ret = g_strdup(cmdline);
	for (i=0; i<G_N_ELEMENTS(subst_table); i++) {
		tmp = str_replace(ret, subst_table[i][0], subst_table[i][1]);
		g_free(ret);
		ret = tmp;
	}

	return ret;
}

static gboolean
load_graph_clicked (GtkWidget *widget, GdkEventButton *event, LoadGraph *g)
{
	int result;
	gchar* cmdline;
	/* check if button event is a double click with first mouse button */
	if (event->button == 1 && event->type == GDK_2BUTTON_PRESS) {
		switch(g->config->dblclick_policy) {
			case DBLCLICK_POLICY_TASKMANAGER:
				cmdline = get_system_monitor_executable();
				g_debug("[load-graph] Detected double click on graph '%s' - action: start task manager (%s)", graph_types[g->id].name, cmdline);
				break;
			case DBLCLICK_POLICY_CMDLINE:
				cmdline = parse_cmdline(g->config->dblclick_cmdline, g->id);
				g_debug("[load-graph] Detected double click on graph '%s' - action: execute command line (%s)", graph_types[g->id].name, cmdline);
				break;
			case DBLCLICK_POLICY_DONOTHING:
				g_debug("[load-graph] Detected double click on graph '%s' - action: none", graph_types[g->id].name);
			default:
				return FALSE;
		}

		result = g_spawn_command_line_async (cmdline, NULL);

		if (G_UNLIKELY (result == FALSE))
			g_warning (_("Unable to execute the following command line: '%s'"), cmdline);

		g_free(cmdline);
	}
	return FALSE;
}

static gboolean
load_graph_enter_cb(GtkWidget *widget, GdkEventCrossing *event, LoadGraph *graph)
{
	graph->tooltip_update = TRUE;
	multiload_tooltip_update(graph);

	return TRUE;
}

static gboolean
load_graph_leave_cb(GtkWidget *widget, GdkEventCrossing *event, LoadGraph *graph)
{
	graph->tooltip_update = FALSE;

	return TRUE;
}

static void
load_graph_extra_data_init(LoadGraph *g) {
	g_assert_nonnull(g);
	switch(g->id) {
		case GRAPH_CPULOAD:
			g->extra_data = (gpointer)g_new0(CpuData, 1);
			break;
		case GRAPH_MEMLOAD:
			g->extra_data = (gpointer)g_new0(MemoryData, 1);
			break;
		case GRAPH_NETLOAD:
			g->extra_data = (gpointer)g_new0(NetData, 1);
			break;
		case GRAPH_SWAPLOAD:
			g->extra_data = (gpointer)g_new0(SwapData, 1);
			break;
		case GRAPH_LOADAVG:
			g->extra_data = (gpointer)g_new0(LoadData, 1);
			break;
		case GRAPH_DISKLOAD:
			g->extra_data = (gpointer)g_new0(DiskData, 1);
			break;
		case GRAPH_TEMPERATURE:
			g->extra_data = (gpointer)g_new0(TemperatureData, 1);
#ifdef MULTILOAD_EXPERIMENTAL
		case GRAPH_PARAMETRIC:
			g->extra_data = (gpointer)g_new0(ParametricData, 1);
			break;
#endif
	}
}

LoadGraph *
load_graph_new (MultiloadPlugin *ma, guint id)
{
	LoadGraph *g;
	guint k;

	g = g_new0 (LoadGraph, 1);
	g->id = id;
	load_graph_extra_data_init(g);

	g->tooltip_update = FALSE;
	g->multiload = ma;
	g->config = &ma->graph_config[id];

	g->main_widget = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

	g->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);


	k = multiload_colors_get_extra_index(id, EXTRA_COLOR_BORDER);
	g->border = gtk_event_box_new();
	#if GTK_API == 2
		g_signal_connect (G_OBJECT(g->border), "expose_event", G_CALLBACK (load_graph_border_expose), &(ma->graph_config[id].colors[k]));
	#elif GTK_API == 3
		g_signal_connect (G_OBJECT(g->border), "draw", G_CALLBACK (load_graph_border_draw_cb), &(ma->graph_config[id].colors[k]));
	#endif
	gtk_container_set_border_width(GTK_CONTAINER(g->box), ma->graph_config[id].border_width);

	gtk_container_add (GTK_CONTAINER (g->border), g->box);
	gtk_box_pack_start (GTK_BOX (g->main_widget), g->border, TRUE, TRUE, 0);


	g->timer_index = -1;

	load_graph_resize(g);

	g->disp = gtk_drawing_area_new ();
	gtk_widget_set_events (g->disp,
						GDK_EXPOSURE_MASK |
						GDK_ENTER_NOTIFY_MASK |
						GDK_LEAVE_NOTIFY_MASK |
						GDK_BUTTON_PRESS_MASK);

/*	//TODO: is this really needed? Works perfectly without (as drawing is done through timers)
	#if GTK_API == 2
		g_signal_connect (G_OBJECT(g->disp), "expose_event", G_CALLBACK (load_graph_expose), g);
	#elif GTK_API == 3
		g_signal_connect (G_OBJECT(g->disp), "draw", G_CALLBACK (load_graph_draw_cb), g);
	#endif */
	g_signal_connect (G_OBJECT(g->disp), "configure_event", G_CALLBACK (load_graph_configure), g);
	g_signal_connect (G_OBJECT(g->disp), "destroy", G_CALLBACK (load_graph_destroy), g);
	g_signal_connect (G_OBJECT(g->disp), "button-press-event", G_CALLBACK (load_graph_clicked), g);
	g_signal_connect (G_OBJECT(g->disp), "enter-notify-event", G_CALLBACK(load_graph_enter_cb), g);
	g_signal_connect (G_OBJECT(g->disp), "leave-notify-event", G_CALLBACK(load_graph_leave_cb), g);

	gtk_box_pack_start (GTK_BOX (g->box), g->disp, TRUE, TRUE, 0);    
	gtk_widget_show_all(g->box);

	return g;
}

void
load_graph_resize (LoadGraph *g)
{
	guint size = CLAMP(g->config->size, MIN_SIZE, MAX_SIZE);
	gint w, h;

	if ( g->multiload->panel_orientation == GTK_ORIENTATION_VERTICAL ) {
		w = -1;
		h = size;
	}
	else { /* GTK_ORIENTATION_HORIZONTAL */ 
		w = size;
		h = -1;
	}

	gtk_widget_set_size_request (g->main_widget, w, h);

	g_debug("[load-graph] Set size request of graph '%s' to [%d, %d]", graph_types[g->id].name, w, h);
}

void
load_graph_start (LoadGraph *g)
{
	guint interval = CLAMP(g->config->interval, MIN_INTERVAL, MAX_INTERVAL);
	load_graph_stop(g);
	g->timer_index = g_timeout_add (interval, (GSourceFunc) load_graph_update, g);
	g_debug("[load-graph] Timer started for graph '%s' (interval: %d ms)", graph_types[g->id].name, interval);
}

void
load_graph_stop (LoadGraph *g)
{
	if (g->timer_index != -1)
		g_source_remove (g->timer_index);

	g->timer_index = -1;
	g_debug("[load-graph] Time stopped for graph '%s'", graph_types[g->id].name);
}
