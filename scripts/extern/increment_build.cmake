# Get the current git commit count and save it to GIT_COMMIT_COUNT
execute_process(
  COMMAND git rev-list --count HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_COUNT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Add a custom target to update the build number. The -P script only rewrites
# buildnumber.h when the number actually changes, so an unchanged build number
# no longer recompiles buildnumber.cpp / forces a relink every build.
add_custom_target(
  update_build_number
  COMMAND ${CMAKE_COMMAND} -DSRC_DIR=${SRC_DIR} -DBUILD_NUMBER=${GIT_COMMIT_COUNT} -P ${SCRIPTS_DIR}/extern/write_build_number.cmake
  COMMENT "Checking build number..."
)