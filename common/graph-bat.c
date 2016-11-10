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
#include <dirent.h>
#include <stdlib.h>

#include "graph-data.h"
#include "autoscaler.h"
#include "info-file.h"
#include "preferences.h"
#include "util.h"


#define CRITICAL_LEVEL 4
#define PATH_POWER_SUPPLY "/sys/class/power_supply"

static gboolean battery_found = FALSE;
static gchar path_battery_root[PATH_MAX];
static gchar path_battery_capacity[PATH_MAX];
static gchar path_battery_status[PATH_MAX];
static gchar path_battery_capacity_level[PATH_MAX];

void
multiload_graph_bat_init (LoadGraph *g, BatteryData *xd)
{
	gchar buf[PATH_MAX];
	struct dirent *dirent;

	// check if /sys node exists, otherwise means no battery support or no battery at all
	DIR *dir = opendir(PATH_POWER_SUPPLY);
	if (dir == NULL)
		return;

	while ((dirent = readdir(dir)) != NULL) {
		g_snprintf(path_battery_root, PATH_MAX, "%s/%s", PATH_POWER_SUPPLY, dirent->d_name);

		// check type
		g_snprintf(buf, PATH_MAX, "%s/type", path_battery_root);
		if (!info_file_exists(buf) || !info_file_has_contents(buf, "Battery"))
			continue;

		// check capacity
		g_snprintf(path_battery_capacity, PATH_MAX, "%s/capacity", path_battery_root);
		if (!info_file_exists(path_battery_capacity))
			continue;

		// check present
		g_snprintf(buf, PATH_MAX, "%s/present", path_battery_root);
		if (info_file_exists(buf) && !info_file_has_contents(buf, "1"))
			continue;


		// Battery found! Set base paths
		g_snprintf(path_battery_status, PATH_MAX, "%s/status", path_battery_root);
		g_snprintf(path_battery_capacity_level, PATH_MAX, "%s/capacity_level", path_battery_root);


		// set battery name
		gchar manufacturer[PATH_MAX];
		gchar model_name[PATH_MAX];

		g_snprintf(buf, PATH_MAX, "%s/manufacturer", path_battery_root);
		if (!info_file_read_string_s (buf, manufacturer, sizeof(manufacturer), NULL))
			manufacturer[0] = '\0';

		g_snprintf(buf, PATH_MAX, "%s/model_name", path_battery_root);
		if (!info_file_read_string_s (buf, model_name, sizeof(model_name), NULL))
			model_name[0] = '\0';

		g_snprintf(xd->battery_name, sizeof(xd->battery_name), "%s %s", manufacturer, model_name);

		battery_found = TRUE;

		break;
	}
	closedir(dir);
}

void
multiload_graph_bat_get_data (int Maximum, int data [3], LoadGraph *g, BatteryData *xd, gboolean first_call)
{
	if (!battery_found)
		return;

	// status
	if (info_file_exists(path_battery_status)) {
		xd->is_charging = !info_file_has_contents(path_battery_status, "Discharging");
	} else {
		xd->is_charging = FALSE;
	}

	// capacity
	gint64 capacity;
	if (!info_file_read_int64 (path_battery_capacity, &capacity))
		return;
	xd->percent = (int)capacity;

	// capacity level
	if (info_file_exists(path_battery_capacity_level)) {
		xd->is_critical = info_file_has_contents(path_battery_capacity_level, "Critical");
	} else {
		xd->is_critical = (xd->percent <= CRITICAL_LEVEL);
	}

	if (xd->is_charging) {
		data[0] = rint (Maximum * xd->percent / 100);
		data[1] = 0;
		data[2] = 0;
	} else if (!xd->is_critical) {
		data[0] = 0;
		data[1] = rint (Maximum * xd->percent / 100);
		data[2] = 0;
	} else {
		data[0] = 0;
		data[1] = 0;
		data[2] = rint (Maximum * xd->percent / 100);
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
		gchar *capacity = g_strdup_printf(_("Capacity: %d%%"), xd->percent);
		gchar *status = (xd->is_charging? _("Charging") : _("Discharging"));

		*title = g_strdup(xd->battery_name);
		if (xd->is_critical)
			*text = g_strdup_printf("%s (%s)\n%s", capacity, _("Critical level"), status);
		else
			*text = g_strdup_printf("%s\n%s", capacity, status);

		g_free(capacity);
	} else {
		*text=g_strdup_printf("%d%%", xd->percent);
	}
}
