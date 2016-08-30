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
#include "preferences.h"
#include "util.h"

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

void
multiload_graph_swap_tooltip_update (char **title, char **text, LoadGraph *g, SwapData *xd)
{
	if (xd->total == 0) {
		*text = g_strdup_printf(_("No swap"));
	} else {
		gchar *used = format_percent(xd->used, xd->total, 0);
		gchar *total = g_format_size_full(xd->total, G_FORMAT_SIZE_IEC_UNITS);

		if (g->config->tooltip_style == TOOLTIP_STYLE_DETAILS) {
			*title = g_strdup_printf(_("%s of swap"), total);
			*text = g_strdup_printf(_("%s used"), used);
		} else {
			*text = g_strdup_printf("%s", used);
		}

		g_free(used);
		g_free(total);
	}
}
