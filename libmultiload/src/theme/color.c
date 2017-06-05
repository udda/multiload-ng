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


struct _MlThemeColor {
	MlThemeColorMode mode;

	/* Depending on "mode" field above, these values can be either:
	 * HSV:   Hue [-180, 180],   Saturation [0, 100],   Value [0, 100]
	 * RGB:   Red [   0, 255],   Green      [0, 255],   Blue  [0, 255] */
	int16_t values[3];

	/* Whether one or more values are relative numbers (to be added/subtracted
	 * from current values). In HSV mode, only hue can be relative */
	bool relative[3];

	uint8_t alpha; // [0, 255]
};


bool
ml_graph_theme_color_fill (const MlThemeColor *gtc, MlColor *c, const MlGraphTypeInterface *iface)
{
	if_unlikely (gtc == NULL || c == NULL || iface == NULL)
		return false;

	switch (gtc->mode) {
		case ML_GRAPH_THEME_COLOR_HSV:
			return ml_color_hsv2rgb (c,
				gtc->relative[0] ? (gtc->values[0] + iface->hue) : gtc->values[0],
				gtc->values[1],
				gtc->values[2],
				gtc->alpha);

		case ML_GRAPH_THEME_COLOR_RGB:
			if (gtc->relative[0])
				c->red += gtc->values[0];
			else
				c->red = gtc->values[0];

			if (gtc->relative[1])
				c->green += gtc->values[1];
			else
				c->green = gtc->values[1];

			if (gtc->relative[2])
				c->blue += gtc->values[2];
			else
				c->blue = gtc->values[2];

			c->alpha = gtc->alpha;

			return true;

		default:
			return false;
	}
}
