#include <config.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <glib.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "multiload.h"
#include "multiload-config.h"
#include "util.h"


/* Wrapper for cairo_set_source_rgba */
static void
cairo_set_source_rgba_from_config(cairo_t *cr, GraphConfig *config, guint color_index)
{
	GdkColor *c = &(config->colors[color_index]);
	guint a = config->alpha[color_index];
	cairo_set_source_rgba(cr, c->red/65535.0, c->green/65535.0, c->blue/65535.0, a/65535.0);
}


/* Redraws the backing pixmap for the load graph and updates the window */
static void
load_graph_draw (LoadGraph *g)
{
	guint i, j, k;
	cairo_t *cr;
	GraphConfig *config = &(g->multiload->graph_config[g->id]);
	GdkColor *colors = config->colors;

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

	k = multiload_config_get_num_colors(g->id) - 1; //this is the index of last color (background)
	gdk_cairo_set_source_color (cr, &(colors[k]));
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

/* Updates the load graph when the timeout expires */
static gboolean
load_graph_update (LoadGraph *g)
{
	guint i;
	gint* tmp;

	if (g->data == NULL)
		return TRUE;

	// rotate data to the right
	tmp = g->data[g->draw_width - 1];
	for(i = g->draw_width - 1; i > 0; --i)
		g->data[i] = g->data[i-1];
	g->data[0] = tmp;

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
}

static void
load_graph_alloc (LoadGraph *g)
{
	guint i;

	if (g->allocated)
		return;

	g->data = g_new0 (gint *, g->draw_width);
	g->pos = g_new0 (guint, g->draw_width);

	// count out border and background
	guint data_size = sizeof (guint) * multiload_config_get_num_data(g->id);

	for (i = 0; i < g->draw_width; i++)
		g->data [i] = g_malloc0 (data_size);

	g->allocated = TRUE;
}

static gint
load_graph_configure (GtkWidget *widget, GdkEventConfigure *event,
			  gpointer data_ptr)
{
	GtkAllocation allocation;
	LoadGraph *c = (LoadGraph *) data_ptr;

	load_graph_unalloc (c);

	gtk_widget_get_allocation (c->disp, &allocation);

	c->draw_width = allocation.width;
	c->draw_height = allocation.height;
	c->draw_width = MAX (c->draw_width, 1);
	c->draw_height = MAX (c->draw_height, 1);

	load_graph_alloc (c);

	if (!c->surface)
		c->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
												c->draw_width, c->draw_height);

	gtk_widget_queue_draw (widget);

	return TRUE;
}

static gint
load_graph_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data_ptr)
{
	LoadGraph *g = (LoadGraph *) data_ptr;
	cairo_t *cr;

	cr = gdk_cairo_create (event->window);

	cairo_set_source_surface (cr, g->surface, 0, 0);
	cairo_paint (cr);

	cairo_destroy (cr);

	return FALSE;
}

static void
load_graph_destroy (GtkWidget *widget, gpointer data_ptr)
{
	LoadGraph *g = (LoadGraph *) data_ptr;

	load_graph_stop (g);
	netspeed_delete(g->netspeed_in);
	netspeed_delete(g->netspeed_out);

	gtk_widget_destroy(widget);
}

/*
static gboolean
load_graph_clicked (GtkWidget *widget, GdkEventButton *event, LoadGraph *load)
{
	// Formerly used to have properties open to this graph.

	return FALSE;
}
*/

static gboolean
load_graph_enter_cb(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
	LoadGraph *graph;
	graph = (LoadGraph *)data;

	graph->tooltip_update = TRUE;
	multiload_tooltip_update(graph);

	return TRUE;
}

static gboolean
load_graph_leave_cb(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
	LoadGraph *graph;
	graph = (LoadGraph *)data;

	graph->tooltip_update = FALSE;

	return TRUE;
}

LoadGraph *
load_graph_new (MultiloadPlugin *ma, guint id)
{
	LoadGraph *g;

	g = g_new0 (LoadGraph, 1);
	g->netspeed_in = netspeed_new(g);
	g->netspeed_out = netspeed_new(g);
	g->id = id;

	g->tooltip_update = FALSE;
	g->multiload = ma;

	g->main_widget = gtk_vbox_new (FALSE, 0);

	g->box = gtk_vbox_new (FALSE, 0);

	if (ma->show_frame) {
		g->frame = gtk_frame_new (NULL);
		// frame border color
		gtk_widget_modify_bg (g->frame, GTK_STATE_NORMAL,
					&(ma->graph_config[id].colors[multiload_config_get_num_colors(id)-2]));
		gtk_frame_set_shadow_type (GTK_FRAME (g->frame), GTK_SHADOW_NONE);
		gtk_container_add (GTK_CONTAINER (g->frame), g->box);
		gtk_box_pack_start (GTK_BOX (g->main_widget), g->frame, TRUE, TRUE, 0);
	} else {
		g->frame = NULL;
		gtk_box_pack_start (GTK_BOX (g->main_widget), g->box, TRUE, TRUE, 0);
	}

	g->timer_index = -1;

	load_graph_resize(g);

	g->disp = gtk_drawing_area_new ();
	gtk_widget_set_events (g->disp,
						GDK_EXPOSURE_MASK |
						GDK_ENTER_NOTIFY_MASK |
						GDK_LEAVE_NOTIFY_MASK |
						GDK_BUTTON_PRESS_MASK);

	g_signal_connect (G_OBJECT (g->disp), "expose_event",
					G_CALLBACK (load_graph_expose), g);
	g_signal_connect (G_OBJECT(g->disp), "configure_event",
					G_CALLBACK (load_graph_configure), g);
	g_signal_connect (G_OBJECT(g->disp), "destroy",
					G_CALLBACK (load_graph_destroy), g);
//	g_signal_connect (G_OBJECT(g->disp), "button-press-event",
//					G_CALLBACK (load_graph_clicked), g);
	g_signal_connect (G_OBJECT(g->disp), "enter-notify-event",
					G_CALLBACK(load_graph_enter_cb), g);
	g_signal_connect (G_OBJECT(g->disp), "leave-notify-event",
					G_CALLBACK(load_graph_leave_cb), g);

	gtk_box_pack_start (GTK_BOX (g->box), g->disp, TRUE, TRUE, 0);    
	gtk_widget_show_all(g->box);

	return g;
}

void
load_graph_resize (LoadGraph *g)
{
	guint size = CLAMP(g->multiload->size, MIN_SIZE, MAX_SIZE);

	if ( multiload_get_orientation(g->multiload) == GTK_ORIENTATION_VERTICAL )
		gtk_widget_set_size_request (g->main_widget, -1, size);
	else /* GTK_ORIENTATION_HORIZONTAL */
		gtk_widget_set_size_request (g->main_widget, size, -1);
}

void
load_graph_start (LoadGraph *g)
{
	guint speed = CLAMP(g->multiload->speed, MIN_SPEED, MAX_SPEED);

	if (g->timer_index != -1)
		g_source_remove (g->timer_index);

	g->timer_index = g_timeout_add (speed, (GSourceFunc) load_graph_update, g);
}

void
load_graph_stop (LoadGraph *g)
{
	if (g->timer_index != -1)
		g_source_remove (g->timer_index);

	g->timer_index = -1;
}
