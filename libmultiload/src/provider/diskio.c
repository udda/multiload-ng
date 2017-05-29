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


const MlGraphTypeInterface ML_PROVIDER_DISKIO_IFACE = {
	.name			= "diskio",
	.label			= N_("Disk I/O"),
	.description	= N_("Shows disk I/O, split by direction (read/write) and type (sectors or busy time)."),

	.hue			= 31,

	.n_data			= 2,
	.dataset_mode	= ML_DATASET_MODE_ABSOLUTE,

	.init_fn		= ml_provider_diskio_init,
	.config_fn		= ml_provider_diskio_config,
	.get_fn			= ml_provider_diskio_get,
	.destroy_fn		= ml_provider_diskio_destroy,
	.sizeof_fn		= ml_provider_diskio_sizeof,
	.unpause_fn		= ml_provider_diskio_unpause,
	.caption_fn		= ml_provider_diskio_caption,
	
	.helptext		= NULL
};


enum { DISK_READ, DISK_WRITE, DISK_MAX };

typedef struct {
	MlAssocArray *aa_disks; // caches MlDiskInfo structures (keys are the device names)
	uint64_t last[DISK_MAX];

	bool do_count[DISK_MAX];
	bool use_time; // count I/O busy time (ms) instead of size (sectors)
	char **filter;

	MlGrowBuffer *used_disks;
} DISKIOstate;


mlPointer
ml_provider_diskio_init (MlConfig *config)
{
	DISKIOstate *s = ml_new (DISKIOstate);

	s->aa_disks = ml_assoc_array_new (8, (MlDestroyFunc)ml_disk_info_destroy);
	s->used_disks = ml_grow_buffer_new (32);
	s->do_count[DISK_READ] = true;
	s->do_count[DISK_WRITE] = true;
	s->use_time = false;
	s->filter = NULL;

	ml_config_add_entry (config,
		"count_reads",
		ML_VALUE_TYPE_BOOLEAN,
		&s->do_count[DISK_READ],
		_("Count reads"),
		_("Measure disk reads. Set to <b>false</b> to disable.")
	);
	ml_config_add_entry (config,
		"count_writes",
		ML_VALUE_TYPE_BOOLEAN,
		&s->do_count[DISK_WRITE],
		_("Count writes"),
		_("Measure disk writes. Set to <b>false</b> to disable.")
	);
	ml_config_add_entry (config,
		"use_time",
		ML_VALUE_TYPE_BOOLEAN,
		&s->use_time,
		_("Use disk time"),
		_("Calculate values using disk busy time instead of bytes read or written.")
	);
	ml_config_add_entry (config,
		"filter",
		ML_VALUE_TYPE_STRV,
		&s->filter,
		_("Filter"),
		_("Devices that aren't in this list will not be used. If this list is empty, all available devices will be used.")
	);

	return s;
}

void
ml_provider_diskio_config (MlGraphContext *context)
{
	// every config change invalidates previous data
	ml_graph_context_set_need_data_reset (context);
}


void
ml_provider_diskio_get (MlGraphContext *context)
{
	DISKIOstate *s = (DISKIOstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	if (!s->do_count[DISK_READ] && !s->do_count[DISK_WRITE])
		return;

	MlGrowArray *mounted = ml_disk_info_list_mounted (s->aa_disks);
	// discriminate between empty array and no array at all (useful for debugging)
	ml_graph_context_assert_with_message (context, mounted != NULL, _("Cannot find any mounted disk or volume"));
	ml_graph_context_assert_with_message (context, !ml_grow_array_is_empty (mounted), _("Cannot find any mounted disk or volume"));

	uint64_t rate[DISK_MAX] = {0, 0};
	uint32_t diff[DISK_MAX] = {0, 0};

	ml_grow_buffer_rewind (s->used_disks);

	ml_grow_array_for (mounted, i) {
		MlDiskInfo *di = (MlDiskInfo*)ml_grow_array_get (mounted, i);
		if_unlikely (di == NULL)
			break;

		// skip entries not present in filter
		if (s->filter != NULL && !ml_strv_contains (s->filter, di->name))
			continue;

		// build list of used devices
		ml_grow_buffer_append_string (s->used_disks, di->name);
		if (ml_grow_array_is_last_index(mounted, i))
			ml_grow_buffer_append_string (s->used_disks, "\0");
		else
			ml_grow_buffer_append_string (s->used_disks, ", ");


		if (s->do_count[DISK_READ])
			rate[DISK_READ]  += (s->use_time) ? di->read_ms  : (di->read_sectors * di->sector_size);

		if (s->do_count[DISK_WRITE])
			rate[DISK_WRITE] += (s->use_time) ? di->write_ms : (di->write_sectors * di->sector_size);
	}

	if_unlikely (!ml_graph_context_is_first_call (context)) { // cannot calculate diff on first call
		for (int i = 0; i < DISK_MAX; i++) {
			/*
			 * Current readings can be less than previous readings for two reasons:
			 * 1. Disk stats are integers with system word size. According to Linux documentation,
			 *    they could (and will) overflow. This is a problem especially in 32 bit systems.
			 * 2. Filter changes (like removing a disk) can make total read/write count decrease.
			 *
			 * In both cases we simply ignore that readings (and set the corresponding
			 * dataset column to 0), the next ones will be correct.
			 */
			if (s->last[i] > rate[i])
				continue;
			else
				diff[i] = (uint32_t)(rate[i]-s->last[i]);
		}
	}

	memcpy (s->last, rate, sizeof(s->last));

	ml_grow_array_destroy (mounted, true, false);

	ml_graph_context_set_data (context, DISK_READ,  diff[DISK_READ]);
	ml_graph_context_set_data (context, DISK_WRITE, diff[DISK_WRITE]);
	ml_graph_context_set_max  (context, 100); // min ceiling = 100 bps
}

void
ml_provider_diskio_destroy (mlPointer provider_data)
{
	DISKIOstate *s = (DISKIOstate*)provider_data;

	if_likely (s != NULL) {
		ml_strv_free (s->filter);
		ml_assoc_array_destroy (s->aa_disks);
		ml_grow_buffer_destroy (s->used_disks, true);
		free (s);
	}
}

size_t
ml_provider_diskio_sizeof (mlPointer provider_data)
{
	DISKIOstate *s = (DISKIOstate*)provider_data;
	if_unlikely (s == NULL)
		return 0;

	size_t size = sizeof (DISKIOstate);

	size += ml_assoc_array_sizeof (s->aa_disks, sizeof (MlDiskInfo));
	size += ml_strv_sizeof (s->filter);
	size += ml_grow_buffer_sizeof (s->used_disks);

	return size;
}

void
ml_provider_diskio_unpause (MlGraphContext *context)
{
	ml_graph_context_set_first_call (context); // avoid spikes after unpause
}

void
ml_provider_diskio_caption (MlCaption *caption, MlDataset *ds, mlPointer provider_data)
{
	DISKIOstate *s = (DISKIOstate*)provider_data;
	if_unlikely (s == NULL)
		return;

	uint32_t diff_read = ml_dataset_get_value (ds, -1, DISK_READ);
	uint32_t diff_write = ml_dataset_get_value (ds, -1, DISK_WRITE);

	char buf[24];

	if (s->do_count[DISK_READ]) {
		if (s->use_time) {
			// TRANSLATORS: short form for "milliseconds"
			snprintf (buf, sizeof(buf), _("%u ms"), diff_read);
		} else {
			ml_string_format_size_s (diff_read, "B/s", false, buf, sizeof (buf));
		}

		// TRANSLATORS: disk read time or disk read speed
		ml_caption_set (caption, ML_CAPTION_COMPONENT_BODY, _("Read: %s"), buf);
	} else {
		// TRANSLATORS: disk read time/speed measuring is not active
		ml_caption_set (caption, ML_CAPTION_COMPONENT_BODY, _("Read: not measuring"));
	}

	ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");

	if (s->do_count[DISK_WRITE]) {
		if (s->use_time) {
			// TRANSLATORS: short form for "milliseconds"
			snprintf (buf, sizeof(buf), _("%u ms"), diff_write);
		} else {
			ml_string_format_size_s (diff_write, "B/s", false, buf, sizeof (buf));
		}

		// TRANSLATORS: disk write time or disk write speed
		ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "Write: %s", buf);
	} else {
		// TRANSLATORS: disk write time/speed measuring is not active
		ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Write: not measuring"));
	}

	// TRANSLATORS: list of monitored devices for disk I/O
	ml_caption_append (caption, ML_CAPTION_COMPONENT_FOOTER, "Monitored devices: %s", (const char*)ml_grow_buffer_get_data (s->used_disks));


	// top processes
	if (ML_SHARED_GET (procmon_enable)) {
		MlProcessInfo *pi_array[ML_CAPTION_TABLE_ROWS];
		ml_process_monitor_get_top (ML_SHARED_GET (procmon), ML_PROCESS_INFO_FIELD_IO, pi_array, ML_CAPTION_TABLE_ROWS);
		char buf_pi [24];

		for (int i = 0; i < ML_CAPTION_TABLE_ROWS; i++) {
			if (pi_array[i] == NULL || pi_array[i]->io_bytes_diff == 0)
				break;

			ml_string_format_size_s (pi_array[i]->io_bytes_diff, "B/s", false, buf_pi, sizeof (buf_pi));
			ml_caption_set_table_data (caption, i, 0, pi_array[i]->name);
			ml_caption_set_table_data (caption, i, 1, buf_pi);
		}
	}
}
