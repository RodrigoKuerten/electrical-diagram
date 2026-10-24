add_executable(
  fiovivo_core_tests
  unit/core/test_geometry.cpp
  unit/core/test_model.cpp
  unit/core/test_result.cpp
  unit/core/test_types.cpp
  unit/core/test_validation.cpp
)
target_link_libraries(fiovivo_core_tests PRIVATE fiovivo_core GTest::gtest_main)
target_include_directories(fiovivo_core_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
fiovivo_configure_target(fiovivo_core_tests)
include(GoogleTest)
gtest_discover_tests(fiovivo_core_tests)

add_executable(
  fiovivo_sim_tests
  unit/sim/test_simulator.cpp
  unit/sim/test_timers.cpp
  unit/sim/test_performance.cpp
)
target_link_libraries(fiovivo_sim_tests PRIVATE fiovivo_sim fiovivo_core GTest::gtest_main)
target_include_directories(fiovivo_sim_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
fiovivo_configure_target(fiovivo_sim_tests)
gtest_discover_tests(fiovivo_sim_tests)

add_executable(
  fiovivo_io_tests
  unit/io/test_json_io.cpp
  unit/io/test_bom.cpp
)
target_link_libraries(fiovivo_io_tests PRIVATE fiovivo_io fiovivo_core GTest::gtest_main)
target_include_directories(fiovivo_io_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
fiovivo_configure_target(fiovivo_io_tests)
gtest_discover_tests(fiovivo_io_tests)

add_executable(fiovivo_symbol_tests unit/library/test_symbols.cpp)
target_link_libraries(fiovivo_symbol_tests PRIVATE fiovivo_symbols GTest::gtest_main)
target_include_directories(fiovivo_symbol_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(
  fiovivo_symbol_tests PRIVATE FIOVIVO_ASSET_DIR="${PROJECT_SOURCE_DIR}/assets"
)
fiovivo_configure_target(fiovivo_symbol_tests)
gtest_discover_tests(fiovivo_symbol_tests)

if(FIOVIVO_BUILD_UI)
  find_package(Qt6 6.5 QUIET COMPONENTS Test)
  if(Qt6Test_FOUND)
    add_executable(fiovivo_ui_tests ui/test_canvas.cpp)
    target_link_libraries(
      fiovivo_ui_tests
      PRIVATE fiovivo_ui Qt6::Test Qt6::Widgets GTest::gtest_main
    )
    target_include_directories(fiovivo_ui_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_definitions(
      fiovivo_ui_tests PRIVATE FIOVIVO_ASSET_DIR="${PROJECT_SOURCE_DIR}/assets"
    )
    fiovivo_configure_target(fiovivo_ui_tests)
    add_dependencies(fiovivo_ui_tests fiovivo)
    gtest_discover_tests(fiovivo_ui_tests)

    add_executable(fiovivo_theme_tests ui/test_theme.cpp)
    target_link_libraries(
      fiovivo_theme_tests
      PRIVATE fiovivo_ui Qt6::Test Qt6::Widgets GTest::gtest_main
    )
    target_include_directories(fiovivo_theme_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_definitions(
      fiovivo_theme_tests
      PRIVATE FIOVIVO_ASSET_DIR="${PROJECT_SOURCE_DIR}/assets"
              FIOVIVO_GOLDEN_DIR="${PROJECT_SOURCE_DIR}/tests/golden"
              FIOVIVO_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
    )
    fiovivo_configure_target(fiovivo_theme_tests)
    add_dependencies(fiovivo_theme_tests fiovivo)
    gtest_discover_tests(fiovivo_theme_tests)
  else()
    message(STATUS "Qt6Test not found; UI tests will not be built")
  endif()
endif()

add_executable(fiovivo_integration_tests integration/test_examples.cpp)
target_link_libraries(
  fiovivo_integration_tests PRIVATE fiovivo_core fiovivo_sim fiovivo_io GTest::gtest_main
)
target_include_directories(fiovivo_integration_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(
  fiovivo_integration_tests
  PRIVATE FIOVIVO_EXAMPLES_DIR="${PROJECT_SOURCE_DIR}/examples"
          FIOVIVO_TEST_DATA_DIR="${PROJECT_SOURCE_DIR}/tests/data"
)
fiovivo_configure_target(fiovivo_integration_tests)
gtest_discover_tests(fiovivo_integration_tests)


