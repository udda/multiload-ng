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
		{	"cpuload",			_("Processor"),		GetCpu,
			7, { // hue: 196
				{ _("User"),						"#FF036F96" },
				{ _("System"),						"#FF42ACD1" },
				{ _("Nice"),						"#FFBEEEFF" },
				{ _("IOWait"),						"#FF002633" },
				{ _("Border"),						"#FF005D80" },
				{ _("Background (top)"),			"#FF303030" },
				{ _("Background (bottom)"),			"#FF000000" }
			}
		},

		{	"memload",			_("Memory"),		GetMemory,
			7, { // hue: 151
				{ _("User"),						"#FF03964F" },
				{ _("Shared"),						"#FF43D18D" },
				{ _("Buffers"),						"#FFBFFFE0" },
				{ _("Cached"),						"#FF00331A" },
				{ _("Border"),						"#FF008042" },
				{ _("Background (top)"),			"#FF303030" },
				{ _("Background (bottom)"),			"#FF000000" }
			}
		},

		{	"netload",			_("Network"),		GetNet,
			6, { // hue: 53
				{ _("In"),							"#FFE2CC05" },
				{ _("Out"),							"#FF696018" },
				{ _("Local"),						"#FFFFF7B1" },
				{ _("Border"),						"#FF807100" },
				{ _("Background (top)"),			"#FF303030" },
				{ _("Background (bottom)"),			"#FF000000" }
			}
		},

		{	"swapload",			_("Swap"),			GetSwap,
			4, { // hue: 278
				{ _("Used"),						"#FF9C43D1" },
				{ _("Border"),						"#FF510080" },
				{ _("Background (top)"),			"#FF303030" },
				{ _("Background (bottom)"),			"#FF000000" }
			}
		},

		{	"loadavg",			_("Load average"),	GetLoadAvg,
			4, { // hue: 0
				{ _("Average"),						"#FFD14343" },
				{ _("Border"),						"#FF800000" },
				{ _("Background (top)"),			"#FF303030" },
				{ _("Background (bottom)"),			"#FF000000" }
			}
		},

		{	"diskload",			_("Disk"),			GetDisk,
			5, { // hue: 31
				{ _("Read"),						"#FFED7A00" },
				{ _("Write"),						"#FFFF6700" },
				{ _("Border"),						"#FF804200" },
				{ _("Background (top)"),			"#FF303030" },
				{ _("Background (bottom)"),			"#FF000000" }
			}
		},

		{	"temperature",		_("Temperature"),	GetTemperature,
			4, { // hue: 310
				{ _("Value"),						"#FFF049D5" },
				{ _("Border"),						"#FF80006B" },
				{ _("Background (top)"),			"#FF303030" },
				{ _("Background (bottom)"),			"#FF000000" }
			}
		}
	};

	memcpy(&graph_types, &temp, sizeof(graph_types));

}