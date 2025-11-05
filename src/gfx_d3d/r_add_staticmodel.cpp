#include "r_add_staticmodel.h"
#include <qcommon/qcommon.h>
#include "r_dvars.h"
#include <universal/com_files.h>
#include <win32/win_local.h>
#include "r_init.h"
#include "r_scene.h"
#include <xanim/xmodel.h>
#include "r_model_lighting.h"
#include "r_staticmodel.h"
#include "r_draw_method.h"
#include "r_xsurface.h"
#include "r_drawsurf.h"
#include "r_pretess.h"
#include <universal/profile.h>
#include "r_buffers.h"
#include "r_staticmodelcache.h"
#include <cgame/cg_local.h>

int g_dumpStaticModelCount;
int g_dumpStaticModelFileHandle;
char g_dumpStaticModelFilePath[20] = "staticModelInfo.csv";

char __cdecl R_PreTessStaticModelCachedList(
    const XModel *model,
    unsigned __int16 *list,
    unsigned int count,
    unsigned int lod,
    int surfaceIndex,
    GfxDrawSurf drawSurf,
    GfxDrawSurfList *drawSurfList,
    GfxDelayedCmdBuf *delayedCmdBuf)
{
    const char *v9; // eax
    unsigned int baseIndex; // [esp+30h] [ebp-28h]
    unsigned __int16 *preTessIndices; // [esp+38h] [ebp-20h]
    unsigned int listIter; // [esp+40h] [ebp-18h]
    unsigned int firstIndex; // [esp+44h] [ebp-14h]
    unsigned __int16 *dstIndices; // [esp+48h] [ebp-10h]
    unsigned int preTessSurf; // [esp+4Ch] [ebp-Ch]
    const XSurface *xsurf; // [esp+50h] [ebp-8h]
    unsigned int surfIndexCount; // [esp+54h] [ebp-4h]

    xsurf = XModelGetSurface(model, lod, surfaceIndex);
    surfIndexCount = 3 * xsurf->triCount;
    preTessIndices = R_AllocPreTessIndices(surfIndexCount * count);
    if (!preTessIndices)
        return 0;

    {
        PROF_SCOPED("R_memcpy");
        dstIndices = preTessIndices;
        for (listIter = 0; listIter < count; ++listIter)
        {
            baseIndex = 3 * xsurf->baseTriIndex + 4 * R_GetCachedSModelSurf(list[listIter])->baseVertIndex;
            if (baseIndex >= 0x100000 || surfIndexCount + baseIndex > 0x100000)
            {
                v9 = va("%i, %i", baseIndex, surfIndexCount);
                MyAssertHandler(
                    ".\\r_add_staticmodel.cpp",
                    135,
                    0,
                    "%s\n\t%s",
                    "baseIndex < SMC_MAX_INDEX_IN_CACHE && baseIndex + surfIndexCount <= SMC_MAX_INDEX_IN_CACHE",
                    v9);
            }
            Com_Memcpy((char *)dstIndices, (char *)&gfxBuf.smodelCache.indices[baseIndex], 2 * surfIndexCount);
            dstIndices += surfIndexCount;
        }
    }

    //HIDWORD(drawSurf.packed) = HIDWORD(drawSurf.packed) & 0xFFC3FFFF | 0xC0000;
    drawSurf.fields.surfType = SF_STATICMODEL_PRETESS;
    if (R_AllocDrawSurf(delayedCmdBuf, drawSurf, drawSurfList, 3u))
    {
        BYTE1(preTessSurf) = lod;
        LOBYTE(preTessSurf) = surfaceIndex;
        HIWORD(preTessSurf) = *list;
        firstIndex = preTessIndices - gfxBuf.preTessIndexBuffer->indices;
        iassert(firstIndex < R_MAX_PRETESS_INDICES);

        R_WritePrimDrawSurfInt(delayedCmdBuf, count);
        R_WritePrimDrawSurfInt(delayedCmdBuf, preTessSurf);
        R_WritePrimDrawSurfInt(delayedCmdBuf, firstIndex);
    }
    return 1;
}

GfxStaticModelId __cdecl R_GetStaticModelId(unsigned int smodelIndex, int lod)
{
    const XModelLodInfo *lodInfo; // [esp+8h] [ebp-20h]
    XModel *model; // [esp+10h] [ebp-18h]
    int surfaceIndex; // [esp+14h] [ebp-14h]
    int surfaceCount; // [esp+18h] [ebp-10h]
    GfxStaticModelId staticModelId; // [esp+1Ch] [ebp-Ch]
    GfxStaticModelId staticModelIda; // [esp+1Ch] [ebp-Ch]
    XSurface *surfaces; // [esp+20h] [ebp-8h] BYREF
    XSurface *xsurf; // [esp+24h] [ebp-4h]

    model = rgp.world->dpvs.smodelDrawInsts[smodelIndex].model;
    iassert( R_StaticModelHasLighting( smodelIndex ) );
    if (!r_smc_enable->current.enabled)
        goto LABEL_9;
    lodInfo = XModelGetLodInfo(model, lod);
    if (!lodInfo->smcIndexPlusOne)
        goto LABEL_9;
    if (smodelIndex >= rgp.world->dpvs.smodelCount)
        MyAssertHandler(
            ".\\r_add_staticmodel.cpp",
            473,
            0,
            "smodelIndex doesn't index rgp.world->dpvs.smodelCount\n\t%i not in [0, %i)",
            smodelIndex,
            rgp.world->dpvs.smodelCount);
    staticModelId.objectId = R_CacheStaticModelSurface(lodInfo->smcIndexPlusOne - 1, smodelIndex, lodInfo);
    if (staticModelId.objectId)
    {
        staticModelId.surfType = SF_STATICMODEL_CACHED;
        return staticModelId;
    }
    else
    {
    LABEL_9:
        staticModelIda.surfType = SF_STATICMODEL_RIGID;
        staticModelIda.objectId = smodelIndex;
        surfaceCount = XModelGetSurfaces(model, &surfaces, lod);
        iassert( surfaceCount );
        for (surfaceIndex = 0; surfaceIndex < surfaceCount; ++surfaceIndex)
        {
            xsurf = &surfaces[surfaceIndex];
            if (xsurf->deformed || !IsFastFileLoad())
            {
                staticModelIda.surfType = SF_STATICMODEL_SKINNED;
                return staticModelIda;
            }
        }
        return staticModelIda;
    }
}

void __cdecl R_AddDelayedStaticModelDrawSurf(
    GfxDelayedCmdBuf *delayedCmdBuf,
    XSurface *xsurf,
    unsigned __int8 *list,
    unsigned int count)
{
    R_WritePrimDrawSurfInt(delayedCmdBuf, count);
    R_WritePrimDrawSurfInt(delayedCmdBuf, (unsigned int)xsurf);
    R_WritePrimDrawSurfData(delayedCmdBuf, list, (count + 1) >> 1);
}

void __cdecl R_EndDumpStaticModelLodInfo()
{
    char *basePath; // [esp+0h] [ebp-4h]

    Dvar_SetBool((dvar_s *)r_staticModelDumpLodInfo, 0);
    if (g_dumpStaticModelFileHandle)
    {
        if (g_dumpStaticModelFileHandle == -1)
        {
            g_dumpStaticModelFileHandle = 0;
        }
        else
        {
            FS_FCloseFile(g_dumpStaticModelFileHandle);
            basePath = Sys_DefaultInstallPath();
            Com_Printf(18, "^7Successfully wrote static model info [%s\\%s].\n", basePath, g_dumpStaticModelFilePath);
        }
    }
}

void __cdecl R_AddAllStaticModelSurfacesCamera()
{
    char* Name; // eax
    float dist; // [esp+8h] [ebp-1104h]
    float val; // [esp+2Ch] [ebp-10E0h]
    GfxStaticModelId StaticModelId; // [esp+54h] [ebp-10B8h]
    GfxStaticModelDrawInst* inst; // [esp+58h] [ebp-10B4h]
    float scale; // [esp+5Ch] [ebp-10B0h]
    float bias; // [esp+60h] [ebp-10ACh]
    unsigned __int16 staticModelLodList[4][4][128]{ 0 }; // [esp+64h] [ebp-10A8h] BYREF
    GfxStaticModelDrawInst* smodelDrawInsts; // [esp+1068h] [ebp-A4h]
    float originDist; // [esp+106Ch] [ebp-A0h]
    int currentProbeIndex; // [esp+1070h] [ebp-9Ch]
    XModel* model; // [esp+1074h] [ebp-98h]
    unsigned __int8 primaryLightIndex; // [esp+107Bh] [ebp-91h]
    XModel* currentModel; // [esp+107Ch] [ebp-90h]
    int reflectionProbeIndex; // [esp+1084h] [ebp-88h]
    unsigned __int16* list; // [esp+1088h] [ebp-84h]
    unsigned int* lodData; // [esp+108Ch] [ebp-80h]
    int lod; // [esp+1090h] [ebp-7Ch]
    unsigned int smodelCount; // [esp+1094h] [ebp-78h]
    _WORD* count; // [esp+1098h] [ebp-74h]
    unsigned __int8* visData; // [esp+109Ch] [ebp-70h]
    _BYTE v25[3]; // [esp+10A0h] [ebp-6Ch] BYREF
    unsigned __int8 currentLightIndex; // [esp+10A3h] [ebp-69h]
    float origin[3]; // [esp+10A4h] [ebp-68h] BYREF
    unsigned __int16 staticModelLodCount[4][4]{ 0 }; // [esp+10B0h] [ebp-5Ch] BYREF
    GfxSModelDrawSurfLightingData surfData; // [esp+10D4h] [ebp-38h] BYREF
    int allocatedLighting; // [esp+1100h] [ebp-Ch]
    unsigned int entryCount; // [esp+1104h] [ebp-8h]
    int smodelIndex; // [esp+1108h] [ebp-4h]

    PROF_SCOPED("SModelSurfaces");
    smodelCount = rgp.world->dpvs.smodelCount;
    lodData = rgp.world->dpvs.lodData;
    Com_Memset(lodData, 0, 32 * ((smodelCount + 127) >> 7));

    visData = rgp.world->dpvs.smodelVisData[0];
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    GfxLodParms *g_lodParms = &rg.correctedLodParms;
    iassert( g_lodParms->valid );
    scale = g_lodParms->ramp[0].scale;
    bias = g_lodParms->ramp[0].bias;
    origin[0] = g_lodParms->origin[0];
    origin[1] = g_lodParms->origin[1];
    origin[2] = g_lodParms->origin[2];

    R_InitDelayedCmdBuf(&surfData.delayedCmdBuf);

    surfData.drawSurf[0].current = scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_LIT];
    surfData.drawSurf[1].current = scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_DECAL];
    surfData.drawSurf[2].current = scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_EMISSIVE];

    surfData.drawSurf[0].end = scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_LIT] + 0x2000;
    surfData.drawSurf[1].end = scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_DECAL] + 0x200;
    surfData.drawSurf[2].end = scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_EMISSIVE] + 0x2000;

    memset(staticModelLodCount, 0, sizeof(staticModelLodCount));

    allocatedLighting = 0;
    currentModel = NULL;
    currentProbeIndex = 255;
    currentLightIndex = 0;

    for (smodelIndex = 0; smodelIndex < smodelCount; ++smodelIndex)
    {
        if (!visData[smodelIndex])
        {
            continue;
        }

        inst = &smodelDrawInsts[smodelIndex];
        originDist = Vec3Distance(inst->placement.origin, origin) * scale + bias;

        if (originDist >= inst->cullDist)
        {
            visData[smodelIndex] = 0;
            continue;
        }

        model = inst->model;
        primaryLightIndex = inst->primaryLightIndex;
        reflectionProbeIndex = inst->reflectionProbeIndex;
        if (model != currentModel || reflectionProbeIndex != currentProbeIndex || primaryLightIndex != currentLightIndex)
        {
            if (allocatedLighting)
            {
                R_SkinStaticModelsCamera(currentModel, currentLightIndex, staticModelLodList, staticModelLodCount, &surfData);
                memset(staticModelLodCount, 0, sizeof(staticModelLodCount));
                allocatedLighting = 0;
            }
            if (model != currentModel)
                currentModel = model;
            currentProbeIndex = reflectionProbeIndex;
            currentLightIndex = primaryLightIndex;
        }
        iassert(inst->placement.scale);
        dist = originDist * (1.0 / inst->placement.scale);
        lod = XModelGetLodForDist(model, dist);
        if (lod >= 0)
        {
            if (r_showCullSModels->current.enabled)
                R_AddDebugBox(
                    &frontEndDataOut->debugGlobals,
                    rgp.world->dpvs.smodelInsts[smodelIndex].mins,
                    rgp.world->dpvs.smodelInsts[smodelIndex].maxs,
                    colorLtGrey);

            if (r_showSModelNames->current.enabled)
            {
                Name = (char*)XModelGetName(model);
                R_AddDebugString(
                    &frontEndDataOut->debugGlobals,
                    inst->placement.origin,
                    colorWhite,
                    0.3f,
                    Name);
            }

            R_ShowCountsStaticModel(smodelIndex, lod);
            if (r_staticModelDumpLodInfo->current.enabled)
                R_DumpStaticModelLodInfo(inst, originDist);
            if (R_AllocStaticModelLighting(inst, smodelIndex))
            {
                allocatedLighting = 1;
                lodData[(unsigned int)smodelIndex >> 4] |= lod << (2 * (smodelIndex & 0xF));
                StaticModelId = R_GetStaticModelId(smodelIndex, lod);
                count = &staticModelLodCount[StaticModelId.surfType - 2][lod];
                list = staticModelLodList[StaticModelId.surfType - 2][lod];
                entryCount = *count;
                list[entryCount++] = StaticModelId.objectId;
                if (entryCount >= 128)
                {
                    R_SkinStaticModelsCameraForLod(
                        model,
                        primaryLightIndex,
                        (unsigned __int8*)list,
                        entryCount,
                        StaticModelId.surfType,
                        lod,
                        &surfData);
                    *count = 0;
                }
                else
                {
                    *count = entryCount;
                }
            }
            else
            {
                R_WarnOncePerFrame(R_WARN_SMODEL_LIGHTING);
                visData[smodelIndex] = 0;
            }
        }
        else
        {
            visData[smodelIndex] = 0;
        }
    }

    if (r_staticModelDumpLodInfo->current.enabled)
        R_EndDumpStaticModelLodInfo();

    if (allocatedLighting)
        R_SkinStaticModelsCamera(currentModel, currentLightIndex, staticModelLodList, staticModelLodCount, &surfData);

    R_EndCmdBuf(&surfData.delayedCmdBuf);
    
    scene.drawSurfCount[DRAW_SURF_SMODEL_CAMERA_LIT] = surfData.drawSurf[0].current - scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_LIT];
    scene.drawSurfCount[DRAW_SURF_SMODEL_CAMERA_DECAL] = surfData.drawSurf[1].current - scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_DECAL];
    scene.drawSurfCount[DRAW_SURF_SMODEL_CAMERA_EMISSIVE] = surfData.drawSurf[2].current - scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_EMISSIVE];
}

void __cdecl R_SkinStaticModelsCameraForLod(
    const XModel *model,
    unsigned __int8 primaryLightIndex,
    unsigned __int8 *list,
    unsigned int count,
    unsigned int surfType,
    unsigned int lod,
    GfxSModelDrawSurfLightingData *surfData)
{
    bool enabled; // [esp+6h] [ebp-26h]
    Material **materialForSurf; // [esp+8h] [ebp-24h]
    GfxDrawSurf drawSurf; // [esp+Ch] [ebp-20h]
    const Material *material; // [esp+18h] [ebp-14h]
    unsigned int surfaceIndex; // [esp+1Ch] [ebp-10h]
    int surfaceCount; // [esp+20h] [ebp-Ch]
    XSurface *surfaces; // [esp+24h] [ebp-8h] BYREF
    unsigned int region; // [esp+28h] [ebp-4h]

    surfaceCount = XModelGetSurfaces(model, &surfaces, lod);
    iassert(surfaceCount);
    materialForSurf = XModelGetSkins(model, lod);
    iassert(materialForSurf);
    iassert(surfaceCount >= 1 && surfaceCount <= 48);
    iassert(gfxDrawMethod.emissiveTechType < TECHNIQUE_COUNT);
    for (surfaceIndex = 0; (int)surfaceIndex < surfaceCount; ++surfaceIndex)
    {
        iassert(surfaceIndex < UCHAR_MAX);
        material = *materialForSurf;
        iassert(material);
        iassert(rgp.sortedMaterials[material->info.drawSurf.fields.materialSortedIndex] == material);
        region = material->cameraRegion;
        if (region != 3)
        {
            drawSurf.packed = material->info.drawSurf.packed;
            
            drawSurf.fields.primaryLightIndex = primaryLightIndex;
            drawSurf.fields.surfType = surfType;

            //HIDWORD(drawSurf.packed) = (primaryLightIndex << 10) // primaryLightIndex
            //    | ((surfType & 0xF) << 18) // surfType
            //    | HIDWORD(drawSurf.packed) & 0xFFC003FF; // other bits
            if (surfType != SF_STATICMODEL_CACHED
                || (!dx.deviceLost ? (enabled = r_pretess->current.enabled) : (enabled = 0),
                    !enabled
                    || !R_PreTessStaticModelCachedList(
                        model,
                        (unsigned __int16 *)list,
                        count,
                        lod,
                        surfaceIndex,
                        drawSurf,
                        &surfData->drawSurf[region],
                        &surfData->delayedCmdBuf)))
            {
                if (!R_AllocDrawSurf(&surfData->delayedCmdBuf, drawSurf, &surfData->drawSurf[region], ((count + 1) >> 1) + 2))
                    return;
                R_AddDelayedStaticModelDrawSurf(&surfData->delayedCmdBuf, &surfaces[surfaceIndex], list, count);
            }
        }
        ++materialForSurf;
    }
}

void __cdecl R_SkinStaticModelsCamera(
    const XModel *model,
    unsigned __int8 primaryLightIndex,
    unsigned __int16 (*staticModelLodList)[4][128],
    unsigned __int16 (*staticModelLodCount)[4],
    GfxSModelDrawSurfLightingData *surfData)
{
    unsigned int surfTypeIndex; // [esp+0h] [ebp-4h]

    for (surfTypeIndex = 0; surfTypeIndex < 4; ++surfTypeIndex)
        R_SkinStaticModelsCameraForSurface(
            model,
            primaryLightIndex,
            (unsigned __int16 (*)[128])(*staticModelLodList)[4 * surfTypeIndex],
            &(*staticModelLodCount)[4 * surfTypeIndex],
            surfTypeIndex + 2,
            surfData);
}

void __cdecl R_SkinStaticModelsCameraForSurface(
    const XModel *model,
    unsigned __int8 primaryLightIndex,
    unsigned __int16 (*staticModelLodList)[128],
    unsigned __int16 *staticModelLodCount,
    unsigned int surfType,
    GfxSModelDrawSurfLightingData *surfData)
{
    signed int lod; // [esp+0h] [ebp-8h]

    for (lod = 0; lod < 4; ++lod)
    {
        if (staticModelLodCount[lod])
            R_SkinStaticModelsCameraForLod(
                model,
                primaryLightIndex,
                (unsigned __int8 *)&(*staticModelLodList)[128 * lod],
                staticModelLodCount[lod],
                surfType,
                lod,
                surfData);
    }
}

void __cdecl R_ShowCountsStaticModel(int smodelIndex, int lod)
{
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    int totalVertCount; // [esp+4h] [ebp-20h]
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+8h] [ebp-1Ch]
    int totalTriCount; // [esp+Ch] [ebp-18h]
    int surfaceIndex; // [esp+10h] [ebp-14h]
    const XModel *model; // [esp+14h] [ebp-10h]
    int surfaceCount; // [esp+18h] [ebp-Ch]
    XSurface *surfaces; // [esp+1Ch] [ebp-8h] BYREF
    const XSurface *xsurf; // [esp+20h] [ebp-4h]

    if (r_showTriCounts->current.enabled || r_showVertCounts->current.enabled || r_showSurfCounts->current.enabled)
    {
        smodelDrawInst = &rgp.world->dpvs.smodelDrawInsts[smodelIndex];
        model = smodelDrawInst->model;
        XModelGetLodInfo(model, lod);
        surfaceCount = XModelGetSurfaces(model, &surfaces, lod);
        iassert( surfaceCount );
        totalTriCount = 0;
        totalVertCount = 0;
        for (surfaceIndex = 0; surfaceIndex < surfaceCount; ++surfaceIndex)
        {
            xsurf = &surfaces[surfaceIndex];
            if (r_showTriCounts->current.enabled)
            {
                totalTriCount += XSurfaceGetNumTris(xsurf);
            }
            else if (r_showVertCounts->current.enabled)
            {
                totalVertCount += XSurfaceGetNumVerts(xsurf);
            }
        }
        if (r_showTriCounts->current.enabled)
        {
            v2 = va("%i", totalTriCount);
            R_AddXModelDebugString(smodelDrawInst->placement.origin, v2);
        }
        else if (r_showVertCounts->current.enabled)
        {
            v3 = va("%i", totalVertCount);
            R_AddXModelDebugString(smodelDrawInst->placement.origin, v3);
        }
        else if (r_showSurfCounts->current.enabled)
        {
            v4 = va("%i", surfaceCount);
            R_AddXModelDebugString(smodelDrawInst->placement.origin, v4);
        }
    }
}

void __cdecl R_DumpStaticModelLodInfo(const GfxStaticModelDrawInst *smodelDrawInst, float dist)
{
    if (g_dumpStaticModelFileHandle != -1)
    {
        if (!g_dumpStaticModelFileHandle)
        {
            g_dumpStaticModelFileHandle = FS_FOpenTextFileWrite(g_dumpStaticModelFilePath);
            if (g_dumpStaticModelFileHandle)
                R_StaticModelWriteInfoHeader(g_dumpStaticModelFileHandle);
            g_dumpStaticModelCount = 0;
        }
        if (g_dumpStaticModelFileHandle)
        {
            R_StaticModelWriteInfo(g_dumpStaticModelFileHandle, smodelDrawInst, dist);
        }
        else
        {
            g_dumpStaticModelFileHandle = -1;
            Com_PrintError(1, "Could not dump model info.\n");
        }
    }
}

unsigned int _S1_0; 
float radius2pixels;
void __cdecl R_StaticModelWriteInfo(int fileHandle, const GfxStaticModelDrawInst *smodelDrawInst, const float dist)
{
    float v3; // [esp+5Ch] [ebp-101Ch]
    char dest; // [esp+60h] [ebp-1018h] BYREF
    _BYTE v5[4103]; // [esp+61h] [ebp-1017h] BYREF
    float v6; // [esp+1068h] [ebp-10h]
    XModel *xmodel; // [esp+106Ch] [ebp-Ch]
    float v8; // [esp+1070h] [ebp-8h]
    float lodDist; // [esp+1074h] [ebp-4h]

    if ((_S1_0 & 1) == 0)
    {
        _S1_0 |= 1u;
        v3 = tan(22.5);
        radius2pixels = 720.0 / v3;
    }
    *(unsigned int *)&v5[4099] = 4096;
    xmodel = smodelDrawInst->model;
    iassert( xmodel );
    iassert( xmodel->name );
    iassert( xmodel->numLods > 0 );
    lodDist = *((float *)&xmodel->parentList + 7 * xmodel->numLods);
    iassert( lodDist > 0.0f );
    v6 = radius2pixels * xmodel->radius / lodDist;
    v8 = radius2pixels * xmodel->radius;
    ++g_dumpStaticModelCount;
    if (smodelDrawInst->placement.scale > 0.0 && dist > 0.0)
        Com_sprintf(
            &dest,
            0x1000u,
            "%d,%s,%.1f,%d,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",
            g_dumpStaticModelCount,
            xmodel->name,
            xmodel->radius,
            xmodel->numLods,
            lodDist,
            v6,
            v8,
            dist / smodelDrawInst->placement.scale,
            smodelDrawInst->placement.origin[0],
            smodelDrawInst->placement.origin[1],
            smodelDrawInst->placement.origin[2],
            v8 / (dist / smodelDrawInst->placement.scale));
    FS_Write(&dest, &v5[strlen(&dest)] - v5, fileHandle);
}

void __cdecl R_SortAllStaticModelSurfacesCamera()
{
    KISAK_NULLSUB();
    R_SortDrawSurfs(scene.drawSurfs[1], scene.drawSurfCount[1]);
    KISAK_NULLSUB();
    R_SortDrawSurfs(scene.drawSurfs[4], scene.drawSurfCount[4]);
    KISAK_NULLSUB();
    R_SortDrawSurfs(scene.drawSurfs[10], scene.drawSurfCount[10]);
}

void __cdecl R_SortAllStaticModelSurfacesSunShadow()
{
    KISAK_NULLSUB();
    R_SortDrawSurfs(scene.drawSurfs[16], scene.drawSurfCount[16]);
    KISAK_NULLSUB();
    R_SortDrawSurfs(scene.drawSurfs[19], scene.drawSurfCount[19]);
}

void __cdecl R_AddAllStaticModelSurfacesSunShadow()
{
    R_AddAllStaticModelSurfacesRangeSunShadow(0, 0x1000u);
    R_AddAllStaticModelSurfacesRangeSunShadow(1u, 0x2000u);
}

void __cdecl R_AddAllStaticModelSurfacesRangeSunShadow(unsigned int partitionIndex, unsigned int maxDrawSurfCount)
{
    float dist; // [esp+4h] [ebp-10E4h]
    float val; // [esp+28h] [ebp-10C0h]
    float diff[2][3]; // [esp+30h] [ebp-10B8h] BYREF
    GfxStaticModelId StaticModelId; // [esp+50h] [ebp-1098h]
    GfxStaticModelDrawInst* inst; // [esp+54h] [ebp-1094h]
    float scale; // [esp+58h] [ebp-1090h]
    float bias; // [esp+5Ch] [ebp-108Ch]
    GfxStaticModelDrawInst* smodelDrawInsts; // [esp+1064h] [ebp-84h]
    float originDist; // [esp+1068h] [ebp-80h]
    XModel* model; // [esp+106Ch] [ebp-7Ch]
    XModel* currentModel; // [esp+1070h] [ebp-78h]
    unsigned int stage; // [esp+1078h] [ebp-70h]
    unsigned __int16* list; // [esp+107Ch] [ebp-6Ch]
    unsigned __int16 *count; // [esp+1088h] [ebp-60h]
    int lod; // [esp+1080h] [ebp-68h]
    unsigned int smodelCount; // [esp+1084h] [ebp-64h]
    unsigned __int8* visData; // [esp+108Ch] [ebp-5Ch] BYREF
    float origin[3]; // [esp+1090h] [ebp-58h] BYREF
    unsigned __int16 staticModelLodCount[4][4]{ 0 }; // [esp+109Ch] [ebp-4Ch] BYREF
    unsigned __int16 staticModelLodList[4][4][128]{ 0 }; // [esp+60h] [ebp-1088h] BYREF
    GfxSModelDrawSurfData surfData; // [esp+10C0h] [ebp-28h] BYREF
    int allocatedLighting; // [esp+10DCh] [ebp-Ch] // LWSS: guessed name
    unsigned int entryCount; // [esp+10E0h] [ebp-8h]
    unsigned int i; // [esp+10E4h] [ebp-4h]

    PROF_SCOPED("SModelSurfacesShadow");
    smodelCount = rgp.world->dpvs.smodelCount;
    visData = rgp.world->dpvs.smodelVisData[partitionIndex + 1];
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    GfxLodParms *g_lodParms = &rg.correctedLodParms;
    iassert( g_lodParms->valid );
    scale = g_lodParms->ramp[0].scale;
    bias =  g_lodParms->ramp[0].bias;
    origin[0] =  g_lodParms->origin[0];
    origin[1] =  g_lodParms->origin[1];
    origin[2] =  g_lodParms->origin[2];

    R_InitDelayedCmdBuf(&surfData.delayedCmdBuf);
    
    stage = DRAW_SURF_SMODEL_SUNSHADOW_0 + (3 * partitionIndex);

    surfData.drawSurfList.current = scene.drawSurfs[stage];
    iassert( (int)maxDrawSurfCount == scene.maxDrawSurfCount[stage] );
    surfData.drawSurfList.end = &surfData.drawSurfList.current[maxDrawSurfCount];
    memset(staticModelLodCount, 0, sizeof(staticModelLodCount));
    allocatedLighting = 0;
    currentModel = 0;

    for (i = 0; i < smodelCount; ++i)
    {
        if (!visData[i])
        {
            continue;
        }

        inst = &smodelDrawInsts[i];

        if ((inst->flags & 1) != 0)
        {
            continue;
        }

        originDist = Vec3Distance(inst->placement.origin, origin) * scale + bias;

        if (inst->cullDist <= originDist)
        {
            visData[i] = 0;
            continue;
        }

        model = inst->model;
        if (model != currentModel)
        {
            if (allocatedLighting)
            {
                // If model changed, flush the batch and set new model
                R_SkinStaticModelsShadow(
                    currentModel,
                    staticModelLodList,
                    staticModelLodCount,
                    &surfData);
                memset(staticModelLodCount, 0, sizeof(staticModelLodCount));
                allocatedLighting = 0;
            }
            currentModel = model;
        }

        iassert(inst->placement.scale);
        dist = originDist * (1.0f / inst->placement.scale);
        lod = XModelGetLodForDist(model, dist);

        if (lod < 0)
        {
            continue;
        }

        // Allocate lighting for this static model
        if (R_AllocStaticModelLighting(inst, i))
        {
            allocatedLighting = 1;
            StaticModelId = R_GetStaticModelId(i, lod);
            iassert(StaticModelId.surfType >= 2 && StaticModelId.surfType < 6); // lwss add
            count = &staticModelLodCount[StaticModelId.surfType - 2][lod];
            list = staticModelLodList[StaticModelId.surfType - 2][lod];
            entryCount = *count;
            list[entryCount++] = StaticModelId.objectId;

            // Flush if list is full
            if (entryCount >= 128)
            {
                R_SkinStaticModelsShadowForLod(
                    model,
                    (unsigned __int8*)list,
                    entryCount,
                    StaticModelId.surfType,
                    lod,
                    &surfData);
                *count = 0;
            }
            else
            {
                *count = entryCount;
            }
        }
        else
        {
            R_WarnOncePerFrame(R_WARN_SMODEL_LIGHTING);
        }
    }

    // Flush last model batch if needed
    if (allocatedLighting)
        R_SkinStaticModelsShadow(currentModel, staticModelLodList, staticModelLodCount, &surfData);

    R_EndCmdBuf(&surfData.delayedCmdBuf);
    scene.drawSurfCount[stage] = surfData.drawSurfList.current - scene.drawSurfs[stage];
}

void __cdecl R_SkinStaticModelsShadowForLod(
    const XModel *model,
    unsigned __int8 *list,
    unsigned int count,
    unsigned int surfType,
    unsigned int lod,
    GfxSModelDrawSurfData *surfData)
{
    bool v6; // zf
    bool enabled; // [esp+7h] [ebp-2Dh]
    Material **materialForSurf; // [esp+10h] [ebp-24h]
    GfxDrawSurf drawSurf; // [esp+14h] [ebp-20h]
    MaterialTechniqueType shadowmapBuildTechType; // [esp+1Ch] [ebp-18h]
    const Material *material; // [esp+20h] [ebp-14h]
    int surfaceIndex; // [esp+24h] [ebp-10h]
    int surfaceCount; // [esp+28h] [ebp-Ch]
    XSurface *surfaces[2]; // [esp+2Ch] [ebp-8h] BYREF

    surfaceCount = XModelGetSurfaces(model, surfaces, lod);
    iassert( surfaceCount );
    materialForSurf = XModelGetSkins(model, lod);
    iassert( materialForSurf );
    if (surfaceCount < 1 || surfaceCount > 48)
        MyAssertHandler(
            ".\\r_add_staticmodel.cpp",
            258,
            0,
            "surfaceCount not in [1, XMODEL_MAX_SURFS]\n\t%i not in [%i, %i]",
            surfaceCount,
            1,
            48);
    shadowmapBuildTechType = gfxMetrics.shadowmapBuildTechType;
    for (surfaceIndex = 0; surfaceIndex < surfaceCount; ++surfaceIndex)
    {
        material = *materialForSurf;
        iassert(material);
        iassert(rgp.sortedMaterials[material->info.drawSurf.fields.materialSortedIndex] == material);
        v6 = (material->info.gameFlags & 0x40) == 0;
        surfaces[1] = (XSurface *)((material->info.gameFlags & 0x40) != 0);
        if (!v6)
        {
            if (!Material_GetTechnique(material, shadowmapBuildTechType))
                MyAssertHandler(
                    ".\\r_add_staticmodel.cpp",
                    287,
                    0,
                    "%s",
                    "Material_HasTechnique( material, shadowmapBuildTechType )");
            drawSurf = material->info.drawSurf;

            drawSurf.fields.surfType = surfType;
            //HIDWORD(drawSurf.packed) = ((surfType & 0xF) << 18) | HIDWORD(material->info.drawSurf.packed) & 0xFFC3FFFF;

            if (surfType != SF_STATICMODEL_CACHED
                || (!dx.deviceLost ? (enabled = r_pretess->current.enabled) : (enabled = 0),
                    !enabled
                    || !R_PreTessStaticModelCachedList(
                        model,
                        (unsigned __int16 *)list,
                        count,
                        lod,
                        surfaceIndex,
                        drawSurf,
                        &surfData->drawSurfList,
                        &surfData->delayedCmdBuf)))
            {
                if (!R_AllocDrawSurf(&surfData->delayedCmdBuf, drawSurf, &surfData->drawSurfList, ((count + 1) >> 1) + 2))
                    return;
                R_AddDelayedStaticModelDrawSurf(&surfData->delayedCmdBuf, &surfaces[0][surfaceIndex], list, count);
            }
        }
        ++materialForSurf;
    }
}

void __cdecl R_SkinStaticModelsShadow(
    const XModel *model,
    unsigned __int16 (*staticModelLodList)[4][128],
    unsigned __int16 (*staticModelLodCount)[4],
    GfxSModelDrawSurfData *surfData)
{
    unsigned int surfTypeIndex; // [esp+0h] [ebp-4h]

    for (surfTypeIndex = 0; surfTypeIndex < 4; ++surfTypeIndex)
        R_SkinStaticModelsShadowForSurface(
            model,
            (unsigned __int16 (*)[128])(*staticModelLodList)[4 * surfTypeIndex],
            &(*staticModelLodCount)[4 * surfTypeIndex],
            surfTypeIndex + 2,
            surfData);
}

void __cdecl R_SkinStaticModelsShadowForSurface(
    const XModel *model,
    unsigned __int16 (*staticModelLodList)[128],
    unsigned __int16 *staticModelLodCount,
    unsigned int surfType,
    GfxSModelDrawSurfData *surfData)
{
    signed int lod; // [esp+0h] [ebp-8h]

    for (lod = 0; lod < 4; ++lod)
    {
        if (staticModelLodCount[lod])
            R_SkinStaticModelsShadowForLod(
                model,
                (unsigned __int8 *)&(*staticModelLodList)[128 * lod],
                staticModelLodCount[lod],
                surfType,
                lod,
                surfData);
    }
}

void __cdecl R_AddAllStaticModelSurfacesSpotShadow(unsigned int spotShadowIndex, unsigned int primaryLightIndex)
{
    double v2; // st7
    float dist; // [esp+4h] [ebp-10B4h]
    float v4; // [esp+8h] [ebp-10B0h]
    float val; // [esp+Ch] [ebp-10ACh]
    float diff[3]; // [esp+14h] [ebp-10A4h] BYREF
    GfxStaticModelId StaticModelId; // [esp+20h] [ebp-1098h]
    GfxStaticModelDrawInst* smodelDrawInst; // [esp+24h] [ebp-1094h]
    float scale; // [esp+28h] [ebp-1090h]
    float bias; // [esp+2Ch] [ebp-108Ch]
    unsigned __int16 staticModelLodList[4][4][128]; // [esp+30h] [ebp-1088h] BYREF
    GfxStaticModelDrawInst* smodelDrawInsts; // [esp+1030h] [ebp-88h]
    float v13; // [esp+1034h] [ebp-84h]
    XModel* model; // [esp+1038h] [ebp-80h]
    GfxShadowGeometry* v15; // [esp+103Ch] [ebp-7Ch]
    XModel* v16; // [esp+1040h] [ebp-78h]
    GfxStaticModelId v17; // [esp+1044h] [ebp-74h]
    unsigned int v18; // [esp+1048h] [ebp-70h]
    unsigned __int16* list; // [esp+104Ch] [ebp-6Ch]
    int lod; // [esp+1050h] [ebp-68h]
    unsigned int i; // [esp+1054h] [ebp-64h]
    unsigned int smodelCount; // [esp+1058h] [ebp-60h]
    _WORD* v23; // [esp+105Ch] [ebp-5Ch] BYREF
    float a[3]; // [esp+1060h] [ebp-58h] BYREF
    unsigned __int16 staticModelLodCount[4][4]; // [esp+106Ch] [ebp-4Ch] BYREF
    GfxSModelDrawSurfData surfData; // [esp+1090h] [ebp-28h] BYREF
    unsigned int v27; // [esp+10ACh] [ebp-Ch]
    unsigned int v28; // [esp+10B0h] [ebp-8h]
    int surfCount; // [esp+10B4h] [ebp-4h]

    smodelCount = rgp.world->dpvs.smodelCount;
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    iassert(rg.lodParms.valid);
    scale = rg.lodParms.ramp[0].scale;
    bias = rg.lodParms.ramp[0].bias;
    a[0] = rg.lodParms.origin[0];
    a[1] = rg.lodParms.origin[1];
    a[2] = rg.lodParms.origin[2];
    R_InitBspDrawSurf((GfxBspDrawSurfData*)&surfData);
    v18 = 3 * spotShadowIndex + 22;
    surfData.drawSurfList.current = scene.drawSurfs[v18];
    surfData.drawSurfList.end = &scene.drawSurfs[v18][scene.maxDrawSurfCount[v18]];
    v16 = 0;
    v15 = &rgp.world->shadowGeom[primaryLightIndex];
    for (i = 0; i < v15->smodelCount; ++i)
    {
        v28 = v15->smodelIndex[i];
        smodelDrawInst = &smodelDrawInsts[v28];
        model = smodelDrawInst->model;
        if ((smodelDrawInst->flags & 1) == 0)
        {
            Vec3Sub(a, smodelDrawInst->placement.origin, diff);
            v2 = Vec3Length(diff);
            v13 = v2 * scale + bias;
            if (smodelDrawInst->cullDist > (double)v13)
            {
                model = smodelDrawInst->model;
                val = smodelDrawInst->placement.scale;
                iassert( val );
                v4 = 1.0 / val;
                dist = v13 * v4;
                lod = XModelGetLodForDist(model, dist);
                if (lod >= 0)
                {
                    if (R_AllocStaticModelLighting(smodelDrawInst, v28))
                    {
                        if (model != v16)
                        {
                            if (v16)
                                R_SkinStaticModelsShadow(v16, staticModelLodList, staticModelLodCount, &surfData);
                            v16 = model;
                            memset(staticModelLodCount, 0, sizeof(staticModelLodCount));
                        }
                        StaticModelId = R_GetStaticModelId(v28, lod);
                        v17 = StaticModelId;
                        v23 = &staticModelLodCount[StaticModelId.surfType - 2][lod];
                        list = staticModelLodList[StaticModelId.surfType - 2][lod];
                        v27 = (unsigned __int16)*v23;
                        list[v27++] = StaticModelId.objectId;
                        if (v27 >= 0x80)
                        {
                            R_SkinStaticModelsShadowForLod(model, (unsigned __int8*)list, v27, v17.surfType, lod, &surfData);
                            *v23 = 0;
                        }
                        else
                        {
                            *v23 = v27;
                        }
                    }
                    else
                    {
                        R_WarnOncePerFrame(R_WARN_SMODEL_LIGHTING);
                    }
                }
            }
        }
    }
    if (v16)
        R_SkinStaticModelsShadow(v16, staticModelLodList, staticModelLodCount, &surfData);
    R_EndCmdBuf(&surfData.delayedCmdBuf);
    surfCount = surfData.drawSurfList.current - scene.drawSurfs[v18];
    scene.drawSurfCount[v18] = surfCount;
    KISAK_NULLSUB();
    R_SortDrawSurfs(scene.drawSurfs[v18], surfCount);
}

