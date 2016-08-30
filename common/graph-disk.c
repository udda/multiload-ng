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
#include <glibtop.h>
#include <glibtop/fsusage.h>
#include <glibtop/mountlist.h>

#include "graph-data.h"
#include "autoscaler.h"
#include "preferences.h"
#include "util.h"


void
multiload_graph_disk_get_data (int Maximum, int data [2], LoadGraph *g)
{
	glibtop_mountlist mountlist;
	glibtop_fsusage fsusage;
	static gboolean first_call = TRUE;

	static const guint64 needed_flags =
		(1 << GLIBTOP_FSUSAGE_BLOCK_SIZE) +
		(1 << GLIBTOP_FSUSAGE_READ) +
		(1 << GLIBTOP_FSUSAGE_WRITE);

	DiskData *xd = (DiskData*) g->extra_data;
	g_assert_nonnull(xd);

	guint i;
	int max;

	guint64 read = 0;
	guint64 write = 0;
	guint64 readdiff, writediff;

	glibtop_mountentry *mountentries = glibtop_get_mountlist (&mountlist, FALSE);

	for (i = 0; i < mountlist.number; i++) {
		if (   strcmp (mountentries[i].type, "smbfs") == 0
			|| strcmp (mountentries[i].type, "nfs") == 0
			|| strcmp (mountentries[i].type, "cifs") == 0
			|| strncmp(mountentries[i].type, "fuse.", 5) == 0)
			continue;

		glibtop_get_fsusage(&fsusage, mountentries[i].mountdir);
		if ((fsusage.flags & needed_flags) != needed_flags)
			continue; // FS does not have required capabilities

		read  += fsusage.read;
		write += fsusage.write;
	}

	g_free(mountentries);

	readdiff  = read  - xd->last_read;
	writediff = write - xd->last_write;

	xd->last_read  = read;
	xd->last_write = write;

	if (first_call) {
		first_call = FALSE;
		return;
	}

	max = autoscaler_get_max(&xd->scaler, g, readdiff + writediff);

	data[0] = (float)Maximum *  readdiff / (float)max;
	data[1] = (float)Maximum * writediff / (float)max;

	/* read/write are relative to SECTORS (standard 512 byte) and not blocks
	 * as glibtop documentation states. So multiply value by 512 */
	xd->read_speed  = calculate_speed(readdiff  * 512, g->config->interval);
	xd->write_speed = calculate_speed(writediff * 512, g->config->interval);
}

void
multiload_graph_disk_tooltip_update (char **title, char **text, LoadGraph *g, DiskData *xd)
{
	gchar *disk_read = format_rate_for_display(xd->read_speed);
	gchar *disk_write = format_rate_for_display(xd->write_speed);

	if (g->config->tooltip_style == TOOLTIP_STYLE_DETAILS) {
		*text = g_strdup_printf(_(	"Read: %s\n"
									"Write: %s"),
									disk_read, disk_write);
	} else {
		*text = g_strdup_printf("\xe2\xac\x86%s \xe2\xac\x87%s", disk_read, disk_write);
	}
	g_free(disk_read);
	g_free(disk_write);
}
