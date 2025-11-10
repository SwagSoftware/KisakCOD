#include "qcommon.h"
#include "mem_track.h"
#include <xanim/xanim.h>

#include <Windows.h>


unsigned __int8 windingPool[12292];

void __cdecl TRACK_cm_showcollision()
{
    track_static_alloc_internal(windingPool, 12292, "windingPool", 0);
}

void __cdecl CM_GetPlaneVec4Form(
    const cbrushside_t *sides,
    const float (*axialPlanes)[4],
    int index,
    float *expandedPlane)
{
    cplane_s *plane; // [esp+0h] [ebp-8h]
    float *v5; // [esp+4h] [ebp-4h]

    if (index >= 6)
    {
        iassert( sides );
        plane = sides[index - 6].plane;
        *expandedPlane = plane->normal[0];
        expandedPlane[1] = plane->normal[1];
        expandedPlane[2] = plane->normal[2];
        expandedPlane[3] = sides[index - 6].plane->dist;
    }
    else
    {
        iassert( axialPlanes );
        v5 = (float *)&(*axialPlanes)[4 * index];
        *expandedPlane = *v5;
        expandedPlane[1] = v5[1];
        expandedPlane[2] = v5[2];
        expandedPlane[3] = v5[3];
    }
}

void __cdecl CM_ShowSingleBrushCollision(
    const cbrush_t *brush,
    const float *color,
    void(__cdecl *drawCollisionPoly)(int, float (*)[3], const float *))
{
    int ptCount; // [esp+30h] [ebp-506Ch]
    ShowCollisionBrushPt brushPts; // [esp+34h] [ebp-5068h] BYREF
    int sideIndex; // [esp+5038h] [ebp-64h]
    float axialPlanes[6][4]; // [esp+503Ch] [ebp-60h] BYREF

    iassert( brush );
    iassert( color );
    CM_BuildAxialPlanes(brush, &axialPlanes);
    ptCount = CM_ForEachBrushPlaneIntersection(brush, (float(*)[4])&axialPlanes, &brushPts);
    if (ptCount >= 4)
    {
        for (sideIndex = 0; (unsigned int)sideIndex < 6; ++sideIndex)
        {
            if (CM_BuildBrushWindingForSide(
                (winding_t *)windingPool,
                (float *)&axialPlanes[sideIndex],
                sideIndex,
                &brushPts,
                ptCount))
            {
                drawCollisionPoly(*(unsigned int *)windingPool, (float (*)[3]) & windingPool[4], color);
            }
        }
        for (sideIndex = 6; sideIndex < brush->numsides + 6; ++sideIndex)
        {
            if (CM_BuildBrushWindingForSide(
                (winding_t *)windingPool,
                brush->sides[sideIndex - 6].plane->normal,
                sideIndex,
                &brushPts,
                ptCount))
            {
                drawCollisionPoly(*(unsigned int *)windingPool, (float (*)[3]) & windingPool[4], color);
            }
        }
    }
}

void __cdecl CM_BuildAxialPlanes(const cbrush_t *brush, float (*axialPlanes)[6][4])
{
    float v2; // [esp+4h] [ebp-28h]
    float v3; // [esp+Ch] [ebp-20h]
    float v4; // [esp+14h] [ebp-18h]
    float v5; // [esp+1Ch] [ebp-10h]
    float v6; // [esp+24h] [ebp-8h]
    float v7; // [esp+28h] [ebp-4h]

    v7 = -brush->mins[0];
    (*axialPlanes)[0][0] = -1.0f;
    (*axialPlanes)[0][1] = 0.0f;
    (*axialPlanes)[0][2] = 0.0f;
    (*axialPlanes)[0][3] = v7;
    v6 = brush->maxs[0];
    (*axialPlanes)[1][0] = 1.0f;
    (*axialPlanes)[1][1] = 0.0f;
    (*axialPlanes)[1][2] = 0.0f;
    (*axialPlanes)[1][3] = v6;
    v5 = -brush->mins[1];
    (*axialPlanes)[2][0] = 0.0f;
    (*axialPlanes)[2][1] = -1.0f;
    (*axialPlanes)[2][2] = 0.0f;
    (*axialPlanes)[2][3] = v5;
    v4 = brush->maxs[1];
    (*axialPlanes)[3][0] = 0.0f;
    (*axialPlanes)[3][1] = 1.0f;
    (*axialPlanes)[3][2] = 0.0f;
    (*axialPlanes)[3][3] = v4;
    v3 = -brush->mins[2];
    (*axialPlanes)[4][0] = 0.0f;
    (*axialPlanes)[4][1] = 0.0f;
    (*axialPlanes)[4][2] = -1.0f;
    (*axialPlanes)[4][3] = v3;
    v2 = brush->maxs[2];
    (*axialPlanes)[5][0] = 0.0f;
    (*axialPlanes)[5][1] = 0.0f;
    (*axialPlanes)[5][2] = 1.0f;
    (*axialPlanes)[5][3] = v2;
}

int __cdecl CM_ForEachBrushPlaneIntersection(
    const cbrush_t *brush,
    const float (*axialPlanes)[4],
    ShowCollisionBrushPt *brushPts)
{
    const float *plane[3]; // [esp+Ch] [ebp-60h] BYREF
    int sideCount; // [esp+18h] [ebp-54h]
    int ptCount; // [esp+1Ch] [ebp-50h]
    float xyz[3]; // [esp+20h] [ebp-4Ch] BYREF
    float expandedPlane[3][4]; // [esp+2Ch] [ebp-40h] BYREF
    __int16 sideIndex[4]; // [esp+5Ch] [ebp-10h] BYREF
    const cbrushside_t *sides; // [esp+68h] [ebp-4h]

    iassert( brush );
    iassert( brushPts );
    ptCount = 0;
    plane[0] = expandedPlane[0];
    plane[1] = expandedPlane[1];
    plane[2] = expandedPlane[2];
    sideCount = brush->numsides + 6;
    sides = brush->sides;
    for (sideIndex[0] = 0; sideIndex[0] < sideCount - 2; ++sideIndex[0])
    {
        CM_GetPlaneVec4Form(sides, axialPlanes, sideIndex[0], expandedPlane[0]);
        for (sideIndex[1] = sideIndex[0] + 1; sideIndex[1] < sideCount - 1; ++sideIndex[1])
        {
            if (sideIndex[0] < 6 || sideIndex[1] < 6 || sides[sideIndex[0] - 6].plane != sides[sideIndex[1] - 6].plane)
            {
                CM_GetPlaneVec4Form(sides, axialPlanes, sideIndex[1], expandedPlane[1]);
                for (sideIndex[2] = sideIndex[1] + 1; sideIndex[2] < sideCount; ++sideIndex[2])
                {
                    if ((sideIndex[0] < 6 || sideIndex[2] < 6 || sides[sideIndex[0] - 6].plane != sides[sideIndex[2] - 6].plane)
                        && (sideIndex[1] < 6 || sideIndex[2] < 6 || sides[sideIndex[1] - 6].plane != sides[sideIndex[2] - 6].plane))
                    {
                        CM_GetPlaneVec4Form(sides, axialPlanes, sideIndex[2], expandedPlane[2]);
                        if (IntersectPlanes(plane, xyz))
                        {
                            SnapPointToIntersectingPlanes(plane, xyz, 0.25f, 0.01f);
                            ptCount = CM_AddSimpleBrushPoint(brush, axialPlanes, sideIndex, xyz, ptCount, brushPts);
                        }
                    }
                }
            }
        }
    }
    return ptCount;
}

int __cdecl CM_AddSimpleBrushPoint(
    const cbrush_t *brush,
    const float (*axialPlanes)[4],
    const __int16 *sideIndices,
    const float *xyz,
    int ptCount,
    ShowCollisionBrushPt *brushPts)
{
    ShowCollisionBrushPt *v7; // [esp+0h] [ebp-10h]
    cplane_s *plane; // [esp+4h] [ebp-Ch]
    float dist; // [esp+8h] [ebp-8h]
    float dista; // [esp+8h] [ebp-8h]
    unsigned int sideIndex; // [esp+Ch] [ebp-4h]
    unsigned int sideIndexa; // [esp+Ch] [ebp-4h]

    iassert( brush );
    iassert( brushPts );
    for (sideIndex = 0; sideIndex < 6; ++sideIndex)
    {
        dist = Vec3Dot(&(*axialPlanes)[4 * sideIndex], xyz) - (float)(*axialPlanes)[4 * sideIndex + 3];
        if (dist > 0.1000000014901161)
            return ptCount;
    }
    for (sideIndexa = 0; sideIndexa < brush->numsides; ++sideIndexa)
    {
        plane = brush->sides[sideIndexa].plane;
        if (plane != brush->sides[*sideIndices - 6].plane
            && plane != brush->sides[sideIndices[1] - 6].plane
            && plane != brush->sides[sideIndices[2] - 6].plane)
        {
            dista = Vec3Dot(plane->normal, xyz) - plane->dist;
            if (dista > 0.1000000014901161)
                return ptCount;
        }
    }
    if (ptCount >= 1024)
        Com_Error(ERR_DROP, "More than %i points from plane intersections on %i-sided brush\n", ptCount, brush->numsides);
    v7 = &brushPts[ptCount];
    v7->xyz[0] = *xyz;
    v7->xyz[1] = xyz[1];
    v7->xyz[2] = xyz[2];
    v7->sideIndex[0] = *sideIndices;
    v7->sideIndex[1] = sideIndices[1];
    v7->sideIndex[2] = sideIndices[2];
    return ptCount + 1;
}

char __cdecl CM_BuildBrushWindingForSide(
    winding_t *winding,
    float *planeNormal,
    int sideIndex,
    const ShowCollisionBrushPt *pts,
    int ptCount)
{
    int j; // [esp+Ch] [ebp-302Ch] BYREF
    int i2; // [esp+10h] [ebp-3028h] BYREF
    int XyzList; // [esp+14h] [ebp-3024h]
    float xyz[3072]; // [esp+18h] [ebp-3020h] BYREF
    int i0; // [esp+3018h] [ebp-20h] BYREF
    int i1; // [esp+301Ch] [ebp-1Ch] BYREF
    int k; // [esp+3020h] [ebp-18h]
    int i; // [esp+3024h] [ebp-14h] BYREF
    float plane[4]; // [esp+3028h] [ebp-10h] BYREF

    iassert( winding );
    iassert( planeNormal );
    iassert( pts );
    XyzList = CM_GetXyzList(sideIndex, pts, ptCount, (float (*)[3])xyz, 1024);
    if (XyzList < 3)
        return 0;
    CM_PickProjectionAxes(planeNormal, &i, &j);
    winding->p[0][0] = xyz[0];
    winding->p[0][1] = xyz[1];
    winding->p[0][2] = xyz[2];
    winding->p[1][0] = xyz[3];
    winding->p[1][1] = xyz[4];
    winding->p[1][2] = xyz[5];
    winding->numpoints = 2;
    for (k = 2; k < XyzList; ++k)
        CM_AddExteriorPointToWindingProjected(winding, &xyz[3 * k], i, j);
    if (CM_RepresentativeTriangleFromWinding(winding, planeNormal, &i0, &i1, &i2) < EQUAL_EPSILON)
        return 0;
    PlaneFromPoints(plane, winding->p[i0], winding->p[i1], winding->p[i2]);
    if (Vec3Dot(plane, planeNormal) > 0.0)
        CM_ReverseWinding(winding);
    return 1;
}

int __cdecl CM_GetXyzList(int sideIndex, const ShowCollisionBrushPt *pts, int ptCount, float (*xyz)[3], int xyzLimit)
{
    float *v6; // [esp+0h] [ebp-10h]
    const ShowCollisionBrushPt *v7; // [esp+4h] [ebp-Ch]
    int xyzCount; // [esp+8h] [ebp-8h]
    int ptIndex; // [esp+Ch] [ebp-4h]

    iassert( pts );
    iassert( xyz );
    xyzCount = 0;
    for (ptIndex = 0; ptIndex < ptCount; ++ptIndex)
    {
        if ((sideIndex == pts[ptIndex].sideIndex[0]
            || sideIndex == pts[ptIndex].sideIndex[1]
            || sideIndex == pts[ptIndex].sideIndex[2])
            && !CM_PointInList(pts[ptIndex].xyz, xyz, xyzCount))
        {
            if (xyzCount == xyzLimit)
                Com_Error(ERR_DROP, "Winding point limit (%i) exceeded on brush face", xyzLimit);
            v6 = &(*xyz)[3 * xyzCount];
            v7 = &pts[ptIndex];
            *v6 = v7->xyz[0];
            v6[1] = v7->xyz[1];
            v6[2] = v7->xyz[2];
            ++xyzCount;
        }
    }
    return xyzCount;
}

int __cdecl CM_PointInList(const float *point, const float (*xyzList)[3], int xyzCount)
{
    int xyzIndex; // [esp+8h] [ebp-4h]

    for (xyzIndex = 0; xyzIndex < xyzCount; ++xyzIndex)
    {
        if (VecNCompareCustomEpsilon(&(*xyzList)[3 * xyzIndex], point, 0.1f, 3))
            return 1;
    }
    return 0;
}

void __cdecl CM_PickProjectionAxes(const float *normal, int *i, int *j)
{
    float v3; // [esp+0h] [ebp-2Ch]
    float v4; // [esp+4h] [ebp-28h]
    float v5; // [esp+8h] [ebp-24h]
    float v6; // [esp+Ch] [ebp-20h]
    int k; // [esp+28h] [ebp-4h]

    v6 = I_fabs(*normal);
    v5 = I_fabs(normal[1]);
    k = v5 > (double)v6;
    v4 = I_fabs(normal[k]);
    v3 = I_fabs(normal[2]);

    if (v3 > (double)v4)
    {
        //LOBYTE(k) = 2;
        k = 2;
    }

    *i = (k & 1) == 0;
    *j = ~(_BYTE)k & 2;
}

void __cdecl CM_AddExteriorPointToWindingProjected(winding_t *w, float *pt, int i, int j)
{
    float *v4; // [esp+0h] [ebp-18h]
    int indexPrev; // [esp+4h] [ebp-14h]
    int bestIndex; // [esp+8h] [ebp-10h]
    int index; // [esp+Ch] [ebp-Ch]
    float signedArea; // [esp+10h] [ebp-8h]
    float bestSignedArea; // [esp+14h] [ebp-4h]

    bestIndex = -1;
    bestSignedArea = FLT_MAX;
    indexPrev = w->numpoints - 1;
    for (index = 0; index < w->numpoints; ++index)
    {
        signedArea = CM_SignedAreaForPointsProjected(w->p[indexPrev], pt, w->p[index], i, j);
        if (signedArea < (double)bestSignedArea)
        {
            bestSignedArea = signedArea;
            bestIndex = index;
        }
        indexPrev = index;
    }
    iassert( bestIndex >= 0 );
    if (bestSignedArea < -EQUAL_EPSILON)
    {
        memmove((unsigned __int8 *)w->p[bestIndex + 1], (unsigned __int8 *)w->p[bestIndex], 12 * (w->numpoints - bestIndex));
        v4 = w->p[bestIndex];
        *v4 = *pt;
        v4[1] = pt[1];
        v4[2] = pt[2];
        ++w->numpoints;
    }
    else if (bestSignedArea <= EQUAL_EPSILON)
    {
        CM_AddColinearExteriorPointToWindingProjected(w, pt, i, j, (bestIndex + w->numpoints - 1) % w->numpoints, bestIndex);
    }
}

double __cdecl CM_SignedAreaForPointsProjected(const float *pt0, const float *pt1, const float *pt2, int i, int j)
{
    return (float)((pt2[j] - pt1[j]) * pt0[i] + (pt0[j] - pt2[j]) * pt1[i] + (pt1[j] - pt0[j]) * pt2[i]);
}

void __cdecl CM_AddColinearExteriorPointToWindingProjected(
    winding_t *w,
    float *pt,
    int i,
    int j,
    int index0,
    int index1)
{
    float v6; // [esp+0h] [ebp-2Ch]
    float v7; // [esp+4h] [ebp-28h]
    float *v8; // [esp+8h] [ebp-24h]
    float *v9; // [esp+Ch] [ebp-20h]
    float *v10; // [esp+10h] [ebp-1Ch]
    float *v11; // [esp+14h] [ebp-18h]
    float delta; // [esp+1Ch] [ebp-10h]
    float v13; // [esp+20h] [ebp-Ch]
    float dj; // [esp+24h] [ebp-8h]
    int axis; // [esp+28h] [ebp-4h]

    if (w->p[index1][i] == w->p[index0][i] && w->p[index1][j] == w->p[index0][j])
        MyAssertHandler(
            ".\\qcommon\\cm_showcollision.cpp",
            232,
            0,
            "%s",
            "w->p[index0][i] != w->p[index1][i] || w->p[index0][j] != w->p[index1][j]");
    v13 = w->p[index1][i] - w->p[index0][i];
    dj = w->p[index1][j] - w->p[index0][j];
    v7 = I_fabs(v13);
    v6 = I_fabs(dj);
    if (v6 > (double)v7)
    {
        axis = j;
        delta = w->p[index1][j] - w->p[index0][j];
    }
    else
    {
        axis = i;
        delta = w->p[index1][i] - w->p[index0][i];
    }
    if (delta <= 0.0)
    {
        iassert( w->p[index0][axis] > w->p[index1][axis] );
        if (w->p[index0][axis] >= (double)pt[axis])
        {
            if (w->p[index1][axis] > (double)pt[axis])
            {
                v8 = w->p[index1];
                *v8 = *pt;
                v8[1] = pt[1];
                v8[2] = pt[2];
            }
        }
        else
        {
            v9 = w->p[index0];
            *v9 = *pt;
            v9[1] = pt[1];
            v9[2] = pt[2];
        }
    }
    else
    {
        iassert( w->p[index0][axis] < w->p[index1][axis] );
        if (w->p[index0][axis] <= (double)pt[axis])
        {
            if (w->p[index1][axis] < (double)pt[axis])
            {
                v10 = w->p[index1];
                *v10 = *pt;
                v10[1] = pt[1];
                v10[2] = pt[2];
            }
        }
        else
        {
            v11 = w->p[index0];
            *v11 = *pt;
            v11[1] = pt[1];
            v11[2] = pt[2];
        }
    }
}

double __cdecl CM_RepresentativeTriangleFromWinding(const winding_t *w, const float *normal, int *i0, int *i1, int *i2)
{
    float v6; // [esp+0h] [ebp-40h]
    float v7; // [esp+4h] [ebp-3Ch]
    int j; // [esp+8h] [ebp-38h]
    float areaBest; // [esp+Ch] [ebp-34h]
    float vb[3]; // [esp+14h] [ebp-2Ch] BYREF
    int k; // [esp+20h] [ebp-20h]
    float vc[3]; // [esp+24h] [ebp-1Ch] BYREF
    float va[3]; // [esp+30h] [ebp-10h] BYREF
    int i; // [esp+3Ch] [ebp-4h]

    *i0 = 0;
    *i1 = 1;
    *i2 = 2;
    areaBest = 0.0;
    for (k = 2; k < w->numpoints; ++k)
    {
        for (j = 1; j < k; ++j)
        {
            Vec3Sub(w->p[k], w->p[j], vb);
            for (i = 0; i < j; ++i)
            {
                Vec3Sub(w->p[i], w->p[j], va);
                Vec3Cross(va, vb, vc);
                v7 = Vec3Dot(vc, normal);
                v6 = I_fabs(v7);
                if (areaBest < (double)v6)
                {
                    areaBest = v6;
                    *i0 = i;
                    *i1 = j;
                    *i2 = k;
                }
            }
        }
    }
    return areaBest;
}

void __cdecl CM_ReverseWinding(winding_t *w)
{
    float *v1; // ecx
    float *v2; // [esp+4h] [ebp-1Ch]
    float *v3; // [esp+8h] [ebp-18h]
    float pt; // [esp+10h] [ebp-10h]
    float pt_4; // [esp+14h] [ebp-Ch]
    float pt_8; // [esp+18h] [ebp-8h]
    int i; // [esp+1Ch] [ebp-4h]

    for (i = 0; i < w->numpoints / 2; ++i)
    {
        pt = w->p[i][0];
        pt_4 = w->p[i][1];
        pt_8 = w->p[i][2];
        v2 = w->p[i];
        v3 = w->p[w->numpoints - 1 - i];
        *v2 = *v3;
        v2[1] = v3[1];
        v2[2] = v3[2];
        v1 = w->p[w->numpoints - 1 - i];
        *v1 = pt;
        v1[1] = pt_4;
        v1[2] = pt_8;
    }
}

void __cdecl CM_ShowBrushCollision(
    int contentMask,
    cplane_s *frustumPlanes,
    int frustumPlaneCount,
    void(__cdecl *drawCollisionPoly)(int, float (*)[3], const float *))
{
    float colorFloat[4]; // [esp+4h] [ebp-1Ch] BYREF
    int brushIndex; // [esp+14h] [ebp-Ch]
    cbrush_t *brush; // [esp+18h] [ebp-8h]
    int colorCounter; // [esp+1Ch] [ebp-4h]

    iassert( frustumPlanes );
    colorCounter = 0;
    for (brushIndex = 0; brushIndex < cm.numBrushes; ++brushIndex)
    {
        brush = &cm.brushes[brushIndex];
        if ((contentMask & brush->contents) != 0)
        {
            CM_GetShowCollisionColor(colorFloat, colorCounter++);
            colorCounter %= 8;
            if (CM_BrushInView(brush, frustumPlanes, frustumPlaneCount))
                CM_ShowSingleBrushCollision(brush, colorFloat, drawCollisionPoly);
        }
    }
}

void __cdecl CM_GetShowCollisionColor(float *colorFloat, char colorCounter)
{
    float v2; // [esp+0h] [ebp-Ch]
    float v3; // [esp+4h] [ebp-8h]
    float v4; // [esp+8h] [ebp-4h]

    iassert( colorFloat );
    if ((colorCounter & 1) != 0)
        v4 = 1.0;
    else
        v4 = 0.0;
    *colorFloat = v4;
    if ((colorCounter & 2) != 0)
        v3 = 1.0;
    else
        v3 = 0.0;
    colorFloat[1] = v3;
    if ((colorCounter & 4) != 0)
        v2 = 1.0;
    else
        v2 = 0.0;
    colorFloat[2] = v2;
    colorFloat[3] = 0.5;
}

char __cdecl CM_BrushInView(const cbrush_t *brush, cplane_s *frustumPlanes, int frustumPlaneCount)
{
    int frustumPlaneIndex; // [esp+0h] [ebp-4h]
    int frustumPlaneIndexa; // [esp+0h] [ebp-4h]

    iassert( frustumPlanes );
    for (frustumPlaneIndex = 0; frustumPlaneIndex < frustumPlaneCount; frustumPlaneIndex++)// = frustumPlaneIndexa + 1)
    {
        if ((BoxOnPlaneSide(
            brush->mins,
            brush->maxs,
            &frustumPlanes[frustumPlaneIndex])))
            return 0;
    }
    return 1;
}

int bops_initialized;
int Ljmptab[8];
BOOL __cdecl BoxOnPlaneSide(const float *emins, const float *emaxs, const cplane_s *p)
{
    // KISAKTODO: Needs ASM jump table and assembly bits (Probably critical function lmao)
    // 
    //int signbits; // eax
    //
    //if (bops_initialized != 1)
    //{
    //    bops_initialized = 1;
    //    Ljmptab[0] = (int)&Lcase0;
    //    Ljmptab[1] = (int)&Lcase1;
    //    Ljmptab[2] = (int)&Lcase2;
    //    Ljmptab[3] = (int)&Lcase3;
    //    Ljmptab[4] = (int)&Lcase4;
    //    Ljmptab[5] = (int)&Lcase5;
    //    Ljmptab[6] = (int)&Lcase6;
    //    Ljmptab[7] = (int)&Lcase7;
    //}
    //signbits = p->signbits;
    //if ((unsigned __int8)signbits < 8u)
    //    __asm { jmp     Ljmptab[eax * 4] }
    //__debugbreak();

    float v3;
    float v4;

    // LWSS: Note that this is not generic-able. The maths are slightly changed for each case.
    // These are opposite per-line
    // v3 = MAX, MAX, MAX
    // v4 = MIN, MIN, MIN
    // ... 
    // v3 = MIN, MAX, MIN
    // v4 = MAX, MIN, MAX
    switch (p->signbits)
    {
    case 0:
        v3 = (p->normal[0] * emaxs[0]) + (emaxs[1] * p->normal[1]) + (emaxs[2] * p->normal[2]); 
        v4 = (p->normal[0] * emins[0]) + (emins[1] * p->normal[1]) + (emins[2] * p->normal[2]);
        break;
    case 1:
        v3 = (p->normal[0] * emins[0]) + (emaxs[1] * p->normal[1]) + (emaxs[2] * p->normal[2]);
        v4 = (p->normal[0] * emaxs[0]) + (emins[1] * p->normal[1]) + (emins[2] * p->normal[2]);
        break;
    case 2:
        v3 = (p->normal[0] * emaxs[0]) + (emaxs[2] * p->normal[2]) + (emins[1] * p->normal[1]);
        v4 = (p->normal[0] * emins[0]) + (emins[2] * p->normal[2]) + (emaxs[1] * p->normal[1]);
        break;
    case 3:
        v3 = (p->normal[0] * emins[0]) + (emaxs[2] * p->normal[2]) + (emins[1] * p->normal[1]);
        v4 = (p->normal[0] * emaxs[0]) + (emins[2] * p->normal[2]) + (emaxs[1] * p->normal[1]);
        break;
    case 4:
        v3 = (p->normal[0] * emaxs[0]) + (emaxs[1] * p->normal[1]) + (emins[2] * p->normal[2]);
        v4 = (p->normal[0] * emins[0]) + (emins[1] * p->normal[1]) + (emaxs[2] * p->normal[2]);
        break;
    case 5:
        v3 = (p->normal[0] * emins[0]) + (emaxs[1] * p->normal[1]) + (emins[2] * p->normal[2]);
        v4 = (p->normal[0] * emaxs[0]) + (emins[1] * p->normal[1]) + (emaxs[2] * p->normal[2]);
        break;
    case 6:
        v3 = (p->normal[0] * emaxs[0]) + (emins[1] * p->normal[1]) + (emins[2] * p->normal[2]);
        v4 = (p->normal[0] * emins[0]) + (emaxs[1] * p->normal[1]) + (emaxs[2] * p->normal[2]);
        break;
    case 7:
        v3 = (p->normal[0] * emins[0]) + (emins[1] * p->normal[1]) + (emins[2] * p->normal[2]);
        v4 = (p->normal[0] * emaxs[0]) + (emaxs[1] * p->normal[1]) + (emaxs[2] * p->normal[2]);
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\universal\\com_math.cpp", 3473, 1, "BoxOnPlaneSide: invalid signbits for plane");

        __debugbreak();
        __debugbreak();
        __debugbreak();
        __debugbreak();
        __debugbreak();
        break;
    }
    
    return (2 * (v4 < p->dist)) | (v3 > p->dist); // KISAKTODO: probably BoxDistSqrdExceeds()
    //return BoxDistSqrdExceeds(emins, emaxs, p->normal, *(float *)&pa);
}
