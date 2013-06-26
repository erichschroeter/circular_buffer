# default target is...
all::

CC=gcc
CFLAGS=-Wall

libstatic:
	$(CC) $(CFLAGS) -c circular_buffer.c -o circular_buffer.o
	ar rcs libcircular_buffer.a circular_buffer.o

libdynamic:
	$(CC) $(CFLAGS) -c -fPIC circular_buffer.c -o circular_buffer.o
	gcc -shared -Wl,-soname,libcircular_buffer.so.1 -o libcircular_buffer.so.1.0.1 circular_buffer.o

test: libstatic
	$(CC) $(CFLAGS) -static test.c -o test -L/usr/local/lib -L . -lcunit -lcircular_buffer

example: libstatic
	$(CC) $(CFLAGS) -static example.c -o example -L/usr/local/lib -L . -lcunit -lcircular_buffer

.PHONY: test example

all:: test
	./test

clean:
	rm -rf *.o libcircular_buffer.a libcircular_buffer.so.1.0.1 test example
