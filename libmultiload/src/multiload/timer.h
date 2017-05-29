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

#ifndef ML_HEADER__MULTILOAD_TIMER_H__INCLUDED
#define ML_HEADER__MULTILOAD_TIMER_H__INCLUDED
ML_HEADER_BEGIN

typedef struct _MlMultiloadTimer MlMultiloadTimer;


MlMultiloadTimer *
ml_multiload_timer_new (MlMultiloadNotifier *notifier, MlGraph *g, int interval_ms);

void
ml_multiload_timer_destroy (MlMultiloadTimer *tm);

size_t
ml_multiload_timer_sizeof (MlMultiloadTimer *tm)
ML_FN_SIZEOF;

int
ml_multiload_timer_get_interval (MlMultiloadTimer *tm)
ML_FN_READ_DYNAMIC_PROPERTY;

bool
ml_multiload_timer_set_interval (MlMultiloadTimer *tm, int interval_ms)
ML_FN_WARN_UNUSED_RESULT;

bool
ml_multiload_timer_is_running (MlMultiloadTimer *tm)
ML_FN_READ_DYNAMIC_PROPERTY;

bool
ml_multiload_timer_start (MlMultiloadTimer *tm)
ML_FN_WARN_UNUSED_RESULT;

void
ml_multiload_timer_stop (MlMultiloadTimer *tm);


ML_HEADER_END
#endif /* ML_HEADER__MULTILOAD_TIMER_H__INCLUDED */
