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


#ifndef MLSRV_HEADER__CORE_LOG_H__INCLUDED
#define MLSRV_HEADER__CORE_LOG_H__INCLUDED


typedef enum {
	MULTILOAD_SERVER_LOG_AUTHENTICATION,
	MULTILOAD_SERVER_LOG_INITIALIZATION,
	MULTILOAD_SERVER_LOG_FINALIZATION,
	MULTILOAD_SERVER_LOG_CONNECTION,
	MULTILOAD_SERVER_LOG_RESPONSE,

	MULTILOAD_SERVER_LOG_UNKNOWN
} MultiloadServerLogScope;

void
multiload_server_log_begin   (MultiloadServerLogScope scope, const char *fmt, ...);

void
multiload_server_log_success (MultiloadServerLogScope scope, const char *fmt, ...);

void
multiload_server_log_failure (MultiloadServerLogScope scope, const char *fmt, ...);

void
multiload_server_log_set_verbose (bool v);


#endif /* MLSRV_HEADER__CORE_LOG_H__INCLUDED */
