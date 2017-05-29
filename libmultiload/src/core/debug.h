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

#ifndef ML_HEADER__CORE_DEBUG_H__INCLUDED
#define ML_HEADER__CORE_DEBUG_H__INCLUDED
ML_HEADER_BEGIN


typedef enum {
	ML_DEBUG_LEVEL_DEBUG	= 1 << 0,		// extra information, useful for debugging
	ML_DEBUG_LEVEL_INFO		= 1 << 1,		// not an error, normal execution flow
	ML_DEBUG_LEVEL_WARNING	= 1 << 2,		// non fatal error: function can continue execution
	ML_DEBUG_LEVEL_ERROR	= 1 << 3,		// fatal error: function cannot continue execution
	ML_DEBUG_LEVEL_BUG		= 1 << 4		// fatal, unexpected error: might be a bug
} MlDebugLevel;


#define ml_debug(...)	ml_debug_printf_full (ML_DEBUG_LEVEL_DEBUG,   __FUNCTION__, __FILE__, __LINE__, false, __VA_ARGS__)
#define ml_warning(...)	ml_debug_printf_full (ML_DEBUG_LEVEL_WARNING, __FUNCTION__, __FILE__, __LINE__, false, __VA_ARGS__)
#define ml_info(...)	ml_debug_printf_full (ML_DEBUG_LEVEL_INFO,    __FUNCTION__, __FILE__, __LINE__, false, __VA_ARGS__)
#define ml_error(...)	ml_debug_printf_full (ML_DEBUG_LEVEL_ERROR,   __FUNCTION__, __FILE__, __LINE__, false, __VA_ARGS__)
#define ml_bug(...)		ml_debug_printf_full (ML_DEBUG_LEVEL_BUG,     __FUNCTION__, __FILE__, __LINE__, true,  __VA_ARGS__)


char **
ml_debug_get_backtrace (size_t *len_ptr)
ML_FN_COLD;

void
ml_debug_printf_full (MlDebugLevel level, const char *function, const char *file, int line, bool backtrace, const char *fmt, ...)
ML_FN_PRINTF(6,7);

int
ml_debug_get_mask ();

FILE *
ml_debug_get_output_file ()
ML_FN_RETURNS_NONNULL;

void
ml_debug_timespec_init (struct timespec *tm)
ML_FN_COLD;

void
ml_debug_timespec_print_elapsed (struct timespec *tm, bool update)
ML_FN_COLD;

bool
ml_debug_collect_to_zip (const char *filename, int compression_level)
ML_FN_COLD;


ML_HEADER_END
#endif /* ML_HEADER__CORE_DEBUG_H__INCLUDED */
