#pragma once
#include "r_rendercmds.h"

enum GfxWarningType : __int32
{                                       // ...
    R_WARN_FRONTEND_ENT_LIMIT = 0x0,
    R_WARN_KNOWN_MODELS = 0x1,
    R_WARN_KNOWN_SPECIAL_MODELS = 0x2,
    R_WARN_MODEL_LIGHT_CACHE = 0x3,
    R_WARN_SCENE_ENTITIES = 0x4,
    R_WARN_TEMP_SKIN_BUF_SIZE = 0x5,
    R_WARN_MAX_SKINNED_CACHE_VERTICES = 0x6,
    R_WARN_MAX_SCENE_SURFS_SIZE = 0x7,
    R_WARN_PORTAL_PLANES = 0x8,
    R_WARN_MAX_CLOUDS = 0x9,
    R_WARN_MAX_DLIGHTS = 0xA,
    R_WARN_SMODEL_LIGHTING = 0xB,
    R_WARN_MAX_DRAWSURFS = 0xC,
    R_WARN_GFX_CODE_MESH_LIMIT = 0xD,
    R_WARN_GFX_MARK_MESH_LIMIT = 0xE,
    R_WARN_MAX_SCENE_DRAWSURFS = 0xF,
    R_WARN_MAX_FX_DRAWSURFS = 0x10,
    R_WARN_NONEMISSIVE_FX_MATERIAL = 0x11,
    R_WARN_NONLIGHTMAP_MARK_MATERIAL = 0x12,
    R_WARN_PRIM_DRAW_SURF_BUFFER_SIZE = 0x13,
    R_WARN_CMDBUF_OVERFLOW = 0x14,
    R_WARN_MISSING_DECL_NONDEBUG = 0x15,
    R_WARN_MAX_DYNENT_REFS = 0x16,
    R_WARN_MAX_SCENE_DOBJ_REFS = 0x17,
    R_WARN_MAX_SCENE_MODEL_REFS = 0x18,
    R_WARN_MAX_SCENE_BRUSH_REFS = 0x19,
    R_WARN_MAX_CODE_INDS = 0x1A,
    R_WARN_MAX_CODE_VERTS = 0x1B,
    R_WARN_MAX_CODE_ARGS = 0x1C,
    R_WARN_MAX_MARK_INDS = 0x1D,
    R_WARN_MAX_MARK_VERTS = 0x1E,
    R_WARN_DEBUG_ALLOC = 0x1F,
    R_WARN_SPOT_LIGHT_LIMIT = 0x20,
    R_WARN_FX_ELEM_LIMIT = 0x21,
    R_WARN_WORKER_CMD_SIZE = 0x22,
    R_WARN_UNKNOWN_STATICMODEL_SHADER = 0x23,
    R_WARN_UNKNOWN_XMODEL_SHADER = 0x24,
    R_WARN_DYNAMIC_INDEX_BUFFER_SIZE = 0x25,
    R_WARN_TOO_MANY_LIGHT_GRID_POINTS = 0x26,
    R_WARN_FOGABLE_2DTEXT = 0x27,
    R_WARN_FOGABLE_2DGLYPH = 0x28,
    R_WARN_COUNT = 0x29,
};

const char * const s_warnFormat[41] =
{
    "entity buffer exceeded - not drawing model",
    "too many existing models (more than %i)",
    "too many existing special models (more than %i)",
    "model light cache alloc failed - not drawing model",
    "too many scene entities (more than %i)",
    "TEMP_SKIN_BUF_SIZE exceeded - not skinning surface",
    "R_MAX_SKINNED_CACHE_VERTICES((1024 * 144)) exceeded - not drawing surface",
    "MAX_SCENE_SURFS_SIZE(131072) exceeded - not drawing surface",
    "Portal plane buffer full - flushing",
    "GFX_PARTICLE_CLOUD_LIMIT(256) exceeded - not drawing particle cloud",
    "MAX_ADDED_DLIGHTS(32) exceeded.",
    "Too many visible static models - not drawing static model",
    "MAX_DRAWSURFS(32768) exceeded - not drawing geometry",
    "GFX_CODE_MESH_LIMIT(2048) exceeded - not drawing code mesh",
    "GFX_MARK_MESH_LIMIT(1536) exceeded - not drawing mark mesh",
    "Max scene drawsurfs exceed from %s - not drawing surface",
    "Max fx drawsurfs %i exceed for region %i - not drawing surface",
    "non effect material ',27h,'%s',27h,' used for effect (or code geom)",
    "non impact mark material ',27h,'%s',27h,' used for impact mark",
    "PRIM_DRAW_SURF_BUFFER_SIZE((128 * 512)) exceeded - not drawing surface",
    "command buffer overflow - not drawing surface",
    "Missing decl %s techset %s tech %s shader %s (ignore for debug settings)",
    "Max dyn ent refs exceeded",
    "Max scene dobj refs (%i) exceeded",
    "Max scene model refs (%i) exceeded",
    "Max scene brush refs",
    "GFX_CODE_MESH_INDEX_LIMIT((2048 * 6 * 2)) exceeded",
    "GFX_CODE_MESH_VERT_LIMIT((2048 * 4 * 2)) exceeded",
    "GFX_CODE_MESH_ARGS_LIMIT(256) exceeded",
    "GFX_MARK_MESH_INDEX_LIMIT((1536 * 6)) exceeded",
    "GFX_MARK_MESH_VERT_LIMIT((1536 * 4)) exceeded",
    "Out of debug memory for (%s)",
    "FX_SPOT_LIGHT_LIMIT(1) exceeded - not spawning spot light effect",
    "FX_ELEM_LIMIT(2048) exceeded - not spawning fx elem",
    "worker command size exceeded for type %i",
    "Unknown static model shader",
    "Unknown xmodel shader",
    "DYNAMIC_INDEX_BUFFER_SIZE exceeded - speed warning",
    "Too many light grid points",
    "Fogable material %s used for 2D text %s",
    "Fogable material %s used for 2D glyph",
};

void __cdecl TRACK_r_debug();
void __cdecl R_AddDebugPolygon(DebugGlobals *debugGlobalsEntry, const float *color, int pointCount, float (*points)[3]);
void __cdecl R_AddDebugLine(DebugGlobals *debugGlobalsEntry, const float *start, const float *end, const float *color);
void __cdecl R_AddDebugBox(DebugGlobals *debugGlobalsEntry, const float *mins, const float *maxs, const float *color);
void __cdecl R_AddDebugString(
    DebugGlobals *debugGlobalsEntry,
    const float *origin,
    const float *color,
    float scale,
    char *string);
void __cdecl R_AddScaledDebugString(
    DebugGlobals *debugGlobalsEntry,
    const GfxViewParms *viewParms,
    const float *origin,
    const float *color,
    char *string);
void __cdecl R_InitDebugEntry(DebugGlobals *debugGlobalsEntry);
void __cdecl R_InitDebug();
void __cdecl R_ShutdownDebugEntry(DebugGlobals *debugGlobalsEntry);
void __cdecl R_TransferDebugGlobals(DebugGlobals *debugGlobalsEntry);
void __cdecl R_ShutdownDebug();
void __cdecl R_CopyDebugStrings(
    trDebugString_t *clStrings,
    int clStringCnt,
    trDebugString_t *svStrings,
    int svStringCnt,
    int maxStringCount);
void __cdecl R_CopyDebugLines(
    trDebugLine_t *clLines,
    int clLineCnt,
    trDebugLine_t *svLines,
    int svLineCnt,
    int maxLineCount);

// r_debug_alloc
void __cdecl R_DebugAlloc(void **memPtr, int size, const char *name);
void __cdecl R_DebugFree(void **dataPtr);


// r_warn
void R_WarnOncePerFrame(GfxWarningType warnType, ...);
double __cdecl R_UpdateFrameRate();
void __cdecl R_WarnInitDvars();

extern DebugGlobals debugGlobals;
