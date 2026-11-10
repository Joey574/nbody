SRC="./src/shaders"
OUT="./src/shaders/compiled"

slangc \
    -target spirv \
    -profile spirv_1_4 \
    -emit-spirv-directly \
    -fvk-use-entrypoint-name \
    -entry vertMain \
    -entry fragMain \
    -o "${OUT}/circles.spv" \
    "${SRC}/circles.slang"

slangc \
    -target spirv \
    -profile spirv_1_4 \
    -emit-spirv-directly \
    -fvk-use-entrypoint-name \
    -entry computeMain \
    -o "${OUT}/direct_sum.spv" \
    "${SRC}/direct_sum.slang"

slangc \
    -target spirv \
    -profile spirv_1_4 \
    -emit-spirv-directly \
    -fvk-use-entrypoint-name \
    -entry broadMain \
    -o "${OUT}/barnes_hut.spv" \
    "${SRC}/barnes_hut.slang"
