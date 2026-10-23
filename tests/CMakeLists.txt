add_executable(
  fiovivo_core_tests
  unit/core/test_geometry.cpp
  unit/core/test_model.cpp
  unit/core/test_types.cpp
  unit/core/test_validation.cpp
)
target_link_libraries(fiovivo_core_tests PRIVATE fiovivo_core GTest::gtest_main)
target_include_directories(fiovivo_core_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
fiovivo_configure_target(fiovivo_core_tests)
include(GoogleTest)
gtest_discover_tests(fiovivo_core_tests)
