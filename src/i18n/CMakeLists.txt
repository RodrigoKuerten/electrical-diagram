set(FIOVIVO_TS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/fiovivo_pt_BR.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/fiovivo_en.ts"
)

find_program(FIOVIVO_LRELEASE_EXECUTABLE NAMES lrelease-qt6 lrelease lrelease-qt5)
find_program(FIOVIVO_LUPDATE_EXECUTABLE NAMES lupdate-qt6 lupdate lupdate-qt5)

if(FIOVIVO_LRELEASE_EXECUTABLE)
  set(FIOVIVO_QM_FILES)
  foreach(ts_file IN LISTS FIOVIVO_TS_FILES)
    get_filename_component(ts_name "${ts_file}" NAME_WE)
    set(qm_file "${CMAKE_BINARY_DIR}/i18n/${ts_name}.qm")
    add_custom_command(
      OUTPUT "${qm_file}"
      COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/i18n"
      COMMAND "${FIOVIVO_LRELEASE_EXECUTABLE}" "${ts_file}" -qm "${qm_file}"
      DEPENDS "${ts_file}"
      COMMENT "Compiling translation ${ts_name}"
      VERBATIM
    )
    list(APPEND FIOVIVO_QM_FILES "${qm_file}")
  endforeach()

  add_custom_target(fiovivo_translations ALL DEPENDS ${FIOVIVO_QM_FILES})
  set(FIOVIVO_QM_FILES "${FIOVIVO_QM_FILES}" PARENT_SCOPE)

  install(FILES ${FIOVIVO_QM_FILES} DESTINATION "${CMAKE_INSTALL_DATADIR}/fiovivo/assets/i18n")
else()
  message(WARNING "lrelease was not found; the UI will fall back to English source strings")
endif()

if(FIOVIVO_LUPDATE_EXECUTABLE)
  add_custom_target(
    fiovivo_update_translations
    COMMAND "${FIOVIVO_LUPDATE_EXECUTABLE}" -no-obsolete -recursive
            "${PROJECT_SOURCE_DIR}/src" -ts ${FIOVIVO_TS_FILES}
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Updating Qt Linguist translation files"
    VERBATIM
  )
endif()

install(FILES ${FIOVIVO_TS_FILES} DESTINATION "${CMAKE_INSTALL_DATADIR}/fiovivo/i18n")
