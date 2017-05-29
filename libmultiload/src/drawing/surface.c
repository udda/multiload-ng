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


struct _MlSurface {
	int width;
	int height;
	union {
		MlColor *pixels;
		uint8_t *pixdata;
	};
};


/* this macro can be used instead of ml_surface_get_pixel when there is no
 * need to check arguments (eg. internal functions). If arguments are
 * guaranteed to be valid, returns always valid data */
#define SURFACE_GET_PIXEL(s,x,y) (&((s)->pixels)[(x) + (y)*((s)->width)])


MlSurface *
ml_surface_new (int width, int height)
{
	MlSurface *s = ml_new (MlSurface);

	if_unlikely (!ml_surface_resize (s, width, height)) {
		free (s);
		return NULL;
	}

	return s;
}

void
ml_surface_destroy (MlSurface *s)
{
	if_unlikely (s == NULL)
		return;

	free (s->pixels);
	free (s);
}

size_t
ml_surface_sizeof (MlSurface *s)
{
	if_unlikely (s == NULL)
		return 0;

	size_t size = sizeof (MlSurface);
	size += sizeof(MlColor) * s->width * s->height;

	return size;
}

void
ml_surface_clear (MlSurface *s)
{
	if_unlikely (s == NULL)
		return;

	memset (s->pixels, 0, s->width * s->height * sizeof (MlColor));
}

bool
ml_surface_resize (MlSurface *s, int width, int height)
{
	if_unlikely (s == NULL || width < 1 || height < 1)
		return false;

	if_unlikely (s->pixels == NULL) { // first call
		s->pixels = ml_new_n (MlColor, width*height);
	} else {
		s->pixels = ml_renew (MlColor, s->pixels, width*height);

		// when enlarging surface, set to zero the newly allocated pixels
		int sizediff = sizeof (MlColor) * (width*height - s->width*s->height);
		if (sizediff > 0)
			memset (s->pixels + s->width*s->height, 0, sizediff);
	}

	s->width = width;
	s->height = height;

	return true;
}

void
ml_surface_desaturate (MlSurface *s)
{
	if_unlikely (s == NULL || s->pixels == NULL)
		return;

	for (int i = 0; i < s->width*s->height; i++) {
		uint8_t lum = ml_color_get_luminance (&s->pixels[i]);
		s->pixels[i].red   = lum;
		s->pixels[i].green = lum;
		s->pixels[i].blue  = lum;
	}
}

MlColor*
ml_surface_get_pixel (MlSurface *s, int x, int y)
{
	if_unlikely (s == NULL || x < 0 || x > s->width || y < 0 || y > s->height)
		return NULL;

	return SURFACE_GET_PIXEL(s, x, y);
}

void
ml_surface_draw_pixel (MlSurface *s, int x, int y, MlColor *color)
{
	if_unlikely (s == NULL || color == NULL)
		return;

	MlColor *base_pixel = ml_surface_get_pixel(s, x, y);
	if_likely (base_pixel != NULL)
		ml_color_overlay (base_pixel, color);
}

void
ml_surface_draw_border (MlSurface *s, MlColor *border_color, int border_width)
{
	if_unlikely (s == NULL || border_color == NULL)
		return;

	for (int y = 0; y < s->height; y++) {
		for (int x = 0; x < s->width; x++) {
			if (y < border_width || y >= (s->height - border_width) || x < border_width || x >= (s->width - border_width)) {
				MlColor *base_pixel = SURFACE_GET_PIXEL(s, x, y);
				ml_color_overlay (base_pixel, border_color);
			}
		}
	}
}

void
ml_surface_draw_stripes (MlSurface *s, MlColor *color)
{
	if_unlikely (s == NULL || color == NULL)
		return;

	// copy color but alter alpha
	MlColor c[2];
	memcpy(&c[0], color, sizeof(MlColor));
	c[0].alpha = 60;
	memcpy(&c[1], color, sizeof(MlColor));
	c[1].alpha = 200;

	int i,j;
	for (i=0; i<s->height; i++) {
		for (j=0; j<s->width; j++) {
			MlColor *base_pixel = SURFACE_GET_PIXEL(s, j, i);
			if ((i%3) != 0)
				ml_color_overlay (base_pixel, &c[0]);
			else
				ml_color_overlay (base_pixel, &c[1]);
		}
	}
}

void
ml_surface_draw_dataset (MlSurface *s, MlDataset *ds, int border_width, MlColor *colors, uint32_t max) // max=0: auto
{
	if_unlikely (s == NULL || ds == NULL || colors == NULL)
		return;

	int i;
	uint32_t ceil = 0;

	MlDatasetMode ds_mode = ml_dataset_get_mode (ds);
	if_unlikely (ds_mode == ML_INVALID)
		return;

	if (ds_mode != ML_DATASET_MODE_PROPORTIONAL) {
		ceil = (max != 0) ? max : ml_dataset_get_max_ceiling (ds);
		if (ceil == 0)
			return;
	}

	int real_width = s->width - 2 * border_width;
	int real_height = s->height - 2 * border_width;
	if (real_width <= 0 || real_height <= 0)
		return;

	for (int x = 0; x < real_width; x++) {
		int y = s->height - border_width;

		if (ds_mode == ML_DATASET_MODE_PROPORTIONAL) {
			ceil = (max != 0) ? max : ml_dataset_get_ceiling (ds, x);
			if (ceil == 0)
				continue;
		}

		uint32_t *ds_entry = ml_dataset_get_entry (ds, x);

		for (i=0; i<ml_dataset_get_n_cols(ds); i++) {
			uint32_t val = ds_entry[i];
			if (val == 0)
				continue;

			/* Standard C integer division truncates, last pixels are often
			 * not drawn. We use there the rounding division (code from Linux
			 * kernel, which had the same problem with integer division) */
			int n = (int)DIV_ROUND_UINT ((uint64_t)real_height * val, ceil); // casting avoids possible overflows

			if (ds_mode == ML_DATASET_MODE_INDEPENDENT) {
				if (y-n < border_width)
					continue;

				// draw opaque value line
				MlColor *pixel = SURFACE_GET_PIXEL(s, x+border_width, y-n);
				ml_color_overlay (pixel, &colors[i]);

				// draw semitransparent value body
				uint8_t old_alpha = colors[i].alpha;
				colors[i].alpha /= 4;

				for (int y1 = y; n > 1; n--) {
					y1--;
					if (y1 < border_width)
						break;

					MlColor *pixel = SURFACE_GET_PIXEL(s, x+border_width, y1);
					ml_color_overlay (pixel, &colors[i]);
				}
				colors[i].alpha = old_alpha;

			} else {
				while (n--) {
					y--;
					if (y < border_width)
						break;

					MlColor *pixel = SURFACE_GET_PIXEL(s, x+border_width, y);
					ml_color_overlay (pixel, &colors[i]);
				}
			}
		}
	}
}

void
ml_surface_fill_background (MlSurface *s, MlGradient *grad)
{
	if_unlikely (s == NULL || grad == NULL)
		return;

	for (int y = 0; y < s->height; y++) {
		for (int x = 0; x < s->width; x++) {
			MlColor *pixel = SURFACE_GET_PIXEL(s, x, y);
			switch (grad->direction) {
				case ML_DIRECTION_NW:
					ml_gradient_step_s (grad, s->width+s->height-y-x,	s->width+s->height,		pixel);
					break;
				case ML_DIRECTION_N:
					ml_gradient_step_s (grad, s->height-y-1,			s->height,				pixel);
					break;
				case ML_DIRECTION_NE:
					ml_gradient_step_s (grad, s->height-y+x,			s->width+s->height,		pixel);
					break;
				case ML_DIRECTION_E:
					ml_gradient_step_s (grad, x,						s->width,				pixel);
					break;
				case ML_DIRECTION_SE:
					ml_gradient_step_s (grad, y+x,						s->width+s->height,		pixel);
					break;
				case ML_DIRECTION_S:
					ml_gradient_step_s (grad, y,						s->height,				pixel);
					break;
				case ML_DIRECTION_SW:
					ml_gradient_step_s (grad, s->width-x+y,				s->width+s->height,		pixel);
					break;
				case ML_DIRECTION_W:
					ml_gradient_step_s (grad, s->width-x-1,				s->width,				pixel);
					break;
			}
		}
	}
}

void
ml_surface_draw_surface (MlSurface *s, int x, int y, MlSurface *source)
{
	if_unlikely (s == NULL || source == NULL)
		return;

	if (x < 0 || x >= s->width) {
		ml_error ("Cannot draw surface: X coordinate (%"PRId32") out of base surface width (%"PRId32")", x, s->width);
		return;
	}

	if (y < 0 || y >= s->height) {
		ml_error ("Cannot draw surface: Y coordinate (%"PRId32") out of base surface height (%"PRId32")", y, s->height);
		return;
	}

	for (int i = 0; i+x < s->width && i < source->width; i++) {
		for (int j = 0; j+y < s->height && j < source->height; j++) {
			MlColor *source_pixel = SURFACE_GET_PIXEL(source, i, j);
			MlColor *base_pixel = SURFACE_GET_PIXEL(s, i+x, j+y);
			ml_color_overlay (base_pixel, source_pixel);
		}
	}
}

void
ml_surface_copy_surface (MlSurface *s, int x, int y, MlSurface *source)
{
	if_unlikely (s == NULL || source == NULL)
		return;

	if (x < 0 || x >= s->width) {
		ml_error ("Cannot copy surface: X coordinate (%"PRId32") out of base surface width (%"PRId32")", x, s->width);
		return;
	}

	if (y < 0 || y >= s->height) {
		ml_error ("Cannot copy surface: Y coordinate (%"PRId32") out of base surface height (%"PRId32")", y, s->height);
		return;
	}

	// copy data row by row
	int w = MIN (source->width, s->width-x);
	for (int row = 0; row+y < s->height && row < source->height; row++) {
		MlColor *source_pixel = SURFACE_GET_PIXEL(source, 0, row);
		MlColor *base_pixel = SURFACE_GET_PIXEL(s, x, y+row);
		memcpy (base_pixel, source_pixel, sizeof(MlColor) * w);
	}
}

uint8_t *
ml_surface_get_data (MlSurface *s, int *width, int *height)
{
	if_unlikely (s == NULL || width == NULL || height == NULL)
		return NULL;

	*width = s->width;
	*height = s->height;

	return s->pixdata;
}

static inline void
_ml_surface_to_rgba (MlSurface *s)
{
	if_unlikely (s == NULL)
		return;

	for (int i = 0; i < s->width * s->height; i++) {
		#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
			// ARGB -> RGBA
			uint8_t t = s->pixels[i].alpha;
			s->pixels[i].alpha = s->pixels[i].red;
			s->pixels[i].red = s->pixels[i].green;
			s->pixels[i].green = s->pixels[i].blue;
			s->pixels[i].blue = t;

		#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			// BGRA -> RGBA
			uint8_t t = s->pixels[i].blue;
			s->pixels[i].blue = s->pixels[i].red;
			s->pixels[i].red = t;

		#elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
			// GBAR -> RGBA
			uint8_t t = s->pixels[i].green;
			s->pixels[i].green = s->pixels[i].red;
			s->pixels[i].red = s->pixels[i].alpha;
			s->pixels[i].alpha = s->pixels[i].blue;
			s->pixels[i].blue = t;

		#else
		#error "Unsupported host byte order."
		#endif
	}
}

uint8_t *
ml_surface_write_to_png_buffer (MlSurface *s, size_t *len_ptr, unsigned compression_level)
{
	// compression_level: 0 to 9: MZ_NO_COMPRESSION=0, MZ_BEST_SPEED=1, MZ_BEST_COMPRESSION=9, MZ_DEFAULT_LEVEL=6
	if_unlikely (s == NULL)
		return NULL;

	// miniz requires a RGBA buffer, so convert byte order
	_ml_surface_to_rgba (s);

	return tdefl_write_image_to_png_file_in_memory_ex (s->pixdata, s->width, s->height, 4, len_ptr, compression_level, MZ_FALSE);
}

bool
ml_surface_write_to_png_file (MlSurface *s, const char* filename, unsigned compression_level)
{
	if_unlikely (s == NULL || filename == NULL)
		return NULL;

	size_t len = 0;
	uint8_t *png_data = ml_surface_write_to_png_buffer (s, &len, compression_level);

	if (png_data == NULL)
		return false;

	FILE *f = fopen (filename, "wb");
	if (f == NULL) {
		ml_warning ("Cannot open '%s' for writing: %s", filename, strerror (errno));
		mz_free (png_data);
		return false;
	}

	size_t wr = fwrite (png_data, 1, len, f);
	fclose (f);
	mz_free (png_data);

	if (wr != len)
		return false;

	return true;
}



/* Old implementations using libpng (new one uses Miniz, embedded in source code) */
#if 0
void
ml_surface_to_big_endian (MlSurface *s)
{
	#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__

		if_unlikely (s == NULL)
			return;

		for (int i = 0; i < s->width * s->height; i ++) {
			uint32_t *n = &((uint32_t*)s->pixdata)[i];
			*n = htonl (*n); // htonl is often optimized for host architecture, so it's better than direct byte swap
		}

	#endif
}

uint8_t *
ml_surface_get_data (MlSurface *s, int *width, int *height, bool big_endian)
{
	if_unlikely (s == NULL || width == NULL || height == NULL)
		return NULL;

	if (big_endian)
		ml_surface_to_big_endian (s);

	*width = s->width;
	*height = s->height;

	return s->pixdata;
}

bool
ml_surface_save_to_png_file (MlSurface *s, const char* filename)
{
	FILE *f = fopen (filename, "wb");
	if_unlikely (f == NULL)
		return false;

	png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if_unlikely (png_ptr == NULL) {
		fclose (f);
		return false;
	}

	png_infop info_ptr = png_create_info_struct (png_ptr);
	if_unlikely (info_ptr == NULL) {
		fclose (f);
		return false;
	}

	if (setjmp (png_jmpbuf(png_ptr))) {
		png_destroy_write_struct (&png_ptr, &info_ptr);
		fclose (f);
		return false;
	}

	png_init_io (png_ptr, f);

	png_set_IHDR (png_ptr, info_ptr, s->width, s->height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info (png_ptr, info_ptr);

	// ARGB to RGBA
	png_set_swap_alpha(png_ptr);

	// swap bytes (PNG accepts big endian only)
	ml_surface_to_big_endian (s);

	for (int i=0; i<s->height; i++)
		png_write_row (png_ptr, (png_bytep)&(s->pixdata [i*s->width*4]));

	png_write_end (png_ptr, NULL);


	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(f);

	return true;
}



static inline void
_ml_surface_png_write_cb (png_structp png_ptr, png_bytep data, png_size_t length)
{
	ml_grow_buffer_append ((MlGrowBuffer*)png_get_io_ptr (png_ptr), data, length);
}

uint8_t *
ml_surface_save_to_png_buffer (MlSurface *s, size_t *len_ptr)
{
	if_unlikely (len_ptr == NULL)
		return NULL;

	MlGrowBuffer *gbuf = ml_grow_buffer_new (s->width * s->height * sizeof(MlColor));

	png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if_unlikely (png_ptr == NULL)
		return NULL;

	png_set_write_fn (png_ptr, gbuf, _ml_surface_png_write_cb, NULL);

	png_infop info_ptr = png_create_info_struct (png_ptr);
	if_unlikely (info_ptr == NULL)
		return NULL;

	if (setjmp (png_jmpbuf(png_ptr))) {
		png_destroy_write_struct (&png_ptr, &info_ptr);
		return NULL;
	}

	png_set_IHDR (png_ptr, info_ptr, s->width, s->height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info (png_ptr, info_ptr);

	// ARGB to RGBA
	png_set_swap_alpha(png_ptr);

	// swap bytes (PNG accepts big endian only)
	ml_surface_to_big_endian (s);

	for (int i=0; i<s->height; i++)
		png_write_row (png_ptr, (png_bytep)&(s->pixdata [i*s->width*4]));

	png_write_end (png_ptr, NULL);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	*len_ptr = ml_grow_buffer_get_length (gbuf);
	return ml_grow_buffer_destroy (gbuf, false);
}
#endif
