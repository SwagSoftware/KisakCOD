# Sanity check the CMakeLists.txt
if (NOT KISAK_PLATFORM STREQUAL "win32")
    message(FATAL_ERROR "KISAK_PLATFORM is incorrect for building win32.")
endif()

# Set the platform override directory
set(PLATFORM_OVERRIDE_DIR "${SRC_DIR}/_platform/win32")

# Apply override for specific directories
apply_platform_overrides(CLIENT_MP "${PLATFORM_OVERRIDE_DIR}")

# Handle the platform specific things FIRST
set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${BIN_NAME} )
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /O2 /Ot /MP /W3 /Zi ${MSVC_WARNING_DISABLES} /permissive-")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /Od /MP /W3 /Zi ${MSVC_WARNING_DISABLES} /permissive-")

#target_compile_options(${PROJECT_NAME} PRIVATE /we4700)
add_compile_options(/we4700 /intentionallbreakthisshit)