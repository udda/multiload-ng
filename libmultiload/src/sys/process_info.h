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

#ifndef ML_HEADER__SYS_PROCESS_INFO_H__INCLUDED
#define ML_HEADER__SYS_PROCESS_INFO_H__INCLUDED
ML_HEADER_BEGIN



typedef enum {
	ML_PROCESS_INFO_FIELD_CPU		= 1 << 0,
	ML_PROCESS_INFO_FIELD_RAM		= 1 << 1,
	ML_PROCESS_INFO_FIELD_THREADS	= 1 << 2,
	ML_PROCESS_INFO_FIELD_IO		= 1 << 3
} MlProcessInfoField;


typedef struct {
	int pid;
	char name[17]; // comm field is 16 chars

	unsigned long cpu_time;
	float cpu_time_percent;

	long num_threads;

	long rss;

	uint64_t read_bytes;		// often empty for root processes
	uint64_t write_bytes;		// often empty for root processes
	uint64_t io_bytes_diff;

	char path_stat [19];		// /proc/<pid>/stat (pid max 2^22 -> 7 chars)
	char path_io   [17];		// /proc/<pid>/io (pid max 2^22 -> 7 chars)

	bool updated;				// internal use
} MlProcessInfo;


MlProcessInfo *
ml_process_info_new (int pid);

void
ml_process_info_destroy (MlProcessInfo *pi);

size_t
ml_process_info_sizeof (MlProcessInfo *pi)
ML_FN_SIZEOF;

bool
ml_process_info_update (MlProcessInfo *pi, unsigned long total_cpu_time_diff)
ML_FN_WARN_UNUSED_RESULT;


ML_HEADER_END
#endif /* ML_HEADER__SYS_PROCESS_INFO_H__INCLUDED */
