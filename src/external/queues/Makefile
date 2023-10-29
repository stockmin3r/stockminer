CC      ?= gcc
AR      ?= ar
RANLIB  ?= ranlib
CFLAGS  ?= -g
LDFLAGS := -lpthread -lm
SRC     := hzdptr.c msqueue.c xxhash.c queue.h hzdptr.h xxhash.h

all:
	$(CC) $(CFLAGS) -c msqueue.c
	$(CC) $(CFLAGS) -c hzdptr.c
	$(CC) $(CFLAGS) -c xxhash.c
	$(AR) -r libqueues.a
	$(RANLIB) libqueues.a

clean:
	rm -rf *.o *~* *.a
