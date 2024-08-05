/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *                    The Free Software Foundation
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


#include <config.h>

#include <errno.h>
#include <stdlib.h>
#include <glib/gi18n-lib.h>

#include "colors.h"
#include "gtk-compat.h"
#include "multiload-config.h"
#include "preferences.h"


#include "color-scheme-icons.inc"

static const gchar magic_header[MULTILOAD_COLOR_SCHEME_HEADER_SIZE] = "MULTILOAD-NG";

guint
multiload_colors_get_extra_index(guint i, MultiloadExtraColor col)
{
	g_assert_cmpuint(col, >=, 0);
	g_assert_cmpuint(col,  <, EXTRA_COLORS);
	return multiload_config_get_num_colors(i) - EXTRA_COLORS + col;
}

static void
multiload_color_scheme_init (MultiloadColorSchemeFileHeader *header)
{
	memcpy(header->magic, magic_header, sizeof(magic_header));
	header->version = MULTILOAD_COLOR_SCHEME_VERSION;
	memset(header->reserved, 0, sizeof(header->reserved));
}

static MultiloadColorSchemeStatus
multiload_color_scheme_validate (MultiloadColorSchemeFileHeader *header)
{
	if (memcmp(header->magic, magic_header, MULTILOAD_COLOR_SCHEME_HEADER_SIZE) != 0)
		return MULTILOAD_COLOR_SCHEME_STATUS_WRONG_FORMAT;

	return MULTILOAD_COLOR_SCHEME_STATUS_VALID;
}


void
multiload_color_scheme_fill (MultiloadColorScheme *scheme, MultiloadPlugin *ma)
{
	guint i;

	for (i=0; i<GRAPH_MAX; i++)
		memcpy(scheme->colors[i], &ma->graph_config[i].colors, sizeof(scheme->colors[i]));
}


void
multiload_color_scheme_apply (const MultiloadColorScheme *scheme, MultiloadPlugin *ma)
{
	guint i;

	for (i=0; i<GRAPH_MAX; i++)
		multiload_color_scheme_apply_single(scheme, ma, i);
}

void
multiload_color_scheme_apply_single (const MultiloadColorScheme *scheme, MultiloadPlugin *ma, guint i)
{
	memcpy(&ma->graph_config[i].colors, scheme->colors[i], sizeof(scheme->colors[i]));
}

gboolean
multiload_color_scheme_to_file(const gchar *filename, MultiloadPlugin *ma)
{
	MultiloadColorSchemeFileHeader header;
	MultiloadColorScheme scheme;
	size_t wr;

	multiload_color_scheme_init(&header);
	multiload_color_scheme_fill(&scheme, ma);

	// at the moment, name field is unused in files
	strcpy(scheme.name, "User scheme");

	FILE *f = fopen(filename, "wb");
	if (f == NULL)
		return FALSE;

	wr = fwrite(&header, sizeof(header), 1, f);
	if (wr != 1) {
		fclose(f);
		return FALSE;
	}

	wr = fwrite(&scheme, sizeof(scheme), 1, f);
	if (wr != 1) {
		fclose(f);
		return FALSE;
	}

	fclose(f);
	return TRUE;
}


MultiloadColorSchemeStatus
multiload_color_scheme_from_file(const gchar *filename, MultiloadPlugin *ma)
{
	MultiloadColorSchemeFileHeader header;
	MultiloadColorScheme scheme;

	MultiloadColorSchemeStatus ret;
	size_t rd;
	guchar buffer[20480]; // 20 KiB oversized buffer - just in case

	FILE *f = fopen(filename, "rb");
	if (f == NULL)
		return FALSE;

	rd = fread (&header, sizeof(header), 1, f);
	if (rd != 1) {
		fclose(f);
		return MULTILOAD_COLOR_SCHEME_STATUS_WRONG_FORMAT;
	}

	ret = multiload_color_scheme_validate(&header);
	if (ret != MULTILOAD_COLOR_SCHEME_STATUS_VALID) {
		fclose(f);
		return ret;
	}

	rd = fread(buffer, 1, sizeof(buffer), f);
	fclose(f);
	if (rd < 1)
		return MULTILOAD_COLOR_SCHEME_STATUS_WRONG_FORMAT;

	g_debug("[colors] Loading color scheme from '%s', version: %d", filename, header.version);

	ret = multiload_color_scheme_parse(buffer, rd, header.version, &scheme);
	if (ret != MULTILOAD_COLOR_SCHEME_STATUS_VALID)
		return ret;

	multiload_color_scheme_apply(&scheme, ma);
	return MULTILOAD_COLOR_SCHEME_STATUS_VALID;
}

const MultiloadColorScheme*
multiload_color_scheme_find_by_name (const gchar *name)
{
	guint i;

	if (name == NULL)
		return NULL;

	for (i=0; multiload_builtin_color_schemes[i].name[0] != '\0'; i++) {
		if (strcmp(multiload_builtin_color_schemes[i].name, name) == 0)
			return &multiload_builtin_color_schemes[i];
	}
	return NULL;
}


// Converts graph colors into a string of the form "#aarrggbb,#aarrggbb,..."
gchar *
multiload_colors_to_string(MultiloadPlugin *ma, guint graph_index)
{
	guint ncolors;
	GdkRGBA *c;
	guint j;
	int rc;
	char *list, *lp;

	ncolors = multiload_config_get_num_colors(graph_index);
	list = g_new0(char, 10*ncolors);

	for ( j=0, lp=list; j<ncolors; j++, lp+=10 ) {
		c = &ma->graph_config[graph_index].colors[j];

		rc = snprintf(lp, 10, "#%02X%02X%02X%02X", (guint8)(c->alpha * 255), (guint8)(c->red * 255), (guint8)(c->green * 255), (guint8)(c->blue * 255));
		g_assert_cmpint(rc, ==, 9);

		lp[9] = ',';
	}
	list[(10*ncolors)-1] = '\0';

	return list;
}

// Set graph colors from a string, as produced by multiload_colors_to_string
gboolean
multiload_colors_from_string(MultiloadPlugin *ma, guint graph_index, const char *list)
{
	guint ncolors;
	GdkRGBA *colors;
	gboolean success;
	double alpha;
	guint j;
	size_t pos;
	char gspec[10];
	char *lp, *tmp;

	memset(gspec, 0, 10);
	if (G_LIKELY(list != NULL)) {
		ncolors = multiload_config_get_num_colors(graph_index);
		colors = ma->graph_config[graph_index].colors;

		for ( j=0, lp=(char*)list; j<ncolors; j++, lp+=10 ) {

			// Check the length of the list item
			if ((tmp = strchr(lp, ',')) != NULL) {
				pos = tmp - lp;
			} else if (j==ncolors-1) {
				pos = strlen(lp);
			} else {
				g_debug("[multiload_colors_from_string] Incomplete color list for graph '%s'", graph_types[graph_index].name);
				success = FALSE;
				break;
			}

			if (G_UNLIKELY(pos!=9 && pos!=7)) {
				g_debug("[multiload_colors_from_string] Wrong length of graph '%s' color list (%zu)", graph_types[graph_index].name, pos);
				success = FALSE;
				break;
			}

			strncpy(gspec, lp, pos);
			if (pos == 7) { // may be a standard RGB hex string, fallback to standard parse
				if (FALSE == gdk_rgba_parse(&colors[j], gspec)) {
					g_debug("[multiload_colors_from_string] Cannot parse color for of graph '%s' (%s)", graph_types[graph_index].name, gspec);
					success = FALSE;
					break;
				}
				colors[j].alpha = 1.0;
			} else {
				// alpha part
				gspec[0] = gspec[1];
				gspec[1] = gspec[2];
				gspec[2] = 0;
				errno = 0;
				alpha = (double)strtol(gspec, NULL, 16) / 255.0;
				if (errno) { // error in strtol, set alpha=max
					alpha = 1.0;
				}

				// color part
				gspec[2] = '#';
				if (FALSE == gdk_rgba_parse(&colors[j], gspec+2)) {
					g_debug("[multiload_colors_from_string] Cannot parse color for of graph '%s' (%s)", graph_types[graph_index].name, gspec+2);
					success = FALSE;
					break;
				} else {
					colors[j].alpha = alpha;
				}
			}
		}
		success = TRUE;
	} else {
		success = FALSE;
	}

	if (success) {
		//ignore alpha value of last three colors (background top, background bottom, border)
		colors[ncolors-1].alpha = 1.0;
		colors[ncolors-2].alpha = 1.0;
		colors[ncolors-3].alpha = 1.0;
	} else {
		multiload_colors_default(ma, graph_index);
	}

	if (success)
		g_debug("[multiload_colors_from_string] Loaded colors for graph '%s'", graph_types[graph_index].name);

	return success;
}

void
multiload_colors_default(MultiloadPlugin *ma, guint graph_index)
{
	multiload_color_scheme_apply_single(&multiload_builtin_color_schemes[0], ma, graph_index);
}


const MultiloadColorScheme multiload_builtin_color_schemes[] = {
	{ DEFAULT_COLOR_SCHEME, color_scheme_default_xpm,
			{  { // CPU  - hue: 196
				HEX_TO_RGBA(036F96, FF),		// User
				HEX_TO_RGBA(48BDE6, FF),		// System
				HEX_TO_RGBA(BEEEFF, FF),		// Nice
				HEX_TO_RGBA(003040, FF),		// IOWait
				HEX_TO_RGBA(005D80, FF),		// Border
				HEX_TO_RGBA(132126, FF),		// Background (top)
				HEX_TO_RGBA(000000, FF)			// Background (bottom)
			}, { // MEM  - hue: 151
				HEX_TO_RGBA(03964F, FF),		// User
				HEX_TO_RGBA(03964F, FF),		// Shared
				HEX_TO_RGBA(43D18D, FF),		// Buffers
				HEX_TO_RGBA(BFFFE0, FF),		// Cached
				HEX_TO_RGBA(008042, FF),		// Border
				HEX_TO_RGBA(13261D, FF),		// Background (top)
				HEX_TO_RGBA(000000, FF)			// Background (bottom)
			}, { // NET  - hue: 53
				HEX_TO_RGBA(E2CC05, FF),		// In
				HEX_TO_RGBA(696018, FF),		// Out
				HEX_TO_RGBA(FFF7B1, FF),		// Local
				HEX_TO_RGBA(807100, FF),		// Border
				HEX_TO_RGBA(262413, FF),		// Background (top)
				HEX_TO_RGBA(000000, FF)			// Background (bottom)
			}, { // SWAP - hue: 278
				HEX_TO_RGBA(9C43D1, FF),		// Used
				HEX_TO_RGBA(510080, FF),		// Border
				HEX_TO_RGBA(1F1326, FF),		// Background (top)
				HEX_TO_RGBA(000000, FF)			// Background (bottom)
			}, { // LOAD - hue: 0
				HEX_TO_RGBA(D14343, FF),		// Average
				HEX_TO_RGBA(800000, FF),		// Border
				HEX_TO_RGBA(261313, FF),		// Background (top)
				HEX_TO_RGBA(000000, FF)			// Background (bottom)
			}, { // DISK - hue: 31
				HEX_TO_RGBA(ED7A00, FF),		// Read
				HEX_TO_RGBA(FFAB73, FF),		// Write
				HEX_TO_RGBA(804200, FF),		// Border
				HEX_TO_RGBA(261D13, FF),		// Background (top)
				HEX_TO_RGBA(000000, FF)			// Background (bottom)
			}, { // TEMP hue: 310
				HEX_TO_RGBA(F049D5, FF),		// Value
				HEX_TO_RGBA(FFBEF2, FF),		// Critical
				HEX_TO_RGBA(80006B, FF),		// Border
				HEX_TO_RGBA(261323, FF),		// Background (top)
				HEX_TO_RGBA(000000, FF)			// Background (bottom)
			}, { // BAT hue: 94
				HEX_TO_RGBA(5B9C1A, FF),		// Charging
				HEX_TO_RGBA(A8E37B, FF),		// Discharging
				HEX_TO_RGBA(EAF9DB, FF),		// Critical level
				HEX_TO_RGBA(3D8000, FF),		// Border
				HEX_TO_RGBA(1C2613, FF),		// Background (top)
				HEX_TO_RGBA(000000, FF)			// Background (bottom)
			}, { // PARM hue: (gray)
				HEX_TO_RGBA(4C4C4C, FF),		// Result 1
				HEX_TO_RGBA(909090, FF),		// Result 2
				HEX_TO_RGBA(CCCCCC, FF),		// Result 3
				HEX_TO_RGBA(F3F3F3, FF),		// Result 4
				HEX_TO_RGBA(808080, FF),		// Border
				HEX_TO_RGBA(000000, FF),		// Background (top)
				HEX_TO_RGBA(000000, FF)			// Background (bottom)
			}
		}
	},

	{ "Tango", color_scheme_tango_xpm,
			{  { // CPU  - Tango Sky Blue
				HEX_TO_RGBA(204A87, FF),		// User
				HEX_TO_RGBA(3465A4, FF),		// System
				HEX_TO_RGBA(729FCF, FF),		// Nice
				HEX_TO_RGBA(274166, FF),		// IOWait
				HEX_TO_RGBA(2E3436, FF),		// Border
				HEX_TO_RGBA(888A85, FF),		// Background (top)
				HEX_TO_RGBA(555753, FF)			// Background (bottom)
			}, { // MEM  - Tango Chameleon
				HEX_TO_RGBA(3E6618, FF),		// User
				HEX_TO_RGBA(3E6618, FF),		// Shared
				HEX_TO_RGBA(73D216, FF),		// Buffers
				HEX_TO_RGBA(ACFF5C, FF),		// Cached
				HEX_TO_RGBA(2E3436, FF),		// Border
				HEX_TO_RGBA(888A85, FF),		// Background (top)
				HEX_TO_RGBA(555753, FF)			// Background (bottom)
			}, { // NET  - Tango Butter
				HEX_TO_RGBA(EDD400, FF),		// In
				HEX_TO_RGBA(C4A000, FF),		// Out
				HEX_TO_RGBA(FCE94F, FF),		// Local
				HEX_TO_RGBA(2E3436, FF),		// Border
				HEX_TO_RGBA(888A85, FF),		// Background (top)
				HEX_TO_RGBA(555753, FF)			// Background (bottom)
			}, { // SWAP - Tango Plum (dark)
				HEX_TO_RGBA(5C3566, FF),		// Used
				HEX_TO_RGBA(2E3436, FF),		// Border
				HEX_TO_RGBA(888A85, FF),		// Background (top)
				HEX_TO_RGBA(555753, FF)			// Background (bottom)
			}, { // LOAD - Tango Scarlet Red
				HEX_TO_RGBA(A40000, FF),		// Average
				HEX_TO_RGBA(2E3436, FF),		// Border
				HEX_TO_RGBA(888A85, FF),		// Background (top)
				HEX_TO_RGBA(555753, FF)			// Background (bottom)
			}, { // DISK - Tango Orange
				HEX_TO_RGBA(F57900, FF),		// Read
				HEX_TO_RGBA(CE5C00, FF),		// Write
				HEX_TO_RGBA(2E3436, FF),		// Border
				HEX_TO_RGBA(888A85, FF),		// Background (top)
				HEX_TO_RGBA(555753, FF)			// Background (bottom)
			}, { // TEMP - Tango Plum (light)
				HEX_TO_RGBA(AD7FA8, FF),		// Value
				HEX_TO_RGBA(75507B, FF),		// Critical
				HEX_TO_RGBA(2E3436, FF),		// Border
				HEX_TO_RGBA(888A85, FF),		// Background (top)
				HEX_TO_RGBA(555753, FF)			// Background (bottom)
			}, { // BAT - Tango Chameleon (a greener variant)
				HEX_TO_RGBA(4C7356, FF),		// Charging
				HEX_TO_RGBA(34DC5F, FF),		// Discharging
				HEX_TO_RGBA(C7EED1, FF),		// Critical level
				HEX_TO_RGBA(2E3436, FF),		// Border
				HEX_TO_RGBA(888A85, FF),		// Background (top)
				HEX_TO_RGBA(555753, FF)			// Background (bottom)
			}, { // PARM - Tango Aluminium (with some addition)
				HEX_TO_RGBA(2E3436, FF),		// Result 1
				HEX_TO_RGBA(707A7D, FF),		// Result 2
				HEX_TO_RGBA(93B1BA, FF),		// Result 3
				HEX_TO_RGBA(E6F3F7, FF),		// Result 4
				HEX_TO_RGBA(2E3436, FF),		// Border
				HEX_TO_RGBA(888A85, FF),		// Background (top)
				HEX_TO_RGBA(555753, FF)			// Background (bottom)
			}
		}
	},

	{ "Solarized Dark", color_scheme_solarized_dark_xpm,
			{  { // CPU  - Solarized Blue
				HEX_TO_RGBA(268BD2, FF),		// User
				HEX_TO_RGBA(657B83, FF),		// System
				HEX_TO_RGBA(839496, FF),		// Nice
				HEX_TO_RGBA(93A1A1, FF),		// IOWait
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(073642, FF),		// Background (top)
				HEX_TO_RGBA(002B36, FF)			// Background (bottom)
			}, { // MEM  - Solarized Green
				HEX_TO_RGBA(859900, FF),		// User
				HEX_TO_RGBA(859900, FF),		// Shared
				HEX_TO_RGBA(657B83, FF),		// Buffers
				HEX_TO_RGBA(839496, FF),		// Cached
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(073642, FF),		// Background (top)
				HEX_TO_RGBA(002B36, FF)			// Background (bottom)
			}, { // NET  - Solarized Yellow
				HEX_TO_RGBA(B58900, FF),		// In
				HEX_TO_RGBA(657B83, FF),		// Out
				HEX_TO_RGBA(839496, FF),		// Local
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(073642, FF),		// Background (top)
				HEX_TO_RGBA(002B36, FF)			// Background (bottom)
			}, { // SWAP - Solarized Violet
				HEX_TO_RGBA(6C71C4, FF),		// Used
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(073642, FF),		// Background (top)
				HEX_TO_RGBA(002B36, FF)			// Background (bottom)
			}, { // LOAD - Solarized Red
				HEX_TO_RGBA(DC322F, FF),		// Average
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(073642, FF),		// Background (top)
				HEX_TO_RGBA(002B36, FF)			// Background (bottom)
			}, { // DISK - Solarized Orange
				HEX_TO_RGBA(CB4B16, FF),		// Read
				HEX_TO_RGBA(657B83, FF),		// Write
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(073642, FF),		// Background (top)
				HEX_TO_RGBA(002B36, FF)			// Background (bottom)
			}, { // TEMP - Solarized Magenta
				HEX_TO_RGBA(D33682, FF),		// Value
				HEX_TO_RGBA(657B83, FF),		// Critical
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(073642, FF),		// Background (top)
				HEX_TO_RGBA(002B36, FF)			// Background (bottom)
			}, { // BAT - Solarized Cyan
				HEX_TO_RGBA(2AA198, FF),		// Charging
				HEX_TO_RGBA(657B83, FF),		// Discharging
				HEX_TO_RGBA(657B83, FF),		// Critical level
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(073642, FF),		// Background (top)
				HEX_TO_RGBA(002B36, FF)			// Background (bottom)
			}, { // PARM - Solarized Base01
				HEX_TO_RGBA(586E75, FF),		// Result 1
				HEX_TO_RGBA(657B83, FF),		// Result 2
				HEX_TO_RGBA(839496, FF),		// Result 3
				HEX_TO_RGBA(93A1A1, FF),		// Result 4
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(073642, FF),		// Background (top)
				HEX_TO_RGBA(002B36, FF)			// Background (bottom)
			}
		}
	},

	{ "Solarized Light", color_scheme_solarized_light_xpm,
			{  { // CPU  - Solarized Blue
				HEX_TO_RGBA(268BD2, FF),		// User
				HEX_TO_RGBA(657B83, FF),		// System
				HEX_TO_RGBA(839496, FF),		// Nice
				HEX_TO_RGBA(93A1A1, FF),		// IOWait
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(FDF6E3, FF),		// Background (top)
				HEX_TO_RGBA(EEE8D5, FF)			// Background (bottom)
			}, { // MEM  - Solarized Green
				HEX_TO_RGBA(859900, FF),		// User
				HEX_TO_RGBA(859900, FF),		// Shared
				HEX_TO_RGBA(657B83, FF),		// Buffers
				HEX_TO_RGBA(839496, FF),		// Cached
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(FDF6E3, FF),		// Background (top)
				HEX_TO_RGBA(EEE8D5, FF)			// Background (bottom)
			}, { // NET  - Solarized Yellow
				HEX_TO_RGBA(B58900, FF),		// In
				HEX_TO_RGBA(657B83, FF),		// Out
				HEX_TO_RGBA(839496, FF),		// Local
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(FDF6E3, FF),		// Background (top)
				HEX_TO_RGBA(EEE8D5, FF)			// Background (bottom)
			}, { // SWAP - Solarized Violet
				HEX_TO_RGBA(6C71C4, FF),		// Used
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(FDF6E3, FF),		// Background (top)
				HEX_TO_RGBA(EEE8D5, FF)			// Background (bottom)
			}, { // LOAD - Solarized Red
				HEX_TO_RGBA(DC322F, FF),		// Average
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(FDF6E3, FF),		// Background (top)
				HEX_TO_RGBA(EEE8D5, FF)			// Background (bottom)
			}, { // DISK - Solarized Orange
				HEX_TO_RGBA(CB4B16, FF),		// Read
				HEX_TO_RGBA(657B83, FF),		// Write
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(FDF6E3, FF),		// Background (top)
				HEX_TO_RGBA(EEE8D5, FF)			// Background (bottom)
			}, { // TEMP - Solarized Magenta
				HEX_TO_RGBA(D33682, FF),		// Value
				HEX_TO_RGBA(657B83, FF),		// Critical
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(FDF6E3, FF),		// Background (top)
				HEX_TO_RGBA(EEE8D5, FF)			// Background (bottom)
			}, { // BAT - Solarized Cyan
				HEX_TO_RGBA(2AA198, FF),		// Charging
				HEX_TO_RGBA(657B83, FF),		// Discharging
				HEX_TO_RGBA(657B83, FF),		// Critical level
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(FDF6E3, FF),		// Background (top)
				HEX_TO_RGBA(EEE8D5, FF)			// Background (bottom)
			}, { // PARM - Solarized Base01
				HEX_TO_RGBA(586E75, FF),		// Result 1
				HEX_TO_RGBA(657B83, FF),		// Result 2
				HEX_TO_RGBA(839496, FF),		// Result 3
				HEX_TO_RGBA(93A1A1, FF),		// Result 4
				HEX_TO_RGBA(586E75, FF),		// Border
				HEX_TO_RGBA(FDF6E3, FF),		// Background (top)
				HEX_TO_RGBA(EEE8D5, FF)			// Background (bottom)
			}
		}
	},

	{ "=====" },

	{ "Fruity", color_scheme_fruity_xpm, // http://allfreedesigns.com/bright-color-palettes/
			{  { // CPU  - Blueberries
				HEX_TO_RGBA(064490, FF),		// User
				HEX_TO_RGBA(2686E0, FF),		// System
				HEX_TO_RGBA(68CAFB, FF),		// Nice
				HEX_TO_RGBA(04235A, FF),		// IOWait
				HEX_TO_RGBA(404040, FF),		// Border
				HEX_TO_RGBA(B0B0E8, FF),		// Background (top)
				HEX_TO_RGBA(B4B4C2, FF)			// Background (bottom)
			}, { // MEM  - Kiwi
				HEX_TO_RGBA(55641F, FF),		// User
				HEX_TO_RGBA(55641F, FF),		// Shared
				HEX_TO_RGBA(789236, FF),		// Buffers
				HEX_TO_RGBA(9AB452, FF),		// Cached
				HEX_TO_RGBA(404040, FF),		// Border
				HEX_TO_RGBA(72511E, FF),		// Background (top)
				HEX_TO_RGBA(809758, FF)			// Background (bottom)
			}, { // NET  - Lemons
				HEX_TO_RGBA(EECB13, FF),		// In
				HEX_TO_RGBA(B08309, FF),		// Out
				HEX_TO_RGBA(FDF063, FF),		// Local
				HEX_TO_RGBA(404040, FF),		// Border
				HEX_TO_RGBA(EBDA82, FF),		// Background (top)
				HEX_TO_RGBA(A79364, FF)			// Background (bottom)
			}, { // SWAP - Grapes
				HEX_TO_RGBA(821532, FF),		// Used
				HEX_TO_RGBA(404040, FF),		// Border
				HEX_TO_RGBA(7A3C3F, FF),		// Background (top)
				HEX_TO_RGBA(3B1719, FF)			// Background (bottom)
			}, { // LOAD - Cherries
				HEX_TO_RGBA(8A0C0D, FF),		// Average
				HEX_TO_RGBA(404040, FF),		// Border
				HEX_TO_RGBA(E0BEBC, FF),		// Background (top)
				HEX_TO_RGBA(CE6370, FF)			// Background (bottom)
			}, { // DISK - Peaches
				HEX_TO_RGBA(FB9924, FF),		// Read
				HEX_TO_RGBA(F36321, FF),		// Write
				HEX_TO_RGBA(404040, FF),		// Border
				HEX_TO_RGBA(F3D5AF, FF),		// Background (top)
				HEX_TO_RGBA(F6BE84, FF)			// Background (bottom)
			}, { // TEMP - Strawberries
				HEX_TO_RGBA(861514, FF),		// Value
				HEX_TO_RGBA(EAD6B3, FF),		// Critical
				HEX_TO_RGBA(404040, FF),		// Border
				HEX_TO_RGBA(E38B5E, FF),		// Background (top)
				HEX_TO_RGBA(D5290C, FF)			// Background (bottom)
			}, { // BAT - Avocado
				HEX_TO_RGBA(878844, FF),		// Charging
				HEX_TO_RGBA(D5D984, FF),		// Discharging
				HEX_TO_RGBA(EBECD0, FF),		// Critical level
				HEX_TO_RGBA(404040, FF),		// Border
				HEX_TO_RGBA(CD8342, FF),		// Background (top)
				HEX_TO_RGBA(885F3C, FF)			// Background (bottom)
			}, { // PARM
				HEX_TO_RGBA(464D47, FF),		// Result 1
				HEX_TO_RGBA(6B756C, FF),		// Result 2
				HEX_TO_RGBA(909E92, FF),		// Result 3
				HEX_TO_RGBA(B3C4B5, FF),		// Result 4
				HEX_TO_RGBA(404040, FF),		// Border
				HEX_TO_RGBA(E2EBE3, FF),		// Background (top)
				HEX_TO_RGBA(A0A7A1, FF)			// Background (bottom)
			}
		}
	},

	{ "Colored Glass", color_scheme_colored_glass_xpm,
			{  { // CPU
				HEX_TO_RGBA(FFFFFF, D8),		// User
				HEX_TO_RGBA(FFFFFF, A5),		// System
				HEX_TO_RGBA(FFFFFF, 72),		// Nice
				HEX_TO_RGBA(FFFFFF, 3F),		// IOWait
				HEX_TO_RGBA(FFFFFF, FF),		// Border
				HEX_TO_RGBA(BEEEFF, FF),		// Background (top)
				HEX_TO_RGBA(48BDE6, FF)			// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(FFFFFF, D8),		// User
				HEX_TO_RGBA(FFFFFF, D8),		// Shared
				HEX_TO_RGBA(FFFFFF, A5),		// Buffers
				HEX_TO_RGBA(FFFFFF, 72),		// Cached
				HEX_TO_RGBA(FFFFFF, FF),		// Border
				HEX_TO_RGBA(BFFFE0, FF),		// Background (top)
				HEX_TO_RGBA(43D18D, FF)			// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(FFFFFF, D8),		// In
				HEX_TO_RGBA(FFFFFF, A5),		// Out
				HEX_TO_RGBA(FFFFFF, 72),		// Local
				HEX_TO_RGBA(FFFFFF, FF),		// Border
				HEX_TO_RGBA(F1F6D1, FF),		// Background (top)
				HEX_TO_RGBA(D7CD46, FF)			// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(FFFFFF, D8),		// Used
				HEX_TO_RGBA(FFFFFF, FF),		// Border
				HEX_TO_RGBA(E6D6F0, FF),		// Background (top)
				HEX_TO_RGBA(D0A1E1, FF)			// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(FFFFFF, D8),		// Average
				HEX_TO_RGBA(FFFFFF, FF),		// Border
				HEX_TO_RGBA(EACECE, FF),		// Background (top)
				HEX_TO_RGBA(DF8181, FF)			// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(FFFFFF, D8),		// Read
				HEX_TO_RGBA(FFFFFF, A5),		// Write
				HEX_TO_RGBA(FFFFFF, FF),		// Border
				HEX_TO_RGBA(F3E8DD, FF),		// Background (top)
				HEX_TO_RGBA(E2AB70, FF)			// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(FFFFFF, D8),		// Value
				HEX_TO_RGBA(FFFFFF, A5),		// Critical
				HEX_TO_RGBA(FFFFFF, FF),		// Border
				HEX_TO_RGBA(FAD8F3, FF),		// Background (top)
				HEX_TO_RGBA(F28AE1, FF)			// Background (bottom)
			}, { // BAT
				HEX_TO_RGBA(FFFFFF, D8),		// Charging
				HEX_TO_RGBA(FFFFFF, A5),		// Discharging
				HEX_TO_RGBA(FFFFFF, 72),		// Critical level
				HEX_TO_RGBA(FFFFFF, FF),		// Border
				HEX_TO_RGBA(E4FFCA, FF),		// Background (top)
				HEX_TO_RGBA(BFFF81, FF)			// Background (bottom)
			}, { // PARM
				HEX_TO_RGBA(FFFFFF, D8),		// Result 1
				HEX_TO_RGBA(FFFFFF, A5),		// Result 2
				HEX_TO_RGBA(FFFFFF, 72),		// Result 3
				HEX_TO_RGBA(FFFFFF, 3F),		// Result 4
				HEX_TO_RGBA(FFFFFF, FF),		// Border
				HEX_TO_RGBA(DDDDDD, FF),		// Background (top)
				HEX_TO_RGBA(BBBBBB, FF)			// Background (bottom)
			}
		}
	},

	{ "=====" },

	{ "Ubuntu Ambiance", color_scheme_ubuntu_ambiance_xpm,
			{  { // CPU
				HEX_TO_RGBA(E96F20, FF),		// User
				HEX_TO_RGBA(E96F20, FF),		// System
				HEX_TO_RGBA(E96F20, FF),		// Nice
				HEX_TO_RGBA(E96F20, FF),		// IOWait
				HEX_TO_RGBA(373737, FF),		// Border
				HEX_TO_RGBA(300A24, FF),		// Background (top)
				HEX_TO_RGBA(300A24, FF)			// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(E96F20, FF),		// User
				HEX_TO_RGBA(E96F20, FF),		// Shared
				HEX_TO_RGBA(E96F20, FF),		// Buffers
				HEX_TO_RGBA(E96F20, FF),		// Cached
				HEX_TO_RGBA(373737, FF),		// Border
				HEX_TO_RGBA(300A24, FF),		// Background (top)
				HEX_TO_RGBA(300A24, FF)			// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(E96F20, FF),		// In
				HEX_TO_RGBA(E96F20, FF),		// Out
				HEX_TO_RGBA(E96F20, FF),		// Local
				HEX_TO_RGBA(373737, FF),		// Border
				HEX_TO_RGBA(300A24, FF),		// Background (top)
				HEX_TO_RGBA(300A24, FF)			// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(E96F20, FF),		// Used
				HEX_TO_RGBA(373737, FF),		// Border
				HEX_TO_RGBA(300A24, FF),		// Background (top)
				HEX_TO_RGBA(300A24, FF)			// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(E96F20, FF),		// Average
				HEX_TO_RGBA(373737, FF),		// Border
				HEX_TO_RGBA(300A24, FF),		// Background (top)
				HEX_TO_RGBA(300A24, FF)			// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(E96F20, FF),		// Read
				HEX_TO_RGBA(E96F20, FF),		// Write
				HEX_TO_RGBA(373737, FF),		// Border
				HEX_TO_RGBA(300A24, FF),		// Background (top)
				HEX_TO_RGBA(300A24, FF)			// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(E96F20, FF),		// Value
				HEX_TO_RGBA(E96F20, FF),		// Critical
				HEX_TO_RGBA(373737, FF),		// Border
				HEX_TO_RGBA(300A24, FF),		// Background (top)
				HEX_TO_RGBA(300A24, FF)			// Background (bottom)
			}, { // BAT
				HEX_TO_RGBA(E96F20, FF),		// Charging
				HEX_TO_RGBA(E96F20, FF),		// Discharging
				HEX_TO_RGBA(E96F20, FF),		// Critical level
				HEX_TO_RGBA(373737, FF),		// Border
				HEX_TO_RGBA(300A24, FF),		// Background (top)
				HEX_TO_RGBA(300A24, FF)			// Background (bottom)
			}, { // PARM
				HEX_TO_RGBA(E96F20, FF),		// Result 1
				HEX_TO_RGBA(E96F20, FF),		// Result 2
				HEX_TO_RGBA(E96F20, FF),		// Result 3
				HEX_TO_RGBA(E96F20, FF),		// Result 4
				HEX_TO_RGBA(373737, FF),		// Border
				HEX_TO_RGBA(300A24, FF),		// Background (top)
				HEX_TO_RGBA(300A24, FF)			// Background (bottom)
			}
		}
	},

	{ "Ubuntu Radiance", color_scheme_ubuntu_radiance_xpm,
			{  { // CPU
				HEX_TO_RGBA(E96F20, FF),		// User
				HEX_TO_RGBA(E96F20, FF),		// System
				HEX_TO_RGBA(E96F20, FF),		// Nice
				HEX_TO_RGBA(E96F20, FF),		// IOWait
				HEX_TO_RGBA(D6D6D6, FF),		// Border
				HEX_TO_RGBA(E8E8E8, FF),		// Background (top)
				HEX_TO_RGBA(E8E8E8, FF)			// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(E96F20, FF),		// User
				HEX_TO_RGBA(E96F20, FF),		// Shared
				HEX_TO_RGBA(E96F20, FF),		// Buffers
				HEX_TO_RGBA(E96F20, FF),		// Cached
				HEX_TO_RGBA(D6D6D6, FF),		// Border
				HEX_TO_RGBA(E8E8E8, FF),		// Background (top)
				HEX_TO_RGBA(E8E8E8, FF)			// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(E96F20, FF),		// In
				HEX_TO_RGBA(E96F20, FF),		// Out
				HEX_TO_RGBA(E96F20, FF),		// Local
				HEX_TO_RGBA(D6D6D6, FF),		// Border
				HEX_TO_RGBA(E8E8E8, FF),		// Background (top)
				HEX_TO_RGBA(E8E8E8, FF)			// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(E96F20, FF),		// Used
				HEX_TO_RGBA(D6D6D6, FF),		// Border
				HEX_TO_RGBA(E8E8E8, FF),		// Background (top)
				HEX_TO_RGBA(E8E8E8, FF)			// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(E96F20, FF),		// Average
				HEX_TO_RGBA(D6D6D6, FF),		// Border
				HEX_TO_RGBA(E8E8E8, FF),		// Background (top)
				HEX_TO_RGBA(E8E8E8, FF)			// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(E96F20, FF),		// Read
				HEX_TO_RGBA(E96F20, FF),		// Write
				HEX_TO_RGBA(D6D6D6, FF),		// Border
				HEX_TO_RGBA(E8E8E8, FF),		// Background (top)
				HEX_TO_RGBA(E8E8E8, FF)			// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(E96F20, FF),		// Value
				HEX_TO_RGBA(E96F20, FF),		// Critical
				HEX_TO_RGBA(D6D6D6, FF),		// Border
				HEX_TO_RGBA(E8E8E8, FF),		// Background (top)
				HEX_TO_RGBA(E8E8E8, FF)			// Background (bottom)
			}, { // BAT
				HEX_TO_RGBA(E96F20, FF),		// Charging
				HEX_TO_RGBA(E96F20, FF),		// Discharging
				HEX_TO_RGBA(E96F20, FF),		// Critical level
				HEX_TO_RGBA(D6D6D6, FF),		// Border
				HEX_TO_RGBA(E8E8E8, FF),		// Background (top)
				HEX_TO_RGBA(E8E8E8, FF)			// Background (bottom)
			}, { // PARM
				HEX_TO_RGBA(E96F20, FF),		// Result 1
				HEX_TO_RGBA(E96F20, FF),		// Result 2
				HEX_TO_RGBA(E96F20, FF),		// Result 3
				HEX_TO_RGBA(E96F20, FF),		// Result 4
				HEX_TO_RGBA(D6D6D6, FF),		// Border
				HEX_TO_RGBA(E8E8E8, FF),		// Background (top)
				HEX_TO_RGBA(E8E8E8, FF)			// Background (bottom)
			}
		}
	},

	{ "Linux Mint", color_scheme_linux_mint_xpm,
			{  { // CPU
				HEX_TO_RGBA(97BF60, FF),		// User
				HEX_TO_RGBA(97BF60, FF),		// System
				HEX_TO_RGBA(97BF60, FF),		// Nice
				HEX_TO_RGBA(97BF60, FF),		// IOWait
				HEX_TO_RGBA(3C3C3C, FF),		// Border
				HEX_TO_RGBA(484848, FF),		// Background (top)
				HEX_TO_RGBA(393939, FF)			// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(97BF60, FF),		// User
				HEX_TO_RGBA(97BF60, FF),		// Shared
				HEX_TO_RGBA(97BF60, FF),		// Buffers
				HEX_TO_RGBA(97BF60, FF),		// Cached
				HEX_TO_RGBA(3C3C3C, FF),		// Border
				HEX_TO_RGBA(484848, FF),		// Background (top)
				HEX_TO_RGBA(393939, FF)			// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(97BF60, FF),		// In
				HEX_TO_RGBA(97BF60, FF),		// Out
				HEX_TO_RGBA(97BF60, FF),		// Local
				HEX_TO_RGBA(3C3C3C, FF),		// Border
				HEX_TO_RGBA(484848, FF),		// Background (top)
				HEX_TO_RGBA(393939, FF)			// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(97BF60, FF),		// Used
				HEX_TO_RGBA(3C3C3C, FF),		// Border
				HEX_TO_RGBA(484848, FF),		// Background (top)
				HEX_TO_RGBA(393939, FF)			// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(97BF60, FF),		// Average
				HEX_TO_RGBA(3C3C3C, FF),		// Border
				HEX_TO_RGBA(484848, FF),		// Background (top)
				HEX_TO_RGBA(393939, FF)			// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(97BF60, FF),		// Read
				HEX_TO_RGBA(97BF60, FF),		// Write
				HEX_TO_RGBA(3C3C3C, FF),		// Border
				HEX_TO_RGBA(484848, FF),		// Background (top)
				HEX_TO_RGBA(393939, FF)			// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(97BF60, FF),		// Value
				HEX_TO_RGBA(97BF60, FF),		// Critical
				HEX_TO_RGBA(3C3C3C, FF),		// Border
				HEX_TO_RGBA(484848, FF),		// Background (top)
				HEX_TO_RGBA(393939, FF)			// Background (bottom)
			}, { // BAT
				HEX_TO_RGBA(97BF60, FF),		// Charging
				HEX_TO_RGBA(97BF60, FF),		// Discharging
				HEX_TO_RGBA(97BF60, FF),		// Critical level
				HEX_TO_RGBA(3C3C3C, FF),		// Border
				HEX_TO_RGBA(484848, FF),		// Background (top)
				HEX_TO_RGBA(393939, FF)			// Background (bottom)
			}, { // PARM
				HEX_TO_RGBA(97BF60, FF),		// Result 1
				HEX_TO_RGBA(97BF60, FF),		// Result 2
				HEX_TO_RGBA(97BF60, FF),		// Result 3
				HEX_TO_RGBA(97BF60, FF),		// Result 4
				HEX_TO_RGBA(3C3C3C, FF),		// Border
				HEX_TO_RGBA(484848, FF),		// Background (top)
				HEX_TO_RGBA(393939, FF)			// Background (bottom)
			}
		}
	},

	{ "Windows Metro", color_scheme_windows_metro_xpm, // http://www.creepyed.com/2012/09/windows-8-colors-hex-code/
			{  { // CPU
				HEX_TO_RGBA(2C4566, FF),		// User
				HEX_TO_RGBA(006AC1, FF),		// System
				HEX_TO_RGBA(006AC1, FF),		// Nice
				HEX_TO_RGBA(006AC1, FF),		// IOWait
				HEX_TO_RGBA(001E4E, FF),		// Border
				HEX_TO_RGBA(001940, FF),		// Background (top)
				HEX_TO_RGBA(001940, FF)			// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(199900, FF),		// User
				HEX_TO_RGBA(199900, FF),		// Shared
				HEX_TO_RGBA(2D652B, FF),		// Buffers
				HEX_TO_RGBA(2D652B, FF),		// Cached
				HEX_TO_RGBA(004A00, FF),		// Border
				HEX_TO_RGBA(003E00, FF),		// Background (top)
				HEX_TO_RGBA(003E00, FF)			// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(E1B700, FF),		// In
				HEX_TO_RGBA(CEA539, FF),		// Out
				HEX_TO_RGBA(CEA539, FF),		// Local
				HEX_TO_RGBA(D39D09, FF),		// Border
				HEX_TO_RGBA(C69408, FF),		// Background (top)
				HEX_TO_RGBA(C69408, FF)			// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(AA40FF, FF),		// Used
				HEX_TO_RGBA(691BB8, FF),		// Border
				HEX_TO_RGBA(57169A, FF),		// Background (top)
				HEX_TO_RGBA(57169A, FF)			// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(FF2E12, FF),		// Average
				HEX_TO_RGBA(B81B1B, FF),		// Border
				HEX_TO_RGBA(9E1716, FF),		// Background (top)
				HEX_TO_RGBA(9E1716, FF)			// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(FF981D, FF),		// Read
				HEX_TO_RGBA(C27D4F, FF),		// Write
				HEX_TO_RGBA(E56C19, FF),		// Border
				HEX_TO_RGBA(C35D15, FF),		// Background (top)
				HEX_TO_RGBA(C35D15, FF)			// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(FF76BC, FF),		// Value
				HEX_TO_RGBA(E773BD, FF),		// Critical
				HEX_TO_RGBA(E064B7, FF),		// Border
				HEX_TO_RGBA(DE4AAD, FF),		// Background (top)
				HEX_TO_RGBA(DE4AAD, FF)			// Background (bottom)
			}, { // BAT
				HEX_TO_RGBA(91D100, FF),		// Charging
				HEX_TO_RGBA(94BD4A, FF),		// Discharging
				HEX_TO_RGBA(94BD4A, FF),		// Critical level
				HEX_TO_RGBA(83BA1F, FF),		// Border
				HEX_TO_RGBA(7BAD18, FF),		// Background (top)
				HEX_TO_RGBA(7BAD18, FF)			// Background (bottom)
			}, { // PARM
				HEX_TO_RGBA(878787, FF),		// Result 1
				HEX_TO_RGBA(727272, FF),		// Result 2
				HEX_TO_RGBA(727272, FF),		// Result 3
				HEX_TO_RGBA(727272, FF),		// Result 4
				HEX_TO_RGBA(606060, FF),		// Border
				HEX_TO_RGBA(505050, FF),		// Background (top)
				HEX_TO_RGBA(505050, FF)			// Background (bottom)
			}
		}
	},

	{ "=====" },

	{ "Arc", color_scheme_arc_xpm,
			{  { // CPU
				HEX_TO_RGBA(5924E2, FF),		// User
				HEX_TO_RGBA(5924E2, FF),		// System
				HEX_TO_RGBA(5924E2, FF),		// Nice
				HEX_TO_RGBA(5924E2, FF),		// IOWait
				HEX_TO_RGBA(1B1E24, FF),		// Border
				HEX_TO_RGBA(383C4A, FF),		// Background (top)
				HEX_TO_RGBA(383C4A, FF)			// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(5924E2, FF),		// User
				HEX_TO_RGBA(5924E2, FF),		// Shared
				HEX_TO_RGBA(5924E2, FF),		// Buffers
				HEX_TO_RGBA(5924E2, FF),		// Cached
				HEX_TO_RGBA(1B1E24, FF),		// Border
				HEX_TO_RGBA(383C4A, FF),		// Background (top)
				HEX_TO_RGBA(383C4A, FF)			// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(5924E2, FF),		// In
				HEX_TO_RGBA(5924E2, FF),		// Out
				HEX_TO_RGBA(5924E2, FF),		// Local
				HEX_TO_RGBA(1B1E24, FF),		// Border
				HEX_TO_RGBA(383C4A, FF),		// Background (top)
				HEX_TO_RGBA(383C4A, FF)			// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(5924E2, FF),		// Used
				HEX_TO_RGBA(1B1E24, FF),		// Border
				HEX_TO_RGBA(383C4A, FF),		// Background (top)
				HEX_TO_RGBA(383C4A, FF)			// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(5924E2, FF),		// Average
				HEX_TO_RGBA(1B1E24, FF),		// Border
				HEX_TO_RGBA(383C4A, FF),		// Background (top)
				HEX_TO_RGBA(383C4A, FF)			// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(5924E2, FF),		// Read
				HEX_TO_RGBA(5924E2, FF),		// Write
				HEX_TO_RGBA(1B1E24, FF),		// Border
				HEX_TO_RGBA(383C4A, FF),		// Background (top)
				HEX_TO_RGBA(383C4A, FF)			// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(5924E2, FF),		// Value
				HEX_TO_RGBA(5924E2, FF),		// Critical
				HEX_TO_RGBA(1B1E24, FF),		// Border
				HEX_TO_RGBA(383C4A, FF),		// Background (top)
				HEX_TO_RGBA(383C4A, FF)			// Background (bottom)
			}, { // BAT
				HEX_TO_RGBA(5924E2, FF),		// Charging
				HEX_TO_RGBA(5924E2, FF),		// Discharging
				HEX_TO_RGBA(5924E2, FF),		// Critical level
				HEX_TO_RGBA(1B1E24, FF),		// Border
				HEX_TO_RGBA(383C4A, FF),		// Background (top)
				HEX_TO_RGBA(383C4A, FF)			// Background (bottom)
			}, { // PARM
				HEX_TO_RGBA(5924E2, FF),		// Result 1
				HEX_TO_RGBA(5924E2, FF),		// Result 2
				HEX_TO_RGBA(5924E2, FF),		// Result 3
				HEX_TO_RGBA(5924E2, FF),		// Result 4
				HEX_TO_RGBA(1B1E24, FF),		// Border
				HEX_TO_RGBA(383C4A, FF),		// Background (top)
				HEX_TO_RGBA(383C4A, FF)			// Background (bottom)
			}
		}
	},

	{ "Numix Dark", color_scheme_numix_dark_xpm,
			{  { // CPU
				HEX_TO_RGBA(D64937, FF),		// User
				HEX_TO_RGBA(D64937, FF),		// System
				HEX_TO_RGBA(D64937, FF),		// Nice
				HEX_TO_RGBA(D64937, FF),		// IOWait
				HEX_TO_RGBA(DEDEDE, FF),		// Border
				HEX_TO_RGBA(333333, FF),		// Background (top)
				HEX_TO_RGBA(333333, FF)			// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(D64937, FF),		// User
				HEX_TO_RGBA(D64937, FF),		// Shared
				HEX_TO_RGBA(D64937, FF),		// Buffers
				HEX_TO_RGBA(D64937, FF),		// Cached
				HEX_TO_RGBA(DEDEDE, FF),		// Border
				HEX_TO_RGBA(333333, FF),		// Background (top)
				HEX_TO_RGBA(333333, FF)			// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(D64937, FF),		// In
				HEX_TO_RGBA(D64937, FF),		// Out
				HEX_TO_RGBA(D64937, FF),		// Local
				HEX_TO_RGBA(DEDEDE, FF),		// Border
				HEX_TO_RGBA(333333, FF),		// Background (top)
				HEX_TO_RGBA(333333, FF)			// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(D64937, FF),		// Used
				HEX_TO_RGBA(DEDEDE, FF),		// Border
				HEX_TO_RGBA(333333, FF),		// Background (top)
				HEX_TO_RGBA(333333, FF)			// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(D64937, FF),		// Average
				HEX_TO_RGBA(DEDEDE, FF),		// Border
				HEX_TO_RGBA(333333, FF),		// Background (top)
				HEX_TO_RGBA(333333, FF)			// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(D64937, FF),		// Read
				HEX_TO_RGBA(D64937, FF),		// Write
				HEX_TO_RGBA(DEDEDE, FF),		// Border
				HEX_TO_RGBA(333333, FF),		// Background (top)
				HEX_TO_RGBA(333333, FF)			// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(D64937, FF),		// Value
				HEX_TO_RGBA(D64937, FF),		// Critical
				HEX_TO_RGBA(DEDEDE, FF),		// Border
				HEX_TO_RGBA(333333, FF),		// Background (top)
				HEX_TO_RGBA(333333, FF)			// Background (bottom)
			}, { // BAT
				HEX_TO_RGBA(D64937, FF),		// Charging
				HEX_TO_RGBA(D64937, FF),		// Discharging
				HEX_TO_RGBA(D64937, FF),		// Critical level
				HEX_TO_RGBA(DEDEDE, FF),		// Border
				HEX_TO_RGBA(333333, FF),		// Background (top)
				HEX_TO_RGBA(333333, FF)			// Background (bottom)
			}, { // PARM
				HEX_TO_RGBA(D64937, FF),		// Result 1
				HEX_TO_RGBA(D64937, FF),		// Result 2
				HEX_TO_RGBA(D64937, FF),		// Result 3
				HEX_TO_RGBA(D64937, FF),		// Result 4
				HEX_TO_RGBA(DEDEDE, FF),		// Border
				HEX_TO_RGBA(333333, FF),		// Background (top)
				HEX_TO_RGBA(333333, FF)			// Background (bottom)
			}
		}
	},

	{ "Numix Light", color_scheme_numix_light_xpm,
			{  { // CPU
				HEX_TO_RGBA(D64937, FF),		// User
				HEX_TO_RGBA(D64937, FF),		// System
				HEX_TO_RGBA(D64937, FF),		// Nice
				HEX_TO_RGBA(D64937, FF),		// IOWait
				HEX_TO_RGBA(333333, FF),		// Border
				HEX_TO_RGBA(DEDEDE, FF),		// Background (top)
				HEX_TO_RGBA(DEDEDE, FF)			// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(D64937, FF),		// User
				HEX_TO_RGBA(D64937, FF),		// Shared
				HEX_TO_RGBA(D64937, FF),		// Buffers
				HEX_TO_RGBA(D64937, FF),		// Cached
				HEX_TO_RGBA(333333, FF),		// Border
				HEX_TO_RGBA(DEDEDE, FF),		// Background (top)
				HEX_TO_RGBA(DEDEDE, FF)			// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(D64937, FF),		// In
				HEX_TO_RGBA(D64937, FF),		// Out
				HEX_TO_RGBA(D64937, FF),		// Local
				HEX_TO_RGBA(333333, FF),		// Border
				HEX_TO_RGBA(DEDEDE, FF),		// Background (top)
				HEX_TO_RGBA(DEDEDE, FF)			// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(D64937, FF),		// Used
				HEX_TO_RGBA(333333, FF),		// Border
				HEX_TO_RGBA(DEDEDE, FF),		// Background (top)
				HEX_TO_RGBA(DEDEDE, FF)			// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(D64937, FF),		// Average
				HEX_TO_RGBA(333333, FF),		// Border
				HEX_TO_RGBA(DEDEDE, FF),		// Background (top)
				HEX_TO_RGBA(DEDEDE, FF)			// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(D64937, FF),		// Read
				HEX_TO_RGBA(D64937, FF),		// Write
				HEX_TO_RGBA(333333, FF),		// Border
				HEX_TO_RGBA(DEDEDE, FF),		// Background (top)
				HEX_TO_RGBA(DEDEDE, FF)			// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(D64937, FF),		// Value
				HEX_TO_RGBA(D64937, FF),		// Critical
				HEX_TO_RGBA(333333, FF),		// Border
				HEX_TO_RGBA(DEDEDE, FF),		// Background (top)
				HEX_TO_RGBA(DEDEDE, FF)			// Background (bottom)
			}, { // BAT
				HEX_TO_RGBA(D64937, FF),		// Charging
				HEX_TO_RGBA(D64937, FF),		// Discharging
				HEX_TO_RGBA(D64937, FF),		// Critical level
				HEX_TO_RGBA(333333, FF),		// Border
				HEX_TO_RGBA(DEDEDE, FF),		// Background (top)
				HEX_TO_RGBA(DEDEDE, FF)			// Background (bottom)
			}, { // PARM
				HEX_TO_RGBA(D64937, FF),		// Result 1
				HEX_TO_RGBA(D64937, FF),		// Result 2
				HEX_TO_RGBA(D64937, FF),		// Result 3
				HEX_TO_RGBA(D64937, FF),		// Result 4
				HEX_TO_RGBA(333333, FF),		// Border
				HEX_TO_RGBA(DEDEDE, FF),		// Background (top)
				HEX_TO_RGBA(DEDEDE, FF)			// Background (bottom)
			}
		}
	},

	{ "=====" },

	{ "Super Mario Bros", color_scheme_super_mario_bros_xpm,
			{  { // CPU - Plant
				HEX_TO_RGBA(48A327, FF),		// User
				HEX_TO_RGBA(82D427, FF),		// System
				HEX_TO_RGBA(CA9258, FF),		// Nice
				HEX_TO_RGBA(669F74, FF),		// IOWait
				HEX_TO_RGBA(030000, FF),		// Border
				HEX_TO_RGBA(6385FB, FF),		// Background (top)
				HEX_TO_RGBA(6385FB, FF)			// Background (bottom)
			}, { // MEM - Mario
				HEX_TO_RGBA(DA0000, FF),		// User
				HEX_TO_RGBA(DA0000, FF),		// Shared
				HEX_TO_RGBA(716800, FF),		// Buffers
				HEX_TO_RGBA(FAB100, FF),		// Cached
				HEX_TO_RGBA(030000, FF),		// Border
				HEX_TO_RGBA(6385FB, FF),		// Background (top)
				HEX_TO_RGBA(6385FB, FF)			// Background (bottom)
			}, { // NET - Luigi
				HEX_TO_RGBA(FBFBFB, FF),		// In
				HEX_TO_RGBA(3B8400, FF),		// Out
				HEX_TO_RGBA(FAB100, FF),		// Local
				HEX_TO_RGBA(030000, FF),		// Border
				HEX_TO_RGBA(6385FB, FF),		// Background (top)
				HEX_TO_RGBA(6385FB, FF)			// Background (bottom)
			}, { // SWAP - Coin
				HEX_TO_RGBA(FAB930, FF),		// Used
				HEX_TO_RGBA(030000, FF),		// Border
				HEX_TO_RGBA(6385FB, FF),		// Background (top)
				HEX_TO_RGBA(6385FB, FF)			// Background (bottom)
			}, { // LOAD - Castle brick
				HEX_TO_RGBA(B9B9B9, FF),		// Average
				HEX_TO_RGBA(030000, FF),		// Border
				HEX_TO_RGBA(6385FB, FF),		// Background (top)
				HEX_TO_RGBA(6385FB, FF)			// Background (bottom)
			}, { // DISK - Koopa
				HEX_TO_RGBA(FAC095, FF),		// Read
				HEX_TO_RGBA(9A4800, FF),		// Write
				HEX_TO_RGBA(030000, FF),		// Border
				HEX_TO_RGBA(6385FB, FF),		// Background (top)
				HEX_TO_RGBA(6385FB, FF)			// Background (bottom)
			}, { // TEMP - Bush
				HEX_TO_RGBA(6AF90D, FF),		// Value
				HEX_TO_RGBA(008700, FF),		// Critical
				HEX_TO_RGBA(030000, FF),		// Border
				HEX_TO_RGBA(6385FB, FF),		// Background (top)
				HEX_TO_RGBA(6385FB, FF)			// Background (bottom)
			}, { // BAT - Mario Fire
				HEX_TO_RGBA(A32E1F, FF),		// Charging
				HEX_TO_RGBA(F7794D, FF),		// Discharging
				HEX_TO_RGBA(EED09E, FF),		// Critical level
				HEX_TO_RGBA(030000, FF),		// Border
				HEX_TO_RGBA(6385FB, FF),		// Background (top)
				HEX_TO_RGBA(6385FB, FF)			// Background (bottom)
			}, { // PARM - Cloud
				HEX_TO_RGBA(709EC7, FF),		// Result 1
				HEX_TO_RGBA(58CAFF, FF),		// Result 2
				HEX_TO_RGBA(ACD0F2, FF),		// Result 3
				HEX_TO_RGBA(FBFBFB, FF),		// Result 4
				HEX_TO_RGBA(030000, FF),		// Border
				HEX_TO_RGBA(6385FB, FF),		// Background (top)
				HEX_TO_RGBA(6385FB, FF)			// Background (bottom)
			}
		}
	},

	{ "Dragon Ball Z", color_scheme_dragon_ball_z_xpm,
			{  { // CPU - Ball
				HEX_TO_RGBA(F01920, FF),		// User
				HEX_TO_RGBA(E34D0A, FF),		// System
				HEX_TO_RGBA(D45C41, FF),		// Nice
				HEX_TO_RGBA(D48341, FF),		// IOWait
				HEX_TO_RGBA(11141B, FF),		// Border
				HEX_TO_RGBA(FCA400, FF),		// Background (top)
				HEX_TO_RGBA(FCA400, FF)			// Background (bottom)
			}, { // MEM - Goku
				HEX_TO_RGBA(333098, FF),		// User
				HEX_TO_RGBA(333098, FF),		// Shared
				HEX_TO_RGBA(FDC9AB, FF),		// Buffers
				HEX_TO_RGBA(11141B, FF),		// Cached
				HEX_TO_RGBA(11141B, FF),		// Border
				HEX_TO_RGBA(FD5700, FF),		// Background (top)
				HEX_TO_RGBA(FD5700, FF)			// Background (bottom)
			}, { // NET - Vegeta
				HEX_TO_RGBA(D9D8D9, FF),		// In
				HEX_TO_RGBA(FDC9AB, FF),		// Out
				HEX_TO_RGBA(11141B, FF),		// Local
				HEX_TO_RGBA(11141B, FF),		// Border
				HEX_TO_RGBA(323399, FF),		// Background (top)
				HEX_TO_RGBA(323399, FF)			// Background (bottom)
			}, { // SWAP - Boo
				HEX_TO_RGBA(FD9899, FF),		// Used
				HEX_TO_RGBA(11141B, FF),		// Border
				HEX_TO_RGBA(D9D9D9, FF),		// Background (top)
				HEX_TO_RGBA(D9D9D9, FF)			// Background (bottom)
			}, { // LOAD - Freeza
				HEX_TO_RGBA(A500FF, FF),		// Average
				HEX_TO_RGBA(11141B, FF),		// Border
				HEX_TO_RGBA(D9D9D9, FF),		// Background (top)
				HEX_TO_RGBA(D9D9D9, FF)			// Background (bottom)
			}, { // DISK - Bulma
				HEX_TO_RGBA(FFE4E0, FF),		// Read
				HEX_TO_RGBA(5CADAE, FF),		// Write
				HEX_TO_RGBA(11141B, FF),		// Border
				HEX_TO_RGBA(FECE02, FF),		// Background (top)
				HEX_TO_RGBA(FECE02, FF)			// Background (bottom)
			}, { // TEMP - Trunks
				HEX_TO_RGBA(F8E7CB, FF),		// Value
				HEX_TO_RGBA(BDB5EE, FF),		// Critical
				HEX_TO_RGBA(11141B, FF),		// Border
				HEX_TO_RGBA(4D5C7D, FF),		// Background (top)
				HEX_TO_RGBA(4D5C7D, FF)			// Background (bottom)
			}, { // BAT - Cell
				HEX_TO_RGBA(35074F, FF),		// Charging
				HEX_TO_RGBA(2C5308, FF),		// Discharging
				HEX_TO_RGBA(629930, FF),		// Critical level
				HEX_TO_RGBA(11141B, FF),		// Border
				HEX_TO_RGBA(CCCDCC, FF),		// Background (top)
				HEX_TO_RGBA(CCCDCC, FF)			// Background (bottom)
			}, { // PARM - Jiaozi
				HEX_TO_RGBA(C58102, FF),		// Result 1
				HEX_TO_RGBA(DA240C, FF),		// Result 2
				HEX_TO_RGBA(F0F0F0, FF),		// Result 3
				HEX_TO_RGBA(29352B, FF),		// Result 4
				HEX_TO_RGBA(11141B, FF),		// Border
				HEX_TO_RGBA(337F37, FF),		// Background (top)
				HEX_TO_RGBA(337F37, FF)			// Background (bottom)
			}
		}
	},

	{ "The Simpsons", color_scheme_simpsons_xpm, // http://www.fubiz.net/en/2015/09/19/the-colors-of-simpsons-pantone/
			{  { // CPU - Homer
				HEX_TO_RGBA(6BADDF, FF),		// User
				HEX_TO_RGBA(FFFFFF, FF),		// System
				HEX_TO_RGBA(D2B17E, FF),		// Nice
				HEX_TO_RGBA(FED420, FF),		// IOWait
				HEX_TO_RGBA(657A7D, FF),		// Border
				HEX_TO_RGBA(657A7D, FF),		// Background (top)
				HEX_TO_RGBA(657A7D, FF)			// Background (bottom)
			}, { // MEM - Bart
				HEX_TO_RGBA(0F9BE0, FF),		// User
				HEX_TO_RGBA(0F9BE0, FF),		// Shared
				HEX_TO_RGBA(F15B30, FF),		// Buffers
				HEX_TO_RGBA(FED420, FF),		// Cached
				HEX_TO_RGBA(84C55F, FF),		// Border
				HEX_TO_RGBA(84C55F, FF),		// Background (top)
				HEX_TO_RGBA(84C55F, FF)			// Background (bottom)
			}, { // NET - Milhouse
				HEX_TO_RGBA(E0C6DF, FF),		// In
				HEX_TO_RGBA(FED420, FF),		// Out
				HEX_TO_RGBA(4B91CC, FF),		// Local
				HEX_TO_RGBA(F15B30, FF),		// Border
				HEX_TO_RGBA(F15B30, FF),		// Background (top)
				HEX_TO_RGBA(F15B30, FF)			// Background (bottom)
			}, { // SWAP - Little Santa's Helper
				HEX_TO_RGBA(FFFFFF, FF),		// Used
				HEX_TO_RGBA(000000, FF),		// Border
				HEX_TO_RGBA(C37C14, FF),		// Background (top)
				HEX_TO_RGBA(C37C14, FF)			// Background (bottom)
			}, { // LOAD - Snowball
				HEX_TO_RGBA(FF88CC, FF),		// Average
				HEX_TO_RGBA(2099AA, FF),		// Border
				HEX_TO_RGBA(475556, FF),		// Background (top)
				HEX_TO_RGBA(475556, FF)			// Background (bottom)
			}, { // DISK - Sideshow Bob
				HEX_TO_RGBA(FED420, FF),		// Read
				HEX_TO_RGBA(C20033, FF),		// Write
				HEX_TO_RGBA(568CAE, FF),		// Border
				HEX_TO_RGBA(357130, FF),		// Background (top)
				HEX_TO_RGBA(357130, FF)			// Background (bottom)
			}, { // TEMP - Edna
				HEX_TO_RGBA(FED420, FF),		// Value
				HEX_TO_RGBA(D3AD7E, FF),		// Critical
				HEX_TO_RGBA(00947E, FF),		// Border
				HEX_TO_RGBA(7BC041, FF),		// Background (top)
				HEX_TO_RGBA(B3C98B, FF)			// Background (bottom)
			}, { // BAT - Stupid Flanders
				HEX_TO_RGBA(FAC5CB, FF),		// Charging
				HEX_TO_RGBA(FED420, FF),		// Discharging
				HEX_TO_RGBA(9B6B3B, FF),		// Critical level
				HEX_TO_RGBA(8D989E, FF),		// Border
				HEX_TO_RGBA(4F8633, FF),		// Background (top)
				HEX_TO_RGBA(4F8633, FF)			// Background (bottom)
			}, { // PARM - Moe
				HEX_TO_RGBA(FFFFFF, FF),		// Result 1
				HEX_TO_RGBA(B1D3EC, FF),		// Result 2
				HEX_TO_RGBA(FED420, FF),		// Result 3
				HEX_TO_RGBA(41535D, FF),		// Result 4
				HEX_TO_RGBA(98887B, FF),		// Border
				HEX_TO_RGBA(98887B, FF),		// Background (top)
				HEX_TO_RGBA(98887B, FF)			// Background (bottom)
			}
		}
	},

	{ "=====" },

	{ "Moon", color_scheme_moon_xpm,
			{  { // CPU
				HEX_TO_RGBA(505050, FF),		// User
				HEX_TO_RGBA(505050, FF),		// System
				HEX_TO_RGBA(505050, FF),		// Nice
				HEX_TO_RGBA(505050, FF),		// IOWait
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(CBCBCB, FF),		// Background (top)
				HEX_TO_RGBA(B5B5B5, FF)			// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(777777, FF),		// User
				HEX_TO_RGBA(777777, FF),		// Shared
				HEX_TO_RGBA(777777, FF),		// Buffers
				HEX_TO_RGBA(777777, FF),		// Cached
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(CBCBCB, FF),		// Background (top)
				HEX_TO_RGBA(B5B5B5, FF)			// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(F7F7F7, FF),		// In
				HEX_TO_RGBA(F7F7F7, FF),		// Out
				HEX_TO_RGBA(F7F7F7, FF),		// Local
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(CBCBCB, FF),		// Background (top)
				HEX_TO_RGBA(B5B5B5, FF)			// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(909090, FF),		// Used
				HEX_TO_RGBA(142329, FF),		// Border
				HEX_TO_RGBA(CBCBCB, FF),		// Background (top)
				HEX_TO_RGBA(B5B5B5, FF)			// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(2D2D2D, FF),		// Average
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(CBCBCB, FF),		// Background (top)
				HEX_TO_RGBA(B5B5B5, FF)			// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(616161, FF),		// Read
				HEX_TO_RGBA(616161, FF),		// Write
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(CBCBCB, FF),		// Background (top)
				HEX_TO_RGBA(B5B5B5, FF)			// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(E2E2E2, FF),		// Value
				HEX_TO_RGBA(E2E2E2, FF),		// Critical
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(CBCBCB, FF),		// Background (top)
				HEX_TO_RGBA(B5B5B5, FF)			// Background (bottom)
			}, { // BAT
				HEX_TO_RGBA(202020, FF),		// Charging
				HEX_TO_RGBA(202020, FF),		// Discharging
				HEX_TO_RGBA(202020, FF),		// Critical level
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(CBCBCB, FF),		// Background (top)
				HEX_TO_RGBA(B5B5B5, FF)			// Background (bottom)
			}, { // PARM
				HEX_TO_RGBA(959595, FF),		// Result 1
				HEX_TO_RGBA(959595, FF),		// Result 2
				HEX_TO_RGBA(959595, FF),		// Result 3
				HEX_TO_RGBA(959595, FF),		// Result 4
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(CBCBCB, FF),		// Background (top)
				HEX_TO_RGBA(B5B5B5, FF)			// Background (bottom)
			}
		}
	},

	{ "Venus", color_scheme_venus_xpm,
			{  { // CPU
				HEX_TO_RGBA(F9C21A, FF),		// User
				HEX_TO_RGBA(F9C21A, FF),		// System
				HEX_TO_RGBA(F9C21A, FF),		// Nice
				HEX_TO_RGBA(F9C21A, FF),		// IOWait
				HEX_TO_RGBA(E39E1C, FF),		// Border
				HEX_TO_RGBA(C18F17, FF),		// Background (top)
				HEX_TO_RGBA(A57C1B, FF)			// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(E7D520, FF),		// User
				HEX_TO_RGBA(E7D520, FF),		// Shared
				HEX_TO_RGBA(E7D520, FF),		// Buffers
				HEX_TO_RGBA(E7D520, FF),		// Cached
				HEX_TO_RGBA(E39E1C, FF),		// Border
				HEX_TO_RGBA(C18F17, FF),		// Background (top)
				HEX_TO_RGBA(A57C1B, FF)			// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(F4B555, FF),		// In
				HEX_TO_RGBA(F4B555, FF),		// Out
				HEX_TO_RGBA(F4B555, FF),		// Local
				HEX_TO_RGBA(E39E1C, FF),		// Border
				HEX_TO_RGBA(C18F17, FF),		// Background (top)
				HEX_TO_RGBA(A57C1B, FF)			// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(F4C44F, FF),		// Used
				HEX_TO_RGBA(E39E1C, FF),		// Border
				HEX_TO_RGBA(C18F17, FF),		// Background (top)
				HEX_TO_RGBA(A57C1B, FF)			// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(F5C47A, FF),		// Average
				HEX_TO_RGBA(E39E1C, FF),		// Border
				HEX_TO_RGBA(C18F17, FF),		// Background (top)
				HEX_TO_RGBA(A57C1B, FF)			// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(FFDA00, FF),		// Read
				HEX_TO_RGBA(FFDA00, FF),		// Write
				HEX_TO_RGBA(E39E1C, FF),		// Border
				HEX_TO_RGBA(C18F17, FF),		// Background (top)
				HEX_TO_RGBA(A57C1B, FF)			// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(D0BA05, FF),		// Value
				HEX_TO_RGBA(D0BA05, FF),		// Critical
				HEX_TO_RGBA(E39E1C, FF),		// Border
				HEX_TO_RGBA(C18F17, FF),		// Background (top)
				HEX_TO_RGBA(A57C1B, FF)			// Background (bottom)
			}, { // BAT
				HEX_TO_RGBA(F9B72C, FF),		// Charging
				HEX_TO_RGBA(F9B72C, FF),		// Discharging
				HEX_TO_RGBA(F9B72C, FF),		// Critical level
				HEX_TO_RGBA(E39E1C, FF),		// Border
				HEX_TO_RGBA(C18F17, FF),		// Background (top)
				HEX_TO_RGBA(A57C1B, FF)			// Background (bottom)
			}, { // PARM
				HEX_TO_RGBA(E6D87E, FF),		// Result 1
				HEX_TO_RGBA(E6D87E, FF),		// Result 2
				HEX_TO_RGBA(E6D87E, FF),		// Result 3
				HEX_TO_RGBA(E6D87E, FF),		// Result 4
				HEX_TO_RGBA(E39E1C, FF),		// Border
				HEX_TO_RGBA(C18F17, FF),		// Background (top)
				HEX_TO_RGBA(A57C1B, FF)			// Background (bottom)
			}
		}
	},

	{ "Earth", color_scheme_earth_xpm,
			{  { // CPU
				HEX_TO_RGBA(338C2E, FF),		// User
				HEX_TO_RGBA(81D09B, FF),		// System
				HEX_TO_RGBA(6CBA67, FF),		// Nice
				HEX_TO_RGBA(E5EAED, FF),		// IOWait
				HEX_TO_RGBA(24313A, FF),		// Border
				HEX_TO_RGBA(79BDD8, FF),		// Background (top)
				HEX_TO_RGBA(006287, FF)			// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(73591C, FF),		// User
				HEX_TO_RGBA(73591C, FF),		// Shared
				HEX_TO_RGBA(B6974F, FF),		// Buffers
				HEX_TO_RGBA(E1C584, FF),		// Cached
				HEX_TO_RGBA(24313A, FF),		// Border
				HEX_TO_RGBA(79BDD8, FF),		// Background (top)
				HEX_TO_RGBA(006287, FF)			// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(1D6F61, FF),		// In
				HEX_TO_RGBA(6DC0B2, FF),		// Out
				HEX_TO_RGBA(8DCBFF, FF),		// Local
				HEX_TO_RGBA(24313A, FF),		// Border
				HEX_TO_RGBA(79BDD8, FF),		// Background (top)
				HEX_TO_RGBA(006287, FF)			// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(B6DEED, FF),		// Used
				HEX_TO_RGBA(24313A, FF),		// Border
				HEX_TO_RGBA(79BDD8, FF),		// Background (top)
				HEX_TO_RGBA(006287, FF)			// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(EFDA71, FF),		// Average
				HEX_TO_RGBA(24313A, FF),		// Border
				HEX_TO_RGBA(79BDD8, FF),		// Background (top)
				HEX_TO_RGBA(006287, FF)			// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(23150F, FF),		// Read
				HEX_TO_RGBA(643C34, FF),		// Write
				HEX_TO_RGBA(24313A, FF),		// Border
				HEX_TO_RGBA(79BDD8, FF),		// Background (top)
				HEX_TO_RGBA(006287, FF)			// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(ECEEEF, FF),		// Value
				HEX_TO_RGBA(556167, FF),		// Critical
				HEX_TO_RGBA(24313A, FF),		// Border
				HEX_TO_RGBA(79BDD8, FF),		// Background (top)
				HEX_TO_RGBA(006287, FF)			// Background (bottom)
			}, { // BAT
				HEX_TO_RGBA(B18C6E, FF),		// Charging
				HEX_TO_RGBA(CD620A, FF),		// Discharging
				HEX_TO_RGBA(EECCB0, FF),		// Critical level
				HEX_TO_RGBA(24313A, FF),		// Border
				HEX_TO_RGBA(79BDD8, FF),		// Background (top)
				HEX_TO_RGBA(006287, FF)			// Background (bottom)
			}, { // PARM
				HEX_TO_RGBA(201E36, FF),		// Result 1
				HEX_TO_RGBA(5D579C, FF),		// Result 2
				HEX_TO_RGBA(9C99BA, FF),		// Result 3
				HEX_TO_RGBA(F1F0FF, FF),		// Result 4
				HEX_TO_RGBA(24313A, FF),		// Border
				HEX_TO_RGBA(79BDD8, FF),		// Background (top)
				HEX_TO_RGBA(006287, FF)			// Background (bottom)
			}
		}
	},

	{ "Mars", color_scheme_mars_xpm,
			{  { // CPU
				HEX_TO_RGBA(DCAC9E, FF),		// User
				HEX_TO_RGBA(DCAC9E, FF),		// System
				HEX_TO_RGBA(DCAC9E, FF),		// Nice
				HEX_TO_RGBA(DCAC9E, FF),		// IOWait
				HEX_TO_RGBA(733E34, FF),		// Border
				HEX_TO_RGBA(A27643, FF),		// Background (top)
				HEX_TO_RGBA(45413F, FF)			// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(D66456, FF),		// User
				HEX_TO_RGBA(D66456, FF),		// Shared
				HEX_TO_RGBA(D66456, FF),		// Buffers
				HEX_TO_RGBA(D66456, FF),		// Cached
				HEX_TO_RGBA(733E34, FF),		// Border
				HEX_TO_RGBA(A27643, FF),		// Background (top)
				HEX_TO_RGBA(45413F, FF)			// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(F96412, FF),		// In
				HEX_TO_RGBA(F96412, FF),		// Out
				HEX_TO_RGBA(F96412, FF),		// Local
				HEX_TO_RGBA(733E34, FF),		// Border
				HEX_TO_RGBA(A27643, FF),		// Background (top)
				HEX_TO_RGBA(45413F, FF)			// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(F41D03, FF),		// Used
				HEX_TO_RGBA(733E34, FF),		// Border
				HEX_TO_RGBA(A27643, FF),		// Background (top)
				HEX_TO_RGBA(45413F, FF)			// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(C01C04, FF),		// Average
				HEX_TO_RGBA(733E34, FF),		// Border
				HEX_TO_RGBA(A27643, FF),		// Background (top)
				HEX_TO_RGBA(45413F, FF)			// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(DB604E, FF),		// Read
				HEX_TO_RGBA(DB604E, FF),		// Write
				HEX_TO_RGBA(733E34, FF),		// Border
				HEX_TO_RGBA(A27643, FF),		// Background (top)
				HEX_TO_RGBA(45413F, FF)			// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(FB0E08, FF),		// Value
				HEX_TO_RGBA(FB0E08, FF),		// Critical
				HEX_TO_RGBA(733E34, FF),		// Border
				HEX_TO_RGBA(A27643, FF),		// Background (top)
				HEX_TO_RGBA(45413F, FF)			// Background (bottom)
			}, { // BAT
				HEX_TO_RGBA(F77559, FF),		// Charging
				HEX_TO_RGBA(F77559, FF),		// Discharging
				HEX_TO_RGBA(F77559, FF),		// Critical level
				HEX_TO_RGBA(733E34, FF),		// Border
				HEX_TO_RGBA(A27643, FF),		// Background (top)
				HEX_TO_RGBA(45413F, FF)			// Background (bottom)
			}, { // PARM
				HEX_TO_RGBA(720503, FF),		// Result 1
				HEX_TO_RGBA(720503, FF),		// Result 2
				HEX_TO_RGBA(720503, FF),		// Result 3
				HEX_TO_RGBA(720503, FF),		// Result 4
				HEX_TO_RGBA(733E34, FF),		// Border
				HEX_TO_RGBA(A27643, FF),		// Background (top)
				HEX_TO_RGBA(45413F, FF)			// Background (bottom)
			}
		}
	},

	{ "Jupiter", color_scheme_jupiter_xpm,
			{  { // CPU
				HEX_TO_RGBA(86CF80, FF),		// User
				HEX_TO_RGBA(86CF80, FF),		// System
				HEX_TO_RGBA(86CF80, FF),		// Nice
				HEX_TO_RGBA(86CF80, FF),		// IOWait
				HEX_TO_RGBA(3B763B, FF),		// Border
				HEX_TO_RGBA(C5C0AA, FF),		// Background (top)
				HEX_TO_RGBA(063D06, FF)			// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(1A8A14, FF),		// User
				HEX_TO_RGBA(1A8A14, FF),		// Shared
				HEX_TO_RGBA(1A8A14, FF),		// Buffers
				HEX_TO_RGBA(1A8A14, FF),		// Cached
				HEX_TO_RGBA(3B763B, FF),		// Border
				HEX_TO_RGBA(C5C0AA, FF),		// Background (top)
				HEX_TO_RGBA(063D06, FF)			// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(04FF9E, FF),		// In
				HEX_TO_RGBA(04FF9E, FF),		// Out
				HEX_TO_RGBA(04FF9E, FF),		// Local
				HEX_TO_RGBA(3B763B, FF),		// Border
				HEX_TO_RGBA(C5C0AA, FF),		// Background (top)
				HEX_TO_RGBA(063D06, FF)			// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(7AFB49, FF),		// Used
				HEX_TO_RGBA(3B763B, FF),		// Border
				HEX_TO_RGBA(C5C0AA, FF),		// Background (top)
				HEX_TO_RGBA(063D06, FF)			// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(02FD21, FF),		// Average
				HEX_TO_RGBA(3B763B, FF),		// Border
				HEX_TO_RGBA(C5C0AA, FF),		// Background (top)
				HEX_TO_RGBA(063D06, FF)			// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(ADD49D, FF),		// Read
				HEX_TO_RGBA(ADD49D, FF),		// Write
				HEX_TO_RGBA(3B763B, FF),		// Border
				HEX_TO_RGBA(C5C0AA, FF),		// Background (top)
				HEX_TO_RGBA(063D06, FF)			// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(7DB787, FF),		// Value
				HEX_TO_RGBA(7DB787, FF),		// Critical
				HEX_TO_RGBA(3B763B, FF),		// Border
				HEX_TO_RGBA(C5C0AA, FF),		// Background (top)
				HEX_TO_RGBA(063D06, FF)			// Background (bottom)
			}, { // BAT
				HEX_TO_RGBA(6CE870, FF),		// Charging
				HEX_TO_RGBA(6CE870, FF),		// Discharging
				HEX_TO_RGBA(6CE870, FF),		// Critical level
				HEX_TO_RGBA(3B763B, FF),		// Border
				HEX_TO_RGBA(C5C0AA, FF),		// Background (top)
				HEX_TO_RGBA(063D06, FF)			// Background (bottom)
			}, { // PARM
				HEX_TO_RGBA(4D7A55, FF),		// Result 1
				HEX_TO_RGBA(4D7A55, FF),		// Result 2
				HEX_TO_RGBA(4D7A55, FF),		// Result 3
				HEX_TO_RGBA(4D7A55, FF),		// Result 4
				HEX_TO_RGBA(3B763B, FF),		// Border
				HEX_TO_RGBA(C5C0AA, FF),		// Background (top)
				HEX_TO_RGBA(063D06, FF)			// Background (bottom)
			}
		}
	},

	{ "Uranus", color_scheme_uranus_xpm,
			{  { // CPU
				HEX_TO_RGBA(5E8587, FF),		// User
				HEX_TO_RGBA(5E8587, FF),		// System
				HEX_TO_RGBA(5E8587, FF),		// Nice
				HEX_TO_RGBA(5E8587, FF),		// IOWait
				HEX_TO_RGBA(101010, FF),		// Border
				HEX_TO_RGBA(228499, FF),		// Background (top)
				HEX_TO_RGBA(03444A, FF)			// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(30ABC0, FF),		// User
				HEX_TO_RGBA(30ABC0, FF),		// Shared
				HEX_TO_RGBA(30ABC0, FF),		// Buffers
				HEX_TO_RGBA(30ABC0, FF),		// Cached
				HEX_TO_RGBA(101010, FF),		// Border
				HEX_TO_RGBA(228499, FF),		// Background (top)
				HEX_TO_RGBA(03444A, FF)			// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(54BFC4, FF),		// In
				HEX_TO_RGBA(54BFC4, FF),		// Out
				HEX_TO_RGBA(54BFC4, FF),		// Local
				HEX_TO_RGBA(101010, FF),		// Border
				HEX_TO_RGBA(228499, FF),		// Background (top)
				HEX_TO_RGBA(03444A, FF)			// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(BCDBE3, FF),		// Used
				HEX_TO_RGBA(101010, FF),		// Border
				HEX_TO_RGBA(228499, FF),		// Background (top)
				HEX_TO_RGBA(03444A, FF)			// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(72DEE0, FF),		// Average
				HEX_TO_RGBA(101010, FF),		// Border
				HEX_TO_RGBA(228499, FF),		// Background (top)
				HEX_TO_RGBA(03444A, FF)			// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(5A9EBA, FF),		// Read
				HEX_TO_RGBA(5A9EBA, FF),		// Write
				HEX_TO_RGBA(101010, FF),		// Border
				HEX_TO_RGBA(228499, FF),		// Background (top)
				HEX_TO_RGBA(03444A, FF)			// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(9DADBA, FF),		// Value
				HEX_TO_RGBA(9DADBA, FF),		// Critical
				HEX_TO_RGBA(101010, FF),		// Border
				HEX_TO_RGBA(228499, FF),		// Background (top)
				HEX_TO_RGBA(03444A, FF)			// Background (bottom)
			}, { // BAT
				HEX_TO_RGBA(84D7F9, FF),		// Charging
				HEX_TO_RGBA(84D7F9, FF),		// Discharging
				HEX_TO_RGBA(84D7F9, FF),		// Critical level
				HEX_TO_RGBA(101010, FF),		// Border
				HEX_TO_RGBA(228499, FF),		// Background (top)
				HEX_TO_RGBA(03444A, FF)			// Background (bottom)
			}, { // PARM
				HEX_TO_RGBA(4B9CDE, FF),		// Result 1
				HEX_TO_RGBA(4B9CDE, FF),		// Result 2
				HEX_TO_RGBA(4B9CDE, FF),		// Result 3
				HEX_TO_RGBA(4B9CDE, FF),		// Result 4
				HEX_TO_RGBA(101010, FF),		// Border
				HEX_TO_RGBA(228499, FF),		// Background (top)
				HEX_TO_RGBA(03444A, FF)			// Background (bottom)
			}
		}
	},

	{ "Neptune", color_scheme_neptune_xpm,
			{  { // CPU
				HEX_TO_RGBA(6DAFCF, FF),		// User
				HEX_TO_RGBA(6DAFCF, FF),		// System
				HEX_TO_RGBA(6DAFCF, FF),		// Nice
				HEX_TO_RGBA(6DAFCF, FF),		// IOWait
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(6459CA, FF),		// Background (top)
				HEX_TO_RGBA(3D2B67, FF)			// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(5A7BE4, FF),		// User
				HEX_TO_RGBA(5A7BE4, FF),		// Shared
				HEX_TO_RGBA(5A7BE4, FF),		// Buffers
				HEX_TO_RGBA(5A7BE4, FF),		// Cached
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(6459CA, FF),		// Background (top)
				HEX_TO_RGBA(3D2B67, FF)			// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(9DEAE0, FF),		// In
				HEX_TO_RGBA(9DEAE0, FF),		// Out
				HEX_TO_RGBA(9DEAE0, FF),		// Local
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(6459CA, FF),		// Background (top)
				HEX_TO_RGBA(3D2B67, FF)			// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(8397D6, FF),		// Used
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(6459CA, FF),		// Background (top)
				HEX_TO_RGBA(3D2B67, FF)			// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(B0CAD9, FF),		// Average
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(6459CA, FF),		// Background (top)
				HEX_TO_RGBA(3D2B67, FF)			// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(059FE5, FF),		// Read
				HEX_TO_RGBA(059FE5, FF),		// Write
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(6459CA, FF),		// Background (top)
				HEX_TO_RGBA(3D2B67, FF)			// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(A0BAD5, FF),		// Value
				HEX_TO_RGBA(A0BAD5, FF),		// Critical
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(6459CA, FF),		// Background (top)
				HEX_TO_RGBA(3D2B67, FF)			// Background (bottom)
			}, { // BAT
				HEX_TO_RGBA(6FD2FF, FF),		// Charging
				HEX_TO_RGBA(6FD2FF, FF),		// Discharging
				HEX_TO_RGBA(6FD2FF, FF),		// Critical level
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(6459CA, FF),		// Background (top)
				HEX_TO_RGBA(3D2B67, FF)			// Background (bottom)
			}, { // PARM
				HEX_TO_RGBA(718091, FF),		// Result 1
				HEX_TO_RGBA(718091, FF),		// Result 2
				HEX_TO_RGBA(718091, FF),		// Result 3
				HEX_TO_RGBA(718091, FF),		// Result 4
				HEX_TO_RGBA(142339, FF),		// Border
				HEX_TO_RGBA(6459CA, FF),		// Background (top)
				HEX_TO_RGBA(3D2B67, FF)			// Background (bottom)
			}
		}
	},

	{ "=====" },

	{ "\0" }

};
