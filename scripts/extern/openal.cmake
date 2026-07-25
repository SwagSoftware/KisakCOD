include(FetchContent)

##### OpenAL Soft #####
# Build as a static lib so it links like the rest of our dependencies
# (no OpenAL32.dll to stage via post_build.cmake).
set(LIBTYPE "STATIC")

# Match the rest of the project's /MTd (static debug CRT). Setting this as a directory-scoped
# default (rather than only via set_property on the OpenAL target below) is required: OpenAL-
# Soft creates several internal sub-targets (alsoft.fmt, alsoft.common, alsoft.build_version,
# etc.) that link into OpenAL32.lib, and those don't inherit a property set on OpenAL alone -
# they'd stay on MSVC's default /MDd, causing CRT-mismatch link errors (LNK2005/LNK2038/
# LNK2019) against the rest of the project.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# The VS-generator + FetchContent combination doesn't reliably wire up module dependency
# scanning across separate vcxprojs; force plain headers instead of C++20 module partitions.
option ( ALSOFT_ENABLE_MODULES "" OFF )

# ALSOFT_ENABLE_MODULES alone isn't enough: with CMAKE_CXX_STANDARD 20 + the VS generator,
# CMake enables its own "Scan Source for Module Dependencies" MSBuild step by default for
# every target in scope, independent of OpenAL's own option above. That scan runs a separate
# preprocessing pass that chokes on cpu_caps.cpp's #warning too, and is pure overhead for a
# target that isn't using C++ modules. Turn it off for every target FetchContent creates below.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

option ( ALSOFT_UTILS "" OFF )
option ( ALSOFT_EXAMPLES "" OFF )
option ( ALSOFT_TESTS "" OFF )
option ( ALSOFT_INSTALL "" OFF )
option ( ALSOFT_INSTALL_CONFIG "" OFF )
option ( ALSOFT_INSTALL_HRTF_DATA "" OFF )
option ( ALSOFT_INSTALL_AMBDEC_PRESETS "" OFF )
option ( ALSOFT_INSTALL_EXAMPLES "" OFF )
option ( ALSOFT_INSTALL_UTILS "" OFF )

FetchContent_Declare (
	openal
	GIT_REPOSITORY https://github.com/kcat/openal-soft.git
	GIT_TAG 1.25.2
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable ( openal )

set_property(TARGET OpenAL PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# OpenAL-Soft's core/cpu_caps.cpp falls back to a #warning directive when it can't detect
# CPUID intrinsics at configure time. MSVC's legacy preprocessor (the default here, since
# the project doesn't opt into /Zc:preprocessor project-wide) treats #warning as an unknown
# directive and hard-errors (C1021) instead of just warning. Scope the conforming
# preprocessor to just this target rather than changing flags for the whole codebase.
if (MSVC)
    target_compile_options(OpenAL PRIVATE /Zc:preprocessor)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${openal_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PUBLIC OpenAL::OpenAL)
#################
