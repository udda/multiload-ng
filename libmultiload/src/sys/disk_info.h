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

#ifndef ML_HEADER__SYS_DISK_INFO_H__INCLUDED
#define ML_HEADER__SYS_DISK_INFO_H__INCLUDED
ML_HEADER_BEGIN


typedef struct {
	char *name;
	char *path_stat;

	unsigned long read_sectors;
	unsigned long write_sectors;
	uint32_t sector_size;

	unsigned long read_ms;
	unsigned long write_ms;
} MlDiskInfo;


MlDiskInfo *
ml_disk_info_new (const char *name);

void
ml_disk_info_destroy (MlDiskInfo *di);

size_t
ml_disk_info_sizeof (MlDiskInfo *di)
ML_FN_SIZEOF;

bool
ml_disk_info_update (MlDiskInfo *di)
ML_FN_WARN_UNUSED_RESULT;

MlGrowArray *
ml_disk_info_list_mounted (MlAssocArray *aa);

MlGrowBuffer *
ml_disk_info_generate_report ()
ML_FN_COLD ML_FN_RETURNS_NONNULL;


ML_HEADER_END
#endif /* ML_HEADER__SYS_DISK_INFO_H__INCLUDED */
