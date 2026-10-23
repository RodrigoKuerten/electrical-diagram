cmake_minimum_required(VERSION 3.21)

project(
  FioVivo
  VERSION 0.1.0
  DESCRIPTION "Educational electrical control diagram editor and simulator"
  HOMEPAGE_URL "https://github.com/RodrigoKuerten/electrical-diagram"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(FioVivoWarnings)

option(FIOVIVO_BUILD_TESTS "Build unit, integration and UI tests" ON)
option(FIOVIVO_BUILD_UI "Build the Qt Widgets application" ON)
option(FIOVIVO_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(FIOVIVO_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(FIOVIVO_COVERAGE "Instrument core and sim targets for coverage" OFF)

include(FioVivoDependencies)

if(FIOVIVO_WARNINGS_AS_ERRORS)
  set(FIOVIVO_WERROR ON)
endif()

if(FIOVIVO_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

if(FIOVIVO_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
endif()

if(FIOVIVO_BUILD_UI)
  find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Svg PrintSupport)
  find_package(Qt6 6.5 QUIET COMPONENTS LinguistTools)
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)
  set(CMAKE_AUTOUIC ON)
endif()

add_subdirectory(src/core)
add_subdirectory(src/sim)

if(FIOVIVO_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

include(GNUInstallDirs)
install(
  TARGETS fiovivo_core fiovivo_sim
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install(DIRECTORY assets/ DESTINATION "${CMAKE_INSTALL_DATADIR}/fiovivo/assets")
install(FILES docs/file-format.schema.json DESTINATION "${CMAKE_INSTALL_DATADIR}/fiovivo")
install(DIRECTORY examples/ DESTINATION "${CMAKE_INSTALL_DATADIR}/fiovivo/examples")
