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


ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
	-DLOCALEDIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

bin_PROGRAMS = multiload-ng-server

multiload_ng_server_CFLAGS = \
	-std=gnu11 \
	-Isrc \
	-Wall -Wextra \
	-Wpedantic

if MULTILOAD_ENABLE_DEBUG
multiload_ng_server_CFLAGS += -Og -ggdb3
else
multiload_ng_server_CFLAGS += -O3 -g
endif


multiload_ng_server_SOURCES = \
	src/main.c \
	src/multiload-server.h \
	src/core/types.h \
	src/core/log.c src/core/log.h \
	src/core/util.c src/core/util.h \
	src/http/command.c src/http/command.h \
	src/http/connection.c src/http/connection.h \
	src/http/response.c src/http/response.h \
	src/http/server.c src/http/server.c


.PHONY:clean-git
clean-git:
	git clean -fdX
