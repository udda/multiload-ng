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

#ifndef ML_HEADER__SYS_TEMP_INFO_H__INCLUDED
#define ML_HEADER__SYS_TEMP_INFO_H__INCLUDED
ML_HEADER_BEGIN


typedef enum {
	TEMP_ACPITZ,
	TEMP_HWMON
} MlTemperatureInfoType;


typedef struct {
	char *label;
	char *id;

	char *current_temp_path;
	double current_temp;
	double critical_temp;
} MlTemperatureInfo;


MlTemperatureInfo *
ml_temperature_info_new (const char *node_path, MlTemperatureInfoType type);

void
ml_temperature_info_destroy (MlTemperatureInfo *ti);

size_t
ml_temperature_info_sizeof (MlTemperatureInfo *ti)
ML_FN_SIZEOF;

bool
ml_temperature_info_update (MlTemperatureInfo *ti)
ML_FN_WARN_UNUSED_RESULT;

MlGrowArray *
ml_temperature_info_get_available ();

MlGrowBuffer *
ml_temperature_info_generate_report ();


ML_HEADER_END
#endif /* ML_HEADER__SYS_TEMP_INFO_H__INCLUDED */
