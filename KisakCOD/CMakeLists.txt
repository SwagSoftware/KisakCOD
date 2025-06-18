cmake_minimum_required(VERSION 3.16)
project(KisakCOD)

set(BIN_NAME "KisakCOD")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_GENERATOR_PLATFORM "WIN32")

include(cmake_files.cmake)

target_compile_definitions(KisakCOD PUBLIC WIN32)
target_compile_definitions(KisakCOD PUBLIC _DEBUG)
target_compile_definitions(KisakCOD PUBLIC _CONSOLE)
target_compile_definitions(KisakCOD PUBLIC _CRT_SECURE_NO_WARNINGS)
target_compile_definitions(KisakCOD PUBLIC _MBCS)

target_include_directories(KisakCOD PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(KisakCOD PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/libwww/src")

set(DXSDK_DIR $ENV{DXSDK_DIR})
target_include_directories(KisakCOD PUBLIC "${DXSDK_DIR}/Include")

target_link_directories(KisakCOD PUBLIC "${DXSDK_DIR}/Lib/x86")
target_link_directories(KisakCOD PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_directories(KisakCOD PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/msslib")

set(MSVC_WARNING_DISABLES
 "/wd\"4065\""
 ) #Switch statement with no cases (Used Legitimately)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /O2 /MP /W3 ${MSVC_WARNING_DISABLES} /permissive-")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /Od /MP /W3 ${MSVC_WARNING_DISABLES} /permissive-")


set_property(TARGET KisakCOD PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# fixme: copied from sln, check whats not needed
target_link_libraries(KisakCOD PUBLIC mss32.lib dsound.lib ddraw.lib d3dx9d.lib d3d9.lib ws2_32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib "${CMAKE_CURRENT_SOURCE_DIR}/src/binklib/binkw32.lib" dxguid.lib)
target_link_options(KisakCOD PRIVATE /machine:x86)
set_target_properties(KisakCOD PROPERTIES WIN32_EXECUTABLE TRUE)