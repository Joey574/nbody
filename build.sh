#!/bin/bash
start_time=$(date +%s.%N)

export OMP_NUM_THREADS=$(nproc)
export OMP_PLACES=cores
export OMP_PROC_BIND=close

LIBS="-lvulkan -lGLEW -lglfw -lpthread -ldl -lX11 -lXxf86vm -lXrandr -lXi"
SOURCES=$(find src -name "*.cpp")
OUT="bin/nbody"

FLAGS="-march=native -mtune=native -O3 -ffast-math -fopenmp"
MODE="RELEASE"

# check for build flags
if [[ "$1" == "-d" ]]; then
    FLAGS="-march=native -mtune=native -g -O0 -ffast-math -fopenmp"
    MODE="DEBUG"
elif [[ "$1" == "-p" ]]; then
    FLAGS="-march=native -mtune=native -g -O3 -ffast-math -fno-omit-frame-pointer -fopenmp"
    MODE="PERF"
fi
ccache g++ $FLAGS $SOURCES -o $OUT $LIBS

file_size=$(stat -c %s $OUT)
size_human=$(numfmt --to=iec --suffix=B "$file_size")
end_time=$(date +%s.%N)
elapsed=$(echo "$end_time - $start_time" | bc)

printf "Build (%s) completed in %.2f seconds (%s)\n" "$MODE" "$elapsed" "$size_human"
