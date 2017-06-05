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

#ifndef ML_HEADER__UTIL_ANSI_ESCAPE_H__INCLUDED
#define ML_HEADER__UTIL_ANSI_ESCAPE_H__INCLUDED
ML_HEADER_BEGIN


typedef enum {
	ML_ANSI_BLACK	= 0,
	ML_ANSI_RED		= 1,
	ML_ANSI_GREEN	= 2,
	ML_ANSI_YELLOW	= 3,
	ML_ANSI_BLUE	= 4,
	ML_ANSI_MAGENTA	= 5,
	ML_ANSI_CYAN	= 6,
	ML_ANSI_WHITE	= 7,

	ML_ANSI_DEFAULT = 100
} MlAnsiColor;


void
ml_ansi_print_sequence (FILE *f, MlAnsiColor bg, MlAnsiColor fg, bool bold);


ML_HEADER_END
#endif /* ML_HEADER__UTIL_ANSI_ESCAPE_H__INCLUDED */
