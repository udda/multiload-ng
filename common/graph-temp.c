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
#include <stdlib.h>

#include "graph-data.h"
#include "autoscaler.h"
#include "preferences.h"
#include "util.h"


typedef struct {
	char name[20];
	char node_path[PATH_MAX];
	char temp_path[PATH_MAX];

	gint64 temp;
	gint64 critical;
} TemperatureSourceData;

typedef enum {
	TEMP_SOURCE_SUPPORT_UNINITIALIZED,

	TEMP_SOURCE_SUPPORT_HWMON,
	TEMP_SOURCE_SUPPORT_ACPITZ,
	// TODO: if other sources are added, this enum should become a bitmask, as they might not be mutually exclusive

	TEMP_SOURCE_NO_SUPPORT
} TemperatureSourceSupport;

static TemperatureSourceSupport list_temp(TemperatureSourceData **list);

gchar *
multiload_graph_temp_get_filter (LoadGraph *g, TemperatureData *xd)
{
	TemperatureSourceData *list = NULL;
	gchar *filter;
	gboolean selected;
	guint i;

	if (list_temp(&list) != TEMP_SOURCE_NO_SUPPORT) {
		for (i=0; list[i].temp_path[0]!='\0'; i++) {}
		filter = g_new0(char, i*(3+sizeof(list[i].name)));
		for (i=0; list[i].temp_path[0]!='\0'; i++) {
			selected = (strcmp(list[i].name, g->config->filter) == 0);
			strcat(filter, selected?"+":"-");
			strcat(filter, list[i].name);
			strcat(filter, MULTILOAD_FILTER_SEPARATOR);
		}
		filter[strlen(filter)-1] = '\0';
	} else
		filter = g_new0(char, 1);

	g_free(list);
	return filter;
}

static gboolean
list_temp_acpitz(TemperatureSourceData **list, gboolean init)
{
	static const char *root_node = "/sys/class/thermal";

	FILE *f;
	DIR *dir;
	struct dirent *dirent;

	guint n_zones = 0;

	gchar buf[PATH_MAX];
	guint i, j;
	size_t s;

	(void)s; // shuts off warning by '-Wunused-but-set-variable'

	if (init) {
		// check if /sys node exists, otherwise means no acpitz support
		dir = opendir(root_node);
		if (dir == NULL)
			return FALSE;

		// count thermal_zone* dirs
		while ((dirent = readdir(dir)) != NULL) {
			if (strncmp(dirent->d_name, "thermal_zone", 12) == 0)
				n_zones++;
		}

		// check whether there is at least one thermal zone, otherwise means no acpitz support
		if (n_zones == 0)
			return FALSE;

		// allocate data (plus guard item to the end, with temp_path="")
		*list = g_new0(TemperatureSourceData, n_zones+1);

		// fill static data (do not read actual temperature in init phase)
		i=0;
		rewinddir(dir);
		while ((dirent = readdir(dir)) != NULL) {
			if (strncmp(dirent->d_name, "thermal_zone", 12) != 0)
				continue;

			// fill paths
			g_snprintf((*list)[i].node_path, sizeof((*list)[i].node_path), "%s/%s", root_node, dirent->d_name);
			g_snprintf((*list)[i].temp_path, sizeof((*list)[i].temp_path), "%s/temp", (*list)[i].node_path);

			// fill name from device path if present, else generate unique name
			g_snprintf(buf, PATH_MAX, "%s/device/path", (*list)[i].node_path);
			if ((f = fopen(buf, "r")) != NULL) {
				s = fscanf(f, "%s", buf);
				fclose(f);

				// remove leading path prefix
				if (strncmp(buf, "\\_TZ_.", 6) == 0)
					strncpy((*list)[i].name, buf+6, sizeof((*list)[i].name));
				else
					strncpy((*list)[i].name, buf, sizeof((*list)[i].name));
			}
			if ((*list)[i].name[0] == '\0')
				g_snprintf((*list)[i].name, sizeof((*list)[i].name), "thermal_zone%d (ACPI)", i);

			// find "critical" temperature searching in trip points
			for (j=0; ; j++) {
				g_snprintf(buf, PATH_MAX, "%s/trip_point_%d_type", (*list)[i].node_path, j);

				if ((f = fopen(buf, "r")) == NULL)
					break; //no more trip point files, stop searching

				if (file_check_contents(f, "critical")) { // found critical temp
					g_snprintf(buf, PATH_MAX, "%s/trip_point_%d_temp", (*list)[i].node_path, j);
					(*list)[i].critical = read_int_from_file(buf);
				}

				fclose(f);
			}
			i++;
		}
		closedir(dir);
		return TRUE;
	}

	// read phase - always return TRUE
	for (i=0; (*list)[i].temp_path[0] != '\0'; i++)
		(*list)[i].temp = read_int_from_file((*list)[i].temp_path);

	return TRUE;
}

static gboolean
list_temp_hwmon(TemperatureSourceData **list, gboolean init)
{
	static const char *root_node = "/sys/class/hwmon";

	FILE *f;
	DIR *dir;
	DIR *subdir;
	struct dirent *dirent;
	struct dirent *subdirent;

	guint n_zones = 0;

	char name[60];
	char buf[PATH_MAX];
	char *tmp;
	size_t s;
	guint i, n;


	(void)s; // shuts off warning by '-Wunused-but-set-variable'

	if (init) {
		// check if /sys node exists, otherwise means no hwmon support
		dir = opendir(root_node);
		if (dir == NULL)
			return FALSE;

		// count thermal_zone* dirs
		while ((dirent = readdir(dir)) != NULL) {
			if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
				continue;

			g_snprintf(buf, PATH_MAX, "%s/%s", root_node, dirent->d_name);

			subdir = opendir(buf);
			if (subdir == NULL)
				continue;

			while ((subdirent = readdir(subdir)) != NULL) {
				if (!strcmp(subdirent->d_name, ".") || !strcmp(subdirent->d_name, ".."))
					continue;
				if (g_regex_match_simple("^temp[0-9]+_input$", subdirent->d_name, 0, 0))
					n_zones++;
			}
			closedir(subdir);
		}

		// check whether there is at least one thermal zone, otherwise means no hwmon support
		if (n_zones == 0)
			return FALSE;

		// allocate data (plus guard item to the end, with temp_path="")
		*list = g_new0(TemperatureSourceData, n_zones+1);

		// fill static data (do not read actual temperature in init phase)
		i=0;
		rewinddir(dir);
		while ((dirent = readdir(dir)) != NULL) {
			if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
				continue;
			g_snprintf(buf, PATH_MAX, "%s/%s", root_node, dirent->d_name);

			subdir = opendir(buf);
			if (subdir == NULL)
				continue;

			while ((subdirent = readdir(subdir)) != NULL) {
				if (!strcmp(subdirent->d_name, ".") || !strcmp(subdirent->d_name, ".."))
					continue;
				if (g_regex_match_simple("^temp[0-9]+_input$", subdirent->d_name, 0, 0) == FALSE)
					continue;

				// get hwmon driver name
				g_snprintf(buf, PATH_MAX, "%s/%s/name", root_node, dirent->d_name);
				f  = fopen(buf, "r");
				if (f != NULL) {
					s = fscanf(f, "%s", name);
					fclose(f);
				}

				// fill name - search for temp*_label, else generate unique name
				tmp = str_replace(subdirent->d_name, "_input", "_label");
				g_snprintf(buf, PATH_MAX, "%s/%s/%s", root_node, dirent->d_name, tmp);
				g_free(tmp);
				f  = fopen(buf, "r");
				if (f != NULL) {
					// build format string to accept spaces too
					tmp = g_strdup_printf("%%%zu[0-9a-zA-Z ]", sizeof((*list)[i].name)-1);
					s = fscanf(f, tmp, buf);
					g_free(tmp);
					fclose(f);
					g_snprintf((*list)[i].name, sizeof((*list)[i].name), "%s (%s)", buf, name);
				} else {
					n = atoi(&subdirent->d_name[4]);
					g_snprintf((*list)[i].name, sizeof((*list)[i].name), "#%d (%s)", n, name);
				}

				// fill paths
				g_snprintf((*list)[i].node_path, sizeof((*list)[i].node_path), "%s/%s", root_node, dirent->d_name);
				g_snprintf((*list)[i].temp_path, sizeof((*list)[i].temp_path), "%s/%s", (*list)[i].node_path, subdirent->d_name);

				// look first for temp*_crit, then for temp*_max, else set critical = 0
				tmp = str_replace(subdirent->d_name, "_input", "_crit");
				g_snprintf(buf, PATH_MAX, "%s/%s/%s", root_node, dirent->d_name, tmp);
				g_free(tmp);
				(*list)[i].critical = read_int_from_file(buf);
				if ((*list)[i].critical == 0) {
					tmp = str_replace(subdirent->d_name, "_input", "_max");
					g_snprintf(buf, PATH_MAX, "%s/%s/%s", root_node, dirent->d_name, tmp);
					g_free(tmp);
					(*list)[i].critical = read_int_from_file(buf);
				}

				i++;
			}
			closedir(subdir);
		}
		closedir(dir);
		return TRUE;
	}

	// read phase - always return TRUE
	for (i=0; (*list)[i].temp_path[0] != '\0'; i++)
		(*list)[i].temp = read_int_from_file((*list)[i].temp_path);

	return TRUE;
}

static TemperatureSourceSupport
list_temp(TemperatureSourceData **list)
{
	if (list_temp_hwmon(list, TRUE))
		return TEMP_SOURCE_SUPPORT_HWMON;
	else if (list_temp_acpitz(list, TRUE))
		return TEMP_SOURCE_SUPPORT_ACPITZ;
	else
		return TEMP_SOURCE_NO_SUPPORT;
}

void
multiload_graph_temp_get_data (int Maximum, int data[2], LoadGraph *g, TemperatureData *xd)
{
	static TemperatureSourceSupport support = TEMP_SOURCE_SUPPORT_UNINITIALIZED;
	static TemperatureSourceData *list = NULL;
	TemperatureSourceData *use = NULL;

	guint i, m;

	if (G_UNLIKELY(support == TEMP_SOURCE_NO_SUPPORT))
		return;

	// initialization phase: looks for support type and builds static data
	if (G_UNLIKELY(support == TEMP_SOURCE_SUPPORT_UNINITIALIZED)) {
		support = list_temp(&list);
	}

	// read phase: fills in current temperature values
	switch (support) {
		case TEMP_SOURCE_SUPPORT_HWMON:
			list_temp_hwmon(&list, FALSE);
			break;
		case TEMP_SOURCE_SUPPORT_ACPITZ:
			list_temp_acpitz(&list, FALSE);
			break;
		default:
			g_assert_not_reached();
			return;
	}

	// select phase: choose which source to show
	if (g->config->filter_enable && g->config->filter[0] != '\0') {
		for (i=0; list[i].temp_path[0]!='\0'; i++) {
			if (strcmp(list[i].name, g->config->filter) == 0) {
				use = &list[i];
				g_debug("[graph-temp] Using source '%s' (selected by filter)", list[i].name);
				break;
			}
			g_debug("[graph-temp] No source found for filter '%s'", g->config->filter);
		}
	}
	if (use == NULL) { // filter disabled or filter value not found - auto selection
		for (i=1, m=0; list[i].temp_path[0]!='\0'; i++) {
			if (list[i].temp > list[m].temp)
				m = i;
		}
		use = &list[m];
	}

	// output phase
	int max = autoscaler_get_max(&xd->scaler, g, use->temp);

	if (use->critical > 0 && use->critical < use->temp) {
		data[0] = rint (Maximum * (float)(use->critical) / max);
		data[1] = rint (Maximum * (float)(use->temp - use->critical) / max);
	} else {
		data[0] = rint (Maximum * (float)(use->temp) / max);
		data[1] = 0;
	}

	strcpy(xd->name, use->name);
	xd->value = use->temp;
	xd->max = use->critical;
}


void
multiload_graph_temp_cmdline_output (LoadGraph *g, TemperatureData *xd)
{
	if (g->output_unit[0] == '\0')
		g_strlcpy(g->output_unit, "m°C", sizeof(g->output_unit));
	g_snprintf(g->output_str[0], sizeof(g->output_str[0]), "%d", xd->value);
	g_snprintf(g->output_str[1], sizeof(g->output_str[1]), "%d", xd->max);
}


void
multiload_graph_temp_tooltip_update (char **title, char **text, LoadGraph *g, TemperatureData *xd)
{
	if (g->config->tooltip_style == TOOLTIP_STYLE_DETAILS) {
		*title = g_strdup(xd->name);

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
