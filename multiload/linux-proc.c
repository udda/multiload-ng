/* From wmload.c, v0.9.2, licensed under the GPL. */
#include <config.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
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

#include "util.h"
#include "linux-proc.h"
#include "autoscaler.h"




static const unsigned needed_netload_flags =
(1 << GLIBTOP_NETLOAD_IF_FLAGS) +
(1 << GLIBTOP_NETLOAD_BYTES_TOTAL);


void
GetCpu (int Maximum, int data [4], LoadGraph *g)
{
	guint32 user, nice, sys, iowait, idle, total;
	gboolean first_call = FALSE;
	glibtop_cpu cpu;

	static const guint64 needed_flags =
		(1 << GLIBTOP_CPU_USER) +
		(1 << GLIBTOP_CPU_IDLE) +
		(1 << GLIBTOP_CPU_SYS) +
		(1 << GLIBTOP_CPU_NICE);

	CpuData *xd = (CpuData*) g->extra_data;
	g_assert_nonnull(xd);
	if (xd->num_cpu == 0) {
		xd->num_cpu = 1 + glibtop_global_server->ncpu;
		first_call = TRUE;
	}


	glibtop_get_cpu (&cpu);
	g_return_if_fail ((cpu.flags & needed_flags) == needed_flags);

	xd->cpu_time [0] = cpu.user;
	xd->cpu_time [1] = cpu.nice;
	xd->cpu_time [2] = cpu.sys;
	xd->cpu_time [3] = cpu.iowait + cpu.irq + cpu.softirq;
	xd->cpu_time [4] = cpu.idle;

	if (!first_call) {
		user	= xd->cpu_time [0] - xd->cpu_last [0];
		nice	= xd->cpu_time [1] - xd->cpu_last [1];
		sys		= xd->cpu_time [2] - xd->cpu_last [2];
		iowait	= xd->cpu_time [3] - xd->cpu_last [3];
		idle	= xd->cpu_time [4] - xd->cpu_last [4];

		total = user + nice + sys + iowait + idle;

		xd->user		= (float)(user) / total;
		xd->iowait		= (float)(iowait) / total;
		xd->total_use	= (float)(total-idle) / total;

		data [0] = rint (Maximum * (float)(user)   / total);
		data [1] = rint (Maximum * (float)(nice)   / total);
		data [2] = rint (Maximum * (float)(sys)    / total);
		data [3] = rint (Maximum * (float)(iowait) / total);
	}

	memcpy(xd->cpu_last, xd->cpu_time, sizeof xd->cpu_last);
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
	enum Types {
		IN_COUNT = 0,
		OUT_COUNT = 1,
		LOCAL_COUNT = 2,
		COUNT_TYPES = 3
	};

	static int ticks = 0;
	static gulong past[COUNT_TYPES] = {0};
	static AutoScaler scaler;

	gulong present[COUNT_TYPES] = {0};

	guint i;
	gchar **devices;
	glibtop_netlist netlist;
	gchar path[PATH_MAX];

	NetData *xd = (NetData*) g->extra_data;
	g_assert_nonnull(xd);



	if (ticks == 0)
		autoscaler_init(&scaler, 60, 501);


	devices = glibtop_get_netlist(&netlist);

	for (i = 0; i < netlist.number; ++i) {
		glibtop_netload netload;

		glibtop_get_netload(&netload, devices[i]);

		g_return_if_fail((netload.flags & needed_netload_flags) == needed_netload_flags);

		if (!(netload.if_flags & (1L << GLIBTOP_IF_FLAGS_UP)))
			continue;

		if (netload.if_flags & (1L << GLIBTOP_IF_FLAGS_LOOPBACK)) {
			/* for loopback in and out are identical, so only count in */
			present[LOCAL_COUNT] += netload.bytes_in;
			continue;
		}

		/* Do not include virtual devices (any device not corresponding to a
		 * physical device: VPN, PPPOE...) to avoid counting the same
		 * throughput several times.
		 * First check if /sys/class/net/DEVNAME/ exists (if not, may be old
		 * linux kernel or not linux at all). */
		sprintf(path, "/sys/class/net/%s", devices[i]);
		if (access(path, F_OK) == 0) {
			/* /sys/class/net/DEVNAME exists. Now check for another dir: physical
			 * devices have a 'device' symlink in /sys/class/net/DEVNAME */
			sprintf(path, "/sys/class/net/%s/device", devices[i]);
			if (access(path, F_OK) != 0) {
				/* symlink does not exist, device is virtual */
				continue;
			}
		}


		present[IN_COUNT] += netload.bytes_in;
		present[OUT_COUNT] += netload.bytes_out;
	}

	g_strfreev(devices);
	netspeed_add(xd->in, present[IN_COUNT]);
	netspeed_add(xd->out, present[OUT_COUNT]);

	if(ticks < 2) { /* avoid initial spike */
		ticks++;
		memset(data, 0, COUNT_TYPES * sizeof data[0]);
	} else {
		int delta[COUNT_TYPES];
		int max;
		int total = 0;

		for (i = 0; i < COUNT_TYPES; i++) {
			/* protect against weirdness */
			if (present[i] >= past[i])
				delta[i] = (present[i] - past[i]);
			else
				delta[i] = 0;
			total += delta[i];
		}

		max = autoscaler_get_max(&scaler, total);

		for (i = 0; i < COUNT_TYPES; i++)
			data[i]   = rint (Maximum * (float)delta[i]  / max);
	}

	memcpy(past, present, sizeof past);
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
	float max;
	float current;

	static const guint64 needed_flags =
		(1 << GLIBTOP_LOADAVG_LOADAVG);

	LoadAvgData *xd = (LoadAvgData*) g->extra_data;
	g_assert_nonnull(xd);

	glibtop_get_loadavg (&loadavg);
	g_return_if_fail ((loadavg.flags & needed_flags) == needed_flags);

	max = PER_CPU_MAX_LOADAVG * (1 + glibtop_global_server->ncpu);
	current = MIN(loadavg.loadavg[0], max);

	memcpy(xd->loadavg, loadavg.loadavg, sizeof loadavg.loadavg);

	data [0] = rint ((float) Maximum * current / max);
}


void
GetDisk (int Maximum, int data [2], LoadGraph *g)
{
	gboolean first_call = FALSE;

	static const guint64 needed_flags =
		(1 << GLIBTOP_FSUSAGE_BLOCK_SIZE) +
		(1 << GLIBTOP_FSUSAGE_READ) +
		(1 << GLIBTOP_FSUSAGE_WRITE);

	DiskData *xd = (DiskData*) g->extra_data;
	g_assert_nonnull(xd);
	if (xd->scaler.floor == 0) {
		autoscaler_init(&xd->scaler, 60, 500);
		first_call = TRUE;
	}

	guint i;
	int max;

	guint64 read = 0;
	guint64 write = 0;
	guint64 readdiff, writediff;

	glibtop_mountlist mountlist;
	glibtop_mountentry *mountentries = glibtop_get_mountlist (&mountlist, FALSE);

	for (i = 0; i < mountlist.number; i++) {
		if (   strcmp (mountentries[i].type, "smbfs") == 0
			|| strcmp (mountentries[i].type, "nfs") == 0
			|| strcmp (mountentries[i].type, "cifs") == 0
			|| strncmp(mountentries[i].type, "fuse.", 5) == 0)
			continue;

		glibtop_fsusage fsusage;
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

	if (first_call)
		return;

	max = autoscaler_get_max(&xd->scaler, readdiff + writediff);

	data[0] = (float)Maximum *  readdiff / (float)max;
	data[1] = (float)Maximum * writediff / (float)max;

	/* read/write are relative to SECTORS (standard 512 byte) and not blocks
	 * as glibtop documentation states. So multiply value by 512 */
	xd->read_speed  = (guint64) ( (readdiff  * 512 * 1000.0)  / (g->multiload->speed) );
	xd->write_speed = (guint64) ( (writediff * 512 * 1000.0)  / (g->multiload->speed) );
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
		maxtemps = (guint*)  malloc( n_zones * sizeof  (guint) );
		memset(maxtemps, 0, n_zones * sizeof (guint));

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
				g_free(d_type);
			}
			paths[i] = g_strdup_printf("%s/temp", d_thermal);
		//	printf("[%d] %s :: %d\n", i, paths[i], maxtemps[i]);
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
	//	printf("read %d (%d)\n", i, t);
		if (t > temp) {
			temp = t;
			j = i;
	//		printf("MAX %d (%d)\n", j, t);
		}
	}

	data[0] = (float)Maximum * temp / (float)maxtemps[j];

	xd->temperature = temp;
}
