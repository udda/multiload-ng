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

//TODO remove this
#define MAX_LOADAVG 10

void
multiload_graph_load_get_data (int Maximum, int data [1], LoadGraph *g, LoadData *xd)
{
	float loadavg_1, loadavg_5, loadavg_15;
	guint proc_active, proc_count;

	int result;
	char *savelocale;

	FILE *f = fopen("/proc/loadavg", "r");
	if (f != NULL) {
		// some countries use commas instead of points for floats
		savelocale = strdup(setlocale(LC_NUMERIC, NULL));
		setlocale(LC_NUMERIC, "C");

		result = fscanf(f, "%f %f %f %d/%d", &loadavg_1, &loadavg_5, &loadavg_15, &proc_active, &proc_count);

		// restore default locale for numbers
		setlocale(LC_NUMERIC, savelocale);
		free(savelocale);

		fclose(f);
	}

//TODO autoscaler
//TODO process list

	static float max = 0;
	float current;

	if (max == 0)
		max = MAX_LOADAVG;
	current = MIN(loadavg_1, max);

	xd->loadavg_1 = loadavg_1;
	xd->loadavg_5 = loadavg_5;
	xd->loadavg_15 = loadavg_15;

//	xd->process_active = proc_active;
//	xd->process_count = proc_count;

	data [0] = rint ((float) Maximum * current / max);
}

void
multiload_graph_load_tooltip_update (char **title, char **text, LoadGraph *g, LoadData *xd)
{
	if (g->config->tooltip_style == TOOLTIP_STYLE_DETAILS) {
		*text = g_strdup_printf(_(	"Last minute: %0.02f\n"
									"Last 5 minutes: %0.02f\n"
									"Last 15 minutes: %0.02f"),
									xd->loadavg_1, xd->loadavg_5, xd->loadavg_15);
	} else {
		*text = g_strdup_printf("%0.02f", xd->loadavg_1);
	}
}
