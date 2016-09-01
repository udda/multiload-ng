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

#include <dirent.h>
#include <math.h>
#include <string.h>

#include "graph-data.h"
#include "autoscaler.h"
#include "preferences.h"
#include "util.h"


void
multiload_graph_temp_get_data (int Maximum, int data[2], LoadGraph *g, TemperatureData *xd)
{
	guint temp = 0;
	guint i, j, t;

	DIR *dir;
	struct dirent *entry;

	static gboolean first_call = TRUE;
	static gboolean support = FALSE;

	// hold path and max temp for each thermal zone, filled on first call
	static guint n_zones = 0;
	static gchar **paths = NULL;
	static guint *maxtemps = NULL;

	// handle errors by providing empty data if something goes wrong
	memset(data, 0, 2 * sizeof data[0]);

	if (G_UNLIKELY(first_call)) {
		first_call = FALSE;

		gchar *d_base = g_strdup("/sys/class/thermal");

		// check if /sys path exists
		dir = opendir(d_base);
		if (!dir)
			return;

		// count thermal_zoneX dirs
		while ((entry = readdir(dir)) != NULL) {
			if (strncmp(entry->d_name, "thermal_zone", 12) == 0)
				n_zones++;
		}

		// if there is at least one thermal zone, we can proceed
		if (n_zones > 0)
			support = TRUE;

		// allocate buffers
		paths    = (gchar**) malloc( n_zones * sizeof (gchar*) );
		maxtemps = g_new0(guint, n_zones);

		// fill buffers
		i=0;
		rewinddir(dir);
		while ((entry = readdir(dir)) != NULL) {
			if (strncmp(entry->d_name, "thermal_zone", 12) != 0)
				continue;

			gchar *d_thermal = g_strdup_printf("%s/%s", d_base, entry->d_name);

			// find "critical" (max) temperature searching in trip points
			for (j=0; ; j++) {
				gchar *d_type = g_strdup_printf("%s/trip_point_%d_type", d_thermal, j);
				FILE *f_type = fopen(d_type, "r");
				g_free(d_type);

				if (!f_type)
					break; //no more trip point files, stop searching
				gboolean found = file_check_contents(f_type, "critical");
				fclose(f_type);

				if (found) { // found critical temp
					gchar *d_temp = g_strdup_printf("%s/trip_point_%d_temp", d_thermal, j);
					t = read_int_from_file(d_temp);
					g_free(d_temp);
					if (t > maxtemps[i])
						maxtemps[i] = t;
				}
			}
			paths[i] = g_strdup_printf("%s/temp", d_thermal);
			i++;
			g_free(d_thermal);
		}
		closedir(dir);
		g_free(d_base);
	}

	// check if we have sysfs thermal support
	if (!support)
		return;

	// finds max temperature and its index (to use the respective maximum)
	for (i=0,j=0; i<n_zones; i++) {
		t = read_int_from_file(paths[i]);
		if (t > temp) {
			temp = t;
			j = i;
		}
	}

	int max = autoscaler_get_max(&xd->scaler, g, temp);

	if (maxtemps[j] > 0 && maxtemps[j] < temp) {
		data[0] = rint (Maximum * (float)(maxtemps[j]) / max);
		data[1] = rint (Maximum * (float)(temp-maxtemps[j]) / max);
	} else {
		data[0] = rint (Maximum * (float)temp / max);
		data[1] = 0;
	}

	xd->value = temp;
	xd->max = maxtemps[j];
}

void
multiload_graph_temp_tooltip_update (char **title, char **text, LoadGraph *g, TemperatureData *xd)
{
	if (g->config->tooltip_style == TOOLTIP_STYLE_DETAILS) {
		if (xd->max > 0)
			*text = g_strdup_printf(_(	"Current: %.1f °C\n"
										"Critical: %.1f °C"),
										(xd->value/1000.0), (xd->max/1000.0));
		else
			*text = g_strdup_printf(_(	"Current: %.1f °C"),
										(xd->value/1000.0));
	} else {
		*text = g_strdup_printf("%.1f °C", xd->value/1000.0);
	}
}
