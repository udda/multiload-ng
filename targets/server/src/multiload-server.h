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


#ifndef MLSRV_HEADER__MULTILOAD_SERVER_H__INCLUDED
#define MLSRV_HEADER__MULTILOAD_SERVER_H__INCLUDED


#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>

#include <multiload.h>
#include <microhttpd.h>


#include "core/types.h"
#include "core/log.h"
#include "core/util.h"

#include "http/server.h"
#include "http/connection.h"
#include "http/command.h"
#include "http/response.h"


#endif /* MLSRV_HEADER__MULTILOAD_SERVER_H__INCLUDED */
