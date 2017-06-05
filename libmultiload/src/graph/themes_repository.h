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

#ifndef ML_HEADER__GRAPH_THEMES_REPOSITORY_H__INCLUDED
#define ML_HEADER__GRAPH_THEMES_REPOSITORY_H__INCLUDED
ML_HEADER_BEGIN


typedef struct {
	int16_t hue;		// -180 ... 180
	int16_t saturation;	//    0 ... 100
	int16_t value;		//    0 ... 100
	int16_t alpha;		//    0 ... 255
} MlGraphThemeElement;

typedef struct {
	MlGraphThemeElement background_gradient_start;
	MlGraphThemeElement background_gradient_end;
	MlDirection			background_direction;

	MlGraphThemeElement border_color;
	MlGraphThemeElement error_color;
	MlGraphThemeElement data_colors[4];
} MlGraphTheme;


#define ML_RELATIVE_HUE ((int16_t)0x4000)

const MlGraphTheme ML_THEME_default;
const MlGraphTheme ML_THEME_Ubuntu_Ambiance;
const MlGraphTheme ML_THEME_Ubuntu_Radiance;
const MlGraphTheme ML_THEME_Linux_Mint;
const MlGraphTheme ML_THEME_Arc;


ML_HEADER_END
#endif /* ML_HEADER__GRAPH_THEMES_REPOSITORY_H__INCLUDED */
