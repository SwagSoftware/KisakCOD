#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\qe3.cpp

#include "stdafx.h"
#include "qe3.h"   // Phase 3: editor object model + permanent layout static_asserts
#include "prefs.h" // g_PrefsDlg / prefData_t (m_strLastProject, m_bLoadLast) — QE_LoadProject
#include "mainfrm.h" // CMainFrame (g_pParentWnd->OkToDiscard) — DoMru
#include <universal/q_parse.h>  // Com_BeginParseSession/EndParseSession/SetCSV/ParseExt/
                                // SkipRestOfLine + parseInfo_t (Get_MaterialNames)
#include <cstdlib>              // malloc / free / atol
#include <cstring>             // memcpy / strlen / strcat
#include <cstdio>               // sprintf
#include <string.h>             // _strlwr / _stricmp

// ── engine deps used by Get_MaterialNames / Script_Link (T2.6) ───────────────
extern void  Assert( const char *file, int line, int type, const char *fmt, ... );  // 0x49CEA0
extern int   LoadFile( const char *filename, void **bufferptr );                     // 0x40ABD0
extern void  Com_PrintMessage( const char *fmt, ... );                               // 0x40A960
extern int   Sys_Printf( const char *fmt, ... );                                     // 0x499E90

// Global editor state — 466144-byte struct, zero-initialized (BSS).
// Defined here per plan §4 (home file = qe3.cpp). Declared extern in qe3.h.
qeglobals_t g_qeglobals;

// ─────────────────────────────────────────────────────────────────────────────
// 0x48b030  AllocMaterialString - heap-duplicate a C string (callers free() it).
// IDB: Assert(str); n=strlen(str)+1; p=operator new(n); memcpy.  malloc is ABI-equivalent
// here (same CRT heap); a null input returns null instead of asserting.
// ─────────────────────────────────────────────────────────────────────────────
char *AllocMaterialString( const char *src )
{
    iassert( src );   // qe3.cpp:56 (the binary's "str" head-check)
    if ( !src ) return nullptr;   // defensive continue after the warn (binary would crash)
    size_t n = strlen( src ) + 1;
    char  *p = (char *)malloc( n );
    if ( p ) memcpy( p, src, n );
    return p;
}

// 0x411280 qe3.cpp_01 (the `face`-type material-name list parser) lives in filters.cpp
// next to its only caller RadiantFilters.



// ─────────────────────────────────────────────────────────────────────────────
// 0x45b010  TexFilter_LoadMenuFile( const char *txt@ecx, void *dest, int startId )
// Read a text file line-by-line (fgets, 1024 cap), trim leading+trailing whitespace, skip
// '//' and blank lines, and fill the menu-string table `dest` (filter_material_t {char*
// name; int index}):  the index counter starts at 1 (entry [0] is the static {"all", 0});
// a normal line becomes { strdup(line), id } and consumes an id; "<separator>" becomes
// { 0, -1 } and does NOT; 256 strings is a fatal cap.  Returns the next free index (== the
// entry count incl. [0]).  Caller FillTextureMenu.  Strings are malloc'd so
// CTexWnd_Shutdown's free() pairs (the binary's static-CRT operator new is malloc).
// ─────────────────────────────────────────────────────────────────────────────
struct RadiantFilterEntry { char *name; int index; };   // = IDB filter_material_t (8B)
static_assert( sizeof( RadiantFilterEntry ) == 8, "filter_material_t must be 8 bytes (IDB)" );

int TexFilter_LoadMenuFile( const char *txt, void *dest, int startId )
{
    RadiantFilterEntry *entries = (RadiantFilterEntry *)dest;
    int   id   = startId;                                // a2
    int   i    = 1;                                      // v16 — entries start at index 1
    char  line[1024];                                   // v17 (1023 + NUL) — fgets buffer
    char  overflow = 0;                                  // v18 — line-too-long sentinel

    FILE *file = fopen( txt, "rb" );                     // 0x45b04d
    if ( !file )                                         // 0x45b05a
        Com_PrintMessage( "missing file '%s'\n", txt );  // 0x45b062 (falls through; fgets on NULL returns NULL)

    if ( !fgets( line, 1024, file ) )                   // 0x45b07b — empty/unreadable: nothing to read
        return i;                                       // 0x45b087 (returns 1)

    while ( 1 )
    {
        if ( overflow )                                 // 0x45b0aa — the prior iteration overran the buffer
        {
            fclose( file );                             // 0x45b0ad
            Com_PrintMessage( "line longer than %i characters in '%s'\n", 4, txt ); // 0x45b0bd
        }

        // Trim leading whitespace → v6 (here `p`).                       0x45b0c5..0x45b0f2
        char *p = line;
        if ( isspace( (unsigned char)line[0] ) )
        {
            do { ++p; } while ( isspace( (unsigned char)*p ) );
        }

        if ( strncmp( p, "//", 2 ) )                    // 0x45b0fc — not a comment line
        {
            unsigned int len = (unsigned int)strlen( p ); // 0x45b10e (esi)
            if ( len )                                  // 0x45b11c — skip empty lines
            {
                // Trim trailing whitespace.                              0x45b122..0x45b143
                char *end = &p[len - 1];
                while ( isspace( (unsigned char)*end ) )
                {
                    --len;
                    --end;
                    if ( !len )                         // 0x45b13e — line was all whitespace
                        goto next_line;
                }
                p[len] = 0;                             // 0x45b14d

                if ( i == 256 )                         // 0x45b15d — 256-string cap
                {
                    fclose( file );                     // 0x45b166
                    Com_PrintMessage( "%i string limit exceeded in '%s'\n", 256, txt ); // 0x45b17b
                }

                if ( !_stricmp( p, "<separator>" ) )    // 0x45b189
                {
                    entries[i].name  = 0;               // 0x45b19b
                    entries[i].index = -1;              // 0x45b1a2
                }
                else
                {
                    iassert( p );                       // 0x45b1ae dead-branch Assert("str") — p already used
                    unsigned int n = (unsigned int)strlen( p ) + 1; // 0x45b1cc..0x45b1db
                    char *copy = (char *)malloc( n );   // 0x45b1df operator new(n) (== malloc in static CRT)
                    memcpy( copy, p, n );               // 0x45b1ec
                    entries[i].index = id;              // 0x45b203
                    entries[i].name  = copy;            // 0x45b20a
                    ++id;                               // 0x45b207
                }
                ++i;                                    // 0x45b212/0x45b215 (v16 = v10 + 1)
            }
        }

next_line:
        if ( !fgets( line, 1024, file ) )               // 0x45b22e
            return i;                                   // 0x45b241 — EOF: return the count
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x48c1f0  ScriptGroup_LinkTo - if entity `e` contains its OWN script_linkName id in its
// script_linkTo list, strip that self-reference, write the cleaned list back and return 1;
// otherwise return 0.  Script_Link uses it to detect "both already in the same group".
// ─────────────────────────────────────────────────────────────────────────────
extern char       *ValueForKey2( int e, const char *key );                            // entity.cpp 0x4825C0
extern void        SetKeyValue( entity_s_def *e, const char *key, const char *value ); // entity.cpp 0x483690
extern char       *va( const char *fmt, ... );                                         // q_shared
extern void       *zero;                                                                // empty-string sentinel (engine_stubs.cpp)
extern entity_s    entities;                                                            // entity.cpp 0x23F17A0

static char ScriptGroup_LinkTo( entity_s_def *e )
{
    // 0x48c20d: find e's "script_linkTo" epair; absent/empty → not a group, return 0.
    const char *linkTo = nullptr;
    for ( epair_t *ep = e->epairs; ep; ep = ep->next )
        if ( !_stricmp( ep->key, "script_linkTo" ) ) { linkTo = ep->value; break; }
    if ( !linkTo || !*linkTo )
        return 0;

    // 0x48c261: e's own link id.
    char *linkName = ValueForKey2( (int)e, "script_linkName" );
    if ( !linkName || !*linkName )
        return 0;
    int myId = atol( linkName );

    LinkList_t links;                          // Map_ParseLinkList parse buffer (qe3.h)
    Map_ParseLinkList( &links, linkTo );       // 0x48c281
    int count = links.size;
    if ( count <= 0 )
        return 0;

    // 0x48c29c: is myId present?  Not present → return 0 (no self-link to strip).
    int idx = 0;
    while ( links.id[idx] != myId )
        if ( ++idx >= count )
            return 0;

    // 0x48c2b8: rebuild the list with myId removed.
    int filtered[1024];
    int n = 0;
    for ( int i = 0; i < count; ++i )
        if ( links.id[i] != myId )
            filtered[n++] = links.id[i];

    char buf[1028];
    if ( n )                                   // 0x48c325: join remaining ids "%i" + " %i"...
    {
        sprintf( buf, "%i", filtered[0] );
        for ( int i = 1; i < n; ++i )
            strcat( buf, va( " %i", filtered[i] ) );
        SetKeyValue( e, "script_linkTo", buf );
    }
    else                                       // 0x48c2f7: nothing left → clear to empty string
    {
        SetKeyValue( e, "script_linkTo", (const char *)zero );
    }
    return 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x48bf10  Script_Link - __fastcall(entity_s_def *a@ecx, entity_s_def *b@edx).  Link `a`
// to `b` for the scripting "link" feature (Selection->Link Selected, cmd 33211, whose
// caller LinkSelected 0x48c7b0 lives in select.cpp).  If neither side is already a
// self-linked script group, give `b` a fresh script_linkName id (max-of-all + 1, or reuse
// its existing one) and add that id to `a`'s script_linkTo list.
// ─────────────────────────────────────────────────────────────────────────────
void Script_Link( entity_s_def *a, entity_s_def *b )
{
    // 0x48bf3a: if either side is already a self-linked group, nothing to do.
    if ( ScriptGroup_LinkTo( a ) || ScriptGroup_LinkTo( b ) )
        return;

    // 0x48bf61: assign / reuse b's script_linkName id.
    int newId;
    char *bLinkName = ValueForKey2( (int)b, "script_linkName" );
    if ( bLinkName && *bLinkName )
    {
        newId = atol( bLinkName );             // 0x48bf7f: reuse existing id
    }
    else
    {
        // 0x48bf8a: scan ALL entities for the max script_linkName, +1.
        newId = 0;
        for ( entity_s *ent = entities.next; ent != &entities; ent = ent->next )
        {
            for ( epair_t *ep = ent->epairs; ep; ep = ep->next )
            {
                if ( !_stricmp( ep->key, "script_linkName" ) )
                {
                    const char *v = ep->value;
                    if ( v && *v )
                    {
                        int id = atol( v );
                        if ( id > newId )
                            newId = id;
                    }
                    break;                     // 0x48bfc5: only the first match per entity
                }
            }
        }
        ++newId;                               // 0x48c00a
    }

    char idStr[1028];
    sprintf( idStr, "%i", newId );             // 0x48c023
    SetKeyValue( b, "script_linkName", idStr ); // 0x48c038

    // 0x48c03d: fetch b's (now-set) script_linkName value (used by the LABEL_24 fallback).
    const char *bNameVal = (const char *)zero;
    for ( epair_t *ep = b->epairs; ep; ep = ep->next )
        if ( !_stricmp( ep->key, "script_linkName" ) ) { bNameVal = ep->value; break; }

    // 0x48c067: fetch a's existing script_linkTo list.
    const char *aLinkTo = nullptr;
    for ( epair_t *ep = a->epairs; ep; ep = ep->next )
        if ( !_stricmp( ep->key, "script_linkTo" ) ) { aLinkTo = ep->value; break; }

    if ( aLinkTo && *aLinkTo )
    {
        // 0x48c0c9: parse a's list, add newId if not already present.
        LinkList_t links;
        Map_ParseLinkList( &links, aLinkTo );
        if ( links.overflowed )                        // 0x48c0d8
            Sys_Printf( "Exceeded maximum links\n" );

        bool present = false;                          // 0x48c0f1
        for ( int i = 0; i < links.size; ++i )
            if ( links.id[i] == newId ) { present = true; break; }
        if ( !present )
            links.id[links.size++] = newId;            // 0x48c116

        iassert( links.size > 0 );                     // qe3.cpp:567 (0x48c134)

        char buf[1028];
        sprintf( buf, "%i", links.id[0] );             // 0x48c167
        for ( int i = 1; i < links.size; ++i )
            strcat( buf, va( " %i", links.id[i] ) );
        SetKeyValue( a, "script_linkTo", buf );        // 0x48c1e6
    }
    else
    {
        // 0x48c08b (LABEL_24): a had no linkTo list → set it to b's linkName value.
        SetKeyValue( a, "script_linkTo", bNameVal );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x48c970  QE_CountBrushesAndUpdateStatusBar - walk active_brushes, tally g_numbrushes
// (convex brushes) vs g_numentities (fixed-size entity brushes), and refresh the status
// bar when either count changed.
// ─────────────────────────────────────────────────────────────────────────────
int g_numbrushes  = 0;     // IDB 0x240A0E4
int g_numentities = 0;     // IDB 0x240A0E0
static int s_lastbrushcount  = 0;
static int s_lastentitycount = 0;
static int s_didonce         = 0;

extern selbrush_t active_brushes;      // map.cpp (0x23F189C)
extern void       Sys_UpdateStatusBar( void );   // win_qe3.cpp

void QE_CountBrushesAndUpdateStatusBar( void )
{
    g_numbrushes  = 0;
    g_numentities = 0;

    for ( selbrush_t *b = active_brushes.next; b && b != &active_brushes; b = b->next )
    {
        // KISAK: the binary gates on b->faces - the per-instance faceVis cache, which is
        // built LAZILY by the draw pass (Brush_MakeFaceVisuals), so an undrawn brush is not
        // counted.  Gate on b->def instead: the same answer once anything has been drawn,
        // and still correct headless (the selftest host never draws).
        if ( !b->def )
            continue;
        iassert( b->owner->def == b->def->owner );   // qe3.cpp:908
        entity_s     *owner = b->owner;
        entity_s_def *eDef  = owner ? (entity_s_def *)owner->def : nullptr;
        eclass_t     *ec    = eDef ? eDef->eclass : nullptr;
        if ( ec && ec->fixedsize )
            ++g_numentities;
        else
            ++g_numbrushes;
    }

    if ( g_numbrushes != s_lastbrushcount || g_numentities != s_lastentitycount || !s_didonce )
    {
        Sys_UpdateStatusBar();
        s_lastbrushcount  = g_numbrushes;
        s_lastentitycount = g_numentities;
        s_didonce         = 1;
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x48C8B0  QE_SingleBrush — gate for ops that require exactly one non-fixed-size
//  brush selected (Region Set Brush / Set Tall Brush, etc.). Faithful to the IDB;
//  the eclass null-guard is a belt-and-suspenders add (the binary derefs directly).
// ═════════════════════════════════════════════════════════════════════════════
extern int Sys_Printf( const char *fmt, ... );

signed int QE_SingleBrush()
{
    if ( selected_brushes.next == &selected_brushes ||
         selected_brushes.next->next != &selected_brushes )
    {
        Sys_Printf( "Error: you must have a single brush selected\n" );
        return 0;
    }
    // eclass is on the entity DEF, not the 0x54-byte instance: read it via the
    // instance's def (IDB QE_SingleBrush 0x48C8B0). Reading
    // selected_brushes.next->owner->eclass directly walks past the instance and can
    // fault on freed adjacent heap (the delete-path UAF class).
    entity_s *eDef = (entity_s *)selected_brushes.next->owner->def;
    if ( eDef->eclass->fixedsize )
    {
        Sys_Printf( "Error: you cannot manipulate fixed size entities\n" );
        return 0;
    }
    return 1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  PROJECT (.prj) SUBSYSTEM - QE_LoadProject + Project_Write + the project dialogs'
//  shared cores.  A .prj is a single entity block of epairs: basepath / mapspath /
//  entitypath / autosave / game / basegame / layeredmaterials.  QE_LoadProject (0x48bab0)
//  parses it into g_qeglobals.d_project_entity, resolves the search-path keys to full
//  paths, registers fs_basepath/basegame/game, then re-inits renderer + textures +
//  eclasses + layered materials and starts a new map.
//  KISAK: CMainFrame::OnCreate owns the renderer/eclass/texture bring-up in this port, so
//  QE_LoadProject is split - QE_LoadProject_ParseFile is the parse/resolve/dvar half and
//  the re-init TAIL stays the caller's responsibility.  OnCreate calls the parse half early
//  so the project entity and the .prj fs_basepath drive its init.
// ═════════════════════════════════════════════════════════════════════════════

// engine / editor deps (all ported)
extern int    LoadFileNoCrash( const char *filename, void **bufferptr );   // cmdlib.cpp 0x40AC60
extern void   Com_BeginParseSession( const char *name );                   // q_parse
extern entity_s *ParseEntity( const char **text, int version, char a2, char a3 ); // entity.cpp 0x483E70
extern void   SetKeyValue( entity_s_def *e, const char *key, const char *value ); // entity.cpp 0x483690
extern char  *ValueForKey2( int e, const char *key );                      // entity.cpp 0x4825C0
extern CMainFrame *g_pParentWnd;                                           // mainfrm.cpp 0x25D5A70
extern void   Prefab_LevelBack();                                          // errorfile.cpp 0x489D50
extern void   Map_LoadFromFile( const char *path );                        // map.cpp 0x486680
// Com_Error / ERR_FATAL come from qcommon.h (via qe3.h → r_material.h).

// The current project-file path (binary global dword_25D65AC, a CString set via str_set in
// QE_LoadProject and read by Project_Write / OnFileNewproject).  str_set is a no-op stub in
// this port, so store the path in a real heap buffer instead (same lifetime: overwritten on
// each project load, freed by the CRT at exit — matching the binary's single persistent
// CString).
static char *s_currentProjectPath = nullptr;   // == dword_25D65AC
static void Project_SetCurrentPath( const char *path )
{
    char *old = s_currentProjectPath;
    s_currentProjectPath = ( path && *path ) ? _strdup( path ) : _strdup( "" );
    if ( old )
        free( old );
}
const char *Project_GetCurrentPath()          // (helper for the dialog cores below)
{
    return s_currentProjectPath ? s_currentProjectPath : "";
}
// Public wrapper so the New-Project handler (mainfrm.cpp) can seed the path before the
// Project Settings dialog's OK writes the .prj to it (binary get_m_strStatus(&dword_25D65AC)).
void Project_SetCurrentPathPublic( const char *path )
{
    Project_SetCurrentPath( path );
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x484460  Project_WriteQuotedString - quote a string into the .prj, backslash-escaping
// '\' and '"' (the same writer as MapFile_WriteEntity's epair path at 0x484712).
static void Project_WriteQuotedString( const char *s, FILE *f )
{
    if ( !s ) s = "";
    fputc( '"', f );
    for ( const char *c = s; *c; ++c )
    {
        if ( *c == '\\' || *c == '"' )
            fputc( '\\', f );
        fputc( *c, f );
    }
    fputc( '"', f );
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x48bd90  Project_Write - write d_project_entity's epairs back to the .prj (the
// Project Settings / New Project dialogs' OK path).
signed int Project_Write( const char *path )
{
    FILE *f = fopen( path, "w+" );
    if ( !f )
    {
        Com_Error( ERR_FATAL, "Could not open project file!" );
        return 0;
    }
    fprintf( f, "{\n" );
    for ( epair_t *i = ( (entity_s_def *)g_qeglobals.d_project_entity )->epairs; i; i = i->next )
    {
        Project_WriteQuotedString( i->key, f );
        fputc( ' ', f );
        Project_WriteQuotedString( i->value, f );
        fputc( '\n', f );
    }
    fprintf( f, "}\n" );
    fclose( f );
    return 1;
}

// The 4 project search-path keys resolved to full paths in QE_LoadProject; order is the
// IDB pointer array off_739DB4 verbatim.
static const char *const s_projectPathKeys[4] = { "basepath", "autosave", "mapspath", "layeredmaterials" };

// FS dvar registration (binary FS_RegisterDvars 0x40b650) from the resolved .prj values.
// KISAK: only OVERRIDE when the .prj supplies a non-empty value - kisak's own
// FS_RegisterDvars already seeds fs_basepath from the exe-relative install heuristic, and
// the stock cod4.prj leaves game/basegame empty.
static void Project_RegisterFsDvar( const char *name, const char *value )
{
    if ( !value || !*value )
        return;                                  // .prj key absent/empty -> keep the current dvar
    const dvar_s *v = Dvar_FindVar( name );
    if ( v )
        Dvar_SetString( (dvar_s *)v, (char *)value );
    else
        Dvar_RegisterString( name, value, 0x4000 /*FS_RegisterDvars 0x40b650 flag*/, "External Dvar" );
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x48bab0  QE_LoadProject_ParseFile - the PARSE + epair-resolve + FS-dvar half of
// QE_LoadProject.  Loads the .prj, parses it into g_qeglobals.d_project_entity (the FULL
// epair set), resolves the 4 search-path keys to full paths, sets m_strLastProject, and
// registers fs_basepath/basegame/game.  Returns 1, or 0 if the file is missing (the
// binary's LoadFileNoCrash == -1 branch).
signed int QE_LoadProject_ParseFile( const char *path )
{
    void *buf = nullptr;
    Sys_Printf( "QE_LoadProject (%s)\n", path ? path : "(null)" );
    if ( LoadFileNoCrash( path, &buf ) == -1 )
        return 0;                                // .prj absent — faithful: return failure

    // Remember the project-file path (binary: str_set(&dword_25D65AC, path, strlen)).
    Project_SetCurrentPath( path );

    // Parse the single { "key" "value" ... } block into the project entity.  The binary
    // calls ParseEntity(4, 1, 1): version=4, a2=1 (return the raw epair-only entity — no
    // eclass/bbox), a3=1.  The text pointer is the loaded buffer.
    Com_BeginParseSession( path );
    const char *text = (const char *)buf;
    g_qeglobals.d_project_entity = ParseEntity( &text, 4, 1, 1 );

    // Com_EndParseSession (binary decrements parseInfoNum inline, asserting non-empty).
    {
        ParseThreadInfo *pi = Com_GetParseThreadInfo();
        if ( !pi->parseInfoNum )
            Com_Error( ERR_FATAL, "Com_EndParseSession: session underflow" );
        --pi->parseInfoNum;
    }
    free( buf );

    if ( !g_qeglobals.d_project_entity )
    {
        Com_Error( ERR_FATAL, "Couldn't parse %s", path );
        return 0;
    }

    // m_strLastProject = path (binary: get_m_strStatus(&g_PrefsDlg->m_strLastProject, path)).
    if ( g_PrefsDlg )
        g_PrefsDlg->m_strLastProject = path;

    // Resolve the 4 search-path keys to full paths (binary 0x48BB9F loop): each value that
    // is present and does NOT already start with '\' or '/' (i.e. is relative) is expanded
    // via GetFullPathNameA against the CWD and written back with SetKeyValue.  A missing
    // key resolves the empty string ("").
    for ( int i = 0; i < 4; ++i )
    {
        const char *value = "";
        for ( epair_t *ep = ( (entity_s_def *)g_qeglobals.d_project_entity )->epairs; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, s_projectPathKeys[i] ) ) { value = ep->value; break; }
        }
        // Absolute paths (leading '\' or '/') are left as-is (binary skips GetFullPathName).
        if ( *value == '\\' || *value == '/' )
            continue;
        char full[1028];
        if ( GetFullPathNameA( value, sizeof( full ), full, 0 ) )
            SetKeyValue( (entity_s_def *)g_qeglobals.d_project_entity, s_projectPathKeys[i], full );
    }

    // FS_RegisterDvars(basepath, basegame, game) @0x48bce5.  KISAK: the stock cod4.prj's
    // basepath ".." was resolved above against the PROCESS CWD - the install root in a real
    // install, but `bin` in the dev build (bin\Debug), where there is no game data.  So only
    // override fs_basepath when the resolved directory actually holds data (raw\ or main\);
    // otherwise keep the port's fs_basepath heuristic (com_files.cpp).
    {
        const char *base = ValueForKey2( (int)(intptr_t)g_qeglobals.d_project_entity, "basepath" );
        if ( base && *base )
        {
            char probe[1028];
            _snprintf( probe, sizeof( probe ), "%s\\raw", base ); probe[sizeof( probe ) - 1] = 0;
            bool hasData = ( GetFileAttributesA( probe ) != INVALID_FILE_ATTRIBUTES );
            if ( !hasData )
            {
                _snprintf( probe, sizeof( probe ), "%s\\main", base ); probe[sizeof( probe ) - 1] = 0;
                hasData = ( GetFileAttributesA( probe ) != INVALID_FILE_ATTRIBUTES );
            }
            if ( hasData )
                Project_RegisterFsDvar( "fs_basepath", base );
            else
                Sys_Printf( "QE_LoadProject: .prj basepath '%s' has no game data — keeping fs_basepath heuristic\n", base );
        }
    }
    Project_RegisterFsDvar( "fs_basegame", ValueForKey2( (int)(intptr_t)g_qeglobals.d_project_entity, "basegame" ) );
    Project_RegisterFsDvar( "fs_game",     ValueForKey2( (int)(intptr_t)g_qeglobals.d_project_entity, "game" ) );

    Sys_Printf( "QE_LoadProject: parsed %s\n", path );
    return 1;
}

// ═════════════════════════════════════════════════════════════════════════════
//  MRU (recent-files) subsystem — the classic Q3Radiant/MFC MRU menu.  All functions
//  transcribed verbatim from the CoD4Radiant disasm.  The MRU menu is stored in
//  g_qeglobals.d_lpMruMenu (an LPMRUMENU, see qe3.h).
//    CreateMruMenuDefault 0x48A150 — GlobalAlloc + seed defaults (created in OnCreate).
//    MRU_NewItem          0x48A2C0 — insert/promote a file to the top of the list.
//    MRU_InsertItem       0x48A400 — rebuild the File-menu recent-files items.
//    DelMenuItem          0x48A3A0 — remove a stale item (open-failed) from the list.
//    SaveMruInReg/LoadMruInReg 0x48A750/0x48A870 — persist to/from the registry.
//    DoMru                0x4994B0 — the recent-file-click handler (open the item).
//  Registry: HKCU\Software\iw\CoD4Radiant\MRU, values File1..File9.
// ═════════════════════════════════════════════════════════════════════════════

// ── 0x48A150  CreateMruMenuDefault ───────────────────────────────────────────
LPMRUMENU *CreateMruMenuDefault()
{
    HGLOBAL h = GlobalAlloc( GHND, sizeof( LPMRUMENU ) );      // 0x42 = GMEM_MOVEABLE|GMEM_ZEROINIT
    LPMRUMENU *mru = (LPMRUMENU *)GlobalLock( h );
    mru->wNbItemFill     = 0;
    mru->wNbLruMenu      = 9;      // v1[2]
    mru->wNbLruShow      = 6;      // v1[1]
    mru->wIdMru          = 8000;   // v1[4]
    mru->wMaxSizeLruItem = 128;    // v1[3]
    HGLOBAL h2 = GlobalAlloc( GHND, 0x480u );                 // 9 * 128 = 1152
    void *store = GlobalLock( h2 );
    mru->lpMRU = (char *)store;                               // *((_DWORD*)v1 + 3)
    if ( store )
        return mru;
    GlobalUnlock( GlobalHandle( mru ) );
    GlobalFree( GlobalHandle( mru ) );
    return nullptr;
}

// ── 0x48A2C0  MRU_NewItem — promote/insert lpString1 to slot 0 ────────────────
void MRU_NewItem( LPMRUMENU *mru, const char *lpString1 )
{
    unsigned short idx = 0;
    if ( mru->wNbItemFill )
    {
        // If already present, shift the entries above it down (promote to front).
        while ( lstrcmpiA( lpString1, &mru->lpMRU[idx * mru->wMaxSizeLruItem] ) )
        {
            if ( ++idx >= mru->wNbItemFill )
                goto make_room;
        }
        for ( int v2 = idx; v2 > 0; --v2 )
            lstrcpyA( &mru->lpMRU[mru->wMaxSizeLruItem * v2],
                      &mru->lpMRU[mru->wMaxSizeLruItem * ( v2 - 1 )] );
        strncpy( mru->lpMRU, lpString1, mru->wMaxSizeLruItem - 1 );
        return;
    }
make_room:
    {
        int cnt = mru->wNbItemFill + 1;
        if ( cnt >= mru->wNbLruMenu )
            cnt = mru->wNbLruMenu;
        mru->wNbItemFill = (unsigned short)cnt;
        for ( int v6 = cnt - 1; v6 > 0; --v6 )
            lstrcpyA( &mru->lpMRU[mru->wMaxSizeLruItem * v6],
                      &mru->lpMRU[mru->wMaxSizeLruItem * ( v6 - 1 )] );
        strncpy( mru->lpMRU, lpString1, mru->wMaxSizeLruItem - 1 );
    }
}

// ── 0x48A400  MRU_InsertItem — rebuild the recent-files entries in a menu ──────
void MRU_InsertItem( LPMRUMENU *mru, HMENU hMenu )
{
    if ( !hMenu )
        return;
    // Remove any existing MRU items (wIdMru .. wIdMru+wNbLruMenu) + the separator.
    for ( int v2 = 0; v2 <= mru->wNbLruMenu; ++v2 )
        RemoveMenu( hMenu, v2 + mru->wIdMru, MF_BYCOMMAND );
    if ( !mru->wNbItemFill )
        return;
    InsertMenuA( hMenu, 0x80B7u, MF_BYCOMMAND | MF_SEPARATOR, mru->wIdMru, 0 );  // 0x800 = MF_SEPARATOR
    int show = mru->wNbItemFill;
    if ( show >= mru->wNbLruShow )
        show = mru->wNbLruShow;
    int last = show - 1;
    for ( int v4 = last; v4 >= 0; --v4 )
    {
        HGLOBAL h = GlobalAlloc( GHND, mru->wMaxSizeLruItem + 20 );
        char *buf = (char *)GlobalLock( h );
        if ( buf )
        {
            wsprintfA( buf, "&%lu %s", v4 + 1, &mru->lpMRU[v4 * mru->wMaxSizeLruItem] );
            UINT insertBefore = mru->wIdMru + v4 + 2;
            if ( v4 == last )
                insertBefore = mru->wIdMru;      // last (oldest) inserted before the separator
            InsertMenuA( hMenu, insertBefore, MF_BYCOMMAND, mru->wIdMru + v4 + 1, buf );
            GlobalUnlock( GlobalHandle( buf ) );
            GlobalFree( GlobalHandle( buf ) );
        }
    }
}

// ── 0x48A3A0  DelMenuItem — drop a stale (open-failed) MRU entry ──────────────
static signed int DelMenuItem( unsigned short nID, LPMRUMENU *mru )
{
    unsigned short v2 = (unsigned short)( nID - mru->wIdMru - 1 );
    if ( mru->wNbItemFill <= v2 )
        return 0;
    mru->wNbItemFill -= 1;
    for ( unsigned short i = v2; i < mru->wNbItemFill; ++i )
        lstrcpyA( &mru->lpMRU[i * mru->wMaxSizeLruItem],
                  &mru->lpMRU[mru->wMaxSizeLruItem * ( i + 1 )] );
    return 1;
}

// ── 0x48A750  SaveMruInReg — write the MRU list to HKCU\...\MRU\File1..File9 ───
void SaveMruInReg( LPMRUMENU *mru )
{
    HGLOBAL h = GlobalAlloc( GHND, mru->wMaxSizeLruItem + 20 );
    char *buf = (char *)GlobalLock( h );
    if ( !buf )
        return;
    HKEY hKey = 0;
    DWORD disp = 0;
    RegCreateKeyExA( HKEY_CURRENT_USER, "Software\\iw\\CoD4Radiant\\MRU", 0, 0, 0,
                     KEY_ALL_ACCESS, 0, &hKey, &disp );
    for ( unsigned short i = 0; i < mru->wNbLruMenu; ++i )
    {
        char valueName[16];
        wsprintfA( valueName, "File%lu", i + 1 );
        int cap = mru->wMaxSizeLruItem + 10;
        if ( i < mru->wNbItemFill )
        {
            strncpy( buf, &mru->lpMRU[mru->wMaxSizeLruItem * i], cap );
            buf[cap - 1] = 0;
        }
        else
        {
            buf[0] = 0;
        }
        RegSetValueExA( hKey, valueName, 0, REG_SZ, (const BYTE *)buf, lstrlenA( buf ) );
    }
    RegCloseKey( hKey );
    GlobalUnlock( GlobalHandle( buf ) );
    GlobalFree( GlobalHandle( buf ) );
}

// ── 0x48A870  LoadMruInReg — read the MRU list from the registry ──────────────
void LoadMruInReg( LPMRUMENU *mru )
{
    HGLOBAL h = GlobalAlloc( GHND, mru->wMaxSizeLruItem + 20 );
    char *buf = (char *)GlobalLock( h );
    if ( !buf )
        return;
    HKEY hKey = 0;
    RegOpenKeyExA( HKEY_CURRENT_USER, "Software\\iw\\CoD4Radiant\\MRU", 0, KEY_READ, &hKey );
    for ( unsigned short i = 0; i < mru->wNbLruMenu; ++i )
    {
        char valueName[16];
        wsprintfA( valueName, "File%lu", i + 1 );
        buf[0] = 0;
        DWORD type = 0;
        DWORD cbData = mru->wMaxSizeLruItem + 10;
        RegQueryValueExA( hKey, valueName, 0, &type, (BYTE *)buf, &cbData );
        buf[cbData] = 0;
        if ( !buf[0] )
            break;                               // first empty slot ends the list
        if ( i < 9 )
        {
            strncpy( &mru->lpMRU[mru->wMaxSizeLruItem * i], buf, mru->wMaxSizeLruItem - 1 );
            if ( mru->wNbItemFill <= i + 1 )
                mru->wNbItemFill = (unsigned short)( i + 1 );
        }
    }
    RegCloseKey( hKey );
    GlobalUnlock( GlobalHandle( buf ) );
    GlobalFree( GlobalHandle( buf ) );
}

// ── 0x4994B0  DoMru — open a recent file (the 8000..8009 command handler body) ─
extern void Pointfile_Clear();                             // points.cpp
BOOL DoMru( short nID, HWND hWnd )
{
    // Unsaved-changes / inside-prefab guard (binary inlines HasUnsavedChangesOrInsidePrefab
    // + layered-material CRC check + ConfirmModified; the port consolidates that exact test
    // in CMainFrame::OkToDiscard).
    if ( g_pParentWnd && !g_pParentWnd->OkToDiscard() )
        return 0;
    Prefab_LevelBack();

    LPMRUMENU *mru = g_qeglobals.d_lpMruMenu;
    char fileName[132] = "";
    unsigned short slot = (unsigned short)( nID - mru->wIdMru - 1 );
    if ( slot < mru->wNbItemFill )
    {
        strncpy( fileName, &mru->lpMRU[slot * mru->wMaxSizeLruItem], 128u );
        fileName[127] = 0;
    }

    OFSTRUCT reopen;
    HFILE hf = OpenFile( fileName, &reopen, OF_EXIST );      // 0x4000 = OF_EXIST
    BOOL ok = ( hf != HFILE_ERROR );
    if ( hf == HFILE_ERROR )
    {
        DelMenuItem( nID, mru );                             // file gone → drop it
    }
    else
    {
        MRU_NewItem( mru, fileName );                        // promote to top
        Pointfile_Clear();
        Map_LoadFromFile( fileName );
    }
    // Rebuild the recent-files menu items in the File menu (submenu 0).
    HMENU subMenu = GetSubMenu( GetMenu( hWnd ), 0 );
    MRU_InsertItem( mru, subMenu );
    return ok;
}

// ═════════════════════════════════════════════════════════════════════════════
//  RELOCATED HOME — this function's embedded Assert() calls name THIS file as
//  their source (see the brush.cpp relocation protocol / line-uniqueness test).
// ═════════════════════════════════════════════════════════════════════════════
// ── sub_48BE20 — parse SPACE-delimited int list (Map_ParseLinkList) ───────────────────────────────
// 0x48be20 Map_ParseLinkList. __usercall(buf@ebx, linkTo).  Parses linkTo (script_linkTo /
// script_linkName) as a SPACE-delimited int list with DEDUP, capped at 30 entries.
// Layout: buf[0..count-1]=values, buf[count]=-1 terminator, buf[1024]=count (byte 0x1000),
// byte buf+0x1004 = overflow flag (cleared each call, set when the 30-cap is hit).  Parses
// in place; cross-file Assert qe3.cpp:446.  Caller buf must be int[1026]+ so buf+0x1004 is
// in bounds.
void Map_ParseLinkList( LinkList_t *buf, const char *linkTo )
{
    int  count = 0;
    bool atBoundary = true;                          // v7: at a token start
    buf->overflowed = false;                         // (IDA 0x48be31)
    size_t i   = 0;
    size_t len = strlen( linkTo );
    if ( len )
    {
        for ( ;; )
        {
            const char *p = &linkTo[i];
            iassert( linkTo[i] );   // qe3.cpp:446
            if ( *p == ' ' )
            {
                atBoundary = true;
            }
            else if ( atBoundary )
            {
                int  value = atol( p );
                bool dup   = false;                  // dedup scan buf[0..count-1] (IDA 0x48be9a)
                for ( int k = 0; k < count; ++k )
                    if ( buf->id[k] == value ) { dup = true; break; }
                if ( !dup )
                {
                    buf->id[count++] = value;
                    atBoundary   = false;
                    if ( count >= 30 )               // overflow (IDA cmp esi,1Eh)
                    {
                        buf->overflowed = true;
                        break;
                    }
                }
            }
            ++i;
            len = strlen( linkTo );                      // IDA recomputes strlen each pass
            if ( i >= len )
                break;
        }
    }
    buf->id[count] = -1;                              // -1 terminator at id[size]
    buf->size      = count;
}
