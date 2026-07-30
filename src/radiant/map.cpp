#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\map.cpp — map load/save, region caging and the prefab edit stack.
// Load: Map_LoadFromFile 0x486680 -> Map_LoadEntities (entity.cpp) -> ParseEntity.
// Save: Map_SaveFile 0x486c00 -> Map_SaveFileToPerforce -> MapFile_WriteEntity.

#include "stdafx.h"
#include <afxdlgs.h>             // CFileDialog — TU-local, NOT in the shared PCH (it reorders
                                 // <windows.h> ahead of the DXSDK headers and breaks gfx_d3d).
#include "qe3.h"
#include "mainfrm.h"
#include "prefs.h"               // g_PrefsDlg (m_bCleanTinyBrushes / m_fTinySize)
#include <universal/q_parse.h>  // Com_ParseExt, parseInfo_t, Com_GetParseThreadInfo

// ─── Assert and Sys_Printf from engine / radiant ──────────────────────────────
extern void  Assert( const char *file, int line, int type, const char *fmt, ... );
extern int   Sys_Printf( const char *fmt, ... );
extern void  MarkMapModified();

// ─── globals from entity.cpp ──────────────────────────────────────────────────
extern entity_s     entities;           // 0x23F17A0 — entity-def doubly-linked list sentinel
extern entity_s     entityInsts;        // 0x23F1748 — entity-instance list sentinel
extern bool         g_bRestoreBetween;  // 0x25D5B07
extern float        world_orient_matrix[4][3];  // 0x6DE290

// ─── globals from engine_stubs.cpp / qe3.h extern ────────────────────────────
extern selbrush_t   selected_brushes;   // 0x23F1864
extern selbrush_t   active_brushes;     // 0x23F189C
extern selbrush_t   filtered_brushes;   // 0x23F182C
extern int          g_nUpdateBits;      // 0x25D5A74
extern char         g_activeLayer_string[];  // 0x73BF78
extern void        *zero;               // empty string sentinel (engine_stubs.cpp)

// ─── globals from engine (qe3.h) ──────────────────────────────────────────────
extern qeglobals_t  g_qeglobals;        // 0x25F39C0
extern CMainFrame  *g_pParentWnd;       // 0x25D5A70

entity_s    *world_entity = nullptr;    // 0x25D5B30

// ─── map.cpp globals (IDB-verified) ───────────────────────────────────────────
char        currentmap[1024] = "";      // 0x23F18D8
int         modified = 0;               // 0x23F179C
int         prefabStackLevel = 0;       // 0x25D5B34
static bool Map_EditingPrefab() { return prefabStackLevel > 0; }   // the binary's inlined helper (map.cpp:360 string)
HCURSOR     hCursor = nullptr;          // 0x240A114
int         g_region_active = 0;        // 0x23F1744
float       region_mins[3] = { -131072.0f, -131072.0f, -131072.0f };  // 0x739C14
float       region_maxs[3] = {  131072.0f,  131072.0f,  131072.0f };  // 0x739D24
// region_sides[4] @0x23F1734 — the 4 region-boundary wall brush instances.  Modeled as one
// contiguous array because the binary index-walks it (loop terminator is the address of
// g_region_active @0x23F1744, immediately after); 4 separate globals need not be adjacent.
selbrush_t *region_sides[4] = { nullptr, nullptr, nullptr, nullptr };

// ─── IDB texWndGlob (0x25D7990) — qtextures chain (texwnd.cpp owns the object) ─
extern qtexture_s *TexWnd_GetMaterialListHead();

// prefab stack storage: g_prefabStack (IDB byte_25EB240) — declared in qe3.h.

// ─── prefab_s (IDB 0x54; mirror of entity.cpp/mayaexport.cpp) ─────────────────
// The 5-pointer head holds the prefab's instanced entity + brush list sentinels;
// Prefab_NextLevel splices these into the live world on enter.
struct prefab_s
{
    entity_s    *prev_entity;           // 0x00
    entity_s    *next_entity;           // 0x04
    void        *unk;                   // 0x08
    selbrush_t  *active_brushlist;      // 0x0C   tail sentinel (prev side)
    selbrush_t  *active_brushlist_next; // 0x10   head sentinel (next side)
    char         _pad[0x54 - 0x14];     // 0x14 .. 0x53
};
static_assert(sizeof(prefab_s) == 0x54, "prefab_s (map.cpp mirror != entity.cpp)");

// ─── surface window flag / dialog ─────────────────────────────────────────────
// surfDlgGlob (surface inspector; .hwnd = HWND when open) comes from qe3.h

// ─── forward declarations for functions defined in this file ──────────────────
void Brush_FreeMapBrushes();
void Map_NewMap();
void Map_Free();
void Map_BuildBrushData();
entity_s_def *Entity_GetClass( const char *name );
void Map_LoadFromFile( const char *path );
void Map_SaveFile( const char *path, char a1, char a2 );
void AddRegionBrushes();
void Map_RegionOff();
void Map_ApplyRegion();
int  Map_GetNextAutoTarget();
char *Map_GetNextExportId( int slot );
void Entity_WriteSelected_R( int *writerVtbl );
void FreePrefabLevel( entity_s *a1, entity_s *a2, entity_s *a3, int level );
FILE *Map_SaveFileToPerforce( const char *path, char a2 );
char MapFile_WriteEntity( entity_s_def *a1, FILE *a2, char a3 );
void Prefab_NextLevel( void *a1 );
void Prefab_PrevLevel();
void Prefab_LevelBack();

// ─── forward declarations for functions from other files ──────────────────────
// WriteFunc_map_t: the writer function type shared between Entity_WriteSelected
// and Brush_Write.  int(int ctx, const char *fmt, ...) — matches brush.cpp WriteFunc_t
// and entity.cpp WriteFunc_entity_t.
typedef int WriteFunc_map_t( int ctx, const char *fmt, ... );

// entity.cpp
extern int          Map_LoadEntities( const char *filename, entity_s *entList, char a3 );   // defined below (0x486500)
extern void         Map_New();
extern entity_s    *Prefab_Init( struct prefab_s *a1, entity_s_def *entDef, selbrush_t *a3 );
extern void         Entity_Free( char *a1 );
extern bool         Entity_HasEpairMatch( entity_s *e, const char *key, const char *val );
extern int          Entity_GetVec3ForKey( entity_s_def *e, float *out, const char *key );
extern void         SetKeyValue( entity_s_def *e, const char *key, const char *value );
extern void         Entity_WriteSelected( entity_s_def *ent, WriteFunc_map_t **writer );
extern void         Entity_FreePrefab( entity_s *e );

// brush.cpp
extern void         Brush_Free( selbrush_t *b );
extern void         Brush_BuildWindings( brush_t *b, int rebuild );
extern selbrush_t  *Brush_AddToList( brush_t *b, entity_s *e );
extern void         Brush_AddToList2( selbrush_t *b );
extern void         Brush_RemoveFromList( selbrush_t *b );
extern bool         Brush_RemoveEmptyFaces02( brush_t *b );   // brush.cpp Chunk C (REAL — not the engine_stubs no-op shadow)
// Brush_Write: WriteWriter_t = WriteFunc_t** (same WriteFunc_map_t type)
extern int          Brush_Write( WriteFunc_map_t **writer, brush_t *b );
extern brush_t     *Brush_Alloc( const void *planeptsSrc, eclass_t *ecls );        // brush.cpp 0x4751e0
extern void         Brush_Create( float *mins, float *maxs, brush_t *b, eclass_t *ecls );  // brush.cpp 0x475300
extern void         Entity_LinkBrush( brush_t *b, entity_s *world_ent );           // entity.cpp 0x484fc0
extern void         SetMaterial( const char *tex_name, patchMesh_material *out );  // materialdef.cpp 0x4315c0
extern int          Init_MaterialLayer( MaterialDef *a1, MaterialDef *a2 );        // materialdef.cpp 0x472c00

// layers.cpp
extern void         Map_InitlLayers();
extern void         Layers_SetMapLayers();
extern void         Layers_02();
extern void         Layers_WriteToFile( FILE *f );   // layer-declaration block writer
extern void         Pointfile_Clear();
extern models_t    *Model_FreeMapModels();   // eclass.cpp 0x480C50 (returns models_t*, discarded here)
extern LRESULT      Texture_ShowInuse();   // texwnd.cpp (0x45B850) — mark/count in-use textures
extern void         Undo_Clear();
extern void         CopySelectedFaceValues();
extern bool         Model_SetModel( entity_brush_s *b, int orientMatrix );  // brush.cpp 0x478780 (prefab load-on-open)
extern int          AddModelToModelInstBuff( XModel *model, float *axis, float scale );
extern void         ModelInstUpdate( int inst, float (*axis)[3], float scale );
extern void         RemoveModelInstFromBuf( int inst );
extern char         LayeredMaterials_Save();   // 0x416f40 — 1 ok/unchanged, 0 write-fail
extern void         sub_41C9C0( const char *name );
extern void         sub_47D060( int list );  // brush list display rebuild
// Prefab-edit-in-place layer-state save/restore (#18). The binary stacks the layer
// RB-tree into each prefab slot (+2088) via sub_489810/sub_41A5A0/sub_489F30; this
// port models the layer map as a std::map, so the deep RB-copy collapses to a
// per-level std::map snapshot — see layers.cpp.
extern void         Layers_SavePrefabLayers( int level );
extern void         Layers_RestorePrefabLayers( int level );

// select.cpp stubs
extern void         Select_Deselect( int a1 );
extern void         SetupVertexSelection();    // select.cpp 0x494bc0 (vertex/edge select mode)

// UI stubs (MFC)
extern void         MainFrm_BrushList( int label, selbrush_t *list );
extern void         MainFrm_EntList( entity_s *list, const char *label );
extern void         sub_47B940( brush_t *b );             // brush.cpp 0x47B940 Brush_UpdateSpecialMaterialFlag
extern void         sub_418A50( const char *modelpath );  // layers.cpp 0x418A50 Layers_MarkModelPrefix

// points.cpp — 0x23F1CD8 s_num_points.  Map_NewMap (0x486134) and Map_SaveFile (0x486cef)
// both zero it, dropping any displayed leak pointfile on new-map / save.
extern void         Pointfile_ResetPoints();

// Perforce
extern void         sub_437850( const char *path );   // p4 add
extern int          sub_4377E0( const char *path );   // p4 open-for-edit
extern int          FileExists( const char *path );

extern void         CTextureBar_GetSurfaceAttributes( void *bar );

// ── prefab-enter (Prefab_NextLevel 0x489190) deps ─────────────────────────────
// Sys_PrintActiveBrushes ("before entering prefab") is a pure diagnostic — call Sys_Printf.
extern void         Eclass_FreeModel( void **modelClass );                // eclass.cpp 0x480CB0
// Entity_GetOrientationMatrix (entity.cpp 0x482940) is file-static there; the GUI
// camera reframe below builds the entity orientation inline via AnglesToAxis.
extern float       *AnglesToAxis( float *angles, float (*axis)[3] );      // engine_stubs.cpp 0x4ABEB0
extern void         AxisToAngles( float *angles, float (*axis)[3] );      // engine_stubs.cpp 0x4A8A00
extern void         OrientationPosToWorldPos( float *out, const float *pos, const orientation_t *orient ); // sub_4BA610 (draw.cpp)
// sub_4BA6B0 (0x4BA6B0) — VectorRotate: out[i] = matrix_row(i+1) · dir.  Ported in draw.cpp.
extern void         VectorRotateByAxis( float *out, const float *axisMatrix, const float *dir );

// CXYWnd class methods — map.cpp calls these via g_pParentWnd->m_pXYWnd
// They are member functions; map.cpp calls them as CXYWnd::Paste(ptr) etc.
// Since mainfrm.h includes CXYWnd class declarations, these are resolved.

// VA — radiant rotating-buffer va() with slot index
extern char *VA( int slot, const char *fmt, ... );


// 0x486050  Brush_FreeMapBrushes — free the three display lists + every entity inst,
// then assert the entity-def list drained.
void Brush_FreeMapBrushes()
{
    while ( active_brushes.next != &active_brushes )
        Brush_Free( active_brushes.next );

    while ( selected_brushes.next != &selected_brushes )
        Brush_Free( selected_brushes.next );

    while ( filtered_brushes.next != &filtered_brushes )
        Brush_Free( filtered_brushes.next );

    while ( entityInsts.next != &entityInsts )
        Entity_Free( (char *)entityInsts.next );

    iassert( entities.next == &entities );   // map.cpp:170
}


// 0x486110  Map_NewMap — reset title, free all brushes, re-init the sentinel lists.
void Map_NewMap()
{
    strcpy( currentmap, "unnamed.map" );
    Pointfile_ResetPoints();            // 0x486134: s_num_points = 0
    SetWindowTextA( g_qeglobals.d_hwndMain, currentmap );
    g_qeglobals.d_num_entities = 0;

    if ( active_brushes.next )
    {
        Brush_FreeMapBrushes();
        Model_FreeMapModels();
        Map_InitlLayers();
    }
    else
    {
        // Bootstrap sentinels (first call)
        active_brushes.next      = &active_brushes;
        active_brushes.prev      = &active_brushes;
        selected_brushes.next    = &selected_brushes;
        selected_brushes.prev    = &selected_brushes;
        filtered_brushes.next    = &filtered_brushes;
        filtered_brushes.prev    = &filtered_brushes;
        entities.next            = &entities;
        entities.prev            = &entities;
        entityInsts.next         = &entityInsts;
        entityInsts.prev         = &entityInsts;
    }
    world_entity = nullptr;
}


// 0x485e50  Map_Free — offer to carry the selection across to the next map (clipboard).
// Does NOT free brushes; that is Map_NewMap's job.
void Map_Free()
{
    g_bRestoreBetween = false;
    if ( selected_brushes.next && selected_brushes.next != &selected_brushes )
    {
        HWND activeWnd = GetActiveWindow();
        if ( MessageBoxA( activeWnd, "Copy selection?", "Radiant", 0x24u ) == IDYES )
        {
            if ( g_pParentWnd && g_pParentWnd->m_pActiveXY )
            {
                g_bRestoreBetween = true;
                g_pParentWnd->m_pActiveXY->Copy();
            }
        }
    }
}


// 0x485EB0  CheckForTinyBrush — 1 (+ warning) if any axis extent is below tinySize.
// i[3] is the matching maxs[] because brush_t.maxs (+0x2C) immediately follows mins (+0x20).
static char CheckForTinyBrush( brush_t *def, int idx, float tinySize )
{
    int n = 0;
    for ( float *i = def->mins; i[3] - *i >= tinySize; ++i )
    {
        if ( ++n >= 3 )
            return 0;
    }
    Sys_Printf( "Possible problem brush (too small) #%i ", idx );
    return 1;
}


// 0x485F00  Map_BuildBrushData — per-brush: rebuild windings, (in vertex/edge mode) rebuild
// the selection handles, bump the version, drop duplicate faces; then cull the brush if it
// has no faces or — when CleanTinyBrushes is on — if it is too small.  Wait cursor throughout.
void Map_BuildBrushData()
{
    if ( !active_brushes.next )
        return;

    HCURSOR prevCursor = SetCursor( LoadCursorA( nullptr, (LPCSTR)IDC_WAIT ) );
    hCursor = prevCursor;

    int tinyIdx = 0;                         // v7  [ebp-4]
    selbrush_t *sb    = active_brushes.next;
    selbrush_t *onext = nullptr;             // ebx — captured before any free

    if ( active_brushes.next )               // IDA re-checks (0x485f3c); kept faithful
    {
        do
        {
            if ( sb == &active_brushes )
                break;

            brush_t *def = sb->def;
            onext        = sb->next;

            Brush_BuildWindings( def, 0 );
            if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
                SetupVertexSelection();
            ++def->version;
            if ( Brush_RemoveEmptyFaces02( sb->def ) )
                Sys_Printf( "Removed duplicate faces from brush\n" );

            if ( sb->def->faces )            // not degenerate (faces != 0)
            {
                if ( !g_PrefsDlg->m_bCleanTinyBrushes )
                {
                    sb = onext;              // keep brush
                    continue;
                }
                if ( !CheckForTinyBrush( sb->def, tinyIdx++, g_PrefsDlg->m_fTinySize ) )
                {
                    sb = onext;              // not tiny → keep
                    continue;
                }
            }

            Brush_Free( sb );                // faces==0 (degenerate) OR tiny
            Sys_Printf( "Removed degenerate brush\n" );
            sb = onext;
        }
        while ( onext );
    }

    if ( hCursor )
    {
        SetCursor( hCursor );
        hCursor = nullptr;
    }
}


// 0x486010  Entity_GetClass — first entity whose "classname" epair matches `name`.
entity_s_def *Entity_GetClass( const char *name )
{
    for ( entity_s_def *e = (entity_s_def *)entities.next;
          e != (entity_s_def *)&entities;
          e  = (entity_s_def *)e->next )
    {
        if ( Entity_HasEpairMatch( e, "classname", name ) )
            return e;
    }
    return nullptr;
}


// 0x486680  Map_LoadFromFile — main map-load entry point.
void Map_LoadFromFile( const char *path )
{
    HCURSOR cursorA = LoadCursorA( nullptr, (LPCSTR)IDC_WAIT );
    hCursor         = SetCursor( cursorA );

    Map_Free();
    Select_Deselect( 1 );

    // Normalise path: backslash -> forward slash
    char String[1028];
    {
        const char *src = path;
        char       *dst = String;
        while ( *src )
        {
            *dst++ = ( *src == '\\' ) ? '/' : *src;
            ++src;
        }
        *dst = '\0';
    }

    Sys_Printf( "Map_LoadFile: %s\n", String );

    iassert( prefabStackLevel >= 0 );
    iassert( !Map_EditingPrefab() );   // map.cpp:360

    Map_NewMap();
    Map_InitlLayers();

    // Clear the in-use flag (qtexture_s.is_in_use @+8) on every registered material,
    // walking the texWndGlob list link (qtexture_s.prev @+0x24).
    for ( qtexture_s *qt = TexWnd_GetMaterialListHead(); qt; qt = qt->prev )
        qt->is_in_use = false;

    g_qeglobals.d_parsed_brushes   = 0;
    strcpy( currentmap, path );
    g_qeglobals.g_layerCount_maybe = 1;

    Sys_Printf( "Updating layers...\n" );
    Layers_SetMapLayers();
    Layers_02();

    g_qeglobals.d_num_entities = Map_LoadEntities( path, &entities, 0 );

    // world_entity must be NULL before post-process loop (Map_NewMap clears it;
    // LoadEntities does NOT set it — the loop below does).
    iassert( world_entity == NULL );   // map.cpp:372

    // ── Post-process each entity def ─────────────────────────────────────────
    entity_s_def *eDef = (entity_s_def *)entities.next;
    while ( eDef != (entity_s_def *)&entities )
    {
        entity_s_def *nextDef = (entity_s_def *)eDef->next;

        // Create entity instance (links brushes into entityInsts + active_brushes)
        entity_s *eInst = Prefab_Init( (struct prefab_s *)&entityInsts,
                                        eDef,
                                        &active_brushes );

        // Find "classname" epair
        const char *clsValue = "";
        for ( epair_t *ep = eDef->epairs; ep; ep = ep->next )
        {
            if ( _stricmp( ep->key, "classname" ) == 0 )
            {
                clsValue = ep->value ? ep->value : "";
                break;
            }
        }

        if ( strcmp( clsValue, "worldspawn" ) == 0 )
        {
            if ( world_entity )
            {
                Sys_Printf( "WARNING: multiple worldspawn\n" );
                Entity_Free( (char *)eInst );
            }
            else
            {
                world_entity = eInst;
            }
        }
        else if ( eDef->eclass && (eDef->eclass->classtype & 0x10) != 0 )
        {
            // Model entity: assign model to first brush instance
            selbrush_t *firstBrush = eInst->brushes.ownerNext;
            if ( firstBrush && firstBrush != (selbrush_t *)&eInst->brushes )
                Model_SetModel( (entity_brush_s *)firstBrush, (int)&world_orient_matrix );
        }

        eDef = nextDef;
    }

    if ( !world_entity )
    {
        Sys_Printf( "No worldspawn in map.\n" );
        Map_New();
        return;
    }

    iassert( entityInsts.next == world_entity );   // map.cpp:404

    Sys_Printf( "--- LoadMapFile ---\n" );
    Sys_Printf( "%s\n", String );
    Sys_Printf( "%5i brushes\n",  g_qeglobals.d_parsed_brushes );
    Sys_Printf( "%5i entities\n", g_qeglobals.d_num_entities );
    Sys_Printf( "Map_BuildAllDisplayLists\n" );

    Map_BuildBrushData();

    // ── Find start-position entity ────────────────────────────────────────────
    entity_s_def *startEnt = Entity_GetClass( "info_player_start" );
    if ( !startEnt )
        startEnt = Entity_GetClass( "info_player_deathmatch" );

    // Position camera from start entity
    if ( g_pParentWnd && g_pParentWnd->m_pCamWnd )
    {
        g_pParentWnd->m_pCamWnd->camera.angles[0] = 0.0f;
        if ( startEnt )
        {
            float origin[3] = { 0.0f, 0.0f, 0.0f };
            Entity_GetVec3ForKey( startEnt, origin, "origin" );
            origin[2] += 60.0f;

            g_pParentWnd->m_pCamWnd->camera.origin[0] = origin[0];
            g_pParentWnd->m_pCamWnd->camera.origin[1] = origin[1];
            g_pParentWnd->m_pCamWnd->camera.origin[2] = origin[2];

            if ( g_pParentWnd->m_pXYWnd )
            {
                g_pParentWnd->m_pXYWnd->m_vOrigin[0] = origin[0];
                g_pParentWnd->m_pXYWnd->m_vOrigin[1] = origin[1];
                g_pParentWnd->m_pXYWnd->m_vOrigin[2] = origin[2];
            }

            Entity_GetVec3ForKey( startEnt,
                                  g_pParentWnd->m_pCamWnd->camera.angles, "angles" );
        }
        else
        {
            g_pParentWnd->m_pCamWnd->camera.angles[1] = 0.0f;
            g_pParentWnd->m_pCamWnd->camera.origin[0] = 0.0f;
            g_pParentWnd->m_pCamWnd->camera.origin[1] = 0.0f;
            g_pParentWnd->m_pCamWnd->camera.origin[2] = 0.0f;
            if ( g_pParentWnd->m_pXYWnd )
            {
                g_pParentWnd->m_pXYWnd->m_vOrigin[0] = 0.0f;
                g_pParentWnd->m_pXYWnd->m_vOrigin[1] = 0.0f;
                g_pParentWnd->m_pXYWnd->m_vOrigin[2] = 0.0f;
            }
        }
    }

    Map_RegionOff();
    Texture_ShowInuse();
    modified = 0;
    SetWindowTextA( g_qeglobals.d_hwndMain, String );

    // ── Per-brush model load for model entities ───────────────────────────────
    for ( selbrush_t *sb = active_brushes.next;
          sb != &active_brushes; sb = sb->next )
    {
        if ( !sb->owner )
            continue;
        entity_s_def *def = (entity_s_def *)sb->owner->def;
        if ( !def || !def->eclass )
            continue;
        if ( (def->eclass->classtype & 0x10) != 0 )
        {
            const char *modelPath = "";
            for ( epair_t *ep = def->epairs; ep; ep = ep->next )
            {
                if ( _stricmp( ep->key, "model" ) == 0 )
                {
                    modelPath = ep->value ? ep->value : "";
                    break;
                }
            }
            sub_418A50( modelPath );
        }
    }

    Sys_Printf( "Updating layers...\n" );
    Layers_SetMapLayers();
    Layers_02();

    // ── Between-map clipboard paste ───────────────────────────────────────────
    if ( g_pParentWnd && g_pParentWnd->m_pActiveXY && g_bRestoreBetween )
        g_pParentWnd->m_pActiveXY->Paste();
    g_bRestoreBetween = false;

    if ( hCursor )
    {
        SetCursor( hCursor );
        hCursor = nullptr;
    }

    // Rebuild brush display lists
    for ( selbrush_t *sb = selected_brushes.next;
          sb != &selected_brushes; sb = sb->next )
        sub_47B940( sb->def );
    for ( selbrush_t *sb = active_brushes.next;
          sb != &active_brushes; sb = sb->next )
        sub_47B940( sb->def );

    MainFrm_BrushList( (int)VA( 0, "%s - active_brushes",   "loaded map" ), &active_brushes );
    MainFrm_BrushList( (int)VA( 1, "%s - active_brushes", "loaded map" ), &selected_brushes );
    MainFrm_EntList( &entityInsts, "loaded map" );

    g_nUpdateBits = -1;
}


// 0x486C00  Map_SaveFile — save the current map.  a1 = region save, a2 = add-to-perforce.
// Empty `path` pops the Save picker; every real call site passes a non-empty path (only
// Map_Snapshot's out-of-disk fallback at 0x48b37a can hand it an empty CString), so that
// branch is ported only so this function can never FATAL — and is GUI-only.
void Map_SaveFile( const char *path, char a1, char a2 )
{
    // 0x486c40..0x486ce8: save-as branch — resolve `path` from a Save file dialog.
    char *savedPathHeap = nullptr;        // _strdup'd copy (binary's v25/var_9F4)
    if ( !path || !path[0] )
    {
        // The binary takes the parent from AfxGetThread(); the port uses g_pParentWnd,
        // as every other CFileDialog site does.
        CFileDialog dlg( FALSE, "map", nullptr,
                         OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                         "Map Files (*.map)|*.map||", g_pParentWnd );
        if ( dlg.DoModal() != IDOK )      // 0x486ca1: DoModal()!=1 → cancel
            return;
        savedPathHeap = _strdup( dlg.GetPathName() );  // 0x486cbf: m_ofn.lpstrFile
        path = savedPathHeap;
        if ( !path || !path[0] )          // 0x486cec: empty pick → fall through (no-op)
        {
            if ( savedPathHeap ) free( savedPathHeap );
            return;
        }
    }

    Pointfile_ResetPoints();          // 0x486cef: s_num_points = 0

    char String[1028];
    {
        const char *src = path;
        char       *dst = String;
        while ( *src )
        {
            *dst++ = ( *src == '\\' ) ? '/' : *src;
            ++src;
        }
        *dst = '\0';
    }

    Sys_Printf( "Map_SaveFile: %s\n", path );

    // IDA 0x486d2d: test al,al; jz epilogue — a failed layered-materials save aborts
    // the ENTIRE map save (no .map written, no "Saved.", modified flag left set).
    if ( !LayeredMaterials_Save() )
    {
        if ( savedPathHeap ) free( savedPathHeap );
        return;
    }

    FILE *fout = Map_SaveFileToPerforce( path, (char)(a2 == 0) );
    if ( !fout )
    {
        if ( savedPathHeap ) free( savedPathHeap );
        return;
    }

    fprintf( fout, "iwmap %i\n", 4 );

    // Layer declarations: layerMap in sorted order, `"<name>" flags <flagstr> [active]`.
    Layers_WriteToFile( fout );

    if ( a1 )
        AddRegionBrushes();

    iassert( entities.next != &entities );   // map.cpp:534
    iassert( !strcmp( entities.next->eclass->name, "worldspawn" ) );   // map.cpp:535 (0x486efe derefs eclass unconditionally)

    // Write gate (0x486f50): def-list NON-empty OR worldspawn.  The def-list head is
    // brushes.prev (entity+0x0C), its sentinel is &def (entity+0x08); fixed-size entities
    // always carry their bbox brush, so only a brushless worldspawn needs the OR.
    int entIdx = 0;
    for ( entity_s *eIter = entities.next; eIter != &entities; eIter = eIter->next )
    {
        entity_s_def *eDef = (entity_s_def *)eIter;

        bool hasDefBrushes = ( (void *)eDef->brushes.prev != (void *)&eDef->def );
        bool isWorld       = ( strcmp( eDef->eclass->name, "worldspawn" ) == 0 );   // IDA 0x486f59 derefs eclass unconditionally

        if ( hasDefBrushes || isWorld )
        {
            fprintf( fout, "// entity %i\n", entIdx );
            ++entIdx;
            MapFile_WriteEntity( eDef, fout, a1 );
        }
    }

    fclose( fout );

    if ( a1 && g_region_active )
    {
        for ( int i = 0; i < 4; ++i )
            Brush_Free( region_sides[i] );   // IDA 0x486fd1 frees all 4 region walls unconditionally
    }

    Sys_Printf( "Saved.\n" );
    modified = 0;

    if ( !strstr( String, "autosave" ) )
        SetWindowTextA( g_qeglobals.d_hwndMain, String );

    if ( !a1 )
    {
        MessageBeep( 0x30 );
        // KISAK: differs from 0x48704e — the binary fcloses the handle TWICE on success
        // (0x487070 + 0x487078) and fclose(NULL)s it on failure.  Closed once here.
        FILE *tslog = fopen( "c:/tstamps.log", "a" );
        if ( tslog )
        {
            fprintf( tslog, "%s", path );
            fclose( tslog );
        }
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER + 1, 0, (LPARAM)"Saved.\n" );
    }

    if ( savedPathHeap ) free( savedPathHeap );   // release the save-as dialog pick
}


// 0x487220  AddRegionBrushes — 4 wall brushes caging the region box.
void AddRegionBrushes()
{
    if ( !g_region_active )
        return;

    MaterialDef mat;
    SetMaterial( "region", (patchMesh_material *)&mat );          // 0x48723d
    {
        float ss = 0.25f;                                        // 0x48724e COERCE_MATERIALDEF_(0.25):
        Init_MaterialLayer( &mat, *(MaterialDef **)&ss );        //   pass 0.25f's bits as the ptr arg
    }

    // Four boundary-wall slabs around the region box.  The ±16/±1 offsets use the binary's
    // DOUBLE constants (dbl_6F4438=16.0, dbl_6F4098=1.0); ±131072 are FLOAT (flt_6F4144/48).
    // Walls 2-4 reuse the components walls 1/3 left in mins[]/maxs[] (matching the binary's
    // partial re-assignment of v10..v15).
    float mins[3], maxs[3];

    // Wall 1 (-X slab)
    mins[0] = (float)( region_mins[0] - 16.0 );
    maxs[0] = (float)( region_mins[0] +  1.0 );
    mins[1] = (float)( region_mins[1] - 16.0 );
    maxs[1] = (float)( region_maxs[1] + 16.0 );
    mins[2] = -131072.0f;
    maxs[2] =  131072.0f;
    {
        brush_t *b = Brush_Alloc( &mat, nullptr );
        Brush_Create( mins, maxs, b, nullptr );
        region_sides[0] = Brush_AddToList( b, world_entity );
    }

    // Wall 2 (+X slab) — reuses mins[1..2]/maxs[1..2]
    mins[0] = (float)( region_maxs[0] -  1.0 );
    maxs[0] = (float)( region_maxs[0] + 16.0 );
    {
        brush_t *b = Brush_Alloc( &mat, nullptr );
        Brush_Create( mins, maxs, b, nullptr );
        region_sides[1] = Brush_AddToList( b, world_entity );
    }

    // Wall 3 (-Y slab) — reuses mins[2]/maxs[2]
    mins[0] = (float)( region_mins[0] - 16.0 );
    maxs[0] = (float)( region_maxs[0] + 16.0 );
    mins[1] = (float)( region_mins[1] - 16.0 );
    maxs[1] = (float)( region_mins[1] +  1.0 );
    {
        brush_t *b = Brush_Alloc( &mat, nullptr );
        Brush_Create( mins, maxs, b, nullptr );
        region_sides[2] = Brush_AddToList( b, world_entity );
    }

    // Wall 4 (+Y slab) — reuses mins[0]/maxs[0]/mins[2]/maxs[2]
    mins[1] = (float)( region_maxs[1] -  1.0 );
    maxs[1] = (float)( region_maxs[1] + 16.0 );
    {
        brush_t *b = Brush_Alloc( &mat, nullptr );
        Brush_Create( mins, maxs, b, nullptr );
        region_sides[3] = Brush_AddToList( b, world_entity );
    }

    // Link each wall into the world entity (0x4873c8..0x4874a6).
    for ( int i = 0; i < 4; ++i )
    {
        selbrush_t *inst = region_sides[i];
        Brush_AddToList2( inst );
        Entity_LinkBrush( inst->def, (entity_s *)world_entity->def );
        Brush_BuildWindings( inst->def, 1 );
        if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge )
            SetupVertexSelection();
        MarkMapModified();
        ++inst->def->version;                                     // brush_t.version@0x4E (16-bit)
        iassert( region_sides[i]->def->owner == world_entity->def );  // 0x48743c (level 0)
        iassert( region_sides[i]->owner != world_entity );           // 0x487467 (level 0)
        inst->owner = world_entity;
    }
}


// 0x487530  Map_RegionOff — reset the region to ±131072 and un-filter everything it covers.
void Map_RegionOff()
{
    region_maxs[0] =  131072.0f;
    region_maxs[1] =  131072.0f;
    region_maxs[2] =  131072.0f;
    region_mins[0] = -131072.0f;
    region_mins[1] = -131072.0f;
    region_mins[2] = -131072.0f;
    g_region_active = 0;

    // 0x487585: each move is gated on a 3-axis AABB overlap against def->mins/maxs (the
    // complement of Map_ApplyRegion's test); a brush failing any axis stays filtered.
    selbrush_t *sb = filtered_brushes.next;
    while ( sb != &filtered_brushes )
    {
        selbrush_t *next = sb->next;

        bool inRegion = true;
        for ( int ax = 0; ax < 3; ++ax )
        {
            if ( region_maxs[ax] < sb->def->mins[ax] ||
                 region_mins[ax] > sb->def->maxs[ax] )
            {
                inRegion = false;
                break;
            }
        }

        if ( inRegion )
        {
            Brush_RemoveFromList( sb );
            // Re-insert at head of active list
            sb->next                  = active_brushes.next;
            active_brushes.next->prev = sb;
            active_brushes.next       = sb;
            sb->prev                  = &active_brushes;
        }

        sb = next;
    }

    g_nUpdateBits = -1;
}


// 0x487650  Map_ApplyRegion — move every brush outside the region onto the filtered list.
void Map_ApplyRegion()
{
    g_region_active = 1;

    selbrush_t *sb = active_brushes.next;
    while ( sb != &active_brushes )
    {
        selbrush_t *next = sb->next;

        bool inRegion = true;
        for ( int ax = 0; ax < 3; ++ax )
        {
            if ( region_maxs[ax] < sb->def->mins[ax] ||
                 region_mins[ax] > sb->def->maxs[ax] )
            {
                inRegion = false;
                break;
            }
        }

        if ( !inRegion )
        {
            Brush_RemoveFromList( sb );
            sb->next                    = filtered_brushes.next;
            filtered_brushes.next->prev = sb;
            filtered_brushes.next       = sb;
            sb->prev                    = &filtered_brushes;
        }

        sb = next;
    }

    g_nUpdateBits = -1;
}

// Region setters (Region menu): Map_RegionXY 0x4877d0, Map_RegionTallBrush 0x487860,
// Map_RegionBrush 0x4878e0, Map_RegionSelectedBrushes 0x487720.  Tall/Brush delete the
// defining brush after caging — faithful: the brush becomes the region, then is removed.
extern signed int QE_SingleBrush();    // qe3.cpp (0x48c8b0)
extern void       Select_Delete();     // select.cpp (0x48e760)
extern void       Select_GetBounds( float *mins, float *maxs );  // select.cpp (0x48fb10)

void Map_RegionXY()
{
    Map_RegionOff();
    CXYWnd *xy = g_pParentWnd->m_pXYWnd;
    // Half the view extent in world units, centred on m_vOrigin.
    region_mins[0] = xy->m_vOrigin[0] - (double)xy->m_nWidth  * 0.5 / xy->m_fScale;
    region_maxs[0] = (double)xy->m_nWidth  * 0.5 / xy->m_fScale + xy->m_vOrigin[0];
    region_mins[1] = xy->m_vOrigin[1] - (double)xy->m_nHeight * 0.5 / xy->m_fScale;
    region_maxs[1] = 0.5 * (double)xy->m_nHeight / xy->m_fScale + xy->m_vOrigin[1];
    // BINARY BUG (faithful): 0x487840 loads flt_6F4148 (+131072) ONCE and fst/fstp's it to
    // BOTH Z bounds, so the Z slab is degenerate and XY-region hides the whole map.
    region_mins[2] = 131072.0f;
    region_maxs[2] = 131072.0f;
    Map_ApplyRegion();
}

void Map_RegionTallBrush()
{
    if ( !QE_SingleBrush() )
        return;
    selbrush_t *b = selected_brushes.next;
    Map_RegionOff();
    region_mins[0] = b->def->mins[0];
    region_mins[1] = b->def->mins[1];
    region_maxs[0] = b->def->maxs[0];
    region_maxs[1] = b->def->maxs[1];
    region_mins[2] = -131072.0f;   // full vertical extent
    region_maxs[2] =  131072.0f;
    Select_Delete();
    Map_ApplyRegion();
}

void Map_RegionBrush()
{
    if ( !QE_SingleBrush() )
        return;
    selbrush_t *b = selected_brushes.next;
    Map_RegionOff();
    region_mins[0] = b->def->mins[0];
    region_mins[1] = b->def->mins[1];
    region_mins[2] = b->def->mins[2];
    region_maxs[0] = b->def->maxs[0];
    region_maxs[1] = b->def->maxs[1];
    region_maxs[2] = b->def->maxs[2];
    Select_Delete();
    Map_ApplyRegion();
}

void Map_RegionSelectedBrushes()
{
    Map_RegionOff();
    if ( selected_brushes.next == &selected_brushes )
    {
        Sys_Printf( "Tried to region with no selection...\n" );
        return;
    }
    g_region_active = 1;
    Select_GetBounds( region_mins, region_maxs );

    // Move the entire active list into filtered_brushes (hide it)…
    filtered_brushes.next        = active_brushes.next;
    filtered_brushes.prev        = active_brushes.prev;
    active_brushes.next->prev    = &filtered_brushes;
    active_brushes.prev->next    = &filtered_brushes;
    // …then move the selection into active_brushes (show only the selection)…
    active_brushes.next          = selected_brushes.next;
    active_brushes.prev          = selected_brushes.prev;
    selected_brushes.next->prev  = &active_brushes;
    selected_brushes.prev->next  = &active_brushes;
    // …and empty the selection list.
    selected_brushes.next        = &selected_brushes;
    selected_brushes.prev        = &selected_brushes;
    g_nUpdateBits = -1;
}


// 0x487950  Map_GetNextAutoTarget — 1 + the highest numeric suffix among all
// "targetname"/"target" epairs beginning "auto".
int Map_GetNextAutoTarget()
{
    int maxId = 0;

    for ( entity_s *e = entities.next; e != &entities; e = e->next )
    {
        for ( epair_t *p = e->epairs; p; p = p->next )
        {
            const char *key = p->key;
            if ( _stricmp( key, "targetname" ) != 0 && _stricmp( key, "target" ) != 0 )
                continue;
            const char *tn = p->value;
            iassert( tn );
            if ( tn && !strncmp( tn, "auto", 4 ) )
            {
                int v = (int)atol( tn + 4 );
                if ( v > maxId ) maxId = v;
            }
        }
    }

    return maxId + 1;
}


// 0x487A70  Map_GetNextExportId — 1 + the highest "export" epair value, as a VA string.
char *Map_GetNextExportId( int slot )
{
    int maxId = 0;

    for ( entity_s *e = entities.next; e != &entities; e = e->next )
    {
        for ( epair_t *p = e->epairs; p; p = p->next )
        {
            if ( _stricmp( p->key, "export" ) != 0 )
                continue;
            const char *tn = p->value;
            iassert( tn );
            if ( tn && *tn )
            {
                int v = (int)atol( tn );
                if ( v > maxId ) maxId = v;
            }
        }
    }

    return VA( slot, "%i", maxId + 1 );
}


// 0x488DF0  Entity_WriteSelected_R — write every entity to a MemFile writer.
// IDA __usercall, ebx = WriteWriter_t (WriteFunc_t**); the writer ptr is also its own ctx.
void Entity_WriteSelected_R( WriteFunc_map_t **writer )
{
    iassert( entities.next != &entities );   // map.cpp:1323
    iassert( !strcmp( entities.next->eclass->name, "worldspawn" ) );   // map.cpp:1324 (0x488e22 derefs eclass unconditionally)

    int entIdx = 0;
    for ( entity_s_def *e = (entity_s_def *)entities.next;
          e != (entity_s_def *)&entities;
          e  = (entity_s_def *)e->next, ++entIdx )
    {
        WriteFunc_map_t *fn = *writer;
        fn( (int)(intptr_t)writer, "// entity %i\n", entIdx );
        Entity_WriteSelected( e, writer );
    }
}


// 0x488FC0  sub_488FC0 (IDB Prefab_ClearLevelLayers) — FreePrefabLevel's tree walk, but
// instead of freeing it NULLs the cached def->modelClass of every already-realized instance
// of the prefab being entered.  a4 is that prefab's modelClass POINTER (sub_4890F0 passes
// (int)modelClass), not a numeric level; the compare base at 0x489089 is `def` (i[2]+0x64),
// NOT def->eclass.
static void sub_488FC0( entity_s *a1, entity_s *a2, entity_s *a3, int a4 )
{
    for ( entity_s *i = a1; i != a2; i = i->next )
    {
        entity_s_def *def = (entity_s_def *)i->def;
        if ( ( def->eclass->classtype & 0x10 ) == 0 )   // not ECLASS_PREFAB
            continue;
        int modelClass = (int)(intptr_t)def->modelClass;   // def+0x64
        if ( !modelClass )
            continue;
        if ( i == a3 )
            continue;
        if ( modelClass == a4 )
        {
            def->modelClass = nullptr;                       // def+0x64 = 0
        }
        else if ( i->prefab )
        {
            prefab_s *pf = (prefab_s *)i->prefab;
            sub_488FC0( pf->next_entity, (entity_s *)pf, a3, a4 );
        }
    }
}

// 0x4890F0  sub_4890F0 — free every realized instance of one prefab across the whole stack
// (each saved slot's instance list + the live entityInsts), then clear their def fields.
// IDA __usercall(ebx=skip-entity, edi=modelClass).
static void sub_4890F0( int skipEnt, int level )
{
    entity_s *a3 = (entity_s *)(intptr_t)skipEnt;
    // unk_25EB660 = &g_prefabStack[0].entityInstsNext (the binary's raw dword walk).
    for ( int v2 = 0; v2 < prefabStackLevel; ++v2 )
        FreePrefabLevel( g_prefabStack[v2].entityInstsNext, &entityInsts, a3, level );
    FreePrefabLevel( entityInsts.next, &entityInsts, a3, level );

    for ( int v5 = 0; v5 < prefabStackLevel; ++v5 )
        sub_488FC0( g_prefabStack[v5].entityInstsNext, &entityInsts, a3, level );
    sub_488FC0( entityInsts.next, &entityInsts, a3, level );
}


// 0x489030  FreePrefabLevel — recursively free the realized child instances of one prefab.
// a4 is the prefab's modelClass pointer, matched against def->modelClass (def+0x64); the
// compare base at 0x48908c is `def` (i[2]), NOT def->eclass.
void FreePrefabLevel( entity_s *a1, entity_s *a2, entity_s *a3, int a4 )
{
    for ( entity_s *i = a1; i != a2; i = i->next )
    {
        if ( !i->prefab )
            continue;

        entity_s_def *def = (entity_s_def *)i->def;
        {
            entity_s *e = i;                     // the binary's local
            iassert( e->def->eclass->nShowFlags & ECLASS_PREFAB );   // map.cpp:1416
        }

        if ( i == a3 )
            continue;

        int modelClass = (int)(intptr_t)def->modelClass;   // def+0x64

        if ( modelClass == a4 )
        {
            Entity_FreePrefab( i );
            if ( i->modelInst )
            {
                RemoveModelInstFromBuf( i->modelInst );
                i->modelInst = 0;
            }
            ++*(unsigned short *)&def->version_prob_wrong;  // 0x4890c1: ++def->version (word)
            // 0x4890c9: clear the def-brush's modelFailed byte (+0x4C).
            if ( i->brushes.ownerNext && i->brushes.ownerNext != (selbrush_t *)&i->brushes )
            {
                brush_t *bdef = i->brushes.ownerNext->def;
                if ( bdef )
                    bdef->modelFailed = 0;
            }
        }
        else
        {
            entity_s *subList = (entity_s *)((int *)i->prefab)[1];
            entity_s *subEnd  = (entity_s *)i->prefab;
            FreePrefabLevel( subList, subEnd, a3, a4 );
        }
    }
}


// 0x48CC70  Map_SaveFileToPerforce — open the save file, optionally via Perforce
// add / open-for-edit.  Returns the open FILE* or NULL.
FILE *Map_SaveFileToPerforce( const char *path, char a2 )
{
    if ( g_qeglobals.toggle_unk05 && a2 )
    {
        // The offer-to-add probe is gated on !FileExists (sub_4379C0, a Perforce-aware
        // existence check) — only offer when the file is not already known to p4.
        if ( !FileExists( path ) )
        {
            FILE *probe = fopen( path, "rb" );
            if ( !probe )
            {
                HWND activeWnd = GetActiveWindow();
                if ( MessageBoxA( activeWnd, "Add file to Perforce?\n", "Radiant", 4u ) == IDYES )
                    sub_437850( path );
            }
            else
            {
                fclose( probe );
            }
        }
    }

    FILE *result = fopen( path, "wb" );
    if ( !result )
    {
        FILE *probe = fopen( path, "rb" );
        if ( !probe )
        {
            Sys_Printf( "ERROR!!!! Couldn't open '%s'!\n", path );
            return nullptr;
        }
        fclose( probe );

        if ( !g_qeglobals.toggle_unk05 )
            return nullptr;

        HWND activeWnd = GetActiveWindow();
        if ( MessageBoxA( activeWnd,
                          "File is Read-Only!\n\nPress OK to open for edit in Perforce and continue save.\nPress Cancel to abort save.\n",
                          "Radiant", 0x31u ) == IDCANCEL )
        {
            Sys_Printf( "Save of '%s' canceled.\n", path );
            return nullptr;
        }

        char editPath[1028];
        strcpy( editPath, path );
        if ( sub_4377E0( editPath ) )
        {
            FILE *f2 = fopen( path, "wb" );
            if ( !f2 )
                Sys_Printf( "ERROR!!!! Couldn't open '%s'!\n", path );
            return f2;
        }
        else
        {
            Sys_Printf( "ERROR!!!! perforce failed to open for edit file: '%s'! Aborting save!\n", path );
            return nullptr;
        }
    }

    return result;
}


// 0x4844E0  MapFile_WriteEntity — write one entity block to fout.
// The binary's writer is `void(**v23[2])(...) = { &NormalFileWrapper::vftable, a2 }`;
// Brush_Write dispatches through (*writer)(writer, fmt, ...).  Modelled here by a
// static WriteFunc_map_t* over the FILE* — the vtable's only method is an fprintf.
char MapFile_WriteEntity( entity_s_def *a1, FILE *a2, char a3 )
{
    static FILE            *s_writer_file = nullptr;
    static WriteFunc_map_t *s_writer_fn   = nullptr;
    struct NormalFileWriter
    {
        static int Write( int ctx, const char *fmt, ... )
        {
            (void)ctx;
            va_list ap;
            va_start( ap, fmt );
            int r = vfprintf( s_writer_file, fmt, ap );
            va_end( ap );
            return r;
        }
    };

    s_writer_file = a2;
    s_writer_fn   = NormalFileWriter::Write;
    WriteFunc_map_t **writer = &s_writer_fn;

    // Region save: an info_player_start is replaced by the live camera position.
    if ( a3 )
    {
        const char *clsname = "";
        for ( epair_t *ep = a1->epairs; ep; ep = ep->next )
        {
            if ( _stricmp( ep->key, "classname" ) == 0 )
            {
                clsname = ep->value ? ep->value : "";
                break;
            }
        }

        if ( strcmp( clsname, "info_player_start" ) == 0 )
        {
            float camOri[3] = { 0, 0, 0 };
            float camAng[3] = { 0, 0, 0 };
            if ( g_pParentWnd && g_pParentWnd->m_pCamWnd )
            {
                camOri[0] = g_pParentWnd->m_pCamWnd->camera.origin[0];
                camOri[1] = g_pParentWnd->m_pCamWnd->camera.origin[1];
                camOri[2] = g_pParentWnd->m_pCamWnd->camera.origin[2];
                camAng[1] = g_pParentWnd->m_pCamWnd->camera.angles[1];
            }
            fprintf( a2, "{\n" );
            fprintf( a2, "\"classname\" \"info_player_start\"\n" );
            fprintf( a2, "\"origin\" \"%.1f %.1f %.1f\"\n", camOri[0], camOri[1], camOri[2] );
            return (char)fprintf( a2, "\"angles\" \"0 %.0f 0\"\n}\n", camAng[1] );
        }

        // Region filter (0x484608): walk the DEF-list — first = brushes.oprev
        // (entity+0x0C), sentinel = &def (entity+0x08), advance via brush_t.onext (+0x04)
        // — calling sub_4874F0 per node (0 == overlaps on all 3 axes).  Skipping is
        // UNCONDITIONAL when nothing overlaps; there is no fixed-size exception.
        bool anyInRegion = false;
        brush_t *precSentinel = (brush_t *)&a1->def;
        for ( brush_t *b = (brush_t *)a1->brushes.prev; b != precSentinel; b = b->onext )
        {
            bool in = true;
            for ( int ax = 0; ax < 3; ++ax )
            {
                if ( region_maxs[ax] < b->mins[ax] ||
                     region_mins[ax] > b->maxs[ax] )
                { in = false; break; }
            }
            if ( in ) { anyInRegion = true; break; }
        }

        if ( !anyInRegion )
            return 0;
    }

    fprintf( a2, "{\n" );

    // Fixed-size entity: emit origin + optional layer line.  The per-entity layer is the
    // bbox brush's parent_layer_string (brush+0x48); 0x4846ac strcmps it against
    // "000_Global" BEFORE null-checking it, and only writes the line on mismatch.
    if ( a1->eclass && a1->eclass->fixedsize )
    {
        char originStr[132];
        sprintf( originStr, "%.1f %.1f %.1f",
                 a1->origin[0], a1->origin[1], a1->origin[2] );
        SetKeyValue( a1, "origin", originStr );

        const char *layerName = ( (brush_t *)a1->brushes.prev )->parent_layer_string;
        if ( strcmp( layerName, "000_Global" ) != 0 )
        {
            // KEEP_VERBOSE: inlined MapLoad_ParseBrush_Layer (engine_stubs 0x42fb40 is the
            // assert carrier) — this writer is a bare FILE*/fprintf, not the writer callback
            // the helper takes, so the call can't be routed through it.
            if ( !layerName )
                Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\common\\mapparsing.cpp",
                        190, 0, "%s", "layerName" );
            fprintf( a2, "layer \"%s\"\n", layerName );
        }
    }

    // Epairs as `"key" "value"` — Project_WriteQuotedString (0x484460) escapes \ and ".
    for ( epair_t *ep = a1->epairs; ep; ep = ep->next )
    {
        auto writeQuoted = [&]( const char *s )
        {
            if ( !s ) s = "";
            fputc( '"', a2 );
            for ( const char *c = s; *c; ++c )
            {
                if ( *c == '\\' || *c == '"' )
                    fputc( '\\', a2 );
                fputc( *c, a2 );
            }
            fputc( '"', a2 );
        };

        writeQuoted( ep->key );
        fputc( ' ', a2 );
        writeQuoted( ep->value );
        fputc( '\n', a2 );
    }

    // Brush blocks for non-fixed-size entities — same DEF-list walk as the region
    // filter above; each node IS the brush_t def.
    if ( a1->eclass && !a1->eclass->fixedsize )
    {
        brush_t *sentinel = (brush_t *)&a1->def;
        int      brushIdx = 0;
        for ( brush_t *b = (brush_t *)a1->brushes.prev; b != sentinel; b = b->onext )
        {
            if ( a3 )
            {
                // Region save: skip brushes entirely outside the region box.
                bool in = true;    // 0x484768, inline sub_4874F0
                for ( int ax = 0; ax < 3; ++ax )
                {
                    if ( region_maxs[ax] < b->mins[ax] || region_mins[ax] > b->maxs[ax] )
                    { in = false; break; }
                }
                if ( !in )
                    continue;
            }
            fprintf( a2, "// brush %i\n", brushIdx++ );
            Brush_Write( writer, b );
        }
    }

    return (char)fprintf( a2, "}\n" );
}


// 0x489190  Prefab_NextLevel — push one level onto the prefab edit stack: save the current
// map state into the 2168-byte (0x878) slot, then swap the live world lists (entities /
// entityInsts / active_brushes) to the selected misc_prefab's instanced sub-map so it can be
// edited in place.  Prefab_PrevLevel reads back the same slot layout.
// a1 = a brush instance to enter directly; NULL enters the single selected misc_prefab
// (CMainFrame::OnPrefabEnter passes NULL).  MFC paths are guarded on g_pParentWnd.
void Prefab_NextLevel( void *a1 )
{
    selbrush_t *sb = (selbrush_t *)a1;

    // Precondition: an explicit brush, OR exactly one selected ECLASS_PREFAB instance.
    bool ok = ( a1 != nullptr );
    if ( !ok )
    {
        selbrush_t *sel = selected_brushes.next;
        if ( sel != &selected_brushes &&
             sel->next == &selected_brushes &&                       // exactly one selected
             sel->owner &&
             sel->owner->def &&
             ( ((entity_s *)sel->owner->def)->eclass->classtype & 0x10 ) != 0 )
        {
            ok = true;
        }
    }
    if ( !ok )
    {
        Sys_Printf( "Must have a single prefab selected.\n" );
        return;
    }

    if ( prefabStackLevel == 16 )
    {
        Sys_Printf( "Max prefab nesting exceeded.\n" );
        return;
    }

    // 0x489206 Sys_PrintActiveBrushes("before entering prefab") — NOT a console line: it
    // repopulates the debug brush-list window, exactly as Prefab_PrevLevel does inline.
    MainFrm_BrushList( (int)VA( 0, "%s - active_brushes", "before entering prefab" ), &active_brushes );
    MainFrm_BrushList( (int)VA( 1, "%s - active_brushes", "before entering prefab" ), &selected_brushes );
    MainFrm_EntList( &entityInsts, "before entering prefab" );

    // Resolve the brush instance + its prefab.
    selbrush_t *v1;
    if ( a1 )
        v1 = sb;
    else
        v1 = selected_brushes.next;

    entity_s  *owner  = v1->owner;
    prefab_s  *prefab = (prefab_s *)owner->prefab;
    if ( !prefab )
    {
        Sys_Printf( "Prefab not loaded.\n" );
        return;
    }

    // The prefab's DEFINITION entity-list container (modelClass->model, models_t.x88
    // @+0x160), whose x2(+8)/entities.next(+12) are the def `entities` sentinel head.
    entity_s_def *def        = (entity_s_def *)owner->def;
    entitymodel_t *modelClass = def->modelClass;
    models_t *defEntsRoot    = modelClass->model;             // models_t.x88 slot
    entity_s *defEntsNext    = (entity_s *)defEntsRoot->entities.next;
    int       defEntsPrev    = defEntsRoot->x2;

    // 0x489267: drop the cached light previews — they belong to the parent map's brushes,
    // which are about to be swapped out (same reset as CMainFrame::OnClearPreviewList).
    if ( g_pParentWnd && g_pParentWnd->m_pCamWnd )
        g_pParentWnd->m_pCamWnd->light_preview_count = 0;

    // Free any previously-realized prefab instances for this owner (recursive).
    sub_4890F0( (int)v1->owner, (int)modelClass );

    if ( !a1 )
        Select_Deselect( 1 );

    Eclass_FreeModel( (void **)modelClass );

    // ── save current world state into the (pre-increment) stack slot ──────────
    const int oldLevel = prefabStackLevel;
    prefabLevel_t *slot = &g_prefabStack[prefabStackLevel];
    ++prefabStackLevel;

    slot->regionActive  = g_region_active;
    slot->regionMins[0] = region_mins[0];
    slot->regionMins[1] = region_mins[1];
    slot->regionMins[2] = region_mins[2];
    slot->regionMaxs[0] = region_maxs[0];
    slot->regionMaxs[1] = region_maxs[1];
    slot->regionMaxs[2] = region_maxs[2];
    if ( g_region_active )
        Map_RegionOff();

    strcpy( slot->mapName, currentmap );
    slot->modified = modified;

    // List heads (PrevLevel reads these back).
    slot->activeNext      = active_brushes.next;
    slot->activePrev      = active_brushes.prev;
    slot->entitiesNext    = entities.next;
    slot->entitiesPrev    = entities.prev;
    slot->entityInstsPrev = entityInsts.prev;
    slot->entityInstsNext = entityInsts.next;
    slot->prefabBrush     = v1;

    // Layer snapshot — the binary (sub_489810) deep-copies the layer RB-tree into slot+2088;
    // the port models layerMap as a std::map, so this collapses to a per-level snapshot.
    Layers_SavePrefabLayers( oldLevel );

    strcpy( slot->activeLayer, g_activeLayer_string );

    if ( a1 )
    {
        slot->selectedNext = selected_brushes.next;
        slot->selectedPrev = selected_brushes.prev;
    }
    else
    {
        slot->selectedNext = nullptr;
        slot->selectedPrev = nullptr;
    }

    // ── GUI camera / XY reframe (guarded; NULL in the headless gate) ──────────
    if ( g_pParentWnd && g_pParentWnd->m_pXYWnd && g_pParentWnd->m_pCamWnd )
    {
        CXYWnd  *xy  = g_pParentWnd->m_pXYWnd;
        CCamWnd *cam = g_pParentWnd->m_pCamWnd;

        // Save the parent view (PrevLevel reads these back at +525..534).
        slot->xyOrigin[0]  = xy->m_vOrigin[0];
        slot->xyOrigin[1]  = xy->m_vOrigin[1];
        slot->xyScale      = xy->m_fScale;
        slot->xyViewType   = (int)xy->m_nViewType;
        slot->camOrigin[0] = cam->camera.origin[0];
        slot->camOrigin[1] = cam->camera.origin[1];
        slot->camOrigin[2] = cam->camera.origin[2];
        slot->camAngles[0] = cam->camera.angles[0];
        slot->camAngles[1] = cam->camera.angles[1];
        slot->camAngles[2] = cam->camera.angles[2];

        // Reframe the live camera into the prefab's local space.  entAxis is the entity
        // orientation matrix, origin-first [4][3] (row0 = origin, rows 1..3 = rotation).
        //   sub_4BA610:  localPt[i] = row(i+1) · (worldPt − origin)    (inverse transform)
        //   sub_4BA6B0:  rotated[i] = row(i+1) · dir
        float entAxis[4][3];
        {
            float angles[3] = { 0, 0, 0 };
            Entity_GetVec3ForKey( (entity_s_def *)v1->owner->def, angles, "angles" );
            AnglesToAxis( angles, entAxis + 1 );
            entAxis[0][0] = v1->owner->origin[0];
            entAxis[0][1] = v1->owner->origin[1];
            entAxis[0][2] = v1->owner->origin[2];
        }
        // localOrigin = rows1..3 · (camOrigin − entOrigin)   [== sub_4BA610]
        float d[3] = { cam->camera.origin[0] - entAxis[0][0],
                       cam->camera.origin[1] - entAxis[0][1],
                       cam->camera.origin[2] - entAxis[0][2] };
        float localOrigin[3];
        localOrigin[0] = entAxis[1][0] * d[0] + entAxis[1][1] * d[1] + entAxis[1][2] * d[2];
        localOrigin[1] = entAxis[2][0] * d[0] + entAxis[2][1] * d[1] + entAxis[2][2] * d[2];
        localOrigin[2] = entAxis[3][0] * d[0] + entAxis[3][1] * d[1] + entAxis[3][2] * d[2];

        float camAxis[3][3];
        AnglesToAxis( cam->camera.angles, camAxis );
        float rotAxis[3][3];
        VectorRotateByAxis( rotAxis[0], &entAxis[0][0], camAxis[0] );
        VectorRotateByAxis( rotAxis[1], &entAxis[0][0], camAxis[1] );
        VectorRotateByAxis( rotAxis[2], &entAxis[0][0], camAxis[2] );

        float newAngles[3];
        AxisToAngles( newAngles, rotAxis );
        // Wrap yaw to [0,360): the binary's *angles*(1/360)→frac→*360 normalisation.
        float wrapped = newAngles[0] * 0.002777777845039964f;   // /360
        newAngles[0] = ( wrapped - floorf( wrapped + 0.5f ) ) * 360.0f;
        newAngles[2] = 0.0f;

        cam->camera.origin[0] = localOrigin[0];
        cam->camera.origin[1] = localOrigin[1];
        cam->camera.origin[2] = localOrigin[2];
        cam->camera.angles[0] = newAngles[0];
        cam->camera.angles[1] = newAngles[1];
        cam->camera.angles[2] = newAngles[2];

        // Re-centre the 2D view: 0x48951d zeroes exactly the TWO in-plane m_vOrigin
        // components — indices (viewType==0) and (viewType!=2)+1 — depth axis untouched.
        int vt = (int)xy->m_nViewType;
        xy->m_vOrigin[ (vt == 0) ? 1 : 0 ] = 0.0f;   // edx = (viewType==0)
        xy->m_vOrigin[ (vt != 2) ? 2 : 1 ] = 0.0f;   // esi = (viewType!=2)+1
    }

    // ── swap the live world lists to the prefab's sub-map ─────────────────────
    entities.prev = (entity_s *)(intptr_t)defEntsPrev;
    entities.next = defEntsNext;
    defEntsNext->prev = &entities;
    ( (entity_s *)(intptr_t)defEntsPrev )->next = &entities;

    entityInsts.next = prefab->next_entity;
    entityInsts.prev = prefab->prev_entity;
    prefab->next_entity->prev = &entityInsts;
    prefab->prev_entity->next = &entityInsts;

    world_entity = prefab->next_entity;

    active_brushes.next = prefab->active_brushlist_next;
    active_brushes.prev = prefab->active_brushlist;
    active_brushes.next->prev = &active_brushes;
    active_brushes.prev->next = &active_brushes;

    // 0x4895c1: the reset RUNS when a1 != 0 (explicit-brush entry — the selection was just
    // saved into the slot).  On the a1==0 path Select_Deselect(1) above already emptied it.
    if ( a1 )
    {
        selected_brushes.next = &selected_brushes;
        selected_brushes.prev = &selected_brushes;
    }

    // Stale the version stamps so every brush/patch/entity re-displays in the sub-map.
    for ( selbrush_t *i = active_brushes.next; i != &active_brushes; i = i->next )
    {
        i->version = (__int16)( i->def->version - 1 );
        // 0x4895ef also stamps the patch INSTANCE's cached version (patch_t+0x04) from the
        // patch DEF version (patchMesh_t.version @0x5040).
        if ( i->patch )
            i->patch->version = (__int16)( i->patch->def->version - 1 );
    }
    for ( entity_s *j = entityInsts.next; j != &entityInsts; j = j->next )
    {
        // def version is the word at def->version_prob_wrong (+0x78); the instance
        // version is the low word of entity_s.version (+0x4C).
        unsigned short dv = *(unsigned short *)&j->def->version_prob_wrong;
        *(unsigned short *)&j->version = (unsigned short)( dv - 1 );
    }

    modified = 0;

    // Build currentmap = "<project mapspath>\<prefab model>" with '/' separators.
    {
        const char *mapsPath = "";
        for ( epair_t *e = g_qeglobals.d_project_entity ? g_qeglobals.d_project_entity->epairs : nullptr;
              e; e = e->next )
        {
            if ( !_stricmp( e->key, "mapspath" ) ) { mapsPath = e->value; break; }
        }
        strcpy( currentmap, mapsPath );
        size_t n = strlen( currentmap );
        if ( n && currentmap[n - 1] != '\\' )
        {
            currentmap[n] = '\\';
            currentmap[n + 1] = '\0';
            ++n;
        }
        // append the misc_prefab def's "model" epair (the referenced .map path)
        const char *model = "";
        for ( epair_t *e = ((entity_s_def *)v1->owner->def)->epairs;
              e; e = e->next )
        {
            if ( !_stricmp( e->key, "model" ) ) { model = e->value; break; }
        }
        strcat( currentmap, model );
        for ( char *k = currentmap; *k; ++k )
            if ( *k == '\\' ) *k = '/';
    }

    // The prefab_s head-pointers now live in the world sentinels, so the 0x54-byte head
    // itself is free-able.  Then invalidate the owner def's cached modelClass and bump its
    // version so the instance re-realizes on leave, and clear the def-brush's +0x4C byte.
    {
        entity_s     *owner2 = v1->owner;
        free( owner2->prefab );
        owner2->prefab = nullptr;
        entity_s_def *odef = (entity_s_def *)owner2->def;
        if ( odef )
        {
            odef->modelClass = nullptr;                    // re-realize on leave
            ++*(unsigned short *)&odef->version_prob_wrong;// ++version (word)
        }
        if ( v1->def )
            v1->def->modelFailed = 0;                      // brush_t+0x4C low byte
    }

    Map_InitlLayers();
    g_qeglobals.g_layerCount_maybe = 1;
    Sys_Printf( "Updating layers...\n" );
    Layers_SetMapLayers();
    Layers_02();
    Undo_Clear();
    g_nUpdateBits = -1;
    SetWindowTextA( g_qeglobals.d_hwndMain, currentmap );

    MainFrm_BrushList( (int)VA( 0, "%s - active_brushes",   "after entering prefab" ), &active_brushes );
    MainFrm_BrushList( (int)VA( 1, "%s - active_brushes",   "after entering prefab" ), &selected_brushes );
    MainFrm_EntList( &entityInsts, "after entering prefab" );
}


// 0x489890  Prefab_PrevLevel — pop one prefab-edit level, restoring the saved map state.
// MFC paths (camera, XY views, surface inspector) are guarded on g_pParentWnd.
void Prefab_PrevLevel()
{
    iassert( prefabStackLevel > 0 );

    MainFrm_BrushList( (int)VA( 0, "%s - active_brushes",   "before leaving prefab" ), &active_brushes );
    MainFrm_BrushList( (int)VA( 1, "%s - active_brushes", "before leaving prefab" ), &selected_brushes );
    MainFrm_EntList( &entityInsts, "before leaving prefab" );

    Select_Deselect( 1 );
    Brush_FreeMapBrushes();
    --prefabStackLevel;

    // Restore from the (already decremented) level's slot.
    prefabLevel_t *slot = &g_prefabStack[prefabStackLevel];

    entities.next = slot->entitiesNext;
    entities.prev = slot->entitiesPrev;

    iassert( entities.next->prev == &entities );
    iassert( entities.prev->next == &entities );

    entityInsts.prev = slot->entityInstsPrev;
    entityInsts.next = slot->entityInstsNext;

    iassert( entityInsts.next->prev == &entityInsts );
    iassert( entityInsts.prev->next == &entityInsts );

    active_brushes.next = slot->activeNext;
    active_brushes.prev = slot->activePrev;

    world_entity = entityInsts.next;

    iassert( active_brushes.next->prev == &active_brushes );
    iassert( active_brushes.prev->next == &active_brushes );

    if ( slot->selectedNext )
    {
        selbrush_t *sn = slot->selectedNext;
        selbrush_t *sp = slot->selectedPrev;
        if ( sp )
        {
            selected_brushes.next = sn;
            selected_brushes.prev = sp;
        }
    }

    strncpy( currentmap, slot->mapName, 1024 );
    currentmap[1023] = '\0';

    modified        = slot->modified;
    g_region_active = slot->regionActive;

    region_mins[0] = slot->regionMins[0];
    region_mins[1] = slot->regionMins[1];
    region_mins[2] = slot->regionMins[2];
    region_maxs[0] = slot->regionMaxs[0];
    region_maxs[1] = slot->regionMaxs[1];
    region_maxs[2] = slot->regionMaxs[2];

    // The prefab brush the user was editing.
    entity_brush_s *prefabBrush = (entity_brush_s *)slot->prefabBrush;
    Model_SetModel( prefabBrush, (int)&world_orient_matrix );
    Map_InitlLayers();

    // Restore the parent map's layers (binary: sub_41A5A0 clears the live RB-tree, then
    // sub_489F30 deep-copies it back from slot+2088).
    Layers_RestorePrefabLayers( prefabStackLevel );

    // 0x489b61: reload the prefab brush's "model" layers, then refresh the live layer set.
    {
        const char  *model = "";   // binary's `zero` default when there is no "model" key
        entity_s_def *pdef  = (entity_s_def *)prefabBrush->owner->def;
        for ( epair_t *ep = pdef->epairs; ep; ep = ep->next )
        {
            if ( _stricmp( ep->key, "model" ) == 0 ) { model = ep->value; break; }
        }
        sub_418A50( model );
        Sys_Printf( "Updating layers...\n" );
        Layers_SetMapLayers();
        Layers_02();
    }

    // Active layer name
    strncpy( g_activeLayer_string, slot->activeLayer, 255 );
    g_activeLayer_string[255] = '\0';
    sub_41C9C0( g_activeLayer_string );

    // Camera restore
    if ( g_pParentWnd && g_pParentWnd->m_pCamWnd )
    {
        g_pParentWnd->m_pCamWnd->camera.origin[0] = slot->camOrigin[0];
        g_pParentWnd->m_pCamWnd->camera.origin[1] = slot->camOrigin[1];
        g_pParentWnd->m_pCamWnd->camera.origin[2] = slot->camOrigin[2];
        g_pParentWnd->m_pCamWnd->camera.angles[0] = slot->camAngles[0];
        g_pParentWnd->m_pCamWnd->camera.angles[1] = slot->camAngles[1];
        g_pParentWnd->m_pCamWnd->camera.angles[2] = slot->camAngles[2];
    }

    if ( g_pParentWnd && g_pParentWnd->m_pXYWnd )
    {
        // 0x489c23: PositionView() runs BEFORE the m_vOrigin/m_fScale restore — it
        // overwrites m_vOrigin[nDim1/nDim2], so the restore afterwards is what wins.
        g_pParentWnd->m_pXYWnd->PositionView();
        g_pParentWnd->m_pXYWnd->m_vOrigin[0] = slot->xyOrigin[0];
        g_pParentWnd->m_pXYWnd->m_vOrigin[1] = slot->xyOrigin[1];
        // BINARY BUG (faithful): 0x489c46/0x489c4f load the scale slot TWICE — m_vOrigin[2]
        // gets the saved m_fScale.  NextLevel only ever saved origin[0]/[1]/scale.
        g_pParentWnd->m_pXYWnd->m_vOrigin[2] = slot->xyScale;
        g_pParentWnd->m_pXYWnd->m_fScale      = slot->xyScale;
        g_pParentWnd->m_pXYWnd->SetViewType( (CXYWnd::EViewType)slot->xyViewType );
    }

    // 0x489c67: `if (surfDlgGlob.hwnd) { SetTexMods(); Select_SetTexture_2(&g_dlgSurface); }` —
    // Surf_UpdateInspector is exactly that pair, and a no-op when the inspector is closed.
    Surf_UpdateInspector();
    if ( g_pParentWnd && g_pParentWnd->m_hWnd )
    {
        if ( g_pParentWnd->m_wndTextureBar.m_hWnd )
            CTextureBar::GetSurfaceAttributes( &g_pParentWnd->m_wndTextureBar );
    }

    sub_47D060( (int)&active_brushes );
    sub_47D060( (int)&selected_brushes );
    sub_47D060( (int)&filtered_brushes );
    CopySelectedFaceValues();

    if ( g_region_active )
        Map_ApplyRegion();

    Undo_Clear();
    g_nUpdateBits = -1;
    SetWindowTextA( g_qeglobals.d_hwndMain, currentmap );

    MainFrm_BrushList( (int)VA( 0, "%s - active_brushes",   "after leaving prefab" ), &active_brushes );
    MainFrm_BrushList( (int)VA( 1, "%s - active_brushes", "after leaving prefab" ), &selected_brushes );
    MainFrm_EntList( &entityInsts, "after leaving prefab" );
}


// 0x489D50  Prefab_LevelBack — pop every prefab level.
void Prefab_LevelBack()
{
    while ( true )
    {
        iassert( prefabStackLevel >= 0 );
        if ( prefabStackLevel <= 0 )
            break;
        Prefab_PrevLevel();
    }
}


// 0x42BF70  CMainFrame::OnPrefabEnter (menu 33173, Edit→Enter Prefab) — body lives here
// with Prefab_NextLevel.
void CMainFrame::OnPrefabEnter()
{
    Prefab_NextLevel( nullptr );
}

// 0x42BF80  CMainFrame::OnPrefabLeave — likewise, with Prefab_PrevLevel.
void CMainFrame::OnPrefabLeave()
{
    iassert( prefabStackLevel >= 0 );
    if ( prefabStackLevel > 0 && ( !modified || ConfirmModified() ) )
        Prefab_PrevLevel();
}

// 0x488C70  Map_ImportFile — File→Load: merge another .map into the current one.  Runs the
// shared parse session (spaceDelimited=0 / negativeNumbers=1, plus the iwmap<4 space-delimit
// fixup), places the entities/brushes via Map_ImportBuffer, then rebuilds brush data.
// The binary's backslash→slash loop (0x488cb5) writes into a scratch buffer it never reads
// AND only stores on the '\\' branch — LoadFile/Com_BeginParseSession both take the RAW
// path.  Not reproduced; `path` is passed through as the binary does.
extern int   LoadFile( const char *filename, void **buf );      // cmdlib.cpp
extern void  Com_BeginParseSession( const char *name );         // q_parse
extern int   Map_ReadVersion( const void **text );              // layers.cpp (sub_4861F0)
extern char *Map_ImportBuffer( const char **text, int version );// defined below (0x487C90)

void Map_ImportFile( const char *path )
{
    HCURSOR wait = SetCursor( LoadCursorA( 0, (LPCSTR)IDC_WAIT ) );

    void *buf = nullptr;
    bool didImport = false;   // LABEL_15 (updateBits/modified) runs on LoadFile-FAIL or on a successful import
    if ( LoadFile( path, &buf ) == -1 )
    {
        didImport = true;     // binary `goto LABEL_15` on load failure — odd, but faithful
    }
    else
    {
        Com_BeginParseSession( path );
        ParseThreadInfo *pi = Com_GetParseThreadInfo();
        pi->parseInfo[pi->parseInfoNum].spaceDelimited  = 0;
        pi->parseInfo[pi->parseInfoNum].negativeNumbers = 1;

        const void *textPtr = buf;
        int mapVer = Map_ReadVersion( &textPtr );
        if ( mapVer >= 0 )
        {
            if ( mapVer < 4 )
                pi->parseInfo[pi->parseInfoNum].spaceDelimited = 1;
            Map_ImportBuffer( (const char **)&textPtr, mapVer );

            ParseThreadInfo *pi2 = Com_GetParseThreadInfo();
            if ( !pi2->parseInfoNum )
                Com_Error( ERR_FATAL, "Com_EndParseSession: session underflow" );
            --pi2->parseInfoNum;
            free( buf );
            Map_BuildBrushData();
            didImport = true;
        }
        else
        {
            // mapVer < 0: end session + free, but DON'T touch updateBits/modified (binary LABEL_16).
            ParseThreadInfo *pi2 = Com_GetParseThreadInfo();
            if ( !pi2->parseInfoNum )
                Com_Error( ERR_FATAL, "Com_EndParseSession: session underflow" );
            --pi2->parseInfoNum;
            free( buf );
        }
    }

    if ( didImport )
    {
        g_nUpdateBits = -1;
        modified = 1;
    }
    if ( wait )
        SetCursor( wait );
}

// ═════════════════════════════════════════════════════════════════════════════
//  RELOCATED HOME — this function's embedded Assert() calls name THIS file as
//  their source (see the brush.cpp relocation protocol / line-uniqueness test).
// ═════════════════════════════════════════════════════════════════════════════
// deps of the moved parse bodies (previously declared in entity.cpp):
extern void      Entity_Free_R( entity_s *e );                        // entity.cpp
extern void      Entity_UnlinkBrush( brush_t *b );                    // entity.cpp
extern entity_s *ParseEntity( const char **text, int version, char a2, char a3 );  // entity.cpp 0x487A30
extern void      Map_ParseLayers( const void **text, int skipFlags ); // 0x486280
extern bool      g_bScreenUpdates;                                    // entity.cpp
// deps (duplicated decls are benign; the parse/undo machinery lives across TUs):
extern void        Select_Deselect( int bDeselectFaces );                  // select.cpp 0x48E800
extern void        Select_Brush( selbrush_t *b, char some_overwrite, char bStatus, char center ); // select.cpp 0x48DCC0
extern void        Undo_ClearRedo();                                       // undo.cpp 0x45DF20
extern void        Undo_GeneralStart( const char *op );                    // undo.cpp 0x45E3F0
extern void        Undo_End();                                             // undo.cpp 0x45EA20
extern bool        Model_SetModel( entity_brush_s *b, int orientMatrix );  // brush.cpp 0x478780
extern undo_s     *g_lastundo;                                             // undo.cpp 0x23F162C

// ─────────────────────────────────────────────────────────────────────────────
// 0x486500  Map_LoadEntities  (map.cpp:304 asserts — now in their own file)
// Loads entities from a file into the given doubly-linked entity list.
// 0x486500: TAIL splice, ParseEntity(mapVer,0,1),
// parse-session setup/teardown all match. 304/305 converted to iassert (same-file after relocation); 690 CONVERTED.
// ─────────────────────────────────────────────────────────────────────────────
int Map_LoadEntities( const char *filename, entity_s *entList, char a3 )
{
    iassert( entList->next == entList );   // map.cpp:304
    iassert( entList->prev == entList );   // map.cpp:305

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

            extern void IncRef( entity_s *e, entity_s *list );   // entity.cpp 0x483bf0
            for ( entity_s *e = ParseEntity( (const char **)&textPtr, mapVer, 0, 1 );
                  e;
                  e = ParseEntity( (const char **)&textPtr, mapVer, 0, 1 ) )
            {
                ++count;
                IncRef( e, entList );                    // inlined in the binary
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
    for ( entity_s *entInst = entityInsts.next; entInst != &entityInsts; entInst = entInst->next )
    {
        iassert( entInst );   // map.cpp:1242
        entity_s_def *iDef = (entity_s_def *)entInst->def;
        iassert( entInst->def );   // map.cpp:1243
        iassert( entInst->def->eclass );   // map.cpp:1244
        if ( ( iDef->eclass->classtype & 0x10 ) != 0 )
        {
            entity_brush_s *fb = entInst->brushes.ownerNext;
            if ( fb != &entInst->brushes )
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
