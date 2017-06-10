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
 * along with Multiload-ng.  If not, see <hutilttp://www.gnu.org/licenses/>.
 */

#include <multiload-server.h>


char *
text_to_password (const char *plaintext)
{
	if (plaintext == NULL)
		return NULL;

	int i;
	char *ret = strdup (plaintext);

	for (i = 0; ret[i] != '\0'; i++)
		ret[i] = '*';

	return ret;
}

const char *
sockaddr_get_protocol (struct sockaddr *addr)
{
	switch(addr->sa_family) {
		case AF_INET:
			return "IPv4";
		case AF_INET6:
			return "IPv6";
		default:
			return NULL;
	}
}

void
sockaddr_get_address (struct sockaddr *addr, char *buf, size_t buflen)
{
	if (buf == NULL || buflen < MAX(INET_ADDRSTRLEN, INET6_ADDRSTRLEN))
		return;

	switch(addr->sa_family) {
		case AF_INET: {
		    struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
		    inet_ntop (AF_INET, &(addr_in->sin_addr), buf, INET_ADDRSTRLEN);
		    break;
		}
		case AF_INET6: {
		    struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)addr;
		    inet_ntop (AF_INET6, &(addr_in6->sin6_addr), buf, INET6_ADDRSTRLEN);
		    break;
		}
		default:
			buf[0] = '\0';
			return;
	}
}

int
sockaddr_get_port (struct sockaddr *addr)
{
	switch(addr->sa_family) {
		case AF_INET:
		    return ((struct sockaddr_in *)addr)->sin_port;
		case AF_INET6:
			return ((struct sockaddr_in6 *)addr)->sin6_port;
		default:
			return 0;
	}
}

bool
parse_credentials (const char *def, char **username, char **password)
{
	if (def == NULL || username == NULL || password == NULL)
		return false;

	size_t len = strlen (def);
	if (len == 0)
		return false;

	char *pch = strchr (def, ':');
	if (pch == NULL || pch <= def)
		return false;

	size_t username_len = pch-def;
	if (username_len >= len - 1)
		return false;

	size_t password_len = len - 1 - username_len;

	*username = strndup (def, username_len);
	*password = strndup (def + username_len + 1, password_len);

	return true;
}
