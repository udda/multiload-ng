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

#ifndef __GNUC__
#error "GCC is required to compile this library."
#endif

#ifndef ML_HEADER__MULTILOAD_H__INCLUDED
#define ML_HEADER__MULTILOAD_H__INCLUDED

/*
 * MULTILOAD-NG MAIN INCLUDE FILE
 * Include this header in every .c file (not in other headers)
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
#error "Missing <config.h>."
#endif


/* GNU feature test macros */
// libc GNU extensions, eg. strtol_l
#define _GNU_SOURCE 1
// LFS (Large File Support), eg. fopen64
#define _LARGEFILE64_SOURCE 1


#include <dirent.h>
#include <errno.h>
#include <execinfo.h>
#include <fcntl.h>
#include <float.h>
#include <inttypes.h>
#include <libintl.h>
#include <locale.h>
#include <math.h>
#include <mntent.h>
#include <pthread.h>
#include <regex.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/wait.h>

//#include <json.h>


#include "global.h"
#include "resources/resources.h"

#include "third-party/cJSON/cJSON.h"
#include "third-party/duktape/duktape.h"
#include "third-party/miniz/miniz.h"

#include "core/types.h"
#include "core/caption.h"
#include "core/config.h"
#include "core/debug.h"
#include "core/dataset.h"
#include "core/direction.h"
#include "core/memory.h"
#include "core/orientation.h"

#include "drawing/color.h"
#include "drawing/gradient.h"
#include "drawing/surface.h"

#include "graph/context.h"
#include "graph/graph.h"
#include "graph/style.h"
#include "graph/theme.h"
#include "graph/themes_repository.h"
#include "graph/type.h"

#include "multiload/container.h"
#include "multiload/notifier.h"
#include "multiload/shared.h"
#include "multiload/timer.h"

#include "provider/battery.h"
#include "provider/cpu.h"
#include "provider/cpufreq.h"
#include "provider/diskio.h"
#include "provider/entropy.h"
#include "provider/intrrate.h"
#include "provider/javascript.h"
#include "provider/loadavg.h"
#include "provider/net.h"
#include "provider/parametric.h"
#include "provider/ping.h"
#include "provider/procrate.h"
#include "provider/ram.h"
#include "provider/random.h"
#include "provider/sockets.h"
#include "provider/storage.h"
#include "provider/swap.h"
#include "provider/testfail.h"
#include "provider/temperature.h"
#include "provider/threads.h"
#include "provider/wifi.h"

#include "theme/color.h"
#include "theme/store.h"
#include "theme/theme.h"

#include "util/ansi_colors.h"
#include "util/arit.h"
#include "util/assoc_array.h"
#include "util/grow_array.h"
#include "util/grow_buffer.h"
#include "util/infofile.h"
#include "util/io.h"
#include "util/json.h"
#include "util/string.h"
#include "util/system.h"
#include "util/zip.h"

#include "sys/battery_info.h"
#include "sys/cpu_info.h"
#include "sys/disk_info.h"
#include "sys/filesystem_info.h"
#include "sys/net_info.h"
#include "sys/process_info.h"
#include "sys/process_monitor.h"
#include "sys/sys_info.h"
#include "sys/temperature_info.h"

#include "include/multiload.h"


#endif /* ML_HEADER__MULTILOAD_H__INCLUDED */
