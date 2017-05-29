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


const MlGraphTypeInterface ML_PROVIDER_LOADAVG_IFACE = {
	.name			= "loadavg",
	.label			= N_("Load average"),
	.description	= N_("Shows system load average for a given interval."),

	.hue			= 0,

	.n_data			= 1,
	.dataset_mode	= ML_DATASET_MODE_ABSOLUTE,

	.init_fn		= ml_provider_loadavg_init,
	.config_fn		= NULL,
	.get_fn			= ml_provider_loadavg_get,
	.destroy_fn		= free,
	.sizeof_fn		= ml_provider_loadavg_sizeof,
	.unpause_fn		= NULL,
	.caption_fn		= ml_provider_loadavg_caption,
	
	.helptext		= NULL
};

const MlGraphTypeInterface ML_PROVIDER_LOADAVG_FULL_IFACE = {
	.name			= "loadavg_full",
	.label			= N_("Load average (full)"),
	.description	= N_("Shows system load average for all intervals."),

	.hue			= 0,

	.n_data			= 3,
	.dataset_mode	= ML_DATASET_MODE_INDEPENDENT,

	.init_fn		= NULL,
	.config_fn		= NULL,
	.get_fn			= ml_provider_loadavg_get,
	.destroy_fn		= NULL,
	.sizeof_fn		= NULL,
	.unpause_fn		= NULL,
	.caption_fn		= ml_provider_loadavg_caption,

	.helptext		= NULL
};


typedef struct {
	int32_t avg_time; // can be 0,1,2 (enum below)
} LOADAVGstate;

enum { LOADAVG_1_MIN = 0, LOADAVG_5_MIN = 1, LOADAVG_15_MIN = 2 };

mlPointer
ml_provider_loadavg_init (MlConfig *config)
{
	LOADAVGstate *s = ml_new (LOADAVGstate);
	s->avg_time = LOADAVG_1_MIN;

	ml_config_add_entry_with_bounds (config,
		"avg_time",
		ML_VALUE_TYPE_INT32,
		&s->avg_time,
		_("Average time"),
		_("Specify which load average to draw. Enter <b>0</b> for last minute, <b>1</b> for last 5 minutes, or <b>2</b> for last 15 minutes."),
		0, 2
	);

	return s;
}

#define MULTIPLIER 1000

void
ml_provider_loadavg_get (MlGraphContext *context)
{
	double loadavg[3];

	int n = getloadavg(loadavg, 3);
	ml_graph_context_assert (context, n >= 0);
	if (ml_graph_context_get_n_data (context) == 3) {
		ml_graph_context_set_data (context, 0, MULTIPLIER * loadavg[0]);
		ml_graph_context_set_data (context, 1, MULTIPLIER * loadavg[1]);
		ml_graph_context_set_data (context, 2, MULTIPLIER * loadavg[2]);
	} else {
		LOADAVGstate *s = (LOADAVGstate*)ml_graph_context_get_provider_data (context);
		ml_graph_context_assert (context, s != NULL);

		ml_graph_context_set_data (context, 0, MULTIPLIER * loadavg[s->avg_time]);
	}

	ml_graph_context_set_max (context, MULTIPLIER * ml_cpu_info_get_cpu_count() * 3); // min ceiling = 3 x num CPU
}

size_t
ml_provider_loadavg_sizeof (mlPointer provider_data)
{
	LOADAVGstate *s = (LOADAVGstate*)provider_data;
	if_unlikely (s == NULL)
		return 0;

	return sizeof (LOADAVGstate);
}

void
ml_provider_loadavg_caption (MlCaption *caption, MlDataset *ds, mlPointer provider_data)
{
	if (provider_data != NULL) { // ML_PROVIDER_LOADAVG_IFACE
		LOADAVGstate *s = (LOADAVGstate*)provider_data;

		uint32_t val = ml_dataset_get_value (ds, -1, 0);
		switch (s->avg_time) {
			case 0:
				ml_caption_set (caption, ML_CAPTION_COMPONENT_BODY, _("Load average for last minute: %0.02f"), (float)val / MULTIPLIER);
				break;
			case 1:
				ml_caption_set (caption, ML_CAPTION_COMPONENT_BODY, _("Load average for last 5 minutes: %0.02f"), (float)val / MULTIPLIER);
				break;
			case 2:
				ml_caption_set (caption, ML_CAPTION_COMPONENT_BODY, _("Load average for last 15 minutes: %0.02f"), (float)val / MULTIPLIER);
				break;
			default:
				ml_bug ("Unexpected value for config element 'avg_time': %d", s->avg_time);
				return;
		}

	} else { // ML_PROVIDER_LOADAVG_FULL_IFACE
		uint32_t val1 = ml_dataset_get_value (ds, -1, 0);
		uint32_t val5 = ml_dataset_get_value (ds, -1, 1);
		uint32_t val15 = ml_dataset_get_value (ds, -1, 2);

		ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Load average for last minute: %0.02f"),     (float)val1 / MULTIPLIER);
		ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");
		ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Load average for last 5 minutes: %0.02f"),  (float)val5 / MULTIPLIER);
		ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");
		ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Load average for last 15 minutes: %0.02f"), (float)val15 / MULTIPLIER);
	}
}
