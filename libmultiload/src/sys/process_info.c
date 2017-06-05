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


MlProcessInfo *
ml_process_info_new (int pid)
{
	MlProcessInfo *pi = ml_new (MlProcessInfo);

	pi->pid = pid;
	snprintf (pi->path_stat, sizeof (pi->path_stat), "/proc/%d/stat", pid);
	snprintf (pi->path_io,   sizeof (pi->path_io),   "/proc/%d/io",   pid);

	if (!ml_file_is_readable (pi->path_stat)) {
		ml_process_info_destroy (pi);
		return NULL;
	}

	return pi;
}

void
ml_process_info_destroy (MlProcessInfo *pi)
{
	if_unlikely (pi == NULL)
		return;

	free (pi);
}

size_t
ml_process_info_sizeof (MlProcessInfo *pi)
{
	if_unlikely (pi == NULL)
		return 0;

	return sizeof (MlProcessInfo);
}

bool
ml_process_info_update (MlProcessInfo *pi, unsigned long total_cpu_time_diff)
{
	if_unlikely (pi == NULL)
		return false;

	FILE *f = fopen (pi->path_stat, "r");
	if_unlikely (f == NULL)
		return false;

	unsigned long utime, stime;

	int n = fscanf(f, "%*d (%16[^)]) %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %ld %*d %*u %*u %ld", pi->name, &utime, &stime, &pi->num_threads, &pi->rss);
	fclose(f);
	if_unlikely (n != 5)
		return false;


	// RAM
	pi->rss *= ml_sys_info_get_page_size ();


	// CPU
	unsigned long new_cpu_time = utime + stime;

	if (new_cpu_time < pi->cpu_time || pi->cpu_time == 0) // recycled PID (old data) or no diff (new data)
		pi->cpu_time_percent = 0;
	else
		pi->cpu_time_percent = 100.0 * (new_cpu_time - pi->cpu_time) / total_cpu_time_diff;

	pi->cpu_time = new_cpu_time;


	// I/O
	static uint64_t new_read_bytes, new_write_bytes;
	static const MlInfofileMappingEntry table[] = {
		{ "read_bytes",		'U',	&new_read_bytes  },
		{ "write_bytes",	'U',	&new_write_bytes }
	};
	if (ml_infofile_read_keys (pi->path_io, table, 2) != 2) { // this may fail for root-owned processes, so we don't consider this an error
		pi->io_bytes_diff = 0;
	} else {
		uint64_t read_diff = 0, write_diff = 0;

		// avoid counting I/O of recycled PIDs. Downside: first read/write is not counted
		if (new_read_bytes >= pi->read_bytes && pi->read_bytes > 0)
			read_diff = new_read_bytes - pi->read_bytes;
		if (new_write_bytes >= pi->write_bytes && pi->write_bytes > 0)
			write_diff = new_write_bytes - pi->write_bytes;

		pi->io_bytes_diff = read_diff + write_diff;

		pi->read_bytes = new_read_bytes;
		pi->write_bytes = new_write_bytes;
	}

	return true;
}
