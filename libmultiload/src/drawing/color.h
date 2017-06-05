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

#ifndef ML_HEADER__DRAWING_COLOR_H__INCLUDED
#define ML_HEADER__DRAWING_COLOR_H__INCLUDED
ML_HEADER_BEGIN


// 32 bit ARGB pixel, arranged in 32 bit host integers (compatible with Cairo surfaces)
typedef struct {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	uint8_t	alpha;
	uint8_t	red;
	uint8_t	green;
	uint8_t	blue;

#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	uint8_t	blue;
	uint8_t	green;
	uint8_t	red;
	uint8_t	alpha;

#elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
	uint8_t	green;
	uint8_t	blue;
	uint8_t	alpha;
	uint8_t	red;

#else
#error "Unsupported host byte order."
#endif
} MlColor;


bool
ml_color_hsv2rgb (MlColor *rgb, int hue, int saturation, int value, int alpha)
ML_FN_WARN_UNUSED_RESULT;

uint8_t
ml_color_get_luminance (MlColor *c);

size_t
ml_color_sizeof (MlColor *c)
ML_FN_SIZEOF;

bool
ml_color_parse (MlColor *c, const char *def);

cJSON *
ml_color_to_json (const MlColor *c)
ML_FN_COLD;

bool
ml_color_parse_json (MlColor *c, cJSON *obj)
ML_FN_WARN_UNUSED_RESULT;

void
ml_color_overlay (MlColor *base, const MlColor *over)
ML_FN_NONNULL(1,2) ML_FN_HOT;


ML_HEADER_END
#endif /* ML_HEADER__DRAWING_COLOR_H__INCLUDED */
