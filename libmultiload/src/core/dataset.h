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

#ifndef ML_HEADER__CORE_DATASET_H__INCLUDED
#define ML_HEADER__CORE_DATASET_H__INCLUDED
ML_HEADER_BEGIN


typedef struct _MlDataset MlDataset;

typedef enum {
	ML_DATASET_MODE_PROPORTIONAL,	// every entry sum up as 100% (memory, cpu, ...) - provider must supply total (100%) value
	ML_DATASET_MODE_ABSOLUTE,		// every entry is an absolute measure, stacked up like ML_DATASET_PROPORTIONAL but every entry has different total (max total is used for drawing) - if provider supplies total value, it will be used as "min ceiling"
	ML_DATASET_MODE_INDEPENDENT		// every chunk is independent from others, they are all drawn starting from zero (with opaque lines and semitransparent bodies)
} MlDatasetMode;


MlDataset *
ml_dataset_new (int n_cols, int size, MlDatasetMode mode);

void
ml_dataset_destroy (MlDataset *ds);

size_t
ml_dataset_sizeof (MlDataset *ds)
ML_FN_SIZEOF;

MlDatasetMode
ml_dataset_get_mode (MlDataset *ds)
ML_FN_READ_PROPERTY ML_FN_HOT;

int
ml_dataset_get_n_cols (MlDataset *ds)
ML_FN_READ_PROPERTY ML_FN_HOT;

uint32_t *
ml_dataset_get_entry (MlDataset *ds, int index)
ML_FN_READ_PROPERTY ML_FN_HOT ML_FN_NONNULL(1);

uint32_t
ml_dataset_get_value (MlDataset *ds, int index, int column)
ML_FN_READ_PROPERTY ML_FN_HOT ML_FN_NONNULL(1);

uint32_t
ml_dataset_get_ceiling (MlDataset *ds, int index)
ML_FN_READ_PROPERTY ML_FN_HOT ML_FN_NONNULL(1);

uint32_t
ml_dataset_get_max_ceiling (MlDataset *ds)
ML_FN_READ_PROPERTY ML_FN_HOT ML_FN_NONNULL(1);

void
ml_dataset_push_entry (MlDataset *ds, uint32_t *data, int data_length)
ML_FN_HOT;

bool
ml_dataset_resize (MlDataset *ds, int newsize)
ML_FN_WARN_UNUSED_RESULT;

void
ml_dataset_clear (MlDataset *ds);

void
ml_dataset_dump (MlDataset *ds)
ML_FN_COLD;


ML_HEADER_END
#endif /* ML_HEADER__CORE_DATASET_H__INCLUDED */
