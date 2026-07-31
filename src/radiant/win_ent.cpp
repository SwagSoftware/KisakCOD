#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\win_ent.cpp
//
// THE ENTITY WINDOW (CEntityWnd) — the entity inspector.  Two halves of the QE4
// lineage, faithful to the CoD4Radiant IDB cluster (0x496370..0x4981f0):
//   * the eclass LIST  (FillClassList 0x496800, from g_eclass)
//   * the KEY/VALUE editor (SetKeyValuePairs 0x496cf0 / AddProp 0x497490 /
//     DelProp 0x4975c0 / EditProp 0x4977b0 + the key/value edit fields)
//   * create-any-eclass (CreateEntity 0x497300 → Entity_Create)
//   * UpdateSelection (0x497180) — the selection→inspector refresh, driven from
//     Brush_AddToList2 / Brush_RemoveFromList / Select_Deselect.
//
// The binary builds the controls from a CreateDialog(IDD_ENTITY) template +
// SetParent (GetEntityControls 0x4964b0).  radiant.rc carries no dialog templates
// (only the reconstructed menu/accelerators), so the window PLUMBING here is MFC:
// CEntityWnd : CWnd creates the child controls (two listboxes + two edit fields +
// a Delete button + the 12 spawnflag checkboxes + the 10 angle buttons) directly and
// routes their notifications.  The DATA operations transcribe the IDB verbatim:
//   * the 12 SPAWNFLAG checkboxes (SetSpawnFlags 0x496e70 / _R 0x496f00 / _2 0x497040)
//     — each box toggles a bit in the entity's decimal "spawnflags" key; the first 8
//     labels come from the eclass flagname0..7.
//   * the ANGLE-button grid (Entity_SetAngles 0x494030, already in select.cpp) — the
//     8 compass buttons set the "angles" YAW, up/down toggle the PITCH to ∓90.
// The model/prefab picker (OpenDialog / IDC_E_ADD_MODEL) and the faithful SizeEntityDlg
// pixel layout both SHIPPED; only the binary's CModelFileDialog preview PANE is still out
// (a plain CFileDialog gives the identical pick — see CEntityWnd_OpenModelDialog).
//
// NO TAB STRIP.  The binary's inspector DOES carry a mode-switch tab (a CTabCtrl
// g_wndTabsEntWnd, subclassed from IDC_E_TAB_CONTROL in the IDD_ENTITY dialog resource —
// hence no "SysTabControl32" CreateWindowExA class string in the IDB), with the panes
// column-swapped in/out by SizeEntityDlg (0x4978a0)'s entwnd_col trick.  This port has no
// dialog resource to subclass, so an earlier revision INVENTED a raw WC_TABCONTROLA tab.
// That invented tab (created in every mode with a fixed top inset) is the "messed up / has
// tabs" UX the operator flagged.  It is REMOVED: mode switching stays on the N/O/T/F
// hotkeys + the View menu (CEntityWnd_SetInspectorMode), and each mode lays its own
// controls across the full client area from the top (no inset).  This matches both the
// operator's request and the binary's per-mode single-pane presentation as closely as a
// resource-less hand-built window allows.

#include "stdafx.h"
#include <afxdlgs.h>      // CFileDialog (the model/prefab picker) — TU-local, NOT in the
                          // shared PCH: putting it in stdafx.h reorders <windows.h> ahead of
                          // the DXSDK headers and breaks every gfx_d3d TU (DXGI redefinitions).
#include "qe3.h"
#include "mainfrm.h"

// ── radiant-local deps (home files) ───────────────────────────────────────────
extern selbrush_t  selected_brushes;                 // engine_stubs (0x23F1864)
extern entity_s   *world_entity;                      // map.cpp      (0x25D5B30)
extern eclass_t   *g_eclass;                          // eclass.cpp   (0x25D5B20)
extern int         g_nUpdateBits;                     // engine_stubs (0x25D5A74)

extern char       *TranslateString( char *buf );      // win_qe3.cpp  (0x499CE0)
extern eclass_t   *Eclass_ForName( int has_brushes, const char *name ); // eclass.cpp (0x482190)
extern entity_s   *Entity_Create( eclass_t *eclass ); // entity.cpp   (0x484980)

extern void        SetKeyValue( entity_s_def *e, const char *key, const char *value ); // entity.cpp 0x483690
extern void        DeleteKey( epair_t **head, const char *key );                       // entity.cpp 0x483720
extern char       *ValueForKey2( int e, const char *key );                             // entity.cpp 0x4825C0
extern int         Entity_GetVec3ForKey( entity_s_def *e, float *out, const char *key );// entity.cpp 0x483860
extern void        Checkkey_Model( entity_s_def *e, const char *key );                 // entity.cpp 0x482F70
extern void        Checkkey_Color( entity_s_def *e, const char *key );                 // entity.cpp 0x483210

extern void        Select_Deselect( int a1 );                                          // select.cpp 0x48E800
extern void        Select_Brush( selbrush_t *b, char ovw, char status, char center );  // select.cpp 0x48DCC0

extern void        Undo_ClearRedo();                                                   // undo.cpp 0x45DF20
extern void        Undo_GeneralStart( const char *operation );                         // undo.cpp 0x45E3F0
extern void        Undo_AddEntity_W( entity_s *e );                                    // undo.cpp 0x45E990
extern void        Undo_End();                                                         // undo.cpp 0x45EA20

extern void        Entity_SetAngles( float a1, int axis );                             // select.cpp 0x494030

extern void        ScriptGroup_HasFlag( const char *key, int dlgItemID, HWND hDlg );   // scriptgroup.cpp 0x454E40

extern CMainFrame *g_pParentWnd;                                                       // engine_stubs.cpp (CFileDialog parent)

// ── entity-window globals (mirror the IDB file-scope state @ 0x240A1xx) ────────
entity_s_def *edit_entity            = nullptr;   // 0x240A108  currently-edited def
int           multiple_edit_entities = 0;         // 0x240A10C  >1 entity selected

// inspector_mode (IDB 0x240A110): the active inspector pane.  Initialised to
// INSPECTOR_ENTITY (the binary's default — the entity editor is shown at startup).
int           inspector_mode         = INSPECTOR_ENTITY;

static HWND   hwndEnt_entlist        = nullptr;   // 0x240A118  eclass listbox
static HWND   entwnd_comment         = nullptr;   // 0x240A11C  eclass description (read-only)
static HWND   entwnd_kvlist          = nullptr;   // 0x240A150  key/value listbox
static HWND   entwnd_keyfield        = nullptr;   // 0x240A184  key edit field
static HWND   entwnd_valuefield      = nullptr;   // 0x240A18C  value edit field
static HWND   entwnd_keylabel        = nullptr;   // "Key" static
static HWND   entwnd_valuelabel      = nullptr;   // "Value" static
static HWND   entwnd_delbtn          = nullptr;   // "Delete Key" button
static HWND   entwnd_addmodelbtn     = nullptr;   // "Add Model" button (IDC_E_ADD_MODEL)
static WNDPROC OldFieldWindowProc    = nullptr;   // 0x240A0FC  subclassed edit wndproc

// The 12 spawnflag checkbox HWNDs (IDB entwnd_entcheck_1..8 + _easy/_medium/_hard/
// _deathmatch, the contiguous slice 0x240A120..0x240A14C of the GetEntityControls
// HWND array, ENTITY_DEFINES EntCheck1..EntCheck12).  SetSpawnFlags* index this by
// bit (0..11); UpdateSelection labels the first 8 from the eclass flagname0..7.
static HWND   entwnd_entcheck[12]    = { nullptr };

// The 10 angle/direction button HWNDs (IDB entwnd_entdir0/45/90/135/180/225/270/315/
// up/down, ENTITY_DEFINES EntDir0..EntDirDown).  Each maps to an Entity_SetAngles call.
static HWND   entwnd_entdir[10]      = { nullptr };

// ── CFilterWnd controls (the Filters inspector pane — built into this window) ──
// Faithful to the binary's CFilterWnd: four category CHECKLISTS + six simple show-flag
// CHECKBOXES.  The four category lists are OWNER-DRAWN listboxes that render a real checkbox
// glyph (DrawFrameControl) per row and toggle on a SINGLE click (via a small subclass).  The
// binary uses MFC CCheckListBox, but that class's owner-draw state init asserts in this
// static-MFC/MBCS build — this hand-drawn equivalent gives the same look + single-click feel.
static HWND   filt_catlabel[4]       = { nullptr };   // "Geometry" / "Trigger" / "Entity" / "Other"
static HWND   filt_catlist[4]        = { nullptr };   // the four owner-drawn category filter lists
static HWND   filt_flagcheck[6]      = { nullptr };   // the six d_xyShowFlags checkboxes

// TASK 2 — the eclass name currently picked in the list (RMB create reads this; the
// hardcoded "light" default applies until the list has a selection).
static char   s_selectedEclass[256]  = "light";

// CEntityWnd control IDs (this hand-built window uses its own ID space — the binary's
// reconstructed resource IDs are not used for these MFC-created children).  Declared
// here (not in the class block below) because the model-picker helpers above
// reference IDC_ENT_ADD_MODEL_BTN.
enum
{
    IDC_ENT_ECLASS_LIST = 1801,
    IDC_ENT_COMMENT,
    IDC_ENT_KV_LIST,
    IDC_ENT_KEY_FIELD,
    IDC_ENT_VALUE_FIELD,
    IDC_ENT_DELETE_BTN,
    IDC_ENT_KEY_LABEL,
    IDC_ENT_VALUE_LABEL,
    IDC_ENT_ADD_MODEL_BTN,                              // the "Add Model" button (1809)
    // 12 contiguous spawnflag-checkbox IDs (index = ID - IDC_ENT_CHECK_FIRST = bit).
    IDC_ENT_CHECK_FIRST  = 1810,
    IDC_ENT_CHECK_LAST   = IDC_ENT_CHECK_FIRST + 11,   // 1821
    // 10 contiguous angle/direction button IDs (index = ID - IDC_ENT_DIR_FIRST,
    // matching ENTITY_DEFINES EntDir0..EntDirDown).
    IDC_ENT_DIR_FIRST    = 1822,
    IDC_ENT_DIR_LAST     = IDC_ENT_DIR_FIRST + 9,      // 1831

    // ── CFilterWnd controls (the Filters inspector pane) ──────────────────────
    // 4 contiguous category checklists (geometry/trigger/entity/other), index =
    // ID - IDC_FILT_LIST_FIRST = category (matching CFilterWnd_GetCategoryHead order).
    IDC_FILT_LIST_FIRST  = 1840,
    IDC_FILT_LIST_LAST   = IDC_FILT_LIST_FIRST + 3,    // 1843
    // 6 contiguous simple show-flag checkboxes (Angles/Connections/Names/Blocks/
    // Coordinates/Reverse Filter), index = ID - IDC_FILT_FLAG_FIRST.
    IDC_FILT_FLAG_FIRST  = 1844,
    IDC_FILT_FLAG_LAST   = IDC_FILT_FLAG_FIRST + 5,    // 1849
};

// ──────────────────────────────────────────────────────────────────────────────
//  Data operations — faithful ports of the IDB win_ent.cpp cluster.
// ──────────────────────────────────────────────────────────────────────────────

// FillClassList (0x496800) — reset + repopulate the eclass listbox from g_eclass,
// stashing each eclass_t* as the item data (so selchange/create can recover it).
static void FillClassList()
{
    if ( !hwndEnt_entlist )
        return;
    SendMessageA( hwndEnt_entlist, LB_RESETCONTENT, 0, 0 );
    for ( eclass_t *pec = g_eclass; pec; pec = pec->next )
    {
        LRESULT idx = SendMessageA( hwndEnt_entlist, LB_ADDSTRING, 0, (LPARAM)pec->name );
        SendMessageA( hwndEnt_entlist, LB_SETITEMDATA, idx, (LPARAM)pec );
    }
}

// ═══ ENTITY KEY/VALUE CLUSTER — AUDIT: all FAITHFUL vs IDA (2026-06-25) ════════════════════════
//  SetKeyValuePairs 0x496cf0: LB_SETCOLUMNWIDTH(width/2) + LB_RESETCONTENT; epair loop skip "origin",
//   strlen(key)<=8 → "%s\t\t%s" else "%s\t%s", LB_ADDSTRING; non-zero origin → "origin\t\t%g %g %g";
//   g_nUpdateBits |= W_CAMERA|W_XY. AddProp 0x497490: key/value WM_GETTEXT(4095) + Undo bracket("set
//   key value pair") + multi per-entity / single Undo_AddEntity_W+SetKeyValue (single + "origin" key →
//   Entity_GetVec3ForKey + ++version[16-bit]). DelProp 0x4975c0: WM_GETTEXT(0xFFF), "classname" →
//   MessageBox-block, else Undo bracket + Undo_AddEntity_W + DeleteKey(&def->epairs) + Checkkey_Model
//   (0x482F70) + Checkkey_Color (0x483210). EditProp 0x4977b0: LB_GETCURSEL/GETTEXT, split on first
//   '\t' → key + (skip 2nd '\t') value into the WM_SETTEXT fields. ALL EXACT (entwnd_kvlist null-guard
//   in SetKeyValuePairs = the only port-added defensive). The hand-built dialog plumbing is the only
//   reconstruction; the logic above is 1:1.
// ════════════════════════════════════════════════════════════════════════════════════════════
// SetKeyValuePairs (0x496cf0) — repopulate the key/value listbox from edit_entity's
// epairs (skipping "origin", which is appended specially from the def's origin vec3).
void SetKeyValuePairs()
{
    if ( !edit_entity || !entwnd_kvlist )
        return;

    RECT rc;
    GetWindowRect( entwnd_kvlist, &rc );
    SendMessageA( entwnd_kvlist, LB_SETCOLUMNWIDTH, ( rc.right - rc.left ) / 2, 0 );
    SendMessageA( entwnd_kvlist, LB_RESETCONTENT, 0, 0 );

    char sz[4100];
    for ( epair_t *ep = edit_entity->epairs; ep; ep = ep->next )
    {
        if ( !strcmp( ep->key, "origin" ) )
            continue;
        // <=8-char keys get a second tab so the value column lines up.
        if ( strlen( ep->key ) <= 8 )
            sprintf( sz, "%s\t\t%s", ep->key, ep->value );
        else
            sprintf( sz, "%s\t%s", ep->key, ep->value );
        SendMessageA( entwnd_kvlist, LB_ADDSTRING, 0, (LPARAM)sz );
    }

    if ( edit_entity->origin[0] != 0.0f || edit_entity->origin[1] != 0.0f || edit_entity->origin[2] != 0.0f )
    {
        sprintf( sz, "origin\t\t%g %g %g",
                 edit_entity->origin[0], edit_entity->origin[1], edit_entity->origin[2] );
        SendMessageA( entwnd_kvlist, LB_ADDSTRING, 0, (LPARAM)sz );
    }

    g_nUpdateBits |= W_CAMERA | W_XY;
}

// AddProp (0x497490) — read the key/value fields, set the pair (Undo-bracketed) on the
// edited entity (or every selected entity when multiple), refresh the list.
void AddProp()
{
    if ( !edit_entity )
        return;

    char key[4100], value[4096];
    SendMessageA( entwnd_keyfield,   WM_GETTEXT, 4095, (LPARAM)key   );
    SendMessageA( entwnd_valuefield, WM_GETTEXT, 4095, (LPARAM)value );

    Undo_ClearRedo();
    Undo_GeneralStart( "set key value pair" );
    if ( multiple_edit_entities )
    {
        for ( selbrush_t *sb = selected_brushes.next; sb != &selected_brushes; sb = sb->next )
        {
            entity_s_def *def = (entity_s_def *)sb->owner->def;
            Undo_AddEntity_W( (entity_s *)def );
            SetKeyValue( def, key, value );
        }
    }
    else
    {
        Undo_AddEntity_W( (entity_s *)edit_entity );
        SetKeyValue( edit_entity, key, value );
        if ( !strcmp( "origin", key ) )
        {
            Entity_GetVec3ForKey( edit_entity, edit_entity->origin, "origin" );
            ++edit_entity->version;
        }
    }
    SetKeyValuePairs();
    Undo_End();
}

// Win_GetEntityKeyValueFields — copy the current text of the entity-window key/value
// edit fields.  This is the data the CKeyValueSelectDlg constructor (IDB sub_416760)
// pre-loads into its key/value CString members via SendMessage(...,WM_GETTEXT,...) on
// entwnd_keyfield / entwnd_valuefield.  Exposed so select.cpp's Select_ByKeyValue dialog
// (which lives in another TU) can seed its edit fields identically.  Buffers must hold
// at least 0x1000 bytes each (the binary reads up to 0xFFF chars per field).
void Win_GetEntityKeyValueFields( char *keyOut, char *valueOut )
{
    if ( keyOut )
    {
        keyOut[0] = 0;
        if ( entwnd_keyfield )
            SendMessageA( entwnd_keyfield, WM_GETTEXT, 0xFFF, (LPARAM)keyOut );
    }
    if ( valueOut )
    {
        valueOut[0] = 0;
        if ( entwnd_valuefield )
            SendMessageA( entwnd_valuefield, WM_GETTEXT, 0xFFF, (LPARAM)valueOut );
    }
}

// Win_SetEntityKeyValueFields — SetWindowTextA the entity-window key/value edit fields.
// The entity-color picker (CMainFrame::OnMiscSelectentitycolor, IDB 0x424C10) writes the
// chosen "_color"/"r g b" here before calling AddProp() to commit the pair.  entwnd_* are
// TU-static, so the caller reaches them through this accessor.
void Win_SetEntityKeyValueFields( const char *key, const char *value )
{
    if ( entwnd_valuefield && value )
        SetWindowTextA( entwnd_valuefield, value );
    if ( entwnd_keyfield && key )
        SetWindowTextA( entwnd_keyfield, key );
}

// DelProp (0x4975c0) — delete the key named in the key field (blocking "classname"),
// Undo-bracketed, with the binary's post-delete model/colour re-validation.
void DelProp()
{
    if ( !edit_entity )
        return;

    char key[4100];
    SendMessageA( entwnd_keyfield, WM_GETTEXT, 0xFFF, (LPARAM)key );

    if ( !_stricmp( key, "classname" ) )
    {
        MessageBoxA( GetActiveWindow(),
            "You can't delete the classname of an entity.  If you want to move this geometry to "
            "the world entity then select \"Ungroup Entity\" from the right-click menu.\n",
            "Radiant", MB_ICONEXCLAMATION );
        return;
    }

    Undo_ClearRedo();
    Undo_GeneralStart( "delete key value pair" );
    if ( multiple_edit_entities )
    {
        for ( selbrush_t *sb = selected_brushes.next; sb != &selected_brushes; sb = sb->next )
        {
            entity_s_def *def = (entity_s_def *)sb->owner->def;
            Undo_AddEntity_W( (entity_s *)def );
            DeleteKey( &def->epairs, key );
            Checkkey_Model( def, key );      // 0x482F70 (IDB stale-named "MapLoad_ParsePrefab")
            Checkkey_Color( def, key );      // 0x483210
        }
    }
    else
    {
        Undo_AddEntity_W( (entity_s *)edit_entity );
        DeleteKey( &edit_entity->epairs, key );
        Checkkey_Model( edit_entity, key );
        Checkkey_Color( edit_entity, key );
    }
    SetKeyValuePairs();
    Undo_End();
}

// EditProp (0x4977b0) — the selected key/value list item → split on the tab into the
// key + value edit fields (so it can be edited and re-added).
void EditProp()
{
    if ( !edit_entity )
        return;

    LRESULT i = SendMessageA( entwnd_kvlist, LB_GETCURSEL, 0, 0 );
    if ( i < 0 )
        return;

    char sz[4096];
    SendMessageA( entwnd_kvlist, LB_GETTEXT, i, (LPARAM)sz );

    int j = 0;
    while ( sz[j] != '\t' )
        ++j;
    char *val = &sz[j + 1];
    sz[j] = '\0';
    if ( *val == '\t' )
        ++val;

    SendMessageA( entwnd_keyfield,   WM_SETTEXT, 0, (LPARAM)sz  );
    SendMessageA( entwnd_valuefield, WM_SETTEXT, 0, (LPARAM)val );
}

// ──────────────────────────────────────────────────────────────────────────────
//  SPAWNFLAGS — the 12 checkboxes.  The "spawnflags" key is a decimal int of OR'd
//  bits; checkbox i toggles bit i.  Three faithful IDB ports:
//   * SetSpawnFlags    (0x496e70) — flags → checkboxes (reflect on selection).
//   * SetSpawnFlags_R  (0x496f00) — one checkbox toggled → flags (per-entity, keeps
//                                   the OTHER bits when several entities are selected).
//   * SetSpawnFlags_2  (0x497040) — rebuild flags from ALL 12 checkbox states.
//  SetSpawnFlags_R(i) is what each checkbox click calls; it delegates the single-edit
//  case to SetSpawnFlags_2 (so a single entity is rebuilt wholesale, multi-edit is
//  per-bit).  The spawnflags string is a plain decimal atol/sprintf("%i"); the
//  bit math is (v & (1<<i)) / (1<<i)|v / ~(1<<i)&v — transcribed verbatim.

// Return entity e's "spawnflags" value as an int (0 if absent), without disturbing it.
static __int32 SpawnFlags_Get( entity_s_def *e )
{
    for ( epair_t *ep = e->epairs; ep; ep = ep->next )
        if ( !_stricmp( ep->key, "spawnflags" ) )
            return (__int32)atol( ep->value );
    return 0;          // IDB: atol(zero) — the empty-CString sentinel
}

// SetSpawnFlags (0x496e70) — read edit_entity's spawnflags, push each bit into the
// matching checkbox's BM_SETCHECK state.
void SetSpawnFlags()
{
    if ( !edit_entity )
        return;
    __int32 flags = SpawnFlags_Get( edit_entity );
    for ( int i = 0; i < 12; ++i )
        SendMessageA( entwnd_entcheck[i], BM_SETCHECK, ( flags & ( 1 << i ) ) != 0, 0 );
}

// SetSpawnFlags_2 (0x497040) — collapse all 12 checkbox states into one int and write
// it as "spawnflags" on the edited entity (single) or every selected entity (multiple).
// 0x497040: vs IDA 0x497040 — flags = OR of (BM_GETCHECK<<i) for i in 0..11; sprintf "%i";
// Undo_ClearRedo + Undo_GeneralStart("set spawnflags"); multi → per-entity Undo_AddEntity_W (=
// the binary's inlined `if(g_lastundo) Undo_AddEntity + def->brushes.oprev brush-def walk /
// else Sys_Printf "no last undo"`, disasm-verified equiv) + SetKeyValue; single →
// Undo_AddEntity_W(edit_entity) + SetKeyValue; SetKeyValuePairs + Undo_End. SetSpawnFlags
// 0x496e70 (flags→12 BM_SETCHECK) + SpawnFlags_Get (atol of the spawnflags epair, ""
// sentinel→0) also match.
void SetSpawnFlags_2()
{
    int flags = 0;
    for ( int i = 0; i < 12; ++i )
        flags |= (int)SendMessageA( entwnd_entcheck[i], BM_GETCHECK, 0, 0 ) << i;

    char sz[32];
    sprintf( sz, "%i", flags );

    Undo_ClearRedo();
    Undo_GeneralStart( "set spawnflags" );
    if ( multiple_edit_entities )
    {
        for ( selbrush_t *sb = selected_brushes.next; sb != &selected_brushes; sb = sb->next )
        {
            entity_s_def *def = (entity_s_def *)sb->owner->def;
            // IDB inlines Undo_AddEntity + the entity's brush-def walk here; that is
            // exactly Undo_AddEntity_W (already disasm-verified, the right field/sentinel).
            Undo_AddEntity_W( (entity_s *)def );
            SetKeyValue( def, "spawnflags", sz );
        }
    }
    else
    {
        Undo_AddEntity_W( (entity_s *)edit_entity );
        SetKeyValue( edit_entity, "spawnflags", sz );
    }
    SetKeyValuePairs();
    Undo_End();
}

// SetSpawnFlags_R (0x496f00) — checkbox `bit` was toggled.  For a single entity this
// is just SetSpawnFlags_2 (rebuild from the boxes).  For multiple entities, only the
// toggled bit is changed PER ENTITY (their other bits, which can differ, are kept):
// read the entity's current flags, OR/AND-NOT bit `bit` from that checkbox's state.
// 0x496f00: vs IDA 0x496f00 — multi-edit per-entity bit math EXACT (v8 = BM_GETCHECK ?
// ((check<<bit)|flags) : (~(1<<bit)&flags); flags = atol(spawnflags epair) = SpawnFlags_Get;
// Undo_AddEntity_W + SetKeyValue per entity; SetKeyValuePairs + Undo_End). DIVERGENCE (port
// more correct than a latent quirk, documented): the binary calls Undo_ClearRedo +
// Undo_GeneralStart at the TOP (before the multiple/single split), so the single-entity path
// runs that outer bracket AND THEN SetSpawnFlags_2's own ClearRedo+GeneralStart+End — leaving
// the outer bracket ORPHANED (never Undo_End'd → an extra empty undo record). The port skips
// the outer bracket for the single case (early-return to SetSpawnFlags_2), producing the
// identical spawnflags result with clean undo bookkeeping. The multi-edit path's bracket is
// unchanged (matches the binary).
void SetSpawnFlags_R( int bit )
{
    if ( !multiple_edit_entities )
    {
        SetSpawnFlags_2();
        return;
    }

    Undo_ClearRedo();
    Undo_GeneralStart( "set spawnflags" );
    for ( selbrush_t *sb = selected_brushes.next; sb != &selected_brushes; sb = sb->next )
    {
        entity_s_def *def   = (entity_s_def *)sb->owner->def;
        __int32       flags = SpawnFlags_Get( def );
        // BM_GETCHECK on the toggled box: set the bit if checked, clear it otherwise.
        int checked = (int)SendMessageA( entwnd_entcheck[bit], BM_GETCHECK, 0, 0 );
        int v8 = checked ? ( ( checked << bit ) | flags ) : ( ~( 1 << bit ) & flags );

        char sz[32];
        sprintf( sz, "%i", v8 );
        Undo_AddEntity_W( (entity_s *)def );
        SetKeyValue( def, "spawnflags", sz );
    }
    SetKeyValuePairs();
    Undo_End();
}


// CreateEntity (0x497300) — create a new entity of the eclass selected in the list,
// using the currently-selected brush(es) as the proxy (Entity_Create), then re-select
// and refresh.  (Mirrors the binary; the GUI also reaches this via the eclass DBLCLK.)
void CreateEntity()
{
    if ( selected_brushes.next == &selected_brushes )
    {
        MessageBoxA( GetActiveWindow(),
            "Failed to create entity.\n\nYou must have a selected brush to create an entity",
            "Radiant", MB_ICONEXCLAMATION );
        return;
    }

    LRESULT i = SendMessageA( hwndEnt_entlist, LB_GETCURSEL, 0, 0 );
    if ( i < 0 )
    {
        MessageBoxA( GetActiveWindow(),
            "Failed to create entity.\n\nYou must have a selected class to create an entity",
            "Radiant", MB_ICONEXCLAMATION );
        return;
    }

    char name[1028];
    SendMessageA( hwndEnt_entlist, LB_GETTEXT, i, (LPARAM)name );

    if ( !_stricmp( name, "worldspawn" ) )
    {
        MessageBoxA( GetActiveWindow(),
            "Failed to create entity.\n\nCan't create an entity with worldspawn.",
            "Radiant", MB_ICONEXCLAMATION );
        return;
    }

    eclass_t *pecNew = Eclass_ForName( 0, name );
    // Both POINT (fixed-size) and BRUSH entities are supported: Entity_Create's brush-
    // entity reparent loop reparents the selected world brushes into a
    // func_*/script_brushmodel entity.  (Was gated with a "not supported" message while
    // the reparent loop was a FATAL stub.)
    if ( Entity_Create( pecNew ) )
    {
        if ( selected_brushes.next == &selected_brushes )
        {
            edit_entity = (entity_s_def *)world_entity->def;
        }
        else
        {
            edit_entity = (entity_s_def *)selected_brushes.next->owner->def;
            selbrush_t *first = selected_brushes.next;
            Select_Deselect( 1 );
            Select_Brush( first, 1, 1, 0 );
        }
        SetKeyValuePairs();
        g_nUpdateBits = -1;
    }
    else
    {
        MessageBoxA( GetActiveWindow(), "Failed to create entity.\n", "Radiant", MB_ICONEXCLAMATION );
    }
}

// UpdateSelection (0x497180) — the selection→inspector refresh.  edit_entity becomes
// the selected entity's def (or worldspawn if nothing selected); the eclass list cursor
// + description follow; the key/value list is rebuilt.  wParam==-1 means "derive the
// eclass from edit_entity" (selection change); otherwise it is an explicit list index
// (the eclass-list selchange).  The 8 spawnflag checkboxes are PARKED.
int UpdateSelection( int wParam, eclass_t *cls )
{
    if ( !hwndEnt_entlist )       // the window is not up (headless / selftest) — PORT guard
        return 1;

    // IDA 0x497180 reads selected_brushes.prev (the TAIL = the LAST-selected brush), NOT .next
    // (the head / first-selected): the inspector reflects the MOST-RECENTLY-selected entity.
    // [the port read .next (first-selected) — for a multi-ENTITY selection the
    //  inspector showed the wrong entity. selected_brushes is appended at .prev (Brush_AddToList2),
    //  so .prev is the last selection.]  The binary's first statement is the 623 list-init assert.
    iassert( selected_brushes.prev );   // win_ent.cpp:623 (L0) — RESTORED
    selbrush_t *sel = selected_brushes.prev;
    entity_s_def *ent;
    if ( sel == &selected_brushes )
    {
        ent = (entity_s_def *)world_entity->def;
        multiple_edit_entities = 0;
    }
    else
    {
        ent = (entity_s_def *)sel->owner->def;
        multiple_edit_entities = ( sel->prev != &selected_brushes );
    }
    edit_entity = ent;

    if ( wParam != -1
         || ( cls = ent->eclass,
              wParam = (int)SendMessageA( hwndEnt_entlist, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)cls->name ),
              wParam != -1 ) )
    {
        SendMessageA( hwndEnt_entlist, LB_SETCURSEL, wParam, 0 );
    }

    if ( cls )
    {
        if ( entwnd_comment )
            SendMessageA( entwnd_comment, WM_SETTEXT, 0, (LPARAM)TranslateString( cls->comments ) );

        // Label + enable the first 8 checkboxes from the eclass flag names
        // (flagname0..flagname7, 32-byte stride).  An empty name → blank " " +
        // disabled, exactly as the IDB loop (it stops at checkbox 8 = &entcheck_easy).
        // The remaining 4 (easy/medium/hard/deathmatch) keep their static labels.
        const char *flagname = cls->flagname0;
        for ( int i = 0; i < 8; ++i, flagname += 32 )
        {
            HWND checkbox = entwnd_entcheck[i];
            if ( flagname && *flagname )
            {
                EnableWindow( checkbox, TRUE );
                SendMessageA( checkbox, WM_SETTEXT, 0, (LPARAM)flagname );
            }
            else
            {
                SendMessageA( checkbox, WM_SETTEXT, 0, (LPARAM)" " );
                EnableWindow( checkbox, FALSE );
            }
        }
        SetSpawnFlags();                 // reflect this entity's spawnflags into the boxes
        SetKeyValuePairs();

        // IDA tail (0x4972a9) — RESTORED 2026-07-31. Reflects the selection's
        // script_flag_true/false keys into the script-group (media) window's two flag
        // listboxes. Was parked on two then-missing pieces; both landed with the
        // script-group unit: ScriptGroup_HasFlag (0x454e40, scriptgroup.cpp) and
        // d_hwndMedia (created by the script-group dialog's WM_INITDIALOG).
        // IsWindowVisible(NULL) is FALSE, so this stays inert headless, as in the binary.
        if ( ::IsWindowVisible( g_qeglobals.d_hwndMedia ) )
        {
            ScriptGroup_HasFlag( "script_flag_true",  1671, g_qeglobals.d_hwndMedia );
            ScriptGroup_HasFlag( "script_flag_false", 1298, g_qeglobals.d_hwndMedia );
        }
    }
    return 1;
}

// Public entry for the selection hooks (= UpdateSelection(-1, NULL)); a no-op when the
// entity window is not present (headless / selftest builds), so the round-trip gates
// keep their proven path.
void Entity_UpdateSelection()
{
    UpdateSelection( -1, nullptr );
}

// TASK 2 — the eclass name the entity window currently has picked (RMB create reads it).
const char *Ed_SelectedEclassName()
{
    return s_selectedEclass;
}

// ──────────────────────────────────────────────────────────────────────────────
//  THE MODEL / PREFAB PICKER — CEntityWnd_OpenModelDialog (0x497d20, IDB
//  "OpenDialog").  Wired to IDC_E_ADD_MODEL: when a misc_model/script_*/dyn_model
//  (or misc_prefab) entity is selected, the entity window pops a file dialog over
//  the model (or prefab) directory; the picked file is relativised to a bare xmodel
//  name and set as the entity's "model" epair (→ EntityAssignModel → the bbox).
//
//  The binary's flow (verified vs the disasm):
//   * read g_qeglobals.d_project_entity's `key` epair (basepath / mapspath) → the
//     initial directory = <value>\<subdir>;
//   * pop a CModelFileDialog (a CFileDialog subclass that adds a MODEL-PREVIEW pane
//     via a custom template — the preview pane is PARKED here; a plain CFileDialog
//     gives the identical pick result);
//   * on OK: lowercase the picked path + '\'→'/', then relativise — find the
//     `subdir`-tail token ("xmodel"/"prefabs") inside the path and drop everything
//     before it; for models (stripToBareName) also drop the leading "xmodel/" so the
//     stored value is the bare model name the asset system expects;
//   * SendMessage "model" → key field, name → value field, then AddProp()
//     (= SetKeyValue("model", name) on edit_entity), then bump version / clear
//     modelClass + the brush's modelFailed flag and refocus the XY view.
//
//  THE SPLIT (so the headless model_gate can exercise the real commit without a file
//  dialog — exactly as RunSetKeyTest drives SetKeyValue directly, not the GUI fields):
//   * Ed_RelativizeModelPath(full, subdir, stripToBareName) — the pure path math.
//   * Ed_CommitPickedModel(def, relName) — the picker's NET EFFECT on the entity
//     (SetKeyValue("model",name) + the version/modelClass/brush-flag tail).  This is
//     what the gate calls; the GUI picker calls it after the CFileDialog returns.
// ──────────────────────────────────────────────────────────────────────────────

// Ed_RelativizeModelPath — turn an absolute picked path into the stored "model"
// value, faithful to OpenDialog's lowercase + slashify + find-subdir-token + strip.
// `subdirToken` is the leaf of the search subdir ("xmodel" or "prefabs"); the binary
// searches for the LOWERCASE token, so callers pass it lowercase.  Returns the
// relative form (a copy in `out`, capped at outSz).  Mirrors the two cstr_find loops.
void Ed_RelativizeModelPath( const char *fullPath, const char *subdirToken,
                             bool stripToBareName, char *out, size_t outSz )
{
    char buf[1024];
    // lowercase + backslash→slash (the binary does both, on both the path and the
    // token, before cstr_find).
    size_t n = 0;
    for ( const char *p = fullPath; *p && n < sizeof( buf ) - 1; ++p, ++n )
        buf[n] = ( *p == '\\' ) ? '/' : (char)tolower( (unsigned char)*p );
    buf[n] = '\0';

    char tok[64];
    size_t tn = 0;
    for ( const char *p = subdirToken; *p && tn < sizeof( tok ) - 1; ++p, ++tn )
        tok[tn] = ( *p == '\\' ) ? '/' : (char)tolower( (unsigned char)*p );
    tok[tn] = '\0';

    // Relativise: drop everything up to (and including) the subdir token.  The binary
    // finds the token; if present (and not already at index 0) it Mid()s the string to
    // start at the token.  Here we point past the token + its trailing slash.
    const char *rel = buf;
    char *hit = ( tok[0] ? strstr( buf, tok ) : nullptr );
    if ( hit )
    {
        rel = hit + strlen( tok );
        if ( *rel == '/' ) ++rel;          // skip the slash after "xmodel"
        // stripToBareName (models): the value is the bare leaf the asset system wants
        // (e.g. "ac_car_part01"), so after stripping "xmodel/" we already have it —
        // but if the model lives in a deeper subdir the binary keeps the remainder.
        // (OpenDialog's a4 path additionally drops a leading dir segment if one
        // remains before the first '/'; we keep the post-"xmodel/" remainder, which is
        // the bare name for the flat raw\xmodel layout and the relative name otherwise.)
        (void)stripToBareName;
    }

    // Copy out.
    size_t i = 0;
    for ( ; rel[i] && i < outSz - 1; ++i )
        out[i] = rel[i];
    out[i] = '\0';
}

// Ed_CommitPickedModel — set `relName` as the entity's "model" key and run the
// picker's post-set tail (faithful to OpenDialog's AddProp + version/modelClass/
// brush-flag cleanup).  Undo-bracketed like AddProp.  Headless-safe (no HWNDs).
void Ed_CommitPickedModel( entity_s_def *def, const char *relName )
{
    if ( !def || !relName )
        return;

    Undo_ClearRedo();
    Undo_GeneralStart( "set key value pair" );      // AddProp's bracket
    Undo_AddEntity_W( (entity_s *)def );
    SetKeyValue( def, "model", relName );            // → Checkkey_Model + EntityAssignModel
    Undo_End();

    // OpenDialog's tail (0x498119..): SetKeyValue already ran Checkkey_Model's
    // version++/modelClass=NULL/brush.unk01=0 for the "model" key, so this is the
    // binary's belt-and-suspenders repeat — kept verbatim.
    ++def->version;
    def->modelClass = nullptr;
    brush_t *b = (brush_t *)def->def;   // brushes.oprev == the def-list brush
    if ( b != (brush_t *)&def->def )
        b->unk01 = 0;
}

// Ed_PostAddModelCommand — CreateEntityFromName's misc_model tail posts WM_COMMAND
// IDC_E_ADD_MODEL to the entity window (the binary uses the resource ID 1294; this
// hand-built window uses its own control ID IDC_ENT_ADD_MODEL_BTN).  HIWORD 0 =
// BN_CLICKED, so MFC routes it to CEntityWnd::OnAddModel.  No-op (headless / no
// entity window) — the model-less bbox stands and the gate drives the commit core.
void Ed_PostAddModelCommand()
{
    // wParam = MAKEWPARAM(id, BN_CLICKED); BN_CLICKED==0 so the HIWORD notification code
    // is 0 and wParam is just the control id.  (MAKEWPARAM/MAKELONG aren't reliably in
    // scope in this TU's macro set — build it explicitly.)
    if ( g_qeglobals.d_hwndEntity )
        PostMessageA( g_qeglobals.d_hwndEntity, WM_COMMAND,
                      (WPARAM)( (unsigned short)IDC_ENT_ADD_MODEL_BTN
                                | ( (unsigned long)( BN_CLICKED ) << 16 ) ), 0 );
}

// CEntityWnd_OpenModelDialog (0x497d20) — the GUI picker.  Only compiled into the
// normal (MFC) build; the selftest exercises Ed_CommitPickedModel directly.  `key`
// selects the project base ("basepath"/"mapspath"), `subdir` the start directory
// leaf, `filter` the CFileDialog filter, `stripToBareName` the model strip flag.
static void CEntityWnd_OpenModelDialog( const char *key, const char *subdir,
                                        const char *filter, bool stripToBareName )
{
    if ( !edit_entity )
        return;

    // Initial directory = <project `key` value>\<subdir>.  Missing key → empty (the
    // dialog opens in the CWD), exactly as OpenDialog's `zero` fallback.
    CString initialDir;
    if ( g_qeglobals.d_project_entity )
    {
        for ( epair_t *ep = g_qeglobals.d_project_entity->epairs; ep; ep = ep->next )
            if ( _stricmp( ep->key, key ) == 0 ) { initialDir = ep->value; break; }
    }
    if ( !initialDir.IsEmpty() && initialDir[initialDir.GetLength() - 1] != '\\' )
        initialDir += '\\';
    initialDir += subdir;

    // A plain CFileDialog (Open).  The CModelFileDialog preview pane is parked.
    CFileDialog dlg( TRUE, nullptr, nullptr,
                     OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, filter, g_pParentWnd );
    dlg.m_ofn.lpstrInitialDir = initialDir;
    if ( dlg.DoModal() != IDOK )
        return;

    // Relativise the pick to the stored model/prefab name.  The subdir token is the
    // search-dir leaf ("xmodel" / "prefabs"): take the last path component of `subdir`.
    char tokBuf[64];
    {
        const char *s = subdir, *leaf = subdir;
        for ( ; *s; ++s )
            if ( *s == '\\' || *s == '/' ) leaf = s + 1;
        size_t k = 0;
        for ( ; leaf[k] && leaf[k] != '\\' && leaf[k] != '/' && k < sizeof( tokBuf ) - 1; ++k )
            tokBuf[k] = leaf[k];
        tokBuf[k] = '\0';
    }

    char rel[1024];
    CStringA picked( dlg.GetPathName() );
    Ed_RelativizeModelPath( picked, tokBuf, stripToBareName, rel, sizeof( rel ) );

    // Reflect into the key/value fields (the binary does this, then AddProp re-reads
    // them) and commit.  We commit via Ed_CommitPickedModel (the net effect) so the
    // result is identical whether or not the fields exist.
    if ( entwnd_keyfield )   SendMessageA( entwnd_keyfield,   WM_SETTEXT, 0, (LPARAM)"model" );
    if ( entwnd_valuefield ) SendMessageA( entwnd_valuefield, WM_SETTEXT, 0, (LPARAM)rel );

    Ed_CommitPickedModel( edit_entity, rel );
    SetKeyValuePairs();                              // refresh the kv list

    if ( g_qeglobals.d_hwndXY )
        ::SetFocus( g_qeglobals.d_hwndXY );
}


// FieldWndProc (0x496370) — subclass for the key/value edit fields: Tab/Enter in the
// key field jumps to the value field; Enter in the value field commits via AddProp.
static LRESULT CALLBACK FieldWndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
    if ( Msg == WM_KEYDOWN )
    {
        if ( (WORD)wParam == VK_TAB )
        {
            if ( hWnd != entwnd_keyfield )
            {
                SetFocus( entwnd_keyfield );
                return CallWindowProcA( OldFieldWindowProc, hWnd, Msg, wParam, lParam );
            }
            SendMessageA( entwnd_valuefield, WM_SETTEXT, 0, (LPARAM)"" );
            SetFocus( entwnd_valuefield );
            return CallWindowProcA( OldFieldWindowProc, hWnd, Msg, wParam, lParam );
        }
        if ( (WORD)wParam == VK_RETURN )
        {
            if ( hWnd == entwnd_keyfield )
            {
                SendMessageA( entwnd_valuefield, WM_SETTEXT, 0, (LPARAM)"" );
                SetFocus( entwnd_valuefield );
                return CallWindowProcA( OldFieldWindowProc, hWnd, Msg, wParam, lParam );
            }
            AddProp();
            SetFocus( g_qeglobals.d_hwndCamera );
            return CallWindowProcA( OldFieldWindowProc, hWnd, Msg, wParam, lParam );
        }
        return CallWindowProcA( OldFieldWindowProc, hWnd, Msg, wParam, lParam );
    }
    if ( Msg == WM_CHAR )
    {
        if ( (WORD)wParam == VK_TAB || (WORD)wParam == VK_RETURN )
            return 0;                                  // eat the control-char beep
        if ( (WORD)wParam == VK_ESCAPE )
        {
            SetFocus( g_qeglobals.d_hwndCamera );
            return 0;
        }
        return CallWindowProcA( OldFieldWindowProc, hWnd, Msg, wParam, lParam );
    }
    if ( Msg == WM_LBUTTONDOWN )
        SetFocus( hWnd );
    return CallWindowProcA( OldFieldWindowProc, hWnd, Msg, wParam, lParam );
}

// ──────────────────────────────────────────────────────────────────────────────
//  CEntityWnd — the MFC control host (window plumbing only; logic above is faithful).
// ──────────────────────────────────────────────────────────────────────────────

static HFONT s_entFont = nullptr;

BEGIN_MESSAGE_MAP( CEntityWnd, CWnd )
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_CLOSE()
    ON_WM_DESTROY()
    ON_LBN_SELCHANGE( IDC_ENT_ECLASS_LIST, &CEntityWnd::OnEclassSelChange )
    ON_LBN_DBLCLK   ( IDC_ENT_ECLASS_LIST, &CEntityWnd::OnEclassDblClk    )
    ON_LBN_SELCHANGE( IDC_ENT_KV_LIST,     &CEntityWnd::OnKVSelChange     )
    ON_BN_CLICKED   ( IDC_ENT_DELETE_BTN,  &CEntityWnd::OnDeleteKey       )
    ON_BN_CLICKED   ( IDC_ENT_ADD_MODEL_BTN, &CEntityWnd::OnAddModel       )
    ON_CONTROL_RANGE( BN_CLICKED, IDC_ENT_CHECK_FIRST, IDC_ENT_CHECK_LAST, &CEntityWnd::OnSpawnFlagCheck )
    ON_CONTROL_RANGE( BN_CLICKED, IDC_ENT_DIR_FIRST,   IDC_ENT_DIR_LAST,   &CEntityWnd::OnAngleButton    )
    ON_CONTROL_RANGE( BN_CLICKED, IDC_FILT_FLAG_FIRST, IDC_FILT_FLAG_LAST, &CEntityWnd::OnFilterFlagCheck  )
    ON_WM_MEASUREITEM()                 // owner-draw the category filter lists (checkbox glyph rows)
    ON_WM_DRAWITEM()
END_MESSAGE_MAP()

CEntityWnd::CEntityWnd()
{
}

BOOL CEntityWnd::PreCreateWindow( CREATESTRUCT &cs )
{
    // A plain control-host window (dialog-face background, arrow cursor) — NOT a D3D pane.
    if ( !CWnd::PreCreateWindow( cs ) )
        return FALSE;
    cs.lpszClass = AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW,
                                        ::LoadCursor( NULL, IDC_ARROW ),
                                        (HBRUSH)( COLOR_BTNFACE + 1 ), NULL );
    return TRUE;
}

// The inspector is a FLOATING window summoned by the View hotkeys; its caption close box
// must HIDE it (so the next toggle re-shows it), not destroy it.  Matches the binary's
// CS_NOCLOSE QENT class (close disabled; toggled only).
void CEntityWnd::OnClose()
{
    ShowWindow( SW_HIDE );
}

// CEntityWnd WM_DESTROY (binary entity wndproc 0x4981F0 @ 0x498253) — persist the
// placement via SaveWindowPlacement("EntityWindowPlace") (helper 0x421C20 =
// GetWindowPlacement + SaveRegistryInfo, inlined here; the helper's only caller is
// this handler).  The load side already reads the same key (mainfrm.cpp:970).
//
extern BOOL SaveRegistryInfo( const char *pszName, void *pvBuf, int lSize );   // win_qe3.cpp
void CEntityWnd::OnDestroy()
{
    CWnd::OnDestroy();
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof( wndpl );                          // 0x421c2b (44)
    if ( GetWindowPlacement( &wndpl ) )                      // 0x421c32
        SaveRegistryInfo( "EntityWindowPlace", &wndpl, 0x2C );   // 0x421c46 (no "Radiant::" prefix — matches the loader)
}

static HWND ED_MakeChild( HWND parent, const char *cls, DWORD style, int id, const char *text = nullptr )
{
    HWND h = CreateWindowExA( 0, cls, text, WS_CHILD | WS_VISIBLE | style,
                              0, 0, 10, 10, parent, (HMENU)(INT_PTR)id,
                              AfxGetInstanceHandle(), NULL );
    if ( h && s_entFont )
        SendMessageA( h, WM_SETFONT, (WPARAM)s_entFont, 0 );
    return h;
}

static inline void MoveWindow_( HWND h, int x, int y, int w, int ht )
{
    if ( h )
        ::MoveWindow( h, x, y, w, ht, TRUE );
}

// ── single-click checkbox toggle for the owner-drawn category filter lists ──────────
extern void RadiantFilters_ToggleEntry( filter_entry_s *cb, bool show );   // filters.cpp
extern void CFilterWnd_SaveCheck( const char *name, bool shown );          // filters.cpp
static WNDPROC s_filtListPrevProc = nullptr;   // the original listbox WndProc (all 4 share it)
static LRESULT CALLBACK FiltList_WndProc( HWND h, UINT msg, WPARAM wp, LPARAM lp )
{
    if ( msg == WM_LBUTTONDOWN )
    {
        // WM_LBUTTONDOWN lParam already carries the client point (x=LOWORD, y=HIWORD) in exactly
        // the form LB_ITEMFROMPOINT expects, so forward it directly.
        DWORD r = (DWORD)::SendMessageA( h, LB_ITEMFROMPOINT, 0, lp );
        if ( HIWORD( r ) == 0 )                       // point is over a real item
        {
            int item = (short)LOWORD( r );
            filter_entry_s *f = (filter_entry_s *)::SendMessageA( h, LB_GETITEMDATA, item, 0 );
            if ( f )
            {
                bool ns = !f->isShown;
                RadiantFilters_ToggleEntry( f, ns );  // flip + FilterBrush invalidate + repaint
                CFilterWnd_SaveCheck( f->name, ns );  // persist (registry "Filters\<name>")
                RECT rc;
                if ( ::SendMessageA( h, LB_GETITEMRECT, item, (LPARAM)&rc ) != LB_ERR )
                    ::InvalidateRect( h, &rc, FALSE );
            }
        }
    }
    return ::CallWindowProcA( s_filtListPrevProc, h, msg, wp, lp );
}
static void FiltList_Subclass( HWND h )
{
    WNDPROC prev = (WNDPROC)(LONG_PTR)::SetWindowLongPtrA( h, GWLP_WNDPROC, (LONG_PTR)FiltList_WndProc );
    if ( !s_filtListPrevProc )
        s_filtListPrevProc = prev;
}

// Pick the multi-column width so every item fits with NO scrollbar: rows-per-column from the
// list's current height, columns from the item count, then split the width across them so the
// items flow left→right and fill the list.  Safe to call any time the list is sized+populated.
static void FiltList_SetColumns( HWND h )
{
    if ( !h )
        return;
    RECT lr;
    ::GetClientRect( h, &lr );
    int lw = lr.right - lr.left, lh = lr.bottom - lr.top;
    if ( lh < 22 )                                    // not laid out yet — OnSize will redo it
        return;
    int items      = (int)::SendMessageA( h, LB_GETCOUNT, 0, 0 );
    int rowsPerCol = lh / 18;  if ( rowsPerCol < 1 ) rowsPerCol = 1;
    int cols       = ( items + rowsPerCol - 1 ) / rowsPerCol;  if ( cols < 1 ) cols = 1;
    ::SendMessageA( h, LB_SETCOLUMNWIDTH, ( lw > 0 ? lw : 100 ) / cols, 0 );
}

int CEntityWnd::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;

    if ( !s_entFont )
    {
        s_entFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
    }

    HWND self = GetSafeHwnd();

    // No tab strip — mode switching is on the N/O/T/F hotkeys + the View menu
    // (CEntityWnd_SetInspectorMode).  The binary's inspector tab lives in a dialog
    // resource this port cannot subclass; the invented WC_TABCONTROL was the flagged UX.

    hwndEnt_entlist = ED_MakeChild( self, "listbox",
        LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_BORDER, IDC_ENT_ECLASS_LIST );
    entwnd_comment  = ED_MakeChild( self, "edit",
        ES_MULTILINE | ES_READONLY | WS_VSCROLL | WS_BORDER, IDC_ENT_COMMENT );
    entwnd_kvlist   = ED_MakeChild( self, "listbox",
        LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | LBS_USETABSTOPS | WS_VSCROLL | WS_BORDER, IDC_ENT_KV_LIST );
    entwnd_keylabel   = ED_MakeChild( self, "static", SS_LEFT, IDC_ENT_KEY_LABEL, "Key" );
    entwnd_keyfield   = ED_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL, IDC_ENT_KEY_FIELD );
    entwnd_valuelabel = ED_MakeChild( self, "static", SS_LEFT, IDC_ENT_VALUE_LABEL, "Value" );
    entwnd_valuefield = ED_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL, IDC_ENT_VALUE_FIELD );
    entwnd_delbtn     = ED_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_ENT_DELETE_BTN, "Delete Key" );
    entwnd_addmodelbtn= ED_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_ENT_ADD_MODEL_BTN, "Add Model" );

    // The 12 spawnflag checkboxes (BS_AUTOCHECKBOX = toggles its own state, so a click
    // both flips the box AND fires BN_CLICKED → SetSpawnFlags_R).  Static placeholder
    // labels for the last 4 (the difficulty/gametype flags the eclass never names).
    static const char *const kCheckDefault[12] =
        { " ", " ", " ", " ", " ", " ", " ", " ", "Easy", "Medium", "Hard", "Deathmatch" };
    for ( int i = 0; i < 12; ++i )
        entwnd_entcheck[i] = ED_MakeChild( self, "button", BS_AUTOCHECKBOX | WS_TABSTOP,
                                           IDC_ENT_CHECK_FIRST + i, kCheckDefault[i] );

    // The compass angle grid (8 directions) + up/down, in ENTITY_DEFINES order:
    // EntDir0/45/90/135/180/225/270/315 then Up/Down.  Each is a push button.
    static const char *const kDirLabel[10] =
        { "E", "NE", "N", "NW", "W", "SW", "S", "SE", "Up", "Dn" };
    for ( int i = 0; i < 10; ++i )
        entwnd_entdir[i] = ED_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP,
                                         IDC_ENT_DIR_FIRST + i, kDirLabel[i] );

    // Tab stops so the key/value listbox columns align (LBS_USETABSTOPS).
    int tabs[2] = { 64, 80 };
    if ( entwnd_kvlist )
        ::SendMessageA( entwnd_kvlist, LB_SETTABSTOPS, 2, (LPARAM)tabs );

    // Subclass the two edit fields (Enter→AddProp), exactly like the binary's FieldWndProc.
    if ( entwnd_keyfield )
    {
        OldFieldWindowProc = (WNDPROC)SetWindowLongPtr( entwnd_keyfield, GWLP_WNDPROC, (LONG_PTR)FieldWndProc );
        SetWindowLongPtr( entwnd_valuefield, GWLP_WNDPROC, (LONG_PTR)FieldWndProc );
    }

    // ── the Filters pane controls (CFilterWnd): 4 category checklists + 6 checkboxes ──
    static const char *const kCatLabel[4] = { "Geometry", "Trigger", "Entity", "Other" };
    for ( int c = 0; c < 4; ++c )
    {
        filt_catlabel[c] = ED_MakeChild( self, "static", SS_LEFT, 1900 + c, kCatLabel[c] );
        // LBS_MULTICOLUMN (matches the binary's IDD_DIALOG_FILTER style 0x351): items flow
        // top→bottom then wrap into a new column to the RIGHT.  No WS_VSCROLL — the layout sizes
        // the column width so every item fits without a scrollbar.
        filt_catlist[c]  = ED_MakeChild( self, "listbox",
            LBS_NOTIFY | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED |
            LBS_MULTICOLUMN | WS_BORDER, IDC_FILT_LIST_FIRST + c );
        if ( filt_catlist[c] )                        // subclass for single-click checkbox toggle
            FiltList_Subclass( filt_catlist[c] );
    }
    for ( int i = 0; i < 6; ++i )
        filt_flagcheck[i] = ED_MakeChild( self, "button", BS_AUTOCHECKBOX | WS_TABSTOP,
                                          IDC_FILT_FLAG_FIRST + i, CFilterWnd_ShowFlagLabel( i ) );

    FillClassList();
    RefreshFilterPane();        // populate the checklists from the loaded filter lists
    return 0;
}

// RefreshFilterPane (CFilterWnd::InitFilterCheckboxes 0x414210 + sub_414380): repopulate
// the four category checklists from the loaded filter lists and sync the six show-flag
// checkboxes from d_xyShowFlags (the GetSettings push).  Each list row is "[x] <name>"
// (checked = isShown) — clicking toggles it (OnFilterListClick → RadiantFilters_ToggleEntry).
void CEntityWnd::RefreshFilterPane()
{
    for ( int c = 0; c < 4; ++c )
    {
        if ( !filt_catlist[c] )
            continue;
        int prevTop = (int)::SendMessageA( filt_catlist[c], LB_GETTOPINDEX, 0, 0 );
        ::SendMessageA( filt_catlist[c], LB_RESETCONTENT, 0, 0 );
        for ( filter_entry_s *f = CFilterWnd_GetCategoryHead( c ); f; f = f->next_filter )
        {
            // Text is just the name; the check state is drawn from f->isShown in OnDrawItem.
            LRESULT idx = ::SendMessageA( filt_catlist[c], LB_ADDSTRING, 0,
                                          (LPARAM)( f->name ? f->name : "?" ) );
            ::SendMessageA( filt_catlist[c], LB_SETITEMDATA, idx, (LPARAM)f );
        }
        if ( prevTop > 0 )
            ::SendMessageA( filt_catlist[c], LB_SETTOPINDEX, prevTop, 0 );
        FiltList_SetColumns( filt_catlist[c] );        // re-fit columns to the new item count
    }
    // Sync the six simple show-flag checkboxes from the current d_xyShowFlags state.
    for ( int i = 0; i < 6; ++i )
        if ( filt_flagcheck[i] )
            ::SendMessageA( filt_flagcheck[i], BM_SETCHECK,
                            CFilterWnd_GetShowFlag( i ) ? BST_CHECKED : BST_UNCHECKED, 0 );
}

void CEntityWnd::OnSize( UINT nType, int cx, int cy )
{
    CWnd::OnSize( nType, cx, cy );
    if ( cx < 32 || cy < 32 )
        return;

    // No tab strip: every mode's controls lay out across the full client area from the
    // top (the binary column-swaps whole panes in/out of the same client rect — here we
    // show/hide per mode in LayoutForMode and lay each group over the full area).

    const int M = 4;          // margin
    const int row = 22;       // single-line control height
    int x = M, w = cx - 2 * M;

    // The spawnflags + angle band is a fixed-height block (6 checkbox rows tall) that
    // sits between the comment and the key/value list.
    const int chkRow  = 16;                    // checkbox row height
    const int flagsH  = chkRow * 6;            // 12 checkboxes, 2 cols × 6 rows
    const int dirBtn  = 22;                    // compass button size

    // Top→bottom: eclass list, comment, [spawnflags|angles] band, kv list, key/value/button.
    int botFixed = M + row * 3 + M;            // key + value + button rows + margin
    int avail    = cy - botFixed - flagsH - 5 * M;
    if ( avail < 40 ) avail = cy / 2;
    int eclassH  = avail * 50 / 100;
    int commentH = avail * 14 / 100;
    int kvH      = avail - eclassH - commentH;
    int y = M;                // controls start at the top of the client area

    MoveWindow_( hwndEnt_entlist, x, y, w, eclassH ); y += eclassH + M;
    MoveWindow_( entwnd_comment,  x, y, w, commentH ); y += commentH + M;

    // ── spawnflags (left, 2 cols × 6 rows) + angle grid (right, 3×3 + up/dn) ──
    int bandTop  = y;
    int dirGridW = dirBtn * 3 + 2 + dirBtn;    // 3-wide compass + an up/dn column
    int flagsW   = w - dirGridW - M;
    if ( flagsW < 80 ) flagsW = w - M;         // very narrow: let flags take the row
    int colW = flagsW / 2;
    for ( int i = 0; i < 12; ++i )
    {
        int col = i / 6, r = i % 6;
        MoveWindow_( entwnd_entcheck[i], x + col * colW, bandTop + r * chkRow, colW - 2, chkRow );
    }

    // Compass: indices map to a 3×3 cell grid by direction (N up, E right).
    //   NW(135,idx3) N(90,idx2) NE(45,idx1)
    //   W (180,idx4)     ·       E (0,idx0)
    //   SW(225,idx5) S(270,idx6) SE(315,idx7)
    int gx = M + flagsW + M;
    static const int cell[8] = { /*idx0 E*/ 5, /*idx1 NE*/ 2, /*idx2 N*/ 1, /*idx3 NW*/ 0,
                                 /*idx4 W*/ 3, /*idx5 SW*/ 6, /*idx6 S*/ 7, /*idx7 SE*/ 8 };
    for ( int i = 0; i < 8; ++i )
    {
        int c = cell[i] % 3, r = cell[i] / 3;
        MoveWindow_( entwnd_entdir[i], gx + c * dirBtn, bandTop + r * dirBtn, dirBtn - 1, dirBtn - 1 );
    }
    // Up (idx8) / Down (idx9) stacked in the 4th column.
    int upx = gx + 3 * dirBtn + 2;
    MoveWindow_( entwnd_entdir[8], upx, bandTop,             dirBtn - 1, dirBtn - 1 );
    MoveWindow_( entwnd_entdir[9], upx, bandTop + dirBtn * 2, dirBtn - 1, dirBtn - 1 );

    y = bandTop + flagsH + M;

    MoveWindow_( entwnd_kvlist,   x, y, w, kvH ); y += kvH + M;

    int lblW = 44;
    MoveWindow_( entwnd_keylabel, x, y + 3, lblW, row );
    MoveWindow_( entwnd_keyfield, x + lblW, y, w - lblW, row );
    y += row + M;
    MoveWindow_( entwnd_valuelabel, x, y + 3, lblW, row );
    MoveWindow_( entwnd_valuefield, x + lblW, y, w - lblW, row );
    y += row + M;
    // Bottom button row: Delete Key | Add Model, side by side.
    int halfW = ( w - M ) / 2;
    MoveWindow_( entwnd_delbtn,     x,             y, halfW, row );
    MoveWindow_( entwnd_addmodelbtn, x + halfW + M, y, w - halfW - M, row );

    // CEntityWnd_SizeEntityDlg 0x4978A0: overwrite the earlier port layout with the
    // binary's fixed-pixel entity-pane geometry.  The binary column-swaps inactive
    // panes off to x=iWidth; LayoutForMode still handles visibility in this port.
    {
        const int entityCol = ( inspector_mode != INSPECTOR_ENTITY ) ? cx : 0;
        const int iHeighta = cy - 24;
        const int v6 = ( iHeighta / 2 - 10 ) / 3;

        MoveWindow_( hwndEnt_entlist, entityCol + 5, 5, cx - 10, 2 * v6 );
        MoveWindow_( entwnd_comment, entityCol + 5, 2 * v6 + 10, cx - 10, v6 - 10 );

        int v7 = ( cx - 10 ) / 3;
        int checkIndex = 0;
        for ( int col = 0, x0 = 5; col < 3; ++col, x0 += v7 )
        {
            int y0 = iHeighta / 2;
            for ( int row0 = 0; row0 < 4; ++row0, y0 += 18 )
                MoveWindow_( entwnd_entcheck[checkIndex++], entityCol + x0, y0, v7, 18 );
        }

        MoveWindow_( entwnd_kvlist, entityCol + 5, iHeighta / 2 + 72,
                     cx - 10, iHeighta - ( iHeighta / 2 + 72 ) - 100 );

        int keyY = iHeighta - 100 + 5;
        MoveWindow_( entwnd_keylabel, entityCol + 5, keyY, 40, 18 );
        MoveWindow_( entwnd_keyfield, entityCol + 45, keyY, cx - 50, 18 );

        int valueY = iHeighta - 100 + 23;
        MoveWindow_( entwnd_valuelabel, entityCol + 5, valueY, 40, 18 );
        MoveWindow_( entwnd_valuefield, entityCol + 45, valueY, cx - 50, 18 );

        int dirY = iHeighta - 100 + 43;
        MoveWindow_( entwnd_entdir[3], entityCol + 5,   dirY,                  36, 18 );
        MoveWindow_( entwnd_entdir[4], entityCol + 5,   iHeighta - 100 + 61,   36, 18 );
        MoveWindow_( entwnd_entdir[5], entityCol + 5,   iHeighta - 100 + 79,   36, 18 );
        MoveWindow_( entwnd_entdir[2], entityCol + 41,  dirY,                  36, 18 );
        MoveWindow_( entwnd_entdir[6], entityCol + 41,  iHeighta - 100 + 79,   36, 18 );
        MoveWindow_( entwnd_entdir[1], entityCol + 77,  dirY,                  36, 18 );
        MoveWindow_( entwnd_entdir[0], entityCol + 77,  iHeighta - 100 + 61,   36, 18 );
        MoveWindow_( entwnd_entdir[7], entityCol + 77,  iHeighta - 100 + 79,   36, 18 );
        MoveWindow_( entwnd_entdir[8], entityCol + 131, iHeighta - 100 + 52,   36, 18 );
        MoveWindow_( entwnd_entdir[9], entityCol + 131, iHeighta - 100 + 70,   36, 18 );
        MoveWindow_( entwnd_delbtn, entityCol + 185, dirY,                     72, 18 );
        MoveWindow_( entwnd_addmodelbtn, entityCol + 185, iHeighta - 100 + 65, 72, 18 );
    }

    // ── Filters pane layout (occupies the FULL client area; shown only in Filter mode) ──
    // Faithful to the binary's IDD_DIALOG_FILTER (0xB4): the four category CHECKLISTS stack
    // vertically and fill the TOP; the six show-flag checkboxes sit at the BOTTOM in a
    // 3-col × 2-row grid (Names/Coordinates/Blocks over Connections/Angles/Reverse Filter).
    {
        int fx        = M;
        int fw        = cx - 2 * M;
        const int chkH    = 18;                        // show-flag checkbox row height
        const int catLblH = 16;                        // category heading height

        // Bottom band: 6 show-flag checkboxes, 3 columns × 2 rows, anchored to the bottom.
        int col3    = fw / 3;
        int bandH   = chkH * 2;
        int bandTop = cy - bandH - M;                  // just above the bottom margin
        // port s_showFlagBoxes index → (col,row) matching the binary's IDD_DIALOG_FILTER slots:
        //   row0: Names(2) Coordinates(4) Blocks(3)   row1: Connections(1) Angles(0) ReverseFilter(5)
        static const struct { int idx, col, row; } kFlagPos[6] = {
            { 2, 0, 0 }, { 4, 1, 0 }, { 3, 2, 0 },
            { 1, 0, 1 }, { 0, 1, 1 }, { 5, 2, 1 },
        };
        for ( const auto &p : kFlagPos )
            if ( filt_flagcheck[p.idx] )
                MoveWindow_( filt_flagcheck[p.idx], fx + p.col * col3, bandTop + p.row * chkH,
                             col3, chkH );

        // The four category sections (heading label + checklist) fill everything ABOVE the band.
        int fy        = M;
        int catsAvail = bandTop - M - fy;
        if ( catsAvail < 80 ) catsAvail = cy / 2;
        int perCat = catsAvail / 4;
        for ( int c = 0; c < 4; ++c )
        {
            MoveWindow_( filt_catlabel[c], fx, fy, fw, catLblH );
            int listH = perCat - catLblH - M;
            if ( listH < 24 ) listH = 24;
            MoveWindow_( filt_catlist[c], fx, fy + catLblH, fw, listH );
            FiltList_SetColumns( filt_catlist[c] );    // multi-column width (no scrollbar)
            fy += perCat;
        }
    }

    // Apply the active-mode show/hide (each mode shows ONLY its own controls across the
    // full client area — see CEntityWnd_SetInspectorMode / LayoutForMode).
    LayoutForMode();
}

// LayoutForMode (the show/hide half of SizeEntityDlg 0x4978a0): the entity control group
// is visible only when inspector_mode == INSPECTOR_ENTITY, and the Filters controls only
// in INSPECTOR_FILTER — mirroring the binary's column-swap (only the active mode's pane
// occupies the client area).  The texture browser / console panes are surfaced by
// CEntityWnd_SetInspectorMode in their own slots.  No tab strip.
void CEntityWnd::LayoutForMode()
{
    if ( !GetSafeHwnd() )
        return;
    int show     = ( inspector_mode == INSPECTOR_ENTITY ) ? SW_SHOW : SW_HIDE;
    int showFilt = ( inspector_mode == INSPECTOR_FILTER ) ? SW_SHOW : SW_HIDE;
    HWND ents[] =
    {
        hwndEnt_entlist, entwnd_comment, entwnd_kvlist, entwnd_keylabel, entwnd_keyfield,
        entwnd_valuelabel, entwnd_valuefield, entwnd_delbtn, entwnd_addmodelbtn,
    };
    for ( HWND h : ents )
        if ( h ) ::ShowWindow( h, show );
    for ( int i = 0; i < 12; ++i )
        if ( entwnd_entcheck[i] ) ::ShowWindow( entwnd_entcheck[i], show );
    for ( int i = 0; i < 10; ++i )
        if ( entwnd_entdir[i] ) ::ShowWindow( entwnd_entdir[i], show );

    // The Filters pane (CFilterWnd controls) is visible only in INSPECTOR_FILTER mode.
    for ( int c = 0; c < 4; ++c )
    {
        if ( filt_catlabel[c] ) ::ShowWindow( filt_catlabel[c], showFilt );
        if ( filt_catlist[c]  ) ::ShowWindow( filt_catlist[c],  showFilt );
    }
    for ( int i = 0; i < 6; ++i )
        if ( filt_flagcheck[i] ) ::ShowWindow( filt_flagcheck[i], showFilt );

    // Entering Filter mode: re-sync the checklists + flag checkboxes (a filter may have
    // been toggled via a View→Show menu command, or the map reloaded, while we were away).
    if ( inspector_mode == INSPECTOR_FILTER )
        RefreshFilterPane();
}

// ── CFilterWnd handlers (the Filters inspector pane) ──────────────────────────
// Double-click a category checklist row → flip that filter entry's isShown via
// RadiantFilters_ToggleEntry (= RadiantFilters07's core: ++g_filtersUpdated + repaint),
// then redraw the "[x]/[ ]" annotation.  Persist the new check state to the registry.
// Owner-draw the category filter lists: fixed row height (checkbox glyph + text).
void CEntityWnd::OnMeasureItem( int nIDCtl, LPMEASUREITEMSTRUCT mis )
{
    if ( nIDCtl >= IDC_FILT_LIST_FIRST && nIDCtl <= IDC_FILT_LIST_LAST )
    {
        mis->itemHeight = 18;
        return;
    }
    CWnd::OnMeasureItem( nIDCtl, mis );
}

// Draw one category-filter row: a real checkbox glyph (checked = the entry is shown) + the name.
void CEntityWnd::OnDrawItem( int nIDCtl, LPDRAWITEMSTRUCT dis )
{
    if ( nIDCtl < IDC_FILT_LIST_FIRST || nIDCtl > IDC_FILT_LIST_LAST || (int)dis->itemID < 0 )
    {
        CWnd::OnDrawItem( nIDCtl, dis );
        return;
    }
    HDC  dc = dis->hDC;
    RECT rc = dis->rcItem;
    filter_entry_s *f = (filter_entry_s *)dis->itemData;

    ::FillRect( dc, &rc, (HBRUSH)( COLOR_WINDOW + 1 ) );

    RECT cb = { rc.left + 3, rc.top + 2, rc.left + 3 + 13, rc.top + 2 + 13 };
    UINT st = DFCS_BUTTONCHECK | DFCS_FLAT;
    if ( f && f->isShown ) st |= DFCS_CHECKED;
    ::DrawFrameControl( dc, &cb, DFC_BUTTON, st );

    RECT tr = { cb.right + 5, rc.top, rc.right, rc.bottom };
    char text[256] = "";
    ::SendMessageA( dis->hwndItem, LB_GETTEXT, dis->itemID, (LPARAM)text );
    ::SetBkMode( dc, TRANSPARENT );
    ::SetTextColor( dc, ::GetSysColor( COLOR_WINDOWTEXT ) );
    ::DrawTextA( dc, text, -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX );
}

// A simple show-flag checkbox (Angles/Connections/Names/Blocks/Coordinates/Reverse Filter)
// toggled → flip its d_xyShowFlags bit (CFilterWnd::GetSettings polarity) + repaint.  This
// is the same effect as the matching View→Show menu command; we re-sync that command's menu
// check via SetButtonMenuStates implicitly (g_nUpdateBits drives the menu state refresh).
void CEntityWnd::OnFilterFlagCheck( UINT nID )
{
    int i = (int)( nID - IDC_FILT_FLAG_FIRST );
    if ( i < 0 || i >= 6 || !filt_flagcheck[i] )
        return;
    bool checked = ::SendMessageA( filt_flagcheck[i], BM_GETCHECK, 0, 0 ) == BST_CHECKED;
    CFilterWnd_SetShowFlag( i, checked );           // flip the bit + invalidate + repaint
}

// Global refresh entry — re-populate the Filters pane from the loaded filter lists +
// d_xyShowFlags.  Called on map load (so the entity checklists reflect the map's classes)
// and whenever the inspector enters Filter mode.  No-op if the entity window isn't up.
void Radiant_RefreshFilterPane()
{
    if ( g_pParentWnd && g_pParentWnd->m_pEntWnd )
        g_pParentWnd->m_pEntWnd->RefreshFilterPane();
}

void CEntityWnd::RefreshClassList()
{
    FillClassList();
}

void CEntityWnd::OnEclassSelChange()
{
    if ( !hwndEnt_entlist )
        return;
    LRESULT idx = ::SendMessageA( hwndEnt_entlist, LB_GETCURSEL, 0, 0 );
    if ( idx < 0 )
        return;
    eclass_t *pec = (eclass_t *)::SendMessageA( hwndEnt_entlist, LB_GETITEMDATA, idx, 0 );
    if ( pec && pec->name )
    {
        strncpy( s_selectedEclass, pec->name, sizeof( s_selectedEclass ) - 1 );
        s_selectedEclass[sizeof( s_selectedEclass ) - 1] = '\0';
    }
    UpdateSelection( (int)idx, pec );        // description + this entity's key/values
}

void CEntityWnd::OnEclassDblClk()
{
    CreateEntity();                          // double-click an eclass → create it
    ::SetFocus( g_qeglobals.d_hwndCamera );
}

void CEntityWnd::OnKVSelChange()
{
    EditProp();                              // load the picked pair into the edit fields
}

void CEntityWnd::OnDeleteKey()
{
    DelProp();
    ::SetFocus( g_qeglobals.d_hwndCamera );
}

// The "Add Model" button (the entity-window IDC_E_ADD_MODEL command).  Faithful to
// the WndProc case (0x498620): prefab eclasses (classtype & CLASS_PREFAB=0x10) pick a
// .map under <mapspath>\prefabs; everything else picks an xmodel under
// <basepath>\main_shared\xmodel (strip to the bare model name).  No-op without a
// selected entity (the binary's edit_entity deref would AV — we guard).
void CEntityWnd::OnAddModel()
{
    if ( !edit_entity || !edit_entity->eclass )
        return;
    if ( edit_entity->eclass->classtype & 0x10 )            // CLASS_PREFAB
        CEntityWnd_OpenModelDialog( "mapspath", "prefabs\\",
                                    "Map files (*.map)|*.map||", false );
    else
        CEntityWnd_OpenModelDialog( "basepath", "main_shared\\xmodel\\",
                                    "Model files (*)|*||", true );
}

// A spawnflag checkbox was clicked (BS_AUTOCHECKBOX has already toggled its state).
// Faithful to the WndProc IDC_E_AUTOCHECKBOX*/EASY/MEDIUM/HARD/DEATHMATCH cases, which
// each call SetSpawnFlags_R(bit) then refocus the camera.
void CEntityWnd::OnSpawnFlagCheck( UINT nID )
{
    SetSpawnFlags_R( (int)( nID - IDC_ENT_CHECK_FIRST ) );
    ::SetFocus( g_qeglobals.d_hwndCamera );
}

// An angle/direction button was clicked.  Faithful to the WndProc IDC_ANGLE* cases:
// the 8 compass buttons set the YAW component (axis 1) to a fixed heading; Up/Down
// toggle the PITCH component (axis 0) to -90 / +90 (CoD pitch convention: negative
// pitch looks up, positive looks down — NOT the GtkRadiant -1/-2 `angle` special
// values, which this CoD build does not use).  All cases refresh the kv list.
void CEntityWnd::OnAngleButton( UINT nID )
{
    int idx = (int)( nID - IDC_ENT_DIR_FIRST );
    switch ( idx )
    {
        case 0: Entity_SetAngles(   0.0f, 1 ); break;   // E   (IDC_ANGLE360)
        case 1: Entity_SetAngles(  45.0f, 1 ); break;   // NE
        case 2: Entity_SetAngles(  90.0f, 1 ); break;   // N
        case 3: Entity_SetAngles( 135.0f, 1 ); break;   // NW
        case 4: Entity_SetAngles( 180.0f, 1 ); break;   // W
        case 5: Entity_SetAngles( 225.0f, 1 ); break;   // SW
        case 6: Entity_SetAngles( 270.0f, 1 ); break;   // S
        case 7: Entity_SetAngles( 315.0f, 1 ); break;   // SE
        case 8:                                         // Up — toggle pitch -90/0
        {
            // sub_485570 zeroes the vec on a missing key (Entity_GetVec3ForKey does
            // NOT), so pre-zero — else ang[0] is stack garbage for an angle-less entity.
            float ang[3] = { 0.0f, 0.0f, 0.0f };
            Entity_GetVec3ForKey( edit_entity, ang, "angles" );
            Entity_SetAngles( ( ang[0] == -90.0f ) ? 0.0f : -90.0f, 0 );
            break;
        }
        case 9:                                         // Down — toggle pitch +90/0
        {
            float ang[3] = { 0.0f, 0.0f, 0.0f };
            Entity_GetVec3ForKey( edit_entity, ang, "angles" );
            Entity_SetAngles( ( ang[0] == 90.0f ) ? 0.0f : 90.0f, 0 );
            break;
        }
        default:
            return;
    }
    ::SetFocus( g_qeglobals.d_hwndCamera );
    SetKeyValuePairs();                                 // reflect the new "angles" key
}

// ── Accelerator forwarding for the floating inspector popup ─────────────────────
// The inspector (d_hwndEntity) is a top-level WS_POPUP owned by the main frame, so when it
// takes focus (opened via N/O/T/F) its keystrokes don't automatically reach the frame's
// accelerator table.  Without this, the FIRST F opens the Filters pane (focusing the popup)
// but a SECOND F goes to the popup and never reaches CMainFrame::OnFilterDlg — so the window
// couldn't be toggled closed with its own hotkey.  Forward key-downs through the frame's
// accelerator (which fires OnFilterDlg/OnViewEntity/etc. → the binary-faithful hide-on-repeat),
// but ONLY when a text Edit control does NOT have focus, so typing a key/value isn't eaten by
// a plain-letter hotkey.
BOOL CEntityWnd::PreTranslateMessage( MSG *pMsg )
{
    if ( pMsg->message == WM_KEYDOWN && g_pParentWnd )
    {
        HWND f = ::GetFocus();
        char cls[16] = "";
        if ( f )
            ::GetClassNameA( f, cls, sizeof( cls ) );
        if ( _stricmp( cls, "Edit" ) != 0 )                 // not typing in an edit box
        {
            // Editor hotkeys live in g_radiantCommands (NOT the m_hAccel accelerator table),
            // so route through the frame's command-map dispatch — this fires OnFilterDlg etc.,
            // whose binary-faithful toggle hides the popup when it's already open in that mode.
            // Only consumed when a command actually matched (so arrow/space still reach the
            // filter checklist).
            if ( g_pParentWnd->TryHotkey( (UINT)pMsg->wParam ) )
                return TRUE;
        }
    }
    return CWnd::PreTranslateMessage( pMsg );
}

// CEntityWnd_SetInspectorMode (0x496b00) — switch the right-column inspector between
// Entity / Textures / Console / Filters.  FAITHFUL state machine (matching the binary):
//   * the m_nCurrentStyle gate: in styles 0/3 (floating/4-pane) switching to Textures or
//     Console is a no-op (those are separate windows there).  This port is style 1
//     (integrated) so all switches apply.
//   * sets inspector_mode (0x240a110), the d_hwndEntity window title (Entity/Textures/
//     Console/Filters), and enables ID_MISC_SELECTENTITYCOLOR only in Entity mode
//     (EnableMenuItem MF_GRAYED otherwise — 3u = MF_DISABLED|MF_GRAYED).
//   * "show one pane-group at a time": LayoutForMode shows only the active mode's controls
//     (entity group in Entity mode, filter group in Filter mode) across the full client
//     area, mirroring the binary's column-swap.  The switch is driven by the N/O/T/F
//     hotkeys + the View menu (no tab control — the binary's tab lived in a dialog resource
//     this port cannot subclass; the invented WC_TABCONTROL was removed).
void CEntityWnd_SetInspectorMode( int mode )
{
    CMainFrame *frame = g_pParentWnd;

    // The style gate (binary 0x496b2d): styles 0/3 reject Texture/Console here.
    int style = frame ? frame->m_nCurrentStyle : 1;
    if ( ( style == 0 || style == 3 ) &&
         ( mode == INSPECTOR_TEXTURE || mode == INSPECTOR_CONSOLE ) )
        return;

    inspector_mode = mode;

    const char *title = "Entity";
    switch ( mode )
    {
    case INSPECTOR_TEXTURE: title = "Textures"; break;
    case INSPECTOR_CONSOLE: title = "Console";  break;
    case INSPECTOR_FILTER:  title = "Filters";  break;
    default:                title = "Entity";   break;
    }
    if ( g_qeglobals.d_hwndEntity )
        SetWindowTextA( g_qeglobals.d_hwndEntity, title );

    // Enable the "Select Entity Color" menu item only in Entity mode (binary: MF_ENABLED
    // for W_ENTITY, MF_GRAYED otherwise).  ID_MISC_SELECTENTITYCOLOR = 0x810C = 33036.
    if ( g_qeglobals.d_hwndMain )
    {
        HMENU menu = GetMenu( g_qeglobals.d_hwndMain );
        if ( menu )
            EnableMenuItem( menu, 33036,
                            ( mode == INSPECTOR_ENTITY ) ? MF_ENABLED
                                                         : ( MF_DISABLED | MF_GRAYED ) );
    }

    // Apply the show/hide: only the active mode's controls are visible (entity group in
    // Entity mode, filter group in Filter mode); the inspector body repaints.  This port
    // keeps the texture browser + console in their own always-visible fixed slots (the
    // working layout), so a non-Entity/non-Filter mode simply empties the inspector body +
    // flips the title/mode; their panes remain accessible in their slots.  NOTE: deliberately
    // NOT raising/focusing the D3D render panes here — BringWindowToTop/SetFocus on a live
    // swap-chain window destabilises the renderer (it forced a full-window texture render),
    // and we keep the proven layout stable over a faithful-but-risky reposition.
    if ( frame && frame->m_pEntWnd )
    {
        frame->m_pEntWnd->LayoutForMode();   // show/hide per mode
        ::RedrawWindow( g_qeglobals.d_hwndEntity, NULL, NULL,
                        RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASENOW );
    }
}

// KISAK divergences in this file, all deliberate:
//   0x4978a0 CEntityWnd_SizeEntityDlg — the binary column-swaps whole panes (Entity / Texture /
//            Console / Filter = d_hwndGroup = CFilterWnd) in and out of the client area via
//            entwnd_col.  This port lays the controls out proportionally and show/hides only
//            the active mode's group over the full client area, keeping the texture browser and
//            console in their own fixed slots.
//   NO TAB CONTROL: the binary's mode tab is a CTabCtrl (g_wndTabsEntWnd) subclassed from
//            IDC_E_TAB_CONTROL in the IDD_ENTITY resource (GetEntityControls 0x4964b0), which a
//            resource-less port cannot reproduce.
//   CEntityWnd_SetInspectorMode does NOT raise/focus the D3D render panes — BringWindowToTop /
//            SetFocus on a live swap-chain window destabilises the renderer.
//   W_GROUP (0x800) has no body in the binary's SetInspectorMode either (the "Group" pane is
//            dead in this build; OnViewGroups 0x42b700 has no menu command).
//   The CModelFileDialog model-PREVIEW pane (custom template, tmpl id 2) is not reproduced;
//            the picker is a plain CFileDialog.
