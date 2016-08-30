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
#include <glibtop/swap.h>

#include "graph-data.h"

void
multiload_graph_swap_get_data (int Maximum, int data [1], LoadGraph *g)
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

