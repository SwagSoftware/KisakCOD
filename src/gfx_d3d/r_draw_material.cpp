#include "r_draw_material.h"
#include "rb_logfile.h"
#include "r_draw_shadowable_light.h"
#include "rb_pixelcost.h"
#include "r_drawsurf.h"


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

    surfType = drawSurf.fields.surfType;
    baseTechType = info->baseTechType;
    if (baseTechType > TECHNIQUE_BUILD_FLOAT_Z)
    {
        if (baseTechType == TECHNIQUE_LIT_BEGIN)
        {
            iassert(drawSurf.fields.primaryLightIndex < info->viewInfo->shadowableLightCount);
            if (!R_SetMaterial(context, drawSurf, (MaterialTechniqueType)context.source->input.data->primaryLightTechType[surfType][drawSurf.fields.primaryLightIndex]))
            {
                return 0;
            }
            R_SetShadowableLight(context.source, drawSurf.fields.primaryLightIndex, info->viewInfo);
        }
        else if (baseTechType == TECHNIQUE_DEBUG_BUMPMAP && (surfType == SF_STATICMODEL_CACHED || surfType == SF_STATICMODEL_PRETESS))
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
        //v5 = ((R_PixelCost_GetAccumulationMaterial(context.state->material)->info.drawSurf.packed >> 29) & 0x7FF) << 29;
        //v5 = R_PixelCost_GetAccumulationMaterial(context.state->material)->info.drawSurf.fields.materialSortedIndex;
        //*(unsigned int *)&overrideDrawSurf.fields = v5 | *(unsigned int *)&drawSurf.fields & 0x1FFFFFFF;
        //HIDWORD(overrideDrawSurf.packed) = HIDWORD(v5) | HIDWORD(drawSurf.packed) & 0xFFFFFF00;
        overrideDrawSurf.fields.materialSortedIndex = R_PixelCost_GetAccumulationMaterial(context.state->material)->info.drawSurf.fields.materialSortedIndex;
        R_SetMaterial(context, overrideDrawSurf, TECHNIQUE_UNLIT);
    }
    return 1;
}

int __cdecl R_SetPrepassMaterial(GfxCmdBufContext context, GfxDrawSurf drawSurf, MaterialTechniqueType techType)
{
    const MaterialTechnique *technique; // [esp+8h] [ebp-Ch]
    Material *material; // [esp+Ch] [ebp-8h]

    if (drawSurf.fields.prepass == MTL_PREPASS_NONE)
        return 0;

    material = rgp.sortedMaterials[drawSurf.fields.materialSortedIndex];

    if (drawSurf.fields.prepass == MTL_PREPASS_STANDARD && (material->stateFlags & 1) != 0)
        material = rgp.depthPrepassMaterial;

    context.state->material = material;
    technique = Material_GetTechnique(material, techType);
    context.state->technique = technique;

    if (!technique)
        return 0;

    iassert(!(technique->flags & MTL_TECHFLAG_NEEDS_RESOLVED_SCENE));
    iassert(!(technique->flags & MTL_TECHFLAG_NEEDS_RESOLVED_POST_SUN));
    iassert(material);

    context.state->techType = techType;
    return 1;
}

int __cdecl R_SetMaterial(GfxCmdBufContext context, GfxDrawSurf drawSurf, MaterialTechniqueType techType)
{
    const MaterialTechnique *technique; // [esp+8h] [ebp-8h]
    Material *material; // [esp+Ch] [ebp-4h]

    material = rgp.sortedMaterials[drawSurf.fields.materialSortedIndex];
    technique = Material_GetTechnique(material, techType);
    context.state->material = material;
    context.state->technique = technique;

    if (!technique)
        return 0;

    iassert(!(technique->flags & MTL_TECHFLAG_NEEDS_RESOLVED_SCENE));

    if ((technique->flags & MTL_TECHFLAG_NEEDS_RESOLVED_POST_SUN) != 0 && !rg.distortion)
        return 0;

    if ((techType == TECHNIQUE_EMISSIVE || techType == TECHNIQUE_UNLIT)
        && (technique->flags & 0x10) != 0
        && !context.source->constVersions[4])
    {
        return 0;
    }

    iassert(material);
    if (r_logFile->current.integer)
    {
        RB_LogPrint(va("R_SetMaterial( %s, %s, %s )\n", material->info.name, technique->name, RB_LogTechniqueType(techType)));
    }
    context.state->techType = techType;
    return 1;
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

