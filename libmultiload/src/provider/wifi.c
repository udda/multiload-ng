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


const MlGraphTypeInterface ML_PROVIDER_WIFI_IFACE = {
	.name			= "wifi",
	.label			= N_("WiFi"),
	.description	= N_("Shows WiFi link quality and signal strength."),

	.hue			= 140,

	.n_data			= 1,
	.dataset_mode	= ML_DATASET_MODE_ABSOLUTE,

	.init_fn		= ml_provider_wifi_init,
	.config_fn		= NULL,
	.get_fn			= ml_provider_wifi_get,
	.destroy_fn		= ml_provider_wifi_destroy,
	.sizeof_fn		= NULL,
	.unpause_fn		= NULL,
	.caption_fn		= ml_provider_wifi_caption,

	.helptext		= NULL
};


#define PATH_WIRELESS "/proc/net/wireless"


typedef struct {
	char *iface;
	bool use_strength;

	char used_iface[32];
	int link_quality;
	int signal_strength;
} WIFIstate;


mlPointer
ml_provider_wifi_init (MlConfig *config)
{
	WIFIstate *s = ml_new (WIFIstate);
	s->iface = NULL;
	s->use_strength = false;

	ml_config_add_entry (config,
		"iface",
		ML_VALUE_TYPE_STRING,
		&s->iface,
		_("Wireless interface"),
		_("Interface to use for measuring. If empty or non existing, the first available interface will be used.")
	);

	ml_config_add_entry (config,
		"use_strength",
		ML_VALUE_TYPE_BOOLEAN,
		&s->use_strength,
		_("Use signal strength"),
		_("Show signal strength instead of link quality.")
	);

	return s;
}

void
ml_provider_wifi_get (MlGraphContext *context)
{
	WIFIstate *s = (WIFIstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	FILE *f = fopen (PATH_WIRELESS, "r");
	ml_graph_context_assert_with_message (context, f != NULL, _("No wireless data in this system"));

	// skip header (2 rows, 80 chars each + newlines)
	fseek (f, 162, SEEK_SET);

	// test for EOF
	int tmp = fgetc(f);
	if (tmp == EOF) {
		// no wireless devices
		fclose(f);
	} else {
		// move back to original position
		fseek (f, 162, SEEK_SET);
	}
	ml_graph_context_assert_with_message (context, tmp != EOF, _("Cannot find any active wireless device"));

	//TODO interface selection (s->iface)

	size_t n = fscanf (f, "%32[^:]: %*X %d%*[. ] %d%*[^\n]", s->used_iface, &s->link_quality, &s->signal_strength);
	fclose (f);
	ml_graph_context_assert_with_message (context, n == 3, _("Wrong line format"));

	/* We have to make some wild guesses about gathered values.
	 * Here is what we know for sure (from iwconfig sources):
	 * - values are all 8 bit integers. 8 bit arithmetic is used
	 * - range for relative/percent values is [0 ... 255]
	 * - range for absolute/dBm values is [-192 ... 63]
	 * - link quality is always relative, actual max value is driver-dependent
	 * - signal strength may be absolute or relative, actual min value is driver-dependent
	 */
	if (s->use_strength) {
		if (s->signal_strength >= 0) {
			ml_graph_context_set_data (context, 0, s->signal_strength);
		} else { // power in dBm
			ml_graph_context_set_data (context, 0, s->signal_strength + 192);
		}
	} else {
		ml_graph_context_set_data (context, 0, s->link_quality);
	}

	ml_graph_context_set_max  (context, 255);
}

void
ml_provider_wifi_destroy (mlPointer provider_data)
{
	WIFIstate *s = (WIFIstate*)provider_data;
	if_unlikely (s == NULL)
		return;

	free (s->iface);
	free (s);
}

void
ml_provider_wifi_caption (MlCaption *caption, ML_UNUSED MlDataset *ds, mlPointer provider_data)
{
	WIFIstate *s = (WIFIstate*)provider_data;
	if_unlikely (s == NULL)
		return;

	// TRANSLATORS: selected wireless interface
	ml_caption_set (caption, ML_CAPTION_COMPONENT_HEADER, _("Wireless interface: %s"), s->used_iface);

	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Link quality: %d"), s->link_quality);
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Signal strength: %d"), s->signal_strength);
}
