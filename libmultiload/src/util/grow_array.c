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


struct _MlGrowArray {
	mlPointer *data;
	size_t len;
	size_t allocated_size;

	MlDestroyFunc destroy_fn;
};


MlGrowArray *
ml_grow_array_new (size_t starting_size, MlDestroyFunc destroy_fn)
{
	if_unlikely (starting_size < 1)
		return NULL;

	MlGrowArray *ga = ml_new (MlGrowArray);

	ga->data = ml_new_n (mlPointer, starting_size);
	ga->len = 0;
	ga->allocated_size = starting_size;
	ga->destroy_fn = destroy_fn;

	return ga;
}

MlGrowArray *
ml_grow_array_new_from_strv (char **strv, MlDestroyFunc destroy_fn, bool deep_copy)
{
	if_unlikely (strv == NULL)
		return NULL;

	size_t len = ml_strv_length (strv);

	MlGrowArray *ga = ml_new (MlGrowArray);
	ga->data = ml_new_n (mlPointer, len);
	ga->len = len;
	ga->allocated_size = len;
	ga->destroy_fn = destroy_fn;

	if (deep_copy) {
		for (size_t i = 0; i < len; i++)
			ga->data[i] = strdup(strv[i]);
	} else {
		memcpy (ga->data, strv, len * sizeof(char*));
	}

	return ga;
}

mlPointer *
ml_grow_array_destroy (MlGrowArray *ga, bool free_data, bool append_null_element)
{
	if_unlikely (ga == NULL)
		return NULL;

	mlPointer *data;

	if (free_data) {
		if (ga->destroy_fn) {
			for (size_t i = 0; i < ga->len; i++)
				ga->destroy_fn (ga->data[i]);
		}
		free (ga->data);
		data = NULL;
	} else {
		if (append_null_element) {
			data = ml_renew (mlPointer, ga->data, ga->len + 1);
			data[ga->len] = NULL;
		} else {
			data = ml_renew (mlPointer, ga->data, ga->len);
		}
	}

	free (ga);

	return data;
}

size_t
ml_grow_array_sizeof (MlGrowArray *ga, size_t element_size)
{
	if_unlikely (ga == NULL)
		return 0;

	size_t size = sizeof (MlGrowArray);

	// element_size can be 0, if elements are counted elsewhere
	size += (element_size + sizeof (mlPointer)) * ga->len;

	return size;
}

size_t
ml_grow_array_sizeof_with_fn (MlGrowArray *ga, MlSizeofFunc sizeof_fn) {
	if_unlikely (ga == NULL || sizeof_fn == NULL)
		return 0;

	size_t size = sizeof (MlGrowArray);

	for (size_t i = 0; i < ga->len; i++) {
		size += sizeof_fn (ga->data[i]);
		size += sizeof (mlPointer);
	}

	return size;
}

size_t
ml_grow_array_get_length (MlGrowArray *ga)
{
	if_unlikely (ga == NULL)
		return 0;

	return ga->len;
}

bool
ml_grow_array_is_empty (MlGrowArray *ga)
{
	if_unlikely (ga == NULL)
		return true;

	return ga->len == 0;
}

bool
ml_grow_array_contains (MlGrowArray *ga, mlPointer ptr)
{
	if_unlikely (ga == NULL)
		return false;

	for (size_t i = 0; i < ga->len; i++) {
		if (ga->data[i] == ptr)
			return true;
	}

	return false;
}

mlPointer
ml_grow_array_get (MlGrowArray *ga, size_t pos)
{
	// arguments are guaranteed to be not NULL

	if_unlikely (pos >= ga->len)
		return NULL;

	return ga->data[pos];
}

void
ml_grow_array_insert (MlGrowArray *ga, mlPointer ptr, size_t pos)
{
	if_unlikely (ga == NULL || ptr == NULL || pos > ga->len)
		return;

	// if array is full, make it grow
	if (ga->len >= ga->allocated_size) {
		ga->allocated_size += ML_GROW_ARRAY_INCREMENT_SIZE;
		ga->data = ml_renew (mlPointer, ga->data, ga->allocated_size);
	}

	memmove (&ga->data[pos+1], &ga->data[pos], sizeof(mlPointer)*(ga->len - pos));

	ga->data[pos] = ptr;
	ga->len++;
}

void
ml_grow_array_append (MlGrowArray *ga, mlPointer ptr)
{
	if_unlikely (ga == NULL || ptr == NULL)
		return;

	ml_grow_array_insert (ga, ptr, ga->len);
}

mlPointer
ml_grow_array_replace (MlGrowArray *ga, mlPointer ptr, size_t pos)
{
	if_unlikely (ga == NULL || ptr == NULL || pos >= ga->len)
		return NULL;

	mlPointer old_ptr = ga->data[pos];
	ga->data[pos] = ptr;

	return old_ptr;
}

void
ml_grow_array_remove (MlGrowArray *ga, size_t pos)
{
	if_unlikely (ga == NULL || pos >= ga->len)
		return;

	if (ga->destroy_fn)
		ga->destroy_fn (ga->data[pos]);

	if (pos < ga->len-1)
		memmove (&ga->data[pos], &ga->data[pos+1], sizeof(mlPointer)*(ga->len - pos));

	ga->len--;
}

void
ml_grow_array_move (MlGrowArray *ga, size_t pos, size_t newpos)
{
	if (pos == newpos)
		return;

	mlPointer ptr = ml_grow_array_get (ga, pos);
	if_unlikely (ptr == NULL)
		return;

	if (pos < newpos) {
		// shift left
		memmove (&ga->data[pos], &ga->data[pos+1], sizeof(mlPointer)*(newpos - pos));
	} else {
		// shift right
		memmove (&ga->data[newpos+1], &ga->data[newpos], sizeof(mlPointer)*(pos - newpos));
	}
	ga->data[newpos] = ptr;
}

void
ml_grow_array_sort (MlGrowArray *ga, MlCompareFunc compare_fn)
{
	if_unlikely (ga == NULL || compare_fn == NULL)
		return;

	qsort (ga->data, ga->len, sizeof(mlPointer), compare_fn);
}
