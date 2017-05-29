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


const MlGraphTypeInterface ML_PROVIDER_BATTERY_IFACE = {
	.name			= "battery",
	.label			= N_("Battery"),
	.description	= N_("Shows battery charge and status (charging, full, critical level, ...)."),

	.hue			= 94,

	.n_data			= 3,
	.dataset_mode	= ML_DATASET_MODE_PROPORTIONAL,

	.init_fn		= ml_provider_battery_init,
	.config_fn		= NULL,
	.get_fn			= ml_provider_battery_get,
	.destroy_fn		= ml_provider_battery_destroy,
	.sizeof_fn		= ml_provider_battery_sizeof,
	.unpause_fn		= NULL,
	.caption_fn		= ml_provider_battery_caption,

	.helptext		= NULL
};


typedef struct {
	MlBatteryInfo *info;
	int32_t critical_level;
} BATTERYstate;


// multiplier for dataset values (better precision/granularity when drawing tall graphs)
#define MULTIPLIER 10

mlPointer
ml_provider_battery_init (MlConfig *config)
{
	MlBatteryInfo *battery = ml_battery_info_get_first_available ();
	if_unlikely (battery == NULL)
		return NULL;

	BATTERYstate *s = ml_new (BATTERYstate);
	s->info = battery;
	s->critical_level = 4;

	ml_config_add_entry_with_bounds (config,
		"critical_level",
		ML_VALUE_TYPE_INT32,
		&s->critical_level,
		_("Critical level"),
		_("Battery percentage to be reported as critical. Will be drawn in a different color."),
		0, 100
	);

	return s;
}

void
ml_provider_battery_get (MlGraphContext *context)
{
	BATTERYstate *s = (BATTERYstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	ml_graph_context_assert_with_message (context, ml_battery_info_update (s->info, (double)s->critical_level), _("Cannot update battery info"));
	ml_graph_context_assert_with_message (context, s->info->status != ML_BATTERY_STATUS_ABSENT, _("Battery not present"));

	uint8_t index;
	if (s->info->status == ML_BATTERY_STATUS_CHARGING || s->info->status == ML_BATTERY_STATUS_FULL)
		index = 0;	// first color: charging
	else if (!s->info->is_critical)
		index = 1;	// second color: not charging
	else
		index = 2;	// third color: critical level

	ml_graph_context_set_data (context, index, MULTIPLIER * s->info->percentage);
	ml_graph_context_set_max  (context, MULTIPLIER * 100);
}

void
ml_provider_battery_destroy (mlPointer provider_data)
{
	BATTERYstate *s = (BATTERYstate*)provider_data;
	if_likely (s != NULL) {
		ml_battery_info_destroy (s->info);
		free (s);
	}
}

size_t
ml_provider_battery_sizeof (mlPointer provider_data)
{
	BATTERYstate *s = (BATTERYstate*)provider_data;
	if_unlikely (s == NULL)
		return 0;

	size_t size = sizeof (BATTERYstate);
	size += ml_battery_info_sizeof (s->info);

	return size;
}

void
ml_provider_battery_caption (MlCaption *caption, ML_UNUSED MlDataset *ds, mlPointer provider_data)
{
	BATTERYstate *s = (BATTERYstate*)provider_data;
	if_unlikely (s == NULL || s->info == NULL)
		return;


	ml_caption_set (caption, ML_CAPTION_COMPONENT_HEADER, "%s\n", s->info->model_name);
	if (s->info->technology != NULL)
		ml_caption_append (caption, ML_CAPTION_COMPONENT_HEADER, _("Technology: %s"), s->info->technology);
	else
		ml_caption_append (caption, ML_CAPTION_COMPONENT_HEADER, _("Unknown technology"));

	ml_caption_append (caption, ML_CAPTION_COMPONENT_HEADER, "\n");

	if (s->info->manufacturer != NULL)
		ml_caption_append (caption, ML_CAPTION_COMPONENT_HEADER, _("Manufacturer: %s"), s->info->manufacturer);
	else
		ml_caption_append (caption, ML_CAPTION_COMPONENT_HEADER, _("Unknown manufacturer"));


	ml_caption_set (caption, ML_CAPTION_COMPONENT_BODY, "%s (%0.02f%%)", ml_battery_info_status_to_string (s->info->status), s->info->percentage);
	if (s->info->is_critical) {
		ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");
		ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Critical level"));
	}

	char buf[200];
	if (s->info->time_left != 0) {
		ml_string_format_time_s (s->info->time_left, buf, sizeof (buf));

		// TRANSLATORS: remaining time for full charge/discharge of the battery
		ml_caption_set (caption, ML_CAPTION_COMPONENT_FOOTER, _("Time left: %s"), buf);
	}
}
