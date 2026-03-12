#!/bin/bash
start_time=$(date +%s.%N)

cd ./src/shaders/
for file in *; do
    name="${file%.*}"
    slangc -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o "../../bin/$name.spv" $file
done
cd ../../

end_time=$(date +%s.%N)
elapsed=$(echo "$end_time - $start_time" | bc)

printf -- "-- Shaders compiled (%.1fs)\n" "$elapsed" 