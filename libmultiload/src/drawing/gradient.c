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


size_t
ml_gradient_sizeof (MlGradient *grad)
{
	if_unlikely (grad == NULL)
		return 0;

	return sizeof (MlGradient);
}

cJSON *
ml_gradient_to_json (const MlGradient *grad)
{
	if_unlikely (grad == NULL)
		return NULL;

	cJSON* obj = cJSON_CreateObject ();

	cJSON_AddItemToObject (obj, "start",      ml_color_to_json (&grad->start));
	cJSON_AddItemToObject (obj, "end",        ml_color_to_json (&grad->end));
	cJSON_AddItemToObject (obj, "direction",  ml_direction_to_json (grad->direction));

	return obj;
}

bool
ml_gradient_parse_json (MlGradient *grad, cJSON *obj)
{
	if_unlikely (grad == NULL || obj == NULL)
		return false;

	if (!ml_color_parse_json (&grad->start, ml_cJSON_GetObjectItem (obj, "start")))
		return false;

	if (!ml_color_parse_json (&grad->end, ml_cJSON_GetObjectItem (obj, "end")))
		return false;

	grad->direction = ml_direction_parse_json (ml_cJSON_GetObjectItem (obj, "direction"));
	if (grad->direction == ML_INVALID)
		return false;

	return true;
}

void
ml_gradient_step_s (const MlGradient *grad, unsigned step, unsigned n_steps, MlColor *dest)
{
	if_unlikely (grad == NULL || dest == NULL)
		return;

	if (n_steps < 3 || step >= n_steps)
		return;

	double frac = (double)step / (n_steps - 1);

	dest->red   = (grad->start.red   * frac) + (grad->end.red   * (1.0f-frac));
	dest->green = (grad->start.green * frac) + (grad->end.green * (1.0f-frac));
	dest->blue  = (grad->start.blue  * frac) + (grad->end.blue  * (1.0f-frac));
	dest->alpha = (grad->start.alpha * frac) + (grad->end.alpha * (1.0f-frac));
}
