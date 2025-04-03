#include "r_draw_method.h"
#include "r_dvars.h"
#include <bgame/bg_local.h>


//struct GfxDrawMethod gfxDrawMethod 85b93648     gfx_d3d : r_draw_method.obj
GfxDrawMethod gfxDrawMethod;

void __cdecl R_InitDrawMethod()
{
    if (r_fullbright->current.enabled)
    {
        gfxDrawMethod.drawScene = GFX_DRAW_SCENE_FULLBRIGHT;
        gfxDrawMethod.baseTechType = TECHNIQUE_UNLIT;
        gfxDrawMethod.emissiveTechType = TECHNIQUE_UNLIT;
        R_ForceLitTechType(TECHNIQUE_UNLIT);
    }
    else if (r_debugShader->current.integer)
    {
        gfxDrawMethod.drawScene = GFX_DRAW_SCENE_DEBUGSHADER;
        gfxDrawMethod.baseTechType = TECHNIQUE_DEBUG_BUMPMAP;
        gfxDrawMethod.emissiveTechType = TECHNIQUE_DEBUG_BUMPMAP;
        R_ForceLitTechType(TECHNIQUE_DEBUG_BUMPMAP);
    }
    else
    {
        gfxDrawMethod.drawScene = GFX_DRAW_SCENE_STANDARD;
        gfxDrawMethod.baseTechType = TECHNIQUE_LIT_BEGIN;
        gfxDrawMethod.emissiveTechType = TECHNIQUE_EMISSIVE;
        R_SetDefaultLitTechTypes();
    }
}

void R_SetDefaultLitTechTypes()
{
    *(unsigned int *)&gfxDrawMethod.litTechType[0][0] = 201984007;
    *(_WORD *)&gfxDrawMethod.litTechType[0][4] = 2825;
    gfxDrawMethod.litTechType[0][6] = 13;
    *(unsigned int *)&gfxDrawMethod.litTechType[1][0] = 201984007;
    *(_WORD *)&gfxDrawMethod.litTechType[1][4] = 2825;
    gfxDrawMethod.litTechType[1][6] = 13;
    *(unsigned int *)&gfxDrawMethod.litTechType[2][0] = 201984007;
    *(_WORD *)&gfxDrawMethod.litTechType[2][4] = 2825;
    gfxDrawMethod.litTechType[2][6] = 13;
    *(unsigned int *)&gfxDrawMethod.litTechType[3][0] = 319885070;
    *(_WORD *)&gfxDrawMethod.litTechType[3][4] = 4624;
    gfxDrawMethod.litTechType[3][6] = 20;
    *(unsigned int *)&gfxDrawMethod.litTechType[4][0] = 319885070;
    *(_WORD *)&gfxDrawMethod.litTechType[4][4] = 4624;
    gfxDrawMethod.litTechType[4][6] = 20;
    *(unsigned int *)&gfxDrawMethod.litTechType[5][0] = 201984007;
    *(_WORD *)&gfxDrawMethod.litTechType[5][4] = 2825;
    gfxDrawMethod.litTechType[5][6] = 13;
    *(unsigned int *)&gfxDrawMethod.litTechType[6][0] = 201984007;
    *(_WORD *)&gfxDrawMethod.litTechType[6][4] = 2825;
    gfxDrawMethod.litTechType[6][6] = 13;
    *(unsigned int *)&gfxDrawMethod.litTechType[7][0] = 201984007;
    *(_WORD *)&gfxDrawMethod.litTechType[7][4] = 2825;
    gfxDrawMethod.litTechType[7][6] = 13;
    *(unsigned int *)&gfxDrawMethod.litTechType[8][0] = 201984007;
    *(_WORD *)&gfxDrawMethod.litTechType[8][4] = 2825;
    gfxDrawMethod.litTechType[8][6] = 13;
    *(unsigned int *)&gfxDrawMethod.litTechType[9][0] = 201984007;
    *(_WORD *)&gfxDrawMethod.litTechType[9][4] = 2825;
    gfxDrawMethod.litTechType[9][6] = 13;
    memset(gfxDrawMethod.litTechType[10], 36, sizeof(gfxDrawMethod.litTechType[10]));
    *(unsigned int *)&gfxDrawMethod.litTechType[11][0] = 201984007;
    *(_WORD *)&gfxDrawMethod.litTechType[11][4] = 2825;
    gfxDrawMethod.litTechType[11][6] = 13;
    memset(gfxDrawMethod.litTechType[12], 36, sizeof(gfxDrawMethod.litTechType[12]));
}

void __cdecl R_ForceLitTechType(MaterialTechniqueType litTechType)
{
    unsigned int surfType; // [esp+0h] [ebp-8h]
    unsigned int lightType; // [esp+4h] [ebp-4h]

    for (surfType = 0; surfType < 0xD; ++surfType)
    {
        for (lightType = 0; lightType < 7; ++lightType)
            gfxDrawMethod.litTechType[surfType][lightType] = litTechType;
    }
}

void __cdecl R_UpdateDrawMethod(GfxBackEndData *data, const GfxViewInfo *viewInfo)
{
    unsigned int primaryLightIndex; // [esp+4h] [ebp-Ch]
    unsigned int surfType; // [esp+8h] [ebp-8h]
    unsigned int lightTypea; // [esp+Ch] [ebp-4h]
    unsigned int lightType; // [esp+Ch] [ebp-4h]

    if (viewInfo->shadowableLightCount > 0xFF)
        MyAssertHandler(
            ".\\r_draw_method.cpp",
            180,
            0,
            "%s\n\t(viewInfo->shadowableLightCount) = %i",
            "(viewInfo->shadowableLightCount <= 255)",
            viewInfo->shadowableLightCount);
    for (primaryLightIndex = 0; primaryLightIndex < viewInfo->shadowableLightCount; ++primaryLightIndex)
    {
        lightTypea = viewInfo->shadowableLights[primaryLightIndex].type;
        lightType = lightTypea + (Com_BitCheckAssert(data->shadowableLightHasShadowMap, primaryLightIndex, 32) ? 3 : 0);
        for (surfType = 0; surfType < 0xD; ++surfType)
            data->primaryLightTechType[surfType][primaryLightIndex] = gfxDrawMethod.litTechType[surfType][lightType];
    }
}

