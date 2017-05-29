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


MlGraphStyle *
ml_graph_style_new_from_json (cJSON *obj)
{
	if_unlikely (obj == NULL)
		return NULL;

	MlGraphStyle *style = ml_new (MlGraphStyle);

	if (!ml_gradient_parse_json (&style->background_gradient, ml_cJSON_GetObjectItem (obj, "background"))) {
		ml_graph_style_destroy (style);
		return NULL;
	}

	if (!ml_color_parse_json (&style->border_color, ml_cJSON_GetObjectItem (obj, "border-color"))) {
		ml_graph_style_destroy (style);
		return NULL;
	}

	if (!ml_color_parse_json (&style->error_color, ml_cJSON_GetObjectItem (obj, "error-color"))) {
		ml_graph_style_destroy (style);
		return NULL;
	}

	cJSON *obj_datacolors = ml_cJSON_GetObjectItem (obj, "data-colors");
	if (obj_datacolors == NULL || !cJSON_IsArray(obj_datacolors)) {
		ml_graph_style_destroy (style);
		return NULL;
	}
	style->n_data_colors = cJSON_GetArraySize (obj_datacolors);
	if (style->n_data_colors < 1) {
		ml_graph_style_destroy (style);
		return NULL;
	}

	style->data_colors = ml_new_n (MlColor, style->n_data_colors);
	for (int i = 0; i < style->n_data_colors; i++) {
		cJSON *child = cJSON_GetArrayItem (obj_datacolors, i);
		if (child == NULL)
			continue;

		if (!ml_color_parse_json (&style->data_colors[i], child)) {
			ml_graph_style_destroy (style);
			return NULL;
		}	
	}

	return style;
}

size_t
ml_graph_style_sizeof (MlGraphStyle *style)
{
	if_unlikely (style == NULL)
		return 0;

	size_t size = sizeof (MlGraphStyle);
	size += sizeof(MlColor) * style->n_data_colors;

	return size;
}

void
ml_graph_style_destroy (MlGraphStyle *style)
{
	if_unlikely (style == NULL)
		return;

	if_likely (style->data_colors != NULL)
		free (style->data_colors);

	free (style);
}

cJSON *
ml_graph_style_to_json (const MlGraphStyle *style)
{
	if_unlikely (style == NULL)
		return NULL;

	cJSON* obj = cJSON_CreateObject ();

	cJSON_AddItemToObject (obj, "background",		ml_gradient_to_json (&style->background_gradient));
	cJSON_AddItemToObject (obj, "border-color",		ml_color_to_json (&style->border_color));
	cJSON_AddItemToObject (obj, "error-color",		ml_color_to_json (&style->error_color));

	cJSON* obj_datacolors = cJSON_CreateArray ();
	for (int i = 0; i < style->n_data_colors; i++)
		cJSON_AddItemToArray (obj_datacolors, ml_color_to_json (&style->data_colors[i]));

	cJSON_AddItemToObject (obj, "data-colors",	obj_datacolors);

	return obj;
}
