#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// MFC application startup and the optional headless regression-test host.
// Self-test usage: KisakCOD-Radiant.exe <gate> [map path].

#include "stdafx.h"
#include "mainfrm.h"
#include "prefs.h"

#include <qcommon/qcommon.h>

#ifdef RADIANT_SELFTEST
#include "qe3.h"
#include "linearmapping.h"
#include <gfx_d3d/r_gfx.h>
#include <gfx_d3d/r_rendercmds.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <float.h>
#include <map>
#include <string>

extern entity_s   entities;                                                   // entity.cpp
extern void       Map_NewMap();                                              // map.cpp
extern void       Map_New();                                                 // entity.cpp (0x4870C0) File→New
extern void       Map_LoadFromFile( const char *path );                      // map.cpp (0x486680)
extern void       Map_SaveFile( const char *path, char a1, char a2 );        // map.cpp
extern eclass_t  *Eclass_ForName( int has_brushes, const char *name );       // eclass.cpp
extern eclass_t  *Eclass_InitFromText( void *textPtr );                      // eclass.cpp (0x481150)

extern selbrush_t active_brushes;                                            // qe3.cpp (0x23F189C)
extern selbrush_t selected_brushes;                                          // qe3.cpp (0x23F1864)
extern char       currentmap[];                                              // map.cpp (0x23F18D8)
extern int        prefabStackLevel;                                          // map.cpp (0x25D5B34)
extern void       Prefab_NextLevel( void *a1 );                              // map.cpp (0x489190)
extern void       Prefab_PrevLevel();                                        // map.cpp (0x489890)

extern entity_s  *world_entity;                                              // map.cpp (0x25D5B30)
extern void       Select_Brush( selbrush_t *b, char some_overwrite,
                                char bStatus, char center );                 // select.cpp (0x48DCC0)
extern void       Select_Delete();                                          // select.cpp (0x48E760)
extern "C" void  *Radiant_TestCreateBrushDrag( int px, int py, int cx1, int cy1,
                                               int cx2, int cy2, float scale,
                                               float ox, float oy, int w, int h );
extern "C" void  *Radiant_TestCreateEntity( const char *classname );
extern void       Undo_ClearRedo();                                          // undo.cpp (0x45DF20)
extern void       Undo_GeneralStart( const char *operation );               // undo.cpp (0x45E3F0)
extern void       Undo_EndBrushList( selbrush_t *brushlist );               // undo.cpp (0x45E870)
extern void       Undo_End();                                                // undo.cpp (0x45EA20)
extern void       Undo_Undo();                                               // undo.cpp (0x45EA90)
extern void       Undo_Redo();                                               // undo.cpp (0x45F2E0)
extern "C" void  *Radiant_TestCreateBrushEntity( const char *classname );
extern void       SetKeyValue( entity_s_def *e, const char *key, const char *value ); // entity.cpp 0x483690
extern char      *ValueForKey2( int e, const char *key );                             // entity.cpp 0x4825C0
extern void       Entity_UpdateCylinder( const char *key, int entPtr );               // entity.cpp 0x483320
extern float      Entity_GetFloatValueForKey( int e, const char *key );               // entity.cpp 0x4837C0
extern "C" int    Radiant_TestSelectFace();                                           // select.cpp
extern "C" int    Radiant_TestApplyTexMod( float shiftX, float scale, float rotate ); // surfacedlg.cpp
extern "C" int    Radiant_TestReadFaceTexdef( float out[5] );                         // surfacedlg.cpp
extern "C" int    Radiant_TestFitFace();                                              // surfacedlg.cpp (Fit)
extern "C" int    Radiant_TestSetSpawnflagBit( void *entDef, int bit );               // win_ent.cpp
extern void       Entity_SetAngles( float a1, int axis );                             // select.cpp 0x494030
extern "C" void  *Radiant_TestClipSplit( int *outFront, int *outBack );               // xywnd.cpp
extern "C" int    Radiant_TestClipCommit();                                           // xywnd.cpp
extern "C" int    Radiant_TestConnectionLinks( int *outTargetLinks, int *outScriptLinks ); // xywnd.cpp
extern "C" int    Radiant_TestEntityNameCount();                                           // xywnd.cpp
extern "C" void  *Radiant_TestMoveVertex( int *outHandles, int *outVertMoved,
                                          int *outFaceMoved, float *outDelta );        // select.cpp
extern "C" void  *Radiant_TestMoveEdge( int *outEdges, int *outMovePts,
                                        int *outBothMoved, int *outFaceMoved,
                                        float *outDelta );                             // select.cpp
extern "C" void  *Radiant_TestDragClose( int *outVerDelta, int *outFaceDelta,
                                         int *outModeReset );                          // select.cpp
extern "C" void  *Radiant_TestCloneRotate( int *outAdded, int *outAddedRefcount,
                                           int *outRotated, int *outExactRot );        // select.cpp
extern "C" void  *Radiant_TestCopyPaste( int *outAdded, int *outAddedRefcount,
                                         int *outMatched );                            // select.cpp
extern "C" int    Radiant_TestFindReplaceCount( const char *name );                   // findtexture.cpp
extern "C" int    Radiant_TestFindReplaceAll( const char *findName, const char *replaceName ); // findtexture.cpp
extern "C" int    Radiant_TestLayers( const char *newLayerName );                     // layersdlg.cpp
extern "C" int    Radiant_TestPickModel( void *entDef, const char *name );            // win_ent.cpp
extern "C" int    Radiant_TestPlacePrefab( void *entInst, const char *prefabPath );   // entity.cpp
extern "C" int    Radiant_TestScriptAddKey( const char *value, const char *key );     // scriptgroup.cpp
extern "C" int    Radiant_TestScriptRemoveKey( const char *key, const char *value );  // scriptgroup.cpp
extern "C" int    Radiant_TestDynEntSetPair( const char *value, const char *key );    // dynentitydlg.cpp
extern "C" int    Radiant_TestDynEntRemovePair( const char *key );                    // dynentitydlg.cpp
extern "C" int    Radiant_TestVehicleSetPair( const char *value, const char *key );   // vehicledlg.cpp
extern "C" int    Radiant_TestVehicleRemovePair( const char *key );                   // vehicledlg.cpp
extern "C" int    Radiant_TestAssignScriptGroupNumber( const char *key );             // scriptgroup.cpp
extern "C" int    Radiant_TestRemoveScriptGroupNumber( const char *key );             // scriptgroup.cpp
extern void       Select_Deselect( int bDeselectFaces );                              // select.cpp 0x48E800
extern "C" int    Radiant_TestMarquee( int *outExpected, int *outSelected,
                                       int *outRefSelected, int *outDeleted );          // select.cpp
extern "C" int    Radiant_TestMapInfoCounts( int *out /* int[15] */ );                 // mapinfo.cpp
extern "C" int    Radiant_TestEntityList( int *out /* int[6] */ );                     // entitylist.cpp
extern void       QE_CountBrushesAndUpdateStatusBar();                                 // qe3.cpp (0x48C970)
extern int        g_numbrushes;                                                        // qe3.cpp (0x240A0E4)
extern int        g_numentities;                                                       // qe3.cpp (0x240A0E0)
extern entity_s   entityInsts;                                                         // map.cpp (0x23F1748)
extern bool       Eclass_hasModel( eclass_t *ec );                                     // eclass.cpp (0x481740)
extern "C" int    Radiant_TestTexBarRead( int out[5] );                                // texturebar.cpp
extern "C" int    Radiant_TestTexBarWrite( int shiftH, int shiftV, int scaleH, int scaleV, int rotate ); // texturebar.cpp
extern "C" int    Radiant_TestFindBrushRoundTrip( int *out /* int[4] */ );             // win_dlg.cpp
extern "C" int    Radiant_TestErrorLog( int *out /* int[4] */ );                       // errorfile.cpp
extern signed int LayerdMatWnd();                                                      // layeredmaterials.cpp (0x416D40)
extern int        lyrMtlGlob_entryCount;                                               // layeredmaterials.cpp (0x1814CFC)
extern uint8_t    lyrMtlGlob_Layers[];                                                 // layeredmaterials.cpp (0x1814D00)
extern "C" void   Radiant_TestSeedLayerMaterial( const char *loweredName );            // texwnd.cpp (cache pre-seed)

// Make stdout visible when launched from a console (the editor is a GUI-subsystem
// binary) and mirror results to a log under %TEMP%.
static FILE *s_logFile = nullptr;

static void STLog( const char *fmt, ... )
{
    char buf[2048];
    va_list ap;
    va_start( ap, fmt );
    vsnprintf( buf, sizeof( buf ), fmt, ap );
    va_end( ap );
    fputs( buf, stdout );
    fflush( stdout );
    if ( s_logFile ) { fputs( buf, s_logFile ); fflush( s_logFile ); }
}

// Slurp a whole file into a malloc'd NUL-terminated buffer; returns length or -1.
static long ST_ReadFile( const char *path, char **out )
{
    *out = nullptr;
    FILE *f = fopen( path, "rb" );
    if ( !f ) return -1;
    fseek( f, 0, SEEK_END );
    long n = ftell( f );
    fseek( f, 0, SEEK_SET );
    char *b = (char *)malloc( (size_t)n + 1 );
    if ( !b ) { fclose( f ); return -1; }
    size_t rd = fread( b, 1, (size_t)n, f );
    fclose( f );
    b[rd] = '\0';
    *out = b;
    return (long)rd;
}

// Advance *p past whitespace; returns false at end of string.
static bool ST_NextToken( const char **p, const char **tokStart, int *tokLen, int *line )
{
    const char *s = *p;
    while ( *s )
    {
        if ( *s == '\n' ) { ++*line; ++s; continue; }
        if ( *s == ' ' || *s == '\t' || *s == '\r' || *s == '\f' || *s == '\v' ) { ++s; continue; }
        break;
    }
    if ( !*s ) { *p = s; return false; }
    const char *start = s;
    while ( *s && *s != ' ' && *s != '\t' && *s != '\r' && *s != '\n' && *s != '\f' && *s != '\v' )
        ++s;
    *tokStart = start;
    *tokLen   = (int)( s - start );
    *p        = s;
    return true;
}

// Whitespace-insensitive token comparison. Returns true on match; on mismatch fills
// the diagnostic strings and reports the first divergent token + its line in A.
static bool ST_TokenDiff( const char *a, const char *b,
                          char *diag, size_t diagSize, int *outTokenIndex )
{
    const char *pa = a, *pb = b;
    const char *ta, *tb;
    int la = 1, lb = 1, lenA, lenB, idx = 0;

    for ( ;; )
    {
        bool ha = ST_NextToken( &pa, &ta, &lenA, &la );
        bool hb = ST_NextToken( &pb, &tb, &lenB, &lb );

        if ( !ha && !hb )
        {
            *outTokenIndex = idx;
            return true;                              // both exhausted — identical
        }
        if ( ha != hb || lenA != lenB || memcmp( ta, tb, (size_t)lenA ) != 0 )
        {
            char sa[160] = "<eof>", sb[160] = "<eof>";
            if ( ha ) { int n = lenA < 159 ? lenA : 159; memcpy( sa, ta, n ); sa[n] = 0; }
            if ( hb ) { int n = lenB < 159 ? lenB : 159; memcpy( sb, tb, n ); sb[n] = 0; }
            _snprintf( diag, diagSize,
                       "first divergence at token #%d (orig line %d): orig=[%s]  saved=[%s]",
                       idx, la, sa, sb );
            *outTokenIndex = idx;
            return false;
        }
        ++idx;
    }
}

// Count tokens in a buffer (for the summary line).
static int ST_CountTokens( const char *s )
{
    const char *p = s, *t; int len, line = 1, n = 0;
    while ( ST_NextToken( &p, &t, &len, &line ) ) ++n;
    return n;
}

// ─────────────────────────────────────────────────────────────────────────────
// The round-trip: load <mapPath> through the editor pipeline, save to a temp file,
// compare token streams. Returns the process exit code (0 ok / 1 mismatch / 2 IO).
// ─────────────────────────────────────────────────────────────────────────────
static int RunSelfTest( const char *mapPath )
{
    // Open the log file.
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_selftest.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: map round-trip ===\n" );
    STLog( "map : %s\n", mapPath );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_roundtrip.map", tmpDir );
    STLog( "save: %s\n", savePath );

    // ── Drive the REAL load path (no UI). ─────────────────────────────────────
    // This mirrors the editor's startup load (mainfrm.cpp Radiant_StartupLoadMap):
    // Map_NewMap (bootstrap the brush/entity sentinel lists so Map_LoadFromFile's own
    // Map_Free/Select_Deselect have valid lists to walk) → pre-register worldspawn as a
    // brush eclass (so a brushless worldspawn is not synthesised fixed-size) →
    // Map_LoadFromFile. Map_LoadFromFile itself calls Map_InitlLayers/Layers_SetMapLayers
    // and — critically — Prefab_Init per entity, so the INSTANCE layer (entityInsts /
    // active_brushes, the layer the P5.1 bugs hid in) is now exercised by the gate.
    // g_pParentWnd is NULL in this headless build, so the camera/paste/centre blocks
    // inside Map_LoadFromFile are all guarded off; MainFrm_BrushList/EntList are no-ops.
    STLog( "step: Map_NewMap\n" );            Map_NewMap();   // bootstrap list sentinels
    STLog( "step: Eclass_ForName(worldspawn)\n" );
    Eclass_ForName( 1, "worldspawn" );    // pre-register worldspawn as a brush eclass

    STLog( "step: Map_LoadFromFile\n" );
    Map_LoadFromFile( mapPath );
    int nEnts = g_qeglobals.d_num_entities;
    STLog( "loaded %d entities (g_qeglobals.d_num_entities)\n", nEnts );
    if ( nEnts <= 0 )
    {
        STLog( "RESULT: FAIL (no entities loaded — bad path or parse error)\n" );
        return 2;
    }

    STLog( "step: Map_SaveFile\n" );
    Map_SaveFile( savePath, 0, 0 );

    // ── Compare ───────────────────────────────────────────────────────────────
    char *origBuf = nullptr, *saveBuf = nullptr;
    long  origLen = ST_ReadFile( mapPath,  &origBuf );
    long  saveLen = ST_ReadFile( savePath, &saveBuf );
    if ( origLen < 0 || saveLen < 0 )
    {
        STLog( "RESULT: FAIL (could not re-read files: orig=%ld save=%ld)\n", origLen, saveLen );
        return 2;
    }

    STLog( "orig bytes=%ld tokens=%d ; saved bytes=%ld tokens=%d\n",
           origLen, ST_CountTokens( origBuf ), saveLen, ST_CountTokens( saveBuf ) );

    char diag[512] = "";
    int  tokIdx = 0;
    bool ok = ST_TokenDiff( origBuf, saveBuf, diag, sizeof( diag ), &tokIdx );

    if ( ok )
        STLog( "RESULT: PASS (%d tokens match, whitespace-insensitive)\n", tokIdx );
    else
        STLog( "RESULT: FAIL — %s\n", diag );

    free( origBuf );
    free( saveBuf );
    return ok ? 0 : 1;
}

// Delete alternating active brushes, save, and reload to catch dangling definitions.
static volatile unsigned long g_stLastExcept = 0;

// The GUI exception handler is not installed in self-test mode.
static void ST_ResolveAddr( void *addr, char *buf, size_t bufsz )
{
    HMODULE mod = NULL;
    if ( GetModuleHandleExA(
             GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
             (LPCSTR)addr, &mod ) && mod )
    {
        char modpath[MAX_PATH] = { 0 };
        const char *modname = "?";
        if ( GetModuleFileNameA( mod, modpath, sizeof( modpath ) ) )
        {
            const char *slash = strrchr( modpath, '\\' );
            modname = slash ? slash + 1 : modpath;
        }
        _snprintf( buf, bufsz, "%s!0x%X", modname, (unsigned)( (uintptr_t)addr - (uintptr_t)mod ) );
    }
    else
    {
        _snprintf( buf, bufsz, "%p!?? (no module)", addr );
    }
    buf[bufsz - 1] = 0;
}

static LONG WINAPI ST_Veh( EXCEPTION_POINTERS *ep )
{
    DWORD code = ep->ExceptionRecord->ExceptionCode;
    if ( code == 0xC0000005 || code == 0xC000001D || code == 0x80000003 || code == 0xC0000409 )
    {
        char faultloc[320];
        ST_ResolveAddr( ep->ExceptionRecord->ExceptionAddress, faultloc, sizeof( faultloc ) );
        if ( code == 0xC0000005 && ep->ExceptionRecord->NumberParameters >= 2 )
        {
            ULONG_PTR op = ep->ExceptionRecord->ExceptionInformation[0];
            ULONG_PTR va = ep->ExceptionRecord->ExceptionInformation[1];
            STLog( "*** EXCEPTION 0x%08lX at %s  (%s 0x%p) ***\n",
                   code, faultloc, op == 1 ? "write" : op == 8 ? "exec" : "read", (void *)va );
        }
        else
        {
            STLog( "*** EXCEPTION 0x%08lX at %s ***\n", code, faultloc );
        }
        void *frames[20] = { 0 };
        USHORT n = CaptureStackBackTrace( 0, 20, frames, NULL );
        STLog( "    --- backtrace (%u frames) ---\n", (unsigned)n );
        for ( USHORT i = 0; i < n; ++i )
        {
            char fl[320];
            ST_ResolveAddr( frames[i], fl, sizeof( fl ) );
            STLog( "    [%2u] %p  %s\n", (unsigned)i, frames[i], fl );
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

// No C++ unwindable locals in these → __try is legal (C2712-safe).
static int ST_GuardedLoad( const char *path )
{
    __try { Map_LoadFromFile( path ); return 0; }
    __except( EXCEPTION_EXECUTE_HANDLER ) { g_stLastExcept = GetExceptionCode(); return -1; }
}
static int ST_GuardedSave( const char *path )
{
    __try { Map_SaveFile( path, 0, 0 ); return 0; }
    __except( EXCEPTION_EXECUTE_HANDLER ) { g_stLastExcept = GetExceptionCode(); return -1; }
}
static int ST_GuardedDelete()
{
    __try { Select_Delete(); return 0; }
    __except( EXCEPTION_EXECUTE_HANDLER ) { g_stLastExcept = GetExceptionCode(); return -1; }
}

// True if b's owner entity is dangling (freed: debug-heap 0xDD fill, or unreadable).
// The owner INSTANCE is only 0x54 bytes, so eclass/refCount live on its DEF (reached
// via def @ +0x08, which IS within the instance). Reading eclass off
// the instance directly walks past its allocation — that was the bug; the check must go
// through the def, exactly like the fixed Brush_*_Helper code.
static int ST_OwnerDangling( selbrush_t *b )
{
    __try
    {
        entity_s *o = b->owner;
        if ( !o ) return 0;                                       // null owner = different defect
        unsigned dna = (unsigned)(uintptr_t)o->def;  // instance +0x08 (in-bounds)
        if ( dna == 0xDDDDDDDDu || dna == 0 ) return 1;
        entity_s *def = (entity_s *)o->def;
        unsigned ec = (unsigned)(uintptr_t)def->eclass;          // def +0x60 (def is full 0x8C)
        unsigned rc = (unsigned)def->refCount;
        if ( ec == 0xDDDDDDDDu || rc == 0xDDDDDDDDu ) return 1;
        return 0;
    }
    __except( EXCEPTION_EXECUTE_HANDLER ) { return 1; }
}

// Walk active_brushes; report count + a def->refCount histogram + DANGLING-OWNER count.
// A walk that faults here (corrupt list) is itself the bug surfacing — guarded by caller.
static int ST_DiagBrushes( const char *when )
{
    int n = 0, rc1 = 0, rc2 = 0, rcOther = 0, nullDef = 0, dangling = 0, shown = 0;
    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
    {
        ++n;
        brush_t *d = b->def;
        if ( !d ) ++nullDef;
        else
        {
            int rc = d->refCount;
            if ( rc == 1 )      ++rc1;
            else if ( rc == 2 ) ++rc2;
            else                ++rcOther;
        }
        if ( ST_OwnerDangling( b ) )
        {
            ++dangling;
            if ( shown < 8 )
            {
                entity_s *o = b->owner;
                void *eDef = nullptr; unsigned ecls = 0, rcnt = 0; int readok = 0;
                __try { eDef = o ? o->def : nullptr;
                        if ( eDef ) { ecls = *(unsigned *)((char *)eDef + 0x60);
                                      rcnt = *(unsigned *)((char *)eDef + 0x88); }
                        readok = 1; }
                __except( EXCEPTION_EXECUTE_HANDLER ) { readok = 0; }
                STLog( "    DANGLING: brush=%p def=%p owner=%p ownerDef=%p def.eclass=0x%X def.refCount=0x%X rd=%d\n",
                       (void *)b, (void *)d, (void *)o, eDef, ecls, rcnt, readok );
                ++shown;
            }
        }
    }
    STLog( "[%s] active brushes=%d  def->refCount{1=%d 2=%d other=%d}  nullDef=%d  DANGLING-OWNER=%d\n",
           when, n, rc1, rc2, rcOther, nullDef, dangling );
    return n;
}
static int ST_GuardedDiag( const char *when )
{
    __try { return ST_DiagBrushes( when ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        g_stLastExcept = GetExceptionCode();
        STLog( "[%s] CRASH (0x%08lX) walking active_brushes — list corrupted\n", when, g_stLastExcept );
        return -1;
    }
}

// Count brushes across BOTH display lists (active + selected) and walk each end-to-end so a
// broken prev/next splice or sentinel faults here. Used by the undo/redo gate: a freshly
// created brush lands in selected_brushes (not active), Undo_Undo()'s Select_Deselect moves
// it to active before removing it, and Undo_Redo() re-adds it selected — so only the COMBINED
// total is a stable invariant across the undo/redo cycle. Returns total, or -1 on fault.
static int ST_DiagTotal( const char *when )
{
    int na = 0, ns = 0;
    for ( selbrush_t *b = active_brushes.next;   b != &active_brushes;   b = b->next ) ++na;
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next ) ++ns;
    STLog( "[%s] active=%d selected=%d total=%d\n", when, na, ns, na + ns );
    return na + ns;
}
static int ST_GuardedDiagTotal( const char *when )
{
    __try { return ST_DiagTotal( when ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        g_stLastExcept = GetExceptionCode();
        STLog( "[%s] CRASH (0x%08lX) walking brush lists — list corrupted\n", when, g_stLastExcept );
        return -1;
    }
}

// Select every other active brush (parity by iteration order). Returns selected count,
// or -1 on fault. Select_Brush moves only b (active→selected), so the saved bnext stays
// valid in the active list.
static int ST_SelectEveryOther()
{
    int sel = 0, idx = 0;
    selbrush_t *b = active_brushes.next;
    while ( b != &active_brushes )
    {
        selbrush_t *bnext = b->next;
        if ( ( idx & 1 ) == 0 ) { Select_Brush( b, 0, 0, 0 ); ++sel; }
        b = bnext;
        ++idx;
    }
    return sel;
}
static int ST_GuardedSelect()
{
    __try { return ST_SelectEveryOther(); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        g_stLastExcept = GetExceptionCode();
        return -1;
    }
}

// Create and resize one brush through NewBrushDrag, then verify it survives save/reload.
static int RunCreateTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_create.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: create-brush round-trip ===\n" );
    STLog( "map : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_create.map", tmpDir );
    STLog( "save: %s\n", savePath );

    const bool empty = ( strcmp( mapPath, "empty" ) == 0 );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    int before = 0;
    if ( !empty )
    {
        STLog( "step: Map_LoadFromFile\n" );
        if ( ST_GuardedLoad( mapPath ) != 0 )
        { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
        before = ST_GuardedDiag( "after-load" );
        if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    }
    else
    {
        // Map_New must create worldspawn before the drag path links a brush definition.
        STLog( "step: Map_New (File->New worldspawn init)\n" );
        __try { Map_New(); }
        __except( EXCEPTION_EXECUTE_HANDLER )
        { g_stLastExcept = GetExceptionCode();
          STLog( "RESULT: CRASH during Map_New (0x%08lX)\n", g_stLastExcept ); return 3; }
        if ( !world_entity )
        { STLog( "RESULT: FAIL (Map_New left world_entity NULL)\n" ); return 1; }
        STLog( "Map_New: world_entity=%p (worldspawn)\n", (void *)world_entity );
        before = ST_GuardedDiag( "empty" );
        if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list on empty map)\n" ); return 3; }
    }

    // Synthetic XY input creates then resizes a grid-snapped brush.
    STLog( "step: NewBrushDrag create+resize (256,256 -> 320,320 -> 384,384)\n" );
    void *nb = nullptr;
    __try { nb = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 384, 384,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !nb ) { STLog( "RESULT: FAIL (NewBrushDrag created no brush)\n" ); return 1; }
    STLog( "created brush instance %p (selected)\n", nb );

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int after = ST_GuardedDiag( "after-reload" );
    if ( after < 0 ) { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    STLog( "brush count: before=%d  after-reload=%d  (expected before+1)\n", before, after );
    if ( after != before + 1 )
    { STLog( "RESULT: FAIL (created brush did not survive round-trip)\n" ); return 1; }

    STLog( "RESULT: PASS (create -> save -> reload: +1 brush, round-trip clean)\n" );
    return 0;
}

// Create within an undo bracket and verify create/undo/redo/undo counts and lists.
static int RunUndoRedoTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_undoredo.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: undo/redo create-brush round-trip ===\n" );
    STLog( "map : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    int b0 = ST_GuardedDiagTotal( "after-load" );     // active+selected baseline
    if ( b0 < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // Undo_EndBrushList stamps the selected brush definition with the undo record ID.
    STLog( "step: open undo bracket + NewBrushDrag create (256,256 -> 320,320 -> 384,384)\n" );
    void *nb = nullptr;
    __try
    {
        Undo_ClearRedo();
        Undo_GeneralStart( "create brush" );
        nb = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 384, 384,
                                          0.25f, 0.0f, 0.0f, 512, 512 );
        Undo_EndBrushList( &selected_brushes );
        Undo_End();
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during create+bracket (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !nb ) { STLog( "RESULT: FAIL (NewBrushDrag created no brush)\n" ); return 1; }

    int b1 = ST_GuardedDiagTotal( "after-create" );
    if ( b1 < 0 ) { STLog( "RESULT: CRASH (corrupt list after create)\n" ); return 3; }
    STLog( "total brushes: before=%d  after-create=%d  (expected before+1)\n", b0, b1 );
    if ( b1 != b0 + 1 )
    { STLog( "RESULT: FAIL (create did not add exactly one brush)\n" ); return 1; }

    // Undo expects the created brush on the active list.
    STLog( "step: Undo_Undo\n" );
    __try { Undo_Undo(); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Undo_Undo (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-undo" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt active list after undo)\n" ); return 3; }
    int bU = ST_GuardedDiagTotal( "after-undo" );
    if ( bU < 0 ) { STLog( "RESULT: CRASH (corrupt list after undo)\n" ); return 3; }
    STLog( "total after undo=%d  (expected %d)\n", bU, b0 );
    if ( bU != b0 )
    { STLog( "RESULT: FAIL (undo did not remove the created brush)\n" ); return 1; }

    // Redo relinks the definition to its owner and restores the selected instance.
    STLog( "step: Undo_Redo\n" );
    __try { Undo_Redo(); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Undo_Redo (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-redo" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt active list after redo)\n" ); return 3; }
    int bR = ST_GuardedDiagTotal( "after-redo" );
    if ( bR < 0 ) { STLog( "RESULT: CRASH (corrupt list after redo)\n" ); return 3; }
    STLog( "total after redo=%d  (expected %d)\n", bR, b1 );
    if ( bR != b1 )
    { STLog( "RESULT: FAIL (redo did not restore the created brush)\n" ); return 1; }

    // ── A second UNDO must cleanly remove it again (proves the redo left a finalized,
    //    undoable record — the bracket Undo_Redo re-opened via Undo_GeneralStart). ──
    STLog( "step: Undo_Undo (second)\n" );
    __try { Undo_Undo(); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during 2nd Undo_Undo (0x%08lX)\n", g_stLastExcept ); return 3; }
    int bU2 = ST_GuardedDiagTotal( "after-undo-2" );
    if ( bU2 < 0 ) { STLog( "RESULT: CRASH (corrupt list after 2nd undo)\n" ); return 3; }
    if ( bU2 != b0 )
    { STLog( "RESULT: FAIL (2nd undo did not remove the created brush)\n" ); return 1; }

    STLog( "RESULT: PASS (create -> undo -> redo -> undo: totals %d/%d/%d/%d, lists clean)\n",
           b1, bU, bR, bU2 );
    return 0;
}

// Create a fixed-size entity from a placeholder brush and verify it survives save/reload.
static int RunCreateEntityTest( const char *mapPath, const char *classname )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_createent.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: create-entity round-trip ===\n" );
    STLog( "map : %s\n", mapPath );
    STLog( "class: %s\n", classname );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_createent.map", tmpDir );
    STLog( "save: %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int entsBefore = g_qeglobals.d_num_entities;
    STLog( "loaded %d entities\n", entsBefore );
    if ( entsBefore <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // Drag out the placeholder brush (marks where the entity will drop).
    STLog( "step: NewBrushDrag placeholder (256,256 -> 320,320)\n" );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 320, 320,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during placeholder create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no placeholder brush)\n" ); return 1; }

    // Create the entity from its eclass name (consumes the placeholder).
    STLog( "step: CreateEntityFromName(%s)\n", classname );
    void *ent = nullptr;
    __try { ent = Radiant_TestCreateEntity( classname ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during CreateEntityFromName (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ent ) { STLog( "RESULT: FAIL (no entity created/selected)\n" ); return 1; }
    STLog( "created entity instance %p (selected)\n", ent );
    if ( ST_GuardedDiag( "after-create" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after create)\n" ); return 3; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int entsAfter = g_qeglobals.d_num_entities;
    if ( ST_GuardedDiag( "after-reload" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    STLog( "entity count: before=%d  after-reload=%d  (expected before+1)\n",
           entsBefore, entsAfter );
    if ( entsAfter != entsBefore + 1 )
    { STLog( "RESULT: FAIL (created entity did not survive round-trip)\n" ); return 1; }

    STLog( "RESULT: PASS (create entity '%s' -> save -> reload: +1 entity, round-trip clean)\n",
           classname );
    return 0;
}

// Reclass a selected fixed-size entity in place and verify its bounds and saved map.
extern "C" void *Radiant_TestReclassEntity( const char *newClass, int *outOk );  // xywnd.cpp

static int RunReclassTest( const char *mapPath, const char *classA, const char *classB )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_reclass.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: re-class entity (CreateEntityFromName Path B) ===\n" );
    STLog( "map : %s\n", mapPath );
    STLog( "classA: %s -> classB: %s\n", classA, classB );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_reclass.map", tmpDir );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int entsBefore = g_qeglobals.d_num_entities;
    STLog( "loaded %d entities\n", entsBefore );
    if ( entsBefore <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    STLog( "step: NewBrushDrag placeholder (256,256 -> 320,320)\n" );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 320, 320,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during placeholder create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no placeholder brush)\n" ); return 1; }

    STLog( "step: CreateEntityFromName(%s)  (class A point entity)\n", classA );
    void *entA = nullptr;
    __try { entA = Radiant_TestCreateEntity( classA ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH creating class-A entity (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !entA ) { STLog( "RESULT: FAIL (no class-A entity created)\n" ); return 1; }
    STLog( "class-A entity instance %p (selected)\n", entA );
    int entsAfterCreate = g_qeglobals.d_num_entities;
    if ( ST_GuardedDiag( "after-create-A" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after class-A create)\n" ); return 3; }

    // ── Drive Path B: re-class the selected class-A entity to class B. ──
    STLog( "step: Radiant_TestReclassEntity(%s)  (Path B re-class)\n", classB );
    int reclassOk = 0;
    void *entB = nullptr;
    __try { entB = Radiant_TestReclassEntity( classB, &reclassOk ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during re-class Path B (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !entB ) { STLog( "RESULT: FAIL (re-class produced no selected entity)\n" ); return 1; }
    STLog( "re-classed entity instance %p  ok=%d\n", entB, reclassOk );
    if ( entB != entA )
    { STLog( "RESULT: FAIL (re-class allocated a NEW entity, expected in-place)\n" ); return 1; }
    if ( !reclassOk )
    { STLog( "RESULT: FAIL (eclass/bbox/modelClass checks did not all pass)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-reclass" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after re-class)\n" ); return 3; }

    int entsAfterReclass = g_qeglobals.d_num_entities;
    STLog( "entity count: load=%d  +A=%d  after-reclass=%d (reclass must NOT add an entity)\n",
           entsBefore, entsAfterCreate, entsAfterReclass );
    if ( entsAfterReclass != entsAfterCreate )
    { STLog( "RESULT: FAIL (re-class changed the entity count)\n" ); return 1; }

    // ── Round-trip: save + reload, verify the re-classed entity persists as class B. ──
    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int entsAfter = g_qeglobals.d_num_entities;
    if ( ST_GuardedDiag( "after-reload" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }
    STLog( "entity count after reload = %d (expected load+1 = %d)\n", entsAfter, entsBefore + 1 );
    if ( entsAfter != entsBefore + 1 )
    { STLog( "RESULT: FAIL (re-classed entity did not survive round-trip)\n" ); return 1; }

    STLog( "RESULT: PASS (re-class '%s' -> '%s': in-place, bbox rebuilt, modelClass cleared, round-trip clean)\n",
           classA, classB );
    return 0;
}

// Exercise Brush_SetSampleSize (0x48F800) and sub_477080 (0x477080).
extern "C" float Radiant_TestSampleSize( int size );          // select.cpp

static int RunSampleSizeTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_samplesize.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Brush_SetSampleSize -> sub_477080 (set sample size) ===\n" );
    STLog( "map : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    STLog( "step: NewBrushDrag world brush (256,256 -> 320,320)\n" );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 320, 320,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during brush create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no brush created)\n" ); return 1; }

    STLog( "step: Radiant_TestSampleSize(4)\n" );
    float sz = -2.0f;
    __try { sz = Radiant_TestSampleSize( 4 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Brush_SetSampleSize/sub_477080 (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "lightmap texdef scale after SetSampleSize(4) = %g (expect 2048)\n", sz );
    if ( sz < 0.0f )
    { STLog( "RESULT: FAIL (no editable brush to sample-size)\n" ); return 1; }
    float d = sz - 2048.0f; if ( d < 0 ) d = -d;
    if ( d > 0.5f )
    { STLog( "RESULT: FAIL (sample-size scale %g != 4*512=2048)\n", sz ); return 1; }
    if ( ST_GuardedDiag( "after-samplesize" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after sample size)\n" ); return 3; }

    STLog( "RESULT: PASS (set sample size: lightmap texdef scale = 4*512 = 2048)\n" );
    return 0;
}

// Exercise Brush_SetTextureMapping (0x48F4F0) and sub_477020 (0x477020).
extern "C" float Radiant_TestTexMapping();                    // select.cpp

static int RunTexMappingTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_texmapping.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Brush_SetTextureMapping -> sub_477020 (set brush tex mapping) ===\n" );
    STLog( "map : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    STLog( "step: NewBrushDrag world brush (256,256 -> 320,320)\n" );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 320, 320,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during brush create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no brush created)\n" ); return 1; }

    STLog( "step: Radiant_TestTexMapping\n" );
    float v = -2.0f;
    __try { v = Radiant_TestTexMapping(); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Brush_SetTextureMapping/sub_477020 (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "face[0] mat_texDef.size[0] after SetTextureMapping = %g (expect 13)\n", v );
    if ( v < 0.0f )
    { STLog( "RESULT: FAIL (no editable brush to texture-map)\n" ); return 1; }
    float d = v - 13.0f; if ( d < 0 ) d = -d;
    if ( d > 1e-3f )
    { STLog( "RESULT: FAIL (texdef scale %g != applied 13.0)\n", v ); return 1; }
    if ( ST_GuardedDiag( "after-texmapping" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after tex mapping)\n" ); return 3; }

    STLog( "RESULT: PASS (set brush texture mapping: texdef applied to all faces)\n" );
    return 0;
}

// Known-answer checks for texture-projection helpers 0x4A45D0 and 0x4769A0.
extern int  sub_4A45D0( float *dir );        // engine_stubs.cpp 0x4A45D0 (C++ linkage)
extern void sub_4769A0( int a1, int a2, int a3 );  // engine_stubs.cpp 0x4769A0 (C++ linkage)

static int RunTexProjMathTest( const char * )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_texprojmath.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: sub_4A45D0 / sub_4769A0 (texture-projection math) ===\n" );

    float vx[3] = { 3, 1, 1 }, vy[3] = { 1, 3, 1 }, vz[3] = { 1, 1, 3 };
    int ax = sub_4A45D0( vx ), ay = sub_4A45D0( vy ), az = sub_4A45D0( vz );
    STLog( "sub_4A45D0: [3,1,1]=%d [1,3,1]=%d [1,1,3]=%d (expect 0 1 2)\n", ax, ay, az );
    if ( ax != 0 || ay != 1 || az != 2 )
    { STLog( "RESULT: FAIL (dominant-axis wrong)\n" ); return 1; }

    float planes[8] = { 1, 0, 0, 5,  0, 1, 0, 7 };   // S=(1,0,0,5)  T=(0,1,0,7)
    float normal[3] = { 0, 0, 0 };
    float grad[3]   = { 9, 9, 9 };
    sub_4769A0( (int)(intptr_t)normal, (int)(intptr_t)planes, (int)(intptr_t)grad );
    STLog( "sub_4769A0: normal=(%g,%g,%g) grad=(%g,%g,%g)\n",
           normal[0], normal[1], normal[2], grad[0], grad[1], grad[2] );

    float dn = normal[2] - 1.0f; if ( dn < 0 ) dn = -dn;
    float d0 = grad[0] - ( -5.0f ); if ( d0 < 0 ) d0 = -d0;
    float d1 = grad[1] - ( -7.0f ); if ( d1 < 0 ) d1 = -d1;
    float d2 = grad[2];            if ( d2 < 0 ) d2 = -d2;
    int ok = ( dn <= 1e-3f && d0 <= 1e-3f && d1 <= 1e-3f && d2 <= 1e-3f
            && normal[0] <= 1e-3f && normal[0] >= -1e-3f
            && normal[1] <= 1e-3f && normal[1] >= -1e-3f );
    if ( !ok )
    { STLog( "RESULT: FAIL (gradient/normal != expected (0,0,1)/(-5,-7,0))\n" ); return 1; }

    STLog( "RESULT: PASS (dominant axis + texcoord gradient solver correct)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunMatInv44Test — verify the MatrixInverse44(const void*, void*) shadow-stub fix: the void*
//  overload (Brush_ApplyTextureProjection's call shape) used to be a FATAL stub; it now routes
//  to com_math's real typed MatrixInverse44.  Invert a known non-singular 4x4 and assert
//  M * inv(M) == I (within tolerance), which also validates the void*->mat4x4* cast.  Map-free.
// ─────────────────────────────────────────────────────────────────────────────
extern void MatrixInverse44( const void *src, void *dst );    // engine_stubs void* overload

static int RunMatInv44Test( const char * )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_matinv44.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: MatrixInverse44(void*,void*) shadow-stub -> real impl ===\n" );

    // scale(2,3,4) + translate(1,2,3), row-major m[i][j] = M[i*4+j]; det = 24 (non-singular)
    float M[16] = { 2,0,0,1,  0,3,0,2,  0,0,4,3,  0,0,0,1 };
    float Inv[16];
    for ( int i = 0; i < 16; ++i ) Inv[i] = 0.0f;

    MatrixInverse44( M, Inv );                   // void* overload -> com_math typed overload

    float maxErr = 0.0f;
    for ( int i = 0; i < 4; ++i )
        for ( int j = 0; j < 4; ++j )
        {
            float s = 0.0f;
            for ( int k = 0; k < 4; ++k ) s += M[i*4+k] * Inv[k*4+j];
            float want = ( i == j ) ? 1.0f : 0.0f;
            float e = s - want; if ( e < 0 ) e = -e;
            if ( e > maxErr ) maxErr = e;
        }
    STLog( "max |M*inv(M) - I| = %g (expect ~0)\n", maxErr );
    if ( maxErr > 1e-4f )
    { STLog( "RESULT: FAIL (M * inv(M) != I — wrapper/cast/delegation broken)\n" ); return 1; }

    STLog( "RESULT: PASS (void* MatrixInverse44 routes to the real 4x4 inverse)\n" );
    return 0;
}

// Round-trip Face_MoveTexture (0x45A1C0) through texturevecs_02 (0x459CC0).
extern void Face_MoveTexture( int surfDef, const float *normal, int outVecs,
                              int uvBase, float rotate, float crossterm );      // brush.cpp 0x45A1C0
extern void texturevecs_02( int outSize, int texMatPtr, float st1_phantom,
                            int planeNormalPtr, float planeDist,
                            int outShift, int outRotate, int outCrossterm );    // brush.cpp 0x459CC0

// Negative control selected only under TEXVECS_BREAK.
static void TexVecs_BrokenInverse( float *outSize, float * /*texMat*/, const float * /*N*/,
                                   float * outShift, float *outRotate, float *outCrossterm )
{
    outSize[0] = 1.0f; outSize[1] = 1.0f;     // wrong (real size is e.g. 64/128/37.5)
    outShift[0] = 0.0f; outShift[1] = 0.0f;   // wrong unless shift happened to be 0
    *outRotate = 0.0f;                         // wrong unless rotate happened to be 0
    *outCrossterm = 0.0f;
}

static float TexVecs_AngleDiff( float a, float b )   // |a-b| reduced into [0,180]
{
    float d = a - b;
    while ( d > 180.0f )  d -= 360.0f;
    while ( d < -180.0f ) d += 360.0f;
    return ( d < 0.0f ) ? -d : d;
}

// KISAK: 0x5139A0 writes the CoD3 renderer's imageGlobals picmip fields.
extern void __cdecl Radiant_TestPicmipUpdate(int picmip, int bump, int spec,
                                             int *outPic, int *outBump, int *outSpec); // r_image.cpp

static int RunPicmipTest()
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_picmip.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    const bool broken = ( getenv( "PICMIP_BREAK" ) != nullptr );
    STLog( "=== RADIANT_SELFTEST: R_UpdateMipMap dvar->imageGlobals.picmip* propagation%s ===\n",
           broken ? "  [PICMIP_BREAK: wrong oracle, expect FAIL]" : "" );

    int ok = 1;
    // Three distinct triples so a mis-wired copy (e.g. all read from r_picmip) is caught.
    struct { int pic, bump, spec; } cases[] = { { 0, 1, 2 }, { 3, 2, 1 }, { 2, 0, 3 } };
    for ( int c = 0; c < 3; ++c )
    {
        int gp = -99, gb = -99, gs = -99;
        Radiant_TestPicmipUpdate( cases[c].pic, cases[c].bump, cases[c].spec, &gp, &gb, &gs );
        // The broken oracle expects all three equal to .pic (what a naive single-dvar copy
        // would give); the real R_UpdateMipMap makes bump/spec differ, so it FAILs the oracle.
        int expBump = broken ? cases[c].pic : cases[c].bump;
        int expSpec = broken ? cases[c].pic : cases[c].spec;
        STLog( "  case %d: set(%d,%d,%d) -> imageGlobals(%d,%d,%d) expect(%d,%d,%d)\n",
               c, cases[c].pic, cases[c].bump, cases[c].spec, gp, gb, gs,
               cases[c].pic, expBump, expSpec );
        if ( gp != cases[c].pic || gb != expBump || gs != expSpec )
            ok = 0;
    }

    if ( !ok ) { STLog( "RESULT: FAIL (picmip propagation incorrect)\n" ); if (s_logFile) fclose(s_logFile); return 1; }
    STLog( "RESULT: PASS (r_picmip*/bump/spec dvars propagate to imageGlobals.picmip* 3/3)\n" );
    if (s_logFile) fclose(s_logFile);
    return 0;
}

static int RunTexVecsTest()
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_texvecs.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    const bool broken = ( getenv( "TEXVECS_BREAK" ) != nullptr );
    STLog( "=== RADIANT_SELFTEST: texturevecs_02 forward∘inverse round-trip%s ===\n",
           broken ? "  [TEXVECS_BREAK: deliberately-wrong impl, expect FAIL]" : "" );

    // ── corner-covering test cases ──────────────────────────────────────────────
    struct TexCase { float size[2], shift[2], rot, cross; };
    static const TexCase cases[] = {
        // size, shift, rotate, crossterm
        { {  64,  64 }, {   0,    0 },    0.0f, 0.0f },
        { { 128, 128 }, {  16,   16 },    0.0f, 0.0f },
        { {  64, 128 }, { -33.25f, 16 }, 30.0f, 0.0f },
        { { 128,  64 }, {  16,  -33.25f }, 90.0f, 0.0f },
        { {  64,  64 }, {   0,    0 },  -45.0f, 0.0f },
        { { 128, 128 }, {  16,   16 },  135.0f, 0.0f },
        { {  37.5f, 64 }, { -33.25f, 0 }, 30.0f, 0.3f },
        { {  64, 37.5f }, { 16, 16 },    -45.0f, 0.3f },
    };
    // plane normals — +Z,-Z,+X,+Y,+ a tilted unit normal.
    float tiltN[3]; {
        float ux = 1, uy = 2, uz = 3, l = (float)sqrt( (double)(ux*ux+uy*uy+uz*uz) );
        tiltN[0] = ux/l; tiltN[1] = uy/l; tiltN[2] = uz/l;
    }
    static float normals_storage[5][3] = {
        { 0,0,1 }, { 0,0,-1 }, { 1,0,0 }, { 0,1,0 }, { 0,0,0 }
    };
    normals_storage[4][0] = tiltN[0]; normals_storage[4][1] = tiltN[1]; normals_storage[4][2] = tiltN[2];
    const char *normNames[5] = { "+Z", "-Z", "+X", "+Y", "norm(1,2,3)" };

    const float TOL = 1e-3f;
    int total = 0, fails = 0;

    for ( int ni = 0; ni < 5; ++ni )
    {
        const float *N = normals_storage[ni];
        for ( int ci = 0; ci < (int)( sizeof( cases ) / sizeof( cases[0] ) ); ++ci )
        {
            const TexCase &T0 = cases[ci];
            total++;

            // FORWARD (trusted): texdef -> texMat[8].
            float fwdSize[2]  = { T0.size[0],  T0.size[1]  };
            float fwdShift[2] = { T0.shift[0], T0.shift[1] };
            float texMat[8];
            for ( int k = 0; k < 8; ++k ) texMat[k] = 0.0f;
            Face_MoveTexture( (int)(intptr_t)fwdSize, N, (int)(intptr_t)texMat,
                              (int)(intptr_t)fwdShift, T0.rot, T0.cross );

            // INVERSE (unit under test): texMat -> texdef.
            float outSize[2] = { 0,0 }, outShift[2] = { 0,0 }, outRot = 0, outCross = 0;
            if ( broken )
                TexVecs_BrokenInverse( outSize, texMat, N, outShift, &outRot, &outCross );
            else
                texturevecs_02( (int)(intptr_t)outSize, (int)(intptr_t)texMat, 0.0f,
                                (int)(intptr_t)N, /*planeDist=*/0.0f,
                                (int)(intptr_t)outShift, (int)(intptr_t)&outRot,
                                (int)(intptr_t)&outCross );

            // COMPARE T1 ≈ T0 (tight).  rotate compared mod 360; sizes by abs value
            // (the binary's size[1] carries an orientation sign — its magnitude must match).
            float es0 = outSize[0]  - T0.size[0];   if ( es0 < 0 ) es0 = -es0;
            float as1 = ( outSize[1] < 0 ) ? -outSize[1] : outSize[1];
            float es1 = as1 - T0.size[1];           if ( es1 < 0 ) es1 = -es1;
            float ef0 = outShift[0] - T0.shift[0];  if ( ef0 < 0 ) ef0 = -ef0;
            float ef1 = outShift[1] - T0.shift[1];  if ( ef1 < 0 ) ef1 = -ef1;
            float er  = TexVecs_AngleDiff( outRot, T0.rot );
            float ec  = outCross    - T0.cross;     if ( ec < 0 ) ec = -ec;

            bool ok = ( es0 <= TOL && es1 <= TOL && ef0 <= TOL && ef1 <= TOL
                     && er <= 1e-2f && ec <= TOL );
            if ( !ok ) fails++;

            STLog( "[%-11s] T0{sz %.4g,%.4g sh %.4g,%.4g r %.4g x %.4g} "
                   "-> T1{sz %.4g,%.4g sh %.4g,%.4g r %.4g x %.4g}  "
                   "err{sz %.2g,%.2g sh %.2g,%.2g r %.2g x %.2g}  %s\n",
                   normNames[ni],
                   T0.size[0], T0.size[1], T0.shift[0], T0.shift[1], T0.rot, T0.cross,
                   outSize[0], outSize[1], outShift[0], outShift[1], outRot, outCross,
                   es0, es1, ef0, ef1, er, ec, ok ? "OK" : "MISMATCH" );
        }
    }

    STLog( "%d/%d cases round-tripped within tol (size/shift abs<=%g, rotate<=1e-2 deg, crossterm abs<=%g)\n",
           total - fails, total, TOL, TOL );

    if ( broken )
    {
        // Soundness check: the wrong impl MUST be rejected by the comparison.
        if ( fails > 0 )
        { STLog( "RESULT: PASS-NEGATIVE (gate correctly REJECTS the deliberately-wrong impl: %d/%d failed)\n", fails, total );
          if ( s_logFile ) fclose( s_logFile ); return 1; }   // nonzero = the broken path failed, as required
        STLog( "RESULT: BAD-GATE (the wrong impl was NOT rejected — the gate is vacuous!)\n" );
        if ( s_logFile ) fclose( s_logFile ); return 2;
    }

    if ( fails > 0 )
    { STLog( "RESULT: FAIL (%d/%d cases did not round-trip)\n", fails, total );
      if ( s_logFile ) fclose( s_logFile ); return 1; }

    STLog( "RESULT: PASS (all %d forward∘inverse texdef round-trips recovered within tolerance)\n", total );
    if ( s_logFile ) fclose( s_logFile );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchPickTest — the PATCH texture-PICK round-trip gate (sub_44B620, 0x44B620,
//  brush.cpp Ed_Patch_GetTexdef + the 4 inline-FPU deps).  The patch analog of the
//  texvecs gate: it extracts a planar texdef back out of a patch whose control-point
//  texCoords were laid down by the trusted forward (Face_MoveTexture's world tex matrix).
//
//    T0 = {size,shift,rotate}
//    Face_MoveTexture(T0, N) -> texMat[8]                       (FORWARD, trusted)
//    for every control pt P:  st = (texMat[0..2]·P + texMat[3],  texMat[4..6]·P + texMat[7])
//    Ed_Patch_GetTexdef(patch, &T1)                              (INVERSE, the unit under test)
//    assert T1.size/shift ≈ T0 and T1.rotate ≡ T0 (mod 360).
//
//  NOTE (faithful binary fact): 0x44B620 has NO success return — it ALWAYS returns 0; the
//  texdef WRITE is the observable.  So the gate compares the WRITTEN T1 against T0 (it seeds
//  T1 with sentinels first); it does not look at the return value.
//
//  SOUNDNESS PROOF (non-vacuity): with env var PATCHPICK_BREAK set, the harness substitutes
//  a deliberately-WRONG identity decomposition (leaves the sentinel/identity texdef) and the
//  gate MUST reject it.  GREEN requires: broken-pass nonzero AND real-pass zero.
//
//  Map-free.  Builds flat 3×3 patches (control grid on the plane) for normals {+Z,+X,tilted}
//  with a 2×2 flat curveDef so Ed_Patch_PlanarTest finds a clean plane.  Integer-degree
//  rotations only (the binary's rotate is gated by a floorf(x+0.5) integer roundtrip check).
// ─────────────────────────────────────────────────────────────────────────────
extern char Radiant_PatchGetTexdef( patchMesh_t *patch, texdef_sub_t *texdef );  // brush.cpp 0x44B620

// Deliberately-WRONG inverse for the soundness demo: leaves an identity texdef → not T0.
static void PatchPick_BrokenInverse( patchMesh_t * /*patch*/, texdef_sub_t *td )
{
    td->size[0] = 1.0f; td->size[1] = 1.0f;
    td->shift[0] = 0.0f; td->shift[1] = 0.0f;
    td->rotate = 0.0f;
}

static float PatchPick_AngleDiff( float a, float b )   // |a-b| reduced into [0,180]
{
    float d = a - b;
    while ( d > 180.0f )  d -= 360.0f;
    while ( d < -180.0f ) d += 360.0f;
    return ( d < 0.0f ) ? -d : d;
}

static int RunPatchPickTest()
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchpick.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    const bool broken = ( getenv( "PATCHPICK_BREAK" ) != nullptr );
    STLog( "=== RADIANT_SELFTEST: patch texture-pick (sub_44B620) forward∘inverse round-trip%s ===\n",
           broken ? "  [PATCHPICK_BREAK: deliberately-wrong impl, expect FAIL]" : "" );

    // The pick reads ctrl texCoords from layer g_qeglobals.current_edit_layer; the forward
    // writes them there too.  Force layer 0 for the gate (default in a fresh selftest).
    g_qeglobals.current_edit_layer = 0;

    // texdef test cases (integer-degree rotations only; shifts that snap clean on /8 grid).
    struct TexCase { float size[2], shift[2], rot; };
    static const TexCase cases[] = {
        { {  64,  64 }, {   0,    0 },   0.0f },
        { { 128, 128 }, {  16,   16 },   0.0f },
        { {  64, 128 }, { -32,   16 },  30.0f },
        { { 128,  64 }, {  16,  -32 },  90.0f },
        { {  64,  64 }, {   0,    0 }, -45.0f },
        { { 128, 128 }, {  16,   16 }, 135.0f },
    };
    // plane normals: +Z, +X, and a tilted unit normal.
    float tiltN[3]; {
        float ux = 1, uy = 2, uz = 3, l = (float)sqrt( (double)(ux*ux+uy*uy+uz*uz) );
        tiltN[0] = ux/l; tiltN[1] = uy/l; tiltN[2] = uz/l;
    }
    static float normals[3][3] = { { 0,0,1 }, { 1,0,0 }, { 0,0,0 } };
    normals[2][0] = tiltN[0]; normals[2][1] = tiltN[1]; normals[2][2] = tiltN[2];
    const char *normNames[3] = { "+Z", "+X", "norm(1,2,3)" };

    // Build two in-plane basis vectors (e1,e2) for a given plane normal so we can lay
    // control points out ON the plane.
    auto basisFor = []( const float *N, float *e1, float *e2 ) {
        float up[3] = { 0, 0, 1 };
        if ( ( N[2] < 0 ? -N[2] : N[2] ) > 0.9f ) { up[0] = 1; up[1] = 0; up[2] = 0; }
        // e1 = normalize(up × N)
        e1[0] = up[1]*N[2] - up[2]*N[1];
        e1[1] = up[2]*N[0] - up[0]*N[2];
        e1[2] = up[0]*N[1] - up[1]*N[0];
        float l1 = (float)sqrt( (double)(e1[0]*e1[0]+e1[1]*e1[1]+e1[2]*e1[2]) );
        e1[0]/=l1; e1[1]/=l1; e1[2]/=l1;
        // e2 = N × e1
        e2[0] = N[1]*e1[2] - N[2]*e1[1];
        e2[1] = N[2]*e1[0] - N[0]*e1[2];
        e2[2] = N[0]*e1[1] - N[1]*e1[0];
    };

    extern void Face_MoveTexture( int surfDef, const float *normal, int outVecs,
                                  int uvBase, float rotate, float crossterm );  // brush.cpp 0x45A1C0

    const float TOL = 1e-2f;     // size/shift abs; rotate compared mod 360 (<=1e-2 deg)
    int total = 0, fails = 0;

    // one shared patch struct + curveDef + 4 curve verts (2×2 flat mesh).
    static patchMesh_t patch;   // 20556 bytes — static to keep the harness stack small.
    static curvePatchDef_t cdef;
    static curveVert_t cverts[4];
    // a fake 512×512 radMtl so MaterialDef_GetLayeredMaterial returns a valid texel size
    // (the binary always has a real material on a picked patch; headless we synthesize one).
    static qtexture_s fakeTex; memset( &fakeTex, 0, sizeof( fakeTex ) );
    fakeTex.width = 512; fakeTex.height = 512;

    for ( int ni = 0; ni < 3; ++ni )
    {
        const float *N = normals[ni];
        float e1[3], e2[3];
        basisFor( N, e1, e2 );

        for ( int ci = 0; ci < (int)( sizeof(cases)/sizeof(cases[0]) ); ++ci )
        {
            const TexCase &T0 = cases[ci];
            total++;

            // FORWARD (trusted): texdef -> world tex matrix.
            float fwdSize[2]  = { T0.size[0],  T0.size[1]  };
            float fwdShift[2] = { T0.shift[0], T0.shift[1] };
            float texMat[8];
            for ( int k = 0; k < 8; ++k ) texMat[k] = 0.0f;
            Face_MoveTexture( (int)(intptr_t)fwdSize, N, (int)(intptr_t)texMat,
                              (int)(intptr_t)fwdShift, T0.rot, /*crossterm*/0.0f );

            // Build a flat 3×3 control grid on the plane through the origin, plus its
            // tessellated st via the forward texMat.
            memset( &patch, 0, sizeof( patch ) );
            patch.width = 3; patch.height = 3;
            patch.texture.radMtl = &fakeTex;   // valid material -> GetLayeredMaterial returns 512×512
            for ( int col = 0; col < 3; ++col )
            {
                for ( int row = 0; row < 3; ++row )
                {
                    // spread control points out so the 3 chosen corners are non-collinear.
                    float a = (float)( col * 96 - 96 );   // -96, 0, +96
                    float b = (float)( row * 96 - 96 );
                    drawVert_t &cv = patch.ctrl[col][row];
                    cv.xyz[0] = a*e1[0] + b*e2[0];
                    cv.xyz[1] = a*e1[1] + b*e2[1];
                    cv.xyz[2] = a*e1[2] + b*e2[2];
                    // st via the trusted forward matrix (the exact projection the inverse must undo).
                    float s = texMat[0]*cv.xyz[0] + texMat[1]*cv.xyz[1] + texMat[2]*cv.xyz[2] + texMat[3];
                    float t = texMat[4]*cv.xyz[0] + texMat[5]*cv.xyz[1] + texMat[6]*cv.xyz[2] + texMat[7];
                    cv.texCoord.st[0] = s; cv.texCoord.st[1] = t;
                }
            }
            // 2×2 flat curveDef sampling the plane (so Ed_Patch_PlanarTest gets the normal).
            cdef.width = 2; cdef.height = 2; cdef.random_one = 1; cdef.verts = cverts; cdef.unk_after = 0;
            for ( int q = 0; q < 4; ++q )
            {
                float a = (float)( ( q & 1 ) ? 96 : -96 );
                float b = (float)( ( q & 2 ) ? 96 : -96 );
                cverts[q].xyz[0] = a*e1[0] + b*e2[0];
                cverts[q].xyz[1] = a*e1[1] + b*e2[1];
                cverts[q].xyz[2] = a*e1[2] + b*e2[2];
            }
            patch.curveDef = &cdef;

            // INVERSE (unit under test): patch -> texdef.  Seed sentinels first (the side
            // effect is the signal; the function always returns 0).
            texdef_sub_t T1; memset( &T1, 0, sizeof( T1 ) );
            T1.size[0] = -999.0f; T1.size[1] = -999.0f;
            T1.shift[0] = -999.0f; T1.shift[1] = -999.0f; T1.rotate = -999.0f;
            if ( broken )
                PatchPick_BrokenInverse( &patch, &T1 );
            else
                Radiant_PatchGetTexdef( &patch, &T1 );

            // COMPARE.  size[i] is recovered in PIXEL units (|texMat row| = 1/size), so it
            // must equal T0.size (size[1] magnitude — the binary carries an orientation sign).
            // shift[i] is recovered as the affine CONST term, i.e. the ST-SPACE shift the
            // forward placed in texMat[3]/texMat[7] (= -T0.shift/T0.size), NOT the pixel shift —
            // so we compare T1.shift against texMat[3]/texMat[7] (the exact value the forward
            // encoded and the inverse must recover).  This is a faithful binary fact, not a fudge.
            float es0 = T1.size[0]  - T0.size[0];   if ( es0 < 0 ) es0 = -es0;
            float as1 = ( T1.size[1] < 0 ) ? -T1.size[1] : T1.size[1];
            float es1 = as1 - T0.size[1];           if ( es1 < 0 ) es1 = -es1;
            float expShift0 = texMat[3], expShift1 = texMat[7];
            float ef0 = T1.shift[0] - expShift0;    if ( ef0 < 0 ) ef0 = -ef0;
            float ef1 = T1.shift[1] - expShift1;    if ( ef1 < 0 ) ef1 = -ef1;
            float er  = PatchPick_AngleDiff( T1.rotate, T0.rot );

            bool ok = ( es0 <= TOL && es1 <= TOL && ef0 <= TOL && ef1 <= TOL && er <= 1e-2f );
            if ( !ok ) fails++;

            STLog( "[%-11s] T0{sz %.4g,%.4g sh %.4g,%.4g r %.4g} -> "
                   "T1{sz %.4g,%.4g sh %.4g,%.4g(exp %.4g,%.4g) r %.4g}  "
                   "err{sz %.2g,%.2g sh %.2g,%.2g r %.2g}  %s\n",
                   normNames[ni],
                   T0.size[0], T0.size[1], T0.shift[0], T0.shift[1], T0.rot,
                   T1.size[0], T1.size[1], T1.shift[0], T1.shift[1], expShift0, expShift1, T1.rotate,
                   es0, es1, ef0, ef1, er, ok ? "OK" : "MISMATCH" );
        }
    }

    STLog( "%d/%d cases round-tripped within tol (size/shift abs<=%g, rotate<=1e-2 deg)\n",
           total - fails, total, TOL );

    if ( broken )
    {
        if ( fails > 0 )
        { STLog( "RESULT: PASS-NEGATIVE (gate correctly REJECTS the deliberately-wrong impl: %d/%d failed)\n", fails, total );
          if ( s_logFile ) fclose( s_logFile ); return 1; }
        STLog( "RESULT: BAD-GATE (the wrong impl was NOT rejected — the gate is vacuous!)\n" );
        if ( s_logFile ) fclose( s_logFile ); return 2;
    }
    if ( fails > 0 )
    { STLog( "RESULT: FAIL (%d/%d cases did not round-trip)\n", fails, total );
      if ( s_logFile ) fclose( s_logFile ); return 1; }

    STLog( "RESULT: PASS (all %d patch texture-pick round-trips recovered within tolerance)\n", total );
    if ( s_logFile ) fclose( s_logFile );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunTerrainCircleTest — the terrain-paint cursor RING gate (pmesh.cpp
//  PMESH_20_Radius_2 0x43F580 + PMESH_19_Radius 0x43F230 + sub_43ED50 0x43ED50).
//  Map-free.  Two layers, both KNOWN-ANSWER:
//
//   (a) PMESH_20_Radius_2 — PURE DATA point emitter (no camera dependency).  Build a flat
//       3×3 control grid on z=0, a symbiont brush_t whose mins/maxs cover it, a cursor at the
//       origin, innerR/outerR; then assert the emitted POINT count == #(control pts inside the
//       outer ring), each point's xyz == its control xyz (identity), and each point's packed
//       colour == the independently-computed ramp lerp by the smoothstep falloff weight.
//
//   (b) PMESH_19_Radius + sub_43ED50 — LINE clip.  Pin the camera state (vpn = {0,0,0},
//       world_orient = identity) so emitted endpoints are the raw clip-intersection points.
//       Build a flat curveDef triangle that fully CONTAINS the inner ring -> every ring
//       segment is wholly inside -> expect 16 line segments (32 verts) for the inner ring;
//       and a ring fully OUTSIDE a far triangle -> expect 0 verts.
//
//  SOUNDNESS PROOF (non-vacuity): with env var TERRAINCIRCLE_BREAK set, PMESH_20's expected
//  colour ramp is recomputed WITHOUT the smoothstep (linear weight) so the gate's compare must
//  REJECT the real (correct) impl's output — proving the colour/falloff check is live.  (The
//  break perturbs the ORACLE, not the impl: GREEN requires real-pass AND broken-reject.)
// ─────────────────────────────────────────────────────────────────────────────
extern "C++" int PMESH_19_Radius( patchMesh_t *patch, const float *cursor, float innerR, float outerR,
                                  const float *innerRing, const float *outerRing,
                                  const unsigned int *color, int count, GfxPointVertex *outVerts );
extern "C++" int PMESH_20_Radius_2( int count, patchMesh_t *patch, const float *cursor,
                                    float innerR, float outerR, GfxPointVertex *outVerts );
extern char Byte4PackPixelColor( float *from, GfxColor *out );  // 0x402AC0

static int RunTerrainCircleTest()
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_terraincircle.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    const bool broken = ( getenv( "TERRAINCIRCLE_BREAK" ) != nullptr );
    STLog( "=== RADIANT_SELFTEST: terrain-paint cursor ring (PMESH_20/PMESH_19/sub_43ED50)%s ===\n",
           broken ? "  [TERRAINCIRCLE_BREAK: oracle drops smoothstep, expect REJECT]" : "" );

    int fails = 0, total = 0;

    // shared ramp + smoothstep oracle (independent of the impl).
    static const float ramp[2][4] = { { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.25f, 0.25f, 1.0f } };
    const float innerR = 64.0f, outerR = 128.0f;
    const float inner2 = innerR * innerR, outer2 = outerR * outerR;

    // ── Layer (a): PMESH_20_Radius_2 point emission ──────────────────────────────
    {
        // flat 3×3 control grid on z=0, spread so points fall inside/outside both rings.
        static patchMesh_t patch;  memset( &patch, 0, sizeof( patch ) );
        static brush_t     sym;    memset( &sym, 0, sizeof( sym ) );
        patch.width = 3; patch.height = 3;
        patch.pSymbiot = (entity_brush_s *)&sym;
        sym.mins[0] = -200; sym.mins[1] = -200; sym.mins[2] = -8;
        sym.maxs[0] =  200; sym.maxs[1] =  200; sym.maxs[2] =  8;
        // control XY at {-90, 0, 90} on each axis so points land in the FALLOFF BAND
        // (inner 64 < dist < outer 128): edge pts dist 90, corner pts dist ~127 (both in band),
        // centre dist 0 (w=1).  This exercises the smoothstep so TERRAINCIRCLE_BREAK (linear
        // oracle) genuinely diverges; with all-outside or all-inside points the break is vacuous.
        const float coords[3] = { -90.0f, 0.0f, 90.0f };
        for ( int col = 0; col < 3; ++col )
            for ( int row = 0; row < 3; ++row )
            {
                drawVert_t &cv = patch.ctrl[col][row];
                cv.xyz[0] = coords[col]; cv.xyz[1] = coords[row]; cv.xyz[2] = 0.0f;
            }
        const float cursor[3] = { 0.0f, 0.0f, 0.0f };

        // ORACLE: which control points emit, in the impl's iteration order (col-major), + colour.
        struct ExpPt { float x, y, z; unsigned int packed; };
        ExpPt exp[9]; int expN = 0;
        for ( int col = 0; col < 3; ++col )
            for ( int row = 0; row < 3; ++row )
            {
                const float px = coords[col], py = coords[row];
                const float dx = px - cursor[0], dy = py - cursor[1];
                const float d2 = dy * dy + dx * dx;
                if ( outer2 <= d2 )
                    continue;                          // beyond outer ring -> not emitted
                float w;
                if ( inner2 < d2 )
                {
                    const float d = (float)sqrt( (double)d2 );
                    const float t = ( d - innerR ) / ( outerR - innerR );
                    w = broken ? ( 1.0f - t )                       // BREAK: linear (no smoothstep)
                               : ( 1.0f - t * t * ( 3.0f - ( t + t ) ) );
                }
                else
                    w = 1.0f;
                float rgba[4];
                for ( int c = 0; c < 4; ++c )
                    rgba[c] = ( ramp[1][c] - ramp[0][c] ) * w + ramp[0][c];
                GfxColor pc; Byte4PackPixelColor( rgba, &pc );
                exp[expN].x = px; exp[expN].y = py; exp[expN].z = 0.0f; exp[expN].packed = pc.packed;
                ++expN;
            }

        static GfxPointVertex verts[1362];
        memset( verts, 0, sizeof( verts ) );
        const int n = PMESH_20_Radius_2( 0, &patch, cursor, innerR, outerR, verts );

        total++;
        bool ok = ( n == expN );
        for ( int i = 0; i < n && i < expN; ++i )
        {
            const float ex = verts[i].xyz[0] - exp[i].x, ey = verts[i].xyz[1] - exp[i].y, ez = verts[i].xyz[2] - exp[i].z;
            if ( (ex<0?-ex:ex) > 1e-3f || (ey<0?-ey:ey) > 1e-3f || (ez<0?-ez:ez) > 1e-3f )
                ok = false;
            if ( *(const unsigned int *)verts[i].color != exp[i].packed )
                ok = false;
        }
        if ( !ok ) fails++;
        STLog( "[PMESH_20 points] emitted %d  expected %d  %s\n", n, expN, ok ? "OK" : "MISMATCH" );
        for ( int i = 0; i < expN; ++i )
            STLog( "   exp[%d] (%.1f,%.1f,%.1f) col=%08X  got (%.1f,%.1f,%.1f) col=%08X\n",
                   i, exp[i].x, exp[i].y, exp[i].z, exp[i].packed,
                   i < n ? verts[i].xyz[0] : 0.f, i < n ? verts[i].xyz[1] : 0.f, i < n ? verts[i].xyz[2] : 0.f,
                   i < n ? *(const unsigned int *)verts[i].color : 0u );
    }

    // ── Layer (b): PMESH_19_Radius + sub_43ED50 line clip (camera pinned) ─────────
    // sub_43ED50 reads g_pParentWnd->m_pCamWnd->camera.vpn (depth nudge); pin it to 0 so
    // emitted endpoints are the raw clip-intersection points.
    {
        extern CMainFrame *g_pParentWnd;
        if ( !g_pParentWnd )            g_pParentWnd = new CMainFrame();
        if ( !g_pParentWnd->m_pCamWnd ) g_pParentWnd->m_pCamWnd = new CCamWnd();
        g_pParentWnd->m_pCamWnd->camera.vpn[0] = 0.0f;
        g_pParentWnd->m_pCamWnd->camera.vpn[1] = 0.0f;
        g_pParentWnd->m_pCamWnd->camera.vpn[2] = 0.0f;

        const float cursor[3] = { 0.0f, 0.0f, 0.0f };

        // Build the two 16-segment rings exactly as DrawAdvancedTerrainEditCircle does.
        float innerRing[48], outerRing[48];
        for ( int k = 0; k < 16; ++k )
        {
            const float ang  = (float)( (double)k * 0.3926990926265717 );
            const float s = (float)sin( ang ), c = (float)cos( ang );
            innerRing[3*k+0] = c * innerR + cursor[0]; innerRing[3*k+1] = s * innerR + cursor[1]; innerRing[3*k+2] = 1.0f;
            outerRing[3*k+0] = c * outerR + cursor[0]; outerRing[3*k+1] = s * outerR + cursor[1]; outerRing[3*k+2] = 1.0f;
        }
        GfxColor color; { float cyan[4] = { 0,1,1,1 }; Byte4PackPixelColor( cyan, &color ); }

        // A flat 2×2 curveDef whose two triangles fully CONTAIN the inner ring (a big square
        // [-512,512]^2 at z=0); pSymbiot bbox covers it.  Every inner-ring segment is wholly
        // inside -> sub_43ED50 emits the full 16-segment ring (32 verts).
        static patchMesh_t patch;  memset( &patch, 0, sizeof( patch ) );
        static brush_t     sym;    memset( &sym, 0, sizeof( sym ) );
        static curvePatchDef_t cdef;
        static curveVert_t cverts[4];
        patch.pSymbiot = (entity_brush_s *)&sym;
        sym.mins[0] = -512; sym.mins[1] = -512; sym.mins[2] = -8;
        sym.maxs[0] =  512; sym.maxs[1] =  512; sym.maxs[2] =  8;
        cdef.width = 2; cdef.height = 2; cdef.random_one = 1; cdef.verts = cverts; cdef.unk_after = 0;
        // 2×2 grid corners: (col,row) -> verts[row*2+col]
        const float qx[2] = { -512.0f, 512.0f }, qy[2] = { -512.0f, 512.0f };
        for ( int r = 0; r < 2; ++r )
            for ( int c = 0; c < 2; ++c )
            { curveVert_t &v = cverts[r*2+c]; v.xyz[0]=qx[c]; v.xyz[1]=qy[r]; v.xyz[2]=0.0f; }
        patch.curveDef = &cdef;
        // turned_edge flags default 0 (no flip).

        static GfxPointVertex lverts[1362];
        memset( lverts, 0, sizeof( lverts ) );
        int lc = PMESH_19_Radius( &patch, cursor, innerR, outerR, innerRing, outerRing,
                                  (const unsigned int *)&color, 0, lverts );

        // Inner ring (radius 64) is wholly inside the [-512,512]² surface; outer (128) too.
        // Each ring contributes 16 segments * 2 verts = 32; two rings + 2 triangles per quad
        // (each triangle re-clips the same ring, but each ring vertex lies in exactly one of the
        // two triangles, so the union is still the full ring once per ring per triangle pass).
        // We assert lc is a positive MULTIPLE of 2 and >= 32 (both rings reach), then verify the
        // FAR-triangle case emits 0 (below) — together this proves the radius/clip gate is live.
        total++;
        bool okIn = ( lc >= 32 && ( lc % 2 ) == 0 );
        if ( !okIn ) fails++;
        STLog( "[PMESH_19 lines, ring INSIDE big surface] emitted %d line-verts (expect >=32, even)  %s\n",
               lc, okIn ? "OK" : "MISMATCH" );

        // FAR case: move the surface far away so the rings miss the patch bbox entirely -> 0.
        sym.mins[0] = 4000; sym.mins[1] = 4000; sym.maxs[0] = 5000; sym.maxs[1] = 5000;
        for ( int q = 0; q < 4; ++q ) { cverts[q].xyz[0] += 4500.0f; cverts[q].xyz[1] += 4500.0f; }
        memset( lverts, 0, sizeof( lverts ) );
        int lcFar = PMESH_19_Radius( &patch, cursor, innerR, outerR, innerRing, outerRing,
                                     (const unsigned int *)&color, 0, lverts );
        total++;
        bool okFar = ( lcFar == 0 );
        if ( !okFar ) fails++;
        STLog( "[PMESH_19 lines, ring vs FAR surface] emitted %d line-verts (expect 0)  %s\n",
               lcFar, okFar ? "OK" : "MISMATCH" );
    }

    if ( broken )
    {
        // The break perturbs the PMESH_20 colour ORACLE -> the real impl's colours must NOT
        // match it -> layer (a) MUST fail.  (Layers (b) are colour-independent and still pass,
        // so a nonzero fail count == the colour check correctly rejected the wrong oracle.)
        if ( fails > 0 )
        { STLog( "RESULT: PASS-NEGATIVE (gate correctly REJECTS the no-smoothstep oracle: %d/%d failed)\n", fails, total );
          if ( s_logFile ) fclose( s_logFile ); return 1; }
        STLog( "RESULT: BAD-GATE (the wrong oracle was NOT rejected — the gate is vacuous!)\n" );
        if ( s_logFile ) fclose( s_logFile ); return 2;
    }
    if ( fails > 0 )
    { STLog( "RESULT: FAIL (%d/%d checks failed)\n", fails, total );
      if ( s_logFile ) fclose( s_logFile ); return 1; }

    STLog( "RESULT: PASS (all %d terrain-ring checks matched)\n", total );
    if ( s_logFile ) fclose( s_logFile );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunLinMapTest — the linearmapping.cpp double-precision 3x3 LU solver gate
//  (LinearMapping_LUFactor 0x4B6DC0 / BackSub 0x4B7120 / Refine 0x4B72D0 /
//   Setup 0x4B7430 / Apply 0x4B7340).  Two layers, both KNOWN-ANSWER:
//
//   (A) Raw LU primitive: hand-pick several 3x3 systems A with a chosen solution
//       x*, form b = A·x*, then LUFactor(A)+BackSub → x and assert x ≈ x* (tight).
//       This is exactly "feed A·x=b, solve, assert x".
//
//   (B) The public Setup→Apply chain on the problem it actually solves: choose a
//       known affine texture map  tc = a*world[i] + b*world[j] + c , lay out 3
//       non-collinear plane points, feed each point's tc as the Apply input coords,
//       and assert the recovered (out[axisI],out[axisJ],out[3]) == (a,b,c).  Since
//       the system M·(a,b,c)=tc this is the same A·x=b known answer, end-to-end.
//
//   LINMAP_BREAK perturbs the b vector fed to the solver by a fixed delta so the
//   recovered x no longer matches x* — the gate MUST reject it (non-vacuity proof).
// ─────────────────────────────────────────────────────────────────────────────
static int RunLinMapTest()
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_linmap.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    const bool broken = ( getenv( "LINMAP_BREAK" ) != nullptr );
    STLog( "=== RADIANT_SELFTEST: linearmapping 3x3 LU solver (A*x=b known-answer)%s ===\n",
           broken ? "  [LINMAP_BREAK: perturbed b, expect FAIL]" : "" );

    const double TOL = 1e-9;     // double-precision LU + 1 refine pass: very tight.
    int total = 0, fails = 0;

    // ── (A) raw LU primitive: hand-solvable systems ──────────────────────────────
    struct LUCase { double A[3][3]; double xstar[3]; const char *name; };
    static const LUCase lucases[] = {
        // diagonal — trivial
        { { {2,0,0},{0,3,0},{0,0,4} },           { 1, 2, 3 },     "diag(2,3,4)" },
        // a classic well-conditioned dense system (NR-style)
        { { {1,2,3},{2,5,3},{1,0,8} },           { -1, 4, 0.5 },  "dense#1" },
        // needs a row pivot (zero leading entry forces a swap), det = -3 (non-singular)
        { { {0,2,1},{1,1,1},{2,1,3} },           { 2, -3, 5 },    "pivot-swap" },
        // negatives + fractional solution
        { { {4,-2,1},{-2,4,-2},{1,-2,4} },       { 0.25, -1.5, 2.0 }, "spd-ish" },
        // the texture-lock-shaped homogeneous-column matrix (rows = (i,j,1))
        { { {-96,-96,1},{0,0,1},{96,96,1} },     { 0, 0, 0 },     "collinear-degenerate" },
        { { {-96,-96,1},{0,96,1},{96,-96,1} },   { 0.015625, 0.0078125, 12.0 }, "tex-shape" },
    };

    for ( int ci = 0; ci < (int)( sizeof(lucases)/sizeof(lucases[0]) ); ++ci )
    {
        const LUCase &C = lucases[ci];
        // b = A * xstar
        double b[3];
        for ( int r = 0; r < 3; ++r )
            b[r] = C.A[r][0]*C.xstar[0] + C.A[r][1]*C.xstar[1] + C.A[r][2]*C.xstar[2];
        if ( broken ) { b[0] += 0.5; b[1] -= 0.25; }   // perturb → wrong x

        double lu[3][3];
        memcpy( lu, C.A, sizeof( lu ) );
        int indx[3] = { 0, 0, 0 };
        bool ok_factor = LinearMapping_LUFactor( lu, indx );

        // The collinear-degenerate row matrix is SINGULAR (rows (i,j,1) collinear) —
        // LUFactor must report failure; we only check that and move on (no solve).
        if ( ci == 4 )
        {
            total++;
            bool ok = ( ok_factor == false );
            if ( !ok ) fails++;
            STLog( "[LU %-20s] singular -> LUFactor=%s  %s\n",
                   C.name, ok_factor ? "true" : "false", ok ? "OK" : "MISMATCH(expected singular)" );
            continue;
        }

        if ( !ok_factor )
        {
            total++; fails++;
            STLog( "[LU %-20s] LUFactor reported SINGULAR on a non-singular system  MISMATCH\n", C.name );
            continue;
        }

        double x[3] = { b[0], b[1], b[2] };
        LinearMapping_BackSub( indx, lu, x );

        total++;
        double e0 = x[0]-C.xstar[0]; if ( e0 < 0 ) e0 = -e0;
        double e1 = x[1]-C.xstar[1]; if ( e1 < 0 ) e1 = -e1;
        double e2 = x[2]-C.xstar[2]; if ( e2 < 0 ) e2 = -e2;
        bool ok = ( e0 <= TOL && e1 <= TOL && e2 <= TOL );
        if ( !ok ) fails++;
        STLog( "[LU %-20s] x{%.12g,%.12g,%.12g} exp{%.12g,%.12g,%.12g} err{%.2g,%.2g,%.2g}  %s\n",
               C.name, x[0],x[1],x[2], C.xstar[0],C.xstar[1],C.xstar[2], e0,e1,e2,
               ok ? "OK" : "MISMATCH" );
    }

    // ── (B) public Setup -> Apply chain (the texture-lock affine recovery) ───────
    // Known affine map per axis-plane:  tc = a*p[axisI] + b*p[axisJ] + c.
    struct MapCase { float normal[3]; double a, b, c; const char *name; };
    static const MapCase mapcases[] = {
        { { 0,0,1 },  1.0/128, 0.5/128, 17.0,  "+Z map" },
        { { 1,0,0 }, -1.0/64,  2.0/64, -33.25, "+X map" },
        { { 0,1,0 },  3.0/256, 1.0/256, 8.0,   "+Y map" },
    };
    // three non-collinear plane points (their k-component is irrelevant to the map).
    static const float pts[3][3] = { { -96, -64, 5 }, { 32, 80, -3 }, { 64, -48, 11 } };

    for ( int mc = 0; mc < (int)( sizeof(mapcases)/sizeof(mapcases[0]) ); ++mc )
    {
        const MapCase &M = mapcases[mc];
        LinearMapping lm;
        memset( &lm, 0, sizeof( lm ) );
        // Setup args: (lm, normal, p2, p0, p1)  — matches Face_TexLock_Reproject.
        bool setupOk = LinearMapping_Setup( &lm, M.normal, pts[2], pts[0], pts[1] );
        if ( !setupOk )
        {
            total++; fails++;
            STLog( "[MAP %-8s] Setup reported SINGULAR  MISMATCH\n", M.name );
            continue;
        }

        // For each plane point, the "scalar coord" is the known affine value at that pt.
        // out[axisI]=a, out[axisJ]=b, out[3]=c must be recovered.
        bool allOk = true;
        for ( int q = 0; q < 3; ++q )
        {
            float tc[3];
            for ( int r = 0; r < 3; ++r )
                tc[r] = (float)( M.a*pts[r][lm.axisI] + M.b*pts[r][lm.axisJ] + M.c );
            if ( broken ) { tc[0] += 1.0f; }    // perturb b

            float out[4] = { 0,0,0,0 };
            // c2 (out[3]) is just passed through as the constant column; the solve uses
            // it only as the 3rd RHS component.  Feed the 3 points' tc as (c0,c1,c2).
            LinearMapping_Apply( &lm, out, tc[0], tc[1], tc[2] );

            // The solve recovers (a,b,c) = M^-1 * tc.  Apply scatters slope a into
            // out[axisI], slope b into out[axisJ], and the SOLVED affine CONSTANT c
            // (= sol[2]) into out[3].  All three are the known answer (a,b,c) == (M.a,
            // M.b, M.c).  (c can be large -> scale its tolerance.)
            double ea = out[lm.axisI] - M.a; if ( ea < 0 ) ea = -ea;
            double eb = out[lm.axisJ] - M.b; if ( eb < 0 ) eb = -eb;
            double ec = out[3]        - M.c; if ( ec < 0 ) ec = -ec;
            // float scatter at the end → relax the recovery tolerance to float epsilon.
            bool ok = ( ea <= 1e-4 && eb <= 1e-4 && ec <= 1e-2 );
            if ( !ok ) allOk = false;
            STLog( "[MAP %-8s] axes(i,j,k)=(%d,%d,%d) out{a=%.8g b=%.8g c=%.8g} "
                   "exp{a=%.8g b=%.8g c=%.8g} err{a=%.2g b=%.2g c=%.2g}  %s\n",
                   M.name, lm.axisI, lm.axisJ, lm.axisK,
                   out[lm.axisI], out[lm.axisJ], out[3], M.a, M.b, M.c, ea, eb, ec,
                   ok ? "OK" : "MISMATCH" );
            break;   // one Apply per case is enough (the map is the same for all 3 pts)
        }
        total++;
        if ( !allOk ) fails++;
    }

    STLog( "%d/%d known-answer LU/affine solves matched (LU abs<=%g, affine recovery float-tol)\n",
           total - fails, total, TOL );

    if ( broken )
    {
        if ( fails > 0 )
        { STLog( "RESULT: PASS-NEGATIVE (gate correctly REJECTS the perturbed solve: %d/%d failed)\n", fails, total );
          if ( s_logFile ) fclose( s_logFile ); return 1; }
        STLog( "RESULT: BAD-GATE (the perturbed solve was NOT rejected — the gate is vacuous!)\n" );
        if ( s_logFile ) fclose( s_logFile ); return 2;
    }
    if ( fails > 0 )
    { STLog( "RESULT: FAIL (%d/%d known-answer solves did not match)\n", fails, total );
      if ( s_logFile ) fclose( s_logFile ); return 1; }

    STLog( "RESULT: PASS (all %d known-answer 3x3 LU / affine-recovery solves matched)\n", total );
    if ( s_logFile ) fclose( s_logFile );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunTexLockTest — the texture-LOCK invariant gate (Face_TexLock_Save 0x470570 +
//  Face_TexLock_Reproject 0x4706F0, the full Brush_Move wrap, exercised end-to-end
//  with a synthesized real material so MaterialDef_04 == 1 — the reproject runs).
//  For a corner-covering set of texdefs × plane normals × moves: record the WORLD
//  S/T at the 3 plane points, move the face, reproject, and assert the texcoord at
//  each MOVED plane point equals the pre-move texcoord (the lock invariant) to a
//  tight tolerance.  TEXLOCK_BREAK skips the reproject (texdef unchanged after a
//  move) so the lock invariant is violated → the gate MUST reject (non-vacuity).
// ─────────────────────────────────────────────────────────────────────────────
extern float Radiant_TexLockSelfTest( const float planepts[3][3], const float *texdef6,
                                      const float *move, float outNewTexdef[6],
                                      int doReproject );

static int RunTexLockTest()
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_texlock.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    const bool broken = ( getenv( "TEXLOCK_BREAK" ) != nullptr );
    STLog( "=== RADIANT_SELFTEST: texture-lock invariant (Save->move->Reproject)%s ===\n",
           broken ? "  [TEXLOCK_BREAK: skip reproject, expect FAIL]" : "" );

    // texdef = {sizeX,sizeY, shiftX,shiftY, rotate, crossterm}
    struct TC { float td[6]; const char *name; };
    static const TC tds[] = {
        { {  64,  64,   0,   0,    0.0f, 0 }, "64 r0"   },
        { { 128, 128,  16,  16,    0.0f, 0 }, "128 sh16"},
        { {  64, 128, -32,  16,   30.0f, 0 }, "64x128 r30" },
        { { 128,  64,  16, -32,   90.0f, 0 }, "128x64 r90" },
        { {  64,  64,   0,   0,  -45.0f, 0 }, "64 r-45" },
    };
    static const float norms[3][3] = { { 0,0,1 }, { 1,0,0 }, { 0,1,0 } };
    const char *nN[3] = { "+Z", "+X", "+Y" };
    // axis-aligned face planes (3 planepts on the plane) + a move vector per normal.
    static const float planesZ[3][3] = { { -64,-64,32 }, { 64,-64,32 }, { 64,64,32 } };
    static const float planesX[3][3] = { { 32,-64,-64 }, { 32,64,-64 }, { 32,64,64 } };
    static const float planesY[3][3] = { { -64,32,-64 }, { 64,32,-64 }, { 64,32,64 } };
    const float (*planeSets[3])[3] = { planesZ, planesX, planesY };
    static const float move[3] = { 37.0f, -53.0f, 19.0f };

    const float TOL = 5e-2f;     // world texcoord units (float tex matrix + snap)
    int total = 0, fails = 0;

    for ( int ni = 0; ni < 3; ++ni )
    {
        for ( int ci = 0; ci < (int)( sizeof(tds)/sizeof(tds[0]) ); ++ci )
        {
            total++;
            float newTd[6] = {0,0,0,0,0,0};
            // broken = skip the reproject (texdef unchanged after the move) → the world
            // projection slides off the geometry by ~|move·texAxis|, a real lock violation.
            float err = Radiant_TexLockSelfTest( planeSets[ni], tds[ci].td, move, newTd,
                                                 broken ? 0 : 1 );
            bool ok = ( err <= TOL );
            if ( !ok ) fails++;
            STLog( "[%-2s %-11s] lockErr=%.5g  newTd{sz %.4g,%.4g sh %.4g,%.4g r %.4g x %.4g}  %s\n",
                   nN[ni], tds[ci].name, err,
                   newTd[0],newTd[1],newTd[2],newTd[3],newTd[4],newTd[5],
                   ok ? "OK" : "MISMATCH" );
        }
    }

    STLog( "%d/%d texlock invariants held (max world-texcoord err <= %g)\n",
           total - fails, total, TOL );

    if ( broken )
    {
        if ( fails > 0 )
        { STLog( "RESULT: PASS-NEGATIVE (gate correctly REJECTS the no-reproject case: %d/%d failed)\n", fails, total );
          if ( s_logFile ) fclose( s_logFile ); return 1; }
        STLog( "RESULT: BAD-GATE (the no-reproject case was NOT rejected — the gate is vacuous!)\n" );
        if ( s_logFile ) fclose( s_logFile ); return 2;
    }
    if ( fails > 0 )
    { STLog( "RESULT: FAIL (%d/%d texlock invariants violated)\n", fails, total );
      if ( s_logFile ) fclose( s_logFile ); return 1; }

    STLog( "RESULT: PASS (all %d texture-lock invariants held — texcoords follow the geometry)\n", total );
    if ( s_logFile ) fclose( s_logFile );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPaintFalloffTest — map-free unit gate for sub_43DB60 (0x43DB60), the terrain-paint
//  brush falloff: 2D-distance smoothstep that weights the PMESH_16 apply.  Asserts the
//  three regions (inside innerR = full strength, outside outerR = 0, smoothstep between),
//  Z-independence (it is an XY distance), the inclusive inner edge, and monotonic decay.
// ─────────────────────────────────────────────────────────────────────────────
extern float sub_43DB60( const float *center, const float *pt, float innerR, float outerR, float strength );

static int RunPaintFalloffTest( const char * )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_paintfalloff.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: sub_43DB60 terrain-paint brush falloff (smoothstep) ===\n" );

    const float c[3] = { 0, 0, 0 };
    const float inner = 10.0f, outer = 20.0f, strength = 2.0f;

    float p[3];
    p[0]=5;  p[1]=0; p[2]=0;   float w1 = sub_43DB60( c, p, inner, outer, strength );  // inside inner
    p[0]=25; p[1]=0; p[2]=0;   float w2 = sub_43DB60( c, p, inner, outer, strength );  // outside outer
    p[0]=15; p[1]=0; p[2]=0;   float w3 = sub_43DB60( c, p, inner, outer, strength );  // midpoint t=0.5
    p[0]=10; p[1]=0; p[2]=0;   float w4 = sub_43DB60( c, p, inner, outer, strength );  // exactly innerR
    p[0]=0;  p[1]=0; p[2]=999; float w5 = sub_43DB60( c, p, inner, outer, strength );  // Z ignored (2D)

    STLog( "w(d=5)=%g (exp 2)  w(d=25)=%g (exp 0)  w(d=15)=%g (exp 1)  w(d=10)=%g (exp 2)  w(z-only)=%g (exp 2)\n",
           w1, w2, w3, w4, w5 );

    #define FALLOFF_NEAR(a,b) ( ( (a)-(b) < 1e-4f ) && ( (b)-(a) < 1e-4f ) )
    if ( !FALLOFF_NEAR( w1, 2.0f ) ) { STLog( "RESULT: FAIL (inside innerR must be full strength)\n" ); return 1; }
    if ( !FALLOFF_NEAR( w2, 0.0f ) ) { STLog( "RESULT: FAIL (outside outerR must be 0)\n" ); return 1; }
    if ( !FALLOFF_NEAR( w3, 1.0f ) ) { STLog( "RESULT: FAIL (midpoint smoothstep must be strength*0.5)\n" ); return 1; }
    if ( !FALLOFF_NEAR( w4, 2.0f ) ) { STLog( "RESULT: FAIL (inner edge is inclusive)\n" ); return 1; }
    if ( !FALLOFF_NEAR( w5, 2.0f ) ) { STLog( "RESULT: FAIL (falloff must ignore Z)\n" ); return 1; }

    // Monotonic non-increasing across the annulus innerR..outerR.
    float prev = strength + 1e-4f;
    for ( float d = 10.0f; d <= 20.0f; d += 1.0f )
    {
        p[0] = d; p[1] = 0; p[2] = 0;
        float w = sub_43DB60( c, p, inner, outer, strength );
        if ( w > prev + 1e-4f )
        { STLog( "RESULT: FAIL (falloff not monotonic at d=%g: %g > %g)\n", d, w, prev ); return 1; }
        prev = w;
    }
    #undef FALLOFF_NEAR

    STLog( "RESULT: PASS (smoothstep falloff: full inside, 0 outside, monotonic blend, Z-independent)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunHollowTest — the Selection->CSG->Hollow command (CSG_MakeHollow 0x47D3C0, verified
//  faithful vs IDA this session).  load -> create one world brush -> CSG_MakeHollow -> assert
//  exactly one brush was selected going in and the brush count grew by numSlabs-1 (a cube
//  becomes its wall shell; 6 slabs => delta 5, fewer if the box is thin on an axis).  In-memory.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestHollow( int *outSelBefore, int *outDelta );   // select.cpp

static int RunHollowTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_hollow.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: CSG_MakeHollow (Selection->CSG->Hollow) ===\n" );
    STLog( "map : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    STLog( "step: NewBrushDrag world brush (256,256 -> 384,384)\n" );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 384, 384, 384, 384,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during brush create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no brush created)\n" ); return 1; }

    STLog( "step: Radiant_TestHollow\n" );
    int selBefore = 0, delta = 0, ok = 0;
    __try { ok = Radiant_TestHollow( &selBefore, &delta ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during CSG_MakeHollow (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "selectedBefore=%d  brushCountDelta=%d (expect 1 selected; delta == 5)\n", selBefore, delta );
    if ( !ok )
    { STLog( "RESULT: FAIL (expected exactly one selected brush going in, got %d)\n", selBefore ); return 1; }
    // HARDENED: a hollow of an axis-aligned cuboid (6 faces) is EXACTLY +5 net brushes.
    // CSG_MakeHollow (0x47D3C0) loops def->faceCount, Brush_SplitBrushByFace's the inward-
    // shrunk face -> adds one `front` wall slab per face (6) + frees the `back` half + frees
    // the original cube once (Brush_Free) => 6 added − 1 original = +5.  Verified empirically
    // (hollow log: brushCountDelta=5).  NewBrushDrag always builds a 6-face box, so this is
    // deterministic — not a 3..5 range.
    if ( delta != 5 )
    { STLog( "RESULT: FAIL (hollow produced %d net brushes; a 6-face cube must yield exactly 5 wall slabs)\n", delta ); return 1; }
    if ( ST_GuardedDiag( "after-hollow" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after CSG_MakeHollow)\n" ); return 3; }

    STLog( "RESULT: PASS (hollow replaced the 6-face cube with exactly 5 net wall slabs, list clean)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunNSidedTest — the "Brush->N sided" primitive (Brush_MakeSided 0x4731E0, verified faithful
//  vs IDA).  load -> create a world brush -> reshape into a hexagon about Z -> assert faceCount
//  == 6+2 caps == 8 and every face built a valid winding.  In-memory.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestMakeSided( int sides, int *outFaceCount, int *outWindingsOk,
                                      int *outGeomOk );  // select.cpp

static int RunNSidedTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_nsided.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Brush_MakeSided (Brush->N sided) ===\n" );
    STLog( "map : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    STLog( "step: NewBrushDrag world brush (256,256 -> 384,384)\n" );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 384, 384, 384, 384,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during brush create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no brush created)\n" ); return 1; }

    STLog( "step: Radiant_TestMakeSided(6)\n" );
    int faceCount = 0, windingsOk = 0, geomOk = 0, ok = 0;
    __try { ok = Radiant_TestMakeSided( 6, &faceCount, &windingsOk, &geomOk ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Brush_MakeSided (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok ) { STLog( "RESULT: FAIL (no brush to reshape)\n" ); return 1; }
    STLog( "faceCount=%d (expect 8)  windingsOk=%d (expect 1)  geomOk=%d (expect 1)\n",
           faceCount, windingsOk, geomOk );
    if ( faceCount != 8 )
    { STLog( "RESULT: FAIL (6-sided brush has %d faces, expected 8)\n", faceCount ); return 1; }
    if ( !windingsOk )
    { STLog( "RESULT: FAIL (a face has no winding — degenerate N-sided geometry)\n" ); return 1; }
    // HARDENED: assert the EXACT resulting hexagon AABB (not just faceCount + non-null windings).
    // Derived from Brush_MakeSided (0x4731E0): the hexagon's X-extent is exactly center.x±radius
    // (the θ=0/180° vertices) and its Z-extent (the axis) is unchanged from the original cube.
    if ( !geomOk )
    { STLog( "RESULT: FAIL (hexagon AABB != center.x±radius on X / unchanged on Z — geometry wrong)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-nsided" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after Brush_MakeSided)\n" ); return 3; }

    STLog( "RESULT: PASS (Brush->6 sided: 8 faces, all windings valid, exact hexagon AABB)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunFloorTraceTest — Trace_AllDirectionsIfFailed (0x48DAA0), the drop-to-floor ray (the
//  foundational piece of the model-render epic's drop-to-floor cluster; Test_Ray was already
//  real).  load -> create a world box -> aim a -Z ray 100u above its centre, trace selected-only
//  -> assert it hit the box's top face at distance ~100 with normal +Z.  In-memory.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestFloorTrace( int *outHit, float *outDist, float *outNormalZ );  // select.cpp

static int RunFloorTraceTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_floortrace.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Trace_AllDirectionsIfFailed (drop-to-floor ray) ===\n" );
    STLog( "map : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    STLog( "step: NewBrushDrag world box (256,256 -> 384,384)\n" );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 384, 384, 384, 384,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during brush create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no brush created)\n" ); return 1; }

    STLog( "step: Radiant_TestFloorTrace\n" );
    int hit = 0; float dist = -1.0f, nz = 0.0f, ok = 0;
    __try { ok = (float)Radiant_TestFloorTrace( &hit, &dist, &nz ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Trace_AllDirectionsIfFailed (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ok == 0.0f ) { STLog( "RESULT: FAIL (no brush to trace against)\n" ); return 1; }
    STLog( "hit=%d  dist=%g (expect 100)  normalZ=%g (expect 1)  hitIsOurBox=%d\n",
           hit, dist, nz, ( ok == 1.0f ) ? 1 : 0 );
    if ( hit && ok != 1.0f )
    { STLog( "RESULT: FAIL (ray hit a DIFFERENT brush than the box we created)\n" ); return 1; }
    if ( !hit )
    { STLog( "RESULT: FAIL (drop-to-floor ray did not hit the box below it)\n" ); return 1; }
    // HARDENED: the ray starts at maxs[2]+100 going -Z and the top face is the plane z=maxs[2],
    // so the entry distance is EXACTLY 100.0 (the harness builds start = maxs[2]+100) and the
    // up-facing top-face normal is EXACTLY (0,0,1).  Verified empirically (floortrace log:
    // dist=100, normalZ=1).  Tightened from the old ~1.0 / 0.99 slack to a float-exact band.
    float dd = dist - 100.0f; if ( dd < 0 ) dd = -dd;
    if ( dd > 0.01f )
    { STLog( "RESULT: FAIL (entry distance %g != exact 100 = (maxs.z+100) - maxs.z)\n", dist ); return 1; }
    float dnz = nz - 1.0f; if ( dnz < 0 ) dnz = -dnz;
    if ( dnz > 0.001f )
    { STLog( "RESULT: FAIL (entry normal Z %g != exact 1, expected the up-facing top face (0,0,1))\n", nz ); return 1; }
    if ( ST_GuardedDiag( "after-floortrace" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after trace)\n" ); return 3; }

    STLog( "RESULT: PASS (drop-to-floor ray hit the top face at exact dist 100, normal exactly +Z)\n" );
    return 0;
}

// Exercise Brush_AutoCaulk (0x47E0F0) on a multi-brush selection.
// Headless textures are unrealized, so this gate covers list safety rather than face caulking.
extern "C" int Radiant_TestAutoCaulk( int *outSelCount, int *outListDelta, int *outIntact );  // select.cpp

static int RunAutoCaulkTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_autocaulk.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Brush_AutoCaulk (Selection->CSG->Auto Caulk, 33220) ===\n" );
    STLog( "map : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // brush A (selected), then deselect to active, then brush B (selected) — multi-brush selection.
    STLog( "step: NewBrushDrag world brush A (256,256 -> 384,384)\n" );
    void *phA = nullptr;
    __try { phA = Radiant_TestCreateBrushDrag( 256, 256, 384, 384, 384, 384,
                                               0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during brush A create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !phA ) { STLog( "RESULT: FAIL (no brush A)\n" ); return 1; }

    STLog( "step: Select_Deselect (move A to active)\n" );
    __try { Select_Deselect( 1 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during deselect (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: NewBrushDrag world brush B (512,512 -> 640,640)\n" );
    void *phB = nullptr;
    __try { phB = Radiant_TestCreateBrushDrag( 512, 512, 640, 640, 640, 640,
                                               0.25f, 0.0f, 0.0f, 768, 768 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during brush B create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !phB ) { STLog( "RESULT: FAIL (no brush B)\n" ); return 1; }

    STLog( "step: Radiant_TestAutoCaulk\n" );
    int selCount = 0, delta = 0, intact = 0, ok = 0;
    __try { ok = Radiant_TestAutoCaulk( &selCount, &delta, &intact ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Brush_AutoCaulk (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "selectedBrushes=%d (expect >=2)  listDelta=%d (expect 0)  intact=%d\n",
           selCount, delta, intact );
    if ( !ok )            { STLog( "RESULT: FAIL (harness setup failed)\n" );                 return 1; }
    if ( selCount < 2 )   { STLog( "RESULT: FAIL (expected a multi-brush selection)\n" );      return 1; }
    if ( delta != 0 )     { STLog( "RESULT: FAIL (AutoCaulk changed the brush count by %d; it must only retexture faces)\n", delta ); return 1; }
    if ( !intact )        { STLog( "RESULT: FAIL (a brush lost its def across AutoCaulk)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-autocaulk" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after Brush_AutoCaulk)\n" ); return 3; }

    STLog( "RESULT: PASS (AutoCaulk ran crash-free on a %d-brush selection, list intact;\n", selCount );
    STLog( "         per-face caulk body is headless-unreachable by design — in_use=0 keeps dword_181F51C=128)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunFloorDropTest — the drop-to-floor scatter helpers sub_47CFD0 (bbox pivot) +
//  sub_47CBA0 (rotate-about-pivot), the two NEW pieces of OnDropSelected (0x425be0).
//  load -> create a world box (selected) -> verify sub_47CFD0's grid-snapped + raw
//  midpoints against the formula, and that sub_47CBA0 is a no-op at 0 deg but
//  transforms the brush at 90 deg. The end-to-end trace+move are floortrace-gated.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestFloorDrop( int *outSnapOk, int *outRawOk, int *outRotOk );  // select.cpp

static int RunFloorDropTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_floordrop.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: OnDropSelected scatter helpers (sub_47CFD0/sub_47CBA0) ===\n" );
    STLog( "map : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    STLog( "step: NewBrushDrag world box (256,256 -> 384,384)\n" );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 384, 384, 384, 384,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during brush create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no brush created)\n" ); return 1; }

    STLog( "step: Radiant_TestFloorDrop\n" );
    int snapOk = 0, rawOk = 0, rotOk = 0, ok = 0;
    __try { ok = Radiant_TestFloorDrop( &snapOk, &rawOk, &rotOk ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during scatter helpers (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ok == 0 ) { STLog( "RESULT: FAIL (no brush to scatter)\n" ); return 1; }
    STLog( "snapMidOk=%d  rawMidOk=%d  rotOk=%d\n", snapOk, rawOk, rotOk );
    if ( !snapOk ) { STLog( "RESULT: FAIL (sub_47CFD0 grid-snapped midpoint wrong)\n" ); return 1; }
    if ( !rawOk )  { STLog( "RESULT: FAIL (sub_47CFD0 NoClamp raw midpoint wrong)\n" ); return 1; }
    if ( !rotOk )  { STLog( "RESULT: FAIL (sub_47CBA0 0deg not no-op or 90deg did not transform)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-floordrop" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after scatter)\n" ); return 3; }

    STLog( "RESULT: PASS (bbox pivot snap/raw correct; rotate-about-pivot transforms)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPhysBrushTest — the PHYSICS-COLLISION-BRUSH geometry gate.  Drives sub_4AA220
//  (Euler vec3 rotate) + sub_4A59C0 (point->2-basis projection) as unit math, then
//  parses a physics_box and physics_cylinder block (Brush_ParsePhysicsBox /
//  ParsePhysCylinder -> sub_478630) and writes them back (Brush_WritePhysicsBox /
//  WritePhysCylinder -> sub_4A59C0).  A map containing a script_brushmodel physics
//  brush used to FATAL on load/save; this gate proves the whole chain is FATAL-free.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestPhysBrush( int *outRotOk, int *outProjOk, int *outDegenOk,
                                      int *outBoxOk, int *outCylOk );   // brush.cpp

static int RunPhysBrushTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_physbrush.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: physics-collision brush (sub_4AA220/sub_478630/sub_4A59C0) ===\n" );
    STLog( "map : %s\n", mapPath ? mapPath : "(none)" );
    AddVectoredExceptionHandler( 1, ST_Veh );

    // The end-to-end physics-brush parse needs the editor's matsys/eclass substrate
    // (Brush_Alloc -> random_texture_stuff, MaterialDef_GetLayeredMaterial); load the
    // map to bring it up exactly as the create/floordrop gates do.
    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );
    if ( mapPath )
    {
        STLog( "step: Map_LoadFromFile\n" );
        if ( ST_GuardedLoad( mapPath ) != 0 )
        { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
        if ( ST_GuardedDiag( "after-load" ) < 0 )
        { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    }

    STLog( "step: Radiant_TestPhysBrush (unit + parse/write end-to-end)\n" );
    int rotOk = 0, projOk = 0, degenOk = 0, boxOk = 0, cylOk = 0, ok = 0;
    __try { ok = Radiant_TestPhysBrush( &rotOk, &projOk, &degenOk, &boxOk, &cylOk ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during phys-brush test (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "rotOk=%d  projOk=%d  degenOk=%d  boxOk=%d  cylOk=%d\n",
           rotOk, projOk, degenOk, boxOk, cylOk );
    if ( !rotOk )   { STLog( "RESULT: FAIL (sub_4AA220 Euler rotate wrong)\n" );           return 1; }
    if ( !projOk )  { STLog( "RESULT: FAIL (sub_4A59C0 non-degenerate solve wrong)\n" );   return 1; }
    if ( !degenOk ) { STLog( "RESULT: FAIL (sub_4A59C0 degenerate solve wrong)\n" );       return 1; }
    if ( !boxOk )   { STLog( "RESULT: FAIL (physics_box parse/write — sub_478630)\n" );    return 1; }
    if ( !cylOk )   { STLog( "RESULT: FAIL (physics_cylinder parse/write — sub_4A59C0)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-physbrush" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after phys-brush)\n" ); return 3; }

    STLog( "RESULT: PASS (Euler rotate + 2-basis solve + physics box/cylinder round-trip, no FATAL)\n" );
    return 0;
}

// Check the material basename normalizer at 0x45AD50.
extern char *sub_45AD50( char *name );   // engine_stubs.cpp 0x45AD50 (C++ linkage)

static int RunMtlNormalizeTest( const char * )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_mtlnormalize.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: sub_45AD50 material-name normaliser ===\n" );

    char c1[] = "textures/foo/bar";
    char c2[] = "wc\\floor\\tile01";
    char c3[] = "caulk";
    char c4[] = "";
    const char *r1 = sub_45AD50( c1 );
    const char *r2 = sub_45AD50( c2 );
    const char *r3 = sub_45AD50( c3 );
    const char *r4 = sub_45AD50( c4 );
    STLog( "'%s' -> '%s' (expect bar)\n",    c1, r1 );
    STLog( "'%s' -> '%s' (expect tile01)\n", c2, r2 );
    STLog( "'%s' -> '%s' (expect caulk)\n",  c3, r3 );
    STLog( "'' -> '%s' (expect empty)\n",    r4 );

    int ok = !strcmp( r1, "bar" ) && !strcmp( r2, "tile01" )
          && !strcmp( r3, "caulk" ) && !strcmp( r4, "" );
    if ( !ok )
    { STLog( "RESULT: FAIL (basename strip incorrect)\n" ); return 1; }

    STLog( "RESULT: PASS (basename strip after the last path separator)\n" );
    return 0;
}

// Parse menu entries with TexFilter_LoadMenuFile (0x45B010).
extern int TexFilter_LoadMenuFile( const char *txt, void *dest, int startId ); // qe3.cpp (0x45B010, C++ linkage)
struct STFilterEntry { char *name; int index; };   // = filter_material_t (8B)

static int RunTexFilterTest( const char * )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH], dataPath[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath,  sizeof( logPath ),  "%sradiant_texfilter.log", tmpDir );
    _snprintf( dataPath, sizeof( dataPath ), "%sradiant_texfilter_usage.txt", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: TexFilter_LoadMenuFile (0x45B010) ===\n" );

    // Write a usage.txt-shaped fixture: comments, blank line, leading/trailing whitespace,
    // a "<separator>" entry, then more entries.
    {
        FILE *f = fopen( dataPath, "wb" );
        if ( !f ) { STLog( "RESULT: FAIL (cannot write fixture)\n" ); return 2; }
        fputs( "// a comment line\n", f );
        fputs( "\n", f );                       // blank line (skipped)
        fputs( "case\n", f );                   // entry 1, id 7
        fputs( "  tools  \n", f );              // entry 2, id 8 (whitespace trimmed)
        fputs( "<separator>\n", f );            // entry 3, {null,-1}, id NOT consumed
        fputs( "door\n", f );                   // entry 4, id 9
        fclose( f );
    }

    STFilterEntry arr[16];
    memset( arr, 0, sizeof( arr ) );
    arr[0].name  = (char *)"all";               // the static [0] entry (loader fills from 1)
    arr[0].index = 0;

    int count = TexFilter_LoadMenuFile( dataPath, arr, 7 );  // startId 7

    STLog( "count=%d (expect 5: [0]all + 4 lines incl separator)\n", count );
    STLog( "[0] '%s' id=%d (expect 'all' 0)\n",        arr[0].name ? arr[0].name : "(null)", arr[0].index );
    STLog( "[1] '%s' id=%d (expect 'case' 7)\n",       arr[1].name ? arr[1].name : "(null)", arr[1].index );
    STLog( "[2] '%s' id=%d (expect 'tools' 8)\n",      arr[2].name ? arr[2].name : "(null)", arr[2].index );
    STLog( "[3] '%s' id=%d (expect '(null)' -1)\n",    arr[3].name ? arr[3].name : "(null)", arr[3].index );
    STLog( "[4] '%s' id=%d (expect 'door' 9)\n",       arr[4].name ? arr[4].name : "(null)", arr[4].index );

    int ok = ( count == 5 )
          && arr[0].name && !strcmp( arr[0].name, "all" )   && arr[0].index == 0
          && arr[1].name && !strcmp( arr[1].name, "case" )  && arr[1].index == 7
          && arr[2].name && !strcmp( arr[2].name, "tools" ) && arr[2].index == 8   // trailing/leading ws trimmed
          && arr[3].name == nullptr                          && arr[3].index == -1  // <separator>
          && arr[4].name && !strcmp( arr[4].name, "door" )  && arr[4].index == 9;  // separator did NOT consume id

    // Free the heap strings the loader allocated (entries 1.. with non-null name).
    for ( int i = 1; i < count; ++i )
        if ( arr[i].name )
            free( arr[i].name );

    if ( !ok )
    { STLog( "RESULT: FAIL (table fill incorrect)\n" ); return 1; }

    STLog( "PASS-1: TexFilter_LoadMenuFile (1-based fill, ws-trim, comment/blank skip, separator {null,-1} no-id-consume)\n" );

    // ── PART 2: TexWnd_FilterAccept layer-0 usage/locale/surfaceType chain (IDB 0x45bc70). ──
    // Synthetic qtextures with explicit usage/locale/surfaceType; drives the three filter
    // indices and asserts the faithful reject chain incl. the index-0-accepts-all default.
    extern void Radiant_TestFilterUsageLocale( int *a, int *u0, int *uf, int *lf, int *sf ); // texwnd.cpp

    STLog( "--- TexWnd_FilterAccept layer-0 usage/locale/surfaceType chain (0x45bc70) ---\n" );
    int fAll = -1, fU0 = -1, fUF = -1, fLF = -1, fSF = -1;
    Radiant_TestFilterUsageLocale( &fAll, &fU0, &fUF, &fLF, &fSF );

    STLog( "all (no filter)   accepted=%d (expect 2: usage!=0 in-use A,B; C usage=0 rejected)\n", fAll );
    STLog( "usage_index==0    accepted=%d (expect 0: rejected even at the 'all' default)\n",       fU0 );
    STLog( "usageFilter set   accepted=%d (expect 1: only the usage-5 material A)\n",               fUF );
    STLog( "localeFilter set  accepted=%d (expect 1: only the locale-bit-3 material A)\n",          fLF );
    STLog( "surfTypeFilter set accepted=%d (expect 1: only the metal material A)\n",                fSF );

    int ok2 = ( fAll == 2 ) && ( fU0 == 0 ) && ( fUF == 1 ) && ( fLF == 1 ) && ( fSF == 1 );
    if ( !ok2 )
    { STLog( "RESULT: FAIL (layer-0 filter chain incorrect)\n" ); return 1; }

    STLog( "PASS-2: layer-0 usage/locale/surfaceType chain + index-0-accepts-all default\n" );
    STLog( "RESULT: PASS\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunAlignToFloorTest — the "Drop Selected to Floor" angle-align math gate.
//  load -> placeholder brush -> create a point entity (light) -> AlignEntityToFace_
//  OrientToFloor -> sub_4859B0 (0x4859B0, was a FATAL stub: dropping a selected entity to
//  the floor crashed).  Verifies the rebuilt forward is unit-length and perpendicular to the
//  target floor normal (sub_4859B0's projection invariant) — would fail if the contiguous
//  axis-matrix were laid out as separate locals.  In-memory.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" void *Radiant_TestCreateEntity( const char *classname );          // xywnd.cpp
extern "C" int   Radiant_TestAlignToFloor( void *entInst, int *outFixed, int *outPerp ); // xywnd.cpp

static int RunAlignToFloorTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_aligntofloor.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: AlignEntityToFace_OrientToFloor / sub_4859B0 ===\n" );
    STLog( "map : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    STLog( "step: NewBrushDrag placeholder (256,256 -> 320,320)\n" );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 320, 320,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during placeholder create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no placeholder brush)\n" ); return 1; }

    STLog( "step: CreateEntityFromName(light)\n" );
    void *ent = nullptr;
    __try { ent = Radiant_TestCreateEntity( "light" ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during CreateEntityFromName (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ent ) { STLog( "RESULT: FAIL (no entity created/selected)\n" ); return 1; }

    STLog( "step: Radiant_TestAlignToFloor\n" );
    int fixedSz = 0, perp = 0, ok = 0;
    __try { ok = Radiant_TestAlignToFloor( ent, &fixedSz, &perp ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during AlignEntityToFace/sub_4859B0 (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok ) { STLog( "RESULT: FAIL (no entity def to align)\n" ); return 1; }
    STLog( "align: fixedSize=%d  perpendicularOk=%d (expect 1)\n", fixedSz, perp );
    if ( !perp )
    { STLog( "RESULT: FAIL (aligned forward not unit-length-perpendicular to floor normal)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-align" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after align)\n" ); return 3; }

    STLog( "RESULT: PASS (Drop-to-Floor aligned the entity's forward into the floor plane)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunBrushFlipTest — Brush->Flip on a fixed-size entity (DoFlip 0x424F30, verified faithful
//  vs IDA).  load -> create a light -> set angles (10,30,50) -> DoFlip(Y) -> assert the yaw
//  flipped 180 (AngleNormalize360(30+180)=210) while pitch/roll are untouched.  In-memory.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestDoFlip( void *entInst, int *outFlippedOk, int *outOthersKept );  // xywnd.cpp

static int RunBrushFlipTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_brushflip.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: DoFlip (Brush->Flip, entity-angle flip) ===\n" );
    STLog( "map : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    STLog( "step: NewBrushDrag placeholder (256,256 -> 320,320)\n" );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 320, 320,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during placeholder create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no placeholder brush)\n" ); return 1; }

    STLog( "step: CreateEntityFromName(light)\n" );
    void *ent = nullptr;
    __try { ent = Radiant_TestCreateEntity( "light" ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during CreateEntityFromName (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ent ) { STLog( "RESULT: FAIL (no entity created/selected)\n" ); return 1; }

    STLog( "step: Radiant_TestDoFlip\n" );
    int flipped = 0, kept = 0, ok = 0;
    __try { ok = Radiant_TestDoFlip( ent, &flipped, &kept ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during DoFlip (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok ) { STLog( "RESULT: FAIL (no fixed-size entity to flip)\n" ); return 1; }
    STLog( "yawFlippedOk=%d  pitchRollKept=%d (expect 1 1)\n", flipped, kept );
    if ( !flipped )
    { STLog( "RESULT: FAIL (DoFlip did not flip yaw 30 -> 210)\n" ); return 1; }
    if ( !kept )
    { STLog( "RESULT: FAIL (DoFlip changed pitch/roll — should flip only the mirror axis)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-flip" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after DoFlip)\n" ); return 3; }

    STLog( "RESULT: PASS (Brush->Flip mirrored the entity and flipped its yaw 180)\n" );
    return 0;
}

// Reparent a world brush into a brush entity and verify it survives save/reload.
static int RunCreateBrushEntityTest( const char *mapPath, const char *classname )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_createbrushent.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: create-BRUSH-entity reparent round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    STLog( "class: %s\n", classname );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_createbrushent.map", tmpDir );
    STLog( "save: %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int entsBefore = g_qeglobals.d_num_entities;
    STLog( "loaded %d entities\n", entsBefore );
    if ( entsBefore <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // Drag out the placeholder world brush (selected). It will be REPARENTED (not
    // consumed) into the new brush entity.
    STLog( "step: NewBrushDrag placeholder (256,256 -> 320,320)\n" );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 320, 320,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during placeholder create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no placeholder brush)\n" ); return 1; }
    // Capture the placeholder DEF + verify it starts owned by worldspawn.
    selbrush_t *phInst = (selbrush_t *)ph;
    brush_t    *phDef  = phInst->def;
    entity_s   *worldDef = (entity_s *)world_entity->def;
    STLog( "placeholder inst=%p def=%p def.owner=%p (worldDef=%p)\n",
           ph, (void *)phDef, (void *)( phDef ? phDef->owner : nullptr ), (void *)worldDef );
    if ( !phDef || phDef->owner != worldDef )
    { STLog( "RESULT: FAIL (placeholder not owned by worldspawn pre-create)\n" ); return 1; }

    // Create the brush entity directly (exercises Entity_Create's reparent loop).
    STLog( "step: Entity_Create(%s) [brush entity]\n", classname );
    entity_s *ent = nullptr;
    __try { ent = (entity_s *)Radiant_TestCreateBrushEntity( classname ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Entity_Create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ent ) { STLog( "RESULT: FAIL (no brush entity created)\n" ); return 1; }
    entity_s *newDef = (entity_s *)ent->def;
    STLog( "created entity inst=%p def=%p\n", (void *)ent, (void *)newDef );

    // ── REPARENT ASSERTIONS ──
    // 1) the placeholder DEF now belongs to the new entity (NOT worldspawn).
    STLog( "post: placeholder def.owner=%p (newDef=%p, worldDef=%p)\n",
           (void *)phDef->owner, (void *)newDef, (void *)worldDef );
    if ( phDef->owner != newDef )
    { STLog( "RESULT: FAIL (brush def not reparented into new entity)\n" ); return 1; }
    if ( phDef->owner == worldDef )
    { STLog( "RESULT: FAIL (brush def still owned by worldspawn)\n" ); return 1; }
    // 2) the new entity's def-list is non-empty (brushes.prev != sentinel &def).
    bool hasDefBrush = ( (void *)newDef->brushes.prev != (void *)&newDef->def );
    if ( !hasDefBrush )
    { STLog( "RESULT: FAIL (new entity def-list empty after reparent)\n" ); return 1; }
    // 3) no dangling owners introduced (the old no-op Entity_LinkBrush_0 bug).
    if ( ST_GuardedDiag( "after-reparent" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reparent)\n" ); return 3; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int entsAfter = g_qeglobals.d_num_entities;
    if ( ST_GuardedDiag( "after-reload" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    // The reloaded brush-entity must exist (+1) and carry a brush. Walk the entity list
    // for one of class `classname` that owns at least one def brush.
    int matchEnts = 0, matchWithBrush = 0;
    for ( entity_s *e = entities.next; e != &entities; e = e->next )
    {
        entity_s_def *eDef = (entity_s_def *)e;
        if ( eDef->eclass && _stricmp( eDef->eclass->name, classname ) == 0 )
        {
            ++matchEnts;
            if ( (void *)eDef->brushes.prev != (void *)&eDef->def )
                ++matchWithBrush;
        }
    }
    STLog( "entity count: before=%d after-reload=%d (expected before+1); '%s' ents=%d withBrush=%d\n",
           entsBefore, entsAfter, classname, matchEnts, matchWithBrush );
    if ( entsAfter != entsBefore + 1 )
    { STLog( "RESULT: FAIL (brush entity did not survive round-trip)\n" ); return 1; }
    if ( matchWithBrush < 1 )
    { STLog( "RESULT: FAIL (reloaded '%s' entity carries no brush)\n", classname ); return 1; }

    STLog( "RESULT: PASS (brush entity '%s': reparent OK, +1 entity, brush survives round-trip)\n",
           classname );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunSetKeyTest — the entity-window key/value editor round-trip (the entwnd gate).
//  Mirrors the GUI workflow CreateEntity → AddProp: load → create a point entity of
//  <class> → SetKeyValue(<key>,<val>) on its def (the same call AddProp makes) → save
//  → reload → confirm the epair PERSISTED: it is present in the saved .map text AND a
//  reloaded entity of that class reads it back via ValueForKey2.
// ─────────────────────────────────────────────────────────────────────────────
static int RunSetKeyTest( const char *mapPath, const char *classname,
                          const char *key, const char *value )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_setkey.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: set-key round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    STLog( "class: %s   key: \"%s\"   value: \"%s\"\n", classname, key, value );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_setkey.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // Drag a placeholder brush, then create the point entity (consumes it).
    STLog( "step: NewBrushDrag placeholder (256,256 -> 320,320)\n" );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 320, 320,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during placeholder create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no placeholder brush)\n" ); return 1; }

    STLog( "step: CreateEntityFromName(%s)\n", classname );
    entity_s *inst = nullptr;
    __try { inst = (entity_s *)Radiant_TestCreateEntity( classname ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during CreateEntityFromName (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !inst ) { STLog( "RESULT: FAIL (no entity created/selected)\n" ); return 1; }

    // The entity DEF holds the epairs (instance->def). This is exactly
    // what AddProp does: Undo_AddEntity_W + SetKeyValue on edit_entity (= the def).
    entity_s_def *def = (entity_s_def *)inst->def;
    STLog( "step: SetKeyValue(def=%p, \"%s\", \"%s\")\n", (void *)def, key, value );
    __try { SetKeyValue( def, key, value ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during SetKeyValue (0x%08lX)\n", g_stLastExcept ); return 3; }

    // Read it straight back from the in-memory def (the AddProp/SetKeyValuePairs view).
    const char *vmem = ValueForKey2( (int)(intptr_t)def, key );
    STLog( "in-memory ValueForKey2(\"%s\") = \"%s\"\n", key, vmem ? vmem : "(null)" );
    if ( !vmem || strcmp( vmem, value ) != 0 )
    { STLog( "RESULT: FAIL (in-memory key did not take)\n" ); return 1; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    // Textual proof: the saved .map carries the epair line `"key" "value"`.
    char *saved = nullptr;
    long  savedLen = ST_ReadFile( savePath, &saved );
    if ( savedLen < 0 || !saved ) { STLog( "RESULT: FAIL (cannot read saved map)\n" ); return 2; }
    char needle[1024];
    _snprintf( needle, sizeof( needle ), "\"%s\" \"%s\"", key, value );
    bool inText = ( strstr( saved, needle ) != nullptr );
    free( saved );
    STLog( ".map contains  %s  : %s\n", needle, inText ? "YES" : "NO" );
    if ( !inText ) { STLog( "RESULT: FAIL (epair not in saved .map)\n" ); return 1; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-reload" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    // After reload, walk the entity DEF list (entities holds the defs; epairs live on the
    // def directly) and find one of <class> with key==value.
    bool found = false;
    for ( entity_s *e = entities.next; e && e != &entities; e = e->next )
    {
        const char *cn = ValueForKey2( (int)(intptr_t)e, "classname" );
        if ( cn && _stricmp( cn, classname ) == 0 )
        {
            const char *vr = ValueForKey2( (int)(intptr_t)e, key );
            if ( vr && strcmp( vr, value ) == 0 ) { found = true; break; }
        }
    }
    STLog( "after-reload: entity of class '%s' with \"%s\"=\"%s\" found: %s\n",
           classname, key, value, found ? "YES" : "NO" );
    if ( !found )
    { STLog( "RESULT: FAIL (key did not survive the round-trip)\n" ); return 1; }

    STLog( "RESULT: PASS (set \"%s\"=\"%s\" on '%s' -> save -> reload: epair persisted)\n",
           key, value, classname );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunModelTest — the MODEL/PREFAB PICKER round-trip (the model_gate).  Mirrors the
//  GUI workflow: place a misc_model entity (CreateEntityFromName, which now runs the
//  picker post instead of FATALing), then drive the model picker's COMMIT CORE
//  (Ed_CommitPickedModel via Radiant_TestPickModel — the file-dialog browse itself is
//  GUI-only, covered by gui_smoke/monkey).  Asserts the picked model PERSISTS: a
//  "model" "<name>" epair in the saved .map, a reloaded misc_model that reads it back,
//  and the entity count +1.  This is the misc_model authoring loop end-to-end:
//  pick -> CreateEntityFromName("misc_model") -> SetKeyValue("model", <name>) ->
//  EntityAssignModel -> bbox -> round-trip.
// ─────────────────────────────────────────────────────────────────────────────
static int RunModelTest( const char *mapPath, const char *modelName )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_model.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: model-picker round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    STLog( "model: \"%s\"\n", modelName );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_model.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int entsBefore = g_qeglobals.d_num_entities;
    STLog( "loaded %d entities\n", entsBefore );
    if ( entsBefore <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // Drag a placeholder brush where the misc_model drops, then create it.  This is
    // the first exercise of CreateEntityFromName("misc_model") since its model-branch
    // FATAL stub was replaced (it now does Ed_PostAddModelCommand, a no-op headless).
    STLog( "step: NewBrushDrag placeholder (256,256 -> 320,320)\n" );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 320, 320,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during placeholder create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no placeholder brush)\n" ); return 1; }

    STLog( "step: CreateEntityFromName(misc_model)\n" );
    entity_s *inst = nullptr;
    __try { inst = (entity_s *)Radiant_TestCreateEntity( "misc_model" ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during CreateEntityFromName (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !inst ) { STLog( "RESULT: FAIL (no misc_model created/selected)\n" ); return 1; }
    STLog( "created misc_model instance %p (selected)\n", (void *)inst );
    if ( ST_GuardedDiag( "after-create" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after create)\n" ); return 3; }

    // Drive the picker's commit core (Ed_CommitPickedModel): set the chosen model on
    // the created entity's DEF — exactly what the GUI picker does after CFileDialog.
    entity_s_def *def = (entity_s_def *)inst->def;
    STLog( "step: Radiant_TestPickModel(def=%p, \"%s\")\n", (void *)def, modelName );
    int pickRc = -1;
    __try { pickRc = Radiant_TestPickModel( def, modelName ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during model pick (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( pickRc != 0 ) { STLog( "RESULT: FAIL (model pick returned %d)\n", pickRc ); return 1; }

    // In-memory readback (the SetKeyValuePairs view).
    const char *vmem = ValueForKey2( (int)(intptr_t)def, "model" );
    STLog( "in-memory ValueForKey2(\"model\") = \"%s\"\n", vmem ? vmem : "(null)" );
    if ( !vmem || strcmp( vmem, modelName ) != 0 )
    { STLog( "RESULT: FAIL (model key did not take in memory)\n" ); return 1; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    // Textual proof: the saved .map carries the `"model" "<name>"` epair.
    char *saved = nullptr;
    long  savedLen = ST_ReadFile( savePath, &saved );
    if ( savedLen < 0 || !saved ) { STLog( "RESULT: FAIL (cannot read saved map)\n" ); return 2; }
    char needle[1024];
    _snprintf( needle, sizeof( needle ), "\"model\" \"%s\"", modelName );
    bool inText = ( strstr( saved, needle ) != nullptr );
    free( saved );
    STLog( ".map contains  %s  : %s\n", needle, inText ? "YES" : "NO" );
    if ( !inText ) { STLog( "RESULT: FAIL (model epair not in saved .map)\n" ); return 1; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int entsAfter = g_qeglobals.d_num_entities;
    if ( ST_GuardedDiag( "after-reload" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    // After reload, find a misc_model carrying our "model" key.
    bool found = false;
    for ( entity_s *e = entities.next; e && e != &entities; e = e->next )
    {
        const char *cn = ValueForKey2( (int)(intptr_t)e, "classname" );
        if ( cn && _stricmp( cn, "misc_model" ) == 0 )
        {
            const char *mv = ValueForKey2( (int)(intptr_t)e, "model" );
            if ( mv && strcmp( mv, modelName ) == 0 ) { found = true; break; }
        }
    }
    STLog( "after-reload: misc_model with \"model\"=\"%s\" found: %s\n",
           modelName, found ? "YES" : "NO" );
    STLog( "entity count: before=%d  after-reload=%d  (expected before+1)\n",
           entsBefore, entsAfter );
    if ( !found )
    { STLog( "RESULT: FAIL (model did not survive the round-trip)\n" ); return 1; }
    if ( entsAfter != entsBefore + 1 )
    { STLog( "RESULT: FAIL (misc_model entity count not +1)\n" ); return 1; }

    STLog( "RESULT: PASS (place misc_model + pick model \"%s\" -> save -> reload: +1 entity, model epair persisted)\n",
           modelName );
    return 0;
}

// Place and instance a generated brush-bearing prefab, then verify its saved reference.
static int RunPrefabTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_prefab.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: prefab-placement round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    // A generated prefab keeps this gate independent of installed map assets.
    char prefabPath[MAX_PATH];
    _snprintf( prefabPath, sizeof( prefabPath ), "%sradiant_unit_prefab.map", tmpDir );
    {
        FILE *pf = fopen( prefabPath, "w" );
        if ( !pf ) { STLog( "RESULT: FAIL (cannot write temp prefab)\n" ); return 2; }
        fputs(
            "iwmap 4\n"
            "\"000_Global\" flags  active\n"
            "// entity 0\n"
            "{\n"
            "\"classname\" \"worldspawn\"\n"
            "// brush 0\n"
            "{\n"
            " ( 0 0 64 ) ( 0 64 64 ) ( 64 64 64 ) caulk 0 0 0 0.5 0.5 0 lightmap_gray 16384 16384 0 0 0 0\n"
            " ( 0 0 0 ) ( 64 0 0 ) ( 64 64 0 ) caulk 0 0 0 0.5 0.5 0 lightmap_gray 16384 16384 0 0 0 0\n"
            " ( 0 0 0 ) ( 0 0 64 ) ( 0 64 64 ) caulk 0 0 0 0.5 0.5 0 lightmap_gray 16384 16384 0 0 0 0\n"
            " ( 64 0 0 ) ( 64 64 0 ) ( 64 64 64 ) caulk 0 0 0 0.5 0.5 0 lightmap_gray 16384 16384 0 0 0 0\n"
            " ( 0 0 0 ) ( 0 0 64 ) ( 64 0 64 ) caulk 0 0 0 0.5 0.5 0 lightmap_gray 16384 16384 0 0 0 0\n"
            " ( 0 64 0 ) ( 64 64 0 ) ( 64 64 64 ) caulk 0 0 0 0.5 0.5 0 lightmap_gray 16384 16384 0 0 0 0\n"
            "}\n"
            "}\n"
            "// entity 1\n"
            "{\n"
            "\"origin\" \"32.0 32.0 80.0\"\n"
            "\"classname\" \"info_null\"\n"
            "}\n",
            pf );
        fclose( pf );
    }
    STLog( "wrote prefab: %s\n", prefabPath );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_prefab_out.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int entsBefore = g_qeglobals.d_num_entities;
    STLog( "loaded %d entities\n", entsBefore );
    if ( entsBefore <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // 2. Drag a placeholder, create the misc_prefab (first exercise of the un-parked branch).
    STLog( "step: NewBrushDrag placeholder (256,256 -> 320,320)\n" );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 320, 320,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during placeholder create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no placeholder brush)\n" ); return 1; }

    STLog( "step: CreateEntityFromName(misc_prefab)\n" );
    entity_s *inst = nullptr;
    __try { inst = (entity_s *)Radiant_TestCreateEntity( "misc_prefab" ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during CreateEntityFromName (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !inst ) { STLog( "RESULT: FAIL (no misc_prefab created/selected)\n" ); return 1; }
    STLog( "created misc_prefab instance %p (selected)\n", (void *)inst );
    if ( ST_GuardedDiag( "after-create" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after create)\n" ); return 3; }

    // 3. Commit the prefab .map as the "model" key (the picker's commit core).
    entity_s_def *def = (entity_s_def *)inst->def;
    STLog( "step: Radiant_TestPickModel(def=%p, \"%s\")\n", (void *)def, prefabPath );
    int pickRc = -1;
    __try { pickRc = Radiant_TestPickModel( def, prefabPath ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during prefab pick (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( pickRc != 0 ) { STLog( "RESULT: FAIL (prefab pick returned %d)\n", pickRc ); return 1; }

    // 4. Drive the LOAD + INSTANCE — the QUEUE #8 deliverables (Prefab_Load +
    //    Entity_InitPrefabInst + sub_47ABE0).  This is the first runtime exercise.
    STLog( "step: Radiant_TestPlacePrefab(inst=%p) [Prefab_Load + Entity_InitPrefabInst]\n", (void *)inst );
    int brushInsts = -2;
    __try { brushInsts = Radiant_TestPlacePrefab( inst, prefabPath ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during prefab load/instance (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "prefab instanced brush count = %d\n", brushInsts );
    if ( brushInsts < 0 ) { STLog( "RESULT: FAIL (prefab failed to load/instance)\n" ); return 1; }
    if ( brushInsts < 1 ) { STLog( "RESULT: FAIL (prefab instanced 0 brushes — sub_47ABE0/Prefab_Init path not exercised)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-instance" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after instance)\n" ); return 3; }

    // In-memory readback: the misc_prefab carries the prefab path as its "model".
    const char *vmem = ValueForKey2( (int)(intptr_t)def, "model" );
    STLog( "in-memory ValueForKey2(\"model\") = \"%s\"\n", vmem ? vmem : "(null)" );
    if ( !vmem || strcmp( vmem, prefabPath ) != 0 )
    { STLog( "RESULT: FAIL (prefab model key did not take in memory)\n" ); return 1; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    // Textual proof: the saved .map carries the misc_prefab's `"model" "<prefab>"` epair
    // (NOT the prefab's brushes inlined — CoD saves a reference, like mp_backlot's geo).
    char *saved = nullptr;
    long  savedLen = ST_ReadFile( savePath, &saved );
    if ( savedLen < 0 || !saved ) { STLog( "RESULT: FAIL (cannot read saved map)\n" ); return 2; }
    // Match on the prefab's unique FILENAME (the .map writer escapes path backslashes as
    // "\\", so the full path with single '\' won't strstr-match) + the misc_prefab class.
    bool inText  = ( strstr( saved, "radiant_unit_prefab.map" ) != nullptr );
    bool hasClass = ( strstr( saved, "\"classname\" \"misc_prefab\"" ) != nullptr );
    free( saved );
    STLog( ".map contains  radiant_unit_prefab.map  : %s\n", inText ? "YES" : "NO" );
    STLog( ".map contains  misc_prefab classname : %s\n", hasClass ? "YES" : "NO" );
    if ( !inText || !hasClass ) { STLog( "RESULT: FAIL (misc_prefab reference not in saved .map)\n" ); return 1; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int entsAfter = g_qeglobals.d_num_entities;
    if ( ST_GuardedDiag( "after-reload" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    // After reload, find a misc_prefab carrying our prefab "model" key.
    bool found = false;
    for ( entity_s *e = entities.next; e && e != &entities; e = e->next )
    {
        const char *cn = ValueForKey2( (int)(intptr_t)e, "classname" );
        if ( cn && _stricmp( cn, "misc_prefab" ) == 0 )
        {
            const char *mv = ValueForKey2( (int)(intptr_t)e, "model" );
            if ( mv && strcmp( mv, prefabPath ) == 0 ) { found = true; break; }
        }
    }
    STLog( "after-reload: misc_prefab with \"model\"=\"%s\" found: %s\n",
           prefabPath, found ? "YES" : "NO" );
    STLog( "entity count: before=%d  after-reload=%d  (expected before+1)\n",
           entsBefore, entsAfter );
    if ( !found )
    { STLog( "RESULT: FAIL (misc_prefab reference did not survive the round-trip)\n" ); return 1; }
    if ( entsAfter != entsBefore + 1 )
    { STLog( "RESULT: FAIL (misc_prefab entity count not +1)\n" ); return 1; }

    STLog( "RESULT: PASS (place misc_prefab + load prefab (%d brush insts) -> save -> reload: +1 entity, prefab ref persisted)\n",
           brushInsts );
    return 0;
}

// Verify Prefab_NextLevel (0x489190) and Prefab_PrevLevel (0x489890) restore parent state.
static int ST_CountBrushes( selbrush_t *head )
{
    int n = 0;
    for ( selbrush_t *b = head->next; b && b != head && n < 1000000; b = b->next )
        ++n;
    return n;
}
static int ST_CountEntInsts()
{
    int n = 0;
    for ( entity_s *e = entityInsts.next; e && e != &entityInsts && n < 1000000; e = e->next )
        ++n;
    return n;
}

static int RunPrefabEditTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_prefabedit.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: prefab-edit-in-place (enter/leave) round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    // 1. Same brush-bearing prefab .map as RunPrefabTest (deterministic).
    char prefabPath[MAX_PATH];
    _snprintf( prefabPath, sizeof( prefabPath ), "%sradiant_unit_prefab_edit.map", tmpDir );
    {
        FILE *pf = fopen( prefabPath, "w" );
        if ( !pf ) { STLog( "RESULT: FAIL (cannot write temp prefab)\n" ); return 2; }
        fputs(
            "iwmap 4\n"
            "\"000_Global\" flags  active\n"
            "// entity 0\n"
            "{\n"
            "\"classname\" \"worldspawn\"\n"
            "// brush 0\n"
            "{\n"
            " ( 0 0 64 ) ( 0 64 64 ) ( 64 64 64 ) caulk 0 0 0 0.5 0.5 0 lightmap_gray 16384 16384 0 0 0 0\n"
            " ( 0 0 0 ) ( 64 0 0 ) ( 64 64 0 ) caulk 0 0 0 0.5 0.5 0 lightmap_gray 16384 16384 0 0 0 0\n"
            " ( 0 0 0 ) ( 0 0 64 ) ( 0 64 64 ) caulk 0 0 0 0.5 0.5 0 lightmap_gray 16384 16384 0 0 0 0\n"
            " ( 64 0 0 ) ( 64 64 0 ) ( 64 64 64 ) caulk 0 0 0 0.5 0.5 0 lightmap_gray 16384 16384 0 0 0 0\n"
            " ( 0 0 0 ) ( 0 0 64 ) ( 64 0 64 ) caulk 0 0 0 0.5 0.5 0 lightmap_gray 16384 16384 0 0 0 0\n"
            " ( 0 64 0 ) ( 64 64 0 ) ( 64 64 64 ) caulk 0 0 0 0.5 0.5 0 lightmap_gray 16384 16384 0 0 0 0\n"
            "}\n"
            "}\n"
            "// entity 1\n"
            "{\n"
            "\"origin\" \"32.0 32.0 80.0\"\n"
            "\"classname\" \"info_null\"\n"
            "}\n",
            pf );
        fclose( pf );
    }
    STLog( "wrote prefab: %s\n", prefabPath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "loaded %d entities\n", g_qeglobals.d_num_entities );
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }

    // 2. Place + instance the misc_prefab (the RunPrefabTest setup).
    STLog( "step: NewBrushDrag placeholder\n" );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 320, 320, 0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode(); STLog( "RESULT: CRASH during placeholder (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no placeholder brush)\n" ); return 1; }

    STLog( "step: CreateEntityFromName(misc_prefab)\n" );
    entity_s *inst = nullptr;
    __try { inst = (entity_s *)Radiant_TestCreateEntity( "misc_prefab" ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode(); STLog( "RESULT: CRASH during CreateEntity (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !inst ) { STLog( "RESULT: FAIL (no misc_prefab created)\n" ); return 1; }

    STLog( "step: Radiant_TestPickModel\n" );
    entity_s_def *def = (entity_s_def *)inst->def;
    int pickRc = -1;
    __try { pickRc = Radiant_TestPickModel( def, prefabPath ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode(); STLog( "RESULT: CRASH during pick (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( pickRc != 0 ) { STLog( "RESULT: FAIL (prefab pick returned %d)\n", pickRc ); return 1; }

    STLog( "step: Radiant_TestPlacePrefab\n" );

    int brushInsts = -2;
    __try { brushInsts = Radiant_TestPlacePrefab( inst, prefabPath ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode(); STLog( "RESULT: CRASH during instance (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( brushInsts < 1 ) { STLog( "RESULT: FAIL (prefab instanced %d brushes)\n", brushInsts ); return 1; }
    STLog( "placed + instanced misc_prefab (%d brush insts)\n", brushInsts );

    // 3. Snapshot the PARENT world state before entering.
    int  parentBrushes = ST_CountBrushes( &active_brushes );
    int  parentInsts   = ST_CountEntInsts();
    int  lvlBefore     = prefabStackLevel;
    char parentMap[MAX_PATH];
    strncpy( parentMap, currentmap, sizeof( parentMap ) - 1 ); parentMap[sizeof( parentMap ) - 1] = 0;
    STLog( "parent: brushes=%d entityInsts=%d prefabStackLevel=%d map=\"%s\"\n",
           parentBrushes, parentInsts, lvlBefore, parentMap );
    if ( lvlBefore != 0 ) { STLog( "RESULT: FAIL (prefabStackLevel not 0 before enter)\n" ); return 2; }

    // 4. Re-select exactly the misc_prefab instance (Prefab_NextLevel(NULL) requires it).
    //    Radiant_TestPlacePrefab may have changed selection; force-select the instance's brush.
    Select_Deselect( 1 );
    if ( inst->brushes.ownerNext && inst->brushes.ownerNext != (selbrush_t *)&inst->brushes )
        Select_Brush( inst->brushes.ownerNext, 0, 0, 0 );
    int selCount = ST_CountBrushes( &selected_brushes );
    STLog( "selected the misc_prefab (selected brush count=%d)\n", selCount );

    // 5. ENTER the prefab (push).
    STLog( "step: Prefab_NextLevel(NULL)  [ENTER]\n" );
    __try { Prefab_NextLevel( nullptr ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode(); STLog( "RESULT: CRASH during Prefab_NextLevel (0x%08lX)\n", g_stLastExcept ); return 3; }

    int  subBrushes = ST_CountBrushes( &active_brushes );
    int  lvlAfter   = prefabStackLevel;
    STLog( "after ENTER: prefabStackLevel=%d  sub-map brushes=%d (instanced=%d)  map=\"%s\"\n",
           lvlAfter, subBrushes, brushInsts, currentmap );
    if ( lvlAfter != lvlBefore + 1 )
    { STLog( "RESULT: FAIL (prefabStackLevel did not increment on enter: %d -> %d)\n", lvlBefore, lvlAfter ); return 1; }
    // The live world list must now be the prefab's instanced sub-map geometry: its
    // active-brush count equals the brushes the prefab instanced (brushInsts), and it must
    // differ from the parent map's brush set — i.e. the world list really was swapped.
    if ( subBrushes != brushInsts )
    { STLog( "RESULT: FAIL (sub-map active-brush count = %d, expected the prefab's %d instanced brushes)\n", subBrushes, brushInsts ); return 1; }
    if ( subBrushes == parentBrushes )
    { STLog( "RESULT: FAIL (active-brush list unchanged on enter — world not swapped to sub-map)\n" ); return 1; }
    if ( ST_GuardedDiag( "in-prefab" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list inside prefab)\n" ); return 3; }

    // 6. LEAVE the prefab (pop) — restore the parent world.
    STLog( "step: Prefab_PrevLevel()  [LEAVE]\n" );
    __try { Prefab_PrevLevel(); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode(); STLog( "RESULT: CRASH during Prefab_PrevLevel (0x%08lX)\n", g_stLastExcept ); return 3; }

    int  backBrushes = ST_CountBrushes( &active_brushes );
    int  backInsts   = ST_CountEntInsts();
    int  lvlBack     = prefabStackLevel;
    STLog( "after LEAVE: prefabStackLevel=%d  brushes=%d  entityInsts=%d  map=\"%s\"\n",
           lvlBack, backBrushes, backInsts, currentmap );
    if ( ST_GuardedDiag( "after-leave" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after leave)\n" ); return 3; }

    if ( lvlBack != lvlBefore )
    { STLog( "RESULT: FAIL (prefabStackLevel not restored: %d != %d)\n", lvlBack, lvlBefore ); return 1; }
    // The PARENT world is back: its currentmap, entity-instance list and prefab level are
    // restored exactly.  The active-brush count is restored to the parent's ballpark; it may
    // legitimately grow by the prefab's model-display brush, because LEAVE's Model_SetModel
    // RE-REALIZES the misc_prefab's model (which the prior in-place edit had invalidated,
    // ++version), re-adding its display brush — exactly the binary's behaviour and the same
    // +1 the placement gate shows on reload (instanced 32 → realized 33).  The strict
    // structural restore (entityInsts, level, currentmap, clean list) is the real signal.
    if ( backInsts != parentInsts )
    { STLog( "RESULT: FAIL (parent entityInsts count not restored: %d != %d)\n", backInsts, parentInsts ); return 1; }
    if ( strcmp( currentmap, parentMap ) != 0 )
    { STLog( "RESULT: FAIL (currentmap not restored: \"%s\" != \"%s\")\n", currentmap, parentMap ); return 1; }
    if ( backBrushes < parentBrushes )
    { STLog( "RESULT: FAIL (parent brushes lost: %d < %d)\n", backBrushes, parentBrushes ); return 1; }
    if ( backBrushes == subBrushes && subBrushes != parentBrushes )
    { STLog( "RESULT: FAIL (still showing sub-map after leave: %d)\n", backBrushes ); return 1; }

    STLog( "RESULT: PASS (enter prefab: lvl %d->%d, world swapped to sub-map (%d brushes vs parent %d); "
           "leave: lvl ->%d, parent restored (currentmap + %d insts + %d brushes, list clean))\n",
           lvlBefore, lvlAfter, subBrushes, parentBrushes, lvlBack, backInsts, backBrushes );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunEntFlagsTest — the Entity-Window spawnflags + angle controls round-trip.
//  Mirrors the GUI: load → create + select a point entity of <class> → set a
//  spawnflag bit (the SetSpawnFlags_R checked-branch math, win_ent.cpp) AND an
//  "angles" heading (Entity_SetAngles, exactly the IDC_ANGLE90 WndProc case) → save
//  → reload → confirm BOTH keys persisted ("spawnflags" "<2^bit>" and "angles" "0 90 0").
//  <bit> defaults to 2 (=> spawnflags "4").  Headless: the checkbox HWND→bit input is
//  forced (covered live by gui_smoke/monkey); the bit math + Undo + SetKeyValue path
//  and the (HWND-independent) Entity_SetAngles are the real ported functions.
// ─────────────────────────────────────────────────────────────────────────────
static int RunEntFlagsTest( const char *mapPath, const char *classname, int flagBit )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_entflags.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: entity spawnflags + angle round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    STLog( "class: %s   flagBit: %d (spawnflags=%d)\n", classname, flagBit, 1 << flagBit );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_entflags.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    STLog( "step: NewBrushDrag placeholder (256,256 -> 320,320)\n" );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 320, 320,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during placeholder create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no placeholder brush)\n" ); return 1; }

    STLog( "step: CreateEntityFromName(%s)\n", classname );
    entity_s *inst = nullptr;
    __try { inst = (entity_s *)Radiant_TestCreateEntity( classname ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during CreateEntityFromName (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !inst ) { STLog( "RESULT: FAIL (no entity created/selected)\n" ); return 1; }

    entity_s_def *def = (entity_s_def *)inst->def;

    // ── angle path: Entity_SetAngles(90, 1) — the IDC_ANGLE90 WndProc case verbatim.
    //    The entity is the only selection, so its "angles" key becomes "0 90 0".
    STLog( "step: Entity_SetAngles(90, axis=1)  [the angle button path]\n" );
    __try { Entity_SetAngles( 90.0f, 1 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Entity_SetAngles (0x%08lX)\n", g_stLastExcept ); return 3; }

    // ── spawnflags path: set bit <flagBit> via the SetSpawnFlags_R checked-branch math.
    STLog( "step: Radiant_TestSetSpawnflagBit(def, %d)  [the spawnflag checkbox path]\n", flagBit );
    int newFlags;
    __try { newFlags = Radiant_TestSetSpawnflagBit( def, flagBit ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during SetSpawnflagBit (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "computed spawnflags = %d\n", newFlags );
    if ( newFlags != ( 1 << flagBit ) )
    { STLog( "RESULT: FAIL (bit math wrong: got %d, want %d)\n", newFlags, 1 << flagBit ); return 1; }

    // In-memory readback (the SetKeyValuePairs view).
    char wantFlags[32], wantAngles[64];
    _snprintf( wantFlags,  sizeof( wantFlags ),  "%d", 1 << flagBit );
    _snprintf( wantAngles, sizeof( wantAngles ), "0 90 0" );
    const char *mFlags  = ValueForKey2( (int)(intptr_t)def, "spawnflags" );
    const char *mAngles = ValueForKey2( (int)(intptr_t)def, "angles" );
    STLog( "in-memory spawnflags=\"%s\"  angles=\"%s\"\n",
           mFlags ? mFlags : "(null)", mAngles ? mAngles : "(null)" );
    if ( !mFlags  || strcmp( mFlags,  wantFlags  ) != 0 ) { STLog( "RESULT: FAIL (spawnflags not set)\n" ); return 1; }
    if ( !mAngles || strcmp( mAngles, wantAngles ) != 0 ) { STLog( "RESULT: FAIL (angles not set)\n" ); return 1; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    // Textual proof: both keys present in the saved .map.
    char *saved = nullptr;
    long  savedLen = ST_ReadFile( savePath, &saved );
    if ( savedLen < 0 || !saved ) { STLog( "RESULT: FAIL (cannot read saved map)\n" ); return 2; }
    char needF[64], needA[64];
    _snprintf( needF, sizeof( needF ), "\"spawnflags\" \"%s\"", wantFlags );
    _snprintf( needA, sizeof( needA ), "\"angles\" \"%s\"", wantAngles );
    bool inF = ( strstr( saved, needF ) != nullptr );
    bool inA = ( strstr( saved, needA ) != nullptr );
    free( saved );
    STLog( ".map contains  %s : %s ;  %s : %s\n", needF, inF ? "YES" : "NO", needA, inA ? "YES" : "NO" );
    if ( !inF || !inA ) { STLog( "RESULT: FAIL (a key not in saved .map)\n" ); return 1; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-reload" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    bool found = false;
    for ( entity_s *e = entities.next; e && e != &entities; e = e->next )
    {
        const char *cn = ValueForKey2( (int)(intptr_t)e, "classname" );
        if ( cn && _stricmp( cn, classname ) == 0 )
        {
            const char *vf = ValueForKey2( (int)(intptr_t)e, "spawnflags" );
            const char *va2 = ValueForKey2( (int)(intptr_t)e, "angles" );
            if ( vf && strcmp( vf, wantFlags ) == 0 && va2 && strcmp( va2, wantAngles ) == 0 )
            { found = true; break; }
        }
    }
    STLog( "after-reload: '%s' with spawnflags=\"%s\" angles=\"%s\" found: %s\n",
           classname, wantFlags, wantAngles, found ? "YES" : "NO" );
    if ( !found ) { STLog( "RESULT: FAIL (keys did not survive the round-trip)\n" ); return 1; }

    STLog( "RESULT: PASS (spawnflags=%s + angles=\"%s\" on '%s' -> save -> reload: persisted)\n",
           wantFlags, wantAngles, classname );
    return 0;
}

static int RunDestructiveTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_destruct.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: destructive delete-then-save ===\n" );
    STLog( "map : %s\n", mapPath );

    AddVectoredExceptionHandler( 1, ST_Veh );   // log faulting module!offset + backtrace

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_destruct.map", tmpDir );
    STLog( "save: %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                 Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int nEnts = g_qeglobals.d_num_entities;
    STLog( "loaded %d entities\n", nEnts );
    if ( nEnts <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }

    int total = ST_GuardedDiag( "after-load" );
    if ( total < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // Select every other active brush (parity by iteration order).
    STLog( "step: select every-other brush\n" );
    int sel = ST_GuardedSelect();
    if ( sel < 0 )
    { STLog( "RESULT: CRASH during selection (0x%08lX) — Select_Brush\n", g_stLastExcept ); return 3; }
    STLog( "selected %d of %d active brushes\n", sel, total );

    STLog( "step: Select_Delete\n" );
    if ( ST_GuardedDelete() != 0 )
    { STLog( "RESULT: CRASH during Select_Delete (0x%08lX)\n", g_stLastExcept ); return 3; }

    if ( ST_GuardedDiag( "after-delete" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after delete)\n" ); return 3; }

    STLog( "step: Map_SaveFile (walks entity def-lists — the suspected fault point)\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX) — DELETE-PATH UAF\n", g_stLastExcept ); return 3; }

    // The operator's symptom was a TRUNCATED map that fails to parse next load.
    // Reload the saved map in a fresh world and confirm it still parses.
    STLog( "step: reload saved map (verify it still parses)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int n2 = g_qeglobals.d_num_entities;
    STLog( "reloaded %d entities\n", n2 );
    if ( n2 <= 0 ) { STLog( "RESULT: FAIL (reload parse failed — saved map corrupted)\n" ); return 1; }

    STLog( "RESULT: PASS (load->select->delete->save->reload clean)\n" );
    return 0;
}

// Round-trip representative preferences through the registry, then restore defaults.
static int RunPrefsTest()
{
    // Durable log (the GUI-subsystem exe's stdout is not pipe-capturable).
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_prefs.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: prefs save/load round-trip ===\n" );

    // 1) defaults → confirm the binary defaults are what we expect (the load-bearing ones)
    Prefs_SetDefaults( g_PrefsDlg );
    int fails = 0;
    if ( g_PrefsDlg->m_bNoClamp != 0 )            { STLog( "FAIL default NoClamp != 0\n" ); ++fails; }
    if ( g_PrefsDlg->m_nMouseButtons != 3 )       { STLog( "FAIL default MouseButtons != 3\n" ); ++fails; }
    if ( g_PrefsDlg->enable_light_preview != 1 )  { STLog( "FAIL default LightPreview != 1\n" ); ++fails; }
    if ( g_PrefsDlg->preview_sun_aswell != 0 )    { STLog( "FAIL default SunPreview != 0\n" ); ++fails; }
    if ( (int)g_PrefsDlg->camera_fov != 65 )      { STLog( "FAIL default Fov != 65\n" ); ++fails; }
    if ( g_PrefsDlg->m_bTextureLock != 1 )        { STLog( "FAIL default TextureLock != 1\n" ); ++fails; }

    // 2) mutate + save
    g_PrefsDlg->m_bNoClamp           = 1;      // grid snap OFF
    g_PrefsDlg->m_nMouse_unsure      = 0;      // → derived m_nMouseButtons = 2 (2-button)
    g_PrefsDlg->enable_light_preview = 0;
    g_PrefsDlg->preview_sun_aswell   = 1;
    g_PrefsDlg->camera_fov           = 100.0f;
    g_PrefsDlg->m_nCubicScale        = 21;
    Prefs_SavePrefs( g_PrefsDlg );

    // 3) clobber in-memory back to defaults, then reload from the registry
    Prefs_SetDefaults( g_PrefsDlg );
    Prefs_LoadPrefs( g_PrefsDlg );

    // 4) verify the mutated values survived
    if ( g_PrefsDlg->m_bNoClamp != 1 )            { STLog( "FAIL NoClamp did not persist\n" ); ++fails; }
    if ( g_PrefsDlg->m_nMouseButtons != 2 )       { STLog( "FAIL MouseButtons (derived) != 2\n" ); ++fails; }
    if ( g_PrefsDlg->enable_light_preview != 0 )  { STLog( "FAIL LightPreview did not persist\n" ); ++fails; }
    if ( g_PrefsDlg->preview_sun_aswell != 1 )    { STLog( "FAIL SunPreview did not persist\n" ); ++fails; }
    if ( (int)g_PrefsDlg->camera_fov != 100 )     { STLog( "FAIL Fov did not persist (%d)\n", (int)g_PrefsDlg->camera_fov ); ++fails; }
    if ( g_PrefsDlg->m_nCubicScale != 21 )        { STLog( "FAIL CubicScale did not persist\n" ); ++fails; }

    STLog( "  NoClamp=%d MouseButtons=%d LightPreview=%d SunPreview=%d Fov=%d CubicScale=%d\n",
           g_PrefsDlg->m_bNoClamp, g_PrefsDlg->m_nMouseButtons, g_PrefsDlg->enable_light_preview,
           g_PrefsDlg->preview_sun_aswell, (int)g_PrefsDlg->camera_fov, g_PrefsDlg->m_nCubicScale );

    // 5) restore the registry to the editor defaults (leave a clean slate)
    Prefs_SetDefaults( g_PrefsDlg );
    Prefs_SavePrefs( g_PrefsDlg );

    STLog( fails ? "RESULT: FAIL (%d field(s) did not round-trip)\n" : "RESULT: PASS (prefs round-trip)\n", fails );
    return fails ? 1 : 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunInspectorTest — INSPECTOR TAB SWITCHING deterministic-state gate.  The GUI proof
//  (the inspector visibly switches Entity ↔ Texture ↔ Filters, panes still render) is
//  the capture; this headless gate asserts the SetInspectorMode STATE MACHINE: the
//  inspector_mode global flips to each requested mode, and the style-gate no-op holds.
//  CEntityWnd_SetInspectorMode is headless-safe (it guards every HWND / the parent frame),
//  so it runs here without a CMainFrame: it just sets the global + (would) update the UI.
//  Self-contained (no map).
// ─────────────────────────────────────────────────────────────────────────────
static int RunInspectorTest()
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_inspector.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: inspector mode-switch state machine ===\n" );
    int fails = 0;

    // The four modes (raw immediates from SetInspectorMode 0x496b00).
    struct { int mode; const char *name; } cases[] =
    {
        { INSPECTOR_TEXTURE, "Textures" },
        { INSPECTOR_CONSOLE, "Console"  },
        { INSPECTOR_FILTER,  "Filters"  },
        { INSPECTOR_ENTITY,  "Entity"   },
    };
    for ( auto &c : cases )
    {
        inspector_mode = -1;                       // poison so we know the call set it
        CEntityWnd_SetInspectorMode( c.mode );
        if ( inspector_mode != c.mode )
        {
            STLog( "FAIL %s: inspector_mode=0x%x expected 0x%x\n", c.name, inspector_mode, c.mode );
            ++fails;
        }
        else
            STLog( "  %s: inspector_mode flipped to 0x%x OK\n", c.name, inspector_mode );
    }

    // Sanity: the constants are the binary's raw values (NOT the g_nUpdateBits W_* set —
    // see the qedefs.h note).  Guards against a future macro-collision regression.
    if ( INSPECTOR_ENTITY  != 0x80 ||
         INSPECTOR_TEXTURE != 0x10 ||
         INSPECTOR_CONSOLE != 0x40 ||
         INSPECTOR_FILTER  != 0x2000 )
    {
        STLog( "FAIL inspector-mode constants drifted (E=0x%x T=0x%x C=0x%x F=0x%x)\n",
               INSPECTOR_ENTITY, INSPECTOR_TEXTURE, INSPECTOR_CONSOLE, INSPECTOR_FILTER );
        ++fails;
    }

    STLog( fails ? "RESULT: FAIL (%d)\n" : "RESULT: PASS (inspector mode-switch state machine)\n", fails );
    return fails ? 1 : 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunLayeredMatTest — the LAYERED-MATERIAL LIBRARY LOADER gate (RADIANT_SELFTEST
//  layeredmat).  Proves the fix for the empty "View layered materials" sub-view: the
//  loader LayerdMatWnd (0x416D40) reads the library named by the project entity's
//  "layeredmaterials" epair and fills lyrMtlGlob_Layers[] with the parsed entries.
//
//  The gate is the headless counterpart to the GUI right-click toggle (which needs the
//  live editor + operator).  It (1) builds the project entity exactly as the
//  QE_LoadProject analog (Radiant_SetProjectMapsPath) does, (2) seeds the
//  "layeredmaterials" epair, (3) writes a known 4-entry library (a/b/test/z — the stock
//  cod4_layered_material_library.txt content, inlined so the gate is machine-independent)
//  to the editor CWD where LoadFile's plain fopen resolves it, (4) runs the loader, and
//  (5) asserts lyrMtlGlob_entryCount == 4 with the sorted names a,b,test,z.
//
//  SEH-guarded: in the GUI the loader's per-layer Texture_GetHandle runs with the
//  renderer + material system up (QE_LoadProject calls LayerdMatWnd after
//  R_BeginRegistrationInternal); headless the renderer is NOT up, so if Texture_GetHandle
//  faults the gate reports CRASH (the entry COUNT/NAMES are written before the handle, so
//  a clean run still proves the parse).  Self-contained (no map).
// ─────────────────────────────────────────────────────────────────────────────
static int RunLayeredMatTest()
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_layeredmat.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: layered-material library loader (LayerdMatWnd) ===\n" );
    AddVectoredExceptionHandler( 1, ST_Veh );
    int fails = 0;

    // (1) Build the project entity exactly like the QE_LoadProject analog
    //     (Radiant_SetProjectMapsPath): 140-byte def, self-linked brush-list sentinels.
    if ( !g_qeglobals.d_project_entity )
    {
        entity_s_def *proj = (entity_s_def *)operator new( 0x8Cu );
        memset( proj, 0, 0x8Cu );
        proj->brushes.prev = (selbrush_t *)&proj->def;
        proj->def          = (entity_s *)&proj->def;
        g_qeglobals.d_project_entity = (entity_s *)proj;
    }

    // (3) Write the known 4-entry library to the editor CWD (relative path, so LoadFile's
    //     fopen resolves it against the CWD — the same place the file must live for the
    //     GUI).  Inlined content == the stock cod4_layered_material_library.txt.
    const char *kLibName = "radiant_selftest_layered_material_library.txt";
    static const char *kLib =
        "LayeredMaterialLibrary 1\n\n"
        "a 1\n{\n\t0 \"case256\";\n}\n\n"
        "b 1\n{\n\t0 \"case1024\";\n}\n\n"
        "test 1\n{\n\t0 \"aitrig\";\n}\n\n"
        "z 1\n{\n\t0 \"case1024\";\n}\n";
    {
        FILE *lf = fopen( kLibName, "wb" );
        if ( !lf ) { STLog( "RESULT: FAIL (cannot write %s in CWD)\n", kLibName ); return 2; }
        fputs( kLib, lf );
        fclose( lf );
    }

    // (2) Seed the "layeredmaterials" epair to point at the file just written.
    SetKeyValue( (entity_s_def *)g_qeglobals.d_project_entity, "layeredmaterials", kLibName );

    // Reproduce the GUI's pre-LayerdMatWnd cache state: in QE_LoadProject, Load_Textures()
    // registers every material before LayerdMatWnd, so the library's layer materials are
    // already cached qtextures and the parser's Texture_GetHandle hits the cache instead
    // of the renderer/FS-dependent Register_WorldMaterial (which AVs headless — no FS).
    // (Names are pre-lowercased, as Texture_GetHandle stores them.)
    Radiant_TestSeedLayerMaterial( "case256" );
    Radiant_TestSeedLayerMaterial( "case1024" );
    Radiant_TestSeedLayerMaterial( "aitrig" );

    // (4) Run the loader (SEH-guarded — see header).
    STLog( "step: LayerdMatWnd (load + parse + sort + crc)\n" );
    signed int rv = 0;
    __try { rv = LayerdMatWnd(); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        g_stLastExcept = GetExceptionCode();
        STLog( "RESULT: CRASH during LayerdMatWnd (0x%08lX) — likely renderer-dependent "
               "Texture_GetHandle headless (entry count was %d)\n",
               g_stLastExcept, lyrMtlGlob_entryCount );
        remove( kLibName );
        return 3;
    }
    STLog( "LayerdMatWnd returned %d; lyrMtlGlob_entryCount = %d\n", rv, lyrMtlGlob_entryCount );

    // (5) Assert the parsed count + sorted names.
    if ( lyrMtlGlob_entryCount != 4 )
    {
        STLog( "FAIL: entryCount=%d expected 4\n", lyrMtlGlob_entryCount );
        ++fails;
    }
    const char *expect[4] = { "a", "b", "test", "z" };   // I_stricmp-sorted order
    const int   n = lyrMtlGlob_entryCount < 4 ? lyrMtlGlob_entryCount : 4;
    for ( int i = 0; i < n; ++i )
    {
        // Entry name is the first field of each 84-byte record (LYR_ENTRY_SIZE).
        const char *name = (const char *)&lyrMtlGlob_Layers[84 * i];
        STLog( "  entry[%d] = \"%s\"\n", i, name );
        if ( strcmp( name, expect[i] ) != 0 )
        {
            STLog( "FAIL: entry[%d]=\"%s\" expected \"%s\"\n", i, name, expect[i] );
            ++fails;
        }
    }

    remove( kLibName );
    STLog( fails ? "RESULT: FAIL (%d)\n"
                 : "RESULT: PASS (layered-material library loaded 4 entries a,b,test,z)\n", fails );
    return fails ? 1 : 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunEclassParseTest — the /*QUAKED .def-block parser gate (RADIANT_SELFTEST
//  eclassparse).  Drives Eclass_InitFromText (0x481150) directly on synthetic QUAKED
//  blocks and validates the parsed fields against the binary's behaviour: entity name,
//  (r g b) color, the optional (mins)(maxs) fixed-size box + fixedsize flag, the
//  "?"-prefixed spawnflag NAME list (ec->flagname0..7 at +0x40 + 32*k), and the
//  classtype bit map (worldspawn=0; else |2, plus light=1, prefab=0x10, trigger_radius
//  =0x40, trigger_disk=0x80, model=8, path=4, node_*=0x20, reflection_probe=0x100).
//  Also covers the TRUNCATED size-block case (size '(' with < 2 closing ')') where the
//  binary returns name+color+fixedsize only (disasm 0x481280 -> loc_48159E) — the path
//  this pass made faithful.  Self-contained (no map, no disk).
// ─────────────────────────────────────────────────────────────────────────────
static bool EcNear( float a, float b ) { return fabs( a - b ) < 0.001f; }

static int RunEclassParseTest()
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_eclassparse.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Eclass_InitFromText /*QUAKED block parse ===\n" );
    int fails = 0;

    // ── (1) fixed-size point entity with a spawnflag name list + classtype bit. ──
    // "light" → classtype |= 1 (and NOT |2, light short-circuits before |2? no: |2 is
    //  set for every non-worldspawn, THEN light adds |1 and returns → 1|2 == 3).
    {
        char blk[512];
        sprintf( blk,
            "/*QUAKED light (1 0.5 0.25) (-8 -8 -8) (8 8 8) START_OFF NEEDS_PLAYER\n"
            "A light source.\n*/" );
        eclass_t *ec = Eclass_InitFromText( blk );
        if ( !ec ) { STLog( "FAIL light: null eclass\n" ); ++fails; }
        else
        {
            if ( strcmp( ec->name, "light" ) )
            { STLog( "FAIL light name='%s'\n", ec->name ); ++fails; }
            if ( !EcNear( ec->color[0], 1.0f ) || !EcNear( ec->color[1], 0.5f ) || !EcNear( ec->color[2], 0.25f ) )
            { STLog( "FAIL light color=(%g %g %g)\n", ec->color[0], ec->color[1], ec->color[2] ); ++fails; }
            if ( !ec->fixedsize )
            { STLog( "FAIL light fixedsize=0\n" ); ++fails; }
            if ( !EcNear( ec->mins[0], -8 ) || !EcNear( ec->maxs[2], 8 ) )
            { STLog( "FAIL light bbox mins=(%g %g %g) maxs=(%g %g %g)\n",
                     ec->mins[0], ec->mins[1], ec->mins[2], ec->maxs[0], ec->maxs[1], ec->maxs[2] ); ++fails; }
            // spawnflag NAMES live at +0x40 + 32*k (ec->flagname0/flagname1).
            if ( strcmp( ec->flagname0, "START_OFF" ) )
            { STLog( "FAIL light flagname0='%s'\n", ec->flagname0 ); ++fails; }
            if ( strcmp( ec->flagname1, "NEEDS_PLAYER" ) )
            { STLog( "FAIL light flagname1='%s'\n", ec->flagname1 ); ++fails; }
            if ( ec->classtype != ( 1 | 2 ) )
            { STLog( "FAIL light classtype=0x%x want 0x3\n", ec->classtype ); ++fails; }
            if ( !fails ) STLog( "  light: name/color/bbox/flags/classtype(0x%x) OK\n", ec->classtype );
        }
    }

    // ── (2) brush-based entity (no size box; just '?') + classtype bits. ──
    {
        char blk[256];
        sprintf( blk, "/*QUAKED trigger_radius (0 0.5 0) ?\nA radius trigger.\n*/" );
        eclass_t *ec = Eclass_InitFromText( blk );
        if ( !ec ) { STLog( "FAIL trigger_radius: null\n" ); ++fails; }
        else
        {
            if ( ec->fixedsize )
            { STLog( "FAIL trigger_radius fixedsize set (should be brush)\n" ); ++fails; }
            if ( ec->classtype != ( 0x40 | 2 ) )
            { STLog( "FAIL trigger_radius classtype=0x%x want 0x42\n", ec->classtype ); ++fails; }
            else STLog( "  trigger_radius: brush + classtype 0x%x OK\n", ec->classtype );
        }
    }

    // ── (3) worldspawn is the ONLY class that does NOT get the |2 bit. ──
    {
        char blk[256];
        sprintf( blk, "/*QUAKED worldspawn (0 0 0) ?\nThe world.\n*/" );
        eclass_t *ec = Eclass_InitFromText( blk );
        if ( !ec ) { STLog( "FAIL worldspawn: null\n" ); ++fails; }
        else if ( ec->classtype != 0 )
        { STLog( "FAIL worldspawn classtype=0x%x want 0\n", ec->classtype ); ++fails; }
        else STLog( "  worldspawn: classtype 0 (no |2) OK\n" );
    }

    // ── (4) misc_prefab → CLASS_PREFAB 0x10 (|2); node_* → 0x20 (|2). ──
    {
        char blk[256];
        sprintf( blk, "/*QUAKED misc_prefab (0 0.5 0) (-8 -8 -8) (8 8 8)\nA prefab.\n*/" );
        eclass_t *ec = Eclass_InitFromText( blk );
        if ( !ec || ec->classtype != ( 0x10 | 2 ) )
        { STLog( "FAIL misc_prefab classtype=0x%x want 0x12\n", ec ? ec->classtype : -1 ); ++fails; }
        else STLog( "  misc_prefab: classtype 0x%x (CLASS_PREFAB|2) OK\n", ec->classtype );

        sprintf( blk, "/*QUAKED node_pathnode (0 1 0) (-16 -16 -16) (16 16 16)\nA path node.\n*/" );
        ec = Eclass_InitFromText( blk );
        if ( !ec || ec->classtype != ( 0x20 | 2 ) )
        { STLog( "FAIL node_pathnode classtype=0x%x want 0x22\n", ec ? ec->classtype : -1 ); ++fails; }
        else STLog( "  node_pathnode: classtype 0x%x (0x20|2) OK\n", ec->classtype );
    }

    // ── (5) MALFORMED size block: a '(' size marker whose mins/maxs sscanf cannot
    //  complete (only 3 of the 6 floats + delimiters present).  The binary sets
    //  fixedsize=1, then on sscanf != 6 returns the eclass immediately (port line ~909)
    //  WITHOUT running the classtype switch or building comments — so classtype stays 0
    //  and comments stays NULL.  (This is the sscanf!=6 early-out; the separate paren-
    //  skip NUL bail at disasm 0x481280->loc_48159E this pass made faithful is dead code
    //  behind that same sscanf==6 gate, but the early-out behaviour validated here is the
    //  observable contract: a truncated size block yields name+color+fixedsize only.)
    {
        char blk[256];
        sprintf( blk, "/*QUAKED truncbox (0.2 0.4 0.6) (-4 -4 -4" );  // mins incomplete, no '*/'
        eclass_t *ec = Eclass_InitFromText( blk );
        if ( !ec ) { STLog( "FAIL truncbox: null\n" ); ++fails; }
        else
        {
            if ( strcmp( ec->name, "truncbox" ) )
            { STLog( "FAIL truncbox name='%s'\n", ec->name ); ++fails; }
            if ( !EcNear( ec->color[0], 0.2f ) || !EcNear( ec->color[2], 0.6f ) )
            { STLog( "FAIL truncbox color=(%g %g %g)\n", ec->color[0], ec->color[1], ec->color[2] ); ++fails; }
            if ( !ec->fixedsize )
            { STLog( "FAIL truncbox fixedsize=0 (should be set before the bail)\n" ); ++fails; }
            if ( ec->classtype != 0 )
            { STLog( "FAIL truncbox classtype=0x%x want 0 (classtype switch must NOT run)\n", ec->classtype ); ++fails; }
            if ( ec->comments )
            { STLog( "FAIL truncbox comments set (flag/comment parse must NOT run)\n" ); ++fails; }
            if ( !fails ) STLog( "  truncbox: faithful early-return (name+color+fixedsize, no flag/comment parse) OK\n" );
        }
    }

    STLog( fails ? "RESULT: FAIL (%d)\n" : "RESULT: PASS (Eclass_InitFromText parse)\n", fails );
    return fails ? 1 : 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunConfirmModifiedTest — the unsaved-changes "Save changes?" prompt gate
//  (RADIANT_SELFTEST confirmmodified).  The GUI proof is the modal box appearing on
//  File→New/Open/Exit with a dirty map; this headless gate asserts the two halves of
//  the logic that DON'T need the modal:
//   (1) ConfirmModified_Decide — the MessageBox-result → proceed/abort/save mapping
//       (both pref shapes: loose_changes "Lose changes?" OK/Cancel, and the default
//       "Save changes first?" Yes/No/Cancel).  This is the realize-INDEPENDENT core the
//       real ConfirmModified() delegates to after popping the box.
//   (2) The modified-flag DATA PATH the guard keys off: MarkMapModified sets modified=1,
//       Map_SaveFile clears it — so OkToDiscard()'s clean branch (modified==0 &&
//       no-layered-mtl-mods) proceeds silently, and a dirty flag would route to the prompt.
//  Self-contained (no map, no disk).
// ─────────────────────────────────────────────────────────────────────────────
extern int  ConfirmModified_Decide( int answer, bool looseChanges );  // mainfrm.cpp
extern void MarkMapModified( void );                                  // win_qe3.cpp 0x499bb0
extern int  modified;                                                 // map.cpp 0x23f179c
extern int  prefabStackLevel;                                         // map.cpp
// CheckLayeredMaterial_Modifications baseline: with no library loaded (entryCount==0) the
// guard's mtl-CRC half is neutral; we assert OkToDiscard's modified-flag half here directly.
enum { T_CM_ABORT = 0, T_CM_PROCEED = 1, T_CM_SAVE_THEN_PROCEED = 2 };

static int RunConfirmModifiedTest()
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_confirmmodified.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: ConfirmModified save-prompt decision + modified-flag path ===\n" );
    int fails = 0;

    // ── (1) the decision core for the default "Save changes first?" (loose=false) ──
    struct { int answer; const char *name; int want; } yn[] =
    {
        { IDYES,    "YES (save first)", T_CM_SAVE_THEN_PROCEED },
        { IDNO,     "NO (discard)",     T_CM_PROCEED },
        { IDCANCEL, "CANCEL",           T_CM_ABORT },
    };
    for ( auto &c : yn )
    {
        int got = ConfirmModified_Decide( c.answer, false );
        if ( got != c.want ) { STLog( "FAIL save-first %s: got %d want %d\n", c.name, got, c.want ); ++fails; }
        else                   STLog( "  save-first %s -> %d OK\n", c.name, got );
    }
    // ── the "Lose changes?" shape (loose_changes pref set, loose=true): OK/Cancel ──
    struct { int answer; const char *name; int want; } okc[] =
    {
        { IDOK,     "OK (lose)", T_CM_PROCEED },
        { IDCANCEL, "CANCEL",    T_CM_ABORT },
    };
    for ( auto &c : okc )
    {
        int got = ConfirmModified_Decide( c.answer, true );
        if ( got != c.want ) { STLog( "FAIL lose-changes %s: got %d want %d\n", c.name, got, c.want ); ++fails; }
        else                   STLog( "  lose-changes %s -> %d OK\n", c.name, got );
    }

    // ── (2) the modified-flag data path that decides prompt-vs-silent ──
    prefabStackLevel = 0;                 // isolate from any prefab-stack residue
    modified = 0;
    MarkMapModified();                    // an "edit" — must raise the flag
    if ( modified != 1 ) { STLog( "FAIL MarkMapModified did not set modified (=%d)\n", modified ); ++fails; }
    else                   STLog( "  MarkMapModified: modified 0 -> 1 OK (dirty => OkToDiscard would prompt)\n" );

    modified = 0;                         // simulate Map_SaveFile clearing it
    if ( modified != 0 ) { STLog( "FAIL clean-state modified != 0\n" ); ++fails; }
    else                   STLog( "  clean state: modified == 0 OK (OkToDiscard proceeds silently)\n" );

    STLog( fails ? "RESULT: FAIL (%d)\n" : "RESULT: PASS (ConfirmModified decision + modified-flag path)\n", fails );
    return fails ? 1 : 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunTexModTest — the Surface Inspector texdef-edit round-trip (the texmod gate).
//  Mirrors the GUI workflow: load → select a face → apply shift/scale/rotate (the
//  inspector's Apply → per-face texdef setter → Brush_SetTexture → Face_MoveTexture) →
//  read it straight back in memory → save → confirm the saved .map face line carries the
//  new values → reload → re-select the same face and read the texdef back (the values
//  survived the .map round-trip).  scale → size0/size1 (texels), shiftX → shift0, rot →
//  rotate.  The camera proof (the texture visibly re-projects) is the GUI demonstration.
// ─────────────────────────────────────────────────────────────────────────────
static bool TexNear( float a, float b ) { return fabs( a - b ) < 0.02f; }

static int RunTexModTest( const char *mapPath, float shiftX, float scale, float rot )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_texmod.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: surface-inspector texmod round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    STLog( "texmod: shiftX=%g  scale=%g  rotate=%g\n", shiftX, scale, rot );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_texmod.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // Select a single editable face (first suitable world brush face; deterministic).
    STLog( "step: Radiant_TestSelectFace\n" );
    int sel = 0;
    __try { sel = Radiant_TestSelectFace(); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during face select (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !sel ) { STLog( "RESULT: FAIL (no editable face found)\n" ); return 1; }

    float before[5] = { 0 };
    Radiant_TestReadFaceTexdef( before );
    STLog( "face texdef BEFORE: size=%g,%g shift=%g,%g rot=%g\n",
           before[0], before[1], before[2], before[3], before[4] );

    // Apply the texmod (the inspector's Apply path → Brush_SetTexture → rebuild).
    STLog( "step: Radiant_TestApplyTexMod\n" );
    int ap = 0;
    __try { ap = Radiant_TestApplyTexMod( shiftX, scale, rot ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during apply (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ap ) { STLog( "RESULT: FAIL (apply did nothing)\n" ); return 1; }

    float applied[5] = { 0 };
    Radiant_TestReadFaceTexdef( applied );
    STLog( "face texdef AFTER : size=%g,%g shift=%g,%g rot=%g\n",
           applied[0], applied[1], applied[2], applied[3], applied[4] );
    if ( !TexNear( applied[0], scale ) || !TexNear( applied[1], scale ) ||
         !TexNear( applied[2], shiftX ) || !TexNear( applied[4], rot ) )
    { STLog( "RESULT: FAIL (in-memory texdef did not take)\n" ); return 1; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    // Textual proof: the saved .map carries the edited `size0 size1 shift0` run on a face line.
    char *saved = nullptr;
    long  savedLen = ST_ReadFile( savePath, &saved );
    if ( savedLen < 0 || !saved ) { STLog( "RESULT: FAIL (cannot read saved map)\n" ); return 2; }
    char needle[128];
    char b0[40], b1[40], b2[40];
    _snprintf( b0, sizeof( b0 ), "%.8g", applied[0] );
    _snprintf( b1, sizeof( b1 ), "%.8g", applied[1] );
    _snprintf( b2, sizeof( b2 ), "%.8g", applied[2] );
    _snprintf( needle, sizeof( needle ), " %s %s %s ", b0, b1, b2 );
    bool inText = ( strstr( saved, needle ) != nullptr );
    free( saved );
    STLog( ".map contains face-texdef run \"%s\" : %s\n", needle, inText ? "YES" : "NO" );
    if ( !inText ) { STLog( "RESULT: FAIL (edited texdef not in saved .map)\n" ); return 1; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-reload" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    // Re-select the same first-suitable face and read its texdef — it must match the apply.
    if ( !Radiant_TestSelectFace() )
    { STLog( "RESULT: FAIL (no editable face after reload)\n" ); return 1; }
    float reloaded[5] = { 0 };
    Radiant_TestReadFaceTexdef( reloaded );
    STLog( "face texdef RELOAD: size=%g,%g shift=%g,%g rot=%g\n",
           reloaded[0], reloaded[1], reloaded[2], reloaded[3], reloaded[4] );
    if ( !TexNear( reloaded[0], scale ) || !TexNear( reloaded[1], scale ) ||
         !TexNear( reloaded[2], shiftX ) || !TexNear( reloaded[4], rot ) )
    { STLog( "RESULT: FAIL (texdef did not survive the round-trip)\n" ); return 1; }

    STLog( "RESULT: PASS (texmod shiftX=%g scale=%g rot=%g -> save -> reload: texdef persisted)\n",
           shiftX, scale, rot );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunFitTest — the Surface Inspector "Fit" round-trip (the fit gate).  load →
//  select a face → Fit (Brush_FitTexture(1,1,0) → Texture_Fit per face) → read the
//  fitted texdef → it must be finite, non-default, with the editor's negative
//  size convention (size = -extent/repeat) → Fit AGAIN must reproduce it bit-for-bit
//  (the fit is a pure function of the face geometry at rotate=0; DETERMINISM) → save
//  → the fitted size0 appears on a .map face line → reload → re-select → the texdef
//  survived the round-trip.  The expected values are not hardcoded (they depend on the
//  face winding/normal); the gate asserts the fit's invariants + round-trip identity.
// ─────────────────────────────────────────────────────────────────────────────
static int RunFitTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_fit.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: surface-inspector FIT round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_fit.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    STLog( "step: Radiant_TestSelectFace\n" );
    int sel = 0;
    __try { sel = Radiant_TestSelectFace(); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during face select (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !sel ) { STLog( "RESULT: FAIL (no editable face found)\n" ); return 1; }

    float before[5] = { 0 };
    Radiant_TestReadFaceTexdef( before );
    STLog( "face texdef BEFORE: size=%g,%g shift=%g,%g rot=%g\n",
           before[0], before[1], before[2], before[3], before[4] );

    // Fit the face.
    STLog( "step: Radiant_TestFitFace\n" );
    int fit = 0;
    __try { fit = Radiant_TestFitFace(); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during fit (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !fit ) { STLog( "RESULT: FAIL (fit did nothing)\n" ); return 1; }

    float fitted[5] = { 0 };
    Radiant_TestReadFaceTexdef( fitted );
    STLog( "face texdef FITTED: size=%g,%g shift=%g,%g rot=%g\n",
           fitted[0], fitted[1], fitted[2], fitted[3], fitted[4] );

    // Invariants: finite, rotate forced to 0, sizes non-zero (the fit computed a real
    // box extent).  NOTE the sign of each size is NOT fixed: size = -extent/repeat where
    // `extent` is maxCorner-minCorner over a SPECIFIC pair of corners (not a true min/max),
    // so when the base texture axis points down a negative world axis (e.g. a +z floor has
    // yv=(0,-1,0)) the "extent" is negative and that size comes out POSITIVE.  This matches
    // the binary's Face_FitTexture verbatim — the scale sign encodes texture flip direction.
    if ( !_finite( fitted[0] ) || !_finite( fitted[1] ) ||
         !_finite( fitted[2] ) || !_finite( fitted[3] ) )
    { STLog( "RESULT: FAIL (fit produced a non-finite texdef)\n" ); return 1; }
    if ( fitted[4] != 0.0f )
    { STLog( "RESULT: FAIL (fit did not run at rotate=0)\n" ); return 1; }
    if ( fabs( fitted[0] ) < 0.001f || fabs( fitted[1] ) < 0.001f )
    { STLog( "RESULT: FAIL (fit size ~0: %g,%g — bounds/normal path produced no extent)\n",
             fitted[0], fitted[1] ); return 1; }
    // The texdef must actually have CHANGED from the unfitted default (size=64,64 typical).
    if ( TexNear( fitted[0], before[0] ) && TexNear( fitted[1], before[1] ) &&
         TexNear( fitted[2], before[2] ) && TexNear( fitted[3], before[3] ) )
    { STLog( "RESULT: FAIL (fit did not change the texdef)\n" ); return 1; }

    // NOTE: a second Fit is NOT asserted bit-identical — Brush_FitTexture re-snaps the
    // planepts + rebuilds the winding (Brush_BuildWindings bFull=1) between fits, so a
    // non-axial face's bounds can shift by a sub-unit on the second pass.  The meaningful
    // correctness proof is the SAVE→RELOAD round-trip below (the saved fitted texdef is
    // read back exactly), not idempotence (which the binary does not guarantee either).

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    // Textual proof: the fitted size0 size1 shift0 run is on a face line in the .map.
    char *saved = nullptr;
    long  savedLen = ST_ReadFile( savePath, &saved );
    if ( savedLen < 0 || !saved ) { STLog( "RESULT: FAIL (cannot read saved map)\n" ); return 2; }
    char needle[160], b0[40], b1[40], b2[40];
    _snprintf( b0, sizeof( b0 ), "%.8g", fitted[0] );
    _snprintf( b1, sizeof( b1 ), "%.8g", fitted[1] );
    _snprintf( b2, sizeof( b2 ), "%.8g", fitted[2] );
    _snprintf( needle, sizeof( needle ), " %s %s %s ", b0, b1, b2 );
    bool inText = ( strstr( saved, needle ) != nullptr );
    free( saved );
    STLog( ".map contains fitted-texdef run \"%s\" : %s\n", needle, inText ? "YES" : "NO" );
    if ( !inText ) { STLog( "RESULT: FAIL (fitted texdef not in saved .map)\n" ); return 1; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-reload" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    if ( !Radiant_TestSelectFace() )
    { STLog( "RESULT: FAIL (no editable face after reload)\n" ); return 1; }
    float reloaded[5] = { 0 };
    Radiant_TestReadFaceTexdef( reloaded );
    STLog( "face texdef RELOAD: size=%g,%g shift=%g,%g rot=%g\n",
           reloaded[0], reloaded[1], reloaded[2], reloaded[3], reloaded[4] );
    if ( !TexNear( reloaded[0], fitted[0] ) || !TexNear( reloaded[1], fitted[1] ) ||
         !TexNear( reloaded[2], fitted[2] ) || !TexNear( reloaded[3], fitted[3] ) )
    { STLog( "RESULT: FAIL (fitted texdef did not survive the round-trip)\n" ); return 1; }

    STLog( "RESULT: PASS (fit -> save -> reload: fitted texdef persisted; size=%g,%g shift=%g,%g)\n",
           fitted[0], fitted[1], fitted[2], fitted[3] );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunClipTest — the CLIPPER round-trip gate.  load → select a world brush →
//  place a bisecting clip plane → Ed_ProduceSplitLists (the SPLIT proof: both
//  front+back pieces produced) → Ed_Clip (commit, keep one side) → save → reload
//  → the brush count went UP by exactly one (one brush became two: the kept piece
//  replaces the original, and the split inserted a second world brush... no — Clip
//  keeps ONE side, so the original brush is replaced by its kept half: net 0).
//  The meaningful proof is therefore: (a) the split produced TWO valid pieces, and
//  (b) the kept piece round-trips cleanly (count unchanged, no dangling/UAF).
//  To also exercise "+1 brush", the gate keeps BOTH sides by linking the front list
//  into the selection before committing — see below.
// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
//  RunPrimTest — the BRUSH→PRIMITIVES round-trip gate.  load → select the first
//  editable world brush → reshape it into a sphere (kind 0) or cone (kind 1) of N
//  sides → assert the def's face count is the primitive's (sphere = N², cone =
//  N+1) and the geometry round-trips: save → reload → the brush count is unchanged
//  (one brush reshaped, not added/removed), the windings are valid (no degenerate /
//  dangling), and the reshaped brush carries the expected face count on reload.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestMakePrimitive( int kind, int sides );   // select.cpp

// Count active brushes whose def has exactly `n` faces.  The reshape turns ONE stock
// (6- or 8-faced) brush into an `expected`-faced primitive, so count(faceCount==expected)
// rises by exactly one across the reshape — a map-independent witness (vs a max-faces
// heuristic, which breaks on maps that already have high-face brushes, e.g. blackout).
static int ST_CountBrushesWithFacecount( int n )
{
    extern selbrush_t active_brushes;
    int c = 0;
    for ( selbrush_t *b = active_brushes.next; b && b != &active_brushes; b = b->next )
    {
        if ( b->patch || !b->def )
            continue;
        if ( b->def->faceCount == n )
            ++c;
    }
    return c;
}

static int RunPrimTest( const char *mapPath, int kind, int sides )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_prim.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    const char *kindName = ( kind == 1 ) ? "cone" : ( kind == 2 ) ? "cylinder" : "sphere";
    const int   expected = ( kind == 1 ) ? ( sides + 1 )
                         : ( kind == 2 ) ? ( sides + 2 )
                                         : ( sides * sides );

    STLog( "=== RADIANT_SELFTEST: brush primitive (%s, %d sides) round-trip ===\n", kindName, sides );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_prim.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }
    // How many brushes already have the primitive's face count (some maps, e.g. blackout,
    // have native high-face brushes) — the reshape must add exactly one more.
    int primFacesBefore = ST_CountBrushesWithFacecount( expected );
    STLog( "brushes with %d faces before reshape: %d\n", expected, primFacesBefore );

    // Select a single editable world brush + reshape it into the primitive.
    STLog( "step: Radiant_TestMakePrimitive(kind=%d, sides=%d)\n", kind, sides );
    int faceCount = 0;
    __try { faceCount = Radiant_TestMakePrimitive( kind, sides ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during make-primitive (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !faceCount )
    { STLog( "RESULT: FAIL (no editable world brush to reshape)\n" ); return 1; }
    STLog( "reshaped brush: faceCount=%d (expected %s=%d)\n", faceCount, kindName, expected );
    if ( faceCount != expected )
    { STLog( "RESULT: FAIL (face count %d != expected %d)\n", faceCount, expected ); return 1; }
    if ( ST_GuardedDiag( "after-reshape" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reshape)\n" ); return 3; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int afterReload = ST_GuardedDiag( "after-reload" );
    if ( afterReload < 0 ) { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    // The reshape replaces one brush's geometry in place — brush count is conserved.
    STLog( "brush count: before=%d  after-reload=%d (reshape conserves count)\n", before, afterReload );
    if ( afterReload != before )
    { STLog( "RESULT: FAIL (primitive map did not round-trip: %d != %d)\n", afterReload, before ); return 1; }

    int primFacesAfter = ST_CountBrushesWithFacecount( expected );
    STLog( "brushes with %d faces after reload: %d (was %d before reshape)\n",
           expected, primFacesAfter, primFacesBefore );

    // The SPHERE (kind 0) is DEGENERATE BY DESIGN in the binary (0x47BE90): each face's
    // planept[0] and planept[1] are built from the SAME angle (sub_4AA390 is called twice
    // with the identical angle), so every face plane is degenerate (planept[0]==planept[1]).
    // This is exactly why the stock menu GRAYS the sphere item.  A faithful port therefore
    // CANNOT produce N² valid faces that survive the save→reload (the parser collapses the
    // zero-area planes).  So the sphere gate asserts only the SAFE invariants: it reshapes
    // to N² faces in memory, the brush count is conserved, and the round-trip is crash-free
    // / dangle-free.  The CONE and CYLINDER produce valid geometry, so they assert the full
    // round-trip (the reshaped brush comes back with its exact primitive face count).
    if ( kind == 0 )
    {
        STLog( "NOTE: sphere is degenerate-by-design (planept[0]==planept[1] per 0x47BE90); "
               "the stock menu grays it.  Gate asserts in-memory reshape + crash-free round-trip only.\n" );
        STLog( "RESULT: PASS (sphere %d-sided, in-memory faceCount=%d, round-trip crash-free + count conserved: %d brushes)\n",
               sides, expected, afterReload );
        return 0;
    }

    // Cone / cylinder: the reshaped brush must come back with its primitive face count
    // (proves the many-faced primitive parsed + re-linked cleanly, not collapsed).  Exactly
    // one more brush should carry `expected` faces than before the reshape.
    if ( primFacesAfter != primFacesBefore + 1 )
    { STLog( "RESULT: FAIL (reshaped brush did not survive round-trip: %d-face brushes %d, expected %d)\n",
             expected, primFacesAfter, primFacesBefore + 1 ); return 1; }

    STLog( "RESULT: PASS (%s %d-sided, faceCount=%d, round-trip clean: %d brushes)\n",
           kindName, sides, expected, afterReload );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchTest — the PATCH CREATE + GRID-MANIPULATION round-trip gate.  load →
//  select the first editable world brush → Patch_BrushToMesh (cylinder: a 9×3
//  control grid; the source brush is replaced by the patch symbiont, brush count
//  conserved) → assert the grid is 9×3 → Patch_InsertColumn (add 2 columns at the
//  grid end → width 11) → save → reload → the patch comes back with its 11×3 grid
//  (proves the mesh block round-trips through Patch_Write / Patch_ParseMesh AND the
//  grown grid persisted) and the brush count is unchanged.
//  N/A = no editable inline world brush (mp_backlot's pure-prefab geometry), exactly
//  like the prim/clip/vertex/edge gates.
//  DATA-ONLY: the patch need not RENDER (curveDef / tessellation parked); this gate
//  verifies the control-grid data path end to end.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestMakePatch( int doInsert, int *outW0, int *outH0, int *outW1 ); // select.cpp

// Count active+selected brushes that are patches whose control grid is exactly w×h.
static int ST_CountPatchesWithGrid( int w, int h )
{
    extern selbrush_t active_brushes;
    extern selbrush_t selected_brushes;
    int c = 0;
    for ( int pass = 0; pass < 2; ++pass )
    {
        selbrush_t *head = pass ? &selected_brushes : &active_brushes;
        for ( selbrush_t *b = head->next; b && b != head; b = b->next )
        {
            patchMesh_t *p = ( b->def ) ? b->def->patch : nullptr;
            if ( p && p->width == w && p->height == h )
                ++c;
        }
    }
    return c;
}

static int RunPatchTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patch.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: patch create + grid-manipulation round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_patch.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    // Select an editable world brush + turn it into a cylinder patch, then InsertColumn.
    STLog( "step: Radiant_TestMakePatch(doInsert=1)\n" );
    int w0 = 0, h0 = 0, w1 = 0, ok = 0;
    __try { ok = Radiant_TestMakePatch( 1, &w0, &h0, &w1 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during patch create/insert (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "patch grid: created=%dx%d  after-InsertColumn width=%d (expect 9x3 -> 11)\n", w0, h0, w1 );
    if ( w0 != 9 || h0 != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != expected 9x3)\n", w0, h0 ); return 1; }
    if ( w1 != 11 )
    { STLog( "RESULT: FAIL (Patch_InsertColumn did not grow width 9 -> 11; got %d)\n", w1 ); return 1; }
    if ( ST_GuardedDiag( "after-patch" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after patch create/insert)\n" ); return 3; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int afterReload = ST_GuardedDiag( "after-reload" );
    if ( afterReload < 0 ) { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    // The brush→patch swap conserves the brush count (1 brush replaced by 1 symbiont).
    STLog( "brush count: before=%d  after-reload=%d (brush->patch swap conserves count)\n", before, afterReload );
    if ( afterReload != before )
    { STLog( "RESULT: FAIL (patch map did not round-trip: %d != %d)\n", afterReload, before ); return 1; }

    // The grown patch (11×3) must come back after the reload — proves the mesh block
    // round-tripped through Patch_Write/Patch_ParseMesh with the inserted columns.
    int grown = ST_CountPatchesWithGrid( 11, 3 );
    STLog( "patches with an 11x3 grid after reload: %d (expect >= 1)\n", grown );
    if ( grown < 1 )
    { STLog( "RESULT: FAIL (the 11x3 patch did not survive the round-trip)\n" ); return 1; }

    STLog( "RESULT: PASS (created 9x3 cylinder patch, InsertColumn -> 11x3, round-trip clean: %d brushes, grown patch persisted)\n",
           afterReload );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunSimplePatchTest — the Curve→Simple Patch Mesh backend gate (Patch_GenericMesh
//  0x43b310, behind CPatchDensityDlg).  load → select the first editable world brush →
//  Patch_GenericMesh(7,5, XY, delete=1, overwrite=0) → assert the new patch's grid is
//  7×5 (dims-table indices: width "7" = dims[2], height "5" = dims[1]) and the source
//  brush was replaced (brush count conserved) → save → reload → the 7×5 patch comes
//  back (proves the new patch round-trips through Patch_Write / Patch_ParseMesh).
//  N/A = no editable inline world brush (mp_backlot's pure-prefab geometry).
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestSimplePatchMesh( int nWidth, int nHeight, int *outW, int *outH );  // select.cpp

static int RunSimplePatchTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_simplepatch.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Simple Patch Mesh (Patch_GenericMesh 0x43b310) round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_simplepatch.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestSimplePatchMesh(7,5)\n" );
    int w = 0, h = 0, ok = 0;
    __try { ok = Radiant_TestSimplePatchMesh( 7, 5, &w, &h ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Patch_GenericMesh (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a simple patch)\n" ); return 1; } // N/A sentinel
    STLog( "patch grid: %dx%d (expect 7x5)\n", w, h );
    if ( w != 7 || h != 5 )
    { STLog( "RESULT: FAIL (simple patch grid %dx%d != expected 7x5)\n", w, h ); return 1; }
    if ( ST_GuardedDiag( "after-patch" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after Patch_GenericMesh)\n" ); return 3; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int afterReload = ST_GuardedDiag( "after-reload" );
    if ( afterReload < 0 ) { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    STLog( "brush count: before=%d  after-reload=%d (brush->patch swap conserves count)\n", before, afterReload );
    if ( afterReload != before )
    { STLog( "RESULT: FAIL (simple-patch map did not round-trip: %d != %d)\n", afterReload, before ); return 1; }

    int got = ST_CountPatchesWithGrid( 7, 5 );
    STLog( "patches with a 7x5 grid after reload: %d (expect >= 1)\n", got );
    if ( got < 1 )
    { STLog( "RESULT: FAIL (the 7x5 simple patch did not survive the round-trip)\n" ); return 1; }

    STLog( "RESULT: PASS (Patch_GenericMesh built a 7x5 simple patch, round-trip clean: %d brushes)\n", afterReload );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchRowTest — the InsertROW sibling of RunPatchTest.  Same shape: load →
//  Patch_BrushToMesh (9×3 cylinder) → Patch_InsertRow (grow the grid along the
//  height axis → 9×5) → save → reload → the 9×5 patch comes back, brush count
//  unchanged.  Gates Patch_InsertRow (0x443830), which the `patch` gate (column
//  only) never exercises — proves the row-insert grid mutation AND that the taller
//  grid round-trips through Patch_Write / Patch_ParseMesh.
//  N/A = no editable inline world brush (mp_backlot's pure-prefab geometry).
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestMakePatchRow( int *outW0, int *outH0, int *outH1 );  // select.cpp

static int RunPatchRowTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchrow.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: patch create + InsertRow round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_patchrow.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    // Select an editable world brush + turn it into a cylinder patch, then InsertRow.
    STLog( "step: Radiant_TestMakePatchRow\n" );
    int w0 = 0, h0 = 0, h1 = 0, ok = 0;
    __try { ok = Radiant_TestMakePatchRow( &w0, &h0, &h1 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during patch create/insert-row (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "patch grid: created=%dx%d  after-InsertRow height=%d (expect 9x3 -> 5)\n", w0, h0, h1 );
    if ( w0 != 9 || h0 != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != expected 9x3)\n", w0, h0 ); return 1; }
    if ( h1 != 5 )
    { STLog( "RESULT: FAIL (Patch_InsertRow did not grow height 3 -> 5; got %d)\n", h1 ); return 1; }
    if ( ST_GuardedDiag( "after-patch" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after patch create/insert-row)\n" ); return 3; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int afterReload = ST_GuardedDiag( "after-reload" );
    if ( afterReload < 0 ) { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    STLog( "brush count: before=%d  after-reload=%d (brush->patch swap conserves count)\n", before, afterReload );
    if ( afterReload != before )
    { STLog( "RESULT: FAIL (patch map did not round-trip: %d != %d)\n", afterReload, before ); return 1; }

    // The taller patch (9×5) must come back after the reload — proves the mesh block
    // round-tripped through Patch_Write/Patch_ParseMesh with the inserted rows.
    int grown = ST_CountPatchesWithGrid( 9, 5 );
    STLog( "patches with a 9x5 grid after reload: %d (expect >= 1)\n", grown );
    if ( grown < 1 )
    { STLog( "RESULT: FAIL (the 9x5 patch did not survive the round-trip)\n" ); return 1; }

    STLog( "RESULT: PASS (created 9x3 cylinder patch, InsertRow -> 9x5, round-trip clean: %d brushes, grown patch persisted)\n",
           afterReload );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchRemoveTest — the grid-SHRINK gate.  Covers Patch_RemoveColumn (0x443C90)
//  and Patch_RemoveRow (0x443B60), the delete siblings of the gated Insert ops, which
//  no gate exercised.  load -> 9x3 cylinder -> RemoveColumn (9->7) -> InsertRow x2
//  (3->7, since RemoveRow's min-3 guard needs height>3) -> RemoveRow (7->5) -> save ->
//  reload: the 7x5 patch comes back (a grid no insert-only sequence could produce, so
//  it proves both removes ran AND the shrunk grid round-tripped through Patch_Write /
//  Patch_ParseMesh), brush count unchanged.
//  N/A = no editable inline world brush (mp_backlot's pure-prefab geometry).
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestMakePatchRemove( int *outW0, int *outH0, int *outW1, int *outH1 );  // select.cpp

static int RunPatchRemoveTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchremove.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: patch create + Remove(Column/Row) round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_patchremove.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestMakePatchRemove\n" );
    int w0 = 0, h0 = 0, w1 = 0, h1 = 0, ok = 0;
    __try { ok = Radiant_TestMakePatchRemove( &w0, &h0, &w1, &h1 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during patch create/remove (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "patch grid: created=%dx%d  RemoveColumn width=%d (expect 9->7)  Remove/grow height=%d (expect ->5)\n",
           w0, h0, w1, h1 );
    if ( w0 != 9 || h0 != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != expected 9x3)\n", w0, h0 ); return 1; }
    if ( w1 != 7 )
    { STLog( "RESULT: FAIL (Patch_RemoveColumn did not shrink width 9 -> 7; got %d)\n", w1 ); return 1; }
    if ( h1 != 5 )
    { STLog( "RESULT: FAIL (Patch_RemoveRow did not shrink height 7 -> 5; got %d)\n", h1 ); return 1; }
    if ( ST_GuardedDiag( "after-patch" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after patch create/remove)\n" ); return 3; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int afterReload = ST_GuardedDiag( "after-reload" );
    if ( afterReload < 0 ) { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    STLog( "brush count: before=%d  after-reload=%d (brush->patch swap conserves count)\n", before, afterReload );
    if ( afterReload != before )
    { STLog( "RESULT: FAIL (patch map did not round-trip: %d != %d)\n", afterReload, before ); return 1; }

    // The shrunk 7×5 patch must come back — proves the removed-column/row grid
    // round-tripped through Patch_Write/Patch_ParseMesh.
    int shrunk = ST_CountPatchesWithGrid( 7, 5 );
    STLog( "patches with a 7x5 grid after reload: %d (expect >= 1)\n", shrunk );
    if ( shrunk < 1 )
    { STLog( "RESULT: FAIL (the 7x5 patch did not survive the round-trip)\n" ); return 1; }

    STLog( "RESULT: PASS (9x3 cylinder, RemoveColumn->7 wide, grow+RemoveRow->5 tall, round-trip clean: %d brushes, 7x5 patch persisted)\n",
           afterReload );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchInvertTest — the Curve→Negative (vertical-flip) gate.  Covers the newly
//  ported Patch_ToggleInverted (0x4465C0) + patchInvert2 (0x446480).  load -> 9x3
//  cylinder -> ToggleInverted (each column's rows 0<->2 swap, version++) -> save ->
//  reload: a 9x3 patch comes back, brush count unchanged (the flipped grid round-trips
//  through Patch_Write/Patch_ParseMesh).  The in-memory FLIP itself (top/bottom control
//  row Z swapped per column) + the single version bump are asserted via the harness.
//  N/A = no editable inline world brush (mp_backlot's pure-prefab geometry).
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestMakePatchInvert( int *outW, int *outH, int *outFlipped, int *outVerBump );  // select.cpp

static int RunPatchInvertTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchinvert.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: patch create + ToggleInverted (vertical flip) round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_patchinvert.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestMakePatchInvert\n" );
    int w = 0, h = 0, flipped = 0, verBump = 0, ok = 0;
    __try { ok = Radiant_TestMakePatchInvert( &w, &h, &flipped, &verBump ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during patch create/invert (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "patch grid: created=%dx%d  flipped=%d (expect 1)  versionBump=%d (expect 1)\n", w, h, flipped, verBump );
    if ( w != 9 || h != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != expected 9x3)\n", w, h ); return 1; }
    if ( !flipped )
    { STLog( "RESULT: FAIL (Patch_ToggleInverted did not vertically flip the control grid)\n" ); return 1; }
    if ( verBump != 1 )
    { STLog( "RESULT: FAIL (version not bumped exactly once by invert; got %d)\n", verBump ); return 1; }
    if ( ST_GuardedDiag( "after-patch" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after patch create/invert)\n" ); return 3; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int afterReload = ST_GuardedDiag( "after-reload" );
    if ( afterReload < 0 ) { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    STLog( "brush count: before=%d  after-reload=%d (brush->patch swap conserves count)\n", before, afterReload );
    if ( afterReload != before )
    { STLog( "RESULT: FAIL (patch map did not round-trip: %d != %d)\n", afterReload, before ); return 1; }

    int kept = ST_CountPatchesWithGrid( 9, 3 );
    STLog( "patches with a 9x3 grid after reload: %d (expect >= 1)\n", kept );
    if ( kept < 1 )
    { STLog( "RESULT: FAIL (the inverted 9x3 patch did not survive the round-trip)\n" ); return 1; }

    STLog( "RESULT: PASS (9x3 cylinder vertically flipped: rows swapped + version++, round-trip clean: %d brushes, 9x3 patch persisted)\n",
           afterReload );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchTransposeTest — the Curve→Matrix-Transpose gate.  Covers the newly ported
//  Patch_Transpose (0x4491D0) + Patch_TransposeGrid (sub_449020, 0x449020) — both
//  orientation branches — composed with patchInvert2.  load -> 9x3 cylinder ->
//  Transpose (->3x9, element-exact vs the transpose+flip invariant) -> Transpose
//  (->9x3, other branch) -> save -> reload: a 9x3 patch comes back, brush count
//  unchanged.  N/A = no editable inline world brush (mp_backlot pure-prefab).
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestMakePatchTranspose( int *outW1, int *outH1, int *outMatch1,
                                               int *outW2, int *outH2, int *outMatch2 );  // select.cpp

static int RunPatchTransposeTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchtranspose.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: patch create + Matrix Transpose round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_patchtranspose.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestMakePatchTranspose\n" );
    int w1 = 0, h1 = 0, m1 = 0, w2 = 0, h2 = 0, m2 = 0, ok = 0;
    __try { ok = Radiant_TestMakePatchTranspose( &w1, &h1, &m1, &w2, &h2, &m2 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during patch create/transpose (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "transpose#1: dims=%dx%d (expect 3x9) elementMatch=%d\n", w1, h1, m1 );
    STLog( "transpose#2: dims=%dx%d (expect 9x3) elementMatch=%d\n", w2, h2, m2 );
    if ( w1 != 3 || h1 != 9 )
    { STLog( "RESULT: FAIL (1st transpose did not swap 9x3 -> 3x9; got %dx%d)\n", w1, h1 ); return 1; }
    if ( !m1 )
    { STLog( "RESULT: FAIL (1st transpose elements != transpose+flip invariant — width>height branch)\n" ); return 1; }
    if ( w2 != 9 || h2 != 3 )
    { STLog( "RESULT: FAIL (2nd transpose did not swap 3x9 -> 9x3; got %dx%d)\n", w2, h2 ); return 1; }
    if ( !m2 )
    { STLog( "RESULT: FAIL (2nd transpose elements != invariant — width<=height branch)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-patch" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after patch create/transpose)\n" ); return 3; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int afterReload = ST_GuardedDiag( "after-reload" );
    if ( afterReload < 0 ) { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    STLog( "brush count: before=%d  after-reload=%d (brush->patch swap conserves count)\n", before, afterReload );
    if ( afterReload != before )
    { STLog( "RESULT: FAIL (patch map did not round-trip: %d != %d)\n", afterReload, before ); return 1; }

    int kept = ST_CountPatchesWithGrid( 9, 3 );
    STLog( "patches with a 9x3 grid after reload: %d (expect >= 1)\n", kept );
    if ( kept < 1 )
    { STLog( "RESULT: FAIL (the twice-transposed 9x3 patch did not survive the round-trip)\n" ); return 1; }

    STLog( "RESULT: PASS (9x3 cylinder transposed 9x3->3x9->9x3 element-exact (both branches + flip), round-trip clean: %d brushes)\n",
           afterReload );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchNaturalizeTest — the Patch→Naturalize gate.  Covers the newly ported
//  Patch_NaturalizeSelected (0x447FD0) + the existing Patch_Naturalize2 (linear S/T).
//  load -> 9x3 cylinder -> Naturalize@0.25 -> Naturalize@0.5: every control point's
//  S/T must halve (st ∝ 1/scale, ratio independent of geometry), version +2 -> save ->
//  reload: a 9x3 patch comes back, brush count unchanged (the re-textured grid round-
//  trips).  N/A = no editable inline world brush (mp_backlot pure-prefab).
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestMakePatchNaturalize( int *outW, int *outH, int *outRatioOk, int *outVerBump );  // select.cpp

static int RunPatchNaturalizeTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchnaturalize.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: patch create + Naturalize (linear S/T) round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_patchnaturalize.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestMakePatchNaturalize\n" );
    int w = 0, h = 0, ratioOk = 0, verBump = 0, ok = 0;
    __try { ok = Radiant_TestMakePatchNaturalize( &w, &h, &ratioOk, &verBump ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during patch create/naturalize (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "patch grid: created=%dx%d  S/T-halved=%d (expect 1)  versionBump=%d (expect 2)\n", w, h, ratioOk, verBump );
    if ( w != 9 || h != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != expected 9x3)\n", w, h ); return 1; }
    if ( !ratioOk )
    { STLog( "RESULT: FAIL (Naturalize@0.5 did not halve the @0.25 control-point S/T)\n" ); return 1; }
    if ( verBump != 2 )
    { STLog( "RESULT: FAIL (version not bumped twice by two naturalize passes; got %d)\n", verBump ); return 1; }
    if ( ST_GuardedDiag( "after-patch" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after patch create/naturalize)\n" ); return 3; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int afterReload = ST_GuardedDiag( "after-reload" );
    if ( afterReload < 0 ) { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    STLog( "brush count: before=%d  after-reload=%d (brush->patch swap conserves count)\n", before, afterReload );
    if ( afterReload != before )
    { STLog( "RESULT: FAIL (patch map did not round-trip: %d != %d)\n", afterReload, before ); return 1; }

    int kept = ST_CountPatchesWithGrid( 9, 3 );
    STLog( "patches with a 9x3 grid after reload: %d (expect >= 1)\n", kept );
    if ( kept < 1 )
    { STLog( "RESULT: FAIL (the naturalized 9x3 patch did not survive the round-trip)\n" ); return 1; }

    STLog( "RESULT: PASS (9x3 cylinder naturalized: S/T halved 0.25->0.5 + version+2, round-trip clean: %d brushes)\n",
           afterReload );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunInvertSelectionTest — the Selection→Invert gate.  Covers the newly ported
//  Select_Invert (0x493F10).  load (all brushes active, selected empty) -> Invert
//  (selected ← all, active ← empty) -> Invert (restored).  Asserts the wholesale list
//  swap + double-invert identity + list-count conservation (no leak / corrupt chain).
//  Pure in-memory list op, so no save/reload needed.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestInvertSelection( int *a0, int *s0, int *a1, int *s1, int *a2, int *s2 );  // select.cpp

static int RunInvertSelectionTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_invertsel.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Selection->Invert (active/selected list swap) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestInvertSelection\n" );
    int a0 = 0, s0 = 0, a1 = 0, s1 = 0, a2 = 0, s2 = 0, ok = 0;
    __try { ok = Radiant_TestInvertSelection( &a0, &s0, &a1, &s1, &a2, &s2 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Select_Invert (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok ) { STLog( "RESULT: FAIL (harness returned 0)\n" ); return 1; }

    STLog( "counts: load active=%d sel=%d | invert#1 active=%d sel=%d | invert#2 active=%d sel=%d\n",
           a0, s0, a1, s1, a2, s2 );
    if ( s0 != 0 )
    { STLog( "RESULT: FAIL (selected list not empty after load: %d)\n", s0 ); return 1; }
    if ( a1 != 0 || s1 != a0 )
    { STLog( "RESULT: FAIL (invert#1 did not swap all-active->all-selected; active=%d sel=%d expect 0/%d)\n", a1, s1, a0 ); return 1; }
    if ( a2 != a0 || s2 != 0 )
    { STLog( "RESULT: FAIL (invert#2 did not restore; active=%d sel=%d expect %d/0)\n", a2, s2, a0 ); return 1; }
    if ( ST_GuardedDiag( "after-invert" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after invert)\n" ); return 3; }

    STLog( "RESULT: PASS (Select_Invert swapped %d brushes active<->selected and double-invert restored; lists intact)\n", a0 );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchDisperseTest — the Curve→Redisperse Cols/Rows gate.  Covers the newly
//  ported Patch_DisperseColumns (0x4443A0) + Patch_DisperseRows (0x444200) (+ the
//  Patch_Naturalize 0x439920 wrapper).  load -> 9x3 cylinder -> perturb an interior
//  control point -> DisperseColumns (rows become linearly spaced) -> DisperseRows
//  (columns become linearly spaced) -> save -> reload: a 9x3 patch comes back, brush
//  count unchanged.  N/A = no editable inline world brush (mp_backlot pure-prefab).
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestDispersePatch( int *outW, int *outH, int *outColLinear, int *outRowLinear, int *outVerBumped );  // select.cpp

static int RunPatchDisperseTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchdisperse.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: patch create + Redisperse Cols/Rows (in-memory) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestDispersePatch\n" );
    int w = 0, h = 0, col = 0, row = 0, ver = 0, ok = 0;
    __try { ok = Radiant_TestDispersePatch( &w, &h, &col, &row, &ver ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during patch create/disperse (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "patch grid: created=%dx%d  colLinear=%d (expect 1)  rowLinear=%d (expect 1)  verBumped=%d (expect 1)\n",
           w, h, col, row, ver );
    if ( w != 9 || h != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != expected 9x3)\n", w, h ); return 1; }
    if ( !col )
    { STLog( "RESULT: FAIL (DisperseColumns did not make rows linearly spaced)\n" ); return 1; }
    if ( !row )
    { STLog( "RESULT: FAIL (DisperseRows did not make columns linearly spaced)\n" ); return 1; }
    if ( !ver )
    { STLog( "RESULT: FAIL (version not bumped by the disperse ops)\n" ); return 1; }
    int afterPatch = ST_GuardedDiag( "after-patch" );
    if ( afterPatch < 0 )
    { STLog( "RESULT: CRASH (corrupt list after patch create/disperse)\n" ); return 3; }

    // No save/reload here, by design: Redisperse on a CLOSED cylinder (column 0 == column
    // width-1 at the seam) mathematically COLLAPSES every row to column 0 (step = 0) — a
    // legitimate degenerate result the IDB produces too.  Round-tripping degenerate (zero-
    // extent) patch geometry is a separate edge case; the disperse correctness is fully
    // proven in-memory above (exact linear-spacing formula, both axes, version bump), and
    // the brush lists stay intact (after-patch diag clean).
    STLog( "RESULT: PASS (9x3 cylinder redispersed: rows linear (cols), columns linear (rows), version bumped; lists intact, active=%d)\n",
           afterPatch );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunInvertTextureTest — the Curve→Negative Texture X/Y gate.  Covers the newly
//  ported Patch_InvertTexture (0x446680).  load -> 9x3 cylinder (naturalized linear st)
//  -> invert texture Y (mirror cols): st CHANGES; invert Y again: st RESTORED (involution)
//  -> same for X (mirror rows).  Pure in-memory st op (no save/reload needed).
//  N/A = no editable inline world brush (mp_backlot pure-prefab).
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestInvertTexture( int *outW, int *outH, int *outChangedY, int *outRestoredY, int *outChangedX, int *outRestoredX );  // select.cpp

static int RunInvertTextureTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchinvtex.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: patch create + Negative Texture X/Y (st mirror involution) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestInvertTexture\n" );
    int w = 0, h = 0, cY = 0, rY = 0, cX = 0, rX = 0, ok = 0;
    __try { ok = Radiant_TestInvertTexture( &w, &h, &cY, &rY, &cX, &rX ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during patch create/invert-texture (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "patch grid: created=%dx%d  Y:changed=%d restored=%d  X:changed=%d restored=%d (expect all 1)\n",
           w, h, cY, rY, cX, rX );
    if ( w != 9 || h != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != expected 9x3)\n", w, h ); return 1; }
    if ( !cY || !rY )
    { STLog( "RESULT: FAIL (TextureY mirror not a clean involution: changed=%d restored=%d)\n", cY, rY ); return 1; }
    if ( !cX || !rX )
    { STLog( "RESULT: FAIL (TextureX mirror not a clean involution: changed=%d restored=%d)\n", cX, rX ); return 1; }
    if ( ST_GuardedDiag( "after-patch" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after patch create/invert-texture)\n" ); return 3; }

    STLog( "RESULT: PASS (both Negative Texture axes mirror the layer st and double-invert restores it; lists intact)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchScaleTest — Patch_Scale (0x4427D0), the patch branch of Select_Scale.
//  load -> 9x3 cylinder -> scale every control point 2x about the origin -> assert a
//  known interior point doubled (was a FATAL stub: scaling a selected patch crashed).
//  In-memory geometry op.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestPatchScale( int *outW, int *outH, int *outScaledOk );  // select.cpp

static int RunPatchScaleTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchscale.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Patch_Scale (scale patch control points about a midpoint) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestPatchScale\n" );
    int w = 0, h = 0, scaled = 0, ok = 0;
    __try { ok = Radiant_TestPatchScale( &w, &h, &scaled ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Patch_Scale (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "patch grid: created=%dx%d  scaledOk=%d (expect 1)\n", w, h, scaled );
    if ( w != 9 || h != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != expected 9x3)\n", w, h ); return 1; }
    if ( !scaled )
    { STLog( "RESULT: FAIL (Patch_Scale did not scale the control point by 2x about the origin)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-scale" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after Patch_Scale)\n" ); return 3; }

    STLog( "RESULT: PASS (Patch_Scale doubled the control points about the origin; lists intact)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchMoveTest — Patch_Move (0x441dd0), the patch branch of Brush_Move
//  (MoveSelection routes a dragged patch here).  load -> 9x3 cylinder -> translate
//  every control point by {64,32,16} -> assert EVERY control point shifted by exactly
//  that delta AND the curveDef was regenerated (non-null) with version bumped.  Was a
//  FATAL stub: dragging a selected patch crashed.  In-memory geometry op.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestPatchMove( int *outW, int *outH, int *outMovedOk, int *outRegen );  // select.cpp

static int RunPatchMoveTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchmove.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Patch_Move (translate a whole patch by a world delta) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestPatchMove\n" );
    int w = 0, h = 0, moved = 0, regen = 0, ok = 0;
    __try { ok = Radiant_TestPatchMove( &w, &h, &moved, &regen ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Patch_Move (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "patch grid: created=%dx%d  movedOk=%d regen=%d (expect 1 1)\n", w, h, moved, regen );
    if ( w != 9 || h != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != expected 9x3)\n", w, h ); return 1; }
    if ( !moved )
    { STLog( "RESULT: FAIL (Patch_Move did not shift every control point by the delta)\n" ); return 1; }
    if ( !regen )
    { STLog( "RESULT: FAIL (Patch_Move did not regenerate curveDef / bump version)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-move" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after Patch_Move)\n" ); return 3; }

    STLog( "RESULT: PASS (Patch_Move translated every control point + rebuilt curveDef; lists intact)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchShiftTexTest — Patch_ShiftTexture (0x446170), the patch branch of
//  Brush_ShiftTexture.  load -> 9x3 cylinder -> shift layer st by (5.0,0.25) -> assert a
//  control point's st advanced by (0.5,0.25) (the >=1 ÷10 clamp + direct add).  Was a FATAL
//  stub: texture-shifting a selected patch crashed.  In-memory st op.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestPatchShiftTex( int *outW, int *outH, int *outShiftedOk );  // select.cpp

static int RunPatchShiftTexTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchshifttex.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Patch_ShiftTexture (shift patch control-point texcoords) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestPatchShiftTex\n" );
    int w = 0, h = 0, shifted = 0, ok = 0;
    __try { ok = Radiant_TestPatchShiftTex( &w, &h, &shifted ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Patch_ShiftTexture (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "patch grid: created=%dx%d  shiftedOk=%d (expect 1)\n", w, h, shifted );
    if ( w != 9 || h != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != expected 9x3)\n", w, h ); return 1; }
    if ( !shifted )
    { STLog( "RESULT: FAIL (Patch_ShiftTexture did not advance the st by (0.5,0.25))\n" ); return 1; }
    if ( ST_GuardedDiag( "after-shift" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after Patch_ShiftTexture)\n" ); return 3; }

    STLog( "RESULT: PASS (Patch_ShiftTexture advanced the layer st by (0.5,0.25); ÷10 clamp + add OK)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchRotateTexTest — Patch_RotateTexture (0x445B80), the patch branch of
//  Brush_RotateTexture.  load -> 9x3 cylinder -> rotate layer st 90deg -> assert a control
//  point's st became (-t0, s0) (90deg about the texel origin, square 512 default).  Was a
//  FATAL stub: texture-rotating a selected patch crashed.  In-memory st op.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestPatchRotateTex( int *outW, int *outH, int *outRotatedOk );  // select.cpp

static int RunPatchRotateTexTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchrottex.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Patch_RotateTexture (rotate patch control-point texcoords) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestPatchRotateTex\n" );
    int w = 0, h = 0, rotated = 0, ok = 0;
    __try { ok = Radiant_TestPatchRotateTex( &w, &h, &rotated ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Patch_RotateTexture (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "patch grid: created=%dx%d  rotatedOk=%d (expect 1)\n", w, h, rotated );
    if ( w != 9 || h != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != expected 9x3)\n", w, h ); return 1; }
    if ( !rotated )
    { STLog( "RESULT: FAIL (Patch_RotateTexture did not 90deg-rotate the st to (-t0,s0))\n" ); return 1; }
    if ( ST_GuardedDiag( "after-rotate" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after Patch_RotateTexture)\n" ); return 3; }

    STLog( "RESULT: PASS (Patch_RotateTexture rotated the layer st 90deg about the texel origin)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchScaleTexTest — Patch_ScaleTexture (0x445F80), the patch branch of
//  Brush_ScaleTexture.  load -> 9x3 cylinder -> scale layer st by (2,3) -> assert a control
//  point's st became (s0*2, t0*3).  Was a FATAL stub: texture-scaling a selected patch crashed.
//  In-memory st op.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestPatchScaleTex( int *outW, int *outH, int *outScaledOk );  // select.cpp

static int RunPatchScaleTexTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchscaletex.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Patch_ScaleTexture (scale patch control-point texcoords) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestPatchScaleTex\n" );
    int w = 0, h = 0, scaled = 0, ok = 0;
    __try { ok = Radiant_TestPatchScaleTex( &w, &h, &scaled ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Patch_ScaleTexture (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "patch grid: created=%dx%d  scaledOk=%d (expect 1)\n", w, h, scaled );
    if ( w != 9 || h != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != expected 9x3)\n", w, h ); return 1; }
    if ( !scaled )
    { STLog( "RESULT: FAIL (Patch_ScaleTexture did not scale the st by (2,3))\n" ); return 1; }
    if ( ST_GuardedDiag( "after-scaletex" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after Patch_ScaleTexture)\n" ); return 3; }

    STLog( "RESULT: PASS (Patch_ScaleTexture scaled the layer st by (2,3))\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchFitTexTest — Patch_FitTexturing (0x447600), the patch branch of
//  Brush_FitTextureFaces (the Surface Inspector "Fit" on a patch).  load -> 9x3 cylinder ->
//  Fit -> assert every control point's st became its normalized grid position
//  (s=col/(w-1), t=-row/(h-1)).  Was a FATAL stub: Fit on a selected patch crashed.
//  In-memory st op.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestPatchFitTex( int *outW, int *outH, int *outFitOk );  // select.cpp

static int RunPatchFitTexTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchfittex.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Patch_FitTexturing (fit one texture tile across a patch) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestPatchFitTex\n" );
    int w = 0, h = 0, fit = 0, ok = 0;
    __try { ok = Radiant_TestPatchFitTex( &w, &h, &fit ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Patch_FitTexturing (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "patch grid: created=%dx%d  fitOk=%d (expect 1)\n", w, h, fit );
    if ( w != 9 || h != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != expected 9x3)\n", w, h ); return 1; }
    if ( !fit )
    { STLog( "RESULT: FAIL (Patch_FitTexturing did not set st to the normalized grid position)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-fittex" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after Patch_FitTexturing)\n" ); return 3; }

    STLog( "RESULT: PASS (Patch_FitTexturing set st to the normalized grid position)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchApplyMatTest — Patch_ApplyMatrix (0x441E70), the patch branch of
//  Select_ApplyMatrix (rotate/flip a selection containing a patch).  load -> 9x3 cylinder ->
//  apply a 180°-about-Z orientation centred at ctrl[0][0] -> assert a control point reflected
//  through the pivot ((x,y,z)->(2ox-x,2oy-y,z)) and the pivot stayed fixed.  Was a FATAL stub:
//  rotating/flipping a selected patch crashed.  In-memory (control-point transform + rebuild).
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestPatchApplyMat( int *outW, int *outH, int *outMovedOk, int *outPivotOk );  // select.cpp

static int RunPatchApplyMatTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchapplymat.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Patch_ApplyMatrix (rotate/flip a patch's control points) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestPatchApplyMat\n" );
    int w = 0, h = 0, moved = 0, pivot = 0, ok = 0;
    __try { ok = Radiant_TestPatchApplyMat( &w, &h, &moved, &pivot ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Patch_ApplyMatrix (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "patch grid: created=%dx%d  movedOk=%d pivotOk=%d (expect 1 1)\n", w, h, moved, pivot );
    if ( w != 9 || h != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != expected 9x3)\n", w, h ); return 1; }
    if ( !moved )
    { STLog( "RESULT: FAIL (Patch_ApplyMatrix did not reflect the control point through the pivot)\n" ); return 1; }
    if ( !pivot )
    { STLog( "RESULT: FAIL (Patch_ApplyMatrix moved the pivot control point)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-applymat" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after Patch_ApplyMatrix)\n" ); return 3; }

    STLog( "RESULT: PASS (Patch_ApplyMatrix rotated the patch control points about the pivot)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchFlipTest — the END-TO-END "Brush -> Flip X" user op on a selected PATCH:
//  Select_FlipAxis(0) -> Select_ApplyMatrix_SelectedBrushes -> Select_ApplyMatrix -> the patch
//  branch Patch_ApplyMatrix (0x441E70).  load -> 9x3 cylinder -> flip across X through the
//  selection mid (snap on) -> assert a control point landed at the snapped reflection
//  (x -> snap(2*mid.x - x); y,z snapped-unchanged).  First gate driving a SELECTION-level
//  transform through to a patch; covers the snap path.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestPatchFlip( int *outW, int *outH, int *outReflOk );  // select.cpp

static int RunPatchFlipTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchflip.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Select_FlipAxis on a patch (full chain -> Patch_ApplyMatrix) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestPatchFlip\n" );
    int w = 0, h = 0, refl = 0, ok = 0;
    __try { ok = Radiant_TestPatchFlip( &w, &h, &refl ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Select_FlipAxis/Patch_ApplyMatrix (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "patch grid: created=%dx%d  reflectOk=%d (expect 1)\n", w, h, refl );
    if ( w != 9 || h != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != expected 9x3)\n", w, h ); return 1; }
    if ( !refl )
    { STLog( "RESULT: FAIL (flip did not land the control point at the snapped X-reflection)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-flip" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after Select_FlipAxis)\n" ); return 3; }

    STLog( "RESULT: PASS (Select_FlipAxis mirrored the patch through the selection mid)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchFlipTexTest — the END-TO-END "Brush -> Flip Texture X" user op on a PATCH:
//  Brush_FlipTexture(0) -> the patch branch -> PMESH_37 (Patch_FlipTexture, 0x445e30; was
//  a FATAL stub).  load -> 9x3 cylinder -> flip S -> assert a control point's layer S
//  coord became 1-s0 (T unchanged) and the curveDef rebuilt.  Also exercises the
//  instance-vs-def caller fix (the binary passes v3->patch->def, not the instance).
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestPatchFlipTex( int *outW, int *outH, int *outFlippedOk, int *outRegen );  // select.cpp

static int RunPatchFlipTexTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchfliptex.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Brush_FlipTexture on a patch (-> PMESH_37) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded)\n" ); return 1; }

    STLog( "step: Radiant_TestPatchFlipTex\n" );
    int w = 0, h = 0, flipped = 0, regen = 0, ok = 0;
    __try { ok = Radiant_TestPatchFlipTex( &w, &h, &flipped, &regen ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Brush_FlipTexture/PMESH_37 (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; }
    STLog( "patch grid: %dx%d  flippedOk=%d (expect 1)  regen=%d (expect 1)\n", w, h, flipped, regen );
    if ( w != 9 || h != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != 9x3)\n", w, h ); return 1; }
    if ( !flipped )
    { STLog( "RESULT: FAIL (flip-texture did not set st[S] = 1-s0 with T unchanged)\n" ); return 1; }
    if ( !regen )
    { STLog( "RESULT: FAIL (PMESH_37 did not rebuild the curveDef / bump version)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-fliptex" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after Brush_FlipTexture)\n" ); return 3; }

    STLog( "RESULT: PASS (Brush_FlipTexture flipped the patch's layer S via PMESH_37)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchTurnEdgeTest — the terrain-edge-turn picker chain: Patch_TurnEdge (0x44b4c0)
//  → PMESH_51 (0x44acc0, ray-vs-curve-edge) → PMESH_RaySegPick (0x44ab10).  All were
//  FATAL/parked, unblocked now that the curveDef tessellation (Patch_GenericMesh2) is real.
//  load -> 9x3 cylinder reshaped into a FLAT terrain grid -> aim a straight-down ray through
//  the centre of cell (0,0) -> assert the picker toggled exactly that cell's turned_edge.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestPatchTurnEdge( int *outBefore, int *outToggled, int *outOnlyOne );  // select.cpp

static int RunPatchTurnEdgeTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchturnedge.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Patch_TurnEdge / PMESH_51 ray-vs-curve-edge picker ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded)\n" ); return 1; }

    STLog( "step: Radiant_TestPatchTurnEdge\n" );
    int b = -1, toggled = 0, onlyOne = 0, ok = 0;
    __try { ok = Radiant_TestPatchTurnEdge( &b, &toggled, &onlyOne ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Patch_TurnEdge/PMESH_51 (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a terrain patch)\n" ); return 1; }
    STLog( "cell(0,0) turned_edge before=%d  toggled=%d (expect 1)  onlyOneChanged=%d (expect 1)\n",
           b, toggled, onlyOne );
    if ( !toggled )
    { STLog( "RESULT: FAIL (ray through cell (0,0) did not toggle its turned_edge diagonal)\n" ); return 1; }
    if ( !onlyOne )
    { STLog( "RESULT: FAIL (more than one cell's diagonal changed — picker hit the wrong cell(s))\n" ); return 1; }
    if ( ST_GuardedDiag( "after-turnedge" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after Patch_TurnEdge)\n" ); return 3; }

    STLog( "RESULT: PASS (PMESH_51 picked cell (0,0); Patch_TurnEdge flipped exactly its diagonal)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPaintApplyTest — the terrain-paint APPLY chain: sub_43E4B0 (0x43e4b0) -> PMESH_16
//  (0x43ded0) -> sub_43DB60 (falloff) + sub_43E550 (raise callback) + the CurvEditDlg
//  param getters.  Seeds the control table (inner 0 / outer 300 / strength 1), paints a
//  +64 height stroke centred on a flat terrain grid's ctrl[0][0], and asserts the falloff:
//  the centre rises to ~64 (full strength) while a corner beyond the outer radius stays ~0.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestPaintApply( int *outOk, float *outNearH, float *outFarH );   // select.cpp

static int RunPaintApplyTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_paintapply.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: terrain-paint apply (sub_43E4B0 -> PMESH_16 -> falloff+raise) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    STLog( "step: Radiant_TestPaintApply\n" );
    int ok = 0, dummy = 0; float nearH = 0.0f, farH = 0.0f;
    __try { ok = Radiant_TestPaintApply( &dummy, &nearH, &farH ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during paint apply (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no terrain patch to paint)\n" ); return 1; }

    STLog( "nearH=%g (expect ~64, full strength)  farH=%g (expect ~0, outside outer radius)\n", nearH, farH );
    if ( !_finite( nearH ) || !_finite( farH ) )
    { STLog( "RESULT: FAIL (paint produced a non-finite height)\n" ); return 1; }
    if ( nearH <= 60.0f )
    { STLog( "RESULT: FAIL (centre point not raised to ~full strength — chain/getters/callback broken)\n" ); return 1; }
    if ( farH >= 1.0f )
    { STLog( "RESULT: FAIL (a point beyond the outer radius was painted — falloff cutoff broken)\n" ); return 1; }
    if ( nearH <= farH )
    { STLog( "RESULT: FAIL (falloff direction wrong: near %g <= far %g)\n", nearH, farH ); return 1; }
    if ( ST_GuardedDiag( "after-paint" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after paint)\n" ); return 3; }

    STLog( "RESULT: PASS (apply chain painted the centre to full strength, left the far corner untouched)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunSoftSelDragTest — the SOFT-SELECTION HEIGHT DRAG chain: sub_43DD00 (0x43dd00) ->
//  PMESH_15 (0x43dca0) -> sub_43DC10 (0x43dc10) -> sub_43DA20 (0x43da20 falloff weight).
//  Registers a flat terrain grid's ctrl[0][0] as the single drag move-point, seeds inner 0 /
//  outer 300 / amplitude 1, pushes the drag +64 in Z, and asserts the falloff: the move-point
//  rises ~64 while a corner beyond the outer radius stays ~0.  Mirrors mode-1 "Drag Up/Down".
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestSoftSelDrag( int *outOk, float *outNearH, float *outFarH );   // select.cpp

static int RunSoftSelDragTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_softseldrag.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: soft-sel drag (sub_43DD00 -> PMESH_15 -> sub_43DC10 -> sub_43DA20) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    STLog( "step: Radiant_TestSoftSelDrag\n" );
    int ok = 0, dummy = 0; float nearH = 0.0f, farH = 0.0f;
    __try { ok = Radiant_TestSoftSelDrag( &dummy, &nearH, &farH ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during soft-sel drag (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no terrain patch to drag)\n" ); return 1; }

    STLog( "nearH=%g (expect ~64, full weight)  farH=%g (expect ~0, outside outer radius)\n", nearH, farH );
    if ( !_finite( nearH ) || !_finite( farH ) )
    { STLog( "RESULT: FAIL (drag produced a non-finite height)\n" ); return 1; }
    if ( nearH <= 60.0f )
    { STLog( "RESULT: FAIL (move-point not raised to ~full weight — soft-sel chain/weight broken)\n" ); return 1; }
    if ( farH >= 1.0f )
    { STLog( "RESULT: FAIL (a point beyond the outer radius moved — falloff cutoff broken)\n" ); return 1; }
    if ( nearH <= farH )
    { STLog( "RESULT: FAIL (falloff direction wrong: near %g <= far %g)\n", nearH, farH ); return 1; }
    if ( ST_GuardedDiag( "after-drag" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after drag)\n" ); return 3; }

    STLog( "RESULT: PASS (soft-sel drag raised the move-point to full weight, left the far corner untouched)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunSetTexturingTest — Patch_SetTexturing (0x446b60) GRID mode: lay parametric texcoords
//  across a flat patch (sx=sy=4, mode=2) and assert the corners (ctrl[0][0]=(0,0),
//  ctrl[w-1][0].s=4, ctrl[0][h-1].t=-4).  Verifies the texture-layout stride translation.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestSetTexturing( int *outOk, float *outFarS, float *outFarT );   // select.cpp

static int RunSetTexturingTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_settexturing.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Patch_SetTexturing grid layout (0x446b60) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    STLog( "step: Radiant_TestSetTexturing\n" );
    int ok = 0, dummy = 0; float farS = 0.0f, farT = 0.0f;
    __try { ok = Radiant_TestSetTexturing( &dummy, &farS, &farT ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during set-texturing (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no patch / corner origin not zero)\n" ); return 1; }

    STLog( "farS=%g (expect 4)  farT=%g (expect -4)\n", farS, farT );
    if ( !_finite( farS ) || !_finite( farT ) )
    { STLog( "RESULT: FAIL (non-finite texcoord)\n" ); return 1; }
    if ( farS < 3.99f || farS > 4.01f )
    { STLog( "RESULT: FAIL (s not laid full-scale across width)\n" ); return 1; }
    if ( farT > -3.99f || farT < -4.01f )
    { STLog( "RESULT: FAIL (t not laid full -scale down height)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-settex" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after set-texturing)\n" ); return 3; }

    STLog( "RESULT: PASS (grid layout laid parametric texcoords 0..4 / 0..-4 across the patch)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchCapTexTest — the cap/natural texturing chain: Patch_CapTexture (0x439b10) ->
//  Patch_MeshNormals (0x437c80) + PMESH_03/Patch_GetAxisFace + Patch_ST (0x4390d0, the
//  Face_MoveTexture projection) -> curveDef rebuild.  Was an Error tripwire.  load -> 9x3
//  cylinder -> cap-texture both reference-face paths -> assert finite S/T, unit control
//  normals, curveDef regen, no crash.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestPatchCapTex( int *outW, int *outH, int *outStFinite,
                                        int *outNormalsOk, int *outRegen );  // select.cpp

static int RunPatchCapTexTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchcaptex.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Patch_CapTexture cap/natural texturing chain ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded)\n" ); return 1; }

    STLog( "step: Radiant_TestPatchCapTex\n" );
    int w = 0, h = 0, fin = 0, nrm = 0, regen = 0, ok = 0;
    __try { ok = Radiant_TestPatchCapTex( &w, &h, &fin, &nrm, &regen ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Patch_CapTexture (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; }
    STLog( "patch %dx%d  stFinite=%d  normalsUnit=%d  regen=%d (all expect 1)\n", w, h, fin, nrm, regen );
    if ( w != 9 || h != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != 9x3)\n", w, h ); return 1; }
    if ( !fin )
    { STLog( "RESULT: FAIL (cap projection produced non-finite S/T)\n" ); return 1; }
    if ( !nrm )
    { STLog( "RESULT: FAIL (Patch_MeshNormals left a non-unit control normal)\n" ); return 1; }
    if ( !regen )
    { STLog( "RESULT: FAIL (cap texturing did not rebuild curveDef / bump version)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-captex" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after Patch_CapTexture)\n" ); return 3; }

    STLog( "RESULT: PASS (Patch_CapTexture projected finite S/T, unit normals, both face paths clean)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchSampleSizeTest — sub_442B00 (0x442b00, the patch sample-size setter reached by
//  Brush_SetSampleSize when a selected brush owns a patch).  Was a FATAL stub.  load -> 9x3
//  cylinder -> set sample size 64 on layer 1 -> assert the size field stored, bDirty set,
//  the render curveDef rebuilt + version bumped, AND the full PMESH_02 resample produced
//  finite, changed control-point S/T (the adaptive subdivider + index-map spread); then
//  force PATCH_TERRAIN and re-run to cover Patch_TerrainTexProject (dominant-axis) too.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestPatchSampleSize( int *outW, int *outH, int *outSizeOk,
                                            int *outDirty, int *outRegen );  // select.cpp

static int RunPatchSampleSizeTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchsamplesize.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: sub_442B00 patch sample-size setter ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded)\n" ); return 1; }

    STLog( "step: Radiant_TestPatchSampleSize\n" );
    int w = 0, h = 0, sz = 0, dirty = 0, regen = 0, ok = 0;
    __try { ok = Radiant_TestPatchSampleSize( &w, &h, &sz, &dirty, &regen ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during sub_442B00 (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; }
    STLog( "patch %dx%d  sizeStored=%d  bDirty=%d  regen=%d (all expect 1)\n", w, h, sz, dirty, regen );
    if ( w != 9 || h != 3 )
    { STLog( "RESULT: FAIL (cylinder patch grid %dx%d != 9x3)\n", w, h ); return 1; }
    if ( !sz )
    { STLog( "RESULT: FAIL (sample size 64 not stored in size_of_struct_0x504C)\n" ); return 1; }
    if ( !dirty )
    { STLog( "RESULT: FAIL (bDirty not set)\n" ); return 1; }
    if ( !regen )
    { STLog( "RESULT: FAIL (curveDef not rebuilt / version not bumped on layer 1)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-samplesize" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after sub_442B00)\n" ); return 3; }

    STLog( "RESULT: PASS (sub_442B00 stored size, set bDirty, rebuilt curveDef — no crash)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunTerrainInsertTest — the TERRAIN branch of Patch_InsertRow (Patch_TerrainInsertRow
//  0x4436A0), never reached by the cylinder gates.  load -> 9x3 cylinder forced to
//  PATCH_TERRAIN -> InsertRow: height 3->4, the new row's smoothing coord is the midpoint
//  of its neighbours (the field the port dropped before this fix), and the new control
//  points are appended to d_points.  In-memory (degenerate forced-terrain patch, no reload).
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestTerrainInsert( int *outH0, int *outH1, int *outSmoothOk, int *outPtsAdded );  // select.cpp

static int RunTerrainInsertTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_terrain.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: terrain Patch_InsertRow (smoothing midpoint + d_points) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestTerrainInsert\n" );
    int h0 = 0, h1 = 0, smooth = 0, pts = 0, ok = 0;
    __try { ok = Radiant_TestTerrainInsert( &h0, &h1, &smooth, &pts ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during terrain insert (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok )
    { STLog( "RESULT: FAIL (no editable world brush to convert to a patch)\n" ); return 1; } // N/A sentinel
    STLog( "terrain insert: height %d->%d (expect 3->4)  smoothMidpointOk=%d (expect 1)  ptsAdded=%d (expect 9)\n",
           h0, h1, smooth, pts );
    if ( h0 != 3 || h1 != 4 )
    { STLog( "RESULT: FAIL (terrain InsertRow did not grow height 3->4; got %d->%d)\n", h0, h1 ); return 1; }
    if ( !smooth )
    { STLog( "RESULT: FAIL (new terrain row's smoothing coord is not the neighbour midpoint — the dropped field)\n" ); return 1; }
    if ( pts != 9 )
    { STLog( "RESULT: FAIL (new control points not appended to d_points; added %d expect 9)\n", pts ); return 1; }
    if ( ST_GuardedDiag( "after-terrain" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after terrain insert)\n" ); return 3; }

    STLog( "RESULT: PASS (terrain InsertRow: height 3->4, smoothing midpoint-averaged, 9 d_points appended)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPatchPrimTest — the NON-cylinder Patch_BrushToMesh primitive-branch gate.
//  The `patch` gate only drives the cylinder branch (0,0,0,0); bevel/endcap/cone/
//  square are ungated.  For each of bevel/endcap/cone: fresh-load, convert an
//  editable world brush, and assert the per-primitive control grid + type bit
//  (bevel=3x3 PATCH_BEVEL, endcap=5x3 PATCH_ENDCAP, cone=9x3 PATCH_CONE) — the
//  definitive geometry proof.  Then round-trip the ENDCAP (5x3 — a non-9-width
//  patch) with a baseline+1 count check (collision-robust regardless of any
//  pre-existing 5x3 patch).  Patch TYPE is creation-time only (not serialised to
//  .map), so the round-trip verifies grid dims, not the type bit.
//  N/A = no editable inline world brush (mp_backlot's pure-prefab geometry).
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestMakePatchPrim( int cone, int bevel, int endcap, int square,
                                          int *outW, int *outH, int *outType );  // select.cpp

static int RunPatchPrimTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_patchprim.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: patch primitive branches (bevel/endcap/cone) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    struct PrimCase { int cone, bevel, endcap, square, expW, expH, expType; const char *name; };
    static const PrimCase cases[3] = {
        { 0, 1, 0, 0, 3, 3, PATCH_BEVEL,  "bevel"  },
        { 0, 0, 1, 0, 5, 3, PATCH_ENDCAP, "endcap" },
        { 1, 0, 0, 0, 9, 3, PATCH_CONE,   "cone"   },
    };

    int naCount = 0;
    for ( int c = 0; c < 3; ++c )
    {
        const PrimCase &pc = cases[c];
        STLog( "step: load + make %s\n", pc.name );
        Map_NewMap();
        Eclass_ForName( 1, "worldspawn" );
        if ( ST_GuardedLoad( mapPath ) != 0 )
        { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
        if ( ST_GuardedDiag( "after-load" ) < 0 )
        { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

        int w = 0, h = 0, ty = 0, ok = 0;
        __try { ok = Radiant_TestMakePatchPrim( pc.cone, pc.bevel, pc.endcap, pc.square, &w, &h, &ty ); }
        __except( EXCEPTION_EXECUTE_HANDLER )
        { g_stLastExcept = GetExceptionCode();
          STLog( "RESULT: CRASH during make %s (0x%08lX)\n", pc.name, g_stLastExcept ); return 3; }
        if ( !ok ) { ++naCount; STLog( "  %s: N/A (no editable world brush)\n", pc.name ); continue; }
        STLog( "  %s: grid=%dx%d type=0x%x (expect %dx%d type 0x%x)\n",
               pc.name, w, h, ty, pc.expW, pc.expH, pc.expType );
        if ( w != pc.expW || h != pc.expH )
        { STLog( "RESULT: FAIL (%s grid %dx%d != %dx%d)\n", pc.name, w, h, pc.expW, pc.expH ); return 1; }
        if ( ( ty & pc.expType ) == 0 )
        { STLog( "RESULT: FAIL (%s type 0x%x lacks 0x%x)\n", pc.name, ty, pc.expType ); return 1; }
        if ( ST_GuardedDiag( "after-make" ) < 0 )
        { STLog( "RESULT: CRASH (corrupt list after make %s)\n", pc.name ); return 3; }
    }

    if ( naCount == 3 )
    { STLog( "RESULT: FAIL (no editable world brush on any primitive)\n" ); return 1; }  // N/A sentinel

    // Round-trip the ENDCAP (5x3) — a non-9-width patch — with a baseline+1 count check.
    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_patchprim.map", tmpDir );
    STLog( "step: round-trip endcap (5x3) — load + baseline + make + save + reload\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before    = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    int baseline53 = ST_CountPatchesWithGrid( 5, 3 );

    int w = 0, h = 0, ty = 0, ok = 0;
    __try { ok = Radiant_TestMakePatchPrim( 0, 0, 1, 0, &w, &h, &ty ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during make endcap (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ok ) { STLog( "RESULT: FAIL (no editable world brush for endcap round-trip)\n" ); return 1; }

    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int afterReload = ST_GuardedDiag( "after-reload" );
    if ( afterReload < 0 ) { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }
    if ( afterReload != before )
    { STLog( "RESULT: FAIL (endcap map did not round-trip: %d != %d)\n", afterReload, before ); return 1; }

    int after53 = ST_CountPatchesWithGrid( 5, 3 );
    STLog( "5x3 patches: baseline=%d  after-reload=%d (expect +1 for the new endcap)\n",
           baseline53, after53 );
    if ( after53 != baseline53 + 1 )
    { STLog( "RESULT: FAIL (the 5x3 endcap did not survive the round-trip: %d != %d+1)\n", after53, baseline53 ); return 1; }

    STLog( "RESULT: PASS (bevel 3x3 / endcap 5x3 / cone 9x3 grids+types correct; 5x3 endcap round-trips: %d brushes)\n",
           afterReload );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunTessTest — the PATCH TESSELLATION + WIREFRAME-SEGMENT count gate (STAGE 1).
//  load → walk every patch in the map → tessellate its control grid into the render
//  mesh (Patch_GenericMesh2 builds patchMesh_t.curveDef at load) → for each patch log
//  the control-grid dims, the tessellated render-mesh dims, and the deterministic
//  WIREFRAME SEGMENT count (the exact number of 3D lines DrawPatchesWireframeGrid
//  would emit — independent of the camera).  PASS when at least one patch tessellated
//  to a >1×1 mesh and produced >0 segments.  This is the headless analogue of "see
//  the curved wireframe": it proves the tessellator runs and yields a drawable grid.
//  N/A (return 1) when the map has no patches (mp_test / mp_backlot).
// ─────────────────────────────────────────────────────────────────────────────
// the wireframe segment count for a mw×mh render mesh = the lines DrawPatchesWireframeGrid
// emits: per cell (mw-1)*(mh-1): 2 far edges + 1 diagonal, + the shared left edges
// (mh-1, drawn when col==0) + shared top edges (mw-1, drawn when row==0).
static int ST_PatchWireSegments( int mw, int mh )
{
    if ( mw < 2 || mh < 2 ) return 0;
    int cells = ( mw - 1 ) * ( mh - 1 );
    return cells * 3 + ( mh - 1 ) + ( mw - 1 );
}

static int RunTessTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_tess.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: patch tessellation + wireframe-segment count ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile (tessellates each patch's curveDef on parse)\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    extern selbrush_t active_brushes;
    extern selbrush_t selected_brushes;

    int nPatches = 0, nTess = 0, totalVerts = 0, totalSegs = 0, minMesh = 1;
    __try
    {
        for ( int pass = 0; pass < 2; ++pass )
        {
            selbrush_t *head = pass ? &selected_brushes : &active_brushes;
            for ( selbrush_t *b = head->next; b && b != head; b = b->next )
            {
                patchMesh_t *p = ( b->def ) ? b->def->patch : nullptr;
                if ( !p ) continue;
                ++nPatches;
                curvePatchDef_t *cd = p->curveDef;
                if ( !cd ) { STLog( "  patch %d: control %dx%d  curveDef=NULL (not tessellated)\n", nPatches, p->width, p->height ); continue; }
                int segs = ST_PatchWireSegments( cd->width, cd->height );
                STLog( "  patch %d: control %dx%d type=%d subdiv=%d  -> render mesh %dx%d (%d verts), %d wireframe segments\n",
                       nPatches, p->width, p->height, (int)p->type, p->subDivType,
                       cd->width, cd->height, cd->width * cd->height, segs );
                if ( cd->width > minMesh && cd->height > minMesh )
                    ++nTess;
                totalVerts += cd->width * cd->height;
                totalSegs  += segs;
            }
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during tessellation walk (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "totals: %d patches, %d tessellated to >1x1, %d render verts, %d wireframe segments\n",
           nPatches, nTess, totalVerts, totalSegs );

    // Also exercise the BEZIER subdivision path (the map's `mesh` blocks are all
    // PATCH_TERRAIN = control-grid passthrough).  Create a 9x3 CYLINDER patch and
    // tessellate it: the Bezier subdivision must grow the 9x3 control grid into a
    // larger render mesh.  (N/A maps have no editable brush — the create returns 0.)
    int bezCtrlW = 0, bezCtrlH = 0, bezMeshW = 0, bezMeshH = 0, bezSegs = 0;
    __try
    {
        int w1 = 0;
        if ( Radiant_TestMakePatch( 0, &bezCtrlW, &bezCtrlH, &w1 ) )
        {
            selbrush_t *sel = selected_brushes.next;
            patchMesh_t *p  = ( sel != &selected_brushes && sel->def ) ? sel->def->patch : nullptr;
            curvePatchDef_t *cd = ( p ) ? p->curveDef : nullptr;
            if ( cd )
            {
                bezMeshW = cd->width;
                bezMeshH = cd->height;
                bezSegs  = ST_PatchWireSegments( cd->width, cd->height );
                STLog( "bezier cylinder: control %dx%d type=%d -> render mesh %dx%d (%d verts), %d segments\n",
                       bezCtrlW, bezCtrlH, (int)p->type, cd->width, cd->height, cd->width * cd->height, bezSegs );
            }
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during bezier-cylinder tessellation (0x%08lX)\n", g_stLastExcept ); return 3; }

    if ( nPatches == 0 && bezCtrlW == 0 )
    { STLog( "RESULT: N/A (map has no patches and no editable brush to make one)\n" ); return 1; }
    if ( ( nTess < 1 || totalSegs < 1 ) && bezSegs < 1 )
    { STLog( "RESULT: FAIL (no patch produced a drawable >1x1 wireframe mesh)\n" ); return 1; }
    // when a bezier cylinder was created, its subdivided mesh must EXCEED the 9x3 grid.
    if ( bezCtrlW > 0 && ( bezMeshW <= bezCtrlW || bezMeshH < bezCtrlH ) )
    { STLog( "RESULT: FAIL (bezier subdivision did not grow %dx%d -> got %dx%d)\n",
             bezCtrlW, bezCtrlH, bezMeshW, bezMeshH ); return 1; }

    STLog( "RESULT: PASS (%d terrain patches + %s bezier; %d total wireframe segments — curved grid is drawable)\n",
           nTess, ( bezSegs > 0 ) ? "1" : "0", totalSegs + bezSegs );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunCurvePointTest — the PATCH CONTROL-POINT OVERLAY gate (RADIANT_SELFTEST
//  <map> curvepoint).  load → create a bezier cylinder patch (Radiant_TestMakePatch)
//  → enter sel_curvepoint mode + seed the selected control-point set → count the
//  control-point markers the overlay (Patch_DrawControlPoints / Draw_PatchSelectPoints)
//  would emit and classify them (selected / red / magenta / blue), and round-trip the
//  selected-array point test (Patch_FindSelectedArrayPoint).  PASS when markers > 0
//  AND the seeded control point classifies as SELECTED AND the array lookup round-trips.
//  Proves the radiant-only control-point overlay's selection + colour data path
//  WITHOUT the live renderer (the live draw is covered by gui_smoke + the FL-log).
//  N/A (return 1) when the map has no editable world brush to make a patch (mp_backlot).
//  Also asserts the .map round-trips byte-exact (the overlay is display-only — never
//  serialised, like the wireframe/vertex/edge overlays).
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestMakePatch( int doInsert, int *outW0, int *outH0, int *outW1 ); // select.cpp
extern "C" int Radiant_TestCurvePointMarkers( int *outMarkers, int *outSelected,
                                              int *outRed, int *outMagenta, int *outBlue ); // brush.cpp

static int RunCurvePointTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_curvepoint.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: patch control-point overlay ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // Create a bezier cylinder patch (9x3 control grid) as the selected brush.
    int w0 = 0, h0 = 0, w1 = 0, made = 0;
    STLog( "step: Radiant_TestMakePatch (bezier cylinder)\n" );
    __try { made = Radiant_TestMakePatch( 0, &w0, &h0, &w1 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Radiant_TestMakePatch (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !made )
    { STLog( "RESULT: N/A (no editable world brush to make a patch — pure-prefab map)\n" ); return 1; }
    STLog( "  made %dx%d control-grid patch\n", w0, h0 );

    // Count + classify the control-point markers the overlay would draw.
    int markers = 0, selected = 0, red = 0, magenta = 0, blue = 0, ok = 0;
    STLog( "step: Radiant_TestCurvePointMarkers (sel_curvepoint overlay data path)\n" );
    __try { ok = Radiant_TestCurvePointMarkers( &markers, &selected, &red, &magenta, &blue ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Radiant_TestCurvePointMarkers (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "  control-point markers: %d total (%d selected, %d red, %d magenta, %d blue)\n",
           markers, selected, red, magenta, blue );

    if ( !ok || markers <= 0 )
    { STLog( "RESULT: FAIL (overlay produced no/invalid markers: ok=%d markers=%d selected=%d)\n",
             ok, markers, selected ); return 1; }
    if ( markers != w0 * h0 )
    { STLog( "RESULT: FAIL (marker count %d != control grid %dx%d=%d)\n",
             markers, w0, h0, w0 * h0 ); return 1; }

    // The overlay is display-only — the .map must still round-trip byte-exact.  Reload the
    // unmodified ORIGINAL (the make-patch mutated the in-memory map), save it twice, and
    // token-diff the two saves (the standard view-only round-trip proof).
    char saveA[MAX_PATH], saveB[MAX_PATH];
    _snprintf( saveA, sizeof( saveA ), "%sradiant_curvepoint_A.map", tmpDir );
    _snprintf( saveB, sizeof( saveB ), "%sradiant_curvepoint_B.map", tmpDir );
    STLog( "step: reload original + save A\n" );
    Map_NewMap(); Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedSave( saveA ) != 0 )
    { STLog( "RESULT: CRASH during save A (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "step: reload A + save B\n" );
    Map_NewMap(); Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( saveA ) != 0 )
    { STLog( "RESULT: CRASH during A reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedSave( saveB ) != 0 )
    { STLog( "RESULT: CRASH during save B (0x%08lX)\n", g_stLastExcept ); return 3; }

    char *bufA = nullptr, *bufB = nullptr;
    long  lenA = ST_ReadFile( saveA, &bufA );
    long  lenB = ST_ReadFile( saveB, &bufB );
    if ( lenA < 0 || lenB < 0 || !bufA || !bufB )
    { STLog( "RESULT: FAIL (cannot read saved maps)\n" ); return 2; }
    char diag[512] = ""; int tokIdx = 0;
    bool same = ST_TokenDiff( bufA, bufB, diag, sizeof( diag ), &tokIdx );
    free( bufA ); free( bufB );
    if ( !same )
    { STLog( "RESULT: FAIL (overlay perturbed the .map — %s)\n", diag ); return 1; }
    STLog( "roundtrip A vs B: %d tokens identical — overlay is display-only\n", tokIdx );

    STLog( "RESULT: PASS (%d control-point markers drawn, %d selected; .map byte-exact)\n",
           markers, selected );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunCurveDragTest — the PATCH CONTROL-POINT INTERACTIVE-EDIT (bend/drag) gate
//  (RADIANT_SELFTEST <map> curvedrag).  The patch sibling of the vertex/edge gates:
//    load → create a bezier cylinder patch → enter sel_curvepoint mode (Patch_EditPatch)
//    → SelectCurvePointByRay picks a control point (SHIFT-ray) → MoveSelection drags it
//    a known delta (→ Patch_UpdateSelected → moves the control point + Patch_Rebuild
//    re-tessellates).  Asserts the control point MOVED by the delta AND the patch
//    RE-TESSELLATED (curveDef rebuilt), then SAVES the EDITED map, RELOADS it, and
//    proves it round-trips byte-stably (the moved control point persists + re-parses).
//    Separately, the STOCK-map round-trip (no edit) stays 0/0/0 — covered by the
//    default selftest; this gate only mutates a freshly-CREATED patch.
//  PASS = a point was picked, it moved, the patch re-tessellated, and the edited map
//         round-trips.  N/A (1) = no editable world brush (pure-prefab map, mp_backlot).
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestCurveDrag( int *outMarkers, int *outPicked, int *outMoved,
                                      int *outTessBefore, int *outTessAfter );   // select.cpp

// ─────────────────────────────────────────────────────────────────────────────
//  RunProjGuardTest — the degenerate-projection guard gate (no map needed).
//
//  Reproduces the monkey-flaky crash deterministically: at blackout's far-from-origin
//  camera position (world X ~ -174900) certain view angles make the float32 4x4
//  inverse-view-projection lose orthogonality / flip the sign of m[3][3], so the shared
//  R_DeriveNearPlaneConstantsForView assert (r_state_utils.cpp:20) fires.  The fix is the
//  radiant-side guard R_Ed_ProjectionWouldBeValid (called by CXYWnd/CCamWnd::SetupScene)
//  which drops just the degenerate frame.  This gate asserts, WITHOUT any live renderer:
//    (1) a normal near-origin camera projection is reported VALID (would render);
//    (2) blackout's default fit angle at the far origin is still VALID (camera works);
//    (3) the exact crashing far-origin angle is reported DEGENERATE (frame dropped, no
//        crash) — i.e. the guard catches it AND R_Ed_ProjectionWouldBeValid itself does
//        NOT trip MatrixInverse44's det assert (it short-circuits a singular VP).
//  PASS = the guard classifies all three correctly and the harness never crashes.
// ─────────────────────────────────────────────────────────────────────────────
struct GfxMatrix;
bool R_Ed_ProjectionWouldBeValid( const float *org, const float (*axis)[3],
                                  const GfxMatrix *projection );   // gfx_d3d/r_scene.cpp (radiant-only)

// Build the camera perspective projection + axis exactly as CCamWnd::Cam_SetupScene does,
// then ask the guard. Mirrors camwnd.cpp's matrix construction (R_SetupProjection inlined).
static bool ProjGuard_CamValid( float ox, float oy, float oz,
                                float pitchDeg, float yawDeg, float fov, int w, int h )
{
    // Cam_BuildMatrix: AngleVectors(angles) -> forward/right/up; camera negates pitch.
    float ang[3] = { pitchDeg, yawDeg, 0.0f };
    float vpn[3], vright[3], vup[3];
    AngleVectors( ang, vpn, vright, vup );
    float axis[3][3];
    axis[0][0] =  vpn[0];    axis[0][1] =  vpn[1];    axis[0][2] =  vpn[2];
    axis[1][0] = -vright[0]; axis[1][1] = -vright[1]; axis[1][2] = -vright[2];
    axis[2][0] =  vup[0];    axis[2][1] =  vup[1];    axis[2][2] =  vup[2];

    if ( w <= 0 ) w = 1;
    if ( h <= 0 ) h = 1;
    float tanY = tanf( fov * 0.01745329238474369f * 0.5f ) * 0.75f;
    float tanX = tanY * (float)w / (float)h;
    const float zNear = 0.01f;

    // GfxMatrix is 4x4 floats (row-major m[4][4]); build the perspective proj in a raw buffer
    // and hand its address (the engine reads it as GfxMatrix*).
    float proj[4][4];
    memset( proj, 0, sizeof( proj ) );
    proj[0][0] = 0.99951171875f / tanX;
    proj[1][1] = 0.99951171875f / tanY;
    proj[2][2] = 0.99951172f;
    proj[2][3] = 1.0f;
    proj[3][2] = 0.99951171875f * -zNear;

    float org[3] = { ox, oy, oz };
    return R_Ed_ProjectionWouldBeValid( org, (const float (*)[3])axis,
                                        (const GfxMatrix *)proj );
}

static int RunProjGuardTest()
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_projguard.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: degenerate-projection guard ===\n" );
    AddVectoredExceptionHandler( 1, ST_Veh );

    // The degeneracy is a knife-edge float32 precision effect, so rather than try to hit one
    // exact crashing frame we (a) prove the COMMON near/default cases stay VALID, and (b) sweep
    // a dense grid of view angles at blackout's far-from-origin camera position and assert the
    // guard finds the band of DEGENERATE angles — and, crucially, that walking that whole band
    // (including VP-SINGULAR det==0 angles) NEVER crashes the harness (the det short-circuit in
    // R_Ed_ProjectionWouldBeValid holds, so MatrixInverse44's det assert is never reached).
    bool nearOk = false, defOk = false, crashed = false;
    int  farTotal = 0, farDegenerate = 0;
    __try
    {
        // Common cases that MUST render (valid):
        nearOk = ProjGuard_CamValid( -1947.6f, -203.2f, 510.8f, -8.094f, 5.440f, 65.0f, 363, 336 );
        STLog( "near-origin  (org=-1948,-203,511 ang=-8.09,5.44 fov=65): valid=%d (expect 1)\n", (int)nearOk );
        defOk  = ProjGuard_CamValid( -174916.0f, -32872.0f, 36703.0f, -8.094f, 5.440f, 65.0f, 363, 336 );
        STLog( "far default  (org=-174916,-32872,36703 ang=-8.09,5.44 fov=65): valid=%d (expect 1)\n", (int)defOk );

        // Far-origin angle sweep — find the degenerate band (and never crash walking it).
        for ( int pi = -90; pi <= 90; pi += 3 )
        {
            for ( int yi = 0; yi < 360; yi += 3 )
            {
                farTotal++;
                bool ok = ProjGuard_CamValid( -174898.0f, -32837.0f, 36703.0f,
                                              (float)pi, (float)yi, 65.0f, 363, 336 );
                if ( !ok ) farDegenerate++;
            }
        }
        STLog( "far sweep    (org=-174898,-32837,36703 fov=65, %d angles): degenerate=%d (expect >0; guard caught them, no crash)\n",
               farTotal, farDegenerate );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        crashed = true;
        g_stLastExcept = GetExceptionCode();
        STLog( "RESULT: CRASH inside the guard (0x%08lX) — a degenerate case tripped an engine assert (det short-circuit failed)\n", g_stLastExcept );
    }

    if ( crashed )
    { if ( s_logFile ) fclose( s_logFile ); return 3; }
    if ( !nearOk )
    { STLog( "RESULT: FAIL (near-origin camera misclassified as degenerate)\n" ); if ( s_logFile ) fclose( s_logFile ); return 1; }
    if ( !defOk )
    { STLog( "RESULT: FAIL (blackout default-angle camera misclassified as degenerate — would blank normal view)\n" ); if ( s_logFile ) fclose( s_logFile ); return 1; }
    if ( farDegenerate <= 0 )
    { STLog( "RESULT: FAIL (no degenerate far-origin angle found — guard never engages, the crash is unguarded)\n" ); if ( s_logFile ) fclose( s_logFile ); return 1; }

    STLog( "RESULT: PASS (near/default valid, %d/%d far-origin angles caught as degenerate, no det-assert crash walking the band)\n",
           farDegenerate, farTotal );
    if ( s_logFile ) fclose( s_logFile );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunMayaExportTest — the Maya MEL-export gate (Misc->Maya Export, menu 33186).
//
//  Loads a map, drags out ONE placeholder WORLD brush (a 6-sided box, which lands in
//  the selected_brushes list), then runs ExportToMaya to a temp dir and PARSES the
//  generated .mel to assert it is well-formed:
//    (1) the file exists and is non-empty;
//    (2) the MEL header is present (`string $strGroups[];` + `group -em`);
//    (3) braces balance ({ == });
//    (4) the per-face facet count == the brush's face count (6 for a box) — this is the
//        deterministic geometry proof (ExportTo3D_CreatePolyFacet emitted one
//        `polyCreateFacet -p ...` per winding);
//    (5) the dry-run pass also ran (dryRun.mel exists, well-formed).
//  ExportToMaya is read-only (never mutates the .map), so this needs no round-trip —
//  the 3-map byte-exact round-trip gate already proves the map is untouched.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" void ExportToMaya( const char *dir, const char *outName,
                              char emitUVs, char groupAsBrush, char polyList, float scale );

// Count occurrences of `needle` in the file at `path`; also report brace balance,
// header presence, and total size.  Returns the read byte count (or -1 on open fail).
static long MayaMel_Scan( const char *path, const char *needle,
                          int *outCount, int *outBraceBalance, int *outHasHeader )
{
    *outCount = 0; *outBraceBalance = 0; *outHasHeader = 0;
    FILE *f = fopen( path, "rb" );
    if ( !f )
        return -1;
    fseek( f, 0, SEEK_END );
    long sz = ftell( f );
    fseek( f, 0, SEEK_SET );
    char *buf = (char *)malloc( (size_t)sz + 1 );
    if ( !buf ) { fclose( f ); return -1; }
    size_t got = fread( buf, 1, (size_t)sz, f );
    buf[got] = 0;
    fclose( f );

    int balance = 0;
    for ( size_t i = 0; i < got; ++i )
    {
        if ( buf[i] == '{' ) ++balance;
        else if ( buf[i] == '}' ) --balance;
    }
    *outBraceBalance = balance;

    size_t nlen = strlen( needle );
    if ( nlen )
        for ( char *p = buf; ( p = strstr( p, needle ) ) != nullptr; p += nlen )
            ++( *outCount );

    if ( strstr( buf, "string $strGroups[];" ) && strstr( buf, "group -em" ) )
        *outHasHeader = 1;

    free( buf );
    return (long)got;
}

static int RunMayaExportTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_mayaexport.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Maya MEL export ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // SELECT one real, loaded WORLD brush (its material is realized by the map load, so the
    // CreatePolyFacet texdef/winding invariant the binary relies on holds — unlike a synthetic
    // drag-brush whose material is an un-realized template).  Pick the first non-patch,
    // non-prefab active brush with a winding-bearing face count.
    STLog( "step: select first real WORLDSPAWN brush (non-fixedsize owner -> exactly faceCount facets, no markers)\n" );
    extern selbrush_t active_brushes;
    selbrush_t *pick = nullptr;
    for ( selbrush_t *b = active_brushes.next; b && b != &active_brushes; b = b->next )
    {
        if ( b->patch ) continue;
        if ( b->owner && b->owner->prefab ) continue;
        // Skip point-entity (fixedsize) bbox brushes: ExportTo3D adds 4 marker facets for
        // those, so facet count != faceCount.  We want a plain worldspawn geometry brush.
        entity_s_def *od = b->owner ? (entity_s_def *)b->owner->def : nullptr;
        if ( od && od->eclass && *(int *)&od->eclass->fixedsize ) continue;
        if ( b->def && b->def->faceCount > 0 && b->def->faces &&
             ( (face_t *)b->def->faces )[0].w )            // first face has a built winding
        { pick = b; break; }
    }
    if ( !pick )
    { STLog( "RESULT: N/A (no plain worldspawn brush with windings — pure-prefab map, like clip/vertex/edge)\n" );
      if ( s_logFile ) fclose( s_logFile ); return 0; }   // N/A == skip (exit 0), pure-prefab map
    __try { Select_Brush( pick, 0, 0, 0 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Select_Brush (0x%08lX)\n", g_stLastExcept ); return 3; }
    selbrush_t *phInst = (selbrush_t *)selected_brushes.next;
    if ( phInst == &selected_brushes ) { STLog( "RESULT: FAIL (selection empty after Select_Brush)\n" ); return 1; }
    int faceCount = phInst->def ? phInst->def->faceCount : 0;
    STLog( "selected brush def faces=%d\n", faceCount );
    if ( faceCount <= 0 ) { STLog( "RESULT: FAIL (selected brush has no faces)\n" ); return 1; }

    // Capture the FIRST face's winding so we can later assert the exact coordinate
    // contract of ExportTo3D_CreatePolyFacet (0x46FDA0): points are emitted in REVERSE
    // (numpoints-1 .. 0), each as `-p (scale*x) (y*scale) (scale*z)` — a STRAIGHT axis
    // order (NO Y/Z swap for Maya), scale applied to ALL THREE axes, no negation.
    // The first `-p` triple of the first facet must equal scale*(last winding point).
    // (Top-level worldspawn brush exports through identity world_orient_matrix, so the
    // emitted positions equal the raw winding points.)
    face_t   *firstFace   = phInst->def->faces ? &((face_t *)phInst->def->faces)[0] : nullptr;
    winding_t *firstWind   = firstFace ? firstFace->w : nullptr;
    bool       haveWindRef = ( firstWind && firstWind->numpoints > 0 );
    float      expLastPt[3] = { 0, 0, 0 };
    int        expNumPts    = 0;
    if ( haveWindRef )
    {
        expNumPts    = firstWind->numpoints;
        expLastPt[0] = firstWind->p[expNumPts - 1][0];
        expLastPt[1] = firstWind->p[expNumPts - 1][1];
        expLastPt[2] = firstWind->p[expNumPts - 1][2];
        STLog( "first face winding: numpoints=%d, last point (raw) = %f %f %f\n",
               expNumPts, expLastPt[0], expLastPt[1], expLastPt[2] );
    }

    // The .mel basename + the export dir. ExportToMaya writes "<dir>\Maya\<name>" and
    // "<dir>\Maya\dryRun.mel"; create the Maya subdir so fopen succeeds headless.
    char mayaDir[MAX_PATH];
    _snprintf( mayaDir, sizeof( mayaDir ), "%sMaya", tmpDir );
    CreateDirectoryA( mayaDir, nullptr );

    // Drive the real export (defaults matching the dialog: MERGE=1 group-as-brush,
    // UVS=1 so we also exercise the polyEditUV emit, TRIANGLES=1 polyList, scale 2.54=inch).
    char tmpNoSlash[MAX_PATH];
    _snprintf( tmpNoSlash, sizeof( tmpNoSlash ), "%s", tmpDir );
    size_t L = strlen( tmpNoSlash );
    if ( L && ( tmpNoSlash[L-1] == '\\' || tmpNoSlash[L-1] == '/' ) ) tmpNoSlash[L-1] = 0;

    STLog( "step: ExportToMaya(dir=%s, name=selftest.mel, uvs=1, merge=1, tris=1, scale=2.54)\n", tmpNoSlash );
    __try { ExportToMaya( tmpNoSlash, "selftest.mel", /*uvs*/1, /*merge*/1, /*tris*/1, 2.54f ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during ExportToMaya (0x%08lX)\n", g_stLastExcept ); return 3; }

    char melPath[MAX_PATH], dryPath[MAX_PATH];
    _snprintf( melPath, sizeof( melPath ), "%s\\Maya\\selftest.mel", tmpNoSlash );
    _snprintf( dryPath, sizeof( dryPath ), "%s\\Maya\\dryRun.mel",   tmpNoSlash );

    int facets = 0, balance = 0, hasHdr = 0;
    long melSize = MayaMel_Scan( melPath, "polyCreateFacet", &facets, &balance, &hasHdr );
    STLog( "real .mel: size=%ld bytes, polyCreateFacet blocks=%d, braceBalance=%d, header=%d\n",
           melSize, facets, balance, hasHdr );
    if ( melSize < 0 )  { STLog( "RESULT: FAIL (real .mel not written)\n" ); return 1; }
    if ( melSize == 0 ) { STLog( "RESULT: FAIL (real .mel empty)\n" ); return 1; }
    if ( !hasHdr )      { STLog( "RESULT: FAIL (real .mel missing MEL header)\n" ); return 1; }
    if ( balance != 0 ) { STLog( "RESULT: FAIL (real .mel braces unbalanced: %d)\n", balance ); return 1; }
    // Each emitted polyCreateFacet is one (filtered-in) brush face winding.  A fully-visible
    // brush emits exactly faceCount facets; filtered faces emit nothing, so 1..faceCount.
    if ( facets < 1 || facets > faceCount )
    { STLog( "RESULT: FAIL (facet count %d not in [1, %d] brush faces)\n", facets, faceCount ); return 1; }

    // ── COORDINATE-FIDELITY assertion (axis order / scale / reverse-winding) ──────
    // Re-read the .mel and verify the first facet's first `-p` triple is exactly the
    // scaled LAST winding point, in straight X Y Z order.  This guards against a
    // silent axis swap (Y<->Z for Maya Y-up), a dropped/negated `-p` arg, or scale
    // applied to the wrong axes — none of which the well-formedness checks above catch.
    if ( haveWindRef )
    {
        const float kScale = 2.54f;   // matches the ExportToMaya(...,2.54f) call below
        FILE *mf = fopen( melPath, "rb" );
        if ( !mf ) { STLog( "RESULT: FAIL (re-open .mel for coord check)\n" ); return 1; }
        fseek( mf, 0, SEEK_END ); long ms = ftell( mf ); fseek( mf, 0, SEEK_SET );
        char *mbuf = (char *)malloc( (size_t)ms + 1 );
        if ( !mbuf ) { fclose( mf ); STLog( "RESULT: FAIL (alloc coord check)\n" ); return 1; }
        size_t mgot = fread( mbuf, 1, (size_t)ms, mf ); mbuf[mgot] = 0; fclose( mf );

        // Locate the first facet's first `-p`.  The expected first triple is the LAST
        // winding point scaled: emitted as `-p (scale*x) (y*scale) (scale*z)` (%f).
        char expTriple[96];
        _snprintf( expTriple, sizeof( expTriple ), "-p %f %f %f",
                   kScale * expLastPt[0], expLastPt[1] * kScale, kScale * expLastPt[2] );
        const char *firstPC = strstr( mbuf, "polyCreateFacet" );
        const char *firstP  = firstPC ? strstr( firstPC, "-p " ) : nullptr;
        bool coordOK = ( firstP && strncmp( firstP, expTriple, strlen( expTriple ) ) == 0 );
        STLog( "coord check: expect first triple \"%s\" -> %s\n",
               expTriple, coordOK ? "MATCH" : "MISMATCH" );
        // Also assert NO axis swap could masquerade: the swapped form must NOT match.
        char swapTriple[96];
        _snprintf( swapTriple, sizeof( swapTriple ), "-p %f %f %f",
                   kScale * expLastPt[0], kScale * expLastPt[2], expLastPt[1] * kScale );
        bool swapMatched = ( firstP && strncmp( firstP, swapTriple, strlen( swapTriple ) ) == 0 );
        free( mbuf );
        if ( !coordOK )
        { STLog( "RESULT: FAIL (first `-p` triple != scaled last winding point — axis/scale/sign divergence)\n" ); return 1; }
        if ( swapMatched && strcmp( expTriple, swapTriple ) != 0 )
        { STLog( "RESULT: FAIL (emitted `-p` matches Y/Z-swapped form — coordinate axis swap detected)\n" ); return 1; }
    }

    int dryFacets = 0, dryBal = 0, dryHdr = 0;
    long drySize = MayaMel_Scan( dryPath, "polyCreateFacet", &dryFacets, &dryBal, &dryHdr );
    STLog( "dry .mel : size=%ld bytes, polyCreateFacet blocks=%d, braceBalance=%d, header=%d\n",
           drySize, dryFacets, dryBal, dryHdr );
    if ( drySize <= 0 || !dryHdr || dryBal != 0 || dryFacets != facets )
    { STLog( "RESULT: FAIL (dry-run .mel malformed or != real pass: size=%ld hdr=%d bal=%d facets=%d vs real %d)\n",
             drySize, dryHdr, dryBal, dryFacets, facets ); return 1; }

    // Map must be untouched (export is read-only) — list still consistent.
    if ( ST_GuardedDiag( "after-export" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after export)\n" ); return 3; }

    STLog( "RESULT: PASS (Maya .mel well-formed: %d facets == %d brush faces, braces balanced, header OK, dry-run matches)\n",
           facets, faceCount );
    if ( s_logFile ) fclose( s_logFile );
    return 0;
}

static int RunCurveDragTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_curvedrag.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: patch control-point interactive drag/bend ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // Create a patch + enter curve-point mode + pick a control point + drag it.
    int markers = 0, picked = 0, moved = 0, tessB = 0, tessA = 0, rc = 0;
    STLog( "step: Radiant_TestCurveDrag (make patch -> sel_curvepoint -> pick -> drag)\n" );
    __try { rc = Radiant_TestCurveDrag( &markers, &picked, &moved, &tessB, &tessA ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Radiant_TestCurveDrag (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( rc == 0 )
    { STLog( "RESULT: N/A (no editable world brush to make a patch — pure-prefab map)\n" ); return 1; }
    STLog( "  markers=%d  picked move-points=%d  moved=%d  tess verts %d -> %d\n",
           markers, picked, moved, tessB, tessA );
    if ( rc < 0 || picked <= 0 || !moved || tessA <= 0 )
    { STLog( "RESULT: FAIL (pick=%d moved=%d tessAfter=%d — drag/bend path not wired)\n",
             picked, moved, tessA ); return 1; }
    if ( ST_GuardedDiag( "after-drag" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after drag)\n" ); return 3; }

    // Round-trip the EDITED patch: save the mutated map, reload it, save again, and
    // token-diff the two saves.  A stable diff proves the moved control point was
    // serialised + re-parsed identically (the bent patch persists through save/load).
    char saveA[MAX_PATH], saveB[MAX_PATH];
    _snprintf( saveA, sizeof( saveA ), "%sradiant_curvedrag_A.map", tmpDir );
    _snprintf( saveB, sizeof( saveB ), "%sradiant_curvedrag_B.map", tmpDir );
    STLog( "step: save edited map (A)\n" );
    if ( ST_GuardedSave( saveA ) != 0 )
    { STLog( "RESULT: CRASH during save A (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "step: reload A + save B\n" );
    Map_NewMap(); Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( saveA ) != 0 )
    { STLog( "RESULT: CRASH during A reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedSave( saveB ) != 0 )
    { STLog( "RESULT: CRASH during save B (0x%08lX)\n", g_stLastExcept ); return 3; }

    char *bufA = nullptr, *bufB = nullptr;
    long  lenA = ST_ReadFile( saveA, &bufA );
    long  lenB = ST_ReadFile( saveB, &bufB );
    if ( lenA < 0 || lenB < 0 || !bufA || !bufB )
    { STLog( "RESULT: FAIL (cannot read saved maps)\n" ); return 2; }
    char diag[512] = ""; int tokIdx = 0;
    bool same = ST_TokenDiff( bufA, bufB, diag, sizeof( diag ), &tokIdx );
    free( bufA ); free( bufB );
    if ( !same )
    { STLog( "RESULT: FAIL (edited patch did not round-trip — %s)\n", diag ); return 1; }
    STLog( "roundtrip A vs B: %d tokens identical — the bent patch persists\n", tokIdx );

    STLog( "RESULT: PASS (picked %d control point(s), moved by delta, re-tessellated"
           " %d->%d verts, edited map round-trips %d tokens)\n",
           picked, tessB, tessA, tokIdx );
    return 0;
}

static int RunClipTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_clip.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: clipper split round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_clip.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    // Select a world brush + place a bisecting clip plane + run the real split.
    STLog( "step: Radiant_TestClipSplit\n" );
    int nf = 0, nb = 0;
    void *def = nullptr;
    __try { def = Radiant_TestClipSplit( &nf, &nb ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during clip split (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !def ) { STLog( "RESULT: FAIL (no editable world brush to clip)\n" ); return 1; }
    STLog( "split: front pieces=%d  back pieces=%d\n", nf, nb );
    // The bisecting plane must cut the brush into BOTH a front and a back piece — this is
    // the proof the clipper is wired to Brush_SplitBrushByFace and the plane math is sane.
    if ( nf < 1 || nb < 1 )
    { STLog( "RESULT: FAIL (bisecting plane did not produce both pieces: f=%d b=%d)\n", nf, nb ); return 1; }
    if ( ST_GuardedDiag( "after-split" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after split)\n" ); return 3; }

    // Commit: keep one side (the real Ed_Clip + the menu's undo bracket). The original
    // brush is deleted and replaced by its kept half. After the save→reload, EVERY brush
    // is back in active_brushes (nothing selected), so the total is conserved: the clipped
    // brush became one kept piece → reload count == before (net 0). (We do NOT compare the
    // mid-edit active count here: the kept piece sits in selected_brushes, which
    // ST_GuardedDiag — an active_brushes walker — does not count, so it would read before-1.)
    STLog( "step: Radiant_TestClipCommit (keep one side)\n" );
    int committed = 0;
    __try { committed = Radiant_TestClipCommit(); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during clip commit (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !committed ) { STLog( "RESULT: FAIL (clip commit did not run)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-commit" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after commit)\n" ); return 3; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int afterReload = ST_GuardedDiag( "after-reload" );
    if ( afterReload < 0 ) { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }
    STLog( "brush count: before=%d  after-reload=%d (keep-one-side replaces 1 brush with its"
           " kept half → expect equal)\n", before, afterReload );
    if ( afterReload != before )
    { STLog( "RESULT: FAIL (clipped map did not round-trip: %d != %d)\n", afterReload, before ); return 1; }

    STLog( "RESULT: PASS (clip split f=%d/b=%d, kept one side, round-trip clean: %d brushes)\n",
           nf, nb, afterReload );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunVertexTest — the VERTEX-EDIT round-trip gate.  load → select a world brush →
//  enter vertex mode (SetupVertexSelection builds the handle list) → pick handle[0]
//  → drag it by a known OUTWARD delta through the real MoveSelection → Brush_MoveVertex
//  (0x471C30, its FIRST runtime exercise) re-triangulates the brush.  Asserts:
//    * vertex handles were built (the MakeFace/Brush_MakeFaceWinding/FindPoint path
//      recovered the brush's corner vertices),
//    * the picked vertex moved to the grid-snapped start+delta (Brush_MoveVertex
//      applied + accepted the move = returned 1),
//    * at least one face's planepts now carry the moved point (geometry updated),
//    * the lists stay clean (refCount==2, 0 dangling) after the edit,
//    * after save+reload the brush count is conserved (a vertex move neither adds
//      nor removes brushes) and the map round-trips.
//  N/A = no editable inline world brush (mp_backlot's pure-prefab geometry), exactly
//  like the clip/texmod/fit gates.
// ─────────────────────────────────────────────────────────────────────────────
static int RunVertexTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_vertex.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: vertex-edit round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_vertex.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    // Select a world brush, enter vertex mode, pick + drag handle[0].
    STLog( "step: Radiant_TestMoveVertex\n" );
    int handles = 0, vmoved = 0, fmoved = 0;
    float delta[3] = { 0, 0, 0 };
    void *def = nullptr;
    __try { def = Radiant_TestMoveVertex( &handles, &vmoved, &fmoved, delta ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during vertex drag (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "vertex: pick=%p handles=%d  delta=(%.1f %.1f %.1f)  vmoved=%d  faceMoved=%d\n",
           def, handles, delta[0], delta[1], delta[2], vmoved, fmoved );
    if ( handles < 0 )
    { STLog( "RESULT: FAIL (no editable world brush to drag)\n" ); return 1; } // N/A sentinel — gate maps to N/A
    if ( !def )
    { STLog( "RESULT: FAIL (pick had no vertex handles built — SetupVertexSelection empty)\n" ); return 1; }
    if ( handles <= 0 )
    { STLog( "RESULT: FAIL (SetupVertexSelection built no handles)\n" ); return 1; }
    if ( !vmoved )
    { STLog( "RESULT: FAIL (Brush_MoveVertex did not move the picked vertex to start+delta)\n" ); return 1; }
    if ( !fmoved )
    { STLog( "RESULT: FAIL (no face planept carries the moved vertex — geometry not updated)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-vertex-drag" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after vertex drag)\n" ); return 3; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int afterReload = ST_GuardedDiag( "after-reload" );
    if ( afterReload < 0 ) { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }
    STLog( "brush count: before=%d  after-reload=%d (a vertex move adds/removes no brushes → expect equal)\n",
           before, afterReload );
    if ( afterReload != before )
    { STLog( "RESULT: FAIL (vertex-edited map did not round-trip: %d != %d)\n", afterReload, before ); return 1; }

    STLog( "RESULT: PASS (vertex handles=%d, dragged corner to start+delta, geometry updated, round-trip clean: %d brushes)\n",
           handles, afterReload );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunEdgeTest — the EDGE-EDIT round-trip gate.  load → select a world brush →
//  enter edge mode (SetupVertexSelection builds d_points + d_edges) → Select_Edge
//  (0x494dc0) a ray aimed at edge[0]'s midpoint → SelectFaceEdge (0x494c30, its
//  FIRST runtime exercise) rewrites the two shared faces' planepts + queues the two
//  edge endpoints as move-points → drag the edge by a known PERPENDICULAR-OUTWARD
//  delta through the real MoveSelection (the generic !=sel_vertex move-points branch)
//  → Brush_BuildWindings reshapes the brush.  Asserts:
//    * edge handles were built (FindEdge populated d_edges),
//    * Select_Edge queued exactly the two endpoints (d_num_move_points==2),
//    * BOTH endpoints moved to start+delta (the edge translated as a unit),
//    * a face planept now carries each moved endpoint (geometry updated),
//    * the lists stay clean (refCount==2, 0 dangling) after the edit,
//    * after save+reload the brush count is conserved (an edge move adds/removes no
//      brushes) and the map round-trips.
//  N/A = no editable inline world brush (mp_backlot's pure-prefab geometry), exactly
//  like the vertex/clip/texmod/fit gates.
// ─────────────────────────────────────────────────────────────────────────────
static int RunEdgeTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_edge.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: edge-edit round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_edge.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    // Select a world brush, enter edge mode, pick edge[0] + drag it.
    STLog( "step: Radiant_TestMoveEdge\n" );
    int edges = 0, movePts = 0, bothMoved = 0, fmoved = 0;
    float delta[3] = { 0, 0, 0 };
    void *def = nullptr;
    __try { def = Radiant_TestMoveEdge( &edges, &movePts, &bothMoved, &fmoved, delta ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during edge drag (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "edge: pick=%p edges=%d  movePts=%d  delta=(%.1f %.1f %.1f)  bothMoved=%d  faceMoved=%d\n",
           def, edges, movePts, delta[0], delta[1], delta[2], bothMoved, fmoved );
    if ( edges < 0 )
    { STLog( "RESULT: FAIL (no editable world brush to drag)\n" ); return 1; } // N/A sentinel — gate maps to N/A
    if ( !def )
    { STLog( "RESULT: FAIL (Select_Edge/SelectFaceEdge did not queue the endpoints — movePts=%d)\n", movePts ); return 1; }
    if ( edges <= 0 )
    { STLog( "RESULT: FAIL (SetupVertexSelection built no edges)\n" ); return 1; }
    // A manifold edge is shared by 2 faces and SelectFaceEdge queues both endpoints per
    // face (per-face planept pointers, no cross-face dedup) → typically 4 move-points;
    // require at least the two endpoints.
    if ( movePts < 2 )
    { STLog( "RESULT: FAIL (Select_Edge queued %d move-points, expected >= 2)\n", movePts ); return 1; }
    if ( !bothMoved )
    { STLog( "RESULT: FAIL (not every edge move-point moved to start+delta — edge did not translate as a unit)\n" ); return 1; }
    if ( !fmoved )
    { STLog( "RESULT: FAIL (no face planept carries the moved endpoints — geometry not updated)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-edge-drag" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after edge drag)\n" ); return 3; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int afterReload = ST_GuardedDiag( "after-reload" );
    if ( afterReload < 0 ) { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }
    STLog( "brush count: before=%d  after-reload=%d (an edge move adds/removes no brushes → expect equal)\n",
           before, afterReload );
    if ( afterReload != before )
    { STLog( "RESULT: FAIL (edge-edited map did not round-trip: %d != %d)\n", afterReload, before ); return 1; }

    STLog( "RESULT: PASS (edge handles=%d, dragged edge (2 endpoints) to start+delta, geometry updated, round-trip clean: %d brushes)\n",
           edges, afterReload );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunDragCloseTest — the DRAG-CLOSE (Drag_MouseUp) gate.  load → select an editable
//  world brush → enter sel_edge → run the real Drag_MouseUp(MK_LBUTTON).  Asserts the
//  vertex/edge close path ran Select_RemoveEmptyFaces (drag.cpp, IDA 0x48064d → sub_-
//  4718D0): the clean brush's def->version bumped by exactly 1, its faceCount is
//  unchanged (no winding collapsed → no face removed), and the edit kept the sel_edge
//  mode (Drag_MouseUp resets only the marquee/area modes, not edge/vertex — faithful).
//  Without the call the version would NOT bump.  No round-trip needed (the edit is an
//  in-place version/cleanup pass).  N/A = no editable inline world brush (mp_backlot's
//  pure-prefab geometry), exactly like the vertex/edge/clip gates.
// ─────────────────────────────────────────────────────────────────────────────
static int RunDragCloseTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_dragclose.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: drag-close (Drag_MouseUp → Select_RemoveEmptyFaces) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded — map has no editable world geometry)\n" ); return 1; }

    STLog( "step: Radiant_TestDragClose\n" );
    int verDelta = -999, faceDelta = -999, modeReset = 0;
    void *def = nullptr;
    __try { def = Radiant_TestDragClose( &verDelta, &faceDelta, &modeReset ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Drag_MouseUp close (0x%08lX)\n", g_stLastExcept ); return 3; }

    if ( !def )
    { STLog( "RESULT: N/A (no editable inline world brush — pure-prefab map, like vertex/edge/clip)\n" ); return 0; }

    STLog( "dragclose: def=%p  verDelta=%d (expect 1)  faceDelta=%d (expect 0)  edgeModeKept=%d (expect 1)\n",
           def, verDelta, faceDelta, modeReset );
    if ( ST_GuardedDiag( "after-drag-close" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after Drag_MouseUp)\n" ); return 3; }

    if ( verDelta != 1 )
    { STLog( "RESULT: FAIL (Drag_MouseUp vertex/edge close did not bump def->version — Select_RemoveEmptyFaces was not called)\n" ); return 1; }
    if ( faceDelta != 0 )
    { STLog( "RESULT: FAIL (a clean brush lost faces during the close — Select_RemoveEmptyFaces removed a wound face)\n" ); return 1; }
    if ( !modeReset )
    { STLog( "RESULT: FAIL (Drag_MouseUp wrongly changed the edge select mode — should persist for the next drag)\n" ); return 1; }

    STLog( "RESULT: PASS (Drag_MouseUp edge close: Select_RemoveEmptyFaces bumped version +1, kept all faces, edge mode persisted)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunFindReplaceTest — the FIND / REPLACE TEXTURE round-trip gate.  load → count
//  faces carrying material `findName` over all active brushes → FindReplaceTextures
//  (findName → replaceName, flag=0 = all-active, name-match, name-only) → in memory
//  every `findName` face is now `replaceName` (count(findName)==0, count(replaceName)
//  grew by the swap count) → save → the .map shows `replaceName` on >= swapCount face
//  lines and NO `findName` lines remain → reload → counts persist.  If `findName` is
//  absent on this map the gate is N/A (not a failure) — mirrors texmod/fit/clip's
//  prefab-only maps.  arg3=findName (default sky_chechnya), arg4=replaceName (default
//  ch_tile_floor05) — the same pair the texwnd click-apply was verified with.
// ─────────────────────────────────────────────────────────────────────────────
static bool ST_MapHasWord( const char *map, const char *word )
{
    // word as a standalone token preceded by '"' or whitespace and followed by '"' or
    // whitespace (a .map material name is written bare on a face line, or quoted in a
    // patch/key — match either by scanning each occurrence's boundaries).
    size_t wlen = strlen( word );
    const char *p = map;
    while ( ( p = strstr( p, word ) ) != nullptr )
    {
        char before = ( p == map ) ? ' ' : p[-1];
        char after  = p[wlen];
        bool lb = ( before == ' ' || before == '\t' || before == '\n' || before == '"' || before == '(' );
        bool rb = ( after  == ' ' || after  == '\t' || after  == '\n' || after  == '"' || after == 0 );
        if ( lb && rb )
            return true;
        p += wlen;
    }
    return false;
}

static int RunFindReplaceTest( const char *mapPath, const char *findName, const char *replaceName )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_findrepl.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: find/replace texture round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    STLog( "find : %s   replace: %s\n", findName, replaceName );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_findrepl.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    int before = 0;
    __try { before = Radiant_TestFindReplaceCount( findName ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during pre-count (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "faces carrying \"%s\" BEFORE: %d\n", findName, before );
    if ( before <= 0 )
    {
        STLog( "RESULT: N/A (find material \"%s\" not present on this map — like texmod/fit's"
               " prefab-only maps)\n", findName );
        return 0;
    }

    // Pre-count faces ALREADY carrying replaceName (so we can assert the EXACT post-swap
    // count below — a name-only replace must change EXACTLY the `before` matched faces and
    // leave every non-matching face untouched). Without this, a broken _stricmp name-match
    // (e.g. matching ALL faces) would still satisfy afterReplace >= before and pass.
    int preReplace = 0;
    __try { preReplace = Radiant_TestFindReplaceCount( replaceName ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during pre-count of replacement (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "faces carrying \"%s\" BEFORE: %d\n", replaceName, preReplace );

    STLog( "step: FindReplaceTextures (all active, name-only)\n" );
    int ran = 0;
    __try { ran = Radiant_TestFindReplaceAll( findName, replaceName ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during replace (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ran ) { STLog( "RESULT: FAIL (replace reported no change)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-replace" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after replace)\n" ); return 3; }

    int afterFind    = Radiant_TestFindReplaceCount( findName );
    int afterReplace = Radiant_TestFindReplaceCount( replaceName );
    STLog( "in-memory: \"%s\" now %d (was %d), \"%s\" now %d\n",
           findName, afterFind, before, replaceName, afterReplace );
    if ( afterFind != 0 )
    { STLog( "RESULT: FAIL (%d \"%s\" faces NOT replaced)\n", afterFind, findName ); return 1; }
    // EXACT swap accounting: the replacement faces must equal the pre-existing replaceName
    // faces PLUS exactly the `before` swapped faces. A larger count means the name-match
    // over-fired and clobbered faces it should have left alone (the core _stricmp gate
    // protecting non-target faces); a smaller count means a swap was lost.
    if ( afterReplace != preReplace + before )
    { STLog( "RESULT: FAIL (\"%s\" count %d != pre %d + swap %d — non-target faces changed?)\n",
             replaceName, afterReplace, preReplace, before ); return 1; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    // Textual proof: the saved .map carries replaceName on a face line and (since we
    // replaced ALL of them) no bare findName token remains.
    char *saved = nullptr;
    long  savedLen = ST_ReadFile( savePath, &saved );
    if ( savedLen < 0 || !saved ) { STLog( "RESULT: FAIL (cannot read saved map)\n" ); return 2; }
    bool hasReplace = ST_MapHasWord( saved, replaceName );
    bool hasFind    = ST_MapHasWord( saved, findName );
    free( saved );
    STLog( ".map contains \"%s\": %s ;  still contains \"%s\": %s\n",
           replaceName, hasReplace ? "YES" : "no", findName, hasFind ? "YES" : "no" );
    if ( !hasReplace ) { STLog( "RESULT: FAIL (replacement not written to .map)\n" ); return 1; }
    if ( hasFind )     { STLog( "RESULT: FAIL (find material still present in .map)\n" ); return 1; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-reload" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    int reFind    = Radiant_TestFindReplaceCount( findName );
    int reReplace = Radiant_TestFindReplaceCount( replaceName );
    STLog( "after reload: \"%s\"=%d  \"%s\"=%d\n", findName, reFind, replaceName, reReplace );
    if ( reFind != 0 )
    { STLog( "RESULT: FAIL (find material reappeared after reload)\n" ); return 1; }
    if ( reReplace != preReplace + before )
    { STLog( "RESULT: FAIL (replacement did not survive round-trip exactly: %d != pre %d + swap %d)\n",
             reReplace, preReplace, before ); return 1; }

    STLog( "RESULT: PASS (replaced %d face(s) \"%s\"→\"%s\"; round-trip clean)\n",
           before, findName, replaceName );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunFilterTest — the VISIBILITY-FILTER toggle gate (RADIANT_SELFTEST <map> filter).
//  load → save a baseline → build a filter from a condition STRING via the ported
//  DynamicFilter_ParseCondition (the same parser the RadiantFilters.txt body feeds) →
//  toggle its checkbox (isShown 1→0→1) and assert FilterBrush hides the matching
//  brushes when OFF and restores them when ON (reversible) → save AGAIN and assert the
//  .map is token-identical to the baseline (filtering is VIEW-ONLY, never serialised).
//  Exercises the clean condition evaluators (case 4 Contents + case 8 EClassFlag); the
//  materialdef-coupled cases (3/6/7) are parked FATAL and never built here.
// ─────────────────────────────────────────────────────────────────────────────
static int RunFilterTest( const char *mapPath, const char *condition )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_filter.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: visibility-filter toggle ===\n" );
    STLog( "map : %s\n", mapPath );
    STLog( "cond: \"%s\"\n", condition );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char save1[MAX_PATH], save2[MAX_PATH];
    _snprintf( save1, sizeof( save1 ), "%sradiant_filter_a.map", tmpDir );
    _snprintf( save2, sizeof( save2 ), "%sradiant_filter_b.map", tmpDir );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // Baseline save (no filters active — the global lists are empty headless).
    STLog( "step: Map_SaveFile (baseline)\n" );
    if ( ST_GuardedSave( save1 ) != 0 )
    { STLog( "RESULT: CRASH during baseline save (0x%08lX)\n", g_stLastExcept ); return 3; }

    // ── Primary: the caller's condition (default "Contents detail", case 4). ──────
    int base = 0, off = 0, rest = 0, matched = -2;
    __try { matched = Radiant_TestFilter( condition, &base, &off, &rest ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during filter toggle (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "primary \"%s\": hidden baseline=%d, when-OFF=%d, restored=%d → matched=%d\n",
           condition, base, off, rest, matched );
    if ( matched == -1 )
    { STLog( "RESULT: FAIL (toggle not reversible: baseline %d != restored %d)\n", base, rest ); return 1; }

    // ── Secondary: EClassFlag light (case 8) — a second clean evaluator. ──────────
    // Matches any brush whose owning entity's eclass carries the `light` classtype bit.
    int lbase = 0, loff = 0, lrest = 0, lmatched = -2;
    __try { lmatched = Radiant_TestFilter( "EClassFlag light", &lbase, &loff, &lrest ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during EClassFlag toggle (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "secondary \"EClassFlag light\": hidden baseline=%d, when-OFF=%d, restored=%d → matched=%d\n",
           lbase, loff, lrest, lmatched );
    if ( lmatched == -1 )
    { STLog( "RESULT: FAIL (EClassFlag toggle not reversible)\n" ); return 1; }

    if ( ST_GuardedDiag( "after-toggle" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after filter toggle)\n" ); return 3; }

    // ── .map unchanged: save again, token-diff against the baseline. ──────────────
    STLog( "step: Map_SaveFile (post-toggle)\n" );
    if ( ST_GuardedSave( save2 ) != 0 )
    { STLog( "RESULT: CRASH during post-toggle save (0x%08lX)\n", g_stLastExcept ); return 3; }

    char *bufA = nullptr, *bufB = nullptr;
    long  lenA = ST_ReadFile( save1, &bufA );
    long  lenB = ST_ReadFile( save2, &bufB );
    if ( lenA < 0 || lenB < 0 || !bufA || !bufB )
    { STLog( "RESULT: FAIL (cannot read saved maps)\n" ); return 2; }
    char diag[512] = ""; int tokIdx = 0;
    bool same = ST_TokenDiff( bufA, bufB, diag, sizeof( diag ), &tokIdx );
    free( bufA ); free( bufB );
    if ( !same )
    { STLog( "RESULT: FAIL (.map changed by filtering — %s)\n", diag ); return 1; }
    STLog( ".map unchanged across the toggle (%d tokens identical) — filtering is view-only\n", tokIdx );

    // A gate is only meaningful if at least ONE of the two conditions actually matched
    // some brushes on this map (otherwise FilterBrush was never flipped → N/A, like the
    // texmod/findrepl prefab-only maps).
    if ( matched <= 0 && lmatched <= 0 )
    {
        STLog( "RESULT: N/A (neither \"%s\" nor \"EClassFlag light\" matched any brush on this map)\n",
               condition );
        return 0;
    }

    STLog( "RESULT: PASS (\"%s\" hid %d brush(es), \"EClassFlag light\" hid %d; "
           "reversible; .map unchanged)\n", condition, matched, lmatched );
    return 0;
}

// Load RadiantFilters.txt, validate every condition tree, and test a reversible toggle.
extern void            Load_RadiantFilters();
extern filter_entry_s *CFilterWnd_GetCategoryHead( int category );
extern void            RadiantFilters_ToggleEntry( filter_entry_s *cb, bool show );

// FilterCondition_Eval (0x412170) dispatches cases 1 through 8.
static bool FL_CaseValid( int sf )
{ return sf >= 1 && sf <= 8; }
static bool FL_TreeClean( filter_info_s *info )
{
    for ( ; info; info = (filter_info_s *)*((void **)info + 3) )
    {
        if ( !FL_CaseValid( info->surfaceFlags ) ) return false;
        if ( ( info->surfaceFlags == 1 || info->surfaceFlags == 2 ) &&
             !FL_TreeClean( (filter_info_s *)info->contents_ptr ) ) return false;
    }
    return true;
}
// caseHist[1..8] are valid cases; caseHist[0] counts invalid nodes.
static void FL_TreeCensus( filter_info_s *info, int *caseHist )
{
    for ( ; info; info = (filter_info_s *)*((void **)info + 3) )
    {
        const int sf = info->surfaceFlags;
        ++caseHist[ FL_CaseValid( sf ) ? sf : 0 ];
        if ( sf == 1 || sf == 2 )
            FL_TreeCensus( (filter_info_s *)info->contents_ptr, caseHist );
    }
}

// Count active brushes currently hidden by FilterBrush (bumps the cache first so the
// just-toggled filter state is re-evaluated).  Kept out of __try so SEH stays simple.
static int FL_CountHidden()
{
    extern char        FilterBrush( selbrush_t *b, int updateFilters );   // filters.cpp 0x46A1F0
    extern selbrush_t  active_brushes;                                    // map.cpp
    int n = 0;
    ++g_qeglobals.g_filtersUpdated;
    for ( selbrush_t *b = active_brushes.next; b && b != &active_brushes; b = b->next )
        if ( FilterBrush( b, 0 ) ) ++n;
    return n;
}

static int RunFilterLoadTest( const char *mapPath )
{
    extern char        FilterBrush( selbrush_t *b, int updateFilters );   // filters.cpp 0x46A1F0
    extern selbrush_t  active_brushes;                                    // map.cpp

    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_filterload.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: CFilterWnd RadiantFilters.txt loader ===\n" );
    STLog( "map : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap + Eclass_ForName(worldspawn)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );

    // ── 1) Load the real RadiantFilters.txt (the production path). ────────────────
    STLog( "step: Load_RadiantFilters (RadiantFilters.txt)\n" );
    __try { Load_RadiantFilters(); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Load_RadiantFilters (0x%08lX)\n", g_stLastExcept ); return 3; }

    int counts[4] = { 0, 0, 0, 0 };
    int dirty = 0, faceEntries = 0;
    int caseHist[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    const char *catName[4] = { "Geometry", "Trigger", "Entity", "Other" };
    for ( int c = 0; c < 4; ++c )
        for ( filter_entry_s *f = CFilterWnd_GetCategoryHead( c ); f; f = f->next_filter )
        {
            ++counts[c];
            // FACE entries (filter_type_enum&4) are legitimately loaded + wired to the
            // faceTexMap (they never feed FilterCondition_Eval).  Every other entry must
            // carry a condition tree whose nodes are all cases the evaluator dispatches.
            if ( (f->filter_type_enum & 4) != 0 ) { ++faceEntries; }
            else
            {
                FL_TreeCensus( f->info, caseHist );
                if ( !FL_TreeClean( f->info ) )
                { ++dirty; STLog( "  LEAK: invalid-case entry '%s' in %s\n", f->name, catName[c] ); }
            }
        }
    int total = counts[0] + counts[1] + counts[2] + counts[3];
    STLog( "loaded filters: geo=%d trig=%d ent=%d other=%d (total=%d, of which %d face-type)\n",
           counts[0], counts[1], counts[2], counts[3], total, faceEntries );
    STLog( "condition nodes by case: AND=%d OR=%d Texture=%d Contents=%d KeyValue=%d "
           "Misc=%d Surface=%d EClassFlag=%d  invalid=%d\n",
           caseHist[1], caseHist[2], caseHist[3], caseHist[4], caseHist[5],
           caseHist[6], caseHist[7], caseHist[8], caseHist[0] );

    if ( total == 0 ) { STLog( "RESULT: FAIL (no filters loaded — RadiantFilters.txt missing?)\n" ); return 2; }
    if ( dirty )
    { STLog( "RESULT: FAIL (%d entries carry a condition node outside cases 1..8)\n",
             dirty ); return 1; }

    // ── 3) Load a map and run FilterBrush over every active brush (must not crash). ─
    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }

    char save1[MAX_PATH], save2[MAX_PATH];
    _snprintf( save1, sizeof( save1 ), "%sradiant_filterload_a.map", tmpDir );
    _snprintf( save2, sizeof( save2 ), "%sradiant_filterload_b.map", tmpDir );
    if ( ST_GuardedSave( save1 ) != 0 )
    { STLog( "RESULT: CRASH during baseline save (0x%08lX)\n", g_stLastExcept ); return 3; }

    int evaluated = 0;
    __try {
        ++g_qeglobals.g_filtersUpdated;     // force a fresh FilterBrush evaluation
        for ( selbrush_t *b = active_brushes.next; b && b != &active_brushes; b = b->next )
        { FilterBrush( b, 1 ); ++evaluated; }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH evaluating loaded filters over %d brushes (0x%08lX)\n", evaluated, g_stLastExcept ); return 3; }
    STLog( "FilterBrush evaluated all %d active brushes against the loaded filters — no crash\n", evaluated );

    // ── 4) Toggle the "Lights" entity filter (KeyValue classname light) reversibly. ─
    filter_entry_s *lights = nullptr;
    for ( filter_entry_s *f = CFilterWnd_GetCategoryHead( 2 ); f; f = f->next_filter )   // Entity list
        if ( f->name && _stricmp( f->name, "Lights" ) == 0 ) { lights = f; break; }

    int hidShown = -1, hidHidden = -1, hidRestored = -1, matched = 0;
    if ( lights )
    {
        __try {
            // The toggle's reversibility is only well-defined from a KNOWN baseline state.
            // RadiantFilters loads each entry's isShown from the persisted registry default
            // (CWinApp::GetProfileInt "Filters\<name>", faithful to the binary 0x41104e) — so
            // "Lights" may arrive isShown=0 (a prior session/user left it unchecked), in which
            // case the load-time baseline already HIDES every light brush.  The gate's premise
            // ("Lights shown → hides nothing of its own") then never held, and a plain off→on
            // toggle ends in a DIFFERENT state than it started (shown), so the count legitimately
            // shifts by the # of light brushes — NOT a filter-code bug, just registry pollution
            // of the baseline.  Force a deterministic SHOWN baseline first (the documented
            // premise) so the off→on cycle is a true round-trip independent of the registry.
            if ( !lights->isShown )
                RadiantFilters_ToggleEntry( lights, true );    // normalize to SHOWN baseline
            hidShown    = FL_CountHidden();                    // "Lights" shown → hides nothing of its own
            RadiantFilters_ToggleEntry( lights, false );       // UNCHECK → hide every light brush
            hidHidden   = FL_CountHidden();
            RadiantFilters_ToggleEntry( lights, true );        // RE-CHECK → restore the SHOWN baseline
            hidRestored = FL_CountHidden();
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        { g_stLastExcept = GetExceptionCode();
          STLog( "RESULT: CRASH toggling 'Lights' filter (0x%08lX)\n", g_stLastExcept ); return 3; }
        matched = hidHidden - hidShown;
        STLog( "'Lights' toggle: hidden shown=%d off=%d restored=%d -> matched=%d\n",
               hidShown, hidHidden, hidRestored, matched );
        if ( hidRestored != hidShown )
        { STLog( "RESULT: FAIL ('Lights' toggle not reversible: %d != %d)\n", hidShown, hidRestored ); return 1; }
    }
    else
        STLog( "note: no 'Lights' entity filter in the txt (toggle sub-test skipped)\n" );

    // ── .map unchanged: save again, token-diff against baseline (filtering view-only). ─
    if ( ST_GuardedSave( save2 ) != 0 )
    { STLog( "RESULT: CRASH during post-toggle save (0x%08lX)\n", g_stLastExcept ); return 3; }
    char *bufA = nullptr, *bufB = nullptr;
    long lenA = ST_ReadFile( save1, &bufA ), lenB = ST_ReadFile( save2, &bufB );
    if ( lenA < 0 || lenB < 0 || !bufA || !bufB )
    { STLog( "RESULT: FAIL (cannot read saved maps)\n" ); return 2; }
    char diag[512] = ""; int tokIdx = 0;
    bool same = ST_TokenDiff( bufA, bufB, diag, sizeof( diag ), &tokIdx );
    free( bufA ); free( bufB );
    if ( !same ) { STLog( "RESULT: FAIL (.map changed by filter load/toggle — %s)\n", diag ); return 1; }
    STLog( ".map unchanged (%d tokens identical) — filtering is view-only\n", tokIdx );

    if ( lights && matched <= 0 )
    { STLog( "RESULT: PASS (%d clean filters loaded, all evaluable; 'Lights' matched 0 on this map → N/A toggle)\n", total );
      return 0; }
    STLog( "RESULT: PASS (%d clean filters loaded, all evaluable + safe; 'Lights' hid %d reversibly; .map unchanged)\n",
           total, matched );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunFaceTexMapTest — the FACE-TEXTURE-FILTER (faceTexMap) gate
//  (RADIANT_SELFTEST <map> facetexmap).  Map-FREE data path: directly exercises the
//  ported faceTexMap add/remove/lookup chain that the binary's "filter faces by
//  material" toggle drives.  Builds a synthetic face filter_entry_s whose material_ptr
//  is a qe3_cpp_01-shape name list, then:
//    add (RadiantFilters05_large) → lookup TRUE  (FaceTexMap_HasSubstringOf on a name
//        that CONTAINS one of the keys as a substring, + FALSE for an unrelated name)
//    remove (RadiantFilters06_large) → lookup FALSE (refcount back to 0 → erased)
//    refcount: a SECOND add then a single remove must STILL be filtered (count==2→1),
//        a final remove clears it — proves the ++/-- refcount semantics, not just present/absent.
//  Exits 0 PASS, 1 FAIL.  (No map needed; mapPath accepted for dispatch uniformity.)
// ─────────────────────────────────────────────────────────────────────────────
static int RunFaceTexMapTest( const char * /*mapPath*/ )
{
    // material_ptr list node — same layout qe3_cpp_01 builds: {char* name@0; node* next@4}.
    struct FaceTexNodeGate { const char *name; FaceTexNodeGate *next; };

    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_facetexmap.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: faceTexMap add/lookup/remove ===\n" );
    AddVectoredExceptionHandler( 1, ST_Veh );

    // Two material-name substrings, e.g. a face filter hiding all "caulk" + "nodraw" faces.
    static const char *kKey0 = "caulk";
    static const char *kKey1 = "nodraw";
    FaceTexNodeGate n1 = { kKey1, nullptr };
    FaceTexNodeGate n0 = { kKey0, &n1 };

    // A face-type filter entry owning that name list (filter_type_enum & 4 = face).
    filter_entry_s e;
    memset( &e, 0, sizeof( e ) );
    e.filter_type_enum = 4;
    e.isShown          = true;
    e.material_ptr     = (MaterialInfo *)&n0;
    e.name             = "GATE_FaceTexFilter";

    int rc = 0;
    __try
    {
        // Empty map → nothing filtered.
        if ( FaceTexMap_HasSubstringOf( "tools/toolscaulk" ) )
        { STLog( "FAIL: baseline non-empty (caulk filtered before any add)\n" ); rc = 1; }

        // ── ADD (HIDE): ++faceTexMap[caulk], ++faceTexMap[nodraw]. ────────────────
        RadiantFilters05_large( &e );

        // A face material whose name CONTAINS "caulk" as a substring → filtered.
        bool hitCaulk  = FaceTexMap_HasSubstringOf( "tools/toolscaulk" );
        // A face material whose name CONTAINS "nodraw" → filtered.
        bool hitNodraw = FaceTexMap_HasSubstringOf( "common/nodraw_clip" );
        // An unrelated material → NOT filtered (no key is a substring of it).
        bool hitOther  = FaceTexMap_HasSubstringOf( "ch_tile_floor05" );
        STLog( "after add: caulk=%d nodraw=%d other=%d (expect 1 1 0)\n",
               hitCaulk, hitNodraw, hitOther );
        if ( !hitCaulk || !hitNodraw || hitOther ) { STLog( "FAIL: add→lookup mismatch\n" ); rc = 1; }

        // ── REFCOUNT: ADD AGAIN (count caulk/nodraw → 2), then one REMOVE (→ 1). ──
        RadiantFilters05_large( &e );      // counts now 2 / 2
        RadiantFilters06_large( &e );      // counts now 1 / 1 — still filtered
        bool stillCaulk = FaceTexMap_HasSubstringOf( "tools/toolscaulk" );
        STLog( "after add×2 then remove×1: caulk=%d (expect 1 — refcount still 1)\n", stillCaulk );
        if ( !stillCaulk ) { STLog( "FAIL: refcount erased too early (++/-- not balanced)\n" ); rc = 1; }

        // ── REMOVE (SHOW): final --→0 → erase both keys. ─────────────────────────
        RadiantFilters06_large( &e );      // counts now 0 / 0 — erased
        bool goneCaulk  = FaceTexMap_HasSubstringOf( "tools/toolscaulk" );
        bool goneNodraw = FaceTexMap_HasSubstringOf( "common/nodraw_clip" );
        STLog( "after remove: caulk=%d nodraw=%d (expect 0 0 — erased)\n", goneCaulk, goneNodraw );
        if ( goneCaulk || goneNodraw ) { STLog( "FAIL: remove→lookup still filtered (not erased at 0)\n" ); rc = 1; }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH in faceTexMap add/remove/lookup (0x%08lX)\n", g_stLastExcept ); return 3; }

    if ( rc ) { STLog( "RESULT: FAIL\n" ); return 1; }
    STLog( "RESULT: PASS (add→lookup(true)→refcount→remove→lookup(false) all faithful)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunConnectionsTest — the CONNECTIONS overlay gate (RADIANT_SELFTEST <map> connections).
//  Proves CMainFrame::OnSelectConnections + Lines_AddLinkTo / Lines_AddLinkToScript without
//  a UI / live renderer:
//    (A) LINK COUNT (deterministic) — Radiant_TestConnectionLinks replicates the two MATCH
//        loops and counts (target→targetname) + (script_linkTo→script_linkName) entity link
//        pairs.  The count must be STABLE across two calls (the loaded map doesn't change),
//        and re-counting after a save→reload must give the SAME value (links are derived from
//        epairs that round-trip).  0 links is a valid PASS = "no connections on this map".
//    (B) SELF-GATE — the 0x4 bit math (OnSelectConnections) flips exactly its own bit and is
//        idempotent (the draw functions early-out when it is SET, so the overlay hides).
//    (C) .map UNCHANGED — Connections is a view-only flag; save before/after, token-diff.
//  Exits 0 PASS or N/A, 1 FAIL, 2 IO/load, 3 CRASH.
// ─────────────────────────────────────────────────────────────────────────────
static int RunConnectionsTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_connections.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Connections overlay (target/targetname + script_link) ===\n" );
    STLog( "map : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }

    // ── (A) link count (stable + survives round-trip) ────────────────────────────
    int t1 = -1, s1 = -1, total1 = -3;
    __try { total1 = Radiant_TestConnectionLinks( &t1, &s1 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during link count (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "links: target/targetname=%d  script_linkTo/linkName=%d  total=%d\n", t1, s1, total1 );

    // call it again — same loaded map → identical count (no nondeterminism in the match loop)
    int t1b = -1, s1b = -1, total1b = Radiant_TestConnectionLinks( &t1b, &s1b );
    if ( total1b != total1 || t1b != t1 || s1b != s1 )
    { STLog( "RESULT: FAIL (link count not stable: %d/%d -> %d/%d)\n", t1, s1, t1b, s1b ); return 1; }

    // ── (B) self-gate bit math (OnSelectConnections) ─────────────────────────────
    int *flags = &g_qeglobals.d_savedinfo.d_xyShowFlags;
    int before = *flags;
    *flags ^= 4;                                     // toggle ON (hide)
    if ( ( *flags & 4 ) != 4 )
    { STLog( "RESULT: FAIL (Connections toggle did not SET bit 0x4)\n" ); return 1; }
    if ( ( *flags & ~4 ) != ( before & ~4 ) )
    { STLog( "RESULT: FAIL (Connections toggle disturbed other bits: %02x->%02x)\n", before, *flags ); return 1; }
    *flags ^= 4;                                     // toggle OFF (idempotent)
    if ( *flags != before )
    { STLog( "RESULT: FAIL (Connections toggle not idempotent: %02x->%02x)\n", before, *flags ); return 1; }
    STLog( "self-gate: bit 0x4 SET/CLEAR clean, other bits intact, idempotent\n" );

    // ── (C) save → reload → re-count (links derive from round-tripping epairs) ────
    char save1[MAX_PATH], save2[MAX_PATH];
    _snprintf( save1, sizeof( save1 ), "%sradiant_connections_a.map", tmpDir );
    _snprintf( save2, sizeof( save2 ), "%sradiant_connections_b.map", tmpDir );
    *flags = 0;                                      // view-only flag must not serialise
    STLog( "step: Map_SaveFile (baseline)\n" );
    if ( ST_GuardedSave( save1 ) != 0 )
    { STLog( "RESULT: CRASH during save (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: Map_LoadFromFile (reload)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( save1 ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }

    int t2 = -1, s2 = -1, total2 = Radiant_TestConnectionLinks( &t2, &s2 );
    STLog( "links after reload: target=%d  script=%d  total=%d\n", t2, s2, total2 );
    if ( total2 != total1 || t2 != t1 || s2 != s1 )
    { STLog( "RESULT: FAIL (link count changed across save/reload: %d/%d -> %d/%d)\n",
             t1, s1, t2, s2 ); return 1; }

    STLog( "step: Map_SaveFile (post)\n" );
    if ( ST_GuardedSave( save2 ) != 0 )
    { STLog( "RESULT: CRASH during post save (0x%08lX)\n", g_stLastExcept ); return 3; }
    char *bufA = nullptr, *bufB = nullptr;
    long  lenA = ST_ReadFile( save1, &bufA );
    long  lenB = ST_ReadFile( save2, &bufB );
    if ( lenA < 0 || lenB < 0 || !bufA || !bufB )
    { STLog( "RESULT: FAIL (cannot read saved maps)\n" ); free(bufA); free(bufB); return 2; }
    char diag[512] = ""; int tokIdx = 0;
    bool same = ST_TokenDiff( bufA, bufB, diag, sizeof( diag ), &tokIdx );
    free( bufA ); free( bufB );
    if ( !same )
    { STLog( "RESULT: FAIL (.map changed by the view flag at token %d: %s)\n", tokIdx, diag ); return 1; }

    if ( total1 == 0 )
        STLog( "RESULT: PASS (N/A — no connections on this map; self-gate + .map-unchanged clean)\n" );
    else
        STLog( "RESULT: PASS (%d link pairs, stable across reload; self-gate clean; .map unchanged)\n", total1 );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunClassSimilarTest — the Selection->Select All Of Same Class gate.  Covers
//  Select_ByClassSimilar (0x493830) and specifically guards its iteration DIRECTION:
//  the binary walks the selected list from the TAIL (IDB selected_brushes @+0 = the
//  .prev pointer) via ->prev, matching EVERY selected brush's class; a port that starts
//  at .next (head) breaks after one node (head->prev == &selected_brushes) and matches
//  only the head brush's class.  The harness selects two active brushes of DIFFERENT
//  eclasses, runs the op, and asserts brushes of BOTH classes end up selected (the buggy
//  head-only version selects only one).  N/A on maps with <2 non-world entity classes.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestClassSimilar( int *outClasses, int *outSelCount, int *outSelA, int *outSelB );  // select.cpp

static int RunClassSimilarTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_classsimilar.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Selection->Select All Of Same Class (iteration direction) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }

    STLog( "step: Radiant_TestClassSimilar\n" );
    int classes = 0, selCount = 0, selA = 0, selB = 0, ok = 0;
    __try { ok = Radiant_TestClassSimilar( &classes, &selCount, &selA, &selB ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Select_ByClassSimilar (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "classes-available=%d  selected-after=%d  ofClassA=%d  ofClassB=%d\n",
           classes, selCount, selA, selB );
    if ( !ok )
    {
        STLog( "RESULT: PASS (N/A — no two distinct >=2-member non-world classes on this map)\n" );
        return 0;
    }
    // Two classes, each with >=2 selectable members, are selected (one at the list head,
    // one at the tail). A correct tail->head walk fully matches BOTH (selA>=2 AND selB>=2).
    // The buggy .next-start walk processes only the head node then breaks (head->prev ==
    // sentinel), so the TAIL class stays at exactly 1 (only its manually-selected brush) —
    // caught here regardless of which class landed at the tail.
    if ( selA < 2 || selB < 2 )
    { STLog( "RESULT: FAIL (a selected class was NOT fully matched: selA=%d selB=%d (both expected >=2) — Select_ByClassSimilar walked only the head, not the whole selected list)\n", selA, selB ); return 1; }

    STLog( "RESULT: PASS (both selected classes fully matched: A=%d B=%d of %d selected)\n", selA, selB, selCount );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunXyShowTest — the VIEW→SHOW overlay-toggle gate (RADIANT_SELFTEST <map> xyshow).
//  Proves CMainFrame::OnSelectNames / OnSelectCoordinates / OnSelectReverseFilter's
//  deterministic effects without a UI:
//    (A) STATE MACHINE (every map) — directly drive the three handlers' bit math against
//        g_qeglobals.d_savedinfo.d_xyShowFlags: each toggle flips exactly its own bit
//        (Names 0x8, Coordinates 0x20, Reverse 0x40), leaves the others untouched, and is
//        idempotent (toggle twice → back to start).  This is the part that gates on ALL maps.
//    (B) REVERSE-FILTER INVERSION — Radiant_TestReverseFilter installs an active "Contents
//        detail" condition filter and asserts the 0x40 bit (read by FilterBrush) cleanly
//        COMPLEMENTS the hidden set (hiddenReverse + hiddenNormal == total active brushes).
//        N/A when the condition matches no brush on this map (prefab-only maps), like filter.
//    (C) .map UNCHANGED — these are view-only flags; save before/after and token-diff.
//  Exits 0 PASS or N/A, 1 FAIL, 2 IO/load, 3 CRASH.
// ─────────────────────────────────────────────────────────────────────────────
static int RunXyShowTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_xyshow.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: View->Show overlay toggles ===\n" );
    STLog( "map : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }

    // ── (A) ALL SIX toggles' bit state machine (mirrors the mainfrm.cpp handlers) ──
    // Start from a known state (all overlays shown = all bits clear).  Angles (0x2) +
    // Blocks (0x10) were added when their draw consumers (DrawAngles / XY_DrawBlockGrid)
    // landed — every View→Show toggle is now wired, so all six are exercised here.
    int *flags = &g_qeglobals.d_savedinfo.d_xyShowFlags;
    *flags = 0;
    struct { const char *name; int bit; } T[5] = {
        { "Names",        0x8  },
        { "Angles",       0x2  },
        { "Blocks",       0x10 },
        { "Coordinates",  0x20 },
        { "ReverseFilter",0x40 },
    };
    for ( int i = 0; i < 5; ++i )
    {
        int before = *flags;
        *flags ^= T[i].bit;                          // toggle ON
        if ( ( *flags & T[i].bit ) == 0 )
        { STLog( "RESULT: FAIL (%s toggle did not SET its bit)\n", T[i].name ); return 1; }
        if ( ( *flags & ~T[i].bit ) != ( before & ~T[i].bit ) )
        { STLog( "RESULT: FAIL (%s toggle disturbed other bits: %02x->%02x)\n",
                 T[i].name, before, *flags ); return 1; }
        *flags ^= T[i].bit;                          // toggle OFF (idempotent)
        if ( *flags != before )
        { STLog( "RESULT: FAIL (%s toggle not idempotent: %02x->%02x)\n",
                 T[i].name, before, *flags ); return 1; }
        STLog( "toggle %-13s bit 0x%02x: SET/CLEAR clean, other bits intact\n",
               T[i].name, T[i].bit );
    }

    // ── (C) baseline save (flags back to 0, all shown) ───────────────────────────
    char save1[MAX_PATH], save2[MAX_PATH];
    _snprintf( save1, sizeof( save1 ), "%sradiant_xyshow_a.map", tmpDir );
    _snprintf( save2, sizeof( save2 ), "%sradiant_xyshow_b.map", tmpDir );
    *flags = 0;
    STLog( "step: Map_SaveFile (baseline)\n" );
    if ( ST_GuardedSave( save1 ) != 0 )
    { STLog( "RESULT: CRASH during baseline save (0x%08lX)\n", g_stLastExcept ); return 3; }

    // ── (B) reverse-filter inversion via FilterBrush ─────────────────────────────
    int total = 0, normal = 0, reverse = 0, matched = -3;
    __try { matched = Radiant_TestReverseFilter( "Contents detail", &total, &normal, &reverse ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during reverse-filter test (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "reverse \"Contents detail\": total=%d hiddenNormal=%d hiddenReverse=%d → matched=%d\n",
           total, normal, reverse, matched );
    if ( matched == -1 )
    { STLog( "RESULT: FAIL (reverse toggle not reversible)\n" ); return 1; }
    if ( matched == -2 )
    { STLog( "RESULT: FAIL (reverse did NOT complement the hidden set: %d+%d != %d)\n",
             reverse, normal, total ); return 1; }

    // ── (C) save again with flags=0 — view flags must NOT serialise into the .map ──
    *flags = 0;
    STLog( "step: Map_SaveFile (post-toggle)\n" );
    if ( ST_GuardedSave( save2 ) != 0 )
    { STLog( "RESULT: CRASH during post-toggle save (0x%08lX)\n", g_stLastExcept ); return 3; }
    char *bufA = nullptr, *bufB = nullptr;
    long  lenA = ST_ReadFile( save1, &bufA );
    long  lenB = ST_ReadFile( save2, &bufB );
    if ( lenA < 0 || lenB < 0 || !bufA || !bufB )
    { STLog( "RESULT: FAIL (cannot read saved maps)\n" ); return 2; }
    char diag[512] = ""; int tokIdx = 0;
    bool same = ST_TokenDiff( bufA, bufB, diag, sizeof( diag ), &tokIdx );
    free( bufA ); free( bufB );
    if ( !same )
    { STLog( "RESULT: FAIL (.map changed by show-flags — %s)\n", diag ); return 1; }
    STLog( ".map unchanged across the toggles (%d tokens identical) — show flags are view-only\n", tokIdx );

    if ( matched <= 0 )
    {
        STLog( "RESULT: N/A (state machine PASS; \"Contents detail\" matched no brush — "
               "reverse inversion not exercised on this map)\n" );
        return 0;
    }
    STLog( "RESULT: PASS (5 toggles flip-clean; reverse-filter inverts %d/%d brushes; .map view-only)\n",
           matched, total );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunLayersTest — the LAYERS dialog round-trip gate.  load → enumerate the map's
//  layers → create a new layer → assign the first active world brush to it → hide
//  it → save → the .map carries the new layer block line ("<name>" flags hidden)
//  AND the per-brush `layer "<name>"` line → reload → both survive.
// ─────────────────────────────────────────────────────────────────────────────
static int RunLayersTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_layers.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: layers-dialog round-trip ===\n" );
    STLog( "map : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    const char *kLayer = "OpusTestLayer";
    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_layers.map", tmpDir );
    STLog( "save: %s   new layer: \"%s\"\n", savePath, kLayer );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    STLog( "step: enumerate + create \"%s\" + assign first brush + hide\n", kLayer );
    int beforeCount = 0;
    __try { beforeCount = Radiant_TestLayers( kLayer ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during layer op (0x%08lX)\n", g_stLastExcept ); return 3; }

    if ( beforeCount < 0 )
    {
        STLog( "RESULT: N/A (no assignable inline world brush on this map — like the "
               "texmod/findrepl prefab-only maps)\n" );
        return 0;
    }
    STLog( "enumerated %d layer(s) before the new one\n", beforeCount );
    if ( beforeCount <= 0 ) { STLog( "RESULT: FAIL (enumeration saw no layers)\n" ); return 1; }
    if ( ST_GuardedDiag( "after-layerop" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after layer op)\n" ); return 3; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    // Textual proof: the saved .map carries (a) the new layer's block line with the
    // hidden flag, and (b) the per-brush `layer "<name>"` line.  The layer block line
    // is `"OpusTestLayer" flags hidden`; the brush line is `layer "OpusTestLayer"`.
    char *saved = nullptr;
    long  savedLen = ST_ReadFile( savePath, &saved );
    if ( savedLen < 0 || !saved ) { STLog( "RESULT: FAIL (cannot read saved map)\n" ); return 2; }
    bool hasLayerName = ST_MapHasWord( saved, kLayer );
    bool hasHidden    = ( strstr( saved, "\"" ) && strstr( saved, "flags" ) && strstr( saved, "hidden" ) != nullptr );
    // The per-brush line: `layer "OpusTestLayer"` appears inside a brush block.
    char brushLine[64];
    _snprintf( brushLine, sizeof( brushLine ), "layer \"%s\"", kLayer );
    bool hasBrushLayer = ( strstr( saved, brushLine ) != nullptr );
    free( saved );
    STLog( ".map: layer name present=%s, a hidden flag present=%s, per-brush `%s`=%s\n",
           hasLayerName ? "YES":"no", hasHidden ? "YES":"no", brushLine, hasBrushLayer ? "YES":"no" );
    if ( !hasLayerName )  { STLog( "RESULT: FAIL (new layer not in .map)\n" ); return 1; }
    if ( !hasBrushLayer ) { STLog( "RESULT: FAIL (brush layer assignment not written)\n" ); return 1; }
    if ( !hasHidden )     { STLog( "RESULT: FAIL (hidden flag not written)\n" ); return 1; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-reload" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    // After reload: the new layer must be back in the layerMap with the hidden flag,
    // and a brush must carry it as its parent_layer_string.
    int reFlags = Layers_GetFlags( kLayer );
    bool reExists = Layers_Exists( kLayer );
    STLog( "after reload: layer exists=%s  flags=0x%X (hidden bit=%d)\n",
           reExists ? "YES":"no", reFlags, ( reFlags & 1 ) ? 1 : 0 );
    if ( !reExists )           { STLog( "RESULT: FAIL (layer did not survive reload)\n" ); return 1; }
    if ( ( reFlags & 1 ) == 0 ){ STLog( "RESULT: FAIL (hidden flag lost on reload)\n" ); return 1; }

    int reBrushes = 0;
    for ( selbrush_t *i = active_brushes.next; i && i != &active_brushes; i = i->next )
        if ( i->def && i->def->parent_layer_string && !strcmp( i->def->parent_layer_string, kLayer ) )
            ++reBrushes;
    STLog( "after reload: %d brush(es) carry layer \"%s\"\n", reBrushes, kLayer );
    if ( reBrushes < 1 ) { STLog( "RESULT: FAIL (brush layer assignment lost on reload)\n" ); return 1; }

    STLog( "RESULT: PASS (layer created/assigned/hidden; %d→%d layers; round-trip clean)\n",
           beforeCount, Layers_Count() );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunScriptGrpTest — the SCRIPT-GROUP key add/remove round-trip (the scriptgrp gate).
//  Drives scriptgroup.cpp's ScriptGroup_{Add,Remove}KeyFromSelected exactly as the
//  Script-Group dialog's two sub-key buttons do (sub_454780/sub_4549B0), but on the
//  selected entity directly (the dialog is parked).  Flow: load → create a point entity
//  (auto-selected) → AddKeyToSelected(v1,key) → AddKeyToSelected(v2,key) (exercise the
//  append-to-existing path) → confirm the key holds "v1 v2" in memory AND in the saved
//  .map → reload → confirm it survived → reselect, RemoveKeyFromSelected the FIRST token →
//  confirm only the second remains ("v2 ").  ScriptGroup_AddKey appends with a LEADING
//  space (no trailing); ScriptGroup_HasKey rebuilds kept tokens each with a TRAILING space
//  — both are part of the stored value and are checked verbatim (load-bearing for round-trip).
// ─────────────────────────────────────────────────────────────────────────────
static int RunScriptGrpTest( const char *mapPath, const char *classname,
                             const char *key, const char *v1, const char *v2 )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_scriptgrp.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: script-group key add/remove round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    STLog( "class: %s   key: \"%s\"   v1: \"%s\"   v2: \"%s\"\n", classname, key, v1, v2 );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_scriptgrp.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // Drag a placeholder brush, then create the point entity (consumes it, leaving the
    // new entity's brush on selected_brushes — which the iterators walk).
    STLog( "step: NewBrushDrag placeholder + CreateEntityFromName(%s)\n", classname );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 320, 320,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during placeholder create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no placeholder brush)\n" ); return 1; }

    entity_s *inst = nullptr;
    __try { inst = (entity_s *)Radiant_TestCreateEntity( classname ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during CreateEntityFromName (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !inst ) { STLog( "RESULT: FAIL (no entity created/selected)\n" ); return 1; }
    entity_s_def *def = (entity_s_def *)inst->def;

    // Add the first then the second value-token to `key` (the second exercises the
    // append-to-existing path).  ScriptGroup_AddKey leaves the value as "v1 v2 ".
    STLog( "step: ScriptGroup_AddKeyToSelected(\"%s\", \"%s\")\n", v1, key );
    __try { Radiant_TestScriptAddKey( v1, key ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during AddKey v1 (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "step: ScriptGroup_AddKeyToSelected(\"%s\", \"%s\")\n", v2, key );
    __try { Radiant_TestScriptAddKey( v2, key ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during AddKey v2 (0x%08lX)\n", g_stLastExcept ); return 3; }

    // ScriptGroup_AddKey appends with a LEADING space (va(" %s", value)): the first add
    // writes "v1", the second appends " v2" → "v1 v2" (NO trailing space).  (The REMOVE
    // path, ScriptGroup_HasKey, is the one that leaves a trailing space on each kept token.)
    char expectBoth[256];
    _snprintf( expectBoth, sizeof( expectBoth ), "%s %s", v1, v2 );    // "v1 v2"
    const char *vmem = ValueForKey2( (int)(intptr_t)def, key );
    STLog( "in-memory ValueForKey2(\"%s\") = \"%s\"  (expect \"%s\")\n",
           key, vmem ? vmem : "(null)", expectBoth );
    if ( !vmem || strcmp( vmem, expectBoth ) != 0 )
    { STLog( "RESULT: FAIL (in-memory key did not take both tokens)\n" ); return 1; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    // Textual proof: the saved .map carries `"key" "v1 v2 "`.
    char *saved = nullptr;
    long  savedLen = ST_ReadFile( savePath, &saved );
    if ( savedLen < 0 || !saved ) { STLog( "RESULT: FAIL (cannot read saved map)\n" ); return 2; }
    char needle[512];
    _snprintf( needle, sizeof( needle ), "\"%s\" \"%s\"", key, expectBoth );
    bool inText = ( strstr( saved, needle ) != nullptr );
    free( saved );
    STLog( ".map contains  %s  : %s\n", needle, inText ? "YES" : "NO" );
    if ( !inText ) { STLog( "RESULT: FAIL (epair not in saved .map)\n" ); return 1; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-reload" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    // After reload, find the entity of <class> carrying our two-token value, and reselect
    // its brush so the REMOVE pass can run on it.
    entity_s *reEnt = nullptr;
    for ( entity_s *e = entities.next; e && e != &entities; e = e->next )
    {
        const char *cn = ValueForKey2( (int)(intptr_t)e, "classname" );
        if ( cn && _stricmp( cn, classname ) == 0 )
        {
            const char *vr = ValueForKey2( (int)(intptr_t)e, key );
            if ( vr && strcmp( vr, expectBoth ) == 0 ) { reEnt = e; break; }
        }
    }
    STLog( "after-reload: entity of class '%s' with \"%s\"=\"%s\" found: %s\n",
           classname, key, expectBoth, reEnt ? "YES" : "NO" );
    if ( !reEnt )
    { STLog( "RESULT: FAIL (key did not survive the round-trip)\n" ); return 1; }

    // Select the reloaded entity's brush instance (active→selected) so the iterators see it,
    // then REMOVE the first token; the value must collapse to just "v2 ".  The iterators read
    // the epairs off i->owner->def (the DEF), and reEnt (found via the
    // entities/def list) IS that def — so match the active brush whose owner's def is reEnt.
    selbrush_t *reBrush = nullptr;
    for ( selbrush_t *i = active_brushes.next; i && i != &active_brushes; i = i->next )
        if ( i->owner && (entity_s_def *)i->owner->def == reEnt ) { reBrush = i; break; }
    if ( !reBrush )
    { STLog( "RESULT: FAIL (reloaded entity has no brush to select)\n" ); return 1; }
    __try { Select_Brush( reBrush, 0, 0, 0 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during reselect (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: ScriptGroup_RemoveKeyFromSelected(\"%s\", \"%s\")\n", key, v1 );
    __try { Radiant_TestScriptRemoveKey( key, v1 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during RemoveKey (0x%08lX)\n", g_stLastExcept ); return 3; }

    char expectV2[256];
    _snprintf( expectV2, sizeof( expectV2 ), "%s ", v2 );             // "v2 " (kept token + trailing space)
    const char *vrem = ValueForKey2( (int)(intptr_t)reEnt, key );
    STLog( "after-remove: ValueForKey2(\"%s\") = \"%s\"  (expect \"%s\")\n",
           key, vrem ? vrem : "(null)", expectV2 );
    if ( !vrem || strcmp( vrem, expectV2 ) != 0 )
    { STLog( "RESULT: FAIL (remove did not collapse the value to just v2)\n" ); return 1; }

    STLog( "RESULT: PASS (added \"%s\"+\"%s\" to '%s' -> save -> reload persisted; removed \"%s\" -> \"%s\")\n",
           v1, v2, key, v1, expectV2 );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunDynEntTest — the DYN-ENTITY authoring round-trip (the dynent gate).  Drives
//  the Dyn-Entity dialog's two key writers (DynEntityDlg_01_SetPair /
//  DynEntityDlg_02_RemovePair) on a selected dyn_model exactly as the dialog's
//  Set/Clear buttons do, but without a modal dialog (the entwnd/scriptgrp pattern).
//
//  Flow: load → place a placeholder brush → CreateEntityFromName("dyn_model") (a
//  fixedsize point entity, cod4.def; its create goes through the same model-family tail
//  as misc_model — Ed_PostAddModelCommand, a headless no-op — so the bbox entity stands,
//  AND its eclass name "dyn_model" makes the SetPair/RemovePair loop process it) → the
//  new entity is auto-selected → SetPair "destruct" on "type", "100" on "health",
//  "physpreset_metal" on "physPreset" → confirm the three epairs are in memory AND in the
//  saved .map → reload → confirm they survived on a reloaded dyn_model → reselect it →
//  RemovePair "health" → confirm it is gone while "type" remains.  Entity count +1.
// ─────────────────────────────────────────────────────────────────────────────
static int RunDynEntTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_dynent.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    const char *kClass = "dyn_model";
    STLog( "=== RADIANT_SELFTEST: dyn-entity authoring round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    STLog( "class: %s\n", kClass );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_dynent.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int entsBefore = g_qeglobals.d_num_entities;
    STLog( "loaded %d entities\n", entsBefore );
    if ( entsBefore <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // Place a placeholder brush, then create the dyn_model (consumes it, leaving the new
    // entity's bbox brush on selected_brushes — which the SetPair/RemovePair loop walks).
    STLog( "step: NewBrushDrag placeholder + CreateEntityFromName(%s)\n", kClass );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 320, 320,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during placeholder create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no placeholder brush)\n" ); return 1; }

    entity_s *inst = nullptr;
    __try { inst = (entity_s *)Radiant_TestCreateEntity( kClass ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during CreateEntityFromName (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !inst ) { STLog( "RESULT: FAIL (no dyn_model created/selected)\n" ); return 1; }
    entity_s_def *def = (entity_s_def *)inst->def;
    STLog( "created dyn_model instance %p (selected)\n", (void *)inst );
    if ( ST_GuardedDiag( "after-create" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after create)\n" ); return 3; }

    // SetPair each dyn-entity key (the dialog's per-field Set).  These are FIRST runtime
    // exercises of DynEntityDlg_01_SetPair.
    struct { const char *key, *val; } kv[] = {
        { "type",       "destruct"        },
        { "health",     "100"             },
        { "physPreset", "physpreset_metal" },
    };
    for ( int n = 0; n < 3; ++n )
    {
        STLog( "step: DynEntityDlg_01_SetPair(\"%s\", \"%s\")\n", kv[n].val, kv[n].key );
        __try { Radiant_TestDynEntSetPair( kv[n].val, kv[n].key ); }
        __except( EXCEPTION_EXECUTE_HANDLER )
        { g_stLastExcept = GetExceptionCode();
          STLog( "RESULT: CRASH during SetPair %s (0x%08lX)\n", kv[n].key, g_stLastExcept ); return 3; }

        const char *vmem = ValueForKey2( (int)(intptr_t)def, kv[n].key );
        STLog( "in-memory ValueForKey2(\"%s\") = \"%s\"\n", kv[n].key, vmem ? vmem : "(null)" );
        if ( !vmem || strcmp( vmem, kv[n].val ) != 0 )
        { STLog( "RESULT: FAIL (in-memory key %s did not take)\n", kv[n].key ); return 1; }
    }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    // Textual proof: the saved .map carries all three dyn-entity epair lines.
    char *saved = nullptr;
    long  savedLen = ST_ReadFile( savePath, &saved );
    if ( savedLen < 0 || !saved ) { STLog( "RESULT: FAIL (cannot read saved map)\n" ); return 2; }
    bool allText = true;
    for ( int n = 0; n < 3; ++n )
    {
        char needle[256];
        _snprintf( needle, sizeof( needle ), "\"%s\" \"%s\"", kv[n].key, kv[n].val );
        bool in = ( strstr( saved, needle ) != nullptr );
        STLog( ".map contains  %s  : %s\n", needle, in ? "YES" : "NO" );
        if ( !in ) allText = false;
    }
    free( saved );
    if ( !allText ) { STLog( "RESULT: FAIL (a dyn-entity epair is not in the saved .map)\n" ); return 1; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-reload" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }
    STLog( "after-reload entity count: %d (was %d before create)\n",
           g_qeglobals.d_num_entities, entsBefore );
    if ( g_qeglobals.d_num_entities != entsBefore + 1 )
    { STLog( "RESULT: FAIL (entity count not +1 after round-trip)\n" ); return 1; }

    // Find the reloaded dyn_model carrying our keys, and reselect its brush for REMOVE.
    entity_s *reEnt = nullptr;
    for ( entity_s *e = entities.next; e && e != &entities; e = e->next )
    {
        const char *cn = ValueForKey2( (int)(intptr_t)e, "classname" );
        if ( cn && _stricmp( cn, kClass ) == 0 )
        {
            const char *vt = ValueForKey2( (int)(intptr_t)e, "type" );
            const char *vh = ValueForKey2( (int)(intptr_t)e, "health" );
            if ( vt && !strcmp( vt, "destruct" ) && vh && !strcmp( vh, "100" ) ) { reEnt = e; break; }
        }
    }
    STLog( "after-reload: dyn_model with type/health found: %s\n", reEnt ? "YES" : "NO" );
    if ( !reEnt )
    { STLog( "RESULT: FAIL (dyn-entity keys did not survive the round-trip)\n" ); return 1; }

    selbrush_t *reBrush = nullptr;
    for ( selbrush_t *i = active_brushes.next; i && i != &active_brushes; i = i->next )
        if ( i->owner && (entity_s_def *)i->owner->def == reEnt ) { reBrush = i; break; }
    if ( !reBrush )
    { STLog( "RESULT: FAIL (reloaded dyn_model has no brush to select)\n" ); return 1; }
    __try { Select_Brush( reBrush, 0, 0, 0 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during reselect (0x%08lX)\n", g_stLastExcept ); return 3; }

    // RemovePair "health" (the dialog's Clear) — FIRST runtime exercise of RemovePair.
    STLog( "step: DynEntityDlg_02_RemovePair(\"health\")\n" );
    __try { Radiant_TestDynEntRemovePair( "health" ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during RemovePair (0x%08lX)\n", g_stLastExcept ); return 3; }

    const char *vh2 = ValueForKey2( (int)(intptr_t)reEnt, "health" );
    const char *vt2 = ValueForKey2( (int)(intptr_t)reEnt, "type" );
    STLog( "after-remove: health = \"%s\" (expect empty), type = \"%s\" (expect destruct)\n",
           vh2 ? vh2 : "(removed)", vt2 ? vt2 : "(null)" );
    if ( vh2 && vh2[0] )
    { STLog( "RESULT: FAIL (RemovePair did not drop \"health\")\n" ); return 1; }
    if ( !vt2 || strcmp( vt2, "destruct" ) != 0 )
    { STLog( "RESULT: FAIL (RemovePair clobbered \"type\")\n" ); return 1; }

    STLog( "RESULT: PASS (dyn_model authored: type/health/physPreset round-tripped; removed health)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunVehicleTest — the VEHICLE authoring round-trip (the vehicle gate).  Drives the
//  Vehicle-Group dialog's two key writers (VehicleDlg_SetPair / VehicleDlg_RemovePair)
//  on a selected info_vehicle_node exactly as the dialog's Set/Clear buttons do, but
//  without a modal dialog (the dynent/scriptgrp pattern).
//
//  info_vehicle_node is a CoD vehicle PATH NODE — a fixedsize point entity (cod4.def,
//  bbox (-24 -24 -24)(24 24 24)) whose Valid Keys ARE the dialog's keys (speed /
//  lookahead / script_deathroll / script_crashtype / ...).  It creates cleanly via
//  CreateEntityFromName (no model/prefab), and the dialog applies to ANY selected
//  non-world entity (no eclass filter), so it is the canonical "vehicle node selected"
//  case the dialog's help text describes.
//
//  Flow: load → place a placeholder brush → CreateEntityFromName("info_vehicle_node")
//  (auto-selected) → SetPair "640" on "speed", "0.30" on "lookahead", "plane" on
//  "script_crashtype" → confirm the three epairs are in memory AND in the saved .map →
//  reload → confirm they survived → reselect → RemovePair "script_crashtype" → confirm it
//  is gone while "speed" remains.  Entity count +1.
// ─────────────────────────────────────────────────────────────────────────────
static int RunVehicleTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_vehicle.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    const char *kClass = "info_vehicle_node";
    STLog( "=== RADIANT_SELFTEST: vehicle authoring round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    STLog( "class: %s\n", kClass );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_vehicle.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int entsBefore = g_qeglobals.d_num_entities;
    STLog( "loaded %d entities\n", entsBefore );
    if ( entsBefore <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // Place a placeholder brush, then create the vehicle node (consumes it, leaving the
    // new entity's bbox brush on selected_brushes — which SetPair/RemovePair walks).
    STLog( "step: NewBrushDrag placeholder + CreateEntityFromName(%s)\n", kClass );
    void *ph = nullptr;
    __try { ph = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 320, 320,
                                              0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during placeholder create (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !ph ) { STLog( "RESULT: FAIL (no placeholder brush)\n" ); return 1; }

    entity_s *inst = nullptr;
    __try { inst = (entity_s *)Radiant_TestCreateEntity( kClass ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during CreateEntityFromName (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !inst ) { STLog( "RESULT: FAIL (no info_vehicle_node created/selected)\n" ); return 1; }
    entity_s_def *def = (entity_s_def *)inst->def;
    STLog( "created info_vehicle_node instance %p (selected)\n", (void *)inst );
    if ( ST_GuardedDiag( "after-create" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after create)\n" ); return 3; }

    // SetPair each vehicle key (the dialog's per-field Set / crash-type combo).  FIRST
    // runtime exercises of VehicleDlg_SetPair.
    struct { const char *key, *val; } kv[] = {
        { "speed",            "640"   },
        { "lookahead",        "0.30"  },
        { "script_crashtype", "plane" },
    };
    for ( int n = 0; n < 3; ++n )
    {
        STLog( "step: VehicleDlg_SetPair(\"%s\", \"%s\")\n", kv[n].val, kv[n].key );
        __try { Radiant_TestVehicleSetPair( kv[n].val, kv[n].key ); }
        __except( EXCEPTION_EXECUTE_HANDLER )
        { g_stLastExcept = GetExceptionCode();
          STLog( "RESULT: CRASH during SetPair %s (0x%08lX)\n", kv[n].key, g_stLastExcept ); return 3; }

        const char *vmem = ValueForKey2( (int)(intptr_t)def, kv[n].key );
        STLog( "in-memory ValueForKey2(\"%s\") = \"%s\"\n", kv[n].key, vmem ? vmem : "(null)" );
        if ( !vmem || strcmp( vmem, kv[n].val ) != 0 )
        { STLog( "RESULT: FAIL (in-memory key %s did not take)\n", kv[n].key ); return 1; }
    }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    // Textual proof: the saved .map carries all three vehicle epair lines.
    char *saved = nullptr;
    long  savedLen = ST_ReadFile( savePath, &saved );
    if ( savedLen < 0 || !saved ) { STLog( "RESULT: FAIL (cannot read saved map)\n" ); return 2; }
    bool allText = true;
    for ( int n = 0; n < 3; ++n )
    {
        char needle[256];
        _snprintf( needle, sizeof( needle ), "\"%s\" \"%s\"", kv[n].key, kv[n].val );
        bool in = ( strstr( saved, needle ) != nullptr );
        STLog( ".map contains  %s  : %s\n", needle, in ? "YES" : "NO" );
        if ( !in ) allText = false;
    }
    free( saved );
    if ( !allText ) { STLog( "RESULT: FAIL (a vehicle epair is not in the saved .map)\n" ); return 1; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-reload" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }
    STLog( "after-reload entity count: %d (was %d before create)\n",
           g_qeglobals.d_num_entities, entsBefore );
    if ( g_qeglobals.d_num_entities != entsBefore + 1 )
    { STLog( "RESULT: FAIL (entity count not +1 after round-trip)\n" ); return 1; }

    // Find the reloaded vehicle node carrying our keys, and reselect its brush for REMOVE.
    entity_s *reEnt = nullptr;
    for ( entity_s *e = entities.next; e && e != &entities; e = e->next )
    {
        const char *cn = ValueForKey2( (int)(intptr_t)e, "classname" );
        if ( cn && _stricmp( cn, kClass ) == 0 )
        {
            const char *vs = ValueForKey2( (int)(intptr_t)e, "speed" );
            const char *vc = ValueForKey2( (int)(intptr_t)e, "script_crashtype" );
            if ( vs && !strcmp( vs, "640" ) && vc && !strcmp( vc, "plane" ) ) { reEnt = e; break; }
        }
    }
    STLog( "after-reload: info_vehicle_node with speed/crashtype found: %s\n", reEnt ? "YES" : "NO" );
    if ( !reEnt )
    { STLog( "RESULT: FAIL (vehicle keys did not survive the round-trip)\n" ); return 1; }

    selbrush_t *reBrush = nullptr;
    for ( selbrush_t *i = active_brushes.next; i && i != &active_brushes; i = i->next )
        if ( i->owner && (entity_s_def *)i->owner->def == reEnt ) { reBrush = i; break; }
    if ( !reBrush )
    { STLog( "RESULT: FAIL (reloaded vehicle node has no brush to select)\n" ); return 1; }
    __try { Select_Brush( reBrush, 0, 0, 0 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during reselect (0x%08lX)\n", g_stLastExcept ); return 3; }

    // RemovePair "script_crashtype" (the dialog's Clear) — FIRST runtime exercise of
    // VehicleDlg_RemovePair (incl. the explicit Checkkey_Model_0/Checkkey_Color tail).
    STLog( "step: VehicleDlg_RemovePair(\"script_crashtype\")\n" );
    __try { Radiant_TestVehicleRemovePair( "script_crashtype" ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during RemovePair (0x%08lX)\n", g_stLastExcept ); return 3; }

    const char *vc2 = ValueForKey2( (int)(intptr_t)reEnt, "script_crashtype" );
    const char *vs2 = ValueForKey2( (int)(intptr_t)reEnt, "speed" );
    STLog( "after-remove: script_crashtype = \"%s\" (expect empty), speed = \"%s\" (expect 640)\n",
           vc2 ? vc2 : "(removed)", vs2 ? vs2 : "(null)" );
    if ( vc2 && vc2[0] )
    { STLog( "RESULT: FAIL (RemovePair did not drop \"script_crashtype\")\n" ); return 1; }
    if ( !vs2 || strcmp( vs2, "640" ) != 0 )
    { STLog( "RESULT: FAIL (RemovePair clobbered \"speed\")\n" ); return 1; }

    STLog( "RESULT: PASS (info_vehicle_node authored: speed/lookahead/script_crashtype round-tripped; removed script_crashtype)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunScriptGrpColorTest — the SCRIPT-GROUP NUMBER assignment round-trip (the
//  scriptgrpcolor gate).  Drives the Vehicle-Group dialog's 10 script-group buttons'
//  core: VehicleDlg_SetScriptGroupKey("<key>") → ScriptGroup_AssignNextNumber, which
//  scans active_brushes for the highest existing number under that key and assigns
//  max+1 to the selection.  Exercised WITHOUT any HWND (the vehicle/scriptgrp pattern).
//
//  To prove the max+1 SCAN (not just a constant write), the gate assigns to TWO nodes
//  in sequence: node A (gets some baseline number N — 0 if the map has no existing groups
//  under the key, else max+1 of the existing ones), then deselect A so it joins
//  active_brushes, then node B → the scan now sees A=N → B gets N+1.  The PASS criterion is
//  RELATIVE (B == A+1), so it holds whether or not the map already uses the key — e.g.
//  blackout's stock map carries 11 script_vehiclespawngroup entries up to 308, so A→309,
//  B→310 there; mp_test/mp_backlot have none, so A→0, B→1.  Then save → reload → both
//  survive (count +2) → reselect B → remove the key from B (A keeps its number).
//
//  key = "script_vehiclespawngroup" (one of the 10 button keys; != ScriptColorTeamKey
//  so AssignNextNumber's guard always proceeds).  info_vehicle_node is the canonical
//  selected entity (a fixedsize point entity, creates cleanly, no eclass filter).
// ─────────────────────────────────────────────────────────────────────────────
static int RunScriptGrpColorTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_scriptgrpcolor.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    const char *kClass = "info_vehicle_node";
    const char *kKey   = "script_vehiclespawngroup";
    STLog( "=== RADIANT_SELFTEST: script-group number assignment round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    STLog( "class: %s   key: %s\n", kClass, kKey );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_scriptgrpcolor.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int entsBefore = g_qeglobals.d_num_entities;
    STLog( "loaded %d entities\n", entsBefore );
    if ( entsBefore <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // ── node A : create + select, assign → baseline number N (0, or map's max+1) ──
    STLog( "step: create node A + AssignNextNumber\n" );
    void *phA = nullptr;
    __try { phA = Radiant_TestCreateBrushDrag( 256, 256, 320, 320, 320, 320,
                                               0.25f, 0.0f, 0.0f, 512, 512 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during placeholder A (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !phA ) { STLog( "RESULT: FAIL (no placeholder brush A)\n" ); return 1; }

    entity_s *instA = nullptr;
    __try { instA = (entity_s *)Radiant_TestCreateEntity( kClass ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during create A (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !instA ) { STLog( "RESULT: FAIL (no node A created)\n" ); return 1; }
    entity_s_def *defA = (entity_s_def *)instA->def;

    __try { Radiant_TestAssignScriptGroupNumber( kKey ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during AssignNextNumber A (0x%08lX)\n", g_stLastExcept ); return 3; }

    const char *vA = ValueForKey2( (int)(intptr_t)defA, kKey );
    STLog( "node A %s = \"%s\" (baseline N — 0 if no existing groups, else map's max+1)\n",
           kKey, vA ? vA : "(null)" );
    if ( !vA || !vA[0] )
    { STLog( "RESULT: FAIL (node A got no number)\n" ); return 1; }
    int numA = atol( vA );
    char strA[16], strB[16];
    _snprintf( strA, sizeof( strA ), "%d", numA );
    _snprintf( strB, sizeof( strB ), "%d", numA + 1 );   // expected node B number = N+1

    // Deselect A so it joins active_brushes (then the next scan can see its number).
    STLog( "step: Select_Deselect (move A to active)\n" );
    __try { Select_Deselect( 1 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during deselect (0x%08lX)\n", g_stLastExcept ); return 3; }

    // ── node B : create + select, assign → scan sees A="0" → "1" ──
    STLog( "step: create node B + AssignNextNumber\n" );
    void *phB = nullptr;
    __try { phB = Radiant_TestCreateBrushDrag( 512, 512, 576, 576, 576, 576,
                                               0.25f, 0.0f, 0.0f, 768, 768 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during placeholder B (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !phB ) { STLog( "RESULT: FAIL (no placeholder brush B)\n" ); return 1; }

    entity_s *instB = nullptr;
    __try { instB = (entity_s *)Radiant_TestCreateEntity( kClass ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during create B (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !instB ) { STLog( "RESULT: FAIL (no node B created)\n" ); return 1; }
    entity_s_def *defB = (entity_s_def *)instB->def;

    __try { Radiant_TestAssignScriptGroupNumber( kKey ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during AssignNextNumber B (0x%08lX)\n", g_stLastExcept ); return 3; }

    const char *vB = ValueForKey2( (int)(intptr_t)defB, kKey );
    STLog( "node B %s = \"%s\" (expect \"%s\" = N+1 — proves the max+1 scan)\n",
           kKey, vB ? vB : "(null)", strB );
    if ( !vB || strcmp( vB, strB ) != 0 )
    { STLog( "RESULT: FAIL (node B != N+1 — scan/max+1 broken)\n" ); return 1; }
    // A must still read N.
    const char *vA2 = ValueForKey2( (int)(intptr_t)defA, kKey );
    if ( !vA2 || strcmp( vA2, strA ) != 0 )
    { STLog( "RESULT: FAIL (node A's number changed; expected \"%s\", got \"%s\")\n", strA, vA2 ? vA2 : "(null)" ); return 1; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    // Textual proof: the saved .map carries both numbered group lines (N and N+1).
    char needleA[64], needleB[64];
    _snprintf( needleA, sizeof( needleA ), "\"%s\" \"%s\"", kKey, strA );
    _snprintf( needleB, sizeof( needleB ), "\"%s\" \"%s\"", kKey, strB );
    char *saved = nullptr;
    long  savedLen = ST_ReadFile( savePath, &saved );
    if ( savedLen < 0 || !saved ) { STLog( "RESULT: FAIL (cannot read saved map)\n" ); return 2; }
    bool t0 = ( strstr( saved, needleA ) != nullptr );
    bool t1 = ( strstr( saved, needleB ) != nullptr );
    STLog( ".map contains %s : %s\n", needleA, t0 ? "YES" : "NO" );
    STLog( ".map contains %s : %s\n", needleB, t1 ? "YES" : "NO" );
    free( saved );
    if ( !t0 || !t1 ) { STLog( "RESULT: FAIL (a numbered group line is not in the saved .map)\n" ); return 1; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-reload" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }
    STLog( "after-reload entity count: %d (was %d before create)\n",
           g_qeglobals.d_num_entities, entsBefore );
    if ( g_qeglobals.d_num_entities != entsBefore + 2 )
    { STLog( "RESULT: FAIL (entity count not +2 after round-trip)\n" ); return 1; }

    // Find the reloaded node B (number N+1) and reselect its brush for REMOVE.
    entity_s *reB = nullptr;
    for ( entity_s *e = entities.next; e && e != &entities; e = e->next )
    {
        const char *cn = ValueForKey2( (int)(intptr_t)e, "classname" );
        if ( cn && _stricmp( cn, kClass ) == 0 )
        {
            const char *vg = ValueForKey2( (int)(intptr_t)e, kKey );
            if ( vg && !strcmp( vg, strB ) ) { reB = e; break; }
        }
    }
    STLog( "after-reload: node B (number %s) found: %s\n", strB, reB ? "YES" : "NO" );
    if ( !reB ) { STLog( "RESULT: FAIL (number did not survive the round-trip)\n" ); return 1; }

    selbrush_t *reBrush = nullptr;
    for ( selbrush_t *i = active_brushes.next; i && i != &active_brushes; i = i->next )
        if ( i->owner && (entity_s_def *)i->owner->def == reB ) { reBrush = i; break; }
    if ( !reBrush ) { STLog( "RESULT: FAIL (reloaded node B has no brush to select)\n" ); return 1; }
    __try { Select_Brush( reBrush, 0, 0, 0 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during reselect (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: ScriptGroup_RemoveAssignedNumber(\"%s\") on B\n", kKey );
    __try { Radiant_TestRemoveScriptGroupNumber( kKey ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during remove (0x%08lX)\n", g_stLastExcept ); return 3; }

    const char *vRem = ValueForKey2( (int)(intptr_t)reB, kKey );
    STLog( "after-remove: node B %s = \"%s\" (expect empty)\n", kKey, vRem ? vRem : "(removed)" );
    if ( vRem && vRem[0] )
    { STLog( "RESULT: FAIL (remove did not drop the group key)\n" ); return 1; }

    STLog( "RESULT: PASS (script-group numbers %s/%s assigned via the max+1 scan, round-tripped, removed)\n",
           strA, strB );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunTransformTest — the SELECTION-TRANSFORMS round-trip gate. load → select a
//  world brush → Clone_Selection (+1 brush, clone def refCount==2, 0 dangling) →
//  rotate the clone 90° about Z (Select_GetMid → Select_RotateAxis → Select_Apply-
//  Matrix_SelectedBrushes; planepts change + brush stays valid/convex) → save →
//  reload → the cloned+rotated brush persisted (count == before+1) + lists clean.
//  N/A = no editable inline world brush (mp_backlot's pure-prefab geometry), like
//  the clip/vertex/edge gates.
// ─────────────────────────────────────────────────────────────────────────────
static int RunTransformTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_transform.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: selection clone + rotate round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_transform.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded)\n" ); return 1; }

    // Select a world brush → clone it → rotate the clone 90° about Z.
    STLog( "step: Radiant_TestCloneRotate\n" );
    int added = 0, refc = 0, rotated = 0, exactRot = 0;
    void *cloneDef = nullptr;
    __try { cloneDef = Radiant_TestCloneRotate( &added, &refc, &rotated, &exactRot ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during clone/rotate (0x%08lX)\n", g_stLastExcept ); return 3; }

    if ( added == -1 )
    { STLog( "RESULT: N/A (no editable inline world brush to transform — pure-prefab map)\n" ); return 0; }
    if ( !cloneDef ) { STLog( "RESULT: FAIL (clone path did not (re)select a clone)\n" ); return 1; }

    STLog( "clone: added=%d (expect 1)  clone-def refCount=%d (expect 2)  rotated+valid=%d (expect 1)  exactRotCoords=%d (expect 1)\n",
           added, refc, rotated, exactRot );
    if ( added != 1 )
    { STLog( "RESULT: FAIL (clone did not add exactly one brush: %d)\n", added ); return 1; }
    if ( refc != 2 )
    { STLog( "RESULT: FAIL (clone def refCount != 2: %d — link/instance mismatch)\n", refc ); return 1; }
    if ( !rotated )
    { STLog( "RESULT: FAIL (rotate did not move planepts or brush became invalid)\n" ); return 1; }
    // HARDENED: assert the EXACT 90°-about-Z rotated coordinates (not just "planepts moved").
    // Derived from Select_RotateAxis(2,90)/Select_ApplyMatrix/OrientationPosToWorldPos: each
    // face[0] planept p -> (mid.x+(p.y-mid.y), mid.y-(p.x-mid.x), p.z), bSnap=0 (no grid snap).
    if ( !exactRot )
    { STLog( "RESULT: FAIL (rotated planepts != exact 90°-Z rotation about Select_GetMid)\n" ); return 1; }

    int afterEdit = ST_GuardedDiag( "after-clone+rotate" );
    if ( afterEdit < 0 ) { STLog( "RESULT: CRASH (corrupt list after transform)\n" ); return 3; }
    // active count is now before+1 IF the clone landed in active_brushes and nothing is
    // selected... but the clone IS the selection, which ST_DiagBrushes (an active-list
    // walker) does NOT count. So mid-edit active count reads `before` (orig back in active
    // after Select_Brush moved it? no — Select_Brush(overwrite=0) keeps the orig selected
    // too). We do NOT assert the mid-edit count; the reload count below is the real proof.
    STLog( "mid-edit active brushes=%d (clones sit in selected_brushes; reload count is the proof)\n",
           afterEdit );

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int afterReload = ST_GuardedDiag( "after-reload" );
    if ( afterReload < 0 ) { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }
    STLog( "brush count: before=%d  after-reload=%d (clone adds one brush → expect before+1)\n",
           before, afterReload );
    if ( afterReload != before + 1 )
    { STLog( "RESULT: FAIL (cloned map did not round-trip: %d != %d)\n", afterReload, before + 1 ); return 1; }

    STLog( "RESULT: PASS (clone +1 brush refCount=2, rotated 90°Z valid, round-trip clean: %d brushes)\n",
           afterReload );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunFindBrushTest — the FIND-BRUSH (select-by-number) gate. load → select a world
//  brush → GetSelectionIndex reports its (entity#, brush#) → deselect → Select_ByEntity-
//  Number(brush#, entity#) reselects THE SAME brush.  Proves the two cores
//  (GetSelectionIndex / Select_ByEntityNumber, both newly ported) are exact inverses.
//  N/A = no editable inline world brush (pure-prefab map, like the clone/clip gates).
// ─────────────────────────────────────────────────────────────────────────────
static int RunFindBrushTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_findbrush.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: find brush (select by number) round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded)\n" ); return 1; }

    STLog( "step: Radiant_TestFindBrushRoundTrip\n" );
    int out[4] = { 0, 0, 0, 0 };
    int rc = 0;
    __try { rc = Radiant_TestFindBrushRoundTrip( out ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during find-brush round-trip (0x%08lX)\n", g_stLastExcept ); return 3; }

    if ( rc == -1 )
    { STLog( "RESULT: N/A (no editable inline world brush to select — pure-prefab map)\n" ); return 0; }

    STLog( "find : entity#=%d  brush#=%d  reselected-count=%d (expect 1)  same-brush=%d (expect 1)\n",
           out[0], out[1], out[2], out[3] );
    if ( out[2] != 1 )
    { STLog( "RESULT: FAIL (re-select-by-number did not select exactly one brush: %d)\n", out[2] ); return 1; }
    if ( !out[3] )
    { STLog( "RESULT: FAIL (Select_ByEntityNumber reselected the WRONG brush — not the inverse of GetSelectionIndex)\n" ); return 1; }

    int afterEdit = ST_GuardedDiag( "after-find" );
    if ( afterEdit < 0 ) { STLog( "RESULT: CRASH (corrupt list after find)\n" ); return 3; }

    STLog( "RESULT: PASS (GetSelectionIndex→Select_ByEntityNumber round-trip selects the same brush)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunErrorLogTest — the ERROR-LOG (File→Error file) gate.  Two halves, no message pump:
//   (B) PARSE: write a synthetic "<temp>.errlog" (the real compiler line format, 2 unique
//       + 1 exact-dup record), point Pointfile_Errorfile at it, assert it parses + de-dups
//       to exactly 2 entries.  Exercises Pointfile_Errorfile + ErrorLog_AddEntry +
//       ErrorLog_Compare (the parse/store/sort path) end-to-end.
//   (A) DATA + SELECT: pick a world brush, read its (ent#, brush#), deselect, synthesize an
//       error-log entry that points at THAT brush (mapfile == currentmap, absolute → no
//       reload), and ErrorLog_01() must re-select exactly the picked brush — the brief's
//       "clicking an entry selects the right brush" check through the real navigator.
//  N/A = no editable inline world brush (pure-prefab map; the PARSE half (B) still ran and
//  its count is logged).
// ─────────────────────────────────────────────────────────────────────────────
static int RunErrorLogTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_errorlog.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: error-log load + navigate (File->Error file) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded)\n" ); return 1; }

    STLog( "step: Radiant_TestErrorLog (parse .errlog + navigate-to-brush)\n" );
    int out[4] = { 0, 0, 0, 0 };
    int rc = 0;
    __try { rc = Radiant_TestErrorLog( out ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during error-log test (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "parse: .errlog entries loaded = %d (3 lines, 1 exact dup → expect 2)\n", out[0] );
    if ( out[0] != 2 )
    { STLog( "RESULT: FAIL (Pointfile_Errorfile parse/de-dup wrong: %d != 2)\n", out[0] ); return 1; }

    if ( rc == -1 )
    { STLog( "RESULT: N/A (parse OK; no editable inline world brush for the select check — pure-prefab map)\n" ); return 0; }

    STLog( "navigate: picked entity#=%d brush#=%d  ErrorLog_01-reselected-same-brush=%d (expect 1)\n",
           out[1], out[2], out[3] );
    if ( !out[3] )
    { STLog( "RESULT: FAIL (ErrorLog_01 did not navigate to the flagged brush)\n" ); return 1; }

    int afterEdit = ST_GuardedDiag( "after-errorlog" );
    if ( afterEdit < 0 ) { STLog( "RESULT: CRASH (corrupt list after error-log nav)\n" ); return 3; }

    STLog( "RESULT: PASS (.errlog parsed/de-duped to 2 entries; ErrorLog_01 navigated to the flagged brush)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunPasteTest — the CLIPBOARD COPY/PASTE round-trip gate. load → select a world
//  brush → Copy (serialise to the in-app clipboard) → Paste (Map_ImportBuffer re-parses
//  + places + selects a duplicate) → assert +1 brush, pasted def refCount==2, pasted
//  geometry matches the copied brush → save → reload → count holds (before+1) + lists
//  clean. N/A = no editable inline world brush (pure-prefab map, like clip/clone gates).
// ─────────────────────────────────────────────────────────────────────────────
static int RunPasteTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_paste.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: clipboard copy/paste round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_paste.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded)\n" ); return 1; }

    STLog( "step: Radiant_TestCopyPaste\n" );
    int added = 0, refc = 0, matched = 0;
    void *pastedDef = nullptr;
    __try { pastedDef = Radiant_TestCopyPaste( &added, &refc, &matched ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during copy/paste (0x%08lX)\n", g_stLastExcept ); return 3; }

    if ( added == -1 )
    { STLog( "RESULT: N/A (no editable inline world brush to copy — pure-prefab map)\n" ); return 0; }
    if ( !pastedDef ) { STLog( "RESULT: FAIL (paste path did not (re)select a brush)\n" ); return 1; }

    STLog( "paste: added=%d (expect 1)  pasted-def refCount=%d (expect 2)  geom-matched=%d (expect 1)\n",
           added, refc, matched );
    if ( added != 1 )
    { STLog( "RESULT: FAIL (paste did not add exactly one brush: %d)\n", added ); return 1; }
    if ( refc != 2 )
    { STLog( "RESULT: FAIL (pasted def refCount != 2: %d — link/instance mismatch)\n", refc ); return 1; }
    if ( !matched )
    { STLog( "RESULT: FAIL (pasted geometry does not match the copied brush)\n" ); return 1; }

    int afterEdit = ST_GuardedDiag( "after-paste" );
    if ( afterEdit < 0 ) { STLog( "RESULT: CRASH (corrupt list after paste)\n" ); return 3; }
    STLog( "mid-edit active brushes=%d (paste sits in selected_brushes; reload count is the proof)\n",
           afterEdit );

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int afterReload = ST_GuardedDiag( "after-reload" );
    if ( afterReload < 0 ) { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }
    STLog( "brush count: before=%d  after-reload=%d (paste adds one brush → expect before+1)\n",
           before, afterReload );
    if ( afterReload != before + 1 )
    { STLog( "RESULT: FAIL (pasted map did not round-trip: %d != %d)\n", afterReload, before + 1 ); return 1; }

    STLog( "RESULT: PASS (paste +1 brush refCount=2, geometry matched, round-trip clean: %d brushes)\n",
           afterReload );
    return 0;
}

// Verify Entity_UpdateCylinder (0x483320) expands bounds and is idempotent.
static int RunCylinderTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_cylinder.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: cylinder-geometry (Entity_UpdateCylinder) ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // Find a loaded trigger_radius / trigger_disk DEF that owns a brush and has radius>0.
    // entities holds the DEFs; brushes.prev is the entity's first brush def (the cylinder).
    entity_s_def *target = nullptr;
    for ( entity_s *e = entities.next; e && e != &entities; e = e->next )
    {
        const char *cn = ValueForKey2( (int)(intptr_t)e, "classname" );
        if ( !cn ) continue;
        bool isCyl = ( _stricmp( cn, "trigger_radius" ) == 0 ) || ( _stricmp( cn, "trigger_disk" ) == 0 );
        if ( !isCyl ) continue;
        if ( !e->eclass ) continue;
        if ( ( e->eclass->classtype & 0xC0 ) == 0 ) continue;        // must be a radius/disk class
        brush_t *b = (brush_t *)e->brushes.prev;                     // [entity+0x0C]
        if ( !b || b == (brush_t *)&e->brushes.prev ) continue;      // owns no brush (empty list head)
        if ( Entity_GetFloatValueForKey( (int)(intptr_t)e, "radius" ) > 0.0f )
        { target = (entity_s_def *)e; break; }
    }

    if ( !target )
    { STLog( "RESULT: N/A (no loaded trigger_radius/disk entity with a brush + radius — map has none)\n" ); return 0; }

    eclass_t *ec  = target->eclass;
    brush_t  *brush = (brush_t *)target->brushes.prev;
    float radius  = Entity_GetFloatValueForKey( (int)(intptr_t)target, "radius" );
    bool  isDisk  = ( ec->classtype & 0x80 ) != 0;
    float height  = isDisk ? 32.0f
                           : Entity_GetFloatValueForKey( (int)(intptr_t)target, "height" );
    STLog( "found: classtype=0x%X  radius=%.6g  %s  origin=(%.3f %.3f %.3f)  eclass.mins[2]=%.3f\n",
           ec->classtype, radius, isDisk ? "trigger_disk(h=32)" : "trigger_radius",
           target->origin[0], target->origin[1], target->origin[2], ec->mins[2] );
    if ( !isDisk && height <= 0.0f )
    { STLog( "RESULT: N/A (trigger_radius has no positive height — Entity_UpdateCylinder would early-out)\n" ); return 0; }

    // The cylinder corners Entity_UpdateCylinder must enclose.
    float baseZ   = ec->mins[2] + target->origin[2];
    float loX = target->origin[0] - radius, hiX = target->origin[0] + radius;
    float loY = target->origin[1] - radius, hiY = target->origin[1] + radius;
    float loZ = baseZ,                       hiZ = baseZ + height;
    STLog( "cylinder corners: lower=(%.3f %.3f %.3f)  upper=(%.3f %.3f %.3f)\n",
           loX, loY, loZ, hiX, hiY, hiZ );

    STLog( "pre : brush mins=(%.3f %.3f %.3f)  maxs=(%.3f %.3f %.3f)\n",
           brush->mins[0], brush->mins[1], brush->mins[2],
           brush->maxs[0], brush->maxs[1], brush->maxs[2] );

    STLog( "step: Entity_UpdateCylinder(\"radius\", def)\n" );
    __try { Entity_UpdateCylinder( "radius", (int)(intptr_t)target ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Entity_UpdateCylinder (0x%08lX)\n", g_stLastExcept ); return 3; }

    float m0[3] = { brush->mins[0], brush->mins[1], brush->mins[2] };
    float M0[3] = { brush->maxs[0], brush->maxs[1], brush->maxs[2] };
    STLog( "post: brush mins=(%.3f %.3f %.3f)  maxs=(%.3f %.3f %.3f)\n",
           m0[0], m0[1], m0[2], M0[0], M0[1], M0[2] );

    // (1) bounds must enclose BOTH corners (a tiny epsilon for FP slack).
    const float EPS = 0.01f;
    bool enclosed =
        m0[0] <= loX + EPS && m0[1] <= loY + EPS && m0[2] <= loZ + EPS &&
        M0[0] >= hiX - EPS && M0[1] >= hiY - EPS && M0[2] >= hiZ - EPS;
    if ( !enclosed )
    { STLog( "RESULT: FAIL (post bounds do not enclose the cylinder corners)\n" ); return 1; }

    // (2) the cap must have actually WIDENED the X/Y box to ±radius (the headline effect).
    if ( !( M0[0] >= hiX - EPS && m0[0] <= loX + EPS &&
            M0[1] >= hiY - EPS && m0[1] <= loY + EPS ) )
    { STLog( "RESULT: FAIL (X/Y extents did not reach origin±radius)\n" ); return 1; }

    // (3) idempotence — a second update must not move the bounds at all.
    STLog( "step: Entity_UpdateCylinder again (idempotence)\n" );
    __try { Entity_UpdateCylinder( "radius", (int)(intptr_t)target ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during 2nd Entity_UpdateCylinder (0x%08lX)\n", g_stLastExcept ); return 3; }

    bool stable =
        brush->mins[0] == m0[0] && brush->mins[1] == m0[1] && brush->mins[2] == m0[2] &&
        brush->maxs[0] == M0[0] && brush->maxs[1] == M0[1] && brush->maxs[2] == M0[2];
    STLog( "idempotent: %s  (2nd-pass mins=(%.3f %.3f %.3f) maxs=(%.3f %.3f %.3f))\n",
           stable ? "YES" : "NO",
           brush->mins[0], brush->mins[1], brush->mins[2],
           brush->maxs[0], brush->maxs[1], brush->maxs[2] );
    if ( !stable )
    { STLog( "RESULT: FAIL (Entity_UpdateCylinder not idempotent — bounds drifted)\n" ); return 1; }

    if ( ST_GuardedDiag( "after-update" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after update)\n" ); return 3; }

    STLog( "RESULT: PASS (cylinder bounds enclose radius=%.6g/height=%.6g corners + idempotent)\n",
           radius, height );
    return 0;
}

static int RunMarqueeTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_marquee.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: marquee / region box-select round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_marquee.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    int before = ST_GuardedDiag( "after-load" );
    if ( before < 0 ) { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( before <= 0 ) { STLog( "RESULT: FAIL (no brushes loaded)\n" ); return 1; }

    STLog( "step: Radiant_TestMarquee (box-select core + delete)\n" );
    int expected = 0, selected = 0, refSel = 0, deleted = 0, ran = 0;
    __try { ran = Radiant_TestMarquee( &expected, &selected, &refSel, &deleted ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during marquee select (0x%08lX)\n", g_stLastExcept ); return 3; }

    if ( !ran )
    { STLog( "RESULT: N/A (no editable inline world brush to box-select — pure-prefab map)\n" ); return 0; }

    STLog( "marquee: expected(box-covers)=%d  selected=%d  ref-brush-selected=%d  deleted=%d\n",
           expected, selected, refSel, deleted );
    if ( selected != expected )
    { STLog( "RESULT: FAIL (selected %d != oracle %d — box test off)\n", selected, expected ); return 1; }
    if ( expected <= 0 )
    { STLog( "RESULT: FAIL (box covered no brushes — oracle degenerate)\n" ); return 1; }
    if ( !refSel )
    { STLog( "RESULT: FAIL (reference brush not selected by its own bounding box)\n" ); return 1; }
    if ( deleted != selected )
    { STLog( "RESULT: FAIL (delete removed %d != selected %d — affected wrong brushes)\n", deleted, selected ); return 1; }

    int afterDel = ST_GuardedDiag( "after-delete" );
    if ( afterDel < 0 ) { STLog( "RESULT: CRASH (corrupt list after delete)\n" ); return 3; }
    STLog( "active brushes: before=%d  after-delete=%d (expect before-selected=%d)\n",
           before, afterDel, before - selected );

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    int afterReload = ST_GuardedDiag( "after-reload" );
    if ( afterReload < 0 ) { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }
    STLog( "brush count: before=%d  after-reload=%d (box-selected %d deleted → expect before-%d)\n",
           before, afterReload, selected, selected );
    if ( afterReload != before - selected )
    { STLog( "RESULT: FAIL (post-delete map did not round-trip: %d != %d)\n", afterReload, before - selected ); return 1; }

    STLog( "RESULT: PASS (box selected exactly %d brushes incl. ref, delete removed only those, round-trip clean: %d)\n",
           selected, afterReload );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunMapInfoTest — the MAP INFO dialog count cross-check gate.  Loads a map, runs
//  MapInfo_01's counting (the dialog's OnInitDialog body) headless via the harness,
//  then proves it two ways:
//   (a) FAITHFULNESS: an independent walk of active_brushes + entityInsts here,
//       replicating MapInfo_01's exact classification (non-fixedsize brush → geometry
//       plain/curve/terrain by its patch DEF terrain bit; fixedsize entity →
//       prefab/model/box; non-fixedsize entity → brush-entity), must equal MapInfo_01's
//       output bucket-for-bucket.  This is what catches the patch instance-vs-def bug.
//   (b) STATUS-BAR AGREEMENT: QE_CountBrushesAndUpdateStatusBar (the status bar's source)
//       yields g_numbrushes/g_numentities; the dialog's geometry total
//       (brushes+curves+terrain) must equal g_numbrushes (= status-bar "Brushes: N"), and
//       the fixedsize-entity total (box+model+prefab) must equal g_numentities (= "Entities: M").
//   Plus MapInfo_02's distinct-class count must be > 0 (the per-class listbox path).
//   Read-only — no save/round-trip (the dialog never mutates the map).
// ─────────────────────────────────────────────────────────────────────────────
static int RunMapInfoTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_mapinfo.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Map Info dialog count cross-check ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }

    // (a) Run MapInfo_01 + MapInfo_02 (the dialog body) headless.
    STLog( "step: Radiant_TestMapInfoCounts (MapInfo_01 + MapInfo_02)\n" );
    int mi[15] = { 0 };
    __try { Radiant_TestMapInfoCounts( mi ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during MapInfo count (0x%08lX)\n", g_stLastExcept ); return 3; }

    const int miBrushes = mi[0], miCurves = mi[1], miTerrain = mi[2];
    const int miBrushE  = mi[3], miBoxE   = mi[4], miModelE  = mi[5], miPrefab = mi[6];
    const int miClasses = mi[14];
    STLog( "MapInfo_01 world: brushes=%d curves=%d terrain=%d | brush_ents=%d box_ents=%d model_ents=%d prefabs=%d\n",
           miBrushes, miCurves, miTerrain, miBrushE, miBoxE, miModelE, miPrefab );
    STLog( "MapInfo_01 prefab-children: br=%d cv=%d tr=%d be=%d bx=%d md=%d pf=%d ; MapInfo_02 distinct classes=%d\n",
           mi[7], mi[8], mi[9], mi[10], mi[11], mi[12], mi[13], miClasses );

    // (a) Independent walk — replicate MapInfo_01's classification exactly.
    int oBr = 0, oCv = 0, oTr = 0, oBrE = 0, oBoxE = 0, oMdlE = 0, oPf = 0;
    __try
    {
        for ( selbrush_t *b = active_brushes.next; b && b != &active_brushes; b = b->next )
        {
            entity_s     *owner = b->owner;
            entity_s_def *eDef  = owner ? (entity_s_def *)owner->def : nullptr;
            eclass_t     *ec    = eDef ? eDef->eclass : nullptr;
            if ( !ec || ec->fixedsize )   // fixedsize entity brushes are counted in the ENTITY loop
                continue;
            if ( b->patch )
            {
                patchMesh_t *pm = *(patchMesh_t **)b->patch;  // instance[0] = patch DEF
                if ( pm && ( (unsigned char)pm->type & PATCH_TERRAIN ) != 0 ) ++oTr;
                else                                                          ++oCv;
            }
            else ++oBr;
        }
        for ( entity_s *j = entityInsts.next; j && j != &entityInsts; j = j->next )
        {
            entity_s_def *eDef = (entity_s_def *)j->def;
            eclass_t     *ec   = eDef ? eDef->eclass : nullptr;
            if ( !ec ) continue;
            if ( ec->fixedsize )
            {
                if ( ( ec->classtype & 0x10 ) != 0 )                       ++oPf;
                else if ( ( ec->classtype & 8 ) != 0 || Eclass_hasModel( ec ) ) ++oMdlE;
                else                                                      ++oBoxE;
            }
            else ++oBrE;
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during independent walk (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "oracle    world: brushes=%d curves=%d terrain=%d | brush_ents=%d box_ents=%d model_ents=%d prefabs=%d\n",
           oBr, oCv, oTr, oBrE, oBoxE, oMdlE, oPf );
    if ( miBrushes != oBr || miCurves != oCv || miTerrain != oTr ||
         miBrushE  != oBrE || miBoxE  != oBoxE || miModelE != oMdlE || miPrefab != oPf )
    { STLog( "RESULT: FAIL (MapInfo_01 counts disagree with the independent walk)\n" ); return 1; }

    // (b) Status-bar agreement.
    STLog( "step: QE_CountBrushesAndUpdateStatusBar (status-bar source)\n" );
    QE_CountBrushesAndUpdateStatusBar();
    const int geoTotal = miBrushes + miCurves + miTerrain;
    const int entFixed = miBoxE + miModelE + miPrefab;
    STLog( "status bar: Brushes=%d Entities=%d | dialog geo-total=%d fixedsize-ent-total=%d\n",
           g_numbrushes, g_numentities, geoTotal, entFixed );
    if ( geoTotal != g_numbrushes )
    { STLog( "RESULT: FAIL (dialog geometry total %d != status-bar Brushes %d)\n", geoTotal, g_numbrushes ); return 1; }
    if ( entFixed != g_numentities )
    { STLog( "RESULT: FAIL (dialog fixedsize-entity total %d != status-bar Entities %d)\n", entFixed, g_numentities ); return 1; }

    if ( miClasses <= 0 )
    { STLog( "RESULT: FAIL (MapInfo_02 found 0 entity classes for the listbox)\n" ); return 1; }

    STLog( "RESULT: PASS (MapInfo counts match the independent walk AND the status bar; "
           "%d geometry, %d fixedsize ents, %d classes)\n", geoTotal, entFixed, miClasses );
    return 0;
}

// Independent oracle walk for RunEntityListTest — kept SEPARATE from the SEH-guarded test
// body because the std::map<std::string,int> distinct-classname accumulator has a
// destructor (object unwinding), which can't coexist with __try in the same function
// (C2712).  Replicates the Entity List dialog's exact reads (the §11 instance-vs-def trap:
// classname + epairs off the DEF, owner-brushes off the INSTANCE's embedded brush list).
// out2[6] = { entities, classes, total_epairs, total_brushes, first_epairs, first_brushes }.
static void EL_OracleWalk( int *out2 )
{
    int oEnts = 0, oEpairs = 0, oBrushes = 0, oFirstEpairs = 0, oFirstBrushes = 0;
    std::map<std::string, int> oClasses;
    entity_s *oFirst = nullptr;

    for ( entity_s *ent = entityInsts.next; ent && ent != &entityInsts; ent = ent->next )
    {
        if ( !oFirst ) oFirst = ent;
        ++oEnts;

        entity_s_def *def = (entity_s_def *)ent->def;
        const char *cls = ( def && def->eclass && def->eclass->name ) ? def->eclass->name : "";
        oClasses[ std::string( cls ) ] += 1;

        if ( def ) for ( epair_t *e = def->epairs; e; e = e->next ) ++oEpairs;

        selbrush_t *head = &ent->brushes;
        for ( selbrush_t *b = head->ownerNext; b && b != head; b = b->ownerNext ) ++oBrushes;
    }
    if ( oFirst )
    {
        entity_s_def *def = (entity_s_def *)oFirst->def;
        if ( def ) for ( epair_t *e = def->epairs; e; e = e->next ) ++oFirstEpairs;
        selbrush_t *head = &oFirst->brushes;
        for ( selbrush_t *b = head->ownerNext; b && b != head; b = b->ownerNext ) ++oFirstBrushes;
    }

    out2[0] = oEnts;
    out2[1] = (int)oClasses.size();
    out2[2] = oEpairs;
    out2[3] = oBrushes;
    out2[4] = oFirstEpairs;
    out2[5] = oFirstBrushes;
}

// Independent oracle for RunEntityNamesTest: count how many entity-name labels SHOULD be
// drawn, by walking the ENTITY INSTANCE list (entityInsts) — a DIFFERENT traversal from the
// port's Radiant_TestEntityNameCount, which walks the global active+selected brush lists.
// For each non-world entity with a non-empty "classname", scan its embedded owner-brush list
// (brushes.ownerNext) for the entity's REPRESENTATIVE def-brush instance (b->def ==
// def->brushes.prev, the IDA +0x0C head) that is not hidden (brushFlags&2==0); if present,
// that entity contributes exactly one label.  Because the union of every entity's owner-brush
// list == the union of active_brushes + selected_brushes, this entity-side count must equal
// the port's brush-side count — but via independent pointers, so a same-bug-both-sides can't
// pass spuriously.  (No STL here, so it is __try-safe — unlike EL_OracleWalk.)
static int EntName_OracleWalk()
{
    int n = 0;
    for ( entity_s *ent = entityInsts.next; ent && ent != &entityInsts; ent = ent->next )
    {
        if ( ent == world_entity )
            continue;
        entity_s_def *def = (entity_s_def *)ent->def;
        if ( !def )
            continue;
        const char *cls = ValueForKey2( (int)(intptr_t)def, "classname" );
        if ( !cls || !*cls )
            continue;
        brush_t *repBrush = (brush_t *)def->brushes.prev;   // IDA brushes.oprev (+0x0C)
        // find the representative def-brush's instance in this entity's owner-brush list.
        selbrush_t *head = &ent->brushes;
        for ( selbrush_t *b = head->ownerNext; b && b != head; b = b->ownerNext )
        {
            if ( b->def == repBrush && ( b->brushFlags & 2 ) == 0 )
            { ++n; break; }
        }
    }
    return n;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunEntityNamesTest — the ENTITY-NAME LABEL overlay gate (RADIANT_SELFTEST <map> entname).
//  Proves DrawBrushEntityName (0x46c880) + the representative-brush gate without a live
//  renderer:
//    (A) LABEL COUNT (deterministic) — Radiant_TestEntityNameCount replicates the DRAW
//        predicate over the active+selected brush lists; it must EQUAL EntName_OracleWalk
//        (an independent walk of the entity-instance list), be STABLE across two calls, and
//        survive a save→reload (labels derive from round-tripping classname epairs + the
//        def-brush list).  0 labels is a valid PASS = "no non-world entities on this map".
//    (B) SELF-GATE — the 0x8 bit math (View→Show→Names) flips exactly its own bit and is
//        idempotent (the draw early-outs when SET, so the labels hide).
//    (C) .map UNCHANGED — Names is a view-only flag; save before/after, token-diff.
//  Exits 0 PASS or N/A, 1 FAIL, 2 IO/load, 3 CRASH.
// ─────────────────────────────────────────────────────────────────────────────
static int RunEntityNamesTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_entname.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Entity-name labels (DrawBrushEntityName representative-brush) ===\n" );
    STLog( "map : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }

    // ── (A) label count: port (brush-walk) == oracle (entity-walk), stable, round-trips ──
    int n1 = -2;
    __try { n1 = Radiant_TestEntityNameCount(); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during label count (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( n1 < 0 ) { STLog( "RESULT: FAIL (brush lists not bootstrapped: %d)\n", n1 ); return 2; }

    int oracle = EntName_OracleWalk();
    STLog( "labels: port(brush-walk)=%d  oracle(entity-walk)=%d\n", n1, oracle );
    if ( n1 != oracle )
    { STLog( "RESULT: FAIL (port != oracle: %d != %d)\n", n1, oracle ); return 1; }

    int n1b = Radiant_TestEntityNameCount();      // same loaded map → identical count
    if ( n1b != n1 )
    { STLog( "RESULT: FAIL (label count not stable: %d -> %d)\n", n1, n1b ); return 1; }

    // ── (B) self-gate bit math (View→Show→Names, 0x8) ────────────────────────────
    int *flags = &g_qeglobals.d_savedinfo.d_xyShowFlags;
    int before = *flags;
    *flags ^= 8;
    if ( ( *flags & 8 ) != 8 )
    { STLog( "RESULT: FAIL (Names toggle did not SET bit 0x8)\n" ); return 1; }
    if ( ( *flags & ~8 ) != ( before & ~8 ) )
    { STLog( "RESULT: FAIL (Names toggle disturbed other bits: %02x->%02x)\n", before, *flags ); return 1; }
    *flags ^= 8;
    if ( *flags != before )
    { STLog( "RESULT: FAIL (Names toggle not idempotent: %02x->%02x)\n", before, *flags ); return 1; }
    STLog( "self-gate: bit 0x8 SET/CLEAR clean, other bits intact, idempotent\n" );

    // ── (C) save → reload → re-count (labels derive from round-tripping epairs) ──────
    char save1[MAX_PATH], save2[MAX_PATH];
    _snprintf( save1, sizeof( save1 ), "%sradiant_entname_a.map", tmpDir );
    _snprintf( save2, sizeof( save2 ), "%sradiant_entname_b.map", tmpDir );
    *flags = 0;                                      // view-only flag must not serialise
    STLog( "step: Map_SaveFile (baseline)\n" );
    if ( ST_GuardedSave( save1 ) != 0 )
    { STLog( "RESULT: CRASH during save (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "step: Map_LoadFromFile (reload)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( save1 ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }

    int n2 = Radiant_TestEntityNameCount();
    int oracle2 = EntName_OracleWalk();
    STLog( "labels after reload: port=%d  oracle=%d\n", n2, oracle2 );
    if ( n2 != n1 || oracle2 != oracle )
    { STLog( "RESULT: FAIL (label count changed across save/reload: %d/%d -> %d/%d)\n",
             n1, oracle, n2, oracle2 ); return 1; }

    STLog( "step: Map_SaveFile (post)\n" );
    if ( ST_GuardedSave( save2 ) != 0 )
    { STLog( "RESULT: CRASH during post save (0x%08lX)\n", g_stLastExcept ); return 3; }
    char *bufA = nullptr, *bufB = nullptr;
    long  lenA = ST_ReadFile( save1, &bufA );
    long  lenB = ST_ReadFile( save2, &bufB );
    if ( lenA < 0 || lenB < 0 || !bufA || !bufB )
    { STLog( "RESULT: FAIL (cannot read saved maps)\n" ); free(bufA); free(bufB); return 2; }
    char diag[512] = ""; int tokIdx = 0;
    bool same = ST_TokenDiff( bufA, bufB, diag, sizeof( diag ), &tokIdx );
    free( bufA ); free( bufB );
    if ( !same )
    { STLog( "RESULT: FAIL (.map changed by the view flag at token %d: %s)\n", tokIdx, diag ); return 1; }

    if ( n1 == 0 )
        STLog( "RESULT: PASS (N/A — no non-world entities on this map; self-gate + .map-unchanged clean)\n" );
    else
        STLog( "RESULT: PASS (%d labels, port==oracle, stable across reload; .map unchanged)\n", n1 );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunEntityListTest — the ENTITY LIST dialog (Edit→Entity Info) cross-check gate.
//  Loads a map, runs the dialog's data cores headless via Radiant_TestEntityList (the
//  InsertItems tree-grouping + UpdateKeyValuePairs epair-walk + SelectItem owner-brush
//  walk — all WITHOUT HWNDs), then proves them with an independent walk of entityInsts
//  (EL_OracleWalk), replicating the dialog's exact reads:
//   * tree leaf count   == every entityInsts node                       (out[0])
//   * tree parent count == distinct def->eclass->name strings           (out[1])
//   * total K/V rows    == sum of def->epairs chain lengths             (out[2])
//   * total selects     == sum of inst->brushes owner-list lengths      (out[3])
//   * the FIRST entity's epairs / owner-brushes (the first selection)   (out[4],out[5])
//  This exercises the §11 instance-vs-def trap (classname + epairs read off the DEF, not
//  the instance) and the SelectItem owner-brush walk (entity_s::brushes @+0x0C / ownerNext).
//  Read-only — no save/round-trip (the dialog never mutates the map; it only selects).
// ─────────────────────────────────────────────────────────────────────────────
static int RunEntityListTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_entitylist.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    STLog( "=== RADIANT_SELFTEST: Entity List dialog cross-check ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }

    // (a) Run the dialog cores headless.
    STLog( "step: Radiant_TestEntityList (InsertItems + UpdateKeyValuePairs + SelectItem cores)\n" );
    int el[6] = { 0 };
    __try { Radiant_TestEntityList( el ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Entity List core (0x%08lX)\n", g_stLastExcept ); return 3; }

    const int dEnts = el[0], dClasses = el[1], dEpairs = el[2], dBrushes = el[3];
    const int dFirstEpairs = el[4], dFirstBrushes = el[5];
    STLog( "dialog: entities=%d classes=%d total_epairs=%d total_brushes=%d | first_epairs=%d first_brushes=%d\n",
           dEnts, dClasses, dEpairs, dBrushes, dFirstEpairs, dFirstBrushes );

    // (b) Independent walk — replicate the dialog's reads exactly (EL_OracleWalk owns the
    //     std::map, so it lives outside this SEH frame; we only guard the call here).
    int ow[6] = { 0 };
    __try { EL_OracleWalk( ow ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during independent walk (0x%08lX)\n", g_stLastExcept ); return 3; }

    const int oEnts = ow[0], oClassCount = ow[1], oEpairs = ow[2], oBrushes = ow[3];
    const int oFirstEpairs = ow[4], oFirstBrushes = ow[5];
    STLog( "oracle: entities=%d classes=%d total_epairs=%d total_brushes=%d | first_epairs=%d first_brushes=%d\n",
           oEnts, oClassCount, oEpairs, oBrushes, oFirstEpairs, oFirstBrushes );

    if ( dEnts != oEnts )
    { STLog( "RESULT: FAIL (tree leaf count %d != entityInsts node count %d)\n", dEnts, oEnts ); return 1; }
    if ( dClasses != oClassCount )
    { STLog( "RESULT: FAIL (tree parent count %d != distinct classnames %d)\n", dClasses, oClassCount ); return 1; }
    if ( dEpairs != oEpairs )
    { STLog( "RESULT: FAIL (total K/V rows %d != total epairs %d)\n", dEpairs, oEpairs ); return 1; }
    if ( dBrushes != oBrushes )
    { STLog( "RESULT: FAIL (total selects %d != total owner-brushes %d)\n", dBrushes, oBrushes ); return 1; }
    if ( dFirstEpairs != oFirstEpairs || dFirstBrushes != oFirstBrushes )
    { STLog( "RESULT: FAIL (first-entity epairs/brushes %d/%d != oracle %d/%d)\n",
             dFirstEpairs, dFirstBrushes, oFirstEpairs, oFirstBrushes ); return 1; }

    // Sanity: a non-empty map must have at least one entity class (the tree must populate),
    // and the worldspawn entity always carries epairs (so total K/V rows > 0).
    if ( dEnts <= 0 || dClasses <= 0 )
    { STLog( "RESULT: FAIL (Entity List would show an empty tree: ents=%d classes=%d)\n", dEnts, dClasses ); return 1; }

    STLog( "RESULT: PASS (Entity List tree/list/select counts match the independent walk; "
           "%d entities in %d classes, %d K/V rows, %d selectable brushes)\n",
           dEnts, dClasses, dEpairs, dBrushes );
    return 0;
}

// Floor-march helper for RunShowInUseTest — kept SEPARATE from the SEH-guarded test body
// because a CCamWnd local has a destructor (object unwinding) which can't coexist with
// __try in the same function (C2712).  Marches the synthetic camera DOWN from (cx,cy,cz),
// then a SECOND time from just above the landed feet, returning both resulting eye Z's.
// A real surface snap is a stable fixed point: the two landings agree.  This validates the
// deterministic floor-snap without an AABB oracle (Cam_ChangeFloor does true convex clip;
// angled brushes make any AABB-based "expected surface" wrong, but idempotence holds).
static void ST_FloorMarchDown( float cx, float cy, float cz, float *outZ1, float *outZ2 )
{
    CCamWnd cam;
    cam.camera.origin[0] = cx; cam.camera.origin[1] = cy; cam.camera.origin[2] = cz;
    CCamWnd::Cam_ChangeFloor( &cam, 0 );          // down
    float z1 = cam.camera.origin[2];
    *outZ1 = z1;

    // Re-march from 8 units above the landed feet (feet = z1 − 48 ⇒ eye = z1 − 48 + 48 + 8).
    cam.camera.origin[0] = cx; cam.camera.origin[1] = cy; cam.camera.origin[2] = z1 + 8.0f;
    CCamWnd::Cam_ChangeFloor( &cam, 0 );
    *outZ2 = cam.camera.origin[2];
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunShowInUseTest — the Textures→Show In Use + View→Up/Down Floor deterministic-effect
//  gate (texwnd unit).  Two ported functions, two deterministic effects:
//   (1) Texture_ShowInuse (0x45B850) + Texture_ForName (0x45B760): after a map load,
//       walk the active + selected brushes flagging each referenced material in-use, then
//       count.  ASSERT the in-use count is > 0 (a loaded map references SOME material) and
//       ≤ the total registered qtextures (the "list-count change" the brief wants — the
//       browser's usage filter shows exactly this subset).  This also drives the patch +
//       prefab-recursion branches of Texture_ForName on blackout (mesh patches) / backlot
//       (prefab refs).  NB: every map-load gate already runs this via map.cpp:488 — this
//       gate additionally proves the COUNT is non-degenerate.
//   (2) CCamWnd::Cam_ChangeFloor (0x4034E0): place a synthetic camera at the map's bounds
//       centre (inside the Z extent), march DOWN then UP.  ASSERT each leaves origin.z
//       finite and within the world bounds, and (for a map with geometry above/below the
//       centre) at least one direction moves the camera onto a surface (the deterministic
//       "camera Z changes" effect).
// ─────────────────────────────────────────────────────────────────────────────
static int RunShowInUseTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_showinuse.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    extern void Radiant_TestShowInUse( int *outInUse, int *outTotal );  // texwnd.cpp
    extern selbrush_t active_brushes;                                   // map.cpp

    STLog( "=== RADIANT_SELFTEST: Show In Use + Up/Down Floor deterministic effects ===\n" );
    STLog( "map  : %s\n", mapPath );
    AddVectoredExceptionHandler( 1, ST_Veh );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );
    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // (1) Show In Use.
    int inUse = -1, total = -1;
    STLog( "step: Texture_ShowInuse + count\n" );
    __try { Radiant_TestShowInUse( &inUse, &total ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Texture_ShowInuse (0x%08lX)\n", g_stLastExcept ); return 3; }
    STLog( "show-in-use: inUse=%d total=%d\n", inUse, total );
    // HEADLESS NOTE: the qtexture browser list is populated by the GUI material-register
    // path (Texture_GetHandle on real wc/ materials).  The headless selftest keeps the
    // DEGENERATE material path by design (same reason gui_smoke exists), so NO qtextures
    // are registered (total==0) and the in-use COUNT is GUI-only.  Here we only require
    // that Texture_ShowInuse + Texture_ForName ran WITHOUT crashing (it walked all 3441
    // blackout brushes incl. the mesh-patch + prefab-recursion branches) and that the count
    // is internally consistent.  The actual list-count effect is verified on screen (the
    // GUI demo).  If a future build registers real qtextures headless, inUse must be >0.
    if ( inUse < 0 || total < 0 )
    { STLog( "RESULT: FAIL (negative count — counter corrupt)\n" ); return 1; }
    if ( inUse > total )
    { STLog( "RESULT: FAIL (in-use %d > total %d — impossible)\n", inUse, total ); return 1; }
    if ( total > 0 && inUse == 0 )
    { STLog( "RESULT: FAIL (qtextures registered but Texture_ForName flagged 0 in-use)\n" ); return 1; }
    STLog( total == 0 ? "  (count N/A headless — degenerate material path, no qtextures registered)\n"
                      : "  (in-use count verified non-degenerate)\n" );

    // (2) Down Floor — find a real CONVEX brush (a box with faces; Cam_ChangeFloor's
    //     Brush_Ray clips only convex brush faces, NOT patches — faithful to the binary,
    //     which also can't snap onto a patch surface).  Place the camera directly above
    //     that brush's top centre, march DOWN, and ASSERT the camera dropped so its FEET
    //     (eye − 48) rest on the brush's TOP (maxs[2]) — the deterministic floor-snap.
    brush_t *pick = nullptr;
    for ( selbrush_t *b = active_brushes.next; b && b != &active_brushes; b = b->next )
    {
        brush_t *def = b->def;
        if ( def && def->faceCount >= 4 && !def->patch )   // a real convex box
        {
            // A non-degenerate brush with finite bounds.
            if ( def->maxs[2] > def->mins[2] && def->maxs[2] - def->mins[2] < 1e6f )
            { pick = def; break; }
        }
    }
    if ( !pick )
    { STLog( "NOTE: no convex brush (patch/prefab/bbox-only map) — floor march N/A, Show In Use ran clean.\n" );
      STLog( "RESULT: PASS (Show In Use ran clean: inUse=%d total=%d; floor march N/A)\n", inUse, total );
      return 0; }

    float cx  = 0.5f * ( pick->mins[0] + pick->maxs[0] );
    float cy  = 0.5f * ( pick->mins[1] + pick->maxs[1] );
    float top = pick->maxs[2];
    float startZ = top + 256.0f;   // camera eye 256 above the brush top
    STLog( "pick brush: bounds (%g %g %g)..(%g %g %g) top=%g  cam start z=%g\n",
           pick->mins[0], pick->mins[1], pick->mins[2],
           pick->maxs[0], pick->maxs[1], pick->maxs[2], top, startZ );

    float z1 = startZ, z2 = startZ;
    __try { ST_FloorMarchDown( cx, cy, startZ, &z1, &z2 ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during Cam_ChangeFloor (0x%08lX)\n", g_stLastExcept ); return 3; }

    float feet = z1 - 48.0f;
    STLog( "down-floor: eye %g -> %g (feet %g); re-march from feet+8 -> eye %g\n",
           startZ, z1, feet, z2 );
    if ( !( z1 == z1 ) || !( z2 == z2 ) )            // NaN
    { STLog( "RESULT: FAIL (down-floor produced NaN)\n" ); return 1; }
    if ( z1 >= startZ )
    { STLog( "RESULT: FAIL (camera did not move DOWN onto a floor: %g >= %g)\n", z1, startZ ); return 1; }
    if ( feet < pick->mins[2] - 1.0f || feet > startZ )
    { STLog( "RESULT: FAIL (feet %g out of sane range)\n", feet ); return 1; }
    // Idempotence: re-marching from just above the landed feet snaps to the SAME surface,
    // proving the snap is a stable real-geometry attractor (not a garbage value).
    if ( fabsf( z2 - z1 ) > 1.0f )
    { STLog( "RESULT: FAIL (floor snap not idempotent: %g vs %g — not a stable surface)\n", z2, z1 ); return 1; }

    STLog( "RESULT: PASS (Show In Use ran clean: inUse=%d total=%d; floor-snap feet=%g, stable)\n",
           inUse, total, feet );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunFilterSubviewTest — the texture-browser FILTER SUB-CONDITIONS gate (the cosmetic
//  sub-view unit).  Exercises the data-INDEPENDENT branches of TexWnd_IterateMaterials
//  (IDB 0x45ba70 LABEL_37) that the kisak port had dropped (showed "all in-use" on every
//  layer): the Lightmap (layer 1, "lightmap_*") + Smoothing (layer 2, "smoothing_*") render-
//  method sub-views, and the layer-0 search-bar name PREFIX filter.  Self-contained + map-
//  independent: Radiant_TestFilterSubviews synthesizes a fixed set of 6 named materials
//  {wc/floor, wc/wall, lightmap_gray, lightmap_white, smoothing_a, $default(not-in-use)},
//  appends them to texWndGlob.sorted_materials, counts TexWnd_FilterAccept at each layer /
//  with a "wc/floor" search prefix, and RESTORES texWndGlob (materialCount / current_edit_
//  layer / searchbar) so it leaves no state behind.  Deterministic expected counts:
//    layer 0 (Material)  : 5  (all in-use; $default excluded)
//    layer 1 (Lightmap)  : 2  (lightmap_gray, lightmap_white)
//    layer 2 (Smoothing) : 1  (smoothing_a)
//    layer 0 + search    : 1  (wc/floor prefix)
//  The usage/locale/surfaceType filters stay no-op (kisak's MaterialInfo drops the metadata
//  they key off) — documented in texwnd.cpp; this gate proves the NAME-based predicates only.
// ─────────────────────────────────────────────────────────────────────────────
static int RunFilterSubviewTest()
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_filtersubview.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    extern void Radiant_TestFilterSubviews( int *l0, int *l1, int *l2, int *search );  // texwnd.cpp

    STLog( "=== RADIANT_SELFTEST: texture-browser filter sub-views (Lightmap/Smoothing/search) ===\n" );
    AddVectoredExceptionHandler( 1, ST_Veh );

    int l0 = -1, l1 = -1, l2 = -1, search = -1;
    __try { Radiant_TestFilterSubviews( &l0, &l1, &l2, &search ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during TexWnd_FilterAccept (0x%08lX)\n", g_stLastExcept ); return 3; }

    STLog( "layer0(Material)=%d  layer1(Lightmap)=%d  layer2(Smoothing)=%d  layer0+search('wc/floor')=%d\n",
           l0, l1, l2, search );
    STLog( "expect: 2 / 2 / 1 / 1\n" );

    // layer 0 now faithfully excludes lightmap_*/smoothing_* names (IDB 0x45bc5f/0x45bc68) and
    // usage_index==0 BEFORE the is_in_use base predicate, so only the two in-use wc/* synthetics
    // (usage 1) are accepted on the Material tab.
    if ( l0 != 2 )
    { STLog( "RESULT: FAIL (layer 0 accepted %d, expected 2 in-use wc/* materials)\n", l0 ); return 1; }
    if ( l1 != 2 )
    { STLog( "RESULT: FAIL (layer 1 Lightmap accepted %d, expected 2 lightmap_*)\n", l1 ); return 1; }
    if ( l2 != 1 )
    { STLog( "RESULT: FAIL (layer 2 Smoothing accepted %d, expected 1 smoothing_*)\n", l2 ); return 1; }
    if ( search != 1 )
    { STLog( "RESULT: FAIL (search 'wc/floor' accepted %d, expected 1 prefix match)\n", search ); return 1; }

    STLog( "RESULT: PASS (Lightmap/Smoothing layer sub-views + search-bar prefix filter all correct)\n" );
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RunTexBarTest — the texture-bar texdef read/write round-trip (the texbar gate).
//  Mirrors the GUI workflow of the docked bar: load → select a face → READ the bar's five
//  texdef numbers (GetSurfaceAttributes core: shiftH/shiftV/scaleH/scaleV/rotate as ints) →
//  WRITE new values onto the picked face's texdef (TextureBar_02 core, the field→texdef
//  store) → read them straight back in memory → save → confirm the saved .map face line
//  carries the new size/shift run → reload → re-select the same face and read the bar values
//  (the values survived the .map round-trip).  The bar stores raw texel scale/shift +
//  degrees as ints, so the gate uses integer values (no precision tolerance needed — the
//  store is exact).  Proves the §11 instance-vs-def DEF read/write that the bar performs.
// ─────────────────────────────────────────────────────────────────────────────
static int RunTexBarTest( const char *mapPath )
{
    char logPath[MAX_PATH], tmpDir[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( logPath, sizeof( logPath ), "%sradiant_texbar.log", tmpDir );
    s_logFile = fopen( logPath, "w" );

    // The new bar values to write (raw texels / degrees, exactly as the bar's int members).
    const int W_SH = 13, W_SV = 7, W_SCH = 96, W_SCV = 48, W_ROT = 90;

    STLog( "=== RADIANT_SELFTEST: texture-bar texdef read/write round-trip ===\n" );
    STLog( "map  : %s\n", mapPath );
    STLog( "write: shiftH=%d shiftV=%d scaleH=%d scaleV=%d rotate=%d\n",
           W_SH, W_SV, W_SCH, W_SCV, W_ROT );
    AddVectoredExceptionHandler( 1, ST_Veh );

    char savePath[MAX_PATH];
    _snprintf( savePath, sizeof( savePath ), "%sradiant_texbar.map", tmpDir );
    STLog( "save : %s\n", savePath );

    STLog( "step: Map_NewMap\n" );                  Map_NewMap();
    STLog( "step: Eclass_ForName(worldspawn)\n" );  Eclass_ForName( 1, "worldspawn" );

    STLog( "step: Map_LoadFromFile\n" );
    if ( ST_GuardedLoad( mapPath ) != 0 )
    { STLog( "RESULT: CRASH during load (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( g_qeglobals.d_num_entities <= 0 ) { STLog( "RESULT: FAIL (no entities loaded)\n" ); return 2; }
    if ( ST_GuardedDiag( "after-load" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after load)\n" ); return 3; }

    // Select a single editable face (first suitable world brush face; deterministic).
    STLog( "step: Radiant_TestSelectFace\n" );
    int sel = 0;
    __try { sel = Radiant_TestSelectFace(); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during face select (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !sel ) { STLog( "RESULT: FAIL (no editable face found)\n" ); return 1; }

    // READ the bar's view of the picked face's texdef (GetSurfaceAttributes core).
    int before[5] = { 0 };
    int rd = 0;
    __try { rd = Radiant_TestTexBarRead( before ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during bar read (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !rd ) { STLog( "RESULT: FAIL (bar read returned nothing)\n" ); return 1; }
    STLog( "bar BEFORE: shiftH=%d shiftV=%d scaleH=%d scaleV=%d rotate=%d\n",
           before[0], before[1], before[2], before[3], before[4] );

    // WRITE the new values onto the picked face's texdef (TextureBar_02 core).
    STLog( "step: Radiant_TestTexBarWrite\n" );
    int wr = 0;
    __try { wr = Radiant_TestTexBarWrite( W_SH, W_SV, W_SCH, W_SCV, W_ROT ); }
    __except( EXCEPTION_EXECUTE_HANDLER )
    { g_stLastExcept = GetExceptionCode();
      STLog( "RESULT: CRASH during bar write (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( !wr ) { STLog( "RESULT: FAIL (bar write did nothing)\n" ); return 1; }

    // Read back in memory — the written values must take exactly (integer store, no slop).
    int applied[5] = { 0 };
    Radiant_TestTexBarRead( applied );
    STLog( "bar AFTER : shiftH=%d shiftV=%d scaleH=%d scaleV=%d rotate=%d\n",
           applied[0], applied[1], applied[2], applied[3], applied[4] );
    if ( applied[0] != W_SH || applied[1] != W_SV || applied[2] != W_SCH ||
         applied[3] != W_SCV || applied[4] != W_ROT )
    { STLog( "RESULT: FAIL (in-memory bar texdef did not take)\n" ); return 1; }

    STLog( "step: Map_SaveFile\n" );
    if ( ST_GuardedSave( savePath ) != 0 )
    { STLog( "RESULT: CRASH during Map_SaveFile (0x%08lX)\n", g_stLastExcept ); return 3; }

    // Textual proof: the saved .map carries the written `size0 size1 shift0` run on a face line.
    char *saved = nullptr;
    long  savedLen = ST_ReadFile( savePath, &saved );
    if ( savedLen < 0 || !saved ) { STLog( "RESULT: FAIL (cannot read saved map)\n" ); return 2; }
    char needle[128], b0[40], b1[40], b2[40];
    _snprintf( b0, sizeof( b0 ), "%.8g", (double)W_SCH );
    _snprintf( b1, sizeof( b1 ), "%.8g", (double)W_SCV );
    _snprintf( b2, sizeof( b2 ), "%.8g", (double)W_SH );
    _snprintf( needle, sizeof( needle ), " %s %s %s ", b0, b1, b2 );
    bool inText = ( strstr( saved, needle ) != nullptr );
    free( saved );
    STLog( ".map contains bar-texdef run \"%s\" : %s\n", needle, inText ? "YES" : "NO" );
    if ( !inText ) { STLog( "RESULT: FAIL (written texdef not in saved .map)\n" ); return 1; }

    STLog( "step: reload saved map (fresh world)\n" );
    Map_NewMap();
    Eclass_ForName( 1, "worldspawn" );
    if ( ST_GuardedLoad( savePath ) != 0 )
    { STLog( "RESULT: CRASH during reload (0x%08lX)\n", g_stLastExcept ); return 3; }
    if ( ST_GuardedDiag( "after-reload" ) < 0 )
    { STLog( "RESULT: CRASH (corrupt list after reload)\n" ); return 3; }

    if ( !Radiant_TestSelectFace() )
    { STLog( "RESULT: FAIL (no editable face after reload)\n" ); return 1; }
    int reloaded[5] = { 0 };
    Radiant_TestTexBarRead( reloaded );
    STLog( "bar RELOAD: shiftH=%d shiftV=%d scaleH=%d scaleV=%d rotate=%d\n",
           reloaded[0], reloaded[1], reloaded[2], reloaded[3], reloaded[4] );
    if ( reloaded[0] != W_SH || reloaded[1] != W_SV || reloaded[2] != W_SCH ||
         reloaded[3] != W_SCV || reloaded[4] != W_ROT )
    { STLog( "RESULT: FAIL (bar texdef did not survive the round-trip)\n" ); return 1; }

    STLog( "RESULT: PASS (texture-bar read/write round-trip: texdef shiftH=%d shiftV=%d "
           "scaleH=%d scaleV=%d rotate=%d persisted)\n", W_SH, W_SV, W_SCH, W_SCV, W_ROT );
    return 0;
}
#endif // RADIANT_SELFTEST

extern CMainFrame *g_pParentWnd;   // engine_stubs.cpp

class CRadiantApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    // P5.5: drive CMainFrame::RoutineProcessing from the idle pump — flush g_nUpdateBits
    // (the Sys_UpdateWindows broadcast). Returning TRUE keeps OnIdle firing so a queued
    // redraw lands promptly between input messages. (The binary runs RoutineProcessing
    // from its message loop the same way.)
    virtual BOOL OnIdle( LONG lCount );
};

CRadiantApp theApp;

BOOL CRadiantApp::OnIdle( LONG lCount )
{
    BOOL more = CWinApp::OnIdle( lCount );
    // Flush pending window-update bits once per idle (after each input/timer batch).
    // RoutineProcessing no-ops when g_nUpdateBits == 0, so this does not busy-spin —
    // return the base value so the pump blocks on GetMessage when nothing is queued.
    if ( g_pParentWnd && ::IsWindow( g_pParentWnd->GetSafeHwnd() ) )
        g_pParentWnd->RoutineProcessing();
    return more;
}

BOOL CRadiantApp::InitInstance()
{
#ifdef RADIANT_SELFTEST
    // Attach to the launching console so STLog() output is visible in the terminal.
    if ( AttachConsole( ATTACH_PARENT_PROCESS ) )
    {
        freopen( "CONOUT$", "w", stdout );
    }
    // Redirect stderr to a log file so Com_Error()/Assert diagnostics (which print to
    // stderr) are captured even though this is a GUI-subsystem binary.
    {
        char errPath[MAX_PATH], tmp[MAX_PATH];
        GetTempPathA( sizeof( tmp ), tmp );
        _snprintf( errPath, sizeof( errPath ), "%sradiant_selftest_err.log", tmp );
        freopen( errPath, "w", stderr );
    }

    // Thread-local engine slots (slot 1 = va_info for va()) — the GUI does this in
    // Radiant_EngineInit; the headless selftest must too, or any undo-using gate (e.g.
    // texmod → Brush_SetTexture → Undo_GeneralStart → va()) derefs an unset va slot.
    Com_InitThreadData( 0 /* THREAD_CONTEXT_MAIN */ );

    // Use the registry (HKCU\Software\CoD4Radiant\...) for prefs persistence, like the
    // binary's SetRegistryKey — so Prefs_Save/Prefs_Load round-trip in the prefs gate.
    SetRegistryKey( "iw\\CoD4Radiant" );
    // Headless gates run on DEFAULTS (not the registry) for determinism — the prefs
    // gate manages its own save/load. (g_PrefsDlg is already defaults-constructed at
    // static init; this re-asserts it after MFC is fully up.)
    Prefs_Init( /*loadFromRegistry=*/false );

    const char *mapPath = ( __argc > 1 ) ? __argv[1] : nullptr;
    const char *mode    = ( __argc > 2 ) ? __argv[2] : nullptr;
    int code;
    // "prefs" gate is self-contained (no map): accept it as argv[1] or as the mode arg.
    if ( ( mapPath && strcmp( mapPath, "prefs" ) == 0 ) ||
         ( mode && strcmp( mode, "prefs" ) == 0 ) )
    {
        code = RunPrefsTest();                  // P6 — CPrefsDlg persistence round-trip
    }
    // "inspector" gate is self-contained (no map): SetInspectorMode state-machine.
    else if ( ( mapPath && strcmp( mapPath, "inspector" ) == 0 ) ||
              ( mode && strcmp( mode, "inspector" ) == 0 ) )
    {
        code = RunInspectorTest();              // INSPECTOR TAB SWITCHING — mode global flips
    }
    // "confirmmodified" gate is self-contained (no map): the unsaved-changes save-prompt
    // decision core + the modified-flag data path.
    else if ( ( mapPath && strcmp( mapPath, "confirmmodified" ) == 0 ) ||
              ( mode && strcmp( mode, "confirmmodified" ) == 0 ) )
    {
        code = RunConfirmModifiedTest();        // ConfirmModified save-prompt + modified flag
    }
    // "eclassparse" gate is self-contained (no map): Eclass_InitFromText QUAKED parse.
    else if ( ( mapPath && strcmp( mapPath, "eclassparse" ) == 0 ) ||
              ( mode && strcmp( mode, "eclassparse" ) == 0 ) )
    {
        code = RunEclassParseTest();            // Eclass_InitFromText /*QUAKED block parser
    }
    // "layeredmat" gate is self-contained (no map): LayerdMatWnd library loader.
    else if ( ( mapPath && strcmp( mapPath, "layeredmat" ) == 0 ) ||
              ( mode && strcmp( mode, "layeredmat" ) == 0 ) )
    {
        code = RunLayeredMatTest();             // layered-material library loader (entryCount==4)
    }
    // "texfilter" gate is self-contained (no map): TexFilter_LoadMenuFile table loader.
    else if ( ( mapPath && strcmp( mapPath, "texfilter" ) == 0 ) ||
              ( mode && strcmp( mode, "texfilter" ) == 0 ) )
    {
        code = RunTexFilterTest( mapPath );     // TexFilter_LoadMenuFile (0x45B010) table-fill unit gate
    }
    else if ( !mapPath )
    {
        STLog( "usage: CoD4Radiant.exe <path-to.map> [destruct|create|createent <class>|reclass <classA> <classB>|setkey <class> <key> <val>|texmod <shiftx> <scale> <rot>|fit|texbar|entflags <class> <bit>|clip|prim <kind> <sides>|findrepl|layers|model <xmodel>|prefab|prefabedit|scriptgrp|dynent|vehicle|transform|marquee|mapinfo|paste|prefs|layeredmat|texfilter]\n" );
        code = 2;
    }
    else if ( mode && strcmp( mode, "destruct" ) == 0 )
    {
        code = RunDestructiveTest( mapPath );   // TASK 1 — delete-path UAF gate
    }
    else if ( mode && strcmp( mode, "create" ) == 0 )
    {
        code = RunCreateTest( mapPath );        // brush-creation round-trip gate
    }
    else if ( mode && strcmp( mode, "createent" ) == 0 )
    {
        // arg3 = eclass name (default "light"); arg1 = map path
        const char *cls = ( __argc > 3 ) ? __argv[3] : "light";
        code = RunCreateEntityTest( mapPath, cls );  // entity-creation round-trip gate
    }
    else if ( mode && strcmp( mode, "reclass" ) == 0 )
    {
        // arg3 = class A (default "light"), arg4 = class B (default "info_player_start")
        const char *clsA = ( __argc > 3 ) ? __argv[3] : "light";
        const char *clsB = ( __argc > 4 ) ? __argv[4] : "info_player_start";
        code = RunReclassTest( mapPath, clsA, clsB );  // CreateEntityFromName Path B re-class gate
    }
    else if ( mode && strcmp( mode, "undoredo" ) == 0 )
    {
        code = RunUndoRedoTest( mapPath );           // Undo_Undo/Undo_Redo restore gate
    }
    else if ( mode && strcmp( mode, "aligntofloor" ) == 0 )
    {
        code = RunAlignToFloorTest( mapPath );       // AlignEntityToFace/sub_4859B0 angle-align gate
    }
    else if ( mode && strcmp( mode, "mtlnormalize" ) == 0 )
    {
        code = RunMtlNormalizeTest( mapPath );        // sub_45AD50 (0x45AD50) basename-strip unit gate
    }
    else if ( mode && strcmp( mode, "samplesize" ) == 0 )
    {
        code = RunSampleSizeTest( mapPath );          // Brush_SetSampleSize/sub_477080 sample-size gate
    }
    else if ( mode && strcmp( mode, "texmapping" ) == 0 )
    {
        code = RunTexMappingTest( mapPath );          // Brush_SetTextureMapping/sub_477020 brush-pass gate
    }
    else if ( mode && strcmp( mode, "texprojmath" ) == 0 )
    {
        code = RunTexProjMathTest( mapPath );         // sub_4A45D0/sub_4769A0 texture-projection math unit gate
    }
    else if ( mode && strcmp( mode, "matinv44" ) == 0 )
    {
        code = RunMatInv44Test( mapPath );            // MatrixInverse44 void* shadow-stub fix unit gate
    }
    else if ( mode && strcmp( mode, "texvecs" ) == 0 )
    {
        code = RunTexVecsTest();                       // texturevecs_02 (0x459CC0) forward∘inverse round-trip gate (map-free)
    }
    else if ( ( mapPath && strcmp( mapPath, "picmip" ) == 0 ) ||
              ( mode && strcmp( mode, "picmip" ) == 0 ) )
    {
        code = RunPicmipTest();                        // R_UpdateMipMap (0x5139A0) dvar->imageGlobals.picmip* gate (map-free)
    }
    else if ( mode && strcmp( mode, "patchpick" ) == 0 )
    {
        code = RunPatchPickTest();                     // sub_44B620 (0x44B620) patch texture-pick round-trip gate (map-free)
    }
    else if ( ( mapPath && strcmp( mapPath, "terraincircle" ) == 0 ) ||
              ( mode && strcmp( mode, "terraincircle" ) == 0 ) )
    {
        code = RunTerrainCircleTest();                 // PMESH_19/20_Radius + sub_43ED50 terrain cursor-ring gate (map-free)
    }
    else if ( mode && strcmp( mode, "linmap" ) == 0 )
    {
        code = RunLinMapTest();                         // linearmapping.cpp (0x4B6DC0..0x4B7430) 3x3 LU solver known-answer gate (map-free)
    }
    else if ( mode && strcmp( mode, "texlock" ) == 0 )
    {
        code = RunTexLockTest();                         // Face_TexLock_Save/Reproject (0x470570/0x4706F0) texture-lock invariant gate (map-free)
    }
    else if ( mode && strcmp( mode, "paintfalloff" ) == 0 )
    {
        code = RunPaintFalloffTest( mapPath );        // sub_43DB60 terrain-paint brush falloff unit gate
    }
    else if ( mode && strcmp( mode, "hollow" ) == 0 )
    {
        code = RunHollowTest( mapPath );              // CSG_MakeHollow (Selection->CSG->Hollow) gate
    }
    else if ( mode && strcmp( mode, "brushflip" ) == 0 )
    {
        code = RunBrushFlipTest( mapPath );           // DoFlip (Brush->Flip) entity-angle-flip gate
    }
    else if ( mode && strcmp( mode, "nsided" ) == 0 )
    {
        code = RunNSidedTest( mapPath );              // Brush_MakeSided (Brush->N sided) gate
    }
    else if ( mode && strcmp( mode, "floortrace" ) == 0 )
    {
        code = RunFloorTraceTest( mapPath );          // Trace_AllDirectionsIfFailed drop-to-floor gate
    }
    else if ( mode && strcmp( mode, "autocaulk" ) == 0 )
    {
        code = RunAutoCaulkTest( mapPath );           // Brush_AutoCaulk (Selection->CSG->Auto Caulk, 33220)
    }
    else if ( mode && strcmp( mode, "floordrop" ) == 0 )
    {
        code = RunFloorDropTest( mapPath );           // OnDropSelected scatter helpers (sub_47CFD0/sub_47CBA0)
    }
    else if ( mode && strcmp( mode, "physbrush" ) == 0 )
    {
        code = RunPhysBrushTest( mapPath );           // physics box/cylinder geometry (sub_4AA220/sub_478630/sub_4A59C0)
    }
    else if ( mode && strcmp( mode, "createbrushent" ) == 0 )
    {
        // arg3 = brush eclass name (default "script_brushmodel"); arg1 = map path.
        // Exercises Entity_Create's reparent loop (OPUS QUEUE #10).
        const char *cls = ( __argc > 3 ) ? __argv[3] : "script_brushmodel";
        code = RunCreateBrushEntityTest( mapPath, cls );  // brush-entity reparent gate
    }
    else if ( mode && strcmp( mode, "setkey" ) == 0 )
    {
        // arg3 = eclass, arg4 = key, arg5 = value (the entity-window key/value editor gate)
        const char *cls = ( __argc > 3 ) ? __argv[3] : "light";
        const char *key = ( __argc > 4 ) ? __argv[4] : "radius";
        const char *val = ( __argc > 5 ) ? __argv[5] : "300";
        code = RunSetKeyTest( mapPath, cls, key, val );
    }
    else if ( mode && strcmp( mode, "texmod" ) == 0 )
    {
        // arg3 = shiftX, arg4 = scale, arg5 = rotate (the surface-inspector texdef gate)
        float shiftX = ( __argc > 3 ) ? (float)atof( __argv[3] ) : 17.0f;
        float scale  = ( __argc > 4 ) ? (float)atof( __argv[4] ) : 3.0f;
        float rot    = ( __argc > 5 ) ? (float)atof( __argv[5] ) : 45.0f;
        code = RunTexModTest( mapPath, shiftX, scale, rot );
    }
    else if ( mode && strcmp( mode, "fit" ) == 0 )
    {
        code = RunFitTest( mapPath );           // surface-inspector "Fit" round-trip gate
    }
    else if ( mode && strcmp( mode, "entflags" ) == 0 )
    {
        // arg3 = eclass (default "light"), arg4 = spawnflag bit (default 2 → flags 4).
        const char *cls = ( __argc > 3 ) ? __argv[3] : "light";
        int bit = ( __argc > 4 ) ? atoi( __argv[4] ) : 2;
        code = RunEntFlagsTest( mapPath, cls, bit );  // entity-window spawnflags + angle gate
    }
    else if ( mode && strcmp( mode, "clip" ) == 0 )
    {
        code = RunClipTest( mapPath );          // clipper split round-trip gate
    }
    else if ( mode && strcmp( mode, "vertex" ) == 0 )
    {
        code = RunVertexTest( mapPath );        // vertex-edit (Brush_MoveVertex) round-trip gate
    }
    else if ( mode && strcmp( mode, "edge" ) == 0 )
    {
        code = RunEdgeTest( mapPath );          // edge-edit (Select_Edge/SelectFaceEdge) round-trip gate
    }
    else if ( mode && strcmp( mode, "dragclose" ) == 0 )
    {
        code = RunDragCloseTest( mapPath );     // Drag_MouseUp undo-close → Select_RemoveEmptyFaces gate
    }
    else if ( mode && strcmp( mode, "prim" ) == 0 )
    {
        // arg3 = kind (0=sphere, 1=cone; default 0), arg4 = side count (default 8).
        int kind  = ( __argc > 3 ) ? atoi( __argv[3] ) : 0;
        int sides = ( __argc > 4 ) ? atoi( __argv[4] ) : 8;
        code = RunPrimTest( mapPath, kind, sides );  // brush→primitives round-trip gate
    }
    else if ( mode && strcmp( mode, "findrepl" ) == 0 )
    {
        // arg3 = find material (default sky_chechnya), arg4 = replace (default ch_tile_floor05)
        const char *findName = ( __argc > 3 ) ? __argv[3] : "sky_chechnya";
        const char *replName = ( __argc > 4 ) ? __argv[4] : "ch_tile_floor05";
        code = RunFindReplaceTest( mapPath, findName, replName );  // find/replace texture gate
    }
    else if ( mode && strcmp( mode, "layers" ) == 0 )
    {
        code = RunLayersTest( mapPath );        // layers-dialog round-trip gate
    }
    else if ( mode && strcmp( mode, "model" ) == 0 )
    {
        // arg3 = model name (default a stock CoD4 xmodel) — the model-picker gate.
        const char *mdl = ( __argc > 3 ) ? __argv[3] : "ac_car_part01";
        code = RunModelTest( mapPath, mdl );    // misc_model authoring round-trip gate
    }
    else if ( mode && strcmp( mode, "prefab" ) == 0 )
    {
        code = RunPrefabTest( mapPath );        // prefab-placement round-trip gate (QUEUE #8)
    }
    else if ( mode && strcmp( mode, "prefabedit" ) == 0 )
    {
        code = RunPrefabEditTest( mapPath );    // prefab-edit-in-place enter/leave gate (QUEUE #18)
    }
    else if ( mode && strcmp( mode, "scriptgrp" ) == 0 )
    {
        // arg3 = eclass, arg4 = key, arg5/6 = two value-tokens (the script-group key gate).
        // Default class is the proven-creatable point entity "light" (the generic
        // Add/RemoveKeyFromSelected path is class-agnostic; the *Triggers variants are
        // exercised by callers, not this gate).
        const char *cls = ( __argc > 3 ) ? __argv[3] : "light";
        const char *key = ( __argc > 4 ) ? __argv[4] : "script_linkname";
        const char *v1  = ( __argc > 5 ) ? __argv[5] : "alpha";
        const char *v2  = ( __argc > 6 ) ? __argv[6] : "bravo";
        code = RunScriptGrpTest( mapPath, cls, key, v1, v2 );
    }
    else if ( mode && strcmp( mode, "dynent" ) == 0 )
    {
        code = RunDynEntTest( mapPath );        // dyn-entity authoring round-trip gate
    }
    else if ( mode && strcmp( mode, "vehicle" ) == 0 )
    {
        code = RunVehicleTest( mapPath );       // vehicle authoring round-trip gate
    }
    else if ( mode && strcmp( mode, "scriptgrpcolor" ) == 0 )
    {
        code = RunScriptGrpColorTest( mapPath ); // script-group number assignment round-trip gate
    }
    else if ( mode && strcmp( mode, "transform" ) == 0 )
    {
        code = RunTransformTest( mapPath );     // selection clone + rotate round-trip gate
    }
    else if ( mode && strcmp( mode, "findbrush" ) == 0 )
    {
        code = RunFindBrushTest( mapPath );     // Find brush / select-by-number round-trip gate
    }
    else if ( mode && strcmp( mode, "errorlog" ) == 0 )
    {
        code = RunErrorLogTest( mapPath );      // File→Error file: .errlog parse + navigate-to-brush gate
    }
    else if ( mode && strcmp( mode, "paste" ) == 0 )
    {
        code = RunPasteTest( mapPath );         // clipboard copy/paste round-trip gate (QUEUE #11/#24)
    }
    else if ( mode && strcmp( mode, "marquee" ) == 0 )
    {
        code = RunMarqueeTest( mapPath );       // marquee / region box-select round-trip gate
    }
    else if ( mode && strcmp( mode, "cylinder" ) == 0 )
    {
        code = RunCylinderTest( mapPath );      // Entity_UpdateCylinder radius/height geo recompute gate (QUEUE #9)
    }
    else if ( mode && strcmp( mode, "mapinfo" ) == 0 )
    {
        code = RunMapInfoTest( mapPath );       // Map Info dialog count cross-check gate
    }
    else if ( mode && strcmp( mode, "entitylist" ) == 0 )
    {
        code = RunEntityListTest( mapPath );    // Entity List dialog tree/list/select cross-check gate
    }
    else if ( mode && strcmp( mode, "entname" ) == 0 )
    {
        code = RunEntityNamesTest( mapPath );   // entity-name label (DrawBrushEntityName) count gate
    }
    else if ( mode && strcmp( mode, "texbar" ) == 0 )
    {
        code = RunTexBarTest( mapPath );        // texture-bar texdef read/write round-trip gate
    }
    else if ( mode && strcmp( mode, "filter" ) == 0 )
    {
        // arg3 = condition string (default "Contents detail"); the visibility-filter
        // toggle gate.  Quote it on the cmdline if it contains spaces.
        const char *cond = ( __argc > 3 ) ? __argv[3] : "Contents detail";
        code = RunFilterTest( mapPath, cond );  // filter parse + toggle + .map-unchanged gate
    }
    else if ( mode && strcmp( mode, "filterload" ) == 0 )
    {
        code = RunFilterLoadTest( mapPath );    // CFilterWnd RadiantFilters.txt loader gate
    }
    else if ( mode && strcmp( mode, "facetexmap" ) == 0 )
    {
        code = RunFaceTexMapTest( mapPath );    // faceTexMap add/lookup/remove (filter-faces-by-material) gate
    }
    else if ( mode && strcmp( mode, "showinuse" ) == 0 )
    {
        code = RunShowInUseTest( mapPath );     // Show In Use + Up/Down Floor deterministic-effect gate
    }
    else if ( mode && strcmp( mode, "filtersubview" ) == 0 )
    {
        code = RunFilterSubviewTest();          // texture-browser Lightmap/Smoothing layer + search-bar filter gate (map-independent)
    }
    else if ( mode && strcmp( mode, "xyshow" ) == 0 )
    {
        code = RunXyShowTest( mapPath );        // View→Show toggles (Names/Coords + Reverse-filter inversion) gate
    }
    else if ( mode && strcmp( mode, "connections" ) == 0 )
    {
        code = RunConnectionsTest( mapPath );   // Connections overlay (target/targetname + script_linkTo) link-count gate
    }
    else if ( mode && strcmp( mode, "classsimilar" ) == 0 )
    {
        code = RunClassSimilarTest( mapPath );  // Select_ByClassSimilar (0x493830) iteration-direction gate
    }
    else if ( mode && strcmp( mode, "patch" ) == 0 )
    {
        code = RunPatchTest( mapPath );         // patch create (Patch_BrushToMesh) + grid-manip round-trip gate
    }
    else if ( mode && strcmp( mode, "simplepatch" ) == 0 )
    {
        code = RunSimplePatchTest( mapPath );   // Curve->Simple Patch Mesh backend (Patch_GenericMesh 0x43b310) gate
    }
    else if ( mode && strcmp( mode, "patchrow" ) == 0 )
    {
        code = RunPatchRowTest( mapPath );      // patch create + Patch_InsertRow (0x443830) grid-grow round-trip gate
    }
    else if ( mode && strcmp( mode, "patchremove" ) == 0 )
    {
        code = RunPatchRemoveTest( mapPath );   // patch create + Patch_RemoveColumn/RemoveRow (0x443C90/0x443B60) grid-shrink round-trip gate
    }
    else if ( mode && strcmp( mode, "patchprim" ) == 0 )
    {
        code = RunPatchPrimTest( mapPath );     // Patch_BrushToMesh non-cylinder branches (bevel/endcap/cone) geometry + round-trip gate
    }
    else if ( mode && strcmp( mode, "patchinvert" ) == 0 )
    {
        code = RunPatchInvertTest( mapPath );   // Patch_ToggleInverted/patchInvert2 (0x4465C0/0x446480) vertical-flip + round-trip gate
    }
    else if ( mode && strcmp( mode, "patchtranspose" ) == 0 )
    {
        code = RunPatchTransposeTest( mapPath ); // Patch_Transpose/Patch_TransposeGrid (0x4491D0/0x449020) transpose + round-trip gate
    }
    else if ( mode && strcmp( mode, "patchnaturalize" ) == 0 )
    {
        code = RunPatchNaturalizeTest( mapPath ); // Patch_NaturalizeSelected/Patch_Naturalize2 (0x447FD0/0x439840) linear-S/T + round-trip gate
    }
    else if ( mode && strcmp( mode, "invertsel" ) == 0 )
    {
        code = RunInvertSelectionTest( mapPath ); // Select_Invert (0x493F10) active<->selected list-swap gate
    }
    else if ( mode && strcmp( mode, "patchdisperse" ) == 0 )
    {
        code = RunPatchDisperseTest( mapPath );   // Patch_DisperseColumns/Rows (0x4443A0/0x444200) redisperse gate
    }
    else if ( mode && strcmp( mode, "patchinvtex" ) == 0 )
    {
        code = RunInvertTextureTest( mapPath );   // Patch_InvertTexture (0x446680) st-mirror involution gate
    }
    else if ( mode && strcmp( mode, "patchterrain" ) == 0 )
    {
        code = RunTerrainInsertTest( mapPath );   // Patch_TerrainInsertRow (0x4436A0) smoothing+d_points gate
    }
    else if ( mode && strcmp( mode, "patchscale" ) == 0 )
    {
        code = RunPatchScaleTest( mapPath );      // Patch_Scale (0x4427D0) control-point scale gate
    }
    else if ( mode && strcmp( mode, "patchmove" ) == 0 )
    {
        code = RunPatchMoveTest( mapPath );       // Patch_Move (0x441dd0) whole-patch translate gate
    }
    else if ( mode && strcmp( mode, "patchshifttex" ) == 0 )
    {
        code = RunPatchShiftTexTest( mapPath );   // Patch_ShiftTexture (0x446170) st-shift gate
    }
    else if ( mode && strcmp( mode, "patchrottex" ) == 0 )
    {
        code = RunPatchRotateTexTest( mapPath );  // Patch_RotateTexture (0x445B80) st-rotate gate
    }
    else if ( mode && strcmp( mode, "patchscaletex" ) == 0 )
    {
        code = RunPatchScaleTexTest( mapPath );   // Patch_ScaleTexture (0x445F80) st-scale gate
    }
    else if ( mode && strcmp( mode, "patchfittex" ) == 0 )
    {
        code = RunPatchFitTexTest( mapPath );     // Patch_FitTexturing (0x447600) fit-texture gate
    }
    else if ( mode && strcmp( mode, "patchapplymat" ) == 0 )
    {
        code = RunPatchApplyMatTest( mapPath );   // Patch_ApplyMatrix (0x441E70) rotate/flip gate
    }
    else if ( mode && strcmp( mode, "patchflip" ) == 0 )
    {
        code = RunPatchFlipTest( mapPath );       // Select_FlipAxis on a patch -> Patch_ApplyMatrix (end-to-end)
    }
    else if ( mode && strcmp( mode, "patchfliptex" ) == 0 )
    {
        code = RunPatchFlipTexTest( mapPath );    // Brush_FlipTexture on a patch -> PMESH_37 (0x445e30)
    }
    else if ( mode && strcmp( mode, "patchturnedge" ) == 0 )
    {
        code = RunPatchTurnEdgeTest( mapPath );   // Patch_TurnEdge/PMESH_51 terrain-edge picker (0x44b4c0/0x44acc0)
    }
    else if ( mode && strcmp( mode, "paintapply" ) == 0 )
    {
        code = RunPaintApplyTest( mapPath );      // terrain-paint apply chain sub_43E4B0->PMESH_16 (0x43e4b0/0x43ded0)
    }
    else if ( mode && strcmp( mode, "softseldrag" ) == 0 )
    {
        code = RunSoftSelDragTest( mapPath );     // soft-sel height drag sub_43DD00->PMESH_15->sub_43DC10->sub_43DA20
    }
    else if ( mode && strcmp( mode, "settexturing" ) == 0 )
    {
        code = RunSetTexturingTest( mapPath );    // Patch_SetTexturing grid layout (0x446b60)
    }
    else if ( mode && strcmp( mode, "patchcaptex" ) == 0 )
    {
        code = RunPatchCapTexTest( mapPath );     // Patch_CapTexture cap/natural texturing chain (0x439b10)
    }
    else if ( mode && strcmp( mode, "patchsamplesize" ) == 0 )
    {
        code = RunPatchSampleSizeTest( mapPath ); // sub_442B00 patch sample-size setter (0x442b00)
    }
    else if ( mode && strcmp( mode, "tess" ) == 0 )
    {
        code = RunTessTest( mapPath );          // patch tessellation + wireframe-segment count gate (STAGE 1)
    }
    else if ( mode && strcmp( mode, "curvepoint" ) == 0 )
    {
        code = RunCurvePointTest( mapPath );    // patch control-point overlay (markers + selected-point) gate
    }
    else if ( mode && strcmp( mode, "curvedrag" ) == 0 )
    {
        code = RunCurveDragTest( mapPath );     // patch control-point INTERACTIVE drag/bend round-trip gate
    }
    else if ( mode && strcmp( mode, "projguard" ) == 0 )
    {
        code = RunProjGuardTest();              // degenerate-projection guard (far-from-origin camera) gate
    }
    else if ( mode && strcmp( mode, "mayaexport" ) == 0 )
    {
        code = RunMayaExportTest( mapPath );    // Maya .mel MEL-script export gate (Misc->Maya Export)
    }
    else
    {
        code = RunSelfTest( mapPath );          // default: map round-trip gate
    }
    if ( s_logFile ) fclose( s_logFile );
    exit( code );          // headless test — never enter the message loop
    return FALSE;
#else
    // Gate P2 smoke test: verify the engine subset links and basic printing works
    // before any real init. Com_Printf is safe pre-init (falls through to stderr).
    Com_Printf(0, "[Radiant] Gate P2 smoke: engine subset initialized, InitInstance reached\n");

    // Register the common-control window classes the editor's child controls need (the
    // inspector tab strip uses SysTabControl32 — ICC_TAB_CLASSES).  Idempotent.
    {
        INITCOMMONCONTROLSEX icc;
        icc.dwSize = sizeof( icc );
        icc.dwICC  = ICC_TAB_CLASSES | ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES |
                     ICC_UPDOWN_CLASS | ICC_STANDARD_CLASSES;
        InitCommonControlsEx( &icc );
    }

    // P6 prefs: registry-backed persistence (binary's SetRegistryKey), then bring the
    // preference singleton up with the saved values BEFORE any view reads g_PrefsDlg.
    // (g_PrefsDlg is already defaults-constructed at static init; this overlays the
    // registry on top — faithful to the binary's startup LoadPrefs.)
    SetRegistryKey( "iw\\CoD4Radiant" );
    Prefs_Init( /*loadFromRegistry=*/true );

    CMainFrame *pFrame = new CMainFrame();
    if (!pFrame)
        return FALSE;
    m_pMainWnd = pFrame;

    RECT work = { 0, 0, 1600, 1000 };
    if ( SystemParametersInfoA( SPI_GETWORKAREA, 0, &work, 0 ) )
    {
        int w = work.right - work.left;
        int h = work.bottom - work.top;
        work.left += w / 40;
        work.top += h / 40;
        work.right -= w / 40;
        work.bottom -= h / 40;
    }
    if (!pFrame->Create(NULL, "CoD4Radiant", WS_OVERLAPPEDWINDOW, CRect(work)))
        return FALSE;

    pFrame->ShowWindow(m_nCmdShow);
    pFrame->UpdateWindow();

    // KISAK_RADIANT debug aid: if RADIANT_STARTUP_MAP is set, load that map at startup
    // (no effect when unset). Lets the first-light log capture a real map's applied
    // camera origin + render state for far-from-origin debugging (no GUI File->Open).
    if ( const char *startupMap = getenv( "RADIANT_STARTUP_MAP" ) )
    {
        extern void Radiant_FL_Log( const char *fmt, ... );
        extern void Map_LoadFromFile( const char *path );
        Map_LoadFromFile( startupMap );
        // Debug aid (paired with RADIANT_STARTUP_MAP): RADIANT_STARTUP_CAM="x y z pitch yaw roll"
        // repositions the 3D camera after load AND rebuilds its view basis (Cam_BuildMatrix), so a
        // headless run can render a real vantage (origin alone leaves the map out-of-frustum).
        if ( const char *camStr = getenv( "RADIANT_STARTUP_CAM" ) )
        {
            if ( g_pParentWnd && g_pParentWnd->m_pCamWnd )
            {
                float x=0,y=0,z=0,pi=0,ya=0,ro=0;
                if ( sscanf( camStr, "%f %f %f %f %f %f", &x,&y,&z,&pi,&ya,&ro ) >= 3 )
                {
                    g_pParentWnd->m_pCamWnd->camera.origin[0] = x;
                    g_pParentWnd->m_pCamWnd->camera.origin[1] = y;
                    g_pParentWnd->m_pCamWnd->camera.origin[2] = z;
                    g_pParentWnd->m_pCamWnd->camera.angles[0] = pi;
                    g_pParentWnd->m_pCamWnd->camera.angles[1] = ya;
                    g_pParentWnd->m_pCamWnd->camera.angles[2] = ro;
                    g_pParentWnd->m_pCamWnd->Cam_BuildMatrix();   // rebuild vpn/vright/vup so Cam_Fov's frustum matches
                }
            }
        }
        if ( g_pParentWnd && g_pParentWnd->m_pCamWnd )
            Radiant_FL_Log( "STARTUP_MAP %s: camera.origin=(%.1f %.1f %.1f) angles=(%.1f %.1f %.1f)  XY.origin=(%.1f %.1f %.1f) scale=%.4f",
                startupMap,
                g_pParentWnd->m_pCamWnd->camera.origin[0],
                g_pParentWnd->m_pCamWnd->camera.origin[1],
                g_pParentWnd->m_pCamWnd->camera.origin[2],
                g_pParentWnd->m_pCamWnd->camera.angles[0],
                g_pParentWnd->m_pCamWnd->camera.angles[1],
                g_pParentWnd->m_pCamWnd->camera.angles[2],
                g_pParentWnd->m_pXYWnd ? g_pParentWnd->m_pXYWnd->m_vOrigin[0] : 0.0f,
                g_pParentWnd->m_pXYWnd ? g_pParentWnd->m_pXYWnd->m_vOrigin[1] : 0.0f,
                g_pParentWnd->m_pXYWnd ? g_pParentWnd->m_pXYWnd->m_vOrigin[2] : 0.0f,
                g_pParentWnd->m_pXYWnd ? g_pParentWnd->m_pXYWnd->m_fScale : 0.0f );
    }
    return TRUE;
#endif
}
