#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Layer map storage, visibility flags, and .map serialization.
// KISAK: a standard std::map reproduces the binary map's sorted semantics.

#include "stdafx.h"
#include "qe3.h"
#include <map>
#include <set>
#include <vector>
#include <tuple>
#include <string>
#include <utility>
#include <cstdio>
#include <universal/q_parse.h>   // Com_Parse, Com_ParseOnLine, Com_UngetToken

extern int   Sys_Printf( const char *fmt, ... );
extern char  g_activeLayer_string[];   // engine_stubs.cpp (256 bytes)
extern qeglobals_t g_qeglobals;        // qe3.cpp
extern void  Assert( const char *file, int line, int type, const char *fmt, ... ); // 0x49cea0
extern "C" int _stricmp( const char *, const char * );

// brush display-list sentinels (qe3 / engine_stubs) — the re-layer + script_layer
// walks iterate these exactly like Select_BrushByLayer does.
extern selbrush_t active_brushes;      // 0x23F189C
extern selbrush_t selected_brushes;    // 0x23F1864
extern selbrush_t filtered_brushes;    // 0x23F182C

// ─── layerMap: layer name → flag bits ────────────────────────────────────────
// std::map is sorted by key (operator<), which reproduces the binary's RB-tree
// iteration order and therefore the on-disk layer-line order.
static std::map<std::string, int> g_layerMap;

// Layer flag bits (sub_418C10 / sub_418C80):
//   hidden=1  prefab=2  expanded=4  frozen=8 ; 0x10 = internal prefab-dirty (not persisted)
static int LayerFlagFromName( const char *name )   // sub_418C10 (0x418C10)
{
    if ( !strcmp( name, "hidden"   ) ) return 1;
    if ( !strcmp( name, "prefab"   ) ) return 2;
    if ( !strcmp( name, "expanded" ) ) return 4;
    if ( !strcmp( name, "frozen"   ) ) return 8;
    return 0;
}

static void LayerFlagsToString( char *out, int flags )   // sub_418C80 (0x418C80)
{
    out[0] = '\0';
    if ( flags & 1 ) { strcat( out, "hidden"   ); strcat( out, " " ); }
    if ( flags & 2 ) { strcat( out, "prefab"   ); strcat( out, " " ); }
    if ( flags & 4 ) { strcat( out, "expanded" ); strcat( out, " " ); }
    if ( flags & 8 ) { strcat( out, "frozen"   ); strcat( out, " " ); }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x419630  Layers_AddLayerPath  (sub_419630) — add or OR-in a layer's flags.
// The original splits hierarchical "a/b/c" names on '/' and inserts each prefix
// (clearing 0x10 on parents). Gate-P4 stock maps use flat layer names (no '/'),
// so the flat OR-insert below is faithful for them; '/'-paths fall back to a
// single insert of the full name (documented limitation — extend if a map needs it).
void Layers_AddLayerPath( const char *name, int flags )
{
    if ( !name ) return;
    g_layerMap[ std::string( name ) ] |= ( flags & ~0x10 );   // 0x10 never persisted here
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x4190F0  layers_01 — OR a flag onto an existing layer.  The faithful port is
// Layers_SetFlag (further below).  A prior DUPLICATE of layers_01 (Layers_SetPrefabFlag)
// lived here but DROPPED the binary's line-203 "mapIter != layerMap.end()" assert; it
// was removed and its one caller (Map_ParseLayers, the `flags & 2` prefab-dirty case)
// redirected to Layers_SetFlag.  The binary has a SINGLE layers_01 (0x4190F0) called
// from BOTH the parse and dialog paths — now matched.
// ─────────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────
// 0x4187A0  Map_InitlLayers — reset active layer + clear the layer map.
// (The original also pokes a layers-dialog window via SendMessageA; UI-only.)
// ─────────────────────────────────────────────────────────────────────────────
void Map_InitlLayers()
{
    strcpy( g_activeLayer_string, "000_Global" );
    g_layerMap.clear();
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x418DD0  Layers_SetMapLayers — ensure the two built-in layers exist.
// The original additionally re-tags every brush with its layer and realizes any
// prefab layers (sub_41A5A0/sub_41A7F0) — that touches instance/window state and
// is NOT needed to round-trip the .map text, so it is omitted here.
// std::map::operator[] inserts a flags=0 entry only if the key is absent (so a
// layer already parsed from the file keeps its flags).
// ─────────────────────────────────────────────────────────────────────────────
void Layers_SetMapLayers()
{
    g_layerMap[ "The Map"    ];   // insert with 0 if absent
    g_layerMap[ "000_Global" ];
}

// ─────────────────────────────────────────────────────────────────────────────
// Prefab-edit-in-place layer-state stack (#18 ENTER/LEAVE).
//
// The binary stores the whole layer std::map node (the RB-tree rooted at
// unk_25D5D48) into each prefab-stack slot at byte offset +2088, via
//   ENTER:  sub_489810 → sub_41A5A0 (clear dest sentinels) + sub_489F30 (deep copy)
//   LEAVE:  sub_41A5A0 (clear live tree) + sub_489F30 (copy slot → live tree)
// i.e. a deep COPY of the layer map onto / off of the stack slot.  This port
// models layerMap as a real std::map (g_layerMap, see top of file) and owns both
// the reader and writer, so the binary's RB-tree clone/erase collapses to ordinary
// std::map value semantics — save = copy g_layerMap into a per-level slot, restore =
// copy it back.  This reproduces the binary's behaviour without reversing the
// MSVC RB-node layout (same architectural decision as the Gate-P4 layer subsystem).
//
// g_activeLayer_string is the layer *name* (saved/restored in the prefab slot at
// +1064 by map.cpp directly), so only the map<name,flags> needs stacking here.
// Indexed by the post-push / pre-pop prefab level (max 16, matching the binary's
// byte_25EB240[16] stack).
// ─────────────────────────────────────────────────────────────────────────────
static std::map<std::string, int> g_prefabLayerStack[16];

void Layers_SavePrefabLayers( int level )      // ENTER: snapshot before swapping to sub-map
{
    if ( level < 0 || level >= 16 )
        return;
    g_prefabLayerStack[ level ] = g_layerMap;
}

void Layers_RestorePrefabLayers( int level )   // LEAVE: restore parent map's layers
{
    if ( level < 0 || level >= 16 )
        return;
    g_layerMap = g_prefabLayerStack[ level ];
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x41A0E0  Layers_02 — refresh the layers dialog (wipe tree → "The Map" root →
// re-insert every layerMap entry → bold/icon the active item via sub_41C9C0).
// Port form: the flat-listbox CLayerDlg::Refresh() performs the whole sequence;
// no-op when the dialog was never opened (matches the binary sending tree messages
// to a NULL hwnd harmlessly).  No window in the selftest.
// ─────────────────────────────────────────────────────────────────────────────
extern void LayersDlg_RefreshIfOpen();          // layersdlg.cpp
void Layers_02()
{
    LayersDlg_RefreshIfOpen();
}

// 0x41C9C0  sub_41C9C0(name) — un-bold/un-icon the previous active tree item, find
// `name`'s item (LayersDlg_Children) and bold it + icon 2.  Port form: the active
// marker is rendered by Refresh() from g_activeLayer_string, so the highlight update
// IS a refresh.  (Caller: map.cpp:1947 after the map-load layer parse; Layers_02 tail.)
void sub_41C9C0( const char * /*name*/ )
{
    LayersDlg_RefreshIfOpen();
}

// ─────────────────────────────────────────────────────────────────────────────
// Layer block writer — extracted from Map_SaveFile (0x486C00).
// For each layer (sorted), unless flags == 2 (a transient prefab-only layer),
// emit:  "<name>" flags <flagstr> [ active]\n
// The flagstr keeps its trailing space exactly as the original (` %s` with the
// sub_418C80 output); the round-trip diff is whitespace-insensitive regardless.
// ─────────────────────────────────────────────────────────────────────────────
void Layers_WriteToFile( FILE *f )
{
    for ( const auto &kv : g_layerMap )
    {
        int flags = kv.second & ~0x10;   // 0x10 is internal; matches sub_418C80 ignoring it
        if ( kv.second == 2 )            // transient prefab-only layer — skip (original: v12[10] != 2)
            continue;

        char flagstr[64];
        fprintf( f, "\"%s\" flags", kv.first.c_str() );
        LayerFlagsToString( flagstr, flags );
        fprintf( f, " %s", flagstr );
        if ( !strcmp( g_activeLayer_string, kv.first.c_str() ) )
            fprintf( f, " active" );
        fprintf( f, "\n" );
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  Map-file header parsers (live in map.cpp in the original; grouped here next to
//  the layer logic they feed). All take the live text pointer and drive the
//  shared q_parse session (Com_Parse advances both *text and g_parse state).
// ═════════════════════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────────────────────
// 0x4861F0  Map_ReadVersion (sub_4861F0) — read the leading `iwmap <n>`.
// Returns the version number, 0 if no version token, -1 on empty file.
// (sub_4861F0's unget prologue + Com_ParseExt(text,1) == Com_Parse; _GetToken ==
//  Com_ParseOnLine.)
// ─────────────────────────────────────────────────────────────────────────────
int Map_ReadVersion( const void **text )
{
    parseInfo_t *tok = Com_Parse( (const char **)text );
    if ( !tok->token[0] )
    {
        Sys_Printf( "WARNING: empty map\n" );
        return -1;
    }
    if ( !strcmp( tok->token, "iwmap" ) )
    {
        parseInfo_t *ver = Com_ParseOnLine( (const char **)text );
        return atoi( ver->token );
    }
    Com_UngetToken();
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x486280  Map_ParseLayers (sub_486280) — parse the layer-declaration block.
// Each line: "<name>" flags [flagnames...] [active]   until the first '{'.
// `skipFlags` (the original's a1) suppresses flag processing (used for prefab
// re-import); the normal map-load path passes 0.
// ─────────────────────────────────────────────────────────────────────────────
void Map_ParseLayers( const void **text, int skipFlags )
{
    const char **t = (const char **)text;

    for ( ;; )
    {
        parseInfo_t *nameTok = Com_Parse( t );          // crossline: the layer name (or '{')
        if ( !*t || nameTok->token[0] == '{' )
            break;

        char layerName[1028];
        strncpy( layerName, nameTok->token, sizeof( layerName ) - 1 );
        layerName[sizeof( layerName ) - 1] = '\0';

        parseInfo_t *kw = Com_ParseOnLine( t );         // "flags"
        if ( !skipFlags && !strstr( kw->token, ";" ) && !strcmp( kw->token, "flags" ) )
        {
            int  flags  = 0;
            bool active = false;
            for ( parseInfo_t *f = Com_ParseOnLine( t ); f->token[0]; f = Com_ParseOnLine( t ) )
            {
                if ( !strcmp( f->token, "active" ) )
                    active = true;
                else
                    flags |= LayerFlagFromName( f->token );
            }

            Layers_AddLayerPath( layerName, flags );
            if ( flags & 2 )
                Layers_SetFlag( layerName, 16 );   // internal prefab-dirty bit (layers_01 0x4190F0)
            if ( active )
                strcpy( g_activeLayer_string, layerName );
        }
    }

    // Unget the '{' so ParseEntity reads the first entity's opening brace.
    Com_UngetToken();
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x42F8A0  Map_ParseEntityLayerKey (sub_42F8A0) — peek for an optional leading
// `<key> "<value>"` line inside an entity block; write the value (or `def`) to
// `out`. Used by ParseEntity to read the per-entity `layer "..."` line.
// ─────────────────────────────────────────────────────────────────────────────
void Map_ParseEntityLayerKey( const char **text, const char *def, char *out, const char *key )
{
    parseInfo_t *tok = Com_Parse( text );
    if ( !strcmp( tok->token, key ) )
    {
        parseInfo_t *val = Com_ParseOnLine( text );
        if ( out )
        {
            if ( val->token[0] )
                strcpy( out, val->token );
            else
                strcpy( out, def );
        }
    }
    else
    {
        Com_UngetToken();
        if ( out )
            strcpy( out, def );
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  LAYERS DIALOG BACKEND (layersdlg.cpp drives these — the unit's core).
//  All operate on the same g_layerMap + g_activeLayer_string the writer/parser use,
//  so every dialog op round-trips through the .map.
// ═════════════════════════════════════════════════════════════════════════════

// ── enumeration: copy the (sorted) layer names + flags out for the dialog list ──
// std::map iteration is already in operator< order == the on-disk + tree order.
void Layers_Enumerate( std::vector<std::pair<std::string,int>> &out )
{
    out.clear();
    for ( const auto &kv : g_layerMap )
        out.push_back( kv );
}
int  Layers_Count() { return (int)g_layerMap.size(); }

bool        Layers_Exists( const char *name )
{
    return g_layerMap.find( std::string( name ) ) != g_layerMap.end();
}
int         Layers_GetFlags( const char *name )
{
    auto it = g_layerMap.find( std::string( name ) );
    return ( it == g_layerMap.end() ) ? 0 : ( it->second & ~0x10 );
}
const char *Layers_GetActive() { return g_activeLayer_string; }

// ── 0x4190F0 layers_01 / 0x419200 layers_02 — generic set/clear-flag-by-name ──
// (The IDB's layers_01 ORs a2 into v4[10]; layers_02 ANDs ~a2.  v4[10] is the flag
//  int at +0x28 of the std::map node = our int value.  Both ++g_layerCount_maybe
//  when the touched flag has bit 0 or 3, faithfully reproduced.)
void Layers_SetFlag( const char *name, int flag )      // layers_01 0x4190F0
{
    auto it = g_layerMap.find( std::string( name ) );
    if ( it == g_layerMap.end() )
    {
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\layers.cpp",
                203, 0, "%s", "mapIter != layerMap.end()" );
        return;
    }
    it->second |= flag;
    if ( flag & 9 )
        ++g_qeglobals.g_layerCount_maybe;
}
void Layers_ClearFlag( const char *name, int flag )    // layers_02 0x419200
{
    auto it = g_layerMap.find( std::string( name ) );
    if ( it == g_layerMap.end() )
    {
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\layers.cpp",
                218, 0, "%s", "mapIter != layerMap.end()" );
        return;
    }
    it->second &= ~flag;
    if ( flag & 9 )
        ++g_qeglobals.g_layerCount_maybe;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x418A50  sub_418A50 — mark every layer whose NAME prefix-matches `path` with the
// internal 0x10 "model/prefab-managed" bit.
//
// Faithful port of the IDB (disasm 0x418A50): the function does a forward walk of the
// whole layerMap (RB-tree rooted at unk_25D5D48, iterated via FaceTexMap_IterNext) and
// for each node compares the layer NAME against `path` with strncmp(name, path, strlen(path))
// — i.e. a PREFIX match of `path` against the start of the layer name — and on a match
// ORs bit 0x10 into the node's flag int ([edi+0x28] |= 0x10, == our map value).  It does
// NOT touch g_layerCount_maybe (0x10 & 9 == 0) and it does NOT assert-on-miss; it simply
// visits every layer.  Callers: Map_LoadFromFile (map.cpp) and the prefab PrevLevel
// restore (map.cpp), once per model entity's "model" path — they tag the layers that a
// loaded model/prefab owns (the inverse of Layers_AddLayerPath, which clears 0x10 on the
// parents it touches and re-ORs the leaf's persisted flags without 0x10).
//
// NOTE the iteration semantics: the binary matches `path` as the strncmp PREFIX argument
// — so a layer name only matches when `name` STARTS WITH `path` (the full `path` is the
// strncmp length).  An empty `path` (no "model" key → "") gives strlen==0 and strncmp==0
// for every node, so every layer is tagged — matching the IDB exactly.
// ─────────────────────────────────────────────────────────────────────────────
void sub_418A50( const char *path )
{
    size_t plen = strlen( path );
    for ( auto &kv : g_layerMap )
    {
        if ( strncmp( kv.first.c_str(), path, plen ) == 0 )
            kv.second |= 0x10;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x4188A0 sub_4188A0 / 0x418B20 sub_418B20 — layerMap exact-name lookups used
// ONLY by Brush_UpdateLayerVis (their sole binary caller).  Both construct a
// std::string from the key, find() it in layerMap, and on a MISS return 0; on
// a hit they test the flags int (the map value): bit 0 ("hidden") / bit 3
// ("frozen").
// ─────────────────────────────────────────────────────────────────────────────
static int Layers_KeyIsHidden( const char *key )     // sub_4188A0 (0x4188A0)
{
    auto it = g_layerMap.find( std::string( key ) );
    return ( it == g_layerMap.end() ) ? 0 : ( it->second & 1 );
}
static bool Layers_KeyIsFrozen( const char *key )    // sub_418B20 (0x418B20)
{
    auto it = g_layerMap.find( std::string( key ) );
    return it != g_layerMap.end() && ( it->second & 8 ) != 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x418990  Brush_UpdateLayerVis — refresh a brush INSTANCE's layer-derived
// brushFlags from the layer map: bit 0x2 (layer-hidden — DrawBrush skips the
// brush) from the layer's "hidden" flag (bit 0), and bit 0x20 (layer-frozen —
// the pick/select paths skip the brush) from the layer's "frozen" flag (bit 3).
// Lazy per-draw refresh cached against g_layerCount_maybe (the layer-change
// generation counter bumped by Layers_SetFlag/ClearFlag on flag bits 1|8).
// The lookup key is layerPrefix ++ def->parent_layer_string — layerPrefix is
// "" (the binary's `zero` global) for world brushes, and the accumulated
// "<model path lc>/" chain for brushes drawn inside placed prefabs (built by
// sub_478B10 → DrawBrush_PrefabContents).  Sole binary caller: the head of
// DrawBrush (call at 0x47aff5).
// ─────────────────────────────────────────────────────────────────────────────
void Brush_UpdateLayerVis( selbrush_t *b, const char *layerPrefix )
{
    if ( b->xx7 == g_qeglobals.g_layerCount_maybe )   // 0x4189b1
        return;

    char key[1028];                                   // v10[1028] @ 0x418990
    strcpy( key, layerPrefix );                       // inline byte-copy loop 0x4189c0
    strcat( key, b->def->parent_layer_string );       // append at the NUL 0x4189e5..0x4189f9

    if ( Layers_KeyIsHidden( key ) )                  // 0x418a08
        b->brushFlags |= 2;                           // 0x418a11
    else
        b->brushFlags &= ~2u;                         // 0x418a17
    if ( Layers_KeyIsFrozen( key ) )                  // 0x418a21
        b->brushFlags |= 0x20;                        // 0x418a2a
    else
        b->brushFlags &= ~0x20u;                      // 0x418a30

    b->xx7 = g_qeglobals.g_layerCount_maybe;          // 0x418a39
}

// ── brush re-layer worker (sub_41A010, 0x41A010) ──────────────────────────────
// Walk one DISPLAY brush list; for each instance whose def's parent_layer_string
// equals `oldName`, set it to `newName` (via Brush_SetLayerString-equivalent
// sub_4758E0).  The IDB does the string compare on def->parent_layer_string (+72)
// and resets the instance faceVis flag (xx7) — both handled by Brush_SetLayerString2.
extern void Brush_SetInstanceLayerString( selbrush_t *inst, const char *str ); // brush.cpp 0x4758E0
static void Layers_RelayerList( const char *newName, selbrush_t *sentinel, const char *oldName )
{
    for ( selbrush_t *b = sentinel->next; b != sentinel; b = b->next )
    {
        if ( b->def && b->def->parent_layer_string
             && !strcmp( b->def->parent_layer_string, oldName ) )
            Brush_SetInstanceLayerString( b, newName );
    }
}

// ── 0x419940 Layers_Update_01 — DELETE a layer (+ its sub-layers) ─────────────
// For every layer whose name has `victim` as a path prefix (next char '/' or end):
// move its brushes to the active layer (000_Global if the active layer was the
// victim), erase the node.  Faithful to the IDB's prefix walk; the std::map gives
// us erase/iteration for free.  Returns the number of layers removed.
int Layers_DeleteLayer( const char *victim )
{
    size_t vlen = strlen( victim );
    std::vector<std::string> toErase;

    for ( const auto &kv : g_layerMap )
    {
        const std::string &nm = kv.first;
        if ( nm.compare( 0, vlen, victim ) == 0 &&
             ( nm.size() == vlen || nm[vlen] == '/' ) )
            toErase.push_back( nm );
    }

    for ( const std::string &nm : toErase )
    {
        // If the active layer is being deleted, fall back to 000_Global first
        // (matches the IDB resetting g_activeLayer_string to a000_global).
        if ( !strcmp( g_activeLayer_string, nm.c_str() ) )
            strcpy( g_activeLayer_string, "000_Global" );

        // Re-tag every brush off the victim layer onto the (new) active layer.
        Layers_RelayerList( g_activeLayer_string, &active_brushes,   nm.c_str() );
        Layers_RelayerList( g_activeLayer_string, &selected_brushes, nm.c_str() );
        Layers_RelayerList( g_activeLayer_string, &filtered_brushes, nm.c_str() );

        g_layerMap.erase( nm );
    }

    Sys_Printf( "Updating layers...\n" );
    Layers_SetMapLayers();   // re-ensure The Map / 000_Global exist
    Layers_02();
    return (int)toErase.size();
}

// ── 0x419BE0 Layers_Update_02 — RENAME a layer's path prefix ──────────────────
// `combined` is "<parentPath>/<newLeaf>" (or just "<newLeaf>" at the root); the
// caller already built it from the picked layer's parent + the typed name.  The
// IDB walks layers whose name has the OLD full path (g_activeLayer_string-relative
// arg) as a prefix and rewrites the prefix to `combined`.  Here we take the old
// full name + the new full name directly (the dialog computes both).
int Layers_RenameLayer( const char *oldFull, const char *newFull )
{
    if ( !strcmp( oldFull, newFull ) || !strcmp( oldFull, "000_Global" ) )
        return 0;

    size_t olen = strlen( oldFull );
    // Collect (oldName, newName, flags) for every affected layer (prefix match).
    std::vector<std::tuple<std::string,std::string,int>> moves;
    for ( const auto &kv : g_layerMap )
    {
        const std::string &nm = kv.first;
        if ( nm.compare( 0, olen, oldFull ) == 0 &&
             ( nm.size() == olen || nm[olen] == '/' ) )
        {
            std::string nn = std::string( newFull ) + nm.substr( olen ); // keep the suffix
            moves.emplace_back( nm, nn, kv.second );
        }
    }

    for ( auto &mv : moves )
    {
        const std::string &on = std::get<0>( mv );
        const std::string &nn = std::get<1>( mv );
        int flags = std::get<2>( mv );

        Layers_RelayerList( nn.c_str(), &active_brushes,   on.c_str() );
        Layers_RelayerList( nn.c_str(), &selected_brushes, on.c_str() );
        Layers_RelayerList( nn.c_str(), &filtered_brushes, on.c_str() );

        if ( !strcmp( g_activeLayer_string, on.c_str() ) )
            strncpy( g_activeLayer_string, nn.c_str(), 255 );

        g_layerMap.erase( on );
        g_layerMap[ nn ] |= ( flags & ~0x10 );
    }

    Sys_Printf( "Updating layers...\n" );
    Layers_SetMapLayers();
    Layers_02();
    return (int)moves.size();
}

// ── set the active layer (CLayerDlg::SetActiveLayer 0x41C610 core) ────────────
void Layers_SetActive( const char *name )
{
    strncpy( g_activeLayer_string, name, 255 );
    g_activeLayer_string[255] = '\0';
}

// ── collect the unique entity script_layer values (sub_41C200 dedup core) ─────
// The visibility CCheckListBox is built from the set of `script_layer` epair
// values carried by the live brushes' owning entities (NOT the layerMap — the
// layerMap drives the TREE; the filter list drives FilterBrush).  This walks the
// active + selected display lists, gathers the unique values, and (re)builds one
// script_layer filter per value via Layers_BuildFilter (filters.cpp).
extern void        Layers_FreeVisibilityFilters();          // filters.cpp
extern const char *Layers_BuildFilter( const char *name );  // filters.cpp 0x411950

static void Layers_CollectScriptLayers( selbrush_t *sentinel, std::set<std::string> &seen )
{
    for ( selbrush_t *b = sentinel->next; b != sentinel; b = b->next )
    {
        if ( !b || !b->owner ) continue;
        entity_s *ent = (entity_s *)b->owner->def;
        if ( !ent ) continue;
        const char *val = nullptr;
        for ( epair_t *ep = ent->epairs; ep; ep = ep->next )
            if ( !_stricmp( ep->key, "script_layer" ) ) { val = ep->value; break; }
        if ( val && val[0] )
            seen.insert( std::string( val ) );
    }
}

void Layers_RebuildVisibilityFilters()   // sub_41C200 backend (no UI)
{
    Layers_FreeVisibilityFilters();

    std::set<std::string> seen;
    Layers_CollectScriptLayers( &active_brushes,   seen );
    Layers_CollectScriptLayers( &selected_brushes, seen );

    for ( const std::string &nm : seen )
        Layers_BuildFilter( nm.c_str() );
}
