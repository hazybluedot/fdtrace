CC := gcc
CFLAGS ?= -Wall -Wextra -std=c99 -pedantic

all: fdtrace

fdtrace: fdtrace.o util.o

clean:
	rm -f *.o fdtrace

install:
	cp fdtrace /usr/local/bin/
