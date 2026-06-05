macro(apply_platform_overrides FILE_LIST_VAR PLATFORM_DIR)
    set(_override_list "")

    foreach(_file ${${FILE_LIST_VAR}})
        # Extract the relative path after SRC_DIR
        file(RELATIVE_PATH _rel_path "${SRC_DIR}" "${_file}")

        # Check if a platform-specific version exists
        set(_platform_file "${PLATFORM_DIR}/${_rel_path}")

        if (EXISTS "${_platform_file}")
            message(STATUS "Platform (${KISAK_PLATFORM}) override: ${_rel_path}")
            list(APPEND _override_list "${_platform_file}")
        else()
            list(APPEND _override_list "${_file}")
        endif()
    endforeach()

    set(${FILE_LIST_VAR} ${_override_list})
endmacro()