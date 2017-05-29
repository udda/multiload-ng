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

#ifndef ML_HEADER__SYS_CPU_INFO_H__INCLUDED
#define ML_HEADER__SYS_CPU_INFO_H__INCLUDED
ML_HEADER_BEGIN


typedef struct {
	int cpu_index;

	char *name;
	char *vendor;

	char *stat_key;
	char *stat_fmt_string;
	uint64_t time_user;
	uint64_t time_nice;
	uint64_t time_sys;
	uint64_t time_iowait;
	uint64_t time_idle;

	char *path_scaling_cur_freq;
	char *path_scaling_min_freq;
	char *path_scaling_max_freq;
	char *path_scaling_governor;

	uint32_t scaling_cur_freq_khz;
	uint32_t scaling_min_freq_khz;
	uint32_t scaling_max_freq_khz;
	char scaling_governor[32];
} MlCpuInfo;


MlCpuInfo *
ml_cpu_info_new (int cpu_index);

void
ml_cpu_info_destroy (MlCpuInfo *ci);

size_t
ml_cpu_info_sizeof (MlCpuInfo *ci)
ML_FN_SIZEOF;

bool
ml_cpu_info_update_time (MlCpuInfo *ci)
ML_FN_WARN_UNUSED_RESULT;

bool
ml_cpu_info_update_scaling (MlCpuInfo *ci)
ML_FN_WARN_UNUSED_RESULT;


int
ml_cpu_info_get_cpu_count ()
ML_FN_PURE;

char *
ml_cpu_info_read_stat_row (const char *key)
ML_FN_MALLOC;

MlGrowBuffer *
ml_cpu_info_generate_report ()
ML_FN_COLD ML_FN_RETURNS_NONNULL;


ML_HEADER_END
#endif /* ML_HEADER__SYS_CPU_INFO_H__INCLUDED */
