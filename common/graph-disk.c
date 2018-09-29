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
#include "info-file.h"
#include "preferences.h"
#include "util.h"



MultiloadFilter *
multiload_graph_disk_get_filter (LoadGraph *g, DiskData *xd)
{
	char *buf = NULL;
	size_t n = 0;

	guint64 blocks;
	char device[20], prefix[20], label[30];
	guint i;

	MultiloadFilter *filter = multiload_filter_new();

	FILE *f = info_file_required_fopen("/proc/partitions", "r");

	while(getline(&buf, &n, f) >= 0) {
		if (2 != fscanf(f, "%*u %*u %"G_GUINT64_FORMAT" %s", &blocks, device))
			continue;

		// extract block device and partition names
		gboolean is_partition = FALSE;
		g_strlcpy(prefix, device, sizeof(prefix));
		for (i=0; prefix[i] != '\0'; i++) {
			if (isdigit(prefix[i])) {
				prefix[i] = '\0';
				is_partition = TRUE;
				break;
			}
		}

		// generate sysfs path
		char sysfs_path[PATH_MAX];
		if (is_partition)
			g_snprintf(sysfs_path, PATH_MAX, "/sys/block/%s/%s/stat", prefix, device);
		else
			g_snprintf(sysfs_path, PATH_MAX, "/sys/block/%s/stat", device);

		if (access(sysfs_path, R_OK) != 0)
			continue;

		gchar *size = format_size_for_display(blocks*1024, g->multiload->size_format_iec);
		g_snprintf(label, sizeof(label), "%s (%s)", device, size);
		g_free(size);

		multiload_filter_append_with_label(filter, device, label);
	}

	g_free(buf);
	fclose(f);

	multiload_filter_import_existing(filter, g->config->filter);

	return filter;
}


void
multiload_graph_disk_get_data (int Maximum, int data [2], LoadGraph *g, DiskData *xd, gboolean first_call)
{
	FILE *f_stat;

	char *buf = NULL;
	size_t n = 0;

	guint i;
	int max;

	char sysfs_path[PATH_MAX];
	guint64 blocks;
	char device[20], prefix[20];
	guint64 read, write;
	guint64 read_total = 0, write_total = 0;
	guint64 readdiff, writediff;

	FILE *f = info_file_required_fopen("/proc/partitions", "r");

	xd->partitions[0] = '\0';

	while(getline(&buf, &n, f) >= 0) {
		if (2 != fscanf(f, "%*u %*u %"G_GUINT64_FORMAT" %s", &blocks, device))
			continue;

		// extract block device and partition names
		gboolean is_partition = FALSE;
		g_strlcpy(prefix, device, sizeof(prefix));
		for (i=0; prefix[i] != '\0'; i++) {
			if (isdigit(prefix[i])) {
				prefix[i] = '\0';
				is_partition = TRUE;
				break;
			}
		}

		// filter
		gboolean ignore=FALSE;
		if (g->config->filter_enable) {
			MultiloadFilter *filter = multiload_filter_new_from_existing(g->config->filter);
			for (i=0, ignore=TRUE; i<multiload_filter_get_length(filter); i++) {
				if (strcmp(multiload_filter_get_element_data(filter,i), device) == 0) {
					ignore = FALSE;
					break;
				}
			}

			if (ignore)
				g_debug("[graph-disk] Ignored device '%s' due to user filter", device);

			multiload_filter_free(filter);
		}
		if (ignore)
			continue;

		// generate sysfs path
		if (is_partition)
			g_snprintf(sysfs_path, PATH_MAX, "/sys/block/%s/%s/stat", prefix, device);
		else
			g_snprintf(sysfs_path, PATH_MAX, "/sys/block/%s/stat", device);

		// read data from sysfs
		f_stat = fopen(sysfs_path, "r");
		if (f_stat == NULL)
			continue;
		int result = fscanf(f_stat, "%*u %*u %"G_GUINT64_FORMAT" %*u %*u %*u %"G_GUINT64_FORMAT" %*u", &read, &write);
		fclose(f_stat);
		if (result != 2)
			continue;

		// data gathered - add to totals
		read_total += read;
		write_total += write;

		g_strlcat (xd->partitions, device, sizeof(xd->partitions));
		g_strlcat (xd->partitions, ", ", sizeof(xd->partitions));
	}

	g_free(buf);
	fclose(f);

	xd->partitions[strlen(xd->partitions)-2] = 0;

	readdiff  = read_total  - xd->last_read;
	writediff = write_total - xd->last_write;

	xd->last_read  = read_total;
	xd->last_write = write_total;

	if (G_LIKELY(!first_call)) { // cannot calculate diff on first call
		max = autoscaler_get_max(&xd->scaler, g, readdiff + writediff);

		if (max == 0) {
			memset(data, 0, 4*sizeof(data[0]));
		} else {
			data[0] = (float)Maximum *  readdiff / (float)max;
			data[1] = (float)Maximum * writediff / (float)max;
		}

		// read/write are relative to standard linux sectors (512 bytes, fixed)
		xd->read_speed  = calculate_speed(readdiff  * 512, g->config->interval);
		xd->write_speed = calculate_speed(writediff * 512, g->config->interval);
	}
}


void
multiload_graph_disk_cmdline_output (LoadGraph *g, DiskData *xd)
{
	g_snprintf(g->output_str[0], sizeof(g->output_str[0]), "%"G_GUINT64_FORMAT, xd->read_speed);
	g_snprintf(g->output_str[1], sizeof(g->output_str[1]), "%"G_GUINT64_FORMAT, xd->write_speed);
}


void
multiload_graph_disk_tooltip_update (char *buf_title, size_t len_title, char *buf_text, size_t len_text, LoadGraph *g, DiskData *xd, gint style)
{
	gchar *disk_read = format_rate_for_display(xd->read_speed, g->multiload->size_format_iec);
	gchar *disk_write = format_rate_for_display(xd->write_speed, g->multiload->size_format_iec);

	if (style == MULTILOAD_TOOLTIP_STYLE_DETAILED) {
		g_snprintf(buf_text, len_text, _(	"Monitored partitions: %s\n"
											"\n"
											"Read: %s\n"
											"Write: %s"),
											xd->partitions, disk_read, disk_write);
	} else {
		g_snprintf(buf_text, len_text, "\xe2\xac\x86%s \xe2\xac\x87%s", disk_read, disk_write);
	}
	g_free(disk_read);
	g_free(disk_write);
}
