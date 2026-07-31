#include <universal/q_shared.h>
#include "r_shade.h"
#include "r_state.h"
#include "rb_logfile.h"
#include "r_dvars.h"
#include "rb_shade.h"
#include <universal/profile.h>
#include "r_buffers.h"
#include "r_utils.h"
#include "r_water.h"


int __cdecl R_ReserveIndexData(GfxCmdBufPrimState *state, int triCount)
{
    uint32_t v2; // edx
    int indexCount; // [esp+8h] [ebp-4h]

    indexCount = 3 * triCount;
    if (3 * triCount > gfxBuf.dynamicIndexBuffer->total)
        MyAssertHandler(
            ".\\r_shade.cpp",
            590,
            0,
            "%s\n\t(indexCount) = %i",
            "(indexCount <= gfxBuf.dynamicIndexBuffer->total)",
            indexCount);
    if (indexCount + gfxBuf.dynamicIndexBuffer->used > gfxBuf.dynamicIndexBuffer->total)
        gfxBuf.dynamicIndexBuffer->used = 0;
    if (!gfxBuf.dynamicIndexBuffer->used)
    {
        v2 = (gfxBuf.dynamicIndexBuffer - gfxBuf.dynamicIndexBufferPool + 1) & 0x80000000;
        if (gfxBuf.dynamicIndexBuffer - gfxBuf.dynamicIndexBufferPool + 1 < 0)
            v2 = 0;
        gfxBuf.dynamicIndexBuffer = &gfxBuf.dynamicIndexBufferPool[v2];
    }
    return gfxBuf.dynamicIndexBuffer->used;
}

int __cdecl R_SetIndexData(GfxCmdBufPrimState *state, uint8_t *indices, int triCount)
{
    int baseIndex; // [esp+60h] [ebp-18h]
    int indexDataSize; // [esp+64h] [ebp-14h]
    uint32_t lockFlags; // [esp+68h] [ebp-10h]
    IDirect3DIndexBuffer9 *ib; // [esp+70h] [ebp-8h]
    uint8_t *bufferData; // [esp+74h] [ebp-4h]

    PROF_SCOPED("RB_SetIndexData");

    baseIndex = R_ReserveIndexData(state, triCount);
    indexDataSize = 6 * triCount;
    ib = gfxBuf.dynamicIndexBuffer->buffer;
    iassert(ib);

    if (gfxBuf.dynamicIndexBuffer->used)
    {
        PROF_SCOPED("LockIndexBufferNoOverwrite");
        lockFlags = D3DLOCK_NOOVERWRITE;
        bufferData = (uint8_t *)R_LockIndexBuffer(ib, 2 * gfxBuf.dynamicIndexBuffer->used, indexDataSize, lockFlags);
    }
    else
    {
        PROF_SCOPED("LockIndexBufferDiscard");
        lockFlags = D3DLOCK_DISCARD;
        bufferData = (uint8_t *)R_LockIndexBuffer(ib, 2 * gfxBuf.dynamicIndexBuffer->used, indexDataSize, lockFlags);
    }

    memcpy(bufferData, indices, indexDataSize);
    R_UnlockIndexBuffer(ib);
    if (state->indexBuffer != ib)
        R_ChangeIndices(state, ib);
    gfxBuf.dynamicIndexBuffer->used += 3 * triCount;
    return baseIndex;
}

void __cdecl R_SetupPassPerPrimArgs(GfxCmdBufContext context)
{
    const MaterialPass *pass; // [esp+0h] [ebp-4h]

    pass = context.state->pass;
    if (pass->perPrimArgCount)
        R_SetPassShaderPrimArguments(context, pass->perPrimArgCount, pass->args);
}

void __cdecl R_HW_SetVertexShaderConstant(
    IDirect3DDevice9 *device,
    uint32_t dest,
    const float *data,
    uint32_t rowCount)
{
    const char *v4; // eax
    const char *v5; // eax
    int hr; // [esp+30h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetVertexShaderConstantF( dest, data, rowCount )\n");
        hr = device->SetVertexShaderConstantF(dest, data, rowCount);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v4 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetVertexShaderConstantF( dest, data, rowCount ) failed: %s\n",
                    94,
                    v4);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    if (r_logFile->current.integer)
    {
        while (rowCount)
        {
            v5 = va("vertex const %i: %g %g %g %g\n", dest, *data, data[1], data[2], data[3]);
            RB_LogPrint(v5);
            ++dest;
            data += 4;
            --rowCount;
        }
    }
}

void __cdecl R_SetVertexShaderConstantFromCode(GfxCmdBufContext context, const MaterialShaderArgument *routingData)
{
    const float *data; // [esp+8h] [ebp-4h]

    if (!R_IsVertexShaderConstantUpToDate(context, routingData))
    {
        if (routingData->u.codeConst.index < 58)
            data = R_GetCodeConstant(context, routingData->u.codeConst.index);
        else
            data = (const float *)R_GetCodeMatrix(context.source, routingData->u.codeConst.index, routingData->u.codeConst.firstRow);
        R_HW_SetVertexShaderConstant(context.state->prim.device, routingData->dest, data, routingData->u.codeConst.rowCount);
    }
}

float *__cdecl R_GetCodeConstant(GfxCmdBufContext context, uint32_t constant)
{
    iassert(context.state);
    iassert(context.source);
    bcassert(constant, CONST_SRC_CODE_COUNT_FLOAT4);

    if (!context.source->constVersions[constant])
    {
        const char * v2 = va(
            "const %i mtl %s tech %s vs %s ps %s",
            constant,
            context.state->material->info.name,
            context.state->technique->name,
            context.state->pass->vertexShader->name,
            context.state->pass->pixelShader->name);
        MyAssertHandler(".\\r_shade.cpp", 29, 0, "%s\n\t%s", "context.source->constVersions[constant]", v2);
    }

    return context.source->input.consts[constant];
}

char __cdecl R_IsVertexShaderConstantUpToDate(GfxCmdBufContext context, const MaterialShaderArgument *routingData)
{
    bcassert(routingData->dest, ARRAY_COUNT(context.state->vertexShaderConstState));

    if (routingData->u.codeConst.index < CONST_SRC_FIRST_CODE_MATRIX)
        return R_IsShaderConstantUpToDate(
            context.source,
            (GfxShaderConstantState *)&context.state->vertexShaderConstState[routingData->dest],
            routingData);
    else
        return R_IsShaderMatrixUpToDate(
            context.source,
            (GfxShaderConstantState *)&context.state->vertexShaderConstState[routingData->dest],
            routingData);
}

char __cdecl R_IsShaderMatrixUpToDate(
    const GfxCmdBufSourceState *source,
    GfxShaderConstantState *constant,
    const MaterialShaderArgument *routingData)
{
    GfxShaderConstantState newState; // [esp+4h] [ebp-10h]
    uint32_t rowCount; // [esp+Ch] [ebp-8h]
    uint32_t rowCounta; // [esp+Ch] [ebp-8h]

    newState.fields.codeConst = routingData->u.codeConst;
    newState.fields.version = source->matrixVersions[(routingData->u.codeConst.index - 58) >> 2];
    if (constant->packed == newState.packed)
        return 1;
    *constant = newState;
    rowCount = routingData->u.codeConst.rowCount;
    iassert( rowCount );
    for (rowCounta = rowCount - 1; rowCounta; --rowCounta)
    {
        ++constant;
        constant->packed = -1LL;
    }
    return 0;
}

char __cdecl R_IsShaderConstantUpToDate(
    const GfxCmdBufSourceState *source,
    GfxShaderConstantState *constant,
    const MaterialShaderArgument *routingData)
{
    GfxShaderConstantState newState; // [esp+4h] [ebp-10h]

    iassert( source );
    newState.fields.codeConst = routingData->u.codeConst;
    newState.fields.version = source->constVersions[routingData->u.codeConst.index];
    if (constant->packed == newState.packed)
        return 1;
    *constant = newState;
    iassert( routingData->u.codeConst.rowCount == 1 );
    return 0;
}

void __cdecl R_SetupPassPerObjectArgs(GfxCmdBufContext context)
{
    const MaterialPass *pass; // [esp+0h] [ebp-4h]

    pass = context.state->pass;
    if (pass->perObjArgCount)
        R_SetPassShaderObjectArguments(context, pass->perObjArgCount, &pass->args[pass->perPrimArgCount]);
}

void __cdecl R_SetPixelShaderConstantFromLiteral(GfxCmdBufState *state, uint32_t dest, const float *literal)
{
    if (dest >= 0x100)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h",
            601,
            0,
            "dest doesn't index ARRAY_COUNT( state->pixelShaderConstState )\n\t%i not in [0, %i)",
            dest,
            256);
    LODWORD(state->pixelShaderConstState[dest]) = -1;
    HIDWORD(state->pixelShaderConstState[dest]) = -1;
    R_HW_SetPixelShaderConstant(state->prim.device, dest, literal, 1u);
}

void __cdecl R_HW_SetPixelShaderConstant(
    IDirect3DDevice9 *device,
    uint32_t dest,
    const float *data,
    uint32_t rowCount)
{
    const char *v4; // eax
    int hr; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetPixelShaderConstantF( dest, data, rowCount )\n");
        hr = device->SetPixelShaderConstantF(dest, data, rowCount);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v4 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetPixelShaderConstantF( dest, data, rowCount ) failed: %s\n",
                    113,
                    v4);
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

int __cdecl R_IsPixelShaderConstantUpToDate(GfxCmdBufContext context, const MaterialShaderArgument *routingData)
{
    GfxShaderConstantState newState; // [esp+4h] [ebp-10h]

    bcassert(routingData->dest, ARRAY_COUNT(context.state->pixelShaderConstState));
    iassert(routingData->u.codeConst.rowCount == 1);
    iassert( context.source );

    newState.fields.codeConst = routingData->u.codeConst;
    newState.fields.version = context.source->constVersions[routingData->u.codeConst.index];

    iassert(newState.fields.version);

    if (context.state->pixelShaderConstState[routingData->dest] == newState.packed)
        return 1;

    context.state->pixelShaderConstState[routingData->dest] = newState.packed;
    return 0;
}

void __cdecl R_SetPixelShaderConstantFromCode(GfxCmdBufContext context, const MaterialShaderArgument *routingData)
{
    iassert(context.source->constVersions[routingData->u.codeConst.index]);
    bcassert(routingData->u.codeConst.index, CONST_SRC_FIRST_CODE_MATRIX);

    R_HW_SetPixelShaderConstant(
        context.state->prim.device,
        routingData->dest,
        R_GetCodeConstant(context, routingData->u.codeConst.index),
        routingData->u.codeConst.rowCount);
}

void __cdecl R_SetupPassCriticalPixelShaderArgs(GfxCmdBufContext context)
{
    const MaterialPass *pass; // [esp+0h] [ebp-4h]

    pass = context.state->pass;
    if (pass->stableArgCount)
        R_SetPassPixelShaderStableArguments(
            context,
            pass->stableArgCount,
            &pass->args[pass->perPrimArgCount + pass->perObjArgCount]);
}

void __cdecl R_SetupPassVertexShaderArgs(GfxCmdBufContext context)
{
    const MaterialPass *pass; // [esp+0h] [ebp-4h]

    pass = context.state->pass;
    if (pass->stableArgCount)
        R_SetPassVertexShaderStableArguments(
            context,
            pass->stableArgCount,
            &pass->args[pass->perPrimArgCount + pass->perObjArgCount]);
}

void __cdecl R_OverrideImage(GfxImage **image, const MaterialTextureDef *texdef)
{
    iassert(image);
    iassert(texdef);

    if ((*image)->mapType == MAPTYPE_2D)
    {
        switch (texdef->semantic)
        {
        case 0u:
        case 1u:
        case 0xBu:
            return;
        case 2u:
            if (r_colorMap->current.integer != 1)
                *image = (GfxImage *)R_OverrideGrayscaleImage(r_colorMap);
            break;
        case 5u:
            if (r_normalMap->current.integer)
            {
                iassert(r_normalMap->current.integer == R_NORMAL_OVERRIDE_NONE);
            }
            else
            {
                *image = rgp.identityNormalMapImage;
            }
            break;
        case 8u:
            if (r_specularMap->current.integer != 1)
                *image = (GfxImage *)R_OverrideGrayscaleImage(r_specularMap);
            break;
        default:
            if (!alwaysfails)
            {
                MyAssertHandler(".\\r_shade.cpp", 473, 1, va("unhandled case %i", texdef->semantic));
            }
            break;
        }
    }
}

#ifdef KISAK_RADIANT
// DOCUMENTED DIVERGENCE (white-xmodel root cause, 2026-07-02): many tools shaders keep
// load-bearing constants as `def` immediates in their bytecode — e.g. the _dtex model
// shaders' packed-UV half-float decompress (vs c8..c12; CTAB only binds the c0/c4
// matrices) and vertcol_shaded's ps c0/c2. D3D9 is supposed to load def constants into
// the register file when the shader is bound, and the binary relies on that. On the
// live editor that guarantee does not hold: DEVPROBE at the model draw showed c10/c11
// live (model positions need them and are correct) while c8/c9 were ZERO — which
// collapses every UV to texel(0,0) and NaNs the fakelight vertex colour = the flat
// white/brown models. Registers persist across shader binds, so any pass whose args
// route a zero-valued code constant to those registers kills them for every later
// def-reliant draw. Fix: re-apply the shader's own def constants whenever the shader
// is bound. Idempotent when the runtime already applied them; def registers are
// disjoint from CTAB-uniform registers within a shader, so no technique arg is
// overridden. The tracked const state for each written dest is invalidated so a later
// technique arg to the same register is not version-skipped. (Editor-only: the game
// targets keep the binary's runtime-reliant behavior.)
static void R_ApplyShaderDefConstants(GfxCmdBufState *state, const void *bytecode,
                                      unsigned sizeDwords, bool pixel)
{
    const uint32_t *tok = (const uint32_t *)bytecode;
    if (!tok || sizeDwords < 2)
        return;
    unsigned i = 1;                              // skip the version token
    while (i < sizeDwords)
    {
        uint32_t t = tok[i];
        if (t == 0x0000FFFF)                     // end token
            break;
        if ((t & 0xFFFF) == 0xFFFE)              // comment (CTAB etc): length in bits 16..30
        {
            i += ((t >> 16) & 0x7FFF) + 1;
            continue;
        }
        uint32_t op = t & 0xFFFF;
        if (op == 0x51 && i + 5 < sizeDwords)    // D3DSIO_DEF: dst token + 4 raw float dwords
        {
            uint32_t dst = tok[i + 1] & 0x7FF;
            if (pixel)
            {
                if (dst < 0x100)
                {
                    LODWORD(state->pixelShaderConstState[dst]) = -1;
                    HIDWORD(state->pixelShaderConstState[dst]) = -1;
                    R_HW_SetPixelShaderConstant(state->prim.device, dst, (const float *)&tok[i + 2], 1u);
                }
            }
            else
            {
                if (dst < 0x20)                  // tracked window; defs beyond it can't collide with args
                {
                    LODWORD(state->vertexShaderConstState[dst]) = -1;
                    HIDWORD(state->vertexShaderConstState[dst]) = -1;
                }
                R_HW_SetVertexShaderConstant(state->prim.device, dst, (const float *)&tok[i + 2], 1u);
            }
            i += 6;
            continue;
        }
        if (op == 0x30) { i += 6; continue; }    // D3DSIO_DEFI: dst + 4 ints
        if (op == 0x2F) { i += 3; continue; }    // D3DSIO_DEFB: dst + 1 bool
        ++i;                                     // other instruction: skip its param tokens
        while (i < sizeDwords && (tok[i] & 0x80000000))
            ++i;
    }
}
#endif

void __cdecl R_SetPixelShader(GfxCmdBufState *state, const MaterialPixelShader *pixelShader)
{
    iassert(pixelShader);
    iassert(pixelShader->prog.ps);

    if (state->pixelShader != pixelShader)
    {
        PROF_SCOPED("RB_SetPixelShader");

        R_HW_SetPixelShader(state->prim.device, pixelShader);
        state->pixelShader = pixelShader;
#ifdef KISAK_RADIANT
        R_ApplyShaderDefConstants(state, pixelShader->prog.loadDef.program,
                                  pixelShader->prog.loadDef.programSize, true);
#endif
    }
}

void __cdecl R_SetVertexShader(GfxCmdBufState *state, const MaterialVertexShader *vertexShader)
{
    if (state->vertexShader != vertexShader)
    {
        iassert(vertexShader);
        iassert(vertexShader->prog.vs);

        PROF_SCOPED("RB_SetVertexShader");

        R_HW_SetVertexShader(state->prim.device, vertexShader);
        state->vertexShader = vertexShader;
#ifdef KISAK_RADIANT
        R_ApplyShaderDefConstants(state, vertexShader->prog.loadDef.program,
                                  vertexShader->prog.loadDef.programSize, false);
#endif
    }
}

void __cdecl R_UpdateVertexDecl(GfxCmdBufState *state)
{
    const MaterialPass *pass; // [esp+40h] [ebp-8h]
    const MaterialVertexShader *vertexShader; // [esp+44h] [ebp-4h]

    pass = state->pass;
    iassert( pass->vertexDecl );
    vertexShader = pass->vertexShader;
    iassert( vertexShader );
    R_SetVertexDecl(&state->prim, pass->vertexDecl);
    iassert( pass->pixelShader );
    iassert( pass->vertexDecl->routing.decl );
    if (!pass->vertexDecl->routing.decl[state->prim.vertDeclType])
        Com_Error(
            ERR_FATAL,
            "Vertex type %i doesn't have the information used by shader %s in material %s\n",
            state->prim.vertDeclType,
            pass->vertexShader->name,
            state->material->info.name);
    R_SetVertexShader(state, vertexShader);
}

void __cdecl R_SetupPass(GfxCmdBufContext context, uint32_t passIndex)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const MaterialPass *pass; // [esp+30h] [ebp-14h]
    const Material *material; // [esp+34h] [ebp-10h]
    const GfxStateBits *refStateBits; // [esp+38h] [ebp-Ch]
    uint32_t stateBits[2]; // [esp+3Ch] [ebp-8h] BYREF

    PROF_SCOPED("R_SetupPass");

    pass = &context.state->technique->passArray[passIndex];
    context.state->pass = pass;
    context.state->passIndex = passIndex;
    material = context.state->material;
    if (material->stateBitsEntry[context.state->techType] >= (uint32_t)material->stateBitsCount)
        MyAssertHandler(
            ".\\r_shade.cpp",
            787,
            0,
            "material->stateBitsEntry[context.state->techType] doesn't index material->stateBitsCount\n\t%i not in [0, %i)",
            material->stateBitsEntry[context.state->techType],
            material->stateBitsCount);
    refStateBits = &material->stateBitsTable[passIndex + material->stateBitsEntry[context.state->techType]];
    stateBits[0] = refStateBits->loadBits[0];
    stateBits[1] = refStateBits->loadBits[1];
    iassert( context.source->viewMode != VIEW_MODE_NONE );
#ifdef KISAK_RADIANT
    // IDB R_SetupPass @0x53c563: for 2D draws the binary forces the low 6 (depth-state) bits of
    // stateBits[1] to 2 — BEFORE the state change — so every 2D pass gets the engine's 2D depth
    // state regardless of the material's authored loadBits. The kisak port dropped this; restore it
    // (gated to the editor: r_shade is shared with the CoD3-based game, which I can't verify against
    // the CoD4Radiant IDB). Pixel-neutral for the depth-cleared texture browser, but correct for any
    // 2D draw over a populated depth buffer (camera/XY overlays).
    if ( context.source->viewMode == VIEW_MODE_2D )
        stateBits[1] = stateBits[1] & 0xFFFFFFC0 | 2;
#endif
    R_SetState(context.state, stateBits);
    if (r_logFile->current.integer)
    {
        RB_LogPrint("---------- R_SetupPass\n");
        v2 = va("---------- vertex shader: %s\n", pass->vertexShader->name);
        RB_LogPrint(v2);
        v3 = va("---------- pixel shader: %s\n", pass->pixelShader->name);
        RB_LogPrint(v3);
        v4 = va("---------- state bits: 0x%08x, 0x%08x\n", stateBits[0], stateBits[1]);
        RB_LogPrint(v4);
    }
    iassert( pass->pixelShader );
    R_SetPixelShader(context.state, pass->pixelShader);
    if (pass->stableArgCount)
        R_SetPassShaderStableArguments(
            context,
            pass->stableArgCount,
            &pass->args[pass->perPrimArgCount + pass->perObjArgCount]);
}

void __cdecl R_SetState(GfxCmdBufState *state, uint32_t *stateBits)
{
    if (*stateBits != state->refStateBits[0])
    {
        R_ChangeState_0(state, *stateBits);
        state->refStateBits[0] = *stateBits;
    }
    if (stateBits[1] != state->refStateBits[1])
    {
        R_ChangeState_1(state, stateBits[1]);
        state->refStateBits[1] = stateBits[1];
    }
}

void __cdecl R_SetVertexShaderConstantFromLiteral(GfxCmdBufState *state, uint32_t dest, const float *literal)
{
    if (dest >= 0x20)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h",
            591,
            0,
            "dest doesn't index ARRAY_COUNT( state->vertexShaderConstState )\n\t%i not in [0, %i)",
            dest,
            32);
    LODWORD(state->vertexShaderConstState[dest]) = -1;
    HIDWORD(state->vertexShaderConstState[dest]) = -1;
    R_HW_SetVertexShaderConstant(state->prim.device, dest, literal, 1u);
}

const MaterialTextureDef *__cdecl R_SetPixelSamplerFromMaterial(
    GfxCmdBufContext context,
    const MaterialShaderArgument *arg,
    const MaterialTextureDef *texDef)
{
    float floatTime; // [esp+4h] [ebp-10h]
    GfxImage *image; // [esp+8h] [ebp-Ch] BYREF
    const Material *material; // [esp+Ch] [ebp-8h]

    material = context.state->material;
    while (texDef->nameHash != arg->u.codeSampler)
    {
        if (++texDef == &material->textureTable[material->textureCount])
        {
            iassert(texDef != &material->textureTable[material->textureCount]); // material is missing a required named texture", 
        }
    }
    if (texDef->semantic == 11)
    {
        image = texDef->u.water->image;
        if (r_drawWater->current.enabled)
            floatTime = context.source->sceneDef.floatTime;
        else
            floatTime = rg.waterFloatTime;
        R_UploadWaterTexture(texDef->u.water, floatTime);
    }
    else
    {
        image = texDef->u.image;
    }

    if (rg.hasAnyImageOverrides)
        R_OverrideImage(&image, texDef);

    R_SetSampler(context, arg->dest, texDef->samplerState, image);
    return texDef;
}

void __cdecl R_SetPassShaderPrimArguments(
    GfxCmdBufContext context,
    uint32_t argCount,
    const MaterialShaderArgument *arg)
{
    while (arg->type == 3)
    {
        R_SetVertexShaderConstantFromCode(context, arg++);
        if (!--argCount)
            return;
    }
    if (!alwaysfails)
        MyAssertHandler(".\\r_shade.cpp", 192, 0, "unreachable");
}

#ifdef KISAK_RADIANT
// idb 0x53B630  R_GetCaseTexture — editor-only (CASE_TEXTURE technique / camera draw_mode 4).
// Returns the rgp.caseTextures[] image whose width/height matches the material's colorMap
// (texel-density visualization), else rgp.whiteImage. The binary keys off material->info.name2
// (a CoD4-only MaterialInfo field absent from kisak's CoD3 MaterialInfo); we replicate the
// INTENT by reading the colorMap GfxImage straight from material->textureTable (the same
// colorMap-extraction the faithful camera draw / TEXPROOF uses). Avoids extending the core
// MaterialInfo struct (CoD3<->CoD4 layout divergence — §11). NEVER returns NULL.
static const GfxImage *R_GetCaseTexture(const Material *material)
{
    // colorMap = first material texture with a real (non-zero-width) image.
    const GfxImage *colorMap = nullptr;
    if (material && material->textureTable)
    {
        for (int ti = 0; ti < material->textureCount; ++ti)
        {
            const GfxImage *img = material->textureTable[ti].u.image;
            if (img && img->width)
            {
                colorMap = img;
                break;
            }
        }
    }
    if (!colorMap)
        return rgp.whiteImage;

    for (int i = 0; i < rgp.caseTextures_count; ++i)
    {
        iassert(rgp.caseTextures[i]);     // idb r_shade.cpp:171 "rgp.caseTextures[caseIndex]"
        if (rgp.caseTextures[i]->width == colorMap->width
            && rgp.caseTextures[i]->height == colorMap->height)
        {
            // One-shot proof (≤4 lines): the CASE_TEXTURE draw path bound a real case image
            // sized to the surface colorMap. Zero overhead after a few faces.
            static int s_caseProof = 0;
            if (s_caseProof < 4)
            {
                ++s_caseProof;
                extern void Radiant_FL_Log( const char *fmt, ... );
                Radiant_FL_Log( "CASEPROOF#%d colorMap %dx%d -> case texture %s %dx%d",
                                s_caseProof, colorMap->width, colorMap->height,
                                rgp.caseTextures[i]->name ? rgp.caseTextures[i]->name : "?",
                                rgp.caseTextures[i]->width, rgp.caseTextures[i]->height );
            }
            return rgp.caseTextures[i];
        }
    }
    return rgp.whiteImage;
}
#endif

void __cdecl R_SetPassShaderObjectArguments(
    GfxCmdBufContext context,
    uint32_t argCount,
    const MaterialShaderArgument *arg)
{
    const GfxImage *image; // [esp+Ch] [ebp-8h]
    uint8_t samplerState; // [esp+13h] [ebp-1h] BYREF

    while (arg->type == MTL_ARG_CODE_VERTEX_CONST)
    {
        R_SetVertexShaderConstantFromCode(context, arg++);
        if (!--argCount)
            return;
    }

    while (arg->type == MTL_ARG_CODE_PIXEL_SAMPLER)
    {
        image = R_GetTextureFromCode(context.source, arg->u.codeSampler, &samplerState);
        if (!image)
        {
#ifdef KISAK_RADIANT
            // idb: the CASE_TEXTURE code sampler (21) has no static code image — it resolves
            // per-surface from the material's colorMap dimensions. Other missing code samplers
            // are still a hard error.
            if (arg->u.codeSampler != TEXTURE_SRC_CODE_CASE_TEXTURE)
                R_TextureFromCodeError(context.source, arg->u.codeSampler);
            image = R_GetCaseTexture(context.state->material);
#else
            R_TextureFromCodeError(context.source, arg->u.codeSampler);
#endif
        }
        R_SetSampler(context, arg->dest, samplerState, image);
        ++arg;
        if (!--argCount)
            return;
    }

    if (!alwaysfails)
        MyAssertHandler(".\\r_shade.cpp", 232, 0, "unreachable");
}

void __cdecl R_SetPassPixelShaderStableArguments(
    GfxCmdBufContext context,
    uint32_t argCount,
    const MaterialShaderArgument *arg)
{
    const char *v3; // eax
    const Material *material; // [esp+Ch] [ebp-8h]
    const MaterialConstantDef *constDef; // [esp+10h] [ebp-4h]

    material = context.state->material;
    while (arg->type < 5u)
    {
        ++arg;
        if (!--argCount)
            return;
    }
    while (arg->type == 5)
    {
        if (!R_IsPixelShaderConstantUpToDate(context, arg))
            R_SetPixelShaderConstantFromCode(context, arg);
        ++arg;
        if (!--argCount)
            return;
    }
    constDef = material->constantTable;
    while (arg->type == 6)
    {
        while (constDef->nameHash != arg->u.codeSampler)
        {
            if (++constDef == &material->constantTable[material->constantCount])
            {
                v3 = va("material '%s' is missing a required named constant", material->info.name);
                MyAssertHandler(
                    ".\\r_shade.cpp",
                    287,
                    0,
                    "%s\n\t%s",
                    "constDef != &material->constantTable[material->constantCount]",
                    v3);
            }
        }
        R_SetPixelShaderConstantFromLiteral(context.state, arg->dest, constDef->literal);
        ++arg;
        if (!--argCount)
            return;
    }
    while (arg->type == 7)
    {
        R_SetPixelShaderConstantFromLiteral(context.state, arg->dest, arg->u.literalConst);
        ++arg;
        if (!--argCount)
            return;
    }
    if (!alwaysfails)
        MyAssertHandler(".\\r_shade.cpp", 300, 0, "unreachable");
}

void __cdecl R_SetPassVertexShaderStableArguments(
    GfxCmdBufContext context,
    uint32_t argCount,
    const MaterialShaderArgument *arg)
{
    while (arg->type < 3u)
    {
        ++arg;
        if (!--argCount)
            return;
    }
    do
    {
        if (arg->type != 3)
            break;
        R_SetVertexShaderConstantFromCode(context, arg++);
        --argCount;
    } while (argCount);
}

void __cdecl R_SetPassShaderStableArguments(
    GfxCmdBufContext context,
    uint32_t argCount,
    const MaterialShaderArgument *arg)
{
    const char *v3; // eax
    const GfxImage *image; // [esp+14h] [ebp-14h]
    const Material *material; // [esp+18h] [ebp-10h]
    const MaterialTextureDef *texDef; // [esp+1Ch] [ebp-Ch]
    uint8_t samplerState; // [esp+23h] [ebp-5h] BYREF
    const MaterialConstantDef *constDef; // [esp+24h] [ebp-4h]

    material = context.state->material;
    constDef = material->constantTable;
    while (!arg->type)
    {
        while (constDef->nameHash != arg->u.codeSampler)
        {
            if (++constDef == &material->constantTable[material->constantCount])
            {
                v3 = va("material '%s' is missing a required named constant", material->info.name);
                MyAssertHandler(
                    ".\\r_shade.cpp",
                    530,
                    0,
                    "%s\n\t%s",
                    "constDef != &material->constantTable[material->constantCount]",
                    v3);
            }
        }
        R_SetVertexShaderConstantFromLiteral(context.state, arg->dest, constDef->literal);
        ++arg;
        if (!--argCount)
            return;
    }
    while (arg->type == 1)
    {
        R_SetVertexShaderConstantFromLiteral(context.state, arg->dest, arg->u.literalConst);
        ++arg;
        if (!--argCount)
            return;
    }
    texDef = material->textureTable;
    while (arg->type == 2)
    {
        texDef = R_SetPixelSamplerFromMaterial(context, arg++, texDef);
        if (!--argCount)
            return;
    }
    while (arg->type == 3)
    {
        R_SetVertexShaderConstantFromCode(context, arg++);
        if (!--argCount)
            return;
    }
    do
    {
        if (arg->type != 4)
            break;
        image = R_GetTextureFromCode(context.source, arg->u.codeSampler, &samplerState);
        if (!image)
        {
#ifdef KISAK_RADIANT
            if (arg->u.codeSampler != TEXTURE_SRC_CODE_CASE_TEXTURE)
                R_TextureFromCodeError(context.source, arg->u.codeSampler);
            image = R_GetCaseTexture(material);
#else
            R_TextureFromCodeError(context.source, arg->u.codeSampler);
#endif
        }
        R_SetSampler(context, arg->dest, samplerState, image);
        ++arg;
        --argCount;
    } while (argCount);
}

void __cdecl R_ChangeObjectPlacement(GfxCmdBufSourceState *source, const GfxScaledPlacement *placement)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    double scale; // [esp+18h] [ebp-54h]
    double scalea; // [esp+18h] [ebp-54h]
    double scaleb; // [esp+18h] [ebp-54h]
    GfxCmdBufSourceState *matrix; // [esp+38h] [ebp-34h]
    float origin[3]; // [esp+3Ch] [ebp-30h] BYREF
    float axis[3][3]; // [esp+48h] [ebp-24h] BYREF

    iassert( placement != source->objectPlacement );
    iassert( placement );
    UnitQuatToAxis(placement->base.quat, axis);
    if (!Vec3IsNormalized(axis[0]))
    {
        scale = Vec3Length(axis[0]);
        v2 = va("(%g %g %g) len %g", axis[0][0], axis[0][1], axis[0][2], scale);
        MyAssertHandler(".\\r_state.cpp", 280, 0, "%s\n\t%s", "Vec3IsNormalized( axis[0] )", v2);
    }
    if (!Vec3IsNormalized(axis[1]))
    {
        scalea = Vec3Length(axis[1]);
        v3 = va("(%g %g %g) len %g", axis[1][0], axis[1][1], axis[1][2], scalea);
        MyAssertHandler(".\\r_state.cpp", 281, 0, "%s\n\t%s", "Vec3IsNormalized( axis[1] )", v3);
    }
    if (!Vec3IsNormalized(axis[2]))
    {
        scaleb = Vec3Length(axis[2]);
        v4 = va("(%g %g %g) len %g", axis[2][0], axis[2][1], axis[2][2], scaleb);
        MyAssertHandler(".\\r_state.cpp", 282, 0, "%s\n\t%s", "Vec3IsNormalized( axis[2] )", v4);
    }
    matrix = R_GetActiveWorldMatrix(source);
    Vec3Sub(placement->base.origin, source->eyeOffset, origin);
    MatrixSet44(*(mat4x4*)&R_GetActiveWorldMatrix(source)->matrices.matrix[0], origin, axis, placement->scale);
    source->objectPlacement = placement;
}

int __cdecl R_SetVertexData(GfxCmdBufState *state, const void *data, int vertexCount, int stride)
{
    IDirect3DVertexBuffer9 *vb; // [esp+6Ch] [ebp-14h]
    volatile int vertexOffset; // [esp+70h] [ebp-10h]
    uint32_t lockFlags; // [esp+74h] [ebp-Ch]
    void *bufferData; // [esp+78h] [ebp-8h]
    int totalSize; // [esp+7Ch] [ebp-4h]

    iassert(vertexCount > 0);

    PROF_SCOPED("RB_SetVertexData");

    totalSize = stride * vertexCount;
    if (stride * vertexCount > gfxBuf.dynamicVertexBuffer->total)
        MyAssertHandler(
            ".\\r_shade.cpp",
            881,
            0,
            "%s\n\t(totalSize) = %i",
            "(totalSize <= gfxBuf.dynamicVertexBuffer->total)",
            totalSize);
    // Binary 0x53C600 @0x53c66c: when the next write would run past the end, WRAP the
    // dynamic VB ring to 0 (the used==0 path below then locks with DISCARD, renaming the
    // buffer).  The port had replaced this wrap with an assert — after continuing past it,
    // Lock(used, totalSize) ran off the buffer end -> D3DERR_INVALIDCALL -> R_FatalLockError.
    // R_ReserveIndexData (above) kept the binary's identical wrap for the index ring.
    if (totalSize + gfxBuf.dynamicVertexBuffer->used > gfxBuf.dynamicVertexBuffer->total)
        gfxBuf.dynamicVertexBuffer->used = 0;
    vb = gfxBuf.dynamicVertexBuffer->buffer;
    iassert( vb );
    lockFlags = gfxBuf.dynamicVertexBuffer->used != 0 ? 4096 : 0x2000;
    {
        PROF_SCOPED("LockVertexBuffer");
        bufferData = R_LockVertexBuffer(vb, gfxBuf.dynamicVertexBuffer->used, totalSize, lockFlags);
    }
    iassert( bufferData );
    //Profile_Begin(167);
    //Profile_Begin(171);
    {
        // LWSS: seems like some weirdness in the original here re: profiler zones
        PROF_SCOPED("RB_memcpy_vb");
        Com_Memcpy(bufferData, data, totalSize);
    }
    //Profile_EndInternal(0);
    //Profile_EndInternal(0);
    R_UnlockVertexBuffer(vb);
    vertexOffset = gfxBuf.dynamicVertexBuffer->used;
    gfxBuf.dynamicVertexBuffer->used += totalSize;
    return vertexOffset;
}