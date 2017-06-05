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

#ifndef ML_HEADER__SYS_BATTERY_INFO_H__INCLUDED
#define ML_HEADER__SYS_BATTERY_INFO_H__INCLUDED
ML_HEADER_BEGIN


typedef enum {
	ML_BATTERY_STATUS_FULL,
	ML_BATTERY_STATUS_CHARGING,
	ML_BATTERY_STATUS_DISCHARGING,
	ML_BATTERY_STATUS_ABSENT
} MlBatteryStatus;

typedef struct {
	char *path_root;
	char *path_present;

	char *path_current_now;
	char *path_voltage_now;

	char *path_charge_now;
	char *path_charge_full;
	char *path_charge_full_design;

	char *path_energy_now;
	char *path_energy_full;
	char *path_energy_full_design;

	char *path_status;
	char *path_capacity;
	char *path_capacity_level;

	MlBatteryStatus status;

	bool is_critical;
	double percentage;
	uint64_t time_left;

	char *model_name;
	char *manufacturer;
	char *technology;
} MlBatteryInfo;


MlBatteryInfo *
ml_battery_info_new (const char *bat_name);

void
ml_battery_info_destroy (MlBatteryInfo *bi);

size_t
ml_battery_info_sizeof (MlBatteryInfo *bi)
ML_FN_SIZEOF;

MlBatteryInfo *
ml_battery_info_get_first_available ();

bool
ml_battery_info_update (MlBatteryInfo *bi, double critical_level)
ML_FN_WARN_UNUSED_RESULT;

const char *
ml_battery_info_status_to_string (MlBatteryStatus status)
ML_FN_CONST;

MlGrowBuffer *
ml_battery_info_generate_report ()
ML_FN_COLD ML_FN_RETURNS_NONNULL;


ML_HEADER_END
#endif /* ML_HEADER__SYS_BATTERY_INFO_H__INCLUDED */
