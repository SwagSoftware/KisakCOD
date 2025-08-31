#include "qcommon.h"
#include <xanim/xmodel.h>
#include <xanim/xanim.h>
#include <cgame/cg_local.h>

void __cdecl CM_TraceStaticModel(
    cStaticModel_s *sm,
    trace_t *trace,
    const float *start,
    const float *end,
    int contentmask)
{
    float localStart[3]; // [esp+10h] [ebp-30h] BYREF
    float delta[3]; // [esp+1Ch] [ebp-24h] BYREF
    float normal[3]; // [esp+28h] [ebp-18h] BYREF
    float localEnd[3]; // [esp+34h] [ebp-Ch] BYREF

    Vec3Sub(start, sm->origin, delta);
    MatrixTransformVector(delta, sm->invScaledAxis, localStart);
    Vec3Sub(end, sm->origin, delta);
    MatrixTransformVector(delta, sm->invScaledAxis, localEnd);
    if (XModelTraceLine(sm->xmodel, trace, localStart, localEnd, contentmask) >= 0)
    {
        iassert( trace );
        trace->hitType = TRACE_HITTYPE_ENTITY;
        trace->hitId = ENTITYNUM_WORLD;
        MatrixTransposeTransformVector(trace->normal, sm->invScaledAxis, normal);
        Vec3Normalize(normal);
        trace->normal[0] = normal[0];
        trace->normal[1] = normal[1];
        trace->normal[2] = normal[2];
    }
}

bool __cdecl CM_TraceStaticModelComplete(cStaticModel_s *sm, const float *start, const float *end, int contentmask)
{
    float localStart[3]; // [esp+0h] [ebp-50h] BYREF
    float delta[3]; // [esp+Ch] [ebp-44h] BYREF
    float localEnd[3]; // [esp+18h] [ebp-38h] BYREF
    trace_t results; // [esp+24h] [ebp-2Ch] BYREF

    Vec3Sub(start, sm->origin, delta);
    MatrixTransformVector(delta, sm->invScaledAxis, localStart);
    Vec3Sub(end, sm->origin, delta);
    MatrixTransformVector(delta, sm->invScaledAxis, localEnd);
    results.fraction = 1.0;
    return XModelTraceLine(sm->xmodel, &results, localStart, localEnd, contentmask) < 0;
}

