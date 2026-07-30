#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Layer-management UI over layers.cpp. The original tree resource is unavailable,
// so the port uses a flat modeless list while preserving full layer paths and commands.

#include "stdafx.h"
#include "qe3.h"
#include "mainfrm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <utility>

extern void       Select_BrushByLayer( char *layer_str );    // select.cpp 0x48EE10
extern void       Brush_SetInstanceLayerString( selbrush_t *inst, const char *str ); // brush.cpp 0x4758E0
extern int        g_nUpdateBits;                              // engine_stubs 0x25D5A74
extern void       Assert( const char *file, int line, int type, const char *fmt, ... );
extern BOOL       LoadRegistryInfo( const char *pszName, void *pvBuf, long *plSize ); // win_qe3.cpp 0x4999C0
void              LayersDlg_RefreshIfOpen();                  // below (dialog refresh hook)

// ══════════════════════════════════════════════════════════════════════════════
//  CORE OPERATIONS (UI-independent — the gate exercises these directly)
// ══════════════════════════════════════════════════════════════════════════════

// 0x41C850  CLayersDlg::AssignSelectionToLayer — assign every selected brush to
// `layerName`.  IDA: walk selected_brushes; free def->parent_layer_string; dup the
// layer string into it; reset the instance's xx7 faceVis flag.  We route through
// Brush_SetInstanceLayerString (sub_4758E0), which does exactly that.
void Layers_AssignSelectionToLayer( const char *layerName )
{
    for ( selbrush_t *i = selected_brushes.next; i != &selected_brushes; i = i->next )
    {
        // (the binary inlines Brush_SetInstanceLayerString here; the helper carries the
        // brush.cpp:2365/2354 checks itself)
        Brush_SetInstanceLayerString( i, layerName );   // free old + dup new + xx7=0
    }
    g_nUpdateBits = -1;
}

// 0x41CA60..0x41CB80  the Hide/Show layer handlers (via sub_41D0F0): set the layer's
// flag in the map (layers_01/layers_02) AND toggle its runtime FilterBrush
// visibility.  `hidden` true → hide.  Returns false if the layer doesn't exist.
bool Layers_SetHidden( const char *layerName, bool hidden )
{
    if ( !Layers_Exists( layerName ) )
        return false;

    // (1) the persisted layer flag (round-trips in the .map "flags" line).
    // sub_41D0F0 (hide)  = Layers_ClearFlag(name, 8) then Layers_SetFlag(name, 1)
    // sub_41D270 (show)  = Layers_ClearFlag(name, 9)   ← clears hidden AND frozen
    // i.e. hiding UNFREEZES and showing UNFREEZES — the tree's three visual states
    // (0x1000 shown / 0x2000 hidden / 0x3000 frozen) are mutually exclusive.
    // [audit U7: the port previously only touched bit 1, so a frozen layer stayed
    //  frozen after Show and Hide+Show could not clear a freeze.]
    if ( hidden )
    {
        Layers_ClearFlag( layerName, 8 );
        Layers_SetFlag( layerName, 1 );
    }
    else
    {
        Layers_ClearFlag( layerName, 9 );
    }

    // (2) the runtime visibility filter (drives FilterBrush → views).  Rebuild the
    // script_layer filter set first so a freshly-relevant layer has an entry, then
    // flip its isShown.  (No-op when the layer carries no script_layer brushes —
    // matches the binary, where only layers present in the CCheckListBox toggle.)
    Layers_RebuildVisibilityFilters();
    Layers_SetLayerVisible( layerName, !hidden );
    return true;
}

// 0x41CB20 / 0x41CB50 via sub_41D1B0 — FREEZE a layer.  The binary's tree helper sets
// the item state image to 0x3000 then Layers_ClearFlag(name,1) + Layers_SetFlag(name,8):
// freezing implies NOT hidden (the three states are exclusive, see Layers_SetHidden).
// There is no separate "unfreeze" command in MENU233 — Show Layer clears bit 8 too.
bool Layers_SetFrozen( const char *layerName )
{
    if ( !Layers_Exists( layerName ) )
        return false;
    Layers_ClearFlag( layerName, 1 );
    Layers_SetFlag( layerName, 8 );
    // Frozen layers still DRAW (they are just not selectable — Layers_KeyIsFrozen in
    // layers.cpp drives that), so unlike Hide there is no visibility-filter flip here.
    return true;
}

// 0x41CB80 / 0x41CBB0 via sub_41D3D0 / sub_41D320 — collapse / expand a layer's subtree.
// The binary does TVM_EXPAND on the CTreeCtrl item (guarded by a re-entrancy byte) and
// then Layers_ClearFlag(name,4) / Layers_SetFlag(name,4) — the "expanded" bit that
// round-trips in the .map layer flags.
// UI REDUCTION (documented): this port's layers pane is a FLAT CListBox (see the file
// header), so there is no tree node to expand or collapse.  The BACKEND flag write is
// reproduced faithfully — the expanded state persists through save/load exactly as the
// binary's does — but nothing visually folds.  Both binary handlers pass bRecurse = 1,
// so the flag is written for the layer AND every descendant.
bool Layers_SetExpanded( const char *layerName, bool expanded )
{
    if ( !Layers_Exists( layerName ) )
        return false;
    if ( expanded ) Layers_SetFlag( layerName, 4 );
    else            Layers_ClearFlag( layerName, 4 );
    return true;
}

// The "and children" recursion.  The binary walks the CTreeCtrl (TVGN_CHILD/TVGN_NEXT);
// here the hierarchy lives in the layer PATH, so a child of "a/b" is any layer whose
// full path starts with "a/b/".  Applies `fn` to `layerName` first, then to every
// descendant — same pre-order as sub_41D0F0/1B0/270/320/3D0.
static void Layers_ForSelfAndChildren( const char *layerName, bool recurse,
                                       void ( *fn )( const char * ) )
{
    fn( layerName );
    if ( !recurse )
        return;

    char prefix[1100];
    _snprintf( prefix, sizeof( prefix ) - 1, "%s/", layerName );
    prefix[sizeof( prefix ) - 1] = '\0';
    const size_t plen = strlen( prefix );

    std::vector<std::pair<std::string,int>> layers;
    Layers_Enumerate( layers );
    for ( const auto &kv : layers )
    {
        if ( kv.first.size() > plen && !strncmp( kv.first.c_str(), prefix, plen ) )
            fn( kv.first.c_str() );
    }
}

static void LY_ApplyHide( const char *n )     { Layers_SetHidden( n, true ); }
static void LY_ApplyShow( const char *n )     { Layers_SetHidden( n, false ); }
static void LY_ApplyFreeze( const char *n )   { Layers_SetFrozen( n ); }
static void LY_ApplyCollapse( const char *n ) { Layers_SetExpanded( n, false ); }
static void LY_ApplyExpand( const char *n )   { Layers_SetExpanded( n, true ); }

// ══════════════════════════════════════════════════════════════════════════════
//  CLayerDlg — the hand-built modeless popup (CSurfaceDlg / CFindTextureDlg pattern)
// ══════════════════════════════════════════════════════════════════════════════

enum
{
    IDC_LY_LIST   = 3000,   // the layer list (flat; the full "a/b/c" path is the text)
    IDC_LY_NEW,             // New
    IDC_LY_DELETE,          // Delete
    IDC_LY_RENAME,          // Rename
    IDC_LY_ASSIGN,          // Assign selection → layer
    IDC_LY_SELECT,          // Select all in layer
    IDC_LY_ACTIVE,          // Make active
    IDC_LY_HIDE,            // Hide
    IDC_LY_SHOW,            // Show
    IDC_LY_NAMEEDIT,        // the name edit (for New/Rename)
    IDC_LY_LBL_ACTIVE,      // "Active: <name>" status label
};

CLayerDlg *g_dlgLayers = nullptr;       // the singleton (NULL until first opened)

static HFONT s_lyFont   = nullptr;
static HWND  s_lyList   = nullptr;
static HWND  s_lyName   = nullptr;
static HWND  s_lyActive = nullptr;

static HWND LY_MakeChild( HWND parent, const char *cls, DWORD style, int id,
                          int x, int y, int w, int hgt, const char *text = nullptr )
{
    HWND h = CreateWindowExA( 0, cls, text, WS_CHILD | WS_VISIBLE | style,
                              x, y, w, hgt, parent, (HMENU)(INT_PTR)id,
                              AfxGetInstanceHandle(), NULL );
    if ( h && s_lyFont )
        SendMessageA( h, WM_SETFONT, (WPARAM)s_lyFont, 0 );
    return h;
}

static inline void MoveWindow_( HWND h, int x, int y, int w, int hgt )
{
    if ( h )
        ::MoveWindow( h, x, y, w, hgt, TRUE );
}

BEGIN_MESSAGE_MAP( CLayerDlg, CWnd )
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_CLOSE()
    ON_WM_DESTROY()
    ON_BN_CLICKED( IDC_LY_NEW,    &CLayerDlg::OnNew )
    ON_BN_CLICKED( IDC_LY_DELETE, &CLayerDlg::OnDelete )
    ON_BN_CLICKED( IDC_LY_RENAME, &CLayerDlg::OnRename )
    ON_BN_CLICKED( IDC_LY_ASSIGN, &CLayerDlg::OnAssign )
    ON_BN_CLICKED( IDC_LY_SELECT, &CLayerDlg::OnSelectAll )
    ON_BN_CLICKED( IDC_LY_ACTIVE, &CLayerDlg::OnMakeActive )
    ON_BN_CLICKED( IDC_LY_HIDE,   &CLayerDlg::OnHide )
    ON_BN_CLICKED( IDC_LY_SHOW,   &CLayerDlg::OnShow )
    ON_WM_CONTEXTMENU()           // MENU233 right-click menu (CLayerDlg::ContextMenu 0x41DEF0)
END_MESSAGE_MAP()

CLayerDlg::CLayerDlg()
{
}

// Repopulate the listbox from the layerMap (sorted), marking flags + active layer.
// (This is the role of CLayerDlg's Layers_02 → tree refresh + maybeLayers, collapsed
//  onto a flat listbox; the on-screen string carries the flag annotations.)
void CLayerDlg::Refresh()
{
    if ( !s_lyList )
        return;
    int prevSel = (int)::SendMessageA( s_lyList, LB_GETCURSEL, 0, 0 );
    ::SendMessageA( s_lyList, LB_RESETCONTENT, 0, 0 );

    std::vector<std::pair<std::string,int>> layers;
    Layers_Enumerate( layers );
    const char *active = Layers_GetActive();
    for ( const auto &kv : layers )
    {
        char line[1100];
        int flags = kv.second & ~0x10;
        char tag[64] = { 0 };
        if ( flags & 1 ) strcat( tag, " [hidden]" );
        if ( flags & 2 ) strcat( tag, " [prefab]" );
        if ( flags & 8 ) strcat( tag, " [frozen]" );
        bool isActive = !strcmp( active, kv.first.c_str() );
        _snprintf( line, sizeof( line ) - 1, "%s%s%s",
                   isActive ? "* " : "  ", kv.first.c_str(), tag );
        line[sizeof( line ) - 1] = '\0';
        ::SendMessageA( s_lyList, LB_ADDSTRING, 0, (LPARAM)line );
    }
    if ( prevSel >= 0 )
        ::SendMessageA( s_lyList, LB_SETCURSEL, prevSel, 0 );

    if ( s_lyActive )
    {
        char buf[300];
        _snprintf( buf, sizeof( buf ) - 1, "Active: %s", active );
        buf[sizeof( buf ) - 1] = '\0';
        ::SetWindowTextA( s_lyActive, buf );
    }
}

// Return the picked layer's bare name (strips the "* "/"  " prefix + the flag tags).
// Empty string if nothing is selected.
static bool LY_SelectedName( char *out, int outSz )
{
    out[0] = '\0';
    if ( !s_lyList ) return false;
    int sel = (int)::SendMessageA( s_lyList, LB_GETCURSEL, 0, 0 );
    if ( sel < 0 ) return false;
    char line[1100];
    int len = (int)::SendMessageA( s_lyList, LB_GETTEXT, sel, (LPARAM)line );
    if ( len <= 0 ) return false;
    // Skip the 2-char active marker.
    const char *p = line;
    if ( p[0] && p[1] ) p += 2;
    // Truncate at the first " [" (flag tag).
    char tmp[1100];
    strncpy( tmp, p, sizeof( tmp ) - 1 );
    tmp[sizeof( tmp ) - 1] = '\0';
    char *tag = strstr( tmp, " [" );
    if ( tag ) *tag = '\0';
    strncpy( out, tmp, outSz - 1 );
    out[outSz - 1] = '\0';
    return out[0] != '\0';
}

int CLayerDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;
    if ( !s_lyFont )
        s_lyFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

    HWND self = GetSafeHwnd();
    const int M = 8, listW = 240, listH = 240, btnW = 90, btnH = 24;
    int bx = M + listW + 10;
    int y  = M;

    LY_MakeChild( self, "static", SS_LEFT, IDC_LY_LBL_ACTIVE - 1, M, y, listW, 16, "Layers:" );
    y += 18;
    s_lyList = LY_MakeChild( self, "listbox",
                             WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_HASSTRINGS,
                             IDC_LY_LIST, M, y, listW, listH );

    int by = M + 18;
    LY_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_LY_NEW,    bx, by, btnW, btnH, "New" );        by += 28;
    LY_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_LY_DELETE, bx, by, btnW, btnH, "Delete" );     by += 28;
    LY_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_LY_RENAME, bx, by, btnW, btnH, "Rename" );     by += 28;
    LY_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_LY_ASSIGN, bx, by, btnW, btnH, "Assign Sel" ); by += 28;
    LY_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_LY_SELECT, bx, by, btnW, btnH, "Select All" ); by += 28;
    LY_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_LY_ACTIVE, bx, by, btnW, btnH, "Make Active" );by += 28;
    LY_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_LY_HIDE,   bx, by, btnW, btnH, "Hide" );       by += 28;
    LY_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_LY_SHOW,   bx, by, btnW, btnH, "Show" );       by += 28;

    y += listH + 8;
    LY_MakeChild( self, "static", SS_LEFT, IDC_LY_LBL_ACTIVE + 1, M, y + 3, 40, 16, "Name:" );
    s_lyName = LY_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL, IDC_LY_NAMEEDIT,
                             M + 42, y, listW - 42, 22 );
    y += 28;
    s_lyActive = LY_MakeChild( self, "static", SS_LEFT, IDC_LY_LBL_ACTIVE, M, y, listW + btnW + 12, 16, "Active:" );

    Refresh();
    return 0;
}

void CLayerDlg::OnSize( UINT nType, int cx, int cy )
{
    CWnd::OnSize( nType, cx, cy );
    if ( !s_lyList || cx < 160 || cy < 180 )
        return;

    const int M = 8;
    const int btnW = 90;
    const int btnH = 24;
    const int gap = 10;
    int listW = cx - ( M * 2 + gap + btnW );
    if ( listW < 120 )
        listW = 120;
    int btnX = M + listW + gap;

    HWND labelLayers = ::GetDlgItem( GetSafeHwnd(), IDC_LY_LBL_ACTIVE - 1 );
    HWND labelName   = ::GetDlgItem( GetSafeHwnd(), IDC_LY_LBL_ACTIVE + 1 );
    MoveWindow_( labelLayers, M, M, listW, 16 );

    int bottomY = cy - M - 16 - 28 - 22;
    if ( bottomY < M + 60 )
        bottomY = M + 60;
    MoveWindow_( s_lyList, M, M + 18, listW, bottomY - ( M + 18 ) );

    int by = M + 18;
    const int ids[] = {
        IDC_LY_NEW, IDC_LY_DELETE, IDC_LY_RENAME, IDC_LY_ASSIGN,
        IDC_LY_SELECT, IDC_LY_ACTIVE, IDC_LY_HIDE, IDC_LY_SHOW
    };
    for ( int i = 0; i < (int)( sizeof( ids ) / sizeof( ids[0] ) ); ++i )
    {
        HWND h = ::GetDlgItem( GetSafeHwnd(), ids[i] );
        MoveWindow_( h, btnX, by, btnW, btnH );
        by += 28;
    }

    MoveWindow_( labelName, M, bottomY + 6, 40, 16 );
    MoveWindow_( s_lyName, M + 42, bottomY + 3, listW - 42, 22 );
    MoveWindow_( s_lyActive, M, bottomY + 31, cx - 2 * M, 16 );
}

// ── New: read the Name edit, add the layer under the picked parent (or root) ──
// Faithful subset of CLayers::NewLayer (0x41C690): reject empty / "./\\" / "prefabs"
// names; if a layer is selected and isn't "The Map", nest under it ("parent/name");
// Layers_AddLayerPath + refresh.
void CLayerDlg::OnNew()
{
    char name[260] = { 0 };
    if ( s_lyName ) ::GetWindowTextA( s_lyName, name, sizeof( name ) - 1 );
    if ( !name[0] || strpbrk( name, "./\\" ) || !strncmp( name, "prefabs", 7 ) )
        return;

    char full[1100];
    char parent[1100];
    if ( LY_SelectedName( parent, sizeof( parent ) ) && strcmp( parent, "The Map" ) )
        _snprintf( full, sizeof( full ) - 1, "%s/%s", parent, name );
    else
        _snprintf( full, sizeof( full ) - 1, "%s", name );
    full[sizeof( full ) - 1] = '\0';

    Layers_AddLayerPath( full, 0 );
    Refresh();
    g_nUpdateBits = -1;
}

// ── Delete: confirm, then Layers_DeleteLayer (re-layers brushes to active) ──
void CLayerDlg::OnDelete()
{
    char name[1100];
    if ( !LY_SelectedName( name, sizeof( name ) ) )
        return;
    if ( !strcmp( name, "000_Global" ) || !strcmp( name, "The Map" ) )
        return;                       // the built-ins are not deletable
    if ( MessageBoxA( "Delete Layer?", "Radiant", MB_YESNO | MB_ICONQUESTION ) == IDNO )
        return;
    Layers_DeleteLayer( name );
    Refresh();
    g_nUpdateBits = -1;
}

// ── Rename: read the Name edit, rename the picked layer's path leaf ──
void CLayerDlg::OnRename()
{
    char oldFull[1100];
    if ( !LY_SelectedName( oldFull, sizeof( oldFull ) ) )
        return;
    if ( !strcmp( oldFull, "000_Global" ) )
        return;
    char newLeaf[260] = { 0 };
    if ( s_lyName ) ::GetWindowTextA( s_lyName, newLeaf, sizeof( newLeaf ) - 1 );
    if ( !newLeaf[0] || strpbrk( newLeaf, "./\\" ) || !strncmp( newLeaf, "prefabs", 7 ) )
        return;

    // Build the new full path = (old's parent path) + newLeaf.
    char newFull[1100];
    const char *slash = strrchr( oldFull, '/' );
    if ( slash )
    {
        int plen = (int)( slash - oldFull );
        _snprintf( newFull, sizeof( newFull ) - 1, "%.*s/%s", plen, oldFull, newLeaf );
    }
    else
        _snprintf( newFull, sizeof( newFull ) - 1, "%s", newLeaf );
    newFull[sizeof( newFull ) - 1] = '\0';

    Layers_RenameLayer( oldFull, newFull );
    Refresh();
    g_nUpdateBits = -1;
}

void CLayerDlg::OnAssign()
{
    char name[1100];
    if ( !LY_SelectedName( name, sizeof( name ) ) )
        return;
    Layers_AssignSelectionToLayer( name );
    Refresh();
}

void CLayerDlg::OnSelectAll()
{
    char name[1100];
    if ( !LY_SelectedName( name, sizeof( name ) ) )
        return;
    Select_BrushByLayer( name );
}

void CLayerDlg::OnMakeActive()
{
    char name[1100];
    if ( !LY_SelectedName( name, sizeof( name ) ) )
        return;
    Layers_SetActive( name );
    Refresh();
}

void CLayerDlg::OnHide()
{
    char name[1100];
    if ( !LY_SelectedName( name, sizeof( name ) ) )
        return;
    Layers_SetHidden( name, true );
    Refresh();
}

void CLayerDlg::OnShow()
{
    char name[1100];
    if ( !LY_SelectedName( name, sizeof( name ) ) )
        return;
    Layers_SetHidden( name, false );
    Refresh();
}

// ══════════════════════════════════════════════════════════════════════════════
//  MENU233 — the layer list's RIGHT-CLICK CONTEXT MENU  (audit U7 item 7)
//
//  The binary pops this from CLayerDlg::ContextMenu (0x41DEF0, the tree's
//  WM_RBUTTONDOWN) and routes the picks through CLayersDlg's own message map
//  (entries @0x6D8920).  All 14 ids + the exact labels/order/separators are the PE's
//  MENU233 resource, transcribed verbatim:
//
//    33959 Create New Layer          CLayers::NewLayer                0x41C690
//    33955 Set as Active Layer       CLayerDlg::SetActiveLayer        0x41C610
//    -----
//    33956 Add Selected Brushes      AssignSelectionToLayer           0x41C850
//    33958 Select All Brushes        SelectAllBrushesInLayer          0x41C960
//    -----
//    33967 Hide Layer                OnHideLayer         (sub_41D0F0, recurse 0)
//    33966 Hide Layer and Children   OnHideLayerAndChildren  (recurse 1)
//    -----
//    33968 Show Layer                OnShowLayer         (sub_41D270, recurse 0)
//    33969 Show Layer and Children   OnShowLayerAndChildren  (recurse 1)
//    -----
//    35006 Freeze Layer              OnFreezeLayer       (sub_41D1B0, recurse 0)
//    35007 Freeze Layer and Children OnFreezeLayerAndChildren(recurse 1)
//    -----
//    33963 Collapse Children         OnCollapseChildren  (sub_41D3D0, recurse 1)
//    33965 Expand Children           OnExpandChildren    (sub_41D320, recurse 1)
//    -----
//    35002 Rename Layer              CLayers::RenameLayer             0x41CC60
//    33976 Delete Layer              CLayers::DeleteLayer             0x41CBE0
//
//  UI REDUCTION (documented, see the file header + Layers_SetExpanded): the pane is a
//  FLAT listbox, so "Collapse/Expand Children" write the persisted `expanded` flag but
//  nothing folds on screen.  "and Children" recursion uses the layer PATH prefix, which
//  is exactly the tree's parent/child relation in a different representation.
//
//  TRAP (memory: TrackPopupMenu x capture): the menu is popped only AFTER any mouse
//  capture is released — the same precedent as CXYWnd::OnRButtonUp.  Popping with a
//  capture live can hang the port even though the binary's literal order works.
// ══════════════════════════════════════════════════════════════════════════════
void CLayerDlg::OnContextMenu( CWnd *pWnd, CPoint point )
{
    if ( !s_lyList )
        return;

    // Keyboard-invoked context menu (Shift+F10) arrives with (-1,-1): anchor on the list.
    if ( point.x == -1 && point.y == -1 )
    {
        RECT rc;
        ::GetWindowRect( s_lyList, &rc );
        point.x = rc.left + 8;
        point.y = rc.top + 8;
    }
    else
    {
        // Select whatever the user right-clicked so every command below acts on it
        // (the binary's tree does the same via TVGN_DROPHILITE/HitTest).
        POINT cp = { point.x, point.y };
        ::ScreenToClient( s_lyList, &cp );
        const LPARAM pt = (LPARAM)( ( (unsigned)cp.y << 16 ) | ( (unsigned)cp.x & 0xFFFF ) );
        LRESULT hit = ::SendMessageA( s_lyList, LB_ITEMFROMPOINT, 0, pt );
        if ( ( ( (unsigned)hit >> 16 ) & 0xFFFF ) == 0 )   // HIWORD==0 -> point is inside an item
            ::SendMessageA( s_lyList, LB_SETCURSEL, (WPARAM)( (unsigned)hit & 0xFFFF ), 0 );
    }

    char name[1100];
    const bool haveSel = LY_SelectedName( name, sizeof( name ) );
    const UINT selFlags = haveSel ? MF_STRING : ( MF_STRING | MF_GRAYED );

    HMENU menu = ::CreatePopupMenu();
    if ( !menu )
        return;
    ::AppendMenuA( menu, MF_STRING, 33959, "Create New Layer" );
    ::AppendMenuA( menu, selFlags,  33955, "Set as Active Layer" );
    ::AppendMenuA( menu, MF_SEPARATOR, 0, nullptr );
    ::AppendMenuA( menu, selFlags,  33956, "Add Selected Brushes" );
    ::AppendMenuA( menu, selFlags,  33958, "Select All Brushes" );
    ::AppendMenuA( menu, MF_SEPARATOR, 0, nullptr );
    ::AppendMenuA( menu, selFlags,  33967, "Hide Layer" );
    ::AppendMenuA( menu, selFlags,  33966, "Hide Layer and Children" );
    ::AppendMenuA( menu, MF_SEPARATOR, 0, nullptr );
    ::AppendMenuA( menu, selFlags,  33968, "Show Layer" );
    ::AppendMenuA( menu, selFlags,  33969, "Show Layer and Children" );
    ::AppendMenuA( menu, MF_SEPARATOR, 0, nullptr );
    ::AppendMenuA( menu, selFlags,  35006, "Freeze Layer" );
    ::AppendMenuA( menu, selFlags,  35007, "Freeze Layer and Children" );
    ::AppendMenuA( menu, MF_SEPARATOR, 0, nullptr );
    ::AppendMenuA( menu, selFlags,  33963, "Collapse Children" );
    ::AppendMenuA( menu, selFlags,  33965, "Expand Children" );
    ::AppendMenuA( menu, MF_SEPARATOR, 0, nullptr );
    ::AppendMenuA( menu, selFlags,  35002, "Rename Layer" );
    ::AppendMenuA( menu, selFlags,  33976, "Delete Layer" );

    // Release any live capture BEFORE the modal menu loop (see TRAP above).
    if ( ::GetCapture() )
        ::ReleaseCapture();

    const int cmd = (int)::TrackPopupMenu( menu,
                                           TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
                                           point.x, point.y, 0, GetSafeHwnd(), nullptr );
    ::DestroyMenu( menu );
    if ( cmd == 0 )
        return;

    switch ( cmd )
    {
    case 33959: OnNew();        return;   // CLayers::NewLayer                 0x41C690
    case 33955: OnMakeActive(); return;   // CLayerDlg::SetActiveLayer         0x41C610
    case 33956: OnAssign();     return;   // AssignSelectionToLayer            0x41C850
    case 33958: OnSelectAll();  return;   // SelectAllBrushesInLayer           0x41C960
    case 35002: OnRename();     return;   // CLayers::RenameLayer              0x41CC60
    case 33976: OnDelete();     return;   // CLayers::DeleteLayer              0x41CBE0
    default:    break;
    }

    if ( !haveSel )
        return;

    switch ( cmd )
    {
    case 33967: Layers_ForSelfAndChildren( name, false, LY_ApplyHide );     break;  // 0x41CA60
    case 33966: Layers_ForSelfAndChildren( name, true,  LY_ApplyHide );     break;  // 0x41CA90
    case 33968: Layers_ForSelfAndChildren( name, false, LY_ApplyShow );     break;  // 0x41CAC0
    case 33969: Layers_ForSelfAndChildren( name, true,  LY_ApplyShow );     break;  // 0x41CAF0
    case 35006: Layers_ForSelfAndChildren( name, false, LY_ApplyFreeze );   break;  // 0x41CB20
    case 35007: Layers_ForSelfAndChildren( name, true,  LY_ApplyFreeze );   break;  // 0x41CB50
    case 33963: Layers_ForSelfAndChildren( name, true,  LY_ApplyCollapse ); break;  // 0x41CB80
    case 33965: Layers_ForSelfAndChildren( name, true,  LY_ApplyExpand );   break;  // 0x41CBB0
    default: return;
    }

    LayersDlg_RefreshIfOpen();
    g_nUpdateBits = -1;
}

void CLayerDlg::OnClose()
{
    // Modeless: hide rather than destroy (CMainFrame::OnLayersDlg toggles visibility).
    ShowWindow( SW_HIDE );
}

// CLayerDlg::OnDestroy (0x41C1B0) — persist the dialog placement; the load side
// already reads "Radiant::LayersDlgPlace" below.  [audit U8/D1 — handler was absent,
// so the dialog always fell back to the hard-coded 373x621 default.]
extern BOOL SaveRegistryInfo( const char *pszName, void *pvBuf, int lSize );   // win_qe3.cpp
void CLayerDlg::OnDestroy()
{
    CWnd::OnDestroy();                                       // 0x41c1b9
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof( wndpl );                          // 0x41c1ca (44)
    if ( GetWindowPlacement( &wndpl ) )                      // 0x41c1d1
        SaveRegistryInfo( "Radiant::LayersDlgPlace", &wndpl, 0x2C );   // 0x41c1e7
}

void CLayerDlg::PostNcDestroy()
{
    g_dlgLayers = nullptr;
    s_lyList = s_lyName = s_lyActive = nullptr;
    delete this;
}

// 0x42BD10  CMainFrame::OnLayersDlg toggles this; first call creates it.
void CLayerDlg::Toggle()
{
    if ( g_dlgLayers && ::IsWindow( g_dlgLayers->GetSafeHwnd() ) )
    {
        BOOL vis = g_dlgLayers->IsWindowVisible();
        g_dlgLayers->ShowWindow( vis ? SW_HIDE : SW_SHOW );
        if ( !vis )
        {
            g_dlgLayers->Refresh();
            g_dlgLayers->SetForegroundWindow();
        }
        g_nUpdateBits |= 1;
        return;
    }

    CWnd *parent = AfxGetMainWnd();
    g_dlgLayers = new CLayerDlg();

    const DWORD style   = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;
    const DWORD exStyle = WS_EX_TOOLWINDOW;
    int px = 200, py = 160;
    if ( parent && parent->GetSafeHwnd() )
    {
        RECT r;  parent->GetWindowRect( &r );
        px = r.left + 120;  py = r.top + 100;
    }
    CRect rc( px, py, px + 373, py + 621 );
    if ( !g_dlgLayers->CreateEx( exStyle, AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW,
                                     ::LoadCursor( NULL, IDC_ARROW ), (HBRUSH)( COLOR_BTNFACE + 1 ), NULL ),
                                 "Layers", style, rc, parent, 0 ) )
    {
        delete g_dlgLayers;
        g_dlgLayers = nullptr;
        return;
    }
    WINDOWPLACEMENT wp;
    memset( &wp, 0, sizeof( wp ) );
    wp.length = sizeof( wp );
    long size = sizeof( wp );
    if ( LoadRegistryInfo( "Radiant::LayersDlgPlace", &wp, &size ) && size >= (long)sizeof( wp ) )
    {
        wp.length = sizeof( wp );
        ::SetWindowPlacement( g_dlgLayers->GetSafeHwnd(), &wp );
    }
    g_dlgLayers->ShowWindow( SW_SHOW );
    g_dlgLayers->SetForegroundWindow();
    g_nUpdateBits |= 1;
}

// ── backend → UI refresh entry ────────────────────────────────────────────────
// The binary's Layers_02 (0x41A0E0) wipes the CTreeCtrl (TVM_DELETEITEM/TVI_ROOT),
// re-inserts the "The Map" root (sub_41A070), walks layerMap re-inserting each layer
// (maybeLayers), then bolds+icons the active item (sub_41C9C0).  In the port's flat
// listbox form ALL of that is CLayerDlg::Refresh().  This free entry lets the
// backend (layers.cpp Layers_02 / sub_41C9C0) reach the singleton without exposing it.
void LayersDlg_RefreshIfOpen()
{
    if ( g_dlgLayers && ::IsWindow( g_dlgLayers->GetSafeHwnd() ) )
        g_dlgLayers->Refresh();
}

