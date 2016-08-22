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
		{	"cpu",				_("Processor"),		GetCpu,
			7, { // hue: 196
				"#FF036F96",		// User
				"#FF42ACD1",		// System
				"#FFBEEEFF",		// Nice
				"#FF002633",		// IOWait
				"#FF005D80",		// Border
				"#FF303030",		// Background (top)
				"#FF000000"			// Background (bottom)
			}
		},

		{	"mem",				_("Memory"),		GetMemory,
			7, { // hue: 151
				"#FF03964F",		// User
				"#FF43D18D",		// Shared
				"#FFBFFFE0",		// Buffers
				"#FF00331A",		// Cached
				"#FF008042",		// Border
				"#FF303030",		// Background (top)
				"#FF000000"			// Background (bottom)
			}
		},

		{	"net",				_("Network"),		GetNet,
			6, { // hue: 53
				"#FFE2CC05",		// In
				"#FF696018",		// Out
				"#FFFFF7B1",		// Local
				"#FF807100",		// Border
				"#FF303030",		// Background (top)
				"#FF000000"			// Background (bottom)
			}
		},

		{	"swap",				_("Swap"),			GetSwap,
			4, { // hue: 278
				"#FF9C43D1",		// Used
				"#FF510080",		// Border
				"#FF303030",		// Background (top)
				"#FF000000"			// Background (bottom)
			}
		},

		{	"load",				_("Load average"),	GetLoadAvg,
			4, { // hue: 0
				"#FFD14343",		// Average
				"#FF800000",		// Border
				"#FF303030",		// Background (top)
				"#FF000000"			// Background (bottom)
			}
		},

		{	"disk",				_("Disk"),			GetDisk,
			5, { // hue: 31
				"#FFED7A00",		// Read
				"#FFFF6700",		// Write
				"#FF804200",		// Border
				"#FF303030",		// Background (top)
				"#FF000000"			// Background (bottom)
			}
		},

		{	"temperature",		_("Temperature"),	GetTemperature,
			4, { // hue: 310
				"#FFF049D5",		// Value
				"#FF80006B",		// Border
				"#FF303030",		// Background (top)
				"#FF000000"			// Background (bottom)
			}
		}
	};

	memcpy(&graph_types, &temp, sizeof(graph_types));

}