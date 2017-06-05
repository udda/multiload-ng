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


const MlGraphTypeInterface ML_PROVIDER_CPU_IFACE = {
	.name			= "cpu",
	.label			= N_("CPU usage"),
	.description	= N_("Shows CPU usage, split by type (user processes, low priority processes, kernel processes, I/O wait)."),

	.hue			= 196,

	.n_data			= 4,
	.dataset_mode	= ML_DATASET_MODE_PROPORTIONAL,

	.init_fn		= ml_provider_cpu_init,
	.config_fn		= ml_provider_cpu_config,
	.get_fn			= ml_provider_cpu_get,
	.destroy_fn		= ml_provider_cpu_destroy,
	.sizeof_fn		= ml_provider_cpu_sizeof,
	.unpause_fn		= ml_provider_cpu_unpause,
	.caption_fn		= ml_provider_cpu_caption,

	.helptext		= NULL
};


#define PATH_STAT "/proc/stat"


typedef struct {
	int32_t cpu_index;

	MlCpuInfo *info;
	uint64_t last_user;
	uint64_t last_nice;
	uint64_t last_sys;
	uint64_t last_iowait;
	uint64_t last_idle;
} CPUstate;


mlPointer
ml_provider_cpu_init (MlConfig *config)
{
	CPUstate *s = ml_new (CPUstate);
	s->cpu_index = -1;
	s->info = ml_cpu_info_new (s->cpu_index);

	ml_config_add_entry_with_bounds (config,
		"cpu_index",
		ML_VALUE_TYPE_INT32,
		&s->cpu_index,
		_("CPU core"),
		_("Core to use for graph values. Enter <b>-1</b> to use all cores together."),
		-1, ml_cpu_info_get_cpu_count() - 1
	);

	return s;
}

void
ml_provider_cpu_config (MlGraphContext *context)
{
	CPUstate *s = (CPUstate*)ml_graph_context_get_provider_data (context);
	if_unlikely (s == NULL)
		return;

	ml_cpu_info_destroy (s->info);
	s->info = ml_cpu_info_new (s->cpu_index);

	ml_graph_context_set_need_data_reset (context);
}

void
ml_provider_cpu_get (MlGraphContext *context)
{
	CPUstate *s = (CPUstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	ml_graph_context_assert_with_message (context, ml_cpu_info_update_time (s->info), _("Unable to get current CPU time information"));

	if_unlikely (!ml_graph_context_is_first_call (context)) { // cannot calculate diff on first call
		uint32_t diff_user		= (uint32_t)(s->info->time_user		- s->last_user);
		uint32_t diff_nice		= (uint32_t)(s->info->time_nice		- s->last_nice);
		uint32_t diff_sys		= (uint32_t)(s->info->time_sys		- s->last_sys);
		uint32_t diff_iowait	= (uint32_t)(s->info->time_iowait	- s->last_iowait);
		uint32_t diff_idle		= (uint32_t)(s->info->time_idle		- s->last_idle);

		uint32_t total = diff_user + diff_nice + diff_sys + diff_iowait + diff_idle;

		ml_graph_context_set_data (context, 0, diff_user);
		ml_graph_context_set_data (context, 1, diff_nice);
		ml_graph_context_set_data (context, 2, diff_sys);
		ml_graph_context_set_data (context, 3, diff_iowait);
		ml_graph_context_set_max  (context, total);
	}

	// copy new values to old values
	s->last_user	= s->info->time_user;
	s->last_nice	= s->info->time_nice;
	s->last_sys		= s->info->time_sys;
	s->last_iowait	= s->info->time_iowait;
	s->last_idle	= s->info->time_idle;
}

void
ml_provider_cpu_destroy (mlPointer provider_data)
{
	CPUstate *s = (CPUstate*)provider_data;
	if_unlikely (s == NULL)
		return;

	ml_cpu_info_destroy (s->info);
	free (s);
}

size_t
ml_provider_cpu_sizeof (mlPointer provider_data)
{
	CPUstate *s = (CPUstate*)provider_data;
	if_unlikely (s == NULL)
		return 0;

	size_t size = sizeof (CPUstate);
	size += ml_cpu_info_sizeof (s->info);

	return size;
}

void
ml_provider_cpu_unpause (MlGraphContext *context)
{
	ml_graph_context_set_first_call (context); // avoid spikes after unpause
}

void
ml_provider_cpu_caption (MlCaption *caption, MlDataset *ds, mlPointer provider_data)
{
	CPUstate *s = (CPUstate*)provider_data;
	if_unlikely (s == NULL || s->info == NULL)
		return;

	uint32_t diff_user   = ml_dataset_get_value (ds, -1, 0);
	uint32_t diff_nice   = ml_dataset_get_value (ds, -1, 1);
	uint32_t diff_sys    = ml_dataset_get_value (ds, -1, 2);
	uint32_t diff_iowait = ml_dataset_get_value (ds, -1, 3);

	uint32_t diff_total  = (diff_user + diff_nice + diff_sys + diff_iowait);

	uint32_t total = ml_dataset_get_ceiling (ds, -1);
	double total_percent = 100.0f / total;


	ml_caption_append (caption, ML_CAPTION_COMPONENT_HEADER, "%s\n", s->info->name);
	// TRANSLATORS: CPU vendor ID
	ml_caption_append (caption, ML_CAPTION_COMPONENT_HEADER, _("Vendor: %s"), s->info->vendor);


	// TRANSLATORS: CPU usage percentage
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("%0.01f%% used by programs"),               diff_user   * total_percent);
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");
	// TRANSLATORS: CPU usage percentage
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("%0.01f%% used by low priority programs"),  diff_nice   * total_percent);
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");
	// TRANSLATORS: CPU usage percentage
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("%0.01f%% used by the kernel"),             diff_sys    * total_percent);
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");
	// TRANSLATORS: CPU usage percentage
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("%0.01f%% waiting for I/O"),                diff_iowait * total_percent);
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");
	// TRANSLATORS: CPU usage percentage
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("%0.01f%% total CPU usage"),                diff_total  * total_percent);

	// TRANSLATORS: number of CPUs in this system
	ml_caption_set (caption, ML_CAPTION_COMPONENT_FOOTER, _("%d total cores"), ml_cpu_info_get_cpu_count());
	if (s->info->cpu_index >= 0) {
		ml_caption_append (caption, ML_CAPTION_COMPONENT_FOOTER, "\n");
		ml_caption_append (caption, ML_CAPTION_COMPONENT_FOOTER, _("Selected core: #%d"), s->info->cpu_index);
	}


	// top processes
	if (ML_SHARED_GET (procmon_enable)) {
		MlProcessInfo *pi_array[ML_CAPTION_TABLE_ROWS];
		ml_process_monitor_get_top (ML_SHARED_GET (procmon), ML_PROCESS_INFO_FIELD_CPU, pi_array, ML_CAPTION_TABLE_ROWS);

		for (int i = 0; i < ML_CAPTION_TABLE_ROWS; i++) {
			if (pi_array[i] == NULL || pi_array[i]->cpu_time_percent == 0)
				break;

			ml_caption_set_table_data (caption, i, 0, pi_array[i]->name);
			ml_caption_set_table_data (caption, i, 1, "%0.02f%%", pi_array[i]->cpu_time_percent);
		}
	}
}
