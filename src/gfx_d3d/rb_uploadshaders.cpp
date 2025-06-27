#include "rb_uploadshaders.h"
#include <qcommon/threads.h>
#include "r_dvars.h"
#include "rb_backend.h"
#include "r_shade.h"
#include "rb_state.h"
#include "r_state.h"
#include "rb_shade.h"
#include "r_draw_method.h"

$C28D828B354D71D7584331F40DBDE744 mtlUploadGlob;

void __cdecl Material_UploadShaders(MaterialTechniqueSet *techSet)
{
    if (!Sys_IsDatabaseThread())
        MyAssertHandler(".\\rb_uploadshaders.cpp", 26, 0, "%s", "Sys_IsDatabaseThread()");
    if (mtlUploadGlob.put - mtlUploadGlob.get >= 0x400)
        MyAssertHandler(
            ".\\rb_uploadshaders.cpp",
            27,
            0,
            "%s",
            "mtlUploadGlob.put - mtlUploadGlob.get < MAX_TECHNIQUE_SETS");
    if (r_preloadShaders->current.enabled)
    {
        techSet->hasBeenUploaded = 0;
        mtlUploadGlob.techSet[mtlUploadGlob.put % 0x400] = techSet;
        ++mtlUploadGlob.put;
    }
}

void __cdecl RB_SetUploadMaterialArg(const MaterialShaderArgument *arg)
{
    switch (arg->type)
    {
    case 0u:
    case 1u:
    case 3u:
    case 5u:
    case 6u:
    case 7u:
        return;
    case 2u:
        R_SetSampler(gfxCmdBufContext, arg->dest, 0xAu, rgp.blackImage);
        break;
    case 4u:
        if (arg->u.codeSampler == 3)
        {
            R_SetSampler(gfxCmdBufContext, arg->dest, 0xAu, rgp.blackImage3D);
        }
        else if (arg->u.codeSampler == 26)
        {
            R_SetSampler(gfxCmdBufContext, arg->dest, 0xAu, rgp.blackImageCube);
        }
        else
        {
            R_SetSampler(gfxCmdBufContext, arg->dest, 0xAu, rgp.blackImage);
        }
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\rb_uploadshaders.cpp", 89, 1, "inconceivable");
        break;
    }
}

unsigned int __cdecl RB_UploadMaterialPassVertexDecl(
    GfxCmdBufPrimState *primState,
    MaterialVertexDeclaration *vertexDecl,
    MaterialVertexDeclType vertDeclType,
    void *data,
    unsigned int stride)
{
    if (!vertexDecl->routing.decl[vertDeclType])
        return 0;
    primState->vertDeclType = vertDeclType;
    R_SetVertexDecl(primState, vertexDecl);
    //(dx.device->DrawPrimitiveUP)(dx.device, dx.device, 4, 1, data, stride);
    dx.device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, data, stride);
    return 1;
}

unsigned int RB_UploadMaterialPass(
    const MaterialPass *pass,
    MaterialVertexDeclType vertDeclType)
{
    unsigned int v4; // [esp-Ch] [ebp-B4h]
    unsigned int v5; // [esp-Ch] [ebp-B4h]
    unsigned int v6; // [esp-Ch] [ebp-B4h]
    unsigned int v7; // [esp-Ch] [ebp-B4h]
    GfxCmdBufPrimState *p_prim; // [esp-8h] [ebp-B0h]
    unsigned int v9[37]; // [esp-4h] [ebp-ACh] BYREF
    unsigned int vertMem_136; // [esp+90h] [ebp-18h]
    unsigned int vertMem_140; // [esp+94h] [ebp-14h]
    unsigned int argIter; // [esp+9Ch] [ebp-Ch]
    unsigned int argCount; // [esp+A0h] [ebp-8h]
    unsigned int retaddr; // [esp+A8h] [ebp+0h]

    //argIter = a1;
    //argCount = retaddr;
    if (!pass->vertexShader)
        MyAssertHandler(".\\rb_uploadshaders.cpp", 110, 0, "%s", "pass->vertexShader");
    if (!pass->vertexShader->prog.vs)
        MyAssertHandler(".\\rb_uploadshaders.cpp", 111, 0, "%s", "pass->vertexShader->prog.vs");
    if (!pass->pixelShader)
        MyAssertHandler(".\\rb_uploadshaders.cpp", 112, 0, "%s", "pass->pixelShader");
    if (!pass->pixelShader->prog.ps)
        MyAssertHandler(".\\rb_uploadshaders.cpp", 113, 0, "%s", "pass->pixelShader->prog.ps");
    if (!pass->vertexDecl)
        MyAssertHandler(".\\rb_uploadshaders.cpp", 114, 0, "%s", "pass->vertexDecl");
    R_SetVertexShader(gfxCmdBufContext.state, pass->vertexShader);
    R_SetPixelShader(gfxCmdBufContext.state, pass->pixelShader);
    vertMem_140 = pass->stableArgCount + pass->perObjArgCount + pass->perPrimArgCount;
    for (vertMem_136 = 0; vertMem_136 < vertMem_140; ++vertMem_136)
        RB_SetUploadMaterialArg(&pass->args[vertMem_136]);
    memset(v9, 0, 0x90u);
    p_prim = &gfxCmdBufContext.state->prim;
    if (vertDeclType)
    {
        v4 = RB_UploadMaterialPassVertexDecl(p_prim, pass->vertexDecl, vertDeclType, v9, 0x30u);
    }
    else
    {
        v5 = RB_UploadMaterialPassVertexDecl(p_prim, pass->vertexDecl, VERTDECL_GENERIC, v9, 0x30u);
        v6 = v5 + RB_UploadMaterialPassVertexDecl(p_prim, pass->vertexDecl, VERTDECL_PACKED, v9, 0x30u);
        v7 = v6 + RB_UploadMaterialPassVertexDecl(p_prim, pass->vertexDecl, VERTDECL_WORLD, v9, 0x30u);
        v4 = v7 + RB_UploadMaterialPassVertexDecl(p_prim, pass->vertexDecl, VERTDECL_STATICMODELCACHE, v9, 0x30u);
    }
    gfxCmdBufContext.state->prim.vertDeclType = VERTDECL_GENERIC;
    return v4;
}

unsigned int __cdecl RB_UploadMaterialTechnique(const MaterialTechnique *tech, unsigned int techType)
{
    unsigned int uploadCount; // [esp+0h] [ebp-Ch]
    MaterialVertexDeclType vertDeclType; // [esp+4h] [ebp-8h]
    unsigned int passIter; // [esp+8h] [ebp-4h]

    if (!strncmp(tech->name, "lm_", 3u))
    {
        vertDeclType = VERTDECL_WORLD;
    }
    else if (!strncmp(tech->name, "lp_", 3u)
        || !strncmp(tech->name, "lprobe_", 7u)
        || !strncmp(tech->name, "effect_", 7u))
    {
        vertDeclType = VERTDECL_PACKED;
        if (techType >= 0xE && techType < 0x15)
            vertDeclType = VERTDECL_STATICMODELCACHE;
    }
    else
    {
        vertDeclType = VERTDECL_GENERIC;
    }
    uploadCount = 0;
    for (passIter = 0; passIter < tech->passCount; ++passIter)
        uploadCount += RB_UploadMaterialPass(&tech->passArray[passIter], vertDeclType);
    return uploadCount;
}

void __cdecl RB_UploadShaderStep()
{
    unsigned int uploadCount; // [esp+0h] [ebp-Ch]
    MaterialTechnique *tech; // [esp+4h] [ebp-8h]
    MaterialTechniqueSet *techSet; // [esp+8h] [ebp-4h]

    if (!Sys_IsRenderThread() && !Sys_IsMainThread())
        MyAssertHandler(".\\rb_uploadshaders.cpp", 179, 0, "%s", "Sys_IsRenderThread() || Sys_IsMainThread()");
    if (backEndData->viewInfoCount && gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD)
    {
        Material_ClearShaderUploadList();
    }
    else
    {
        uploadCount = 0;
        while (mtlUploadGlob.get != mtlUploadGlob.put && !uploadCount)
        {
            techSet = mtlUploadGlob.techSet[mtlUploadGlob.get % 0x400];
            if (mtlUploadGlob.techTypeIter)
            {
                while (1)
                {
                LABEL_16:
                    tech = techSet->techniques[mtlUploadGlob.techTypeIter];
                    if (tech)
                    {
                        if ((tech->flags & 0x8000) == 0)
                        {
                            tech->flags |= 0x8000u;
                            if (tech->passCount)
                            {
                                if (!tech->passArray[0].pixelShader)
                                    MyAssertHandler(".\\rb_uploadshaders.cpp", 210, 0, "%s", "tech->passArray[0].pixelShader");
                                if (tech->passArray[0].pixelShader->prog.loadDef.loadForRenderer == r_rendererInUse->current.integer)
                                    uploadCount += RB_UploadMaterialTechnique(tech, mtlUploadGlob.techTypeIter);
                            }
                        }
                    }
                    if (++mtlUploadGlob.techTypeIter == 30)
                        break;
                    if (uploadCount)
                        return;
                }
                mtlUploadGlob.techTypeIter = 0;
            }
            else if (!techSet->hasBeenUploaded && !Material_WouldTechniqueSetBeOverridden(techSet))
            {
                techSet->hasBeenUploaded = 1;
                goto LABEL_16;
            }
            ++mtlUploadGlob.get;
        }
    }
}