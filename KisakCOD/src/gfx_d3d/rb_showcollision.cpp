#include "rb_showcollision.h"
#include <qcommon/mem_track.h>
#include <universal/assertive.h>
#include "r_dvars.h"
#include "rb_shade.h"
#include "r_dpvs.h"
#include "r_utils.h"
#include "rb_stats.h"
#include "rb_state.h"
#include "rb_debug.h"

const float shadowFrustumSidePlanes[5][4] =
{
  { -1.0, 0.0, 0.0, 1.0 },
  { 1.0, 0.0, 0.0, 1.0 },
  { 0.0, -1.0, 0.0, 1.0 },
  { 0.0, 1.0, 0.0, 1.0 },
  { 0.0, 0.0, 1.0, 0.0 }
}; // idb
const float frustumSidePlanes[5][4] =
{
  { -1.0, 0.0, 0.0, 1.0 },
  { 1.0, 0.0, 0.0, 1.0 },
  { 0.0, -1.0, 0.0, 1.0 },
  { 0.0, 1.0, 0.0, 1.0 },
  { 0.0, 0.0, 1.0, 1.0 }
}; // idb

int showCollisionContentMasks[10] = { 0, 210624, 65536, 8192, 128, 512, 131072, 1024, 64, 4096 }; // idb

GfxPointVertex debugLineVerts[2725];

void __cdecl TRACK_rb_showcollision()
{
    track_static_alloc_internal(debugLineVerts, 43600, "debugLineVerts", 0);
}

void __cdecl RB_ShowCollision(const GfxViewParms *viewParms)
{
    char v1; // [esp+8h] [ebp-98h]
    char v2; // [esp+Ch] [ebp-94h]
    unsigned __int8 v3; // [esp+10h] [ebp-90h]
    int contentMask; // [esp+18h] [ebp-88h]
    cplane_s frustumPlanes[6]; // [esp+20h] [ebp-80h] BYREF

    if (!viewParms)
        MyAssertHandler(".\\rb_showcollision.cpp", 133, 0, "%s", "viewParms");
    if (r_showCollision && r_showCollision->current.integer != 0)
    {
        //contentMask = showCollisionContentMasks[*(unsigned int *)(LODWORD(r_lightTweakSunDirection.vector[3]) + 12)];
        contentMask = showCollisionContentMasks[r_showCollision->current.integer];

        BuildFrustumPlanes(viewParms, frustumPlanes);
        Vec3Scale(frustumPlanes[4].normal, -1.0, frustumPlanes[5].normal);
        frustumPlanes[5].dist = -frustumPlanes[4].dist - r_showCollisionDist->current.value;
        if (frustumPlanes[5].normal[0] == 1.0)
        {
            v3 = 0;
        }
        else
        {
            if (frustumPlanes[5].normal[1] == 1.0)
            {
                v2 = 1;
            }
            else
            {
                if (frustumPlanes[5].normal[2] == 1.0)
                    v1 = 2;
                else
                    v1 = 3;
                v2 = v1;
            }
            v3 = v2;
        }
        frustumPlanes[5].type = v3;
        SetPlaneSignbits(&frustumPlanes[5]);
        if (r_showCollisionGroups->current.integer <= 1u)
            CM_ShowBrushCollision(contentMask, frustumPlanes, 6, RB_DrawCollisionPoly);
        if (tess.indexCount)
            RB_EndTessSurface();
    }
}

void __cdecl BuildFrustumPlanes(const GfxViewParms *viewParms, cplane_s *frustumPlanes)
{
    char v2; // [esp+0h] [ebp-84h]
    char v3; // [esp+4h] [ebp-80h]
    unsigned __int8 v4; // [esp+8h] [ebp-7Ch]
    cplane_s *v5; // [esp+Ch] [ebp-78h]
    DpvsPlane *v6; // [esp+10h] [ebp-74h]
    DpvsPlane dpvsFrustumPlanes[5]; // [esp+14h] [ebp-70h] BYREF
    unsigned int planeIndex; // [esp+80h] [ebp-4h]

    if (!viewParms)
        MyAssertHandler(".\\rb_showcollision.cpp", 59, 0, "%s", "viewParms");
    if (!frustumPlanes)
        MyAssertHandler(".\\rb_showcollision.cpp", 60, 0, "%s", "frustumPlanes");
    R_FrustumClipPlanes(&viewParms->viewProjectionMatrix, frustumSidePlanes, 5, dpvsFrustumPlanes);
    for (planeIndex = 0; planeIndex < 5; ++planeIndex)
    {
        v5 = &frustumPlanes[planeIndex];
        v6 = &dpvsFrustumPlanes[planeIndex];
        v5->normal[0] = v6->coeffs[0];
        v5->normal[1] = v6->coeffs[1];
        v5->normal[2] = v6->coeffs[2];
        v5->dist = v6->coeffs[3];
        frustumPlanes[planeIndex].dist = frustumPlanes[planeIndex].dist * -1.0;
        if (frustumPlanes[planeIndex].normal[0] == 1.0)
        {
            v4 = 0;
        }
        else
        {
            if (frustumPlanes[planeIndex].normal[1] == 1.0)
            {
                v3 = 1;
            }
            else
            {
                if (frustumPlanes[planeIndex].normal[2] == 1.0)
                    v2 = 2;
                else
                    v2 = 3;
                v3 = v2;
            }
            v4 = v3;
        }
        frustumPlanes[planeIndex].type = v4;
        SetPlaneSignbits(&frustumPlanes[planeIndex]);
    }
}

void __cdecl RB_SetPolyVert(float *xyz, GfxColor color, int tessVertIndex)
{
    float *xyzw; // [esp+4h] [ebp-4h]

    xyzw = tess.verts[tessVertIndex].xyzw;
    *xyzw = *xyz;
    xyzw[1] = xyz[1];
    xyzw[2] = xyz[2];
    *&tess.indices[16 * tessVertIndex - 87194] = 1.0;
    *&tess.indices[16 * tessVertIndex - 87186] = 1073643391;
    *&tess.indices[16 * tessVertIndex - 87192] = color;
    *&tess.indices[16 * tessVertIndex - 87190] = 0.0;
    *&tess.indices[16 * tessVertIndex - 87188] = 0.0;
}

void __cdecl RB_DrawCollisionPoly(int numPoints, float (*points)[3], const float *colorFloat)
{
    int vertCount; // [esp+4h] [ebp-10h]
    int vertIndex; // [esp+8h] [ebp-Ch]
    int vertIndexa; // [esp+8h] [ebp-Ch]
    int vertIndexb; // [esp+8h] [ebp-Ch]
    int vertIndexPrev; // [esp+Ch] [ebp-8h]
    GfxColor color; // [esp+10h] [ebp-4h] BYREF

    if (numPoints < 3)
        MyAssertHandler(".\\rb_showcollision.cpp", 84, 0, "%s", "numPoints >= 3");
    if (!points)
        MyAssertHandler(".\\rb_showcollision.cpp", 85, 0, "%s", "points");
    if (r_showCollisionPolyType->current.integer == 2 || !r_showCollisionPolyType->current.integer)
    {
        R_ConvertColorToBytes(colorFloat, (unsigned int *)&color);
        RB_BeginSurface(rgp.whiteMaterial, TECHNIQUE_UNLIT);
        R_TrackPrims(&gfxCmdBufState, GFX_PRIM_STATS_DEBUG);
        RB_CheckTessOverflow(numPoints, 3 * (numPoints - 2));
        for (vertIndex = 0; vertIndex < numPoints; ++vertIndex)
            RB_SetPolyVert(&(*points)[3 * vertIndex], color, vertIndex + tess.vertexCount);
        for (vertIndexa = 2; vertIndexa < numPoints; ++vertIndexa)
        {
            tess.indices[tess.indexCount] = tess.vertexCount;
            tess.indices[tess.indexCount + 1] = vertIndexa + LOWORD(tess.vertexCount);
            tess.indices[tess.indexCount + 2] = LOWORD(tess.vertexCount) + vertIndexa - 1;
            tess.indexCount += 3;
        }
        tess.vertexCount += numPoints;
        RB_EndTessSurface();
    }
    if (r_showCollisionPolyType->current.integer <= 1u)
    {
        vertCount = 0;
        vertIndexPrev = numPoints - 1;
        for (vertIndexb = 0; vertIndexb < numPoints; ++vertIndexb)
        {
            vertCount = RB_AddDebugLine(
                &(*points)[3 * vertIndexPrev],
                &(*points)[3 * vertIndexb],
                colorFloat,
                r_showCollisionDepthTest->current.enabled,
                vertCount,
                2725,
                debugLineVerts);
            vertIndexPrev = vertIndexb;
        }
        RB_EndDebugLines(r_showCollisionDepthTest->current.enabled, vertCount / 2, debugLineVerts);
    }
}

