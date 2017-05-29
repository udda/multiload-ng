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


#define PATH_CPUINFO "/proc/cpuinfo"
#define PATH_STAT "/proc/stat"


MlCpuInfo *
ml_cpu_info_new (int cpu_index)
{
	if_unlikely (cpu_index >= ml_cpu_info_get_cpu_count ())
		return NULL;

	MlCpuInfo *ci = ml_new (MlCpuInfo);

	ci->cpu_index = cpu_index;

	// CPU name (if cpu_index < 0 takes the name of cpu #0)
	char buf_name[64];
	if_likely (ml_infofile_read_key_string_nth_s(PATH_CPUINFO, "model name", MAX(cpu_index,0), buf_name, sizeof(buf_name), NULL))
		ci->name = strdup(buf_name);
	else
		ci->name = _("Unknown CPU");

	// CPU vendor (if cpu_index < 0 takes the vendor of cpu #0)
	char buf_vendor[64];
	if_likely (ml_infofile_read_key_string_nth_s(PATH_CPUINFO, "vendor_id", MAX(cpu_index,0), buf_vendor, sizeof(buf_vendor), NULL))
		ci->vendor = strdup(buf_vendor);
	else
		ci->vendor = _("Unknown vendor");

	// format string
	if (cpu_index < 0)
		ci->stat_key = ml_strdup ("cpu");
	else
		ci->stat_key = ml_strdup_printf ("cpu%d", ci->cpu_index);

	ci->stat_fmt_string = ml_strdup_printf ("%s %%"SCNu64" %%"SCNu64" %%"SCNu64" %%"SCNu64" %%"SCNu64" %%"SCNu64" %%"SCNu64" %%"SCNu64" %%"SCNu64" %%"SCNu64, ci->stat_key);

	// paths
	if (cpu_index >= 0) {
		ci->path_scaling_cur_freq = ml_strdup_printf ("/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", ci->cpu_index);
		ci->path_scaling_min_freq = ml_strdup_printf ("/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", ci->cpu_index);
		ci->path_scaling_max_freq = ml_strdup_printf ("/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", ci->cpu_index);
		ci->path_scaling_governor = ml_strdup_printf ("/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", ci->cpu_index);
	}

	return ci;
}

void
ml_cpu_info_destroy (MlCpuInfo *ci)
{
	if_unlikely (ci == NULL)
		return;

	free (ci->stat_key);
	free (ci->stat_fmt_string);
	free (ci->path_scaling_cur_freq);
	free (ci->path_scaling_min_freq);
	free (ci->path_scaling_max_freq);
	free (ci->path_scaling_governor);

	free (ci);
}

size_t
ml_cpu_info_sizeof (MlCpuInfo *ci)
{
	if_unlikely (ci == NULL)
		return 0;

	size_t size = sizeof (MlCpuInfo);
	size += ml_string_sizeof (ci->stat_key);
	size += ml_string_sizeof (ci->stat_fmt_string);
	size += ml_string_sizeof (ci->path_scaling_cur_freq);
	size += ml_string_sizeof (ci->path_scaling_min_freq);
	size += ml_string_sizeof (ci->path_scaling_max_freq);
	size += ml_string_sizeof (ci->path_scaling_governor);

	return size;
}

bool
ml_cpu_info_update_time (MlCpuInfo *ci)
{
	if_unlikely (ci == NULL || ci->stat_key == NULL || ci->stat_fmt_string == NULL)
		return false;

	uint64_t irq, softirq;
	uint64_t steal = 0, guest = 0, guest_nice = 0;

	char *row = ml_cpu_info_read_stat_row (ci->stat_key);
	if (row == NULL)
		return false;

	size_t n = sscanf (row, ci->stat_fmt_string,
		&ci->time_user,
		&ci->time_nice,
		&ci->time_sys,
		&ci->time_idle,
		&ci->time_iowait,
		&irq,
		&softirq,
		&steal,
		&guest,
		&guest_nice
	);

	free (row);

	/* we don't check whether n == 10, because steal, guest and guest_nice
	 * are newer fields and they are not strictly required */
	if (n < 7)
		return false;

	ci->time_sys += irq+softirq;

	/* According to the /proc man page, guest and guest_nice fields are CPU
	 * user/nice jiffies spent running a virtual CPU for guest operating systems
	 * under the control of the Linux kernel.
	 * Technically, these times do not belong to host, so we don't count them */
	ci->time_user -= guest;
	ci->time_nice -= guest_nice;

	/* According to the /proc man page, steal field is CPU time "stolen" by other
	 * operating systems when running under a VM.
	 * To the host this is unused time, so we count it under idle time */
	ci->time_idle += steal;

	return true;
}

bool
ml_cpu_info_update_scaling (MlCpuInfo *ci)
{
	if_unlikely (ci == NULL)
		return false;

	if (!ml_infofile_read_uint32 (ci->path_scaling_cur_freq, &ci->scaling_cur_freq_khz))
		return false;

	if (!ml_infofile_read_uint32 (ci->path_scaling_min_freq, &ci->scaling_min_freq_khz))
		return false;

	if (!ml_infofile_read_uint32 (ci->path_scaling_max_freq, &ci->scaling_max_freq_khz))
		return false;

	if (!ml_infofile_read_string_s (ci->path_scaling_governor, ci->scaling_governor, sizeof(ci->scaling_governor), NULL))
		return false;

	return true;
}


int
ml_cpu_info_get_cpu_count ()
{
	static int count = 0;
	if_unlikely (count < 1) {
		count = sysconf (_SC_NPROCESSORS_CONF);

		// fallback for systems which do not support querying through sysconf (very rare)
		if_unlikely (count < 1)
			count = (int)ml_infofile_count_key_values (PATH_CPUINFO, "processor");
	}

	return count;
}

char*
ml_cpu_info_read_stat_row (const char *key)
{
	FILE *f = fopen (PATH_STAT, "r");
	if_unlikely (f == NULL)
		return NULL;

	char *line = NULL;
	size_t n = 0;

	size_t keylen = strlen (key);
	ssize_t linelen;
	bool found = false;

	while ((linelen = getline (&line, &n, f)) >= 0) {
		if ((size_t)linelen <= keylen)
			continue;

		if (!strncmp (key, line, keylen)) {
			found = true;
			break;
		}
	}
	fclose (f);

	if (found) {
		return line;
	} else {
		free (line);
		return NULL;
	}
}

MlGrowBuffer *
ml_cpu_info_generate_report ()
{
	MlGrowBuffer *gbuf = ml_grow_buffer_new (1024);

	int cpu_count = ml_cpu_info_get_cpu_count();

	ml_grow_buffer_append_printf (gbuf, "System reports %d CPUs.\n", cpu_count);

	for (int i = 0; i < cpu_count; i++) {
		MlCpuInfo *ci = ml_cpu_info_new (i);
		
		ml_grow_buffer_append_printf (gbuf, "\n\nCPU #%d\n------\n", i);
		if (ci == NULL) {
			ml_grow_buffer_append_printf (gbuf, "Error: cannot create MlCpuInfo. This is a bug.\n");
			continue;
		}
		ml_grow_buffer_append_printf (gbuf, "Name:    %s", ci->name);
		ml_grow_buffer_append_printf (gbuf, "Vendor:  %s", ci->vendor);

		ml_grow_buffer_append_printf (gbuf, "CPU time since boot:");
		if (ml_cpu_info_update_time (ci)) {
			ml_grow_buffer_append_printf (gbuf, "\n");
			ml_grow_buffer_append_printf (gbuf, "  user:     %"PRIu64" jiffies\n", ci->time_user);
			ml_grow_buffer_append_printf (gbuf, "  nice:     %"PRIu64" jiffies\n", ci->time_nice);
			ml_grow_buffer_append_printf (gbuf, "  system:   %"PRIu64" jiffies\n", ci->time_sys);
			ml_grow_buffer_append_printf (gbuf, "  iowait:   %"PRIu64" jiffies\n", ci->time_iowait);
			ml_grow_buffer_append_printf (gbuf, "  idle:     %"PRIu64" jiffies\n", ci->time_idle);
		} else {
			ml_grow_buffer_append_printf (gbuf, " NOT AVAILABLE");
		}

		ml_grow_buffer_append_printf (gbuf, "CPU scaling info:");
		if (ml_cpu_info_update_scaling (ci)) {
			ml_grow_buffer_append_printf (gbuf, "\n");
			ml_grow_buffer_append_printf (gbuf, "  Current frequency:  %"PRIu32" kHz\n", ci->scaling_cur_freq_khz);
			ml_grow_buffer_append_printf (gbuf, "  Maximum frequency:  %"PRIu32" kHz\n", ci->scaling_max_freq_khz);
			ml_grow_buffer_append_printf (gbuf, "  Minimum frequency:  %"PRIu32" kHz\n", ci->scaling_min_freq_khz);
			ml_grow_buffer_append_printf (gbuf, "  Governor:           %s\n", ci->scaling_governor);
		} else {
			ml_grow_buffer_append_printf (gbuf, " NOT AVAILABLE");
		}

		ml_cpu_info_destroy (ci);
	}

	return gbuf;
}
