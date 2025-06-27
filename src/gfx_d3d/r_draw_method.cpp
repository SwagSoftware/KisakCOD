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
    gfxDrawMethod.litTechType[0][0] = TECHNIQUE_LIT_BEGIN;
    gfxDrawMethod.litTechType[0][1] = TECHNIQUE_LIT_SUN;
    gfxDrawMethod.litTechType[0][2] = TECHNIQUE_LIT_SPOT;
    gfxDrawMethod.litTechType[0][2] = TECHNIQUE_LIT_OMNI;
    gfxDrawMethod.litTechType[0][4] = TECHNIQUE_LIT_SUN_SHADOW;
    gfxDrawMethod.litTechType[0][5] = TECHNIQUE_LIT_SPOT_SHADOW;
    gfxDrawMethod.litTechType[0][6] = TECHNIQUE_LIT_OMNI_SHADOW;

    gfxDrawMethod.litTechType[1][0] = TECHNIQUE_LIT_BEGIN;
    gfxDrawMethod.litTechType[1][1] = TECHNIQUE_LIT_SUN;
    gfxDrawMethod.litTechType[1][2] = TECHNIQUE_LIT_SPOT;
    gfxDrawMethod.litTechType[1][3] = TECHNIQUE_LIT_OMNI;
    gfxDrawMethod.litTechType[1][4] = TECHNIQUE_LIT_SUN_SHADOW;
    gfxDrawMethod.litTechType[1][5] = TECHNIQUE_LIT_SPOT_SHADOW;
    gfxDrawMethod.litTechType[1][6] = TECHNIQUE_LIT_OMNI_SHADOW;

    gfxDrawMethod.litTechType[2][0] = TECHNIQUE_LIT_BEGIN;
    gfxDrawMethod.litTechType[2][1] = TECHNIQUE_LIT_SUN;
    gfxDrawMethod.litTechType[2][2] = TECHNIQUE_LIT_SPOT;
    gfxDrawMethod.litTechType[2][3] = TECHNIQUE_LIT_OMNI;
    gfxDrawMethod.litTechType[2][4] = TECHNIQUE_LIT_SUN_SHADOW;
    gfxDrawMethod.litTechType[2][5] = TECHNIQUE_LIT_SPOT_SHADOW;
    gfxDrawMethod.litTechType[2][6] = TECHNIQUE_LIT_OMNI_SHADOW;

    gfxDrawMethod.litTechType[3][0] = TECHNIQUE_LIT_INSTANCED;
    gfxDrawMethod.litTechType[3][1] = TECHNIQUE_LIT_INSTANCED_SUN;
    gfxDrawMethod.litTechType[3][2] = TECHNIQUE_LIT_INSTANCED_SPOT;
    gfxDrawMethod.litTechType[3][3] = TECHNIQUE_LIT_INSTANCED_OMNI;
    gfxDrawMethod.litTechType[3][4] = TECHNIQUE_LIT_INSTANCED_SUN_SHADOW;
    gfxDrawMethod.litTechType[3][5] = TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW;
    gfxDrawMethod.litTechType[3][6] = TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW;

    gfxDrawMethod.litTechType[4][0] = TECHNIQUE_LIT_INSTANCED;
    gfxDrawMethod.litTechType[4][1] = TECHNIQUE_LIT_INSTANCED_SUN;
    gfxDrawMethod.litTechType[4][2] = TECHNIQUE_LIT_INSTANCED_SPOT;
    gfxDrawMethod.litTechType[4][3] = TECHNIQUE_LIT_INSTANCED_OMNI;
    gfxDrawMethod.litTechType[4][4] = TECHNIQUE_LIT_INSTANCED_SUN_SHADOW;
    gfxDrawMethod.litTechType[4][5] = TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW;
    gfxDrawMethod.litTechType[4][6] = TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW;

    gfxDrawMethod.litTechType[5][0] = TECHNIQUE_LIT_BEGIN;
    gfxDrawMethod.litTechType[5][1] = TECHNIQUE_LIT_SUN;
    gfxDrawMethod.litTechType[5][2] = TECHNIQUE_LIT_SPOT;
    gfxDrawMethod.litTechType[5][3] = TECHNIQUE_LIT_OMNI;
    gfxDrawMethod.litTechType[5][4] = TECHNIQUE_LIT_SUN_SHADOW;
    gfxDrawMethod.litTechType[5][5] = TECHNIQUE_LIT_SPOT_SHADOW;
    gfxDrawMethod.litTechType[5][6] = TECHNIQUE_LIT_OMNI_SHADOW;

    gfxDrawMethod.litTechType[6][0] = TECHNIQUE_LIT_BEGIN;
    gfxDrawMethod.litTechType[6][1] = TECHNIQUE_LIT_SUN;
    gfxDrawMethod.litTechType[6][2] = TECHNIQUE_LIT_SPOT;
    gfxDrawMethod.litTechType[6][3] = TECHNIQUE_LIT_OMNI;
    gfxDrawMethod.litTechType[6][4] = TECHNIQUE_LIT_SUN_SHADOW;
    gfxDrawMethod.litTechType[6][5] = TECHNIQUE_LIT_SPOT_SHADOW;
    gfxDrawMethod.litTechType[6][6] = TECHNIQUE_LIT_OMNI_SHADOW;

    gfxDrawMethod.litTechType[7][0] = TECHNIQUE_LIT_BEGIN;
    gfxDrawMethod.litTechType[7][1] = TECHNIQUE_LIT_SUN;
    gfxDrawMethod.litTechType[7][2] = TECHNIQUE_LIT_SPOT;
    gfxDrawMethod.litTechType[7][3] = TECHNIQUE_LIT_OMNI;
    gfxDrawMethod.litTechType[7][4] = TECHNIQUE_LIT_SUN_SHADOW;
    gfxDrawMethod.litTechType[7][5] = TECHNIQUE_LIT_SPOT_SHADOW;
    gfxDrawMethod.litTechType[7][6] = TECHNIQUE_LIT_OMNI_SHADOW;

    gfxDrawMethod.litTechType[8][0] = TECHNIQUE_LIT_BEGIN;
    gfxDrawMethod.litTechType[8][1] = TECHNIQUE_LIT_SUN;
    gfxDrawMethod.litTechType[8][2] = TECHNIQUE_LIT_SPOT;
    gfxDrawMethod.litTechType[8][3] = TECHNIQUE_LIT_OMNI;
    gfxDrawMethod.litTechType[8][4] = TECHNIQUE_LIT_SUN_SHADOW;
    gfxDrawMethod.litTechType[8][5] = TECHNIQUE_LIT_SPOT_SHADOW;
    gfxDrawMethod.litTechType[8][6] = TECHNIQUE_LIT_OMNI_SHADOW;

    gfxDrawMethod.litTechType[9][0] = TECHNIQUE_LIT_BEGIN;
    gfxDrawMethod.litTechType[9][1] = TECHNIQUE_LIT_SUN;
    gfxDrawMethod.litTechType[9][2] = TECHNIQUE_LIT_SPOT;
    gfxDrawMethod.litTechType[9][3] = TECHNIQUE_LIT_OMNI;
    gfxDrawMethod.litTechType[9][4] = TECHNIQUE_LIT_SUN_SHADOW;
    gfxDrawMethod.litTechType[9][5] = TECHNIQUE_LIT_SPOT_SHADOW;
    gfxDrawMethod.litTechType[9][6] = TECHNIQUE_LIT_OMNI_SHADOW;

    gfxDrawMethod.litTechType[10][0] = TECHNIQUE_NONE;
    gfxDrawMethod.litTechType[10][1] = TECHNIQUE_NONE;
    gfxDrawMethod.litTechType[10][2] = TECHNIQUE_NONE;
    gfxDrawMethod.litTechType[10][3] = TECHNIQUE_NONE;
    gfxDrawMethod.litTechType[10][4] = TECHNIQUE_NONE;
    gfxDrawMethod.litTechType[10][5] = TECHNIQUE_NONE;
    gfxDrawMethod.litTechType[10][6] = TECHNIQUE_NONE;

    gfxDrawMethod.litTechType[11][0] = TECHNIQUE_LIT_BEGIN;
    gfxDrawMethod.litTechType[11][1] = TECHNIQUE_LIT_SUN;
    gfxDrawMethod.litTechType[11][2] = TECHNIQUE_LIT_SPOT;
    gfxDrawMethod.litTechType[11][3] = TECHNIQUE_LIT_OMNI;
    gfxDrawMethod.litTechType[11][4] = TECHNIQUE_LIT_SUN_SHADOW;
    gfxDrawMethod.litTechType[11][5] = TECHNIQUE_LIT_SPOT_SHADOW;
    gfxDrawMethod.litTechType[11][6] = TECHNIQUE_LIT_OMNI_SHADOW;

    gfxDrawMethod.litTechType[12][0] = TECHNIQUE_NONE;
    gfxDrawMethod.litTechType[12][1] = TECHNIQUE_NONE;
    gfxDrawMethod.litTechType[12][2] = TECHNIQUE_NONE;
    gfxDrawMethod.litTechType[12][3] = TECHNIQUE_NONE;
    gfxDrawMethod.litTechType[12][4] = TECHNIQUE_NONE;
    gfxDrawMethod.litTechType[12][5] = TECHNIQUE_NONE;
    gfxDrawMethod.litTechType[12][6] = TECHNIQUE_NONE;
}

void __cdecl R_ForceLitTechType(MaterialTechniqueType litTechType)
{
    unsigned int surfType; // [esp+0h] [ebp-8h]
    unsigned int lightType; // [esp+4h] [ebp-4h]

    for (surfType = 0; surfType < 13; ++surfType)
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

    iassert(viewInfo->shadowableLightCount <= 255);

    for (primaryLightIndex = 0; primaryLightIndex < viewInfo->shadowableLightCount; ++primaryLightIndex)
    {
        lightTypea = viewInfo->shadowableLights[primaryLightIndex].type;
        lightType = lightTypea + (Com_BitCheckAssert(data->shadowableLightHasShadowMap, primaryLightIndex, 32) ? 3 : 0);
        for (surfType = 0; surfType < 13; ++surfType)
            data->primaryLightTechType[surfType][primaryLightIndex] = gfxDrawMethod.litTechType[surfType][lightType];
    }
}

