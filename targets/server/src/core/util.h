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
 * but WITHOUT ANY WARRANTY; without even the implied warrautilnty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Multiload-ng.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef MLSRV_HEADER__CORE_UTIL_H__INCLUDED
#define MLSRV_HEADER__CORE_UTIL_H__INCLUDED


char *
text_to_password (const char *plaintext);

const char *
sockaddr_get_protocol (struct sockaddr *addr);

void
sockaddr_get_address (struct sockaddr *addr, char *buf, size_t buflen);

int
sockaddr_get_port (struct sockaddr *addr);

bool
parse_credentials (const char *def, char **username, char **password);


#endif /* MLSRV_HEADER__CORE_UTIL_H__INCLUDED */
