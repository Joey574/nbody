include(GoogleTest)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(nbody_unit_tests ${TEST_SOURCES})
target_link_libraries(nbody_unit_tests PRIVATE nbody_core GTest::gtest_main)

gtest_discover_tests(nbody_unit_tests DISCOVERY_MODE PRE_TEST)
