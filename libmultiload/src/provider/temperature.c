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


const MlGraphTypeInterface ML_PROVIDER_TEMPERATURE_IFACE = {
	.name			= "temperature",
	.label			= N_("Temperature"),
	.description	= N_("Shows temperature from one or more sensors."),

	.hue			= 310,

	.n_data			= 2,
	.dataset_mode	= ML_DATASET_MODE_ABSOLUTE,

	.init_fn		= ml_provider_temperature_init,
	.config_fn		= NULL,
	.get_fn			= ml_provider_temperature_get,
	.destroy_fn		= ml_provider_temperature_destroy,
	.sizeof_fn		= ml_provider_temperature_sizeof,
	.unpause_fn		= NULL,
	.caption_fn		= ml_provider_temperature_caption,

	.helptext		= NULL
};


typedef struct {
	MlGrowArray *aa_sources;

	char *temp_label;
	double temp_current;
	double temp_critical;

	bool average_mode;
	char **filter;
} TEMPERATUREstate;


mlPointer
ml_provider_temperature_init (MlConfig *config)
{
	MlGrowArray *aa_sources = ml_temperature_info_get_available ();
	if (aa_sources == NULL)
		return NULL;

	TEMPERATUREstate *s = ml_new (TEMPERATUREstate);

	s->aa_sources = aa_sources;
	s->average_mode = false;
	s->filter = NULL;

	ml_config_add_entry (config,
		"average_mode",
		ML_VALUE_TYPE_BOOLEAN,
		&s->average_mode,
		_("Calculate average"),
		_("If <b>true</b>, graph will draw average temperature of available sources. If <b>false</b>, graph will draw maximum temperature of available sources.")
	);
	ml_config_add_entry (config,
		"filter",
		ML_VALUE_TYPE_STRV,
		&s->filter,
		_("Filter"),
		_("Temperature sources that aren't in this list will not be used. If this list is empty, all available temperature sources will be used.")
	);

	return s;
}

#define MULTIPLIER 10

void
ml_provider_temperature_get (MlGraphContext *context)
{
	TEMPERATUREstate *s = (TEMPERATUREstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	ml_graph_context_assert_with_message (context, !ml_grow_array_is_empty (s->aa_sources), _("Cannot find any temperature sensor"));

	double temp = 0;
	double crit = DBL_MAX;

	int n_sources = 0;
	s->temp_label = NULL;

	ml_grow_array_for (s->aa_sources, i) {
		MlTemperatureInfo *ti = (MlTemperatureInfo*)ml_grow_array_get (s->aa_sources, i);
		if_unlikely (ti == NULL)
			break;

		// skip sources not present in filter
		if (s->filter != NULL && !ml_strv_contains (s->filter, ti->id))
			continue;

		ml_graph_context_assert_with_message (context, ml_temperature_info_update (ti), _("Cannot update temperature data for %s"), ti->id);

		if (s->average_mode) { // avg mode
			temp += ti->current_temp;

			// in average mode, critical temperature is the lowest
			if (ti->critical_temp > 0 && ti->critical_temp < crit)
				crit = ti->critical_temp;

			n_sources ++;
		} else { // max mode
			if (ti->current_temp > temp) {
				temp = ti->current_temp;
				crit = ti->critical_temp;
				s->temp_label = ti->label;
			}
		}

	}

	if (s->average_mode) {
		ml_graph_context_assert_with_message (context, n_sources > 0, _("All usable temperature sources are filtered out or outdated"));
		temp /= n_sources;
	}

	s->temp_current = temp;
	s->temp_critical = crit;

	if (crit < DBL_MAX && crit > 0 && temp > crit) {
		ml_graph_context_set_data (context, 0, MULTIPLIER * crit);
		ml_graph_context_set_data (context, 1, MULTIPLIER * (temp-crit));
	} else {
		ml_graph_context_set_data (context, 0, MULTIPLIER * temp);
		ml_graph_context_set_max  (context, MULTIPLIER * crit);
	}

}

void
ml_provider_temperature_destroy (mlPointer provider_data)
{
	TEMPERATUREstate *s = (TEMPERATUREstate*)provider_data;

	if_likely (s != NULL) {
		ml_strv_free (s->filter);
		ml_grow_array_destroy (s->aa_sources, true, false);

		free (s);
	}
}

size_t
ml_provider_temperature_sizeof (mlPointer provider_data)
{
	TEMPERATUREstate *s = (TEMPERATUREstate*)provider_data;
	if_unlikely (s == NULL)
		return 0;

	size_t size = sizeof (TEMPERATUREstate);

	size += ml_grow_array_sizeof (s->aa_sources, 0);
	size += ml_strv_sizeof (s->filter);

	return size;
}

void
ml_provider_temperature_caption (MlCaption *caption, ML_UNUSED MlDataset *ds, mlPointer provider_data)
{
	TEMPERATUREstate *s = (TEMPERATUREstate*)provider_data;
	if_unlikely (s == NULL)
		return;

	if (s->average_mode) {
		// TRANSLATORS: multiple temperature sources
		ml_caption_set (caption, ML_CAPTION_COMPONENT_HEADER, _("Average temperature from %zu sources"), ml_grow_array_get_length (s->aa_sources));
	} else {
		// TRANSLATORS: single temperature source
		ml_caption_set (caption, ML_CAPTION_COMPONENT_HEADER, _("Temperature source: %s"), s->temp_label);
	}

	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Current temperature: %0.01f °C"), s->temp_current);
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Critical temperature: %0.01f °C"), s->temp_critical);
}
