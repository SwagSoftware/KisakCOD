#include "r_bsp.h"
#include <cgame_mp/cg_local_mp.h>
#include "r_init.h"
#include "r_light.h"
#include "r_primarylights.h"
#include <DynEntity/DynEntity_client.h>
#include <qcommon/com_bsp.h>
#include "r_dvars.h"
#include <universal/q_parse.h>
#include "r_image.h"

r_globals_load_t rgl;

void __cdecl R_InterpretSunLightParseParamsIntoLights(SunLightParseParams *sunParse, GfxLight *sunLight)
{
    float scale; // [esp+8h] [ebp-20h]
    float sunColor[3]; // [esp+Ch] [ebp-1Ch] BYREF
    float sunScale; // [esp+18h] [ebp-10h]
    float sunDirection[3]; // [esp+1Ch] [ebp-Ch] BYREF

    AngleVectors(sunParse->angles, sunDirection, 0, 0);
    sunScale = sunParse->sunLight - sunParse->ambientScale;
    scale = (1.0 - sunParse->diffuseFraction) * sunScale;
    Vec3Scale(sunParse->sunColor, scale, sunColor);
    if (sunLight)
        R_SetUpSunLight(sunColor, sunDirection, sunLight);
}

void __cdecl R_SetUpSunLight(const float *sunColor, const float *sunDirection, GfxLight *light)
{
    if (!light)
        MyAssertHandler(".\\r_bsp_load_obj.cpp", 4559, 0, "%s", "light");
    memset(&light->type, 0, sizeof(GfxLight));
    light->type = 1;
    light->dir[0] = *sunDirection;
    light->dir[1] = sunDirection[1];
    light->dir[2] = sunDirection[2];
    light->color[0] = *sunColor;
    light->color[1] = sunColor[1];
    light->color[2] = sunColor[2];
}

void __cdecl R_InitPrimaryLights(GfxLight *primaryLights)
{
    GfxLight *out; // [esp+20h] [ebp-Ch]
    const ComPrimaryLight *in; // [esp+24h] [ebp-8h]
    unsigned int lightIndex; // [esp+28h] [ebp-4h]

    if (!rgp.world)
        MyAssertHandler(".\\r_bsp_load_obj.cpp", 4614, 0, "%s", "rgp.world");
    for (lightIndex = 0; lightIndex < rgp.world->primaryLightCount; ++lightIndex)
    {
        in = Com_GetPrimaryLight(lightIndex);
        out = &primaryLights[lightIndex];
        out->type = in->type;
        out->canUseShadowMap = in->canUseShadowMap;
        out->color[0] = in->color[0];
        out->color[1] = in->color[1];
        out->color[2] = in->color[2];
        out->dir[0] = in->dir[0];
        out->dir[1] = in->dir[1];
        out->dir[2] = in->dir[2];
        out->origin[0] = in->origin[0];
        out->origin[1] = in->origin[1];
        out->origin[2] = in->origin[2];
        out->radius = in->radius;
        out->cosHalfFovOuter = in->cosHalfFovOuter;
        out->cosHalfFovInner = in->cosHalfFovInner;
        out->exponent = in->exponent;
        if (in->defName)
            out->def = R_RegisterLightDef(in->defName);
        else
            out->def = 0;
    }
    if (rgp.world->sunPrimaryLightIndex)
        memcpy(&primaryLights[rgp.world->sunPrimaryLightIndex], rgp.world->sunLight, sizeof(GfxLight));
}

void __cdecl R_AddShadowSurfaceToPrimaryLight(
    GfxWorld *world,
    unsigned int primaryLightIndex,
    unsigned int sortedSurfIndex)
{
    GfxShadowGeometry *shadowGeom; // [esp+0h] [ebp-4h]

    shadowGeom = &world->shadowGeom[primaryLightIndex];
    if (shadowGeom->sortedSurfIndex)
    {
        if (sortedSurfIndex != (unsigned __int16)sortedSurfIndex)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                281,
                0,
                "i == static_cast< Type >( i )\n\t%i, %i",
                sortedSurfIndex,
                (unsigned __int16)sortedSurfIndex);
        shadowGeom->sortedSurfIndex[shadowGeom->surfaceCount++] = sortedSurfIndex;
    }
}

void __cdecl R_ForEachPrimaryLightAffectingSurface(
    GfxWorld *world,
    const GfxSurface *surface,
    unsigned int sortedSurfIndex,
    void(__cdecl *Callback)(GfxWorld *, unsigned int, unsigned int))
{
    char v4; // [esp+3h] [ebp-35h]
    GfxLightRegion *v5; // [esp+4h] [ebp-34h]
    unsigned int i; // [esp+8h] [ebp-30h]
    float diff[3]; // [esp+Ch] [ebp-2Ch] BYREF
    unsigned int primaryLightIndex; // [esp+18h] [ebp-20h]
    const ComPrimaryLight *light; // [esp+1Ch] [ebp-1Ch]
    float boxHalfSize[3]; // [esp+20h] [ebp-18h] BYREF
    float boxMidPoint[3]; // [esp+2Ch] [ebp-Ch] BYREF

    if ((surface->material->info.gameFlags & 2) != 0)
    {
        Callback(world, surface->primaryLightIndex, sortedSurfIndex);
    }
    else
    {
        Vec3Avg(surface->bounds[0], surface->bounds[1], boxMidPoint);
        Vec3Sub(boxMidPoint, surface->bounds[0], boxHalfSize);
        boxHalfSize[0] = boxHalfSize[0] + 1.0;
        boxHalfSize[1] = boxHalfSize[1] + 1.0;
        boxHalfSize[2] = boxHalfSize[2] + 1.0;
        for (primaryLightIndex = world->sunPrimaryLightIndex + 1;
            primaryLightIndex < world->primaryLightCount;
            ++primaryLightIndex)
        {
            light = Com_GetPrimaryLight(primaryLightIndex);
            if (!Com_CullBoxFromPrimaryLight(light, boxMidPoint, boxHalfSize))
            {
                v5 = &s_world.lightRegion[primaryLightIndex];
                if (v5->hullCount)
                {
                    Vec3Sub(boxMidPoint, light->origin, diff);
                    for (i = 0; i < v5->hullCount; ++i)
                    {
                        if (!R_CullBoxFromLightRegionHull(&v5->hulls[i], diff, boxHalfSize))
                        {
                            v4 = 0;
                            goto LABEL_15;
                        }
                    }
                    v4 = 1;
                }
                else
                {
                    v4 = 0;
                }
            LABEL_15:
                if (!v4)
                    Callback(world, primaryLightIndex, sortedSurfIndex);
            }
        }
    }
}

BOOL __cdecl R_ChooseTrisContextType()
{
    if (Com_GetBspVersion() <= 0x12)
        return 0;
    if (Com_BspHasLump(LUMP_UNLAYERED_TRIANGLES))
        return !r_useLayeredMaterials->current.enabled;
    return 0;
}

void __cdecl R_LoadStep(const char *description)
{
    Com_Printf(8, "Loading %s...\n", description);
    SCR_UpdateLoadScreen();
}

void __cdecl R_LoadMaterials(GfxBspLoad *load)
{
    load->diskMaterials = (const dmaterial_t*)Com_GetBspLump(LUMP_MATERIALS, 0x48u, &load->materialCount);
}

char *__cdecl R_ParseSunLight(SunLightParseParams *params, char *text)
{
    parseInfo_t *src; // [esp+1Ch] [ebp-100Ch]
    parseInfo_t *srca; // [esp+1Ch] [ebp-100Ch]
    char dest[2048]; // [esp+20h] [ebp-1008h] BYREF
    char nptr[2052]; // [esp+820h] [ebp-808h] BYREF

    if (!params)
        MyAssertHandler(".\\r_bsp_load_obj.cpp", 3097, 0, "%s", "params");
    while (1)
    {
        src = Com_Parse(&text);
        if (!src->token[0] || src->token[0] == 125)
            break;
        if (src->token[0] == 123)
        {
            params->ambientScale = 0.0;
            params->sunLight = 0.0;
            params->diffuseFraction = 0.5;
            params->diffuseColorHasBeenSet = 0;
            params->ambientColor[0] = 0.0;
            params->ambientColor[1] = 0.0;
            params->ambientColor[2] = 0.0;
            params->sunColor[0] = 0.0;
            params->sunColor[1] = 0.0;
            params->sunColor[2] = 0.0;
            params->diffuseColor[0] = 0.0;
            params->diffuseColor[1] = 0.0;
            params->diffuseColor[2] = 0.0;
        }
        else
        {
            I_strncpyz(dest, src->token, 2048);
            srca = Com_Parse(&text);
            if (!srca->token[0] || srca->token[0] == 125)
                return text;
            I_strncpyz(nptr, srca->token, 2048);
            if (I_stricmp(dest, "ambient"))
            {
                if (I_stricmp(dest, "_color"))
                {
                    if (I_stricmp(dest, "diffuseFraction"))
                    {
                        if (I_stricmp(dest, "suncolor"))
                        {
                            if (I_stricmp(dest, "sundiffusecolor"))
                            {
                                if (I_stricmp(dest, "sunlight"))
                                {
                                    if (I_stricmp(dest, "sundirection"))
                                    {
                                        if (!I_stricmp(dest, "name"))
                                            I_strncpyz(params->name, nptr, 64);
                                    }
                                    else
                                    {
                                        params->angles[0] = 0.0;
                                        params->angles[1] = 0.0;
                                        params->angles[2] = 0.0;
                                        sscanf(nptr, "%f %f %f", params->angles, &params->angles[1], &params->angles[2]);
                                    }
                                }
                                else
                                {
                                    params->sunLight = atof(nptr);
                                }
                            }
                            else
                            {
                                params->diffuseColor[0] = 0.0;
                                params->diffuseColor[1] = 0.0;
                                params->diffuseColor[2] = 0.0;
                                sscanf(nptr, "%f %f %f", params->diffuseColor, &params->diffuseColor[1], &params->diffuseColor[2]);
                                ColorNormalize(params->diffuseColor, params->diffuseColor);
                                params->diffuseColorHasBeenSet = 1;
                            }
                        }
                        else
                        {
                            params->sunColor[0] = 0.0;
                            params->sunColor[1] = 0.0;
                            params->sunColor[2] = 0.0;
                            sscanf(nptr, "%f %f %f", params->sunColor, &params->sunColor[1], &params->sunColor[2]);
                            ColorNormalize(params->sunColor, params->sunColor);
                        }
                    }
                    else
                    {
                        params->diffuseFraction = atof(nptr);
                    }
                }
                else
                {
                    params->ambientColor[0] = 0.0;
                    params->ambientColor[1] = 0.0;
                    params->ambientColor[2] = 0.0;
                    sscanf(nptr, "%f %f %f", params->ambientColor, &params->ambientColor[1], &params->ambientColor[2]);
                }
            }
            else
            {
                params->ambientScale = atof(nptr);
                if (params->ambientScale > 2.0)
                {
                    Com_PrintWarning(
                        8,
                        "WARNING: ambient too big, assuming it uses the old 0-255 scale instead of the proper 0-1 scale (value = '%s')\n",
                        nptr);
                    params->ambientScale = params->ambientScale * 0.01568627543747425;
                }
            }
        }
    }
    return text;
}

void R_LoadSunSettings()
{
    unsigned int size; // [esp+0h] [ebp-8h] BYREF
    const char *text; // [esp+4h] [ebp-4h]

    text = Com_GetBspLump(LUMP_ENTITIES, 1u, &size);
    R_ParseSunLight(&s_world.sunParse, (char*)text);
    s_world.sunLight = (GfxLight*)Hunk_Alloc(0x40u, "R_LoadSunSettings", 20);
    R_InterpretSunLightParseParamsIntoLights(&s_world.sunParse, s_world.sunLight);
}

void __cdecl R_LoadPrimaryLights(unsigned int bspVersion)
{
    const ComPrimaryLight *primaryLight; // [esp+0h] [ebp-8h]
    unsigned int lightIndex; // [esp+4h] [ebp-4h]

    if (bspVersion > 0xE)
    {
        if (!comWorld.isInUse)
            MyAssertHandler("c:\\trees\\cod3\\src\\gfx_d3d\\../qcommon/com_bsp_api.h", 23, 0, "%s", "comWorld.isInUse");
        s_world.primaryLightCount = comWorld.primaryLightCount;
        s_world.sunPrimaryLightIndex = comWorld.primaryLightCount > 1 && Com_GetPrimaryLight(1u)->type == 1;
        for (lightIndex = 0; lightIndex < s_world.primaryLightCount; ++lightIndex)
        {
            primaryLight = Com_GetPrimaryLight(lightIndex);
            if (primaryLight->defName)
                R_RegisterLightDef(primaryLight->defName);
        }
    }
    else
    {
        s_world.sunPrimaryLightIndex = 1;
        s_world.primaryLightCount = 2;
    }
}

void R_LoadLightRegions()
{
    GfxLightRegionAxis *v0; // [esp+8h] [ebp-34h]
    char *diskHulls; // [esp+Ch] [ebp-30h]
    unsigned int hullIter; // [esp+10h] [ebp-2Ch]
    unsigned __int8 *diskAxes; // [esp+14h] [ebp-28h]
    unsigned int usedAxisCount; // [esp+18h] [ebp-24h]
    unsigned __int8 *axes; // [esp+1Ch] [ebp-20h]
    unsigned int hullCount; // [esp+20h] [ebp-1Ch] BYREF
    unsigned int axisCount; // [esp+24h] [ebp-18h] BYREF
    unsigned int regionCount; // [esp+28h] [ebp-14h] BYREF
    unsigned int usedHullCount; // [esp+2Ch] [ebp-10h]
    GfxLightRegionHull *hulls; // [esp+30h] [ebp-Ch]
    const DiskLightRegion *diskRegions; // [esp+34h] [ebp-8h]
    unsigned int regionIter; // [esp+38h] [ebp-4h]

    s_world.lightRegion = (GfxLightRegion*)Hunk_Alloc(8 * s_world.primaryLightCount, "R_LoadLightRegions", 20);
    diskRegions = (DiskLightRegion*)Com_GetBspLump(LUMP_LIGHTREGIONS, 1u, &regionCount);
    s_world.lightGrid.hasLightRegions = diskRegions != 0;
    if (diskRegions)
    {
        diskHulls = Com_GetBspLump(LUMP_LIGHTREGION_HULLS, 0x4Cu, &hullCount);
        diskAxes = (unsigned char*)Com_GetBspLump(LUMP_LIGHTREGION_AXES, 0x14u, &axisCount);
        hulls = (GfxLightRegionHull*)Hunk_Alloc(80 * hullCount, "R_LoadLightRegionHulls", 20);
        axes = Hunk_Alloc(20 * axisCount, "R_LoadLightRegionAxes", 20);
        if (regionCount != s_world.primaryLightCount)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                3270,
                0,
                "regionCount == s_world.primaryLightCount\n\t%i, %i",
                regionCount,
                s_world.primaryLightCount);
        usedHullCount = 0;
        for (regionIter = 0; regionIter < regionCount; ++regionIter)
        {
            s_world.lightRegion[regionIter].hullCount = diskRegions[regionIter].hullCount;
            if (s_world.lightRegion[regionIter].hulls)
                MyAssertHandler(".\\r_bsp_load_obj.cpp", 3275, 1, "%s", "s_world.lightRegion[regionIter].hulls == NULL");
            if (s_world.lightRegion[regionIter].hullCount)
            {
                s_world.lightRegion[regionIter].hulls = &hulls[usedHullCount];
                usedHullCount += diskRegions[regionIter].hullCount;
            }
        }
        if (usedHullCount != hullCount)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                3282,
                0,
                "usedHullCount == hullCount\n\t%i, %i",
                usedHullCount,
                hullCount);
        usedAxisCount = 0;
        for (hullIter = 0; hullIter < hullCount; ++hullIter)
        {
            qmemcpy(&hulls[hullIter], &diskHulls[76 * hullIter], 0x24u);
            qmemcpy(hulls[hullIter].kdopHalfSize, &diskHulls[76 * hullIter + 36], 0x28u);
            if (hulls[hullIter].axisCount)
                v0 = (GfxLightRegionAxis*)&axes[20 * usedAxisCount];
            else
                v0 = 0;
            hulls[hullIter].axis = v0;
            usedAxisCount += *&diskHulls[76 * hullIter + 72];
        }
        if (usedAxisCount != axisCount)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                3295,
                0,
                "usedAxisCount == axisCount\n\t%i, %i",
                usedAxisCount,
                axisCount);
        memcpy(axes, diskAxes, 20 * axisCount);
    }
}

DiskTriangleSoup *__cdecl R_UpdateDiskSurfaces_Version8(const DiskTriangleSoup_Version8 *oldSurfs, int surfCount)
{
    int surfIndex; // [esp+4h] [ebp-8h]
    DiskTriangleSoup *newSurfs; // [esp+8h] [ebp-4h]

    newSurfs = (DiskTriangleSoup*)Hunk_AllocateTempMemory(24 * surfCount, "R_UpdateDiskSurfaces");
    for (surfIndex = 0; surfIndex < surfCount; ++surfIndex)
    {
        newSurfs[surfIndex].materialIndex = oldSurfs[surfIndex].materialIndex;
        newSurfs[surfIndex].lightmapIndex = oldSurfs[surfIndex].lightmapIndex;
        newSurfs[surfIndex].reflectionProbeIndex = oldSurfs[surfIndex].reflectionProbeIndex;
        newSurfs[surfIndex].primaryLightIndex = 1;
        newSurfs[surfIndex].vertexLayerData = 0;
        newSurfs[surfIndex].firstVertex = oldSurfs[surfIndex].firstVertex;
        newSurfs[surfIndex].vertexCount = oldSurfs[surfIndex].vertexCount;
        newSurfs[surfIndex].indexCount = oldSurfs[surfIndex].indexCount;
        newSurfs[surfIndex].firstIndex = oldSurfs[surfIndex].firstIndex;
    }
    return newSurfs;
}

DiskTriangleSoup *__cdecl R_UpdateDiskSurfaces_Version12(
    const DiskTriangleSoup_Version12 *oldSurfs,
    int surfCount)
{
    int surfIndex; // [esp+4h] [ebp-8h]
    DiskTriangleSoup *newSurfs; // [esp+8h] [ebp-4h]

    newSurfs = (DiskTriangleSoup * )Hunk_AllocateTempMemory(24 * surfCount, "R_UpdateDiskSurfaces");
    for (surfIndex = 0; surfIndex < surfCount; ++surfIndex)
    {
        newSurfs[surfIndex].materialIndex = oldSurfs[surfIndex].materialIndex;
        newSurfs[surfIndex].lightmapIndex = oldSurfs[surfIndex].lightmapIndex;
        newSurfs[surfIndex].reflectionProbeIndex = oldSurfs[surfIndex].reflectionProbeIndex;
        newSurfs[surfIndex].primaryLightIndex = 1;
        newSurfs[surfIndex].vertexLayerData = oldSurfs[surfIndex].vertexLayerData;
        newSurfs[surfIndex].firstVertex = oldSurfs[surfIndex].firstVertex;
        newSurfs[surfIndex].vertexCount = oldSurfs[surfIndex].vertexCount;
        newSurfs[surfIndex].indexCount = oldSurfs[surfIndex].indexCount;
        newSurfs[surfIndex].firstIndex = oldSurfs[surfIndex].firstIndex;
    }
    return newSurfs;
}

DiskTriangleSoup *__cdecl R_UpdateDiskSurfaces_Version14(DiskTriangleSoup *oldSurfs, int surfCount)
{
    int surfIndex; // [esp+0h] [ebp-8h]
    DiskTriangleSoup *newSurfs; // [esp+4h] [ebp-4h]

    newSurfs = (DiskTriangleSoup * )Hunk_AllocateTempMemory(24 * surfCount, "R_UpdateDiskSurfaces");
    memcpy(newSurfs, oldSurfs, 24 * surfCount);
    for (surfIndex = 0; surfIndex < surfCount; ++surfIndex)
    {
        if (newSurfs[surfIndex].primaryLightIndex == 255)
            newSurfs[surfIndex].primaryLightIndex = 0;
        else
            ++newSurfs[surfIndex].primaryLightIndex;
    }
    return newSurfs;
}

void __cdecl R_LoadTriangleSurfaces(
    unsigned int bspVersion,
    TrisType trisType,
    DiskTriangleSoup **diskSurfaces,
    unsigned int *surfCount)
{
    const DiskTriangleSoup *BspLump; // eax

    if (bspVersion > 8)
    {
        if (bspVersion > 0xC)
        {
            if (bspVersion > 0xE)
            {
                BspLump = (DiskTriangleSoup*)Com_GetBspLump(
                    (trisType != TRIS_TYPE_LAYERED ? LUMP_UNLAYERED_TRIANGLES : LUMP_TRIANGLES),
                    0x18u,
                    surfCount);
            }
            else
            {
                *diskSurfaces = (DiskTriangleSoup *)Com_GetBspLump(LUMP_TRIANGLES, 0x18u, surfCount);
                BspLump = R_UpdateDiskSurfaces_Version14(*diskSurfaces, *surfCount);
            }
            *diskSurfaces = (DiskTriangleSoup *)BspLump;
        }
        else
        {
            *diskSurfaces = (DiskTriangleSoup *)Com_GetBspLump(LUMP_TRIANGLES, 0x14u, surfCount);
            *diskSurfaces = R_UpdateDiskSurfaces_Version12((const DiskTriangleSoup_Version12 * )*diskSurfaces, *surfCount);
        }
    }
    else
    {
        *diskSurfaces = (DiskTriangleSoup *)Com_GetBspLump(LUMP_TRIANGLES, 0x10u, surfCount);
        *diskSurfaces = R_UpdateDiskSurfaces_Version8((const DiskTriangleSoup_Version8 * )*diskSurfaces, *surfCount);
    }
}

void __cdecl R_UnloadTriangleSurfaces(unsigned int bspVersion, DiskTriangleSoup *diskSurfaces)
{
    if (bspVersion <= 0xE)
        Hunk_FreeTempMemory((char*)diskSurfaces);
}

unsigned int __cdecl R_DetermineLightmapCoupling(GfxBspLoad *load, int (*coupling)[31])
{
    unsigned int otherLmapIndex; // [esp+4h] [ebp-A4h]
    unsigned int lmapIndex; // [esp+8h] [ebp-A0h]
    unsigned int lmapIndexa; // [esp+8h] [ebp-A0h]
    unsigned int lmapIndexb; // [esp+8h] [ebp-A0h]
    unsigned int materialIndex; // [esp+Ch] [ebp-9Ch]
    int lmapVertCount[31]; // [esp+10h] [ebp-98h] BYREF
    unsigned int origLmapCount; // [esp+94h] [ebp-14h]
    DiskTriangleSoup *triSurfs; // [esp+98h] [ebp-10h] BYREF
    unsigned int triSurfCount; // [esp+9Ch] [ebp-Ch] BYREF
    unsigned int diskLmapCount; // [esp+A0h] [ebp-8h] BYREF
    unsigned int triSurfIndex; // [esp+A4h] [ebp-4h]

    if (!load)
        MyAssertHandler(".\\r_bsp_load_obj.cpp", 474, 0, "%s", "load");
    if (!coupling)
        MyAssertHandler(".\\r_bsp_load_obj.cpp", 475, 0, "%s", "coupling");
    R_LoadTriangleSurfaces(load->bspVersion, load->trisType, &triSurfs, &triSurfCount);
    if (load->bspVersion >= 7)
        Com_GetBspLump(LUMP_LIGHTBYTES, 3145728u, &diskLmapCount);
    else
        diskLmapCount = 0;
    origLmapCount = 0;
    for (triSurfIndex = 0; triSurfIndex < triSurfCount; ++triSurfIndex)
    {
        lmapIndex = triSurfs[triSurfIndex].lightmapIndex;
        if (lmapIndex != 31 && origLmapCount <= lmapIndex)
            origLmapCount = lmapIndex + 1;
    }
    if (diskLmapCount && diskLmapCount != origLmapCount)
        Com_Error(ERR_DROP, "LoadMap: funny lump size in %s", s_world.name);
    memset(lmapVertCount, 0, sizeof(lmapVertCount));
    memset(coupling, 0, 3844u);
    for (materialIndex = 0; materialIndex < load->materialCount; ++materialIndex)
    {
        for (triSurfIndex = 0; triSurfIndex < triSurfCount; ++triSurfIndex)
        {
            if (triSurfs[triSurfIndex].materialIndex == materialIndex)
            {
                lmapIndexa = triSurfs[triSurfIndex].lightmapIndex;
                if (lmapIndexa != 31)
                    lmapVertCount[lmapIndexa] += triSurfs[triSurfIndex].vertexCount;
            }
        }
        for (lmapIndexb = 0; lmapIndexb < origLmapCount; ++lmapIndexb)
        {
            if (lmapVertCount[lmapIndexb])
            {
                for (otherLmapIndex = lmapIndexb + 1; otherLmapIndex < origLmapCount; ++otherLmapIndex)
                {
                    if (lmapVertCount[otherLmapIndex])
                    {
                        (*coupling)[31 * lmapIndexb + otherLmapIndex] += lmapVertCount[otherLmapIndex] + lmapVertCount[lmapIndexb];
                        if ((*coupling)[31 * lmapIndexb + otherLmapIndex] < 0)
                            (*coupling)[31 * lmapIndexb + otherLmapIndex] = 0x7FFFFFFF;
                        (*coupling)[31 * otherLmapIndex + lmapIndexb] = (*coupling)[31 * lmapIndexb + otherLmapIndex];
                    }
                }
                lmapVertCount[lmapIndexb] = 0;
            }
        }
    }
    if (triSurfCount)
        R_UnloadTriangleSurfaces(load->bspVersion, triSurfs);
    return origLmapCount;
}

int __cdecl R_BuildLightmapMergability(GfxBspLoad *load, r_lightmapGroup_t *groupInfo, int *reorder)
{
    int otherLmapIndex; // [esp+Ch] [ebp-F58h]
    int otherLmapIndexa; // [esp+Ch] [ebp-F58h]
    bool used[32]; // [esp+10h] [ebp-F54h] BYREF
    int coupling[31][31]; // [esp+34h] [ebp-F30h] BYREF
    int usedCount; // [esp+F3Ch] [ebp-28h]
    int highCount; // [esp+F40h] [ebp-24h]
    int maxTextureSize; // [esp+F44h] [ebp-20h]
    int lmapIndex; // [esp+F48h] [ebp-1Ch]
    int origLmapCount; // [esp+F4Ch] [ebp-18h]
    int mergedCount; // [esp+F50h] [ebp-14h]
    int newLmapCount; // [esp+F54h] [ebp-10h]
    int bestLmapIndex; // [esp+F58h] [ebp-Ch]
    int bestOtherLmapIndex; // [esp+F5Ch] [ebp-8h]
    int wideCount; // [esp+F60h] [ebp-4h]

    origLmapCount = R_DetermineLightmapCoupling(load, coupling);
    memset(used, 0, 31);
    newLmapCount = 0;
    maxTextureSize = 2048;
    wideCount = 2;
    highCount = 2;
    usedCount = 0;
    while (usedCount < origLmapCount)
    {
        while (highCount * wideCount > origLmapCount - usedCount)
        {
            if (wideCount < highCount)
                highCount >>= 1;
            else
                wideCount >>= 1;
        }
        if (highCount * wideCount < 2)
        {
            mergedCount = 1;
            for (lmapIndex = 0; lmapIndex < origLmapCount; ++lmapIndex)
            {
                if (!used[lmapIndex])
                {
                    reorder[usedCount++] = lmapIndex;
                    used[lmapIndex] = 1;
                    break;
                }
            }
        }
        else
        {
            bestLmapIndex = 31;
            bestOtherLmapIndex = 31;
            for (lmapIndex = 0; lmapIndex < origLmapCount; ++lmapIndex)
            {
                if (!used[lmapIndex])
                {
                    for (otherLmapIndex = lmapIndex + 1; otherLmapIndex < origLmapCount; ++otherLmapIndex)
                    {
                        if (!used[otherLmapIndex]
                            && (bestLmapIndex == 31
                                || coupling[lmapIndex][otherLmapIndex] > coupling[bestLmapIndex][bestOtherLmapIndex]))
                        {
                            bestLmapIndex = lmapIndex;
                            bestOtherLmapIndex = otherLmapIndex;
                        }
                    }
                }
            }
            reorder[usedCount++] = bestOtherLmapIndex;
            reorder[usedCount++] = bestLmapIndex;
            used[bestOtherLmapIndex] = 1;
            used[bestLmapIndex] = 1;
            for (mergedCount = 2; mergedCount < highCount * wideCount; ++mergedCount)
            {
                for (lmapIndex = 0; lmapIndex < origLmapCount; ++lmapIndex)
                {
                    coupling[bestLmapIndex][lmapIndex] += coupling[bestOtherLmapIndex][lmapIndex];
                    coupling[lmapIndex][bestLmapIndex] = coupling[bestLmapIndex][lmapIndex];
                }
                bestOtherLmapIndex = 31;
                for (otherLmapIndexa = 0; otherLmapIndexa < origLmapCount; ++otherLmapIndexa)
                {
                    if (!used[otherLmapIndexa]
                        && (bestOtherLmapIndex == 31
                            || coupling[bestLmapIndex][otherLmapIndexa] > coupling[bestLmapIndex][bestOtherLmapIndex]))
                    {
                        bestOtherLmapIndex = otherLmapIndexa;
                    }
                }
                reorder[usedCount++] = bestOtherLmapIndex;
                used[bestOtherLmapIndex] = 1;
            }
        }
        groupInfo[newLmapCount].wideCount = wideCount;
        groupInfo[newLmapCount++].highCount = highCount;
    }
    Com_Printf(8, "%i merged lightmaps from %i original lightmaps\n", newLmapCount, origLmapCount);
    return origLmapCount;
}

void __cdecl R_CopyLightmap(
    char *srcImage,
    int srcWidth,
    int srcHeight,
    int bytesPerPixel,
    unsigned __int8 *dstImage,
    int tileX,
    int tileY,
    int tilesWide)
{
    int y; // [esp+0h] [ebp-4h]
    unsigned __int8 *dstImagea; // [esp+1Ch] [ebp+18h]

    dstImagea = &dstImage[bytesPerPixel * (srcWidth * tilesWide * srcHeight * tileY + srcWidth * tileX)];
    for (y = 0; y < srcHeight; ++y)
    {
        Com_Memcpy(dstImagea, srcImage, bytesPerPixel * srcWidth);
        srcImage += bytesPerPixel * srcWidth;
        dstImagea += tilesWide * bytesPerPixel * srcWidth;
    }
}

void __cdecl R_CopyLightDefAttenuationImage(XAssetHeader header, _DWORD *anonymousConfig)
{
    int endCount; // [esp+30h] [ebp-7Ch]
    unsigned __int8 *dstPixel; // [esp+38h] [ebp-74h]
    unsigned __int8 *dstPixela; // [esp+38h] [ebp-74h]
    unsigned int lerp; // [esp+3Ch] [ebp-70h]
    GfxRawPixel *srcPixel; // [esp+40h] [ebp-6Ch]
    GfxRawImage rawImage; // [esp+44h] [ebp-68h] BYREF
    GfxRawPixel lerpedPixel; // [esp+A4h] [ebp-8h]
    int iter; // [esp+A8h] [ebp-4h]

    Image_GetRawPixels(*(header.xmodelPieces->numpieces + 32), &rawImage);
    if (rawImage.width != *(header.xmodelPieces->numpieces + 24))
        MyAssertHandler(".\\r_light_load_obj.cpp", 144, 0, "%s", "rawImage.width == def->attenuation.image->width");
    dstPixel = (*anonymousConfig + anonymousConfig[1] * (4 * header.xmodelPieces[1].name - 4));
    srcPixel = rawImage.pixels;
    if (anonymousConfig[1] == 1)
    {
        *dstPixel = (rawImage.pixels->a << 24) | rawImage.pixels->b | (rawImage.pixels->g << 8) | (rawImage.pixels->r << 16);
        dstPixela = dstPixel + 4;
        for (iter = 0; iter < rawImage.width; ++iter)
        {
            *dstPixela = (srcPixel->a << 24) | srcPixel->b | (srcPixel->g << 8) | (srcPixel->r << 16);
            dstPixela += 4;
            ++srcPixel;
        }
        *dstPixela = (rawImage.pixels[rawImage.width - 1].a << 24)
            | rawImage.pixels[rawImage.width - 1].b
            | (rawImage.pixels[rawImage.width - 1].g << 8)
            | (rawImage.pixels[rawImage.width - 1].r << 16);
    }
    else
    {
        endCount = anonymousConfig[1] + (anonymousConfig[1] >> 1);
        for (iter = 0; iter < endCount; ++iter)
        {
            *dstPixel = (srcPixel->a << 24) | srcPixel->b | (srcPixel->g << 8) | (srcPixel->r << 16);
            dstPixel += 4;
        }
        if ((anonymousConfig[1] & (anonymousConfig[1] - 1)) != 0)
            MyAssertHandler(
                ".\\r_light_load_obj.cpp",
                172,
                0,
                "%s\n\t(cfg->zoom) = %i",
                "((((cfg->zoom) & ((cfg->zoom) - 1)) == 0))",
                anonymousConfig[1]);
        lerp = 1;
        do
        {
            do
            {
                lerpedPixel.r = (anonymousConfig[1] + lerp * srcPixel[1].r + (2 * anonymousConfig[1] - lerp) * srcPixel->r)
                    / (2
                        * anonymousConfig[1]);
                lerpedPixel.g = (anonymousConfig[1] + lerp * srcPixel[1].g + (2 * anonymousConfig[1] - lerp) * srcPixel->g)
                    / (2
                        * anonymousConfig[1]);
                lerpedPixel.b = (anonymousConfig[1] + lerp * srcPixel[1].b + (2 * anonymousConfig[1] - lerp) * srcPixel->b)
                    / (2
                        * anonymousConfig[1]);
                lerpedPixel.a = (anonymousConfig[1] + lerp * srcPixel[1].a + (2 * anonymousConfig[1] - lerp) * srcPixel->a)
                    / (2
                        * anonymousConfig[1]);
                *dstPixel = (lerpedPixel.a << 24) | lerpedPixel.b | (lerpedPixel.g << 8) | (lerpedPixel.r << 16);
                dstPixel += 4;
                lerp += 2;
            } while (lerp <= 2 * anonymousConfig[1]);
            lerp = 1;
            ++srcPixel;
        } while (srcPixel != &rawImage.pixels[rawImage.width - 1]);
        if (&dstPixel[-*anonymousConfig] >> 2 != anonymousConfig[1] * &header.xmodelPieces[1].name[rawImage.width + 1]
            - endCount)
            MyAssertHandler(
                ".\\r_light_load_obj.cpp",
                193,
                1,
                "(dstPixel - cfg->dest) / 4u == (def->lmapLookupStart + rawImage.width + 1) * cfg->zoom - endCount\n\t%i, %i",
                &dstPixel[-*anonymousConfig] >> 2,
                anonymousConfig[1] * &header.xmodelPieces[1].name[rawImage.width + 1] - endCount);
        for (iter = 0; iter < endCount; ++iter)
        {
            *dstPixel = (srcPixel->a << 24) | srcPixel->b | (srcPixel->g << 8) | (srcPixel->r << 16);
            dstPixel += 4;
        }
    }
    Image_FreeRawPixels(&rawImage);
}

void __cdecl R_LoadLightmaps(GfxBspLoad *load)
{
    char *v1; // eax
    GfxImage *v2; // eax
    char *v3; // eax
    GfxImage *v4; // eax
    unsigned __int8 *primaryImage; // [esp+8h] [ebp-1C4h]
    int reorder[32]; // [esp+Ch] [ebp-1C0h] BYREF
    int tileIndex; // [esp+8Ch] [ebp-140h]
    int totalImageSize; // [esp+90h] [ebp-13Ch]
    const unsigned __int8 *buf; // [esp+94h] [ebp-138h]
    int width; // [esp+98h] [ebp-134h]
    int height; // [esp+9Ch] [ebp-130h]
    const unsigned __int8 *buf_p; // [esp+A0h] [ebp-12Ch]
    r_lightmapGroup_t groupInfo[31]; // [esp+A4h] [ebp-128h] BYREF
    unsigned __int8 newLmapIndex; // [esp+19Fh] [ebp-2Dh]
    int groupCount; // [esp+1A0h] [ebp-2Ch]
    unsigned int len; // [esp+1A4h] [ebp-28h] BYREF
    int oldLmapBaseIndex; // [esp+1A8h] [ebp-24h]
    int x; // [esp+1ACh] [ebp-20h]
    int y; // [esp+1B0h] [ebp-1Ch]
    int imageFlags; // [esp+1B4h] [ebp-18h]
    LightDefCopyConfig defCopyCfg; // [esp+1B8h] [ebp-14h] BYREF
    unsigned __int8 *secondaryImage; // [esp+1C0h] [ebp-Ch]
    int oldLmapCount; // [esp+1C4h] [ebp-8h]
    int oldLmapIndex; // [esp+1C8h] [ebp-4h]

    if (!load)
        MyAssertHandler(".\\r_bsp_load_obj.cpp", 687, 0, "%s", "load");
    load->lmapMergeInfo[31].index = 31;
    load->lmapMergeInfo[31].shift[0] = 0.0;
    load->lmapMergeInfo[31].shift[1] = 0.0;
    load->lmapMergeInfo[31].scale[0] = 1.0;
    load->lmapMergeInfo[31].scale[1] = 1.0;
    oldLmapCount = R_BuildLightmapMergability(load, groupInfo, reorder);
    if (oldLmapCount)
    {
        totalImageSize = groupInfo[0].highCount * 0x300000 * groupInfo[0].wideCount;
        primaryImage = (unsigned __int8 *)Hunk_AllocateTempMemory(totalImageSize, "R_LoadLightmaps");
        secondaryImage = &primaryImage[groupInfo[0].highCount * (groupInfo[0].wideCount << 20)];
        buf = (const unsigned char*)Com_GetBspLump(LUMP_LIGHTBYTES, 1u, &len);
        if (load->bspVersion < 7)
            len = 0;
        if (!len)
            memset(primaryImage, 0xFFu, totalImageSize);
        imageFlags = 56;
        s_world.lightmaps = (GfxLightmapArray*)Hunk_Alloc(0x100u, "R_LoadLightmaps", 20);
        newLmapIndex = 0;
        oldLmapBaseIndex = 0;
        while (oldLmapBaseIndex < oldLmapCount)
        {
            if (newLmapIndex && groupInfo[newLmapIndex].wideCount > *(&height + 2 * newLmapIndex))
                MyAssertHandler(
                    ".\\r_bsp_load_obj.cpp",
                    722,
                    0,
                    "%s",
                    "newLmapIndex == 0 || groupInfo[newLmapIndex].wideCount <= groupInfo[newLmapIndex - 1].wideCount");
            if (newLmapIndex && groupInfo[newLmapIndex].highCount > (int)(&buf_p)[2 * newLmapIndex])
                MyAssertHandler(
                    ".\\r_bsp_load_obj.cpp",
                    723,
                    0,
                    "%s",
                    "newLmapIndex == 0 || groupInfo[newLmapIndex].highCount <= groupInfo[newLmapIndex - 1].highCount");
            groupCount = groupInfo[newLmapIndex].highCount * groupInfo[newLmapIndex].wideCount;
            for (tileIndex = 0; tileIndex < groupCount; ++tileIndex)
            {
                oldLmapIndex = reorder[tileIndex + oldLmapBaseIndex];
                x = tileIndex % groupInfo[newLmapIndex].wideCount;
                y = tileIndex / groupInfo[newLmapIndex].wideCount;
                if (len)
                {
                    buf_p = &buf[0x300000 * oldLmapIndex];
                    R_CopyLightmap((char*)buf_p, 512, 512, 4, secondaryImage, x, y, groupInfo[newLmapIndex].wideCount);
                    buf_p += 0x100000;
                    R_CopyLightmap(
                        (char *)buf_p,
                        512,
                        512,
                        4,
                        secondaryImage,
                        x,
                        groupInfo[newLmapIndex].highCount + y,
                        groupInfo[newLmapIndex].wideCount);
                    buf_p += 0x100000;
                    R_CopyLightmap((char *)buf_p, 1024, 1024, 1, primaryImage, x, y, groupInfo[newLmapIndex].wideCount);
                }
                load->lmapMergeInfo[oldLmapIndex].index = newLmapIndex;
                load->lmapMergeInfo[oldLmapIndex].scale[0] = 1.0 / groupInfo[newLmapIndex].wideCount;
                load->lmapMergeInfo[oldLmapIndex].scale[1] = 1.0 / groupInfo[newLmapIndex].highCount;
                load->lmapMergeInfo[oldLmapIndex].shift[0] = x * load->lmapMergeInfo[oldLmapIndex].scale[0];
                load->lmapMergeInfo[oldLmapIndex].shift[1] = y * load->lmapMergeInfo[oldLmapIndex].scale[1];
            }
            defCopyCfg.dest = secondaryImage;
            defCopyCfg.zoom = groupInfo[newLmapIndex].wideCount;
            R_EnumLightDefs(R_CopyLightDefAttenuationImage, &defCopyCfg);
            v1 = va("*lightmap%i_primary", newLmapIndex);
            v2 = Image_Alloc(v1, 2u, 1u, 4u);
            s_world.lightmaps[newLmapIndex].primary = v2;
            if (!s_world.lightmaps[newLmapIndex].primary)
                MyAssertHandler(".\\r_bsp_load_obj.cpp", 756, 1, "%s", "s_world.lightmaps[newLmapIndex].primary");
            width = groupInfo[newLmapIndex].wideCount << 10;
            height = groupInfo[newLmapIndex].highCount << 10;
            Image_Generate2D(s_world.lightmaps[newLmapIndex].primary, primaryImage, width, height, D3DFMT_L8);
            v3 = va("*lightmap%i_secondary", newLmapIndex);
            v4 = Image_Alloc(v3, 2u, 1u, 4u);
            s_world.lightmaps[newLmapIndex].secondary = v4;
            if (!s_world.lightmaps[newLmapIndex].secondary)
                MyAssertHandler(".\\r_bsp_load_obj.cpp", 762, 1, "%s", "s_world.lightmaps[newLmapIndex].secondary");
            width = groupInfo[newLmapIndex].wideCount << 9;
            height = groupInfo[newLmapIndex].highCount << 10;
            Image_Generate2D(s_world.lightmaps[newLmapIndex].secondary, secondaryImage, width, height, D3DFMT_A8R8G8B8);
            oldLmapBaseIndex += groupCount;
            ++newLmapIndex;
        }
        s_world.lightmapCount = newLmapIndex;
        Hunk_FreeTempMemory((char*)primaryImage);
        if (s_world.lightmapCount > 31)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                771,
                0,
                "%s\n\t(s_world.lightmapCount) = %i",
                "(s_world.lightmapCount <= ((93 * 1024 * 1024) / ((1024 * 1024 * 1 * 1) + (512 * 512 * 4 * 2))))",
                s_world.lightmapCount);
        s_world.lightmapPrimaryTextures = (GfxTexture*)Hunk_Alloc(4 * s_world.lightmapCount, "R_LoadLightmaps", 20);
        s_world.lightmapSecondaryTextures = (GfxTexture*)Hunk_Alloc(4 * s_world.lightmapCount, "R_LoadLightmaps", 20);
    }
    else
    {
        s_world.lightmapCount = 0;
    }
}

GfxWorld *__cdecl R_LoadWorldInternal(const char *name)
{
    char *FilenameSubString; // eax
    unsigned __int16 EntityCount; // ax
    char v4; // [esp+3h] [ebp-8Dh]
    char *v5; // [esp+8h] [ebp-88h]
    char *v6; // [esp+Ch] [ebp-84h]
    char v7; // [esp+23h] [ebp-6Dh]
    char *v8; // [esp+28h] [ebp-68h]
    const char *v9; // [esp+2Ch] [ebp-64h]
    char baseName[68]; // [esp+40h] [ebp-50h] BYREF
    unsigned int drawType; // [esp+88h] [ebp-8h]
    DynEntityCollType collType; // [esp+8Ch] [ebp-4h]

    memset(&s_world, 0, sizeof(s_world));
    memset(&rgl, 0, sizeof(rgl));
    ProfLoad_Begin("Load world initialization");
    rgl.load.trisType = (TrisType)R_ChooseTrisContextType();
    rgl.load.bspVersion = Com_GetBspVersion();
    s_world.name = (const char*)Hunk_Alloc(strlen(name) + 1, "R_LoadWorldInternal", 20);
    v9 = name;
    v8 = (char*)s_world.name;
    do
    {
        v7 = *v9;
        *v8++ = *v9++;
    } while (v7);
    FilenameSubString = (char*)Com_GetFilenameSubString(s_world.name);
    I_strncpyz(baseName, FilenameSubString, 64);
    Com_StripExtension(baseName, baseName);
    s_world.baseName = (char*)Hunk_Alloc(&baseName[strlen(baseName) + 1] - &baseName[1] + 1, "R_LoadWorldInternal", 20);
    v6 = baseName;
    v5 = (char*)s_world.baseName;
    do
    {
        v4 = *v6;
        *v5++ = *v6++;
    } while (v4);
    s_world.dpvsPlanes.planes = CM_GetPlanes();
    s_world.planeCount = CM_GetPlaneCount();
    ProfLoad_End();
    for (drawType = 0; drawType < 2; ++drawType)
    {
        collType = (DynEntityCollType)drawType;
        EntityCount = DynEnt_GetEntityCount((DynEntityCollType)drawType);
        s_world.dpvsDyn.dynEntClientCount[drawType] = EntityCount;
        s_world.dpvsDyn.dynEntClientWordCount[drawType] = (s_world.dpvsDyn.dynEntClientCount[drawType] + 31) >> 5;
    }
    ProfLoad_Begin("Load world materials");
    R_LoadStep("materials");
    R_LoadMaterials(&rgl.load);
    ProfLoad_End();

    ProfLoad_Begin("Load lighting");
    R_LoadStep("sun settings");
    R_LoadSunSettings();
    R_LoadStep("primary lights");
    R_LoadPrimaryLights(rgl.load.bspVersion);
    R_LoadStep("light regions");
    R_LoadLightRegions();
    R_LoadStep("lightmaps");
    R_LoadLightmaps(&rgl.load);
    ProfLoad_End();

    ProfLoad_Begin("Load light grid");
    if (rgl.load.bspVersion > 0xF)
    {
        R_LoadStep("lightgrid colors");
        R_LoadLightGridColors(rgl.load.bspVersion);
        R_LoadStep("lightgrid row data");
        R_LoadLightGridRowData();
        R_LoadStep("lightgrid entries");
        R_LoadLightGridEntries();
        R_LoadStep("lightgrid header");
        R_LoadLightGridHeader();
    }
    else
    {
        R_LoadStep("lightgrid colors");
        R_LoadLightGridColors(rgl.load.bspVersion);
        R_LoadStep("lightgrid points");
        R_LoadLightGridPoints_Version15(rgl.load.bspVersion);
    }
    ProfLoad_End();
    R_AllocShadowGeometryHeaderMemory();
    ProfLoad_Begin("Load world submodels");
    R_LoadStep("submodels");
    R_LoadSubmodels(rgl.load.trisType);
    ProfLoad_End();
    ProfLoad_Begin("Load world surfaces");
    R_LoadStep("surfaces");
    R_LoadSurfaces(&rgl.load);
    ProfLoad_End();
    ProfLoad_Begin("Load cull groups");
    R_LoadStep("cull groups");
    R_LoadCullGroups(rgl.load.trisType);
    R_LoadStep("cull group indices");
    R_LoadCullGroupIndices();
    ProfLoad_End();
    ProfLoad_Begin("Load portals");
    R_LoadStep("portal vertices");
    R_LoadPortalVerts();
    R_LoadStep("AABB trees");
    R_LoadAabbTrees(rgl.load.trisType);
    R_LoadStep("cells");
    R_LoadCells(rgl.load.bspVersion, rgl.load.trisType);
    R_LoadStep("portals");
    R_LoadPortals();
    R_LoadStep("nodes and leafs");
    R_LoadNodesAndLeafs(rgl.load.bspVersion);
    ProfLoad_End();
    Material_Sort();
    R_SortSurfaces();
    R_BuildNoDecalSubModels();
    ProfLoad_Begin("Load renderer entities");
    R_LoadStep("entities");
    R_LoadEntities(rgl.load.bspVersion);
    ProfLoad_End();
    ProfLoad_Begin("Load reflection probes");
    R_LoadStep("reflection probes");
    if (rgl.load.bspVersion > 7)
        R_LoadReflectionProbes(rgl.load.bspVersion);
    else
        R_CreateDefaultProbes();
    if (rgl.load.bspVersion <= 0x15)
        R_AddAllProbesToAllCells();
    ProfLoad_End();
    R_SetStaticModelReflectionProbes();
    R_PostLoadEntities();
    R_InitShadowGeometryArrays();
    ProfLoad_Begin("Load sun");
    R_LoadSun(name, &s_world.sun);
    ProfLoad_End();
    ProfLoad_Begin("Register outdoor image");
    R_RegisterOutdoorImage(&s_world, rgl.load.outdoorMins, rgl.load.outdoorMaxs);
    ProfLoad_End();
    R_LoadWorldRuntime();
    R_AssignSModelCacheResources(&s_world);
    memset(&rgl, 0, sizeof(rgl));
    return &s_world;
}