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

#include "colors.h"
#include "graph-data.h"
#include "multiload-config.h"

GraphType graph_types[GRAPH_MAX];

guint multiload_config_get_num_colors(guint id)
{
	g_assert_cmpuint (graph_types[id].num_colors, <=, MAX_COLORS);
	return graph_types[id].num_colors;
}

guint multiload_config_get_num_data(guint id)
{
	// Subtract colors that do not map 1:1 with graph data
	return multiload_config_get_num_colors(id) - EXTRA_COLORS;
}

void multiload_config_init()
{
	GraphType temp[] = {
		{	"cpu",	_("Processor"),		7,	-1,		-1,		"%",
			(GraphInitFunc)				multiload_graph_cpu_init,
			(GraphGetDataFunc)			multiload_graph_cpu_get_data,
			(GraphTooltipUpdateFunc)	multiload_graph_cpu_tooltip_update,
			(GraphCmdlineOutputFunc)	multiload_graph_cpu_cmdline_output,
			(GraphGetFilterFunc)		NULL
		},
		{	"mem",	_("Memory"),		7,	-1,		-1,		"byte",
			(GraphInitFunc)				NULL,
			(GraphGetDataFunc)			multiload_graph_mem_get_data,
			(GraphTooltipUpdateFunc)	multiload_graph_mem_tooltip_update,
			(GraphCmdlineOutputFunc)	multiload_graph_mem_cmdline_output,
			(GraphGetFilterFunc)		NULL
		},
		{	"net",	_("Network"),		6,	-1,		500,	"Bps",
			(GraphInitFunc)				multiload_graph_net_init,
			(GraphGetDataFunc)			multiload_graph_net_get_data,
			(GraphTooltipUpdateFunc)	multiload_graph_net_tooltip_update,
			(GraphCmdlineOutputFunc)	multiload_graph_net_cmdline_output,
			(GraphGetFilterFunc)		multiload_graph_net_get_filter
		},
		{	"swap",	_("Swap"),			4,	-1,		-1,		"byte",
			(GraphInitFunc)				NULL,
			(GraphGetDataFunc)			multiload_graph_swap_get_data,
			(GraphTooltipUpdateFunc)	multiload_graph_swap_tooltip_update,
			(GraphCmdlineOutputFunc)	multiload_graph_swap_cmdline_output,
			(GraphGetFilterFunc)		NULL
		},
		{	"load",	_("Load average"),	4,	8,		3,		"",
			(GraphInitFunc)				multiload_graph_load_init,
			(GraphGetDataFunc)			multiload_graph_load_get_data,
			(GraphTooltipUpdateFunc)	multiload_graph_load_tooltip_update,
			(GraphCmdlineOutputFunc)	multiload_graph_load_cmdline_output,
			(GraphGetFilterFunc)		NULL
		},
		{	"disk",	_("Disk"),			5,	-1,		500,	"Bps",
			(GraphInitFunc)				NULL,
			(GraphGetDataFunc)			multiload_graph_disk_get_data,
			(GraphTooltipUpdateFunc)	multiload_graph_disk_tooltip_update,
			(GraphCmdlineOutputFunc)	multiload_graph_disk_cmdline_output,
			(GraphGetFilterFunc)		multiload_graph_disk_get_filter
		},
		{	"temp",	_("Temperature"),	5,	120,	60,		"°C",
			(GraphInitFunc)				multiload_graph_temp_init,
			(GraphGetDataFunc)			multiload_graph_temp_get_data,
			(GraphTooltipUpdateFunc)	multiload_graph_temp_tooltip_update,
			(GraphCmdlineOutputFunc)	multiload_graph_temp_cmdline_output,
			(GraphGetFilterFunc)		multiload_graph_temp_get_filter
		},
		{	"bat",	_("Battery"),		6,	-1,		-1,		"%",
			(GraphInitFunc)				multiload_graph_bat_init,
			(GraphGetDataFunc)			multiload_graph_bat_get_data,
			(GraphTooltipUpdateFunc)	multiload_graph_bat_tooltip_update,
			(GraphCmdlineOutputFunc)	multiload_graph_bat_cmdline_output,
			(GraphGetFilterFunc)		NULL
		},
		{	"parm",	_("Parametric"),	7,	-1,		-1,		"",
			(GraphInitFunc)				NULL,
			(GraphGetDataFunc)			multiload_graph_parm_get_data,
			(GraphTooltipUpdateFunc)	multiload_graph_parm_tooltip_update,
			(GraphCmdlineOutputFunc)	multiload_graph_parm_cmdline_output,
			(GraphGetFilterFunc)		NULL
		}
	};

	memcpy(&graph_types, &temp, sizeof(graph_types));
}
