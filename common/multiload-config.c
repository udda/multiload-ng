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
		{	"cpuload",			_("_Processor"),		_("Processor"),		GetCpu,
			7, { // hue: 196
				{ _("_User"),					_("User"),					"#FF036F96" },
				{ _("_System"),					_("System"),				"#FF42ACD1" },
				{ _("N_ice"),					_("Nice"),					"#FFBEEEFF" },
				{ _("I_OWait"),					_("IOWait"),				"#FF002633" },
				{ _("Bor_der"),					_("Border"),				"#FF005D80" },
				{ _("_Background (top)"),		_("Background (top)"),		"#FF303030" },
				{ _("_Background (bottom)"),	_("Background (bottom)"),	"#FF000000" }
			}
		},

		{	"memload",			_("_Memory"),			_("Memory"),		GetMemory,
			7, { // hue: 151
				{ _("_User"),					_("User"),					"#FF03964F" },
				{ _("_Shared"),					_("Shared"),				"#FF43D18D" },
				{ _("_Buffers"),				_("Buffers"),				"#FFBFFFE0" },
				{ _("Cach_ed"),					_("Cached"),				"#FF00331A" },
				{ _("Bor_der"),					_("Border"),				"#FF008042" },
				{ _("_Background (top)"),		_("Background (top)"),		"#FF303030" },
				{ _("_Background (bottom)"),	_("Background (bottom)"),	"#FF000000" }
			}
		},

		{	"netload",			_("_Network"),			_("Network"),		GetNet,
			6, { // hue: 53
				{ _("_In"),						_("In"),					"#FFE2CC05" },
				{ _("O_ut"),					_("Out"),					"#FF696018" },
				{ _("L_ocal"),					_("Local"),					"#FFFFF7B1" },
				{ _("Bor_der"),					_("Border"),				"#FF807100" },
				{ _("_Background (top)"),		_("Background (top)"),		"#FF303030" },
				{ _("_Background (bottom)"),	_("Background (bottom)"),	"#FF000000" }
			}
		},

		{	"swapload",			_("S_wap"),				_("Swap"),			GetSwap,
			4, { // hue: 278
				{ _("_Used"),					_("Used"),					"#FF9C43D1" },
				{ _("Bor_der"),					_("Border"),				"#FF510080" },
				{ _("_Background (top)"),		_("Background (top)"),		"#FF303030" },
				{ _("_Background (bottom)"),	_("Background (bottom)"),	"#FF000000" }
			}
		},

		{	"loadavg",			_("_Load avg."),		_("Load average"),	GetLoadAvg,
			4, { // hue: 0
				{ _("A_verage")	,				_("Average"),				"#FFD14343" },
				{ _("Bor_der"),					_("Border"),				"#FF800000" },
				{ _("_Background (top)"),		_("Background (top)"),		"#FF303030" },
				{ _("_Background (bottom)"),	_("Background (bottom)"),	"#FF000000" }
			}
		},

		{	"diskload",			_("_Disk"),				_("Disk"),			GetDisk,
			5, { // hue: 31
				{ _("_Read"),			_("Read"),			"#FFED7A00" },
				{ _("Wr_ite"),					_("Write"),					"#FFFF6700" },
				{ _("Bor_der"),					_("Border"),				"#FF804200" },
				{ _("_Background (top)"),		_("Background (top)"),		"#FF303030" },
				{ _("_Background (bottom)"),	_("Background (bottom)"),	"#FF000000" }
			}
		},

		{	"temperature",		_("_Temperature"),		_("Temperature"),	GetTemperature,
			4, { // hue: 310
				{ _("_Value"),					_("Value"),					"#FFF049D5" },
				{ _("Bor_der"),					_("Border"),				"#FF80006B" },
				{ _("_Background (top)"),		_("Background (top)"),		"#FF303030" },
				{ _("_Background (bottom)"),	_("Background (bottom)"),	"#FF000000" }
			}
		}
	};

	memcpy(&graph_types, &temp, sizeof(graph_types));

}