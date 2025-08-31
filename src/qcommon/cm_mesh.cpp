#include "qcommon.h"
#include <EffectsCore/fx_system.h>
#include <xanim/xanim.h>

void __cdecl CM_TraceThroughAabbTree(const traceWork_t *tw, const CollisionAabbTree *aabbTree, trace_t *trace)
{
    dmaterial_t *materialInfo; // [esp+0h] [ebp-8h]
    float oldFraction; // [esp+4h] [ebp-4h]

    materialInfo = &cm.materials[aabbTree->materialIndex];
    if ((materialInfo->contentFlags & tw->contents) != 0)
    {
        oldFraction = trace->fraction;
        CM_TraceThroughAabbTree_r(tw, aabbTree, trace);
        if (oldFraction > (double)trace->fraction)
        {
            trace->surfaceFlags = materialInfo->surfaceFlags;
            trace->contents = materialInfo->contentFlags;
            trace->material = (const char *)materialInfo;
        }
    }
}

void __cdecl CM_TraceThroughAabbTree_r(const traceWork_t *tw, const CollisionAabbTree *aabbTree, trace_t *trace)
{
    int borderIndex; // [esp+50h] [ebp-1Ch]
    int childIndex; // [esp+54h] [ebp-18h]
    int partitionIndex; // [esp+58h] [ebp-14h]
    CollisionAabbTree *child; // [esp+5Ch] [ebp-10h]
    int checkStamp; // [esp+60h] [ebp-Ch]
    int triIndex; // [esp+64h] [ebp-8h]
    int triIndexa; // [esp+64h] [ebp-8h]
    const CollisionPartition *partition; // [esp+68h] [ebp-4h]

    if (!CM_CullBox(tw, aabbTree->origin, aabbTree->halfSize))
    {
        if (aabbTree->childCount)
        {
            childIndex = 0;
            child = &cm.aabbTrees[aabbTree->u.firstChildIndex];
            while (childIndex < aabbTree->childCount)
            {
                CM_TraceThroughAabbTree_r(tw, child, trace);
                ++childIndex;
                ++child;
            }
        }
        else
        {
            partitionIndex = aabbTree->u.firstChildIndex;
            checkStamp = tw->threadInfo.checkcount.global;
            if (tw->threadInfo.checkcount.partitions[partitionIndex] != checkStamp)
            {
                tw->threadInfo.checkcount.partitions[partitionIndex] = checkStamp;
                partition = &cm.partitions[partitionIndex];
                if (tw->isPoint)
                {
                    for (triIndex = partition->firstTri; triIndex < partition->firstTri + partition->triCount; ++triIndex)
                        CM_TracePointThroughTriangle(tw, &cm.triIndices[3 * triIndex], trace);
                }
                else
                {
                    for (triIndexa = partition->firstTri; triIndexa < partition->firstTri + partition->triCount; ++triIndexa)
                        CM_TraceCapsuleThroughTriangle(tw, triIndexa, &cm.triIndices[3 * triIndexa], trace);
                    if ((tw->delta[0] != 0.0 || tw->delta[1] != 0.0) && tw->offsetZ != 0.0)
                    {
                        for (borderIndex = 0; borderIndex < partition->borderCount; ++borderIndex)
                            CM_TraceCapsuleThroughBorder(tw, &partition->borders[borderIndex], trace);
                    }
                }
            }
        }
    }
}

bool __cdecl CM_CullBox(const traceWork_t *tw, const float *origin, const float *halfSize)
{
    float v4; // [esp+0h] [ebp-78h]
    float v5; // [esp+4h] [ebp-74h]
    float v6; // [esp+8h] [ebp-70h]
    float v7; // [esp+Ch] [ebp-6Ch]
    float v8; // [esp+10h] [ebp-68h]
    float v9; // [esp+14h] [ebp-64h]
    float v10; // [esp+18h] [ebp-60h]
    float v11; // [esp+1Ch] [ebp-5Ch]
    float v12; // [esp+20h] [ebp-58h]
    float v13; // [esp+24h] [ebp-54h]
    float v14; // [esp+28h] [ebp-50h]
    float v15; // [esp+2Ch] [ebp-4Ch]
    float v16; // [esp+34h] [ebp-44h]
    float v17; // [esp+3Ch] [ebp-3Ch]
    float v18; // [esp+44h] [ebp-34h]
    float centerDelta[3]; // [esp+60h] [ebp-18h] BYREF
    float halfBoxSize[3]; // [esp+6Ch] [ebp-Ch] BYREF

    Vec3Sub(tw->midpoint, origin, centerDelta);
    Vec3Add(halfSize, tw->size, halfBoxSize);
    v15 = I_fabs(centerDelta[0]);
    v14 = halfBoxSize[0] + tw->halfDeltaAbs[0];
    if (v15 > (double)v14)
        return 1;
    v13 = I_fabs(centerDelta[1]);
    v12 = halfBoxSize[1] + tw->halfDeltaAbs[1];
    if (v13 > (double)v12)
        return 1;
    v11 = I_fabs(centerDelta[2]);
    v10 = halfBoxSize[2] + tw->halfDeltaAbs[2];
    if (v11 > (double)v10)
        return 1;
    if (tw->axialCullOnly)
        return 0;
    v18 = centerDelta[2] * tw->halfDelta[1] - centerDelta[1] * tw->halfDelta[2];
    v9 = I_fabs(v18);
    v8 = halfBoxSize[1] * tw->halfDeltaAbs[2] + halfBoxSize[2] * tw->halfDeltaAbs[1];
    if (v9 > (double)v8)
        return 1;
    v17 = centerDelta[0] * tw->halfDelta[2] - centerDelta[2] * tw->halfDelta[0];
    v7 = I_fabs(v17);
    v6 = halfBoxSize[2] * tw->halfDeltaAbs[0] + halfBoxSize[0] * tw->halfDeltaAbs[2];
    if (v7 > (double)v6)
        return 1;
    v16 = centerDelta[1] * tw->halfDelta[0] - centerDelta[0] * tw->halfDelta[1];
    v5 = I_fabs(v16);
    v4 = halfBoxSize[0] * tw->halfDeltaAbs[1] + halfBoxSize[1] * tw->halfDeltaAbs[0];
    return v5 > (double)v4;
}

void __cdecl CM_TracePointThroughTriangle(const traceWork_t *tw, const unsigned __int16 *indices, trace_t *trace)
{
    const float *v0; // [esp+8h] [ebp-50h]
    float t; // [esp+Ch] [ebp-4Ch]
    float negativeU; // [esp+10h] [ebp-48h]
    float triNormalScaledByAreaX2[3]; // [esp+14h] [ebp-44h] BYREF
    float v0_start[3]; // [esp+20h] [ebp-38h] BYREF
    float v0_v2[3]; // [esp+2Ch] [ebp-2Ch] BYREF
    float tracePlaneScaledNormal[3]; // [esp+38h] [ebp-20h] BYREF
    float projTriAreaScaledByTraceLenX2; // [esp+44h] [ebp-14h]
    float v; // [esp+48h] [ebp-10h]
    float v0_v1[3]; // [esp+4Ch] [ebp-Ch] BYREF

    v0 = cm.verts[*indices];
    Vec3Sub(v0, cm.verts[indices[1]], v0_v1);
    Vec3Sub(v0, cm.verts[indices[2]], v0_v2);
    Vec3Cross(v0_v2, v0_v1, triNormalScaledByAreaX2);
    projTriAreaScaledByTraceLenX2 = Vec3Dot(tw->delta, triNormalScaledByAreaX2);
    if (projTriAreaScaledByTraceLenX2 < 0.0)
    {
        Vec3Sub(v0, tw->extents.start, v0_start);
        t = Vec3Dot(v0_start, triNormalScaledByAreaX2);
        if (t <= 0.0 && t > trace->fraction * projTriAreaScaledByTraceLenX2)
        {
            Vec3Cross(tw->delta, v0_start, tracePlaneScaledNormal);
            v = Vec3Dot(tracePlaneScaledNormal, v0_v1);
            if (v <= 0.0 && projTriAreaScaledByTraceLenX2 <= (double)v)
            {
                negativeU = Vec3Dot(tracePlaneScaledNormal, v0_v2);
                if (negativeU >= 0.0 && projTriAreaScaledByTraceLenX2 <= v - negativeU)
                {
                    trace->walkable = 0;
                    Vec3NormalizeTo(triNormalScaledByAreaX2, trace->normal);
                    trace->fraction = t / projTriAreaScaledByTraceLenX2;
                    if (trace->fraction < 0.0 || trace->fraction > 1.0)
                        MyAssertHandler(
                            ".\\qcommon\\cm_mesh.cpp",
                            108,
                            1,
                            "%s\n\t(trace->fraction) = %g",
                            "(trace->fraction >= 0 && trace->fraction <= 1.0f)",
                            trace->fraction);
                }
            }
        }
    }
}

void __cdecl CM_TraceCapsuleThroughTriangle(
    const traceWork_t *tw,
    int triIndex,
    const unsigned __int16 *indices,
    trace_t *trace)
{
    const char *v4; // eax
    float scale; // [esp+14h] [ebp-F8h]
    float scalea; // [esp+14h] [ebp-F8h]
    double v7; // [esp+18h] [ebp-F4h]
    float v8; // [esp+24h] [ebp-E8h]
    float v9; // [esp+28h] [ebp-E4h]
    const float *v10; // [esp+2Ch] [ebp-E0h]
    const float *v11; // [esp+30h] [ebp-DCh]
    const float *v12; // [esp+34h] [ebp-D8h]
    float v13; // [esp+38h] [ebp-D4h]
    float offsetZ; // [esp+3Ch] [ebp-D0h]
    const float *v0; // [esp+64h] [ebp-A8h]
    const float *vertToCheck; // [esp+68h] [ebp-A4h]
    float start_v0[3]; // [esp+6Ch] [ebp-A0h] BYREF
    float triNormalScaledByAreaX2[3]; // [esp+78h] [ebp-94h] BYREF
    float hitFrac; // [esp+84h] [ebp-88h]
    bool missedEdge; // [esp+8Bh] [ebp-81h]
    SphereEdgeTraceResult edgeTraceResult; // [esp+8Ch] [ebp-80h]
    float v1_v2[3]; // [esp+90h] [ebp-7Ch] BYREF
    float areaX2; // [esp+9Ch] [ebp-70h]
    float v0_v2[3]; // [esp+A0h] [ebp-6Ch] BYREF
    float normal[3]; // [esp+ACh] [ebp-60h] BYREF
    float startDist; // [esp+B8h] [ebp-54h]
    float sphereStart[3]; // [esp+BCh] [ebp-50h] BYREF
    const float *v2; // [esp+C8h] [ebp-44h]
    float tracePlaneScaledNormal[3]; // [esp+CCh] [ebp-40h] BYREF
    float projTriAreaScaledByTraceLenX2; // [esp+D8h] [ebp-34h]
    float negativeV; // [esp+DCh] [ebp-30h]
    const float *v1; // [esp+E0h] [ebp-2Ch]
    float u; // [esp+E4h] [ebp-28h]
    float shiftedStart[3]; // [esp+E8h] [ebp-24h] BYREF
    bool startSolid; // [esp+F7h] [ebp-15h]
    float v0_v1[3]; // [esp+F8h] [ebp-14h] BYREF
    float hitDist; // [esp+104h] [ebp-8h]
    bool isWalkable; // [esp+10Bh] [ebp-1h]

    v0 = cm.verts[*indices];
    v1 = cm.verts[indices[1]];
    v2 = cm.verts[indices[2]];
    Vec3Sub(v0, v1, v0_v1);
    Vec3Sub(v0, v2, v0_v2);
    Vec3Cross(v0_v2, v0_v1, triNormalScaledByAreaX2);
    projTriAreaScaledByTraceLenX2 = Vec3Dot(tw->delta, triNormalScaledByAreaX2);
    if (projTriAreaScaledByTraceLenX2 < 0.0)
    {
        areaX2 = Vec3NormalizeTo(triNormalScaledByAreaX2, normal);
        sphereStart[0] = tw->extents.start[0];
        sphereStart[1] = tw->extents.start[1];
        sphereStart[2] = tw->extents.start[2];
        if (normal[2] < 0.0)
            offsetZ = -tw->offsetZ;
        else
            offsetZ = tw->offsetZ;
        sphereStart[2] = sphereStart[2] - offsetZ;
        scale = -tw->radius;
        Vec3Mad(sphereStart, scale, normal, shiftedStart);
        Vec3Sub(shiftedStart, v0, start_v0);
        hitDist = Vec3Dot(start_v0, normal);
        if (hitDist >= 0.0)
        {
            hitFrac = -(hitDist - 0.125) * areaX2 / projTriAreaScaledByTraceLenX2;
            if (trace->fraction <= (double)hitFrac)
                return;
            startSolid = 0;
        }
        else
        {
            Vec3Sub(sphereStart, v0, start_v0);
            startDist = Vec3Dot(start_v0, normal);
            v13 = tw->radius * tw->radius;
            if (v13 <= startDist * startDist)
                return;
            scalea = -startDist;
            Vec3Mad(start_v0, scalea, normal, start_v0);
            hitFrac = 0.0;
            startSolid = 1;
        }
        Vec3Cross(tw->delta, start_v0, tracePlaneScaledNormal);
        missedEdge = 0;
        isWalkable = 1;
        vertToCheck = 0;
        negativeV = Vec3Dot(tracePlaneScaledNormal, v0_v1);
        if (negativeV < 0.0)
        {
            missedEdge = 1;
            edgeTraceResult = CM_TraceSphereThroughEdge(tw, sphereStart, v0, v0_v1, trace);
            if (edgeTraceResult == SPHERE_HITS_EDGE)
            {
                trace->walkable = ((1 << ((3 * triIndex + 2) & 7)) & cm.triEdgeIsWalkable[(3 * triIndex + 2) >> 3]) != 0;
                return;
            }
            if (edgeTraceResult != SPHERE_MISSES_EDGE)
            {
                if (edgeTraceResult == SPHERE_MAY_HIT_V0)
                    v12 = v0;
                else
                    v12 = v1;
                vertToCheck = v12;
                isWalkable &= ((1 << ((3 * triIndex + 2) & 7)) & cm.triEdgeIsWalkable[(3 * triIndex + 2) >> 3]) != 0;
            }
        }
        u = Vec3Dot(tracePlaneScaledNormal, v0_v2);
        if (u > 0.0)
        {
            missedEdge = 1;
            edgeTraceResult = CM_TraceSphereThroughEdge(tw, sphereStart, v0, v0_v2, trace);
            if (edgeTraceResult == SPHERE_HITS_EDGE)
            {
                trace->walkable = ((1 << ((3 * triIndex + 1) & 7)) & cm.triEdgeIsWalkable[(3 * triIndex + 1) >> 3]) != 0;
                return;
            }
            if (edgeTraceResult != SPHERE_MISSES_EDGE)
            {
                if (edgeTraceResult == SPHERE_MAY_HIT_V0)
                    v11 = v0;
                else
                    v11 = v2;
                vertToCheck = v11;
                isWalkable &= ((1 << ((3 * triIndex + 1) & 7)) & cm.triEdgeIsWalkable[(3 * triIndex + 1) >> 3]) != 0;
            }
        }
        if (projTriAreaScaledByTraceLenX2 > u - negativeV)
        {
            missedEdge = 1;
            Vec3Sub(v1, v2, v1_v2);
            edgeTraceResult = CM_TraceSphereThroughEdge(tw, sphereStart, v1, v1_v2, trace);
            if (edgeTraceResult == SPHERE_HITS_EDGE)
            {
                trace->walkable = ((1 << ((3 * triIndex) & 7)) & cm.triEdgeIsWalkable[(3 * triIndex) >> 3]) != 0;
                return;
            }
            if (edgeTraceResult != SPHERE_MISSES_EDGE)
            {
                if (edgeTraceResult == SPHERE_MAY_HIT_V0)
                    v10 = v1;
                else
                    v10 = v2;
                vertToCheck = v10;
                isWalkable &= ((1 << ((3 * triIndex) & 7)) & cm.triEdgeIsWalkable[(3 * triIndex) >> 3]) != 0;
            }
        }
        if (missedEdge)
        {
            if (vertToCheck)
                CM_TraceSphereThroughVertex(tw, isWalkable, sphereStart, vertToCheck, trace);
        }
        else
        {
            trace->normal[0] = normal[0];
            trace->normal[1] = normal[1];
            trace->normal[2] = normal[2];
            if (!Vec3IsNormalized(trace->normal))
            {
                v7 = Vec3Length(trace->normal);
                v4 = va("(%g %g %g) len %g", trace->normal[0], trace->normal[1], trace->normal[2], v7);
                MyAssertHandler(".\\qcommon\\cm_mesh.cpp", 530, 0, "%s\n\t%s", "Vec3IsNormalized( trace->normal )", v4);
            }
            trace->walkable = 0;
            v9 = 0.0 - hitFrac;
            if (v9 < 0.0)
                v8 = hitFrac;
            else
                v8 = 0.0;
            trace->fraction = v8;
            trace->startsolid = startSolid;
            if (trace->fraction < 0.0 || trace->fraction > 1.0)
                MyAssertHandler(
                    ".\\qcommon\\cm_mesh.cpp",
                    534,
                    1,
                    "%s\n\t(trace->fraction) = %g",
                    "(trace->fraction >= 0 && trace->fraction <= 1.0f)",
                    trace->fraction);
        }
    }
}

SphereEdgeTraceResult __cdecl CM_TraceSphereThroughEdge(
    const traceWork_t *tw,
    const float *sphereStart,
    const float *v0,
    const float *v0_v1,
    trace_t *trace)
{
    const char *v6; // eax
    float v7; // [esp+78h] [ebp-88h]
    float v8; // [esp+7Ch] [ebp-84h]
    float scale; // [esp+84h] [ebp-7Ch]
    float v10; // [esp+88h] [ebp-78h]
    float v11; // [esp+8Ch] [ebp-74h]
    float perpendicularLenSq; // [esp+94h] [ebp-6Ch]
    float t; // [esp+98h] [ebp-68h]
    float scaledProjectionDist; // [esp+9Ch] [ebp-64h]
    float scaledProjectionDista; // [esp+9Ch] [ebp-64h]
    float hitDelta[3]; // [esp+A0h] [ebp-60h] BYREF
    float discriminant; // [esp+ACh] [ebp-54h]
    float edgeLenSq; // [esp+B0h] [ebp-50h]
    float edgeCrossDelta[3]; // [esp+B4h] [ebp-4Ch] BYREF
    float radius; // [esp+C0h] [ebp-40h]
    float radiusSq; // [esp+C4h] [ebp-3Ch]
    float scaledNormal[3]; // [esp+C8h] [ebp-38h] BYREF
    float tScaled; // [esp+D4h] [ebp-2Ch]
    float fracEnter; // [esp+D8h] [ebp-28h]
    float startDelta[3]; // [esp+DCh] [ebp-24h] BYREF
    float fracLeave; // [esp+E8h] [ebp-18h]
    float f; // [esp+ECh] [ebp-14h]
    float perpendicular[3]; // [esp+F0h] [ebp-10h] BYREF
    float scaledDist; // [esp+FCh] [ebp-4h]

    Vec3Sub(sphereStart, v0, startDelta);
    Vec3Cross(v0_v1, tw->delta, perpendicular);
    scaledDist = Vec3Dot(startDelta, perpendicular);
    perpendicularLenSq = Vec3LengthSq(perpendicular);
    radius = tw->radius + 0.125;
    radiusSq = radius * radius;
    discriminant = radiusSq * perpendicularLenSq - scaledDist * scaledDist;
    if (discriminant <= 0.0)
        return SPHERE_MISSES_EDGE;
    edgeLenSq = Vec3LengthSq(v0_v1);
    v11 = discriminant * edgeLenSq;
    v10 = sqrt(v11);
    f = v10 / perpendicularLenSq;
    Vec3Cross(startDelta, v0_v1, edgeCrossDelta);
    tScaled = Vec3Dot(edgeCrossDelta, perpendicular);
    t = tScaled / perpendicularLenSq;
    fracLeave = t + f;
    if (fracLeave < 0.0)
        return SPHERE_MISSES_EDGE;
    fracEnter = t - f;
    if (trace->fraction <= (double)fracEnter)
        return SPHERE_MISSES_EDGE;
    if (fracEnter >= 0.0)
    {
        Vec3Mad(startDelta, fracEnter, tw->delta, hitDelta);
        scaledProjectionDista = -Vec3Dot(hitDelta, v0_v1);
        if (scaledProjectionDista > 0.0)
        {
            if (edgeLenSq > (double)scaledProjectionDista)
            {
                v8 = scaledProjectionDista / edgeLenSq;
                Vec3Mad(hitDelta, v8, v0_v1, scaledNormal);
                v7 = 1.0 / radius;
                Vec3Scale(scaledNormal, v7, trace->normal);
                if (!Vec3IsNormalizedEpsilon(trace->normal, 0.003f))
                {
                    v6 = va(
                        "(%g %g %g) from (%g %g %g) / %g;\n\t\tdelta (%g %g %g), scale %g / %g, edge (%g %g %g)",
                        trace->normal[0],
                        trace->normal[1],
                        trace->normal[2],
                        scaledNormal[0],
                        scaledNormal[1],
                        scaledNormal[2],
                        radius,
                        hitDelta[0],
                        hitDelta[1],
                        hitDelta[2],
                        scaledProjectionDista,
                        edgeLenSq,
                        *v0_v1,
                        v0_v1[1],
                        v0_v1[2]);
                    MyAssertHandler(
                        ".\\qcommon\\cm_mesh.cpp",
                        290,
                        0,
                        "%s\n\t%s",
                        "Vec3IsNormalizedEpsilon( trace->normal, TRACE_NORMAL_EPSILON )",
                        v6);
                }
                trace->fraction = fracEnter;
                return SPHERE_HITS_EDGE;
            }
            else
            {
                return SPHERE_MAY_HIT_V1;
            }
        }
        else
        {
            return SPHERE_MAY_HIT_V0;
        }
    }
    else
    {
        scaledProjectionDist = -Vec3Dot(startDelta, v0_v1);
        if (scaledProjectionDist > 0.0f)
        {
            if (edgeLenSq > scaledProjectionDist)
            {
                scale = scaledProjectionDist / edgeLenSq;
                Vec3Mad(startDelta, scale, v0_v1, scaledNormal);
                if (Vec3Dot(scaledNormal, tw->delta) < 0.0)
                {
                    Vec3NormalizeTo(scaledNormal, trace->normal);
                    trace->fraction = 0.0;
                    discriminant = tw->radius * tw->radius * perpendicularLenSq - scaledDist * scaledDist;
                    trace->startsolid = edgeLenSq * discriminant > tScaled * tScaled;
                    return SPHERE_HITS_EDGE;
                }
                else
                {
                    return SPHERE_MISSES_EDGE;
                }
            }
            else
            {
                return SPHERE_MAY_HIT_V1;
            }
        }
        else
        {
            return SPHERE_MAY_HIT_V0;
        }
    }
}

bool __cdecl Vec3IsNormalizedEpsilon(const float *v, float epsilon)
{
    float v3; // [esp+4h] [ebp-Ch]
    float v4; // [esp+8h] [ebp-8h]
    float v5; // [esp+Ch] [ebp-4h]

    v5 = Vec3LengthSq(v) - 1.0;
    v4 = epsilon * 2.0;
    v3 = I_fabs(v5);
    return v3 < (double)v4;
}

void __cdecl CM_TraceSphereThroughVertex(
    const traceWork_t *tw,
    bool isWalkable,
    const float *sphereStart,
    const float *v,
    trace_t *trace)
{
    const char *v5; // eax
    double v6; // [esp+18h] [ebp-6Ch]
    double v7; // [esp+20h] [ebp-64h]
    float v8; // [esp+30h] [ebp-54h]
    float scale; // [esp+34h] [ebp-50h]
    float v10; // [esp+38h] [ebp-4Ch]
    float v11; // [esp+3Ch] [ebp-48h]
    float v12; // [esp+40h] [ebp-44h]
    float v13; // [esp+4Ch] [ebp-38h]
    float delta[3]; // [esp+54h] [ebp-30h] BYREF
    float deltaLenSq; // [esp+60h] [ebp-24h]
    float c; // [esp+64h] [ebp-20h]
    float frac; // [esp+68h] [ebp-1Ch]
    float discriminant; // [esp+6Ch] [ebp-18h]
    float b; // [esp+70h] [ebp-14h]
    float bSquared; // [esp+74h] [ebp-10h]
    float approxRecipLen; // [esp+78h] [ebp-Ch]
    float a; // [esp+7Ch] [ebp-8h]
    float lenSq; // [esp+80h] [ebp-4h]

    Vec3Sub(sphereStart, v, delta);
    b = Vec3Dot(tw->delta, delta);
    if (b < 0.0)
    {
        deltaLenSq = Vec3Dot(delta, delta);
        iassert( (deltaLenSq > 0.0f) );
        c = deltaLenSq - (tw->radius + 0.125) * (tw->radius + 0.125);
        if (c > 0.0)
        {
            a = tw->deltaLenSq;
            iassert( (a > 0.0f) );
            bSquared = b * b;
            discriminant = bSquared - a * c;
            if (discriminant >= bSquared * EQUAL_EPSILON)
            {
                v10 = sqrt(discriminant);
                frac = (-v10 - b) / a;
                if (trace->fraction > frac)
                {
                    Vec3Mad(delta, frac, tw->delta, trace->normal);
                    scale = 1.0f / (tw->radius + 0.125f);
                    Vec3Scale(trace->normal, scale, trace->normal);
                    lenSq = Vec3LengthSq(trace->normal);
                    approxRecipLen = (3.0f - lenSq) * 0.5f;
                    Vec3Scale(trace->normal, approxRecipLen, trace->normal);
                    if (!Vec3IsNormalizedEpsilon(trace->normal, 0.003f) && !alwaysfails)
                    {
                        v13 = Vec3LengthSq(trace->normal) - 1.0f;
                        v8 = I_fabs(v13);
                        v7 = tw->radius + 0.125f;
                        v6 = Vec3Length(trace->normal);
                        v5 = va(
                            "Vec3IsNormalized (%g %g %g) %g, %g %g",
                            trace->normal[0],
                            trace->normal[1],
                            trace->normal[2],
                            v6,
                            v7,
                            v8 / 2.0f);
                        MyAssertHandler(".\\qcommon\\cm_mesh.cpp", 385, 0, v5);
                    }
                    trace->walkable = isWalkable;
                    trace->fraction = frac;
                    if (trace->fraction < 0.0f || trace->fraction > 1.0f)
                        MyAssertHandler(
                            ".\\qcommon\\cm_mesh.cpp",
                            390,
                            1,
                            "%s\n\t(trace->fraction) = %g",
                            "(trace->fraction >= 0 && trace->fraction <= 1.0f)",
                            trace->fraction);
                }
            }
        }
        else
        {
            v12 = sqrt(deltaLenSq);
            frac = 1.0f / v12;
            Vec3Scale(delta, frac, trace->normal);
            trace->walkable = isWalkable;
            trace->fraction = 0.0f;
            v11 = tw->radius * tw->radius;
            if (deltaLenSq < v11)
                trace->startsolid = 1;
        }
    }
}

void __cdecl CM_TraceCapsuleThroughBorder(const traceWork_t *tw, CollisionBorder *border, trace_t *trace)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    double offsetZ; // [esp+18h] [ebp-D4h]
    double offsetZa; // [esp+18h] [ebp-D4h]
    float offsetZb; // [esp+18h] [ebp-D4h]
    double offsetZc; // [esp+18h] [ebp-D4h]
    float v13; // [esp+20h] [ebp-CCh]
    float v14; // [esp+24h] [ebp-C8h]
    float v15; // [esp+28h] [ebp-C4h]
    float v16; // [esp+2Ch] [ebp-C0h]
    float v17; // [esp+30h] [ebp-BCh]
    float v18; // [esp+34h] [ebp-B8h]
    float v19; // [esp+38h] [ebp-B4h]
    float v20; // [esp+3Ch] [ebp-B0h]
    float v21; // [esp+40h] [ebp-ACh]
    float v22; // [esp+54h] [ebp-98h]
    float v23; // [esp+6Ch] [ebp-80h]
    float v24; // [esp+74h] [ebp-78h]
    float v25; // [esp+90h] [ebp-5Ch]
    float v26; // [esp+98h] [ebp-54h]
    float edgeZ; // [esp+A4h] [ebp-48h]
    float edgePoint; // [esp+A8h] [ebp-44h]
    float edgePointa; // [esp+A8h] [ebp-44h]
    float edgePoint_4; // [esp+ACh] [ebp-40h]
    float edgePoint_4a; // [esp+ACh] [ebp-40h]
    float edgePoint_8; // [esp+B0h] [ebp-3Ch]
    float edgePoint_8a; // [esp+B0h] [ebp-3Ch]
    float traceDeltaDot; // [esp+B4h] [ebp-38h]
    float t; // [esp+B8h] [ebp-34h]
    float c; // [esp+BCh] [ebp-30h]
    float ca; // [esp+BCh] [ebp-30h]
    float deltaDotOffset; // [esp+C0h] [ebp-2Ch]
    float deltaDotOffseta; // [esp+C0h] [ebp-2Ch]
    float discriminant; // [esp+C4h] [ebp-28h]
    float discriminanta; // [esp+C4h] [ebp-28h]
    float radius; // [esp+C8h] [ebp-24h]
    float traceStartDist; // [esp+CCh] [ebp-20h]
    float offset; // [esp+D0h] [ebp-1Ch]
    float offseta; // [esp+D0h] [ebp-1Ch]
    float offset_4; // [esp+D4h] [ebp-18h]
    float offset_4a; // [esp+D4h] [ebp-18h]
    float endpos[3]; // [esp+D8h] [ebp-14h] BYREF
    float s; // [esp+E4h] [ebp-8h]
    float offsetLenSq; // [esp+E8h] [ebp-4h]

    traceDeltaDot = border->distEq[1] * tw->delta[1] + border->distEq[0] * tw->delta[0];
    if (traceDeltaDot >= 0.0)
        return;
    radius = tw->radius + 0.125;
    v21 = border->distEq[1] * tw->extents.start[1] + border->distEq[0] * tw->extents.start[0];
    traceStartDist = v21 - border->distEq[2];
    t = (radius - traceStartDist) / traceDeltaDot;
    if (trace->fraction <= (double)t || -radius > t * tw->deltaLen)
        return;
    Vec3Mad(tw->extents.start, t, tw->delta, endpos);
    s = border->distEq[1] * endpos[0] - border->distEq[0] * endpos[1] - border->start;
    if (s < 0.0)
    {
        edgePoint = border->distEq[1] * border->start + border->distEq[0] * border->distEq[2];
        edgePoint_4 = border->distEq[1] * border->distEq[2] - border->distEq[0] * border->start;
        offset = tw->extents.start[0] - edgePoint;
        offset_4 = tw->extents.start[1] - edgePoint_4;
        deltaDotOffset = offset_4 * tw->delta[1] + offset * tw->delta[0];
        if (deltaDotOffset >= 0.0)
            return;
        offsetLenSq = offset_4 * offset_4 + offset * offset;
        c = offsetLenSq - radius * radius;
        if (c < 0.0)
        {
            edgePoint_8 = border->zBase;
            if (tw->offsetZ != tw->size[2] - tw->radius)
            {
                v3 = va("tw->offsetZ: %f, tw->size[2]: %f, tw->radius: %f", tw->offsetZ, tw->size[2], tw->radius);
                MyAssertHandler(".\\qcommon\\cm_mesh.cpp", 1209, 0, "%s\n\t%s", "tw->offsetZ == tw->size[2] - tw->radius", v3);
            }
            iassert( tw->offsetZ >= 0 );
            v26 = edgePoint_8 - tw->extents.start[2];
            v20 = I_fabs(v26);
            if (tw->offsetZ >= (double)v20)
            {
                v25 = border->distEq[1];
                trace->normal[0] = border->distEq[0];
                trace->normal[1] = v25;
                trace->normal[2] = 0.0;
                if (!Vec3IsNormalized(trace->normal))
                {
                    offsetZ = Vec3Length(trace->normal);
                    v4 = va("(%g %g %g) len %g", trace->normal[0], trace->normal[1], trace->normal[2], offsetZ);
                    MyAssertHandler(".\\qcommon\\cm_mesh.cpp", 1215, 0, "%s\n\t%s", "Vec3IsNormalized( trace->normal )", v4);
                }
                trace->walkable = 0;
                trace->fraction = 0.0;
                v19 = tw->radius * tw->radius;
                if (offsetLenSq < (double)v19)
                    trace->startsolid = 1;
            }
            return;
        }
        discriminant = deltaDotOffset * deltaDotOffset - tw->deltaLenSq * c;
        if (discriminant < 0.0)
            return;
        if (tw->deltaLenSq <= 0.0)
            MyAssertHandler(
                ".\\qcommon\\cm_mesh.cpp",
                1227,
                0,
                "%s\n\t(tw->deltaLenSq) = %g",
                "(tw->deltaLenSq > 0.0f)",
                tw->deltaLenSq);
        v18 = sqrt(discriminant);
        t = (-deltaDotOffset - v18) / tw->deltaLenSq;
        if (trace->fraction <= (double)t || t <= 0.0)
            return;
        Vec3Mad(tw->extents.start, t, tw->delta, endpos);
        s = 0.0;
    LABEL_47:
        edgeZ = s * border->zSlope + border->zBase - endpos[2];
        if (tw->offsetZ != tw->size[2] - tw->radius)
        {
            v7 = va("tw->offsetZ: %f, tw->size[2]: %f, tw->radius: %f", tw->offsetZ, tw->size[2], tw->radius);
            MyAssertHandler(".\\qcommon\\cm_mesh.cpp", 1278, 0, "%s\n\t%s", "tw->offsetZ == tw->size[2] - tw->radius", v7);
        }
        iassert( tw->offsetZ >= 0 );
        if (tw->offsetZ >= (double)edgeZ)
        {
            if (edgeZ >= -tw->offsetZ)
            {
                trace->fraction = t;
                if (trace->fraction < 0.0 || trace->fraction > 1.0)
                    MyAssertHandler(
                        ".\\qcommon\\cm_mesh.cpp",
                        1293,
                        1,
                        "%s\n\t(trace->fraction) = %g",
                        "(trace->fraction >= 0 && trace->fraction <= 1.0f)",
                        trace->fraction);
                trace->walkable = 0;
                v22 = border->distEq[1];
                trace->normal[0] = border->distEq[0];
                trace->normal[1] = v22;
                trace->normal[2] = 0.0;
                if (!Vec3IsNormalized(trace->normal))
                {
                    offsetZc = Vec3Length(trace->normal);
                    v8 = va("(%g %g %g) len %g", trace->normal[0], trace->normal[1], trace->normal[2], offsetZc);
                    MyAssertHandler(".\\qcommon\\cm_mesh.cpp", 1296, 0, "%s\n\t%s", "Vec3IsNormalized( trace->normal )", v8);
                }
            }
            else
            {
                v13 = -tw->offsetZ - tw->radius;
                if (edgeZ > (double)v13)
                {
                    offsetZb = -tw->offsetZ;
                    CM_TraceSphereThroughBorder(tw, border, offsetZb, trace);
                }
            }
        }
        else
        {
            v14 = tw->offsetZ + tw->radius;
            if (edgeZ < (double)v14)
                CM_TraceSphereThroughBorder(tw, border, tw->offsetZ, trace);
        }
        return;
    }
    if (border->length >= (double)s)
    {
        if (t < 0.0)
            t = 0.0;
        goto LABEL_47;
    }
    edgePointa = (border->start + border->length) * border->distEq[1] + border->distEq[0] * border->distEq[2];
    edgePoint_4a = border->distEq[1] * border->distEq[2] - (border->start + border->length) * border->distEq[0];
    offseta = tw->extents.start[0] - edgePointa;
    offset_4a = tw->extents.start[1] - edgePoint_4a;
    deltaDotOffseta = offset_4a * tw->delta[1] + offseta * tw->delta[0];
    if (deltaDotOffseta >= 0.0)
        return;
    offsetLenSq = offset_4a * offset_4a + offseta * offseta;
    ca = offsetLenSq - radius * radius;
    if (ca < 0.0)
    {
        edgePoint_8a = border->zSlope * border->length + border->zBase;
        if (tw->offsetZ != tw->size[2] - tw->radius)
        {
            v5 = va("tw->offsetZ: %f, tw->size[2]: %f, tw->radius: %f", tw->offsetZ, tw->size[2], tw->radius);
            MyAssertHandler(".\\qcommon\\cm_mesh.cpp", 1248, 0, "%s\n\t%s", "tw->offsetZ == tw->size[2] - tw->radius", v5);
        }
        iassert( tw->offsetZ >= 0 );
        v24 = tw->extents.start[2] - edgePoint_8a;
        v17 = I_fabs(v24);
        if (tw->offsetZ >= (double)v17)
        {
            v23 = border->distEq[1];
            trace->normal[0] = border->distEq[0];
            trace->normal[1] = v23;
            trace->normal[2] = 0.0;
            if (!Vec3IsNormalized(trace->normal))
            {
                offsetZa = Vec3Length(trace->normal);
                v6 = va("(%g %g %g) len %g", trace->normal[0], trace->normal[1], trace->normal[2], offsetZa);
                MyAssertHandler(".\\qcommon\\cm_mesh.cpp", 1254, 0, "%s\n\t%s", "Vec3IsNormalized( trace->normal )", v6);
            }
            trace->walkable = 0;
            trace->fraction = 0.0;
            v16 = tw->radius * tw->radius;
            if (offsetLenSq < (double)v16)
                trace->startsolid = 1;
        }
        return;
    }
    discriminanta = deltaDotOffseta * deltaDotOffseta - tw->deltaLenSq * ca;
    if (discriminanta >= 0.0)
    {
        if (tw->deltaLenSq <= 0.0)
            MyAssertHandler(
                ".\\qcommon\\cm_mesh.cpp",
                1265,
                0,
                "%s\n\t(tw->deltaLenSq) = %g",
                "(tw->deltaLenSq > 0.0f)",
                tw->deltaLenSq);
        v15 = sqrt(discriminanta);
        t = (-deltaDotOffseta - v15) / tw->deltaLenSq;
        if (trace->fraction > (double)t && t > 0.0)
        {
            Vec3Mad(tw->extents.start, t, tw->delta, endpos);
            s = border->length;
            goto LABEL_47;
        }
    }
}

void __cdecl CM_TraceSphereThroughBorder(
    const traceWork_t *tw,
    const CollisionBorder *border,
    float offsetZ,
    trace_t *trace)
{
    float v0[3]; // [esp+3Ch] [ebp-34h] BYREF
    SphereEdgeTraceResult result; // [esp+48h] [ebp-28h]
    float sphereStart[3]; // [esp+4Ch] [ebp-24h] BYREF
    float v1[3]; // [esp+58h] [ebp-18h] BYREF
    float v0_v1[3]; // [esp+64h] [ebp-Ch] BYREF

    v0[0] = border->distEq[1] * border->start + border->distEq[0] * border->distEq[2];
    v0[1] = border->distEq[1] * border->distEq[2] - border->distEq[0] * border->start;
    v0[2] = border->zBase;
    v1[0] = (border->start + border->length) * border->distEq[1] + border->distEq[0] * border->distEq[2];
    v1[1] = border->distEq[1] * border->distEq[2] - (border->start + border->length) * border->distEq[0];
    v1[2] = border->zSlope * border->length + border->zBase;
    Vec3Sub(v0, v1, v0_v1);
    sphereStart[0] = tw->extents.start[0];
    sphereStart[1] = tw->extents.start[1];
    sphereStart[2] = tw->extents.start[2];
    sphereStart[2] = sphereStart[2] + offsetZ;
    result = CM_TraceSphereThroughEdge(tw, sphereStart, v0, v0_v1, trace);
    if (result == SPHERE_MAY_HIT_V0)
    {
        CM_TraceSphereThroughVertex(tw, 0, sphereStart, v0, trace);
    }
    else if (result == SPHERE_MAY_HIT_V1)
    {
        CM_TraceSphereThroughVertex(tw, 0, sphereStart, v1, trace);
    }
}

void __cdecl CM_SightTraceThroughAabbTree(const traceWork_t *tw, const CollisionAabbTree *aabbTree, trace_t *trace)
{
    if ((cm.materials[aabbTree->materialIndex].contentFlags & tw->contents) != 0)
        CM_TraceThroughAabbTree(tw, aabbTree, trace);
}

void __cdecl CM_MeshTestInLeaf(const traceWork_t *tw, cLeaf_t *leaf, trace_t *trace)
{
    dmaterial_t *materialInfo; // [esp+0h] [ebp-Ch]
    int k; // [esp+4h] [ebp-8h]
    CollisionAabbTree *aabbTree; // [esp+8h] [ebp-4h]

    iassert( !tw->isPoint );
    iassert( !trace->allsolid );
    for (k = 0; k < leaf->collAabbCount; ++k)
    {
        aabbTree = &cm.aabbTrees[k + leaf->firstCollAabbIndex];
        materialInfo = &cm.materials[aabbTree->materialIndex];
        if ((materialInfo->contentFlags & tw->contents) != 0)
        {
            CM_PositionTestInAabbTree_r(tw, aabbTree, trace);
            if (trace->allsolid)
            {
                trace->surfaceFlags = materialInfo->surfaceFlags;
                trace->contents = materialInfo->contentFlags;
                trace->material = (const char *)materialInfo;
                return;
            }
        }
    }
}

void __cdecl CM_PositionTestInAabbTree_r(const traceWork_t *tw, CollisionAabbTree *aabbTree, trace_t *trace)
{
    int childIndex; // [esp+0h] [ebp-1Ch]
    int triCount; // [esp+4h] [ebp-18h]
    unsigned __int16 *indices; // [esp+8h] [ebp-14h]
    int partitionIndex; // [esp+Ch] [ebp-10h]
    CollisionAabbTree *child; // [esp+10h] [ebp-Ch]
    int checkStamp; // [esp+14h] [ebp-8h]

    if (!CM_CullBox(tw, aabbTree->origin, aabbTree->halfSize))
    {
        if (aabbTree->childCount)
        {
            childIndex = 0;
            for (child = &cm.aabbTrees[aabbTree->u.firstChildIndex]; childIndex < aabbTree->childCount; ++child)
            {
                CM_PositionTestInAabbTree_r(tw, child, trace);
                if (trace->startsolid)
                    break;
                ++childIndex;
            }
        }
        else
        {
            partitionIndex = aabbTree->u.firstChildIndex;
            checkStamp = SLOWORD(tw->threadInfo.checkcount.global);
            if (tw->threadInfo.checkcount.partitions[partitionIndex] != checkStamp)
            {
                tw->threadInfo.checkcount.partitions[partitionIndex] = checkStamp;
                indices = &cm.triIndices[3 * cm.partitions[partitionIndex].firstTri];
                for (triCount = cm.partitions[partitionIndex].triCount; triCount; --triCount)
                {
                    CM_PositionTestCapsuleInTriangle(tw, indices, trace);
                    if (trace->startsolid)
                        break;
                    indices += 3;
                }
            }
        }
    }
}

void __cdecl CM_PositionTestCapsuleInTriangle(const traceWork_t *tw, const unsigned __int16 *indices, trace_t *trace)
{
    float v3; // [esp+8h] [ebp-3Ch]
    float radiusSq; // [esp+Ch] [ebp-38h]
    float v5; // [esp+14h] [ebp-30h]
    float v6; // [esp+18h] [ebp-2Ch]
    float v7; // [esp+20h] [ebp-24h]
    float v8; // [esp+24h] [ebp-20h]
    float start[3]; // [esp+28h] [ebp-1Ch] BYREF
    float end[3]; // [esp+34h] [ebp-10h] BYREF
    float distSq; // [esp+40h] [ebp-4h]

    if (tw->offsetZ == 0.0)
    {
        distSq = CM_DistanceSquaredFromPointToTriangle(tw->extents.start, indices);
        v3 = tw->radius * tw->radius;
        if (distSq < (double)v3)
        {
            trace->fraction = 0.0;
            trace->startsolid = 1;
            trace->allsolid = 1;
        }
    }
    else
    {
        v7 = tw->extents.start[1];
        v8 = tw->offsetZ + tw->extents.start[2];
        start[0] = tw->extents.start[0];
        start[1] = v7;
        start[2] = v8;
        v5 = tw->extents.start[1];
        v6 = tw->extents.start[2] - tw->offsetZ;
        end[0] = tw->extents.start[0];
        end[1] = v5;
        end[2] = v6;
        radiusSq = tw->radius * tw->radius;
        if (CM_DoesCapsuleIntersectTriangle(start, end, radiusSq, indices))
        {
            trace->fraction = 0.0;
            trace->startsolid = 1;
            trace->allsolid = 1;
        }
    }
}

double __cdecl CM_DistanceSquaredFromPointToTriangle(const float *pt, const unsigned __int16 *indices)
{
    float diff[3]; // [esp+10h] [ebp-48h] BYREF
    const float *v0; // [esp+1Ch] [ebp-3Ch]
    float a01; // [esp+20h] [ebp-38h]
    float e1[3]; // [esp+24h] [ebp-34h] BYREF
    float a11; // [esp+30h] [ebp-28h]
    float a00; // [esp+34h] [ebp-24h]
    const float *v2; // [esp+38h] [ebp-20h]
    const float *v1; // [esp+3Ch] [ebp-1Ch]
    float e0[3]; // [esp+40h] [ebp-18h] BYREF
    float ptOnTri[3]; // [esp+4Ch] [ebp-Ch] BYREF

    v0 = cm.verts[*indices];
    v1 = cm.verts[indices[1]];
    v2 = cm.verts[indices[2]];
    Vec3Sub(v1, v0, e0);
    Vec3Sub(v2, v0, e1);
    a00 = Vec3Dot(e0, e0);
    a01 = Vec3Dot(e0, e1);
    a11 = Vec3Dot(e1, e1);
    CM_ClosestPointOnTri(pt, v0, e0, e1, a00, a01, a11, ptOnTri);
    Vec3Sub(ptOnTri, pt, diff);
    return Vec3LengthSq(diff);
}

void __cdecl CM_ClosestPointOnTri(
    const float *pt,
    const float *v0,
    const float *e0,
    const float *e1,
    float a00,
    float a01,
    float a11,
    float *ptOnTri)
{
    float b1; // [esp+14h] [ebp-30h]
    float invDet; // [esp+18h] [ebp-2Ch]
    float det; // [esp+20h] [ebp-24h]
    float u; // [esp+24h] [ebp-20h]
    float ua; // [esp+24h] [ebp-20h]
    float ub; // [esp+24h] [ebp-20h]
    float uc; // [esp+24h] [ebp-20h]
    float v; // [esp+28h] [ebp-1Ch]
    float va; // [esp+28h] [ebp-1Ch]
    float vc; // [esp+28h] [ebp-1Ch]
    float vb; // [esp+28h] [ebp-1Ch]
    float denom; // [esp+2Ch] [ebp-18h]
    float v0_pt[3]; // [esp+30h] [ebp-14h] BYREF
    float b0; // [esp+3Ch] [ebp-8h]
    float numer; // [esp+40h] [ebp-4h]

    Vec3Sub(pt, v0, v0_pt);
    b0 = Vec3Dot(e0, v0_pt);
    b1 = Vec3Dot(e1, v0_pt);
    Vec3Dot(v0_pt, v0_pt);
    det = a00 * a11 - a01 * a01;
    u = a01 * b1 - a11 * b0;
    v = a01 * b0 - a00 * b1;
    if (det < u + v)
    {
        if (u >= 0.0)
        {
            if (v < 0.0)
            {
                ua = 1.0;
                va = 0.0;
                if (a00 + b0 > 0.0)
                    goto region_5;
                if (a00 + b0 <= a01 + b1)
                    goto LABEL_30;
            }
        }
        else
        {
            ua = 0.0;
            va = 1.0;
            if (a11 + b1 > 0.0)
            {
            region_3:
                ub = 0.0;
                if (b1 < 0.0)
                {
                    if (a11 > -b1)
                    {
                        vc = -b1 / a11;
                        Vec3MadMad(v0, ub, e0, vc, e1, ptOnTri);
                    }
                    else
                    {
                        Vec3MadMad(v0, ub, e0, 1.0, e1, ptOnTri);
                    }
                }
                else
                {
                    Vec3MadMad(v0, ub, e0, 0.0, e1, ptOnTri);
                }
                return;
            }
            if (a11 + b1 <= a01 + b0)
            {
            LABEL_30:
                Vec3MadMad(v0, ua, e0, va, e1, ptOnTri);
                return;
            }
        }
        numer = a11 - a01 + b1 - b0;
        if (numer < 0.0)
        {
            Vec3MadMad(v0, 0.0, e0, 1.0, e1, ptOnTri);
            return;
        }
        denom = a00 - a01 * 2.0 + a11;
        if (denom <= (double)numer)
        {
            Vec3MadMad(v0, 1.0, e0, 0.0, e1, ptOnTri);
            return;
        }
        ua = numer / denom;
        va = 1.0 - ua;
        goto LABEL_30;
    }
    if (u >= 0.0)
    {
        if (v >= 0.0)
        {
            invDet = 1.0 / det;
            ua = u * invDet;
            va = v * invDet;
            goto LABEL_30;
        }
    }
    else
    {
        if (v >= 0.0)
            goto region_3;
        ua = 0.0;
        va = 0.0;
        if (b0 >= 0.0)
        {
            if (b1 < 0.0)
                goto region_3;
            goto LABEL_30;
        }
    }
region_5:
    vb = 0.0;
    if (b0 < 0.0)
    {
        if (a00 > -b0)
        {
            uc = -b0 / a00;
            Vec3MadMad(v0, uc, e0, vb, e1, ptOnTri);
        }
        else
        {
            Vec3MadMad(v0, 1.0, e0, vb, e1, ptOnTri);
        }
    }
    else
    {
        Vec3MadMad(v0, 0.0, e0, vb, e1, ptOnTri);
    }
}

bool __cdecl CM_DoesCapsuleIntersectTriangle(
    const float *start,
    const float *end,
    float radiusSq,
    const unsigned __int16 *indices)
{
    double v4; // st7
    double v5; // st7
    bool v7; // [esp+8h] [ebp-160h]
    bool v8; // [esp+Ch] [ebp-15Ch]
    bool v9; // [esp+20h] [ebp-148h]
    float fraction; // [esp+24h] [ebp-144h]
    bool v11; // [esp+2Bh] [ebp-13Dh]
    double v12; // [esp+2Ch] [ebp-13Ch]
    double v13; // [esp+34h] [ebp-134h]
    double v14; // [esp+3Ch] [ebp-12Ch]
    float v15[3]; // [esp+44h] [ebp-124h] BYREF
    double v16; // [esp+50h] [ebp-118h]
    bool v17; // [esp+5Bh] [ebp-10Dh]
    double v18; // [esp+5Ch] [ebp-10Ch]
    double v19; // [esp+64h] [ebp-104h]
    double v20; // [esp+6Ch] [ebp-FCh]
    float b[3]; // [esp+74h] [ebp-F4h] BYREF
    double v22; // [esp+80h] [ebp-E8h]
    bool v23; // [esp+8Bh] [ebp-DDh]
    double v24; // [esp+8Ch] [ebp-DCh]
    double v25; // [esp+94h] [ebp-D4h]
    double v26; // [esp+9Ch] [ebp-CCh]
    float diff[3]; // [esp+A4h] [ebp-C4h] BYREF
    double v28; // [esp+B0h] [ebp-B8h]
    const float *v0; // [esp+B8h] [ebp-B0h]
    float v2_v0[3]; // [esp+BCh] [ebp-ACh] BYREF
    double a01; // [esp+C8h] [ebp-A0h]
    float scaledPlaneNormal[3]; // [esp+D0h] [ebp-98h] BYREF
    bool isEndCloseEnough; // [esp+DFh] [ebp-89h]
    float distSqToEdge; // [esp+E0h] [ebp-88h]
    float delta[3]; // [esp+E4h] [ebp-84h] BYREF
    double a11; // [esp+F0h] [ebp-78h]
    double scaledPlaneDist; // [esp+F8h] [ebp-70h]
    bool isStartCloseEnough; // [esp+107h] [ebp-61h]
    double a00; // [esp+108h] [ebp-60h]
    double det; // [esp+110h] [ebp-58h]
    const float *v2; // [esp+11Ch] [ebp-4Ch]
    float v1_v0[3]; // [esp+120h] [ebp-48h] BYREF
    const float *v1; // [esp+12Ch] [ebp-3Ch]
    float v2_v1[3]; // [esp+130h] [ebp-38h] BYREF
    float pt[3]; // [esp+13Ch] [ebp-2Ch] BYREF
    double cutoffDistSq; // [esp+148h] [ebp-20h]
    double scaleSq; // [esp+150h] [ebp-18h]
    long double scaledDist[2]; // [esp+158h] [ebp-10h]

    v0 = cm.verts[*indices];
    v1 = cm.verts[indices[1]];
    v2 = cm.verts[indices[2]];
    Vec3Sub(v1, v0, v1_v0);
    Vec3Sub(v2, v0, v2_v0);
    Vec3Cross(v1_v0, v2_v0, scaledPlaneNormal);
    scaledPlaneDist = Vec3Dot(v0, scaledPlaneNormal);
    v4 = Vec3Dot(start, scaledPlaneNormal);
    scaledDist[0] = v4 - scaledPlaneDist;
    v5 = Vec3Dot(end, scaledPlaneNormal);
    scaledDist[1] = v5 - scaledPlaneDist;
    if (scaledDist[0] * scaledDist[1] >= 0.0)
    {
        scaleSq = Vec3LengthSq(scaledPlaneNormal);
        cutoffDistSq = radiusSq * scaleSq;
        isStartCloseEnough = scaledDist[0] * scaledDist[0] < cutoffDistSq;
        isEndCloseEnough = scaledDist[1] * scaledDist[1] < cutoffDistSq;
        if (!isEndCloseEnough && !isStartCloseEnough)
            return 0;
        a00 = Vec3Dot(v1_v0, v1_v0);
        a01 = Vec3Dot(v1_v0, v2_v0);
        a11 = Vec3Dot(v2_v0, v2_v0);
        det = a00 * a11 - a01 * a01;
    }
    else
    {
        a00 = Vec3Dot(v1_v0, v1_v0);
        a01 = Vec3Dot(v1_v0, v2_v0);
        a11 = Vec3Dot(v2_v0, v2_v0);
        det = a00 * a11 - a01 * a01;
        fraction = scaledDist[0] / (scaledDist[0] - scaledDist[1]);
        Vec3Lerp(start, end, fraction, pt);
        Vec3Sub(v0, pt, diff);
        v28 = Vec3Dot(v1_v0, diff);
        v24 = Vec3Dot(v2_v0, diff);
        v25 = a01 * v24 - a11 * v28;
        if (v25 >= 0.0)
        {
            v26 = a01 * v28 - a00 * v24;
            v9 = v26 >= 0.0 && v25 + v26 <= det;
            v23 = v9;
        }
        else
        {
            v23 = 0;
        }
        if (v23)
            return 1;
        scaleSq = Vec3LengthSq(scaledPlaneNormal);
        cutoffDistSq = radiusSq * scaleSq;
        isStartCloseEnough = scaledDist[0] * scaledDist[0] < cutoffDistSq;
        isEndCloseEnough = scaledDist[1] * scaledDist[1] < cutoffDistSq;
    }
    if (isStartCloseEnough)
    {
        Vec3Sub(v0, start, b);
        v22 = Vec3Dot(v1_v0, b);
        v18 = Vec3Dot(v2_v0, b);
        v19 = a01 * v18 - a11 * v22;
        if (v19 >= 0.0)
        {
            v20 = a01 * v22 - a00 * v18;
            v8 = v20 >= 0.0 && v19 + v20 <= det;
            v17 = v8;
        }
        else
        {
            v17 = 0;
        }
        if (v17)
            return 1;
    }
    if (isEndCloseEnough)
    {
        Vec3Sub(v0, end, v15);
        v16 = Vec3Dot(v1_v0, v15);
        v12 = Vec3Dot(v2_v0, v15);
        v13 = a01 * v12 - a11 * v16;
        if (v13 >= 0.0)
        {
            v14 = a01 * v16 - a00 * v12;
            v7 = v14 >= 0.0 && v13 + v14 <= det;
            v11 = v7;
        }
        else
        {
            v11 = 0;
        }
        if (v11)
            return 1;
    }
    Vec3Sub(end, start, delta);
    distSqToEdge = CM_DistanceSquaredBetweenSegments(start, delta, v0, v1_v0);
    if (radiusSq > (double)distSqToEdge)
        return 1;
    distSqToEdge = CM_DistanceSquaredBetweenSegments(start, delta, v0, v2_v0);
    if (radiusSq > (double)distSqToEdge)
        return 1;
    Vec3Sub(v2, v1, v2_v1);
    distSqToEdge = CM_DistanceSquaredBetweenSegments(start, delta, v1, v2_v1);
    return radiusSq > (double)distSqToEdge;
}

double __cdecl CM_DistanceSquaredBetweenSegments(
    const float *start0,
    const float *delta0,
    const float *start1,
    const float *delta1)
{
    double b1; // [esp+24h] [ebp-78h]
    double invDet; // [esp+2Ch] [ebp-70h]
    double a01; // [esp+3Ch] [ebp-60h]
    double c; // [esp+44h] [ebp-58h]
    double a11; // [esp+5Ch] [ebp-40h]
    double numer0; // [esp+64h] [ebp-38h]
    double a00; // [esp+6Ch] [ebp-30h]
    double det; // [esp+74h] [ebp-28h]
    float s0_s1[3]; // [esp+80h] [ebp-1Ch] BYREF
    double numer1; // [esp+8Ch] [ebp-10h]
    double b0; // [esp+94h] [ebp-8h]

    a00 = Vec3Dot(delta0, delta0);
    a01 = Vec3Dot(delta0, delta1);
    a11 = Vec3Dot(delta1, delta1);
    Vec3Sub(start0, start1, s0_s1);
    b0 = Vec3Dot(delta0, s0_s1);
    b1 = Vec3Dot(delta1, s0_s1);
    c = Vec3Dot(s0_s1, s0_s1);
    det = a00 * a11 - a01 * a01;
    numer0 = b1 * a01 - b0 * a11;
    if (numer0 > 0.0)
    {
        if (det > numer0)
        {
            numer1 = b1 * a00 - b0 * a01;
            if (numer1 > 0.0)
            {
                if (det > numer1)
                {
                    invDet = 1.0 / det;
                    return (float)((b0 - a01 * (numer1 * invDet) + b0 - a01 * (numer1 * invDet) + a00 * (numer0 * invDet))
                        * (numer0
                            * invDet)
                        + (a11 * (numer1 * invDet) - (b1 + b1)) * (numer1 * invDet)
                        + c);
                }
                goto t1_is_clamped_to_1;
            }
        }
        else
        {
            numer1 = b1 + a01;
            if (numer1 > 0.0)
            {
                if (a11 > numer1)
                    return (float)((a11 * (numer1 / a11) - (a01 + b1 + a01 + b1)) * (numer1 / a11) + a00 + b0 + b0 + c);
                goto t1_is_clamped_to_1;
            }
        }
    t1_is_clamped_to_0:
        if (b0 < 0.0)
        {
            if (-b0 < a00)
                return (float)((b0 + b0 + a00 * (-b0 / a00)) * (-b0 / a00) + c);
            else
                return (float)(b0 + b0 + a00 + c);
        }
        else
        {
            return (float)c;
        }
    }
    if (b1 <= 0.0)
        goto t1_is_clamped_to_0;
    if (a11 > b1)
        return (float)((a11 * (b1 / a11) - (b1 + b1)) * (b1 / a11) + c);
t1_is_clamped_to_1:
    if (b0 < a01)
    {
        if (a01 - b0 < a00)
            return (float)((a00 * ((a01 - b0) / a00) + b0 - a01 + b0 - a01) * ((a01 - b0) / a00) + a11 - (b1 + b1) + c);
        else
            return (float)(a00 + a11 + b0 - b1 - a01 + b0 - b1 - a01 + c);
    }
    else
    {
        return (float)(a11 - (b1 + b1) + c);
    }
}

