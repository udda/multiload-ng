# Copyright (C) 2017 Mario Cianciolo <mr.udda@gmail.com>
#
# This file is part of Multiload-ng.
#
# Multiload-ng is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Multiload-ng is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Multiload-ng.  If not, see <http://www.gnu.org/licenses/>.


SUBDIRS = doc icon po src/resources

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
	-DLOCALEDIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

lib_LTLIBRARIES = libmultiload.la


libmultiload_la_CFLAGS = \
	-pthread -lm \
	-Isrc \
	-std=gnu11 \
	-fvisibility=hidden \
	-Wall -Wextra -Werror
#	-Wpedantic

libmultiload_la_LIBADD = src/resources/libmultiload_resources.la

libmultiload_la_LDFLAGS = \
	-version-info `echo $(PACKAGE_VERSION) | tr . :` \
	-export-symbols-regex '^multiload_' \
	-O 1

if LTO
libmultiload_la_CFLAGS += -flto
libmultiload_la_LDFLAGS += -Wc,-flto
endif

if DEBUG
#debug flags
libmultiload_la_CFLAGS += -Og -ggdb3
else
#release flags
libmultiload_la_CFLAGS += -O3 -g
libmultiload_la_LDFLAGS += -Wc,-O3
endif


libmultiload_la_SOURCES = \
	src/multiload.h \
	src/global.h \
	src/resources/resources.h \
	src/core/caption.c 					src/core/caption.h \
	src/core/config.c 					src/core/config.h \
	src/core/debug.c 					src/core/debug.h \
	src/core/dataset.c 					src/core/dataset.h \
	src/core/direction.c 				src/core/direction.h \
	src/core/memory.c 					src/core/memory.h \
	src/core/orientation.c 				src/core/orientation.h \
										src/core/types.h \
	src/drawing/color.c 				src/drawing/color.h \
	src/drawing/gradient.c 				src/drawing/gradient.h \
	src/drawing/surface.c 				src/drawing/surface.h \
	src/graph/context.c 				src/graph/context.h \
	src/graph/graph.c 					src/graph/graph.h \
	src/graph/style.c 					src/graph/style.h \
	src/graph/theme.c 					src/graph/theme.h \
	src/graph/themes_repository.c 		src/graph/themes_repository.h \
	src/graph/type.c 					src/graph/type.h \
	src/multiload/container.c 			src/multiload/container.h \
	src/multiload/notifier.c 			src/multiload/notifier.h \
	src/multiload/shared.c 				src/multiload/shared.h \
	src/multiload/timer.c 				src/multiload/timer.h \
	src/provider/battery.c 				src/provider/battery.h \
	src/provider/cpu.c 					src/provider/cpu.h \
	src/provider/cpufreq.c 				src/provider/cpufreq.h \
	src/provider/diskio.c 				src/provider/diskio.h \
	src/provider/entropy.c 				src/provider/entropy.h \
	src/provider/intrrate.c 			src/provider/intrrate.h \
	src/provider/javascript.c 			src/provider/javascript.h \
	src/provider/loadavg.c 				src/provider/loadavg.h \
	src/provider/net.c 					src/provider/net.h \
	src/provider/parametric.c 			src/provider/parametric.h \
	src/provider/ping.c 				src/provider/ping.h \
	src/provider/procrate.c 			src/provider/procrate.h \
	src/provider/ram.c 					src/provider/ram.h \
	src/provider/random.c 				src/provider/random.h \
	src/provider/sockets.c 				src/provider/sockets.h \
	src/provider/storage.c 				src/provider/storage.h \
	src/provider/swap.c 				src/provider/swap.h \
	src/provider/testfail.c 			src/provider/testfail.h \
	src/provider/temperature.c 			src/provider/temperature.h \
	src/provider/threads.c 				src/provider/threads.h \
	src/provider/wifi.c 				src/provider/wifi.h \
	src/sys/battery_info.c 				src/sys/battery_info.h \
	src/sys/cpu_info.c 					src/sys/cpu_info.h \
	src/sys/disk_info.c 				src/sys/disk_info.h \
	src/sys/filesystem_info.c 			src/sys/filesystem_info.h \
	src/sys/net_info.c 					src/sys/net_info.h \
	src/sys/process_info.c				src/sys/process_info.h \
	src/sys/process_monitor.c			src/sys/process_monitor.h \
	src/sys/sys_info.c 					src/sys/sys_info.h \
	src/sys/temperature_info.c			src/sys/temperature_info.h \
	src/theme/color.c					src/theme/color.h \
	src/theme/theme.c					src/theme/theme.h \
	src/theme/store.c					src/theme/store.h \
	src/third-party/cJSON/cJSON.c		src/third-party/cJSON/cJSON.h \
	src/third-party/duktape/duktape.c	src/third-party/duktape/duktape.h \
										src/third-party/duktape/duk_config.h \
	src/third-party/miniz/miniz.c		src/third-party/miniz/miniz.h \
	src/util/ansi_colors.c				src/util/ansi_colors.h \
	src/util/arit.c 					src/util/arit.h \
	src/util/assoc_array.c 				src/util/assoc_array.h \
	src/util/grow_array.c 				src/util/grow_array.h \
	src/util/grow_buffer.c 				src/util/grow_buffer.h \
	src/util/infofile.c 				src/util/infofile.h \
	src/util/io.c 						src/util/io.h \
	src/util/json.c						src/util/json.h \
	src/util/string.c 					src/util/string.h \
	src/util/system.c 					src/util/system.h \
	src/util/zip.c						src/util/zip.h \
	src/include/multiload.c 			src/include/multiload.h


# pkg-config
pkgconfig_DATA = libmultiload-2.pc
pkgconfigdir = ${libdir}/pkgconfig


# include
include_HEADERS = src/include/multiload.h


# multiload-debug-collector
bin_PROGRAMS = multiload-debug-collector
multiload_debug_collector_SOURCES = bin/multiload-debug-collector.c
multiload_debug_collector_LDADD = -lmultiload
multiload_debug_collector_CFLAGS = -O3 -Isrc/include

if DEBUG
#debug flags
multiload_debug_collector_CFLAGS += -Og -ggdb3
else
#release flags
multiload_debug_collector_CFLAGS += -O3 -g
endif


EXTRA_DIST = \
	build-aux/config.rpath \
	m4/ChangeLog \
	build-aux/config.rpath


# Additional targets
.PHONY:doc clean-git
doc:
	make -C doc

clean-git:
	git clean -fdX
