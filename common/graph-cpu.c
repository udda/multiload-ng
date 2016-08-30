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
#include <glibtop.h>
#include <glibtop/cpu.h>
#include <glibtop/uptime.h>

#include "graph-data.h"
#include "preferences.h"
#include "util.h"

static void get_cpu0_name(char* cpuname) {
	char *buf = NULL;
	size_t n = 0;

	FILE *f = fopen("/proc/cpuinfo", "r");
	if (f != NULL) {
		while(TRUE) {
			if (getline(&buf, &n, f) < 0)
				break;
			if (strncmp(buf, "model name", 10) == 0) {
				strcpy(cpuname, strchr(buf, ':')+2);
				cpuname[strlen(cpuname)-1] = 0;
				break;
			}
		}
		free(buf);
		fclose(f);
	}
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
multiload_graph_cpu_get_data (int Maximum, int data [4], LoadGraph *g)
{
	guint32 user, nice, sys, iowait, idle, total;
	gboolean first_call = FALSE;
	glibtop_cpu cpu;
	glibtop_uptime uptime;

	enum {
		CPU_USER	= 0,
		CPU_NICE	= 1,
		CPU_SYS		= 2,
		CPU_IOWAIT	= 3,
		CPU_IDLE	= 4,

		CPU_MAX		= 5
	};

	guint64 time[CPU_MAX];

	static const guint64 needed_flags_cpu =
		(1 << GLIBTOP_CPU_USER) +
		(1 << GLIBTOP_CPU_IDLE) +
		(1 << GLIBTOP_CPU_SYS) +
		(1 << GLIBTOP_CPU_NICE);

	static const guint64 needed_flags_uptime =
		(1 << GLIBTOP_UPTIME_UPTIME);

	CpuData *xd = (CpuData*) g->extra_data;
	g_assert_nonnull(xd);
	if (xd->num_cpu == 0) {
		get_cpu0_name(xd->cpu0_name);
		xd->num_cpu = 1 + glibtop_global_server->ncpu;
		first_call = TRUE;
	}


	glibtop_get_cpu (&cpu);
	g_return_if_fail ((cpu.flags & needed_flags_cpu) == needed_flags_cpu);

	get_cpu0_info(&xd->cpu0_mhz, xd->cpu0_governor);

	time [CPU_USER]		= cpu.user;
	time [CPU_NICE]		= cpu.nice;
	time [CPU_SYS]		= cpu.sys;
	time [CPU_IOWAIT]	= cpu.iowait + cpu.irq + cpu.softirq;
	time [CPU_IDLE]		= cpu.idle;

	glibtop_get_uptime(&uptime);
	if ((uptime.flags & needed_flags_uptime) == needed_flags_uptime)
		xd->uptime = uptime.uptime;

	if (!first_call) {
		user	= time [CPU_USER]	- xd->last [CPU_USER];
		nice	= time [CPU_NICE]	- xd->last [CPU_NICE];
		sys		= time [CPU_SYS]	- xd->last [CPU_SYS];
		iowait	= time [CPU_IOWAIT]	- xd->last [CPU_IOWAIT];
		idle	= time [CPU_IDLE]	- xd->last [CPU_IDLE];

		total = user + nice + sys + iowait + idle;

		xd->user			= (float)(user) / total;
		xd->iowait			= (float)(iowait) / total;
		xd->total_use		= (float)(total-idle) / total;

		data [CPU_USER]		= rint (Maximum * (float)(user)   / total);
		data [CPU_NICE]		= rint (Maximum * (float)(nice)   / total);
		data [CPU_SYS]		= rint (Maximum * (float)(sys)    / total);
		data [CPU_IOWAIT]	= rint (Maximum * (float)(iowait) / total);
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
