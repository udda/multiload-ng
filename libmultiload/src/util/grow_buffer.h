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

#ifndef ML_HEADER__UTIL_GROW_BUFFER_H__INCLUDED
#define ML_HEADER__UTIL_GROW_BUFFER_H__INCLUDED
ML_HEADER_BEGIN


typedef struct _MlGrowBuffer MlGrowBuffer;


MlGrowBuffer *
ml_grow_buffer_new (size_t starting_len)
ML_FN_RETURNS_NONNULL;

MlGrowBuffer *
ml_grow_buffer_new_from_buffer (mlPointer buf, size_t len);

MlGrowBuffer *
ml_grow_buffer_new_from_string (char *str);

MlGrowBuffer *
ml_grow_buffer_new_from_file (const char *path);

mlPointer
ml_grow_buffer_destroy (MlGrowBuffer *gbuf, bool free_data);

size_t
ml_grow_buffer_sizeof (MlGrowBuffer *gbuf)
ML_FN_SIZEOF;

void
ml_cleanup_MlGrowBuffer (MlGrowBuffer **p);

void
ml_grow_buffer_rewind (MlGrowBuffer *gbuf);

mlPointer
ml_grow_buffer_get_data (MlGrowBuffer *gbuf)
ML_FN_READ_PROPERTY;

#define ml_grow_buffer_get_string(gbuf) (char*)ml_grow_buffer_get_data(gbuf)

size_t
ml_grow_buffer_get_length (MlGrowBuffer *gbuf)
ML_FN_READ_PROPERTY;

void
ml_grow_buffer_append (MlGrowBuffer *gbuf, mlPointer buf, size_t len)
ML_FN_HOT;

void
ml_grow_buffer_append_string (MlGrowBuffer *gbuf, char *str)
ML_FN_HOT;

void
ml_grow_buffer_append_printf (MlGrowBuffer *gbuf, const char *fmt, ...)
ML_FN_HOT;

void
ml_grow_buffer_append_vprintf (MlGrowBuffer *gbuf, const char *fmt, va_list va)
ML_FN_HOT;

bool
ml_grow_buffer_append_file (MlGrowBuffer *gbuf, const char *path)
ML_FN_HOT ML_FN_WARN_UNUSED_RESULT;

void
ml_grow_buffer_append_grow_buffer (MlGrowBuffer *gbuf, MlGrowBuffer *append)
ML_FN_HOT;


ML_HEADER_END
#endif /* ML_HEADER__UTIL_GROW_BUFFER_H__INCLUDED */
