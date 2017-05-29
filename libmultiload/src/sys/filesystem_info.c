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


#define PATH_FILESYSTEMS "/proc/filesystems"


MlFilesystemInfo *
ml_filesystem_info_new (const char *mountpoint)
{
	if_unlikely (mountpoint == NULL)
		return NULL;

	MlFilesystemInfo *fi = ml_new (MlFilesystemInfo);
	fi->mountpoint = ml_strdup (mountpoint);

	return fi;
}

void
ml_filesystem_info_destroy (MlFilesystemInfo *fi)
{
	if_unlikely (fi == NULL)
		return;

	free (fi->mountpoint);

	free (fi);
}

size_t
ml_filesystem_info_sizeof (MlFilesystemInfo *fi)
{
	if_unlikely (fi == NULL)
		return 0;

	size_t size = sizeof (MlFilesystemInfo);

	size += ml_string_sizeof (fi->mountpoint);

	return size;
}


bool
ml_filesystem_info_update (MlFilesystemInfo *fi, struct mntent *mnt)
{
	if_unlikely (fi == NULL || fi->mountpoint == NULL)
		return false;

	if (mnt != NULL) {
		strncpy (fi->fs_type, mnt->mnt_type, sizeof (fi->fs_type));

		if (realpath (mnt->mnt_fsname, fi->device) == NULL)
			strncpy (fi->device, mnt->mnt_fsname, sizeof (fi->device));

		fi->is_block_device = ml_strv_contains (ML_SHARED_GET(dev_fstypes), fi->fs_type);
	}

	struct statvfs vfsd;
	if (statvfs (fi->mountpoint, &vfsd) < 0)
		return false;

	uint64_t blocksize = vfsd.f_frsize ? vfsd.f_frsize : vfsd.f_bsize;
	if (blocksize == 0)
		return false;

	fi->bytes_free_user  = blocksize * vfsd.f_bavail;
	fi->bytes_free_root  = blocksize * vfsd.f_bfree;
	fi->bytes_total      = blocksize * vfsd.f_blocks;

	fi->inodes_free_user = vfsd.f_favail;
	fi->inodes_free_root = vfsd.f_ffree;
	fi->inodes_total     = vfsd.f_files;

	return true;
}


MlGrowArray*
ml_filesystem_info_list_mounted (MlAssocArray *aa)
{
	// aa must be a MlAssocArray that caches MlFilesystemInfo pointers (keys are the device names)
	if_unlikely (aa == NULL)
		return NULL;

	FILE *f = setmntent (MOUNTED, "r");
	if_unlikely (f == NULL)
		return NULL;

	MlGrowArray *mounted = ml_grow_array_new (8, NULL);

	char buf[400];
	struct mntent *mnt;
	struct mntent mntbuf;

	// loop through mountpoints
	while ((mnt = getmntent_r (f, &mntbuf, buf, sizeof (buf))) != NULL) {
		// lookup existing data and create it if necessary
		MlFilesystemInfo *fi = (MlFilesystemInfo*)ml_assoc_array_get (aa, mnt->mnt_dir);
		if (fi == NULL) {
			fi = ml_filesystem_info_new (mnt->mnt_dir);
			if (fi == NULL)
				continue;

			ml_assoc_array_put (aa, mnt->mnt_dir, fi);
		}

		// do not count the same filesystem twice
		if (ml_grow_array_contains (mounted, fi))
			continue;

		if (!ml_filesystem_info_update (fi, mnt))
			continue;

		// skip zero-blocks filesystems
		if (fi->bytes_total == 0)
			continue;


		/* if two mountpoints refer to the same device (multiple mounts, bind mounts), choose
		 * the one with shortest path (less subdirectories). This is the way it's done by df */
		if (fi->is_block_device) {
			bool skip = false;

			ml_grow_array_for (mounted, i) {
				MlFilesystemInfo *_fi = (MlFilesystemInfo*)ml_grow_array_get (mounted, i);

				if (_fi->is_block_device && !strcmp (fi->device, _fi->device)) {
					ml_debug ("Detected multiple mount! \"%s\" -> [\"%s\", \"%s\"]", fi->device, fi->mountpoint, _fi->mountpoint);
					//ml_grow_array_replace (mounted, _fi, i);
				}
			}

			if (skip)
				continue;
		}


		ml_grow_array_append (mounted, fi);
	}
	endmntent (f);

	return mounted;
}

char **
ml_filesystem_info_list_dev_fstypes()
{
	MlGrowArray *ga = ml_grow_array_new (10, NULL);

	FILE *f = fopen (PATH_FILESYSTEMS, "r");
	if_unlikely (f == NULL)
		return NULL;

	char *line = NULL;
	size_t n = 0;
	while (getline (&line, &n, f) >= 0) {
		if (!ml_string_has_prefix (line, "nodev"))
			ml_grow_array_append (ga, strdup(ml_string_trim (line)));
	}

	fclose (f);
	free (line);

	return (char **)ml_grow_array_destroy (ga, false, true);
}

MlGrowBuffer *
ml_filesystem_info_generate_report ()
{
	MlGrowBuffer *gbuf = ml_grow_buffer_new (10240);

	FILE *f = setmntent (MOUNTED, "r");
	if_unlikely (f == NULL) {
		ml_grow_buffer_append_printf (gbuf, "Error: cannot read mounted devices from %s\n", MOUNTED);
		return gbuf;
	}
	ml_grow_buffer_append_printf (gbuf, "Reading mounted devices from %s\n", MOUNTED);


	char buf[400];
	struct mntent *mnt;
	struct mntent mntbuf;

	int total = 0, valid = 0;
	while ((mnt = getmntent_r(f, &mntbuf, buf, sizeof (buf))) != NULL) {
		ml_grow_buffer_append_printf (gbuf, "\n\nMOUNT ENTRY #%d\n---------------\n", total++);
		ml_grow_buffer_append_printf (gbuf, "Mountpoint:         %s\n", mnt->mnt_dir);

		MlFilesystemInfo *fi = ml_filesystem_info_new (mnt->mnt_dir);
		if_unlikely (fi == NULL) {
			ml_grow_buffer_append_printf (gbuf, "Error: cannot create MlFilesystemInfo. This is a bug.\n");
			continue;
		}

		if (!ml_filesystem_info_update (fi, mnt)) {
			ml_grow_buffer_append_printf (gbuf, "Error: cannot update filesystem stats for %s\n", mnt->mnt_dir);
			continue;
		}

		ml_grow_buffer_append_printf (gbuf, "Filesystem type:    %s\n", fi->fs_type);
		ml_grow_buffer_append_printf (gbuf, "Underlying device:  %s\n", fi->device);
		ml_grow_buffer_append_printf (gbuf, "Is a block device:  %s\n", fi->is_block_device ? "yes" : "no");


		if (fi->bytes_total > 0) {
			char buf_user[24];
			char buf_root[24];
			char buf_total[24];
			ml_string_format_size_s (fi->bytes_free_user, "B", false, buf_user,  sizeof (buf_user));
			ml_string_format_size_s (fi->bytes_free_root, "B", false, buf_root,  sizeof (buf_root));
			ml_string_format_size_s (fi->bytes_total,     "B", false, buf_total, sizeof (buf_total));
			ml_grow_buffer_append_printf (gbuf, "Free storage:       %s (user), %s (root)\n", buf_user, buf_root);
			ml_grow_buffer_append_printf (gbuf, "Total storage:      %s\n", buf_total);
		} else {
			ml_grow_buffer_append_printf (gbuf, "Total storage:      This filesystem has no storage space.\n");
		}

		if (fi->inodes_total > 0) {
			ml_grow_buffer_append_printf (gbuf, "Free inodes:        %"PRIu32" (user), %"PRIu32" (root)\n", fi->inodes_free_user, fi->inodes_free_root);
			ml_grow_buffer_append_printf (gbuf, "Total inodes:       %"PRIu32"\n", fi->inodes_total);
		} else {
			ml_grow_buffer_append_printf (gbuf, "Total inodes:       This filesystem has no inodes.\n");
		}

		if (fi->bytes_total > 0 && fi->inodes_total > 0)
			valid++;

		ml_filesystem_info_destroy (fi);
	}
	endmntent (f);

	ml_grow_buffer_append_printf (gbuf, "\n\n");
	ml_grow_buffer_append_printf (gbuf, "Found %d usable mountpoints, out of %d total mountpoints.\n", valid, total);

	return gbuf;
}
