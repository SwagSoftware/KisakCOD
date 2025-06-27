#include "rb_shade.h"
#include "rb_logfile.h"
#include "rb_state.h"
#include "rb_stats.h"
#include "rb_pixelcost.h"
#include "r_shade.h"
#include "r_state.h"
#include "r_draw_bsp.h"
#include <universal/profile.h>



void __cdecl R_SetVertexDecl(GfxCmdBufPrimState *primState, const MaterialVertexDeclaration *vertexDecl)
{
    IDirect3DVertexDeclaration9 *v3; // [esp+0h] [ebp-40h]
    int hr; // [esp+34h] [ebp-Ch]
    IDirect3DDevice9 *device; // [esp+3Ch] [ebp-4h]

    if (vertexDecl)
        v3 = vertexDecl->routing.decl[primState->vertDeclType];
    else
        v3 = 0;

    if (primState->vertexDecl != v3)
    {
        PROF_SCOPED("RB_SetVertexDeclaration");
        device = primState->device;
        iassert(device);
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("device->SetVertexDeclaration( decl )\n");

            hr = device->SetVertexDeclaration(v3);
            if (hr < 0)
            {
                do
                {
                    ++g_disableRendering;
                    Com_Error(
                        ERR_FATAL,
                        "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h (%i) device->SetVertexDeclaration( decl ) failed: %s\n",
                        674,
                        R_ErrorDescription(hr));
                } while (alwaysfails);
            }
        } while (alwaysfails);
        primState->vertexDecl = v3;
    }
}

void __cdecl RB_ClearPixelShader()
{
    if (gfxCmdBufState.pixelShader)
        R_HW_SetPixelShader(gfxCmdBufState.prim.device, 0);
    gfxCmdBufState.pixelShader = 0;
}

void __cdecl R_HW_SetPixelShader(IDirect3DDevice9 *device, const MaterialPixelShader *mtlShader)
{
    int v2; // eax
    HRESULT hr; // [esp+4h] [ebp-4h]

    iassert(device);

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetPixelShader( mtlShader ? mtlShader->prog.ps : 0 )\n");
        if (mtlShader)
        {
            v2 = device->SetPixelShader(mtlShader->prog.ps);
        }
        else
        {
            v2 = device->SetPixelShader(0);
        }
        hr = v2;
        if (v2 < 0)
        {
            do
            {
                ++g_disableRendering;
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetPixelShader( mtlShader ? mtlShader->prog.ps : "
                    "0 ) failed: %s\n",
                    454,
                    R_ErrorDescription(hr));
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl RB_ClearVertexShader()
{
    if (gfxCmdBufState.vertexShader)
        R_HW_SetVertexShader(gfxCmdBufState.prim.device, 0);
    gfxCmdBufState.vertexShader = 0;
}

void __cdecl R_HW_SetVertexShader(IDirect3DDevice9 *device, const MaterialVertexShader *mtlShader)
{
    int v2; // eax
    HRESULT hr; // [esp+4h] [ebp-4h]

    iassert(device);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetVertexShader( mtlShader ? mtlShader->prog.vs : 0 )\n");
        if (mtlShader)
        {
            v2 = device->SetVertexShader(mtlShader->prog.vs);
        }
        else
        {
            v2 = device->SetVertexShader(0);
        }
        hr = v2;
        if (v2 < 0)
        {
            do
            {
                ++g_disableRendering;
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetVertexShader( mtlShader ? mtlShader->prog.vs :"
                    " 0 ) failed: %s\n",
                    461,
                    R_ErrorDescription(hr));
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl RB_ClearVertexDecl()
{
    if (gfxCmdBufState.prim.vertexDecl)
        R_SetVertexDecl(&gfxCmdBufState.prim, 0);
    iassert(gfxCmdBufState.prim.vertexDecl == NULL);
}

void __cdecl RB_SetTessTechnique(const Material *material, MaterialTechniqueType techType)
{
    iassert(material);
    if (gfxCmdBufState.origMaterial != material || gfxCmdBufState.origTechType != techType)
    {
        if (tess.indexCount)
            RB_EndTessSurface();
        RB_BeginSurface(material, techType);
    }
}

void __cdecl RB_BeginSurface(const Material *material, MaterialTechniqueType techType)
{
    iassert(tess.indexCount == 0);
    iassert(tess.vertexCount == 0);

    iassert(material);
    iassert(!g_primStats);

    if (r_logFile->current.integer)
    {
        RB_LogPrint(va("---------- RB_BeginSurface( %s, %s )\n", material->info.name, RB_LogTechniqueType(techType)));
    }
    tess.firstVertex = 0;
    tess.lastVertex = 0;
    gfxCmdBufState.material = material;
    gfxCmdBufState.techType = techType;
    gfxCmdBufState.prim.vertDeclType = VERTDECL_GENERIC;
    gfxCmdBufState.origMaterial = material;
    gfxCmdBufState.origTechType = techType;
    if (pixelCostMode > GFX_PIXEL_COST_MODE_MEASURE_MSEC)
    {
        gfxCmdBufState.material = R_PixelCost_GetAccumulationMaterial(material);
        gfxCmdBufState.techType = TECHNIQUE_UNLIT;
    }
    gfxCmdBufState.technique = Material_GetTechnique(gfxCmdBufState.material, gfxCmdBufState.techType);
    iassert(gfxCmdBufState.technique);
}

void __cdecl RB_EndTessSurface()
{
    {
        PROF_SCOPED("EndSurface_Standard");
        RB_EndSurfacePrologue();
        RB_DrawTessSurface();
    }

    RB_EndSurfaceEpilogue();
}

GfxPrimStats *RB_EndSurfacePrologue()
{
    GfxPrimStats *result; // eax

    iassert(gfxCmdBufState.material);
    tess.finishedFilling = 1;
    iassert(g_primStats);
    g_primStats->dynamicIndexCount += tess.indexCount;
    result = g_primStats;
    g_primStats->dynamicVertexCount += tess.vertexCount;
    return result;
}

void RB_EndSurfaceEpilogue()
{
    iassert(tess.vertexCount == 0);
    iassert(tess.indexCount == 0);

    g_primStats = 0;
    tess.finishedFilling = 0;
}

void RB_DrawTessSurface()
{
    GfxViewport viewport; // [esp+30h] [ebp-1Ch] BYREF
    GfxDrawPrimArgs args; // [esp+40h] [ebp-Ch] BYREF

    iassert(tess.indexCount);

    PROF_SCOPED("EndSurface_Standard");

    if (gfxCmdBufSourceState.viewportIsDirty)
    {
        R_GetViewport(&gfxCmdBufSourceState, &viewport);
        R_SetViewport(&gfxCmdBufState, &viewport);
        R_UpdateViewport(&gfxCmdBufSourceState, &viewport);
    }

    args.vertexCount = tess.vertexCount;
    args.triCount = tess.indexCount / 3;
    iassert(gfxCmdBufState.prim.vertDeclType == VERTDECL_GENERIC);
    args.baseIndex = R_SetIndexData(&gfxCmdBufState.prim, (unsigned __int8 *)tess.indices, tess.indexCount / 3);
    R_DrawTessTechnique(gfxCmdBufContext, &args);
    tess.indexCount = 0;
    tess.vertexCount = 0;
}

void __cdecl R_DrawTessTechnique(GfxCmdBufContext context, const GfxDrawPrimArgs *args)
{
    const MaterialTechnique *technique; // [esp+38h] [ebp-14h]
    IDirect3DVertexBuffer9 *vb; // [esp+3Ch] [ebp-10h]
    unsigned int vertexOffset; // [esp+40h] [ebp-Ch]
    bool isPixelCostEnabled; // [esp+47h] [ebp-5h]
    unsigned int passIndex; // [esp+48h] [ebp-4h]

    PROF_SCOPED("RB_DrawTechnique");

    iassert(dx.d3d9 && dx.device);
    iassert(context.state->material);

    technique = context.state->technique;
    iassert(technique);

    if (r_logFile->current.integer)
    {
        RB_LogPrint(va("\n---------- R_DrawTechnique( %s ) ----------\n", technique->name));
    }
    isPixelCostEnabled = pixelCostMode != GFX_PIXEL_COST_MODE_OFF;

    if (pixelCostMode)
        R_PixelCost_BeginSurface(context);

    iassert(context.state->prim.vertDeclType == VERTDECL_GENERIC);
    R_CheckVertexDataOverflow(32 * tess.vertexCount);
    vertexOffset = R_SetVertexData(context.state, &tess.verts, tess.vertexCount, 32);
    for (passIndex = 0; passIndex < technique->passCount; ++passIndex)
    {
        R_SetupPass(context, passIndex);
        R_UpdateVertexDecl(context.state);
        R_SetupPassCriticalPixelShaderArgs(context);
        vb = gfxBuf.dynamicVertexBuffer->buffer;
        iassert(vb);
        R_SetStreamSource(&context.state->prim, vb, vertexOffset, 0x20u);
        R_SetupPassPerObjectArgs(context);
        R_SetupPassPerPrimArgs(context);
        R_DrawIndexedPrimitive(&context.state->prim, args);
    }

    if (isPixelCostEnabled)
        R_PixelCost_EndSurface(context);

    if (r_logFile->current.integer)
        RB_LogPrint("\n");
}

void __cdecl RB_TessOverflow()
{
    GfxPrimStats *primStats; // [esp+0h] [ebp-4h]

    primStats = g_primStats;
    RB_EndTessSurface();
    RB_BeginSurface(gfxCmdBufState.origMaterial, gfxCmdBufState.origTechType);
    g_primStats = primStats;
}

