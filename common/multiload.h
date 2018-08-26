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
	GRAPH_BATTERY,
	GRAPH_PARAMETRIC,

	GRAPH_MAX
};


typedef struct _LoadGraph LoadGraph;
typedef void (*GraphUpdateFunc)	(LoadGraph *g, gpointer user_data);

typedef struct _GraphConfig {
	gboolean visible;
	gint border_width;
	GdkRGBA colors[MAX_COLORS];
	gint bg_direction;

	gint interval;
	gint size;
	gint tooltip_style;
	gint dblclick_policy;
	gchar dblclick_cmdline[1024];

	gchar filter[150];
	gboolean filter_enable;
} GraphConfig;

typedef struct _MultiloadPlugin {
	gpointer panel_data;
	GtkWidget *pref_dialog;
	gboolean pref_dialog_maximized;
	gint pref_dialog_width;
	gint pref_dialog_height;

	GtkContainer *container;
	GtkWidget *box;
	GtkOrientation panel_orientation;

	LoadGraph *graphs[GRAPH_MAX];
	GraphConfig graph_config[GRAPH_MAX];
	gpointer extra_data[GRAPH_MAX]; // data depend on graph type

	gint padding;
	gint spacing;
	gint orientation_policy;
	gchar color_scheme[20];
	gboolean size_format_iec;
	gint graph_order[GRAPH_MAX];
} MultiloadPlugin;


struct _LoadGraph {
	MultiloadPlugin *multiload;

	guint id;
	guint draw_width, draw_height;

	gint **data;
	guint *pos;

	char output_str[4][20];

	GtkWidget *main_widget;
	GtkWidget *box, *disp;
	cairo_surface_t *surface;
	int timer_index;

	gboolean allocated;
	gboolean tooltip_update;
	gboolean filter_changed;
	gboolean first_update;

	GraphUpdateFunc update_cb;
	gpointer update_cb_user_data;

	GraphConfig *config;
};

typedef struct {
	gboolean show_about;
	gboolean show_preferences;
	gboolean reset_settings;
} MultiloadOptions;

typedef enum {
	MULTILOAD_GRADIENT_LINEAR_N_TO_S,
	MULTILOAD_GRADIENT_LINEAR_NE_TO_SW,
	MULTILOAD_GRADIENT_LINEAR_E_TO_W,
	MULTILOAD_GRADIENT_LINEAR_SE_TO_NW,
	MULTILOAD_GRADIENT_LINEAR_S_TO_N,
	MULTILOAD_GRADIENT_LINEAR_SW_TO_NE,
	MULTILOAD_GRADIENT_LINEAR_W_TO_E,
	MULTILOAD_GRADIENT_LINEAR_NW_TO_SE,
	MULTILOAD_GRADIENT_RADIAL,

	MULTILOAD_GRADIENT_MAX
} MultiloadGradientDirection;


G_BEGIN_DECLS

void _debug(const char *fmt, ...);

G_GNUC_INTERNAL void
multiload_start(MultiloadPlugin *ma);
G_GNUC_INTERNAL GtkOrientation
multiload_get_orientation(MultiloadPlugin *ma);
G_GNUC_INTERNAL void
multiload_tooltip_update(LoadGraph *g);
G_GNUC_INTERNAL void
multiload_set_spacing (MultiloadPlugin *ma, gint val);
G_GNUC_INTERNAL void
multiload_set_padding (MultiloadPlugin *ma, gint val);
G_GNUC_INTERNAL void
multiload_set_max_value (MultiloadPlugin *ma, guint graph_id, int val);
G_GNUC_INTERNAL int
multiload_get_max_value (MultiloadPlugin *ma, guint graph_id);
G_GNUC_INTERNAL gint
multiload_calculate_size_request (MultiloadPlugin *ma);
G_GNUC_INTERNAL void
multiload_refresh_orientation (MultiloadPlugin *ma);
G_GNUC_INTERNAL void
multiload_set_order (MultiloadPlugin *ma, gint new_order[GRAPH_MAX]);
G_GNUC_INTERNAL void
multiload_init();
G_GNUC_INTERNAL void
multiload_sanitize(MultiloadPlugin *ma);
G_GNUC_INTERNAL void
multiload_set_update_cb (MultiloadPlugin *ma, guint graph_id, GraphUpdateFunc callback, gpointer user_data);
G_GNUC_INTERNAL void
multiload_defaults(MultiloadPlugin *ma);
G_GNUC_INTERNAL MultiloadPlugin*
multiload_new();
G_GNUC_INTERNAL void
multiload_free(MultiloadPlugin *ma);
G_GNUC_INTERNAL int
multiload_find_graph_by_name(char *str, char **suffix);

G_END_DECLS

#endif /* __MULTILOAD_H__ */
