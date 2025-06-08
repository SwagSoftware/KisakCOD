#include "r_dpvs.h"
#include <qcommon/mem_track.h>
#include "r_model_lighting.h"
#include "r_dvars.h"
#include <DynEntity/DynEntity_client.h>
#include "r_drawsurf.h"
#include "r_primarylights.h"
#include "r_model.h"
#include <qcommon/threads.h>
#include <universal/com_convexhull.h>
#include "r_workercmds.h"
#include "r_utils.h"
#include "r_light.h"
#include <devgui/devgui.h>
#include <cgame/cg_local.h>
#include <cgame_mp/cg_local_mp.h>
#include "rb_light.h"


DpvsGlobals dpvsGlob;

thread_local DpvsView *g_dpvsView;
thread_local int g_viewIndex;
thread_local EntVisData g_dynEntVisData;
thread_local byte *g_smodelVisData;
thread_local byte *g_surfaceVisData;

void __cdecl TRACK_r_dpvs()
{
    track_static_alloc_internal(&dpvsGlob, 44664, "dpvsGlob", 18);
}

void __cdecl R_FrustumClipPlanes(
    const GfxMatrix *viewProjMtx,
    const float (*sidePlanes)[4],
    int sidePlaneCount,
    DpvsPlane *frustumPlanes)
{
    DpvsPlane *v4; // [esp+8h] [ebp-1Ch]
    int term; // [esp+14h] [ebp-10h]
    float scale; // [esp+18h] [ebp-Ch]
    float length; // [esp+1Ch] [ebp-8h]
    int planeIndex; // [esp+20h] [ebp-4h]

    for (planeIndex = 0; planeIndex < sidePlaneCount; ++planeIndex)
    {
        for (term = 0; term < 4; ++term)
            frustumPlanes[planeIndex].coeffs[term] = Vec4Dot(&(*sidePlanes)[4 * planeIndex], viewProjMtx->m[term]);
        length = Vec3Length(frustumPlanes[planeIndex].coeffs);
        if (length <= 0.0)
            MyAssertHandler(".\\r_dpvs.cpp", 300, 0, "%s", "length > 0");
        scale = 1.0 / length;
        Vec4Scale(frustumPlanes[planeIndex].coeffs, scale, frustumPlanes[planeIndex].coeffs);
        v4 = &frustumPlanes[planeIndex];
        v4->side[0] = COERCE_INT(v4->coeffs[0]) <= 0 ? 0 : 0xC;
        v4->side[1] = COERCE_INT(v4->coeffs[1]) <= 0 ? 4 : 16;
        v4->side[2] = COERCE_INT(v4->coeffs[2]) <= 0 ? 8 : 20;
    }
}

char *__cdecl R_PortalAssertMsg()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax

    v0 = va("\torg %.8gf, %.8gf, %.8gf\n", dpvsGlob.viewOrg[0], dpvsGlob.viewOrg[1], dpvsGlob.viewOrg[2]);
    v1 = va(
        "%s\tplane %.8gf, %.8gf, %.8gf, %.8gf\n",
        v0,
        dpvsGlob.viewPlane.coeffs[0],
        dpvsGlob.viewPlane.coeffs[1],
        dpvsGlob.viewPlane.coeffs[2],
        dpvsGlob.viewPlane.coeffs[3]);
    v2 = va(
        "%s\t%.8gf, %.8gf, %.8gf, %.8gf\n",
        v1,
        dpvsGlob.viewProjMtx->m[0][0],
        dpvsGlob.viewProjMtx->m[0][1],
        dpvsGlob.viewProjMtx->m[0][2],
        dpvsGlob.viewProjMtx->m[0][3]);
    v3 = va(
        "%s\t%.8gf, %.8gf, %.8gf, %.8gf\n",
        v2,
        dpvsGlob.viewProjMtx->m[1][0],
        dpvsGlob.viewProjMtx->m[1][1],
        dpvsGlob.viewProjMtx->m[1][2],
        dpvsGlob.viewProjMtx->m[1][3]);
    v4 = va(
        "%s\t%.8gf, %.8gf, %.8gf, %.8gf\n",
        v3,
        dpvsGlob.viewProjMtx->m[2][0],
        dpvsGlob.viewProjMtx->m[2][1],
        dpvsGlob.viewProjMtx->m[2][2],
        dpvsGlob.viewProjMtx->m[2][3]);
    return va(
        "%s\t%.8gf, %.8gf, %.8gf, %.8gf\n",
        v4,
        dpvsGlob.viewProjMtx->m[3][0],
        dpvsGlob.viewProjMtx->m[3][1],
        dpvsGlob.viewProjMtx->m[3][2],
        dpvsGlob.viewProjMtx->m[3][3]);
}

int __cdecl R_CellForPoint(const float *origin)
{
    mnode_t *node; // [esp+4h] [ebp-1Ch]
    cplane_s *plane; // [esp+Ch] [ebp-14h]
    float d; // [esp+10h] [ebp-10h]
    int cellIndex; // [esp+14h] [ebp-Ch]
    int cellCount; // [esp+18h] [ebp-8h]

    if (!rgp.world)
        MyAssertHandler(".\\r_dpvs.cpp", 566, 0, "%s", "rgp.world");
    node = (mnode_t *)rgp.world->dpvsPlanes.nodes;
    cellCount = rgp.world->dpvsPlanes.cellCount + 1;
    while (1)
    {
        cellIndex = node->cellIndex;
        if (cellIndex - cellCount < 0)
            break;
        plane = &rgp.world->dpvsPlanes.planes[cellIndex - cellCount];
        d = Vec3Dot(origin, plane->normal) - plane->dist;
        node = (mnode_t *)((char *)node + 2 * (d <= 0.0) * (node->rightChildOffset - 2) + 4);
    }
    return cellIndex - 1;
}

unsigned int __cdecl R_FindNearestReflectionProbeInCell(
    const GfxWorld *world,
    const GfxCell *cell,
    const float *origin)
{
    float diff[3]; // [esp+4h] [ebp-1Ch] BYREF
    float bestProbeDist; // [esp+10h] [ebp-10h]
    unsigned __int8 bestProbe; // [esp+16h] [ebp-Ah]
    unsigned __int8 probeIndex; // [esp+17h] [ebp-9h]
    float testProbeDist; // [esp+18h] [ebp-8h]
    unsigned int cellProbeIndex; // [esp+1Ch] [ebp-4h]

    if (world->reflectionProbeCount >= 0xFF)
        MyAssertHandler(".\\r_dpvs.cpp", 704, 0, "%s", "world->reflectionProbeCount < 0xff");
    bestProbe = 0;
    bestProbeDist = FLT_MAX;
    testProbeDist = FLT_MAX;
    if (!cell->reflectionProbeCount)
        MyAssertHandler(".\\r_dpvs.cpp", 710, 0, "%s", "cell->reflectionProbeCount > 0");
    for (cellProbeIndex = 0; cellProbeIndex < cell->reflectionProbeCount; ++cellProbeIndex)
    {
        probeIndex = cell->reflectionProbes[cellProbeIndex];
        if (probeIndex >= world->reflectionProbeCount)
            MyAssertHandler(
                ".\\r_dpvs.cpp",
                714,
                0,
                "probeIndex doesn't index world->reflectionProbeCount\n\t%i not in [0, %i)",
                probeIndex,
                world->reflectionProbeCount);
        Vec3Sub(origin, world->reflectionProbes[probeIndex].origin, diff);
        testProbeDist = Vec3LengthSq(diff);
        if (bestProbeDist > (double)testProbeDist)
        {
            bestProbeDist = testProbeDist;
            bestProbe = probeIndex;
        }
    }
    return bestProbe;
}

unsigned int __cdecl R_FindNearestReflectionProbe(const GfxWorld *world, const float *origin)
{
    float diff[3]; // [esp+4h] [ebp-18h] BYREF
    float bestProbeDist; // [esp+10h] [ebp-Ch]
    unsigned __int8 bestProbe; // [esp+16h] [ebp-6h]
    unsigned __int8 probeIndex; // [esp+17h] [ebp-5h]
    float testProbeDist; // [esp+18h] [ebp-4h]

    if (world->reflectionProbeCount >= 0xFF)
        MyAssertHandler(".\\r_dpvs.cpp", 735, 0, "%s", "world->reflectionProbeCount < 0xff");
    bestProbe = 0;
    bestProbeDist = FLT_MAX;
    testProbeDist = FLT_MAX;
    for (probeIndex = 1; probeIndex < world->reflectionProbeCount; ++probeIndex)
    {
        Vec3Sub(origin, world->reflectionProbes[probeIndex].origin, diff);
        testProbeDist = Vec3LengthSq(diff);
        if (bestProbeDist > (double)testProbeDist)
        {
            bestProbeDist = testProbeDist;
            bestProbe = probeIndex;
        }
    }
    return bestProbe;
}

unsigned int __cdecl R_CalcReflectionProbeIndex(const float *origin)
{
    unsigned int cellIndex; // [esp+0h] [ebp-4h]

    cellIndex = R_CellForPoint(origin);
    if (cellIndex == -1)
        return R_FindNearestReflectionProbe(rgp.world, origin);
    if (cellIndex >= rgp.world->dpvsPlanes.cellCount)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            765,
            0,
            "cellIndex doesn't index rgp.world->dpvsPlanes.cellCount\n\t%i not in [0, %i)",
            cellIndex,
            rgp.world->dpvsPlanes.cellCount);
    return R_FindNearestReflectionProbeInCell(rgp.world, &rgp.world->cells[cellIndex], origin);
}

void __cdecl R_AddAllSceneEntSurfacesCamera(const GfxViewInfo *viewInfo)
{
    bool v1; // [esp+Ch] [ebp-D4h]
    GfxSceneDynBrush *sceneDynBrush; // [esp+70h] [ebp-70h]
    DynEntityPose *dynEntPose; // [esp+74h] [ebp-6Ch]
    DynEntityPose *dynEntPosea; // [esp+74h] [ebp-6Ch]
    GfxSceneModel *sceneModel; // [esp+78h] [ebp-68h]
    GfxLightingInfo lightingInfo; // [esp+7Ch] [ebp-64h] BYREF
    GfxDrawSurf *lastDrawSurfs[3]; // [esp+80h] [ebp-60h] BYREF
    unsigned int sceneEntCount; // [esp+8Ch] [ebp-54h]
    DynEntityClient *dynEntClient; // [esp+90h] [ebp-50h]
    unsigned int reflectionProbeIndex; // [esp+94h] [ebp-4Ch]
    GfxEntity *gfxEnt; // [esp+98h] [ebp-48h]
    const DynEntityDef *dynEntDef; // [esp+9Ch] [ebp-44h]
    int depthHack; // [esp+A0h] [ebp-40h]
    unsigned int sceneEntIndex; // [esp+A4h] [ebp-3Ch]
    GfxSceneEntity *sceneEnt; // [esp+A8h] [ebp-38h]
    unsigned __int16 *cachedLightingHandle; // [esp+ACh] [ebp-34h]
    int isShadowReceiver; // [esp+B0h] [ebp-30h]
    GfxSceneBrush *sceneBrush; // [esp+B4h] [ebp-2Ch]
    GfxDrawSurf *drawSurfs[3]; // [esp+B8h] [ebp-28h] BYREF
    unsigned __int16 dynEntId; // [esp+C4h] [ebp-1Ch]
    unsigned int gfxEntIndex; // [esp+C8h] [ebp-18h]
    unsigned __int8 *sceneEntVisData; // [esp+CCh] [ebp-14h]
    GfxSceneDynModel *sceneDynModel; // [esp+D0h] [ebp-10h]
    unsigned int lightingHandle; // [esp+D4h] [ebp-Ch]
    const GfxBrushModel *bmodel; // [esp+D8h] [ebp-8h]
    int drawSurfCount; // [esp+DCh] [ebp-4h]

    //Profile_Begin(413);
    drawSurfs[0] = scene.drawSurfs[2];
    lastDrawSurfs[0] = &scene.drawSurfs[2][scene.maxDrawSurfCount[2]];
    drawSurfs[1] = scene.drawSurfs[5];
    lastDrawSurfs[1] = &scene.drawSurfs[5][scene.maxDrawSurfCount[5]];
    drawSurfs[2] = scene.drawSurfs[11];
    lastDrawSurfs[2] = &scene.drawSurfs[11][scene.maxDrawSurfCount[11]];
    sceneEntCount = scene.sceneDObjCount;
    sceneEntVisData = scene.sceneDObjVisData[0];
    for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
    {
        if (sceneEntVisData[sceneEntIndex] == 1)
        {
            sceneEnt = &scene.sceneDObj[sceneEntIndex];
            iassert(sceneEnt->cull.state >= CULL_STATE_BOUNDED);
            cachedLightingHandle = (unsigned __int16 *)LongNoSwap((unsigned int)sceneEnt->info.cachedLightingHandle);
            lightingHandle = R_AllocModelLighting_Box(
                viewInfo,
                sceneEnt->lightingOrigin,
                sceneEnt->cull.mins,
                sceneEnt->cull.maxs,
                cachedLightingHandle,
                &lightingInfo);
            if (lightingHandle)
            {
                sceneEnt->reflectionProbeIndex = lightingInfo.reflectionProbeIndex;
                R_AddDObjSurfacesCamera(sceneEnt, lightingHandle, lightingInfo.primaryLightIndex, drawSurfs, lastDrawSurfs);
            }
            else
            {
                sceneEntVisData[sceneEntIndex] = 0;
            }
        }
    }
    sceneEntCount = scene.sceneModelCount;
    sceneEntVisData = scene.sceneModelVisData[0];
    for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
    {
        if (sceneEntVisData[sceneEntIndex] == 1)
        {
            sceneModel = &scene.sceneModel[sceneEntIndex];
            lightingHandle = R_AllocModelLighting_Sphere(
                viewInfo,
                sceneModel->lightingOrigin,
                sceneModel->placement.base.origin,
                sceneModel->radius,
                sceneModel->cachedLightingHandle,
                &lightingInfo);
            if (lightingHandle)
            {
                gfxEntIndex = sceneModel->gfxEntIndex;
                if (gfxEntIndex)
                {
                    gfxEnt = &frontEndDataOut->gfxEnts[gfxEntIndex];
                    isShadowReceiver = sc_enable->current.enabled && (gfxEnt->renderFxFlags & 0x100) != 0;
                    depthHack = (gfxEnt->renderFxFlags & 2) != 0;
                }
                else
                {
                    isShadowReceiver = 0;
                    depthHack = 0;
                }
                sceneModel->reflectionProbeIndex = lightingInfo.reflectionProbeIndex;
                R_AddXModelSurfacesCamera(
                    &sceneModel->info,
                    sceneModel->model,
                    sceneModel->placement.base.origin,
                    sceneModel->gfxEntIndex,
                    lightingHandle,
                    lightingInfo.primaryLightIndex,
                    isShadowReceiver,
                    depthHack,
                    drawSurfs,
                    lastDrawSurfs,
                    lightingInfo.reflectionProbeIndex);
            }
            else
            {
                sceneEntVisData[sceneEntIndex] = 0;
            }
        }
    }
    sceneEntCount = scene.sceneDynModelCount;
    sceneEntVisData = rgp.world->dpvsDyn.dynEntVisData[0][0];
    for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
    {
        sceneDynModel = &rgp.world->sceneDynModel[sceneEntIndex];
        dynEntId = sceneDynModel->dynEntId;
        if (sceneEntVisData[dynEntId] == 1)
        {
            dynEntPose = DynEnt_GetClientPose(dynEntId, DYNENT_DRAW_MODEL);
            dynEntClient = DynEnt_GetClientEntity(dynEntId, DYNENT_DRAW_MODEL);
            lightingHandle = R_AllocModelLighting_PrimaryLight(
                dynEntPose->pose.origin,
                dynEntId,
                &dynEntClient->lightingHandle,
                &lightingInfo);
            if (lightingHandle)
            {
                dynEntDef = DynEnt_GetEntityDef(dynEntId, DYNENT_DRAW_MODEL);
                R_AddXModelSurfacesCamera(
                    &sceneDynModel->info,
                    dynEntDef->xModel,
                    dynEntPose->pose.origin,
                    0,
                    lightingHandle,
                    lightingInfo.primaryLightIndex,
                    0,
                    0,
                    drawSurfs,
                    lastDrawSurfs,
                    lightingInfo.reflectionProbeIndex);
            }
            else
            {
                sceneEntVisData[sceneEntIndex] = 0;
            }
        }
    }
    sceneEntCount = scene.sceneBrushCount;
    sceneEntVisData = scene.sceneBrushVisData[0];
    for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
    {
        if (sceneEntVisData[sceneEntIndex] == 1)
        {
            sceneBrush = &scene.sceneBrush[sceneEntIndex];
            reflectionProbeIndex = R_CalcReflectionProbeIndex(sceneBrush->placement.origin);
            sceneBrush->reflectionProbeIndex = reflectionProbeIndex;
            iassert(sceneBrush->reflectionProbeIndex == reflectionProbeIndex);
            R_AddBModelSurfacesCamera(&sceneBrush->info, sceneBrush->bmodel, drawSurfs, lastDrawSurfs, reflectionProbeIndex);
        }
    }
    sceneEntCount = scene.sceneDynBrushCount;
    sceneEntVisData = rgp.world->dpvsDyn.dynEntVisData[1][0];
    for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
    {
        sceneDynBrush = &rgp.world->sceneDynBrush[sceneEntIndex];
        dynEntId = sceneDynBrush->dynEntId;
        if (sceneEntVisData[dynEntId] == 1)
        {
            dynEntPosea = DynEnt_GetClientPose(dynEntId, DYNENT_DRAW_BRUSH);
            dynEntDef = DynEnt_GetEntityDef(dynEntId, DYNENT_DRAW_BRUSH);
            bmodel = R_GetBrushModel(dynEntDef->brushModel);
            reflectionProbeIndex = R_CalcReflectionProbeIndex(dynEntPosea->pose.origin);
            R_AddBModelSurfacesCamera((BModelDrawInfo *)sceneDynBrush, bmodel, drawSurfs, lastDrawSurfs, reflectionProbeIndex);
        }
    }
    //Profile_EndInternal(0);
    //Profile_Begin(414);
    drawSurfCount = drawSurfs[0] - scene.drawSurfs[2];
    scene.drawSurfCount[2] = drawSurfCount;
    CL_ResetStats_f();
    R_SortDrawSurfs(scene.drawSurfs[2], drawSurfCount);
    drawSurfCount = drawSurfs[1] - scene.drawSurfs[5];
    scene.drawSurfCount[5] = drawSurfCount;
    CL_ResetStats_f();
    R_SortDrawSurfs(scene.drawSurfs[5], drawSurfCount);
    drawSurfCount = drawSurfs[2] - scene.drawSurfs[11];
    scene.drawSurfCount[11] = drawSurfCount;
    CL_ResetStats_f();
    R_SortDrawSurfs(scene.drawSurfs[11], drawSurfCount);
    //Profile_EndInternal(0);
}

void __cdecl R_AddAllSceneEntSurfacesSunShadow()
{
    unsigned int partitionIndex; // [esp+0h] [ebp-4h]

    for (partitionIndex = 0; partitionIndex < 2; ++partitionIndex)
        R_AddAllSceneEntSurfacesRangeSunShadow(partitionIndex);
}

void __cdecl R_AddAllSceneEntSurfacesRangeSunShadow(unsigned int partitionIndex)
{
    GfxSceneDynBrush *sceneDynBrush; // [esp+30h] [ebp-40h]
    GfxDrawSurf *drawSurf; // [esp+38h] [ebp-38h]
    MaterialTechniqueType shadowmapBuildTechType; // [esp+3Ch] [ebp-34h]
    volatile unsigned int sceneEntCount; // [esp+40h] [ebp-30h]
    volatile unsigned int sceneEntCounta; // [esp+40h] [ebp-30h]
    unsigned int sceneEntCountb; // [esp+40h] [ebp-30h]
    volatile unsigned int sceneEntCountc; // [esp+40h] [ebp-30h]
    unsigned int sceneEntCountd; // [esp+40h] [ebp-30h]
    unsigned int stage; // [esp+44h] [ebp-2Ch]
    const DynEntityDef *dynEntDef; // [esp+48h] [ebp-28h]
    const DynEntityDef *dynEntDefa; // [esp+48h] [ebp-28h]
    unsigned int sceneEntIndex; // [esp+4Ch] [ebp-24h]
    unsigned int sceneEntIndexa; // [esp+4Ch] [ebp-24h]
    unsigned int sceneEntIndexb; // [esp+4Ch] [ebp-24h]
    unsigned int sceneEntIndexc; // [esp+4Ch] [ebp-24h]
    unsigned int sceneEntIndexd; // [esp+4Ch] [ebp-24h]
    unsigned __int16 dynEntId; // [esp+58h] [ebp-18h]
    unsigned __int16 dynEntIda; // [esp+58h] [ebp-18h]
    unsigned __int8 *sceneEntVisData; // [esp+5Ch] [ebp-14h]
    unsigned __int8 *sceneEntVisDataa; // [esp+5Ch] [ebp-14h]
    GfxSceneDynModel *sceneDynModel; // [esp+60h] [ebp-10h]
    GfxBrushModel *bmodel; // [esp+64h] [ebp-Ch]
    signed int drawSurfCount; // [esp+68h] [ebp-8h]
    GfxDrawSurf *lastDrawSurf; // [esp+6Ch] [ebp-4h]

    //Profile_Begin(415);
    stage = 3 * partitionIndex + 17;
    drawSurf = scene.drawSurfs[stage];
    lastDrawSurf = &drawSurf[scene.maxDrawSurfCount[stage]];
    shadowmapBuildTechType = gfxMetrics.shadowmapBuildTechType;
    sceneEntCount = scene.sceneDObjCount;
    for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
    {
        if (scene.sceneDObjVisData[partitionIndex + 1][sceneEntIndex] == 1)
            drawSurf = R_AddDObjSurfaces(&scene.sceneDObj[sceneEntIndex], shadowmapBuildTechType, drawSurf, lastDrawSurf);
    }
    sceneEntCounta = scene.sceneModelCount;
    for (sceneEntIndexa = 0; sceneEntIndexa < sceneEntCounta; ++sceneEntIndexa)
    {
        if (scene.sceneModelVisData[partitionIndex + 1][sceneEntIndexa] == 1)
            drawSurf = R_AddXModelSurfaces(
                &scene.sceneModel[sceneEntIndexa].info,
                scene.sceneModel[sceneEntIndexa].model,
                shadowmapBuildTechType,
                drawSurf,
                lastDrawSurf);
    }
    sceneEntCountb = scene.sceneDynModelCount;
    sceneEntVisData = rgp.world->dpvsDyn.dynEntVisData[0][partitionIndex + 1];
    for (sceneEntIndexb = 0; sceneEntIndexb < sceneEntCountb; ++sceneEntIndexb)
    {
        sceneDynModel = &rgp.world->sceneDynModel[sceneEntIndexb];
        dynEntId = sceneDynModel->dynEntId;
        if (sceneEntVisData[dynEntId] == 1)
        {
            dynEntDef = DynEnt_GetEntityDef(dynEntId, DYNENT_DRAW_MODEL);
            drawSurf = R_AddXModelSurfaces(
                &sceneDynModel->info,
                dynEntDef->xModel,
                shadowmapBuildTechType,
                drawSurf,
                lastDrawSurf);
        }
    }
    sceneEntCountc = scene.sceneBrushCount;
    for (sceneEntIndexc = 0; sceneEntIndexc < sceneEntCountc; ++sceneEntIndexc)
    {
        if (scene.sceneBrushVisData[partitionIndex + 1][sceneEntIndexc] == 1)
            drawSurf = R_AddBModelSurfaces(
                &scene.sceneBrush[sceneEntIndexc].info,
                scene.sceneBrush[sceneEntIndexc].bmodel,
                shadowmapBuildTechType,
                drawSurf,
                lastDrawSurf);
    }
    sceneEntCountd = scene.sceneDynBrushCount;
    sceneEntVisDataa = rgp.world->dpvsDyn.dynEntVisData[1][partitionIndex + 1];
    for (sceneEntIndexd = 0; sceneEntIndexd < sceneEntCountd; ++sceneEntIndexd)
    {
        sceneDynBrush = &rgp.world->sceneDynBrush[sceneEntIndexd];
        dynEntIda = sceneDynBrush->dynEntId;
        if (sceneEntVisDataa[dynEntIda] == 1)
        {
            dynEntDefa = DynEnt_GetEntityDef(dynEntIda, DYNENT_DRAW_BRUSH);
            bmodel = R_GetBrushModel(dynEntDefa->brushModel);
            R_AddBModelSurfaces((BModelDrawInfo *)sceneDynBrush, bmodel, shadowmapBuildTechType, drawSurf, lastDrawSurf);
        }
    }
    drawSurfCount = drawSurf - scene.drawSurfs[stage];
    scene.drawSurfCount[stage] = drawSurfCount;
    CL_ResetStats_f();
    R_SortDrawSurfs(scene.drawSurfs[stage], drawSurfCount);
    //Profile_EndInternal(0);
}

void __cdecl R_AddAllSceneEntSurfacesSpotShadow(
    const GfxViewInfo *viewInfo,
    unsigned int spotShadowIndex,
    unsigned int primaryLightIndex)
{
    GfxSceneDynBrush *sceneDynBrush; // [esp+0h] [ebp-44h]
    GfxDrawSurf *drawSurf; // [esp+8h] [ebp-3Ch]
    MaterialTechniqueType shadowmapBuildTechType; // [esp+Ch] [ebp-38h]
    volatile unsigned int sceneEntCount; // [esp+10h] [ebp-34h]
    volatile unsigned int sceneEntCounta; // [esp+10h] [ebp-34h]
    unsigned int sceneEntCountb; // [esp+10h] [ebp-34h]
    volatile unsigned int sceneEntCountc; // [esp+10h] [ebp-34h]
    unsigned int sceneEntCountd; // [esp+10h] [ebp-34h]
    unsigned int stage; // [esp+18h] [ebp-2Ch]
    const DynEntityDef *dynEntDef; // [esp+1Ch] [ebp-28h]
    const DynEntityDef *dynEntDefa; // [esp+1Ch] [ebp-28h]
    unsigned int sceneEntIndex; // [esp+20h] [ebp-24h]
    unsigned int sceneEntIndexa; // [esp+20h] [ebp-24h]
    unsigned int sceneEntIndexb; // [esp+20h] [ebp-24h]
    unsigned int sceneEntIndexc; // [esp+20h] [ebp-24h]
    unsigned int sceneEntIndexd; // [esp+20h] [ebp-24h]
    GfxSceneBrush *sceneBrush; // [esp+28h] [ebp-1Ch]
    unsigned __int16 dynEntId; // [esp+2Ch] [ebp-18h]
    unsigned __int16 dynEntIda; // [esp+2Ch] [ebp-18h]
    GfxSceneDynModel *sceneDynModel; // [esp+34h] [ebp-10h]
    GfxBrushModel *bmodel; // [esp+38h] [ebp-Ch]
    signed int drawSurfCount; // [esp+3Ch] [ebp-8h]
    GfxDrawSurf *lastDrawSurf; // [esp+40h] [ebp-4h]

    if (!R_IsPrimaryLight(primaryLightIndex))
        MyAssertHandler(".\\r_dpvs.cpp", 1107, 0, "%s", "R_IsPrimaryLight( primaryLightIndex )");
    stage = 3 * spotShadowIndex + 23;
    drawSurf = scene.drawSurfs[stage];
    lastDrawSurf = &drawSurf[scene.maxDrawSurfCount[stage]];
    shadowmapBuildTechType = gfxMetrics.shadowmapBuildTechType;
    sceneEntCount = scene.sceneDObjCount;
    for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
    {
        if (scene.sceneDObjVisData[spotShadowIndex + 3][sceneEntIndex] == 1)
            drawSurf = R_AddDObjSurfaces(&scene.sceneDObj[sceneEntIndex], shadowmapBuildTechType, drawSurf, lastDrawSurf);
    }
    sceneEntCounta = scene.sceneModelCount;
    for (sceneEntIndexa = 0; sceneEntIndexa < sceneEntCounta; ++sceneEntIndexa)
    {
        if (scene.sceneModelVisData[spotShadowIndex + 3][sceneEntIndexa] == 1)
            drawSurf = R_AddXModelSurfaces(
                &scene.sceneModel[sceneEntIndexa].info,
                scene.sceneModel[sceneEntIndexa].model,
                shadowmapBuildTechType,
                drawSurf,
                lastDrawSurf);
    }
    sceneEntCountb = scene.sceneDynModelCount;
    for (sceneEntIndexb = 0; sceneEntIndexb < sceneEntCountb; ++sceneEntIndexb)
    {
        sceneDynModel = &rgp.world->sceneDynModel[sceneEntIndexb];
        dynEntId = sceneDynModel->dynEntId;
        if (R_IsDynEntVisibleToPrimaryLight(dynEntId, DYNENT_DRAW_MODEL, primaryLightIndex))
        {
            dynEntDef = DynEnt_GetEntityDef(dynEntId, DYNENT_DRAW_MODEL);
            drawSurf = R_AddXModelSurfaces(
                &sceneDynModel->info,
                dynEntDef->xModel,
                shadowmapBuildTechType,
                drawSurf,
                lastDrawSurf);
        }
    }
    sceneEntCountc = scene.sceneBrushCount;
    for (sceneEntIndexc = 0; sceneEntIndexc < sceneEntCountc; ++sceneEntIndexc)
    {
        sceneBrush = &scene.sceneBrush[sceneEntIndexc];
        if (R_IsEntityVisibleToPrimaryLight(viewInfo->localClientNum, sceneBrush->entnum, primaryLightIndex)
            && !Com_BitCheckAssert(scene.entOverflowedDrawBuf, sceneBrush->entnum, 0xFFFFFFF))
        {
            drawSurf = R_AddBModelSurfaces(
                &sceneBrush->info,
                sceneBrush->bmodel,
                shadowmapBuildTechType,
                drawSurf,
                lastDrawSurf);
        }
    }
    sceneEntCountd = scene.sceneDynBrushCount;
    for (sceneEntIndexd = 0; sceneEntIndexd < sceneEntCountd; ++sceneEntIndexd)
    {
        sceneDynBrush = &rgp.world->sceneDynBrush[sceneEntIndexd];
        dynEntIda = sceneDynBrush->dynEntId;
        if (R_IsDynEntVisibleToPrimaryLight(dynEntIda, DYNENT_DRAW_BRUSH, primaryLightIndex))
        {
            dynEntDefa = DynEnt_GetEntityDef(dynEntIda, DYNENT_DRAW_BRUSH);
            bmodel = R_GetBrushModel(dynEntDefa->brushModel);
            R_AddBModelSurfaces((BModelDrawInfo *)sceneDynBrush, bmodel, shadowmapBuildTechType, drawSurf, lastDrawSurf);
        }
    }
    drawSurfCount = drawSurf - scene.drawSurfs[stage];
    scene.drawSurfCount[stage] = drawSurfCount;
    CL_ResetStats_f();
    R_SortDrawSurfs(scene.drawSurfs[stage], drawSurfCount);
}

void __cdecl R_AddSceneDObj(unsigned int entnum, unsigned int viewIndex)
{
    if (entnum == gfxCfg.entnumNone)
        MyAssertHandler(".\\r_dpvs.cpp", 1198, 0, "%s", "entnum != gfxCfg.entnumNone");
    scene.dpvs.entVisData[viewIndex][entnum] = 1;
}

void __cdecl R_DrawAllSceneEnt(const GfxViewInfo *viewInfo)
{
    unsigned __int8 v1; // [esp+Ch] [ebp-58h]
    unsigned __int8 v2; // [esp+10h] [ebp-54h]
    unsigned __int8 viewVisData; // [esp+14h] [ebp-50h]
    unsigned __int8 viewVisDataa; // [esp+14h] [ebp-50h]
    unsigned __int8 viewVisDatab; // [esp+14h] [ebp-50h]
    unsigned __int8 viewVisDatac; // [esp+14h] [ebp-50h]
    unsigned __int8 viewVisDatad; // [esp+14h] [ebp-50h]
    unsigned int *entVisBits; // [esp+18h] [ebp-4Ch]
    GfxSceneModel *sceneModel; // [esp+1Ch] [ebp-48h]
    volatile unsigned int sceneEntCount; // [esp+20h] [ebp-44h]
    volatile unsigned int sceneEntCounta; // [esp+20h] [ebp-44h]
    volatile unsigned int sceneEntCountb; // [esp+20h] [ebp-44h]
    DpvsView *views; // [esp+24h] [ebp-40h]
    GfxEntity *gfxEnt; // [esp+28h] [ebp-3Ch]
    GfxEntity *gfxEnta; // [esp+28h] [ebp-3Ch]
    unsigned int sceneEntIndex; // [esp+2Ch] [ebp-38h]
    unsigned int sceneEntIndexa; // [esp+2Ch] [ebp-38h]
    unsigned int sceneEntIndexb; // [esp+2Ch] [ebp-38h]
    GfxSceneEntity *sceneEnt; // [esp+30h] [ebp-34h]
    GfxSceneBrush *sceneBrush; // [esp+34h] [ebp-30h]
    unsigned int entnum; // [esp+3Ch] [ebp-28h]
    unsigned int entnuma; // [esp+3Ch] [ebp-28h]
    unsigned int entnumb; // [esp+3Ch] [ebp-28h]
    unsigned __int8 *sceneEntVisData[7]; // [esp+40h] [ebp-24h]
    unsigned int viewIndex; // [esp+5Ch] [ebp-8h]
    unsigned int visData; // [esp+60h] [ebp-4h]
    int savedregs; // [esp+64h] [ebp+0h] BYREF

    entVisBits = dpvsGlob.entVisBits[scene.dpvs.localClientNum];
    for (viewIndex = 0; viewIndex < 7; ++viewIndex)
        sceneEntVisData[viewIndex] = scene.sceneDObjVisData[viewIndex];
    sceneEntCount = scene.sceneDObjCount;
    if (scene.dpvs.localClientNum != viewInfo->localClientNum)
        MyAssertHandler(".\\r_dpvs.cpp", 1353, 0, "%s", "scene.dpvs.localClientNum == (uint)viewInfo->localClientNum");
    views = dpvsGlob.views[scene.dpvs.localClientNum];
    for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
    {
        sceneEnt = &scene.sceneDObj[sceneEntIndex];
        entnum = sceneEnt->entnum;
        if (sceneEnt->gfxEntIndex)
        {
            gfxEnt = &frontEndDataOut->gfxEnts[scene.sceneDObj[sceneEntIndex].gfxEntIndex];
            if (entnum == gfxCfg.entnumNone)
            {
                visData = 0;
                for (viewIndex = 0; viewIndex < 3; ++viewIndex)
                {
                    sceneEntVisData[viewIndex][sceneEntIndex] = (views[viewIndex].renderFxFlagsCull & gfxEnt->renderFxFlags) == 0;
                    visData |= sceneEntVisData[viewIndex][sceneEntIndex];
                }
                while (viewIndex < 7)
                {
                    sceneEntVisData[viewIndex][sceneEntIndex] = scene.dpvs.entVisData[viewIndex][entnum];
                    visData |= sceneEntVisData[viewIndex++][sceneEntIndex];
                }
            }
            else
            {
                if ((entVisBits[entnum >> 5] & (0x80000000 >> (entnum & 0x1F))) != 0)
                {
                    visData = 0;
                    for (viewIndex = 0; viewIndex < 3; ++viewIndex)
                    {
                        if ((views[viewIndex].renderFxFlagsCull & gfxEnt->renderFxFlags) != 0)
                            v2 = 0;
                        else
                            v2 = scene.dpvs.entVisData[viewIndex][entnum];
                        sceneEntVisData[viewIndex][sceneEntIndex] = v2;
                        visData |= sceneEntVisData[viewIndex][sceneEntIndex];
                    }
                    while (viewIndex < 7)
                    {
                        sceneEntVisData[viewIndex][sceneEntIndex] = scene.dpvs.entVisData[viewIndex][entnum];
                        visData |= sceneEntVisData[viewIndex++][sceneEntIndex];
                    }
                    if ((visData & 1) != 0 && sceneEnt->cull.state < 2)
                        MyAssertHandler(
                            ".\\r_dpvs.cpp",
                            1393,
                            0,
                            "sceneEnt->cull.state >= CULL_STATE_BOUNDED\n\t%i, %i",
                            sceneEnt->cull.state,
                            2);
                    continue;
                }
                visData = 0;
                for (viewIndex = 0; viewIndex < 3; ++viewIndex)
                {
                    viewVisData = scene.dpvs.entVisData[viewIndex][entnum];
                    if (!viewVisData)
                        viewVisData = 1;
                    sceneEntVisData[viewIndex][sceneEntIndex] = (views[viewIndex].renderFxFlagsCull & gfxEnt->renderFxFlags) == 0
                        ? viewVisData
                        : 0;
                    visData |= sceneEntVisData[viewIndex][sceneEntIndex];
                }
                while (viewIndex < 7)
                {
                    sceneEntVisData[viewIndex][sceneEntIndex] = scene.dpvs.entVisData[viewIndex][entnum];
                    visData |= sceneEntVisData[viewIndex++][sceneEntIndex];
                }
            }
            goto LABEL_62;
        }
        if (entnum == gfxCfg.entnumNone)
        {
            visData = 0;
            for (viewIndex = 0; viewIndex < 3; ++viewIndex)
            {
                sceneEntVisData[viewIndex][sceneEntIndex] = 1;
                visData |= sceneEntVisData[viewIndex][sceneEntIndex];
            }
            while (viewIndex < 7)
            {
                sceneEntVisData[viewIndex][sceneEntIndex] = scene.dpvs.entVisData[viewIndex][entnum];
                visData |= sceneEntVisData[viewIndex++][sceneEntIndex];
            }
        LABEL_62:
            if ((visData & 1) != 0)
            {
                if (R_SkinAndBoundSceneEnt(sceneEnt))
                {
                    if (sceneEnt->cull.state < 2)
                        MyAssertHandler(
                            ".\\r_dpvs.cpp",
                            1505,
                            0,
                            "sceneEnt->cull.state >= CULL_STATE_BOUNDED\n\t%i, %i",
                            sceneEnt->cull.state,
                            2);
                }
                else
                {
                    for (viewIndex = 0; viewIndex < 7; ++viewIndex)
                        sceneEntVisData[viewIndex][sceneEntIndex] = 0;
                }
            }
            continue;
        }
        if ((entVisBits[entnum >> 5] & (0x80000000 >> (entnum & 0x1F))) == 0)
        {
            visData = 0;
            for (viewIndex = 0; viewIndex < 3; ++viewIndex)
            {
                viewVisDataa = scene.dpvs.entVisData[viewIndex][entnum];
                if (!viewVisDataa)
                    viewVisDataa = 1;
                sceneEntVisData[viewIndex][sceneEntIndex] = viewVisDataa;
                visData |= sceneEntVisData[viewIndex][sceneEntIndex];
            }
            while (viewIndex < 7)
            {
                sceneEntVisData[viewIndex][sceneEntIndex] = scene.dpvs.entVisData[viewIndex][entnum];
                visData |= sceneEntVisData[viewIndex++][sceneEntIndex];
            }
            goto LABEL_62;
        }
        visData = 0;
        for (viewIndex = 0; viewIndex < 3; ++viewIndex)
        {
            sceneEntVisData[viewIndex][sceneEntIndex] = scene.dpvs.entVisData[viewIndex][entnum];
            visData |= sceneEntVisData[viewIndex][sceneEntIndex];
        }
        while (viewIndex < 7)
        {
            sceneEntVisData[viewIndex][sceneEntIndex] = scene.dpvs.entVisData[viewIndex][entnum];
            visData |= sceneEntVisData[viewIndex++][sceneEntIndex];
        }
        if ((visData & 1) != 0 && sceneEnt->cull.state < 2)
            MyAssertHandler(
                ".\\r_dpvs.cpp",
                1460,
                0,
                "sceneEnt->cull.state >= CULL_STATE_BOUNDED\n\t%i, %i",
                sceneEnt->cull.state,
                2);
    }
    for (viewIndex = 0; viewIndex < 7; ++viewIndex)
        sceneEntVisData[viewIndex] = scene.sceneModelVisData[viewIndex];
    sceneEntCounta = scene.sceneModelCount;
    for (sceneEntIndexa = 0; sceneEntIndexa < sceneEntCounta; ++sceneEntIndexa)
    {
        entnuma = scene.sceneModel[sceneEntIndexa].entnum;
        visData = 0;
        if (scene.sceneModel[sceneEntIndexa].gfxEntIndex)
        {
            gfxEnta = &frontEndDataOut->gfxEnts[scene.sceneModel[sceneEntIndexa].gfxEntIndex];
            if (entnuma == gfxCfg.entnumNone)
            {
                for (viewIndex = 0; viewIndex < 3; ++viewIndex)
                {
                    if ((views[viewIndex].renderFxFlagsCull & gfxEnta->renderFxFlags) != 0)
                        sceneEntVisData[viewIndex][sceneEntIndexa] = 0;
                    else
                        visData |= sceneEntVisData[viewIndex][sceneEntIndexa];
                }
            }
            else if ((entVisBits[entnuma >> 5] & (0x80000000 >> (entnuma & 0x1F))) != 0)
            {
                for (viewIndex = 0; viewIndex < 3; ++viewIndex)
                {
                    if ((views[viewIndex].renderFxFlagsCull & gfxEnta->renderFxFlags) != 0)
                        v1 = 0;
                    else
                        v1 = scene.dpvs.entVisData[viewIndex][entnuma];
                    sceneEntVisData[viewIndex][sceneEntIndexa] = v1;
                    visData |= sceneEntVisData[viewIndex][sceneEntIndexa];
                }
            }
            else
            {
                for (viewIndex = 0; viewIndex < 3; ++viewIndex)
                {
                    viewVisDatab = scene.dpvs.entVisData[viewIndex][entnuma];
                    if (!viewVisDatab)
                        viewVisDatab = 1;
                    sceneEntVisData[viewIndex][sceneEntIndexa] = (views[viewIndex].renderFxFlagsCull & gfxEnta->renderFxFlags) == 0
                        ? viewVisDatab
                        : 0;
                    visData |= sceneEntVisData[viewIndex][sceneEntIndexa];
                }
            }
            while (viewIndex < 7)
            {
                sceneEntVisData[viewIndex][sceneEntIndexa] = scene.dpvs.entVisData[viewIndex][entnuma];
                visData |= sceneEntVisData[viewIndex++][sceneEntIndexa];
            }
        }
        else
        {
            if (entnuma == gfxCfg.entnumNone)
            {
                for (viewIndex = 0; viewIndex < 3; ++viewIndex)
                    visData |= sceneEntVisData[viewIndex][sceneEntIndexa];
            }
            else if ((entVisBits[entnuma >> 5] & (0x80000000 >> (entnuma & 0x1F))) != 0)
            {
                for (viewIndex = 0; viewIndex < 3; ++viewIndex)
                {
                    sceneEntVisData[viewIndex][sceneEntIndexa] = scene.dpvs.entVisData[viewIndex][entnuma];
                    visData |= sceneEntVisData[viewIndex][sceneEntIndexa];
                }
            }
            else
            {
                for (viewIndex = 0; viewIndex < 3; ++viewIndex)
                {
                    viewVisDatac = scene.dpvs.entVisData[viewIndex][entnuma];
                    if (!viewVisDatac)
                        viewVisDatac = 1;
                    sceneEntVisData[viewIndex][sceneEntIndexa] = viewVisDatac;
                    visData |= sceneEntVisData[viewIndex][sceneEntIndexa];
                }
            }
            while (viewIndex < 7)
            {
                sceneEntVisData[viewIndex][sceneEntIndexa] = scene.dpvs.entVisData[viewIndex][entnuma];
                visData |= sceneEntVisData[viewIndex++][sceneEntIndexa];
            }
        }
        if ((visData & 1) != 0)
        {
            sceneModel = &scene.sceneModel[sceneEntIndexa];
            if (!R_SkinXModel(
                &sceneModel->info,
                sceneModel->model,
                sceneModel->obj,
                &sceneModel->placement.base,
                sceneModel->placement.scale,
                sceneModel->gfxEntIndex))
            {
                for (viewIndex = 0; viewIndex < 7; ++viewIndex)
                    sceneEntVisData[viewIndex][sceneEntIndexa] = 0;
            }
        }
    }
    for (viewIndex = 0; viewIndex < 3; ++viewIndex)
        sceneEntVisData[viewIndex] = scene.sceneBrushVisData[viewIndex];
    sceneEntCountb = scene.sceneBrushCount;
    for (sceneEntIndexb = 0; sceneEntIndexb < sceneEntCountb; ++sceneEntIndexb)
    {
        sceneBrush = &scene.sceneBrush[sceneEntIndexb];
        entnumb = sceneBrush->entnum;
        if (entnumb == gfxCfg.entnumNone)
            MyAssertHandler(".\\r_dpvs.cpp", 1625, 0, "%s", "entnum != gfxCfg.entnumNone");
        visData = 0;
        if ((entVisBits[entnumb >> 5] & (0x80000000 >> (entnumb & 0x1F))) != 0)
        {
            for (viewIndex = 0; viewIndex < 3; ++viewIndex)
            {
                sceneEntVisData[viewIndex][sceneEntIndexb] = scene.dpvs.entVisData[viewIndex][entnumb];
                visData |= sceneEntVisData[viewIndex][sceneEntIndexb];
            }
        }
        else
        {
            for (viewIndex = 0; viewIndex < 3; ++viewIndex)
            {
                viewVisDatad = scene.dpvs.entVisData[viewIndex][entnumb];
                if (!viewVisDatad)
                    viewVisDatad = 1;
                sceneEntVisData[viewIndex][sceneEntIndexb] = viewVisDatad;
                visData |= sceneEntVisData[viewIndex][sceneEntIndexb];
            }
        }
        if (((visData & 1) != 0 || R_IsEntityVisibleToAnyShadowedPrimaryLight(viewInfo, entnumb))
            && !R_DrawBModel(&sceneBrush->info, sceneBrush->bmodel, &sceneBrush->placement))
        {
            Com_BitSetAssert(scene.entOverflowedDrawBuf, sceneBrush->entnum, 0xFFFFFFF);
            for (viewIndex = 0; viewIndex < 3; ++viewIndex)
                sceneEntVisData[viewIndex][sceneEntIndexb] = 0;
        }
    }
}

int __cdecl R_DrawBModel(BModelDrawInfo *bmodelInfo, const GfxBrushModel *bmodel, const GfxPlacement *placement)
{
    unsigned __int16 surfaceCount; // [esp+Ah] [ebp-26h]
    unsigned int surfId; // [esp+10h] [ebp-20h]
    int startSurfPos; // [esp+14h] [ebp-1Ch]
    GfxScaledPlacement *newPlacement; // [esp+18h] [ebp-18h]
    unsigned int surfIndex; // [esp+1Ch] [ebp-14h]
    unsigned int surfIndexa; // [esp+1Ch] [ebp-14h]
    const GfxSurface *surf; // [esp+24h] [ebp-Ch]
    const GfxSurface *surfa; // [esp+24h] [ebp-Ch]
    BModelSurface *bmodelSurf; // [esp+28h] [ebp-8h]

    if (r_drawDecals->current.enabled)
        surfaceCount = bmodel->surfaceCount;
    else
        surfaceCount = bmodel->surfaceCountNoDecal;
    if (!surfaceCount)
        MyAssertHandler(".\\r_dpvs.cpp", 1220, 0, "%s", "visibleSurfaceCount");
    startSurfPos = InterlockedExchangeAdd(&frontEndDataOut->surfPos, 8 * surfaceCount + 32);
    if (8 * (unsigned int)surfaceCount + 32 + startSurfPos <= 0x20000)
    {
        if ((startSurfPos & 3) != 0)
            MyAssertHandler(".\\r_dpvs.cpp", 1236, 0, "%s", "!(startSurfPos & 3)");
        newPlacement = (GfxScaledPlacement *)&frontEndDataOut->surfsBuffer[startSurfPos];
        memcpy(&frontEndDataOut->surfsBuffer[startSurfPos], placement, 0x1Cu);
        newPlacement->scale = 1.0;
        bmodelSurf = (BModelSurface *)&newPlacement[1];
        surfId = (char *)&newPlacement[1] - (char *)frontEndDataOut;
        if ((((_BYTE)newPlacement + 32 - (_BYTE)frontEndDataOut) & 3) != 0)
            MyAssertHandler(".\\r_dpvs.cpp", 1244, 0, "%s", "!(surfId & 3)");
        bmodelInfo->surfId = surfId >> 2;
        if (r_drawDecals->current.enabled)
        {
            surfIndexa = 0;
            surfa = &rgp.world->dpvs.surfaces[bmodel->startSurfIndex];
            while (surfIndexa < bmodel->surfaceCount)
            {
                bmodelSurf->placement = newPlacement;
                bmodelSurf->surf = surfa;
                ++bmodelSurf;
                ++surfIndexa;
                ++surfa;
            }
        }
        else
        {
            surfIndex = 0;
            surf = &rgp.world->dpvs.surfaces[bmodel->startSurfIndex];
            while (surfIndex < bmodel->surfaceCount)
            {
                if ((surf->flags & 2) == 0)
                {
                    bmodelSurf->placement = newPlacement;
                    bmodelSurf->surf = surf;
                    ++bmodelSurf;
                }
                ++surfIndex;
                ++surf;
            }
        }
        return 1;
    }
    else
    {
        R_WarnOncePerFrame(R_WARN_MAX_SCENE_SURFS_SIZE);
        return 0;
    }
}

void __cdecl R_DrawAllDynEnt(const GfxViewInfo *viewInfo)
{
    DynEntityPose *dynEntPose; // [esp+38h] [ebp-38h]
    DynEntityPose *dynEntPosea; // [esp+38h] [ebp-38h]
    GfxSceneDynBrush *sceneDynBrush; // [esp+3Ch] [ebp-34h]
    unsigned int dynEntIndex; // [esp+40h] [ebp-30h]
    unsigned int dynEntIndexa; // [esp+40h] [ebp-30h]
    unsigned int dynEntCount; // [esp+44h] [ebp-2Ch]
    unsigned int dynEntCounta; // [esp+44h] [ebp-2Ch]
    const DynEntityDef *dynEntDef; // [esp+48h] [ebp-28h]
    const DynEntityDef *dynEntDefa; // [esp+48h] [ebp-28h]
    unsigned __int8 *dynEntVisData[3]; // [esp+54h] [ebp-1Ch]
    GfxSceneDynModel *sceneDynModel; // [esp+60h] [ebp-10h]
    unsigned int viewIndex; // [esp+64h] [ebp-Ch]
    unsigned int visData; // [esp+68h] [ebp-8h]
    GfxBrushModel *bmodel; // [esp+6Ch] [ebp-4h]
    int savedregs; // [esp+70h] [ebp+0h] BYREF

    //Profile_Begin(407);
    for (viewIndex = 0; viewIndex < 3; ++viewIndex)
        dynEntVisData[viewIndex] = rgp.world->dpvsDyn.dynEntVisData[0][viewIndex];
    dynEntCount = rgp.world->dpvsDyn.dynEntClientCount[0];
    for (dynEntIndex = 0; dynEntIndex < dynEntCount; ++dynEntIndex)
    {
        visData = dynEntVisData[2][dynEntIndex] | dynEntVisData[1][dynEntIndex] | dynEntVisData[0][dynEntIndex];
        if ((visData & 1) != 0 || R_IsDynEntVisibleToAnyShadowedPrimaryLight(viewInfo, dynEntIndex, DYNENT_DRAW_MODEL))
        {
            dynEntPose = DynEnt_GetClientPose(dynEntIndex, DYNENT_DRAW_MODEL);
            dynEntDef = DynEnt_GetEntityDef(dynEntIndex, DYNENT_DRAW_MODEL);
            if (!dynEntDef->xModel)
                MyAssertHandler(".\\r_dpvs.cpp", 1696, 0, "%s", "dynEntDef->xModel");
            sceneDynModel = &rgp.world->sceneDynModel[scene.sceneDynModelCount];
            if (R_SkinXModel(&sceneDynModel->info, dynEntDef->xModel, 0, &dynEntPose->pose, 1.0, 0))
            {
                sceneDynModel->dynEntId = dynEntIndex;
                ++scene.sceneDynModelCount;
            }
            else
            {
                dynEntVisData[0][dynEntIndex] = 0;
                dynEntVisData[1][dynEntIndex] = 0;
                dynEntVisData[2][dynEntIndex] = 0;
            }
        }
    }
    if (!rg.drawXModels)
        scene.sceneDynModelCount = 0;
    for (viewIndex = 0; viewIndex < 3; ++viewIndex)
        dynEntVisData[viewIndex] = rgp.world->dpvsDyn.dynEntVisData[1][viewIndex];
    dynEntCounta = rgp.world->dpvsDyn.dynEntClientCount[1];
    for (dynEntIndexa = 0; dynEntIndexa < dynEntCounta; ++dynEntIndexa)
    {
        visData = dynEntVisData[2][dynEntIndexa] | dynEntVisData[1][dynEntIndexa] | dynEntVisData[0][dynEntIndexa];
        if ((visData & 1) != 0)
        {
            dynEntPosea = DynEnt_GetClientPose(dynEntIndexa, DYNENT_DRAW_BRUSH);
            dynEntDefa = DynEnt_GetEntityDef(dynEntIndexa, DYNENT_DRAW_BRUSH);
            if (dynEntDefa->xModel)
                MyAssertHandler(".\\r_dpvs.cpp", 1731, 0, "%s", "!dynEntDef->xModel");
            if (!dynEntDefa->brushModel)
                MyAssertHandler(".\\r_dpvs.cpp", 1732, 0, "%s", "dynEntDef->brushModel");
            bmodel = R_GetBrushModel(dynEntDefa->brushModel);
            if (bmodel->surfaceCount)
            {
                sceneDynBrush = &rgp.world->sceneDynBrush[scene.sceneDynBrushCount];
                if (R_DrawBModel((BModelDrawInfo *)sceneDynBrush, bmodel, &dynEntPosea->pose))
                {
                    sceneDynBrush->dynEntId = dynEntIndexa;
                    ++scene.sceneDynBrushCount;
                }
                else
                {
                    dynEntVisData[0][dynEntIndexa] = 0;
                    dynEntVisData[1][dynEntIndexa] = 0;
                    dynEntVisData[2][dynEntIndexa] = 0;
                }
            }
        }
    }
    if (!rg.drawBModels)
        scene.sceneDynBrushCount = 0;
    //Profile_EndInternal(0);
}

void __cdecl R_UnfilterEntFromCells(unsigned int localClientNum, unsigned int entnum)
{
    unsigned int cellIndex; // [esp+0h] [ebp-18h]
    unsigned int invBit; // [esp+4h] [ebp-14h]
    unsigned int offset; // [esp+8h] [ebp-10h]
    unsigned int *entCellBits; // [esp+Ch] [ebp-Ch]
    unsigned int cellCount; // [esp+10h] [ebp-8h]
    unsigned int cellCounta; // [esp+10h] [ebp-8h]
    unsigned int wordIndex; // [esp+14h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 1772, 0, "%s", "Sys_IsMainThread()");
    if (!rgp.world)
        MyAssertHandler(".\\r_dpvs.cpp", 1773, 0, "%s", "rgp.world");
    if (entnum == gfxCfg.entnumNone)
        MyAssertHandler(".\\r_dpvs.cpp", 1774, 0, "%s", "entnum != gfxCfg.entnumNone");
    if (gfxCfg.entCount * gfxCfg.maxClientViews > 0x1000)
        MyAssertHandler(".\\r_dpvs.cpp", 1776, 0, "%s", "gfxCfg.maxClientViews * gfxCfg.entCount <= MAX_TOTAL_ENT_COUNT");
    cellCount = rgp.world->dpvsPlanes.cellCount;
    if ((gfxCfg.entCount & 0x1F) != 0)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            1778,
            0,
            "%s\n\t(gfxCfg.entCount) = %i",
            "(!(gfxCfg.entCount & 31))",
            gfxCfg.entCount);
    offset = localClientNum * (gfxCfg.entCount >> 5);
    if (offset >= 0x80)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            1781,
            0,
            "offset doesn't index MAX_TOTAL_ENT_COUNT >> 5\n\t%i not in [0, %i)",
            offset,
            128);
    entCellBits = &rgp.world->dpvsPlanes.sceneEntCellBits[offset];
    wordIndex = entnum >> 5;
    invBit = ~(0x80000000 >> (entnum & 0x1F));
    dpvsGlob.entVisBits[localClientNum][entnum >> 5] &= invBit;
    cellCounta = 2 * cellCount;
    for (cellIndex = 0; cellIndex < cellCounta; ++cellIndex)
    {
        entCellBits[wordIndex] &= invBit;
        wordIndex += 128;
    }
}

void __cdecl R_UnfilterDynEntFromCells(unsigned int dynEntId, DynEntityDrawType drawType)
{
    unsigned int cellIndex; // [esp+0h] [ebp-18h]
    unsigned int cellCount; // [esp+8h] [ebp-10h]
    unsigned int dynEntClientWordCount; // [esp+Ch] [ebp-Ch]
    unsigned int *dynEntCellBits; // [esp+10h] [ebp-8h]
    unsigned int wordIndex; // [esp+14h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 1805, 0, "%s", "Sys_IsMainThread()");
    dynEntCellBits = rgp.world->dpvsDyn.dynEntCellBits[drawType];
    dynEntClientWordCount = rgp.world->dpvsDyn.dynEntClientWordCount[drawType];
    cellCount = rgp.world->dpvsPlanes.cellCount;
    wordIndex = dynEntId >> 5;
    for (cellIndex = 0; cellIndex < cellCount; ++cellIndex)
    {
        dynEntCellBits[wordIndex] &= ~(0x80000000 >> (dynEntId & 0x1F));
        wordIndex += dynEntClientWordCount;
    }
}

void __cdecl R_FilterXModelIntoScene(
    const XModel *model,
    const GfxScaledPlacement *placement,
    unsigned __int16 renderFxFlags,
    unsigned __int16 *cachedLightingHandle)
{
    const char *v4; // eax
    int v5; // [esp+38h] [ebp-3Ch]
    int frustumPlaneCount; // [esp+3Ch] [ebp-38h]
    float *a; // [esp+40h] [ebp-34h]
    int v8; // [esp+44h] [ebp-30h]
    GfxSceneModel *sceneModel; // [esp+4Ch] [ebp-28h]
    float radius; // [esp+54h] [ebp-20h]
    DpvsView *views; // [esp+58h] [ebp-1Ch]
    GfxEntity *gfxEnt; // [esp+5Ch] [ebp-18h]
    unsigned int sceneEntIndex; // [esp+60h] [ebp-14h]
    unsigned int gfxEntIndex; // [esp+64h] [ebp-10h]
    unsigned int cullCount; // [esp+68h] [ebp-Ch]
    unsigned __int8 sceneEntVisData[4]; // [esp+6Ch] [ebp-8h]
    unsigned int viewIndex; // [esp+70h] [ebp-4h]

    if (!model)
        MyAssertHandler(".\\r_dpvs.cpp", 2059, 0, "%s", "model");
    if (placement->scale <= 0.0)
        MyAssertHandler(".\\r_dpvs.cpp", 2060, 0, "%s", "placement->scale > 0");
    if (!Vec4IsNormalized(placement->base.quat))
    {
        v4 = va(
            "%g %g %g %g",
            placement->base.quat[0],
            placement->base.quat[1],
            placement->base.quat[2],
            placement->base.quat[3]);
        MyAssertHandler(".\\r_dpvs.cpp", 2061, 0, "%s\n\t%s", "Vec4IsNormalized( placement->base.quat )", v4);
    }
    radius = XModelGetRadius(model) * placement->scale;
    cullCount = 0;
    views = dpvsGlob.views[scene.dpvs.localClientNum];
    for (viewIndex = 0; viewIndex < 3; ++viewIndex)
    {
        frustumPlaneCount = views[viewIndex].frustumPlaneCount;
        v8 = 0;
        a = views[viewIndex].frustumPlanes[0].coeffs;
        while (v8 < frustumPlaneCount)
        {
            if (Vec3Dot(a, placement->base.origin) + a[3] + radius <= 0.0)
            {
                v5 = 1;
                goto LABEL_16;
            }
            ++v8;
            a += 5;
        }
        v5 = 0;
    LABEL_16:
        if (v5)
        {
            ++cullCount;
            sceneEntVisData[viewIndex] = 2;
        }
        else
        {
            sceneEntVisData[viewIndex] = 1;
        }
    }
    if (cullCount != 3 && r_drawXModels->current.enabled)
    {
        if (renderFxFlags)
        {
            gfxEntIndex = InterlockedExchangeAdd(&frontEndDataOut->gfxEntCount, 1);
            if (gfxEntIndex >= 0x80)
            {
                frontEndDataOut->gfxEntCount = 128;
                R_WarnOncePerFrame(R_WARN_KNOWN_SPECIAL_MODELS, 128);
                return;
            }
            gfxEnt = &frontEndDataOut->gfxEnts[gfxEntIndex];
            frontEndDataOut->gfxEnts[gfxEntIndex].materialTime = 0.0;
            gfxEnt->renderFxFlags = renderFxFlags;
        }
        else
        {
            LOWORD(gfxEntIndex) = 0;
        }
        sceneEntIndex = R_AllocSceneModel();
        if (sceneEntIndex < 0x400)
        {
            sceneModel = &scene.sceneModel[sceneEntIndex];
            sceneModel->model = model;
            if (sceneModel->obj)
                MyAssertHandler(".\\r_dpvs.cpp", 2113, 1, "%s", "!sceneModel->obj");
            sceneModel->gfxEntIndex = gfxEntIndex;
            memcpy(&sceneModel->placement, placement, sizeof(sceneModel->placement));
            sceneModel->entnum = gfxCfg.entnumNone;
            sceneModel->cachedLightingHandle = cachedLightingHandle;
            sceneModel->radius = radius;
            sceneModel->lightingOrigin[0] = sceneModel->placement.base.origin[0];
            sceneModel->lightingOrigin[1] = sceneModel->placement.base.origin[1];
            sceneModel->lightingOrigin[2] = sceneModel->placement.base.origin[2];
            sceneModel->lightingOrigin[2] = sceneModel->lightingOrigin[2] + 4.0;
            for (viewIndex = 0; viewIndex < 3; ++viewIndex)
                scene.sceneModelVisData[viewIndex][sceneEntIndex] = sceneEntVisData[viewIndex];
        }
    }
}

void __cdecl R_FilterDObjIntoCells(unsigned int localClientNum, unsigned int entnum, float *origin, float radius)
{
    float s; // [esp+0h] [ebp-30h]
    float mins[3]; // [esp+8h] [ebp-28h] BYREF
    FilterEntInfo entInfo; // [esp+14h] [ebp-1Ch] BYREF
    float maxs[3]; // [esp+24h] [ebp-Ch] BYREF

    if (entnum == gfxCfg.entnumNone)
        MyAssertHandler(".\\r_dpvs.cpp", 2136, 0, "%s", "entnum != gfxCfg.entnumNone");
    if (localClientNum >= gfxCfg.maxClientViews)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            2137,
            0,
            "localClientNum doesn't index gfxCfg.maxClientViews\n\t%i not in [0, %i)",
            localClientNum,
            gfxCfg.maxClientViews);
    R_UnfilterEntFromCells(localClientNum, entnum);
    s = -radius;
    Vec3AddScalar(origin, s, mins);
    Vec3AddScalar(origin, radius, maxs);
    entInfo.localClientNum = localClientNum;
    entInfo.entnum = entnum;
    entInfo.info.radius = radius;
    entInfo.cellOffset = 0;
    R_FilterEntIntoCells_r(&entInfo, (mnode_t *)rgp.world->dpvsPlanes.nodes, mins, maxs);
}

void __cdecl R_FilterEntIntoCells_r(FilterEntInfo *entInfo, mnode_t *node, const float *mins, const float *maxs)
{
    float localmaxs[3]; // [esp+0h] [ebp-50h]
    float dist; // [esp+Ch] [ebp-44h]
    float localmins[3]; // [esp+10h] [ebp-40h] BYREF
    unsigned int type; // [esp+1Ch] [ebp-34h]
    int side; // [esp+20h] [ebp-30h]
    cplane_s *plane; // [esp+24h] [ebp-2Ch]
    int cellIndex; // [esp+28h] [ebp-28h]
    float mins2[3]; // [esp+2Ch] [ebp-24h] BYREF
    int cellCount; // [esp+38h] [ebp-18h]
    float maxs2[3]; // [esp+3Ch] [ebp-14h] BYREF
    mnode_t *rightNode; // [esp+48h] [ebp-8h]
    int planeIndex; // [esp+4Ch] [ebp-4h]

    cellCount = rgp.world->dpvsPlanes.cellCount + 1;
    mins2[0] = *mins;
    mins2[1] = mins[1];
    mins2[2] = mins[2];
    maxs2[0] = *maxs;
    maxs2[1] = maxs[1];
    maxs2[2] = maxs[2];
    while (1)
    {
        cellIndex = node->cellIndex;
        planeIndex = cellIndex - cellCount;
        if (cellIndex - cellCount < 0)
            break;
        plane = &rgp.world->dpvsPlanes.planes[planeIndex];
        side = BoxOnPlaneSide(mins2, maxs2, plane);
        if (side == 3)
        {
            type = plane->type;
            rightNode = (mnode_t *)((char *)node + 2 * node->rightChildOffset);
            if (type >= 3)
            {
                R_FilterEntIntoCells_r(entInfo, node + 1, mins2, maxs2);
            }
            else
            {
                dist = plane->dist;
                localmins[0] = mins2[0];
                localmins[1] = mins2[1];
                localmins[2] = mins2[2];
                localmins[type] = dist;
                localmaxs[0] = maxs2[0];
                localmaxs[1] = maxs2[1];
                localmaxs[2] = maxs2[2];
                localmaxs[type] = dist;
                if (BoxOnPlaneSide(localmins, maxs2, plane) != 1)
                    MyAssertHandler(".\\r_dpvs.cpp", 1933, 0, "%s", "BoxOnPlaneSide( localmins, maxs2, plane ) == BOXSIDE_FRONT");
                if (maxs2[type] > (double)dist)
                    R_FilterEntIntoCells_r(entInfo, node + 1, localmins, maxs2);
                maxs2[0] = localmaxs[0];
                maxs2[1] = localmaxs[1];
                maxs2[2] = localmaxs[2];
            }
            node = rightNode;
        }
        else
        {
            if (side != 1 && side != 2)
                MyAssertHandler(".\\r_dpvs.cpp", 1953, 0, "%s", "(side == BOXSIDE_FRONT) || (side == BOXSIDE_BACK)");
            node = (mnode_t *)((char *)node + 2 * (side - 1) * (node->rightChildOffset - 2) + 4);
        }
    }
    if (cellIndex)
        R_AddEntToCell(entInfo, cellIndex - 1);
}

void __cdecl R_AddEntToCell(FilterEntInfo *entInfo, unsigned int cellIndex)
{
    unsigned int bit; // [esp+0h] [ebp-1Ch]
    unsigned int localClientNum; // [esp+4h] [ebp-18h]
    unsigned int offset; // [esp+8h] [ebp-14h]
    unsigned int *entCellBits; // [esp+Ch] [ebp-10h]
    unsigned int entnum; // [esp+14h] [ebp-8h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 1837, 0, "%s", "Sys_IsMainThread()");
    localClientNum = entInfo->localClientNum;
    if (entInfo->localClientNum >= gfxCfg.maxClientViews)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            1840,
            0,
            "localClientNum doesn't index gfxCfg.maxClientViews\n\t%i not in [0, %i)",
            localClientNum,
            gfxCfg.maxClientViews);
    entnum = entInfo->entnum;
    if (gfxCfg.entCount * gfxCfg.maxClientViews > 0x1000)
        MyAssertHandler(".\\r_dpvs.cpp", 1844, 0, "%s", "gfxCfg.maxClientViews * gfxCfg.entCount <= MAX_TOTAL_ENT_COUNT");
    if ((gfxCfg.entCount & 7) != 0)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            1846,
            0,
            "%s\n\t(gfxCfg.entCount) = %i",
            "(!(gfxCfg.entCount & 7))",
            gfxCfg.entCount);
    offset = localClientNum * (gfxCfg.entCount >> 5);
    if (offset >= 0x80)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            1849,
            0,
            "offset doesn't index MAX_TOTAL_ENT_COUNT >> 5\n\t%i not in [0, %i)",
            offset,
            128);
    entCellBits = &rgp.world->dpvsPlanes.sceneEntCellBits[128 * entInfo->cellOffset + 128 * cellIndex + offset];
    bit = 0x80000000 >> (entnum & 0x1F);
    entCellBits[entnum >> 5] |= bit;
    dpvsGlob.entVisBits[localClientNum][entnum >> 5] |= bit;
    scene.dpvs.entInfo[localClientNum][entnum].bmodel = entInfo->info.bmodel;
}

void __cdecl R_FilterBModelIntoCells(unsigned int localClientNum, unsigned int entnum, GfxBrushModel *bmodel)
{
    FilterEntInfo entInfo; // [esp+0h] [ebp-10h] BYREF

    if (entnum == gfxCfg.entnumNone)
        MyAssertHandler(".\\r_dpvs.cpp", 2157, 0, "%s", "entnum != gfxCfg.entnumNone");
    if (localClientNum >= gfxCfg.maxClientViews)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            2158,
            0,
            "localClientNum doesn't index gfxCfg.maxClientViews\n\t%i not in [0, %i)",
            localClientNum,
            gfxCfg.maxClientViews);
    R_UnfilterEntFromCells(localClientNum, entnum);
    entInfo.localClientNum = localClientNum;
    entInfo.entnum = entnum;
    entInfo.info.bmodel = bmodel;
    entInfo.cellOffset = rgp.world->dpvsPlanes.cellCount;
    R_FilterEntIntoCells_r(&entInfo, (mnode_t *)rgp.world->dpvsPlanes.nodes, bmodel->writable.mins, bmodel->writable.maxs);
}

void __cdecl R_FilterDynEntIntoCells(unsigned int dynEntId, DynEntityDrawType drawType, float *mins, float *maxs)
{
    R_UnfilterDynEntFromCells(dynEntId, drawType);
    R_FilterDynEntIntoCells_r((mnode_t *)rgp.world->dpvsPlanes.nodes, dynEntId, drawType, mins, maxs);
}

void __cdecl R_FilterDynEntIntoCells_r(
    mnode_t *node,
    unsigned int dynEntIndex,
    DynEntityDrawType drawType,
    const float *mins,
    const float *maxs)
{
    float localmaxs[3]; // [esp+0h] [ebp-50h]
    float dist; // [esp+Ch] [ebp-44h]
    float localmins[3]; // [esp+10h] [ebp-40h] BYREF
    unsigned int type; // [esp+1Ch] [ebp-34h]
    int side; // [esp+20h] [ebp-30h]
    cplane_s *plane; // [esp+24h] [ebp-2Ch]
    int cellIndex; // [esp+28h] [ebp-28h]
    float mins2[3]; // [esp+2Ch] [ebp-24h] BYREF
    int cellCount; // [esp+38h] [ebp-18h]
    float maxs2[3]; // [esp+3Ch] [ebp-14h] BYREF
    mnode_t *rightNode; // [esp+48h] [ebp-8h]
    int planeIndex; // [esp+4Ch] [ebp-4h]

    cellCount = rgp.world->dpvsPlanes.cellCount + 1;
    mins2[0] = *mins;
    mins2[1] = mins[1];
    mins2[2] = mins[2];
    maxs2[0] = *maxs;
    maxs2[1] = maxs[1];
    maxs2[2] = maxs[2];
    while (1)
    {
        cellIndex = node->cellIndex;
        planeIndex = cellIndex - cellCount;
        if (cellIndex - cellCount < 0)
            break;
        plane = &rgp.world->dpvsPlanes.planes[planeIndex];
        side = BoxOnPlaneSide(mins2, maxs2, plane);
        if (side == 3)
        {
            type = plane->type;
            rightNode = (mnode_t *)((char *)node + 2 * node->rightChildOffset);
            if (type >= 3)
            {
                R_FilterDynEntIntoCells_r(node + 1, dynEntIndex, drawType, mins2, maxs2);
            }
            else
            {
                dist = plane->dist;
                localmins[0] = mins2[0];
                localmins[1] = mins2[1];
                localmins[2] = mins2[2];
                localmins[type] = dist;
                localmaxs[0] = maxs2[0];
                localmaxs[1] = maxs2[1];
                localmaxs[2] = maxs2[2];
                localmaxs[type] = dist;
                if (BoxOnPlaneSide(localmins, maxs2, plane) != 1)
                    MyAssertHandler(".\\r_dpvs.cpp", 2018, 0, "%s", "BoxOnPlaneSide( localmins, maxs2, plane ) == BOXSIDE_FRONT");
                if (maxs2[type] > (double)dist)
                    R_FilterDynEntIntoCells_r(node + 1, dynEntIndex, drawType, localmins, maxs2);
                maxs2[0] = localmaxs[0];
                maxs2[1] = localmaxs[1];
                maxs2[2] = localmaxs[2];
            }
            node = rightNode;
        }
        else
        {
            if (side != 1 && side != 2)
                MyAssertHandler(".\\r_dpvs.cpp", 2038, 0, "%s", "(side == BOXSIDE_FRONT) || (side == BOXSIDE_BACK)");
            node = (mnode_t *)((char *)node + 2 * (side - 1) * (node->rightChildOffset - 2) + 4);
        }
    }
    if (cellIndex)
        R_AddDynEntToCell(cellIndex - 1, dynEntIndex, drawType);
}

void __cdecl R_AddDynEntToCell(unsigned int cellIndex, unsigned int dynEntIndex, DynEntityDrawType drawType)
{
    unsigned int wordIndex; // [esp+Ch] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 1867, 0, "%s", "Sys_IsMainThread()");
    wordIndex = rgp.world->dpvsDyn.dynEntClientWordCount[drawType] * cellIndex + (dynEntIndex >> 5);
    rgp.world->dpvsDyn.dynEntCellBits[drawType][wordIndex] |= 0x80000000 >> (dynEntIndex & 0x1F);
}

void __cdecl R_FilterEntitiesIntoCells(int cameraCellIndex)
{
    float s; // [esp+0h] [ebp-84h]
    unsigned int v2; // [esp+14h] [ebp-70h]
    int v3; // [esp+18h] [ebp-6Ch]
    DpvsPlane *v4; // [esp+1Ch] [ebp-68h]
    int v5; // [esp+20h] [ebp-64h]
    int v6; // [esp+24h] [ebp-60h]
    int frustumPlaneCount; // [esp+28h] [ebp-5Ch]
    float radius; // [esp+2Ch] [ebp-58h]
    DpvsPlane *a; // [esp+30h] [ebp-54h]
    int v10; // [esp+34h] [ebp-50h]
    int v11; // [esp+38h] [ebp-4Ch]
    DpvsPlane *frustumPlanes; // [esp+40h] [ebp-44h]
    int v13; // [esp+44h] [ebp-40h]
    float mins[3]; // [esp+48h] [ebp-3Ch] BYREF
    float maxs[3]; // [esp+54h] [ebp-30h] BYREF
    GfxSceneModel *sceneModel; // [esp+60h] [ebp-24h]
    DpvsView *dpvsView; // [esp+64h] [ebp-20h]
    DpvsView *views; // [esp+68h] [ebp-1Ch]
    int sceneEntIndex; // [esp+6Ch] [ebp-18h]
    GfxSceneEntity *sceneEnt; // [esp+70h] [ebp-14h]
    GfxSceneBrush *sceneBrush; // [esp+74h] [ebp-10h]
    unsigned int entnum; // [esp+78h] [ebp-Ch]
    unsigned int viewIndex; // [esp+7Ch] [ebp-8h]
    const GfxBrushModel *bmodel; // [esp+80h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 2190, 0, "%s", "Sys_IsMainThread()");
    if (cameraCellIndex < 0)
        v2 = 0;
    else
        v2 = cameraCellIndex;
    dpvsGlob.cameraCellIndex = v2;
    views = dpvsGlob.views[scene.dpvs.localClientNum];
    for (sceneEntIndex = 0; sceneEntIndex < scene.sceneDObjCount; ++sceneEntIndex)
    {
        sceneEnt = &scene.sceneDObj[sceneEntIndex];
        if (sceneEnt->cull.state != 4)
        {
            entnum = sceneEnt->entnum;
            for (viewIndex = 0; viewIndex < 3; ++viewIndex)
            {
                dpvsView = &views[viewIndex];
                v13 = 0;
                frustumPlanes = dpvsView->frustumPlanes;
                while (v13 < dpvsView->frustumPlaneCount)
                {
                    if (*(float *)((char *)sceneEnt->cull.mins + frustumPlanes->side[0]) * frustumPlanes->coeffs[0]
                        + frustumPlanes->coeffs[3]
                        + *(float *)((char *)sceneEnt->cull.mins + frustumPlanes->side[1]) * frustumPlanes->coeffs[1]
                        + *(float *)((char *)sceneEnt->cull.mins + frustumPlanes->side[2]) * frustumPlanes->coeffs[2] <= 0.0)
                    {
                        v11 = 1;
                        goto LABEL_19;
                    }
                    ++v13;
                    ++frustumPlanes;
                }
                v11 = 0;
            LABEL_19:
                if (v11)
                    scene.dpvs.entVisData[viewIndex][entnum] = 2;
            }
            if (r_showCullXModels->current.enabled)
                R_AddDebugBox(&frontEndDataOut->debugGlobals, sceneEnt->cull.mins, sceneEnt->cull.maxs, colorCyan);
        }
    }
    for (sceneEntIndex = 0; sceneEntIndex < scene.sceneModelCount; ++sceneEntIndex)
    {
        sceneModel = &scene.sceneModel[sceneEntIndex];
        entnum = sceneModel->entnum;
        for (viewIndex = 0; viewIndex < 3; ++viewIndex)
        {
            dpvsView = &views[viewIndex];
            frustumPlaneCount = dpvsView->frustumPlaneCount;
            radius = sceneModel->radius;
            v10 = 0;
            a = dpvsView->frustumPlanes;
            while (v10 < frustumPlaneCount)
            {
                if (Vec3Dot(a->coeffs, sceneModel->placement.base.origin) + a->coeffs[3] + radius <= 0.0)
                {
                    v6 = 1;
                    goto LABEL_35;
                }
                ++v10;
                ++a;
            }
            v6 = 0;
        LABEL_35:
            if (v6)
                scene.dpvs.entVisData[viewIndex][entnum] = 2;
        }
        if (r_showCullXModels->current.enabled)
        {
            s = -sceneModel->radius;
            Vec3AddScalar(sceneModel->placement.base.origin, s, mins);
            Vec3AddScalar(sceneModel->placement.base.origin, sceneModel->radius, maxs);
            R_AddDebugBox(&frontEndDataOut->debugGlobals, mins, maxs, colorCyan);
        }
    }
    for (sceneEntIndex = 0; sceneEntIndex < scene.sceneBrushCount; ++sceneEntIndex)
    {
        sceneBrush = &scene.sceneBrush[sceneEntIndex];
        entnum = sceneBrush->entnum;
        bmodel = sceneBrush->bmodel;
        for (viewIndex = 0; viewIndex < 3; ++viewIndex)
        {
            dpvsView = &views[viewIndex];
            v5 = 0;
            v4 = dpvsView->frustumPlanes;
            while (v5 < dpvsView->frustumPlaneCount)
            {
                if (*(float *)((char *)bmodel->writable.mins + v4->side[0]) * v4->coeffs[0]
                    + v4->coeffs[3]
                    + *(float *)((char *)bmodel->writable.mins + v4->side[1]) * v4->coeffs[1]
                    + *(float *)((char *)bmodel->writable.mins + v4->side[2]) * v4->coeffs[2] <= 0.0)
                {
                    v3 = 1;
                    goto LABEL_51;
                }
                ++v5;
                ++v4;
            }
            v3 = 0;
        LABEL_51:
            if (v3)
                scene.dpvs.entVisData[viewIndex][entnum] = 2;
        }
    }
}

unsigned int __cdecl R_SetVisData(unsigned int viewIndex)
{
    unsigned int oldViewIndex; // [esp+4h] [ebp-8h]
    unsigned int drawType; // [esp+8h] [ebp-4h]

    oldViewIndex = g_viewIndex; // *(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 12);
    g_viewIndex = oldViewIndex; // *(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 12) = viewIndex;
    for (drawType = 0; drawType < 2; ++drawType)
        g_dynEntVisData[drawType] = rgp.world->dpvsDyn.dynEntVisData[drawType][viewIndex]; // *(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 4 * drawType + 16) =
    g_dpvsView = &dpvsGlob.views[scene.dpvs.localClientNum][viewIndex]; // *(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 8)
    return oldViewIndex;
}

void __cdecl R_AddCellDynBrushSurfacesInFrustumCmd(const DpvsDynamicCellCmd *data)
{
    unsigned int oldViewIndex; // [esp+0h] [ebp-8h]

    oldViewIndex = R_SetVisData(data->viewIndex);
    if (r_drawDynEnts->current.enabled)
        R_CullDynBrushInCell(data->cellIndex, data->planes, data->planeCount);
    R_SetVisData(oldViewIndex);
}

void __cdecl R_CullDynBrushInCell(unsigned int cellIndex, const DpvsPlane *planes, int planeCount)
{
    unsigned long v4; // eax
    int v5; // [esp+4h] [ebp-34h]
    const DpvsPlane *v6; // [esp+8h] [ebp-30h]
    int v7; // [esp+Ch] [ebp-2Ch]
    unsigned int dynEntIndex; // [esp+14h] [ebp-24h]
    const DynEntityDef *dynEntDef; // [esp+18h] [ebp-20h]
    unsigned int bits; // [esp+1Ch] [ebp-1Ch]
    unsigned int dynEntClientWordCount; // [esp+20h] [ebp-18h]
    unsigned int indexLow; // [esp+24h] [ebp-14h]
    unsigned __int8 *dynEntVisData; // [esp+28h] [ebp-10h]
    unsigned int *dynEntCellBits; // [esp+2Ch] [ebp-Ch]
    unsigned int wordIndex; // [esp+30h] [ebp-8h]
    const GfxBrushModel *bmodel; // [esp+34h] [ebp-4h]

    if (cellIndex >= rgp.world->dpvsPlanes.cellCount)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            1289,
            0,
            "cellIndex doesn't index rgp.world->dpvsPlanes.cellCount\n\t%i not in [0, %i)",
            cellIndex,
            rgp.world->dpvsPlanes.cellCount);
    dynEntVisData = g_dynEntVisData[1];
    dynEntClientWordCount = rgp.world->dpvsDyn.dynEntClientWordCount[1];
    dynEntCellBits = &rgp.world->dpvsDyn.dynEntCellBits[1][dynEntClientWordCount * cellIndex];
    for (wordIndex = 0; wordIndex < dynEntClientWordCount; ++wordIndex)
    {
        bits = dynEntCellBits[wordIndex];
        while (1)
        {
            if (!_BitScanReverse(&v4, bits))
                v4 = 0x3F;
            indexLow = v4 ^ 0x1F;
            if ((v4 ^ 0x1Fu) >= 0x20)
                break;
            dynEntIndex = indexLow + 32 * wordIndex;
            if (((0x80000000 >> indexLow) & bits) == 0)
                MyAssertHandler(".\\r_dpvs.cpp", 1308, 0, "%s", "bits & bit");
            bits &= ~(0x80000000 >> indexLow);
            if (!dynEntVisData[dynEntIndex])
            {
                dynEntDef = DynEnt_GetEntityDef(dynEntIndex, DYNENT_DRAW_BRUSH);
                if (dynEntDef->xModel)
                    MyAssertHandler(".\\r_dpvs.cpp", 1315, 0, "%s", "!dynEntDef->xModel");
                if (!dynEntDef->brushModel)
                    MyAssertHandler(".\\r_dpvs.cpp", 1316, 0, "%s", "dynEntDef->brushModel");
                bmodel = R_GetBrushModel(dynEntDef->brushModel);
                v7 = 0;
                v6 = planes;
                while (v7 < planeCount)
                {
                    if (*(float *)((char *)bmodel->writable.mins + v6->side[0]) * v6->coeffs[0]
                        + v6->coeffs[3]
                        + *(float *)((char *)bmodel->writable.mins + v6->side[1]) * v6->coeffs[1]
                        + *(float *)((char *)bmodel->writable.mins + v6->side[2]) * v6->coeffs[2] <= 0.0)
                    {
                        v5 = 1;
                        goto LABEL_22;
                    }
                    ++v7;
                    ++v6;
                }
                v5 = 0;
            LABEL_22:
                if (!v5)
                    dynEntVisData[dynEntIndex] = 1;
            }
        }
    }
}

void __cdecl R_GenerateShadowMapCasterCells()
{
    GfxLight *sunLight; // edx
    GfxCell *cell; // [esp+4h] [ebp-Ch]
    int cellIndex; // [esp+8h] [ebp-8h]
    unsigned int cellCasterBitsCount; // [esp+Ch] [ebp-4h]

    if (!rgp.world->sunLight)
        MyAssertHandler(".\\r_dpvs.cpp", 2925, 0, "%s", "rgp.world->sunLight");
    cellCasterBitsCount = (rgp.world->dpvsPlanes.cellCount + 31) >> 5;
    memset((unsigned __int8 *)rgp.world->cellCasterBits, 0, 4 * cellCasterBitsCount * rgp.world->dpvsPlanes.cellCount);
    if (rgp.world->sunPrimaryLightIndex)
    {
        if (Vec3LengthSq(rgp.world->sunLight->dir) == 0.0)
            MyAssertHandler(".\\r_dpvs.cpp", 2933, 0, "%s", "Vec3LengthSq( rgp.world->sunLight->dir )");
        sunLight = rgp.world->sunLight;
        dpvsGlob.viewOrg[0] = -sunLight->dir[0];
        dpvsGlob.viewOrg[1] = -sunLight->dir[1];
        dpvsGlob.viewOrg[2] = -sunLight->dir[2];
        dpvsGlob.viewOrg[3] = 0.0;
        dpvsGlob.viewOrgIsDir = 1;
        dpvsGlob.farPlane = 0;
        dpvsGlob.nearPlane = 0;
        for (cellIndex = 0; cellIndex < rgp.world->dpvsPlanes.cellCount; ++cellIndex)
        {
            cell = &rgp.world->cells[cellIndex];
            dpvsGlob.cellBits = &rgp.world->cellCasterBits[cellCasterBitsCount * cellIndex];
            R_VisitPortalsNoFrustum(cell);
        }
    }
}

void __cdecl R_VisitPortalsNoFrustum(const GfxCell *cell)
{
    float scale; // [esp+4h] [ebp-D80h]
    GfxHullPointsPool(*hullPointsPoolArray)[256]; // [esp+5Ch] [ebp-D28h]
    int childPlaneCount; // [esp+60h] [ebp-D24h]
    GfxPortal *portal; // [esp+64h] [ebp-D20h]
    int queueIndex; // [esp+68h] [ebp-D1Ch]
    float portalVerts[64][3]; // [esp+6Ch] [ebp-D18h] BYREF
    float hullOrigin[3]; // [esp+36Ch] [ebp-A18h] BYREF
    unsigned int vertIndex; // [esp+378h] [ebp-A0Ch]
    PortalHeapNode portalQueue[256]; // [esp+37Ch] [ebp-A08h] BYREF
    float hull[64][2]; // [esp+B7Ch] [ebp-208h] BYREF
    unsigned int hullPointCount; // [esp+D80h] [ebp-4h]

    LargeLocal hullPointsPoolArray_large_local(0x20000);
    //LargeLocal::LargeLocal(&hullPointsPoolArray_large_local, 0x20000);
    //hullPointsPoolArray = (GfxHullPointsPool(*)[256])LargeLocal::GetBuf(&hullPointsPoolArray_large_local);
    hullPointsPoolArray = (GfxHullPointsPool(*)[256])hullPointsPoolArray_large_local.GetBuf();
    //Profile_Begin(188);
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 2879, 0, "%s", "Sys_IsMainThread()");
    for (queueIndex = 0; queueIndex < 255; ++queueIndex)
        (*hullPointsPoolArray)[queueIndex].nextFree = &(*hullPointsPoolArray)[queueIndex + 1];
    (*hullPointsPoolArray)[queueIndex].nextFree = 0;
    dpvsGlob.nextFreeHullPoints = (GfxHullPointsPool *)hullPointsPoolArray;
    dpvsGlob.portalQueue = portalQueue;
    dpvsGlob.queuedCount = 0;
    R_VisitPortalsForCellNoFrustum(cell, 0, 0, 0, 0, 0, 0);
    while (dpvsGlob.queuedCount)
    {
        portal = R_NextQueuedPortal();
        if (!portal)
            MyAssertHandler(".\\r_dpvs.cpp", 2892, 0, "%s", "portal");
        //Profile_Begin(84);
        hullPointCount = Com_ConvexHull(portal->writable.hullPoints, portal->writable.hullPointCount, hull);
        //Profile_EndInternal(0);
        R_FreeHullPoints((GfxHullPointsPool *)portal->writable.hullPoints);
        portal->writable.hullPoints = 0;
        if (hullPointCount)
        {
            scale = -portal->plane.coeffs[3];
            Vec3Scale(portal->plane.coeffs, scale, hullOrigin);
            for (vertIndex = 0; vertIndex < hullPointCount; ++vertIndex)
            {
                Vec3Mad(hullOrigin, hull[vertIndex][0], portal->hullAxis[0], portalVerts[vertIndex]);
                Vec3Mad(portalVerts[vertIndex], hull[vertIndex][1], portal->hullAxis[1], portalVerts[vertIndex]);
            }
            childPlaneCount = R_PortalClipPlanesNoFrustum(dpvsGlob.childPlanes, hullPointCount, portalVerts);
            if (childPlaneCount > 16)
                MyAssertHandler(".\\r_dpvs.cpp", 2911, 0, "%s", "childPlaneCount <= DPVS_PORTAL_MAX_PLANES");
            R_VisitPortalsForCellNoFrustum(
                portal->cell,
                portal,
                &portal->plane,
                dpvsGlob.childPlanes,
                childPlaneCount,
                0,
                portal->writable.recursionDepth + 1);
        }
    }
    //Profile_EndInternal(0);
    //LargeLocal::~LargeLocal(&hullPointsPoolArray_large_local);
}

unsigned int __cdecl R_PortalClipPlanesNoFrustum(
    DpvsPlane *planes,
    unsigned int vertexCount,
    const float (*winding)[3])
{
    DpvsPlane *a; // [esp+0h] [ebp-620h]
    DpvsPlane *v5; // [esp+8h] [ebp-618h]
    float *v6; // [esp+Ch] [ebp-614h]
    float normals[128][3]; // [esp+10h] [ebp-610h] BYREF
    unsigned int windingVertIndex; // [esp+614h] [ebp-Ch]
    unsigned int planeCount; // [esp+618h] [ebp-8h]
    bool useNormalPlanes; // [esp+61Fh] [ebp-1h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 531, 0, "%s", "Sys_IsMainThread()");
    if (vertexCount < 3)
        MyAssertHandler(".\\r_dpvs.cpp", 532, 0, "%s\n\t(vertexCount) = %i", "(vertexCount >= 3)", vertexCount);
    useNormalPlanes = vertexCount <= 0xA;
    R_GetSidePlaneNormals(winding, vertexCount, normals);
    planeCount = 0;
    if (useNormalPlanes)
    {
        for (windingVertIndex = 0; windingVertIndex < vertexCount; ++windingVertIndex)
        {
            if (Vec3LengthSq(normals[windingVertIndex]) != 0.0)
            {
                v5 = &planes[planeCount];
                v6 = normals[windingVertIndex];
                v5->coeffs[0] = *v6;
                v5->coeffs[1] = v6[1];
                v5->coeffs[2] = v6[2];
                a = &planes[planeCount];
                a->coeffs[3] = 0.001 - Vec3Dot(a->coeffs, &(*winding)[3 * windingVertIndex]);
                a->side[0] = COERCE_INT(a->coeffs[0]) <= 0 ? 0 : 0xC;
                a->side[1] = COERCE_INT(a->coeffs[1]) <= 0 ? 4 : 16;
                a->side[2] = COERCE_INT(a->coeffs[2]) <= 0 ? 8 : 20;
                ++planeCount;
            }
        }
    }
    return planeCount;
}

void __cdecl R_GetSidePlaneNormals(const float (*winding)[3], unsigned int vertexCount, float (*normals)[3])
{
    float *v3; // [esp+18h] [ebp-61Ch]
    float delta[388]; // [esp+1Ch] [ebp-618h] BYREF
    unsigned int vertexIndex; // [esp+62Ch] [ebp-8h]
    unsigned int vertexIndexNext; // [esp+630h] [ebp-4h]

    if (vertexCount >= 0x81)
        MyAssertHandler(".\\r_dpvs.cpp", 157, 0, "%s", "vertexCount < ARRAY_COUNT( delta )");
    if (dpvsGlob.viewOrgIsDir)
    {
        vertexIndex = vertexCount - 1;
        for (vertexIndexNext = 0; vertexIndexNext < vertexCount; ++vertexIndexNext)
        {
            Vec3Sub(&(*winding)[3 * vertexIndexNext], &(*winding)[3 * vertexIndex], delta);
            Vec3Cross(dpvsGlob.viewOrg, delta, &(*normals)[3 * vertexIndex]);
            Vec3Normalize(&(*normals)[3 * vertexIndex]);
            vertexIndex = vertexIndexNext;
        }
    }
    else
    {
        for (vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
            Vec3Sub(&(*winding)[3 * vertexIndex], dpvsGlob.viewOrg, &delta[3 * vertexIndex]);
        v3 = &delta[3 * vertexCount];
        *v3 = delta[0];
        v3[1] = delta[1];
        v3[2] = delta[2];
        for (vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
        {
            Vec3Cross(&delta[3 * vertexIndex + 3], &delta[3 * vertexIndex], &(*normals)[3 * vertexIndex]);
            Vec3Normalize(&(*normals)[3 * vertexIndex]);
        }
    }
}

GfxPortal *__cdecl R_NextQueuedPortal()
{
    float dist; // eax
    PortalHeapNode *portalQueue; // esi
    float v2; // eax
    PortalHeapNode *v3; // esi
    int heapIndex; // [esp+4h] [ebp-Ch]
    int chosenChildIndex; // [esp+8h] [ebp-8h]
    GfxPortal *portal; // [esp+Ch] [ebp-4h]

    if (dpvsGlob.queuedCount <= 0)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            2394,
            0,
            "%s\n\t(dpvsGlob.queuedCount) = %i",
            "(dpvsGlob.queuedCount > 0)",
            dpvsGlob.queuedCount);
    portal = dpvsGlob.portalQueue->portal;
    dpvsGlob.portalQueue->portal->writable.isQueued = 0;
    --dpvsGlob.queuedCount;
    for (heapIndex = 0; ; heapIndex = chosenChildIndex)
    {
        chosenChildIndex = 2 * heapIndex + 1;
        if (chosenChildIndex > dpvsGlob.queuedCount)
            break;
        if (chosenChildIndex < dpvsGlob.queuedCount
            && dpvsGlob.portalQueue[chosenChildIndex].dist >(double)dpvsGlob.portalQueue[chosenChildIndex + 1].dist)
        {
            chosenChildIndex = 2 * heapIndex + 2;
        }
        if (dpvsGlob.portalQueue[chosenChildIndex].dist >= (double)dpvsGlob.portalQueue[dpvsGlob.queuedCount].dist)
            break;
        dist = dpvsGlob.portalQueue[chosenChildIndex].dist;
        portalQueue = dpvsGlob.portalQueue;
        dpvsGlob.portalQueue[heapIndex].portal = dpvsGlob.portalQueue[chosenChildIndex].portal;
        portalQueue[heapIndex].dist = dist;
    }
    v2 = dpvsGlob.portalQueue[dpvsGlob.queuedCount].dist;
    v3 = dpvsGlob.portalQueue;
    dpvsGlob.portalQueue[heapIndex].portal = dpvsGlob.portalQueue[dpvsGlob.queuedCount].portal;
    v3[heapIndex].dist = v2;
    R_AssertValidQueue();
    return portal;
}

int R_AssertValidQueue()
{
    int result = 0; // eax
    int queueIndex; // [esp+4h] [ebp-4h]

    for (queueIndex = 1; queueIndex < dpvsGlob.queuedCount; ++queueIndex)
    {
        if (dpvsGlob.portalQueue[queueIndex].dist < (double)dpvsGlob.portalQueue[(queueIndex - 1) >> 1].dist)
            MyAssertHandler(
                ".\\r_dpvs.cpp",
                2347,
                0,
                "%s",
                "dpvsGlob.portalQueue[parentIndex].dist <= dpvsGlob.portalQueue[queueIndex].dist");
        result = queueIndex + 1;
    }
    return result;
}

void __cdecl R_VisitPortalsForCellNoFrustum(
    const GfxCell *cell,
    GfxPortal *parentPortal,
    const DpvsPlane *parentPlane,
    const DpvsPlane *planes,
    int planeCount,
    int frustumPlaneCount,
    signed int recursionDepth)
{
    unsigned __int8 v7; // [esp+0h] [ebp-10h]
    GfxPortal *portal; // [esp+8h] [ebp-8h]
    int portalIndex; // [esp+Ch] [ebp-4h]

    R_SetCellVisible(cell);
    R_SetAncestorListStatus(parentPortal, 1);
    for (portalIndex = 0; portalIndex < cell->portalCount; ++portalIndex)
    {
        portal = &cell->portals[portalIndex];
        if (!R_ShouldSkipPortal(portal, planes, planeCount)
            && R_ChopPortalAndAddHullPointsNoFrustum(portal, parentPlane, planes, planeCount))
        {
            if (portal->writable.isQueued)
            {
                if (portal->writable.recursionDepth < recursionDepth)
                    v7 = portal->writable.recursionDepth;
                else
                    v7 = recursionDepth;
                portal->writable.recursionDepth = v7;
                if (portal->writable.queuedParent != parentPortal)
                    portal->writable.queuedParent = 0;
            }
            else
            {
                portal->writable.recursionDepth = recursionDepth;
                portal->writable.queuedParent = parentPortal;
                R_EnqueuePortal(portal);
            }
        }
    }
    R_SetAncestorListStatus(parentPortal, 0);
}

void __cdecl R_EnqueuePortal(GfxPortal *portal)
{
    float v1; // edx
    PortalHeapNode *portalQueue; // esi
    int heapIndex; // [esp+4h] [ebp-Ch]
    float dist; // [esp+8h] [ebp-8h]
    int parentIndex; // [esp+Ch] [ebp-4h]

    if (!portal)
        MyAssertHandler(".\\r_dpvs.cpp", 2359, 0, "%s", "portal");
    if (portal->writable.isQueued)
        MyAssertHandler(".\\r_dpvs.cpp", 2360, 0, "%s", "!portal->writable.isQueued");
    if (!portal->writable.hullPoints)
        MyAssertHandler(".\\r_dpvs.cpp", 2361, 0, "%s", "portal->writable.hullPoints");
    if (portal->writable.hullPointCount < 3u)
        MyAssertHandler(".\\r_dpvs.cpp", 2362, 0, "%s", "portal->writable.hullPointCount >= 3");
    if (dpvsGlob.queuedCount >= 256)
        Com_Error(ERR_DROP, "More than %i queued portals", 256);
    portal->writable.isQueued = 1;
    dist = R_FurthestPointOnWinding(portal->vertices, portal->vertexCount, &dpvsGlob.viewPlane);
    for (heapIndex = dpvsGlob.queuedCount; ; heapIndex = (heapIndex - 1) >> 1)
    {
        parentIndex = (heapIndex - 1) >> 1;
        if (parentIndex < 0 || dist >= (double)dpvsGlob.portalQueue[parentIndex].dist)
            break;
        v1 = dpvsGlob.portalQueue[parentIndex].dist;
        portalQueue = dpvsGlob.portalQueue;
        dpvsGlob.portalQueue[heapIndex].portal = dpvsGlob.portalQueue[parentIndex].portal;
        portalQueue[heapIndex].dist = v1;
    }
    if (heapIndex < 0 || heapIndex > dpvsGlob.queuedCount)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            2379,
            1,
            "%s\n\t(heapIndex) = %i",
            "(heapIndex >= 0 && heapIndex <= dpvsGlob.queuedCount)",
            heapIndex);
    dpvsGlob.portalQueue[heapIndex].portal = portal;
    dpvsGlob.portalQueue[heapIndex].dist = dist;
    ++dpvsGlob.queuedCount;
    R_AssertValidQueue();
}

double __cdecl R_FurthestPointOnWinding(const float (*points)[3], int pointCount, const DpvsPlane *plane)
{
    float v4; // [esp+0h] [ebp-24h]
    float v5; // [esp+4h] [ebp-20h]
    float v6; // [esp+8h] [ebp-1Ch]
    float v7; // [esp+Ch] [ebp-18h]
    int pointIndex; // [esp+18h] [ebp-Ch]
    int pointIndexa; // [esp+18h] [ebp-Ch]
    float distMax; // [esp+20h] [ebp-4h]

    v7 = Vec3Dot(plane->coeffs, (const float *)points) + plane->coeffs[3];
    v6 = Vec3Dot(plane->coeffs, &(*points)[3 * pointCount - 3]) + plane->coeffs[3];
    if (v6 >= (double)v7)
    {
        distMax = v6;
        for (pointIndexa = pointCount - 2; pointIndexa > 0; --pointIndexa)
        {
            v4 = Vec3Dot(plane->coeffs, &(*points)[3 * pointIndexa]) + plane->coeffs[3];
            if (v4 < (double)distMax)
                break;
            distMax = v4;
        }
    }
    else
    {
        distMax = v7;
        for (pointIndex = 1; pointIndex < pointCount - 1; ++pointIndex)
        {
            v5 = Vec3Dot(plane->coeffs, &(*points)[3 * pointIndex]) + plane->coeffs[3];
            if (v5 < (double)distMax)
                break;
            distMax = v5;
        }
    }
    return distMax;
}

bool __cdecl R_ShouldSkipPortal(const GfxPortal *portal, const DpvsPlane *planes, int planeCount)
{
    float v4; // [esp+0h] [ebp-4h]

    if (portal->writable.isAncestor)
        return 1;
    v4 = Vec4Dot(portal->plane.coeffs, dpvsGlob.viewOrg);
    return v4 > 0.0 || R_PortalBehindAnyPlane(portal, planes, planeCount) != 0;
}

char __cdecl R_PortalBehindAnyPlane(const GfxPortal *portal, const DpvsPlane *planes, int planeCount)
{
    while (planeCount)
    {
        if (R_PortalBehindPlane(portal, planes))
            return 1;
        --planeCount;
        ++planes;
    }
    return 0;
}

char __cdecl R_PortalBehindPlane(const GfxPortal *portal, const DpvsPlane *plane)
{
    float v3; // [esp+0h] [ebp-Ch]
    int c; // [esp+4h] [ebp-8h]
    float *v; // [esp+8h] [ebp-4h]

    v = (float *)portal->vertices;
    for (c = portal->vertexCount; c; --c)
    {
        v3 = Vec3Dot(plane->coeffs, v) + plane->coeffs[3];
        if (v3 > 0.0)
            return 0;
        v += 3;
    }
    return 1;
}

char __cdecl R_ChopPortalAndAddHullPointsNoFrustum(
    GfxPortal *portal,
    const DpvsPlane *parentPlane,
    const DpvsPlane *planes,
    int planeCount)
{
    int vertCount; // [esp+0h] [ebp-C10h]
    int vertIndex; // [esp+4h] [ebp-C0Ch]
    float v[2][128][3]; // [esp+8h] [ebp-C08h] BYREF
    const float (*w)[3]; // [esp+C0Ch] [ebp-4h] BYREF

    if (parentPlane)
    {
        vertCount = R_ChopPortal(portal, parentPlane, planes, planeCount, v, &w);
        if (!vertCount)
            return 0;
    }
    else
    {
        vertCount = portal->vertexCount;
        if (!portal->vertexCount)
            MyAssertHandler(".\\r_dpvs.cpp", 2562, 0, "%s", "vertCount");
        w = portal->vertices;
    }
    for (vertIndex = 0; vertIndex < vertCount; ++vertIndex)
        R_AddVertToPortalHullPoints(portal, w[vertIndex]);
    return 1;
}

void __cdecl R_AddVertToPortalHullPoints(GfxPortal *portal, const float *v)
{
    float hull[64][2]; // [esp+14h] [ebp-208h] BYREF
    int hullPointCount; // [esp+218h] [ebp-4h]

    if (portal->writable.hullPoints)
    {
        if (portal->writable.hullPointCount == 64)
        {
            //Profile_Begin(84);
            hullPointCount = Com_ConvexHull(portal->writable.hullPoints, portal->writable.hullPointCount, hull);
            //Profile_EndInternal(0);
            if (hullPointCount == 64)
                Com_Error(ERR_DROP, "More than %i points on a clipped portal's convex hull\n", 64);
            portal->writable.hullPointCount = hullPointCount;
            memcpy(
                (unsigned __int8 *)portal->writable.hullPoints,
                (unsigned __int8 *)hull,
                8 * portal->writable.hullPointCount);
        }
    }
    else
    {
        portal->writable.hullPoints = (float (*)[2])R_AllocHullPoints();
        portal->writable.hullPointCount = 0;
    }
    portal->writable.hullPoints[portal->writable.hullPointCount][0] = Vec3Dot(v, portal->hullAxis[0]);
    portal->writable.hullPoints[portal->writable.hullPointCount++][1] = Vec3Dot(v, portal->hullAxis[1]);
}

GfxHullPointsPool *__cdecl R_AllocHullPoints()
{
    GfxHullPointsPool *hullPointsPool; // [esp+0h] [ebp-4h]

    hullPointsPool = dpvsGlob.nextFreeHullPoints;
    if (!dpvsGlob.nextFreeHullPoints)
        Com_Error(ERR_DROP, "more than %i queued portals", 256);
    dpvsGlob.nextFreeHullPoints = hullPointsPool->nextFree;
    return hullPointsPool;
}

int __cdecl R_ChopPortal(
    const GfxPortal *portal,
    const DpvsPlane *parentPlane,
    const DpvsPlane *planes,
    int planeCount,
    float (*v)[128][3],
    const float (**finalVerts)[3])
{
    int vertCount; // [esp+0h] [ebp-Ch] BYREF
    int planeIndex; // [esp+4h] [ebp-8h]
    const float (*w)[3]; // [esp+8h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 2492, 0, "%s", "Sys_IsMainThread()");
    if (!portal)
        MyAssertHandler(".\\r_dpvs.cpp", 2493, 0, "%s", "portal");
    if (!parentPlane)
        MyAssertHandler(".\\r_dpvs.cpp", 2494, 0, "%s", "parentPlane");
    if (!planes && planeCount)
        MyAssertHandler(".\\r_dpvs.cpp", 2495, 0, "%s", "planes || planeCount == 0");
    vertCount = portal->vertexCount;
    w = portal->vertices;
    w = R_ChopPortalWinding(w, &vertCount, parentPlane, (float (*)[3])v);
    if (!vertCount)
        return 0;
    if (dpvsGlob.farPlane)
    {
        w = R_ChopPortalWinding(w, &vertCount, dpvsGlob.farPlane, (float (*)[3])(*v)[128 * (w == (const float (*)[3])v)]);
        if (!vertCount)
            return 0;
    }
    for (planeIndex = 0; planeIndex < planeCount; ++planeIndex)
    {
        w = R_ChopPortalWinding(w, &vertCount, &planes[planeIndex], (float (*)[3])(*v)[128 * (w == (const float (*)[3])v)]);
        if (!vertCount)
            return 0;
    }
    if (finalVerts)
        *finalVerts = w;
    return vertCount;
}

const float (*__cdecl R_ChopPortalWinding(
    const float (*vertsIn)[3],
    int *vertexCount,
    const DpvsPlane *plane,
    float (*vertsOut)[3]))[3]
{
    float v5; // [esp+0h] [ebp-2D0h]
    float v6; // [esp+4h] [ebp-2CCh]
    float v7; // [esp+8h] [ebp-2C8h]
    float *v8; // [esp+Ch] [ebp-2C4h]
    float *v9; // [esp+10h] [ebp-2C0h]
    float *v10; // [esp+14h] [ebp-2BCh]
    float *v11; // [esp+18h] [ebp-2B8h]
    float v12; // [esp+1Ch] [ebp-2B4h]
    unsigned __int8 sideForVert[136]; // [esp+20h] [ebp-2B0h]
    float lerpFactor; // [esp+ACh] [ebp-224h]
    int backCount; // [esp+B0h] [ebp-220h]
    int vertexIndex; // [esp+B4h] [ebp-21Ch]
    float distForVert[131]; // [esp+B8h] [ebp-218h]
    int newVertCount; // [esp+2C4h] [ebp-Ch]
    int frontCount; // [esp+2C8h] [ebp-8h]
    const float *v; // [esp+2CCh] [ebp-4h]

    frontCount = 0;
    backCount = 0;
    for (vertexIndex = 0; vertexIndex < *vertexCount; ++vertexIndex)
    {
        v12 = Vec3Dot(plane->coeffs, &(*vertsIn)[3 * vertexIndex]) + plane->coeffs[3];
        distForVert[vertexIndex] = v12 - EQUAL_EPSILON;
        sideForVert[vertexIndex] = 2;
        if (distForVert[vertexIndex] >= -EQUAL_EPSILON)
        {
            if (distForVert[vertexIndex] > EQUAL_EPSILON)
            {
                sideForVert[vertexIndex] = 0;
                ++frontCount;
            }
        }
        else
        {
            sideForVert[vertexIndex] = 1;
            ++backCount;
        }
    }
    if (frontCount)
    {
        if (backCount)
        {
            sideForVert[vertexIndex] = sideForVert[0];
            distForVert[vertexIndex] = distForVert[0];
            newVertCount = 0;
            for (vertexIndex = 0; vertexIndex < *vertexCount && newVertCount < 128; ++vertexIndex)
            {
                if (sideForVert[vertexIndex] == 2)
                {
                    v10 = &(*vertsOut)[3 * newVertCount];
                    v11 = (float *)&(*vertsIn)[3 * vertexIndex];
                    *v10 = *v11;
                    v10[1] = v11[1];
                    v10[2] = v11[2];
                    ++newVertCount;
                }
                else
                {
                    if (!sideForVert[vertexIndex])
                    {
                        v8 = &(*vertsOut)[3 * newVertCount];
                        v9 = (float *)&(*vertsIn)[3 * vertexIndex];
                        *v8 = *v9;
                        v8[1] = v9[1];
                        v8[2] = v9[2];
                        ++newVertCount;
                    }
                    if (sideForVert[vertexIndex + 1] != 2 && sideForVert[vertexIndex + 1] != sideForVert[vertexIndex])
                    {
                        lerpFactor = distForVert[vertexIndex] / (distForVert[vertexIndex] - distForVert[vertexIndex + 1]);
                        v = &(*vertsIn)[3 * ((vertexIndex + 1) % *vertexCount)];
                        v7 = (*v - (float)(*vertsIn)[3 * vertexIndex]) * lerpFactor + (float)(*vertsIn)[3 * vertexIndex];
                        (*vertsOut)[3 * newVertCount] = v7;
                        v6 = (v[1] - (float)(*vertsIn)[3 * vertexIndex + 1]) * lerpFactor + (float)(*vertsIn)[3 * vertexIndex + 1];
                        (*vertsOut)[3 * newVertCount + 1] = v6;
                        v5 = (v[2] - (float)(*vertsIn)[3 * vertexIndex + 2]) * lerpFactor + (float)(*vertsIn)[3 * vertexIndex + 2];
                        (*vertsOut)[3 * newVertCount++ + 2] = v5;
                    }
                }
            }
            if (newVertCount < 3)
                MyAssertHandler(".\\r_dpvs.cpp", 690, 0, "%s", "newVertCount >= 3");
            *vertexCount = newVertCount;
            return vertsOut;
        }
        else
        {
            return vertsIn;
        }
    }
    else
    {
        *vertexCount = 0;
        return 0;
    }
}

void __cdecl R_SetCellVisible(const GfxCell *cell)
{
    if (!dpvsGlob.cellBits)
        MyAssertHandler(".\\r_dpvs.cpp", 2618, 0, "%s", "dpvsGlob.cellBits");
    dpvsGlob.cellBits[(unsigned int)(cell - rgp.world->cells) >> 5] |= 1 << ((cell - rgp.world->cells) & 0x1F);
}

void __cdecl R_SetAncestorListStatus(GfxPortal *portal, bool isAncestor)
{
    while (portal)
    {
        if (portal->writable.isAncestor == isAncestor)
            MyAssertHandler(".\\r_dpvs.cpp", 2645, 0, "%s", "portal->writable.isAncestor != isAncestor");
        portal->writable.isAncestor = isAncestor;
        portal = portal->writable.queuedParent;
    }
}
void __cdecl R_AddWorldSurfacesFrustumOnly()
{
    int v0; // [esp+8h] [ebp-B4h]
    DpvsPlane *v1; // [esp+Ch] [ebp-B0h]
    int v2; // [esp+10h] [ebp-ACh]
    int v3; // [esp+14h] [ebp-A8h]
    DpvsPlane *v4; // [esp+18h] [ebp-A4h]
    int v5; // [esp+1Ch] [ebp-A0h]
    GfxCell *cell; // [esp+20h] [ebp-9Ch]
    GfxCell *cella; // [esp+20h] [ebp-9Ch]
    unsigned int cellIndex; // [esp+24h] [ebp-98h]
    unsigned int cellIndexa; // [esp+24h] [ebp-98h]
    unsigned int casterIndex; // [esp+28h] [ebp-94h]
    unsigned int cellDrawBits[32]; // [esp+2Ch] [ebp-90h] BYREF
    unsigned int cellCount; // [esp+ACh] [ebp-10h]
    unsigned int visibleCellIndex; // [esp+B0h] [ebp-Ch]
    unsigned int *cellCasterBits; // [esp+B4h] [ebp-8h]
    unsigned int cellBitsCount; // [esp+B8h] [ebp-4h]

    cellCount = rgp.world->dpvsPlanes.cellCount;
    if (!cellCount)
        MyAssertHandler(".\\r_dpvs.cpp", 2994, 0, "%s", "cellCount");
    if (sm_strictCull->current.enabled)
    {
        cellBitsCount = (cellCount + 31) >> 5;
        memset((unsigned __int8 *)cellDrawBits, 0, 4 * cellBitsCount);
        for (visibleCellIndex = 0; visibleCellIndex < cellCount; ++visibleCellIndex)
        {
            if ((dpvsGlob.cellVisibleBits[visibleCellIndex >> 5] & (1 << (visibleCellIndex & 0x1F))) != 0)
            {
                cellCasterBits = &rgp.world->cellCasterBits[cellBitsCount * visibleCellIndex];
                for (casterIndex = 0; casterIndex < cellBitsCount; ++casterIndex)
                    cellDrawBits[casterIndex] |= cellCasterBits[casterIndex];
            }
        }
        for (cellIndex = 0; cellIndex < cellCount; ++cellIndex)
        {
            if ((cellDrawBits[cellIndex >> 5] & (1 << (cellIndex & 0x1F))) != 0)
            {
                cell = &rgp.world->cells[cellIndex];
                v5 = 0;
                v4 = g_dpvsView->frustumPlanes;
                while (v5 < g_dpvsView->frustumPlaneCount)
                {
                    if (*(float *)((char *)cell->mins + v4->side[0]) * v4->coeffs[0]
                        + v4->coeffs[3]
                        + *(float *)((char *)cell->mins + v4->side[1]) * v4->coeffs[1]
                        + *(float *)((char *)cell->mins + v4->side[2]) * v4->coeffs[2] <= 0.0)
                    {
                        v3 = 1;
                        goto LABEL_22;
                    }
                    ++v5;
                    ++v4;
                }
                v3 = 0;
            LABEL_22:
                if (!v3)
                    R_AddCellSurfacesAndCullGroupsInFrustumDelayed(
                        cell,
                        g_dpvsView->frustumPlanes,
                        g_dpvsView->frustumPlaneCount, g_dpvsView->frustumPlaneCount);
            }
        }
    }
    else
    {
        for (cellIndexa = 0; cellIndexa < cellCount; ++cellIndexa)
        {
            cella = &rgp.world->cells[cellIndexa];
            v2 = 0;
            v1 = g_dpvsView->frustumPlanes;
            while (v2 < g_dpvsView->frustumPlaneCount)
            {
                if (*(float *)((char *)cella->mins + v1->side[0]) * v1->coeffs[0]
                    + v1->coeffs[3]
                    + *(float *)((char *)cella->mins + v1->side[1]) * v1->coeffs[1]
                    + *(float *)((char *)cella->mins + v1->side[2]) * v1->coeffs[2] <= 0.0)
                {
                    v0 = 1;
                    goto LABEL_34;
                }
                ++v2;
                ++v1;
            }
            v0 = 0;
        LABEL_34:
            if (!v0)
                R_AddCellSurfacesAndCullGroupsInFrustumDelayed(
                    cella,
                    g_dpvsView->frustumPlanes,
                    g_dpvsView->frustumPlaneCount,
                    g_dpvsView->frustumPlaneCount);
        }
    }
}

void __cdecl R_AddCellSurfacesAndCullGroupsInFrustumDelayed(
    const GfxCell *cell,
    const DpvsPlane *planes,
    unsigned __int8 planeCount,
    unsigned __int8 frustumPlaneCount)
{
    DpvsDynamicCellCmd dpvsDynamicCell; // [esp+0h] [ebp-18h] BYREF
    DpvsStaticCellCmd dpvsStaticCell; // [esp+Ch] [ebp-Ch] BYREF

    dpvsStaticCell.cell = cell;
    dpvsStaticCell.planes = planes;
    dpvsStaticCell.planeCount = planeCount;
    dpvsStaticCell.frustumPlaneCount = frustumPlaneCount;
    dpvsStaticCell.viewIndex = g_viewIndex; // *(_WORD *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 12);
    R_AddWorkerCmd(3, (unsigned __int8 *)&dpvsStaticCell);
    dpvsDynamicCell.cellIndex = cell - rgp.world->cells;
    dpvsDynamicCell.planes = planes;
    dpvsDynamicCell.planeCount = planeCount;
    dpvsDynamicCell.frustumPlaneCount = frustumPlaneCount;
    dpvsDynamicCell.viewIndex = g_viewIndex; //*(_WORD *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 12);
    R_AddWorkerCmd(5, (unsigned __int8 *)&dpvsDynamicCell);
    R_AddWorkerCmd(4, (unsigned __int8 *)&dpvsDynamicCell);
    R_AddWorkerCmd(6, (unsigned __int8 *)&dpvsDynamicCell);
}

void __cdecl R_ShowCull()
{
    float s; // [esp+0h] [ebp-50h]
    float *origin; // [esp+14h] [ebp-3Ch]
    GfxSceneModel *sceneModel; // [esp+18h] [ebp-38h]
    float mins[3]; // [esp+1Ch] [ebp-34h] BYREF
    float radius; // [esp+28h] [ebp-28h]
    unsigned int sceneEntCount; // [esp+2Ch] [ebp-24h]
    float maxs[3]; // [esp+30h] [ebp-20h] BYREF
    unsigned int sceneEntIndex; // [esp+3Ch] [ebp-14h]
    GfxSceneEntity *sceneEnt; // [esp+40h] [ebp-10h]
    GfxSceneBrush *sceneBrush; // [esp+44h] [ebp-Ch]
    unsigned __int8 *sceneEntVisData; // [esp+48h] [ebp-8h]
    const GfxBrushModel *bmodel; // [esp+4Ch] [ebp-4h]

    if (r_showCullXModels->current.enabled)
    {
        sceneEntCount = scene.sceneDObjCount;
        sceneEntVisData = scene.sceneDObjVisData[0];
        for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
        {
            sceneEnt = &scene.sceneDObj[sceneEntIndex];
            if (sceneEntVisData[sceneEntIndex] == 1)
                R_AddDebugBox(&frontEndDataOut->debugGlobals, sceneEnt->cull.mins, sceneEnt->cull.maxs, colorGreen);
            else
                R_AddDebugBox(&frontEndDataOut->debugGlobals, sceneEnt->cull.mins, sceneEnt->cull.maxs, colorRed);
        }
        sceneEntCount = scene.sceneModelCount;
        sceneEntVisData = scene.sceneModelVisData[0];
        for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
        {
            sceneModel = &scene.sceneModel[sceneEntIndex];
            origin = CG_GetEntityOrigin(scene.dpvs.localClientNum, sceneModel->entnum);
            radius = XModelGetRadius(sceneModel->model);
            s = -radius;
            Vec3AddScalar(origin, s, mins);
            Vec3AddScalar(origin, radius, maxs);
            if (sceneEntVisData[sceneEntIndex] == 1)
                R_AddDebugBox(&frontEndDataOut->debugGlobals, mins, maxs, colorGreen);
            else
                R_AddDebugBox(&frontEndDataOut->debugGlobals, mins, maxs, colorRed);
        }
    }
    if (r_showCullBModels->current.enabled)
    {
        sceneEntCount = scene.sceneBrushCount;
        sceneEntVisData = scene.sceneBrushVisData[0];
        for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
        {
            sceneBrush = &scene.sceneBrush[sceneEntIndex];
            bmodel = sceneBrush->bmodel;
            if (sceneEntVisData[sceneEntIndex] == 1)
                R_AddDebugBox(&frontEndDataOut->debugGlobals, bmodel->writable.mins, bmodel->writable.maxs, colorGreen);
            else
                R_AddDebugBox(&frontEndDataOut->debugGlobals, bmodel->writable.mins, bmodel->writable.maxs, colorRed);
        }
    }
}

void __cdecl R_InitSceneData(int localClientNum)
{
    unsigned int cellIndex; // [esp+0h] [ebp-10h]
    unsigned int offset; // [esp+4h] [ebp-Ch]
    unsigned int cellCount; // [esp+Ch] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 3148, 0, "%s", "Sys_IsMainThread()");
    if (!rgp.world)
        MyAssertHandler(".\\r_dpvs.cpp", 3149, 0, "%s", "rgp.world");
    if (gfxCfg.entCount * gfxCfg.maxClientViews > 0x1000)
        MyAssertHandler(".\\r_dpvs.cpp", 3151, 0, "%s", "gfxCfg.maxClientViews * gfxCfg.entCount <= MAX_TOTAL_ENT_COUNT");
    cellCount = rgp.world->dpvsPlanes.cellCount;
    if ((gfxCfg.entCount & 0x1F) != 0)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            3153,
            0,
            "%s\n\t(gfxCfg.entCount) = %i",
            "(!(gfxCfg.entCount & 31))",
            gfxCfg.entCount);
    offset = localClientNum * (gfxCfg.entCount >> 5);
    if (offset >= 0x80)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            3156,
            0,
            "offset doesn't index MAX_TOTAL_ENT_COUNT >> 5\n\t%i not in [0, %i)",
            offset,
            128);
    for (cellIndex = 0; cellIndex < 2 * cellCount; ++cellIndex)
        Com_Memset(&rgp.world->dpvsPlanes.sceneEntCellBits[128 * cellIndex + offset], 0, 4 * (gfxCfg.entCount >> 5));
    memset((unsigned __int8 *)dpvsGlob.entVisBits[localClientNum], 0, 4 * (gfxCfg.entCount >> 5));
    memset((unsigned __int8 *)scene.dpvs.entInfo[localClientNum], 0, 4 * gfxCfg.entCount);
}

void __cdecl DynEntCl_InitFilter()
{
    unsigned int drawType; // [esp+0h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 3173, 0, "%s", "Sys_IsMainThread()");
    if (!rgp.world)
        MyAssertHandler(".\\r_dpvs.cpp", 3174, 0, "%s", "rgp.world");
    scene.sceneDynModelCount = 0;
    scene.sceneDynBrushCount = 0;
    for (drawType = 0; drawType < 2; ++drawType)
        Com_Memset(
            rgp.world->dpvsDyn.dynEntCellBits[drawType],
            0,
            4 * rgp.world->dpvsPlanes.cellCount * rgp.world->dpvsDyn.dynEntClientWordCount[drawType]);
}

void __cdecl R_InitSceneBuffers()
{
    unsigned int localClientNum; // [esp+0h] [ebp-8h]
    unsigned int viewIndex; // [esp+4h] [ebp-4h]

    if ((gfxCfg.entCount & 0x1F) != 0)
        MyAssertHandler(".\\r_dpvs.cpp", 3189, 1, "%s", "(gfxCfg.entCount & 31) == 0");
    scene.entOverflowedDrawBuf = (unsigned int *)R_AllocGlobalVariable(gfxCfg.entCount >> 3, "R_InitSceneBuffers");
    for (viewIndex = 0; viewIndex < 7; ++viewIndex)
        scene.dpvs.entVisData[viewIndex] = (unsigned __int8 *)R_AllocGlobalVariable(gfxCfg.entCount, "R_InitSceneBuffers");
    scene.dpvs.sceneXModelIndex = (unsigned __int16 *)R_AllocGlobalVariable(2 * gfxCfg.entCount, "R_InitSceneBuffers");
    scene.dpvs.sceneDObjIndex = (unsigned __int16 *)R_AllocGlobalVariable(2 * gfxCfg.entCount, "R_InitSceneBuffers");
    for (localClientNum = 0; localClientNum < gfxCfg.maxClientViews; ++localClientNum)
    {
        dpvsGlob.entVisBits[localClientNum] = (unsigned int *)R_AllocGlobalVariable(
            4 * (gfxCfg.entCount >> 5),
            "R_InitSceneBuffers");
        scene.dpvs.entInfo[localClientNum] = (GfxEntCellRefInfo *)R_AllocGlobalVariable(
            4 * gfxCfg.entCount,
            "R_InitSceneBuffers");
    }
}

void __cdecl R_ClearDpvsScene()
{
    unsigned int drawType; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]
    int ia; // [esp+4h] [ebp-4h]

    if (!rgp.world)
        MyAssertHandler(".\\r_dpvs.cpp", 3232, 0, "%s", "rgp.world");
    if (!rgp.world->cells)
        MyAssertHandler(".\\r_dpvs.cpp", 3233, 0, "%s", "rgp.world->cells");
    Com_Memset((unsigned int *)scene.dpvs.sceneXModelIndex, 255, 2 * gfxCfg.entCount);
    if (*scene.dpvs.sceneXModelIndex != 0xFFFF)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            3236,
            0,
            "%s\n\t(scene.dpvs.sceneXModelIndex[0]) = %i",
            "(scene.dpvs.sceneXModelIndex[0] == (65535))",
            *scene.dpvs.sceneXModelIndex);
    Com_Memset((unsigned int *)scene.dpvs.sceneDObjIndex, 255, 2 * gfxCfg.entCount);
    if (*scene.dpvs.sceneDObjIndex != 0xFFFF)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            3239,
            0,
            "%s\n\t(scene.dpvs.sceneDObjIndex[0]) = %i",
            "(scene.dpvs.sceneDObjIndex[0] == (65535))",
            *scene.dpvs.sceneDObjIndex);
    if ((gfxCfg.entCount & 0x1F) != 0)
        MyAssertHandler(".\\r_dpvs.cpp", 3241, 1, "%s", "(gfxCfg.entCount & 31) == 0");
    Com_Memset(scene.entOverflowedDrawBuf, 0, gfxCfg.entCount >> 3);
    for (i = 0; i < 7; ++i)
        Com_Memset((unsigned int *)scene.dpvs.entVisData[i], 0, gfxCfg.entCount);
    for (ia = 0; ia < 3; ++ia)
    {
        Com_Memset((unsigned int *)rgp.world->dpvs.smodelVisData[ia], 0, rgp.world->dpvs.smodelCount);
        Com_Memset((unsigned int *)rgp.world->dpvs.surfaceVisData[ia], 0, rgp.world->models->surfaceCount);
        for (drawType = 0; drawType < 2; ++drawType)
            Com_Memset(
                (unsigned int *)rgp.world->dpvsDyn.dynEntVisData[drawType][ia],
                0,
                rgp.world->dpvsDyn.dynEntClientCount[drawType]);
    }
    Com_Memset((unsigned int *)&rgp.world->sceneDynModel->info, 0, 6 * scene.sceneDynModelCount);
    scene.sceneDynModelCount = 0;
    Com_Memset((unsigned int *)rgp.world->sceneDynBrush, 0, 4 * scene.sceneDynBrushCount);
    scene.sceneDynBrushCount = 0;
    R_SetVisData(0);
}

bool __cdecl R_CullDynamicSpotLightInCameraView()
{
    if (!scene.addedLightCount)
        return 1;
    if (scene.addedLight[0].type != 2)
        return 1;
    scene.isAddedLightCulled[0] = R_CullPointAndRadius(
        scene.addedLight[0].origin,
        scene.addedLight[0].radius,
        dpvsGlob.views[scene.dpvs.localClientNum][0].frustumPlanes,
        dpvsGlob.views[scene.dpvs.localClientNum][0].frustumPlaneCount);
    return scene.isAddedLightCulled[0];
}

void __cdecl R_CullDynamicPointLightsInCameraView()
{
    int planeCount; // [esp+Ch] [ebp-10h]
    DpvsPlane *planes; // [esp+10h] [ebp-Ch]
    GfxLight *v2; // [esp+14h] [ebp-8h]
    int lightIndex; // [esp+18h] [ebp-4h]

    planes = dpvsGlob.views[scene.dpvs.localClientNum][0].frustumPlanes;
    planeCount = dpvsGlob.views[scene.dpvs.localClientNum][0].frustumPlaneCount;
    for (lightIndex = 0; lightIndex < scene.addedLightCount; ++lightIndex)
    {
        v2 = &scene.addedLight[lightIndex];
        if (v2->type != 2 || lightIndex)
        {
            if (v2->type != 3)
                MyAssertHandler(".\\r_dpvs.cpp", 3309, 1, "%s", "dl->type == GFX_LIGHT_TYPE_OMNI");
            scene.isAddedLightCulled[lightIndex] = R_CullPointAndRadius(v2->origin, v2->radius, planes, planeCount);
        }
    }
}

const float standardFrustumSidePlanes[4][4] =
{
  { -1.0, 0.0, 0.0, 1.0 },
  { 1.0, 0.0, 0.0, 1.0 },
  { 0.0, -1.0, 0.0, 1.0 },
  { 0.0, 1.0, 0.0, 1.0 }
}; // idb

void __cdecl R_SetupWorldSurfacesDpvs(const GfxViewParms *viewParms)
{
    DpvsView *dpvsView; // [esp+0h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 3319, 0, "%s", "Sys_IsMainThread()");
    if (!rgp.world)
        MyAssertHandler(".\\r_dpvs.cpp", 3320, 0, "%s", "rgp.world");
    if (!viewParms)
        MyAssertHandler(".\\r_dpvs.cpp", 3321, 0, "%s", "viewParms");
    dpvsView = dpvsGlob.views[scene.dpvs.localClientNum];
    dpvsView->renderFxFlagsCull = 0;
    dpvsGlob.viewProjMtx = &viewParms->viewProjectionMatrix;
    dpvsGlob.invViewProjMtx = &viewParms->inverseViewProjectionMatrix;
    R_FrustumClipPlanes(&viewParms->viewProjectionMatrix, standardFrustumSidePlanes, 4, dpvsView->frustumPlanes);
    if (viewParms->projectionMatrix.m[3][3] != 0.0)
        MyAssertHandler(".\\r_dpvs.cpp", 3333, 0, "%s", "viewParms->projectionMatrix.m[3][3] == 0");
    R_SetupDpvsForPoint(viewParms);
    dpvsGlob.sideFrustumPlanes = dpvsView->frustumPlanes;
    dpvsView->frustumPlaneCount = R_AddNearAndFarClipPlanes(dpvsView->frustumPlanes, 4);
}

int __cdecl R_AddNearAndFarClipPlanes(DpvsPlane *planes, int planeCount)
{
    int planeCounta; // [esp+Ch] [ebp+Ch]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 185, 0, "%s", "Sys_IsMainThread()");
    if (!dpvsGlob.nearPlane)
        MyAssertHandler(".\\r_dpvs.cpp", 187, 0, "%s", "dpvsGlob.nearPlane");
    planes[planeCount] = *dpvsGlob.nearPlane;
    planeCounta = planeCount + 1;
    if (dpvsGlob.farPlane)
        planes[planeCounta++] = *dpvsGlob.farPlane;
    return planeCounta;
}

void __cdecl R_SetupDpvsForPoint(const GfxViewParms *viewParms)
{
    float zfar; // [esp+14h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 2956, 0, "%s", "Sys_IsMainThread()");
    dpvsGlob.viewOrg[0] = viewParms->origin[0];
    dpvsGlob.viewOrg[1] = viewParms->origin[1];
    dpvsGlob.viewOrg[2] = viewParms->origin[2];
    dpvsGlob.viewOrg[3] = 1.0;
    dpvsGlob.viewOrgIsDir = 0;
    dpvsGlob.viewPlane.coeffs[0] = viewParms->axis[0][0];
    dpvsGlob.viewPlane.coeffs[1] = viewParms->axis[0][1];
    dpvsGlob.viewPlane.coeffs[2] = viewParms->axis[0][2];
    dpvsGlob.viewPlane.coeffs[3] = 0.1 - Vec3Dot(dpvsGlob.viewPlane.coeffs, dpvsGlob.viewOrg);
    dpvsGlob.viewPlane.side[0] = dpvsGlob.viewPlane.coeffs[0] <= 0 ? 0 : 0xC;
    dpvsGlob.viewPlane.side[1] = dpvsGlob.viewPlane.coeffs[1] <= 0 ? 4 : 16;
    dpvsGlob.viewPlane.side[2] = dpvsGlob.viewPlane.coeffs[2] <= 0 ? 8 : 20;
    dpvsGlob.nearPlane = (DpvsPlane *)&dpvsGlob;
    zfar = R_GetFarPlaneDist();
    if (zfar > 0.0)
    {
        dpvsGlob.fogPlane.coeffs[0] = -viewParms->axis[0][0];
        dpvsGlob.fogPlane.coeffs[1] = -viewParms->axis[0][1];
        dpvsGlob.fogPlane.coeffs[2] = -viewParms->axis[0][2];
        dpvsGlob.fogPlane.coeffs[3] = zfar - Vec3Dot(dpvsGlob.fogPlane.coeffs, dpvsGlob.viewOrg);
        dpvsGlob.fogPlane.side[0] = dpvsGlob.fogPlane.coeffs[0] <= 0 ? 0 : 0xC;
        dpvsGlob.fogPlane.side[1] = dpvsGlob.fogPlane.coeffs[1] <= 0 ? 4 : 16;
        dpvsGlob.fogPlane.side[2] = dpvsGlob.fogPlane.coeffs[2] <= 0 ? 8 : 20;
        dpvsGlob.farPlane = &dpvsGlob.fogPlane;
    }
    else
    {
        dpvsGlob.farPlane = 0;
    }
}

void __cdecl R_SetViewFrustumPlanes(GfxViewInfo *viewInfo)
{
    float *v1; // [esp+0h] [ebp-Ch]
    DpvsPlane *v2; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    if (dpvsGlob.views[scene.dpvs.localClientNum][0].frustumPlaneCount < 4)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            3346,
            0,
            "%s\n\t(dpvsGlob.views[scene.dpvs.localClientNum][SCENE_VIEW_CAMERA].frustumPlaneCount) = %i",
            "(dpvsGlob.views[scene.dpvs.localClientNum][SCENE_VIEW_CAMERA].frustumPlaneCount >= 4)",
            dpvsGlob.views[scene.dpvs.localClientNum][0].frustumPlaneCount);
    for (i = 0; i < 4; ++i)
    {
        v1 = viewInfo->frustumPlanes[i];
        v2 = &dpvsGlob.views[scene.dpvs.localClientNum][0].frustumPlanes[i];
        *v1 = v2->coeffs[0];
        v1[1] = v2->coeffs[1];
        v1[2] = v2->coeffs[2];
        v1[3] = v2->coeffs[3];
    }
}

// lol.
void __cdecl R_CopyClipPlane(const DpvsPlane *in, DpvsPlane *out)
{
    *out = *in;
}

void __cdecl R_AddSkySurfacesDpvs(const DpvsPlane *planes, int planeCount)
{
    int v2; // [esp+4h] [ebp-1B0h]
    int v3; // [esp+8h] [ebp-1ACh]
    const DpvsPlane *v4; // [esp+Ch] [ebp-1A8h]
    unsigned int i; // [esp+10h] [ebp-1A4h]
    GfxSurface *v6; // [esp+18h] [ebp-19Ch]
    DpvsPlane clipPlanePool[16]; // [esp+1Ch] [ebp-198h] BYREF
    int surfIndex; // [esp+160h] [ebp-54h]
    DpvsClipPlaneSet clipSet; // [esp+164h] [ebp-50h]
    int planeIndex; // [esp+1B0h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs_static.cpp", 970, 0, "%s", "Sys_IsMainThread()");
    g_smodelVisData = rgp.world->dpvs.smodelVisData[0]; // (_DWORD *)(*((_DWORD *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 24)
    g_surfaceVisData = rgp.world->dpvs.surfaceVisData[0]; // *(_DWORD *)(*((_DWORD *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 28)
    for (planeIndex = 0; planeIndex < planeCount; ++planeIndex)
    {
        R_CopyClipPlane(&planes[planeIndex], &clipPlanePool[planeIndex]);
        clipSet.planes[planeIndex] = &clipPlanePool[planeIndex];
    }
    clipSet.count = planeCount;
    for (surfIndex = 0; surfIndex < rgp.world->skySurfCount; ++surfIndex)
    {
        v2 = rgp.world->skyStartSurfs[surfIndex];
        if (!*(_BYTE *)(g_surfaceVisData + v2))
        {
            v6 = &rgp.world->dpvs.surfaces[v2];
            for (i = 0; i < clipSet.count; ++i)
            {
                v4 = clipSet.planes[i];
                if (*(float *)((char *)v6->bounds[0] + v4->side[0]) * v4->coeffs[0]
                    + v4->coeffs[3]
                    + *(float *)((char *)v6->bounds[0] + v4->side[1]) * v4->coeffs[1]
                    + *(float *)((char *)v6->bounds[0] + v4->side[2]) * v4->coeffs[2] <= 0.0)
                {
                    v3 = 1;
                    goto LABEL_15;
                }
            }
            v3 = 0;
        LABEL_15:
            if (!v3)
            {
                if ((r_showAabbTrees->current.integer & 2) != 0)
                    R_AddDebugBox(&frontEndDataOut->debugGlobals, v6->bounds[0], v6->bounds[1], colorGreen);
                *(_BYTE *)(g_surfaceVisData + v2) = 1;
            }
        }
    }
}

void __cdecl R_AddWorldSurfacesDpvs(const GfxViewParms *viewParms, int cameraCellIndex)
{
    DpvsView *dpvsView; // [esp+0h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 3357, 0, "%s", "Sys_IsMainThread()");
    if (!rgp.world)
        MyAssertHandler(".\\r_dpvs.cpp", 3358, 0, "%s", "rgp.world");
    if (!viewParms)
        MyAssertHandler(".\\r_dpvs.cpp", 3359, 0, "%s", "viewParms");
    rg.debugViewParms = viewParms;
    R_AddWorldSurfacesPortalWalk(cameraCellIndex);
    dpvsView = dpvsGlob.views[scene.dpvs.localClientNum];
    if (dpvsGlob.farPlane)
    {
        if (dpvsView->frustumPlaneCount <= 0)
            MyAssertHandler(
                ".\\r_dpvs.cpp",
                3369,
                0,
                "%s\n\t(dpvsView->frustumPlaneCount) = %i",
                "(dpvsView->frustumPlaneCount > 0)",
                dpvsView->frustumPlaneCount);
        if (!Vec4Compare(dpvsView->frustumPlanes[dpvsView->frustumPlaneCount - 1].coeffs, dpvsGlob.farPlane->coeffs))
            MyAssertHandler(
                ".\\r_dpvs.cpp",
                3370,
                0,
                "%s",
                "Vec4Compare( dpvsView->frustumPlanes[dpvsView->frustumPlaneCount - 1].coeffs, dpvsGlob.farPlane->coeffs )");
        R_AddSkySurfacesDpvs(dpvsView->frustumPlanes, dpvsView->frustumPlaneCount - 1);
    }
    if (r_vc_makelog->current.integer)
        R_ShowLightVisCachePoints(viewParms->origin, dpvsView->frustumPlanes, dpvsView->frustumPlaneCount);
}

void __cdecl R_AddWorldSurfacesPortalWalk(int cameraCellIndex)
{
    GfxCell *cell; // [esp+0h] [ebp-Ch]
    int cellIndex; // [esp+4h] [ebp-8h]
    DpvsView *dpvsView; // [esp+8h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 3045, 0, "%s", "Sys_IsMainThread()");
    if (!rgp.world->dpvsPlanes.cellCount)
        MyAssertHandler(".\\r_dpvs.cpp", 3047, 0, "%s", "rgp.world->dpvsPlanes.cellCount");
    memset((unsigned __int8 *)dpvsGlob.cellVisibleBits, 0, 4 * ((rgp.world->dpvsPlanes.cellCount + 31) >> 5));
    dpvsGlob.cellBits = dpvsGlob.cellVisibleBits;
    if (!r_skipPvs->current.enabled)
    {
        dpvsView = dpvsGlob.views[scene.dpvs.localClientNum];

        // *(DpvsView **)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 8)
        vassert(dpvsView == g_dpvsView, "dpsView == %p, g_dpsView == %p", dpvsView, g_dpvsView);
        if (cameraCellIndex < 0)
        {
            for (cellIndex = 0; cellIndex < rgp.world->dpvsPlanes.cellCount; ++cellIndex)
            {
                R_AddCellSurfacesAndCullGroupsInFrustumDelayed(
                    &rgp.world->cells[cellIndex],
                    dpvsView->frustumPlanes,
                    dpvsView->frustumPlaneCount,
                    dpvsView->frustumPlaneCount);
                R_SetCellVisible(&rgp.world->cells[cellIndex]);
            }
        }
        else
        {
            cell = &rgp.world->cells[cameraCellIndex];
            if (r_singleCell->current.enabled)
            {
                dpvsGlob.farPlane = 0;
                R_AddCellSurfacesAndCullGroupsInFrustumDelayed(
                    cell,
                    dpvsView->frustumPlanes,
                    dpvsView->frustumPlaneCount,
                    dpvsView->frustumPlaneCount);
                R_SetCellVisible(cell);
            }
            else
            {
                R_VisitPortals(cell, &dpvsGlob.viewPlane, dpvsView->frustumPlanes, dpvsView->frustumPlaneCount);
            }
        }
    }
}

const float color[4] = { 0.0f, 1.0f, 1.0f, 0.25f };

void __cdecl R_VisitPortals(const GfxCell *cell, const DpvsPlane *parentPlane, const DpvsPlane *planes, int planeCount)
{
    float scale; // [esp+4h] [ebp-D98h]
    float v5; // [esp+28h] [ebp-D74h]
    float v6; // [esp+2Ch] [ebp-D70h]
    DpvsPlane *childPlanes; // [esp+58h] [ebp-D44h]
    GfxHullPointsPool(*hullPointsPoolArray)[256]; // [esp+64h] [ebp-D38h]
    unsigned int childPlanesCount; // [esp+68h] [ebp-D34h]
    int childPlaneCount; // [esp+6Ch] [ebp-D30h]
    int iteration; // [esp+70h] [ebp-D2Ch]
    GfxPortal *portal; // [esp+74h] [ebp-D28h]
    int queueIndex; // [esp+78h] [ebp-D24h]
    float portalVerts[64][3]; // [esp+7Ch] [ebp-D20h] BYREF
    float hullOrigin[3]; // [esp+380h] [ebp-A1Ch] BYREF
    DpvsClipChildren clipChildren; // [esp+38Ch] [ebp-A10h] BYREF
    unsigned int vertIndex; // [esp+390h] [ebp-A0Ch]
    PortalHeapNode portalQueue[256]; // [esp+394h] [ebp-A08h] BYREF
    float hull[64][2]; // [esp+B94h] [ebp-208h] BYREF
    unsigned int hullPointCount; // [esp+D98h] [ebp-4h]

    LargeLocal hullPointsPoolArray_large_local(0x20000);
    //LargeLocal::LargeLocal(&hullPointsPoolArray_large_local, 0x20000);
    //hullPointsPoolArray = (GfxHullPointsPool(*)[256])LargeLocal::GetBuf(&hullPointsPoolArray_large_local);
    hullPointsPoolArray = (GfxHullPointsPool(*)[256])hullPointsPoolArray_large_local.GetBuf();
    //Profile_Begin(188);
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 2778, 0, "%s", "Sys_IsMainThread()");
    childPlanesCount = 0;
    for (queueIndex = 0; queueIndex < 255; ++queueIndex)
        (*hullPointsPoolArray)[queueIndex].nextFree = &(*hullPointsPoolArray)[queueIndex + 1];
    (*hullPointsPoolArray)[queueIndex].nextFree = 0;
    dpvsGlob.nextFreeHullPoints = (GfxHullPointsPool *)hullPointsPoolArray;
    dpvsGlob.portalQueue = portalQueue;
    dpvsGlob.queuedCount = 0;
    R_VisitPortalsForCell(cell, 0, parentPlane, planes, planeCount, planeCount, 0, DPVS_CLIP_CHILDREN);
    iteration = 0;
    while (dpvsGlob.queuedCount)
    {
        portal = R_NextQueuedPortal();
        if (!portal)
            MyAssertHandler(".\\r_dpvs.cpp", 2794, 0, "%s", "portal");
        //Profile_Begin(84);
        hullPointCount = Com_ConvexHull(portal->writable.hullPoints, portal->writable.hullPointCount, hull);
        //Profile_EndInternal(0);
        R_FreeHullPoints((GfxHullPointsPool *)portal->writable.hullPoints);
        portal->writable.hullPoints = 0;
        if (hullPointCount)
        {
            if (++iteration == r_portalWalkLimit->current.integer)
            {
                while (dpvsGlob.queuedCount)
                {
                    portal = R_NextQueuedPortal();
                    R_FreeHullPoints((GfxHullPointsPool *)portal->writable.hullPoints);
                    portal->writable.hullPoints = 0;
                }
                break;
            }
            v6 = Vec4Dot(portal->plane.coeffs, dpvsGlob.viewOrg);
            if (v6 > 0.1248750016093254)
            {
                v5 = Vec4Dot(portal->plane.coeffs, dpvsGlob.viewOrg);
                MyAssertHandler(
                    ".\\r_dpvs.cpp",
                    2817,
                    0,
                    "%s\n\t(R_DpvsPlaneDistToEye( &portal->plane )) = %g",
                    "(R_DpvsPlaneDistToEye( &portal->plane ) <= 0.125f * 0.999f)",
                    v5);
            }
            scale = -portal->plane.coeffs[3];
            Vec3Scale(portal->plane.coeffs, scale, hullOrigin);
            for (vertIndex = 0; vertIndex < hullPointCount; ++vertIndex)
            {
                Vec3Mad(hullOrigin, hull[vertIndex][0], portal->hullAxis[0], portalVerts[vertIndex]);
                Vec3Mad(portalVerts[vertIndex], hull[vertIndex][1], portal->hullAxis[1], portalVerts[vertIndex]);
            }
            if (r_showPortals->current.integer && !r_portalBevelsOnly->current.enabled)
                R_AddDebugPolygon(&frontEndDataOut->debugGlobals, color, hullPointCount, portalVerts);
            if (childPlanesCount + 16 > 0x800)
            {
                R_WarnOncePerFrame(R_WARN_PORTAL_PLANES);
                R_WaitWorkerCmdsOfType(3);
                R_WaitWorkerCmdsOfType(5);
                R_WaitWorkerCmdsOfType(4);
                R_WaitWorkerCmdsOfType(7);
                R_WaitWorkerCmdsOfType(6);
                childPlanesCount = 0;
            }
            childPlanes = &dpvsGlob.childPlanes[childPlanesCount];
            childPlaneCount = R_PortalClipPlanes(childPlanes, hullPointCount, portalVerts, portal->cell, &clipChildren);
            if (childPlaneCount > 16)
                MyAssertHandler(".\\r_dpvs.cpp", 2852, 0, "%s", "childPlaneCount <= DPVS_PORTAL_MAX_PLANES");
            childPlanesCount += childPlaneCount;
            if (portal->writable.recursionDepth < r_portalMinRecurseDepth->current.integer)
                clipChildren = DPVS_CLIP_CHILDREN;
            R_VisitPortalsForCell(
                portal->cell,
                portal,
                &portal->plane,
                childPlanes,
                childPlaneCount,
                0,
                portal->writable.recursionDepth + 1,
                clipChildren);
        }
    }
    //Profile_EndInternal(0);
    //LargeLocal::~LargeLocal(&hullPointsPoolArray_large_local);
}

unsigned int __cdecl R_PortalClipPlanes(
    DpvsPlane *planes,
    unsigned int vertexCount,
    const float (*winding)[3],
    GfxCell *cell,
    DpvsClipChildren *clipChildren)
{
    bool v6; // [esp+0h] [ebp-640h]
    DpvsForceBevels v7; // [esp+4h] [ebp-63Ch]
    DpvsPlane *a; // [esp+8h] [ebp-638h]
    DpvsPlane *v9; // [esp+10h] [ebp-630h]
    float *v10; // [esp+14h] [ebp-62Ch]
    float normals[128][3]; // [esp+18h] [ebp-628h] BYREF
    unsigned int windingVertIndex; // [esp+618h] [ebp-28h]
    unsigned int planeCount; // [esp+61Ch] [ebp-24h]
    float clipSpaceMins[2]; // [esp+620h] [ebp-20h] BYREF
    float clipSpaceMaxs[2]; // [esp+628h] [ebp-18h] BYREF
    bool useNormalPlanes; // [esp+633h] [ebp-Dh]
    float distMin; // [esp+634h] [ebp-Ch]
    DpvsForceBevels forceBevels; // [esp+638h] [ebp-8h]
    bool useBevelPlanes; // [esp+63Fh] [ebp-1h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 473, 0, "%s", "Sys_IsMainThread()");
    if (vertexCount < 3)
        MyAssertHandler(".\\r_dpvs.cpp", 474, 0, "%s\n\t(vertexCount) = %i", "(vertexCount >= 3)", vertexCount);
    useNormalPlanes = vertexCount <= 0xA;
    v7 = (DpvsForceBevels)(vertexCount > 0xA || r_portalBevelsOnly->current.enabled);
    forceBevels = v7;
    v6 = v7 == DPVS_FORCE_BEVELS || r_portalBevels->current.value > 0.0;
    useBevelPlanes = v6;
    R_GetSidePlaneNormals(winding, vertexCount, normals);
    planeCount = 0;
    if (useBevelPlanes || r_portalMinClipArea->current.value > 0.0)
    {
        R_ProjectPortal(vertexCount, winding, clipSpaceMins, clipSpaceMaxs, clipChildren);
        if (useBevelPlanes)
            planeCount = R_AddBevelPlanes(planes, vertexCount, winding, normals, clipSpaceMins, clipSpaceMaxs, forceBevels);
    }
    else
    {
        *clipChildren = DPVS_CLIP_CHILDREN;
    }
    if (useNormalPlanes)
    {
        for (windingVertIndex = 0; windingVertIndex < vertexCount; ++windingVertIndex)
        {
            if (Vec3LengthSq(normals[windingVertIndex]) != 0.0)
            {
                v9 = &planes[planeCount];
                v10 = normals[windingVertIndex];
                v9->coeffs[0] = *v10;
                v9->coeffs[1] = v10[1];
                v9->coeffs[2] = v10[2];
                a = &planes[planeCount];
                a->coeffs[3] = 0.001 - Vec3Dot(a->coeffs, &(*winding)[3 * windingVertIndex]);
                a->side[0] = COERCE_INT(a->coeffs[0]) <= 0 ? 0 : 0xC;
                a->side[1] = COERCE_INT(a->coeffs[1]) <= 0 ? 4 : 16;
                a->side[2] = COERCE_INT(a->coeffs[2]) <= 0 ? 8 : 20;
                ++planeCount;
            }
        }
    }
    if (!dpvsGlob.nearPlane)
        MyAssertHandler(".\\r_dpvs.cpp", 506, 0, "%s", "dpvsGlob.nearPlane");
    planes[planeCount] = *dpvsGlob.nearPlane;
    distMin = R_NearestPointOnWinding(winding, vertexCount, &planes[planeCount]);
    if (distMin > 0.0)
        planes[planeCount].coeffs[3] = planes[planeCount].coeffs[3] - distMin;
    ++planeCount;
    if (dpvsGlob.farPlane)
        planes[planeCount++] = *dpvsGlob.farPlane;
    return planeCount;
}

double __cdecl R_NearestPointOnWinding(const float (*points)[3], int pointCount, const DpvsPlane *plane)
{
    float v4; // [esp+0h] [ebp-24h]
    float v5; // [esp+4h] [ebp-20h]
    float v6; // [esp+8h] [ebp-1Ch]
    float v7; // [esp+Ch] [ebp-18h]
    float distMin; // [esp+18h] [ebp-Ch]
    int pointIndex; // [esp+1Ch] [ebp-8h]
    int pointIndexa; // [esp+1Ch] [ebp-8h]

    v7 = Vec3Dot(plane->coeffs, (const float *)points) + plane->coeffs[3];
    v6 = Vec3Dot(plane->coeffs, &(*points)[3 * pointCount - 3]) + plane->coeffs[3];
    if (v6 <= (double)v7)
    {
        distMin = v6;
        for (pointIndexa = pointCount - 2; pointIndexa > 0; --pointIndexa)
        {
            v4 = Vec3Dot(plane->coeffs, &(*points)[3 * pointIndexa]) + plane->coeffs[3];
            if (v4 > (double)distMin)
                break;
            distMin = v4;
        }
    }
    else
    {
        distMin = v7;
        for (pointIndex = 1; pointIndex < pointCount - 1; ++pointIndex)
        {
            v5 = Vec3Dot(plane->coeffs, &(*points)[3 * pointIndex]) + plane->coeffs[3];
            if (v5 > (double)distMin)
                break;
            distMin = v5;
        }
    }
    return distMin;
}

void __cdecl R_ProjectPortal(
    int vertexCount,
    const float (*winding)[3],
    float *mins,
    float *maxs,
    DpvsClipChildren *clipChildren)
{
    char *v5; // eax
    const char *v6; // eax
    char *v7; // eax
    const char *v8; // eax
    int windingVertIndex; // [esp+40h] [ebp-440h]
    int windingVertIndexa; // [esp+40h] [ebp-440h]
    float area; // [esp+44h] [ebp-43Ch]
    float areaa; // [esp+44h] [ebp-43Ch]
    float areab; // [esp+44h] [ebp-43Ch]
    const float *xyz; // [esp+48h] [ebp-438h]
    float invW; // [esp+4Ch] [ebp-434h]
    float x; // [esp+50h] [ebp-430h]
    float y; // [esp+54h] [ebp-42Ch]
    float screenSpaceWinding[265]; // [esp+58h] [ebp-428h] BYREF
    float w; // [esp+47Ch] [ebp-4h]

    if (vertexCount < 3)
        MyAssertHandler(".\\r_dpvs.cpp", 335, 0, "%s\n\t(vertexCount) = %i", "(vertexCount >= 3)", vertexCount);
    *mins = 1.0;
    mins[1] = 1.0;
    *maxs = -1.0;
    maxs[1] = -1.0;
    for (windingVertIndex = 0; windingVertIndex < vertexCount; ++windingVertIndex)
    {
        xyz = &(*winding)[3 * windingVertIndex];
        w = *xyz * dpvsGlob.viewProjMtx->m[0][3]
            + xyz[1] * dpvsGlob.viewProjMtx->m[1][3]
            + xyz[2] * dpvsGlob.viewProjMtx->m[2][3]
            + dpvsGlob.viewProjMtx->m[3][3];
        if (w < 0.125)
        {
            *mins = -1.0;
            mins[1] = -1.0;
            *maxs = 1.0;
            maxs[1] = 1.0;
            *clipChildren = DPVS_CLIP_CHILDREN;
            return;
        }
        x = *xyz * dpvsGlob.viewProjMtx->m[0][0]
            + xyz[1] * dpvsGlob.viewProjMtx->m[1][0]
            + xyz[2] * dpvsGlob.viewProjMtx->m[2][0]
            + dpvsGlob.viewProjMtx->m[3][0];
        y = *xyz * dpvsGlob.viewProjMtx->m[0][1]
            + xyz[1] * dpvsGlob.viewProjMtx->m[1][1]
            + xyz[2] * dpvsGlob.viewProjMtx->m[2][1]
            + dpvsGlob.viewProjMtx->m[3][1];
        invW = 1.0 / w;
        screenSpaceWinding[2 * windingVertIndex] = invW * x;
        screenSpaceWinding[2 * windingVertIndex + 1] = invW * y;
        if (screenSpaceWinding[2 * windingVertIndex] < (double)*mins)
            *mins = screenSpaceWinding[2 * windingVertIndex];
        if (screenSpaceWinding[2 * windingVertIndex] > (double)*maxs)
            *maxs = screenSpaceWinding[2 * windingVertIndex];
        if (screenSpaceWinding[2 * windingVertIndex + 1] < (double)mins[1])
            mins[1] = screenSpaceWinding[2 * windingVertIndex + 1];
        if (screenSpaceWinding[2 * windingVertIndex + 1] > (double)maxs[1])
            maxs[1] = screenSpaceWinding[2 * windingVertIndex + 1];
    }
    area = (*maxs - *mins) * (maxs[1] - mins[1]) * 0.25;
    if (area < 0.0)
    {
        v5 = R_PortalAssertMsg();
        v6 = va("area %g bounds [%g,%g]-[%g,%g]\n%s", area, *mins, mins[1], *maxs, maxs[1], v5);
        MyAssertHandler(".\\r_dpvs.cpp", 366, 0, "%s\n\t%s", "area >= 0", v6);
    }
    if (r_portalMinClipArea->current.value <= (double)area)
    {
        screenSpaceWinding[2 * vertexCount] = screenSpaceWinding[0];
        screenSpaceWinding[2 * vertexCount + 1] = screenSpaceWinding[1];
        screenSpaceWinding[2 * vertexCount + 2] = screenSpaceWinding[2];
        screenSpaceWinding[2 * vertexCount + 3] = screenSpaceWinding[3];
        areaa = 0.0;
        for (windingVertIndexa = 1; windingVertIndexa <= vertexCount; ++windingVertIndexa)
            areaa = (screenSpaceWinding[2 * windingVertIndexa + 3] - screenSpaceWinding[2 * windingVertIndexa - 1])
            * screenSpaceWinding[2 * windingVertIndexa]
            + areaa;
        areab = areaa * 0.125;
        if (areab < 0.0)
        {
            v7 = R_PortalAssertMsg();
            v8 = va("area %g bounds [%g,%g]-[%g,%g]\n%s", areab, *mins, mins[1], *maxs, maxs[1], v7);
            MyAssertHandler(".\\r_dpvs.cpp", 379, 0, "%s\n\t%s", "area >= 0", v8);
        }
        *clipChildren = (DpvsClipChildren)(r_portalMinClipArea->current.value <= (double)areab);
    }
    else
    {
        *clipChildren = DPVS_DONT_CLIP_CHILDREN;
    }
}

unsigned int __cdecl R_AddBevelPlanes(
    DpvsPlane *planes,
    unsigned int vertexCount,
    const float (*winding)[3],
    const float (*windingNormals)[3],
    float *mins,
    float *maxs,
    DpvsForceBevels forceBevels)
{
    float v8; // [esp+0h] [ebp-B0h]
    float v9; // [esp+4h] [ebp-ACh]
    DpvsPlane *a; // [esp+8h] [ebp-A8h]
    DpvsPlane *v11; // [esp+10h] [ebp-A0h]
    float *v12; // [esp+14h] [ebp-9Ch]
    unsigned int windingVertIndex; // [esp+1Ch] [ebp-94h]
    float projected[2]; // [esp+20h] [ebp-90h] BYREF
    float bevelVerts[5][3]; // [esp+28h] [ebp-88h] BYREF
    unsigned int planeCount; // [esp+64h] [ebp-4Ch]
    float bevelNormals[4][3]; // [esp+68h] [ebp-48h] BYREF
    float invW; // [esp+98h] [ebp-18h]
    unsigned int bevelVertIndex; // [esp+9Ch] [ebp-14h]
    float unprojected[4]; // [esp+A0h] [ebp-10h] BYREF

    for (bevelVertIndex = 0; bevelVertIndex < 4; ++bevelVertIndex)
    {
        if (bevelVertIndex >= 2)
            v9 = *maxs;
        else
            v9 = *mins;
        projected[0] = v9;
        if (bevelVertIndex == 1 || bevelVertIndex == 2)
            v8 = mins[1];
        else
            v8 = maxs[1];
        projected[1] = v8;
        R_UnprojectPoint(projected, unprojected);
        invW = 1.0 / unprojected[3];
        bevelVerts[bevelVertIndex][0] = unprojected[0] * invW;
        bevelVerts[bevelVertIndex][1] = unprojected[1] * invW;
        bevelVerts[bevelVertIndex][2] = unprojected[2] * invW;
    }
    bevelVerts[4][0] = bevelVerts[0][0];
    bevelVerts[4][1] = bevelVerts[0][1];
    bevelVerts[4][2] = bevelVerts[0][2];
    R_GetSidePlaneNormals(bevelVerts, 4u, bevelNormals);
    planeCount = 0;
    for (bevelVertIndex = 0; bevelVertIndex < 4; ++bevelVertIndex)
    {
        v11 = &planes[planeCount];
        v12 = bevelNormals[bevelVertIndex];
        v11->coeffs[0] = *v12;
        v11->coeffs[1] = v12[1];
        v11->coeffs[2] = v12[2];
        if (forceBevels == DPVS_DONT_FORCE_BEVELS)
        {
            for (windingVertIndex = 0; windingVertIndex < vertexCount; ++windingVertIndex)
            {
                if (r_portalBevels->current.value < Vec3Dot(
                    &(*windingNormals)[3 * windingVertIndex],
                    planes[planeCount].coeffs))
                {
                    if ((r_showPortals->current.integer & 2) != 0)
                        R_AddDebugLine(
                            &frontEndDataOut->debugGlobals,
                            bevelVerts[bevelVertIndex],
                            bevelVerts[bevelVertIndex + 1],
                            colorMdCyan);
                    goto LABEL_12;
                }
            }
        }
        if (r_showPortals->current.integer)
            R_AddDebugLine(
                &frontEndDataOut->debugGlobals,
                bevelVerts[bevelVertIndex],
                bevelVerts[bevelVertIndex + 1],
                colorLtCyan);
        a = &planes[planeCount];
        a->coeffs[3] = 0.001 - Vec3Dot(a->coeffs, bevelVerts[bevelVertIndex]);
        a->side[0] = COERCE_INT(a->coeffs[0]) <= 0 ? 0 : 0xC;
        a->side[1] = COERCE_INT(a->coeffs[1]) <= 0 ? 4 : 16;
        a->side[2] = COERCE_INT(a->coeffs[2]) <= 0 ? 8 : 20;
        ++planeCount;
    LABEL_12:
        ;
    }
    return planeCount;
}

void __cdecl R_UnprojectPoint(const float *projected, float *unprojected)
{
    *unprojected = dpvsGlob.invViewProjMtx->m[0][0] * *projected
        + dpvsGlob.invViewProjMtx->m[1][0] * projected[1]
        + dpvsGlob.invViewProjMtx->m[3][0];
    unprojected[1] = dpvsGlob.invViewProjMtx->m[0][1] * *projected
        + dpvsGlob.invViewProjMtx->m[1][1] * projected[1]
        + dpvsGlob.invViewProjMtx->m[3][1];
    unprojected[2] = dpvsGlob.invViewProjMtx->m[0][2] * *projected
        + dpvsGlob.invViewProjMtx->m[1][2] * projected[1]
        + dpvsGlob.invViewProjMtx->m[3][2];
    unprojected[3] = dpvsGlob.invViewProjMtx->m[0][3] * *projected
        + dpvsGlob.invViewProjMtx->m[1][3] * projected[1]
        + dpvsGlob.invViewProjMtx->m[3][3];
}

void __cdecl R_VisitPortalsForCell(
    const GfxCell *cell,
    GfxPortal *parentPortal,
    const DpvsPlane *parentPlane,
    const DpvsPlane *planes,
    int planeCount,
    int frustumPlaneCount,
    signed int recursionDepth,
    DpvsClipChildren clipChildren)
{
    unsigned __int8 v8; // [esp+0h] [ebp-24h]
    int vertCount; // [esp+8h] [ebp-1Ch]
    const float *verts; // [esp+Ch] [ebp-18h]
    int xCoord; // [esp+10h] [ebp-14h] BYREF
    int yCoord; // [esp+14h] [ebp-10h] BYREF
    float v13; // [esp+18h] [ebp-Ch]
    GfxPortal *portal; // [esp+1Ch] [ebp-8h]
    int portalIndex; // [esp+20h] [ebp-4h]

    R_AddCellSurfacesAndCullGroupsInFrustumDelayed(cell, planes, planeCount, frustumPlaneCount);
    R_SetCellVisible(cell);
    R_SetAncestorListStatus(parentPortal, 1);
    if (clipChildren)
    {
        if (clipChildren != DPVS_CLIP_CHILDREN)
            MyAssertHandler(
                ".\\r_dpvs.cpp",
                2679,
                0,
                "%s\n\t(clipChildren) = %i",
                "(clipChildren == DPVS_CLIP_CHILDREN)",
                clipChildren);
        for (portalIndex = 0; portalIndex < cell->portalCount; ++portalIndex)
        {
            portal = &cell->portals[portalIndex];
            if (!R_ShouldSkipPortal(portal, planes, planeCount))
            {
                if (dpvsGlob.viewOrgIsDir)
                    MyAssertHandler(".\\r_dpvs.cpp", 2687, 0, "%s", "!dpvsGlob.viewOrgIsDir");
                v13 = Vec4Dot(portal->plane.coeffs, dpvsGlob.viewOrg);
                if (v13 <= -0.125)
                {
                    if (R_ChopPortalAndAddHullPoints(portal, parentPlane, planes, planeCount))
                    {
                        if (portal->writable.isQueued)
                        {
                            if (portal->writable.recursionDepth < recursionDepth)
                                v8 = portal->writable.recursionDepth;
                            else
                                v8 = recursionDepth;
                            portal->writable.recursionDepth = v8;
                            if (portal->writable.queuedParent != parentPortal)
                                portal->writable.queuedParent = 0;
                        }
                        else
                        {
                            portal->writable.recursionDepth = recursionDepth;
                            portal->writable.queuedParent = parentPortal;
                            R_EnqueuePortal(portal);
                        }
                    }
                }
                else
                {
                    if (portal->writable.isAncestor)
                        MyAssertHandler(".\\r_dpvs.cpp", 2690, 1, "%s", "!portal->writable.isAncestor");
                    if (portal->writable.isQueued)
                        MyAssertHandler(".\\r_dpvs.cpp", 2691, 1, "%s", "!portal->writable.isQueued");
                    vertCount = portal->vertexCount;
                    verts = (const float *)portal->vertices;
                    Vec3ProjectionCoords(portal->plane.coeffs, &xCoord, &yCoord);
                    if (ProjectedWindingContainsCoplanarPoint(
                        (const float (*)[3])verts,
                        vertCount,
                        xCoord,
                        yCoord,
                        dpvsGlob.viewOrg))
                    {
                        portal->writable.queuedParent = 0;
                        R_VisitPortalsForCell(
                            portal->cell,
                            portal,
                            &portal->plane,
                            planes,
                            planeCount,
                            frustumPlaneCount,
                            portal->writable.recursionDepth + 1,
                            clipChildren);
                    }
                    if (portal->writable.isAncestor)
                        MyAssertHandler(".\\r_dpvs.cpp", 2699, 1, "%s", "!portal->writable.isAncestor");
                    if (parentPortal)
                    {
                        if (!parentPortal->writable.isAncestor)
                            MyAssertHandler(
                                ".\\r_dpvs.cpp",
                                2700,
                                1,
                                "%s",
                                "parentPortal == NULL || parentPortal->writable.isAncestor");
                    }
                }
            }
        }
        R_SetAncestorListStatus(parentPortal, 0);
    }
    else
    {
        R_VisitAllFurtherCells(cell, parentPlane, planes, planeCount, frustumPlaneCount);
        R_SetAncestorListStatus(parentPortal, 0);
    }
}

char __cdecl R_ChopPortalAndAddHullPoints(
    GfxPortal *portal,
    const DpvsPlane *parentPlane,
    const DpvsPlane *planes,
    int planeCount)
{
    int vertCount; // [esp+0h] [ebp-C10h]
    int vertIndex; // [esp+4h] [ebp-C0Ch]
    float v[2][128][3]; // [esp+8h] [ebp-C08h] BYREF
    const float (*w)[3]; // [esp+C0Ch] [ebp-4h] BYREF

    vertCount = R_ChopPortal(portal, parentPlane, planes, planeCount, v, &w);
    if (!vertCount)
        return 0;
    for (vertIndex = 0; vertIndex < vertCount; ++vertIndex)
        R_AddVertToPortalHullPoints(portal, w[vertIndex]);
    return 1;
}

void __cdecl R_VisitAllFurtherCells(
    const GfxCell *cell,
    const DpvsPlane *parentPlane,
    const DpvsPlane *planes,
    int planeCount,
    unsigned __int8 frustumPlaneCount)
{
    int i; // [esp+0h] [ebp-1C0Ch]
    GfxCell *list[1025]; // [esp+4h] [ebp-1C08h] BYREF
    int FurtherCellList_r; // [esp+1008h] [ebp-C04h]
    float v[256][3]; // [esp+100Ch] [ebp-C00h] BYREF

    FurtherCellList_r = R_GetFurtherCellList_r(
        cell,
        parentPlane,
        planes,
        planeCount,
        (float (*)[128][3])v,
        (const GfxCell **)list,
        0);
    for (i = 0; i < FurtherCellList_r; ++i)
    {
        R_AddCellSurfacesAndCullGroupsInFrustumDelayed(list[i], planes, planeCount, frustumPlaneCount);
        R_SetCellVisible(list[i]);
    }
}

int __cdecl R_GetFurtherCellList_r(
    const GfxCell *cell,
    const DpvsPlane *parentPlane,
    const DpvsPlane *planes,
    int planeCount,
    float (*v)[128][3],
    const GfxCell **list,
    int count)
{
    int v7; // eax
    const GfxPortal *portal; // [esp+0h] [ebp-8h]
    int portalIndex; // [esp+4h] [ebp-4h]

    for (portalIndex = 0; portalIndex < cell->portalCount; ++portalIndex)
    {
        portal = &cell->portals[portalIndex];
        if (!R_IsCellInList(portal->cell, list, count) && !R_ShouldSkipPortal(portal, planes, planeCount))
        {
            if (R_ChopPortal(portal, parentPlane, planes, planeCount, v, 0))
            {
                v7 = R_AddCellToList(portal->cell, list, count);
                count = R_GetFurtherCellList_r(portal->cell, parentPlane, planes, planeCount, v, list, v7);
            }
        }
    }
    return count;
}

char __cdecl R_IsCellInList(const GfxCell *cell, const GfxCell **list, int count)
{
    int index; // [esp+0h] [ebp-4h]

    for (index = 0; index < count; ++index)
    {
        if (cell == list[index])
            return 1;
    }
    return 0;
}

int __cdecl R_AddCellToList(const GfxCell *cell, const GfxCell **list, int count)
{
    list[count] = cell;
    return count + 1;
}

void __cdecl R_SetupShadowSurfacesDpvs(
    const GfxViewParms *viewParms,
    const float (*sidePlanes)[4],
    unsigned int sidePlaneCount,
    int partitionIndex)
{
    DpvsPlane *v4; // ecx
    DpvsPlane *v5; // edx
    DpvsPlane *v6; // eax
    DpvsPlane *v7; // ecx
    DpvsPlane *v8; // eax
    DpvsPlane *v9; // ecx
    DpvsView *dpvsView; // [esp+0h] [ebp-8h]
    unsigned int planeIndex; // [esp+4h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_dpvs.cpp", 3384, 0, "%s", "Sys_IsMainThread()");
    if (!rgp.world)
        MyAssertHandler(".\\r_dpvs.cpp", 3385, 0, "%s", "rgp.world");
    if (!viewParms)
        MyAssertHandler(".\\r_dpvs.cpp", 3386, 0, "%s", "viewParms");
    dpvsView = &dpvsGlob.views[scene.dpvs.localClientNum][partitionIndex + 1];
    dpvsView->renderFxFlagsCull = 1;
    if (sidePlaneCount > 0xE)
        MyAssertHandler(
            ".\\r_dpvs.cpp",
            3392,
            0,
            "%s\n\t(sidePlaneCount) = %i",
            "(sidePlaneCount <= (sizeof( dpvsView->frustumPlanes ) / (sizeof( dpvsView->frustumPlanes[0] ) * (sizeof( dpvsView-"
            ">frustumPlanes ) != 4 || sizeof( dpvsView->frustumPlanes[0] ) <= 4))))",
            sidePlaneCount);
    R_FrustumClipPlanes(&viewParms->viewProjectionMatrix, sidePlanes, sidePlaneCount, dpvsView->frustumPlanes);
    if (Vec3Dot(viewParms->axis[0], scene.shadowNearPlane[partitionIndex].coeffs) < 0.0)
    {
        if (sidePlaneCount >= 0xE)
            MyAssertHandler(
                ".\\r_dpvs.cpp",
                3397,
                0,
                "%s\n\t(sidePlaneCount) = %i",
                "(sidePlaneCount < (sizeof( dpvsView->frustumPlanes ) / (sizeof( dpvsView->frustumPlanes[0] ) * (sizeof( dpvsView"
                "->frustumPlanes ) != 4 || sizeof( dpvsView->frustumPlanes[0] ) <= 4))))",
                sidePlaneCount);
        v4 = &scene.shadowNearPlane[partitionIndex];
        v5 = &dpvsView->frustumPlanes[sidePlaneCount];
        v5->coeffs[0] = v4->coeffs[0];
        v5->coeffs[1] = v4->coeffs[1];
        v5->coeffs[2] = v4->coeffs[2];
        v5->coeffs[3] = v4->coeffs[3];
        *(unsigned int *)v5->side = *(unsigned int *)v4->side;
        ++sidePlaneCount;
    }
    if ((!partitionIndex || !rg.sunShadowFull)
        && Vec3Dot(viewParms->axis[0], scene.shadowFarPlane[partitionIndex].coeffs) < 0.0)
    {
        if (sidePlaneCount >= 0xE)
            MyAssertHandler(
                ".\\r_dpvs.cpp",
                3406,
                0,
                "%s\n\t(sidePlaneCount) = %i",
                "(sidePlaneCount < (sizeof( dpvsView->frustumPlanes ) / (sizeof( dpvsView->frustumPlanes[0] ) * (sizeof( dpvsView"
                "->frustumPlanes ) != 4 || sizeof( dpvsView->frustumPlanes[0] ) <= 4))))",
                sidePlaneCount);
        v6 = &scene.shadowFarPlane[partitionIndex];
        v7 = &dpvsView->frustumPlanes[sidePlaneCount];
        v7->coeffs[0] = v6->coeffs[0];
        v7->coeffs[1] = v6->coeffs[1];
        v7->coeffs[2] = v6->coeffs[2];
        v7->coeffs[3] = v6->coeffs[3];
        *(unsigned int *)v7->side = *(unsigned int *)v6->side;
        ++sidePlaneCount;
    }
    for (planeIndex = 0; planeIndex < 4; ++planeIndex)
    {
        if (Vec3Dot(viewParms->axis[0], dpvsGlob.sideFrustumPlanes[planeIndex].coeffs) < 0.0)
        {
            if (sidePlaneCount >= 0xE)
                MyAssertHandler(
                    ".\\r_dpvs.cpp",
                    3416,
                    0,
                    "%s\n\t(sidePlaneCount) = %i",
                    "(sidePlaneCount < (sizeof( dpvsView->frustumPlanes ) / (sizeof( dpvsView->frustumPlanes[0] ) * (sizeof( dpvsVi"
                    "ew->frustumPlanes ) != 4 || sizeof( dpvsView->frustumPlanes[0] ) <= 4))))",
                    sidePlaneCount);
            v8 = &dpvsGlob.sideFrustumPlanes[planeIndex];
            v9 = &dpvsView->frustumPlanes[sidePlaneCount];
            v9->coeffs[0] = v8->coeffs[0];
            v9->coeffs[1] = v8->coeffs[1];
            v9->coeffs[2] = v8->coeffs[2];
            v9->coeffs[3] = v8->coeffs[3];
            *(unsigned int *)v9->side = *(unsigned int *)v8->side;
            ++sidePlaneCount;
        }
    }
    dpvsView->frustumPlaneCount = sidePlaneCount;
}

double __cdecl R_GetFarPlaneDist()
{
    if (r_zfar->current.value == 0.0)
        return dpvsGlob.cullDist;
    else
        return r_zfar->current.value;
}

unsigned int __cdecl R_CalcReflectionProbeIndex(const GfxWorld *world, const float *origin)
{
    unsigned int cellIndex; // [esp+0h] [ebp-4h]

    cellIndex = R_CellForPoint(world, origin);
    if (cellIndex == -1)
        return R_FindNearestReflectionProbe(world, origin);
    if (cellIndex >= world->dpvsPlanes.cellCount)
        MyAssertHandler(
            ".\\r_staticmodel_load_obj.cpp",
            552,
            0,
            "cellIndex doesn't index world->dpvsPlanes.cellCount\n\t%i not in [0, %i)",
            cellIndex,
            world->dpvsPlanes.cellCount);
    return R_FindNearestReflectionProbeInCell(world, &world->cells[cellIndex], origin);
}

int __cdecl R_CellForPoint(const GfxWorld *world, const float *origin)
{
    mnode_t *node; // [esp+4h] [ebp-1Ch]
    cplane_s *plane; // [esp+Ch] [ebp-14h]
    float d; // [esp+10h] [ebp-10h]
    int cellIndex; // [esp+14h] [ebp-Ch]
    int cellCount; // [esp+18h] [ebp-8h]

    if (!world)
        MyAssertHandler(".\\r_staticmodel_load_obj.cpp", 522, 0, "%s", "world");
    node = (mnode_t *)world->dpvsPlanes.nodes;
    cellCount = world->dpvsPlanes.cellCount + 1;
    while (1)
    {
        cellIndex = node->cellIndex;
        if (cellIndex - cellCount < 0)
            break;
        plane = &world->dpvsPlanes.planes[cellIndex - cellCount];
        d = Vec3Dot(origin, plane->normal) - plane->dist;
        node = (mnode_t *)((char *)node + 2 * (d <= 0.0) * (node->rightChildOffset - 2) + 4);
    }
    return cellIndex - 1;
}

void __cdecl R_FreeHullPoints(GfxHullPointsPool *hullPoints)
{
    hullPoints->nextFree = dpvsGlob.nextFreeHullPoints;
    dpvsGlob.nextFreeHullPoints = hullPoints;
}

