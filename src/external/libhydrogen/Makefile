PREFIX ?= /usr/local
WFLAGS ?= -Wall -Wextra -Wmissing-prototypes -Wdiv-by-zero -Wbad-function-cast -Wcast-align -Wcast-qual -Wfloat-equal -Wmissing-declarations -Wnested-externs -Wno-unknown-pragmas -Wpointer-arith -Wredundant-decls -Wstrict-prototypes -Wswitch-enum -Wno-type-limits
CFLAGS ?= -Os -mtune=native -fno-exceptions $(WFLAGS) -ggdb
CFLAGS += -I.
OBJ     = hydrogen.o
AR     ?= ar
RANLIB ?= ranlib

SRC = \
	hydrogen.c \
	hydrogen.h \
	impl/common.h \
	impl/core.h \
	impl/gimli-core.h \
	impl/hash.h \
	impl/hydrogen_p.h \
	impl/kdf.h \
	impl/kx.h \
	impl/pwhash.h \
	impl/random.h \
	impl/secretbox.h \
	impl/sign.h \
	impl/x25519.h

all: lib

lib: libhydrogen.a

uninstall:
	rm -f $(PREFIX)/lib/libhydrogen.a
	rm -f $(PREFIX)/include/hydrogen.h

$(OBJ): $(SRC)

libhydrogen.a: $(OBJ)
	$(AR) -r $@ $^
	$(RANLIB) $@

.PHONY: clean

clean:
	rm -f libhydrogen.a *.wasm hydrogen.html hydrogen.js $(OBJ)

distclean: clean
