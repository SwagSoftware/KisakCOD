#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Vehicle and vehicle-path-node script-key authoring dialog. It applies to every
// selected non-world entity. The supported keys are:
//   EDIT fields (free value):
//     "script_startinghealth"  starting HP                          (edit 0x460140)
//     "script_accuracy"        0..100 → stored as a 0.00..1.00 float (edit 0x460250,
//                              value = atol(text)/100.0 formatted "%.2f"; default 44)
//     "speed"                  node drive speed                     (edit 0x460790)
//     "lookahead"              spline lookahead distance            (edit 0x4608A0)
//   ON/OFF radios (fixed value, a button per state):
//     "script_deathroll"   1/0   "script_team"  allies/axis
//     "script_turretmg"    1/0   "script_turret" 1/0
//     "script_badplace"    1/0   "script_avoidvehicles" 1/0
//     "script_attackai"    1/0   "script_crashtype" default/plane/forced (combo 0x460700)
//   SCRIPT-GROUP buttons (the PARKED colour/number machinery — see below):
//     "script_vehiclespawngroup" "script_vehiclestartmove" "script_vehiclegroupdelete"
//     "script_vehicleride" "script_vehiclewalk" "script_vehicleattackgroup"
//     "script_vehiclefocusfiregroup" "script_vehicledetour" "script_gatetrigger"
//     "script_attackorgs"
//
// Like the binary dialog, it is write-only and empty edits remove their keys.

#include "stdafx.h"
#include "qe3.h"
#include "mainfrm.h"
#include <cstring>

extern void Assert( const char *file, int line, int type, const char *fmt, ... );

// ── externs (per-TU, matching dynentitydlg.cpp / select.cpp) ──────────────────
extern entity_s *world_entity;                                              // 0x25D5B30 (map.cpp)
extern void      SetKeyValue( entity_s_def *e, const char *key, const char *value ); // 0x483690 (entity.cpp)
extern void      DeleteKey( epair_t **head, const char *key );              // 0x483720 (entity.cpp)
extern void      Checkkey_Model( entity_s_def *e, const char *key );        // 0x482F70 (=Checkkey_Model_0)
extern void      Checkkey_Color( entity_s_def *e, const char *key );        // 0x483210 (entity.cpp)
extern void      SetKeyValuePairs();                                        // 0x496CF0 (win_ent.cpp)
extern int       g_nUpdateBits;                                            // 0x25D5A74 (engine_stubs.cpp)
extern void      VehicleDlg_SetScriptGroupKey( const char *key );          // 0x45FBA0 (scriptgroup.cpp)
// selected_brushes sentinel is declared in qe3.h.

// Is this selected-brush instance one we should edit?  Faithful to the SetPair/RemovePair
// loop guards: skip a null owner, skip world_entity.  (No eclass-name filter — the vehicle
// dialog applies to any selected vehicle entity / vehicle path node.)  When it returns
// true, *outDef is the entity DEF that carries the epairs.
static bool Veh_SelectedDef( selbrush_t *i, entity_s_def **outDef )
{
    *outDef = nullptr;
    entity_s *owner = i->owner;
    if ( !owner || owner == world_entity )
        return false;

    entity_s_def *def = (entity_s_def *)owner->def;
    if ( !def )
        return false;

    // The binary's `b->owner->def == b->def->owner` invariant (type 0 → log+continue).
    if ( i->def && def != (entity_s_def *)i->def->owner )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\VehicleDlg.cpp",
                133, 0, "%s", "b->owner->def == b->def->owner" );

    *outDef = def;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45FC00  VehicleDlg_SetPair(value@<ebx>, key@<edi>) — SET `key`=`value` on every
// selected (non-world) entity's def.  The binary iterates selected_brushes.next and
// for each owner != world: assert the def invariant, then SetKeyValue(def, key, value);
// finally EndDialog(GetActiveWindow(),1) + SetFocus(camera) + g_nUpdateBits=-1.
// (No Undo bracket — transcribed verbatim.)
// ─────────────────────────────────────────────────────────────────────────────
void VehicleDlg_SetPair( const char *value, const char *key )
{
    for ( selbrush_t *i = selected_brushes.next; i != &selected_brushes; i = i->next )
    {
        entity_s_def *def = nullptr;
        if ( !Veh_SelectedDef( i, &def ) )
            continue;
        SetKeyValue( def, key, value );
    }
    // The dialog refreshes the entity inspector + invalidates the views.  (The binary's
    // EndDialog/SetFocus tail is the modeless-dialog "field applied, return focus to the
    // camera" gesture; the headless gate has no HWNDs so this is just the data refresh.)
    SetKeyValuePairs();
    g_nUpdateBits = -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45FC90  VehicleDlg_RemovePair(key@<eax>) — REMOVE `key` from every selected
// (non-world) entity's def.  DeleteKey(&def->epairs, key) then the post-delete
// revalidation Checkkey_Model_0 + Checkkey_Color (SetKeyValue fires those on a SET, but
// DeleteKey does not, so RemovePair calls them explicitly — verified at 0x45FCE4..0x45FCF4).
// ─────────────────────────────────────────────────────────────────────────────
void VehicleDlg_RemovePair( const char *key )
{
    for ( selbrush_t *i = selected_brushes.next; i != &selected_brushes; i = i->next )
    {
        entity_s_def *def = nullptr;
        if ( !Veh_SelectedDef( i, &def ) )
            continue;
        DeleteKey( &def->epairs, key );      // entity_s::epairs @0x74
        Checkkey_Model( def, key );          // 0x482F70 (Checkkey_Model_0)
        Checkkey_Color( def, key );          // 0x483210
    }
    SetKeyValuePairs();
    g_nUpdateBits = -1;
}

// ══════════════════════════════════════════════════════════════════════════════
//  CVehicleDlg — the hand-built modeless popup (CDynEntityDlg / CLayerDlg pattern)
// ══════════════════════════════════════════════════════════════════════════════
//
// EDIT rows (label + edit + [Set] [Clear]) for the four free-value keys, plus a row of
// on/off toggle pushbuttons for the boolean keys, plus a crash-type combo.  Set pushes
// the field/state onto the selected vehicle entities (SetPair); Clear removes the key
// (RemovePair).  Faithful to the binary's controls; the SCRIPT-GROUP buttons (the parked
// colour/number machinery) are deliberately omitted.

enum
{
    IDC_VEH_HEALTH = 4200,    // script_startinghealth edit
    IDC_VEH_ACCUR,            // script_accuracy edit (0..100)
    IDC_VEH_SPEED,            // speed edit
    IDC_VEH_LOOK,             // lookahead edit

    IDC_VEH_SET_HEALTH,       // per-field Set buttons
    IDC_VEH_SET_ACCUR,
    IDC_VEH_SET_SPEED,
    IDC_VEH_SET_LOOK,

    IDC_VEH_CLR_HEALTH,       // per-field Clear (remove key) buttons
    IDC_VEH_CLR_ACCUR,
    IDC_VEH_CLR_SPEED,
    IDC_VEH_CLR_LOOK,

    IDC_VEH_CRASH,            // crash-type combo (default/plane/forced)
    IDC_VEH_SET_CRASH,
    IDC_VEH_CLR_CRASH,

    // on/off toggle pushbuttons (each key gets an On and an Off button)
    IDC_VEH_DEATHROLL_ON, IDC_VEH_DEATHROLL_OFF,
    IDC_VEH_TURRET_ON,    IDC_VEH_TURRET_OFF,
    IDC_VEH_TURRETMG_ON,  IDC_VEH_TURRETMG_OFF,
    IDC_VEH_BADPLACE_ON,  IDC_VEH_BADPLACE_OFF,
    IDC_VEH_AVOID_ON,     IDC_VEH_AVOID_OFF,
    IDC_VEH_ATTACKAI_ON,  IDC_VEH_ATTACKAI_OFF,
    IDC_VEH_TEAM_ALLIES,  IDC_VEH_TEAM_AXIS,

    // script-group buttons (each stores its key into g_PrefsDlg->ScriptGroupKey then
    // ScriptGroup_AssignNextNumber).  Contiguous range (our own ids; the binary's are
    // the non-contiguous ctrl ids 1540/1542/1543/1545/1546/1552/1553/1554/1555/1605).
    IDC_VEH_SG_SPAWNGROUP,    // script_vehiclespawngroup
    IDC_VEH_SG_STARTMOVE,     // script_vehiclestartmove
    IDC_VEH_SG_GROUPDELETE,   // script_vehiclegroupdelete
    IDC_VEH_SG_RIDE,          // script_vehicleride
    IDC_VEH_SG_WALK,          // script_vehiclewalk
    IDC_VEH_SG_ATTACKGROUP,   // script_vehicleattackgroup
    IDC_VEH_SG_FOCUSFIRE,     // script_vehiclefocusfiregroup
    IDC_VEH_SG_DETOUR,        // script_vehicledetour
    IDC_VEH_SG_GATETRIGGER,   // script_gatetrigger
    IDC_VEH_SG_ATTACKORGS,    // script_attackorgs
};

CVehicleDlg *g_dlgVehicle = nullptr;        // the singleton (NULL until first opened)

static HFONT s_vehFont   = nullptr;
static HWND  s_vehHealth = nullptr;
static HWND  s_vehAccur  = nullptr;
static HWND  s_vehSpeed  = nullptr;
static HWND  s_vehLook   = nullptr;
static HWND  s_vehCrash  = nullptr;

static HWND VEH_MakeChild( HWND parent, const char *cls, DWORD style, int id,
                           int x, int y, int w, int hgt, const char *text = nullptr )
{
    HWND h = CreateWindowExA( 0, cls, text, WS_CHILD | WS_VISIBLE | style,
                              x, y, w, hgt, parent, (HMENU)(INT_PTR)id,
                              AfxGetInstanceHandle(), NULL );
    if ( h && s_vehFont )
        SendMessageA( h, WM_SETFONT, (WPARAM)s_vehFont, 0 );
    return h;
}

BEGIN_MESSAGE_MAP( CVehicleDlg, CWnd )
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_BN_CLICKED( IDC_VEH_SET_HEALTH, &CVehicleDlg::OnSetHealth )
    ON_BN_CLICKED( IDC_VEH_SET_ACCUR,  &CVehicleDlg::OnSetAccuracy )
    ON_BN_CLICKED( IDC_VEH_SET_SPEED,  &CVehicleDlg::OnSetSpeed )
    ON_BN_CLICKED( IDC_VEH_SET_LOOK,   &CVehicleDlg::OnSetLookahead )
    ON_BN_CLICKED( IDC_VEH_CLR_HEALTH, &CVehicleDlg::OnClearHealth )
    ON_BN_CLICKED( IDC_VEH_CLR_ACCUR,  &CVehicleDlg::OnClearAccuracy )
    ON_BN_CLICKED( IDC_VEH_CLR_SPEED,  &CVehicleDlg::OnClearSpeed )
    ON_BN_CLICKED( IDC_VEH_CLR_LOOK,   &CVehicleDlg::OnClearLookahead )
    ON_BN_CLICKED( IDC_VEH_SET_CRASH,  &CVehicleDlg::OnSetCrashType )
    ON_BN_CLICKED( IDC_VEH_CLR_CRASH,  &CVehicleDlg::OnClearCrashType )
    ON_CONTROL_RANGE( BN_CLICKED, IDC_VEH_DEATHROLL_ON, IDC_VEH_TEAM_AXIS, &CVehicleDlg::OnToggle )
    ON_CONTROL_RANGE( BN_CLICKED, IDC_VEH_SG_SPAWNGROUP, IDC_VEH_SG_ATTACKORGS, &CVehicleDlg::OnScriptGroup )
END_MESSAGE_MAP()

CVehicleDlg::CVehicleDlg()
{
}

int CVehicleDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;
    if ( !s_vehFont )
        s_vehFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

    HWND self = GetSafeHwnd();
    const int M = 10, lblW = 110, inW = 150, btnW = 50, rowH = 26;
    int lx   = M;
    int ix   = M + lblW + 4;
    int setx = ix + inW + 6;
    int clrx = setx + btnW + 4;
    int y = M;

    struct { const char *lbl; HWND *out; int eid, sid, cid; } edits[] = {
        { "startinghealth:", &s_vehHealth, IDC_VEH_HEALTH, IDC_VEH_SET_HEALTH, IDC_VEH_CLR_HEALTH },
        { "accuracy(0-100):",&s_vehAccur,  IDC_VEH_ACCUR,  IDC_VEH_SET_ACCUR,  IDC_VEH_CLR_ACCUR  },
        { "speed:",          &s_vehSpeed,  IDC_VEH_SPEED,  IDC_VEH_SET_SPEED,  IDC_VEH_CLR_SPEED  },
        { "lookahead:",      &s_vehLook,   IDC_VEH_LOOK,   IDC_VEH_SET_LOOK,   IDC_VEH_CLR_LOOK   },
    };
    for ( int n = 0; n < 4; ++n )
    {
        VEH_MakeChild( self, "static", SS_LEFT, 0, lx, y + 4, lblW, 16, edits[n].lbl );
        *edits[n].out = VEH_MakeChild( self, "edit", WS_BORDER | ES_AUTOHSCROLL,
                                       edits[n].eid, ix, y, inW, 22 );
        VEH_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, edits[n].sid, setx, y, btnW, 22, "Set" );
        VEH_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, edits[n].cid, clrx, y, btnW, 22, "Clear" );
        y += rowH;
    }

    // crash type — a combobox of the 3 presets (default/plane/forced).
    VEH_MakeChild( self, "static", SS_LEFT, 0, lx, y + 4, lblW, 16, "crashtype:" );
    s_vehCrash = VEH_MakeChild( self, "combobox",
                                CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
                                IDC_VEH_CRASH, ix, y, inW, 120 );
    if ( s_vehCrash )
    {
        ::SendMessageA( s_vehCrash, CB_ADDSTRING, 0, (LPARAM)"default" );
        ::SendMessageA( s_vehCrash, CB_ADDSTRING, 0, (LPARAM)"plane" );
        ::SendMessageA( s_vehCrash, CB_ADDSTRING, 0, (LPARAM)"forced" );
        ::SendMessageA( s_vehCrash, CB_SETCURSEL, 0, 0 );
    }
    VEH_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_VEH_SET_CRASH, setx, y, btnW, 22, "Set" );
    VEH_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, IDC_VEH_CLR_CRASH, clrx, y, btnW, 22, "Clear" );
    y += rowH + 6;

    // on/off toggle pushbuttons: each key a labelled pair.
    struct { const char *lbl; int onId, offId; const char *onTxt, *offTxt; } toggles[] = {
        { "deathroll:",     IDC_VEH_DEATHROLL_ON, IDC_VEH_DEATHROLL_OFF, "On", "Off" },
        { "turret:",        IDC_VEH_TURRET_ON,    IDC_VEH_TURRET_OFF,    "On", "Off" },
        { "turretmg:",      IDC_VEH_TURRETMG_ON,  IDC_VEH_TURRETMG_OFF,  "On", "Off" },
        { "badplace:",      IDC_VEH_BADPLACE_ON,  IDC_VEH_BADPLACE_OFF,  "On", "Off" },
        { "avoidvehicles:", IDC_VEH_AVOID_ON,     IDC_VEH_AVOID_OFF,     "On", "Off" },
        { "attackai:",      IDC_VEH_ATTACKAI_ON,  IDC_VEH_ATTACKAI_OFF,  "On", "Off" },
        { "team:",          IDC_VEH_TEAM_ALLIES,  IDC_VEH_TEAM_AXIS,     "Allies", "Axis" },
    };
    for ( int n = 0; n < 7; ++n )
    {
        VEH_MakeChild( self, "static", SS_LEFT, 0, lx, y + 4, lblW, 16, toggles[n].lbl );
        VEH_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, toggles[n].onId,  ix,        y, btnW + 14, 22, toggles[n].onTxt );
        VEH_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, toggles[n].offId, ix + btnW + 20, y, btnW + 14, 22, toggles[n].offTxt );
        y += rowH;
    }
    y += 6;

    // script-group buttons: clicking one assigns the next free group number under that
    // key to the selection (ScriptGroup_AssignNextNumber).  Laid out 2 columns x 5 rows.
    VEH_MakeChild( self, "static", SS_LEFT, 0, lx, y, 360, 16, "Script groups (assign next number to selection):" );
    y += 20;
    struct { const char *lbl; int id; } sg[] = {
        { "spawngroup",    IDC_VEH_SG_SPAWNGROUP  }, { "startmove",   IDC_VEH_SG_STARTMOVE   },
        { "groupdelete",   IDC_VEH_SG_GROUPDELETE }, { "ride",        IDC_VEH_SG_RIDE        },
        { "walk",          IDC_VEH_SG_WALK        }, { "attackgroup", IDC_VEH_SG_ATTACKGROUP },
        { "focusfire",     IDC_VEH_SG_FOCUSFIRE   }, { "detour",      IDC_VEH_SG_DETOUR      },
        { "gatetrigger",   IDC_VEH_SG_GATETRIGGER }, { "attackorgs",  IDC_VEH_SG_ATTACKORGS  },
    };
    const int sgW = 170, sgGap = 8;
    for ( int n = 0; n < 10; ++n )
    {
        int col = n & 1;
        VEH_MakeChild( self, "button", BS_PUSHBUTTON | WS_TABSTOP, sg[n].id,
                       lx + col * ( sgW + sgGap ), y, sgW, 22, sg[n].lbl );
        if ( col )
            y += rowH;
    }
    return 0;
}

// ── per-field commit: read the edit, SetPair (non-empty) / RemovePair (empty) ──
// Faithful to each edit handler (0x460140 etc.): GetWindowText → empty? RemovePair :
// SetPair.  (The accuracy field additionally scales — handled in OnSetAccuracy.)
static void VEH_Commit( HWND field, const char *key )
{
    if ( !field )
        return;
    char buf[1024] = { 0 };
    ::GetWindowTextA( field, buf, sizeof( buf ) - 1 );
    if ( buf[0] )
        VehicleDlg_SetPair( buf, key );
    else
        VehicleDlg_RemovePair( key );
    g_nUpdateBits |= 1;
}

void CVehicleDlg::OnSetHealth()   { VEH_Commit( s_vehHealth, "script_startinghealth" ); }
void CVehicleDlg::OnSetSpeed()    { VEH_Commit( s_vehSpeed,  "speed" ); }
void CVehicleDlg::OnSetLookahead(){ VEH_Commit( s_vehLook,   "lookahead" ); }

// Accuracy (0x460250): empty → RemovePair (binary resets the slider to 44 first); else
// value = atol(text)/100.0 formatted "%.2f" → SetPair("script_accuracy", value).
void CVehicleDlg::OnSetAccuracy()
{
    if ( !s_vehAccur )
        return;
    char buf[256] = { 0 };
    ::GetWindowTextA( s_vehAccur, buf, sizeof( buf ) - 1 );
    if ( !buf[0] )
    {
        VehicleDlg_RemovePair( "script_accuracy" );
        g_nUpdateBits |= 1;
        return;
    }
    char val[64];
    _snprintf( val, sizeof( val ), "%.2f", (double)atol( buf ) / 100.0 );
    VehicleDlg_SetPair( val, "script_accuracy" );
    g_nUpdateBits |= 1;
}

void CVehicleDlg::OnClearHealth()   { VehicleDlg_RemovePair( "script_startinghealth" ); g_nUpdateBits |= 1; }
void CVehicleDlg::OnClearAccuracy() { VehicleDlg_RemovePair( "script_accuracy" );       g_nUpdateBits |= 1; }
void CVehicleDlg::OnClearSpeed()    { VehicleDlg_RemovePair( "speed" );                 g_nUpdateBits |= 1; }
void CVehicleDlg::OnClearLookahead(){ VehicleDlg_RemovePair( "lookahead" );             g_nUpdateBits |= 1; }

// Crash type (0x460700): CB_GETCURSEL → 0 default / 1 plane / 2 forced → SetPair.
void CVehicleDlg::OnSetCrashType()
{
    if ( !s_vehCrash )
        return;
    int cur = (int)::SendMessageA( s_vehCrash, CB_GETCURSEL, 0, 0 );
    const char *v = ( cur == 1 ) ? "plane" : ( cur == 2 ) ? "forced" : "default";
    VehicleDlg_SetPair( v, "script_crashtype" );
    g_nUpdateBits |= 1;
}
void CVehicleDlg::OnClearCrashType() { VehicleDlg_RemovePair( "script_crashtype" ); g_nUpdateBits |= 1; }

// on/off toggle pushbuttons — each button is a fixed SetPair (the binary's per-button
// thunks at 0x45FDE0.. each SetKeyValue a constant value, e.g. script_turret 1/0).
void CVehicleDlg::OnToggle( UINT nID )
{
    switch ( nID )
    {
    case IDC_VEH_DEATHROLL_ON:  VehicleDlg_SetPair( "1", "script_deathroll" );      break;
    case IDC_VEH_DEATHROLL_OFF: VehicleDlg_SetPair( "0", "script_deathroll" );      break;
    case IDC_VEH_TURRET_ON:     VehicleDlg_SetPair( "1", "script_turret" );         break;
    case IDC_VEH_TURRET_OFF:    VehicleDlg_SetPair( "0", "script_turret" );         break;
    case IDC_VEH_TURRETMG_ON:   VehicleDlg_SetPair( "1", "script_turretmg" );       break;
    case IDC_VEH_TURRETMG_OFF:  VehicleDlg_SetPair( "0", "script_turretmg" );       break;
    case IDC_VEH_BADPLACE_ON:   VehicleDlg_SetPair( "1", "script_badplace" );       break;
    case IDC_VEH_BADPLACE_OFF:  VehicleDlg_SetPair( "0", "script_badplace" );       break;
    case IDC_VEH_AVOID_ON:      VehicleDlg_SetPair( "1", "script_avoidvehicles" );  break;
    case IDC_VEH_AVOID_OFF:     VehicleDlg_SetPair( "0", "script_avoidvehicles" );  break;
    case IDC_VEH_ATTACKAI_ON:   VehicleDlg_SetPair( "1", "script_attackai" );       break;
    case IDC_VEH_ATTACKAI_OFF:  VehicleDlg_SetPair( "0", "script_attackai" );       break;
    case IDC_VEH_TEAM_ALLIES:   VehicleDlg_SetPair( "allies", "script_team" );      break;
    case IDC_VEH_TEAM_AXIS:     VehicleDlg_SetPair( "axis", "script_team" );        break;
    default: return;
    }
    g_nUpdateBits |= 1;
}

// script-group buttons (the binary's thunks @0x45fd50.. + OnAttackOrgs @0x460670): each
// is VehicleDlg_SetScriptGroupKey("<key>") → store the key NAME into ScriptGroupKey, then
// ScriptGroup_AssignNextNumber assigns the next free group number to the selection.
void CVehicleDlg::OnScriptGroup( UINT nID )
{
    const char *key;
    switch ( nID )
    {
    case IDC_VEH_SG_SPAWNGROUP:  key = "script_vehiclespawngroup";     break;
    case IDC_VEH_SG_STARTMOVE:   key = "script_vehiclestartmove";      break;
    case IDC_VEH_SG_GROUPDELETE: key = "script_vehiclegroupdelete";    break;
    case IDC_VEH_SG_RIDE:        key = "script_vehicleride";           break;
    case IDC_VEH_SG_WALK:        key = "script_vehiclewalk";           break;
    case IDC_VEH_SG_ATTACKGROUP: key = "script_vehicleattackgroup";    break;
    case IDC_VEH_SG_FOCUSFIRE:   key = "script_vehiclefocusfiregroup"; break;
    case IDC_VEH_SG_DETOUR:      key = "script_vehicledetour";         break;
    case IDC_VEH_SG_GATETRIGGER: key = "script_gatetrigger";           break;
    case IDC_VEH_SG_ATTACKORGS:  key = "script_attackorgs";            break;
    default: return;
    }
    VehicleDlg_SetScriptGroupKey( key );
    g_nUpdateBits |= 1;
}

void CVehicleDlg::OnClose()
{
    // Modeless: hide rather than destroy (OnMiscVehicleGroup toggles visibility).
    ShowWindow( SW_HIDE );
}

void CVehicleDlg::PostNcDestroy()
{
    g_dlgVehicle = nullptr;
    s_vehHealth = s_vehAccur = s_vehSpeed = s_vehLook = s_vehCrash = nullptr;
    delete this;
}

// 0x42BD50  CMainFrame::OnMiscVehicleGroup toggles this; first call creates it.
void CVehicleDlg::Toggle()
{
    if ( g_dlgVehicle && ::IsWindow( g_dlgVehicle->GetSafeHwnd() ) )
    {
        BOOL vis = g_dlgVehicle->IsWindowVisible();
        g_dlgVehicle->ShowWindow( vis ? SW_HIDE : SW_SHOW );
        if ( !vis )
            g_dlgVehicle->SetForegroundWindow();
        g_nUpdateBits |= 1;
        return;
    }

    CWnd *parent = AfxGetMainWnd();
    g_dlgVehicle = new CVehicleDlg();

    const DWORD style   = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    const DWORD exStyle = WS_EX_TOOLWINDOW;
    int px = 220, py = 160;
    if ( parent && parent->GetSafeHwnd() )
    {
        RECT r;  parent->GetWindowRect( &r );
        px = r.left + 140;  py = r.top + 100;
    }
    CRect rc( px, py, px + 400, py + 640 );
    if ( !g_dlgVehicle->CreateEx( exStyle,
             AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW,
                 ::LoadCursor( NULL, IDC_ARROW ), (HBRUSH)( COLOR_BTNFACE + 1 ), NULL ),
             "Vehicle Group", style, rc, parent, 0 ) )
    {
        delete g_dlgVehicle;
        g_dlgVehicle = nullptr;
        return;
    }
    g_nUpdateBits |= 1;
}

#ifdef RADIANT_SELFTEST
// ══════════════════════════════════════════════════════════════════════════════
//  Deterministic test entry (the `vehicle` gate).  Drives the dialog's CORE writers
//  WITHOUT any HWNDs — exactly as the dynent/scriptgrp gates drive the key writers on
//  a selected entity directly:  SetPair(value, key) / RemovePair(key).
//  RunVehicleTest (radiantapp.cpp) creates a vehicle entity, selects it, sets the keys,
//  saves + reloads, and asserts they round-trip.
// ══════════════════════════════════════════════════════════════════════════════
extern "C" int Radiant_TestVehicleSetPair( const char *value, const char *key )
{
    VehicleDlg_SetPair( value, key );
    return 0;
}
extern "C" int Radiant_TestVehicleRemovePair( const char *key )
{
    VehicleDlg_RemovePair( key );
    return 0;
}
#endif // RADIANT_SELFTEST
