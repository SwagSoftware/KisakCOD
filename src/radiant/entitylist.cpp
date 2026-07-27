#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Entity browser: groups map entities by classname, shows definition epairs, and
// selects the chosen instance's brushes.

#include "stdafx.h"
#include "qe3.h"
#include "mainfrm.h"
#include <commctrl.h>
#include <stdio.h>
#include <map>
#include <string>

// ── map entity instance list (entity.cpp / map.cpp) ──────────────────────────
extern entity_s   entityInsts;        // 0x23F1748 — entity-instance list sentinel (ring)

// ── selection core (select.cpp) ──────────────────────────────────────────────
extern void       Select_Deselect( int bDeselectFaces );                    // 0x48E800
extern void       Select_Brush( selbrush_t *b, char overwrite, char status, char center ); // 0x48DCC0
extern int        g_nUpdateBits;      // engine_stubs (0x25D5A74)

// The hand-built popup instance (NULL until opened).
class CEntityListDlg;
static CEntityListDlg *g_pEntListDlg = nullptr;

// Control ids — match the binary's dialog template (dialogs.json id 129).
enum
{
    IDC_EL_TREE = 1022,   // 0x3FE  SysTreeView32 "Tree1"
    IDC_EL_LIST = 1025,   // 0x401  SysListView32 "List2"
    IDC_EL_SELECT = 2,    // IDCANCEL slot — labelled "Select" in the binary
};

// ══════════════════════════════════════════════════════════════════════════════
//  DATA CORE — headless-safe walks of the live entity list (no HWNDs).  Used both
//  by the dialog (to fill the controls) and by the RADIANT_SELFTEST gate harness.
// ══════════════════════════════════════════════════════════════════════════════

// Resolve the definition that owns an instance's classname and epairs.
static inline entity_s_def *EL_Def( entity_s *inst )
{
    return inst ? (entity_s_def *)inst->def : nullptr;
}

// The classname shown in the tree for an entity (the binary uses def->eclass->name
// verbatim; empty entities can't reach here because entityInsts only holds realized
// instances, but guard anyway).
static const char *EL_ClassName( entity_s *inst )
{
    entity_s_def *def = EL_Def( inst );
    if ( def && def->eclass && def->eclass->name )
        return def->eclass->name;
    return "";
}

// ══════════════════════════════════════════════════════════════════════════════
//  EntityListDlg::SelectItem (0x40F750) essence — select the picked entity's brushes.
//  `inst` is the entity_s* stored as the tree leaf's item data.  Deselect everything,
//  then add every brush in the entity's owner list to the selection.  g_nUpdateBits=-1.
//  Headless-safe (no HWNDs); the dialog passes the GetItemData result, the gate passes
//  an entity directly.
// ══════════════════════════════════════════════════════════════════════════════
static void EL_SelectEntityBrushes( entity_s *inst )
{
    // IDA SelectItem 0x40F750: Select_Deselect + the brush-walk run ONLY when the tree item's
    // data (entity_s*) is non-null.  A caret on a classname GROUP node (inserted without
    // TVIF_PARAM, so lParam==0) is a no-op and must not wipe the selection.
    if ( inst )
    {
        Select_Deselect( 1 );
        // head = &inst->brushes (the embedded entity_brush_s sentinel @+0x0C); the owner brush
        // list is linked via ownerNext until it wraps back to the sentinel.
        selbrush_t *head = &inst->brushes;
        for ( selbrush_t *b = head->ownerNext; b && b != head; b = b->ownerNext )
            Select_Brush( b, 1, 1, 1 );
    }
    g_nUpdateBits = -1;
}

// Count how many brushes EL_SelectEntityBrushes would select for `inst` (the gate oracle).
static int EL_EntityBrushCount( entity_s *inst )
{
    if ( !inst ) return 0;
    int n = 0;
    selbrush_t *head = &inst->brushes;
    for ( selbrush_t *b = head->ownerNext; b && b != head; b = b->ownerNext )
        ++n;
    return n;
}

// Count an entity's key/value pairs (= rows UpdateKeyValuePairs would insert).
static int EL_EntityEpairCount( entity_s *inst )
{
    entity_s_def *def = EL_Def( inst );
    if ( !def ) return 0;
    int n = 0;
    for ( epair_t *e = def->epairs; e; e = e->next )
        ++n;
    return n;
}

// ══════════════════════════════════════════════════════════════════════════════
//  THE HAND-BUILT POPUP (CWnd) — the binary's CEntityListDlg, shell-only; the data
//  comes from the cores above (faithful to InsertItems / UpdateKeyValuePairs).
// ══════════════════════════════════════════════════════════════════════════════
class CEntityListDlg : public CWnd
{
public:
    CEntityListDlg() : m_hTree( nullptr ), m_hList( nullptr ) {}

    HWND m_hTree;
    HWND m_hList;

    void PopulateTree();      // InsertItems body
    void ShowKeyValues();     // UpdateKeyValuePairs body
    void SelectFromTree();    // SelectItem body (caret -> brushes)

protected:
    virtual BOOL PreCreateWindow( CREATESTRUCT &cs );
    virtual void PostNcDestroy();
    afx_msg int  OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnClose();
    afx_msg void OnSelectButton();                              // "Select" button
    afx_msg BOOL OnNotify( WPARAM wParam, LPARAM lParam, LRESULT *pResult );
    DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP( CEntityListDlg, CWnd )
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_BN_CLICKED( IDC_EL_SELECT, &CEntityListDlg::OnSelectButton )
END_MESSAGE_MAP()

BOOL CEntityListDlg::PreCreateWindow( CREATESTRUCT &cs )
{
    if ( !CWnd::PreCreateWindow( cs ) )
        return FALSE;
    cs.lpszClass = AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW,
                                        ::LoadCursor( NULL, IDC_ARROW ),
                                        (HBRUSH)( COLOR_BTNFACE + 1 ), NULL );
    return TRUE;
}

int CEntityListDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;

    // Tree + list common controls.
    INITCOMMONCONTROLSEX icc = { sizeof( icc ), ICC_TREEVIEW_CLASSES | ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx( &icc );

    HWND self = GetSafeHwnd();
    HFONT font = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

    RECT rc;
    ::GetClientRect( self, &rc );
    const int M = 6, btnH = 26;
    const int bodyH = ( rc.bottom - rc.top ) - btnH - 3 * M;
    const int halfW = ( ( rc.right - rc.left ) - 3 * M ) / 2;

    m_hTree = CreateWindowExA( WS_EX_CLIENTEDGE, WC_TREEVIEWA, nullptr,
                               WS_CHILD | WS_VISIBLE | WS_TABSTOP | TVS_HASLINES | TVS_HASBUTTONS |
                               TVS_LINESATROOT | TVS_SHOWSELALWAYS,
                               M, M, halfW, bodyH, self, (HMENU)(INT_PTR)IDC_EL_TREE,
                               AfxGetInstanceHandle(), NULL );

    m_hList = CreateWindowExA( WS_EX_CLIENTEDGE, WC_LISTVIEWA, nullptr,
                               WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL,
                               M + halfW + M, M, halfW, bodyH, self, (HMENU)(INT_PTR)IDC_EL_LIST,
                               AfxGetInstanceHandle(), NULL );

    // "Select" + "Close" buttons along the bottom.
    HWND hSel = CreateWindowExA( 0, "button", "Select", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                 M, M + bodyH + M, 90, btnH, self, (HMENU)(INT_PTR)IDC_EL_SELECT,
                                 AfxGetInstanceHandle(), NULL );
    HWND hClose = CreateWindowExA( 0, "button", "Close", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                   M + 98, M + bodyH + M, 90, btnH, self, (HMENU)(INT_PTR)IDOK,
                                   AfxGetInstanceHandle(), NULL );

    if ( font )
    {
        // ::SendMessageA (global) — inside a CWnd method the bare name binds to the 2-arg
        // CWnd::SendMessageA member; these target child HWNDs, so use the Win32 API.
        ::SendMessageA( m_hTree, WM_SETFONT, (WPARAM)font, 0 );
        ::SendMessageA( m_hList, WM_SETFONT, (WPARAM)font, 0 );
        if ( hSel )   ::SendMessageA( hSel,   WM_SETFONT, (WPARAM)font, 0 );
        if ( hClose ) ::SendMessageA( hClose, WM_SETFONT, (WPARAM)font, 0 );
    }

    PopulateTree();
    return 0;
}

// InsertItems (0x40F7D0) — walk entityInsts; one parent tree node per classname (deduped
// via a std::map, the binary's CMap<CString,HTREEITEM>), one leaf per entity with the
// entity_s* as item data; two list columns "Key"/"Value" (half the list width each).
void CEntityListDlg::PopulateTree()
{
    if ( !m_hTree || !m_hList )
        return;

    TreeView_DeleteAllItems( m_hTree );

    std::map<std::string, HTREEITEM> byClass;     // classname -> parent node (dedup)

    for ( entity_s *ent = entityInsts.next; ent && ent != &entityInsts; ent = ent->next )
    {
        const char *cls = EL_ClassName( ent );
        std::string key( cls ? cls : "" );

        HTREEITEM parent;
        std::map<std::string, HTREEITEM>::iterator it = byClass.find( key );
        if ( it == byClass.end() )
        {
            TVINSERTSTRUCTA tis;
            ZeroMemory( &tis, sizeof( tis ) );
            tis.hParent = TVI_ROOT;
            tis.hInsertAfter = TVI_SORT;
            tis.item.mask = TVIF_TEXT;
            tis.item.pszText = (LPSTR)( cls ? cls : "" );
            parent = TreeView_InsertItem( m_hTree, &tis );
            byClass[key] = parent;
        }
        else
        {
            parent = it->second;
        }

        // Leaf — labelled with the classname too (the binary inserts the classname for the
        // leaf as well), item data = the entity_s* (read back by SelectItem/UpdateKV).
        TVINSERTSTRUCTA leaf;
        ZeroMemory( &leaf, sizeof( leaf ) );
        leaf.hParent = parent;
        leaf.hInsertAfter = TVI_SORT;
        leaf.item.mask = TVIF_TEXT | TVIF_PARAM;
        leaf.item.pszText = (LPSTR)( cls ? cls : "" );
        leaf.item.lParam = (LPARAM)ent;
        TreeView_InsertItem( m_hTree, &leaf );
    }

    // Two columns sized to half the list client width (the binary's InsertColumn calls).
    RECT lr;
    ::GetClientRect( m_hList, &lr );
    int colW = ( lr.right - lr.left ) / 2;
    if ( colW < 40 ) colW = 40;

    LVCOLUMNA col;
    ZeroMemory( &col, sizeof( col ) );
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    col.cx = colW;  col.pszText = (LPSTR)"Key";   col.iSubItem = 0;
    ListView_InsertColumn( m_hList, 0, &col );
    col.cx = colW;  col.pszText = (LPSTR)"Value"; col.iSubItem = 1;
    ListView_InsertColumn( m_hList, 1, &col );
}

// UpdateKeyValuePairs (0x40F990) — the tree-caret entity's epairs into the list.
void CEntityListDlg::ShowKeyValues()
{
    if ( !m_hTree || !m_hList )
        return;

    ListView_DeleteAllItems( m_hList );

    HTREEITEM sel = TreeView_GetSelection( m_hTree );
    if ( !sel )
        return;
    entity_s *inst = nullptr;
    {
        TVITEMA ti;
        ZeroMemory( &ti, sizeof( ti ) );
        ti.mask = TVIF_PARAM;
        ti.hItem = sel;
        if ( TreeView_GetItem( m_hTree, &ti ) )
            inst = (entity_s *)ti.lParam;
    }
    entity_s_def *def = EL_Def( inst );
    if ( !def )
        return;

    int row = 0;
    for ( epair_t *e = def->epairs; e; e = e->next )
    {
        LVITEMA lvi;
        ZeroMemory( &lvi, sizeof( lvi ) );
        lvi.mask = LVIF_TEXT;
        lvi.iItem = row;
        lvi.iSubItem = 0;
        lvi.pszText = (LPSTR)( e->key ? e->key : "" );
        int idx = ListView_InsertItem( m_hList, &lvi );
        ListView_SetItemText( m_hList, idx, 1, (LPSTR)( e->value ? e->value : "" ) );
        ++row;
    }
}

// SelectItem (0x40F750) — read the tree caret's entity, select its brushes.
void CEntityListDlg::SelectFromTree()
{
    if ( !m_hTree )
        return;
    HTREEITEM sel = TreeView_GetSelection( m_hTree );
    if ( !sel )
        return;
    TVITEMA ti;
    ZeroMemory( &ti, sizeof( ti ) );
    ti.mask = TVIF_PARAM;
    ti.hItem = sel;
    if ( !TreeView_GetItem( m_hTree, &ti ) )
        return;
    EL_SelectEntityBrushes( (entity_s *)ti.lParam );
}

// WM_NOTIFY from the tree: TVN_SELCHANGED -> refresh the K/V list; NM_DBLCLK -> select the
// entity's brushes (OnSelectItem 0x40FB00).  This mirrors the binary's two message-map
// entries (tree id 1022) without depending on the exact MFC ON_NOTIFY codes.
BOOL CEntityListDlg::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT *pResult )
{
    NMHDR *nm = (NMHDR *)lParam;
    if ( nm && nm->idFrom == IDC_EL_TREE )
    {
        if ( nm->code == TVN_SELCHANGEDA || nm->code == TVN_SELCHANGEDW )
        {
            ShowKeyValues();
            if ( pResult ) *pResult = 0;
            return TRUE;
        }
        if ( nm->code == (UINT)NM_DBLCLK )
        {
            SelectFromTree();
            if ( pResult ) *pResult = 0;
            return TRUE;
        }
    }
    return CWnd::OnNotify( wParam, lParam, pResult );
}

void CEntityListDlg::OnSelectButton()
{
    SelectFromTree();
}

void CEntityListDlg::OnClose()
{
    ShowWindow( SW_HIDE );
}

void CEntityListDlg::PostNcDestroy()
{
    g_pEntListDlg = nullptr;
    delete this;
}

// CMainFrame::OnEditEntityinfo (0x426D6F) opens this.  The binary makes a fresh modal
// dialog each invoke; Show() recreates (or re-shows + re-populates against the current
// map) the popup on every open — a browser view of the live entity list.
void EntList_Open()
{
    if ( g_pEntListDlg && ::IsWindow( g_pEntListDlg->GetSafeHwnd() ) )
    {
        g_pEntListDlg->PopulateTree();       // re-read the current map
        g_pEntListDlg->ShowWindow( SW_SHOW );
        g_pEntListDlg->SetForegroundWindow();
        return;
    }

    CWnd *parent = AfxGetMainWnd();
    g_pEntListDlg = new CEntityListDlg();

    const DWORD style   = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_VISIBLE;
    const DWORD exStyle = WS_EX_TOOLWINDOW;
    int px = 240, py = 160;
    if ( parent && parent->GetSafeHwnd() )
    {
        RECT r;  parent->GetWindowRect( &r );
        px = r.left + 140;  py = r.top + 100;
    }
    CRect rc( px, py, px + 460, py + 400 );
    if ( !g_pEntListDlg->CreateEx( exStyle,
             AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW,
                 ::LoadCursor( NULL, IDC_ARROW ), (HBRUSH)( COLOR_BTNFACE + 1 ), NULL ),
             "Entities", style, rc, parent, 0 ) )
    {
        delete g_pEntListDlg;
        g_pEntListDlg = nullptr;
    }
}

#ifdef RADIANT_SELFTEST
// ══════════════════════════════════════════════════════════════════════════════
//  Deterministic test entry (the `entitylist` gate).  Runs the dialog's data cores
//  (InsertItems tree-grouping + UpdateKeyValuePairs epair-walk + SelectItem brush-walk)
//  WITHOUT any HWNDs and returns counts so RunEntityListTest (radiantapp.cpp) can
//  cross-check them against an independent walk of entityInsts (= the .map content).
//  out[0] = total entities (= tree leaf count = entityInsts node count)
//  out[1] = distinct classnames (= tree parent-node count)
//  out[2] = total epairs across all entities (= sum of every list-population)
//  out[3] = total owner-brushes across all entities (= sum of every SelectItem select)
//  out[4] = epairs of the FIRST entity (what the list would show on the first selection)
//  out[5] = owner-brushes of the FIRST entity (what SelectItem would select for it)
// ══════════════════════════════════════════════════════════════════════════════
extern "C" int Radiant_TestEntityList( int *out /* int[6] */ )
{
    std::map<std::string, int> classes;
    int totalEnts = 0, totalEpairs = 0, totalBrushes = 0;
    int firstEpairs = 0, firstBrushes = 0;
    entity_s *first = nullptr;

    for ( entity_s *ent = entityInsts.next; ent && ent != &entityInsts; ent = ent->next )
    {
        if ( !first ) first = ent;
        ++totalEnts;
        classes[ std::string( EL_ClassName( ent ) ) ] += 1;
        totalEpairs  += EL_EntityEpairCount( ent );
        totalBrushes += EL_EntityBrushCount( ent );
    }
    if ( first )
    {
        firstEpairs  = EL_EntityEpairCount( first );
        firstBrushes = EL_EntityBrushCount( first );
    }

    out[0] = totalEnts;
    out[1] = (int)classes.size();
    out[2] = totalEpairs;
    out[3] = totalBrushes;
    out[4] = firstEpairs;
    out[5] = firstBrushes;
    return 0;
}
#endif // RADIANT_SELFTEST
