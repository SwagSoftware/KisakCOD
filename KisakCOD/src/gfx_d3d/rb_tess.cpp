#include "rb_tess.h"
#include "r_dvars.h"


void __cdecl RB_ShowTess(GfxCmdBufContext context, const float *center, const char *tessName, const float *color)
{
    const char *v4; // eax
    const char *v5; // eax
    const GfxBackEndData *data; // [esp+14h] [ebp-2Ch]
    float TEXT_TECH_MEAN; // [esp+18h] [ebp-28h]
    const char *infoIdString; // [esp+1Ch] [ebp-24h]
    float TEXT_TECH_VERTICAL_OFFSET; // [esp+20h] [ebp-20h]
    float offsetCenter[3]; // [esp+24h] [ebp-1Ch] BYREF
    const MaterialTechnique *tech; // [esp+30h] [ebp-10h]
    const MaterialTechniqueSet *techSet; // [esp+34h] [ebp-Ch]
    float TEXT_SIZE; // [esp+38h] [ebp-8h]
    const char *infoString; // [esp+3Ch] [ebp-4h]

    TEXT_SIZE = 0.60000002;
    TEXT_TECH_VERTICAL_OFFSET = 0.30000001;
    TEXT_TECH_MEAN = 16.0;
    if (!center)
        MyAssertHandler(".\\rb_tess.cpp", 56, 0, "%s", "center");
    if (!tessName)
        MyAssertHandler(".\\rb_tess.cpp", 57, 0, "%s", "tessName");
    offsetCenter[0] = *center;
    offsetCenter[1] = center[1];
    offsetCenter[2] = center[2];
    if (!context.state->material)
        MyAssertHandler(".\\rb_tess.cpp", 61, 0, "%s", "context.state->material");
    tech = Material_GetTechnique(context.state->material, context.state->techType);
    if (!tech)
        MyAssertHandler(".\\rb_tess.cpp", 64, 0, "%s", "tech");
    switch (r_showTess->current.integer)
    {
    case 1:
        infoString = tech->name;
        offsetCenter[2] = ((double)(int)context.state->techType - TEXT_TECH_MEAN) * TEXT_TECH_VERTICAL_OFFSET
            + offsetCenter[2];
        infoIdString = "T";
        break;
    case 2:
        techSet = Material_GetTechniqueSet(context.state->material);
        if (!techSet)
            MyAssertHandler(".\\rb_tess.cpp", 96, 0, "%s", "techSet");
        infoString = techSet->name;
        infoIdString = "TS";
        break;
    case 3:
        infoString = context.state->material->info.name;
        infoIdString = "M";
        break;
    case 4:
        if (!tech->passCount)
            MyAssertHandler(".\\rb_tess.cpp", 75, 0, "%s", "tech->passCount > 0");
        if (tech->passArray[0].vertexShader)
            infoString = tech->passArray[0].vertexShader->name;
        else
            infoString = "<NONE>";
        offsetCenter[2] = ((double)(int)context.state->techType - TEXT_TECH_MEAN) * TEXT_TECH_VERTICAL_OFFSET
            + offsetCenter[2];
        infoIdString = "VS";
        break;
    case 5:
        if (!tech->passCount)
            MyAssertHandler(".\\rb_tess.cpp", 85, 0, "%s", "tech->passCount > 0");
        if (tech->passArray[0].pixelShader)
            infoString = tech->passArray[0].pixelShader->name;
        else
            infoString = "<NONE>";
        offsetCenter[2] = ((double)(int)context.state->techType - TEXT_TECH_MEAN) * TEXT_TECH_VERTICAL_OFFSET
            + offsetCenter[2];
        infoIdString = "PS";
        break;
    default:
        if (!alwaysfails)
        {
            v4 = va("Unknown value for r_showTess: %i", r_showTess);
            MyAssertHandler(".\\rb_tess.cpp", 107, 0, v4);
        }
        infoString = "?";
        infoIdString = "?";
        break;
    }
    data = context.source->input.data;
    v5 = va("%s:%s=%s", tessName, infoIdString, infoString);
    R_AddDebugString(&data->debugGlobals, offsetCenter, color, TEXT_SIZE, v5);
}

unsigned int __cdecl R_TessCodeMeshList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    const char *v2; // eax
    GfxDepthRangeType depthRangeType; // [esp+24h] [ebp-6Ch]
    GfxCmdBufContext context; // [esp+3Ch] [ebp-54h]
    const GfxDrawSurfListInfo *info; // [esp+44h] [ebp-4Ch]
    GfxDrawSurf drawSurf; // [esp+48h] [ebp-48h]
    const GfxBackEndData *data; // [esp+50h] [ebp-40h]
    const FxCodeMeshData *codeMesh; // [esp+58h] [ebp-38h]
    unsigned int drawSurfIndex; // [esp+5Ch] [ebp-34h]
    unsigned __int8 *indices; // [esp+68h] [ebp-28h]
    unsigned int argCount; // [esp+6Ch] [ebp-24h]
    const GfxDrawSurf *drawSurfList; // [esp+70h] [ebp-20h]
    GfxDrawPrimArgs args; // [esp+74h] [ebp-1Ch] BYREF
    unsigned __int64 drawSurfKey; // [esp+80h] [ebp-10h]
    unsigned int drawSurfCount; // [esp+8Ch] [ebp-4h]

    //Profile_Begin(100);
    context = listArgs->context;
    info = listArgs->info;
    drawSurfCount = info->drawSurfCount - listArgs->firstDrawSurfIndex;
    drawSurfList = &info->drawSurfs[listArgs->firstDrawSurfIndex];
    R_SetupPassCriticalPixelShaderArgs(context);
    if (prepassContext.state && context.source != prepassContext.source)
        MyAssertHandler(
            ".\\rb_tess.cpp",
            197,
            0,
            "%s",
            "prepassContext.state == NULL || commonSource == prepassContext.source");
    drawSurf.fields = drawSurfList->fields;
    if (context.source->objectPlacement != &rg.identityPlacement)
        R_ChangeObjectPlacement(context.source, &rg.identityPlacement);
    R_ChangeDepthHackNearClip(context.source, 0);
    R_SetVertexDeclTypeNormal(context.state, VERTDECL_PACKED);
    depthRangeType = (context.source->cameraView != 0) - 1;
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    data = context.source->input.data;
    R_SetMeshStream(context.state, &data->codeMesh);
    indices = 0;
    args.baseIndex = 0;
    args.vertexCount = 0x4000;
    args.triCount = 0;
    drawSurfKey = drawSurf.packed & 0xFFFFFFFFE0000000uLL;
    R_TrackPrims(context.state, GFX_PRIM_STATS_FX);
    argCount = 0;
    drawSurfIndex = 0;
    do
    {
        codeMesh = &data->codeMeshes[LOWORD(drawSurf.packed)];
        if (!codeMesh->triCount)
            MyAssertHandler(
                ".\\rb_tess.cpp",
                234,
                0,
                "%s\n\t(codeMesh->triCount) = %i",
                "(codeMesh->triCount > 0)",
                codeMesh->triCount);
        if (data->codeMeshes[LOWORD(drawSurf.packed)].indices < data->codeMesh.indices
            || &data->codeMeshes[LOWORD(drawSurf.packed)].indices[3 * data->codeMeshes[LOWORD(drawSurf.packed)].triCount] > data->codeMesh.indices + 24576)
        {
            v2 = va(
                "0x%08x 0x%08x %i %i %s",
                data->codeMeshes[LOWORD(drawSurf.packed)].indices,
                data->codeMesh.indices,
                3 * data->codeMeshes[LOWORD(drawSurf.packed)].triCount,
                24576,
                context.state->material->info.name);
            MyAssertHandler(
                ".\\rb_tess.cpp",
                237,
                0,
                "%s\n\t%s",
                "&codeMesh->indices[0] >= &data->codeMesh.indices[0] && &codeMesh->indices[codeMesh->triCount * 3] <= &data->code"
                "Mesh.indices[GFX_CODE_MESH_INDEX_LIMIT]",
                v2);
        }
        if (argCount
            || data->codeMeshes[LOWORD(drawSurf.packed)].argCount
            || &indices[6 * args.triCount] != (unsigned __int8 *)data->codeMeshes[LOWORD(drawSurf.packed)].indices)
        {
            if (args.triCount)
            {
                args.baseIndex = R_SetIndexData(&context.state->prim, indices, args.triCount);
                R_DrawIndexedPrimitive(&context.state->prim, &args);
                args.triCount = 0;
            }
            indices = (unsigned __int8 *)data->codeMeshes[LOWORD(drawSurf.packed)].indices;
            R_TessCodeMeshList_AddCodeMeshArgs(context.source, data, codeMesh);
            R_SetupPassPerObjectArgs(context);
            R_SetupPassPerPrimArgs(context);
        }
        argCount = data->codeMeshes[LOWORD(drawSurf.packed)].argCount;
        args.triCount += codeMesh->triCount;
        g_frameStatsCur.fxIndexCount += 3 * codeMesh->triCount;
        if (++drawSurfIndex == drawSurfCount)
            break;
        drawSurf.fields = (GfxDrawSurfFields)drawSurfList[drawSurfIndex];
    } while ((drawSurf.packed & 0xFFFFFFFFE0000000uLL) == drawSurfKey);
    if (args.triCount)
    {
        args.baseIndex = R_SetIndexData(&context.state->prim, indices, args.triCount);
        R_DrawIndexedPrimitive(&context.state->prim, &args);
    }
    g_primStats = 0;
    //Profile_EndInternal(0);
    return drawSurfIndex;
}

void __cdecl R_SetVertexDeclTypeNormal(GfxCmdBufState *state, MaterialVertexDeclType vertDeclType)
{
    state->prim.vertDeclType = vertDeclType;
    R_UpdateVertexDecl(state);
}

void __cdecl R_TessCodeMeshList_AddCodeMeshArgs(
    GfxCmdBufSourceState *source,
    const GfxBackEndData *data,
    const FxCodeMeshData *codeMesh)
{
    float v3; // [esp+0h] [ebp-2Ch]
    float v4; // [esp+4h] [ebp-28h]
    float v5; // [esp+8h] [ebp-24h]
    float v6; // [esp+Ch] [ebp-20h]
    float *v7; // [esp+10h] [ebp-1Ch]
    unsigned int argIndex; // [esp+14h] [ebp-18h]
    unsigned int argGlobalIndex; // [esp+18h] [ebp-14h]
    unsigned int argCount; // [esp+20h] [ebp-Ch]
    unsigned int argOffset; // [esp+24h] [ebp-8h]
    unsigned int constantId; // [esp+28h] [ebp-4h]

    argOffset = codeMesh->argOffset;
    argCount = codeMesh->argCount;
    for (argIndex = 0; argIndex != argCount; ++argIndex)
    {
        constantId = argIndex + 55;
        if (argIndex + 55 > 0x38)
            MyAssertHandler(
                ".\\rb_tess.cpp",
                154,
                0,
                "%s\n\t(constantId) = %i",
                "(constantId <= CONST_SRC_CODE_CODE_MESH_ARG_LAST)",
                constantId);
        argGlobalIndex = argOffset + argIndex;
        if (argOffset + argIndex >= data->codeMeshArgsCount)
            MyAssertHandler(
                ".\\rb_tess.cpp",
                156,
                0,
                "%s\n\t(argGlobalIndex) = %i",
                "(argGlobalIndex < static_cast< uint >( data->codeMeshArgsCount ))",
                argGlobalIndex);
        v3 = data->codeMeshArgs[argGlobalIndex][0];
        v4 = data->codeMeshArgs[argGlobalIndex][1];
        v5 = data->codeMeshArgs[argGlobalIndex][2];
        v6 = data->codeMeshArgs[argGlobalIndex][3];
        if (constantId >= 0x3A)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\gfx_d3d\\r_state.h",
                495,
                0,
                "constant doesn't index CONST_SRC_CODE_COUNT_FLOAT4\n\t%i not in [0, %i)",
                constantId,
                58);
        v7 = source->input.consts[constantId];
        *v7 = v3;
        v7[1] = v4;
        v7[2] = v5;
        v7[3] = v6;
        R_DirtyCodeConstant(source, constantId);
    }
}

unsigned int __cdecl R_TessMarkMeshList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    int packed_low; // eax
    int v3; // eax
    bool v5; // [esp+10h] [ebp-B4h]
    bool v6; // [esp+18h] [ebp-ACh]
    bool v7; // [esp+1Ch] [ebp-A8h]
    GfxDepthRangeType depthRangeType; // [esp+40h] [ebp-84h]
    GfxCmdBufContext context; // [esp+58h] [ebp-6Ch]
    const GfxDrawSurfListInfo *info; // [esp+60h] [ebp-64h]
    GfxDrawSurf drawSurf; // [esp+64h] [ebp-60h]
    const GfxBackEndData *data; // [esp+6Ch] [ebp-58h]
    unsigned __int64 drawSurfSubKey; // [esp+74h] [ebp-50h]
    GfxDrawSurf drawSurfSubMask; // [esp+7Ch] [ebp-48h]
    MaterialTechniqueType baseTechType; // [esp+84h] [ebp-40h]
    unsigned int drawSurfIndex; // [esp+88h] [ebp-3Ch]
    unsigned __int8 *indices; // [esp+94h] [ebp-30h]
    MaterialVertexDeclType declType; // [esp+98h] [ebp-2Ch]
    const FxMarkMeshData *markMesh; // [esp+9Ch] [ebp-28h]
    const FxMarkMeshData *markMesha; // [esp+9Ch] [ebp-28h]
    unsigned int markType; // [esp+A0h] [ebp-24h]
    unsigned int markTypea; // [esp+A0h] [ebp-24h]
    unsigned int markTypeb; // [esp+A0h] [ebp-24h]
    const GfxDrawSurf *drawSurfList; // [esp+A4h] [ebp-20h]
    GfxDrawPrimArgs args; // [esp+A8h] [ebp-1Ch] BYREF
    unsigned __int64 drawSurfKey; // [esp+B4h] [ebp-10h]
    unsigned int drawSurfCount; // [esp+C0h] [ebp-4h]

    //Profile_Begin(100);
    context = listArgs->context;
    info = listArgs->info;
    drawSurfCount = info->drawSurfCount - listArgs->firstDrawSurfIndex;
    drawSurfList = &info->drawSurfs[listArgs->firstDrawSurfIndex];
    R_SetupPassCriticalPixelShaderArgs(context);
    if (prepassContext.state && context.source != prepassContext.source)
        MyAssertHandler(
            ".\\rb_tess.cpp",
            351,
            0,
            "%s",
            "prepassContext.state == NULL || commonSource == prepassContext.source");
    baseTechType = info->baseTechType;
    if (baseTechType == TECHNIQUE_LIT_BEGIN)
    {
        if (sc_enable->current.enabled)
            R_SetCodeImageTexture(context.source, 0x10u, (const GfxImage *)dword_EA74F68);
        else
            R_SetCodeImageTexture(context.source, 0x10u, rgp.whiteImage);
    }
    drawSurf.fields = drawSurfList->fields;
    if (context.source->objectPlacement != &rg.identityPlacement)
        R_ChangeObjectPlacement(context.source, &rg.identityPlacement);
    R_ChangeDepthHackNearClip(context.source, 0);
    data = context.source->input.data;
    markType = data->markMeshes[LOWORD(drawSurf.packed)].modelTypeAndSurf & 0xC0;
    v7 = markType == 64 || markType == 192;
    declType = 2 - v7;
    R_SetVertexDeclTypeNormal(context.state, declType);
    depthRangeType = (context.source->cameraView != 0) - 1;
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    R_SetMeshStream(context.state, &data->markMesh);
    indices = 0;
    args.baseIndex = 0;
    args.vertexCount = 6144;
    args.triCount = 0;
    drawSurfSubMask.packed = -65536;
    if (baseTechType != TECHNIQUE_LIT_BEGIN)
    {
        *(unsigned int *)&drawSurfSubMask.fields = -536870912;
        R_SetupPassPerObjectArgs(context);
    }
    drawSurfKey = drawSurf.packed & 0xFFFFFFFFE0000000uLL;
    R_TrackPrims(context.state, GFX_PRIM_STATS_FX);
    drawSurfIndex = 0;
    do
    {
        if (baseTechType == TECHNIQUE_LIT_BEGIN)
        {
            if (args.triCount)
            {
                R_SetupPassPerPrimArgs(context);
                args.baseIndex = R_SetIndexData(&context.state->prim, indices, args.triCount);
                R_DrawIndexedPrimitive(&context.state->prim, &args);
                indices = 0;
                args.triCount = 0;
            }
            packed_low = LOWORD(drawSurf.packed);
            markMesh = &data->markMeshes[packed_low];
            markTypea = data->markMeshes[packed_low].modelTypeAndSurf & 0xC0;
            v6 = markTypea == 64 || markTypea == 192;
            if (declType != 2 - v6)
                MyAssertHandler(
                    ".\\rb_tess.cpp",
                    423,
                    0,
                    "%s\n\t(markType) = %i",
                    "(declType == R_Tess_DeclForMarkType( markType ))",
                    markTypea);
            if (markTypea == 64)
            {
                R_SetStaticModelLightingCoordsForSource(markMesh->modelIndex, context.source);
            }
            else if (markTypea == 192)
            {
                R_SetModelLightingCoordsForSource(markMesh->modelIndex, context.source);
            }
            else
            {
                if (markTypea && markTypea != 128)
                    MyAssertHandler(
                        ".\\rb_tess.cpp",
                        434,
                        0,
                        "%s\n\t(markType) = %i",
                        "((markType == MARK_MODEL_TYPE_WORLD_BRUSH) || (markType == MARK_MODEL_TYPE_ENT_BRUSH))",
                        markTypea);
                R_SetLightmap(context, (drawSurf.packed >> 24) & 0x1F);
            }
            R_SetReflectionProbe(context, BYTE2(drawSurf.packed));
            R_SetupPassPerObjectArgs(context);
        }
        drawSurfSubKey = drawSurfSubMask.packed & drawSurf.packed;
        do
        {
            v3 = LOWORD(drawSurf.packed);
            markMesha = &data->markMeshes[v3];
            markTypeb = data->markMeshes[v3].modelTypeAndSurf & 0xC0;
            v5 = markTypeb == 64 || markTypeb == 192;
            if (declType != 2 - v5)
                MyAssertHandler(
                    ".\\rb_tess.cpp",
                    449,
                    0,
                    "%s\n\t(markType) = %i",
                    "(declType == R_Tess_DeclForMarkType( markType ))",
                    markTypeb);
            if (!markMesha->triCount)
                MyAssertHandler(
                    ".\\rb_tess.cpp",
                    450,
                    0,
                    "%s\n\t(markMesh->triCount) = %i",
                    "(markMesh->triCount > 0)",
                    markMesha->triCount);
            if (markMesha->indices < data->markMesh.indices
                || &markMesha->indices[3 * markMesha->triCount] > data->markMesh.indices + 9216)
            {
                MyAssertHandler(
                    ".\\rb_tess.cpp",
                    452,
                    0,
                    "%s",
                    "&markMesh->indices[0] >= &data->markMesh.indices[0] && &markMesh->indices[markMesh->triCount * 3] <= &data->ma"
                    "rkMesh.indices[GFX_MARK_MESH_INDEX_LIMIT]");
            }
            if (&indices[6 * args.triCount] != (unsigned __int8 *)markMesha->indices)
            {
                if (args.triCount)
                {
                    R_SetupPassPerPrimArgs(context);
                    args.baseIndex = R_SetIndexData(&context.state->prim, indices, args.triCount);
                    R_DrawIndexedPrimitive(&context.state->prim, &args);
                    args.triCount = 0;
                }
                indices = (unsigned __int8 *)markMesha->indices;
            }
            args.triCount += markMesha->triCount;
            g_frameStatsCur.fxIndexCount += 3 * markMesha->triCount;
            if (++drawSurfIndex == drawSurfCount)
                break;
            drawSurf.fields = (GfxDrawSurfFields)drawSurfList[drawSurfIndex];
        } while ((drawSurfSubMask.packed & drawSurf.packed) == drawSurfSubKey);
    } while (drawSurfIndex != drawSurfCount && (drawSurf.packed & 0xFFFFFFFFE0000000uLL) == drawSurfKey);
    if (args.triCount)
    {
        R_SetupPassPerPrimArgs(context);
        args.baseIndex = R_SetIndexData(&context.state->prim, indices, args.triCount);
        R_DrawIndexedPrimitive(&context.state->prim, &args);
    }
    g_primStats = 0;
    //Profile_EndInternal(0);
    return drawSurfIndex;
}

unsigned int __cdecl R_TessParticleCloudList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    const char *v2; // eax
    GfxDepthRangeType depthRangeType; // [esp+24h] [ebp-5Ch]
    const GfxParticleCloud *cloud; // [esp+50h] [ebp-30h]
    GfxCmdBufContext context; // [esp+54h] [ebp-2Ch]
    const GfxDrawSurfListInfo *info; // [esp+5Ch] [ebp-24h]
    int drawSurf; // [esp+60h] [ebp-20h]
    const GfxBackEndData *data; // [esp+6Ch] [ebp-14h]
    GfxCmdBufSourceState *commonSource; // [esp+70h] [ebp-10h]
    GfxDrawPrimArgs args; // [esp+74h] [ebp-Ch] BYREF

    //Profile_Begin(101);
    context = listArgs->context;
    commonSource = listArgs->context.source;
    if (prepassContext.state && commonSource != prepassContext.source)
        MyAssertHandler(
            ".\\rb_tess.cpp",
            629,
            0,
            "%s",
            "prepassContext.state == NULL || commonSource == prepassContext.source");
    info = listArgs->info;
    if (r_logFile->current.integer)
    {
        v2 = va("--- RB_TessParticleCloud( %s ) ---\n", context.state->material->info.name);
        RB_LogPrint(v2);
    }
    drawSurf = (int)info->drawSurfs[listArgs->firstDrawSurfIndex].fields;
    data = commonSource->input.data;
    if ((unsigned int)(unsigned __int16)drawSurf >= data->cloudCount)
        MyAssertHandler(
            ".\\rb_tess.cpp",
            639,
            0,
            "drawSurf.fields.objectId doesn't index data->cloudCount\n\t%i not in [0, %i)",
            (unsigned __int16)drawSurf,
            data->cloudCount);
    cloud = &data->clouds[(unsigned __int16)drawSurf];
    //Profile_Begin(97);
    R_SetParticleCloudConstants(commonSource, cloud);
    //Profile_EndInternal(0);
    R_SetupPassCriticalPixelShaderArgs(context);
    R_ChangeDepthHackNearClip(commonSource, 0);
    R_SetVertexDeclTypeNormal(context.state, VERTDECL_POS_TEX);
    depthRangeType = (context.source->cameraView != 0) - 1;
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    if (context.state->prim.indexBuffer != gfxBuf.particleCloudIndexBuffer)
        R_ChangeIndices(&context.state->prim, gfxBuf.particleCloudIndexBuffer);
    R_SetStreamSource(&context.state->prim, gfxBuf.particleCloudVertexBuffer, 0, 0x14u);
    if ((const GfxParticleCloud *)commonSource->objectPlacement != cloud)
        R_ChangeObjectPlacement(commonSource, &cloud->placement);
    R_SetupPassPerObjectArgs(context);
    R_SetupPassPerPrimArgs(context);
    args.baseIndex = 0;
    args.vertexCount = 4096;
    args.triCount = 2048;
    g_frameStatsCur.fxIndexCount += 6144;
    RB_TrackImmediatePrims(GFX_PRIM_STATS_FX);
    if (!g_primStats)
        MyAssertHandler(".\\rb_tess.cpp", 678, 0, "%s", "g_primStats");
    R_DrawIndexedPrimitive(&context.state->prim, &args);
    if (!g_primStats)
        MyAssertHandler(".\\rb_tess.cpp", 684, 0, "%s", "g_primStats");
    g_primStats->staticVertexCount += args.vertexCount;
    g_primStats->staticIndexCount += 3 * args.triCount;
    RB_EndTrackImmediatePrims();
    //Profile_EndInternal(0);
    return 1;
}

void __cdecl R_SetParticleCloudConstants(GfxCmdBufSourceState *source, const GfxParticleCloud *cloud)
{
    float v3; // [esp+34h] [ebp-60h]
    float scaledWorldUp[3]; // [esp+40h] [ebp-54h] BYREF
    float viewUp[5]; // [esp+4Ch] [ebp-48h] BYREF
    float viewAxis_8; // [esp+60h] [ebp-34h]
    float viewAxis_12; // [esp+64h] [ebp-30h]
    float particleCloudMatrix[4]; // [esp+68h] [ebp-2Ch] BYREF
    float particleColor[4]; // [esp+78h] [ebp-1Ch] BYREF
    float worldUp[3]; // [esp+88h] [ebp-Ch] BYREF

    if (!cloud)
        MyAssertHandler(".\\rb_tess.cpp", 587, 0, "%s", "cloud");
    if (cloud->radius[1] == cloud->radius[0]
        || VecNCompareCustomEpsilon(cloud->placement.base.origin, cloud->endpos, 0.001, 3))
    {
        viewUp[3] = cloud->radius[0];
        viewUp[4] = 0.0;
        v3 = cloud->radius[1];
        viewAxis_8 = 0.0;
        viewAxis_12 = v3;
    }
    else
    {
        Vec3Sub(cloud->endpos, cloud->placement.base.origin, worldUp);
        Vec3Normalize(worldUp);
        Vec3Scale(worldUp, cloud->radius[1], scaledWorldUp);
        RB_Vec3DirWorldToView(source, scaledWorldUp, viewUp);
        RB_CreateParticleCloud2dAxis(cloud, viewUp, (float (*)[2]) & viewUp[3]);
    }
    particleCloudMatrix[0] = viewUp[3];
    particleCloudMatrix[1] = viewUp[4];
    particleCloudMatrix[2] = viewAxis_8;
    particleCloudMatrix[3] = viewAxis_12;
    R_SetCodeConstantFromVec4(source, 0x35u, particleCloudMatrix);
    Byte4UnpackBgra((const unsigned __int8 *)&cloud->color, particleColor);
    R_SetCodeConstantFromVec4(source, 0x11u, particleColor);
}

void __cdecl RB_Vec3DirWorldToView(const GfxCmdBufSourceState *source, const float *worldDir, float *viewDir)
{
    float v3; // [esp+8h] [ebp-4Ch]
    float v4; // [esp+Ch] [ebp-48h]
    float v5; // [esp+18h] [ebp-3Ch]
    float v6; // [esp+1Ch] [ebp-38h]
    float v7; // [esp+24h] [ebp-30h]
    float v8; // [esp+28h] [ebp-2Ch]
    float viewAxis[3][3]; // [esp+30h] [ebp-24h] BYREF

    if (source->viewMode != VIEW_MODE_3D)
        MyAssertHandler(
            ".\\rb_tess.cpp",
            528,
            0,
            "%s\n\t(source->viewMode) = %i",
            "(source->viewMode == VIEW_MODE_3D)",
            source->viewMode);
    v7 = source->viewParms.viewMatrix.m[0][1];
    v8 = source->viewParms.viewMatrix.m[0][2];
    viewAxis[0][0] = source->viewParms.viewMatrix.m[0][0];
    viewAxis[0][1] = v7;
    viewAxis[0][2] = v8;
    v5 = source->viewParms.viewMatrix.m[1][1];
    v6 = source->viewParms.viewMatrix.m[1][2];
    viewAxis[1][0] = source->viewParms.viewMatrix.m[1][0];
    viewAxis[1][1] = v5;
    viewAxis[1][2] = v6;
    v3 = source->viewParms.viewMatrix.m[2][1];
    v4 = source->viewParms.viewMatrix.m[2][2];
    viewAxis[2][0] = source->viewParms.viewMatrix.m[2][0];
    viewAxis[2][1] = v3;
    viewAxis[2][2] = v4;
    Vec3RotateTranspose(worldDir, viewAxis, viewDir);
}

void __cdecl RB_CreateParticleCloud2dAxis(const GfxParticleCloud *cloud, const float *viewUp, float (*viewAxis)[2])
{
    float v3; // [esp+8h] [ebp-50h]
    float v4; // [esp+10h] [ebp-48h]
    float v5; // [esp+18h] [ebp-40h]
    float v6; // [esp+1Ch] [ebp-3Ch]
    float v7; // [esp+30h] [ebp-28h]
    float v8; // [esp+38h] [ebp-20h]
    float viewAxisLength; // [esp+40h] [ebp-18h]
    float viewRight; // [esp+44h] [ebp-14h]
    float viewRight_4; // [esp+48h] [ebp-10h]

    if (!cloud)
        MyAssertHandler(".\\rb_tess.cpp", 545, 0, "%s", "cloud");
    if (*viewUp >= 0.001000000047497451 || viewUp[1] >= 0.001000000047497451)
    {
        viewRight = -(float)-1.0 * viewUp[1];
        viewRight_4 = (float)-1.0 * *viewUp;
        (*viewAxis)[0] = viewRight;
        (*viewAxis)[1] = viewRight_4;
        v7 = viewUp[1];
        (*viewAxis)[2] = *viewUp;
        (*viewAxis)[3] = v7;
        viewAxisLength = Vec2Length((const float *)viewAxis);
        if (viewAxisLength <= 0.0)
            MyAssertHandler(".\\rb_tess.cpp", 564, 0, "%s\n\t(viewAxisLength) = %g", "(viewAxisLength > 0)", viewAxisLength);
        v6 = viewAxisLength - Vec2Length(&(*viewAxis)[2]);
        v3 = fabs(v6);
        if (v3 >= 0.001000000047497451)
            MyAssertHandler(
                ".\\rb_tess.cpp",
                565,
                0,
                "%s",
                "fabs( viewAxisLength - Vec2Length( viewAxis[1] ) ) < EQUAL_EPSILON");
        v5 = cloud->radius[0] / viewAxisLength;
        (*viewAxis)[0] = v5 * (*viewAxis)[0];
        (*viewAxis)[1] = v5 * (*viewAxis)[1];
        if (cloud->radius[0] > (double)viewAxisLength)
        {
            v4 = cloud->radius[0] / viewAxisLength;
            (*viewAxis)[2] = v4 * (*viewAxis)[2];
            (*viewAxis)[3] = v4 * (*viewAxis)[3];
        }
    }
    else
    {
        (*viewAxis)[0] = cloud->radius[0];
        (*viewAxis)[1] = 0.0;
        v8 = cloud->radius[1];
        (*viewAxis)[2] = 0.0;
        (*viewAxis)[3] = v8;
    }
}

unsigned int __cdecl R_TessXModelSkinnedDrawSurfList(
    const GfxDrawSurfListArgs *listArgs,
    GfxCmdBufContext prepassContext)
{
    GfxCmdBufContext context; // [esp+5Ch] [ebp-60h]
    unsigned int baseGfxEntIndex; // [esp+64h] [ebp-58h]
    unsigned int baseGfxEntIndexa; // [esp+64h] [ebp-58h]
    const GfxDrawSurfListInfo *info; // [esp+68h] [ebp-54h]
    GfxDrawSurf drawSurf; // [esp+6Ch] [ebp-50h]
    const GfxBackEndData *data; // [esp+74h] [ebp-48h]
    GfxCmdBufSourceState *commonSource; // [esp+78h] [ebp-44h]
    MaterialTechniqueType baseTechType; // [esp+7Ch] [ebp-40h]
    int setupVertexShader; // [esp+80h] [ebp-3Ch]
    int setupVertexShadera; // [esp+80h] [ebp-3Ch]
    unsigned int drawSurfIndex; // [esp+84h] [ebp-38h]
    const GfxModelSkinnedSurface *modelSurf; // [esp+98h] [ebp-24h]
    const GfxModelSkinnedSurface *modelSurfa; // [esp+98h] [ebp-24h]
    GfxDepthRangeType depthHackFlags; // [esp+9Ch] [ebp-20h]
    unsigned int gfxEntIndex; // [esp+A0h] [ebp-1Ch]
    unsigned int gfxEntIndexa; // [esp+A0h] [ebp-1Ch]
    unsigned int gfxEntIndexb; // [esp+A0h] [ebp-1Ch]
    float materialTime; // [esp+A4h] [ebp-18h]
    float materialTimea; // [esp+A4h] [ebp-18h]
    const GfxDrawSurf *drawSurfList; // [esp+A8h] [ebp-14h]
    unsigned int drawSurfKey; // [esp+ACh] [ebp-10h]
    unsigned int drawSurfKeya; // [esp+ACh] [ebp-10h]
    unsigned int drawSurfKeyb; // [esp+ACh] [ebp-10h]
    unsigned int drawSurfCount; // [esp+B8h] [ebp-4h]

    //Profile_Begin(102);
    context = listArgs->context;
    commonSource = listArgs->context.source;
    if (prepassContext.state && commonSource != prepassContext.source)
        MyAssertHandler(
            ".\\rb_tess.cpp",
            856,
            0,
            "%s",
            "prepassContext.state == NULL || commonSource == prepassContext.source");
    data = commonSource->input.data;
    info = listArgs->info;
    baseTechType = info->baseTechType;
    drawSurfCount = info->drawSurfCount - listArgs->firstDrawSurfIndex;
    drawSurfList = &info->drawSurfs[listArgs->firstDrawSurfIndex];
    drawSurf.fields = drawSurfList->fields;
    if (commonSource->objectPlacement != &commonSource->skinnedPlacement)
        R_ChangeObjectPlacement(commonSource, &commonSource->skinnedPlacement);
    RB_TrackImmediatePrims(GFX_PRIM_STATS_XMODELSKINNED);
    drawSurfIndex = 0;
    if (info->cameraView)
    {
        modelSurf = (const GfxModelSkinnedSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
        baseGfxEntIndex = modelSurf->info.gfxEntIndex;
        if (modelSurf->info.gfxEntIndex)
        {
            depthHackFlags = data->gfxEnts[baseGfxEntIndex].renderFxFlags & 2;
            materialTime = data->gfxEnts[baseGfxEntIndex].materialTime;
        }
        else
        {
            depthHackFlags = GFX_DEPTH_RANGE_SCENE;
            materialTime = 0.0;
        }
        setupVertexShader = R_UpdateMaterialTime(commonSource, materialTime);
        R_SetupPassCriticalPixelShaderArgs(context);
        if (setupVertexShader)
            R_SetupPassVertexShaderArgs(context);
        R_ChangeDepthHackNearClip(commonSource, depthHackFlags);
        R_SetVertexDeclTypeNormal(context.state, VERTDECL_PACKED);
        if (depthHackFlags != context.state->depthRangeType)
            R_ChangeDepthRange(context.state, depthHackFlags);
        if (baseTechType == TECHNIQUE_LIT_BEGIN)
        {
            if (sc_enable->current.enabled && ((drawSurf.packed >> 24) & 0x1F) != 0)
                R_SetCodeImageTexture(commonSource, 0x10u, (const GfxImage *)dword_EA74F68);
            else
                R_SetCodeImageTexture(commonSource, 0x10u, rgp.whiteImage);
            R_SetupPassPerObjectArgs(context);
            if (prepassContext.state)
            {
                R_SetupPassCriticalPixelShaderArgs(prepassContext);
                if (setupVertexShader)
                    R_SetupPassVertexShaderArgs(prepassContext);
                R_SetVertexDeclTypeNormal(prepassContext.state, VERTDECL_PACKED);
                if (depthHackFlags != prepassContext.state->depthRangeType)
                    R_ChangeDepthRange(prepassContext.state, depthHackFlags);
                R_SetupPassPerObjectArgs(prepassContext);
                R_SetupPassPerPrimArgs(prepassContext);
            }
            drawSurfKey = *(unsigned int *)&drawSurf.fields & 0xFF000000;
            while (1)
            {
                if (prepassContext.state)
                    R_DrawXModelSkinnedModelSurf(prepassContext, modelSurf);
                R_SetModelLightingCoordsForSource(modelSurf->info.lightingHandle, commonSource);
                R_SetReflectionProbe(context, BYTE2(drawSurf.packed));
                R_SetupPassPerPrimArgs(context);
                R_DrawXModelSkinnedModelSurf(context, modelSurf);
                if (++drawSurfIndex == drawSurfCount)
                    break;
                *(unsigned int *)&drawSurf.fields = drawSurfList[drawSurfIndex].fields;
                if ((drawSurfList[drawSurfIndex].packed & 0xFFFFFFFFFF000000uLL) != __PAIR64__(
                    HIDWORD(drawSurf.packed),
                    drawSurfKey))
                    break;
                modelSurf = (const GfxModelSkinnedSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
                gfxEntIndex = modelSurf->info.gfxEntIndex;
                if (gfxEntIndex != baseGfxEntIndex)
                {
                    if (modelSurf->info.gfxEntIndex)
                    {
                        if ((data->gfxEnts[gfxEntIndex].renderFxFlags & 2) != depthHackFlags
                            || materialTime != data->gfxEnts[gfxEntIndex].materialTime)
                        {
                            break;
                        }
                    }
                    else if (depthHackFlags || materialTime != 0.0)
                    {
                        break;
                    }
                }
            }
        }
        else
        {
            if (prepassContext.state)
                MyAssertHandler(".\\rb_tess.cpp", 992, 0, "%s", "prepassContext.state == NULL");
            R_SetupPassPerObjectArgs(context);
            R_SetupPassPerPrimArgs(context);
            drawSurfKeya = *(unsigned int *)&drawSurf.fields & 0xE0000000;
            while (1)
            {
                R_DrawXModelSkinnedModelSurf(context, modelSurf);
                if (++drawSurfIndex == drawSurfCount)
                    break;
                *(unsigned int *)&drawSurf.fields = drawSurfList[drawSurfIndex].fields;
                if ((drawSurfList[drawSurfIndex].packed & 0xFFFFFFFFE0000000uLL) != __PAIR64__(
                    HIDWORD(drawSurf.packed),
                    drawSurfKeya))
                    break;
                modelSurf = (const GfxModelSkinnedSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
                gfxEntIndexa = modelSurf->info.gfxEntIndex;
                if (gfxEntIndexa != baseGfxEntIndex)
                {
                    if (modelSurf->info.gfxEntIndex)
                    {
                        if ((data->gfxEnts[gfxEntIndexa].renderFxFlags & 2) != depthHackFlags
                            || materialTime != data->gfxEnts[gfxEntIndexa].materialTime)
                        {
                            break;
                        }
                    }
                    else if (depthHackFlags || materialTime != 0.0)
                    {
                        break;
                    }
                }
            }
        }
    }
    else
    {
        if (prepassContext.state)
            MyAssertHandler(".\\rb_tess.cpp", 1046, 0, "%s", "prepassContext.state == NULL");
        if (baseTechType == TECHNIQUE_LIT_BEGIN)
            MyAssertHandler(".\\rb_tess.cpp", 1047, 0, "%s", "baseTechType != TECHNIQUE_LIT");
        modelSurfa = (const GfxModelSkinnedSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
        baseGfxEntIndexa = modelSurfa->info.gfxEntIndex;
        if (modelSurfa->info.gfxEntIndex)
            materialTimea = data->gfxEnts[baseGfxEntIndexa].materialTime;
        else
            materialTimea = 0.0;
        setupVertexShadera = R_UpdateMaterialTime(commonSource, materialTimea);
        R_SetupPassCriticalPixelShaderArgs(context);
        if (setupVertexShadera)
            R_SetupPassVertexShaderArgs(context);
        R_ChangeDepthHackNearClip(commonSource, 0);
        R_SetVertexDeclTypeNormal(context.state, VERTDECL_PACKED);
        if (context.state->depthRangeType != GFX_DEPTH_RANGE_FULL)
            R_ChangeDepthRange(context.state, GFX_DEPTH_RANGE_FULL);
        R_SetupPassPerObjectArgs(context);
        R_SetupPassPerPrimArgs(context);
        drawSurfKeyb = *(unsigned int *)&drawSurf.fields & 0xE0000000;
        while (1)
        {
            R_DrawXModelSkinnedModelSurf(context, modelSurfa);
            if (++drawSurfIndex == drawSurfCount)
                break;
            *(unsigned int *)&drawSurf.fields = drawSurfList[drawSurfIndex].fields;
            if ((drawSurfList[drawSurfIndex].packed & 0xFFFFFFFFE0000000uLL) != __PAIR64__(
                HIDWORD(drawSurf.packed),
                drawSurfKeyb))
                break;
            modelSurfa = (const GfxModelSkinnedSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
            gfxEntIndexb = modelSurfa->info.gfxEntIndex;
            if (gfxEntIndexb != baseGfxEntIndexa)
            {
                if (modelSurfa->info.gfxEntIndex)
                {
                    if (materialTimea != data->gfxEnts[gfxEntIndexb].materialTime)
                        break;
                }
                else if (materialTimea != 0.0)
                {
                    break;
                }
            }
        }
    }
    RB_EndTrackImmediatePrims();
    //Profile_EndInternal(0);
    if (!drawSurfIndex)
        MyAssertHandler(".\\rb_tess.cpp", 1136, 0, "%s", "drawSurfIndex");
    return drawSurfIndex;
}

void __cdecl R_DrawXModelSkinnedModelSurf(GfxCmdBufContext context, const GfxModelSkinnedSurface *modelSurf)
{
    if (!modelSurf)
        MyAssertHandler(".\\rb_tess.cpp", 815, 0, "%s", "modelSurf");
    if (modelSurf->skinnedCachedOffset < 0)
        R_DrawXModelSkinnedUncached(context, modelSurf->xsurf, modelSurf->skinnedVert);
    else
        R_DrawXModelSkinnedCached(context, modelSurf);
}

void __cdecl R_DrawXModelSkinnedCached(GfxCmdBufContext context, const GfxModelSkinnedSurface *modelSurf)
{
    const char *v2; // eax
    GfxCmdBufSourceState *ActiveWorldMatrix; // eax
    const GfxBackEndData *data; // [esp+48h] [ebp-18h]
    XSurface *xsurf; // [esp+4Ch] [ebp-14h]
    IDirect3DIndexBuffer9 *ib; // [esp+50h] [ebp-10h] BYREF
    GfxDrawPrimArgs args; // [esp+54h] [ebp-Ch] BYREF

    //Profile_Begin(103);
    if (r_logFile->current.integer)
    {
        v2 = va("--- R_DrawXModelSkinnedCached( %s ) ---\n", context.state->material->info.name);
        RB_LogPrint(v2);
    }
    if (!modelSurf)
        MyAssertHandler(".\\rb_tess.cpp", 715, 0, "%s", "modelSurf");
    xsurf = modelSurf->xsurf;
    if (!xsurf)
        MyAssertHandler(".\\rb_tess.cpp", 718, 0, "%s", "xsurf");
    if (!xsurf->vertCount)
        MyAssertHandler(".\\rb_tess.cpp", 719, 0, "%s", "xsurf->vertCount");
    if (modelSurf->skinnedCachedOffset < 0)
        MyAssertHandler(".\\rb_tess.cpp", 721, 0, "%s", "modelSurf->skinnedCachedOffset >= 0");
    args.vertexCount = xsurf->vertCount;
    args.triCount = xsurf->triCount;
    g_frameStatsCur.geoIndexCount += 3 * args.triCount;
    DB_GetIndexBufferAndBase(xsurf->zoneHandle, xsurf->triIndices, (void **)&ib, &args.baseIndex);
    if (!ib)
        MyAssertHandler(".\\rb_tess.cpp", 733, 0, "%s", "ib");
    data = context.source->input.data;
    if (context.state->prim.indexBuffer != ib)
        R_ChangeIndices(&context.state->prim, ib);
    R_SetStreamSource(&context.state->prim, data->skinnedCacheVb->buffer, modelSurf->skinnedCachedOffset, 0x20u);
    R_DrawIndexedPrimitive(&context.state->prim, &args);
    if (!g_primStats)
        MyAssertHandler(".\\rb_tess.cpp", 750, 0, "%s", "g_primStats");
    g_primStats->staticIndexCount += 3 * args.triCount;
    g_primStats->staticVertexCount += args.vertexCount;
    if (r_showTess->current.integer)
    {
        ActiveWorldMatrix = R_GetActiveWorldMatrix(context.source);
        RB_ShowTess(context, ActiveWorldMatrix->matrices.matrix[0].m[3], "XMSkin$", colorWhite);
    }
    //Profile_EndInternal(0);
}

unsigned int __cdecl R_TessXModelRigidDrawSurfList(
    const GfxDrawSurfListArgs *listArgs,
    GfxCmdBufContext prepassContext)
{
    const char *v2; // eax
    GfxCmdBufContext context; // [esp+40h] [ebp-44h]
    const GfxDrawSurfListInfo *info; // [esp+48h] [ebp-3Ch]
    GfxDrawSurf drawSurf; // [esp+4Ch] [ebp-38h]
    const GfxBackEndData *data; // [esp+54h] [ebp-30h]
    GfxCmdBufSourceState *commonSource; // [esp+58h] [ebp-2Ch]
    MaterialTechniqueType baseTechType; // [esp+5Ch] [ebp-28h]
    int setupVertexShader; // [esp+60h] [ebp-24h]
    unsigned int drawSurfIndex; // [esp+64h] [ebp-20h]
    const GfxModelRigidSurface *modelSurf; // [esp+6Ch] [ebp-18h]
    GfxDepthRangeType depthHackFlags; // [esp+70h] [ebp-14h]
    unsigned int gfxEntIndex; // [esp+74h] [ebp-10h]
    float materialTime; // [esp+78h] [ebp-Ch]
    const GfxDrawSurf *drawSurfList; // [esp+7Ch] [ebp-8h]
    unsigned int drawSurfCount; // [esp+80h] [ebp-4h]

    //Profile_Begin(105);
    context = listArgs->context;
    commonSource = listArgs->context.source;
    if (prepassContext.state && commonSource != prepassContext.source)
        MyAssertHandler(
            ".\\rb_tess.cpp",
            1163,
            0,
            "%s",
            "prepassContext.state == NULL || commonSource == prepassContext.source");
    data = commonSource->input.data;
    info = listArgs->info;
    baseTechType = info->baseTechType;
    if (r_logFile->current.integer)
    {
        v2 = va("--- RB_TessXModelRigid( %s ) ---\n", context.state->material->info.name);
        RB_LogPrint(v2);
    }
    drawSurfCount = info->drawSurfCount - listArgs->firstDrawSurfIndex;
    drawSurfList = &info->drawSurfs[listArgs->firstDrawSurfIndex];
    drawSurf.fields = drawSurfList->fields;
    commonSource->objectPlacement = &s_manualObjectPlacement;
    RB_TrackImmediatePrims(GFX_PRIM_STATS_XMODELRIGID);
    if (!g_primStats)
        MyAssertHandler(".\\rb_tess.cpp", 1179, 0, "%s", "g_primStats");
    if (info->cameraView)
    {
        modelSurf = (const GfxModelRigidSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
        gfxEntIndex = modelSurf->surf.info.gfxEntIndex;
        if (modelSurf->surf.info.gfxEntIndex)
        {
            depthHackFlags = data->gfxEnts[gfxEntIndex].renderFxFlags & 2;
            materialTime = data->gfxEnts[gfxEntIndex].materialTime;
        }
        else
        {
            depthHackFlags = GFX_DEPTH_RANGE_SCENE;
            materialTime = 0.0;
        }
        setupVertexShader = R_UpdateMaterialTime(commonSource, materialTime);
        R_SetupPassCriticalPixelShaderArgs(context);
        if (setupVertexShader)
            R_SetupPassVertexShaderArgs(context);
        R_ChangeDepthHackNearClip(commonSource, depthHackFlags);
        R_SetVertexDeclTypeNormal(context.state, VERTDECL_PACKED);
        if (depthHackFlags != context.state->depthRangeType)
            R_ChangeDepthRange(context.state, depthHackFlags);
        if (baseTechType == TECHNIQUE_LIT_BEGIN)
        {
            if (sc_enable->current.enabled && ((drawSurf.packed >> 24) & 0x1F) != 0)
                R_SetCodeImageTexture(commonSource, 0x10u, (const GfxImage *)dword_EA74F68);
            else
                R_SetCodeImageTexture(commonSource, 0x10u, rgp.whiteImage);
            if (prepassContext.state)
            {
                R_SetupPassCriticalPixelShaderArgs(prepassContext);
                if (setupVertexShader)
                    R_SetupPassVertexShaderArgs(prepassContext);
                R_SetVertexDeclTypeNormal(prepassContext.state, VERTDECL_PACKED);
                if (depthHackFlags != prepassContext.state->depthRangeType)
                    R_ChangeDepthRange(prepassContext.state, depthHackFlags);
            }
            if (prepassContext.state)
                R_SetupPassPerObjectArgs(prepassContext);
            R_SetupPassPerObjectArgs(context);
            drawSurfIndex = R_DrawXModelRigidSurfLit(drawSurfList, drawSurfCount, context, prepassContext, info);
        }
        else
        {
            if (prepassContext.state)
                MyAssertHandler(".\\rb_tess.cpp", 1252, 0, "%s", "prepassContext.state == NULL");
            R_SetupPassPerObjectArgs(context);
            drawSurfIndex = R_DrawXModelRigidSurfCamera(drawSurfList, drawSurfCount, context);
        }
    }
    else
    {
        if (prepassContext.state)
            MyAssertHandler(".\\rb_tess.cpp", 1262, 0, "%s", "prepassContext.state == NULL");
        if (baseTechType == TECHNIQUE_LIT_BEGIN)
            MyAssertHandler(".\\rb_tess.cpp", 1263, 0, "%s", "baseTechType != TECHNIQUE_LIT");
        R_SetupPassCriticalPixelShaderArgs(context);
        R_ChangeDepthHackNearClip(commonSource, 0);
        R_SetVertexDeclTypeNormal(context.state, VERTDECL_PACKED);
        if (context.state->depthRangeType != GFX_DEPTH_RANGE_FULL)
            R_ChangeDepthRange(context.state, GFX_DEPTH_RANGE_FULL);
        R_SetupPassPerObjectArgs(context);
        drawSurfIndex = R_DrawXModelRigidSurf(drawSurfList, drawSurfCount, context);
    }
    RB_EndTrackImmediatePrims();
    //Profile_EndInternal(0);
    if (!drawSurfIndex)
        MyAssertHandler(".\\rb_tess.cpp", 1295, 0, "%s", "drawSurfIndex");
    return drawSurfIndex;
}

unsigned int __cdecl R_TessStaticModelRigidDrawSurfList(
    const GfxDrawSurfListArgs *listArgs,
    GfxCmdBufContext prepassContext)
{
    const char *v2; // eax
    GfxDepthRangeType v4; // [esp+1Ch] [ebp-38h]
    GfxDepthRangeType depthRangeType; // [esp+20h] [ebp-34h]
    GfxCmdBufContext context; // [esp+38h] [ebp-1Ch]
    const GfxDrawSurfListInfo *info; // [esp+40h] [ebp-14h]
    GfxCmdBufSourceState *commonSource; // [esp+48h] [ebp-Ch]
    MaterialTechniqueType baseTechType; // [esp+4Ch] [ebp-8h]
    const unsigned int *primDrawSurfPos; // [esp+50h] [ebp-4h]

    //Profile_Begin(106);
    context = listArgs->context;
    commonSource = listArgs->context.source;
    if (prepassContext.state && commonSource != prepassContext.source)
        MyAssertHandler(
            ".\\rb_tess.cpp",
            1476,
            0,
            "%s",
            "prepassContext.state == NULL || commonSource == prepassContext.source");
    info = listArgs->info;
    if (r_logFile->current.integer)
    {
        v2 = va("--- R_TessStaticModelRigidDrawSurfList( %s ) ---\n", context.state->material->info.name);
        RB_LogPrint(v2);
    }
    baseTechType = info->baseTechType;
    R_SetupPassCriticalPixelShaderArgs(context);
    commonSource->objectPlacement = &s_manualObjectPlacement;
    R_ChangeDepthHackNearClip(commonSource, 0);
    R_SetVertexDeclTypeNormal(context.state, VERTDECL_PACKED);
    depthRangeType = (context.source->cameraView != 0) - 1;
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    if (prepassContext.state)
    {
        R_SetupPassCriticalPixelShaderArgs(prepassContext);
        R_SetVertexDeclTypeNormal(prepassContext.state, VERTDECL_PACKED);
        v4 = (prepassContext.source->cameraView != 0) - 1;
        if (v4 != prepassContext.state->depthRangeType)
            R_ChangeDepthRange(prepassContext.state, v4);
    }
    primDrawSurfPos = &commonSource->input.data->primDrawSurfsBuf[(unsigned __int16)*(unsigned int *)&info->drawSurfs[listArgs->firstDrawSurfIndex].fields];
    RB_TrackImmediatePrims(GFX_PRIM_STATS_SMODELRIGID);
    if (!g_primStats)
        MyAssertHandler(".\\rb_tess.cpp", 1517, 0, "%s", "g_primStats");
    if (baseTechType == TECHNIQUE_LIT_BEGIN)
    {
        R_SetCodeImageTexture(commonSource, 0x10u, rgp.whiteImage);
        if (prepassContext.state)
            R_SetupPassPerObjectArgs(prepassContext);
        R_SetupPassPerObjectArgs(context);
        R_DrawStaticModelSurfLit(primDrawSurfPos, context);
    }
    else
    {
        if (prepassContext.state)
            MyAssertHandler(".\\rb_tess.cpp", 1532, 0, "%s", "prepassContext.state == NULL");
        R_SetupPassPerObjectArgs(context);
        R_DrawStaticModelSurf(primDrawSurfPos, context);
    }
    RB_EndTrackImmediatePrims();
    //Profile_EndInternal(0);
    return 1;
}

unsigned int __cdecl R_TessTrianglesList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    GfxDepthRangeType v3; // [esp+20h] [ebp-38h]
    GfxDepthRangeType depthRangeType; // [esp+24h] [ebp-34h]
    GfxCmdBufContext context; // [esp+3Ch] [ebp-1Ch]
    const GfxDrawSurfListInfo *info; // [esp+44h] [ebp-14h]
    GfxCmdBufSourceState *commonSource; // [esp+4Ch] [ebp-Ch]
    MaterialTechniqueType baseTechType; // [esp+50h] [ebp-8h]
    const unsigned int *primDrawSurfPos; // [esp+54h] [ebp-4h]

    //Profile_Begin(108);
    context = listArgs->context;
    commonSource = listArgs->context.source;
    if (prepassContext.state && commonSource != prepassContext.source)
        MyAssertHandler(
            ".\\rb_tess.cpp",
            1729,
            0,
            "%s",
            "prepassContext.state == NULL || commonSource == prepassContext.source");
    info = listArgs->info;
    baseTechType = info->baseTechType;
    R_SetupPassCriticalPixelShaderArgs(context);
    if (commonSource->objectPlacement != &rg.identityPlacement)
        R_ChangeObjectPlacement(commonSource, &rg.identityPlacement);
    R_ChangeDepthHackNearClip(commonSource, 0);
    R_SetVertexDeclTypeWorld(context.state);
    depthRangeType = (context.source->cameraView != 0) - 1;
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    if (prepassContext.state)
    {
        R_SetupPassCriticalPixelShaderArgs(prepassContext);
        R_SetVertexDeclTypeWorld(prepassContext.state);
        v3 = (prepassContext.source->cameraView != 0) - 1;
        if (v3 != prepassContext.state->depthRangeType)
            R_ChangeDepthRange(prepassContext.state, v3);
    }
    primDrawSurfPos = &commonSource->input.data->primDrawSurfsBuf[(unsigned __int16)*(unsigned int *)&info->drawSurfs[listArgs->firstDrawSurfIndex].fields];
    R_TrackPrims(context.state, GFX_PRIM_STATS_WORLD);
    if (baseTechType == TECHNIQUE_LIT_BEGIN)
    {
        if (sc_enable->current.enabled)
            R_SetCodeImageTexture(commonSource, 0x10u, (const GfxImage *)dword_EA74F68);
        else
            R_SetCodeImageTexture(commonSource, 0x10u, rgp.whiteImage);
        R_SetupPassPerObjectArgs(context);
        R_SetupPassPerPrimArgs(context);
        if (prepassContext.state)
        {
            R_SetupPassPerObjectArgs(prepassContext);
            R_SetupPassPerPrimArgs(prepassContext);
        }
        R_DrawBspDrawSurfsLit(primDrawSurfPos, context, prepassContext);
    }
    else
    {
        R_SetupPassPerObjectArgs(context);
        R_SetupPassPerPrimArgs(context);
        R_DrawBspDrawSurfs(primDrawSurfPos, context.state);
    }
    g_primStats = 0;
    //Profile_EndInternal(0);
    return 1;
}

void __cdecl R_SetVertexDeclTypeWorld(GfxCmdBufState *state)
{
    if ((state->technique->flags & 8) != 0)
        R_SetVertexDeclTypeNormal(state, (MaterialVertexDeclType)(state->material->techniqueSet->worldVertFormat + 2));
    else
        R_SetVertexDeclTypeNormal(state, VERTDECL_WORLD);
}

unsigned int __cdecl R_TessBModel(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    GfxDepthRangeType v3; // [esp+34h] [ebp-88h]
    GfxDepthRangeType depthRangeType; // [esp+38h] [ebp-84h]
    GfxCmdBufContext context; // [esp+50h] [ebp-6Ch]
    const GfxDrawSurfListInfo *info; // [esp+58h] [ebp-64h]
    GfxDrawSurf drawSurf; // [esp+5Ch] [ebp-60h]
    const GfxBackEndData *data; // [esp+68h] [ebp-54h]
    GfxCmdBufSourceState *commonSource; // [esp+6Ch] [ebp-50h]
    const srfTriangles_t *tris; // [esp+70h] [ebp-4Ch]
    unsigned __int64 drawSurfSubKey; // [esp+74h] [ebp-48h]
    GfxDrawSurf drawSurfSubMask; // [esp+7Ch] [ebp-40h]
    MaterialTechniqueType baseTechType; // [esp+84h] [ebp-38h]
    unsigned int drawSurfIndex; // [esp+88h] [ebp-34h]
    const BModelSurface *bmodelSurf; // [esp+98h] [ebp-24h]
    const GfxDrawSurf *drawSurfList; // [esp+9Ch] [ebp-20h]
    GfxDrawPrimArgs args; // [esp+A0h] [ebp-1Ch] BYREF
    unsigned __int64 drawSurfKey; // [esp+ACh] [ebp-10h]
    unsigned int drawSurfCount; // [esp+B8h] [ebp-4h]

    //Profile_Begin(109);
    context = listArgs->context;
    commonSource = listArgs->context.source;
    if (prepassContext.state && commonSource != prepassContext.source)
        MyAssertHandler(
            ".\\rb_tess.cpp",
            1882,
            0,
            "%s",
            "prepassContext.state == NULL || commonSource == prepassContext.source");
    info = listArgs->info;
    drawSurfList = &info->drawSurfs[listArgs->firstDrawSurfIndex];
    drawSurfCount = info->drawSurfCount - listArgs->firstDrawSurfIndex;
    R_SetupPassCriticalPixelShaderArgs(context);
    data = commonSource->input.data;
    baseTechType = info->baseTechType;
    if (baseTechType == TECHNIQUE_LIT_BEGIN)
    {
        if (sc_enable->current.enabled)
            R_SetCodeImageTexture(commonSource, 0x10u, (const GfxImage *)dword_EA74F68);
        else
            R_SetCodeImageTexture(commonSource, 0x10u, rgp.whiteImage);
    }
    R_ChangeDepthHackNearClip(commonSource, 0);
    R_SetVertexDeclTypeWorld(context.state);
    depthRangeType = (context.source->cameraView != 0) - 1;
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    if (prepassContext.state)
    {
        R_SetupPassCriticalPixelShaderArgs(prepassContext);
        R_SetVertexDeclTypeWorld(prepassContext.state);
        v3 = (prepassContext.source->cameraView != 0) - 1;
        if (v3 != prepassContext.state->depthRangeType)
            R_ChangeDepthRange(prepassContext.state, v3);
        R_SetupPassPerObjectArgs(prepassContext);
    }
    drawSurf.fields = drawSurfList->fields;
    drawSurfSubMask.packed = -65536;
    if (baseTechType != TECHNIQUE_LIT_BEGIN)
    {
        *(unsigned int *)&drawSurfSubMask.fields = -536870912;
        R_SetupPassPerObjectArgs(context);
    }
    drawSurfKey = drawSurf.packed & 0xFFFFFFFFE0000000uLL;
    R_TrackPrims(context.state, GFX_PRIM_STATS_BMODEL);
    drawSurfIndex = 0;
    do
    {
        if (baseTechType == TECHNIQUE_LIT_BEGIN)
        {
            R_SetLightmap(context, (drawSurf.packed >> 24) & 0x1F);
            R_SetReflectionProbe(context, BYTE2(drawSurf.packed));
            R_SetupPassPerObjectArgs(context);
        }
        drawSurfSubKey = drawSurfSubMask.packed & drawSurf.packed;
        do
        {
            bmodelSurf = (const BModelSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
            if (commonSource->objectPlacement != bmodelSurf->placement)
                R_ChangeObjectPlacement(commonSource, bmodelSurf->placement);
            tris = &bmodelSurf->surf->tris;
            if (!tris->triCount)
                MyAssertHandler(".\\rb_tess.cpp", 1969, 0, "%s", "tris->triCount");
            args.vertexCount = tris->vertexCount;
            args.triCount = tris->triCount;
            g_frameStatsCur.geoIndexCount += 3 * args.triCount;
            R_SetStreamsForBspSurface(&context.state->prim, tris);
            R_SetupPassPerPrimArgs(context);
            args.baseIndex = R_SetIndexData(
                &context.state->prim,
                (unsigned __int8 *)&rgp.world->indices[tris->baseIndex],
                args.triCount);
            R_DrawIndexedPrimitive(&context.state->prim, &args);
            if (prepassContext.state)
            {
                R_SetStreamsForBspSurface(&prepassContext.state->prim, tris);
                R_SetupPassPerPrimArgs(prepassContext);
                args.baseIndex = R_SetIndexData(
                    &prepassContext.state->prim,
                    (unsigned __int8 *)&rgp.world->indices[tris->baseIndex],
                    args.triCount);
                R_DrawIndexedPrimitive(&prepassContext.state->prim, &args);
            }
            if (!g_primStats)
                MyAssertHandler(".\\rb_tess.cpp", 2005, 0, "%s", "g_primStats");
            g_primStats->dynamicIndexCount += 3 * args.triCount;
            g_primStats->dynamicVertexCount += args.vertexCount;
            args.triCount = 0;
            if (++drawSurfIndex == drawSurfCount)
                break;
            drawSurf.fields = (GfxDrawSurfFields)drawSurfList[drawSurfIndex];
        } while ((drawSurfSubMask.packed & drawSurf.packed) == drawSurfSubKey);
    } while (drawSurfIndex != drawSurfCount && (drawSurf.packed & 0xFFFFFFFFE0000000uLL) == drawSurfKey);
    g_primStats = 0;
    //Profile_EndInternal(0);
    return drawSurfIndex;
}

