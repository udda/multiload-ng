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


struct _MlMultiloadNotifier {
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	bool data_available;
};


MlMultiloadNotifier *
ml_multiload_notifier_new ()
{
	MlMultiloadNotifier *tmn = ml_new (MlMultiloadNotifier);
	pthread_cond_init (&tmn->cond, NULL);
	pthread_mutex_init (&tmn->mutex, NULL);
	tmn->data_available = false;

	return tmn;
}

void
ml_multiload_notifier_destroy (MlMultiloadNotifier *tmn)
{
	if_unlikely (tmn == NULL)
		return;

	pthread_cond_destroy (&tmn->cond);
	pthread_mutex_destroy (&tmn->mutex);

	free (tmn);
}

size_t
ml_multiload_notifier_sizeof (MlMultiloadNotifier *tmn)
{
	if_unlikely (tmn == NULL)
		return 0;

	return sizeof (MlMultiloadNotifier);
}

void
ml_multiload_notifier_signal (MlMultiloadNotifier *tmn)
{
	pthread_mutex_lock (&tmn->mutex);
	tmn->data_available = true;
	pthread_cond_signal (&tmn->cond);
	pthread_mutex_unlock (&tmn->mutex);
}

void
ml_multiload_notifier_wait (MlMultiloadNotifier *tmn)
{
	pthread_mutex_lock (&tmn->mutex);
	if (tmn->data_available == false)
		pthread_cond_wait (&tmn->cond, &tmn->mutex);
	tmn->data_available = false;
	pthread_mutex_unlock (&tmn->mutex);
}

bool
ml_multiload_notifier_check (MlMultiloadNotifier *tmn)
{
	bool ret;

	pthread_mutex_lock (&tmn->mutex);
	ret = tmn->data_available;
	tmn->data_available = false;
	pthread_mutex_unlock (&tmn->mutex);

	return ret;
}
