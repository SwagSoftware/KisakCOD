#include "r_draw_sunshadow.h"
#include "rb_backend.h"
#include "r_state.h"
#include "r_utils.h"
#include "r_dvars.h"
#include "rb_postfx.h"


void __cdecl R_DrawSunShadowMapCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    int height; // [esp+10h] [ebp-28h]
    int width; // [esp+14h] [ebp-24h]
    int verticalOffset; // [esp+18h] [ebp-20h]
    RECT rect; // [esp+24h] [ebp-14h] BYREF
    const GfxSunShadowPartition *partition; // [esp+34h] [ebp-4h]

    partition = (const GfxSunShadowPartition * )userData;
    R_SetRenderTarget(context, R_RENDERTARGET_SHADOWMAP_SUN);

    if (partition->partitionIndex == 0)
        R_ClearScreen(context.state->prim.device, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, shadowmapClearColor, 1.0f, 0, 0);

    height = partition->viewport.height;
    width = partition->viewport.width;
    verticalOffset = partition->viewport.y + (partition->partitionIndex * 1024);

    rect.left = partition->viewport.x;
    rect.top = verticalOffset;
    rect.right = partition->viewport.x + width;
    rect.bottom = verticalOffset + height;

    context.state->prim.device->SetRenderState(D3DRS_SCISSORTESTENABLE, 1);
    context.state->prim.device->SetScissorRect(&rect);

    R_DrawSurfs(context, 0, &partition->info);

    context.state->prim.device->SetRenderState(D3DRS_SCISSORTESTENABLE, 0);
}

void R_DrawSunShadowMap(
    const GfxViewInfo *viewInfo,
    unsigned int partitionIndex,
    GfxCmdBuf *cmdBuf)
{
    float x; // [esp+14h] [ebp-F28h]
    GfxCmdBufSourceState state; // [esp+20h] [ebp-F1Ch] BYREF

    R_InitCmdBufSourceState(&state, &viewInfo->input, 0);
    R_SetRenderTargetSize(&state, R_RENDERTARGET_SHADOWMAP_SUN);

    if (r_rendererInUse->current.integer || gfxMetrics.shadowmapBuildTechType != TECHNIQUE_BUILD_SHADOWMAP_COLOR)
    {
        x = sm_polygonOffsetBias->current.value * 0.25f * viewInfo->sunShadow.partition[partitionIndex].shadowViewParms.projectionMatrix.m[2][2];
        R_UpdateCodeConstant(&state, CONST_SRC_CODE_SHADOWMAP_POLYGON_OFFSET, x, sm_polygonOffsetScale->current.value, 0.0f, 0.0f);
    }
    else
    {
        x = sm_polygonOffsetBias->current.value * 4.0f * viewInfo->sunShadow.partition[partitionIndex].shadowViewParms.projectionMatrix.m[2][2];
        R_UpdateCodeConstant(&state, CONST_SRC_CODE_SHADOWMAP_POLYGON_OFFSET, x, 0.0f, 0.0f, 0.0f);
    }

    R_SetViewportValues(&state, 0, partitionIndex * 1024, 1024, 1024);
    R_DrawCall(
        R_DrawSunShadowMapCallback,
        &viewInfo->sunShadow.partition[partitionIndex],
        &state,
        viewInfo,
        &viewInfo->sunShadow.partition[partitionIndex].info,
        &viewInfo->sunShadow.partition[partitionIndex].shadowViewParms,
        cmdBuf,
        0);
}

// LWSS ADD from blops
void __cdecl DrawSunDirectionDebug(const float *viewOrg, const float *viewForward)
{
    float boxColor[4]; // [esp+4h] [ebp-A4h] BYREF
    float lineEnd[3]; // [esp+14h] [ebp-94h] BYREF
    float zdir[3]; // [esp+20h] [ebp-88h] BYREF
    float sunColor[4]; // [esp+2Ch] [ebp-7Ch] BYREF
    float mins[3]; // [esp+3Ch] [ebp-6Ch] BYREF
    float zColor[4]; // [esp+48h] [ebp-60h] BYREF
    float xColor[4]; // [esp+58h] [ebp-50h] BYREF
    float ext[3]; // [esp+68h] [ebp-40h]
    float ydir[3]; // [esp+74h] [ebp-34h] BYREF
    float maxs[3]; // [esp+80h] [ebp-28h] BYREF
    float yColor[4]; // [esp+8Ch] [ebp-1Ch] BYREF
    float xdir[3]; // [esp+9Ch] [ebp-Ch] BYREF

    static bool lastShowDebug;
    static float debugOrigin[3];

    if (r_showSunDirectionDebug->current.enabled)
    {
        if (!lastShowDebug)
        {
            debugOrigin[0] = (float)(100.0 * *viewForward) + *viewOrg;
            debugOrigin[1] = (float)(100.0 * viewForward[1]) + viewOrg[1];
            debugOrigin[2] = (float)(100.0 * viewForward[2]) + viewOrg[2];
        }
        lineEnd[0] = (float)(200.0 * frontEndDataOut->sunLight.dir[0]) + debugOrigin[0];
        lineEnd[1] = (float)(200.0 * frontEndDataOut->sunLight.dir[1]) + debugOrigin[1];
        lineEnd[2] = (float)(200.0 * frontEndDataOut->sunLight.dir[2]) + debugOrigin[2];
        sunColor[0] = 1.0f;
        sunColor[1] = 1.0f;
        sunColor[2] = 1.0f;
        sunColor[3] = 1.0f;
        R_AddDebugLine(&frontEndDataOut->debugGlobals, debugOrigin, lineEnd, sunColor);
        boxColor[0] = 1.0f;
        boxColor[1] = 1.0f;
        boxColor[2] = 1.0f;
        boxColor[3] = 1.0f;
        ext[0] = 5.0f;
        ext[1] = 5.0f;
        ext[2] = 5.0f;
        maxs[0] = debugOrigin[0] + 5.0;
        maxs[1] = debugOrigin[1] + 5.0;
        maxs[2] = debugOrigin[2] + 5.0;
        mins[0] = (float)(-1.0 * 5.0) + debugOrigin[0];
        mins[1] = (float)(-1.0 * 5.0) + debugOrigin[1];
        mins[2] = (float)(-1.0 * 5.0) + debugOrigin[2];
        R_AddDebugBox(&frontEndDataOut->debugGlobals, mins, maxs, boxColor);
        xColor[0] = 1.0f;
        xColor[1] = 0.0f;
        xColor[2] = 0.0f;
        xColor[3] = 1.0f;
        yColor[0] = 0.0f;
        yColor[1] = 1.0f;
        yColor[2] = 0.0f;
        yColor[3] = 1.0f;
        zColor[0] = 0.0f;
        zColor[1] = 0.0f;
        zColor[2] = 1.0f;
        zColor[3] = 1.0f;
        xdir[0] = (float)(40.0 * 1.0) + debugOrigin[0];
        xdir[1] = (float)(40.0 * 0.0) + debugOrigin[1];
        xdir[2] = (float)(40.0 * 0.0) + debugOrigin[2];
        ydir[0] = (float)(40.0 * 0.0) + debugOrigin[0];
        ydir[1] = (float)(40.0 * 1.0) + debugOrigin[1];
        ydir[2] = xdir[2];
        zdir[0] = ydir[0];
        zdir[1] = xdir[1];
        zdir[2] = (float)(40.0 * 1.0) + debugOrigin[2];
        R_AddDebugLine(&frontEndDataOut->debugGlobals, debugOrigin, xdir, xColor);
        R_AddDebugLine(&frontEndDataOut->debugGlobals, debugOrigin, ydir, yColor);
        R_AddDebugLine(&frontEndDataOut->debugGlobals, debugOrigin, zdir, zColor);
    }
    lastShowDebug = r_showSunDirectionDebug->current.enabled;
}