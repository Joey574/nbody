cmake_minimum_required(VERSION 3.28)
project(nbody LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(nbody)

file(GLOB_RECURSE CXX_SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE CXXM_SOURCE_FILES CONFIGURE_DEPENDS src/*.cppm)

target_sources(nbody
    PRIVATE
        CXX_SOURCE_FILES
    PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
            CXXM_SOURCE_FILES
)

set("FLAGS -march=native -mtune=native -fopenmp -fassociative-math -ffast-math -fmodules-ts -O3 -s")
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set("FLAGS -march=native -mtune=native -fopenmp -fassociative-math -ffast-math -fmodules-ts -g -O0")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Perf")
    set("FLAGS -march=native -mtune=native -fopenmp -fassociative-math -ffast-math -fmodules-ts -g -O3 -fno-omit-frame-pointer")
endif()

target_compile_options(nbody PRIVATE ${FLAGS})