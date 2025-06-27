cmake_minimum_required(VERSION 3.16)

if (DEDICATED)
  project(KC-Dedicated)
else()
  project(KisakCOD)
endif()

set(CMAKE_CXX_STANDARD 20)

# If we are building on windows
if (WIN32)
  # Set the generator platform
  set(CMAKE_GENERATOR_PLATFORM "WIN32")

  # Check to see if we are running a github action
  if (DEFINED "$ENV{GITHUB_SHA}")
    # Check if this variable has been set
    if (NOT DEFINED DXSDK_DIR)
      message(FATAL_ERROR "DXSDK_DIR needs to be set to the nuget location, this can be done with -DDXSDK_DIR=")
    endif()

    # Example: C:\Users\USERNAME\.nuget\packages\microsoft.dxsdk.d3dx\9.29.952.8\build\native
    set(DXSDK_INC_DIR ${DXSDK_DIR}/include)
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      set(DXSDK_LIB_DIR ${DXSDK_DIR}/debug/lib/x86)
    else()
      set(DXSDK_LIB_DIR ${DXSDK_DIR}/release/lib/x86)
    endif()
  else()
    set(DXSDK_DIR $ENV{DXSDK_DIR})
    set(DXSDK_INC_DIR ${DXSDK_DIR}/include)
    set(DXSDK_LIB_DIR ${DXSDK_DIR}/lib/x86)
  endif()
  
  
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

target_compile_definitions(${BIN_NAME} PUBLIC _CONSOLE)
target_compile_definitions(${BIN_NAME} PUBLIC _MBCS)

target_include_directories(${BIN_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_include_directories(${BIN_NAME} PUBLIC ${DXSDK_INC_DIR})

target_link_directories(${BIN_NAME} PUBLIC ${DXSDK_LIB_DIR})
target_link_directories(${BIN_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_directories(${BIN_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/msslib")
target_link_directories(${BIN_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/steamsdk")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /O2 /Ot /MP /W3 /Zi ${MSVC_WARNING_DISABLES} /permissive-")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /Od /MP /W3 /Zi ${MSVC_WARNING_DISABLES} /permissive-")

set_property(TARGET ${BIN_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# fixme: copied from sln, check whats not needed
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
target_link_directories(${BIN_NAME} PUBLIC d3dx9d.lib)
else()
target_link_directories(${BIN_NAME} PUBLIC d3dx9.lib)
endif()

target_link_libraries(${BIN_NAME} PUBLIC mss32.lib dsound.lib d3d9.lib ddraw.lib ws2_32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib "${CMAKE_CURRENT_SOURCE_DIR}/src/binklib/binkw32.lib" "${CMAKE_CURRENT_SOURCE_DIR}/src/steamsdk/steam_api.lib" dxguid.lib )
target_link_options(${BIN_NAME} PRIVATE /machine:x86)
set_target_properties(${BIN_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${BIN_NAME} )

#Enable PDB for "Release" Build. (There is also RelWithDebInfo, but it has different settings)
target_link_options(${BIN_NAME} PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
target_link_options(${BIN_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
target_link_options(${BIN_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")
