#include "r_scene.h"
#include <qcommon/mem_track.h>
#include "r_init.h"
#include "r_debug.h"
#include "r_dvars.h"
#include <qcommon/threads.h>
#include <xanim/dobj_utils.h>
#include "r_draw_method.h"
#include "r_dobj_skin.h"
#include "r_xsurface.h"
#include "r_dpvs.h"
#include "rb_logfile.h"
#include "r_model_lighting.h"
#include "rb_fog.h"
#include "r_workercmds.h"
#include "r_bsp.h"
#include "r_primarylights.h"
#include "r_sunshadow.h"
#include "r_add_staticmodel.h"
#include <DynEntity/DynEntity_client.h>
#include "r_spotshadow.h"
#include "r_shadowcookie.h"
#include "r_pretess.h"
#include "r_light.h"
#include <EffectsCore/fx_system.h>
#include "r_drawsurf.h"
#include "rb_postfx.h"
#include "r_meshdata.h"
#include <qcommon/com_bsp.h>
#include "r_cinematic.h"
#include <win32/win_net.h>
#include <universal/profile.h>
#include "rb_state.h"


//struct GfxScene scene      859c8280     gfx_d3d : r_scene.obj
GfxViewParms lockPvsViewParms;
GfxScene scene;

void __cdecl TRACK_r_scene()
{
    track_static_alloc_internal(&scene, 1395968, "scene", 18);
}

unsigned int __cdecl R_AllocSceneDObj()
{
    unsigned int sceneEntIndex; // [esp+0h] [ebp-4h]

    if (!rg.registered)
        MyAssertHandler(".\\r_scene.cpp", 123, 0, "%s", "rg.registered");
    if (!rg.inFrame)
        MyAssertHandler(".\\r_scene.cpp", 125, 0, "%s", "rg.inFrame");
    sceneEntIndex = InterlockedExchangeAdd(&scene.sceneDObjCount, 1);
    if (sceneEntIndex >= 0x200)
    {
        scene.sceneDObjCount = 512;
        R_WarnOncePerFrame(R_WARN_KNOWN_MODELS, 512);
    }
    return sceneEntIndex;
}

unsigned int __cdecl R_AllocSceneModel()
{
    unsigned int sceneEntIndex; // [esp+0h] [ebp-4h]

    if (!rg.registered)
        MyAssertHandler(".\\r_scene.cpp", 143, 0, "%s", "rg.registered");
    if (!rg.inFrame)
        MyAssertHandler(".\\r_scene.cpp", 145, 0, "%s", "rg.inFrame");
    sceneEntIndex = InterlockedExchangeAdd(&scene.sceneModelCount, 1);
    if (sceneEntIndex >= 0x400)
    {
        scene.sceneModelCount = 1024;
        R_WarnOncePerFrame(R_WARN_KNOWN_MODELS, 1024);
    }
    return sceneEntIndex;
}

unsigned int __cdecl R_AllocSceneBrush()
{
    unsigned int sceneEntIndex; // [esp+0h] [ebp-4h]

    if (!rg.registered)
        MyAssertHandler(".\\r_scene.cpp", 163, 0, "%s", "rg.registered");
    if (!rg.inFrame)
        MyAssertHandler(".\\r_scene.cpp", 165, 0, "%s", "rg.inFrame");
    sceneEntIndex = InterlockedExchangeAdd(&scene.sceneBrushCount, 1);
    if (sceneEntIndex >= 0x200)
    {
        scene.sceneBrushCount = 512;
        R_WarnOncePerFrame(R_WARN_KNOWN_MODELS, 512);
    }
    return sceneEntIndex;
}

GfxBrushModel *__cdecl R_GetBrushModel(unsigned int modelIndex)
{
    if (!rgp.world)
        MyAssertHandler(".\\r_scene.cpp", 180, 0, "%s", "rgp.world");
    if (modelIndex >= rgp.world->modelCount)
        MyAssertHandler(
            ".\\r_scene.cpp",
            181,
            0,
            "modelIndex doesn't index rgp.world->modelCount\n\t%i not in [0, %i)",
            modelIndex,
            rgp.world->modelCount);
    return &rgp.world->models[modelIndex];
}

void __cdecl R_AddBrushModelToSceneFromAngles(
    const GfxBrushModel *bmodel,
    const float *origin,
    const float *angles,
    unsigned __int16 entnum)
{
    unsigned int sceneEntIndex; // [esp+4h] [ebp-8h]
    GfxSceneBrush *sceneBrush; // [esp+8h] [ebp-4h]

    if (!bmodel)
        MyAssertHandler(".\\r_scene.cpp", 191, 0, "%s", "bmodel");
    if (r_drawEntities->current.enabled && r_drawBModels->current.enabled && bmodel->surfaceCount)
    {
        sceneEntIndex = R_AllocSceneBrush();
        if (sceneEntIndex < 0x200)
        {
            sceneBrush = &scene.sceneBrush[sceneEntIndex];
            sceneBrush->bmodel = bmodel;
            sceneBrush->entnum = entnum;
            sceneBrush->placement.origin[0] = *origin;
            sceneBrush->placement.origin[1] = origin[1];
            sceneBrush->placement.origin[2] = origin[2];
            AnglesToQuat(angles, sceneBrush->placement.quat);
        }
    }
}

void __cdecl R_AddDObjToScene(
    const DObj_s *obj,
    const cpose_t *pose,
    unsigned int entnum,
    unsigned int renderFxFlags,
    float *lightingOrigin,
    float materialTime)
{
    float s; // [esp+0h] [ebp-38h]
    GfxSceneModel *sceneModel; // [esp+10h] [ebp-28h]
    XModel *model; // [esp+14h] [ebp-24h]
    float radius; // [esp+18h] [ebp-20h]
    float radiusa; // [esp+18h] [ebp-20h]
    GfxEntity *gfxEnt; // [esp+1Ch] [ebp-1Ch]
    float angles[3]; // [esp+20h] [ebp-18h] BYREF
    GfxSceneEntity *sceneEnt; // [esp+2Ch] [ebp-Ch]
    unsigned int sceneEntIndex; // [esp+30h] [ebp-8h]
    unsigned int gfxEntIndex; // [esp+34h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_scene.cpp", 225, 0, "%s", "Sys_IsMainThread()");
    if (!obj)
        MyAssertHandler(".\\r_scene.cpp", 226, 0, "%s", "obj");
    if (!pose)
        MyAssertHandler(".\\r_scene.cpp", 227, 0, "%s", "pose");
    if (entnum >= gfxCfg.entCount)
        MyAssertHandler(
            ".\\r_scene.cpp",
            228,
            0,
            "entnum doesn't index gfxCfg.entCount\n\t%i not in [0, %i)",
            entnum,
            gfxCfg.entCount);
    if (r_drawEntities->current.enabled)
    {
        if (scene.dpvs.sceneDObjIndex[entnum] != 0xFFFF)
            MyAssertHandler(
                ".\\r_scene.cpp",
                235,
                0,
                "%s\n\t(scene.dpvs.sceneDObjIndex[entnum]) = %i",
                "(scene.dpvs.sceneDObjIndex[entnum] == (65535))",
                scene.dpvs.sceneDObjIndex[entnum]);
        if (materialTime == 0.0 && !renderFxFlags)
        {
            gfxEntIndex = 0;
        }
        else
        {
            gfxEntIndex = InterlockedExchangeAdd(&frontEndDataOut->gfxEntCount, 1);
            if (gfxEntIndex >= 0x80)
            {
                frontEndDataOut->gfxEntCount = 128;
                R_WarnOncePerFrame(R_WARN_KNOWN_SPECIAL_MODELS, 128);
                return;
            }
            gfxEnt = &frontEndDataOut->gfxEnts[gfxEntIndex];
            frontEndDataOut->gfxEnts[gfxEntIndex].materialTime = materialTime;
            gfxEnt->renderFxFlags = renderFxFlags;
        }
        if ((renderFxFlags & 4) != 0 || DObjGetTree(obj) || DObjGetNumModels(obj) != 1)
        {
            sceneEntIndex = R_AllocSceneDObj();
            if (sceneEntIndex < 0x200)
            {
                sceneEnt = &scene.sceneDObj[sceneEntIndex];
                sceneEnt->obj = obj;
                sceneEnt->entnum = entnum;
                scene.dpvs.sceneDObjIndex[entnum] = sceneEntIndex;
                sceneEnt->info.pose = (cpose_t*)pose;
                if (sceneEnt->cull.state)
                    MyAssertHandler(
                        ".\\r_scene.cpp",
                        296,
                        1,
                        "%s\n\t(sceneEnt->cull.state) = %i",
                        "(sceneEnt->cull.state == CULL_STATE_OUT)",
                        sceneEnt->cull.state);
                radiusa = DObjGetRadius(obj);
                CG_GetPoseOrigin(pose, sceneEnt->placement.base.origin);
                CG_GetPoseAngles(pose, angles);
                AnglesToQuat(angles, sceneEnt->placement.base.quat);
                sceneEnt->placement.scale = 1.0;
                s = -radiusa;
                Vec3AddScalar(sceneEnt->placement.base.origin, s, sceneEnt->cull.mins);
                Vec3AddScalar(sceneEnt->placement.base.origin, radiusa, sceneEnt->cull.maxs);
                sceneEnt->lightingOrigin[0] = *lightingOrigin;
                sceneEnt->lightingOrigin[1] = lightingOrigin[1];
                sceneEnt->lightingOrigin[2] = lightingOrigin[2];
                sceneEnt->gfxEntIndex = gfxEntIndex;
                CG_PredictiveSkinCEntity(sceneEnt);
            }
        }
        else
        {
            sceneEntIndex = R_AllocSceneModel();
            if (sceneEntIndex < 0x400)
            {
                sceneModel = &scene.sceneModel[sceneEntIndex];
                model = DObjGetModel(obj, 0);
                sceneModel->model = model;
                sceneModel->obj = obj;
                sceneModel->entnum = entnum;
                scene.dpvs.sceneXModelIndex[entnum] = sceneEntIndex;
                sceneModel->cachedLightingHandle = (unsigned __int16 *)LongNoSwap((unsigned int)pose);
                radius = XModelGetRadius(model);
                CG_GetPoseOrigin(pose, sceneModel->placement.base.origin);
                CG_GetPoseAngles(pose, angles);
                AnglesToQuat(angles, sceneModel->placement.base.quat);
                sceneModel->placement.scale = 1.0;
                sceneModel->radius = radius;
                sceneModel->lightingOrigin[0] = *lightingOrigin;
                sceneModel->lightingOrigin[1] = lightingOrigin[1];
                sceneModel->lightingOrigin[2] = lightingOrigin[2];
                sceneModel->gfxEntIndex = gfxEntIndex;
            }
        }
    }
}

GfxParticleCloud *__cdecl R_AddParticleCloudToScene(Material *material)
{
    volatile unsigned int cloudIndex; // [esp+Ch] [ebp-4h]

    cloudIndex = InterlockedIncrement(&frontEndDataOut->cloudCount) - 1;
    if (cloudIndex < 0x100)
    {
        if (R_AddParticleCloudDrawSurf(cloudIndex, material))
            return &frontEndDataOut->clouds[cloudIndex];
        else
            return 0;
    }
    else
    {
        InterlockedDecrement(&frontEndDataOut->cloudCount);
        R_WarnOncePerFrame(R_WARN_MAX_CLOUDS);
        return 0;
    }
}

void __cdecl R_AddOmniLightToScene(const float *org, float radius, float r, float g, float b)
{
    GfxLight *dst; // [esp+8h] [ebp-4h]

    if (rg.registered && rgp.world)
    {
        if (!rg.inFrame)
            MyAssertHandler(".\\r_scene.cpp", 353, 0, "%s", "rg.inFrame");
        if (radius > 0.0)
        {
            if (scene.addedLightCount < 32)
            {
                dst = &scene.addedLight[scene.addedLightCount++];
                memset(&dst->type, 0, sizeof(GfxLight));
                dst->def = rgp.dlightDef;
                dst->type = 3;
                dst->origin[0] = *org;
                dst->origin[1] = org[1];
                dst->origin[2] = org[2];
                dst->radius = radius;
                dst->color[0] = r;
                dst->color[1] = g;
                dst->color[2] = b;
                dst->canUseShadowMap = 0;
                dst->spotShadowIndex = -1;
            }
            else
            {
                R_WarnOncePerFrame(R_WARN_MAX_DLIGHTS);
            }
        }
    }
}

void __cdecl R_AddSpotLightToScene(const float *org, const float *dir, float radius, float r, float g, float b)
{
    float v6; // [esp+Ch] [ebp-38h]
    float v7; // [esp+10h] [ebp-34h]
    float v8; // [esp+14h] [ebp-30h]
    float v9; // [esp+18h] [ebp-2Ch]
    float v10; // [esp+1Ch] [ebp-28h]
    float value; // [esp+20h] [ebp-24h]
    float v12; // [esp+30h] [ebp-14h]
    float spotLightFovInner; // [esp+38h] [ebp-Ch]
    float spotLightOffset; // [esp+3Ch] [ebp-8h]
    GfxLight *dst; // [esp+40h] [ebp-4h]

    if (rg.registered && rgp.world)
    {
        if (!rg.inFrame)
            MyAssertHandler(".\\r_scene.cpp", 392, 0, "%s", "rg.inFrame");
        if (radius > 0.0)
        {
            if (scene.addedLightCount < 32)
            {
                if (r_spotLightStartRadius->current.value >= (double)r_spotLightEndRadius->current.value)
                {
                    value = r_spotLightStartRadius->current.value + 0.1000000014901161;
                    Dvar_SetFloat((dvar_s *)r_spotLightEndRadius, value);
                }
                if (r_spotLightEndRadius->current.value >= r_spotLightStartRadius->current.value + radius)
                {
                    v10 = r_spotLightStartRadius->current.value + radius - 0.1000000014901161;
                    Dvar_SetFloat((dvar_s *)r_spotLightEndRadius, v10);
                }
                dst = &scene.addedLight[scene.addedLightCount++];
                v12 = (r_spotLightEndRadius->current.value - r_spotLightStartRadius->current.value) / radius;
                v9 = atan(v12);
                spotLightFovInner = v9 * r_spotLightFovInnerFraction->current.value;
                if (v9 <= 0.0)
                    MyAssertHandler(".\\r_scene.cpp", 413, 0, "%s", "spotLightFovOuter > 0.0f");
                v8 = tan(v9);
                spotLightOffset = r_spotLightStartRadius->current.value / v8;
                memset(&dst->type, 0, sizeof(GfxLight));
                dst->def = rgp.dlightDef;
                dst->type = 2;
                dst->origin[0] = *org;
                dst->origin[1] = org[1];
                dst->origin[2] = org[2];
                Vec3Scale(dir, -1.0, dst->dir);
                Vec3Mad(dst->origin, spotLightOffset, dst->dir, dst->origin);
                dst->radius = radius + spotLightOffset;
                dst->color[0] = r;
                dst->color[1] = g;
                dst->color[2] = b;
                Vec3Scale(dst->color, r_spotLightBrightness->current.value, dst->color);
                scene.dynamicSpotLightNearPlaneOffset = spotLightOffset;
                dst->exponent = 1;
                if (spotLightFovInner >= (double)v9)
                    MyAssertHandler(".\\r_scene.cpp", 429, 0, "%s", "spotLightFovOuter > spotLightFovInner");
                v7 = cos(spotLightFovInner);
                dst->cosHalfFovInner = v7;
                v6 = cos(v9);
                dst->cosHalfFovOuter = v6;
                dst->canUseShadowMap = r_spotLightShadows->current.color[0];
                dst->spotShadowIndex = -1;
            }
            else
            {
                R_WarnOncePerFrame(R_WARN_MAX_DLIGHTS);
            }
        }
    }
}

double __cdecl R_GetDefaultNearClip()
{
    float v2; // [esp+4h] [ebp-8h]
    float value; // [esp+8h] [ebp-4h]

    value = r_znear->current.value;
    v2 = 0.0099999998 - value;
    if (v2 < 0.0)
        return value;
    else
        return (float)0.0099999998;
}

void __cdecl R_SetupViewProjectionMatrices(GfxViewParms *viewParms)
{
    MatrixMultiply44(viewParms->viewMatrix.m, viewParms->projectionMatrix.m, viewParms->viewProjectionMatrix.m);
    MatrixInverse44(viewParms->viewProjectionMatrix.m, viewParms->inverseViewProjectionMatrix.m);
}

void __cdecl R_AddBModelSurfacesCamera(
    BModelDrawInfo *bmodelInfo,
    const GfxBrushModel *bmodel,
    GfxDrawSurf **drawSurfs,
    GfxDrawSurf **lastDrawSurfs,
    unsigned int reflectionProbeIndex)
{
    unsigned __int16 surfaceCount; // [esp+4h] [ebp-28h]
    unsigned int surfId; // [esp+8h] [ebp-24h]
    //unsigned __int64 drawSurf; // [esp+Ch] [ebp-20h]
    const Material *material; // [esp+14h] [ebp-18h]
    BModelSurface *modelSurf; // [esp+18h] [ebp-14h]
    const GfxSurface *bspSurf; // [esp+1Ch] [ebp-10h]
    unsigned int region; // [esp+20h] [ebp-Ch]
    unsigned int count; // [esp+28h] [ebp-4h]

    iassert(bmodel);
    surfId = bmodelInfo->surfId;
    modelSurf = (BModelSurface *)((char *)frontEndDataOut + 4 * surfId);
    if (reflectionProbeIndex >= 0x100)
        MyAssertHandler(
            ".\\r_scene.cpp",
            548,
            0,
            "reflectionProbeIndex doesn't index 1 << MTL_SORT_ENVMAP_BITS\n\t%i not in [0, %i)",
            reflectionProbeIndex,
            256);
    if (gfxDrawMethod.emissiveTechType >= (unsigned int)TECHNIQUE_COUNT)
        MyAssertHandler(
            ".\\r_scene.cpp",
            550,
            0,
            "gfxDrawMethod.emissiveTechType doesn't index TECHNIQUE_COUNT\n\t%i not in [0, %i)",
            gfxDrawMethod.emissiveTechType,
            34);
    if (r_drawDecals->current.enabled)
        surfaceCount = bmodel->surfaceCount;
    else
        surfaceCount = bmodel->surfaceCountNoDecal;
    for (count = 0; count < surfaceCount; ++count)
    {
        bspSurf = modelSurf->surf;
        material = bspSurf->material;
        iassert(rgp.sortedMaterials[material->info.drawSurf.fields.materialSortedIndex] == material);
        region = material->cameraRegion;
        if (region != 3)
        {
            if (drawSurfs[region] >= lastDrawSurfs[region])
            {
                R_WarnOncePerFrame(R_WARN_MAX_SCENE_DRAWSURFS, "R_AddBModelSurfacesCamera");
                return;
            }
            if (surfId >= 0x10000)
                MyAssertHandler(
                    ".\\r_scene.cpp",
                    573,
                    0,
                    "surfId doesn't index 1 << MTL_SORT_OBJECT_ID_BITS\n\t%i not in [0, %i)",
                    surfId,
                    0x10000);

            drawSurfs[region]->packed = material->info.drawSurf.packed;

            drawSurfs[region]->fields.reflectionProbeIndex = reflectionProbeIndex;
            drawSurfs[region]->fields.customIndex = bspSurf->lightmapIndex;
            drawSurfs[region]->fields.objectId = surfId;
            drawSurfs[region]->fields.primaryLightIndex = bspSurf->primaryLightIndex;

            //LODWORD(drawSurf) = ((unsigned __int8)reflectionProbeIndex << 16) // reflectionProbeIndex
            //    | ((bspSurf->lightmapIndex & 0x1F) << 24) // customIndex
            //    | (unsigned __int16)surfId // objectId
            //    | *(unsigned int *)&material->info.drawSurf.fields & 0xE0000000; // copy upper 3 bits of LODWORD

            //HIDWORD(drawSurf) = (bspSurf->primaryLightIndex << 10) // PrimaryLightIndex
            //    | HIDWORD(material->info.drawSurf.packed) & 0xFFC003FF // all other bits in HIDWORD
            //    | 0x180000; // this sets surfType but idk the exact value in bits...

            HIDWORD(drawSurfs[region]->packed) |= 0x180000;// this sets surfType but idk the exact value in bits...

            ++drawSurfs[region];
        }
        ++modelSurf;
        surfId += 2;
    }
}

GfxDrawSurf *__cdecl R_AddBModelSurfaces(
    BModelDrawInfo *bmodelInfo,
    const GfxBrushModel *bmodel,
    MaterialTechniqueType techType,
    GfxDrawSurf *drawSurf,
    GfxDrawSurf *lastDrawSurf)
{
    unsigned __int16 surfaceCount; // [esp+6h] [ebp-2Ah]
    unsigned int surfId; // [esp+10h] [ebp-20h]
    const Material *material; // [esp+14h] [ebp-1Ch]
    unsigned int newDrawSurf_4; // [esp+1Ch] [ebp-14h]
    BModelSurface *modelSurf; // [esp+20h] [ebp-10h]
    unsigned int count; // [esp+2Ch] [ebp-4h]

    if (!bmodel)
        MyAssertHandler(".\\r_scene.cpp", 598, 0, "%s", "bmodel");
    surfId = bmodelInfo->surfId;
    modelSurf = (BModelSurface *)((char *)frontEndDataOut + 4 * surfId);
    if (r_drawDecals->current.enabled)
        surfaceCount = bmodel->surfaceCount;
    else
        surfaceCount = bmodel->surfaceCountNoDecal;
    for (count = 0; count < surfaceCount; ++count)
    {
        if (drawSurf >= lastDrawSurf)
        {
            R_WarnOncePerFrame(R_WARN_MAX_SCENE_DRAWSURFS, "R_AddBModelSurfaces");
            return drawSurf;
        }
        material = modelSurf->surf->material;
        iassert(rgp.sortedMaterials[material->info.drawSurf.fields.materialSortedIndex] == material);
        if (Material_GetTechnique(material, techType))
        {
            if (surfId >= 0x10000)
                MyAssertHandler(
                    ".\\r_scene.cpp",
                    623,
                    0,
                    "surfId doesn't index 1 << MTL_SORT_OBJECT_ID_BITS\n\t%i not in [0, %i)",
                    surfId,
                    0x10000);
            newDrawSurf_4 = HIDWORD(material->info.drawSurf.packed) & 0xFFC3FFFF | 0x180000;
            *(unsigned int *)&drawSurf->fields = (unsigned __int16)surfId | *(unsigned int *)&material->info.drawSurf.fields & 0xFFFF0000;
            HIDWORD(drawSurf->packed) = newDrawSurf_4;
            ++drawSurf;
        }
        ++modelSurf;
        surfId += 2;
    }
    return drawSurf;
}

const XSurface *__cdecl R_GetXSurface(unsigned int *modelSurf, surfaceType_t surfType)
{
    if (!modelSurf)
        MyAssertHandler(".\\r_scene.cpp", 640, 0, "%s", "modelSurf");
    if ((unsigned int)surfType < SF_BEGIN_XMODEL || (unsigned int)surfType >= SF_END_XMODEL)
        MyAssertHandler(".\\r_scene.cpp", 641, 0, "%s", "R_IsModelSurfaceType( surfType )");
    return (const XSurface *)modelSurf[1];
}

void __cdecl R_AddXModelSurfacesCamera(
    XModelDrawInfo *modelInfo,
    const XModel *model,
    float *origin,
    unsigned __int16 gfxEntIndex,
    unsigned int lightingHandle,
    unsigned __int8 primaryLightIndex,
    char isShadowReceiver,
    int depthHack,
    GfxDrawSurf **drawSurfs,
    GfxDrawSurf **lastDrawSurfs,
    unsigned int reflectionProbeIndex)
{
    const XSurface *xSurf; // eax
    const XSurface *v12; // eax
    char *v13; // eax
    char *v14; // eax
    char *v15; // eax
    unsigned int surfId; // [esp+8h] [ebp-38h]
    int totalVertCount; // [esp+Ch] [ebp-34h]
    //__int64 drawSurf; // [esp+10h] [ebp-30h]
    int totalTriCount; // [esp+1Ch] [ebp-24h]
    Material **material; // [esp+20h] [ebp-20h]
    unsigned int subMatIndex; // [esp+24h] [ebp-1Ch]
    int skinnedCachedOffset; // [esp+28h] [ebp-18h]
    int lod; // [esp+2Ch] [ebp-14h]
    GfxModelRigidSurface *modelSurf; // [esp+30h] [ebp-10h]
    surfaceType_t surfType; // [esp+34h] [ebp-Ch]
    unsigned int region; // [esp+38h] [ebp-8h]
    unsigned int numsurfs; // [esp+3Ch] [ebp-4h]

    if (!lightingHandle)
        MyAssertHandler(".\\r_scene.cpp", 674, 0, "%s", "lightingHandle");
    totalTriCount = 0;
    totalVertCount = 0;
    if (!model)
        MyAssertHandler(".\\r_scene.cpp", 681, 0, "%s", "model");
    surfId = modelInfo->surfId;
    modelSurf = (GfxModelRigidSurface*)((char*)frontEndDataOut + 4 * surfId);
    lod = modelInfo->lod;
    numsurfs = XModelGetSurfCount(model, lod);
    material = XModelGetSkins(model, lod);
    if (!material)
        MyAssertHandler(".\\r_scene.cpp", 691, 0, "%s", "material");
    if (reflectionProbeIndex >= 0x100)
        MyAssertHandler(
            ".\\r_scene.cpp",
            693,
            0,
            "reflectionProbeIndex doesn't index 1 << MTL_SORT_ENVMAP_BITS\n\t%i not in [0, %i)",
            reflectionProbeIndex,
            256);
    if (gfxDrawMethod.emissiveTechType >= TECHNIQUE_COUNT)
        MyAssertHandler(
            ".\\r_scene.cpp",
            695,
            0,
            "gfxDrawMethod.emissiveTechType doesn't index TECHNIQUE_COUNT\n\t%i not in [0, %i)",
            gfxDrawMethod.emissiveTechType,
            34);
    for (subMatIndex = 0; subMatIndex < numsurfs; ++subMatIndex)
    {
        skinnedCachedOffset = modelSurf->surf.skinnedCachedOffset;
        if (modelSurf->surf.skinnedCachedOffset == -3)
        {
            ++surfId;
            modelSurf = (GfxModelRigidSurface*)((char*)modelSurf + 4);
        }
        else
        {
            if (!*material)
                MyAssertHandler(".\\r_scene.cpp", 708, 0, "%s", "*material");
            if (rgp.sortedMaterials[(*material)->info.drawSurf.fields.materialSortedIndex] != *material)
                MyAssertHandler(
                    ".\\r_scene.cpp",
                    709,
                    0,
                    "%s",
                    "rgp.sortedMaterials[(*material)->info.drawSurf.fields.materialSortedIndex] == *material");
            region = (*material)->cameraRegion;
            if (region == 3)
            {
                surfId += 14;
                ++modelSurf;
            }
            else
            {
                if (drawSurfs[region] >= lastDrawSurfs[region])
                {
                    R_WarnOncePerFrame(R_WARN_MAX_SCENE_DRAWSURFS, "R_AddXModelSurfacesCamera");
                    break;
                }
                if (skinnedCachedOffset == -2)
                {
                    surfType = SF_BEGIN_XMODEL;
                }
                else
                {
                    if (skinnedCachedOffset != -1)
                        MyAssertHandler(
                            ".\\r_scene.cpp",
                            728,
                            0,
                            "%s\n\t(skinnedCachedOffset) = %i",
                            "(skinnedCachedOffset == (-1))",
                            skinnedCachedOffset);
                    surfType = SF_XMODEL_RIGID_SKINNED;
                }
                modelSurf->surf.info.gfxEntIndex = gfxEntIndex;
                modelSurf->surf.info.lightingHandle = lightingHandle;
                if (surfId >= 0x10000)
                    MyAssertHandler(
                        ".\\r_scene.cpp",
                        740,
                        0,
                        "surfId doesn't index 1 << MTL_SORT_OBJECT_ID_BITS\n\t%i not in [0, %i)",
                        surfId,
                        0x10000);

                //drawSurf = (*material)->info.drawSurf.packed;

                drawSurfs[region]->packed = (*material)->info.drawSurf.packed;

                drawSurfs[region]->fields.surfType = surfType;
                drawSurfs[region]->fields.primarySortKey -= depthHack;
                
                drawSurfs[region]->fields.customIndex = isShadowReceiver;
                drawSurfs[region]->fields.reflectionProbeIndex = reflectionProbeIndex;
                drawSurfs[region]->fields.objectId = surfId;
                drawSurfs[region]->fields.primaryLightIndex = primaryLightIndex;

                //HIDWORD(drawSurf) = ((surfType & 0xF) << 18) // surfType
                //    | (((((drawSurf >> 54) & 0x3F) - depthHack) & 0x3F) << 22) // primarySortKey - depthHAck
                //    | HIDWORD(drawSurf) & 0xF003FFFF; // all bits except the 10 above

                //LODWORD(drawSurf) = ((isShadowReceiver & 0x1F) << 24) // customIndex
                //    | (reflectionProbeIndex << 16) & 0xE0FFFFFF // reflectionProbIndex
                //    | surfId // objectId
                //    | drawSurf & 0xE0000000; // top 3 bits of LODWORD
                //HIDWORD(drawSurf) = (primaryLightIndex << 10) | HIDWORD(drawSurf) & 0xFFFC03FF; // primaryLightIndex and rest of bits

                //drawSurfs[region]->packed = drawSurf;

                ++drawSurfs[region];
                if (r_showTriCounts->current.enabled)
                {
                    xSurf = R_GetXSurface((unsigned int*)modelSurf, surfType);
                    totalTriCount += XSurfaceGetNumTris(xSurf);
                }
                else if (r_showVertCounts->current.enabled)
                {
                    v12 = R_GetXSurface((unsigned int *)modelSurf, surfType);
                    totalVertCount += XSurfaceGetNumVerts(v12);
                }
                surfId += 14;
                ++modelSurf;
            }
        }
        ++material;
    }
    if (r_showTriCounts->current.enabled)
    {
        v13 = va("%i", totalTriCount);
        R_AddXModelDebugString(origin, v13);
    }
    else if (r_showVertCounts->current.enabled)
    {
        v14 = va("%i", totalVertCount);
        R_AddXModelDebugString(origin, v14);
    }
    else if (r_showSurfCounts->current.enabled)
    {
        v15 = va("%i", numsurfs);
        R_AddXModelDebugString(origin, v15);
    }
}

void __cdecl R_AddXModelDebugString(const float *origin, char *string)
{
    R_AddScaledDebugString(&frontEndDataOut->debugGlobals, rg.debugViewParms, origin, colorCyan, string);
}

GfxDrawSurf *__cdecl R_AddXModelSurfaces(
    XModelDrawInfo *modelInfo,
    const XModel *model,
    MaterialTechniqueType techType,
    GfxDrawSurf *drawSurf,
    GfxDrawSurf *lastDrawSurf)
{
    unsigned int surfId; // [esp+10h] [ebp-2Ch]
    Material **material; // [esp+14h] [ebp-28h]
    unsigned int subMatIndex; // [esp+18h] [ebp-24h]
    //unsigned __int64 newDrawSurf; // [esp+1Ch] [ebp-20h]
    int skinnedCachedOffset; // [esp+28h] [ebp-14h]
    int lod; // [esp+2Ch] [ebp-10h]
    GfxModelRigidSurface *modelSurf; // [esp+30h] [ebp-Ch]
    char surfType; // [esp+34h] [ebp-8h]
    unsigned int numsurfs; // [esp+38h] [ebp-4h]

    if (!model)
        MyAssertHandler(".\\r_scene.cpp", 787, 0, "%s", "model");
    surfId = modelInfo->surfId;
    modelSurf = (GfxModelRigidSurface *)((char *)frontEndDataOut + 4 * surfId);
    lod = modelInfo->lod;
    numsurfs = XModelGetSurfCount(model, lod);
    material = XModelGetSkins(model, lod);
    if (!material)
        MyAssertHandler(".\\r_scene.cpp", 797, 0, "%s", "material");
    for (subMatIndex = 0; subMatIndex < numsurfs; ++subMatIndex)
    {
        skinnedCachedOffset = modelSurf->surf.skinnedCachedOffset;
        if (modelSurf->surf.skinnedCachedOffset == -3)
        {
            ++surfId;
            modelSurf = (GfxModelRigidSurface *)((char *)modelSurf + 4);
        }
        else
        {
            iassert(*material);
            iassert(rgp.sortedMaterials[(*material)->info.drawSurf.fields.materialSortedIndex] == *material);

            if (Material_GetTechnique(*material, techType))
            {
                if (drawSurf >= lastDrawSurf)
                {
                    R_WarnOncePerFrame(R_WARN_MAX_SCENE_DRAWSURFS, "R_AddXModelSurfaces");
                    return drawSurf;
                }
                if (skinnedCachedOffset == -2)
                {
                    surfType = 7;
                }
                else
                {
                    if (skinnedCachedOffset != -1)
                        MyAssertHandler(
                            ".\\r_scene.cpp",
                            829,
                            0,
                            "%s\n\t(skinnedCachedOffset) = %i",
                            "(skinnedCachedOffset == (-1))",
                            skinnedCachedOffset);
                    surfType = 8;
                }
                if (surfId >= 0x10000)
                    MyAssertHandler(
                        ".\\r_scene.cpp",
                        838,
                        0,
                        "surfId doesn't index 1 << MTL_SORT_OBJECT_ID_BITS\n\t%i not in [0, %i)",
                        surfId,
                        0x10000);

                drawSurf->packed = (*material)->info.drawSurf.packed; // copy all to start with

                drawSurf->fields.surfType = surfType;
                drawSurf->fields.objectId = surfId;
                //HIDWORD(newDrawSurf) = ((surfType & 0xF) << 18)  // surfType
                //    | HIDWORD((*material)->info.drawSurf.packed) & 0xFFC3FFFF; // rest of bits in HIDWORD
                //LODWORD(newDrawSurf) = (unsigned __int16)surfId  // objectID
                //    | *(unsigned int *)&(*material)->info.drawSurf.fields & 0xFFFF0000; // rest of bits in LODWORD

                //drawSurf->packed = newDrawSurf;

                ++drawSurf;
                surfId += 14;
                ++modelSurf;
            }
            else
            {
                surfId += 14;
                ++modelSurf;
            }
        }
        ++material;
    }
    return drawSurf;
}

void __cdecl R_AddDObjSurfacesCamera(
    GfxSceneEntity *sceneEnt,
    __int16 lightingHandle,
    unsigned __int8 primaryLightIndex,
    GfxDrawSurf **drawSurfs,
    GfxDrawSurf **lastDrawSurfs)
{
    const XSurface *xSurf; // eax
    const XSurface *v6; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    bool v10; // [esp+4h] [ebp-6Ch]
    unsigned int surfId; // [esp+Ch] [ebp-64h]
    unsigned int surfIda; // [esp+Ch] [ebp-64h]
    int totalVertCount; // [esp+10h] [ebp-60h]
    const DObj_s *obj; // [esp+14h] [ebp-5Ch]
    //unsigned __int64 drawSurf; // [esp+18h] [ebp-58h]
    int totalTriCount; // [esp+20h] [ebp-50h]
    Material **material; // [esp+24h] [ebp-4Ch]
    XModel *model; // [esp+28h] [ebp-48h]
    unsigned int subMatIndex; // [esp+2Ch] [ebp-44h]
    unsigned int surfSize; // [esp+30h] [ebp-40h]
    int lod; // [esp+3Ch] [ebp-34h]
    unsigned int depthHack; // [esp+40h] [ebp-30h]
    char *modelSurf; // [esp+44h] [ebp-2Ch]
    bool isShadowReceiver; // [esp+48h] [ebp-28h]
    surfaceType_t surfType; // [esp+4Ch] [ebp-24h]
    unsigned int region; // [esp+50h] [ebp-20h]
    unsigned int gfxEntIndex; // [esp+54h] [ebp-1Ch]
    unsigned int numsurfs; // [esp+5Ch] [ebp-14h]
    unsigned int modelIndex; // [esp+60h] [ebp-10h]
    unsigned int modelCount; // [esp+64h] [ebp-Ch]
    int totalSurfCount; // [esp+6Ch] [ebp-4h]

    modelSurf = (char *)sceneEnt->cull.skinnedSurfs.firstSurf;
    if (!modelSurf)
        return;
    totalTriCount = 0;
    totalVertCount = 0;
    totalSurfCount = 0;
    obj = sceneEnt->obj;
    if (!obj)
        MyAssertHandler(".\\r_scene.cpp", 895, 0, "%s", "obj");
    modelCount = DObjGetNumModels(obj);
    if (gfxDrawMethod.emissiveTechType >= (unsigned int)TECHNIQUE_COUNT)
        MyAssertHandler(
            ".\\r_scene.cpp",
            903,
            0,
            "gfxDrawMethod.emissiveTechType doesn't index TECHNIQUE_COUNT\n\t%i not in [0, %i)",
            gfxDrawMethod.emissiveTechType,
            34);
    gfxEntIndex = sceneEnt->gfxEntIndex;
    if (sceneEnt->gfxEntIndex)
    {
        v10 = sc_enable->current.enabled && (frontEndDataOut->gfxEnts[gfxEntIndex].renderFxFlags & 0x100) != 0;
        isShadowReceiver = v10;
        depthHack = (frontEndDataOut->gfxEnts[gfxEntIndex].renderFxFlags & 2) != 0;
    }
    else
    {
        isShadowReceiver = 0;
        depthHack = 0;
    }
    modelIndex = 0;
LABEL_15:
    if (modelIndex < modelCount)
    {
        lod = sceneEnt->cull.lods[modelIndex];
        if (lod < 0)
            goto LABEL_14;
        model = DObjGetModel(obj, modelIndex);
        if (!model)
            MyAssertHandler(".\\r_scene.cpp", 926, 0, "%s", "model");
        numsurfs = XModelGetSurfCount(model, lod);
        material = XModelGetSkins(model, lod);
        if (!material)
            MyAssertHandler(".\\r_scene.cpp", 930, 0, "%s", "material");
        for (subMatIndex = 0; ; ++subMatIndex)
        {
            if (subMatIndex >= numsurfs)
            {
            LABEL_45:
                totalSurfCount += numsurfs;
            LABEL_14:
                ++modelIndex;
                goto LABEL_15;
            }
            if (*(unsigned int *)modelSurf == -2)
            {
                surfType = SF_BEGIN_XMODEL;
                surfSize = 56;
            }
            else
            {
                if (*(unsigned int *)modelSurf == -3)
                {
                    surfSize = 4;
                    goto LABEL_22;
                }
                surfType = SF_XMODEL_SKINNED;
                surfSize = 24;
            }
            iassert(*material);
            iassert(rgp.sortedMaterials[(*material)->info.drawSurf.fields.materialSortedIndex] == *material);
            region = (*material)->cameraRegion;
            if (region != 3)
            {
                if (drawSurfs[region] >= lastDrawSurfs[region])
                {
                    R_WarnOncePerFrame(R_WARN_MAX_SCENE_DRAWSURFS, "R_AddDObjSurfacesCamera");
                    goto LABEL_45;
                }
                *((_WORD *)modelSurf + 7) = gfxEntIndex;
                *((_WORD *)modelSurf + 8) = lightingHandle;
                surfId = modelSurf - (char *)frontEndDataOut;
                if ((((_BYTE)modelSurf - (_BYTE)frontEndDataOut) & 3) != 0)
                    MyAssertHandler(".\\r_scene.cpp", 971, 0, "%s", "!(surfId & 3)");
                surfIda = surfId >> 2;
                if (surfIda >= 0x10000)
                    MyAssertHandler(
                        ".\\r_scene.cpp",
                        973,
                        0,
                        "surfId doesn't index 1 << MTL_SORT_OBJECT_ID_BITS\n\t%i not in [0, %i)",
                        surfIda,
                        0x10000);
                //drawSurf = (*material)->info.drawSurf.packed;

                drawSurfs[region]->packed = (*material)->info.drawSurf.packed;

                drawSurfs[region]->fields.surfType = surfType;
                drawSurfs[region]->fields.primarySortKey -= depthHack;
                drawSurfs[region]->fields.customIndex = isShadowReceiver;
                drawSurfs[region]->fields.reflectionProbeIndex = sceneEnt->reflectionProbeIndex;
                drawSurfs[region]->fields.objectId = surfIda;
                drawSurfs[region]->fields.primaryLightIndex = primaryLightIndex;

                //HIDWORD(drawSurf) = ((surfType & 0xF) << 18) // surftype
                //    | (((((drawSurf >> 54) & 0x3F) - depthHack) & 0x3F) << 22) // primarySortKey
                //    | HIDWORD(drawSurf) & 0xF003FFFF; // all bits except the 10 we just set
                //LODWORD(drawSurf) = ((isShadowReceiver & 0x1F) << 24) // customIndex
                    //| (sceneEnt->reflectionProbeIndex << 16) & 0xE0FFFFFF
                    //| (unsigned __int16)surfIda
                    //| drawSurf & 0xE0000000; // top 3 bits
                //HIDWORD(drawSurf) = (primaryLightIndex << 10) | HIDWORD(drawSurf) & 0xFFFC03FF;

                //drawSurfs[region]->packed = drawSurf;
                ++drawSurfs[region];
                if (r_showTriCounts->current.enabled)
                {
                    xSurf = R_GetXSurface((unsigned int *)modelSurf, surfType);
                    totalTriCount += XSurfaceGetNumTris(xSurf);
                }
                else if (r_showVertCounts->current.enabled)
                {
                    v6 = R_GetXSurface((unsigned int*)modelSurf, surfType);
                    totalVertCount += XSurfaceGetNumVerts(v6);
                }
            }
        LABEL_22:
            ++material;
            modelSurf += surfSize;
        }
    }
    if (r_showTriCounts->current.enabled)
    {
        v7 = va("%i", totalTriCount);
        R_AddXModelDebugString(sceneEnt->placement.base.origin, v7);
    }
    else if (r_showVertCounts->current.enabled)
    {
        v8 = va("%i", totalVertCount);
        R_AddXModelDebugString(sceneEnt->placement.base.origin, v8);
    }
    else if (r_showSurfCounts->current.enabled)
    {
        v9 = va("%i", totalSurfCount);
        R_AddXModelDebugString(sceneEnt->placement.base.origin, v9);
    }
}

GfxDrawSurf *__cdecl R_AddDObjSurfaces(
    GfxSceneEntity *sceneEnt,
    MaterialTechniqueType techType,
    GfxDrawSurf *drawSurf,
    GfxDrawSurf *lastDrawSurf)
{
    unsigned int surfId; // [esp+10h] [ebp-50h]
    const DObj_s *obj; // [esp+14h] [ebp-4Ch]
    Material **material; // [esp+18h] [ebp-48h]
    XModel *model; // [esp+1Ch] [ebp-44h]
    unsigned int subMatIndex; // [esp+20h] [ebp-40h]
    unsigned int surfSize; // [esp+24h] [ebp-3Ch]
    GfxDrawSurf newDrawSurf; // [esp+28h] [ebp-38h]
    int lod; // [esp+38h] [ebp-28h]
    unsigned int depthHack; // [esp+3Ch] [ebp-24h]
    char *modelSurf; // [esp+40h] [ebp-20h]
    char surfType; // [esp+44h] [ebp-1Ch]
    unsigned int numsurfs; // [esp+50h] [ebp-10h]
    unsigned int modelIndex; // [esp+54h] [ebp-Ch]
    unsigned int modelCount; // [esp+58h] [ebp-8h]

    modelSurf = (char *)sceneEnt->cull.skinnedSurfs.firstSurf;
    if (!modelSurf)
        return drawSurf;
    obj = sceneEnt->obj;
    if (!obj)
        MyAssertHandler(".\\r_scene.cpp", 1037, 0, "%s", "obj");
    modelCount = DObjGetNumModels(obj);
    if (sceneEnt->gfxEntIndex)
        depthHack = (frontEndDataOut->gfxEnts[sceneEnt->gfxEntIndex].renderFxFlags & 2) != 0;
    else
        depthHack = 0;
    for (modelIndex = 0; modelIndex < modelCount; ++modelIndex)
    {
        lod = sceneEnt->cull.lods[modelIndex];
        if (lod >= 0)
        {
            model = DObjGetModel(obj, modelIndex);
            if (!model)
                MyAssertHandler(".\\r_scene.cpp", 1060, 0, "%s", "model");
            numsurfs = XModelGetSurfCount(model, lod);
            material = XModelGetSkins(model, lod);
            if (!material)
                MyAssertHandler(".\\r_scene.cpp", 1064, 0, "%s", "material");
            subMatIndex = 0;
        LABEL_18:
            if (subMatIndex >= numsurfs)
                continue;
            if (*(unsigned int *)modelSurf == -2)
            {
                surfType = 7;
                surfSize = 56;
            }
            else
            {
                if (*(unsigned int *)modelSurf == -3)
                {
                    surfSize = 4;
                LABEL_17:
                    ++subMatIndex;
                    ++material;
                    modelSurf += surfSize;
                    goto LABEL_18;
                }
                surfType = 9;
                surfSize = 24;
            }
            iassert(*material);
            iassert(rgp.sortedMaterials[(*material)->info.drawSurf.fields.materialSortedIndex] == *material);
            if (Material_GetTechnique(*material, techType))
            {
                if (drawSurf >= lastDrawSurf)
                {
                    R_WarnOncePerFrame(R_WARN_MAX_SCENE_DRAWSURFS, "R_AddDObjSurfaces");
                    continue;
                }
                surfId = modelSurf - (char *)frontEndDataOut;
                iassert(!(surfId & 3));

                surfId = surfId / 4;
                iassert(surfId < MTL_SORT_OBJECT_ID_BITS);

                newDrawSurf = (*material)->info.drawSurf;

                drawSurf->packed = newDrawSurf.packed; // LWSS: see explanation below, it basically copies the whole thing and sets a few custom fields.

                drawSurf->fields.objectId = (unsigned short)surfId;
                //*(unsigned int *)&drawSurf->fields = (unsigned __int16)surfIda  // set lower 16 (objectId)
                //    | *(unsigned int *)&newDrawSurf.fields & 0xFFFF0000; // Copy the higher 2 bytes (bits 16-32)
                
                drawSurf->fields.surfType = surfType;
                drawSurf->fields.primarySortKey = (newDrawSurf.fields.primarySortKey - depthHack);

                // HIDWORD(drawSurf->packed) = // Adds << 32 to all below
                //     ((surfType & 0xF) << 18) // surfType
                //     | (((((newDrawSurf.packed >> 54) & 0x3F) - depthHack) & 0x3F) << 22) // newDrawSurf.primarySortKey - depthhack
                //     | HIDWORD(newDrawSurf.packed) & 0xF003FFFF; // Copy everything higher than 32 except for 10bits (Surftype and primarySortKey, we did those already)

                ++drawSurf;
            }
            goto LABEL_17;
        }
    }
    return drawSurf;
}

bool __cdecl R_EndFencePending()
{
    _BYTE v2[4]; // [esp+Ch] [ebp-4h] BYREF

    return frontEndDataOut->endFence && frontEndDataOut->endFence->GetData(v2, 4u, 1u) == 1;
}

void __cdecl R_SetEndTime(int endTime)
{
    rg.endTime = endTime;
}

void __cdecl R_WaitEndTime()
{
    CL_ResetStats_f();
    //Profile_Begin(361);
    while ((int)(Sys_Milliseconds() - rg.endTime) < 0)
        NET_Sleep(1u);
    //Profile_EndInternal(0);
}

void __cdecl R_InitScene()
{
    scene.drawSurfs[0] = (GfxDrawSurf *)&scene;
    scene.drawSurfs[1] = scene.smodelDrawSurfsLight;
    scene.drawSurfs[2] = scene.entDrawSurfsLight;
    scene.drawSurfs[3] = scene.bspDrawSurfsDecal;
    scene.drawSurfs[4] = scene.smodelDrawSurfsDecal;
    scene.drawSurfs[5] = scene.entDrawSurfsDecal;
    scene.drawSurfs[6] = scene.fxDrawSurfsLight;
    scene.drawSurfs[7] = scene.fxDrawSurfsLightAuto;
    scene.drawSurfs[8] = scene.fxDrawSurfsLightDecal;
    scene.drawSurfs[9] = scene.bspDrawSurfsEmissive;
    scene.drawSurfs[10] = scene.smodelDrawSurfsEmissive;
    scene.drawSurfs[11] = scene.entDrawSurfsEmissive;
    scene.drawSurfs[12] = scene.fxDrawSurfsEmissive;
    scene.drawSurfs[13] = scene.fxDrawSurfsEmissiveAuto;
    scene.drawSurfs[14] = scene.fxDrawSurfsEmissiveDecal;
    scene.drawSurfs[15] = scene.bspSunShadowDrawSurfs0;
    scene.drawSurfs[16] = scene.smodelSunShadowDrawSurfs0;
    scene.drawSurfs[17] = scene.entSunShadowDrawSurfs0;
    scene.drawSurfs[18] = scene.bspSunShadowDrawSurfs1;
    scene.drawSurfs[19] = scene.smodelSunShadowDrawSurfs1;
    scene.drawSurfs[20] = scene.entSunShadowDrawSurfs1;
    scene.drawSurfs[21] = scene.bspSpotShadowDrawSurfs0;
    scene.drawSurfs[22] = scene.smodelSpotShadowDrawSurfs0;
    scene.drawSurfs[23] = scene.entSpotShadowDrawSurfs0;
    scene.drawSurfs[24] = scene.bspSpotShadowDrawSurfs1;
    scene.drawSurfs[25] = scene.smodelSpotShadowDrawSurfs1;
    scene.drawSurfs[26] = scene.entSpotShadowDrawSurfs1;
    scene.drawSurfs[27] = scene.bspSpotShadowDrawSurfs2;
    scene.drawSurfs[28] = scene.smodelSpotShadowDrawSurfs2;
    scene.drawSurfs[29] = scene.entSpotShadowDrawSurfs2;
    scene.drawSurfs[30] = scene.bspSpotShadowDrawSurfs3;
    scene.drawSurfs[31] = scene.smodelSpotShadowDrawSurfs3;
    scene.drawSurfs[32] = scene.entSpotShadowDrawSurfs3;
    scene.drawSurfs[33] = scene.shadowDrawSurfs;
    scene.maxDrawSurfCount[0] = 0x2000;
    scene.maxDrawSurfCount[1] = 0x2000;
    scene.maxDrawSurfCount[2] = 0x2000;
    scene.maxDrawSurfCount[3] = 512;
    scene.maxDrawSurfCount[4] = 512;
    scene.maxDrawSurfCount[5] = 512;
    scene.maxDrawSurfCount[6] = 0x2000;
    scene.maxDrawSurfCount[7] = 0x2000;
    scene.maxDrawSurfCount[8] = 0x2000;
    scene.maxDrawSurfCount[9] = 0x2000;
    scene.maxDrawSurfCount[10] = 0x2000;
    scene.maxDrawSurfCount[11] = 0x2000;
    scene.maxDrawSurfCount[12] = 0x2000;
    scene.maxDrawSurfCount[13] = 0x2000;
    scene.maxDrawSurfCount[14] = 0x2000;
    scene.maxDrawSurfCount[15] = 4096;
    scene.maxDrawSurfCount[16] = 4096;
    scene.maxDrawSurfCount[17] = 4096;
    scene.maxDrawSurfCount[18] = 0x2000;
    scene.maxDrawSurfCount[19] = 0x2000;
    scene.maxDrawSurfCount[20] = 0x2000;
    scene.maxDrawSurfCount[21] = 256;
    scene.maxDrawSurfCount[22] = 256;
    scene.maxDrawSurfCount[23] = 512;
    scene.maxDrawSurfCount[24] = 256;
    scene.maxDrawSurfCount[25] = 256;
    scene.maxDrawSurfCount[26] = 512;
    scene.maxDrawSurfCount[27] = 256;
    scene.maxDrawSurfCount[28] = 256;
    scene.maxDrawSurfCount[29] = 512;
    scene.maxDrawSurfCount[30] = 256;
    scene.maxDrawSurfCount[31] = 256;
    scene.maxDrawSurfCount[32] = 512;
    scene.maxDrawSurfCount[33] = 512;
}

void __cdecl R_ClearScene(unsigned int localClientNum)
{
    unsigned int viewIndex; // [esp+0h] [ebp-4h]

    if (!rg.inFrame)
        MyAssertHandler(".\\r_scene.cpp", 2422, 0, "%s", "rg.inFrame");
    if (!Sys_IsMainThread() && !Sys_IsRenderThread())
        MyAssertHandler(".\\r_scene.cpp", 2423, 0, "%s", "Sys_IsMainThread() || Sys_IsRenderThread()");
    scene.dpvs.localClientNum = localClientNum;
    Com_Memset((unsigned int *)scene.sceneDObj, 0, 124 * scene.sceneDObjCount);
    Com_Memset((unsigned int *)&scene.sceneModel[0].info, 0, 72 * scene.sceneModelCount);
    Com_Memset((unsigned int *)&scene.sceneBrush[0].info.surfId, 0, 40 * scene.sceneBrushCount);
    scene.addedLightCount = 0;
    memset((unsigned __int8 *)scene.drawSurfCount, 0, sizeof(scene.drawSurfCount));
    for (viewIndex = 0; viewIndex < 7; ++viewIndex)
        Com_Memset((unsigned int *)scene.sceneModelVisData[viewIndex], 1, scene.sceneModelCount);
    scene.sceneDObjCount = 0;
    scene.sceneModelCount = 0;
    scene.sceneBrushCount = 0;
    if (rgp.world)
        R_ClearDpvsScene();
}

unsigned int __cdecl R_GetLocalClientNum()
{
    return scene.dpvs.localClientNum;
}

void __cdecl R_SetLodOrigin(const refdef_s *refdef)
{
    if (r_lockPvs->modified)
    {
        Dvar_ClearModified((dvar_t*)r_lockPvs);
        R_SetViewParmsForScene(refdef, &lockPvsViewParms);
    }
    R_UpdateLodParms(refdef, &rg.lodParms);
    scene.def.viewOffset[0] = refdef->viewOffset[0];
    scene.def.viewOffset[1] = refdef->viewOffset[1];
    scene.def.viewOffset[2] = refdef->viewOffset[2];
    scene.def.time = refdef->time;
    scene.def.floatTime = (double)scene.def.time * 0.001000000047497451;
    R_UpdateFrameFog();
    R_UpdateFrameSun();
}

unsigned __int8 R_UpdateFrameFog()
{
    unsigned __int8 result; // al
    GfxFog *p_fogSettings; // edx
    float lerpPos; // [esp+8h] [ebp-8h]
    int fadeTime; // [esp+Ch] [ebp-4h]

    result = (unsigned __int8)frontEndDataOut;
    if (!frontEndDataOut->viewInfoCount)
    {
        if (scene.def.time < rg.fogSettings[4].finishTime)
        {
            fadeTime = rg.fogSettings[4].finishTime - rg.fogSettings[4].startTime;
            if (rg.fogSettings[4].finishTime - rg.fogSettings[4].startTime <= 0)
                fadeTime = 1;
            lerpPos = (double)(scene.def.time - rg.fogSettings[4].startTime) / (double)fadeTime;
            if (lerpPos > 1.0)
                lerpPos = 1.0;
            rg.fogSettings[2].fogStart = (rg.fogSettings[4].fogStart - rg.fogSettings[3].fogStart) * lerpPos
                + rg.fogSettings[3].fogStart;
            rg.fogSettings[2].density = (rg.fogSettings[4].density - rg.fogSettings[3].density) * lerpPos
                + rg.fogSettings[3].density;
            rg.fogSettings[2].color.array[0] = LerpByte(
                rg.fogSettings[3].color.array[0],
                rg.fogSettings[4].color.array[0],
                lerpPos);
            rg.fogSettings[2].color.array[1] = LerpByte(
                rg.fogSettings[3].color.array[1],
                rg.fogSettings[4].color.array[1],
                lerpPos);
            rg.fogSettings[2].color.array[2] = LerpByte(
                rg.fogSettings[3].color.array[2],
                rg.fogSettings[4].color.array[2],
                lerpPos);
            result = LerpByte(rg.fogSettings[3].color.array[3], rg.fogSettings[4].color.array[3], lerpPos);
            rg.fogSettings[2].color.array[3] = result;
        }
        else
        {
            rg.fogSettings[2] = rg.fogSettings[4];
            result = LOBYTE(rg.fogSettings[4].density);
        }
    }
    p_fogSettings = &frontEndDataOut->fogSettings;
    if (rg.fogIndex)
    {
        frontEndDataOut->fogSettings.startTime = rg.fogSettings[2].startTime;
        p_fogSettings->finishTime = rg.fogSettings[2].finishTime;
        p_fogSettings->color.packed = rg.fogSettings[2].color.packed;
        p_fogSettings->fogStart = rg.fogSettings[2].fogStart;
        result = LOBYTE(rg.fogSettings[2].density);
        p_fogSettings->density = rg.fogSettings[2].density;
    }
    else
    {
        frontEndDataOut->fogSettings.startTime = 0;
        p_fogSettings->finishTime = 0;
        p_fogSettings->color.packed = 0;
        p_fogSettings->fogStart = 0.0;
        p_fogSettings->density = 0.0;
    }
    return result;
}

unsigned __int8 __cdecl LerpByte(unsigned __int8 from, unsigned __int8 to, float frac)
{
    return (int)((double)from + (double)(to - from) * frac);
}

void __cdecl R_SetViewParmsForScene(const refdef_s *refdef, GfxViewParms *viewParms)
{
    float DefaultNearClip; // [esp+Ch] [ebp-24h]

    memset((unsigned __int8 *)viewParms, 0, sizeof(GfxViewParms));
    viewParms->origin[0] = refdef->vieworg[0];
    viewParms->origin[1] = refdef->vieworg[1];
    viewParms->origin[2] = refdef->vieworg[2];
    viewParms->origin[3] = 1.0;
    viewParms->axis[0][0] = refdef->viewaxis[0][0];
    viewParms->axis[0][1] = refdef->viewaxis[0][1];
    viewParms->axis[0][2] = refdef->viewaxis[0][2];
    viewParms->axis[1][0] = refdef->viewaxis[1][0];
    viewParms->axis[1][1] = refdef->viewaxis[1][1];
    viewParms->axis[1][2] = refdef->viewaxis[1][2];
    viewParms->axis[2][0] = refdef->viewaxis[2][0];
    viewParms->axis[2][1] = refdef->viewaxis[2][1];
    viewParms->axis[2][2] = refdef->viewaxis[2][2];
    MatrixForViewer(viewParms->viewMatrix.m, viewParms->origin, viewParms->axis);
    if (refdef->zNear <= 0.0)
        DefaultNearClip = R_GetDefaultNearClip();
    else
        DefaultNearClip = refdef->zNear;
    viewParms->zNear = DefaultNearClip;
    if (viewParms->zNear == 0.0)
        MyAssertHandler(".\\r_scene.cpp", 2471, 0, "%s", "viewParms->zNear");
    R_SetupProjection(refdef->tanHalfFovX, refdef->tanHalfFovY, viewParms);
    R_SetupViewProjectionMatrices(viewParms);
}

void __cdecl R_SetupProjection(float tanHalfFovX, float tanHalfFovY, GfxViewParms *viewParms)
{
    InfinitePerspectiveMatrix(viewParms->projectionMatrix.m, tanHalfFovX, tanHalfFovY, viewParms->zNear);
    viewParms->depthHackNearClip = -r_znear_depthhack->current.value;
}

bool R_UpdateFrameSun()
{
    bool result; // eax
    bool v1; // [esp+10h] [ebp-24h]
    int v2; // [esp+14h] [ebp-20h]
    int v3; // [esp+18h] [ebp-1Ch]
    float *color; // [esp+2Ch] [ebp-8h]
    float *dir; // [esp+30h] [ebp-4h]

    if (!rgp.world)
        MyAssertHandler(".\\r_scene.cpp", 2535, 0, "%s", "rgp.world");
    if (!rgp.world->sunLight)
        MyAssertHandler(".\\r_scene.cpp", 2536, 0, "%s", "rgp.world->sunLight");
    memcpy(&frontEndDataOut->sunLight, rgp.world->sunLight, sizeof(frontEndDataOut->sunLight));
    if (rg.useSunDirOverride)
    {
        if (rg.useSunDirLerp)
        {
            R_LerpDir(
                rg.sunDirOverride,
                rg.sunDirOverrideTarget,
                rg.sunDirLerpBeginTime,
                rg.sunDirLerpEndTime,
                scene.def.time,
                frontEndDataOut->sunLight.dir);
        }
        else
        {
            dir = frontEndDataOut->sunLight.dir;
            frontEndDataOut->sunLight.dir[0] = rg.sunDirOverride[0];
            dir[1] = rg.sunDirOverride[1];
            dir[2] = rg.sunDirOverride[2];
        }
    }
    if (rg.useSunLightOverride)
    {
        color = frontEndDataOut->sunLight.color;
        frontEndDataOut->sunLight.color[0] = rg.sunLightOverride[0];
        color[1] = rg.sunLightOverride[1];
        color[2] = rg.sunLightOverride[2];
    }
    v2 = 0;
    const DvarValue &dvarVal = r_lightTweakSunDirection->current;
    if (sm_enable->current.enabled)
    {
        if (rg.useSunDirOverride
            || ((r_lightTweakSunDirection->reset.vector[0] != r_lightTweakSunDirection->current.vector[0]
                || r_lightTweakSunDirection->reset.vector[1] != r_lightTweakSunDirection->current.vector[1]
                || r_lightTweakSunDirection->reset.vector[2] != r_lightTweakSunDirection->current.vector[2])
                ? (v3 = 0)
                : (v3 = 1),
                !v3))
        {
            v2 = 1;
        }
    }
    frontEndDataOut->prim.hasSunDirChanged = v2;
    v1 = rg.useSunDirOverride
        || AngleDelta(r_lightTweakSunDirection->current.vector[0], r_lightTweakSunDirection->reset.vector[0]) > 5.0
        || AngleDelta(r_lightTweakSunDirection->current.vector[1], r_lightTweakSunDirection->reset.vector[1]) > 5.0;
    result = v1;
    frontEndDataOut->hasApproxSunDirChanged = v1;
    return result;
}

void __cdecl R_LerpDir(
    const float *dirBegin,
    const float *dirEnd,
    int beginLerpTime,
    int endLerpTime,
    int currTime,
    float *result)
{
    float fraction; // [esp+8h] [ebp-28h]
    float v7; // [esp+Ch] [ebp-24h]
    float v8; // [esp+10h] [ebp-20h]
    float v9; // [esp+28h] [ebp-8h]
    float lerpFraction; // [esp+2Ch] [ebp-4h]

    if (endLerpTime <= beginLerpTime)
        MyAssertHandler(".\\r_scene.cpp", 2523, 0, "%s", "endLerpTime > beginLerpTime");
    lerpFraction = (double)(currTime - beginLerpTime) / (double)(endLerpTime - beginLerpTime);
    v8 = lerpFraction - 1.0;
    if (v8 < 0.0)
        v9 = (double)(currTime - beginLerpTime) / (double)(endLerpTime - beginLerpTime);
    else
        v9 = 1.0;
    v7 = 0.0 - lerpFraction;
    if (v7 < 0.0)
        fraction = v9;
    else
        fraction = 0.0;
    Vec3Lerp(dirBegin, dirEnd, fraction, result);
    Vec3Normalize(result);
}

void __cdecl R_UpdateLodParms(const refdef_s *refdef, GfxLodParms *lodParms)
{
    float v2; // [esp+8h] [ebp-18h]
    float value; // [esp+14h] [ebp-Ch]
    float invFovScale; // [esp+1Ch] [ebp-4h]

    if (r_lockPvs->current.enabled)
    {
        lodParms->origin[0] = lockPvsViewParms.origin[0];
        lodParms->origin[1] = lockPvsViewParms.origin[1];
        lodParms->origin[2] = lockPvsViewParms.origin[2];
    }
    else
    {
        lodParms->origin[0] = refdef->vieworg[0];
        lodParms->origin[1] = refdef->vieworg[1];
        lodParms->origin[2] = refdef->vieworg[2];
    }
    invFovScale = refdef->tanHalfFovY * 2.118673086166382;
    value = r_lodBiasRigid->current.value;
    lodParms->ramp[0].scale = r_lodScaleRigid->current.value * invFovScale;
    lodParms->ramp[0].bias = value * invFovScale;
    v2 = r_lodBiasSkinned->current.value;
    lodParms->ramp[1].scale = r_lodScaleSkinned->current.value * invFovScale;
    lodParms->ramp[1].bias = v2 * invFovScale;
    lodParms->valid = 1;
}

void __cdecl R_CorrectLodScale(const refdef_s *refdef)
{
    R_UpdateLodParms(refdef, &rg.correctedLodParms);
}

void __cdecl R_RenderScene(const refdef_s *refdef)
{
    GfxViewParms *v1; // [esp+0h] [ebp-E8h]
    GfxSceneParms sceneParms; // [esp+40h] [ebp-A8h] BYREF
    GfxViewParms *viewParmsDraw; // [esp+E4h] [ebp-4h]

    if (refdef->tanHalfFovX <= 0.0)
        MyAssertHandler(".\\r_scene.cpp", 2631, 0, "%s", "refdef->tanHalfFovX > 0");
    if (refdef->tanHalfFovY <= 0.0)
        MyAssertHandler(".\\r_scene.cpp", 2632, 0, "%s", "refdef->tanHalfFovY > 0");
    if (!refdef->height)
        MyAssertHandler(".\\r_scene.cpp", 2633, 0, "%s", "refdef->height > 0");
    if (!refdef->width)
        MyAssertHandler(".\\r_scene.cpp", 2634, 0, "%s", "refdef->width > 0");
    if (refdef->localClientNum >= gfxCfg.maxClientViews)
        MyAssertHandler(
            ".\\r_scene.cpp",
            2635,
            0,
            "refdef->localClientNum doesn't index gfxCfg.maxClientViews\n\t%i not in [0, %i)",
            refdef->localClientNum,
            gfxCfg.maxClientViews);
    if (rg.registered && !r_norefresh->current.enabled)
    {
        //Profile_Begin(14);
        if (r_logFile->current.integer)
            RB_LogPrint("====== R_RenderScene ======\n");
        if (!rgp.world)
            Com_Error(ERR_DROP, "R_RenderScene: NULL worldmodel");
        rg.viewOrg[0] = refdef->vieworg[0];
        rg.viewOrg[1] = refdef->vieworg[1];
        rg.viewOrg[2] = refdef->vieworg[2];
        rg.viewDir[0] = refdef->viewaxis[0][0];
        rg.viewDir[1] = refdef->viewaxis[0][1];
        rg.viewDir[2] = refdef->viewaxis[0][2];
        viewParmsDraw = R_AllocViewParms();
        R_SetViewParmsForScene(refdef, viewParmsDraw);
        R_SetSceneParms(refdef, &sceneParms);
        R_CorrectLodScale(refdef);
        if (scene.def.time != refdef->time)
            MyAssertHandler(".\\r_scene.cpp", 2666, 0, "%s", "scene.def.time == refdef->time");
        if (!rg.lodParms.valid)
            MyAssertHandler(".\\r_scene.cpp", 2668, 0, "%s", "rg.lodParms.valid");
        if (r_lockPvs->current.enabled)
            v1 = &lockPvsViewParms;
        else
            v1 = viewParmsDraw;
        R_GenerateSortedDrawSurfs(&sceneParms, v1, viewParmsDraw);
        //Profile_EndInternal(0);
    }
}

char __cdecl R_DoesDrawSurfListInfoNeedFloatz(GfxDrawSurfListInfo *emissiveInfo)
{
    const MaterialTechnique *technique; // [esp+38h] [ebp-Ch]
    unsigned int surfIndex; // [esp+3Ch] [ebp-8h]

    Profile_Begin(70);
    for (surfIndex = 0; ; ++surfIndex)
    {
        if (surfIndex == emissiveInfo->drawSurfCount)
        {
            Profile_EndInternal(0);
            return 0;
        }
        technique = Material_GetTechnique(rgp.sortedMaterials[emissiveInfo->drawSurfs[surfIndex].fields.materialSortedIndex], emissiveInfo->baseTechType);
        if (technique)
        {
            if ((technique->flags & 0x20) != 0)
                break;
        }
    }
    Profile_EndInternal(0);
    if (!gfxRenderTargets[5].surface.color)
        Com_Error(ERR_FATAL, "Renderer attempted to use technique that uses floatz buffer, but it wasn't created.\n");
    return 1;
}

void __cdecl R_GenerateSortedDrawSurfs(
    const GfxSceneParms *sceneParms,
    const GfxViewParms *viewParmsDpvs,
    const GfxViewParms *viewParmsDraw)
{
    MaterialTechniqueType EmissiveTechnique; // eax
    char DoesDrawSurfListInfoNeedFloatz; // al
    float x; // [esp+1Ch] [ebp-15Ch]
    float v7; // [esp+30h] [ebp-148h]
    float v8; // [esp+4Ch] [ebp-12Ch]
    float v9; // [esp+70h] [ebp-108h]
    float *v10; // [esp+7Ch] [ebp-FCh]
    float *v11; // [esp+88h] [ebp-F0h]
    float *viewOrigin; // [esp+BCh] [ebp-BCh]
    unsigned int data[20]; // [esp+C4h] [ebp-B4h] BYREF
    float v15; // [esp+114h] [ebp-64h]
    float v16; // [esp+118h] [ebp-60h]
    float v17; // [esp+11Ch] [ebp-5Ch]
    float bestError; // [esp+120h] [ebp-58h]
    unsigned int bestNum; // [esp+124h] [ebp-54h]
    float error; // [esp+128h] [ebp-50h]
    unsigned int num; // [esp+12Ch] [ebp-4Ch]
    unsigned int bestDen; // [esp+130h] [ebp-48h]
    unsigned int den; // [esp+134h] [ebp-44h]
    int pointLightCount; // [esp+138h] [ebp-40h]
    int firstDrawSurfCount; // [esp+13Ch] [ebp-3Ch]
    int cameraCellIndex; // [esp+140h] [ebp-38h]
    GfxDrawSurfListInfo *litInfo; // [esp+144h] [ebp-34h]
    GfxDrawSurfListInfo *decalInfo; // [esp+148h] [ebp-30h]
    GfxDrawSurfListInfo *emissiveInfo; // [esp+14Ch] [ebp-2Ch]
    bool usePreTess; // [esp+153h] [ebp-25h]
    GfxViewInfo *viewInfo; // [esp+154h] [ebp-24h]
    int viewInfoIndex; // [esp+158h] [ebp-20h]
    const GfxLight *visibleLights[4]; // [esp+15Ch] [ebp-1Ch] BYREF
    ShadowType dynamicShadowType; // [esp+16Ch] [ebp-Ch]
    SceneEntCmd sceneEntCmd; // [esp+170h] [ebp-8h] BYREF
    int visibleLightCount; // [esp+174h] [ebp-4h]

    if (frontEndDataOut->viewInfoCount != rg.viewInfoCount)
        MyAssertHandler(".\\r_scene.cpp", 1783, 0, "%s", "frontEndDataOut->viewInfoCount == rg.viewInfoCount");
    viewInfoIndex = rg.viewInfoCount++;
    frontEndDataOut->viewInfoCount = rg.viewInfoCount;
    if ((unsigned int)viewInfoIndex >= 4)
        MyAssertHandler(
            ".\\r_scene.cpp",
            1789,
            1,
            "viewInfoIndex doesn't index GFX_MAX_CLIENT_VIEWS\n\t%i not in [0, %i)",
            viewInfoIndex,
            4);
    frontEndDataOut->viewInfoIndex = viewInfoIndex;
    viewInfo = &frontEndDataOut->viewInfo[viewInfoIndex];
    dynamicShadowType = (ShadowType)R_DynamicShadowType();
    rg.sunShadowFull = r_rendererInUse->current.integer == 1;
    if (rg.sunShadowFull)
    {
        rg.sunShadowmapScale = sm_sunShadowScale->current.value;
        bestDen = 1;
        bestNum = 1;
        bestError = 1.0;
        for (den = 1; den <= 0xA; ++den)
        {
            v17 = (double)den * rg.sunShadowmapScale + 0.5;
            v9 = floor(v17);
            num = (__int64)v9;
            v16 = (double)num / (double)den - rg.sunShadowmapScale;
            v8 = fabs(v16);
            error = v8;
            if (bestError > (double)v8)
            {
                bestError = error;
                bestDen = den;
                bestNum = num;
            }
        }
        rg.sunShadowmapScale = (double)bestNum / (double)bestDen;
        rg.sunShadowmapScaleNum = (float)bestNum;
        v15 = rg.sunShadowmapScale * 1024.0;
        v7 = ceil(v15);
        rg.sunShadowSize = (__int64)v7;
        rg.sunShadowPartitionRatio = 4.0 / rg.sunShadowmapScale;
    }
    else
    {
        rg.sunShadowSize = 1024;
        rg.sunShadowmapScale = 1.0;
        rg.sunShadowmapScaleNum = 1.0;
        rg.sunShadowPartitionRatio = 4.0;
    }
    rg.drawSunShadow = 0;
    if (!rg.lodParms.valid)
        MyAssertHandler(".\\r_scene.cpp", 1843, 0, "%s", "rg.lodParms.valid");
    if (!rg.correctedLodParms.valid)
        MyAssertHandler(".\\r_scene.cpp", 1844, 0, "%s", "rg.correctedLodParms.valid");
    memcpy(&viewInfo->input, &gfxCmdBufInput, sizeof(viewInfo->input));
    viewInfo->input.data = frontEndDataOut;
    viewInfo->sceneDef = scene.def;
    memcpy((void *)viewInfo, viewParmsDraw, 0x140u);
    viewInfo->sceneViewport = sceneParms->sceneViewport;
    viewInfo->displayViewport = sceneParms->displayViewport;
    viewInfo->scissorViewport = sceneParms->scissorViewport;
    viewInfo->dynamicShadowType = dynamicShadowType;
    viewInfo->localClientNum = sceneParms->localClientNum;
    viewInfo->isRenderingFullScreen = sceneParms->isRenderingFullScreen;
    viewInfo->blurRadius = sceneParms->blurRadius;
    viewInfo->spotShadowCount = 0;
    viewInfo->needsFloatZ = 0;
    Com_Memcpy((char *)viewInfo->shadowableLights, (char *)sceneParms->primaryLights, rgp.world->primaryLightCount << 6);
    viewInfo->shadowableLightCount = rgp.world->primaryLightCount;
    scene.shadowableLightIsUsed[0] = 0;
    scene.shadowableLightIsUsed[1] = 0;
    scene.shadowableLightIsUsed[2] = 0;
    scene.shadowableLightIsUsed[3] = 0;
    scene.shadowableLightIsUsed[4] = 0;
    scene.shadowableLightIsUsed[5] = 0;
    scene.shadowableLightIsUsed[6] = 0;
    scene.shadowableLightIsUsed[7] = 0;
    R_SetupDynamicModelLighting(&viewInfo->input);
    R_SetFrameFog(&viewInfo->input);
    R_SetSunConstants(&viewInfo->input);
    R_SetDepthOfField(viewInfo, sceneParms);
    R_SetFilmInfo(viewInfo, sceneParms);
    R_SetGlowInfo(viewInfo, sceneParms);
    R_DrawCineWarning();
    x = viewParmsDraw->zNear * 0.984375;
    R_SetInputCodeConstant(&viewInfo->input, 0x22u, x, 0.0, 0.0, 0.0);
    R_SetFullSceneViewMesh(viewInfoIndex, viewInfo);
    R_SetupWorldSurfacesDpvs(viewParmsDpvs);
    R_SetViewFrustumPlanes(viewInfo);
    cameraCellIndex = R_CellForPoint(viewParmsDpvs->origin);
    CL_ResetStats_f();
    R_FilterEntitiesIntoCells(cameraCellIndex);
    CL_ResetStats_f();
    //Profile_Begin(71);
    R_AddWorldSurfacesDpvs(viewParmsDpvs, cameraCellIndex);
    //Profile_EndInternal(0);
    R_BeginAllStaticModelLighting();
    CL_ResetStats_f();
    //Profile_Begin(134);
    R_WaitWorkerCmdsOfType(0);
    //Profile_EndInternal(0);
    R_AddEmissiveSpotLight(viewInfo);
    usePreTess = !dx.deviceLost && r_pretess->current.enabled;
    if (usePreTess)
        R_BeginPreTess();
    R_WaitWorkerCmdsOfType(3);
    CL_ResetStats_f();
    if (gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD)
    {
        R_AddAllBspDrawSurfacesCamera();
        if (!sm_sunEnable->current.enabled && rgp.world->sunPrimaryLightIndex)
            Com_BitClearAssert(scene.shadowableLightIsUsed, rgp.world->sunPrimaryLightIndex, 128);
        Com_Memset(frontEndDataOut->shadowableLightHasShadowMap, 0, 32);
        if (R_GetAllowShadowMaps())
            R_ChooseShadowedLights(viewInfo);
        R_UpdateDrawMethod(frontEndDataOut, viewInfo);
        if (dynamicShadowType == SHADOW_MAP
            && Com_BitCheckAssert(frontEndDataOut->shadowableLightHasShadowMap, rgp.world->sunPrimaryLightIndex, 32))
        {
            rg.drawSunShadow = 1;
            R_SetupSunShadowMaps(viewParmsDpvs, &viewInfo->sunShadow);
            R_SetSunShadowConstants(&viewInfo->input, &viewInfo->sunShadow.sunProj);
            R_SunShadowMaps();
        }
    }
    else
    {
        R_AddAllBspDrawSurfacesCameraNonlit(rgp.world->dpvs.litSurfsBegin, rgp.world->dpvs.litSurfsEnd, 0);
        R_AddAllBspDrawSurfacesCameraNonlit(rgp.world->dpvs.decalSurfsBegin, rgp.world->dpvs.decalSurfsEnd, 3u);
    }
    R_AddAllBspDrawSurfacesCameraNonlit(rgp.world->dpvs.emissiveSurfsBegin, rgp.world->dpvs.emissiveSurfsEnd, 9u);
    R_AddAllStaticModelSurfacesCamera();
    CL_ResetStats_f();
    DynEntPieces_AddDrawSurfs();
    R_WaitWorkerCmdsOfType(5);
    R_WaitWorkerCmdsOfType(6);
    CL_ResetStats_f();
    R_DrawAllDynEnt(viewInfo);
    if (gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD
        && dynamicShadowType == SHADOW_MAP
        && Com_BitCheckAssert(frontEndDataOut->shadowableLightHasShadowMap, rgp.world->sunPrimaryLightIndex, 32))
    {
        R_WaitWorkerCmdsOfType(3);
        CL_ResetStats_f();
        R_AddAllBspDrawSurfacesSunShadow();
        CL_ResetStats_f();
        R_AddAllStaticModelSurfacesSunShadow();
    }
    CL_ResetStats_f();
    //Profile_Begin(134);
    R_WaitWorkerCmdsOfType(1);
    R_WaitWorkerCmdsOfType(2);
    //Profile_EndInternal(0);
    R_WaitWorkerCmdsOfType(4);
    R_WaitWorkerCmdsOfType(7);
    R_WaitWorkerCmdsOfType(9);
    CL_ResetStats_f();
    R_DrawAllSceneEnt(viewInfo);
    R_WaitWorkerCmdsOfType(12);
    sceneEntCmd.viewInfo = viewInfo;
    R_AddWorkerCmd(8, (unsigned __int8 *)&sceneEntCmd);
    R_SortAllStaticModelSurfacesCamera();
    visibleLightCount = R_GetVisibleDLights(visibleLights);
    R_GetLightSurfs(visibleLightCount, visibleLights);
    R_GetPointLightShadowSurfs(viewInfo, scene.visLightShadow, visibleLights);
    viewInfo->shadowCookieList.cookieCount = 0;
    if (gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD)
    {
        if (dynamicShadowType == SHADOW_MAP)
        {
            if (Com_BitCheckAssert(frontEndDataOut->shadowableLightHasShadowMap, rgp.world->sunPrimaryLightIndex, 32))
            {
                CL_ResetStats_f();
                R_AddAllSceneEntSurfacesSunShadow();
                R_SortAllStaticModelSurfacesSunShadow();
                R_GenerateSortedSunShadowDrawSurfs(viewInfo);
            }
            R_GenerateAllSortedSpotShadowDrawSurfs(viewInfo);
        }
        else if (dynamicShadowType == SHADOW_COOKIE)
        {
            data[0] = (unsigned int)viewParmsDpvs;
            data[1] = (unsigned int)viewParmsDraw;
            data[2] = (unsigned int)&viewInfo->shadowCookieList;
            data[3] = viewInfo->localClientNum;
            R_AddWorkerCmd(10, (unsigned __int8 *)data);
        }
    }
    R_SetAllStaticModelLighting();
    if (dynamicShadowType == SHADOW_COOKIE)
        R_WaitWorkerCmdsOfType(10);
    CL_ResetStats_f();
    R_EmitShadowCookieSurfs(viewInfo);
    R_WaitWorkerCmdsOfType(8);
    if (usePreTess)
        R_EndPreTess();
    litInfo = &viewInfo->litInfo;
    R_InitDrawSurfListInfo(&viewInfo->litInfo);
    litInfo->baseTechType = gfxDrawMethod.baseTechType;
    litInfo->viewInfo = viewInfo;
    viewOrigin = litInfo->viewOrigin;
    litInfo->viewOrigin[0] = viewParmsDraw->origin[0];
    viewOrigin[1] = viewParmsDraw->origin[1];
    viewOrigin[2] = viewParmsDraw->origin[2];
    viewOrigin[3] = viewParmsDraw->origin[3];
    litInfo->cameraView = 1;
    firstDrawSurfCount = frontEndDataOut->drawSurfCount;
    R_MergeAndEmitDrawSurfLists(0, 3u);
    litInfo->drawSurfs = &frontEndDataOut->drawSurfs[firstDrawSurfCount];
    litInfo->drawSurfCount = frontEndDataOut->drawSurfCount - firstDrawSurfCount;
    pointLightCount = 0;
    if (r_dlightLimit->current.integer && gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD)
    {
        CL_ResetStats_f();
        pointLightCount = R_EmitPointLightPartitionSurfs(viewInfo, visibleLights, visibleLightCount, viewParmsDpvs->origin);
    }
    viewInfo->pointLightCount = pointLightCount;
    if (!viewInfo->localClientNum)
        CL_UpdateSound();
    FX_RunPhysics(viewInfo->localClientNum);
    DynEntCl_ProcessEntities(viewInfo->localClientNum);
    R_WaitWorkerCmdsOfType(14);
    if (!dx.deviceLost && fx_marks->current.enabled)
    {
        if (fx_marks_smodels->current.enabled)
            FX_GenerateMarkVertsForStaticModels(
                viewInfo->localClientNum,
                rgp.world->dpvs.smodelCount,
                rgp.world->dpvs.smodelVisData[0]);
        if (fx_marks_ents->current.enabled)
            R_GenerateMarkVertsForDynamicModels();
    }
    CL_ResetStats_f();
    R_SortDrawSurfs(scene.drawSurfs[6], scene.drawSurfCount[6]);
    decalInfo = &viewInfo->decalInfo;
    R_InitDrawSurfListInfo(&viewInfo->decalInfo);
    decalInfo->baseTechType = gfxDrawMethod.baseTechType;
    decalInfo->viewInfo = viewInfo;
    v11 = decalInfo->viewOrigin;
    decalInfo->viewOrigin[0] = viewParmsDraw->origin[0];
    v11[1] = viewParmsDraw->origin[1];
    v11[2] = viewParmsDraw->origin[2];
    v11[3] = viewParmsDraw->origin[3];
    decalInfo->cameraView = 1;
    firstDrawSurfCount = frontEndDataOut->drawSurfCount;
    R_MergeAndEmitDrawSurfLists(3u, 6u);
    decalInfo->drawSurfs = &frontEndDataOut->drawSurfs[firstDrawSurfCount];
    decalInfo->drawSurfCount = frontEndDataOut->drawSurfCount - firstDrawSurfCount;
    R_WaitWorkerCmdsOfType(13);
    CL_ResetStats_f();
    R_SortDrawSurfs(scene.drawSurfs[12], scene.drawSurfCount[12]);
    emissiveInfo = &viewInfo->emissiveInfo;
    R_InitDrawSurfListInfo(&viewInfo->emissiveInfo);
    EmissiveTechnique = R_GetEmissiveTechnique(viewInfo, gfxDrawMethod.emissiveTechType);
    emissiveInfo->baseTechType = EmissiveTechnique;
    emissiveInfo->viewInfo = viewInfo;
    v10 = emissiveInfo->viewOrigin;
    emissiveInfo->viewOrigin[0] = viewParmsDraw->origin[0];
    v10[1] = viewParmsDraw->origin[1];
    v10[2] = viewParmsDraw->origin[2];
    v10[3] = viewParmsDraw->origin[3];
    emissiveInfo->cameraView = 1;
    if (viewInfo->emissiveSpotLightCount)
    {
        if (viewInfo->emissiveSpotLightCount != 1)
            MyAssertHandler(".\\r_scene.cpp", 2298, 0, "%s", "viewInfo->emissiveSpotLightCount == 1");
        emissiveInfo->light = &viewInfo->emissiveSpotLight;
    }
    firstDrawSurfCount = frontEndDataOut->drawSurfCount;
    R_MergeAndEmitDrawSurfLists(9u, 6u);
    emissiveInfo->drawSurfs = &frontEndDataOut->drawSurfs[firstDrawSurfCount];
    emissiveInfo->drawSurfCount = frontEndDataOut->drawSurfCount - firstDrawSurfCount;
    if (!viewInfo->needsFloatZ)
    {
        DoesDrawSurfListInfoNeedFloatz = R_DoesDrawSurfListInfoNeedFloatz(emissiveInfo);
        viewInfo->needsFloatZ = DoesDrawSurfListInfoNeedFloatz;
    }
    R_ShowCull();
}

bool g_allowShadowMaps; // KISAKTODO: value here?
bool __cdecl R_GetAllowShadowMaps()
{
    return sm_enable->current.enabled && g_allowShadowMaps;
}

int __cdecl R_DynamicShadowType()
{
    if (R_GetAllowShadowMaps())
        return 2;
    else
        return sc_enable->current.enabled;
}

void __cdecl R_SetDepthOfField(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms)
{
    Font_s *font; // [esp+24h] [ebp-4h]

    if (r_dof_tweak->current.enabled)
    {
        if (r_dof_nearBlur->current.value < 4.0)
            MyAssertHandler(
                ".\\r_scene.cpp",
                1151,
                0,
                "%s\n\t(r_dof_nearBlur->current.value) = %g",
                "(r_dof_nearBlur->current.value >= 4.0f)",
                r_dof_nearBlur->current.value);
        viewInfo->dof.viewModelStart = r_dof_viewModelStart->current.value;
        viewInfo->dof.viewModelEnd = r_dof_viewModelEnd->current.value;
        viewInfo->dof.nearStart = r_dof_nearStart->current.value;
        viewInfo->dof.nearEnd = r_dof_nearEnd->current.value;
        viewInfo->dof.farStart = r_dof_farStart->current.value;
        viewInfo->dof.farEnd = r_dof_farEnd->current.value;
        viewInfo->dof.nearBlur = r_dof_nearBlur->current.value;
        viewInfo->dof.farBlur = r_dof_farBlur->current.value;
    }
    else if (r_dof_enable->current.enabled)
    {
        qmemcpy(&viewInfo->dof, &sceneParms->dof, sizeof(viewInfo->dof));
    }
    else
    {
        viewInfo->dof.viewModelStart = 0.0;
        viewInfo->dof.viewModelEnd = 0.0;
        viewInfo->dof.nearStart = 0.0;
        viewInfo->dof.nearEnd = 0.0;
        viewInfo->dof.farStart = 0.0;
        viewInfo->dof.farEnd = 0.0;
        viewInfo->dof.nearBlur = 0.0;
        viewInfo->dof.farBlur = 0.0;
    }
    if (R_UsingDepthOfField(viewInfo))
    {
        if (!gfxRenderTargets[5].surface.color)
            Com_Error(
                ERR_FATAL,
                "Depth of field used (enabled via r_dof_enable or r_dof_tweak) with no float-z buffer (r_floatz wasn't enabled wh"
                "en the renderer was started.)\n");
        viewInfo->needsFloatZ = 1;
        if (com_statmon->current.enabled)
        {
            font = R_RegisterFont("fonts/consoleFont", 1);
            R_AddCmdDrawText((char*)"DOF", 0x7FFFFFFF, font, 0.0, 320.0, 1.5, 2.0, 0.0, colorRedFaded, 0);
        }
    }
}

void __cdecl R_SetFilmInfo(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms)
{
    const DvarValue *p_current; // [esp+Ch] [ebp-10h]
    int v3; // [esp+14h] [ebp-8h]
    float desaturationScale; // [esp+18h] [ebp-4h]

    if (!viewInfo)
        MyAssertHandler(".\\r_scene.cpp", 1237, 0, "%s", "viewInfo");
    if (!sceneParms)
        MyAssertHandler(".\\r_scene.cpp", 1238, 0, "%s", "sceneParms");
    if (r_filmUseTweaks->current.enabled)
    {
        viewInfo->film.enabled = r_filmTweakEnable->current.enabled;
        viewInfo->film.contrast = r_filmTweakContrast->current.value;
        viewInfo->film.brightness = r_filmTweakBrightness->current.value;
        viewInfo->film.desaturation = r_filmTweakDesaturation->current.value;
        viewInfo->film.invert = r_filmTweakInvert->current.enabled;
        //v3 = LODWORD(r_lightTweakSunDirection.vector[1]) + 12;
        //viewInfo->film.tintLight[0] = *(float *)(LODWORD(r_lightTweakSunDirection.vector[1]) + 12);
        //viewInfo->film.tintLight[1] = *(float *)(v3 + 4);
        //viewInfo->film.tintLight[2] = *(float *)(v3 + 8);
        viewInfo->film.tintLight[0] = r_lightTweakSunDirection->current.vector[0];
        viewInfo->film.tintLight[1] = r_lightTweakSunDirection->current.vector[1];
        viewInfo->film.tintLight[2] = r_lightTweakSunDirection->current.vector[2];
        p_current = &r_filmTweakDarkTint->current;
        viewInfo->film.tintDark[0] = r_filmTweakDarkTint->current.value;
        viewInfo->film.tintDark[1] = p_current->vector[1];
        viewInfo->film.tintDark[2] = p_current->vector[2];
    }
    else
    {
        memcpy(&viewInfo->film, &sceneParms->film, sizeof(viewInfo->film));
    }
    desaturationScale = (1.0 - viewInfo->film.desaturation) * r_desaturation->current.value + viewInfo->film.desaturation;
    viewInfo->film.desaturation = viewInfo->film.desaturation * desaturationScale;
    viewInfo->film.contrast = viewInfo->film.contrast * r_contrast->current.value;
    viewInfo->film.brightness = viewInfo->film.brightness + r_brightness->current.value;
    R_UpdateColorManipulation(viewInfo);
}

void __cdecl R_UpdateColorManipulation(GfxViewInfo *viewInfo)
{
    float v1; // [esp+10h] [ebp-50h]
    float v2; // [esp+14h] [ebp-4Ch]
    float desaturation; // [esp+20h] [ebp-40h]
    float colorTintDelta[4]; // [esp+34h] [ebp-2Ch] BYREF
    float tintScale; // [esp+44h] [ebp-1Ch]
    float colorTintBase[4]; // [esp+48h] [ebp-18h] BYREF
    float desaturationScale; // [esp+58h] [ebp-8h]
    float tintBias; // [esp+5Ch] [ebp-4h]

    if (!viewInfo)
        MyAssertHandler(".\\r_scene.cpp", 1202, 0, "%s", "viewInfo");
    if (viewInfo->film.enabled)
    {
        desaturation = viewInfo->film.desaturation;
        v2 = 0.00024414062 - desaturation;
        if (v2 < 0.0)
            v1 = desaturation;
        else
            v1 = 0.00024414062;
        desaturationScale = 1.0 / v1 - 1.0;
        tintScale = viewInfo->film.contrast * v1;
        tintBias = viewInfo->film.brightness + 0.5 - viewInfo->film.contrast * 0.5;
        if (viewInfo->film.invert)
        {
            tintScale = -tintScale;
            tintBias = tintBias + 1.0;
        }
        Vec3Scale(viewInfo->film.tintDark, tintScale, colorTintBase);
        colorTintBase[3] = 0.0;
        Vec3Sub(viewInfo->film.tintLight, viewInfo->film.tintDark, colorTintDelta);
        Vec3Scale(colorTintDelta, tintScale, colorTintDelta);
        colorTintDelta[3] = 0.0;
        R_SetInputCodeConstant(&viewInfo->input, 0x2Du, tintBias, tintBias, tintBias, desaturationScale);
        R_SetInputCodeConstantFromVec4(&viewInfo->input, 0x2Eu, colorTintBase);
        R_SetInputCodeConstantFromVec4(&viewInfo->input, 0x2Fu, colorTintDelta);
    }
    else
    {
        R_SetInputCodeConstant(&viewInfo->input, 0x2Du, 0.0, 0.0, 0.0, 4095.0);
        R_SetInputCodeConstant(&viewInfo->input, 0x2Eu, 0.00024414062, 0.00024414062, 0.00024414062, 0.0);
        R_SetInputCodeConstant(&viewInfo->input, 0x2Fu, 0.0, 0.0, 0.0, 0.0);
    }
}

void __cdecl R_SetGlowInfo(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms)
{
    Font_s *font; // [esp+24h] [ebp-10h]
    float bloomCutoffRescale; // [esp+28h] [ebp-Ch]
    float bloomCutoff; // [esp+30h] [ebp-4h]

    if (!viewInfo)
        MyAssertHandler(".\\r_scene.cpp", 1277, 0, "%s", "viewInfo");
    if (!sceneParms)
        MyAssertHandler(".\\r_scene.cpp", 1278, 0, "%s", "sceneParms");
    if (r_glowUseTweaks->current.enabled)
    {
        viewInfo->glow.enabled = r_glowTweakEnable->current.enabled;
        viewInfo->glow.radius = r_glowTweakRadius->current.value;
        viewInfo->glow.bloomIntensity = r_glowTweakBloomIntensity->current.value;
        viewInfo->glow.bloomCutoff = r_glowTweakBloomCutoff->current.value;
        viewInfo->glow.bloomDesaturation = r_glowTweakBloomDesaturation->current.value;
    }
    else
    {
        viewInfo->glow = sceneParms->glow;
    }
    if (viewInfo->glow.bloomIntensity < 0.0)
        MyAssertHandler(".\\r_scene.cpp", 1293, 0, "%s", "viewInfo->glow.bloomIntensity >= 0.0f");
    if (viewInfo->glow.bloomIntensity != 0.0)
    {
        bloomCutoff = viewInfo->glow.bloomCutoff;
        bloomCutoffRescale = 1.0 / (1.0 - bloomCutoff);
        R_SetInputCodeConstant(
            &viewInfo->input,
            0x2Bu,
            bloomCutoff,
            bloomCutoffRescale,
            0.0,
            viewInfo->glow.bloomDesaturation);
        R_SetInputCodeConstant(&viewInfo->input, 0x2Cu, 0.0, 0.0, 0.0, viewInfo->glow.bloomIntensity);
    }
    if (R_UsingGlow(viewInfo) && com_statmon->current.enabled)
    {
        font = R_RegisterFont("fonts/consoleFont", 1);
        R_AddCmdDrawText((char*)"GLOW", 0x7FFFFFFF, font, 0.0, 340.0, 1.5, 2.0, 0.0, colorRedFaded, 0);
    }
}

void __cdecl R_SetFullSceneViewMesh(int viewInfoIndex, GfxViewInfo *viewInfo)
{
    float width; // [esp+24h] [ebp-14h]
    float height; // [esp+28h] [ebp-10h]
    GfxQuadMeshData *quadMesh; // [esp+2Ch] [ebp-Ch]
    float x; // [esp+30h] [ebp-8h]
    float y; // [esp+34h] [ebp-4h]

    quadMesh = &gfxMeshGlob.fullSceneViewMesh[viewInfoIndex];
    viewInfo->fullSceneViewMesh = quadMesh;
    x = (float)viewInfo->sceneViewport.x;
    y = (float)viewInfo->sceneViewport.y;
    width = (float)viewInfo->sceneViewport.width;
    height = (float)viewInfo->sceneViewport.height;
    if (x != quadMesh->x || y != quadMesh->y || width != quadMesh->width || height != quadMesh->height)
    {
        R_SyncRenderThread();
        R_SetQuadMesh(quadMesh, x, y, width, height, 0.0, 0.0, 1.0, 1.0, 0xFFFFFFFF);
        if (x != quadMesh->x || y != quadMesh->y || width != quadMesh->width || height != quadMesh->height)
            MyAssertHandler(
                ".\\r_scene.cpp",
                1339,
                1,
                "%s",
                "quadMesh->x == x && quadMesh->y == y && quadMesh->width == width && quadMesh->height == height");
    }
}

void __cdecl R_GenerateSortedSunShadowDrawSurfs(GfxViewInfo *viewInfo)
{
    //Profile_Begin(73);
    R_MergeAndEmitSunShadowMapsSurfs(viewInfo);
    //Profile_EndInternal(0);
}

void __cdecl R_AddEmissiveSpotLight(GfxViewInfo *viewInfo)
{
    bool v1; // [esp+8h] [ebp-8h]

    viewInfo->emissiveSpotLightCount = 0;
    v1 = r_dlightLimit->current.integer && gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD;
    if (v1 && !R_CullDynamicSpotLightInCameraView())
    {
        if (scene.addedLightCount <= 0)
            MyAssertHandler(".\\r_scene.cpp", 1518, 0, "%s", "scene.addedLightCount > 0");
        if (scene.isAddedLightCulled[0])
            MyAssertHandler(".\\r_scene.cpp", 1519, 0, "%s", "!scene.isAddedLightCulled[0]");
        if (scene.addedLight[0].type != 2)
            MyAssertHandler(".\\r_scene.cpp", 1521, 0, "%s", "spotLight->type == GFX_LIGHT_TYPE_SPOT");
        memcpy(&viewInfo->emissiveSpotLight, scene.addedLight, sizeof(viewInfo->emissiveSpotLight));
        viewInfo->emissiveSpotLightIndex = 0;
        viewInfo->emissiveSpotDrawSurfCount = 0;
        viewInfo->emissiveSpotDrawSurfs = 0;
        viewInfo->emissiveSpotLightCount = 1;
        if (scene.addedLight[0].canUseShadowMap)
            R_AddDynamicShadowableLight(viewInfo, scene.addedLight);
    }
}

void R_GenerateMarkVertsForDynamicModels()
{
    GfxSceneModel *sceneModel; // [esp+4h] [ebp-24h]
    int dobjIndex; // [esp+8h] [ebp-20h]
    unsigned __int8 reflectionProbeIndex; // [esp+Fh] [ebp-19h]
    unsigned int indexCount; // [esp+10h] [ebp-18h] BYREF
    int brushModelIndex; // [esp+14h] [ebp-14h]
    unsigned __int16 entnum; // [esp+18h] [ebp-10h]
    unsigned __int16 lightHandle; // [esp+1Ch] [ebp-Ch]
    int modelIndex; // [esp+20h] [ebp-8h]
    GfxSceneEntity *sceneEntity; // [esp+24h] [ebp-4h]

    FX_BeginGeneratingMarkVertsForEntModels(scene.dpvs.localClientNum, &indexCount);
    for (dobjIndex = 0; dobjIndex != scene.sceneDObjCount; ++dobjIndex)
    {
        sceneEntity = &scene.sceneDObj[dobjIndex];
        entnum = sceneEntity->entnum;
        if (entnum < gfxCfg.entnumOrdinaryEnd && (scene.sceneDObjVisData[0][dobjIndex] & 1) != 0)
        {
            lightHandle = *(_WORD *)LongNoSwap((unsigned int)sceneEntity->info.pose);
            FX_GenerateMarkVertsForEntDObj(
                scene.dpvs.localClientNum,
                entnum,
                &indexCount,
                lightHandle,
                sceneEntity->reflectionProbeIndex,
                sceneEntity->obj,
                sceneEntity->info.pose);
        }
    }
    for (modelIndex = 0; modelIndex != scene.sceneModelCount; ++modelIndex)
    {
        sceneModel = &scene.sceneModel[modelIndex];
        entnum = sceneModel->entnum;
        if (entnum < gfxCfg.entnumOrdinaryEnd && (scene.sceneModelVisData[0][modelIndex] & 1) != 0)
        {
            reflectionProbeIndex = sceneModel->reflectionProbeIndex;
            if (!sceneModel->cachedLightingHandle)
                MyAssertHandler(".\\r_scene.cpp", 1574, 0, "%s", "sceneModel->cachedLightingHandle");
            if (sceneModel->obj)
                FX_GenerateMarkVertsForEntXModel(
                    scene.dpvs.localClientNum,
                    entnum,
                    &indexCount,
                    *sceneModel->cachedLightingHandle,
                    reflectionProbeIndex,
                    &sceneModel->placement);
        }
    }
    for (brushModelIndex = 0; brushModelIndex != scene.sceneBrushCount; ++brushModelIndex)
    {
        entnum = scene.sceneBrush[brushModelIndex].entnum;
        if (entnum < gfxCfg.entnumOrdinaryEnd && (scene.sceneBrushVisData[0][brushModelIndex] & 1) != 0)
            FX_GenerateMarkVertsForEntBrush(
                scene.dpvs.localClientNum,
                entnum,
                &indexCount,
                scene.sceneBrush[brushModelIndex].reflectionProbeIndex,
                &scene.sceneBrush[brushModelIndex].placement);
    }
    FX_EndGeneratingMarkVertsForEntModels(scene.dpvs.localClientNum);
}

int __cdecl R_GetVisibleDLights(const GfxLight **visibleLights)
{
    int visibleLightCount; // [esp+4h] [ebp-4h]

    CL_ResetStats_f();
    visibleLightCount = 0;
    if (r_dlightLimit->current.integer && gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD)
    {
        R_CullDynamicPointLightsInCameraView();
        CL_ResetStats_f();
        return R_GetPointLightPartitions(visibleLights);
    }
    return visibleLightCount;
}

void __cdecl R_GetLightSurfs(int visibleLightCount, const GfxLight **visibleLights)
{
    if (visibleLightCount)
    {
        //Profile_Begin(416);
        CL_ResetStats_f();
        R_GetBspLightSurfs(visibleLights, visibleLightCount);
        CL_ResetStats_f();
        R_GetStaticModelLightSurfs(visibleLights, visibleLightCount);
        CL_ResetStats_f();
        R_GetSceneEntLightSurfs(visibleLights, visibleLightCount);
        //Profile_EndInternal(0);
    }
}

void __cdecl R_GetPointLightShadowSurfs(GfxViewInfo *viewInfo, GfxVisibleLight *visibleLights, const GfxLight **lights)
{
    if (viewInfo->emissiveSpotLightCount)
    {
        if (viewInfo->emissiveSpotLightCount != 1)
            MyAssertHandler(".\\r_scene.cpp", 1649, 0, "%s", "viewInfo->emissiveSpotLightCount == 1");
        if (viewInfo->emissiveSpotLightIndex >= 4)
            MyAssertHandler(
                ".\\r_scene.cpp",
                1650,
                0,
                "viewInfo->emissiveSpotLightIndex doesn't index MAX_VISIBLE_DLIGHTS\n\t%i not in [0, %i)",
                viewInfo->emissiveSpotLightIndex,
                4);
        if (viewInfo->emissiveSpotLightIndex)
            MyAssertHandler(".\\r_scene.cpp", 1652, 0, "%s", "viewInfo->emissiveSpotLightIndex == 0");
        if ((*lights)->type != 2)
            MyAssertHandler(".\\r_scene.cpp", 1654, 0, "%s", "lights[0]->type == GFX_LIGHT_TYPE_SPOT");
        viewInfo->emissiveSpotDrawSurfCount = visibleLights->drawSurfCount;
        viewInfo->emissiveSpotDrawSurfs = visibleLights->drawSurfs;
    }
}

MaterialTechniqueType __cdecl R_GetEmissiveTechnique(const GfxViewInfo *viewInfo, MaterialTechniqueType baseTech)
{
    if (baseTech != TECHNIQUE_EMISSIVE)
        return baseTech;
    if (!viewInfo->emissiveSpotLightCount || !r_spotLightShadows->current.enabled)
        return (MaterialTechniqueType)5;
    if (!comWorld.isInUse)
        MyAssertHandler("c:\\trees\\cod3\\src\\gfx_d3d\\../qcommon/com_bsp_api.h", 23, 0, "%s", "comWorld.isInUse");
    if (!Com_BitCheckAssert(frontEndDataOut->shadowableLightHasShadowMap, comWorld.primaryLightCount, 32))
        return (MaterialTechniqueType)5;
    if (!comWorld.isInUse)
        MyAssertHandler("c:\\trees\\cod3\\src\\gfx_d3d\\../qcommon/com_bsp_api.h", 23, 0, "%s", "comWorld.isInUse");
    if (comWorld.primaryLightCount + 1 != viewInfo->shadowableLightCount)
        MyAssertHandler(
            ".\\r_scene.cpp",
            1669,
            0,
            "%s",
            "Com_GetPrimaryLightCount() + GFX_MAX_EMISSIVE_SPOT_LIGHTS == viewInfo->shadowableLightCount");
    return (MaterialTechniqueType)6;
}

void __cdecl R_SetSunShadowConstants(GfxCmdBufInput *input, const GfxSunShadowProjection *sunProj)
{
    R_SetInputCodeConstantFromVec4(input, 0x20u, (float*)sunProj->switchPartition);
    R_SetInputCodeConstantFromVec4(input, 0x21u, (float*)sunProj->shadowmapScale);
}

void __cdecl R_SetSunConstants(GfxCmdBufInput *input)
{
    const GfxLight *sun; // [esp+1Ch] [ebp-10h]
    float specularColor[3]; // [esp+20h] [ebp-Ch] BYREF

    if (input->data->sunLight.type != 1)
        MyAssertHandler(".\\r_scene.cpp", 1691, 0, "%s", "input->data->sunLight.type == GFX_LIGHT_TYPE_DIR");
    sun = &input->data->sunLight;
    Vec3Scale(input->data->sunLight.color, r_specularColorScale->current.value, specularColor);
    R_SetInputCodeConstantFromVec4(input, 0x23u, (float*)sun->dir);
    R_SetInputCodeConstant(input, 0x24u, sun->color[0], sun->color[1], sun->color[2], 1.0);
    R_SetInputCodeConstant(input, 0x25u, specularColor[0], specularColor[1], specularColor[2], 1.0);
}

void R_DrawCineWarning()
{
    Font_s *font; // [esp+1Ch] [ebp-8h]
    const char *msg; // [esp+20h] [ebp-4h]

    if (com_statmon->current.enabled && R_Cinematic_IsStarted())
    {
        msg = "CINE";
        font = R_RegisterFont("fonts/consoleFont", 1);
        if (R_Cinematic_IsUnderrun())
            msg = "CINE UNDERRUN!";
        R_AddCmdDrawText((char*)msg, 0x7FFFFFFF, font, 0.0, 360.0, 1.5, 2.0, 0.0, colorRedFaded, 0);
    }
}

void __cdecl R_SetSceneParms(const refdef_s *refdef, GfxSceneParms *sceneParms)
{
    bool v2; // [esp+10h] [ebp-4h]

    if (!refdef)
        MyAssertHandler(".\\r_scene.cpp", 2480, 0, "%s", "refdef");
    if (!sceneParms)
        MyAssertHandler(".\\r_scene.cpp", 2481, 0, "%s", "sceneParms");
    if (refdef->blurRadius < 0.0)
        MyAssertHandler(
            ".\\r_scene.cpp",
            2482,
            0,
            "%s\n\t(refdef->blurRadius) = %g",
            "(refdef->blurRadius >= 0.0f)",
            refdef->blurRadius);
    sceneParms->localClientNum = refdef->localClientNum;
    sceneParms->blurRadius = refdef->blurRadius;
    memcpy(&sceneParms->dof, &refdef->dof, sizeof(sceneParms->dof));
    memcpy(&sceneParms->film, &refdef->film, 0x40u);
    v2 = refdef->width == vidConfig.displayWidth && refdef->height == vidConfig.displayHeight;
    sceneParms->isRenderingFullScreen = v2;
    sceneParms->displayViewport.x = refdef->x;
    sceneParms->displayViewport.y = refdef->y;
    sceneParms->displayViewport.width = refdef->width;
    sceneParms->displayViewport.height = refdef->height;
    sceneParms->sceneViewport.x = vidConfig.sceneWidth * refdef->x / vidConfig.displayWidth;
    sceneParms->sceneViewport.y = vidConfig.sceneHeight * refdef->y / vidConfig.displayHeight;
    sceneParms->sceneViewport.width = vidConfig.sceneWidth * (refdef->width + refdef->x) / vidConfig.displayWidth
        - sceneParms->sceneViewport.x;
    sceneParms->sceneViewport.height = vidConfig.sceneHeight * (refdef->height + refdef->y) / vidConfig.displayHeight
        - sceneParms->sceneViewport.y;
    if (refdef->useScissorViewport)
    {
        sceneParms->scissorViewport.x = vidConfig.sceneWidth * refdef->scissorViewport.x / vidConfig.displayWidth;
        sceneParms->scissorViewport.y = vidConfig.sceneHeight * refdef->scissorViewport.y / vidConfig.displayHeight;
        sceneParms->scissorViewport.width = vidConfig.sceneWidth
            * (refdef->scissorViewport.width + refdef->scissorViewport.x)
            / vidConfig.displayWidth
            - sceneParms->scissorViewport.x;
        sceneParms->scissorViewport.height = vidConfig.sceneHeight
            * (refdef->scissorViewport.height + refdef->scissorViewport.y)
            / vidConfig.displayHeight
            - sceneParms->scissorViewport.y;
    }
    else
    {
        sceneParms->scissorViewport = sceneParms->sceneViewport;
    }
    sceneParms->primaryLights = refdef->primaryLights;
}

void __cdecl R_LinkDObjEntity(unsigned int localClientNum, unsigned int entnum, float *origin, float radius)
{
    R_FilterDObjIntoCells(localClientNum, entnum, origin, radius);
    R_LinkSphereEntityToPrimaryLights(localClientNum, entnum, origin, radius);
}

void __cdecl R_LinkBModelEntity(unsigned int localClientNum, unsigned int entnum, GfxBrushModel *bmodel)
{
    R_FilterBModelIntoCells(localClientNum, entnum, bmodel);
    R_LinkBoxEntityToPrimaryLights(localClientNum, entnum, bmodel->writable.mins, bmodel->writable.maxs);
}

void __cdecl R_UnlinkEntity(unsigned int localClientNum, unsigned int entnum)
{
    R_UnfilterEntFromCells(localClientNum, entnum);
    R_UnlinkEntityFromPrimaryLights(localClientNum, entnum);
}

void __cdecl R_LinkDynEnt(unsigned int dynEntId, DynEntityDrawType drawType, float *mins, float *maxs)
{
    R_FilterDynEntIntoCells(dynEntId, drawType, mins, maxs);
    R_LinkDynEntToPrimaryLights(dynEntId, drawType, mins, maxs);
}

void __cdecl R_UnlinkDynEnt(unsigned int dynEntId, DynEntityDrawType drawType)
{
    R_UnfilterDynEntFromCells(dynEntId, drawType);
    R_UnlinkDynEntFromPrimaryLights(dynEntId, drawType);
}

