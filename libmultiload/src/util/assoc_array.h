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

#ifndef ML_HEADER__UTIL_ASSOC_ARRAY_H__INCLUDED
#define ML_HEADER__UTIL_ASSOC_ARRAY_H__INCLUDED
ML_HEADER_BEGIN


typedef struct _MlAssocArray MlAssocArray;


MlAssocArray *
ml_assoc_array_new (int reserved_size, MlDestroyFunc destroy_fn);

void
ml_assoc_array_destroy (MlAssocArray *aa);

size_t
ml_assoc_array_sizeof (MlAssocArray *aa, size_t element_size)
ML_FN_SIZEOF;

size_t
ml_assoc_array_sizeof_with_fn (MlAssocArray *aa, MlSizeofFunc sizeof_fn)
ML_FN_COLD;

void
ml_assoc_array_put (MlAssocArray *aa, const char *key, mlPointer ptr);

void
ml_assoc_array_remove (MlAssocArray *aa, const char *key);

mlPointer
ml_assoc_array_get (MlAssocArray *aa, const char *key)
ML_FN_READ_DYNAMIC_PROPERTY;

unsigned
ml_assoc_array_get_size (MlAssocArray *aa)
ML_FN_READ_PROPERTY;

bool
ml_assoc_array_get_pair (MlAssocArray *aa, unsigned index, char **key, mlPointer *ptr);

char * const *
ml_assoc_array_get_keys (MlAssocArray *aa);

void
ml_assoc_array_print (MlAssocArray *aa)
ML_FN_COLD;


ML_HEADER_END
#endif /* ML_HEADER__UTIL_ASSOC_ARRAY_H__INCLUDED */
