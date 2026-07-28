#include <universal/q_shared.h>
#include "r_debug.h"
#include "r_init.h"

const dvar_s *r_warningRepeatDelay;
uint32_t s_warnCount[R_WARN_COUNT];

static const char *const s_warnFormat[R_WARN_COUNT] =
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

void R_WarnOncePerFrame(GfxWarningType warnType, ...)
{
    char message[1028]; // [esp+0h] [ebp-410h] BYREF
    float frameRate; // [esp+408h] [ebp-8h]
    char *vargs; // [esp+40Ch] [ebp-4h]
    va_list va; // [esp+41Ch] [ebp+Ch] BYREF

    va_start(va, warnType);
    iassert( r_warningRepeatDelay );
    frameRate = R_UpdateFrameRate();
    if (s_warnCount[warnType] < rg.frontEndFrameCount)
    {
        s_warnCount[warnType] = rg.frontEndFrameCount + (int)(frameRate * r_warningRepeatDelay->current.value);
        va_copy(vargs, va);
        _vsnprintf(message, 0x400u, s_warnFormat[warnType], va);
        vargs = 0;
        Com_PrintWarning(8, "%s\n", message);
    }
}

uint32_t frameCount;
int previous_0;
float frameRate;
double __cdecl R_UpdateFrameRate()
{
    int frameTime; // [esp+0h] [ebp-8h]
    uint32_t current; // [esp+4h] [ebp-4h]

    if (frameCount != rg.frontEndFrameCount)
    {
        if (frameCount)
        {
            if (frameCount + 1 == rg.frontEndFrameCount)
            {
                current = Sys_Milliseconds();
                frameTime = current - previous_0;
                previous_0 = current;
                if (!frameTime)
                    frameTime = 1;
                if (frameTime >= 0)
                    frameRate = 1000.0 / (double)frameTime;
                else
                    frameRate = 0.0;
            }
            else
            {
                frameRate = 0.0;
            }
        }
        else
        {
            previous_0 = Sys_Milliseconds();
        }
        frameCount = rg.frontEndFrameCount;
    }
    return frameRate;
}

void __cdecl R_WarnInitDvars()
{
    DvarLimits min; // [esp+4h] [ebp-10h]

    min.value.max = 30.0;
    min.value.min = 0.0;
    r_warningRepeatDelay = Dvar_RegisterFloat(
        "r_warningRepeatDelay",
        5.0,
        min,
        DVAR_NOFLAG,
        "Number of seconds after displaying a \"per-frame\" warning before it will display again");
}

