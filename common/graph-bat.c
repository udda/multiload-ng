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

void
multiload_graph_bat_get_data (int Maximum, int data [2], LoadGraph *g, BatteryData *xd)
{
	static const char *root_node = "/sys/class/power_supply";
	static gboolean first_call = TRUE;
	static gchar battery_path[PATH_MAX];

	gchar buf[PATH_MAX];
	gchar status_path[PATH_MAX];
	gchar capacity_path[PATH_MAX];
	gchar capacity_level_path[PATH_MAX];
	size_t s;

	(void)s; // shuts off warning by '-Wunused-but-set-variable'

	if (G_UNLIKELY(first_call)) {
		first_call = FALSE;
		battery_path[0] = '\0';

		// check if /sys node exists, otherwise means no battery support or no battery at all
		struct dirent *dirent;
		DIR *dir = opendir(root_node);
		if (dir == NULL)
			return;

		while ((dirent = readdir(dir)) != NULL) {
			// check type
			g_snprintf(buf, PATH_MAX, "%s/%s/type", root_node, dirent->d_name);
			if (!info_file_exists(buf) || !info_file_has_contents(buf, "Battery"))
				continue;

			// check capacity
			g_snprintf(capacity_path, PATH_MAX, "%s/%s/capacity", root_node, dirent->d_name);
			if (!info_file_exists(capacity_path))
				continue;

			// check present
			g_snprintf(buf, PATH_MAX, "%s/%s/present", root_node, dirent->d_name);
			if (info_file_exists(buf) && !info_file_has_contents(buf, "1"))
				continue;


			// Battery found! Set base paths
			g_snprintf(battery_path, PATH_MAX, "%s/%s", root_node, dirent->d_name);
			g_snprintf(status_path, PATH_MAX, "%s/status", battery_path);
			g_snprintf(capacity_level_path, PATH_MAX, "%s/capacity_level", battery_path);

			// set battery name
			gchar manufacturer[PATH_MAX];
			gchar model_name[PATH_MAX];

			g_snprintf(buf, PATH_MAX, "%s/manufacturer", battery_path);
			if (!info_file_read_string_s (buf, manufacturer, sizeof(manufacturer), NULL))
				manufacturer[0] = '\0';

			g_snprintf(buf, PATH_MAX, "%s/model_name", battery_path);
			if (!info_file_read_string_s (buf, model_name, sizeof(model_name), NULL))
				model_name[0] = '\0';

			g_snprintf(xd->battery_name, sizeof(xd->battery_name), "%s %s", manufacturer, model_name);

			break;
		}
		closedir(dir);
	}

	if (battery_path[0] == '\0')
		return;

	// status
	if (info_file_exists(status_path)) {
		xd->is_charging = !info_file_has_contents(status_path, "Discharging");
	} else {
		xd->is_charging = FALSE;
	}

	// capacity
	if (!info_file_read_int64 (capacity_path, (gint64*)&xd->percent))
		return;

	// capacity level
	if (info_file_exists(capacity_level_path)) {
		xd->is_critical = info_file_has_contents(capacity_level_path, "Critical");
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
