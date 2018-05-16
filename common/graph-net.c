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
#include "info-file.h"
#include "preferences.h"
#include "util.h"


#define PATH_NET_DEV "/proc/net/dev"

typedef struct {
	char name[32];
	guint64 rx_bytes;
	guint64 tx_bytes;

	gchar path_address[PATH_MAX];
	char address[40]; // enough for IPv6 addresses

	gchar path_flags[PATH_MAX];
	guint64 flags;

	gchar path_ifindex[PATH_MAX];
	guint64 ifindex;

	gchar path_carrier[PATH_MAX];
	guint64 carrier;
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


void
multiload_graph_net_init (LoadGraph *g, NetData *xd)
{

}

MultiloadFilter *
multiload_graph_net_get_filter (LoadGraph *g, NetData *xd)
{
	char *buf = NULL;
	size_t n = 0;

	char *start, *end;
	char iface[20];

	MultiloadFilter *filter = multiload_filter_new();

	FILE *f = info_file_required_fopen(PATH_NET_DEV, "r");
	while (getline(&buf, &n, f) >= 0) {
		// skip header lines of /proc/net/dev
		if ((end = strchr(buf, ':')) == NULL)
			continue;

		for (start=buf; isspace(*start); start++) {}

		g_snprintf(iface, end-start+1, "%s", start);

		multiload_filter_append(filter, iface);
	}
	g_free(buf);
	fclose(f);

	multiload_filter_import_existing(filter, g->config->filter);

	return filter;
}


void
multiload_graph_net_get_data (int Maximum, int data [3], LoadGraph *g, NetData *xd, gboolean first_call)
{
	enum {
		NET_IN		= 0,
		NET_OUT		= 1,
		NET_LOCAL	= 2,

		NET_MAX		= 3
	};

	static GHashTable *table = NULL;

	char *buf = NULL;
	size_t n = 0;
	uint i,j;
	ulong valid_ifaces_len=0;
	FILE *f_net;

	guint64 present[NET_MAX] = { 0, 0, 0 };
	gint64 delta[NET_MAX];
	gint64 total = 0;

	if_data d;
	if_data *d_ptr;

	GArray *valid_ifaces = g_array_sized_new(TRUE, FALSE, sizeof(if_data), 10);

	if (table == NULL) {
		table = g_hash_table_new (g_str_hash, g_str_equal);
	}

	xd->ifaces[0] = 0;

	f_net = info_file_required_fopen(PATH_NET_DEV, "r");
	while (getline(&buf, &n, f_net) >= 0) {
		// skip header lines of /proc/net/dev
		if (strchr(buf, ':') == NULL)
			continue;

		if (3 != sscanf(buf, "%s %"G_GUINT64_FORMAT" %*u %*u %*u %*u %*u %*u %*u %"G_GUINT64_FORMAT, d.name, &d.rx_bytes, &d.tx_bytes))
			continue; // bad data
		d.name[strlen(d.name)-1] = '\0'; // remove trailing colon

		// lookup existing data and create it if necessary
		d_ptr = (if_data*)g_hash_table_lookup(table, d.name);
		if (d_ptr == NULL) {
			d_ptr = g_new(if_data,1);
			strcpy(d_ptr->name, d.name);

			sprintf(d_ptr->path_address, "/sys/class/net/%s/address", d_ptr->name);
			sprintf(d_ptr->path_flags, "/sys/class/net/%s/flags", d_ptr->name);
			sprintf(d_ptr->path_ifindex, "/sys/class/net/%s/ifindex", d_ptr->name);
			sprintf(d_ptr->path_carrier, "/sys/class/net/%s/carrier", d_ptr->name);
			if (
				!info_file_exists(d_ptr->path_address) ||
				!info_file_exists(d_ptr->path_flags) ||
				!info_file_exists(d_ptr->path_ifindex) ||
				!info_file_exists(d_ptr->path_carrier)
			) {
				g_free (d_ptr);
				continue;
			}

			g_hash_table_insert(table, d_ptr->name, d_ptr);
		}

		d_ptr->rx_bytes = d.rx_bytes;
		d_ptr->tx_bytes = d.tx_bytes;

		if (!info_file_read_hex64(d_ptr->path_flags, &d_ptr->flags)) {
			continue;
		}

		if (!info_file_read_string_s(d_ptr->path_address, d_ptr->address, sizeof(d_ptr->address), NULL)) {
			continue;
		}

		if (!info_file_read_uint64(d_ptr->path_ifindex, &d_ptr->ifindex)) {
			continue;
		}

		// Apparently the 'carrier' file is not always readable...
		// https://unix.stackexchange.com/a/252009/26139
		if (!info_file_read_uint64(d_ptr->path_carrier, &d_ptr->carrier)) {
			d_ptr->carrier = 0;
		}

		if (!(d_ptr->flags & IFF_UP)) {
			continue; // device is down, ignore
		}

		// all OK - add interface to valid list
		g_array_append_val(valid_ifaces, *d_ptr);
		valid_ifaces_len++;
	}
	g_free(buf);
	fclose(f_net);

	// sort array by ifindex (so we can take first device when they are same address)
	g_array_sort(valid_ifaces, sort_if_data_by_ifindex);

	xd->connected = FALSE;

	for (i=0; i<valid_ifaces_len; i++) {
		d_ptr = &g_array_index(valid_ifaces, if_data, i);
		if (d_ptr == NULL)
			break;

		gboolean ignore = FALSE;

		// find devices with same HW address (e.g. ifaces put in monitor mode from airmon-ng)
		for (j=0; j<i; j++) {
			if_data *d_tmp = &g_array_index(valid_ifaces, if_data, j);
			if (strcmp(d_tmp->address, d_ptr->address) == 0) {
				g_debug("[graph-net] Ignored interface '%s' because has the same HW address of '%s' (%s)", d_tmp->name, d_ptr->name, d_ptr->address);
				ignore = TRUE;
				break;
			}
		}

		if (ignore == FALSE && g->config->filter_enable) {
			MultiloadFilter *filter = multiload_filter_new_from_existing(g->config->filter);
			for (j=0, ignore=TRUE; j<multiload_filter_get_length(filter); j++) {
				if (strcmp(multiload_filter_get_element_data(filter,j), d_ptr->name) == 0) {
					ignore = FALSE;
					break;
				}
			}

			if (ignore)
				g_debug("[graph-net] Ignored interface '%s' due to user filter", d_ptr->name);

			multiload_filter_free(filter);
		}

		if (ignore)
			continue;

		if (d_ptr->flags & IFF_LOOPBACK) {
			present[NET_LOCAL] += d_ptr->rx_bytes;
		} else {
			present[NET_IN] += d_ptr->rx_bytes;
			present[NET_OUT] += d_ptr->tx_bytes;
			if (d_ptr->carrier > 0) {
				xd->connected = TRUE;
			}
		}

		g_strlcat (xd->ifaces, d_ptr->name, sizeof(xd->ifaces));
		g_strlcat (xd->ifaces, ", ", sizeof(xd->ifaces));
	}

	g_array_free(valid_ifaces, TRUE);
	xd->ifaces[strlen(xd->ifaces)-2] = 0;


	if (G_UNLIKELY(first_call || g->filter_changed)) { // avoid initial spike
		xd->in_speed = 0;
		xd->out_speed = 0;
		xd->local_speed = 0;

		g->filter_changed = FALSE;

		memset(data, 0, NET_MAX * sizeof data[0]);
	} else {
		for (i = 0; i < NET_MAX; i++) {
			delta[i] = present[i] - xd->last[i];
			if (delta[i] < 0) {
				const char *traffic_names[NET_MAX] = {"input", "output", "local"};
				g_debug("[graph-net] Measured negative delta for %s traffic. This is a bug, but it is harmless.", traffic_names[i]);
				continue;
			}
			total += delta[i];
		}

		int max = autoscaler_get_max(&xd->scaler, g, total);

		xd->in_speed	= calculate_speed(delta[NET_IN], 	g->config->interval);
		xd->out_speed	= calculate_speed(delta[NET_OUT],	g->config->interval);
		xd->local_speed	= calculate_speed(delta[NET_LOCAL],	g->config->interval);

		if (max == 0) {
			memset(data, 0, 4*sizeof(data[0]));
		} else {
			for (i=0; i<NET_MAX; i++)
				data[i] = rint (Maximum * (float)delta[i] / max);
		}
	}

	memcpy(xd->last, present, sizeof xd->last);
}


void
multiload_graph_net_inline_output (LoadGraph *g, NetData *xd)
{
	gchar *net_read = format_size_for_display_short(xd->in_speed, g->multiload->size_format_iec);
	gchar *net_write = format_size_for_display_short(xd->out_speed, g->multiload->size_format_iec);
	if (xd->connected) {
		g_strlcpy(g->output_str[0], net_read, sizeof(g->output_str[0]));
		g_strlcpy(g->output_str[1], net_write, sizeof(g->output_str[1]));
		if (strcmp(g->output_str[0], "0b") == 0) {
			g->output_str[0][1] = 'd';
		}
		if (strcmp(g->output_str[1], "0b") == 0) {
			g->output_str[1][1] = 'u';
		}
	} else {
		// Strip suffix (should always be 1 char)
		net_read[strlen(net_read) - 1] = '\0';
		net_write[strlen(net_write) - 1] = '\0';
		// Show read/write together (almost always 0/0)
		g_snprintf(g->output_str[0], sizeof(g->output_str[0]), "%s/%s", net_read, net_write);
		g_snprintf(g->output_str[1], sizeof(g->output_str[1]), "NC");
	}

	g_free(net_read);
	g_free(net_write);
}


void
multiload_graph_net_cmdline_output (LoadGraph *g, NetData *xd)
{
	g_snprintf(g->output_str[0], sizeof(g->output_str[0]), "%"G_GUINT64_FORMAT, xd->in_speed);
	g_snprintf(g->output_str[1], sizeof(g->output_str[1]), "%"G_GUINT64_FORMAT, xd->out_speed);
	g_snprintf(g->output_str[2], sizeof(g->output_str[2]), "%"G_GUINT64_FORMAT, xd->local_speed);
}


void
multiload_graph_net_tooltip_update (char *buf_title, size_t len_title, char *buf_text, size_t len_text, LoadGraph *g, NetData *xd, gint style)
{
	gchar *tx_in = format_rate_for_display(xd->in_speed, g->multiload->size_format_iec);
	gchar *tx_out = format_rate_for_display(xd->out_speed, g->multiload->size_format_iec);
	gchar *tx_local = format_rate_for_display(xd->local_speed, g->multiload->size_format_iec);

	if (style == MULTILOAD_TOOLTIP_STYLE_DETAILED) {
		g_snprintf(buf_text, len_text, _(	"Monitored interfaces: %s\n"
											"\n"
											"Receiving: %s\n"
											"Sending: %s\n"
											"Local: %s"),
											xd->ifaces, tx_in, tx_out, tx_local);
	} else {
		g_snprintf(buf_text, len_text, "\xe2\xac\x87%s \xe2\xac\x86%s", tx_in, tx_out);
	}

	g_free(tx_in);
	g_free(tx_out);
	g_free(tx_local);
}
