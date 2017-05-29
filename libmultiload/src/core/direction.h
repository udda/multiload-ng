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

#ifndef ML_HEADER__CORE_DIRECTION_H__INCLUDED
#define ML_HEADER__CORE_DIRECTION_H__INCLUDED
ML_HEADER_BEGIN


typedef enum {
	ML_DIRECTION_NW			= 0,
	ML_DIRECTION_N			= 1,
	ML_DIRECTION_NE			= 2,
	ML_DIRECTION_E			= 3,
	ML_DIRECTION_SE			= 4,
	ML_DIRECTION_S			= 5,
	ML_DIRECTION_SW			= 6,
	ML_DIRECTION_W			= 7
} MlDirection;


const char *
ml_direction_to_string (MlDirection dir)
ML_FN_PURE;

MlDirection
ml_direction_parse (const char* def)
ML_FN_PURE;

cJSON *
ml_direction_to_json (MlDirection dir);

MlDirection
ml_direction_parse_json (cJSON* obj)
ML_FN_PURE ML_FN_COLD;


ML_HEADER_END
#endif /* ML_HEADER__CORE_DIRECTION_H__INCLUDED */
