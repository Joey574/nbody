include(GoogleTest)

file(GLOB_RECURSE CORE_SOURCES
  CONFIGURE_DEPENDS
  *.cpp
)

add_executable(nbody_unit_tests ${CORE_SOURCES})
target_link_libraries(
  nbody_unit_tests
  PRIVATE
  nbody_core
  GTest::gtest_main
)

gtest_discover_tests(nbody_unit_tests)
