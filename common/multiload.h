/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *               1997 The Free Software Foundation
 *                    (Authors: Tim P. Gerla, Martin Baulig, Todd Kulesza)
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


#ifndef __MULTILOAD_H__
#define __MULTILOAD_H__

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

#include "gtk-compat.h"


#define MAX_COLORS 7

enum GraphType {
	GRAPH_CPULOAD,
	GRAPH_MEMLOAD,
	GRAPH_NETLOAD,
	GRAPH_SWAPLOAD,
	GRAPH_LOADAVG,
	GRAPH_DISKLOAD,
	GRAPH_TEMPERATURE,
	GRAPH_PARAMETRIC,

	GRAPH_MAX
};


typedef struct _LoadGraph LoadGraph;

typedef struct _GraphConfig {
	gboolean visible;
	gint border_width;
	GdkRGBA colors[MAX_COLORS];

	gint interval;
	gint size;
	gint tooltip_style;
	gint dblclick_policy;
	gchar dblclick_cmdline[1024];
} GraphConfig;

typedef struct _MultiloadPlugin {
	gpointer panel_data;
	GtkWidget *pref_dialog;

	GtkContainer *container;
	GtkWidget *box;
	GtkOrientation panel_orientation;

	LoadGraph *graphs[GRAPH_MAX];
	GraphConfig graph_config[GRAPH_MAX];
	gpointer extra_data[GRAPH_MAX]; // data depend on graph type

	gint padding;
	gint spacing;
	gboolean fill_between;
	gint orientation_policy;
	gchar color_scheme[20];
} MultiloadPlugin;


struct _LoadGraph {
	MultiloadPlugin *multiload;

	guint id;
	guint draw_width, draw_height;

	gint **data;
	guint *pos;

	char output_str[4][20];
	char output_unit[8];

	GtkWidget *main_widget;
	GtkWidget *box, *disp;
	cairo_surface_t *surface;
	int timer_index;

	gboolean allocated;
	gboolean tooltip_update;

	GraphConfig *config;
};


G_BEGIN_DECLS

G_GNUC_INTERNAL void
multiload_refresh(MultiloadPlugin *ma);
G_GNUC_INTERNAL GtkOrientation
multiload_get_orientation(MultiloadPlugin *ma);
G_GNUC_INTERNAL void
multiload_tooltip_update(LoadGraph *g);
G_GNUC_INTERNAL void
multiload_set_fill_between (MultiloadPlugin *ma, gboolean fill);
G_GNUC_INTERNAL void
multiload_set_spacing (MultiloadPlugin *ma, gint val);
G_GNUC_INTERNAL void
multiload_set_padding (MultiloadPlugin *ma, gint val);
G_GNUC_INTERNAL void
multiload_set_max_value (MultiloadPlugin *ma, guint graph_id, int val);
G_GNUC_INTERNAL int
multiload_get_max_value (MultiloadPlugin *ma, guint graph_id);
G_GNUC_INTERNAL void
multiload_refresh_orientation (MultiloadPlugin *ma);
G_GNUC_INTERNAL void
multiload_init();
G_GNUC_INTERNAL void
multiload_sanitize(MultiloadPlugin *ma);
G_GNUC_INTERNAL void
multiload_defaults(MultiloadPlugin *ma);
G_GNUC_INTERNAL MultiloadPlugin*
multiload_new();
G_GNUC_INTERNAL void
multiload_destroy(MultiloadPlugin *ma);
G_GNUC_INTERNAL int
multiload_find_graph_by_name(char *str, char **suffix);

G_END_DECLS

#endif /* __MULTILOAD_H__ */
