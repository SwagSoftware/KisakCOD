#include "r_draw_material.h"
#include "rb_logfile.h"
#include "r_draw_shadowable_light.h"


int __cdecl R_SetupMaterial(
    GfxCmdBufContext context,
    GfxCmdBufContext *prepassContext,
    const GfxDrawSurfListInfo *info,
    GfxDrawSurf drawSurf)
{
    unsigned __int64 v5; // rax
    GfxDrawSurf overrideDrawSurf; // [esp+8h] [ebp-18h]
    MaterialTechniqueType baseTechType; // [esp+14h] [ebp-Ch]
    unsigned int surfType; // [esp+18h] [ebp-8h]

    surfType = (drawSurf.packed >> 50) & 0xF;
    baseTechType = info->baseTechType;
    if (baseTechType > TECHNIQUE_BUILD_FLOAT_Z)
    {
        if (baseTechType == TECHNIQUE_LIT_BEGIN)
        {
            if ((unsigned __int8)(drawSurf.packed >> 42) >= info->viewInfo->shadowableLightCount)
                MyAssertHandler(
                    ".\\r_draw_material.cpp",
                    367,
                    0,
                    "drawSurf.fields.primaryLightIndex doesn't index info->viewInfo->shadowableLightCount\n\t%i not in [0, %i)",
                    (unsigned __int8)(drawSurf.packed >> 42),
                    info->viewInfo->shadowableLightCount);
            if (!R_SetMaterial(
                context,
                drawSurf,
                (MaterialTechniqueType)context.source->input.data->primaryLightTechType[surfType][(unsigned __int8)(drawSurf.packed >> 42)]))
                return 0;
            R_SetShadowableLight(context.source, (unsigned __int8)(drawSurf.packed >> 42), info->viewInfo);
        }
        else if (baseTechType == TECHNIQUE_DEBUG_BUMPMAP && (surfType == 4 || surfType == 3))
        {
            if (!R_SetMaterial(context, drawSurf, TECHNIQUE_DEBUG_BUMPMAP_INSTANCED))
                return 0;
        }
        else if (!R_SetMaterial(context, drawSurf, baseTechType))
        {
            return 0;
        }
    }
    else if (!R_SetPrepassMaterial(context, drawSurf, baseTechType))
    {
        return 0;
    }
    if (prepassContext->state && !R_SetPrepassMaterial(*prepassContext, drawSurf, TECHNIQUE_DEPTH_PREPASS))
        prepassContext->state = 0;
    context.state->origMaterial = context.state->material;
    context.state->origTechType = context.state->techType;
    if (pixelCostMode > GFX_PIXEL_COST_MODE_MEASURE_MSEC)
    {
        v5 = ((R_PixelCost_GetAccumulationMaterial(context.state->material)->info.drawSurf.packed >> 29) & 0x7FF) << 29;
        *(unsigned int *)&overrideDrawSurf.fields = v5 | *(unsigned int *)&drawSurf.fields & 0x1FFFFFFF;
        HIDWORD(overrideDrawSurf.packed) = HIDWORD(v5) | HIDWORD(drawSurf.packed) & 0xFFFFFF00;
        R_SetMaterial(context, overrideDrawSurf, TECHNIQUE_UNLIT);
    }
    return 1;
}

int __cdecl R_SetPrepassMaterial(GfxCmdBufContext context, GfxDrawSurf drawSurf, MaterialTechniqueType techType)
{
    const MaterialTechnique *technique; // [esp+8h] [ebp-Ch]
    Material *material; // [esp+Ch] [ebp-8h]

    if (((drawSurf.packed >> 40) & 3) == 3)
        return 0;
    material = rgp.sortedMaterials[(drawSurf.packed >> 29) & 0x7FF];
    if (((drawSurf.packed >> 40) & 3) == 0 && (material->stateFlags & 1) != 0)
        material = rgp.depthPrepassMaterial;
    context.state->material = material;
    technique = Material_GetTechnique(material, techType);
    context.state->technique = technique;
    if (!technique)
        return 0;
    if ((technique->flags & 2) != 0)
        MyAssertHandler(".\\r_draw_material.cpp", 187, 0, "%s", "!(technique->flags & MTL_TECHFLAG_NEEDS_RESOLVED_SCENE)");
    if ((technique->flags & 1) != 0)
        MyAssertHandler(
            ".\\r_draw_material.cpp",
            188,
            0,
            "%s",
            "!(technique->flags & MTL_TECHFLAG_NEEDS_RESOLVED_POST_SUN)");
    if (!material)
        MyAssertHandler(".\\r_draw_material.cpp", 190, 0, "%s", "material");
    context.state->techType = techType;
    return 1;
}

int __cdecl R_SetMaterial(GfxCmdBufContext context, GfxDrawSurf drawSurf, MaterialTechniqueType techType)
{
    const char *v4; // eax
    const char *v5; // eax
    const MaterialTechnique *technique; // [esp+8h] [ebp-8h]
    Material *material; // [esp+Ch] [ebp-4h]

    material = rgp.sortedMaterials[(drawSurf.packed >> 29) & 0x7FF];
    technique = Material_GetTechnique(material, techType);
    context.state->material = material;
    context.state->technique = technique;
    if (!technique)
        return 0;
    if ((technique->flags & 2) != 0)
        MyAssertHandler(".\\r_draw_material.cpp", 316, 0, "%s", "!(technique->flags & MTL_TECHFLAG_NEEDS_RESOLVED_SCENE)");
    if ((technique->flags & 1) != 0 && !rg.distortion)
        return 0;
    if ((techType == TECHNIQUE_EMISSIVE || techType == TECHNIQUE_UNLIT)
        && (technique->flags & 0x10) != 0
        && !context.source->constVersions[4])
    {
        return 0;
    }
    if (!material)
        MyAssertHandler(".\\r_draw_material.cpp", 337, 0, "%s", "material");
    if (r_logFile->current.integer)
    {
        v4 = RB_LogTechniqueType(techType);
        v5 = va("R_SetMaterial( %s, %s, %s )\n", material->info.name, technique->name, v4);
        RB_LogPrint(v5);
    }
    context.state->techType = techType;
    return 1;
}

void __cdecl R_SetGameTime(GfxCmdBufSourceState *source, float gameTime)
{
    float v2; // [esp+8h] [ebp-30h]
    float v3; // [esp+28h] [ebp-10h]
    float cosOfFracPartOfGameTime; // [esp+2Ch] [ebp-Ch]
    float sinOfFracPartOfGameTime; // [esp+30h] [ebp-8h]
    float fracPartOfGameTime; // [esp+34h] [ebp-4h]

    v2 = floor(gameTime);
    fracPartOfGameTime = gameTime - v2;
    v3 = fracPartOfGameTime * 6.283185482025146;
    cosOfFracPartOfGameTime = cos(v3);
    sinOfFracPartOfGameTime = sin(v3);
    source->input.consts[18][0] = sinOfFracPartOfGameTime;
    source->input.consts[18][1] = cosOfFracPartOfGameTime;
    source->input.consts[18][2] = fracPartOfGameTime;
    source->input.consts[18][3] = gameTime;
    R_DirtyCodeConstant(source, 0x12u);
}

int __cdecl R_UpdateMaterialTime(GfxCmdBufSourceState *source, float materialTime)
{
    float gameTime; // [esp+4h] [ebp-4h]

    if (source->materialTime == materialTime)
        return 0;
    source->materialTime = materialTime;
    gameTime = source->sceneDef.floatTime - materialTime;
    R_SetGameTime(source, gameTime);
    return 1;
}

