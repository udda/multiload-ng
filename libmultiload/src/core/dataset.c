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


struct _MlDataset {
	int				n_cols;		// number of columns
	int				size;		// length of every column
	uint32_t *		data;		// raw data (from providers) (n_cols*size elements)

	int				head;		// first element of the ring buffer;

	MlDatasetMode	mode;		// dataset mode
	uint32_t *		ceilings;	// ceiling of each record (size elements)

	pthread_mutex_t mutex;		// dataset access must be synchronized (eg. to avoid push/resize race)
};


// fast macros without sanity checks, for internal use only
#define DATASET_REAL_INDEX(ds,index) (((ds)->size + (ds)->head - (index) - 1) % (ds)->size)
#define DATASET_GET_ENTRY(ds,index) (&(ds)->data[ (ds)->n_cols * DATASET_REAL_INDEX((ds),(index))])


MlDataset *
ml_dataset_new (int n_cols, int size, MlDatasetMode mode)
{
	if_unlikely (n_cols < 1 || size < 1)
		return NULL;

	MlDataset *ds = ml_new (MlDataset);

	ds->n_cols		= n_cols;
	ds->size		= size;
	ds->head		= 0;
	ds->mode		= mode;
	ds->data		= ml_new_n (uint32_t, n_cols * size);
	ds->ceilings	= ml_new_n (uint32_t, size);

	pthread_mutex_init (&ds->mutex, NULL);

	return ds;
}

void
ml_dataset_destroy (MlDataset *ds)
{
	if_unlikely (ds == NULL)
		return;

	pthread_mutex_destroy (&ds->mutex);

	free (ds->data);
	free (ds->ceilings);

	free (ds);
}

size_t
ml_dataset_sizeof (MlDataset *ds)
{
	if (ds == NULL || ds->size < 1 || ds->n_cols < 1)
		return 0;

	size_t size = sizeof (MlDataset);
	size += sizeof(uint32_t) * ds->size * ds->n_cols; // data
	size += sizeof(uint32_t) * ds->size; // ceilings

	return size;
}

MlDatasetMode
ml_dataset_get_mode (MlDataset *ds)
{
	if_unlikely (ds == NULL)
		return ML_INVALID;

	return ds->mode;
}

int
ml_dataset_get_n_cols (MlDataset *ds)
{
	if_unlikely (ds == NULL)
		return -1;

	return ds->n_cols;
}

uint32_t *
ml_dataset_get_entry (MlDataset *ds, int index)
{
	return DATASET_GET_ENTRY(ds,index);
}

uint32_t
ml_dataset_get_value (MlDataset *ds, int index, int column)
{
	// arguments are guaranteed to be not NULL
	if (column < 0 || column >= ds->n_cols)
		return 0;

	uint32_t *entry = DATASET_GET_ENTRY(ds,index);
	return entry[column];
}

uint32_t
ml_dataset_get_ceiling (MlDataset *ds, int index)
{
	// arguments are guaranteed to be not NULL
	return ds->ceilings[ DATASET_REAL_INDEX(ds,index) ];
}

uint32_t
ml_dataset_get_max_ceiling (MlDataset *ds)
{
	// arguments are guaranteed to be not NULL
	return array_max_uint (ds->ceilings, ds->size);
}

void
ml_dataset_push_entry (MlDataset *ds, uint32_t *data, int data_length)
{
	if_unlikely (ds == NULL)
		return;

	if (
		(ds->mode == ML_DATASET_MODE_PROPORTIONAL && data_length != ds->n_cols+1) ||
		(ds->mode == ML_DATASET_MODE_ABSOLUTE && data_length != ds->n_cols+1) ||
		(ds->mode == ML_DATASET_MODE_INDEPENDENT && data_length != ds->n_cols)
	) {
		ml_error ("Cannot push entry into dataset: wrong data length");
		return;
	}

	pthread_mutex_lock (&ds->mutex);

	ds->head = (ds->size + ds->head - 1) % ds->size;
	memcpy (&ds->data[ds->n_cols * ds->head], data, ds->n_cols * sizeof(uint32_t));

	switch (ds->mode) {
		case ML_DATASET_MODE_PROPORTIONAL:
			// ceiling is additional data element
			if (data[ds->n_cols] < array_sum_uint (data, ds->n_cols)) {
				ml_error ("Cannot push entry into dataset: ceiling too low");
				break;
			}

			ds->ceilings[ds->head] = data[ds->n_cols];
			break;
		case ML_DATASET_MODE_ABSOLUTE:
			// ceiling is additional data element, if greater than the sum of the values
			ds->ceilings[ds->head] = MAX (data[ds->n_cols], array_sum_uint (data, ds->n_cols));
			break;
		case ML_DATASET_MODE_INDEPENDENT:
			// ceiling is the max value
			ds->ceilings[ds->head] = array_max_uint (data, ds->n_cols);
			break;
		default:
			ml_bug ("Unknown dataset mode (%d)", ds->mode);
			break;
	}

	pthread_mutex_unlock (&ds->mutex);
}

bool
ml_dataset_resize (MlDataset *ds, int newsize)
{
	if_unlikely (ds == NULL || newsize < 0)
		return false;

	pthread_mutex_lock (&ds->mutex);

	uint32_t *newdata		= ml_new_n (uint32_t, ds->n_cols * newsize);
	uint32_t *newceilings	= ml_new_n (uint32_t, newsize);

	// copy ordered into newdata and newceilings
	for (int i = 0; i<newsize && i<ds->size; i++) {
		int orig_index = ds->size - 1 - i;

		uint32_t *orig = ml_dataset_get_entry (ds, orig_index);
		memcpy (&newdata[ds->n_cols * i], orig, ds->n_cols * sizeof(uint32_t));

		newceilings[i] = ml_dataset_get_ceiling (ds, orig_index);
	}

	// delete old data
	free (ds->data);
	free (ds->ceilings);
	ds->head = 0;

	// apply new data
	ds->size = newsize;
	ds->data = newdata;
	ds->ceilings = newceilings;

	pthread_mutex_unlock (&ds->mutex);

	return true;
}

void
ml_dataset_clear (MlDataset *ds)
{
	if_unlikely (ds == NULL || ds->data == NULL)
		return;

	pthread_mutex_lock (&ds->mutex);
	memset (ds->data, 0, ds->n_cols * ds->size * sizeof (uint32_t));
	pthread_mutex_unlock (&ds->mutex);
}


void
ml_dataset_dump (MlDataset *ds)
{
	if_unlikely (ds == NULL)
		return;

	pthread_mutex_lock (&ds->mutex);
	printf ("-----\n");
	printf ("RAW (head=%d, max_ceil=%"PRIu32"): \n", ds->head, ml_dataset_get_max_ceiling(ds));

	for (int i = 0; i < ds->size; i++) {
		uint32_t *entry = &ds->data[ ds->n_cols * i ];
		printf ("CEIL(% 8d) [ ", ds->ceilings[i]);
		for (int j = 0; j < ds->n_cols; j++)
			printf ("% 8d ", entry[j]);
		printf ("] -> ordered %d\n", (ds->size+ds->head-i-1)%ds->size);
	}

	printf ("ORDERED: \n");

	for (int i = 0; i < ds->size; i++) {
		uint32_t *entry = ml_dataset_get_entry (ds, i);
		printf ("CEIL(% 8d) [ ", ds->ceilings[DATASET_REAL_INDEX (ds, i)]);
		for (int j = 0; j < ds->n_cols; j++)
			printf ("% 8d ", entry[j]);
		printf ("] -> raw %d\n", DATASET_REAL_INDEX (ds, i));
	}

	printf ("-----\n\n");
	pthread_mutex_unlock (&ds->mutex);
}
