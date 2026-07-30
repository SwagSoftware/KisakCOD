#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Radiant brush/entity undo and redo. Undo brush lists contain brush_t
// definitions; entity instance lists use entity_s nodes.

#include "stdafx.h"
#include "qe3.h"
#include <cstdlib>
#include <cstring>
#include <ctime>

// ── forward declarations ──────────────────────────────────────────────────────

// Runtime assert (engine_stubs.cpp -- same signature used across all radiant TUs).
extern void Assert( const char *file, int line, int type, const char *fmt, ... );

// prefab_s is defined in entity.cpp; forward-declare here to use in Prefab_Init signature.
struct prefab_s;

// brush.cpp
extern void        Brush_Free_R( brush_t *def );
extern void        Brush_Free( selbrush_t *inst );
extern size_t      Brush_MemorySize( brush_t *def );
extern selbrush_t *Brush_AddToList( brush_t *def, entity_s *owner );
extern void        Brush_AddToList2( selbrush_t *inst );
extern void        Brush_RemoveFromList( selbrush_t *inst );
extern brush_t    *Brush_FullClone_sub475E80( selbrush_t *src );
extern char       *AllocMaterialString( const char *src );

// entity.cpp
extern unsigned int Entity_ColorSth( brush_t *b );
extern void        Entity_Free( char *e );
extern void        Entity_Free_R( entity_s *e );
extern void        Entity_LinkBrush( brush_t *b, entity_s *world_ent );
extern void        Entity_UnlinkBrush( brush_t *def );
extern void        IncRef( entity_s *e, entity_s *list );   // entity.cpp 0x483bf0
extern void        DecRef( entity_s *e );                   // entity.cpp 0x483c30
extern entity_s   *Prefab_Init( prefab_s *instList, entity_s_def *def, selbrush_t *activeList );

// Entity_Clone (sub_485090 @ 0x485090) -- allocs a new entity_s_def, copies epairs.
extern entity_s   *Entity_Clone_sub485090( int srcPtr );
// Entity_MemorySize (sub_483DE0 @ 0x483de0) -- returns _msize(e) + sum of epair strings.
extern size_t      Entity_MemorySize_sub483DE0( entity_s *e );

// engine_stubs.cpp / qe3.cpp / other
extern int         Sys_Printf( const char *fmt, ... );
extern void        MarkMapModified();
// Sys_PrintActiveBrushes (IDB 0x42c640) populates the brush/entity list diagnostics. Used by
// Undo_GeneralStart's head and Undo_End's tail; both call MainFrm_BrushList/EntList here.
extern void        Select_Deselect( int bSilent );
extern void        MainFrm_BrushList( int label, selbrush_t *list );
extern void        MainFrm_EntList( entity_s *list, const char *label );
// va is in q_shared.h (included via stdafx.h)
// entity.cpp global -- screen-update guard
extern bool        g_bScreenUpdates;
// brush.cpp helper stub
extern void        sub_47B940( brush_t *def );

// layers.cpp
extern void        Layers_SetMapLayers();
extern void        Layers_02();

// ── globals ───────────────────────────────────────────────────────────────────
// 0x23F1628
static undo_s  *g_redolist         = nullptr;
// 0x23F15CC
static undo_s  *g_lastredo         = nullptr;
// 0x23F1630
static undo_s  *g_undolist         = nullptr;
// 0x23F162C -- also used by select.cpp via extern
undo_s         *g_lastundo         = nullptr;

// 0x25D5B18
static int      g_undoSize         = 0;
// 0x25D5B1C
static int      g_undoMemorySize   = 0;
// 0x739F6C
static int      g_undoMaxSize      = 64;
// 0x739F70
static int      g_undoMaxMemorySize = 2 * 1024 * 1024;
// 0x739F74
static int      g_undoId           = 1;
// 0x739F78
static int      g_redoId           = 1;

// Accessor for the ON_UPDATE_COMMAND_UI Redo-greying handler (g_lastredo is static).
// True when a redo record is available (matches OnUpdateEditRedo 0x428790).
bool Undo_RedoAvailable() { return g_lastredo != nullptr; }

// ── extern globals from other TUs ────────────────────────────────────────────
extern selbrush_t  active_brushes;     // engine_stubs.cpp (sentinel)
extern selbrush_t  selected_brushes;   // engine_stubs.cpp
extern entity_s    entities;           // entity.cpp
extern entity_s    entityInsts;        // entity.cpp
extern entity_s   *world_entity;       // map.cpp
extern int         g_nUpdateBits;      // engine_stubs.cpp

// ─────────────────────────────────────────────────────────────────────────────
// Wrappers for __usercall functions
// ─────────────────────────────────────────────────────────────────────────────

static inline brush_t *BrushDef_FullClone( selbrush_t *src )
{
    return Brush_FullClone_sub475E80( src );
}

static inline entity_s *EntityDef_Clone( entity_s *src )
{
    return Entity_Clone_sub485090( (int)(intptr_t)src );
}

static inline size_t EntityDef_MemorySize( entity_s *e )
{
    return Entity_MemorySize_sub483DE0( e );
}

// Convenience: cast entity_s_def's def to entity_s*
static inline entity_s *EntityDef_GetDef( entity_s_def *def )
{
    return (entity_s *)def->def;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45de20  Undo_01_NoMoreUndo  (89 bytes)
// Inserts a brush DEF at the HEAD of a brushlist sentinel (via onext/oprev).
// __usercall: a1@<edi>=list, a2@<esi>=brush def to insert.
// ─────────────────────────────────────────────────────────────────────────────
static brush_t *Undo_01_NoMoreUndo( brush_t *list, brush_t *brush )
{
    vassert( (brush->refCount >= 0), "(brush->refCount) = %i", brush->refCount );   // 85
    iassert( brush->owner == NULL );                                                // 86

    ++brush->refCount;
    brush_t *old_next = (brush_t *)list->onext;
    brush->oprev      = list;
    brush->onext      = (brush_t *)old_next;
    old_next->oprev   = brush;
    list->onext       = brush;
    return old_next;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45de80  Undo_Refcount  (134 bytes)
// Decrements refCount and UNLINKS a brush DEF from its current list.
// ─────────────────────────────────────────────────────────────────────────────
static brush_t *Undo_Refcount( brush_t *brush )
{
    vassert( (brush->refCount >= 1), "(brush->refCount) = %i", brush->refCount );   // 98
    --brush->refCount;
    iassert( brush->onext );   // 101
    iassert( brush->oprev );   // 102

    brush_t *next       = (brush_t *)brush->onext;
    ((brush_t *)brush->oprev)->onext = brush->onext;
    next->oprev         = brush->oprev;
    brush->onext        = nullptr;
    brush->oprev        = nullptr;
    return next;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45df20  Undo_ClearRedo  (323 bytes)
// Frees all redo records (brush defs + entity defs + undo_s nodes).
// ─────────────────────────────────────────────────────────────────────────────
void Undo_ClearRedo()
{
    undo_s *redo = g_redolist;
    while ( redo )
    {
        undo_s *nextredo = redo->next;

        // Free all brush defs in this redo's brushlist (brush_t chain via onext)
        brush_t *brush = (brush_t *)redo->brushlist.onext;
        while ( brush != &redo->brushlist )
        {
            iassert( brush );   // 128
            brush_t *pNext = (brush_t *)brush->onext;
            Undo_Refcount( brush );
            vassert( (brush->refCount == 0), "(brush->refCount) = %i", brush->refCount );   // Undo.cpp:131
            Brush_Free_R( brush );
            brush = pNext;
        }

        // Free all entity defs in this redo's entitylist
        entity_s *entity = redo->entitylist.next;
        while ( entity != &redo->entitylist )
        {
            iassert( entity );   // 136
            entity_s *pNext = entity->next;
            iassert( entity->refCount == 1 );   // Undo.cpp:138
            --entity->refCount;
            Entity_Free_R( entity );
            entity = pNext;
        }

        ::operator delete( redo );
        redo = nextredo;
    }
    g_redolist  = nullptr;
    g_lastredo  = nullptr;
    g_redoId    = 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45e070  Undo_Clear  (440 bytes)
// Clears both the undo and redo lists.
// ─────────────────────────────────────────────────────────────────────────────
void Undo_Clear()
{
    Undo_ClearRedo();

    undo_s *undo = g_undolist;
    while ( undo )
    {
        undo_s *nextundo = undo->next;

        // Free all brush defs in this undo's brushlist (brush_t chain via onext)
        brush_t *brush = (brush_t *)undo->brushlist.onext;
        while ( brush != &undo->brushlist )
        {
            iassert( brush );   // 162
            brush_t *bNext = (brush_t *)brush->onext;
            g_undoMemorySize -= (int)Brush_MemorySize( brush );
            Undo_Refcount( brush );
            vassert( (brush->refCount == 0), "(brush->refCount) = %i", brush->refCount );   // Undo.cpp:166
            Brush_Free_R( brush );
            brush = bNext;
        }

        // Free all entity defs in this undo's entitylist
        entity_s *entity = undo->entitylist.next;
        while ( entity != &undo->entitylist )
        {
            iassert( entity );   // 171
            entity_s *eNext = entity->next;
            g_undoMemorySize -= (int)EntityDef_MemorySize( entity );
            iassert( entity->refCount == 1 );   // Undo.cpp:174
            --entity->refCount;
            Entity_Free_R( entity );
            entity = eNext;
        }

        g_undoMemorySize -= 256; // sizeof(undo_s)
        ::operator delete( undo );
        undo = nextundo;
    }

    g_undolist       = nullptr;
    g_lastundo       = nullptr;
    g_undoSize       = 0;
    g_undoMemorySize = 0;
    g_undoId         = 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45e230  Undo_SetMaxSize
// ─────────────────────────────────────────────────────────────────────────────
void Undo_SetMaxSize( int size )
{
    Undo_Clear();
    g_undoMaxSize = 1;
    if ( size >= 1 )
        g_undoMaxSize = size;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45e250  Undo_GetMaxSize
// ─────────────────────────────────────────────────────────────────────────────
int Undo_GetMaxSize()
{
    return g_undoMaxSize;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45e260  Undo_SetMaxMemorySize
// ─────────────────────────────────────────────────────────────────────────────
void Undo_SetMaxMemorySize( int size )
{
    Undo_Clear();
    if ( size < 1024 )
        g_undoMaxMemorySize = 1024;
    else
        g_undoMaxMemorySize = size;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45e280  Undo_GetMaxMemorySize
// ─────────────────────────────────────────────────────────────────────────────
int Undo_GetMaxMemorySize()
{
    return g_undoMaxMemorySize;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45e290  Undo_FreeFirstUndo  (352 bytes)
// Removes and frees the oldest undo from the undo list.
// ─────────────────────────────────────────────────────────────────────────────
void Undo_FreeFirstUndo()
{
    undo_s *undo         = g_undolist;
    undo_s *nextUndo     = undo->next;
    nextUndo->prev       = nullptr;
    g_undolist           = nextUndo;

    // Free all brush defs (brush_t chain via onext; re-read head each iteration
    // because Undo_Refcount removes b from the list before Brush_Free_R)
    brush_t *brush = (brush_t *)undo->brushlist.onext;
    while ( brush != &undo->brushlist )
    {
        iassert( brush );   // 235
        g_undoMemorySize -= (int)Brush_MemorySize( brush );
        iassert( brush->refCount == 1 );   // Undo.cpp:237
        Undo_Refcount( brush );
        vassert( (brush->refCount == 0), "(brush->refCount) = %i", brush->refCount );   // Undo.cpp:239
        Brush_Free_R( brush );
        // Re-read head (Undo_Refcount removes brush, Brush_Free_R frees it)
        brush = (brush_t *)undo->brushlist.onext;
    }

    // Free all entity defs
    entity_s *entity = undo->entitylist.next;
    while ( entity != &undo->entitylist )
    {
        iassert( entity );   // 244
        entity_s *eNext  = entity->next;
        g_undoMemorySize -= (int)EntityDef_MemorySize( entity );
        iassert( entity->refCount == 1 );   // Undo.cpp:247
        --entity->refCount;
        Entity_Free_R( entity );
        entity = eNext;
    }

    g_undoMemorySize -= 256; // sizeof(undo_s)
    ::operator delete( undo );
    --g_undoSize;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45e3f0  Undo_GeneralStart  (407 bytes)
// Allocates a new undo record and appends it to the undo list.
// __usercall: operation@<eax>
// ─────────────────────────────────────────────────────────────────────────────
void Undo_GeneralStart( const char *operation )
{
    char *s1 = (char *)va( "%s - active_brushes", operation );
    MainFrm_BrushList( (int)(intptr_t)s1, &active_brushes );
    char *s2 = (char *)va( "%s - active_brushes", operation );
    MainFrm_BrushList( (int)(intptr_t)s2, &selected_brushes );
    MainFrm_EntList( &entityInsts, operation );

    if ( g_lastundo && !g_lastundo->done )
        Sys_Printf( "Undo_Start: WARNING last undo not finished.\n" );

    undo_s *_undo = (undo_s *)::operator new( 0x100u );
    if ( !_undo )
        return;

    memset( _undo, 0, sizeof(undo_s) );

    // Init brushlist sentinel (brush_t chain, self-pointing)
    _undo->brushlist.onext = (brush_t_def *)&_undo->brushlist;
    _undo->brushlist.oprev = (brush_t_def *)&_undo->brushlist;

    // Init entitylist sentinel
    _undo->entitylist.next = &_undo->entitylist;
    _undo->entitylist.prev = &_undo->entitylist;

    // Link at tail of undo list
    undo_s *prevLast = g_lastundo;
    if ( prevLast )
        prevLast->next = _undo;
    else
        g_undolist = _undo;
    _undo->prev = prevLast;
    _undo->next = nullptr;
    g_lastundo  = _undo;

    _undo->time = (double)clock() / 1000.0;

    int v9 = g_undoId;
    if ( v9 > 2 * g_undoMaxSize || v9 <= 0 )
        v9 = 1;
    _undo->id    = v9;
    g_undoId     = v9 + 1;
    _undo->done  = 0;
    _undo->operation = operation;

    // Reset ownerPrev (undoId) on all active brush defs that already carry this ID
    // IDA: iterates the active display list via selbrush_t.next
    for ( selbrush_t *pInst = active_brushes.next;
          pInst != &active_brushes;
          pInst = pInst->next )
    {
        brush_t *def = pInst->def;
        if ( def->ownerPrev == (entity_s *)(intptr_t)_undo->id )
            def->ownerPrev = nullptr;
    }
    for ( selbrush_t *pInst = selected_brushes.next;
          pInst != &selected_brushes;
          pInst = pInst->next )
    {
        brush_t *def = pInst->def;
        if ( def->ownerPrev == (entity_s *)(intptr_t)_undo->id )
            def->ownerPrev = nullptr;
    }

    // Reset epairEdits on all entities that already carry this ID
    entity_s *pEntity = entities.next;
    while ( pEntity )
    {
        if ( pEntity == &entities )
            break;
        if ( pEntity->epairEdits == _undo->id )
            pEntity->epairEdits = 0;
        pEntity = pEntity->next;
    }

    g_undoMemorySize += 256; // sizeof(undo_s)
    if ( ++g_undoSize > g_undoMaxSize )
        Undo_FreeFirstUndo();
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45e590  Undo_06 / Undo_BrushInUndo  (88 bytes)
// Returns true if a brush DEF is already registered in the current undo.
// __usercall: a1@<esi> = brush def pointer (int)
// IDA: *(DWORD*)(a1+16) == g_lastundo->id  (brush_t.ownerPrev at offset 0x10)
// ─────────────────────────────────────────────────────────────────────────────
static bool Undo_BrushInUndo( brush_t *brush )
{
    undo_s *undo = g_lastundo;              // the binary's local
    iassert( undo );    // Undo.cpp:335
    iassert( brush );   // Undo.cpp:336
    return brush->ownerPrev == (entity_s *)(intptr_t)undo->id;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45e5f0  Undo_06_LastUndo / Undo_EntityInUndo  (99 bytes)
// Returns true if an entity DEF is already registered in the current undo.
// __usercall: a1@<edi> = entity_s_def*
// ─────────────────────────────────────────────────────────────────────────────
static bool Undo_EntityInUndo( entity_s_def *ent )
{
    undo_s *undo = g_lastundo;
    iassert( undo );   // 351
    iassert( ent );    // 352 — #ent byte-matches the embedded "ent"
    return ent->epairEdits == undo->id
        || ent == (entity_s_def *)world_entity->def;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45e660  Undo_Start  (the public Undo_Start2 entry point ~20 bytes)
// Clears redo list, then starts a new undo record.
// ─────────────────────────────────────────────────────────────────────────────
void Undo_Start( const char *operation )
{
    Undo_ClearRedo();
    Undo_GeneralStart( operation );
}

// Forward declaration so Undo_AddBrushList can call Undo_AddEntity below.
void Undo_AddEntity( int a1 );

// ─────────────────────────────────────────────────────────────────────────────
// 0x45e680  Undo_AddBrush  (236 bytes)
// Adds a brush DEF to the current undo record.
// __cdecl: a1 = brush_t* (the brush DEF to save)
// IDA parameter type is entity_brush_s* but it's the same 88-byte brush_t DEF node.
// ─────────────────────────────────────────────────────────────────────────────
void Undo_AddBrush( entity_brush_s *pBrushInst )
{
    // IDA receives the brush DEF (88-byte node) but types it entity_brush_s.
    // Our brush DEF is brush_t (88-byte); cast immediately.
    brush_t *brush = (brush_t *)(intptr_t)pBrushInst;

    iassert( brush );        // 380
    iassert( g_lastundo );   // 381

    // Skip if already in undo (ownerPrev == current undo id)
    if ( Undo_BrushInUndo( brush ) )
        return;

    // Clone the brush def (sub_475E80).
    // Brush_FullClone_sub475E80 is typed entity_brush_s* but actually receives
    // a brush_t DEF pointer — cast matches IDA's own mixed typing.
    brush_t *clone = BrushDef_FullClone( (selbrush_t *)brush );

    // IDA: ownerPrev of source (a1[1].ownerPrev = *(entity_brush_s*)((char*)a1+56)+ownerPrev)
    // a1 is 88-byte brush_t. a1[1] would be second 88-byte element. But IDA uses
    // entity_brush_s (56 bytes) so a1[1].ownerPrev = *(brush at offset 56).ownerPrev.
    // In our type: brush_t has parent_layer_string at offset 0x48 = 72.
    // IDA a1[1].ownerPrev: entity_brush_s* stride 56, [1] = offset 56, ownerPrev = +16 = offset 72.
    // So this reads the parent_layer_string of the source brush (at offset 72 = 0x48).
    char *srcLayerStr = brush->parent_layer_string;  // offset 0x48 = 72

    // IDA 0x45e6fd: the binary inlines Brush_SetLayerString( clone, srcLayerStr ) here —
    // its brush.cpp:2354 head-check lives in the helper.
    extern void Brush_SetLayerString( brush_t *b, const char *str );   // brush.cpp 0x4758A0
    Brush_SetLayerString( clone, srcLayerStr );

    // IDA: v4->unk1 = a1->owner[2].next — the IDB's 64-byte selbrush_t stride lands on
    // owner+132 = the owning entity's numberId (NOT an entity_s-stride index).
    clone->unk1 = brush->owner->numberId;

    // IDA: v4->ownerPrev = a1->ownerPrev; v4->owner = 0;  (the prior port dropped both;
    // clone->owner = 0 in particular keeps Undo_Undo's owner-walk from following a stale
    // pointer.) Set BEFORE linking the clone into the brushlist.
    clone->ownerPrev = brush->ownerPrev;
    clone->owner     = nullptr;

    // Link clone into undo brushlist at head
    Undo_01_NoMoreUndo( &g_lastundo->brushlist, clone );

    // Mark source brush's ownerPrev with current undo ID
    brush->ownerPrev = (entity_s *)(intptr_t)g_lastundo->id;

    // Track memory
    g_undoMemorySize += (int)Brush_MemorySize( clone );
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45e7c0  Undo_AddBrushList  (92 bytes)
// Adds all brush DEFs from a selbrush_t display-list into the current undo.
// __usercall: sb@<edi> = selbrush_t* (list sentinel)
// IDA: iterates via sb->prev (backwards through display list).
// ─────────────────────────────────────────────────────────────────────────────
void Undo_AddBrushList( selbrush_t *sb )
{
    if ( !g_lastundo )
    {
        Sys_Printf( "Undo_AddBrushList: no last undo.\n" );
        return;
    }
    if ( g_lastundo->entitylist.next != &g_lastundo->entitylist )
        Sys_Printf( "Undo_AddBrushList: WARNING adding brushes after entity.\n" );

    // IDA iterates via sb->prev (backwards through the display list)
    for ( selbrush_t *i = sb->prev; i != sb; i = i->prev )
    {
        entity_s_def *owner = (entity_s_def *)i->def->owner;
        if ( *(int *)&owner->eclass->fixedsize )
            Undo_AddEntity( (int)(intptr_t)owner );
        Undo_AddBrush( (entity_brush_s *)i->def );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45e870  Undo_EndBrushList  (64 bytes)
// Stamps the current undo ID into all brush defs in a display list.
// __usercall: brushlist@<esi>
// ─────────────────────────────────────────────────────────────────────────────
void Undo_EndBrushList( selbrush_t *brushlist )
{
    if ( !g_lastundo )
        return;
    if ( g_lastundo->done )
        return;

    for ( selbrush_t *i = brushlist->next; i; i = i->next )
    {
        if ( i == brushlist )
            break;
        brush_t *def = i->def;
        def->ownerPrev = (entity_s *)(intptr_t)g_lastundo->id;
        entity_s *owner = (entity_s *)def->owner;
        if ( *(int *)&owner->eclass->fixedsize )
            owner->epairEdits = g_lastundo->id;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45e8b0  Undo_AddEntity  (212 bytes)
// Adds an entity DEF to the current undo record.
// __usercall: a1@<eax> = entity_s* (as int for __usercall).
// Called from select.cpp as Undo_AddEntity(int entPtr).
// ─────────────────────────────────────────────────────────────────────────────
void Undo_AddEntity( int a1 )
{
    entity_s *entity = (entity_s *)(intptr_t)a1;
    iassert( entity );   // 500

    if ( Undo_EntityInUndo( (entity_s_def *)entity ) )
        return;

    entity_s *clone = EntityDef_Clone( entity );
    // KEEP_VERBOSE: binary passes level=1 (SANITY CHECK) here (push 1 @0x45e901); the
    // iassert/vassert macros hardcode level 0, so converting would drop the level-1 class.
    vassert( (clone->refCount == 0), "(clone->refCount) = %i", clone->refCount );   // Undo.cpp:506
    ++clone->refCount;

    // Save old undo ID from source into clone
    clone->epairEdits = entity->epairEdits;
    clone->numberId   = entity->numberId;

    // Stamp source entity with current undo ID
    entity->epairEdits = g_lastundo->id;

    // Link clone into the undo entitylist
    IncRef( clone, &g_lastundo->entitylist );        // inlined in the binary

    g_undoMemorySize += (int)EntityDef_MemorySize( clone );
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45e990  Undo_AddEntity_W  (76 bytes)
// Adds an entity AND all of its brush defs to the current undo.
// __usercall: a1@<eax> = entity_s*
// ─────────────────────────────────────────────────────────────────────────────
void Undo_AddEntity_W( entity_s *a1 )
{
    if ( g_lastundo )
    {
        Undo_AddEntity( (int)(intptr_t)a1 );
        if ( a1 != (entity_s *)world_entity->def )
        {
            // Walk the entity's brush DEF list and Undo_AddBrush each.  Disasm-verified
            // (0x45E9BE..0x45E9D6): start = *(a1+0x0C) (= a1->brushes.prev, IDA's
            // "brushes.oprev" — the first 88-byte brush def), sentinel = &a1->def___or_
            // firstActive (a1+0x08), step via the brush def's onext (brush_t+0x04), and
            // pass the def DIRECTLY.  The prior port walked the wrong field (ownerNext),
            // the wrong sentinel (&a1->brushes), and passed b->def — a latent NULL+0x14
            // deref that AddProp (this loop's first GUI exerciser) finally hit.
            brush_t *b = (brush_t *)a1->brushes.prev;
            for ( ; (void *)b != (void *)&a1->def; b = b->onext )
                Undo_AddBrush( (entity_brush_s *)b );
        }
    }
    else
    {
        Sys_Printf( "Undo_AddEntity: no last undo.\n" );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45e9e0  Undo_SetIdForEntity  (63 bytes)
// Stamps the current undo ID into an entity and all its brush defs.
// __usercall: ent@<edx> = entity_s_def*
// ─────────────────────────────────────────────────────────────────────────────
void Undo_SetIdForEntity( entity_s_def *ent )
{
    if ( !g_lastundo )
        return;
    if ( g_lastundo->done )
        return;
    if ( ent == (entity_s_def *)world_entity->def )
        return;

    ent->epairEdits = g_lastundo->id;
    // Stamp the undo id into each brush DEF's +0x10 (brush_t.ownerPrev).  Disasm-verified
    // (0x45EA00..0x45EA1B): same def-list walk as Undo_AddEntity_W — start = *(ent+0x0C)
    // (ent->brushes.prev), sentinel = &ent->def (ent+0x08), step via the
    // def's onext (brush_t+0x04).  (The prior ownerNext walk silently skipped the defs.)
    for ( brush_t *b = (brush_t *)ent->brushes.prev;
          (void *)b != (void *)&ent->def; b = b->onext )
        b->ownerPrev = (entity_s *)(intptr_t)g_lastundo->id;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45ea20  Undo_End  (100 bytes)
// Closes the current undo record.
// ─────────────────────────────────────────────────────────────────────────────
void Undo_End()
{
    if ( !g_lastundo )
        return;
    if ( g_lastundo->done )
        return;

    undo_s *v0 = g_lastundo;
    g_lastundo->done = 1;

    // Trim oldest undos while memory exceeds limit (always keep at least one)
    while ( g_undoMemorySize > g_undoMaxMemorySize )
    {
        if ( g_undolist == v0 )
            break;
        Undo_FreeFirstUndo();
        if ( g_undoMemorySize <= g_undoMaxMemorySize )
            break;
        v0 = g_lastundo;
    }

    MarkMapModified();
    // Refresh the brush/entity list diagnostics.
    {
        char *s1 = (char *)va( "%s - active_brushes", g_lastundo->operation );
        MainFrm_BrushList( (int)(intptr_t)s1, &active_brushes );
        char *s2 = (char *)va( "%s - active_brushes", g_lastundo->operation );
        MainFrm_BrushList( (int)(intptr_t)s2, &selected_brushes );
        MainFrm_EntList( &entityInsts, g_lastundo->operation );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45ea90  Undo_Undo  (2125 bytes)
// Undoes the last completed operation.
// __thiscall (this@<ecx> unused -- plain cdecl wrapper).
// ─────────────────────────────────────────────────────────────────────────────
void Undo_Undo()
{
    if ( !g_lastundo )
    {
        Sys_Printf( "Nothing left to undo.\n" );
        return;
    }
    if ( !g_lastundo->done )
        Sys_Printf( "Undo_Undo: WARNING: last undo not yet finished!\n" );

    // Refresh the brush/entity list diagnostics.
    {
        char *s1 = (char *)va( "%s - active_brushes", g_lastundo->operation );
        MainFrm_BrushList( (int)(intptr_t)s1, &active_brushes );
        char *s2 = (char *)va( "%s - active_brushes", g_lastundo->operation );
        MainFrm_BrushList( (int)(intptr_t)s2, &selected_brushes );
        MainFrm_EntList( &entityInsts, g_lastundo->operation );
    }

    undo_s *undo = g_lastundo;

    // Detach undo from the list tail
    if ( undo->prev )
        undo->prev->next = nullptr;
    else
        g_undolist = nullptr;
    g_lastundo = undo->prev;

    // Allocate a new redo record
    undo_s *redo = (undo_s *)::operator new( 0x100u );
    if ( !redo )
        return;
    memset( redo, 0, sizeof(undo_s) );

    redo->brushlist.onext  = (brush_t_def *)&redo->brushlist;
    redo->brushlist.oprev  = (brush_t_def *)&redo->brushlist;
    redo->entitylist.next  = &redo->entitylist;
    redo->entitylist.prev  = &redo->entitylist;

    // Append redo to the redo list tail
    undo_s *prevRedo = g_lastredo;
    if ( prevRedo )
        prevRedo->next = redo;
    else
        g_redolist = redo;
    redo->prev = prevRedo;
    redo->next = nullptr;
    g_lastredo = redo;

    redo->time      = (double)clock() / 1000.0;
    redo->id        = g_redoId;
    redo->done      = 1;
    g_redoId        = g_redoId + 1;
    redo->operation = undo->operation;

    // Reset redoId on all active brush defs and entities that already carry it
    // IDA: iterates active_brushes via selbrush_t.next; tests def->def==redo->id
    for ( selbrush_t *i = active_brushes.next; i != &active_brushes; i = i->next )
    {
        brush_t *def = i->def;
        if ( def->def == (brush_t_def *)(intptr_t)redo->id )
            def->def = nullptr;
    }
    // The binary clears the redoId stamp at entity+0x80, NOT epairEdits (0x7C); the same
    // field Phase 3 stamps (0x45efae `mov [esi+80h],ecx`) — stale stamps must not collide.
    for ( entity_s *v12 = entities.next; v12 != &entities; v12 = v12->next )
    {
        if ( v12->redoId == redo->id )
            v12->redoId = 0;
    }

    // Deselect all
    Select_Deselect( 1 );

    // ── Phase 1: Move "created" brush instances (ownerPrev==undo->id) into redo's brushlist.
    {
        brush_t   *p_redoBrushlist = &redo->brushlist;
        selbrush_t *inst = active_brushes.next;
        while ( inst != &active_brushes )
        {
            selbrush_t *pNextInst = (selbrush_t *)&inst->next->prev; // IDA advance
            brush_t *def = inst->def;

            if ( def->ownerPrev == (entity_s *)(intptr_t)undo->id )
            {
                // Save entity instance ID for later redo restore
                def->unk1 = def->owner->numberId;

                {
                    selbrush_t *brushInst = inst;   // the binary's local
                    vassert( (brushInst->def->refCount >= 2), "(brushInst->def->refCount) = %i", brushInst->def->refCount );   // Undo.cpp:702
                }

                Entity_UnlinkBrush( def );   // inlined in the binary (entity.cpp:1208)

                // Move def into redo's brushlist
                Undo_01_NoMoreUndo( p_redoBrushlist, def );
                Brush_Free( inst );

                {
                    brush_t *brushDef = def;        // the binary's local
                    iassert( brushDef->refCount == 1 );   // Undo.cpp:707
                }
            }
            inst = (selbrush_t *)pNextInst;
        }
    }

    // ── Phase 2: Move "created" entity instances (epairEdits==undo->id) into redo's entitylist.
    {
        entity_s *pEInst = entityInsts.next;
        entity_s *p_entitylist_redo = &redo->entitylist;
        while ( pEInst != &entityInsts )
        {
            entity_s     *pNextEInst = (entity_s *)&pEInst->next->prev;
            entity_s_def *def        = (entity_s_def *)pEInst->def;

            if ( def->epairEdits == undo->id )
            {
                // DecRef + IncRef inlined in the binary (IncRef's 690 assert is
                // optimized out -- provably true after DecRef nulls both links).
                DecRef( (entity_s *)def );
                IncRef( (entity_s *)def, p_entitylist_redo );

                {
                    entity_s_def *entity = def;     // the binary's local
                    iassert( entity->refCount == 1 );   // Undo.cpp:720
                    ++entity->refCount;
                    Entity_Free( (char *)pEInst );
                    iassert( entity->refCount == 1 );   // Undo.cpp:723
                }
            }
            pEInst = pNextEInst;
        }
    }

    // ── Phase 3: Restore entity defs from the undo record back into the live list.
    for ( entity_s *j = undo->entitylist.next;
          j && j != &undo->entitylist;
          j = undo->entitylist.next )
    {
        {
            entity_s *entity = j;                   // the binary's local
            iassert( entity->refCount == 1 );   // Undo.cpp:728
        }

        g_undoMemorySize -= (int)EntityDef_MemorySize( j );

        entity_s_def *worldDef = (entity_s_def *)world_entity->def;
        int jEntityId = j->numberId;

        if ( jEntityId == worldDef->numberId )
        {
            // World entity: restore the saved worldspawn epairs onto the live world.
            // Same swap idiom as Undo_Redo (see the note there): entNode/j takes the OLD
            // world epairs so Entity_Free_R frees those, world keeps the restored chain.
            // Here the binary's path is correct too (it zeroes j->epairs before the free),
            // so this matches the IDA end state; the swap just keeps both branches uniform.
            epair_t *tmpEpairs       = worldDef->epairs;
            worldDef->epairs         = j->epairs;
            j->epairs                = tmpEpairs;
            Entity_Free_R( j );
        }
        else
        {
            // Non-world entity: unlink from undo list, insert into entities.
            // DecRef + IncRef inlined in the binary (IncRef's 690 assert optimized out).
            DecRef( j );
            IncRef( j, &entities );

            j->redoId = redo->id;

            {
                entity_s *entity = j;               // the binary's local
                iassert( entity->refCount == 1 );   // Undo.cpp:746
                Prefab_Init( (prefab_s *)&entityInsts, (entity_s_def *)j, &active_brushes );
                --entity->refCount;
                iassert( entity->refCount == 1 );   // Undo.cpp:749
            }

            // Verify the def's brush-def-list sentinel is empty. The sentinel spans
            // offsets 0x08 (firstActive / "oprev") and 0x0C (its ->next, == "onext").
            // §11 OFFSET FIX: disasm 0x45f019 reads `[esi+0Ch]` (= j->brushes.prev, the
            // sentinel's onext side), NOT brushes.ownerPrev (0x1C). The prior port read
            // 0x1C — an unrelated field that essentially never equals &def, so this FATAL
            // assert would fire on every undo of a non-world entity (a path no gate drives).
            // KEEP_VERBOSE (pair): the binary's entity->brushes head overlays the
            // port's def(+0x08)/brushes(+0x0C) split — member path inexpressible.
            if ( (void *)j->brushes.prev != (void *)&j->def )   // [j+0x0C] vs &[j+0x08]
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\Undo.cpp",
                        750, 0, "%s", "entity->brushes.onext == &entity->brushes" );
            if ( (void **)j->def != (void **)&j->def )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\Undo.cpp",
                        751, 0, "%s", "entity->brushes.oprev == &entity->brushes" );
        }
    }

    // ── Phase 4: Restore brush defs from the undo record back into the live list.
    undo_s *v35 = undo;
    {
        brush_t *bDef = (brush_t *)undo->brushlist.onext;
        while ( bDef != &undo->brushlist )
        {
            g_undoMemorySize -= (int)Brush_MemorySize( bDef );
            Undo_Refcount( bDef );

            // Find the entity instance whose DEF carries the saved owner numberId
            // (disasm 0x45f0a1 `[ecx+84h]` — hex-rays' `instDef[2].next` is the IDB's
            // 64-byte selbrush_t stride landing on the def's numberId).
            entity_s *ownerInst = nullptr;
            for ( entity_s *e = entityInsts.next; e != &entityInsts; e = e->next )
            {
                entity_s_def *eDef = (entity_s_def *)e->def;
                if ( eDef->numberId == bDef->unk1 )
                {
                    ownerInst = e;
                    break;
                }
            }

            // Link brush def into entity (Entity_LinkBrush inline)
            if ( ownerInst && ownerInst != &entityInsts )
            {
                entity_s_def *ownerDef = (entity_s_def *)ownerInst->def;
                Entity_LinkBrush( bDef, (entity_s *)ownerDef );   // inlined in the binary
            }
            else
            {
                // Fall back to world entity
                entity_s_def *worldDef = (entity_s_def *)world_entity->def;
                Entity_LinkBrush( bDef, (entity_s *)worldDef );   // inlined in the binary
                ownerInst = world_entity;
            }

            // Add instance to active list
            selbrush_t *newInst = Brush_AddToList( bDef, ownerInst );
            if ( newInst->next || newInst->prev )
                Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
            Brush_AddToList2( newInst );

            // Stamp with redo ID
            bDef->def = (brush_t_def *)(intptr_t)redo->id;

            // Advance: re-read head (list shrinks as Undo_Refcount removes each node)
            bDef = (brush_t *)v35->brushlist.onext;
        }
    }

    Sys_Printf( "%s undone.\n", v35->operation );

    g_undoMemorySize -= 256; // sizeof(undo_s)
    ::operator delete( v35 );
    --g_undoSize;

    if ( --g_undoId <= 0 )
        g_undoId = 2 * g_undoMaxSize;

    // Rebuild brush visuals for all selected + active brushes
    g_bScreenUpdates = 1;
    for ( selbrush_t *v44 = selected_brushes.next; v44 != &selected_brushes; v44 = v44->next )
        sub_47B940( v44->def );
    for ( selbrush_t *m = active_brushes.next; m != &active_brushes; m = m->next )
        sub_47B940( m->def );

    char *v46 = (char *)va( "%s - active_brushes", "after undo" );
    MainFrm_BrushList( (int)(intptr_t)v46, &active_brushes );
    char *v47 = (char *)va( "%s - active_brushes", "after undo" );
    MainFrm_BrushList( (int)(intptr_t)v47, &selected_brushes );
    MainFrm_EntList( &entityInsts, "after undo" );

    Sys_Printf( "Updating layers...\n" );
    Layers_SetMapLayers();
    Layers_02();
    g_nUpdateBits = -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// 0x45f2e0  Undo_Redo  (1423 bytes)
// Redoes the last undone operation.
// __thiscall (this@<ecx> unused -- plain cdecl wrapper).
// ─────────────────────────────────────────────────────────────────────────────
void Undo_Redo()
{
    if ( !g_lastredo )
    {
        Sys_Printf( "Nothing left to redo.\n" );
        return;
    }

    if ( g_lastundo && !g_lastundo->done )
        Sys_Printf( "WARNING: last undo not finished.\n" );

    // Refresh the brush/entity list diagnostics.
    {
        char *s1 = (char *)va( "%s - active_brushes", g_lastredo->operation );
        MainFrm_BrushList( (int)(intptr_t)s1, &active_brushes );
        char *s2 = (char *)va( "%s - active_brushes", g_lastredo->operation );
        MainFrm_BrushList( (int)(intptr_t)s2, &selected_brushes );
        MainFrm_EntList( &entityInsts, g_lastredo->operation );
    }

    undo_s *redo = g_lastredo;

    // Detach redo from the list tail
    if ( redo->prev )
        redo->prev->next = nullptr;
    else
        g_redolist = nullptr;
    const char *operation = redo->operation;
    g_lastredo = redo->prev;

    // Start a new undo for this redo operation
    Undo_GeneralStart( operation );
    Select_Deselect( 1 );

    // ── Phase 1: Move active brush instances with redoId == redo->id
    //            into the new undo's brushlist.
    {
        selbrush_t *v4 = active_brushes.next;
        while ( v4 != &active_brushes )
        {
            selbrush_t **pPrev = &v4->next->prev;
            brush_t *def = v4->def;

            if ( def->def == (brush_t_def *)(intptr_t)redo->id )
            {
                {
                    brush_t *brushDef = def;        // the binary's local
                    vassert( (brushDef->refCount >= 2), "(brushDef->refCount) = %i", brushDef->refCount );   // Undo.cpp:849
                }
                def->unk1 = def->owner->numberId;
                Entity_UnlinkBrush( def );
                Undo_01_NoMoreUndo( &g_lastundo->brushlist, def );
                g_undoMemorySize += (int)Brush_MemorySize( def );
                Brush_Free( v4 );
            }
            v4 = (selbrush_t *)pPrev;
        }
    }

    // ── Phase 2: Move entity instances with redoId == redo->id
    //            into the new undo's entitylist.
    {
        entity_s *v7 = entityInsts.next;
        while ( v7 != &entityInsts )
        {
            entity_s **v9 = &v7->next->prev;
            entity_s_def *def = (entity_s_def *)v7->def;

            if ( def->redoId == redo->id )
            {
                // Unlink from redo list, link into the new undo's entitylist.
                // DecRef + IncRef inlined in the binary (IncRef's 690 assert optimized out).
                DecRef( (entity_s *)def );
                IncRef( (entity_s *)def, &g_lastundo->entitylist );

                {
                    entity_s_def *entity = def;     // the binary's local
                    iassert( entity->refCount == 1 );   // Undo.cpp:867
                    ++entity->refCount;
                    Entity_Free( (char *)v7 );
                    iassert( entity->refCount == 1 );   // Undo.cpp:870
                }
                g_undoMemorySize += (int)EntityDef_MemorySize( def );
            }
            v7 = (entity_s *)v9;
        }
    }

    // ── Phase 3: Restore entity defs from redo's entitylist.
    undo_s *v31 = redo;
    entity_s *entNode = redo->entitylist.next;
    while ( entNode )
    {
        if ( entNode == &v31->entitylist )
            break;

        entity_s_def *worldDef = (entity_s_def *)world_entity->def;
        int entEntityId = entNode->numberId;

        if ( entEntityId == worldDef->numberId )
        {
            // World entity: restore the saved worldspawn epairs onto the live world.
            // DELIBERATE DIVERGENCE FROM IDA (do NOT "re-faithful" this): the binary
            // (sub_45F2E0) frees world's current epairs inline, sets world->epairs =
            // entNode->epairs, then Entity_Free_R(entNode) WITHOUT first zeroing
            // entNode->epairs — but Entity_Free_R (0x483C80) calls Entity_FreeEpairs,
            // so it frees entNode->epairs, which now ALIASES world->epairs → the
            // freshly-restored world epairs are use-after-freed. (Undo_Undo's binary
            // path zeroes j->epairs first and is fine; Undo_Redo's omits that zero —
            // a latent binary UAF on redo of a worldspawn epair edit.) We swap instead:
            // entNode takes the OLD world epairs so Entity_Free_R frees THOSE, and the
            // restored chain (now world->epairs) survives. Same intended end state,
            // no UAF.
            epair_t *tmpEpairs  = worldDef->epairs;
            worldDef->epairs    = entNode->epairs;
            entNode->epairs     = tmpEpairs;
            Entity_Free_R( entNode );
        }
        else
        {
            // Unlink from redo list, insert into entities.
            // DecRef + IncRef inlined in the binary (IncRef's 690 assert optimized out).
            DecRef( entNode );
            IncRef( entNode, &entities );

            {
                entity_s *entity = entNode;         // the binary's local
                iassert( entity->refCount == 1 );   // Undo.cpp:891
                Prefab_Init( (prefab_s *)&entityInsts, (entity_s_def *)entNode, &active_brushes );
                --entity->refCount;
                iassert( entity->refCount == 1 );   // Undo.cpp:894
            }

            // §11 OFFSET FIX (mirrors Undo_Undo 750): disasm 0x45f7... reads `[next+12]`
            // (= entNode->brushes.prev, the def-list sentinel's onext side at 0x0C), NOT
            // brushes.ownerPrev (0x1C). Prior port read 0x1C → this FATAL would fire on
            // every redo of a non-world entity.
            // KEEP_VERBOSE (pair): same brushes-head overlay as Undo.cpp:750/751.
            if ( (void *)entNode->brushes.prev != (void *)&entNode->def )  // [next+0x0C] vs &[next+0x08]
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\Undo.cpp",
                        895, 0, "%s", "entity->brushes.onext == &entity->brushes" );
            if ( (void **)entNode->def != (void **)&entNode->def )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\Radiant\\Undo.cpp",
                        896, 0, "%s", "entity->brushes.oprev == &entity->brushes" );
        }
        entNode = v31->entitylist.next;
    }

    // ── Phase 4: Restore brush defs from redo's brushlist.
    {
        brush_t *onext = (brush_t *)redo->brushlist.onext;
        while ( onext != &redo->brushlist )
        {
            Undo_Refcount( onext );

            // Find owner entity instance by saved owner numberId.
            // §11 STRIDE FIX (mirrors Undo_Undo Phase 4): disasm 0x45f6d3 reads `[ecx+84h]`
            // (the entity_s_def's numberId @ 0x84), not the 56-byte-stride `instDef[2].next`
            // (= 0x74 = epairs ptr) the IDA pseudocode rendered with the 64-byte IDB selbrush_t.
            entity_s *v17 = nullptr;
            for ( entity_s *e = entityInsts.next; e != &entityInsts; e = e->next )
            {
                entity_s_def *eDef = (entity_s_def *)e->def;
                if ( eDef->numberId == onext->unk1 )
                {
                    v17 = e;
                    break;
                }
            }

            if ( v17 && v17 != &entityInsts )
            {
                entity_s_def *v19 = (entity_s_def *)v17->def;
                Entity_LinkBrush( onext, (entity_s *)v19 );   // inlined in the binary
            }
            else
            {
                // World fallback. The inline link sets onext->owner = world_entity->def, but the
                // binary then passes &entityInsts (NOT world_entity) to Brush_AddToList: IDA
                // Undo_Redo sets edi = entityInsts.next @0x45f6ba, both no-match/empty exits leave
                // edi == &entityInsts, and loc_45F748..0x45f7a7 never touch edi -> push edi = &entityInsts.
                // This is a LATENT BUG in the original Undo_Redo: it trips Brush_AddToList's
                // brush.cpp:2386 `def->owner == owner->def` assert (def->owner = world_entity->def,
                // &entityInsts.def = 0) and links the instance into the entityInsts sentinel's chain
                // instead of world's. The sibling Undo_Undo (0x45f17c `mov ebx, world_entity`) does NOT
                // share the bug. Reproduced verbatim per the match-IDA-exactly directive (a prior port
                // had "fixed" this to world_entity, which diverged from the IDB).
                entity_s_def *worldDef = (entity_s_def *)world_entity->def;
                Entity_LinkBrush( onext, (entity_s *)worldDef );   // inlined in the binary
                v17 = (entity_s *)&entityInsts;
            }

            selbrush_t *newInst = Brush_AddToList( onext, v17 );
            if ( newInst->next || newInst->prev )
                Com_Error( ERR_FATAL, "Brush_AddToList: already linked" );
            Brush_AddToList2( newInst );

            // Advance: re-read head
            onext = (brush_t *)v31->brushlist.onext;
        }
    }

    Undo_End();

    Sys_Printf( "%s redone.\n", redo->operation );
    --g_redoId;
    ::operator delete( redo );

    g_bScreenUpdates = 1;
    char *v24 = (char *)va( "%s - active_brushes", "after redo" );
    MainFrm_BrushList( (int)(intptr_t)v24, &active_brushes );
    char *v25 = (char *)va( "%s - active_brushes", "after redo" );
    MainFrm_BrushList( (int)(intptr_t)v25, &selected_brushes );
    MainFrm_EntList( &entityInsts, "after redo" );
    g_nUpdateBits = -1;
}
