#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Maya MEL geometry exporter for selected brushes, patches, and prefabs.

#include "stdafx.h"
#include "winding.h"
#include "prefs.h"

// ─── editor geometry types (qe3.h) ──────────────────────────────────────────
//   selbrush_t (56)  brush instance / selection list node     (def@0x14, faces@0x1C, patch@0x20)
//   brush_t    (88)  brush definition (geometry)              (faces@0x44)
//   face_t     (232) one brush face                           (mtldef@0x24, plane@0xC0, w@0xE0)
//   faceVis_s  (12)  per-instance face visibility
//   winding_t        { int numpoints; float p[][3]; }         (points @ offset 4)
//   entity_s_def     entity definition                        (eclass@0x60, origin@0x68, epairs@0x74)
// All declared in qe3.h via stdafx.h.

// ─── patch INSTANCE (IDB pPatch_t, 68 bytes) — only the fields Maya export reads ──
//   off 0x00 mesh        patchMesh_t*  (its curveDef @ +0x5038 holds the tessellated verts)
//   off 0x0C indexCount  int           (built by PMESH_24 = (cw-1)*(6*ch-6))
//   off 0x10 indices     unsigned short* (forward tri index list, 3 per tri)
// Verified against PMESH_24 (0x440213) which writes a1[3]=indexCount, a1[4]=indices.
struct edPatchInst_t
{
    patchMesh_t    *mesh;        // 0x00
    char            pad04[8];    // 0x04
    int             indexCount;  // 0x0C
    unsigned short *indices;     // 0x10
    char            pad14[48];   // 0x14 .. 0x44 (revIndices @0x14 + tail; unused here)
};
static_assert(sizeof(edPatchInst_t) == 68, "edPatchInst_t (pPatch_t) != 68");
static_assert(offsetof(edPatchInst_t, indexCount) == 12, "edPatchInst_t.indexCount");
static_assert(offsetof(edPatchInst_t, indices) == 16, "edPatchInst_t.indices");

// ─── prefab_s (IDB 0x54) — mirror of entity.cpp/brush.cpp.  Maya export only reads
//     the instanced-brush list head/tail sentinels (the prefab-render lineage). ──────
struct prefab_s
{
    entity_s   *prev_entity;            // 0x00
    entity_s   *next_entity;            // 0x04
    void       *unk;                    // 0x08
    selbrush_t *active_brushlist;       // 0x0C  tail sentinel (prev side)
    selbrush_t *active_brushlist_next;  // 0x10  head sentinel (next side)
    char        _pad[0x54 - 0x14];      // 0x14 .. 0x53
};
static_assert(sizeof(prefab_s) == 0x54, "prefab_s (mayaexport mirror != entity.cpp)");

// ─── externs (radiant target) ───────────────────────────────────────────────
extern void  Assert( const char *file, int line, int type, const char *fmt, ... );

extern selbrush_t selected_brushes;                 // 0x23F1864 (display-list sentinel)
extern float      world_orient_matrix[4][3];        // 0x6DE290  (identity world orientation)

// brush.cpp
extern char  FilterBrush( selbrush_t *b, int flag );
extern void *PlanePts_Alloc( int count );
extern void  Vis_Free( int count, faceVis_s *f, int brushPtr );
extern void  Face_MoveTexture( int surfDef, const float *normal, int outVecs,
                               int uvBase, float rotate, float crossterm );
extern int   TexWnd_06_LayerCount( int mtlDef, int layerHandle );
extern void  sub_46F6C0( int mtlDef, int faceDef, int visIdx, int *outData );  // packed face colour

// draw.cpp
extern void  OrientationPosToWorldPos( float *out, const float *pos, const orientation_t *orient );
extern void  OrientationDirToWorldDir( float *out, const orientation_t *orient, const float *dir );

// entity.cpp
extern int   Entity_GetVec3ForKey( entity_s_def *e, float *out, const char *key );
extern bool  HasKeyValuePair( entity_s_def *e, const char *key );
extern void  Entity_GetOrientation( entity_s_def *ent, orientation_t *orParent, orientation_t *orOut );

// ─── progress counters (IDB dword_240A0EC = "this pass" count; dword_240A0E8 = total) ──
// Not file-local: ExportTo3D bumps s_mayaObjCount and its source file is brush.cpp (its
// asserts are brush.cpp:3499/3500), so it lives there and reads these through an extern.
int s_mayaObjCount = 0;   // dword_240A0EC
int s_mayaObjTotal = 0;   // dword_240A0E8

// Forward decls (mutual recursion via the prefab emitter).
static int  Patch_ExportToMaya( FILE *f, edPatchInst_t *patch, char groupAsBrush, char emitUVs, char polyList );
// ExportTo3D / ExportTo3D_CreatePolyFacet live in brush.cpp (asserts brush.cpp:241/242/
// 250/265/266 and 3499/3500 — brush.cpp is their source file).
extern char ExportTo3D( selbrush_t *b, const orientation_t *orient, FILE *f,
                        char groupAsBrush, char emitUVs, float scale );
static int  Prefab_ExportToMaya( selbrush_t *b, orientation_t *parentOrient, FILE *f,
                                 char groupAsBrush, char emitUVs, char polyList, float scale );

// ════════════════════════════════════════════════════════════════════════════
//  0x46FBE0  MtlDef_IsFaceFiltered — "is this face's material filtered out of the
//  view?" (returns 1 => SKIP the facet).  PORTED 2026-06-27 (faceTexMap landed).
//  IDA 0x46FBE0, faithful:
//    1. MtlDef_IsValid(mtlDef) assert; derive the face material NAME
//       (lyrMtl ? (char*)lyrMtl : radMtl->name) — same idiom as FilterCond_Material.
//    2. Walk g_qeglobals.d_filterGlobals.faceTexMap (std::map<string,int>) and return
//       1 if ANY key is a SUBSTRING of the face name (strstr(name, key)).  This is
//       the "filter faces by material" hide list, populated by RadiantFilters05/06_large.
//    3. If g_PrefsDlg->draw_toggle is set, also run the MaterialDef realize probe
//       (dword_181F51C=2 seed; MaterialDef_02(mtlDef, MaterialDef_06) ANDs each layer's
//       game-flag low word) — a survivor means the "special draw" flag set, also filter.
//    4. Else return 0 (emit the facet).
//  The faceTexMap walk + draw_toggle probe both reach REAL data now (the texture-filter
//  subsystem is wired); empty map + draw_toggle=0 still degenerates to "not filtered",
//  the default/headless path.
// ════════════════════════════════════════════════════════════════════════════
extern bool FaceTexMap_HasSubstringOf( const char *materialName );        // filters.cpp
extern void MaterialDef_02( MaterialDef *m, int (*cb)( qtexture_s * ) );   // 0x431520
extern int  MaterialDef_06( qtexture_s *radMtl );                         // 0x4318A0
extern int  dword_181F51C;                                               // realize-state seed

// Non-static so the light-region submit (brush.cpp Brush_DrawSubmitFaceWindings)
// can reach the binary's MtlDef_IsFaceFiltered (0x46FBE0) directly.
char MtlDef_IsFaceFiltered( MaterialDef *mtlDef )
{
    extern LayerMaterialDef *Materialdef_GetName( MaterialDef *m );   // materialdef.cpp 0x431640
    // the binary inlines Materialdef_GetName here (MaterialDef.cpp:85 lives in it)
    const char *name = (const char *)Materialdef_GetName( mtlDef );

    // faceTexMap substring filter (IDA: strstr(name, key) over the whole map).
    if ( FaceTexMap_HasSubstringOf( name ) )
        return 1;

    // draw_toggle realize-probe (IDA 0x46FCBB): special-draw flag survivor → filtered.
    if ( g_PrefsDlg->draw_toggle )
    {
        dword_181F51C = 2;
        MaterialDef_02( mtlDef, MaterialDef_06 );
        if ( dword_181F51C )
            return 1;
    }
    return 0;
}



// ════════════════════════════════════════════════════════════════════════════
//  0x4402C0  Patch_ExportToMaya — emit ONE patch.  polyList mode: one polyCreateFacet
//  per tessellated triangle (with per-corner polyEditUV) then polyUnite/merge/softEdge.
//  Non-polyList mode: a polyPlane sized to the tessellated grid + a per-vertex move.
// ════════════════════════════════════════════════════════════════════════════
static int Patch_ExportToMaya( FILE *f, edPatchInst_t *patch, char groupAsBrush,
                               char emitUVs, char polyList )
{
    fprintf( f, "\t{\r\n" );
    if ( groupAsBrush )
    {
        fprintf( f, "\t\tstring $strPolyList[];\r\n" );
        fprintf( f, "\t\tstring $strBrush[];\r\n\r\n" );
    }
    fprintf( f, "\t\tstring $strPolyInfo[];\r\n" );

    curvePatchDef_t *curve = patch->mesh->curveDef;        // [*patch + 0x5038]
    const char      *vbase = (const char *)curve->verts;   // [curve+0x0C]  curveVert_t[] (stride 44)

    if ( polyList )
    {
        const unsigned short *idx = patch->indices;        // [patch+0x10]
        for ( int i = 0; i < patch->indexCount; i += 3 )   // [patch+0x0C]
        {
            fprintf( f, "\t\t$strPolyInfo = `polyCreateFacet -ch off -tx 1 -s 1" );

            const float *v2 = (const float *)( vbase + 44 * idx[i + 2] );
            fprintf( f, " -p %f %f %f", v2[0], v2[1], v2[2] );
            const float *v1 = (const float *)( vbase + 44 * idx[i + 1] );
            fprintf( f, " -p %f %f %f", v1[0], v1[1], v1[2] );
            const float *v0 = (const float *)( vbase + 44 * idx[i] );
            fprintf( f, " -p %f %f %f", v0[0], v0[1], v0[2] );

            fprintf( f, "`;\r\n" );
            fprintf( f, "\t$strPolyList[%d] = $strPolyInfo[0];\r\n", i / 3 );

            if ( emitUVs )
            {
                // st @ curveVert_t+12 (u@12, v@16)
                fprintf( f, "\t\tpolyEditUV -r false -u %f -v %f ($strPolyInfo[0]).map[2];\r\n",
                         v2[3], v2[4] );
                fprintf( f, "\t\tpolyEditUV -r false -u %f -v %f ($strPolyInfo[0]).map[1];\r\n",
                         v1[3], v1[4] );
                fprintf( f, "\t\tpolyEditUV -r false -u %f -v %f ($strPolyInfo[0]).map[0];\r\n",
                         v0[3], v0[4] );
            }
        }
        if ( groupAsBrush )
        {
            fprintf( f, "\t\t$strBrush = `polyUnite -ch 0 ($strPolyList)`;\r\n" );
            fprintf( f, "\t\tpolyMergeVertex -d 0.01 -ch 0 $strBrush[0];\r\n" );
            fprintf( f, "\t\tpolySoftEdge -a 180 -ch 0 $strBrush[0];\r\n" );
            fprintf( f, "\t\tparent $strBrush[0] $strGroups[$iCurGroup];\r\n" );
        }
        else
        {
            fprintf( f, "\t\tparent $strPolyList $strGroups[$iCurGroup];\r\n" );
        }
    }
    else
    {
        int cw = curve->width;          // *curve
        int ch = curve->height;         // curve[1]
        int total = ch * cw;
        fprintf( f, "\t\t$strPolyInfo = `polyPlane -ch off -sx %d -sy %d`;\r\n", cw - 1, ch - 1 );
        for ( int i = 0; i < total; ++i )
        {
            const float *v = (const float *)( vbase + 44 * i );
            fprintf( f, "\t\tmove -ws %f %f %f ($strPolyInfo[0] + \".vtx[%d]\");\r\n",
                     v[0], v[1], v[2], i );
        }
        fprintf( f, "\t\tparent $strPolyInfo[0] $strGroups[$iCurGroup];\r\n" );
    }

    ++s_mayaObjCount;
    fprintf( f, "\t}\r\n" );
    fprintf( f, "\tprogressWindow -edit -s 1;\r\n" );
    return fprintf( f, "\tprogressWindow -edit -st \"%d of %d objects\";\r\n",
                    s_mayaObjCount, s_mayaObjTotal );
}

// ════════════════════════════════════════════════════════════════════════════
//  0x491960  Prefab_ExportToMaya — emit ONE prefab instance as a nested `group`.
//  Recurses into the prefab's own brush/patch/sub-prefab list (each in the prefab's
//  local orientation), then move/rotates the group to the instance's origin/angles.
// ════════════════════════════════════════════════════════════════════════════
static int Prefab_ExportToMaya( selbrush_t *b, orientation_t *parentOrient, FILE *f,
                                char groupAsBrush, char emitUVs, char polyList, float scale )
{
    // owner = b->owner (instance entity); its prefab_s holds the loaded brush instances,
    // and its DEF (owner->def) carries origin/angles.  (IDB: v18 =
    // owner->prefab @ owner+0x48; entity def = owner->def @ owner+0x08.)
    entity_s     *owner   = b->owner;
    prefab_s     *pf      = (prefab_s *)owner->prefab;
    entity_s_def *instDef = (entity_s_def *)owner->def;

    fprintf( f, "\t{\r\n" );
    fprintf( f, "\t\t$iCurGroup++;\r\n" );
    fprintf( f, "\t\t$strGroups[$iCurGroup] = `group -em`;\r\n" );

    orientation_t prefabOrient;
    Entity_GetOrientation( instDef, parentOrient, &prefabOrient );

    // Iterate the prefab's instanced brush list (active_brushlist_next .. &active_brushlist),
    // linked via selbrush_t.next — the same lineage as DrawBrush_PrefabContents.
    selbrush_t *sentinel = (selbrush_t *)&pf->active_brushlist;
    for ( selbrush_t *pb = pf->active_brushlist_next; pb && pb != sentinel; pb = pb->next )
    {
        if ( pb->owner->prefab )
            Prefab_ExportToMaya( pb, &prefabOrient, f, groupAsBrush, emitUVs, polyList, scale );
        else if ( pb->patch )
            Patch_ExportToMaya( f, (edPatchInst_t *)pb->patch, groupAsBrush, emitUVs, polyList );
        else
            ExportTo3D( pb, &prefabOrient, f, groupAsBrush, emitUVs, scale );
    }

    const float *o = instDef->origin;
    fprintf( f, "\t\tmove -x %f -y %f -z %f ($strGroups[$iCurGroup]);\r\n", o[0], o[1], o[2] );

    if ( HasKeyValuePair( instDef, "angles" ) )
    {
        float ang[3];
        if ( !Entity_GetVec3ForKey( instDef, ang, "angles" ) )
        {
            ang[0] = 0.0f; ang[1] = 0.0f; ang[2] = 0.0f;
        }
        fprintf( f, "\t\trotate -os %f %f %f ($strGroups[$iCurGroup]);\r\n", ang[2], ang[0], ang[1] );
    }

    fprintf( f, "\t\tparent $strGroups[$iCurGroup] $strGroups[$iCurGroup - 1];\r\n" );
    fprintf( f, "\t\t$iCurGroup--;\r\n" );
    return fprintf( f, "\t}\r\n" );
}

// ════════════════════════════════════════════════════════════════════════════
//  0x491B20  ExportToMaya — the two-pass driver.  Pass 1 writes `<dir>\dryRun.mel`
//  purely to COUNT the objects (so pass 2 can set progressWindow -max correctly).
//  Pass 2 writes `<dir>\<outName>` with the real geometry.  `dir` is "<appPath>\Maya"
//  in the binary; the caller supplies the full dir + name (decoupled from the prefs
//  CString punning — see the file header).
//
//  emitUVs       = IDC_MAYA_UVS        (bl in the binary; a4-side: per-vertex polyEditUV)
//  groupAsBrush  = IDC_MAYA_MERGE      (polyUnite the faces of a brush into one mesh)
//  polyList      = IDC_MAYA_TRIANGLES  (patches: per-tri facets vs a single polyPlane)
//  scale         = 1.0 (cm) or 2.54 (IDC_MAYA_UNIT_INCH)
// ════════════════════════════════════════════════════════════════════════════
extern "C" void ExportToMaya( const char *dir, const char *outName,
                              char emitUVs, char groupAsBrush, char polyList, float scale )
{
    char path[MAX_PATH];

    // ── PASS 1: dry run (count) ──────────────────────────────────────────────
    _snprintf( path, sizeof( path ), "%s\\Maya\\dryRun.mel", dir );
    FILE *f = fopen( path, "wt" );
    if ( !f )
        return;

    s_mayaObjTotal = s_mayaObjCount;     // (carry prior total; immaterial on first run)
    s_mayaObjCount = 0;
    fprintf( f, "{\r\n" );
    fprintf( f, "\tstring $strGroups[];\r\n" );
    fprintf( f, "\t$iCurGroup = 0;\r\n" );
    fprintf( f, "\t$strGroups[$iCurGroup] = `group -em`;\r\n" );
    fprintf( f, "\tprogressWindow -min 0 -max 1;\r\n" );

    for ( selbrush_t *bp = selected_brushes.next; bp != &selected_brushes; bp = bp->next )
    {
        if ( bp->owner->prefab )
            Prefab_ExportToMaya( bp, (orientation_t *)world_orient_matrix, f,
                                 groupAsBrush, emitUVs, polyList, scale );
        else if ( bp->patch )
            Patch_ExportToMaya( f, (edPatchInst_t *)bp->patch, groupAsBrush, emitUVs, polyList );
        else
            ExportTo3D( bp, (orientation_t *)world_orient_matrix, f, groupAsBrush, emitUVs, scale );
    }

    fprintf( f, "\tprogressWindow -edit -ep;\r\n" );
    fprintf( f, "}\r\n" );
    fclose( f );

    // ── PASS 2: real export ──────────────────────────────────────────────────
    _snprintf( path, sizeof( path ), "%s\\Maya\\%s", dir, outName );
    f = fopen( path, "wt" );
    if ( !f )
        return;

    fprintf( f, "{\r\n" );
    fprintf( f, "\tstring $strGroups[];\r\n" );
    fprintf( f, "\t$iCurGroup = 0;\r\n" );
    fprintf( f, "\t$strGroups[$iCurGroup] = `group -em`;\r\n" );
    fprintf( f, "\tprogressWindow -min 0 -max %d;\r\n", s_mayaObjCount );

    s_mayaObjTotal = s_mayaObjCount;     // pass-1 count is the pass-2 "of N" total
    s_mayaObjCount = 0;
    for ( selbrush_t *bp = selected_brushes.next; bp != &selected_brushes; bp = bp->next )
    {
        if ( bp->owner->prefab )
            Prefab_ExportToMaya( bp, (orientation_t *)world_orient_matrix, f,
                                 groupAsBrush, emitUVs, polyList, scale );
        else if ( bp->patch )
            Patch_ExportToMaya( f, (edPatchInst_t *)bp->patch, groupAsBrush, emitUVs, polyList );
        else
            ExportTo3D( bp, (orientation_t *)world_orient_matrix, f, groupAsBrush, emitUVs, scale );
    }

    fprintf( f, "\tprogressWindow -edit -ep;\r\n" );
    fprintf( f, "}\r\n" );
    fclose( f );
}
