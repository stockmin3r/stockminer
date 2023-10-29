# eigen3
git clone https://github.com/libigl/eigen
mkdir build
cd build
emcmake cmake -DCMAKE_EXE_LINKER_FLAGS="-sEXPORTED_RUNTIME_METHODS=ccall,cwrap -sEXPORTED_FUNCTIONS=_main" .
make install


