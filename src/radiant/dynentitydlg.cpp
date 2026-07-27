#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Dynamic/destructible entity authoring dialog. The dyn_model keys are:
//   "type"          - "clutter" (default) | "destruct"  (a CComboBox preset list)
//   "physPreset"    - the physics preset                (main_shared\physic\)
//   "health"        - destruct only: HP → triggers a destroy event on death
//   "destroyEfx"    - destruct only: the destroy FX     (raw_shared\fx\, *.efx)
//   "destroyPieces" - destruct only: the breakable model (main_shared\xmodelpieces\)
//
// The dialog is write-only: empty fields remove their keys.

#include "stdafx.h"
#include "qe3.h"
#include "mainfrm.h"
#include <cstring>

extern void Assert( const char *file, int line, int type, const char *fmt, ... );

// ── externs (per-TU, matching select.cpp / scriptgroup.cpp) ───────────────────
extern entity_s *world_entity;                                              // 0x25D5B30 (map.cpp)
extern void      SetKeyValue( entity_s_def *e, const char *key, const char *value ); // 0x483690 (entity.cpp)
extern void      DeleteKey( epair_t **head, const char *key );              // 0x483720 (entity.cpp)
extern void      Checkkey_Model( entity_s_def *e, const char *key );        // 0x482F70 (=Checkkey_Model_0)
extern void      Checkkey_Color( entity_s_def *e, const char *key );        // 0x483210 (entity.cpp)
extern void      Undo_ClearRedo();                                          // 0x45DF20 (undo.cpp)
extern void      Undo_GeneralStart( const char *op );                       // 0x45E3F0 (undo.cpp)
extern void      Undo_AddEntity_W( entity_s *e );                           // 0x45E990 (undo.cpp)
extern void      Undo_End();                                               // 0x45EA20 (undo.cpp)
extern void      SetKeyValuePairs();                                        // 0x496CF0 (win_ent.cpp)
extern int       g_nUpdateBits;                                            // 0x25D5A74 (engine_stubs.cpp)
// selected_brushes sentinel is declared in qe3.h.

// Is this selected-brush instance owned by a dyn_ entity we should edit?  Faithful to
// the SetPair/RemovePair loop guards (skip null owner, skip world_entity, require an
// eclass name starting with "dyn_"), with the §11 NULL-eclass/name guard added.  When
// it returns true, `*outDef` is the entity DEF that carries the epairs.
static bool DynEnt_SelectedDef( selbrush_t *i, entity_s_def **outDef )
{
    *outDef = nullptr;
    entity_s *owner = i->owner;
    if ( !owner || owner == world_entity )
        return false;

    entity_s_def *def = (entity_s_def *)owner->def;
    // The binary asserts (type 0 = log-and-continue) on a null eclass->name, then
    // dereferences it in strncmp.  Guard the whole chain (a real dyn_ entity always
    // has both) so a malformed selection never AVs on the never-run path.
    if ( !def || !def->eclass || !def->eclass->name )
    {
        if ( def && ( !def->eclass || !def->eclass->name ) )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\DynEntityDlg.cpp",
                    72, 0, "%s", "b->owner->def->eclass->name" );
        return false;
    }
    if ( strncmp( def->eclass->name, "dyn_", 4 ) != 0 )
        return false;

    // The binary's `b->owner->def == b->def->owner` invariant (type 0 → log+continue).
    if ( i->def && (entity_s_def *)def != (entity_s_def *)i->def->owner )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\DynEntityDlg.cpp",
                76, 0, "%s", "b->owner->def == b->def->owner" );

    *outDef = def;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x40DF20  DynEntityDlg_01_SetPair(value@<ebx>, key@<edi>) — SET `key`=`value` on
// every selected dyn_ entity's def (Undo-bracketed).  The binary asserts on a null
// value/key, then for each selected brush whose owner is a non-world dyn_ entity:
// Undo_AddEntity_W(def) + SetKeyValue(def, key, value).
// ─────────────────────────────────────────────────────────────────────────────
void DynEntityDlg_01_SetPair( const char *value, const char *key )
{
    iassert(key);
    iassert(value);

    Undo_ClearRedo();
    Undo_GeneralStart( "Set key pair" );
    for ( selbrush_t *i = selected_brushes.prev; i != &selected_brushes; i = i->prev )
    {
        entity_s_def *def = nullptr;
        if ( !DynEnt_SelectedDef( i, &def ) )
            continue;
        Undo_AddEntity_W( (entity_s *)def );
        SetKeyValue( def, key, value );
    }
    Undo_End();
    SetKeyValuePairs();
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x40E040  DynEntityDlg_02_RemovePair(key@<eax>) — REMOVE `key` from every selected
// dyn_ entity's def (Undo-bracketed).  DeleteKey(&def->epairs, key) then the post-
// delete revalidation Checkkey_Model_0 + Checkkey_Color (SetKeyValue fires those on a
// SET, but DeleteKey does not, so RemovePair calls them explicitly — verified vs the
// disasm at 0x40E11F..0x40E12F).
// ─────────────────────────────────────────────────────────────────────────────
void DynEntityDlg_02_RemovePair( const char *key )
{
    iassert(key);

    Undo_ClearRedo();
    Undo_GeneralStart( "Remove key pair" );
    for ( selbrush_t *i = selected_brushes.prev; i != &selected_brushes; i = i->prev )
    {
        entity_s_def *def = nullptr;
        if ( !DynEnt_SelectedDef( i, &def ) )
            continue;
        Undo_AddEntity_W( (entity_s *)def );
        DeleteKey( &def->epairs, key );      // entity_s::epairs @0x74
        Checkkey_Model( def, key );          // 0x482F70 (Checkkey_Model_0)
        Checkkey_Color( def, key );          // 0x483210
    }
    Undo_End();
    SetKeyValuePairs();
}

// ══════════════════════════════════════════════════════════════════════════════
//  CDynEntityDlg — the hand-built modeless popup (CLayerDlg / CSurfaceDlg pattern)
// ══════════════════════════════════════════════════════════════════════════════
//
// One row per dyn-entity key: a label + an input (combobox for "type", edit for the
// rest) + [Set] [Clear] (and [Browse] for the file-backed keys).  Set pushes the
// field value onto the selected dyn_ entities (SetPair); Clear removes the key
// (RemovePair); Browse pops the file picker over the key's asset dir and writes the
// relative path back into the field.  Help shows the picked entity's eclass comments.

enum
{
    IDC_DE_TYPE = 4100,     // type combobox
    IDC_DE_HEALTH,          // health edit
    IDC_DE_PHYS,            // physPreset edit
    IDC_DE_EFX,             // destroyEfx edit
    IDC_DE_PIECES,          // destroyPieces edit

    IDC_DE_SET_TYPE,        // per-field Set buttons
    IDC_DE_SET_HEALTH,
    IDC_DE_SET_PHYS,
    IDC_DE_SET_EFX,
    IDC_DE_SET_PIECES,

    IDC_DE_CLR_TYPE,        // per-field Clear (remove key) buttons
    IDC_DE_CLR_HEALTH,
    IDC_DE_CLR_PHYS,
    IDC_DE_CLR_EFX,
    IDC_DE_CLR_PIECES,

    IDC_DE_BR_PHYS,         // Browse buttons (file-backed keys)
    IDC_DE_BR_EFX,
    IDC_DE_BR_PIECES,

    IDC_DE_HELP,            // Help
};

CDynEntityDlg *g_dlgDynEnt = nullptr;       // the singleton (NULL until first opened)

static HFONT s_deFont   = nullptr;
static HWND  s_deType   = nullptr;
static HWND  s_deHealth = nullptr;
static HWND  s_dePhys   = nullptr;
static HWND  s_deEfx    = nullptr;
static HWND  s_dePieces = nullptr;

static HWND DE_MakeChild( HWND parent, const char *cls, DWORD style, int id,
                          int x, int y, int w, int hgt, const char *text = nullptr )
{
    HWND h = CreateWindowExA( 0, cls, text, WS_CHILD | WS_VISIBLE | style,
                              x, y, w, hgt, parent, (HMENU)(INT_PTR)id,
                              AfxGetInstanceHandle(), NULL );
    if ( h && s_deFont )
        SendMessageA( h, WM_SETFONT, (WPARAM)s_deFont, 0 );
    return h;
}

BEGIN_MESSAGE_MAP( CDynEntityDlg, CWnd )
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_BN_CLICKED( IDC_DE_SET_TYPE,   &CDynEntityDlg::OnSetType )
    ON_BN_CLICKED( IDC_DE_SET_HEALTH, &CDynEntityDlg::OnSetHealth )
    ON_BN_CLICKED( IDC_DE_SET_PHYS,   &CDynEntityDlg::OnSetPhys )
    ON_BN_CLICKED( IDC_DE_SET_EFX,    &CDynEntityDlg::OnSetEfx )
    ON_BN_CLICKED( IDC_DE_SET_PIECES, &CDynEntityDlg::OnSetPieces )
    ON_BN_CLICKED( IDC_DE_CLR_TYPE,   &CDynEntityDlg::OnClearType )
    ON_BN_CLICKED( IDC_DE_CLR_HEALTH, &CDynEntityDlg::OnClearHealth )
    ON_BN_CLICKED( IDC_DE_CLR_PHYS,   &CDynEntityDlg::OnClearPhys )
    ON_BN_CLICKED( IDC_DE_CLR_EFX,    &CDynEntityDlg::OnClearEfx )
    ON_BN_CLICKED( IDC_DE_CLR_PIECES, &CDynEntityDlg::OnClearPieces )
    ON_BN_CLICKED( IDC_DE_BR_PHYS,    &CDynEntityDlg::OnBrowsePhys )
    ON_BN_CLICKED( IDC_DE_BR_EFX,     &CDynEntityDlg::OnBrowseEfx )
    ON_BN_CLICKED( IDC_DE_BR_PIECES,  &CDynEntityDlg::OnBrowsePieces )
    ON_BN_CLICKED( IDC_DE_HELP,       &CDynEntityDlg::OnHelp )
END_MESSAGE_MAP()

CDynEntityDlg::CDynEntityDlg()
{
}

int CDynEntityDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;
    if ( !s_deFont )
        s_deFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

    HWND self = GetSafeHwnd();
    const int M = 10, lblW = 90, inW = 200, btnW = 56, brW = 60, rowH = 26;
    int lx = M;
    int ix = M + lblW + 4;
    int setx = ix + inW + 6;
    int clrx = setx + btnW + 4;
    int brx  = clrx + btnW + 4;
    int y = M;

    // type — a combobox seeded with the two presets (dyn_model's Valid Keys).
    DE_MakeChild( self, "static", SS_LEFT, 0, lx, y + 4, lblW, 16, "type:" );
    s_deType = DE_MakeChild( self, "combobox",
                             CBS_DROPDOWN | WS_VSCROLL | WS_TABSTOP,
                             IDC_DE_TYPE, ix, y, inW, 120 );
    if ( s_deType )
    {
        ::SendMessageA( s_deType, CB_ADDSTRING, 0, (LPARAM)"clutter" );
        ::SendMessageA( s_deType, CB_ADDSTRING, 0, (LPARAM)"destruct" );
    }
    DE_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_DE_SET_TYPE, setx, y, btnW, 22, "Set" );
    DE_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_DE_CLR_TYPE, clrx, y, btnW, 22, "Clear" );
    y += rowH;

    // health
    DE_MakeChild( self, "static", SS_LEFT, 0, lx, y + 4, lblW, 16, "health:" );
    s_deHealth = DE_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL, IDC_DE_HEALTH, ix, y, inW, 22 );
    DE_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_DE_SET_HEALTH, setx, y, btnW, 22, "Set" );
    DE_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_DE_CLR_HEALTH, clrx, y, btnW, 22, "Clear" );
    y += rowH;

    // physPreset (Browse → main_shared\physic\)
    DE_MakeChild( self, "static", SS_LEFT, 0, lx, y + 4, lblW, 16, "physPreset:" );
    s_dePhys = DE_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL, IDC_DE_PHYS, ix, y, inW, 22 );
    DE_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_DE_SET_PHYS, setx, y, btnW, 22, "Set" );
    DE_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_DE_CLR_PHYS, clrx, y, btnW, 22, "Clear" );
    DE_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_DE_BR_PHYS,  brx,  y, brW,  22, "Browse" );
    y += rowH;

    // destroyEfx (Browse → raw_shared\fx\, *.efx)
    DE_MakeChild( self, "static", SS_LEFT, 0, lx, y + 4, lblW, 16, "destroyEfx:" );
    s_deEfx = DE_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL, IDC_DE_EFX, ix, y, inW, 22 );
    DE_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_DE_SET_EFX, setx, y, btnW, 22, "Set" );
    DE_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_DE_CLR_EFX, clrx, y, btnW, 22, "Clear" );
    DE_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_DE_BR_EFX,  brx,  y, brW,  22, "Browse" );
    y += rowH;

    // destroyPieces (Browse → main_shared\xmodelpieces\)
    DE_MakeChild( self, "static", SS_LEFT, 0, lx, y + 4, lblW, 16, "destroyPieces:" );
    s_dePieces = DE_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL, IDC_DE_PIECES, ix, y, inW, 22 );
    DE_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_DE_SET_PIECES, setx, y, btnW, 22, "Set" );
    DE_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_DE_CLR_PIECES, clrx, y, btnW, 22, "Clear" );
    DE_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_DE_BR_PIECES,  brx,  y, brW,  22, "Browse" );
    y += rowH + 6;

    DE_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_DE_HELP, lx, y, btnW, 22, "Help" );
    return 0;
}

// ── per-field commit: read the control, SetPair (non-empty) / RemovePair (empty) ──
// Faithful to each edit handler (0x40E760 etc.): GetWindowText → empty? RemovePair :
// SetPair.  (The binary's per-field control also has dedicated Clear buttons that
// always RemovePair; OnClear* below provide those.)
static void DE_Commit( HWND field, const char *key )
{
    if ( !field )
        return;
    char buf[1024] = { 0 };
    ::GetWindowTextA( field, buf, sizeof( buf ) - 1 );
    if ( buf[0] )
        DynEntityDlg_01_SetPair( buf, key );
    else
        DynEntityDlg_02_RemovePair( key );
    g_nUpdateBits |= 1;
}

// The type combo handler (0x40E6C0): require a selection, else "Type is not selected".
void CDynEntityDlg::OnSetType()
{
    if ( !s_deType )
        return;
    char buf[256] = { 0 };
    int cur = (int)::SendMessageA( s_deType, CB_GETCURSEL, 0, 0 );
    if ( cur == CB_ERR )
    {
        // Allow a typed (dropdown-edit) value too; fall back to the message only if the
        // edit is empty as well — the binary uses CB_GETLBTEXT(cursel) and errors when
        // cursel == -1, but a CBS_DROPDOWN also lets the user type a preset.
        ::GetWindowTextA( s_deType, buf, sizeof( buf ) - 1 );
        if ( !buf[0] )
        {
            MessageBoxA( "Could not complete operation.\n\nType is not selected",
                         "Radiant", MB_OK | MB_ICONEXCLAMATION );
            return;
        }
    }
    else
    {
        ::SendMessageA( s_deType, CB_GETLBTEXT, cur, (LPARAM)buf );
    }
    DynEntityDlg_01_SetPair( buf, "type" );
    g_nUpdateBits |= 1;
}

void CDynEntityDlg::OnSetHealth() { DE_Commit( s_deHealth, "health" ); }
void CDynEntityDlg::OnSetPhys()   { DE_Commit( s_dePhys,   "physPreset" ); }
void CDynEntityDlg::OnSetEfx()    { DE_Commit( s_deEfx,    "destroyEfx" ); }
void CDynEntityDlg::OnSetPieces() { DE_Commit( s_dePieces, "destroyPieces" ); }

void CDynEntityDlg::OnClearType()   { DynEntityDlg_02_RemovePair( "type" );          g_nUpdateBits |= 1; }
void CDynEntityDlg::OnClearHealth() { DynEntityDlg_02_RemovePair( "health" );        g_nUpdateBits |= 1; }
void CDynEntityDlg::OnClearPhys()   { DynEntityDlg_02_RemovePair( "physPreset" );    g_nUpdateBits |= 1; }
void CDynEntityDlg::OnClearEfx()    { DynEntityDlg_02_RemovePair( "destroyEfx" );    g_nUpdateBits |= 1; }
void CDynEntityDlg::OnClearPieces() { DynEntityDlg_02_RemovePair( "destroyPieces" ); g_nUpdateBits |= 1; }

#ifndef RADIANT_SELFTEST
// ── the file-picker helper (GUI-only) ────────────────────────────────────────
// 0x40E150  DynEntityDlg_OpenDialog(parentWnd, editControl, subdir, ofnFlags) — pick a
// file under <project basepath>\<subdir>; if the pick is NOT under that directory pop
// "File must be under [<dir>]"; else strip the directory prefix and SetWindowText the
// relative path into `editControl`.  Returns true on a kept pick.  Modelled on the
// proven CEntityWnd_OpenModelDialog (the same project-basepath lookup + CFileDialog).
static bool DynEntityDlg_OpenDialog( CWnd *parent, CWnd *dialog,
                                     const char *subdir, const char *filter )
{
    iassert(dialog);

    // Initial / base directory = <project "basepath" value>\<subdir>, with a trailing
    // '\' before the subdir (the binary appends a '\' if the basepath lacks one).
    CString baseDir;
    if ( g_qeglobals.d_project_entity )
    {
        for ( epair_t *ep = g_qeglobals.d_project_entity->epairs; ep; ep = ep->next )
            if ( _stricmp( ep->key, "basepath" ) == 0 ) { baseDir = ep->value; break; }
    }
    if ( !baseDir.IsEmpty() && baseDir[baseDir.GetLength() - 1] != '\\' )
        baseDir += '\\';
    CString initialDir = baseDir + subdir;

    CFileDialog dlg( TRUE, nullptr, nullptr,
                     OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, filter, parent );
    dlg.m_ofn.lpstrInitialDir = initialDir;
    if ( dlg.DoModal() != IDOK )
        return false;

    // The pick must live under <basepath>\<subdir> (case-insensitive prefix, the
    // binary's _strnicmp(picked, base+sub, strlen(base+sub))).
    CString picked = dlg.GetPathName();
    int prefixLen = initialDir.GetLength();
    if ( _strnicmp( picked, initialDir, prefixLen ) != 0 )
    {
        CString msg = "Could not complete operation.\n\nFile must be under ["
                    + initialDir + "]";
        if ( parent )
            parent->MessageBoxA( msg, "Radiant", MB_OK | MB_ICONEXCLAMATION );
        return false;
    }

    // Strip the directory prefix → the relative path the key stores, then push it into
    // the field (the binary's sub_40EDB0 Mid + SetWindowText).
    CString rel = picked.Mid( prefixLen );
    dialog->SetWindowTextA( rel );
    return true;
}

void CDynEntityDlg::OnBrowsePhys()
{
    if ( DynEntityDlg_OpenDialog( this, CWnd::FromHandle( s_dePhys ),
                                  "main_shared\\physic\\", "All files (*.*)|*.*||" ) )
        OnSetPhys();
}
void CDynEntityDlg::OnBrowseEfx()
{
    if ( DynEntityDlg_OpenDialog( this, CWnd::FromHandle( s_deEfx ),
                                  "raw_shared\\fx\\", "EFX files (*.efx)|*.efx||" ) )
        OnSetEfx();
}
void CDynEntityDlg::OnBrowsePieces()
{
    if ( DynEntityDlg_OpenDialog( this, CWnd::FromHandle( s_dePieces ),
                                  "main_shared\\xmodelpieces\\", "All files (*.*)|*.*||" ) )
        OnSetPieces();
}
#else
// Headless: the file-dialog browse is GUI-only (gui_smoke / monkey cover the window
// plumbing).  The Browse handlers are no-ops in the selftest variant.
void CDynEntityDlg::OnBrowsePhys()   {}
void CDynEntityDlg::OnBrowseEfx()    {}
void CDynEntityDlg::OnBrowsePieces() {}
#endif // RADIANT_SELFTEST

// Help (0x40E5C0): show the first selected dyn_ entity's eclass comments (the QUAKED
// doc block) in a message box, like the binary's "Radiant - Help".
void CDynEntityDlg::OnHelp()
{
    for ( selbrush_t *i = selected_brushes.prev; i != &selected_brushes; i = i->prev )
    {
        entity_s_def *def = nullptr;
        if ( !DynEnt_SelectedDef( i, &def ) )
            continue;
        if ( def->eclass && def->eclass->comments )
            MessageBoxA( def->eclass->comments, "Radiant - Help", MB_OK | MB_ICONINFORMATION );
        return;
    }
}

void CDynEntityDlg::OnClose()
{
    // Modeless: hide rather than destroy (OnMiscDynEntities toggles visibility).
    ShowWindow( SW_HIDE );
}

void CDynEntityDlg::PostNcDestroy()
{
    g_dlgDynEnt = nullptr;
    s_deType = s_deHealth = s_dePhys = s_deEfx = s_dePieces = nullptr;
    delete this;
}

// 0x42BD90  CMainFrame::OnMiscDynEntities toggles this; first call creates it.
void CDynEntityDlg::Toggle()
{
    if ( g_dlgDynEnt && ::IsWindow( g_dlgDynEnt->GetSafeHwnd() ) )
    {
        BOOL vis = g_dlgDynEnt->IsWindowVisible();
        g_dlgDynEnt->ShowWindow( vis ? SW_HIDE : SW_SHOW );
        if ( !vis )
            g_dlgDynEnt->SetForegroundWindow();
        g_nUpdateBits |= 1;
        return;
    }

    CWnd *parent = AfxGetMainWnd();
    g_dlgDynEnt = new CDynEntityDlg();

    const DWORD style   = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    const DWORD exStyle = WS_EX_TOOLWINDOW;
    int px = 220, py = 180;
    if ( parent && parent->GetSafeHwnd() )
    {
        RECT r;  parent->GetWindowRect( &r );
        px = r.left + 140;  py = r.top + 120;
    }
    CRect rc( px, py, px + 470, py + 210 );
    if ( !g_dlgDynEnt->CreateEx( exStyle,
             AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW,
                 ::LoadCursor( NULL, IDC_ARROW ), (HBRUSH)( COLOR_BTNFACE + 1 ), NULL ),
             "Dyn Entities", style, rc, parent, 0 ) )
    {
        delete g_dlgDynEnt;
        g_dlgDynEnt = nullptr;
        return;
    }
    g_nUpdateBits |= 1;
}

#ifdef RADIANT_SELFTEST
// ══════════════════════════════════════════════════════════════════════════════
//  Deterministic test entry (the `dynent` gate).  Drives the dialog's CORE writers
//  WITHOUT any HWNDs — exactly as the entwnd/scriptgrp gates drive AddProp /
//  ScriptGroup_AddKeyToSelected on a selected entity directly:
//    SetPair(value, key) / RemovePair(key) on whatever dyn_ entity is selected.
//  RunDynEntTest (radiantapp.cpp) creates a dyn_model, selects it, sets the keys,
//  saves + reloads, and asserts they round-trip.
// ══════════════════════════════════════════════════════════════════════════════
extern "C" int Radiant_TestDynEntSetPair( const char *value, const char *key )
{
    DynEntityDlg_01_SetPair( value, key );
    return 0;
}
extern "C" int Radiant_TestDynEntRemovePair( const char *key )
{
    DynEntityDlg_02_RemovePair( key );
    return 0;
}
#endif // RADIANT_SELFTEST
