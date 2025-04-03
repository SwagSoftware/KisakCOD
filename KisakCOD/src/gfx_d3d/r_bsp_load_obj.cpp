#include "r_bsp.h"
#include <cgame_mp/cg_local_mp.h>
#include "r_init.h"

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

