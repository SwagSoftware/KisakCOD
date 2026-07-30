#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// CoD4Radiant filter settings — MFC dispatch wrappers that forward UI events
// (checklist-toggle, select-names, select-angles, etc.) to CMainFrame methods.

#include "stdafx.h"
#include "mainfrm.h"
#include <universal/assertive.h>    // iassert (USE_ASSERTS always on; same handler as Assert)

// The binary reaches the main frame through AfxGetApp()->m_pMainWnd — hex-rays renders the
// AfxGetApp() inline as AfxGetModuleState()->m_pCurrentWinApp, and each wrapper RE-READS it
// after the assert before the tail-call (two AfxGetModuleState() calls per function).  The
// embedded assert condition string is "AfxGetApp()->m_pMainWnd", so the iassert expression
// is written in that exact form; the dispatch then goes through the port's already-typed
// g_pParentWnd alias (IDB 0x25D5A70), which is how the rest of the port translates this
// access (see camwnd.cpp OnKeyDown, drag.cpp, entity.cpp).  Same object: the CMainFrame
// ctor sets g_pParentWnd = this (mainfrm.cpp) and CRadiantApp::InitInstance assigns that
// very frame to m_pMainWnd.
extern CMainFrame *g_pParentWnd;

// The binary has no xrefs to these wrappers; its message map targets CMainFrame directly.
// The CMainFrame::OnSelect* handlers are afx_msg void in the port, so the binary's
// "return <callee's return>" tail-call becomes a plain call + the callee's 0/NULL result.

// ─────────────────────────────────────────────────────────────────────────────
// FilterSettings_OnSelectNames (sub_414B20, 0x414B20, 60 bytes)
// Dispatches "select names" filter action to the main frame.
// ─────────────────────────────────────────────────────────────────────────────
LRESULT FilterSettings_OnSelectNames()
{
    // IDA 0x414b20  assert line 281  filtersettings.cpp
    iassert( AfxGetApp()->m_pMainWnd );
    g_pParentWnd->OnSelectNames();          // IDB CMainFrame::OnSelectNames (0x42ba40)
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// FilterSettings_OnSelectAngles (sub_414B60, 0x414B60, 60 bytes)
// ─────────────────────────────────────────────────────────────────────────────
LRESULT FilterSettings_OnSelectAngles()
{
    // IDA 0x414b60  assert line 287  filtersettings.cpp
    iassert( AfxGetApp()->m_pMainWnd );
    g_pParentWnd->OnSelectAngles();         // IDB CMainFrame::OnSelectAngles (0x42baa0)
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// FilterSettings_OnSelectConnections (sub_414BA0, 0x414BA0, 60 bytes)
// ─────────────────────────────────────────────────────────────────────────────
LRESULT FilterSettings_OnSelectConnections()
{
    // IDA 0x414ba0  assert line 293  filtersettings.cpp
    iassert( AfxGetApp()->m_pMainWnd );
    g_pParentWnd->OnSelectConnections();    // IDB CMainFrame::OnSelectConnections (0x42bbc0)
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// FilterSettings_OnSelectBlocks (sub_414BE0, 0x414BE0, 60 bytes)
// ─────────────────────────────────────────────────────────────────────────────
LRESULT FilterSettings_OnSelectBlocks()
{
    // IDA 0x414be0  assert line 299  filtersettings.cpp
    iassert( AfxGetApp()->m_pMainWnd );
    g_pParentWnd->OnSelectBlocks();         // IDB CMainFrame::OnSelectBlocks (0x42bb00)
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// FilterSettings_OnSelectCoordinates (sub_414C20, 0x414C20, 60 bytes)
// ─────────────────────────────────────────────────────────────────────────────
LRESULT FilterSettings_OnSelectCoordinates()
{
    // IDA 0x414c20  assert line 305  filtersettings.cpp
    iassert( AfxGetApp()->m_pMainWnd );
    g_pParentWnd->OnSelectCoordinates();    // IDB CMainFrame::OnSelectCoordinates (0x42bb60)
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// FilterSettings_OnSelectReverseFilter (sub_414C60, 0x414C60, 60 bytes)
// ─────────────────────────────────────────────────────────────────────────────
HWND FilterSettings_OnSelectReverseFilter()
{
    // IDA 0x414c60  assert line 311  filtersettings.cpp
    iassert( AfxGetApp()->m_pMainWnd );
    g_pParentWnd->OnSelectReverseFilter();  // IDB CMainFrame::OnSelectReverseFilter (0x42bc20)
    return nullptr;
}
