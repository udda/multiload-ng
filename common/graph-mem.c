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
#include <glibtop/mem.h>

#include "graph-data.h"

void
multiload_graph_mem_get_data (int Maximum, int data [4], LoadGraph *g)
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