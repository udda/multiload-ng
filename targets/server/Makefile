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


EXECUTABLE_NAME = multiload-ng-server

all: $(EXECUTABLE_NAME)

$(EXECUTABLE_NAME):		src/main.c \
						src/core/log.c \
						src/core/util.c \
						src/http/command.c \
						src/http/connection.c \
						src/http/response.c \
						src/http/server.c
	gcc \
		`pkg-config --libs --cflags libmicrohttpd` \
		`pkg-config --libs --cflags libmultiload-2` \
		-ggdb3 -O0 \
		-std=gnu11 \
		-Isrc \
		-Wall -Wextra \
		-Wpedantic \
		-o $@ \
		$^
#		-O3
#		-Werror


clean:
	rm -f $(EXECUTABLE_NAME)

valgrind:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes ./$(EXECUTABLE_NAME)

gdb:
	gdb $(GDB_ARGS) ./$(EXECUTABLE_NAME) -ex "set args --file test-config.json" -ex "break main" -ex "run"

bt:
	gdb -ex "set args --file test-config.json" -ex "run" -ex "bt full" ./$(EXECUTABLE_NAME)

test: all
	./$(EXECUTABLE_NAME) --file "test-config.json"

help: all
	./$(EXECUTABLE_NAME) --help
