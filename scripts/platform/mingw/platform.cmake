#
if (NOT KISAK_PLATFORM STREQUAL "mingw")
    message(FATAL_ERROR "KISAK_PLATFORM is incorrect for building mingw.")
endif()

set(PLATFORM_OVERRIDE_DIR "${SRC_DIR}/_platform/win32")
apply_platform_overrides(CLIENT_MP "${PLATFORM_OVERRIDE_DIR}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wno-comment)
add_compile_options(-Werror=uninitialized)
add_compile_options(-Wno-function-effects)

add_compile_options(-Wno-c++11-narrowing)
add_compile_options(-Wno-int-to-pointer-cast -Wno-pointer-to-int-cast)

add_link_options(-m32 -Wl,--subsystem,windows)
