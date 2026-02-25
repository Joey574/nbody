#!/bin/bash
start_time=$(date +%s.%N)

export OMP_NUM_THREADS=$(nproc)
export OMP_PLACES=cores
export OMP_PROC_BIND=close

LIBS="-lvulkan -lGLEW -lglfw -lpthread -ldl -lX11 -lXxf86vm -lXrandr -lXi"
SOURCES=$(find src -name "*.cpp")
OUT="bin/nbody"

mkdir bin -p

DEFAULT_FLAGS="-march=native -mtune=native -fopenmp -fassociative-math -ffast-math -fmodules-ts"


# default to release mode build
FLAGS="-O3 -s"
MODE="RELEASE"

# check for build flags
if [[ "$1" == "-d" ]]; then
    FLAGS="-g -O0"
    MODE="DEBUG"
elif [[ "$1" == "-p" ]]; then
    FLAGS="-g -O3 -fno-omit-frame-pointer"
    MODE="PERF"
fi

# compile
g++ -std=c++23 $DEFAULT_FLAGS $FLAGS $SOURCES -o $OUT $LIBS

file_size=$(stat -c %s $OUT)
size_human=$(numfmt --to=iec --suffix=B "$file_size")
end_time=$(date +%s.%N)
elapsed=$(echo "$end_time - $start_time" | bc)

printf "Build (%s) completed in %.2f seconds (%s)\n" "$MODE" "$elapsed" "$size_human"
