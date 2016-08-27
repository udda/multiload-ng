/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *                    The Free Software Foundation
 *
 * With code from wmload.c, v0.9.2,
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
#include <dirent.h>

#include <glibtop.h>
#include <glibtop/cpu.h>
#include <glibtop/mem.h>
#include <glibtop/swap.h>
#include <glibtop/loadavg.h>
#include <glibtop/netload.h>
#include <glibtop/netlist.h>
#include <glibtop/mountlist.h>
#include <glibtop/fsusage.h>
#include <glibtop/uptime.h>

#include "util.h"
#include "linux-proc.h"
#include "autoscaler.h"


#define PER_CPU_MAX_LOADAVG 3



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
GetCpu (int Maximum, int data [4], LoadGraph *g)
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
GetMemory (int Maximum, int data [4], LoadGraph *g)
{
	glibtop_mem mem;

	static const guint64 needed_flags =
		(1 << GLIBTOP_MEM_USER) +
		(1 << GLIBTOP_MEM_SHARED) +
		(1 << GLIBTOP_MEM_BUFFER) +
		(1 << GLIBTOP_MEM_CACHED) +
		(1 << GLIBTOP_MEM_FREE) +
		(1 << GLIBTOP_MEM_TOTAL);

	MemoryData *xd = (MemoryData*) g->extra_data;
	g_assert_nonnull(xd);


	glibtop_get_mem (&mem);
	g_return_if_fail ((mem.flags & needed_flags) == needed_flags);

	xd->user = mem.user;
	xd->cache = mem.shared + mem.buffer + mem.cached;
	xd->total = mem.total;

	data [0] = rint (Maximum * (float)mem.user   / (float)mem.total);
	data [1] = rint (Maximum * (float)mem.shared / (float)mem.total);
	data [2] = rint (Maximum * (float)mem.buffer / (float)mem.total);
	data [3] = rint (Maximum * (float)mem.cached / (float)mem.total);
}


void
GetNet (int Maximum, int data [3], LoadGraph *g)
{
	glibtop_netlist netlist;
	glibtop_netload netload;
	static int ticks = 0;
	gchar path[PATH_MAX];
	gchar **devices;
	guint i;
	guint ifacelen;

	enum {
		NET_IN		= 0,
		NET_OUT		= 1,
		NET_LOCAL	= 2,

		NET_MAX		= 3
	};

	guint64 present[NET_MAX];

	static const unsigned needed_flags =
		(1 << GLIBTOP_NETLOAD_IF_FLAGS) +
		(1 << GLIBTOP_NETLOAD_BYTES_TOTAL);

	NetData *xd = (NetData*) g->extra_data;
	g_assert_nonnull(xd);


	ifacelen = sizeof(xd->ifaces)/sizeof(gchar);
	xd->ifaces[0] = 0;

	devices = glibtop_get_netlist(&netlist);

	for ( i=0; i<netlist.number; i++ ) {
		/* Exclude virtual devices (any device not corresponding to a
		 * physical device) to avoid counting the same throughput several times.
		 * First check if /sys/class/net/DEVNAME/ exists (if not, may be old
		 * linux kernel or not linux at all). */
		sprintf(path, "/sys/class/net/%s", devices[i]);
		if (access(path, F_OK) == 0) {
			sprintf(path, "/sys/class/net/%s/device", devices[i]);
			if (access(path, F_OK) != 0) {
				// symlink does not exist, device is virtual
				continue;
			}
		}

		glibtop_get_netload(&netload, devices[i]);
		g_return_if_fail((netload.flags & needed_flags) == needed_flags);

		if (!(netload.if_flags & (1L << GLIBTOP_IF_FLAGS_UP)))
			continue;

		if (netload.if_flags & (1L << GLIBTOP_IF_FLAGS_LOOPBACK)) {
			/* for loopback in and out are identical, so only count once */
			present[NET_LOCAL] += netload.bytes_in;
		} else {
			present[NET_IN] += netload.bytes_in;
			present[NET_OUT] += netload.bytes_out;
		}

		g_strlcat (xd->ifaces, devices[i], ifacelen);
		g_strlcat (xd->ifaces, ", ", ifacelen);
	}
	xd->ifaces[strlen(xd->ifaces)-2] = 0;

	g_strfreev(devices);

	if (ticks < 2) { /* avoid initial spike */
		ticks++;
		memset(data, 0, NET_MAX * sizeof data[0]);
	} else {
		int delta[NET_MAX];
		int max;
		int total = 0;

		for (i = 0; i < NET_MAX; i++) {
			/* protect against weirdness */
			if (present[i] >= xd->last[i])
				delta[i] = (present[i] - xd->last[i]);
			else
				delta[i] = 0;
			total += delta[i];
		}

		max = autoscaler_get_max(&xd->scaler, g, total);

		xd->in_speed	= calculate_speed(delta[NET_IN], 	g->config->interval);
		xd->out_speed	= calculate_speed(delta[NET_OUT],	g->config->interval);
		xd->local_speed	= calculate_speed(delta[NET_LOCAL],	g->config->interval);

		for ( i=0; i<NET_MAX; i++ )
			data[i] = rint (Maximum * (float)delta[i] / max);
	}

	memcpy(xd->last, present, sizeof xd->last);
}


void
GetSwap (int Maximum, int data [1], LoadGraph *g)
{
	glibtop_swap swap;

	static const guint64 needed_flags =
		(1 << GLIBTOP_SWAP_USED) +
		(1 << GLIBTOP_SWAP_FREE);

	SwapData *xd = (SwapData*) g->extra_data;
	g_assert_nonnull(xd);


	glibtop_get_swap (&swap);
	g_return_if_fail ((swap.flags & needed_flags) == needed_flags);

	xd->used = swap.used;
	xd->total = swap.total;

	if (swap.total == 0)
	   data [0] = 0;
	else
	   data [0] = rint (Maximum * (float)swap.used / swap.total);
}


void
GetLoadAvg (int Maximum, int data [1], LoadGraph *g)
{
	glibtop_loadavg loadavg;
	static float max = 0;
	float current;

	static const guint64 needed_flags =
		(1 << GLIBTOP_LOADAVG_LOADAVG);

	LoadAvgData *xd = (LoadAvgData*) g->extra_data;
	g_assert_nonnull(xd);

	glibtop_get_loadavg (&loadavg);
	g_return_if_fail ((loadavg.flags & needed_flags) == needed_flags);

	if (max == 0)
		max = PER_CPU_MAX_LOADAVG * (1 + glibtop_global_server->ncpu);
	current = MIN(loadavg.loadavg[0], max);

	memcpy(xd->loadavg, loadavg.loadavg, sizeof loadavg.loadavg);

	data [0] = rint ((float) Maximum * current / max);
}


void
GetDisk (int Maximum, int data [2], LoadGraph *g)
{
	glibtop_mountlist mountlist;
	glibtop_fsusage fsusage;
	static gboolean first_call = TRUE;

	static const guint64 needed_flags =
		(1 << GLIBTOP_FSUSAGE_BLOCK_SIZE) +
		(1 << GLIBTOP_FSUSAGE_READ) +
		(1 << GLIBTOP_FSUSAGE_WRITE);

	DiskData *xd = (DiskData*) g->extra_data;
	g_assert_nonnull(xd);

	guint i;
	int max;

	guint64 read = 0;
	guint64 write = 0;
	guint64 readdiff, writediff;

	glibtop_mountentry *mountentries = glibtop_get_mountlist (&mountlist, FALSE);

	for (i = 0; i < mountlist.number; i++) {
		if (   strcmp (mountentries[i].type, "smbfs") == 0
			|| strcmp (mountentries[i].type, "nfs") == 0
			|| strcmp (mountentries[i].type, "cifs") == 0
			|| strncmp(mountentries[i].type, "fuse.", 5) == 0)
			continue;

		glibtop_get_fsusage(&fsusage, mountentries[i].mountdir);
		if ((fsusage.flags & needed_flags) != needed_flags)
			continue; // FS does not have required capabilities

		read  += fsusage.read;
		write += fsusage.write;
	}

	g_free(mountentries);

	readdiff  = read  - xd->last_read;
	writediff = write - xd->last_write;

	xd->last_read  = read;
	xd->last_write = write;

	if (first_call) {
		first_call = FALSE;
		return;
	}

	max = autoscaler_get_max(&xd->scaler, g, readdiff + writediff);

	data[0] = (float)Maximum *  readdiff / (float)max;
	data[1] = (float)Maximum * writediff / (float)max;

	/* read/write are relative to SECTORS (standard 512 byte) and not blocks
	 * as glibtop documentation states. So multiply value by 512 */
	xd->read_speed  = calculate_speed(readdiff  * 512, g->config->interval);
	xd->write_speed = calculate_speed(writediff * 512, g->config->interval);
}


void
GetTemperature (int Maximum, int data[1], LoadGraph *g)
{
	guint temp = 0;
	guint i, j, t;

	DIR *dir;
	struct dirent *entry;

	TemperatureData *xd = (TemperatureData*) g->extra_data;
	g_assert_nonnull(xd);


	static gboolean first_call = TRUE;
	static gboolean support = FALSE;

	// hold path and max temp for each thermal zone, filled on first call
	static guint n_zones = 0;
	static gchar **paths = NULL;
	static guint *maxtemps = NULL;

	// handle errors by providing empty data if something goes wrong
	memset(data, 0, 1 * sizeof data[0]);

	if (G_UNLIKELY(first_call)) {
		first_call = FALSE;

		gchar *d_base = g_strdup("/sys/class/thermal");

		// check if /sys path exists
		dir = opendir(d_base);
		if (!dir)
			return;

		// count thermal_zoneX dirs
		while ((entry = readdir(dir)) != NULL) {
			if (strncmp(entry->d_name, "thermal_zone", 12) == 0)
				n_zones++;
		}

		// if there is at least one thermal zone, we can proceed
		if (n_zones > 0)
			support = TRUE;

		// allocate buffers
		paths    = (gchar**) malloc( n_zones * sizeof (gchar*) );
		maxtemps = g_new0(guint, n_zones);

		// fill buffers
		i=0;
		rewinddir(dir);
		while ((entry = readdir(dir)) != NULL) {
			if (strncmp(entry->d_name, "thermal_zone", 12) != 0)
				continue;

			gchar *d_thermal = g_strdup_printf("%s/%s", d_base, entry->d_name);

			// find "critical" (max) temperature searching in trip points
			for (j=0; ; j++) {
				gchar *d_type = g_strdup_printf("%s/trip_point_%d_type", d_thermal, j);
				FILE *f_type = fopen(d_type, "r");
				g_free(d_type);

				if (!f_type)
					break; //no more trip point files, stop searching
				gboolean found = file_check_contents(f_type, "critical");
				fclose(f_type);

				if (found) { // found critical temp
					gchar *d_temp = g_strdup_printf("%s/trip_point_%d_temp", d_thermal, j);
					t = read_int_from_file(d_temp);
					g_free(d_temp);
					if (t > maxtemps[i])
						maxtemps[i] = t;
				}
			}
			paths[i] = g_strdup_printf("%s/temp", d_thermal);
			i++;
			g_free(d_thermal);
		}
		closedir(dir);
		g_free(d_base);
	}

	// check if we have sysfs thermal support
	if (!support)
		return;

	// finds max temperature and its index (to use the respective maximum)
	for (i=0,j=0; i<n_zones; i++) {
		t = read_int_from_file(paths[i]);
		if (t > temp) {
			temp = t;
			j = i;
		}
	}

	if (maxtemps[j] > 0 && maxtemps[j] > temp)
		data[0] = (float)Maximum * temp / (float)maxtemps[j];
	else {
		int max = autoscaler_get_max(&xd->scaler, g, temp);
		data[0] = rint (Maximum * (float)temp / max);
	}

	xd->value = temp;
	xd->max = maxtemps[j];
}
