# Writes buildnumber.txt / buildnumber.h, but ONLY if the build number actually
# changed. An unconditional rewrite (as the old increment_build.cmd/.sh did)
# bumps the header timestamp every build, recompiling buildnumber.cpp and
# forcing a ~5-7s relink even when nothing changed.
#
# Invoked as: cmake -DSRC_DIR=... [-DBUILD_NUMBER=<git commit count>] -P write_build_number.cmake

set(BUILD_FILE "${SRC_DIR}/buildnumber.txt")
set(HEADER_FILE "${SRC_DIR}/buildnumber.h")

if (NOT DEFINED BUILD_NUMBER OR BUILD_NUMBER STREQUAL "")
    # No git commit count available: fall back to self-incrementing (old behavior).
    # This inherently changes every build, so the relink is unavoidable here.
    set(BUILD_NUMBER 0)
    if (EXISTS "${BUILD_FILE}")
        file(READ "${BUILD_FILE}" BUILD_NUMBER)
        string(STRIP "${BUILD_NUMBER}" BUILD_NUMBER)
    endif()
    math(EXPR BUILD_NUMBER "${BUILD_NUMBER} + 1")
endif()

set(HEADER_CONTENT "#pragma once
#define BUILD_NUMBER ${BUILD_NUMBER}

char *__cdecl getBuildNumber();
int getBuildNumberAsInt();
")

set(OLD_CONTENT "")
if (EXISTS "${HEADER_FILE}")
    file(READ "${HEADER_FILE}" OLD_CONTENT)
endif()

if (NOT OLD_CONTENT STREQUAL HEADER_CONTENT)
    file(WRITE "${BUILD_FILE}" "${BUILD_NUMBER}\n")
    file(WRITE "${HEADER_FILE}" "${HEADER_CONTENT}")
    message(STATUS "Updated build number to ${BUILD_NUMBER}")
endif()
