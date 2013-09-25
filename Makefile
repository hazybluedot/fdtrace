CC := cc
CFLAGS ?= -Wall -Wextra -std=c99

all: fdtrace

fdtrace: fdtrace.o util.o

clean:
	rm -f *.o fdtrace

install:
	cp fdtrace /usr/local/bin/

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^
