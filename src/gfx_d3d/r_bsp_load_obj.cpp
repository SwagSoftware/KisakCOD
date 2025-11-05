#include "r_bsp.h"
#include "r_init.h"
#include "r_light.h"
#include "r_primarylights.h"
#include <DynEntity/DynEntity_client.h>
#include <qcommon/com_bsp.h>
#include "r_dvars.h"
#include <universal/q_parse.h>
#include "r_image.h"
#include <qcommon/com_pack.h>
#include "r_utils.h"
#include "r_buffers.h"

#include <algorithm>
#include "r_model.h"
#include "r_xsurface.h"
#include "rb_light.h"
#include "r_reflection_probe.h"
#include "r_staticmodel.h"
#include "r_dpvs.h"
#include "r_outdoor.h"

#ifdef KISAK_MP
#include <cgame_mp/cg_local_mp.h>
#elif KISAK_SP
#include <cgame/cg_ents.h>
#include <client/cl_scrn.h>
#endif

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
    iassert( light );
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

    iassert( rgp.world );
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

    iassert( params );
    while (1)
    {
        src = Com_Parse((const char**)&text);
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
            srca = Com_Parse((const char **)&text);
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
        iassert( comWorld.isInUse );
        s_world.primaryLightCount = comWorld.primaryLightCount;
        s_world.sunPrimaryLightIndex = comWorld.primaryLightCount > 1 && Com_GetPrimaryLight(1)->type == 1;
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
            iassert( s_world.lightRegion[regionIter].hulls == NULL );
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
            usedAxisCount += *(_DWORD *)&diskHulls[76 * hullIter + 72];
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

    iassert( load );
    iassert( coupling );
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

void __cdecl R_CopyLightDefAttenuationImage(GfxLightDef *def, _DWORD *anonymousConfig)
{
    int endCount; // [esp+30h] [ebp-7Ch]
    unsigned __int8 *dstPixel; // [esp+38h] [ebp-74h]
    unsigned __int8 *dstPixela; // [esp+38h] [ebp-74h]
    unsigned int lerp; // [esp+3Ch] [ebp-70h]
    GfxRawPixel *srcPixel; // [esp+40h] [ebp-6Ch]
    GfxRawImage rawImage; // [esp+44h] [ebp-68h] BYREF
    GfxRawPixel lerpedPixel; // [esp+A4h] [ebp-8h]
    int iter; // [esp+A8h] [ebp-4h]

    Image_GetRawPixels((char*)def->attenuation.image->name, &rawImage);
    iassert( rawImage.width == def->attenuation.image->width );
    dstPixel = (unsigned char*)(*anonymousConfig + anonymousConfig[1] * (4 * def->lmapLookupStart - 4));
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
        if ((int)((int)&dstPixel[-(int)*anonymousConfig] >> 2) != ((int)(anonymousConfig[1] * (def->lmapLookupStart + rawImage.width + 1) - endCount)))
            MyAssertHandler(
                ".\\r_light_load_obj.cpp",
                193,
                1,
                "(dstPixel - cfg->dest) / 4u == (def->lmapLookupStart + rawImage.width + 1) * cfg->zoom - endCount\n\t%i, %i",
                (int)&dstPixel[-(int)*anonymousConfig] >> 2,
                anonymousConfig[1] * (def->lmapLookupStart + rawImage.width + 1) - endCount);
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

    iassert( load );
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
            R_EnumLightDefs((void(*)(GfxLightDef*, void*))R_CopyLightDefAttenuationImage, &defCopyCfg);
            v1 = va("*lightmap%i_primary", newLmapIndex);
            v2 = Image_Alloc(v1, 2u, 1u, 4u);
            s_world.lightmaps[newLmapIndex].primary = v2;
            iassert( s_world.lightmaps[newLmapIndex].primary );
            width = groupInfo[newLmapIndex].wideCount << 10;
            height = groupInfo[newLmapIndex].highCount << 10;
            Image_Generate2D(s_world.lightmaps[newLmapIndex].primary, primaryImage, width, height, D3DFMT_L8);
            v3 = va("*lightmap%i_secondary", newLmapIndex);
            v4 = Image_Alloc(v3, 2u, 1u, 4u);
            s_world.lightmaps[newLmapIndex].secondary = v4;
            iassert( s_world.lightmaps[newLmapIndex].secondary );
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

GfxShadowGeometry *R_AllocShadowGeometryHeaderMemory()
{
    GfxShadowGeometry *result; // eax

    iassert( s_world.shadowGeom == NULL );
    result = (GfxShadowGeometry*)Hunk_Alloc(12 * s_world.primaryLightCount, "R_AllocShadowGeometryHeaderMemory", 20);
    s_world.shadowGeom = result;
    return result;
}

void __cdecl R_LoadSubmodels(TrisType trisType)
{
    unsigned __int16 v1; // [esp+4h] [ebp-18h]
    GfxBrushModel *out; // [esp+8h] [ebp-14h]
    DiskBrushModel *in; // [esp+Ch] [ebp-10h]
    int axis; // [esp+10h] [ebp-Ch]
    unsigned int modelIndex; // [esp+14h] [ebp-8h]
    unsigned int modelCount; // [esp+18h] [ebp-4h] BYREF

    in = (DiskBrushModel *)Com_GetBspLump(LUMP_MODELS, 48u, &modelCount);
    out = (GfxBrushModel *)Hunk_Alloc(56 * modelCount, "R_LoadSubmodels", 20);
    s_world.models = out;
    s_world.modelCount = modelCount;
    for (modelIndex = 0; modelIndex < modelCount; ++modelIndex)
    {
        for (axis = 0; axis < 3; ++axis)
        {
            out->bounds[0][axis] = in->mins[axis];
            out->bounds[1][axis] = in->maxs[axis];
        }
        out->surfaceCount = in->triSoupCount[trisType];
        if (out->surfaceCount)
            v1 = in->firstTriSoup[trisType];
        else
            v1 = -1;
        out->startSurfIndex = v1;
        ++in;
        ++out;
    }
}

void __cdecl R_SurfCalculateMagicPortalVerts(
    const Material *material,
    GfxSurface *surface,
    const DiskGfxVertex *vertsDisk,
    const r_lightmapMerge_t *merge,
    GfxWorldVertex *vertsMem)
{
    float scale; // [esp+8h] [ebp-5058h]
    float *v6; // [esp+Ch] [ebp-5054h]
    int v7; // [esp+10h] [ebp-5050h]
    int kk; // [esp+14h] [ebp-504Ch]
    int v9; // [esp+18h] [ebp-5048h]
    int v10; // [esp+1Ch] [ebp-5044h]
    int jj; // [esp+24h] [ebp-503Ch]
    int ii; // [esp+28h] [ebp-5038h]
    int v13[3]; // [esp+2Ch] [ebp-5034h]
    int n; // [esp+38h] [ebp-5028h]
    int k; // [esp+3Ch] [ebp-5024h]
    unsigned int m; // [esp+40h] [ebp-5020h]
    int v17[3]; // [esp+44h] [ebp-501Ch]
    unsigned int j; // [esp+50h] [ebp-5010h]
    char v19; // [esp+57h] [ebp-5009h]
    float a[4096]; // [esp+58h] [ebp-5008h] BYREF
    unsigned int triCount; // [esp+4058h] [ebp-1008h]
    unsigned int i; // [esp+405Ch] [ebp-1004h]
    int v23[1024]; // [esp+4060h] [ebp-1000h]

    triCount = surface->tris.triCount;
    if (triCount > 0x400)
    {
        MyAssertHandler(".\\r_bsp_load_obj.cpp", 1678, 0, "%s", "triCount <= ARRAY_COUNT( triFillId )");
        MyAssertHandler(".\\r_bsp_load_obj.cpp", 1679, 0, "%s", "triCount <= ARRAY_COUNT( centerAccum )");
        MyAssertHandler(".\\r_bsp_load_obj.cpp", 1680, 0, "%s", "triCount <= ARRAY_COUNT( centerWeight )");
    }
    for (i = 0; i < triCount; ++i)
    {
        v23[i] = i;
        v6 = &a[3 * i];
        *v6 = 0.0;
        v6[1] = 0.0;
        v6[2] = 0.0;
        a[i + 3072] = 0.0;
    }
    v19 = 0;
    while (!v19)
    {
        v19 = 1;
        for (j = 0; j < triCount; ++j)
        {
            for (k = 0; k != 3; ++k)
                v17[k] = surface->tris.firstVertex + s_world.indices[3 * j + k + surface->tris.baseIndex];
            for (m = 0; m < triCount; ++m)
            {
                for (k = 0; k != 3; ++k)
                    v13[k] = surface->tris.firstVertex + s_world.indices[3 * m + k + surface->tris.baseIndex];
                for (n = 0; n != 3; ++n)
                {
                    for (ii = 0; ii != 3; ++ii)
                    {
                        if (v17[n] == v13[ii] && v23[j] != v23[m])
                        {
                            if (v23[j] >= v23[m])
                                v23[j] = v23[m];
                            else
                                v23[m] = v23[j];
                            v19 = 0;
                        }
                    }
                }
            }
        }
    }
    for (i = 0; i < triCount; ++i)
    {
        for (jj = 0; jj != 3; ++jj)
        {
            v10 = v23[i];
            Vec3Add(
                &a[3 * v10],
                vertsMem[surface->tris.firstVertex + s_world.indices[3 * i + jj + surface->tris.baseIndex]].xyz,
                &a[3 * v10]);
            a[v10 + 3072] = a[v10 + 3072] + 1.0;
        }
    }
    for (i = 0; i < triCount; ++i)
    {
        if (a[i + 3072] > 0.0)
        {
            scale = 1.0 / a[i + 3072];
            Vec3Scale(&a[3 * i], scale, &a[3 * i]);
        }
    }
    for (i = 0; i < triCount; ++i)
    {
        v9 = v23[i];
        for (kk = 0; kk != 3; ++kk)
        {
            v7 = surface->tris.firstVertex + s_world.indices[3 * i + kk + surface->tris.baseIndex];
            vertsMem[v7].texCoord[0] = a[3 * v9];
            vertsMem[v7].texCoord[1] = a[3 * v9 + 1];
            vertsMem[v7].lmapCoord[0] = a[3 * v9 + 2];
            vertsMem[v7].lmapCoord[1] = 1.0;
        }
    }
}

void __cdecl R_FinalizeSurfVerts(
    const Material *material,
    GfxSurface *surface,
    const DiskGfxVertex *vertsDisk,
    const r_lightmapMerge_t *merge,
    GfxWorldVertex *vertsMem,
    unsigned int vertCount)
{
    float v6; // [esp+0h] [ebp-14h]
    float v7; // [esp+4h] [ebp-10h]
    unsigned int indexCount; // [esp+8h] [ebp-Ch]
    unsigned int vertIndex; // [esp+Ch] [ebp-8h]
    unsigned int indexIndex; // [esp+10h] [ebp-4h]

    iassert( material );
    iassert( surface );
    iassert( vertsDisk );
    iassert( merge );
    iassert( vertsMem );
    ClearBounds(surface->bounds[0], surface->bounds[1]);
    indexCount = 3 * surface->tris.triCount;
    if (surface->tris.baseIndex + indexCount - 1 >= s_world.indexCount)
        MyAssertHandler(
            ".\\r_bsp_load_obj.cpp",
            1787,
            0,
            "surface->tris.baseIndex + indexCount - 1 doesn't index s_world.indexCount\n\t%i not in [0, %i)",
            surface->tris.baseIndex + indexCount - 1,
            s_world.indexCount);
    for (indexIndex = 0; indexIndex < indexCount; ++indexIndex)
    {
        vertIndex = surface->tris.firstVertex + s_world.indices[indexIndex + surface->tris.baseIndex];
        if (vertIndex >= vertCount)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                1791,
                0,
                "vertIndex doesn't index vertCount\n\t%i not in [0, %i)",
                vertIndex,
                vertCount);
        AddPointToBounds(vertsMem[vertIndex].xyz, surface->bounds[0], surface->bounds[1]);
        v7 = vertsDisk[vertIndex].lmapCoord[0] * merge->scale[0] + merge->shift[0];
        vertsMem[vertIndex].lmapCoord[0] = v7;
        v6 = vertsDisk[vertIndex].lmapCoord[1] * merge->scale[1] + merge->shift[1];
        vertsMem[vertIndex].lmapCoord[1] = v6;
    }
    if ((material->info.gameFlags & 0x20) != 0)
        R_SurfCalculateMagicPortalVerts(material, surface, vertsDisk, merge, vertsMem);
}

unsigned __int8 *__cdecl R_LoadSurfaceAlloc(unsigned int bytes)
{
    return Hunk_Alloc(bytes, "R_LoadSurfaces", 20);
}

MaterialUsage *__cdecl R_GetMaterialUsageData(Material *material)
{
    MaterialUsage *materialUsage; // [esp+0h] [ebp-Ch]
    bool exists; // [esp+7h] [ebp-5h] BYREF
    unsigned __int16 hashIndex; // [esp+8h] [ebp-4h] BYREF

    Material_GetHashIndex(material->info.name, &hashIndex, &exists);
    if (!exists)
        return 0;
    materialUsage = &rg.materialUsage[hashIndex];
    iassert( material == rg.materialHashTable[hashIndex] );
    materialUsage->material = material;
    return materialUsage;
}

void __cdecl R_MaterialUsage(Material *material, unsigned int firstVertex, int vertexCount, int surfPlusIndexSize)
{
    unsigned int *v4; // eax
    VertUsage *vertUsage; // [esp+0h] [ebp-8h]
    MaterialUsage *materialUsage; // [esp+4h] [ebp-4h]

    materialUsage = R_GetMaterialUsageData(material);
    if (materialUsage)
    {
        materialUsage->memory += surfPlusIndexSize + 48;
        for (vertUsage = materialUsage->verts; vertUsage; vertUsage = vertUsage->next)
        {
            if (firstVertex == vertUsage->index)
                return;
        }
        v4 = (unsigned int *)Z_Malloc(8, "R_MaterialUsage", 0);
        *v4 = firstVertex;
        v4[1] = (unsigned int)materialUsage->verts;
        materialUsage->verts = (VertUsage*)v4;
        materialUsage->memory += 44 * vertexCount;
    }
}

void __cdecl R_ValidateSurfaceLightmapUsage(const GfxSurface *surface)
{
    const MaterialTechnique *technique; // [esp+8h] [ebp-Ch]
    MaterialTechniqueType techType; // [esp+Ch] [ebp-8h]
    unsigned int passIter; // [esp+10h] [ebp-4h]

    if (surface->lightmapIndex == 31)
    {
        for (techType = TECHNIQUE_DEPTH_PREPASS; techType < TECHNIQUE_COUNT; ++techType)
        {
            technique = Material_GetTechnique(surface->material, techType);
            if (technique)
            {
                for (passIter = 0; passIter < technique->passCount; ++passIter)
                {
                    if ((technique->passArray[passIter].customSamplerFlags & 6) != 0)
                        Com_Error(
                            ERR_DROP,
                            "World surface using material '%s' doesn't have a lightmap.  This usually means the map was compiled with a"
                            " different version of this material than you have locally.",
                            surface->material->info.name);
                }
            }
        }
    }
}

void __cdecl R_SetSkyImage(const Material *skyMaterial)
{
    unsigned int colorMapHash; // [esp+0h] [ebp-Ch]
    int textureIndex; // [esp+4h] [ebp-8h]
    const MaterialTextureDef *texdef; // [esp+8h] [ebp-4h]

    colorMapHash = R_HashString("colorMap");
    for (textureIndex = 0; textureIndex < skyMaterial->textureCount; ++textureIndex)
    {
        texdef = &skyMaterial->textureTable[textureIndex];
        if (texdef->nameHash == colorMapHash)
        {
            if (texdef->semantic == 11 || texdef->u.image->mapType != MAPTYPE_CUBE)
                Com_Error(
                    ERR_DROP,
                    "colorMap '%s' for sky material '%s' is not a cubemap\n",
                    texdef->u.image->name,
                    skyMaterial->info.name);
            s_world.skyImage = texdef->u.image;
            s_world.skySamplerState = texdef->samplerState;
            return;
        }
    }
}

void __cdecl R_CalculateWorldBounds(float *mins, float *maxs)
{
    int surfIndex; // [esp+0h] [ebp-8h]

    if (s_world.surfaceCount)
    {
        ClearBounds(mins, maxs);
        for (surfIndex = 0; surfIndex < s_world.surfaceCount; ++surfIndex)
            ExpandBounds(s_world.dpvs.surfaces[surfIndex].bounds[0], s_world.dpvs.surfaces[surfIndex].bounds[1], mins, maxs);
    }
    else
    {
        *mins = 0.0;
        mins[1] = 0.0;
        mins[2] = 0.0;
        *maxs = 0.0;
        maxs[1] = 0.0;
        maxs[2] = 0.0;
    }
}

void __cdecl R_CalculateOutdoorBounds(GfxBspLoad *load, const DiskTriangleSoup *diskSurfaces)
{
    int surfIndex; // [esp+0h] [ebp-10h]
    int surfCount; // [esp+4h] [ebp-Ch]

    ClearBounds(load->outdoorMins, load->outdoorMaxs);
    iassert( s_world.modelCount > 0 );
    if (s_world.models->startSurfIndex)
        MyAssertHandler(
            ".\\r_bsp_load_obj.cpp",
            1896,
            0,
            "%s\n\t(s_world.models[0].startSurfIndex) = %i",
            "(s_world.models[0].startSurfIndex == 0)",
            s_world.models->startSurfIndex);
    surfCount = s_world.models->surfaceCount;
    for (surfIndex = 0; surfIndex < surfCount; ++surfIndex)
    {
        if ((s_world.dpvs.surfaces[surfIndex].material->info.gameFlags & 8) == 0
            && (load->diskMaterials[diskSurfaces[surfIndex].materialIndex].contentFlags & 0x2001) != 0)
        {
            ExpandBounds(
                s_world.dpvs.surfaces[surfIndex].bounds[0],
                s_world.dpvs.surfaces[surfIndex].bounds[1],
                load->outdoorMins,
                load->outdoorMaxs);
        }
    }
}

void __cdecl R_CreateMaterialList()
{
    int memory; // [esp+4h] [ebp-10h]
    unsigned __int16 hashIndex; // [esp+8h] [ebp-Ch]
    unsigned __int16 hashIndexa; // [esp+8h] [ebp-Ch]
    MaterialMemory *materialMemory; // [esp+Ch] [ebp-8h]
    int index; // [esp+10h] [ebp-4h]

    s_world.materialMemoryCount = 0;
    for (hashIndex = 0; hashIndex < 0x800u; ++hashIndex)
    {
        if (rg.materialUsage[hashIndex].memory)
            ++s_world.materialMemoryCount;
    }
    if (s_world.materialMemoryCount)
    {
        s_world.materialMemory = (MaterialMemory*)Hunk_Alloc(8 * s_world.materialMemoryCount, "R_CreateMaterialList", 20);
        index = 0;
        for (hashIndexa = 0; hashIndexa < 0x800u; ++hashIndexa)
        {
            memory = rg.materialUsage[hashIndexa].memory;
            if (memory)
            {
                iassert( index < s_world.materialMemoryCount );
                materialMemory = &s_world.materialMemory[index];
                materialMemory->material = rg.materialUsage[hashIndexa].material;
                materialMemory->memory = memory;
                ++index;
            }
        }
        iassert( index == s_world.materialMemoryCount );
    }
}

void __cdecl R_LoadSurfaces(GfxBspLoad *load)
{
    float v1; // [esp+4h] [ebp-94h]
    PackedUnitVec v2; // [esp+10h] [ebp-88h]
    PackedUnitVec v3; // [esp+14h] [ebp-84h]
    unsigned __int8 *vertLayerDataMem; // [esp+18h] [ebp-80h]
    unsigned int firstSurfIndex; // [esp+1Ch] [ebp-7Ch]
    unsigned __int8 dummyData[4]; // [esp+20h] [ebp-78h] BYREF
    unsigned int surfIndex; // [esp+24h] [ebp-74h]
    int baseIndex; // [esp+28h] [ebp-70h]
    int lmapIndex; // [esp+2Ch] [ebp-6Ch]
    const unsigned __int8 *vertLayerDataDisk; // [esp+30h] [ebp-68h]
    Material *material; // [esp+34h] [ebp-64h]
    srfTriangles_t *tris; // [esp+38h] [ebp-60h]
    float normal[3]; // [esp+3Ch] [ebp-5Ch] BYREF
    float tangent[3]; // [esp+48h] [ebp-50h] BYREF
    unsigned int surfCount; // [esp+54h] [ebp-44h] BYREF
    float binormal[3]; // [esp+58h] [ebp-40h] BYREF
    DiskTriangleSoup *diskSurfaces; // [esp+64h] [ebp-34h] BYREF
    unsigned int indexCount; // [esp+68h] [ebp-30h] BYREF
    const unsigned __int16 *indices; // [esp+6Ch] [ebp-2Ch]
    LumpType lumpType; // [esp+70h] [ebp-28h]
    const DiskGfxVertex *vertsDisk; // [esp+74h] [ebp-24h]
    unsigned int vertCount; // [esp+78h] [ebp-20h] BYREF
    const Material *skyMaterial; // [esp+7Ch] [ebp-1Ch]
    GfxSurface *surface; // [esp+80h] [ebp-18h]
    GfxWorldVertex *vertsMem; // [esp+84h] [ebp-14h]
    unsigned int vertIndex; // [esp+88h] [ebp-10h]
    unsigned int vertLayerDataSize; // [esp+8Ch] [ebp-Ch] BYREF
    unsigned __int16 *worldIndices; // [esp+90h] [ebp-8h]
    unsigned __int16 surfIndexCount; // [esp+94h] [ebp-4h]

    iassert( load );
    if (load->bspVersion < 0x16)
        Com_PrintWarning(8, "Bsp compiled with old version of cod2map.\n");
    R_LoadTriangleSurfaces(load->bspVersion, load->trisType, &diskSurfaces, &surfCount);
    if (!surfCount)
        Com_Error(ERR_DROP, "LoadMap: no surfaces in %s", s_world.name);
    lumpType = load->trisType != TRIS_TYPE_LAYERED ? LUMP_UNLAYERED_DRAWVERTS : LUMP_DRAWVERTS;
    vertsDisk = (const DiskGfxVertex*)Com_GetBspLump(lumpType, 0x44u, &vertCount);
    if (!vertCount)
        Com_Error(ERR_DROP, "LoadMap: no vertices in %s", s_world.name);
    s_world.vertexCount = vertCount;
    if (load->trisType)
    {
        vertLayerDataSize = 4;
        vertLayerDataDisk = dummyData;
    }
    else
    {
        vertLayerDataDisk = (const unsigned __int8 *)Com_GetBspLump(LUMP_VERTEX_LAYER_DATA, 1u, &vertLayerDataSize);
        if (!vertLayerDataSize)
        {
            vertLayerDataSize = 4;
            vertLayerDataDisk = dummyData;
        }
    }
    vertLayerDataMem = R_LoadSurfaceAlloc(vertLayerDataSize);
    Com_Memcpy(vertLayerDataMem, vertLayerDataDisk, vertLayerDataSize);
    s_world.vertexLayerDataSize = vertLayerDataSize;
    s_world.vld.data = vertLayerDataMem;
    vertsMem = (GfxWorldVertex*)R_LoadSurfaceAlloc(44 * vertCount);
    iassert( vertsMem );
    s_world.vd.vertices = vertsMem;
    for (vertIndex = 0; vertIndex < vertCount; ++vertIndex)
    {
        vertsMem[vertIndex].xyz[0] = vertsDisk[vertIndex].xyz[0];
        vertsMem[vertIndex].xyz[1] = vertsDisk[vertIndex].xyz[1];
        vertsMem[vertIndex].xyz[2] = vertsDisk[vertIndex].xyz[2];
        Byte4CopyBgraToVertexColor(vertsDisk[vertIndex].color, (unsigned char*)&vertsMem[vertIndex].color);
        vertsMem[vertIndex].texCoord[0] = vertsDisk[vertIndex].texCoord[0];
        vertsMem[vertIndex].texCoord[1] = vertsDisk[vertIndex].texCoord[1];
        vertsMem[vertIndex].lmapCoord[0] = vertsDisk[vertIndex].lmapCoord[0];
        vertsMem[vertIndex].lmapCoord[1] = vertsDisk[vertIndex].lmapCoord[1];
        tangent[0] = vertsDisk[vertIndex].tangent[0];
        tangent[1] = vertsDisk[vertIndex].tangent[1];
        tangent[2] = vertsDisk[vertIndex].tangent[2];
        v3.packed = Vec3PackUnitVec(tangent).packed;
        vertsMem[vertIndex].tangent = v3;
        normal[0] = vertsDisk[vertIndex].normal[0];
        normal[1] = vertsDisk[vertIndex].normal[1];
        normal[2] = vertsDisk[vertIndex].normal[2];
        v2.packed = Vec3PackUnitVec(normal).packed;
        vertsMem[vertIndex].normal = v2;
        Vec3Cross(vertsDisk[vertIndex].normal, vertsDisk[vertIndex].tangent, binormal);
        if (Vec3Dot(binormal, vertsDisk[vertIndex].binormal) < 0.0)
            v1 = -1.0;
        else
            v1 = 1.0;
        vertsMem[vertIndex].binormalSign = v1;
    }
    lumpType = load->trisType != TRIS_TYPE_LAYERED ? LUMP_UNLAYERED_DRAWINDICES : LUMP_DRAWINDICES;
    indices = (const unsigned short*)Com_GetBspLump(lumpType, 2u, &indexCount);
    iassert( (surfCount <= 65536) );
    s_world.surfaceCount = surfCount;
    s_world.dpvs.surfaces = (GfxSurface*)Hunk_Alloc(48 * surfCount, "R_LoadSurfaces", 20);
    s_world.indexCount = 0;
    for (surfIndex = 0; surfIndex < surfCount; ++surfIndex)
    {
        s_world.indexCount += diskSurfaces[surfIndex].indexCount;
        s_world.dpvs.surfaces[surfIndex].tris.baseIndex = -1;
    }
    worldIndices = (unsigned short*)R_LoadSurfaceAlloc(2 * s_world.indexCount);
    s_world.indices = worldIndices;
    baseIndex = 0;
    for (surfIndex = 0; surfIndex < surfCount; ++surfIndex)
    {
        if (diskSurfaces[surfIndex].firstVertex + diskSurfaces[surfIndex].vertexCount > s_world.vertexCount)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                2712,
                0,
                "%s",
                "diskSurfaces[surfIndex].firstVertex + diskSurfaces[surfIndex].vertexCount <= s_world.vertexCount");
    }
    for (firstSurfIndex = 0; firstSurfIndex < surfCount; ++firstSurfIndex)
    {
        surface = &s_world.dpvs.surfaces[firstSurfIndex];
        if (surface->tris.baseIndex < 0)
        {
            for (surfIndex = firstSurfIndex; surfIndex < surfCount; ++surfIndex)
            {
                surface = &s_world.dpvs.surfaces[surfIndex];
                if (surface->tris.baseIndex < 0
                    && (diskSurfaces[surfIndex].materialIndex == diskSurfaces[firstSurfIndex].materialIndex
                        || !I_stricmp(
                            load->diskMaterials[diskSurfaces[surfIndex].materialIndex].material,
                            load->diskMaterials[firstSurfIndex].material))
                    && diskSurfaces[surfIndex].reflectionProbeIndex == diskSurfaces[firstSurfIndex].reflectionProbeIndex
                    && diskSurfaces[surfIndex].lightmapIndex == diskSurfaces[firstSurfIndex].lightmapIndex)
                {
                    tris = &surface->tris;
                    surfIndexCount = diskSurfaces[surfIndex].indexCount;
                    surface->tris.baseIndex = baseIndex;
                    iassert( surface->flags == 0 );
                    if (load->bspVersion <= 0x13 || diskSurfaces[surfIndex].castsSunShadow)
                        surface->flags |= 1u;
                    if (surfIndexCount % 3)
                        MyAssertHandler(
                            ".\\r_bsp_load_obj.cpp",
                            2749,
                            0,
                            "%s\n\t(surfIndexCount) = %i",
                            "(!(surfIndexCount % 3))",
                            surfIndexCount);
                    tris->triCount = surfIndexCount / 3;
                    if (!tris->triCount)
                        MyAssertHandler(
                            ".\\r_bsp_load_obj.cpp",
                            2751,
                            0,
                            "%s\n\t(tris->triCount) = %i",
                            "(tris->triCount > 0)",
                            tris->triCount);
                    Com_Memcpy(&worldIndices[baseIndex], &indices[diskSurfaces[surfIndex].firstIndex], 2 * surfIndexCount);
                    baseIndex += surfIndexCount;
                }
            }
        }
    }
    skyMaterial = 0;
    s_world.skySurfCount = 0;
    for (surfIndex = 0; surfIndex < surfCount; ++surfIndex)
    {
        surface = &s_world.dpvs.surfaces[surfIndex];
        tris = &surface->tris;
        surface->tris.vertexLayerData = diskSurfaces[surfIndex].vertexLayerData;
        tris->firstVertex = diskSurfaces[surfIndex].firstVertex;
        tris->vertexCount = diskSurfaces[surfIndex].vertexCount;
        lmapIndex = diskSurfaces[surfIndex].lightmapIndex;
        if (lmapIndex >= 32)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                2768,
                0,
                "%s\n\t(lmapIndex) = %i",
                "((lmapIndex >= 0 && lmapIndex < ((93 * 1024 * 1024) / ((1024 * 1024 * 1 * 1) + (512 * 512 * 4 * 2)))) || lmapIndex == 31)",
                lmapIndex);
        if (load->lmapMergeInfo[lmapIndex].index >= 0x20u)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                2769,
                0,
                "%s\n\t(load->lmapMergeInfo[lmapIndex].index) = %i",
                "((load->lmapMergeInfo[lmapIndex].index >= 0 && load->lmapMergeInfo[lmapIndex].index < ((93 * 1024 * 1024) / ((10"
                "24 * 1024 * 1 * 1) + (512 * 512 * 4 * 2)))) || load->lmapMergeInfo[lmapIndex].index == 31)",
                load->lmapMergeInfo[lmapIndex].index);
        material = R_GetBspMaterial(diskSurfaces[surfIndex].materialIndex);
        R_FinalizeSurfVerts(material, surface, vertsDisk, &load->lmapMergeInfo[lmapIndex], vertsMem, vertCount);
        surface->material = material;
        surface->lightmapIndex = load->lmapMergeInfo[lmapIndex].index;
        surface->reflectionProbeIndex = diskSurfaces[surfIndex].reflectionProbeIndex;
        surface->primaryLightIndex = diskSurfaces[surfIndex].primaryLightIndex;
        R_MaterialUsage(material, tris->firstVertex, tris->vertexCount, 6 * tris->triCount + 16);
        if ((material->info.gameFlags & 8) != 0)
        {
            if (skyMaterial && skyMaterial != material)
                Com_Error(
                    ERR_DROP,
                    "map has at least two different skies: %s and %s\nOnly one sky per map is supported\n",
                    material->info.name,
                    skyMaterial->info.name);
            skyMaterial = material;
            ++s_world.skySurfCount;
            if (surface->primaryLightIndex
                && surface->primaryLightIndex != s_world.sunPrimaryLightIndex
                && (load->bspVersion > 0xC || surface->primaryLightIndex != 1))
            {
                MyAssertHandler(
                    ".\\r_bsp_load_obj.cpp",
                    2794,
                    0,
                    "%s\n\t(surface->primaryLightIndex) = %i",
                    "(surface->primaryLightIndex == 0 || surface->primaryLightIndex == s_world.sunPrimaryLightIndex || (load->bspVe"
                    "rsion <= 12 && surface->primaryLightIndex == 1))",
                    surface->primaryLightIndex);
            }
            surface->primaryLightIndex = s_world.sunPrimaryLightIndex;
        }
        R_ValidateSurfaceLightmapUsage(surface);
    }
    if (s_world.skySurfCount)
    {
        iassert( skyMaterial != NULL );
        R_SetSkyImage(skyMaterial);
        s_world.skyStartSurfs = (int*)Hunk_Alloc(4 * s_world.skySurfCount, "Sky surfaces", 20);
        s_world.skySurfCount = 0;
        for (surfIndex = 0; surfIndex < surfCount; ++surfIndex)
        {
            surface = &s_world.dpvs.surfaces[surfIndex];
            if (surface->material == skyMaterial)
                s_world.skyStartSurfs[s_world.skySurfCount++] = surfIndex;
        }
    }
    else
    {
        iassert( skyMaterial == NULL );
        s_world.skyImage = 0;
        s_world.skyStartSurfs = 0;
    }
    R_CalculateWorldBounds(s_world.mins, s_world.maxs);
    R_CalculateOutdoorBounds(load, diskSurfaces);
    R_UnloadTriangleSurfaces(load->bspVersion, diskSurfaces);
    R_CreateWorldVertexBuffer(&s_world.vd.worldVb, (int*)s_world.vd.vertices, 44 * s_world.vertexCount);
    R_CreateWorldVertexBuffer(&s_world.vld.layerVb, (int*)s_world.vld.data, s_world.vertexLayerDataSize);
    R_ShutdownMaterialUsage();
    R_CreateMaterialList();
}

void __cdecl R_LoadCullGroups(TrisType trisType)
{
    unsigned int firstSurface; // [esp+4h] [ebp-20h]
    GfxCullGroup *out; // [esp+8h] [ebp-1Ch]
    unsigned int surfaceCount; // [esp+Ch] [ebp-18h]
    unsigned int cullGroupCount; // [esp+10h] [ebp-14h] BYREF
    const DiskGfxCullGroup *in; // [esp+14h] [ebp-10h]
    LumpType lumpType; // [esp+18h] [ebp-Ch]
    unsigned int cullGroupIndex; // [esp+1Ch] [ebp-8h]
    unsigned int axis; // [esp+20h] [ebp-4h]

    lumpType = trisType != TRIS_TYPE_LAYERED ? LUMP_UNLAYERED_CULLGROUPS : LUMP_CULLGROUPS;
    in = (const DiskGfxCullGroup *)Com_GetBspLump(lumpType, 0x20u, &cullGroupCount);
    out = (GfxCullGroup *)Hunk_Alloc(32 * cullGroupCount, "R_LoadCullGroups", 22);
    s_world.dpvs.cullGroups = out;
    s_world.cullGroupCount = cullGroupCount;
    for (cullGroupIndex = 0; cullGroupIndex < cullGroupCount; ++cullGroupIndex)
    {
        for (axis = 0; axis < 3; ++axis)
        {
            out[cullGroupIndex].mins[axis] = in[cullGroupIndex].mins[axis];
            out[cullGroupIndex].maxs[axis] = in[cullGroupIndex].maxs[axis];
        }
        surfaceCount = in[cullGroupIndex].surfaceCount;
        if (surfaceCount)
            firstSurface = in[cullGroupIndex].firstSurface;
        else
            firstSurface = -1;
        out[cullGroupIndex].startSurfIndex = firstSurface;
        out[cullGroupIndex].surfaceCount = surfaceCount;
    }
}

void R_LoadCullGroupIndices()
{
    int *out; // [esp+0h] [ebp-Ch]
    unsigned int indexCount; // [esp+4h] [ebp-8h] BYREF
    const int *in; // [esp+8h] [ebp-4h]

    in = (int*)Com_GetBspLump(LUMP_CULLGROUPINDICES, 4u, &indexCount);
    out = (int*)Hunk_Alloc(4 * indexCount, "R_LoadCullGroupIndices", 22);
    rgl.cullGroupIndices = out;
    iassert( !indexCount || s_world.dpvs.cullGroups );
    Com_Memcpy(out, in, 4 * indexCount);
}

void R_LoadPortalVerts()
{
    char *in; // [esp+4h] [ebp-8h]
    unsigned int vertCount; // [esp+8h] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_PORTALVERTS, 0xCu, &vertCount);
    rgl.portalVerts = (float(*)[3])Hunk_Alloc(12 * vertCount, "R_LoadPortalVerts", 22);
    Com_Memcpy(rgl.portalVerts, in, 12 * vertCount);
}

int __cdecl R_FinishLoadingAabbTrees_r(GfxAabbTree *tree, int totalTreesUsed)
{
    GfxAabbTree *children; // [esp+0h] [ebp-10h]
    int childIndex; // [esp+4h] [ebp-Ch]
    const GfxSurface *surf; // [esp+8h] [ebp-8h]
    int surfNodeIndex; // [esp+Ch] [ebp-4h]

    iassert( tree );
    iassert( rgl.aabbTrees );
    ClearBounds(tree->mins, tree->maxs);
    if (tree->childCount)
    {
        tree->childrenOffset = &rgl.aabbTrees[totalTreesUsed] - tree;
        children = (tree + tree->childrenOffset);
        iassert( children[0].startSurfIndex == tree->startSurfIndex );
        totalTreesUsed += tree->childCount;
        for (childIndex = 0; childIndex < tree->childCount; ++childIndex)
        {
            totalTreesUsed = R_FinishLoadingAabbTrees_r(&children[childIndex], totalTreesUsed);
            ExpandBounds(children[childIndex].mins, children[childIndex].maxs, tree->mins, tree->maxs);
        }
    }
    else
    {
        surfNodeIndex = 0;
        surf = &s_world.dpvs.surfaces[tree->startSurfIndex];
        while (surfNodeIndex < tree->surfaceCount)
        {
            ExpandBounds(surf->bounds[0], surf->bounds[1], tree->mins, tree->maxs);
            ++surfNodeIndex;
            ++surf;
        }
    }
    return totalTreesUsed;
}

void __cdecl R_LoadAabbTrees(TrisType trisType)
{
    GfxAabbTree *out; // [esp+4h] [ebp-18h]
    unsigned int aabbTreeIndex; // [esp+8h] [ebp-14h]
    unsigned int aabbTreeIndexa; // [esp+8h] [ebp-14h]
    unsigned int aabbTreeCount; // [esp+Ch] [ebp-10h] BYREF
    int surfaceCount; // [esp+10h] [ebp-Ch]
    const DiskGfxAabbTree *in; // [esp+14h] [ebp-8h]
    LumpType lumpType; // [esp+18h] [ebp-4h]

    lumpType = trisType != TRIS_TYPE_LAYERED ? LUMP_UNLAYERED_AABBTREES : LUMP_AABBTREES;
    in = (const DiskGfxAabbTree *)Com_GetBspLump(lumpType, 0xCu, &aabbTreeCount);
    out = (GfxAabbTree *)Hunk_Alloc(44 * aabbTreeCount, "R_LoadAabbTrees", 22);
    rgl.aabbTrees = out;
    rgl.aabbTreeCount = aabbTreeCount;
    for (aabbTreeIndex = 0; aabbTreeIndex < aabbTreeCount; ++aabbTreeIndex)
    {
        surfaceCount = in[aabbTreeIndex].surfaceCount;
        if (surfaceCount)
        {
            out[aabbTreeIndex].startSurfIndex = in[aabbTreeIndex].firstSurface;
            if (out[aabbTreeIndex].startSurfIndex != in[aabbTreeIndex].firstSurface)
                MyAssertHandler(
                    ".\\r_bsp_load_obj.cpp",
                    3633,
                    0,
                    "%s",
                    "out[aabbTreeIndex].startSurfIndex == in[aabbTreeIndex].firstSurface");
        }
        else
        {
            out[aabbTreeIndex].startSurfIndex = 0;
        }
        out[aabbTreeIndex].surfaceCount = surfaceCount;
        iassert( out[aabbTreeIndex].surfaceCount == surfaceCount );
        out[aabbTreeIndex].childCount = in[aabbTreeIndex].childCount;
        if (out[aabbTreeIndex].childCount != in[aabbTreeIndex].childCount)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                3644,
                0,
                "%s",
                "out[aabbTreeIndex].childCount == in[aabbTreeIndex].childCount");
    }
    for (aabbTreeIndexa = 0;
        aabbTreeIndexa < aabbTreeCount;
        aabbTreeIndexa = R_FinishLoadingAabbTrees_r(&rgl.aabbTrees[aabbTreeIndexa], aabbTreeIndexa + 1))
    {
        ;
    }
}

void __cdecl R_LoadCells(unsigned int bspVersion, TrisType trisType)
{
    int *v2; // [esp+0h] [ebp-18h]
    GfxCell *out; // [esp+4h] [ebp-14h]
    unsigned int cellIndex; // [esp+8h] [ebp-10h]
    char *in; // [esp+Ch] [ebp-Ch]
    int cullGroupCount; // [esp+10h] [ebp-8h]
    unsigned int cellCount; // [esp+14h] [ebp-4h] BYREF

    if (bspVersion > 0xE)
    {
        if (bspVersion > 0x15)
            in = Com_GetBspLump(LUMP_CELLS, 0x70u, &cellCount);
        else
            in = Com_GetBspLump(LUMP_CELLS, 0x2Cu, &cellCount);
    }
    else
    {
        in = Com_GetBspLump(LUMP_CELLS, 0x34u, &cellCount);
    }
    out = (GfxCell *)Hunk_Alloc(56 * cellCount, "R_LoadCells", 22);
    s_world.cells = out;
    s_world.dpvsPlanes.cellCount = cellCount;
    s_world.cellBitsCount = 16 * ((cellCount + 127) >> 7);
    for (cellIndex = 0; cellIndex < cellCount; ++cellIndex)
    {
        out->mins[0] = *(float *)in;
        out->mins[1] = *((float *)in + 1);
        out->mins[2] = *((float *)in + 2);
        out->maxs[0] = *((float *)in + 3);
        out->maxs[1] = *((float *)in + 4);
        out->maxs[2] = *((float *)in + 5);
        out->aabbTree = &rgl.aabbTrees[*(unsigned __int16 *)&in[2 * trisType + 24]];
        out->portals = (GfxPortal *)(68 * *((_DWORD *)in + 7));
        out->portalCount = *((_DWORD *)in + 8);
        cullGroupCount = *((_DWORD *)in + 10);
        if (cullGroupCount)
            v2 = &rgl.cullGroupIndices[*((_DWORD *)in + 9)];
        else
            v2 = 0;
        out->cullGroups = v2;
        out->cullGroupCount = cullGroupCount;
        out->reflectionProbeCount = 0;
        out->reflectionProbes = 0;
        if (bspVersion > 0xE)
        {
            if (bspVersion > 0x15)
            {
                out->reflectionProbeCount = in[44];
                if (out->reflectionProbeCount)
                {
                    out->reflectionProbes = Hunk_Alloc((unsigned __int8)in[44], "R_LoadCells", 22);
                    memcpy(out->reflectionProbes, (unsigned __int8 *)in + 45, (unsigned __int8)in[44]);
                }
                else
                {
                    out->reflectionProbes = Hunk_Alloc(1u, "R_LoadCells", 22);
                    out->reflectionProbeCount = 1;
                    *out->reflectionProbes = 0;
                }
                in += 112;
            }
            else
            {
                in += 44;
            }
        }
        else
        {
            in += 52;
        }
        ++out;
    }
}

unsigned int R_LoadPortals()
{
    unsigned int result; // eax
    GfxPortal *v1; // [esp+4h] [ebp-28h]
    DpvsPlane *p_plane; // [esp+10h] [ebp-1Ch]
    GfxPortal *out; // [esp+14h] [ebp-18h]
    cplane_s *plane; // [esp+18h] [ebp-14h]
    int cellIndex; // [esp+1Ch] [ebp-10h]
    char *in; // [esp+20h] [ebp-Ch]
    unsigned int portalIndex; // [esp+24h] [ebp-8h]
    unsigned int portalCount; // [esp+28h] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_PORTALS, 0x10u, &portalCount);
    out = (GfxPortal *)Hunk_Alloc(68 * portalCount, "R_LoadPortals", 22);
    iassert( s_world.cells );
    iassert( s_world.dpvsPlanes.planes );
    for (portalIndex = 0; ; ++portalIndex)
    {
        result = portalIndex;
        if (portalIndex >= portalCount)
            break;
        plane = &s_world.dpvsPlanes.planes[*(_DWORD *)&in[16 * portalIndex]];
        p_plane = &out[portalIndex].plane;
        p_plane->coeffs[0] = plane->normal[0];
        p_plane->coeffs[1] = plane->normal[1];
        p_plane->coeffs[2] = plane->normal[2];
        out[portalIndex].plane.coeffs[3] = -plane->dist;
        out[portalIndex].plane.side[0] = COERCE_INT(p_plane->coeffs[0]) <= 0 ? 0 : 0xC;
        p_plane->side[1] = COERCE_INT(out[portalIndex].plane.coeffs[1]) <= 0 ? 4 : 16;
        p_plane->side[2] = COERCE_INT(out[portalIndex].plane.coeffs[2]) <= 0 ? 8 : 20;
        if (*(_DWORD *)&in[16 * portalIndex + 4] >= s_world.dpvsPlanes.cellCount)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                3785,
                0,
                "in[portalIndex].cellIndex doesn't index s_world.dpvsPlanes.cellCount\n\t%i not in [0, %i)",
                *(_DWORD *)&in[16 * portalIndex + 4],
                s_world.dpvsPlanes.cellCount);
        out[portalIndex].cell = &s_world.cells[*(_DWORD *)&in[16 * portalIndex + 4]];
        iassert( rgl.portalVerts );
        out[portalIndex].vertices = (float (*)[3])rgl.portalVerts[*(_DWORD *)&in[16 * portalIndex + 8]];
        out[portalIndex].vertexCount = in[16 * portalIndex + 12];
        PerpendicularVector(plane->normal, out[portalIndex].hullAxis[0]);
        Vec3Cross(plane->normal, out[portalIndex].hullAxis[0], out[portalIndex].hullAxis[1]);
    }
    for (cellIndex = 0; cellIndex < s_world.dpvsPlanes.cellCount; ++cellIndex)
    {
        if (s_world.cells[cellIndex].portalCount)
            v1 = &out[(int)s_world.cells[cellIndex].portals / 68];
        else
            v1 = 0;
        s_world.cells[cellIndex].portals = v1;
        result = cellIndex + 1;
    }
    return result;
}

void __cdecl R_SetParentAndCell_r(mnode_load_t *node)
{
    int cellIndex; // [esp+0h] [ebp-8h]

    if (node - rgl.nodes < rgl.nodeCount)
    {
        R_SetParentAndCell_r(&rgl.nodes[node->children[0]]);
        R_SetParentAndCell_r(&rgl.nodes[node->children[1]]);
        node->cellIndex = -2;
        cellIndex = rgl.nodes[node->children[0]].cellIndex;
        if (cellIndex == rgl.nodes[node->children[1]].cellIndex)
            node->cellIndex = cellIndex;
    }
}

unsigned int __cdecl R_CountNodes_r(mnode_load_t *node)
{
    unsigned int v2; // esi

    if (node->cellIndex != -2)
        return 1;
    v2 = R_CountNodes_r(&rgl.nodes[node->children[0]]);
    return v2 + R_CountNodes_r(&rgl.nodes[node->children[1]]) + 2;
}

mnode_t *__cdecl R_SortNodes_r(mnode_load_t *node, mnode_t *out)
{
    mnode_t *outb; // [esp+14h] [ebp+Ch]

    if (node->cellIndex == -2)
    {
        out->cellIndex = LOWORD(s_world.dpvsPlanes.cellCount) + node->planeIndex + 1;

        if (out->cellIndex != s_world.dpvsPlanes.cellCount + node->planeIndex + 1)
            Com_Error(ERR_DROP, "Max planes exceeded");

        outb = R_SortNodes_r(&rgl.nodes[node->children[0]], out + 1);

        out->rightChildOffset = ((char*)outb - (char*)out) / 2;

        // lwss: this can't be hit
//        if (out->rightChildOffset != (outb - out) >> 1)
//            Com_Error(ERR_DROP, "Max cells exceeded");

        return R_SortNodes_r(&rgl.nodes[node->children[1]], outb);
    }
    else
    {
        out->cellIndex = node->cellIndex + 1;
        iassert( out->cellIndex == node->cellIndex + 1 );
        return (mnode_t *)&out->rightChildOffset;
    }
}

void __cdecl R_LoadNodesAndLeafs(unsigned int bspVersion)
{
    char *inNode; // [esp+0h] [ebp-30h]
    mnode_load_t *out; // [esp+4h] [ebp-2Ch]
    unsigned int nodeIndex; // [esp+8h] [ebp-28h]
    unsigned int leafIndex; // [esp+Ch] [ebp-24h]
    unsigned int leafIndexa; // [esp+Ch] [ebp-24h]
    int nodeOrLeafIndex; // [esp+10h] [ebp-20h]
    int childIndex; // [esp+18h] [ebp-18h]
    char *inLeaf_v14; // [esp+1Ch] [ebp-14h]
    unsigned int leafCount; // [esp+20h] [ebp-10h] BYREF
    unsigned int nodeCount; // [esp+24h] [ebp-Ch] BYREF
    const DiskLeaf *inLeaf; // [esp+28h] [ebp-8h]
    int totalNodeCount; // [esp+2Ch] [ebp-4h]

    inNode = Com_GetBspLump(LUMP_NODES, 0x24u, &nodeCount);
    if (bspVersion > 0xE)
    {
        inLeaf = (const DiskLeaf *)Com_GetBspLump(LUMP_LEAFS, 0x18u, &leafCount);
        inLeaf_v14 = 0;
    }
    else
    {
        inLeaf_v14 = Com_GetBspLump(LUMP_LEAFS, 0x24u, &leafCount);
        inLeaf = 0;
    }
    totalNodeCount = leafCount + nodeCount;
    out = (mnode_load_t *)Z_Malloc(16 * (leafCount + nodeCount), "R_LoadNodesAndLeafs", 22);
    rgl.nodes = out;
    rgl.nodeCount = nodeCount;
    for (nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
    {
        out->planeIndex = *(_DWORD *)inNode;
        for (childIndex = 0; childIndex < 2; ++childIndex)
        {
            nodeOrLeafIndex = *(_DWORD *)&inNode[4 * childIndex + 4];
            if (nodeOrLeafIndex < 0)
                out->children[childIndex] = nodeCount + -1 - nodeOrLeafIndex;
            else
                out->children[childIndex] = nodeOrLeafIndex;
        }
        inNode += 36;
        ++out;
    }
    if (bspVersion > 0xE)
    {
        for (leafIndexa = 0; leafIndexa < leafCount; ++leafIndexa)
        {
            out->cellIndex = SLOWORD(inLeaf->cellNum);
            iassert( out->cellIndex == inLeaf->cellNum );
            ++inLeaf;
            ++out;
        }
    }
    else
    {
        iassert(0); // lwss test add
        for (leafIndex = 0; leafIndex < leafCount; ++leafIndex)
        {
            out->cellIndex = *((__int16 *)inLeaf_v14 + 12);
            //iassert( out->cellIndex == inLeaf_v14->cellNum );
            inLeaf_v14 += 36;
            ++out;
        }
    }
    R_SetParentAndCell_r(rgl.nodes);
    s_world.nodeCount = R_CountNodes_r(rgl.nodes);
    s_world.dpvsPlanes.nodes = (unsigned __int16 *)Hunk_Alloc(16 * s_world.nodeCount, "R_LoadNodesAndLeafs", 22);

    mnode_t *out2 = R_SortNodes_r(rgl.nodes, (mnode_t *)s_world.dpvsPlanes.nodes);
    iassert(reinterpret_cast<ushort *>(out2) - s_world.dpvsPlanes.nodes == s_world.nodeCount);

    Z_Free((char *)rgl.nodes, 22);
}

BOOL __cdecl R_CompareSurfaces(const GfxSurface &surf0, const GfxSurface &surf1)
{
    const MaterialTechnique *techniqueEmissive; // [esp+28h] [ebp-64h]
    int surfIndex; // [esp+30h] [ebp-5Ch]
    int surfIndex_4; // [esp+34h] [ebp-58h]
    const MaterialTechnique *techniqueLit; // [esp+38h] [ebp-54h]
    Material *material[2]; // [esp+40h] [ebp-4Ch]
    int firstVertex; // [esp+48h] [ebp-44h]
    int firstVertex_4; // [esp+4Ch] [ebp-40h]
    int reflectionProbeIndex; // [esp+58h] [ebp-34h]
    int reflectionProbeIndex_4; // [esp+5Ch] [ebp-30h]
    int hasTechniqueLit; // [esp+70h] [ebp-1Ch]
    int hasTechniqueLit_4; // [esp+74h] [ebp-18h]
    int lightmapIndex; // [esp+78h] [ebp-14h]
    int lightmapIndex_4; // [esp+7Ch] [ebp-10h]
    MaterialTechniqueSet *techSet[2]; // [esp+80h] [ebp-Ch]
    int comparison; // [esp+88h] [ebp-4h]

    material[0] = surf0.material;
    material[1] = surf1.material;
    techSet[0] = Material_GetTechniqueSet(material[0]);
    techSet[1] = Material_GetTechniqueSet(material[1]);
    iassert( techSet[0] && techSet[1] );
    techniqueLit = Material_GetTechnique(material[0], TECHNIQUE_LIT_BEGIN);
    hasTechniqueLit = techniqueLit != 0;
    hasTechniqueLit_4 = Material_GetTechnique(material[1], TECHNIQUE_LIT_BEGIN) != 0;
    if (hasTechniqueLit_4 != hasTechniqueLit)
        return hasTechniqueLit_4 - hasTechniqueLit < 0;
    if (!techniqueLit)
    {
        techniqueEmissive = Material_GetTechnique(material[0], TECHNIQUE_EMISSIVE);
        comparison = (Material_GetTechnique(material[1], TECHNIQUE_EMISSIVE) != 0) - (techniqueEmissive != 0);
        if (comparison)
            return comparison < 0;
    }
    comparison = (material[0]->info.drawSurf.fields.primarySortKey - material[1]->info.drawSurf.fields.primarySortKey);
    if (comparison)
        return comparison < 0;
    Com_GetPrimaryLight(surf0.primaryLightIndex);
    Com_GetPrimaryLight(surf1.primaryLightIndex);
    comparison = surf0.primaryLightIndex - surf1.primaryLightIndex;
    if (comparison)
        return comparison < 0;
    comparison = (material[0]->info.drawSurf.fields.materialSortedIndex - material[1]->info.drawSurf.fields.materialSortedIndex);
    if (comparison)
    {
        iassert( surf0.tris.firstVertex != surf1.tris.firstVertex );
        return comparison < 0;
    }
    else
    {
        iassert( material[0] == material[1] );
        reflectionProbeIndex = surf0.reflectionProbeIndex;
        reflectionProbeIndex_4 = surf1.reflectionProbeIndex;
        if (reflectionProbeIndex == reflectionProbeIndex_4)
        {
            lightmapIndex = surf0.lightmapIndex;
            lightmapIndex_4 = surf1.lightmapIndex;
            if (lightmapIndex == lightmapIndex_4)
            {
                firstVertex = surf0.tris.firstVertex;
                firstVertex_4 = surf1.tris.firstVertex;
                if (firstVertex == firstVertex_4)
                {
                    surfIndex = surf0.tris.vertexCount;
                    surfIndex_4 = surf1.tris.vertexCount;
                    //iassert( comparison ); // var optimized out (surfIndex == surfIndex_4)
                    return surfIndex - surfIndex_4 < 0;
                }
                else
                {
                    return firstVertex - firstVertex_4 < 0;
                }
            }
            else
            {
                return lightmapIndex - lightmapIndex_4 < 0;
            }
        }
        else
        {
            return reflectionProbeIndex - reflectionProbeIndex_4 < 0;
        }
    }
}

unsigned int R_SortSurfaces()
{
    unsigned int result; // eax
    int origSurfIndex; // [esp+68h] [ebp-14h]
    int surfIndex; // [esp+6Ch] [ebp-10h]
    int surfIndexa; // [esp+6Ch] [ebp-10h]
    signed int surfIndexb; // [esp+6Ch] [ebp-10h]
    int surfaceCount; // [esp+70h] [ebp-Ch]
    int surfaceCounta; // [esp+70h] [ebp-Ch]
    GfxSurface *surface; // [esp+78h] [ebp-4h]

    if (s_world.modelCount <= 0)
        MyAssertHandler(
            ".\\r_bsp_load_obj.cpp",
            2144,
            0,
            "%s\n\t(s_world.modelCount) = %i",
            "(s_world.modelCount > 0)",
            s_world.modelCount);
    if (s_world.models->startSurfIndex)
        MyAssertHandler(
            ".\\r_bsp_load_obj.cpp",
            2145,
            0,
            "%s\n\t(s_world.models[0].startSurfIndex) = %i",
            "(s_world.models[0].startSurfIndex == 0)",
            s_world.models->startSurfIndex);
    surfaceCount = s_world.models->surfaceCount;
    if (s_world.models->surfaceCount)
        s_world.dpvs.sortedSurfIndex = (unsigned short*)Hunk_Alloc(4 * surfaceCount, "R_InitDynamicData", 20);
    else
        s_world.dpvs.sortedSurfIndex = 0;
    for (surfIndex = 0; surfIndex < surfaceCount; ++surfIndex)
    {
        surface = &s_world.dpvs.surfaces[surfIndex];
        s_world.dpvs.sortedSurfIndex[surfIndex] = surface->tris.vertexCount;
        surface->tris.vertexCount = surfIndex;
        iassert( surface->tris.vertexCount == surfIndex );
    }
    //std::_Sort<GfxSurface *, int, bool(__cdecl *)(GfxSurface const &, GfxSurface const &)>(
    //    s_world.dpvs.surfaces,
    //    &s_world.dpvs.surfaces[surfaceCount],
    //    48 * surfaceCount / 48,
    //    R_CompareSurfaces);
    std::sort(&s_world.dpvs.surfaces[0], &s_world.dpvs.surfaces[surfaceCount], R_CompareSurfaces);

    for (surfIndexa = 0; surfIndexa < surfaceCount; ++surfIndexa)
    {
        origSurfIndex = s_world.dpvs.surfaces[surfIndexa].tris.vertexCount;
        s_world.dpvs.surfaces[surfIndexa].tris.vertexCount = s_world.dpvs.sortedSurfIndex[origSurfIndex];
        s_world.dpvs.sortedSurfIndex[origSurfIndex] = surfIndexa;
        if (s_world.dpvs.sortedSurfIndex[origSurfIndex] != surfIndexa)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                2179,
                0,
                "%s",
                "s_world.dpvs.sortedSurfIndex[origSurfIndex] == surfIndex");
    }
    surfIndexb = 0;
    surfaceCounta = s_world.models->surfaceCount;
    s_world.dpvs.litSurfsBegin = 0;
    while (surfIndexb < surfaceCounta
        && s_world.dpvs.surfaces[surfIndexb].material->techniqueSet
        && Material_GetTechnique(s_world.dpvs.surfaces[surfIndexb].material, TECHNIQUE_LIT_BEGIN)
        && s_world.dpvs.surfaces[surfIndexb].material->info.sortKey < 0x18u)
        ++surfIndexb;
    s_world.dpvs.litSurfsEnd = surfIndexb;
    result = surfIndexb;
    s_world.dpvs.decalSurfsBegin = surfIndexb;
    while (surfIndexb < surfaceCounta)
    {
        result = 48 * surfIndexb;
        if (!s_world.dpvs.surfaces[surfIndexb].material->techniqueSet)
            break;
        result = (uint)Material_GetTechnique(s_world.dpvs.surfaces[surfIndexb].material, TECHNIQUE_LIT_BEGIN);
        if (!result)
            break;
        result = s_world.dpvs.surfaces[surfIndexb].material->info.sortKey;
        if (result < 0x18)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                2378,
                0,
                "%s",
                "s_world.dpvs.surfaces[surfIndex].material->info.sortKey >= SORTKEY_DECAL");
        ++surfIndexb;
    }
    s_world.dpvs.decalSurfsEnd = surfIndexb;
    s_world.dpvs.emissiveSurfsBegin = surfIndexb;
    while (surfIndexb < surfaceCounta)
    {
        result = (uint)s_world.dpvs.surfaces;
        if (!s_world.dpvs.surfaces[surfIndexb].material->techniqueSet)
            break;
        result = (uint)Material_GetTechnique(s_world.dpvs.surfaces[surfIndexb].material, TECHNIQUE_EMISSIVE);
        if (!result)
            break;
        result = ++surfIndexb;
    }
    s_world.dpvs.emissiveSurfsEnd = surfIndexb;
    return result;
}

char __cdecl R_DoWorldTrisCoincide(const float **xyz0, const float **xyz1)
{
    char v3; // [esp+0h] [ebp-78h]
    BOOL v5; // [esp+8h] [ebp-70h]
    char v7; // [esp+10h] [ebp-68h]
    BOOL v9; // [esp+18h] [ebp-60h]
    char v11; // [esp+20h] [ebp-58h]
    BOOL v13; // [esp+28h] [ebp-50h]
    const float *v15; // [esp+30h] [ebp-48h]
    const float *v16; // [esp+34h] [ebp-44h]
    int v17; // [esp+38h] [ebp-40h]
    const float *v18; // [esp+3Ch] [ebp-3Ch]
    const float *v19; // [esp+40h] [ebp-38h]
    int v20; // [esp+44h] [ebp-34h]
    int v21; // [esp+48h] [ebp-30h]
    const float *v22; // [esp+4Ch] [ebp-2Ch]
    const float *v23; // [esp+50h] [ebp-28h]
    const float *v24; // [esp+54h] [ebp-24h]
    const float *v25; // [esp+58h] [ebp-20h]
    int v26; // [esp+5Ch] [ebp-1Ch]
    const float *v27; // [esp+60h] [ebp-18h]
    const float *v28; // [esp+64h] [ebp-14h]
    const float *v29; // [esp+68h] [ebp-10h]
    const float *v30; // [esp+6Ch] [ebp-Ch]
    int v31; // [esp+70h] [ebp-8h]
    int v32; // [esp+74h] [ebp-4h]

    v31 = *(int *)xyz1;
    v32 = *(int *)xyz0;
    if (**xyz1 == **xyz0 && *(int*)(v31 + 4) == *(int*)(v32 + 4) && *(int *)(v31 + 8) == *(int *)(v32 + 8))
    {
        v29 = xyz1[1];
        v30 = xyz0[1];
        v13 = *v29 == *v30 && v29[1] == v30[1] && v29[2] == v30[2];
        v11 = 0;
        if (v13)
        {
            v27 = xyz1[2];
            v28 = xyz0[2];
            if (*v27 == *v28 && v27[1] == v28[1] && v27[2] == v28[2])
                return 1;
        }
        return v11;
    }
    else
    {
        v25 = xyz1[1];
        v26 = *(int *)xyz0;
        if (*v25 == **xyz0 && v25[1] == *(int *)(v26 + 4) && v25[2] == *(int *)(v26 + 8))
        {
            v23 = xyz1[2];
            v24 = xyz0[1];
            v9 = *v23 == *v24 && v23[1] == v24[1] && v23[2] == v24[2];
            v7 = 0;
            if (v9)
            {
                v21 = *(int *)xyz1;
                v22 = xyz0[2];
                if (**xyz1 == *v22 && *(int *)(v21 + 4) == v22[1] && *(int *)(v21 + 8) == v22[2])
                    return 1;
            }
            return v7;
        }
        else
        {
            v19 = xyz1[2];
            v20 = *(int *)xyz0;
            if (*v19 == **xyz0 && v19[1] == *(int *)(v20 + 4) && v19[2] == *(int *)(v20 + 8))
            {
                v17 = *(int *)xyz1;
                v18 = xyz0[1];
                v5 = **xyz1 == *v18 && *(int *)(v17 + 4) == v18[1] && *(int *)(v17 + 8) == v18[2];
                v3 = 0;
                if (v5)
                {
                    v15 = xyz1[1];
                    v16 = xyz0[2];
                    if (*v15 == *v16 && v15[1] == v16[1] && v15[2] == v16[2])
                        return 1;
                }
                return v3;
            }
            else
            {
                return 0;
            }
        }
    }
}

char __cdecl R_DoesTriCoverAnyOtherTri(
    unsigned int modelSurfIndexBegin,
    unsigned int modelSurfIndexEnd,
    unsigned int firstVertex,
    unsigned int baseIndex,
    unsigned int materialSortedIndex)
{
    unsigned int surfFirstVertex; // [esp+18h] [ebp-44h]
    float mins[3]; // [esp+1Ch] [ebp-40h] BYREF
    unsigned int surfBaseIndex; // [esp+28h] [ebp-34h]
    float maxs[3]; // [esp+2Ch] [ebp-30h] BYREF
    const GfxSurface *surf; // [esp+38h] [ebp-24h]
    unsigned int surfIter; // [esp+3Ch] [ebp-20h]
    unsigned int triIter; // [esp+40h] [ebp-1Ch]
    const float *xyzRef[3]; // [esp+44h] [ebp-18h] BYREF
    const float *xyzSurf[3]; // [esp+50h] [ebp-Ch] BYREF

    xyzRef[0] = s_world.vd.vertices[firstVertex + s_world.indices[baseIndex]].xyz;
    xyzRef[1] = s_world.vd.vertices[firstVertex + s_world.indices[baseIndex + 1]].xyz;
    xyzRef[2] = s_world.vd.vertices[firstVertex + s_world.indices[baseIndex + 2]].xyz;
    ClearBounds(mins, maxs);
    AddPointToBounds(xyzRef[0], mins, maxs);
    AddPointToBounds(xyzRef[1], mins, maxs);
    AddPointToBounds(xyzRef[2], mins, maxs);
    for (surfIter = modelSurfIndexBegin; surfIter != modelSurfIndexEnd; ++surfIter)
    {
        surf = &s_world.dpvs.surfaces[surfIter];
        if (materialSortedIndex > surf->material->info.drawSurf.fields.materialSortedIndex
            && BoundsOverlap(surf->bounds[0], surf->bounds[1], mins, maxs))
        {
            surfFirstVertex = surf->tris.firstVertex;
            for (triIter = 0; triIter < surf->tris.triCount; ++triIter)
            {
                surfBaseIndex = surf->tris.baseIndex + 3 * triIter;
                xyzSurf[0] = s_world.vd.vertices[surfFirstVertex + s_world.indices[surfBaseIndex]].xyz;
                xyzSurf[1] = s_world.vd.vertices[surfFirstVertex + s_world.indices[surfBaseIndex + 1]].xyz;
                xyzSurf[2] = s_world.vd.vertices[surfFirstVertex + s_world.indices[surfBaseIndex + 2]].xyz;
                if (R_DoWorldTrisCoincide(xyzRef, xyzSurf))
                    return 1;
            }
        }
    }
    return 0;
}

char __cdecl R_IsSurfaceDecalLayer(
    unsigned int surfIndex,
    unsigned int modelSurfIndexBegin,
    unsigned int modelSurfIndexEnd)
{
    unsigned int materialSortedIndex; // [esp+0h] [ebp-Ch]
    const GfxSurface *surf; // [esp+4h] [ebp-8h]
    unsigned int triIter; // [esp+8h] [ebp-4h]

    surf = &s_world.dpvs.surfaces[surfIndex];
    materialSortedIndex = surf->material->info.drawSurf.fields.materialSortedIndex;
    for (triIter = 0; triIter < surf->tris.triCount; ++triIter)
    {
        if (!R_DoesTriCoverAnyOtherTri(
            modelSurfIndexBegin,
            modelSurfIndexEnd,
            surf->tris.firstVertex,
            surf->tris.baseIndex,
            materialSortedIndex))
            return 0;
    }
    return 1;
}

unsigned int __cdecl R_BuildNoDecalAabbTree_r(GfxAabbTree *tree, unsigned int startSurfIndex)
{
    int startSurfIndexNoDecal; // eax
    int v4; // [esp+0h] [ebp-18h]
    unsigned __int16 childIter; // [esp+4h] [ebp-14h]
    GfxAabbTree *children; // [esp+8h] [ebp-10h]
    unsigned __int16 surfIndex; // [esp+Ch] [ebp-Ch]
    const unsigned __int16 *srcIndices; // [esp+10h] [ebp-8h]
    unsigned __int16 surfIter; // [esp+14h] [ebp-4h]

    if (startSurfIndex != startSurfIndex)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
            281,
            0,
            "i == static_cast< Type >( i )\n\t%i, %i",
            startSurfIndex,
            startSurfIndex);
    tree->startSurfIndexNoDecal = startSurfIndex;
    if (tree->childCount)
    {
        children = (tree + tree->childrenOffset);
        for (childIter = 0; childIter < tree->childCount; ++childIter)
            startSurfIndex = R_BuildNoDecalAabbTree_r(&children[childIter], startSurfIndex);
    }
    else
    {
        srcIndices = &s_world.dpvs.sortedSurfIndex[tree->startSurfIndex];
        for (surfIter = 0; surfIter < tree->surfaceCount; ++surfIter)
        {
            surfIndex = srcIndices[surfIter];
            if ((s_world.dpvs.surfaces[surfIndex].flags & 2) == 0)
                s_world.dpvs.sortedSurfIndex[startSurfIndex++] = surfIndex;
        }
    }
    startSurfIndexNoDecal = tree->startSurfIndexNoDecal;
    v4 = startSurfIndex - startSurfIndexNoDecal;
    if (startSurfIndex - startSurfIndexNoDecal != (startSurfIndex - startSurfIndexNoDecal))
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
            281,
            0,
            "i == static_cast< Type >( i )\n\t%i, %i",
            v4,
            v4);
    tree->surfaceCountNoDecal = v4;
    return startSurfIndex;
}

void R_BuildNoDecalSubModels()
{
    unsigned int modelSurfIndexBegin; // [esp+0h] [ebp-20h]
    unsigned int surfIndex; // [esp+4h] [ebp-1Ch]
    GfxBrushModel *model; // [esp+8h] [ebp-18h]
    unsigned int modelSurfIndexEnd; // [esp+Ch] [ebp-14h]
    int cellIter; // [esp+10h] [ebp-10h]
    unsigned int surfIter; // [esp+14h] [ebp-Ch]
    int modelIndex; // [esp+18h] [ebp-8h]
    unsigned int startSurfIndex; // [esp+1Ch] [ebp-4h]

    for (modelIndex = 0; modelIndex < s_world.modelCount; ++modelIndex)
    {
        model = &s_world.models[modelIndex];
        modelSurfIndexBegin = model->startSurfIndex;
        modelSurfIndexEnd = model->surfaceCount + modelSurfIndexBegin;
        model->surfaceCountNoDecal = 0;
        for (surfIter = 0; surfIter < model->surfaceCount; ++surfIter)
        {
            surfIndex = surfIter + model->startSurfIndex;
            if (R_IsSurfaceDecalLayer(surfIndex, modelSurfIndexBegin, modelSurfIndexEnd))
                s_world.dpvs.surfaces[surfIndex].flags |= 2u;
            else
                ++model->surfaceCountNoDecal;
        }
    }
    startSurfIndex = s_world.models->surfaceCount;
    for (cellIter = 0; cellIter < s_world.dpvsPlanes.cellCount; ++cellIter)
        startSurfIndex = R_BuildNoDecalAabbTree_r(s_world.cells[cellIter].aabbTree, startSurfIndex);
    if (s_world.models->surfaceCountNoDecal != startSurfIndex - s_world.models->surfaceCount)
        MyAssertHandler(
            ".\\r_bsp_load_obj.cpp",
            2579,
            1,
            "s_world.models[0].surfaceCountNoDecal == startSurfIndex - s_world.models[0].surfaceCount\n\t%i, %i",
            s_world.models->surfaceCountNoDecal,
            startSurfIndex - s_world.models->surfaceCount);
}

char *__cdecl R_ValueForKey(const char *key, char *(*spawnVars)[2], int spawnVarCount)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 1; i < spawnVarCount; ++i)
    {
        if (!I_stricmp((*spawnVars)[2 * i], key))
            return (*spawnVars)[2 * i + 1];
    }
    return 0;
}

bool __cdecl R_VectorForKey(const char *key, char *defaultString, char *(*spawnVars)[2], int spawnVarCount, float *v)
{
    char *string; // [esp+0h] [ebp-8h]
    bool success; // [esp+7h] [ebp-1h]

    iassert( defaultString );
    success = 1;
    string = R_ValueForKey(key, spawnVars, spawnVarCount);
    if (!string)
    {
        success = 0;
        string = defaultString;
    }
    *v = 0.0;
    v[1] = 0.0;
    v[2] = 0.0;
    sscanf(string, "%f %f %f", v, v + 1, v + 2);
    return success;
}

static void __cdecl R_CheckValidStaticModel(char *(*spawnVars)[2], int spawnVarCount, XModel **model, float *origin)
{
    bool v4; // [esp+1Bh] [ebp-15h]
    char *modelName; // [esp+1Ch] [ebp-14h]
    float tempOrigin[3]; // [esp+20h] [ebp-10h] BYREF
    XModel *tempModel; // [esp+2Ch] [ebp-4h]

    if (!R_VectorForKey("origin", (char*)"0 0 0", (char *(*)[2])spawnVars, spawnVarCount, tempOrigin))
        Com_Error(ERR_DROP, "R_CheckValidStaticModel: no origin specified");
    modelName = R_ValueForKey("model", spawnVars, spawnVarCount);
    if (!modelName)
        Com_Error(ERR_DROP, "R_CheckValidStaticModel: no model specified in misc_model at (%.0f %.0f %.0f)", tempOrigin[0], tempOrigin[1], tempOrigin[2]);
    if (Com_IsLegacyXModelName(modelName))
        modelName += 7;
    tempModel = R_RegisterModel(modelName);
    if (tempModel)
        v4 = XModelBad(tempModel);
    else
        v4 = 1;
    if (v4)
    {
        Com_PrintError(
            8,
            "bad static model '%s' at (%.0f %.0f %.0f)\n",
            modelName,
            tempOrigin[0],
            tempOrigin[1],
            tempOrigin[2]);
        tempModel = R_RegisterModel("default_static_model");
        if (!tempModel || XModelBad(tempModel))
            Com_Error(ERR_DROP, "R_CheckValidStaticModel: could not find xmodel 'default_static_model'");
    }
    iassert( model );
    *model = tempModel;
    iassert( origin );
    *origin = tempOrigin[0];
    origin[1] = tempOrigin[1];
    origin[2] = tempOrigin[2];
}

double __cdecl R_FloatForKey(const char *key, float defaultValue, char *(*spawnVars)[2], int spawnVarCount)
{
    char *string; // [esp+4h] [ebp-4h]

    string = R_ValueForKey(key, spawnVars, spawnVarCount);
    if (!string)
        return defaultValue;
    return atof(string);
}

int __cdecl R_IntForKey(const char *key, int defaultValue, char *(*spawnVars)[2], int spawnVarCount)
{
    char *string; // [esp+0h] [ebp-4h]

    string = R_ValueForKey(key, spawnVars, spawnVarCount);
    if (string)
        return atoi(string);
    else
        return defaultValue;
}

void __cdecl R_GetXModelBounds(XModel *model, const float (*axes)[3], float *mins, float *maxs)
{
    float coord; // [esp+0h] [ebp-24h]
    int surfaceCount; // [esp+4h] [ebp-20h]
    int axisIndex; // [esp+8h] [ebp-1Ch]
    XSurface *surfaces; // [esp+Ch] [ebp-18h] BYREF
    int index; // [esp+10h] [ebp-14h]
    XSurface *xsurf; // [esp+14h] [ebp-10h]
    int vertCount; // [esp+18h] [ebp-Ch]
    int vertIndex; // [esp+1Ch] [ebp-8h]
    float (*vert)[3]; // [esp+20h] [ebp-4h]

    *mins = FLT_MAX;
    mins[1] = FLT_MAX;
    mins[2] = FLT_MAX;
    *maxs = -FLT_MAX;
    maxs[1] = -FLT_MAX;
    maxs[2] = -FLT_MAX;
    surfaceCount = XModelGetSurfaces(model, &surfaces, 0);
    iassert( surfaces );
    vert = (float(*)[3])Hunk_AllocateTempMemory(393216, "R_GetXModelBounds");
    iassert( surfaceCount > 0 );
    for (index = 0; index < surfaceCount; ++index)
    {
        xsurf = &surfaces[index];
        vertCount = XSurfaceGetNumVerts(xsurf);
        XSurfaceGetVerts(xsurf, (float*)vert, 0, 0);
        iassert( vertCount > 0 );
        for (vertIndex = 0; vertIndex < vertCount; ++vertIndex)
        {
            for (axisIndex = 0; axisIndex < 3; ++axisIndex)
            {
                coord = vert[vertIndex][0] * (*axes)[axisIndex]
                    + vert[vertIndex][1] * (*axes)[axisIndex + 3]
                        + vert[vertIndex][2] * (*axes)[axisIndex + 6];
                    if (coord < mins[axisIndex])
                        mins[axisIndex] = coord;
                    if (coord > maxs[axisIndex])
                        maxs[axisIndex] = coord;
            }
        }
    }
    Hunk_FreeTempMemory((char*)vert);
}

void __cdecl R_CreateStaticModel(
    XModel *model,
    const float *origin,
    const float (*axis)[3],
    float scale,
    GfxStaticModelDrawInst *smodelDrawInst,
    GfxStaticModelInst *smodelInst,
    unsigned __int8 staticModelFlags)
{
    smodelDrawInst->model = model;
    smodelDrawInst->placement.origin[0] = *origin;
    smodelDrawInst->placement.origin[1] = origin[1];
    smodelDrawInst->placement.origin[2] = origin[2];
    AxisCopy(*(const mat3x3*)axis, smodelDrawInst->placement.axis);
    smodelDrawInst->placement.scale = scale;
    R_GetXModelBounds(model, axis, smodelInst->mins, smodelInst->maxs);
    Vec3Mad(origin, scale, smodelInst->mins, smodelInst->mins);
    Vec3Mad(origin, scale, smodelInst->maxs, smodelInst->maxs);
    iassert( scale );
    smodelDrawInst->cullDist = XModelGetLodOutDist(model) * scale;
    smodelDrawInst->reflectionProbeIndex = 0;
    smodelDrawInst->flags = staticModelFlags;
}

unsigned __int8 __cdecl XModelGetFlags(const XModel *model)
{
    return model->flags;
}

bool __cdecl R_DecodeGroundLighting(
    const char *key,
    const char *defaultString,
    char *(*spawnVars)[2],
    int spawnVarCount,
    int bspVersion,
    unsigned __int8 *outPrimaryLightIndex,
    unsigned __int8 *outValue)
{
    unsigned int valueInt[4]; // [esp+0h] [ebp-24h] BYREF
    const char *string; // [esp+10h] [ebp-14h]
    unsigned int primaryLightIndex; // [esp+14h] [ebp-10h] BYREF
    bool success; // [esp+1Bh] [ebp-9h]
    int fieldsRead; // [esp+1Ch] [ebp-8h]
    int dimIter; // [esp+20h] [ebp-4h]

    iassert( defaultString );
    success = 1;
    string = R_ValueForKey(key, spawnVars, spawnVarCount);
    if (!string)
    {
        success = 0;
        string = defaultString;
    }
    fieldsRead = sscanf(
        string,
        "%02x%02x%02x%02x%02x",
        &valueInt[2],
        &valueInt[1],
        valueInt,
        &valueInt[3],
        &primaryLightIndex);
    if (fieldsRead == 4)
    {
        primaryLightIndex = s_world.sunPrimaryLightIndex;
    }
    else if (fieldsRead == 5)
    {
        if (bspVersion <= 14)
        {
            if (primaryLightIndex == 255)
                primaryLightIndex = 0;
            else
                ++primaryLightIndex;
        }
    }
    else
    {
        Com_Error(ERR_DROP, "R_Vec4ForKeyHex: invalid value");
    }
    for (dimIter = 0; dimIter != 4; ++dimIter)
        outValue[dimIter] = valueInt[dimIter];
    *outPrimaryLightIndex = primaryLightIndex;
    return success;
}

static void __cdecl R_LoadMiscModel(char *(*spawnVars)[2], int spawnVarCount, int bspVersion)
{
    bool groundLightContainsValidData; // [esp+13h] [ebp-69h]
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+14h] [ebp-68h]
    float origin[3]; // [esp+18h] [ebp-64h] BYREF
    XModel *model; // [esp+24h] [ebp-58h] BYREF
    bool isModelGroundLit; // [esp+2Bh] [ebp-51h]
    GfxStaticModelInst *smodelInst; // [esp+2Ch] [ebp-50h]
    float angle; // [esp+30h] [ebp-4Ch]
    float angles[3]; // [esp+34h] [ebp-48h] BYREF
    int spawnflags; // [esp+40h] [ebp-3Ch]
    float scale; // [esp+44h] [ebp-38h]
    float lightingOrigin[3]; // [esp+48h] [ebp-34h] BYREF
    float axis[3][3]; // [esp+54h] [ebp-28h] BYREF
    unsigned __int8 staticModelFlags; // [esp+7Bh] [ebp-1h]

    R_CheckValidStaticModel(spawnVars, spawnVarCount, &model, origin);
    smodelDrawInst = &s_world.dpvs.smodelDrawInsts[s_world.dpvs.smodelCount];
    smodelInst = &s_world.dpvs.smodelInsts[s_world.dpvs.smodelCount++];
    angle = R_FloatForKey("angle", 0.0, spawnVars, spawnVarCount);
    if (angle == 0.0)
    {
        R_VectorForKey("angles", (char*)"0 0 0", spawnVars, spawnVarCount, angles);
    }
    else
    {
        angles[0] = 0.0;
        angles[1] = angle;
        angles[2] = 0.0;
    }
    AnglesToAxis(angles, axis);
    scale = R_FloatForKey("modelscale", 1.0, spawnVars, spawnVarCount);
    spawnflags = R_IntForKey("spawnflags", 0, spawnVars, spawnVarCount);
    staticModelFlags = 0;
    if ((spawnflags & 2) != 0)
        staticModelFlags |= 1u;
    R_CreateStaticModel(model, origin, axis, scale, smodelDrawInst, smodelInst, staticModelFlags);
    isModelGroundLit = (XModelGetFlags(model) & 1) != 0;
    groundLightContainsValidData = R_DecodeGroundLighting(
        "gndLt",
        "FF00000000",
        spawnVars,
        spawnVarCount,
        bspVersion,
        &smodelDrawInst->primaryLightIndex,
        (unsigned char*)&smodelInst->groundLighting);
    if (!isModelGroundLit || !groundLightContainsValidData || !smodelInst->groundLighting.packed)
    {
        smodelInst->groundLighting.packed = 0;
        smodelDrawInst->primaryLightIndex = R_GetPrimaryLightForModel(
            model,
            origin,
            axis,
            scale,
            smodelInst->mins,
            smodelInst->maxs,
            s_world.lightRegion);
        if (!smodelDrawInst->primaryLightIndex && s_world.sunPrimaryLightIndex == 1)
        {
            Vec3Avg(smodelInst->mins, smodelInst->maxs, lightingOrigin);
            smodelDrawInst->primaryLightIndex = R_GetPrimaryLightFromGrid(&s_world.lightGrid, lightingOrigin, 1u);
            if (smodelDrawInst->primaryLightIndex != 1)
                smodelDrawInst->primaryLightIndex = 0;
        }
        if (smodelDrawInst->primaryLightIndex >= s_world.primaryLightCount)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                3084,
                0,
                "smodelDrawInst->primaryLightIndex doesn't index s_world.primaryLightCount\n\t%i not in [0, %i)",
                smodelDrawInst->primaryLightIndex,
                s_world.primaryLightCount);
    }
    ++s_world.shadowGeom[smodelDrawInst->primaryLightIndex].smodelCount;
}

void __cdecl R_LoadEntities(unsigned int bspVersion)
{
    __int64 v1; // [esp-Ch] [ebp-278h]
    int spawnVarCount; // [esp+44h] [ebp-228h]
    char *startPos; // [esp+48h] [ebp-224h]
    char *spawnVars[64][2]; // [esp+4Ch] [ebp-220h] BYREF
    unsigned int smodelCount; // [esp+250h] [ebp-1Ch]
    unsigned int textLen; // [esp+254h] [ebp-18h] BYREF
    char *textPool; // [esp+258h] [ebp-14h]
    const char *token; // [esp+25Ch] [ebp-10h]
    int charsUsed; // [esp+260h] [ebp-Ch]
    const char *text; // [esp+264h] [ebp-8h] BYREF
    int spawnVarIndex; // [esp+268h] [ebp-4h]

    startPos = Com_GetBspLump(LUMP_ENTITIES, 1u, &textLen);
    Hunk_CheckTempMemoryClear();
    textPool = (char*)Hunk_AllocateTempMemory(textLen, "R_LoadEntities");
    smodelCount = 0;
    text = startPos;
    while (1)
    {
        token = Com_Parse(&text)->token;
        if (!text || *token != 123)
            break;
        spawnVars[0][0] = (char *)"";
        spawnVarCount = 1;
        charsUsed = 0;
        while (1)
        {
            token = Com_Parse(&text)->token;
            if (!*token || *token == 125)
                break;
            if (I_stricmp(token, "classname"))
            {
                if (spawnVarCount == 64)
                    Com_Error(ERR_DROP, "R_LoadEntities: MAX_SPAWN_VARS (%i) reached", 64);
                spawnVarIndex = spawnVarCount++;
            }
            else
            {
                spawnVarIndex = 0;
            }
            spawnVars[spawnVarIndex][0] = &textPool[charsUsed];
            charsUsed += strlen(token) + 1;
            memcpy(spawnVars[spawnVarIndex][0], token, strlen(token) + 1);
            token = Com_Parse(&text)->token;
            spawnVars[spawnVarIndex][1] = &textPool[charsUsed];
            charsUsed += strlen(token) + 1;
            memcpy(spawnVars[spawnVarIndex][1], token, strlen(token) + 1);
        }
        if (!*spawnVars[0][0])
            Com_Error(ERR_DROP, "R_LoadEntities: entity without a classname");
        if (!I_stricmp(spawnVars[0][1], "misc_model"))
            ++smodelCount;
    }
    s_world.dpvs.smodelDrawInsts = (GfxStaticModelDrawInst*)Hunk_Alloc(76 * smodelCount, "R_LoadEntities", 21);
    s_world.dpvs.smodelInsts = (GfxStaticModelInst*)Hunk_Alloc(28 * smodelCount, "R_LoadEntities", 21);
    s_world.dpvs.smodelCount = 0;
    text = startPos;
    iassert( s_world.cells );
    while (1)
    {
        token = Com_Parse(&text)->token;
        if (*token != 123)
            break;
        spawnVars[0][0] = (char*)"";
        spawnVarCount = 1;
        charsUsed = 0;
        while (1)
        {
            token = Com_Parse(&text)->token;
            if (!*token || *token == 125)
                break;
            if (I_stricmp(token, "classname"))
            {
                if (spawnVarCount == 64)
                    Com_Error(ERR_DROP, "R_LoadEntities: MAX_SPAWN_VARS (%i) reached", 64);
                spawnVarIndex = spawnVarCount++;
            }
            else
            {
                spawnVarIndex = 0;
            }
            spawnVars[spawnVarIndex][0] = &textPool[charsUsed];
            charsUsed += strlen(token) + 1;
            memcpy(spawnVars[spawnVarIndex][0], token, strlen(token) + 1);
            token = Com_Parse(&text)->token;
            spawnVars[spawnVarIndex][1] = &textPool[charsUsed];
            charsUsed += strlen(token) + 1;
            memcpy(spawnVars[spawnVarIndex][1], token, strlen(token) + 1);
        }
        if (!*spawnVars[0][0])
            Com_Error(ERR_DROP, "R_LoadEntities: entity without a classname");
        if (!I_stricmp(spawnVars[0][1], "misc_model"))
        {
            R_LoadMiscModel(spawnVars, spawnVarCount, bspVersion);
        }
    }
    iassert( s_world.dpvs.smodelCount == smodelCount );
    iassert( (smodelCount <= 65536) );
    Hunk_FreeTempMemory(textPool);
}

void R_AddAllProbesToAllCells()
{
    GfxCell *cell; // [esp+0h] [ebp-Ch]
    int cellIndex; // [esp+4h] [ebp-8h]
    unsigned __int8 reflectionProbeIndex; // [esp+Bh] [ebp-1h]

    iassert( s_world.reflectionProbeCount > 0 );
    if (s_world.reflectionProbeCount == 1)
    {
        for (cellIndex = 0; cellIndex < s_world.dpvsPlanes.cellCount; ++cellIndex)
        {
            cell = &s_world.cells[cellIndex];
            iassert( cell->reflectionProbeCount == 0 );
            iassert( cell->reflectionProbes == NULL );
            cell->reflectionProbeCount = 1;
            cell->reflectionProbes = Hunk_Alloc(1u, "R_AddAllProbesToAllCells", 22);
            *cell->reflectionProbes = 0;
        }
    }
    else
    {
        for (cellIndex = 0; cellIndex < s_world.dpvsPlanes.cellCount; ++cellIndex)
        {
            cell = &s_world.cells[cellIndex];
            iassert( cell->reflectionProbeCount == 0 );
            iassert( cell->reflectionProbes == NULL );
            cell->reflectionProbeCount = LOBYTE(s_world.reflectionProbeCount) - 1;
            iassert(cell->reflectionProbeCount == s_world.reflectionProbeCount - 1);
            cell->reflectionProbes = Hunk_Alloc(cell->reflectionProbeCount, "R_AddAllProbesToAllCells", 22);
            for (reflectionProbeIndex = 0; reflectionProbeIndex < s_world.reflectionProbeCount - 1; ++reflectionProbeIndex)
                cell->reflectionProbes[reflectionProbeIndex] = reflectionProbeIndex + 1;
        }
    }
}

void __cdecl R_SetStaticModelReflectionProbe(
    const GfxWorld *world,
    const GfxStaticModelInst *smodelInst,
    GfxStaticModelDrawInst *smodelDrawInst)
{
    unsigned int reflectionProbeIndex; // [esp+0h] [ebp-10h]
    float center[3]; // [esp+4h] [ebp-Ch] BYREF

    Vec3Avg(smodelInst->mins, smodelInst->maxs, center);
    reflectionProbeIndex = R_CalcReflectionProbeIndex(world, center);
    smodelDrawInst->reflectionProbeIndex = reflectionProbeIndex;
}

void R_SetStaticModelReflectionProbes()
{
    unsigned int smodelIndex; // [esp+0h] [ebp-4h]

    iassert( rgl.reflectionProbesLoaded );
    for (smodelIndex = 0; smodelIndex < s_world.dpvs.smodelCount; ++smodelIndex)
        R_SetStaticModelReflectionProbe(
            &s_world,
            &s_world.dpvs.smodelInsts[smodelIndex],
            &s_world.dpvs.smodelDrawInsts[smodelIndex]);
    rgl.staticModelReflectionProbesLoaded = 1;
}

void __cdecl R_AddStaticModelToAabbTree_r(GfxWorld *world, GfxAabbTree *tree, int smodelIndex)
{
    int v3; // [esp+4h] [ebp-30h]
    unsigned __int8 *children; // [esp+14h] [ebp-20h]
    GfxAabbTree *childTree; // [esp+18h] [ebp-1Ch]
    GfxAabbTree *childTreea; // [esp+18h] [ebp-1Ch]
    unsigned __int8 *newChildren; // [esp+1Ch] [ebp-18h]
    int childIndex; // [esp+20h] [ebp-14h]
    int childIndexa; // [esp+20h] [ebp-14h]
    int childIndexb; // [esp+20h] [ebp-14h]
    unsigned __int8 *smodelIndexes; // [esp+24h] [ebp-10h]
    GfxStaticModelInst *smodelInst; // [esp+28h] [ebp-Ch]
    int i; // [esp+30h] [ebp-4h]

    if (((tree->smodelIndexCount - 1) & tree->smodelIndexCount) == 0)
    {
        if (tree->smodelIndexCount)
            v3 = 2 * tree->smodelIndexCount;
        else
            v3 = 1;
        smodelIndexes = (unsigned __int8 *)Hunk_AllocateTempMemory(2 * v3, "R_AddModelToCell");
        memcpy(smodelIndexes, (unsigned __int8 *)tree->smodelIndexes, 2 * tree->smodelIndexCount);
        tree->smodelIndexes = (unsigned __int16 *)smodelIndexes;
    }
    tree->smodelIndexes[tree->smodelIndexCount] = smodelIndex;
    if (tree->smodelIndexes[tree->smodelIndexCount] != smodelIndex)
        MyAssertHandler(
            ".\\r_staticmodel_load_obj.cpp",
            281,
            0,
            "%s",
            "tree->smodelIndexes[tree->smodelIndexCount] == smodelIndex");
    ++tree->smodelIndexCount;
    if (tree->childCount)
    {
        smodelInst = &world->dpvs.smodelInsts[smodelIndex];
        for (childIndex = 0; childIndex < tree->childCount; ++childIndex)
        {
            childTree = (GfxAabbTree *)((char *)&tree[childIndex] + tree->childrenOffset);
            if (smodelInst->mins[0] >= (double)childTree->mins[0]
                && smodelInst->mins[1] >= (double)childTree->mins[1]
                && smodelInst->mins[2] >= (double)childTree->mins[2]
                && smodelInst->maxs[0] <= (double)childTree->maxs[0]
                && smodelInst->maxs[1] <= (double)childTree->maxs[1]
                && smodelInst->maxs[2] <= (double)childTree->maxs[2])
            {
                goto LABEL_18;
            }
        }
        for (childIndexa = 0; ; ++childIndexa)
        {
            if (childIndexa >= tree->childCount)
            {
                newChildren = Hunk_AllocAlign(44 * (tree->childCount + 1), 4, "R_AddStaticModelToAabbTree_r", 21);
                children = (unsigned __int8 *)tree + tree->childrenOffset;
                memcpy(newChildren, children, 44 * tree->childCount);
                tree->childrenOffset = newChildren - (unsigned __int8 *)tree;
                for (childIndexb = 0; childIndexb < tree->childCount; ++childIndexb)
                    *(_DWORD *)&newChildren[44 * childIndexb + 40] = &children[44 * childIndexb
                    + *(_DWORD *)&children[44 * childIndexb + 40]]
                    - &newChildren[44 * childIndexb];
                childTreea = (GfxAabbTree *)((char *)&tree[tree->childCount++] + tree->childrenOffset);
                childTreea->mins[0] = smodelInst->mins[0];
                childTreea->mins[1] = smodelInst->mins[1];
                childTreea->mins[2] = smodelInst->mins[2];
                childTreea->maxs[0] = smodelInst->maxs[0];
                childTreea->maxs[1] = smodelInst->maxs[1];
                childTreea->maxs[2] = smodelInst->maxs[2];
                R_AddStaticModelToAabbTree_r(world, childTreea, smodelIndex);
                return;
            }
            childTree = (GfxAabbTree *)((char *)&tree[childIndexa] + tree->childrenOffset);
            if (!childTree->surfaceCount)
                break;
        }
        for (i = 0; i < 3; ++i)
        {
            if (smodelInst->mins[i] < (double)childTree->mins[i])
                childTree->mins[i] = smodelInst->mins[i];
            if (smodelInst->maxs[i] > (double)childTree->maxs[i])
                childTree->maxs[i] = smodelInst->maxs[i];
        }
    LABEL_18:
        R_AddStaticModelToAabbTree_r(world, childTree, smodelIndex);
    }
}

void __cdecl R_AddStaticModelToCell(GfxWorld *world, GfxStaticModelInst *smodelInst, int cellIndex)
{
    GfxCell *cell; // [esp+0h] [ebp-Ch]
    GfxAabbTree *tree; // [esp+4h] [ebp-8h]
    int smodelIndex; // [esp+8h] [ebp-4h]

    iassert( smodelInst );
    if (cellIndex < 0 || cellIndex >= world->dpvsPlanes.cellCount)
        MyAssertHandler(
            ".\\r_staticmodel_load_obj.cpp",
            356,
            0,
            "%s",
            "cellIndex >= 0 && cellIndex < world->dpvsPlanes.cellCount");
    cell = &world->cells[cellIndex];
    smodelIndex = smodelInst - world->dpvs.smodelInsts;
    tree = cell->aabbTree;
    iassert( tree );
    if (!tree->smodelIndexCount || tree->smodelIndexes[tree->smodelIndexCount - 1] != smodelIndex)
        R_AddStaticModelToAabbTree_r(world, cell->aabbTree, smodelIndex);
}

void __cdecl R_FilterStaticModelIntoCells_r(
    GfxWorld *world,
    mnode_t *node,
    GfxStaticModelInst *smodelInst,
    const float *mins,
    const float *maxs)
{
    float localmaxs[3]; // [esp+0h] [ebp-50h]
    float dist; // [esp+Ch] [ebp-44h]
    float localmins[3]; // [esp+10h] [ebp-40h] BYREF
    unsigned int type; // [esp+1Ch] [ebp-34h]
    cplane_s *plane; // [esp+20h] [ebp-30h]
    int cellIndex; // [esp+24h] [ebp-2Ch]
    int boxSide; // [esp+28h] [ebp-28h]
    float mins2[4]; // [esp+2Ch] [ebp-24h] BYREF
    float maxs2[3]; // [esp+3Ch] [ebp-14h] BYREF
    mnode_t *rightNode; // [esp+48h] [ebp-8h]
    int planeIndex; // [esp+4Ch] [ebp-4h]

    LODWORD(mins2[3]) = world->dpvsPlanes.cellCount + 1;
    mins2[0] = *mins;
    mins2[1] = mins[1];
    mins2[2] = mins[2];
    maxs2[0] = *maxs;
    maxs2[1] = maxs[1];
    maxs2[2] = maxs[2];
    while (1)
    {
        cellIndex = node->cellIndex;
        planeIndex = cellIndex - (world->dpvsPlanes.cellCount + 1);
        if (planeIndex < 0)
            break;
        plane = &world->dpvsPlanes.planes[planeIndex];
        boxSide = BoxOnPlaneSide(mins2, maxs2, plane);
        if (boxSide == 3)
        {
            type = plane->type;
            rightNode = (node + 2 * node->rightChildOffset);
            if (type >= 3)
            {
                R_FilterStaticModelIntoCells_r(world, node + 1, smodelInst, mins2, maxs2);
            }
            else
            {
                dist = plane->dist;
                localmins[0] = mins2[0];
                localmins[1] = mins2[1];
                localmins[2] = mins2[2];
                localmins[plane->type] = dist;
                localmaxs[0] = maxs2[0];
                localmaxs[1] = maxs2[1];
                localmaxs[2] = maxs2[2];
                localmaxs[plane->type] = dist;
                if (!BoxOnPlaneSide(localmins, maxs2, plane))
                    MyAssertHandler(
                        ".\\r_staticmodel_load_obj.cpp",
                        423,
                        0,
                        "%s",
                        "BoxOnPlaneSide( localmins, maxs2, plane ) == BOXSIDE_FRONT");
                if (maxs2[plane->type] > dist)
                    R_FilterStaticModelIntoCells_r(world, node + 1, smodelInst, localmins, maxs2);
                maxs2[0] = localmaxs[0];
                maxs2[1] = localmaxs[1];
                maxs2[2] = localmaxs[2];
            }
            node = rightNode;
        }
        else
        {
            if (boxSide != 1 && boxSide != 2)
                MyAssertHandler(
                    ".\\r_staticmodel_load_obj.cpp",
                    444,
                    0,
                    "%s\n\t(boxSide) = %i",
                    "(boxSide == (1 << 0) || boxSide == (1 << 1))",
                    boxSide);
            node = (node + 2 * (boxSide - 1) * (node->rightChildOffset - 2) + 4);
        }
    }
    if (cellIndex)
        R_AddStaticModelToCell(world, smodelInst, cellIndex - 1);
}

void __cdecl R_AllocStaticModels(GfxAabbTree *tree)
{
    GfxAabbTree *children; // [esp+0h] [ebp-Ch]
    int childIndex; // [esp+4h] [ebp-8h]
    unsigned __int8 *smodelIndexes; // [esp+8h] [ebp-4h]

    if (tree->smodelIndexCount)
    {
        smodelIndexes = Hunk_AllocAlign(2 * tree->smodelIndexCount, 4, "R_AllocStaticModels", 21);
        memcpy(smodelIndexes, tree->smodelIndexes, 2 * tree->smodelIndexCount);
        tree->smodelIndexes = (unsigned short*)smodelIndexes;
    }
    children = (tree + tree->childrenOffset);
    for (childIndex = 0; childIndex < tree->childCount; ++childIndex)
        R_AllocStaticModels(&children[childIndex]);
}

int __cdecl CompareStaticModels(unsigned __int16 *smodel0, unsigned __int16 *smodel1)
{
    return *smodel0 - *smodel1;
}

int __cdecl R_SortGfxAabbTreeChildren(
    GfxWorld *world,
    float *mins,
    float *maxs,
    unsigned __int16 *staticModels,
    int staticModelCount)
{
    GfxStaticModelInst *smodelInst; // [esp+0h] [ebp-14h]
    int smodelSwapIndex; // [esp+4h] [ebp-10h]
    int childCount; // [esp+8h] [ebp-Ch]
    int smodelChildIndex; // [esp+Ch] [ebp-8h]
    int smodelIndex; // [esp+10h] [ebp-4h]

    childCount = 0;
    for (smodelChildIndex = 0; smodelChildIndex < staticModelCount; ++smodelChildIndex)
    {
        smodelIndex = staticModels[smodelChildIndex];
        smodelInst = &world->dpvs.smodelInsts[smodelIndex];
        if (*mins <= smodelInst->mins[0]
            && mins[1] <= smodelInst->mins[1]
            && mins[2] <= smodelInst->mins[2]
            && *maxs >= smodelInst->maxs[0]
            && maxs[1] >= smodelInst->maxs[1]
            && maxs[2] >= smodelInst->maxs[2])
        {
            smodelSwapIndex = staticModels[childCount];
            staticModels[childCount] = smodelIndex;
            iassert( staticModels[childCount] == smodelIndex );
            staticModels[smodelChildIndex] = smodelSwapIndex;
            if (staticModels[smodelChildIndex] != smodelSwapIndex)
                MyAssertHandler(
                    ".\\r_staticmodel_load_obj.cpp",
                    76,
                    0,
                    "%s",
                    "staticModels[smodelChildIndex] == smodelSwapIndex");
            ++childCount;
        }
    }
    return childCount < 2 ? 0 : childCount;
}

void __cdecl R_SortGfxAabbTree(GfxWorld *world, GfxAabbTree *tree)
{
    float *v2; // [esp+8h] [ebp-84h]
    int j; // [esp+14h] [ebp-78h]
    int ja; // [esp+14h] [ebp-78h]
    float middle[3]; // [esp+18h] [ebp-74h] BYREF
    GfxAabbTree *children; // [esp+24h] [ebp-68h]
    int smodelIndexIter; // [esp+28h] [ebp-64h]
    GfxAabbTree *childTree; // [esp+2Ch] [ebp-60h]
    float mins[3]; // [esp+30h] [ebp-5Ch] BYREF
    float childMaxs[3]; // [esp+3Ch] [ebp-50h] BYREF
    unsigned __int16 *smodelIndexes; // [esp+48h] [ebp-44h]
    int childIndex; // [esp+4Ch] [ebp-40h]
    GfxStaticModelInst *smodelInst; // [esp+50h] [ebp-3Ch]
    float childMins[3]; // [esp+54h] [ebp-38h] BYREF
    float maxs[3]; // [esp+60h] [ebp-2Ch] BYREF
    int smodelIndexCount; // [esp+6Ch] [ebp-20h]
    int i; // [esp+70h] [ebp-1Ch]
    int childCount[4]; // [esp+74h] [ebp-18h]
    int count; // [esp+84h] [ebp-8h]
    int smodelIndex; // [esp+88h] [ebp-4h]

    qsort(tree->smodelIndexes, tree->smodelIndexCount, 2u, (int(*)(const void *, const void *))CompareStaticModels);
    if (tree->childCount)
    {
        children = (tree + tree->childrenOffset);
        for (childIndex = 0; childIndex < tree->childCount; ++childIndex)
            R_SortGfxAabbTree(world, &children[childIndex]);
    }
    else
    {
        mins[0] = FLT_MAX;
        mins[1] = FLT_MAX;
        mins[2] = FLT_MAX;
        maxs[0] = -FLT_MAX;
        maxs[1] = -FLT_MAX;
        maxs[2] = -FLT_MAX;
        for (smodelIndexIter = 0; smodelIndexIter < tree->smodelIndexCount; ++smodelIndexIter)
        {
            smodelIndex = tree->smodelIndexes[smodelIndexIter];
            smodelInst = &world->dpvs.smodelInsts[smodelIndex];
            for (i = 0; i < 3; ++i)
            {
                if (smodelInst->mins[i] < mins[i])
                    mins[i] = smodelInst->mins[i];
                if (smodelInst->maxs[i] > maxs[i])
                    maxs[i] = smodelInst->maxs[i];
            }
        }
        if (!tree->surfaceCount)
        {
            tree->mins[0] = mins[0];
            tree->mins[1] = mins[1];
            tree->mins[2] = mins[2];
            tree->maxs[0] = maxs[0];
            tree->maxs[1] = maxs[1];
            tree->maxs[2] = maxs[2];
        }
        if (tree->smodelIndexCount >= 8u)
        {
            Vec3Add(mins, maxs, middle);
            Vec3Scale(middle, 0.5, middle);
            smodelIndexes = tree->smodelIndexes;
            smodelIndexCount = tree->smodelIndexCount;
            childMins[0] = mins[0];
            childMins[1] = mins[1];
            childMins[2] = mins[2];
            childMaxs[1] = maxs[1];
            childMaxs[2] = maxs[2];
            childMaxs[0] = middle[0];
            childCount[0] = R_SortGfxAabbTreeChildren(world, childMins, childMaxs, smodelIndexes, smodelIndexCount);
            smodelIndexes += childCount[0];
            smodelIndexCount -= childCount[0];
            childMins[1] = mins[1];
            childMins[2] = mins[2];
            childMaxs[0] = maxs[0];
            childMaxs[1] = maxs[1];
            childMaxs[2] = maxs[2];
            childMins[0] = middle[0];
            childCount[1] = R_SortGfxAabbTreeChildren(world, childMins, childMaxs, smodelIndexes, smodelIndexCount);
            smodelIndexes += childCount[1];
            smodelIndexCount -= childCount[1];
            childMins[0] = mins[0];
            childMins[1] = mins[1];
            childMins[2] = mins[2];
            childMaxs[0] = maxs[0];
            childMaxs[2] = maxs[2];
            childMaxs[1] = middle[1];
            childCount[2] = R_SortGfxAabbTreeChildren(world, childMins, childMaxs, smodelIndexes, smodelIndexCount);
            smodelIndexes += childCount[2];
            smodelIndexCount -= childCount[2];
            childMins[0] = mins[0];
            childMins[2] = mins[2];
            childMaxs[0] = maxs[0];
            childMaxs[1] = maxs[1];
            childMaxs[2] = maxs[2];
            childMins[1] = middle[1];
            childCount[3] = R_SortGfxAabbTreeChildren(world, childMins, childMaxs, smodelIndexes, smodelIndexCount);
            smodelIndexes += childCount[3];
            smodelIndexCount -= childCount[3];
            count = 0;
            for (j = 0; j < 4; ++j)
                count += childCount[j] != 0;
            if (count)
            {
                if (tree->surfaceCount)
                    ++count;
                if (smodelIndexCount)
                    ++count;
                tree->childrenOffset = Hunk_AllocAlign(44 * count, 4, "R_SortGfxAabbTree", 21) - (unsigned char*)tree;
                if (tree->surfaceCount)
                {
                    children = (tree + tree->childrenOffset);
                    childTree = &children[tree->childCount++];
                    childTree->mins[0] = tree->mins[0];
                    childTree->mins[1] = tree->mins[1];
                    childTree->mins[2] = tree->mins[2];
                    v2 = childTree->maxs;
                    childTree->maxs[0] = tree->maxs[0];
                    v2[1] = tree->maxs[1];
                    v2[2] = tree->maxs[2];
                    childTree->startSurfIndex = tree->startSurfIndex;
                    childTree->surfaceCount = tree->surfaceCount;
                    childTree->startSurfIndexNoDecal = tree->startSurfIndexNoDecal;
                    childTree->surfaceCountNoDecal = tree->surfaceCountNoDecal;
                }
                smodelIndexes = tree->smodelIndexes;
                smodelIndexCount = tree->smodelIndexCount;
                for (ja = 0; ja < 4; ++ja)
                {
                    count = childCount[ja];
                    if (count)
                    {
                        children = (tree + tree->childrenOffset);
                        childTree = &children[tree->childCount++];
                        childTree->smodelIndexCount = count;
                        iassert( childTree->smodelIndexCount == count );
                        childTree->smodelIndexes = smodelIndexes;
                        R_SortGfxAabbTree(world, childTree);
                        smodelIndexes += count;
                        smodelIndexCount -= count;
                    }
                }
                if (smodelIndexCount)
                {
                    children = (tree + tree->childrenOffset);
                    childTree = &children[tree->childCount++];
                    childTree->smodelIndexCount = smodelIndexCount;
                    if (childTree->smodelIndexCount != smodelIndexCount)
                        MyAssertHandler(
                            ".\\r_staticmodel_load_obj.cpp",
                            253,
                            0,
                            "%s",
                            "childTree->smodelIndexCount == smodelIndexCount");
                    childTree->smodelIndexes = smodelIndexes;
                    R_SortGfxAabbTree(world, childTree);
                }
            }
        }
    }
}

int __cdecl R_AabbTreeChildrenCount_r(GfxAabbTree *tree)
{
    GfxAabbTree *children; // [esp+0h] [ebp-Ch]
    unsigned int childIndex; // [esp+4h] [ebp-8h]
    int count; // [esp+8h] [ebp-4h]

    count = 1;
    children = (tree + tree->childrenOffset);
    for (childIndex = 0; childIndex < tree->childCount; ++childIndex)
        count += R_AabbTreeChildrenCount_r(&children[childIndex]);
    return count;
}

GfxAabbTree *__cdecl R_AabbTreeMove_r(GfxAabbTree *tree, GfxAabbTree *newTree, GfxAabbTree *newChildren)
{
    GfxAabbTree *children; // [esp+8h] [ebp-Ch]
    unsigned int childIndex; // [esp+Ch] [ebp-8h]
    GfxAabbTree *allocChildren; // [esp+10h] [ebp-4h]

    qmemcpy(newTree, tree, sizeof(GfxAabbTree));
    children = (tree + tree->childrenOffset);
    newTree->childrenOffset = newChildren - newTree;
    allocChildren = &newChildren[tree->childCount];
    for (childIndex = 0; childIndex < tree->childCount; ++childIndex)
        allocChildren = R_AabbTreeMove_r(&children[childIndex], &newChildren[childIndex], allocChildren);
    return allocChildren;
}

void __cdecl R_FixupGfxAabbTrees(GfxCell *cell)
{
    GfxAabbTree *tree; // [esp+0h] [ebp-Ch]
    GfxAabbTree *newTree; // [esp+4h] [ebp-8h]

    tree = cell->aabbTree;
    iassert( tree );
    cell->aabbTreeCount = R_AabbTreeChildrenCount_r(tree);
    newTree = (GfxAabbTree*)Hunk_AllocAlign(44 * cell->aabbTreeCount, 4, "R_FixupGfxAabbTrees", 21);
    if (R_AabbTreeMove_r(tree, newTree, newTree + 1) - newTree != cell->aabbTreeCount)
        MyAssertHandler(".\\r_bsp_load_obj.cpp", 3383, 0, "%s", "allocChildren - newTree == cell->aabbTreeCount");
    cell->aabbTree = newTree;
}

int R_PostLoadEntities()
{
    int result; // eax
    int cellIndex; // [esp+514h] [ebp-10h]
    int cellIndexa; // [esp+514h] [ebp-10h]
    int cellIndexb; // [esp+514h] [ebp-10h]
    GfxStaticModelCombinedInst *smodelCombinedInsts; // [esp+51Ch] [ebp-8h]
    unsigned int smodelIndex; // [esp+520h] [ebp-4h]
    unsigned int smodelIndexa; // [esp+520h] [ebp-4h]
    unsigned int smodelIndexb; // [esp+520h] [ebp-4h]

    iassert( rgl.staticModelReflectionProbesLoaded );
    smodelCombinedInsts = (GfxStaticModelCombinedInst*)Z_Malloc(104 * s_world.dpvs.smodelCount, "R_PostLoadEntities", 21);
    for (smodelIndex = 0; smodelIndex < s_world.dpvs.smodelCount; ++smodelIndex)
    {
        qmemcpy(&smodelCombinedInsts[smodelIndex], &s_world.dpvs.smodelDrawInsts[smodelIndex], 0x4Cu);
        qmemcpy(
            &smodelCombinedInsts[smodelIndex].smodelInst,
            &s_world.dpvs.smodelInsts[smodelIndex],
            sizeof(smodelCombinedInsts[smodelIndex].smodelInst));
    }
    //std::_Sort<GfxStaticModelCombinedInst *, int, bool(__cdecl *)(GfxStaticModelCombinedInst const &, GfxStaticModelCombinedInst const &)>(
    //    smodelCombinedInsts,
    //    &smodelCombinedInsts[s_world.dpvs.smodelCount],
    //    (104 * s_world.dpvs.smodelCount) / 104,
    //    R_StaticModelCompare);
    std::sort(&smodelCombinedInsts[0], &smodelCombinedInsts[s_world.dpvs.smodelCount], R_StaticModelCompare);
    for (smodelIndexa = 0; smodelIndexa < s_world.dpvs.smodelCount; ++smodelIndexa)
    {
        qmemcpy(
            &s_world.dpvs.smodelDrawInsts[smodelIndexa],
            &smodelCombinedInsts[smodelIndexa],
            sizeof(s_world.dpvs.smodelDrawInsts[smodelIndexa]));
        qmemcpy(
            &s_world.dpvs.smodelInsts[smodelIndexa],
            &smodelCombinedInsts[smodelIndexa].smodelInst,
            sizeof(s_world.dpvs.smodelInsts[smodelIndexa]));
    }
    Z_Free(smodelCombinedInsts, 21);
    iassert( s_world.dpvsPlanes.nodes );
    for (smodelIndexb = 0; smodelIndexb < s_world.dpvs.smodelCount; ++smodelIndexb)
        R_FilterStaticModelIntoCells_r(
            &s_world,
            (mnode_t*)s_world.dpvsPlanes.nodes,
            &s_world.dpvs.smodelInsts[smodelIndexb],
            s_world.dpvs.smodelInsts[smodelIndexb].mins,
            s_world.dpvs.smodelInsts[smodelIndexb].maxs);
    for (cellIndex = 0; cellIndex < s_world.dpvsPlanes.cellCount; ++cellIndex)
        R_AllocStaticModels(s_world.cells[cellIndex].aabbTree);
    for (cellIndexa = 0; cellIndexa < s_world.dpvsPlanes.cellCount; ++cellIndexa)
        R_SortGfxAabbTree(&s_world, s_world.cells[cellIndexa].aabbTree);
    Hunk_ClearTempMemory();
    for (cellIndexb = 0; ; ++cellIndexb)
    {
        result = cellIndexb;
        if (cellIndexb >= s_world.dpvsPlanes.cellCount)
            break;
        R_FixupGfxAabbTrees(&s_world.cells[cellIndexb]);
    }
    return result;
}

void __cdecl R_ForEachShadowCastingSurfaceOnEachLight(void(__cdecl *Callback)(GfxWorld *, unsigned int, unsigned int))
{
    unsigned int sortedSurfIndex; // [esp+0h] [ebp-8h]

    iassert( s_world.shadowGeom );
    iassert( s_world.lightRegion );
    for (sortedSurfIndex = 0; sortedSurfIndex < s_world.models->surfaceCount; ++sortedSurfIndex)
    {
        if ((s_world.dpvs.surfaces[sortedSurfIndex].material->info.gameFlags & 0x40) != 0)
            R_ForEachPrimaryLightAffectingSurface(
                &s_world,
                &s_world.dpvs.surfaces[sortedSurfIndex],
                sortedSurfIndex,
                Callback);
    }
}

void __cdecl R_IncrementShadowGeometryCount(GfxWorld *world, unsigned int primaryLightIndex, unsigned int idk)
{
    ++world->shadowGeom[primaryLightIndex].surfaceCount;
}

unsigned int R_InitShadowGeometryArrays()
{
    unsigned int result; // eax
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+0h] [ebp-10h]
    unsigned int primaryLightIndex; // [esp+4h] [ebp-Ch]
    GfxShadowGeometry *shadowGeom; // [esp+8h] [ebp-8h]
    GfxShadowGeometry *shadowGeoma; // [esp+8h] [ebp-8h]
    unsigned int smodelIndex; // [esp+Ch] [ebp-4h]

    iassert( s_world.shadowGeom );
    R_ForEachShadowCastingSurfaceOnEachLight(R_IncrementShadowGeometryCount);
    s_world.shadowGeom->surfaceCount = 0;
    s_world.shadowGeom[s_world.sunPrimaryLightIndex].surfaceCount = 0;
    for (primaryLightIndex = 0; primaryLightIndex < s_world.primaryLightCount; ++primaryLightIndex)
    {
        shadowGeom = &s_world.shadowGeom[primaryLightIndex];
        if (shadowGeom->surfaceCount)
        {
            shadowGeom->sortedSurfIndex = (unsigned short*)Hunk_Alloc(2 * shadowGeom->surfaceCount, "R_AllocShadowGeometryArrayMemory", 20);
            shadowGeom->surfaceCount = 0;
        }
        if (shadowGeom->smodelCount)
        {
            shadowGeom->smodelIndex = (unsigned short*)Hunk_Alloc(2 * shadowGeom->smodelCount, "R_AllocShadowGeometryArrayMemory", 20);
            shadowGeom->smodelCount = 0;
        }
    }
    R_ForEachShadowCastingSurfaceOnEachLight(R_AddShadowSurfaceToPrimaryLight);
    for (smodelIndex = 0; ; ++smodelIndex)
    {
        result = smodelIndex;
        if (smodelIndex >= s_world.dpvs.smodelCount)
            break;
        smodelDrawInst = &s_world.dpvs.smodelDrawInsts[smodelIndex];
        if (smodelDrawInst->primaryLightIndex >= s_world.primaryLightCount)
            MyAssertHandler(
                ".\\r_bsp_load_obj.cpp",
                4312,
                0,
                "smodelDrawInst->primaryLightIndex doesn't index s_world.primaryLightCount\n\t%i not in [0, %i)",
                smodelDrawInst->primaryLightIndex,
                s_world.primaryLightCount);
        shadowGeoma = &s_world.shadowGeom[smodelDrawInst->primaryLightIndex];
        if (shadowGeoma->smodelIndex)
        {
            shadowGeoma->smodelIndex[shadowGeoma->smodelCount] = smodelIndex;
            if (shadowGeoma->smodelIndex[shadowGeoma->smodelCount] != smodelIndex)
                MyAssertHandler(
                    ".\\r_bsp_load_obj.cpp",
                    4318,
                    1,
                    "shadowGeom->smodelIndex[shadowGeom->smodelCount] == smodelIndex\n\t%i, %i",
                    shadowGeoma->smodelIndex[shadowGeoma->smodelCount],
                    smodelIndex);
            ++shadowGeoma->smodelCount;
        }
    }
    return result;
}

void __cdecl R_LoadSun(const char *name, sunflare_t *sun)
{
    char *v2; // eax
    const char *nameIter; // [esp+0h] [ebp-54h]
    char sunFile[68]; // [esp+4h] [ebp-50h] BYREF
    const char *firstCharToCopy; // [esp+4Ch] [ebp-8h]
    char *firstPeriod; // [esp+50h] [ebp-4h]

    iassert( name );
    iassert( sun );
    Com_Memset(sun, 0, 96);
    firstCharToCopy = name;
    for (nameIter = name; *nameIter; ++nameIter)
    {
        if (*nameIter == 47 || *nameIter == 92)
            firstCharToCopy = nameIter + 1;
    }
    I_strncpyz(sunFile, firstCharToCopy, 64);
    v2 = strchr(sunFile, 0x2Eu);
    firstPeriod = v2;
    if (v2)
        *firstPeriod = 0;
    if (sunFile[0])
        R_LoadSunThroughDvars(sunFile, sun);
}

unsigned __int8 *R_AllocPrimaryLightBuffers()
{
    unsigned __int16 EntityCount; // ax
    unsigned __int16 v1; // ax
    unsigned __int8 *result; // eax
    unsigned int relevantPrimaryLightCount; // [esp+4h] [ebp-Ch]
    unsigned int totalDynEntBits; // [esp+8h] [ebp-8h]

    if (s_world.sunPrimaryLightIndex > 1)
        MyAssertHandler(
            ".\\r_bsp_load_obj.cpp",
            4170,
            0,
            "%s",
            "s_world.sunPrimaryLightIndex == PRIMARY_LIGHT_SUN || s_world.sunPrimaryLightIndex == PRIMARY_LIGHT_NONE");
    relevantPrimaryLightCount = s_world.primaryLightCount - (s_world.sunPrimaryLightIndex + 1);
    s_world.primaryLightEntityShadowVis = (unsigned int*)Hunk_Alloc(
        4 * (((relevantPrimaryLightCount << 12) + 31) >> 5),
        "R_AllocPrimaryLightBuffers",
        20);
    EntityCount = DynEnt_GetEntityCount(DYNENT_COLL_CLIENT_FIRST);
    s_world.nonSunPrimaryLightForModelDynEnt = Hunk_Alloc(EntityCount, "R_AllocPrimaryLightBuffers", 20);
    v1 = DynEnt_GetEntityCount(DYNENT_COLL_CLIENT_FIRST);
    s_world.primaryLightDynEntShadowVis[0] = (unsigned int *)Hunk_Alloc(
        4 * ((relevantPrimaryLightCount * v1 + 31) >> 5),
        "R_AllocPrimaryLightBuffers",
        20);
    totalDynEntBits = relevantPrimaryLightCount * DynEnt_GetEntityCount(DYNENT_COLL_CLIENT_BRUSH);
    result = Hunk_Alloc(4 * ((totalDynEntBits + 31) >> 5), "R_AllocPrimaryLightBuffers", 20);
    s_world.primaryLightDynEntShadowVis[1] = (unsigned int *)result;
    return result;
}

unsigned __int8 *R_LoadWorldRuntime()
{
    unsigned __int8 *v1; // [esp+0h] [ebp-24h]
    unsigned __int8 *v2; // [esp+4h] [ebp-20h]
    unsigned __int8 *v3; // [esp+8h] [ebp-1Ch]
    unsigned __int8 *v4; // [esp+Ch] [ebp-18h]
    unsigned __int8 *v5; // [esp+10h] [ebp-14h]
    unsigned __int8 *v6; // [esp+14h] [ebp-10h]
    unsigned __int8 *v7; // [esp+18h] [ebp-Ch]
    unsigned int drawType; // [esp+1Ch] [ebp-8h]
    unsigned int drawTypea; // [esp+1Ch] [ebp-8h]
    int i; // [esp+20h] [ebp-4h]

    for (i = 0; i < 3; ++i)
    {
        s_world.dpvs.smodelVisDataCount = 4 * ((s_world.dpvs.smodelCount + 127) >> 7);
        s_world.dpvs.surfaceVisDataCount = 4 * ((s_world.models->surfaceCount + 127) >> 7);
        if (s_world.dpvs.smodelCount)
            v7 = Hunk_Alloc(s_world.dpvs.smodelCount, "R_InitDynamicData", 21);
        else
            v7 = 0;
        s_world.dpvs.smodelVisData[i] = v7;
        if (s_world.models->surfaceCount)
            v6 = Hunk_Alloc(s_world.models->surfaceCount, "R_InitDynamicData", 20);
        else
            v6 = 0;
        s_world.dpvs.surfaceVisData[i] = v6;
        for (drawType = 0; drawType < 2; ++drawType)
            s_world.dpvsDyn.dynEntVisData[drawType][i] = Hunk_Alloc(
                32 * s_world.dpvsDyn.dynEntClientWordCount[drawType],
                "R_InitDynamicData",
                20);
    }
    if (s_world.dpvs.smodelCount)
        s_world.dpvs.lodData = (unsigned int*)Hunk_Alloc(8 * s_world.dpvs.smodelVisDataCount, "R_InitDynamicData", 21);
    else
        s_world.dpvs.lodData = 0;
    s_world.dpvs.staticSurfaceCount = s_world.models->surfaceCount;
    s_world.dpvs.staticSurfaceCountNoDecal = s_world.models->surfaceCountNoDecal;
    if (s_world.dpvs.staticSurfaceCount)
        v4 = Hunk_Alloc(8 * s_world.dpvs.staticSurfaceCount, "R_InitDynamicData", 20);
    else
        v4 = 0;
    s_world.dpvs.surfaceMaterials = (GfxDrawSurf*)v4;
    if (s_world.dpvs.staticSurfaceCount)
        v3 = Hunk_Alloc(4 * s_world.dpvs.surfaceVisDataCount, "R_InitDynamicData", 20);
    else
        v3 = 0;
    s_world.dpvs.surfaceCastsSunShadow = (unsigned int*)v3;
    if (s_world.dpvsPlanes.cellCount)
        v2 = Hunk_Alloc(
            4 * s_world.dpvsPlanes.cellCount * ((s_world.dpvsPlanes.cellCount + 31) >> 5),
            "R_InitDynamicData",
            20);
    else
        v2 = 0;
    s_world.cellCasterBits = (unsigned int*)v2;
    if (s_world.dpvsPlanes.cellCount)
        v1 = Hunk_Alloc(s_world.dpvsPlanes.cellCount << 10, "R_InitDynamicData", 20);
    else
        v1 = 0;
    s_world.dpvsPlanes.sceneEntCellBits = (unsigned int*)v1;
    for (drawTypea = 0; drawTypea < 2; ++drawTypea)
        s_world.dpvsDyn.dynEntCellBits[drawTypea] = (unsigned int*)Hunk_Alloc(
            4
            * s_world.dpvsPlanes.cellCount
            * s_world.dpvsDyn.dynEntClientWordCount[drawTypea],
            "R_InitDynamicData",
            20);
    s_world.sceneDynModel = (GfxSceneDynModel*)Hunk_Alloc(6 * s_world.dpvsDyn.dynEntClientCount[0], "R_InitDynamicData", 20);
    s_world.sceneDynBrush = (GfxSceneDynBrush*)Hunk_Alloc(4 * s_world.dpvsDyn.dynEntClientCount[1], "R_InitDynamicData", 20);
    return R_AllocPrimaryLightBuffers();
}

struct GfxSModelSurfStats // sizeof=0x10
{                                       // ...
    XModel *model;                      // ...
    unsigned int lod;                   // ...
    unsigned int smcAllocBits;          // ...
    unsigned int useCount;              // ...
};

BOOL __cdecl R_CompareSModels_Model(const GfxStaticModelDrawInst *s0, const GfxStaticModelDrawInst *s1)
{
    return s0->model < s1->model;
}

int __cdecl R_GetSModelCacheAllocBits(const XModel *model, unsigned int lod)
{
    DWORD v3; // eax
    int v6; // [esp+4h] [ebp-30h]
    unsigned int surfCount; // [esp+18h] [ebp-1Ch]
    unsigned int triCount; // [esp+1Ch] [ebp-18h]
    unsigned int surfIter; // [esp+24h] [ebp-10h]
    unsigned int vertCount; // [esp+28h] [ebp-Ch]
    XSurface *surfs; // [esp+2Ch] [ebp-8h] BYREF
    unsigned int minPoolSize; // [esp+30h] [ebp-4h]

    surfCount = XModelGetSurfCount(model, lod);
    XModelGetSurfaces(model, &surfs, lod);
    triCount = 0;
    vertCount = 0;
    for (surfIter = 0; surfIter < surfCount; ++surfIter)
    {
        vertCount += XSurfaceGetNumVerts(&surfs[surfIter]);
        triCount += XSurfaceGetNumTris(&surfs[surfIter]);
    }
    if ((4 * vertCount) < (3 * triCount))
        v6 = 3 * triCount;
    else
        v6 = 4 * vertCount;
    minPoolSize = (v6 + 3) / 4;
    if (!_BitScanReverse(&v3, minPoolSize - 1))
        v3 = 63;
    if (32 - (v3 ^ 0x1F) > 4)
        return 32 - (v3 ^ 0x1F);
    else
        return 4;
}

unsigned int __cdecl R_MaxModelsInDistRange(
    const GfxStaticModelDrawInst **drawInstArray,
    unsigned int drawInstCount,
    const float *mins,
    const float *maxs,
    float distMin,
    float distMax)
{
    float radiusDeviate; // [esp+0h] [ebp-50h]
    float yawDeviate; // [esp+4h] [ebp-4Ch]
    unsigned int v9; // [esp+Ch] [ebp-44h]
    unsigned int maxModelsInRange; // [esp+14h] [ebp-3Ch]
    unsigned int drawInstCutoff; // [esp+18h] [ebp-38h]
    float distMinSq; // [esp+1Ch] [ebp-34h]
    float size; // [esp+20h] [ebp-30h]
    float size_4; // [esp+24h] [ebp-2Ch]
    unsigned int modelsInRange; // [esp+28h] [ebp-28h]
    unsigned int drawInstIter; // [esp+2Ch] [ebp-24h]
    unsigned int dartIndex; // [esp+34h] [ebp-1Ch]
    float distMaxSq; // [esp+38h] [ebp-18h]
    float distSq; // [esp+3Ch] [ebp-14h]
    float mid; // [esp+40h] [ebp-10h]
    float mid_4; // [esp+44h] [ebp-Ch]
    float testPos[2]; // [esp+48h] [ebp-8h] BYREF

    mid = (*mins + *maxs) * 0.5;
    mid_4 = (mins[1] + maxs[1]) * 0.5;
    size = mid - *mins;
    size_4 = mid_4 - mins[1];
    maxModelsInRange = 0;
    if (drawInstCount >> 2 > 0x64)
        v9 = drawInstCount >> 2;
    else
        v9 = 100;
    for (dartIndex = 0; dartIndex < v9; ++dartIndex)
    {
        yawDeviate = random();
        radiusDeviate = random();
        PointInCircleFromUniformDeviates(radiusDeviate, yawDeviate, testPos);
        testPos[0] = testPos[0] * size + mid;
        testPos[1] = testPos[1] * size_4 + mid_4;
        modelsInRange = 0;
        drawInstCutoff = drawInstCount - maxModelsInRange;
        for (drawInstIter = 0; drawInstIter < drawInstCutoff; ++drawInstIter)
        {
            distSq = Vec2DistanceSq(testPos, drawInstArray[drawInstIter]->placement.origin);
            distMinSq = distMin * distMin;
            if (distMinSq < distSq)
            {
                distMaxSq = distMax * distMax;
                if (distMaxSq > distSq)
                {
                    ++modelsInRange;
                    if (drawInstCutoff < drawInstCount)
                        ++drawInstCutoff;
                }
            }
        }
        if (maxModelsInRange < modelsInRange)
        {
            maxModelsInRange = modelsInRange;
            if (modelsInRange == drawInstCount)
                break;
        }
    }
    return maxModelsInRange;
}

unsigned int __cdecl R_AddSModelListStats(
    const GfxStaticModelDrawInst **drawInstArray,
    unsigned int drawInstCount,
    GfxSModelSurfStats *stats,
    unsigned int statsCount,
    unsigned int statsLimit)
{
    unsigned int lodIter; // [esp+8h] [ebp-30h]
    XModel *model; // [esp+Ch] [ebp-2Ch]
    float mins[2]; // [esp+10h] [ebp-28h] BYREF
    float prevLodDist; // [esp+18h] [ebp-20h]
    unsigned int lodCount; // [esp+1Ch] [ebp-1Ch]
    float maxs[2]; // [esp+20h] [ebp-18h] BYREF
    unsigned int drawInstIter; // [esp+28h] [ebp-10h]
    unsigned int useCount; // [esp+2Ch] [ebp-Ch]
    unsigned int smcAllocBits; // [esp+30h] [ebp-8h]
    float lodDist; // [esp+34h] [ebp-4h]

    if (drawInstCount < 0x10)
        return statsCount;
    model = (*drawInstArray)->model;
    lodCount = XModelGetNumLods(model);
    if (lodCount + statsCount > statsLimit)
        return statsCount;
    ClearBounds2D(mins, maxs);
    for (drawInstIter = 0; drawInstIter < drawInstCount; ++drawInstIter)
        AddPointToBounds2D(drawInstArray[drawInstIter]->placement.origin, mins, maxs);
    lodIter = 0;
    prevLodDist = 0.0;
    while (lodIter < lodCount)
    {
        lodDist = XModelGetLodDist(model, lodIter);
        smcAllocBits = R_GetSModelCacheAllocBits(model, lodIter);
        if (smcAllocBits <= 9)
        {
            useCount = R_MaxModelsInDistRange(drawInstArray, drawInstCount, mins, maxs, prevLodDist, lodDist);
            if (useCount >= 0x10)
            {
                stats[statsCount].model = model;
                stats[statsCount].lod = lodIter;
                stats[statsCount].useCount = useCount;
                stats[statsCount++].smcAllocBits = smcAllocBits;
            }
        }
        ++lodIter;
        prevLodDist = lodDist;
    }
    return statsCount;
}

unsigned int __cdecl R_OptimalSModelResourceStats(GfxWorld *world, GfxSModelSurfStats *stats, unsigned int statLimit)
{
    const GfxStaticModelDrawInst **drawInstArray; // [esp+ECh] [ebp-14h]
    unsigned int smodelIter; // [esp+F0h] [ebp-10h]
    unsigned int smodelItera; // [esp+F0h] [ebp-10h]
    unsigned int smodelIterNext; // [esp+F8h] [ebp-8h]
    unsigned int statCount; // [esp+FCh] [ebp-4h]

    iassert( world );
    if (!world->dpvs.smodelCount)
        return 0;
    drawInstArray = (const GfxStaticModelDrawInst **)Hunk_AllocateTempMemory(4 * world->dpvs.smodelCount, "R_AssignSModelCacheResources");
    for (smodelIter = 0; smodelIter != world->dpvs.smodelCount; ++smodelIter)
        drawInstArray[smodelIter] = &world->dpvs.smodelDrawInsts[smodelIter];
    //std::_Sort<int *, int, bool(__cdecl *)(int, int)>(
    //    drawInstArray,
    //    &drawInstArray[world->dpvs.smodelCount],
    //    (4 * world->dpvs.smodelCount) >> 2,
    //    R_CompareSModels_Model);
    std::sort(&drawInstArray[0], &drawInstArray[world->dpvs.smodelCount], R_CompareSModels_Model);
    statCount = 0;
    for (smodelItera = 0; smodelItera != world->dpvs.smodelCount; smodelItera = smodelIterNext)
    {
        for (smodelIterNext = smodelItera;
            smodelIterNext != world->dpvs.smodelCount
            && drawInstArray[smodelIterNext]->model == drawInstArray[smodelItera]->model;
            ++smodelIterNext)
        {
            ;
        }
        statCount = R_AddSModelListStats(
            &drawInstArray[smodelItera],
            smodelIterNext - smodelItera,
            stats,
            statCount,
            statLimit);
    }
    Hunk_FreeTempMemory((char*)drawInstArray);
    return statCount;
}

static BOOL __cdecl R_CompareSModelStats_Score(const GfxSModelSurfStats &s0, const GfxSModelSurfStats &s1)
{
    return s1.useCount << s0.smcAllocBits < s0.useCount << s1.smcAllocBits;
}

unsigned int __cdecl R_GetEntryCount(GfxSModelSurfStats *stats)
{
    if (1 << (16 - LOBYTE(stats->smcAllocBits)) < stats->useCount)
        return 1 << (16 - LOBYTE(stats->smcAllocBits));
    else
        return stats->useCount;
}

unsigned __int8 __cdecl R_AssignSModelCacheIndex(
    char smcAllocBits,
    unsigned int maxEntryCount,
    unsigned int *smcUseCount)
{
    unsigned int leastUsedCache; // [esp+8h] [ebp-8h]
    unsigned int cacheIter; // [esp+Ch] [ebp-4h]

    leastUsedCache = 0;
    for (cacheIter = 1; cacheIter < 4; ++cacheIter)
    {
        if (smcUseCount[cacheIter] < smcUseCount[leastUsedCache])
            leastUsedCache = cacheIter;
    }
    smcUseCount[leastUsedCache] += maxEntryCount << smcAllocBits;
    return leastUsedCache;
}

void __cdecl XModelSetSModelCacheForLod(
    XModel *model,
    unsigned int lod,
    unsigned int smcIndex,
    unsigned int smcAllocBits)
{
    int v4; // [esp+0h] [ebp-4h]

    iassert( model );
    if (lod >= 4)
        MyAssertHandler(".\\xanim\\xmodel_utils.cpp", 107, 0, "lod doesn't index MAX_LODS\n\t%i not in [0, %i)", lod, 4);
    iassert( model->lodInfo[lod].smcIndexPlusOne == 0 );
    iassert( model->lodInfo[lod].lod == lod );
    v4 = smcIndex + 1;
    if (smcIndex + 1 != (smcIndex + 1))
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
            281,
            0,
            "i == static_cast< Type >( i )\n\t%i, %i",
            v4,
            v4);
    model->lodInfo[lod].smcIndexPlusOne = v4;
    if (smcAllocBits != smcAllocBits)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
            281,
            0,
            "i == static_cast< Type >( i )\n\t%i, %i",
            smcAllocBits,
            smcAllocBits);
    model->lodInfo[lod].smcAllocBits = smcAllocBits;
}

void __cdecl R_AssignSModelCacheResources(GfxWorld *world)
{
    unsigned __int8 v1; // [esp+17Bh] [ebp-2029h]
    XModel *model; // [esp+17Ch] [ebp-2028h]
    unsigned int smcUseCount[4]; // [esp+180h] [ebp-2024h] BYREF
    unsigned int lod; // [esp+190h] [ebp-2014h]
    GfxSModelSurfStats stats[512]; // [esp+194h] [ebp-2010h] BYREF
    unsigned int maxEntryCount; // [esp+2198h] [ebp-Ch]
    unsigned int v7; // [esp+219Ch] [ebp-8h]
    unsigned int i; // [esp+21A0h] [ebp-4h]

    iassert( world );
    memset(smcUseCount, 0, sizeof(smcUseCount));
    v7 = R_OptimalSModelResourceStats(world, stats, 0x200u);
    //std::_Sort<DBReorderAssetEntry *, int, bool(__cdecl *)(DBReorderAssetEntry const &, DBReorderAssetEntry const &)>(
    //    stats,
    //    &stats[v7],
    //    (16 * v7) >> 4,
    //    R_CompareSModelStats_Score);
    std::sort(&stats[0], &stats[v7], R_CompareSModelStats_Score);
    for (i = 0; i < v7; ++i)
    {
        model = stats[i].model;
        lod = stats[i].lod;
        maxEntryCount = R_GetEntryCount(&stats[i]);
        v1 = R_AssignSModelCacheIndex(stats[i].smcAllocBits, maxEntryCount, smcUseCount);
        XModelSetSModelCacheForLod(model, lod, v1, stats[i].smcAllocBits);
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