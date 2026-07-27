#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// CoD4Radiant filter settings — MFC dispatch wrappers that forward UI events
// (checklist-toggle, select-names, select-angles, etc.) to CMainFrame methods.

#include "stdafx.h"
#include "mainfrm.h"

// IDB g_pParentWnd (0x25D5A70) — the binary's AfxGetModuleState()->m_pCurrentWinApp->
// m_pMainWnd is the CMainFrame; the rest of the port already translates that access to
// g_pParentWnd (see camwnd.cpp OnKeyDown, drag.cpp, entity.cpp).
extern CMainFrame *g_pParentWnd;
extern void Assert( const char *file, int line, int type, const char *fmt, ... );

// The binary has no xrefs to these wrappers; its message map targets CMainFrame directly.

// ─────────────────────────────────────────────────────────────────────────────
// FilterSettings_OnSelectNames (sub_414B20, 0x414B20, 60 bytes)
// Dispatches "select names" filter action to the main frame.
// ─────────────────────────────────────────────────────────────────────────────
LRESULT FilterSettings_OnSelectNames()
{
    // IDA 0x414b20  assert line 281  filtersettings.cpp
    if ( !g_pParentWnd )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\filtersettings.cpp",
                281, 0, "%s", "AfxGetApp()->m_pMainWnd" );
    g_pParentWnd->OnSelectNames();      // IDB CMainFrame::OnSelectNames (0x42ba40)
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// FilterSettings_OnSelectAngles (sub_414B60, 0x414B60, 60 bytes)
// ─────────────────────────────────────────────────────────────────────────────
LRESULT FilterSettings_OnSelectAngles()
{
    // IDA 0x414b60  assert line 287  filtersettings.cpp
    if ( !g_pParentWnd )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\filtersettings.cpp",
                287, 0, "%s", "AfxGetApp()->m_pMainWnd" );
    g_pParentWnd->OnSelectAngles();     // IDB CMainFrame::OnSelectAngles (0x42bac0)
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// FilterSettings_OnSelectConnections (sub_414BA0, 0x414BA0, 60 bytes)
// ─────────────────────────────────────────────────────────────────────────────
LRESULT FilterSettings_OnSelectConnections()
{
    // IDA 0x414ba0  assert line 293  filtersettings.cpp
    if ( !g_pParentWnd )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\filtersettings.cpp",
                293, 0, "%s", "AfxGetApp()->m_pMainWnd" );
    g_pParentWnd->OnSelectConnections();    // IDB CMainFrame::OnSelectConnections (0x42bbc0)
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// FilterSettings_OnSelectBlocks (sub_414BE0, 0x414BE0, 60 bytes)
// ─────────────────────────────────────────────────────────────────────────────
LRESULT FilterSettings_OnSelectBlocks()
{
    // IDA 0x414be0  assert line 299  filtersettings.cpp
    if ( !g_pParentWnd )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\filtersettings.cpp",
                299, 0, "%s", "AfxGetApp()->m_pMainWnd" );
    g_pParentWnd->OnSelectBlocks();     // IDB CMainFrame::OnSelectBlocks (0x42bb00)
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// FilterSettings_OnSelectCoordinates (sub_414C20, 0x414C20, 60 bytes)
// ─────────────────────────────────────────────────────────────────────────────
LRESULT FilterSettings_OnSelectCoordinates()
{
    // IDA 0x414c20  assert line 305  filtersettings.cpp
    if ( !g_pParentWnd )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\filtersettings.cpp",
                305, 0, "%s", "AfxGetApp()->m_pMainWnd" );
    g_pParentWnd->OnSelectCoordinates();    // IDB CMainFrame::OnSelectCoordinates (0x42bb60)
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// FilterSettings_OnSelectReverseFilter (sub_414C60, 0x414C60, 60 bytes)
// ─────────────────────────────────────────────────────────────────────────────
HWND FilterSettings_OnSelectReverseFilter()
{
    // IDA 0x414c60  assert line 311  filtersettings.cpp
    if ( !g_pParentWnd )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\filtersettings.cpp",
                311, 0, "%s", "AfxGetApp()->m_pMainWnd" );
    g_pParentWnd->OnSelectReverseFilter();  // IDB CMainFrame::OnSelectReverseFilter (0x42bc20)
    return nullptr;
}
