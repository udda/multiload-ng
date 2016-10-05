/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *               1997 The Free Software Foundation
 *                    (Authors: Tim P. Gerla, Martin Baulig, Todd Kulesza)
 *
 * This file is part of multiload-ng.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "colors.h"

typedef struct { // first revision
	char name[24];
	GdkRGBA colors[7][7];
} MultiloadColorSchemeV1;

typedef struct { // added parm, 1 color
	char name[24];
	GdkRGBA colors[8][7];
} MultiloadColorSchemeV2;

typedef struct { // parm 1 color -> 4 colors
	char name[24];
	GdkRGBA colors[8][7];
} MultiloadColorSchemeV3;

typedef struct { // added xpm_data
	char name[24];
	char **xpm_data;
	GdkRGBA colors[8][7];
} MultiloadColorSchemeV4;

typedef MultiloadColorScheme MultiloadColorSchemeV5;


MultiloadColorSchemeStatus
multiload_color_scheme_parse (gpointer data, size_t length, guint32 version, MultiloadColorScheme *scheme)
{
	// Remember to avoid constants in this function as they might have changed. Type numbers directly.

	if (version == MULTILOAD_COLOR_SCHEME_VERSION) { // last version
		if (length != sizeof(MultiloadColorScheme))
			return MULTILOAD_COLOR_SCHEME_STATUS_WRONG_FORMAT;
		memcpy(scheme, data, length);
	}

	else if (version == 4) {
		if (length != sizeof(MultiloadColorSchemeV4))
			return MULTILOAD_COLOR_SCHEME_STATUS_WRONG_FORMAT;
		/* CONVERT V4 TO V5
		 * Changes: removed Shared color from Memory graph
		 */

		GdkRGBA *mem_colors = ((MultiloadColorSchemeV4*)data)->colors[1];

		// shift Memory graph colors 1 position left, starting from the second
		memmove(&mem_colors[1], &mem_colors[2], 5*sizeof(GdkRGBA));

		return multiload_color_scheme_parse(data, sizeof(MultiloadColorSchemeV5), version+1, scheme);
	}

	else if (version == 3) {
		if (length != sizeof(MultiloadColorSchemeV3))
			return MULTILOAD_COLOR_SCHEME_STATUS_WRONG_FORMAT;
		/* CONVERT V3 TO V4
		 * Changes: added field xpm_data
		 */

		MultiloadColorSchemeV3 *old = (MultiloadColorSchemeV3*)data;
		MultiloadColorSchemeV4 *new = g_malloc0(sizeof(MultiloadColorSchemeV4));

		memcpy(new->name, old->name, 24);
		memcpy(new->colors, old->colors, sizeof(new->colors));

		MultiloadColorSchemeStatus ret = multiload_color_scheme_parse(new, sizeof(MultiloadColorSchemeV4), version+1, scheme);

		g_free(new);
		return ret;
	}

	else if (version == 2) {
		if (length != sizeof(MultiloadColorSchemeV2))
			return MULTILOAD_COLOR_SCHEME_STATUS_WRONG_FORMAT;
		/* CONVERT V2 TO V3
		 * Changes: Parametric graph from 1 to 4 colors
		 */

		GdkRGBA *parm_colors = ((MultiloadColorSchemeV2*)data)->colors[7];

		// shift border and BG color 3 positions right
		memmove(&parm_colors[4], &parm_colors[1], 3*sizeof(GdkRGBA));
		// zero-set new colors
		memset(&parm_colors[1], 0, 3*sizeof(GdkRGBA));

		return multiload_color_scheme_parse(data, sizeof(MultiloadColorSchemeV3), version+1, scheme);
	}

	else if (version == 1) {
		if (length != sizeof(MultiloadColorSchemeV1))
			return MULTILOAD_COLOR_SCHEME_STATUS_WRONG_FORMAT;
		/* CONVERT V1 TO V2
		 * Changes: Added Parametric graph, only one color
		 */

		// no action required: new data is at the end of the buffer. Just return a new buffer of proper size
		guchar *buf = g_malloc(sizeof(MultiloadColorSchemeV2));
		memcpy(buf, data, sizeof(MultiloadColorSchemeV2));

		MultiloadColorSchemeStatus ret = multiload_color_scheme_parse(buf, sizeof(MultiloadColorSchemeV2), version+1, scheme);

		g_free(buf);
		return ret;
	}

	else {
		return MULTILOAD_COLOR_SCHEME_STATUS_WRONG_VERSION;
	}

	return MULTILOAD_COLOR_SCHEME_STATUS_VALID;
}
