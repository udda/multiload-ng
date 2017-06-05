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


const MlGraphTypeInterface ML_PROVIDER_CTXTRATE_IFACE = {
	.name			= "ctxtrate",
	.label			= N_("Context switch rate"),
	.description	= N_("Shows number of context switch events."),

	.hue			= 335,

	.n_data			= 1,
	.dataset_mode	= ML_DATASET_MODE_ABSOLUTE,

	.init_fn		= ml_provider_ctxtrate_init,
	.config_fn		= NULL,
	.get_fn			= ml_provider_ctxtrate_get,
	.destroy_fn		= free,
	.sizeof_fn		= ml_provider_ctxtrate_sizeof,
	.unpause_fn		= ml_provider_ctxtrate_unpause,
	.caption_fn		= ml_provider_ctxtrate_caption,

	.helptext		= NULL
};


typedef struct {
	uint64_t last;
} CTXTRATEstate;


mlPointer
ml_provider_ctxtrate_init (MlConfig *config)
{
	CTXTRATEstate *s = ml_new (CTXTRATEstate);
	s->last = 0;

	return s;
}

void
ml_provider_ctxtrate_get (MlGraphContext *context)
{
	CTXTRATEstate *s = (CTXTRATEstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	uint64_t ctxt;

	char *row = ml_cpu_info_read_stat_row ("ctxt");
	ml_graph_context_assert_with_message (context, row != NULL, _("Context switch stats do not exist in this system"));

	size_t n = sscanf (row, "ctxt %"SCNu64, &ctxt);
	free (row);
	ml_graph_context_assert_with_message (context, n == 1, _("Wrong line format"));

	if_unlikely (!ml_graph_context_is_first_call (context)) // cannot calculate diff on first call
		ml_graph_context_set_data (context, 0, (uint32_t)(ctxt - s->last));

	ml_graph_context_set_max (context, 5); // min ceiling = 5

	s->last = ctxt;
}

size_t
ml_provider_ctxtrate_sizeof (mlPointer provider_data)
{
	CTXTRATEstate *s = (CTXTRATEstate*)provider_data;
	if_unlikely (s == NULL)
		return 0;

	return sizeof (CTXTRATEstate);
}

void
ml_provider_ctxtrate_unpause (MlGraphContext *context)
{
	ml_graph_context_set_first_call (context); // avoid spikes after unpause
}

void
ml_provider_ctxtrate_caption (MlCaption *caption, MlDataset *ds, ML_UNUSED mlPointer provider_data)
{
	uint32_t val = ml_dataset_get_value (ds, -1, 0);
	ml_caption_set (caption, ML_CAPTION_COMPONENT_BODY, _("%u context switches since last update"), val);
}
