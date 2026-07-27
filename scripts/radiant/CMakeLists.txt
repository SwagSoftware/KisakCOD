message("----- KisakCOD Radiant -----")

# MFC: static MFC (matches the original binary, statically linked MFC 7.1).
# Enabled once Microsoft.VisualStudio.Component.VC.ATLMFC was installed (afxwin.h
# present). stdafx.h includes the afx headers; CMainFrame is a real CFrameWnd and
# CRadiantApp a CWinApp (MFC provides WinMain). Static MFC requires /MT (set below).
set(CMAKE_MFC_FLAG 1)

project(KisakCOD-Radiant)

# Phase 2: engine file sets (UNIVERSAL, XANIM, etc.) from common_files.cmake
include(${SCRIPTS_DIR}/common_files.cmake)

# Editor file list + radiant-specific engine subset
include(radiant_files.cmake)

# DXSDK — same pattern as pre_build.cmake
if (DEFINED CICD)
    if (NOT DEFINED DXSDK_DIR)
        message(FATAL_ERROR "DXSDK_DIR must be set for CI builds (-DDXSDK_DIR=...)")
    endif()
    set(DXSDK_INC_DIR ${DXSDK_DIR}/include)
    set(DXSDK_LIB_DIR ${DXSDK_DIR}/${CMAKE_BUILD_TYPE}/lib/x86)
else()
    set(DXSDK_DIR $ENV{DXSDK_DIR})
    set(DXSDK_INC_DIR ${DXSDK_DIR}/include)
    set(DXSDK_LIB_DIR ${DXSDK_DIR}/lib/x86)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(D3DX_LIB d3dx9d.lib)
else()
    set(D3DX_LIB d3dx9.lib)
endif()

# WIN32 = GUI subsystem (not console)
# Engine subset: UNIVERSAL (leaf math/string/FS), COMMON (brush_edges), XANIM (model preview),
# RADIANT_SCRIPT (scr_memorytree + scr_stringlist), RADIANT_PHYSICS (physpreset + ode mass/matrix),
# RADIANT_QCOMMON (com_bsp + cmd), RADIANT_GFX_D3D (filtered: game-only files excluded).
# WIN32 group: added only as link errors demand it (Radiant has its own Sys_* in win_qe3.cpp).
add_executable(${PROJECT_NAME} WIN32
    ${RADIANT_SRCS}
    ${UNIVERSAL}
    ${COMMON}
    ${XANIM}
    ${ZLIB}
    ${RADIANT_SCRIPT}
    ${RADIANT_PHYSICS}
    ${RADIANT_QCOMMON}
    ${RADIANT_GFX_D3D}
)

# KISAK_RADIANT guards all editor additions.
# _MBCS / not UNICODE: the binary was built ANSI; all decompiled code is char*.
# _WINDOWS / _AFXAPP: standard MFC flags (CMAKE_MFC_FLAG=1 adds _AFX implicitly, but be explicit).
target_compile_definitions(${PROJECT_NAME} PUBLIC
    KISAK_RADIANT
    WIN32
    _WINDOWS
    _MBCS
    # Radiant edits SP maps; use SP entity count so q_shared.h MAX_GENTITIES resolves
    MAX_GENTITIES=2176
    # The editor runs without fastfiles (loose assets). KISAK_NO_FASTFILES selects the
    # loose-asset code paths: precompiled shaders from raw/shader_bin/ keyed by
    # shader_bin/shader_names (Material_PreLoadAllShaderText / Material_Load*Shader),
    # the loose StringTable loader, and IsFastFileLoad()==false at compile time. The
    # stock install ships no shaders/*.hlsl sources, only the precompiled binaries.
    KISAK_NO_FASTFILES
)

# Gate P4 (plan §6): headless map round-trip self-test. When ON, InitInstance runs
# the load->save->diff harness (no UI) and exits 0/1/2. Build a dedicated selftest
# variant with `cmake -DRADIANT_SELFTEST=ON ...`.
option(RADIANT_SELFTEST "Build the headless map round-trip self-test harness" OFF)
if(RADIANT_SELFTEST)
    message("  RADIANT_SELFTEST: ON (headless map round-trip harness)")
    target_compile_definitions(${PROJECT_NAME} PUBLIC RADIANT_SELFTEST)
endif()

# Compile flags — match the SP/MP builds (/MT static CRT to match static MFC)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /O2 /Ot /MP4 /W3 /Z7 /permissive-")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   /MTd /Od /MP4 /W3 /Z7 /permissive-")

# Static MSVC runtime (required for static MFC)
set_property(TARGET ${PROJECT_NAME} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# x86 / Win32
set(CMAKE_GENERATOR_PLATFORM "Win32")
target_link_options(${PROJECT_NAME} PRIVATE /machine:x86)


# PDB for Release
target_link_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
target_link_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
target_link_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")

# Include paths
target_include_directories(${PROJECT_NAME} PUBLIC ${SRC_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${DEPS_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${DXSDK_INC_DIR})

# Library search paths
target_link_directories(${PROJECT_NAME} PUBLIC ${DXSDK_LIB_DIR})

# Link libraries — no Miles/Bink/Steam (Radiant doesn't use them)
target_link_libraries(${PROJECT_NAME} PUBLIC
    d3d9.lib
    ${D3DX_LIB}
    ws2_32.lib
    gdiplus.lib
    winmm.lib
    comctl32.lib
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
    dxguid.lib
    ddraw.lib
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${BIN_DIR}/$<CONFIG>"
)

# VS Solution Explorer folders mirroring the on-disk tree (src/… and deps/…).
# source_group(TREE) requires every listed file to live under its root, hence the split.
source_group(TREE ${SRC_DIR} PREFIX "src" FILES
    ${RADIANT_SRCS}
    ${UNIVERSAL}
    ${COMMON}
    ${XANIM}
    ${RADIANT_SCRIPT}
    ${RADIANT_PHYSICS}
    ${RADIANT_QCOMMON}
    ${RADIANT_GFX_D3D}
)
source_group(TREE ${DEPS_DIR} PREFIX "deps" FILES ${ZLIB})
