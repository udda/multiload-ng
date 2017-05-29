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

#ifndef ML_HEADER__GRAPH_STYLE_H__INCLUDED
#define ML_HEADER__GRAPH_STYLE_H__INCLUDED
ML_HEADER_BEGIN


typedef struct {
	MlGradient	background_gradient;
	MlColor 	border_color;
	MlColor 	error_color; // alpha component is not used

	int			n_data_colors;
	MlColor* 	data_colors;
} MlGraphStyle;


MlGraphStyle *
ml_graph_style_new_from_json (cJSON *obj);

size_t
ml_graph_style_sizeof (MlGraphStyle *style)
ML_FN_SIZEOF;

void
ml_graph_style_destroy (MlGraphStyle *style);

cJSON *
ml_graph_style_to_json (const MlGraphStyle *style)
ML_FN_COLD;


ML_HEADER_END
#endif /* ML_HEADER__GRAPH_STYLE_H__INCLUDED */
