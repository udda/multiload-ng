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

#ifndef ML_HEADER__PROVIDER_PARAMETRIC_H__INCLUDED
#define ML_HEADER__PROVIDER_PARAMETRIC_H__INCLUDED
ML_HEADER_BEGIN


extern const MlGraphTypeInterface ML_PROVIDER_PARAMETRIC_IFACE;


mlPointer
ml_provider_parametric_init (MlConfig *config);

void
ml_provider_parametric_get (MlGraphContext *context)
ML_FN_HOT;

void
ml_provider_parametric_destroy (mlPointer provider_data);

size_t
ml_provider_parametric_sizeof (mlPointer provider_data)
ML_FN_SIZEOF;

void
ml_provider_parametric_caption (MlCaption *caption, MlDataset *ds, mlPointer provider_data);


ML_HEADER_END
#endif /* ML_HEADER__PROVIDER_PARAMETRIC_H__INCLUDED */
