# [POST_BUILD] Copy over MILES dependency.
# copy_directory_if_different, NOT copy_directory: every target (mp/sp/dedi) runs this
# into the same bin/<config> dir, so unconditional copies race each other during parallel
# builds (and fail outright if the game is running with the DLLs loaded). Skipping
# unchanged files avoids the write entirely.
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${DEPS_DIR}/msslib/dlls
        ${BIN_DIR}/${CMAKE_BUILD_TYPE}
        COMMENT "COPYING MILES DEPENDENCIES"
)
# [POST_BUILD] Copy over steam depdendency
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DEPS_DIR}/steamsdk/steam_api.dll
        ${BIN_DIR}/${CMAKE_BUILD_TYPE}
        COMMENT "COPYING STEAM DEPENDENCIES"
)