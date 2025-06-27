cmake_minimum_required(VERSION 3.16)
include(FetchContent)

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

# === Increment Build ===

if (WIN32)
  set(SCRIPT_EXT .cmd)
else()
  set(SCRIPT_EXT .sh)
endif()

# Get the current git commit count and save it to GIT_COMMIT_COUNT
execute_process(
  COMMAND git rev-list --count HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_COUNT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_custom_target(
  update_build_number
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/increment_build${SCRIPT_EXT} ${SRC_DIR} ${GIT_COMMIT_COUNT}
  COMMENT "Running build number script..."
)

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

# We want to update the build number before building
add_dependencies(${BIN_NAME} update_build_number)


set_target_properties(${BIN_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../$<CONFIG>"
)

target_compile_definitions(${BIN_NAME} PUBLIC WIN32)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(${BIN_NAME} PUBLIC _DEBUG)
endif()

##### Tracy #####
option ( TRACY_ENABLE "" ON )
option ( TRACY_ON_DEMAND "" ON )
option ( TRACY_ONLY_LOCALHOST "" ON )
#option ( TRACY_FIBERS "" ON )


target_compile_definitions(${BIN_NAME} PUBLIC TRACY_ENABLE)
target_compile_definitions(${BIN_NAME} PUBLIC TRACY_ON_DEMAND)

FetchContent_Declare (
	tracy
	GIT_REPOSITORY https://github.com/wolfpld/tracy.git
	GIT_TAG v0.12.2
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable ( tracy )

message( ${CMAKE_BINARY_DIR}/_deps/tracy-src/public)
target_include_directories(${BIN_NAME} PUBLIC ${CMAKE_BINARY_DIR}/_deps/tracy-src/public)
target_link_libraries(${BIN_NAME} PUBLIC TracyClient)
#################

target_compile_definitions(${BIN_NAME} PUBLIC _CONSOLE)
target_compile_definitions(${BIN_NAME} PUBLIC _MBCS)

target_include_directories(${BIN_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(DXSDK_DIR $ENV{DXSDK_DIR})
target_include_directories(${BIN_NAME} PUBLIC "${DXSDK_DIR}/Include")

target_link_directories(${BIN_NAME} PUBLIC "${DXSDK_DIR}/Lib/x86")
target_link_directories(${BIN_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_directories(${BIN_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/msslib")
target_link_directories(${BIN_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/steamsdk")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /O2 /Ot /MP /W3 /Zi ${MSVC_WARNING_DISABLES} /permissive-")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /Od /MP /W3 /Zi ${MSVC_WARNING_DISABLES} /permissive-")

set_property(TARGET ${BIN_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# fixme: copied from sln, check whats not needed
target_link_libraries(${BIN_NAME} PUBLIC mss32.lib dsound.lib ddraw.lib d3dx9d.lib d3d9.lib ws2_32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib "${CMAKE_CURRENT_SOURCE_DIR}/src/binklib/binkw32.lib" "${CMAKE_CURRENT_SOURCE_DIR}/src/steamsdk/steam_api.lib" dxguid.lib )
target_link_options(${BIN_NAME} PRIVATE /machine:x86)
set_target_properties(${BIN_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${BIN_NAME} )

#Enable PDB for "Release" Build. (There is also RelWithDebInfo, but it has different settings)
target_link_options(${BIN_NAME} PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
target_link_options(${BIN_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
target_link_options(${BIN_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")
