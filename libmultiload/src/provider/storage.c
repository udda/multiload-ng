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


const MlGraphTypeInterface ML_PROVIDER_STORAGE_IFACE = {
	.name			= "storage",
	.label			= N_("Storage"),
	.description	= N_("Shows used storage space or used inodes."),

	.hue			= 224,

	.n_data			= 1,
	.dataset_mode	= ML_DATASET_MODE_PROPORTIONAL,

	.init_fn		= ml_provider_storage_init,
	.config_fn		= ml_provider_storage_config,
	.get_fn			= ml_provider_storage_get,
	.destroy_fn		= ml_provider_storage_destroy,
	.sizeof_fn		= ml_provider_storage_sizeof,
	.unpause_fn		= NULL,
	.caption_fn		= ml_provider_storage_caption,

	.helptext		= NULL
};


typedef struct {
	MlAssocArray *aa_disks; // caches MlFilesystemInfo structures (keys are the device names)

	bool use_inodes; // count inodes instead of blocks
	bool count_root; // also count blocks/inodes available for root user
	char **filter;

	uint64_t avail;
	uint64_t total;
	MlGrowBuffer *used_fs;
} STORAGEstate;


mlPointer
ml_provider_storage_init (MlConfig *config)
{
	STORAGEstate *s = ml_new (STORAGEstate);

	s->aa_disks = ml_assoc_array_new (8, (MlDestroyFunc)ml_filesystem_info_destroy);
	s->used_fs = ml_grow_buffer_new (128);
	s->use_inodes = false;
	s->count_root = false;
	s->filter = NULL;

	ml_config_add_entry (config,
		"use_inodes",
		ML_VALUE_TYPE_BOOLEAN,
		&s->use_inodes,
		_("Use inodes"),
		_("Calculate values using inodes instead of blocks.")
	);
	ml_config_add_entry (config,
		"count_root",
		ML_VALUE_TYPE_BOOLEAN,
		&s->count_root,
		_("Include root quota"),
		_("Also count blocks/inodes reserved for root user.")
	);
	ml_config_add_entry (config,
		"filter",
		ML_VALUE_TYPE_STRV,
		&s->filter,
		_("Filter"),
		_("Mountpoints that aren't in this list will not be used. If this list is empty, mountpoints will be filtered automatically.")
	);

	return s;
}

void
ml_provider_storage_config (MlGraphContext *context)
{
	// every config change invalidates previous data
	ml_graph_context_set_need_data_reset (context);
}

/* in order to represent disk size with 32 bit unsigned values,
 * we count in 256KB blocks (this way being able to measure up to 1 PB) */
#define SCALE (256*1024)

void
ml_provider_storage_get (MlGraphContext *context)
{
	STORAGEstate *s = (STORAGEstate*)ml_graph_context_get_provider_data (context);
	ml_graph_context_assert (context, s != NULL);

	MlGrowArray *mounted = ml_filesystem_info_list_mounted (s->aa_disks);
	// discriminate between empty array and no array at all (useful for debugging)
	ml_graph_context_assert_with_message (context, mounted != NULL, _("Cannot find any mounted disk or volume"));
	ml_graph_context_assert_with_message (context, !ml_grow_array_is_empty (mounted), _("Cannot find any mounted disk or volume"));

	s->avail = 0;
	s->total = 0;

	ml_grow_buffer_rewind (s->used_fs);

	ml_grow_array_for (mounted, i) {
		MlFilesystemInfo *fi = (MlFilesystemInfo*)ml_grow_array_get (mounted, i);
		if_unlikely (fi == NULL)
			break;

		// skip entries not present in filter
		if (s->filter != NULL && !ml_strv_contains (s->filter, fi->mountpoint))
			continue;

		// skip RAM filesystems
		if (!fi->is_block_device)
			continue;

		// build list of used devices
		ml_grow_buffer_append_string (s->used_fs, fi->mountpoint);
		if (!ml_grow_array_is_last_index(mounted, i))
			ml_grow_buffer_append_string (s->used_fs, ", ");

		if (s->use_inodes) {
			s->avail += (s->count_root) ? fi->inodes_free_root : fi->inodes_free_user;
			s->total += fi->inodes_total;
		} else {
			s->avail += (s->count_root) ? fi->bytes_free_root : fi->bytes_free_user;
			s->total += fi->bytes_total;
		}
	}

	ml_grow_array_destroy (mounted, true, false);

	uint32_t used  = (s->use_inodes) ? (s->total - s->avail) : ((s->total - s->avail) / SCALE);
	uint32_t total = (s->use_inodes) ? (s->total)            : ((s->total)            / SCALE);

	ml_graph_context_set_data (context, 0, used);
	ml_graph_context_set_max  (context,    total);
}

void
ml_provider_storage_destroy (mlPointer provider_data)
{
	STORAGEstate *s = (STORAGEstate*)provider_data;

	if_likely (s != NULL) {
		ml_strv_free (s->filter);

		ml_assoc_array_destroy (s->aa_disks);
		ml_grow_buffer_destroy (s->used_fs, true);
		free (s);
	}
}

size_t
ml_provider_storage_sizeof (mlPointer provider_data)
{
	STORAGEstate *s = (STORAGEstate*)provider_data;
	if_unlikely (s == NULL)
		return 0;

	size_t size = sizeof (STORAGEstate);

	size += ml_assoc_array_sizeof (s->aa_disks, sizeof (MlFilesystemInfo));
	size += ml_strv_sizeof (s->filter);
	size += ml_grow_buffer_sizeof (s->used_fs);

	return size;
}

void
ml_provider_storage_caption (MlCaption *caption, ML_UNUSED MlDataset *ds, mlPointer provider_data)
{
	STORAGEstate *s = (STORAGEstate*)provider_data;
	if_unlikely (s == NULL)
		return;

	if (s->use_inodes) {
		ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Free inodes: %llu"), (unsigned long long)s->avail);
		ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");
		ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Total inodes: %llu"), (unsigned long long)s->total);
	} else {
		char buf_avail[24];
		char buf_total[24];
		ml_string_format_size_s (s->avail, "B", false, buf_avail, sizeof (buf_avail));
		ml_string_format_size_s (s->total, "B", false, buf_total, sizeof (buf_total));

		ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Free storage: %s"), buf_avail);
		ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, "\n");
		ml_caption_append (caption, ML_CAPTION_COMPONENT_BODY, _("Total storage: %s"), buf_total);
	}

	// TRANSLATORS: list of monitored mountpoints for measuring storage space
	ml_caption_append (caption, ML_CAPTION_COMPONENT_FOOTER, _("Monitored mountpoints: %s"), ml_grow_buffer_get_string (s->used_fs));
}
