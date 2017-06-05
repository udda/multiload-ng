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


const MlGraphTypeInterface ML_PROVIDER_PROCRATE_IFACE = {
	.name			= "procrate",
	.label			= N_("Process creation rate"),
	.description	= N_("Shows number of new processes/threads."),

	.hue			= 249,

	.n_data			= 1,
	.dataset_mode	= ML_DATASET_MODE_ABSOLUTE,

	.init_fn		= ml_provider_procrate_init,
	.config_fn		= NULL,
	.get_fn			= ml_provider_procrate_get,
	.destroy_fn		= free,
	.sizeof_fn		= ml_provider_procrate_sizeof,
	.unpause_fn		= ml_provider_procrate_unpause,
	.caption_fn		= ml_provider_procrate_caption,

	.helptext		= NULL
};


typedef struct {
	uint64_t last;
} PROCRATEstate;


mlPointer
ml_provider_procrate_init (ML_UNUSED MlConfig *config)
{
	PROCRATEstate *s = ml_new (PROCRATEstate);
	s->last = 0;

	return s;
}


void
ml_provider_procrate_get (MlGraphContext *context)
{
	PROCRATEstate *s = (PROCRATEstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	uint64_t processes;

	char *row = ml_cpu_info_read_stat_row ("processes");
	ml_graph_context_assert_with_message (context, row != NULL, _("Processes stats do not exist in this system"));

	size_t n = sscanf (row, "processes %"SCNu64, &processes);
	free (row);
	ml_graph_context_assert_with_message (context, n == 1, _("Wrong line format"));

	if_unlikely (!ml_graph_context_is_first_call (context)) // cannot calculate diff on first call
		ml_graph_context_set_data (context, 0, (uint32_t)(processes - s->last));

	ml_graph_context_set_max (context, 5); // min ceiling = 5

	s->last = processes;
}

size_t
ml_provider_procrate_sizeof (mlPointer provider_data)
{
	PROCRATEstate *s = (PROCRATEstate*)provider_data;
	if_unlikely (s == NULL)
		return 0;

	return sizeof (PROCRATEstate);
}

void
ml_provider_procrate_unpause (MlGraphContext *context)
{
	ml_graph_context_set_first_call (context); // avoid spikes after unpause
}

void
ml_provider_procrate_caption (MlCaption *caption, MlDataset *ds, ML_UNUSED mlPointer provider_data)
{
	uint32_t val = ml_dataset_get_value (ds, -1, 0);
	ml_caption_set (caption, ML_CAPTION_COMPONENT_BODY, _("%u new processes since last update"), val);
}
