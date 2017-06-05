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


const MlGraphTypeInterface ML_PROVIDER_CPUFREQ_IFACE = {
	.name			= "cpufreq",
	.label			= N_("CPU frequency"),
	.description	= N_("Shows CPU frequency and governor."),

	.hue			= 164,

	.n_data			= 1,
	.dataset_mode	= ML_DATASET_MODE_PROPORTIONAL,

	.init_fn		= ml_provider_cpufreq_init,
	.config_fn		= ml_provider_cpufreq_config,
	.get_fn			= ml_provider_cpufreq_get,
	.destroy_fn		= ml_provider_cpufreq_destroy,
	.sizeof_fn		= ml_provider_cpufreq_sizeof,
	.unpause_fn		= ml_provider_cpufreq_unpause,
	.caption_fn		= ml_provider_cpufreq_caption,

	.helptext		= NULL
};


typedef struct {
	int32_t cpu_index;

	MlCpuInfo *info;
} CPUFREQstate;


mlPointer
ml_provider_cpufreq_init (MlConfig *config)
{
	CPUFREQstate *s = ml_new (CPUFREQstate);
	s->cpu_index = 0;
	s->info = ml_cpu_info_new (s->cpu_index);

	ml_config_add_entry_with_bounds (config,
		"cpu_index",
		ML_VALUE_TYPE_INT32,
		&s->cpu_index,
		_("CPU core"),
		_("Core to use for graph values."),
		0, ml_cpu_info_get_cpu_count()-1
	);

	return s;
}

void
ml_provider_cpufreq_config (MlGraphContext *context)
{
	CPUFREQstate *s = (CPUFREQstate*)ml_graph_context_get_provider_data (context);
	if_unlikely (s == NULL)
		return;

	ml_cpu_info_destroy (s->info);
	s->info = ml_cpu_info_new (s->cpu_index);

	ml_graph_context_set_need_data_reset (context);
}

void
ml_provider_cpufreq_get (MlGraphContext *context)
{
	CPUFREQstate *s = (CPUFREQstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	ml_graph_context_assert_with_message (context, ml_cpu_info_update_scaling (s->info), _("Unable to get current CPU scaling information"));

	ml_graph_context_set_data (context, 0, s->info->scaling_cur_freq_khz);
	ml_graph_context_set_max  (context,    s->info->scaling_max_freq_khz);
}

void
ml_provider_cpufreq_destroy (mlPointer provider_data)
{
	CPUFREQstate *s = (CPUFREQstate*)provider_data;

	if_unlikely (s == NULL)
		return;

	ml_cpu_info_destroy (s->info);
	free (s);
}

size_t
ml_provider_cpufreq_sizeof (mlPointer provider_data)
{
	CPUFREQstate *s = (CPUFREQstate*)provider_data;
	if_unlikely (s == NULL)
		return 0;

	size_t size = sizeof (CPUFREQstate);
	size += ml_cpu_info_sizeof (s->info);

	return size;
}

void
ml_provider_cpufreq_unpause (MlGraphContext *context)
{
	ml_graph_context_set_first_call (context); // avoid spikes after unpause
}

void
ml_provider_cpufreq_caption (MlCaption *caption, ML_UNUSED MlDataset *ds, mlPointer provider_data)
{
	CPUFREQstate *s = (CPUFREQstate*)provider_data;
	if_unlikely (s == NULL || s->info == NULL)
		return;

	char buf_cur_freq[24];
	char buf_min_freq[24];
	char buf_max_freq[24];

	ml_string_format_size_s (s->info->scaling_cur_freq_khz * 1000, "Hz", true, buf_cur_freq, sizeof (buf_cur_freq));
	ml_string_format_size_s (s->info->scaling_min_freq_khz * 1000, "Hz", true, buf_min_freq, sizeof (buf_min_freq));
	ml_string_format_size_s (s->info->scaling_max_freq_khz * 1000, "Hz", true, buf_max_freq, sizeof (buf_max_freq));

	// TRANSLATORS: CPU governor (many languages leave it untranslated)
	ml_caption_set (caption, ML_CAPTION_COMPONENT_HEADER, _("Governor: %s"), s->info->scaling_governor);

	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Current frequency: %s"), buf_cur_freq);
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Minimum frequency: %s"), buf_min_freq);
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");
	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Maximum frequency: %s"), buf_max_freq);

	ml_caption_set (caption, ML_CAPTION_COMPONENT_FOOTER, _("Selected core: #%d"), s->info->cpu_index);
}
