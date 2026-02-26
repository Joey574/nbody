#!/bin/bash
start_time=$(date +%s.%N)

# check for build flags
MODE="Release"
if [[ "$1" == "-d" ]]; then
    MODE="Debug"
elif [[ "$1" == "-p" ]]; then
    MODE="RelWithDebInfo"
fi

mkdir -p build
mkdir -p bin

cmake -S . -B build -DCMAKE_BUILD_TYPE="$MODE" -G Ninja
cmake --build build -j

file_size=$(stat -c %s ./bin/nbody)
size_human=$(numfmt --to=iec --suffix=B "$file_size")
end_time=$(date +%s.%N)
elapsed=$(echo "$end_time - $start_time" | bc)

printf "Build (%s) completed in %.2f seconds (%s)\n" "$MODE" "$elapsed" "$size_human"
