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

#include "linux-proc.h"
#include "multiload-colors.h"
#include "multiload-config.h"

guint multiload_config_get_num_colors(guint id)
{
	g_assert (graph_types[id].num_colors <= MAX_COLORS);
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
		{ "cpu",			_("Processor"),		GetCpu,			7 },
		{ "mem",			_("Memory"),		GetMemory,		7 },
		{ "net",			_("Network"),		GetNet,			6 },
		{ "swap",			_("Swap"),			GetSwap,		4 },
		{ "load",			_("Load average"),	GetLoadAvg,		4 },
		{ "disk",			_("Disk"),			GetDisk,		5 },
		{ "temperature",	_("Temperature"),	GetTemperature,	4 }
	};

	memcpy(&graph_types, &temp, sizeof(graph_types));
}