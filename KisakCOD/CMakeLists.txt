cmake_minimum_required(VERSION 3.16)

if (DEDICATED)
  project(KC-Dedicated)
else()
  project(KisakCOD)
endif()

set(CMAKE_CXX_STANDARD 20)

if (WIN32)
  set(CMAKE_GENERATOR_PLATFORM "WIN32")
endif()

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../${CMAKE_BUILD_TYPE}/")

if (DEDICATED)
  message("===== KisakCOD Dedicated =====")
  set(BIN_NAME "KisakCOD-Dedi")
  #include(cmake_files_dedicated.cmake)
  #target_compile_definitions(${BIN_NAME} PUBLIC DEDICATED) # LWSS: true headless DEDICATED is disabled for now. It doesn't fully work.
  include(cmake_files.cmake)
  target_compile_definitions(${BIN_NAME} PUBLIC KISAK_DEDICATED) # this is a cope DEBUG mode that basically just does `+set dedicated 2`
else()
  message("----- KisakCOD Client -----")
  set(BIN_NAME "KisakCOD")
  include(cmake_files.cmake)
endif()




set_target_properties(${BIN_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../$<CONFIG>"
)

target_compile_definitions(${BIN_NAME} PUBLIC WIN32)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(${BIN_NAME} PUBLIC _DEBUG)
endif()

target_compile_definitions(${BIN_NAME} PUBLIC _CONSOLE)
target_compile_definitions(${BIN_NAME} PUBLIC _MBCS)

target_include_directories(${BIN_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(DXSDK_DIR $ENV{DXSDK_DIR})
target_include_directories(${BIN_NAME} PUBLIC "${DXSDK_DIR}/Include")

target_link_directories(${BIN_NAME} PUBLIC "${DXSDK_DIR}/Lib/x86")
target_link_directories(${BIN_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_directories(${BIN_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/msslib")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /O2 /Ot /MP /W3 /Zi ${MSVC_WARNING_DISABLES} /permissive-")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /Od /MP /W3 /Zi ${MSVC_WARNING_DISABLES} /permissive-")

set_property(TARGET ${BIN_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# fixme: copied from sln, check whats not needed
target_link_libraries(${BIN_NAME} PUBLIC mss32.lib dsound.lib ddraw.lib d3dx9d.lib d3d9.lib ws2_32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib "${CMAKE_CURRENT_SOURCE_DIR}/src/binklib/binkw32.lib" dxguid.lib)
target_link_options(${BIN_NAME} PRIVATE /machine:x86)
set_target_properties(${BIN_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${BIN_NAME} )

#Enable PDB for "Release" Build. (There is also RelWithDebInfo, but it has different settings)
target_link_options(${BIN_NAME} PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
target_link_options(${BIN_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
target_link_options(${BIN_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")
