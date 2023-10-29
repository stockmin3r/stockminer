# Makefile for liblmdb (Lightning memory-mapped database library).
########################################################################
# Configuration. The compiler options must enable threaded compilation.
# Preprocessor macros (for CPPFLAGS) of interest...
# - MDB_USE_POSIX_MUTEX, MDB_USE_POSIX_SEM, MDB_USE_SYSV_SEM
# - MDB_DSYNC
# - MDB_FDATASYNC
# - MDB_FDATASYNC_WORKS
# - MDB_USE_PWRITEV
# - MDB_USE_ROBUST
CC	= gcc
AR	= ar
W	= -W -Wall -Wno-unused-parameter -Wbad-function-cast -Wuninitialized
THREADS = -pthread
OPT     = -O2 -g
CFLAGS	= $(THREADS) $(OPT) $(W) $(XCFLAGS)
########################################################################

IHDRS	= lmdb.h
all:	liblmdb.a

clean:
	rm -rf *.[ao] *.[ls]o *~ testdb

liblmdb.a:	mdb.o midl.o
	$(AR) rs $@ mdb.o midl.o

mdb.o: mdb.c lmdb.h midl.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c mdb.c

midl.o: midl.c midl.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c midl.c

mdb.lo: mdb.c lmdb.h midl.h
	$(CC) $(CFLAGS) -fPIC $(CPPFLAGS) -c mdb.c -o $@

midl.lo: midl.c midl.h
	$(CC) $(CFLAGS) -fPIC $(CPPFLAGS) -c midl.c -o $@

%:	%.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

%.o:	%.c lmdb.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

