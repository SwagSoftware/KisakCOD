#pragma once
#include "r_rendercmds.h"

enum GfxDrawSceneMethod : __int32
{                                       // ...
    GFX_DRAW_SCENE_NONE = 0x0,
    GFX_DRAW_SCENE_FULLBRIGHT = 0x1,
    GFX_DRAW_SCENE_DEBUGSHADER = 0x2,
    GFX_DRAW_SCENE_STANDARD = 0x3,
};

struct __declspec(align(2)) GfxDrawMethod // sizeof=0x68
{                                       // ...
    GfxDrawSceneMethod drawScene;       // ...
    MaterialTechniqueType baseTechType; // ...
    MaterialTechniqueType emissiveTechType; // ...
    unsigned __int8 litTechType[13][7]; // ...
    // padding byte
};

void __cdecl R_InitDrawMethod();
void R_SetDefaultLitTechTypes();
void __cdecl R_ForceLitTechType(MaterialTechniqueType litTechType);
void __cdecl R_UpdateDrawMethod(GfxBackEndData *data, const GfxViewInfo *viewInfo);

extern GfxDrawMethod gfxDrawMethod;
