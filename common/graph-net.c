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
#include <glibtop/netlist.h>
#include <glibtop/netload.h>

#include "graph-data.h"
#include "autoscaler.h"
#include "preferences.h"
#include "util.h"


void
multiload_graph_net_get_data (int Maximum, int data [3], LoadGraph *g, NetData *xd)
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
multiload_graph_net_tooltip_update (char **title, char **text, LoadGraph *g, NetData *xd)
{
	gchar *tx_in = format_rate_for_display(xd->in_speed);
	gchar *tx_out = format_rate_for_display(xd->out_speed);
	gchar *tx_local = format_rate_for_display(xd->local_speed);

	if (g->config->tooltip_style == TOOLTIP_STYLE_DETAILS) {
		*text = g_strdup_printf(_(	"Monitored interfaces: %s\n"
									"\n"
									"Receiving: %s\n"
									"Sending: %s\n"
									"Local: %s"),
									xd->ifaces, tx_in, tx_out, tx_local);
	} else {
		*text = g_strdup_printf("\xe2\xac\x87%s \xe2\xac\x86%s", tx_in, tx_out);
	}

	g_free(tx_in);
	g_free(tx_out);
	g_free(tx_local);
}
