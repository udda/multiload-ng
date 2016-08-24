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

#define HEX_TO_RGBA(r, g, b, a) {(gdouble)(0x##r)/255.0, (gdouble)(0x##g)/255.0, (gdouble)(0x##b)/255.0, (gdouble)(0x##a)/255.0}


void multiload_config_init()
{
	GraphType temp[] = {
		{	"cpu",				_("Processor"),		GetCpu,
			7, { // hue: 196
				HEX_TO_RGBA (03,6F,96, FF),		// User
				HEX_TO_RGBA (42,AC,D1, FF),		// System
				HEX_TO_RGBA (BE,EE,FF, FF),		// Nice
				HEX_TO_RGBA (00,26,33, FF),		// IOWait
				HEX_TO_RGBA (00,5D,80, FF),		// Border
				HEX_TO_RGBA (30,30,30, FF),		// Background (top)
				HEX_TO_RGBA (00,00,00, FF)		// Background (bottom)
			}
		},

		{	"mem",				_("Memory"),		GetMemory,
			7, { // hue: 151
				HEX_TO_RGBA (03,96,4F, FF),		// User
				HEX_TO_RGBA (43,D1,8D, FF),		// Shared
				HEX_TO_RGBA (BF,FF,E0, FF),		// Buffers
				HEX_TO_RGBA (00,33,1A, FF),		// Cached
				HEX_TO_RGBA (00,80,42, FF),		// Border
				HEX_TO_RGBA (30,30,30, FF),		// Background (top)
				HEX_TO_RGBA (00,00,00, FF)		// Background (bottom)
			}
		},

		{	"net",				_("Network"),		GetNet,
			6, { // hue: 53
				HEX_TO_RGBA (E2,CC,05, FF),		// In
				HEX_TO_RGBA (69,60,18, FF),		// Out
				HEX_TO_RGBA (FF,F7,B1, FF),		// Local
				HEX_TO_RGBA (80,71,00, FF),		// Border
				HEX_TO_RGBA (30,30,30, FF),		// Background (top)
				HEX_TO_RGBA (00,00,00, FF)		// Background (bottom)
			}
		},

		{	"swap",				_("Swap"),			GetSwap,
			4, { // hue: 278
				HEX_TO_RGBA (9C,43,D1, FF),		// Used
				HEX_TO_RGBA (51,00,80, FF),		// Border
				HEX_TO_RGBA (30,30,30, FF),		// Background (top)
				HEX_TO_RGBA (00,00,00, FF)		// Background (bottom)
			}
		},

		{	"load",				_("Load average"),	GetLoadAvg,
			4, { // hue: 0
				HEX_TO_RGBA (D1,43,43, FF),		// Average
				HEX_TO_RGBA (80,00,00, FF),		// Border
				HEX_TO_RGBA (30,30,30, FF),		// Background (top)
				HEX_TO_RGBA (00,00,00, FF)		// Background (bottom)
			}
		},

		{	"disk",				_("Disk"),			GetDisk,
			5, { // hue: 31
				HEX_TO_RGBA (ED,7A,00, FF),		// Read
				HEX_TO_RGBA (FF,67,00, FF),		// Write
				HEX_TO_RGBA (80,42,00, FF),		// Border
				HEX_TO_RGBA (30,30,30, FF),		// Background (top)
				HEX_TO_RGBA (00,00,00, FF)		// Background (bottom)
			}
		},

		{	"temperature",		_("Temperature"),	GetTemperature,
			4, { // hue: 310
				HEX_TO_RGBA (F0,49,D5, FF),		// Value
				HEX_TO_RGBA (80,00,6B, FF),		// Border
				HEX_TO_RGBA (30,30,30, FF),		// Background (top)
				HEX_TO_RGBA (00,00,00, FF)		// Background (bottom)
			}
		}
	};

	memcpy(&graph_types, &temp, sizeof(graph_types));

}