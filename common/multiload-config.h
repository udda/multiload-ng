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


#ifndef __MULTILOAD_CONFIG_H__
#define __MULTILOAD_CONFIG_H__

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include "filter.h"
#include "gtk-compat.h"
#include "multiload.h"


G_BEGIN_DECLS

typedef void 				(*GraphInitFunc)			(LoadGraph *g, gpointer xd);
typedef void 				(*GraphGetDataFunc)			(int Maximum, int data[], LoadGraph *g, gpointer xd, gboolean first_call);
typedef void				(*GraphTooltipUpdateFunc)	(char *buf_title, size_t len_title, char *buf_text, size_t len_text, LoadGraph *g, gpointer xd, gint style);
typedef void				(*GraphCmdlineOutputFunc)	(LoadGraph *g, gpointer xd);
typedef void				(*GraphInlineOutputFunc)	(LoadGraph *g, gpointer xd);
typedef MultiloadFilter*	(*GraphGetFilterFunc)		(LoadGraph *g, gpointer xd);

typedef struct _GraphType {
	const char *name;
	const char *label;
	const guint num_colors;
	const gint scaler_max;			// fixed max value, or -1 for autoscaler
	const gint scaler_max_floor;	// fixed minimum value of scaler max, or -1 for default
	const gchar output_unit[10];
	const GraphInitFunc init;
	const GraphGetDataFunc get_data;
	const GraphTooltipUpdateFunc tooltip_update;
	const GraphCmdlineOutputFunc cmdline_output;
	const GraphInlineOutputFunc inline_output;
	const GraphGetFilterFunc get_filter;
} GraphType;


// global variable
GraphType graph_types[GRAPH_MAX];


G_GNUC_INTERNAL guint
multiload_config_get_num_colors(guint id);
G_GNUC_INTERNAL guint
multiload_config_get_num_data(guint id);
G_GNUC_INTERNAL void
multiload_config_init();

G_END_DECLS


#endif /* __MULTILOAD_CONFIG_H__ */
