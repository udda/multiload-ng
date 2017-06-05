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

#ifndef ML_HEADER__GLOBAL_H__INCLUDED
#define ML_HEADER__GLOBAL_H__INCLUDED

/* MULTILOAD-NG BUILD-TIME SETTINGS */


/* Community */
#define ML_HOME_GITHUB					"https://github.com/udda/multiload-ng"
#define ML_HOME_GOOGLEPLUS				"https://plus.google.com/communities/106518305533935900936"

/* Donate */
#define ML_DONATE_PAYPAL				"https://paypal.me/udda"
#define ML_DONATE_BITCOIN				"17BU3bPadWhmMwVe1heCs77wAY5SCMZJw8"

/* MLCaption table size (rows, columns) */
#define ML_CAPTION_TABLE_ROWS			3
#define ML_CAPTION_TABLE_COLUMNS		2

/* Separator for ML_CONFIG_TYPE_STRV elements */
#define ML_CONFIG_STRV_SEPARATOR		"|"

/* Default debug level, if it's not changed through environment variable
 * (see below). Note that ML_DEBUG_LEVEL_BUG messages are always shown */
#define ML_DEBUG_DEFAULT_LEVEL			(ML_DEBUG_LEVEL_WARNING | ML_DEBUG_LEVEL_ERROR)

/* Max number of entries returned by ml_debug_get_backtrace() */
#define ML_DEBUG_BACKTRACE_SIZE			20

/* Environment variable name to select enabled debug levels */
#define ML_DEBUG_LEVEL_ENVIRON_KEY		"MULTILOAD_DEBUG"

/* Environment variable name to redirect debug output to a file */
#define ML_DEBUG_FILE_ENVIRON_KEY		"MULTILOAD_DEBUG_FILE"

/* Max line length for ml_debug_printf_full (size of static buffer) */
#define ML_DEBUG_MAX_LINE_LENGTH		512

/* Max number of elements in a container (must be less than INT32_MAX,
 * but the actual bottleneck is performance rather than memory) */
#define ML_CONTAINER_MAX_ELEMENTS		1000

/* Codename of separator elements. This is required to offer in the API
 * a single function to get element name, regardless of whether they are
 * graphs (which have a codename that is graph type) or separators (whose
 * codename is this string) */
#define ML_ELEMENT_SEPARATOR_NAME		"separator"

/* Priority (nice) of provider threads (each graph
 * has its own provider thread) */
#define ML_PROVIDER_THREAD_PRIORITY		3

/* Priority (nice) of process monitor thread. Should
 * be higher than ML_PROVIDER_THREAD_PRIORITY */
#define ML_PROCESS_MONITOR_PRIORITY		5

/* Interval (in milliseconds) between two updates of MlProcessMonitor.
 * Updates are pretty CPU intensive, because they read and parse a couple
 * of files for each existing process in the system */
#define ML_PROCESS_MONITOR_INTERVAL		500

/* Interval (in milliseconds) between two checks of child process termination.
 * Thread will do a psaudo-busy waiting, sleeping this much after every check */
#define ML_CHILD_SPINLOCK_INTERVAL		5

/* Min number of milliseconds allowed for a timer interval.
 * Remember that lower intervals cause higher CPU load */
#define ML_TIMER_MIN_INTERVAL			10

/* Size of static buffer allocated for printf-like functions (currently
 * ml_grow_buffer_append_printf and ml_strdup_printf).
 * The use of static buffers greatly improve performance (especially with
 * repetitive writes), the downside is that these functions cannot print
 * more than this number of characters at a time */
#define ML_PRINTF_STATIC_BUFFER_SIZE	1024

/* Number of empty slots that will be added to a MlGrowArray
 * every time it runs out of space */
#define ML_GROW_ARRAY_INCREMENT_SIZE	3

/* Number of empty slots that will be added to a MlGrowArray
 * every time it runs out of space */
#define ML_GROW_ARRAY_INCREMENT_SIZE	3

/* Size of the buffer used by functions that read from a file */
#define ML_FREAD_BUFFER_SIZE			200


#endif /* ML_HEADER__GLOBAL_H__INCLUDED */
