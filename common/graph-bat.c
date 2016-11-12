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

#define B(xd) ((bat_info*)(xd->battery))
#define R(b, i) (info_file_read_int64(b->path_##i, &b->i))

#define CRITICAL_LEVEL 4
#define PATH_POWER_SUPPLY "/sys/class/power_supply"


typedef struct {
	gchar *path_root;
	gchar *path_present;
	gchar *path_charge_now;
	gchar *path_energy_now;
	gchar *path_current_now;
	gchar *path_charge_full_design;
	gchar *path_energy_full_design;
	gchar *path_charge_full;
	gchar *path_energy_full;
	gchar *path_status;
	gchar *path_capacity;
	gchar *path_capacity_level;

	gboolean is_battery;
	gboolean is_charging;
	gboolean is_critical;
	gboolean is_present;
	gchar *name_label;
	gdouble percentage;

	gint64 charge_now;
	gint64 energy_now;
	gint64 current_now;
	gint64 charge_full_design;
	gint64 energy_full_design;
	gint64 charge_full;
	gint64 energy_full;
} bat_info;

static void
battery_update (bat_info *b)
{
	gchar buf[200];

	// present
	if (info_file_exists(b->path_present)) {
		b->is_present = info_file_has_contents(b->path_present, "1", TRUE);
		if (!b->is_present)
			return;
	} else {
		b->is_present = TRUE;
	}

	// charging
	if (R(b, current_now) && b->current_now==0) {
		b->is_charging = TRUE;
	} else if (info_file_read_string_s(b->path_status, buf, sizeof(buf), NULL)) {
		b->is_charging = (g_ascii_strcasecmp(buf, "Unknown")==0 || g_ascii_strcasecmp(buf, "Full") == 0 || g_ascii_strcasecmp(buf, "Charging") == 0 );
	} else {
		b->is_charging = TRUE;
	}

	// try to get/calculate percentage
	if (R(b, charge_now) && R(b, charge_full)) {
		b->percentage = 100.0 * b->charge_now / b->charge_full;
	} else if (R(b, charge_now) && R(b, charge_full_design)) {
		b->percentage = 100.0 * b->charge_now / b->charge_full_design;
	} else if (R(b, energy_now) && R(b, energy_full)) {
		b->percentage = 100.0 * b->energy_now / b->energy_full;
	} else if (R(b, energy_now) && R(b, energy_full_design)) {
		b->percentage = 100.0 * b->energy_now / b->energy_full_design;
	} else if (!info_file_read_double(b->path_capacity, &b->percentage, 1)) { // last resort, as it returns an integer
		b->percentage = 0.0; // no luck - percentage not found
	}
	b->percentage = CLAMP(b->percentage, 0.0, 100.0);

	// capacity level
	if (info_file_exists(b->path_capacity_level)) {
		b->is_critical = info_file_has_contents(b->path_capacity_level, "Critical", FALSE);
	} else {
		b->is_critical = (b->percentage <= CRITICAL_LEVEL);
	}
}

static bat_info*
battery_new (const gchar *bat_name)
{
	gchar buf[PATH_MAX];

	bat_info *b = g_new(bat_info, 1);

	b->path_root				= g_strdup_printf("%s/%s",					PATH_POWER_SUPPLY, bat_name);
	b->path_present				= g_strdup_printf("%s/present"	,			b->path_root);
	b->path_charge_now			= g_strdup_printf("%s/charge_now",			b->path_root);
	b->path_energy_now			= g_strdup_printf("%s/energy_now",			b->path_root);
	b->path_current_now			= g_strdup_printf("%s/current_now",			b->path_root);
	b->path_charge_full_design	= g_strdup_printf("%s/charge_full_design",	b->path_root);
	b->path_energy_full_design	= g_strdup_printf("%s/energy_full_design",	b->path_root);
	b->path_charge_full			= g_strdup_printf("%s/charge_full",			b->path_root);
	b->path_energy_full			= g_strdup_printf("%s/energy_full",			b->path_root);
	b->path_status				= g_strdup_printf("%s/status",				b->path_root);
	b->path_capacity			= g_strdup_printf("%s/capacity",			b->path_root);
	b->path_capacity_level		= g_strdup_printf("%s/capacity_level",		b->path_root);

	g_snprintf(buf, PATH_MAX, "%s/type", b->path_root);
	b->is_battery = info_file_has_contents(buf, "battery", FALSE);

	// battery name
	gchar manufacturer[120], model_name[120];
	g_snprintf(buf, PATH_MAX, "%s/manufacturer", b->path_root);
	if (!info_file_read_string_s (buf, manufacturer, sizeof(manufacturer), NULL))
		manufacturer[0] = '\0';
	g_snprintf(buf, PATH_MAX, "%s/model_name", b->path_root);
	if (!info_file_read_string_s (buf, model_name, sizeof(model_name), NULL))
		model_name[0] = '\0';
	b->name_label = g_strdup_printf ("%s%s%s",
		manufacturer,
		(manufacturer[0]!='\0' && model_name[0]!='\0')?" ":"",
		model_name);

	return b;
}

static void
battery_free (bat_info *b)
{
	if (b == NULL)
		return;

	g_free (b->path_root);
	g_free (b->path_present);
	g_free (b->path_charge_now);
	g_free (b->path_energy_now);
	g_free (b->path_current_now);
	g_free (b->path_charge_full_design);
	g_free (b->path_energy_full_design);
	g_free (b->path_charge_full);
	g_free (b->path_energy_full);
	g_free (b->path_status);
	g_free (b->path_capacity);
	g_free (b->path_capacity_level);

	g_free (b->name_label);

	g_free (b);
}

static void
battery_clear (bat_info **b)
{
	battery_free(*b);
	*b = NULL;
}


void
multiload_graph_bat_init (LoadGraph *g, BatteryData *xd)
{
	struct dirent *dirent;

	// check if /sys node exists, otherwise means no battery support or no battery at all
	DIR *dir = opendir(PATH_POWER_SUPPLY);
	if (dir == NULL)
		return;

	while ((dirent = readdir(dir)) != NULL) {
		xd->battery = battery_new(dirent->d_name);
		bat_info *battery = B(xd);

		if (!battery->is_battery) {
			battery_clear(&battery);
			continue;
		}

		break;
	}
	closedir(dir);
}

void
multiload_graph_bat_get_data (int Maximum, int data [3], LoadGraph *g, BatteryData *xd, gboolean first_call)
{
	memset(data, 0, 3*sizeof(data[0]));

	bat_info *battery = B(xd);

	if (battery == NULL)
		return;

	battery_update(battery);

	if (!battery->is_present)
		return;

	int val = rint (Maximum * battery->percentage / 100);
	if (battery->is_charging)
		data[0] = val;
	else if (!battery->is_critical)
		data[1] = val;
	else
		data[2] = val;
}


void
multiload_graph_bat_cmdline_output (LoadGraph *g, BatteryData *xd)
{
	bat_info *battery = B(xd);

	if (battery == NULL)
		return;

	g_snprintf(g->output_str[0], sizeof(g->output_str[0]), "%.1f", battery->percentage);
}


void
multiload_graph_bat_tooltip_update (char *buf_title, size_t len_title, char *buf_text, size_t len_text, LoadGraph *g, BatteryData *xd, gint style)
{
	bat_info *battery = B(xd);

	if (battery == NULL)
		return;

	if (style == MULTILOAD_TOOLTIP_STYLE_DETAILED) {
		gchar *capacity = g_strdup_printf(_("Capacity: %.1f%%"), battery->percentage);
		gchar *status = (battery->is_charging? _("Charging") : _("Discharging"));

		strncpy(buf_title, battery->name_label, len_title);
		if (battery->is_critical)
			g_snprintf(buf_text, len_text, "%s (%s)\n%s", capacity, _("Critical level"), status);
		else
			g_snprintf(buf_text, len_text, "%s\n%s", capacity, status);

		g_free(capacity);
	} else {
		g_snprintf(buf_text, len_text, "%.1f%%", battery->percentage);
	}
}
