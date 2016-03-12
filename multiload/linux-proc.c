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

#include "linux-proc.h"
#include "autoscaler.h"

static const unsigned needed_cpu_flags =
(1 << GLIBTOP_CPU_USER) +
(1 << GLIBTOP_CPU_IDLE) +
(1 << GLIBTOP_CPU_SYS) +
(1 << GLIBTOP_CPU_NICE);

static const unsigned needed_fsusage_flags = 
(1 << GLIBTOP_FSUSAGE_BLOCK_SIZE) +
(1 << GLIBTOP_FSUSAGE_READ) +
(1 << GLIBTOP_FSUSAGE_WRITE);

static const unsigned needed_mem_flags =
(1 << GLIBTOP_MEM_USED) +
(1 << GLIBTOP_MEM_FREE);

static const unsigned needed_swap_flags =
(1 << GLIBTOP_SWAP_USED) +
(1 << GLIBTOP_SWAP_FREE);

static const unsigned needed_loadavg_flags =
(1 << GLIBTOP_LOADAVG_LOADAVG);

static const unsigned needed_netload_flags =
(1 << GLIBTOP_NETLOAD_IF_FLAGS) +
(1 << GLIBTOP_NETLOAD_BYTES_TOTAL);


void
GetLoad (int Maximum, int data [5], LoadGraph *g)
{
	int usr, nice, sys, iowait, free;
	int total;

	glibtop_cpu cpu;

	glibtop_get_cpu (&cpu);

	g_return_if_fail ((cpu.flags & needed_cpu_flags) == needed_cpu_flags);

	g->cpu_time [0] = cpu.user;
	g->cpu_time [1] = cpu.nice;
	g->cpu_time [2] = cpu.sys;
	g->cpu_time [3] = cpu.iowait + cpu.irq + cpu.softirq;
	g->cpu_time [4] = cpu.idle;

	if (!g->cpu_initialized) {
		memcpy (g->cpu_last, g->cpu_time, sizeof (g->cpu_last));
		g->cpu_initialized = 1;
	}

	usr  = g->cpu_time [0] - g->cpu_last [0];
	nice = g->cpu_time [1] - g->cpu_last [1];
	sys  = g->cpu_time [2] - g->cpu_last [2];
	iowait = g->cpu_time [3] - g->cpu_last [3];
	free = g->cpu_time [4] - g->cpu_last [4];

	total = usr + nice + sys + free + iowait;

	memcpy(g->cpu_last, g->cpu_time, sizeof g->cpu_last);

	usr  = rint (Maximum * (float)(usr)  / total);
	nice = rint (Maximum * (float)(nice) / total);
	sys  = rint (Maximum * (float)(sys)  / total);
	iowait = rint (Maximum * (float)(iowait) / total);
	free = Maximum - usr - nice - sys - iowait;

	data [0] = usr;
	data [1] = sys;
	data [2] = nice;
	data [3] = iowait;
	data [4] = free;
}

void
GetDiskLoad (int Maximum, int data [3], LoadGraph *g)
{
	static gboolean first_call = TRUE;
	static guint64 lastread = 0;
	static guint64 lastwrite = 0;
	static AutoScaler scaler;

	glibtop_mountlist mountlist;
	glibtop_mountentry *mountentries;
	glibtop_fsusage fsusage;

	guint i;
	int max;

	guint64 read = 0;
	guint64 write = 0;
	guint64 readdiff, writediff;

	mountentries = glibtop_get_mountlist (&mountlist, FALSE);

	for (i = 0; i < mountlist.number; i++) {
		if (   strcmp (mountentries[i].type, "smbfs") == 0
			|| strcmp (mountentries[i].type, "nfs") == 0
			|| strcmp (mountentries[i].type, "cifs") == 0
			|| strncmp(mountentries[i].type, "fuse.", 5) == 0)
			continue;

		glibtop_get_fsusage(&fsusage, mountentries[i].mountdir);
		if ((fsusage.flags & needed_fsusage_flags) != needed_fsusage_flags)
			continue; // FS does not have required capabilities

		read  += fsusage.block_size * fsusage.read;
		write += fsusage.block_size * fsusage.write;
	}

	g_free(mountentries);

	readdiff  = read  - lastread;
	writediff = write - lastwrite;

	lastread  = read;
	lastwrite = write;

	if (first_call) {
		first_call = FALSE;
		autoscaler_init(&scaler, 60, 500);
		memset(data, 0, 3 * sizeof data[0]);
		return;
	}

	max = autoscaler_get_max(&scaler, readdiff + writediff);

	data[0] = (float)Maximum *  readdiff / (float)max;
	data[1] = (float)Maximum * writediff / (float)max;
	data[2] = (float)Maximum - (data [0] + data[1]);

	// TODO HACK: I don't know why these speeds need to be divided by 8...
	g->diskread  = (guint64) ( (readdiff  * 1000.0)  / (8 * g->multiload->speed) );
	g->diskwrite = (guint64) ( (writediff * 1000.0)  / (8 * g->multiload->speed) );
}

static int
read_temp_from_file(const gchar *path) {
	FILE *f;
	size_t s;
	// temperatures are in millicelsius, 8 chars are enough
	gchar buf[8];


	f = fopen(path, "r");
	if (!f)
		return 0;

	s = fread(buf, 1, sizeof(buf), f);
	fclose(f);

	if (s < 1)
		return 0;

	return atoi(buf);
}

static gboolean
file_check_contents(FILE *f, const gchar *string) {
	size_t n;
	size_t s;
	gchar *buf;

	n = strlen(string);
	buf = (gchar*)malloc(n);

	s = fread(buf, 1, n, f);

	if (s != n)
		return FALSE;

	if (strncmp(buf, string, n) != 0)
		return FALSE;

	return TRUE;
}

void
GetTemperature (int Maximum, int data[2], LoadGraph *g)
{
	guint temp = 0;

	guint i, j, t;

	DIR *dir;
	struct dirent *entry;

	static gboolean first_call = TRUE;
	static gboolean support = FALSE;

	// hold path and max temp for each thermal zone, filled on first call
	static guint n_zones = 0;
	static gchar **paths = NULL;
	static guint *maxtemps = NULL;

	// handle errors by providing empty data if something goes wrong
	memset(data, 0, 2 * sizeof data[0]);

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
					t = read_temp_from_file(d_temp);
					g_free(d_temp);
					if (t > maxtemps[i])
						maxtemps[i] = t;
				}
				g_free(d_type);
			}
			paths[i] = g_strdup_printf("%s/temp", d_thermal);
//			printf("[%d] %s :: %d\n", i, paths[i], maxtemps[i]);
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
		t = read_temp_from_file(paths[i]);
//		printf("read %d (%d)\n", i, t);
		if (t > temp) {
			temp = t;
			j = i;
//			printf("MAX %d (%d)\n", j, t);
		}
	}

	data[0] = (float)Maximum * temp / (float)maxtemps[j];
	data[1] = Maximum - data[0];

	g->temperature = temp;
}

void
GetMemory (int Maximum, int data [5], LoadGraph *g)
{
	int user, shared, buffer, cached;

	glibtop_mem mem;

	glibtop_get_mem (&mem);

	g_return_if_fail ((mem.flags & needed_mem_flags) == needed_mem_flags);

	user    = rint (Maximum * (float)mem.user   / (float)mem.total);
	shared  = rint (Maximum * (float)mem.shared / (float)mem.total);
	buffer  = rint (Maximum * (float)mem.buffer / (float)mem.total);
	cached  = rint (Maximum * (float)mem.cached / (float)mem.total);

	data [0] = user;
	data [1] = shared;
	data [2] = buffer;
	data [3] = cached;
	data [4] = Maximum-user-shared-buffer-cached;
}

void
GetSwap (int Maximum, int data [2], LoadGraph *g)
{
	int used;

	glibtop_swap swap;

	glibtop_get_swap (&swap);
	g_return_if_fail ((swap.flags & needed_swap_flags) == needed_swap_flags);

	if (swap.total == 0)
	   used = 0;
	else
	   used = rint (Maximum * (float)swap.used / swap.total);

	data [0] = used;
	data [1] = Maximum - used;
}

void
GetLoadAvg (int Maximum, int data [2], LoadGraph *g)
{
	const float per_cpu_max_loadavg = 5.0f;
	float max_loadavg;
	float loadavg1;
	float used;

	glibtop_loadavg loadavg;
	glibtop_get_loadavg (&loadavg);

	g_return_if_fail ((loadavg.flags & needed_loadavg_flags) == needed_loadavg_flags);

	max_loadavg = per_cpu_max_loadavg * (1 + glibtop_global_server->ncpu);

	g->loadavg1 = loadavg.loadavg[0];
	loadavg1 = MIN(loadavg.loadavg[0], max_loadavg);

	used    = loadavg1 / max_loadavg;

	data [0] = rint ((float) Maximum * used);
	data [1] = Maximum - data[0];
}

/*
 * Return true if a network device (identified by its name) is virtual
 * (ie: not corresponding to a physical device). In case it is a physical
 * device or unknown, returns false.
 */
static gboolean
is_net_device_virtual(char *device)
{
	/*
	 * There is not definitive way to find out. On some systems (Linux
	 * kernels ≳ 2.19 without option SYSFS_DEPRECATED), there exist a
	 * directory /sys/devices/virtual/net which only contains virtual
	 * devices.  It's also possible to detect by the fact that virtual
	 * devices do not have a symlink "device" in
	 * /sys/class/net/name-of-dev/ .  This second method is more complex
	 * but more reliable.
	 */
	char path[PATH_MAX];

	/* Check if /sys/class/net/name-of-dev/ exists (may be old linux kernel
	 * or not linux at all). */
	if (sprintf(path, "/sys/class/net/%s", device) < 0)
		return FALSE;
	if (access(path, F_OK) != 0)
		return FALSE; /* unknown */

	if (sprintf(path, "/sys/class/net/%s/device", device) < 0)
		return FALSE;
	if (access(path, F_OK) != 0)
		return TRUE;

	return FALSE;
}

void
GetNet (int Maximum, int data [4], LoadGraph *g)
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

		/*
		 * Do not include virtual devices (VPN, PPPOE...) to avoid
		 * counting the same throughput several times.
		 */
		if (is_net_device_virtual(devices[i]))
			continue;

		present[IN_COUNT] += netload.bytes_in;
		present[OUT_COUNT] += netload.bytes_out;
	}

	g_strfreev(devices);
	netspeed_add(g->netspeed_in, present[IN_COUNT]);
	netspeed_add(g->netspeed_out, present[OUT_COUNT]);

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

	//data[4] = Maximum - data[3] - data[2] - data[1] - data[0];
	data[COUNT_TYPES] = Maximum;
	for (i = 0; i < COUNT_TYPES; i++)
		data[COUNT_TYPES] -= data[i];

	memcpy(past, present, sizeof past);
}



