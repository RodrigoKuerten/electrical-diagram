add_executable(fiovivo WIN32 main.cpp)
fiovivo_configure_target(fiovivo)
target_link_libraries(fiovivo PRIVATE fiovivo_ui)
target_compile_definitions(
  fiovivo PRIVATE FIOVIVO_SOURCE_ASSET_DIR="${PROJECT_SOURCE_DIR}/assets"
)

add_custom_command(
  TARGET fiovivo POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${PROJECT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:fiovivo>/assets"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${PROJECT_SOURCE_DIR}/examples" "$<TARGET_FILE_DIR:fiovivo>/examples"
  COMMENT "Copying FioVivo assets and examples next to the executable"
)

if(FIOVIVO_QM_FILES)
  add_dependencies(fiovivo fiovivo_translations)
  add_custom_command(
    TARGET fiovivo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:fiovivo>/assets/i18n"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FIOVIVO_QM_FILES}
            "$<TARGET_FILE_DIR:fiovivo>/assets/i18n"
    COMMENT "Installing translations next to the executable"
  )
endif()

install(TARGETS fiovivo RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
