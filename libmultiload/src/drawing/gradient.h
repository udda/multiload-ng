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

#ifndef ML_HEADER__DRAWING_GRADIENT_H__INCLUDED
#define ML_HEADER__DRAWING_GRADIENT_H__INCLUDED
ML_HEADER_BEGIN
#include "color.h"


typedef struct {
	MlColor start;
	MlColor end;
	MlDirection direction;
} MlGradient;


size_t
ml_gradient_sizeof (MlGradient *grad)
ML_FN_SIZEOF;

cJSON *
ml_gradient_to_json (const MlGradient *grad)
ML_FN_COLD;

bool
ml_gradient_parse_json (MlGradient *grad, cJSON *obj)
ML_FN_WARN_UNUSED_RESULT;

void
ml_gradient_step_s (const MlGradient *grad, unsigned step, unsigned n_steps, MlColor *dest);


ML_HEADER_END
#endif /* ML_HEADER__DRAWING_GRADIENT_H__INCLUDED */
