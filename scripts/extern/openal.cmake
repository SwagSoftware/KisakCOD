# extern/openal.cmake
#
# Fetches and builds OpenAL Soft from source using FetchContent, and exposes
# it as an importable target for the including project.
#
# Usage (from your top-level CMakeLists.txt):
#   include(extern/openal.cmake)
#   target_link_libraries(your_target PRIVATE OpenAL::OpenAL)
#
# Optional variables you can set BEFORE calling include():
#   OPENAL_SOFT_GIT_TAG   - tag/branch/commit to fetch (default: 1.24.2)
#   OPENAL_SOFT_GIT_REPO  - repo URL (default: official kcat/openal-soft repo)
 
include_guard(GLOBAL)
include(FetchContent)
 
# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
if(NOT DEFINED OPENAL_SOFT_GIT_REPO)
    set(OPENAL_SOFT_GIT_REPO "https://github.com/kcat/openal-soft.git")
endif()
 
if(NOT DEFINED OPENAL_SOFT_GIT_TAG)
    set(OPENAL_SOFT_GIT_TAG "1.24.2")
endif()
 
# ---------------------------------------------------------------------------
# Build options for openal-soft
# (Cache variables so they can still be overridden by the parent project.)
# ---------------------------------------------------------------------------
set(LIBTYPE "STATIC" CACHE STRING "Library type to build for OpenAL Soft" FORCE)
 
option(ALSOFT_UTILS        "Build OpenAL Soft utilities"      OFF)
option(ALSOFT_EXAMPLES     "Build OpenAL Soft examples"       OFF)
option(ALSOFT_TESTS        "Build OpenAL Soft test programs"  OFF)
option(ALSOFT_INSTALL      "Install OpenAL Soft"              OFF)
option(ALSOFT_INSTALL_CONFIG  "Install alsoft config files"   OFF)
option(ALSOFT_INSTALL_HRTF_DATA "Install HRTF data files"     OFF)
option(ALSOFT_INSTALL_AMBDEC_PRESETS "Install AmbDec presets" OFF)
option(ALSOFT_INSTALL_EXAMPLES "Install examples"             OFF)
option(ALSOFT_INSTALL_UTILS    "Install utilities"            OFF)
option(ALSOFT_EAX           "EAX Legacy"                      OFF)
 
# ---------------------------------------------------------------------------
# PortAudio backend wiring (optional)
#
# If your project already brought in PortAudio (e.g. via its own
# FetchContent_Declare(portaudio ...) + FetchContent_MakeAvailable(portaudio)
# BEFORE including this file), redirect openal-soft's find_package(PortAudio)
# to that in-tree target instead of letting it search the filesystem.
# ---------------------------------------------------------------------------
if(TARGET portaudio AND NOT DEFINED PORTAUDIO_LIBRARY)
    # Point FindPortAudio.cmake's expected cache vars at the real target.
    # Using the target NAME (not a file path) lets CMake resolve the actual
    # build artifact and dependency order automatically, even though the
    # library hasn't been built yet at configure time.
    set(PORTAUDIO_LIBRARY "portaudio" CACHE STRING
        "PortAudio library (resolved to in-tree FetchContent target)" FORCE)
 
    get_target_property(_openal_portaudio_include_dir portaudio SOURCE_DIR)
    set(PORTAUDIO_INCLUDE_DIR "${_openal_portaudio_include_dir}/include" CACHE PATH
        "PortAudio include dir (resolved to in-tree FetchContent target)" FORCE)
    unset(_openal_portaudio_include_dir)
 
    set(PORTAUDIO_FOUND TRUE CACHE BOOL "" FORCE)
    mark_as_advanced(PORTAUDIO_LIBRARY PORTAUDIO_INCLUDE_DIR PORTAUDIO_FOUND)
 
    message(STATUS "openal.cmake: wiring OpenAL Soft's PortAudio backend to in-tree 'portaudio' target")
endif()
 
option(ALSOFT_BACKEND_PORTAUDIO "Enable PortAudio backend in OpenAL Soft" ON)

# ---------------------------------------------------------------------------
# Fetch
# ---------------------------------------------------------------------------
FetchContent_Declare(
    openal-soft
    GIT_REPOSITORY ${OPENAL_SOFT_GIT_REPO}
    GIT_TAG        ${OPENAL_SOFT_GIT_TAG}
    GIT_SHALLOW    TRUE
)
 
FetchContent_GetProperties(openal-soft)
if(NOT openal-soft_POPULATED)
    FetchContent_MakeAvailable(openal-soft)
endif()
 
# ---------------------------------------------------------------------------
# Normalize target name -> OpenAL::OpenAL
#
# openal-soft's own CMakeLists creates a target named "OpenAL" (and, on
# older versions, may not export an ALIAS). We create a consistent
# namespaced alias here so consuming code can always use OpenAL::OpenAL
# regardless of the upstream version quirks.
# ---------------------------------------------------------------------------
if(TARGET OpenAL AND NOT TARGET OpenAL::OpenAL)
    add_library(OpenAL::OpenAL ALIAS OpenAL)
elseif(TARGET openal AND NOT TARGET OpenAL::OpenAL)
    add_library(OpenAL::OpenAL ALIAS openal)
endif()
 
if(NOT TARGET OpenAL::OpenAL)
    message(FATAL_ERROR
        "openal.cmake: could not locate the OpenAL target exported by "
        "openal-soft. The upstream target name may have changed; check "
        "${openal-soft_SOURCE_DIR}/CMakeLists.txt for the actual target name."
    )
endif()
