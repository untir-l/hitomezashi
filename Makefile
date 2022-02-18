.POSIX:

CC = gcc
CFLAGS = -std=c11 -Wall -Wpedantic -O3 `sdl2-config --cflags`
LDLIBS = `sdl2-config --libs`
AR = ar
ARFLAGS = rcs

all: libhitomezashi.a hitomezashi_cli

libhitomezashi.a: hitomezashi.o
	$(AR) $(ARFLAGS) libhitomezashi.a hitomezashi.o

hitomezashi.o: hitomezashi.c hitomezashi.h

hitomezashi_cli: hitomezashi_cli.o libhitomezashi.a

hitomezashi_cli.o: hitomezashi_cli.c hitomezashi_cli.h

clean:
	rm -f *.a *.o hitomezashi_cli