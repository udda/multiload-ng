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

#include <multiload-server.h>


typedef enum {
	MULTILOAD_SERVER_LOG_BEGIN,
	MULTILOAD_SERVER_LOG_SUCCESS,
	MULTILOAD_SERVER_LOG_FAILURE
} MultiloadServerLogType;

static bool verbose_log = false;

// <editor-fold> Textual representation of enums
const char *
_log_scope_to_string (MultiloadServerLogScope scope)
{
	switch (scope) {
		case MULTILOAD_SERVER_LOG_AUTHENTICATION:
			return "Authentication";

		case MULTILOAD_SERVER_LOG_INITIALIZATION:
			return "Initialization";

		case MULTILOAD_SERVER_LOG_FINALIZATION:
			return "Finalization";

		case MULTILOAD_SERVER_LOG_CONNECTION:
			return "Connection";

		case MULTILOAD_SERVER_LOG_RESPONSE:
			return "Response";

		case MULTILOAD_SERVER_LOG_UNKNOWN:
		default:
			return "??";
	}
}

const char *
_log_type_to_string (MultiloadServerLogType type)
{
	switch (type) {
		case MULTILOAD_SERVER_LOG_BEGIN:
			return "BEGIN";

		case MULTILOAD_SERVER_LOG_SUCCESS:
			return "SUCCESS";

		case MULTILOAD_SERVER_LOG_FAILURE:
			return "FAILURE";

		default:
			return "??";
	}
}
// </editor-fold>

// <editor-fold> Actual implementation
static void
multiload_server_log (MultiloadServerLogScope scope, MultiloadServerLogType type, const char *fmt, va_list va)
{
	if (!verbose_log && type != MULTILOAD_SERVER_LOG_FAILURE)
		return;

	fprintf (stderr, "[ML-SERVER] [%-8s %15s]:  ", _log_type_to_string (type), _log_scope_to_string (scope));
	vfprintf (stderr, fmt, va);
	fprintf (stderr, "\n");
}
// </editor-fold>

// <editor-fold> Public overloads
void
multiload_server_log_begin (MultiloadServerLogScope scope, const char *fmt, ...)
{
	if (fmt == NULL)
		return;

	va_list va;
	va_start (va, fmt);

	multiload_server_log (scope, MULTILOAD_SERVER_LOG_BEGIN, fmt, va);

	va_end (va);
}

void
multiload_server_log_success (MultiloadServerLogScope scope, const char *fmt, ...)
{
	if (fmt == NULL)
		return;

	va_list va;
	va_start (va, fmt);

	multiload_server_log (scope, MULTILOAD_SERVER_LOG_SUCCESS, fmt, va);

	va_end (va);
}

void
multiload_server_log_failure (MultiloadServerLogScope scope, const char *fmt, ...)
{
	if (fmt == NULL)
		return;

	va_list va;
	va_start (va, fmt);

	multiload_server_log (scope, MULTILOAD_SERVER_LOG_FAILURE, fmt, va);

	va_end (va);
}
// </editor-fold>

void
multiload_server_log_set_verbose (bool v)
{
	verbose_log = v;
}
