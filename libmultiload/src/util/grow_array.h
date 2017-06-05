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

#ifndef ML_HEADER__UTIL_GROW_ARRAY_H__INCLUDED
#define ML_HEADER__UTIL_GROW_ARRAY_H__INCLUDED
ML_HEADER_BEGIN


typedef struct _MlGrowArray MlGrowArray;


MlGrowArray *
ml_grow_array_new (size_t starting_len, MlDestroyFunc destroy_fn);

MlGrowArray *
ml_grow_array_new_from_strv (char **strv, MlDestroyFunc destroy_fn, bool deep_copy);

mlPointer *
ml_grow_array_destroy (MlGrowArray *ga, bool free_data, bool append_null_element);

size_t
ml_grow_array_sizeof (MlGrowArray *ga, size_t element_size)
ML_FN_SIZEOF;

size_t
ml_grow_array_sizeof_with_fn (MlGrowArray *ga, MlSizeofFunc sizeof_fn)
ML_FN_SIZEOF;

size_t
ml_grow_array_get_length (MlGrowArray *ga)
ML_FN_READ_PROPERTY;

bool
ml_grow_array_is_empty (MlGrowArray *ga)
ML_FN_READ_PROPERTY;

bool
ml_grow_array_contains (MlGrowArray *ga, mlPointer ptr)
ML_FN_READ_PROPERTY;

mlPointer
ml_grow_array_get (MlGrowArray *ga, size_t pos)
ML_FN_READ_PROPERTY ML_FN_NONNULL(1);

void
ml_grow_array_insert (MlGrowArray *ga, mlPointer ptr, size_t pos);

void
ml_grow_array_append (MlGrowArray *ga, mlPointer ptr);

mlPointer
ml_grow_array_replace (MlGrowArray *ga, mlPointer ptr, size_t pos);

void
ml_grow_array_remove (MlGrowArray *ga, size_t pos);

void
ml_grow_array_move (MlGrowArray *ga, size_t pos, size_t newpos);

void
ml_grow_array_sort (MlGrowArray *ga, MlCompareFunc compare_fn);

#define ml_grow_array_for(ga,index) for (int (index) = 0; (index) < (int)ml_grow_array_get_length (ga); (index)++)

#define ml_grow_array_is_last_index(ga,index) (((size_t)(index)) == ml_grow_array_get_length (ga) - 1)


ML_HEADER_END
#endif /* ML_HEADER__UTIL_GROW_ARRAY_H__INCLUDED */
