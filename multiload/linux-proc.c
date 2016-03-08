/* From wmload.c, v0.9.2, licensed under the GPL. */
#include <config.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>

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

#if 0
static const unsigned needed_page_flags =
(1 << GLIBTOP_SWAP_PAGEIN) +
(1 << GLIBTOP_SWAP_PAGEOUT);
#endif

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
	static guint64 lastread = 0, lastwrite = 0;
	static AutoScaler scaler;

	glibtop_mountlist mountlist;
	glibtop_mountentry *mountentries;
	guint i;
	int max;

	guint64 read, write;
	guint64 readdiff, writediff;


	if(first_call)
	{
	    autoscaler_init(&scaler, 60, 500);
	}

	read = write = 0;

	mountentries = glibtop_get_mountlist (&mountlist, FALSE);

	for (i = 0; i < mountlist.number; i++)
	{
		glibtop_fsusage fsusage;

		if (strcmp(mountentries[i].type, "smbfs") == 0
		    || strcmp(mountentries[i].type, "nfs") == 0
		    || strcmp(mountentries[i].type, "cifs") == 0
		    || strcmp(mountentries[i].type, "fuse.sshfs") == 0)
			continue;

		glibtop_get_fsusage(&fsusage, mountentries[i].mountdir);
		read += fsusage.read; write += fsusage.write;
	}

	g_free(mountentries);

	readdiff  = read - lastread;
	writediff = write - lastwrite;

	lastread  = read;
	lastwrite = write;

	if(first_call)
	{
		first_call = FALSE;
		memset(data, 0, 3 * sizeof data[0]);
		return;
	}

	max = autoscaler_get_max(&scaler, readdiff + writediff);

	data[0] = (float)Maximum *  readdiff / (float)max;
	data[1] = (float)Maximum * writediff / (float)max;
	data[2] = (float)Maximum - (data [0] + data[1]);
}

#if 0
void
GetPage (int Maximum, int data [3], LoadGraph *g)
{
    static int max = 100; /* guess at maximum page rate (= in + out) */
    static u_int64_t lastin = 0;
    static u_int64_t lastout = 0;
    int in, out, idle;

    glibtop_swap swap;
	
    glibtop_get_swap (&swap);
	
    assert ((swap.flags & needed_page_flags) == needed_page_flags);

    if ((lastin > 0) && (lastin < swap.pagein)) {
	in = swap.pagein - lastin;
    }
    else {
	in = 0;
    }
    lastin = swap.pagein;

    if ((lastout > 0) && (lastout < swap.pageout)) {
	out = swap.pageout - lastout;
    }
    else {
	out = 0;
    }
    lastout = swap.pageout;

    if ((in + out) > max) {
	/* Maximum page rate has increased. Change the scale without
	   any indication whatsoever to the user (not a nice thing to
	   do). */
	max = in + out;
    }

    in   = rint (Maximum * ((float)in / max));
    out  = rint (Maximum * ((float)out / max));
    idle = Maximum - in - out;

    data [0] = in;
    data [1] = out;
    data [2] = idle;
}
#endif /* 0 */

void
GetMemory (int Maximum, int data [5], LoadGraph *g)
{
    int user, shared, buffer, cached;
    
    glibtop_mem mem;
	
    glibtop_get_mem (&mem);
	
    g_return_if_fail ((mem.flags & needed_mem_flags) == needed_mem_flags);

    user    = rint (Maximum * (float)mem.user / (float)mem.total);
    shared  = rint (Maximum * (float)mem.shared / (float)mem.total);
    buffer  = rint (Maximum * (float)mem.buffer / (float)mem.total);
    cached = rint (Maximum * (float)mem.cached / (float)mem.total);
    
    data [0] = user;
    data [1] = shared;
    data [2] = buffer;
    data[3] = cached;
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
	   used    = rint (Maximum * (float)swap.used / swap.total);

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


    if(ticks == 0)
    {
	autoscaler_init(&scaler, 60, 501);
    }


    devices = glibtop_get_netlist(&netlist);

    for(i = 0; i < netlist.number; ++i)
    {
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

    if(ticks < 2) /* avoid initial spike */
    {
	ticks++;
	memset(data, 0, COUNT_TYPES * sizeof data[0]);
    }
    else
    {
	int delta[COUNT_TYPES];
	int max;
	int total = 0;

	for (i = 0; i < COUNT_TYPES; i++)
	{
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



