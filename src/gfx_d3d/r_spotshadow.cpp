#include "r_spotshadow.h"
#include "r_dobj_skin.h"
#include "r_dpvs.h"
#include "r_dvars.h"
#include "r_primarylights.h"
#include "r_workercmds.h"
#include <qcommon/com_bsp.h>
#include "r_bsp.h"
#include "r_model_pose.h"
#include "r_meshdata.h"
#include "r_light.h"
#include "r_pretess.h"
#include "r_add_staticmodel.h"
#include "r_cmdbuf.h"
#include "r_utils.h"
#include "r_state.h"
#include "r_draw_sunshadow.h"
#include <universal/profile.h>

#ifdef KISAK_MP
#include <cgame_mp/cg_local_mp.h>
#elif KISAK_SP
#include <cgame/cg_local.h>
#endif

void __cdecl R_AddSpotShadowEntCmd(const GfxSpotShadowEntCmd *data)
{
    DObjAnimMat *boneMatrix; // [esp+Ch] [ebp-30h]
    const DObj_s *obj; // [esp+10h] [ebp-2Ch] BYREF
    GfxSceneEntity *localSceneEnt; // [esp+14h] [ebp-28h] BYREF
    const GfxLight *localLight; // [esp+18h] [ebp-24h]
    float boxHalfSize[3]; // [esp+1Ch] [ebp-20h] BYREF
    GfxSceneEntity *sceneEnt; // [esp+28h] [ebp-14h]
    float boxCenter[3]; // [esp+2Ch] [ebp-10h] BYREF
    const GfxSpotShadowEntCmd *cmd; // [esp+38h] [ebp-4h]

    cmd = data;
    sceneEnt = data->sceneEnt;
    boneMatrix = R_UpdateSceneEntBounds(sceneEnt, &localSceneEnt, &obj, 1);
    if (boneMatrix)
    {
        iassert( localSceneEnt );
        localLight = cmd->light;
        Vec3Avg(localSceneEnt->cull.mins, localSceneEnt->cull.maxs, boxCenter);
        Vec3Sub(boxCenter, localSceneEnt->cull.mins, boxHalfSize);
        if (CullBoxFromCone(localLight->origin, localLight->dir, localLight->cosHalfFovOuter, boxCenter, boxHalfSize))
        {
            CG_UsedDObjCalcPose(localSceneEnt->info.pose);
        }
        else
        {
            CG_CullIn(localSceneEnt->info.pose);
            R_SkinSceneDObj(sceneEnt, localSceneEnt, obj, boneMatrix, 0);
            R_AddSceneDObj(localSceneEnt->entnum, localLight->spotShadowIndex + 3);
        }
    }
    else if (localSceneEnt)
    {
        CG_UsedDObjCalcPose(localSceneEnt->info.pose);
    }
}

char __cdecl R_AddSpotShadowsForLight(
    GfxViewInfo *viewInfo,
    GfxLight *light,
    unsigned int shadowableLightIndex,
    float spotShadowFade)
{
    float nearPlaneBias; // [esp+4h] [ebp-20h]
    BOOL useQualitySpotShadow; // [esp+8h] [ebp-1Ch]
    unsigned int tileCount; // [esp+18h] [ebp-Ch]
    unsigned int spotShadowIndex; // [esp+20h] [ebp-4h]

    iassert(light);
    iassert(light->type == GFX_LIGHT_TYPE_SPOT || light->type == GFX_LIGHT_TYPE_OMNI);

    spotShadowIndex = viewInfo->spotShadowCount;
    bcassert(spotShadowIndex, R_SPOTSHADOW_TILE_COUNT);

    ++viewInfo->spotShadowCount;
    light->spotShadowIndex = spotShadowIndex;

    iassert(shadowableLightIndex == (unsigned __int8)shadowableLightIndex);

    viewInfo->spotShadows[spotShadowIndex].shadowableLightIndex = shadowableLightIndex;
    viewInfo->spotShadows[spotShadowIndex].light = light;
    viewInfo->spotShadows[spotShadowIndex].fade = spotShadowFade;
    useQualitySpotShadow = sm_qualitySpotShadow->current.enabled && !Com_BitCheckAssert(frontEndDataOut->shadowableLightHasShadowMap, rgp.world->sunPrimaryLightIndex, 32);
    if (useQualitySpotShadow && spotShadowIndex < 2)
    {
        viewInfo->spotShadows[spotShadowIndex].viewport.x = 0;
        viewInfo->spotShadows[spotShadowIndex].viewport.y = spotShadowIndex << 10;
        viewInfo->spotShadows[spotShadowIndex].viewport.width = 1024;
        viewInfo->spotShadows[spotShadowIndex].viewport.height = 1024;
        viewInfo->spotShadows[spotShadowIndex].image = gfxRenderTargets[R_RENDERTARGET_SHADOWMAP_SUN].image;
        viewInfo->spotShadows[spotShadowIndex].renderTargetId = R_RENDERTARGET_SHADOWMAP_SUN;
        viewInfo->spotShadows[spotShadowIndex].pixelAdjust[0] = (1.0f / 4096.0f);
        viewInfo->spotShadows[spotShadowIndex].pixelAdjust[1] = (1.0f / 4096.0f);
        viewInfo->spotShadows[spotShadowIndex].pixelAdjust[2] = (1.0f / 2048.0f);
        viewInfo->spotShadows[spotShadowIndex].pixelAdjust[3] = -(1.0f / 8192.0f);
        viewInfo->spotShadows[spotShadowIndex].clearScreen = spotShadowIndex == 0;
        viewInfo->spotShadows[spotShadowIndex].clearMesh = &gfxMeshGlob.sunShadowClearMeshData[spotShadowIndex];
        tileCount = 2;
    }
    else
    {
        viewInfo->spotShadows[spotShadowIndex].viewport.x = 0;
        viewInfo->spotShadows[spotShadowIndex].viewport.y = spotShadowIndex << 9;
        viewInfo->spotShadows[spotShadowIndex].viewport.width = 512;
        viewInfo->spotShadows[spotShadowIndex].viewport.height = 512;
        viewInfo->spotShadows[spotShadowIndex].image = gfxRenderTargets[R_RENDERTARGET_SHADOWMAP_SPOT].image;
        viewInfo->spotShadows[spotShadowIndex].renderTargetId = R_RENDERTARGET_SHADOWMAP_SPOT;
        viewInfo->spotShadows[spotShadowIndex].pixelAdjust[0] = (1.0f / 2048.0f);
        viewInfo->spotShadows[spotShadowIndex].pixelAdjust[1] = (1.0f / 4096.0f);
        viewInfo->spotShadows[spotShadowIndex].pixelAdjust[2] = (1.0f / 1024.0f);
        viewInfo->spotShadows[spotShadowIndex].pixelAdjust[3] = -(1.0f / 8192.0f);
        if (useQualitySpotShadow)
            viewInfo->spotShadows[spotShadowIndex].clearScreen = spotShadowIndex == 2;
        else
            viewInfo->spotShadows[spotShadowIndex].clearScreen = spotShadowIndex == 0;
        viewInfo->spotShadows[spotShadowIndex].clearMesh = &gfxMeshGlob.spotShadowClearMeshData[spotShadowIndex];
        tileCount = 4;
    }
    if (R_IsPrimaryLight(shadowableLightIndex))
        nearPlaneBias = 0.0f;
    else
        nearPlaneBias = scene.dynamicSpotLightNearPlaneOffset;
    R_SetViewParmsForLight(light, &viewInfo->spotShadows[spotShadowIndex].shadowViewParms, nearPlaneBias);
    R_GetSpotShadowLookupMatrix(
        &viewInfo->spotShadows[spotShadowIndex].shadowViewParms,
        spotShadowIndex,
        tileCount,
        &viewInfo->spotShadows[spotShadowIndex].lookupMatrix);
    if (R_IsPrimaryLight(shadowableLightIndex))
        R_AddSpotShadowModelEntities(viewInfo->localClientNum, shadowableLightIndex, light);
    return 1;
}

void __cdecl R_SetViewParmsForLight(const GfxLight *light, GfxViewParms *viewParms, float nearPlaneBias)
{
    float zNear; // [esp+10h] [ebp-24h]
    float v4; // [esp+14h] [ebp-20h]
    float v5; // [esp+1Ch] [ebp-18h]
    float tanHalfFov; // [esp+30h] [ebp-4h]

    viewParms->axis[0][0] = -light->dir[0];
    viewParms->axis[0][1] = -light->dir[1];
    viewParms->axis[0][2] = -light->dir[2];

    PerpendicularVector(viewParms->axis[0], viewParms->axis[2]);
    Vec3Cross(viewParms->axis[2], viewParms->axis[0], viewParms->axis[1]);

    viewParms->origin[0] = light->origin[0];
    viewParms->origin[1] = light->origin[1];
    viewParms->origin[2] = light->origin[2];
    viewParms->origin[3] = 1.0;

    MatrixForViewer(viewParms->viewMatrix.m, viewParms->origin, viewParms->axis);

    iassert(light->cosHalfFovOuter > 0.0f && light->cosHalfFovOuter < 1.0f);

    v5 = 1.0 - light->cosHalfFovOuter * light->cosHalfFovOuter;
    v4 = sqrt(v5);
    tanHalfFov = v4 / light->cosHalfFovOuter;
    zNear = nearPlaneBias + 1.0;
    FinitePerspectiveMatrix(viewParms->projectionMatrix.m, tanHalfFov, tanHalfFov, zNear, light->radius);
    R_SetupViewProjectionMatrices(viewParms);
    viewParms->depthHackNearClip = viewParms->projectionMatrix.m[3][2];
}

void __cdecl R_GetSpotShadowLookupMatrix(
    const GfxViewParms *shadowViewParms,
    unsigned int spotShadowIndex,
    unsigned int tileCount,
    GfxMatrix *lookupMatrix)
{
    float xScale; // [esp+1Ch] [ebp-1Ch]
    float xShift; // [esp+24h] [ebp-14h]
    float yShift; // [esp+28h] [ebp-10h]
    float y1; // [esp+2Ch] [ebp-Ch]
    float y0; // [esp+30h] [ebp-8h]
    float yScale; // [esp+34h] [ebp-4h]

    iassert( tileCount );
    y1 = (double)spotShadowIndex * (1.0 / (double)tileCount);
    y0 = 1.0 / (double)tileCount + y1;
    xScale = ((float)1.0 - (float)0.0) * 0.5;
    xShift = ((float)1.0 + (float)0.0) * 0.5;
    yScale = (y1 - y0) * 0.5;
    yShift = (y1 + y0) * 0.5;
    lookupMatrix->m[0][0] = shadowViewParms->viewProjectionMatrix.m[0][0] * xScale
        + shadowViewParms->viewProjectionMatrix.m[0][3] * xShift;
    lookupMatrix->m[1][0] = shadowViewParms->viewProjectionMatrix.m[1][0] * xScale
        + shadowViewParms->viewProjectionMatrix.m[1][3] * xShift;
    lookupMatrix->m[2][0] = shadowViewParms->viewProjectionMatrix.m[2][0] * xScale
        + shadowViewParms->viewProjectionMatrix.m[2][3] * xShift;
    lookupMatrix->m[3][0] = shadowViewParms->viewProjectionMatrix.m[3][0] * xScale
        + shadowViewParms->viewProjectionMatrix.m[3][3] * xShift;
    lookupMatrix->m[0][1] = shadowViewParms->viewProjectionMatrix.m[0][1] * yScale
        + shadowViewParms->viewProjectionMatrix.m[0][3] * yShift;
    lookupMatrix->m[1][1] = shadowViewParms->viewProjectionMatrix.m[1][1] * yScale
        + shadowViewParms->viewProjectionMatrix.m[1][3] * yShift;
    lookupMatrix->m[2][1] = shadowViewParms->viewProjectionMatrix.m[2][1] * yScale
        + shadowViewParms->viewProjectionMatrix.m[2][3] * yShift;
    lookupMatrix->m[3][1] = shadowViewParms->viewProjectionMatrix.m[3][1] * yScale
        + shadowViewParms->viewProjectionMatrix.m[3][3] * yShift;
    lookupMatrix->m[0][2] = shadowViewParms->viewProjectionMatrix.m[0][2];
    lookupMatrix->m[1][2] = shadowViewParms->viewProjectionMatrix.m[1][2];
    lookupMatrix->m[2][2] = shadowViewParms->viewProjectionMatrix.m[2][2];
    lookupMatrix->m[3][2] = shadowViewParms->viewProjectionMatrix.m[3][2];
    lookupMatrix->m[0][3] = shadowViewParms->viewProjectionMatrix.m[0][3];
    lookupMatrix->m[1][3] = shadowViewParms->viewProjectionMatrix.m[1][3];
    lookupMatrix->m[2][3] = shadowViewParms->viewProjectionMatrix.m[2][3];
    lookupMatrix->m[3][3] = shadowViewParms->viewProjectionMatrix.m[3][3];
}

void __cdecl R_AddSpotShadowModelEntities(
    unsigned int localClientNum,
    unsigned int primaryLightIndex,
    const GfxLight *light)
{
    volatile int sceneEntIndex; // [esp+8h] [ebp-14h]
    volatile int sceneEntIndexa; // [esp+8h] [ebp-14h]
    unsigned int entnum; // [esp+10h] [ebp-Ch]
    unsigned int entnuma; // [esp+10h] [ebp-Ch]
    GfxSpotShadowEntCmd cmd; // [esp+14h] [ebp-8h] BYREF

    cmd.light = light;
    for (sceneEntIndex = 0; sceneEntIndex < scene.sceneDObjCount; ++sceneEntIndex)
    {
        cmd.sceneEnt = &scene.sceneDObj[sceneEntIndex];
        if (!cmd.sceneEnt->gfxEntIndex
            || (frontEndDataOut->gfxEnts[scene.sceneDObj[sceneEntIndex].gfxEntIndex].renderFxFlags & 1) == 0)
        {
            entnum = cmd.sceneEnt->entnum;
            if (entnum != gfxCfg.entnumNone)
            {
                if (R_IsEntityVisibleToPrimaryLight(localClientNum, entnum, primaryLightIndex))
                    R_AddWorkerCmd(WRKCMD_SPOT_SHADOW_ENT, (unsigned __int8 *)&cmd);
            }
        }
    }
    for (sceneEntIndexa = 0; sceneEntIndexa < scene.sceneModelCount; ++sceneEntIndexa)
    {
        if (!scene.sceneModel[sceneEntIndexa].gfxEntIndex
            || (frontEndDataOut->gfxEnts[scene.sceneModel[sceneEntIndexa].gfxEntIndex].renderFxFlags & 1) == 0)
        {
            entnuma = scene.sceneModel[sceneEntIndexa].entnum;
            if (entnuma != gfxCfg.entnumNone)
            {
                if (R_IsEntityVisibleToPrimaryLight(localClientNum, entnuma, primaryLightIndex))
                    scene.dpvs.entVisData[light->spotShadowIndex + 3][entnuma] = 1;
            }
        }
    }
}

void __cdecl R_GenerateAllSortedSpotShadowDrawSurfs(GfxViewInfo *viewInfo)
{
    unsigned int spotShadowIndex; // [esp+4h] [ebp-4h]

    for (spotShadowIndex = 0; spotShadowIndex < viewInfo->spotShadowCount; ++spotShadowIndex)
    {
        if (R_IsPrimaryLight(viewInfo->spotShadows[spotShadowIndex].shadowableLightIndex))
            R_GenerateSortedPrimarySpotShadowDrawSurfs(
                viewInfo,
                spotShadowIndex,
                viewInfo->spotShadows[spotShadowIndex].shadowableLightIndex);
    }
    R_EmitSpotShadowMapSurfs(viewInfo);
}

void __cdecl R_GenerateSortedPrimarySpotShadowDrawSurfs(
    const GfxViewInfo *viewInfo,
    unsigned int spotShadowIndex,
    unsigned int shadowableLightIndex)
{
    bcassert(shadowableLightIndex, Com_GetPrimaryLightCount());

    {
        PROF_SCOPED("bsp surfaces");
        R_AddAllBspDrawSurfacesSpotShadow(spotShadowIndex, shadowableLightIndex);
    }
    {
        PROF_SCOPED("static model surfaces");
        R_AddAllStaticModelSurfacesSpotShadow(spotShadowIndex, shadowableLightIndex);
    }
    {
        PROF_SCOPED("scene ent surfaces");
        R_AddAllSceneEntSurfacesSpotShadow(viewInfo, spotShadowIndex, shadowableLightIndex);
    }
}

void __cdecl R_EmitSpotShadowMapSurfs(GfxViewInfo *viewInfo)
{
    const float *origin; // [esp+20h] [ebp-28h]
    int firstDrawSurf; // [esp+38h] [ebp-10h]
    GfxDrawSurfListInfo *info; // [esp+3Ch] [ebp-Ch]
    unsigned int spotShadowIndex; // [esp+44h] [ebp-4h]

    KISAK_NULLSUB();
    for (spotShadowIndex = 0; spotShadowIndex < viewInfo->spotShadowCount; ++spotShadowIndex)
    {
        PROF_SCOPED("EmitSpotShadow");
        info = &viewInfo->spotShadows[spotShadowIndex].info;
        R_InitDrawSurfListInfo(info);
        info->baseTechType = gfxMetrics.shadowmapBuildTechType;
        info->viewInfo = viewInfo;
        origin = viewInfo->spotShadows[spotShadowIndex].light->origin;
        info->viewOrigin[0] = *origin;
        info->viewOrigin[1] = origin[1];
        info->viewOrigin[2] = origin[2];
        info->viewOrigin[3] = 1.0;
        iassert( !info->cameraView );
        firstDrawSurf = frontEndDataOut->drawSurfCount;
        if (R_IsPrimaryLight(viewInfo->spotShadows[spotShadowIndex].shadowableLightIndex))
        {
            DrawSurfType bspSpotShadowDrawType = (DrawSurfType)((int)DRAW_SURF_BSP_SPOTSHADOW_0 + (3 * spotShadowIndex));
            DrawSurfType smodelSpotShadowDrawType = (DrawSurfType)((int)DRAW_SURF_SMODEL_SPOTSHADOW_0 + (3 * spotShadowIndex));
            R_MergeAndEmitDrawSurfLists(bspSpotShadowDrawType, 1);
            R_MergeAndEmitDrawSurfLists(smodelSpotShadowDrawType, 2);
            viewInfo->spotShadows[spotShadowIndex].info.drawSurfs = &frontEndDataOut->drawSurfs[firstDrawSurf];
            viewInfo->spotShadows[spotShadowIndex].info.drawSurfCount = frontEndDataOut->drawSurfCount - firstDrawSurf;
        }
        else
        {
            if (!R_IsDynamicShadowedLight(viewInfo->spotShadows[spotShadowIndex].shadowableLightIndex))
                MyAssertHandler(
                    ".\\r_spotshadow.cpp",
                    318,
                    0,
                    "%s",
                    "R_IsDynamicShadowedLight( spotShadow->shadowableLightIndex )");
            if (viewInfo->emissiveSpotLightCount == 1)
                R_EmitShadowedLightPartitionSurfs(
                    viewInfo,
                    viewInfo->emissiveSpotDrawSurfCount,
                    viewInfo->emissiveSpotDrawSurfs,
                    info);
        }
    }
}

unsigned int R_InitSpotShadowMeshes()
{
    unsigned int result; // eax
    unsigned int sunShadowIndex; // [esp+24h] [ebp-10h]
    float x; // [esp+28h] [ebp-Ch]
    float y; // [esp+2Ch] [ebp-8h]
    float ya; // [esp+2Ch] [ebp-8h]
    unsigned int spotShadowIndex; // [esp+30h] [ebp-4h]

    x = 0.0;
    y = 0.0;
    for (spotShadowIndex = 0; spotShadowIndex < 4; ++spotShadowIndex)
    {
        R_InitDynamicMesh(&gfxMeshGlob.spotShadowClearMeshData[spotShadowIndex], 6u, 4u, 0x20u);
        R_SetQuadMeshData(
            &gfxMeshGlob.spotShadowClearMeshData[spotShadowIndex],
            x,
            y,
            512.0,
            512.0,
            0.0,
            0.0,
            1.0,
            1.0,
            0xFFFFFFFF);
        y = y + 512.0;
        result = spotShadowIndex + 1;
    }
    ya = 0.0;
    for (sunShadowIndex = 0; sunShadowIndex < 2; ++sunShadowIndex)
    {
        R_InitDynamicMesh(&gfxMeshGlob.sunShadowClearMeshData[sunShadowIndex], 6u, 4u, 0x20u);
        R_SetQuadMeshData(
            &gfxMeshGlob.sunShadowClearMeshData[sunShadowIndex],
            x,
            ya,
            1024.0,
            1024.0,
            0.0,
            0.0,
            1.0,
            1.0,
            0xFFFFFFFF);
        ya = ya + 1024.0;
        result = sunShadowIndex + 1;
    }
    return result;
}

void __cdecl R_ShutdownSpotShadowMeshes()
{
    unsigned int sunShadowIndex; // [esp+0h] [ebp-8h]
    unsigned int spotShadowIndex; // [esp+4h] [ebp-4h]

    for (spotShadowIndex = 0; spotShadowIndex < 4; ++spotShadowIndex)
        R_ShutdownDynamicMesh(&gfxMeshGlob.spotShadowClearMeshData[spotShadowIndex]);
    for (sunShadowIndex = 0; sunShadowIndex < 2; ++sunShadowIndex)
        R_ShutdownDynamicMesh(&gfxMeshGlob.sunShadowClearMeshData[sunShadowIndex]);
}

void __cdecl R_DrawSpotShadowMapCallback(
    const void *userData,
    GfxCmdBufContext context,
    GfxCmdBufContext prepassContext)
{
    GfxSpotShadow *shadow = (GfxSpotShadow *)userData;

    R_SetRenderTarget(context, shadow->renderTargetId);

    if (shadow->clearScreen)
        R_ClearScreen(context.state->prim.device, 3u, shadowmapClearColor, 1.0, 0, 0);

    if (!gfxMetrics.hasHardwareShadowmap)
        R_DrawQuadMesh(context, rgp.shadowClearMaterial, shadow->clearMesh);

    R_DrawSurfs(context, 0, &shadow->info);
}
void __cdecl R_DrawSpotShadowMapArray(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf)
{
    GfxCmdBufSourceState state; // [sp+50h] [-F10h] BYREF

    R_InitCmdBufSourceState(&state, &viewInfo->input, 0);

    for (int i = 0; i < viewInfo->spotShadowCount; i++)
    {
        const GfxSpotShadow *spotShadow = &viewInfo->spotShadows[i];

        R_SetRenderTargetSize(&state, spotShadow->renderTargetId);
        R_UpdateCodeConstant(&state, CONST_SRC_CODE_SHADOWMAP_POLYGON_OFFSET, (sm_polygonOffsetBias->current.value * 0.25f), sm_polygonOffsetScale->current.value, 0.0f, 0.0f);
        R_SetViewportValues(&state, spotShadow->viewport.x, spotShadow->viewport.y, spotShadow->viewport.width, spotShadow->viewport.height);

        R_DrawCall(
            R_DrawSpotShadowMapCallback,
            spotShadow,
            &state,
            viewInfo,
            &spotShadow->info,
            &spotShadow->shadowViewParms,
            cmdBuf,
            0);
    }
}

void RB_SpotShadowMaps(const GfxBackEndData *data, const GfxViewInfo *viewInfo)
{
    GfxCmdBuf v3[4]; // [sp+50h] [-20h] BYREF

    R_InitContext(data, v3);
    R_DrawSpotShadowMapArray(viewInfo, v3);
}