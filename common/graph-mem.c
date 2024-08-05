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
#include "info-file.h"
#include "preferences.h"
#include "util.h"

#define PATH_MEMINFO "/proc/meminfo"

void
multiload_graph_mem_get_data (int Maximum, int data [5], LoadGraph *g, MemoryData *xd, gboolean first_call)
{
	// displayed keys
	static guint64 kb_main_total = 0;
	static guint64 kb_main_shared = 0;
	static guint64 kb_main_buffers = 0;
	static guint64 kb_main_cached = 0;
	static guint64 kb_main_used = 0;

	// auxiliary keys
	static guint64 kb_main_free = 0;
	static guint64 kb_page_cache = 0;
	static guint64 kb_reclaimable = 0;

	static const InfoFileMappingEntry table[] = {
		{ "MemTotal",		'u',	&kb_main_total },
		{ "MemFree",		'u',	&kb_main_free},
		{ "Buffers",		'u',	&kb_main_buffers },
		{ "Shmem:",			'u',	&kb_main_shared},
		{ "Cached",			'u',	&kb_page_cache },
		{ "SReclaimable",	'u',	&kb_reclaimable }
	};

	gint r = info_file_read_keys (PATH_MEMINFO, table, 6);
	g_assert_cmpint(r, ==, 6);

	// https://stackoverflow.com/a/41251290
	// https://github.com/htop-dev/htop/blob/3.3.0/linux/LinuxMachine.c#L199-L218
	kb_main_cached = kb_page_cache + kb_reclaimable - kb_main_shared;

	kb_main_used = kb_main_total - kb_main_free - kb_page_cache - kb_main_buffers - kb_reclaimable;
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
	g_snprintf(g->output_str[0], sizeof(g->output_str[0]), "%"G_GUINT64_FORMAT, xd->user);
	g_snprintf(g->output_str[1], sizeof(g->output_str[1]), "%"G_GUINT64_FORMAT, xd->shared);
	g_snprintf(g->output_str[2], sizeof(g->output_str[2]), "%"G_GUINT64_FORMAT, xd->buffers);
	g_snprintf(g->output_str[3], sizeof(g->output_str[3]), "%"G_GUINT64_FORMAT, xd->cache);
}

void
multiload_graph_mem_tooltip_update (char *buf_title, size_t len_title, char *buf_text, size_t len_text, LoadGraph *g, MemoryData *xd, gint style)
{
	if (style == MULTILOAD_TOOLTIP_STYLE_DETAILED) {
		gchar *total = format_size_for_display(xd->total, g->multiload->size_format_iec);

		gchar *user = format_size_for_display(xd->user, g->multiload->size_format_iec);
		gchar *user_percent = format_percent(xd->user, xd->total, 1);

		gchar *shared = format_size_for_display(xd->shared, g->multiload->size_format_iec);
		gchar *shared_percent = format_percent(xd->shared, xd->total, 1);

		gchar *buffers = format_size_for_display(xd->buffers, g->multiload->size_format_iec);
		gchar *buffers_percent = format_percent(xd->buffers, xd->total, 1);

		gchar *cache = format_size_for_display(xd->cache, g->multiload->size_format_iec);
		gchar *cache_percent = format_percent(xd->cache, xd->total, 1);

		g_snprintf(buf_title, len_title, _("%s of RAM"), total);
		g_snprintf(buf_text, len_text, _(	"%s (%s) used by programs\n"
											"%s (%s) shared among programs/tmpfs\n"
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
		gchar *use = format_percent(xd->user, xd->total, 0);
		g_snprintf(buf_text, len_text, "%s", use);
		g_free(use);
	}
}
