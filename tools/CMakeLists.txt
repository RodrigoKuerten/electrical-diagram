add_executable(fiovivo_make_examples make_examples.cpp)
fiovivo_configure_target(fiovivo_make_examples)
target_link_libraries(fiovivo_make_examples PRIVATE fiovivo_symbols fiovivo_io fiovivo_core)
target_compile_definitions(
  fiovivo_make_examples
  PRIVATE FIOVIVO_ASSET_DIR="${PROJECT_SOURCE_DIR}/assets"
          FIOVIVO_EXAMPLES_DIR="${PROJECT_SOURCE_DIR}/examples"
)

if(FIOVIVO_BUILD_UI)
  add_executable(fiovivo_theme_screenshots theme_screenshots.cpp)
  fiovivo_configure_target(fiovivo_theme_screenshots)
  target_link_libraries(fiovivo_theme_screenshots PRIVATE fiovivo_ui)
  target_compile_definitions(
    fiovivo_theme_screenshots
    PRIVATE FIOVIVO_ASSET_DIR="${PROJECT_SOURCE_DIR}/assets"
            FIOVIVO_EXAMPLES_DIR="${PROJECT_SOURCE_DIR}/examples"
  )
endif()
