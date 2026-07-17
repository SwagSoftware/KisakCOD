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
add_compile_options(/we4700)

# PortAudio Configuration

# Windows backends — enable WASAPI only (modern low-latency API)
set(PA_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(PA_USE_DS            OFF CACHE BOOL "" FORCE)  # DirectSound (legacy)
set(PA_USE_WMME          OFF CACHE BOOL "" FORCE)  # Windows Multimedia (very old)
set(PA_USE_WASAPI        ON  CACHE BOOL "" FORCE)  # WASAPI (recommended for games)
set(PA_USE_ASIO          OFF CACHE BOOL "" FORCE)  # ASIO (needs separate SDK)
set(PA_USE_WDMKS         OFF CACHE BOOL "" FORCE)  # WDM kernel streaming

# Match the project-wide static runtime (/MT, /MTd)
#set_property(TARGET PortAudio PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Expose source dir so FindPortAudio.cmake override can use it
#set(PORTAUDIO_SOURCE_DIR "${portaudio_SOURCE_DIR}" CACHE INTERNAL "")
