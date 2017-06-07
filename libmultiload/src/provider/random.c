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


const MlGraphTypeInterface ML_PROVIDER_RANDOM_IFACE = {
	.name			= "random",
	.label			= N_("Random"),
	.description	= N_("Shows random numbers, in a given range."),

	.hue			= 178,

	.n_data			= 1,
	.dataset_mode	= ML_DATASET_MODE_PROPORTIONAL,

	.init_fn		= ml_provider_random_init,
	.config_fn		= ml_provider_random_config,
	.get_fn			= ml_provider_random_get,
	.destroy_fn		= ml_provider_random_destroy,
	.sizeof_fn		= ml_provider_random_sizeof,
	.unpause_fn		= NULL,
	.caption_fn		= ml_provider_random_caption,

	.helptext		= NULL
};


typedef struct {
	int64_t seed;
	int32_t min;
	int32_t max;
} RANDOMstate;


mlPointer
ml_provider_random_init (MlConfig *config)
{
	RANDOMstate *s = ml_new (RANDOMstate);
	s->seed = 0;
	s->min = 0;
	s->max = 100;

	ml_config_add_entry_with_bounds (config,
		"seed",
		ML_VALUE_TYPE_INT64,
		&s->seed,
		_("Seed"),
		_("Seed to use for initializing the random number generator. If <b>0</b>, a default seed will be used."),
		// seed is actually 32 bit long, we are storing it as a int64 because it's unsigned
		0, UINT32_MAX
	);
	ml_config_add_entry_with_bounds (config,
		"min",
		ML_VALUE_TYPE_INT32,
		&s->min,
		_("Minimum value"),
		_("Minimum allowed value for random numbers."),
		0, INT32_MAX-1
	);
	ml_config_add_entry_with_bounds (config,
		"max",
		ML_VALUE_TYPE_INT32,
		&s->max,
		_("Maximum value"),
		_("Maximum allowed value for random numbers."),
		1, INT32_MAX
	);

	return s;
}

void
ml_provider_random_config (MlGraphContext *context)
{
	RANDOMstate *s = (RANDOMstate*)ml_graph_context_get_provider_data (context);
	if_unlikely (s == NULL)
		return;

	if (s->max <= s->min)
		s->max = s->min + 1;
}


void
ml_provider_random_get (MlGraphContext *context)
{
	RANDOMstate *s = (RANDOMstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	int n_data = ml_graph_context_get_n_data (context);
	for (int i = 0; i < n_data; i++) {
		unsigned seed = (unsigned)s->seed;
		uint32_t v = s->min + (rand_r(&seed) * ((double)s->max / (double)RAND_MAX));
		s->seed = seed;
		ml_graph_context_set_data (context, i, v);
	}

	ml_graph_context_set_max (context, n_data * s->max);
}

void
ml_provider_random_destroy (mlPointer provider_data)
{
	RANDOMstate *s = (RANDOMstate*)provider_data;

	if_likely (s != NULL)
		free (s);
}

size_t
ml_provider_random_sizeof (mlPointer provider_data)
{
	RANDOMstate *s = (RANDOMstate*)provider_data;
	if_unlikely (s == NULL)
		return 0;

	return sizeof (RANDOMstate);
}

void
ml_provider_random_caption (MlCaption *caption, MlDataset *ds, mlPointer provider_data)
{
	RANDOMstate *s = (RANDOMstate*)provider_data;
	if_unlikely (s == NULL)
		return;

	uint32_t r = ml_dataset_get_value (ds, -1, 0);

	// TRANSLATORS: minimum and maximum possible values
	ml_caption_set (caption, ML_CAPTION_COMPONENT_HEADER, _("Range: [%ld,%ld]"), (long)s->min, (long)s->max);
	// TRANSLATORS: current random seed
	ml_caption_set (caption, ML_CAPTION_COMPONENT_FOOTER, _("Seed: %lu"), (unsigned long)s->seed);

	ml_caption_set (caption, ML_CAPTION_COMPONENT_BODY, _("Current random number: %ld"), (long)r);
}
