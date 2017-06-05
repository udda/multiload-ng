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


bool
ml_color_hsv2rgb (MlColor *rgb, int hue, int saturation, int value, int alpha)
{
	if_unlikely (rgb == NULL)
		return false;

	if_unlikely (saturation < 0 || saturation > 100)
		return false;

	if_unlikely (value < 0 || value > 100)
		return false;

	if_unlikely (alpha <= 0 || alpha > 255)
		return false;

	rgb->alpha = alpha;

	if (value == 0) {
		rgb->red = 0;
		rgb->green = 0;
		rgb->blue = 0;
		return true;
	}

	if (saturation == 0) {
		rgb->red = rgb->green = rgb->blue = ((0xFF * value) / 100) & 0xFF;
		return true;
	}

	hue %= 360;

	int i = hue / 60;

	double f = (double)hue / 60.0 - i;
	double s = (double)saturation / 100.0;
	uint8_t v = 255 * ((double)value / 100.0);

	double p = v * (1.0 - s);
	double q = v * (1.0 - (s * f));
	double t = v * (1.0 - (s * (1.0 - f)));


	switch (i) {
		case 0:
			rgb->red = v;
			rgb->green = t;
			rgb->blue = p;
			break;

		case 1:
			rgb->red = q;
			rgb->green = v;
			rgb->blue = p;
			break;

		case 2:
			rgb->red = p;
			rgb->green = v;
			rgb->blue = t;
			break;

		case 3:
			rgb->red = p;
			rgb->green = q;
			rgb->blue = v;
			break;

		case 4:
			rgb->red = t;
			rgb->green = p;
			rgb->blue = v;
			break;

		default:
			rgb->red = v;
			rgb->green = p;
			rgb->blue = q;
			break;
	}

	return true;
}

uint8_t
ml_color_get_luminance (MlColor *c)
{
	if_unlikely (c == NULL)
		return 0;

	return (c->red*3 + c->green*4 + c->blue) >> 3;
}

size_t
ml_color_sizeof (MlColor *c)
{
	if_unlikely (c == NULL)
		return 0;

	return sizeof (MlColor);
}

bool
ml_color_parse (MlColor *c, const char *def)
{
	if_unlikely (c == NULL || def == NULL)
		return false;

	// perform conversion in a temporary buffer
	MlColor tmp;

	if (ml_string_matches (def, "^#[0-9a-f]{8}$", false)) {
		// pattern: #RRGGBB
		if (4 != sscanf (def, "#%02"SCNx8"%02"SCNx8"%02"SCNx8"%02"SCNx8, &tmp.alpha, &tmp.red, &tmp.green, &tmp.blue))
			return false;

	} else if (ml_string_matches (def, "^#[0-9a-f]{6}$", false)) {
		// pattern: #AARRGGBB
		if (3 != sscanf (def, "#%02"SCNx8"%02"SCNx8"%02"SCNx8, &tmp.red, &tmp.green, &tmp.blue))
			return false;
		tmp.alpha = 0xFF;

	} else if (ml_string_matches (def, "^rgb\\s*\\(\\s*[0-9]+\\s*,\\s*[0-9]+\\s*,\\s*[0-9]+\\s*\\)$", false)) {
		// pattern: rgb(R,G,B)
		if (3 != sscanf (def, "%*[^0-9]%"SCNu8"%*[^0-9]%"SCNu8"%*[^0-9]%"SCNu8, &tmp.red, &tmp.green, &tmp.blue))
			return false;
		tmp.alpha = 0xFF;

	} else if (ml_string_matches (def, "^rgba\\s*\\(\\s*[0-9]+\\s*,\\s*[0-9]+\\s*,\\s*[0-9]+\\s*,\\s*[0-9]+\\s*\\)$", false)) {
		// pattern: rgba(R,G,B,A)
		if (4 != sscanf (def, "%*[^0-9]%"SCNu8"%*[^0-9]%"SCNu8"%*[^0-9]%"SCNu8"%*[^0-9]%"SCNu8, &tmp.red, &tmp.green, &tmp.blue, &tmp.alpha))
			return false;

	} else if (ml_string_matches (def, "^hsv\\s*\\(\\s*[0-9]+\\s*,\\s*[0-9]+\\s*,\\s*[0-9]+\\s*\\)$", false)) {
		// pattern: hsv(H,S,V)
		int h, s, v;
		if (3 != sscanf (def, "%*[^0-9]%d%*[^0-9]%d%*[^0-9]%d", &h, &s, &v))
			return false;

		if (!ml_color_hsv2rgb (&tmp, h, s, v, 0xFF))
			return false;

	} else if (ml_string_matches (def, "^hsva\\s*\\(\\s*[0-9]+\\s*,\\s*[0-9]+\\s*,\\s*[0-9]+\\s*,\\s*[0-9]+\\s*\\)$", false)) {
		// pattern: hsva(H,S,V,A)
		int h, s, v, a;
		if (4 != sscanf (def, "%*[^0-9]%d%*[^0-9]%d%*[^0-9]%d%*[^0-9]%d", &h, &s, &v, &a))
			return false;

		if (!ml_color_hsv2rgb (&tmp, h, s, v, a))
			return false;

	} else {
		// no recognized pattern
		return false;
	}

	// if conversion succeeded, copy values into returned MlColor
	memcpy (c, &tmp, sizeof(MlColor));
	return true;
}

cJSON *
ml_color_to_json (const MlColor *c)
{
	if_unlikely (c == NULL)
		return NULL;

	char buf[10];
	snprintf (buf, sizeof(buf), "#%02"PRIx8"%02"PRIx8"%02"PRIx8"%02"PRIx8, c->alpha, c->red, c->green, c->blue);

	return cJSON_CreateString (buf);
}

bool
ml_color_parse_json (MlColor *c, cJSON *obj)
{
	if (c == NULL || obj == NULL)
		return false;

	return ml_color_parse (c, obj->valuestring);
}

void
ml_color_overlay (MlColor *base, const MlColor *over)
{
	/* Here is the original implementation, for reference purpose.
	 * It's been replaced because it contains many divisions and double values, so it's slow.
	 * New implementation is faster and should behave exactly the same.
	 *
	 * double baf = (double)base->alpha / 0xFF;
	 * double oaf = (double)over->alpha / 0xFF;
	 * double out_alpha = baf + oaf * (1-baf);
	 *
	 * base->red   = ((over->red   * oaf) + (base->red   * baf * (1.0 - oaf))) / out_alpha;
	 * base->green = ((over->green * oaf) + (base->green * baf * (1.0 - oaf))) / out_alpha;
	 * base->blue  = ((over->blue  * oaf) + (base->blue  * baf * (1.0 - oaf))) / out_alpha;
	 * base->alpha = 255 * out_alpha;
	 */

	// arguments are guaranteed to be not NULL
	if (over->alpha == 0)
		return;

	// opaque over or transparent base: direct copy (special case that improves performance, but below formula is still valid)
	if (base->alpha == 0 || over->alpha == 0xFF) {
		memcpy (base, over, sizeof(MlColor));
		return;
	}

	int h = (base->alpha * over->alpha) / 255;
	int d = base->alpha + over->alpha - h;

	base->alpha = d;
	base->red   = ((over->red   * over->alpha) + (base->red   * base->alpha) - (base->red   * h)) / d;
	base->green = ((over->green * over->alpha) + (base->green * base->alpha) - (base->green * h)) / d;
	base->blue  = ((over->blue  * over->alpha) + (base->blue  * base->alpha) - (base->blue  * h)) / d;
}
