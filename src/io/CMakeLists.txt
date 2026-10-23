file(READ "${PROJECT_SOURCE_DIR}/docs/file-format.schema.json" FIOVIVO_SCHEMA_JSON)
set(FIOVIVO_SCHEMA_JSON "${FIOVIVO_SCHEMA_JSON}")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated/io")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/schema_embedded.hpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/generated/io/schema_embedded.hpp"
  @ONLY
)
set_property(
  DIRECTORY
  APPEND
  PROPERTY CMAKE_CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/docs/file-format.schema.json"
)

add_library(
  fiovivo_io
  STATIC
  bom.cpp
  json_io.cpp
  schema_validator.cpp
)

fiovivo_configure_target(fiovivo_io)
target_include_directories(fiovivo_io PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(fiovivo_io PUBLIC fiovivo_core nlohmann_json::nlohmann_json)
