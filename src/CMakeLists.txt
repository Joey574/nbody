include(FetchContent)

set(YAML_CPP_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS   OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS        OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS       OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES    OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL           OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG 0.9.0
  GIT_SHALLOW TRUE
  FIND_PACKAGE_ARGS 0.8 CONFIG
)

FetchContent_Declare(
  glfw3
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
  GIT_SHALLOW TRUE
  FIND_PACKAGE_ARGS 3.4 CONFIG
)
FetchContent_MakeAvailable(yaml-cpp glfw3)

find_package(OpenMP REQUIRED COMPONENTS CXX)
find_package(Threads REQUIRED)
find_package(Vulkan REQUIRED)

add_library(nbody_options INTERFACE)

if (MSVC)
  target_compile_options(nbody_options INTERFACE
    /utf-8 /permissive- /Zc:__cplusplus /fp:fast /arch:AVX2)

  target_compile_definitions(nbody_options INTERFACE
    NOMINMAX WIN32_LEAN_AND_MEAN _USE_MATH_DEFINES _CRT_SECURE_NO_WARNINGS)
else()
  target_compile_options(nbody_options INTERFACE
    -ffast-math -march=native -mtune=native)

  target_compile_options(nbody_options INTERFACE
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Release>:-Ofast>
    $<$<CONFIG:RelWithDebInfo>:-O3>
    $<$<CONFIG:RelWithDebInfo>:-fno-omit-frame-pointer>)
endif()
target_compile_definitions(nbody_options INTERFACE $<$<CONFIG:Debug>:DEBUG>)

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
list(REMOVE_ITEM CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_library(nbody_core STATIC ${CORE_SOURCES})
target_include_directories(nbody_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(nbody_core PRIVATE
    nbody_options
    OpenMP::OpenMP_CXX
    Threads::Threads
    yaml-cpp::yaml-cpp
    Vulkan::Vulkan
    glfw
    ${CMAKE_DL_FILES}
)

add_executable(nbody main.cpp)
target_link_libraries(nbody PRIVATE nbody_core)
if(NOT MSVC)
  target_link_options(nbody PRIVATE $<$<CONFIG:Release>:-s>)
endif()
