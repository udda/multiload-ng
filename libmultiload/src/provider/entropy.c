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


const MlGraphTypeInterface ML_PROVIDER_ENTROPY_IFACE = {
	.name			= "entropy",
	.label			= N_("Available entropy"),
	.description	= N_("Shows system available entropy."),

	.hue			= 73,

	.n_data			= 1,
	.dataset_mode	= ML_DATASET_MODE_ABSOLUTE,

	.init_fn		= NULL,
	.config_fn		= NULL,
	.get_fn			= ml_provider_entropy_get,
	.destroy_fn		= NULL,
	.sizeof_fn		= NULL,
	.unpause_fn		= NULL,
	.caption_fn		= ml_provider_entropy_caption,

	.helptext		= NULL
};


#define PATH_ENTROPY "/proc/sys/kernel/random/entropy_avail"

void
ml_provider_entropy_get (MlGraphContext *context)
{
	uint32_t n;

	ml_graph_context_assert_with_message (context, ml_infofile_read_uint32 (PATH_ENTROPY, &n), _("Entropy data is invalid or missing"));

	ml_graph_context_set_data (context, 0, n);
}

void
ml_provider_entropy_caption (MlCaption *caption, MlDataset *ds, ML_UNUSED mlPointer provider_data)
{
	uint32_t val = ml_dataset_get_value (ds, -1, 0);
	ml_caption_set (caption, ML_CAPTION_COMPONENT_BODY, _("Available entropy: %u bytes"), val);
}
