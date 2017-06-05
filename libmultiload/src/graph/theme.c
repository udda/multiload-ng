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


static bool
_fill_color (const MlGraphThemeElement *gte, MlColor *c, int hue)
{
	if_unlikely (gte == NULL || c == NULL || hue < 0)
		return false;

	if (gte->hue & ML_RELATIVE_HUE)
		hue += gte->hue & (~ML_RELATIVE_HUE);
	else
		hue = gte->hue & (~ML_RELATIVE_HUE);

	return ml_color_hsv2rgb (c, hue, gte->saturation, gte->value, gte->alpha);
}

static MlGraphStyle *
ml_graph_theme_generator_default (const MlGraphTypeInterface *iface, const MlGraphTheme *theme)
{
	if_unlikely (iface == NULL || theme == NULL)
		return NULL;

	bool success = true;

	MlGraphStyle *style = ml_new (MlGraphStyle);
	success &= _fill_color (&theme->background_gradient_start, &style->background_gradient.start, iface->hue);
	success &= _fill_color (&theme->background_gradient_end, &style->background_gradient.end, iface->hue);
	style->background_gradient.direction = theme->background_direction;

	success &= _fill_color (&theme->border_color, &style->border_color, iface->hue);
	success &= _fill_color (&theme->error_color, &style->error_color, iface->hue);

	style->n_data_colors = iface->n_data;
	style->data_colors = ml_new_n (MlColor, style->n_data_colors);

	for (int i = 0; i < style->n_data_colors; i++)
		success &= _fill_color (&theme->data_colors[i], &style->data_colors[i], iface->hue);

	if_unlikely (!success) {
		ml_graph_style_destroy (style);
		return NULL;
	}

	return style;
}


MlGraphStyle *
ml_graph_theme_generate_style (const char *theme_name, const MlGraphTypeInterface *iface)
{
	if_unlikely (theme_name == NULL || iface == NULL)
		return NULL;

	if (ml_string_equals (theme_name, "default", false))
		return ml_graph_theme_generator_default (iface, &ML_THEME_default);

	if (ml_string_equals (theme_name, "ambiance", false))
		return ml_graph_theme_generator_default (iface, &ML_THEME_Ubuntu_Ambiance);

	if (ml_string_equals (theme_name, "radiance", false))
		return ml_graph_theme_generator_default (iface, &ML_THEME_Ubuntu_Radiance);

	if (ml_string_equals (theme_name, "linuxmint", false))
		return ml_graph_theme_generator_default (iface, &ML_THEME_Linux_Mint);

	if (ml_string_equals (theme_name, "arc", false))
		return ml_graph_theme_generator_default (iface, &ML_THEME_Arc);

	// fallback
	return ml_graph_theme_generator_default (iface, &ML_THEME_default);
}
