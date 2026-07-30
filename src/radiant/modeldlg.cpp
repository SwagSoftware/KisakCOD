#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Modeless bulk model replacement dialog. It replaces matching misc_model,
// dyn_model, or misc_prefab "model" keys with a random entry from the target set.

#include "stdafx.h"
#include "qe3.h"
#include "mainfrm.h"
#include <cstring>
#include <cstdlib>

extern void Assert( const char *file, int line, int type, const char *fmt, ... );

// ── externs (per-TU, matching vehicledlg.cpp / undo.cpp) ──────────────────────
extern entity_s *world_entity;                                              // 0x25D5B30 (map.cpp)
extern void      SetKeyValue( entity_s_def *e, const char *key, const char *value ); // 0x483690 (entity.cpp)
extern bool      Entity_HasEpairMatch( entity_s *e, const char *key, const char *value ); // 0x483930 (entity.cpp)
extern char      FilterBrush( selbrush_t *b, int a2 );                      // 0x46A1F0 (filters.cpp)
extern int       g_nUpdateBits;                                            // 0x25D5A74 (engine_stubs.cpp)
// active_brushes / selected_brushes sentinels are declared in qe3.h.

// ═════════════════════════════════════════════════════════════════════════════
//  THE CORE — 0x434EC0  CModelDlg::OnReplace (BN_CLICKED ctrl 0x627/1575)
//  Replace every matching entity's "model" with a random TO entry.
//    classFlags bit0 = touch misc_model, bit1 = touch dyn_model, bit2 = touch misc_prefab
//    (in the binary these are the three BM_GETCHECK reads off the dialog's CButton members
//     at this+0x98 / +0xEC / +0x140).
// ═════════════════════════════════════════════════════════════════════════════
void ModelDlg_DoReplace( const char *const *fromSet, int fromCount,
                         const char *const *toSet,   int toCount,
                         int classFlags )
{
    if ( fromCount <= 0 || toCount <= 0 )       // 0x434EDE/EF2: either set empty → bail
    {
        g_nUpdateBits = -1;
        return;
    }

    const bool doModel  = ( classFlags & 1 ) != 0;
    const bool doDyn    = ( classFlags & 2 ) != 0;
    const bool doPrefab = ( classFlags & 4 ) != 0;

    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; )
    {
        selbrush_t *next = b->next->prev;       // pre-save (faithful sentinel-walk; +0x04 then +0x00)

        // 0x434F1B: skip filtered.  0x434F2B: skip brushflags & 2 and & 0x20.  skip world.
        if ( FilterBrush( b, 0 ) )                              { b = next; continue; }
        if ( ( b->brushFlags & 2 ) || ( b->brushFlags & 0x20 ) ){ b = next; continue; }

        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )                 { b = next; continue; }

        // 0x434F55: the `brush->owner->def == brush->def->owner` invariant (type 0 → log+continue).
        entity_s_def *ent = (entity_s_def *)owner->def;
        if ( b->def && ent != (entity_s_def *)b->def->owner )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\ModelDlg.cpp",
                    292, 0, "%s", "brush->owner->def == brush->def->owner" );
        if ( !ent )                                            { b = next; continue; }

        // 0x434FA6..: per FROM entry, test this entity by class + the class checkbox.
        const char *cls = ent->eclass->name;                   // eclass@0x60, name@0x04
        const bool isModel  = ( strcmp( cls, "misc_model" )  == 0 );
        const bool isDyn    = ( strcmp( cls, "dyn_model" )   == 0 );
        const bool isPrefab = ( strcmp( cls, "misc_prefab" ) == 0 );

        // The checkbox gate: only process this entity if its class is checked.  (In the
        // binary the per-class BM_GETCHECK short-circuits to the NEXT class test / next
        // brush; here the combined predicate is equivalent.)
        const bool process =
            ( isModel  && doModel  ) ||
            ( isDyn    && doDyn    ) ||
            ( isPrefab && doPrefab );

        if ( process )
        {
            for ( int i = 0; i < fromCount; ++i )
            {
                char fromBuf[256];
                const char *fromVal;
                if ( isPrefab )
                {
                    _snprintf( fromBuf, sizeof( fromBuf ), "prefabs/misc_models/%s.map", fromSet[i] );
                    fromVal = fromBuf;
                }
                else
                {
                    fromVal = fromSet[i];
                }

                if ( Entity_HasEpairMatch( (entity_s *)ent, "model", fromVal ) )
                {
                    int pick = ( rand() % toCount );           // 0x4350B7: random TO entry
                    char toBuf[256];
                    const char *toVal;
                    if ( isPrefab )
                    {
                        _snprintf( toBuf, sizeof( toBuf ), "prefabs/misc_models/%s.map", toSet[pick] );
                        toVal = toBuf;
                    }
                    else
                    {
                        toVal = toSet[pick];
                    }
                    SetKeyValue( ent, "model", toVal );
                    break;                                     // 0x435031: one replace per brush
                }
            }
        }

        b = next;
    }

    g_nUpdateBits = -1;
}

// ══════════════════════════════════════════════════════════════════════════════
//  CModelDlg — the hand-built modeless popup (CVehicleDlg / CDynEntityDlg pattern)
// ══════════════════════════════════════════════════════════════════════════════
//  Two list boxes (FROM / TO), each with [Add file...] (→ a CFileDialog at main_shared\
//  xmodel\, matching the binary's CModelFileDialog 0x434330/0x43464D) and [Remove], plus
//  the three class checkboxes (misc_model / dyn_model / misc_prefab) and [Replace].
//  radiant.rc has no IDD template, so it is hand-built (the established sibling pattern).

enum
{
    IDC_MDL_FROM_LIST = 4400,   // FROM model set (binary ctrl 1566)
    IDC_MDL_TO_LIST,            // TO model set   (binary ctrl 1564)
    IDC_MDL_FROM_ADD,           // [Add file...] for the FROM list
    IDC_MDL_FROM_REMOVE,
    IDC_MDL_TO_ADD,             // [Add file...] for the TO list
    IDC_MDL_TO_REMOVE,
    IDC_MDL_CHK_MODEL,          // touch misc_model entities
    IDC_MDL_CHK_DYN,            // touch dyn_model entities
    IDC_MDL_CHK_PREFAB,         // touch misc_prefab entities
    IDC_MDL_REPLACE,            // [Replace] → ModelDlg_DoReplace
};

CModelDlg *g_dlgModel = nullptr;            // the singleton (NULL until first opened)

static HFONT s_mdlFont    = nullptr;
static HWND  s_mdlFrom    = nullptr;        // FROM listbox HWND
static HWND  s_mdlTo      = nullptr;        // TO listbox HWND
static HWND  s_mdlChkMdl  = nullptr;        // class checkboxes
static HWND  s_mdlChkDyn  = nullptr;
static HWND  s_mdlChkPfb  = nullptr;

static HWND MDL_MakeChild( HWND parent, const char *cls, DWORD style, int id,
                           int x, int y, int w, int hgt, const char *text = nullptr )
{
    HWND h = CreateWindowExA( ( style & LBS_NOTIFY ) ? WS_EX_CLIENTEDGE : 0,
                              cls, text, WS_CHILD | WS_VISIBLE | style,
                              x, y, w, hgt, parent, (HMENU)(INT_PTR)id,
                              AfxGetInstanceHandle(), NULL );
    if ( h && s_mdlFont )
        SendMessageA( h, WM_SETFONT, (WPARAM)s_mdlFont, 0 );
    return h;
}

// The model file picker — a plain CFileDialog at the project's basepath\main_shared\xmodel\,
// faithful to the binary's CModelFileDialog (0x434330): the preview-pane subclass is PARKED
// (a plain Open dialog gives the identical pick, exactly as the win_ent.cpp model picker
// documents).  Adds the picked, lowercased model name to `list`.
static void MDL_AddFromFile( HWND list )
{
    if ( !list )
        return;

    // Resolve <project basepath>\main_shared\xmodel\ for the initial dir (the binary reads the
    // project entity's "basepath" key; if unavailable the dialog just opens at the CWD).
    char initDir[1024] = { 0 };
    const char *basepath = nullptr;
    if ( g_qeglobals.d_project_entity )
    {
        for ( epair_t *e = g_qeglobals.d_project_entity->epairs; e; e = e->next )
            if ( _stricmp( e->key, "basepath" ) == 0 ) { basepath = e->value; break; }
    }
    if ( basepath && basepath[0] )
        _snprintf( initDir, sizeof( initDir ), "%s\\main_shared\\xmodel\\", basepath );

    char fileBuf[1024] = { 0 };
    // Double-null-terminated filter ("Model files (*)\0*\0\0"); a plain string literal would
    // truncate at the first embedded NUL, so build it as an explicit char array.
    static const char modelFilter[] = "Model files (*)\0*\0";
    OPENFILENAMEA ofn = { 0 };
    ofn.lStructSize  = sizeof( ofn );
    ofn.hwndOwner    = ::GetActiveWindow();
    ofn.lpstrFilter  = modelFilter;
    ofn.lpstrFile    = fileBuf;
    ofn.nMaxFile     = sizeof( fileBuf );
    ofn.lpstrInitialDir = ( initDir[0] ? initDir : nullptr );
    ofn.Flags        = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
    if ( !::GetOpenFileNameA( &ofn ) )
        return;

    // Lowercase + forward-slash, then relativise to the xmodel root (matching CString_MakeLower
    // 0x435390 + the picker's path normalisation).  For the replace match we want the bare
    // model name the entity stores in its "model" key.
    char name[1024];
    strncpy( name, fileBuf, sizeof( name ) - 1 );
    name[sizeof( name ) - 1] = 0;
    for ( char *p = name; *p; ++p )
    {
        if ( *p >= 'A' && *p <= 'Z' ) *p += 32;
        if ( *p == '\\' ) *p = '/';
    }
    const char *rel = name;
    if ( const char *x = strstr( name, "xmodel/" ) )
        rel = x + 7;                                   // strip up to and including "xmodel/"

    // De-dup then add (LB_FINDSTRINGEXACT then LB_ADDSTRING — the binary's 0x434EC0-sibling
    // listbox add path).
    if ( SendMessageA( list, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)rel ) == LB_ERR )
        SendMessageA( list, LB_ADDSTRING, 0, (LPARAM)rel );
}

// Remove the selected entry from `list` (the binary's LB_GETCURSEL → LB_DELETESTRING handler,
// 0x434C80 family).
static void MDL_RemoveSel( HWND list )
{
    if ( !list )
        return;
    int sel = (int)SendMessageA( list, LB_GETCURSEL, 0, 0 );
    if ( sel != LB_ERR )
        SendMessageA( list, LB_DELETESTRING, sel, 0 );
}

// Snapshot a listbox into a heap string-pointer array (the binary's sub_4351A0: read both
// list boxes into the FROM/TO CStringArrays before the replace).  Caller frees with MDL_FreeSet.
static int MDL_SnapshotList( HWND list, char ***outSet )
{
    *outSet = nullptr;
    if ( !list )
        return 0;
    int n = (int)SendMessageA( list, LB_GETCOUNT, 0, 0 );
    if ( n <= 0 )
        return 0;
    char **set = (char **)malloc( sizeof( char * ) * n );
    for ( int i = 0; i < n; ++i )
    {
        int len = (int)SendMessageA( list, LB_GETTEXTLEN, i, 0 );
        char *s = (char *)malloc( len + 1 );
        SendMessageA( list, LB_GETTEXT, i, (LPARAM)s );
        set[i] = s;
    }
    *outSet = set;
    return n;
}

static void MDL_FreeSet( char **set, int n )
{
    if ( !set )
        return;
    for ( int i = 0; i < n; ++i )
        free( set[i] );
    free( set );
}

BEGIN_MESSAGE_MAP( CModelDlg, CWnd )
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_BN_CLICKED( IDC_MDL_FROM_ADD,    &CModelDlg::OnAddFrom )
    ON_BN_CLICKED( IDC_MDL_FROM_REMOVE, &CModelDlg::OnRemoveFrom )
    ON_BN_CLICKED( IDC_MDL_TO_ADD,      &CModelDlg::OnAddTo )
    ON_BN_CLICKED( IDC_MDL_TO_REMOVE,   &CModelDlg::OnRemoveTo )
    ON_BN_CLICKED( IDC_MDL_REPLACE,     &CModelDlg::OnReplace )
END_MESSAGE_MAP()

CModelDlg::CModelDlg()
{
}

int CModelDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;
    if ( !s_mdlFont )
        s_mdlFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

    HWND self = GetSafeHwnd();
    const int M = 10, colW = 220, listH = 200, btnW = 90, rowH = 26;
    int lx = M;                                  // FROM column x
    int tx = M + colW + 30;                      // TO column x
    int y  = M;

    MDL_MakeChild( self, "static", SS_LEFT, 0, lx, y, colW, 16, "Replace these models:" );
    MDL_MakeChild( self, "static", SS_LEFT, 0, tx, y, colW, 16, "...with one of these (random):" );
    y += 20;

    s_mdlFrom = MDL_MakeChild( self, "listbox", LBS_NOTIFY | LBS_HASSTRINGS | WS_VSCROLL | WS_BORDER,
                               IDC_MDL_FROM_LIST, lx, y, colW, listH );
    s_mdlTo   = MDL_MakeChild( self, "listbox", LBS_NOTIFY | LBS_HASSTRINGS | WS_VSCROLL | WS_BORDER,
                               IDC_MDL_TO_LIST,   tx, y, colW, listH );
    y += listH + 6;

    MDL_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_MDL_FROM_ADD,    lx, y, btnW, 22, "Add file..." );
    MDL_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_MDL_FROM_REMOVE, lx + btnW + 6, y, btnW, 22, "Remove" );
    MDL_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_MDL_TO_ADD,      tx, y, btnW, 22, "Add file..." );
    MDL_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_MDL_TO_REMOVE,   tx + btnW + 6, y, btnW, 22, "Remove" );
    y += rowH + 8;

    // The three class checkboxes (the binary's CButton members read by the replace).
    s_mdlChkMdl = MDL_MakeChild( self, "button", BS_AUTOCHECKBOX | WS_TABSTOP, IDC_MDL_CHK_MODEL,  lx, y, 150, 20, "misc_model" );
    s_mdlChkDyn = MDL_MakeChild( self, "button", BS_AUTOCHECKBOX | WS_TABSTOP, IDC_MDL_CHK_DYN,    lx + 160, y, 150, 20, "dyn_model" );
    s_mdlChkPfb = MDL_MakeChild( self, "button", BS_AUTOCHECKBOX | WS_TABSTOP, IDC_MDL_CHK_PREFAB, lx + 320, y, 150, 20, "misc_prefab" );
    if ( s_mdlChkMdl ) ::SendMessageA( s_mdlChkMdl, BM_SETCHECK, BST_CHECKED, 0 );   // default: misc_model on
    y += rowH + 8;

    MDL_MakeChild( self, "button", BS_DEFPUSHBUTTON | WS_TABSTOP, IDC_MDL_REPLACE, lx, y, 120, 24, "Replace" );
    return 0;
}

void CModelDlg::OnAddFrom()    { MDL_AddFromFile( s_mdlFrom ); }
void CModelDlg::OnRemoveFrom() { MDL_RemoveSel( s_mdlFrom ); }
void CModelDlg::OnAddTo()      { MDL_AddFromFile( s_mdlTo ); }
void CModelDlg::OnRemoveTo()   { MDL_RemoveSel( s_mdlTo ); }

// [Replace] — snapshot both lists + the checkbox state, then run the replace core.
void CModelDlg::OnReplace()
{
    char **fromSet = nullptr, **toSet = nullptr;
    int fromN = MDL_SnapshotList( s_mdlFrom, &fromSet );
    int toN   = MDL_SnapshotList( s_mdlTo,   &toSet );

    int flags = 0;
    if ( s_mdlChkMdl && ::SendMessageA( s_mdlChkMdl, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) flags |= 1;
    if ( s_mdlChkDyn && ::SendMessageA( s_mdlChkDyn, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) flags |= 2;
    if ( s_mdlChkPfb && ::SendMessageA( s_mdlChkPfb, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) flags |= 4;

    ModelDlg_DoReplace( fromSet, fromN, toSet, toN, flags );

    MDL_FreeSet( fromSet, fromN );
    MDL_FreeSet( toSet, toN );
    g_nUpdateBits |= 1;
}

void CModelDlg::OnClose()
{
    // Modeless: hide rather than destroy (OnReplaceModels toggles visibility).
    ShowWindow( SW_HIDE );
}

void CModelDlg::PostNcDestroy()
{
    g_dlgModel = nullptr;
    s_mdlFrom = s_mdlTo = nullptr;
    s_mdlChkMdl = s_mdlChkDyn = s_mdlChkPfb = nullptr;
    delete this;
}

// 0x42BF00  CMainFrame::OnReplaceModels toggles this; first call creates it.
void CModelDlg::Toggle()
{
    if ( g_dlgModel && ::IsWindow( g_dlgModel->GetSafeHwnd() ) )
    {
        BOOL vis = g_dlgModel->IsWindowVisible();
        g_dlgModel->ShowWindow( vis ? SW_HIDE : SW_SHOW );
        if ( !vis )
            g_dlgModel->SetForegroundWindow();
        g_nUpdateBits |= 1;
        return;
    }

    CWnd *parent = AfxGetMainWnd();
    g_dlgModel = new CModelDlg();

    const DWORD style   = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    const DWORD exStyle = WS_EX_TOOLWINDOW;
    int px = 220, py = 160;
    if ( parent && parent->GetSafeHwnd() )
    {
        RECT r;  parent->GetWindowRect( &r );
        px = r.left + 140;  py = r.top + 100;
    }
    CRect rc( px, py, px + 510, py + 360 );
    if ( !g_dlgModel->CreateEx( exStyle,
             AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW,
                 ::LoadCursor( NULL, IDC_ARROW ), (HBRUSH)( COLOR_BTNFACE + 1 ), NULL ),
             "Replace Models", style, rc, parent, 0 ) )
    {
        delete g_dlgModel;
        g_dlgModel = nullptr;
        return;
    }
    g_nUpdateBits |= 1;
}

