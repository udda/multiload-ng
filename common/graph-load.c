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
#include <stdlib.h>

#include "graph-data.h"
#include "preferences.h"
#include "util.h"


void
multiload_graph_load_get_data (int Maximum, int data [1], LoadGraph *g, LoadData *xd)
{
	char *savelocale;

	FILE *f = cached_fopen_r("/proc/loadavg", TRUE);

	// some countries use commas instead of points for floats
	savelocale = strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");

	g_assert(5 == fscanf(f, "%f %f %f %d/%d", &xd->loadavg_1, &xd->loadavg_5, &xd->loadavg_15, &xd->proc_active, &xd->proc_count));

	// restore default locale for numbers
	setlocale(LC_NUMERIC, savelocale);
	free(savelocale);


	int max = autoscaler_get_max(&xd->scaler, g, rint(xd->loadavg_1));
	data [0] = rint ((float) Maximum * xd->loadavg_1 / max);
}

void
multiload_graph_load_tooltip_update (char **title, char **text, LoadGraph *g, LoadData *xd)
{
	if (g->config->tooltip_style == TOOLTIP_STYLE_DETAILS) {
		*text = g_strdup_printf(_(	"Last minute: %0.02f\n"
									"Last 5 minutes: %0.02f\n"
									"Last 15 minutes: %0.02f\n"
									"Processes/threads: %d active out of %d."),
									xd->loadavg_1, xd->loadavg_5, xd->loadavg_15,
									xd->proc_active, xd->proc_count);
	} else {
		*text = g_strdup_printf("%0.02f", xd->loadavg_1);
	}
}
