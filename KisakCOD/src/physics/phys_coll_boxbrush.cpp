#include "phys_local.h"
#include <cgame/cg_local.h>


void __cdecl Phys_DrawPoly(const Poly *poly, const float *color)
{
    unsigned int edgeIndex; // [esp+0h] [ebp-8h]
    unsigned int lastEdgeIndex; // [esp+4h] [ebp-4h]

    lastEdgeIndex = poly->ptCount - 1;
    for (edgeIndex = 0; edgeIndex < poly->ptCount; ++edgeIndex)
    {
        CG_DebugLine(poly->pts[lastEdgeIndex], poly->pts[edgeIndex], color, 0, 8);
        lastEdgeIndex = edgeIndex;
    }
}

dContactGeomExt *__cdecl AddContact(Results *results)
{
    if (results->contactCount >= results->maxContacts)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 74, 0, "%s", "results->contactCount < results->maxContacts");
    return (dContactGeomExt *)((char *)results->contacts + results->stride * results->contactCount++);
}

bool __cdecl Phys_AddContactData(Results *results, float depth, float *normal, float *pos, int surfaceFlags)
{
    const char *v5; // eax
    dContactGeomExt *v7; // eax

    if ((COERCE_UNSIGNED_INT(*normal) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(normal[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(normal[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\physics\\phys_coll_boxbrush.cpp",
            87,
            0,
            "%s",
            "!IS_NAN((normal)[0]) && !IS_NAN((normal)[1]) && !IS_NAN((normal)[2])");
    }
    if (!Vec3IsNormalized(normal))
    {
        v5 = va("normal: (%f, %f, %f)", *normal, normal[1], normal[2]);
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 88, 0, "%s\n\t%s", "Vec3IsNormalized( normal )", v5);
    }
    if ((COERCE_UNSIGNED_INT(*pos) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(pos[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(pos[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\physics\\phys_coll_boxbrush.cpp",
            89,
            0,
            "%s",
            "!IS_NAN((pos)[0]) && !IS_NAN((pos)[1]) && !IS_NAN((pos)[2])");
    }
    if ((LODWORD(depth) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 90, 0, "%s", "!IS_NAN(depth)");
    if (results->contactCount >= results->maxContacts)
        return 0;
    v7 = AddContact(results);
    v7->contact.depth = depth;
    v7->contact.normal[0] = *normal;
    v7->contact.normal[1] = normal[1];
    v7->contact.normal[2] = normal[2];
    v7->contact.normal[3] = 0.0;
    v7->contact.pos[0] = *pos;
    v7->contact.pos[1] = pos[1];
    v7->contact.pos[2] = pos[2];
    v7->contact.pos[3] = 0.0;
    v7->surfFlags = surfaceFlags;
    return results->contactCount < results->maxContacts;
}

PolyOrientation __cdecl GetPolyOrientation(const float *polyNormal, const float (*poly)[3], unsigned int ptCount)
{
    unsigned int ptIndex; // [esp+4h] [ebp-28h]
    float v2[3]; // [esp+8h] [ebp-24h] BYREF
    float v1[3]; // [esp+14h] [ebp-18h] BYREF
    float v3[3]; // [esp+20h] [ebp-Ch] BYREF

    if (ptCount <= 2)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 149, 0, "%s", "ptCount > 2");
    Vec3Sub(&(*poly)[3], (const float *)poly, v1);
    for (ptIndex = 1; ptIndex < ptCount - 1; ++ptIndex)
    {
        Vec3Sub(&(*poly)[3 * ptIndex + 3], &(*poly)[3 * ptIndex], v2);
        Vec3Cross(v1, v2, v3);
        if (Vec3LengthSq(v3) > 0.009999999776482582)
            return (PolyOrientation)(Vec3Dot(v3, polyNormal) < 0.0);
    }
    return (PolyOrientation)2;
}

char __cdecl Phys_GetChoppingPlaneForPolyEdge(
    const float *polyNormal,
    const float *pt1,
    const float *pt2,
    bool clockwise,
    float *outPlane)
{
    float edge[3]; // [esp+10h] [ebp-Ch] BYREF

    Vec3Sub(pt2, pt1, edge);
    if (Vec3LengthSq(edge) < 0.1000000014901161)
        return 0;
    if (clockwise)
        Vec3Cross(polyNormal, edge, outPlane);
    else
        Vec3Cross(edge, polyNormal, outPlane);
    if (Vec3LengthSq(outPlane) < 0.1000000014901161)
        return 0;
    outPlane[3] = Vec3Dot(pt1, outPlane);
    if ((COERCE_UNSIGNED_INT(*outPlane) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(outPlane[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(outPlane[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\physics\\phys_coll_boxbrush.cpp",
            192,
            0,
            "%s",
            "!IS_NAN((outPlane)[0]) && !IS_NAN((outPlane)[1]) && !IS_NAN((outPlane)[2])");
    }
    if ((COERCE_UNSIGNED_INT(outPlane[3]) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 193, 0, "%s", "!IS_NAN(outPlane[3])");
    return 1;
}

unsigned int __cdecl Phys_ClipLineSegmentAgainstPlane(float *pt1, float *pt2, const float *choppingPlane)
{
    float dist1; // [esp+8h] [ebp-Ch]
    float frac; // [esp+Ch] [ebp-8h]
    float fraca; // [esp+Ch] [ebp-8h]
    float dist2; // [esp+10h] [ebp-4h]

    dist1 = Vec3Dot(pt1, choppingPlane) - choppingPlane[3];
    dist2 = Vec3Dot(pt2, choppingPlane) - choppingPlane[3];
    if (dist1 > 0.0)
    {
        if (dist2 > 0.0)
        {
            return 0;
        }
        else
        {
            if (dist2 - dist1 >= 0.0)
                MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 222, 1, "%s", "( dist2 - dist1 ) < 0");
            fraca = dist2 / (dist2 - dist1);
            Vec3Lerp(pt2, pt1, fraca, pt1);
            return 2;
        }
    }
    else if (dist2 > 0.0)
    {
        if (dist1 - dist2 >= 0.0)
            MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 213, 1, "%s", "( dist1 - dist2 ) < 0");
        frac = dist1 / (dist1 - dist2);
        Vec3Lerp(pt1, pt2, frac, pt2);
        return 2;
    }
    else
    {
        return 2;
    }
}

unsigned int __cdecl Phys_ClipLineSegmentAgainstPoly(
    const float *polyNormal,
    const float (*poly)[3],
    unsigned int polyCount,
    float *pt1,
    float *pt2)
{
    PolyOrientation orient; // [esp+4h] [ebp-18h]
    unsigned int polyIndex; // [esp+8h] [ebp-14h]
    float choppingPlane[4]; // [esp+Ch] [ebp-10h] BYREF

    if (polyCount <= 2)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 434, 0, "%s", "polyCount > 2");
    if (!poly)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 435, 0, "%s", "poly");
    orient = GetPolyOrientation(polyNormal, poly, polyCount);
    if (orient == POLY_ERROR)
        return 0;
    for (polyIndex = 0; polyIndex < polyCount; ++polyIndex)
    {
        if (Phys_GetChoppingPlaneForPolyEdge(
            polyNormal,
            &(*poly)[3 * polyIndex],
            &(*poly)[3 * ((polyIndex + 1) % polyCount)],
            orient == POLY_CLOCKWISE,
            choppingPlane)
            && !Phys_ClipLineSegmentAgainstPlane(pt1, pt2, choppingPlane))
        {
            return 0;
        }
    }
    return 2;
}

void __cdecl Phys_ProjectFaceOntoFaceAndClip(
    const float *referencePlane,
    const Poly *referencePoly,
    const Poly *poly2,
    int surfaceFlags,
    Results *results,
    float *collisionNormal)
{
    float depth; // [esp+10h] [ebp-C10h]
    unsigned int ptCount; // [esp+14h] [ebp-C0Ch]
    float clippedPoly[256][3]; // [esp+18h] [ebp-C08h] BYREF
    unsigned int ptIndex; // [esp+C1Ch] [ebp-4h]

    if (!referencePlane)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 549, 0, "%s", "referencePlane");
    if (!referencePoly)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 550, 0, "%s", "referencePoly");
    if (!poly2)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 551, 0, "%s", "poly2");
    if (referencePoly->ptCount <= 2)
        MyAssertHandler(
            ".\\physics\\phys_coll_boxbrush.cpp",
            552,
            0,
            "referencePoly->ptCount > 2\n\t%i, %i",
            referencePoly->ptCount,
            2);
    if (poly2->ptCount <= 2)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 553, 0, "poly2->ptCount > 2\n\t%i, %i", poly2->ptCount, 2);
    if (!results)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 554, 0, "%s", "results");
    if (!collisionNormal)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 555, 0, "%s", "collisionNormal");
    ptCount = ClipPolys(
        referencePlane,
        referencePoly->pts,
        referencePoly->ptCount,
        poly2->pts,
        poly2->ptCount,
        clippedPoly,
        0x100u);
    for (ptIndex = 0; ptIndex < ptCount; ++ptIndex)
    {
        depth = referencePlane[3] - Vec3Dot(referencePlane, clippedPoly[ptIndex]);
        if (depth > 0.0 && !Phys_AddContactData(results, depth, collisionNormal, clippedPoly[ptIndex], surfaceFlags))
            break;
    }
}

unsigned int __cdecl ClipPolys(
    const float *polyNormal,
    const float (*poly1)[3],
    unsigned int poly1Count,
    float (*poly2)[3],
    unsigned int poly2Count,
    float (*result)[3],
    unsigned int maxCount)
{
    PolyOrientation orient; // [esp+4h] [ebp-1Ch]
    unsigned int poly1Index; // [esp+8h] [ebp-18h]
    unsigned int lastPoly1Index; // [esp+Ch] [ebp-14h]
    float choppingPlane[4]; // [esp+10h] [ebp-10h] BYREF

    if (poly1Count <= 2)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 397, 0, "%s", "poly1Count > 2");
    if (poly2Count <= 2)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 398, 0, "%s", "poly2Count > 2");
    if (maxCount < poly2Count + poly1Count)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 399, 0, "%s", "maxCount >= poly1Count + poly2Count");
    if (!poly1)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 400, 0, "%s", "poly1");
    if (!poly2)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 401, 0, "%s", "poly2");
    if (!result)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 402, 0, "%s", "result");
    if (poly1 == result)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 403, 0, "%s", "poly1 != result");
    if (poly2 == result)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 404, 0, "%s", "poly2 != result");
    memcpy((unsigned __int8 *)result, (unsigned __int8 *)poly2, 12 * poly2Count);
    orient = GetPolyOrientation(polyNormal, poly1, poly1Count);
    if (orient == POLY_ERROR)
        return 0;
    lastPoly1Index = poly1Count - 1;
    for (poly1Index = 0; poly1Index < poly1Count; ++poly1Index)
    {
        if (Phys_GetChoppingPlaneForPolyEdge(
            polyNormal,
            &(*poly1)[3 * lastPoly1Index],
            &(*poly1)[3 * poly1Index],
            orient == POLY_CLOCKWISE,
            choppingPlane))
        {
            poly2Count = Phys_ClipPolyAgainstPlane(result, poly2Count, maxCount, choppingPlane);
            if (poly2Count < 3)
                return 0;
        }
        lastPoly1Index = poly1Index;
    }
    return poly2Count;
}

void __cdecl Phys_GetWindingForBrushFace2(
    const cbrush_t *brush,
    unsigned int brushSide,
    Poly *outWinding,
    int maxVerts,
    const float (*axialPlanes)[4])
{
    float v5; // [esp+30h] [ebp-E0h]
    float v[5]; // [esp+3Ch] [ebp-D4h] BYREF
    float *a; // [esp+50h] [ebp-C0h]
    float *b; // [esp+54h] [ebp-BCh]
    float diff[3]; // [esp+58h] [ebp-B8h] BYREF
    float v10; // [esp+64h] [ebp-ACh]
    float v11; // [esp+68h] [ebp-A8h]
    float v12; // [esp+6Ch] [ebp-A4h]
    int v13; // [esp+70h] [ebp-A0h]
    float *v14; // [esp+74h] [ebp-9Ch]
    float v15; // [esp+78h] [ebp-98h]
    float v16; // [esp+7Ch] [ebp-94h]
    float v17; // [esp+80h] [ebp-90h]
    float v18; // [esp+84h] [ebp-8Ch]
    float v19; // [esp+88h] [ebp-88h]
    float v20; // [esp+8Ch] [ebp-84h]
    float v21; // [esp+90h] [ebp-80h]
    float v22; // [esp+94h] [ebp-7Ch]
    float *v23; // [esp+98h] [ebp-78h]
    cplane_s *v24; // [esp+9Ch] [ebp-74h]
    float *v25; // [esp+A0h] [ebp-70h]
    float *v26; // [esp+A4h] [ebp-6Ch]
    float *v27; // [esp+A8h] [ebp-68h]
    cplane_s *v28; // [esp+ACh] [ebp-64h]
    float *v29; // [esp+B0h] [ebp-60h]
    float *v30; // [esp+B4h] [ebp-5Ch]
    cplane_s *plane; // [esp+B8h] [ebp-58h]
    float *v32; // [esp+BCh] [ebp-54h]
    float *pt; // [esp+C0h] [ebp-50h]
    int edgeIndex; // [esp+C4h] [ebp-4Ch]
    float planes[3][4]; // [esp+C8h] [ebp-48h] BYREF
    int offset; // [esp+F8h] [ebp-18h]
    int side2; // [esp+FCh] [ebp-14h]
    int edgeCount; // [esp+100h] [ebp-10h]
    int side1; // [esp+104h] [ebp-Ch]
    unsigned __int8 *edges; // [esp+108h] [ebp-8h]
    unsigned int nonAxialSideIndex; // [esp+10Ch] [ebp-4h]

    if (!brush)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 824, 0, "%s", "brush");
    if (brushSide >= brush->numsides + 6)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 825, 0, "%s", "brushSide < brush->numsides + 6");
    if (!outWinding)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 826, 0, "%s", "outWinding");
    if (!outWinding->pts)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 827, 0, "%s", "outWinding->pts");
    if (brushSide >= 6)
    {
        if (!brush->sides)
            MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 837, 0, "%s", "brush->sides");
        nonAxialSideIndex = brushSide - 6;
        offset = brush->sides[brushSide - 6].firstAdjacentSideOffset;
        edgeCount = brush->sides[brushSide - 6].edgeCount;
        plane = brush->sides[brushSide - 6].plane;
        planes[0][0] = plane->normal[0];
        planes[0][1] = plane->normal[1];
        planes[0][2] = plane->normal[2];
        planes[0][3] = brush->sides[brushSide - 6].plane->dist;
    }
    else
    {
        offset = brush->firstAdjacentSideOffsets[brushSide & 1][brushSide >> 1];
        edgeCount = brush->edgeCount[brushSide & 1][brushSide >> 1];
        v32 = (float *)&(*axialPlanes)[4 * brushSide];
        planes[0][0] = *v32;
        planes[0][1] = v32[1];
        planes[0][2] = v32[2];
        planes[0][3] = v32[3];
    }
    if (edgeCount >= 3 && edgeCount <= maxVerts)
    {
        if (!brush->baseAdjacentSide)
            MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 851, 0, "%s", "brush->baseAdjacentSide");
        edges = &brush->baseAdjacentSide[offset];
        side1 = edges[edgeCount - 1];
        if (side1 >= brush->numsides + 6)
            MyAssertHandler(
                ".\\physics\\phys_coll_boxbrush.cpp",
                855,
                0,
                "side1 doesn't index brush->numsides + 6\n\t%i not in [0, %i)",
                side1,
                brush->numsides + 6);
        if (side1 >= 6)
        {
            if (!brush->sides)
                MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 862, 0, "%s", "brush->sides");
            nonAxialSideIndex = side1 - 6;
            v27 = planes[1];
            v28 = brush->sides[side1 - 6].plane;
            planes[1][0] = v28->normal[0];
            planes[1][1] = v28->normal[1];
            planes[1][2] = v28->normal[2];
            planes[1][3] = brush->sides[side1 - 6].plane->dist;
        }
        else
        {
            v29 = planes[1];
            v30 = (float *)&(*axialPlanes)[4 * side1];
            planes[1][0] = *v30;
            planes[1][1] = v30[1];
            planes[1][2] = v30[2];
            planes[1][3] = v30[3];
        }
        outWinding->ptCount = 0;
        for (edgeIndex = 0; edgeIndex < edgeCount; ++edgeIndex)
        {
            side2 = edges[edgeIndex];
            if (side2 >= brush->numsides + 6)
                MyAssertHandler(
                    ".\\physics\\phys_coll_boxbrush.cpp",
                    872,
                    0,
                    "side2 doesn't index brush->numsides + 6\n\t%i not in [0, %i)",
                    side2,
                    brush->numsides + 6);
            if (side2 >= 6)
            {
                if (!brush->sides)
                    MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 879, 0, "%s", "brush->sides");
                nonAxialSideIndex = side2 - 6;
                v23 = planes[2];
                v24 = brush->sides[side2 - 6].plane;
                planes[2][0] = v24->normal[0];
                planes[2][1] = v24->normal[1];
                planes[2][2] = v24->normal[2];
                planes[2][3] = brush->sides[side2 - 6].plane->dist;
            }
            else
            {
                v25 = planes[2];
                v26 = (float *)&(*axialPlanes)[4 * side2];
                planes[2][0] = *v26;
                planes[2][1] = v26[1];
                planes[2][2] = v26[2];
                planes[2][3] = v26[3];
            }
            v14 = outWinding->pts[outWinding->ptCount];
            v17 = planes[1][1] * planes[2][2] - planes[2][1] * planes[1][2];
            v18 = planes[2][1] * planes[0][2] - planes[0][1] * planes[2][2];
            v19 = planes[0][1] * planes[1][2] - planes[1][1] * planes[0][2];
            v20 = planes[0][0] * v17;
            v21 = planes[1][0] * v18;
            v22 = planes[2][0] * v19;
            v16 = v20 + v21 + v22;
            v5 = fabs(v16);
            if (v5 >= 0.001000000047497451)
            {
                v15 = 1.0 / v16;
                v20 = planes[0][3] * v17;
                v21 = planes[1][3] * v18;
                v22 = planes[2][3] * v19;
                *v14 = (v20 + v21 + v22) * v15;
                v20 = (planes[1][2] * planes[2][0] - planes[2][2] * planes[1][0]) * planes[0][3];
                v21 = (planes[2][2] * planes[0][0] - planes[0][2] * planes[2][0]) * planes[1][3];
                v22 = (planes[0][2] * planes[1][0] - planes[1][2] * planes[0][0]) * planes[2][3];
                v14[1] = (v20 + v21 + v22) * v15;
                v20 = (planes[1][0] * planes[2][1] - planes[2][0] * planes[1][1]) * planes[0][3];
                v21 = (planes[2][0] * planes[0][1] - planes[0][0] * planes[2][1]) * planes[1][3];
                v22 = (planes[0][0] * planes[1][1] - planes[1][0] * planes[0][1]) * planes[2][3];
                v14[2] = (v20 + v21 + v22) * v15;
                v13 = 1;
            }
            else
            {
                v13 = 0;
            }
            if (v13)
            {
                pt = outWinding->pts[outWinding->ptCount];
                v12 = *pt;
                if ((LODWORD(v12) & 0x7F800000) == 0x7F800000
                    || (v11 = pt[1], (LODWORD(v11) & 0x7F800000) == 0x7F800000)
                    || (v10 = pt[2], (LODWORD(v10) & 0x7F800000) == 0x7F800000))
                {
                    Com_PrintError(20, "Bad intersection of three planes:\n");
                    Com_PrintError(20, "Resulting point: (%f, %f, %f)\n", *pt, pt[1], pt[2]);
                    Com_PrintError(20, "plane 0: (%f, %f, %f, %f)\n", planes[0][0], planes[0][1], planes[0][2], planes[0][3]);
                    Com_PrintError(20, "plane 1: (%f, %f, %f, %f)\n", planes[1][0], planes[1][1], planes[1][2], planes[1][3]);
                    Com_PrintError(20, "plane 2: (%f, %f, %f, %f)\n", planes[2][0], planes[2][1], planes[2][2], planes[2][3]);
                    Com_PrintError(
                        20,
                        "brush mins/maxs: (%f, %f, %f) (%f, %f, %f)\n",
                        brush->mins[0],
                        brush->mins[1],
                        brush->mins[2],
                        brush->maxs[0],
                        brush->maxs[1],
                        brush->maxs[2]);
                    Com_PrintError(20, "brush numsides: %i\n", brush->numsides);
                    Com_PrintError(20, "brushside: %i\n", brushSide);
                    if (!alwaysfails)
                        MyAssertHandler(
                            ".\\physics\\phys_coll_boxbrush.cpp",
                            900,
                            0,
                            "Phys_GetWindingForBrushFace2: bad intersection.  Please include console log in bug report.");
                }
                if (!outWinding->ptCount
                    || (a = outWinding->pts[outWinding->ptCount - 1],
                        b = outWinding->pts[outWinding->ptCount],
                        Vec3Sub(a, b, diff),
                        Vec3LengthSq(diff) >= 1.0))
                {
                    ++outWinding->ptCount;
                }
            }
            LODWORD(v[3]) = planes[1];
            LODWORD(v[4]) = planes[2];
            planes[1][0] = planes[2][0];
            planes[1][1] = planes[2][1];
            planes[1][2] = planes[2][2];
            planes[1][3] = planes[2][3];
        }
        if (outWinding->ptCount > 1)
        {
            Vec3Sub((const float *)outWinding->pts, outWinding->pts[outWinding->ptCount - 1], v);
            if (Vec3LengthSq(v) < 1.0)
                --outWinding->ptCount;
        }
        if (outWinding->ptCount < 3)
            outWinding->ptCount = 0;
    }
    else
    {
        outWinding->ptCount = 0;
    }
}

void __cdecl Phys_CollideBoxWithBrush(const cbrush_t *brush, const objInfo *info, Results *results)
{
    double v3; // st7
    float v4; // [esp+8h] [ebp-39B0h]
    float v5; // [esp+Ch] [ebp-39ACh]
    float v6; // [esp+10h] [ebp-39A8h]
    float v7; // [esp+14h] [ebp-39A4h]
    float v8; // [esp+18h] [ebp-39A0h]
    float v9; // [esp+1Ch] [ebp-399Ch]
    float v10; // [esp+20h] [ebp-3998h]
    float v11; // [esp+24h] [ebp-3994h]
    float v12; // [esp+28h] [ebp-3990h]
    float v13; // [esp+2Ch] [ebp-398Ch]
    float v14; // [esp+30h] [ebp-3988h]
    float v15; // [esp+34h] [ebp-3984h]
    cplane_s *plane; // [esp+38h] [ebp-3980h]
    float bestBrushPlane[4]; // [esp+90h] [ebp-3928h] BYREF
    Poly outWinding[256]; // [esp+A0h] [ebp-3918h] BYREF
    float v19; // [esp+8A0h] [ebp-3118h]
    unsigned int v20; // [esp+8A4h] [ebp-3114h]
    int surfaceFlags; // [esp+8A8h] [ebp-3110h]
    int boxAxis; // [esp+8ACh] [ebp-310Ch]
    int boxSign; // [esp+8B0h] [ebp-3108h]
    unsigned int ptCount; // [esp+8B4h] [ebp-3104h]
    int outSideIndex; // [esp+8B8h] [ebp-3100h] BYREF
    float a[3]; // [esp+8BCh] [ebp-30FCh] BYREF
    int k; // [esp+8C8h] [ebp-30F0h]
    float *v28; // [esp+8CCh] [ebp-30ECh]
    unsigned int brushSide; // [esp+8D0h] [ebp-30E8h]
    float normal; // [esp+8D4h] [ebp-30E4h] BYREF
    float v31; // [esp+8D8h] [ebp-30E0h]
    float v32; // [esp+8DCh] [ebp-30DCh]
    unsigned int v33[3073]; // [esp+8E0h] [ebp-30D8h] BYREF
    unsigned int j; // [esp+38E4h] [ebp-D4h]
    float out[3]; // [esp+38E8h] [ebp-D0h] BYREF
    int brushSideIndex; // [esp+38F4h] [ebp-C4h]
    float outMaxSeparation; // [esp+38F8h] [ebp-C0h] BYREF
    float v38; // [esp+38FCh] [ebp-BCh] BYREF
    float v39; // [esp+3900h] [ebp-B8h]
    float v40; // [esp+3904h] [ebp-B4h]
    float v41; // [esp+3908h] [ebp-B0h]
    float v42; // [esp+390Ch] [ebp-ACh]
    float v43; // [esp+3910h] [ebp-A8h]
    int m; // [esp+3914h] [ebp-A4h]
    unsigned int v45; // [esp+3918h] [ebp-A0h]
    float diff[3]; // [esp+391Ch] [ebp-9Ch] BYREF
    unsigned int n; // [esp+3928h] [ebp-90h]
    float outBrushPlane[4]; // [esp+392Ch] [ebp-8Ch] BYREF
    unsigned int i; // [esp+393Ch] [ebp-7Ch]
    float axialPlanes[6][4]; // [esp+3940h] [ebp-78h] BYREF
    float sum; // [esp+39A0h] [ebp-18h] BYREF
    float v52; // [esp+39A4h] [ebp-14h]
    float v53; // [esp+39A8h] [ebp-10h]
    float v54; // [esp+39ACh] [ebp-Ch] BYREF
    float v55; // [esp+39B0h] [ebp-8h]
    float v56; // [esp+39B4h] [ebp-4h]

    if (!brush)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1233, 0, "%s", "brush");
    if (!info)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1234, 0, "%s", "info");
    if (!results)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1235, 0, "%s", "results");
    if (results->contactCount >= results->maxContacts)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1236, 0, "%s", "results->contactCount < results->maxContacts");
    if (Phys_TestBoxAgainstEachBrushPlane(brush, info, outBrushPlane, &outSideIndex, &outMaxSeparation))
    {
        //Profile_Begin(381);
        v38 = 0.0;
        v39 = 0.0;
        v40 = 0.0;
        v41 = 0.0;
        v42 = 0.0;
        v43 = 0.0;
        a[0] = -info->u.sideExtents[0];
        a[1] = -info->u.sideExtents[1];
        a[2] = -info->u.sideExtents[2];
        v20 = brush->numsides + 6;
        if (v20 > 0x100)
            MyAssertHandler(
                ".\\physics\\phys_coll_boxbrush.cpp",
                1249,
                0,
                "brushSideCount <= ARRAY_COUNT( brushWindings )\n\t%i, %i",
                v20,
                256);
        v45 = 0;
        CM_BuildAxialPlanes(brush, (float (*)[4])axialPlanes);
        for (brushSide = 0; brushSide < v20; ++brushSide)
        {
            v28 = (float *)&v33[3 * v45];
            outWinding[brushSide].pts = (float (*)[3])v28;
            Phys_GetWindingForBrushFace2(
                brush,
                brushSide,
                &outWinding[brushSide],
                1024 - v45,
                (const float (*)[4])axialPlanes);
            ptCount = outWinding[brushSide].ptCount;
            v45 += ptCount;
            if (v45 > 0x400)
                MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1259, 0, "%s", "vertCount <= ARRAY_COUNT( brushVerts )");
            for (i = 0; i < ptCount; ++i)
            {
                Vec3Sub(&v28[3 * i], info->pos, diff);
                MatrixTransformVector(diff, info->RTransposed, out);
                Vec3Add(a, out, &sum);
                Vec3Sub(a, out, &v54);
                v15 = sum - v38;
                if (v15 < 0.0)
                    v14 = sum;
                else
                    v14 = v38;
                v38 = v14;
                v13 = v52 - v39;
                if (v13 < 0.0)
                    v12 = v52;
                else
                    v12 = v39;
                v39 = v12;
                v11 = v53 - v40;
                if (v11 < 0.0)
                    v10 = v53;
                else
                    v10 = v40;
                v40 = v10;
                v9 = v54 - v41;
                if (v9 < 0.0)
                    v8 = v54;
                else
                    v8 = v41;
                v41 = v8;
                v7 = v55 - v42;
                if (v7 < 0.0)
                    v6 = v55;
                else
                    v6 = v42;
                v42 = v6;
                v5 = v56 - v43;
                if (v5 < 0.0)
                    v4 = v56;
                else
                    v4 = v43;
                v43 = v4;
            }
        }
        //Profile_EndInternal(0);
        if (v45)
        {
            if (phys_drawCollisionWorld->current.enabled)
            {
                for (j = 0; j < brush->numsides + 6; ++j)
                    Phys_DrawPoly(&outWinding[j], colorCyan);
            }
            boxAxis = 0;
            boxSign = 0;
            for (k = 0; k < 3; ++k)
            {
                for (m = 0; m < 2; ++m)
                {
                    if (*(&v38 + 3 * m + k) >= 0.0)
                        return;
                    if (*(&v38 + 3 * boxSign + boxAxis) < (double)*(&v38 + 3 * m + k))
                    {
                        boxAxis = k;
                        boxSign = m;
                    }
                }
            }
            v19 = *(&v38 + 3 * boxSign + boxAxis);
            if (v19 >= 0.0)
                MyAssertHandler(
                    ".\\physics\\phys_coll_boxbrush.cpp",
                    1314,
                    0,
                    "%s\n\t(boxPlaneSeparation) = %g",
                    "(boxPlaneSeparation < 0.0f)",
                    v19);
            if (outMaxSeparation >= (double)v19 && outSideIndex != -1 && outWinding[outSideIndex].ptCount)
            {
                Phys_CollideBoxWithBrushFace(
                    brush,
                    outSideIndex,
                    outBrushPlane,
                    &outWinding[outSideIndex],
                    info,
                    results,
                    &normal);
            }
            else
            {
                if (boxSign)
                {
                    normal = -info->R[boxAxis][0];
                    v31 = -info->R[boxAxis][1];
                    v32 = -info->R[boxAxis][2];
                }
                else
                {
                    normal = info->R[boxAxis][0];
                    v31 = info->R[boxAxis][1];
                    v32 = info->R[boxAxis][2];
                }
                outSideIndex = GetClosestBrushFace(&normal, brush, outWinding, outBrushPlane);
                if (outSideIndex >= 0)
                {
                    if (outSideIndex >= brush->numsides + 6)
                        MyAssertHandler(
                            ".\\physics\\phys_coll_boxbrush.cpp",
                            1326,
                            0,
                            "brushSideIndex doesn't index static_cast< int >( brush->numsides ) + 6\n\t%i not in [0, %i)",
                            outSideIndex,
                            brush->numsides + 6);
                    surfaceFlags = Phys_GetSurfaceFlagsFromBrush(brush, outSideIndex);
                    Phys_ProjectBoxFaceOntoBrushFaceAndClip(
                        info,
                        boxAxis,
                        boxSign,
                        outBrushPlane,
                        &outWinding[outSideIndex],
                        surfaceFlags,
                        results,
                        &normal);
                }
            }
            if (info->isNarrow)
            {
                for (n = 0; n < 3; ++n)
                {
                    brushSideIndex = 2 * n;
                    if (outWinding[2 * n].ptCount && brush->mins[n] > (double)info->pos[n])
                    {
                        if (brushSideIndex != outSideIndex && Phys_DoesPolyIntersectBox(&outWinding[brushSideIndex], info))
                            Phys_CollideBoxWithBrushFace(
                                brush,
                                brushSideIndex,
                                (const float *)&axialPlanes[brushSideIndex],
                                &outWinding[brushSideIndex],
                                info,
                                results,
                                &normal);
                    }
                    else
                    {
                        brushSideIndex = 2 * n + 1;
                        if (outWinding[brushSideIndex].ptCount
                            && brush->maxs[n] < (double)info->pos[n]
                            && brushSideIndex != outSideIndex
                                && Phys_DoesPolyIntersectBox(&outWinding[brushSideIndex], info))
                        {
                            Phys_CollideBoxWithBrushFace(
                                brush,
                                brushSideIndex,
                                (const float *)&axialPlanes[brushSideIndex],
                                &outWinding[brushSideIndex],
                                info,
                                results,
                                &normal);
                        }
                    }
                }
                for (n = 0; n < brush->numsides; ++n)
                {
                    brushSideIndex = n + 6;
                    if (outSideIndex != n + 6)
                    {
                        if (outWinding[brushSideIndex].ptCount)
                        {
                            v3 = Vec3Dot(info->pos, brush->sides[n].plane->normal);
                            if (brush->sides[n].plane->dist < v3)
                            {
                                if (Phys_DoesPolyIntersectBox(&outWinding[brushSideIndex], info))
                                {
                                    plane = brush->sides[n].plane;
                                    bestBrushPlane[0] = plane->normal[0];
                                    bestBrushPlane[1] = plane->normal[1];
                                    bestBrushPlane[2] = plane->normal[2];
                                    bestBrushPlane[3] = brush->sides[n].plane->dist;
                                    Phys_CollideBoxWithBrushFace(
                                        brush,
                                        brushSideIndex,
                                        bestBrushPlane,
                                        &outWinding[brushSideIndex],
                                        info,
                                        results,
                                        &normal);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void __cdecl Phys_ProjectBoxFaceOntoBrushFaceAndClip(
    const objInfo *info,
    int boxAxis,
    int boxSign,
    const float *brushPlane,
    const Poly *winding,
    int surfaceFlags,
    Results *results,
    float *collisionNormal)
{
    double v8; // st7
    float scale; // [esp+4h] [ebp-CB4h]
    float v10; // [esp+10h] [ebp-CA8h]
    float v11; // [esp+14h] [ebp-CA4h]
    float v12; // [esp+18h] [ebp-CA0h]
    float *v13; // [esp+24h] [ebp-C94h]
    float *v14; // [esp+28h] [ebp-C90h]
    float depth; // [esp+30h] [ebp-C88h]
    float boxFaceCorners[4][3]; // [esp+34h] [ebp-C84h] BYREF
    int axisY; // [esp+64h] [ebp-C54h]
    float clippedPoly[256][3]; // [esp+68h] [ebp-C50h] BYREF
    float dist; // [esp+C6Ch] [ebp-4Ch]
    int axisX; // [esp+C70h] [ebp-48h]
    unsigned int cornerIndex; // [esp+C74h] [ebp-44h]
    bool clockwise; // [esp+C7Bh] [ebp-3Dh]
    float tempV[3]; // [esp+C7Ch] [ebp-3Ch] BYREF
    PolyOrientation orient; // [esp+C88h] [ebp-30h]
    unsigned int pointIndex; // [esp+C8Ch] [ebp-2Ch]
    unsigned int pointCount; // [esp+C90h] [ebp-28h]
    int poly1Index; // [esp+C94h] [ebp-24h]
    float boxPlane[4]; // [esp+C98h] [ebp-20h] BYREF
    float choppingPlane[4]; // [esp+CA8h] [ebp-10h] BYREF

    if (!winding)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 473, 0, "%s", "winding");
    if (winding->ptCount <= 2)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 474, 0, "winding->ptCount > 2\n\t%i, %i", winding->ptCount, 2);
    if (!info)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 475, 0, "%s", "info");
    if (!brushPlane)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 476, 0, "%s", "brushPlane");
    if (!results)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 477, 0, "%s", "results");
    axisX = (boxAxis + 1) % 3;
    axisY = (axisX + 1) % 3;
    tempV[0] = info->u.sideExtents[0];
    tempV[1] = info->u.sideExtents[1];
    tempV[2] = info->u.sideExtents[2];
    if (boxSign)
        tempV[boxAxis] = -tempV[boxAxis];
    MatrixTransformVector(tempV, info->R, boxFaceCorners[0]);
    v12 = info->u.sideExtents[axisX] * -2.0;
    Vec3Mad(boxFaceCorners[0], v12, info->R[axisX], boxFaceCorners[1]);
    v11 = info->u.sideExtents[axisY] * -2.0;
    Vec3Mad(boxFaceCorners[1], v11, info->R[axisY], boxFaceCorners[2]);
    v10 = info->u.sideExtents[axisX] * 2.0;
    Vec3Mad(boxFaceCorners[2], v10, info->R[axisX], boxFaceCorners[3]);
    for (cornerIndex = 0; cornerIndex < 4; ++cornerIndex)
    {
        Vec3Add(boxFaceCorners[cornerIndex], info->pos, boxFaceCorners[cornerIndex]);
        dist = Vec3Dot(boxFaceCorners[cornerIndex], brushPlane) - brushPlane[3];
        scale = -dist;
        Vec3Mad(boxFaceCorners[cornerIndex], scale, brushPlane, boxFaceCorners[cornerIndex]);
    }
    pointCount = winding->ptCount;
    if (pointCount <= 2)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 499, 0, "%s", "pointCount > 2");
    if (pointCount + 4 > 0x100)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 500, 0, "%s", "ARRAY_COUNT( clippedPoly ) >= 4 + pointCount");
    for (cornerIndex = 0; cornerIndex < pointCount; ++cornerIndex)
    {
        v13 = clippedPoly[cornerIndex];
        v14 = winding->pts[cornerIndex];
        *v13 = *v14;
        v13[1] = v14[1];
        v13[2] = v14[2];
    }
    orient = GetPolyOrientation(brushPlane, boxFaceCorners, 4u);
    if (orient != POLY_ERROR)
    {
        clockwise = orient == POLY_CLOCKWISE;
        for (poly1Index = 0; poly1Index < 4; ++poly1Index)
        {
            if (Phys_GetChoppingPlaneForPolyEdge(
                brushPlane,
                boxFaceCorners[poly1Index],
                boxFaceCorners[(poly1Index + 1) % 4],
                clockwise,
                choppingPlane))
            {
                pointCount = Phys_ClipPolyAgainstPlane(clippedPoly, pointCount, 0x100u, choppingPlane);
                if (pointCount < 3)
                    return;
            }
        }
        if (!pointCount)
            MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 521, 0, "%s", "pointCount");
        if (boxSign)
        {
            boxPlane[0] = -info->R[boxAxis][0];
            boxPlane[1] = -info->R[boxAxis][1];
            boxPlane[2] = -info->R[boxAxis][2];
        }
        else
        {
            boxPlane[0] = info->R[boxAxis][0];
            boxPlane[1] = info->R[boxAxis][1];
            boxPlane[2] = info->R[boxAxis][2];
        }
        boxPlane[3] = Vec3Dot(info->pos, boxPlane) + info->u.sideExtents[boxAxis];
        for (pointIndex = 0; pointIndex < pointCount; ++pointIndex)
        {
            v8 = Vec3Dot(clippedPoly[pointIndex], boxPlane);
            depth = boxPlane[3] - v8;
            if (depth > 0.0 && !Phys_AddContactData(results, depth, collisionNormal, clippedPoly[pointIndex], surfaceFlags))
                break;
        }
    }
}

int __cdecl GetClosestBrushFace(
    const float *normal,
    const cbrush_t *brush,
    const Poly *brushWindings,
    float *outBrushPlane)
{
    cplane_s *plane; // [esp+0h] [ebp-98h]
    float *v6; // [esp+4h] [ebp-94h]
    int outSideIndex; // [esp+24h] [ebp-74h]
    unsigned int sideIndex; // [esp+28h] [ebp-70h]
    unsigned int sideIndexa; // [esp+28h] [ebp-70h]
    float minDot; // [esp+2Ch] [ebp-6Ch]
    float axialPlanes[6][4]; // [esp+30h] [ebp-68h] BYREF
    float dot; // [esp+90h] [ebp-8h]
    float DOT_THRESHOLD; // [esp+94h] [ebp-4h]

    DOT_THRESHOLD = -0.1;
    CM_BuildAxialPlanes(brush, axialPlanes);
    minDot = 2.0;
    outSideIndex = -1;
    for (sideIndex = 0; sideIndex < 6; ++sideIndex)
    {
        if (brushWindings[sideIndex].ptCount)
        {
            dot = Vec3Dot(normal, axialPlanes[sideIndex]);
            if (minDot > (double)dot && DOT_THRESHOLD > (double)dot)
            {
                minDot = dot;
                outSideIndex = sideIndex;
                v6 = axialPlanes[sideIndex];
                *outBrushPlane = *v6;
                outBrushPlane[1] = v6[1];
                outBrushPlane[2] = v6[2];
                outBrushPlane[3] = v6[3];
            }
        }
    }
    for (sideIndexa = 0; sideIndexa < brush->numsides; ++sideIndexa)
    {
        if (brushWindings[sideIndexa + 6].ptCount)
        {
            dot = Vec3Dot(normal, brush->sides[sideIndexa].plane->normal);
            if (minDot > (double)dot && DOT_THRESHOLD > (double)dot)
            {
                minDot = dot;
                outSideIndex = sideIndexa + 6;
                plane = brush->sides[sideIndexa].plane;
                *outBrushPlane = plane->normal[0];
                outBrushPlane[1] = plane->normal[1];
                outBrushPlane[2] = plane->normal[2];
                outBrushPlane[3] = brush->sides[sideIndexa].plane->dist;
            }
        }
    }
    return outSideIndex;
}

char __cdecl Phys_TestBoxAgainstEachBrushPlane(
    const cbrush_t *brush,
    const objInfo *info,
    float *outBrushPlane,
    int *outSideIndex,
    float *outMaxSeparation)
{
    float v6; // [esp+0h] [ebp-B0h]
    float v7; // [esp+4h] [ebp-ACh]
    float v8; // [esp+8h] [ebp-A8h]
    float v9; // [esp+Ch] [ebp-A4h]
    float v10; // [esp+3Ch] [ebp-74h]
    float v11; // [esp+40h] [ebp-70h]
    float v12; // [esp+44h] [ebp-6Ch]
    float v13; // [esp+48h] [ebp-68h]
    float v14; // [esp+4Ch] [ebp-64h]
    float v15; // [esp+50h] [ebp-60h]
    int c; // [esp+58h] [ebp-58h]
    int r; // [esp+5Ch] [ebp-54h]
    float distg; // [esp+60h] [ebp-50h]
    float dist; // [esp+60h] [ebp-50h]
    float dista; // [esp+60h] [ebp-50h]
    float distb; // [esp+60h] [ebp-50h]
    float distc; // [esp+60h] [ebp-50h]
    float distd; // [esp+60h] [ebp-50h]
    float diste; // [esp+60h] [ebp-50h]
    float distf; // [esp+60h] [ebp-50h]
    cplane_s *brushPlane; // [esp+64h] [ebp-4Ch]
    float rotatedNormal[3]; // [esp+68h] [ebp-48h] BYREF
    float radius; // [esp+74h] [ebp-3Ch]
    unsigned int sideIndex; // [esp+78h] [ebp-38h]
    float absR[3][3]; // [esp+7Ch] [ebp-34h] BYREF
    float maxs[3]; // [esp+A0h] [ebp-10h] BYREF
    float maxSeparation; // [esp+ACh] [ebp-4h]

    if (!brush)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 989, 0, "%s", "brush");
    if (!info)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 990, 0, "%s", "info");
    if (!outBrushPlane)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 991, 0, "%s", "outBrushPlane");
    if (!outSideIndex)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 992, 0, "%s", "outSideIndex");
    if (!outMaxSeparation)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 993, 0, "%s", "outMaxSeparation");
    *outSideIndex = -1;
    *outBrushPlane = 0.0;
    outBrushPlane[1] = 0.0;
    outBrushPlane[2] = 0.0;
    outBrushPlane[3] = 0.0;
    for (r = 0; r < 3; ++r)
    {
        for (c = 0; c < 3; ++c)
        {
            v9 = fabs(info->R[r][c]);
            absR[r][c] = v9;
        }
    }
    MatrixTransformVector(info->u.sideExtents, absR, maxs);
    maxSeparation = -3.4028235e38;
    if (info->pos[0] - brush->maxs[0] > 0.0)
    {
        distg = info->pos[0] - brush->maxs[0] - maxs[0];
        if (distg >= 0.0)
            return 0;
        if (brush->edgeCount[1][0])
        {
            maxSeparation = info->pos[0] - brush->maxs[0] - maxs[0];
            v15 = brush->maxs[0];
            *outBrushPlane = 1.0;
            outBrushPlane[1] = 0.0;
            outBrushPlane[2] = 0.0;
            outBrushPlane[3] = v15;
            *outSideIndex = 1;
        }
    }
    if (brush->mins[0] - info->pos[0] > 0.0)
    {
        dist = brush->mins[0] - info->pos[0] - maxs[0];
        if (dist >= 0.0)
            return 0;
        if (brush->edgeCount[0][0] && maxSeparation < (double)dist)
        {
            maxSeparation = brush->mins[0] - info->pos[0] - maxs[0];
            v14 = -brush->mins[0];
            *outBrushPlane = -1.0;
            outBrushPlane[1] = 0.0;
            outBrushPlane[2] = 0.0;
            outBrushPlane[3] = v14;
            *outSideIndex = 0;
        }
    }
    if (info->pos[1] - brush->maxs[1] > 0.0)
    {
        dista = info->pos[1] - brush->maxs[1] - maxs[1];
        if (dista >= 0.0)
            return 0;
        if (brush->edgeCount[1][1] && maxSeparation < (double)dista)
        {
            maxSeparation = info->pos[1] - brush->maxs[1] - maxs[1];
            v13 = brush->maxs[1];
            *outBrushPlane = 0.0;
            outBrushPlane[1] = 1.0;
            outBrushPlane[2] = 0.0;
            outBrushPlane[3] = v13;
            *outSideIndex = 3;
        }
    }
    if (brush->mins[1] - info->pos[1] > 0.0)
    {
        distb = brush->mins[1] - info->pos[1] - maxs[1];
        if (distb >= 0.0)
            return 0;
        if (brush->edgeCount[0][1] && maxSeparation < (double)distb)
        {
            maxSeparation = brush->mins[1] - info->pos[1] - maxs[1];
            v12 = -brush->mins[1];
            *outBrushPlane = 0.0;
            outBrushPlane[1] = -1.0;
            outBrushPlane[2] = 0.0;
            outBrushPlane[3] = v12;
            *outSideIndex = 2;
        }
    }
    if (info->pos[2] - brush->maxs[2] > 0.0)
    {
        distc = info->pos[2] - brush->maxs[2] - maxs[2];
        if (distc >= 0.0)
            return 0;
        if (brush->edgeCount[1][2] && maxSeparation < (double)distc)
        {
            maxSeparation = info->pos[2] - brush->maxs[2] - maxs[2];
            v11 = brush->maxs[2];
            *outBrushPlane = 0.0;
            outBrushPlane[1] = 0.0;
            outBrushPlane[2] = 1.0;
            outBrushPlane[3] = v11;
            *outSideIndex = 5;
        }
    }
    if (brush->mins[2] - info->pos[2] > 0.0)
    {
        distd = brush->mins[2] - info->pos[2] - maxs[2];
        if (distd >= 0.0)
            return 0;
        if (brush->edgeCount[0][2] && maxSeparation < (double)distd)
        {
            maxSeparation = brush->mins[2] - info->pos[2] - maxs[2];
            v10 = -brush->mins[2];
            *outBrushPlane = 0.0;
            outBrushPlane[1] = 0.0;
            outBrushPlane[2] = -1.0;
            outBrushPlane[3] = v10;
            *outSideIndex = 4;
        }
    }
    for (sideIndex = 0; sideIndex < brush->numsides; ++sideIndex)
    {
        brushPlane = brush->sides[sideIndex].plane;
        if ((COERCE_UNSIGNED_INT(brushPlane->dist) & 0x7F800000) == 0x7F800000)
            MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1106, 0, "%s", "!IS_NAN(brushPlane->dist)");
        if ((COERCE_UNSIGNED_INT(brushPlane->normal[0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(brushPlane->normal[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(brushPlane->normal[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\physics\\phys_coll_boxbrush.cpp",
                1107,
                0,
                "%s",
                "!IS_NAN((brushPlane->normal)[0]) && !IS_NAN((brushPlane->normal)[1]) && !IS_NAN((brushPlane->normal)[2])");
        }
        MatrixTransformVector(brushPlane->normal, info->RTransposed, rotatedNormal);
        v8 = fabs(rotatedNormal[0]);
        v7 = fabs(rotatedNormal[1]);
        v6 = fabs(rotatedNormal[2]);
        radius = v8 * info->u.sideExtents[0] + v7 * info->u.sideExtents[1] + v6 * info->u.sideExtents[2];
        diste = Vec3Dot(info->pos, brushPlane->normal) - brushPlane->dist;
        if ((LODWORD(diste) & 0x7F800000) == 0x7F800000)
            MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1114, 0, "%s", "!IS_NAN(dist)");
        if (diste > 0.0)
        {
            distf = diste - radius;
            if (distf >= 0.0)
                return 0;
            if (brush->sides[sideIndex].edgeCount && maxSeparation < (double)distf)
            {
                maxSeparation = distf;
                *outBrushPlane = brushPlane->normal[0];
                outBrushPlane[1] = brushPlane->normal[1];
                outBrushPlane[2] = brushPlane->normal[2];
                outBrushPlane[3] = brushPlane->dist;
                *outSideIndex = sideIndex + 6;
            }
        }
    }
    *outMaxSeparation = maxSeparation;
    return 1;
}

void __cdecl Phys_CollideBoxWithBrushFace(
    const cbrush_t *brush,
    unsigned int brushSideIndex,
    const float *bestBrushPlane,
    const Poly *brushWinding,
    const objInfo *info,
    Results *results,
    float *collisionNormal)
{
    float v7; // [esp+Ch] [ebp-68h]
    float v8; // [esp+10h] [ebp-64h]
    float scale; // [esp+14h] [ebp-60h]
    int axisY; // [esp+1Ch] [ebp-58h]
    float boxFaceCorners[4][3]; // [esp+20h] [ebp-54h] BYREF
    int surfaceFlags; // [esp+50h] [ebp-24h]
    int boxAxis; // [esp+54h] [ebp-20h] BYREF
    int boxSign; // [esp+58h] [ebp-1Ch] BYREF
    int axisX; // [esp+5Ch] [ebp-18h]
    Poly boxPoly; // [esp+60h] [ebp-14h] BYREF
    float boxCorner[3]; // [esp+68h] [ebp-Ch] BYREF

    if (brushSideIndex >= brush->numsides + 6)
        MyAssertHandler(
            ".\\physics\\phys_coll_boxbrush.cpp",
            1149,
            0,
            "brushSideIndex doesn't index static_cast< int >( brush->numsides ) + 6\n\t%i not in [0, %i)",
            brushSideIndex,
            brush->numsides + 6);
    *collisionNormal = -*bestBrushPlane;
    collisionNormal[1] = -bestBrushPlane[1];
    collisionNormal[2] = -bestBrushPlane[2];
    GetClosestBoxFace(info, bestBrushPlane, &boxAxis, &boxSign);
    axisX = (boxAxis + 1) % 3;
    axisY = (axisX + 1) % 3;
    boxCorner[0] = info->u.sideExtents[0];
    boxCorner[1] = info->u.sideExtents[1];
    boxCorner[2] = info->u.sideExtents[2];
    if (boxSign)
        boxCorner[boxAxis] = -boxCorner[boxAxis];
    MatrixTransformVector(boxCorner, info->R, boxFaceCorners[0]);
    Vec3Add(info->pos, boxFaceCorners[0], boxFaceCorners[0]);
    scale = info->u.sideExtents[axisX] * -2.0;
    Vec3Mad(boxFaceCorners[0], scale, info->R[axisX], boxFaceCorners[1]);
    v8 = info->u.sideExtents[axisY] * -2.0;
    Vec3Mad(boxFaceCorners[1], v8, info->R[axisY], boxFaceCorners[2]);
    v7 = info->u.sideExtents[axisX] * 2.0;
    Vec3Mad(boxFaceCorners[2], v7, info->R[axisX], boxFaceCorners[3]);
    surfaceFlags = Phys_GetSurfaceFlagsFromBrush(brush, brushSideIndex);
    boxPoly.ptCount = 4;
    boxPoly.pts = boxFaceCorners;
    Phys_ProjectFaceOntoFaceAndClip(bestBrushPlane, brushWinding, &boxPoly, surfaceFlags, results, collisionNormal);
}

void __cdecl GetClosestBoxFace(const objInfo *info, const float *normal, int *minAxis, int *minSign)
{
    int sign; // [esp+0h] [ebp-10h]
    float minDot; // [esp+4h] [ebp-Ch]
    int axis; // [esp+8h] [ebp-8h]
    float dot; // [esp+Ch] [ebp-4h]

    minDot = 1.0;
    *minAxis = -1;
    *minSign = 0;
    for (axis = 0; axis < 3; ++axis)
    {
        for (sign = 0; sign <= 1; ++sign)
        {
            dot = Vec3Dot(normal, info->R[axis]);
            if (sign)
                dot = -dot;
            if (minDot > (double)dot)
            {
                minDot = dot;
                *minAxis = axis;
                *minSign = sign;
            }
        }
    }
    if (minDot >= 0.0)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 677, 1, "%s", "minDot < 0");
}

char __cdecl Phys_DoesPolyIntersectBox(const Poly *poly, const objInfo *info)
{
    unsigned int clippedPtsCount; // [esp+4h] [ebp-C1Ch]
    int clippedPtsCounta; // [esp+4h] [ebp-C1Ch]
    float testPlane[4]; // [esp+8h] [ebp-C18h] BYREF
    float clippedPts[256][3]; // [esp+18h] [ebp-C08h] BYREF
    unsigned int axis; // [esp+C18h] [ebp-8h]
    float centerDist; // [esp+C1Ch] [ebp-4h]

    if (poly->ptCount > 0x100)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1180, 0, "%s", "poly->ptCount <= ARRAY_COUNT( clippedPts )");
    clippedPtsCount = poly->ptCount;
    memcpy((unsigned __int8 *)clippedPts, (unsigned __int8 *)poly->pts, 12 * clippedPtsCount);
    for (axis = 0; axis < 3; ++axis)
    {
        testPlane[0] = info->R[axis][0];
        testPlane[1] = info->R[axis][1];
        testPlane[2] = info->R[axis][2];
        centerDist = Vec3Dot(info->pos, testPlane);
        testPlane[3] = info->u.sideExtents[axis] + centerDist;
        clippedPtsCounta = Phys_ClipPolyAgainstPlane(clippedPts, clippedPtsCount, 0x100u, testPlane);
        if (!clippedPtsCounta)
            return 0;
        testPlane[0] = -testPlane[0];
        testPlane[1] = -testPlane[1];
        testPlane[2] = -testPlane[2];
        testPlane[3] = info->u.sideExtents[axis] - centerDist;
        clippedPtsCount = Phys_ClipPolyAgainstPlane(clippedPts, clippedPtsCounta, 0x100u, testPlane);
        if (!clippedPtsCount)
            return 0;
    }
    return 1;
}

void __cdecl Phys_CollideOrientedBrushWithBrush(
    const cbrush_t *orientedBrush,
    const cbrush_t *fixedBrush,
    const objInfo *input,
    Results *results)
{
    double v4; // st7
    bool v5; // [esp+10h] [ebp-39B8h]
    bool v6; // [esp+14h] [ebp-39B4h]
    cplane_s *v7; // [esp+18h] [ebp-39B0h]
    int v8; // [esp+48h] [ebp-3980h]
    cplane_s *v9; // [esp+4Ch] [ebp-397Ch]
    float plane; // [esp+58h] [ebp-3970h] BYREF
    float v11; // [esp+5Ch] [ebp-396Ch]
    float v12; // [esp+60h] [ebp-3968h]
    float dist; // [esp+64h] [ebp-3964h]
    float v14; // [esp+68h] [ebp-3960h]
    unsigned int k; // [esp+6Ch] [ebp-395Ch]
    float v16; // [esp+70h] [ebp-3958h]
    float v17[8][4]; // [esp+74h] [ebp-3954h] BYREF
    float v18[4]; // [esp+100h] [ebp-38C8h] BYREF
    Poly verts; // [esp+110h] [ebp-38B8h] BYREF
    int outSideIndex; // [esp+118h] [ebp-38B0h] BYREF
    float bestFixedBrushPlane; // [esp+11Ch] [ebp-38ACh] BYREF
    float v22; // [esp+120h] [ebp-38A8h]
    float v23; // [esp+124h] [ebp-38A4h]
    float v24; // [esp+128h] [ebp-38A0h]
    bool v25; // [esp+12Fh] [ebp-3899h]
    unsigned int vertCount; // [esp+130h] [ebp-3898h]
    float v27; // [esp+134h] [ebp-3894h]
    float referenceBrushPlane[4]; // [esp+138h] [ebp-3890h] BYREF
    float outPlane[256][4]; // [esp+148h] [ebp-3880h] BYREF
    float v30[769]; // [esp+1148h] [ebp-2880h] BYREF
    unsigned int i; // [esp+1D4Ch] [ebp-1C7Ch]
    float outVerts[768]; // [esp+1D50h] [ebp-1C78h] BYREF
    unsigned int j; // [esp+2950h] [ebp-1078h]
    bool v34; // [esp+2957h] [ebp-1071h]
    unsigned int v35; // [esp+2958h] [ebp-1070h]
    unsigned int m; // [esp+295Ch] [ebp-106Ch]
    float axialPlanes[6][4]; // [esp+2960h] [ebp-1068h] BYREF
    Poly outPolys[256]; // [esp+29C0h] [ebp-1008h] BYREF
    Poly poly; // [esp+31C0h] [ebp-808h] BYREF
    Poly v40[5]; // [esp+31C8h] [ebp-800h] BYREF
    Poly v41[250]; // [esp+31F0h] [ebp-7D8h] BYREF
    float outMaxSeparation; // [esp+39C0h] [ebp-8h] BYREF
    int fixedBrushSideIndex; // [esp+39C4h] [ebp-4h]

    if (results->contactCount >= results->maxContacts)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1562, 0, "%s", "results->contactCount < results->maxContacts");
    if (orientedBrush->numsides >= 0x100)
        MyAssertHandler(
            ".\\physics\\phys_coll_boxbrush.cpp",
            1565,
            0,
            "orientedBrush->numsides doesn't index ARRAY_COUNT( transformedPlanes )\n\t%i not in [0, %i)",
            orientedBrush->numsides,
            256);
    CM_BuildAxialPlanes(orientedBrush, (float (*)[4])axialPlanes);
    for (i = 0; i < 6; ++i)
        Phys_TransformPlane(
            (const float *)&axialPlanes[i],
            *((float *)&axialPlanes[i] + 3),
            input->pos,
            input->R,
            (float *)&outPlane[i]);
    for (i = 0; i < orientedBrush->numsides; ++i)
        Phys_TransformPlane(
            orientedBrush->sides[i].plane->normal,
            orientedBrush->sides[i].plane->dist,
            input->pos,
            input->R,
            (float *)&outPlane[i + 6]);
    v35 = Phys_BuildWindingsForBrush(
        orientedBrush,
        (const float (*)[4])outPlane,
        outPolys,
        0x100u,
        (float (*)[3])outVerts,
        0x100u);
    if (v35)
    {
        if (phys_drawCollisionObj->current.enabled)
        {
            for (j = 0; j < orientedBrush->numsides + 6; ++j)
                Phys_DrawPoly(&outPolys[j], colorLtGreen);
        }
        verts.pts = (float (*)[3])outVerts;
        verts.ptCount = v35;
        if (!fixedBrush)
            MyAssertHandler("c:\\trees\\cod3\\src\\physics\\phys_coll_local.h", 175, 0, "%s", "brush");
        bestFixedBrushPlane = 0.0;
        v22 = 0.0;
        v23 = 0.0;
        v24 = 0.0;
        v16 = -3.4028235e38;
        fixedBrushSideIndex = -1;
        CM_BuildAxialPlanes(fixedBrush, (float (*)[4])v17);
        for (k = 0; k < 6; ++k)
        {
            v14 = Phys_TestVertsAgainstPlane_Wrapper((const float *)&v17[k], &verts);
            if (v14 >= 0.0)
            {
                v8 = 0;
                goto LABEL_37;
            }
            if (fixedBrush->edgeCount[k & 1][k >> 1] && v16 < (double)v14)
            {
                v16 = v14;
                fixedBrushSideIndex = k;
                bestFixedBrushPlane = *(float *)&v17[k];
                v22 = *((float *)&v17[k] + 1);
                v23 = *((float *)&v17[k] + 2);
                v24 = *((float *)&v17[k] + 3);
            }
        }
        for (k = 0; k < fixedBrush->numsides; ++k)
        {
            v9 = fixedBrush->sides[k].plane;
            plane = v9->normal[0];
            v11 = v9->normal[1];
            v12 = v9->normal[2];
            dist = fixedBrush->sides[k].plane->dist;
            v14 = Phys_TestVertsAgainstPlane_Wrapper(&plane, &verts);
            if (v14 >= 0.0)
            {
                v8 = 0;
                goto LABEL_37;
            }
            if (fixedBrush->sides[k].edgeCount && v16 < (double)v14)
            {
                v16 = v14;
                fixedBrushSideIndex = k + 6;
                bestFixedBrushPlane = plane;
                v22 = v11;
                v23 = v12;
                v24 = dist;
            }
        }
        v27 = v16;
        v8 = 1;
    LABEL_37:
        if (v8)
        {
            vertCount = Phys_BuildWindingsForBrush2(fixedBrush, &poly, 0x100u, (float (*)[3])v30, 0x100u);
            if (vertCount)
            {
                if (phys_drawCollisionWorld->current.enabled)
                {
                    for (j = 0; j < fixedBrush->numsides + 6; ++j)
                        Phys_DrawPoly(&poly + j, colorCyan);
                }
                if (Phys_TestVertsAgainstPlanes(
                    (const float (*)[3])v30,
                    vertCount,
                    orientedBrush,
                    (const float (*)[4])outPlane,
                    referenceBrushPlane,
                    &outSideIndex,
                    &outMaxSeparation))
                {
                    v6 = fixedBrushSideIndex != -1 && *(&poly.ptCount + 2 * fixedBrushSideIndex);
                    v25 = v6;
                    v5 = outSideIndex != -1 && outPolys[outSideIndex].ptCount;
                    v34 = v5;
                    if (v25 || v34)
                    {
                        if (!v25 || outMaxSeparation >= (double)v27 && v34)
                        {
                            if (!v34)
                                MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1616, 1, "%s", "isOrientedBrushPlaneValid");
                            if (outMaxSeparation < (double)v27 && v25)
                                MyAssertHandler(
                                    ".\\physics\\phys_coll_boxbrush.cpp",
                                    1617,
                                    1,
                                    "%s",
                                    "fixedBrushPlaneSeparation <= orientedBrushPlaneSeparation || !isFixedBrushPlaneValid");
                            fixedBrushSideIndex = Phys_CollideBrushAgainstBrushFace(
                                fixedBrush,
                                &poly,
                                orientedBrush,
                                outSideIndex,
                                referenceBrushPlane,
                                outPolys,
                                results);
                        }
                        else
                        {
                            Phys_CollideOrientedBrushAgainstFixedBrushFace(
                                fixedBrush,
                                fixedBrushSideIndex,
                                &bestFixedBrushPlane,
                                &poly,
                                orientedBrush,
                                outPolys,
                                (const float (*)[4])outPlane,
                                results);
                        }
                        if (input->isNarrow)
                        {
                            CM_BuildAxialPlanes(fixedBrush, (float (*)[4])axialPlanes);
                            for (m = 0; m < 3; ++m)
                            {
                                if (*(&poly.ptCount + 4 * m) && fixedBrush->mins[m] > (double)input->pos[m])
                                {
                                    if (2 * m != fixedBrushSideIndex)
                                    {
                                        if (Phys_DoesPolyIntersectOrientedBrush(
                                            &poly + 2 * m,
                                            (const float (*)[4])outPlane,
                                            orientedBrush->numsides + 6))
                                        {
                                            Phys_CollideOrientedBrushAgainstFixedBrushFace(
                                                fixedBrush,
                                                2 * m,
                                                (float *)&axialPlanes[2 * m],
                                                &poly,
                                                orientedBrush,
                                                outPolys,
                                                (const float (*)[4])outPlane,
                                                results);
                                        }
                                    }
                                }
                                else if (v40[2 * m].ptCount
                                    && fixedBrush->maxs[m] < (double)input->pos[m]
                                    && 2 * m + 1 != fixedBrushSideIndex
                                        && Phys_DoesPolyIntersectOrientedBrush(
                                            &v40[2 * m],
                                            (const float (*)[4])outPlane,
                                            orientedBrush->numsides + 6))
                                {
                                    Phys_CollideOrientedBrushAgainstFixedBrushFace(
                                        fixedBrush,
                                        2 * m + 1,
                                        (float *)&axialPlanes[2 * m + 1],
                                        &poly,
                                        orientedBrush,
                                        outPolys,
                                        (const float (*)[4])outPlane,
                                        results);
                                }
                            }
                            for (m = 0; m < fixedBrush->numsides; ++m)
                            {
                                if (m + 6 != fixedBrushSideIndex)
                                {
                                    if (v41[m].ptCount)
                                    {
                                        v4 = Vec3Dot(input->pos, fixedBrush->sides[m].plane->normal);
                                        if (fixedBrush->sides[m].plane->dist < v4)
                                        {
                                            if (Phys_DoesPolyIntersectOrientedBrush(
                                                &v41[m],
                                                (const float (*)[4])outPlane,
                                                orientedBrush->numsides + 6))
                                            {
                                                v7 = fixedBrush->sides[m].plane;
                                                v18[0] = v7->normal[0];
                                                v18[1] = v7->normal[1];
                                                v18[2] = v7->normal[2];
                                                v18[3] = fixedBrush->sides[m].plane->dist;
                                                Phys_CollideOrientedBrushAgainstFixedBrushFace(
                                                    fixedBrush,
                                                    m + 6,
                                                    v18,
                                                    &poly,
                                                    orientedBrush,
                                                    outPolys,
                                                    (const float (*)[4])outPlane,
                                                    results);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

unsigned int __cdecl Phys_BuildWindingsForBrush(
    const cbrush_t *brush,
    const float (*planes)[4],
    Poly *outPolys,
    unsigned int maxPolys,
    float (*outVerts)[3],
    unsigned int maxVerts)
{
    unsigned int sideIndex; // [esp+5Ch] [ebp-68h]
    unsigned int vertCount; // [esp+60h] [ebp-64h]
    float axialPlanes[6][4]; // [esp+64h] [ebp-60h] BYREF

    //Profile_Begin(381);
    if (!planes)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 934, 0, "%s", "planes");
    CM_BuildAxialPlanes(brush, axialPlanes);
    if (brush->numsides + 6 > maxPolys)
        MyAssertHandler(
            ".\\physics\\phys_coll_boxbrush.cpp",
            938,
            0,
            "brush->numsides + 6 <= maxPolys\n\t%i, %i",
            brush->numsides + 6,
            maxPolys);
    vertCount = 0;
    for (sideIndex = 0; sideIndex < brush->numsides + 6; ++sideIndex)
    {
        outPolys[sideIndex].pts = (float (*)[3]) & (*outVerts)[3 * vertCount];
        Phys_GetWindingForBrushFace(brush, planes, sideIndex, &outPolys[sideIndex], maxVerts - vertCount);
        vertCount += outPolys[sideIndex].ptCount;
        if (vertCount > maxVerts)
            MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 945, 0, "%s", "vertCount <= maxVerts");
    }
    //Profile_EndInternal(0);
    return vertCount;
}

void __cdecl Phys_GetWindingForBrushFace(
    const cbrush_t *brush,
    const float (*inPlanes)[4],
    unsigned int brushSide,
    Poly *outWinding,
    int maxVerts)
{
    float v5; // [esp+30h] [ebp-D0h]
    float v[5]; // [esp+3Ch] [ebp-C4h] BYREF
    float *a; // [esp+50h] [ebp-B0h]
    float *b; // [esp+54h] [ebp-ACh]
    float diff[3]; // [esp+58h] [ebp-A8h] BYREF
    float v10; // [esp+64h] [ebp-9Ch]
    float v11; // [esp+68h] [ebp-98h]
    float v12; // [esp+6Ch] [ebp-94h]
    int v13; // [esp+70h] [ebp-90h]
    float *v14; // [esp+74h] [ebp-8Ch]
    float v15; // [esp+78h] [ebp-88h]
    float v16; // [esp+7Ch] [ebp-84h]
    float v17; // [esp+80h] [ebp-80h]
    float v18; // [esp+84h] [ebp-7Ch]
    float v19; // [esp+88h] [ebp-78h]
    float v20; // [esp+8Ch] [ebp-74h]
    float v21; // [esp+90h] [ebp-70h]
    float v22; // [esp+94h] [ebp-6Ch]
    float *v23; // [esp+98h] [ebp-68h]
    float *v24; // [esp+9Ch] [ebp-64h]
    float *v25; // [esp+A0h] [ebp-60h]
    float *v26; // [esp+A4h] [ebp-5Ch]
    float *v27; // [esp+A8h] [ebp-58h]
    float *v28; // [esp+ACh] [ebp-54h]
    float *pt; // [esp+B0h] [ebp-50h]
    int edgeIndex; // [esp+B4h] [ebp-4Ch]
    float planes[3][4]; // [esp+B8h] [ebp-48h] BYREF
    int offset; // [esp+E8h] [ebp-18h]
    int side2; // [esp+ECh] [ebp-14h]
    int edgeCount; // [esp+F0h] [ebp-10h]
    int side1; // [esp+F4h] [ebp-Ch]
    unsigned __int8 *edges; // [esp+F8h] [ebp-8h]
    unsigned int nonAxialSideIndex; // [esp+FCh] [ebp-4h]

    if (!brush)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 731, 0, "%s", "brush");
    if (brushSide >= brush->numsides + 6)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 732, 0, "%s", "brushSide < brush->numsides + 6");
    if (!outWinding)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 733, 0, "%s", "outWinding");
    if (!outWinding->pts)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 734, 0, "%s", "outWinding->pts");
    if (!inPlanes)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 735, 0, "%s", "inPlanes");
    if (brushSide >= 6)
    {
        if (!brush->sides)
            MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 745, 0, "%s", "brush->sides");
        nonAxialSideIndex = brushSide - 6;
        offset = brush->sides[brushSide - 6].firstAdjacentSideOffset;
        edgeCount = brush->sides[brushSide - 6].edgeCount;
        v27 = (float *)&(*inPlanes)[4 * brushSide];
        planes[0][0] = *v27;
        planes[0][1] = v27[1];
        planes[0][2] = v27[2];
        planes[0][3] = v27[3];
    }
    else
    {
        offset = brush->firstAdjacentSideOffsets[brushSide & 1][brushSide >> 1];
        edgeCount = brush->edgeCount[brushSide & 1][brushSide >> 1];
        v28 = (float *)&(*inPlanes)[4 * brushSide];
        planes[0][0] = *v28;
        planes[0][1] = v28[1];
        planes[0][2] = v28[2];
        planes[0][3] = v28[3];
    }
    if (edgeCount >= 3 && edgeCount <= maxVerts)
    {
        if (!brush->baseAdjacentSide)
            MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 758, 0, "%s", "brush->baseAdjacentSide");
        edges = &brush->baseAdjacentSide[offset];
        side1 = edges[edgeCount - 1];
        if (side1 >= brush->numsides + 6)
            MyAssertHandler(
                ".\\physics\\phys_coll_boxbrush.cpp",
                762,
                0,
                "side1 doesn't index brush->numsides + 6\n\t%i not in [0, %i)",
                side1,
                brush->numsides + 6);
        v25 = planes[1];
        v26 = (float *)&(*inPlanes)[4 * side1];
        planes[1][0] = *v26;
        planes[1][1] = v26[1];
        planes[1][2] = v26[2];
        planes[1][3] = v26[3];
        outWinding->ptCount = 0;
        for (edgeIndex = 0; edgeIndex < edgeCount; ++edgeIndex)
        {
            side2 = edges[edgeIndex];
            if (side2 >= brush->numsides + 6)
                MyAssertHandler(
                    ".\\physics\\phys_coll_boxbrush.cpp",
                    769,
                    0,
                    "side2 doesn't index brush->numsides + 6\n\t%i not in [0, %i)",
                    side2,
                    brush->numsides + 6);
            v23 = planes[2];
            v24 = (float *)&(*inPlanes)[4 * side2];
            planes[2][0] = *v24;
            planes[2][1] = v24[1];
            planes[2][2] = v24[2];
            planes[2][3] = v24[3];
            v14 = outWinding->pts[outWinding->ptCount];
            v17 = planes[1][1] * planes[2][2] - planes[2][1] * planes[1][2];
            v18 = planes[2][1] * planes[0][2] - planes[0][1] * planes[2][2];
            v19 = planes[0][1] * planes[1][2] - planes[1][1] * planes[0][2];
            v20 = planes[0][0] * v17;
            v21 = planes[1][0] * v18;
            v22 = planes[2][0] * v19;
            v16 = v20 + v21 + v22;
            v5 = fabs(v16);
            if (v5 >= 0.001000000047497451)
            {
                v15 = 1.0 / v16;
                v20 = planes[0][3] * v17;
                v21 = planes[1][3] * v18;
                v22 = planes[2][3] * v19;
                *v14 = (v20 + v21 + v22) * v15;
                v20 = (planes[1][2] * planes[2][0] - planes[2][2] * planes[1][0]) * planes[0][3];
                v21 = (planes[2][2] * planes[0][0] - planes[0][2] * planes[2][0]) * planes[1][3];
                v22 = (planes[0][2] * planes[1][0] - planes[1][2] * planes[0][0]) * planes[2][3];
                v14[1] = (v20 + v21 + v22) * v15;
                v20 = (planes[1][0] * planes[2][1] - planes[2][0] * planes[1][1]) * planes[0][3];
                v21 = (planes[2][0] * planes[0][1] - planes[0][0] * planes[2][1]) * planes[1][3];
                v22 = (planes[0][0] * planes[1][1] - planes[1][0] * planes[0][1]) * planes[2][3];
                v14[2] = (v20 + v21 + v22) * v15;
                v13 = 1;
            }
            else
            {
                v13 = 0;
            }
            if (v13)
            {
                pt = outWinding->pts[outWinding->ptCount];
                v12 = *pt;
                if ((LODWORD(v12) & 0x7F800000) == 0x7F800000
                    || (v11 = pt[1], (LODWORD(v11) & 0x7F800000) == 0x7F800000)
                    || (v10 = pt[2], (LODWORD(v10) & 0x7F800000) == 0x7F800000))
                {
                    Com_PrintError(20, "Bad intersection of three planes:\n");
                    Com_PrintError(20, "Resulting point: (%f, %f, %f)\n", *pt, pt[1], pt[2]);
                    Com_PrintError(20, "plane 0: (%f, %f, %f, %f)\n", planes[0][0], planes[0][1], planes[0][2], planes[0][3]);
                    Com_PrintError(20, "plane 1: (%f, %f, %f, %f)\n", planes[1][0], planes[1][1], planes[1][2], planes[1][3]);
                    Com_PrintError(20, "plane 2: (%f, %f, %f, %f)\n", planes[2][0], planes[2][1], planes[2][2], planes[2][3]);
                    Com_PrintError(
                        20,
                        "brush mins/maxs: (%f, %f, %f) (%f, %f, %f)\n",
                        brush->mins[0],
                        brush->mins[1],
                        brush->mins[2],
                        brush->maxs[0],
                        brush->maxs[1],
                        brush->maxs[2]);
                    Com_PrintError(20, "brush numsides: %i\n", brush->numsides);
                    Com_PrintError(20, "brushside: %i\n", brushSide);
                    if (!alwaysfails)
                        MyAssertHandler(
                            ".\\physics\\phys_coll_boxbrush.cpp",
                            787,
                            0,
                            "Phys_GetWindingForBrushFace: bad intersection.  Please include console log in bug report.");
                }
                if (!outWinding->ptCount
                    || (a = outWinding->pts[outWinding->ptCount - 1],
                        b = outWinding->pts[outWinding->ptCount],
                        Vec3Sub(a, b, diff),
                        Vec3LengthSq(diff) >= 1.0))
                {
                    ++outWinding->ptCount;
                }
            }
            LODWORD(v[3]) = planes[1];
            LODWORD(v[4]) = planes[2];
            planes[1][0] = planes[2][0];
            planes[1][1] = planes[2][1];
            planes[1][2] = planes[2][2];
            planes[1][3] = planes[2][3];
        }
        if (outWinding->ptCount > 1)
        {
            Vec3Sub((const float *)outWinding->pts, outWinding->pts[outWinding->ptCount - 1], v);
            if (Vec3LengthSq(v) < 1.0)
                --outWinding->ptCount;
        }
        if (outWinding->ptCount < 3)
            outWinding->ptCount = 0;
    }
    else
    {
        outWinding->ptCount = 0;
    }
}

unsigned int __cdecl Phys_BuildWindingsForBrush2(
    const cbrush_t *brush,
    Poly *outPolys,
    unsigned int maxPolys,
    float (*outVerts)[3],
    unsigned int maxVerts)
{
    unsigned int sideIndex; // [esp+5Ch] [ebp-68h]
    unsigned int vertCount; // [esp+60h] [ebp-64h]
    float axialPlanes[6][4]; // [esp+64h] [ebp-60h] BYREF

    //Profile_Begin(381);
    CM_BuildAxialPlanes(brush, axialPlanes);
    if (brush->numsides + 6 > maxPolys)
        MyAssertHandler(
            ".\\physics\\phys_coll_boxbrush.cpp",
            963,
            0,
            "brush->numsides + 6 <= maxPolys\n\t%i, %i",
            brush->numsides + 6,
            maxPolys);
    vertCount = 0;
    for (sideIndex = 0; sideIndex < brush->numsides + 6; ++sideIndex)
    {
        outPolys[sideIndex].pts = (float (*)[3]) & (*outVerts)[3 * vertCount];
        Phys_GetWindingForBrushFace2(brush, sideIndex, &outPolys[sideIndex], maxVerts - vertCount, axialPlanes);
        vertCount += outPolys[sideIndex].ptCount;
        if (vertCount > maxVerts)
            MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 970, 0, "%s", "vertCount <= maxVerts");
    }
    //Profile_EndInternal(0);
    return vertCount;
}

double __cdecl Phys_TestVertsAgainstPlane_Wrapper(const float *plane, const Poly *verts)
{
    return Phys_TestVertsAgainstPlane(verts->pts, verts->ptCount, plane);
}

double __cdecl Phys_TestVertsAgainstPlane(const float (*verts)[3], unsigned int vertCount, const float *plane)
{
    double v3; // st7
    float v5; // [esp+0h] [ebp-18h]
    float v6; // [esp+4h] [ebp-14h]
    float dist; // [esp+Ch] [ebp-Ch]
    float minDist; // [esp+10h] [ebp-8h]
    unsigned int vertIndex; // [esp+14h] [ebp-4h]

    minDist = 3.4028235e38;
    for (vertIndex = 0; vertIndex < vertCount; ++vertIndex)
    {
        v3 = Vec3Dot(&(*verts)[3 * vertIndex], plane);
        dist = v3 - plane[3];
        v6 = dist - minDist;
        if (v6 < 0.0)
            v5 = v3 - plane[3];
        else
            v5 = minDist;
        minDist = v5;
    }
    if ((LODWORD(minDist) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1401, 0, "%s", "!IS_NAN(minDist)");
    return minDist;
}

char __cdecl Phys_TestVertsAgainstPlanes(
    const float (*verts)[3],
    unsigned int vertCount,
    const cbrush_t *brushContainingThePlanes,
    const float (*planes)[4],
    float *outPlane,
    int *outSideIndex,
    float *outMaxSeparation)
{
    float *v8; // [esp+0h] [ebp-10h]
    float dist; // [esp+4h] [ebp-Ch]
    unsigned int sideIndex; // [esp+8h] [ebp-8h]
    float maxSeparation; // [esp+Ch] [ebp-4h]

    maxSeparation = -3.4028235e38;
    *outSideIndex = -1;
    for (sideIndex = 0; sideIndex < brushContainingThePlanes->numsides + 6; ++sideIndex)
    {
        dist = Phys_TestVertsAgainstPlane(verts, vertCount, &(*planes)[4 * sideIndex]);
        if (dist >= 0.0)
            return 0;
        if (sideIndex >= 6)
        {
            if (!brushContainingThePlanes->sides[sideIndex - 6].edgeCount)
                continue;
        }
        else if (!brushContainingThePlanes->edgeCount[sideIndex & 1][sideIndex >> 1])
        {
            continue;
        }
        if (maxSeparation < (double)dist)
        {
            maxSeparation = dist;
            *outSideIndex = sideIndex;
            v8 = (float *)&(*planes)[4 * sideIndex];
            *outPlane = *v8;
            outPlane[1] = v8[1];
            outPlane[2] = v8[2];
            outPlane[3] = v8[3];
        }
    }
    *outMaxSeparation = maxSeparation;
    return 1;
}

void __cdecl Phys_TransformPlane(
    const float *normal,
    float dist,
    const float *translate,
    const float (*rotate)[3],
    float *outPlane)
{
    MatrixTransformVector(normal, *(const mat3x3*)rotate, outPlane);
    outPlane[3] = Vec3Dot(outPlane, translate) + dist;
}

void __cdecl Phys_CollideOrientedBrushAgainstFixedBrushFace(
    const cbrush_t *fixedBrush,
    unsigned int fixedBrushSideIndex,
    float *bestFixedBrushPlane,
    const Poly *fixedBrushPolys,
    const cbrush_t *orientedBrush,
    const Poly *orientedBrushPolys,
    const float (*transformedPlanes)[4],
    Results *results)
{
    int orientedBrushSideIndex; // [esp+0h] [ebp-24h]
    int surfaceFlags; // [esp+4h] [ebp-20h]
    float bestOrientedBrushPlane[4]; // [esp+8h] [ebp-1Ch] BYREF
    float collisionNormal[3]; // [esp+18h] [ebp-Ch] BYREF

    if (fixedBrushSideIndex >= fixedBrush->numsides + 6)
        MyAssertHandler(
            ".\\physics\\phys_coll_boxbrush.cpp",
            1488,
            0,
            "fixedBrushSideIndex doesn't index static_cast< int >( fixedBrush->numsides ) + 6\n\t%i not in [0, %i)",
            fixedBrushSideIndex,
            fixedBrush->numsides + 6);
    collisionNormal[0] = -*bestFixedBrushPlane;
    collisionNormal[1] = -bestFixedBrushPlane[1];
    collisionNormal[2] = -bestFixedBrushPlane[2];
    orientedBrushSideIndex = GetClosestOrientedBrushFace(
        bestFixedBrushPlane,
        orientedBrush,
        orientedBrushPolys,
        transformedPlanes,
        bestOrientedBrushPlane);
    if (orientedBrushSideIndex >= 0)
    {
        if (orientedBrushSideIndex >= orientedBrush->numsides + 6)
            MyAssertHandler(
                ".\\physics\\phys_coll_boxbrush.cpp",
                1493,
                0,
                "orientedBrushSideIndex doesn't index static_cast< int >( orientedBrush->numsides ) + 6\n\t%i not in [0, %i)",
                orientedBrushSideIndex,
                orientedBrush->numsides + 6);
        surfaceFlags = Phys_GetSurfaceFlagsFromBrush(fixedBrush, fixedBrushSideIndex);
        Phys_ProjectFaceOntoFaceAndClip(
            bestFixedBrushPlane,
            &fixedBrushPolys[fixedBrushSideIndex],
            &orientedBrushPolys[orientedBrushSideIndex],
            surfaceFlags,
            results,
            collisionNormal);
    }
}

int __cdecl GetClosestOrientedBrushFace(
    const float *normal,
    const cbrush_t *brush,
    const Poly *brushPolys,
    const float (*orientedPlanes)[4],
    float *outBrushPlane)
{
    float *v6; // [esp+0h] [ebp-14h]
    int outSideIndex; // [esp+4h] [ebp-10h]
    unsigned int sideIndex; // [esp+8h] [ebp-Ch]
    float minDot; // [esp+Ch] [ebp-8h]
    float dot; // [esp+10h] [ebp-4h]

    minDot = 2.0;
    outSideIndex = -1;
    for (sideIndex = 0; sideIndex < brush->numsides + 6; ++sideIndex)
    {
        if (brushPolys[sideIndex].ptCount)
        {
            dot = Vec3Dot(normal, &(*orientedPlanes)[4 * sideIndex]);
            if (minDot > (double)dot)
            {
                minDot = dot;
                outSideIndex = sideIndex;
                v6 = (float *)&(*orientedPlanes)[4 * sideIndex];
                *outBrushPlane = *v6;
                outBrushPlane[1] = v6[1];
                outBrushPlane[2] = v6[2];
                outBrushPlane[3] = v6[3];
            }
        }
    }
    if (minDot >= 2.0)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 644, 1, "%s", "minDot < 2");
    if (outSideIndex < 0)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 645, 1, "%s", "outSideIndex >= 0");
    return outSideIndex;
}

int __cdecl Phys_CollideBrushAgainstBrushFace(
    const cbrush_t *brush,
    const Poly *brushPolys,
    const cbrush_t *referenceBrush,
    unsigned int referenceBrushSideIndex,
    float *referenceBrushPlane,
    const Poly *referenceBrushPolys,
    Results *results)
{
    float bestFixedBrushPlane[4]; // [esp+0h] [ebp-24h] BYREF
    int surfaceFlags; // [esp+10h] [ebp-14h]
    float collisionNormal[3]; // [esp+14h] [ebp-10h] BYREF
    int fixedBrushSideIndex; // [esp+20h] [ebp-4h]

    if (referenceBrushSideIndex >= referenceBrush->numsides + 6)
        MyAssertHandler(
            ".\\physics\\phys_coll_boxbrush.cpp",
            1506,
            0,
            "referenceBrushSideIndex doesn't index static_cast< int >( referenceBrush->numsides ) + 6\n\t%i not in [0, %i)",
            referenceBrushSideIndex,
            referenceBrush->numsides + 6);
    collisionNormal[0] = *referenceBrushPlane;
    collisionNormal[1] = referenceBrushPlane[1];
    collisionNormal[2] = referenceBrushPlane[2];
    fixedBrushSideIndex = GetClosestBrushFace(collisionNormal, brush, brushPolys, bestFixedBrushPlane);
    if (fixedBrushSideIndex >= 0)
    {
        if (fixedBrushSideIndex >= brush->numsides + 6)
            MyAssertHandler(
                ".\\physics\\phys_coll_boxbrush.cpp",
                1511,
                0,
                "fixedBrushSideIndex doesn't index static_cast< int >( brush->numsides ) + 6\n\t%i not in [0, %i)",
                fixedBrushSideIndex,
                brush->numsides + 6);
        surfaceFlags = Phys_GetSurfaceFlagsFromBrush(brush, fixedBrushSideIndex);
        Phys_ProjectFaceOntoFaceAndClip(
            referenceBrushPlane,
            &referenceBrushPolys[referenceBrushSideIndex],
            &brushPolys[fixedBrushSideIndex],
            surfaceFlags,
            results,
            collisionNormal);
    }
    return fixedBrushSideIndex;
}

char __cdecl Phys_DoesPolyIntersectOrientedBrush(
    const Poly *poly,
    const float (*transformedPlanes)[4],
    unsigned int brushSides)
{
    unsigned int clippedPtsCount; // [esp+0h] [ebp-C0Ch]
    float clippedPts[256][3]; // [esp+4h] [ebp-C08h] BYREF
    unsigned int sideIndex; // [esp+C08h] [ebp-4h]

    if (poly->ptCount > 0x100)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1526, 0, "%s", "poly->ptCount <= ARRAY_COUNT( clippedPts )");
    clippedPtsCount = poly->ptCount;
    memcpy((unsigned __int8 *)clippedPts, (unsigned __int8 *)poly->pts, 12 * clippedPtsCount);
    for (sideIndex = 0; sideIndex < brushSides; ++sideIndex)
    {
        clippedPtsCount = Phys_ClipPolyAgainstPlane(
            clippedPts,
            clippedPtsCount,
            0x100u,
            (float *)&(*transformedPlanes)[4 * sideIndex]);
        if (!clippedPtsCount)
            return 0;
    }
    return 1;
}

void __cdecl Phys_CollideOrientedBrushModelWithBrush(const cbrush_t *fixedBrush, const objInfo *info, Results *results)
{
    BrushBrushData data; // [esp+0h] [ebp-24h] BYREF
    float mins[3]; // [esp+Ch] [ebp-18h] BYREF
    float maxs[3]; // [esp+18h] [ebp-Ch] BYREF

    if (results->contactCount >= results->maxContacts)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1691, 0, "%s", "results->contactCount < results->maxContacts");
    mins[0] = -3.4028235e38;
    mins[1] = -3.4028235e38;
    mins[2] = -3.4028235e38;
    maxs[0] = 3.4028235e38;
    maxs[1] = 3.4028235e38;
    maxs[2] = 3.4028235e38;
    data.fixedBrush = fixedBrush;
    data.input = info;
    data.results = results;
    CM_ForEachBrushInLeafBrushNode_r(
        &cm.leafbrushNodes[info->u.brushModel->leaf.leafBrushNode],
        mins,
        maxs,
        0,
        0,
        Phys_CollideOrientedBrushWithBrush_Wrapper,
        &data);
}

void __cdecl Phys_CollideOrientedBrushWithBrush_Wrapper(const cbrush_t *orientedBrush, void *userData)
{
    Results *results; // [esp+4h] [ebp-4h]

    if (!userData)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1673, 0, "%s", "userData");
    results = (Results *)*((unsigned int *)userData + 2);
    if (results->contactCount < results->maxContacts)
        Phys_CollideOrientedBrushWithBrush(
            orientedBrush,
            *(const cbrush_t **)userData,
            *((const objInfo **)userData + 1),
            results);
}

void __cdecl Phys_CollideOrientedBrushWithTriangleList(
    const cbrush_t *orientedBrush,
    const unsigned __int16 *indices,
    const float (*verts)[3],
    int triCount,
    const objInfo *input,
    int surfaceFlags,
    Results *results)
{
    float *v7; // [esp+A4h] [ebp-78h]
    float *v8; // [esp+ACh] [ebp-70h]
    float *v9; // [esp+B0h] [ebp-6Ch]
    float pos[3]; // [esp+B8h] [ebp-64h] BYREF
    float triVerts[3][3]; // [esp+C4h] [ebp-58h] BYREF
    float triPlane[4]; // [esp+E8h] [ebp-34h] BYREF
    BrushTrimeshData data; // [esp+F8h] [ebp-24h] BYREF
    float radius; // [esp+110h] [ebp-Ch]
    int i; // [esp+114h] [ebp-8h]
    const unsigned __int16 *pIndices; // [esp+118h] [ebp-4h]

    data.input = input;
    data.results = results;
    data.indices = indices;
    data.verts = verts;
    data.triCount = triCount;
    data.surfaceFlags = surfaceFlags;
    pos[0] = input->pos[0];
    pos[1] = input->pos[1];
    pos[2] = input->pos[2];
    radius = input->radius;
    pIndices = indices;
    for (i = 0; i < triCount && results->contactCount < results->maxContacts; ++i)
    {
        v9 = (float *)&(*verts)[3 * *pIndices];
        triVerts[0][0] = *v9;
        triVerts[0][1] = v9[1];
        triVerts[0][2] = v9[2];
        v8 = (float *)&(*verts)[3 * *++pIndices];
        triVerts[1][0] = *v8;
        triVerts[1][1] = v8[1];
        triVerts[1][2] = v8[2];
        v7 = (float *)&(*verts)[3 * *++pIndices];
        triVerts[2][0] = *v7;
        triVerts[2][1] = v7[1];
        triVerts[2][2] = v7[2];
        ++pIndices;
        if (Phys_GetPlaneForTriangle2(triVerts, pos, radius, triPlane))
            Phys_CollideOrientedBrushWithTriangle(orientedBrush, triVerts[0], triVerts[1], triVerts[2], &data);
    }
}

int __cdecl Phys_GetPlaneForTriangle2(const float (*triangle)[3], const float *origin, float radius, float *result)
{
    float v5; // [esp+0h] [ebp-108h]
    float v6; // [esp+4h] [ebp-104h]
    float v7; // [esp+8h] [ebp-100h]
    float v8; // [esp+Ch] [ebp-FCh]
    float v9; // [esp+10h] [ebp-F8h]
    float v10; // [esp+14h] [ebp-F4h]
    float v11; // [esp+18h] [ebp-F0h]
    float v12; // [esp+1Ch] [ebp-ECh]
    float v13; // [esp+20h] [ebp-E8h]
    float v14; // [esp+24h] [ebp-E4h]
    float v15; // [esp+28h] [ebp-E0h]
    float v16; // [esp+2Ch] [ebp-DCh]
    float v17; // [esp+30h] [ebp-D8h]
    float v18; // [esp+34h] [ebp-D4h]
    float v19; // [esp+38h] [ebp-D0h]
    float v20; // [esp+3Ch] [ebp-CCh]
    float v21; // [esp+40h] [ebp-C8h]
    float v22; // [esp+44h] [ebp-C4h]
    float v23; // [esp+48h] [ebp-C0h]
    float v24; // [esp+64h] [ebp-A4h]
    float v25; // [esp+74h] [ebp-94h]
    float v26; // [esp+78h] [ebp-90h]
    float v27; // [esp+7Ch] [ebp-8Ch]
    float v28; // [esp+80h] [ebp-88h]
    float v29; // [esp+84h] [ebp-84h]
    float v30; // [esp+88h] [ebp-80h]
    float v31; // [esp+8Ch] [ebp-7Ch]
    float v32; // [esp+90h] [ebp-78h]
    float v33; // [esp+94h] [ebp-74h]
    float v34; // [esp+98h] [ebp-70h]
    float v35; // [esp+9Ch] [ebp-6Ch]
    float v36; // [esp+A0h] [ebp-68h]
    float v37; // [esp+A4h] [ebp-64h]
    float v38; // [esp+A8h] [ebp-60h]
    float v39; // [esp+ACh] [ebp-5Ch]
    float v40; // [esp+B0h] [ebp-58h]
    float clipPlane[3]; // [esp+B4h] [ebp-54h] BYREF
    float mins[3]; // [esp+C0h] [ebp-48h] BYREF
    float triangleEdge[3][3]; // [esp+CCh] [ebp-3Ch] BYREF
    float maxs[3]; // [esp+F0h] [ebp-18h] BYREF
    float point[3]; // [esp+FCh] [ebp-Ch] BYREF

    Vec3Sub((const float *)triangle, &(*triangle)[3], triangleEdge[0]);
    if (triangleEdge[0][0] < 0.0)
        v39 = (*triangle)[0];
    else
        v39 = (*triangle)[3];
    v40 = (*triangle)[6];
    v23 = v40 - v39;
    if (v23 < 0.0)
        v22 = v40;
    else
        v22 = v39;
    mins[0] = v22;
    if (triangleEdge[0][1] < 0.0)
        v37 = (*triangle)[1];
    else
        v37 = (*triangle)[4];
    v38 = (*triangle)[7];
    v21 = v38 - v37;
    if (v21 < 0.0)
        v20 = v38;
    else
        v20 = v37;
    mins[1] = v20;
    if (triangleEdge[0][2] < 0.0)
        v35 = (*triangle)[2];
    else
        v35 = (*triangle)[5];
    v36 = (*triangle)[8];
    v19 = v36 - v35;
    if (v19 < 0.0)
        v18 = v36;
    else
        v18 = v35;
    mins[2] = v18;
    if (triangleEdge[0][0] < 0.0)
        v33 = (*triangle)[3];
    else
        v33 = (*triangle)[0];
    v34 = (*triangle)[6];
    v17 = v33 - v34;
    if (v17 < 0.0)
        v16 = v34;
    else
        v16 = v33;
    maxs[0] = v16;
    if (triangleEdge[0][1] < 0.0)
        v31 = (*triangle)[4];
    else
        v31 = (*triangle)[1];
    v32 = (*triangle)[7];
    v15 = v31 - v32;
    if (v15 < 0.0)
        v14 = v32;
    else
        v14 = v31;
    maxs[1] = v14;
    if (triangleEdge[0][2] < 0.0)
        v29 = (*triangle)[5];
    else
        v29 = (*triangle)[2];
    v30 = (*triangle)[8];
    v13 = v29 - v30;
    if (v13 < 0.0)
        v12 = v30;
    else
        v12 = v29;
    maxs[2] = v12;
    Vec3Sub(mins, origin, mins);
    Vec3Sub(origin, maxs, maxs);
    v11 = maxs[0] - maxs[1];
    if (v11 < 0.0)
        v28 = maxs[1];
    else
        v28 = maxs[0];
    v10 = v28 - maxs[2];
    if (v10 < 0.0)
        v26 = maxs[2];
    else
        v26 = v28;
    v9 = mins[0] - mins[1];
    if (v9 < 0.0)
        v27 = mins[1];
    else
        v27 = mins[0];
    v8 = v27 - mins[2];
    if (v8 < 0.0)
        v25 = mins[2];
    else
        v25 = v27;
    v7 = v25 - v26;
    if (v7 < 0.0)
        v6 = v26;
    else
        v6 = v25;
    if (radius <= (double)v6)
        return 0;
    Vec3Sub(&(*triangle)[6], (const float *)triangle, triangleEdge[1]);
    Vec3Cross(triangleEdge[0], triangleEdge[1], result);
    Vec3Normalize(result);
    Vec3Sub(origin, (const float *)triangle, point);
    v24 = Vec3Dot(point, result);
    v5 = fabs(v24);
    if (radius <= (double)v5)
        return 0;
    Vec3Cross(result, triangleEdge[0], clipPlane);
    Vec3Normalize(clipPlane);
    if (-radius >= Vec3Dot(point, clipPlane))
        return 0;
    Vec3Cross(result, triangleEdge[1], clipPlane);
    Vec3Normalize(clipPlane);
    if (-radius >= Vec3Dot(point, clipPlane))
        return 0;
    Vec3Sub(&(*triangle)[3], &(*triangle)[6], triangleEdge[2]);
    Vec3Sub(origin, &(*triangle)[3], point);
    Vec3Cross(result, triangleEdge[2], clipPlane);
    Vec3Normalize(clipPlane);
    if (-radius >= Vec3Dot(point, clipPlane))
        return 0;
    result[3] = Vec3Dot((const float *)triangle, result);
    return 1;
}

void __cdecl Phys_CollideOrientedBrushWithTriangle(
    const cbrush_t *orientedBrush,
    const float *tri0,
    const float *tri1,
    const float *tri2,
    BrushTrimeshData *data)
{
    const objInfo *info; // [esp+4h] [ebp-54h]
    unsigned int contactCount; // [esp+8h] [ebp-50h]
    dContactGeomExt *contactExt; // [esp+Ch] [ebp-4Ch]
    unsigned int firstContactIndex; // [esp+10h] [ebp-48h]
    float tempV[3]; // [esp+14h] [ebp-44h] BYREF
    unsigned int contactIndex; // [esp+20h] [ebp-38h]
    float translated[3]; // [esp+24h] [ebp-34h] BYREF
    float transformedTri[3][3]; // [esp+30h] [ebp-28h] BYREF
    Results *results; // [esp+54h] [ebp-4h]

    info = data->input;
    results = data->results;
    Vec3Sub(tri0, info->pos, translated);
    MatrixTransformVector(translated, info->RTransposed, transformedTri[0]);
    Vec3Sub(tri1, info->pos, translated);
    MatrixTransformVector(translated, info->RTransposed, transformedTri[1]);
    Vec3Sub(tri2, info->pos, translated);
    MatrixTransformVector(translated, info->RTransposed, transformedTri[2]);
    firstContactIndex = results->contactCount;
    Phys_CollideFixedBrushWithTriangle(orientedBrush, transformedTri, data);
    contactExt = &results->contacts[firstContactIndex];
    contactCount = results->contactCount - firstContactIndex;
    for (contactIndex = 0; contactIndex < contactCount; ++contactIndex)
    {
        MatrixTransformVector(contactExt->contact.normal, info->R, tempV);
        contactExt->contact.normal[0] = tempV[0];
        contactExt->contact.normal[1] = tempV[1];
        contactExt->contact.normal[2] = tempV[2];
        MatrixTransformVector(contactExt->contact.pos, info->R, tempV);
        Vec3Add(tempV, info->pos, contactExt->contact.pos);
        ++contactExt;
    }
}

void __cdecl Phys_CollideFixedBrushWithTriangle(const cbrush_t *brush, float (*triangle)[3], BrushTrimeshData *data)
{
    int v3; // [esp+1Ch] [ebp-38D8h]
    cplane_s *plane; // [esp+20h] [ebp-38D4h]
    float brushPlane[4]; // [esp+24h] [ebp-38D0h] BYREF
    float v6; // [esp+34h] [ebp-38C0h]
    unsigned int axialSide; // [esp+38h] [ebp-38BCh]
    float v8; // [esp+3Ch] [ebp-38B8h]
    float axialPlanes[6][4]; // [esp+40h] [ebp-38B4h] BYREF
    unsigned int vertCount; // [esp+A0h] [ebp-3854h]
    const objInfo *input; // [esp+A4h] [ebp-3850h]
    float result[4]; // [esp+A8h] [ebp-384Ch] BYREF
    float to; // [esp+B8h] [ebp-383Ch] BYREF
    float v14; // [esp+BCh] [ebp-3838h]
    float v15; // [esp+C0h] [ebp-3834h]
    float v16; // [esp+C4h] [ebp-3830h]
    unsigned int ClosestBrushFace; // [esp+C8h] [ebp-382Ch]
    float v18; // [esp+CCh] [ebp-3828h]
    float v19; // [esp+D0h] [ebp-3824h]
    float outVerts[3073]; // [esp+D4h] [ebp-3820h] BYREF
    float collisionNormal; // [esp+30D8h] [ebp-81Ch] BYREF
    float v22; // [esp+30DCh] [ebp-818h]
    float v23; // [esp+30E0h] [ebp-814h]
    unsigned int i; // [esp+30E4h] [ebp-810h]
    Poly poly2; // [esp+30E8h] [ebp-80Ch] BYREF
    Results *results; // [esp+30F0h] [ebp-804h]
    Poly outPolys[256]; // [esp+30F4h] [ebp-800h] BYREF

    if (!data)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1719, 0, "%s", "data");
    if (!brush)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1720, 0, "%s", "brush");
    input = data->input;
    results = data->results;
    if (!brush)
        MyAssertHandler("c:\\trees\\cod3\\src\\physics\\phys_coll_local.h", 175, 0, "%s", "brush");
    to = 0.0;
    v14 = 0.0;
    v15 = 0.0;
    v16 = 0.0;
    v8 = -3.4028235e38;
    ClosestBrushFace = -1;
    CM_BuildAxialPlanes(brush, (float (*)[4])axialPlanes);
    for (axialSide = 0; axialSide < 6; ++axialSide)
    {
        v6 = Phys_TestTriangleAgainstBrushPlane((const float *)&axialPlanes[axialSide], triangle);
        if (v6 >= 0.0)
        {
            v3 = 0;
            goto LABEL_26;
        }
        if (brush->edgeCount[axialSide & 1][Phys_AxialSideToJ(axialSide)] && v8 < (double)v6)
        {
            v8 = v6;
            ClosestBrushFace = axialSide;
            Vec4Copy((const float *)&axialPlanes[axialSide], &to);
        }
    }
    for (axialSide = 0; axialSide < brush->numsides; ++axialSide)
    {
        plane = brush->sides[axialSide].plane;
        brushPlane[0] = plane->normal[0];
        brushPlane[1] = plane->normal[1];
        brushPlane[2] = plane->normal[2];
        brushPlane[3] = brush->sides[axialSide].plane->dist;
        v6 = Phys_TestTriangleAgainstBrushPlane(brushPlane, triangle);
        if (v6 >= 0.0)
        {
            v3 = 0;
            goto LABEL_26;
        }
        if (brush->sides[axialSide].edgeCount && v8 < (double)v6)
        {
            v8 = v6;
            ClosestBrushFace = axialSide + 6;
            Vec4Copy(brushPlane, &to);
        }
    }
    v19 = v8;
    v3 = 1;
LABEL_26:
    if (v3)
    {
        vertCount = Phys_BuildWindingsForBrush2(brush, outPolys, 0x100u, (float (*)[3])outVerts, 0x400u);
        if (vertCount)
        {
            if (phys_drawCollisionObj->current.enabled)
            {
                for (i = 0; i < brush->numsides + 6; ++i)
                    Phys_DrawPolyTransformed(&outPolys[i], colorLtGreen, input->pos, input->R);
            }
            Phys_GetPlaneForTriangle(triangle, result);
            v18 = Phys_TestVertsAgainstPlane((const float (*)[3])outVerts, vertCount, result);
            if (v18 < 0.0)
            {
                poly2.ptCount = 3;
                poly2.pts = triangle;
                if (v18 >= (double)v19 || ClosestBrushFace == -1 || !outPolys[ClosestBrushFace].ptCount)
                {
                    collisionNormal = -result[0];
                    v22 = -result[1];
                    v23 = -result[2];
                    ClosestBrushFace = GetClosestBrushFace(result, brush, outPolys, &to);
                    if ((ClosestBrushFace & 0x80000000) == 0)
                    {
                        if (ClosestBrushFace >= brush->numsides + 6)
                            MyAssertHandler(
                                ".\\physics\\phys_coll_boxbrush.cpp",
                                1763,
                                0,
                                "brushSideIndex doesn't index static_cast< int >( brush->numsides ) + 6\n\t%i not in [0, %i)",
                                ClosestBrushFace,
                                brush->numsides + 6);
                        Phys_ProjectFaceOntoFaceAndClip(
                            result,
                            &poly2,
                            &outPolys[ClosestBrushFace],
                            data->surfaceFlags,
                            results,
                            &collisionNormal);
                    }
                }
                else
                {
                    if (ClosestBrushFace >= brush->numsides + 6)
                        MyAssertHandler(
                            ".\\physics\\phys_coll_boxbrush.cpp",
                            1753,
                            0,
                            "brushSideIndex doesn't index static_cast< int >( brush->numsides ) + 6\n\t%i not in [0, %i)",
                            ClosestBrushFace,
                            brush->numsides + 6);
                    collisionNormal = to;
                    v22 = v14;
                    v23 = v15;
                    Phys_ProjectFaceOntoFaceAndClip(
                        &to,
                        &outPolys[ClosestBrushFace],
                        &poly2,
                        data->surfaceFlags,
                        results,
                        &collisionNormal);
                }
            }
        }
    }
}

void __cdecl Phys_GetPlaneForTriangle(const float (*triangle)[3], float *result)
{
    float triangleEdge[2][3]; // [esp+Ch] [ebp-18h] BYREF

    Vec3Sub((const float *)triangle, &(*triangle)[3], triangleEdge[0]);
    Vec3Sub(&(*triangle)[6], (const float *)triangle, triangleEdge[1]);
    Vec3Cross(triangleEdge[0], triangleEdge[1], result);
    Vec3Normalize(result);
    result[3] = Vec3Dot((const float *)triangle, result);
}

unsigned int __cdecl Phys_AxialSideToJ(unsigned int axialSide)
{
    if (axialSide >= 6)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\physics\\phys_coll_local.h",
            161,
            0,
            "axialSide doesn't index 6\n\t%i not in [0, %i)",
            axialSide,
            6);
    return axialSide >> 1;
}

void __cdecl Phys_DrawPolyTransformed(const Poly *poly, const float *color, const float *pos, const float (*R)[3])
{
    float pt1[3]; // [esp+0h] [ebp-20h] BYREF
    unsigned int edgeIndex; // [esp+Ch] [ebp-14h]
    float pt2[3]; // [esp+10h] [ebp-10h] BYREF
    unsigned int lastEdgeIndex; // [esp+1Ch] [ebp-4h]

    lastEdgeIndex = poly->ptCount - 1;
    for (edgeIndex = 0; edgeIndex < poly->ptCount; ++edgeIndex)
    {
        MatrixTransformVector(poly->pts[lastEdgeIndex], *(const mat3x3*)R, pt1);
        Vec3Add(pt1, pos, pt1);
        MatrixTransformVector(poly->pts[edgeIndex], *(const mat3x3 *)R, pt2);
        Vec3Add(pt2, pos, pt2);
        CG_DebugLine(pt1, pt2, color, 0, 8);
        lastEdgeIndex = edgeIndex;
    }
}

double __cdecl Phys_TestTriangleAgainstBrushPlane(const float *brushPlane, const float (*triangle)[3])
{
    float triangleNormal[3]; // [esp+0h] [ebp-24h] BYREF
    float triangleEdge[2][3]; // [esp+Ch] [ebp-18h] BYREF

    Vec3Sub(&(*triangle)[3], (const float *)triangle, triangleEdge[0]);
    if (Vec3LengthSq(triangleEdge[0]) <= 0.009999999776482582)
        return Phys_TestVertsAgainstPlane(triangle, 3u, brushPlane);
    Vec3Sub(&(*triangle)[6], (const float *)triangle, triangleEdge[1]);
    if (Vec3LengthSq(triangleEdge[1]) <= 0.009999999776482582)
        return Phys_TestVertsAgainstPlane(triangle, 3u, brushPlane);
    Vec3Cross(triangleEdge[1], triangleEdge[0], triangleNormal);
    if (Vec3LengthSq(triangleNormal) <= 0.009999999776482582 || Vec3Dot(brushPlane, triangleNormal) <= 0.0)
        return Phys_TestVertsAgainstPlane(triangle, 3u, brushPlane);
    else
        return -3.4028235e38;
}

void __cdecl Phys_CollideOrientedBrushModelWithTriangleList(
    const unsigned __int16 *indices,
    const float (*verts)[3],
    int triCount,
    const objInfo *info,
    int surfaceFlags,
    Results *results)
{
    BrushTrimeshData data; // [esp+0h] [ebp-30h] BYREF
    float mins[3]; // [esp+18h] [ebp-18h] BYREF
    float maxs[3]; // [esp+24h] [ebp-Ch] BYREF

    if (results->contactCount < results->maxContacts)
    {
        mins[0] = -3.4028235e38;
        mins[1] = -3.4028235e38;
        mins[2] = -3.4028235e38;
        maxs[0] = 3.4028235e38;
        maxs[1] = 3.4028235e38;
        maxs[2] = 3.4028235e38;
        data.input = info;
        data.results = results;
        data.indices = indices;
        data.verts = verts;
        data.triCount = triCount;
        data.surfaceFlags = surfaceFlags;
        CM_ForEachBrushInLeafBrushNode_r(
            &cm.leafbrushNodes[info->u.brushModel->leaf.leafBrushNode],
            mins,
            maxs,
            0,
            0,
            Phys_CollideOrientedBrushWithTriangleList_Wrapper,
            &data);
    }
}

void __cdecl Phys_CollideOrientedBrushWithTriangleList_Wrapper(const cbrush_t *orientedBrush, void *userData)
{
    if (!userData)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 1855, 0, "%s", "userData");
    Phys_CollideOrientedBrushWithTriangleList(
        orientedBrush,
        *(const unsigned __int16 **)userData,
        *((const float (**)[3])userData + 1),
        *((unsigned int *)userData + 2),
        *((const objInfo **)userData + 3),
        *((unsigned int *)userData + 4),
        *((Results **)userData + 5));
}

void __cdecl Phys_CollideBoxWithTriangleList(
    const unsigned __int16 *indices,
    const float (*verts)[3],
    unsigned int triCount,
    const objInfo *info,
    int surfaceFlags,
    Results *results)
{
    float pos[4]; // [esp+Ch] [ebp-48h] BYREF
    int firstContact; // [esp+1Ch] [ebp-38h]
    float R[12]; // [esp+20h] [ebp-34h] BYREF
    int i; // [esp+50h] [ebp-4h]

    Phys_AxisToOdeMatrix3(info->R, R);
    pos[0] = info->pos[0];
    pos[1] = info->pos[1];
    pos[2] = info->pos[2];
    pos[3] = 0.0;
    firstContact = results->contactCount;
    results->contactCount += dCollideBoxTriangleList(
        indices,
        verts,
        triCount,
        R,
        pos,
        info->u.sideExtents,
        info->bodyCenter,
        results->maxContacts - results->contactCount,
        &results->contacts[results->contactCount].contact,
        results->stride);
    for (i = firstContact; i < results->contactCount; ++i)
        results->contacts[i].surfFlags = surfaceFlags;
}

void __cdecl Phys_AxisToOdeMatrix3(const float (*inAxis)[3], float *outMatrix)
{
    int r; // [esp+4h] [ebp-8h]
    int c; // [esp+8h] [ebp-4h]

    for (r = 0; r < 3; ++r)
    {
        for (c = 0; c < 3; ++c)
            outMatrix[4 * r + c] = (*inAxis)[3 * c + r];
        outMatrix[4 * r + 3] = 0.0;
    }
}

int __cdecl CircularRemoveRange(float (*xyz)[3], int pointCount, int begin, int end)
{
    if (!xyz)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 108, 0, "%s", "xyz");
    if (begin >= pointCount || begin < 0)
        MyAssertHandler(
            ".\\physics\\phys_coll_boxbrush.cpp",
            109,
            0,
            "%s\n\t(begin) = %i",
            "(begin < pointCount && begin >= 0)",
            begin);
    if (end >= pointCount || end < 0)
        MyAssertHandler(
            ".\\physics\\phys_coll_boxbrush.cpp",
            110,
            0,
            "%s\n\t(end) = %i",
            "(end < pointCount && end >= 0)",
            end);
    if ((begin + 1) % pointCount == end)
        return pointCount;
    if (begin >= end)
    {
        if (xyz != (float (*)[3]) & (*xyz)[3 * end])
            memmove((unsigned __int8 *)xyz, (unsigned __int8 *)&(*xyz)[3 * end], 12 * (begin - end + 1));
        return begin - end + 1;
    }
    else
    {
        memmove((unsigned __int8 *)&(*xyz)[3 * begin + 3], (unsigned __int8 *)&(*xyz)[3 * end], 12 * (pointCount - end));
        return pointCount - end + begin + 1;
    }
}

void __cdecl InsertPoint(float (*xyz)[3], int pointCount, int maxPoints, int insertAfter)
{
    if (!xyz)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 130, 0, "%s", "xyz");
    if (insertAfter >= pointCount)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 131, 0, "%s", "insertAfter < pointCount");
    if (pointCount >= maxPoints)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 132, 0, "%s", "pointCount < maxPoints");
    if (insertAfter + 1 != pointCount)
        memmove(
            (unsigned __int8 *)&(*xyz)[3 * insertAfter + 6],
            (unsigned __int8 *)&(*xyz)[3 * insertAfter + 3],
            12 * (pointCount - insertAfter - 1));
}

int __cdecl Phys_ClipPolyAgainstPlane(
    float (*poly)[3],
    unsigned int polyCount,
    unsigned int maxCount,
    float *choppingPlane)
{
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    double v8; // st7
    double v9; // st7
    double v10; // st7
    double v11; // st7
    float *v12; // [esp+10h] [ebp-70h]
    float *v13; // [esp+14h] [ebp-6Ch]
    signed int exitPair; // [esp+44h] [ebp-3Ch]
    int exitPair_4; // [esp+48h] [ebp-38h]
    bool anyInside; // [esp+4Fh] [ebp-31h]
    float frac; // [esp+50h] [ebp-30h]
    float fraca; // [esp+50h] [ebp-30h]
    float fracb; // [esp+50h] [ebp-30h]
    float fracc; // [esp+50h] [ebp-30h]
    unsigned int enterPair; // [esp+54h] [ebp-2Ch]
    unsigned int enterPaira; // [esp+54h] [ebp-2Ch]
    int enterPair_4; // [esp+58h] [ebp-28h]
    unsigned int enterPair_4a; // [esp+58h] [ebp-28h]
    unsigned int ptIndex; // [esp+5Ch] [ebp-24h]
    unsigned int ptIndexa; // [esp+5Ch] [ebp-24h]
    bool isPrevPointInside; // [esp+63h] [ebp-1Dh]
    float v2[3]; // [esp+64h] [ebp-1Ch] BYREF
    bool isLastPointInside; // [esp+73h] [ebp-Dh]
    float denom; // [esp+74h] [ebp-Ch]
    unsigned int prevPtIndex; // [esp+78h] [ebp-8h]
    bool isCurrPointInside; // [esp+7Fh] [ebp-1h]
    unsigned int polyCounta; // [esp+8Ch] [ebp+Ch]

    if (polyCount <= 2)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 249, 0, "%s", "polyCount > 2");
    if (polyCount > maxCount)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 250, 0, "%s", "polyCount <= maxCount");
    if ((COERCE_UNSIGNED_INT(*choppingPlane) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(choppingPlane[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(choppingPlane[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\physics\\phys_coll_boxbrush.cpp",
            251,
            0,
            "%s",
            "!IS_NAN((choppingPlane)[0]) && !IS_NAN((choppingPlane)[1]) && !IS_NAN((choppingPlane)[2])");
    }
    if ((COERCE_UNSIGNED_INT(choppingPlane[3]) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 252, 0, "%s", "!IS_NAN(choppingPlane[3])");
    exitPair = -1;
    exitPair_4 = -1;
    enterPair = -1;
    enterPair_4 = -1;
    anyInside = 0;
    prevPtIndex = polyCount - 1;
    if ((COERCE_UNSIGNED_INT((*poly)[3 * polyCount - 3]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((*poly)[3 * prevPtIndex + 1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((*poly)[3 * prevPtIndex + 2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\physics\\phys_coll_boxbrush.cpp",
            262,
            0,
            "%s",
            "!IS_NAN((poly[prevPtIndex])[0]) && !IS_NAN((poly[prevPtIndex])[1]) && !IS_NAN((poly[prevPtIndex])[2])");
    }
    isPrevPointInside = choppingPlane[3] >= Vec3Dot(&(*poly)[3 * prevPtIndex], choppingPlane);
    isLastPointInside = isPrevPointInside;
    for (ptIndex = 0; ptIndex < polyCount; ++ptIndex)
    {
        if ((COERCE_UNSIGNED_INT((*poly)[3 * ptIndex]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT((*poly)[3 * ptIndex + 1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT((*poly)[3 * ptIndex + 2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\physics\\phys_coll_boxbrush.cpp",
                268,
                0,
                "%s",
                "!IS_NAN((poly[ptIndex])[0]) && !IS_NAN((poly[ptIndex])[1]) && !IS_NAN((poly[ptIndex])[2])");
        }
        if (ptIndex == polyCount - 1)
            isCurrPointInside = isLastPointInside;
        else
            isCurrPointInside = choppingPlane[3] >= Vec3Dot(&(*poly)[3 * ptIndex], choppingPlane);
        if (isPrevPointInside)
        {
            anyInside = 1;
            if (!isCurrPointInside)
            {
                exitPair = prevPtIndex;
                exitPair_4 = ptIndex;
            }
        }
        else if (isCurrPointInside)
        {
            enterPair = prevPtIndex;
            enterPair_4 = ptIndex;
        }
        isPrevPointInside = isCurrPointInside;
        prevPtIndex = ptIndex;
    }
    if (!anyInside)
        return 0;
    if (exitPair == -1)
    {
        if (enterPair != -1)
            MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 302, 0, "%s", "enterPair[0] == -1");
        return polyCount;
    }
    else
    {
        if (enterPair == -1)
        {
            Com_PrintError(20, "Physics poly-clipping bug!!\n");
            v5 = va("    polyCount: %i. poly points:\n", polyCount);
            Com_PrintError(20, v5);
            for (ptIndexa = 0; ptIndexa < polyCount; ++ptIndexa)
            {
                v6 = va(
                    " %i) 0x%x, 0x%x, 0x%x\n",
                    ptIndexa,
                    (*poly)[3 * ptIndexa],
                    (*poly)[3 * ptIndexa + 1],
                    (*poly)[3 * ptIndexa + 2]);
                Com_PrintError(20, v6);
            }
            v7 = va(
                " choppingPlane: (0x%x, 0x%x, 0x%x), 0x%x\n",
                *(_DWORD *)choppingPlane,
                *((_DWORD *)choppingPlane + 1),
                *((_DWORD *)choppingPlane + 2),
                *((_DWORD *)choppingPlane + 3));
            Com_PrintError(20, v7);
            if (!alwaysfails)
                MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 319, 0, "Physics poly-clipping assert");
        }
        if (exitPair_4 == enterPair)
        {
            if (polyCount >= maxCount)
                MyAssertHandler(".\\physics\\phys_coll_boxbrush.cpp", 325, 0, "%s", "polyCount < maxCount");
            InsertPoint(poly, polyCount, maxCount, exitPair_4);
            polyCounta = polyCount + 1;
            enterPaira = (enterPair + 1) % polyCounta;
            enterPair_4a = (enterPaira + 1) % polyCounta;
            if (exitPair > exitPair_4)
                ++exitPair;
            Vec3Sub(&(*poly)[3 * exitPair_4], &(*poly)[3 * enterPair_4a], v2);
            denom = Vec3Dot(v2, choppingPlane);
            if (denom == 0.0)
            {
                v12 = &(*poly)[3 * enterPaira];
                v13 = &(*poly)[3 * exitPair_4];
                *v12 = *v13;
                v12[1] = v13[1];
                v12[2] = v13[2];
            }
            else
            {
                v8 = Vec3Dot(&(*poly)[3 * enterPair_4a], choppingPlane);
                frac = (choppingPlane[3] - v8) / denom;
                Vec3Lerp(&(*poly)[3 * enterPair_4a], &(*poly)[3 * exitPair_4], frac, &(*poly)[3 * enterPaira]);
            }
            Vec3Sub(&(*poly)[3 * exitPair_4], &(*poly)[3 * exitPair], v2);
            denom = Vec3Dot(v2, choppingPlane);
            if (denom != 0.0)
            {
                v9 = Vec3Dot(&(*poly)[3 * exitPair], choppingPlane);
                fraca = (choppingPlane[3] - v9) / denom;
                Vec3Lerp(&(*poly)[3 * exitPair], &(*poly)[3 * exitPair_4], fraca, &(*poly)[3 * exitPair_4]);
            }
        }
        else
        {
            Vec3Sub(&(*poly)[3 * exitPair_4], &(*poly)[3 * exitPair], v2);
            denom = Vec3Dot(v2, choppingPlane);
            if (denom != 0.0)
            {
                v10 = Vec3Dot(&(*poly)[3 * exitPair], choppingPlane);
                fracb = (choppingPlane[3] - v10) / denom;
                Vec3Lerp(&(*poly)[3 * exitPair], &(*poly)[3 * exitPair_4], fracb, &(*poly)[3 * exitPair_4]);
            }
            Vec3Sub(&(*poly)[3 * enterPair], &(*poly)[3 * enterPair_4], v2);
            denom = Vec3Dot(v2, choppingPlane);
            if (denom != 0.0)
            {
                v11 = Vec3Dot(&(*poly)[3 * enterPair_4], choppingPlane);
                fracc = (choppingPlane[3] - v11) / denom;
                Vec3Lerp(&(*poly)[3 * enterPair_4], &(*poly)[3 * enterPair], fracc, &(*poly)[3 * enterPair]);
            }
            return CircularRemoveRange(poly, polyCount, exitPair_4, enterPair);
        }
        return polyCounta;
    }
}