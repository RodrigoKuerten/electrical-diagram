add_library(
  fiovivo_core
  STATIC
  geometry.cpp
  ids.cpp
  model.cpp
  types.cpp
  validation.cpp
)

fiovivo_configure_target(fiovivo_core)

if(FIOVIVO_COVERAGE)
  target_compile_options(fiovivo_core PRIVATE --coverage -O0 -g)
endif()
