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
#include <errno.h>
#include <math.h>
#include <stdlib.h>

#include "graph-data.h"
#include "preferences.h"
#include "util.h"

typedef struct {
	char key[20];
	long long unsigned *address;
} meminfo_mapping_table;

void
multiload_graph_mem_get_data (int Maximum, int data [4], LoadGraph *g, MemoryData *xd)
{
	// displayed keys
	static long long unsigned kb_main_total = 0;
	static long long unsigned kb_main_buffers = 0;
	static long long unsigned kb_main_shared = 0;
	static long long unsigned kb_main_cached = 0;
	static long long unsigned kb_main_used = 0;

	// auxiliary keys
	static long long unsigned kb_main_available = 0;
	static long long unsigned kb_main_free = 0;
	static long long unsigned kb_page_cache = 0;
	static long long unsigned kb_slab = 0;

	const static meminfo_mapping_table table[] = {
		{ "MemTotal",		&kb_main_total },
		{ "MemAvailable",	&kb_main_available},
		{ "MemFree",		&kb_main_free},
		{ "Buffers",		&kb_main_buffers },
		{ "Shmem",			&kb_main_shared },
		{ "Cached",			&kb_page_cache },
		{ "Slab",			&kb_slab },
		{ "",				NULL }
	};

	char *buf = NULL;
	char *tmp;
	size_t n = 0;
	size_t len;
	guint i;

	FILE *f = cached_fopen_r("/proc/meminfo", FALSE);
	while(getline(&buf, &n, f) >= 0) {
		for (i=0; table[i].address != NULL; i++) {
			len = strlen(table[i].key);
			if (strncmp(buf, table[i].key, len) == 0) {
				// tmp will start with numeric value
				for (tmp = buf+len+1; isspace(tmp[0]); tmp++);

				errno = 0;
				*(table[i].address) = g_ascii_strtoull(tmp, NULL, 0);
				if (errno != 0)
					g_warning("[graph-mem] Parsing of key %s failed", table[i].key);

				break;
			}
		}
	}
	free(buf);

	kb_main_cached = kb_page_cache + kb_slab;

	kb_main_used = kb_main_total - kb_main_free - kb_main_cached - kb_main_buffers;
	if (kb_main_used < 0)
		kb_main_used = kb_main_total - kb_main_free;

	xd->user = kb_main_used * 1024;
	xd->shared = kb_main_shared * 1024;
	xd->buffers = kb_main_buffers * 1024;
	xd->cache = kb_main_cached * 1024;
	xd->total = kb_main_total * 1024;

	data [0] = rint (Maximum * (float)kb_main_used   / (float)kb_main_total);
	data [1] = rint (Maximum * (float)kb_main_shared / (float)kb_main_total);
	data [2] = rint (Maximum * (float)kb_main_buffers / (float)kb_main_total);
	data [3] = rint (Maximum * (float)kb_main_cached / (float)kb_main_total);
}


void
multiload_graph_mem_cmdline_output (LoadGraph *g, MemoryData *xd)
{
	g_snprintf(g->output_str[0], sizeof(g->output_str[0]), "%llu", xd->user);
	g_snprintf(g->output_str[1], sizeof(g->output_str[1]), "%llu", xd->shared);
	g_snprintf(g->output_str[2], sizeof(g->output_str[2]), "%llu", xd->buffers);
	g_snprintf(g->output_str[3], sizeof(g->output_str[3]), "%llu", xd->cache);
}

void
multiload_graph_mem_tooltip_update (char **title, char **text, LoadGraph *g, MemoryData *xd)
{
	if (g->config->tooltip_style == MULTILOAD_TOOLTIP_STYLE_DETAILED) {
		gchar *total = g_format_size_full(xd->total, G_FORMAT_SIZE_IEC_UNITS);

		gchar *user = g_format_size_full(xd->user, G_FORMAT_SIZE_IEC_UNITS);
		gchar *user_percent = format_percent(xd->user, xd->total, 1);

		gchar *shared = g_format_size_full(xd->shared, G_FORMAT_SIZE_IEC_UNITS);
		gchar *shared_percent = format_percent(xd->shared, xd->total, 1);

		gchar *buffers = g_format_size_full(xd->buffers, G_FORMAT_SIZE_IEC_UNITS);
		gchar *buffers_percent = format_percent(xd->buffers, xd->total, 1);

		gchar *cache = g_format_size_full(xd->cache, G_FORMAT_SIZE_IEC_UNITS);
		gchar *cache_percent = format_percent(xd->cache, xd->total, 1);

		*title = g_strdup_printf(_("%s of RAM"), total);
		*text = g_strdup_printf(_(	"%s (%s) used by programs\n"
									"%s (%s) used by shared memory\n"
									"%s (%s) used for buffers\n"
									"%s (%s) used as cache"),
									user_percent, user,
									shared_percent, shared,
									buffers_percent, buffers,
									cache_percent, cache);
		g_free(total);
		g_free(user);
		g_free(user_percent);
		g_free(shared);
		g_free(shared_percent);
		g_free(buffers);
		g_free(buffers_percent);
		g_free(cache);
		g_free(cache_percent);
	} else {
		gchar *use = format_percent(xd->user+xd->cache, xd->total, 0);
		*text = g_strdup_printf("%s", use);
		g_free(use);
	}
}
