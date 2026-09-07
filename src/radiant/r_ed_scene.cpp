#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\src\gfx_d3d\r_ed_scene.cpp
// Editor-only scene rendering — the per-frame surface cache that the editor draw
// paths (DrawGeo filled branch, R_SunPrev_Main, LightPreview_DrawLight) accumulate
// brush/model surfaces into, then flush as one RC_DRAW_EDITOR_SKINNEDCACHED command.
// No kisak equivalent.
//
// Adapted to kisak's backend API: tess (== binary tess_r), gfxCmdBufState/SourceState/
// Context, the R_SetupPass* family, R_SetIndexData/R_DrawIndexedPrimitive.  Only
// DrawGeo's filled branch (r_ed_surfcache dvar, default OFF) emits into this cache.

#include "stdafx.h"
#include <gfx_d3d/r_init.h>        // rg, rgp, frontEndFrameCount, dx, needSortMaterials
#include <gfx_d3d/r_material.h>    // Material_Sort, Material_FromHandle
#include <gfx_d3d/r_scene.h>       // R_ClearScene
#include <gfx_d3d/rb_backend.h>    // tess, gfxCmdBufContext, GfxCmdBufState, frontEndDataOut
#include <gfx_d3d/rb_state.h>      // gfxCmdBufState, gfxCmdBufSourceState
#include <gfx_d3d/rb_shade.h>      // RB_BeginSurface, RB_EndTessSurface
#include <gfx_d3d/r_shade.h>       // R_SetupPass*, R_UpdateVertexDecl, R_SetIndexData, R_SetVertexData
#include <gfx_d3d/r_state.h>       // R_ChangeStreamSource, R_DrawIndexedPrimitive, R_*Viewport
#include <gfx_d3d/r_utils.h>       // R_GetActiveWorldMatrix, R_MatrixIdentity44, R_Set3D
#include <gfx_d3d/r_debug.h>       // R_WarnOncePerFrame
#include <gfx_d3d/r_dobj_skin.h>   // GfxModelSkinnedSurface, GfxModelSurfaceInfo
#include <gfx_d3d/r_buffers.h>     // gfxBuf (dynamicVertexBuffer)
#include <gfx_d3d/r_xsurface.h>    // XSurfaceGetNumVerts/Tris
#include <xanim/xmodel.h>          // XModel, XSurface, XModelBad, XModelGetBounds/Surfaces
#include <universal/com_math.h>    // AxisToQuat, QuatToAxis, mat3x3

extern void Assert(const char *file, int line, int type, const char *fmt, ...); // 0x49cea0
void FatalError(int code, const char *fmt, ...);                                // 0x49a9e0

// r_ed_vertbuf.cpp — handle → (vb, firstIndex)
void Editor_GetVertexBufferAndIndex(unsigned int handle, IDirect3DVertexBuffer9 **vb, uint16_t *firstIndex);

// ── surface-cache types (IDB editorMesh_s/editorSurf_s) ──────────────────────
enum EDITOR_SURF_TYPE { ED_SURF_MESH = 0, ED_SURF_MODEL = 1 };

struct editorMesh_s {              // 24 bytes
    const Material *material;      // +0
    int       techType;           // +4
    int       sortKey;            // +8  (IDB "unk" — primary sort key)
    int       handle;             // +C  (LOWORD=firstIndex, HIWORD=vb buffer)
    uint16_t  vertCount;          // +10
    uint16_t  indexCount;         // +12
    int       indexTable;         // +14 (IDB "unk3" — edFaceIndices/edBackFaceIndices)
};

// IDB editorSurf_sub (the ED_SURF_MODEL surf record, 20 bytes) — a model surface
// queued for the cached draw.  Distinct from editorMesh_s (the brush-face record).
struct editorSurf_sub {            // 20 bytes
    Material               *material;   // +0
    int                     techType;   // +4
    int                     sortKey;    // +8  (IDB "unk3")
    union {                             // +C  the built skinned surface (verts+indices)
        GfxModelSkinnedSurface *skinnedSurf;
        GfxModelSkinnedSurface *surf;   //     (the binary's name — assert strings)
    };
    GfxScaledPlacement     *placement;  // +10 the model instance's world transform
};

struct editorSurf_s {              // 8 bytes
    EDITOR_SURF_TYPE type;        // +0
    void            *mesh_or_surfSub; // +4  (editorMesh_s* for MESH, editorSurf_sub* for MODEL)
};

#define ED_SCENE_MAX_MESHES  0x40000
#define ED_SCENE_MAX_SURFS   0x44000
#define ED_SCENE_MAX_MODELSURFS 0x4000      // IDB radiant_surfs[0x4000]

static struct {                        // the binary's edSceneGlobals block (assert strings)
    editorMesh_s sceneMeshes[ED_SCENE_MAX_MESHES];
    editorSurf_s sceneSurfs[ED_SCENE_MAX_SURFS];
    int          sceneMeshCount;
    int          sceneSurfCount;
    int          sceneSurfCount_saved;
} edSceneGlobals;
static editorSurf_sub radiant_surfs[ED_SCENE_MAX_MODELSURFS];     // IDB 0x10F5658
static GfxModelSkinnedSurface radiant_modelSkinnedSurfs[ED_SCENE_MAX_MODELSURFS]; // built surfs
static int radiant_surfCount;          // IDB 0x10F5654 — model-surf counter
static int radiant_modelSurfPos;       // stands in for the binary's frontEndDataOut->surfPos
static int edScene_lastFrameCount;     // IDB dword_1365660 (per-frame reset guard)

// Triangle-fan index tables (IDB unk_62D7C8 / unk_62D940). For a fan of N verts,
// triangle t (0-based) = { t+1, t+2, 0 } front, { t+2, t+1, 0 } back. Sized for the
// editor's max face (indexCount 3*(N-2) <= 0xBA → N <= 64 → 62 tris × 3 = 186).
#define ED_FACE_MAX_INDICES 186
static uint16_t edFaceIndices[ED_FACE_MAX_INDICES];
static uint16_t edBackFaceIndices[ED_FACE_MAX_INDICES];
static bool     s_edFaceIndicesInit;

static void Editor_InitFaceIndices()
{
    for (int t = 0; t * 3 < ED_FACE_MAX_INDICES; ++t) {
        edFaceIndices[3 * t + 0]     = (uint16_t)(t + 1);
        edFaceIndices[3 * t + 1]     = (uint16_t)(t + 2);
        edFaceIndices[3 * t + 2]     = 0;
        edBackFaceIndices[3 * t + 0] = (uint16_t)(t + 2);
        edBackFaceIndices[3 * t + 1] = (uint16_t)(t + 1);
        edBackFaceIndices[3 * t + 2] = 0;
    }
    s_edFaceIndicesInit = true;
}

// ── front-end accumulation ────────────────────────────────────────────────────

// 0x4FDA50  Editor_AddMeshCmd — append one cached mesh + its surf entry.
void __cdecl Editor_AddMeshCmd(Material *handle, int techType, int sortKey,
                               int vertCount, int vbIndexAndOffs, int indexCount, int indexTable)
{
    iassert(techType >= TECHNIQUE_DEPTH_PREPASS);         // 0x4fda66 (level 0)
    const Material *material = Material_FromHandle(handle);
    iassert( material );   // r_ed_scene.cpp:219

    // Skip if the material lacks the requested technique. The IDB uses
    // techniques[techType+1] (its MaterialTechniqueSet reserves slot 0); kisak's
    // MaterialTechniqueSet is indexed directly by techType (r_material.cpp:753).
    if (techType < TECHNIQUE_COUNT && !material->techniqueSet->techniques[techType])
        return;

    if (edSceneGlobals.sceneMeshCount == ED_SCENE_MAX_MESHES) {
        R_WarnOncePerFrame((GfxWarningType)34, ED_SCENE_MAX_MESHES);
        return;
    }

    editorMesh_s *mesh = &edSceneGlobals.sceneMeshes[edSceneGlobals.sceneMeshCount++];
    mesh->handle      = vbIndexAndOffs;
    mesh->material    = material;
    mesh->techType    = techType;
    mesh->sortKey     = sortKey;
    mesh->vertCount = (uint16_t)vertCount;
    iassert(mesh->vertCount == vertCount);                // r_ed_scene.cpp:236, after the store
    mesh->indexCount  = (uint16_t)indexCount;
    iassert(mesh->indexCount == indexCount);              // 0x4fdb40 (level 0), after the store
    mesh->indexTable  = indexTable;

        iassert(edSceneGlobals.sceneSurfCount < ARRAY_COUNT( edSceneGlobals.sceneSurfs ));   // r_ed_scene.cpp:241
    editorSurf_s *surf = &edSceneGlobals.sceneSurfs[edSceneGlobals.sceneSurfCount++];
    surf->mesh_or_surfSub = mesh;
    surf->type = ED_SURF_MESH;
}

// 0x4FEEF0  sub_4FEEF0 — emit a front-facing fan (edFaceIndices) for one face.
void __cdecl Editor_AddGeoFace(Material *handle, int techType, int sortKey, int vertCount, int vbIndexAndOffs)
{
    if (!s_edFaceIndicesInit)
        Editor_InitFaceIndices();
    if (vertCount < 3 || (unsigned)(3 * vertCount - 6) > ED_FACE_MAX_INDICES)
        // KEEP_VERBOSE: the binary's condition string is PROSE ("fan fits"), not an
        // expression — no iassert/vassert can stringize it 1:1.
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_scene.cpp", 197, 0, "%s\n\t(vertCount) = %i", "vertCount fan fits edFaceIndices", vertCount);
    Editor_AddMeshCmd(handle, techType, sortKey, vertCount, vbIndexAndOffs, 3 * vertCount - 6, (int)edFaceIndices);
}

// 0x4FEF50  sub_4FEF50 — emit a back-facing fan (edBackFaceIndices) for one face.
void __cdecl Editor_AddGeoBackFace(Material *handle, int techType, int sortKey, int vertCount, int vbIndexAndOffs)
{
    if (!s_edFaceIndicesInit)
        Editor_InitFaceIndices();
    if (vertCount < 3 || (unsigned)(3 * vertCount - 6) > ED_FACE_MAX_INDICES)
        // KEEP_VERBOSE: the binary's condition string is PROSE ("fan fits"), not an
        // expression — no iassert/vassert can stringize it 1:1.
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_scene.cpp", 204, 0, "%s\n\t(vertCount) = %i", "vertCount fan fits edBackFaceIndices", vertCount);
    Editor_AddMeshCmd(handle, techType, sortKey, vertCount, vbIndexAndOffs, 3 * vertCount - 6, (int)edBackFaceIndices);
}

// 0x4FDBB0  sub_4FDBB0 — the surf sortKey DrawGeo (0x47acf0) passes to Editor_AddGeoFace:
// 100 × the material's primary sort bucket. IDA extracts packed bits 29..40, but Kisak's
// shared GfxDrawSurf layout stores the same material sort bucket in fields.primarySortKey.
// Per-layer visuals of one face get consecutive keys (base+layerIndex) so layer order survives.
int __cdecl Editor_MaterialSortKey(Material *handle)
{
    return 100 * (int)Material_FromHandle(handle)->info.drawSurf.fields.primarySortKey;
}

// 0x4FD9C0  sub_4FD9C0 — surf sort comparator: sortKey, then techType, then firstIndex.
// editorMesh_s and editorSurf_sub share the first three fields (material@0, techType@4,
// sortKey@8) at identical offsets, so the primary sort works on both surf types; the
// firstIndex (handle) tiebreak only applies to MESH surfs (model surfs key on 0 there).
static int __cdecl Editor_SurfCompare(const void *pa, const void *pb)
{
    const editorSurf_s *a = (const editorSurf_s *)pa;
    const editorSurf_s *b = (const editorSurf_s *)pb;
    const editorMesh_s *ma = (const editorMesh_s *)a->mesh_or_surfSub;
    const editorMesh_s *mb = (const editorMesh_s *)b->mesh_or_surfSub;
    int result = ma->sortKey - mb->sortKey;
    if (!result) {
        result = ma->techType - mb->techType;
        if (!result) {
            int ka = (a->type == ED_SURF_MESH) ? ma->handle : 0;
            int kb = (b->type == ED_SURF_MESH) ? mb->handle : 0;
            result = ka - kb;
        }
    }
    return result;
}

// 0x4FD300  Editor_AddCmd_DrawSkinnedCached — emit the backend command.
struct GfxCmdEditorSkinnedCached { GfxCmdHeader header; int index; int amount; };

void *__cdecl Editor_AddCmd_DrawSkinnedCached(int index, int amount)
{
    GfxCmdEditorSkinnedCached *cmd =
        (GfxCmdEditorSkinnedCached *)R_GetCommandBuffer(RC_DRAW_EDITOR_SKINNEDCACHED, sizeof(GfxCmdEditorSkinnedCached));
    if (cmd) {
        cmd->index  = index;
        cmd->amount = amount;
    }
    return cmd;
}

// 0x4FDA10  R_AddEditorSurfsCmd — sort the surfs added since the last flush and emit
// one RC_DRAW_EDITOR_SKINNEDCACHED for them.
void *__cdecl R_AddEditorSurfsCmd()
{
    int first = edSceneGlobals.sceneSurfCount_saved;
    int count = edSceneGlobals.sceneSurfCount - first;
    if (count) {
        qsort(&edSceneGlobals.sceneSurfs[first], count, sizeof(editorSurf_s), Editor_SurfCompare);
        return Editor_AddCmd_DrawSkinnedCached(first, count);
    }
    return (void *)first;
}

// ── the xmodel mesh path (ED_SURF_MODEL) ──────────────────────────────────────
// DrawBrush → DrawModels → SkinModelInst → AddModelSurfBuf + Editor_AddSurfCmd; the
// R_AddEditorSurfsCmd flush then draws them in RB_DrawEditorSkinnedCached_Sub.  This
// branch does NOT use the per-material VB pool: it uploads the xmodel's verts to the
// engine dynamic vertex buffer per frame (R_SetVertexData), VERTDECL_PACKED.
// edMapGlobals.modelInst[] is the registry of placed models (AddModelToModelInstBuff /
// ModelInstUpdate / RemoveModelInstFromBuf); model_inst's first 32 bytes alias a
// GfxScaledPlacement {quat,origin,scale} so an inst pointer casts straight to a
// placement for R_ChangeObjectPlacement.

// IDB model_inst (44 bytes) — origin/quat/scale alias GfxScaledPlacement at +0.
struct model_inst {                // 44 bytes
    float    angles[4];            // +0   quat (GfxPlacement.quat)
    float    origin[3];            // +16  (GfxPlacement.origin)
    float    modelscale;           // +28  (GfxScaledPlacement.scale)
    int      colorOverride;        // +32  pad_0x0020 — per-vert color (or -1)
    XModel  *model;                // +36
    int      inuse;                // +40  IDB "random_one"
};
static_assert(sizeof(model_inst) == 44, "model_inst must alias GfxScaledPlacement+{pad,model,inuse}");

#define ED_MAP_MAX_MODELINST 65536
struct EdMapGlobals {              // IDB edMapGlobals @ 0x835648
    int        modelInstMax;       // highest-used slot + 1 (NOT a fixed capacity)
    model_inst modelInst[ED_MAP_MAX_MODELINST + 16];
};
static EdMapGlobals edMapGlobals;

// 0x4FDBE0  AddModelToModelInstBuff — find a free slot, store {model, origin, quat, scale},
// return its 1-based handle.
int __cdecl AddModelToModelInstBuff(XModel *model, float *axis, float scale)
{
    iassert(model);                                       // 0x4fdbe0 (level 0)
    if (XModelBad(model))
        return 0;

    int idx = 0;
    while (idx < edMapGlobals.modelInstMax && edMapGlobals.modelInst[idx].inuse)
        ++idx;

    if (idx == edMapGlobals.modelInstMax) {
        if (edMapGlobals.modelInstMax == 65536)
            FatalError(0, "too many models in map");
        if (edMapGlobals.modelInstMax == 65520)
            MessageBoxA(GetActiveWindow(),
                "WARNING!\nThis map is dangerously close to the editor's internal model limit.\n"
                "Adding any more models may cause the editor to exit without saving.\n", "Warning", 0x30u);
        ++edMapGlobals.modelInstMax;
    }

    model_inst *v8 = &edMapGlobals.modelInst[idx];
    memset(v8, 0, sizeof(model_inst));
    v8->inuse      = 1;            // LOBYTE(random_one) = 1 (faithful: only the low byte)
    v8->model      = model;
    v8->origin[0]  = axis[0];
    v8->origin[1]  = axis[1];
    v8->origin[2]  = axis[2];
    AxisToQuat((vec3_t *)(axis + 3), v8->angles);
    v8->modelscale = scale;
    return idx + 1;
}

// 0x4FDD80  ModelInstUpdate — re-pose an existing instance.
void __cdecl ModelInstUpdate(int instanceHandle, float (*axis)[3], float scale)
{
    iassert(instanceHandle > 0 && instanceHandle <= edMapGlobals.modelInstMax);
    model_inst *v3 = &edMapGlobals.modelInst[instanceHandle - 1];
    iassert(edMapGlobals.modelInst[instanceHandle - 1].inuse);   // r_ed_scene.cpp:325
    v3->origin[0] = axis[0][0];
    v3->origin[1] = axis[0][1];
    v3->origin[2] = axis[0][2];
    AxisToQuat((vec3_t *)&axis[1][0], v3->angles);
    v3->modelscale = scale;
}

// 0x4FDCE0  RemoveModelInstFromBuf — free a slot, shrink modelInstMax past trailing frees.
void __cdecl RemoveModelInstFromBuf(int instanceHandle)
{
    iassert(instanceHandle > 0 && instanceHandle <= edMapGlobals.modelInstMax);
    iassert(edMapGlobals.modelInst[instanceHandle - 1].inuse);   // r_ed_scene.cpp:309
    edMapGlobals.modelInst[instanceHandle - 1].inuse = 0;
    edMapGlobals.modelInst[instanceHandle - 1].model = 0;
    int n = edMapGlobals.modelInstMax;
    while (n > 0 && !edMapGlobals.modelInst[n - 1].inuse)
        --n;
    edMapGlobals.modelInstMax = n;
}

// 0x4FDE10  Entity_GetModelInstBounds (sub_4FDE10) — local-space mins/maxs of an instance's
// model (xmodel bounds rotated by the instance quat, scaled by modelscale).
extern void OrientationPosToWorldPos(float *out, const float *pos, const orientation_t *orient);
void __cdecl Entity_GetModelInstBounds(int instanceHandle, float *out_mins, float *out_maxs)
{
    iassert(instanceHandle > 0 && instanceHandle <= edMapGlobals.modelInstMax);
    model_inst *mi = &edMapGlobals.modelInst[instanceHandle - 1];
    iassert(edMapGlobals.modelInst[instanceHandle - 1].inuse);   // r_ed_scene.cpp:346

    float mins[3], maxs[3];
    XModelGetBounds(mi->model, mins, maxs);          // sub_4C6ED0
    mat3x3 axis;
    QuatToAxis(mi->angles, axis);                    // sub_4A6FA0
    // sub_4A8780: rotate the local AABB by axis into an axis-aligned [mins,maxs] pair.
    float rmin[3], rmax[3];
    for (int i = 0; i < 3; ++i) { rmin[i] = 1e30f; rmax[i] = -1e30f; }
    for (int c = 0; c < 8; ++c) {
        float p[3] = { (c & 1) ? maxs[0] : mins[0], (c & 2) ? maxs[1] : mins[1], (c & 4) ? maxs[2] : mins[2] };
        float r[3];
        for (int i = 0; i < 3; ++i)
            r[i] = axis[0][i] * p[0] + axis[1][i] * p[1] + axis[2][i] * p[2];
        for (int i = 0; i < 3; ++i) { if (r[i] < rmin[i]) rmin[i] = r[i]; if (r[i] > rmax[i]) rmax[i] = r[i]; }
    }
    for (int i = 0; i < 3; ++i) {
        // sub_4A8780 seeds its running min/max WITH origin, so the binary's `v7[i] - origin[i]`
        // (0x4fdeb3) recovers the pure rotated bound before scaling.  rmin/rmax above are
        // already origin-free; subtracting origin here would double-subtract.
        out_mins[i] = mi->modelscale * rmin[i] + mi->origin[i];
        out_maxs[i] = mi->modelscale * rmax[i] + mi->origin[i];
    }
}

// ── model-surf builder (the ED_SURF_MODEL queue) ──────────────────────────────

// 0x4FE0D0  AddSurfTempSkinBuf — copy the surface's base verts (verts0) into the per-frame
// frontEndDataOut->tempSkinBuf and point skinnedVert at that WRITABLE COPY, so SkinModelInst's
// per-vertex colour stamp (0x4fe455) writes the copy and never the shared asset verts0.
// skinnedCachedOffset = -1 (rigid/uncached).  tempSkinPos is reset each editor frame by
// R_ToggleSmpFrame; on overflow the binary drops the model (return 0) — kept verbatim.
void __cdecl Z_VirtualCommit(void *ptr, int size);    // com_memory.cpp (0x4AC210 == sub_4AC210)
static bool AddSurfTempSkinBuf(XSurface *xsurf, GfxModelSkinnedSurface *out, XModel *model)
{
    iassert(model);
    iassert(xsurf);
    out->skinnedCachedOffset = -1;            // RIGID/UNCACHED — draw from the temp copy
    out->xsurf               = xsurf;
    if (!xsurf->verts0)
        return false;

    const int numVerts  = XSurfaceGetNumVerts(xsurf);
    const unsigned vsize = 32u * (unsigned)numVerts;   // v3 = 32 * XSurfaceGetNumVerts (0x4fe129)
    GfxBackEndData *fe = frontEndDataOut;
    if (!fe || !fe->tempSkinBuf)
        return false;                                        // 0x4fe12c assert -> no buffer, drop
    if (vsize + (unsigned)fe->tempSkinPos > 0x5000000u) {
        return false;                                        // 0x4fe169: drop this surf/model
    }
    uint8_t *copy = fe->tempSkinBuf + fe->tempSkinPos;       // 0x4fe18c: tempSkinBuf + tempSkinPos
    fe->tempSkinPos += (long)vsize;                          // 0x4fe1a2
    Z_VirtualCommit(copy, vsize);                            // 0x4fe1ac (sub_4AC210)
    memcpy(copy, xsurf->verts0, vsize);                      // 0x4fe1ba: copy the base verts
    out->skinnedVert = (GfxPackedVertex *)copy;              // 0x4fe1c2-ish: skinnedVert = the copy
    return true;
}

// IDB xmodel_utils helpers kisak lacks (CoD4Radiant-only) — trivial LOD-0 accessors.
//  GetXmodelNumSurfs (0x4CBD80)      = lodInfo[lod].numsurfs
//  GetXmodelMaterialHandle (0x4CBDE0)= &materialHandles[lodInfo[lod].surfIndex]
static inline int GetXmodelNumSurfs(const XModel *m, int lod) { return (uint16_t)m->lodInfo[lod].numsurfs; }
static inline Material **GetXmodelMaterialHandle(XModel *m, int lod) { return &m->materialHandles[(uint16_t)m->lodInfo[lod].surfIndex]; }

// 0x4FE1D0  AddModelSurfBuf — build one GfxModelSkinnedSurface per surface of the xmodel's
// LOD 0 and return the base pointer (nullptr on overflow / a failed surf).
// KISAK: differs from 0x4FE1D0 in WHERE the surfaces live.  The binary builds them on the
// stack, then bump-allocates out of frontEndDataOut->surfsBuffer (cursor surfPos, cap
// 0x20000 bytes, reset per frame with tempSkinPos).  kisak's GfxBackEndData has no
// surfsBuffer/surfPos, so the editor keeps its own array + cursor with the same semantics:
// the cursor advances by the FULL surface count of every model (NOT by the number of surfs
// actually queued — Editor_AddSurfCmd can drop some), so a queued surf's record can never be
// overwritten by the next model.
static GfxModelSkinnedSurface *AddModelSurfBuf(XModel *xmodel)
{
    if (XModelBad(xmodel))
        return 0;
    XModelNumBones(xmodel);                   // (IDB call; bone count not needed for rigid)

    XSurface *surfBase = nullptr;
    int surfaceCount = XModelGetSurfaces(xmodel, &surfBase, 0);   // == LODForXmodel(model,&s,0)
    iassert(surfaceCount);

    const int firstSurfSlot = radiant_modelSurfPos;
    if (firstSurfSlot + surfaceCount > ED_SCENE_MAX_MODELSURFS)   // binary: surfsBuffer cap
        return 0;
    GfxModelSkinnedSurface *dst = &radiant_modelSkinnedSurfs[firstSurfSlot];
    for (int i = 0; i < surfaceCount; ++i) {
        if (!AddSurfTempSkinBuf(&surfBase[i], &dst[i], xmodel))
            return 0;                         // 0x4fe257: drop the model, cursor unmoved
    }
    radiant_modelSurfPos = firstSurfSlot + surfaceCount;          // 0x4fe290
    return dst;
}

// 0x4FDF40  sub_4FDF40 — multiply/skip-multiply draw-flag filter.  With no DRAWFLAG_*_MULTIPLY
// bit set it returns true immediately and never touches the material.  Otherwise the binary
// keys "effect" on material byte 30 (editorToolFlags) & 0x70 == 0x70 and tests
// drawFlags & (4*(!isEffect)+4): mask 8 for opaque, mask 4 for effect.
static bool Editor_SurfFilter(int drawFlags, const Material *material)
{
    if ((drawFlags & 0xC) == 0)
        return true;
    iassert(!(drawFlags & DRAWFLAG_ONLY_MULTIPLY) || !(drawFlags & DRAWFLAG_SKIP_MULTIPLY));   // r_ed_scene.cpp:369
    const unsigned flags  = material ? material->editorToolFlags : 0;   // no material -> opaque
    const bool     opaque = (flags & 0x70) != 0x70;
    return (drawFlags & (opaque ? 8 : 4)) != 0;          // binary: (drawFlags & (4*(!effect)+4)) != 0
}

// 0x4FDFA0  Editor_AddSurfCmd — queue one ED_SURF_MODEL surf (a built skinned surface +
// the instance placement) into the scene surf list.
void __cdecl Editor_AddSurfCmd(int drawFlags, Material *material, model_inst *inst,
                               GfxModelSkinnedSurface *surf, int techType)
{
    iassert(surf);
    iassert(material);

    // IDA gate (0x4fdff3): queue the surf ONLY when the material carries the requested
    // technique (or techType >= TECHNIQUE_COUNT); a surf whose material lacks the tech is DROPPED and the
    // requested techType is stored unchanged.  techniqueSet is dereferenced unconditionally
    // (kisak techniques[techType], no +1 — the IDB pseudocode's +1 is its narrower
    // MaterialTechniqueSet).  CONSEQUENCE: a 2D-view model whose material lacks tech 29 is
    // dropped, exactly as the binary behaves.
    if (techType >= TECHNIQUE_COUNT || material->techniqueSet->techniques[techType]) {
        if (radiant_surfCount == ED_SCENE_MAX_MODELSURFS) {
            R_WarnOncePerFrame((GfxWarningType)35, ED_SCENE_MAX_MODELSURFS);
        } else if (Editor_SurfFilter(drawFlags, material)) {
            editorSurf_sub *v5 = &radiant_surfs[radiant_surfCount++];
            v5->material  = material;
            v5->techType  = techType;
            // IDB sortKey = 0x64 * ((Material_FromHandle(material)->info.drawSurf >> 29) & 0xFFF);
            // Kisak's packed layout differs, so use the equivalent material primarySortKey field.
            v5->sortKey   = 0x64 * Material_FromHandle(material)->info.drawSurf.fields.primarySortKey;
            v5->skinnedSurf = surf;
            v5->placement = (GfxScaledPlacement *)inst;   // model_inst aliases GfxScaledPlacement
                iassert(edSceneGlobals.sceneSurfCount < ARRAY_COUNT( edSceneGlobals.sceneSurfs ));   // r_ed_scene.cpp:402
            editorSurf_s *v6 = &edSceneGlobals.sceneSurfs[edSceneGlobals.sceneSurfCount++];
            v6->mesh_or_surfSub = v5;
            v6->type = ED_SURF_MODEL;
        }
    }
}

// 0x4FE2E0  SkinModelInst — build + queue every surface of a placed model instance.
//   instanceHandle = 1-based handle into edMapGlobals.modelInst[]
//   checkhandle    = optional material override (Material handle), else use model's own
//   techType (a3) / colorPtr (a4) / drawFlags
void __cdecl SkinModelInst(int instanceHandle, Material *checkhandle, int techType,
                           const int *colorPtr, int drawFlags)
{
    iassert(instanceHandle > 0 && instanceHandle <= edMapGlobals.modelInstMax);
    model_inst *mi = &edMapGlobals.modelInst[instanceHandle - 1];
    iassert(edMapGlobals.modelInst[instanceHandle - 1].inuse);   // r_ed_scene.cpp:485

    GfxModelSkinnedSurface *skinned = AddModelSurfBuf(mi->model);
    if (!skinned)
        return;

    unsigned numsurfs    = (unsigned)GetXmodelNumSurfs(mi->model, 0);
    Material **modelMaterial = GetXmodelMaterialHandle(mi->model, 0);
    iassert(modelMaterial);

    for (unsigned i = 0; i < numsurfs; ++i) {
        GfxModelSkinnedSurface *skinnedSurf = &skinned[i];   // the binary's local (assert strings)
        Material *material = modelMaterial[i];
        iassert(material);
        if (colorPtr) {
            // IDB 0x4fe3f0-0x4fe45f: record the override in mi->colorOverride AND write
            // *colorPtr over each skinned vert's colour (GfxPackedVertex.color @+0x10,
            // stride 0x20) — the white per-vertex colour of the tech-29 selected-model
            // wireframe.  Only ever stamp the tempSkinBuf COPY, never the shared verts0.
            mi->colorOverride = *colorPtr;
            iassert(skinnedSurf->skinnedCachedOffset != RIGID_SKINNED_CACHE_OFFSET);   // r_ed_scene.cpp:505
            iassert(skinnedSurf->skinnedCachedOffset != HIDDEN_SURFACE_OFFSET);        // r_ed_scene.cpp:506
            const int nv  = XSurfaceGetNumVerts(skinned[i].xsurf);         // 0x4fe43d
            uint8_t  *base = (uint8_t *)skinned[i].skinnedVert;
            for (int vi = 0; vi < nv; ++vi)                                // 0x4fe449-0x4fe45f
                *(uint32_t *)(base + 32 * vi + 0x10) = (uint32_t)*colorPtr;
        } else {
            mi->colorOverride = -1;
        }
        Material *useMat = checkhandle ? (Material *)Material_FromHandle(checkhandle) : material;
        Editor_AddSurfCmd(drawFlags, useMat, mi, &skinned[i], techType);
        iassert(skinnedSurf->skinnedCachedOffset != RIGID_SKINNED_CACHE_OFFSET);   // r_ed_scene.cpp:522
        iassert(skinnedSurf->skinnedCachedOffset != HIDDEN_SURFACE_OFFSET);        // r_ed_scene.cpp:523
    }
}

// ── backend draw (mesh branch only) ───────────────────────────────────────────

// 0x4FE500  Editor_DrawIndexedPrimitive — the editor's OWN indexed draw: a raw
// device->DrawIndexedPrimitive with no prim-stats tracking.  It must NOT go through the
// game's R_DrawIndexedPrimitive: that one's RB_TrackDrawPrimCall asserts on g_primStats,
// which no editor frame ever sets (int3 0xC0000409).  MinVertexIndex 0 /
// NumVertices=vertexCount / StartIndex=baseIndex / PrimCount=triCount.
static void Editor_DrawIndexedPrimitive(GfxCmdBufPrimState *state, const GfxDrawPrimArgs *args)
{
    IDirect3DDevice9 *device = state->device;
    iassert(device);

    HRESULT hr = device->DrawIndexedPrimitive(
        D3DPT_TRIANGLELIST, 0, 0, args->vertexCount, args->baseIndex, args->triCount);
    if (hr < 0) {
        ++g_disableRendering;
        FatalError(0,
            "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_scene.cpp (%i) "
            "device->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, args->vertexCount, "
            "args->baseIndex, args->triCount ) failed: %s\n", 545, R_ErrorDescription(hr));
    }
}

// 0x4FE5A0  R_DrawTessTechnique_Brushes — run the bound technique's passes over the
// accumulated tess indices. Adapted to kisak's GfxCmdBufContext pass API. Draws through
// the editor's own untracked Editor_DrawIndexedPrimitive (above) — NOT the game's
// prim-stats-tracking R_DrawIndexedPrimitive (see the Stage-1c note there).
static void R_DrawTessTechnique_Brushes(const GfxDrawPrimArgs *args)
{
    iassert(dx.d3d9 && dx.device);
    if (!gfxCmdBufState.material)   // KEEP_VERBOSE: binary string "context.state->material" != port member gfxCmdBufState.material
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_scene.cpp", 555, 0, "%s", "context.state->material");
    const MaterialTechnique *technique = gfxCmdBufState.technique;
    iassert(technique);

    for (uint32_t passIndex = 0; passIndex < technique->passCount; ++passIndex) {
        R_SetupPass(gfxCmdBufContext, passIndex);
        R_UpdateVertexDecl(&gfxCmdBufState);
        R_SetupPassCriticalPixelShaderArgs(gfxCmdBufContext);
        R_SetupPassPerObjectArgs(gfxCmdBufContext);
        R_SetupPassPerPrimArgs(gfxCmdBufContext);

        Editor_DrawIndexedPrimitive(&gfxCmdBufState.prim, args);
    }
}

// 0x4FE690  RB_DrawTessSurface (editor-local — distinct from kisak's parameterless
// RB_DrawTessSurface). Draws the accumulated indices over the bound world stream for
// the vertex range [minVert, maxVert].
static void RB_DrawEditorTessSurface(uint16_t minVert, uint16_t maxVert)
{
    GfxDrawPrimArgs args;
    tess.finishedFilling = 1;
    if (gfxCmdBufSourceState.viewportIsDirty) {
        GfxViewport vp;
        R_GetViewport(&gfxCmdBufSourceState, &vp);
        R_SetViewport(&gfxCmdBufState, &vp);
        R_UpdateViewport(&gfxCmdBufSourceState, &vp);
    }
    // kisak's R_DrawIndexedPrimitive issues DrawIndexedPrimitive(TRIANGLELIST, 0, 0,
    // NumVertices=args.vertexCount, StartIndex=args.baseIndex, PrimCount=args.triCount)
    // with MinVertexIndex hardwired to 0 — so vertexCount must span all referenced
    // verts [0, maxVert] and triCount is the real triangle count. (The IDB's editor
    // R_DrawIndexedPrimitive used minVert/range + a separate indexCount field; kisak's
    // GfxDrawPrimArgs has no indexCount, so the mapping differs.)
    args.baseIndex   = R_SetIndexData(&gfxCmdBufState.prim, (uint8_t *)tess.indices, tess.indexCount / 3);
    args.vertexCount = maxVert + 1;          // NumVertices (verts span [0, maxVert])
    args.triCount    = tess.indexCount / 3;  // PrimitiveCount (triangles)
    R_DrawTessTechnique_Brushes(&args);
    tess.indexCount = 0;
    tess.finishedFilling = 0;
}

// Reset the active world matrix to eye-relative identity (world verts are stored in
// world space; the camera-relative offset lives in eyeOffset). kisak's
// R_GetActiveWorldMatrix returns the source state; the active matrix is matrix[0].
static void Editor_SetEyeRelativeWorldMatrix()
{
    GfxCmdBufSourceState *src = R_GetActiveWorldMatrix(&gfxCmdBufSourceState);
    float (*m)[4] = src->matrices.matrix[0].m;
    R_MatrixIdentity44(m);
    m[3][0] -= gfxCmdBufSourceState.eyeOffset[0];
    m[3][1] -= gfxCmdBufSourceState.eyeOffset[1];
    m[3][2] -= gfxCmdBufSourceState.eyeOffset[2];
}

// KISAK (not in the binary): re-apply every `def cN, x,y,z,w` immediate found in a vertex
// shader's bytecode.  The editor _dtex shaders decompress their packed UBYTE4 texcoords from
// def constants (c8..c12) that live in the bytecode itself, not in any engine constant;
// re-issuing them per draw keeps the decode live when one shader is reused across surfaces.
static void Editor_ForceVsDefConstants(IDirect3DDevice9 *dev, const MaterialVertexShader *vs)
{
    if (!vs || !vs->prog.loadDef.program)
        return;
    const uint32_t *tok = (const uint32_t *)vs->prog.loadDef.program;
    unsigned n = vs->prog.loadDef.programSize;   // dwords
    unsigned i = 1;                              // skip version token
    while (i < n) {
        uint32_t t = tok[i];
        if (t == 0x0000FFFF)                     // end token
            break;
        if ((t & 0xFFFF) == 0xFFFE) {            // comment (CTAB etc): length in bits 16..30
            i += ((t >> 16) & 0x7FFF) + 1;
            continue;
        }
        uint32_t op = t & 0xFFFF;
        if (op == 0x51) {                        // D3DSIO_DEF: dst tok + 4 raw float dwords
            uint32_t dst = tok[i + 1] & 0x7FF;
            dev->SetVertexShaderConstantF(dst, (const float *)&tok[i + 2], 1);
            i += 6;
            continue;
        }
        if (op == 0x30) { i += 6; continue; }    // D3DSIO_DEFI: dst + 4 ints
        if (op == 0x2F) { i += 3; continue; }    // D3DSIO_DEFB: dst + 1 bool
        ++i;                                     // other instruction: skip its param tokens
        while (i < n && (tok[i] & 0x80000000))
            ++i;
    }
}

// 0x53AA30  R_DrawXModelSkinnedUncached_2 — the EDITOR's xmodel skinned draw. Uploads the
// surface's verts to the engine dynamic vertex buffer (VERTDECL_PACKED, stride 32) and draws
// indexed.  The binary draws through R_DrawIndexedPrimitive_R (0x538990, untracked); kisak
// lacks that variant, and Editor_DrawIndexedPrimitive emits the identical D3D9 call (the only
// delta _R adds is r_drawPrimFloor/Cap/r_skipDrawTris, inert at their defaults).  0x53AA30 has
// NO R_CheckVertexDataOverflow — that lives only in the GAME variant rb_shade.cpp:267.
static void Editor_DrawXModelSkinnedUncached(XSurface *xsurf, GfxPackedVertex *skinnedVert)
{
    if (!xsurf)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\rb_shade.cpp", 383, 0, "%s", "xsurf");
    if (!skinnedVert)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\rb_shade.cpp", 384, 0, "%s", "skinnedVert");
    if (tess.indexCount)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\rb_shade.cpp", 385, 0, "%s", "tess.indexCount == 0");
    if (tess.vertexCount)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\rb_shade.cpp", 386, 0, "%s", "tess.vertexCount == 0");

    // IDA copies the IB from tess.indices after R_CheckTris(xsurf, tess.indices, 0) — a plain
    // memcpy of xsurf->triIndices (a3=0, no base offset) plus 3 r_xsurface.cpp alignment asserts.
    // kisak has no R_CheckTris; reading xsurf->triIndices directly into R_SetIndexData yields a
    // byte-identical index buffer (no-copy SUBSET, as with AddSurfTempSkinBuf), dropping only
    // those three cross-file debug asserts.

    if (gfxCmdBufSourceState.viewportIsDirty) {
        GfxViewport vp;
        R_GetViewport(&gfxCmdBufSourceState, &vp);
        R_SetViewport(&gfxCmdBufState, &vp);
        R_UpdateViewport(&gfxCmdBufSourceState, &vp);
    }

    GfxDrawPrimArgs args;
    args.vertexCount = XSurfaceGetNumVerts(xsurf);
    args.triCount    = XSurfaceGetNumTris(xsurf);

    // KEEP_VERBOSE (405/409/412): rb_shade.cpp editor-variant strings (foreign gfx TU).
    if (gfxCmdBufState.prim.vertDeclType != VERTDECL_PACKED)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\rb_shade.cpp", 405, 1,
               "%s\n\t(gfxCmdBufState.prim.vertDeclType) = %i", "(gfxCmdBufState.prim.vertDeclType == VERTDECL_PACKED)", gfxCmdBufState.prim.vertDeclType);

    args.baseIndex   = R_SetIndexData(&gfxCmdBufState.prim, (uint8_t *)xsurf->triIndices, args.triCount);

    if (!gfxCmdBufState.technique)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\rb_shade.cpp", 409, 0, "%s", "gfxCmdBufState.technique");

    int vertexOffset = R_SetVertexData(&gfxCmdBufState, skinnedVert, args.vertexCount, 32);
    IDirect3DVertexBuffer9 *vb = gfxBuf.dynamicVertexBuffer->buffer;
    if (!vb)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\rb_shade.cpp", 412, 0, "%s", "vb");
    if (gfxCmdBufState.prim.streams[0].vb != vb || gfxCmdBufState.prim.streams[0].offset != (uint32_t)vertexOffset ||
        gfxCmdBufState.prim.streams[0].stride != 32)
        R_ChangeStreamSource(&gfxCmdBufState.prim, 0, vb, vertexOffset, 32);
    if (gfxCmdBufState.prim.streams[1].vb || gfxCmdBufState.prim.streams[1].offset || gfxCmdBufState.prim.streams[1].stride)
        R_ChangeStreamSource(&gfxCmdBufState.prim, 1, 0, 0, 0);

    for (uint32_t pass = 0; pass < gfxCmdBufState.technique->passCount; ++pass) {
        R_SetupPass(gfxCmdBufContext, pass);
        R_UpdateVertexDecl(&gfxCmdBufState);
        R_SetupPassCriticalPixelShaderArgs(gfxCmdBufContext);
        R_SetupPassPerObjectArgs(gfxCmdBufContext);
        R_SetupPassPerPrimArgs(gfxCmdBufContext);
        // Keep the shader's own `def` decode constants live across reused shaders.
        {
            IDirect3DDevice9 *dev = gfxCmdBufState.prim.device;
            const MaterialPass *p = &gfxCmdBufState.technique->passArray[pass];
            if (dev)
                Editor_ForceVsDefConstants(dev, p->vertexShader);
        }
        Editor_DrawIndexedPrimitive(&gfxCmdBufState.prim, &args);   // editor-untracked draw
    }
}

// 0x4FE750  RB_DrawEditorSkinnedCached_Sub — for each accumulated surf, batch consecutive
// ED_SURF_MESH (brush face) surfs sharing (vb, material, techType) into tess and flush;
// ED_SURF_MODEL (xmodel) surfs draw individually via the editor uncached xmodel path with
// the instance's object placement (VERTDECL_PACKED, no VB pool).
static void RB_DrawEditorSkinnedCached_Sub(int index, int amount)
{
    if (tess.indexCount)
        RB_EndTessSurface();
    R_Set3D(&gfxCmdBufSourceState);

    uint16_t minVert = 0xFFFF;
    int      maxVert = 0;
    IDirect3DVertexBuffer9 *boundVb = 0;
    bool     haveBatch = false;

    for (int i = 0; i < amount; ++i) {
        editorSurf_s *edSurf = &edSceneGlobals.sceneSurfs[index + i];

        editorMesh_s           *mesh = nullptr;   // set for ED_SURF_MESH
        editorSurf_sub         *modelSurf = nullptr; // set for ED_SURF_MODEL
        GfxModelSkinnedSurface *skinnedSurf = nullptr;
        IDirect3DVertexBuffer9 *vb = 0;
        uint16_t firstIndex = 0;
        const Material *material;
        int techType, indexCount;

        if (edSurf->type == ED_SURF_MESH) {
            mesh = (editorMesh_s *)edSurf->mesh_or_surfSub;
            Editor_GetVertexBufferAndIndex(mesh->handle, &vb, &firstIndex);
            material   = mesh->material;
            techType   = mesh->techType;
            indexCount = (uint16_t)mesh->indexCount;
        } else {
            vassert((edSurf->type == ED_SURF_MODEL), "(edSurf->type) = %i", edSurf->type);   // r_ed_scene.cpp:652
            modelSurf   = (editorSurf_sub *)edSurf->mesh_or_surfSub;
            vb          = 0;             // model surfs do NOT use the VB pool
            firstIndex  = 0;
            skinnedSurf = modelSurf->skinnedSurf;
            material    = modelSurf->material;
            techType    = modelSurf->techType;
            iassert(skinnedSurf->skinnedCachedOffset != RIGID_SKINNED_CACHE_OFFSET);   // r_ed_scene.cpp:664
            iassert(skinnedSurf->skinnedCachedOffset != HIDDEN_SURFACE_OFFSET);        // r_ed_scene.cpp:665
            indexCount  = 3 * XSurfaceGetNumTris(skinnedSurf->xsurf);
        }

        // Flush the current (mesh) batch when the source/material/technique changes or tess
        // would overflow.  A model edSurf (vb==0, but a fresh skinnedSurf each time) always
        // breaks the batch (its indexCount also pushes the overflow check).
        if (vb != boundVb || material != gfxCmdBufState.material ||
            techType != gfxCmdBufState.techType || indexCount + tess.indexCount > 0x7FC0)
        {
            // Binary's `if (vb_x)` gate (0x4fe8a7): vb_x is the PREVIOUS edSurf's VB — 0 after a
            // MODEL (R_ChangeObjectPlacement left matrix[0] holding that model's placement),
            // non-zero after a MESH.  Previous MESH -> flush its tess batch (matrix[0] is still
            // the eye-relative world matrix); previous MODEL or first edSurf -> reset matrix[0]
            // to the eye-relative world matrix before starting this mesh batch.
            if (boundVb) {
                RB_DrawEditorTessSurface(minVert, (uint16_t)maxVert);
                minVert = 0xFFFF;
                maxVert = 0;
            } else {
                Editor_SetEyeRelativeWorldMatrix();   // model-dirtied or first — restore world xform
            }
            RB_BeginSurface(material, (MaterialTechniqueType)techType);
            if (vb) {
                gfxCmdBufState.prim.vertDeclType = VERTDECL_WORLD;
                if (gfxCmdBufState.prim.streams[0].vb != vb || gfxCmdBufState.prim.streams[0].offset ||
                    gfxCmdBufState.prim.streams[0].stride != sizeof(GfxWorldVertex))
                    R_ChangeStreamSource(&gfxCmdBufState.prim, 0, vb, 0, sizeof(GfxWorldVertex));
                if (gfxCmdBufState.prim.streams[1].vb || gfxCmdBufState.prim.streams[1].offset ||
                    gfxCmdBufState.prim.streams[1].stride)
                    R_ChangeStreamSource(&gfxCmdBufState.prim, 1, 0, 0, 0);
            } else {
                gfxCmdBufState.prim.vertDeclType = VERTDECL_PACKED;   // model path
            }
            boundVb = vb;
            haveBatch = true;
        }

        if (mesh) {
            iassert( !modelSurf );   // r_ed_scene.cpp:699
            // brush face: copy its indices into tess with the firstIndex base offset.
            if ((uint16_t)firstIndex < minVert)
                minVert = firstIndex;
            if (maxVert < mesh->vertCount + firstIndex - 1)
                maxVert = mesh->vertCount + firstIndex - 1;
            const uint16_t *itab = (const uint16_t *)mesh->indexTable;
            for (int k = 0; k < (int)(uint16_t)mesh->indexCount; ++k)
                tess.indices[tess.indexCount + k] = (uint16_t)(firstIndex + itab[k]);
            tess.indexCount += (uint16_t)mesh->indexCount;
        } else {
            // xmodel edSurf: object placement + uncached skinned draw (no tess batching).
            iassert( modelSurf );   // r_ed_scene.cpp:711
            iassert( modelSurf->surf );   // r_ed_scene.cpp:712
            iassert( tess.indexCount == 0 );   // r_ed_scene.cpp:713
            iassert( tess.vertexCount == 0 );   // r_ed_scene.cpp:714
            iassert(skinnedSurf->skinnedCachedOffset != RIGID_SKINNED_CACHE_OFFSET);   // r_ed_scene.cpp:718
            iassert(skinnedSurf->skinnedCachedOffset != HIDDEN_SURFACE_OFFSET);        // r_ed_scene.cpp:719
            if (gfxCmdBufSourceState.objectPlacement != modelSurf->placement)
                R_ChangeObjectPlacement(&gfxCmdBufSourceState, modelSurf->placement);
            Editor_DrawXModelSkinnedUncached(skinnedSurf->xsurf, skinnedSurf->skinnedVert);
            gfxCmdBufSourceState.objectPlacement = 0;
        }
    }

    if (haveBatch && boundVb)         // only a mesh batch needs a final tess flush
        RB_DrawEditorTessSurface(minVert, (uint16_t)maxVert);

    gfxCmdBufState.prim.vertDeclType = VERTDECL_GENERIC;
    Editor_SetEyeRelativeWorldMatrix();
}

// 0x533880  RB_DrawEditorSkinnedCached — RC_DRAW_EDITOR_SKINNEDCACHED handler.
void __cdecl RB_DrawEditorSkinnedCachedCmd(GfxRenderCommandExecState *execState)
{
    const GfxCmdEditorSkinnedCached *cmd = (const GfxCmdEditorSkinnedCached *)execState->cmd;
    // KISAK: differs from 0x533880 — clear the persisted material first.  Sub's batch-start
    // condition (0x4fe89f) compares against gfxCmdBufState.material/techType, which Sub leaves
    // set while resetting vertDeclType to GENERIC; two back-to-back flushes whose first surfs
    // share material+techType would skip RB_BeginSurface and draw with the stale GENERIC decl
    // (the binary's own rb_shade.cpp:405 assert).  A redundant RB_BeginSurface is harmless.
    gfxCmdBufState.material = 0;
    RB_DrawEditorSkinnedCached_Sub(cmd->index, cmd->amount);
    execState->cmd = (const char *)execState->cmd + cmd->header.byteCount;
}

// ── per-frame reset ───────────────────────────────────────────────────────────
// 0x4FD910  R_SortMaterials — sorts newly-registered materials and, once per
// front-end frame, resets the editor scene accumulation for the next view.
void __cdecl R_SortMaterials()
{
    bool inFrame = rg.inFrame;
    rg.inFrame = 1;

    if (rgp.needSortMaterials) {
        Material_Sort();
        rgp.needSortMaterials = 0;
    }

    if (edScene_lastFrameCount != (int)rg.frontEndFrameCount) {
        frontEndDataOut->viewInfo[frontEndDataOut->viewInfoCount].cmds = 0;  // sub_4FB170
        R_ClearScene(0);
        edScene_lastFrameCount        = rg.frontEndFrameCount;
        edSceneGlobals.sceneMeshCount = 0;
        radiant_surfCount             = 0;
        radiant_modelSurfPos          = 0;   // binary: surfPos resets with tempSkinPos
        edSceneGlobals.sceneSurfCount = 0;
    }

    edSceneGlobals.sceneSurfCount_saved = edSceneGlobals.sceneSurfCount;
    iassert(rg.inFrame);
    rg.inFrame = inFrame;
}
