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

#ifndef ML_HEADER__CORE_ORIENTATION_H__INCLUDED
#define ML_HEADER__CORE_ORIENTATION_H__INCLUDED
ML_HEADER_BEGIN


typedef enum {
	ML_ORIENTATION_HORIZONTAL = 0,
	ML_ORIENTATION_VERTICAL = 1
} MlOrientation;


const char *
ml_orientation_to_string (MlOrientation o)
ML_FN_PURE;

MlOrientation
ml_orientation_parse (const char* def)
ML_FN_PURE;

cJSON *
ml_orientation_to_json (MlOrientation o);

MlOrientation
ml_orientation_parse_json (cJSON* obj)
ML_FN_PURE ML_FN_COLD;

int
ml_orientation_x (MlOrientation o, int coord_x, int coord_y)
ML_FN_CONST;

int
ml_orientation_y (MlOrientation o, int coord_x, int coord_y)
ML_FN_CONST;


ML_HEADER_END
#endif /* ML_HEADER__CORE_ORIENTATION_H__INCLUDED */
