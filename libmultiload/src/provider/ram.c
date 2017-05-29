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


const MlGraphTypeInterface ML_PROVIDER_RAM_IFACE = {
	.name			= "ram",
	.label			= N_("RAM usage"),
	.description	= N_("Shows RAM usage, split by type (user processes, buffers, cache)."),

	.hue			= 151,

	.n_data			= 3,
	.dataset_mode	= ML_DATASET_MODE_PROPORTIONAL,

	.init_fn		= ml_provider_ram_init,
	.config_fn		= NULL,
	.get_fn			= ml_provider_ram_get,
	.destroy_fn		= free,
	.sizeof_fn		= ml_provider_ram_sizeof,
	.unpause_fn		= NULL,
	.caption_fn		= ml_provider_ram_caption,
	
	.helptext		= NULL
};


#define PATH_MEMINFO "/proc/meminfo"


typedef struct {
	bool procps_like;
} RAMstate;

mlPointer
ml_provider_ram_init (MlConfig *config)
{
	RAMstate *s = ml_new (RAMstate);
	s->procps_like = true;

	ml_config_add_entry (config,
		"procps_like",
		ML_VALUE_TYPE_BOOLEAN,
		&s->procps_like,
		_("Procps-compliant measurement"),
		_("Specifies whether to follow the measurement method used by standard UNIX procps tools, like <i>free</i> and <i>top</i>. In particular, specifies how to count <i>Slab cache</i>. If <b>true</b>, it will appear as cached memory (this is the behavior of <i>free</i> and <i>top</i>). If <b>false</b>, it will appear as free memory (this is the behavior of eg. <i>htop</i> and some graphical tasak managers).")
		);

	return s;
}

void
ml_provider_ram_get (MlGraphContext *context)
{
	RAMstate *s = (RAMstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	static uint32_t kb_main_total = 0;
	static uint32_t kb_main_free = 0;
	static uint32_t kb_main_buffers = 0;
	static uint32_t kb_main_cached = 0;
	static uint32_t kb_slab = 0;

	static int64_t  kb_main_used = 0;

	static const MlInfofileMappingEntry table[] = {
		{ "MemTotal",		'u',	&kb_main_total },
		{ "MemFree",		'u',	&kb_main_free},
		{ "Buffers",		'u',	&kb_main_buffers },
		{ "Cached",			'u',	&kb_main_cached },
		{ "Slab",			'u',	&kb_slab }
	};

	int r = ml_infofile_read_keys (PATH_MEMINFO, table, 5);
	ml_graph_context_assert_with_message (context, r == 5, _("Cannot retrieve one or more values"));

	if (s->procps_like)
		kb_main_cached += kb_slab;

	kb_main_used = kb_main_total - kb_main_free - kb_main_cached - kb_main_buffers;
	if_unlikely (kb_main_used < 0) // older kernels
		kb_main_used = kb_main_total - kb_main_free;

	// Leave these values as KB (this way maximum size representable with 32 bits is 4 TiB)
	ml_graph_context_set_data (context, 0, (uint32_t)kb_main_used);
	ml_graph_context_set_data (context, 1, kb_main_buffers);
	ml_graph_context_set_data (context, 2, kb_main_cached);
	ml_graph_context_set_max  (context, kb_main_total);
}

size_t
ml_provider_ram_sizeof (mlPointer provider_data)
{
	RAMstate *s = (RAMstate*)provider_data;
	if_unlikely (s == NULL)
		return 0;

	return sizeof (RAMstate);
}

void
ml_provider_ram_caption (MlCaption *caption, MlDataset *ds, ML_UNUSED mlPointer provider_data)
{
	uint32_t kb_used    = ml_dataset_get_value (ds, -1, 0);
	uint32_t kb_buffers = ml_dataset_get_value (ds, -1, 1);
	uint32_t kb_cache   = ml_dataset_get_value (ds, -1, 2);
	uint32_t kb_total   = ml_dataset_get_ceiling (ds, -1);

	char buf_used    [24];
	char buf_buffers [24];
	char buf_cache   [24];
	char buf_total   [24];

	ml_string_format_size_s ((uint64_t)kb_used    * 1024, "B", false, buf_used,    sizeof (buf_used));
	ml_string_format_size_s ((uint64_t)kb_buffers * 1024, "B", false, buf_buffers, sizeof (buf_buffers));
	ml_string_format_size_s ((uint64_t)kb_cache   * 1024, "B", false, buf_cache,   sizeof (buf_cache));
	ml_string_format_size_s ((uint64_t)kb_total   * 1024, "B", false, buf_total,   sizeof (buf_total));


	ml_caption_set (caption, ML_CAPTION_COMPONENT_HEADER, _("Total RAM: %s"), buf_total);


	// TRANSLATORS: used RAM, absolute and percentage value
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("%s (%0.01f%%) used by programs"), buf_used,    100.0 * kb_used    / kb_total);
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");
	// TRANSLATORS: used RAM, absolute and percentage value
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("%s (%0.01f%%) used for buffers"), buf_buffers, 100.0 * kb_buffers / kb_total);
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");
	// TRANSLATORS: used RAM, absolute and percentage value
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("%s (%0.01f%%) used as cache"),    buf_cache,   100.0 * kb_cache   / kb_total);


	// top processes
	if (ML_SHARED_GET (procmon_enable)) {
		MlProcessInfo *pi_array[ML_CAPTION_TABLE_ROWS];
		ml_process_monitor_get_top (ML_SHARED_GET (procmon), ML_PROCESS_INFO_FIELD_RAM, pi_array, ML_CAPTION_TABLE_ROWS);
		char buf_pi [24];

		for (int i = 0; i < ML_CAPTION_TABLE_ROWS; i++) {
			if (pi_array[i] == NULL || pi_array[i]->rss == 0)
				break;

			ml_string_format_size_s (pi_array[i]->rss, "B", false, buf_pi, sizeof (buf_pi));
			ml_caption_set_table_data (caption, i, 0, pi_array[i]->name);
			ml_caption_set_table_data (caption, i, 1, buf_pi);
		}
	}
}
