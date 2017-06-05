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

#ifndef ML_HEADER__UTIL_SYSTEM_H__INCLUDED
#define ML_HEADER__UTIL_SYSTEM_H__INCLUDED
ML_HEADER_BEGIN


int
ml_timespec_to_milliseconds (struct timespec *tm);

void
ml_milliseconds_to_timespec (struct timespec *tm, int ms);

bool
ml_thread_set_priority (int priority);

bool
ml_execute_cmdline_sync (const char *cmdline, const char *working_dir, int timeout_ms, MlGrowBuffer *gbuf_stdout, MlGrowBuffer *gbuf_stderr, MlGrowBuffer *gbuf_report, int *exit_status);

bool
ml_execute_cmdline_async (const char *cmdline, const char *working_dir, int timeout_ms, MlGrowBuffer *gbuf_report);


ML_HEADER_END
#endif /* ML_HEADER__UTIL_SYSTEM_H__INCLUDED */
