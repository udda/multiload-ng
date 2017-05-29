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


struct _MlAssocArray {
	MlGrowArray *array;
	MlDestroyFunc destroy_fn;
};

typedef struct {
	char *	key;
	mlPointer	ptr;
} MlAssocArrayItem;



static MlAssocArrayItem *
ml_assoc_array_item_new (const char *key, mlPointer ptr)
{
	MlAssocArrayItem *aai = ml_new (MlAssocArrayItem);

	aai->key = ml_strdup (key);
	aai->ptr = ptr;

	return aai;
}

static void
ml_assoc_array_item_destroy (MlAssocArrayItem *aai)
{
	if_unlikely (aai != NULL)
		free (aai->key);

	free (aai);
}


static inline MlAssocArrayItem *
ml_assoc_array_get_item (MlAssocArray *aa, unsigned index)
{
	if_unlikely (aa == NULL || aa->array == NULL)
		return NULL;

	return (MlAssocArrayItem*)ml_grow_array_get (aa->array, index);
}

MlAssocArray *
ml_assoc_array_new (int reserved_size, MlDestroyFunc destroy_fn)
{
	if_unlikely (reserved_size < 1)
		return NULL;

	MlAssocArray *aa = ml_new (MlAssocArray);

	aa->array = ml_grow_array_new (reserved_size, (MlDestroyFunc)ml_assoc_array_item_destroy);
	aa->destroy_fn = destroy_fn;

	return aa;
}

void
ml_assoc_array_destroy (MlAssocArray *aa)
{
	if_unlikely (aa == NULL)
		return;

	if_likely (aa->array != NULL) {
		ml_grow_array_for (aa->array, i) {
			MlAssocArrayItem *k = ml_assoc_array_get_item (aa, i);
			if (k != NULL && aa->destroy_fn != NULL)
				aa->destroy_fn (k->ptr);
		}

		ml_grow_array_destroy (aa->array, true, false);
	}

	free (aa);
}

size_t
ml_assoc_array_sizeof (MlAssocArray *aa, size_t element_size)
{
	if_unlikely (aa == NULL || aa->array == NULL || element_size < 1)
		return 0;

	size_t size = sizeof (MlAssocArray);

	size += ml_grow_array_sizeof (aa->array, sizeof(MlAssocArrayItem));

	ml_grow_array_for (aa->array, i) {
		MlAssocArrayItem *k = ml_assoc_array_get_item (aa, i);
		if_unlikely (k == NULL)
			continue;

		size += element_size;
		size += ml_string_sizeof (k->key);
	}

	return size;
}

size_t
ml_assoc_array_sizeof_with_fn (MlAssocArray *aa, MlSizeofFunc sizeof_fn)
{
	if_unlikely (aa == NULL || aa->array == NULL || sizeof_fn == NULL)
		return 0;

	size_t s = sizeof (MlAssocArray);

	s += ml_grow_array_sizeof (aa->array, sizeof(MlAssocArrayItem));

	ml_grow_array_for (aa->array, i) {
		MlAssocArrayItem *k = ml_assoc_array_get_item (aa, i);
		if_unlikely (k == NULL)
			continue;

		s += sizeof_fn (k->ptr);
		s += ml_string_sizeof (k->key);
	}

	return s;
}

void
ml_assoc_array_put (MlAssocArray *aa, const char *key, mlPointer ptr)
{
	if_unlikely (aa == NULL || aa->array == NULL || key == NULL)
		return;

	// search for existing key
	ml_grow_array_for (aa->array, i) {
		MlAssocArrayItem *k = ml_assoc_array_get_item (aa, i);
		if_unlikely (k == NULL)
			break;

		if (ml_string_equals (key, k->key, true)) {
			if (ptr == NULL) { // existing key, remove
				if (aa->destroy_fn != NULL)
					aa->destroy_fn (k->ptr);

				ml_grow_array_remove (aa->array, i);
				return;
			} else { // existing key, update ptr
				k->ptr = ptr;
				return;
			}
		}
	}

	// new key
	MlAssocArrayItem *element = ml_assoc_array_item_new (key, ptr);
	if_likely (element != NULL)
		ml_grow_array_append (aa->array, element);
}

void
ml_assoc_array_remove (MlAssocArray *aa, const char *key)
{
	ml_assoc_array_put (aa, key, NULL);
}

mlPointer
ml_assoc_array_get (MlAssocArray *aa, const char *key)
{
	if_unlikely (aa == NULL || aa->array == NULL)
		return NULL;

	ml_grow_array_for (aa->array, i) {
		MlAssocArrayItem *k = ml_assoc_array_get_item (aa, i);
		if_unlikely (k == NULL)
			break;

		if (ml_string_equals (key, k->key, true))
			return k->ptr;
	}

	return NULL;
}

unsigned
ml_assoc_array_get_size (MlAssocArray *aa)
{
	if_unlikely (aa == NULL || aa->array == NULL)
		return 0;

	return ml_grow_array_get_length (aa->array);
}

bool
ml_assoc_array_get_pair (MlAssocArray *aa, unsigned index, char **key, mlPointer *ptr)
{
	if_unlikely (aa == NULL || aa->array == NULL)
		return false;

	if_unlikely (key == NULL || ptr == NULL)
		return false;

	if (index >= ml_grow_array_get_length (aa->array))
		return false;

	MlAssocArrayItem *k = ml_assoc_array_get_item (aa, index);
	if (k == NULL)
		return false;

	*key = k->key;
	*ptr = k->ptr;
	return true;
}


/* IMPORTANT: albeit this is a char**, its elements belong to the associative
 * array. Only the outermost array must be freed (use free not ml_strv_free) */
char * const *
ml_assoc_array_get_keys (MlAssocArray *aa)
{
	if_unlikely (aa == NULL || aa->array == NULL)
		return NULL;

	size_t len = ml_grow_array_get_length (aa->array);
	char ** arr = ml_new_n (char*, len + 1);

	ml_grow_array_for (aa->array, i) {
		MlAssocArrayItem *k = ml_assoc_array_get_item (aa, i);
		if_unlikely (k == NULL)
			break;

		arr[i] = k->key;
	}
	arr[len] = NULL;

	return (char * const *)arr;
}

void
ml_assoc_array_print (MlAssocArray *aa)
{
	if_unlikely (aa == NULL || aa->array == NULL)
		return;

	ml_grow_array_for (aa->array, i) {
		MlAssocArrayItem *k = ml_assoc_array_get_item (aa, i);
		if_unlikely (k == NULL)
			continue;

		printf("% 3d  [%s] => %p\n", i, k->key, k->ptr);
	}
}
