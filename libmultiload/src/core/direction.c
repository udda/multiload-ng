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


static const char *_direction_str[] = {"nw", "n", "ne", "e", "se", "s", "sw", "w", NULL};

const char *
ml_direction_to_string (MlDirection dir) {
	if (dir < 0 || dir > 7)
		return NULL;

	return _direction_str[(int)dir];
}

MlDirection
ml_direction_parse (const char* def) {
	ml_strv_for (_direction_str, i) {
		if (ml_string_equals (_direction_str[i], def, true))
			return (MlDirection)i;
	}

	return ML_INVALID;
}

cJSON *
ml_direction_to_json (MlDirection dir)
{
	const char *str = ml_direction_to_string (dir);
	if_unlikely (dir == ML_INVALID)
		return NULL;

	return cJSON_CreateString (str);
}

MlDirection
ml_direction_parse_json (cJSON* obj)
{
	if_unlikely (obj == NULL)
		return ML_INVALID;

	return ml_direction_parse (obj->valuestring);
}
