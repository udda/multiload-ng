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


struct _MlMultiloadTimer {
	pthread_t thread;

	MlGraph *graph;

	bool is_running;
	struct timespec interval;
	pthread_mutex_t mutex;

	MlMultiloadNotifier *notifier;
};

static void *
_ml_multiload_timer_routine (mlPointer user_data)
{
	if_unlikely (user_data == NULL)
		return NULL;

	MlMultiloadTimer *tm = (MlMultiloadTimer*)user_data;
	struct timespec interval_local_copy;

	ml_thread_set_priority (ML_PROVIDER_THREAD_PRIORITY);

	while (tm->graph != NULL) {
		/* the thread should not terminate while the graph is collecting data,
		 * otherwise bad things could happen (memory leaks, files left open, etc)
		 * so we disable cancellation for this thread just before ml_graph_collect_data
		 * and then enable it again */
		pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, NULL);
		if (ml_graph_collect_data (tm->graph))
			ml_multiload_notifier_signal (tm->notifier);
		pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);

		// now sleep
		pthread_mutex_lock (&tm->mutex);
		memcpy (&interval_local_copy, &tm->interval, sizeof(struct timespec));
		pthread_mutex_unlock (&tm->mutex);

		nanosleep (&interval_local_copy, NULL); // nanosleep is a pthread cancellation point
	}

	return NULL;
}

MlMultiloadTimer*
ml_multiload_timer_new (MlMultiloadNotifier *notifier, MlGraph *g, int interval_ms)
{
	if_unlikely (notifier == NULL || g == NULL)
		return NULL;

	MlMultiloadTimer *tm = ml_new (MlMultiloadTimer);
	tm->graph = g;
	tm->notifier = notifier;
	tm->is_running = false;

	pthread_mutex_init (&tm->mutex, NULL);

	if_unlikely (!ml_multiload_timer_set_interval (tm, interval_ms)) {
		ml_multiload_timer_destroy (tm);
		return NULL;
	}

	return tm;
}

void
ml_multiload_timer_destroy (MlMultiloadTimer *tm)
{
	if_unlikely (tm == NULL)
		return;

	tm->graph = NULL; // avoid further data collect operations

	ml_multiload_timer_stop (tm);
	pthread_mutex_destroy (&tm->mutex);
	free (tm);
}

size_t
ml_multiload_timer_sizeof (MlMultiloadTimer *tm)
{
	if_unlikely (tm == NULL)
		return 0;

	return sizeof (MlMultiloadTimer);
}

int
ml_multiload_timer_get_interval (MlMultiloadTimer *tm)
{
	if_unlikely (tm == NULL)
		return -1;

	pthread_mutex_lock (&tm->mutex);
	int ms = ml_timespec_to_milliseconds (&tm->interval);
	pthread_mutex_unlock (&tm->mutex);

	return ms;
}

bool
ml_multiload_timer_set_interval (MlMultiloadTimer *tm, int interval_ms)
{
	if_unlikely (tm == NULL)
		return false;

	if_unlikely (interval_ms < ML_TIMER_MIN_INTERVAL) {
		ml_warning ("Interval not set: value too short (%d)", interval_ms);
		return false;
	}

	pthread_mutex_lock (&tm->mutex);
	ml_milliseconds_to_timespec (&tm->interval, interval_ms);
	pthread_mutex_unlock (&tm->mutex);

	return true;
}

bool
ml_multiload_timer_is_running (MlMultiloadTimer *tm)
{
	if_unlikely (tm == NULL)
		return false;

	return tm->is_running;
}

bool
ml_multiload_timer_start (MlMultiloadTimer *tm)
{
	if_unlikely (tm == NULL)
		return false;

	if (tm->is_running)
		return true;

	if (pthread_create (&tm->thread, NULL, _ml_multiload_timer_routine, tm) != 0)
		return false;

	pthread_mutex_lock (&tm->mutex);
	tm->is_running = true;
	pthread_mutex_unlock (&tm->mutex);

	return true;
}

void
ml_multiload_timer_stop (MlMultiloadTimer *tm)
{
	if_unlikely (tm == NULL)
		return;

	if (!tm->is_running)
		return;

	pthread_cancel (tm->thread);

	// wait for termination
	pthread_join (tm->thread, NULL);

	pthread_mutex_lock (&tm->mutex);
	tm->is_running = false;
	pthread_mutex_unlock (&tm->mutex);

	return;
}
