#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\src\gfx_d3d\r_ed_vertbuf.cpp
// Editor vertex-buffer management — the per-material D3D9 vertex-buffer pool that
// backs the editor surface cache (r_ed_scene.cpp). No kisak equivalent.
//
// Visuals_InitFaceVis (brush.cpp, via Brush_CheckBuildFaceVis in DrawBrush) uploads through
// this pool on every faceVis rebuild; DrawGeo's FILLED branch consumes it (Editor_AddGeoFace
// -> R_AddEditorSurfsCmd).
//
// ── Data model (from the IDB; all heap nodes are operator new) ────────────────
//   editorGlobals (0x14F7CE0): { matVertBufs list head; unusedVertBuf scratch;
//        uint16 vbCount; IDirect3DVertexBuffer9 *vb[256] }.  Each D3D9 vertex
//        buffer holds ED_VERTBUF_VERTEX_COUNT (0x10000) GfxWorldVertex (stride 44).
//   edMatVertBuf (12B, sub_51C380): one per Material — { Material*; edVbPool*
//        pools; edMatVertBuf* next }.  Linked list rooted at editorGlobals.matVertBufs.
//   edVbPool   (12B, sub_51C280): one per (material, vb) — { uint16 buffer (1-based
//        vb index); edVbSlot* slots; edVbPool* next }.
//   edVbSlot   ( 8B, sub_51C1D0): a free run inside a vb — { uint16 firstIndex;
//        uint16 lastIndex; edVbSlot* next }.  firstIndex advances as runs are handed
//        out; lastIndex is the inclusive end of the slot.
//   handle: HIWORD = vb buffer index (1-based), LOWORD = firstIndex within that vb.

#include "stdafx.h"
#include <gfx_d3d/r_init.h>        // dx, R_ErrorDescription
#include <gfx_d3d/r_gfx.h>        // GfxWorldVertex, GfxColor, PackedUnitVec
#include <qcommon/com_pack.h>     // Vec3PackUnitVec

// d3d9 lock flag used by the original (D3DLOCK_NOOVERWRITE)
#ifndef D3DLOCK_NOOVERWRITE
#define D3DLOCK_NOOVERWRITE 0x1000
#endif

#define ED_VERTBUF_VERTEX_COUNT       0x10000   // verts per D3D9 buffer (64K)
#define ED_VERTBUF_POOL_GRANULARITY   256       // alloc rounding granularity
#define ED_VERTBUF_MAX_VERTEX_BUFFERS 256

extern char *va(const char *format, ...);
extern void Assert(const char *file, int line, int type, const char *fmt, ...); // 0x49cea0
void FatalError(int code, const char *fmt, ...);                                // 0x49a9e0

// ── pool node structs (file-local; the IDB keeps them anonymous) ──────────────
struct edVbSlot {            // 8 bytes — sub_51C1D0
    uint16_t firstIndex;     // +0
    uint16_t lastIndex;      // +2  (inclusive)
    edVbSlot *next;          // +4
};

struct edVbPool {            // 12 bytes — sub_51C280
    uint16_t buffer;         // +0  (1-based vb index)
    uint16_t _pad;
    edVbSlot *slots;         // +4
    edVbPool *next;          // +8
};

struct edMatVertBuf {        // 12 bytes — sub_51C380
    Material *material;      // +0
    edVbPool *pools;         // +4
    edMatVertBuf *next;      // +8
};

// ── editorGlobals (0x14F7CE0) ─────────────────────────────────────────────────
// matVertBufs (dword_14F7CE0) is the head of the per-material list; the rest is
// the named editorGlobals struct (unusedVertBuf scratch + vbCount + vb[256]).
static edMatVertBuf *editorGlobals_matVertBufs;   // 0x14F7CE0
static struct {
    edMatVertBuf unusedVertBuf;                    // 0x14F7CE4 {material,pools,next}
    uint16_t     vbCount;                          // 0x14F7CF0
    uint16_t     _pad;
    IDirect3DVertexBuffer9 *vb[ED_VERTBUF_MAX_VERTEX_BUFFERS]; // 0x14F7CF4 (1-based)
} editorGlobals;

// vb[] is addressed 1-based via the handle's HIWORD: buffer index B → vb[B-1].
static inline IDirect3DVertexBuffer9 *Editor_VB_ForBuffer(unsigned buffer)
{
    return editorGlobals.vb[buffer - 1];
}

// 0x51C1D0  sub_51C1D0 — alloc a free-run slot [firstIndex, firstIndex+vertCount-1].
static edVbSlot *Editor_VB_AllocSlot_New(int vertCount, uint16_t firstIndex)
{
    iassert(vertCount > 0);                                   // 0x51c1d0 (level 0)
    iassert(vertCount <= ED_VERTBUF_VERTEX_COUNT);            // 0x51c1ef (level 0)
    edVbSlot *slot = (edVbSlot *)operator new(sizeof(edVbSlot));
    if (!slot)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 83, 1, "%s", "slot");   // LEVEL 1 -> KEEP_VERBOSE
    uint16_t last = (uint16_t)(firstIndex + vertCount - 1);
    slot->firstIndex = firstIndex;
    slot->lastIndex  = last;
    iassert(slot->lastIndex >= slot->firstIndex);            // 0x51c24c (level 0)
    return slot;
}

// 0x51C280  sub_51C280 — alloc a pool node for a given vb buffer index.
static edVbPool *Editor_VB_AllocPool(uint16_t buffer)
{
    iassert(buffer);                                          // 0x51c280 (level 0)
    edVbPool *pool = (edVbPool *)operator new(sizeof(edVbPool));
    if (!pool)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 104, 1, "%s", "pool");
    pool->buffer = buffer;
    pool->slots  = 0;
    return pool;
}

// 0x51C2F0  sub_51C2F0 — free a pool list (and each pool's slot list).
static void Editor_VB_FreePoolList(edVbPool *pool)
{
    while (pool) {
        edVbSlot *slot = pool->slots;
        edVbPool *nextPool = pool->next;
        while (slot) {
            edVbSlot *nextSlot = slot->next;
            free(slot);
            slot = nextSlot;
        }
        free(pool);
        pool = nextPool;
    }
}

// KISAK (not in the binary): release every editor D3DPOOL_DEFAULT vertex buffer + all pool
// bookkeeping before a device Reset() — default-pool resources must be freed or Reset fails.
// The pool re-creates its buffers lazily on the next Editor_VB_Upload.  Called from
// R_ReleaseForShutdownOrReset.
void Editor_VB_ReleaseForReset()
{
    for (unsigned i = 0; i < editorGlobals.vbCount; ++i)
    {
        if (editorGlobals.vb[i])
        {
            editorGlobals.vb[i]->Release();
            editorGlobals.vb[i] = nullptr;
        }
    }
    editorGlobals.vbCount = 0;

    edMatVertBuf *m = editorGlobals_matVertBufs;
    while (m)
    {
        edMatVertBuf *next = m->next;
        Editor_VB_FreePoolList(m->pools);
        operator delete(m);            // paired with operator new in Editor_GetMaterialVertexBuffer
        m = next;
    }
    editorGlobals_matVertBufs = nullptr;
    Editor_VB_FreePoolList(editorGlobals.unusedVertBuf.pools);
    editorGlobals.unusedVertBuf.pools = nullptr;
}

// 0x51C380  sub_51C380 — find/create the per-material vertex-buffer record.
static edMatVertBuf *Editor_GetMaterialVertexBuffer(Material *material)
{
    edMatVertBuf *m = editorGlobals_matVertBufs;
    while (m) {
        if (m->material == material)
            return m;
        m = m->next;
    }
    m = (edMatVertBuf *)operator new(sizeof(edMatVertBuf));
    m->material = material;
    m->pools    = 0;
    m->next     = editorGlobals_matVertBufs;
    editorGlobals_matVertBufs = m;
    return m;
}

// 0x51C420  editorvb_gethandle — return a cached handle for an EXACT-fit free slot
// (lastIndex-firstIndex == vertCount-1), consuming the slot. 0 if none.
static unsigned int Editor_VB_GetHandle(edMatVertBuf *matVertBuf, int vertCount)
{
    iassert(matVertBuf);                                      // 0x51c420 (level 0)

    edVbPool *pool = matVertBuf->pools;
    while (pool) {
        edVbSlot **pslot = &pool->slots;
        edVbSlot *slot   = pool->slots;
        while (slot) {
            if (slot->lastIndex - slot->firstIndex == vertCount - 1) {
                uint16_t buffer = pool->buffer;
                uint16_t first  = slot->firstIndex;
                vassert(buffer > 0 && buffer <= editorGlobals.vbCount, "%s", va("%i %i", buffer, editorGlobals.vbCount));
                *pslot = slot->next;           // unlink the consumed slot
                free(slot);
                return (unsigned int)((buffer << 16) | first);
            }
            pslot = &slot->next;
            slot  = slot->next;
        }
        pool = pool->next;
    }
    return 0;
}

// 0x51C500  sub_51C500 — allocate `vertCount` verts from the existing pools,
// best-fit by least waste (waste >= minWaste a3). Returns a handle, or 0 if no
// pool can satisfy it. An exact-fit slot is consumed via Editor_VB_GetHandle.
static unsigned int Editor_VB_AllocFromPools(edMatVertBuf *matVertBuf, int vertCount, int minWaste)
{
    iassert(matVertBuf);                                      // 0x51c500 (level 0)

    edVbSlot *slotBest = 0;
    edVbPool *poolBest = 0;
    int wasteBest = 0x7FFFFFFF;

    for (edVbPool *pool = matVertBuf->pools; pool; pool = pool->next) {
        for (edVbSlot *slot = pool->slots; slot; slot = slot->next) {
            int avail = slot->lastIndex - slot->firstIndex + 1;     // verts left in slot
            if (avail == vertCount) {
                // exact fit — must be granularity-aligned, hand it back whole.
                iassert((vertCount % ED_VERTBUF_POOL_GRANULARITY) == 0);   // 0x51c5?? (level 0)
                unsigned int handle = Editor_VB_GetHandle(matVertBuf, vertCount);
                iassert(handle);                                            // (level 0)
                return handle;
            }
            int waste = avail - vertCount;
            if (waste >= minWaste && waste < wasteBest) {
                slotBest  = slot;
                poolBest  = pool;
                wasteBest = waste;
            }
        }
    }

    if (!poolBest)
        return 0;
    if (!slotBest)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 289, 1, "%s", "slotBest");
    if (wasteBest >= ED_VERTBUF_VERTEX_COUNT)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 290, 1, "%s", "wasteBest < ED_VERTBUF_VERTEX_COUNT");
    if (slotBest->lastIndex - slotBest->firstIndex + 1 <= vertCount)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 291, 1, "%s", "slotBest->lastIndex - slotBest->firstIndex + 1 > vertCount");

    uint16_t buffer = poolBest->buffer;
    uint16_t first  = slotBest->firstIndex;
    // 0x51c6a7 (IDA line 173, level 0): the extra va("%i %i",...) prints buffer/vbCount, so
    // this is a vassert (matching the sibling in Editor_VB_GetHandle).
    vassert(buffer > 0 && buffer <= editorGlobals.vbCount, "%s", va("%i %i", buffer, editorGlobals.vbCount));

    slotBest->firstIndex = (uint16_t)(slotBest->firstIndex + vertCount); // carve from front
    if (slotBest->firstIndex < vertCount)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 295, 1, "%s", "slotBest->firstIndex >= vertCount");
    return (unsigned int)((buffer << 16) | first);
}

// 0x51C6F0  Editor_CreateAdditionalVertexBuffer — create one more D3D9 vertex
// buffer (ED_VERTBUF_VERTEX_COUNT verts) and seed editorGlobals.unusedVertBuf with
// a whole-buffer pool/slot covering it.
static void Editor_CreateAdditionalVertexBuffer()
{
    if (editorGlobals.vbCount > ED_VERTBUF_MAX_VERTEX_BUFFERS)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 306, 1, "%s", "editorGlobals.vbCount <= ED_VERTBUF_MAX_VERTEX_BUFFERS");
    if (editorGlobals.vbCount == ED_VERTBUF_MAX_VERTEX_BUFFERS)
        FatalError(0, "ED_VERTBUF_MAX_VERTEX_BUFFERS (%i) exceeded", ED_VERTBUF_MAX_VERTEX_BUFFERS);
    if (editorGlobals.vbCount >= 254)
        MessageBoxA(GetActiveWindow(),
            "WARNING!\nSave your work NOW!\nThe geometry buffer is almost full.  If it becomes full, the program will exit and your work will be lost.\nRe-optimizing geometry will probably make this problem go away for a while.",
            "Warning", MB_ICONEXCLAMATION);

    HRESULT hr = dx.device->CreateVertexBuffer(
        ED_VERTBUF_VERTEX_COUNT * sizeof(GfxWorldVertex), // 0x2C0000
        520, 0, D3DPOOL_DEFAULT, &editorGlobals.vb[editorGlobals.vbCount], 0);
    if (hr < 0)
        FatalError(0, "Couldn't create another editor vertex buffer: %s", R_ErrorDescription(hr));

    ++editorGlobals.vbCount;
    edVbPool *pool = Editor_VB_AllocPool(editorGlobals.vbCount);
    edVbSlot *slot = (edVbSlot *)operator new(sizeof(edVbSlot));
    if (!slot)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 83, 1, "%s", "slot");
    slot->firstIndex = 0;
    slot->lastIndex  = 0xFFFF;                  // whole 64K-vert buffer
    pool->slots      = slot;
    slot->next       = 0;
    pool->next       = editorGlobals.unusedVertBuf.pools;
    editorGlobals.unusedVertBuf.pools = pool;
}

// 0x51C810  sub_51C810 — grow a material's pools by carving a granularity-rounded
// run out of the global unusedVertBuf scratch (creating a new D3D9 buffer if the
// scratch is exhausted) and prepending it as a fresh pool on `matVertBuf`.
static void Editor_VB_GrowPool(int minVertCount, edMatVertBuf *matVertBuf)
{
    iassert(matVertBuf);                                      // 0x51c816 (level 0)
    iassert(minVertCount > 0);                                // 0x51c83f (level 0)

    int snapped = (minVertCount + (ED_VERTBUF_POOL_GRANULARITY - 1)) & ~(ED_VERTBUF_POOL_GRANULARITY - 1);
    if (snapped <= 0 || snapped > ED_VERTBUF_VERTEX_COUNT)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 346, 1, "%s", "snappedVertCount > 0 && snappedVertCount <= ED_VERTBUF_VERTEX_COUNT");
    if (snapped % ED_VERTBUF_POOL_GRANULARITY)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 347, 1, "%s", "(snappedVertCount % ED_VERTBUF_POOL_GRANULARITY) == 0");
    if (editorGlobals.unusedVertBuf.material)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 349, 1, "%s", "editorGlobals.unusedVertBuf.material == NULL");
    if (editorGlobals.unusedVertBuf.next)   // 0x51c8ec (level 1) — was MISSING vs the binary
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 350, 1, "%s", "editorGlobals.unusedVertBuf.next == NULL");

    unsigned int handle = Editor_VB_AllocFromPools(&editorGlobals.unusedVertBuf, snapped, 0);
    if (!handle) {
        Editor_CreateAdditionalVertexBuffer();
        handle = Editor_VB_AllocFromPools(&editorGlobals.unusedVertBuf, snapped, 0);
        if (!handle)
            Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 356, 1, "%s", "ev.handle");
    }

    edVbPool *pool = Editor_VB_AllocPool((uint16_t)(handle >> 16));
    edVbSlot *slot = Editor_VB_AllocSlot_New(snapped, (uint16_t)handle);
    pool->slots = slot;
    slot->next  = 0;
    pool->next  = matVertBuf->pools;
    matVertBuf->pools = pool;
}

// 0x51CCE0  Editor_GetVertexBufferAndIndex — unpack a handle to (vb*, firstIndex).
void Editor_GetVertexBufferAndIndex(unsigned int handle, IDirect3DVertexBuffer9 **vb, uint16_t *firstIndex)
{
    iassert(vb);                                              // 0x51cce0 (level 0)
    iassert(firstIndex);                                      // (level 0)
    *vb = Editor_VB_ForBuffer(handle >> 16);
    *firstIndex = (uint16_t)handle;
}

// 0x51CD50  sub_51CD50 — lock the handle's run of the D3D9 buffer and pack
// `vertCount` GfxWorldVertex into it.
//
// RECONSTRUCTED FROM INTENT: the IDB body is reused-register pointer arithmetic over six
// stack arrays.  Inputs are the per-vertex arrays Visuals_InitFaceVis fills: xyz, normal,
// tangent, binormal (vec3 each), texCoord (2 floats), color (GfxColor).  normal/tangent/
// binormalSign feed lighting only and are packed with Vec3PackUnitVec.
static void Editor_VB_WriteVertices(unsigned int handle, const float *tangent, int vertCount,
    const float *xyz, const float *binormal, const float *normal,
    const float *texCoord, const float *color)
{
    iassert(handle);                                          // 0x51cd72 (level 0)
    iassert(vertCount > 0);                                   // 0x51cd95 (level 0)
    iassert(xyz);                                             // 0x51cdb9 (level 0)
    iassert(tangent);                                         // 0x51cddb (level 0)
    iassert(binormal);                                        // 0x51cdfe (level 0)
    iassert(normal);                                          // 0x51ce20 (level 0)
    iassert(texCoord);                                        // 0x51ce43 (level 0)
    iassert(color);                                           // 0x51ce67 (level 0)

    IDirect3DVertexBuffer9 *vb = 0;
    uint16_t firstIndex = 0;
    Editor_GetVertexBufferAndIndex(handle, &vb, &firstIndex);
    iassert(vb);                                              // 0x51ce9d (level 0)

    int offset = sizeof(GfxWorldVertex) * firstIndex;
    GfxWorldVertex *out = 0;
    HRESULT hr = vb->Lock(offset, sizeof(GfxWorldVertex) * vertCount, (void **)&out, D3DLOCK_NOOVERWRITE);
    if (hr < 0) {
        ++g_disableRendering;
        FatalError(0, "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp (%i) vb->Lock( offset, vertCount * sizeof( GfxWorldVertex ), &lockedBuf, 0x00001000L ) failed: %s\n", 205, R_ErrorDescription(hr));
    }

    for (int i = 0; i < vertCount; ++i) {
        const float *p = xyz      + 3 * i;
        const float *n = normal   + 3 * i;
        const float *t = tangent  + 3 * i;
        const float *b = binormal + 3 * i;
        GfxWorldVertex *o = &out[i];

        o->xyz[0] = p[0];
        o->xyz[1] = p[1];
        o->xyz[2] = p[2];

        // binormalSign = handedness of the (normal, tangent, binormal) frame.
        float cr[3];
        Vec3Cross(n, t, cr);
        float d = b[0] * cr[0] + b[1] * cr[1] + b[2] * cr[2];
        o->binormalSign = (d < 0.0f) ? -1.0f : 1.0f;

        o->color = ((const GfxColor *)color)[i];
        o->texCoord[0] = texCoord[2 * i];
        o->texCoord[1] = texCoord[2 * i + 1];
        // 0x51cd50 never writes lmapCoord (@+0x1C/0x20): the loop stores only offsets
        // 0/4/8/0xC/0x10/0x14/0x18/0x24/0x28.
        o->normal  = Vec3PackUnitVec(n);
        o->tangent = Vec3PackUnitVec(t);
    }

    hr = vb->Unlock();
    if (hr < 0) {
        ++g_disableRendering;
        FatalError(0, "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp (%i) vb->Unlock() failed: %s\n", 219, R_ErrorDescription(hr));
    }
}

// 0x51D1D0  sub_51D1D0 — upload `vertCount` verts for `material`; returns the
// packed handle (HIWORD=buffer, LOWORD=firstIndex) the surf cache stores per layer.
unsigned int Editor_VB_Upload(Material *material, int vertCount,
    const float *xyz, const float *tangent, const float *binormal,
    const float *normal, const float *texCoord, const float *color)
{
    iassert(material);                                        // 0x51d1db (level 0)
    iassert(vertCount > 0);                                   // 0x51d1ff (level 0)
    iassert(vertCount <= ED_VERTBUF_VERTEX_COUNT);            // 0x51d225 (level 0)

    edMatVertBuf *matVertBuf = Editor_GetMaterialVertexBuffer(material);
    if (!matVertBuf)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 377, 1, "%s", "matVertBuf");

    unsigned int handle = Editor_VB_GetHandle(matVertBuf, vertCount);
    if (!handle)
        handle = Editor_VB_AllocFromPools(matVertBuf, vertCount, 1);
    if (!handle) {
        Editor_VB_GrowPool(vertCount, matVertBuf);
        // 0x51d2f0-0x51d3a2: four grow-branch invariants (level 1).
        if (!matVertBuf)
            Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 394, 1, "%s", "matVertBuf");
        if (!matVertBuf->pools)
            Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 395, 1, "%s", "matVertBuf->pools");
        if (!matVertBuf->pools->slots)
            Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 396, 1, "%s", "matVertBuf->pools->slots");
        if (matVertBuf->pools->slots->lastIndex - matVertBuf->pools->slots->firstIndex + 1 < vertCount)
            Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 397, 1, "%s\n\t%s",
                   "matVertBuf->pools->slots->lastIndex - matVertBuf->pools->slots->firstIndex + 1 >= vertCount",
                   va("%i %i %i", matVertBuf->pools->slots->lastIndex, matVertBuf->pools->slots->firstIndex, vertCount));
        handle = Editor_VB_AllocFromPools(matVertBuf, vertCount, 1);
        if (!handle)
            Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 400, 1, "%s\n\t(vertCount) = %i", "(handle)", vertCount);
    }

    Editor_VB_WriteVertices(handle, tangent, vertCount, xyz, binormal, normal, texCoord, color);
    return handle;
}

// ── FREE PATH ─────────────────────────────────────────────────────────────────
// The release side of the per-material VB pool.  R_Ed_FreeVertices (0x51CB70) dispatches
// through
// R_Ed_FreeVertices_Inner (0x51CAE0) to R_Ed_VB_FreeSlot_Coalesce (0x51C9A0), which
// returns a freed [firstIndex, firstIndex+vertCount-1] run to the material's pool
// free-list, coalescing with any adjacent free slots.  A run that cannot be merged
// with an existing slot becomes a brand-new slot (creating its own pool if the matching
// vb-buffer index has none yet).
//
// handle (a3) packing — same as the alloc side: HIWORD = vb buffer index (1-based),
// LOWORD = firstIndex within that vb.  The disasm (0x51c9b2 onward) confirms coalesce
// reads only the LOWORD (firstIndex) of the packed handle; the inner uses HIWORD to
// pick the pool whose `buffer` matches.

// 0x51C9A0  sub_51C9A0 — return the freed run [firstIndex, firstIndex+vertCount-1] to
// `pool`'s slot free-list, coalescing it with an immediately-adjacent free slot (and
// merging two slots into one if it bridges the gap between them).  Returns true if the
// run was merged into an existing slot (so the inner won't allocate a fresh one).
static bool R_Ed_VB_FreeSlot_Coalesce(edVbPool *pool, int vertCount, uint16_t firstIndex)
{
    if (!pool)                                                // 0x51c9a7
        return false;

    int lastFreedIndex = firstIndex + vertCount;             // 0x51c9b9 (one past the freed run)
    if (lastFreedIndex <= 0 || lastFreedIndex > ED_VERTBUF_VERTEX_COUNT)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 421, 1, "%s", "lastFreedIndex > 0 && lastFreedIndex <= ED_VERTBUF_VERTEX_COUNT");

    edVbSlot *mergedSlot = 0;                                 // edi
    edVbSlot **pslot = &pool->slots;                         // ebx = &pool->slots, then &slot->next
    if (!*pslot)                                             // 0x51c9ee — empty free-list
        return false;

    edVbSlot *slot;
    while ((slot = *pslot) != 0)                              // 0x51c9f2
    {
        if (slot->firstIndex == lastFreedIndex)              // freed run is immediately BEFORE this slot
        {
            if (mergedSlot)                                  // 0x51ca00 — bridge: mergedSlot ends at the gap
            {
                if (mergedSlot->lastIndex != lastFreedIndex - 1)   // 0x51ca47
                    Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 430, 1, "%s", "mergedSlot->lastIndex == lastFreedIndex - 1");
                mergedSlot->lastIndex = slot->lastIndex;     // 0x51ca70 — absorb `slot` into mergedSlot
                *pslot = slot->next;                         // 0x51ca7c — unlink `slot`
                free(slot);
                return true;
            }
            slot->firstIndex = firstIndex;                   // 0x51ca06 — extend `slot` start backward
            mergedSlot = slot;                               // 0x51ca2a
        }
        else if (slot->lastIndex == (uint16_t)(firstIndex - 1))   // freed run is immediately AFTER this slot
        {
            if (mergedSlot)                                  // 0x51ca1f — bridge: mergedSlot starts at the gap
            {
                if (mergedSlot->firstIndex != firstIndex)    // 0x51ca8f
                    Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 444, 1, "%s", "mergedSlot->firstIndex == ev.s.firstIndex");
                mergedSlot->lastIndex = (uint16_t)(slot->lastIndex + vertCount);  // 0x51cabf
                *pslot = slot->next;                         // 0x51cac6 — unlink `slot`
                free(slot);
                return true;
            }
            slot->lastIndex = (uint16_t)(slot->lastIndex + vertCount);   // 0x51ca26 — extend `slot` end forward
            mergedSlot = slot;                               // 0x51ca2a
        }
        pslot = &slot->next;                                 // 0x51ca2c (ebx = *ebx + 4)
    }

    return mergedSlot != 0;                                   // 0x51ca36/0x51ca38
}

// 0x51CAE0  sub_51CAE0 — release the run identified by `handle` for a given
// per-material VB record.  Finds the pool whose vb-buffer index matches the handle's
// HIWORD, then coalesces the freed run into that pool; if it couldn't be merged, makes
// a fresh slot (creating the pool first if the buffer had none).
static char R_Ed_FreeVertices_Inner(edMatVertBuf *matVertBuf, int vertCount, int handle)
{
    if (!matVertBuf)                                          // 0x51caea
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 469, 0, "%s", "matVertBuf");  // binary pushes eax (=null matVertBuf) as the type arg

    uint16_t buffer = (uint16_t)((unsigned int)handle >> 16);  // HIWORD(handle) = vb buffer index

    edVbPool *pool = matVertBuf->pools;                       // 0x51cb09 = matVertBuf->pools
    while (pool)                                              // 0x51cb13 — find the pool for this vb buffer
    {
        if (pool->buffer == buffer)
            break;
        pool = pool->next;                                   // 0x51cb18 = pool->next
    }

    char merged = R_Ed_VB_FreeSlot_Coalesce(pool, vertCount, (uint16_t)handle);  // 0x51cb29 (LOWORD = firstIndex)
    if (!merged)
    {
        if (!pool)                                           // 0x51cb37 — no pool for this vb yet
        {
            pool = Editor_VB_AllocPool(buffer);              // 0x51cb39
            pool->next = matVertBuf->pools;                  // 0x51cb43
            matVertBuf->pools = pool;                        // 0x51cb46
        }
        edVbSlot *slot = Editor_VB_AllocSlot_New(vertCount, (uint16_t)handle);  // 0x51cb4f
        slot->next  = pool->slots;                           // 0x51cb54/0x51cb57
        pool->slots = slot;                                  // 0x51cb5a
    }
    return merged;                                           // 0x51cb31 -> al
}

// 0x51CB70  R_Ed_FreeVertices — public entry: release `vertCount` verts (identified by
// the packed `vertHandle`) from `handle`'s per-material VB pool.  THIS replaces the
// engine_stubs.cpp FATAL stub.
char R_Ed_FreeVertices(Material *handle, int vertCount, int vertHandle)
{
    iassert(vertCount > 0);                                   // 0x51cb7a (level 0)  r_ed_vertbuf.cpp:496
    iassert(vertCount <= ED_VERTBUF_VERTEX_COUNT);           // 0x51cba0 (level 0)  r_ed_vertbuf.cpp:497

    edMatVertBuf *matVertBuf = Editor_GetMaterialVertexBuffer(handle);   // 0x51cbc3
    if (!matVertBuf)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 500, 1, "%s", "matVertBuf");

    return R_Ed_FreeVertices_Inner(matVertBuf, vertCount, vertHandle);   // 0x51cbf3
}

// engine_stubs.cpp / brush.cpp call this through the unported-name alias `sub_51CB70`.
char sub_51CB70(Material *handle, int vertCount, int vertHandle)
{
    return R_Ed_FreeVertices(handle, vertCount, vertHandle);
}

// 0x51CC00  editorVB_freeBuffers — tear down every per-material record, the unused
// scratch pool, and release all D3D9 vertex buffers.  Called on shutdown / reopt.
void editorVB_freeBuffers()
{
    edMatVertBuf *m = editorGlobals_matVertBufs;
    while (m) {
        edMatVertBuf *next = m->next;
        Editor_VB_FreePoolList(m->pools);
        free(m);
        m = next;
    }
    editorGlobals_matVertBufs = 0;

    if (editorGlobals.unusedVertBuf.next)
        Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 519, 1, "%s", "editorGlobals.unusedVertBuf.next == NULL");
    Editor_VB_FreePoolList(editorGlobals.unusedVertBuf.pools);
    editorGlobals.unusedVertBuf.pools = 0;

    while (editorGlobals.vbCount) {
        uint16_t idx = --editorGlobals.vbCount;
        if (!editorGlobals.vb[idx])
            Assert("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_ed_vertbuf.cpp", 526, 1, "%s", "editorGlobals.vb[editorGlobals.vbCount]");
        editorGlobals.vb[idx]->Release();
        // 0x51cccb only Releases — it does NOT null vb[idx] (the slot is dead once vbCount
        // is decremented).
    }
}
