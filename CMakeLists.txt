cmake_minimum_required(VERSION 3.28)
project(nbody LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b1491-57a2-4f33-914b-610427c6a0d0")

file(GLOB_RECURSE CXX_SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE CXXM_SOURCE_FILES CONFIGURE_DEPENDS src/*.cppm)

add_executable(nbody)

target_sources(nbody
    PRIVATE
        ${CXX_SOURCE_FILES}
    PUBLIC
        FILE_SET CXX_MODULES FILES
            ${CXXM_SOURCE_FILES}
)

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

target_compile_features(nbody PUBLIC cxx_std_23)

# Switch compiler flags based on build mode
target_compile_options(nbody PRIVATE
    ${COMMON_FLAGS}
    $<$<CONFIG:Debug>:-O0 -g>
    $<$<CONFIG:Release>:-O3 -s>
    $<$<CONFIG:RelWithDebInfo>:-O3 -g -fno-omit-frame-pointer>
)

# Set output to bin folder
set_target_properties(nbody PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin CXX_MODULE_STD ON)