# [POST_BUILD] Copy over MILES dependency
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
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