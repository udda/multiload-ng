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

#ifndef ML_HEADER__CORE_MEMORY_H__INCLUDED
#define ML_HEADER__CORE_MEMORY_H__INCLUDED
ML_HEADER_BEGIN


#define ml_new(type)			((type*)ml_enforce_calloc  (1, sizeof(type)));
#define ml_new_n(type,n)		((type*)ml_enforce_calloc  (n, sizeof(type)));
#define ml_renew(type,ptr,n)	((type*)ml_enforce_realloc (ptr, (n)*sizeof(type)));

// returns data pointed by ptr as a specific type
#define ML_POINTER_VALUE_AS(ptr, type) ((type) *((type*)ptr))
// use this when pointed data needs to be a lvalue
#define ML_POINTER_LVALUE_AS(ptr, type) (*((type*)ptr))


mlPointer
ml_enforce_calloc (size_t nmemb, size_t size)
ML_FN_RETURNS_NONNULL ML_FN_ALLOC_SIZE(1,2);

mlPointer
ml_enforce_realloc (mlPointer ptr, size_t size)
ML_FN_RETURNS_NONNULL ML_FN_ALLOC_SIZE(2);

void
ml_cleanup_string (char **str);

void
ml_cleanup_pointer (mlPointer *pptr);


ML_HEADER_END
#endif /* ML_HEADER__CORE_MEMORY_H__INCLUDED */
