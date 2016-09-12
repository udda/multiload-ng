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
	guint c_top, c_bottom, c_border;
	cairo_t *cr;
	GdkRGBA *colors = g->config->colors;

	guint x = 0;
	guint y = 0;
	guint W = g->draw_width;
	guint H = g->draw_height;

	double line_x, line_y, line_y_dest;

	/* we might get called before the configure event so that
	 * g->disp->allocation may not have the correct size
	 * (after the user resized the applet in the prop dialog). */

	if (!g->surface)
		g->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, W, H);

	cr = cairo_create (g->surface);
	cairo_set_line_width (cr, 1.0);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);

	c_top = multiload_colors_get_extra_index(g->id, EXTRA_COLOR_BACKGROUND_TOP);
	c_bottom = multiload_colors_get_extra_index(g->id, EXTRA_COLOR_BACKGROUND_BOTTOM);
	c_border = multiload_colors_get_extra_index(g->id, EXTRA_COLOR_BORDER);


	// border
	if (g->config->border_width > 0) {
		cairo_set_source_rgba_from_config(cr, g->config, c_border);
		cairo_rectangle(cr, 0, 0, W, H);
		cairo_fill(cr);

		if (2*g->config->border_width < W)
			W -= 2*g->config->border_width;
		else
			W=0;

		if (2*g->config->border_width < H)
			H -= 2*g->config->border_width;
		else
			H=0;

		x = g->config->border_width;
		y = g->config->border_width;
	}

	if (W > 0 && H > 0) {
		// background
		cairo_set_vertical_gradient(cr, H, &(colors[c_top]), &(colors[c_bottom]));
		cairo_rectangle(cr, x, y, W, H);
		cairo_fill(cr);

		// graph data
		for (i = 0; i < W; i++)
			g->pos[i] = H;

		for (j = 0; j < multiload_config_get_num_data(g->id); j++) {
			cairo_set_source_rgba_from_config(cr, g->config, j);
			for (i = 0; i < W; i++) {
				if (g->data[i][j] == 0)
					continue;

				line_x = x + W - i - 0.5;
				line_y = y + g->pos[i] - 0.5;
				line_y_dest = line_y - g->data[i][j] + 1;

				// Ensure 1px lines are drawn
				if (g->data[i][j] == 1)
					line_y_dest -= 1;

				if (line_y > y) { // don't even begin to draw out of scale values
					if (line_y_dest < y) // makes sure that line ends to graph border
						line_y_dest = y + 0.5;

					cairo_move_to (cr, line_x, line_y);
					cairo_line_to (cr, line_x, line_y_dest);
				}

				g->pos[i] -= g->data[i][j];
			}

			cairo_stroke (cr);
		}
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

	g_assert_nonnull(g->multiload->extra_data);
	guint H = g->draw_height - 2 * (g->multiload->graph_config[g->id].border_width);
	graph_types[g->id].get_data(H, g->data [0], g, g->multiload->extra_data[g->id]);

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

static void
load_graph_destroy (GtkWidget *widget, LoadGraph *g)
{
	load_graph_stop (g);

	gtk_widget_destroy(widget);
}

static gchar* parse_cmdline(const gchar* cmdline, LoadGraph *g) {
	guint i;
	gchar *ret, *tmp;

	gchar number[2] = {'1'+g->id, '\0'};
	// Convert escaped percent to UTF-8 invalid byte (so it isn't parsed by other means).
	// Once other replacements are done, convert back invalid UTF-8 character to escaped percent
	gchar percent_escape[] = "\xff";

	g_assert_nonnull(g->multiload->extra_data);
	graph_types[g->id].cmdline_output(g, g->multiload->extra_data[g->id]);

	const gchar *subst_table[][2] = {
		{ "%%",				percent_escape },
		{ "%n",				number },
		{ "%x",				graph_types[g->id].name },
		{ "%1",				g->output_str[0] },
		{ "%2",				g->output_str[1] },
		{ "%3",				g->output_str[2] },
		{ "%4",				g->output_str[3] },
		{ "%u",				g->output_unit }, // leave that for last, as it can contain '%'s
		{ percent_escape,	"%" }
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
				cmdline = parse_cmdline(g->config->dblclick_cmdline, g);
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
load_graph_mouse_move_cb(GtkWidget *widget, GdkEventCrossing *event, LoadGraph *graph)
{
	graph->tooltip_update = (event->type == GDK_ENTER_NOTIFY);
	return TRUE;
}

LoadGraph *
load_graph_new (MultiloadPlugin *ma, guint id)
{
	LoadGraph *g;

	g = g_new0 (LoadGraph, 1);
	g->id = id;

	g->tooltip_update = FALSE;
	g->multiload = ma;
	g->config = &ma->graph_config[id];

	g->main_widget = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

	g->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start (GTK_BOX (g->main_widget), g->box, TRUE, TRUE, 0);

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
	g_signal_connect (G_OBJECT(g->disp), "enter-notify-event", G_CALLBACK(load_graph_mouse_move_cb), g);
	g_signal_connect (G_OBJECT(g->disp), "leave-notify-event", G_CALLBACK(load_graph_mouse_move_cb), g);

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
