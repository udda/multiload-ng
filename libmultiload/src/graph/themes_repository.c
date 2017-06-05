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


#define HSVA(h,s,v,a)		{ .hue = (h), .saturation = (s), .value = (v), .alpha = (a) }
#define HSV(h,s,v)			HSVA((h),(s),(v),(0xFF))

#define rHSVA(h,s,v,a)		HSVA(((h) | ML_RELATIVE_HUE),(s),(v),(a))
#define rHSV(h,s,v)			HSV (((h) | ML_RELATIVE_HUE),(s),(v))



const MlGraphTheme ML_THEME_default = {
	.background_gradient_start	= rHSV (0, 50, 15),
	.background_gradient_end	= rHSV (0, 0, 0),
	.background_direction		= ML_DIRECTION_S,

	.border_color				= rHSV (0, 100, 50),
	.error_color				= HSV  (0, 60, 100),

	.data_colors =	{			  rHSV (0, 96, 59),
								  rHSV (0, 69, 90),
								  rHSV (0, 25, 100),
								  rHSV (0, 100, 40)		}
};

const MlGraphTheme ML_THEME_Ubuntu_Ambiance = {
	.background_gradient_start	= HSV (319, 79, 19),
	.background_gradient_end	= HSV (319, 79, 19),
	.background_direction		= ML_DIRECTION_S,

	.border_color				= HSV (0, 0, 22),
	.error_color				= HSV (337, 100, 100),

	.data_colors =	{			  HSV (24, 86, 91),
								  HSV (24, 86, 91),
								  HSV (24, 86, 91),
								  HSV (24, 86, 91)	}
};

const MlGraphTheme ML_THEME_Ubuntu_Radiance = {
	.background_gradient_start	= HSV (0, 0, 91),
	.background_gradient_end	= HSV (0, 0, 91),
	.background_direction		= ML_DIRECTION_S,

	.border_color				= HSV (0, 0, 84),
	.error_color				= HSV (337, 100, 100),

	.data_colors =	{			  HSV (24, 86, 91),
								  HSV (24, 86, 91),
								  HSV (24, 86, 91),
								  HSV (24, 86, 91)	}
};

const MlGraphTheme ML_THEME_Linux_Mint = {
	.background_gradient_start	= HSV (0, 0, 28),
	.background_gradient_end	= HSV (0, 0, 22),
	.background_direction		= ML_DIRECTION_S,

	.border_color				= HSV (0, 0, 24),
	.error_color				= HSV (334, 50, 75),

	.data_colors =	{			  HSV (85, 50, 75),
								  HSV (85, 50, 75),
								  HSV (85, 50, 75),
								  HSV (85, 50, 75)	}
};

const MlGraphTheme ML_THEME_Arc = {
	.background_gradient_start	= HSV (227, 24, 29),
	.background_gradient_end	= HSV (227, 24, 29),
	.background_direction		= ML_DIRECTION_S,

	.border_color				= HSV (220, 25, 14),
	.error_color				= HSV (334, 84, 89),

	.data_colors =	{			  HSV (257, 84, 89),
								  HSV (257, 84, 89),
								  HSV (257, 84, 89),
								  HSV (257, 84, 89)	}
};
