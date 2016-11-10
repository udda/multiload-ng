/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of multiload-ng.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include <config.h>

#include <errno.h>
#include <math.h>
#include <stdlib.h>

#include "graph-data.h"
#include "info-file.h"
#include "preferences.h"
#include "util.h"


enum {
	CPU_USER	= 0,
	CPU_NICE	= 1,
	CPU_SYS		= 2,
	CPU_IOWAIT	= 3,
	CPU_IDLE	= 4,

	CPU_MAX		= 5
};

#define PATH_CPUFREQ "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
#define PATH_UPTIME "/proc/uptime"
#define PATH_CPUINFO "/proc/cpuinfo"
#define PATH_STAT "/proc/stat"

void
multiload_graph_cpu_get_data (int Maximum, int data [4], LoadGraph *g, CpuData *xd)
{
	static gboolean first_call = TRUE;
	static gboolean have_cpufreq = TRUE;

	guint64 irq, softirq, total;
	guint i;

	guint64 time[CPU_MAX];
	guint64 diff[CPU_MAX];

	size_t n;

	if (first_call) {
		info_file_read_key_string_s (PATH_CPUINFO, "model name", xd->cpu0_name, sizeof(xd->cpu0_name), NULL);
		xd->num_cpu = info_file_count_key_values (PATH_CPUINFO, "processor");

		have_cpufreq = info_file_exists(PATH_CPUFREQ);
		if (!have_cpufreq) {
			//xgettext: Not Available
			strcpy(xd->cpu0_governor, _("N/A"));
			g_debug("[graph-cpu] cpufreq scaling support not found");
		}
	}

	info_file_read_key_double (PATH_CPUINFO, "cpu MHz", &xd->cpu0_mhz, 1);
	info_file_read_double (PATH_UPTIME, &xd->uptime, 1);

	if (have_cpufreq && !info_file_read_string_s (PATH_CPUFREQ, xd->cpu0_governor, sizeof(xd->cpu0_governor), NULL)) {
		g_warning("[graph-cpu] Could not retrieve CPU0 governor");
		have_cpufreq = FALSE;
	}

	// CPU stats
	FILE *f = info_file_required_fopen(PATH_STAT, "r");
	n = fscanf(f, "cpu %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT,
				time+CPU_USER, time+CPU_NICE, time+CPU_SYS, time+CPU_IDLE, time+CPU_IOWAIT, &irq, &softirq);
	fclose(f);
	g_assert_cmpuint(n, ==, 7);
	time[CPU_IOWAIT] += irq+softirq;

	if (G_UNLIKELY(first_call)) {
		first_call = FALSE;
	} else {
		for (i=0, total=0; i<CPU_MAX; i++) {
			diff[i] = time[i] - xd->last[i];
			total += diff[i];
		}

		xd->user			= 100.0 * (float)(diff[CPU_USER]) / total;
		xd->nice			= 100.0 * (float)(diff[CPU_NICE]) / total;
		xd->system			= 100.0 * (float)(diff[CPU_SYS]) / total;
		xd->iowait			= 100.0 * (float)(diff[CPU_IOWAIT]) / total;
		xd->total_use		= 100.0 * (float)(total-diff[CPU_IDLE]) / total;

		for (i=0; i<4; i++)
			data[i] = rint (Maximum * (float)diff[i] / total);
	}

	memcpy(xd->last, time, sizeof xd->last);
}


void
multiload_graph_cpu_cmdline_output (LoadGraph *g, CpuData *xd)
{
	g_snprintf(g->output_str[0], sizeof(g->output_str[0]), "%.03f", xd->user);
	g_snprintf(g->output_str[1], sizeof(g->output_str[1]), "%.03f", xd->nice);
	g_snprintf(g->output_str[2], sizeof(g->output_str[2]), "%.03f", xd->system);
	g_snprintf(g->output_str[3], sizeof(g->output_str[3]), "%.03f", xd->iowait);
}


void
multiload_graph_cpu_tooltip_update (char **title, char **text, LoadGraph *g, CpuData *xd)
{
	if (g->config->tooltip_style == MULTILOAD_TOOLTIP_STYLE_DETAILED) {
		gchar *uptime = format_time_duration(xd->uptime);
		*title = g_strdup(xd->cpu0_name);
		*text = g_strdup_printf(_(	"%lu processors  -  %.2f GHz  -  Governor: %s\n"
									"%.1f%% in use by programs\n"
									"%.1f%% in use by low priority programs\n"
									"%.1f%% in use by the kernel\n"
									"%.1f%% in wait for I/O\n"
									"%.1f%% total CPU use\n"
									"\n"
									"Uptime: %s"),
									xd->num_cpu, xd->cpu0_mhz/1000.0, xd->cpu0_governor,
									xd->user, xd->nice, xd->system, xd->iowait, xd->total_use,
									uptime);
		g_free(uptime);
	} else {
		*text = g_strdup_printf("%.1f%%", xd->total_use);
	}
}
