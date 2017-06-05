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


struct _MlProcessMonitor {
	MlGrowArray *list;

	pthread_t thread;
	pthread_mutex_t mutex;

	DIR *proc_dirp;

	uint64_t total_cpu_time;
	uint64_t total_cpu_time_diff;

	int active_counter;
	MlProcessInfoField last_sort;
};


static ML_FN_NORETURN void *
_procmon_routine (mlPointer user_data)
{
	MlProcessMonitor *pm = (MlProcessMonitor*)user_data;

	struct timespec tm;
	ml_milliseconds_to_timespec (&tm, ML_PROCESS_MONITOR_INTERVAL);

	ml_thread_set_priority (ML_PROCESS_MONITOR_PRIORITY);

	while (true) {
		uint64_t c[4] = {0, 0, 0, 0};
		uint64_t sum = 0;

		char *row = ml_cpu_info_read_stat_row ("cpu");
		if (row != NULL) {
			sscanf (row, "cpu %"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64, &c[0], &c[1], &c[2], &c[3]);
			free (row);

			for (int i = 0; i < 4; i++)
				sum += c[i];
		}

		pthread_mutex_lock (&pm->mutex);

		if (sum < pm->total_cpu_time)
			pm->total_cpu_time_diff = 0;
		else
			pm->total_cpu_time_diff = sum - pm->total_cpu_time;
		pm->total_cpu_time = sum;

		ml_process_monitor_update (pm);
		pm->last_sort = ML_INVALID; // force resort


		pthread_mutex_unlock (&pm->mutex);
		nanosleep (&tm, NULL);
	}
}


MlProcessMonitor *
ml_process_monitor_new ()
{
	MlProcessMonitor *pm = ml_new (MlProcessMonitor);

	pm->total_cpu_time = 0;
	pm->total_cpu_time_diff = 0;
	pm->active_counter = 0;
	pm->last_sort = ML_INVALID;

	pm->proc_dirp = opendir("/proc");
	if_unlikely (pm->proc_dirp == NULL) {
		free (pm);
		return NULL;
	}

	pthread_mutex_init(&pm->mutex, NULL);
	if (pthread_create (&pm->thread, NULL, _procmon_routine, pm) != 0) {
		closedir (pm->proc_dirp);
		free (pm);
		return NULL;
	}

	pm->list = ml_grow_array_new (200, (MlDestroyFunc)ml_process_info_destroy);

	return pm;
}

void
ml_process_monitor_destroy (MlProcessMonitor *pm)
{
	if_unlikely (pm == NULL)
		return;

	pthread_mutex_destroy (&pm->mutex);

	// cancel thread and wait for its termination
	pthread_cancel (pm->thread);
	pthread_join (pm->thread, NULL);

	closedir (pm->proc_dirp);
	ml_grow_array_destroy (pm->list, true, false);

	free (pm);
}

void
ml_process_monitor_update (MlProcessMonitor *pm)
{
	if_unlikely (pm == NULL)
		return;

	if (pm->active_counter <= 0)
		return;
	pm->active_counter--;

	struct dirent *dirent;
	rewinddir (pm->proc_dirp);

	// loop through all used PIDs
	while ((dirent = readdir (pm->proc_dirp)) != NULL) {
		if (!ml_isdigit (dirent->d_name[0]))
			continue;

		int pid = (int)ml_ascii_strtoull(dirent->d_name, NULL, 10);
		if (pid == 0)
			continue;

		// find PID in existing list
		MlProcessInfo *pi = NULL;
		ml_grow_array_for (pm->list, i) {
			MlProcessInfo *pi_tmp = (MlProcessInfo*)ml_grow_array_get (pm->list, i);
			if (pi_tmp == NULL)
				continue;

			if (pi_tmp->pid == pid)
				pi = pi_tmp;
		}

		// PID not found, create a MlProcessInfo for it
		if (pi == NULL) {
			pi = ml_process_info_new(pid);
			if (pi == NULL)
				continue;

			ml_grow_array_append (pm->list, pi);
		}


		if (!ml_process_info_update (pi, pm->total_cpu_time_diff))
			continue;

		pi->updated = true;
	}

	// clear old values
	ml_grow_array_for (pm->list, i) {
		MlProcessInfo *pi = (MlProcessInfo*)ml_grow_array_get (pm->list, i);
		if (pi->updated)
			pi->updated = false;
		else
			ml_grow_array_remove (pm->list, i);
	}
}

static inline int
_ml_process_info_compare_cpu_reverse (const MlProcessInfo **a, const MlProcessInfo **b)
{
	if ((*a)->cpu_time_percent > (*b)->cpu_time_percent)
		return -1;
	else if ((*a)->cpu_time_percent < (*b)->cpu_time_percent)
		return 1;
	else
		return 0;
}

static inline int
_ml_process_info_compare_ram_reverse (const MlProcessInfo **a, const MlProcessInfo **b)
{
	if ((*a)->rss > (*b)->rss)
		return -1;
	else if ((*a)->rss < (*b)->rss)
		return 1;
	else
		return 0;
}

static inline int
_ml_process_info_compare_threads_reverse (const MlProcessInfo **a, const MlProcessInfo **b)
{
	if ((*a)->num_threads > (*b)->num_threads)
		return -1;
	else if ((*a)->num_threads < (*b)->num_threads)
		return 1;
	else
		return 0;
}

static inline int
_ml_process_info_compare_io_reverse (const MlProcessInfo **a, const MlProcessInfo **b)
{
	if ((*a)->io_bytes_diff > (*b)->io_bytes_diff)
		return -1;
	else if ((*a)->io_bytes_diff < (*b)->io_bytes_diff)
		return 1;
	else
		return 0;
}

void
ml_process_monitor_get_top (MlProcessMonitor *pm, MlProcessInfoField sort_field, MlProcessInfo **array, size_t len)
{
	if_unlikely (pm == NULL)
		return;

	// clear array
	memset (array, 0, sizeof (MlProcessInfo*) * len);

	pthread_mutex_lock (&pm->mutex);

	unsigned old_counter = pm->active_counter;

	/* In order to be able to calculate diffs, keep the
	 * monitor active for at least three cycles */
	pm->active_counter = 3;

	// Don't print first update
	if (old_counter == 0) {
		pthread_mutex_unlock (&pm->mutex);
		return;
	}

	if (pm->list == NULL || ml_grow_array_get_length (pm->list) == 0) {
		pthread_mutex_unlock (&pm->mutex);
		return;
	}

	// sort MlGrowArray only if its content or sorting type changed
	if (pm->last_sort != sort_field) {
		switch (sort_field) {
			case ML_PROCESS_INFO_FIELD_CPU:
				ml_grow_array_sort (pm->list, (MlCompareFunc)_ml_process_info_compare_cpu_reverse);
				break;
			case ML_PROCESS_INFO_FIELD_RAM:
				ml_grow_array_sort (pm->list, (MlCompareFunc)_ml_process_info_compare_ram_reverse);
				break;
			case ML_PROCESS_INFO_FIELD_THREADS:
				ml_grow_array_sort (pm->list, (MlCompareFunc)_ml_process_info_compare_threads_reverse);
				break;
			case ML_PROCESS_INFO_FIELD_IO:
				ml_grow_array_sort (pm->list, (MlCompareFunc)_ml_process_info_compare_io_reverse);
				break;
			default:
				break;
		}

		pm->last_sort = sort_field;
	}

	// fill output array
	for (size_t i = 0; i < len && i < ml_grow_array_get_length (pm->list); i++)
		array[i] = ml_grow_array_get (pm->list, i);

	pthread_mutex_unlock (&pm->mutex);
}
