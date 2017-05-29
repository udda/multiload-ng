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

#ifndef ML_HEADER__DRAWING_SURFACE_H__INCLUDED
#define ML_HEADER__DRAWING_SURFACE_H__INCLUDED
ML_HEADER_BEGIN


typedef struct _MlSurface MlSurface;


MlSurface *
ml_surface_new (int width, int height);

void
ml_surface_destroy (MlSurface *s);

size_t
ml_surface_sizeof (MlSurface *s)
ML_FN_SIZEOF;

void
ml_surface_clear (MlSurface *s);

bool
ml_surface_resize (MlSurface *s, int width, int height)
ML_FN_WARN_UNUSED_RESULT;

void
ml_surface_desaturate (MlSurface *s);

MlColor *
ml_surface_get_pixel (MlSurface *s, int x, int y)
ML_FN_READ_PROPERTY;

void
ml_surface_draw_pixel (MlSurface *s, int x, int y, MlColor *color);

void
ml_surface_draw_border (MlSurface *s, MlColor *border_color, int border_width)
ML_FN_HOT;

void
ml_surface_draw_stripes (MlSurface *s, MlColor *color)
ML_FN_HOT;

void
ml_surface_draw_dataset (MlSurface *s, MlDataset *ds, int border_width, MlColor *colors, uint32_t max)
ML_FN_HOT;

void
ml_surface_fill_background (MlSurface *s, MlGradient *grad);

void
ml_surface_draw_surface (MlSurface *s, int x, int y, MlSurface *source)
ML_FN_HOT;

void
ml_surface_copy_surface (MlSurface *s, int x, int y, MlSurface *source)
ML_FN_HOT;

uint8_t *
ml_surface_get_data (MlSurface *s, int *width, int *height);

uint8_t *
ml_surface_write_to_png_buffer (MlSurface *s, size_t *len_ptr, unsigned compression_level)
ML_FN_MALLOC ML_FN_HOT;

bool
ml_surface_write_to_png_file (MlSurface *s, const char* filename, unsigned compression_level)
ML_FN_WARN_UNUSED_RESULT;


ML_HEADER_END
#endif /* ML_HEADER__DRAWING_SURFACE_H__INCLUDED */
