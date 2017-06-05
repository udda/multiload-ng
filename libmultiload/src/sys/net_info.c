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


#define PATH_NET_DEV "/proc/net/dev"

static int
_ml_net_info_sort_by_ifindex (MlNetInfo **a, MlNetInfo **b)
{
	if ((*a)->ifindex > (*b)->ifindex)
		return 1;
	else if ((*a)->ifindex < (*b)->ifindex)
		return -1;
	else // equals
		return 0;
}

MlNetInfo*
ml_net_info_new (const char *if_name)
{
	MlNetInfo *ni = ml_new (MlNetInfo);

	ni->name = ml_strdup (if_name);

	ni->path_address =	ml_strdup_printf ("/sys/class/net/%s/address", ni->name);
	ni->path_flags =	ml_strdup_printf ("/sys/class/net/%s/flags",   ni->name);
	ni->path_ifindex =	ml_strdup_printf ("/sys/class/net/%s/ifindex", ni->name);

	if (!ml_file_is_readable (ni->path_address) || !ml_file_is_readable (ni->path_flags) || !ml_file_is_readable (ni->path_ifindex)) {
		ml_net_info_destroy (ni);
		return NULL;
	}

	return ni;
}

void
ml_net_info_destroy (MlNetInfo *ni)
{
	if_unlikely (ni == NULL)
		return;

	free (ni->name);
	free (ni->path_address);
	free (ni->path_flags);
	free (ni->path_ifindex);

	free (ni);
}


size_t
ml_net_info_sizeof (MlNetInfo *ni)
{
	if_unlikely (ni == NULL)
		return 0;

	size_t size = sizeof (MlNetInfo);

	size += ml_string_sizeof(ni->name);
	size += ml_string_sizeof(ni->path_address);
	size += ml_string_sizeof(ni->path_flags);
	size += ml_string_sizeof(ni->path_ifindex);

	return size;
}

bool
ml_net_info_update (MlNetInfo *ni, char *net_dev_line)
{
	if_unlikely (ni == NULL || net_dev_line == NULL)
		return false;

	if (!ml_string_has_prefix (net_dev_line, ni->name))
		return false; // wrong /proc/net/dev line!

	if (4 != sscanf(net_dev_line, "%*s %"SCNu64" %"SCNu64" %*u %*u %*u %*u %*u %*u %"SCNu64" %"SCNu64, &ni->byte_read, &ni->pk_read, &ni->byte_write, &ni->pk_write))
		return false;

	if (!ml_infofile_read_hex32 (ni->path_flags, &ni->flags))
		return false;
	if (!ml_infofile_read_string_s (ni->path_address, ni->address, sizeof(ni->address), NULL))
		return false;
	if (!ml_infofile_read_uint32 (ni->path_ifindex, &ni->ifindex))
		return false;

	ni->is_loopback = ni->flags & IFF_LOOPBACK;

	return true;
}

MlGrowArray*
ml_net_info_list_usable_ifaces (MlAssocArray *aa)
{
	// aa must be a MlAssocArray that caches MlNetInfo pointers (keys are the iface names)
	if_unlikely (aa == NULL)
		return NULL;

	FILE *f = fopen (PATH_NET_DEV, "r");
	if_unlikely (f == NULL)
		return NULL;

	MlGrowArray *ifaces = ml_grow_array_new (8, NULL);

	char if_name [32];
	char *line = NULL;
	size_t n = 0;
	while (getline (&line, &n, f) >= 0) {
		// skip header lines of /proc/net/dev
		if (strchr (line, ':') == NULL)
			continue;

		ml_string_trim (line);

		// extract if_name from line
		for (size_t i = 0; i < sizeof(if_name); i++) {
			if (line[i] != ':' && line[i] != ' ') {
				if_name[i] = line[i];
			} else {
				if_name[i] = '\0';
				break;
			}
		}

		// lookup existing data and create it if necessary
		MlNetInfo *ni = (MlNetInfo*)ml_assoc_array_get (aa, if_name);
		if (ni == NULL) {
			ni = ml_net_info_new (if_name);
			if (ni == NULL)
				continue;

			ml_assoc_array_put (aa, if_name, ni);
		}

		if (!ml_net_info_update (ni, line))
			continue;

		if (!(ni->flags & IFF_UP))
			continue; // device is down, ignore

		// all OK - add interface to list
		ml_grow_array_append (ifaces, ni);
	}
	free (line);
	fclose (f);

	// sort array by ifindex (so we can take first device when they are same address)
	ml_grow_array_sort (ifaces, (MlCompareFunc)_ml_net_info_sort_by_ifindex);
	return ifaces;
}

MlGrowBuffer *
ml_net_info_generate_report ()
{
	MlGrowBuffer *gbuf = ml_grow_buffer_new (2048);

	FILE *f = fopen (PATH_NET_DEV, "r");
	if_unlikely (f == NULL) {
		ml_grow_buffer_append_printf (gbuf, "Error: cannot read network interfaces from '%s': %s\n", PATH_NET_DEV, strerror (errno));
		return gbuf;
	}
	ml_grow_buffer_append_printf (gbuf, "Reading network interfaces from %s\n", PATH_NET_DEV);

	char if_name [32];
	char *line = NULL;
	size_t n = 0;

	int total = 0, valid = 0;
	while (getline (&line, &n, f) >= 0) {
		ml_string_trim (line);

		// skip header lines of /proc/net/dev
		if (strchr (line, ':') == NULL) {
			ml_grow_buffer_append_printf (gbuf, "(skipping header line: \"%s\")\n", line);
			continue;
		}

		ml_grow_buffer_append_printf (gbuf, "\n\nNETWORK INTERFACE #%d\n---------------\n", total++);

		// extract if_name from line
		for (size_t i = 0; i < sizeof(if_name); i++) {
			if (line[i] != ':' && line[i] != ' ') {
				if_name[i] = line[i];
			} else {
				if_name[i] = '\0';
				break;
			}
		}

		ml_grow_buffer_append_printf (gbuf, "Name:                     %s\n", if_name);

		MlNetInfo *ni = ml_net_info_new (if_name);
		if_unlikely (ni == NULL) {
			ml_grow_buffer_append_printf (gbuf, "Error: cannot create MlNetInfo. Maybe access denied to some files.\n");
			continue;
		}

		if (!ml_net_info_update (ni, line)) {
			ml_grow_buffer_append_printf (gbuf, "Error: cannot update interface stats for %s\n", if_name);
			continue;
		}

		ml_grow_buffer_append_printf (gbuf, "Status:                   %s\n", (ni->flags & IFF_UP) ? "up" : "down");
		ml_grow_buffer_append_printf (gbuf, "Is loopback interface:    %s\n", ni->is_loopback ? "yes" : "no");
		ml_grow_buffer_append_printf (gbuf, "Physical address:         %s\n", ni->address);
		ml_grow_buffer_append_printf (gbuf, "Unique interface index:   %"PRIu32"\n", ni->ifindex);
		ml_grow_buffer_append_printf (gbuf, "Device flags:             0x%"PRIx32"\n", ni->flags);
		ml_grow_buffer_append_printf (gbuf, "Read/Write (bytes):       %"PRIu64" / %"PRIu64"\n", ni->byte_read, ni->byte_write);
		ml_grow_buffer_append_printf (gbuf, "Read/Write (packets):     %"PRIu64" / %"PRIu64"\n", ni->pk_read, ni->pk_write);

		valid++;

		ml_net_info_destroy (ni);
	}
	free (line);
	fclose (f);

	ml_grow_buffer_append_printf (gbuf, "\n\n");
	ml_grow_buffer_append_printf (gbuf, "Found %d usable interfaces, out of %d total interfaces.\n", valid, total);

	return gbuf;
}
