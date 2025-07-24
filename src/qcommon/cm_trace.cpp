#include "qcommon.h"
#include "threads.h"

#include <xanim/xanim.h>
#include <game/game_public.h>
#include <universal/profile.h>

unsigned __int16 __cdecl Trace_GetEntityHitId(const trace_t *trace)
{
    if (!trace)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 13, 0, "%s", "trace");
    if (trace->hitType == TRACE_HITTYPE_DYNENT_MODEL || trace->hitType == TRACE_HITTYPE_DYNENT_BRUSH)
        return 1022;
    if (trace->hitType == TRACE_HITTYPE_ENTITY)
        return trace->hitId;
    else
        return 1023;
}

unsigned __int16 __cdecl Trace_GetDynEntHitId(const trace_t *trace, DynEntityDrawType *drawType)
{
    if (!trace)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 25, 0, "%s", "trace");
    if (!drawType)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 27, 0, "%s", "drawType");
    if (trace->hitType == TRACE_HITTYPE_DYNENT_MODEL)
    {
        *drawType = DYNENT_DRAW_MODEL;
        return trace->hitId;
    }
    else if (trace->hitType == TRACE_HITTYPE_DYNENT_BRUSH)
    {
        *drawType = DYNENT_DRAW_BRUSH;
        return trace->hitId;
    }
    else
    {
        return -1;
    }
}

unsigned int __cdecl CM_TempBoxModel(const float *mins, const float *maxs, int contents)
{
    float *v4; // [esp+0h] [ebp-18h]
    cbrush_t *v5; // [esp+4h] [ebp-14h]
    float *v6; // [esp+8h] [ebp-10h]
    cmodel_t *v7; // [esp+Ch] [ebp-Ch]
    cbrush_t *box_brush; // [esp+10h] [ebp-8h] BYREF
    cmodel_t *box_model; // [esp+14h] [ebp-4h] BYREF

    CM_GetBox(&box_brush, &box_model);
    v7 = box_model;
    box_model->mins[0] = *mins;
    v7->mins[1] = mins[1];
    v7->mins[2] = mins[2];
    v6 = box_model->maxs;
    box_model->maxs[0] = *maxs;
    v6[1] = maxs[1];
    v6[2] = maxs[2];
    v5 = box_brush;
    box_brush->mins[0] = *mins;
    v5->mins[1] = mins[1];
    v5->mins[2] = mins[2];
    v4 = box_brush->maxs;
    box_brush->maxs[0] = *maxs;
    v4[1] = maxs[1];
    v4[2] = maxs[2];
    box_brush->contents = contents;
    return 4095;
}

void __cdecl CM_GetBox(cbrush_t **box_brush, cmodel_t **box_model)
{
    TraceThreadInfo *value; // [esp+0h] [ebp-4h]

    value = (TraceThreadInfo *)Sys_GetValue(3);
    if (!value)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 76, 0, "%s", "value");
    if (value->box_model->leaf.brushContents != -1)
        MyAssertHandler(
            ".\\qcommon\\cm_trace.cpp",
            78,
            0,
            "%s\n\t(value->box_model->leaf.brushContents) = %i",
            "(value->box_model->leaf.brushContents == ~0)",
            value->box_model->leaf.brushContents);
    if (value->box_model->leaf.terrainContents)
        MyAssertHandler(
            ".\\qcommon\\cm_trace.cpp",
            79,
            0,
            "%s\n\t(value->box_model->leaf.terrainContents) = %i",
            "(value->box_model->leaf.terrainContents == 0)",
            value->box_model->leaf.terrainContents);
    *box_brush = value->box_brush;
    *box_model = value->box_model;
}

bool __cdecl CM_ClipHandleIsValid(unsigned int handle)
{
    return handle < cm.numSubModels || handle == 4095;
}

cmodel_t *__cdecl CM_ClipHandleToModel(unsigned int handle)
{
    const char *v2; // eax
    cbrush_t *box_brush; // [esp+0h] [ebp-8h] BYREF
    cmodel_t *box_model; // [esp+4h] [ebp-4h] BYREF

    if (handle < cm.numSubModels)
        return &cm.cmodels[handle];
    if (handle != 4095)
    {
        v2 = va("handle: %d, cm.numSubModels: %d", handle, cm.numSubModels);
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 133, 0, "%s\n\t%s", "handle == CAPSULE_MODEL_HANDLE", v2);
    }
    CM_GetBox(&box_brush, &box_model);
    return box_model;
}

int __cdecl CM_ContentsOfModel(unsigned int handle)
{
    cmodel_t *v1; // edx

    v1 = CM_ClipHandleToModel(handle);
    return v1->leaf.terrainContents | v1->leaf.brushContents;
}

void __cdecl CM_BoxTrace(
    trace_t *results,
    const float *start,
    float *end,
    const float *mins,
    const float *maxs,
    unsigned int model,
    int brushmask)
{
    memset((unsigned __int8 *)results, 0, sizeof(trace_t));
    results->fraction = 1.0;
    CM_Trace(results, start, end, mins, maxs, model, brushmask);
}

void __cdecl CM_Trace(
    trace_t *results,
    const float *start,
    float *end,
    const float *mins,
    const float *maxs,
    unsigned int model,
    int brushmask)
{
    const char *v7; // eax
    const char *v8; // eax
    double v9; // st7
    double v10; // st7
    float v12; // [esp+1Ch] [ebp-148h]
    float v13; // [esp+20h] [ebp-144h]
    float v14; // [esp+24h] [ebp-140h]
    cmodel_t *cmod; // [esp+78h] [ebp-ECh]
    traceWork_t tw; // [esp+7Ch] [ebp-E8h] BYREF
    float offset[3]; // [esp+12Ch] [ebp-38h]
    float oldFrac; // [esp+138h] [ebp-2Ch]
    float start_[4]; // [esp+13Ch] [ebp-28h] BYREF
    int oldSurfaceFlags; // [esp+14Ch] [ebp-18h]
    int i; // [esp+150h] [ebp-14h]
    float end_[4]; // [esp+154h] [ebp-10h] BYREF

    iassert(cm.numNodes);
    iassert(mins);
    iassert(maxs);
    iassert(!IS_NAN(end[0]) && !IS_NAN(end[1]) && !IS_NAN(end[2]));

    PROF_SCOPED("CM_Trace");

    cmod = CM_ClipHandleToModel(model);
    tw.contents = brushmask;
    for (i = 0; i < 3; ++i)
    {
        if (mins[i] > (double)maxs[i])
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1335, 0, "%s", "maxs[i] >= mins[i]");
        offset[i] = (mins[i] + maxs[i]) * 0.5;
        tw.size[i] = maxs[i] - offset[i];
        tw.extents.start[i] = start[i] + offset[i];
        tw.delta[i - 6] = end[i] + offset[i];
        tw.midpoint[i] = (tw.extents.start[i] + tw.delta[i - 6]) * 0.5;
        tw.delta[i] = tw.delta[i - 6] - tw.extents.start[i];
        tw.halfDelta[i] = tw.delta[i] * 0.5;
        v14 = I_fabs(tw.halfDelta[i]);
        tw.halfDeltaAbs[i] = v14;
    }
    CM_CalcTraceExtents(&tw.extents);
    tw.deltaLenSq = Vec3LengthSq(tw.delta);
    v13 = sqrt(tw.deltaLenSq);
    tw.deltaLen = v13;
    if (tw.size[0] - tw.size[2] >= 0.009999999776482582)
    {
        v7 = va("tw.size[0]: %f, tw.size[2]: %f", tw.size[0], tw.size[2]);
        MyAssertHandler(
            ".\\qcommon\\cm_trace.cpp",
            1349,
            0,
            "%s\n\t%s",
            "(tw.size[0] - tw.size[2]) < CAPSULE_SIZE_EPSILON",
            v7);
    }
    if (tw.size[1] - tw.size[2] >= 0.009999999776482582)
    {
        v8 = va("tw.size[1]: %f, tw.size[2]: %f", tw.size[1], tw.size[2]);
        MyAssertHandler(
            ".\\qcommon\\cm_trace.cpp",
            1350,
            0,
            "%s\n\t%s",
            "(tw.size[1] - tw.size[2]) < CAPSULE_SIZE_EPSILON",
            v8);
    }
    if (tw.size[2] >= (double)tw.size[0])
        v12 = tw.size[0];
    else
        v12 = tw.size[2];
    tw.radius = v12;
    tw.boundingRadius = Vec3Length(tw.size);
    tw.offsetZ = tw.size[2] - tw.radius;
    for (i = 0; i < 2; ++i)
    {
        if (tw.delta[i - 6] <= (double)tw.extents.start[i])
        {
            tw.bounds[0][i] = tw.delta[i - 6] - tw.radius;
            v9 = tw.extents.start[i] + tw.radius;
        }
        else
        {
            tw.bounds[0][i] = tw.extents.start[i] - tw.radius;
            v9 = tw.delta[i - 6] + tw.radius;
        }
        tw.bounds[1][i] = v9;
    }
    if (tw.offsetZ < 0.0)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1374, 0, "%s", "tw.offsetZ >= 0");
    if (tw.extents.end[2] <= (double)tw.extents.start[2])
    {
        tw.bounds[0][2] = tw.extents.end[2] - tw.offsetZ - tw.radius;
        v10 = tw.extents.start[2] + tw.offsetZ + tw.radius;
    }
    else
    {
        tw.bounds[0][2] = tw.extents.start[2] - tw.offsetZ - tw.radius;
        v10 = tw.extents.end[2] + tw.offsetZ + tw.radius;
    }
    tw.bounds[1][2] = v10;
    CM_SetAxialCullOnly(&tw);
    CM_GetTraceThreadInfo(&tw.threadInfo);
    if (results->surfaceFlags == -1)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1392, 0, "%s", "results->surfaceFlags != SURF_INVALID");
    oldSurfaceFlags = results->surfaceFlags;
    oldFrac = results->fraction;
    results->surfaceFlags = -1;
    if (*end == *start && end[1] == start[1] && end[2] == start[2])
    {
        tw.isPoint = 0;
        if (model)
        {
            if (model == 4095)
            {
                if ((tw.contents & tw.threadInfo.box_brush->contents) != 0)
                    CM_TestCapsuleInCapsule(&tw, results);
                if ((COERCE_UNSIGNED_INT(results->fraction) & 0x7F800000) == 0x7F800000)
                    MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1412, 0, "%s", "!IS_NAN(results->fraction)");
            }
            else
            {
                if (!results->allsolid)
                    CM_TestInLeaf(&tw, &cmod->leaf, results);
                if ((COERCE_UNSIGNED_INT(results->fraction) & 0x7F800000) == 0x7F800000)
                    MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1419, 0, "%s", "!IS_NAN(results->fraction)");
            }
        }
        else
        {
            CM_PositionTest(&tw, results);
        }
    }
    else
    {
        if (tw.size[0] < 0.0)
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1432, 0, "%s", "tw.size[0] >= 0");
        if (tw.size[1] < 0.0)
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1433, 0, "%s", "tw.size[1] >= 0");
        if (tw.size[2] < 0.0)
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1434, 0, "%s", "tw.size[2] >= 0");
        tw.isPoint = tw.size[0] + tw.size[1] + tw.size[2] == 0.0;
        if (tw.offsetZ < 0.0)
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1438, 0, "%s", "tw.offsetZ >= 0");
        tw.radiusOffset[0] = tw.radius;
        tw.radiusOffset[1] = tw.radius;
        tw.radiusOffset[2] = tw.radius + tw.offsetZ;
        if (model)
        {
            if (model == 4095)
            {
                if ((tw.contents & tw.threadInfo.box_brush->contents) != 0)
                    CM_TraceCapsuleThroughCapsule(&tw, results);
            }
            else
            {
                CM_TraceThroughLeaf(&tw, &cmod->leaf, results);
            }
        }
        else
        {
            start_[0] = tw.extents.start[0];
            start_[1] = tw.extents.start[1];
            start_[2] = tw.extents.start[2];
            start_[3] = 0.0;
            end_[0] = tw.extents.end[0];
            end_[1] = tw.extents.end[1];
            end_[2] = tw.extents.end[2];
            end_[3] = results->fraction;
            CM_TraceThroughTree(&tw, 0, start_, end_, results);
        }
    }
    if (!results->walkable && !results->startsolid)
        results->walkable = results->normal[2] >= 0.699999988079071;
    if (oldFrac > (double)results->fraction)
    {
        if (results->surfaceFlags == -1)
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1481, 0, "%s", "results->surfaceFlags != SURF_INVALID");
    }
    else
    {
        if (results->surfaceFlags != -1 && oldFrac != results->fraction)
            MyAssertHandler(
                ".\\qcommon\\cm_trace.cpp",
                1476,
                0,
                "%s",
                "results->surfaceFlags == SURF_INVALID || results->fraction == oldFrac");
        results->surfaceFlags = oldSurfaceFlags;
    }
}

void __cdecl CM_GetTraceThreadInfo(TraceThreadInfo *threadInfo)
{
    TraceThreadInfo *value; // [esp+0h] [ebp-4h]

    if (!threadInfo)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 54, 0, "%s", "threadInfo");
    value = (TraceThreadInfo *)Sys_GetValue(3);
    if (!value)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 57, 0, "%s", "value");
    ++value->checkcount.global;
    *threadInfo = *value;
    if (!threadInfo->checkcount.partitions)
    {
        if (cm.partitionCount)
            MyAssertHandler(
                ".\\qcommon\\cm_trace.cpp",
                62,
                0,
                "%s",
                "threadInfo->checkcount.partitions || cm.partitionCount == 0");
    }
}

void __cdecl CM_TestInLeaf(traceWork_t *tw, cLeaf_t *leaf, trace_t *trace)
{
    if (((tw->contents & leaf->brushContents) == 0 || !CM_TestInLeafBrushNode(tw, leaf, trace))
        && (tw->contents & leaf->terrainContents) != 0)
    {
        CM_MeshTestInLeaf(tw, leaf, trace);
    }
}

bool __cdecl CM_TestInLeafBrushNode(traceWork_t *tw, cLeaf_t *leaf, trace_t *trace)
{
    int i; // [esp+0h] [ebp-4h]

    if (!leaf->leafBrushNode)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 351, 0, "%s", "leaf->leafBrushNode");
    for (i = 0; i < 3; ++i)
    {
        if (leaf->mins[i] >= (double)tw->bounds[1][i])
            return 0;
        if (leaf->maxs[i] <= (double)tw->bounds[0][i])
            return 0;
    }
    CM_TestInLeafBrushNode_r(tw, &cm.leafbrushNodes[leaf->leafBrushNode], trace);
    return trace->allsolid;
}

void __cdecl CM_TestInLeafBrushNode_r(const traceWork_t *tw, cLeafBrushNode_s *node, trace_t *trace)
{
    int k; // [esp+0h] [ebp-10h]
    cbrush_t *b; // [esp+4h] [ebp-Ch]
    unsigned __int16 *brushes; // [esp+8h] [ebp-8h]

    if (!node)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 293, 0, "%s", "node");
    while ((tw->contents & node->contents) != 0)
    {
        if (node->leafBrushCount)
        {
            if (node->leafBrushCount > 0)
            {
                brushes = node->data.leaf.brushes;
                for (k = 0; k < node->leafBrushCount; ++k)
                {
                    b = &cm.brushes[brushes[k]];
                    if ((tw->contents & b->contents) != 0)
                    {
                        CM_TestBoxInBrush(tw, b, trace);
                        if (trace->allsolid)
                            break;
                    }
                }
                return;
            }
            CM_TestInLeafBrushNode_r(tw, node + 1, trace);
            if (trace->allsolid)
                return;
        }
        if (node->data.children.dist >= (double)tw->bounds[0][node->axis])
        {
            if (node->data.children.dist <= (double)tw->bounds[1][node->axis])
            {
                CM_TestInLeafBrushNode_r(tw, &node[node->data.children.childOffset[0]], trace);
                if (trace->allsolid)
                    return;
            }
            node += node->data.children.childOffset[1];
        }
        else
        {
            node += node->data.children.childOffset[0];
        }
    }
}

void __cdecl CM_TestBoxInBrush(const traceWork_t *tw, cbrush_t *brush, trace_t *trace)
{
    float v3; // [esp+0h] [ebp-58h]
    float v4; // [esp+Ch] [ebp-4Ch]
    float d1; // [esp+40h] [ebp-18h]
    cbrushside_t *side; // [esp+44h] [ebp-14h]
    cplane_s *plane; // [esp+48h] [ebp-10h]
    float dist; // [esp+4Ch] [ebp-Ch]
    signed int i; // [esp+54h] [ebp-4h]

    if ((COERCE_UNSIGNED_INT(tw->extents.start[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tw->extents.start[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tw->extents.start[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\qcommon\\cm_trace.cpp",
            224,
            0,
            "%s",
            "!IS_NAN((tw->extents.start)[0]) && !IS_NAN((tw->extents.start)[1]) && !IS_NAN((tw->extents.start)[2])");
    }
    if ((COERCE_UNSIGNED_INT(tw->extents.end[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tw->extents.end[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tw->extents.end[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\qcommon\\cm_trace.cpp",
            225,
            0,
            "%s",
            "!IS_NAN((tw->extents.end)[0]) && !IS_NAN((tw->extents.end)[1]) && !IS_NAN((tw->extents.end)[2])");
    }
    if (brush->maxs[0] > (double)tw->bounds[0][0]
        && brush->maxs[1] > (double)tw->bounds[0][1]
        && brush->maxs[2] > (double)tw->bounds[0][2]
        && brush->mins[0] < (double)tw->bounds[1][0]
        && brush->mins[1] < (double)tw->bounds[1][1]
        && brush->mins[2] < (double)tw->bounds[1][2])
    {
        side = brush->sides;
        i = brush->numsides;
        if (i < 0)
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 246, 0, "%s", "i >= 0");
        while (i)
        {
            plane = side->plane;
            if ((COERCE_UNSIGNED_INT(side->plane->dist) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 251, 0, "%s", "!IS_NAN(plane->dist)");
            if ((COERCE_UNSIGNED_INT(tw->radius) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 252, 0, "%s", "!IS_NAN(tw->radius)");
            if ((COERCE_UNSIGNED_INT(plane->normal[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(plane->normal[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(plane->normal[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\qcommon\\cm_trace.cpp",
                    255,
                    0,
                    "%s",
                    "!IS_NAN((plane->normal)[0]) && !IS_NAN((plane->normal)[1]) && !IS_NAN((plane->normal)[2])");
            }
            if ((COERCE_UNSIGNED_INT(tw->offsetZ) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 256, 0, "%s", "!IS_NAN(tw->offsetZ)");
            v4 = tw->offsetZ * plane->normal[2];
            v3 = I_fabs(v4);
            dist = plane->dist + tw->radius + v3;
            if ((LODWORD(dist) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 259, 0, "%s", "!IS_NAN(dist)");
            d1 = Vec3Dot(tw->extents.start, plane->normal) - dist;
            if ((LODWORD(d1) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 263, 0, "%s", "!IS_NAN(d1)");
            if (d1 > 0.0)
                return;
            --i;
            ++side;
        }
        trace->startsolid = 1;
        trace->allsolid = 1;
        trace->fraction = 0.0;
        trace->contents = brush->contents;
        trace->surfaceFlags = 0;
    }
}

void __cdecl CM_TestCapsuleInCapsule(const traceWork_t *tw, trace_t *trace)
{
    float v2; // [esp+0h] [ebp-88h]
    float v3; // [esp+4h] [ebp-84h]
    float r; // [esp+Ch] [ebp-7Ch]
    float top[3]; // [esp+10h] [ebp-78h] BYREF
    float offs; // [esp+1Ch] [ebp-6Ch]
    float halfheight; // [esp+20h] [ebp-68h]
    float p1[3]; // [esp+24h] [ebp-64h] BYREF
    float symetricSize[2][3]; // [esp+30h] [ebp-58h]
    float radius; // [esp+48h] [ebp-40h]
    float fTotalHalfHeight; // [esp+4Ch] [ebp-3Ch]
    float offset[3]; // [esp+50h] [ebp-38h]
    float p2[3]; // [esp+5Ch] [ebp-2Ch] BYREF
    float tmp[3]; // [esp+68h] [ebp-20h] BYREF
    int i; // [esp+74h] [ebp-14h]
    float bottom[3]; // [esp+78h] [ebp-10h] BYREF
    float fHeightDiff; // [esp+84h] [ebp-4h]

    top[0] = tw->extents.start[0];
    top[1] = tw->extents.start[1];
    top[2] = tw->extents.start[2] + tw->offsetZ;
    bottom[0] = tw->extents.start[0];
    bottom[1] = tw->extents.start[1];
    bottom[2] = tw->extents.start[2] - tw->offsetZ;
    for (i = 0; i < 3; ++i)
    {
        offset[i] = (tw->threadInfo.box_model->mins[i] + tw->threadInfo.box_model->maxs[i]) * 0.5;
        symetricSize[0][i] = tw->threadInfo.box_model->mins[i] - offset[i];
        symetricSize[1][i] = tw->threadInfo.box_model->maxs[i] - offset[i];
    }
    halfheight = symetricSize[1][2];
    if (symetricSize[1][2] >= symetricSize[1][0])
        v3 = symetricSize[1][0];
    else
        v3 = halfheight;
    radius = v3;
    offs = halfheight - v3;
    r = (tw->radius + v3) * (tw->radius + v3);
    p1[0] = offset[0];
    p1[1] = offset[1];
    p1[2] = offset[2] + offs;
    Vec3Sub(p1, top, tmp);
    if (r > Vec3LengthSq(tmp))
        goto LABEL_8;
    Vec3Sub(p1, bottom, tmp);
    if (r > Vec3LengthSq(tmp))
        goto LABEL_17;
    p2[0] = offset[0];
    p2[1] = offset[1];
    p2[2] = offset[2] - offs;
    Vec3Sub(p2, top, tmp);
    if (r > Vec3LengthSq(tmp))
    {
        trace->startsolid = 1;
        trace->allsolid = 1;
        trace->fraction = 0.0;
        trace->surfaceFlags = 0;
        return;
    }
    Vec3Sub(p2, bottom, tmp);
    if (r > Vec3LengthSq(tmp))
    {
    LABEL_8:
        trace->startsolid = 1;
        trace->allsolid = 1;
        trace->fraction = 0.0;
        trace->surfaceFlags = 0;
        return;
    }
    fHeightDiff = tw->extents.start[2] - offset[2];
    fTotalHalfHeight = offs + tw->size[2] - tw->radius;
    if (fTotalHalfHeight < 0.0)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 462, 0, "%s", "fTotalHalfHeight >= 0");
    v2 = I_fabs(fHeightDiff);
    if (fTotalHalfHeight >= v2)
    {
        p1[2] = 0.0;
        top[2] = 0.0;
        Vec3Sub(top, p1, tmp);
        if (r > Vec3LengthSq(tmp))
        {
        LABEL_17:
            trace->startsolid = 1;
            trace->allsolid = 1;
            trace->fraction = 0.0;
            trace->surfaceFlags = 0;
        }
    }
}

void __cdecl CM_PositionTest(traceWork_t *tw, trace_t *trace)
{
    leafList_s ll; // [esp+0h] [ebp-834h] BYREF
    unsigned __int16 leafs[1024]; // [esp+2Ch] [ebp-808h] BYREF
    int i; // [esp+830h] [ebp-4h]

    if (!trace->allsolid)
    {
        Vec3Sub(tw->extents.start, tw->size, ll.bounds[0]);
        Vec3Add(tw->extents.start, tw->size, ll.bounds[1]);
        for (i = 0; i < 3; ++i)
        {
            ll.bounds[0][i] = ll.bounds[0][i] - 1.0;
            ll.bounds[1][i] = ll.bounds[1][i] + 1.0;
        }
        ll.count = 0;
        ll.maxcount = 1024;
        ll.list = leafs;
        ll.lastLeaf = 0;
        ll.overflowed = 0;
        CM_BoxLeafnums_r(&ll, 0);
        if (ll.count)
        {
            for (i = 0; i < ll.count && !trace->allsolid; ++i)
                CM_TestInLeaf(tw, &cm.leafs[leafs[i]], trace);
        }
    }
}

void __cdecl CM_TraceThroughLeaf(const traceWork_t *tw, cLeaf_t *leaf, trace_t *trace)
{
    int k; // [esp+60h] [ebp-4h]

    if (trace->fraction != 0.0)
    {
        if ((tw->contents & leaf->brushContents) != 0)
        {
            PROF_SCOPED("CM_TraceBrush"); 
            if (CM_TraceThroughLeafBrushNode(tw, leaf, trace))
            {
                return;
            }
        }

        if ((tw->contents & leaf->terrainContents) == 0)
            return;

        PROF_SCOPED("CM_TraceTerrain");
        for (k = 0; k < leaf->collAabbCount && trace->fraction != 0.0; ++k)
            CM_TraceThroughAabbTree(tw, &cm.aabbTrees[k + leaf->firstCollAabbIndex], trace);
        return;
    }
}

bool __cdecl CM_TraceThroughLeafBrushNode(const traceWork_t *tw, cLeaf_t *leaf, trace_t *trace)
{
    __int64 _FFFFFFFC; // [esp-4h] [ebp-48h]
    float absmin[3]; // [esp+Ch] [ebp-38h] BYREF
    float start[4]; // [esp+18h] [ebp-2Ch] BYREF
    float end[4]; // [esp+28h] [ebp-1Ch] BYREF
    float absmax[3]; // [esp+38h] [ebp-Ch] BYREF

    if (!leaf->leafBrushNode)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 860, 0, "%s", "leaf->leafBrushNode");
    Vec3Sub(leaf->mins, tw->size, absmin);
    Vec3Add(leaf->maxs, tw->size, absmax);
    if (CM_TraceBox(&tw->extents, absmin, absmax, trace->fraction))
        return 0;
    start[0] = tw->extents.start[0];
    start[1] = tw->extents.start[1];
    start[2] = tw->extents.start[2];
    end[0] = tw->extents.end[0];
    end[1] = tw->extents.end[1];
    end[2] = tw->extents.end[2];
    start[3] = 0.0;
    end[3] = trace->fraction;
    CM_TraceThroughLeafBrushNode_r(tw, &cm.leafbrushNodes[leaf->leafBrushNode], start, end, trace);
    return trace->fraction == 0.0;
}

void __cdecl CM_TraceThroughLeafBrushNode_r(
    const traceWork_t *tw,
    cLeafBrushNode_s *node,
    float *p1_,
    const float *p2,
    trace_t *trace)
{
    float v5; // [esp+10h] [ebp-8Ch]
    float v6; // [esp+14h] [ebp-88h]
    float v7; // [esp+18h] [ebp-84h]
    float v8; // [esp+1Ch] [ebp-80h]
    float v9; // [esp+24h] [ebp-78h]
    float v10; // [esp+28h] [ebp-74h]
    float v11; // [esp+2Ch] [ebp-70h]
    float v12; // [esp+30h] [ebp-6Ch]
    float v13; // [esp+34h] [ebp-68h]
    float v14; // [esp+38h] [ebp-64h]
    BOOL side; // [esp+3Ch] [ebp-60h]
    float diff; // [esp+40h] [ebp-5Ch]
    float t1; // [esp+48h] [ebp-54h]
    float frac; // [esp+4Ch] [ebp-50h]
    int k; // [esp+50h] [ebp-4Ch]
    float p1[4]; // [esp+58h] [ebp-44h] BYREF
    float offset; // [esp+68h] [ebp-34h]
    float tmax; // [esp+6Ch] [ebp-30h]
    float t2; // [esp+70h] [ebp-2Ch]
    float frac2; // [esp+74h] [ebp-28h]
    unsigned __int16 *brushes; // [esp+78h] [ebp-24h]
    float absDiff; // [esp+7Ch] [ebp-20h]
    float invDist; // [esp+80h] [ebp-1Ch]
    float tmin; // [esp+84h] [ebp-18h]
    int brushnum; // [esp+88h] [ebp-14h]
    float mid[4]; // [esp+8Ch] [ebp-10h] BYREF

    if (!node)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 753, 0, "%s", "node");
    p1[0] = *p1_;
    p1[1] = p1_[1];
    p1[2] = p1_[2];
    p1[3] = p1_[3];
    while ((tw->contents & node->contents) != 0)
    {
        if (node->leafBrushCount)
        {
            if (node->leafBrushCount > 0)
            {
                brushes = node->data.leaf.brushes;
                for (k = 0; k < node->leafBrushCount; ++k)
                {
                    brushnum = brushes[k];
                    if ((tw->contents & cm.brushes[brushnum].contents) != 0)
                        CM_TraceThroughBrush(tw, &cm.brushes[brushnum], trace);
                }
                return;
            }
            CM_TraceThroughLeafBrushNode_r(tw, node + 1, p1, p2, trace);
        }
        t1 = p1[node->axis] - node->data.children.dist;
        t2 = p2[node->axis] - node->data.children.dist;
        offset = tw->size[node->axis] + 0.125 - node->data.children.range;
        v14 = t1 - t2;
        if (v14 < 0.0)
            v13 = t2;
        else
            v13 = t1;
        tmax = v13;
        v12 = t2 - t1;
        if (v12 < 0.0)
            v11 = t2;
        else
            v11 = t1;
        tmin = v11;
        if (offset > v11)
        {
            if (tmax > -offset)
            {
                if (p1[3] >= trace->fraction)
                    return;
                diff = t2 - t1;
                v10 = I_fabs(diff);
                absDiff = v10;
                if (v10 <= 0.000000476837158203125)
                {
                    side = 0;
                    frac = 1.0;
                    frac2 = 0.0;
                }
                else
                {
                    if (diff < 0.0)
                        v9 = t1;
                    else
                        v9 = -t1;
                    invDist = 1.0 / absDiff;
                    frac2 = (v9 - offset) * invDist;
                    frac = (v9 + offset) * invDist;
                    side = diff >= 0.0;
                }
                if (frac < 0.0)
                    MyAssertHandler(".\\qcommon\\cm_trace.cpp", 825, 0, "frac >= 0.0f\n\t%g, %g", frac, 0.0);
                v8 = 1.0 - frac;
                if (v8 < 0.0)
                    v7 = 1.0;
                else
                    v7 = frac;
                mid[0] = (*p2 - p1[0]) * v7 + p1[0];
                mid[1] = (p2[1] - p1[1]) * v7 + p1[1];
                mid[2] = (p2[2] - p1[2]) * v7 + p1[2];
                mid[3] = (p2[3] - p1[3]) * v7 + p1[3];
                CM_TraceThroughLeafBrushNode_r(tw, &node[node->data.children.childOffset[side]], p1, mid, trace);
                if (frac2 > 1.000000476837158)
                    MyAssertHandler(
                        ".\\qcommon\\cm_trace.cpp",
                        835,
                        0,
                        "frac2 <= 1.0f + TRACE_EPSILON\n\t%g, %g",
                        frac2,
                        1.000000476837158);
                v6 = frac2 - 0.0;
                if (v6 < 0.0)
                    v5 = 0.0;
                else
                    v5 = frac2;
                frac2 = v5;
                p1[0] = (*p2 - p1[0]) * v5 + p1[0];
                p1[1] = (p2[1] - p1[1]) * v5 + p1[1];
                p1[2] = (p2[2] - p1[2]) * v5 + p1[2];
                p1[3] = (p2[3] - p1[3]) * v5 + p1[3];
                node += node->data.children.childOffset[1 - side];
            }
            else
            {
                node += node->data.children.childOffset[1];
            }
        }
        else
        {
            if (tmax <= -offset)
                return;
            node += node->data.children.childOffset[0];
        }
    }
}

void __cdecl CM_TraceThroughBrush(const traceWork_t *tw, cbrush_t *brush, trace_t *trace)
{
    float v3; // [esp+8h] [ebp-100h]
    float v4; // [esp+Ch] [ebp-FCh]
    float v5; // [esp+10h] [ebp-F8h]
    float v6; // [esp+14h] [ebp-F4h]
    float v7; // [esp+18h] [ebp-F0h]
    float v8; // [esp+1Ch] [ebp-ECh]
    float v9; // [esp+20h] [ebp-E8h]
    float *normal; // [esp+28h] [ebp-E0h]
    float v11; // [esp+3Ch] [ebp-CCh]
    float d1; // [esp+9Ch] [ebp-6Ch]
    float d1a; // [esp+9Ch] [ebp-6Ch]
    cbrushside_t *side; // [esp+A0h] [ebp-68h]
    int j; // [esp+A4h] [ebp-64h]
    cplane_s *plane; // [esp+A8h] [ebp-60h]
    float enterFrac; // [esp+ACh] [ebp-5Ch]
    float delta; // [esp+B0h] [ebp-58h]
    float deltaa; // [esp+B0h] [ebp-58h]
    bool allsolid; // [esp+B7h] [ebp-51h]
    float frac; // [esp+B8h] [ebp-50h]
    float fraca; // [esp+B8h] [ebp-50h]
    float dist; // [esp+BCh] [ebp-4Ch]
    cbrushside_t *leadside; // [esp+C0h] [ebp-48h]
    cbrushside_t axialSide; // [esp+C4h] [ebp-44h] BYREF
    cplane_s axialPlane; // [esp+D0h] [ebp-38h] BYREF
    float sign; // [esp+E8h] [ebp-20h]
    float d2; // [esp+ECh] [ebp-1Ch]
    const float *bounds; // [esp+F0h] [ebp-18h]
    float f; // [esp+F4h] [ebp-14h]
    int index; // [esp+F8h] [ebp-10h]
    float leaveFrac; // [esp+FCh] [ebp-Ch]
    float offsetDotNormal; // [esp+100h] [ebp-8h]
    int i; // [esp+104h] [ebp-4h]

    if ((COERCE_UNSIGNED_INT(tw->extents.start[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tw->extents.start[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tw->extents.start[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\qcommon\\cm_trace.cpp",
            563,
            0,
            "%s",
            "!IS_NAN((tw->extents.start)[0]) && !IS_NAN((tw->extents.start)[1]) && !IS_NAN((tw->extents.start)[2])");
    }
    if ((COERCE_UNSIGNED_INT(tw->extents.end[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tw->extents.end[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tw->extents.end[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\qcommon\\cm_trace.cpp",
            564,
            0,
            "%s",
            "!IS_NAN((tw->extents.end)[0]) && !IS_NAN((tw->extents.end)[1]) && !IS_NAN((tw->extents.end)[2])");
    }
    if ((COERCE_UNSIGNED_INT(tw->extents.invDelta[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tw->extents.invDelta[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tw->extents.invDelta[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\qcommon\\cm_trace.cpp",
            565,
            0,
            "%s",
            "!IS_NAN((tw->extents.invDelta)[0]) && !IS_NAN((tw->extents.invDelta)[1]) && !IS_NAN((tw->extents.invDelta)[2])");
    }
    enterFrac = 0.0;
    leaveFrac = trace->fraction;
    allsolid = 1;
    leadside = 0;
    sign = -1.0;
    bounds = (const float *)brush;
    index = 0;
    while (2)
    {
        if ((COERCE_UNSIGNED_INT(*bounds) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(bounds[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(bounds[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\qcommon\\cm_trace.cpp",
                586,
                0,
                "%s",
                "!IS_NAN((bounds)[0]) && !IS_NAN((bounds)[1]) && !IS_NAN((bounds)[2])");
        }
        if ((COERCE_UNSIGNED_INT(tw->radiusOffset[0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(tw->radiusOffset[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(tw->radiusOffset[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\qcommon\\cm_trace.cpp",
                587,
                0,
                "%s",
                "!IS_NAN((tw->radiusOffset)[0]) && !IS_NAN((tw->radiusOffset)[1]) && !IS_NAN((tw->radiusOffset)[2])");
        }
        for (j = 0; j < 3; ++j)
        {
            d1 = (tw->extents.start[j] - bounds[j]) * sign - tw->radiusOffset[j];
            d2 = (tw->extents.end[j] - bounds[j]) * sign - tw->radiusOffset[j];
            if ((LODWORD(d1) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 593, 0, "%s", "!IS_NAN(d1)");
            if ((LODWORD(d2) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 594, 0, "%s", "!IS_NAN(d2)");
            if (d1 <= 0.0)
            {
                if (d2 > 0.0)
                {
                    fraca = d1 * tw->extents.invDelta[j] * sign;
                    if (enterFrac >= (double)fraca)
                        return;
                    allsolid = 0;
                    v7 = fraca - leaveFrac;
                    if (v7 < 0.0)
                        v6 = d1 * tw->extents.invDelta[j] * sign;
                    else
                        v6 = leaveFrac;
                    leaveFrac = v6;
                }
            }
            else
            {
                v9 = 0.125 - d1;
                if (v9 < 0.0)
                    v8 = 0.125;
                else
                    v8 = d1;
                if (v8 <= (double)d2)
                    return;
                frac = (d1 - 0.125) * tw->extents.invDelta[j] * sign;
                if (leaveFrac <= (double)frac)
                    return;
                if (d2 > 0.0)
                    allsolid = 0;
                if (enterFrac >= (double)frac)
                {
                    if (leadside)
                        continue;
                }
                else
                {
                    enterFrac = (d1 - 0.125) * tw->extents.invDelta[j] * sign;
                }
                axialSide.materialNum = brush->axialMaterialNum[index][j];
                axialPlane.normal[0] = 0.0;
                axialPlane.normal[1] = 0.0;
                axialPlane.normal[2] = 0.0;
                axialPlane.normal[j] = sign;
                axialSide.plane = &axialPlane;
                leadside = &axialSide;
            }
        }
        if (!index)
        {
            sign = 1.0;
            bounds = brush->maxs;
            index = 1;
            continue;
        }
        break;
    }
    side = brush->sides;
    i = brush->numsides;
    if (i < 0)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 638, 0, "%s", "i >= 0");
    while (2)
    {
        if (i)
        {
            plane = side->plane;
            if ((COERCE_UNSIGNED_INT(side->plane->dist) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 643, 0, "%s", "!IS_NAN(plane->dist)");
            if ((COERCE_UNSIGNED_INT(tw->radius) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 644, 0, "%s", "!IS_NAN(tw->radius)");
            if ((COERCE_UNSIGNED_INT(plane->normal[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(plane->normal[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(plane->normal[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\qcommon\\cm_trace.cpp",
                    647,
                    0,
                    "%s",
                    "!IS_NAN((plane->normal)[0]) && !IS_NAN((plane->normal)[1]) && !IS_NAN((plane->normal)[2])");
            }
            if ((COERCE_UNSIGNED_INT(tw->offsetZ) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 648, 0, "%s", "!IS_NAN(tw->offsetZ)");
            v11 = tw->offsetZ * plane->normal[2];
            v5 = I_fabs(v11);
            offsetDotNormal = v5;
            dist = plane->dist + tw->radius + v5;
            if ((LODWORD(dist) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 651, 0, "%s", "!IS_NAN(dist)");
            d1a = Vec3Dot(tw->extents.start, plane->normal) - dist;
            d2 = Vec3Dot(tw->extents.end, plane->normal) - dist;
            if ((LODWORD(d1a) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 656, 0, "%s", "!IS_NAN(d1)");
            if ((LODWORD(d2) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 657, 0, "%s", "!IS_NAN(d2)");
            if (d1a <= 0.0)
            {
                if (d2 > 0.0)
                {
                    deltaa = d1a - d2;
                    if (deltaa >= 0.0)
                        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 687, 0, "%s", "delta < 0");
                    if (d1a > leaveFrac * deltaa)
                    {
                        leaveFrac = d1a / deltaa;
                        if (leaveFrac <= (double)enterFrac)
                            return;
                    }
                    allsolid = 0;
                }
            }
            else
            {
                v4 = 0.125 - d1a;
                if (v4 < 0.0)
                    v3 = 0.125;
                else
                    v3 = d1a;
                if (v3 <= (double)d2)
                    return;
                if (d2 > 0.0)
                    allsolid = 0;
                delta = d1a - d2;
                if ((LODWORD(delta) & 0x7F800000) == 0x7F800000)
                    MyAssertHandler(".\\qcommon\\cm_trace.cpp", 668, 0, "%s", "!IS_NAN(delta)");
                if (delta <= 0.0)
                    MyAssertHandler(".\\qcommon\\cm_trace.cpp", 670, 0, "%s", "delta > 0");
                f = d1a - 0.125;
                if (f <= enterFrac * delta)
                {
                    if (!leadside)
                        goto LABEL_86;
                }
                else
                {
                    enterFrac = f / delta;
                    if (leaveFrac <= (double)enterFrac)
                        return;
                LABEL_86:
                    leadside = side;
                }
            }
            --i;
            ++side;
            continue;
        }
        break;
    }
    trace->contents = brush->contents;
    if (leadside)
    {
        trace->fraction = enterFrac;
        if (trace->fraction < 0.0 || trace->fraction > 1.0)
            MyAssertHandler(
                ".\\qcommon\\cm_trace.cpp",
                717,
                1,
                "%s\n\t(trace->fraction) = %g",
                "(trace->fraction >= 0 && trace->fraction <= 1.0f)",
                trace->fraction);
        normal = leadside->plane->normal;
        trace->normal[0] = leadside->plane->normal[0];
        trace->normal[1] = normal[1];
        trace->normal[2] = normal[2];
        trace->surfaceFlags = cm.materials[leadside->materialNum].surfaceFlags;
        trace->material = cm.materials[leadside->materialNum].material;
        trace->walkable = 0;
    }
    else
    {
        trace->startsolid = 1;
        if (allsolid)
        {
            trace->allsolid = 1;
            trace->fraction = 0.0;
            trace->surfaceFlags = 0;
        }
    }
}

void __cdecl CM_TraceCapsuleThroughCapsule(const traceWork_t *tw, trace_t *trace)
{
    float v2; // [esp+Ch] [ebp-A4h]
    float v3; // [esp+10h] [ebp-A0h]
    float v4; // [esp+14h] [ebp-9Ch]
    float v5; // [esp+18h] [ebp-98h]
    float v6; // [esp+1Ch] [ebp-94h]
    float v7; // [esp+20h] [ebp-90h]
    float v8; // [esp+24h] [ebp-8Ch]
    float endtop[3]; // [esp+30h] [ebp-80h] BYREF
    float starttop[3]; // [esp+3Ch] [ebp-74h] BYREF
    float halfwidth; // [esp+48h] [ebp-68h]
    float top[3]; // [esp+4Ch] [ebp-64h] BYREF
    float offs; // [esp+58h] [ebp-58h]
    float halfheight; // [esp+5Ch] [ebp-54h]
    float symetricSize[2][3]; // [esp+60h] [ebp-50h]
    float radius; // [esp+78h] [ebp-38h]
    float offset[3]; // [esp+7Ch] [ebp-34h] BYREF
    float endbottom[3]; // [esp+88h] [ebp-28h] BYREF
    float startbottom[3]; // [esp+94h] [ebp-1Ch] BYREF
    int i; // [esp+A0h] [ebp-10h]
    float bottom[3]; // [esp+A4h] [ebp-Ch] BYREF

    v8 = tw->threadInfo.box_model->maxs[0] + 1.0;
    if (tw->bounds[0][0] <= (double)v8)
    {
        v7 = tw->threadInfo.box_model->maxs[1] + 1.0;
        if (tw->bounds[0][1] <= (double)v7)
        {
            v6 = tw->threadInfo.box_model->maxs[2] + 1.0;
            if (tw->bounds[0][2] <= (double)v6)
            {
                v5 = tw->threadInfo.box_model->mins[0] - 1.0;
                if (tw->bounds[1][0] >= (double)v5)
                {
                    v4 = tw->threadInfo.box_model->mins[1] - 1.0;
                    if (tw->bounds[1][1] >= (double)v4)
                    {
                        v3 = tw->threadInfo.box_model->mins[2] - 1.0;
                        if (tw->bounds[1][2] >= (double)v3)
                        {
                            starttop[0] = tw->extents.start[0];
                            starttop[1] = tw->extents.start[1];
                            starttop[2] = tw->extents.start[2] + tw->offsetZ;
                            startbottom[0] = tw->extents.start[0];
                            startbottom[1] = tw->extents.start[1];
                            startbottom[2] = tw->extents.start[2] - tw->offsetZ;
                            endtop[0] = tw->extents.end[0];
                            endtop[1] = tw->extents.end[1];
                            endtop[2] = tw->extents.end[2] + tw->offsetZ;
                            endbottom[0] = tw->extents.end[0];
                            endbottom[1] = tw->extents.end[1];
                            endbottom[2] = tw->extents.end[2] - tw->offsetZ;
                            for (i = 0; i < 3; ++i)
                            {
                                offset[i] = (tw->threadInfo.box_model->mins[i] + tw->threadInfo.box_model->maxs[i]) * 0.5;
                                symetricSize[0][i] = tw->threadInfo.box_model->mins[i] - offset[i];
                                symetricSize[1][i] = tw->threadInfo.box_model->maxs[i] - offset[i];
                            }
                            halfwidth = symetricSize[1][0];
                            halfheight = symetricSize[1][2];
                            if (symetricSize[1][2] >= (double)symetricSize[1][0])
                                v2 = halfwidth;
                            else
                                v2 = halfheight;
                            radius = v2;
                            offs = halfheight - v2;
                            top[0] = offset[0];
                            top[1] = offset[1];
                            top[2] = offset[2] + offs;
                            bottom[0] = offset[0];
                            bottom[1] = offset[1];
                            bottom[2] = offset[2] - offs;
                            if (top[2] >= (double)startbottom[2])
                            {
                                if (bottom[2] > (double)starttop[2]
                                    && (!CM_TraceSphereThroughSphere(tw, starttop, endtop, bottom, radius, trace) || tw->delta[2] <= 0.0))
                                {
                                    return;
                                }
                            }
                            else if (!CM_TraceSphereThroughSphere(tw, startbottom, endbottom, top, radius, trace)
                                || tw->delta[2] >= 0.0)
                            {
                                return;
                            }
                            if (CM_TraceCylinderThroughCylinder(tw, offset, offs, radius, trace))
                            {
                                if (top[2] >= (double)endbottom[2])
                                {
                                    if (bottom[2] > (double)endtop[2] && bottom[2] <= (double)starttop[2])
                                        CM_TraceSphereThroughSphere(tw, starttop, endtop, bottom, radius, trace);
                                }
                                else if (top[2] >= (double)startbottom[2])
                                {
                                    CM_TraceSphereThroughSphere(tw, startbottom, endbottom, top, radius, trace);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

int __cdecl CM_TraceSphereThroughSphere(
    const traceWork_t *tw,
    const float *vStart,
    const float *vEnd,
    const float *vStationary,
    float radius,
    trace_t *trace)
{
    float v7; // [esp+8h] [ebp-4Ch]
    float v8; // [esp+Ch] [ebp-48h]
    float v9; // [esp+10h] [ebp-44h]
    float fDiscriminant; // [esp+20h] [ebp-34h]
    float fEntry; // [esp+24h] [ebp-30h]
    float fRadiusSqrd; // [esp+28h] [ebp-2Ch]
    float fC; // [esp+2Ch] [ebp-28h]
    float fB; // [esp+30h] [ebp-24h]
    float fA; // [esp+34h] [ebp-20h]
    float vNormal[3]; // [esp+38h] [ebp-1Ch] BYREF
    float vDelta[3]; // [esp+44h] [ebp-10h] BYREF
    float fDeltaLen; // [esp+50h] [ebp-4h]

    Vec3Sub(vStart, vStationary, vDelta);
    fRadiusSqrd = (radius + tw->radius) * (radius + tw->radius);
    fC = Vec3Dot(vDelta, vDelta) - fRadiusSqrd;
    if (fC > 0.0)
    {
        fB = Vec3Dot(tw->delta, vDelta);
        if (fB < 0.0)
        {
            fA = tw->deltaLenSq;
            if (fA <= 0.0)
                MyAssertHandler(
                    ".\\qcommon\\cm_trace.cpp",
                    959,
                    0,
                    "%s\n\t(tw->deltaLenSq) = %g",
                    "(fA > 0.0f)",
                    tw->deltaLenSq);
            fDiscriminant = fB * fB - fA * fC;
            if (fDiscriminant >= 0.0)
            {
                fDeltaLen = Vec3NormalizeTo(vDelta, vNormal);
                v9 = sqrt(fDiscriminant);
                fEntry = (-fB - v9) / fA + fDeltaLen * 0.125 / fB;
                if (trace->fraction <= (double)fEntry)
                {
                    return 1;
                }
                else
                {
                    v8 = fEntry - 0.0;
                    if (v8 < 0.0)
                        v7 = 0.0;
                    else
                        v7 = (-fB - v9) / fA + fDeltaLen * 0.125 / fB;
                    trace->fraction = v7;
                    if (trace->fraction < 0.0 || trace->fraction > 1.0)
                        MyAssertHandler(
                            ".\\qcommon\\cm_trace.cpp",
                            972,
                            1,
                            "%s\n\t(trace->fraction) = %g",
                            "(trace->fraction >= 0 && trace->fraction <= 1.0f)",
                            trace->fraction);
                    trace->normal[0] = vNormal[0];
                    trace->normal[1] = vNormal[1];
                    trace->normal[2] = vNormal[2];
                    trace->contents = tw->threadInfo.box_brush->contents;
                    trace->walkable = 0;
                    trace->surfaceFlags = 0;
                    return 0;
                }
            }
            else
            {
                return 1;
            }
        }
        else
        {
            return 1;
        }
    }
    else
    {
        trace->fraction = 0.0;
        trace->startsolid = 1;
        trace->walkable = 0;
        Vec3NormalizeTo(vDelta, trace->normal);
        trace->contents = tw->threadInfo.box_brush->contents;
        trace->surfaceFlags = 0;
        Vec3Sub(vEnd, vStationary, vDelta);
        if (fRadiusSqrd >= Vec3LengthSq(vDelta))
            trace->allsolid = 1;
        return 0;
    }
}

int __cdecl CM_TraceCylinderThroughCylinder(
    const traceWork_t *tw,
    const float *vStationary,
    float fStationaryHalfHeight,
    float radius,
    trace_t *trace)
{
    float v6; // [esp+8h] [ebp-7Ch]
    float v7; // [esp+Ch] [ebp-78h]
    float v8; // [esp+10h] [ebp-74h]
    float v9; // [esp+14h] [ebp-70h]
    float v10; // [esp+20h] [ebp-64h]
    float v11; // [esp+24h] [ebp-60h]
    float v12; // [esp+28h] [ebp-5Ch]
    float fDiscriminant; // [esp+44h] [ebp-40h]
    float fEntry; // [esp+48h] [ebp-3Ch]
    float fRadiusSqrd; // [esp+4Ch] [ebp-38h]
    float fHitHeight; // [esp+50h] [ebp-34h]
    float fEpsilon; // [esp+54h] [ebp-30h]
    float fTotalHeight; // [esp+58h] [ebp-2Ch]
    float fTotalHeighta; // [esp+58h] [ebp-2Ch]
    float fC; // [esp+5Ch] [ebp-28h]
    float fB; // [esp+60h] [ebp-24h]
    float fA; // [esp+64h] [ebp-20h]
    float vNormal[3]; // [esp+68h] [ebp-1Ch] BYREF
    float vDelta[3]; // [esp+74h] [ebp-10h] BYREF
    float fDeltaLen; // [esp+80h] [ebp-4h]

    Vec3Sub(tw->extents.start, vStationary, vDelta);
    fRadiusSqrd = (radius + tw->radius) * (radius + tw->radius);
    v12 = vDelta[1] * vDelta[1] + vDelta[0] * vDelta[0];
    fC = v12 - fRadiusSqrd;
    if (fC > 0.0)
    {
        fB = vDelta[1] * tw->delta[1] + vDelta[0] * tw->delta[0];
        if (fB < 0.0)
        {
            fA = tw->delta[1] * tw->delta[1] + tw->delta[0] * tw->delta[0];
            if (fA <= 0.0)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1036, 0, "%s\n\t(fA) = %g", "(fA > 0.0f)", fA);
            fDiscriminant = fB * fB - fA * fC;
            if (fDiscriminant >= 0.0)
            {
                vDelta[2] = 0.0;
                fDeltaLen = Vec3NormalizeTo(vDelta, vNormal);
                fEpsilon = fDeltaLen * 0.125 / fB;
                v9 = sqrt(fDiscriminant);
                fEntry = (-fB - v9) / fA + fEpsilon;
                if (trace->fraction <= (double)fEntry)
                {
                    return 1;
                }
                else
                {
                    fTotalHeighta = tw->size[2] - tw->radius + fStationaryHalfHeight;
                    fHitHeight = (fEntry - fEpsilon) * tw->delta[2] + tw->extents.start[2] - vStationary[2];
                    if (fTotalHeighta < 0.0)
                        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1051, 0, "%s", "fTotalHeight >= 0");
                    v8 = I_fabs(fHitHeight);
                    if (fTotalHeighta >= (double)v8)
                    {
                        v7 = fEntry - 0.0;
                        if (v7 < 0.0)
                            v6 = 0.0;
                        else
                            v6 = (-fB - v9) / fA + fEpsilon;
                        trace->fraction = v6;
                        if (trace->fraction < 0.0 || trace->fraction > 1.0)
                            MyAssertHandler(
                                ".\\qcommon\\cm_trace.cpp",
                                1056,
                                1,
                                "%s\n\t(trace->fraction) = %g",
                                "(trace->fraction >= 0 && trace->fraction <= 1.0f)",
                                trace->fraction);
                        trace->normal[0] = vNormal[0];
                        trace->normal[1] = vNormal[1];
                        trace->normal[2] = vNormal[2];
                        trace->contents = tw->threadInfo.box_brush->contents;
                        trace->surfaceFlags = 0;
                        trace->walkable = 0;
                        return 0;
                    }
                    else
                    {
                        return 1;
                    }
                }
            }
            else
            {
                return 1;
            }
        }
        else
        {
            return 1;
        }
    }
    else
    {
        fTotalHeight = tw->size[2] - tw->radius + fStationaryHalfHeight;
        if (fTotalHeight < 0.0)
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1011, 0, "%s", "fTotalHeight >= 0");
        v11 = I_fabs(vDelta[2]);
        if (fTotalHeight >= (double)v11)
        {
            trace->fraction = 0.0;
            trace->startsolid = 1;
            trace->walkable = 0;
            vDelta[2] = 0.0;
            Vec3NormalizeTo(vDelta, trace->normal);
            trace->contents = tw->threadInfo.box_brush->contents;
            trace->surfaceFlags = 0;
            Vec3Sub(tw->extents.end, vStationary, vDelta);
            if (fTotalHeight < 0.0)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1024, 0, "%s", "fTotalHeight >= 0");
            v10 = I_fabs(vDelta[2]);
            if (fTotalHeight >= (double)v10)
                trace->allsolid = 1;
            return 0;
        }
        else
        {
            return 1;
        }
    }
}

void __cdecl CM_TraceThroughTree(const traceWork_t *tw, int num, const float *p1_, const float *p2, trace_t *trace)
{
    float v5; // [esp+8h] [ebp-7Ch]
    float v6; // [esp+Ch] [ebp-78h]
    float v7; // [esp+10h] [ebp-74h]
    float v8; // [esp+14h] [ebp-70h]
    float v9; // [esp+1Ch] [ebp-68h]
    float v10; // [esp+20h] [ebp-64h]
    float v11; // [esp+24h] [ebp-60h]
    float v12; // [esp+28h] [ebp-5Ch]
    float v13; // [esp+2Ch] [ebp-58h]
    float v14; // [esp+30h] [ebp-54h]
    cNode_t *node; // [esp+34h] [ebp-50h]
    bool side; // [esp+38h] [ebp-4Ch]
    float diff; // [esp+3Ch] [ebp-48h]
    cplane_s *plane; // [esp+40h] [ebp-44h]
    float t1; // [esp+48h] [ebp-3Ch]
    float frac; // [esp+4Ch] [ebp-38h]
    float p1[4]; // [esp+50h] [ebp-34h] BYREF
    float offset; // [esp+60h] [ebp-24h]
    float t2; // [esp+64h] [ebp-20h]
    float frac2; // [esp+68h] [ebp-1Ch]
    float absDiff; // [esp+6Ch] [ebp-18h]
    float invDist; // [esp+70h] [ebp-14h]
    float mid[4]; // [esp+74h] [ebp-10h] BYREF

    p1[0] = *p1_;
    p1[1] = p1_[1];
    p1[2] = p1_[2];
    p1[3] = p1_[3];
    while (num >= 0)
    {
        node = &cm.nodes[num];
        plane = node->plane;
        if (node->plane->type >= 3u)
        {
            t1 = Vec3Dot(plane->normal, p1) - plane->dist;
            t2 = Vec3Dot(plane->normal, p2) - plane->dist;
            if (tw->isPoint)
                offset = 0.125;
            else
                offset = tw->boundingRadius + 0.125;
        }
        else
        {
            t1 = p1[plane->type] - plane->dist;
            t2 = p2[plane->type] - plane->dist;
            offset = tw->size[plane->type] + 0.125;
        }
        v14 = t2 - t1;
        if (v14 < 0.0)
            v13 = t2;
        else
            v13 = t1;
        if (offset > (double)v13)
        {
            v12 = t1 - t2;
            if (v12 < 0.0)
                v11 = t2;
            else
                v11 = t1;
            if (v11 > -offset)
            {
                if (p1[3] >= (double)trace->fraction)
                    return;
                diff = t2 - t1;
                v10 = I_fabs(diff);
                absDiff = v10;
                if (v10 <= 0.000000476837158203125)
                {
                    side = 0;
                    frac = 1.0;
                    frac2 = 0.0;
                }
                else
                {
                    if (diff < 0.0)
                        v9 = t1;
                    else
                        v9 = -t1;
                    invDist = 1.0 / absDiff;
                    frac2 = (v9 - offset) * invDist;
                    frac = (v9 + offset) * invDist;
                    side = diff >= 0.0;
                }
                if (frac < 0.0)
                    MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1257, 0, "%s", "frac >= 0");
                v8 = 1.0 - frac;
                if (v8 < 0.0)
                    v7 = 1.0;
                else
                    v7 = frac;
                mid[0] = (*p2 - p1[0]) * v7 + p1[0];
                mid[1] = (p2[1] - p1[1]) * v7 + p1[1];
                mid[2] = (p2[2] - p1[2]) * v7 + p1[2];
                mid[3] = (p2[3] - p1[3]) * v7 + p1[3];
                CM_TraceThroughTree(tw, node->children[side], p1, mid, trace);
                if (frac2 > 1.0)
                    MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1268, 0, "%s\n\t(frac) = %g", "(frac2 <= 1.0f)", v7);
                v6 = frac2 - 0.0;
                if (v6 < 0.0)
                    v5 = 0.0;
                else
                    v5 = frac2;
                frac2 = v5;
                p1[0] = (*p2 - p1[0]) * v5 + p1[0];
                p1[1] = (p2[1] - p1[1]) * v5 + p1[1];
                p1[2] = (p2[2] - p1[2]) * v5 + p1[2];
                p1[3] = (p2[3] - p1[3]) * v5 + p1[3];
                num = node->children[!side];
            }
            else
            {
                num = node->children[1];
            }
        }
        else
        {
            num = node->children[0];
        }
    }
    CM_TraceThroughLeaf(tw, &cm.leafs[-1 - num], trace);
}

void __cdecl CM_SetAxialCullOnly(traceWork_t *tw)
{
    float totalExtents[3]; // [esp+4h] [ebp-18h] BYREF
    float totalVolume; // [esp+10h] [ebp-Ch]
    float boxOctantVolume; // [esp+14h] [ebp-8h]
    float twiceSweptVolume; // [esp+18h] [ebp-4h]

    Vec3Sub(tw->bounds[1], tw->bounds[0], totalExtents);
    totalVolume = totalExtents[0] * totalExtents[1] * totalExtents[2];
    boxOctantVolume = tw->size[0] * tw->size[1] * tw->size[2];
    twiceSweptVolume = boxOctantVolume * 16.0 * tw->deltaLen;
    tw->axialCullOnly = totalVolume < (double)twiceSweptVolume;
}

void __cdecl CM_TransformedBoxTraceRotated(
    trace_t *results,
    const float *start,
    const float *end,
    const float *mins,
    const float *maxs,
    unsigned int model,
    int brushmask,
    const float *origin,
    float (*matrix)[3])
{
    float transpose[3][3]; // [esp+4h] [ebp-6Ch] BYREF
    float halfheight; // [esp+28h] [ebp-48h]
    float symetricSize[3][3]; // [esp+2Ch] [ebp-44h] BYREF
    float end_l[3]; // [esp+50h] [ebp-20h] BYREF
    float start_l[3]; // [esp+5Ch] [ebp-14h] BYREF
    int i; // [esp+68h] [ebp-8h]
    float oldFraction; // [esp+6Ch] [ebp-4h]

    if (!mins)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1521, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1522, 0, "%s", "maxs");
    for (i = 0; i < 3; ++i)
    {
        symetricSize[2][i] = (mins[i] + maxs[i]) * 0.5;
        symetricSize[0][i] = mins[i] - symetricSize[2][i];
        symetricSize[1][i] = maxs[i] - symetricSize[2][i];
        start_l[i] = start[i] + symetricSize[2][i];
        end_l[i] = end[i] + symetricSize[2][i];
    }
    Vec3Sub(start_l, origin, start_l);
    Vec3Sub(end_l, origin, end_l);
    halfheight = symetricSize[1][2];
    G_RotatePoint(start_l, matrix);
    G_RotatePoint(end_l, matrix);
    oldFraction = results->fraction;
    CM_Trace(results, start_l, end_l, symetricSize[0], symetricSize[1], model, brushmask);
    if (oldFraction > (double)results->fraction)
    {
        G_TransposeMatrix(matrix, transpose);
        G_RotatePoint(results->normal, transpose);
    }
}

void __cdecl CM_TransformedBoxTrace(
    trace_t *results,
    const float *start,
    const float *end,
    const float *mins,
    const float *maxs,
    __int64 model,
    const float *origin,
    const float *angles)
{
    const char *v8; // eax
    float matrix[3][3]; // [esp+10h] [ebp-70h] BYREF
    float halfwidth; // [esp+34h] [ebp-4Ch]
    float halfheight; // [esp+38h] [ebp-48h]
    float symetricSize[3][3]; // [esp+3Ch] [ebp-44h] BYREF
    float end_l[3]; // [esp+60h] [ebp-20h] BYREF
    float start_l[3]; // [esp+6Ch] [ebp-14h] BYREF
    int i; // [esp+78h] [ebp-8h]
    float oldFraction; // [esp+7Ch] [ebp-4h]

    if (!mins)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1580, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1581, 0, "%s", "maxs");
    if (*angles == 0.0 && angles[1] == 0.0 && angles[2] == 0.0)
    {
        for (i = 0; i < 3; ++i)
        {
            symetricSize[2][i] = (mins[i] + maxs[i]) * 0.5;
            symetricSize[0][i] = mins[i] - symetricSize[2][i];
            symetricSize[1][i] = maxs[i] - symetricSize[2][i];
            start_l[i] = start[i] + symetricSize[2][i];
            end_l[i] = end[i] + symetricSize[2][i];
        }
        Vec3Sub(start_l, origin, start_l);
        Vec3Sub(end_l, origin, end_l);
        halfwidth = symetricSize[1][0];
        halfheight = symetricSize[1][2];
        if (symetricSize[1][0] - symetricSize[1][2] >= 0.009999999776482582)
        {
            v8 = va("halfwidth: %f, halfheight: %f", halfwidth, halfheight);
            MyAssertHandler(
                ".\\qcommon\\cm_trace.cpp",
                1609,
                0,
                "%s\n\t%s",
                "(halfwidth - halfheight) < CAPSULE_SIZE_EPSILON",
                v8);
        }
        oldFraction = results->fraction;
        CM_Trace(results, start_l, end_l, symetricSize[0], symetricSize[1], model, SHIDWORD(model));
    }
    else
    {
        AnglesToAxis(angles, matrix);
        CM_TransformedBoxTraceRotated(results, start, end, mins, maxs, model, SHIDWORD(model), origin, matrix);
    }
}

void __cdecl CM_TransformedBoxTraceExternal(
    trace_t *results,
    const float *start,
    const float *end,
    const float *mins,
    const float *maxs,
    __int64 model,
    const float *origin,
    const float *angles)
{
    memset((unsigned __int8 *)results, 0, sizeof(trace_t));
    results->fraction = 1.0;
    CM_TransformedBoxTrace(results, start, end, mins, maxs, model, origin, angles);
}

int __cdecl CM_BoxSightTrace(
    int oldHitNum,
    const float *start,
    const float *end,
    const float *mins,
    const float *maxs,
    unsigned int model,
    int brushmask)
{
    const char *v7; // eax
    const char *v8; // eax
    double v9; // st7
    double v10; // st7
    float v12; // [esp+14h] [ebp-13Ch]
    float v13; // [esp+18h] [ebp-138h]
    float v14; // [esp+1Ch] [ebp-134h]
    cmodel_t *cmod; // [esp+5Ch] [ebp-F4h]
    traceWork_t tw; // [esp+60h] [ebp-F0h] BYREF
    float offset[3]; // [esp+110h] [ebp-40h]
    trace_t trace; // [esp+11Ch] [ebp-34h] BYREF
    int i; // [esp+148h] [ebp-8h]
    int hitNum; // [esp+14Ch] [ebp-4h]

    memset(&trace, 0, sizeof(trace_t));

    iassert(cm.numNodes);
    iassert(mins);
    iassert(maxs);

    PROF_SCOPED("CM_SightTrace");

    cmod = CM_ClipHandleToModel(model);
    trace.fraction = 1.0;
    trace.startsolid = 0;
    trace.allsolid = 0;
    tw.contents = brushmask;

    for (i = 0; i < 3; ++i)
    {
        iassert(maxs[i] >= mins[i]);

        offset[i] = (mins[i] + maxs[i]) * 0.5f;
        tw.size[i] = maxs[i] - offset[i];
        tw.extents.start[i] = start[i] + offset[i];
        tw.extents.end[i] = end[i] + offset[i];
        tw.midpoint[i] = (tw.extents.start[i] + tw.extents.end[i]) * 0.5f;
        tw.delta[i] = tw.extents.end[i] - tw.extents.start[i];
        tw.halfDelta[i] = tw.delta[i] * 0.5f;
        tw.halfDeltaAbs[i] = I_fabs(tw.halfDelta[i]);
    }

    CM_CalcTraceExtents(&tw.extents);
    tw.deltaLenSq = Vec3LengthSq(tw.delta);
    tw.deltaLen = sqrt(tw.deltaLenSq);

    iassert((tw.size[0] - tw.size[2]) < CAPSULE_SIZE_EPSILON);
    iassert((tw.size[1] - tw.size[2]) < CAPSULE_SIZE_EPSILON);

    if (tw.size[2] >= tw.size[0])
        v12 = tw.size[0];
    else
        v12 = tw.size[2];

    tw.radius = v12;
    tw.boundingRadius = Vec3Length(tw.size);
    tw.offsetZ = tw.size[2] - tw.radius;

    for (i = 0; i < 2; ++i)
    {
        if (tw.extents.end[i] <= tw.extents.start[i])
        {
            tw.bounds[0][i] = tw.extents.end[i] - tw.radius;
            v9 = tw.extents.start[i] + tw.radius;
        }
        else
        {
            tw.bounds[0][i] = tw.extents.start[i] - tw.radius;
            v9 = tw.extents.end[i] + tw.radius;
        }
        tw.bounds[1][i] = v9;
    }

    iassert(tw.offsetZ >= 0);

    if (tw.extents.end[2] <= tw.extents.start[2])
    {
        tw.bounds[0][2] = tw.extents.end[2] - tw.offsetZ - tw.radius;
        v10 = tw.extents.start[2] + tw.offsetZ + tw.radius;
    }
    else
    {
        tw.bounds[0][2] = tw.extents.start[2] - tw.offsetZ - tw.radius;
        v10 = tw.extents.end[2] + tw.offsetZ + tw.radius;
    }

    tw.bounds[1][2] = v10;
    CM_SetAxialCullOnly(&tw);
    iassert(tw.size[0] >= 0);
    iassert(tw.size[1] >= 0);
    iassert(tw.size[2] >= 0);
    tw.isPoint = (tw.size[0] + tw.size[1] + tw.size[2]) == 0.0f;
    iassert(tw.offsetZ >= 0);
    tw.radiusOffset[0] = tw.radius;
    tw.radiusOffset[1] = tw.radius;
    tw.radiusOffset[2] = tw.radius + tw.offsetZ;
    CM_GetTraceThreadInfo(&tw.threadInfo);
    if (model)
    {
        if (model == 4095)
        {
            if ((tw.contents & tw.threadInfo.box_brush->contents) != 0)
                hitNum = CM_SightTraceCapsuleThroughCapsule(&tw, &trace);
            else
                hitNum = 0;
        }
        else
        {
            hitNum = CM_SightTraceThroughLeaf(&tw, &cmod->leaf, &trace);
        }
    }
    else
    {
        hitNum = 0;
        if (oldHitNum > 0)
        {
            if ((oldHitNum - 1) < cm.numBrushes)
                hitNum = CM_SightTraceThroughBrush(&tw, &cm.brushes[oldHitNum - 1]);
        }
        if (!hitNum)
            hitNum = CM_SightTraceThroughTree(&tw, 0, tw.extents.start, tw.extents.end, &trace);
    }

    return hitNum;
}

int __cdecl CM_SightTraceThroughBrush(const traceWork_t *tw, cbrush_t *brush)
{
    float v3; // [esp+0h] [ebp-B4h]
    float v4; // [esp+4h] [ebp-B0h]
    float v5; // [esp+8h] [ebp-ACh]
    float v6; // [esp+Ch] [ebp-A8h]
    float v7; // [esp+10h] [ebp-A4h]
    float v8; // [esp+24h] [ebp-90h]
    float d1; // [esp+78h] [ebp-3Ch]
    float d1a; // [esp+78h] [ebp-3Ch]
    cbrushside_t *side; // [esp+7Ch] [ebp-38h]
    int j; // [esp+80h] [ebp-34h]
    cplane_s *plane; // [esp+84h] [ebp-30h]
    float enterFrac; // [esp+88h] [ebp-2Ch]
    float delta; // [esp+8Ch] [ebp-28h]
    float deltaa; // [esp+8Ch] [ebp-28h]
    float frac; // [esp+90h] [ebp-24h]
    float fraca; // [esp+90h] [ebp-24h]
    float dist; // [esp+94h] [ebp-20h]
    float sign; // [esp+98h] [ebp-1Ch]
    float d2; // [esp+9Ch] [ebp-18h]
    float d2a; // [esp+9Ch] [ebp-18h]
    cbrush_t *bounds; // [esp+A0h] [ebp-14h]
    int index; // [esp+A4h] [ebp-10h]
    float leaveFrac; // [esp+A8h] [ebp-Ch]
    signed int i; // [esp+B0h] [ebp-4h]

    if ((COERCE_UNSIGNED_INT(tw->extents.start[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tw->extents.start[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tw->extents.start[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\qcommon\\cm_trace.cpp",
            1665,
            0,
            "%s",
            "!IS_NAN((tw->extents.start)[0]) && !IS_NAN((tw->extents.start)[1]) && !IS_NAN((tw->extents.start)[2])");
    }
    if ((COERCE_UNSIGNED_INT(tw->extents.end[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tw->extents.end[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(tw->extents.end[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\qcommon\\cm_trace.cpp",
            1666,
            0,
            "%s",
            "!IS_NAN((tw->extents.end)[0]) && !IS_NAN((tw->extents.end)[1]) && !IS_NAN((tw->extents.end)[2])");
    }
    enterFrac = 0.0;
    leaveFrac = 1.0;
    sign = -1.0;
    bounds = brush;
    for (index = 0; ; index = 1)
    {
        if ((COERCE_UNSIGNED_INT(bounds->mins[0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(bounds->mins[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(bounds->mins[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\qcommon\\cm_trace.cpp",
                1683,
                0,
                "%s",
                "!IS_NAN((bounds)[0]) && !IS_NAN((bounds)[1]) && !IS_NAN((bounds)[2])");
        }
        if ((COERCE_UNSIGNED_INT(tw->radiusOffset[0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(tw->radiusOffset[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(tw->radiusOffset[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\qcommon\\cm_trace.cpp",
                1684,
                0,
                "%s",
                "!IS_NAN((tw->radiusOffset)[0]) && !IS_NAN((tw->radiusOffset)[1]) && !IS_NAN((tw->radiusOffset)[2])");
        }
        for (j = 0; j < 3; ++j)
        {
            d1 = (tw->extents.start[j] - bounds->mins[j]) * sign - tw->radiusOffset[j];
            d2 = (tw->extents.end[j] - bounds->mins[j]) * sign - tw->radiusOffset[j];
            if ((LODWORD(d1) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1690, 0, "%s", "!IS_NAN(d1)");
            if ((LODWORD(d2) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1691, 0, "%s", "!IS_NAN(d2)");
            if (d1 <= 0.0)
            {
                if (d2 > 0.0)
                {
                    fraca = d1 * tw->extents.invDelta[j] * sign;
                    if (enterFrac >= (double)fraca)
                        return 0;
                    v5 = fraca - leaveFrac;
                    if (v5 < 0.0)
                        v4 = d1 * tw->extents.invDelta[j] * sign;
                    else
                        v4 = leaveFrac;
                    leaveFrac = v4;
                }
            }
            else
            {
                if (d2 > 0.0)
                    return 0;
                frac = d1 * tw->extents.invDelta[j] * sign;
                if (leaveFrac <= (double)frac)
                    return 0;
                v7 = enterFrac - frac;
                if (v7 < 0.0)
                    v6 = d1 * tw->extents.invDelta[j] * sign;
                else
                    v6 = enterFrac;
                enterFrac = v6;
            }
        }
        if (index)
            break;
        sign = 1.0;
        bounds = (cbrush_t *)brush->maxs;
    }
    side = brush->sides;
    i = brush->numsides;
    if (i < 0)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1721, 0, "%s", "i >= 0");
    while (i)
    {
        plane = side->plane;
        if ((COERCE_UNSIGNED_INT(side->plane->dist) & 0x7F800000) == 0x7F800000)
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1726, 0, "%s", "!IS_NAN(plane->dist)");
        if ((COERCE_UNSIGNED_INT(tw->radius) & 0x7F800000) == 0x7F800000)
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1727, 0, "%s", "!IS_NAN(tw->radius)");
        if ((COERCE_UNSIGNED_INT(plane->normal[0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(plane->normal[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(plane->normal[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\qcommon\\cm_trace.cpp",
                1730,
                0,
                "%s",
                "!IS_NAN((plane->normal)[0]) && !IS_NAN((plane->normal)[1]) && !IS_NAN((plane->normal)[2])");
        }
        if ((COERCE_UNSIGNED_INT(tw->offsetZ) & 0x7F800000) == 0x7F800000)
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1731, 0, "%s", "!IS_NAN(tw->offsetZ)");
        v8 = tw->offsetZ * plane->normal[2];
        v3 = I_fabs(v8);
        dist = plane->dist + tw->radius + v3;
        if ((LODWORD(dist) & 0x7F800000) == 0x7F800000)
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1734, 0, "%s", "!IS_NAN(dist)");
        d1a = Vec3Dot(tw->extents.start, plane->normal) - dist;
        d2a = Vec3Dot(tw->extents.end, plane->normal) - dist;
        if ((LODWORD(d1a) & 0x7F800000) == 0x7F800000)
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1739, 0, "%s", "!IS_NAN(d1)");
        if ((LODWORD(d2a) & 0x7F800000) == 0x7F800000)
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1740, 0, "%s", "!IS_NAN(d2)");
        if (d1a <= 0.0)
        {
            if (d2a > 0.0)
            {
                deltaa = d1a - d2a;
                if (deltaa >= 0.0)
                    MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1759, 0, "%s", "delta < 0");
                if (d1a > leaveFrac * deltaa)
                {
                    leaveFrac = d1a / deltaa;
                    if (leaveFrac <= (double)enterFrac)
                        return 0;
                }
            }
        }
        else
        {
            delta = d1a - d2a;
            if ((LODWORD(delta) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1745, 0, "%s", "!IS_NAN(delta)");
            if (d2a > 0.0)
                return 0;
            if (delta <= 0.0)
                MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1748, 0, "%s", "delta > 0");
            if (d1a > enterFrac * delta)
            {
                enterFrac = d1a / delta;
                if (leaveFrac <= (double)enterFrac)
                    return 0;
            }
        }
        --i;
        ++side;
    }
    return brush - cm.brushes + 1;
}

int __cdecl CM_SightTraceThroughLeaf(const traceWork_t *tw, cLeaf_t *leaf, trace_t *trace)
{
    int hitnum; // [esp+60h] [ebp-8h]
    int k; // [esp+64h] [ebp-4h]

    if ((tw->contents & leaf->brushContents) != 0)
    {
        PROF_SCOPED("CM_SightBrush");
        hitnum = CM_SightTraceThroughLeafBrushNode(tw, leaf);

        if (hitnum)
            return hitnum;
    }

    iassert(trace->fraction == 1.0f);

    if ((tw->contents & leaf->terrainContents) != 0)
    {
        PROF_SCOPED("CM_SightTerrain");
        for (k = 0; k < leaf->collAabbCount; ++k)
        {
            CM_SightTraceThroughAabbTree(tw, &cm.aabbTrees[k + leaf->firstCollAabbIndex], trace);
            if (trace->fraction != 1.0)
            {
                return leaf->firstCollAabbIndex + k + cm.numBrushes + 1;
            }
        }
    }

    return 0;
}

int __cdecl CM_SightTraceThroughLeafBrushNode(const traceWork_t *tw, cLeaf_t *leaf)
{
    float absmin[3]; // [esp+4h] [ebp-18h] BYREF
    float absmax[3]; // [esp+10h] [ebp-Ch] BYREF

    if (!leaf->leafBrushNode)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1907, 0, "%s", "leaf->leafBrushNode");
    Vec3Sub(leaf->mins, tw->size, absmin);
    Vec3Add(leaf->maxs, tw->size, absmax);
    if (CM_TraceBox(&tw->extents, absmin, absmax, 1.0))
        return 0;
    else
        return CM_SightTraceThroughLeafBrushNode_r(
            tw,
            &cm.leafbrushNodes[leaf->leafBrushNode],
            tw->extents.start,
            tw->extents.end);
}

int __cdecl CM_SightTraceThroughLeafBrushNode_r(
    const traceWork_t *tw,
    cLeafBrushNode_s *node,
    const float *p1_,
    const float *p2)
{
    float v5; // [esp+8h] [ebp-88h]
    float v6; // [esp+Ch] [ebp-84h]
    float v7; // [esp+10h] [ebp-80h]
    float v8; // [esp+14h] [ebp-7Ch]
    float v9; // [esp+1Ch] [ebp-74h]
    float v10; // [esp+20h] [ebp-70h]
    float v11; // [esp+24h] [ebp-6Ch]
    float v12; // [esp+28h] [ebp-68h]
    float v13; // [esp+2Ch] [ebp-64h]
    float v14; // [esp+30h] [ebp-60h]
    bool side; // [esp+34h] [ebp-5Ch]
    float diff; // [esp+38h] [ebp-58h]
    float t1; // [esp+40h] [ebp-50h]
    float frac; // [esp+44h] [ebp-4Ch]
    int k; // [esp+48h] [ebp-48h]
    float p1[3]; // [esp+50h] [ebp-40h] BYREF
    float offset; // [esp+5Ch] [ebp-34h]
    float tmax; // [esp+60h] [ebp-30h]
    float t2; // [esp+64h] [ebp-2Ch]
    float frac2; // [esp+68h] [ebp-28h]
    unsigned __int16 *brushes; // [esp+6Ch] [ebp-24h]
    float absDiff; // [esp+70h] [ebp-20h]
    float invDist; // [esp+74h] [ebp-1Ch]
    int hitNum; // [esp+78h] [ebp-18h]
    float tmin; // [esp+7Ch] [ebp-14h]
    int brushnum; // [esp+80h] [ebp-10h]
    float mid[3]; // [esp+84h] [ebp-Ch] BYREF

    if (!node)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1802, 0, "%s", "node");
    p1[0] = *p1_;
    p1[1] = p1_[1];
    p1[2] = p1_[2];
    while (1)
    {
        if ((tw->contents & node->contents) == 0)
            return 0;
        if (node->leafBrushCount)
            break;
    LABEL_18:
        t1 = p1[node->axis] - node->data.children.dist;
        t2 = p2[node->axis] - node->data.children.dist;
        offset = tw->size[node->axis] + 0.125 - node->data.children.range;
        v14 = t1 - t2;
        if (v14 < 0.0)
            v13 = t2;
        else
            v13 = t1;
        tmax = v13;
        v12 = t2 - t1;
        if (v12 < 0.0)
            v11 = t2;
        else
            v11 = t1;
        tmin = v11;
        if (offset > (double)v11)
        {
            if (tmax > -offset)
            {
                diff = t2 - t1;
                v10 = I_fabs(diff);
                absDiff = v10;
                if (v10 <= 0.000000476837158203125)
                {
                    side = 0;
                    frac = 1.0;
                    frac2 = 0.0;
                }
                else
                {
                    if (diff < 0.0)
                        v9 = t1;
                    else
                        v9 = -t1;
                    invDist = 1.0 / absDiff;
                    frac2 = (v9 - offset) * invDist;
                    frac = (v9 + offset) * invDist;
                    side = diff >= 0.0;
                }
                if (frac < 0.0)
                    MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1874, 0, "%s", "frac >= 0");
                v8 = 1.0 - frac;
                if (v8 < 0.0)
                    v7 = 1.0;
                else
                    v7 = frac;
                mid[0] = (*p2 - p1[0]) * v7 + p1[0];
                mid[1] = (p2[1] - p1[1]) * v7 + p1[1];
                mid[2] = (p2[2] - p1[2]) * v7 + p1[2];
                hitNum = CM_SightTraceThroughLeafBrushNode_r(tw, &node[node->data.children.childOffset[side]], p1, mid);
                if (hitNum)
                    return hitNum;
                if (frac2 > 1.000100016593933)
                    MyAssertHandler(".\\qcommon\\cm_trace.cpp", 1885, 0, "%s\n\t(frac2) = %g", "(frac2 <= 1.0001f)", frac2);
                v6 = frac2 - 0.0;
                if (v6 < 0.0)
                    v5 = 0.0;
                else
                    v5 = frac2;
                frac2 = v5;
                p1[0] = (*p2 - p1[0]) * v5 + p1[0];
                p1[1] = (p2[1] - p1[1]) * v5 + p1[1];
                p1[2] = (p2[2] - p1[2]) * v5 + p1[2];
                node += node->data.children.childOffset[1 - side];
            }
            else
            {
                node += node->data.children.childOffset[1];
            }
        }
        else
        {
            if (tmax <= -offset)
                return 0;
            node += node->data.children.childOffset[0];
        }
    }
    if (node->leafBrushCount <= 0)
    {
        hitNum = CM_SightTraceThroughLeafBrushNode_r(tw, node + 1, p1, p2);
        if (hitNum)
            return hitNum;
        goto LABEL_18;
    }
    brushes = node->data.leaf.brushes;
    for (k = 0; k < node->leafBrushCount; ++k)
    {
        brushnum = brushes[k];
        if ((tw->contents & cm.brushes[brushnum].contents) != 0)
        {
            hitNum = CM_SightTraceThroughBrush(tw, &cm.brushes[brushnum]);
            if (hitNum)
                return hitNum;
        }
    }
    return 0;
}

int __cdecl CM_SightTraceCapsuleThroughCapsule(const traceWork_t *tw, trace_t *trace)
{
    float v3; // [esp+Ch] [ebp-A4h]
    float v4; // [esp+10h] [ebp-A0h]
    float v5; // [esp+14h] [ebp-9Ch]
    float v6; // [esp+18h] [ebp-98h]
    float v7; // [esp+1Ch] [ebp-94h]
    float v8; // [esp+20h] [ebp-90h]
    float v9; // [esp+24h] [ebp-8Ch]
    float endtop[3]; // [esp+30h] [ebp-80h] BYREF
    float starttop[3]; // [esp+3Ch] [ebp-74h] BYREF
    float halfwidth; // [esp+48h] [ebp-68h]
    float top[3]; // [esp+4Ch] [ebp-64h] BYREF
    float offs; // [esp+58h] [ebp-58h]
    float halfheight; // [esp+5Ch] [ebp-54h]
    float symetricSize[2][3]; // [esp+60h] [ebp-50h]
    float radius; // [esp+78h] [ebp-38h]
    float offset[3]; // [esp+7Ch] [ebp-34h] BYREF
    float endbottom[3]; // [esp+88h] [ebp-28h] BYREF
    float startbottom[3]; // [esp+94h] [ebp-1Ch] BYREF
    int i; // [esp+A0h] [ebp-10h]
    float bottom[3]; // [esp+A4h] [ebp-Ch] BYREF

    v9 = tw->threadInfo.box_model->maxs[0] + 1.0;
    if (tw->bounds[0][0] > (double)v9)
        return 0;
    v8 = tw->threadInfo.box_model->maxs[1] + 1.0;
    if (tw->bounds[0][1] > (double)v8)
        return 0;
    v7 = tw->threadInfo.box_model->maxs[2] + 1.0;
    if (tw->bounds[0][2] > (double)v7)
        return 0;
    v6 = tw->threadInfo.box_model->mins[0] - 1.0;
    if (tw->bounds[1][0] < (double)v6)
        return 0;
    v5 = tw->threadInfo.box_model->mins[1] - 1.0;
    if (tw->bounds[1][1] < (double)v5)
        return 0;
    v4 = tw->threadInfo.box_model->mins[2] - 1.0;
    if (tw->bounds[1][2] < (double)v4)
        return 0;

    starttop[0] = tw->extents.start[0];
    starttop[1] = tw->extents.start[1];
    starttop[2] = tw->extents.start[2] + tw->offsetZ;
    startbottom[0] = tw->extents.start[0];
    startbottom[1] = tw->extents.start[1];
    startbottom[2] = tw->extents.start[2] - tw->offsetZ;
    endtop[0] = tw->extents.end[0];
    endtop[1] = tw->extents.end[1];
    endtop[2] = tw->extents.end[2] + tw->offsetZ;
    endbottom[0] = tw->extents.end[0];
    endbottom[1] = tw->extents.end[1];
    endbottom[2] = tw->extents.end[2] - tw->offsetZ;
    for (i = 0; i < 3; ++i)
    {
        offset[i] = (tw->threadInfo.box_model->mins[i] + tw->threadInfo.box_model->maxs[i]) * 0.5;
        symetricSize[0][i] = tw->threadInfo.box_model->mins[i] - offset[i];
        symetricSize[1][i] = tw->threadInfo.box_model->maxs[i] - offset[i];
    }
    halfwidth = symetricSize[1][0];
    halfheight = symetricSize[1][2];
    if (symetricSize[1][0] <= symetricSize[1][2])
        v3 = halfwidth;
    else
        v3 = halfheight;
    radius = v3;
    offs = halfheight - v3;
    top[0] = offset[0];
    top[1] = offset[1];
    top[2] = offset[2] + offs;
    bottom[0] = offset[0];
    bottom[1] = offset[1];
    bottom[2] = offset[2] - offs;

    if (startbottom[2] <= top[2])
    {
        if (bottom[2] > starttop[2])
        {
            if (!CM_SightTraceSphereThroughSphere(tw, starttop, endtop, bottom, radius, trace))
                return -1;
            if (tw->delta[2] <= 0.0)
                return 0;
        }
    }
    else
    {
        if (!CM_SightTraceSphereThroughSphere(tw, startbottom, endbottom, top, radius, trace))
            return -1;
        if (tw->delta[2] >= 0.0)
            return 0;
    }
    if (!CM_SightTraceCylinderThroughCylinder(tw, offset, offs, radius, trace))
        return -1;
    if (endbottom[2] <= top[2]) 
    {
        if (bottom[2] > endtop[2]
            && starttop[2] >= bottom[2]
            && !CM_SightTraceSphereThroughSphere(tw, starttop, endtop, bottom, radius, trace))
        {
            return -1;
        }
    }
    else if (top[2] >= startbottom[2]
        && !CM_SightTraceSphereThroughSphere(tw, startbottom, endbottom, top, radius, trace))
    {
        return -1;
    }
    return 0;
}

bool __cdecl CM_SightTraceSphereThroughSphere(
    const traceWork_t *tw,
    const float *vStart,
    const float *vEnd,
    const float *vStationary,
    float radius,
    trace_t *trace)
{
    float v7; // [esp+4h] [ebp-40h]
    float fDiscriminant; // [esp+10h] [ebp-34h]
    float fEntry; // [esp+14h] [ebp-30h]
    float fRadiusSqrd; // [esp+18h] [ebp-2Ch]
    float fC; // [esp+1Ch] [ebp-28h]
    float fB; // [esp+20h] [ebp-24h]
    float fA; // [esp+24h] [ebp-20h]
    float vNormal[3]; // [esp+28h] [ebp-1Ch] BYREF
    float vDelta[3]; // [esp+34h] [ebp-10h] BYREF
    float fDeltaLen; // [esp+40h] [ebp-4h]

    Vec3Sub(vStart, vStationary, vDelta);
    fRadiusSqrd = (radius + tw->radius) * (radius + tw->radius);
    fC = Vec3Dot(vDelta, vDelta) - fRadiusSqrd;
    if (fC <= 0.0)
        return 0;
    fB = Vec3Dot(tw->delta, vDelta);
    if (fB >= 0.0)
        return 1;
    fA = tw->deltaLenSq;
    fDiscriminant = fB * fB - fA * fC;
    if (fDiscriminant < 0.0)
        return 1;
    fDeltaLen = Vec3NormalizeTo(vDelta, vNormal);
    v7 = sqrt(fDiscriminant);
    fEntry = (-fB - v7) / fA + fB * 0.125 / fDeltaLen;
    return trace->fraction <= (double)fEntry;
}

bool __cdecl CM_SightTraceCylinderThroughCylinder(
    const traceWork_t *tw,
    const float *vStationary,
    float fStationaryHalfHeight,
    float radius,
    trace_t *trace)
{
    float v6; // [esp+4h] [ebp-68h]
    float v7; // [esp+8h] [ebp-64h]
    float v8; // [esp+18h] [ebp-54h]
    float v9; // [esp+1Ch] [ebp-50h]
    float fDiscriminant; // [esp+2Ch] [ebp-40h]
    float fEntry; // [esp+30h] [ebp-3Ch]
    float fRadiusSqrd; // [esp+34h] [ebp-38h]
    float fHitHeight; // [esp+38h] [ebp-34h]
    float fEpsilon; // [esp+3Ch] [ebp-30h]
    float fTotalHeight; // [esp+40h] [ebp-2Ch]
    float fTotalHeighta; // [esp+40h] [ebp-2Ch]
    float fC; // [esp+44h] [ebp-28h]
    float fB; // [esp+48h] [ebp-24h]
    float fA; // [esp+4Ch] [ebp-20h]
    float vNormal[3]; // [esp+50h] [ebp-1Ch] BYREF
    float vDelta[3]; // [esp+5Ch] [ebp-10h] BYREF
    float fDeltaLen; // [esp+68h] [ebp-4h]

    Vec3Sub(tw->extents.start, vStationary, vDelta);
    fRadiusSqrd = (radius + tw->radius) * (radius + tw->radius);
    v9 = vDelta[1] * vDelta[1] + vDelta[0] * vDelta[0];
    fC = v9 - fRadiusSqrd;
    if (fC > 0.0)
    {
        fB = vDelta[1] * tw->delta[1] + vDelta[0] * tw->delta[0];
        if (fB < 0.0)
        {
            fA = tw->delta[1] * tw->delta[1] + tw->delta[0] * tw->delta[0];
            fDiscriminant = fB * fB - fA * fC;
            if (fDiscriminant >= 0.0)
            {
                vDelta[2] = 0.0;
                fDeltaLen = Vec3NormalizeTo(vDelta, vNormal);
                fEpsilon = fB * 0.125 / fDeltaLen;
                v7 = sqrt(fDiscriminant);
                fEntry = (-fB - v7) / fA + fEpsilon;
                if (trace->fraction > (double)fEntry)
                {
                    fTotalHeighta = tw->size[2] - tw->radius + fStationaryHalfHeight;
                    fHitHeight = (fEntry - fEpsilon) * tw->delta[2] + tw->extents.start[2] - vStationary[2];
                    if (fTotalHeighta < 0.0)
                        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 2053, 0, "%s", "fTotalHeight >= 0");
                    v6 = I_fabs(fHitHeight);
                    return fTotalHeighta < (double)v6;
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                return 1;
            }
        }
        else
        {
            return 1;
        }
    }
    else
    {
        fTotalHeight = tw->size[2] - tw->radius + fStationaryHalfHeight;
        if (fTotalHeight < 0.0)
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 2029, 0, "%s", "fTotalHeight >= 0");
        v8 = I_fabs(vDelta[2]);
        return fTotalHeight < (double)v8;
    }
}

int __cdecl CM_SightTraceThroughTree(const traceWork_t *tw, int num, const float *p1_, const float *p2, trace_t *trace)
{
    float v6; // [esp+8h] [ebp-78h]
    float v7; // [esp+Ch] [ebp-74h]
    float v8; // [esp+10h] [ebp-70h]
    float v9; // [esp+14h] [ebp-6Ch]
    float v10; // [esp+1Ch] [ebp-64h]
    float v11; // [esp+20h] [ebp-60h]
    float v12; // [esp+24h] [ebp-5Ch]
    float v13; // [esp+28h] [ebp-58h]
    float v14; // [esp+2Ch] [ebp-54h]
    float v15; // [esp+30h] [ebp-50h]
    cNode_t *node; // [esp+34h] [ebp-4Ch]
    bool side; // [esp+38h] [ebp-48h]
    float diff; // [esp+3Ch] [ebp-44h]
    cplane_s *plane; // [esp+40h] [ebp-40h]
    float t1; // [esp+48h] [ebp-38h]
    float frac; // [esp+4Ch] [ebp-34h]
    float p1[3]; // [esp+50h] [ebp-30h] BYREF
    float offset; // [esp+5Ch] [ebp-24h]
    float t2; // [esp+60h] [ebp-20h]
    float frac2; // [esp+64h] [ebp-1Ch]
    float absDiff; // [esp+68h] [ebp-18h]
    float invDist; // [esp+6Ch] [ebp-14h]
    int hitNum; // [esp+70h] [ebp-10h]
    float mid[3]; // [esp+74h] [ebp-Ch] BYREF

    p1[0] = *p1_;
    p1[1] = p1_[1];
    p1[2] = p1_[2];
    while (1)
    {
        while (1)
        {
            while (1)
            {
                if (num < 0)
                    return CM_SightTraceThroughLeaf(tw, &cm.leafs[-1 - num], trace);
                node = &cm.nodes[num];
                plane = node->plane;
                if (node->plane->type >= 3u)
                {
                    t1 = Vec3Dot(plane->normal, p1) - plane->dist;
                    t2 = Vec3Dot(plane->normal, p2) - plane->dist;
                    offset = tw->isPoint ? 0.125 : tw->boundingRadius + 0.125;
                }
                else
                {
                    t1 = p1[plane->type] - plane->dist;
                    t2 = p2[plane->type] - plane->dist;
                    offset = tw->size[plane->type] + 0.125;
                }
                v15 = t2 - t1;
                v14 = v15 < 0.0 ? t2 : t1;
                if (offset > (double)v14)
                    break;
                num = node->children[0];
            }
            v13 = t1 - t2;
            v12 = v13 < 0.0 ? t2 : t1;
            if (v12 > -offset)
                break;
            num = node->children[1];
        }
        diff = t2 - t1;
        v11 = I_fabs(diff);
        absDiff = v11;
        if (v11 <= 0.000000476837158203125)
        {
            side = 0;
            frac = 1.0;
            frac2 = 0.0;
        }
        else
        {
            if (diff < 0.0)
                v10 = t1;
            else
                v10 = -t1;
            invDist = 1.0 / absDiff;
            frac2 = (v10 - offset) * invDist;
            frac = (v10 + offset) * invDist;
            side = diff >= 0.0;
        }
        if (frac < 0.0)
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 2252, 0, "%s", "frac >= 0");
        v9 = 1.0 - frac;
        v8 = v9 < 0.0 ? 1.0 : frac;
        mid[0] = (*p2 - p1[0]) * v8 + p1[0];
        mid[1] = (p2[1] - p1[1]) * v8 + p1[1];
        mid[2] = (p2[2] - p1[2]) * v8 + p1[2];
        hitNum = CM_SightTraceThroughTree(tw, node->children[side], p1, mid, trace);
        if (hitNum)
            break;
        if (frac2 > 1.0)
            MyAssertHandler(".\\qcommon\\cm_trace.cpp", 2264, 0, "%s\n\t(frac2) = %g", "(frac2 <= 1.0f)", frac2);
        v7 = frac2 - 0.0;
        if (v7 < 0.0)
            v6 = 0.0;
        else
            v6 = frac2;
        frac2 = v6;
        p1[0] = (*p2 - p1[0]) * v6 + p1[0];
        p1[1] = (p2[1] - p1[1]) * v6 + p1[1];
        p1[2] = (p2[2] - p1[2]) * v6 + p1[2];
        num = node->children[!side];
    }
    return hitNum;
}

int __cdecl CM_TransformedBoxSightTrace(
    int hitNum,
    const float *start,
    const float *end,
    const float *mins,
    const float *maxs,
    unsigned int model,
    int brushmask,
    const float *origin,
    const float *angles)
{
    bool v10; // [esp+0h] [ebp-74h]
    float matrix[3][3]; // [esp+4h] [ebp-70h] BYREF
    float halfwidth; // [esp+28h] [ebp-4Ch]
    float halfheight; // [esp+2Ch] [ebp-48h]
    float symetricSize[3][3]; // [esp+30h] [ebp-44h] BYREF
    float end_l[3]; // [esp+54h] [ebp-20h] BYREF
    float start_l[4]; // [esp+60h] [ebp-14h] BYREF
    int i; // [esp+70h] [ebp-4h]

    if (!mins)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 2438, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\qcommon\\cm_trace.cpp", 2439, 0, "%s", "maxs");
    for (i = 0; i < 3; ++i)
    {
        symetricSize[2][i] = (mins[i] + maxs[i]) * 0.5;
        symetricSize[0][i] = mins[i] - symetricSize[2][i];
        symetricSize[1][i] = maxs[i] - symetricSize[2][i];
        start_l[i] = start[i] + symetricSize[2][i];
        end_l[i] = end[i] + symetricSize[2][i];
    }
    Vec3Sub(start_l, origin, start_l);
    Vec3Sub(end_l, origin, end_l);
    v10 = *angles != 0.0 || angles[1] != 0.0 || angles[2] != 0.0;
    LODWORD(start_l[3]) = v10;
    halfwidth = symetricSize[1][0];
    halfheight = symetricSize[1][2];
    if (v10)
    {
        AnglesToAxis(angles, matrix);
        G_RotatePoint(start_l, matrix);
        G_RotatePoint(end_l, matrix);
    }
    return CM_BoxSightTrace(hitNum, start_l, end_l, symetricSize[0], symetricSize[1], model, brushmask);
}

