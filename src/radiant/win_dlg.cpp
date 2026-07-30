#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Find-brush, go-to-position, and arbitrary-rotation utility dialogs.
// Selection indices resolve entity instances to their definition brush lists.

#include "stdafx.h"
#include "qe3.h"
#include "mainfrm.h"
#include <afxdlgs.h>   // CColorDialog — TU-LOCAL ONLY (adding it to the shared stdafx.h PCH
                       // reorders windows.h ahead of the DXSDK headers and breaks every
                       // gfx_d3d TU — same gotcha noted in verteditdlg.cpp).
#include <cstdio>
#include <cstdlib>

// ── globals / cores reused from other TUs ─────────────────────────────────────
extern entity_s   entityInsts;                                  // 0x23F1748 (map.cpp) — instance list sentinel
extern entity_s  *world_entity;                                 // 0x25D5B30 (map.cpp)
extern int        Sys_Printf( const char *fmt, ... );           // 0x499E90
extern void       Assert( const char *file, int line, int type, const char *fmt, ... ); // 0x49CEA0
extern void       Select_Brush( selbrush_t *b, char overwrite, char status, char center ); // select.cpp (0x48DCC0)
extern void       Select_Deselect( int bDeselectFaces );        // select.cpp (0x48E800)
extern void       Select_GetMid( float *mid );                  // select.cpp (0x48FC70)
extern void       Select_RotateAxis( int axis, float deg, float (*rot_around)[4][3] ); // select.cpp (0x48FF40)
extern void       Select_ApplyMatrix_SelectedBrushes( int bSnap, float *mat, float deg, char bSwap ); // select.cpp (0x48FD10)
extern int        g_nUpdateBits;                                // 0x25D5A74 (engine_stubs.cpp)
extern CMainFrame *g_pParentWnd;                                // 0x25D5A70 (engine_stubs.cpp)

extern void       Undo_ClearRedo();                             // undo.cpp (0x45DF20)
extern void       Undo_GeneralStart( const char *opName );      // undo.cpp (0x45E3F0)
extern void       Undo_AddBrushList( selbrush_t *list );        // undo.cpp (0x45E7C0)
extern void       Undo_EndBrushList( selbrush_t *list );        // undo.cpp (0x45E870)
extern void       Undo_End();                                   // undo.cpp (0x45EA20)
extern int        UpdateSelection( int wParam, eclass_t *cls ); // win_ent.cpp (0x497180)

// selected_brushes sentinel is declared in qe3.h.

// ══════════════════════════════════════════════════════════════════════════════
//  0x495550  Select_ByEntityNumber(brushIdx, entIdx) — select the entIdx-th entity
//  instance's brushIdx-th DEF brush, by number.  Walks entityInsts to the entIdx-th
//  entity, then that entity's DEF brush list to the brushIdx-th brush_t, then finds the
//  matching INSTANCE (selbrush_t whose .def == that brush_t) and Select_Brush()es it;
//  finally recenters the XY view + camera on the brush's bbox centre.  Status messages
//  go to the status bar exactly as the binary (Sys_Printf + the d_hwndStatus pane text).
// ══════════════════════════════════════════════════════════════════════════════
void Select_ByEntityNumber( int brushIdx, int entIdx )
{
    iassert(entityInsts.next == world_entity);

    // Walk to the entIdx-th entity instance.
    entity_s *ent = entityInsts.next;
    if ( ent == &entityInsts )
    {
        Sys_Printf( "Couldn't select brush by number: no such entity.\n" );
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0, (LPARAM)"No such entity." );
        return;
    }
    while ( entIdx )
    {
        ent = ent->next;
        --entIdx;
        if ( ent == &entityInsts )
        {
            Sys_Printf( "Couldn't select brush by number: no such entity.\n" );
            SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0, (LPARAM)"No such entity." );
            return;
        }
    }

    // Walk that entity's DEF brush list to the brushIdx-th brush_t def.
    entity_s_def *def = (entity_s_def *)ent->def;
    brush_t      *bdef = (brush_t *)def->brushes.prev;            // IDA brushes.oprev (+0x0C)
    void         *defSentinel = (void *)&def->def; // IDA &def->def (+0x08)
    if ( (void *)bdef == defSentinel )
    {
        Sys_Printf( "Couldn't select brush by number: no such brush in entity.\n" );
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0, (LPARAM)"No such brush." );
        return;
    }
    while ( brushIdx )
    {
        bdef = bdef->onext;
        --brushIdx;
        if ( (void *)bdef == defSentinel )
        {
            Sys_Printf( "Couldn't select brush by number: no such brush in entity.\n" );
            SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0, (LPARAM)"No such brush." );
            return;
        }
    }

    // Find the INSTANCE whose def == bdef (walk the entity's instance brush list).
    selbrush_t *inst = ent->brushes.ownerNext;
    for ( ; ; inst = inst->ownerNext )
    {
        if ( inst == &ent->brushes )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\win_dlg.cpp",
                    113, 0, "%s", "selectedBrushInst != &(selectedEntityInst->brushes)" );
        if ( inst->def == bdef )
            break;
    }

    Select_Brush( inst, 0, 1, 0 );
    g_nUpdateBits = -1;

    // Recenter XY view + camera on the brush's bbox centre.
    float cx = ( bdef->mins[0] + bdef->maxs[0] ) * 0.5f;
    float cy = ( bdef->mins[1] + bdef->maxs[1] ) * 0.5f;
    float cz = ( bdef->mins[2] + bdef->maxs[2] ) * 0.5f;
    if ( g_pParentWnd )
    {
        CXYWnd *xy = g_pParentWnd->m_pXYWnd;
        if ( xy )
        {
            xy->m_vOrigin[0] = cx;
            xy->m_vOrigin[1] = cy;
            xy->m_vOrigin[2] = cz;
        }
        CCamWnd *cam = g_pParentWnd->m_pCamWnd;
        if ( cam )
        {
            cam->camera.origin[0] = cx;
            cam->camera.origin[1] = cy;
            cam->camera.origin[2] = cz;
        }
    }
    SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0, (LPARAM)"Selected." );
}

// ══════════════════════════════════════════════════════════════════════════════
//  0x495700  GetSelectionIndex(&entOut, &brushOut) — report the entity# + brush# of the
//  first selected brush (the inverse of Select_ByEntityNumber, used to preload the Find
//  brush dialog).  Counts the selected instance's owner position in entityInsts (entOut)
//  and its def's position in the owner's DEF brush list (brushOut).  No selection → 0/0.
// ══════════════════════════════════════════════════════════════════════════════
void GetSelectionIndex( int *entOut, int *brushOut )
{
    selbrush_t *sb = selected_brushes.next;
    *entOut   = 0;
    *brushOut = 0;
    if ( sb == &selected_brushes )
        return;

    iassert(entityInsts.next == world_entity);

    // entity index = position of sb->owner in the instance list.
    for ( entity_s *iterEntityInst = entityInsts.next; ; iterEntityInst = iterEntityInst->next )
    {
        iassert(iterEntityInst != &entityInsts);
        if ( iterEntityInst == sb->owner )
            break;
        ++*entOut;
    }

    // brush index = position of sb->def in the owner's DEF brush list.
    entity_s_def *def = (entity_s_def *)sb->owner->def;
    void         *defSentinel = (void *)&def->def;
    for ( brush_t *j = (brush_t *)def->brushes.prev; ; j = j->onext )
    {
        if ( (void *)j == defSentinel )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\win_dlg.cpp",
                    175, 0, "%s", "iterBrushDef != &(selectedBrushInst->owner->def->brushes)" );
        if ( j == sb->def )
            break;
        ++*brushOut;
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  Shared hand-built-popup helpers (the CMapInfo / CVehicleDlg pattern).
// ══════════════════════════════════════════════════════════════════════════════
static HFONT s_dlgFont = nullptr;

static HWND DLG_MakeChild( HWND parent, const char *cls, DWORD style, int id,
                           int x, int y, int w, int hgt, const char *text = nullptr )
{
    HWND h = CreateWindowExA( 0, cls, text, WS_CHILD | WS_VISIBLE | style,
                              x, y, w, hgt, parent, (HMENU)(INT_PTR)id,
                              AfxGetInstanceHandle(), NULL );
    if ( h && s_dlgFont )
        SendMessageA( h, WM_SETFONT, (WPARAM)s_dlgFont, 0 );
    return h;
}

// Create the popup (if needed) and show it; on re-show just foreground.  `make` builds a
// fresh instance, `existing` is the singleton slot.  Returns the live window (or null on
// failure).  The popup is sized to `w`×`h` near the main window.
static CWnd *DLG_ShowPopup( CWnd **existing, CWnd *(*make)(), const char *title, int w, int h )
{
    if ( *existing && ::IsWindow( (*existing)->GetSafeHwnd() ) )
    {
        (*existing)->ShowWindow( SW_SHOW );
        (*existing)->SetForegroundWindow();
        return *existing;
    }
    CWnd *parent = AfxGetMainWnd();
    CWnd *dlg = make();
    *existing = dlg;

    const DWORD style   = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    const DWORD exStyle = WS_EX_TOOLWINDOW;
    int px = 260, py = 180;
    if ( parent && parent->GetSafeHwnd() )
    {
        RECT r;  parent->GetWindowRect( &r );
        px = r.left + 180;  py = r.top + 130;
    }
    CRect rc( px, py, px + w, py + h );
    if ( !dlg->CreateEx( exStyle,
             AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW,
                 ::LoadCursor( NULL, IDC_ARROW ), (HBRUSH)( COLOR_BTNFACE + 1 ), NULL ),
             title, style, rc, parent, 0 ) )
    {
        delete dlg;
        *existing = nullptr;
        return nullptr;
    }
    g_nUpdateBits |= 1;
    return dlg;
}

// Common control ids (per-dialog ranges; our own — the binary's IDC_* don't exist here).
enum
{
    IDC_DLG_OK = 5300,        // the action button (Find / Go / OK)
    IDC_DLG_CANCEL,           // Cancel / Close
    IDC_FB_ENT,               // Find brush: entity # edit
    IDC_FB_BRUSH,             // Find brush: brush # edit
    IDC_GT_COORDS,            // Go to position: coords edit
    IDC_AR_X,                 // Arbitrary rotation: X angle edit
    IDC_AR_Y,
    IDC_AR_Z,
};

// ══════════════════════════════════════════════════════════════════════════════
//  CFindBrushDlg — Misc→Find brush (FindBrushDlgProc @0x4957E0).
// ══════════════════════════════════════════════════════════════════════════════
static CFindBrushDlg *g_dlgFindBrush = nullptr;
static HWND s_fbEnt   = nullptr;
static HWND s_fbBrush = nullptr;

BEGIN_MESSAGE_MAP( CFindBrushDlg, CWnd )
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_BN_CLICKED( IDC_DLG_OK,     &CFindBrushDlg::OnFind )
    ON_BN_CLICKED( IDC_DLG_CANCEL, &CFindBrushDlg::OnCancel2 )
END_MESSAGE_MAP()

CFindBrushDlg::CFindBrushDlg()
{
}

int CFindBrushDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;
    if ( !s_dlgFont )
        s_dlgFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

    HWND self = GetSafeHwnd();
    const int M = 12, lblW = 70, inW = 120, rowH = 26;
    int ix = M + lblW + 6;
    int y  = M;

    DLG_MakeChild( self, "static", SS_LEFT, 0, M, y + 4, lblW, 16, "Entity #:" );
    s_fbEnt = DLG_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER, IDC_FB_ENT, ix, y, inW, 22 );
    y += rowH;
    DLG_MakeChild( self, "static", SS_LEFT, 0, M, y + 4, lblW, 16, "Brush #:" );
    s_fbBrush = DLG_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER, IDC_FB_BRUSH, ix, y, inW, 22 );
    y += rowH + 8;

    int btnW = 80, btnGap = 12;
    DLG_MakeChild( self, "button", BS_DEFPUSHBUTTON | WS_TABSTOP, IDC_DLG_OK,     ix,                 y, btnW, 24, "Find" );
    DLG_MakeChild( self, "button", BS_PUSHBUTTON    | WS_TABSTOP, IDC_DLG_CANCEL, ix + btnW + btnGap, y, btnW, 24, "Cancel" );

    // Preload from the current selection (FindBrushDlgProc WM_INITDIALOG: GetSelectionIndex
    // → _itoa → SetWindowText; focus the entity edit).
    int entIdx = 0, brushIdx = 0;
    GetSelectionIndex( &entIdx, &brushIdx );
    char buf[32];
    _itoa( entIdx, buf, 10 );    if ( s_fbEnt )   ::SetWindowTextA( s_fbEnt, buf );
    _itoa( brushIdx, buf, 10 );  if ( s_fbBrush ) ::SetWindowTextA( s_fbBrush, buf );
    if ( s_fbEnt )
        ::SetFocus( s_fbEnt );
    return 0;
}

// FindBrushDlgProc OK (wParam==1): atol the brush# + ent# edits → Select_ByEntityNumber.
void CFindBrushDlg::OnFind()
{
    char entTxt[256] = { 0 }, brushTxt[256] = { 0 };
    if ( s_fbEnt )   ::GetWindowTextA( s_fbEnt,   entTxt,   sizeof( entTxt )   - 1 );
    if ( s_fbBrush ) ::GetWindowTextA( s_fbBrush, brushTxt, sizeof( brushTxt ) - 1 );
    int brushIdx = atol( brushTxt );
    int entIdx   = atol( entTxt );
    Select_ByEntityNumber( brushIdx, entIdx );
    ShowWindow( SW_HIDE );      // binary EndDialog(,1); modeless → hide
    g_nUpdateBits |= 1;
}

void CFindBrushDlg::OnCancel2() { ShowWindow( SW_HIDE ); }
void CFindBrushDlg::OnClose()   { ShowWindow( SW_HIDE ); }

void CFindBrushDlg::PostNcDestroy()
{
    g_dlgFindBrush = nullptr;
    s_fbEnt = s_fbBrush = nullptr;
    delete this;
}

void CFindBrushDlg::Show()
{
    DLG_ShowPopup( (CWnd **)&g_dlgFindBrush,
                   []() -> CWnd * { return new CFindBrushDlg(); },
                   "Find Brush", 260, 150 );
}

// ══════════════════════════════════════════════════════════════════════════════
//  CGoToDlg — Misc→Go to position (GoToDlgProc @0x495980).
// ══════════════════════════════════════════════════════════════════════════════
static CGoToDlg *g_dlgGoTo = nullptr;
static HWND s_gtCoords = nullptr;

BEGIN_MESSAGE_MAP( CGoToDlg, CWnd )
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_BN_CLICKED( IDC_DLG_OK,     &CGoToDlg::OnGo )
    ON_BN_CLICKED( IDC_DLG_CANCEL, &CGoToDlg::OnCancel2 )
END_MESSAGE_MAP()

CGoToDlg::CGoToDlg()
{
}

int CGoToDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;
    if ( !s_dlgFont )
        s_dlgFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

    HWND self = GetSafeHwnd();
    const int M = 12;
    int y = M;
    DLG_MakeChild( self, "static", SS_LEFT, 0, M, y, 300, 16, "Position (e.g.  256 256 16  or  (256, 256, 16)):" );
    y += 20;
    s_gtCoords = DLG_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL, IDC_GT_COORDS, M, y, 300, 22 );
    y += 30;

    int btnW = 80, btnGap = 12;
    int bx = M + 300 - ( 2 * btnW + btnGap );
    DLG_MakeChild( self, "button", BS_DEFPUSHBUTTON | WS_TABSTOP, IDC_DLG_OK,     bx,                 y, btnW, 24, "Go" );
    DLG_MakeChild( self, "button", BS_PUSHBUTTON    | WS_TABSTOP, IDC_DLG_CANCEL, bx + btnW + btnGap, y, btnW, 24, "Cancel" );

    if ( s_gtCoords )
        ::SetFocus( s_gtCoords );   // GoToDlgProc WM_INITDIALOG focuses the edit
    return 0;
}

// GoToDlgProc OK (a3==1): if the text is non-empty, zero camera origin then try the four
// coordinate formats in order (last 3-field match wins); Select_Deselect + PositionView.
void CGoToDlg::OnGo()
{
    char text[128] = { 0 };
    if ( s_gtCoords )
        ::GetWindowTextA( s_gtCoords, text, sizeof( text ) - 1 );

    if ( text[0] && g_pParentWnd )
    {
        CCamWnd *cam = g_pParentWnd->m_pCamWnd;
        if ( cam )
        {
            cam->camera.origin[0] = 0.0f;
            cam->camera.origin[1] = 0.0f;
            cam->camera.origin[2] = 0.0f;

            float x, yv, z;
            if ( sscanf( text, "%f, %f, %f", &x, &yv, &z ) == 3 )
            { cam->camera.origin[0] = x; cam->camera.origin[1] = yv; cam->camera.origin[2] = z; }
            if ( sscanf( text, "(%f, %f, %f)", &x, &yv, &z ) == 3 )
            { cam->camera.origin[0] = x; cam->camera.origin[1] = yv; cam->camera.origin[2] = z; }
            if ( sscanf( text, "%f %f %f", &x, &yv, &z ) == 3 )
            { cam->camera.origin[0] = x; cam->camera.origin[1] = yv; cam->camera.origin[2] = z; }
            if ( sscanf( text, "(%f %f %f)", &x, &yv, &z ) == 3 )
            { cam->camera.origin[0] = x; cam->camera.origin[1] = yv; cam->camera.origin[2] = z; }
        }
        Select_Deselect( 1 );
        if ( g_pParentWnd->m_pXYWnd )
            g_pParentWnd->m_pXYWnd->PositionView();
    }
    g_nUpdateBits |= 5;     // W_CAMERA | W_XY_OVERLAY
    ShowWindow( SW_HIDE );
}

void CGoToDlg::OnCancel2() { ShowWindow( SW_HIDE ); }
void CGoToDlg::OnClose()   { ShowWindow( SW_HIDE ); }

void CGoToDlg::PostNcDestroy()
{
    g_dlgGoTo = nullptr;
    s_gtCoords = nullptr;
    delete this;
}

void CGoToDlg::Show()
{
    DLG_ShowPopup( (CWnd **)&g_dlgGoTo,
                   []() -> CWnd * { return new CGoToDlg(); },
                   "Go to position", 324, 116 );
}

// ══════════════════════════════════════════════════════════════════════════════
//  CArbRotateDlg — Selection→Rotate→Arbitrary (CRotateDlg @0x450B10, OnOK = sub_450D50).
//  The CMainFrame handler (0x425300) undo-brackets around DoModal; since this popup is
//  modeless, the bracket lives in the OK handler instead (same captured state).
// ══════════════════════════════════════════════════════════════════════════════
static CArbRotateDlg *g_dlgArbRot = nullptr;
static HWND s_arX = nullptr;
static HWND s_arY = nullptr;
static HWND s_arZ = nullptr;

BEGIN_MESSAGE_MAP( CArbRotateDlg, CWnd )
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_BN_CLICKED( IDC_DLG_OK,     &CArbRotateDlg::OnApply )
    ON_BN_CLICKED( IDC_DLG_CANCEL, &CArbRotateDlg::OnCancel2 )
END_MESSAGE_MAP()

CArbRotateDlg::CArbRotateDlg()
{
}

int CArbRotateDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;
    if ( !s_dlgFont )
        s_dlgFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

    HWND self = GetSafeHwnd();
    const int M = 12, lblW = 90, inW = 110, rowH = 26;
    int ix = M + lblW + 6;
    int y  = M;

    struct { const char *lbl; HWND *out; int id; } rows[] = {
        { "X axis (deg):", &s_arX, IDC_AR_X },
        { "Y axis (deg):", &s_arY, IDC_AR_Y },
        { "Z axis (deg):", &s_arZ, IDC_AR_Z },
    };
    for ( int n = 0; n < 3; ++n )
    {
        DLG_MakeChild( self, "static", SS_LEFT, 0, M, y + 4, lblW, 16, rows[n].lbl );
        *rows[n].out = DLG_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL, rows[n].id, ix, y, inW, 22 );
        if ( *rows[n].out )
            ::SetWindowTextA( *rows[n].out, "0" );
        y += rowH;
    }
    y += 8;

    int btnW = 80, btnGap = 12;
    DLG_MakeChild( self, "button", BS_DEFPUSHBUTTON | WS_TABSTOP, IDC_DLG_OK,     ix,                 y, btnW, 24, "OK" );
    DLG_MakeChild( self, "button", BS_PUSHBUTTON    | WS_TABSTOP, IDC_DLG_CANCEL, ix + btnW + btnGap, y, btnW, 24, "Cancel" );

    if ( s_arX )
        ::SetFocus( s_arX );
    return 0;
}

// Apply one typed angle about `axis` to the selection (the Brush→Rotate pattern: pivot →
// matrix → apply).  Mirrors sub_450D50's per-axis block (`0.0 != atof(text)` gate).
static void ArbRot_ApplyAxis( int axis, float deg )
{
    if ( deg == 0.0f )
        return;
    float rot_around[4][3];
    Select_GetMid( rot_around[0] );
    Select_RotateAxis( axis, deg, (float (*)[4][3])rot_around );
    Select_ApplyMatrix_SelectedBrushes( 0, rot_around[0], deg, 0 );
    g_nUpdateBits = -1;
}

// CRotateDlg OnOK (sub_450D50): UpdateData(TRUE) then atof X/Y/Z → per-axis rotate.  The
// undo bracket (CMainFrame::OnSelectionArbitraryrotation 0x425300) is applied here.
void CArbRotateDlg::OnApply()
{
    if ( selected_brushes.next == &selected_brushes )
    {
        ShowWindow( SW_HIDE );
        return;
    }
    char xb[64] = { 0 }, yb[64] = { 0 }, zb[64] = { 0 };
    if ( s_arX ) ::GetWindowTextA( s_arX, xb, sizeof( xb ) - 1 );
    if ( s_arY ) ::GetWindowTextA( s_arY, yb, sizeof( yb ) - 1 );
    if ( s_arZ ) ::GetWindowTextA( s_arZ, zb, sizeof( zb ) - 1 );

    Undo_ClearRedo();
    Undo_GeneralStart( "arbitrary rotation" );
    Undo_AddBrushList( &selected_brushes );

    ArbRot_ApplyAxis( 0, (float)atof( xb ) );
    ArbRot_ApplyAxis( 1, (float)atof( yb ) );
    ArbRot_ApplyAxis( 2, (float)atof( zb ) );

    UpdateSelection( -1, 0 );
    Undo_EndBrushList( &selected_brushes );
    Undo_End();

    ShowWindow( SW_HIDE );
    g_nUpdateBits = -1;
}

void CArbRotateDlg::OnCancel2() { ShowWindow( SW_HIDE ); }
void CArbRotateDlg::OnClose()   { ShowWindow( SW_HIDE ); }

void CArbRotateDlg::PostNcDestroy()
{
    g_dlgArbRot = nullptr;
    s_arX = s_arY = s_arZ = nullptr;
    delete this;
}

void CArbRotateDlg::Show()
{
    DLG_ShowPopup( (CWnd **)&g_dlgArbRot,
                   []() -> CWnd * { return new CArbRotateDlg(); },
                   "Arbitrary Rotation", 290, 168 );
}

// ══════════════════════════════════════════════════════════════════════════════
//  CAboutDlg — Help→About (AboutDlgProc @0x496300).  The binary's DlgProc just shows
//  the box and EndDialogs on OK / WM_CLOSE; HAND-BUILT here as a modeless info popup.
// ══════════════════════════════════════════════════════════════════════════════
static CAboutDlg *g_dlgAbout = nullptr;

BEGIN_MESSAGE_MAP( CAboutDlg, CWnd )
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_BN_CLICKED( IDC_DLG_OK, &CAboutDlg::OnOk2 )
END_MESSAGE_MAP()

CAboutDlg::CAboutDlg()
{
}

int CAboutDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;
    if ( !s_dlgFont )
        s_dlgFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

    HWND self = GetSafeHwnd();
    const int M = 16;
    int y = M;
    DLG_MakeChild( self, "static", SS_LEFT, 0, M, y, 300, 40, "Kisak Radiant" );
    y += 22;
    DLG_MakeChild( self, "static", SS_LEFT, 0, M, y, 300, 32, "AI Decompiled and based on Call of Duty 4's map editor (CoD4Radiant)" );
    y += 32;
    DLG_MakeChild( self, "static", SS_LEFT, 0, M, y, 300, 32, "Special Thanks to xoxor4d for providing his IDA file, which had at least 60% of the functions named by hand!" );
    y += 64;

    int btnW = 80;
    DLG_MakeChild( self, "button", BS_DEFPUSHBUTTON | WS_TABSTOP, IDC_DLG_OK,
                   M + 300 - btnW, y, btnW, 24, "OK" );
    return 0;
}

void CAboutDlg::OnOk2()  { ShowWindow( SW_HIDE ); }    // binary EndDialog(,1)
void CAboutDlg::OnClose(){ ShowWindow( SW_HIDE ); }

void CAboutDlg::PostNcDestroy()
{
    g_dlgAbout = nullptr;
    delete this;
}

void CAboutDlg::Show()
{
    DLG_ShowPopup( (CWnd **)&g_dlgAbout,
                   []() -> CWnd * { return new CAboutDlg(); },
                   "About CoD4Radiant", 350, 225 );
}

// ══════════════════════════════════════════════════════════════════════════════
//  DoColor (0x499350) — the editor's colour picker, wired to every Colors-menu item
//  and the K-accelerator (Select Entity Color).  Seeds a CColorDialog from
//  g_qeglobals.d_savedinfo.colors[n] (float[0,1] RGB → COLORREF), pops it modal, and on
//  OK converts the chosen COLORREF back to float[0,1] into colors[n] (with alpha = 1).
//
//  The binary is textbook MFC — CColorDialog IS a ChooseColorA wrapper, so this is a
//  1:1 port of the common-dialog approach (not a hand-built template).  What is custom:
//  the editor keeps its OWN 16-entry custom-colour array (dword_240A1E0), separate from
//  MFC's built-in CColorDialog::GetSavedCustomColors(), persisted to the registry
//  "Custom Colors" section (values "0".."15").  The dialog's m_cc.lpCustColors is pointed
//  at the editor's array (the CMyColorDialog ctor's `a2[33] = dword_240A1E0`), so edits to
//  the 16 custom swatches round-trip through the editor's own persisted array.
//
//  §11: the float→byte seed is `(int)(colors[n][c] * 255.0)` — _ftol2 truncation, matched
//  by C++ (int) cast.  The byte→float write-back multiplies by 1/255.
// ══════════════════════════════════════════════════════════════════════════════

// The editor's 16 custom colours (IDB dword_240A1E0 = live, dword_240A1A0 = last-persisted
// baseline).  s_customColorsLoaded (IDB byte_739B0C, initial 1) forces a one-shot registry
// load on the first DoColor call.  0xFFFFFF is the "unset" sentinel (IDB unk_FFFFFF).
static COLORREF s_customColorsLive[16];      // dword_240A1E0
static COLORREF s_customColorsSaved[16];     // dword_240A1A0
static bool     s_customColorsLoaded = false;// !byte_739B0C (binary: byte_739B0C==1 → not loaded)

// sub_498F90 — first-use loader: read the 16 custom colours from the registry into both the
// live and the baseline arrays, then clear the "needs load" flag.
static void CustomColors_LoadFromRegistry()
{
    CWinApp *app = AfxGetApp();
    for ( int i = 0; i < 16; ++i )
    {
        CString name;
        name.Format( "%d", i );
        COLORREF c = (COLORREF)app->GetProfileInt( "Custom Colors", name, (int)0xFFFFFF );
        s_customColorsLive[i]  = c;
        s_customColorsSaved[i] = c;
    }
    s_customColorsLoaded = true;               // byte_739B0C = 0
}

// sub_499070 — after DoModal: write back any custom colours the user changed (live vs
// baseline diff) to the registry, updating the baseline.  0xFFFFFF entries are DELETED
// (WriteProfileString(...,NULL)) rather than written, matching the binary's unk_FFFFFF
// branch.  Runs regardless of OK/Cancel (the binary calls it before checking the result).
static void CustomColors_SaveToRegistry()
{
    CWinApp *app = AfxGetApp();
    for ( int i = 0; i < 16; ++i )
    {
        if ( s_customColorsLive[i] != s_customColorsSaved[i] )
        {
            CString name;
            name.Format( "%d", i );
            if ( s_customColorsLive[i] == (COLORREF)0xFFFFFF )
                app->WriteProfileString( "Custom Colors", name, NULL );
            else
                app->WriteProfileInt( "Custom Colors", name, (int)s_customColorsLive[i] );
            s_customColorsSaved[i] = s_customColorsLive[i];
        }
    }
}

int DoColor( int index )
{
    // Seed the dialog from colors[index] (float[0,1] → COLORREF bytes).  (int) casts
    // truncate exactly like the binary's _ftol2.
    int r = (int)( g_qeglobals.d_savedinfo.colors[index][0] * 255.0 );
    int g = (int)( g_qeglobals.d_savedinfo.colors[index][1] * 255.0 );
    int b = (int)( 255.0 * g_qeglobals.d_savedinfo.colors[index][2] );
    COLORREF seed = ( (unsigned __int8)r ) | ( (unsigned __int8)g << 8 ) | ( (unsigned __int8)b << 16 );

    // First use: pull the editor's 16 custom swatches from the registry (CMyColorDialog ctor
    // gate on byte_739B0C).
    if ( !s_customColorsLoaded )
        CustomColors_LoadFromRegistry();

    // CC_FULLOPEN | CC_RGBINIT (== 3): open expanded with the seed pre-selected.
    CColorDialog dlg( seed, CC_FULLOPEN | CC_RGBINIT, g_pParentWnd );
    // Point the dialog at the editor's own custom-colour array (CMyColorDialog: a2[33] =
    // dword_240A1E0), so edits round-trip through the editor's persisted swatches.
    dlg.m_cc.lpCustColors = s_customColorsLive;

    INT_PTR rc = dlg.DoModal();
    CustomColors_SaveToRegistry();             // sub_499070 — persist any edited swatches
    if ( rc != IDOK )
        return 0;

    // COLORREF back to float[0,1] (0.003921568859368563 == 1/255).
    COLORREF c = dlg.m_cc.rgbResult;
    g_qeglobals.d_savedinfo.colors[index][0] = (double)( c & 0xFF )          * 0.003921568859368563;
    g_qeglobals.d_savedinfo.colors[index][1] = (double)( ( c >> 8 ) & 0xFF ) * 0.003921568859368563;
    g_qeglobals.d_savedinfo.colors[index][2] = 0.003921568859368563 * (double)( ( c >> 16 ) & 0xFF );
    g_qeglobals.d_savedinfo.colors[index][3] = 1.0;
    g_nUpdateBits = -1;
    return 1;
}

