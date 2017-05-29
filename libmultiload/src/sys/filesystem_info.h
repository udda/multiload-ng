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

#ifndef ML_HEADER__SYS_FILESYSTEM_INFO_H__INCLUDED
#define ML_HEADER__SYS_FILESYSTEM_INFO_H__INCLUDED
ML_HEADER_BEGIN


typedef struct {
	char *mountpoint;

	char fs_type[24];       // this should contain every possible filesystem type
	char device[PATH_MAX];  // actually, /proc/mounts lines may be longer than this, but device paths are capped to this length
	bool is_block_device;

	uint64_t bytes_free_user;
	uint64_t bytes_free_root;
	uint64_t bytes_total;

	uint32_t inodes_free_user;
	uint32_t inodes_free_root;
	uint32_t inodes_total;
} MlFilesystemInfo;


MlFilesystemInfo *
ml_filesystem_info_new (const char *mountpoint);

void
ml_filesystem_info_destroy (MlFilesystemInfo *fi);

size_t
ml_filesystem_info_sizeof (MlFilesystemInfo *fi)
ML_FN_SIZEOF;

bool
ml_filesystem_info_update (MlFilesystemInfo *fi, struct mntent *mnt)
ML_FN_WARN_UNUSED_RESULT;

MlGrowArray *
ml_filesystem_info_list_mounted (MlAssocArray *aa);

char **
ml_filesystem_info_list_dev_fstypes()
ML_FN_COLD;

MlGrowBuffer *
ml_filesystem_info_generate_report ()
ML_FN_COLD;


ML_HEADER_END
#endif /* ML_HEADER__SYS_FILESYSTEM_INFO_H__INCLUDED */
