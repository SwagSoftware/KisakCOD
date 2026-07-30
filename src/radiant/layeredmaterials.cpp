#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\layeredmaterials.cpp
// IDA: 0x416ac0 (LayeredMaterialLibrary, 616 B)
//      0x417190 (LayeredMaterials_texcoords, 185 B)
//      0x4D6C40 (CheckLayeredMaterial_Modifications — real CRC32, was silent-0 in engine_stubs)
//      0x416a40 (q_shared_texcoords — file-static helper, no external callsites)
// Assert: LayeredMaterials.cpp:65, :297
//
// Layered material library parser and texture utility.

#include "stdafx.h"
#include <cstdio>                 // FILE, fprintf, fclose
#include <cstdlib>                // qsort, free (LayerdMatWnd)
#include <cstring>                // memset, _stricmp (LayerdMatWnd)
#include "qe3.h"                  // g_qeglobals, entity_s, epair_t, qtexture_s
#include <universal/q_shared.h>   // I_stricmp, I_strnicmp
#include <universal/q_parse.h>    // Com_Parse, Com_ParseInt, Com_MatchToken

// Assert/Sys_Printf — defined in engine_stubs.cpp.
extern void Assert( const char *file, int line, int type, const char *fmt, ... );
extern int  Sys_Printf( const char *fmt, ... );
extern void Com_Error( int code, const char *fmt, ... );
extern char *va( const char *fmt, ... );

// ─────────────────────────────────────────────────────────────────────────────
// Forward declarations
// ─────────────────────────────────────────────────────────────────────────────
extern qtexture_s *Texture_GetHandle( const char *name );   // texwnd.cpp

// j__atol (0x5c0495) — CRT atol wrapper; imported from engine.
extern long j__atol( const char *str );

// Disk write (map.cpp) — opens the path "wb" (Perforce-aware when enabled).
extern FILE *Map_SaveFileToPerforce( const char *path, char a2 );          // 0x48CC70

// LoadFile (cmdlib.cpp 0x40ABD0) — read a file into a qblockmalloc'd buffer; returns
// the byte length, or -1 if the path is NULL/empty/not found.  NOTE: it does a plain
// fopen() relative to the editor CWD (no FS search paths) — see LayerdMatWnd's epair.
extern int LoadFile( const char *filename, void **bufferptr );             // 0x40ABD0

// ── Layered-material WINDOW globals/helpers (layeredmaterialwnd.cpp) ──────────
// AddEntries makes the newly-created entry the window's active material and refreshes
// the toolbar/title.  These are inert when the window has not been created
// (lyrMtlWndGlob.hwnd == NULL → SetWindowTextA on NULL is a harmless no-op; the
// toolbar sync early-returns on a NULL toolbar HWND from SendMessage).
// (lyrMtlWndGlob extern comes from qe3.h)
extern int  g_nUpdateBits;                           // 0x25D5A74
// sub_4174E0 (toolbar button enable/check sync) lives in layeredmaterialwnd.cpp.
extern "C" int LayeredMaterialWnd_SyncToolbar();     // wrapper around sub_4174E0

// ─────────────────────────────────────────────────────────────────────────────
// lyrMtlGlob — global storage for the layered material library.
//
// IDB layout (verified from disasm of LayeredMaterialLibrary 0x416ac0):
//   0x1814CF8  int      lyrMtlGlob.crcToken           CRC32 token
//   0x1814CFC  int      lyrMtlGlob.entryCount   # of loaded entries
//   0x1814D00  uint8[]  lyrMtlGlob.Layers        inline array, 512 × 84 bytes
//
// Entry (84 bytes) layout:
//   +0x00  char[64]  name         entry name
//   +0x40  int       nextId       next layer id to assign
//   +0x44  int       layerCount   number of layers in this entry (max 1 per IDA check)
//   +0x48  int       _pad
//   +0x4C  int       layer[n].id       layer ids (8 bytes per layer: id + handle)
//   +0x50  int       layer[n].handle   qtexture_s* handle (stored as int)
//
// IDA confirmed lyrMtlGlob.Layers is an INLINE array (not a pointer):
//   disasm of 0x416b78: "add esi, offset lyrMtlGlob.Layers"  — address-of, not deref.
// The earlier engine_stubs.cpp "void *lyrMtlGlob.Layers = nullptr" was incorrect.
// These globals are now defined here; engine_stubs.cpp entries must be removed.
// ─────────────────────────────────────────────────────────────────────────────

// Offsets within each 84-byte entry.
enum {
    LYR_ENTRY_SIZE    = 84,
    LYR_NAME_SIZE     = 64,
    LYR_NEXTID_OFF    = 0x40,
    LYR_LAYERCNT_OFF  = 0x44,
    LYR_ID_OFF        = 0x4C,   // layer[0].id
    LYR_HANDLE_OFF    = 0x50,   // layer[0].handle
    LYR_MAX_ENTRIES   = 512,
    LYR_MAX_LAYERS    = 1,
};

// Typed view of one 84-byte library entry (stride within lyrMtlGlob.Layers).
// Same head layout as materialdef.cpp's 120-byte LayerMaterialDef, but the library
// stride only carries LYR_MAX_LAYERS layer slots.
struct LyrEntry_t
{
    char name[LYR_NAME_SIZE];                                   // 0x00
    int  nextId;                                                // 0x40
    int  layerCount;                                            // 0x44
    int  activeLayer;                                           // 0x48
    struct { int id; qtexture_s *handle; } layers[LYR_MAX_LAYERS]; // 0x4C
};
static_assert( sizeof( LyrEntry_t ) == LYR_ENTRY_SIZE, "LyrEntry_t" );
static_assert( offsetof( LyrEntry_t, nextId ) == LYR_NEXTID_OFF
            && offsetof( LyrEntry_t, layers ) == LYR_ID_OFF, "LyrEntry_t offsets" );

LyrMtlGlob_t lyrMtlGlob = {};   // type in qe3.h
static_assert( sizeof( lyrMtlGlob.Layers ) == LYR_MAX_ENTRIES * LYR_ENTRY_SIZE,
               "LyrMtlGlob_t.Layers size" );

// ─────────────────────────────────────────────────────────────────────────────
// CheckLayeredMaterial_Modifications (IDA 0x4D6C40)
// CRC32 with Ethernet polynomial 0xEDB88320, Lsb-first, 8 bits per byte.
// a1=buffer, a2=size (bytes), a3=seed (used as ~a3 for the initial CRC value).
// Returns ~crc (standard final XOR).
// Was a silent return-0 stub in engine_stubs.cpp; real implementation here.
// ─────────────────────────────────────────────────────────────────────────────
unsigned int CheckLayeredMaterial_Modifications( uint8_t *a1, int a2, int a3 )
{
    const uint8_t *v3  = a1;
    const uint8_t *end = a1 + a2;
    unsigned int   crc = ~(unsigned int)a3;

    while ( v3 != end )
    {
        // Lsb-first CRC32 bit-at-a-time, 8 rounds per byte.
        // IDA unrolls to 4 rounds via v5/v6/v7 intermediates; we match behaviour.
        unsigned int x = (*v3 ^ crc);

        // Round 1 (IDA v5):
        unsigned int r1 = (x >> 1) ^ (0xEDB88320u & -(x & 1u));
        // Round 2:
        unsigned int r2 = (r1 >> 1) ^ (0xEDB88320u & -(r1 & 1u));
        // Round 3:
        unsigned int r3 = (r2 >> 1) ^ (0xEDB88320u & -(r2 & 1u));
        // Round 4 (IDA v7 state, then 4 more in the loop update expression):
        unsigned int r4 = (r3 >> 1) ^ (0xEDB88320u & -(r3 & 1u));
        unsigned int r5 = (r4 >> 1) ^ (0xEDB88320u & -(r4 & 1u));
        unsigned int r6 = (r5 >> 1) ^ (0xEDB88320u & -(r5 & 1u));
        unsigned int r7 = (r6 >> 1) ^ (0xEDB88320u & -(r6 & 1u));
        unsigned int r8 = (r7 >> 1) ^ (0xEDB88320u & -(r7 & 1u));

        crc = r8;
        ++v3;
    }
    return ~crc;
}

// ─────────────────────────────────────────────────────────────────────────────
// q_shared_texcoords (IDA 0x416a40) — file-static
// Linear search for `name` in lyrMtlGlob.Layers[].
// Returns the index of the matching entry, or lyrMtlGlob.entryCount if not found
// (i.e. the "append here" slot).
// Assert strings from q_shared.cpp:640 / :641 — the function was inlined from
// q_shared.cpp's I_strnicmp wrapper.
// ─────────────────────────────────────────────────────────────────────────────
static int q_shared_texcoords( const char *name )
{
    int v1 = 0;
    if ( lyrMtlGlob.entryCount > 0 )
    {
        const uint8_t *v2 = lyrMtlGlob.Layers;
        do
        {
            // I_stricmp inlined in the binary (it carries q_shared.cpp:640/641)
            if ( !I_stricmp( (const char *)v2, name ) )
                break;
            ++v1;
            v2 += LYR_ENTRY_SIZE;
        }
        while ( v1 < lyrMtlGlob.entryCount );
    }
    return v1;
}

// ─────────────────────────────────────────────────────────────────────────────
// LayeredMaterials_GetMaterial (IDA 0x4172f0)
// Binary search over the SORTED lyrMtlGlob.Layers[] for an entry whose name
// (offset 0, 64-byte) case-insensitively equals `name`. Returns the entry pointer
// (cast by SetMaterial to LayerMaterialDef*), or NULL if not found / library empty.
// When the layered-material library has not been loaded (lyrMtlGlob.entryCount==0 —
// the case for the headless gate AND for stock maps with no layered materials),
// this returns NULL immediately, so SetMaterial falls through to Texture_GetHandle.
// NOTE (latent, P5.6): library entries are 84 bytes but a realised LayerMaterialDef
// is 120; reading past offset 84 only happens once the library is actually loaded
// (LayeredMaterialLibrary), which the editor does not do on the basic map-display
// path — flagged for the layered-material window work.
// ─────────────────────────────────────────────────────────────────────────────
void *LayeredMaterials_GetMaterial( const char *name )
{
    int hi = lyrMtlGlob.entryCount - 1;
    int lo = 0;
    if ( hi < 0 )
        return nullptr;                       // empty library

    int  mid;
    int  cmp;
    for ( ;; )
    {
        mid = (hi + lo) / 2;
        const uint8_t *entry = &lyrMtlGlob.Layers[LYR_ENTRY_SIZE * mid];
        // I_stricmp inlined in the binary (it carries q_shared.cpp:640/641)
        cmp = I_stricmp( (const char *)entry, name );               // entry vs name
        if ( cmp >= 0 )
        {
            if ( cmp == 0 )
                return &lyrMtlGlob.Layers[LYR_ENTRY_SIZE * mid];    // exact match
            hi = mid - 1;                     // entry > name → search lower half
        }
        else
        {
            lo = mid + 1;                     // entry < name → search upper half
        }
        if ( lo > hi )
            return nullptr;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// LayeredMaterialLibrary (IDA 0x416ac0)
// Assert: LayeredMaterials.cpp:65
//
// Parses a "LayeredMaterialLibrary" block from a text token stream.
// Returns 1 on success/recoverable error, 0 on fatal parse error.
//
// Grammar (from IDA control flow):
//   "LayeredMaterialLibrary" <version:int>
//   { <name> <nextId:int> { (<id:int> <material> ";")* } }*
// ─────────────────────────────────────────────────────────────────────────────
char LayeredMaterialLibrary( const char **data_p )
{
    // v1 tracks the outer parse cursor (same as data_p, as confirmed by IDA).
    const char **v1 = data_p;

    if ( !Com_MatchToken( data_p, "LayeredMaterialLibrary", 1 ) )
        return 0;

    int v2 = Com_ParseInt( data_p );
    if ( v2 < 1 )
    {
        Sys_Printf( "Layered material library has bad version number %i\n", v2 );
        return 1;
    }
    if ( v2 > 1 )
    {
        Sys_Printf( "Can't read newer layered material library version "
                    "(code is version %i, file is version %i)\n", 1, v2 );
        return 1;
    }

    iassert( lyrMtlGlob.entryCount == 0 );   // LayeredMaterials.cpp:65

    // Read first entry name (or end of stream).
    // Com_Parse returns parseInfo_t*; use ->token for the string.
    const char *v4 = Com_Parse( data_p )->token;
    if ( !*data_p )
        return 1;

    // Outer loop: parse each named entry.
LABEL_9:
    {
        int v5  = q_shared_texcoords( v4 );
        int v11 = v5;

        if ( v5 == LYR_MAX_ENTRIES )
        {
            Sys_Printf( "ERROR: More than %i entries in layered material library.\n",
                        LYR_MAX_ENTRIES );
            return 0;
        }

        LyrEntry_t *v6 = (LyrEntry_t *)( lyrMtlGlob.Layers + LYR_ENTRY_SIZE * v5 );

        strcpy( v6->name, v4 );
        v6->nextId = Com_ParseInt( v1 );

        if ( Com_MatchToken( (const char **)v1, "{", 1 ) )
        {
            // Parse layers for this entry.
            while ( true )
            {
                const char *v7 = Com_Parse( (const char **)v1 )->token;
                if ( !*v1 )
                    return 0;   // end of stream inside block = failure

                if ( *v7 == '}' )
                {
                    // Close brace: commit entry if new.
                    if ( v11 == lyrMtlGlob.entryCount )
                        ++lyrMtlGlob.entryCount;

                    // Read next entry name.
                    v4 = Com_Parse( (const char **)v1 )->token;
                    if ( *v1 )
                        goto LABEL_9;
                    return 1;
                }

                // Layer count check: IDA allows max LYR_MAX_LAYERS (1).
                int layerCount = v6->layerCount;
                if ( layerCount == LYR_MAX_LAYERS )
                {
                    Sys_Printf( "ERROR: more than %i layers in layered material %s\n",
                                LYR_MAX_LAYERS, v6->name );
                    return 0;
                }

                // Parse id (stored in v12[4] in the original).
                long v8 = j__atol( v7 );

                if ( v8 >= (long)v6->nextId )
                {
                    Sys_Printf( "ERROR: layer id %i >= next id %i in layered material %s\n",
                                (int)v8, v6->nextId, v6->name );
                    return 0;
                }

                int v9 = 0;
                const char *v13 = Com_Parse( (const char **)v1 )->token;

                // Walk existing layers to check for duplicates.
                if ( layerCount > 0 )
                {
                    while ( true )
                    {
                        if ( (long)v6->layers[v9].id == v8 )
                        {
                            Sys_Printf( "ERROR: Duplicate layer id %i in layered material %s\n",
                                        (int)v8, v6->name );
                            return 0;
                        }
                        if ( !I_stricmp( v13, v6->layers[v9].handle->name ) )
                        {
                            // Fall through to dup-material error below.
                            break;
                        }
                        ++v9;
                        if ( v9 >= layerCount )
                        {
                            // No duplicate found — proceed to LABEL_21 (write).
                            v1 = data_p;    // IDA restores v1 = data_p here
                            goto LABEL_21;
                        }
                    }
                    // Broke out of loop = duplicate material.
                    Sys_Printf( "ERROR: Duplicate material %s in layered material %s\n",
                                v13, v6->name );
                    return 0;
                }

LABEL_21:
                // Write the new layer entry.
                v6->layers[layerCount].id     = (int)v8;
                v6->layers[layerCount].handle = Texture_GetHandle( v13 );
                ++v6->layerCount;

                if ( !Com_MatchToken( (const char **)v1, ";", 1 ) )
                    return 0;
            }
        }
    }
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// LayeredMaterial_NameCompare (IDA 0x416d30) — qsort comparator for the entry array.
// The entry name is the first field of each 84-byte record, so a bare I_stricmp on the
// two record pointers sorts case-insensitively by name.  (The IDB symbol was the
// /OPT:ICF fold name _WinMain@16; its only live use is this qsort callback.)
// ─────────────────────────────────────────────────────────────────────────────
static int __cdecl LayeredMaterial_NameCompare( const void *a, const void *b )
{
    return I_stricmp( (const char *)a, (const char *)b );
}

// ─────────────────────────────────────────────────────────────────────────────
// LayerdMatWnd (IDA 0x416d40) — load the layered-material library.
//
// Called from QE_LoadProject (0x48bab0 @ 0x48bcfc), right after Load_Textures(), once
// the project entity has been parsed.  Resets the in-memory library, reads the file
// named by the project entity's "layeredmaterials" epair, parses it with
// LayeredMaterialLibrary (with spaceDelimited DISABLED so layer ids/names tokenise the
// way the parser expects), sorts the entries by name, and CRC-baselines the clean load
// (so LayeredMaterials_Save only writes when the operator has actually modified it).
// Returns the CRC token on the load-and-parse path (the IDB's `result = CheckLayered...`),
// or the LoadFile result (-1) when the file is absent.
//
// FILE RESOLUTION (important): LoadFile() does a plain fopen() relative to the editor
// CWD — it does NOT consult FS search paths.  cod4.prj names the library
// "cod4_layered_material_library.txt"; that file ships in the CoD4 install's bin\ dir,
// so for the loader to find it the file must sit in the editor's working directory
// (bin\Debug).  When it is absent, LoadFile returns -1, this function leaves
// lyrMtlGlob.entryCount == 0, and the layered-material view is (faithfully) empty.
// ─────────────────────────────────────────────────────────────────────────────
signed int LayerdMatWnd()
{
    lyrMtlGlob.entryCount = 0;
    memset( lyrMtlGlob.Layers, 0, LYR_MAX_ENTRIES * LYR_ENTRY_SIZE );   // 0xA800 = 512*84

    // Resolve the library path from the project entity's "layeredmaterials" epair.
    // (Mirror of LayeredMaterials_Save's epair walk, and of the IDB's inline loop.)
    const char *value = "";
    epair_t *ep = g_qeglobals.d_project_entity ? g_qeglobals.d_project_entity->epairs : nullptr;
    for ( ; ep; ep = ep->next )
    {
        if ( !_stricmp( ep->key, "layeredmaterials" ) ) { value = ep->value; break; }
    }

    void *data = nullptr;
    signed int result = LoadFile( value, &data );
    if ( result >= 0 )
    {
        Com_BeginParseSession( value );
        // IDB: g_parse.parseInfo[g_parse.parseInfoNum].spaceDelimited = 0;
        // The parser relies on this — Com_SetSpaceDelimited(0) is the exact API equivalent.
        Com_SetSpaceDelimited( 0 );

        const char *cursor = (const char *)data;
        if ( !LayeredMaterialLibrary( &cursor ) )
            Com_Error( ERR_DROP, "Layered material library is corrupt.\n" );

        Com_EndParseSession();          // IDB: underflow-guard then --g_parse.parseInfoNum
        free( data );

        qsort( lyrMtlGlob.Layers, lyrMtlGlob.entryCount, LYR_ENTRY_SIZE, LayeredMaterial_NameCompare );
        lyrMtlGlob.crcToken = (int)CheckLayeredMaterial_Modifications( lyrMtlGlob.Layers,
                                                                 LYR_ENTRY_SIZE * lyrMtlGlob.entryCount, 0 );
        result = lyrMtlGlob.crcToken;         // IDB returns the CRC token from this path
    }
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// LayeredMaterial_ValidateName (IDA 0x417000, file-static sub_417000)
// Returns an error string if `name` is invalid (empty / non-alnum-non-'_' / >= 64
// chars), or NULL if valid.  The >=64 message uses va().
// ─────────────────────────────────────────────────────────────────────────────
static const char *LayeredMaterial_ValidateName( const char *name )
{
    if ( !*name )
        return "Layered material names must be at least one character long.";
    const char *p = name;
    do
    {
        if ( !isalnum( (unsigned char)*p ) && *p != '_' )
            return "Layered material names must contain only alphanumeric characters and '_'.";
        ++p;
    }
    while ( *p );
    if ( p - name < 64 )
        return nullptr;
    return va( "Layered material name length must not exceed %i characters.", 63 );
}

// ─────────────────────────────────────────────────────────────────────────────
// LayeredMaterials_WriteFile (IDA 0x416E50, file-static sub_416E50)
// Writes the whole library to an open FILE*: a "LayeredMaterialLibrary 1" header
// then, per entry, "<name> <nextId>\n{ \t<id> "<material>";\n... }".  Returns 1.
// The material name for a layer is the radMtl handle's name (qtexture_s.name @+4),
// reached via the handle stored at the layer's +0x50 slot.
// ─────────────────────────────────────────────────────────────────────────────
static char LayeredMaterials_WriteFile( FILE *f )
{
    fprintf( f, "LayeredMaterialLibrary %i\n", 1 );
    for ( int i = 0; i < lyrMtlGlob.entryCount; ++i )
    {
        LyrEntry_t *entry = (LyrEntry_t *)&lyrMtlGlob.Layers[LYR_ENTRY_SIZE * i];
        fprintf( f, "\n%s %i\n", entry->name, entry->nextId );
        fprintf( f, "{\n" );
        for ( int j = 0; j < entry->layerCount; ++j )
            fprintf( f, "\t%i \"%s\";\n", entry->layers[j].id, entry->layers[j].handle->name );
        fprintf( f, "}\n" );
    }
    return 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// LayeredMaterials_AddEntries (IDA 0x417050)
// Validate the name, reject if the library is full (512) or the name already exists,
// otherwise insert a new ZEROED 84-byte entry KEEPING the array sorted by name
// (case-insensitive), make it the window's active material, refresh title/toolbar.
// PURE IN-MEMORY (the disk write is LayeredMaterials_Save, separately gated).
// ─────────────────────────────────────────────────────────────────────────────
void LayeredMaterials_AddEntries( char *name, HWND hWnd )
{
    const char *err = LayeredMaterial_ValidateName( name );
    if ( err )
    {
        MessageBoxA( hWnd, err, "Radiant", MB_ICONEXCLAMATION );
        return;
    }
    if ( lyrMtlGlob.entryCount == LYR_MAX_ENTRIES )
    {
        Sys_Printf( "Layered material library is currently limited to %i entries.\n", LYR_MAX_ENTRIES );
        return;
    }

    // Find the sorted insertion slot, shifting larger entries up.  IDA (0x417050) walks
    // from the LAST entry downward and decrements the slot index (v12) once per shift:
    //   cursor = &entry[count-1];  slot = count;
    //   while (1) { cmp = I_stricmp(cursor,name);
    //              if (cmp==0) dup-error;
    //              if (cmp>=0) { copy cursor up one; cursor-=84; if(--slot<=0) break; continue; }
    //              break; }                       // cmp<0 → insert at slot (unchanged)
    // The final slot is v12 (slot here).
    int slot = lyrMtlGlob.entryCount;
    if ( lyrMtlGlob.entryCount > 0 )
    {
        uint8_t *cursor = &lyrMtlGlob.Layers[LYR_ENTRY_SIZE * ( lyrMtlGlob.entryCount - 1 )];
        for ( ;; )
        {
            int cmp = I_stricmp( (const char *)cursor, name );
            if ( !cmp )
            {
                MessageBoxA( hWnd,
                             "Invalid name.\n\nA layered material with that name already exists.",
                             "Radiant", MB_ICONEXCLAMATION );
                return;
            }
            if ( cmp >= 0 )
            {
                // cursor entry sorts AFTER name → shift it up one slot, keep scanning down.
                memcpy( cursor + LYR_ENTRY_SIZE, cursor, LYR_ENTRY_SIZE );
                cursor -= LYR_ENTRY_SIZE;
                if ( --slot <= 0 )
                    break;
                continue;
            }
            break;   // cursor entry sorts BEFORE name → insert at `slot` (unchanged)
        }
    }

    LyrEntry_t *entry = (LyrEntry_t *)&lyrMtlGlob.Layers[LYR_ENTRY_SIZE * slot];
    memset( entry, 0, LYR_ENTRY_SIZE );
    entry->layerCount = 0;                                   // (IDA dword_1814D44[21*slot])
    strcpy( entry->name, name );

    lyrMtlWndGlob.activeLyrMtl = (int)(intptr_t)entry;
    lyrMtlWndGlob.selectedLayerIndex  = 0;
    SetWindowTextA( lyrMtlWndGlob.hwnd, va( "Editing \"%s\"", (const char *)entry ) );
    LayeredMaterialWnd_SyncToolbar();
    InvalidateRect( lyrMtlWndGlob.layerList, nullptr, FALSE );
    g_nUpdateBits |= 0x10u;   // W_TEXTURE
    ++lyrMtlGlob.entryCount;
}

// ─────────────────────────────────────────────────────────────────────────────
// LayeredMaterials_Save (IDA 0x416f40)
// **DISK-MUTATING** (Directive #4).  If the library's CRC differs from the clean-load
// token (i.e. the operator modified it), write it to the path named by the worldspawn
// "layeredmaterials" project epair, then re-baseline the token.  Returns 1 on success
// or when nothing was modified (the round-trip-gate path: empty library → CRC matches
// the 0 token → no write); 0 on a write failure (which aborts the whole map save).
//
// SAFETY: the CRC gate means this only writes when the operator has actually changed
// the library through the authoring window (which is operator-attended), so in a
// NORMAL (operator) build it writes faithfully.
// ─────────────────────────────────────────────────────────────────────────────
char LayeredMaterials_Save()
{
    // lyrMtlGlob.crcToken is the clean-library CRC token (lyrMtlGlob_crcToken).  When the
    // current CRC matches it, nothing was modified → return 1 without writing.  The
    // headless round-trip gate has an empty library (entryCount==0 → CRC of 0 bytes ==
    // 0 == the initial token) so this is the path it always takes.
    if ( CheckLayeredMaterial_Modifications( lyrMtlGlob.Layers, 84 * lyrMtlGlob.entryCount, 0 )
         != (unsigned)lyrMtlGlob.crcToken )
    {
        // Resolve the destination path from the project's "layeredmaterials" epair.
        const char *value = "";
        epair_t *ep = g_qeglobals.d_project_entity ? g_qeglobals.d_project_entity->epairs : nullptr;
        for ( ; ep; ep = ep->next )
        {
            if ( !_stricmp( ep->key, "layeredmaterials" ) ) { value = ep->value; break; }
        }

        // IDA: f = Map_SaveFileToPerforce(value, 0);
        //      if ( !f || (v5 = WriteFile(f), fclose(f), !v5) ) { error; return 0; }
        FILE *f = Map_SaveFileToPerforce( value, 0 );
        char ok = 0;
        if ( f )
        {
            ok = LayeredMaterials_WriteFile( f );
            fclose( f );
        }
        if ( !f || !ok )
        {
            Sys_Printf( "ERROR!!!! Failed to save layeredmaterials\n" );
            return 0;
        }
        // Re-baseline the clean-library token to the just-written contents.
        lyrMtlGlob.crcToken = (int)CheckLayeredMaterial_Modifications( lyrMtlGlob.Layers,
                                                                 84 * lyrMtlGlob.entryCount, 0 );
    }
    return 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// LayeredMaterials_texcoords (0x417190) deletes an entry after replacing its uses
// with "$default", including referenced prefab maps.
// ─────────────────────────────────────────────────────────────────────────────
extern bool FindReplaceTextures( const char *find, const char *replace, char flags );  // findtexture.cpp 0x493160
extern void FindReplaceVisited_Reset();  // findtexture.cpp (Set_EraseTreeRec + reset sentinels)

void *LayeredMaterials_texcoords( char *a1 )
{
    // (1) reset the prefab-recursion visited set (IDB 0x4171A1: Set_EraseTreeRec(...right)
    //     + reset the 3 RB-tree sentinels + dword_26656B8=0 = g_findReplaceVisited.clear()).
    //     Now a real reset: the visited set is the genuine std::set<std::string> the flag&4
    //     prefab-recursion branch of FindReplaceTexture_Brush consults (findtexture.cpp).
    FindReplaceVisited_Reset();

    // (2) replace all uses of the deleted material with "$default" (flag&4 = recurse prefabs).
    FindReplaceTextures( a1, "$default", 4 );

    // (3) validate the entry index.
    int entryCount = lyrMtlGlob.entryCount;
    unsigned int entryIndex = (unsigned int)( a1 - (char *)lyrMtlGlob.Layers ) / 84u;
    bcassert( entryIndex, (unsigned int)lyrMtlGlob.entryCount );   // LayeredMaterials.cpp:297

    // (4) drop the entry: shrink count, shift the remaining entries down.
    lyrMtlGlob.entryCount = entryCount - 1;
    void *result = memcpy( &lyrMtlGlob.Layers[84 * entryIndex],
                           &lyrMtlGlob.Layers[84 * ( entryIndex + 1 )],
                           84 * ( entryCount - 1 - entryIndex ) );

    // (5) request a redraw.
    g_nUpdateBits |= 0x10;   // W_TEXTURE
    return result;
}
