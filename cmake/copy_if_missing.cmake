if (NOT DEFINED SOURCE OR NOT DEFINED DEST)
    message(FATAL_ERROR "SOURCE and DEST are required")
endif()

if (NOT EXISTS "${DEST}")
    get_filename_component(DEST_DIR "${DEST}" DIRECTORY)
    file(MAKE_DIRECTORY "${DEST_DIR}")
    file(COPY_FILE "${SOURCE}" "${DEST}")
endif()
