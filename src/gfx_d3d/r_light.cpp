#include "r_light.h"
#include <qcommon/qcommon.h>
#include <universal/com_files.h>
#include <universal/com_memory.h>
#include "r_image.h"
#include "r_init.h"
#include "r_scene.h"
#include "r_dvars.h"
#include <database/database.h>
#include "r_bsp.h"
#include "r_marks.h"

#include <algorithm>
#include "r_staticmodelcache.h"
#include "r_add_staticmodel.h"
#include <DynEntity/DynEntity_client.h>
#include "fxprimitives.h"
#include <EffectsCore/fx_system.h>
#include "r_pretess.h"
#include <qcommon/com_bsp.h>
#include <universal/profile.h>

struct StaticModelLightCallback // sizeof=0x74
{                                       // ...
    unsigned __int8 *smodelVisData;     // ...
    float position[3];                  // ...
    float radiusSq;                     // ...
    float planes[6][4];                 // ...
};

LightGlobals lightGlob;
StaticModelLightCallback g_staticModelLightCallback;

int(__cdecl *allowSurf_0[1])(int, void *) = { R_AllowBspOmniLight };
int(__cdecl *allowSurf_1[2])(int, void *) = { R_AllowBspSpotLight, R_AllowBspSpotLightShadows };

void __cdecl R_EnumLightDefs(void(__cdecl *func)(GfxLightDef *, void *), void *data)
{
    GfxLightDef *def; // [esp+0h] [ebp-8h]
    int defIndex; // [esp+4h] [ebp-4h]

    for (defIndex = 0; defIndex < lightGlob.defCount; ++defIndex)
    {
        def = lightGlob.defs[defIndex];
        iassert( def );
        func(def, data);
    }
}

GfxLightDef *__cdecl R_RegisterLightDef_LoadObj(const char *name)
{
    GfxLightDef *def; // [esp+0h] [ebp-8h]
    GfxLightDef *defa; // [esp+0h] [ebp-8h]
    int defIndex; // [esp+4h] [ebp-4h]
    int defIndexa; // [esp+4h] [ebp-4h]

    iassert( name );
    for (defIndex = 0; defIndex < lightGlob.defCount; ++defIndex)
    {
        iassert( lightGlob.defs[defIndex] );
        iassert( lightGlob.defs[defIndex]->name );
        if (!I_stricmp(name, lightGlob.defs[defIndex]->name))
            return lightGlob.defs[defIndex];
    }
    if (lightGlob.defCount == 64)
    {
        Com_Printf(8, "Loaded light defs:\n");
        for (defIndexa = 0; defIndexa < lightGlob.defCount; ++defIndexa)
            Com_Printf(8, "  %s\n", lightGlob.defs[defIndexa]->name);
        Com_Error(
            ERR_DROP,
            "Can't load light def %s; %i unique light defs already loaded",
            name,
            lightGlob.defCount
            );
    }
    defa = R_LoadLightDef(name);
    if (defa)
    {
        lightGlob.defs[lightGlob.defCount++] = defa;
        return defa;
    }
    else
    {
        if (!I_stricmp(name, "light_dynamic"))
            Com_Error(ERR_DROP, "Can't load the default light def '%s'", name);
        return R_RegisterLightDef("light_dynamic");
    }
}

GfxLightDef *__cdecl R_RegisterLightDef(const char *name)
{
    if (useFastFile->current.enabled)
        return R_RegisterLightDef_FastFile(name);
    else
        return R_RegisterLightDef_LoadObj(name);
}

GfxLightDef *__cdecl R_RegisterLightDef_FastFile(const char *name)
{
    return DB_FindXAssetHeader(ASSET_TYPE_LIGHT_DEF, name).lightDef;
}

void __cdecl R_InitLightDefs()
{
    iassert( lightGlob.defCount == 0 );
    rgp.dlightDef = R_RegisterLightDef("light_dynamic");
}

void __cdecl R_ShutdownLightDefs()
{
    lightGlob.defCount = 0;
}

int __cdecl R_GetPointLightPartitions(const GfxLight **visibleLights)
{
    const GfxLight *addedLights[32]; // [esp+0h] [ebp-90h] BYREF
    int visibleCount; // [esp+84h] [ebp-Ch]
    int visibleLimit; // [esp+88h] [ebp-8h]
    int lightIndex; // [esp+8Ch] [ebp-4h]

    if (scene.addedLightCount > 32)
        MyAssertHandler(
            ".\\r_light.cpp",
            229,
            1,
            "%s\n\t(scene.addedLightCount) = %i",
            "(scene.addedLightCount <= 32)",
            scene.addedLightCount);
    visibleCount = 0;
    for (lightIndex = 0; lightIndex < scene.addedLightCount; ++lightIndex)
    {
        if (!scene.isAddedLightCulled[lightIndex])
        {
            iassert( (visibleCount <= 32) );
            addedLights[visibleCount++] = &scene.addedLight[lightIndex];
        }
    }
    visibleLimit = r_dlightLimit->current.integer;
    iassert( (visibleLimit <= 4) );
    if (visibleCount > visibleLimit)
    {
        R_MostImportantLights(addedLights, visibleCount, visibleLimit);
        visibleCount = visibleLimit;
    }
    for (lightIndex = 0; lightIndex < visibleCount; ++lightIndex)
    {
        visibleLights[lightIndex] = addedLights[lightIndex];
        scene.visLightShadow[lightIndex - 4].drawSurfCount = 0;
    }
    return visibleCount;
}

void __cdecl R_MostImportantLights(const GfxLight **lights, int lightCount, int keepCount)
{
    int top; // [esp+54h] [ebp-10h]
    int bot; // [esp+58h] [ebp-Ch]
    const GfxLight *pivot; // [esp+5Ch] [ebp-8h]
    const GfxLight *swapCache; // [esp+60h] [ebp-4h]
    const GfxLight *swapCachea; // [esp+60h] [ebp-4h]

    iassert( lightCount > keepCount );
    iassert( keepCount >= 1 );
    while (1)
    {
        bot = 0;
        top = lightCount;
        pivot = *lights;
        while (1)
        {
            iassert( top >= bot );
            do
                ++bot;
            while (bot < top && R_LightImportanceGreaterEqual(lights[bot], pivot));
            if (bot > top)
                break;
            do
                --top;
            while (top >= bot && R_LightImportanceGreaterEqual(pivot, lights[top]));
            if (bot > top)
                break;
            swapCache = lights[bot];
            lights[bot] = lights[top];
            lights[top] = swapCache;
        }
        iassert( bot == top + 1 );
        if (bot == lightCount)
        {
            swapCachea = *lights;
            *lights = lights[top];
            lights[top] = swapCachea;
            --bot;
        }
        if (bot == keepCount)
            break;
        iassert( lightCount > bot );
        if (bot >= keepCount)
        {
            lightCount = bot;
        }
        else
        {
            lights += bot;
            lightCount -= bot;
            keepCount -= bot;
        }
    }
}

bool __cdecl R_LightImportanceGreaterEqual(const GfxLight *light0, const GfxLight *light1)
{
    float v[3]; // [esp+4h] [ebp-28h] BYREF
    float diff[3]; // [esp+10h] [ebp-1Ch] BYREF
    float radiusSq[2]; // [esp+1Ch] [ebp-10h]
    float distSq[2]; // [esp+24h] [ebp-8h]

    if (light0->type != 3 && light0->type != 2)
        MyAssertHandler(
            ".\\r_light.cpp",
            132,
            1,
            "%s",
            "light0->type == GFX_LIGHT_TYPE_OMNI || light0->type == GFX_LIGHT_TYPE_SPOT");
    if (light1->type != 3 && light1->type != 2)
        MyAssertHandler(
            ".\\r_light.cpp",
            133,
            1,
            "%s",
            "light1->type == GFX_LIGHT_TYPE_OMNI || light1->type == GFX_LIGHT_TYPE_SPOT");
    if (light0->type != light1->type)
        return light0->type == 2;
    radiusSq[0] = light0->radius * light0->radius;
    radiusSq[1] = light1->radius * light1->radius;
    Vec3Sub(rg.viewOrg, light0->origin, diff);
    distSq[0] = Vec3LengthSq(diff);
    Vec3Sub(rg.viewOrg, light1->origin, v);
    distSq[1] = Vec3LengthSq(v);
    return radiusSq[1] * distSq[0] <= radiusSq[0] * distSq[1];
}

void __cdecl R_GetBspLightSurfs(const GfxLight **visibleLights, int visibleCount)
{
    GfxBspDrawSurfData surfData[2];

    iassert(visibleCount);
    iassert(rgp.world);

    R_InitBspDrawSurf(&surfData[0]);
    R_InitBspDrawSurf(&surfData[1]);

    for (int lightIndex = 0; lightIndex < visibleCount; ++lightIndex)
    {
        const GfxLight *light = visibleLights[lightIndex];
        iassert(light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT);

        if (light->type == GFX_LIGHT_TYPE_OMNI)
            R_GetBspOmniLightSurfs(light, lightIndex, surfData);
        else
            R_GetBspSpotLightSurfs(light, lightIndex, surfData);
    }
}

BOOL __cdecl R_SortBspShadowReceiverSurfaces(GfxSurface *surface0, GfxSurface *surface1)
{
    return surface0 < surface1;
}

void __cdecl R_GetBspOmniLightSurfs(const GfxLight *light, int lightIndex, GfxBspDrawSurfData *surfData)
{
    unsigned __int16 triSurfList[2]; // [esp+F0h] [ebp-4Ch] BYREF
    unsigned int surfIndex; // [esp+F4h] [ebp-48h]
    float mins[3]; // [esp+F8h] [ebp-44h] BYREF
    BspOmniLightCallback bspLightCallback; // [esp+104h] [ebp-38h] BYREF
    unsigned int visLightDrawSurfCount; // [esp+118h] [ebp-24h] BYREF
    unsigned __int8 *surfaceVisData; // [esp+11Ch] [ebp-20h]
    GfxSurface **surfaces[1]; // [esp+120h] [ebp-1Ch] BYREF
    float maxs[3]; // [esp+124h] [ebp-18h] BYREF
    GfxDrawSurf *drawSurfs; // [esp+130h] [ebp-Ch]
    GfxDrawSurf *surfaceMaterials; // [esp+134h] [ebp-8h]
    unsigned int listSurfIndex; // [esp+138h] [ebp-4h]
    //int savedregs; // [esp+13Ch] [ebp+0h] BYREF

    surfaceVisData = rgp.world->dpvs.surfaceVisData[0];
    surfaceMaterials = rgp.world->dpvs.surfaceMaterials;
    mins[0] = light->origin[0] - light->radius;
    mins[1] = light->origin[1] - light->radius;
    mins[2] = light->origin[2] - light->radius;
    maxs[0] = light->origin[0] + light->radius;
    maxs[1] = light->origin[1] + light->radius;
    maxs[2] = light->origin[2] + light->radius;
    drawSurfs = scene.visLight[lightIndex].drawSurfs;
    surfaces[0] = (GfxSurface **)&drawSurfs[512];
    bspLightCallback.surfaceVisData = surfaceVisData;
    bspLightCallback.position[0] = light->origin[0];
    bspLightCallback.position[1] = light->origin[1];
    bspLightCallback.position[2] = light->origin[2];
    bspLightCallback.radiusSq = light->radius * light->radius;
    R_BoxSurfaces(
        mins,
        maxs,
        allowSurf_0,
        &bspLightCallback,
        surfaces,
        0x400u,
        &visLightDrawSurfCount,
        1u);
    if (visLightDrawSurfCount)
    {
        surfData->drawSurfList.current = drawSurfs;
        surfData->drawSurfList.end = (GfxDrawSurf *)&scene.visLightShadow[lightIndex - 3];
        //std::_Sort<int *, int, bool(__cdecl *)(int, int)>(
        //    (const GfxStaticModelDrawInst **)surfaces[0],
        //    (const GfxStaticModelDrawInst **)&surfaces[0][visLightDrawSurfCount],
        //    (int)(4 * visLightDrawSurfCount) >> 2,
        //    (bool(__cdecl *)(const GfxStaticModelDrawInst *, const GfxStaticModelDrawInst *))R_SortBspShadowReceiverSurfaces);
        std::sort(&surfaces[0][0], &surfaces[0][visLightDrawSurfCount], R_SortBspShadowReceiverSurfaces);
        for (listSurfIndex = 0; listSurfIndex < visLightDrawSurfCount; ++listSurfIndex)
        {
            if (listSurfIndex >= rgp.world->surfaceCount)
                MyAssertHandler(
                    ".\\r_light.cpp",
                    491,
                    0,
                    "listSurfIndex doesn't index rgp.world->surfaceCount\n\t%i not in [0, %i)",
                    listSurfIndex,
                    rgp.world->surfaceCount);
            surfIndex = surfaces[0][listSurfIndex] - rgp.world->dpvs.surfaces;
            triSurfList[0] = surfIndex;
            R_AddBspDrawSurfs(surfaceMaterials[surfIndex], (unsigned __int8 *)triSurfList, 1u, surfData);
        }
        R_EndCmdBuf(&surfData->delayedCmdBuf);
        scene.visLightShadow[lightIndex - 4].drawSurfCount = surfData->drawSurfList.current
            - scene.visLight[lightIndex].drawSurfs;
    }
    else
    {
        scene.visLightShadow[lightIndex - 4].drawSurfCount = 0;
    }
}

int __cdecl R_AllowBspOmniLight(int surfIndex, void *bspLightCallbackAsVoid)
{
    return *(_BYTE *)(*(unsigned int *)bspLightCallbackAsVoid + surfIndex)
        && *((float *)bspLightCallbackAsVoid + 4) >= PointToBoxDistSq(
            (const float *)bspLightCallbackAsVoid + 1,
            rgp.world->dpvs.surfaces[surfIndex].bounds[0],
            rgp.world->dpvs.surfaces[surfIndex].bounds[1]);
}

void __cdecl R_GetBspSpotLightSurfs(const GfxLight *light, int lightIndex, GfxBspDrawSurfData *surfData)
{
    unsigned __int16 triSurfList[2]; // [esp+1B4h] [ebp-ACh] BYREF
    unsigned int surfIndex; // [esp+1B8h] [ebp-A8h]
    float mins[3]; // [esp+1BCh] [ebp-A4h] BYREF
    BspSpotLightCallback bspLightCallback; // [esp+1C8h] [ebp-98h] BYREF
    unsigned __int8 *surfaceVisData; // [esp+230h] [ebp-30h]
    GfxSurface **surfaces[2]; // [esp+234h] [ebp-2Ch] BYREF
    float maxs[3]; // [esp+23Ch] [ebp-24h] BYREF
    GfxDrawSurf *drawSurfs[2]; // [esp+248h] [ebp-18h]
    GfxDrawSurf *surfaceMaterials; // [esp+250h] [ebp-10h]
    unsigned int listSurfIndex; // [esp+254h] [ebp-Ch]
    unsigned int surfCounts[2]; // [esp+258h] [ebp-8h] BYREF

    iassert(lightIndex < MAX_VISIBLE_SHADOWABLE_DLIGHTS);

    surfaceVisData = rgp.world->dpvs.surfaceVisData[0];
    surfaceMaterials = rgp.world->dpvs.surfaceMaterials;

    mins[0] = light->origin[0] - light->radius;
    mins[1] = light->origin[1] - light->radius;
    mins[2] = light->origin[2] - light->radius;

    maxs[0] = light->origin[0] + light->radius;
    maxs[1] = light->origin[1] + light->radius;
    maxs[2] = light->origin[2] + light->radius;

    drawSurfs[0] = scene.visLight[lightIndex].drawSurfs;
    surfaces[0] = (GfxSurface **)&drawSurfs[0][512];
    drawSurfs[1] = scene.visLightShadow[lightIndex].drawSurfs;
    surfaces[1] = (GfxSurface **)&drawSurfs[1][512];
    bspLightCallback.surfaceVisData = surfaceVisData;
    R_CalcSpotLightPlanes(light, bspLightCallback.planes);
    R_BoxSurfaces(mins, maxs, allowSurf_1, &bspLightCallback, surfaces, 0x400u, surfCounts, 2u);
    if (surfCounts[0])
    {
        scene.visLightShadow[lightIndex - 4].drawSurfCount = surfCounts[0];
        surfData->drawSurfList.current = drawSurfs[0];
        surfData->drawSurfList.end = (GfxDrawSurf *)&scene.visLightShadow[lightIndex - 3];
        //std::_Sort<int *, int, bool(__cdecl *)(int, int)>(
        //    (const GfxStaticModelDrawInst **)surfaces[0],
        //    (const GfxStaticModelDrawInst **)&surfaces[0][surfCounts[0]],
        //    (signed int)(4 * surfCounts[0]) >> 2,
        //    (bool(__cdecl *)(const GfxStaticModelDrawInst *, const GfxStaticModelDrawInst *))R_SortBspShadowReceiverSurfaces);
        std::sort(&surfaces[0][0], &surfaces[0][surfCounts[0]], R_SortBspShadowReceiverSurfaces);
        for (listSurfIndex = 0; listSurfIndex < surfCounts[0]; ++listSurfIndex)
        {
            if (listSurfIndex >= rgp.world->surfaceCount)
                MyAssertHandler(
                    ".\\r_light.cpp",
                    557,
                    0,
                    "listSurfIndex doesn't index rgp.world->surfaceCount\n\t%i not in [0, %i)",
                    listSurfIndex,
                    rgp.world->surfaceCount);
            surfIndex = surfaces[0][listSurfIndex] - rgp.world->dpvs.surfaces;
            triSurfList[0] = surfIndex;
            R_AddBspDrawSurfs(surfaceMaterials[surfIndex], (unsigned __int8 *)triSurfList, 1u, surfData);
        }
        R_EndCmdBuf(&surfData->delayedCmdBuf);
        scene.visLightShadow[lightIndex - 4].drawSurfCount = surfData->drawSurfList.current
            - scene.visLight[lightIndex].drawSurfs;
    }
    else
    {
        scene.visLightShadow[lightIndex - 4].drawSurfCount = 0;
    }
    if (surfCounts[1])
    {
        scene.visLightShadow[lightIndex].drawSurfCount = surfCounts[1];
        surfData[1].drawSurfList.current = drawSurfs[1];
        surfData[1].drawSurfList.end = (GfxDrawSurf *)((char *)scene.cookie + 8200 * lightIndex);
        //std::_Sort<int *, int, bool(__cdecl *)(int, int)>(
        //    (const GfxStaticModelDrawInst **)surfaces[1],
        //    (const GfxStaticModelDrawInst **)&surfaces[1][surfCounts[1]],
        //    (signed int)(4 * surfCounts[1]) >> 2,
        //    (bool(__cdecl *)(const GfxStaticModelDrawInst *, const GfxStaticModelDrawInst *))R_SortBspShadowReceiverSurfaces);
        std::sort(&surfaces[1][0], &surfaces[1][surfCounts[1]], R_SortBspShadowReceiverSurfaces);
        for (listSurfIndex = 0; listSurfIndex < surfCounts[1]; ++listSurfIndex)
        {
            if (listSurfIndex >= rgp.world->surfaceCount)
                MyAssertHandler(
                    ".\\r_light.cpp",
                    585,
                    0,
                    "listSurfIndex doesn't index rgp.world->surfaceCount\n\t%i not in [0, %i)",
                    listSurfIndex,
                    rgp.world->surfaceCount);
            surfIndex = surfaces[1][listSurfIndex] - rgp.world->dpvs.surfaces;
            triSurfList[0] = surfIndex;
            R_AddBspDrawSurfs(surfaceMaterials[surfIndex], (unsigned __int8 *)triSurfList, 1u, surfData + 1);
        }
        R_EndCmdBuf(&surfData[1].delayedCmdBuf);
        scene.visLightShadow[lightIndex].drawSurfCount = surfData[1].drawSurfList.current
            - scene.visLightShadow[lightIndex].drawSurfs;
    }
    else
    {
        scene.visLightShadow[lightIndex].drawSurfCount = 0;
    }
}

int __cdecl R_AllowBspSpotLightShadows(int surfIndex, void *bspLightCallbackAsVoid)
{
    if (r_spotLightShadows->current.enabled)
        return R_BoxInPlanes(
            (const float (*)[4])((unsigned int)bspLightCallbackAsVoid + 4),
            rgp.world->dpvs.surfaces[surfIndex].bounds[0],
            rgp.world->dpvs.surfaces[surfIndex].bounds[1]);
    else
        return 0;
}

int __cdecl R_BoxInPlanes(const float (*planes)[4], const float *mins, const float *maxs)
{
    float v4; // [esp+0h] [ebp-4Ch]
    float v5; // [esp+4h] [ebp-48h]
    float v6; // [esp+8h] [ebp-44h]
    float v7; // [esp+Ch] [ebp-40h]
    float v8; // [esp+10h] [ebp-3Ch]
    float v9; // [esp+14h] [ebp-38h]
    float v10; // [esp+18h] [ebp-34h]
    float v11; // [esp+1Ch] [ebp-30h]
    float v12; // [esp+20h] [ebp-2Ch]
    float v13; // [esp+24h] [ebp-28h]
    float v14; // [esp+28h] [ebp-24h]
    float v15; // [esp+2Ch] [ebp-20h]
    float v16; // [esp+30h] [ebp-1Ch]
    float v17; // [esp+34h] [ebp-18h]
    float v18; // [esp+38h] [ebp-14h]
    float v19; // [esp+3Ch] [ebp-10h]
    const float *plane; // [esp+40h] [ebp-Ch]
    float insidef; // [esp+44h] [ebp-8h]
    float inside; // [esp+44h] [ebp-8h]
    float insidea; // [esp+44h] [ebp-8h]
    float insideb; // [esp+44h] [ebp-8h]
    float insidec; // [esp+44h] [ebp-8h]
    float insided; // [esp+44h] [ebp-8h]
    float insidee; // [esp+44h] [ebp-8h]
    float insideg; // [esp+44h] [ebp-8h]
    unsigned int planeIndex; // [esp+48h] [ebp-4h]

    for (planeIndex = 0; planeIndex < 6; ++planeIndex)
    {
        plane = &(*planes)[4 * planeIndex];
        v19 = plane[3] + mins[2] * plane[2] + mins[1] * plane[1] + *mins * *plane;
        if (v19 < 0.0)
            v18 = 1.0;
        else
            v18 = 0.0;
        v17 = plane[3] + maxs[2] * plane[2] + mins[1] * plane[1] + *mins * *plane;
        if (v17 < 0.0)
            v16 = 1.0;
        else
            v16 = 0.0;
        insidef = (float)0.0 + v18;
        inside = insidef + v16;
        v15 = plane[3] + mins[2] * plane[2] + maxs[1] * plane[1] + *mins * *plane;
        if (v15 < 0.0)
            v14 = 1.0;
        else
            v14 = 0.0;
        insidea = inside + v14;
        v13 = plane[3] + maxs[2] * plane[2] + maxs[1] * plane[1] + *mins * *plane;
        if (v13 < 0.0)
            v12 = 1.0;
        else
            v12 = 0.0;
        insideb = insidea + v12;
        v11 = plane[3] + mins[2] * plane[2] + mins[1] * plane[1] + *maxs * *plane;
        if (v11 < 0.0)
            v10 = 1.0;
        else
            v10 = 0.0;
        insidec = insideb + v10;
        v9 = plane[3] + maxs[2] * plane[2] + mins[1] * plane[1] + *maxs * *plane;
        if (v9 < 0.0)
            v8 = 1.0;
        else
            v8 = 0.0;
        insided = insidec + v8;
        v7 = plane[3] + mins[2] * plane[2] + maxs[1] * plane[1] + *maxs * *plane;
        if (v7 < 0.0)
            v6 = 1.0;
        else
            v6 = 0.0;
        insidee = insided + v6;
        v5 = plane[3] + maxs[2] * plane[2] + maxs[1] * plane[1] + *maxs * *plane;
        if (v5 < 0.0)
            v4 = 1.0;
        else
            v4 = 0.0;
        insideg = insidee + v4;
        if (insideg == 0.0)
            return 0;
    }
    return 1;
}

int __cdecl R_AllowBspSpotLight(int surfIndex, void *bspLightCallbackAsVoid)
{
    if (*(_BYTE *)(*(unsigned int *)bspLightCallbackAsVoid + surfIndex))
        return R_BoxInPlanes(
            (const float (*)[4])((unsigned int)bspLightCallbackAsVoid + 4),
            rgp.world->dpvs.surfaces[surfIndex].bounds[0],
            rgp.world->dpvs.surfaces[surfIndex].bounds[1]);
    else
        return 0;
}

void __cdecl R_CalcSpotLightPlanes(const GfxLight *light, float (*planes)[4])
{
    float fCos; // [esp+0h] [ebp-50h]
    float fCosa; // [esp+0h] [ebp-50h]
    float v4; // [esp+4h] [ebp-4Ch]
    float v5; // [esp+4h] [ebp-4Ch]
    float v6; // [esp+4h] [ebp-4Ch]
    float v7; // [esp+4h] [ebp-4Ch]
    float v8; // [esp+14h] [ebp-3Ch]
    float v9; // [esp+18h] [ebp-38h]
    float origin[3]; // [esp+1Ch] [ebp-34h] BYREF
    float crossDirs[2][3]; // [esp+28h] [ebp-28h] BYREF
    float lightDirection[3]; // [esp+40h] [ebp-10h] BYREF
    float fSin; // [esp+4Ch] [ebp-4h]

    Vec3Scale(light->dir, -1.0, lightDirection);
    Vec3Scale(lightDirection, -1.0, (float *)planes);
    Vec3Mad(light->origin, scene.dynamicSpotLightNearPlaneOffset, lightDirection, origin);
    (*planes)[3] = -Vec3Dot((const float *)planes, origin);
    R_ComputeSpotLightCrossDirs(light, crossDirs);
    v9 = 1.0 - light->cosHalfFovOuter * light->cosHalfFovOuter;
    v8 = sqrt(v9);
    fSin = v8;
    v4 = -v8;
    R_CalcPlaneFromCosSinPointDirs(&(*planes)[4], light->cosHalfFovOuter, v4, light->origin, lightDirection, crossDirs[0]);
    v5 = -fSin;
    R_CalcPlaneFromCosSinPointDirs(&(*planes)[8], light->cosHalfFovOuter, v5, light->origin, lightDirection, crossDirs[1]);
    v6 = -fSin;
    fCos = -light->cosHalfFovOuter;
    R_CalcPlaneFromCosSinPointDirs(&(*planes)[12], fCos, v6, light->origin, lightDirection, crossDirs[0]);
    v7 = -fSin;
    fCosa = -light->cosHalfFovOuter;
    R_CalcPlaneFromCosSinPointDirs(&(*planes)[16], fCosa, v7, light->origin, lightDirection, crossDirs[1]);
    Vec3Mad(light->origin, light->radius, lightDirection, origin);
    R_CalcPlaneFromPointDir(&(*planes)[20], origin, lightDirection);
}

void __cdecl R_CalcPlaneFromPointDir(float *plane, const float *origin, const float *dir)
{
    *plane = *dir;
    plane[1] = dir[1];
    plane[2] = dir[2];
    plane[3] = -Vec3Dot(origin, dir);
}

void __cdecl R_ComputeSpotLightCrossDirs(const GfxLight *light, float (*crossDirs)[3])
{
    float v2; // [esp+0h] [ebp-34h]
    float v3; // [esp+4h] [ebp-30h]
    int axisIndex; // [esp+2Ch] [ebp-8h]
    int bestCrossAxis; // [esp+30h] [ebp-4h]

    bestCrossAxis = 0;
    for (axisIndex = 1; axisIndex < 3; ++axisIndex)
    {
        v3 = I_fabs(light->dir[axisIndex]);
        v2 = I_fabs(light->dir[bestCrossAxis]);
        if (v2 > (double)v3)
            bestCrossAxis = axisIndex;
    }
    (*crossDirs)[0] = 0.0;
    (*crossDirs)[1] = 0.0;
    (*crossDirs)[2] = 0.0;
    (*crossDirs)[bestCrossAxis] = 1.0;
    Vec3Cross(light->dir, (const float *)crossDirs, &(*crossDirs)[3]);
    Vec3Normalize(&(*crossDirs)[3]);
    Vec3Cross(&(*crossDirs)[3], light->dir, (float *)crossDirs);
    Vec3Normalize((float *)crossDirs);
}

void __cdecl R_CalcPlaneFromCosSinPointDirs(
    float *plane,
    float fCos,
    float fSin,
    const float *origin,
    const float *forward,
    const float *lateral)
{
    Vec3ScaleMad(fCos, lateral, fSin, forward, plane);
    plane[3] = -Vec3Dot(plane, origin);
}

void __cdecl R_GetStaticModelLightSurfs(const GfxLight **visibleLights, int visibleCount)
{
    const GfxStaticModelDrawInst* smodelDrawInst; // [esp+18h] [ebp-89Ch]
    GfxDrawSurf drawSurf; // [esp+1Ch] [ebp-898h]
    float mins[3]; // [esp+28h] [ebp-88Ch] BYREF
    unsigned int surfaceIndex; // [esp+34h] [ebp-880h]
    const Material* material; // [esp+38h] [ebp-87Ch]
    GfxBspDrawSurfData shadowSurfData; // [esp+3Ch] [ebp-878h] BYREF
    unsigned int surfaceCount; // [esp+58h] [ebp-85Ch]
    const GfxLight* light; // [esp+5Ch] [ebp-858h]
    GfxStaticModelId staticModelId; // [esp+60h] [ebp-854h]
    Material* const* pMaterial; // [esp+64h] [ebp-850h]
    unsigned __int16 list[2]; // [esp+68h] [ebp-84Ch] BYREF
    unsigned int* lodData; // [esp+6Ch] [ebp-848h]
    int lod; // [esp+70h] [ebp-844h]
    unsigned __int16 smodels[1024]; // [esp+74h] [ebp-840h] BYREF
    float maxs[3]; // [esp+878h] [ebp-3Ch] BYREF
    int smodelCount; // [esp+884h] [ebp-30h]
    XSurface* surfaces; // [esp+888h] [ebp-2Ch] BYREF
    int index; // [esp+88Ch] [ebp-28h]
    int lightIndex; // [esp+890h] [ebp-24h]
    GfxBspDrawSurfData surfData; // [esp+894h] [ebp-20h] BYREF
    int smodelIndex; // [esp+8B0h] [ebp-4h]
    //int savedregs; // [esp+8B4h] [ebp+0h] BYREF

    iassert( visibleCount );
    R_InitBspDrawSurf(&surfData);
    R_InitBspDrawSurf(&shadowSurfData);
    g_staticModelLightCallback.smodelVisData = rgp.world->dpvs.smodelVisData[0];
    lodData = rgp.world->dpvs.lodData;
    for (lightIndex = 0; lightIndex < visibleCount; ++lightIndex)
    {
        light = visibleLights[lightIndex];
        if (light->type != 3 && light->type != 2)
            MyAssertHandler(
                ".\\r_light.cpp",
                720,
                1,
                "%s",
                "light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT");
        mins[0] = light->origin[0] - light->radius;
        mins[1] = light->origin[1] - light->radius;
        mins[2] = light->origin[2] - light->radius;
        maxs[0] = light->origin[0] + light->radius;
        maxs[1] = light->origin[1] + light->radius;
        maxs[2] = light->origin[2] + light->radius;
        surfData.drawSurfList.current = &scene.visLight[lightIndex].drawSurfs[scene.visLightShadow[lightIndex - 4].drawSurfCount];
        surfData.drawSurfList.end = (GfxDrawSurf*)&scene.visLightShadow[lightIndex - 3];
        if (light->type == 3)
        {
            g_staticModelLightCallback.position[0] = light->origin[0];
            g_staticModelLightCallback.position[1] = light->origin[1];
            g_staticModelLightCallback.position[2] = light->origin[2];
            g_staticModelLightCallback.radiusSq = light->radius * light->radius;
            smodelCount = R_BoxStaticModels(mins, maxs, R_AllowStaticModelOmniLight, smodels, 1024);
        }
        else
        {
            shadowSurfData.drawSurfList.current = &scene.visLightShadow[lightIndex].drawSurfs[scene.visLightShadow[lightIndex].drawSurfCount];
            shadowSurfData.drawSurfList.end = (GfxDrawSurf*)((char*)scene.cookie + 8200 * lightIndex);
            R_CalcSpotLightPlanes(light, g_staticModelLightCallback.planes);
            smodelCount = R_BoxStaticModels(mins, maxs, R_AllowStaticModelSpotLight, smodels, 1024);
        }
        for (index = 0; index < smodelCount; ++index)
        {
            smodelIndex = smodels[index];
            smodelDrawInst = &rgp.world->dpvs.smodelDrawInsts[smodelIndex];
            lod = (lodData[smodelIndex >> 4] >> (2 * (smodelIndex & 0xF))) & 3;
            surfaceCount = XModelGetSurfaces(smodelDrawInst->model, &surfaces, lod);
            iassert( surfaceCount );
            staticModelId = R_GetStaticModelId(smodelIndex, lod);
            pMaterial = XModelGetSkins(smodelDrawInst->model, lod);
            iassert( pMaterial );
            list[0] = staticModelId.objectId;
            surfaceIndex = 0;
            while (surfaceIndex < surfaceCount)
            {
                material = *pMaterial;
                iassert( material );
                if (rgp.sortedMaterials[material->info.drawSurf.fields.materialSortedIndex] != material)
                    MyAssertHandler(
                        ".\\r_light.cpp",
                        776,
                        0,
                        "%s",
                        "rgp.sortedMaterials[material->info.drawSurf.fields.materialSortedIndex] == material");
                if (Material_GetTechnique(material, TECHNIQUE_LIGHT_OMNI))
                {
                    drawSurf = material->info.drawSurf;
                    //HIDWORD(drawSurf.packed) = ((staticModelId.surfType & 0xF) << 18) | HIDWORD(drawSurf.packed) & 0xFFC3FFFF;
                    drawSurf.fields.surfType = staticModelId.surfType;
                    if (!R_AllocDrawSurf(&surfData.delayedCmdBuf, drawSurf, &surfData.drawSurfList, 3u))
                        break;
                    R_AddDelayedStaticModelDrawSurf(&surfData.delayedCmdBuf, &surfaces[surfaceIndex], (unsigned __int8*)list, 1u);
                    if (light->type == 2 && r_spotLightShadows->current.enabled && r_spotLightSModelShadows->current.enabled)
                    {
                        if (!R_AllocDrawSurf(&shadowSurfData.delayedCmdBuf, drawSurf, &shadowSurfData.drawSurfList, 3u))
                            break;
                        R_AddDelayedStaticModelDrawSurf(
                            &shadowSurfData.delayedCmdBuf,
                            &surfaces[surfaceIndex],
                            (unsigned __int8*)list,
                            1u);
                    }
                }
                ++surfaceIndex;
                ++pMaterial;
            }
        }
        R_EndCmdBuf(&surfData.delayedCmdBuf);
        scene.visLightShadow[lightIndex - 4].drawSurfCount = surfData.drawSurfList.current
            - scene.visLight[lightIndex].drawSurfs;
        R_EndCmdBuf(&shadowSurfData.delayedCmdBuf);
        scene.visLightShadow[lightIndex].drawSurfCount = shadowSurfData.drawSurfList.current
            - scene.visLightShadow[lightIndex].drawSurfs;
    }
}

int __cdecl R_AllowStaticModelOmniLight(int smodelIndex)
{
    return g_staticModelLightCallback.smodelVisData[smodelIndex]
        && g_staticModelLightCallback.radiusSq >= PointToBoxDistSq(
            g_staticModelLightCallback.position,
            rgp.world->dpvs.smodelInsts[smodelIndex].mins,
            rgp.world->dpvs.smodelInsts[smodelIndex].maxs);
}

int __cdecl R_AllowStaticModelSpotLight(int smodelIndex)
{
    if (g_staticModelLightCallback.smodelVisData[smodelIndex])
        return R_BoxInPlanes(
            g_staticModelLightCallback.planes,
            rgp.world->dpvs.smodelInsts[smodelIndex].mins,
            rgp.world->dpvs.smodelInsts[smodelIndex].maxs);
    else
        return 0;
}

void __cdecl R_GetSceneEntLightSurfs(const GfxLight **visibleLights, int visibleCount)
{
    float v2; // [esp+4h] [ebp-208h]
    float v3; // [esp+8h] [ebp-204h]
    float v4; // [esp+Ch] [ebp-200h]
    float v[3]; // [esp+10h] [ebp-1FCh] BYREF
    float diff[3]; // [esp+1Ch] [ebp-1F0h] BYREF
    GfxSceneDynBrush *sceneDynBrush; // [esp+28h] [ebp-1E4h]
    DynEntityPose *dynEntPose; // [esp+2Ch] [ebp-1E0h]
    GfxVisibleLight *visLightShadow; // [esp+30h] [ebp-1DCh]
    GfxSceneModel *sceneModel; // [esp+34h] [ebp-1D8h]
    GfxDrawSurf *drawSurf; // [esp+38h] [ebp-1D4h]
    unsigned int dynEntIndex; // [esp+3Ch] [ebp-1D0h]
    GfxDrawSurf *newDrawSurf; // [esp+40h] [ebp-1CCh]
    float planes[4][6][4]; // [esp+44h] [ebp-1C8h] BYREF
    unsigned int sceneEntCount; // [esp+1C8h] [ebp-44h]
    const GfxLight *light; // [esp+1CCh] [ebp-40h]
    unsigned int visLightDrawSurfCount; // [esp+1D0h] [ebp-3Ch]
    float radius; // [esp+1D4h] [ebp-38h]
    GfxVisibleLight *visLight; // [esp+1D8h] [ebp-34h]
    const DynEntityDef *dynEntDef; // [esp+1DCh] [ebp-30h]
    const float *bounds; // [esp+1E0h] [ebp-2Ch]
    unsigned int visLightShadowDrawSurfCount; // [esp+1E4h] [ebp-28h]
    unsigned int sceneEntIndex; // [esp+1E8h] [ebp-24h]
    GfxSceneEntity *sceneEnt; // [esp+1ECh] [ebp-20h]
    GfxSceneBrush *sceneBrush; // [esp+1F0h] [ebp-1Ch]
    unsigned __int8 *sceneEntVisData; // [esp+1F4h] [ebp-18h]
    int lightIndex; // [esp+1F8h] [ebp-14h]
    GfxSceneDynModel *sceneDynModel; // [esp+1FCh] [ebp-10h]
    float distSq; // [esp+200h] [ebp-Ch]
    const GfxBrushModel *bmodel; // [esp+204h] [ebp-8h]
    GfxDrawSurf *lastDrawSurf; // [esp+208h] [ebp-4h]

    iassert( (visibleCount <= 4) );
    for (lightIndex = 0; lightIndex < visibleCount; ++lightIndex)
    {
        light = visibleLights[lightIndex];
        if (light->type == 2)
            R_CalcSpotLightPlanes(light, planes[lightIndex]);
    }
    sceneEntCount = scene.sceneDObjCount;
    sceneEntVisData = scene.sceneDObjVisData[0];
    for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
    {
        if (sceneEntVisData[sceneEntIndex] == 1)
        {
            sceneEnt = &scene.sceneDObj[sceneEntIndex];
            if (sceneEnt->cull.state < 2)
                MyAssertHandler(
                    ".\\r_light.cpp",
                    895,
                    0,
                    "sceneEnt->cull.state >= CULL_STATE_BOUNDED\n\t%i, %i",
                    sceneEnt->cull.state,
                    2);
            bounds = sceneEnt->cull.mins;
            lightIndex = 0;
        LABEL_16:
            if (lightIndex >= visibleCount)
                continue;
            light = visibleLights[lightIndex];
            if (light->type != 3 && light->type != 2)
                MyAssertHandler(
                    ".\\r_light.cpp",
                    902,
                    1,
                    "%s",
                    "light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT");
            if (light->type == 3)
            {
                distSq = PointToBoxDistSq(light->origin, bounds, bounds + 3);
                v4 = light->radius * light->radius;
                if (distSq <= (double)v4)
                    goto LABEL_29;
            }
            else if (!R_SpotLightIsAttachedToDobj(sceneEnt->obj))
            {
                iassert( light->type == GFX_LIGHT_TYPE_SPOT );
                if (R_BoxInPlanes(planes[lightIndex], bounds, bounds + 3)
                    && (frontEndDataOut->gfxEnts[sceneEnt->gfxEntIndex].renderFxFlags & 8) == 0)
                {
                LABEL_29:
                    visLight = &scene.visLightShadow[lightIndex - 4];
                    lastDrawSurf = (GfxDrawSurf *)&visLight[1];
                    visLightDrawSurfCount = visLight->drawSurfCount;
                    drawSurf = &visLight->drawSurfs[visLightDrawSurfCount];
                    newDrawSurf = R_AddDObjSurfaces(sceneEnt, TECHNIQUE_LIGHT_OMNI, drawSurf, (GfxDrawSurf *)&visLight[1]);
                    visLight->drawSurfCount += newDrawSurf - drawSurf;
                    if (light->type == 2
                        && r_spotLightShadows->current.enabled
                        && r_spotLightEntityShadows->current.enabled
                        && (frontEndDataOut->gfxEnts[sceneEnt->gfxEntIndex].renderFxFlags & 1) == 0)
                    {
                        if (lightIndex)
                            MyAssertHandler(
                                ".\\r_light.cpp",
                                938,
                                0,
                                "lightIndex doesn't index MAX_VISIBLE_SHADOWABLE_DLIGHTS\n\t%i not in [0, %i)",
                                lightIndex,
                                1);
                        visLightShadow = &scene.visLightShadow[lightIndex];
                        lastDrawSurf = (GfxDrawSurf *)&visLightShadow[1];
                        visLightShadowDrawSurfCount = visLightShadow->drawSurfCount;
                        drawSurf = &visLightShadow->drawSurfs[visLightShadowDrawSurfCount];
                        newDrawSurf = R_AddDObjSurfaces(sceneEnt, TECHNIQUE_LIGHT_OMNI, drawSurf, (GfxDrawSurf *)&visLightShadow[1]);
                        visLightShadow->drawSurfCount += newDrawSurf - drawSurf;
                    }
                }
            }
            ++lightIndex;
            goto LABEL_16;
        }
    }
    sceneEntCount = scene.sceneModelCount;
    sceneEntVisData = scene.sceneModelVisData[0];
    for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
    {
        if (sceneEntVisData[sceneEntIndex] == 1)
        {
            sceneModel = &scene.sceneModel[sceneEntIndex];
            for (lightIndex = 0; ; ++lightIndex)
            {
                if (lightIndex >= visibleCount)
                    goto LABEL_39;
                light = visibleLights[lightIndex];
                if (light->type != 3 && light->type != 2)
                    MyAssertHandler(
                        ".\\r_light.cpp",
                        963,
                        1,
                        "%s",
                        "light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT");
                if (light->type == 3)
                    break;
                iassert( light->type == GFX_LIGHT_TYPE_SPOT );
                if (R_SphereInPlanes(planes[lightIndex], sceneModel->placement.base.origin, sceneModel->radius))
                    goto LABEL_55;
            LABEL_43:
                ;
            }
            Vec3Sub(sceneModel->placement.base.origin, light->origin, diff);
            distSq = Vec3LengthSq(diff);
            radius = light->radius + sceneModel->radius;
            if (distSq > radius * radius)
                goto LABEL_43;
        LABEL_55:
            visLight = &scene.visLightShadow[lightIndex - 4];
            lastDrawSurf = (GfxDrawSurf *)&visLight[1];
            visLightDrawSurfCount = visLight->drawSurfCount;
            drawSurf = &visLight->drawSurfs[visLightDrawSurfCount];
            newDrawSurf = R_AddXModelSurfaces(
                &sceneModel->info,
                sceneModel->model,
                TECHNIQUE_LIGHT_OMNI,
                drawSurf,
                (GfxDrawSurf *)&visLight[1]);
            visLight->drawSurfCount += newDrawSurf - drawSurf;
            if (light->type == 2 && r_spotLightShadows->current.enabled && r_spotLightEntityShadows->current.enabled)
            {
                if (lightIndex)
                    MyAssertHandler(
                        ".\\r_light.cpp",
                        989,
                        0,
                        "lightIndex doesn't index MAX_VISIBLE_SHADOWABLE_DLIGHTS\n\t%i not in [0, %i)",
                        lightIndex,
                        1);
                visLightShadow = &scene.visLightShadow[lightIndex];
                lastDrawSurf = (GfxDrawSurf *)&visLightShadow[1];
                visLightShadowDrawSurfCount = visLightShadow->drawSurfCount;
                drawSurf = &visLightShadow->drawSurfs[visLightShadowDrawSurfCount];
                newDrawSurf = R_AddXModelSurfaces(
                    &sceneModel->info,
                    sceneModel->model,
                    TECHNIQUE_LIGHT_OMNI,
                    drawSurf,
                    (GfxDrawSurf *)&visLightShadow[1]);
                visLightShadow->drawSurfCount += newDrawSurf - drawSurf;
            }
            goto LABEL_43;
        }
    LABEL_39:
        ;
    }
    sceneEntCount = scene.sceneDynModelCount;
    sceneEntVisData = rgp.world->dpvsDyn.dynEntVisData[0][0];
    for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
    {
        sceneDynModel = &rgp.world->sceneDynModel[sceneEntIndex];
        dynEntIndex = sceneDynModel->dynEntId;
        if (sceneEntVisData[dynEntIndex] == 1)
        {
            dynEntPose = DynEnt_GetClientPose(dynEntIndex, DYNENT_DRAW_MODEL);
            for (lightIndex = 0; ; ++lightIndex)
            {
                if (lightIndex >= visibleCount)
                    goto LABEL_64;
                light = visibleLights[lightIndex];
                if (light->type != 3 && light->type != 2)
                    MyAssertHandler(
                        ".\\r_light.cpp",
                        1018,
                        1,
                        "%s",
                        "light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT");
                if (light->type == 3)
                    break;
                iassert( light->type == GFX_LIGHT_TYPE_SPOT );
                if (R_SphereInPlanes(planes[lightIndex], dynEntPose->pose.origin, dynEntPose->radius))
                    goto LABEL_80;
            LABEL_68:
                ;
            }
            Vec3Sub(dynEntPose->pose.origin, light->origin, v);
            distSq = Vec3LengthSq(v);
            radius = light->radius + dynEntPose->radius;
            if (distSq > radius * radius)
                goto LABEL_68;
        LABEL_80:
            visLight = &scene.visLightShadow[lightIndex - 4];
            lastDrawSurf = (GfxDrawSurf *)&visLight[1];
            visLightDrawSurfCount = visLight->drawSurfCount;
            drawSurf = &visLight->drawSurfs[visLightDrawSurfCount];
            dynEntDef = DynEnt_GetEntityDef(dynEntIndex, DYNENT_DRAW_MODEL);
            newDrawSurf = R_AddXModelSurfaces(
                &sceneDynModel->info,
                dynEntDef->xModel,
                TECHNIQUE_LIGHT_OMNI,
                drawSurf,
                lastDrawSurf);
            visLight->drawSurfCount += newDrawSurf - drawSurf;
            if (light->type == 2 && r_spotLightShadows->current.enabled && r_spotLightEntityShadows->current.enabled)
            {
                if (lightIndex)
                    MyAssertHandler(
                        ".\\r_light.cpp",
                        1045,
                        0,
                        "lightIndex doesn't index MAX_VISIBLE_SHADOWABLE_DLIGHTS\n\t%i not in [0, %i)",
                        lightIndex,
                        1);
                visLightShadow = &scene.visLightShadow[lightIndex];
                lastDrawSurf = (GfxDrawSurf *)&visLightShadow[1];
                visLightShadowDrawSurfCount = visLightShadow->drawSurfCount;
                drawSurf = &visLightShadow->drawSurfs[visLightShadowDrawSurfCount];
                dynEntDef = DynEnt_GetEntityDef(dynEntIndex, DYNENT_DRAW_MODEL);
                newDrawSurf = R_AddXModelSurfaces(
                    &sceneDynModel->info,
                    dynEntDef->xModel,
                    TECHNIQUE_LIGHT_OMNI,
                    drawSurf,
                    lastDrawSurf);
                visLightShadow->drawSurfCount += newDrawSurf - drawSurf;
            }
            goto LABEL_68;
        }
    LABEL_64:
        ;
    }
    sceneEntCount = scene.sceneBrushCount;
    sceneEntVisData = scene.sceneBrushVisData[0];
    for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
    {
        if (sceneEntVisData[sceneEntIndex] == 1)
        {
            sceneBrush = &scene.sceneBrush[sceneEntIndex];
            bmodel = sceneBrush->bmodel;
            for (lightIndex = 0; ; ++lightIndex)
            {
                if (lightIndex >= visibleCount)
                    goto LABEL_89;
                light = visibleLights[lightIndex];
                if (light->type != 3 && light->type != 2)
                    MyAssertHandler(
                        ".\\r_light.cpp",
                        1074,
                        1,
                        "%s",
                        "light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT");
                if (light->type == 3)
                    break;
                iassert( light->type == GFX_LIGHT_TYPE_SPOT );
                if (R_BoxInPlanes(planes[lightIndex], bmodel->writable.mins, bmodel->writable.maxs))
                    goto LABEL_105;
            LABEL_93:
                ;
            }
            distSq = PointToBoxDistSq(light->origin, bmodel->writable.mins, bmodel->writable.maxs);
            v3 = light->radius * light->radius;
            if (distSq > (double)v3)
                goto LABEL_93;
        LABEL_105:
            visLight = &scene.visLightShadow[lightIndex - 4];
            lastDrawSurf = (GfxDrawSurf *)&visLight[1];
            visLightDrawSurfCount = visLight->drawSurfCount;
            drawSurf = &visLight->drawSurfs[visLightDrawSurfCount];
            newDrawSurf = R_AddBModelSurfaces(
                &sceneBrush->info,
                sceneBrush->bmodel,
                TECHNIQUE_LIGHT_OMNI,
                drawSurf,
                (GfxDrawSurf *)&visLight[1]);
            visLight->drawSurfCount += newDrawSurf - drawSurf;
            if (light->type == 2 && r_spotLightShadows->current.enabled && r_spotLightEntityShadows->current.enabled)
            {
                if (lightIndex)
                    MyAssertHandler(
                        ".\\r_light.cpp",
                        1099,
                        0,
                        "lightIndex doesn't index MAX_VISIBLE_SHADOWABLE_DLIGHTS\n\t%i not in [0, %i)",
                        lightIndex,
                        1);
                visLightShadow = &scene.visLightShadow[lightIndex];
                lastDrawSurf = (GfxDrawSurf *)&visLightShadow[1];
                visLightShadowDrawSurfCount = visLightShadow->drawSurfCount;
                drawSurf = &visLightShadow->drawSurfs[visLightShadowDrawSurfCount];
                newDrawSurf = R_AddBModelSurfaces(
                    &sceneBrush->info,
                    sceneBrush->bmodel,
                    TECHNIQUE_LIGHT_OMNI,
                    drawSurf,
                    (GfxDrawSurf *)&visLightShadow[1]);
                visLightShadow->drawSurfCount += newDrawSurf - drawSurf;
            }
            goto LABEL_93;
        }
    LABEL_89:
        ;
    }
    sceneEntCount = scene.sceneDynBrushCount;
    sceneEntVisData = rgp.world->dpvsDyn.dynEntVisData[1][0];
    for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
    {
        sceneDynBrush = &rgp.world->sceneDynBrush[sceneEntIndex];
        dynEntIndex = sceneDynBrush->dynEntId;
        if (sceneEntVisData[dynEntIndex] == 1)
        {
            dynEntDef = DynEnt_GetEntityDef(dynEntIndex, DYNENT_DRAW_BRUSH);
            bmodel = R_GetBrushModel(dynEntDef->brushModel);
            for (lightIndex = 0; ; ++lightIndex)
            {
                if (lightIndex >= visibleCount)
                    goto LABEL_114;
                light = visibleLights[lightIndex];
                if (light->type != 3 && light->type != 2)
                    MyAssertHandler(
                        ".\\r_light.cpp",
                        1129,
                        1,
                        "%s",
                        "light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT");
                if (light->type == 3)
                    break;
                iassert( light->type == GFX_LIGHT_TYPE_SPOT );
                if (R_BoxInPlanes(planes[lightIndex], bmodel->writable.mins, bmodel->writable.maxs))
                    goto LABEL_130;
            LABEL_118:
                ;
            }
            distSq = PointToBoxDistSq(light->origin, bmodel->writable.mins, bmodel->writable.maxs);
            v2 = light->radius * light->radius;
            if (distSq > (double)v2)
                goto LABEL_118;
        LABEL_130:
            visLight = &scene.visLightShadow[lightIndex - 4];
            lastDrawSurf = (GfxDrawSurf *)&visLight[1];
            visLightDrawSurfCount = visLight->drawSurfCount;
            drawSurf = &visLight->drawSurfs[visLightDrawSurfCount];
            newDrawSurf = R_AddBModelSurfaces(
                (BModelDrawInfo *)sceneDynBrush,
                bmodel,
                TECHNIQUE_LIGHT_OMNI,
                drawSurf,
                (GfxDrawSurf *)&visLight[1]);
            visLight->drawSurfCount += newDrawSurf - drawSurf;
            if (light->type == 2 && r_spotLightShadows->current.enabled && r_spotLightEntityShadows->current.enabled)
            {
                if (lightIndex)
                    MyAssertHandler(
                        ".\\r_light.cpp",
                        1155,
                        0,
                        "lightIndex doesn't index MAX_VISIBLE_SHADOWABLE_DLIGHTS\n\t%i not in [0, %i)",
                        lightIndex,
                        1);
                visLightShadow = &scene.visLightShadow[lightIndex];
                lastDrawSurf = (GfxDrawSurf *)&visLightShadow[1];
                visLightShadowDrawSurfCount = visLightShadow->drawSurfCount;
                drawSurf = &visLightShadow->drawSurfs[visLightShadowDrawSurfCount];
                newDrawSurf = R_AddBModelSurfaces(
                    (BModelDrawInfo *)sceneDynBrush,
                    bmodel,
                    TECHNIQUE_LIGHT_OMNI,
                    drawSurf,
                    (GfxDrawSurf *)&visLightShadow[1]);
                visLightShadow->drawSurfCount += newDrawSurf - drawSurf;
            }
            goto LABEL_118;
        }
    LABEL_114:
        ;
    }
}

int __cdecl R_SphereInPlanes(const float (*planes)[4], const float *center, float radius)
{
    unsigned int planeIndex; // [esp+4h] [ebp-4h]

    for (planeIndex = 0; planeIndex < 6; ++planeIndex)
    {
        if (Vec3Dot(&(*planes)[4 * planeIndex], center) + (float)(*planes)[4 * planeIndex + 3] - radius > 0.0)
            return 0;
    }
    return 1;
}

bool __cdecl R_SpotLightIsAttachedToDobj(const DObj_s *obj)
{
    FxSystem *system; // [esp+0h] [ebp-8h]
    DObj_s *attachedDobj; // [esp+4h] [ebp-4h]

    iassert( obj );
    system = FX_GetSystem(0);
    iassert( system );
    if (!system->activeSpotLightEffectCount)
        return 0;
    if (system->activeSpotLightBoltDobj == -1)
        return 0;
    attachedDobj = Com_GetClientDObj(system->activeSpotLightBoltDobj, 0);
    return attachedDobj && attachedDobj == obj;
}

// KISAKTODO: unfk
void __cdecl ShortSortArray_GfxReverseSortDrawSurfsInterface_GfxDrawSurf_(GfxDrawSurf *lo, GfxDrawSurf *hi)
{
    int packed_high; // edx
    unsigned __int64 v3; // [esp+4h] [ebp-34h]
    unsigned __int64 packed; // [esp+Ch] [ebp-2Ch]
    GfxDrawSurf *max; // [esp+1Ch] [ebp-1Ch]
    unsigned __int64 maxKey; // [esp+20h] [ebp-18h]
    GfxDrawSurf *walk; // [esp+34h] [ebp-4h]

    while (hi > lo)
    {
        max = lo;
        LODWORD(maxKey) = LODWORD(lo->fields);
        HIDWORD(maxKey) = ((~((lo->packed >> 54) & 0x3F) & 0x3F) << 22) | HIDWORD(lo->packed) & 0xF03FFFFF;
        for (walk = lo + 1; walk <= hi; ++walk)
        {
            packed = walk->packed;
            HIDWORD(packed) = ((~((walk->packed >> 54) & 0x3F) & 0x3F) << 22) | HIDWORD(walk->packed) & 0xF03FFFFF;
            if (maxKey < packed)
            {
                LODWORD(maxKey) = LODWORD(walk->fields);
                HIDWORD(maxKey) = ((~((walk->packed >> 54) & 0x3F) & 0x3F) << 22) | HIDWORD(walk->packed) & 0xF03FFFFF;
                max = walk;
            }
        }
        v3 = max->packed;
        packed_high = HIDWORD(hi->packed);
        //*&max->fields = hi->fields;
        LODWORD(max->packed) = LODWORD(hi->packed);
        HIDWORD(max->packed) = packed_high;
        hi->packed = v3;
        --hi;
    }
}

// KISAKTODO: unfk
void __cdecl qsortArray_GfxReverseSortDrawSurfsInterface_GfxDrawSurf_(GfxDrawSurf *elems, int count)
{
    int packed_high; // edx
    GfxDrawSurf *v3; // eax
    int v4; // eax
    int v5; // ecx
    GfxDrawSurf *v6; // edx
    GfxDrawSurf v7; // [esp+4h] [ebp-180h]
    unsigned __int64 fields; // [esp+Ch] [ebp-178h]
    int v9; // [esp+10h] [ebp-174h]
    GfxDrawSurf v10; // [esp+14h] [ebp-170h]
    GfxDrawSurf v11; // [esp+1Ch] [ebp-168h]
    GfxDrawSurf v12; // [esp+2Ch] [ebp-158h]
    unsigned __int64 pivotKey; // [esp+64h] [ebp-120h]
    GfxDrawSurf *loWalk; // [esp+74h] [ebp-110h]
    int sortCount; // [esp+78h] [ebp-10Ch]
    GfxDrawSurf *hiEnd; // [esp+7Ch] [ebp-108h]
    GfxDrawSurf *hiWalk; // [esp+80h] [ebp-104h]
    GfxDrawSurf *loStack[30]; // [esp+84h] [ebp-100h]
    GfxDrawSurf *hiStack[30]; // [esp+FCh] [ebp-88h]
    int stackPos; // [esp+178h] [ebp-Ch]
    GfxDrawSurf *loEnd; // [esp+17Ch] [ebp-8h]
    GfxDrawSurf *mid; // [esp+180h] [ebp-4h]

    if (count >= 2)
    {
        stackPos = 0;
        loEnd = elems;
        hiEnd = &elems[count - 1];
        while (1)
        {
            while (1)
            {
                sortCount = hiEnd - loEnd + 1;
                if (sortCount <= 8)
                {
//                    ShortSortArray<GfxReverseSortDrawSurfsInterface, GfxDrawSurf>(loEnd, hiEnd);
                    ShortSortArray_GfxReverseSortDrawSurfsInterface_GfxDrawSurf_(loEnd, hiEnd);
                    goto LABEL_22;
                }
                mid = &loEnd[sortCount / 2];
                v12.fields = mid->fields;
                packed_high = HIDWORD(loEnd->packed);
                v3 = mid;
                *&mid->fields = loEnd->fields;
                HIDWORD(v3->packed) = packed_high;
                loEnd->fields = v12.fields;
                loWalk = loEnd;
                hiWalk = hiEnd + 1;
                LODWORD(pivotKey) = loEnd->packed;
                HIDWORD(pivotKey) = ((~((loEnd->packed >> 54) & 0x3F) & 0x3F) << 22) | HIDWORD(loEnd->packed) & 0xF03FFFFF;
                while (1)
                {
                    do
                    {
                        if (++loWalk > hiEnd)
                            break;
                        v11.fields = loWalk->fields;
                        HIDWORD(v11.packed) = ((~((loWalk->packed >> 54) & 0x3F) & 0x3F) << 22)
                            | HIDWORD(loWalk->packed) & 0xF03FFFFF;
                    } while (v11.packed <= pivotKey);
                    do
                    {
                        if (loEnd >= --hiWalk)
                            break;
                        v10.fields = hiWalk->fields;
                        HIDWORD(v10.packed) = ((~((hiWalk->packed >> 54) & 0x3F) & 0x3F) << 22)
                            | HIDWORD(hiWalk->packed) & 0xF03FFFFF;
                    } while (pivotKey <= v10.packed);
                    if (hiWalk < loWalk)
                        break;
                    fields = loWalk->packed;
                    v9 = HIDWORD(loWalk->packed);
                    v4 = HIDWORD(hiWalk->packed);
                    *&loWalk->fields = hiWalk->fields;
                    HIDWORD(loWalk->packed) = v4;
                    *&hiWalk->packed = fields;
                    HIDWORD(hiWalk->packed) = v9;
                }
                v7.fields = loEnd->fields;
                v5 = HIDWORD(hiWalk->packed);
                v6 = loEnd;
                *&loEnd->fields = hiWalk->fields;
                HIDWORD(v6->packed) = v5;
                hiWalk->fields = v7.fields;
                if (&hiWalk[-1] - loEnd >= hiEnd - loWalk)
                    break;
                if (loWalk < hiEnd)
                {
                    loStack[stackPos] = loWalk;
                    hiStack[stackPos++] = hiEnd;
                }
                if (loEnd >= &hiWalk[-1])
                {
                LABEL_22:
                    if (--stackPos < 0)
                        return;
                    loEnd = loStack[stackPos];
                    hiEnd = hiStack[stackPos];
                }
                else
                {
                    hiEnd = hiWalk - 1;
                }
            }
            if (loEnd <= hiWalk)
            {
                loStack[stackPos] = loEnd;
                hiStack[stackPos++] = hiWalk - 1;
            }
            if (loWalk >= hiEnd)
                goto LABEL_22;
            loEnd = loWalk;
        }
    }
}


void __cdecl R_ReverseSortDrawSurfs(GfxDrawSurf *drawSurfList, int surfCount)
{
    PROF_SCOPED("R_SortDrawSurfs");
    //qsortArray<GfxReverseSortDrawSurfsInterface, GfxDrawSurf>(drawSurfList, surfCount);
    qsortArray_GfxReverseSortDrawSurfsInterface_GfxDrawSurf_(drawSurfList, surfCount);
}

int __cdecl R_EmitPointLightPartitionSurfs(
    GfxViewInfo *viewInfo,
    const GfxLight **visibleLights,
    int visibleCount,
    const float *viewOrigin)
{
    int firstDrawSurf; // [esp+Ch] [ebp-28h]
    PointLightPartition *partitions; // [esp+10h] [ebp-24h]
    const GfxLight *light; // [esp+18h] [ebp-1Ch]
    int partitionCount; // [esp+20h] [ebp-14h]
    unsigned int lightDrawSurfCount; // [esp+24h] [ebp-10h]
    int lightIndex; // [esp+28h] [ebp-Ch]
    PointLightPartition *partition; // [esp+2Ch] [ebp-8h]
    int drawSurfCount; // [esp+30h] [ebp-4h]

    partitions = viewInfo->pointLightPartitions;
    partitionCount = 0;
    for (lightIndex = 0; lightIndex < visibleCount; ++lightIndex)
    {
        light = visibleLights[lightIndex];
        lightDrawSurfCount = scene.visLightShadow[lightIndex - 4].drawSurfCount;
        R_ReverseSortDrawSurfs(scene.visLight[lightIndex].drawSurfs, lightDrawSurfCount);
        partition = &partitions[partitionCount];
        R_InitDrawSurfListInfo(&partition->info);
        partition->info.baseTechType = (MaterialTechniqueType)R_GetTechniqueForLightType(light, viewInfo);
        partition->info.viewInfo = viewInfo;
        partition->info.viewOrigin[0] = *viewOrigin;
        partition->info.viewOrigin[1] = viewOrigin[1];
        partition->info.viewOrigin[2] = viewOrigin[2];
        partition->info.viewOrigin[3] = viewOrigin[3];
        partition->info.light = &partition->light;
        partition->info.cameraView = 1;
        firstDrawSurf = frontEndDataOut->drawSurfCount;
        R_EmitDrawSurfList(scene.visLight[lightIndex].drawSurfs, lightDrawSurfCount);
        drawSurfCount = frontEndDataOut->drawSurfCount - firstDrawSurf;
        if (drawSurfCount)
        {
            memcpy(partition, light, 0x40u);
            partition->info.drawSurfs = &frontEndDataOut->drawSurfs[firstDrawSurf];
            partitions[partitionCount++].info.drawSurfCount = drawSurfCount;
        }
    }
    return partitionCount;
}

int __cdecl R_GetTechniqueForLightType(const GfxLight *light, const GfxViewInfo *viewInfo)
{
    const char *v3; // eax

    iassert( viewInfo );
    if (light->type == 2)
    {
        if (!r_spotLightShadows->current.enabled)
            return 21;
        iassert( comWorld.isInUse );
        if (Com_BitCheckAssert(frontEndDataOut->shadowableLightHasShadowMap, comWorld.primaryLightCount, 32))
        {
            iassert( viewInfo->emissiveSpotLightCount == 1 );
            iassert( comWorld.isInUse );
            if (comWorld.primaryLightCount + 1 != viewInfo->shadowableLightCount)
                MyAssertHandler(
                    ".\\r_light.cpp",
                    1182,
                    0,
                    "%s",
                    "Com_GetPrimaryLightCount() + GFX_MAX_EMISSIVE_SPOT_LIGHTS == viewInfo->shadowableLightCount");
            return 23;
        }
        else
        {
            return 21;
        }
    }
    else if (light->type == 3)
    {
        return 22;
    }
    else
    {
        if (!alwaysfails)
        {
            v3 = va("Dynamic light type %d isn't supported.", light->type);
            MyAssertHandler(".\\r_light.cpp", 1195, 0, v3);
        }
        return 22;
    }
}

void __cdecl R_EmitShadowedLightPartitionSurfs(
    GfxViewInfo *viewInfo,
    unsigned int lightDrawSurfCount,
    GfxDrawSurf *lightDrawSurfs,
    GfxDrawSurfListInfo *info)
{
    int firstDrawSurf; // [esp+0h] [ebp-8h]
    unsigned int drawSurfCount; // [esp+4h] [ebp-4h]

    firstDrawSurf = frontEndDataOut->drawSurfCount;
    R_EmitDrawSurfList(lightDrawSurfs, lightDrawSurfCount);
    drawSurfCount = frontEndDataOut->drawSurfCount - firstDrawSurf;
    info->drawSurfs = &frontEndDataOut->drawSurfs[firstDrawSurf];
    info->drawSurfCount = drawSurfCount;
}

