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


#define PATH_ACPITZ "/sys/class/thermal"
#define PATH_HWMON "/sys/class/hwmon"


static char *
_ml_temperature_info_generate_unique_name (const char *driver_name)
{
	static unsigned uid = 0;

	if (driver_name != NULL && *driver_name != '\0')
		return ml_strdup_printf ("TZ #%u (%s)", uid++, driver_name);
	else
		return ml_strdup_printf ("TZ #%u", uid++);
}

MlTemperatureInfo *
ml_temperature_info_new (const char *node_path, MlTemperatureInfoType type)
{
	char buf_path[PATH_MAX];
	char buf_label[32];

	MlTemperatureInfo *ti = ml_new (MlTemperatureInfo);

	if (type == TEMP_ACPITZ) {
		// node_path is /sys/class/thermal/thermal_zone*
		ti->current_temp_path = ml_strdup_printf ("%s/temp", node_path);

		// id is in the form acpitz:<number>
		ti->id = ml_strdup_printf ("acpitz:%s", node_path+31);

		// fill label from device path if present, else generate unique name
		snprintf (buf_path, sizeof(buf_path), "%s/device/path", node_path);
		if (ml_infofile_read_string_s (buf_path, buf_label, sizeof(buf_label), NULL)) {
			// remove leading path prefix
			if (strncmp(buf_label, "\\_TZ_.", 6) == 0)
				ti->label = ml_strdup (buf_label+6);
			else
				ti->label = ml_strdup (buf_label);
		} else {
			ti->label = _ml_temperature_info_generate_unique_name ("acpitz");
		}

		// find "critical" temperature searching in trip points
		for (int i = 0; ; i++) {
			snprintf (buf_path, sizeof(buf_path), "%s/trip_point_%d_type", node_path, i);
			if (!ml_file_is_readable (buf_path))
				break; //no more trip point files, stop searching

			if (ml_infofile_has_contents (buf_path, "critical", false)) { // found critical temp
				snprintf (buf_path, sizeof(buf_path), "%s/trip_point_%d_temp", node_path, i);
				if (!ml_infofile_read_double (buf_path, &ti->critical_temp, 1000.0))
					ti->critical_temp = 0;
			}
		}

	} else if (type == TEMP_HWMON) {
		// node_path is /sys/class/hwmon/*/temp[0-9]+_input
		char driver_name [60];
		char *pch;
		unsigned n;

		// id is in the form hwmon:<name>/<number>
		if (2 == sscanf (node_path, PATH_HWMON"/%31[^/]/temp%u_input", buf_label, &n))
			ti->id = ml_strdup_printf ("hwmon:%s/%u", buf_label, n);
		else
			ti->id = ml_strdup_printf ("hwmon:%s", node_path+17);
		
		
		// current temp path is node_path
		ti->current_temp_path = ml_strdup (node_path);

		// driver name (used in label)
		strncpy (buf_path, node_path, sizeof(buf_path));
		pch = strrchr (buf_path, '/');
		if (pch != NULL)
			sprintf (pch, "/name");
		if (pch == NULL || !ml_infofile_read_string_s (buf_path, driver_name, sizeof(driver_name), NULL))
			driver_name[0] = '\0';

		// fill label - search for temp*_label, else generate unique name
		strncpy (buf_path, node_path, sizeof(buf_path));
		pch = strstr (buf_path, "_input");
		if (pch != NULL)
			sprintf (pch, "_label");
		if (pch != NULL && ml_infofile_read_string_s (buf_path, buf_label, sizeof(buf_label), NULL))
			ti->label = ml_strdup_printf ("%s (%s)", buf_label, driver_name);
		else
			ti->label = _ml_temperature_info_generate_unique_name (driver_name);

		// look first for temp*_crit, then for temp*_max
		if (pch != NULL) {
			sprintf (pch, "_crit");
			if (!ml_infofile_read_double (buf_path, &ti->critical_temp, 1000.0)) {
				sprintf (pch, "_max");
				if (!ml_infofile_read_double (buf_path, &ti->critical_temp, 1000.0))
					ti->critical_temp = 0;
			}
		}
	}

	return ti;
}

void
ml_temperature_info_destroy (MlTemperatureInfo *ti)
{
	if_unlikely (ti == NULL)
		return;

	free (ti->id);
	free (ti->label);
	free (ti->current_temp_path);
	free (ti);
}

size_t
ml_temperature_info_sizeof (MlTemperatureInfo *ti)
{
	if_unlikely (ti == NULL)
		return 0;

	size_t size = sizeof (MlTemperatureInfo);

	size += ml_string_sizeof (ti->id);
	size += ml_string_sizeof (ti->label);
	size += ml_string_sizeof (ti->current_temp_path);

	return size;
}

bool
ml_temperature_info_update (MlTemperatureInfo *ti)
{
	if_unlikely (ti == NULL)
		return false;
	
	return ml_infofile_read_double (ti->current_temp_path, &ti->current_temp, 1000.0);
}

MlGrowArray*
ml_temperature_info_get_available ()
{
	DIR *dir, *subdir;
	struct dirent *dirent, *subdirent;
	char buf[PATH_MAX];

	MlGrowArray *arr = ml_grow_array_new (8, (MlDestroyFunc)ml_temperature_info_destroy);

	// first check hwmon (if present, includes acpitz devices)
	if ((dir = opendir (PATH_HWMON)) != NULL) {
		while ((dirent = readdir (dir)) != NULL) {
			if (ml_string_equals (dirent->d_name, ".", false) || ml_string_equals (dirent->d_name, "..", false))
				continue;

			snprintf (buf, sizeof(buf), PATH_HWMON"/%s", dirent->d_name);
			if ((subdir = opendir (buf)) == NULL)
				continue;

			while ((subdirent = readdir (subdir)) != NULL) {
				if (ml_string_equals (subdirent->d_name, ".", false) || ml_string_equals (subdirent->d_name, "..", false))
					continue;

				if (!ml_string_matches (subdirent->d_name, "^temp[0-9]+_input$", true))
					continue;

				snprintf (buf, sizeof(buf), PATH_HWMON"/%s/%s", dirent->d_name, subdirent->d_name);
				MlTemperatureInfo *ti = ml_temperature_info_new (buf, TEMP_HWMON);
				if (ti != NULL)
					ml_grow_array_append (arr, ti);
			}
			closedir (subdir);

		}
		closedir (dir);

		if (!ml_grow_array_is_empty (arr))
			return arr;
	}

	// no hwmon devices - check acpitz as fallback
	if ((dir = opendir (PATH_ACPITZ)) != NULL) {
		while ((dirent = readdir (dir)) != NULL) {
			if (ml_string_equals (dirent->d_name, ".", false) || ml_string_equals (dirent->d_name, "..", false))
				continue;

			if (strncmp (dirent->d_name, "thermal_zone", 12) != 0)
				continue;

			snprintf (buf, sizeof(buf), PATH_ACPITZ"/%s", dirent->d_name);
			MlTemperatureInfo *ti = ml_temperature_info_new (buf, TEMP_ACPITZ);
			if (ti != NULL)
				ml_grow_array_append (arr, ti);
		}
		closedir (dir);

		if (!ml_grow_array_is_empty (arr))
			return arr;
	}

	// no luck - free empty array and return
	ml_grow_array_destroy (arr, true, false);
	return NULL;
}

MlGrowBuffer *
ml_temperature_info_generate_report ()
{
	MlGrowBuffer *gbuf = ml_grow_buffer_new(10240);

	DIR *dir, *subdir;
	struct dirent *dirent, *subdirent;
	char buf[PATH_MAX];

	MlGrowArray *ga = ml_grow_array_new (8, (MlDestroyFunc)ml_temperature_info_destroy);

	// first check hwmon (if present, includes acpitz devices)
	if ((dir = opendir (PATH_HWMON)) == NULL) {
		ml_grow_buffer_append_printf (gbuf, "Cannot read hwmon devices from %s\n", PATH_HWMON);
	} else {
		ml_grow_buffer_append_printf (gbuf, "Reading hwmon devices from %s\n", PATH_HWMON);

		while ((dirent = readdir (dir)) != NULL) {
			if (ml_string_equals (dirent->d_name, ".", false) || ml_string_equals (dirent->d_name, "..", false))
				continue;

			snprintf (buf, sizeof(buf), PATH_HWMON"/%s", dirent->d_name);
			if ((subdir = opendir (buf)) == NULL) {
				ml_grow_buffer_append_printf (gbuf, "Hwmon: cannot open %s, skipping\n", dirent->d_name);
				continue;
			}

			while ((subdirent = readdir (subdir)) != NULL) {
				if (ml_string_equals (subdirent->d_name, ".", false) || ml_string_equals (subdirent->d_name, "..", false))
					continue;

				if (!ml_string_matches (subdirent->d_name, "^temp[0-9]+_input$", true))
					continue;

				snprintf (buf, sizeof(buf), PATH_HWMON"/%s/%s", dirent->d_name, subdirent->d_name);
				MlTemperatureInfo *ti = ml_temperature_info_new (buf, TEMP_HWMON);
				if (ti == NULL) {
					ml_grow_buffer_append_printf (gbuf, "Hwmon: cannot create MlTemperatureInfo for %s/%s. This is a bug.\n", dirent->d_name, subdirent->d_name);
					continue;
				} else {
					ml_grow_buffer_append_printf (gbuf, "Hwmon: found valid device %s/%s\n", dirent->d_name, subdirent->d_name);
					ml_grow_array_append (ga, ti);
				}
			}
			closedir (subdir);

		}
		closedir (dir);
	}

	if (ml_grow_array_is_empty (ga)) {
		ml_grow_buffer_append_printf (gbuf, "No hwmon devices found. Trying ACPI thermal zones as fallback.\n");

		if ((dir = opendir (PATH_ACPITZ)) == NULL) {
			ml_grow_buffer_append_printf (gbuf, "Cannot read ACPI thermal zone devices from %s\n", PATH_ACPITZ);
		} else {
			ml_grow_buffer_append_printf (gbuf, "Reading ACPI thermal zone devices from %s\n", PATH_ACPITZ);
			while ((dirent = readdir (dir)) != NULL) {
				if (ml_string_equals (dirent->d_name, ".", false) || ml_string_equals (dirent->d_name, "..", false))
					continue;

				if (strncmp (dirent->d_name, "thermal_zone", 12) != 0)
					continue;

				snprintf (buf, sizeof(buf), PATH_ACPITZ"/%s", dirent->d_name);
				MlTemperatureInfo *ti = ml_temperature_info_new (buf, TEMP_ACPITZ);
				if (ti == NULL) {
					ml_grow_buffer_append_printf (gbuf, "AcpiTZ: cannot create MlTemperatureInfo for %s. This is a bug.\n", dirent->d_name);
					continue;
				} else {
					ml_grow_buffer_append_printf (gbuf, "AcpiTZ: found valid device %s\n", dirent->d_name);
					ml_grow_array_append (ga, ti);
				}
			}
			closedir (dir);
		}
	}


	if (ml_grow_array_is_empty(ga)) {
		ml_grow_buffer_append_printf (gbuf, "No temperature sources found.\n");
	} else {
		ml_grow_buffer_append_printf (gbuf, "Found %d temperature sources.\n", ml_grow_array_get_length (ga));

		ml_grow_array_for (ga, i) {
			MlTemperatureInfo *ti = ml_grow_array_get(ga, i);

			ml_grow_buffer_append_printf (gbuf, "\n\nTEMPERATURE SOURCE #%d\n----------\n", i);
			ml_grow_buffer_append_printf (gbuf, "Internal name:         %s\n", ti->id);
			ml_grow_buffer_append_printf (gbuf, "Label:                 %s\n", ti->label);
			ml_grow_buffer_append_printf (gbuf, "Critical temperature:  %0.02f\n", ti->critical_temp);

			if (ml_temperature_info_update (ti))
				ml_grow_buffer_append_printf (gbuf, "Current temperature:   %0.02f\n", ti->current_temp);
			else
				ml_grow_buffer_append_printf (gbuf, "Error: cannot retrieve current temperature.\n");
		}
	}

	ml_grow_array_destroy (ga, true, false);

	return gbuf;
}
