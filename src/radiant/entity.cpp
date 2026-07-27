#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\entity.cpp — entity DEFs and INSTANCES: epairs, the def/inst brush
// lists, prefab instancing, and the .map entity parse/write halves.

#include "stdafx.h"
#include "qe3.h"
#include <universal/q_parse.h>   // Com_ParseExt, parseInfo_t, Com_GetParseThreadInfo
#include <universal/assertive.h> // iassert (USE_ASSERTS always on; same handler as Assert)

// ─── helpers declared by other Radiant files ─────────────────────────────────
extern void  Assert( const char *file, int line, int type, const char *fmt, ... );
extern int   Sys_Printf( const char *fmt, ... );
extern char *AllocMaterialString( const char *src );
extern void  Error( const char *fmt, ... );    // wraps Com_Error (engine_stubs.cpp)
extern void  MarkMapModified();
extern void  SetupVertexSelection();
extern void  Select_Delete();                   // map.cpp (0x48E760) — delete selected brushes
extern int   UpdateSelection( int wParam, eclass_t *cls );   // win_ent.cpp (0x497180)
namespace SurfaceInspector { void UpdateSurfaceDialog(); }

// ─── globals from engine_stubs.cpp / other radiant files ─────────────────────
extern entity_s     *world_entity;              // engine_stubs.cpp line 576
extern char          g_activeLayer_string[];    // engine_stubs.cpp line 549
extern int           g_nUpdateBits;             // engine_stubs.cpp line 553
extern int           modified;                  // map.cpp (0x23F179C) — dirty flag
extern void         *zero;                      // engine_stubs.cpp (MFC empty string)
extern void         *lpBuffer;                  // engine_stubs.cpp
#include "mainfrm.h"
extern CMainFrame   *g_pParentWnd;             // engine_stubs.cpp

// ─── forward declarations for functions defined later in this file ────────────
struct prefab_s;  // defined further below; forward-declare for use in Prefab_Init decl
void            Entity_Free( char *a1 );
void            Entity_Free_R( entity_s *e );
entity_s       *Prefab_Init( prefab_s *a1, entity_s_def *entDef, selbrush_t *a3 );
void            EntityAssignModel( entity_s_def *a1 );
void            Entity_LinkBrush( brush_t *b, entity_s *world_ent );
void            Entity_UnlinkBrush( brush_t *b );
unsigned int    Entity_ColorSth( brush_t *b_in );
void            Entity_RebuildBounds( entity_s *e );

// ─── declarations for ported-elsewhere functions used here ───────────────────
// brush.cpp
extern brush_t      *Brush_Alloc( const void *planeptsSrc, eclass_t *ecls );
extern void          Brush_Create( float *mins, float *maxs, brush_t *b, eclass_t *ecls );
extern void          Brush_SetLayerString( brush_t *b, const char *str );  // 0x4758A0
extern selbrush_t   *Brush_AddToList( brush_t *b, entity_s *e );
extern void          Brush_AddToList2( selbrush_t *b );
extern void          Brush_Free( selbrush_t *b );
extern void          Brush_Free_R( brush_t *b );
extern void          Brush_BuildWindings( brush_t *b, int rebuild );
extern void          Brush_Build( brush_t *b, float *mins = nullptr, float *maxs = nullptr );
// Brush_Write is defined in brush.cpp as int Brush_Write(WriteWriter_t, brush_t*).
// WriteWriter_t = WriteFunc_t** — see brush.cpp.  Declared here with compatible type.
typedef int (*WriteFunc_entity_t)( int ctx, const char *fmt, ... );
extern int           Brush_Write( WriteFunc_entity_t *writer, brush_t *b );
extern selbrush_t   *Entity_LinkBrush_0_extern( entity_s *e, entity_brush_s *b );
extern void          sub_476330( selbrush_t *b );   // Brush_Deselect_Helper
extern void          sub_476470( selbrush_t *b );   // Brush_Select_Helper
// sub_478630 (0x478630, rotate face planepts about a pivot) is now REAL in brush.cpp
// (Brush_RotateFacePlanepts, file-static).  It is only called there, on the physics
// box/cylinder map-parse path — never from entity.cpp.

// eclass.cpp
extern eclass_t     *Eclass_ForName( int hasBrushes, const char *name );
// Eclass_01 (0x482300): real signature takes the entity-context as an int (a2);
// *(a2+96)=eclass, *(a2+100)=modelClass-out.  (A `entity_s_def*` decl mangled
// differently and didn't link once Eclass_01 was first actually called.)
extern models_t     *Eclass_01( const char *modelName, int entDefCtx );

// ─── epair access ────────────────────────────────────────────────────────────
//     epair list head is at entity offset 0x74; node is
//     epair_t {next@0,key@4,value@8}. The engine `zero` empty-string is stubbed
//     nullptr, so we return a real "" for missing keys (avoids strcmp/sscanf NULL deref).
extern void sub_483500( int listPtr, const char *key, const char *value );  // eclass.cpp key/value setter

static char s_emptyKeyValue[] = "";

// ValueForKey2 (0x4825C0)
// 0x4825c0: epairs@0x74, next@0/key@4/value@8, _stricmp (case-insensitive), not-found returns
// "" (engine zero global; s_emptyKeyValue safer than stubbed null).
char *ValueForKey2( int e, const char *key )
{
    for ( epair_t *ep = *(epair_t **)( (char *)(intptr_t)e + 0x74 ); ep; ep = ep->next )
        if ( !_stricmp( ep->key, key ) )
            return ep->value;
    return s_emptyKeyValue;
}

// Entity_GetVec3ForKey (0x483860) — sscanf "%f %f %f"; returns 1 iff all 3 parsed.
// 0x483860: _stricmp walk, sscanf "%f %f %f"==3, default "" on not-found.
int Entity_GetVec3ForKey( entity_s_def *e, float *out, const char *key )
{
    const char *value = s_emptyKeyValue;
    for ( epair_t *ep = e->epairs; ep; ep = ep->next )
        if ( !_stricmp( ep->key, key ) ) { value = ep->value; break; }
    return sscanf( value, "%f %f %f", out, out + 1, out + 2 ) == 3;
}

// Entity_GetFloatValueForKey (0x4837C0)
// 0x4837c0: _stricmp walk, (float)atof, not-found = atof("")=0.0.
float Entity_GetFloatValueForKey( int e, const char *key )
{
    for ( epair_t *ep = *(epair_t **)( (char *)(intptr_t)e + 0x74 ); ep; ep = ep->next )
        if ( !_stricmp( ep->key, key ) )
            return (float)atof( ep->value );
    return (float)atof( s_emptyKeyValue );
}

// HasKeyValuePair (0x4838B0)
// 0x4838b0: strcmp (case-sensitive) epair-walk; returns bool.
bool HasKeyValuePair( entity_s_def *e, const char *key )
{
    for ( epair_t *ep = e->epairs; ep; ep = ep->next )
        if ( !strcmp( ep->key, key ) )
            return true;
    return false;
}

// Entity_FreeEpairs (0x483BB0) — free the whole epair list and null the head.
// 0x483bb0: next captured pre-free, free key/value/node order, head nulled.
void Entity_FreeEpairs( int e )
{
    epair_t **head = (epair_t **)( (char *)(intptr_t)e + 0x74 );
    for ( epair_t *ep = *head; ep; )
    {
        epair_t *next = ep->next;
        free( ep->key );
        free( ep->value );
        free( ep );
        ep = next;
    }
    *head = nullptr;
}

// DeleteKey (0x483720) — remove the FIRST epair whose key matches (case-sensitive),
// freeing the node + its key/value strings.  The binary passes `(epair_t *)&e->epairs`
// (a fake head node whose `.next` field aliases e->epairs); the equivalent clean form
// walks `&e->epairs` directly.  Only the first match is removed (faithful).
// 0x483720: strcmp (case-sensitive), head-case relink, first-match free key/value/node
// verified.
void DeleteKey( epair_t **head, const char *key )
{
    for ( epair_t **pp = head; *pp; pp = &( *pp )->next )
    {
        epair_t *cur = *pp;
        if ( !strcmp( cur->key, key ) )
        {
            *pp = cur->next;
            free( cur->key );
            free( cur->value );
            free( cur );
            return;
        }
    }
}

// Forward decls for SetKeyValue's side-effect chain (defined later in this file).
void  SetKeyValue( entity_s_def *e, const char *key, const char *value );  // 0x483690 (defined below)
void  Checkkey_Model( entity_s_def *e, const char *key );        // 0x482f70 (model rebuild on key change)
void  MapLoad_ParsePrefab( const char *classname, int a2 );      // 0x483010 (classname re-class handler)
void  Checkkey_Color( entity_s_def *a1, const char *a2 );        // 0x483210
void  Entity_UpdateCylinder( const char *key, int entPtr );      // 0x483320 (stub)

// 0x483440 / 0x4834A0 — radius/height clamps. Only act on cylinder/prefab classes
// (classtype & 0xC0 / & 0x40); for plain fixed-size and brush entities these are
// no-ops. Ported faithfully so SetKeyValue's side-effect chain is complete.
static void Entity_ClampRadius( const char *key, entity_s_def *e )   // sub_483440
{
    if ( !_stricmp( key, "radius" ) && ( e->eclass->classtype & 0xC0 ) != 0 )
    {
        if ( Entity_GetFloatValueForKey( (int)(intptr_t)e, "radius" ) > 0.0f )
            Entity_UpdateCylinder( key, (int)(intptr_t)e );
        else
            SetKeyValue( e, "radius", "256" );
    }
}
static void Entity_ClampHeight( const char *key, entity_s_def *e )   // sub_4834A0
{
    if ( !_stricmp( key, "height" ) && ( e->eclass->classtype & 0x40 ) != 0 )
    {
        if ( Entity_GetFloatValueForKey( (int)(intptr_t)e, "height" ) > 0.0f )
            Entity_UpdateCylinder( key, (int)(intptr_t)e );
        else
            SetKeyValue( e, "height", "128" );
    }
}

// SetKeyValue (0x483690) — set key=value in entity e's epair list (@0x74) AND fire the
// binary's side-effect chain: raw set (SetKeyValue_0 == sub_483500), Checkkey_Model (model
// rebuild on angles/modelscale/model), MapLoad_ParsePrefab on "classname", EntityAssignModel
// on "model", Checkkey_Color, then the radius/height clamps.
void SetKeyValue( entity_s_def *e, const char *key, const char *value )
{
    if ( !e || !key || !*key )
        return;

    sub_483500( (int)(intptr_t)e + 0x74, key, value );   // SetKeyValue_0 — raw set (primary effect)

    Checkkey_Model( e, key );                            // 0x482f70 (model rebuild)
    if ( !_stricmp( key, "classname" ) )
        MapLoad_ParsePrefab( value, (int)(intptr_t)e );  // 0x483010
    if ( !_stricmp( key, "model" ) )
        EntityAssignModel( e );
    Checkkey_Color( e, key );                            // 0x483210
    Entity_ClampRadius( key, e );                        // 0x483440
    Entity_ClampHeight( key, e );                        // 0x4834A0
}

// math (engine or engine_stubs)
extern char          Byte4PackPixelColor( float *rgba, GfxColor *out );  // engine_stubs.cpp returns char
extern void          OrientationConcatenate( const orientation_t *a, const orientation_t *b, orientation_t *out );
extern float        *AnglesToAxis( float *angles, float (*axisOut)[3] );
extern void          Vec3RotateTranspose( vec3_t in, float (*axis)[3], vec3_t out );
extern float         Vec3Normalize_R( float *v );                        // 0x40A5E0 (engine_stubs)
extern void          Vec3Cross( const float *a, const float *b, float *out ); // 0x40A4D0 (out = a × b)
extern void          AxisToAngles( float *angles, float (*axis)[3] );    // 0x4A8A00 (engine_stubs)
// VectorMaxValues (0x4a8240) lives in brush.cpp's file-local `namespace edwind` — NOT a
// global symbol — so it can't be reached via a global extern from here. entity.cpp callers
// (Entity_GetModelInstBounds, Entity_UpdateCylinder) inline the expand-AABB loop instead.

// map load helpers (stubs / unported)
extern void          Map_Free();
extern void          Map_NewMap();
extern void          Map_InitlLayers();
extern void          Pointfile_Clear();
extern void          Layers_SetMapLayers();
extern void          Layers_02();
extern int           LoadFile( const char *filename, void **buf );
extern void          Com_BeginParseSession( const char *name );
// Map header parsers (layers.cpp) — real ports of sub_4861F0 / sub_486280 / sub_42F8A0.
extern int           Map_ReadVersion( const void **text );                 // 0x4861F0
extern void          Map_ParseLayers( const void **text, int skipFlags );  // 0x486280
extern void          Map_ParseEntityLayerKey( const char **text, const char *def, char *out, const char *key ); // 0x42F8A0
extern brush_t      *Brush_Parse( const char **text, int version );

// model-instance helpers (r_ed_scene.cpp — the editor model-inst buffer / model-render epic).
extern int           AddModelToModelInstBuff( XModel *model, float *axis, float scale );
extern void          ModelInstUpdate( int inst, float (*axis)[3], float scale );
extern void          RemoveModelInstFromBuf( int inst );
extern void          Entity_GetModelInstBounds( int inst, float *out_mins, float *out_maxs ); // 0x4FDE10
extern void          OrientationPosToWorldPos( float *out, const float *pos, const orientation_t *orient ); // sub_4BA610
extern void          Brush_AccumulateWorldBounds( int mtx, int b, int mins, int maxs ); // 0x47ABE0

// brushFlags / classtype (local enum).  Bit values are GROUND TRUTH from
// Eclass_InitFromText (eclass.cpp:909) + the IDB:
//   light 0x1, not-worldspawn 0x2, path 0x4, model-class 0x8 (misc_model/script_*/
//   dyn_model), misc_prefab 0x10, node_ 0x20, trigger_radius 0x40, trigger_disk 0x80,
//   reflection_probe 0x100.
// PREFAB-PLACEMENT FIX: CLASS_PREFAB was 0x40 (= trigger_radius!) and CLASS_LIGHT was
// 0x10 (= prefab!).  SetupModelInst gates Entity_InitPrefabInst on `classtype & CLASS_PREFAB`
// — with PREFAB=0x40 a misc_prefab (classtype 0x10) NEVER matched, so it was routed to
// Entity_UpdateModelInst instead and the prefab was never instanced.  The IDB SetupModelInst
// / Entity_InitPrefabInst / Eclass_01 all test `& 0x10`.  Latent until now (both fns stubbed).
enum CLASS_FLAGS
{
    CLASS_LIGHT       = 0x01,
    CLASS_MODELCLASS  = 0x08,
    CLASS_PREFAB      = 0x10,
    CLASS_SCRIPTNODE  = 0x20,
};

// ─── globals defined in this translation unit ────────────────────────────────
// Entity DEFINITION circular doubly-linked list head (sentinel).
// IDB: entities @ 0x23F17A0.
entity_s    entities{};          // 0x23F17A0

// Entity INSTANCE circular list (prefab instance list, headed by entityInsts).
// IDB: entityInsts @ 0x23F1748.
entity_s    entityInsts{};       // 0x23F1748

// Running entity numberId counter. IDB: dword_739DC4 @ 0x739DC4.
int         dword_739DC4 = 0;

// Screen-update guard. IDB: g_bScreenUpdates @ 0x739B0F.
bool        g_bScreenUpdates = true;

// Between-map clipboard flag. IDB: g_bRestoreBetween @ 0x25D5B07.
bool        g_bRestoreBetween = false;

// World-space identity orientation matrix (read-only .data).
// IDB: world_orient_matrix @ 0x6DE290. Defined as local identity here.
float       world_orient_matrix[4][3] = {
    { 0.0f, 0.0f, 0.0f },   // origin
    { 1.0f, 0.0f, 0.0f },   // axis[0]
    { 0.0f, 1.0f, 0.0f },   // axis[1]
    { 0.0f, 0.0f, 1.0f },   // axis[2]
};

// ─── prefab_s struct (IDB: 5 pointer fields = 0x14 bytes head; 0x54 total) ───
struct prefab_s
{
    entity_s    *prev_entity;           // 0x00
    entity_s    *next_entity;           // 0x04
    void        *unk;                   // 0x08
    selbrush_t  *active_brushlist;      // 0x0C   tail sentinel (prev side)
    selbrush_t  *active_brushlist_next; // 0x10   head sentinel (next side)
    char         _pad[0x54 - 0x14];    // 0x14 .. 0x53
};
static_assert(sizeof(prefab_s) == 0x54, "prefab_s");

// ─────────────────────────────────────────────────────────────────────────────
// 0x475110  Entity_ColorSth  (brush.cpp:2167 assert — belongs here per IDB layout)
// Sets the GfxColor of all faces in a brush from the owning entity's _color or eclass color.
// Called after Entity_LinkBrush to paint the brush in the editor.
// IDA: esi = brush_t* (passed as int to avoid include confusion in callers).
// ─────────────────────────────────────────────────────────────────────────────
// 0x475110: _color/eclass-color/default branches, Byte4PackPixelColor(src,dest), face-color
// loop @+228 stride 232, version@0x4E 16-bit ++. brush.cpp:2167 assert KEEP_VERBOSE
// (CROSS_FILE: preserves brush.cpp file/line iassert can't).
unsigned int Entity_ColorSth( brush_t *b_in )
{
    int a1 = (int)(intptr_t)b_in;

    if ( !a1 )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\brush.cpp", 2167, 0, "%s", "b" );

    entity_s_def *entDef = *(entity_s_def **)((char *)b_in + 8); // b->owner

    // IDA passes &rgba[0] to Entity_GetVec3ForKey, which writes rgba[0..2].  ONE float[4],
    // never three loose locals: MSVC may reorder separate locals and the write runs off.
    float rgba[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    eclass_t *eclass = nullptr;
    if ( entDef && ( eclass = entDef->eclass ) != nullptr && *(int *)&eclass->fixedsize )
    {
        if ( HasKeyValuePair( entDef, "_color" ) )
        {
            Entity_GetVec3ForKey( entDef, rgba, "_color" );
        }
        else
        {
            rgba[0] = eclass->color[0];
            rgba[1] = eclass->color[1];
            rgba[2] = eclass->color[2];
            rgba[3] = eclass->unk;
        }
    }

    GfxColor v11;
    Byte4PackPixelColor( rgba, &v11 );

    unsigned int result = 0;
    int faceCount = b_in->faceCount;
    if ( faceCount )
    {
        int v6 = 0;
        do
        {
            // face_t at b_in->faces[result]; GfxColor at face+228
            *(GfxColor *)( (char *)b_in->faces + v6 + 228 ) = v11;
            ++result;
            v6 += 232;  // sizeof(face_t)
        }
        while ( result < (unsigned)faceCount );
    }
    ++b_in->version;
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x483c30  DecRef  (entity.cpp:701)
// Unlinks an entity from the global entities linked list.
// ─────────────────────────────────────────────────────────────────────────────
static void DecRef( entity_s *e )
{
    iassert( e->next && e->prev );
    e->next->prev = e->prev;
    e->prev->next = e->next;
    e->prev = nullptr;
    e->next = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Static helper: frees child entity instances belonging to a prefab.
// (sub_4825F0, 0x4825F0)
// ─────────────────────────────────────────────────────────────────────────────
// 0x4825f0: circular prefab-instance list (next_entity@prefab+0x04) freed to self-sentinel,
// then free(pf), e->prefab@0x48 cleared. No asserts.
void Entity_FreePrefab( entity_s *e )
{
    prefab_s *pf = (prefab_s *)e->prefab;
    if ( !pf )
        return;
    // Free all entity instances in the prefab's own list until only the sentinel remains.
    while ( pf->next_entity != (entity_s *)pf )
        Entity_Free( (char *)pf->next_entity );
    free( pf );
    e->prefab = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x483c80  Entity_Free_R  (entity.cpp:712)
// Frees an entity DEFINITION and all its brush defs.
// ─────────────────────────────────────────────────────────────────────────────
// 0x483c80: oprev-end def-list walk (e->brushes.prev, sentinel &e->def@e+0x08, re-read each
// iter), per-brush unlink (onext->oprev/oprev->onext) + --refCount + zero + Brush_Free_R, then
// Entity_FreeEpairs+free. Asserts entity.cpp-native: 713/724/1208 CONVERTIBLE (vassert), 721
// CONVERTIBLE (iassert b->owner==e); 722 KEEP_VERBOSE (binary string "&e->brushes" = the
// e+0x08 field the port renamed `def` — converting would mis-target e->brushes@0x0C). [in-body
// conversions pending]
void Entity_Free_R( entity_s *e )
{
    iassert( e );
    vassert( (e->refCount == 0), "(e->refCount) = %i", e->refCount );
    if ( e->next )
        DecRef( e );

    // Walk the brush-def list (brush_t circular via onext/oprev). entity_s.def___or_
    // firstActive (at e+0x08) is the circular sentinel; brush_t.onext/oprev (brush+0x04/
    // +0x00) chain the defs. IDA (0x483CDE) re-reads e->brushes.oprev (e+0x0C — the def-
    // list FIRST element / oldest-inserted brush) as the loop variable every iteration,
    // freeing the list from the oprev end; after each unlink the sentinel's onext slot
    // (= e+0x0C) advances to the next first element. (The prior port walked from
    // def (e+0x08, the newest end) instead — it still freed every brush,
    // but diverged from the binary's walk direction and dropped the b->oprev==sentinel and
    // post-decrement refCount==0 asserts, which only make sense from the oprev end.)
    for ( brush_t *b = (brush_t *)e->brushes.prev;
          b != (brush_t *)&e->def;
          b = (brush_t *)e->brushes.prev )
    {
        // 721 LEVEL 1 (disasm 0x483cf7 push 1) — per-iter list invariant; NOT iassert (would downgrade to L0).
        if ( b->owner != e )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 721, 1, "%s", "b->owner == e" );
        // 722 KEEP_VERBOSE: binary string "&e->brushes" is the e+0x08 field the port renamed `def`;
        // converting would mis-target e->brushes@0x0C (the guard correctly uses &e->def = e+0x08).
        if ( (void *)b->oprev != (void *)&e->def )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 722, 1, "%s", "b->oprev == &e->brushes" );
        if ( !b->onext || !b->oprev )
            Com_Error( ERR_FATAL, "Entity_UnlinkBrush: Not currently linked" );
        vassert( (b->refCount > 0), "(b->refCount) = %i", b->refCount );

        // Unlink b from the def-list:
        b->onext->oprev = b->oprev;
        b->oprev->onext = b->onext;
        int newRef = --b->refCount;
        b->oprev  = nullptr;
        b->onext  = nullptr;
        b->owner  = nullptr;
        // 724 LEVEL 1 (disasm 0x483d91 push 1) — post-decrement invariant; NOT vassert (would downgrade to L0).
        // b->refCount == newRef here (post-dec, pre-free); binary prints the post-dec value.
        if ( newRef != 0 )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 724, 1, "%s\n\t(b->refCount) = %i", "(b->refCount == 0)", newRef );
        Brush_Free_R( b );
    }

    Entity_FreeEpairs( (int)(intptr_t)e );
    free( e );
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x484fc0  Entity_LinkBrush  (entity.cpp:1190)
// Links a brush_t DEFINITION into an entity's active brush-def list.
// IDA: eax = brush_t*, edi = entity_s*
// ─────────────────────────────────────────────────────────────────────────────
// 0x484fc0: canonical Entity_LinkBrush (cf brush.cpp Split_LinkCloneToEntity inline). Insert
// at def-list head@&world_ent->def(0x08): onext=&head, oprev=*head, (*head)->next=b, head=b
// (the binary's "writes ->next not ->prev" fix preserved). 1190 (b->refCount>=0) -> vassert
// DONE (verified accurate vs IDA : cond str "(b->refCount >= 0)" + "%s\n\t(b->refCount) = %i"
// fmt + level 0 byte-match).
void Entity_LinkBrush( brush_t *b, entity_s *world_ent )
{
    if ( b->oprev || b->onext )
        Com_Error( ERR_FATAL, "Entity_LinkBrush: Allready linked" );
    vassert( (b->refCount >= 0), "(b->refCount) = %i", b->refCount );
    b->owner = world_ent;
    ++b->refCount;
    // Insert into the def-list. The list HEAD is at &world_ent->def
    // (entity offset 0x08); the per-entity "first element" pointer lives at the
    // overlapping brushes-field offset 0x0C and is updated ONLY on the first insert.
    // IDA (Entity_LinkBrush 0x484FC0) writes old_head->NEXT (offset 0x04), NOT ->prev:
    //   a1->onext = &world_ent->def;  a1->oprev = world_ent->def;
    //   world_ent->def->next = a1;    world_ent->def = a1;
    // On the empty list old_head == sentinel (ent+0x08), so writing (ent+0x08)->next
    // == *(ent+0x0C) sets the brushes.oprev "first" pointer that MapFile_WriteEntity
    // reads. (The prior port wrote ->prev/0x00 here, leaving 0x0C at the sentinel →
    // the def-list looked empty to the writer → zero brushes emitted.)
    b->onext                          = (brush_t *)&world_ent->def;
    b->oprev                          = (brush_t *)world_ent->def;
    ( (entity_s *)world_ent->def )->next = (entity_s *)b;
    world_ent->def                        = (void *)b;
    Entity_ColorSth( b );
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x485020  Entity_UnlinkBrush  (entity.cpp:1208)
// Unlinks a brush_t from its entity's def-list. Decrements refCount.
// IDA: __usercall esi = brush_t*
// ─────────────────────────────────────────────────────────────────────────────
// 0x485020: canonical Entity_UnlinkBrush (cf brush.cpp Entity_UnlinkBrush_def forwarder).
// onext->oprev/oprev->onext unlink + --refCount + zero oprev/onext/owner. 1208 (b->refCount>0)
// -> vassert DONE (verified accurate vs IDA : "(b->refCount > 0)" + value-fmt + level 0 byte-
// match).
void Entity_UnlinkBrush( brush_t *b )
{
    if ( !b->onext || !b->oprev )
        Com_Error( ERR_FATAL, "Entity_UnlinkBrush: Not currently linked" );
    vassert( (b->refCount > 0), "(b->refCount) = %i", b->refCount );
    b->onext->oprev = b->oprev;
    b->oprev->onext = b->onext;
    --b->refCount;
    b->oprev = nullptr;
    b->onext = nullptr;
    b->owner = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x485390  Entity_RebuildBounds  (entity.cpp:1329)
// Rebuilds the AABB brush for a fixed-size entity using eclass mins/maxs + origin.
// ─────────────────────────────────────────────────────────────────────────────
void Entity_RebuildBounds( entity_s *e )
{
    iassert( e );
    eclass_t *ec = e->eclass;
    float mins[3], maxs[3];
    mins[0] = ec->mins[0] + e->origin[0];
    mins[1] = ec->mins[1] + e->origin[1];
    mins[2] = ec->mins[2] + e->origin[2];
    maxs[0] = ec->maxs[0] + e->origin[0];
    maxs[1] = ec->maxs[1] + e->origin[1];
    maxs[2] = ec->maxs[2] + e->origin[2];
    // Get the sole brush def for this fixed-size entity. IDA (0x4853F6) reads
    // e->brushes.oprev (entity+0x0C — the def-list first element), NOT def
    // (entity+0x08). For a fixedsize entity the single bbox brush sits at both, so this is
    // equivalent; use 0x0C to match the binary.
    brush_t *b = (brush_t *)e->brushes.prev;
    Brush_Build( b, mins, maxs );
}

// ─────────────────────────────────────────────────────────────────────────────
// Static helper: check if entity has epair key with specific value (0x483930).
// ─────────────────────────────────────────────────────────────────────────────
// 0x483930: first-key-match short-circuit, value strcmp==0; epairs@0x74, next@0/key@4/value@8.
bool Entity_HasEpairMatch( entity_s *e, const char *key, const char *val )
{
    for ( epair_t *ep = e->epairs; ep; ep = ep->next )
    {
        if ( strcmp( ep->key, key ) == 0 )
            return strcmp( ep->value, val ) == 0;
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Substring-aware epair matchers used by Select_ByKeyValue (select.cpp 0x490C00).
// All eight matchers walk e->epairs; the six below are the substring variants of
// HasKeyValuePair / Entity_HasEpairMatch (which are the exact-match pair).  Each is
// a verbatim port of the IDB sub at the noted address.  NOTE the substring matchers
// keep walking past a key match whose value mismatches (the OR loop condition),
// unlike Entity_HasEpairMatch which short-circuits on the first key hit.
// ─────────────────────────────────────────────────────────────────────────────

// sub_483900 — key is a substring of an epair key (no value test).
// 0x483900: strstr(ep->key, key); epairs@0x74, next@0/key@4.
bool Entity_HasKeySubstr( entity_s_def *e, const char *key )
{
    for ( epair_t *ep = e->epairs; ep; ep = ep->next )
        if ( strstr( ep->key, key ) )
            return true;
    return false;
}

// sub_483AC0 — value exact (no key test).
// 0x483ac0: strcmp(ep->value, val)==0; value@8.
bool Entity_HasValue( entity_s_def *e, const char *val )
{
    for ( epair_t *ep = e->epairs; ep; ep = ep->next )
        if ( !strcmp( ep->value, val ) )
            return true;
    return false;
}

// sub_483B10 — value is a substring of an epair value (no key test).
// 0x483b10: strstr(ep->value, val); value@8.
bool Entity_HasValueSubstr( entity_s_def *e, const char *val )
{
    for ( epair_t *ep = e->epairs; ep; ep = ep->next )
        if ( strstr( ep->value, val ) )
            return true;
    return false;
}

// sub_4839C0 — key is a substring AND value matches exactly.
// 0x4839c0: while(!strstr(key) || strcmp(value)) advance.
bool Entity_HasKeySubstrValue( entity_s_def *e, const char *key, const char *val )
{
    for ( epair_t *ep = e->epairs; ep; ep = ep->next )
        if ( strstr( ep->key, key ) && !strcmp( ep->value, val ) )
            return true;
    return false;
}

// sub_483A20 — key matches exactly AND value is a substring.
// 0x483a20: while(strcmp(key) || !strstr(value)) advance.
bool Entity_HasKeyValueSubstr( entity_s_def *e, const char *key, const char *val )
{
    for ( epair_t *ep = e->epairs; ep; ep = ep->next )
        if ( !strcmp( ep->key, key ) && strstr( ep->value, val ) )
            return true;
    return false;
}

// sub_483A80 — key is a substring AND value is a substring.
// 0x483a80: while(!strstr(key) || !strstr(value)) advance.
bool Entity_HasKeySubstrValueSubstr( entity_s_def *e, const char *key, const char *val )
{
    for ( epair_t *ep = e->epairs; ep; ep = ep->next )
        if ( strstr( ep->key, key ) && strstr( ep->value, val ) )
            return true;
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x482940  Entity_GetOrientationMatrix  (entity.cpp:123)
// Builds a 4x3 rotation matrix for a fixed-size entity from its "angles" and origin.
// ─────────────────────────────────────────────────────────────────────────────
// 0x482940: anglesToAxis(angles, axis+1) fills rotation, axis[0]=origin. Asserts 123/124/125
// -> iassert(ent/ent->eclass/ent->eclass->fixedsize) DONE (param renamed a1->ent; verified
// byte-match vs IDA ); 126 "or" KEEP_VERBOSE (C++ reserved word, port uses `axis`).
// non-static: also called by the prefab ray trace (sub_48D240) in select.cpp.
void Entity_GetOrientationMatrix( entity_s *ent, float (*axis)[3] )
{
    iassert( ent );
    iassert( ent->eclass );
    iassert( ent->eclass->fixedsize );   // binary reads *(int*); bool nonzero-check equivalent
    if ( !axis )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 126, 0, "%s", "or" );   // "or" = C++ reserved word, KEEP_VERBOSE
    float angles[3];
    if ( !Entity_GetVec3ForKey( (entity_s_def *)ent, angles, "angles" ) )
        angles[0] = angles[1] = angles[2] = 0.0f;
    // AnglesToAxis fills axis[1..3] (the rotation part); axis[0] gets origin.
    AnglesToAxis( angles, axis + 1 );
    axis[0][0] = ent->origin[0];
    axis[0][1] = ent->origin[1];
    axis[0][2] = ent->origin[2];
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x482a70  Entity_GetOrientation  (entity.cpp:148)
// ─────────────────────────────────────────────────────────────────────────────
// 0x482a70: getOrientationMatrix + OrientationConcatenate(&v4, orParent, orOut).
// 148/149/150/151 -> iassert(ent/ent->eclass/ent->eclass->fixedsize/orParent) DONE (verified
// byte-match vs IDA ); 152 "or" KEEP_VERBOSE (reserved word, port uses `orOut`).
void Entity_GetOrientation( entity_s_def *ent, orientation_t *orParent, orientation_t *orOut )
{
    iassert( ent );
    iassert( ent->eclass );
    iassert( ent->eclass->fixedsize );   // binary reads *(int*); bool nonzero-check equivalent
    iassert( orParent );
    if ( !orOut )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 152, 0, "%s", "or" );
    orientation_t v4;
    Entity_GetOrientationMatrix( (entity_s *)ent, (float (*)[3])&v4 );
    OrientationConcatenate( &v4, orParent, orOut );
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x482b50  Entity_GetOrientationInverse  (entity.cpp:163)
// ─────────────────────────────────────────────────────────────────────────────
// 0x482b50: getOrientationMatrix + Vec3RotateTranspose(origin, v4.axis, out) + v4.origin-=out
// + OrientationConcatenate. Asserts 163/164/165/166 ->
// iassert(ent/ent->eclass/ent->eclass->fixedsize/orParent) DONE (params renamed
// a1->ent/a2->orParent; verified byte-match vs IDA ); 167 "or" KEEP_VERBOSE (reserved word,
// port uses `a3`).
void Entity_GetOrientationInverse( entity_s_def *ent, orientation_t *orParent, orientation_t *a3 )
{
    iassert( ent );
    iassert( ent->eclass );
    iassert( ent->eclass->fixedsize );   // binary reads *(int*); bool nonzero-check equivalent
    iassert( orParent );
    if ( !a3 )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 167, 0, "%s", "or" );   // "or" = C++ reserved word, KEEP_VERBOSE
    orientation_t v4;
    Entity_GetOrientationMatrix( (entity_s *)ent, (float (*)[3])&v4 );
    float out[3];
    Vec3RotateTranspose( ent->origin, v4.axis, out );
    v4.origin[0] -= out[0];
    v4.origin[1] -= out[1];
    v4.origin[2] -= out[2];
    OrientationConcatenate( &v4, orParent, a3 );
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x482650  Entity_UpdateModelInst  (entity.cpp:65)
// Updates (or creates) the model instance for a fixed-size entity, and (with out-params)
// accumulates its world-space AABB over the 8 local-bounds corners.
// 0x482650: after the line-665 transform
// fix below.  NOT a deferred stub: the whole bounds path is implemented.
// ─────────────────────────────────────────────────────────────────────────────
void Entity_UpdateModelInst( entity_s *ent, float *_org, float *out_maxs, float *out_mins )
{
    iassert( ent );
    entity_s_def *def = (entity_s_def *)ent->def;
    iassert( ent->def );
    entitymodel_t *emc = (entitymodel_t *)def->modelClass;
    if ( !emc )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 67, 0, "%s", "ent->def->modelClass" );
    // IDB: ent->def->modelClass->model->handle.  modelClass (entitymodel_t*) ->model is the
    // models_t* sub-node; its handle (+4) holds the XModel* after Model_load → R_RegisterModel.
    models_t *mc = emc->model;
    if ( !mc )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 68, 0, "%s", "ent->def->modelClass->model" );
    XModel *xmodel = (XModel *)mc->handle;
    if ( !xmodel )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 69, 0, "%s", "ent->def->modelClass->model->handle" );

    // modelscale epair (default 1.0; <= 0 → 1.0).
    float scale = 1.0f;
    for ( epair_t *ep = def->epairs; ep; ep = ep->next )
    {
        if ( _stricmp( ep->key, "modelscale" ) == 0 )
        {
            if ( ep->value )
            {
                float s = (float)atof( ep->value );
                if ( s > 0.0f ) scale = s;
            }
            break;
        }
    }

    // World orientation = the entity's own orientation matrix concatenated with the parent
    // orientation passed in _org (a 12-float orientation_t: origin[3] then axis[3][3]).
    orientation_t entOrient, worldOrient;
    Entity_GetOrientationMatrix( (entity_s *)def, (float (*)[3])&entOrient );
    OrientationConcatenate( &entOrient, (const orientation_t *)_org, &worldOrient );

    if ( ent->modelInst )
        ModelInstUpdate( ent->modelInst, (float (*)[3])&worldOrient, scale );
    else
        ent->modelInst = AddModelToModelInstBuff( xmodel, &worldOrient.origin[0], scale );

    // Bounds out-params (both NULL or both non-NULL).
    if ( (out_maxs == nullptr) != (out_mins == nullptr) )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 91, 0, "%s",
                "(mins == NULL && maxs == NULL) || (mins != NULL && maxs != NULL)" );

    int modelInst = ent->modelInst;
    if ( modelInst && out_maxs && out_mins )
    {
        // Local-space model bounds → world-space AABB over the 8 corners.
        float lmin[3], lmax[3];
        Entity_GetModelInstBounds( modelInst, lmin, lmax );
        out_maxs[0] = out_maxs[1] = out_maxs[2] =  131072.0f;
        out_mins[0] = out_mins[1] = out_mins[2] = -131072.0f;
        for ( int c = 0; c < 8; ++c )
        {
            float corner[3] = { (c & 1) ? lmin[0] : lmax[0],
                                (c & 2) ? lmin[1] : lmax[1],
                                (c & 4) ? lmin[2] : lmax[2] };
            // Binary 0x482889 calls sub_4BA610 (q_shared.cpp:1638 @0x4ba610) -- the INVERSE
            // world->local transform  worldPt[i] = axis_row[i] . (corner - origin) -- NOT the
            // forward OrientationPosToWorldPos (0x4ba430, axis^T*pos + origin) the old stub
            // wrongly called.  (map.cpp inlines the same inverse.)
            const orientation_t *o = (const orientation_t *)_org;
            float rel[3] = { corner[0] - o->origin[0],
                             corner[1] - o->origin[1],
                             corner[2] - o->origin[2] };
            float worldPt[3] = {
                o->axis[0][0]*rel[0] + o->axis[0][1]*rel[1] + o->axis[0][2]*rel[2],
                o->axis[1][0]*rel[0] + o->axis[1][1]*rel[1] + o->axis[1][2]*rel[2],
                o->axis[2][0]*rel[0] + o->axis[2][1]*rel[1] + o->axis[2][2]*rel[2] };
            // VectorMaxValues(pt, out_maxs, out_mins) inlined (out_maxs holds the running
            // MIN, out_mins the running MAX — the swapped-name convention sub 0x482650 uses
            // when it passes maxs as the "mins" arg).
            for ( int k = 0; k < 3; ++k )
            {
                if ( out_maxs[k] > worldPt[k] ) out_maxs[k] = worldPt[k];   // running min
                if ( out_mins[k] < worldPt[k] ) out_mins[k] = worldPt[k];   // running max
            }
        }
        // Pad/clamp per axis (out_maxs holds the running MIN, out_mins the running MAX -- the
        // swapped convention).  The binary's verbose assert here ("maxs[axis] >= mins[axis]",
        // type-0) fires ONLY on a genuinely inverted box (running MIN > running MAX), NOT on
        // every model -- VERIFIED vs disasm 0x4828c2 (fcompp; jnp skips when maxs <= mins; the
        // port condition out_mins[axis] < out_maxs[axis] is identical).  The message string is
        // inconsistent with the checked condition, so it can't be iassert/vassert 1:1 -> KEEP.
        for ( int axis = 0; axis < 3; ++axis )
        {
            if ( out_mins[axis] < out_maxs[axis] )   // true MAX < true MIN => genuinely inverted
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 107, 0, "%s", "maxs[axis] >= mins[axis]" );
            if ( out_mins[axis] - out_maxs[axis] <= 8.0f )
            {
                float mid = ( out_mins[axis] + out_maxs[axis] ) * 0.5f;
                out_mins[axis] = mid + 4.0f;     // mins (MAX) := mid+4, maxs (MIN) := mid-4
                out_maxs[axis] = mid - 4.0f;     // verbatim from sub 0x482650 (swapped names)
            }
        }
    }
}

// Model_SetModel (brush.cpp/0x478780) — assigns/loads the model for a brush's owning
// entity.  Entity_InitPrefabInst calls it ONLY to recurse into a child instance whose
// own eclass is a prefab/model class (nested prefabs).  (Still a documented no-op until
// the model-render epic; for plain brush prefabs the recursion branch is not taken.)
extern bool Model_SetModel( entity_brush_s *b, int orientMatrix );

// ─────────────────────────────────────────────────────────────────────────────
// 0x482c60  Entity_InitPrefabInst
// Build the prefab_s for a misc_prefab entity and populate it with brush INSTANCES of
// every entity-def in the loaded prefab model, then compute the placed prefab's overall
// bounds (out_mins/out_maxs) which SetupModelInst feeds to Brush_Build for the bbox.
//
//   ent       = the misc_prefab entity instance
//   identMtx  = world orientation matrix (forwarded to Model_SetModel for nested prefabs)
//   out_mins/out_maxs = OUT: the placed prefab's world-space bounding box
//
// The prefab model was loaded into ent->def->modelClass->x88 (a models_t sub-node) by
// Model_SetModel→Eclass_01→Eclass_LoadModel→Prefab_Load→Eclass_RealizeModel.  Its loaded
// entity-defs form a circular list: sentinel at subNode+8 (subNode->x2), first at
// subNode->entities_next (subNode+0xC), each linked via entity_s.next (+4).  Prefab_Init
// (already ported) instances one def's brushes into the prefab_s active list.
// 0x482c60: CLASS_PREFAB gate = classtype
// & 0x10; prefab_s 0x54 self-sentinel alloc; asserts 183/187/188/205 CONVERTED (187/188 via
// out_mins/out_maxs->mins/maxs rename), 184-186 KEEP_VERBOSE (ent->def->modelClass void*-chain).
// ─────────────────────────────────────────────────────────────────────────────
// (Port out-params out_mins/out_maxs renamed mins/maxs to match the binary's identifiers
// so the line-187/188 iasserts below stringize byte-identically to the originals "mins"/"maxs".)
void Entity_InitPrefabInst( entity_s *ent, int identMtx, float *mins, float *maxs )
{
    iassert( ent );
    entity_s_def *def = (entity_s_def *)ent->def;
    iassert( ent->def );
    models_t *modelClass = (models_t *)def->modelClass;
    if ( !modelClass )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 184, 0, "%s", "ent->def->modelClass" );
    models_t *model = (models_t *)modelClass->x88;          // +0x160 = loaded sub-node
    if ( !model )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 185, 0, "%s", "ent->def->modelClass->model" );
    if ( !model->entities_next__substruct )                 // +0xC = entities.next
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 186, 0, "%s", "ent->def->modelClass->model->entities.next" );
    iassert( mins );
    iassert( maxs );
    iassert( ent->prefab == NULL );

    // Allocate + init the prefab_s (0x54), with both entity-list and brush-list as
    // empty circular sentinels (IDA: next/prev = self; active_brushlist = &active_brushlist).
    prefab_s *pf = (prefab_s *)operator new( 0x54u );
    ent->prefab = pf;
    memset( pf, 0, 0x54u );
    pf->next_entity           = (entity_s *)pf;             // +0x04 = self
    pf->prev_entity           = (entity_s *)pf;             // +0x00 = self
    pf->active_brushlist_next = (selbrush_t *)&pf->active_brushlist; // +0x10 = &(+0x0C)
    pf->active_brushlist      = (selbrush_t *)&pf->active_brushlist; // +0x0C = self

    // Walk the loaded prefab's entity-defs and instance each (Prefab_Init links the
    // brush instances into pf->active_brushlist).  Circular list: sentinel = &model->x2
    // (model+8), first = model->entities_next (model+0xC), advance via entity_s.next (+4).
    entity_s_def *sentinel = (entity_s_def *)( (char *)model + 8 );
    for ( entity_s_def *entDef = (entity_s_def *)model->entities_next__substruct;
          entDef != sentinel;
          entDef = entDef->next )
    {
        entity_s *inst = Prefab_Init( pf, entDef, (selbrush_t *)&pf->active_brushlist );

        // If the instanced child is itself a prefab/model class, recurse via Model_SetModel
        // on its (single) brush instance (nested prefabs).  (IDA tests classtype & 0x10.)
        if ( ( ( (entity_s_def *)inst->def )->eclass->classtype & CLASS_PREFAB ) != 0 )
        {
            entity_brush_s *b = (entity_brush_s *)inst->brushes.ownerNext;
            iassert( b );
            if ( b != (entity_brush_s *)&inst->brushes )
                Model_SetModel( b, identMtx );
        }
    }

    // Compute the placed prefab's world bounds: orient by THIS entity's matrix, then
    // accumulate every instanced brush's def windings (sub_47ABE0).
    float orientMtx[4][3];
    Entity_GetOrientationMatrix( (entity_s *)def, orientMtx );

    mins[0] = mins[1] = mins[2] =  131072.0f;
    maxs[0] = maxs[1] = maxs[2] = -131072.0f;

    for ( selbrush_t *bi = pf->active_brushlist_next;
          bi != (selbrush_t *)&pf->active_brushlist;
          bi = bi->next )
    {
        Brush_AccumulateWorldBounds( (int)(intptr_t)orientMtx, (int)(intptr_t)bi->def,
                                     (int)(intptr_t)mins, (int)(intptr_t)maxs );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x483010  MapLoad_ParsePrefab  (IDB name "Checkkey_Model"; entity.cpp:285)
// Classname-change handler: re-class a fixed-size entity and (re)build its model +
// bbox brush.  Called by SetKeyValue when "classname" changes.  NB the un-ported
// function was 0x483010 (the COMPLEX handler); 0x482f70 is the SIMPLE epair handler,
// ported above as Checkkey_Model.  Inert on Entity_Create/Map_New (eclass still NULL
// at the classname set).  IDA __fastcall(ecx=classname, edx=entity); the port keeps
// (classname, a2) so the SetKeyValue call site is already wired.
//
// Disasm field map (esi = entity_s_def* a2):
//   [esi+0x60]=eclass  [esi+0x0C]=brushes.prev (the single bbox brush)
//   [esi+0x64]=modelClass  [esi+0x68/6C/70]=origin  [esi+0x78]=def version (16-bit)
//   [ebx+0x08]=v4->fixedsize  [ebx+0x04]=v4->name  [ebx+0x0C/0x18]=v4->mins/maxs
//   [brush+0x4C]=unk01 (LOBYTE=modelFailed) [brush+0x4E]=brush version (16-bit)
// ─────────────────────────────────────────────────────────────────────────────
void MapLoad_ParsePrefab( const char *classname, int a2 )
{
    entity_s_def *e = (entity_s_def *)(intptr_t)a2;

    // 0x483027/2d/33: bail unless the entity already has a FIXED-SIZE eclass.
    eclass_t *cur = e->eclass;                            // [esi+0x60]
    if ( !cur )
        return;
    if ( !*(int *)&cur->fixedsize )                      // cmp dword ptr [eax+8],0 → eclass->fixedsize@+8 (32-bit)
        return;

    // 0x483044: look up the eclass for the NEW classname (no-brushes variant).
    eclass_t *v4 = Eclass_ForName( 0, classname );        // ebx = v4
    if ( !*(int *)&v4->fixedsize )                        // 0x483049: only fixed-size classes get a bbox brush
        return;

    // 0x483053..0x48307e: if NEITHER the current eclass NOR the new eclass is "misc_prefab",
    // it's a plain re-class — set the new eclass and let EntityAssignModel rebuild.
    if ( I_stricmp( e->eclass->name, "misc_prefab" ) && I_stricmp( v4->name, "misc_prefab" ) )
    {
        e->eclass = v4;                                  // 0x483081: [esi+0x60] = v4
        EntityAssignModel( e );                          // 0x483084
    }
    else
    {
        // 0x48308E: a model/prefab class — resolve the model name epair.
        char *modelName = ValueForKey2( (int)(intptr_t)e, "model" );
        if ( !modelName )                                // 0x48309e
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 292, 0, "%s", "modelName" );

        // 0x4830BD..0x4830FD: for misc_model / script_model whose model isn't already a .map,
        // synthesise the prefab path "prefabs/misc_models/<name>.map", store it back, and
        // refresh the selection (UpdateSelection(-1, 0)).
        if ( ( !I_stricmp( e->eclass->name, "misc_model" ) || !I_stricmp( e->eclass->name, "script_model" ) )
             && !strstr( modelName, ".map" ) )
        {
            char modelPath[516];                         // [ebp-208h]
            _snprintf( modelPath, 0x200u, "prefabs/misc_models/%s.map", modelName );  // 0x483111
            SetKeyValue( e, "model", modelPath );        // 0x483123
            modelName = ValueForKey2( (int)(intptr_t)e, "model" );                    // 0x483132
            UpdateSelection( 0xFFFFFFFF, 0 );            // 0x48313e (wParam=-1, cls=0)
        }

        // 0x483143..0x483157: assign the new eclass, clear modelClass + the brush's
        // model-failed flag, then (re)build the model entity class via Eclass_01.
        brush_t *brush = (brush_t *)e->brushes.prev;     // [esi+0x0C]
        e->eclass      = v4;                             // [esi+0x60] = v4
        e->modelClass  = nullptr;                        // [esi+0x64] = 0
        *(unsigned __int8 *)&brush->unk01 = 0;           // mov byte ptr [eax+4Ch],0 (LOBYTE = modelFailed)
        Eclass_01( modelName, (int)(intptr_t)e );        // 0x483157
    }

    // 0x48315C (loc_48315C): bump the def version and rebuild the bbox brush around
    // the new eclass mins/maxs offset by the entity origin.
    ++*(unsigned __int16 *)&e->version_prob_wrong;       // add word ptr [esi+78h],1

    float bmins[3];                                      // var_214 (v10) — disasm "mins" arg
    float bmaxs[3];                                      // var_220 (v9)  — disasm "maxs" arg
    bmins[0] = e->origin[0] + v4->mins[0];               // 0x483165..0x483175
    bmins[1] = v4->mins[1] + e->origin[1];               // 0x483181..0x483187
    bmins[2] = v4->mins[2] + e->origin[2];               // 0x48318d..0x483193
    bmaxs[0] = e->origin[0] + v4->maxs[0];               // 0x483199..0x48319f
    bmaxs[1] = v4->maxs[1] + e->origin[1];               // 0x4831a5..0x4831ab
    bmaxs[2] = v4->maxs[2] + e->origin[2];               // 0x4831b1..0x4831bb

    brush_t *brush = (brush_t *)e->brushes.prev;         // 0x4831b7: esi = [esi+0x0C]
    Brush_Create( bmins, bmaxs, brush, v4 );             // 0x4831c1 (mins, maxs, brush, eclass)
    Brush_BuildWindings( brush, 1 );                     // 0x4831c9 (bsnap=1)

    if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();                          // 0x4831df

    MarkMapModified();                                   // 0x4831e4
    ++*(unsigned __int16 *)&brush->version;              // add word ptr [esi+4Eh],1
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x483210  Checkkey_Color  (entity.cpp:340)
// Called when an entity epair changes; refreshes brush color if "_color" or "targetname".
// 0x483210: scriptnode gate = classtype
// & 0x20; asserts 344/345/346 KEEP_VERBOSE (binary uses field name "onext" vs port "prev").
// ─────────────────────────────────────────────────────────────────────────────
void Checkkey_Color( entity_s_def *a1, const char *a2 )
{
    if ( _stricmp( a2, "_color" ) && _stricmp( a2, "targetname" ) )
        return;

    eclass_t *ec = a1->eclass;
    if ( !*(int *)&ec->fixedsize )
        return;

    // The sole brush for a fixed-size entity is the first def in the def-list. IDA
    // (0x48324A) reads a1->brushes.oprev (entity+0x0C), not def (0x08);
    // for a single-brush fixedsize entity they coincide. Three asserts in the binary:
    // !oprev (344), oprev==sentinel (345), oprev->onext != sentinel (346).
    brush_t *b = (brush_t *)a1->brushes.prev;
    if ( !b )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 344, 0, "%s", "ent->brushes.onext" );
    if ( b == (brush_t *)&a1->def )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 345, 0, "%s", "ent->brushes.onext != &ent->brushes" );
    if ( b && (void *)b->onext != (void *)&a1->def )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 346, 0, "%s", "ent->brushes.onext->onext == &ent->brushes" );

    if ( !_stricmp( a2, "_color" ) )
    {
        Entity_ColorSth( b );
        return;
    }
    if ( ( ec->classtype & CLASS_SCRIPTNODE ) != 0 )
    {
        if ( strncmp( ec->name, "node_scr", 8 ) != 0 )
        {
            char *tn = ValueForKey2( (int)(intptr_t)a1, "targetname" );
            if ( tn && strncmp( tn, "auto", 4 ) != 0 )
                Entity_ColorSth( b );
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x483320  Entity_UpdateCylinder  (entity.cpp:374)  AUDIT: full trace vs IDA (2026-06-20)
// -- FAITHFUL.  classtype & 0x80 byte-mask (trigger_disk); assert 374 KEEP_VERBOSE (binary
// "stricmp" vs port "_stricmp").
// When "radius" or "height" changes on a cylinder/prefab entity (classtype & 0xC0,
// gated by Entity_ClampRadius/Height), recomputes the entity brush's bounds so the
// 2D/camera bbox tracks the cylinder cap radius + height. The brush winding is rebuilt
// first (Brush_BuildWindings), then the AABB is EXPANDED to enclose the two cylinder
// corners: XY = origin ± radius, Z = origin.z + eclass->mins[2] .. + height.
//
// Faithful to the disasm (0x483320): entPtr is the entity_s_def*. Field offsets:
//   [a2+0x0C] = e->brushes.oprev (the entity's single brush DEF)   — Brush_BuildWindings arg
//   [a2+0x60] = e->eclass ;  [eclass+0x180] = classtype ; [eclass+0x14] = eclass->mins[2]
//   [a2+0x68/6C/70] = origin[0..2] ;  brush def +0x20 = mins, +0x2C = maxs (VectorMaxValues)
//
// The binary's lower corner is a float[3] the decompiler split into v7[2]+v8
// (v7[2] / v8 share ebp-0Ch); both VectorMaxValues
// calls pass &corner[0] over 3 floats. VectorMaxValues is namespace-local to brush.cpp
// (edwind::), so the expand is INLINED here (matching the Entity_GetModelInstBounds port
// above). VectorMaxValues only ever shrinks mins / grows maxs — it expands the
// post-winding bbox, it does not reset it.
// ─────────────────────────────────────────────────────────────────────────────
void Entity_UpdateCylinder( const char *key, int entPtr )
{
    if ( _stricmp( key, "radius" ) && _stricmp( key, "height" ) )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 374, 0,
                "%s", "!stricmp( key, \"radius\" ) || !stricmp( key, \"height\" )" );

    entity_s_def *e = (entity_s_def *)(intptr_t)entPtr;

    float radius = Entity_GetFloatValueForKey( entPtr, "radius" );
    if ( radius <= 0.0f )                                    // 0x483387: only act on a real radius
        return;

    // height: classtype & 0x80 (trigger_disk, the FLAT disk) => fixed 32.0; otherwise
    // (trigger_radius, classtype 0x40) read the "height" key.
    // The disasm is `test byte [eclass+0x180], 80h` — bit 0x80 of the LOW byte of
    // classtype, NOT the int sign bit.  Hex-rays renders it as `*(char*)(...) < 0`, which
    // a literal port would turn into bit 31.  0x80 == trigger_disk, 0x40 == trigger_radius.
    float height;
    if ( ( e->eclass->classtype & 0x80 ) != 0 )              // 0x483390: trigger_disk => fixed height
        height = 32.0f;                                      // flt_6F430C
    else
    {
        height = Entity_GetFloatValueForKey( entPtr, "height" );
        if ( height <= 0.0f )                                // 0x4833B2: bail on non-positive height
            return;
    }

    // [a2+0x0C] = entity_s.brushes first link = the entity's first brush DEF. In the kisak
    // selbrush_t layout this is `brushes.prev`, treated as a brush_t* (matching the gate-proven
    // Map_ImportBuffer idiom `(brush_t *)eDef->brushes.prev`). A trigger_radius/disk owns exactly
    // one brush, so this is the cylinder geometry brush.
    brush_t *brush = (brush_t *)e->brushes.prev;             // [a2+0x0C] — the cylinder brush DEF
    Brush_BuildWindings( brush, 1 );                         // 0x4833C8: rebuild winding + base bounds

    const float baseZ = e->eclass->mins[2] + e->origin[2];   // [eclass+0x14] + origin.z
    const float lower[3] = { e->origin[0] - radius, e->origin[1] - radius, baseZ };
    const float upper[3] = { e->origin[0] + radius, e->origin[1] + radius, baseZ + height };

    // VectorMaxValues(corner, &brush->mins, &brush->maxs): expand AABB to enclose corner.
    for ( int i = 0; i < 3; ++i )
    {
        if ( brush->mins[i] > lower[i] ) brush->mins[i] = lower[i];
        if ( brush->maxs[i] < lower[i] ) brush->maxs[i] = lower[i];
    }
    for ( int i = 0; i < 3; ++i )
    {
        if ( brush->mins[i] > upper[i] ) brush->mins[i] = upper[i];
        if ( brush->maxs[i] < upper[i] ) brush->maxs[i] = upper[i];
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x482f70  Checkkey_Model (IDB "Checkkey_Model_0") — an entity epair changed; if the key
// affects model geometry, trigger a rebuild.  The binary bumps the 16-bit DEF version @0x78
// (add word ptr [esi+78h],1), NOT the 32-bit version @0x4C.  (The COMPLEX classname handler
// 0x483010, IDB "Checkkey_Model", is a DIFFERENT function — MapLoad_ParsePrefab above.)
// ─────────────────────────────────────────────────────────────────────────────
void Checkkey_Model( entity_s_def *e, const char *key )
{
    iassert( e );
    eclass_t *ec = e->eclass;
    if ( ec && *(int *)&ec->fixedsize )
    {
        if ( !_stricmp( key, "angles" ) || !_stricmp( key, "modelscale" ) )
        {
            Entity_RebuildBounds( (entity_s *)e );
            ++*(unsigned __int16 *)&e->version_prob_wrong;   // IDA add word ptr [esi+78h],1 (def ver @0x78,16-bit) -- NOT version@0x4C
        }
    }
    if ( !_stricmp( key, "model" ) )
    {
        e->modelClass = nullptr;
        // IDA (Checkkey_Model_0 0x482FF0) reads e->brushes.oprev (entity+0x0C — the def-list
        // FIRST element), NOT def (entity+0x08), and clears LOBYTE(brush+0x4C)
        // (the model-failed flag) UNCONDITIONALLY. For a fixedsize entity the single bbox brush
        // sits at both 0x08 and 0x0C, so the read is equivalent; we keep the sentinel guard
        // (the binary, when the list is empty, writes a harmless 0 to the sentinel-as-brush).
        brush_t *b = (brush_t *)e->brushes.prev;
        if ( b != (brush_t *)&e->def )
            *(unsigned __int8 *)&b->unk01 = 0;   // IDA mov byte ptr [eax+4Ch],0 -- LOBYTE only (modelFailed)
        ++*(unsigned __int16 *)&e->version_prob_wrong;   // IDA add word ptr [esi+78h],1 -- NOT version@0x4C
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x483e70  ParseEntity — parse one entity block { ... } from the text stream.
// IDA __usercall: eax = entity_s_def*, ecx = const char** (the text cursor).
// SUBSET: the fixed-size model-offset tail is documented at its site below.  hasBrushes /
// empty checks read brushes.prev@0x0C (the binary's field), not def@0x08.
// ─────────────────────────────────────────────────────────────────────────────
entity_s *ParseEntity( const char **text, int version, char a2, char a3 )
{
    ParseThreadInfo *parseInfo = Com_GetParseThreadInfo();
    parseInfo_t     *pi        = &parseInfo->parseInfo[parseInfo->parseInfoNum];

    // Read opening brace. IDA does the unget-restore prologue + Com_ParseExt(text,1),
    // which is exactly Com_Parse — required so the '{' that Map_ParseLayers ungot
    // (after the layer block) is honoured. (Raw Com_ParseExt skips the prologue and
    // would read the token *after* '{'.)
    parseInfo_t *tok = Com_Parse( text );
    if ( !tok->token[0] )
        return nullptr;
    if ( strcmp( tok->token, "{" ) )
        Com_Error( ERR_FATAL, "ParseEntity: { not found" );

    // Allocate entity def (0x8C = 140 bytes)
    char *ent_raw = (char *)operator new( 0x8Cu );
    memset( ent_raw, 0, 0x8Cu );
    entity_s     *ent45 = (entity_s *)ent_raw;
    // Self-initialise the entity unique numberId.  IDA (0x483F17) writes
    // `*((_DWORD*)v8 + 33) = dword_739DC4` — field index 33 == byte offset 0x84
    // (entity_s.pad_0x0080[4]), the same id field undo.cpp matches on (see undo.cpp:676,
    // 972-974). The prior port wrote someCount@0x54 instead, leaving 0x84 zeroed for every
    // map-loaded entity → undo/redo entity-matching mis-identified them (all id 0).
    *(int *)( ent_raw + 0x84 ) = dword_739DC4++;
    // Initialise the brush-def circular list sentinel. IDA sets BOTH the firstActive
    // head (offset 8) AND the embedded brushes node's prev link (offset 0xC) to &head
    // (= ent_raw+8) — an empty circular list. The porter set only offset 8; restore 0xC
    // so functions that read ent->brushes.oprev (Entity_Free/WriteSelected) see the sentinel.
    *(void **)( ent_raw + offsetof( entity_s, def ) ) = ent_raw + offsetof( entity_s, def );
    *(void **)( ent_raw + offsetof( entity_s, brushes ) )             = ent_raw + offsetof( entity_s, def );

    char layerBuf[1028];
    Map_ParseEntityLayerKey( text, "000_Global", layerBuf, "layer" );

    // Parse key/value pairs and brush blocks. IDA reads each key with the
    // unget-restore prologue + Com_ParseExt(text,1) == Com_Parse (honours the
    // unget left by Map_ParseEntityLayerKey and by each prior epair).
    // Epairs are APPENDED (IDA keeps a running tail pointer `origin` starting at
    // &ent->epairs), preserving file order — prepending would reverse them.
    epair_t **epairTail = &ent45->epairs;
    while ( true )
    {
        parseInfo_t *tok2 = Com_Parse( text );
        if ( !tok2->token[0] )
        {
            Error( "ParseEntity: EOF without closing brace" );
            return nullptr;
        }
        if ( !strcmp( tok2->token, "}" ) )
            break;

        if ( !strcmp( tok2->token, "{" ) )
        {
            // Brush block (a patch/mesh block parses into a patch brush too).
            brush_t *b = Brush_Parse( text, version );
            if ( !b )
                break;
            if ( !a3 )
            {
                if ( b->parent_layer_string )
                    free( b->parent_layer_string );
                size_t layerLen = strlen( g_activeLayer_string );
                char  *lc       = (char *)operator new( layerLen + 1 );
                memcpy( lc, g_activeLayer_string, layerLen + 1 );
                b->parent_layer_string = lc;
            }
            Entity_LinkBrush( b, ent45 );
        }
        else
        {
            // tok2->token is the KEY. The value read reuses the SAME parseInfo token
            // buffer (Com_GetParseThreadInfo() returns the same slot), so the key must
            // be copied to its own storage BEFORE reading the value — exactly as IDA
            // does (operator new + memcpy the key, THEN parse the value). The value is
            // read on the same line with the unget prologue == Com_ParseOnLine.
            epair_t *ep = (epair_t *)operator new( sizeof(epair_t) );
            ep->next    = nullptr;
            size_t klen = strlen( tok2->token );
            ep->key     = (char *)operator new( klen + 1 );
            memcpy( ep->key, tok2->token, klen + 1 );

            parseInfo_t *pi3   = Com_ParseOnLine( text );
            const char *valStr = pi3->token;
            size_t vlen = strlen( valStr );
            ep->value   = (char *)operator new( vlen + 1 );
            memcpy( ep->value, valStr, vlen + 1 );

            // Append to epairs list (file order)
            ep->next    = nullptr;
            *epairTail  = ep;
            epairTail   = &ep->next;
        }
    }

    // Check if we should return the raw parsed entity (group_info or force-return flag)
    if ( !strcmp( ValueForKey2( (int)(intptr_t)ent45, "classname" ), "group_info" ) || a2 )
        return ent45;

    bool hasBrushes = ( (void *)ent45->brushes.prev != (void *)&ent45->def );   // IDA reads brushes.oprev@0x0C (not def@0x08)
    Entity_GetVec3ForKey( (entity_s_def *)ent45, ent45->origin, "origin" );
    char     *clsName = ValueForKey2( (int)(intptr_t)ent45, "classname" );
    eclass_t *ec      = Eclass_ForName( hasBrushes ? 1 : 0, clsName );
    ent45->eclass     = ec;

    if ( *(int *)&ec->fixedsize )
    {
        // Fixed-size entity (light/spawn/prefab/...): create the bounding-box brush
        // (#14, IDA ParseEntity 0x483E70 fixed-size tail). The bbox brush is the
        // editor's selectable proxy; it is NEVER written to the .map (MapFile_WriteEntity
        // skips brush output for fixedsize). It IS required so the def-list is non-empty
        // (the Map_SaveFile entity-write gate) and so the per-entity layer can be stored
        // on / read back from the brush (parent_layer_string).
        if ( hasBrushes )
        {
            Sys_Printf( "Warning: Fixed size entity with brushes\n" );
            // IDA (0x484293): a fixedsize entity must own NO parsed brushes — reset the
            // def-list to the empty sentinel (BOTH def@0x08 and
            // brushes.oprev@0x0C point back at &def), discarding the
            // warned brushes so the bbox brush below is the sole def-list element. (The
            // prior port left the stray brushes linked and added the bbox on top.)
            ent45->def = (void *)&ent45->def;
            ent45->brushes.prev         = (selbrush_t *)&ent45->def;
        }

        EntityAssignModel( (entity_s_def *)ent45 );

        float *origin = ent45->origin;
        float  mins[3], maxs[3];
        mins[0] = ec->mins[0] + origin[0];
        mins[1] = ec->mins[1] + origin[1];
        mins[2] = ec->mins[2] + origin[2];
        maxs[0] = ec->maxs[0] + origin[0];
        maxs[1] = ec->maxs[1] + origin[1];
        maxs[2] = ec->maxs[2] + origin[2];

        // KISAK: SUBSET of 0x4842e3-0x48441d.  The binary's fixed-size tail additionally
        //   (1) for CLASS_MODELCLASS (classtype & 8) with a non-empty "model" key, reads the
        //       entity's "angles" (sub_485570 == Entity_GetVec3ForKey(e,out,"angles") with a
        //       zero fill) and calls Eclass_01(modelName, e) to realize the model class;
        //   (2) Init_MaterialLayer(&matdef, flt_6F42F0) before building the bbox brush;
        //   (3) when the angles are non-zero, Brush_RotateFacePlanepts(origin, angles, bbox)
        //       so an angled misc_model gets a ROTATED bbox.
        // All four deps are REAL in the port now (Brush_RotateFacePlanepts brush.cpp,
        // Eclass_01 eclass.cpp, Init_MaterialLayer materialdef.cpp; sub_485570 is 5 lines).
        // The bbox brush of a fixed-size entity is never written to the .map, so this is
        // display-only — but it wants its own porter unit, not a cleanup edit.

        // Build the bounding-box brush = sub_475AC0(&matdef, ec, mins, maxs)
        //   == Brush_Alloc(&matdef, ec) + Brush_Create(mins, maxs, brush, ec).
        MaterialDef matdef;
        memset( &matdef, 0, sizeof( matdef ) );
        matdef.lyrMtl = *(LayerMaterialDef **)&ec->material[0];   // ec->material is a 4-byte ptr field
        matdef.radMtl = (qtexture_s *)ec->textureTableOrSth;

        brush_t *bbox = Brush_Alloc( &matdef, ec );
        Brush_Create( mins, maxs, bbox, ec );
        if ( a3 )
            Brush_SetLayerString( bbox, layerBuf );   // store per-entity layer on the brush
        Entity_LinkBrush( bbox, (entity_s *)ent45 );
        return ent45;
    }

    if ( (void *)ent45->brushes.prev == (void *)&ent45->def )   // IDA reads brushes.oprev@0x0C
        Sys_Printf( "Warning: Brush entity with no brushes\n" );

    return ent45;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x4847f0  Entity_WriteSelected  (entity.cpp:1006)
// Writes a single entity and its selected brushes to the file.
// The MemFile_fprintf handle is a pointer-to-pointer opaque writer (WriteWriter_t = WriteFunc_t**).
// ─────────────────────────────────────────────────────────────────────────────
void Entity_WriteSelected( entity_s_def *world_ent, WriteFunc_entity_t *MemFile_fprintf )
{
    iassert( world_entity );

    // Check if this entity has any selected brushes (or IS the worldspawn def)
    bool isWorldDef = ( world_entity->def == (void *)world_ent );
    if ( !isWorldDef )
    {
        bool found = false;
        for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        {
            if ( b->owner->def == (void *)world_ent )
            {
                found = true;
                break;
            }
        }
        if ( !found )
            return;
    }

    if ( *(int *)&world_ent->eclass->fixedsize )
    {
        char v8[132];
        sprintf( v8, "%.1f %.1f %.1f", world_ent->origin[0], world_ent->origin[1], world_ent->origin[2] );
        SetKeyValue( world_ent, "origin", v8 );
    }

    // MemFile_fprintf is a WriteWriter_t = WriteFunc_entity_t* (ptr-to-fn-ptr).
    // Call through the function pointer, passing the handle as context.
    WriteFunc_entity_t fn = *MemFile_fprintf;
    int ctx = (int)(intptr_t)MemFile_fprintf;

    fn( ctx, "{\n" );
    for ( epair_t *i = world_ent->epairs; i; i = i->next )
        fn( ctx, "\"%s\" \"%s\"\n", i->key, i->value );

    if ( !*(int *)&world_ent->eclass->fixedsize )
    {
        int j = 0;
        for ( selbrush_t *v5 = selected_brushes.next; v5 != &selected_brushes; v5 = v5->next )
        {
            if ( v5->owner->def == (void *)world_ent )
            {
                fn( ctx, "// brush %i\n", j++ );
                Brush_Write( MemFile_fprintf, v5->def );
            }
        }
    }
    fn( ctx, "}\n" );
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x485410  EntityAssignModel  (entity.cpp:1348)
// Reads the "model" epair; if the entity has a default_model_name that differs,
// forces it. Strips the leading "xmodel/" prefix from model keys.
// 0x485410: the version bump is
// the 16-bit DEF version @0x78 (add word [edi+78h],1), NOT version@0x4C; the unk01 clear is
// byte-wide (mov byte [eax+4Ch],bl), not 16-bit. modelName NULL assert (1351) restored.
// ─────────────────────────────────────────────────────────────────────────────
void EntityAssignModel( entity_s_def *a1 )
{
    const char *modelName = "";
    for ( epair_t *ep = a1->epairs; ep; ep = ep->next )
    {
        if ( _stricmp( ep->key, "model" ) == 0 )
        {
            modelName = ep->value;                 // IDA 0x485448: esi = ep->value (may be NULL)
            iassert( modelName );                  // IDA 0x48544f Assert(...,1351,0,"%s","modelName") -- fires on NULL model value
            if ( !modelName ) modelName = "";      // release-safe: binary carries NULL into _stricmp (latent AV)
            break;
        }
    }

    eclass_t *ec = a1->eclass;
    if ( ec && *(int *)&ec->fixedsize &&
         ec->default_model_name && *ec->default_model_name )
    {
        if ( _stricmp( modelName, ec->default_model_name ) )
        {
            SetKeyValue( a1, "model", ec->default_model_name );
            // IDA (0x4854A9): oprev = a1->brushes.oprev (entity+0x0C, the first brush def),
            // then ++version / modelClass=0 / LOBYTE(oprev+0x4C)=0 unconditionally. Read 0x0C
            // (not def@0x08); keep the sentinel guard (the binary's empty-list
            // write is a harmless 0 — see Checkkey_Model above).
            brush_t *b = (brush_t *)a1->brushes.prev;
            if ( b != (brush_t *)&a1->def )
                *(unsigned __int8 *)&b->unk01 = 0;   // IDA 0x4854b7 mov byte ptr [eax+4Ch],bl -- LOBYTE only (high byte 0x4D = live cull flag)
            ++*(unsigned __int16 *)&a1->version_prob_wrong;   // IDA 0x4854af add word ptr [edi+78h],1 -- def ver @0x78 16-bit, NOT version@0x4C
            a1->modelClass = nullptr;
        }
    }
    else if ( I_strnicmp( modelName, "xmodel", 6 ) == 0 )
    {
        char c = modelName[6];
        if ( c == '/' || c == '\\' )
        {
            char *stripped = AllocMaterialString( modelName + 7 );
            SetKeyValue( a1, "model", stripped );
            free( stripped );
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x485510  Entity_SetDefaultModelKey  (sub_485510)
// Tail of CreateEntityFromName's re-class (Path B): after the owning entity has been
// repointed to the new eclass `a2`, set the entity's "model" key to that eclass's
// default_model_name (when the eclass is a fixedsize class WITH a non-empty default
// model name) and refresh the model-render caches.
//
//   a1 = brush_t* (the entity's new bbox brush DEF)   [esi, __usercall@<eax>]
//   a2 = eclass_t* (the new eclass = v21 = Eclass_ForName result)
//
// 0x485510: the "selbrush
// 64B-vs-56B stride trap" deferral was a MISDIAGNOSIS: [esi+8]+0x0C is entity->brushes
// (the embedded brush-list head), whose FIRST field (oprev) is a brush_t_def* — so
// [edx+0x4C] clears brush_t.unk01 (88-byte def, offset 0x4C), NOT a selbrush instance.
// This is the SAME tail EntityAssignModel (0x485410, already ported above) runs:
//   SetKeyValue("model", default) / ++entity->version_prob_wrong (16-bit @0x78) /
//   entity->modelClass = 0 / firstBrushDef->unk01 LOBYTE = 0.
// The gate reads the ENTITY's eclass (brush->owner->eclass) for the fixedsize/non-empty
// check, but the model-name VALUE comes from the passed eclass a2 (default_model_name
// @0x164); at the call site owner->def->eclass was just set to a2, so they are the same
// object.  Match the binary EXACTLY (check via entity eclass, write a2's name).
// ─────────────────────────────────────────────────────────────────────────────
eclass_t *Entity_SetDefaultModelKey( brush_t *a1, eclass_t *a2 )
{
    entity_s_def *v2 = (entity_s_def *)a1->owner;   // [esi+8] = brush->owner (the entity DEF)
    eclass_t *ec = v2->eclass;                       // [ecx+0x60]
    if ( !ec )                                        return ec;
    if ( !*(int *)&ec->fixedsize )                    return ec;          // [eax+8]
    if ( !ec->default_model_name )                    return (eclass_t *)ec->default_model_name;  // [eax+0x164]
    if ( !*ec->default_model_name )                   return (eclass_t *)ec->default_model_name;  // first char

    SetKeyValue( v2, "model", a2->default_model_name );           // value = a2 (new eclass) default model
    ++*(unsigned __int16 *)&v2->version_prob_wrong;              // IDA 0x48554d add word ptr [eax+78h],1
    v2->modelClass = nullptr;                                     // IDA 0x485555 mov dword ptr [eax+64h],0
    // IDA 0x48555c-0x485562: ecx = entity(+8); edx = *(ecx+0x0C) (= brushes.oprev = first brush DEF);
    // mov byte ptr [edx+0x4C],0 -- LOBYTE of brush_t.unk01 (88-byte def @0x4C).  No sentinel guard
    // in the binary here (unlike EntityAssignModel), so port it unconditionally.
    brush_t *firstBrushDef = (brush_t *)v2->brushes.prev;        // entity+0x0C = brushes.oprev (a brush_t_def*)
    *(unsigned __int8 *)&firstBrushDef->unk01 = 0;
    return (eclass_t *)v2;   // IDA returns eax = entity(+8); value unused by the sole caller
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x485630  Prefab_Init  (entity.cpp:1423)
// Creates a new entity INSTANCE from a def, links it into the prefab/entity list,
// and creates brush instances for each brush def owned by entDef.
// instance ver @0x4C seeded from DEF version_prob_wrong @0x78 minus 1, 16-bit (the prior port
// read entDef->version @0x4C, the wrong field). 1427/1428 KEEP_VERBOSE (type-1 + v4-vs-entInst).
// ─────────────────────────────────────────────────────────────────────────────
entity_s *Prefab_Init( prefab_s *a1, entity_s_def *entDef, selbrush_t *a3 )
{
    iassert( entDef );

    entity_s *v4 = (entity_s *)operator new( 0x54u );
    memset( v4, 0, 0x54u );
    v4->def = entDef;

    if ( v4->modelInst )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 1427, 1, "%s", "entInst->modelInst == NULL" );
    if ( v4->prefab )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 1428, 1, "%s", "entInst->prefab == NULL" );

    *(unsigned __int16 *)&v4->version = (unsigned __int16)entDef->version_prob_wrong - 1;   // IDA 0x4856bd mov ax,[ebx+78h]/sub ax,1/mov [esi+4Ch],ax -- DEF ver @0x78 -> instance ver @0x4C low word
    v4->mapLayer = nullptr;

    // Insert into the prefab entity list (via a1 sentinel)
    v4->next                 = (entity_s *)a1;
    v4->prev                 = a1->prev_entity;
    a1->prev_entity->next    = v4;
    a1->prev_entity          = v4;

    // Initialise the embedded brush-instance sentinel
    v4->brushes.ownerNext = (selbrush_t *)&v4->brushes;
    v4->brushes.ownerPrev = (selbrush_t *)&v4->brushes;

    // Walk the entity's brush-DEF list and create an instance per brush.
    // IDA (Prefab_Init 0x485630) + MapFile_WriteEntity both start at brushes.oprev
    // (= our brushes.prev, entity+0x0C — the def-list FIRST element), advance via
    // brush_t.onext, and stop at the sentinel &def (entity+0x08).
    // (The earlier port started at def itself — the list HEAD whose
    // onext jumps straight to the sentinel — so it created only ONE instance regardless
    // of how many brushes the entity owns. Harmless for 1-brush point entities, but it
    // dropped 11 of worldspawn's 12 brushes — the whole map geometry.)
    brush_t *bDef = (brush_t *)entDef->brushes.prev;
    brush_t *sentinel = (brush_t *)&entDef->def;
    while ( bDef != sentinel )
    {
        selbrush_t *v7 = Brush_AddToList( bDef, v4 );
        if ( v7->next || v7->prev )
            Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
        if ( a3 == &selected_brushes )
        {
            Brush_AddToList2( v7 );
        }
        else
        {
            v7->next       = a3->next;
            a3->next->prev = v7;
            a3->next       = v7;
            v7->prev       = a3;
        }
        bDef = bDef->onext;
    }
    ++entDef->refCount;
    return v4;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x485750  Entity_Free  (entity.cpp:1453)
// Frees an entity INSTANCE: unlinks it, removes model instance, frees brush instances,
// decrements def refcount, frees def if last ref.
// IDA: __usercall esi = entity_s*
// 0x485750: 1453/1456/1457 + 1454 CONVERTED;
// 1455 KEEP_VERBOSE (void*-chain (e->def->refCount) vassert).
// ─────────────────────────────────────────────────────────────────────────────
void Entity_Free( char *a1 )
{
    entity_s *e = (entity_s *)a1;
    iassert( e );
    iassert( e->def );
    entity_s_def *def = (entity_s_def *)e->def;
    if ( def->refCount <= 0 )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 1455, 0,
                "%s\n\t(e->def->refCount) = %i", "(e->def->refCount > 0)", def->refCount );
    iassert( e->next );
    iassert( e->prev );

    e->next->prev = e->prev;
    e->prev->next = e->next;

    Entity_FreePrefab( e );

    if ( e->modelInst )
    {
        RemoveModelInstFromBuf( e->modelInst );
        e->modelInst = 0;
    }

    while ( e->brushes.ownerNext != &e->brushes )
        Brush_Free( e->brushes.ownerNext );

    --def->refCount;
    if ( !def->refCount )
        Entity_Free_R( (entity_s *)def );
    free( e );
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x485890  SetupModelInst  (entity.cpp:1480)
// If the entity instance is out-of-date (version mismatch), rebuilds model instance and
// bounding brush.  AUDIT: full trace vs IDA 0x485890 (2026-06-20) -- FAITHFUL after the
// version-field fix (compare/write the DEF's 16-bit version_prob_wrong @0x78, not version@0x4C).
// CLASS_PREFAB gate = classtype & 0x10; asserts 1480/1491/1492 CONVERTED.
// ─────────────────────────────────────────────────────────────────────────────
void SetupModelInst( float *ident_mtx, entity_s *e )
{
    iassert( e );
    entity_s_def *def = (entity_s_def *)e->def;
    // IDA 0x4858c3: mov cx,[edi+4Ch] / cmp cx,[eax+78h] -- compare the INSTANCE version (0x4C)
    // against the DEF's version_prob_wrong (0x78), both 16-bit.  (Prior port compared
    // def->version @0x4C -- the wrong field; after Checkkey_Model's 0x78 bump it would never
    // match, so the model instance would never be rebuilt.)
    if ( (unsigned __int16)e->version == (unsigned __int16)def->version_prob_wrong )
        return;

    Entity_FreePrefab( e );
    if ( e->modelInst )
    {
        RemoveModelInstFromBuf( e->modelInst );
        e->modelInst = 0;
    }

    float v7[3] = {}, v6[3] = {};
    if ( ( def->eclass->classtype & CLASS_PREFAB ) != 0 )
        Entity_InitPrefabInst( e, (int)(intptr_t)ident_mtx, v7, v6 );
    else
        Entity_UpdateModelInst( e, ident_mtx, v7, v6 );

    // Rebuild the sole brush instance.  IDA (0x485924) asserts the instance owns exactly one
    // brush (ownerNext != &brushes && ownerNext == ownerPrev, line 1491) and that brush has a
    // def with refCount>=2 (line 1492), then builds it UNCONDITIONALLY. (The prior port replaced
    // both asserts with silent if-guards that skipped Brush_Build — diverges from the binary in
    // the assert-disabled case.)
    iassert( e->brushes.ownerNext != &e->brushes && e->brushes.ownerNext == e->brushes.ownerPrev );
    selbrush_t *ownerNext = e->brushes.ownerNext;
    brush_t *brushDef = ownerNext->def;
    iassert( e->brushes.ownerNext->def && e->brushes.ownerNext->def->refCount >= 2 );
    Brush_Build( brushDef, v7, v6 );
    e->version = (unsigned __int16)def->version_prob_wrong;   // IDA mov [edi+4Ch],dx ([ecx+78h])
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x4859B0  sub_4859B0 — re-aim an entity's angles onto a target direction `dir`.
// Builds the angles' current FORWARD vector (cos/sin of pitch/yaw), projects it onto the
// plane perpendicular to `dir` (Gram-Schmidt), then assembles a 3x3 axis whose rows are
// [forward', dir × forward', dir] and reads pitch/yaw/roll back out via AxisToAngles.
// The IDB lays forward (axis[3]) and the cross/up rows (v5[6]) at CONTIGUOUS stack slots and
// hands AxisToAngles the address of the first — i.e. one float[3][3].  MSVC may reorder
// separate locals, so this MUST be a single contiguous matrix[9], not three loose vec3s.
static void sub_4859B0( float *angles_out, float *dir )
{
    float matrix[9];   // row0 = forward', row1 = dir × forward', row2 = dir  (contiguous!)

    const double yaw   = angles_out[1] * 0.01745329238474369;
    const float  cy = (float)cos( yaw ),   sy = (float)sin( yaw );
    const double pitch = angles_out[0] * 0.01745329238474369;
    const float  cp = (float)cos( pitch ), sp = (float)sin( pitch );

    matrix[0] = cp * cy;
    matrix[1] = cp * sy;
    matrix[2] = -sp;

    // forward' = forward - dot(dir,forward)*dir  (project onto plane ⟂ dir)
    const float neg = -( dir[0] * matrix[0] + dir[1] * matrix[1] + dir[2] * matrix[2] );
    matrix[0] += dir[0] * neg;
    matrix[1] += dir[1] * neg;
    matrix[2] += dir[2] * neg;
    Vec3Normalize_R( matrix );

    Vec3Cross( dir, matrix, matrix + 3 );   // row1 = dir × forward'
    Vec3Normalize_R( matrix + 3 );

    matrix[6] = dir[0];                      // row2 = dir
    matrix[7] = dir[1];
    matrix[8] = dir[2];

    AxisToAngles( angles_out, (float (*)[3])matrix );
}

// 0x485ad0  AlignEntityToFace_OrientToFloor  (entity.cpp:1519)
// Adjusts the entity's "angles" epair toward a given face direction.
// 0x485ad0: (sub_4859B0 Gram-Schmidt, va %g
// order, SetKeyValue args). 1519 KEEP_VERBOSE (fixedsize dword-read cast pollutes #expr).
// ─────────────────────────────────────────────────────────────────────────────
void AlignEntityToFace_OrientToFloor( entity_s_def *ent, float *dir )
{
    if ( !*(int *)&ent->eclass->fixedsize )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 1519, 0, "%s", "ent->eclass->fixedsize" );
    float angles[3];
    if ( !Entity_GetVec3ForKey( ent, angles, "angles" ) )
        angles[0] = angles[1] = angles[2] = 0.0f;
    sub_4859B0( angles, dir );
    char *v2 = va( "%g %g %g", angles[0], angles[1], angles[2] );
    SetKeyValue( ent, "angles", v2 );
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x484980  Entity_Create  (entity.cpp)
// Creates a new entity from the selected brushes (merge-into-existing OR fresh def+instance OR
// fixedsize point-entity bbox).  AUDIT: full trace vs IDA 0x484980 (2026-06-20) -- verdict
// SUBSET: all flow/offsets/links/version(brush_t.version@0x4E,16-bit)/arg-order FAITHFUL; the
// only divergence is the dropped Init_MaterialLayer(&matdef,0.25f) @0x484b5b in the bbox path
// (cosmetic bbox-texcoord seeding; documented below). 690 CONVERTED; 1160/1166/1167 KEEP_VERBOSE
// (binary names inst/brushInst/e collide with locals -> rename unsafe; type-1 merge-path).
// ─────────────────────────────────────────────────────────────────────────────
entity_s *Entity_Create( eclass_t *eclass )
{
    char      foundNonWorld = 0;       // v3  — a non-world brush was selected
    entity_s *targetDef     = nullptr; // v42 — existing entity def to merge into
    entity_s *ownerInst     = nullptr; // a2  — owner instance / the result

    // ── Scan the current selection ───────────────────────────────────────────
    // A point entity can't be built from existing brushes; brushes of a single
    // non-world entity can be re-classed (merge). World brushes are ignored here —
    // for a point entity, the single selected world brush is the PLACEHOLDER that
    // marks where to drop the entity (consumed by Select_Delete in the tail below).
    if ( selected_brushes.next != &selected_brushes )
    {
        for ( selbrush_t *sb = selected_brushes.next; sb != &selected_brushes; sb = sb->next )
        {
            if ( sb->owner == world_entity )
                continue;

            if ( *(int *)&eclass->fixedsize )
            {
                Sys_Printf( "Entity NOT created, can't make a point entity out of brushes\n" );
                MessageBeep( 0x40u );
                return nullptr;
            }
            if ( foundNonWorld && targetDef != (entity_s *)sb->def->owner )
            {
                Sys_Printf( "Can't merge entities\n" );
                MessageBeep( 0x40u );
                return nullptr;
            }
            entity_s_def *owner = (entity_s_def *)sb->def->owner;
            ownerInst = sb->owner;
            targetDef = (entity_s *)owner;
            if ( !Entity_HasEpairMatch( owner, "classname", eclass->name ) )
            {
                const char *cur = (const char *)zero;
                for ( epair_t *ep = owner->epairs; ep; ep = ep->next )
                    if ( !_stricmp( ep->key, "classname" ) )
                    { cur = ep->value ? ep->value : (const char *)zero; break; }
                Sys_Printf( "Can't change entity type from %s to %s; ungroup first\n",
                            cur, eclass->name );
                MessageBeep( 0x40u );
                return nullptr;
            }
            foundNonWorld = 1;
        }

        if ( foundNonWorld )
        {
            // Re-class the selected brushes into the existing entity `targetDef`
            // (ownerInst = its instance).  Faithful to the binary's loc_484E01 loop:
            // a WORLD brush is reparented into targetDef (inlined Entity_UnlinkBrush +
            // Entity_LinkBrush(def, targetDef) + Entity_LinkBrush_0(ownerInst, inst));
            // a brush ALREADY owned by the target is asserted-consistent and left in
            // place.  (No new entity is allocated here — the selection collapses into
            // the existing one.)
            if ( !ownerInst )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 1160, 1, "%s", "inst" );

            for ( selbrush_t *sb = selected_brushes.next; sb != &selected_brushes; sb = sb->next )
            {
                if ( sb->owner != world_entity )
                {
                    // Already in (the merge target's) instance — verify + skip.
                    if ( sb->owner != ownerInst )
                        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 1166, 1, "%s", "brushInst->owner == inst" );
                    if ( sb->def->owner != targetDef )
                        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\entity.cpp", 1167, 1, "%s", "brushInst->def->owner == e" );
                    continue;
                }
                sub_476330( sb );                            // Brush_Deselect_Helper
                brush_t *bDef = sb->def;
                Entity_UnlinkBrush( bDef );                  // def out of worldspawn's def-list
                Entity_LinkBrush( bDef, targetDef );         // def into targetDef's def-list (+ColorSth)
                Entity_LinkBrush_0_extern( ownerInst, sb );  // instance into ownerInst's owner-chain
                bDef = sb->def;
                Brush_BuildWindings( bDef, 1 );
                if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
                    SetupVertexSelection();
                MarkMapModified();
                ++bDef->version;
                sub_476470( sb );                            // Brush_Select_Helper
            }
            g_nUpdateBits = -1;
            return ownerInst;
        }
    }

    // ── Allocate a fresh entity def + instance (the binary's LABEL_10) ─────────
    entity_s_def *e = (entity_s_def *)operator new( 0x8Cu );
    memset( e, 0, 0x8Cu );
    *(int *)( (char *)e + 0x84 ) = dword_739DC4++;                   // entity numberId (IDA v5[33])
    // empty def-side brush-list sentinel — BOTH the head ptr (+0x08) AND brushes.prev
    // (+0x0C) point at &def (see Map_New / Entity_LinkBrush 0x484fc0).
    e->brushes.prev          = (selbrush_t *)&e->def;
    e->def  = (entity_s *)&e->def;
    SetKeyValue( e, "classname", eclass->name );
    e->eclass = eclass;
    // copy the eclass's default key/values (eclass->xx8 list: [0]=next [4]=key [8]=value)
    for ( int node = eclass->xx8; node; node = *(int *)node )
        sub_483500( (int)e + 0x74, *(const char **)(node + 4), *(const char **)(node + 8) );

    iassert( !e->next && !e->prev );
    e->next             = &entities;                                 // TAIL insertion
    e->prev             = entities.prev;
    entities.prev->next = (entity_s *)e;
    entities.prev       = (entity_s *)e;

    ownerInst = Prefab_Init( (prefab_s *)&entityInsts, e, &active_brushes );

    if ( !*(int *)&eclass->fixedsize )
    {
        // Brush entity (func_*/trigger_*): reparent every selected world brush into the
        // new entity `e`/`ownerInst`.  Faithful to the binary's loc_484CCC loop (the
        // inlined Entity_UnlinkBrush(def) + Entity_LinkBrush(def, e) + Entity_LinkBrush_0
        // (ownerInst, inst)).  With no selection the entity is returned empty (LABEL_73).
        // (The scan above already proved every selected brush is a WORLD brush — a
        // non-world selection would have early-returned via the merge/refuse paths.)
        for ( selbrush_t *sb = selected_brushes.next; sb != &selected_brushes; sb = sb->next )
        {
            sub_476330( sb );                            // Brush_Deselect_Helper
            brush_t *bDef = sb->def;
            Entity_UnlinkBrush( bDef );                  // def out of worldspawn's def-list
            Entity_LinkBrush( bDef, (entity_s *)e );     // def into the new entity's def-list (+ColorSth)
            Entity_LinkBrush_0_extern( ownerInst, sb );  // instance into ownerInst's owner-chain
            bDef = sb->def;
            Brush_BuildWindings( bDef, 1 );
            if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
                SetupVertexSelection();
            MarkMapModified();
            ++bDef->version;
            sub_476470( sb );                            // Brush_Select_Helper
        }
        g_nUpdateBits = -1;
        return ownerInst;
    }

    // ── Point entity: build the bounding-box brush at the placeholder location ──
    // Origin = placeholder mins − eclass mins; the bbox spans the eclass [mins,maxs]
    // placed at that origin (== the proven ParseEntity fixedsize tail above). The
    // origin is written to the def's origin field; MapFile_WriteEntity derives the
    // "origin" epair from it on save (fixedsize entities emit no brush).
    brush_t *refDef = selected_brushes.next->def;
    float origin[3];
    origin[0] = refDef->mins[0] - eclass->mins[0];
    origin[1] = refDef->mins[1] - eclass->mins[1];
    origin[2] = refDef->mins[2] - eclass->mins[2];
    e->origin[0] = origin[0];
    e->origin[1] = origin[1];
    e->origin[2] = origin[2];

    float mins[3], maxs[3];
    mins[0] = eclass->mins[0] + origin[0];
    mins[1] = eclass->mins[1] + origin[1];
    mins[2] = eclass->mins[2] + origin[2];
    maxs[0] = eclass->maxs[0] + origin[0];
    maxs[1] = eclass->maxs[1] + origin[1];
    maxs[2] = eclass->maxs[2] + origin[2];

    // IDA calls Init_MaterialLayer(&matdef, 0.25f) here; omitted (as the proven loader
    // does) — it only seeds the bbox texcoords, which are never emitted for a fixedsize
    // entity. The eclass material (lyrMtl/radMtl) is enough for a valid bbox brush.
    MaterialDef matdef;
    memset( &matdef, 0, sizeof( matdef ) );
    matdef.lyrMtl = *(LayerMaterialDef **)&eclass->material[0];
    matdef.radMtl = (qtexture_s *)eclass->textureTableOrSth;

    brush_t *bbox = Brush_Alloc( &matdef, eclass );
    Brush_Create( mins, maxs, bbox, eclass );
    Entity_LinkBrush( bbox, (entity_s *)e );          // def-side link (refCount 0->1)

    Brush_BuildWindings( bbox, 1 );
    if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
        SetupVertexSelection();
    MarkMapModified();
    ++bbox->version;

    Select_Delete();                                  // consume the placeholder selection
    selbrush_t *inst = Brush_AddToList( bbox, ownerInst );  // instance (refCount 1->2)
    if ( inst->next || inst->prev )
        Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
    Brush_AddToList2( inst );                          // add to the selection list

    g_nUpdateBits = -1;
    return ownerInst;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x486500  Map_LoadEntities  (map.cpp:304 asserts)
// Loads entities from a file into the given doubly-linked entity list.
// 0x486500: TAIL splice, ParseEntity(mapVer,0,1),
// parse-session setup/teardown all match. 304/305 KEEP_VERBOSE (cross-file map.cpp); 690 CONVERTED.
// ─────────────────────────────────────────────────────────────────────────────
int Map_LoadEntities( const char *filename, entity_s *entList, char a3 )
{
    // CROSS_FILE: binary asserts are map.cpp:304/305 (0x486525/0x486547) -- an iassert here would
    // stamp entity.cpp; keep verbose to preserve the binary's __FILE__/__LINE__.
    if ( entList->next != entList )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\map.cpp", 304, 0, "%s", "entList->next == entList" );
    if ( entList->prev != entList )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\map.cpp", 305, 0, "%s", "entList->prev == entList" );

    int   count = 0;
    void *buf   = nullptr;
    if ( LoadFile( filename, &buf ) == -1 )
        goto done;

    {
        Com_BeginParseSession( filename );
        ParseThreadInfo *pi = Com_GetParseThreadInfo();
        pi->parseInfo[pi->parseInfoNum].spaceDelimited  = 0;
        pi->parseInfo[pi->parseInfoNum].negativeNumbers = 1;

        const void *textPtr  = buf;
        int         mapVer   = Map_ReadVersion( &textPtr );
        if ( mapVer >= 0 )
        {
            if ( mapVer < 4 )
                pi->parseInfo[pi->parseInfoNum].spaceDelimited = 1;
            if ( mapVer > 3 )
                Map_ParseLayers( &textPtr, a3 != 0 );

            for ( entity_s *e = ParseEntity( (const char **)&textPtr, mapVer, 0, 1 );
                  e;
                  e = ParseEntity( (const char **)&textPtr, mapVer, 0, 1 ) )
            {
                ++count;
                iassert( !e->next && !e->prev );
                e->next            = entList;
                e->prev            = entList->prev;
                entList->prev->next = e;
                entList->prev       = e;
            }
        }

        // End parse session
        ParseThreadInfo *pi2 = Com_GetParseThreadInfo();
        if ( !pi2->parseInfoNum )
            Com_Error( ERR_FATAL, "Com_EndParseSession: session underflow" );
        --pi2->parseInfoNum;
    }

done:
    free( buf );
    return count;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x4870c0  Map_New  (entity.cpp:690 asserts) — THE File→New WORLDSPAWN INIT.
// Frees the current map, re-inits the sentinel lists, and builds a fresh worldspawn
// entity DEF by hand (the text loader's per-entity build, done without .map text).
// world_entity ends valid so a drag-created brush can Entity_LinkBrush onto it.
// Faithful to the IDB: the entity-list insert is TAIL insertion (the decompiler
// aliases the `entities` global to its .prev field — disasm 0x48717F is
// [v0+4]=&entities / [v0]=entities.prev / entities.prev->next=v0 / entities.prev=v0).
// 0x4870c0: (worldspawn def build, def sentinel
// @0x08/0x0C, TAIL splice, camera/XY reset @CCamWnd+0x58 GUI-guarded). 690 CONVERTED to iassert
// (port var v0 renamed `e` to match the binary so #expr == "!e->next && !e->prev" byte-exact, level 0).
// ─────────────────────────────────────────────────────────────────────────────
void Map_New()
{
    Sys_Printf( "Map_New\n" );
    Map_Free();
    Map_NewMap();
    Pointfile_Clear();
    Map_InitlLayers();
    g_qeglobals.g_layerCount_maybe = 1;
    Sys_Printf( "Updating layers...\n" );
    Layers_SetMapLayers();
    Layers_02();

    // Allocate worldspawn entity def (0x8C = 140 bytes)
    // (Port var v0 renamed `e` to match the binary's identifier so the line-690
    // iassert below stringizes byte-identically to the original "!e->next && !e->prev".)
    entity_s_def *e = (entity_s_def *)operator new( 0x8Cu );
    memset( e, 0, 0x8Cu );
    // Self-initialise the def-side brush-list sentinel. The head lives at
    // def (+0x08); an EMPTY list needs BOTH the head ptr (+0x08)
    // AND brushes.prev (+0x0C, IDA's brushes.oprev) to point at &def — see
    // Entity_LinkBrush (0x484fc0). Disasm 0x48712E-3A: lea eax,[ebx+8];
    // mov [ebx+0Ch],eax; mov [eax],eax. (The prior port omitted the +0x0C write —
    // self-corrected on first insert, but NULL-deref'd a backward walk of the empty
    // list, e.g. saving an empty worldspawn.)
    e->brushes.prev          = (selbrush_t *)&e->def;
    e->def  = (entity_s *)&e->def;

    SetKeyValue( e, "classname", "worldspawn" );
    eclass_t *ec  = Eclass_ForName( 1, "worldspawn" );
    e->eclass    = ec;

    // Link into entities list (circular, TAIL insertion)
    iassert( !e->next && !e->prev );
    e->next              = &entities;
    e->prev              = entities.prev;
    entities.prev->next   = (entity_s *)e;
    entities.prev         = (entity_s *)e;

    entity_s *v3  = Prefab_Init( (prefab_s *)&entityInsts, e, &active_brushes );
    world_entity  = v3;

    // Reset the camera + XY view to the new-map default (faithful 0x4871AB-end;
    // GUI-only — guarded on g_pParentWnd so the headless gate skips it). Camera goes
    // to origin (0,0,48), pitch/yaw 0 (roll untouched); the active XY view recentres.
    if ( g_pParentWnd )
    {
        if ( g_pParentWnd->m_pCamWnd )
        {
            camera_s &cam = g_pParentWnd->m_pCamWnd->camera;
            cam.angles[0] = 0.0f;
            cam.angles[1] = 0.0f;
            cam.origin[0] = 0.0f;
            cam.origin[1] = 0.0f;
            cam.origin[2] = 48.0f;
        }
        if ( g_pParentWnd->m_pXYWnd )
        {
            g_pParentWnd->m_pXYWnd->m_vOrigin[0] = 0.0f;
            g_pParentWnd->m_pXYWnd->m_vOrigin[1] = 0.0f;
            g_pParentWnd->m_pXYWnd->m_vOrigin[2] = 0.0f;
        }
        if ( g_pParentWnd->m_pActiveXY && g_bRestoreBetween )
            g_pParentWnd->m_pActiveXY->Paste();   // carry a selection across File→New
    }

    g_bRestoreBetween = false;
    modified          = 0;     // a fresh map is clean
    g_nUpdateBits     = -1;    // invalidate all views
}

// ─── deps for Map_ImportBuffer (ported elsewhere) ─────────────────────────────
extern void        Select_Deselect( int bDeselectFaces );                  // select.cpp 0x48E800
extern void        Select_Brush( selbrush_t *b, char some_overwrite, char bStatus, char center ); // select.cpp 0x48DCC0
extern void        Undo_ClearRedo();                                       // undo.cpp 0x45DF20
extern void        Undo_GeneralStart( const char *op );                    // undo.cpp 0x45E3F0
extern void        Undo_End();                                             // undo.cpp 0x45EA20
extern int         Map_GetNextAutoTarget();                                // map.cpp 0x487950
extern bool        Model_SetModel( entity_brush_s *b, int orientMatrix );  // brush.cpp 0x478780 (prefab load-on-open)
extern undo_s     *g_lastundo;                                             // undo.cpp 0x23F162C

// ─────────────────────────────────────────────────────────────────────────────
// 0x487c90  Map_ImportBuffer  (map.cpp ~4051 bytes) — the PASTE parse-and-place.
//
// Parses a .map text buffer (produced by Entity_WriteSelected_R into the clipboard)
// back into live brushes/entities and selects them. This is the engine behind
// CXYWnd::Paste and (in the binary) Clone_Selection. The parse machinery is the
// proven ParseEntity / Brush_Parse / Prefab_Init chain (same as Map_LoadEntities).
// 0x487c90: SUBSET: Implemented core (worldspawn
// merge, undo-stamp loops @0x10/0x7C, version bumps brush_t@0x4E 16-bit, classtype & 0x10) all
// FAITHFUL; parked auto-target/targetname/script_link renumber + OLE clipboard documented. map.cpp asserts KEEP_VERBOSE.
//
// Faithful to the IDB control flow (0x487C90):
//   1. Select_Deselect(1); Undo bracket ("import buffer"); d_parsed_brushes = 0.
//   2. For each entity ParseEntity returns:
//        * worldspawn: MERGE its brushes into world_entity's def-list (Entity_Unlink/
//          LinkBrush), rebuild windings, instance (Brush_AddToList) + select
//          (Brush_AddToList2); then Entity_Free_R the now-empty parsed worldspawn def.
//        * other: Prefab_Init it into the entityInsts/active list, splice the DEF into
//          the `entities` list, then SELECT every instance brush (Select_Brush). The
//          undo records get stamped onto the new entity/brushes so the paste undoes.
//   3. Final pass: instance the model for misc_model/prefab entities (classtype&0x10),
//      rebuild windings on the freshly-selected brushes, MarkMapModified, Undo_End.
//
// KISAK, deliberate subset: the auto-target/targetname RENUMBERING (the binary's
// CMapStringToString remap that rewrites pasted target/targetname/script_link* to fresh
// "auto%i" ids via Map_GetNextAutoTarget so a paste-on-top doesn't collide) and the
// script_linkName/script_linkTo link-number renumbering (Map_ParseLinkList @0x48BE20).
// Effect: pasted entities keep their ORIGINAL target/targetname/link ids — identical to
// the existing in-memory Clone_Selection (clones land coincident, references intact).
// All map geometry, epairs, layers, selection, and undo are faithful and round-trippable.
// ─────────────────────────────────────────────────────────────────────────────
char *Map_ImportBuffer( const char **text, int version )
{
    Select_Deselect( 1 );
    Undo_ClearRedo();
    Undo_GeneralStart( "import buffer" );
    g_qeglobals.d_parsed_brushes = 0;

    // (PARKED) auto-target seed + link-number map are built here in the binary; skipped.
    (void)Map_GetNextAutoTarget;

    g_qeglobals.d_num_entities = 0;

    for ( entity_s *ent = ParseEntity( text, version, 0, 0 ); ent; ent = ParseEntity( text, version, 0, 0 ) )
    {
        entity_s_def *eDef = (entity_s_def *)ent;
        undo_s       *u    = g_lastundo;

        // ── Undo-record stamping (IDB 0x488090-0x48810B). Associates the parsed entity
        //    + its brush defs with the current undo record so Undo removes the paste.
        if ( u && !u->done && ent != (entity_s *)world_entity->def )
        {
            eDef->epairEdits = u->id;                                   // entity+0x7C
            brush_t *sentinel = (brush_t *)&eDef->def; // entity+0x08
            for ( brush_t *b = (brush_t *)eDef->brushes.prev; b != sentinel; b = b->onext )
                b->ownerPrev = (entity_s *)(intptr_t)u->id;            // brush+0x10
        }
        {
            brush_t *sentinel = (brush_t *)&eDef->def;
            for ( brush_t *b = (brush_t *)eDef->brushes.prev; b && b != sentinel; b = b->onext )
            {
                if ( u && !u->done )
                {
                    b->ownerPrev = (entity_s *)(intptr_t)u->id;        // brush+0x10
                    entity_s_def *owner = (entity_s_def *)b->owner;    // brush+0x08
                    if ( owner->eclass && *(int *)&owner->eclass->fixedsize )
                    {
                        owner->epairEdits = u->id;                     // owner+0x7C
                        u = g_lastundo;
                    }
                }
            }
        }

        if ( Entity_HasEpairMatch( ent, "classname", "worldspawn" ) )
        {
            // ── WORLDSPAWN MERGE (IDB 0x488127-0x488254). Move every brush DEF from the
            //    parsed worldspawn entity onto the live worldspawn DEF, instance + select
            //    each. The walk captures `next` BEFORE the relink (relink rewrites onext).
            //
            //    instance-vs-def: the brush DEF's owner must be the worldspawn DEF
            //    (world_entity->def), NOT the world_entity INSTANCE —
            //    Entity_LinkBrush links into a DEF's def-list and sets owner=that def, and
            //    Brush_AddToList(def, instance) then asserts def->owner == instance->def
            //    (brush.cpp:2386). Passing world_entity (the instance) sets owner=instance
            //    and trips that assert. The binary's inline relinks into
            //    world_entity->def->def with owner=the DEF.
            entity_s *worldDef = (entity_s *)world_entity->def;
            brush_t  *sentinel = (brush_t *)&eDef->def;
            brush_t  *b        = (brush_t *)eDef->brushes.prev;   // def-list first element
            while ( b != sentinel )
            {
                brush_t *next = b->onext;                        // capture before relink

                Entity_UnlinkBrush( b );                         // off the parsed worldspawn def-list
                Entity_LinkBrush( b, worldDef );                 // onto live worldspawn DEF (owner=DEF, refCount++, Entity_ColorSth)

                g_bScreenUpdates = false;
                Brush_BuildWindings( b, 1 );
                if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
                    SetupVertexSelection();
                ++b->version;
                selbrush_t *inst = Brush_AddToList( b, world_entity );   // instance (refCount->2; owner=world_entity)
                if ( inst->next || inst->prev )
                    Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
                Brush_AddToList2( inst );                                // select it
                g_bScreenUpdates = true;

                b = next;
            }
            Entity_Free_R( ent );                               // drop the now-empty parsed worldspawn def
            continue;
        }

        // ── NON-WORLDSPAWN ENTITY (IDB 0x488279-0x4888DD). Instance it, splice the DEF
        //    into the `entities` list, then select every instance brush.
        entity_s *inst = Prefab_Init( (prefab_s *)&entityInsts, eDef, &active_brushes );

        // (PARKED) target/targetname/script_link* auto-renumber happens here in the binary.

        // Splice the DEF onto the `entities` list (TAIL insert — IDB 0x488894).
        eDef->next            = &entities;
        eDef->prev            = entities.prev;
        entities.prev->next   = ent;
        entities.prev         = ent;
        ++g_qeglobals.d_num_entities;

        // Select every instance brush of this pasted entity (IDB LABEL: 0x4888BE).
        for ( selbrush_t *ib = inst->brushes.ownerNext; ib != &inst->brushes; ib = ib->ownerNext )
            Select_Brush( ib, 1, 0, 0 );
    }

    // ── Final pass A (0x48890F): instance the 3D model for prefab insts (classtype & 0x10).
    for ( entity_s *ii = entityInsts.next; ii != &entityInsts; ii = ii->next )
    {
        if ( !ii )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\map.cpp", 1242, 0, "%s", "entInst" );
        entity_s_def *iDef = (entity_s_def *)ii->def;
        if ( !iDef )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\map.cpp", 1243, 0, "%s", "entInst->def" );
        if ( !iDef->eclass )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\map.cpp", 1244, 0, "%s", "entInst->def->eclass" );
        if ( ( iDef->eclass->classtype & 0x10 ) != 0 )
        {
            entity_brush_s *fb = ii->brushes.ownerNext;
            if ( fb != &ii->brushes )
                Model_SetModel( fb, (int)(intptr_t)&world_orient_matrix );
        }
    }

    // ── Final pass B: rebuild windings on the freshly-selected brushes (IDB 0x4889B8-end).
    //    The binary also dedups a target/targetname pair between two newly-selected ents
    //    when they accidentally collide; that belongs to the PARKED auto-rename path
    //    (no collisions are introduced without renumbering), so only the windings rebuild
    //    + MarkMapModified survive here.
    for ( selbrush_t *sb = selected_brushes.next; sb != &selected_brushes; sb = sb->next )
    {
        brush_t *def = sb->def;
        Brush_BuildWindings( def, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++def->version;
    }

    g_nUpdateBits = -1;
    modified      = 1;
    Undo_End();
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// In-app clipboard (the buffer half of CXYWnd::Copy / Paste).
//
// The binary stores the copied selection in a global CMemFile (g_Clipboard @0x25EB210)
// AND mirrors it to the Win32 OLE clipboard (RegisterClipboardFormatA("RadiantClippings")
// + SetClipboardData) so brushes can be pasted between two running editors. The OLE
// clipboard + CMemFile + CString refcounting is NOT reproduced.  This in-app buffer covers
// the entire single-editor copy/paste round trip; only cross-process paste is lost.
//
// Copy writes the selection to the buffer via Entity_WriteSelected_R (the SAME .map
// writer Map_SaveFile uses); Paste re-parses the buffer through Map_ImportBuffer.
// Reset/Append/Writer are port-authored glue with no IDA target.  Copy core == 0x46E140
// (Entity_WriteSelected_R); Paste core == 0x46E280 (Com_BeginParseSession +
// Map_ImportBuffer(&buf,4) + teardown, "paste from clipboard", spaceDelim=0 / negNum=1).
// ─────────────────────────────────────────────────────────────────────────────
// Writer handle = WriteFunc_entity_t* (pointer to the fn-ptr at slot 0); matches the
// entity.cpp Entity_WriteSelected convention (the fn is called with ctx==the handle).
extern void Entity_WriteSelected_R( WriteFunc_entity_t *writer );   // map.cpp 0x488DF0

// The clipboard text. Heap buffer grown on demand (no std::string in this TU's PCH path).
static char  *s_clipboardBuf = nullptr;
static size_t s_clipboardLen = 0;   // strlen (excludes the NUL)
static size_t s_clipboardCap = 0;

static void RadiantClipboard_Reset()
{
    s_clipboardLen = 0;
    if ( s_clipboardBuf && s_clipboardCap )
        s_clipboardBuf[0] = '\0';
}

static void RadiantClipboard_Append( const char *txt, size_t n )
{
    if ( s_clipboardLen + n + 1 > s_clipboardCap )
    {
        size_t newCap = s_clipboardCap ? s_clipboardCap * 2 : 8192;
        while ( newCap < s_clipboardLen + n + 1 )
            newCap *= 2;
        char *nb = (char *)realloc( s_clipboardBuf, newCap );
        if ( !nb )
            Com_Error( ERR_FATAL, "RadiantClipboard_Append: out of memory" );
        s_clipboardBuf = nb;
        s_clipboardCap = newCap;
    }
    memcpy( s_clipboardBuf + s_clipboardLen, txt, n );
    s_clipboardLen += n;
    s_clipboardBuf[s_clipboardLen] = '\0';
}

// The string-capturing writer. Matches the WriteWriter_t (WriteFunc_t**) contract used by
// Entity_WriteSelected_R / Entity_WriteSelected / Brush_Write: the writer is a 2-slot
// array whose [0] is this format function and whose [1] is the sink; the function is
// invoked with ctx == the writer pointer itself and recovers the sink from writer[1].
static int RadiantClipboard_Writer( int ctx, const char *fmt, ... )
{
    char    line[4096];
    va_list ap;
    va_start( ap, fmt );
    int n = vsnprintf( line, sizeof( line ), fmt, ap );
    va_end( ap );
    if ( n < 0 )
        n = 0;
    if ( (size_t)n >= sizeof( line ) )
        n = (int)sizeof( line ) - 1;
    RadiantClipboard_Append( line, (size_t)n );
    (void)ctx;
    return n;
}

// CXYWnd::Copy core (IDB 0x46E140, minus the OLE mirror). Serialise the current
// selection to the in-app clipboard buffer.
void RadiantClipboard_Copy()
{
    RadiantClipboard_Reset();
    // 2-slot writer object: [0] = the format fn-ptr (recovered by *writer), [1] = sink
    // (unused — our writer appends to the static buffer, not writer[1]).
    WriteFunc_entity_t writer[2];
    writer[0] = &RadiantClipboard_Writer;
    writer[1] = nullptr;
    Entity_WriteSelected_R( writer );   // writer decays to WriteFunc_entity_t* (the handle)
}

// CXYWnd::Paste core (IDB 0x46E280, minus the OLE fetch). Re-parse the in-app clipboard
// buffer and place + select the result. Sets up / tears down the parse session exactly
// like the binary (Com_BeginParseSession + spaceDelimited=0/negativeNumbers=1) so the
// version-4 .map tokens parse identically.
void RadiantClipboard_Paste()
{
    if ( !s_clipboardLen || !s_clipboardBuf )
        return;

    // Map_ImportBuffer consumes the buffer through a moving text pointer (const char**).
    const char *cursor = s_clipboardBuf;

    Com_BeginParseSession( "paste from clipboard" );
    ParseThreadInfo *pi = Com_GetParseThreadInfo();
    pi->parseInfo[pi->parseInfoNum].spaceDelimited  = 0;
    pi->parseInfo[pi->parseInfoNum].negativeNumbers = 1;

    Map_ImportBuffer( &cursor, 4 );

    ParseThreadInfo *pi2 = Com_GetParseThreadInfo();
    if ( !pi2->parseInfoNum )
        Com_Error( ERR_FATAL, "Com_EndParseSession: session underflow" );
    --pi2->parseInfoNum;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x490780  Select_Ungroup  (select.cpp:1719 asserts)
// Ungroups selected non-worldspawn brush entities by relinking brushes to worldspawn.
// 0x490780: after BUG fix: added the gated
// SetupVertexSelection() (sel_vertex/sel_edge) between Brush_BuildWindings and MarkMapModified
// (IDA 0x490972), missing before -> dangling vtx/edge handles after ungroup. ++version brush_t@0x4E
// 16-bit (correct). 6 select.cpp asserts KEEP_VERBOSE (cross-file).
// ─────────────────────────────────────────────────────────────────────────────
LRESULT Select_Ungroup()
{
    selbrush_t *v0 = selected_brushes.next;
    if ( v0 == &selected_brushes )
        return (LRESULT)Sys_Printf( "No grouped entities selected.\n" );

    int numselectedgroups = 0;

    // We need to iterate selected_brushes but it changes as we relink; snapshot owner list.
    // IDA iterates by walking the sentinel forward and tracking the entity being processed.
    selbrush_t *cur  = selected_brushes.next;
    entity_s   *last = nullptr;

    while ( cur != &selected_brushes )
    {
        if ( !cur->owner )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp", 1719, 0, "%s", "sb->owner" );
        if ( !cur->def )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp", 1720, 0, "%s", "sb->def" );
        // IDA (0x4907FC) asserts the instance's owner and its def's owner agree (the def's
        // owner is the entity DEF, reached via the instance's def). Was dropped.
        if ( (void *)cur->owner->def != (void *)cur->def->owner )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp", 1721, 0, "%s", "sb->owner->def == sb->def->owner" );

        entity_s *e      = cur->owner;
        selbrush_t *next = cur->next;

        if ( e != world_entity && !*(int *)&((entity_s_def *)e->def)->eclass->fixedsize )
        {
            // Walk this entity's brush instances, relinking each to worldspawn.
            selbrush_t *bi = e->brushes.ownerNext;
            while ( bi != &e->brushes )
            {
                selbrush_t *biNext = bi->ownerNext;
                // IDA (0x490860) asserts the instance's def->owner == instance->owner->def. Dropped.
                if ( (void *)bi->def->owner != (void *)bi->owner->def )
                    Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp", 1730, 0, "%s", "b->def->owner == b->owner->def" );
                bool wasSelected = ( ( bi->brushFlags & 0x80 ) != 0 );

                if ( wasSelected )
                    sub_476330( bi );

                // Unlink brush def from old entity
                brush_t *bDef = bi->def;
                Entity_UnlinkBrush( bDef );

                // Relink into worldspawn def
                entity_s_def *worldDef = (entity_s_def *)world_entity->def;
                Entity_LinkBrush( bDef, (entity_s *)worldDef );

                // Relink brush instance into worldspawn's instance list
                Entity_LinkBrush_0_extern( world_entity, bi );

                Brush_BuildWindings( bDef, 1 );
                if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
                    SetupVertexSelection();   // IDA 0x490972 -- rebuild d_points/d_edges; missing -> dangling vtx handles after ungroup
                MarkMapModified();
                ++bDef->version;

                if ( wasSelected )
                    sub_476470( bi );

                // IDA (0x49099A) re-asserts def->owner == owner->def after the relink (now both
                // point at the worldspawn DEF). Dropped by the prior port.
                if ( (void *)bi->def->owner != (void *)bi->owner->def )
                    Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp", 1746, 0, "%s", "b->def->owner == b->owner->def" );

                bi = biNext;
            }

            if ( e->brushes.ownerNext != &e->brushes )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\select.cpp", 1748, 1, "%s", "e->brushes.ownerNext == &e->brushes" );

            Entity_Free( (char *)e );
            ++numselectedgroups;
        }

        cur = next;
    }

    if ( numselectedgroups <= 0 )
        return (LRESULT)Sys_Printf( "No grouped entities selected.\n" );

    const char *suffix = ( numselectedgroups == 1 ) ? "y" : "ies";
    LRESULT result     = (LRESULT)Sys_Printf( "Ungrouped %d entit%s.\n", numselectedgroups, suffix );
    g_nUpdateBits      = -1;
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x483de0  Entity_MemorySize  (65 bytes)
// Returns the total heap size of an entity_s_def + all its epairs (key+value+node).
// __usercall: a1@<eax>
// Called from undo.cpp as Entity_MemorySize_sub483DE0.
// 0x483de0: (_msize + epair walk @0x74, no asserts).
// ─────────────────────────────────────────────────────────────────────────────
size_t Entity_MemorySize_sub483DE0( entity_s *a1 )
{
    // IDA: result = _msize(a1); v3 = (void**)a1[29] = a1->epairs
    size_t total = _msize( a1 );
    epair_t *ep = a1->epairs;
    while ( ep )
    {
        total += _msize( ep->key );
        total += _msize( ep->value );
        total += _msize( ep );
        ep = ep->next;
    }
    return total;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x485090  Entity_Clone_sub485090  (330 bytes)
// Allocates a new entity_s_def (0x8C = 140 bytes), copies the source's
// eclass/origin/refCount fields, deep-copies all epairs.
// __usercall: a1@<eax> = entity_s* source (as int)
// Called from undo.cpp as Entity_Clone_sub485090.
// 0x485090: (numberId@0x84, def sentinel @0x08/0x0C,
// straight float origin copies -- no double-read-of-float; epair deep-copy). key/value asserts
// KEEP_VERBOSE (qe3.cpp:56 CROSS_FILE).
// ─────────────────────────────────────────────────────────────────────────────
entity_s *Entity_Clone_sub485090( int a1 )
{
    entity_s *src = (entity_s *)(intptr_t)a1;

    // Alloc new entity_s_def (0x8C = 140 bytes)
    entity_s *clone = (entity_s *)::operator new( 0x8Cu );
    memset( clone, 0, 0x8Cu );

    // Assign unique entity ID.  IDA (0x4850BF) writes `v2[33] = dword_739DC4` —
    // field index 33 == byte offset 0x84 (entity_s.pad_0x0080[4], the numberId), NOT
    // epairEdits@0x7C. (Undo_AddEntity, the only caller, then overwrites BOTH 0x7C and
    // 0x84 from the source — see undo.cpp:675-676 — so this was output-neutral in the
    // undo path, but the prior 0x7C write diverged from the binary and clobbered the id
    // slot a future caller could rely on.)
    *(int *)&clone->pad_0x0080[4] = dword_739DC4++;

    // Init the DEF-side brush-list sentinel. The binary does v2[2]=v2[3]=v2+2, i.e.
    // BOTH def (+0x08) AND brushes.prev (+0x0C) point at
    // &def (the empty-list sentinel; see Map_New / Entity_LinkBrush).
    // The prior port wrongly set the INSTANCE-side ownerNext/ownerPrev (+0x18/+0x1C),
    // leaving the def sentinel NULL — latent until an entity recorded for undo was
    // EVICTED (Undo_FreeFirstUndo): Entity_Free_R read b->owner at NULL+8 → AV. (0x485090)
    clone->def = (entity_s *)&clone->def;
    clone->brushes.prev         = (selbrush_t *)&clone->def;

    // Copy source fields
    clone->eclass  = *(eclass_t **)((char *)src + 96);         // eclass @0x60
    // IDA: *((float*)v2+26..28) = *(float*)(a1+104..112) = origin
    clone->origin[0] = *(float *)((char *)src + 104);
    clone->origin[1] = *(float *)((char *)src + 108);
    clone->origin[2] = *(float *)((char *)src + 112);

    // Deep-copy epairs
    epair_t *srcEp = *(epair_t **)((char *)src + 116); // a1->epairs
    while ( srcEp )
    {
        epair_t *newEp = (epair_t *)::operator new( 0xCu );
        const char *key = srcEp->key;
        if ( !key )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\qe3.cpp",
                    56, 0, "%s", "str" );
        size_t kLen = strlen( key );
        void *kCopy = ::operator new( kLen + 1 );
        memcpy( kCopy, key, kLen + 1 );
        newEp->key = (char *)kCopy;

        const char *val = srcEp->value;
        if ( !val )
            Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\qe3.cpp",
                    56, 0, "%s", "str" );
        size_t vLen = strlen( val );
        void *vCopy = ::operator new( vLen + 1 );
        memcpy( vCopy, val, vLen + 1 );
        newEp->value = (char *)vCopy;

        // Prepend to clone's epair list (IDA: *v8 = v15[29]; v15[29] = v8)
        newEp->next  = clone->epairs;
        clone->epairs = newEp;

        srcEp = srcEp->next;
    }

    return clone;
}

// ═════════════════════════════════════════════════════════════════════════════
//  0x487B10  Entity_NeedsAutoExportId — "is this entity one of the auto-exported
//  placeholder kinds?"  True for a FIXED-SIZE entity whose classname contains "actor"
//  or "misc_turret".  (The binary builds a CString from ValueForKey2 before the two
//  strstr calls; the CString is pure MFC scaffolding around the same char*.)
//
//  0x487C30  Delete_Exportables — Misc→Delete exportables (cmd 206).  Deselect, then
//  select every ACTIVE brush whose owning entity qualifies, and delete the selection.
//  The `i = i->prev` step before Select_Brush is the binary's own: Select_Brush moves
//  the node out of active_brushes, so the loop resumes from the node BEFORE it.
// ═════════════════════════════════════════════════════════════════════════════
extern void Select_Deselect( char bResetMode );                       // select.cpp 0x48E800
extern void Select_Brush( selbrush_t *b, char some_overwrite,
                          char bStatus, char center_grid_on_selection ); // select.cpp 0x48DCC0

bool Entity_NeedsAutoExportId( entity_s_def *def )
{
    if ( !def || !def->eclass || !*(int *)&def->eclass->fixedsize )
        return false;
    const char *cn = ValueForKey2( (int)(intptr_t)def, "classname" );
    if ( !cn )
        return false;
    if ( strstr( cn, "actor" ) )
        return true;
    return strstr( cn, "misc_turret" ) != nullptr;
}

void Delete_Exportables()
{
    Select_Deselect( 1 );
    for ( selbrush_t *i = active_brushes.next; i != &active_brushes; i = i->next )
    {
        if ( !i )
            continue;
        if ( Entity_NeedsAutoExportId( (entity_s_def *)i->owner->def ) )
        {
            selbrush_t *victim = i;
            i = i->prev;                      // 0x487C6C — resume from the node before it
            Select_Brush( victim, 1, 1, 0 );
        }
    }
    Select_Delete();
}

#ifdef RADIANT_SELFTEST
// ─────────────────────────────────────────────────────────────────────────────
// Radiant_TestPlacePrefab — headless prefab_gate driver.
//
// The "model" key has already been set on `entDef` (the misc_prefab entity DEF) to
// `prefabPath` — exactly what the GUI prefab picker's Ed_CommitPickedModel does.  Here
// we drive the LOAD + INSTANCE that Model_SetModel performs on the normal map-load /
// render path (Model_SetModel itself is the model-render epic, still a no-op):
//   Eclass_01(prefabPath, entDef)  — loads the prefab .map into entDef->modelClass via
//                                    Eclass_LoadModel -> Prefab_Load -> Eclass_RealizeModel
//   Entity_RebuildBounds + SetupModelInst(world_orient_matrix, inst)
//                                  — instances every prefab entity-def's brushes via
//                                    Entity_InitPrefabInst (+ sub_47ABE0 bounds).
// Returns the number of brush INSTANCES created in the prefab (>=0), or -1 on failure.
// ─────────────────────────────────────────────────────────────────────────────
extern "C" int Radiant_TestPlacePrefab( void *entInst, const char *prefabPath )
{
    entity_s *inst = (entity_s *)entInst;
    if ( !inst || !prefabPath )
        return -1;
    entity_s_def *def = (entity_s_def *)inst->def;
    if ( !def )
        return -1;

    // Load the prefab model into def->modelClass (the keystone Prefab_Load call path).
    Eclass_01( prefabPath, (int)(intptr_t)def );
    if ( !def->modelClass )
        return -1;                       // prefab .map failed to load / realise

    // Instance it onto this entity (Entity_InitPrefabInst + sub_47ABE0).
    Entity_RebuildBounds( def );
    SetupModelInst( (float *)world_orient_matrix, inst );

    // Count the brush instances now linked into the prefab's active list.
    prefab_s *pf = (prefab_s *)inst->prefab;
    if ( !pf )
        return 0;                        // no prefab_s (empty prefab) — still a pass
    int count = 0;
    for ( selbrush_t *bi = pf->active_brushlist_next;
          bi != (selbrush_t *)&pf->active_brushlist;
          bi = bi->next )
        ++count;
    return count;
}
#endif // RADIANT_SELFTEST
