#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\layeredmaterialwnd.cpp
//
// The Layered-Material authoring window — a raw-Win32 tool palette (NOT an MFC
// CWnd/CDialog): a top-level WS_EX_PALETTEWINDOW frame ("LayeredMaterialWindow")
// hosting a COMCTL32 toolbar (5 command buttons) plus a custom-painted, scrolling
// "layer list" child ("LayeredMaterialList") that renders each layer's material
// thumbnail through the CoD D3D renderer (R_AddCmdDraw2DImage / R_AddCmdDrawText).
// Both windows use plain DefWindowProcA window procs (no MFC message maps).
//
// Layered-material authoring window: a raw Win32 palette with a toolbar and custom
// rendered layer list. Library writes occur only after its CRC changes.

#include "stdafx.h"
#include <commctrl.h>
#include "qe3.h"
#include "mainfrm.h"                  // CMainFrame (g_pParentWnd->OnKeyDown)
#include <gfx_d3d/r_material.h>       // Material
#include <gfx_d3d/r_init.h>           // R_SetupRendertarget_CheckDevice, R_CheckTargetWindow, R_Hwnd_Resize, R_InitRendererForWindow, R_SortMaterials
#include <gfx_d3d/r_rendercmds.h>     // R_BeginFrame/EndFrame, R_IssueRenderCommands, R_AddCmdClearScreen, R_AddCmdProjectionSet2D, R_AddCmdDraw2DImage, R_AddCmdDrawText, Font_s

// ── engine / cross-file deps ─────────────────────────────────────────────────
#include <qcommon/qcommon.h>          // Com_Error (errorParm_t), ERR_FATAL
extern int  Sys_Printf( const char *fmt, ... );
extern void Assert( const char *file, int line, int type, const char *fmt, ... );
extern char *va( const char *fmt, ... );

// Renderer entry points come from the gfx_d3d headers above (same set CCamWnd/CTexWnd::
// OnPaint use).  IDB→kisak name map for this file:
//   R_AddClearCmd      → R_AddCmdClearScreen
//   SetProjection2D    → R_AddCmdProjectionSet2D
//   R_GetFontHeight    → font->pixelHeight  (sub_5120A0 just returns it)
//   R_InitRendererForWindow takes HWND (not CWnd*) in kisak.

// Brush-list display rebuild.
extern void sub_47D060( int listHead );   // 0x47D060

// CMainFrame keyboard dispatch (mainfrm.cpp).
extern CMainFrame *g_pParentWnd;           // 0x25D5A70

// Global brush lists (qe3.cpp).
extern selbrush_t active_brushes;          // 0x23F189C
extern selbrush_t selected_brushes;        // 0x23F1864
extern selbrush_t filtered_brushes;        // 0x23F182C

// Window-update bit-mask (qe3 / Sys_UpdateWindows).
extern int g_nUpdateBits;                  // 0x25D5A74

extern BOOL SaveRegistryInfo( const char *pszName, void *pvBuf, int lSize );
extern BOOL LoadRegistryInfo( const char *pszName, void *pvBuf, long *plSize );

// texwnd "layered-material window active" flag (texwnd_s.unk_bool @0x10039) — set so
// the texture-window click-apply path can add a layer.  Accessor avoids exporting the
// TU-local texwnd_s.  (Currently no consumer reads it in kisak, but the write is faithful.)
extern "C" void TexWnd_SetLayeredMaterialActive( int active );   // texwnd.cpp

// Data layer (layeredmaterials.cpp).
extern unsigned int CheckLayeredMaterial_Modifications( uint8_t *a1, int a2, int a3 );
extern int     dword_1814CF8;                    // lyrMtlGlob_crcToken (clean-library CRC)
extern int     lyrMtlGlob_entryCount;            // 0x1814CFC
extern uint8_t lyrMtlGlob_Layers[];              // 0x1814D00 — 512 × 84 bytes
extern void    LayeredMaterials_AddEntries( char *name, HWND hWnd );   // 0x417050
extern void   *LayeredMaterials_texcoords( char *entry );              // 0x417190 (delete entry; ported)

// ═══════════════════════════════════════════════════════════════════════════════
//  lyrMtlWndGlob — the window's global state (IDB struct @ 0x181F500; type in
//  qe3.h — field names from the binary's assert strings).
// ═══════════════════════════════════════════════════════════════════════════════
LyrMtlWndGlob_t lyrMtlWndGlob = {};

// ── 84-byte library-entry view (the realised "activeLyrMtl" points into
//    lyrMtlGlob_Layers[]).  Same layout the data layer (layeredmaterials.cpp) uses via
//    its offset enum; named here for the window's field accesses.  Per the IDA cap
//    (LayeredMaterialWnd_RadMtl 0x4185C0) a library entry holds at most ONE layer, so
//    only layer[0] fits the 84-byte stride.
struct LyrMtlEntry
{
    char        name[64];      // 0x00
    int         nextId;        // 0x40  next layer id to allocate
    int         layerCount;    // 0x44  # layers (0 or 1)
    int         _pad48;        // 0x48
    int         layerId;       // 0x4C  layer[0].id
    qtexture_s *layerHandle;   // 0x50  layer[0].handle (radMtl)
};
static_assert( sizeof( LyrMtlEntry ) == 84, "LyrMtlEntry must be the 84-byte library entry" );
// The per-layer pair {id,handle} stride is 8 bytes; the window indexes layer i at
// (0x4C + 8*i, 0x50 + 8*i).  Helpers keep the byte arithmetic identical to the IDB.
static inline int  *EntryLayerId    ( LyrMtlEntry *e, int i ) { return (int *)( (char *)e + 0x4C + 8 * i ); }
static inline void **EntryLayerHandle( LyrMtlEntry *e, int i ) { return (void **)( (char *)e + 0x50 + 8 * i ); }
static inline LyrMtlEntry *ActiveEntry() { return (LyrMtlEntry *)(intptr_t)lyrMtlWndGlob.activeLyrMtl; }

// Forward decls (mutual references within this TU).
static LRESULT sub_417440();
static LRESULT sub_4174E0();
static int     sub_4173D0();
static BOOL    sub_417710();
static BOOL    LayeredMaterialWnd_Layer( unsigned int newLayerIndex );
static int     sub_417D60();
ATOM           LayeredMaterialWnd_PreCreateWindow();


// ═══════════════════════════════════════════════════════════════════════════════
//  CNameDlg — the "New Layered Material" name modal (IDB 0x435F10 + CNameDlg vftable).
//  A real CDialog subclass (template ID 0xB2 = 178) with two CString members.  The
//  binary inlines the MFC CString refcount dance + the no-op str_set helper; per the
//  established convention we use real CString members (RAII handles the release on all
//  return paths — exactly as Prefab_Load/Select_Move do).  m_result holds the entered
//  name; OnNewMaterial reads it after a non-cancel DoModal.
//  radiant.rc carries no IDD_NAME template, so (like CFindTextureDlg / CSurfaceDlg) the
//  dialog is built in code: a single edit + OK/Cancel.  The binary's resource is a one-
//  edit name prompt; we reproduce it with a runtime in-memory DLGTEMPLATE.
// ═══════════════════════════════════════════════════════════════════════════════
class CNameDlg : public CDialog
{
public:
    CString m_prompt;    // a1+116 — the prompt/initial text (binary stores Src here)
    CString m_result;    // a1+120 — the entered name (binary str_set "" then DoModal)
    CString m_title;     // window caption (the binary passes "New Layered Material")

    CNameDlg( const char *title, const char *src, CWnd *parent );
    INT_PTR DoModal() override;
    BOOL OnInitDialog() override;
    void OnOK() override;

    enum { IDC_NAMEDLG_EDIT = 110 };
};

// Build a runtime in-memory DLGTEMPLATE so DoModal works WITHOUT a .rc resource
// (radiant.rc carries only the menu/accelerators). This is the standard
// MFC InitModalIndirect path: one edit + OK + Cancel, exactly the binary's CNameDlg.
// All strings are stored as UTF-16 (dialog templates are always Unicode).
#pragma pack(push, 2)
struct DlgItemHdr { DWORD style; DWORD exStyle; short x, y, cx, cy; WORD id; };
#pragma pack(pop)

static void DT_PushWord( BYTE *&p, WORD w )            { *(WORD *)p = w; p += sizeof( WORD ); }
static void DT_PushDword( BYTE *&p, DWORD d )          { *(DWORD *)p = d; p += sizeof( DWORD ); }
static void DT_PushWStr( BYTE *&p, const char *s )     { while ( *s ) DT_PushWord( p, (WORD)(BYTE)*s++ ); DT_PushWord( p, 0 ); }
static void DT_Align4( BYTE *&p, BYTE *base )          { while ( ( ( p - base ) & 3 ) != 0 ) *p++ = 0; }

CNameDlg::CNameDlg( const char *title, const char *src, CWnd *parent )
    : CDialog()
    , m_prompt( src ? src : "" )      // binary str_set(m_prompt, Src)
    , m_result( "" )                  // binary str_set(m_result, "")
    , m_title( title ? title : "" )
{
    m_pParentWnd = parent;
}

INT_PTR CNameDlg::DoModal()
{
    // Assemble the template in a stack buffer.
    BYTE  buf[512];
    BYTE *p = buf;

    // DLGTEMPLATE header (DS_MODALFRAME | DS_SETFONT | WS_POPUP | WS_CAPTION | WS_SYSMENU).
    DT_PushDword( p, DS_MODALFRAME | DS_SETFONT | WS_POPUP | WS_CAPTION | WS_SYSMENU );
    DT_PushDword( p, 0 );                 // dwExtendedStyle
    DT_PushWord ( p, 3 );                 // cdit — 3 controls (edit, OK, Cancel)
    DT_PushWord ( p, 60 ); DT_PushWord( p, 60 );   // x, y (dialog units)
    DT_PushWord ( p, 180 ); DT_PushWord( p, 60 );  // cx, cy
    DT_PushWord ( p, 0 );                 // menu (none)
    DT_PushWord ( p, 0 );                 // window class (default)
    DT_PushWStr ( p, (const char *)m_title );      // title
    DT_PushWord ( p, 8 );                 // font point size (DS_SETFONT)
    DT_PushWStr ( p, "MS Shell Dlg" );    // font face

    // Item 1: the edit control.
    DT_Align4( p, buf );
    DT_PushDword( p, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL );
    DT_PushDword( p, 0 );
    DT_PushWord ( p, 8 ); DT_PushWord( p, 10 ); DT_PushWord( p, 164 ); DT_PushWord( p, 14 );
    DT_PushWord ( p, IDC_NAMEDLG_EDIT );
    DT_PushWord ( p, 0xFFFF ); DT_PushWord( p, 0x0081 );   // class atom = EDIT
    DT_PushWStr ( p, "" );                // no caption
    DT_PushWord ( p, 0 );                 // no creation data

    // Item 2: OK (default push button).
    DT_Align4( p, buf );
    DT_PushDword( p, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON );
    DT_PushDword( p, 0 );
    DT_PushWord ( p, 36 ); DT_PushWord( p, 34 ); DT_PushWord( p, 50 ); DT_PushWord( p, 14 );
    DT_PushWord ( p, IDOK );
    DT_PushWord ( p, 0xFFFF ); DT_PushWord( p, 0x0080 );   // class atom = BUTTON
    DT_PushWStr ( p, "OK" );
    DT_PushWord ( p, 0 );

    // Item 3: Cancel.
    DT_Align4( p, buf );
    DT_PushDword( p, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON );
    DT_PushDword( p, 0 );
    DT_PushWord ( p, 94 ); DT_PushWord( p, 34 ); DT_PushWord( p, 50 ); DT_PushWord( p, 14 );
    DT_PushWord ( p, IDCANCEL );
    DT_PushWord ( p, 0xFFFF ); DT_PushWord( p, 0x0080 );   // class atom = BUTTON
    DT_PushWStr ( p, "Cancel" );
    DT_PushWord ( p, 0 );

    if ( !InitModalIndirect( (LPCDLGTEMPLATE)buf, m_pParentWnd ) )
        return -1;
    return CDialog::DoModal();
}

BOOL CNameDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    SetDlgItemTextA( CNameDlg::IDC_NAMEDLG_EDIT, m_prompt );
    // Pre-select the seeded text so typing replaces it (the binary seeds m_prompt = Src).
    CEdit *edit = (CEdit *)GetDlgItem( CNameDlg::IDC_NAMEDLG_EDIT );
    if ( edit )
    {
        edit->SetFocus();
        edit->SetSel( 0, -1 );
    }
    return FALSE;   // we set focus explicitly
}

void CNameDlg::OnOK()
{
    CString s;
    GetDlgItemTextA( CNameDlg::IDC_NAMEDLG_EDIT, s );
    m_result = s;   // the binary copies the edit text into the result CString on OK
    CDialog::OnOK();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  LayeredMaterialWnd_Show  (0x4176A0)
// ═══════════════════════════════════════════════════════════════════════════════
BOOL LayeredMaterialWnd_Show()
{
    return ShowWindow( lyrMtlWndGlob.hwnd, SW_SHOW );
}

// ═══════════════════════════════════════════════════════════════════════════════
//  LayeredMaterialWnd_OnClose  (0x417680)
//  Un-toggle the "Live add" button (if armed), then hide the frame.
// ═══════════════════════════════════════════════════════════════════════════════
BOOL LayeredMaterialWnd_OnClose()
{
    if ( (BYTE)lyrMtlWndGlob.liveAddActive )
        sub_417440();
    return ShowWindow( lyrMtlWndGlob.hwnd, SW_HIDE );
}

// ═══════════════════════════════════════════════════════════════════════════════
//  LayeredMaterialWnd_ToggleVisibility  (0x4176B0)
//  (Identical body to CMainFrame::OnToggleLayeredMaterials 0x42BFE0.)
// ═══════════════════════════════════════════════════════════════════════════════
BOOL LayeredMaterialWnd_ToggleVisibility()
{
    if ( !IsWindowVisible( lyrMtlWndGlob.hwnd ) )
        return ShowWindow( lyrMtlWndGlob.hwnd, SW_SHOW );
    if ( (BYTE)lyrMtlWndGlob.liveAddActive )
        sub_417440();
    return ShowWindow( lyrMtlWndGlob.hwnd, SW_HIDE );
}

// ═══════════════════════════════════════════════════════════════════════════════
//  sub_4173D0  (0x4173D0) — recompute the layer-list vertical scrollbar extent.
//  Each layer row is 68px tall; nMax = 68 * layerCount (0 when no active material).
// ═══════════════════════════════════════════════════════════════════════════════
static int sub_4173D0()
{
    RECT rc;
    GetClientRect( lyrMtlWndGlob.layerList, &rc );

    int n = 0;
    if ( lyrMtlWndGlob.activeLyrMtl )
        n = ActiveEntry()->layerCount;

    SCROLLINFO si;
    si.nMax   = 68 * n;
    si.cbSize = sizeof( SCROLLINFO );   // 28
    si.fMask  = SIF_RANGE | SIF_PAGE;   // 3
    si.nMin   = 0;
    si.nPage  = rc.bottom - rc.top;
    return SetScrollInfo( lyrMtlWndGlob.layerList, SB_VERT, &si, TRUE );
}

// ═══════════════════════════════════════════════════════════════════════════════
//  sub_417440  (0x417440) — toggle the "Live add layer" toolbar button (cmd 0x88C4 =
//  35012) and mirror its checked state into the liveAddActive flag + texwnd flag.
//  Uses TB_GETSTATE (0x412) / TB_SETSTATE (0x411); flips TBSTATE bit 0 (CHECKED).
// ═══════════════════════════════════════════════════════════════════════════════
static LRESULT sub_417440()
{
    LRESULT st = SendMessageA( lyrMtlWndGlob.toolbar, TB_GETSTATE, 0x88C4u, 0 );
    if ( st != -1 )
    {
        char newChecked = (char)( st ^ 1 );
        SendMessageA( lyrMtlWndGlob.toolbar, TB_SETSTATE, 0x88C4u, (LPARAM)( st ^ 1 ) );
        *(BYTE *)&lyrMtlWndGlob.liveAddActive = (BYTE)( newChecked & 1 );
        TexWnd_SetLayeredMaterialActive( newChecked & 1 );
        return st;
    }
    return st;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  sub_4174E0  (0x4174E0) — sync the five toolbar buttons' state to the model.
//  Buttons 0x88C3/0x88C4 (New/Live) enabled iff an active material exists; 0x88C5
//  (Remove layer) iff the selected layer is in range; 0x88C6 (Down) iff a lower layer
//  exists; 0x88C7 (Up) iff a higher layer exists.  Each toggles TBSTATE_ENABLED (0x04).
// ═══════════════════════════════════════════════════════════════════════════════
static LRESULT sub_4174E0()
{
    int sel = lyrMtlWndGlob.selectedLayerIndex;
    int mtl = lyrMtlWndGlob.activeLyrMtl;
    int selCopy = sel;

    LRESULT v2 = SendMessageA( lyrMtlWndGlob.toolbar, TB_GETSTATE, 0x88C3u, 0 );
    if ( v2 != -1 && ( ( ( v2 & 4 ) != 0 ) != ( mtl != 0 ) ) )
        SendMessageA( lyrMtlWndGlob.toolbar, TB_SETSTATE, 0x88C3u, (LPARAM)( v2 ^ 4 ) );

    LRESULT v3 = SendMessageA( lyrMtlWndGlob.toolbar, TB_GETSTATE, 0x88C4u, 0 );
    if ( v3 != -1 && ( ( ( v3 & 4 ) != 0 ) != ( mtl != 0 ) ) )
        SendMessageA( lyrMtlWndGlob.toolbar, TB_SETSTATE, 0x88C4u, (LPARAM)( v3 ^ 4 ) );

    bool v4 = mtl && sel >= 0 && sel < ActiveEntry()->layerCount;
    LRESULT v5 = SendMessageA( lyrMtlWndGlob.toolbar, TB_GETSTATE, 0x88C5u, 0 );
    if ( v5 != -1 && ( ( ( v5 & 4 ) != 0 ) != v4 ) )
        SendMessageA( lyrMtlWndGlob.toolbar, TB_SETSTATE, 0x88C5u, (LPARAM)( v5 ^ 4 ) );

    bool v6 = mtl && ( selCopy + 1 ) < ActiveEntry()->layerCount;
    LRESULT v7 = SendMessageA( lyrMtlWndGlob.toolbar, TB_GETSTATE, 0x88C6u, 0 );
    if ( v7 != -1 && ( ( ( v7 & 4 ) != 0 ) != v6 ) )
        SendMessageA( lyrMtlWndGlob.toolbar, TB_SETSTATE, 0x88C6u, (LPARAM)( v7 ^ 4 ) );

    bool v8 = mtl && ( selCopy - 1 ) >= 0;
    LRESULT result = SendMessageA( lyrMtlWndGlob.toolbar, TB_GETSTATE, 0x88C7u, 0 );
    if ( result != -1 && ( ( ( result & 4 ) != 0 ) != v8 ) )
        return SendMessageA( lyrMtlWndGlob.toolbar, TB_SETSTATE, 0x88C7u, (LPARAM)( result ^ 4 ) );
    return result;
}

// Exported thin wrappers so other TUs (layeredmaterials.cpp / mainfrm.cpp) can reach the
// file-static helpers without seeing them: SyncToolbar = sub_4174E0 (button enable/check
// state); UntoggleLiveAdd = sub_417440 (clear the "Live add" toolbar toggle on hide/close).
extern "C" int LayeredMaterialWnd_SyncToolbar()   { return (int)sub_4174E0(); }
extern "C" int LayeredMaterialWnd_UntoggleLiveAdd() { return (int)sub_417440(); }

// ═══════════════════════════════════════════════════════════════════════════════
//  sub_417710  (0x417710) — rebuild the three brush-list displays then invalidate the
//  layer-list child (a layered-material change can affect drawn brushes).
// ═══════════════════════════════════════════════════════════════════════════════
static BOOL sub_417710()
{
    sub_47D060( (int)(intptr_t)&active_brushes );
    sub_47D060( (int)(intptr_t)&selected_brushes );
    sub_47D060( (int)(intptr_t)&filtered_brushes );
    sub_4174E0();
    BOOL result = InvalidateRect( lyrMtlWndGlob.layerList, nullptr, FALSE );
    g_nUpdateBits = -1;
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  LayeredMaterialWnd_OnNewMaterial  (0x417760) — the "New" command: pop the name
//  modal; on OK, add a new (empty) library entry named by the operator.
//  The binary attaches a temporary CWnd to the frame HWND as the modal's parent.
// ═══════════════════════════════════════════════════════════════════════════════
int LayeredMaterialWnd_OnNewMaterial()
{
    CWnd parent;
    parent.Attach( lyrMtlWndGlob.hwnd );

    int ret;
    {
        // The binary's CNameDlg(0x435F10) is constructed with the string "New Layered
        // Material" stored in m_prompt (a1+116) — used as the dialog caption/prompt; the
        // edit starts empty and m_result (a1+120) receives the entered name on OK.
        CNameDlg dlg( "New Layered Material", "", &parent );
        ret = (int)dlg.DoModal();
        parent.Detach();
        if ( ret == IDOK )
            LayeredMaterials_AddEntries( (char *)(LPCSTR)dlg.m_result, lyrMtlWndGlob.hwnd );
        // dlg destructor (the binary's sub_417820) releases the two CStrings (RAII).
    }
    return ret;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  LayeredMaterialWnd_Layer  (0x417940) — reorder: move the selected layer to
//  newLayerIndex by swapping the two {id,handle} pairs, then re-select newLayerIndex.
// ═══════════════════════════════════════════════════════════════════════════════
static BOOL LayeredMaterialWnd_Layer( unsigned int newLayerIndex )
{
    LyrMtlEntry *e = ActiveEntry();
    iassert( lyrMtlWndGlob.activeLyrMtl );   // LayeredMaterialWnd.cpp:200
    bcassert( newLayerIndex, (unsigned)e->layerCount );        // LayeredMaterialWnd.cpp:201
    int sel = lyrMtlWndGlob.selectedLayerIndex;
    bcassert( (unsigned)sel, (unsigned)e->layerCount );        // LayeredMaterialWnd.cpp:202

    // Swap layer[newLayerIndex] <-> layer[sel].
    int   tmpId     = *EntryLayerId( e, newLayerIndex );
    void *tmpHandle = *EntryLayerHandle( e, newLayerIndex );
    *EntryLayerId( e, newLayerIndex )     = *EntryLayerId( e, sel );
    *EntryLayerHandle( e, newLayerIndex ) = *EntryLayerHandle( e, sel );
    *EntryLayerId( ActiveEntry(), lyrMtlWndGlob.selectedLayerIndex )     = tmpId;
    *EntryLayerHandle( ActiveEntry(), lyrMtlWndGlob.selectedLayerIndex ) = tmpHandle;

    lyrMtlWndGlob.selectedLayerIndex = newLayerIndex;
    return sub_417710();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  LayeredMaterialWnd_Commands  (0x417A60) — toolbar / WM_COMMAND dispatch.
//    0 New, 1 Delete-material, 2 toggle-Live, 3 Remove-layer, 4 Down, 5 Up.
// ═══════════════════════════════════════════════════════════════════════════════
LRESULT LayeredMaterialWnd_Commands( int cmd )
{
    switch ( cmd )
    {
    case 0:
        return LayeredMaterialWnd_OnNewMaterial();

    case 1:   // Delete the active layered material.
        SetWindowTextA( lyrMtlWndGlob.hwnd, "(no layered material)" );
        // LayeredMaterials_texcoords (0x417190, layeredmaterials.cpp) — now a real port:
        // replaces the deleted material with "$default" on the live brushes (via the ported
        // FindReplaceTextures), then compacts the in-memory library.  Its only remaining
        // FATAL is the FindReplaceTextures flag&4 prefab-recursion branch, which fires ONLY
        // on an explicit operator delete with a REFERENCING prefab present (disk-mutating
        // Map_SaveFile of stock .maps — HARD RULE).  Faithful to the binary.
        LayeredMaterials_texcoords( (char *)(intptr_t)lyrMtlWndGlob.activeLyrMtl );
        lyrMtlWndGlob.activeLyrMtl = 0;
        return sub_417710();

    case 2:
        return sub_417440();

    case 3:   // Remove the selected layer (shift the rest down, shrink layerCount).
    {
        LyrMtlEntry *e = ActiveEntry();
        iassert( lyrMtlWndGlob.activeLyrMtl );   // LayeredMaterialWnd.cpp:180
        int sel = lyrMtlWndGlob.selectedLayerIndex;
        bcassert( (unsigned)sel, (unsigned)e->layerCount );    // LayeredMaterialWnd.cpp:181
        // memcpy( &layer[sel], &layer[sel+1], 8 * (--layerCount - sel) )
        int remaining = --e->layerCount - sel;
        memcpy( EntryLayerId( e, sel ), EntryLayerId( e, sel + 1 ), (size_t)8 * remaining );
        if ( lyrMtlWndGlob.selectedLayerIndex )
            --lyrMtlWndGlob.selectedLayerIndex;
        return sub_417710();
    }

    case 4:
        return LayeredMaterialWnd_Layer( lyrMtlWndGlob.selectedLayerIndex + 1 );

    case 5:
        return LayeredMaterialWnd_Layer( lyrMtlWndGlob.selectedLayerIndex - 1 );
    }
    return 0;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  sub_417AC0  (0x417AC0) — TTN_NEEDTEXT tooltip strings for the toolbar buttons.
// ═══════════════════════════════════════════════════════════════════════════════
static const char *sub_417AC0( int cmdId )
{
    switch ( cmdId )
    {
    case 35010: return "Create a new layered material";
    case 35011: return "Delete the selected layered material";
    case 35012: return "When checked, clicking in the texture window adds a layer";
    case 35013: return "Remove selected layer from the layered material";
    case 35014: return "Move selected layer up";
    case 35015: return "Move selected layer down";
    default:    return nullptr;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  LayeredMaterialWnd_SavePosition  (0x417B10)
// ═══════════════════════════════════════════════════════════════════════════════
static BOOL LayeredMaterialWnd_SavePosition( HWND hWnd )
{
    RECT rc;
    GetWindowRect( hWnd, &rc );
    return SaveRegistryInfo( "LayeredMaterialsRect", &rc, sizeof( rc ) );
}

// ═══════════════════════════════════════════════════════════════════════════════
//  sub_417B40  (0x417B40) — compute the layer-list child's rect (full client width,
//  below the toolbar).  Out: x=0, y=toolbarHeight, w=clientW, h=clientH-toolbarH.
// ═══════════════════════════════════════════════════════════════════════════════
static LONG sub_417B40( int *x, LONG *y, LONG *w, int *h )
{
    RECT toolbar, client;
    GetClientRect( lyrMtlWndGlob.hwnd, &client );
    GetWindowRect( lyrMtlWndGlob.toolbar, &toolbar );
    LONG toolbarH = toolbar.bottom - toolbar.top;
    *x = 0;
    *y = toolbarH;
    *w = client.right - client.left;
    *h = client.bottom - toolbarH - client.top;
    return toolbarH;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  LayeredMaterialWnd_SaveSize  (0x417BA0) — re-lay the list child and recompute
//  the scroll extent.
// ═══════════════════════════════════════════════════════════════════════════════
static int LayeredMaterialWnd_SaveSize()
{
    if ( lyrMtlWndGlob.layerList )
    {
        int  x, h;
        LONG y, w;
        sub_417B40( &x, &y, &w, &h );
        SetWindowPos( lyrMtlWndGlob.layerList, nullptr, x, y, (int)w, h, SWP_NOZORDER );
        RECT rc;
        GetWindowRect( lyrMtlWndGlob.hwnd, &rc );
        SaveRegistryInfo( "LayeredMaterialsRect", &rc, sizeof( rc ) );
        return sub_4173D0();
    }
    return 0;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  LayeredMaterialWnd_WindowProc  (0x417C20) — the FRAME window proc.
// ═══════════════════════════════════════════════════════════════════════════════
LRESULT CALLBACK LayeredMaterialWnd_WindowProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
    HWND hwnd = hWnd;   // the binary's param name (assert string)
    iassert( hwnd == lyrMtlWndGlob.hwnd || lyrMtlWndGlob.hwnd == NULL );   // LayeredMaterialWnd.cpp:330

    if ( Msg <= WM_NOTIFY )
    {
        switch ( Msg )
        {
        case WM_NOTIFY:
        {
            TOOLTIPTEXTA *tt = (TOOLTIPTEXTA *)lParam;
            if ( (int)tt->hdr.code == -520 )   // TTN_NEEDTEXTA
            {
                tt->lpszText = (char *)sub_417AC0( (int)tt->hdr.idFrom );
                return DefWindowProcA( hWnd, Msg, wParam, lParam );
            }
            break;
        }
        case WM_MOVE:
            LayeredMaterialWnd_SavePosition( hWnd );
            return DefWindowProcA( hWnd, WM_MOVE, wParam, lParam );
        case WM_SIZE:
            LayeredMaterialWnd_SaveSize();
            return DefWindowProcA( hWnd, WM_SIZE, wParam, lParam );
        case WM_CLOSE:
            LayeredMaterialWnd_OnClose();
            return 0;
        }
        return DefWindowProcA( hWnd, Msg, wParam, lParam );
    }

    if ( Msg != WM_KEYDOWN )
    {
        if ( Msg == WM_COMMAND )
        {
            unsigned int c = (unsigned __int16)wParam - 35010;
            if ( c <= 5 )
                LayeredMaterialWnd_Commands( c );
        }
        return DefWindowProcA( hWnd, Msg, wParam, lParam );
    }

    // WM_KEYDOWN: ESC closes; everything else routes to the main frame's accelerator map.
    if ( wParam == VK_ESCAPE )
    {
        LayeredMaterialWnd_OnClose();
        return 0;
    }
    if ( g_pParentWnd )
        g_pParentWnd->OnKeyDown( wParam, (unsigned __int16)lParam, HIWORD( lParam ) );
    return 0;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  sub_417D60  (0x417D60) — paint the layer list: for each layer (top-down) draw the
//  selected-row highlight band, the material thumbnail (aspect-fit into 64×64), and the
//  material name.  Pure D3D 2D immediate-mode draw (same primitives as CTexWnd).
// ═══════════════════════════════════════════════════════════════════════════════
static int sub_417D60()
{
    int result = 0;
    LyrMtlEntry *e = ActiveEntry();
    if ( !lyrMtlWndGlob.activeLyrMtl )
        return result;

    R_AddCmdProjectionSet2D();   // IDB SetProjection2D
    int baseY = 2 - GetScrollPos( lyrMtlWndGlob.layerList, SB_VERT );   // v2
    // IDB sub_5120A0 (R_GetFontHeight) just returns font->pixelHeight.
    int fontH = ( (Font_s *)g_qeglobals.d_font_list )->pixelHeight;    // result

    static const float s_white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    int v3 = e->layerCount - 1;       // walk layers bottom index → 0 (rows top-down)
    if ( v3 < 0 )
        return fontH;

    int textY = baseY + ( fontH + 64 ) / 2;   // v21 — name baseline for the bottom row
    int rowTop = baseY - 2;                    // v22 — highlight band top
    int v2 = baseY;

    for ( ; v3 >= 0; --v3 )
    {
        const float *textColor;
        if ( v3 == lyrMtlWndGlob.selectedLayerIndex )
        {
            RECT fr;
            GetClientRect( lyrMtlWndGlob.hwnd, &fr );
            float fw = (float)( fr.right - fr.left );
            R_AddCmdDraw2DImage( 0.0f, (float)rowTop, fw, 68.0f, 0.0f, 0.0f, 1.0f, 1.0f,
                                 g_qeglobals.d_savedinfo.colors[25], g_qeglobals.d_white );
            textColor = g_qeglobals.d_savedinfo.colors[26];
        }
        else
        {
            textColor = g_qeglobals.d_savedinfo.colors[8];
        }

        qtexture_s *q = (qtexture_s *)*EntryLayerHandle( e, v3 );   // v4
        int h = q->height;                            // v5 (= *(qtex+24))
        int w = q->width;                             // v24 (= *(qtex+20))
        int drawH = h;                                // v23
        // Aspect-fit the thumbnail into a 64×64 cell.
        if ( w >= h )
        {
            if ( w > 64 )
            {
                drawH = ( h << 6 ) / q->width;        // (h*64)/w
                w     = 64;
            }
        }
        else if ( h > 64 )
        {
            drawH = 64;
            w     = ( w << 6 ) / h;                   // (w*64)/h
        }

        Material *mtl = q->next;                      // v17 (Material* @ qtex+0)
        int imgX = ( 64 - w ) / 2 + 2;                // v23 (final)
        int imgY = v2 + ( 64 - drawH ) / 2;           // v23 (intermediate)
        R_AddCmdDraw2DImage( (float)imgX, (float)imgY, (float)w, (float)drawH,
                             0.0f, 0.0f, 1.0f, 1.0f, s_white, mtl );
        R_AddCmdDrawText( q->name, 64, (Font_s *)g_qeglobals.d_font_list, 68.0f, (float)textY,
                          1.0f, 1.0f, 0.0f, textColor, 0 );

        result = 68;
        rowTop += 68;
        textY  += 68;
        v2     += 68;
    }
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CLayermatWnd_OnPaint  (0x417F50) — list-child WM_PAINT: render one editor frame.
// ═══════════════════════════════════════════════════════════════════════════════
static int CLayermatWnd_OnPaint()
{
    PAINTSTRUCT ps;
    BeginPaint( lyrMtlWndGlob.layerList, &ps );
    if ( !R_SetupRendertarget_CheckDevice( lyrMtlWndGlob.layerList ) )
        return EndPaint( lyrMtlWndGlob.layerList, &ps );

    R_BeginFrame();
    R_AddCmdClearScreen( 7, g_qeglobals.d_savedinfo.colors[0], 1.0f, 0 );   // IDB R_AddClearCmd
    sub_417D60();
    R_EndFrame();
    R_IssueRenderCommands( (unsigned)-1 );
    R_SortMaterials();
    R_CheckTargetWindow( lyrMtlWndGlob.layerList );
    // The IDB resets the hunk-temp watermark here (CLayermatWnd_OnPaint tail, hunk_low
    // .temp = hunk_low.permanent).  R_IssueRenderCommands already drained the frame; the
    // editor renderer manages its own hunk reset inside the shared OnPaint path, so the
    // explicit reset is unnecessary in the port (matches CCamWnd/CTexWnd::OnPaint).
    return 0;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  CLayermatWnd_OnScroll  (0x418030) — list-child WM_VSCROLL.
//  nMax = thumbpos; a2 = SB_* request code.  Line = 68px; page = nPage.
// ═══════════════════════════════════════════════════════════════════════════════
static UINT CLayermatWnd_OnScroll( int pos, int code )
{
    SCROLLINFO si;
    si.cbSize = sizeof( SCROLLINFO );   // 28
    si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;   // 7
    BOOL got = GetScrollInfo( lyrMtlWndGlob.layerList, SB_VERT, &si );

    switch ( code )
    {
    case SB_LINEUP:    pos -= 68;          break;   // 0
    case SB_LINEDOWN:  pos += 68;          break;   // 1
    case SB_PAGEUP:    pos -= si.nPage;    break;   // 2
    case SB_PAGEDOWN:  pos += si.nPage;    break;   // 3
    case SB_THUMBTRACK:                    break;   // 5 (pos already the thumb pos)
    case SB_TOP:       pos  = 0;           break;   // 6
    case SB_BOTTOM:    pos  = si.nMax;     break;   // 7
    default:
        return got;
    }

    int maxPos = si.nMax - (int)si.nPage;
    if ( pos > maxPos ) pos = maxPos;
    if ( pos < 0 )      pos = 0;
    UINT result = (UINT)maxPos;
    if ( pos != si.nPos )
    {
        SetScrollPos( lyrMtlWndGlob.layerList, SB_VERT, pos, TRUE );
        result = (UINT)InvalidateRect( lyrMtlWndGlob.layerList, nullptr, FALSE );
    }
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  sub_4180E0  (0x4180E0) — list-child WM_LBUTTONDOWN: hit-test the y to a layer row
//  (66px pitch, 64px hot band) and select it.  Rows are drawn top-down, so the index
//  is mirrored: selected = layerCount - rowFromTop - 1.
// ═══════════════════════════════════════════════════════════════════════════════
static int sub_4180E0( int y )
{
    int v1 = GetScrollPos( lyrMtlWndGlob.layerList, SB_VERT ) + y;
    int row = ( v1 - 2 ) / 66;
    if ( row >= 0 )
    {
        int n = ActiveEntry()->layerCount;
        if ( row < n && ( v1 - 2 ) % 66 < 64 )
        {
            lyrMtlWndGlob.selectedLayerIndex = n - row - 1;
            sub_4174E0();
            int result = InvalidateRect( lyrMtlWndGlob.layerList, nullptr, FALSE );
            g_nUpdateBits |= 0x10u;   // W_TEXTURE
            return result;
        }
    }
    return v1;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  LayeredMaterialWnd_WindowProcA  (0x418140) — the layer-LIST child window proc.
// ═══════════════════════════════════════════════════════════════════════════════
LRESULT CALLBACK LayeredMaterialWnd_WindowProcA( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
    HWND hwnd = hWnd;   // the binary's param name (assert string)
    iassert( hwnd == lyrMtlWndGlob.layerList || lyrMtlWndGlob.layerList == NULL );   // LayeredMaterialWnd.cpp:528

    if ( Msg <= WM_VSCROLL )
    {
        switch ( Msg )
        {
        case WM_VSCROLL:
            CLayermatWnd_OnScroll( HIWORD( wParam ), (unsigned __int16)wParam );
            return 0;
        case WM_SIZE:
            R_Hwnd_Resize( (HWND__ *)hWnd, (unsigned __int16)lParam, HIWORD( lParam ) );
            return 0;
        case WM_PAINT:
            CLayermatWnd_OnPaint();
            return 0;
        }
        return DefWindowProcA( hWnd, Msg, wParam, lParam );
    }

    if ( Msg != WM_LBUTTONDOWN )
        return DefWindowProcA( hWnd, Msg, wParam, lParam );

    if ( lyrMtlWndGlob.activeLyrMtl )
        sub_4180E0( (short)HIWORD( lParam ) );   // y = GET_Y_LPARAM(lParam)
    return 0;
}

// ═══════════════════════════════════════════════════════════════════════════════
//  LayeredMaterialWnd_PreCreateWindow  (0x418210) — register both window classes.
// ═══════════════════════════════════════════════════════════════════════════════
ATOM LayeredMaterialWnd_PreCreateWindow()
{
    WNDCLASSEXA wc;
    memset( &wc, 0, sizeof( wc ) );
    wc.cbSize        = sizeof( wc );          // 48
    wc.style         = CS_DBLCLKS;            // 8
    wc.lpfnWndProc   = LayeredMaterialWnd_WindowProc;
    wc.hInstance     = GetModuleHandleA( nullptr );
    wc.hCursor       = LoadCursorA( nullptr, (LPCSTR)IDC_ARROW );   // 0x7F00
    wc.hbrBackground = (HBRUSH)( COLOR_BTNTEXT );                    // 16
    wc.lpszClassName = "LayeredMaterialWindow";
    RegisterClassExA( &wc );

    wc.lpfnWndProc   = LayeredMaterialWnd_WindowProcA;
    wc.lpszClassName = "LayeredMaterialList";
    return RegisterClassExA( &wc );
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Create_ToolBar  (0x4182E0) — create the COMCTL32 toolbar with the 5 command buttons.
//  The binary builds the TBBUTTON array from a control-byte string and TB_ADDBUTTONS;
//  we build an explicit TBBUTTON[6] (5 buttons + a separator placeholder) for clarity —
//  the command IDs (35010..35015) and the TB_ message sequence are byte-faithful.
// ═══════════════════════════════════════════════════════════════════════════════
static LRESULT Create_ToolBar()
{
    HMODULE hInst = GetModuleHandleA( nullptr );
    HWND toolbar = CreateWindowExA( 0, TOOLBARCLASSNAMEA, nullptr,
                                    WS_CHILD | WS_VISIBLE | WS_BORDER | CCS_NORESIZE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT,
                                    0, 0, 168, 23, lyrMtlWndGlob.hwnd, nullptr, hInst, nullptr );
    lyrMtlWndGlob.toolbar = toolbar;
    if ( !toolbar )
        Com_Error( ERR_FATAL, "%s", "Couldn't create a toolbar; you may need a newer version of Internet Explorer" );

    SendMessageA( toolbar, TB_BUTTONSTRUCTSIZE, sizeof( TBBUTTON ), 0 );   // 0x41E, wParam=20
    SendMessageA( toolbar, TB_SETBITMAPSIZE, 0, 0x000F0010 );              // 0x420, lParam=983056 = 16×15

    // Six button command ids 35010..35015 (the binary's control string is 6 entries:
    // {35010..35015}).  Image index 0 for each; separators get TBSTATE_WRAP and no cmd.
    static const int s_cmd[6] = { 35010, 35011, 35012, 35013, 35014, 35015 };
    TBBUTTON buttons[6];
    memset( buttons, 0, sizeof( buttons ) );
    for ( int i = 0; i < 6; ++i )
    {
        buttons[i].iBitmap   = 0;
        buttons[i].idCommand = s_cmd[i];
        buttons[i].fsState   = TBSTATE_ENABLED;   // 4
        buttons[i].fsStyle   = TBSTYLE_BUTTON;    // 0
    }
    SendMessageA( toolbar, TB_ADDBUTTONS, 6, (LPARAM)buttons );            // 0x414

    return sub_4174E0();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Create_LayerList  (0x418440) — create the custom-painted list child below the toolbar.
// ═══════════════════════════════════════════════════════════════════════════════
static int Create_LayerList()
{
    RECT toolbar, client;
    GetClientRect( lyrMtlWndGlob.hwnd, &client );
    GetWindowRect( lyrMtlWndGlob.toolbar, &toolbar );
    int toolbarH = toolbar.bottom - toolbar.top;
    lyrMtlWndGlob.layerList = CreateWindowExA( 0, "LayeredMaterialList", nullptr,
                                              WS_CHILD | WS_VISIBLE | WS_VSCROLL,
                                              0, toolbarH,
                                              client.right - client.left,
                                              client.bottom - client.top - toolbarH,
                                              lyrMtlWndGlob.hwnd, nullptr, nullptr, nullptr );
    if ( !lyrMtlWndGlob.layerList )
        Com_Error( ERR_FATAL, "%s", "Couldn't create the material layer list" );
    return sub_4173D0();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  Create_LayerdMaterialWnd  (0x4184C0) — create the frame, toolbar, and list.
//
//  Integration: this is operator-attended.  It is NOT auto-invoked at startup because
//  the editor shell (mainfrm.cpp OnCreateClient) already attaches lyrMtlWndGlob.layerList
//  to the renderer via a hidden placeholder pane, and replacing that with this real
//  frame's list child changes the gate-critical renderer multi-window attach.  An
//  attended session can call this (e.g. lazily on the first F4 toggle) once the attach
//  hand-off is verified on a live desktop.
// ═══════════════════════════════════════════════════════════════════════════════
int Create_LayerdMaterialWnd()
{
    lyrMtlWndGlob.hwnd            = nullptr;
    lyrMtlWndGlob.toolbar = nullptr;
    lyrMtlWndGlob.layerList          = nullptr;
    lyrMtlWndGlob.liveAddActive                   = 0;
    lyrMtlWndGlob.activeLyrMtl = 0;
    lyrMtlWndGlob.selectedLayerIndex  = 0;

    LayeredMaterialWnd_PreCreateWindow();

    int x = 100, y = 100, w = 150, h = 400;
    RECT savedRect;
    long savedSize = sizeof( savedRect );
    if ( LoadRegistryInfo( "LayeredMaterialsRect", &savedRect, &savedSize )
         && savedSize == sizeof( savedRect ) )
    {
        x = savedRect.left;
        y = savedRect.top;
        w = savedRect.right - savedRect.left;
        h = savedRect.bottom - savedRect.top;
    }

    lyrMtlWndGlob.hwnd = CreateWindowExA( WS_EX_PALETTEWINDOW, "LayeredMaterialWindow",
                                            "(no layered material)",
                                            WS_THICKFRAME | WS_SYSMENU | WS_DLGFRAME | WS_BORDER | WS_POPUP,
                                            x, y, w, h, nullptr, nullptr, nullptr, nullptr );
    if ( !lyrMtlWndGlob.hwnd )
        Com_Error( ERR_FATAL, "%s", "Couldn't create layered material window." );

    Create_ToolBar();
    return Create_LayerList();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  LayeredMaterialWnd_InitRenderer  (0x418580) — attach the CoD renderer to the
//  layer-list child.  R_BeginRegistrationInternal (gfxwrapper.cpp) calls this.
// ═══════════════════════════════════════════════════════════════════════════════
char LayeredMaterialWnd_InitRenderer()
{
    iassert( lyrMtlWndGlob.layerList );   // LayeredMaterialWnd.cpp:726
    return R_InitRendererForWindow( lyrMtlWndGlob.layerList );
}

// ═══════════════════════════════════════════════════════════════════════════════
//  LayeredMaterialWnd_RadMtl  (0x4185C0) — add a radMtl (texture-window material) as a
//  new layer of the active layered material.  Rejects: no active material, layer cap
//  (1), or a duplicate of an existing layer.  On success, appends the layer (id = nextId++)
//  and rebuilds.  This is the texture-window click-apply target when "Live add" is armed.
// ═══════════════════════════════════════════════════════════════════════════════
LRESULT LayeredMaterialWnd_RadMtl( qtexture_s *radMtl )
{
    iassert( radMtl );   // LayeredMaterialWnd.cpp:736

    LyrMtlEntry *lyrMtl = ActiveEntry();   // the binary's local name
    iassert( lyrMtl );   // LayeredMaterialWnd.cpp:739

    int count = lyrMtl->layerCount;
    if ( count == 1 )
        return Sys_Printf( "Cannot have more than %i layers in a layered material.\n", 1 );

    // Duplicate check: scan existing layers for the same radMtl handle.
    if ( count > 0 )
    {
        for ( int i = 0; i < count; ++i )
        {
            if ( *EntryLayerHandle( lyrMtl, i ) == radMtl )
                return Sys_Printf( "Material '%s' is already a layer in '%s'.\n",
                                   radMtl->name, lyrMtl->name );
        }
    }

    // Append the new layer: handle then id (= nextId, post-incremented).
    *EntryLayerHandle( lyrMtl, count ) = radMtl;
    *EntryLayerId( lyrMtl, lyrMtl->layerCount++ ) = lyrMtl->nextId++;

    sub_4173D0();
    sub_47D060( (int)(intptr_t)&active_brushes );
    sub_47D060( (int)(intptr_t)&selected_brushes );
    sub_47D060( (int)(intptr_t)&filtered_brushes );
    sub_4174E0();
    LRESULT result = InvalidateRect( lyrMtlWndGlob.layerList, nullptr, FALSE );
    g_nUpdateBits = -1;
    return result;
}
