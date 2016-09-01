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

#include <math.h>
#include <stdlib.h>

#include "graph-data.h"
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


static void get_cpu0_static(char* cpuname, gulong *num_cpu) {
	char *buf = NULL;
	size_t n = 0;
	gulong ncpu = 0;

	FILE *f = fopen("/proc/cpuinfo", "r");
	if (f != NULL) {
		while(TRUE) {
			if (getline(&buf, &n, f) < 0)
				break;
			if (strncmp(buf, "model name", 10) == 0) {
				strcpy(cpuname, strchr(buf, ':')+2);
				cpuname[strlen(cpuname)-1] = '\0'; //remove newline
			}
			else if (strncmp(buf, "processor", 9) == 0)
				ncpu++;
		}
		free(buf);
		fclose(f);
	}

	if (ncpu > 0)
		*num_cpu = ncpu;
}

static void get_cpu0_info(double *mhz, char *governor) {
	static gboolean have_cpuinfo = TRUE;
	static gboolean have_governor = TRUE;

	char *buf = NULL;
	size_t n = 0;

	if (have_cpuinfo) {
		FILE *f = fopen("/proc/cpuinfo", "r");
		if (f != NULL) {
			while(TRUE) {
				if (getline(&buf, &n, f) < 0)
					break;
				if (strncmp(buf, "cpu MHz", 7) == 0) {
					*mhz = g_ascii_strtod(strchr(buf, ':')+2, NULL);
					break;
				}
			}
			free(buf);
			fclose(f);
		} else {
			have_cpuinfo = FALSE;
		}
	}

	buf = NULL;
	if (have_governor) {
		FILE *f = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "r");
		if (f != NULL) {
			if (getline(&buf, &n, f) < 0)
				have_governor = FALSE;
			else
				strcpy(governor, buf);
			free(buf);
			fclose(f);
		} else {
			have_governor = FALSE;
		}
	}

}

void
multiload_graph_cpu_get_data (int Maximum, int data [4], LoadGraph *g, CpuData *xd)
{
	FILE *f;
	guint64 irq, softirq, total;
	gboolean first_call = FALSE;
	guint i;

	guint64 time[CPU_MAX];
	guint64 diff[CPU_MAX];

	if (xd->num_cpu == 0) {
		get_cpu0_static(xd->cpu0_name, &xd->num_cpu);
		first_call = TRUE;
	}

	get_cpu0_info(&xd->cpu0_mhz, xd->cpu0_governor);

	f = fopen("/proc/uptime", "r");
	if (f != NULL) {
		// some countries use commas instead of points for floats
		char *savelocale = strdup(setlocale(LC_NUMERIC, NULL));
		setlocale(LC_NUMERIC, "C");

		fscanf(f, "%lf %*f", &xd->uptime);

		// restore default locale for numbers
		setlocale(LC_NUMERIC, savelocale);
		free(savelocale);

		fclose(f);
	}

	f = fopen("/proc/stat", "r");
	if (f != NULL) {
		fscanf(f, "cpu %ld %ld %ld %ld %ld %ld %ld", time+CPU_USER, time+CPU_NICE, time+CPU_SYS, time+CPU_IDLE, time+CPU_IOWAIT, &irq, &softirq);

		time[CPU_IOWAIT] += irq+softirq;
		fclose(f);
	}

	if (!first_call) {
		for (i=0, total=0; i<CPU_MAX; i++) {
			diff[i] = time[i] - xd->last[i];
			total += diff[i];
		}

		xd->user			= (float)(diff[CPU_USER]) / total;
		xd->iowait			= (float)(diff[CPU_IOWAIT]) / total;
		xd->total_use		= (float)(total-diff[CPU_IDLE]) / total;

		for (i=0; i<CPU_MAX; i++)
			data[i] = rint (Maximum * (float)diff[i] / total);
	}

	memcpy(xd->last, time, sizeof xd->last);
}


void
multiload_graph_cpu_tooltip_update (char **title, char **text, LoadGraph *g, CpuData *xd)
{
	if (g->config->tooltip_style == TOOLTIP_STYLE_DETAILS) {
		gchar *uptime = format_time_duration(xd->uptime);
		*title = g_strdup(xd->cpu0_name);
		*text = g_strdup_printf(_(	"%lu processors  -  %.2f GHz  -  Governor: %s\n"
									"%.1f%% in use by programs\n"
									"%.1f%% in wait for I/O\n"
									"%.1f%% total CPU use\n"
									"\n"
									"Uptime: %s"),
									xd->num_cpu, xd->cpu0_mhz/1000.0, xd->cpu0_governor,
									(xd->user*100),
									(xd->iowait*100),
									(xd->total_use*100),
									uptime);
		g_free(uptime);
	} else {
		*text = g_strdup_printf("%.1f%%", xd->total_use*100);
	}
}
