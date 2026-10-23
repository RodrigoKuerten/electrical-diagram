add_library(fiovivo_symbols STATIC symbol_definition.cpp)
fiovivo_configure_target(fiovivo_symbols)
target_link_libraries(fiovivo_symbols PUBLIC fiovivo_core nlohmann_json::nlohmann_json)

if(FIOVIVO_BUILD_UI)
  add_library(fiovivo_library STATIC svg_renderer.cpp)
  fiovivo_configure_target(fiovivo_library)
  target_link_libraries(
    fiovivo_library
    PUBLIC fiovivo_symbols Qt6::Core Qt6::Gui Qt6::Svg Qt6::Widgets
  )
endif()
