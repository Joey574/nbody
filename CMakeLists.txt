cmake_minimum_required(VERSION 3.28)
project(nbody LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(nbody)
file(GLOB_RECURSE CXX_SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)
target_sources(nbody PRIVATE ${CXX_SOURCE_FILES})

find_package(OpenMP REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

target_link_libraries(nbody PRIVATE 
    OpenMP::OpenMP_CXX 
    Vulkan::Vulkan
    glfw 
    dl 
    pthread
)

# Flags common to all build modes
set(COMMON_FLAGS
    -march=native
    -mtune=native
    -fassociative-math
    -ffast-math
)

# Switch compiler flags based on build mode
target_compile_options(nbody PRIVATE
    ${COMMON_FLAGS}
    $<$<CONFIG:Debug>:-O0 -g>
    $<$<CONFIG:Release>:-O3 -s>
    $<$<CONFIG:RelWithDebInfo>:-O3 -g -fno-omit-frame-pointer>
)

# Set output to bin folder
set_target_properties(nbody PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)