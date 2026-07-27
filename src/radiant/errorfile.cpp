#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Map-compiler error-log parser and navigator.

#include "stdafx.h"
#include "mainfrm.h"
#include "qe3.h"
#include <universal/q_parse.h>   // parseInfo_t / Com_Parse / Com_BeginParseSession
#include <cstdio>
#include <cstdlib>
#include <cstring>

// ── extern declarations for globals/functions from other .cpp files ──────────
extern void  Assert( const char *file, int line, int type, const char *fmt, ... );
extern int   Sys_Printf( const char *fmt, ... );
extern char  currentmap[];            // map.cpp 0x23F18D8
extern int   g_nUpdateBits;          // engine_stubs.cpp 0x25D5A74
extern CMainFrame *g_pParentWnd;     // engine_stubs.cpp
extern void  Select_Deselect( int bits ); // select.cpp 0x48E800

// ── forward declarations ──────────────────────────────────────────────────────
// Functions implemented in other ported files.
// HasUnsavedChangesOrInsidePrefab (0x489D90) is ported below; not extern.
extern unsigned int CheckLayeredMaterial_Modifications( uint8_t *a1, int a2, int a3 ); // layeredmaterials.cpp
extern void Prefab_LevelBack();
extern void Map_LoadFromFile( const char *filename );
extern void I_strncpyz( char *dest, const char *src, int destsize ); // q_shared.cpp (0x4B9290)
extern void Select_ByEntityNumber( int brushIdx, int entIdx );  // win_dlg.cpp (0x495550)
extern void Material_SetMode( int lightmapMode );
extern void vectoangles( float *angles, int vec );

// ── error-log loader deps (Pointfile_Errorfile parse path) ───────────────────
// Com_Parse / Com_BeginParseSession come from <universal/q_parse.h> (included above).
extern int    LoadFile( const char *filename, void **bufferptr );   // cmdlib.cpp (0x40ABD0)
extern void   StripExtension( char *path );                         // cmdlib.cpp (0x40AE90)

// g_qeglobals_d_pointfile_display_list — defined in points.cpp (this is s_errLogCount).
extern int g_qeglobals_d_pointfile_display_list;

// zero — empty string sentinel defined in engine_stubs.cpp.
extern void *zero;

// ── s_errLogEntry_t — 40-byte error-log record ───────────────────────────────
// Layout verified against IDA's field access pattern in ErrorLog_01 and
// Pointfile_Clear (points.cpp). Stride = 40 bytes.
struct s_errLogEntry_t
{
    char  *mapfile;      // +0   heap string (char*)
    char  *matname;      // +4   heap string (char*)
    int    enum1;        // +8   entity number 1 (brush or face)
    int    enum2;        // +12  entity number 2
    float  origin[3];    // +16  camera target position
    float  dir[3];       // +28  view direction (3 floats, total 12B → +28..+39)
};
// IDB base 0x180ACE8; array sits in .bss.  We declare it as an extern large enough
// for the error-log maximum.  The actual capacity is bounded by the compiler output;
// 1024 entries is a safe overestimate for the .bss reservation.
// NOTE: the IDB does not define the array as a named struct; it names each field
// individually (dword_180ACE8 = entry[0].mapfile, etc.).
// errorfile.cpp defines this array; points.cpp uses it via extern.
s_errLogEntry_t s_errLog[1024]; // IDB block starting at 0x180ACE8

// s_errLogIndex — which entry is currently selected / being navigated.
// IDB: dword_180A8E4 @ 0x180A8E4.
static int s_errLogIndex = 0;

// s_errLogMapPath — the .map path the loaded error log belongs to.  IDB `path` @ 0x180A8E8
// (a 1024-byte global).  Written by Pointfile_Errorfile (I_strncpyz(path, currentmap, 1024))
// and read by the error-log navigators (sub_410640/670/710) to decide whether the user has
// since navigated to a different map and the original must be reloaded.  Previously omitted
// (the §11 note below); restored now that the navigators are ported.
static char s_errLogMapPath[1024];

// ── lyrMtlGlob globals (IDB 0x1814CFC / 0x1814D00) ──────────────────────────
// Defined in layeredmaterials.cpp (ported 2026-06-11, unit 2).
// Forward-declared here for the CheckLayeredMaterial_Modifications call below.
extern int     lyrMtlGlob_entryCount;    // 0x1814CFC (layeredmaterials.cpp)
extern uint8_t lyrMtlGlob_Layers[];     // 0x1814D00 inline array (layeredmaterials.cpp)
extern int     dword_1814CF8;           // 0x1814CF8 (layeredmaterials.cpp)

// ── 0x489D90  HasUnsavedChangesOrInsidePrefab ─────────────────────────────────
// Returns true if the map has unsaved changes, or we are inside a prefab level
// with pending edits.
// Globals: modified (0x23F179C), byte_25EB240 (0x25EB240), prefabStackLevel (0x25D5B34).
// These are already defined in engine_stubs.cpp / map.cpp.
extern int  modified;              // 0x23F179C (map.cpp or engine_stubs.cpp)
extern char byte_25EB240[];        // 0x25EB240 (engine_stubs.cpp)
extern int  prefabStackLevel;      // 0x25D5B34 (map.cpp)

static bool HasUnsavedChangesOrInsidePrefab()
{
    if ( modified )
        return true;

    if ( prefabStackLevel > 0 )
    {
        // Walk the prefab stack; return true if any level is non-null (modified).
        char *p = byte_25EB240;
        for ( int v1 = 0; ; p += 2168 )
        {
            if ( *(int *)p )
                return true;
            if ( ++v1 >= prefabStackLevel )
                return false;
        }
    }
    return false;
}

// ── Q_stristr (sub_4B93C0) — case-insensitive substring search ───────────────
// Returns pointer to first occurrence of substr in str (case-insensitive), or NULL.
// Used to test whether the material name contains "lightmap".
static const char *Q_stristr( const char *str, const char *substr )
{
    if ( !str || !substr )
        return NULL;
    if ( !*str )
        return NULL;
    size_t sublen = strlen( substr );
    while ( *str )
    {
        if ( _strnicmp( str, substr, sublen ) == 0 )
            return str;
        ++str;
    }
    return NULL;
}

// ── 0x40FC80  ErrorLog_01 ─────────────────────────────────────────────────────
// Navigate to the currently-selected error-log entry.
// Steps:
//   1. Deselect everything.
//   2. Build the map file path by prepending the project "mapspath" if the entry
//      doesn't have an absolute path (colon test).
//   3. If the entry references a different map from currentmap: check for unsaved
//      changes, optionally save, then load the map.
//   4. Jump the camera to the error position and select the flagged entity.
//
// Return value: 1 on success, 0 on abort (user cancelled save dialog).
char ErrorLog_01()
{
    unsigned int v0;
    const char  *value;
    unsigned int pathlen;
    char         v4;
    char         path[1024];
    int          v7;

    Select_Deselect( 1 );

    // Does the entry's map path contain a drive letter or UNC prefix (':')?
    // IDA: strchr((&dword_180ACE8)[10 * dword_180A8E4], ':')
    // i.e. strchr(s_errLog[s_errLogIndex].mapfile, ':')
    if ( strchr( s_errLog[s_errLogIndex].mapfile, ':' ) )
    {
        // Absolute path — use directly as-is (v0 = 0, path built at offset 0).
        v0 = 0;
    }
    else
    {
        // Relative path — prepend "mapspath" from the project entity's epairs.
        value = (const char *)zero; // fallback = empty string (IDB: (const char *)zero)
        epair_t *ep = g_qeglobals.d_project_entity->epairs;
        if ( ep )
        {
            while ( _stricmp( ep->key, "mapspath" ) != 0 )
            {
                ep = ep->next;
                if ( !ep )
                    goto no_mapspath;
            }
            value = ep->value;
            if ( !value )
            {
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\q_shared.cpp",
                        554, 0, "%s", "src" );
            }
            goto have_value;
        }
no_mapspath:
        value = (const char *)zero;
have_value:
        strncpy( path, value, 0x3FFu );
        path[1023] = 0;
        pathlen = (unsigned int)strlen( path );
        v0 = pathlen;
        if ( !pathlen ||
             ( pathlen < 0x3FF
               && ( v4 = path[pathlen - 1], v4 != '\\' )
               && v4 != '/' ) )
        {
            path[pathlen]     = '\\';
            path[pathlen + 1] = 0;
            v0 = pathlen + 1;
        }
    }

    // Append the entry's map filename to path[v0..].
    // IDA: strcpy(&path[v0], s_errLog[s_errLogIndex].mapfile, 1024 - v0)
    // That's the 3-arg Radiant safe-strcpy (strncpy + null-terminate).
    {
        const char *src  = s_errLog[s_errLogIndex].mapfile;
        int         room = 1024 - (int)v0;
        if ( src && room >= 1 )
        {
            strncpy( path + v0, src, (size_t)(room - 1) );
            path[v0 + (size_t)(room - 1)] = 0;
        }
    }

    // Assert that we have at least one error-log entry.
    if ( !g_qeglobals_d_pointfile_display_list )
    {
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\errorfile.cpp",
                47, 0, "%s", "s_errLogCount" );
    }

    // If the current map already matches, jump directly to positioning.
    if ( _stricmp( path, currentmap ) == 0 )
        goto do_position;

    // Check for unsaved changes or layered-material modifications.
    if ( ( HasUnsavedChangesOrInsidePrefab()
           || CheckLayeredMaterial_Modifications( lyrMtlGlob_Layers,
                                                  84 * lyrMtlGlob_entryCount,
                                                  0 ) != dword_1814CF8 )
         && !( g_pParentWnd && g_pParentWnd->ConfirmModified() ) )
    {
        return 0;
    }

    Prefab_LevelBack();
    Map_LoadFromFile( path );

    if ( !g_qeglobals_d_pointfile_display_list )
        return 1;

do_position:
    {
        // Detect if the material is a lightmap material (contains "lightmap").
        // IDA: sub_4B93C0(*(_BYTE**)&dword_180ACEC[40*s_errLogIndex], "lightmap") != 0
        // sub_4B93C0 is Q_stristr.
        int lightmap = ( Q_stristr( s_errLog[s_errLogIndex].matname, "lightmap" ) != NULL ) ? 1 : 0;
        Material_SetMode( lightmap );

        Sys_Printf( "Error from log: %s\n", s_errLog[s_errLogIndex].matname );

        v7 = s_errLogIndex;
        int e1 = s_errLog[s_errLogIndex].enum1;
        if ( e1 >= 0 )
        {
            int e2 = s_errLog[s_errLogIndex].enum2;
            if ( e2 >= 0 )
            {
                Select_ByEntityNumber( e2, e1 );
                v7 = s_errLogIndex;
            }
        }

        // Position the camera at the error origin and orient it toward the
        // error direction. Phase 5 dep: g_pParentWnd->m_pCamWnd.
        CCamWnd *cam = g_pParentWnd ? g_pParentWnd->m_pCamWnd : NULL;
        if ( cam )
        {
            cam->camera.origin[0] = s_errLog[v7].origin[0];
            cam->camera.origin[1] = s_errLog[v7].origin[1];
            cam->camera.origin[2] = s_errLog[v7].origin[2];

            // vectoangles converts a direction vector into (pitch, yaw, 0).
            // IDA: vectoangles(m_pCamWnd->camera.angles, (int)&unk_180AD04 + 40*v7)
            vectoangles( cam->camera.angles, (int)(intptr_t)&s_errLog[v7].dir[0] );

            // Negate pitch (IDA: *angles = -v12 where v12 = *angles before g_nUpdateBits).
            float pitch = cam->camera.angles[0];
            g_nUpdateBits |= 0x104u;  // request camera + XY view updates
            cam->camera.angles[0] = -pitch;
        }
    }

    return 1;
}

// ── 0x40FF30  ErrorLog_AddEntry (sub_40FF30) ──────────────────────────────────
// Append one parsed error-log entry into s_errLog[], with de-duplication.  The IDA
// __usercall is sub_40FF30(dir@<ebx>, mapfile, enum1, enum2, origin, matname); each
// field maps to the s_errLogEntry_t layout above:
//   mapfile  (+0)   = the map-file reference (1st token; can be relative or absolute)
//   matname  (+4)   = the material name (last token)
//   enum1    (+8)   = entity number 1   (the ENTITY index for Select_ByEntityNumber)
//   enum2    (+12)  = entity number 2   (the BRUSH  index)
//   origin   (+16)  = camera target position (vec3)
//   dir      (+28)  = view direction (vec3)
// De-dup: if an existing entry already matches (mapfile, enum1, enum2, matname) the new
// one is dropped (the binary's leading scan loop).  Capacity is hard-capped at 1024.
// The mapfile/matname strings are _strdup'd (freed by Pointfile_Clear).
static void ErrorLog_AddEntry( const float *dir, const char *mapfile,
                               int enum1, int enum2, const float *origin,
                               const char *matname )
{
    int count = g_qeglobals_d_pointfile_display_list;   // == s_errLogCount

    // De-dup scan over the existing entries (IDA: the while-loop on dword_180ACF4).
    for ( int i = 0; i < count; ++i )
    {
        if ( enum1 == s_errLog[i].enum1 &&
             enum2 == s_errLog[i].enum2 &&
             strcmp( mapfile, s_errLog[i].mapfile ) == 0 &&
             strcmp( matname, s_errLog[i].matname ) == 0 )
            return;                                      // duplicate — drop it
    }

    if ( count == 1024 )                                 // capacity wall (IDA: != 1024)
        return;

    s_errLog[count].mapfile = _strdup( mapfile );
    s_errLog[count].enum1   = enum1;
    s_errLog[count].enum2   = enum2;
    s_errLog[count].matname = _strdup( matname );
    s_errLog[count].origin[0] = origin[0];
    s_errLog[count].origin[1] = origin[1];
    s_errLog[count].origin[2] = origin[2];
    s_errLog[count].dir[0]    = dir[0];
    s_errLog[count].dir[1]    = dir[1];
    s_errLog[count].dir[2]    = dir[2];

    ++g_qeglobals_d_pointfile_display_list;
}

// ── 0x410070  ErrorLog_Compare (sub_410070) — qsort comparator ────────────────
// Sort error-log entries by material name, then by map-file reference (both
// case-insensitive).  IDA: I_stricmp(a1[1], a2[1]) then I_stricmp(*a1, *a2) — a1[1]
// is the matname (+4), *a1 is the mapfile (+0).
static int __cdecl ErrorLog_Compare( const void *a, const void *b )
{
    const s_errLogEntry_t *ea = (const s_errLogEntry_t *)a;
    const s_errLogEntry_t *eb = (const s_errLogEntry_t *)b;
    int r = _stricmp( ea->matname, eb->matname );
    if ( r != 0 )
        return r;
    return _stricmp( ea->mapfile, eb->mapfile );
}

// ── 0x4100B0  Pointfile_Errorfile ─────────────────────────────────────────────
// Load the error-log file produced by the CoD map compiler for the current map.  The
// file is "<currentmap-with-extension-stripped>.errlog" (verified: the appended literal
// at 0x6D6D64 is the byte sequence ".errlog\0").  Each line is:
//     <mapfile> <enum1> <enum2> <ox> <oy> <oz> <dx> <dy> <dz> <matname>
// parsed token-by-token through the engine parser (Com_Parse — the binary inlines the
// unget-prologue + Com_ParseExt(.,1), the established radiant idiom; see filters.cpp).
// Each parsed line is appended via ErrorLog_AddEntry (de-duplicated).  When done, the
// entries are sorted (ErrorLog_Compare) and ErrorLog_01() navigates to the first one.
// If the file is missing, a MessageBox is shown (matching the binary).
//
// §11 — the binary does I_strncpyz(path, currentmap, 1024) to a bookkeeping global
// (0x180A8E8) right before Com_BeginParseSession; it is read by the error-log navigators
// (Errorfile_NextError/PrevError) to detect that the user navigated to a different map and
// reload the original.  Now that those navigators are ported, the write is RESTORED.
char Pointfile_Errorfile()
{
    char filename[1024];
    strncpy( filename, currentmap, 0x3FFu );
    filename[1023] = 0;
    StripExtension( filename );

    // Append ".errlog" iff it fits (IDA: strlen + 7 < 0x400).
    size_t len = strlen( filename );
    if ( len + 7 >= 0x400 )
        return (char)( len + 7 );          // (matches the IDA early-out return value)
    strcpy( filename + len, ".errlog" );   // 7 chars + NUL

    void *buffer = nullptr;
    if ( LoadFile( filename, &buffer ) < 0 )
    {
        MessageBoxA( GetActiveWindow(),
                     "Error log file was not found.\n\nMake sure it is in the same folder as the map source.\n",
                     "Radiant", MB_ICONEXCLAMATION );
        return 0;
    }

    I_strncpyz( s_errLogMapPath, currentmap, 0x400 );   // IDB: I_strncpyz(path, currentmap, 1024)
    Com_BeginParseSession( filename );
    const char *text = (const char *)buffer;
    Sys_Printf( "Reading error log %s\n", filename );

    // Parse one error record per loop iteration: mapfile enum1 enum2 origin[3] dir[3] matname.
    for ( ;; )
    {
        parseInfo_t *t = Com_Parse( &text );
        if ( !text )
            break;                                   // EOF (IDA: !v53)
        if ( !t )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\q_shared.cpp",
                    554, 0, "%s", "src" );

        char mapfile[1024];
        strncpy( mapfile, t->token, 0x3FFu );
        mapfile[1023] = 0;

        int   enum1 = atol( Com_Parse( &text )->token );
        int   enum2 = atol( Com_Parse( &text )->token );
        float origin[3];
        origin[0] = (float)atof( Com_Parse( &text )->token );
        origin[1] = (float)atof( Com_Parse( &text )->token );
        origin[2] = (float)atof( Com_Parse( &text )->token );
        float dir[3];
        dir[0] = (float)atof( Com_Parse( &text )->token );
        dir[1] = (float)atof( Com_Parse( &text )->token );
        dir[2] = (float)atof( Com_Parse( &text )->token );

        parseInfo_t *mt = Com_Parse( &text );
        if ( !mt )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\q_shared.cpp",
                    554, 0, "%s", "src" );

        char matname[1024];
        strncpy( matname, mt->token, 0x3FFu );
        matname[1023] = 0;

        ErrorLog_AddEntry( dir, mapfile, enum1, enum2, origin, matname );
    }

    free( buffer );
    Sys_Printf( "%i error(s) loaded\n", g_qeglobals_d_pointfile_display_list );

    s_errLogIndex = 0;
    if ( g_qeglobals_d_pointfile_display_list )
    {
        qsort( &s_errLog[0], (size_t)g_qeglobals_d_pointfile_display_list,
               sizeof( s_errLogEntry_t ), ErrorLog_Compare );
        return ErrorLog_01();
    }
    return (char)g_qeglobals_d_pointfile_display_list;
}

// ── menu-handler entry points (CMainFrame::OnErrorFile, mainfrm.cpp) ───────────
// Exposed so the File→Error file toggle handler (0x423B40) can drive the load/clear.
void Pointfile_Errorfile_Public() { Pointfile_Errorfile(); }
// (Pointfile_Clear lives in points.cpp; the handler calls it directly via extern.)

// ── error-log navigation (Misc→Next/Previous leak spot, error-log branch) ─────
// 0x410640 — if the user navigated to a different map since the error log loaded, reload it.
static void Errorfile_GotoMap()
{
    if ( _stricmp( currentmap, s_errLogMapPath ) )
    {
        Select_Deselect( 1 );
        Map_LoadFromFile( s_errLogMapPath );
    }
}

// 0x410670 — advance to the next error-log entry; wrap (with a prompt) at the end.
void Errorfile_NextError()
{
    if ( ++s_errLogIndex == g_qeglobals_d_pointfile_display_list )
    {
        if ( MessageBoxA( GetActiveWindow(),
                          "End of error list reached.\n\nContinue from start?",
                          "Radiant", MB_YESNO | MB_ICONQUESTION ) == IDNO )
        {
            s_errLogIndex = -1;
            Errorfile_GotoMap();
            return;
        }
        s_errLogIndex = 0;
    }
    if ( !ErrorLog_01() )
    {
        int v1 = s_errLogIndex;
        if ( !s_errLogIndex )
            v1 = g_qeglobals_d_pointfile_display_list;
        s_errLogIndex = v1 - 1;
    }
}

// 0x410710 — go to the previous error-log entry; wrap (with a prompt) at the start.
void Errorfile_PrevError()
{
    int v0 = s_errLogIndex;
    if ( s_errLogIndex <= 0 )
    {
        if ( !s_errLogIndex )
        {
            if ( MessageBoxA( GetActiveWindow(),
                              "Start of error list reached.\n\nContinue from end?",
                              "Radiant", MB_YESNO | MB_ICONQUESTION ) == IDNO )
            {
                s_errLogIndex = -1;
                Errorfile_GotoMap();
                return;
            }
        }
        v0 = g_qeglobals_d_pointfile_display_list;
    }
    s_errLogIndex = v0 - 1;
    if ( !ErrorLog_01() && ++s_errLogIndex == g_qeglobals_d_pointfile_display_list )
        s_errLogIndex = 0;
}

#ifdef RADIANT_SELFTEST
// ══════════════════════════════════════════════════════════════════════════════
//  Deterministic ERROR-LOG gate (the `errorlog` CLI command, RunErrorLogTest).  Two
//  halves, both with no message pump:
//   (A) DATA + SELECT path: pick a world brush, read its (ent#, brush#) via
//       GetSelectionIndex, deselect, synthesize an error-log entry that points at THAT
//       brush (mapfile == currentmap, absolute → ErrorLog_01 skips the map reload), then
//       ErrorLog_01() must re-select exactly the picked brush.  This is the brief's
//       "clicking an entry selects the right brush" check, driven through the REAL
//       ErrorLog_AddEntry + ErrorLog_01 + Select_ByEntityNumber chain.
//   (B) PARSE path: write a synthetic "<temp>.errlog" with two records (the real
//       compiler line format), point Pointfile_Errorfile at it (currentmap = the temp map
//       stem), and assert it parses + de-dups + counts correctly.
//  Both share the s_errLog[] store; (B) runs first into a scratch count, is cleared, then
//  (A) runs the select check (so the picked-brush pointers stay valid).
// ══════════════════════════════════════════════════════════════════════════════
extern void GetSelectionIndex( int *entOut, int *brushOut );        // win_dlg.cpp (0x495700)
extern void Select_Brush( selbrush_t *b, char overwrite, char status, char center ); // select.cpp
extern void Pointfile_Clear();                                      // points.cpp (0x410600)

// out[0] = #errlog entries the PARSE half loaded (expect 2 — the dup is dropped)
// out[1] = picked entity#   (GetSelectionIndex)
// out[2] = picked brush#
// out[3] = 1 iff ErrorLog_01 re-selected exactly the SAME brush (the select path is faithful)
// Returns 0 on success, -1 = N/A (no editable inline world brush — pure-prefab map).
extern "C" int Radiant_TestErrorLog( int *out /* int[4] */ )
{
    out[0] = out[1] = out[2] = out[3] = 0;

    // ── (B) PARSE half: synthesize a .errlog and load it ──────────────────────
    // Reset the store, then write 3 lines (2 unique + 1 exact dup of line 1).
    Pointfile_Clear();
    char tmpDir[MAX_PATH], errPath[MAX_PATH], mapStem[MAX_PATH];
    GetTempPathA( sizeof( tmpDir ), tmpDir );
    _snprintf( mapStem, sizeof( mapStem ), "%sradiant_errlog_probe.map", tmpDir );
    _snprintf( errPath, sizeof( errPath ), "%sradiant_errlog_probe.errlog", tmpDir );
    {
        FILE *ef = fopen( errPath, "w" );
        if ( ef )
        {
            // <mapfile> <ent#> <brush#> <ox oy oz> <dx dy dz> <matname>
            fprintf( ef, "%s 1 2 10 20 30 0 0 1 caulk\n", mapStem );
            fprintf( ef, "%s 3 4 11 21 31 0 1 0 wc_metal\n", mapStem );
            fprintf( ef, "%s 1 2 10 20 30 0 0 1 caulk\n", mapStem ); // exact dup → dropped
            fclose( ef );
        }
    }
    // Point currentmap at the temp stem so Pointfile_Errorfile derives "<stem>.errlog".
    // (The select inside ErrorLog_01 will fail benignly — ent#1/brush#2 don't exist in the
    //  current map — but the parse + de-dup + count is what (B) verifies.)
    char savedCurrent[1024];
    strncpy( savedCurrent, currentmap, sizeof( savedCurrent ) - 1 );
    savedCurrent[sizeof( savedCurrent ) - 1] = 0;
    strncpy( currentmap, mapStem, 1023 );
    currentmap[1023] = 0;
    Pointfile_Errorfile();
    out[0] = g_qeglobals_d_pointfile_display_list;     // expect 2 (dup dropped)
    // Restore currentmap and clear the scratch entries.
    strcpy( currentmap, savedCurrent );
    Pointfile_Clear();
    s_errLogIndex = 0;
    remove( errPath );

    // ── (A) DATA + SELECT half ────────────────────────────────────────────────
    // Pick an editable, non-patch world brush (same filter as the findbrush gate).
    selbrush_t *pick = nullptr;
    for ( selbrush_t *b = active_brushes.next; b && b != &active_brushes; b = b->next )
    {
        if ( b->patch || !b->def || b->def->faceCount <= 0 )
            continue;
        if ( *(int *)&( (entity_s_def *)b->owner->def )->eclass->fixedsize )
            continue;
        pick = b;
        break;
    }
    if ( !pick )
        return -1;       // N/A — but (B) already ran; the dispatcher logs out[0]

    Select_Deselect( 1 );
    Select_Brush( pick, 1, 0, 0 );
    brush_t *pickDef = pick->def;

    int entIdx = 0, brushIdx = 0;
    GetSelectionIndex( &entIdx, &brushIdx );
    out[1] = entIdx;
    out[2] = brushIdx;

    Select_Deselect( 1 );

    // Synthesize ONE entry pointing at the picked brush.  mapfile == currentmap (absolute,
    // contains ':') so ErrorLog_01 takes the no-reload path straight to the select.
    // enum1 = ent#, enum2 = brush# (ErrorLog_01 calls Select_ByEntityNumber(enum2, enum1)).
    float origin[3] = { (pickDef->mins[0] + pickDef->maxs[0]) * 0.5f,
                        (pickDef->mins[1] + pickDef->maxs[1]) * 0.5f,
                        (pickDef->mins[2] + pickDef->maxs[2]) * 0.5f };
    float dir[3]    = { 1.0f, 0.0f, 0.0f };
    ErrorLog_AddEntry( dir, currentmap, entIdx, brushIdx, origin, "caulk" );
    s_errLogIndex = 0;

    ErrorLog_01();

    // Did ErrorLog_01 select exactly the picked brush?
    int n = 0;
    selbrush_t *sel = selected_brushes.next;
    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        ++n;
    out[3] = ( n == 1 && sel->def == pickDef ) ? 1 : 0;

    Pointfile_Clear();
    s_errLogIndex = 0;
    return 0;
}
#endif // RADIANT_SELFTEST
