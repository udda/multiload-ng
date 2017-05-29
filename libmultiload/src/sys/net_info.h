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

#ifndef ML_HEADER__SYS_NET_INFO_H__INCLUDED
#define ML_HEADER__SYS_NET_INFO_H__INCLUDED
ML_HEADER_BEGIN


typedef struct {
	char *name;

	char address[18]; // enough for a standard MAC address, must be statically allocated (ml_infofile_read_string_s)
	uint32_t flags;
	uint32_t ifindex;

	bool is_loopback;

	uint64_t byte_read;
	uint64_t byte_write;
	uint64_t pk_read;
	uint64_t pk_write;

	char *path_address;
	char *path_flags;
	char *path_ifindex;
} MlNetInfo;


MlNetInfo *
ml_net_info_new (const char *if_name);

void
ml_net_info_destroy (MlNetInfo *ni)
;

size_t
ml_net_info_sizeof (MlNetInfo *ni)
ML_FN_SIZEOF;

bool
ml_net_info_update (MlNetInfo *ni, char *net_dev_line)
ML_FN_WARN_UNUSED_RESULT;

MlGrowArray *
ml_net_info_list_usable_ifaces (MlAssocArray *aa);

MlGrowBuffer *
ml_net_info_generate_report ();


ML_HEADER_END
#endif /* ML_HEADER__SYS_NET_INFO_H__INCLUDED */
