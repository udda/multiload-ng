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

#ifndef ML_HEADER__THEME_COLOR_H__INCLUDED
#define ML_HEADER__THEME_COLOR_H__INCLUDED
ML_HEADER_BEGIN


typedef enum {
	ML_GRAPH_THEME_COLOR_RGB,
	ML_GRAPH_THEME_COLOR_HSV
} MlThemeColorMode;

typedef struct _MlThemeColor MlThemeColor;


bool
ml_graph_theme_color_fill (const MlThemeColor *gtc, MlColor *c, const MlGraphTypeInterface *iface);


ML_HEADER_END
#endif /* ML_HEADER__THEME_COLOR_H__INCLUDED */
