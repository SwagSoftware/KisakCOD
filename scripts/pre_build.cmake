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
  COMMAND ${SCRIPTS_DIR}/increment_build${SCRIPT_EXT} ${SRC_DIR} ${GIT_COMMIT_COUNT}
  COMMENT "Running build number script..."
)

# We want to update the build number before building
add_dependencies(${PROJECT_NAME} update_build_number)

# Set Win32 Visual Studio Specifics
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${BIN_DIR}/$<CONFIG>"
)

# Feature flag for gate more drastic changes to the code, or experimental
if (DEFINED KISAK_EXTENDED)
  target_compile_definitions(${PROJECT_NAME} KISAK_EXTENDED)
endif()

# Set Win32 compiler flag
target_compile_definitions(${PROJECT_NAME} PUBLIC WIN32 _CONSOLE _MBCS)

# Set the generator platform
set(CMAKE_GENERATOR_PLATFORM "WIN32")

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

target_include_directories(${PROJECT_NAME} PUBLIC ${SRC_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${DEPS_DIR})

target_include_directories(${PROJECT_NAME} PUBLIC ${DXSDK_INC_DIR})

target_link_directories(${PROJECT_NAME} PUBLIC ${DXSDK_LIB_DIR})
target_link_directories(${PROJECT_NAME} PUBLIC "${DEPS_DIR}/msslib")
target_link_directories(${PROJECT_NAME} PUBLIC "${DEPS_DIR}/steamsdk")
target_link_directories(${PROJECT_NAME} PUBLIC "${DEPS_DIR}/binklib")

#Enable PDB for "Release" Build. (There is also RelWithDebInfo, but it has different settings)
target_link_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
target_link_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
target_link_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")

target_link_options(${PROJECT_NAME} PRIVATE /machine:x86)
set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

target_link_libraries(${PROJECT_NAME} PUBLIC
        mss32.lib
        dsound.lib
        ${D3DX_LIB}
        d3d9.lib
        ddraw.lib
        ws2_32.lib
        winmm.lib
        kernel32.lib
        user32.lib
        gdi32.lib
        winspool.lib
        comdlg32.lib
        advapi32.lib
        shell32.lib
        ole32.lib
        oleaut32.lib
        uuid.lib
        odbc32.lib
        odbccp32.lib
        binkw32.lib
        steam_api.lib
        dxguid.lib
)

set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

