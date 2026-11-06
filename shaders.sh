slangc \
    -target spirv \
    -profile spirv_1_4 \
    -emit-spirv-directly \
    -fvk-use-entrypoint-name \
    -entry vertMain \
    -entry fragMain \
    -o ./src/shaders/circles.spv \
    ./src/shaders/circles.slang

slangc \
    -target spirv \
    -profile spirv_1_4 \
    -emit-spirv-directly \
    -fvk-use-entrypoint-name \
    -entry computeMain \
    -o ./src/shaders/direct_sum.spv \
    ./src/shaders/direct_sum.slang

slangc \
    -target spirv \
    -profile spirv_1_4 \
    -emit-spirv-directly \
    -fvk-use-entrypoint-name \
    -entry bhMain \
    -o ./src/shaders/barnes_hut.spv \
    ./src/shaders/barnes_hut.slang
