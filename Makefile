
################
#   Makefile   #
################
EXE      := stockminer
EXT      := src/external/
CC       := gcc
CFLAGS   := -O0
CPPFLAGS := -fpermissive
LDFLAGS  := -lz -lpthread -lssl -lcrypto -ldl -lm -lsodium -lcurl -Lbuild/lib
LIBOBJ   := $(EXT)/libhydrogen/libhydrogen.a $(EXT)/lmdb/liblmdb.a $(EXT)/queues/libqueues.a
WARN     := -Wno-address-of-packed-member -Wno-unused-result -Wno-write-strings -Wno-deprecated -Wno-misleading-indentation
DEBUG    := yes

LDFLAGS  += $(LIBOBJ)

# XXX: refuses to add src/external/yyjson.c for some reason (Makefile is broken)
################
#  SRC & OBJ   #
################
C_SUBDIR := src/c src/c/stocks src/external
H_SUBDIR := src/c/include/ src/external/
INCS     := $(wildcard *.h $(foreach fd, $(H_SUBDIR), $(fd)/*.h))
SRC      := $(wildcard *.c $(foreach fd, $(C_SUBDIR), $(fd)/*.c))
OBJ      := $(filter %.o, $(SRC:.c=.o),$(OBJ))

################
#  filter-out  #
################
SRC      := $(filter-out src/c/os/windows.c,$(SRC))
OBJ      := $(filter-out src/c/os/windows.o,$(OBJ))
SRC      := $(filter-out src/c/os/unix.c src/external/strptime.c,$(SRC))
OBJ      := $(filter-out src/c/os/unix.o src/external/strptime.o,$(OBJ))
SRC      := $(filter-out src/external/strptime.c,$(SRC))
OBJ      := $(filter-out src/external/strptime.o,$(OBJ))

################
#   includes   #
################
INCLUDE  := -Isrc/c/include/ -Ibuild/ -Isrc/external/
INCLUDE  += -Isrc/external/lmdb
INCLUDE  += -Isrc/external/libhydrogen
INCLUDE  += -Isrc/external/queues
CFLAGS   += $(INCLUDE)
CFLAGS   += $(WARN)

################
# Unix/Windows #
################
ifeq ($(OS),Windows_NT)
	OperatingSystem := Windows
	SRC += src/c/os/windows.c src/external/strptime.c
	OBJ += src/c/os/windows.o src/external/strptime.o
	SRC += src/external/yyjson.c
	OBJ += src/external/yyjson.o
else
	OperatingSystem := $(shell sh -c 'uname')
	SRC += src/c/os/unix.c
	OBJ += src/c/os/unix.o
	SRC += src/external/yyjson.c
	OBJ += src/external/yyjson.o
endif

ifeq ($(DEBUG),yes)
	CFLAGS += -ggdb3
endif

###############
# Main Target #
###############
.PHONY:all

all:$(OBJ) $(EXE)

$(OBJ):$(INCS) | libhydrogen lmdb queues certs wasm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

libhydrogen:
	@cd src/external/libhydrogen && make
lmdb:
	@cd src/external/lmdb && make
queues:
	@cd src/external/queues && make

certs: www/key.pem

www/key.pem:
	openssl req -new -newkey rsa:2048 -sha256 -days 365 -nodes -x509 -keyout www/key.pem -out www/cert.pem -subj '/CN=localhost/O=XX/C=XX'

j:
	make -j 12
wasm:
	@cd src/external/libhydrogen && emcc -O0 hydrogen.c -o hydrogen.html -DNDEBUG -s EXPORTED_FUNCTIONS='["_hydro_init","_hydro_sign_create","_hydro_pwhash_deterministic","_hydro_pwhash_keygen","_hydro_sign_keygen_deterministic"]' -s STANDALONE_WASM -s WASM_BIGINT=1 --no-entry -s MINIFY_HTML=0 -s ASSERTIONS=0 -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -s ALLOW_MEMORY_GROWTH=1 --no-entry
	@cp src/external/libhydrogen/hydrogen.js src/website/mainpage/stockminer/js
	@cp src/external/libhydrogen/hydrogen.wasm www/
	@sed -i 's/hydrogen.wasm/\/wasm/g' src/website/mainpage/stockminer/js/hydrogen.js

clean:
	make c2
	make c3
	make c4
	@cd src/external/lmdb && make clean
	@cd src/external/queues && make clean
	@cd src/external/libhydrogen && make clean
	rm -f $(EXE) *~ .gitignore~ www/hydrogen.wasm

c2:
	find ./src -name "*~"    -delete
	find ./src -name "*.swp" -delete

c3:
	find ./src -name "*.o"   -delete

c4:
	find ./src/python -name "*.pyc" -delete
	find ./src/python -name "__pycache__" -delete

r:
	./stockminer -r

.PHONY:data
data:
	tar -czf www/stockdata.tar.gz data/stocks/

linux:
	mkdir -p data/stocks/stockdb/{csv,wsj,yahoo}
	sudo apt-get -y install libtool gcc make cmake npm python3-pip zlib1g-dev python3-pbr libssl-dev emscripten libcurl4-openssl-dev libcurl4 libcurl3-gnutls libcurl4-openssl-dev libcap2-bin texinfo python3-venv wkhtmltopdf libsodium23 libsodium-dev
	sudo npm install -g html-minifier terser
	sudo pip3 install pandas pandas-datareader parsedatetime pbr yahoo-earnings-calendar yfinance urllib3 certbot xlsxwriter xlwt scipy holidays datedelta ta pandas_market_calendars
	sudo pip3 install --upgrade ta
	sudo ln -s /home/stockminer/stockminer /stockminer
	make -j 4

candles:
	cd build && git clone https://github.com/stoni/ta-lib/
	cd build/ta-lib && chmod +x autogen.sh && ./autogen.sh && ./configure && make && cp src/.libs/libta_lib.so ../../build

windows:
	sudo apt-get install gcc-multilib
	sudo ln -s /usr/bin/x86_64-w64-mingw32-windres /usr/bin/windres

