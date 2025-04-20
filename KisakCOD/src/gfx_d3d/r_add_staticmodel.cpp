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
    Profile_Begin(166);
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
    Profile_EndInternal(0);
    HIDWORD(drawSurf.packed) = HIDWORD(drawSurf.packed) & 0xFFC3FFFF | 0xC0000;
    if (R_AllocDrawSurf(delayedCmdBuf, drawSurf, drawSurfList, 3u))
    {
        BYTE1(preTessSurf) = lod;
        LOBYTE(preTessSurf) = surfaceIndex;
        HIWORD(preTessSurf) = *list;
        firstIndex = preTessIndices - gfxBuf.preTessIndexBuffer->indices;
        if (firstIndex >= 0x100000)
            MyAssertHandler(
                ".\\r_add_staticmodel.cpp",
                150,
                0,
                "firstIndex doesn't index R_MAX_PRETESS_INDICES\n\t%i not in [0, %i)",
                firstIndex,
                0x100000);
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
    if (!R_StaticModelHasLighting(smodelIndex))
        MyAssertHandler(".\\r_add_staticmodel.cpp", 466, 0, "%s", "R_StaticModelHasLighting( smodelIndex )");
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
        staticModelId.surfType = 4;
        return staticModelId;
    }
    else
    {
    LABEL_9:
        staticModelIda.surfType = 2;
        staticModelIda.objectId = smodelIndex;
        surfaceCount = XModelGetSurfaces(model, &surfaces, lod);
        if (!surfaceCount)
            MyAssertHandler(".\\r_add_staticmodel.cpp", 489, 0, "%s", "surfaceCount");
        for (surfaceIndex = 0; surfaceIndex < surfaceCount; ++surfaceIndex)
        {
            xsurf = &surfaces[surfaceIndex];
            if (xsurf->deformed || !useFastFile->current.enabled)
            {
                staticModelIda.surfType = 5;
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
    double v0; // st7
    char *Name; // eax
    float dist; // [esp+8h] [ebp-1104h]
    float v3; // [esp+Ch] [ebp-1100h]
    float v4; // [esp+2Ch] [ebp-10E0h]
    float diff[8]; // [esp+34h] [ebp-10D8h] BYREF
    GfxStaticModelId StaticModelId; // [esp+54h] [ebp-10B8h]
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+58h] [ebp-10B4h]
    float scale; // [esp+5Ch] [ebp-10B0h]
    float bias; // [esp+60h] [ebp-10ACh]
    unsigned __int16 staticModelLodList[4][4][128]; // [esp+64h] [ebp-10A8h] BYREF
    GfxStaticModelDrawInst *smodelDrawInsts; // [esp+1068h] [ebp-A4h]
    float v12; // [esp+106Ch] [ebp-A0h]
    int v13; // [esp+1070h] [ebp-9Ch]
    XModel *v14; // [esp+1074h] [ebp-98h]
    unsigned __int8 primaryLightIndex; // [esp+107Bh] [ebp-91h]
    XModel *model; // [esp+107Ch] [ebp-90h]
    GfxStaticModelId v17; // [esp+1080h] [ebp-8Ch]
    int reflectionProbeIndex; // [esp+1084h] [ebp-88h]
    unsigned __int16 *list; // [esp+1088h] [ebp-84h]
    void *dest; // [esp+108Ch] [ebp-80h]
    int lod; // [esp+1090h] [ebp-7Ch]
    unsigned int smodelCount; // [esp+1094h] [ebp-78h]
    _WORD *v23; // [esp+1098h] [ebp-74h]
    unsigned __int8 *v24; // [esp+109Ch] [ebp-70h]
    _BYTE v25[3]; // [esp+10A0h] [ebp-6Ch] BYREF
    unsigned __int8 v26; // [esp+10A3h] [ebp-69h]
    float a[3]; // [esp+10A4h] [ebp-68h] BYREF
    unsigned __int16 staticModelLodCount[4]; // [esp+10B0h] [ebp-5Ch] BYREF
    int v29; // [esp+10B8h] [ebp-54h]
    int v30; // [esp+10BCh] [ebp-50h]
    int v31; // [esp+10C0h] [ebp-4Ch]
    int v32; // [esp+10C4h] [ebp-48h]
    int v33; // [esp+10C8h] [ebp-44h]
    int v34; // [esp+10CCh] [ebp-40h]
    GfxSModelDrawSurfLightingData surfData; // [esp+10D4h] [ebp-38h] BYREF
    int v36; // [esp+1100h] [ebp-Ch]
    unsigned int count; // [esp+1104h] [ebp-8h]
    int smodelIndex; // [esp+1108h] [ebp-4h]

    //Profile_Begin(410);
    smodelCount = rgp.world->dpvs.smodelCount;
    dest = rgp.world->dpvs.lodData;
    Com_Memset((unsigned int *)dest, 0, 32 * ((smodelCount + 127) >> 7));
    v24 = rgp.world->dpvs.smodelVisData[0];
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    if (!rg.correctedLodParms.valid)
        MyAssertHandler(".\\r_add_staticmodel.cpp", 671, 0, "%s", "g_lodParms->valid");
    scale = rg.correctedLodParms.ramp[0].scale;
    bias = rg.correctedLodParms.ramp[0].bias;
    a[0] = rg.correctedLodParms.origin[0];
    a[1] = rg.correctedLodParms.origin[1];
    a[2] = rg.correctedLodParms.origin[2];
    R_InitBspDrawSurf(&surfData);
    surfData.drawSurf[0].current = scene.drawSurfs[1];
    surfData.drawSurf[1].current = scene.drawSurfs[4];
    surfData.drawSurf[2].current = scene.drawSurfs[10];
    surfData.drawSurf[0].end = scene.drawSurfs[1] + 0x2000;
    surfData.drawSurf[1].end = scene.drawSurfs[4] + 512;
    surfData.drawSurf[2].end = scene.drawSurfs[10] + 0x2000;
    memset(staticModelLodCount, 0, sizeof(staticModelLodCount));
    v29 = 0;
    v30 = 0;
    v31 = 0;
    v32 = 0;
    v33 = 0;
    v34 = 0;
    v36 = 0;
    model = 0;
    v13 = 255;
    v26 = 0;
    for (smodelIndex = 0; smodelIndex < smodelCount; ++smodelIndex)
    {
        if (v24[smodelIndex])
        {
            smodelDrawInst = &smodelDrawInsts[smodelIndex];
            Vec3Sub(a, smodelDrawInst->placement.origin, diff);
            v0 = Vec3Length(diff);
            v12 = v0 * scale + bias;
            if (smodelDrawInst->cullDist > (double)v12)
            {
                v14 = smodelDrawInst->model;
                primaryLightIndex = smodelDrawInst->primaryLightIndex;
                reflectionProbeIndex = smodelDrawInst->reflectionProbeIndex;
                if (v14 != model || reflectionProbeIndex != v13 || primaryLightIndex != v26)
                {
                    if (v36)
                    {
                        R_SkinStaticModelsCamera(
                            model,
                            v26,
                            staticModelLodList,
                            (unsigned __int16 (*)[4])staticModelLodCount,
                            &surfData);
                        memset(staticModelLodCount, 0, sizeof(staticModelLodCount));
                        v29 = 0;
                        v30 = 0;
                        v31 = 0;
                        v32 = 0;
                        v33 = 0;
                        v34 = 0;
                        v36 = 0;
                    }
                    if (v14 != model)
                        model = v14;
                    v13 = reflectionProbeIndex;
                    v26 = primaryLightIndex;
                }
                v4 = smodelDrawInst->placement.scale;
                if (v4 == 0.0)
                    MyAssertHandler("c:\\trees\\cod3\\src\\universal\\com_math.h", 107, 0, "%s", "val");
                v3 = 1.0 / v4;
                dist = v12 * v3;
                lod = XModelGetLodForDist(v14, dist);
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
                        Name = (char *)XModelGetName(v14);
                        R_AddDebugString(
                            &frontEndDataOut->debugGlobals,
                            smodelDrawInst->placement.origin,
                            colorWhite,
                            0.30000001,
                            Name);
                    }
                    R_ShowCountsStaticModel(smodelIndex, lod);
                    if (r_staticModelDumpLodInfo->current.enabled)
                        R_DumpStaticModelLodInfo(smodelDrawInst, v12);
                    if (R_AllocStaticModelLighting(smodelDrawInst, smodelIndex))
                    {
                        v36 = 1;
                        *((unsigned int *)dest + ((unsigned int)smodelIndex >> 4)) |= lod << (2 * (smodelIndex & 0xF));
                        StaticModelId = R_GetStaticModelId(smodelIndex, lod);
                        v17 = StaticModelId;
                        v23 = (uint16*)&v25[8 * StaticModelId.surfType + 2 * lod];
                        list = staticModelLodList[StaticModelId.surfType - 2][lod];
                        count = (unsigned __int16)*v23;
                        list[count++] = StaticModelId.objectId;
                        if (count >= 0x80)
                        {
                            R_SkinStaticModelsCameraForLod(
                                v14,
                                primaryLightIndex,
                                (unsigned __int8 *)list,
                                count,
                                v17.surfType,
                                lod,
                                &surfData);
                            *v23 = 0;
                        }
                        else
                        {
                            *v23 = count;
                        }
                    }
                    else
                    {
                        R_WarnOncePerFrame(R_WARN_SMODEL_LIGHTING);
                        v24[smodelIndex] = 0;
                    }
                }
                else
                {
                    v24[smodelIndex] = 0;
                }
            }
            else
            {
                v24[smodelIndex] = 0;
            }
        }
    }
    if (r_staticModelDumpLodInfo->current.enabled)
        R_EndDumpStaticModelLodInfo();
    if (v36)
        R_SkinStaticModelsCamera(model, v26, staticModelLodList, (unsigned __int16 (*)[4])staticModelLodCount, &surfData);
    R_EndCmdBuf(&surfData.delayedCmdBuf);
    scene.drawSurfCount[1] = surfData.drawSurf[0].current - scene.drawSurfs[1];
    scene.drawSurfCount[4] = surfData.drawSurf[1].current - scene.drawSurfs[4];
    scene.drawSurfCount[10] = surfData.drawSurf[2].current - scene.drawSurfs[10];
    //Profile_EndInternal(0);
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
    if (!surfaceCount)
        MyAssertHandler(".\\r_add_staticmodel.cpp", 178, 0, "%s", "surfaceCount");
    materialForSurf = XModelGetSkins(model, lod);
    if (!materialForSurf)
        MyAssertHandler(".\\r_add_staticmodel.cpp", 181, 0, "%s", "materialForSurf");
    if (surfaceCount < 1 || surfaceCount > 48)
        MyAssertHandler(
            ".\\r_add_staticmodel.cpp",
            184,
            0,
            "surfaceCount not in [1, XMODEL_MAX_SURFS]\n\t%i not in [%i, %i]",
            surfaceCount,
            1,
            48);
    if (gfxDrawMethod.emissiveTechType >= (unsigned int)TECHNIQUE_COUNT)
        MyAssertHandler(
            ".\\r_add_staticmodel.cpp",
            187,
            0,
            "gfxDrawMethod.emissiveTechType doesn't index TECHNIQUE_COUNT\n\t%i not in [0, %i)",
            gfxDrawMethod.emissiveTechType,
            34);
    for (surfaceIndex = 0; (int)surfaceIndex < surfaceCount; ++surfaceIndex)
    {
        if (surfaceIndex >= 0xFF)
            MyAssertHandler(
                ".\\r_add_staticmodel.cpp",
                192,
                0,
                "surfaceIndex doesn't index UCHAR_MAX\n\t%i not in [0, %i)",
                surfaceIndex,
                255);
        material = *materialForSurf;
        if (!*materialForSurf)
            MyAssertHandler(".\\r_add_staticmodel.cpp", 195, 0, "%s", "material");
        if (rgp.sortedMaterials[(material->info.drawSurf.packed >> 29) & 0x7FF] != material)
            MyAssertHandler(
                ".\\r_add_staticmodel.cpp",
                201,
                0,
                "%s",
                "rgp.sortedMaterials[material->info.drawSurf.fields.materialSortedIndex] == material");
        region = material->cameraRegion;
        if (region != 3)
        {
            drawSurf.fields = (GfxDrawSurfFields)material->info.drawSurf.fields;
            HIDWORD(drawSurf.packed) = (primaryLightIndex << 10)
                | ((surfType & 0xF) << 18)
                | HIDWORD(drawSurf.packed) & 0xFFC003FF;
            if (surfType != 4
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
        if (!surfaceCount)
            MyAssertHandler(".\\r_add_staticmodel.cpp", 421, 0, "%s", "surfaceCount");
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

void __cdecl R_StaticModelWriteInfoHeader(int fileHandle)
{
    char dest; // [esp+10h] [ebp-1008h] BYREF
    _BYTE v2[4103]; // [esp+11h] [ebp-1007h] BYREF

    *(unsigned int *)&v2[4099] = 4096;
    Com_sprintf(
        &dest,
        0x1000u,
        "index,name,radius,numLods,lodDist,lodPixels720p,1PixelDist720p,scaledDist,posx,posy,posz,pixels\n");
    FS_Write(&dest, &v2[strlen(&dest)] - v2, fileHandle);
}

unsigned int _S1_0; 
float radius2pixels;
void __cdecl R_StaticModelWriteInfo(int fileHandle, const GfxStaticModelDrawInst *smodelDrawInst, const float dist)
{
    float v3; // [esp+5Ch] [ebp-101Ch]
    char dest; // [esp+60h] [ebp-1018h] BYREF
    _BYTE v5[4103]; // [esp+61h] [ebp-1017h] BYREF
    float v6; // [esp+1068h] [ebp-10h]
    XModel *model; // [esp+106Ch] [ebp-Ch]
    float v8; // [esp+1070h] [ebp-8h]
    float v9; // [esp+1074h] [ebp-4h]

    if ((_S1_0 & 1) == 0)
    {
        _S1_0 |= 1u;
        v3 = tan(22.5);
        radius2pixels = 720.0 / v3;
    }
    *(unsigned int *)&v5[4099] = 4096;
    model = smodelDrawInst->model;
    if (!model)
        MyAssertHandler(".\\r_add_staticmodel.cpp", 538, 0, "%s", "xmodel");
    if (!model->name)
        MyAssertHandler(".\\r_add_staticmodel.cpp", 539, 0, "%s", "xmodel->name");
    if (model->numLods <= 0)
        MyAssertHandler(".\\r_add_staticmodel.cpp", 540, 0, "%s", "xmodel->numLods > 0");
    v9 = *((float *)&model->parentList + 7 * model->numLods);
    if (v9 <= 0.0)
        MyAssertHandler(".\\r_add_staticmodel.cpp", 544, 0, "%s", "lodDist > 0.0f");
    v6 = radius2pixels * model->radius / v9;
    v8 = radius2pixels * model->radius;
    ++g_dumpStaticModelCount;
    if (smodelDrawInst->placement.scale > 0.0 && dist > 0.0)
        Com_sprintf(
            &dest,
            0x1000u,
            "%d,%s,%.1f,%d,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",
            g_dumpStaticModelCount,
            model->name,
            model->radius,
            model->numLods,
            v9,
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
    CL_ResetStats_f();
    R_SortDrawSurfs(scene.drawSurfs[1], scene.drawSurfCount[1]);
    CL_ResetStats_f();
    R_SortDrawSurfs(scene.drawSurfs[4], scene.drawSurfCount[4]);
    CL_ResetStats_f();
    R_SortDrawSurfs(scene.drawSurfs[10], scene.drawSurfCount[10]);
}

void __cdecl R_SortAllStaticModelSurfacesSunShadow()
{
    CL_ResetStats_f();
    R_SortDrawSurfs(scene.drawSurfs[16], scene.drawSurfCount[16]);
    CL_ResetStats_f();
    R_SortDrawSurfs(scene.drawSurfs[19], scene.drawSurfCount[19]);
}

void __cdecl R_AddAllStaticModelSurfacesSunShadow()
{
    R_AddAllStaticModelSurfacesRangeSunShadow(0, 0x1000u);
    R_AddAllStaticModelSurfacesRangeSunShadow(1u, 0x2000u);
}

void __cdecl R_AddAllStaticModelSurfacesRangeSunShadow(unsigned int partitionIndex, unsigned int maxDrawSurfCount)
{
    double v2; // st7
    float dist; // [esp+4h] [ebp-10E4h]
    float v4; // [esp+8h] [ebp-10E0h]
    float v5; // [esp+28h] [ebp-10C0h]
    float diff[8]; // [esp+30h] [ebp-10B8h] BYREF
    GfxStaticModelId StaticModelId; // [esp+50h] [ebp-1098h]
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+54h] [ebp-1094h]
    float scale; // [esp+58h] [ebp-1090h]
    float bias; // [esp+5Ch] [ebp-108Ch]
    unsigned __int16 staticModelLodList[4][4][128]; // [esp+60h] [ebp-1088h] BYREF
    GfxStaticModelDrawInst *smodelDrawInsts; // [esp+1064h] [ebp-84h]
    float v13; // [esp+1068h] [ebp-80h]
    XModel *v14; // [esp+106Ch] [ebp-7Ch]
    XModel *model; // [esp+1070h] [ebp-78h]
    GfxStaticModelId v16; // [esp+1074h] [ebp-74h]
    unsigned int v17; // [esp+1078h] [ebp-70h]
    unsigned __int16 *list; // [esp+107Ch] [ebp-6Ch]
    int lod; // [esp+1080h] [ebp-68h]
    GfxDrawSurf *smodelCount; // [esp+1084h] [ebp-64h]
    _WORD *v21; // [esp+1088h] [ebp-60h]
    unsigned __int8 *v22; // [esp+108Ch] [ebp-5Ch] BYREF
    float a[3]; // [esp+1090h] [ebp-58h] BYREF
    unsigned __int16 staticModelLodCount[4]; // [esp+109Ch] [ebp-4Ch] BYREF
    int v25; // [esp+10A4h] [ebp-44h]
    int v26; // [esp+10A8h] [ebp-40h]
    int v27; // [esp+10ACh] [ebp-3Ch]
    int v28; // [esp+10B0h] [ebp-38h]
    int v29; // [esp+10B4h] [ebp-34h]
    int v30; // [esp+10B8h] [ebp-30h]
    GfxSModelDrawSurfLightingData surfData; // [esp+10C0h] [ebp-28h] BYREF

    //Profile_Begin(411);
    smodelCount = (GfxDrawSurf *)rgp.world->dpvs.smodelCount;
    v22 = rgp.world->dpvs.smodelVisData[partitionIndex + 1];
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    if (!rg.correctedLodParms.valid)
        MyAssertHandler(".\\r_add_staticmodel.cpp", 1003, 0, "%s", "g_lodParms->valid");
    scale = rg.correctedLodParms.ramp[0].scale;
    bias = rg.correctedLodParms.ramp[0].bias;
    a[0] = rg.correctedLodParms.origin[0];
    a[1] = rg.correctedLodParms.origin[1];
    a[2] = rg.correctedLodParms.origin[2];
    R_InitBspDrawSurf(&surfData);
    v17 = 3 * partitionIndex + 16;
    surfData.drawSurf[0].current = scene.drawSurfs[v17];
    if (maxDrawSurfCount != scene.maxDrawSurfCount[v17])
        MyAssertHandler(".\\r_add_staticmodel.cpp", 1023, 0, "%s", "(int)maxDrawSurfCount == scene.maxDrawSurfCount[stage]");
    surfData.drawSurf[0].end = &surfData.drawSurf[0].current[maxDrawSurfCount];
    memset(staticModelLodCount, 0, sizeof(staticModelLodCount));
    v25 = 0;
    v26 = 0;
    v27 = 0;
    v28 = 0;
    v29 = 0;
    v30 = 0;
    surfData.drawSurf[1].end = 0;
    model = 0;
    for (surfData.drawSurf[2].end = 0; surfData.drawSurf[2].end < smodelCount; ++surfData.drawSurf[2].end)
    {
        if (*((_BYTE *)&surfData.drawSurf[2].end->fields + (unsigned int)v22))
        {
            smodelDrawInst = &smodelDrawInsts[(int)surfData.drawSurf[2].end];
            if ((smodelDrawInst->flags & 1) == 0)
            {
                Vec3Sub(a, smodelDrawInst->placement.origin, diff);
                v2 = Vec3Length(diff);
                v13 = v2 * scale + bias;
                if (smodelDrawInst->cullDist > (double)v13)
                {
                    v14 = smodelDrawInst->model;
                    if (v14 != model)
                    {
                        if (surfData.drawSurf[1].end)
                        {
                            R_SkinStaticModelsShadow(
                                model,
                                staticModelLodList,
                                (unsigned __int16 (*)[4])staticModelLodCount,
                                (GfxSModelDrawSurfData *)&surfData);
                            memset(staticModelLodCount, 0, sizeof(staticModelLodCount));
                            v25 = 0;
                            v26 = 0;
                            v27 = 0;
                            v28 = 0;
                            v29 = 0;
                            v30 = 0;
                            surfData.drawSurf[1].end = 0;
                        }
                        model = v14;
                    }
                    v5 = smodelDrawInst->placement.scale;
                    if (v5 == 0.0)
                        MyAssertHandler("c:\\trees\\cod3\\src\\universal\\com_math.h", 107, 0, "%s", "val");
                    v4 = 1.0 / v5;
                    dist = v13 * v4;
                    lod = XModelGetLodForDist(v14, dist);
                    if (lod >= 0)
                    {
                        if (R_AllocStaticModelLighting(smodelDrawInst, (unsigned int)surfData.drawSurf[2].end))
                        {
                            surfData.drawSurf[1].end = (GfxDrawSurf *)1;
                            StaticModelId = R_GetStaticModelId((unsigned int)surfData.drawSurf[2].end, lod);
                            v16 = StaticModelId;
                            v21 = (_WORD *)&a[2 * StaticModelId.surfType - 1] + lod;
                            list = staticModelLodList[StaticModelId.surfType - 2][lod];
                            surfData.drawSurf[2].current = (GfxDrawSurf *)(unsigned __int16)*v21;
                            list[(int)surfData.drawSurf[2].current++] = StaticModelId.objectId;
                            if (surfData.drawSurf[2].current >= (GfxDrawSurf *)0x80)
                            {
                                R_SkinStaticModelsShadowForLod(
                                    v14,
                                    (unsigned __int8 *)list,
                                    (unsigned int)surfData.drawSurf[2].current,
                                    v16.surfType,
                                    lod,
                                    (GfxSModelDrawSurfData *)&surfData);
                                *v21 = 0;
                            }
                            else
                            {
                                *v21 = (uint16)surfData.drawSurf[2].current;
                            }
                        }
                        else
                        {
                            R_WarnOncePerFrame(R_WARN_SMODEL_LIGHTING);
                        }
                    }
                }
                else
                {
                    *((_BYTE *)&surfData.drawSurf[2].end->fields + (unsigned int)v22) = 0;
                }
            }
        }
    }
    if (surfData.drawSurf[1].end)
        R_SkinStaticModelsShadow(
            model,
            staticModelLodList,
            (unsigned __int16 (*)[4])staticModelLodCount,
            (GfxSModelDrawSurfData *)&surfData);
    R_EndCmdBuf(&surfData.delayedCmdBuf);
    scene.drawSurfCount[v17] = surfData.drawSurf[0].current - scene.drawSurfs[v17];
    //Profile_EndInternal(0);
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
    if (!surfaceCount)
        MyAssertHandler(".\\r_add_staticmodel.cpp", 252, 0, "%s", "surfaceCount");
    materialForSurf = XModelGetSkins(model, lod);
    if (!materialForSurf)
        MyAssertHandler(".\\r_add_staticmodel.cpp", 255, 0, "%s", "materialForSurf");
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
        if (!*materialForSurf)
            MyAssertHandler(".\\r_add_staticmodel.cpp", 271, 0, "%s", "material");
        if (rgp.sortedMaterials[(material->info.drawSurf.packed >> 29) & 0x7FF] != material)
            MyAssertHandler(
                ".\\r_add_staticmodel.cpp",
                277,
                0,
                "%s",
                "rgp.sortedMaterials[material->info.drawSurf.fields.materialSortedIndex] == material");
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
            drawSurf.fields = material->info.drawSurf.fields;
            HIDWORD(drawSurf.packed) = ((surfType & 0xF) << 18) | HIDWORD(material->info.drawSurf.packed) & 0xFFC3FFFF;
            if (surfType != 4
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
    float v5; // [esp+Ch] [ebp-10ACh]
    float diff[3]; // [esp+14h] [ebp-10A4h] BYREF
    GfxStaticModelId StaticModelId; // [esp+20h] [ebp-1098h]
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+24h] [ebp-1094h]
    float scale; // [esp+28h] [ebp-1090h]
    float bias; // [esp+2Ch] [ebp-108Ch]
    unsigned __int16 staticModelLodList[4][4][128]; // [esp+30h] [ebp-1088h] BYREF
    GfxStaticModelDrawInst *smodelDrawInsts; // [esp+1030h] [ebp-88h]
    float v13; // [esp+1034h] [ebp-84h]
    XModel *model; // [esp+1038h] [ebp-80h]
    GfxShadowGeometry *v15; // [esp+103Ch] [ebp-7Ch]
    XModel *v16; // [esp+1040h] [ebp-78h]
    GfxStaticModelId v17; // [esp+1044h] [ebp-74h]
    unsigned int v18; // [esp+1048h] [ebp-70h]
    unsigned __int16 *list; // [esp+104Ch] [ebp-6Ch]
    int lod; // [esp+1050h] [ebp-68h]
    unsigned int i; // [esp+1054h] [ebp-64h]
    unsigned int smodelCount; // [esp+1058h] [ebp-60h]
    _WORD *v23; // [esp+105Ch] [ebp-5Ch] BYREF
    float a[3]; // [esp+1060h] [ebp-58h] BYREF
    unsigned __int16 staticModelLodCount[4]; // [esp+106Ch] [ebp-4Ch] BYREF
    int v26; // [esp+1074h] [ebp-44h]
    int v27; // [esp+1078h] [ebp-40h]
    int v28; // [esp+107Ch] [ebp-3Ch]
    int v29; // [esp+1080h] [ebp-38h]
    int v30; // [esp+1084h] [ebp-34h]
    int v31; // [esp+1088h] [ebp-30h]
    GfxSModelDrawSurfLightingData surfData; // [esp+1090h] [ebp-28h] BYREF

    smodelCount = rgp.world->dpvs.smodelCount;
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    if (!rg.lodParms.valid)
        MyAssertHandler(".\\r_add_staticmodel.cpp", 1266, 0, "%s", "rg.lodParms.valid");
    scale = rg.lodParms.ramp[0].scale;
    bias = rg.lodParms.ramp[0].bias;
    a[0] = rg.lodParms.origin[0];
    a[1] = rg.lodParms.origin[1];
    a[2] = rg.lodParms.origin[2];
    R_InitBspDrawSurf(&surfData);
    v18 = 3 * spotShadowIndex + 22;
    surfData.drawSurf[0].current = scene.drawSurfs[v18];
    surfData.drawSurf[0].end = &scene.drawSurfs[v18][scene.maxDrawSurfCount[v18]];
    v16 = 0;
    v15 = &rgp.world->shadowGeom[primaryLightIndex];
    for (i = 0; i < v15->smodelCount; ++i)
    {
        surfData.drawSurf[2].current = (GfxDrawSurf *)v15->smodelIndex[i];
        smodelDrawInst = &smodelDrawInsts[(int)surfData.drawSurf[2].current];
        model = smodelDrawInst->model;
        if ((smodelDrawInst->flags & 1) == 0)
        {
            Vec3Sub(a, smodelDrawInst->placement.origin, diff);
            v2 = Vec3Length(diff);
            v13 = v2 * scale + bias;
            if (smodelDrawInst->cullDist > (double)v13)
            {
                model = smodelDrawInst->model;
                v5 = smodelDrawInst->placement.scale;
                if (v5 == 0.0)
                    MyAssertHandler("c:\\trees\\cod3\\src\\universal\\com_math.h", 107, 0, "%s", "val");
                v4 = 1.0 / v5;
                dist = v13 * v4;
                lod = XModelGetLodForDist(model, dist);
                if (lod >= 0)
                {
                    if (R_AllocStaticModelLighting(smodelDrawInst, (unsigned int)surfData.drawSurf[2].current))
                    {
                        if (model != v16)
                        {
                            if (v16)
                                R_SkinStaticModelsShadow(
                                    v16,
                                    staticModelLodList,
                                    (unsigned __int16 (*)[4])staticModelLodCount,
                                    (GfxSModelDrawSurfData *)&surfData);
                            v16 = model;
                            memset(staticModelLodCount, 0, sizeof(staticModelLodCount));
                            v26 = 0;
                            v27 = 0;
                            v28 = 0;
                            v29 = 0;
                            v30 = 0;
                            v31 = 0;
                        }
                        StaticModelId = R_GetStaticModelId((unsigned int)surfData.drawSurf[2].current, lod);
                        v17 = StaticModelId;
                        v23 = (_WORD *)&a[2 * StaticModelId.surfType - 1] + lod;
                        list = staticModelLodList[StaticModelId.surfType - 2][lod];
                        surfData.drawSurf[1].end = (GfxDrawSurf *)(unsigned __int16)*v23;
                        list[(int)surfData.drawSurf[1].end++] = StaticModelId.objectId;
                        if (surfData.drawSurf[1].end >= (GfxDrawSurf *)0x80)
                        {
                            R_SkinStaticModelsShadowForLod(
                                model,
                                (unsigned __int8 *)list,
                                (unsigned int)surfData.drawSurf[1].end,
                                v17.surfType,
                                lod,
                                (GfxSModelDrawSurfData *)&surfData);
                            *v23 = 0;
                        }
                        else
                        {
                            *v23 = (uint16)surfData.drawSurf[1].end;
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
        R_SkinStaticModelsShadow(
            v16,
            staticModelLodList,
            (unsigned __int16 (*)[4])staticModelLodCount,
            (GfxSModelDrawSurfData *)&surfData);
    R_EndCmdBuf(&surfData.delayedCmdBuf);
    surfData.drawSurf[2].end = (GfxDrawSurf *)(surfData.drawSurf[0].current - scene.drawSurfs[v18]);
    scene.drawSurfCount[v18] = (volatile int)surfData.drawSurf[2].end;
    CL_ResetStats_f();
    R_SortDrawSurfs(scene.drawSurfs[v18], (int)surfData.drawSurf[2].end);
}

