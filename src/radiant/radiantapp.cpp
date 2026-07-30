#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// MFC application startup: CRadiantApp::InitInstance brings up the editor shell.

#include "stdafx.h"
#include "mainfrm.h"
#include "prefs.h"

#include <qcommon/qcommon.h>

extern CMainFrame *g_pParentWnd;   // engine_stubs.cpp

class CRadiantApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    // P5.5: drive CMainFrame::RoutineProcessing from the idle pump — flush g_nUpdateBits
    // (the Sys_UpdateWindows broadcast). Returning TRUE keeps OnIdle firing so a queued
    // redraw lands promptly between input messages. (The binary runs RoutineProcessing
    // from its message loop the same way.)
    virtual BOOL OnIdle( LONG lCount );
};

CRadiantApp theApp;

BOOL CRadiantApp::OnIdle( LONG lCount )
{
    BOOL more = CWinApp::OnIdle( lCount );
    // Flush pending window-update bits once per idle (after each input/timer batch).
    // RoutineProcessing no-ops when g_nUpdateBits == 0, so this does not busy-spin —
    // return the base value so the pump blocks on GetMessage when nothing is queued.
    if ( g_pParentWnd && ::IsWindow( g_pParentWnd->GetSafeHwnd() ) )
        g_pParentWnd->RoutineProcessing();
    return more;
}

BOOL CRadiantApp::InitInstance()
{
    // Gate P2 smoke test: verify the engine subset links and basic printing works
    // before any real init. Com_Printf is safe pre-init (falls through to stderr).
    Com_Printf(0, "[Radiant] Gate P2 smoke: engine subset initialized, InitInstance reached\n");

    // Register the common-control window classes the editor's child controls need (the
    // inspector tab strip uses SysTabControl32 — ICC_TAB_CLASSES).  Idempotent.
    {
        INITCOMMONCONTROLSEX icc;
        icc.dwSize = sizeof( icc );
        icc.dwICC  = ICC_TAB_CLASSES | ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES |
                     ICC_UPDOWN_CLASS | ICC_STANDARD_CLASSES;
        InitCommonControlsEx( &icc );
    }

    // P6 prefs: registry-backed persistence (binary's SetRegistryKey), then bring the
    // preference singleton up with the saved values BEFORE any view reads g_PrefsDlg.
    // (g_PrefsDlg is already defaults-constructed at static init; this overlays the
    // registry on top — faithful to the binary's startup LoadPrefs.)
    SetRegistryKey( "iw\\CoD4Radiant" );
    Prefs_Init( /*loadFromRegistry=*/true );

    CMainFrame *pFrame = new CMainFrame();
    if (!pFrame)
        return FALSE;
    m_pMainWnd = pFrame;

    RECT work = { 0, 0, 1600, 1000 };
    if ( SystemParametersInfoA( SPI_GETWORKAREA, 0, &work, 0 ) )
    {
        int w = work.right - work.left;
        int h = work.bottom - work.top;
        work.left += w / 40;
        work.top += h / 40;
        work.right -= w / 40;
        work.bottom -= h / 40;
    }
    if (!pFrame->Create(NULL, "CoD4Radiant", WS_OVERLAPPEDWINDOW, CRect(work)))
        return FALSE;

    pFrame->ShowWindow(m_nCmdShow);
    pFrame->UpdateWindow();

    // KISAK_RADIANT debug aid: if RADIANT_STARTUP_MAP is set, load that map at startup
    // (no effect when unset). Lets the first-light log capture a real map's applied
    // camera origin + render state for far-from-origin debugging (no GUI File->Open).
    if ( const char *startupMap = getenv( "RADIANT_STARTUP_MAP" ) )
    {
        extern void Radiant_FL_Log( const char *fmt, ... );
        extern void Map_LoadFromFile( const char *path );
        Map_LoadFromFile( startupMap );
        // Debug aid (paired with RADIANT_STARTUP_MAP): RADIANT_STARTUP_CAM="x y z pitch yaw roll"
        // repositions the 3D camera after load AND rebuilds its view basis (Cam_BuildMatrix), so a
        // headless run can render a real vantage (origin alone leaves the map out-of-frustum).
        if ( const char *camStr = getenv( "RADIANT_STARTUP_CAM" ) )
        {
            if ( g_pParentWnd && g_pParentWnd->m_pCamWnd )
            {
                float x=0,y=0,z=0,pi=0,ya=0,ro=0;
                if ( sscanf( camStr, "%f %f %f %f %f %f", &x,&y,&z,&pi,&ya,&ro ) >= 3 )
                {
                    g_pParentWnd->m_pCamWnd->camera.origin[0] = x;
                    g_pParentWnd->m_pCamWnd->camera.origin[1] = y;
                    g_pParentWnd->m_pCamWnd->camera.origin[2] = z;
                    g_pParentWnd->m_pCamWnd->camera.angles[0] = pi;
                    g_pParentWnd->m_pCamWnd->camera.angles[1] = ya;
                    g_pParentWnd->m_pCamWnd->camera.angles[2] = ro;
                    g_pParentWnd->m_pCamWnd->Cam_BuildMatrix();   // rebuild vpn/vright/vup so Cam_Fov's frustum matches
                }
            }
        }
        if ( g_pParentWnd && g_pParentWnd->m_pCamWnd )
            Radiant_FL_Log( "STARTUP_MAP %s: camera.origin=(%.1f %.1f %.1f) angles=(%.1f %.1f %.1f)  XY.origin=(%.1f %.1f %.1f) scale=%.4f",
                startupMap,
                g_pParentWnd->m_pCamWnd->camera.origin[0],
                g_pParentWnd->m_pCamWnd->camera.origin[1],
                g_pParentWnd->m_pCamWnd->camera.origin[2],
                g_pParentWnd->m_pCamWnd->camera.angles[0],
                g_pParentWnd->m_pCamWnd->camera.angles[1],
                g_pParentWnd->m_pCamWnd->camera.angles[2],
                g_pParentWnd->m_pXYWnd ? g_pParentWnd->m_pXYWnd->m_vOrigin[0] : 0.0f,
                g_pParentWnd->m_pXYWnd ? g_pParentWnd->m_pXYWnd->m_vOrigin[1] : 0.0f,
                g_pParentWnd->m_pXYWnd ? g_pParentWnd->m_pXYWnd->m_vOrigin[2] : 0.0f,
                g_pParentWnd->m_pXYWnd ? g_pParentWnd->m_pXYWnd->m_fScale : 0.0f );
    }
    return TRUE;
}
