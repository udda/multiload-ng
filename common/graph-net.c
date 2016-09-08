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

#include <ctype.h>
#include <math.h>
#include <net/if.h>

#include "graph-data.h"
#include "autoscaler.h"
#include "preferences.h"
#include "util.h"


typedef struct {
	char name[32];
	guint64 rx_bytes;
	guint64 tx_bytes;

	FILE *f_address;
	char address[40]; // IPv6

	FILE* f_flags;
	guint flags;

	FILE *f_ifindex;
	guint ifindex;
} if_data;


static gint
sort_if_data_by_ifindex (gconstpointer a, gconstpointer b)
{
	if (((if_data*)a)->ifindex > ((if_data*)b)->ifindex)
		return 1;
	else if (((if_data*)a)->ifindex < ((if_data*)b)->ifindex)
		return -1;
	else // equals
		return 0;
}


gchar *
multiload_graph_net_get_filter (LoadGraph *g, NetData *xd)
{
	gboolean present;
	guint i;

	FILE *f;
	char *buf = NULL;
	char *start, *end;
	size_t n = 0;

	char iface[12];
	char *filter;

	char **active_filter = g_strsplit(g->config->filter, MULTILOAD_FILTER_SEPARATOR_INLINE, -1);

	f = cached_fopen_r("/proc/net/dev", FALSE);
	for (i=0; getline(&buf, &n, f) >= 0;) {
		// skip header lines of /proc/net/dev
		if (strchr(buf, ':') != NULL)
			i++;
	}
	filter = g_malloc0(i+(2+sizeof(iface)));

	buf=NULL, n=0;
	f = cached_fopen_r("/proc/net/dev", FALSE);
	while (getline(&buf, &n, f) >= 0) {
		end = strchr(buf, ':');
		// skip header lines of /proc/net/dev
		if (end == NULL)
			continue;

		for (start=buf; isspace(*start); start++) {}

		g_snprintf(iface, end-start+1, "%s", start);

		for (i=0, present=FALSE; active_filter[i] != NULL; i++) {
			if (strcmp(active_filter[i], iface) == 0) {
				present = TRUE;
				break;
			}
		}

		strcat(filter, present?"+":"-");
		strcat(filter, iface);
		strcat(filter, MULTILOAD_FILTER_SEPARATOR);
	}
	filter[strlen(filter)-1] = '\0';

	g_strfreev(active_filter);
	return filter;
}


void
multiload_graph_net_get_data (int Maximum, int data [3], LoadGraph *g, NetData *xd)
{
	enum {
		NET_IN		= 0,
		NET_OUT		= 1,
		NET_LOCAL	= 2,

		NET_MAX		= 3
	};

	static GHashTable *table = NULL;
	static gboolean first_call = TRUE;

	char *buf = NULL;
	char tmp[PATH_MAX];
	size_t n = 0;
	uint i,j;
	ulong valid_ifaces_len=0;
	FILE *f_net;

	guint64 present[NET_MAX] = { 0, 0, 0 };
	gint64 delta[NET_MAX];
	gint64 total = 0;

	if_data d;
	if_data *d_ptr;

	if (g->filter_changed) {
		first_call = TRUE;
		g->filter_changed = FALSE;
	}

	GArray *valid_ifaces = g_array_sized_new(TRUE, FALSE, sizeof(if_data), 10);

	if (table == NULL) {
		table = g_hash_table_new (g_str_hash, g_str_equal);
	}

	xd->ifaces[0] = 0;

	f_net = cached_fopen_r("/proc/net/dev", FALSE);
	while (getline(&buf, &n, f_net) >= 0) {
		// skip header lines of /proc/net/dev
		if (strchr(buf, ':') == NULL)
			continue;

		if (3 != sscanf(buf, "%s %lu %*u %*u %*u %*u %*u %*u %*u %lu", d.name, &d.rx_bytes, &d.tx_bytes))
			continue; // bad data
		d.name[strlen(d.name)-1] = '\0'; // remove trailing colon

		// lookup existing data and create it if necessary
		d_ptr = (if_data*)g_hash_table_lookup(table, d.name);
		if (d_ptr == NULL) {
			d_ptr = g_new(if_data,1);
			strcpy(d_ptr->name, d.name);

			sprintf(tmp, "/sys/class/net/%s/address", d_ptr->name);
			d_ptr->f_address = fopen(tmp, "r");
			if (d_ptr->f_address == NULL) {
				g_free(d_ptr);
				continue;
			}

			sprintf(tmp, "/sys/class/net/%s/flags", d_ptr->name);
			d_ptr->f_flags = fopen(tmp, "r");
			if (d_ptr->f_flags == NULL) {
				g_free(d_ptr);
				continue;
			}

			sprintf(tmp, "/sys/class/net/%s/ifindex", d_ptr->name);
			d_ptr->f_ifindex = fopen(tmp, "r");
			if (d_ptr->f_ifindex == NULL) {
				g_free(d_ptr);
				continue;
			}

			g_hash_table_insert(table, d_ptr->name, d_ptr);
		}

		d_ptr->rx_bytes = d.rx_bytes;
		d_ptr->tx_bytes = d.tx_bytes;

		rewind(d_ptr->f_flags);
		if (1 != fscanf(d_ptr->f_flags, "%x", &d_ptr->flags))
			continue; // bad data

		if (!(d_ptr->flags & IFF_UP))
			continue; // device is down, ignore

		rewind(d_ptr->f_address);
		if (1 != fscanf(d_ptr->f_address, "%s", d_ptr->address))
			continue; // bad data

		rewind(d_ptr->f_ifindex);
		if (1 != fscanf(d_ptr->f_ifindex, "%u", &d_ptr->ifindex))
			continue; // bad data

		// all OK - add interface to valid list
		g_array_append_val(valid_ifaces, *d_ptr);
		valid_ifaces_len++;
	}

	// sort array by ifindex (so we can take first device when they are same address)
	g_array_sort(valid_ifaces, sort_if_data_by_ifindex);

	for (i=0; i<valid_ifaces_len; i++) {
		d_ptr = &g_array_index(valid_ifaces, if_data, i);
		if (d_ptr == NULL)
			break;

		gboolean can_add = TRUE;

		// find devices with same HW address (e.g. ifaces put in monitor mode from airmon-ng)
		for (j=0; j<i; j++) {
			if_data *d_tmp = &g_array_index(valid_ifaces, if_data, j);
			if (strcmp(d_tmp->address, d_ptr->address) == 0) {
				g_debug("[graph-net] Ignored interface '%s' because has the same HW address of '%s' (%s)", d_tmp->name, d_ptr->name, d_ptr->address);
				can_add = FALSE;
				break;
			}
		}

		if (can_add && g->config->filter_enable) {
			can_add = FALSE;
			gchar ** filter_array = g_strsplit(g->config->filter, MULTILOAD_FILTER_SEPARATOR_INLINE, -1);
			for (j=0; filter_array[j]!=NULL; j++) {
				if (strcmp(filter_array[j], d_ptr->name) == 0) {
					can_add = TRUE;
					break;
				}
			}
			if (!can_add)
				g_debug("[graph-net] Ignored interface '%s' due to user filter", d_ptr->name);
			g_strfreev(filter_array);
		}

		if (!can_add)
			continue;

		if (d_ptr->flags & IFF_LOOPBACK) {
			present[NET_LOCAL] += d_ptr->rx_bytes;
		} else {
			present[NET_IN] += d_ptr->rx_bytes;
			present[NET_OUT] += d_ptr->tx_bytes;
		}

		g_strlcat (xd->ifaces, d_ptr->name, sizeof(xd->ifaces));
		g_strlcat (xd->ifaces, ", ", sizeof(xd->ifaces));
	}

	g_array_free(valid_ifaces, TRUE);
	xd->ifaces[strlen(xd->ifaces)-2] = 0;


	if (G_UNLIKELY(first_call)) { // avoid initial spike
		first_call = FALSE;

		xd->in_speed = 0;
		xd->out_speed = 0;
		xd->local_speed = 0;

		memset(data, 0, NET_MAX * sizeof data[0]);
	} else {
		for (i = 0; i < NET_MAX; i++) {
			delta[i] = present[i] - xd->last[i];
			if (delta[i] < 0) {
				g_warning("[graph-net] Measured negative delta for traffic #%d. This is a bug.", i);
				continue
			}
			total += delta[i];
		}

		int max = autoscaler_get_max(&xd->scaler, g, total);

		xd->in_speed	= calculate_speed(delta[NET_IN], 	g->config->interval);
		xd->out_speed	= calculate_speed(delta[NET_OUT],	g->config->interval);
		xd->local_speed	= calculate_speed(delta[NET_LOCAL],	g->config->interval);

		for (i=0; i<NET_MAX; i++)
			data[i] = rint (Maximum * (float)delta[i] / max);
	}

	memcpy(xd->last, present, sizeof xd->last);
}


void
multiload_graph_net_cmdline_output (LoadGraph *g, NetData *xd)
{
	if (g->output_unit[0] == '\0')
		g_strlcpy(g->output_unit, "Bps", sizeof(g->output_unit));
	g_snprintf(g->output_str[0], sizeof(g->output_str[0]), "%ld", xd->in_speed);
	g_snprintf(g->output_str[1], sizeof(g->output_str[1]), "%ld", xd->out_speed);
	g_snprintf(g->output_str[2], sizeof(g->output_str[2]), "%ld", xd->local_speed);
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
