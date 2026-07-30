#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Script-group editor: assigns color, trigger, team, and numbered-group epairs
// to selected entities.

#include "stdafx.h"     // pulls in qe3.h (g_qeglobals / brush lists / Com_Error / ERR_FATAL)
                       //   and res/resource.h (IDD_SCRIPT_GROUP_NAME = 217 / 0xD9)
#include "prefs.h"      // prefData_t / g_PrefsDlg (ScriptGroupKey / ScriptColorTeamKey CStrings)
#include <cstring>
#include <cstdlib>     // atol / _itoa

extern void Assert( const char *file, int line, int type, const char *fmt, ... );

// ── externs (per-TU, matching the established pattern in select.cpp/brush.cpp) ──
extern int          Sys_Printf( const char *fmt, ... );
extern char        *va( const char *fmt, ... );
extern entity_s    *world_entity;                                         // 0x25D5B30 (map.cpp)
extern void         SetKeyValue( entity_s_def *e, const char *key, const char *value );  // 0x483690
extern void         DeleteKey( epair_t **head, const char *key );         // 0x483720
extern bool         Entity_HasEpairMatch( entity_s *e, const char *key, const char *val );// 0x483930
extern bool         HasKeyValuePair( entity_s_def *e, const char *key );  // 0x4838B0 (entity.cpp)
extern char        *ValueForKey2( int defPtr, const char *key );          // 0x4825C0 (entity.cpp)
extern int          UpdateSelection( int wParam, eclass_t *cls );         // 0x497180 (win_ent.cpp)
// selected_brushes / active_brushes sentinels are declared in qe3.h.

// ─────────────────────────────────────────────────────────────────────────────
// 0x451170  ScriptGroup_Unreachable  (70 bytes)   [PORTED — dependency-free]
// Maps a script-colour code substring to its index 0..6 (r,b,y,c,g,p,o =
// red/blue/yellow/cyan/green/purple/orange).  Returns -1 (and asserts) if none
// match.  The only scriptgroup.cpp function with no g_PrefsDlg/UpdateSelection
// dependency.  (off_73B07C in the binary; ASSERT_UNKOWN gate omitted — the assert
// message is literally "Unreachable", i.e. the not-found path is a programming
// error, so we always assert there.)
// ─────────────────────────────────────────────────────────────────────────────
int ScriptGroup_Unreachable( const char *a1 )
{
    static const char *const codes[7] = { "r", "b", "y", "c", "g", "p", "o" };
    for ( int i = 0; i < 7; ++i )
    {
        if ( strstr( a1, codes[i] ) )
            return i;
    }
    Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\ScriptGroup.cpp",
            39, 0, "Unreachable" );
    return -1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  KEY ADD/REMOVE subsystem (PORTED — real, gate-verified)
// ═════════════════════════════════════════════════════════════════════════════
//
// CoD editor design fact (relied on by every iterator's Assert and by the def
// dereferences): for a placed entity, entity_s and entity_s_def are the SAME struct
// and `def` is self-referential (entity IS its own def), so the
// binary's `b->owner->def == b->def->owner` assert is `entity->def == entity`.  The
// epair list head lives at offset 0x74 on that def (entity_s::epairs).  The asserts
// pass type=0 (engine_stubs Assert only DebugBreak()s on type!=0), so they log-and-
// continue — faithful and gate-safe.

// 0x4537A0  ScriptGroup_AddKey  (512) — ADD a space-separated value-token to one
// entity's key.  Binary: copy the existing key value into a 1024 buffer; if empty,
// the value becomes the whole string; else tokenise and, if the value isn't already
// a token, append " <value>".  (The decompile's v16/v17 contiguous-buffer + strtok-
// then-restore dance is exactly a strcpy(existing)+membership-test+strcat below.)
static void ScriptGroup_AddKey( const char *key, const char *value, entity_s_def *def )
{
    // Resolve the existing value for `key` (case-insensitive, like the binary's _stricmp walk).
    const char *existing = "";
    for ( epair_t *ep = def->epairs; ep; ep = ep->next )
    {
        if ( !_stricmp( ep->key, key ) ) { existing = ep->value; break; }
    }

    char buf[1024];
    strcpy( buf, existing );

    if ( !buf[0] )
    {
        // No existing value → the value becomes the whole string.
        Sys_Printf( "Added value %s to key %s\n", value, key );
        strcpy( buf, value );
        SetKeyValue( def, key, buf );
        return;
    }

    // Existing value present → test membership against a scratch copy (the binary
    // strtok's buf in place, then restores it from the saved value pointer; tokenising
    // a copy keeps buf intact so we can append directly — identical net string).
    char scratch[1024];
    strcpy( scratch, buf );
    for ( char *token = strtok( scratch, " " ); token; token = strtok( nullptr, " " ) )
    {
        iassert( token[0] );
        if ( !strcmp( token, value ) )
        {
            Sys_Printf( "Entity already has value %s on their key %s\n", value, key );
            return;
        }
    }

    // Not present → append " <value>" (binary: strcpy(buf,existing)+va(" %s",value)).
    strcat( buf, va( " %s", value ) );
    SetKeyValue( def, key, buf );
    Sys_Printf( "Added value %s to key %s\n", value, key );
}

// 0x453440  ScriptGroup_HasKey  (553) — REMOVE a space-separated value-token from one
// entity's key (the name is from the ID-pass; it is a REMOVE, not a query).  Binary:
// copy the existing value into v18; tokenise; rebuild the kept tokens (each followed by
// a trailing space) into the contiguous v19; then SetKeyValue(v19) if non-empty else
// DeleteKey.  The TRAILING SPACE on every kept token is preserved verbatim (the binary
// emits `va("%s ", token)`), which is load-bearing for the .map round-trip diff.
static void ScriptGroup_HasKey( const char *value, const char *key, entity_s_def *def )
{
    const char *existing = "";
    for ( epair_t *ep = def->epairs; ep; ep = ep->next )
    {
        if ( !_stricmp( ep->key, key ) ) { existing = ep->value; break; }
    }

    char buf[1024];
    strcpy( buf, existing );
    if ( !buf[0] )
    {
        Sys_Printf( "Entity doesn't have key %s\n", key );
        return;
    }

    char out[1024];
    out[0] = 0;
    bool hasKey = false;

    char *token = strtok( buf, " " );
    if ( !token )
    {
        Sys_Printf( "Entity doesn't have key %s value %s\n", key, value );
        return;
    }
    do
    {
        iassert( token[0] );
        if ( !strcmp( token, value ) )
        {
            iassert( !hasKey );
            hasKey = true;
        }
        else
        {
            // Keep this token — append "<token> " (TRAILING SPACE, matching va("%s ",tok)).
            strcat( out, va( "%s ", token ) );
        }
        token = strtok( nullptr, " " );
    }
    while ( token );

    if ( !hasKey )
    {
        Sys_Printf( "Entity doesn't have key %s value %s\n", key, value );
        return;
    }

    if ( strlen( out ) )
        SetKeyValue( def, key, out );
    else
        DeleteKey( &def->epairs, key );           // binary: DeleteKey(&def->epairs, key)
    Sys_Printf( "Removed value %s from key %s\n", value, key );
}

// ── selection iterators ───────────────────────────────────────────────────────
// All four walk selected_brushes, skip world/null-owner brushes, and (faithfully) carry
// the `b->owner->def == b->def->owner` self-ref assert; the *Triggers variants additionally
// gate on the entity being trigger_multiple/trigger_radius.  Each ends with UpdateSelection.

// 0x453A50  ScriptGroup_AddKeyToSelected — add `value` to `key` on every selected entity.
// (Binary calls ScriptGroup_AddKey(key, value, def); the dialog wrapper sub_454780 passes
//  value=field2(ScriptSubValue), key=field1(ScriptSubKey).)
int ScriptGroup_AddKeyToSelected( const char *value, const char *key )
{
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        iassert( b->owner->def == b->def->owner );
        ScriptGroup_AddKey( key, value, (entity_s_def *)owner->def );
    }
    return UpdateSelection( 0xFFFFFFFF, 0 );
}

// 0x4539A0  ScriptGroup_AddKeyToSelectedTriggers — add only to selected TRIGGER entities.
int ScriptGroup_AddKeyToSelectedTriggers( const char *key, const char *value )
{
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        iassert( b->owner->def == b->def->owner );
        entity_s_def *def = (entity_s_def *)owner->def;
        if ( Entity_HasEpairMatch( def, "classname", "trigger_multiple" )
          || Entity_HasEpairMatch( def, "classname", "trigger_radius" ) )
            ScriptGroup_AddKey( key, value, def );
    }
    return UpdateSelection( 0xFFFFFFFF, 0 );
}

// 0x453670  ScriptGroup_RemoveKeyFromSelected — remove `value` from `key` on every
// selected entity.  (Binary calls ScriptGroup_HasKey(value, key, def); wrapper sub_4549B0
// passes key=field1, value=field2.)
int ScriptGroup_RemoveKeyFromSelected( const char *key, const char *value )
{
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        iassert( b->owner->def == b->def->owner );
        ScriptGroup_HasKey( value, key, (entity_s_def *)owner->def );
    }
    return UpdateSelection( 0xFFFFFFFF, 0 );
}

// 0x4536F0  ScriptGroup_RemoveKeyFromSelectedTriggers — remove only from selected TRIGGER ents.
int ScriptGroup_RemoveKeyFromSelectedTriggers( const char *key, const char *value )
{
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        iassert( b->owner->def == b->def->owner );
        entity_s_def *def = (entity_s_def *)owner->def;
        if ( Entity_HasEpairMatch( def, "classname", "trigger_multiple" )
          || Entity_HasEpairMatch( def, "classname", "trigger_radius" ) )
            ScriptGroup_HasKey( value, key, def );
    }
    return UpdateSelection( 0xFFFFFFFF, 0 );
}

// ═════════════════════════════════════════════════════════════════════════════
//  SCRIPT-GROUP NUMBER ASSIGNMENT (PORTED — real, scriptgrpcolor-gate verified)
// ═════════════════════════════════════════════════════════════════════════════
//
// This is the core both the Script-Group dialog's colour buttons AND the Vehicle
// dialog's 10 script-group buttons drive.  Each button stores a key NAME into
// g_PrefsDlg->ScriptGroupKey (VehicleDlg_SetScriptGroupKey 0x45FBA0 →
// str_set(&ScriptGroupKey, key)) and then calls ScriptGroup_AssignNextNumber, which
// finds the highest existing group number stored under that key across all
// (non-selected) world entities and assigns max+1 to the current selection.
//
// SCOPE NOTE — this is NOT the colour-token machinery (ScriptGroup_RemoveColors[_02]
// / ScriptGroup_Color / the single-char ScriptColorKey "r/b/y/c/g/p/o" codes).  Those
// edit a DIFFERENT key (g_PrefsDlg->ScriptColorTeamKey, a space-separated list of
// <colourCode><number> tokens) and remain PARKED with the Script-Group MFC dialog
// (see the worklist below).  AssignNextNumber writes a single SCALAR number under
// ScriptGroupKey via SetKeyValue (wholesale replace), so its faithful inverse is a
// plain DeleteKey of that key — ScriptGroup_RemoveAssignedNumber below (NOT
// ScriptGroup_RemoveColors, which is the colour-list subsystem).

// 0x479FF0  ScriptGroup_AssignNextNumber — assign the next free group NUMBER (under
// g_PrefsDlg->ScriptGroupKey) to every selected entity.  Faithful to the disasm:
//   * guard: only run when ScriptGroupKey != ScriptColorTeamKey (the colour-team key
//     uses the colour-token path instead; a vehicle/group key is always != the team key)
//   * pass 1: walk active_brushes, max = -1; for each non-world entity carrying the key,
//     atol(value) and keep the max (skip empty values)
//   * pass 2: itoa(max+1) and SetKeyValue it onto every selected non-world entity
// (The asserts carry the binary's literal brush.cpp file/line — AssignNextNumber was
//  inlined from a brush.cpp context; type 0 → log+continue, type 1 → would-be-fatal but
//  ValueForKey2 returns the "" global, never null, so the *v guard handles it.)
void ScriptGroup_AssignNextNumber()
{
    const char *groupKey = (const char *)g_PrefsDlg->ScriptGroupKey;
    if ( !strcmp( groupKey, (const char *)g_PrefsDlg->ScriptColorTeamKey ) )
        return;   // ScriptGroupKey == ScriptColorTeamKey → the colour-token path owns it.

    // PASS 1 — find the highest existing number under groupKey across active brushes.
    int maxNum = -1;
    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        entity_s_def *def = (entity_s_def *)owner->def;
        // Binary 0x47a080 derefs b->def->owner UNCONDITIONALLY (no `b->def &&` guard) — matches
        // the sibling iterators; the prior port's `b->def &&` was an invented guard, dropped.
        if ( def != (entity_s_def *)b->def->owner )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp",
                    4293, 0, "%s", "b->owner->def == b->def->owner" );

        if ( !HasKeyValuePair( def, groupKey ) )
            continue;
        const char *val = ValueForKey2( (int)(intptr_t)def, groupKey );
        if ( !val )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp",
                    4298, 1, "%s", "group" );
        if ( val && *val )
        {
            int n = atol( val );
            if ( maxNum < n )
                maxNum = n;
        }
    }

    // PASS 2 — assign maxNum+1 to every selected non-world entity.
    char next[16];
    _itoa( maxNum + 1, next, 10 );
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        entity_s_def *def = (entity_s_def *)owner->def;
        // Binary 0x47a149 derefs b->def->owner UNCONDITIONALLY; `b->def &&` was invented, dropped.
        if ( def != (entity_s_def *)b->def->owner )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp",
                    4313, 0, "%s", "b->owner->def == b->def->owner" );
        SetKeyValue( def, groupKey, next );
    }
}

// 0x45FBA0  VehicleDlg_SetScriptGroupKey — the Vehicle dialog's script-group button
// entry: store the chosen key NAME into g_PrefsDlg->ScriptGroupKey, then assign the next
// group number to the selection.  (The binary's str_set is a CString assignment; the
// EndDialog/SetFocus/g_nUpdateBits tail is the modeless-dialog "applied, return focus"
// gesture, supplied by the caller in the UI.)  Ported here so the assignment closure
// lives with the rest of the script-group machinery; vehicledlg.cpp's buttons call it.
void VehicleDlg_SetScriptGroupKey( const char *key )
{
    g_PrefsDlg->ScriptGroupKey = ( key ? key : "" );
    ScriptGroup_AssignNextNumber();
}

// ── INVERSE (the faithful undo of AssignNextNumber's scalar write) ─────────────
// AssignNextNumber does SetKeyValue(def, ScriptGroupKey, "<N>") — a wholesale scalar
// write — so removing it is a plain DeleteKey of that key on every selected non-world
// entity.  (This is the number subsystem's inverse; the colour-LIST inverse is the
// PARKED ScriptGroup_RemoveColors[_02], a different key + token-strip.)  Used by the
// scriptgrpcolor gate to verify assign → remove → gone.
void ScriptGroup_RemoveAssignedNumber( const char *key )
{
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        entity_s_def *def = (entity_s_def *)owner->def;
        if ( b->def && def != (entity_s_def *)b->def->owner )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp",
                    4313, 0, "%s", "b->owner->def == b->def->owner" );
        DeleteKey( &def->epairs, key );      // entity_s::epairs @0x74
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  COLOUR-GROUP / TRIGGER-NUMBER machinery (partial — ScriptGroup_Type only)
// ═════════════════════════════════════════════════════════════════════════════
#define MAX_COLORGROUPS 512
// MAX_COLORENTREES is the per-turret export-token cap in the turret-share helper.
#define MAX_COLORENTREES 32

// IDB `zero` (0x6d58f0) is the shared empty-string global the colour fns default an absent
// epair value to.  Reproduced as a local empty string (value-identical to "").
static const char zero[] = "";

extern void Brush_RemoveFromList( selbrush_t *b );   // brush.cpp 0x476680
extern void Brush_AddToList2( selbrush_t *b );       // brush.cpp 0x4765a0
extern void Select_Deselect( int bDeselectFaces );   // select.cpp 0x48E800
extern int  g_nUpdateBits;                           // engine_stubs.cpp 0x25D5A74
extern void sub_47D060( int listSentinel );          // brush.cpp (brush-list display rebuild)

// 0x451200  ScriptGroup_Type  (ScriptGroup.cpp:62)
// Grow the selection by trigger COLOUR GROUP: gather the colour-group numbers of every
// selected trigger_multiple/trigger_radius entity (parsed from the ScriptColorTeamKey
// epair's ScriptColorKey-prefixed token), then select every active trigger whose own
// colour-group number is one of them.  Called by SelectedAssociated when
// ScriptGroupKey == ScriptColorTeamKey.  The binary builds the per-entity value string
// into a 1024 stack buffer (inlined strcpy of the `zero`-defaulted epair value) and
// parses it with strtok/strstr/atol; reproduced 1:1 below.  The active-brush walk saves
// `&match->next->prev` before Brush_RemoveFromList/AddToList2 relink the node.
void ScriptGroup_Type()
{
    const char *scriptColorTeamKey = g_PrefsDlg->ScriptColorTeamKey;
    const char *scriptColorKey     = g_PrefsDlg->ScriptColorKey;

    int colorGroups[MAX_COLORGROUPS];
    int colors = 0;

    // Pass 1 — collect each selected trigger entity's colour-group number.
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;

        iassert( b->owner->def == b->def->owner );

        entity_s_def *def = (entity_s_def *)owner->def;
        if ( !Entity_HasEpairMatch( def, "classname", "trigger_multiple" )
             && !Entity_HasEpairMatch( def, "classname", "trigger_radius" ) )
            continue;

        const char *value = "";   // IDB `zero` — empty string when the key is absent
        for ( epair_t *ep = def->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, scriptColorTeamKey ) ) { value = ep->value; break; }
        }

        char buf[1024];
        strcpy( buf, value );
        if ( !buf[0] )
            continue;
        if ( !strstr( buf, scriptColorKey ) )
            continue;

        // Find the single ScriptColorKey-prefixed token and atol its numeric tail.
        int triggerNumber = -1;
        for ( char *token = strtok( buf, " " ); token; token = strtok( nullptr, " " ) )
        {
            iassert( token[0] );
            if ( strstr( token, scriptColorKey ) )
            {
                if ( triggerNumber != -1 )
                    Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\ScriptGroup.cpp",
                            98, 0, "%s\n\t(token) = %s", "(triggerNumber == -1)", token );
                triggerNumber = atol( &token[strlen( scriptColorKey )] );
            }
        }
        if ( triggerNumber != -1 )
            iassert( triggerNumber < MAX_COLORGROUPS );
        else
            iassert( triggerNumber != -1 );

        colorGroups[colors++] = triggerNumber;
        iassert( colors < MAX_COLORGROUPS );
    }

    // Pass 2 — select every active trigger whose colour-group number is in the set.
    for ( selbrush_t *bAll = active_brushes.next; bAll != &active_brushes; )
    {
        selbrush_t *next = bAll->next;   // captured before Brush_RemoveFromList/AddToList2 relink
        entity_s *owner = bAll->owner;
        if ( owner && owner != world_entity )
        {
            iassert( bAll->owner->def == bAll->def->owner );

            entity_s_def *def = (entity_s_def *)bAll->owner->def;
            const char *value = "";
            for ( epair_t *ep = def->epairs; ep; ep = ep->next )
            {
                if ( !_stricmp( ep->key, scriptColorTeamKey ) ) { value = ep->value; break; }
            }

            char buf[1024];
            strcpy( buf, value );
            if ( buf[0] && strstr( buf, scriptColorKey ) )
            {
                for ( char *token = strtok( buf, " " ); token; token = strtok( nullptr, " " ) )
                {
                    iassert( token[0] );
                    if ( strstr( token, scriptColorKey ) )
                    {
                        int triggerNumber = atol( &token[strlen( scriptColorKey )] );
                        for ( int n = 0; n < colors; ++n )
                        {
                            if ( colorGroups[n] == triggerNumber )
                            {
                                Brush_RemoveFromList( bAll );
                                Brush_AddToList2( bAll );
                                break;
                            }
                        }
                    }
                }
            }
        }
        bAll = next;
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  COLOUR-GROUP / TRIGGER-NUMBER cluster
// ═════════════════════════════════════════════════════════════════════════════
//
// CONVENTIONS used throughout (all verified against the disasm, NOT hex-rays):
//  * selbrush_t (display-list node): owner@0x08, def@0x14.  The self-ref invariant
//    `b->owner->def == b->def->owner` (= entity->def == entity) is carried verbatim.
//  * the colour-team value is a space-separated list of "<colourCode><number>" tokens
//    stored under the epair key g_PrefsDlg->ScriptColorTeamKey; the single-char colour
//    code is (const char*)g_PrefsDlg->ScriptColorKey (IDB `*(const char**)…ScriptColorKey`
//    is the CString-buffer-as-ptr idiom).  `zero` (IDB) → "".
//  * the binary copies an epair value into a 1024 stack buffer with an inlined strcpy
//    (`v5 = buf - src; src[v5] = *src; ++src`) — reproduced as strcpy below.
//  * the rebuild-into-a-fixed-16-byte-stride-scratch + rejoin-with-trailing-spaces dance
//    (`strcpy(scratch+16*n, tok)` then `qmemcpy(&buf[..],scratch+16*n); strcpy(end," ")`)
//    is reproduced as a real scratch[N][16] + strcat with a trailing space.

// ─────────────────────────────────────────────────────────────────────────────
// 0x453fd0  ScriptGroup_BrushIsTrigger (ScriptGroup.cpp:1150) — true if the brush's
// entity is trigger_multiple/trigger_radius.  (Leaf.)  Exported (non-static) so the
// render-decoration draws (brush.cpp DrawModels_Decorations / camwnd.cpp Cam_DrawTokens)
// can gate on it.
// ─────────────────────────────────────────────────────────────────────────────
bool ScriptGroup_BrushIsTrigger( selbrush_t *b )
{
    entity_s *owner = b->owner;
    if ( !owner || owner == world_entity )
        return false;
    iassert( b->owner->def == b->def->owner );
    entity_s_def *def = (entity_s_def *)owner->def;
    return Entity_HasEpairMatch( def, "classname", "trigger_multiple" )
        || Entity_HasEpairMatch( def, "classname", "trigger_radius" );
}

// 0x454050  ScriptGroup_BrushIsTurret (ScriptGroup.cpp:1162) — true if the brush's
// entity is misc_turret.  (Leaf; unused by the wired paths but ported for completeness.)
static bool ScriptGroup_BrushIsTurret( selbrush_t *b )
{
    entity_s *owner = b->owner;
    if ( !owner || owner == world_entity )
        return false;
    iassert( b->owner->def == b->def->owner );
    return Entity_HasEpairMatch( (entity_s_def *)owner->def, "classname", "misc_turret" );
}

// 0x454260  ScriptGroup_IsValidColorCode (already IDB-renamed) — true if the current
// ScriptColorKey exactly matches one of the 7 single-char codes {r,b,y,c,g,p,o}.
// (Binary walks off_73B07C with strcmp until the array-end sentinel flt_73B098.)
static bool ScriptGroup_IsValidColorCode()
{
    static const char *const codes[7] = { "r", "b", "y", "c", "g", "p", "o" };
    const char *key = (const char *)g_PrefsDlg->ScriptColorKey;
    for ( int i = 0; i < 7; ++i )
    {
        if ( !strcmp( key, codes[i] ) )
            return true;
    }
    return false;
}

// 0x451a40  ScriptGroup_TeamKeyContains (already IDB-renamed) — true if the entity DEF
// `def`'s ScriptColorTeamKey value contains the substring `code`.  Walks the def's epairs
// directly (defaults to `zero`=""), copies into a 1024 buffer, returns buf[0]&&strstr.
static bool ScriptGroup_TeamKeyContains( const char *code, entity_s_def *def )
{
    const char *value = zero;       // IDB `zero` = ""
    for ( epair_t *ep = def->epairs; ep; ep = ep->next )
    {
        if ( !_stricmp( ep->key, (const char *)g_PrefsDlg->ScriptColorTeamKey ) ) { value = ep->value; break; }
    }
    char buf[1024];
    strcpy( buf, value );
    return buf[0] && strstr( buf, code ) != nullptr;
}

// 0x4540b0  ScriptGroup_TriggerColorNumber (already IDB-renamed) — read the FIRST
// ScriptColorKey-prefixed token's number from the brush entity's ScriptColorTeamKey
// value, or -1 if absent.  (Single-match: no `triggerNumber==-1` duplicate assert.)
static int ScriptGroup_TriggerColorNumber( selbrush_t *b )
{
    entity_s_def *def = (entity_s_def *)b->owner->def;
    const char *scriptColorKey = (const char *)g_PrefsDlg->ScriptColorKey;

    const char *value = zero;
    for ( epair_t *ep = def->epairs; ep; ep = ep->next )
    {
        if ( !_stricmp( ep->key, (const char *)g_PrefsDlg->ScriptColorTeamKey ) ) { value = ep->value; break; }
    }
    char buf[1024];
    strcpy( buf, value );
    if ( !buf[0] || !strstr( buf, scriptColorKey ) )
        return -1;

    for ( char *token = strtok( buf, " " ); token; token = strtok( nullptr, " " ) )
    {
        iassert( token[0] );
        if ( strstr( token, scriptColorKey ) )
            return atol( &token[strlen( scriptColorKey )] );
    }
    return -1;
}

// 0x454220  ScriptGroup_SelectionTriggerNumber (already IDB-renamed) — the LAST
// trigger-colour number across the selected trigger brushes (or -1).
static int ScriptGroup_SelectionTriggerNumber()
{
    int n = -1;
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        if ( ScriptGroup_BrushIsTrigger( b ) )
            n = ScriptGroup_TriggerColorNumber( b );
    }
    return n;
}

// 0x453e50  ScriptGroup_SelectionHasTrigger — true if any selected entity is a
// trigger_multiple/trigger_radius.  Non-static — OnScriptGroup_Disassociate calls it.
bool ScriptGroup_SelectionHasTrigger()
{
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        if ( ScriptGroup_BrushIsTrigger( b ) )
            return true;
    }
    return false;
}

// 0x452a50  ScriptGroup_SelectedHasColorTeam — true if the brush's NON-trigger entity
// (AI / node / goal volume) holds the substring `code` in its ScriptColorTeamKey value.
// (Returns false for trigger entities — the trigger side is handled separately.)
static bool ScriptGroup_SelectedHasColorTeam( selbrush_t *b, const char *code )
{
    entity_s *owner = b->owner;
    if ( !owner || owner == world_entity )
        return false;
    iassert( b->owner->def == b->def->owner );
    entity_s_def *def = (entity_s_def *)owner->def;
    if ( Entity_HasEpairMatch( def, "classname", "trigger_multiple" ) )
        return false;
    if ( Entity_HasEpairMatch( def, "classname", "trigger_radius" ) )
        return false;

    char buf[1024];
    strcpy( buf, ValueForKey2( (int)(intptr_t)def, (const char *)g_PrefsDlg->ScriptColorTeamKey ) );
    return buf[0] && strstr( buf, code ) != nullptr;
}

// 0x451650  ScriptGroup_Color — pick the FIRST free colour-group number (0..511) not
// currently used by any active OR selected trigger entity.  Builds a used[512] bitmap by
// parsing every entity's ScriptColorTeamKey token list (atol of each ScriptColorKey-
// prefixed token → used[n]=1), then returns the first index where used==0.
static int ScriptGroup_Color()
{
    char used[MAX_COLORGROUPS];
    memset( used, 0, sizeof( used ) );
    const char *scriptColorKey = (const char *)g_PrefsDlg->ScriptColorKey;

    // Pass 1 — active brushes.
    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        iassert( b->owner->def == b->def->owner );

        entity_s_def *def = (entity_s_def *)owner->def;
        const char *value = zero;
        for ( epair_t *ep = def->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, (const char *)g_PrefsDlg->ScriptColorTeamKey ) ) { value = ep->value; break; }
        }
        char buf[1024];
        strcpy( buf, value );
        if ( !buf[0] || !strstr( buf, scriptColorKey ) )
            continue;
        for ( char *token = strtok( buf, " " ); token; token = strtok( nullptr, " " ) )
        {
            iassert( token[0] );
            if ( strstr( token, scriptColorKey ) )
            {
                int n = atol( &token[strlen( scriptColorKey )] );
                iassert( n < MAX_COLORGROUPS );      // "colorNumber < MAX_COLORGROUPS"
                used[n] = 1;
            }
        }
    }

    // Pass 2 — selected brushes (identical parse).
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        iassert( b->owner->def == b->def->owner );

        entity_s_def *def = (entity_s_def *)owner->def;
        const char *value = zero;
        for ( epair_t *ep = def->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, (const char *)g_PrefsDlg->ScriptColorTeamKey ) ) { value = ep->value; break; }
        }
        char buf[1024];
        strcpy( buf, value );
        if ( !buf[0] || !strstr( buf, scriptColorKey ) )
            continue;
        for ( char *token = strtok( buf, " " ); token; token = strtok( nullptr, " " ) )
        {
            iassert( token[0] );
            if ( strstr( token, scriptColorKey ) )
            {
                int n = atol( &token[strlen( scriptColorKey )] );
                iassert( n < MAX_COLORGROUPS );
                used[n] = 1;
            }
        }
    }

    // First free index.  (Binary: while (used[result]) if (++result>=512) assert "unreachable".)
    int result = 0;
    while ( used[result] )
    {
        if ( ++result >= MAX_COLORGROUPS )
        {
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\ScriptGroup.cpp",
                    249, 0, "unreachable" );
            return 0;
        }
    }
    return result;
}

// ── helper used by ScriptGroup_SetKey/SetKey2/ApplyColorToSelected/RemoveColors* ──
// Rejoin a fixed 16-byte-stride scratch array of `count` tokens into `out` with a
// trailing space after EACH token (matching the binary's `qmemcpy + strcpy(" ")` dance).
// Net string is byte-identical (incl. the trailing space, load-bearing for the .map diff).
static void ScriptGroup_JoinTokens( char *out, const char ( *scratch )[16], int count )
{
    for ( int n = 0; n < count; ++n )
    {
        strcat( out, scratch[n] );
        strcat( out, " " );
    }
}

// 0x4520c0  ScriptGroup_SetKey2 (ScriptGroup.cpp:401) — remove the colour token whose
// number == `colorNumber` from ONE brush entity's ScriptColorTeamKey value (keep all
// other tokens, re-joined with trailing spaces).  Brush arrives in ecx (RemoveUnused
// passes it); colorNumber is the second arg.  SetKeyValue if non-empty else DeleteKey.
static void ScriptGroup_SetKey2( selbrush_t *b, int colorNumber )
{
    entity_s *owner = b->owner;
    if ( !owner || owner == world_entity )
        return;
    iassert( b->owner->def == b->def->owner );
    entity_s_def *def = (entity_s_def *)owner->def;

    const char *scriptColorKey = (const char *)g_PrefsDlg->ScriptColorKey;
    char buf[1024];
    strcpy( buf, ValueForKey2( (int)(intptr_t)def, (const char *)g_PrefsDlg->ScriptColorTeamKey ) );
    if ( !buf[0] || !strstr( buf, scriptColorKey ) )
        return;

    char scratch[MAX_COLORGROUPS][16];
    int kept = 0;
    for ( char *token = strtok( buf, " " ); token; token = strtok( nullptr, " " ) )
    {
        iassert( token[0] );
        // Keep every token EXCEPT the one matching this colour code AND number.
        if ( !strstr( token, scriptColorKey ) || atol( &token[strlen( scriptColorKey )] ) != colorNumber )
        {
            strcpy( scratch[kept], token );
            ++kept;
        }
    }

    char out[1024];
    out[0] = 0;
    if ( kept > 0 )
        ScriptGroup_JoinTokens( out, scratch, kept );

    if ( strlen( out ) )
        SetKeyValue( def, (const char *)g_PrefsDlg->ScriptColorTeamKey, out );
    else
        DeleteKey( &def->epairs, (const char *)g_PrefsDlg->ScriptColorTeamKey );
    Sys_Printf( "Set entity key value to %s\n", out );
}

// 0x452330  ScriptGroup_RemoveUnused (ScriptGroup.cpp:~447) — strip colour group
// `colorNumber` from EVERY selected then EVERY active brush entity (via SetKey2).
static void ScriptGroup_RemoveUnused( int colorNumber )
{
    Sys_Printf( "Removing unused %s %i from all entities:\n",
                (const char *)g_PrefsDlg->ScriptColorKey, colorNumber );
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        ScriptGroup_SetKey2( b, colorNumber );
    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
        ScriptGroup_SetKey2( b, colorNumber );
}

// 0x452a... forward decl needed by RemoveUnusedIfOrphaned.
// 0x452b50  ScriptGroup_RemoveUnusedIfOrphaned (already IDB-renamed) — remove colour
// group `colorNumber` everywhere ONLY IF no non-trigger (AI/node) entity still references
// it (scan selected then active brushes for the "<code><n> " token; bail if found).
static bool ScriptGroup_RemoveUnusedIfOrphaned( int colorNumber )
{
    char token[1024];
    strcpy( token, va( "%s%i ", (const char *)g_PrefsDlg->ScriptColorKey, colorNumber ) );

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        if ( ScriptGroup_SelectedHasColorTeam( b, token ) )
            return true;       // still in use → keep
    }
    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
    {
        if ( ScriptGroup_SelectedHasColorTeam( b, token ) )
            return true;
    }
    ScriptGroup_RemoveUnused( colorNumber );
    return false;
}

// 0x4523a0  ScriptGroup_RemoveColors (ScriptGroup.cpp:~478) — strip colour token
// (code `colorCode`, number `colorNumber`) from every selected NON-trigger (AI/node)
// entity.  Returns true if any entity was modified.  (The binary takes `colorCode` as a
// CString-by-value and releases it at the end; we take a plain const char* — it is only
// ever read via strstr/strlen — so the CString fork/release pair in the caller is dropped.)
static bool ScriptGroup_RemoveColors( int colorNumber, const char *colorCode )
{
    bool modified = false;
    Sys_Printf( "Removing colorNumber %s %i\n", colorCode, colorNumber );

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        iassert( b->owner->def == b->def->owner );
        entity_s_def *def = (entity_s_def *)owner->def;
        if ( Entity_HasEpairMatch( def, "classname", "trigger_multiple" )
          || Entity_HasEpairMatch( def, "classname", "trigger_radius" ) )
            continue;

        const char *value = zero;
        for ( epair_t *ep = def->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, (const char *)g_PrefsDlg->ScriptColorTeamKey ) ) { value = ep->value; break; }
        }
        char buf[1024];
        strcpy( buf, value );
        if ( !buf[0] || !strstr( buf, colorCode ) )
            continue;

        char scratch[MAX_COLORGROUPS][16];
        char out[1024];
        out[0] = 0;
        int kept = 0;
        for ( char *token = strtok( buf, " " ); token; token = strtok( nullptr, " " ) )
        {
            iassert( token[0] );
            if ( !strstr( token, colorCode ) || atol( &token[strlen( colorCode )] ) != colorNumber )
            {
                strcpy( scratch[kept], token );
                ++kept;
            }
        }
        if ( kept > 0 )
            ScriptGroup_JoinTokens( out, scratch, kept );

        if ( strlen( out ) )
            SetKeyValue( def, (const char *)g_PrefsDlg->ScriptColorTeamKey, out );
        else
            DeleteKey( &def->epairs, (const char *)g_PrefsDlg->ScriptColorTeamKey );
        modified = true;
        Sys_Printf( "Set entity key value %s\n", out );
    }
    return modified;
}

// 0x452720  ScriptGroup_RemoveColors_02 (ScriptGroup.cpp:~553) — the TRIGGER-entity
// variant of the above (uses g_PrefsDlg->ScriptColorKey directly; no return value, no
// CString release).  Strips colour group `colorNumber` from selected trigger entities.
static void ScriptGroup_RemoveColors_02( int colorNumber )
{
    Sys_Printf( "Removing colorNumber %s %i\n",
                (const char *)g_PrefsDlg->ScriptColorKey, colorNumber );
    const char *scriptColorKey = (const char *)g_PrefsDlg->ScriptColorKey;

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        iassert( b->owner->def == b->def->owner );
        entity_s_def *def = (entity_s_def *)owner->def;
        if ( !Entity_HasEpairMatch( def, "classname", "trigger_multiple" )
          && !Entity_HasEpairMatch( def, "classname", "trigger_radius" ) )
            continue;

        const char *value = zero;
        for ( epair_t *ep = def->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, (const char *)g_PrefsDlg->ScriptColorTeamKey ) ) { value = ep->value; break; }
        }
        char buf[1024];
        strcpy( buf, value );
        if ( !buf[0] || !strstr( buf, scriptColorKey ) )
            continue;

        char scratch[MAX_COLORGROUPS][16];
        char out[1024];
        out[0] = 0;
        int kept = 0;
        for ( char *token = strtok( buf, " " ); token; token = strtok( nullptr, " " ) )
        {
            iassert( token[0] );
            if ( !strstr( token, scriptColorKey ) || atol( &token[strlen( scriptColorKey )] ) != colorNumber )
            {
                strcpy( scratch[kept], token );
                ++kept;
            }
        }
        if ( kept > 0 )
            ScriptGroup_JoinTokens( out, scratch, kept );

        if ( strlen( out ) )
            SetKeyValue( def, (const char *)g_PrefsDlg->ScriptColorTeamKey, out );
        else
            DeleteKey( &def->epairs, (const char *)g_PrefsDlg->ScriptColorTeamKey );
        Sys_Printf( "Set entity key value %s\n", out );
    }
}

// 0x453170  ScriptGroup_TriggerNumber — the DISASSOCIATE core.  For each selected
// trigger's single colour number, try removing it from non-trigger entities; if NONE had
// it (RemoveColors returned false) fall back to removing it from the trigger entities;
// then RemoveUnusedIfOrphaned.  (The binary passes a CString COPY of ScriptColorKey to
// RemoveColors via sub_40F2D0 [CString fork] and RemoveColors releases it; we pass the
// const char* directly — behaviour-identical, the fork/release pair is dropped.)
void ScriptGroup_TriggerNumber()
{
    const char *scriptColorKey = (const char *)g_PrefsDlg->ScriptColorKey;

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        iassert( b->owner->def == b->def->owner );
        entity_s_def *def = (entity_s_def *)owner->def;
        if ( !Entity_HasEpairMatch( def, "classname", "trigger_multiple" )
          && !Entity_HasEpairMatch( def, "classname", "trigger_radius" ) )
            continue;

        const char *value = zero;
        for ( epair_t *ep = def->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, (const char *)g_PrefsDlg->ScriptColorTeamKey ) ) { value = ep->value; break; }
        }
        char buf[1024];
        strcpy( buf, value );
        if ( !buf[0] || !strstr( buf, scriptColorKey ) )
            continue;

        int triggerNumber = -1;
        bool hasColor = false;
        for ( char *token = strtok( buf, " " ); token; token = strtok( nullptr, " " ) )
        {
            iassert( token[0] );
            if ( strstr( token, scriptColorKey ) )
            {
                if ( triggerNumber != -1 )
                    Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\ScriptGroup.cpp",
                            842, 0, "%s", "triggerNumber == -1" );
                triggerNumber = atol( &token[strlen( scriptColorKey )] );
                if ( hasColor )
                    Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\ScriptGroup.cpp",
                            846, 0, "%s", "!hasColor" );
                hasColor = true;
            }
        }
        if ( triggerNumber != -1 )
            iassert( triggerNumber < MAX_COLORGROUPS );
        else
            iassert( triggerNumber != -1 );

        if ( !ScriptGroup_RemoveColors( triggerNumber, scriptColorKey ) )
            ScriptGroup_RemoveColors_02( triggerNumber );
        ScriptGroup_RemoveUnusedIfOrphaned( triggerNumber );
    }
}

// 0x452c20  ScriptGroup_Trigger (ScriptGroup.cpp:~683) — reconcile a trigger COLOUR
// GROUP toward target `colorNumber`.  Two 512-bitmaps (verified slot-for-slot vs disasm):
//   selSeen = var_604 (written by PASS 1, the SELECTED triggers, at 0x452ea2)
//   actSeen = var_804 (written by PASS 2, the ACTIVE  triggers, at 0x453100)
// PASS 1 marks selSeen[n] for every selected trigger's colour number n != target.
// PASS 2 marks actSeen[n] for every active  trigger's colour number n != target (read-
//   only; the active walk uses the SAME selbrush_t ->next link [esi+4], not an entity
//   onext — the hex-rays `entity_brush_s::onext`/`owner->owner` were type artifacts).
// FINAL (0x453130): for j in 0..511, if selSeen[j] && j!=target && !actSeen[j] →
//   RemoveUnused(j) — drop a colour group a selected trigger used that no active one does.
static void ScriptGroup_Trigger( int colorNumber )
{
    const char *scriptColorKey = (const char *)g_PrefsDlg->ScriptColorKey;
    char selSeen[MAX_COLORGROUPS];   // var_604 — PASS 1 (selected)
    char actSeen[MAX_COLORGROUPS];   // var_804 — PASS 2 (active)
    memset( selSeen, 0, sizeof( selSeen ) );
    memset( actSeen, 0, sizeof( actSeen ) );

    // PASS 1 — selected triggers (single colour number each, asserted).
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        iassert( b->owner->def == b->def->owner );
        entity_s_def *def = (entity_s_def *)owner->def;
        if ( !Entity_HasEpairMatch( def, "classname", "trigger_multiple" )
          && !Entity_HasEpairMatch( def, "classname", "trigger_radius" ) )
            continue;

        const char *value = zero;
        for ( epair_t *ep = def->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, (const char *)g_PrefsDlg->ScriptColorTeamKey ) ) { value = ep->value; break; }
        }
        char buf[1024];
        strcpy( buf, value );
        if ( !buf[0] || !strstr( buf, scriptColorKey ) )
            continue;

        int triggerNumber = -1;
        for ( char *token = strtok( buf, " " ); token; token = strtok( nullptr, " " ) )
        {
            iassert( token[0] );
            if ( strstr( token, scriptColorKey ) )
            {
                if ( triggerNumber != -1 )
                    Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\ScriptGroup.cpp",
                            725, 0, "%s", "triggerNumber == -1" );
                triggerNumber = atol( &token[strlen( scriptColorKey )] );
            }
        }
        if ( triggerNumber == -1 )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\ScriptGroup.cpp",
                    729, 0, "%s", "triggerNumber != -1" );
        if ( triggerNumber != colorNumber )
        {
            iassert( triggerNumber < MAX_COLORGROUPS );
            selSeen[triggerNumber] = 1;          // var_604 (0x452ea2)
        }
    }

    // PASS 2 — active triggers (read-only; same ->next link as the selected list).
    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        iassert( b->owner->def == b->def->owner );   // "bAll->owner->def == bAll->def->owner"
        entity_s_def *def = (entity_s_def *)owner->def;
        if ( !Entity_HasEpairMatch( def, "classname", "trigger_multiple" )
          && !Entity_HasEpairMatch( def, "classname", "trigger_radius" ) )
            continue;

        const char *value = zero;
        for ( epair_t *ep = def->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, (const char *)g_PrefsDlg->ScriptColorTeamKey ) ) { value = ep->value; break; }
        }
        char buf[1024];
        strcpy( buf, value );
        if ( !buf[0] || !strstr( buf, scriptColorKey ) )
            continue;

        int triggerNumber = -1;
        bool hasColor = false;
        for ( char *token = strtok( buf, " " ); token; token = strtok( nullptr, " " ) )
        {
            iassert( token[0] );
            if ( strstr( token, scriptColorKey ) )
            {
                triggerNumber = atol( &token[strlen( scriptColorKey )] );
                if ( triggerNumber == colorNumber )
                    break;                       // target found on an active trigger — stop scanning
                if ( hasColor )
                    Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\ScriptGroup.cpp",
                            774, 0, "%s\n\t(token) = %s", "(!hasColor)", token );
                hasColor = true;
            }
        }
        if ( triggerNumber == -1 )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\ScriptGroup.cpp",
                    778, 0, "%s", "triggerNumber != -1" );
        if ( triggerNumber != colorNumber )
        {
            iassert( triggerNumber < MAX_COLORGROUPS );
            actSeen[triggerNumber] = 1;          // var_804 (0x453100)
        }
    }

    // FINAL — drop numbers a selected trigger uses (other than target) that no active
    // trigger still uses.  (IDB 0x453130: if (selSeen[j] && j!=target && !actSeen[j]).)
    for ( int j = 0; j < MAX_COLORGROUPS; ++j )
    {
        if ( selSeen[j] && j != colorNumber && !actSeen[j] )
            ScriptGroup_RemoveUnused( j );
    }
}

// 0x453ad0  ScriptGroup_ApplyColorToSelected (ScriptGroup.cpp:~1043) — set the colour
// token to `colorNumber` on every selected trigger entity, REPLACING any existing colour
// token (exactly one allowed: "!hasColor" assert) while keeping non-colour tokens.
static void ScriptGroup_ApplyColorToSelected( int colorNumber )
{
    const char *scriptColorKey = (const char *)g_PrefsDlg->ScriptColorKey;

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        iassert( b->owner->def == b->def->owner );
        entity_s_def *def = (entity_s_def *)owner->def;
        if ( !Entity_HasEpairMatch( def, "classname", "trigger_multiple" )
          && !Entity_HasEpairMatch( def, "classname", "trigger_radius" ) )
            continue;

        const char *value = zero;
        for ( epair_t *ep = def->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, (const char *)g_PrefsDlg->ScriptColorTeamKey ) ) { value = ep->value; break; }
        }
        char buf[1024];
        strcpy( buf, value );

        char out[1024];
        out[0] = 0;
        bool hasColor = false;
        if ( buf[0] )
        {
            char scratch[MAX_COLORGROUPS][16];
            int kept = 0;
            for ( char *token = strtok( buf, " " ); token; token = strtok( nullptr, " " ) )
            {
                iassert( token[0] );
                if ( strstr( token, scriptColorKey ) )
                {
                    if ( hasColor )
                        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\ScriptGroup.cpp",
                                1076, 0, "%s", "!hasColor" );
                    hasColor = true;
                }
                else
                {
                    strcpy( scratch[kept], token );
                    ++kept;
                }
            }
            if ( kept > 0 )
                ScriptGroup_JoinTokens( out, scratch, kept );
            // The binary appends one extra trailing space after the kept block here.
            strcat( out, " " );
        }
        // Append the new colour token "<code><n> " (trailing space).
        strcat( out, va( "%s%i ", scriptColorKey, colorNumber ) );

        if ( strlen( out ) )
            SetKeyValue( def, (const char *)g_PrefsDlg->ScriptColorTeamKey, out );
        else
            DeleteKey( &def->epairs, (const char *)g_PrefsDlg->ScriptColorTeamKey );
        Sys_Printf( "Set trigger key value to %s\n", out );
    }
}

// 0x451af0  ScriptGroup_01 (ScriptGroup.cpp:~276) — colour-token reconciliation against
// one node/AI entity DEF (`def`) for colour number `colorNumber`.  For each selected
// trigger whose team key already holds "<code><n>", walk that trigger's team-key tokens
// and, for any token (other than "<code><n>") that `def` ALSO carries, remove it from
// `def`'s team key.  Keeps a node/AI entity from holding colour tokens a selected trigger
// now owns.  (Called from ScriptGroup_SetKey for each freshly-written token.)
static void ScriptGroup_01( entity_s_def *def, int colorNumber )
{
    const char *scriptColorKey = (const char *)g_PrefsDlg->ScriptColorKey;

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        iassert( b->owner->def == b->def->owner );
        entity_s_def *trigDef = (entity_s_def *)owner->def;
        if ( !Entity_HasEpairMatch( trigDef, "classname", "trigger_multiple" )
          && !Entity_HasEpairMatch( trigDef, "classname", "trigger_radius" ) )
            continue;

        // colourToken = "<code><n>" (no trailing space here — binary va("%s%i",code,n)).
        char colourToken[1024];
        strcpy( colourToken, va( "%s%i", scriptColorKey, colorNumber ) );

        // Only act when THIS trigger's team key already contains the colour token.
        if ( !ScriptGroup_TeamKeyContains( colourToken, trigDef ) )
            continue;

        char list[1024];
        strcpy( list, ValueForKey2( (int)(intptr_t)trigDef, (const char *)g_PrefsDlg->ScriptColorTeamKey ) );
        for ( char *token = strtok( list, " " ); token; token = strtok( nullptr, " " ) )
        {
            iassert( token[0] );
            if ( strcmp( colourToken, token ) )       // skip the colour token itself
            {
                if ( ScriptGroup_TeamKeyContains( token, def ) )
                {
                    // Remove `token` from def's team-key value.  The binary builds the key via
                    // va("%s%", ScriptColorTeamKey) — the format string is literally "%s%" with a
                    // trailing LONE '%'.  Both the binary and this port route va through _vsnprintf,
                    // which emits NOTHING for a malformed trailing '%', so the key is exactly the
                    // team key (e.g. "script_color_allies").  Passing the same "%s%" reproduces it
                    // byte-for-byte (do NOT write "%s%%" — that would append a literal '%').
                    char teamKeyArg[256];
                    strcpy( teamKeyArg, va( "%s%", (const char *)g_PrefsDlg->ScriptColorTeamKey ) );
                    ScriptGroup_HasKey( token, teamKeyArg, def );
                }
            }
        }
    }
}

// 0x451d40  ScriptGroup_SetKey (ScriptGroup.cpp:~328) — write the colour token to every
// selected NODE/AI/info_volume entity's team key (append to existing list, or create it),
// then reconcile via ScriptGroup_01.  Skips entities whose classname isn't actor/node/
// info_volume; a "node" with no radius gets radius=64.
static void ScriptGroup_SetKey( int colorNumber )
{
    const char *scriptColorKey = (const char *)g_PrefsDlg->ScriptColorKey;

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        iassert( b->owner->def == b->def->owner );
        entity_s_def *def = (entity_s_def *)owner->def;

        const char *classname = zero;
        for ( epair_t *ep = def->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, "classname" ) ) { classname = ep->value; break; }
        }

        // Filter: only actor / node / info_volume entities are eligible.
        if ( !strstr( classname, "actor" ) )
        {
            if ( strstr( classname, "node" ) )
            {
                if ( !strcmp( ValueForKey2( (int)(intptr_t)def, "radius" ), zero ) )
                    SetKeyValue( def, "radius", "64" );
            }
            else if ( !strstr( classname, "info_volume" ) )
            {
                continue;
            }
        }

        // Read existing team-key value.
        const char *value = zero;
        for ( epair_t *ep = def->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, (const char *)g_PrefsDlg->ScriptColorTeamKey ) ) { value = ep->value; break; }
        }
        char tempString[1024];
        strcpy( tempString, value );

        if ( tempString[0] )
        {
            // Append " <code><n> " (the binary's leading-and-trailing-space variant).
            strcat( tempString, va( " %s%i ", scriptColorKey, colorNumber ) );
            iassert( strlen( tempString ) );      // "strlen(tempString)"
            SetKeyValue( def, (const char *)g_PrefsDlg->ScriptColorTeamKey, tempString );
            Sys_Printf( "Set node/AI key value to %s\n", tempString );
            // Reconcile each token (binary strtok's tempString and calls ScriptGroup_01 per token).
            for ( char *token = strtok( tempString, " " ); token; token = strtok( nullptr, " " ) )
            {
                iassert( token[0] );
                ScriptGroup_01( def, colorNumber );
            }
        }
        else
        {
            // Create the value as "<code><n> " (trailing space).
            strcpy( tempString, va( "%s%i ", scriptColorKey, colorNumber ) );
            if ( strlen( tempString ) == 0 )
                DeleteKey( &def->epairs, (const char *)g_PrefsDlg->ScriptColorTeamKey );
            else
                SetKeyValue( def, (const char *)g_PrefsDlg->ScriptColorTeamKey, tempString );
            Sys_Printf( "Set node/AI key value to %s\n", tempString );
        }
    }
}

// 0x4543b0  ScriptGroup_AddColorToSelection (ScriptGroup.cpp:~1243) — the top-level
// "add colour group to selection" command.  If the selection has triggers: pick/reconcile
// a colour number, apply it to the triggers, write the team token onto node/AI entities.
// Else if a valid 1-char colour code is selected: tag selected ACTOR entities with
// script_forcecolor.  Else prompt.
void ScriptGroup_AddColorToSelection()
{
    if ( ScriptGroup_SelectionHasTrigger() )
    {
        int triggerNumber = ScriptGroup_SelectionTriggerNumber();
        if ( triggerNumber == -1 )
        {
            triggerNumber = ScriptGroup_Color();
        }
        else
        {
            ScriptGroup_Trigger( triggerNumber );
        }
        if ( triggerNumber >= MAX_COLORGROUPS )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\ScriptGroup.cpp",
                    1284, 0, "%s", "triggerNumber < MAX_COLORGROUPS" );
        Sys_Printf( "Adding %s%i to entities:\n",
                    (const char *)g_PrefsDlg->ScriptColorKey, triggerNumber );
        ScriptGroup_ApplyColorToSelected( triggerNumber );
        ScriptGroup_SetKey( triggerNumber );
        UpdateSelection( 0xFFFFFFFF, 0 );
        g_nUpdateBits = -1;
    }
    else if ( ScriptGroup_IsValidColorCode() )
    {
        for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        {
            entity_s *owner = b->owner;
            if ( !owner || owner == world_entity )
                continue;
            iassert( b->owner->def == b->def->owner );
            entity_s_def *def = (entity_s_def *)owner->def;

            const char *classname = zero;
            for ( epair_t *ep = def->epairs; ep; ep = ep->next )
            {
                if ( !_stricmp( ep->key, "classname" ) ) { classname = ep->value; break; }
            }
            if ( strstr( classname, "actor" ) )
                SetKeyValue( def, "script_forcecolor", (const char *)g_PrefsDlg->ScriptColorKey );
        }
    }
    else
    {
        Sys_Printf( "Select a color you want to add to the entity\n" );
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  SCRIPT-GROUP MFC DIALOG (template IDD_SCRIPT_GROUP_NAME = 217 / 0xD9, radiant.rc)
// ═════════════════════════════════════════════════════════════════════════════
//
// The dialog drives the cluster above. It uses the .rc template (the binary calls
// CreateDialogParamA(hInst, MAKEINTRESOURCE(0xD9), ...)), so no hand-built DLGTEMPLATE is
// needed — radiant.rc already defines IDD_SCRIPT_GROUP_NAME with all the controls.  The
// numeric control IDs below are the binary's raw immediates (decoded from disasm; the
// hex-rays IDC_*|ID_*|0x200 names were enum-OR artifacts), matching res/resource.h's
// IDC_<n> symbols.  Control map:
//   1441 group-key edit        1631 sub-key edit      1635 sub-value edit
//   1636 flag-value edit (true)  1639 flag-value edit (false)
//   1661..1667 colour radios r,b,y,c,g,p,o            1668 allies  1670 axis radio
//   1671 script_flag_true listbox    1298 script_flag_false listbox
//
// MFC CString machinery in the original sub-handlers (sub_454780/9B0/BF0/D00 fork two
// local CString scratch buffers, read a control into them, and release at function end)
// is just UI-string plumbing — reproduced here with plain char[256] locals (str_set is a
// no-op stub in this build, so it must NOT carry value; the real value is the char buffer).

// (GetDlgItem / SendMessageA / CheckDlgButton / CreateDialogParamA come from <windows.h>
//  via stdafx.h; the dialog code below uses the ::A variants to stay ANSI/_MBCS.)

// 0x454e40  ScriptGroup_HasFlag — fill listbox `dlgItemID` with the DISTINCT token values
// held under epair key `key` across the selected trigger entities.  (Binary uses raw
// listbox messages: 0x184=LB_RESETCONTENT, 0x180=LB_ADDSTRING — the hex-rays WM_*|WM_SETICON
// ORs were artifacts.)  Dedups via a 16-byte-stride scratch, capped implicitly by the list.
void ScriptGroup_HasFlag( const char *key, int dlgItemID, HWND hDlg )
{
    HWND hWnd = GetDlgItem( hDlg, dlgItemID );
    ::SendMessageA( hWnd, LB_RESETCONTENT, 0, 0 );

    char seen[256][16];
    int  seenCount = 0;

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        if ( !owner || owner == world_entity )
            continue;
        iassert( b->owner->def == b->def->owner );
        entity_s_def *def = (entity_s_def *)owner->def;
        if ( !Entity_HasEpairMatch( def, "classname", "trigger_multiple" )
          && !Entity_HasEpairMatch( def, "classname", "trigger_radius" ) )
            continue;

        const char *value = zero;
        for ( epair_t *ep = def->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, key ) ) { value = ep->value; break; }
        }
        char buf[1024];
        strcpy( buf, value );
        if ( !buf[0] )
            continue;

        for ( char *token = strtok( buf, " " ); token; token = strtok( nullptr, " " ) )
        {
            iassert( token[0] );
            // Add only if not already seen.
            bool found = false;
            for ( int i = 0; i < seenCount; ++i )
            {
                if ( !strcmp( token, seen[i] ) ) { found = true; break; }
            }
            if ( !found )
            {
                ::SendMessageA( hWnd, LB_ADDSTRING, 0, (LPARAM)token );
                strcpy( seen[seenCount], token );
                ++seenCount;
            }
        }
    }
}

// 0x454480  ScriptGroup_SyncGroupKeyToTeam (was sub_454480) — force the active script
// "group key" to be the colour-team key (so the colour-team path owns the selection), save
// prefs, then (if the dialog is open) mirror the value into the group-key edit control and
// return focus to the camera.  The two menu handlers call this BEFORE the cluster ops, to
// switch the system into colour-team mode.  (CString assign; the GUI part is d_hwndMedia-
// guarded so it is a no-op off the dialog.)  Non-static — the CMainFrame handlers call it.
void ScriptGroup_SyncGroupKeyToTeam()
{
    g_PrefsDlg->ScriptGroupKey = g_PrefsDlg->ScriptColorTeamKey;   // CString_Assign
    Prefs_SavePrefs( g_PrefsDlg );
    if ( g_qeglobals.d_hwndMedia )
    {
        HWND hCtl = GetDlgItem( g_qeglobals.d_hwndMedia, 1441 );
        ::SetWindowTextA( hCtl, (const char *)g_PrefsDlg->ScriptGroupKey );
        ::SetFocus( g_qeglobals.d_hwndCamera );
    }
}

// ── dialog button handlers (the sub_454*/455B* helpers) ───────────────────────
// Each reads its edit control(s) and drives a cluster function.  The original CString
// fork/release is replaced with char[256] locals (see header note).

// 0x454630  ScriptGroupDlg_OnOK — "OK": commit the group-key edit, AssignNextNumber, close.
static void ScriptGroupDlg_OnOK( HWND hDlg )
{
    char text[256];
    ::GetWindowTextA( GetDlgItem( hDlg, 1441 ), text, 255 );
    if ( text[0] )
    {
        g_PrefsDlg->ScriptGroupKey = text;
        Prefs_SavePrefs( g_PrefsDlg );
        ScriptGroup_AssignNextNumber();
        UpdateSelection( 0xFFFFFFFF, 0 );
        g_nUpdateBits = -1;
    }
    ::EndDialog( hDlg, 1 );
}

// 0x454780  ScriptGroupDlg_AddSubKey — read sub-key (1631) + sub-value (1635) edits, store
// to prefs if non-empty, then add that key/value to every selected entity.
static void ScriptGroupDlg_AddSubKey( HWND hDlg )
{
    char subKey[256], subValue[256];
    ::GetWindowTextA( GetDlgItem( hDlg, 1631 ), subKey, 255 );
    if ( subKey[0] )
    {
        g_PrefsDlg->ScriptSubKey_key = subKey;
        Prefs_SavePrefs( g_PrefsDlg );
    }
    ::GetWindowTextA( GetDlgItem( hDlg, 1635 ), subValue, 255 );
    if ( subValue[0] )
    {
        g_PrefsDlg->ScriptSubValue_key = subValue;
        Prefs_SavePrefs( g_PrefsDlg );
    }
    ScriptGroup_AddKeyToSelected( subValue, subKey );   // (value, key)
    ::SetFocus( g_qeglobals.d_hwndCamera );
}

// 0x4549b0  ScriptGroupDlg_RemoveSubKey — same controls, but REMOVE the key/value.
static void ScriptGroupDlg_RemoveSubKey( HWND hDlg )
{
    char subKey[256], subValue[256];
    ::GetWindowTextA( GetDlgItem( hDlg, 1631 ), subKey, 255 );
    if ( subKey[0] )
    {
        g_PrefsDlg->ScriptSubKey_key = subKey;
        Prefs_SavePrefs( g_PrefsDlg );
    }
    ::GetWindowTextA( GetDlgItem( hDlg, 1635 ), subValue, 255 );
    if ( subValue[0] )
    {
        g_PrefsDlg->ScriptSubValue_key = subValue;
        Prefs_SavePrefs( g_PrefsDlg );
    }
    ScriptGroup_RemoveKeyFromSelected( subKey, subValue );   // (key, value)
    ::SetFocus( g_qeglobals.d_hwndCamera );
}

// 0x454bf0  ScriptGroupDlg_AddFlag — read the flag-value edit `dlgItemID`, add it under
// `key` to every selected TRIGGER entity.
static void ScriptGroupDlg_AddFlag( const char *key, HWND hDlg, int dlgItemID )
{
    char text[256];
    ::GetWindowTextA( GetDlgItem( hDlg, dlgItemID ), text, 255 );
    ScriptGroup_AddKeyToSelectedTriggers( key, text );
    ::SetFocus( g_qeglobals.d_hwndCamera );
}

// 0x454d00  ScriptGroupDlg_RemoveFlag — same, but REMOVE.
static void ScriptGroupDlg_RemoveFlag( const char *key, HWND hDlg, int dlgItemID )
{
    char text[256];
    ::GetWindowTextA( GetDlgItem( hDlg, dlgItemID ), text, 255 );
    ScriptGroup_RemoveKeyFromSelectedTriggers( key, text );
    ::SetFocus( g_qeglobals.d_hwndCamera );
}

// 0x455b20  ScriptGroupDlg_Disassociate — the dialog's "Disassociate" button (identical to
// CMainFrame::OnScriptGroup_Disassociate).
static void ScriptGroupDlg_Disassociate()
{
    ScriptGroup_SyncGroupKeyToTeam();
    if ( !ScriptGroup_SelectionHasTrigger() )
    {
        Sys_Printf( "You must select a trigger_multiple or trigger_radius in combination with the nodes, AI, or goal volumes you with to disassociate.\n" );
        return;
    }
    ScriptGroup_TriggerNumber();
    UpdateSelection( 0xFFFFFFFF, 0 );
    g_nUpdateBits = -1;
}

// ── team-radio click helper (cases 8/0xB/0x684/0x686) ────────────────────────
// Sets ScriptColorTeamKey to axis/allies, re-syncs the group key, updates the two team
// radio buttons, bumps the update bits, and saves prefs.  (Binary: get_m_strStatus +
// sub_454480 + CheckDlgButton(allies/axis) + g_nUpdateBits + SavePrefs.)
static void ScriptGroupDlg_SetTeam( HWND hDlg, const char *teamKey )
{
    g_PrefsDlg->ScriptColorTeamKey = teamKey;
    ScriptGroup_SyncGroupKeyToTeam();
    bool allies = !strcmp( teamKey, "script_color_allies" );
    ::CheckDlgButton( hDlg, 1668, allies ? BST_CHECKED : BST_UNCHECKED );   // allies
    ::CheckDlgButton( hDlg, 1670, allies ? BST_UNCHECKED : BST_CHECKED );   // axis
    g_nUpdateBits = -1;
    Prefs_SavePrefs( g_PrefsDlg );
}

// ── colour-radio / team-radio click helper used by the dialog proc ────────────
// Sets ScriptColorKey to the chosen 1-char code and updates the 7 radio buttons.
static void ScriptGroupDlg_SetColorCode( HWND hDlg, const char *code )
{
    g_PrefsDlg->ScriptColorKey = code;
    static const struct { const char *c; int id; } radios[7] =
        { {"r",1661}, {"b",1662}, {"y",1663}, {"c",1664}, {"g",1665}, {"p",1666}, {"o",1667} };
    for ( int i = 0; i < 7; ++i )
        ::CheckDlgButton( hDlg, radios[i].id, !strcmp( code, radios[i].c ) ? BST_CHECKED : BST_UNCHECKED );
    ScriptGroup_SyncGroupKeyToTeam();
    g_nUpdateBits = -1;
    Prefs_SavePrefs( g_PrefsDlg );
}

// ── turret-share button handlers ──────────────────────────────────────────────
// 0x455b60  ScriptGroupDlg_TurretShare — the dialog's "Share" turret-key button.
// Sets the group key to "token" + the token key to "script_turret_share" (mirrored into
// the sub-key edit 0x65F), then for EACH selected misc_turret entity whose "export" key
// has atol>0, writes that export number into the sub-value edit 0x663 and calls
// ScriptGroupDlg_AddSubKey (which adds the token+export to every selected entity).
static void ScriptGroupDlg_TurretShare()   // 0x455b60
{
    CString exportStr;   // IDB local CString (str_set scratch)

    g_PrefsDlg->ScriptGroupKey      = "token";                  // str_set(&ScriptGroupKey,"token",5)
    g_PrefsDlg->ScriptGroupTokenKey = "script_turret_share";    // str_set(&ScriptGroupTokenKey,...)
    ::SetWindowTextA( GetDlgItem( g_qeglobals.d_hwndMedia, 0x65F ),
                      (const char *)g_PrefsDlg->ScriptGroupTokenKey );

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s_def *def = (entity_s_def *)b->owner->def;

        // classname == "misc_turret"?
        const char *classname = zero;
        for ( epair_t *ep = def->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, "classname" ) ) { classname = ep->value; break; }
        }
        if ( strcmp( classname, "misc_turret" ) )
            continue;

        // its "export" key value.
        const char *value = zero;
        for ( epair_t *ep = def->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, "export" ) ) { value = ep->value ? ep->value : zero; break; }
        }
        exportStr = value;
        if ( atol( (const char *)exportStr ) > 0 )
        {
            ::SetWindowTextA( GetDlgItem( g_qeglobals.d_hwndMedia, 0x663 ),
                              (const char *)exportStr );
            ScriptGroupDlg_AddSubKey( g_qeglobals.d_hwndMedia );
        }
    }

    sub_47D060( (int)(intptr_t)&active_brushes );
    sub_47D060( (int)(intptr_t)&selected_brushes );
    sub_47D060( (int)(intptr_t)&filtered_brushes );
    g_nUpdateBits = -1;
}

// 0x455d80  ScriptGroupDlg_TurretKey — the dialog's "ambush"/"share" turret-key rows.
// Sets the group key to "token" + the token key to `turretKey` (mirrored into the sub-key
// edit 0x65F).  If the FIRST selected entity's "export" key has atol>0 (the turret being
// shared), it gathers the export numbers of every OTHER selected misc_turret (up to
// MAX_COLORENTREES=32, packed into a [32][16] table), re-links the first brush to the FRONT
// of the selection (Select_Deselect(1) + Brush_RemoveFromList/AddToList2), then for each
// gathered export writes it into the sub-value edit 0x663 and calls ScriptGroupDlg_AddSubKey.
static void ScriptGroupDlg_TurretKey( const char *turretKey )   // 0x455d80
{
    CString exportStr;   // IDB local CString (str_set scratch)
    char    String[MAX_COLORENTREES][16];   // IDB String[512] = export-token table, stride 16
    int     exports = 0;

    g_PrefsDlg->ScriptGroupKey      = "token";                                  // str_set(...,"token",5)
    g_PrefsDlg->ScriptGroupTokenKey = turretKey ? turretKey : (const char *)0;  // str_set(&...,Src,strlen)
    ::SetWindowTextA( GetDlgItem( g_qeglobals.d_hwndMedia, 0x65F ),
                      (const char *)g_PrefsDlg->ScriptGroupTokenKey );

    selbrush_t *first = selected_brushes.next;
    if ( first == &selected_brushes )
        return;

    // the FIRST selected entity's "export" value.
    entity_s_def *firstDef = (entity_s_def *)first->owner->def;
    const char *value = zero;
    for ( epair_t *ep = firstDef->epairs; ep; ep = ep->next )
    {
        if ( !_stricmp( ep->key, "export" ) ) { value = ep->value; break; }
    }
    if ( atol( value ) <= 0 )
        return;

    // gather every OTHER selected misc_turret's export number into the table.
    for ( selbrush_t *b = first->next; b != &selected_brushes; b = b->next )
    {
        entity_s_def *def = (entity_s_def *)b->owner->def;

        const char *classname = zero;
        for ( epair_t *ep = def->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, "classname" ) ) { classname = ep->value; break; }
        }
        if ( strcmp( classname, "misc_turret" ) )
            continue;

        const char *exp = zero;
        for ( epair_t *ep = def->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, "export" ) ) { exp = ep->value ? ep->value : zero; break; }
        }
        exportStr = exp;
        if ( atol( (const char *)exportStr ) > 0 )
        {
            strcpy( String[exports], (const char *)exportStr );
            if ( ++exports >= MAX_COLORENTREES )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\ScriptGroup.cpp",
                        1818, 0, "%s", "exports < MAX_COLORENTREES" );
        }
    }

    // re-link the shared turret to the front of the selection, then propagate.
    Select_Deselect( 1 );
    Brush_RemoveFromList( first );
    Brush_AddToList2( first );
    for ( int i = 0; i < exports; ++i )
    {
        ::SetWindowTextA( GetDlgItem( g_qeglobals.d_hwndMedia, 0x663 ), String[i] );
        ScriptGroupDlg_AddSubKey( g_qeglobals.d_hwndMedia );
    }

    sub_47D060( (int)(intptr_t)&active_brushes );
    sub_47D060( (int)(intptr_t)&selected_brushes );
    sub_47D060( (int)(intptr_t)&filtered_brushes );
    g_nUpdateBits = -1;
}

// 0x455100  ScriptGroupDlgProc — the Script-Group dialog procedure.  WM_INITDIALOG seeds
// the edit controls + colour/team radios from prefs; WM_COMMAND dispatches the buttons.
// Control-command IDs are the binary's raw immediates (decoded from disasm).
INT_PTR CALLBACK ScriptGroupDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    if ( message == WM_INITDIALOG )
    {
        g_qeglobals.d_hwndMedia = hDlg;
        HWND hGroupEdit = GetDlgItem( hDlg, 1441 );
        ::SetDlgItemTextA( hDlg, 1441, (const char *)g_PrefsDlg->ScriptGroupKey );
        ::SetDlgItemTextA( hDlg, 1631, (const char *)g_PrefsDlg->ScriptSubKey_key );
        ::SetDlgItemTextA( hDlg, 1635, (const char *)g_PrefsDlg->ScriptSubValue_key );
        ::SetFocus( hGroupEdit );
        ::ShowWindow( hDlg, SW_SHOW );

        const char *code = (const char *)g_PrefsDlg->ScriptColorKey;
        if ( !strcmp( code, "r" ) ) ::CheckDlgButton( hDlg, 1661, BST_CHECKED );
        if ( !strcmp( code, "b" ) ) ::CheckDlgButton( hDlg, 1662, BST_CHECKED );
        if ( !strcmp( code, "y" ) ) ::CheckDlgButton( hDlg, 1663, BST_CHECKED );
        if ( !strcmp( code, "c" ) ) ::CheckDlgButton( hDlg, 1664, BST_CHECKED );
        if ( !strcmp( code, "g" ) ) ::CheckDlgButton( hDlg, 1665, BST_CHECKED );
        if ( !strcmp( code, "p" ) ) ::CheckDlgButton( hDlg, 1666, BST_CHECKED );
        if ( !strcmp( code, "o" ) ) ::CheckDlgButton( hDlg, 1667, BST_CHECKED );
        const char *team = (const char *)g_PrefsDlg->ScriptColorTeamKey;
        if ( !strcmp( team, "script_color_axis" ) )   ::CheckDlgButton( hDlg, 1670, BST_CHECKED );
        if ( !strcmp( team, "script_color_allies" ) ) ::CheckDlgButton( hDlg, 1668, BST_CHECKED );
        return TRUE;
    }
    if ( message != WM_COMMAND )
        return FALSE;

    WORD id = LOWORD( wParam );
    switch ( id )
    {
        case 1:    ScriptGroupDlg_OnOK( hDlg );                          return TRUE;   // OK
        case 2:    ::EndDialog( hDlg, 0 );                              return TRUE;   // Cancel
        case 4:    ScriptGroupDlg_AddSubKey( hDlg );                     return TRUE;
        case 5:    ScriptGroupDlg_AddFlag( "script_flag_true", hDlg, 1636 );
                   ScriptGroup_HasFlag( "script_flag_true", 1671, hDlg ); return TRUE;
        case 6:    ScriptGroupDlg_RemoveSubKey( hDlg );                  return TRUE;
        case 7:    ScriptGroupDlg_RemoveFlag( "script_flag_true", hDlg, 1636 );
                   ScriptGroup_HasFlag( "script_flag_true", 1671, hDlg ); return TRUE;
        case 8:    ScriptGroupDlg_SetTeam( hDlg, "script_color_axis" );  return TRUE;   // AXIS radio
        case 9:    ScriptGroup_SyncGroupKeyToTeam();
                   ScriptGroup_AddColorToSelection();                    return TRUE;   // Add colour
        case 0xA:  ScriptGroupDlg_Disassociate();                       return TRUE;
        case 0xB:  ScriptGroupDlg_SetTeam( hDlg, "script_color_allies" );return TRUE;   // ALLIES radio
        case 0xE:  ScriptGroupDlg_AddFlag( "script_flag_false", hDlg, 1639 );
                   ScriptGroup_HasFlag( "script_flag_false", 1298, hDlg );return TRUE;
        case 0xF:  ScriptGroupDlg_RemoveFlag( "script_flag_false", hDlg, 1639 );
                   ScriptGroup_HasFlag( "script_flag_false", 1298, hDlg );return TRUE;
        case 1442: // "script_health" group-key shortcut row → AssignNextNumber + close.
            g_PrefsDlg->ScriptGroupKey = "script_health";
            sub_47D060( (int)(intptr_t)&active_brushes );
            sub_47D060( (int)(intptr_t)&selected_brushes );
            sub_47D060( (int)(intptr_t)&filtered_brushes );
            Prefs_SavePrefs( g_PrefsDlg );
            ScriptGroup_AssignNextNumber();
            g_nUpdateBits = -1;
            UpdateSelection( 0xFFFFFFFF, 0 );
            ::EndDialog( hDlg, 1 );
            return TRUE;
        // colour-radio rows (two command IDs each: the button + its accelerator twin).
        case 0x674: case 0x67D: ScriptGroupDlg_SetColorCode( hDlg, "r" ); return TRUE;
        case 0x675: case 0x67E: ScriptGroupDlg_SetColorCode( hDlg, "b" ); return TRUE;
        case 0x676: case 0x67F: ScriptGroupDlg_SetColorCode( hDlg, "y" ); return TRUE;
        case 0x678: case 0x680: ScriptGroupDlg_SetColorCode( hDlg, "c" ); return TRUE;
        case 0x677: case 0x681: ScriptGroupDlg_SetColorCode( hDlg, "g" ); return TRUE;
        case 0x679: case 0x682: ScriptGroupDlg_SetColorCode( hDlg, "p" ); return TRUE;
        case 0x67A: case 0x683: ScriptGroupDlg_SetColorCode( hDlg, "o" ); return TRUE;
        case 0x684: ScriptGroupDlg_SetTeam( hDlg, "script_color_allies" ); return TRUE;
        case 0x686: ScriptGroupDlg_SetTeam( hDlg, "script_color_axis" );   return TRUE;
        case 0x5A7: // "script_killspawner" group-key row.
            g_PrefsDlg->ScriptGroupKey = "script_killspawner";
            sub_47D060( (int)(intptr_t)&active_brushes );
            sub_47D060( (int)(intptr_t)&selected_brushes );
            sub_47D060( (int)(intptr_t)&filtered_brushes );
            Prefs_SavePrefs( g_PrefsDlg );
            ScriptGroup_AssignNextNumber();
            g_nUpdateBits = -1;
            UpdateSelection( 0xFFFFFFFF, 0 );
            ::EndDialog( hDlg, 1 );
            return TRUE;
        case 0x5A9: ScriptGroupDlg_TurretKey( "script_turret_share" );  return TRUE;
        case 0x5AA: ScriptGroupDlg_TurretShare();                       return TRUE;
        case 0x5AB: ScriptGroupDlg_TurretKey( "script_turret_ambush" ); return TRUE;
        case 0x669: // "script_objective_active" sub-key row.
            g_PrefsDlg->ScriptSubKey_key = "script_objective_active";
            Prefs_SavePrefs( g_PrefsDlg );
            ::SetDlgItemTextA( hDlg, 1631, (const char *)g_PrefsDlg->ScriptSubKey_key );
            ::SetFocus( g_qeglobals.d_hwndCamera );
            return TRUE;
        case 0x66A: // "script_objective_inactive" sub-key row.
            g_PrefsDlg->ScriptSubKey_key = "script_objective_inactive";
            Prefs_SavePrefs( g_PrefsDlg );
            ::SetDlgItemTextA( hDlg, 1631, (const char *)g_PrefsDlg->ScriptSubKey_key );
            ::SetFocus( g_qeglobals.d_hwndCamera );
            return TRUE;
        default:
            return FALSE;
    }
}

// 0x455a80  AssociateEntities — toggle entry for the Script-Group dialog (the CMainFrame
// "Associate Entities" menu).  Populates the two flag listboxes, then creates the dialog
// (if not already created) or toggles its visibility.
void AssociateEntities()
{
    ScriptGroup_HasFlag( "script_flag_true",  1671, g_qeglobals.d_hwndMedia );
    ScriptGroup_HasFlag( "script_flag_false", 1298, g_qeglobals.d_hwndMedia );
    if ( !g_qeglobals.d_hwndMedia )
    {
        ::CreateDialogParamA( g_qeglobals.d_hInstance, MAKEINTRESOURCEA( IDD_SCRIPT_GROUP_NAME ),
                              g_qeglobals.d_hwndMain, (DLGPROC)ScriptGroupDlgProc, 0 );
        return;
    }
    if ( ::IsWindowVisible( g_qeglobals.d_hwndMedia ) )
        ::ShowWindow( g_qeglobals.d_hwndMedia, SW_HIDE );
    else
        ::ShowWindow( g_qeglobals.d_hwndMedia, SW_SHOW );
}

// ═════════════════════════════════════════════════════════════════════════════
//  SCRIPT-GROUP single-char TEAM-COLOUR VISUALIZATION (the 2D-view marker)
//  sub_46B110 (0x46B110) + its two leaves CamTokens_EntityGate (0x46AAE0) /
//  CamTokens_BrushMatchesToken (0x46AA80).  Ported VERBATIM from the disasm
// This is the XYWnd analog of camwnd.cpp's Cam_DrawTokens
//  (CamWnd_Tokens 0x4076C0) — same token-parse + per-entity billboard machinery,
//  but driven from CXYWnd::XY_Draw with the view-rect AABB cull + the two view
//  axes the 2D view supplies.  Wired into XY_DrawBrushes (xywnd.cpp) at the
//  binary's call position (XY_Draw 0x46D867, gated on a selected trigger's
//  non-empty ScriptColorTeamKey value).
// ═════════════════════════════════════════════════════════════════════════════
extern char  FilterBrush( selbrush_t *b, int updateFilters );        // filters.cpp 0x46A1F0
extern void  Ed_DrawScriptColorQuad( int entDef, const float *color );// brush.cpp  0x46AE10

// 0x4560F0  PrefsDlg_ScriptTeamColorEnabled (sub_4560F0) — true when the team-colour
// visualization is active: ScriptGroupKey != "token" AND ScriptGroupKey == the
// ScriptColorTeamKey.  Disasm-faithful (the && of strcmp!=0 and strcmp==0).  XY_Draw
// (xywnd.cpp) gates the per-trigger team-colour capture on this; the camera path inlines
// the same test.
bool PrefsDlg_ScriptTeamColorEnabled()
{
    return strcmp( (const char *)g_PrefsDlg->ScriptGroupKey, "token" ) != 0
        && strcmp( (const char *)g_PrefsDlg->ScriptGroupKey,
                   (const char *)g_PrefsDlg->ScriptColorTeamKey ) == 0;
}

// flt_73B098 (0x73B098) — the 7 script-colour token colours (r/b/y/c/g/p/o),
// indexed by ScriptGroup_Unreachable.  Same table as brush.cpp/camwnd.cpp.
static const float kScriptColorVizTable[7][4] = {
    { 1.0f, 0.0f, 0.0f, 1.0f },   // r — red
    { 0.0f, 0.0f, 1.0f, 1.0f },   // b — blue
    { 1.0f, 1.0f, 0.0f, 1.0f },   // y — yellow
    { 0.0f, 1.0f, 1.0f, 1.0f },   // c — cyan
    { 0.0f, 1.0f, 0.0f, 1.0f },   // g — green
    { 1.0f, 0.0f, 1.0f, 1.0f },   // p — purple
    { 1.0f, 0.4f, 0.0f, 1.0f },   // o — orange
};

// 0x46AA80  CamTokens_BrushMatchesToken (ecx=brush, arg0=token) — true if the brush
// entity's ScriptColorTeamKey value strstr-contains `token`.  Disasm-faithful: walks
// b->owner->def->epairs (selbrush+8 → entity+8 → epairs@0x74) for the team key
// (g_PrefsDlg+0x32C), defaulting to `zero`="" when absent.
static bool CamTokens_BrushMatchesToken( selbrush_t *b, const char *token )
{
    const char *teamKey = (const char *)g_PrefsDlg->ScriptColorTeamKey;
    entity_s_def *def = (entity_s_def *)b->owner->def;        // [ecx+8]→[+8]
    const char *value = zero;                                 // IDB `zero` = ""
    for ( epair_t *ep = def->epairs; ep; ep = ep->next )      // [+0x74]
    {
        if ( !_stricmp( ep->key, teamKey ) ) { value = ep->value; break; }
    }
    return strstr( value, token ) != nullptr;
}

// 0x46AAE0  CamTokens_EntityGate (eax=brush) — true if the brush entity HAS the
// ScriptColorTeamKey AND its classname contains actor / node / info_volume.
// Disasm-faithful: v1 = b->owner->def; HasKeyValuePair(v1, teamKey) gate, then the
// classname epair-walk (default `zero`).
static bool CamTokens_EntityGate( selbrush_t *b )
{
    entity_s_def *def = (entity_s_def *)b->owner->def;        // *(int**)(*(int*)(a1+8)+8)
    if ( !HasKeyValuePair( def, (const char *)g_PrefsDlg->ScriptColorTeamKey ) )
        return false;
    const char *value = zero;
    for ( epair_t *ep = def->epairs; ep; ep = ep->next )
    {
        if ( !_stricmp( ep->key, "classname" ) ) { value = ep->value; break; }
    }
    return strstr( value, "actor" ) || strstr( value, "node" )
        || strstr( value, "info_volume" ) != nullptr;
}

// 0x46B110  ScriptGroup_DrawTeamColorViz (sub_46B110) — the script-group single-char
// team-colour visualization.  a1 = the team-colour token string (a selected trigger's
// ScriptColorTeamKey value); a2/a3 = the view-rect mins/maxs on the two view axes
// (XY_Draw's &v47 / &tdp); a4/a5 = the two world axis indices for the view plane (v58/v60).
//
// Copy a1 into a 1024 buffer; tokenize by " "; for each token
// store "<tok> " into a fixed 16-byte-stride array (v42) and its colour into a parallel
// 4-float-stride array (v34, token n → v34[4n+4] = kScriptColorVizTable[Unreachable(tok)]).
// Then if there is more than one token, find the first (among the first tokens-1) whose
// text contains ScriptColorKey and SWAP it (string + colour) with the LAST token — the 3
// overlapping strcpy/qmemcpy loops in the binary are exactly a token[M] ⟷ token[tokens-1]
// exchange via a 16-byte scratch (v43).  Finally, for every active then selected entity in
// view that passes !FilterBrush + CamTokens_EntityGate, draw a colour billboard
// (Ed_DrawScriptColorQuad) for each token the entity matches (CamTokens_BrushMatchesToken).
//
// MAX_COLORENTREES == 32 (the binary's `tokens < MAX_COLORENTREES` cap at XYWnd.cpp:3129).
void ScriptGroup_DrawTeamColorViz( const char *a1, const float *viewMins,
                                   const float *viewMaxs, int axis0, int axis1 )
{
    char buf[1024];                          // v41 — strtok working copy of a1
    strcpy( buf, a1 );
    if ( !buf[0] )
        return;

    // token strings (16-byte stride, each "<tok> ") + parallel colours (token n at [4n+4]).
    char  tokStr[MAX_COLORENTREES][16];      // v42 (516 bytes; entry n = &v42[16*n])
    float tokCol[(MAX_COLORENTREES + 1) * 4];// v34 (132 floats; token n colour at [4n+4])
    int   tokens = 0;                        // i

    for ( char *t = strtok( buf, " " ); t; t = strtok( nullptr, " " ) )
    {
        iassert( t[0] );                     // XYWnd.cpp:3122 "token[0]"
        strcpy( tokStr[tokens], t );
        strcat( tokStr[tokens], " " );       // trailing space (binary appends asc_6D56FC)
        int ci = ScriptGroup_Unreachable( t );
        // binary indexes flt_73B098[4*ci] unconditionally; ci is -1 only on a no-match
        // (which would assert + read flt_73B098[-4]) — guard the never-valid case, matching
        // the established camwnd/brush.cpp convention.
        const float *c = ( ci >= 0 && ci <= 6 ) ? kScriptColorVizTable[ci]
                                                : kScriptColorVizTable[0];
        const int colSlot = 4 * tokens + 4;  // v34[4n+4]
        tokCol[colSlot + 0] = c[0];
        tokCol[colSlot + 1] = c[1];
        tokCol[colSlot + 2] = c[2];
        tokCol[colSlot + 3] = c[3];
        ++tokens;
        if ( tokens >= MAX_COLORENTREES )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\XYWnd.cpp",
                    3129, 0, "%s", "tokens < MAX_COLORENTREES" );
    }

    // SWAP the first ScriptColorKey-matching token (among the first tokens-1) with the
    // LAST token (binary's 3 overlapping-copy loops, 0x46B268..0x46B382).
    const char *colorKey = (const char *)g_PrefsDlg->ScriptColorKey;  // *(char**)(+0x330)
    const int last = tokens - 1;
    for ( int m = 0; m < last; ++m )
    {
        if ( strstr( tokStr[m], colorKey ) )
        {
            // save LAST token colour, copy MATCHED→LAST, restore saved→MATCHED (string too).
            float saveCol[4];
            saveCol[0] = tokCol[4 * last + 4 + 0];
            saveCol[1] = tokCol[4 * last + 4 + 1];
            saveCol[2] = tokCol[4 * last + 4 + 2];
            saveCol[3] = tokCol[4 * last + 4 + 3];
            char saveStr[16];
            strcpy( saveStr, tokStr[last] );

            tokCol[4 * last + 4 + 0] = tokCol[4 * m + 4 + 0];
            tokCol[4 * last + 4 + 1] = tokCol[4 * m + 4 + 1];
            tokCol[4 * last + 4 + 2] = tokCol[4 * m + 4 + 2];
            tokCol[4 * last + 4 + 3] = tokCol[4 * m + 4 + 3];
            strcpy( tokStr[last], tokStr[m] );

            tokCol[4 * m + 4 + 0] = saveCol[0];
            tokCol[4 * m + 4 + 1] = saveCol[1];
            tokCol[4 * m + 4 + 2] = saveCol[2];
            tokCol[4 * m + 4 + 3] = saveCol[3];
            strcpy( tokStr[m], saveStr );
            break;
        }
    }

    if ( tokens <= 0 )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\XYWnd.cpp",
                3150, 0, "%s", "tokens > 0" );

    // Draw a colour billboard per matching token on every in-view active then selected
    // gated entity.  Cull: viewMaxs[0]>=def->mins[axis0] && viewMaxs[1]>=def->mins[axis1]
    //                    && viewMins[0]<=def->maxs[axis0] && viewMins[1]<=def->maxs[axis1].
    for ( int pass = 0; pass < 2; ++pass )
    {
        selbrush_t *head = pass ? &selected_brushes : &active_brushes;
        for ( selbrush_t *b = head->next; b != head; b = b->next )
        {
            brush_t *def = b->def;
            if ( !( viewMaxs[0] >= def->mins[axis0] && viewMaxs[1] >= def->mins[axis1]
                 && viewMins[0] <= def->maxs[axis0] && viewMins[1] <= def->maxs[axis1] ) )
                continue;
            if ( FilterBrush( b, 0 ) )
                continue;
            if ( !CamTokens_EntityGate( b ) )
                continue;
            for ( int n = 0; n < tokens; ++n )
            {
                if ( CamTokens_BrushMatchesToken( b, tokStr[n] ) )
                    Ed_DrawScriptColorQuad( (int)(intptr_t)b->owner->def, &tokCol[4 * n + 4] );
            }
        }
    }
}

