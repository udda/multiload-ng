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

#ifndef ML_HEADER__CAPTION_H__INCLUDED
#define ML_HEADER__CAPTION_H__INCLUDED
ML_HEADER_BEGIN


typedef struct _MlCaption MlCaption;


typedef enum {
	ML_CAPTION_COMPONENT_HEADER,
	ML_CAPTION_COMPONENT_BODY,
	ML_CAPTION_COMPONENT_FOOTER
} MlCaptionComponent;


MlCaption *
ml_caption_new ()
ML_FN_RETURNS_NONNULL;

void
ml_caption_destroy (MlCaption *cap);

size_t
ml_caption_sizeof (MlCaption *cap)
ML_FN_SIZEOF;

void
ml_caption_clear (MlCaption *cap);

void
ml_caption_set (MlCaption *cap, MlCaptionComponent component, const char *fmt, ...)
ML_FN_PRINTF(3,4);

void
ml_caption_append (MlCaption *cap, MlCaptionComponent component, const char *fmt, ...)
ML_FN_PRINTF(3,4);

void
ml_caption_set_table_data (MlCaption *cap, int row, int col, const char *fmt, ...)
ML_FN_PRINTF(4,5);

void
ml_caption_append_table_data (MlCaption *cap, int row, int col, const char *fmt, ...)
ML_FN_PRINTF(4,5);

const char *
ml_caption_get (MlCaption *cap, MlCaptionComponent component)
ML_FN_READ_PROPERTY;

const char *
ml_caption_get_table_data (MlCaption *cap, int row, int col)
ML_FN_READ_PROPERTY;

int
ml_caption_get_table_size (MlCaption *cap);

cJSON *
ml_caption_to_json (MlCaption *cap);


ML_HEADER_END
#endif /* ML_HEADER__CAPTION_H__INCLUDED */
