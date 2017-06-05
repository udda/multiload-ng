/*
 * Copyright (C) 2017 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of Multiload-ng.
 *
 * Multiload-ng is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Multiload-ng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Multiload-ng.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <multiload.h>


const MlGraphTypeInterface ML_PROVIDER_SWAP_IFACE = {
	.name			= "swap",
	.label			= N_("Swap usage"),
	.description	= N_("Shows swap usage."),

	.hue			= 278,

	.n_data			= 1,
	.dataset_mode	= ML_DATASET_MODE_PROPORTIONAL,

	.init_fn		= NULL,
	.config_fn		= NULL,
	.get_fn			= ml_provider_swap_get,
	.destroy_fn		= NULL,
	.sizeof_fn		= NULL,
	.unpause_fn		= NULL,
	.caption_fn		= ml_provider_swap_caption,

	.helptext		= NULL
};


#define PATH_MEMINFO "/proc/meminfo"

void
ml_provider_swap_get (MlGraphContext *context)
{
	static uint32_t kb_swap_total = 0;
	static uint32_t kb_swap_free = 0;

	static const MlInfofileMappingEntry table[] = {
		{ "SwapTotal",	'u',	&kb_swap_total },
		{ "SwapFree",	'u',	&kb_swap_free }
	};

	int r = ml_infofile_read_keys (PATH_MEMINFO, table, 2);
	ml_graph_context_assert_with_message (context, r == 2, _("Cannot retrieve one or more values"));

	// Leave these values as KB (this way maximum size representable with 32 bits is 4 TiB)
	ml_graph_context_set_data (context, 0, (kb_swap_total - kb_swap_free));
	ml_graph_context_set_max  (context, kb_swap_total);
}

void
ml_provider_swap_caption (MlCaption *caption, MlDataset *ds, ML_UNUSED mlPointer provider_data)
{
	uint32_t kb_used    = ml_dataset_get_value (ds, -1, 0);
	uint32_t kb_total   = ml_dataset_get_ceiling (ds, -1);

	char buf_used  [24];
	char buf_total [24];

	ml_string_format_size_s ((uint64_t)kb_used  * 1024, "B", false, buf_used,  sizeof (buf_used));
	ml_string_format_size_s ((uint64_t)kb_total * 1024, "B", false, buf_total, sizeof (buf_total));

	if (kb_total > 0) {
		ml_caption_set (caption, ML_CAPTION_COMPONENT_HEADER, _("Total swap: %s"), buf_total);
		// TRANSLATORS: used swap, absolute and percentage value
		ml_caption_set (caption, ML_CAPTION_COMPONENT_BODY, _("%s (%0.01f%%) used swap"), buf_used, (100.0 * kb_used / kb_total));
	} else {
		ml_caption_set (caption, ML_CAPTION_COMPONENT_BODY, _("No swap"));
	}
}
