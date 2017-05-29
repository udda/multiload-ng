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

#ifndef ML_HEADER__MULTILOAD_SHARED_H__INCLUDED
#define ML_HEADER__MULTILOAD_SHARED_H__INCLUDED
ML_HEADER_BEGIN


// forward definitions
typedef struct _MlProcessMonitor MlProcessMonitor;


typedef struct _MlMultiloadShared {
	/* Objects shared by all Multiload-ng instances */
	locale_t			C_locale;		// C locale instance, used by strto*_l
	MlProcessMonitor *	procmon;		// Process monitor
	char **				dev_fstypes;	// list of filesystems that have an underlying device

	/* Static configuration (loaded once at startup and then unchanged) */
	MlDebugLevel		debug_mask;		// Bitmask of enabled debug levels
	FILE *				debug_output;	// Destination of debug messages


	/* Dynamic configuration (can be changed at runtime) */
	MlConfig *			config;			// Shared (global) config
	// config items
	MlTristate			console_colors;	// whether to use colors when printing to terminal
	bool				procmon_enable; // whether to use the process monitor
} MlMultiloadShared;


void
ml_multiload_shared_init ();

const MlMultiloadShared *
ml_multiload_shared_get ()
ML_FN_RETURNS_NONNULL ML_FN_CONST;


#define ML_SHARED_GET(v) (ml_multiload_shared_get ()->v)


ML_HEADER_END
#endif /* ML_HEADER__MULTILOAD_SHARED_H__INCLUDED */
