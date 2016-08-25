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

#include <glib/gi18n-lib.h>

#include "colors.h"


static const gchar magic_header[MULTILOAD_COLOR_SCHEME_HEADER_SIZE] = "MULTILOAD-NG";


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

	if (header->version != MULTILOAD_COLOR_SCHEME_VERSION)
		return MULTILOAD_COLOR_SCHEME_STATUS_WRONG_VERSION;

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

	FILE *f = fopen(filename, "rb");
	if (f == NULL)
		return FALSE;

	rd = fread (&header, sizeof(header), 1, f);
	if (rd != 1) {
		fclose(f);
		return MULTILOAD_COLOR_SCHEME_STATUS_WRONG_FORMAT;
	}

	ret = multiload_color_scheme_validate(&header);
	if (ret != MULTILOAD_COLOR_SCHEME_STATUS_VALID)
		return ret;

	rd = fread(&scheme, sizeof(scheme), 1, f);
	if (rd != 1) {
		fclose(f);
		return MULTILOAD_COLOR_SCHEME_STATUS_WRONG_FORMAT;
	}

	fclose(f);

	multiload_color_scheme_apply(&scheme, ma);
	multiload_refresh(ma);
	return MULTILOAD_COLOR_SCHEME_STATUS_VALID;
}


const MultiloadColorScheme multiload_builtin_color_schemes[] = {
	{ "Multiload-ng",
			{  { // CPU  - hue: 196
				HEX_TO_RGBA(03,6F,96, FF),		// User
				HEX_TO_RGBA(48,BD,E6, FF),		// System
				HEX_TO_RGBA(BE,EE,FF, FF),		// Nice
				HEX_TO_RGBA(00,30,40, FF),		// IOWait
				HEX_TO_RGBA(00,5D,80, FF),		// Border
				HEX_TO_RGBA(13,21,26, FF),		// Background (top)
				HEX_TO_RGBA(00,00,00, FF)		// Background (bottom)
			}, { // MEM  - hue: 151
				HEX_TO_RGBA(03,96,4F, FF),		// User
				HEX_TO_RGBA(43,D1,8D, FF),		// Shared
				HEX_TO_RGBA(BF,FF,E0, FF),		// Buffers
				HEX_TO_RGBA(00,40,21, FF),		// Cached
				HEX_TO_RGBA(00,80,42, FF),		// Border
				HEX_TO_RGBA(13,26,1D, FF),		// Background (top)
				HEX_TO_RGBA(00,00,00, FF)		// Background (bottom)
			}, { // NET  - hue: 53
				HEX_TO_RGBA(E2,CC,05, FF),		// In
				HEX_TO_RGBA(69,60,18, FF),		// Out
				HEX_TO_RGBA(FF,F7,B1, FF),		// Local
				HEX_TO_RGBA(80,71,00, FF),		// Border
				HEX_TO_RGBA(26,24,13, FF),		// Background (top)
				HEX_TO_RGBA(00,00,00, FF)		// Background (bottom)
			}, { // SWAP - hue: 278
				HEX_TO_RGBA(9C,43,D1, FF),		// Used
				HEX_TO_RGBA(51,00,80, FF),		// Border
				HEX_TO_RGBA(1F,13,26, FF),		// Background (top)
				HEX_TO_RGBA(00,00,00, FF)		// Background (bottom)
			}, { // LOAD - hue: 0
				HEX_TO_RGBA(D1,43,43, FF),		// Average
				HEX_TO_RGBA(80,00,00, FF),		// Border
				HEX_TO_RGBA(26,13,13, FF),		// Background (top)
				HEX_TO_RGBA(00,00,00, FF)		// Background (bottom)
			}, { // DISK - hue: 31
				HEX_TO_RGBA(ED,7A,00, FF),		// Read
				HEX_TO_RGBA(FF,85,33, FF),		// Write
				HEX_TO_RGBA(80,42,00, FF),		// Border
				HEX_TO_RGBA(26,1D,13, FF),		// Background (top)
				HEX_TO_RGBA(00,00,00, FF)		// Background (bottom)
			}, { // TEMP hue: 310
				HEX_TO_RGBA(F0,49,D5, FF),		// Value
				HEX_TO_RGBA(80,00,6B, FF),		// Border
				HEX_TO_RGBA(26,13,23, FF),		// Background (top)
				HEX_TO_RGBA(00,00,00, FF)		// Background (bottom)
			}
		}
	},

	{ "Tango",
			{  { // CPU  - Tango Sky Blue
				HEX_TO_RGBA(20,4A,87, FF),		// User
				HEX_TO_RGBA(34,65,A4, FF),		// System
				HEX_TO_RGBA(72,9F,CF, FF),		// Nice
				HEX_TO_RGBA(27,41,66, FF),		// IOWait
				HEX_TO_RGBA(2E,34,36, FF),		// Border
				HEX_TO_RGBA(88,8A,85, FF),		// Background (top)
				HEX_TO_RGBA(55,57,53, FF)		// Background (bottom)
			}, { // MEM  - Tango Chameleon
				HEX_TO_RGBA(4E,9A,06, FF),		// User
				HEX_TO_RGBA(73,D2,16, FF),		// Shared
				HEX_TO_RGBA(8A,E2,34, FF),		// Buffers
				HEX_TO_RGBA(3E,66,18, FF),		// Cached
				HEX_TO_RGBA(2E,34,36, FF),		// Border
				HEX_TO_RGBA(88,8A,85, FF),		// Background (top)
				HEX_TO_RGBA(55,57,53, FF)		// Background (bottom)
			}, { // NET  - Tango Butter
				HEX_TO_RGBA(ED,D4,00, FF),		// In
				HEX_TO_RGBA(C4,A0,00, FF),		// Out
				HEX_TO_RGBA(FC,E9,4F, FF),		// Local
				HEX_TO_RGBA(2E,34,36, FF),		// Border
				HEX_TO_RGBA(88,8A,85, FF),		// Background (top)
				HEX_TO_RGBA(55,57,53, FF)		// Background (bottom)
			}, { // SWAP - Tango Plum
				HEX_TO_RGBA(5C,25,66, FF),		// Used
				HEX_TO_RGBA(2E,34,36, FF),		// Border
				HEX_TO_RGBA(88,8A,85, FF),		// Background (top)
				HEX_TO_RGBA(55,57,53, FF)		// Background (bottom)
			}, { // LOAD - Tango Scarlet Red
				HEX_TO_RGBA(A4,00,00, FF),		// Average
				HEX_TO_RGBA(2E,34,36, FF),		// Border
				HEX_TO_RGBA(88,8A,85, FF),		// Background (top)
				HEX_TO_RGBA(55,57,53, FF)		// Background (bottom)
			}, { // DISK - Tango Orange
				HEX_TO_RGBA(F5,79,00, FF),		// Read
				HEX_TO_RGBA(CE,5C,00, FF),		// Write
				HEX_TO_RGBA(2E,34,36, FF),		// Border
				HEX_TO_RGBA(88,8A,85, FF),		// Background (top)
				HEX_TO_RGBA(55,57,53, FF)		// Background (bottom)
			}, { // TEMP - Tango Aluminium
				HEX_TO_RGBA(BA,BD,B6, FF),		// Value
				HEX_TO_RGBA(2E,34,36, FF),		// Border
				HEX_TO_RGBA(88,8A,85, FF),		// Background (top)
				HEX_TO_RGBA(55,57,53, FF)		// Background (bottom)
			}
		}
	},

	{ "Solarized Dark",
			{  { // CPU  - Solarized Blue
				HEX_TO_RGBA(26,8B,D2, FF),		// User
				HEX_TO_RGBA(65,7B,83, FF),		// System
				HEX_TO_RGBA(83,94,96, FF),		// Nice
				HEX_TO_RGBA(93,A1,A1, FF),		// IOWait
				HEX_TO_RGBA(58,6E,75, FF),		// Border
				HEX_TO_RGBA(07,36,42, FF),		// Background (top)
				HEX_TO_RGBA(00,2B,36, FF)		// Background (bottom)
			}, { // MEM  - Solarized Green
				HEX_TO_RGBA(85,99,00, FF),		// User
				HEX_TO_RGBA(65,7B,83, FF),		// Shared
				HEX_TO_RGBA(83,94,96, FF),		// Buffers
				HEX_TO_RGBA(93,A1,A1, FF),		// Cached
				HEX_TO_RGBA(58,6E,75, FF),		// Border
				HEX_TO_RGBA(07,36,42, FF),		// Background (top)
				HEX_TO_RGBA(00,2B,36, FF)		// Background (bottom)
			}, { // NET  - Solarized Yellow
				HEX_TO_RGBA(B5,89,00, FF),		// In
				HEX_TO_RGBA(65,7B,83, FF),		// Out
				HEX_TO_RGBA(83,94,96, FF),		// Local
				HEX_TO_RGBA(58,6E,75, FF),		// Border
				HEX_TO_RGBA(07,36,42, FF),		// Background (top)
				HEX_TO_RGBA(00,2B,36, FF)		// Background (bottom)
			}, { // SWAP - Solarized Violet
				HEX_TO_RGBA(6C,71,C4, FF),		// Used
				HEX_TO_RGBA(58,6E,75, FF),		// Border
				HEX_TO_RGBA(07,36,42, FF),		// Background (top)
				HEX_TO_RGBA(00,2B,36, FF)		// Background (bottom)
			}, { // LOAD - Solarized Red
				HEX_TO_RGBA(DC,32,2F, FF),		// Average
				HEX_TO_RGBA(58,6E,75, FF),		// Border
				HEX_TO_RGBA(07,36,42, FF),		// Background (top)
				HEX_TO_RGBA(00,2B,36, FF)		// Background (bottom)
			}, { // DISK - Solarized Orange
				HEX_TO_RGBA(CB,4B,16, FF),		// Read
				HEX_TO_RGBA(65,7B,83, FF),		// Write
				HEX_TO_RGBA(58,6E,75, FF),		// Border
				HEX_TO_RGBA(07,36,42, FF),		// Background (top)
				HEX_TO_RGBA(00,2B,36, FF)		// Background (bottom)
			}, { // TEMP - Solarized Magenta
				HEX_TO_RGBA(D3,36,82, FF),		// Value
				HEX_TO_RGBA(58,6E,75, FF),		// Border
				HEX_TO_RGBA(07,36,42, FF),		// Background (top)
				HEX_TO_RGBA(00,2B,36, FF)		// Background (bottom)
			}
		}
	},

	{ "Solarized Light",
			{  { // CPU  - Solarized Blue
				HEX_TO_RGBA(26,8B,D2, FF),		// User
				HEX_TO_RGBA(65,7B,83, FF),		// System
				HEX_TO_RGBA(83,94,96, FF),		// Nice
				HEX_TO_RGBA(93,A1,A1, FF),		// IOWait
				HEX_TO_RGBA(58,6E,75, FF),		// Border
				HEX_TO_RGBA(FD,F6,E3, FF),		// Background (top)
				HEX_TO_RGBA(EE,E8,D5, FF)		// Background (bottom)
			}, { // MEM  - Solarized Green
				HEX_TO_RGBA(85,99,00, FF),		// User
				HEX_TO_RGBA(65,7B,83, FF),		// Shared
				HEX_TO_RGBA(83,94,96, FF),		// Buffers
				HEX_TO_RGBA(93,A1,A1, FF),		// Cached
				HEX_TO_RGBA(58,6E,75, FF),		// Border
				HEX_TO_RGBA(FD,F6,E3, FF),		// Background (top)
				HEX_TO_RGBA(EE,E8,D5, FF)		// Background (bottom)
			}, { // NET  - Solarized Yellow
				HEX_TO_RGBA(B5,89,00, FF),		// In
				HEX_TO_RGBA(65,7B,83, FF),		// Out
				HEX_TO_RGBA(83,94,96, FF),		// Local
				HEX_TO_RGBA(58,6E,75, FF),		// Border
				HEX_TO_RGBA(FD,F6,E3, FF),		// Background (top)
				HEX_TO_RGBA(EE,E8,D5, FF)		// Background (bottom)
			}, { // SWAP - Solarized Violet
				HEX_TO_RGBA(6C,71,C4, FF),		// Used
				HEX_TO_RGBA(58,6E,75, FF),		// Border
				HEX_TO_RGBA(FD,F6,E3, FF),		// Background (top)
				HEX_TO_RGBA(EE,E8,D5, FF)		// Background (bottom)
			}, { // LOAD - Solarized Red
				HEX_TO_RGBA(DC,32,2F, FF),		// Average
				HEX_TO_RGBA(58,6E,75, FF),		// Border
				HEX_TO_RGBA(FD,F6,E3, FF),		// Background (top)
				HEX_TO_RGBA(EE,E8,D5, FF)		// Background (bottom)
			}, { // DISK - Solarized Orange
				HEX_TO_RGBA(CB,4B,16, FF),		// Read
				HEX_TO_RGBA(65,7B,83, FF),		// Write
				HEX_TO_RGBA(58,6E,75, FF),		// Border
				HEX_TO_RGBA(FD,F6,E3, FF),		// Background (top)
				HEX_TO_RGBA(EE,E8,D5, FF)		// Background (bottom)
			}, { // TEMP - Solarized Magenta
				HEX_TO_RGBA(D3,36,82, FF),		// Value
				HEX_TO_RGBA(58,6E,75, FF),		// Border
				HEX_TO_RGBA(FD,F6,E3, FF),		// Background (top)
				HEX_TO_RGBA(EE,E8,D5, FF)		// Background (bottom)
			}
		}
	},

	{ "Fruity", // http://allfreedesigns.com/bright-color-palettes/
			{  { // CPU  - Blueberries
				HEX_TO_RGBA(06,44,90, FF),		// User
				HEX_TO_RGBA(26,86,E0, FF),		// System
				HEX_TO_RGBA(68,CA,FB, FF),		// Nice
				HEX_TO_RGBA(04,23,5A, FF),		// IOWait
				HEX_TO_RGBA(40,40,40, FF),		// Border
				HEX_TO_RGBA(B0,B0,E8, FF),		// Background (top)
				HEX_TO_RGBA(B4,B4,C2, FF)		// Background (bottom)
			}, { // MEM  - Kiwi
				HEX_TO_RGBA(55,64,1F, FF),		// User
				HEX_TO_RGBA(78,92,36, FF),		// Shared
				HEX_TO_RGBA(9A,B4,52, FF),		// Buffers
				HEX_TO_RGBA(27,2C,1E, FF),		// Cached
				HEX_TO_RGBA(40,40,40, FF),		// Border
				HEX_TO_RGBA(72,51,1E, FF),		// Background (top)
				HEX_TO_RGBA(80,97,58, FF)		// Background (bottom)
			}, { // NET  - Lemons
				HEX_TO_RGBA(EE,CB,13, FF),		// In
				HEX_TO_RGBA(B0,83,09, FF),		// Out
				HEX_TO_RGBA(FD,F0,63, FF),		// Local
				HEX_TO_RGBA(40,40,40, FF),		// Border
				HEX_TO_RGBA(EB,DA,82, FF),		// Background (top)
				HEX_TO_RGBA(A7,93,64, FF)		// Background (bottom)
			}, { // SWAP - Grapes
				HEX_TO_RGBA(82,15,32, FF),		// Used
				HEX_TO_RGBA(40,40,40, FF),		// Border
				HEX_TO_RGBA(7A,3C,3F, FF),		// Background (top)
				HEX_TO_RGBA(3B,17,19, FF)		// Background (bottom)
			}, { // LOAD - Cherries
				HEX_TO_RGBA(8A,0C,0D, FF),		// Average
				HEX_TO_RGBA(40,40,40, FF),		// Border
				HEX_TO_RGBA(E0,BE,BC, FF),		// Background (top)
				HEX_TO_RGBA(CE,63,70, FF)		// Background (bottom)
			}, { // DISK - Peaches
				HEX_TO_RGBA(FB,99,24, FF),		// Read
				HEX_TO_RGBA(F3,63,21, FF),		// Write
				HEX_TO_RGBA(40,40,40, FF),		// Border
				HEX_TO_RGBA(F3,D5,AF, FF),		// Background (top)
				HEX_TO_RGBA(F6,BE,84, FF)		// Background (bottom)
			}, { // TEMP - Strawberries
				HEX_TO_RGBA(86,15,14, FF),		// Value
				HEX_TO_RGBA(40,40,40, FF),		// Border
				HEX_TO_RGBA(E3,8B,5E, FF),		// Background (top)
				HEX_TO_RGBA(D5,29,0C, FF)		// Background (bottom)
			}
		}
	},

	{ "Ubuntu Ambiance",
			{  { // CPU
				HEX_TO_RGBA(E9,6F,20, FF),		// User
				HEX_TO_RGBA(E9,6F,20, FF),		// System
				HEX_TO_RGBA(E9,6F,20, FF),		// Nice
				HEX_TO_RGBA(E9,6F,20, FF),		// IOWait
				HEX_TO_RGBA(37,37,37, FF),		// Border
				HEX_TO_RGBA(45,45,45, FF),		// Background (top)
				HEX_TO_RGBA(45,45,45, FF)		// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(E9,6F,20, FF),		// User
				HEX_TO_RGBA(E9,6F,20, FF),		// Shared
				HEX_TO_RGBA(E9,6F,20, FF),		// Buffers
				HEX_TO_RGBA(E9,6F,20, FF),		// Cached
				HEX_TO_RGBA(37,37,37, FF),		// Border
				HEX_TO_RGBA(45,45,45, FF),		// Background (top)
				HEX_TO_RGBA(45,45,45, FF)		// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(E9,6F,20, FF),		// In
				HEX_TO_RGBA(E9,6F,20, FF),		// Out
				HEX_TO_RGBA(E9,6F,20, FF),		// Local
				HEX_TO_RGBA(37,37,37, FF),		// Border
				HEX_TO_RGBA(45,45,45, FF),		// Background (top)
				HEX_TO_RGBA(45,45,45, FF)		// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(E9,6F,20, FF),		// Used
				HEX_TO_RGBA(37,37,37, FF),		// Border
				HEX_TO_RGBA(45,45,45, FF),		// Background (top)
				HEX_TO_RGBA(45,45,45, FF)		// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(E9,6F,20, FF),		// Average
				HEX_TO_RGBA(37,37,37, FF),		// Border
				HEX_TO_RGBA(45,45,45, FF),		// Background (top)
				HEX_TO_RGBA(45,45,45, FF)		// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(E9,6F,20, FF),		// Read
				HEX_TO_RGBA(E9,6F,20, FF),		// Write
				HEX_TO_RGBA(37,37,37, FF),		// Border
				HEX_TO_RGBA(45,45,45, FF),		// Background (top)
				HEX_TO_RGBA(45,45,45, FF)		// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(E9,6F,20, FF),		// Value
				HEX_TO_RGBA(37,37,37, FF),		// Border
				HEX_TO_RGBA(45,45,45, FF),		// Background (top)
				HEX_TO_RGBA(45,45,45, FF)		// Background (bottom)
			}
		}
	},

	{ "Ubuntu Radiance",
			{  { // CPU
				HEX_TO_RGBA(E9,6F,20, FF),		// User
				HEX_TO_RGBA(E9,6F,20, FF),		// System
				HEX_TO_RGBA(E9,6F,20, FF),		// Nice
				HEX_TO_RGBA(E9,6F,20, FF),		// IOWait
				HEX_TO_RGBA(D6,D6,D6, FF),		// Border
				HEX_TO_RGBA(E8,E8,E8, FF),		// Background (top)
				HEX_TO_RGBA(E8,E8,E8, FF)		// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(E9,6F,20, FF),		// User
				HEX_TO_RGBA(E9,6F,20, FF),		// Shared
				HEX_TO_RGBA(E9,6F,20, FF),		// Buffers
				HEX_TO_RGBA(E9,6F,20, FF),		// Cached
				HEX_TO_RGBA(D6,D6,D6, FF),		// Border
				HEX_TO_RGBA(E8,E8,E8, FF),		// Background (top)
				HEX_TO_RGBA(E8,E8,E8, FF)		// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(E9,6F,20, FF),		// In
				HEX_TO_RGBA(E9,6F,20, FF),		// Out
				HEX_TO_RGBA(E9,6F,20, FF),		// Local
				HEX_TO_RGBA(D6,D6,D6, FF),		// Border
				HEX_TO_RGBA(E8,E8,E8, FF),		// Background (top)
				HEX_TO_RGBA(E8,E8,E8, FF)		// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(E9,6F,20, FF),		// Used
				HEX_TO_RGBA(D6,D6,D6, FF),		// Border
				HEX_TO_RGBA(E8,E8,E8, FF),		// Background (top)
				HEX_TO_RGBA(E8,E8,E8, FF)		// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(E9,6F,20, FF),		// Average
				HEX_TO_RGBA(D6,D6,D6, FF),		// Border
				HEX_TO_RGBA(E8,E8,E8, FF),		// Background (top)
				HEX_TO_RGBA(E8,E8,E8, FF)		// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(E9,6F,20, FF),		// Read
				HEX_TO_RGBA(E9,6F,20, FF),		// Write
				HEX_TO_RGBA(D6,D6,D6, FF),		// Border
				HEX_TO_RGBA(E8,E8,E8, FF),		// Background (top)
				HEX_TO_RGBA(E8,E8,E8, FF)		// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(E9,6F,20, FF),		// Value
				HEX_TO_RGBA(D6,D6,D6, FF),		// Border
				HEX_TO_RGBA(E8,E8,E8, FF),		// Background (top)
				HEX_TO_RGBA(E8,E8,E8, FF)		// Background (bottom)
			}
		}
	},

	{ "Linux Mint",
			{  { // CPU
				HEX_TO_RGBA(97,BF,60, FF),		// User
				HEX_TO_RGBA(97,BF,60, FF),		// System
				HEX_TO_RGBA(97,BF,60, FF),		// Nice
				HEX_TO_RGBA(97,BF,60, FF),		// IOWait
				HEX_TO_RGBA(3C,3C,3C, FF),		// Border
				HEX_TO_RGBA(48,48,48, FF),		// Background (top)
				HEX_TO_RGBA(39,39,39, FF)		// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(97,BF,60, FF),		// User
				HEX_TO_RGBA(97,BF,60, FF),		// Shared
				HEX_TO_RGBA(97,BF,60, FF),		// Buffers
				HEX_TO_RGBA(97,BF,60, FF),		// Cached
				HEX_TO_RGBA(3C,3C,3C, FF),		// Border
				HEX_TO_RGBA(48,48,48, FF),		// Background (top)
				HEX_TO_RGBA(39,39,39, FF)		// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(97,BF,60, FF),		// In
				HEX_TO_RGBA(97,BF,60, FF),		// Out
				HEX_TO_RGBA(97,BF,60, FF),		// Local
				HEX_TO_RGBA(3C,3C,3C, FF),		// Border
				HEX_TO_RGBA(48,48,48, FF),		// Background (top)
				HEX_TO_RGBA(39,39,39, FF)		// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(97,BF,60, FF),		// Used
				HEX_TO_RGBA(3C,3C,3C, FF),		// Border
				HEX_TO_RGBA(48,48,48, FF),		// Background (top)
				HEX_TO_RGBA(39,39,39, FF)		// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(97,BF,60, FF),		// Average
				HEX_TO_RGBA(3C,3C,3C, FF),		// Border
				HEX_TO_RGBA(48,48,48, FF),		// Background (top)
				HEX_TO_RGBA(39,39,39, FF)		// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(97,BF,60, FF),		// Read
				HEX_TO_RGBA(97,BF,60, FF),		// Write
				HEX_TO_RGBA(3C,3C,3C, FF),		// Border
				HEX_TO_RGBA(48,48,48, FF),		// Background (top)
				HEX_TO_RGBA(39,39,39, FF)		// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(97,BF,60, FF),		// Value
				HEX_TO_RGBA(3C,3C,3C, FF),		// Border
				HEX_TO_RGBA(48,48,48, FF),		// Background (top)
				HEX_TO_RGBA(39,39,39, FF)		// Background (bottom)
			}
		}
	},

	{ "Arc",
			{  { // CPU
				HEX_TO_RGBA(59,24,E2, FF),		// User
				HEX_TO_RGBA(59,24,E2, FF),		// System
				HEX_TO_RGBA(59,24,E2, FF),		// Nice
				HEX_TO_RGBA(59,24,E2, FF),		// IOWait
				HEX_TO_RGBA(1B,1E,24, FF),		// Border
				HEX_TO_RGBA(38,3C,4A, FF),		// Background (top)
				HEX_TO_RGBA(38,3C,4A, FF)		// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(59,24,E2, FF),		// User
				HEX_TO_RGBA(59,24,E2, FF),		// Shared
				HEX_TO_RGBA(59,24,E2, FF),		// Buffers
				HEX_TO_RGBA(59,24,E2, FF),		// Cached
				HEX_TO_RGBA(1B,1E,24, FF),		// Border
				HEX_TO_RGBA(38,3C,4A, FF),		// Background (top)
				HEX_TO_RGBA(38,3C,4A, FF)		// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(59,24,E2, FF),		// In
				HEX_TO_RGBA(59,24,E2, FF),		// Out
				HEX_TO_RGBA(59,24,E2, FF),		// Local
				HEX_TO_RGBA(1B,1E,24, FF),		// Border
				HEX_TO_RGBA(38,3C,4A, FF),		// Background (top)
				HEX_TO_RGBA(38,3C,4A, FF)		// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(59,24,E2, FF),		// Used
				HEX_TO_RGBA(1B,1E,24, FF),		// Border
				HEX_TO_RGBA(38,3C,4A, FF),		// Background (top)
				HEX_TO_RGBA(38,3C,4A, FF)		// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(59,24,E2, FF),		// Average
				HEX_TO_RGBA(1B,1E,24, FF),		// Border
				HEX_TO_RGBA(38,3C,4A, FF),		// Background (top)
				HEX_TO_RGBA(38,3C,4A, FF)		// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(59,24,E2, FF),		// Read
				HEX_TO_RGBA(59,24,E2, FF),		// Write
				HEX_TO_RGBA(1B,1E,24, FF),		// Border
				HEX_TO_RGBA(38,3C,4A, FF),		// Background (top)
				HEX_TO_RGBA(38,3C,4A, FF)		// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(59,24,E2, FF),		// Value
				HEX_TO_RGBA(1B,1E,24, FF),		// Border
				HEX_TO_RGBA(38,3C,4A, FF),		// Background (top)
				HEX_TO_RGBA(38,3C,4A, FF)		// Background (bottom)
			}
		}
	},

	{ "Numix Dark",
			{  { // CPU
				HEX_TO_RGBA(D6,49,37, FF),		// User
				HEX_TO_RGBA(D6,49,37, FF),		// System
				HEX_TO_RGBA(D6,49,37, FF),		// Nice
				HEX_TO_RGBA(D6,49,37, FF),		// IOWait
				HEX_TO_RGBA(DE,DE,DE, FF),		// Border
				HEX_TO_RGBA(33,33,33, FF),		// Background (top)
				HEX_TO_RGBA(33,33,33, FF)		// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(D6,49,37, FF),		// User
				HEX_TO_RGBA(D6,49,37, FF),		// Shared
				HEX_TO_RGBA(D6,49,37, FF),		// Buffers
				HEX_TO_RGBA(D6,49,37, FF),		// Cached
				HEX_TO_RGBA(DE,DE,DE, FF),		// Border
				HEX_TO_RGBA(33,33,33, FF),		// Background (top)
				HEX_TO_RGBA(33,33,33, FF)		// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(D6,49,37, FF),		// In
				HEX_TO_RGBA(D6,49,37, FF),		// Out
				HEX_TO_RGBA(D6,49,37, FF),		// Local
				HEX_TO_RGBA(DE,DE,DE, FF),		// Border
				HEX_TO_RGBA(33,33,33, FF),		// Background (top)
				HEX_TO_RGBA(33,33,33, FF)		// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(D6,49,37, FF),		// Used
				HEX_TO_RGBA(DE,DE,DE, FF),		// Border
				HEX_TO_RGBA(33,33,33, FF),		// Background (top)
				HEX_TO_RGBA(33,33,33, FF)		// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(D6,49,37, FF),		// Average
				HEX_TO_RGBA(DE,DE,DE, FF),		// Border
				HEX_TO_RGBA(33,33,33, FF),		// Background (top)
				HEX_TO_RGBA(33,33,33, FF)		// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(D6,49,37, FF),		// Read
				HEX_TO_RGBA(D6,49,37, FF),		// Write
				HEX_TO_RGBA(DE,DE,DE, FF),		// Border
				HEX_TO_RGBA(33,33,33, FF),		// Background (top)
				HEX_TO_RGBA(33,33,33, FF)		// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(D6,49,37, FF),		// Value
				HEX_TO_RGBA(DE,DE,DE, FF),		// Border
				HEX_TO_RGBA(33,33,33, FF),		// Background (top)
				HEX_TO_RGBA(33,33,33, FF)		// Background (bottom)
			}
		}
	},

	{ "Numix Light",
			{  { // CPU
				HEX_TO_RGBA(D6,49,37, FF),		// User
				HEX_TO_RGBA(D6,49,37, FF),		// System
				HEX_TO_RGBA(D6,49,37, FF),		// Nice
				HEX_TO_RGBA(D6,49,37, FF),		// IOWait
				HEX_TO_RGBA(33,33,33, FF),		// Border
				HEX_TO_RGBA(DE,DE,DE, FF),		// Background (top)
				HEX_TO_RGBA(DE,DE,DE, FF)		// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(D6,49,37, FF),		// User
				HEX_TO_RGBA(D6,49,37, FF),		// Shared
				HEX_TO_RGBA(D6,49,37, FF),		// Buffers
				HEX_TO_RGBA(D6,49,37, FF),		// Cached
				HEX_TO_RGBA(33,33,33, FF),		// Border
				HEX_TO_RGBA(DE,DE,DE, FF),		// Background (top)
				HEX_TO_RGBA(DE,DE,DE, FF)		// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(D6,49,37, FF),		// In
				HEX_TO_RGBA(D6,49,37, FF),		// Out
				HEX_TO_RGBA(D6,49,37, FF),		// Local
				HEX_TO_RGBA(33,33,33, FF),		// Border
				HEX_TO_RGBA(DE,DE,DE, FF),		// Background (top)
				HEX_TO_RGBA(DE,DE,DE, FF)		// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(D6,49,37, FF),		// Used
				HEX_TO_RGBA(33,33,33, FF),		// Border
				HEX_TO_RGBA(DE,DE,DE, FF),		// Background (top)
				HEX_TO_RGBA(DE,DE,DE, FF)		// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(D6,49,37, FF),		// Average
				HEX_TO_RGBA(33,33,33, FF),		// Border
				HEX_TO_RGBA(DE,DE,DE, FF),		// Background (top)
				HEX_TO_RGBA(DE,DE,DE, FF)		// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(D6,49,37, FF),		// Read
				HEX_TO_RGBA(D6,49,37, FF),		// Write
				HEX_TO_RGBA(33,33,33, FF),		// Border
				HEX_TO_RGBA(DE,DE,DE, FF),		// Background (top)
				HEX_TO_RGBA(DE,DE,DE, FF)		// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(D6,49,37, FF),		// Value
				HEX_TO_RGBA(33,33,33, FF),		// Border
				HEX_TO_RGBA(DE,DE,DE, FF),		// Background (top)
				HEX_TO_RGBA(DE,DE,DE, FF)		// Background (bottom)
			}
		}
	},

	{ "Moon",
			{  { // CPU
				HEX_TO_RGBA(50,50,50, FF),		// User
				HEX_TO_RGBA(50,50,50, FF),		// System
				HEX_TO_RGBA(50,50,50, FF),		// Nice
				HEX_TO_RGBA(50,50,50, FF),		// IOWait
				HEX_TO_RGBA(14,23,39, FF),		// Border
				HEX_TO_RGBA(CB,CB,CB, FF),		// Background (top)
				HEX_TO_RGBA(B5,B5,B5, FF)		// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(77,77,77, FF),		// User
				HEX_TO_RGBA(77,77,77, FF),		// Shared
				HEX_TO_RGBA(77,77,77, FF),		// Buffers
				HEX_TO_RGBA(77,77,77, FF),		// Cached
				HEX_TO_RGBA(14,23,39, FF),		// Border
				HEX_TO_RGBA(CB,CB,CB, FF),		// Background (top)
				HEX_TO_RGBA(B5,B5,B5, FF)		// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(F7,F7,F7, FF),		// In
				HEX_TO_RGBA(F7,F7,F7, FF),		// Out
				HEX_TO_RGBA(F7,F7,F7, FF),		// Local
				HEX_TO_RGBA(14,23,39, FF),		// Border
				HEX_TO_RGBA(CB,CB,CB, FF),		// Background (top)
				HEX_TO_RGBA(B5,B5,B5, FF)		// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(90,90,90, FF),		// Used
				HEX_TO_RGBA(14,23,29, FF),		// Border
				HEX_TO_RGBA(CB,CB,CB, FF),		// Background (top)
				HEX_TO_RGBA(B5,B5,B5, FF)		// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(2D,2D,2D, FF),		// Average
				HEX_TO_RGBA(14,23,39, FF),		// Border
				HEX_TO_RGBA(CB,CB,CB, FF),		// Background (top)
				HEX_TO_RGBA(B5,B5,B5, FF)		// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(61,61,61, FF),		// Read
				HEX_TO_RGBA(05,9F,E5, FF),		// Write
				HEX_TO_RGBA(14,23,39, FF),		// Border
				HEX_TO_RGBA(CB,CB,CB, FF),		// Background (top)
				HEX_TO_RGBA(B5,B5,B5, FF)		// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(E2,E2,E2, FF),		// Value
				HEX_TO_RGBA(14,23,39, FF),		// Border
				HEX_TO_RGBA(CB,CB,CB, FF),		// Background (top)
				HEX_TO_RGBA(B5,B5,B5, FF)		// Background (bottom)
			}
		}
	},

	{ "Venus",
			{  { // CPU
				HEX_TO_RGBA(F9,C2,1A, FF),		// User
				HEX_TO_RGBA(F9,C2,1A, FF),		// System
				HEX_TO_RGBA(F9,C2,1A, FF),		// Nice
				HEX_TO_RGBA(F9,C2,1A, FF),		// IOWait
				HEX_TO_RGBA(E3,9E,1C, FF),		// Border
				HEX_TO_RGBA(C1,8F,17, FF),		// Background (top)
				HEX_TO_RGBA(A5,7C,1B, FF)		// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(E7,D5,20, FF),		// User
				HEX_TO_RGBA(E7,D5,20, FF),		// Shared
				HEX_TO_RGBA(E7,D5,20, FF),		// Buffers
				HEX_TO_RGBA(E7,D5,20, FF),		// Cached
				HEX_TO_RGBA(E3,9E,1C, FF),		// Border
				HEX_TO_RGBA(C1,8F,17, FF),		// Background (top)
				HEX_TO_RGBA(A5,7C,1B, FF)		// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(F4,B5,55, FF),		// In
				HEX_TO_RGBA(F4,B5,55, FF),		// Out
				HEX_TO_RGBA(F4,B5,55, FF),		// Local
				HEX_TO_RGBA(E3,9E,1C, FF),		// Border
				HEX_TO_RGBA(C1,8F,17, FF),		// Background (top)
				HEX_TO_RGBA(A5,7C,1B, FF)		// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(F4,C4,4F, FF),		// Used
				HEX_TO_RGBA(E3,9E,1C, FF),		// Border
				HEX_TO_RGBA(C1,8F,17, FF),		// Background (top)
				HEX_TO_RGBA(A5,7C,1B, FF)		// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(F5,C4,7A, FF),		// Average
				HEX_TO_RGBA(E3,9E,1C, FF),		// Border
				HEX_TO_RGBA(C1,8F,17, FF),		// Background (top)
				HEX_TO_RGBA(A5,7C,1B, FF)		// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(FF,DA,00, FF),		// Read
				HEX_TO_RGBA(FF,DA,00, FF),		// Write
				HEX_TO_RGBA(E3,9E,1C, FF),		// Border
				HEX_TO_RGBA(C1,8F,17, FF),		// Background (top)
				HEX_TO_RGBA(A5,7C,1B, FF)		// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(D0,BA,05, FF),		// Value
				HEX_TO_RGBA(E3,9E,1C, FF),		// Border
				HEX_TO_RGBA(C1,8F,17, FF),		// Background (top)
				HEX_TO_RGBA(A5,7C,1B, FF)		// Background (bottom)
			}
		}
	},

	{ "Earth",
			{  { // CPU
				HEX_TO_RGBA(33,8C,2E, FF),		// User
				HEX_TO_RGBA(81,D0,9B, FF),		// System
				HEX_TO_RGBA(6C,BA,67, FF),		// Nice
				HEX_TO_RGBA(E5,EA,ED, FF),		// IOWait
				HEX_TO_RGBA(24,31,3A, FF),		// Border
				HEX_TO_RGBA(79,BD,D8, FF),		// Background (top)
				HEX_TO_RGBA(00,62,87, FF)		// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(73,59,1C, FF),		// User
				HEX_TO_RGBA(8C,6A,1B, FF),		// Shared
				HEX_TO_RGBA(DE,BC,6B, FF),		// Buffers
				HEX_TO_RGBA(C8,AA,66, FF),		// Cached
				HEX_TO_RGBA(24,31,3A, FF),		// Border
				HEX_TO_RGBA(79,BD,D8, FF),		// Background (top)
				HEX_TO_RGBA(00,62,87, FF)		// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(1D,6F,61, FF),		// In
				HEX_TO_RGBA(6D,C0,B2, FF),		// Out
				HEX_TO_RGBA(8D,CB,FF, FF),		// Local
				HEX_TO_RGBA(24,31,3A, FF),		// Border
				HEX_TO_RGBA(79,BD,D8, FF),		// Background (top)
				HEX_TO_RGBA(00,62,87, FF)		// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(B6,DE,ED, FF),		// Used
				HEX_TO_RGBA(24,31,3A, FF),		// Border
				HEX_TO_RGBA(79,BD,D8, FF),		// Background (top)
				HEX_TO_RGBA(00,62,87, FF)		// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(EF,DA,71, FF),		// Average
				HEX_TO_RGBA(24,31,3A, FF),		// Border
				HEX_TO_RGBA(79,BD,D8, FF),		// Background (top)
				HEX_TO_RGBA(00,62,87, FF)		// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(23,15,0F, FF),		// Read
				HEX_TO_RGBA(64,3C,34, FF),		// Write
				HEX_TO_RGBA(24,31,3A, FF),		// Border
				HEX_TO_RGBA(79,BD,D8, FF),		// Background (top)
				HEX_TO_RGBA(00,62,87, FF)		// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(EC,EE,EF, FF),		// Value
				HEX_TO_RGBA(24,31,3A, FF),		// Border
				HEX_TO_RGBA(79,BD,D8, FF),		// Background (top)
				HEX_TO_RGBA(00,62,87, FF)		// Background (bottom)
			}
		}
	},

	{ "Mars",
			{  { // CPU
				HEX_TO_RGBA(DC,AC,9E, FF),		// User
				HEX_TO_RGBA(DC,AC,9E, FF),		// System
				HEX_TO_RGBA(DC,AC,9E, FF),		// Nice
				HEX_TO_RGBA(DC,AC,9E, FF),		// IOWait
				HEX_TO_RGBA(73,3E,34, FF),		// Border
				HEX_TO_RGBA(A2,76,43, FF),		// Background (top)
				HEX_TO_RGBA(45,41,3F, FF)		// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(D6,64,56, FF),		// User
				HEX_TO_RGBA(D6,64,56, FF),		// Shared
				HEX_TO_RGBA(D6,64,56, FF),		// Buffers
				HEX_TO_RGBA(D6,64,56, FF),		// Cached
				HEX_TO_RGBA(73,3E,34, FF),		// Border
				HEX_TO_RGBA(A2,76,43, FF),		// Background (top)
				HEX_TO_RGBA(45,41,3F, FF)		// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(F9,64,12, FF),		// In
				HEX_TO_RGBA(F9,64,12, FF),		// Out
				HEX_TO_RGBA(F9,64,12, FF),		// Local
				HEX_TO_RGBA(73,3E,34, FF),		// Border
				HEX_TO_RGBA(A2,76,43, FF),		// Background (top)
				HEX_TO_RGBA(45,41,3F, FF)		// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(F4,1D,03, FF),		// Used
				HEX_TO_RGBA(73,3E,34, FF),		// Border
				HEX_TO_RGBA(A2,76,43, FF),		// Background (top)
				HEX_TO_RGBA(45,41,3F, FF)		// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(C0,1C,04, FF),		// Average
				HEX_TO_RGBA(73,3E,34, FF),		// Border
				HEX_TO_RGBA(A2,76,43, FF),		// Background (top)
				HEX_TO_RGBA(45,41,3F, FF)		// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(DB,60,4E, FF),		// Read
				HEX_TO_RGBA(DB,60,4E, FF),		// Write
				HEX_TO_RGBA(73,3E,34, FF),		// Border
				HEX_TO_RGBA(A2,76,43, FF),		// Background (top)
				HEX_TO_RGBA(45,41,3F, FF)		// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(FB,0E,08, FF),		// Value
				HEX_TO_RGBA(73,3E,34, FF),		// Border
				HEX_TO_RGBA(A2,76,43, FF),		// Background (top)
				HEX_TO_RGBA(45,41,3F, FF)		// Background (bottom)
			}
		}
	},

	{ "Jupiter",
			{  { // CPU
				HEX_TO_RGBA(86,CF,80, FF),		// User
				HEX_TO_RGBA(86,CF,80, FF),		// System
				HEX_TO_RGBA(86,CF,80, FF),		// Nice
				HEX_TO_RGBA(86,CF,80, FF),		// IOWait
				HEX_TO_RGBA(3B,76,3B, FF),		// Border
				HEX_TO_RGBA(C5,C0,AA, FF),		// Background (top)
				HEX_TO_RGBA(06,3D,06, FF)		// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(1A,8A,14, FF),		// User
				HEX_TO_RGBA(1A,8A,14, FF),		// Shared
				HEX_TO_RGBA(1A,8A,14, FF),		// Buffers
				HEX_TO_RGBA(1A,8A,14, FF),		// Cached
				HEX_TO_RGBA(3B,76,3B, FF),		// Border
				HEX_TO_RGBA(C5,C0,AA, FF),		// Background (top)
				HEX_TO_RGBA(06,3D,06, FF)		// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(04,FF,9E, FF),		// In
				HEX_TO_RGBA(04,FF,9E, FF),		// Out
				HEX_TO_RGBA(04,FF,9E, FF),		// Local
				HEX_TO_RGBA(3B,76,3B, FF),		// Border
				HEX_TO_RGBA(C5,C0,AA, FF),		// Background (top)
				HEX_TO_RGBA(06,3D,06, FF)		// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(7A,FB,49, FF),		// Used
				HEX_TO_RGBA(3B,76,3B, FF),		// Border
				HEX_TO_RGBA(C5,C0,AA, FF),		// Background (top)
				HEX_TO_RGBA(06,3D,06, FF)		// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(02,FD,21, FF),		// Average
				HEX_TO_RGBA(3B,76,3B, FF),		// Border
				HEX_TO_RGBA(C5,C0,AA, FF),		// Background (top)
				HEX_TO_RGBA(06,3D,06, FF)		// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(AD,D4,9D, FF),		// Read
				HEX_TO_RGBA(AD,D4,9D, FF),		// Write
				HEX_TO_RGBA(3B,76,3B, FF),		// Border
				HEX_TO_RGBA(C5,C0,AA, FF),		// Background (top)
				HEX_TO_RGBA(06,3D,06, FF)		// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(7D,B7,87, FF),		// Value
				HEX_TO_RGBA(3B,76,3B, FF),		// Border
				HEX_TO_RGBA(C5,C0,AA, FF),		// Background (top)
				HEX_TO_RGBA(06,3D,06, FF)		// Background (bottom)
			}
		}
	},

	{ "Uranus",
			{  { // CPU
				HEX_TO_RGBA(5E,85,87, FF),		// User
				HEX_TO_RGBA(5E,85,87, FF),		// System
				HEX_TO_RGBA(5E,85,87, FF),		// Nice
				HEX_TO_RGBA(5E,85,87, FF),		// IOWait
				HEX_TO_RGBA(10,10,10, FF),		// Border
				HEX_TO_RGBA(22,84,99, FF),		// Background (top)
				HEX_TO_RGBA(03,44,4A, FF)		// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(30,AB,C0, FF),		// User
				HEX_TO_RGBA(30,AB,C0, FF),		// Shared
				HEX_TO_RGBA(30,AB,C0, FF),		// Buffers
				HEX_TO_RGBA(30,AB,C0, FF),		// Cached
				HEX_TO_RGBA(10,10,10, FF),		// Border
				HEX_TO_RGBA(22,84,99, FF),		// Background (top)
				HEX_TO_RGBA(03,44,4A, FF)		// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(54,BF,C4, FF),		// In
				HEX_TO_RGBA(54,BF,C4, FF),		// Out
				HEX_TO_RGBA(54,BF,C4, FF),		// Local
				HEX_TO_RGBA(10,10,10, FF),		// Border
				HEX_TO_RGBA(22,84,99, FF),		// Background (top)
				HEX_TO_RGBA(03,44,4A, FF)		// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(BC,DB,E3, FF),		// Used
				HEX_TO_RGBA(10,10,10, FF),		// Border
				HEX_TO_RGBA(22,84,99, FF),		// Background (top)
				HEX_TO_RGBA(03,44,4A, FF)		// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(72,DE,E0, FF),		// Average
				HEX_TO_RGBA(10,10,10, FF),		// Border
				HEX_TO_RGBA(22,84,99, FF),		// Background (top)
				HEX_TO_RGBA(03,44,4A, FF)		// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(5A,9E,BA, FF),		// Read
				HEX_TO_RGBA(5A,9E,BA, FF),		// Write
				HEX_TO_RGBA(10,10,10, FF),		// Border
				HEX_TO_RGBA(22,84,99, FF),		// Background (top)
				HEX_TO_RGBA(03,44,4A, FF)		// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(9D,AD,BA, FF),		// Value
				HEX_TO_RGBA(10,10,10, FF),		// Border
				HEX_TO_RGBA(22,84,99, FF),		// Background (top)
				HEX_TO_RGBA(03,44,4A, FF)		// Background (bottom)
			}
		}
	},

	{ "Neptune",
			{  { // CPU
				HEX_TO_RGBA(6D,AF,CF, FF),		// User
				HEX_TO_RGBA(6D,AF,CF, FF),		// System
				HEX_TO_RGBA(6D,AF,CF, FF),		// Nice
				HEX_TO_RGBA(6D,AF,CF, FF),		// IOWait
				HEX_TO_RGBA(14,23,39, FF),		// Border
				HEX_TO_RGBA(64,59,CA, FF),		// Background (top)
				HEX_TO_RGBA(3D,2B,67, FF)		// Background (bottom)
			}, { // MEM
				HEX_TO_RGBA(5A,7B,E4, FF),		// User
				HEX_TO_RGBA(5A,7B,E4, FF),		// Shared
				HEX_TO_RGBA(5A,7B,E4, FF),		// Buffers
				HEX_TO_RGBA(5A,7B,E4, FF),		// Cached
				HEX_TO_RGBA(14,23,39, FF),		// Border
				HEX_TO_RGBA(64,59,CA, FF),		// Background (top)
				HEX_TO_RGBA(3D,2B,67, FF)		// Background (bottom)
			}, { // NET
				HEX_TO_RGBA(9D,EA,E0, FF),		// In
				HEX_TO_RGBA(9D,EA,E0, FF),		// Out
				HEX_TO_RGBA(9D,EA,E0, FF),		// Local
				HEX_TO_RGBA(14,23,39, FF),		// Border
				HEX_TO_RGBA(64,59,CA, FF),		// Background (top)
				HEX_TO_RGBA(3D,2B,67, FF)		// Background (bottom)
			}, { // SWAP
				HEX_TO_RGBA(83,97,D6, FF),		// Used
				HEX_TO_RGBA(14,23,39, FF),		// Border
				HEX_TO_RGBA(64,59,CA, FF),		// Background (top)
				HEX_TO_RGBA(3D,2B,67, FF)		// Background (bottom)
			}, { // LOAD
				HEX_TO_RGBA(B0,CA,D9, FF),		// Average
				HEX_TO_RGBA(14,23,39, FF),		// Border
				HEX_TO_RGBA(64,59,CA, FF),		// Background (top)
				HEX_TO_RGBA(3D,2B,67, FF)		// Background (bottom)
			}, { // DISK
				HEX_TO_RGBA(05,9F,E5, FF),		// Read
				HEX_TO_RGBA(05,9F,E5, FF),		// Write
				HEX_TO_RGBA(14,23,39, FF),		// Border
				HEX_TO_RGBA(64,59,CA, FF),		// Background (top)
				HEX_TO_RGBA(3D,2B,67, FF)		// Background (bottom)
			}, { // TEMP
				HEX_TO_RGBA(A0,BA,D5, FF),		// Value
				HEX_TO_RGBA(14,23,39, FF),		// Border
				HEX_TO_RGBA(64,59,CA, FF),		// Background (top)
				HEX_TO_RGBA(3D,2B,67, FF)		// Background (bottom)
			}
		}
	},

	{ "\0" }

};
