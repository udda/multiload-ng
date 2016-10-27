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
#include <ctype.h>
#include <mntent.h>
#include <stdlib.h>

#include "graph-data.h"
#include "autoscaler.h"
#include "preferences.h"
#include "util.h"


#define CRITICAL_LEVEL 4

void
multiload_graph_bat_get_data (int Maximum, int data [2], LoadGraph *g, BatteryData *xd)
{
	static const char *root_node = "/sys/class/power_supply";

	FILE *f;
	DIR *dir;
	struct dirent *dirent;

	gboolean found = FALSE;
	gboolean bat_charging;
	gboolean bat_critical;
	glong bat_capacity;

	gchar buf[PATH_MAX];
	gchar buf2[PATH_MAX];
	gchar capacity_path[PATH_MAX];
	gchar capacity_level_path[PATH_MAX];
	gchar status_path[PATH_MAX];
	gchar manufacturer_path[PATH_MAX];
	gchar model_name_path[PATH_MAX];
	size_t s;

	(void)s; // shuts off warning by '-Wunused-but-set-variable'

	// check if /sys node exists, otherwise means no battery support or no battery at all
	dir = opendir(root_node);
	if (dir == NULL)
		return;

	while ((dirent = readdir(dir)) != NULL) {

		// check type
		g_snprintf(buf, PATH_MAX, "%s/%s/type", root_node, dirent->d_name);
		if (!g_file_test(buf, G_FILE_TEST_EXISTS))
			continue;
		f = cached_fopen_r(buf, FALSE);
		if (!file_check_contents(f, "Battery"))
			continue;

		// check capacity
		g_snprintf(capacity_path, PATH_MAX, "%s/%s/capacity", root_node, dirent->d_name);
		if (!g_file_test(capacity_path, G_FILE_TEST_EXISTS))
			continue;

		// check present
		g_snprintf(buf, PATH_MAX, "%s/%s/present", root_node, dirent->d_name);
		if (g_file_test(buf, G_FILE_TEST_EXISTS)) {
			f = cached_fopen_r(buf, FALSE);
			if (!file_check_contents(f, "1"))
				continue;
		}

		g_snprintf(capacity_level_path, PATH_MAX, "%s/%s/capacity_level", root_node, dirent->d_name);
		g_snprintf(status_path, PATH_MAX, "%s/%s/status", root_node, dirent->d_name);
		g_snprintf(manufacturer_path, PATH_MAX, "%s/%s/manufacturer", root_node, dirent->d_name);
		g_snprintf(model_name_path, PATH_MAX, "%s/%s/model_name", root_node, dirent->d_name);

		found = TRUE;
		break;
	}
	closedir(dir);

	if (!found)
		return;

	// status
	if (g_file_test(status_path, G_FILE_TEST_EXISTS)) {
		f = cached_fopen_r(status_path, TRUE);
		bat_charging = !file_check_contents(f, "Discharging");
	} else
		bat_charging = FALSE;

	// capacity
	f = cached_fopen_r(capacity_path, FALSE);
	if (f == NULL || fscanf(f, "%ld", &bat_capacity) != 1)
		return;

	// capacity level
	if (g_file_test(capacity_level_path, G_FILE_TEST_EXISTS)) {
		f = cached_fopen_r(capacity_level_path, TRUE);
		bat_critical = file_check_contents(f, "Critical");
	} else
		bat_critical = bat_capacity <= CRITICAL_LEVEL;

	xd->percent = bat_capacity;
	read_string_from_file(manufacturer_path, buf, sizeof(buf));
	read_string_from_file(model_name_path, buf2, sizeof(buf2));
	g_snprintf(xd->battery_name, sizeof(xd->battery_name), "%s %s", buf, buf2);


	if (bat_charging) {
		data[0] = rint (Maximum * bat_capacity / 100);
		data[1] = 0;
		data[2] = 0;
	} else if (!bat_critical) {
		data[0] = 0;
		data[1] = rint (Maximum * bat_capacity / 100);
		data[2] = 0;
	} else {
		data[0] = 0;
		data[1] = 0;
		data[2] = rint (Maximum * bat_capacity / 100);
	}
}


void
multiload_graph_bat_cmdline_output (LoadGraph *g, BatteryData *xd)
{
	g_snprintf(g->output_str[0], sizeof(g->output_str[0]), "%d", xd->percent);
}


void
multiload_graph_bat_tooltip_update (char **title, char **text, LoadGraph *g, BatteryData *xd)
{
	if (g->config->tooltip_style == MULTILOAD_TOOLTIP_STYLE_DETAILED) {
		*title = g_strdup(xd->battery_name);
		*text = g_strdup_printf(_(	"Capacity: %d%%"),
									xd->percent);
	} else {
		*text=g_strdup_printf("%d%%", xd->percent);
	}
}
