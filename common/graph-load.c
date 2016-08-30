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
#include <glibtop/loadavg.h>

#include "graph-data.h"
#include "preferences.h"
#include "util.h"

#define PER_CPU_MAX_LOADAVG 3


void
multiload_graph_load_get_data (int Maximum, int data [1], LoadGraph *g, LoadData *xd)
{
	glibtop_loadavg loadavg;
	static float max = 0;
	float current;

	static const guint64 needed_flags =
		(1 << GLIBTOP_LOADAVG_LOADAVG);

	glibtop_get_loadavg (&loadavg);
	g_return_if_fail ((loadavg.flags & needed_flags) == needed_flags);

	if (max == 0)
		max = PER_CPU_MAX_LOADAVG * (1 + glibtop_global_server->ncpu);
	current = MIN(loadavg.loadavg[0], max);

	memcpy(xd->loadavg, loadavg.loadavg, sizeof loadavg.loadavg);

	data [0] = rint ((float) Maximum * current / max);
}

void
multiload_graph_load_tooltip_update (char **title, char **text, LoadGraph *g, LoadData *xd)
{
	if (g->config->tooltip_style == TOOLTIP_STYLE_DETAILS) {
		*text = g_strdup_printf(_(	"Last minute: %0.02f\n"
									"Last 5 minutes: %0.02f\n"
									"Last 15 minutes: %0.02f"),
									xd->loadavg[0], xd->loadavg[1], xd->loadavg[2]);
	} else {
		*text = g_strdup_printf("%0.02f", xd->loadavg[0]);
	}
}
