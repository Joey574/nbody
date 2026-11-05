file(GLOB_RECURSE CORE_SOURCES
  CONFIGURE_DEPENDS
  *.cpp
  *.hpp
)
list (FILTER CORE_SOURCES EXCLUDE REGEX "main\\.cpp$")
add_library(nbody_core STATIC ${CORE_SOURCES})

include(FetchContent)

# yaml-cpp fetch
find_package(yaml-cpp 0.8 CONFIG QUIET)
if (NOT TARGET yaml-cpp::yaml-cpp)
  message(STATUS "yaml-cpp not found; fetching it")
  set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
  set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)

  FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 0.9.0
    GIT_SHALLOW TRUE
  )

  FetchContent_MakeAvailable(yaml-cpp)
endif()

# glfw3 fetch
find_package(glfw3 3.4 CONFIG QUIET)
if (NOT TARGET glfw)
  message(STATUS "GLFW not found; fetching it")

  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
  )

  FetchContent_MakeAvailable(glfw)
endif()

# openmp fetch
find_package(OpenMP REQUIRED COMPONENTS CXX)
find_package(Threads REQUIRED)

find_package(Vulkan REQUIRED)

target_include_directories(nbody_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nbody_core PRIVATE
    OpenMP::OpenMP_CXX
    Threads::Threads

    yaml-cpp::yaml-cpp
    Vulkan::Vulkan

    glfw
    dl
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
