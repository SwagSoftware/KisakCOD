cmake_minimum_required(VERSION 3.16)
include(FetchContent)

if (DEDICATED)
  project(KisakCOD-Dedi)
else()
  project(KisakCOD)
endif()

set(CMAKE_CXX_STANDARD 20)

# If we are building on windows
if (WIN32)
  # Set the generator platform
  set(CMAKE_GENERATOR_PLATFORM "WIN32")

  # Check to see if we are running a github action
  if (DEFINED CICD)
    message("===== BUILDING FOR GITHUB ACTIONS =====")

    # Check if this variable has been set
    if (NOT DEFINED DXSDK_DIR)
      message(FATAL_ERROR "DXSDK_DIR needs to be set to the nuget location, this can be done with -DDXSDK_DIR=")
    endif()

    message("DXSDK_DIR: ${DXSDK_DIR}")

    # Example: C:\Users\USERNAME\.nuget\packages\microsoft.dxsdk.d3dx\9.29.952.8\build\native
    set(DXSDK_INC_DIR ${DXSDK_DIR}/include)
    set(DXSDK_LIB_DIR ${DXSDK_DIR}/${CMAKE_BUILD_TYPE}/lib/x86)
    message("DXSDK_LIB_DIR: ${DXSDK_LIB_DIR}")
  else()
    message("===== BUILDING FOR LOCAL DXSDK =====")
    set(DXSDK_DIR $ENV{DXSDK_DIR})
    set(DXSDK_INC_DIR ${DXSDK_DIR}/include)
    set(DXSDK_LIB_DIR ${DXSDK_DIR}/lib/x86)
  endif() # DEFINED CICD
  
  # Set the required library
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(D3DX_LIB d3dx9d.lib)
  else()
    set(D3DX_LIB d3dx9.lib)
  endif() # CMAKE_BUILD_TYPE Debug

endif() # WIN32

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps")
set(BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

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

# Add a custom target to increment the build number
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

# Feature flag for gate more drastic changes to the code, or experimental
if (DEFINED KAISK_EXTENDED)
  message("Building KisakCOD with enhancement extensions!")
  target_compile_definitions(${BIN_NAME} KAISAK_EXTENDED)
endif()

# We want to update the build number before building
add_dependencies(${BIN_NAME} update_build_number)

# Copy over MILES dependency
add_custom_command(
  TARGET ${BIN_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${DEPS_DIR}/msslib/dlls
          ${BIN_DIR}/${CMAKE_BUILD_TYPE}
  COMMENT "COPYING MILES DEPENDENCIES"
)

# Copy over steam depdendency
add_custom_command(
  TARGET ${BIN_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${DEPS_DIR}/steamsdk/steam_api.dll
          ${BIN_DIR}/${CMAKE_BUILD_TYPE}
  COMMENT "COPYING STEAM DEPENDENCIES"
)

set_target_properties(${BIN_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${BIN_DIR}/$<CONFIG>"
)

target_compile_definitions(${BIN_NAME} PUBLIC WIN32)

#Set MSVC Runtime MDd/ MD
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

##### Tracy #####
option ( TRACY_ENABLE "" ON )
option ( TRACY_ON_DEMAND "" ON )
option ( TRACY_ONLY_LOCALHOST "" ON )
#option ( TRACY_FIBERS "" ON )

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
target_compile_definitions(${BIN_NAME} PUBLIC $<$<CONFIG:Debug>:TRACY_ENABLE>)
target_compile_definitions(${BIN_NAME} PUBLIC $<$<CONFIG:Debug>:TRACY_ON_DEMAND>)
target_link_libraries(${BIN_NAME} PUBLIC $<$<CONFIG:Debug>:TracyClient>) # Enable Profiler in Debug
#################

target_compile_definitions(${BIN_NAME} PUBLIC _CONSOLE)
target_compile_definitions(${BIN_NAME} PUBLIC _MBCS)

target_include_directories(${BIN_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(${BIN_NAME} PUBLIC ${DEPS_DIR})

target_include_directories(${BIN_NAME} PUBLIC ${DXSDK_INC_DIR})

target_link_directories(${BIN_NAME} PUBLIC ${DXSDK_LIB_DIR})
target_link_directories(${BIN_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_directories(${BIN_NAME} PUBLIC "${DEPS_DIR}/msslib")
target_link_directories(${BIN_NAME} PUBLIC "${DEPS_DIR}/steamsdk")
target_link_directories(${BIN_NAME} PUBLIC "${DEPS_DIR}/binklib")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /O2 /Ot /MP /W3 /Zi ${MSVC_WARNING_DISABLES} /permissive-")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /Od /MP /W3 /Zi ${MSVC_WARNING_DISABLES} /permissive-")

set_property(TARGET ${BIN_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# fixme: copied from sln, check whats not needed
target_link_libraries(${BIN_NAME} PUBLIC mss32.lib dsound.lib ${D3DX_LIB} d3d9.lib ddraw.lib ws2_32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib binkw32.lib steam_api.lib dxguid.lib )

get_target_property(LIBS_OUT ${BIN_NAME} LINK_LIBRARIES)
message("LIBS: ${LIBS_OUT}")

target_link_options(${BIN_NAME} PRIVATE /machine:x86)
set_target_properties(${BIN_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${BIN_NAME} )

#Enable PDB for "Release" Build. (There is also RelWithDebInfo, but it has different settings)
target_link_options(${BIN_NAME} PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
target_link_options(${BIN_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
target_link_options(${BIN_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")