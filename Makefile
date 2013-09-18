all: fdtrace

fdtrace: fdtrace.o util.o

clean:
	rm -f *.o fdtrace
