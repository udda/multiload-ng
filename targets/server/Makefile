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

run: all
	./$(EXECUTABLE_NAME) --file "test-config.json"

help: all
	./$(EXECUTABLE_NAME) --help
