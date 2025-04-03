#include "r_bsp.h"
#include "r_dvars.h"
#include "r_init.h"
#include "r_scene.h"
#include "r_pretess.h"
#include "r_buffers.h"

void __cdecl R_InitDelayedCmdBuf(GfxDelayedCmdBuf *delayedCmdBuf)
{
    delayedCmdBuf->primDrawSurfPos = -1;
    delayedCmdBuf->primDrawSurfSize = 0;
    *(_DWORD *)&delayedCmdBuf->drawSurfKey.fields = -1;
    HIDWORD(delayedCmdBuf->drawSurfKey.packed) = -1;
}

void __cdecl R_InitBspDrawSurf(GfxSModelDrawSurfLightingData *surfData)
{
    R_InitDelayedCmdBuf(&surfData->delayedCmdBuf);
}

char __cdecl R_PreTessBspDrawSurfs(
    GfxDrawSurf drawSurf,
    const unsigned __int16 *list,
    unsigned int count,
    GfxBspDrawSurfData *surfData)
{
    int v5; // [esp+1Ch] [ebp-248h]
    unsigned int simplifiedCount; // [esp+34h] [ebp-230h]
    unsigned __int16 surfIndex; // [esp+38h] [ebp-22Ch]
    const GfxSurface *tris; // [esp+3Ch] [ebp-228h]
    unsigned int copyIndex; // [esp+40h] [ebp-224h]
    GfxBspPreTessDrawSurf simplifiedList[128]; // [esp+44h] [ebp-220h] BYREF
    unsigned int lmapIndex; // [esp+244h] [ebp-20h]
    unsigned __int16 *preTessIndices; // [esp+248h] [ebp-1Ch]
    unsigned int firstIndex; // [esp+24Ch] [ebp-18h]
    unsigned int triCount; // [esp+250h] [ebp-14h]
    unsigned int reflectionProbeIndex; // [esp+254h] [ebp-10h]
    const GfxSurface *surf; // [esp+258h] [ebp-Ch]
    unsigned int surfIter; // [esp+25Ch] [ebp-8h]
    int baseVertex; // [esp+260h] [ebp-4h]

    triCount = 0;
    for (surfIter = 0; surfIter < count; ++surfIter)
    {
        if ((unsigned int)list[surfIter] >= rgp.world->surfaceCount)
            MyAssertHandler(
                ".\\r_add_bsp.cpp",
                108,
                0,
                "list[surfIter] doesn't index rgp.world->surfaceCount\n\t%i not in [0, %i)",
                list[surfIter],
                rgp.world->surfaceCount);
        triCount += rgp.world->dpvs.surfaces[list[surfIter]].tris.triCount;
    }
    preTessIndices = R_AllocPreTessIndices(3 * triCount);
    if (!preTessIndices)
        return 0;
    //Profile_Begin(166);
    copyIndex = 0;
    simplifiedCount = 0;
    baseVertex = 0x7FFFFFFF;
    lmapIndex = 31;
    reflectionProbeIndex = 255;
    for (surfIter = 0; surfIter < count; ++surfIter)
    {
        surfIndex = list[surfIter];
        surf = &rgp.world->dpvs.surfaces[surfIndex];
        tris = surf;
        if (baseVertex != surf->tris.firstVertex
            || lmapIndex != surf->lightmapIndex
            || reflectionProbeIndex != surf->reflectionProbeIndex)
        {
            baseVertex = surf->tris.firstVertex;
            lmapIndex = surf->lightmapIndex;
            reflectionProbeIndex = surf->reflectionProbeIndex;
            simplifiedList[simplifiedCount].baseSurfIndex = surfIndex;
            simplifiedList[simplifiedCount++].totalTriCount = 0;
        }
        Com_Memcpy(
            (char *)&preTessIndices[copyIndex],
            (char *)&rgp.world->indices[tris->tris.baseIndex],
            6 * tris->tris.triCount);
        copyIndex += 3 * tris->tris.triCount;
        v5 = tris->tris.triCount + *((unsigned __int16 *)&copyIndex + 2 * simplifiedCount + 1);
        if (v5 != (unsigned __int16)v5)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                281,
                0,
                "i == static_cast< Type >( i )\n\t%i, %i",
                v5,
                (unsigned __int16)v5);
        *((_WORD *)&copyIndex + 2 * simplifiedCount + 1) = v5;
    }
    //Profile_EndInternal(0);
    HIDWORD(drawSurf.packed) = HIDWORD(drawSurf.packed) & 0xFFC3FFFF | 0x40000;
    if (R_AllocDrawSurf(&surfData->delayedCmdBuf, drawSurf, &surfData->drawSurfList, simplifiedCount + 2))
    {
        firstIndex = preTessIndices - gfxBuf.preTessIndexBuffer->indices;
        if (firstIndex >= 0x100000)
            MyAssertHandler(
                ".\\r_add_bsp.cpp",
                148,
                0,
                "firstIndex doesn't index R_MAX_PRETESS_INDICES\n\t%i not in [0, %i)",
                firstIndex,
                0x100000);
        R_WritePrimDrawSurfInt(&surfData->delayedCmdBuf, simplifiedCount);
        R_WritePrimDrawSurfInt(&surfData->delayedCmdBuf, firstIndex);
        R_WritePrimDrawSurfData(&surfData->delayedCmdBuf, (unsigned __int8 *)simplifiedList, simplifiedCount);
    }
    return 1;
}

void __cdecl R_AddBspDrawSurfs(
    GfxDrawSurf drawSurf,
    unsigned __int8 *list,
    unsigned int count,
    GfxBspDrawSurfData *surfData)
{
    bool v4; // [esp+Bh] [ebp-1h]

    if (((drawSurf.packed >> 50) & 0xF) != 0)
        MyAssertHandler(
            ".\\r_add_bsp.cpp",
            162,
            0,
            "%s\n\t(drawSurf.fields.surfType) = %i",
            "(drawSurf.fields.surfType == SF_TRIANGLES)",
            (unsigned int)(drawSurf.packed >> 50) & 0xF);
    v4 = !dx.deviceLost && r_pretess->current.enabled;
    if (!v4 || !R_PreTessBspDrawSurfs(drawSurf, (const unsigned __int16 *)list, count, surfData))
    {
        if (R_AllocDrawSurf(&surfData->delayedCmdBuf, drawSurf, &surfData->drawSurfList, ((count + 1) >> 1) + 1))
        {
            R_WritePrimDrawSurfInt(&surfData->delayedCmdBuf, count);
            R_WritePrimDrawSurfData(&surfData->delayedCmdBuf, list, (count + 1) >> 1);
        }
    }
}

void __cdecl R_AddAllBspDrawSurfacesCamera()
{
    R_AddAllBspDrawSurfacesRangeCamera(rgp.world->dpvs.litSurfsBegin, rgp.world->dpvs.litSurfsEnd, 0, 0x2000u);
    R_AddAllBspDrawSurfacesRangeCamera(rgp.world->dpvs.decalSurfsBegin, rgp.world->dpvs.decalSurfsEnd, 3u, 0x200u);
}

void __cdecl R_AddAllBspDrawSurfacesRangeCamera(
    unsigned int beginSurface,
    unsigned int endSurface,
    unsigned int stage,
    unsigned int maxDrawSurfCount)
{
    int packed_high; // edx
    unsigned __int16 triSurfList[128]; // [esp+30h] [ebp-148h] BYREF
    int debugFastSunShadow; // [esp+138h] [ebp-40h]
    unsigned int *surfaceCastsSunShadow; // [esp+13Ch] [ebp-3Ch]
    GfxDrawSurf drawSurf; // [esp+140h] [ebp-38h]
    GfxDrawSurf prevDrawSurf; // [esp+148h] [ebp-30h]
    unsigned int sortedSurfIndex; // [esp+150h] [ebp-28h]
    unsigned __int8 *surfaceVisData; // [esp+154h] [ebp-24h]
    unsigned int triSurfCount; // [esp+158h] [ebp-20h]
    GfxDrawSurf *surfaceMaterials; // [esp+15Ch] [ebp-1Ch]
    GfxBspDrawSurfData surfData; // [esp+160h] [ebp-18h] BYREF

    //Profile_Begin(408);
    if (!rgp.world)
        MyAssertHandler(".\\r_add_bsp.cpp", 205, 0, "%s", "rgp.world");
    if (beginSurface >= (unsigned int)rgp.world->models->surfaceCount + 1)
        MyAssertHandler(
            ".\\r_add_bsp.cpp",
            206,
            0,
            "beginSurface doesn't index rgp.world->models[0].surfaceCount + 1\n\t%i not in [0, %i)",
            beginSurface,
            rgp.world->models->surfaceCount + 1);
    if (endSurface >= (unsigned int)rgp.world->models->surfaceCount + 1)
        MyAssertHandler(
            ".\\r_add_bsp.cpp",
            207,
            0,
            "endSurface doesn't index rgp.world->models[0].surfaceCount + 1\n\t%i not in [0, %i)",
            endSurface,
            rgp.world->models->surfaceCount + 1);
    surfaceVisData = rgp.world->dpvs.surfaceVisData[0];
    R_InitBspDrawSurf((GfxSModelDrawSurfLightingData *)&surfData);
    surfData.drawSurfList.current = scene.drawSurfs[stage];
    if (maxDrawSurfCount != scene.maxDrawSurfCount[stage])
        MyAssertHandler(".\\r_add_bsp.cpp", 222, 0, "%s", "(int)maxDrawSurfCount == scene.maxDrawSurfCount[stage]");
    surfaceMaterials = rgp.world->dpvs.surfaceMaterials;
    surfData.drawSurfList.end = &surfData.drawSurfList.current[maxDrawSurfCount];
    prevDrawSurf.packed = -1;
    triSurfCount = 0;
    debugFastSunShadow = sm_debugFastSunShadow->current.color[0];
    surfaceCastsSunShadow = rgp.world->dpvs.surfaceCastsSunShadow;
    for (sortedSurfIndex = beginSurface; sortedSurfIndex < endSurface; ++sortedSurfIndex)
    {
        if (surfaceVisData[sortedSurfIndex]
            && (!debugFastSunShadow || (surfaceCastsSunShadow[sortedSurfIndex >> 5] & (1 << (sortedSurfIndex & 0x1F))) != 0))
        {
            packed_high = HIDWORD(surfaceMaterials[sortedSurfIndex].packed);
            //*(unsigned int *)&drawSurf.fields = surfaceMaterials[sortedSurfIndex].fields;
            drawSurf.fields = surfaceMaterials[sortedSurfIndex].fields;
            HIDWORD(drawSurf.packed) = packed_high;
            if (drawSurf.packed != prevDrawSurf.packed)
            {
                if (triSurfCount)
                    R_AddBspDrawSurfs(prevDrawSurf, (unsigned __int8 *)triSurfList, triSurfCount, &surfData);
                Com_BitSetAssert(scene.shadowableLightIsUsed, (unsigned __int8)(drawSurf.packed >> 42), 128);
                prevDrawSurf.fields = drawSurf.fields;
                triSurfCount = 0;
            }
            triSurfList[triSurfCount] = sortedSurfIndex;
            if (triSurfList[triSurfCount] != sortedSurfIndex)
                MyAssertHandler(
                    ".\\r_add_bsp.cpp",
                    273,
                    0,
                    "triSurfList[triSurfCount] == sortedSurfIndex\n\t%i, %i",
                    triSurfList[triSurfCount],
                    sortedSurfIndex);
            if (++triSurfCount >= 0x80)
            {
                R_AddBspDrawSurfs(drawSurf, (unsigned __int8 *)triSurfList, triSurfCount, &surfData);
                triSurfCount = 0;
            }
        }
    }
    if (triSurfCount)
        R_AddBspDrawSurfs(prevDrawSurf, (unsigned __int8 *)triSurfList, triSurfCount, &surfData);
    R_EndCmdBuf(&surfData.delayedCmdBuf);
    scene.drawSurfCount[stage] = surfData.drawSurfList.current - scene.drawSurfs[stage];
    //Profile_EndInternal(0);
}

void __cdecl R_AddAllBspDrawSurfacesCameraNonlit(
    unsigned int beginSurface,
    unsigned int endSurface,
    unsigned int stage)
{
    int packed_high; // edx
    unsigned __int16 triSurfList[128]; // [esp+0h] [ebp-148h] BYREF
    GfxDrawSurf drawSurf; // [esp+108h] [ebp-40h]
    GfxDrawSurf prevDrawSurf; // [esp+110h] [ebp-38h]
    unsigned int sortedSurfIndex; // [esp+118h] [ebp-30h]
    unsigned __int8 *surfaceVisData; // [esp+11Ch] [ebp-2Ch]
    unsigned int triSurfCount; // [esp+120h] [ebp-28h]
    GfxDrawSurf *surfaceMaterials; // [esp+124h] [ebp-24h]
    GfxDelayedCmdBuf surfData; // [esp+128h] [ebp-20h] BYREF
    GfxDrawSurf *surfData_16; // [esp+138h] [ebp-10h]
    GfxDrawSurf *surfData_20; // [esp+13Ch] [ebp-Ch]
    int drawSurfCount; // [esp+144h] [ebp-4h]

    if (!rgp.world)
        MyAssertHandler(".\\r_add_bsp.cpp", 357, 0, "%s", "rgp.world");
    surfaceVisData = rgp.world->dpvs.surfaceVisData[0];
    surfaceMaterials = rgp.world->dpvs.surfaceMaterials;
    R_InitBspDrawSurf((GfxSModelDrawSurfLightingData *)&surfData);
    surfData_16 = scene.drawSurfs[stage];
    surfData_20 = &scene.drawSurfs[stage][scene.maxDrawSurfCount[stage]];
    prevDrawSurf.packed = -1;
    triSurfCount = 0;
    for (sortedSurfIndex = beginSurface; sortedSurfIndex < endSurface; ++sortedSurfIndex)
    {
        if (surfaceVisData[sortedSurfIndex])
        {
            packed_high = HIDWORD(surfaceMaterials[sortedSurfIndex].packed);
            //*(unsigned int *)&drawSurf.fields = surfaceMaterials[sortedSurfIndex].fields;
            drawSurf.fields = surfaceMaterials[sortedSurfIndex].fields;
            HIDWORD(drawSurf.packed) = packed_high;
            if (drawSurf.packed != prevDrawSurf.packed)
            {
                if (triSurfCount)
                    R_AddBspDrawSurfs(prevDrawSurf, (unsigned __int8 *)triSurfList, triSurfCount, (GfxBspDrawSurfData *)&surfData);
                prevDrawSurf.fields = drawSurf.fields;
                triSurfCount = 0;
            }
            triSurfList[triSurfCount] = sortedSurfIndex;
            if (triSurfList[triSurfCount] != sortedSurfIndex)
                MyAssertHandler(
                    ".\\r_add_bsp.cpp",
                    390,
                    0,
                    "triSurfList[triSurfCount] == sortedSurfIndex\n\t%i, %i",
                    triSurfList[triSurfCount],
                    sortedSurfIndex);
            if (++triSurfCount >= 0x80)
            {
                R_AddBspDrawSurfs(drawSurf, (unsigned __int8 *)triSurfList, triSurfCount, (GfxBspDrawSurfData *)&surfData);
                triSurfCount = 0;
            }
        }
    }
    if (triSurfCount)
        R_AddBspDrawSurfs(prevDrawSurf, (unsigned __int8 *)triSurfList, triSurfCount, (GfxBspDrawSurfData *)&surfData);
    R_EndCmdBuf(&surfData);
    drawSurfCount = surfData_16 - scene.drawSurfs[stage];
    scene.drawSurfCount[stage] = drawSurfCount;
}

void __cdecl R_AddAllBspDrawSurfacesSunShadow()
{
    if (rgp.world->dpvs.litSurfsEnd != rgp.world->dpvs.decalSurfsBegin)
        MyAssertHandler(".\\r_add_bsp.cpp", 593, 0, "%s", "rgp.world->dpvs.litSurfsEnd == rgp.world->dpvs.decalSurfsBegin");
    if (rgp.world->dpvs.decalSurfsEnd != rgp.world->dpvs.emissiveSurfsBegin)
        MyAssertHandler(
            ".\\r_add_bsp.cpp",
            594,
            0,
            "%s",
            "rgp.world->dpvs.decalSurfsEnd == rgp.world->dpvs.emissiveSurfsBegin");
    R_AddAllBspDrawSurfacesRangeSunShadow(0, rgp.world->dpvs.litSurfsBegin, rgp.world->dpvs.emissiveSurfsEnd, 0x1000u);
    R_AddAllBspDrawSurfacesRangeSunShadow(1u, rgp.world->dpvs.litSurfsBegin, rgp.world->dpvs.emissiveSurfsEnd, 0x2000u);
}

void __cdecl R_AddAllBspDrawSurfacesRangeSunShadow(
    unsigned int partitionIndex,
    unsigned int beginSurface,
    unsigned int endSurface,
    unsigned int maxDrawSurfCount)
{
    int v4; // eax
    int packed_high; // edx
    unsigned __int16 triSurfList[128]; // [esp+3Ch] [ebp-158h] BYREF
    unsigned int *surfaceCastsSunShadow; // [esp+140h] [ebp-54h]
    GfxDrawSurf drawSurf; // [esp+144h] [ebp-50h]
    GfxDrawSurf prevDrawSurf; // [esp+14Ch] [ebp-48h]
    unsigned int stage; // [esp+158h] [ebp-3Ch]
    unsigned int sortedSurfIndex; // [esp+15Ch] [ebp-38h]
    unsigned __int8 *surfaceVisData; // [esp+160h] [ebp-34h]
    int hasApproxSunDirChanged; // [esp+164h] [ebp-30h]
    unsigned int triSurfCount; // [esp+168h] [ebp-2Ch]
    GfxDrawSurf *surfaceMaterials; // [esp+16Ch] [ebp-28h]
    int fastSunShadow; // [esp+170h] [ebp-24h]
    GfxDelayedCmdBuf surfData; // [esp+174h] [ebp-20h] BYREF
    GfxDrawSurf *surfData_16; // [esp+184h] [ebp-10h]
    GfxDrawSurf *surfData_20; // [esp+188h] [ebp-Ch]
    int skipMaterial; // [esp+190h] [ebp-4h]

    //Profile_Begin(409);
    if (!rgp.world)
        MyAssertHandler(".\\r_add_bsp.cpp", 437, 0, "%s", "rgp.world");
    if (beginSurface >= (unsigned int)rgp.world->models->surfaceCount + 1)
        MyAssertHandler(
            ".\\r_add_bsp.cpp",
            438,
            0,
            "beginSurface doesn't index rgp.world->models[0].surfaceCount + 1\n\t%i not in [0, %i)",
            beginSurface,
            rgp.world->models->surfaceCount + 1);
    if (endSurface >= (unsigned int)rgp.world->models->surfaceCount + 1)
        MyAssertHandler(
            ".\\r_add_bsp.cpp",
            439,
            0,
            "endSurface doesn't index rgp.world->models[0].surfaceCount + 1\n\t%i not in [0, %i)",
            endSurface,
            rgp.world->models->surfaceCount + 1);
    surfaceVisData = rgp.world->dpvs.surfaceVisData[partitionIndex + 1];
    R_InitBspDrawSurf((GfxSModelDrawSurfLightingData *)&surfData);
    stage = 3 * partitionIndex + 15;
    surfData_16 = scene.drawSurfs[stage];
    if (maxDrawSurfCount != scene.maxDrawSurfCount[stage])
        MyAssertHandler(".\\r_add_bsp.cpp", 456, 0, "%s", "(int)maxDrawSurfCount == scene.maxDrawSurfCount[stage]");
    surfaceMaterials = rgp.world->dpvs.surfaceMaterials;
    surfData_20 = &surfData_16[maxDrawSurfCount];
    prevDrawSurf.packed = -1;
    skipMaterial = 0;
    triSurfCount = 0;
    surfaceCastsSunShadow = rgp.world->dpvs.surfaceCastsSunShadow;
    fastSunShadow = sm_fastSunShadow->current.color[0];
    hasApproxSunDirChanged = frontEndDataOut->hasApproxSunDirChanged;
    if (!hasApproxSunDirChanged && fastSunShadow)
    {
        for (sortedSurfIndex = beginSurface; sortedSurfIndex < endSurface; ++sortedSurfIndex)
        {
            if (surfaceVisData[sortedSurfIndex]
                && (surfaceCastsSunShadow[sortedSurfIndex >> 5] & (1 << (sortedSurfIndex & 0x1F))) != 0)
            {
                packed_high = HIDWORD(surfaceMaterials[sortedSurfIndex].packed);
                //*(unsigned int *)&drawSurf.fields = surfaceMaterials[sortedSurfIndex].fields;
                drawSurf.fields = surfaceMaterials[sortedSurfIndex].fields;
                HIDWORD(drawSurf.packed) = packed_high;
                if (drawSurf.packed != prevDrawSurf.packed)
                {
                    if (triSurfCount)
                        R_AddBspDrawSurfs(
                            prevDrawSurf,
                            (unsigned __int8 *)triSurfList,
                            triSurfCount,
                            (GfxBspDrawSurfData *)&surfData);
                    prevDrawSurf.fields = drawSurf.fields;
                    triSurfCount = 0;
                }
                triSurfList[triSurfCount] = sortedSurfIndex;
                if (triSurfList[triSurfCount] != sortedSurfIndex)
                    MyAssertHandler(
                        ".\\r_add_bsp.cpp",
                        551,
                        0,
                        "triSurfList[triSurfCount] == sortedSurfIndex\n\t%i, %i",
                        triSurfList[triSurfCount],
                        sortedSurfIndex);
                if (++triSurfCount >= 0x80)
                {
                    R_AddBspDrawSurfs(drawSurf, (unsigned __int8 *)triSurfList, triSurfCount, (GfxBspDrawSurfData *)&surfData);
                    triSurfCount = 0;
                }
            }
        }
    }
    else
    {
        for (sortedSurfIndex = beginSurface; sortedSurfIndex < endSurface; ++sortedSurfIndex)
        {
            if (surfaceVisData[sortedSurfIndex])
            {
                v4 = HIDWORD(surfaceMaterials[sortedSurfIndex].packed);
                //*(unsigned int *)&drawSurf.fields = surfaceMaterials[sortedSurfIndex].fields;
                drawSurf.fields = surfaceMaterials[sortedSurfIndex].fields;
                HIDWORD(drawSurf.packed) = v4;
                if (drawSurf.packed != prevDrawSurf.packed)
                {
                    if (triSurfCount)
                    {
                        if (skipMaterial)
                            MyAssertHandler(".\\r_add_bsp.cpp", 499, 0, "%s", "!skipMaterial");
                        R_AddBspDrawSurfs(
                            prevDrawSurf,
                            (unsigned __int8 *)triSurfList,
                            triSurfCount,
                            (GfxBspDrawSurfData *)&surfData);
                    }
                    prevDrawSurf.fields = drawSurf.fields;
                    skipMaterial = ((drawSurf.packed >> 24) & 0x1F) == 0;
                    triSurfCount = 0;
                }
                if (!skipMaterial)
                {
                    triSurfList[triSurfCount] = sortedSurfIndex;
                    if (triSurfList[triSurfCount] != sortedSurfIndex)
                        MyAssertHandler(
                            ".\\r_add_bsp.cpp",
                            511,
                            0,
                            "triSurfList[triSurfCount] == sortedSurfIndex\n\t%i, %i",
                            triSurfList[triSurfCount],
                            sortedSurfIndex);
                    if (++triSurfCount >= 0x80)
                    {
                        R_AddBspDrawSurfs(drawSurf, (unsigned __int8 *)triSurfList, triSurfCount, (GfxBspDrawSurfData *)&surfData);
                        triSurfCount = 0;
                    }
                }
            }
        }
    }
    if (triSurfCount)
    {
        if (skipMaterial)
            MyAssertHandler(".\\r_add_bsp.cpp", 564, 0, "%s", "!skipMaterial");
        R_AddBspDrawSurfs(prevDrawSurf, (unsigned __int8 *)triSurfList, triSurfCount, (GfxBspDrawSurfData *)&surfData);
    }
    R_EndCmdBuf(&surfData);
    scene.drawSurfCount[stage] = surfData_16 - scene.drawSurfs[stage];
    //Profile_EndInternal(0);
}

void __cdecl R_AddAllBspDrawSurfacesSpotShadow(unsigned int spotShadowIndex, unsigned int primaryLightIndex)
{
    int packed_high; // eax
    unsigned __int16 triSurfList[128]; // [esp+0h] [ebp-158h] BYREF
    GfxDrawSurf drawSurf; // [esp+108h] [ebp-50h]
    GfxShadowGeometry *shadowGeom; // [esp+114h] [ebp-44h]
    GfxDrawSurf prevDrawSurf; // [esp+118h] [ebp-40h]
    unsigned int stage; // [esp+120h] [ebp-38h]
    unsigned int sortedSurfIndex; // [esp+124h] [ebp-34h]
    unsigned int triSurfCount; // [esp+128h] [ebp-30h]
    GfxDrawSurf *drawSurfs; // [esp+12Ch] [ebp-2Ch]
    unsigned int surfIter; // [esp+130h] [ebp-28h]
    GfxDrawSurf *surfaceMaterials; // [esp+134h] [ebp-24h]
    GfxDelayedCmdBuf surfData; // [esp+138h] [ebp-20h] BYREF
    GfxDrawSurf *surfData_16; // [esp+148h] [ebp-10h]
    GfxDrawSurf *surfData_20; // [esp+14Ch] [ebp-Ch]
    int drawSurfCount; // [esp+154h] [ebp-4h]

    if (!rgp.world)
        MyAssertHandler(".\\r_add_bsp.cpp", 659, 0, "%s", "rgp.world");
    surfaceMaterials = rgp.world->dpvs.surfaceMaterials;
    stage = 3 * spotShadowIndex + 21;
    R_InitBspDrawSurf((GfxSModelDrawSurfLightingData *)&surfData);
    drawSurfs = &scene.drawSurfs[stage][scene.drawSurfCount[stage]];
    surfData_16 = drawSurfs;
    surfData_20 = &scene.drawSurfs[stage][scene.maxDrawSurfCount[stage]];
    prevDrawSurf.packed = -1;
    triSurfCount = 0;
    shadowGeom = &rgp.world->shadowGeom[primaryLightIndex];
    for (surfIter = 0; surfIter < shadowGeom->surfaceCount; ++surfIter)
    {
        sortedSurfIndex = shadowGeom->sortedSurfIndex[surfIter];
        packed_high = HIDWORD(surfaceMaterials[sortedSurfIndex].packed);
        //*(unsigned int *)&drawSurf.fields = surfaceMaterials[sortedSurfIndex].fields;
        drawSurf.fields = surfaceMaterials[sortedSurfIndex].fields;
        HIDWORD(drawSurf.packed) = packed_high;
        if (drawSurf.packed != prevDrawSurf.packed)
        {
            if (triSurfCount)
                R_AddBspDrawSurfs(prevDrawSurf, (unsigned __int8 *)triSurfList, triSurfCount, (GfxBspDrawSurfData *)&surfData);
            prevDrawSurf.fields = drawSurf.fields;
            triSurfCount = 0;
        }
        triSurfList[triSurfCount] = sortedSurfIndex;
        if (triSurfList[triSurfCount] != sortedSurfIndex)
            MyAssertHandler(
                ".\\r_add_bsp.cpp",
                688,
                0,
                "triSurfList[triSurfCount] == sortedSurfIndex\n\t%i, %i",
                triSurfList[triSurfCount],
                sortedSurfIndex);
        if (++triSurfCount >= 0x80)
        {
            R_AddBspDrawSurfs(drawSurf, (unsigned __int8 *)triSurfList, triSurfCount, (GfxBspDrawSurfData *)&surfData);
            triSurfCount = 0;
        }
    }
    if (triSurfCount)
        R_AddBspDrawSurfs(prevDrawSurf, (unsigned __int8 *)triSurfList, triSurfCount, (GfxBspDrawSurfData *)&surfData);
    R_EndCmdBuf(&surfData);
    drawSurfCount = surfData_16 - drawSurfs;
    scene.drawSurfCount[stage] += drawSurfCount;
}

