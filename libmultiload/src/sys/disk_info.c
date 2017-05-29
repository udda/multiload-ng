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


MlDiskInfo *
ml_disk_info_new (const char *name)
{
	MlDiskInfo *di = ml_new (MlDiskInfo);

	char path_sector_size[PATH_MAX];

	di->name = ml_strdup (name);

	// LVM devices (dm-*), optical drives (sr*)
	if (ml_string_has_prefix (name, "dm-") || ml_string_has_prefix (name, "sr")	) {
		di->path_stat = ml_strdup_printf ("/sys/block/%s/stat", name);
		snprintf (path_sector_size, PATH_MAX, "/sys/block/%s/queue/hw_sector_size", name);
	}

	// classic block devices (sd*, hd*)
	else if (ml_string_has_prefix (name, "sd") || ml_string_has_prefix (name, "hd")) {
		char device_name [sizeof(di->name)];
		if (sscanf (name, "%[a-zA-Z]%*d", device_name) == 1) { // is a volume
			di->path_stat = ml_strdup_printf ("/sys/block/%s/%s/stat", device_name, name);
			snprintf (path_sector_size, PATH_MAX, "/sys/block/%s/queue/hw_sector_size", device_name);
		} else { // is a whole disk
			di->path_stat = ml_strdup_printf ("/sys/block/%s/stat", name);
			snprintf (path_sector_size, PATH_MAX, "/sys/block/%s/queue/hw_sector_size", name);
		}

	} else {
		ml_warning ("Unsupported block device '%s'. Please report this.", name);
		ml_disk_info_destroy (di);
		return NULL;
	}

	if (!ml_infofile_read_uint32 (path_sector_size, &di->sector_size))
		di->sector_size = 512; // fallback, true for most block devices

	if (!ml_file_is_readable (di->path_stat)) {
		ml_error ("sysfs path for block device '%s' is not existing", name);
		ml_disk_info_destroy (di);
		return NULL;
	}

	return di;
}

void
ml_disk_info_destroy (MlDiskInfo *di)
{
	if_unlikely (di == NULL)
		return;

	free (di->name);
	free (di->path_stat);

	free (di);
}

size_t
ml_disk_info_sizeof (MlDiskInfo *di)
{
	if_unlikely (di == NULL)
		return 0;

	size_t size = sizeof (MlDiskInfo);

	size += ml_string_sizeof (di->name);
	size += ml_string_sizeof (di->path_stat);

	return size;
}

bool
ml_disk_info_update (MlDiskInfo *di)
{
	if_unlikely (di == NULL)
		return false;

	FILE *f = fopen (di->path_stat, "r");
	if_unlikely (f == NULL)
		return false;

	int n = fscanf(f, "%*u %*u %lu %lu %*u %*u %lu %lu", &di->read_sectors, &di->read_ms, &di->write_sectors, &di->write_ms);
	fclose(f);
	if_unlikely (n != 4)
		return false;

	return true;
}

MlGrowArray*
ml_disk_info_list_mounted (MlAssocArray *aa)
{
	// aa must be a MlAssocArray that caches MlDiskInfo pointers (keys are the device names)
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
	while ((mnt = getmntent_r(f, &mntbuf, buf, sizeof (buf))) != NULL) {
		// skip filesystems that do not have an absolute device path
		if (mnt->mnt_fsname[0] != '/')
			continue;

		char real_path[PATH_MAX];
		if (realpath (mnt->mnt_fsname, real_path) == NULL)
			continue;

		// skip filesystems that do not have a block device
		if (!ml_string_has_prefix (real_path, "/dev/"))
			continue;

		// lookup existing data and create it if necessary
		char *dev_name = real_path + 5;
		MlDiskInfo *di = (MlDiskInfo*)ml_assoc_array_get (aa, dev_name);
		if (di == NULL) {
			di = ml_disk_info_new (dev_name);
			if_unlikely (di == NULL)
				continue;

			ml_assoc_array_put (aa, dev_name, di);
		}

		// do not add the same device twice
		if (ml_grow_array_contains (mounted, di))
			continue;

		if (!ml_disk_info_update (di))
			continue;

		ml_grow_array_append (mounted, di);

	}
	endmntent (f);

	return mounted;
}

MlGrowBuffer *
ml_disk_info_generate_report ()
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
		ml_grow_buffer_append_printf (gbuf, "Device path:                 %s\n", mnt->mnt_fsname);

		if (mnt->mnt_fsname[0] != '/') {
			ml_grow_buffer_append_printf (gbuf, "(device skipped because it doesn't have an absolute path)\n");
			continue;
		}

		char real_path[PATH_MAX];
		if (realpath (mnt->mnt_fsname, real_path) == NULL) {
			ml_grow_buffer_append_printf (gbuf, "(device skipped because its path cannot be canonicalized)\n");
			continue;
		}

		ml_grow_buffer_append_printf (gbuf, "Canonicalized device path:   %s\n", real_path);

		if (!ml_string_has_prefix (real_path, "/dev/")) {
			ml_grow_buffer_append_printf (gbuf, "(device skipped because it's not a block device, as it's not located under /dev/)\n");
			continue;
		}

		char *dev_name = real_path + 5;
		MlDiskInfo *di = ml_disk_info_new (dev_name);
		if_unlikely (di == NULL) {
			ml_grow_buffer_append_printf (gbuf, "Error: cannot create MlDiskInfo. Either unsupported device type or missing disk stats file.\n");
			continue;
		}

		ml_grow_buffer_append_printf (gbuf, "Disk stats file:             %s\n", di->path_stat);
		ml_grow_buffer_append_printf (gbuf, "Sector size:                 %"PRIu32" byte\n", di->sector_size);

		if (!ml_disk_info_update (di)) {
			ml_grow_buffer_append_printf (gbuf, "Error: cannot update disk stats from %s\n", di->path_stat);
			continue;
		}

		ml_grow_buffer_append_printf (gbuf, "I/O since boot (R/W):        %lu/%lu sectors, %lu/%lu milliseconds\n", di->read_sectors, di->write_sectors, di->read_ms, di->write_ms);
		ml_grow_buffer_append_printf (gbuf, "Busy time since boot (R/W):  %lu/%lu sectors, %lu/%lu milliseconds\n", di->read_sectors, di->write_sectors, di->read_ms, di->write_ms);

		valid++;

		ml_disk_info_destroy (di);
	}
	endmntent (f);

	ml_grow_buffer_append_printf (gbuf, "\n\n");
	ml_grow_buffer_append_printf (gbuf, "Found %d valid devices, out of %d total mounted devices.\n", valid, total);
	ml_grow_buffer_append_printf (gbuf, "Note: some devices may be duplicated.\n");

	return gbuf;
}
