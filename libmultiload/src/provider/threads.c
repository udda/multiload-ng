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


const MlGraphTypeInterface ML_PROVIDER_THREADS_IFACE = {
	.name			= "threads",
	.label			= N_("Processes and threads"),
	.description	= N_("Shows number of active processes/threads."),

	.hue			= 113,

	.n_data			= 1,
	.dataset_mode	= ML_DATASET_MODE_ABSOLUTE,

	.init_fn		= ml_provider_threads_init,
	.config_fn		= NULL,
	.get_fn			= ml_provider_threads_get,
	.destroy_fn		= free,
	.sizeof_fn		= NULL,
	.unpause_fn		= NULL,
	.caption_fn		= ml_provider_threads_caption,

	.helptext		= NULL
};


typedef struct {
	unsigned long active;
	unsigned long total;
} THREADSstate;


#define PATH_LOADAVG "/proc/loadavg"

mlPointer
ml_provider_threads_init (ML_UNUSED MlConfig *config)
{
	THREADSstate *s = ml_new (THREADSstate);
	return s;
}

void
ml_provider_threads_get (MlGraphContext *context)
{
	THREADSstate *s = (THREADSstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	FILE *f = fopen (PATH_LOADAVG, "r");
	ml_graph_context_assert_with_message (context, f != NULL, _("Processes/threads stats do not exist in this system"));

	int n = fscanf (f, "%*s %*s %*s %lu/%lu", &s->active, &s->total);
	fclose(f);

	ml_graph_context_assert (context, n == 2);

	/* We don't set graph max to the total number of processes (next number in
	 * /proc/loadavg) because it's usually a very large number compared to
	 * active ones (eg. 3 out of 500), so it would draw always very small values */
	ml_graph_context_set_data (context, 0, s->active);
}

void
ml_provider_threads_caption (MlCaption *caption, ML_UNUSED MlDataset *ds, mlPointer provider_data)
{
	THREADSstate *s = (THREADSstate*)provider_data;
	if_unlikely (s == NULL)
		return;

	ml_caption_set (caption, ML_CAPTION_COMPONENT_BODY, _("Currently there are %lu active processes/threads"), s->active);
	ml_caption_set (caption, ML_CAPTION_COMPONENT_FOOTER, _("%lu total processes/threads in this system"), s->total);

	// top processes
	if (ML_SHARED_GET (procmon_enable)) {
		MlProcessInfo *pi_array[ML_CAPTION_TABLE_ROWS];
		ml_process_monitor_get_top (ML_SHARED_GET (procmon), ML_PROCESS_INFO_FIELD_THREADS, pi_array, ML_CAPTION_TABLE_ROWS);

		for (int i = 0; i < ML_CAPTION_TABLE_ROWS; i++) {
			if (pi_array[i] == NULL || pi_array[i]->num_threads == 0)
				break;

			ml_caption_set_table_data (caption, i, 0, pi_array[i]->name);
			// TRANSLATORS: number of threads of a process
			ml_caption_set_table_data (caption, i, 1, _("%lu threads"), pi_array[i]->num_threads);
		}
	}
}
