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


#define STR_HORIZONTAL "horizontal"
#define STR_VERTICAL   "vertical"

const char *
ml_orientation_to_string (MlOrientation o) {
	if (o == ML_ORIENTATION_HORIZONTAL)
		return STR_HORIZONTAL;
	else if (o == ML_ORIENTATION_VERTICAL)
		return STR_VERTICAL;
	else
		return NULL;
}

MlOrientation
ml_orientation_parse (const char* def) {
	if_unlikely (def == NULL)
		return ML_INVALID;

	if (ml_string_equals (def, STR_HORIZONTAL, true))
		return ML_ORIENTATION_HORIZONTAL;
	else if (ml_string_equals (def, STR_VERTICAL, true))
		return ML_ORIENTATION_VERTICAL;
	else
		return ML_INVALID;
}


cJSON *
ml_orientation_to_json (MlOrientation o)
{
	const char *str = ml_orientation_to_string (o);
	if_unlikely (o == ML_INVALID)
		return NULL;

	return cJSON_CreateString (str);
}

MlOrientation
ml_orientation_parse_json (cJSON* obj)
{
	if_unlikely (obj == NULL)
		return ML_INVALID;

	return ml_orientation_parse (obj->valuestring);
}


int
ml_orientation_x (MlOrientation o, int coord_x, int coord_y)
{
	if (o == ML_ORIENTATION_HORIZONTAL)
		return coord_x;
	else
		return coord_y;
}

int
ml_orientation_y (MlOrientation o, int coord_x, int coord_y)
{
	if (o == ML_ORIENTATION_HORIZONTAL)
		return coord_y;
	else
		return coord_x;
}
