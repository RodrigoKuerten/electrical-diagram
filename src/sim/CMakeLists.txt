add_library(fiovivo_sim STATIC simulator.cpp)

fiovivo_configure_target(fiovivo_sim)
target_link_libraries(fiovivo_sim PUBLIC fiovivo_core)
