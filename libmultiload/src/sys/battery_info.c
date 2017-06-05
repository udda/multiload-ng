/*
 * Copyright (C) 2017 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of Multiload-ng.
 *
 * Multiload-ng is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Multiload-ng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Multiload-ng.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <multiload.h>


#define PATH_POWER_SUPPLY "/sys/class/power_supply"


MlBatteryInfo*
ml_battery_info_new (const char *bat_name)
{
	if_unlikely (bat_name == NULL)
		return NULL;

	char buf[PATH_MAX];
	snprintf (buf, PATH_MAX, "%s/%s/type", PATH_POWER_SUPPLY, bat_name);
	if (!ml_infofile_has_contents(buf, "battery", false))
		return NULL;

	MlBatteryInfo *bi = ml_new (MlBatteryInfo);
	bi->path_root				= ml_strdup_printf ("%s/%s",					PATH_POWER_SUPPLY, bat_name);
	bi->path_present			= ml_strdup_printf ("%s/present"	,			bi->path_root);
	bi->path_current_now		= ml_strdup_printf ("%s/current_now",			bi->path_root);
	bi->path_voltage_now		= ml_strdup_printf ("%s/voltage_now",			bi->path_root);
	bi->path_charge_now			= ml_strdup_printf ("%s/charge_now",			bi->path_root);
	bi->path_charge_full		= ml_strdup_printf ("%s/charge_full",			bi->path_root);
	bi->path_charge_full_design	= ml_strdup_printf ("%s/charge_full_design",	bi->path_root);
	bi->path_energy_now			= ml_strdup_printf ("%s/energy_now",			bi->path_root);
	bi->path_energy_full		= ml_strdup_printf ("%s/energy_full",			bi->path_root);
	bi->path_energy_full_design	= ml_strdup_printf ("%s/energy_full_design",	bi->path_root);
	bi->path_status				= ml_strdup_printf ("%s/status",				bi->path_root);
	bi->path_capacity			= ml_strdup_printf ("%s/capacity",				bi->path_root);
	bi->path_capacity_level		= ml_strdup_printf ("%s/capacity_level",		bi->path_root);

	// battery name
	snprintf (buf, PATH_MAX, "%s/model_name", bi->path_root);
	if (!ml_infofile_read_string (buf, &bi->model_name, NULL))
		bi->model_name = ml_strdup (bat_name);
	snprintf (buf, PATH_MAX, "%s/manufacturer", bi->path_root);
	if (!ml_infofile_read_string (buf, &bi->manufacturer, NULL)) {}
	snprintf (buf, PATH_MAX, "%s/technology", bi->path_root);
	if (!ml_infofile_read_string (buf, &bi->technology, NULL)) {}

	return bi;
}

void
ml_battery_info_destroy (MlBatteryInfo *bi)
{
	if_unlikely (bi == NULL)
		return;

	free (bi->path_root);
	free (bi->path_present);
	free (bi->path_current_now);
	free (bi->path_voltage_now);
	free (bi->path_charge_now);
	free (bi->path_charge_full);
	free (bi->path_charge_full_design);
	free (bi->path_energy_now);
	free (bi->path_energy_full);
	free (bi->path_energy_full_design);
	free (bi->path_status);
	free (bi->path_capacity);
	free (bi->path_capacity_level);

	free (bi->model_name);
	free (bi->manufacturer);
	free (bi->technology);

	free (bi);
}

size_t
ml_battery_info_sizeof (MlBatteryInfo *bi)
{
	if_unlikely (bi == NULL)
		return 0;

	size_t size = sizeof (MlBatteryInfo);

	size += ml_string_sizeof(bi->path_root);
	size += ml_string_sizeof(bi->path_present);
	size += ml_string_sizeof(bi->path_current_now);
	size += ml_string_sizeof(bi->path_voltage_now);
	size += ml_string_sizeof(bi->path_charge_now);
	size += ml_string_sizeof(bi->path_charge_full);
	size += ml_string_sizeof(bi->path_charge_full_design);
	size += ml_string_sizeof(bi->path_energy_now);
	size += ml_string_sizeof(bi->path_energy_full);
	size += ml_string_sizeof(bi->path_energy_full_design);
	size += ml_string_sizeof(bi->path_status);
	size += ml_string_sizeof(bi->path_capacity);
	size += ml_string_sizeof(bi->path_capacity_level);

	size += ml_string_sizeof(bi->model_name);
	size += ml_string_sizeof(bi->manufacturer);
	size += ml_string_sizeof(bi->technology);

	return size;
}

MlBatteryInfo*
ml_battery_info_get_first_available ()
{
	struct dirent *dirent;

	// check if /sys node exists, otherwise means no battery support or no battery at all
	DIR *dir = opendir(PATH_POWER_SUPPLY);
	if_unlikely (dir == NULL)
		return NULL;

	while ((dirent = readdir(dir)) != NULL) {
		MlBatteryInfo *bi = ml_battery_info_new (dirent->d_name);
		if (bi != NULL) {
			closedir (dir);
			return bi;
		}
	}
	closedir (dir);

	return NULL;
}

#define READ_INT32_OR_FALLBACK(path,variable,fallback) if (!ml_infofile_read_int32((path), &(variable))) { (variable) = (fallback); }

bool
ml_battery_info_update (MlBatteryInfo *bi, double critical_level)
{
	if_unlikely (bi == NULL)
		return false;

	// battery present
	if (ml_file_is_readable(bi->path_present) && !ml_infofile_has_contents(bi->path_present, "1", true)) {
		bi->status = ML_BATTERY_STATUS_ABSENT;
		return true; // battery is not present, but update technically succeeded
	}


	// read values
	char status[20];
	int32_t current_now, voltage_now;
	int32_t charge_now, charge_full;
	int32_t energy_now, energy_full;

	READ_INT32_OR_FALLBACK (bi->path_current_now, current_now, -1)
	READ_INT32_OR_FALLBACK (bi->path_voltage_now, voltage_now, -1)

	READ_INT32_OR_FALLBACK (bi->path_charge_now, charge_now, -1)
	READ_INT32_OR_FALLBACK (bi->path_charge_full, charge_full, -1)
	if (charge_now == -1)
		READ_INT32_OR_FALLBACK (bi->path_charge_full_design, charge_full, -1)

	if (charge_now == -1) { // fallback: read energy (µWh)
		READ_INT32_OR_FALLBACK (bi->path_energy_now, energy_now, -1)
		READ_INT32_OR_FALLBACK (bi->path_energy_full, energy_full, -1)
		if (energy_full == -1)
			READ_INT32_OR_FALLBACK (bi->path_energy_full_design, energy_full, -1)
	}

	if (!ml_infofile_read_string_s(bi->path_status, status, sizeof(status), NULL))
		strncpy (status, "Unknown", sizeof(status));


	// percentage
	if (charge_now >= 0 && charge_full > 0)
		bi->percentage = 100.0 * charge_now / charge_full;
	else if (energy_now >= 0 && energy_full > 0)
		bi->percentage = 100.0 * energy_now / energy_full;
	else if (!ml_infofile_read_double(bi->path_capacity, &bi->percentage, 1)) // last resort, as it returns an integer
		return false; // no luck - percentage not found

	if (bi->percentage < 0)
		bi->percentage = 0;


	// battery status
	if (ml_string_equals (status, "Full", false) || (ml_string_equals (status, "Unknown", false) && current_now == 0)) { // full
		bi->status = ML_BATTERY_STATUS_FULL;
	} else if (ml_string_equals (status, "Charging", false) || (ml_string_equals (status, "Unknown", false) && current_now != 0)) { // charging
		bi->status = ML_BATTERY_STATUS_CHARGING;
	} else { // discharging
		bi->status = ML_BATTERY_STATUS_DISCHARGING;
	}


	// if charge is not available, try to calculate it from energy
	if (charge_now <= 0 && voltage_now > 0 && energy_now > 0)
		charge_now = (uint64_t)1000000 * energy_now / voltage_now;


	// time left for charging and discharging
	if (bi->status == ML_BATTERY_STATUS_DISCHARGING && charge_now > 0 && current_now > 0)
		bi->time_left = (uint64_t)3600 * charge_now / current_now;
	else if (bi->status == ML_BATTERY_STATUS_CHARGING && charge_full > charge_now && charge_now > 0 && current_now > 0)
		bi->time_left = (uint64_t)3600 * (charge_full - charge_now) / current_now;
	else
		bi->time_left = 0;


	// critical level
	if (ml_infofile_has_contents(bi->path_capacity_level, "Critical", false) || (bi->percentage <= critical_level))
		bi->is_critical = true;


	return true;
}

const char *
ml_battery_info_status_to_string (MlBatteryStatus status)
{
	switch (status) {
		case ML_BATTERY_STATUS_FULL:
			return _("Full");
		case ML_BATTERY_STATUS_CHARGING:
			return _("Charging");
		case ML_BATTERY_STATUS_DISCHARGING:
			return _("Discharging");
		case ML_BATTERY_STATUS_ABSENT:
			return _("Absent");
		default:
			return NULL;
	}
}

MlGrowBuffer *
ml_battery_info_generate_report ()
{
	MlGrowBuffer *gbuf = ml_grow_buffer_new (1024);


	DIR *dir = opendir(PATH_POWER_SUPPLY);
	if_unlikely (dir == NULL) {
		ml_grow_buffer_append_printf (gbuf, "No batteries found: cannot open '%s'\n", PATH_POWER_SUPPLY);
		return gbuf;
	}

	MlGrowArray *ga = ml_grow_array_new (1, (MlDestroyFunc)ml_battery_info_destroy);

	ml_grow_buffer_append_printf (gbuf, "Searching for batteries in '%s'.\n", PATH_POWER_SUPPLY);

	struct dirent *dirent;
	while ((dirent = readdir(dir)) != NULL) {
		if (ml_string_equals(dirent->d_name, ".", false) || ml_string_equals(dirent->d_name, "..", false))
			continue;

		ml_grow_buffer_append_printf (gbuf, "(analyzing %s ... ", dirent->d_name);

		MlBatteryInfo *bi = ml_battery_info_new (dirent->d_name);
		if (bi != NULL) {
			ml_grow_buffer_append_printf (gbuf, "OK)\n");
			ml_grow_array_append (ga, bi);
		} else {
			ml_grow_buffer_append_printf (gbuf, "not a battery)\n");
		}
	}
	closedir (dir);

	if (ml_grow_array_is_empty(ga)) {
		ml_grow_buffer_append_printf (gbuf, "No batteries found.\n");
	} else {
		ml_grow_buffer_append_printf (gbuf, "Found %d batteries.\n", ml_grow_array_get_length (ga));

		ml_grow_array_for (ga, i) {
			char buf[1024];
			MlBatteryInfo *bi = ml_grow_array_get(ga, i);

			ml_grow_buffer_append_printf (gbuf, "\n\nBATTERY #%d\n----------\n", i);
			ml_grow_buffer_append_printf (gbuf, "Base path             %s\n", bi->path_root);
			ml_grow_buffer_append_printf (gbuf, "Model name            %s\n", bi->model_name);
			ml_grow_buffer_append_printf (gbuf, "Manufacturer          %s\n", bi->manufacturer);
			ml_grow_buffer_append_printf (gbuf, "Technology            %s\n", bi->technology);

			if (ml_battery_info_update (bi, 0)) {
				ml_grow_buffer_append_printf (gbuf, "Status                %s\n", ml_battery_info_status_to_string(bi->status));
				ml_grow_buffer_append_printf (gbuf, "Estimated charge      %0.2f%%\n", bi->percentage);

				ml_string_format_time_s (bi->time_left, buf, sizeof(buf));
				ml_grow_buffer_append_printf (gbuf, "Estimated time left   %"PRIu64"s (%s)\n", bi->time_left, buf);
			} else {
				ml_grow_buffer_append_printf (gbuf, "Cannot retrieve extra info! Battery update failed.\n");
			}

			ml_grow_buffer_append_printf (gbuf, "\n", buf);
			char path[PATH_MAX];
			snprintf (path, sizeof(path), "%s/uevent", bi->path_root);
			if (ml_infofile_read_string_s (path, buf, sizeof(buf), NULL))
				ml_grow_buffer_append_printf (gbuf, "%s\n", buf);
			else
				ml_grow_buffer_append_printf (gbuf, "(no uevent file)\n");
		}
	}

	ml_grow_array_destroy (ga, true, false);
	return gbuf;
}
