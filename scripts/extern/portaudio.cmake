# PortAudio.cmake
# Fetches PortAudio from Git and makes it available as a CMake target.
#
# Usage:
#   include(PortAudio)
#
# Targets:
#   portaudio        - shared library (or static if PA_BUILD_SHARED=OFF)
#   portaudio_static - always static
#
# Options (set before including this file to override defaults):
#   PORTAUDIO_GIT_TAG      - Git tag/branch/commit to fetch (default: v19.7.0)
#   PA_BUILD_SHARED        - Build shared library (default: OFF)
#   PA_BUILD_TESTS         - Build PortAudio tests (default: OFF)
#   PA_BUILD_EXAMPLES      - Build PortAudio examples (default: OFF)

cmake_minimum_required(VERSION 3.14)
include(FetchContent)

# --- Configurable options ---------------------------------------------------

if(NOT DEFINED PORTAUDIO_GIT_TAG)
    set(PORTAUDIO_GIT_TAG "master")
endif()

if(NOT DEFINED PA_BUILD_SHARED)
    set(PA_BUILD_SHARED OFF CACHE BOOL "Build PortAudio shared library" FORCE)
endif()

if(NOT DEFINED PA_BUILD_TESTS)
    set(PA_BUILD_TESTS OFF CACHE BOOL "Build PortAudio tests" FORCE)
endif()

if(NOT DEFINED PA_BUILD_EXAMPLES)
    set(PA_BUILD_EXAMPLES OFF CACHE BOOL "Build PortAudio examples" FORCE)
endif()

if(NOT DEFINED PA_ENABLE_DEBUG_OUTPUT)
    set(PA_ENABLE_DEBUG_OUTPUT ON CACHE BOOL "Enable PortAudio debug output" FORCE)
endif()

# --- Fetch -------------------------------------------------------------------

FetchContent_Declare(
    portaudio
    GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
    GIT_TAG        ${PORTAUDIO_GIT_TAG}
    GIT_SHALLOW    TRUE
)

# Set this BEFORE FetchContent_MakeAvailable(portaudio)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

FetchContent_MakeAvailable(portaudio)

FetchContent_GetProperties(portaudio SOURCE_DIR PORTAUDIO_INCLUDE_DIR)
set(PORTAUDIO_INCLUDE_DIR "${PORTAUDIO_INCLUDE_DIR}/include")

# --- Friendly status message -------------------------------------------------

message(STATUS "PortAudio: using tag '${PORTAUDIO_GIT_TAG}'")