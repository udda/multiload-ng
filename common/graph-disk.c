/*
 * Copyright (C) 2016 Mario Cianciolo <mr.udda@gmail.com>
 *
 * This file is part of multiload-ng.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include <config.h>

#include <math.h>
#include <ctype.h>
#include <mntent.h>
#include <stdlib.h>

#include "graph-data.h"
#include "autoscaler.h"
#include "preferences.h"
#include "util.h"


void
multiload_graph_disk_get_data (int Maximum, int data [2], LoadGraph *g, DiskData *xd)
{
	static gboolean first_call = TRUE;
	const static char *fstype_ignore_list[] = { "rootfs", "smbfs", "nfs", "cifs", "fuse.", NULL };

	FILE *f_mntent;
	FILE *f_stat;
	struct mntent *mnt;

	guint i;
	int max;

	char *sysfs_path, *device, *prefix;
	guint64 read, write;
	guint64 read_total = 0, write_total = 0;
	guint64 readdiff, writediff;


	if ((f_mntent = setmntent(MOUNTED, "r")) == NULL)
		return;

	xd->partitions[0] = '\0';

	// loop through mountpoints
	while ((mnt = getmntent(f_mntent)) != NULL) {

		// skip filesystens that do not have a block device
		if (strncmp (mnt->mnt_fsname, "/dev/", 5) != 0)
			continue;

		// skip filesystems of certain types, defined in fstype_ignore_list[]
		gboolean ignore = FALSE;
		for (i=0; fstype_ignore_list[i] != NULL; i++) {
			if (strncmp (mnt->mnt_type, fstype_ignore_list[i], strlen(fstype_ignore_list[i])) == 0) {
				ignore = TRUE;
				break;
			}

		}
		if (ignore)
			continue;

		// extract block device and partition names
		gboolean is_partition = FALSE;
		device = &mnt->mnt_fsname[5];
		prefix = g_strdup(device);
		for (i=0; prefix[i] != '\0'; i++) {
			if (isdigit(prefix[i])) {
				prefix[i] = '\0';
				is_partition = TRUE;
				break;
			}
		}

		// generate sysfs path
		if (is_partition)
			sysfs_path = g_strdup_printf("/sys/block/%s/%s/stat", prefix, device);
		else
			sysfs_path = g_strdup_printf("/sys/block/%s/stat", device);
		g_free(prefix);

		// read data from sysfs
		f_stat = fopen(sysfs_path, "r");
		g_free(sysfs_path);
		if (f_stat == NULL)
			continue;
		int result = fscanf(f_stat, "%*u %*u %lu %*u %*u %*u %lu %*u", &read, &write);
		fclose(f_stat);
		if (result != 2)
			continue;

		// data gathered - add to totals
		read_total += read;
		write_total += write;

		g_strlcat (xd->partitions, device, sizeof(xd->partitions));
		g_strlcat (xd->partitions, ", ", sizeof(xd->partitions));
	}
	endmntent(f_mntent);
	xd->partitions[strlen(xd->partitions)-2] = 0;

	readdiff  = read_total  - xd->last_read;
	writediff = write_total - xd->last_write;

	xd->last_read  = read_total;
	xd->last_write = write_total;

	if (first_call) { // cannot calculate diff on first call
		first_call = FALSE;
	} else {
		max = autoscaler_get_max(&xd->scaler, g, readdiff + writediff);

		data[0] = (float)Maximum *  readdiff / (float)max;
		data[1] = (float)Maximum * writediff / (float)max;

		// read/write are relative to standard linux sectors (512 bytes, fixed)
		xd->read_speed  = calculate_speed(readdiff  * 512, g->config->interval);
		xd->write_speed = calculate_speed(writediff * 512, g->config->interval);
	}
}


void
multiload_graph_disk_cmdline_output (LoadGraph *g, DiskData *xd)
{
	if (g->output_unit[0] == '\0')
		g_strlcpy(g->output_unit, "Bps", sizeof(g->output_unit));
	g_snprintf(g->output_str[0], sizeof(g->output_str[0]), "%ld", xd->read_speed);
	g_snprintf(g->output_str[1], sizeof(g->output_str[1]), "%ld", xd->write_speed);
}


void
multiload_graph_disk_tooltip_update (char **title, char **text, LoadGraph *g, DiskData *xd)
{
	gchar *disk_read = format_rate_for_display(xd->read_speed);
	gchar *disk_write = format_rate_for_display(xd->write_speed);

	if (g->config->tooltip_style == TOOLTIP_STYLE_DETAILS) {
		*text = g_strdup_printf(_(	"Monitored partitions: %s\n"
									"\n"
									"Read: %s\n"
									"Write: %s"),
									xd->partitions, disk_read, disk_write);
	} else {
		*text = g_strdup_printf("\xe2\xac\x86%s \xe2\xac\x87%s", disk_read, disk_write);
	}
	g_free(disk_read);
	g_free(disk_write);
}
