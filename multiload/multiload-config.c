#include <config.h>

#include "multiload-config.h"

guint multiload_config_get_num_colors(guint id)
{
	g_assert (graph_types[id].num_colors <= MAX_COLORS);
	return graph_types[id].num_colors;
}

guint multiload_config_get_num_data(guint id)
{
	// Subtract bg color and border color. Remaining colors map 1:1 to graph data.
	return multiload_config_get_num_colors(id) - 2;
}

void multiload_config_init()
{
	gchar *str_background = g_strdup(_("_Background"));
	gchar *str_background_n = g_strdup(_("Background"));
	gchar *str_border = g_strdup(_("Bor_der"));
	gchar *str_border_n = g_strdup(_("Border"));

	GraphType temp[] = {
		{	"cpuload",			_("_Processor"),		_("Processor"),		GetCpu,
			6, { // hue: 196
				{ _("_User"),			_("User"),			"#FF036F96" },
				{ _("_System"),			_("System"),		"#FF42ACD1" },
				{ _("N_ice"),			_("Nice"),			"#FFBEEEFF" },
				{ _("I_OWait"),			_("IOWait"),		"#FF002633" },
				{ str_border,			str_border_n,		"#FF005D80" },
				{ str_background,		str_background_n,	"#FF000000" }
			}
		},

		{	"memload",			_("_Memory"),			_("Memory"),		GetMemory,
			6, { // hue: 151
				{ _("_User"),			_("User"),			"#FF03964F" },
				{ _("_Shared"),			_("Shared"),		"#FF43D18D" },
				{ _("_Buffers"),		_("Buffers"),		"#FFBFFFE0" },
				{ _("Cach_ed"),			_("Cached"),		"#FF00331A" },
				{ str_border,			str_border_n,		"#FF008042" },
				{ str_background,		str_background_n,	"#FF000000" }
			}
		},

		{	"netload",			_("_Network"),			_("Network"),		GetNet,
			5, { // hue: 53
				{ _("_In"),				_("In"),			"#FFE2CC05" },
				{ _("O_ut"),			_("Out"),			"#FF696018" },
				{ _("L_ocal"),			_("Local"),			"#FFFFF7B1" },
				{ str_border,			str_border_n,		"#FF807100" },
				{ str_background,		str_background_n,	"#FF000000" }
			}
		},

		{	"swapload",			_("S_wap"),				_("Swap"),			GetSwap,
			3, { // hue: 278
				{ _("_Used"),			_("Used"),			"#FF9C43D1" },
				{ str_border,			str_border_n,		"#FF510080" },
				{ str_background,		str_background_n,	"#FF000000" }
			}
		},

		{	"loadavg",			_("_Load average"),		_("Load average"),	GetLoadAvg,
			3, { // hue: 0
				{ _("A_verage"),		_("Average"),		"#FFD14343" },
				{ str_border,			str_border_n,		"#FF800000" },
				{ str_background,		str_background_n,	"#FF000000" }
			}
		},

		{	"diskload",			_("_Disk"),				_("Disk"),			GetDisk,
			4, { // hue: 31
				{ _("_Read"),			_("Read"),			"#FFED7A00" },
				{ _("Wr_ite"),			_("Write"),			"#FFFF6700" },
				{ str_border,			str_border_n,		"#FF804200" },
				{ str_background,		str_background_n,	"#FF000000" }
			}
		},

		{	"temperature",		_("_Temperature"),		_("Temperature"),	GetTemperature,
			3, { // hue: 310
				{ _("_Value"),			_("Value"),			"#FFF049D5" },
				{ str_border,			str_border_n,		"#FF80006B" },
				{ str_background,		str_background_n,	"#FF000000" }
			}
		}
	};

	memcpy(&graph_types, &temp, sizeof(graph_types));

}