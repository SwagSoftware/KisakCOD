#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Radiant-to-D3D renderer bootstrap. Engine-owned wrapper symbols are supplied by
// the normal KisakCOD engine sources.

#include "stdafx.h"
#include <gfx_d3d/r_init.h>         // R_Init, R_InitRendererForWindow
#include <gfx_d3d/r_rendercmds.h>   // R_InitRenderCommands
#include <gfx_d3d/r_font.h>         // R_RegisterFont
#include <gfx_d3d/r_material.h>     // Material_RegisterHandle
#include <universal/com_memory.h>   // Com_InitHunkMemory

// ─── Editor globals owned by this TU ────────────────────────────────────────────
int  R_Initiated;             // IDB 0x25d5a68 — "editor renderer is up" flag.
HWND laymatwnd_content_HWND;  // IDB 0x181f508 — LayeredMaterialWnd content-list HWND.
                              // LayeredMaterialWnd.cpp (Phase 6) takes ownership later;
                              // declared here so R_BeginRegistrationInternal links now.

// The radiant verbose Assert stub (engine_stubs.cpp): type 0 = log-to-stderr + CONTINUE.
extern void Assert( const char *file, int line, int type, const char *fmt, ... );

// ─── R_BeginRegistrationInternal (IDB 0x416510) ─────────────────────────────────
// The editor renderer bootstrap. Registers the few dvars the tools path touches,
// applies the saved picmip preferences, brings up the render-command system + hunk +
// renderer, then attaches the D3D device to each editor child window and registers the
// editor's font and utility materials. Sole caller in the binary: QE_LoadProject
// Called by QE_LoadProject during editor startup.
//
// §11 signature reconciliation against the compiled kisak renderer:
//  • R_Init() and R_InitRenderCommands() are VOID in kisak; the IDB passes a scratch
//    out-param / a {mode=1,size=32} config block (CoD4 forms). Both dropped.
//  • Hunk_Init() (editor) == Com_InitHunkMemory() (kisak).
//  • Dvar_SetIntByName is (name, value) in kisak — the IDB pseudocode lists the args
//    SWAPPED (value, name); we follow the kisak prototype.
//  • R_RegisterFont(name, imageTrack) and Material_RegisterHandle(name, imageTrack)
//    match the IDB.
Material *R_BeginRegistrationInternal()
{
    // The binary assigns the com_statmon/sv_cheats/sys_SSE globals from these returns;
    // in the kisak port those globals are owned + assigned by the engine's own dvar
    // registration (common.cpp / r_dvars.cpp / com_playerprofile.cpp), so we only ensure
    // they are registered (Dvar_RegisterBool is idempotent) and discard the returns —
    // re-binding the engine-owned globals here is both redundant and a const-qualifier
    // ABI mismatch against their canonical declarations.
    Dvar_RegisterBool("com_statmon", 0, 0, "Draw stats monitor");
    Dvar_RegisterBool("sv_cheats", 1, 72, "Allow server side cheats");
    Dvar_RegisterBool("sys_SSE", 0, 64, "Operating system allows Streaming SIMD Extensions");

    Dvar_SetIntByName("r_picmip", g_qeglobals.d_savedinfo.d_picmip);
    Dvar_SetIntByName("r_picmip_spec", 3);
    Dvar_SetIntByName("r_picmip_bump", 3);

    R_InitRenderCommands();
    Com_InitHunkMemory();
    // IDB calls R_Init at 0x500820 — that EA is the EDITOR R_Init (R_InitEditor:
    // Direct3DCreate9 + adapter enum only, leaves dx.device NULL), NOT kisak's game
    // R_Init (which creates its own window + device). The per-window R_InitRendererForWindow
    // calls below create the device. (First exercised at P5.5; the First-Light bootstrap
    // called R_InitEditor directly, which this now subsumes.)
    R_InitEditor();

    iassert( g_qeglobals.d_hwndCamera );
    R_InitRendererForWindow(g_qeglobals.d_hwndCamera);
    iassert( g_qeglobals.d_hwndXY );
    R_InitRendererForWindow(g_qeglobals.d_hwndXY);
    iassert( g_qeglobals.d_hwndZ );
    R_InitRendererForWindow(g_qeglobals.d_hwndZ);
    iassert( g_qeglobals.d_hwndTexture );
    R_InitRendererForWindow(g_qeglobals.d_hwndTexture);
    // CROSS-FILE assert: the binary inlines LayeredMaterialWnd.cpp:726 here (its layer-list HWND
    // check); KEEP VERBOSE to preserve that origin + the "lyrMtlWndGlob.layerList" string (the
    // port's var is laymatwnd_content_HWND, which a stringized iassert can't byte-match).
    if ( !laymatwnd_content_HWND )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\LayeredMaterialWnd.cpp",
                726, 0, "%s", "lyrMtlWndGlob.layerList" );
    R_InitRendererForWindow(laymatwnd_content_HWND);

    g_qeglobals.d_font_list = R_RegisterFont("fonts/qerfont", 7);
    g_qeglobals.d_white     = Material_RegisterHandle("white_tools", 0);
    g_qeglobals.d_opague    = Material_RegisterHandle("$opaque", 0);
    g_qeglobals.d_additive  = Material_RegisterHandle("$additive", 0);

    R_Initiated = 1;
    return g_qeglobals.d_additive;
}

// The original tools build also defined engine-wrapper stubs here. KisakCOD links
// their canonical implementations from the engine modules instead.
