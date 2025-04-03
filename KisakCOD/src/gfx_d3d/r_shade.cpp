#include "r_shade.h"
#include "r_state.h"
#include "rb_logfile.h"
#include "r_dvars.h"
#include "rb_shade.h"


void __cdecl R_SetupPassPerPrimArgs(GfxCmdBufContext context)
{
    const MaterialPass *pass; // [esp+0h] [ebp-4h]

    pass = context.state->pass;
    if (pass->perPrimArgCount)
        R_SetPassShaderPrimArguments(context, pass->perPrimArgCount, pass->args);
}

void __cdecl R_SetVertexShaderConstantFromCode(GfxCmdBufContext context, const MaterialShaderArgument *routingData)
{
    __int64 v2; // [esp-10h] [ebp-1Ch]
    __int64 v3; // [esp-8h] [ebp-14h]
    const float *data; // [esp+8h] [ebp-4h]

    if (!R_IsVertexShaderConstantUpToDate(context, routingData))
    {
        if (routingData->u.codeConst.index < 0x3Au)
            data = (const float *)R_GetCodeConstant(context, routingData->u.codeConst.index);
        else
            data = (const float *)R_GetCodeMatrix(
                context.source,
                routingData->u.codeConst.index,
                routingData->u.codeConst.firstRow);
        HIDWORD(v3) = routingData->u.codeConst.rowCount;
        LODWORD(v3) = data;
        HIDWORD(v2) = routingData->dest;
        LODWORD(v2) = context.state->prim.device;
        R_HW_SetVertexShaderConstant(v2, v3);
    }
}

void __cdecl R_HW_SetVertexShaderConstant(__int64 device, __int64 data)
{
    const char *v2; // eax
    const char *v3; // eax
    int hr; // [esp+30h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetVertexShaderConstantF( dest, data, rowCount )\n");
        hr = (*(int(__stdcall **)(unsigned int, unsigned int, unsigned int, unsigned int))(*(unsigned int *)device + 376))(
            device,
            HIDWORD(device),
            data,
            HIDWORD(data));
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\trees\\cod3\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetVertexShaderConstantF( dest, data, rowCount ) failed: %s\n",
                    94,
                    v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    if (r_logFile->current.integer)
    {
        while (HIDWORD(data))
        {
            v3 = va(
                "vertex const %i: %g %g %g %g\n",
                HIDWORD(device),
                *(float *)data,
                *(float *)(data + 4),
                *(float *)(data + 8),
                *(float *)(data + 12));
            RB_LogPrint(v3);
            ++HIDWORD(device);
            LODWORD(data) = data + 16;
            --HIDWORD(data);
        }
    }
}

GfxCmdBufInput *__cdecl R_GetCodeConstant(GfxCmdBufContext context, unsigned int constant)
{
    const char *v2; // eax

    if (!context.state)
        MyAssertHandler(".\\r_shade.cpp", 25, 0, "%s", "context.state");
    if (!context.source)
        MyAssertHandler(".\\r_shade.cpp", 26, 0, "%s", "context.source");
    if (constant >= 0x3A)
        MyAssertHandler(
            ".\\r_shade.cpp",
            27,
            0,
            "constant doesn't index CONST_SRC_CODE_COUNT_FLOAT4\n\t%i not in [0, %i)",
            constant,
            58);
    if (!context.source->constVersions[constant])
    {
        v2 = va(
            "const %i mtl %s tech %s vs %s ps %s",
            constant,
            context.state->material->info.name,
            context.state->technique->name,
            context.state->pass->vertexShader->name,
            context.state->pass->pixelShader->name);
        MyAssertHandler(".\\r_shade.cpp", 29, 0, "%s\n\t%s", "context.source->constVersions[constant]", v2);
    }
    return (GfxCmdBufInput *)((char *)&context.source->input + 16 * constant);
}

char __cdecl R_IsVertexShaderConstantUpToDate(GfxCmdBufContext context, const MaterialShaderArgument *routingData)
{
    if (routingData->dest >= 0x20u)
        MyAssertHandler(
            ".\\r_shade.cpp",
            89,
            0,
            "routingData->dest doesn't index ARRAY_COUNT( context.state->vertexShaderConstState )\n\t%i not in [0, %i)",
            routingData->dest,
            32);
    if (routingData->u.codeConst.index < 0x3Au)
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
    unsigned int rowCount; // [esp+Ch] [ebp-8h]
    unsigned int rowCounta; // [esp+Ch] [ebp-8h]

    newState.fields.codeConst = (MaterialArgumentCodeConst)routingData->u.codeSampler;
    newState.fields.version = source->matrixVersions[(routingData->u.codeConst.index - 58) >> 2];
    if (constant->packed == newState.packed)
        return 1;
    *constant = newState;
    rowCount = routingData->u.codeConst.rowCount;
    if (!routingData->u.codeConst.rowCount)
        MyAssertHandler(".\\r_shade.cpp", 56, 0, "%s", "rowCount");
    for (rowCounta = rowCount - 1; rowCounta; --rowCounta)
    {
        ++constant;
        constant->fields.codeConst = (MaterialArgumentCodeConst)-1;
        constant->fields.version = -1;
    }
    return 0;
}

char __cdecl R_IsShaderConstantUpToDate(
    const GfxCmdBufSourceState *source,
    GfxShaderConstantState *constant,
    const MaterialShaderArgument *routingData)
{
    GfxShaderConstantState newState; // [esp+4h] [ebp-10h]

    if (!source)
        MyAssertHandler(".\\r_shade.cpp", 73, 0, "%s", "source");
    newState.fields.codeConst = (MaterialArgumentCodeConst)routingData->u.codeSampler;
    newState.fields.version = source->constVersions[routingData->u.codeConst.index];
    if (constant->packed == newState.packed)
        return 1;
    *constant = newState;
    if (routingData->u.codeConst.rowCount != 1)
        MyAssertHandler(".\\r_shade.cpp", 82, 0, "%s", "routingData->u.codeConst.rowCount == 1");
    return 0;
}

void __cdecl R_SetupPassPerObjectArgs(GfxCmdBufContext context)
{
    const MaterialPass *pass; // [esp+0h] [ebp-4h]

    pass = context.state->pass;
    if (pass->perObjArgCount)
        R_SetPassShaderObjectArguments(context, pass->perObjArgCount, &pass->args[pass->perPrimArgCount]);
}

void __cdecl R_SetPixelShaderConstantFromLiteral(GfxCmdBufState *state, unsigned int dest, const float *literal)
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
    unsigned int dest,
    const float *data,
    unsigned int rowCount)
{
    const char *v4; // eax
    int hr; // [esp+0h] [ebp-4h]

    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetPixelShaderConstantF( dest, data, rowCount )\n");
        hr = device->SetPixelShaderConstantF(device, dest, data, rowCount);
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
    const char *v2; // eax
    GfxShaderConstantState newState; // [esp+4h] [ebp-10h]

    if (routingData->dest >= 0x100u)
        MyAssertHandler(
            ".\\r_shade.cpp",
            123,
            0,
            "routingData->dest doesn't index ARRAY_COUNT( context.state->pixelShaderConstState )\n\t%i not in [0, %i)",
            routingData->dest,
            256);
    if (routingData->u.codeConst.rowCount != 1)
        MyAssertHandler(
            ".\\r_shade.cpp",
            124,
            0,
            "%s\n\t(routingData->u.codeConst.rowCount) = %i",
            "(routingData->u.codeConst.rowCount == 1)",
            routingData->u.codeConst.rowCount);
    if (!context.source)
        MyAssertHandler(".\\r_shade.cpp", 125, 0, "%s", "context.source");
    newState.fields.codeConst = (MaterialArgumentCodeConst)routingData->u.codeSampler;
    newState.fields.version = context.source->constVersions[routingData->u.codeConst.index];
    if (!context.source->constVersions[routingData->u.codeConst.index])
    {
        v2 = va(
            "constant: %d, material: %s, technique: %s",
            routingData->u.codeConst.index,
            context.state->material->info.name,
            context.state->technique->name);
        MyAssertHandler(".\\r_shade.cpp", 130, 0, "%s\n\t%s", "newState.fields.version", v2);
    }
    if (context.state->pixelShaderConstState[routingData->dest] == newState.packed)
        return 1;
    context.state->pixelShaderConstState[routingData->dest] = newState.packed;
    return 0;
}

void __cdecl R_SetPixelShaderConstantFromCode(GfxCmdBufContext context, const MaterialShaderArgument *routingData)
{
    const char *v2; // eax
    GfxCmdBufInput *data; // [esp+4h] [ebp-4h]

    if (!context.source->constVersions[routingData->u.codeConst.index])
    {
        v2 = va(
            "constant: %d, material: %s, technique: %s",
            routingData->u.codeConst.index,
            context.state->material->info.name,
            context.state->technique->name);
        MyAssertHandler(
            ".\\r_shade.cpp",
            147,
            0,
            "%s\n\t%s",
            "context.source->constVersions[routingData->u.codeConst.index]",
            v2);
    }
    if (routingData->u.codeConst.index >= 0x3Au)
        MyAssertHandler(
            ".\\r_shade.cpp",
            150,
            0,
            "routingData->u.codeConst.index doesn't index CONST_SRC_FIRST_CODE_MATRIX\n\t%i not in [0, %i)",
            routingData->u.codeConst.index,
            58);
    data = R_GetCodeConstant(context, routingData->u.codeConst.index);
    R_HW_SetPixelShaderConstant(
        context.state->prim.device,
        routingData->dest,
        (const float *)data,
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
    const char *v2; // eax
    unsigned __int8 semantic; // [esp+7h] [ebp-1h]

    if (!image)
        MyAssertHandler(".\\r_shade.cpp", 438, 0, "%s", "image");
    if (!texdef)
        MyAssertHandler(".\\r_shade.cpp", 439, 0, "%s", "texdef");
    if ((*image)->mapType == MAPTYPE_2D)
    {
        semantic = texdef->semantic;
        switch (semantic)
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
                if (r_normalMap->current.integer != 1)
                    MyAssertHandler(
                        ".\\r_shade.cpp",
                        464,
                        0,
                        "%s\n\t(r_normalMap->current.integer) = %i",
                        "(r_normalMap->current.integer == R_NORMAL_OVERRIDE_NONE)",
                        r_normalMap->current.integer);
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
                v2 = va("unhandled case %i", semantic);
                MyAssertHandler(".\\r_shade.cpp", 473, 1, v2);
            }
            break;
        }
    }
}

void __cdecl R_SetPixelShader(GfxCmdBufState *state, const MaterialPixelShader *pixelShader)
{
    if (!pixelShader)
        MyAssertHandler(".\\r_shade.cpp", 680, 0, "%s", "pixelShader");
    if (!pixelShader->prog.ps)
        MyAssertHandler(
            ".\\r_shade.cpp",
            682,
            0,
            "%s\n\t(pixelShader->name) = %s",
            "(pixelShader->prog.ps)",
            pixelShader->name);
    if (state->pixelShader != pixelShader)
    {
        //Profile_Begin(96);
        R_HW_SetPixelShader(state->prim.device, pixelShader);
        state->pixelShader = pixelShader;
        //Profile_EndInternal(0);
    }
}

void __cdecl R_SetVertexShader(GfxCmdBufState *state, const MaterialVertexShader *vertexShader)
{
    if (state->vertexShader != vertexShader)
    {
        if (!vertexShader)
            MyAssertHandler(".\\r_shade.cpp", 702, 0, "%s", "vertexShader");
        if (!vertexShader->prog.vs)
            MyAssertHandler(
                ".\\r_shade.cpp",
                704,
                0,
                "%s\n\t(vertexShader->name) = %s",
                "(vertexShader->prog.vs)",
                vertexShader->name);
        //Profile_Begin(95);
        R_HW_SetVertexShader(state->prim.device, vertexShader);
        state->vertexShader = vertexShader;
        //Profile_EndInternal(0);
    }
}

void __cdecl R_UpdateVertexDecl(GfxCmdBufState *state)
{
    const MaterialPass *pass; // [esp+40h] [ebp-8h]
    const MaterialVertexShader *vertexShader; // [esp+44h] [ebp-4h]

    pass = state->pass;
    if (!pass->vertexDecl)
        MyAssertHandler(".\\r_shade.cpp", 721, 0, "%s", "pass->vertexDecl");
    vertexShader = pass->vertexShader;
    if (!vertexShader)
        MyAssertHandler(".\\r_shade.cpp", 752, 0, "%s", "vertexShader");
    R_SetVertexDecl(&state->prim, pass->vertexDecl);
    if (!pass->pixelShader)
        MyAssertHandler(".\\r_shade.cpp", 756, 0, "%s", "pass->pixelShader");
    if (pass->vertexDecl == (MaterialVertexDeclaration *)-36)
        MyAssertHandler(".\\r_shade.cpp", 759, 0, "%s", "pass->vertexDecl->routing.decl");
    if (!pass->vertexDecl->routing.decl[state->prim.vertDeclType])
        Com_Error(
            ERR_FATAL,
            "Vertex type %i doesn't have the information used by shader %s in material %s\n",
            state->prim.vertDeclType,
            pass->vertexShader->name,
            state->material->info.name);
    R_SetVertexShader(state, vertexShader);
}

void __cdecl R_SetupPass(GfxCmdBufContext context, unsigned int passIndex)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const MaterialPass *pass; // [esp+30h] [ebp-14h]
    const Material *material; // [esp+34h] [ebp-10h]
    const GfxStateBits *refStateBits; // [esp+38h] [ebp-Ch]
    unsigned int stateBits[2]; // [esp+3Ch] [ebp-8h] BYREF

    //Profile_Begin(142);
    pass = &context.state->technique->passArray[passIndex];
    context.state->pass = pass;
    context.state->passIndex = passIndex;
    material = context.state->material;
    if (material->stateBitsEntry[context.state->techType] >= (unsigned int)material->stateBitsCount)
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
    if (context.source->viewMode == VIEW_MODE_NONE)
        MyAssertHandler(".\\r_shade.cpp", 795, 0, "%s", "context.source->viewMode != VIEW_MODE_NONE");
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
    if (!pass->pixelShader)
        MyAssertHandler(".\\r_shade.cpp", 850, 0, "%s", "pass->pixelShader");
    R_SetPixelShader(context.state, pass->pixelShader);
    if (pass->stableArgCount)
        R_SetPassShaderStableArguments(
            context,
            pass->stableArgCount,
            &pass->args[pass->perPrimArgCount + pass->perObjArgCount]);
    //Profile_EndInternal(0);
}

void __cdecl R_SetState(GfxCmdBufState *state, unsigned int *stateBits)
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

void __cdecl R_SetVertexShaderConstantFromLiteral(GfxCmdBufState *state, unsigned int dest, const float *literal)
{
    __int64 v3; // [esp-10h] [ebp-14h]

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
    HIDWORD(v3) = dest;
    LODWORD(v3) = state->prim.device;
    R_HW_SetVertexShaderConstant(v3, (unsigned int)literal | 0x100000000LL);
}

const MaterialTextureDef *__cdecl R_SetPixelSamplerFromMaterial(
    GfxCmdBufContext context,
    const MaterialShaderArgument *arg,
    const MaterialTextureDef *texDef)
{
    const char *v3; // eax
    float floatTime; // [esp+4h] [ebp-10h]
    const GfxImage *image; // [esp+8h] [ebp-Ch] BYREF
    const Material *material; // [esp+Ch] [ebp-8h]
    unsigned __int8 samplerState; // [esp+13h] [ebp-1h]

    material = context.state->material;
    while (texDef->nameHash != arg->u.codeSampler)
    {
        if (++texDef == &material->textureTable[material->textureCount])
        {
            v3 = va("material '%s' is missing a required named texture", material->info.name);
            MyAssertHandler(
                ".\\r_shade.cpp",
                489,
                1,
                "%s\n\t%s",
                "texDef != &material->textureTable[material->textureCount]",
                v3);
        }
    }
    if (texDef->semantic == 11)
    {
        image = *(const GfxImage **)&texDef->u.image[1].depth;
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
    samplerState = texDef->samplerState;
    if (rg.hasAnyImageOverrides)
        R_OverrideImage((GfxImage **)&image, texDef);
    R_SetSampler(context, arg->dest, samplerState, image);
    return texDef;
}

void __cdecl R_SetPassShaderPrimArguments(
    GfxCmdBufContext context,
    unsigned int argCount,
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

void __cdecl R_SetPassShaderObjectArguments(
    GfxCmdBufContext context,
    unsigned int argCount,
    const MaterialShaderArgument *arg)
{
    const GfxImage *image; // [esp+Ch] [ebp-8h]
    unsigned __int8 samplerState; // [esp+13h] [ebp-1h] BYREF

    while (arg->type == 3)
    {
        R_SetVertexShaderConstantFromCode(context, arg++);
        if (!--argCount)
            return;
    }
    while (arg->type == 4)
    {
        image = R_GetTextureFromCode(context.source, arg->u.codeSampler, &samplerState);
        if (!image)
            R_TextureFromCodeError(context.source, arg->u.codeSampler);
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
    unsigned int argCount,
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
    unsigned int argCount,
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
    unsigned int argCount,
    const MaterialShaderArgument *arg)
{
    const char *v3; // eax
    const GfxImage *image; // [esp+14h] [ebp-14h]
    const Material *material; // [esp+18h] [ebp-10h]
    const MaterialTextureDef *texDef; // [esp+1Ch] [ebp-Ch]
    unsigned __int8 samplerState; // [esp+23h] [ebp-5h] BYREF
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
            R_TextureFromCodeError(context.source, arg->u.codeSampler);
        R_SetSampler(context, arg->dest, samplerState, image);
        ++arg;
        --argCount;
    } while (argCount);
}