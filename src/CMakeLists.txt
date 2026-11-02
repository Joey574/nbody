file(GLOB_RECURSE CORE_SOURCES
  CONFIGURE_DEPENDS
  *.cpp
  *.hpp
)
list (FILTER CORE_SOURCES EXCLUDE REGEX "main\\.cpp$")


add_library(nbody_core STATIC ${CORE_SOURCES})

find_package(OpenMP REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(yaml-cpp REQUIRED)

target_include_directories(nbody_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nbody_core PRIVATE
    OpenMP::OpenMP_CXX
    yaml-cpp::yaml-cpp
    Vulkan::Vulkan
    glfw
    dl
    pthread
)
add_executable(nbody main.cpp)
target_link_libraries(nbody PRIVATE nbody_core)

set(COMMON_FLAGS
    -march=native
    -mtune=native
    -fassociative-math
    -ffast-math
)

target_compile_options(nbody PRIVATE
    ${COMMON_FLAGS}
    $<$<CONFIG:Debug>:-O0 -g -DDEBUG>
    $<$<CONFIG:Release>:-Ofast -s>
    $<$<CONFIG:RelWithDebInfo>:-O3 -g -fno-omit-frame-pointer>
)
