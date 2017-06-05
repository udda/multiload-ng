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

void
ml_ansi_print_sequence (FILE *f, MlAnsiColor bg, MlAnsiColor fg, bool bold)
{
	fputc ('\033', f);
	fputc ('[', f);
	fputc (bold ? '1' : '0', f);
	if (fg != ML_ANSI_DEFAULT) {
		fputc (';', f);
		fputc ('3', f);
		fputc (('0'+fg), f);
	}
	if (bg != ML_ANSI_DEFAULT) {
		fputc (';', f);
		fputc ('4', f);
		fputc (('0'+bg), f);
	}
	fputc ('m', f);
	fputc ('\0', f);
}
