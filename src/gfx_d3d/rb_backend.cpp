#include "rb_backend.h"
#include <qcommon/mem_track.h>

#include "rb_logfile.h"
#include "rb_stats.h"
#include "r_image.h"
#include "rb_state.h"
#include "rb_shade.h"
#include "r_cmdbuf.h"
#include "r_utils.h"
#include <EffectsCore/fx_system.h>
#include "r_draw_shadowable_light.h"
#include "r_state.h"
#include "r_draw_material.h"
#include "r_shade.h"
#include "r_setstate_d3d.h"
#include <win32/win_local.h>
#include "rb_pixelcost.h"
#include "rb_drawprofile.h"
#include <stringed/stringed_hooks.h>
#include "rb_draw3d.h"
#include "r_dvars.h"
#include "r_pixelcost_load_obj.h"
#include <win32/win_net.h>
#include <qcommon/threads.h>
#include "r_workercmds.h"
#include "rb_tess.h"
#include "r_cinematic.h"
#include "r_model_lighting.h"
#include "r_draw_bsp.h"
#include "r_dobj_skin.h"
#include "r_draw_xmodel.h"
#include "r_staticmodelcache.h"
#include "rb_uploadshaders.h"
#include <universal/timing.h>

#include <setjmp.h>
#ifdef KISAK_SP
#include <client/cl_scrn.h>
#endif

void(__cdecl *const RB_RenderCommandTable[22])(GfxRenderCommandExecState *) =
{
  NULL,
  &RB_SetMaterialColorCmd,
  &RB_SaveScreenCmd,
  &RB_SaveScreenSectionCmd,
  &RB_ClearScreenCmd,
  &RB_SetViewportCmd,
  &RB_StretchPicCmd,
  &RB_StretchPicCmdFlipST,
  &RB_StretchPicRotateXYCmd,
  &RB_StretchPicRotateSTCmd,
  &RB_StretchRawCmd,
  &RB_DrawQuadPicCmd,
  &RB_DrawFullScreenColoredQuadCmd,
  &RB_DrawText2DCmd,
  &RB_DrawText3DCmd,
  &RB_BlendSavedScreenBlurredCmd,
  &RB_BlendSavedScreenFlashedCmd,
  &RB_DrawPointsCmd,
  &RB_DrawLinesCmd,
  &RB_DrawTrianglesCmd,
  &RB_DrawProfileCmd,
  &RB_ProjectionSetCmd
}; // idb

GfxBackEndData *backEndData;
GfxRenderTarget gfxRenderTargets[17]; // LWSS: changed to 17 to please ASAN. (GfxRenderTargetId)

r_backEndGlobals_t backEnd;
materialCommands_t tess;

void __cdecl TRACK_rb_backend()
{
    track_static_alloc_internal(&backEnd, 640, "backEnd", 18);
    track_static_alloc_internal(&tess, 2271584, "tess", 18);
    track_static_alloc_internal(&gfxCmdBufInput, 1072, "gfxCmdBufInput", 18);
}

bool __cdecl R_GpuFenceTimeout()
{
    if (RB_IsGpuFenceFinished())
        return 1;
    dx.gpuSyncEnd = __rdtsc();
    return dx.gpuSyncEnd - dx.gpuSyncStart >= dx.gpuSyncDelay;
}

void __cdecl R_FinishGpuFence()
{
    PROF_SCOPED("R_SyncGpu");

    while (!RB_IsGpuFenceFinished());
}

void __cdecl R_AcquireGpuFenceLock()
{
    Sys_EnterCriticalSection(CRITSECT_GPU_FENCE);
}

void __cdecl R_ReleaseGpuFenceLock()
{
    Sys_LeaveCriticalSection(CRITSECT_GPU_FENCE);
}

void __cdecl R_InsertGpuFence()
{
    const char *v0; // eax
    int hr; // [esp+0h] [ebp-4h]

	// KISAKGPUFENCE: Comment asserts out for now. Sometimes goes off when alt-tabbing.
    //if (dx.flushGpuQueryCount)
    //    MyAssertHandler(".\\rb_backend.cpp", 2602, 0, "%s", "!dx.flushGpuQueryCount");
    //if (!dx.flushGpuQuery)
    //    MyAssertHandler(".\\rb_backend.cpp", 2604, 0, "%s", "dx.flushGpuQuery");
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("dx.flushGpuQuery->Issue( (1 << 0) )\n");
        hr = dx.flushGpuQuery->Issue(1);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v0 = R_ErrorDescription(hr);
                Com_Error(ERR_FATAL, ".\\rb_backend.cpp (%i) dx.flushGpuQuery->Issue( (1 << 0) ) failed: %s\n", 2605, v0);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    dx.flushGpuQueryIssued = 1;
    ++dx.flushGpuQueryCount;
}

void RB_AbandonGpuFence()
{
    iassert( dx.flushGpuQuery );
    iassert( dx.flushGpuQueryIssued );
    iassert( dx.flushGpuQueryCount == 1 );
    dx.flushGpuQueryIssued = 0;
    --dx.flushGpuQueryCount;
}

void __cdecl RB_CopyBackendStats()
{
    rg.stats->c_indexes = g_frameStatsCur.geoIndexCount;
    rg.stats->c_fxIndexes = g_frameStatsCur.fxIndexCount;
    rg.stats->c_viewIndexes = RB_Stats_ViewIndexCount(g_frameStatsCur.viewStats);
    rg.stats->c_shadowIndexes = RB_Stats_ViewIndexCount(&g_frameStatsCur.viewStats[1]);
    rg.stats->c_vertexes = RB_Stats_TotalVertexCount();
    rg.stats->c_batches = RB_Stats_TotalPrimCount();
    R_SumOfUsedImages(&rg.stats->c_imageUsage);
    rg.stats->dc = 0.0;
}

void __cdecl RB_SetIdentity()
{
    if (gfxCmdBufSourceState.viewMode != VIEW_MODE_IDENTITY)
    {
        if (tess.indexCount)
            RB_EndTessSurface();
        gfxCmdBufSourceState.viewMode = VIEW_MODE_IDENTITY;
        memcpy(&gfxCmdBufSourceState.viewParms, &rg, sizeof(gfxCmdBufSourceState.viewParms));
        gfxCmdBufSourceState.eyeOffset[0] = 0.0;
        gfxCmdBufSourceState.eyeOffset[1] = 0.0;
        gfxCmdBufSourceState.eyeOffset[2] = 0.0;
        gfxCmdBufSourceState.eyeOffset[3] = 1.0;
        R_CmdBufSet3D(&gfxCmdBufSourceState);
    }
}

void __cdecl R_SetVertex2d(GfxVertex *vert, float x, float y, float s, float t, unsigned int color)
{
    vert->xyzw[0] = x;
    vert->xyzw[1] = y;
    vert->xyzw[2] = 0.0;
    vert->xyzw[3] = 1.0;
    vert->normal.packed = 1073643391;
    vert->color.packed = color;
    vert->texCoord[0] = s;
    vert->texCoord[1] = t;
}

void __cdecl R_SetVertex4dWithNormal(
    GfxVertex *vert,
    float x,
    float y,
    float z,
    float w,
    float nx,
    float ny,
    float nz,
    float s,
    float t,
    const unsigned __int8 *color)
{
    PackedUnitVec v11; // [esp+28h] [ebp-30h]

    vert->xyzw[0] = x;
    vert->xyzw[1] = y;
    vert->xyzw[2] = z;
    vert->xyzw[3] = w;
    v11.array[0] = (int)(nx * 127.0 + 127.5);
    v11.array[1] = (int)(ny * 127.0 + 127.5);
    v11.array[2] = (int)(nz * 127.0 + 127.5);
    v11.array[3] = 63;
    vert->normal = v11;
    vert->color.packed = *(unsigned int *)color;
    vert->texCoord[0] = s;
    vert->texCoord[1] = t;
}

void __cdecl RB_DrawStretchPic(
    const Material *material,
    float x,
    float y,
    float w,
    float h,
    float s0,
    float t0,
    float s1,
    float t1,
    unsigned int color,
    GfxPrimStatsTarget statsTarget)
{
    unsigned __int16 vertCount; // [esp+24h] [ebp-4h]

    iassert(gfxCmdBufSourceState.viewMode == VIEW_MODE_2D);

    RB_SetTessTechnique(material, TECHNIQUE_UNLIT);
    R_TrackPrims(&gfxCmdBufState, statsTarget);
    RB_CheckTessOverflow(4, 6);
    vertCount = tess.vertexCount;
    tess.indices[tess.indexCount] = vertCount + 3;
    tess.indices[tess.indexCount + 1] = vertCount;
    tess.indices[tess.indexCount + 2] = vertCount + 2;
    tess.indices[tess.indexCount + 3] = vertCount + 2;
    tess.indices[tess.indexCount + 4] = vertCount;
    tess.indices[tess.indexCount + 5] = vertCount + 1;
    R_SetVertex2d(&tess.verts[tess.vertexCount], x, y, s0, t0, color);
    R_SetVertex2d(&tess.verts[tess.vertexCount + 1], x + w, y, s1, t0, color);
    R_SetVertex2d(&tess.verts[tess.vertexCount + 2], x + w, y + h, s1, t1, color);
    R_SetVertex2d(&tess.verts[tess.vertexCount + 3], x, y + h, s0, t1, color);
    tess.vertexCount += 4;
    tess.indexCount += 6;
}

void __cdecl RB_CheckTessOverflow(int vertexCount, int indexCount)
{
    if (vertexCount > 5450)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\rb_backend.h",
            153,
            0,
            "%s\n\t(vertexCount) = %i",
            "(vertexCount <= 5450)",
            vertexCount);
    if (indexCount > 0x100000)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\rb_backend.h",
            154,
            0,
            "%s\n\t(indexCount) = %i",
            "(indexCount <= ((2 * 1024 * 1024) / 2))",
            indexCount);
    if (vertexCount + tess.vertexCount > 5450 || indexCount + tess.indexCount > 0x100000)
        RB_TessOverflow();
}

void __cdecl RB_DrawStretchPicFlipST(
    const Material *material,
    float x,
    float y,
    float w,
    float h,
    float s0,
    float t0,
    float s1,
    float t1,
    unsigned int color,
    GfxPrimStatsTarget statsTarget)
{
    float v11; // [esp+1Ch] [ebp-Ch]
    float v12; // [esp+20h] [ebp-8h]
    unsigned __int16 vertCount; // [esp+24h] [ebp-4h]

    iassert( gfxCmdBufSourceState.viewMode == VIEW_MODE_2D );
    RB_SetTessTechnique(material, TECHNIQUE_UNLIT);
    R_TrackPrims(&gfxCmdBufState, statsTarget);
    RB_CheckTessOverflow(4, 6);
    vertCount = tess.vertexCount;
    tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 3;
    tess.indices[tess.indexCount + 1] = vertCount;
    tess.indices[tess.indexCount + 2] = vertCount + 2;
    tess.indices[tess.indexCount + 3] = vertCount + 2;
    tess.indices[tess.indexCount + 4] = vertCount;
    tess.indices[tess.indexCount + 5] = vertCount + 1;
    R_SetVertex2d(&tess.verts[tess.vertexCount], x, y, s0, t0, color);
    v12 = x + w;
    R_SetVertex2d(&tess.verts[tess.vertexCount + 1], v12, y, s0, t1, color);
    v11 = y + h;
    R_SetVertex2d(&tess.verts[tess.vertexCount + 2], v12, v11, s1, t1, color);
    R_SetVertex2d(&tess.verts[tess.vertexCount + 3], x, v11, s1, t0, color);
    tess.vertexCount += 4;
    tess.indexCount += 6;
}

void __cdecl RB_DrawFullScreenColoredQuad(
    const Material *material,
    float s0,
    float t0,
    float s1,
    float t1,
    unsigned int color)
{
    float screenWidth; // [esp+28h] [ebp-8h]
    float screenHeight; // [esp+2Ch] [ebp-4h]

    if (tess.indexCount)
        RB_EndTessSurface();
    R_Set2D(&gfxCmdBufSourceState);
    screenWidth = (float)gfxCmdBufSourceState.renderTargetWidth;
    screenHeight = (float)gfxCmdBufSourceState.renderTargetHeight;
    RB_DrawStretchPic(material, 0.0, 0.0, screenWidth, screenHeight, s0, t0, s1, t1, color, GFX_PRIM_STATS_CODE);
    RB_EndTessSurface();
}

void __cdecl RB_FullScreenColoredFilter(const Material *material, unsigned int color)
{
    RB_DrawFullScreenColoredQuad(material, 0.0, 0.0, 1.0, 1.0, color);
}

void __cdecl RB_FullScreenFilter(const Material *material)
{
    RB_FullScreenColoredFilter(material, 0xFFFFFFFF);
}

void __cdecl RB_SplitScreenFilter(const Material *material, const GfxViewInfo *viewInfo)
{
    float t0; // [esp+28h] [ebp-20h] BYREF
    float t1; // [esp+2Ch] [ebp-1Ch] BYREF
    float s1; // [esp+30h] [ebp-18h] BYREF
    float s0; // [esp+34h] [ebp-14h] BYREF
    float x; // [esp+38h] [ebp-10h]
    float y; // [esp+3Ch] [ebp-Ch]
    float h; // [esp+40h] [ebp-8h]
    float w; // [esp+44h] [ebp-4h]

    if (tess.indexCount)
        RB_EndTessSurface();
    R_Set2D(&gfxCmdBufSourceState);
    x = (float)viewInfo->displayViewport.x;
    y = (float)viewInfo->displayViewport.y;
    w = (float)viewInfo->displayViewport.width;
    h = (float)viewInfo->displayViewport.height;
    RB_SplitScreenTexCoords(x, y, w, h, &s0, &t0, &s1, &t1);
    RB_DrawStretchPic(material, 0.0, 0.0, w, h, s0, t0, s1, t1, 0xFFFFFFFF, GFX_PRIM_STATS_CODE);
    RB_EndTessSurface();
}

void __cdecl RB_SplitScreenTexCoords(float x, float y, float w, float h, float *s0, float *t0, float *s1, float *t1)
{
    float screenWidth; // [esp+0h] [ebp-8h]
    float screenHeight; // [esp+4h] [ebp-4h]
    float xa; // [esp+10h] [ebp+8h]
    float ya; // [esp+14h] [ebp+Ch]
    float wa; // [esp+18h] [ebp+10h]
    float ha; // [esp+1Ch] [ebp+14h]

    iassert( s0 );
    iassert( s1 );
    iassert( t0 );
    iassert( t1 );
    screenWidth = (float)gfxCmdBufSourceState.renderTargetWidth;
    screenHeight = (float)gfxCmdBufSourceState.renderTargetHeight;
    xa = x / screenWidth;
    ya = y / screenHeight;
    wa = w / screenWidth;
    ha = h / screenHeight;
    *s0 = xa;
    *t0 = ya;
    *s1 = xa + wa;
    *t1 = ya + ha;
}

void __cdecl R_Resolve(GfxCmdBufContext context, GfxImage *image)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    int v6; // [esp+0h] [ebp-Ch]
    int hr; // [esp+4h] [ebp-8h]
    IDirect3DSurface9 *imageSurface; // [esp+8h] [ebp-4h]

    iassert( image );
    if (image->width != gfxRenderTargets[context.state->renderTargetId].width)
    {
        v2 = va("%s,%i", image->name, context.state->renderTargetId);
        MyAssertHandler(
            ".\\rb_backend.cpp",
            661,
            0,
            "%s\n\t%s",
            "image->width == gfxRenderTargets[context.state->renderTargetId].width",
            v2);
    }
    if (image->height != gfxRenderTargets[context.state->renderTargetId].height)
    {
        v3 = va("%s,%i", image->name, context.state->renderTargetId);
        MyAssertHandler(
            ".\\rb_backend.cpp",
            662,
            0,
            "%s\n\t%s",
            "image->height == gfxRenderTargets[context.state->renderTargetId].height",
            v3);
    }
    iassert( image != gfxRenderTargets[context.state->renderTargetId].image );
    imageSurface = Image_GetSurface(image);
    iassert( imageSurface );
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint(
                "context.state->prim.device->StretchRect( gfxRenderTargets[context.state->renderTargetId].surface.color, 0, image"
                "Surface, 0, D3DTEXF_LINEAR )\n");
        //hr = context.state->prim.device->StretchRect(
        //    context.state->prim.device,
        //    gfxRenderTargets[context.state->renderTargetId].surface.color,
        //    0,
        //    imageSurface,
        //    0,
        //    D3DTEXF_LINEAR);
        hr = context.state->prim.device->StretchRect(gfxRenderTargets[context.state->renderTargetId].surface.color, 0, imageSurface, 0, D3DTEXF_LINEAR);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v4 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    ".\\rb_backend.cpp (%i) context.state->prim.device->StretchRect( gfxRenderTargets[context.state->renderTargetId"
                    "].surface.color, 0, imageSurface, 0, D3DTEXF_LINEAR ) failed: %s\n",
                    672,
                    v4);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("imageSurface->Release()\n");
        //v6 = imageSurface->Release(imageSurface);
        v6 = imageSurface->Release();
        if (v6 < 0)
        {
            do
            {
                ++g_disableRendering;
                Com_Error(ERR_FATAL, ".\\rb_backend.cpp (%i) imageSurface->Release() failed: %s\n", 674, R_ErrorDescription(v6));
            } while (alwaysfails);
        }
    } while (alwaysfails);
}

void __cdecl RB_StretchPicCmd(GfxRenderCommandExecState *execState)
{
    GfxCmdStretchPic *cmd = (GfxCmdStretchPic *)execState->cmd;

    RB_DrawStretchPic(cmd->material, cmd->x, cmd->y, cmd->w, cmd->h, cmd->s0, cmd->t0, cmd->s1, cmd->t1, cmd->color, GFX_PRIM_STATS_HUD);
    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_StretchPicCmdFlipST(GfxRenderCommandExecState *execState)
{
    GfxCmdStretchPic *cmd = (GfxCmdStretchPic *)execState->cmd;

    RB_DrawStretchPicFlipST(cmd->material, cmd->x, cmd->y, cmd->w, cmd->h, cmd->s0, cmd->t0, cmd->s1, cmd->t1, cmd->color, GFX_PRIM_STATS_HUD);
    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_StretchPicRotateXYCmd(GfxRenderCommandExecState *execState)
{
    float v1; // [esp+14h] [ebp-64h]
    float v2; // [esp+18h] [ebp-60h]
    float v3; // [esp+1Ch] [ebp-5Ch]
    float v4; // [esp+20h] [ebp-58h]
    float v5; // [esp+24h] [ebp-54h]
    float v6; // [esp+28h] [ebp-50h]
    float x; // [esp+2Ch] [ebp-4Ch]
    float y; // [esp+30h] [ebp-48h]
    float v9; // [esp+40h] [ebp-38h]
    float halfWidth; // [esp+44h] [ebp-34h]
    float stepY; // [esp+48h] [ebp-30h]
    float stepY_4; // [esp+4Ch] [ebp-2Ch]
    float cosAngle; // [esp+50h] [ebp-28h]
    float stepX; // [esp+54h] [ebp-24h]
    float stepX_4; // [esp+58h] [ebp-20h]
    int indexCount; // [esp+5Ch] [ebp-1Ch]
    float midX; // [esp+60h] [ebp-18h]
    float sinAngle; // [esp+64h] [ebp-14h]
    unsigned __int16 vertCount; // [esp+68h] [ebp-10h]
    float midY; // [esp+6Ch] [ebp-Ch]
    const GfxCmdStretchPicRotateXY *cmd; // [esp+70h] [ebp-8h]
    float halfHeight; // [esp+74h] [ebp-4h]

    cmd = (const GfxCmdStretchPicRotateXY *)execState->cmd;
    iassert( gfxCmdBufSourceState.viewMode == VIEW_MODE_2D );
    RB_SetTessTechnique(cmd->material, TECHNIQUE_UNLIT);
    R_TrackPrims(&gfxCmdBufState, GFX_PRIM_STATS_HUD);
    RB_CheckTessOverflow(4, 6);
    vertCount = tess.vertexCount;
    indexCount = tess.indexCount;
    tess.vertexCount += 4;
    tess.indexCount += 6;
    tess.indices[indexCount] = vertCount + 3;
    tess.indices[indexCount + 1] = vertCount;
    tess.indices[indexCount + 2] = vertCount + 2;
    tess.indices[indexCount + 3] = vertCount + 2;
    tess.indices[indexCount + 4] = vertCount;
    tess.indices[indexCount + 5] = vertCount + 1;
    halfWidth = cmd->w * 0.5;
    halfHeight = cmd->h * 0.5;
    midX = cmd->x + halfWidth;
    midY = cmd->y + halfHeight;
    v9 = cmd->rotation * 0.01745329238474369;
    cosAngle = cos(v9);
    sinAngle = sin(v9);
    stepX = halfWidth * cosAngle;
    stepX_4 = halfWidth * sinAngle;
    stepY = -halfHeight * sinAngle;
    stepY_4 = halfHeight * cosAngle;
    y = midY - stepX_4 - stepY_4;
    x = midX - stepX - stepY;
    R_SetVertex2d(&tess.verts[vertCount], x, y, cmd->s0, cmd->t0, cmd->color.packed);
    v6 = midY + stepX_4 - stepY_4;
    v5 = midX + stepX - stepY;
    R_SetVertex2d(&tess.verts[vertCount + 1], v5, v6, cmd->s1, cmd->t0, cmd->color.packed);
    v4 = midY + stepX_4 + stepY_4;
    v3 = midX + stepX + stepY;
    R_SetVertex2d(&tess.verts[vertCount + 2], v3, v4, cmd->s1, cmd->t1, cmd->color.packed);
    v2 = midY - stepX_4 + stepY_4;
    v1 = midX - stepX + stepY;
    R_SetVertex2d(&tess.verts[vertCount + 3], v1, v2, cmd->s0, cmd->t1, cmd->color.packed);
    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_StretchPicRotateSTCmd(GfxRenderCommandExecState *execState)
{
    float v1; // [esp+14h] [ebp-64h]
    float v2; // [esp+18h] [ebp-60h]
    float y; // [esp+1Ch] [ebp-5Ch]
    float x; // [esp+20h] [ebp-58h]
    float v5; // [esp+30h] [ebp-48h]
    float cosAngle; // [esp+34h] [ebp-44h]
    int indexCount; // [esp+38h] [ebp-40h]
    float sinAngle; // [esp+3Ch] [ebp-3Ch]
    float texS; // [esp+40h] [ebp-38h]
    float texS_4; // [esp+44h] [ebp-34h]
    float texS_8; // [esp+48h] [ebp-30h]
    float texS_12; // [esp+4Ch] [ebp-2Ch]
    unsigned __int16 vertCount; // [esp+50h] [ebp-28h]
    float stepT; // [esp+54h] [ebp-24h]
    float stepT_4; // [esp+58h] [ebp-20h]
    const GfxCmdStretchPicRotateST *cmd; // [esp+5Ch] [ebp-1Ch]
    float stepS; // [esp+60h] [ebp-18h]
    float stepS_4; // [esp+64h] [ebp-14h]
    float texT; // [esp+68h] [ebp-10h]
    float texT_4; // [esp+6Ch] [ebp-Ch]
    float texT_8; // [esp+70h] [ebp-8h]
    float texT_12; // [esp+74h] [ebp-4h]

    cmd = (const GfxCmdStretchPicRotateST *)execState->cmd;
    iassert( gfxCmdBufSourceState.viewMode == VIEW_MODE_2D );
    RB_SetTessTechnique(cmd->material, TECHNIQUE_UNLIT);
    R_TrackPrims(&gfxCmdBufState, GFX_PRIM_STATS_HUD);
    vertCount = tess.vertexCount;
    indexCount = tess.indexCount;
    RB_CheckTessOverflow(4, 6);
    tess.vertexCount += 4;
    tess.indexCount += 6;
    tess.indices[indexCount] = vertCount + 3;
    tess.indices[indexCount + 1] = vertCount;
    tess.indices[indexCount + 2] = vertCount + 2;
    tess.indices[indexCount + 3] = vertCount + 2;
    tess.indices[indexCount + 4] = vertCount;
    tess.indices[indexCount + 5] = vertCount + 1;
    v5 = cmd->rotation * 0.01745329238474369;
    cosAngle = cos(v5);
    sinAngle = sin(v5);
    stepS = cmd->radiusST * cosAngle * cmd->scaleFinalS;
    stepS_4 = cmd->radiusST * sinAngle * cmd->scaleFinalT;
    stepT = -cmd->radiusST * sinAngle * cmd->scaleFinalS;
    stepT_4 = cmd->radiusST * cosAngle * cmd->scaleFinalT;
    texS = cmd->centerS - stepS - stepT;
    texT = cmd->centerT - stepS_4 - stepT_4;
    texS_4 = cmd->centerS + stepS - stepT;
    texT_4 = cmd->centerT + stepS_4 - stepT_4;
    texS_8 = cmd->centerS + stepS + stepT;
    texT_8 = cmd->centerT + stepS_4 + stepT_4;
    texS_12 = cmd->centerS - stepS + stepT;
    texT_12 = cmd->centerT - stepS_4 + stepT_4;
    R_SetVertex2d(&tess.verts[vertCount], cmd->x, cmd->y, texS, texT, cmd->color.packed);
    x = cmd->x + cmd->w;
    R_SetVertex2d(&tess.verts[vertCount + 1], x, cmd->y, texS_4, texT_4, cmd->color.packed);
    y = cmd->y + cmd->h;
    v2 = cmd->x + cmd->w;
    R_SetVertex2d(&tess.verts[vertCount + 2], v2, y, texS_8, texT_8, cmd->color.packed);
    v1 = cmd->y + cmd->h;
    R_SetVertex2d(&tess.verts[vertCount + 3], cmd->x, v1, texS_12, texT_12, cmd->color.packed);
    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_DrawQuadPicCmd(GfxRenderCommandExecState *execState)
{
    int indexCount; // [esp+18h] [ebp-Ch]
    unsigned __int16 vertCount; // [esp+1Ch] [ebp-8h]
    const GfxCmdDrawQuadPic *cmd; // [esp+20h] [ebp-4h]

    cmd = (const GfxCmdDrawQuadPic *)execState->cmd;
    iassert( gfxCmdBufSourceState.viewMode == VIEW_MODE_2D );
    RB_SetTessTechnique(cmd->material, TECHNIQUE_UNLIT);
    R_TrackPrims(&gfxCmdBufState, GFX_PRIM_STATS_HUD);
    RB_CheckTessOverflow(4, 6);
    vertCount = tess.vertexCount;
    indexCount = tess.indexCount;
    tess.vertexCount += 4;
    tess.indexCount += 6;
    tess.indices[indexCount] = vertCount + 3;
    tess.indices[indexCount + 1] = vertCount;
    tess.indices[indexCount + 2] = vertCount + 2;
    tess.indices[indexCount + 3] = vertCount + 2;
    tess.indices[indexCount + 4] = vertCount;
    tess.indices[indexCount + 5] = vertCount + 1;
    R_SetVertex2d(&tess.verts[vertCount], cmd->verts[0][0], cmd->verts[0][1], 0.0, 0.0, cmd->color.packed);
    R_SetVertex2d(&tess.verts[vertCount + 1], cmd->verts[1][0], cmd->verts[1][1], 1.0, 0.0, cmd->color.packed);
    R_SetVertex2d(&tess.verts[vertCount + 2], cmd->verts[2][0], cmd->verts[2][1], 1.0, 1.0, cmd->color.packed);
    R_SetVertex2d(&tess.verts[vertCount + 3], cmd->verts[3][0], cmd->verts[3][1], 0.0, 1.0, cmd->color.packed);
    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_DrawFullScreenColoredQuadCmd(GfxRenderCommandExecState *execState)
{
    GfxCmdDrawFullScreenColoredQuad *cmd = (GfxCmdDrawFullScreenColoredQuad *)execState->cmd;

    RB_DrawFullScreenColoredQuad(cmd->material, cmd->s0, cmd->t0, cmd->s1, cmd->t1, cmd->color);
    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_StretchRawCmd(GfxRenderCommandExecState *execState)
{
    GfxCmdStretchRaw *cmd = (GfxCmdStretchRaw *)execState->cmd;

    RB_StretchRaw(cmd->x, cmd->y, cmd->w, cmd->h, cmd->cols, cmd->rows, cmd->data);
    execState->cmd = (char*)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_StretchRaw(int x, int y, int w, int h, int cols, int rows, const unsigned __int8 *data)
{
    const char *v7; // eax
    int v8; // [esp+8h] [ebp-34h]
    _D3DLOCKED_RECT lockedRect; // [esp+10h] [ebp-2Ch] BYREF
    IDirect3DSurface9 *rawSurf; // [esp+18h] [ebp-24h] BYREF
    unsigned __int8 *dest; // [esp+1Ch] [ebp-20h]
    tagRECT dstRect; // [esp+20h] [ebp-1Ch] BYREF
    int colIndex; // [esp+30h] [ebp-Ch]
    int newline; // [esp+34h] [ebp-8h]
    int rowIndex; // [esp+38h] [ebp-4h]

    if (dx.device->CreateOffscreenPlainSurface(cols, rows, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &rawSurf, 0) >= 0)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("rawSurf->LockRect( &lockedRect, 0, 0x00002000L )\n");
            v8 = rawSurf->LockRect(&lockedRect, 0, 0x2000u);
            if (v8 < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v7 = R_ErrorDescription(v8);
                    Com_Error(
                        ERR_FATAL,
                        ".\\rb_backend.cpp (%i) rawSurf->LockRect( &lockedRect, 0, 0x00002000L ) failed: %s\n",
                        939,
                        v7);
                } while (alwaysfails);
            }
        } while (alwaysfails);
        dest = (unsigned __int8 *)lockedRect.pBits;
        newline = lockedRect.Pitch - 4 * cols;
        for (rowIndex = 0; rowIndex < rows; ++rowIndex)
        {
            for (colIndex = 0; colIndex < cols; ++colIndex)
            {
                Byte4CopyRgbaToVertexColor(data, dest);
                data += 4;
                dest += 4;
            }
            dest += newline;
        }
        rawSurf->UnlockRect();
        dstRect.left = x;
        dstRect.top = y;
        dstRect.right = w + x;
        dstRect.bottom = h + y;
        //((void(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, IDirect3DSurface9 *, _DWORD, IDirect3DSurface9 *, tagRECT *, int))dx.device->StretchRect)(
        //    dx.device,
        //    dx.device,
        //    rawSurf,
        //    0,
        //    gfxRenderTargets[1].surface.color,
        //    &dstRect,
        //    2);
        dx.device->StretchRect(rawSurf, 0, gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.color, &dstRect, D3DTEXF_LINEAR);

        rawSurf->Release();
    }
}

void __cdecl R_DrawSurfs(GfxCmdBufContext context, GfxCmdBufState *prepassState, const GfxDrawSurfListInfo *info)
{
    GfxViewport viewport; // [esp+30h] [ebp-30h] BYREF
    GfxCmdBufContext prepassContext; // [esp+40h] [ebp-20h]
    GfxDrawSurfListArgs listArgs; // [esp+48h] [ebp-18h] BYREF
    unsigned int processedDrawSurfCount; // [esp+58h] [ebp-8h]
    unsigned int drawSurfCount; // [esp+5Ch] [ebp-4h]

    PROF_SCOPED("R_DrawSurfs");

    iassert(context.source->cameraView == info->cameraView);
    context.state->origMaterial = 0;
    R_SetDrawSurfsShadowableLight(context.source, info);
    R_Set3D(context.source);
    if (context.source->viewportIsDirty)
    {
        R_GetViewport(context.source, &viewport);
        R_SetViewport(context.state, &viewport);
        if (prepassState)
            R_SetViewport(prepassState, &viewport);
        R_UpdateViewport(context.source, &viewport);
    }
    prepassContext.source = prepassState != 0 ? context.source : 0;
    prepassContext.state = prepassState;
    iassert(dx.d3d9 && dx.device);
    drawSurfCount = info->drawSurfCount;
    listArgs.context = context;
    listArgs.firstDrawSurfIndex = 0;
    listArgs.info = info;
    while (listArgs.firstDrawSurfIndex != drawSurfCount)
    {
        processedDrawSurfCount = R_RenderDrawSurfListMaterial(&listArgs, prepassContext);
        listArgs.firstDrawSurfIndex += processedDrawSurfCount;
    }
    g_viewStats->drawSurfCount += info->drawSurfCount;
    R_TessEnd(context, prepassContext);
    context.state->origMaterial = 0;
}


unsigned int(__cdecl *const rb_tessTable[13])(const GfxDrawSurfListArgs *, GfxCmdBufContext) =
{
  &R_TessTrianglesList,
  &R_TessTrianglesPreTessList,
  &R_TessStaticModelRigidDrawSurfList,
  &R_TessStaticModelPreTessList,
  &R_TessStaticModelCachedList,
  &R_TessStaticModelSkinnedDrawSurfList,
  &R_TessBModel,
  &R_TessXModelRigidDrawSurfList,
  &R_TessXModelRigidSkinnedDrawSurfList,
  &R_TessXModelSkinnedDrawSurfList,
  &R_TessCodeMeshList,
  &R_TessMarkMeshList,
  &R_TessParticleCloudList
}; // idb

unsigned int __cdecl R_RenderDrawSurfListMaterial(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    //GfxCmdBufSourceState *passPrepassContext; // [esp+4h] [ebp-28h]
    //GfxCmdBufState *passPrepassContext_4; // [esp+8h] [ebp-24h]
    GfxCmdBufContext passPrepassContext;
    GfxDrawSurf drawSurf; // [esp+Ch] [ebp-20h]
    unsigned int subListCount; // [esp+18h] [ebp-14h]
    const GfxDrawSurf *drawSurfList; // [esp+1Ch] [ebp-10h]
    unsigned int passIndex; // [esp+20h] [ebp-Ch]
    bool isPixelCostEnabled; // [esp+27h] [ebp-5h]
    unsigned int drawSurfCount; // [esp+28h] [ebp-4h]

    drawSurfCount = listArgs->info->drawSurfCount - listArgs->firstDrawSurfIndex;
    drawSurfList = &listArgs->info->drawSurfs[listArgs->firstDrawSurfIndex];
    drawSurf.fields = drawSurfList->fields;
    if (!R_SetupMaterial(listArgs->context, &prepassContext, listArgs->info, drawSurf))
        return R_SkipDrawSurfListMaterial(drawSurfList, drawSurfCount);
    isPixelCostEnabled = pixelCostMode != GFX_PIXEL_COST_MODE_OFF;
    if (pixelCostMode)
        R_PixelCost_BeginSurface(listArgs->context);
    if (prepassContext.state && prepassContext.state->technique->passCount != 1)
        MyAssertHandler(
            ".\\rb_backend.cpp",
            1013,
            0,
            "%s",
            "!prepassContext.state || (prepassContext.state->technique->passCount == 1)");
    passPrepassContext.source = prepassContext.source;
    subListCount = 0;
    for (passIndex = 0; passIndex < listArgs->context.state->technique->passCount; ++passIndex)
    {
        R_UpdateMaterialTime(listArgs->context.source, 0.0);
        R_SetupPass(listArgs->context, passIndex);
        if (passIndex || !prepassContext.state)
        {
            passPrepassContext.state = 0;
        }
        else
        {
            R_SetupPass(prepassContext, 0);
            passPrepassContext.state = prepassContext.state;
        }
        iassert(drawSurf.fields.surfType < ARRAY_COUNT(rb_tessTable));
        subListCount = rb_tessTable[drawSurf.fields.surfType](listArgs, passPrepassContext);
    }
    if (isPixelCostEnabled)
        R_PixelCost_EndSurface(listArgs->context);
    if (!subListCount || subListCount > drawSurfCount)
        MyAssertHandler(
            ".\\rb_backend.cpp",
            1049,
            0,
            "subListCount not in [1, drawSurfCount]\n\t%i not in [%i, %i]",
            subListCount,
            1,
            drawSurfCount);
    return subListCount;
}

void __cdecl R_TessEnd(GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    GfxDepthRangeType v2; // [esp+0h] [ebp-Ch]
    GfxDepthRangeType depthRangeType; // [esp+4h] [ebp-8h]

    if (prepassContext.state && context.source != prepassContext.source)
        MyAssertHandler(
            ".\\rb_backend.cpp",
            1059,
            0,
            "%s",
            "prepassContext.state == NULL || commonSource == prepassContext.source");
    context.source->objectPlacement = 0;
    R_ChangeDepthHackNearClip(context.source, 0);
    depthRangeType = (GfxDepthRangeType)((context.source->cameraView != 0) - 1);
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    if (prepassContext.state)
    {
        v2 = (GfxDepthRangeType)((prepassContext.source->cameraView != 0) - 1);
        if (v2 != prepassContext.state->depthRangeType)
            R_ChangeDepthRange(prepassContext.state, v2);
    }
}

void __cdecl RB_ClearScreenCmd(GfxRenderCommandExecState *execState)
{
    const GfxCmdClearScreen *cmd = (const GfxCmdClearScreen *)execState->cmd;

    if (tess.indexCount)
        RB_EndTessSurface();

    R_ClearScreen(
        gfxCmdBufState.prim.device,
        cmd->whichToClear,
        cmd->color,
        cmd->depth,
        cmd->stencil,
        0);

    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_SetGammaRamp(const GfxGammaRamp *gammaTable)
{
    int colorIndex; // [esp+0h] [ebp-60Ch]
    _D3DGAMMARAMP d3dGammaRamp; // [esp+4h] [ebp-608h] BYREF

    iassert( gammaTable != NULL );
    iassert( vidConfig.deviceSupportsGamma == true );
    iassert( dx.device != NULL );
    for (colorIndex = 0; colorIndex < 256; ++colorIndex)
    {
        d3dGammaRamp.red[colorIndex] = gammaTable->entries[colorIndex];
        d3dGammaRamp.green[colorIndex] = gammaTable->entries[colorIndex];
        d3dGammaRamp.blue[colorIndex] = gammaTable->entries[colorIndex];
    }
    dx.device->SetGammaRamp(dx.targetWindowIndex, 0, &d3dGammaRamp);
}

void __cdecl RB_SaveScreenCmd(GfxRenderCommandExecState *execState)
{
    const GfxCmdSaveScreen *cmd; // [esp+4h] [ebp-4h]

    cmd = (const GfxCmdSaveScreen *)execState->cmd;

    bcassert(cmd->screenTimerId, ARRAY_COUNT(rgp.savedScreenTimes));

    if (tess.indexCount)
        RB_EndTessSurface();
    R_Resolve(gfxCmdBufContext, gfxRenderTargets[R_RENDERTARGET_SAVED_SCREEN].image);
    rgp.savedScreenTimes[cmd->screenTimerId] = gfxCmdBufSourceState.sceneDef.time;

    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_SaveScreenSectionCmd(GfxRenderCommandExecState *execState)
{
    const GfxCmdSaveScreenSection *cmd; // [esp+14h] [ebp-4h]

    cmd = (const GfxCmdSaveScreenSection *)execState->cmd;

    bcassert(cmd->screenTimerId, ARRAY_COUNT(rgp.savedScreenTimes));

    if (tess.indexCount)
        RB_EndTessSurface();

    R_ResolveSection(gfxCmdBufContext, gfxRenderTargets[R_RENDERTARGET_SAVED_SCREEN].image);
    rgp.savedScreenTimes[cmd->screenTimerId] = gfxCmdBufSourceState.sceneDef.time;

    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl R_ResolveSection(GfxCmdBufContext context, GfxImage *image)
{
    iassert(image);
    if (!alwaysfails)
        MyAssertHandler(".\\rb_backend.cpp", 706, 0, "R_ResolveSection(): Not implemented on win32.");
}

void __cdecl RB_BlendSavedScreenBlurredCmd(GfxRenderCommandExecState *execState)
{
    float s1; // [esp+28h] [ebp-48h]
    float t1; // [esp+2Ch] [ebp-44h]
    float v3; // [esp+30h] [ebp-40h]
    float v4; // [esp+44h] [ebp-2Ch]
    float v5; // [esp+54h] [ebp-1Ch]
    float screenWidth; // [esp+58h] [ebp-18h]
    int frameTime; // [esp+5Ch] [ebp-14h]
    float screenHeight; // [esp+64h] [ebp-Ch]
    const GfxCmdBlendSavedScreenBlurred *cmd; // [esp+68h] [ebp-8h]
    float alpha; // [esp+6Ch] [ebp-4h]

    cmd = (const GfxCmdBlendSavedScreenBlurred *)execState->cmd;
    iassert( cmd->fadeMsec > 0 );
    if (cmd->screenTimerId >= 4u)
        MyAssertHandler(
            ".\\rb_backend.cpp",
            1280,
            0,
            "cmd->screenTimerId doesn't index ARRAY_COUNT( rgp.savedScreenTimes )\n\t%i not in [0, %i)",
            cmd->screenTimerId,
            4);
    if (tess.indexCount)
        RB_EndTessSurface();
    iassert( gfxCmdBufSourceState.viewMode == VIEW_MODE_2D );
    frameTime = gfxCmdBufSourceState.sceneDef.time - rgp.savedScreenTimes[cmd->screenTimerId];
    if (frameTime >= 0 && frameTime < cmd->fadeMsec)
    {
        v5 = (double)frameTime / (double)cmd->fadeMsec;
        v3 = pow(0.0099999998, v5);
        alpha = v3;
        if (v3 > 0.99f)
            alpha = 0.99f;
        screenWidth = (double)gfxCmdBufSourceState.renderTargetWidth * cmd->ds;
        screenHeight = (double)gfxCmdBufSourceState.renderTargetHeight * cmd->dt;
        R_SetCodeImageTexture(&gfxCmdBufSourceState, TEXTURE_SRC_CODE_FEEDBACK, gfxRenderTargets[R_RENDERTARGET_SAVED_SCREEN].image);
        t1 = cmd->t0 + cmd->dt;
        s1 = cmd->s0 + cmd->ds;
        RB_DrawStretchPic(
            rgp.shellShockBlurredMaterial,
            0.0,
            0.0,
            screenWidth,
            screenHeight,
            cmd->s0,
            cmd->t0,
            s1,
            t1,
            ((unsigned __int8)(int)(alpha * 255.0f) << 24) | 0xFFFFFF,
            GFX_PRIM_STATS_CODE);
    }
    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl R_SetCodeImageTexture(GfxCmdBufSourceState *source, MaterialTextureSource codeTexture, const GfxImage *image)
{
    iassert(source);
    iassert(codeTexture < TEXTURE_SRC_CODE_COUNT);

    source->input.codeImages[codeTexture] = image;
}

void __cdecl RB_BlendSavedScreenFlashedCmd(GfxRenderCommandExecState *execState)
{
    float s1; // [esp+28h] [ebp-50h]
    float t1; // [esp+2Ch] [ebp-4Ch]
    float v3; // [esp+40h] [ebp-38h]
    float v4; // [esp+54h] [ebp-24h]
    float screenWidth; // [esp+64h] [ebp-14h]
    float screenHeight; // [esp+70h] [ebp-8h]
    const GfxCmdBlendSavedScreenFlashed *cmd; // [esp+74h] [ebp-4h]

    cmd = (const GfxCmdBlendSavedScreenFlashed *)execState->cmd;
    if (tess.indexCount)
        RB_EndTessSurface();
    iassert( gfxCmdBufSourceState.viewMode == VIEW_MODE_2D );
    screenWidth = (double)gfxCmdBufSourceState.renderTargetWidth * cmd->ds;
    screenHeight = (double)gfxCmdBufSourceState.renderTargetHeight * cmd->dt;
    R_SetCodeImageTexture(&gfxCmdBufSourceState, TEXTURE_SRC_CODE_FEEDBACK, gfxRenderTargets[R_RENDERTARGET_SAVED_SCREEN].image);
    t1 = cmd->t0 + cmd->dt;
    s1 = cmd->s0 + cmd->ds;
    RB_DrawStretchPic(
        rgp.shellShockFlashedMaterial,
        0.0,
        0.0,
        screenWidth,
        screenHeight,
        cmd->s0,
        cmd->t0,
        s1,
        t1,
        ((unsigned __int8)(int)(cmd->intensityScreengrab * 255.0f) << 24)
        | (unsigned __int8)(int)(cmd->intensityWhiteout * 255.0f)
        | ((unsigned __int8)(int)(cmd->intensityWhiteout * 255.0f) << 8)
        | ((unsigned __int8)(int)(cmd->intensityWhiteout * 255.0f) << 16),
        GFX_PRIM_STATS_CODE);
    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_DrawPointsCmd(GfxRenderCommandExecState *execState)
{
    const GfxCmdDrawPoints *cmd; // [esp+4h] [ebp-4h]

    cmd = (const GfxCmdDrawPoints *)execState->cmd;

    if (cmd->dimensions == 2)
    {
        RB_DrawPoints2D(cmd);
    }
    else
    {
        iassert(cmd->dimensions == 3);
        RB_DrawPoints3D(cmd);
    }
    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_DrawPoints2D(const GfxCmdDrawPoints *cmd)
{
    float v1; // [esp+1Ch] [ebp-30h]
    float v2; // [esp+20h] [ebp-2Ch]
    float v3; // [esp+24h] [ebp-28h]
    float v4; // [esp+28h] [ebp-24h]
    float v5; // [esp+2Ch] [ebp-20h]
    float v6; // [esp+30h] [ebp-1Ch]
    float x; // [esp+34h] [ebp-18h]
    float y; // [esp+38h] [ebp-14h]
    float size; // [esp+40h] [ebp-Ch]
    int pointIndex; // [esp+44h] [ebp-8h]
    const GfxPointVertex *v; // [esp+48h] [ebp-4h]

    iassert( gfxCmdBufSourceState.viewMode == VIEW_MODE_2D );
    RB_SetTessTechnique(rgp.whiteMaterial, TECHNIQUE_UNLIT);
    R_TrackPrims(&gfxCmdBufState, GFX_PRIM_STATS_DEBUG);
    size = (double)cmd->size * 0.5;
    pointIndex = 0;
    v = cmd->verts;
    while (pointIndex < cmd->pointCount)
    {
        RB_CheckTessOverflow(4, 6);
        tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 1;
        tess.indices[tess.indexCount + 1] = tess.vertexCount;
        tess.indices[tess.indexCount + 2] = LOWORD(tess.vertexCount) + 2;
        tess.indices[tess.indexCount + 3] = LOWORD(tess.vertexCount) + 2;
        tess.indices[tess.indexCount + 4] = tess.vertexCount;
        tess.indices[tess.indexCount + 5] = LOWORD(tess.vertexCount) + 3;
        tess.indexCount += 6;
        y = v->xyz[1] - size;
        x = v->xyz[0] - size;
        R_SetVertex4d(&tess.verts[tess.vertexCount], x, y, v->xyz[2], 1.0, 0.0, 0.0, v->color);
        v6 = v->xyz[1] + size;
        v5 = v->xyz[0] - size;
        R_SetVertex4d(&tess.verts[tess.vertexCount + 1], v5, v6, v->xyz[2], 1.0, 0.0, 1.0, v->color);
        v4 = v->xyz[1] + size;
        v3 = v->xyz[0] + size;
        R_SetVertex4d(&tess.verts[tess.vertexCount + 2], v3, v4, v->xyz[2], 1.0, 1.0, 1.0, v->color);
        v2 = v->xyz[1] - size;
        v1 = v->xyz[0] + size;
        R_SetVertex4d(&tess.verts[tess.vertexCount + 3], v1, v2, v->xyz[2], 1.0, 1.0, 0.0, v->color);
        tess.vertexCount += 4;
        ++pointIndex;
        ++v;
    }
}

void __cdecl R_SetVertex4d(
    GfxVertex *vert,
    float x,
    float y,
    float z,
    float w,
    float s,
    float t,
    const unsigned __int8 *color)
{
    vert->xyzw[0] = x;
    vert->xyzw[1] = y;
    vert->xyzw[2] = z;
    vert->xyzw[3] = w;
    vert->normal.packed = 1073643391;
    vert->color.packed = *(unsigned int *)color;
    vert->texCoord[0] = s;
    vert->texCoord[1] = t;
}

void __cdecl RB_DrawPoints3D(const GfxCmdDrawPoints *cmd)
{
    float v1; // [esp+24h] [ebp-4Ch]
    float v2; // [esp+30h] [ebp-40h]
    float x; // [esp+34h] [ebp-3Ch]
    float y; // [esp+38h] [ebp-38h]
    const float *transform; // [esp+44h] [ebp-2Ch]
    float xyz; // [esp+48h] [ebp-28h]
    float xyz_4; // [esp+4Ch] [ebp-24h]
    float xyz_8; // [esp+50h] [ebp-20h]
    float xyz_8a; // [esp+50h] [ebp-20h]
    float xyz_12; // [esp+54h] [ebp-1Ch]
    float invWidth; // [esp+58h] [ebp-18h]
    float invHeight; // [esp+5Ch] [ebp-14h]
    float offset; // [esp+60h] [ebp-10h]
    float offset_4; // [esp+64h] [ebp-Ch]
    int pointIndex; // [esp+68h] [ebp-8h]
    const GfxPointVertex *v; // [esp+6Ch] [ebp-4h]

    RB_SetTessTechnique(rgp.pointMaterial, TECHNIQUE_UNLIT);
    R_TrackPrims(&gfxCmdBufState, GFX_PRIM_STATS_DEBUG);
    RB_SetIdentity();
    transform = (const float *)&gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix;
    invWidth = (double)cmd->size * 1.0 / (double)gfxCmdBufSourceState.renderTargetWidth;
    invHeight = (double)cmd->size * 1.0 / (double)gfxCmdBufSourceState.renderTargetHeight;
    pointIndex = 0;
    v = cmd->verts;
    while (pointIndex < cmd->pointCount)
    {
        xyz = v->xyz[0] * *transform + v->xyz[1] * transform[4] + v->xyz[2] * transform[8] + transform[12];
        xyz_4 = v->xyz[0] * transform[1] + v->xyz[1] * transform[5] + v->xyz[2] * transform[9] + transform[13];
        xyz_8 = v->xyz[0] * transform[2] + v->xyz[1] * transform[6] + v->xyz[2] * transform[10] + transform[14];
        xyz_12 = v->xyz[0] * transform[3] + v->xyz[1] * transform[7] + v->xyz[2] * transform[11] + transform[15];
        offset = invWidth * xyz_12;
        offset_4 = invHeight * xyz_12;
        xyz_8a = xyz_8 - xyz_12 * EQUAL_EPSILON;
        RB_CheckTessOverflow(4, 6);
        tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 3;
        tess.indices[tess.indexCount + 1] = tess.vertexCount;
        tess.indices[tess.indexCount + 2] = LOWORD(tess.vertexCount) + 2;
        tess.indices[tess.indexCount + 3] = LOWORD(tess.vertexCount) + 2;
        tess.indices[tess.indexCount + 4] = tess.vertexCount;
        tess.indices[tess.indexCount + 5] = LOWORD(tess.vertexCount) + 1;
        tess.indexCount += 6;
        y = xyz_4 - offset_4;
        x = xyz - offset;
        R_SetVertex4d(&tess.verts[tess.vertexCount], x, y, xyz_8a, xyz_12, 0.0, 0.0, v->color);
        v2 = xyz_4 + offset_4;
        R_SetVertex4d(&tess.verts[tess.vertexCount + 1], x, v2, xyz_8a, xyz_12, 0.0, 1.0, v->color);
        v1 = xyz + offset;
        R_SetVertex4d(&tess.verts[tess.vertexCount + 2], v1, v2, xyz_8a, xyz_12, 1.0, 1.0, v->color);
        R_SetVertex4d(&tess.verts[tess.vertexCount + 3], v1, y, xyz_8a, xyz_12, 1.0, 0.0, v->color);
        tess.vertexCount += 4;
        ++pointIndex;
        ++v;
    }
    RB_EndTessSurface();
}

void __cdecl RB_DrawLines2D(int count, int width, const GfxPointVertex *verts)
{
    float v3; // [esp+18h] [ebp-34h]
    float v4; // [esp+1Ch] [ebp-30h]
    float v5; // [esp+20h] [ebp-2Ch]
    float v6; // [esp+24h] [ebp-28h]
    float v7; // [esp+28h] [ebp-24h]
    float v8; // [esp+2Ch] [ebp-20h]
    float x; // [esp+30h] [ebp-1Ch]
    float y; // [esp+34h] [ebp-18h]
    float delta[2]; // [esp+38h] [ebp-14h] BYREF
    int lineIndex; // [esp+40h] [ebp-Ch]
    const GfxPointVertex *v[2]; // [esp+44h] [ebp-8h]

    iassert( (count > 0) );
    iassert( gfxCmdBufSourceState.viewMode == VIEW_MODE_2D );
    RB_SetTessTechnique(rgp.whiteMaterial, TECHNIQUE_UNLIT);
    R_TrackPrims(&gfxCmdBufState, GFX_PRIM_STATS_DEBUG);
    for (lineIndex = 0; lineIndex < count; ++lineIndex)
    {
        v[0] = &verts[2 * lineIndex];
        v[1] = &verts[2 * lineIndex + 1];
        delta[0] = v[1]->xyz[1] - v[0]->xyz[1];
        delta[1] = v[0]->xyz[0] - v[1]->xyz[0];
        Vec2Normalize(delta);
        delta[0] = delta[0] * 0.5;
        delta[1] = delta[1] * 0.5;
        RB_CheckTessOverflow(4, 6);
        tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 1;
        tess.indices[tess.indexCount + 1] = tess.vertexCount;
        tess.indices[tess.indexCount + 2] = LOWORD(tess.vertexCount) + 2;
        tess.indices[tess.indexCount + 3] = LOWORD(tess.vertexCount) + 2;
        tess.indices[tess.indexCount + 4] = tess.vertexCount;
        tess.indices[tess.indexCount + 5] = LOWORD(tess.vertexCount) + 3;
        tess.indexCount += 6;
        y = v[0]->xyz[1] - delta[1];
        x = v[0]->xyz[0] - delta[0];
        R_SetVertex3d(&tess.verts[tess.vertexCount], x, y, v[0]->xyz[2], 0.0, 0.0, v[0]->color);
        v8 = v[1]->xyz[1] - delta[1];
        v7 = v[1]->xyz[0] - delta[0];
        R_SetVertex3d(&tess.verts[tess.vertexCount + 1], v7, v8, v[1]->xyz[2], 0.0, 1.0, v[1]->color);
        v6 = v[1]->xyz[1] + delta[1];
        v5 = v[1]->xyz[0] + delta[0];
        R_SetVertex3d(&tess.verts[tess.vertexCount + 2], v5, v6, v[1]->xyz[2], 1.0, 1.0, v[1]->color);
        v4 = v[0]->xyz[1] + delta[1];
        v3 = v[0]->xyz[0] + delta[0];
        R_SetVertex3d(&tess.verts[tess.vertexCount + 3], v3, v4, v[0]->xyz[2], 1.0, 0.0, v[0]->color);
        tess.vertexCount += 4;
    }
}

void __cdecl R_SetVertex3d(GfxVertex *vert, float x, float y, float z, float s, float t, const unsigned __int8 *color)
{
    vert->xyzw[0] = x;
    vert->xyzw[1] = y;
    vert->xyzw[2] = z;
    vert->xyzw[3] = 1.0;
    vert->normal.packed = 0x3FFE7F7F;
    vert->color.packed = *(unsigned int *)color;
    vert->texCoord[0] = s;
    vert->texCoord[1] = t;
}

void __cdecl RB_DrawLines3D(int count, int width, const GfxPointVertex *verts, bool depthTest)
{
    float v4; // [esp+1Ch] [ebp-74h]
    float v5; // [esp+20h] [ebp-70h]
    float v6; // [esp+24h] [ebp-6Ch]
    float v7; // [esp+28h] [ebp-68h]
    float v8; // [esp+2Ch] [ebp-64h]
    float v9; // [esp+30h] [ebp-60h]
    float x; // [esp+34h] [ebp-5Ch]
    float y; // [esp+38h] [ebp-58h]
    const float *transform; // [esp+40h] [ebp-50h]
    float delta[2]; // [esp+44h] [ebp-4Ch] BYREF
    float xyz[2][4]; // [esp+4Ch] [ebp-44h]
    float invWidth; // [esp+6Ch] [ebp-24h]
    float invHeight; // [esp+70h] [ebp-20h]
    float offset[2][2]; // [esp+74h] [ebp-1Ch]
    int lineIndex; // [esp+84h] [ebp-Ch]
    const GfxPointVertex *v[2]; // [esp+88h] [ebp-8h]

    if (depthTest)
        RB_SetTessTechnique(rgp.lineMaterial, TECHNIQUE_UNLIT);
    else
        RB_SetTessTechnique(rgp.lineMaterialNoDepth, TECHNIQUE_UNLIT);
    R_TrackPrims(&gfxCmdBufState, GFX_PRIM_STATS_DEBUG);
    RB_SetIdentity();
    transform = (const float *)&gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix;
    invWidth = (double)width / (double)gfxCmdBufSourceState.renderTargetWidth;
    invHeight = (double)width / (double)gfxCmdBufSourceState.renderTargetHeight;
    for (lineIndex = 0; lineIndex < count; ++lineIndex)
    {
        v[0] = &verts[2 * lineIndex];
        xyz[0][0] = v[0]->xyz[0] * *transform + v[0]->xyz[1] * transform[4] + v[0]->xyz[2] * transform[8] + transform[12];
        xyz[0][1] = v[0]->xyz[0] * transform[1] + v[0]->xyz[1] * transform[5] + v[0]->xyz[2] * transform[9] + transform[13];
        xyz[0][2] = v[0]->xyz[0] * transform[2] + v[0]->xyz[1] * transform[6] + v[0]->xyz[2] * transform[10] + transform[14];
        xyz[0][3] = v[0]->xyz[0] * transform[3] + v[0]->xyz[1] * transform[7] + v[0]->xyz[2] * transform[11] + transform[15];
        v[1] = &verts[2 * lineIndex + 1];
        xyz[1][0] = v[1]->xyz[0] * *transform + v[1]->xyz[1] * transform[4] + v[1]->xyz[2] * transform[8] + transform[12];
        xyz[1][1] = v[1]->xyz[0] * transform[1] + v[1]->xyz[1] * transform[5] + v[1]->xyz[2] * transform[9] + transform[13];
        xyz[1][2] = v[1]->xyz[0] * transform[2] + v[1]->xyz[1] * transform[6] + v[1]->xyz[2] * transform[10] + transform[14];
        xyz[1][3] = v[1]->xyz[0] * transform[3] + v[1]->xyz[1] * transform[7] + v[1]->xyz[2] * transform[11] + transform[15];
        delta[0] = xyz[1][1] * xyz[0][3] - xyz[0][1] * xyz[1][3];
        delta[1] = xyz[0][0] * xyz[1][3] - xyz[1][0] * xyz[0][3];
        Vec2Normalize(delta);
        delta[0] = delta[0] * invWidth;
        delta[1] = delta[1] * invHeight;
        offset[0][0] = xyz[0][3] * delta[0];
        offset[0][1] = xyz[0][3] * delta[1];
        offset[1][0] = xyz[1][3] * delta[0];
        offset[1][1] = xyz[1][3] * delta[1];
        RB_CheckTessOverflow(4, 6);
        tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 3;
        tess.indices[tess.indexCount + 1] = tess.vertexCount;
        tess.indices[tess.indexCount + 2] = LOWORD(tess.vertexCount) + 2;
        tess.indices[tess.indexCount + 3] = LOWORD(tess.vertexCount) + 2;
        tess.indices[tess.indexCount + 4] = tess.vertexCount;
        tess.indices[tess.indexCount + 5] = LOWORD(tess.vertexCount) + 1;
        tess.indexCount += 6;
        y = xyz[0][1] - offset[0][1];
        x = xyz[0][0] - offset[0][0];
        R_SetVertex4d(&tess.verts[tess.vertexCount], x, y, xyz[0][2], xyz[0][3], 0.0, 0.0, v[0]->color);
        v9 = xyz[1][1] - offset[1][1];
        v8 = xyz[1][0] - offset[1][0];
        R_SetVertex4d(&tess.verts[tess.vertexCount + 1], v8, v9, xyz[1][2], xyz[1][3], 0.0, 1.0, v[1]->color);
        v7 = xyz[1][1] + offset[1][1];
        v6 = xyz[1][0] + offset[1][0];
        R_SetVertex4d(&tess.verts[tess.vertexCount + 2], v6, v7, xyz[1][2], xyz[1][3], 1.0, 1.0, v[1]->color);
        v5 = xyz[0][1] + offset[0][1];
        v4 = xyz[0][0] + offset[0][0];
        R_SetVertex4d(&tess.verts[tess.vertexCount + 3], v4, v5, xyz[0][2], xyz[0][3], 1.0, 0.0, v[0]->color);
        tess.vertexCount += 4;
    }
    RB_EndTessSurface();
}

void __cdecl RB_DrawLinesCmd(GfxRenderCommandExecState *execState)
{
    const GfxCmdDrawLines *cmd; // [esp+4h] [ebp-4h]

    cmd = (const GfxCmdDrawLines *)execState->cmd;

    if (cmd->dimensions == 2)
    {
        RB_DrawLines2D(cmd->lineCount, cmd->width, cmd->verts);
    }
    else
    {
        iassert(cmd->dimensions == 3);
        RB_DrawLines3D(cmd->lineCount, cmd->width, cmd->verts, 1);
    }
    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_DrawTrianglesCmd(GfxRenderCommandExecState *execState)
{
    int stOffset; // [esp+4h] [ebp-3Ch]
    int normalOffset; // [esp+2Ch] [ebp-14h]
    int normalSize; // [esp+30h] [ebp-10h]
    int indexOffset;
    int stSize;
    int xyzwOffset;
    int xyzwSize;
    int colorOffset;
    int colorSize;

    GfxCmdDrawTriangles *cmd = (GfxCmdDrawTriangles *)execState->cmd;

    xyzwOffset = 16;
    xyzwSize = 16 * cmd->vertexCount;

    normalOffset = xyzwOffset + xyzwSize;
    normalSize = 12 * cmd->vertexCount;

    colorOffset = normalOffset + normalSize;
    colorSize = 4 * cmd->vertexCount;

    stOffset = colorOffset + colorSize;
    stSize = cmd->vertexCount * 8;

    indexOffset = stOffset + stSize;

    RB_DrawTriangles_Internal(
        cmd->material,
        cmd->techType,
        cmd->indexCount,
        (const unsigned short *)((char *)cmd + indexOffset),
        cmd->vertexCount,
        (const float (*)[4])((char *)cmd + xyzwOffset),
        (const float(*)[3])((char *)cmd + normalOffset),
        (const GfxColor *)((char *)cmd + colorOffset),
        (const float (*)[2])((char *)cmd + stOffset)
    );

    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_DrawTriangles_Internal(
    const Material *material,
    MaterialTechniqueType techType,
    __int16 indexCount,
    const unsigned __int16 *indices,
    __int16 vertexCount,
    const float (*xyzw)[4],
    const float (*normal)[3],
    const GfxColor *color,
    const float (*st)[2])
{
    int index; // [esp+28h] [ebp-4h]
    int indexa; // [esp+28h] [ebp-4h]

    if (tess.indexCount)
        RB_EndTessSurface();
    R_Set3D(&gfxCmdBufSourceState);
    RB_SetTessTechnique(material, techType);
    R_TrackPrims(&gfxCmdBufState, GFX_PRIM_STATS_DEBUG);
    RB_CheckTessOverflow(vertexCount, indexCount);
    for (index = 0; index < indexCount; ++index)
        tess.indices[index + tess.indexCount] = LOWORD(tess.vertexCount) + indices[index];
    for (indexa = 0; indexa < vertexCount; ++indexa)
        R_SetVertex4dWithNormal(
            &tess.verts[indexa + tess.vertexCount],
            (*xyzw)[4 * indexa],
            (*xyzw)[4 * indexa + 1],
            (*xyzw)[4 * indexa + 2],
            (*xyzw)[4 * indexa + 3],
            (*normal)[3 * indexa],
            (*normal)[3 * indexa + 1],
            (*normal)[3 * indexa + 2],
            (*st)[2 * indexa],
            (*st)[2 * indexa + 1],
            (const unsigned __int8 *)&color[indexa]);
    tess.indexCount += indexCount;
    tess.vertexCount += vertexCount;
    RB_EndTessSurface();
}

void __cdecl RB_DrawProfileCmd(GfxRenderCommandExecState *execState)
{
    PROF_SCOPED("RB_DrawProfileCmd");

    GfxCmdDrawProfile *cmd = (GfxCmdDrawProfile *)execState->cmd;
    RB_DrawProfile();
    RB_DrawProfileScript();

    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_SetMaterialColorCmd(GfxRenderCommandExecState *execState)
{
    const GfxCmdSetMaterialColor *cmd; // [esp+8h] [ebp-4h]

    cmd = (const GfxCmdSetMaterialColor *)execState->cmd;
    if (tess.indexCount)
        RB_EndTessSurface();
    R_SetCodeConstantFromVec4(&gfxCmdBufSourceState, CONST_SRC_CODE_MATERIAL_COLOR, (float*)cmd->color);

    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_SetViewportCmd(GfxRenderCommandExecState *execState)
{
    const GfxCmdSetViewport *cmd = (const GfxCmdSetViewport *)execState->cmd;

    if (tess.indexCount)
        RB_EndTessSurface();

    R_SetViewportStruct(&gfxCmdBufSourceState, &cmd->viewport);

    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

GfxColor color_table[8] =
{
  { 4278190080u },
  { 4284243199u },
  { 4278255360u },
  { 4278255615u },
  { 4294901760u },
  { 4294967040u },
  { 4294925567u },
  { 4294967295u }
}; // weak
void __cdecl RB_LookupColor(unsigned __int8 c, GfxColor *color)
{
    GfxColor *p_color_axis; // [esp+8h] [ebp-Ch]
    GfxColor *p_color_allies; // [esp+Ch] [ebp-8h]
    unsigned int index; // [esp+10h] [ebp-4h]

    index = ColorIndex(c);
    if (index >= 8)
    {
        if (c == 56)
        {
            if (rg.team == 2)
                p_color_allies = &rg.color_allies;
            else
                p_color_allies = &rg.color_axis;
            color->packed = p_color_allies->packed;
        }
        else if (c == 57)
        {
            if (rg.team == 2)
                p_color_axis = &rg.color_axis;
            else
                p_color_axis = &rg.color_allies;
            color->packed = p_color_axis->packed;
        }
        else
        {
            color->packed = -1;
        }
    }
    else
    {
        color->packed = (unsigned int)color_table[index];
    }
}

void __cdecl RB_DrawText(const char *text, Font_s *font, float x, float y, GfxColor color)
{
    DrawText2D(text, x, y, font, 1.0, 1.0, 0.0, 1.0, color, 0x7FFFFFFF, 0, 0, 0, 0.0, color, 0, 0, 0, 0, 0, 0);
}

const unsigned __int8 MY_ALTCOLOR_TWO[4] = { 0xE6, 0xFF, 0xE6, 0xDC };
const float MY_OFFSETS_0[4][2] =
{
    { -1.0f, 1.0f },
    { -1.0f, 1.0f },
    { 1.0f, -1.0f },
    { 1.0f, 1.0f }
};
void __cdecl DrawText2D(
    const char *text,
    float x,
    float y,
    Font_s *font,
    float xScale,
    float yScale,
    float sinAngle,
    float cosAngle,
    GfxColor color,
    int maxLength,
    __int16 renderFlags,
    int cursorPos,
    char cursorLetter,
    float padding,
    GfxColor glowForcedColor,
    int fxBirthTime,
    int fxLetterTime,
    int fxDecayStartTime,
    int fxDecayDuration,
    const Material *fxMaterial,
    const Material *fxMaterialGlow)
{
    int v21; // esi
    const char *v22; // eax
    double v23; // st7
    float v24; // [esp+3Ch] [ebp-1B4h]
    float v25; // [esp+44h] [ebp-1ACh]
    float v26; // [esp+5Ch] [ebp-194h]
    float v27; // [esp+64h] [ebp-18Ch]
    float v28; // [esp+74h] [ebp-17Ch]
    float h; // [esp+7Ch] [ebp-174h]
    float v30; // [esp+9Ch] [ebp-154h]
    float v31; // [esp+A0h] [ebp-150h]
    float v32; // [esp+A4h] [ebp-14Ch]
    float v33; // [esp+A8h] [ebp-148h]
    GfxColor v34; // [esp+ACh] [ebp-144h]
    float v35; // [esp+C0h] [ebp-130h]
    float v36; // [esp+C4h] [ebp-12Ch]
    float v37; // [esp+C8h] [ebp-128h]
    float v38; // [esp+CCh] [ebp-124h]
    GfxColor v39; // [esp+D0h] [ebp-120h]
    float v40; // [esp+E4h] [ebp-10Ch]
    float v41; // [esp+E8h] [ebp-108h]
    float w; // [esp+ECh] [ebp-104h]
    float v43; // [esp+F0h] [ebp-100h]
    float resizeOffsY; // [esp+148h] [ebp-A8h]
    int offIdx; // [esp+14Ch] [ebp-A4h]
    float resizeOffsX; // [esp+150h] [ebp-A0h]
    int ofs; // [esp+154h] [ebp-9Ch]
    const Glyph *glyphOriginal; // [esp+158h] [ebp-98h]
    int tempSeed; // [esp+15Ch] [ebp-94h] BYREF
    float iconWidth; // [esp+160h] [ebp-90h]
    GfxColor lookupColor; // [esp+164h] [ebp-8Ch] BYREF
    const unsigned __int8 *altColorTwo; // [esp+168h] [ebp-88h]
    GfxColor finalColor; // [esp+16Ch] [ebp-84h] BYREF
    bool drawExtraFxChar; // [esp+173h] [ebp-7Dh] BYREF
    const Glyph *glyph; // [esp+174h] [ebp-7Ch]
    float yAdj; // [esp+178h] [ebp-78h]
    float decayOffset; // [esp+17Ch] [ebp-74h]
    float xAdj; // [esp+180h] [ebp-70h]
    bool skipDrawing; // [esp+187h] [ebp-69h] BYREF
    unsigned int letter; // [esp+188h] [ebp-68h] BYREF
    int extraFxChar; // [esp+18Ch] [ebp-64h]
    float deltaX; // [esp+190h] [ebp-60h]
    unsigned int origLetter; // [esp+194h] [ebp-5Ch]
    unsigned __int8 fadeAlpha; // [esp+19Bh] [ebp-55h] BYREF
    float yRot; // [esp+19Ch] [ebp-54h] BYREF
    int passRandSeed; // [esp+1A0h] [ebp-50h] BYREF
    int maxLengthRemaining; // [esp+1A4h] [ebp-4Ch]
    float xRot; // [esp+1A8h] [ebp-48h] BYREF
    bool subtitleAllowGlow; // [esp+1AFh] [ebp-41h]
    GfxColor currentColor; // [esp+1B0h] [ebp-40h]
    const char *curText; // [esp+1B4h] [ebp-3Ch] BYREF
    int count; // [esp+1B8h] [ebp-38h]
    int passIdx; // [esp+1BCh] [ebp-34h]
    GfxColor dropShadowColor; // [esp+1C0h] [ebp-30h]
    const Material *material; // [esp+1C4h] [ebp-2Ch]
    bool drawRandomCharAtEnd; // [esp+1CBh] [ebp-25h] BYREF
    int randSeed; // [esp+1CCh] [ebp-24h] BYREF
    float startX; // [esp+1D0h] [ebp-20h]
    int decayTimeElapsed; // [esp+1D4h] [ebp-1Ch] BYREF
    const Material *glowMaterial; // [esp+1D8h] [ebp-18h]
    float monospaceWidth; // [esp+1E0h] [ebp-10h]
    bool decaying; // [esp+1E7h] [ebp-9h] BYREF
    float startY; // [esp+1E8h] [ebp-8h]
    int passCount; // [esp+1ECh] [ebp-4h]
    float xa; // [esp+1FCh] [ebp+Ch]
    float ya; // [esp+200h] [ebp+10h]

    iassert( text );
    iassert( font );
    dropShadowColor.packed = 0;
    dropShadowColor.array[3] = color.array[3];
    randSeed = 1;
    drawRandomCharAtEnd = 0;
    monospaceWidth = GetMonospaceWidth(font, renderFlags);
    glowMaterial = 0;
    material = Material_FromHandle(font->material);
    iassert( material );
    if ((renderFlags & 0x40) != 0 && (!fxMaterial || !fxMaterial->techniqueSet))
        MyAssertHandler(
            ".\\rb_backend.cpp",
            2143,
            0,
            "%s",
            "!(renderFlags & TEXT_RENDERFLAG_FX_DECODE) || (fxMaterial && fxMaterial->techniqueSet)");
    if ((renderFlags & 0x40) != 0 && (!fxMaterialGlow || !fxMaterialGlow->techniqueSet))
        MyAssertHandler(
            ".\\rb_backend.cpp",
            2144,
            0,
            "%s",
            "!(renderFlags & TEXT_RENDERFLAG_FX_DECODE) || (fxMaterialGlow && fxMaterialGlow->techniqueSet)");
    if (SetupPulseFXVars(
        text,
        maxLength,
        renderFlags,
        fxBirthTime,
        fxLetterTime,
        fxDecayStartTime,
        fxDecayDuration,
        &drawRandomCharAtEnd,
        &randSeed,
        &maxLength,
        &decaying,
        &decayTimeElapsed))
    {
        passCount = 1;
        if ((renderFlags & 0x10) != 0)
        {
            glowMaterial = Material_FromHandle(font->glowMaterial);
            iassert( glowMaterial );
            ++passCount;
        }
        if ((renderFlags & 0x40) != 0)
        {
            iassert( fxMaterialGlow );
            iassert( fxMaterial );
        }
        startX = x - xScale * 0.5;
        startY = y - yScale * 0.5;
        for (passIdx = 0; passIdx < passCount; ++passIdx)
        {
            maxLengthRemaining = maxLength;
            passRandSeed = randSeed;
            currentColor.packed = color.packed;
            xa = startX;
            ya = startY;
            subtitleAllowGlow = 0;
            count = 0;
            curText = text;
            while (*curText && maxLengthRemaining)
            {
                letter = SEH_ReadCharFromString(&curText, 0);
                skipDrawing = 0;
                fadeAlpha = 0;
                drawExtraFxChar = 0;
                extraFxChar = 0;
                if (letter == 94 && curText && *curText != 94 && *curText >= 48 && *curText <= 57)
                {
                    subtitleAllowGlow = 0;
                    v21 = ColorIndex(*curText);
                    if (v21 == ColorIndex(0x37u))
                    {
                        currentColor.packed = color.packed;
                    }
                    else if ((renderFlags & 0x100) != 0 && ColorIndex(*curText) == 2)
                    {
                        altColorTwo = MY_ALTCOLOR_TWO;
                        currentColor.array[3] = ModulateByteColors(MY_ALTCOLOR_TWO[3], color.array[3]);
                        currentColor.array[0] = altColorTwo[2];
                        currentColor.array[1] = altColorTwo[1];
                        currentColor.array[2] = *altColorTwo;
                        subtitleAllowGlow = 1;
                    }
                    else
                    {
                        RB_LookupColor(*curText, &lookupColor);
                        currentColor.array[3] = color.array[3];
                        currentColor.array[0] = lookupColor.array[2];
                        currentColor.array[1] = lookupColor.array[1];
                        currentColor.array[2] = lookupColor.array[0];
                    }
                    ++curText;
                    count += 2;
                }
                else
                {
                    if (drawRandomCharAtEnd && maxLengthRemaining == 1)
                    {
                        letter = R_FontGetRandomLetter(font, passRandSeed);
                        fadeAlpha = -64;
                        if ((int)RandWithSeed(&passRandSeed) % 2)
                        {
                            drawExtraFxChar = 1;
                            letter = 79;
                        }
                    }
                    if (letter == 94 && (*curText == 1 || *curText == 2))
                    {
                        RotateXY(cosAngle, sinAngle, startX, startY, xa, ya, &xRot, &yRot);
                        iconWidth = RB_DrawHudIcon(
                            curText,
                            xRot,
                            yRot,
                            sinAngle,
                            cosAngle,
                            font,
                            xScale,
                            yScale,
                            currentColor.packed);
                        if (iconWidth <= 0.0)
                        {
                            v22 = va("Invalid hud icon.  Text: \"%s\"", text);
                            MyAssertHandler(".\\rb_backend.cpp", 2266, 0, "%s\n\t%s", "iconWidth > 0", v22);
                        }
                        xa = xa + iconWidth;
                        if ((renderFlags & 0x80) != 0)
                            xa = padding * xScale + xa;
                        curText += 7;
                        ++count;
                        --maxLengthRemaining;
                    }
                    else if (letter == 10)
                    {
                        xa = startX;
                        ya = (double)font->pixelHeight * yScale + ya;
                    }
                    else if (letter == 13)
                    {
                        xa = startX;
                    }
                    else
                    {
                        origLetter = letter;
                        if (decaying)
                            GetDecayingLetterInfo(
                                letter,
                                font,
                                &passRandSeed,
                                decayTimeElapsed,
                                fxBirthTime,
                                fxDecayDuration,
                                currentColor.array[3],
                                &skipDrawing,
                                &fadeAlpha,
                                &letter,
                                &drawExtraFxChar);
                        if (drawExtraFxChar)
                        {
                            tempSeed = passRandSeed;
                            extraFxChar = RandWithSeed(&tempSeed);
                        }
                        glyph = R_GetCharacterGlyph(font, letter);
                        if (letter == origLetter)
                        {
                            decayOffset = 0.0;
                            deltaX = (float)glyph->dx;
                        }
                        else
                        {
                            glyphOriginal = R_GetCharacterGlyph(font, origLetter);
                            decayOffset = (double)glyphOriginal->pixelWidth * 0.5 - (double)glyph->pixelWidth * 0.5;
                            deltaX = (float)glyphOriginal->dx;
                        }
                        xAdj = ((double)glyph->x0 + decayOffset) * xScale;
                        yAdj = (double)glyph->y0 * yScale;
                        finalColor.packed = LongNoSwap(currentColor.packed);
                        if (decaying || drawRandomCharAtEnd && maxLengthRemaining == 1)
                            finalColor.array[3] = ModulateByteColors(finalColor.array[3], fadeAlpha);
                        if (!skipDrawing)
                        {
                            if (passIdx)
                            {
                                if (passIdx == 1 && ((renderFlags & 0x100) == 0 || subtitleAllowGlow))
                                {
                                    GlowColor(&finalColor, finalColor, glowForcedColor, renderFlags);
                                    resizeOffsX = (double)glyph->pixelWidth * -0.75 * 0.5 * xScale;
                                    resizeOffsY = (double)glyph->pixelHeight * -0.125 * 0.5 * yScale;
                                    for (offIdx = 0; offIdx < 4; ++offIdx)
                                    {
                                        xRot = xa + xAdj + resizeOffsX + (float)MY_OFFSETS_0[offIdx][0] * 2.0 * xScale;
                                        yRot = ya + yAdj + resizeOffsY + (float)MY_OFFSETS_0[offIdx][1] * 2.0 * yScale;
                                        RotateXY(cosAngle, sinAngle, startX, startY, xRot, yRot, &xRot, &yRot);
                                        iassert( glowMaterial );
                                        if (drawExtraFxChar)
                                        {
                                            v25 = (double)glyph->pixelHeight * yScale;
                                            v24 = (double)glyph->pixelWidth * xScale;
                                            DrawTextFxExtraCharacter(
                                                fxMaterialGlow,
                                                extraFxChar,
                                                xRot,
                                                yRot,
                                                v24,
                                                v25,
                                                sinAngle,
                                                cosAngle,
                                                finalColor.packed);
                                        }
                                        else
                                        {
                                            v30 = xRot;
                                            v31 = yRot;
                                            v32 = (0.75 + 1.0) * (xScale * (double)glyph->pixelWidth);
                                            v33 = (0.125 + 1.0) * (yScale * (double)glyph->pixelHeight);
                                            v34.packed = finalColor.packed;
                                            if (Material_HasAnyFogableTechnique(glowMaterial))
                                                R_WarnOncePerFrame(R_WARN_FOGABLE_2DTEXT, glowMaterial->info.name);
                                            else
                                                RB_DrawStretchPicRotate(
                                                    glowMaterial,
                                                    v30,
                                                    v31,
                                                    v32,
                                                    v33,
                                                    glyph->s0,
                                                    glyph->t0,
                                                    glyph->s1,
                                                    glyph->t1,
                                                    sinAngle,
                                                    cosAngle,
                                                    v34.packed,
                                                    GFX_PRIM_STATS_HUD);
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if ((renderFlags & 4) != 0)
                                {
                                    ofs = 1;
                                    if ((renderFlags & 8) != 0)
                                        ofs = 2;
                                    xRot = xa + xAdj + (double)ofs;
                                    yRot = ya + yAdj + (double)ofs;
                                    RotateXY(cosAngle, sinAngle, startX, startY, xRot, yRot, &xRot, &yRot);
                                    if (drawExtraFxChar)
                                    {
                                        h = (double)glyph->pixelHeight * yScale;
                                        v28 = (double)glyph->pixelWidth * xScale;
                                        DrawTextFxExtraCharacter(
                                            fxMaterial,
                                            extraFxChar,
                                            xRot,
                                            yRot,
                                            v28,
                                            h,
                                            sinAngle,
                                            cosAngle,
                                            dropShadowColor.packed);
                                    }
                                    else
                                    {
                                        v40 = xRot;
                                        v41 = yRot;
                                        w = xScale * (double)glyph->pixelWidth;
                                        v43 = yScale * (double)glyph->pixelHeight;
                                        if (Material_HasAnyFogableTechnique(material))
                                            R_WarnOncePerFrame(R_WARN_FOGABLE_2DTEXT, material->info.name);
                                        else
                                            RB_DrawStretchPicRotate(
                                                material,
                                                v40,
                                                v41,
                                                w,
                                                v43,
                                                glyph->s0,
                                                glyph->t0,
                                                glyph->s1,
                                                glyph->t1,
                                                sinAngle,
                                                cosAngle,
                                                dropShadowColor.packed,
                                                GFX_PRIM_STATS_HUD);
                                    }
                                }
                                xRot = xa + xAdj;
                                yRot = ya + yAdj;
                                RotateXY(cosAngle, sinAngle, startX, startY, xRot, yRot, &xRot, &yRot);
                                if (drawExtraFxChar)
                                {
                                    v27 = (double)glyph->pixelHeight * yScale;
                                    v26 = (double)glyph->pixelWidth * xScale;
                                    DrawTextFxExtraCharacter(
                                        fxMaterial,
                                        extraFxChar,
                                        xRot,
                                        yRot,
                                        v26,
                                        v27,
                                        sinAngle,
                                        cosAngle,
                                        finalColor.packed);
                                }
                                else
                                {
                                    v35 = xRot;
                                    v36 = yRot;
                                    v37 = xScale * (double)glyph->pixelWidth;
                                    v38 = yScale * (double)glyph->pixelHeight;
                                    v39.packed = finalColor.packed;
                                    if (Material_HasAnyFogableTechnique(material))
                                        R_WarnOncePerFrame(R_WARN_FOGABLE_2DTEXT, material->info.name);
                                    else
                                        RB_DrawStretchPicRotate(
                                            material,
                                            v35,
                                            v36,
                                            v37,
                                            v38,
                                            glyph->s0,
                                            glyph->t0,
                                            glyph->s1,
                                            glyph->t1,
                                            sinAngle,
                                            cosAngle,
                                            v39.packed,
                                            GFX_PRIM_STATS_HUD);
                                }
                                if ((renderFlags & 2) != 0 && count == cursorPos)
                                {
                                    xRot = xa + xAdj;
                                    RotateXY(cosAngle, sinAngle, startX, startY, xRot, ya, &xRot, &yRot);
                                    RB_DrawCursor(
                                        material,
                                        cursorLetter,
                                        xRot,
                                        yRot,
                                        sinAngle,
                                        cosAngle,
                                        font,
                                        xScale,
                                        yScale,
                                        finalColor.packed);
                                }
                            }
                        }
                        if ((renderFlags & 1) != 0)
                            v23 = monospaceWidth * xScale + xa;
                        else
                            v23 = deltaX * xScale + xa;
                        xa = v23;
                        if ((renderFlags & 0x80) != 0)
                            xa = padding * xScale + xa;
                        ++count;
                        --maxLengthRemaining;
                    }
                }
            }
            if ((renderFlags & 2) != 0 && count == cursorPos)
            {
                xRot = xa;
                RotateXY(cosAngle, sinAngle, startX, startY, xa, ya, &xRot, &yRot);
                RB_DrawCursor(material, cursorLetter, xRot, yRot, sinAngle, cosAngle, font, xScale, yScale, color.packed);
            }
        }
    }
}

void __cdecl RB_DrawStretchPicRotate(
    const Material *material,
    float x,
    float y,
    float w,
    float h,
    float s0,
    float t0,
    float s1,
    float t1,
    float sinAngle,
    float cosAngle,
    unsigned int color,
    GfxPrimStatsTarget statsTarget)
{
    float v13; // [esp+14h] [ebp-30h]
    float v14; // [esp+18h] [ebp-2Ch]
    float v15; // [esp+1Ch] [ebp-28h]
    float v16; // [esp+20h] [ebp-24h]
    float v17; // [esp+24h] [ebp-20h]
    float v18; // [esp+28h] [ebp-1Ch]
    float stepY; // [esp+2Ch] [ebp-18h]
    float stepY_4; // [esp+30h] [ebp-14h]
    float stepX; // [esp+34h] [ebp-10h]
    float stepX_4; // [esp+38h] [ebp-Ch]
    int indexCount; // [esp+3Ch] [ebp-8h]
    unsigned __int16 vertCount; // [esp+40h] [ebp-4h]

    iassert( gfxCmdBufSourceState.viewMode == VIEW_MODE_2D );
    RB_SetTessTechnique(material, TECHNIQUE_UNLIT);
    R_TrackPrims(&gfxCmdBufState, statsTarget);
    RB_CheckTessOverflow(4, 6);
    vertCount = tess.vertexCount;
    indexCount = tess.indexCount;
    tess.vertexCount += 4;
    tess.indexCount += 6;
    tess.indices[indexCount] = vertCount + 3;
    tess.indices[indexCount + 1] = vertCount;
    tess.indices[indexCount + 2] = vertCount + 2;
    tess.indices[indexCount + 3] = vertCount + 2;
    tess.indices[indexCount + 4] = vertCount;
    tess.indices[indexCount + 5] = vertCount + 1;
    stepX = w * cosAngle;
    stepX_4 = w * sinAngle;
    stepY = -h * sinAngle;
    stepY_4 = h * cosAngle;
    R_SetVertex2d(&tess.verts[vertCount], x, y, s0, t0, color);
    v18 = y + stepX_4;
    v17 = x + stepX;
    R_SetVertex2d(&tess.verts[vertCount + 1], v17, v18, s1, t0, color);
    v16 = y + stepX_4 + stepY_4;
    v15 = x + stepX + stepY;
    R_SetVertex2d(&tess.verts[vertCount + 2], v15, v16, s1, t1, color);
    v14 = y + stepY_4;
    v13 = x + stepY;
    R_SetVertex2d(&tess.verts[vertCount + 3], v13, v14, s0, t1, color);
}

double __cdecl RB_DrawHudIcon(
    const char *text,
    float x,
    float y,
    float sinAngle,
    float cosAngle,
    Font_s *font,
    float xScale,
    float yScale,
    unsigned int color)
{
    const Material *v9; // eax
    float s1; // [esp+40h] [ebp-10h]
    float s0; // [esp+44h] [ebp-Ch]
    float h; // [esp+48h] [ebp-8h]
    float w; // [esp+4Ch] [ebp-4h]
    float ya; // [esp+60h] [ebp+10h]

    iassert( text );
    if (*text == 1)
    {
        s0 = 0.0;
        s1 = 1.0;
    }
    else
    {
        iassert( text[0] == CONTXTCMD_TYPE_HUDICON_FLIP );
        s0 = 1.0;
        s1 = 0.0;
    }
    w = (double)((font->pixelHeight * (text[1] - 16) + 16) / 32) * xScale;
    h = (double)((font->pixelHeight * (text[2] - 16) + 16) / 32) * yScale;
    ya = y - ((double)font->pixelHeight * yScale + h) * 0.5;
    iassert( w > 0 );
    iassert( h > 0 );
    if (!IsValidMaterialHandle(*(Material *const *)(text + 3)))
        return 0.0;
    v9 = Material_FromHandle(*(Material **)(text + 3));
    RB_DrawStretchPicRotate(v9, x, ya, w, h, s0, 0.0, s1, 1.0, sinAngle, cosAngle, color, GFX_PRIM_STATS_HUD);
    return w;
}

void __cdecl RB_DrawCursor(
    const Material *material,
    unsigned __int8 cursor,
    float x,
    float y,
    float sinAngle,
    float cosAngle,
    Font_s *font,
    float xScale,
    float yScale,
    unsigned int color)
{
    float v10; // [esp+3Ch] [ebp-24h]
    float w; // [esp+40h] [ebp-20h]
    float h; // [esp+44h] [ebp-1Ch]
    const Glyph *cursorGlyph; // [esp+58h] [ebp-8h]
    unsigned int newColor; // [esp+5Ch] [ebp-4h]

    iassert( font );
    if (((CL_ScaledMilliseconds() / 256) & 1) == 0)
    {
        cursorGlyph = R_GetCharacterGlyph(font, cursor);
        newColor = LongNoSwap(color);
        v10 = (double)cursorGlyph->y0 * yScale + y;
        w = xScale * (double)cursorGlyph->pixelWidth;
        h = yScale * (double)cursorGlyph->pixelHeight;
        if (Material_HasAnyFogableTechnique(material))
            R_WarnOncePerFrame(R_WARN_FOGABLE_2DTEXT, material->info.name);
        else
            RB_DrawStretchPicRotate(
                material,
                x,
                v10,
                w,
                h,
                cursorGlyph->s0,
                cursorGlyph->t0,
                cursorGlyph->s1,
                cursorGlyph->t1,
                sinAngle,
                cosAngle,
                newColor,
                GFX_PRIM_STATS_HUD);
    }
}

void __cdecl RotateXY(
    float cosAngle,
    float sinAngle,
    float pivotX,
    float pivotY,
    float x,
    float y,
    float *outX,
    float *outY)
{
    float tempOutX; // [esp+0h] [ebp-8h]
    float tempOutY; // [esp+4h] [ebp-4h]

    tempOutX = (x - pivotX) * cosAngle + pivotX - (y - pivotY) * sinAngle;
    tempOutY = (y - pivotY) * cosAngle + pivotY + (x - pivotX) * sinAngle;
    *outX = tempOutX;
    *outY = tempOutY;
}

double __cdecl GetMonospaceWidth(Font_s *font, char renderFlags)
{
    if ((renderFlags & 1) != 0)
        return (double)R_GetCharacterGlyph(font, 0x6Fu)->dx;
    else
        return 0.0;
}

void __cdecl GlowColor(GfxColor *result, GfxColor baseColor, GfxColor forcedGlowColor, char renderFlags)
{
    if ((renderFlags & 0x20) != 0)
    {
        *(_WORD *)((char *)&result->packed + 1) = *(_WORD *)((char *)&forcedGlowColor.packed + 1);
        result->array[0] = forcedGlowColor.array[0];
    }
    else
    {
        result->array[2] = (int)((double)baseColor.array[2] * 0.059999999);
        result->array[1] = (int)((double)baseColor.array[1] * 0.059999999);
        result->array[0] = (int)((double)baseColor.array[0] * 0.059999999);
    }
}

char __cdecl SetupPulseFXVars(
    const char *text,
    int maxLength,
    char renderFlags,
    int fxBirthTime,
    int fxLetterTime,
    int fxDecayStartTime,
    int fxDecayDuration,
    bool *resultDrawRandChar,
    int *resultRandSeed,
    int *resultMaxLength,
    bool *resultDecaying,
    int *resultdecayTimeElapsed)
{
    int timeRemainder; // [esp+0h] [ebp-24h]
    int timeElapsed; // [esp+8h] [ebp-1Ch]
    int randSeed; // [esp+10h] [ebp-14h] BYREF
    int strLength; // [esp+14h] [ebp-10h]
    bool drawRandCharAtEnd; // [esp+1Bh] [ebp-9h]
    int decayTimeElapsed; // [esp+1Ch] [ebp-8h]
    bool decaying; // [esp+23h] [ebp-1h]
    int maxLengtha; // [esp+30h] [ebp+Ch]

    if ((renderFlags & 0x40) != 0)
    {
        drawRandCharAtEnd = 0;
        randSeed = 1;
        decaying = 0;
        decayTimeElapsed = 0;
        timeElapsed = gfxCmdBufSourceState.sceneDef.time - fxBirthTime;
        iassert( timeElapsed >= 0 );
        strLength = SEH_PrintStrlen(text);
        if (strLength > maxLength)
            strLength = maxLength;
        if (timeElapsed <= fxDecayDuration + fxDecayStartTime)
        {
            if (timeElapsed < fxLetterTime * strLength)
            {
                iassert( fxLetterTime );
                maxLengtha = timeElapsed / fxLetterTime;
                drawRandCharAtEnd = 1;
                timeRemainder = timeElapsed % fxLetterTime;
                if (fxLetterTime / 4)
                    timeRemainder /= fxLetterTime / 4;
                randSeed = maxLengtha + timeRemainder + strLength + fxBirthTime;
                RandWithSeed(&randSeed);
                RandWithSeed(&randSeed);
                maxLength = maxLengtha + 1;
            }
            else if (timeElapsed > fxDecayStartTime)
            {
                decaying = 1;
                randSeed = strLength + fxBirthTime;
                RandWithSeed(&randSeed);
                RandWithSeed(&randSeed);
                decayTimeElapsed = timeElapsed - fxDecayStartTime;
            }
            *resultDrawRandChar = drawRandCharAtEnd;
            *resultRandSeed = randSeed;
            *resultMaxLength = maxLength;
            *resultDecaying = decaying;
            *resultdecayTimeElapsed = decayTimeElapsed;
            return 1;
        }
        else
        {
            *resultDrawRandChar = 0;
            *resultRandSeed = 1;
            *resultMaxLength = maxLength;
            *resultDecaying = 0;
            *resultdecayTimeElapsed = 0;
            return 0;
        }
    }
    else
    {
        *resultDrawRandChar = 0;
        *resultRandSeed = 1;
        *resultMaxLength = maxLength;
        *resultDecaying = 0;
        *resultdecayTimeElapsed = 0;
        return 1;
    }
}

void __cdecl GetDecayingLetterInfo(
    unsigned int letter,
    Font_s *font,
    int *randSeed,
    int decayTimeElapsed,
    int fxBirthTime,
    int fxDecayDuration,
    unsigned __int8 alpha,
    bool *resultSkipDrawing,
    unsigned __int8 *resultAlpha,
    unsigned int *resultLetter,
    bool *resultDrawExtraFxChar)
{
    unsigned __int8 v11; // [esp+0h] [ebp-48h]
    int v12; // [esp+10h] [ebp-38h]
    float v13; // [esp+18h] [ebp-30h]
    int scrambleSeed; // [esp+28h] [ebp-20h] BYREF
    float tickRatio; // [esp+2Ch] [ebp-1Ch]
    int tickPeriod; // [esp+30h] [ebp-18h]
    bool drawExtraFxChar; // [esp+37h] [ebp-11h]
    float fade; // [esp+38h] [ebp-10h]
    int tickCount; // [esp+3Ch] [ebp-Ch]
    bool skipDrawing; // [esp+43h] [ebp-5h]
    int timeLimit; // [esp+44h] [ebp-4h]

    skipDrawing = 0;
    fade = 1.0;
    drawExtraFxChar = 0;
    tickRatio = (double)fxDecayDuration / 1000.0;
    tickCount = (int)(tickRatio * 30.0);
    tickPeriod = fxDecayDuration / tickCount;
    timeLimit = fxDecayDuration / tickCount * ((int)RandWithSeed(randSeed) % tickCount);
    if (decayTimeElapsed < timeLimit)
    {
        if (decayTimeElapsed + 60 >= timeLimit)
        {
            scrambleSeed = decayTimeElapsed + letter + fxBirthTime;
            if ((int)RandWithSeed(&scrambleSeed) % 2)
            {
                drawExtraFxChar = 1;
                letter = 79;
            }
            else
            {
                letter = R_FontGetRandomLetter(font, scrambleSeed);
            }
            fade = (double)(decayTimeElapsed + 60 - timeLimit) / 60.0;
            fade = 1.0 - fade;
            fade = (double)alpha / 255.0 * fade;
        }
    }
    else
    {
        skipDrawing = 1;
    }
    *resultSkipDrawing = skipDrawing;
    *resultLetter = letter;
    *resultAlpha = CLAMP((int)(fade * 255.0f), 0, 255);
    *resultDrawExtraFxChar = drawExtraFxChar;
}

void __cdecl DrawTextFxExtraCharacter(
    const Material *material,
    int charIndex,
    float x,
    float y,
    float w,
    float h,
    float sinAngle,
    float cosAngle,
    unsigned int color)
{
    float s1; // [esp+38h] [ebp-8h]
    float s0; // [esp+3Ch] [ebp-4h]

    s0 = (double)(charIndex % 16) * 0.0625;
    s1 = s0 + 0.0625;
    RB_DrawStretchPicRotate(material, x, y, w, h, s0, 0.0, s1, 1.0, sinAngle, cosAngle, color, GFX_PRIM_STATS_HUD);
}

unsigned __int8 __cdecl ModulateByteColors(unsigned __int8 colorA, unsigned __int8 colorB)
{
    return (int)((double)colorA / 255.0 * ((double)colorB / 255.0) * 255.0);
}

void __cdecl RB_DrawTextInSpace(
    const char *text,
    Font_s *font,
    const float *org,
    const float *xPixelStep,
    const float *yPixelStep,
    unsigned int color)
{
    float scale; // [esp+0h] [ebp-60h]
    float scalea; // [esp+0h] [ebp-60h]
    float scaleb; // [esp+0h] [ebp-60h]
    float pixelWidth; // [esp+4h] [ebp-5Ch]
    float pixelHeight; // [esp+4h] [ebp-5Ch]
    float curOrg[3]; // [esp+20h] [ebp-40h] BYREF
    float result[3]; // [esp+2Ch] [ebp-34h] BYREF
    const Glyph *glyph; // [esp+38h] [ebp-28h]
    float xyz[3]; // [esp+3Ch] [ebp-24h] BYREF
    const Material *material; // [esp+48h] [ebp-18h]
    unsigned int letter; // [esp+4Ch] [ebp-14h]
    float dy[3]; // [esp+50h] [ebp-10h] BYREF
    unsigned int newColor; // [esp+5Ch] [ebp-4h]

    iassert( text );
    iassert( font );
    material = Material_FromHandle(font->material);
    iassert( material );
    if (tess.indexCount)
        RB_EndTessSurface();
    R_Set3D(&gfxCmdBufSourceState);
    Vec3Mad(org, -0.5, xPixelStep, curOrg);
    Vec3Mad(curOrg, -0.5, yPixelStep, curOrg);
    while (*text)
    {
        letter = SEH_ReadCharFromString(&text, 0);
        iassert( text );
        glyph = R_GetCharacterGlyph(font, letter);
        newColor = LongNoSwap(color);
        scale = (float)glyph->x0;
        Vec3Mad(curOrg, scale, xPixelStep, xyz);
        scalea = (float)glyph->y0;
        Vec3Mad(xyz, scalea, yPixelStep, xyz);
        pixelWidth = (float)glyph->pixelWidth;
        Vec3Scale(xPixelStep, pixelWidth, result);
        pixelHeight = (float)glyph->pixelHeight;
        Vec3Scale(yPixelStep, pixelHeight, dy);
        RB_DrawCharInSpace(material, xyz, result, dy, glyph, newColor);
        scaleb = (float)glyph->dx;
        Vec3Mad(curOrg, scaleb, xPixelStep, curOrg);
    }
    if (tess.indexCount)
        RB_EndTessSurface();
}

void __cdecl RB_DrawCharInSpace(
    const Material *material,
    float *xyz,
    const float *dx,
    const float *dy,
    const Glyph *glyph,
    unsigned int color)
{
    float v6; // [esp+18h] [ebp-30h]
    float v7; // [esp+1Ch] [ebp-2Ch]
    float v8; // [esp+20h] [ebp-28h]
    float v9; // [esp+24h] [ebp-24h]
    float v10; // [esp+28h] [ebp-20h]
    float v11; // [esp+2Ch] [ebp-1Ch]
    float x; // [esp+30h] [ebp-18h]
    float y; // [esp+34h] [ebp-14h]
    float z; // [esp+38h] [ebp-10h]
    int indexCount; // [esp+3Ch] [ebp-Ch]
    unsigned __int16 vertCount; // [esp+40h] [ebp-8h]
    GfxColor unpackedColor; // [esp+44h] [ebp-4h] BYREF

    RB_SetTessTechnique(material, TECHNIQUE_UNLIT);
    R_TrackPrims(&gfxCmdBufState, GFX_PRIM_STATS_DEBUG);
    RB_CheckTessOverflow(4, 6);
    vertCount = tess.vertexCount;
    indexCount = LOWORD(tess.indexCount);
    tess.vertexCount += 4;
    tess.indexCount += 6;
    tess.indices[indexCount] = vertCount + 3;
    tess.indices[indexCount + 1] = vertCount;
    tess.indices[indexCount + 2] = vertCount + 2;
    tess.indices[indexCount + 3] = vertCount + 2;
    tess.indices[indexCount + 4] = vertCount;
    tess.indices[indexCount + 5] = vertCount + 1;
    unpackedColor.packed = color;
    R_SetVertex3d(
        &tess.verts[vertCount],
        *xyz,
        xyz[1],
        xyz[2],
        glyph->s0,
        glyph->t0,
        (const unsigned __int8 *)&unpackedColor);
    z = xyz[2] + dx[2];
    y = xyz[1] + dx[1];
    x = *xyz + *dx;
    R_SetVertex3d(&tess.verts[vertCount + 1], x, y, z, glyph->s1, glyph->t0, (const unsigned __int8 *)&unpackedColor);
    v11 = xyz[2] + dx[2] + dy[2];
    v10 = xyz[1] + dx[1] + dy[1];
    v9 = *xyz + *dx + *dy;
    R_SetVertex3d(&tess.verts[vertCount + 2], v9, v10, v11, glyph->s1, glyph->t1, (const unsigned __int8 *)&unpackedColor);
    v8 = xyz[2] + dy[2];
    v7 = xyz[1] + dy[1];
    v6 = *xyz + *dy;
    R_SetVertex3d(&tess.verts[vertCount + 3], v6, v7, v8, glyph->s0, glyph->t1, (const unsigned __int8 *)&unpackedColor);
}

void __cdecl RB_DrawText2DCmd(GfxRenderCommandExecState *execState)
{
    float v1; // [esp+5Ch] [ebp-10h]
    float cosAngle; // [esp+60h] [ebp-Ch]
    float sinAngle; // [esp+64h] [ebp-8h]
    const GfxCmdDrawText2D *cmd; // [esp+68h] [ebp-4h]

    cmd = (const GfxCmdDrawText2D *)execState->cmd;
    v1 = cmd->rotation * 0.01745329238474369;
    cosAngle = cos(v1);
    sinAngle = sin(v1);
    DrawText2D(
        cmd->text,
        cmd->x,
        cmd->y,
        cmd->font,
        cmd->xScale,
        cmd->yScale,
        sinAngle,
        cosAngle,
        cmd->color,
        cmd->maxChars,
        cmd->renderFlags,
        cmd->cursorPos,
        cmd->cursorLetter,
        cmd->padding,
        cmd->glowForceColor,
        cmd->fxBirthTime,
        cmd->fxLetterTime,
        cmd->fxDecayStartTime,
        cmd->fxDecayDuration,
        cmd->fxMaterial,
        cmd->fxMaterialGlow);

    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_DrawText3DCmd(GfxRenderCommandExecState *execState)
{
    GfxCmdDrawText3D *cmd = (GfxCmdDrawText3D *)execState->cmd;

    RB_DrawTextInSpace(cmd->text, cmd->font, cmd->org, cmd->xPixelStep, cmd->yPixelStep, cmd->color);
    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_ProjectionSetCmd(GfxRenderCommandExecState *execState)
{
    GfxCmdProjectionSet *cmd = (GfxCmdProjectionSet *)execState->cmd;

    if (cmd->projection)
    {
        if (cmd->projection == GFX_PROJECTION_3D)
        {
            if (tess.indexCount)
                RB_EndTessSurface();
            R_Set3D(&gfxCmdBufSourceState);
        }
        else if (!alwaysfails)
        {
            MyAssertHandler(".\\rb_backend.cpp", 2543, 0, "Invalid projection type");
        }
    }
    else
    {
        if (tess.indexCount)
            RB_EndTessSurface();
        R_Set2D(&gfxCmdBufSourceState);
    }

    execState->cmd = (char *)execState->cmd + cmd->header.byteCount;
}

void __cdecl RB_ResetStatTracking()
{
    RB_Stats_UpdateMaxs(&g_frameStatsCur, &backEnd.frameStatsMax);
    memset((unsigned __int8 *)&g_frameStatsCur, 0, sizeof(g_frameStatsCur));
    g_viewStats = (GfxViewStats *)&g_frameStatsCur;
}

void __cdecl RB_BeginFrame(const GfxBackEndData *data)
{
    int hr; // [esp+0h] [ebp-4h]

    backEndData = (GfxBackEndData*)data;
    if ((data->drawType & 1) != 0)
    {
        ++r_glob.backEndFrameCount;
        RB_UpdateBackEndDvarOptions();
        RB_PatchStaticModelCache();
        RB_PatchModelLighting(backEndData->modelLightingPatchList, backEndData->modelLightingPatchCount);

        iassert(dx.device);
        iassert(!dx.inScene);

        dx.inScene = 1;

        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("dx.device->BeginScene()\n");
            hr = dx.device->BeginScene();
            if (hr < 0)
            {
                do
                {
                    ++g_disableRendering;
                    Com_Error(ERR_FATAL, ".\\rb_backend.cpp (%i) dx.device->BeginScene() failed: %s\n", 2730, R_ErrorDescription(hr));
                } while (alwaysfails);
            }
        } while (alwaysfails);

        RB_UploadShaderStep();
        RB_ResetStatTracking();
        R_Cinematic_UpdateFrame();
        tess.indexCount = 0;
        tess.vertexCount = 0;
    }
}

void __cdecl RB_EndFrame(char drawType)
{
    if ((drawType & 2) != 0)
    {
        if (r_logFile->current.integer)
            RB_LogPrint("***************** RB_SwapBuffers *****************\n\n\n");
        RB_SwapBuffers();
        RB_UpdateLogging();
        iassert( r_gamma );
        iassert( r_ignoreHwGamma );
        if (r_gamma->modified || r_ignoreHwGamma->modified)
        {
            Dvar_ClearModified((dvar_s*)r_gamma);
            Dvar_ClearModified((dvar_s*)r_ignoreHwGamma);
            if (!r_ignoreHwGamma->current.enabled)
                R_SetColorMappings();
        }
    }
}

GfxIndexBufferState *RB_SwapBuffers()
{
    GfxIndexBufferState *result;
    int hr;

    if (dx.targetWindowIndex < 0 || dx.targetWindowIndex >= dx.windowCount)
        MyAssertHandler(
            ".\\rb_backend.cpp",
            2794,
            0,
            "%s\n\t(dx.targetWindowIndex) = %i",
            "(dx.targetWindowIndex >= 0 && dx.targetWindowIndex < dx.windowCount)",
            dx.targetWindowIndex);

    {
        PROF_SCOPED("Present");
        hr = dx.windows[dx.targetWindowIndex].swapChain->Present(0, 0, 0, 0, 0);
    }

    if (hr < 0 && hr != -2005530520)
    {
        Com_Error(ERR_FATAL, "Direct3DDevice9::Present failed: %s\n", R_ErrorDescription(hr));
    }

    R_HW_InsertFence(&dx.swapFence);
    result = gfxBuf.dynamicIndexBuffer;
    gfxBuf.dynamicIndexBuffer->used = 0;
    return result;
}

void RB_UpdateBackEndDvarOptions()
{
    if (!dx.deviceLost)
    {
        if (R_CheckDvarModified(r_texFilterAnisoMax)
            || R_CheckDvarModified(r_texFilterDisable)
            || R_CheckDvarModified(r_texFilterAnisoMin)
            || R_CheckDvarModified(r_texFilterMipMode)
            || R_CheckDvarModified(r_texFilterMipBias))
        {
            R_SetTexFilter();
        }
        if (R_CheckDvarModified(r_showPixelCost) && !r_showPixelCost->current.integer)
            R_PixelCost_PrintColorCodeKey();
        if (R_CheckDvarModified(r_aaAlpha))
        {
            if (gfxMetrics.hasTransparencyMsaa)
                R_SetAlphaAntiAliasingState(gfxCmdBufState.prim.device, gfxCmdBufState.activeStateBits[0]);
        }
    }
}

void __cdecl RB_ExecuteRenderCommandsLoop(const void *cmds)
{
    const GfxCmdHeader *header; // [esp+0h] [ebp-Ch]
    GfxRenderCommandExecState execState; // [esp+4h] [ebp-8h] BYREF
    const void *prevCmd; // [esp+8h] [ebp-4h]

    iassert(!tess.indexCount);

    execState.cmd = cmds;
    prevCmd = cmds;
    while (1)
    {
        iassert((reinterpret_cast<ptype_int>(execState.cmd) & 3) == 0);
        header = (const GfxCmdHeader *)execState.cmd;

        if (!header->id)
            break;

        iassert(header->id < (sizeof(RB_RenderCommandTable) / (sizeof(RB_RenderCommandTable[0]) * (sizeof(RB_RenderCommandTable) != 4 || sizeof(RB_RenderCommandTable[0]) <= 4))));
        iassert(RB_RenderCommandTable[header->id]);
        RB_RenderCommandTable[header->id](&execState);
        iassert(execState.cmd != prevCmd);
        prevCmd = execState.cmd;
        iassert(!g_primStats || tess.indexCount);
    }
    if (tess.indexCount)
        RB_EndTessSurface();
}

void __cdecl RB_Draw3D()
{
    const GfxBackEndData *data; // [esp+30h] [ebp-8h]

    data = backEndData;
    if (backEndData->viewInfoCount)
    {
        PROF_SCOPED("ExecuteRenderCmds");
        RB_Draw3DInternal(&data->viewInfo[data->viewInfoIndex]);
    }
}

int RB_AdaptiveGpuSyncFinal()
{
    unsigned __int64 v0; // rax
    int waitedTime; // [esp+18h] [ebp-8h]
    int startTime; // [esp+1Ch] [ebp-4h]

    LODWORD(v0) = RB_IsGpuFenceFinished();
    if (v0)
    {
        if (dx.gpuSyncDelay > 0x4E20)
        {
            v0 = 127 * ((dx.gpuSyncDelay - 20000) / 0x80);
            dx.gpuSyncDelay = v0;
        }
        else
        {
            dx.gpuSyncDelay = 0;
        }
    }
    else
    {
        startTime = __rdtsc();
        while (!RB_IsGpuFenceFinished())
        {
            if ((__rdtsc() - startTime) < 0)
            {
                RB_AbandonGpuFence();
                break;
            }
        }
        LODWORD(v0) = __rdtsc() - startTime;
        waitedTime = v0;
        if ((v0 & 0x80000000) == 0LL)
        {
            LODWORD(v0) = LODWORD(dx.gpuSyncDelay) + v0 / 16;
            dx.gpuSyncDelay += waitedTime / 16;
        }
    }
    return v0;
}

void __cdecl RB_CallExecuteRenderCommands()
{
    const char *v0; // eax
    int hr; // [esp+40h] [ebp-4h]
    
    PROF_SCOPED("ExecuteRenderCmds");
    if ((backEndData->drawType & 2) != 0)
    {
        if (g_primStats)
            MyAssertHandler(
                ".\\rb_backend.cpp",
                3055,
                0,
                "%s\n\t(g_primStats - g_viewStats->primStats) = %i",
                "(!g_primStats)",
                ((char *)g_primStats - (char *)g_viewStats) / 24);
        if (tess.indexCount)
            MyAssertHandler(
                ".\\rb_backend.cpp",
                3057,
                0,
                "%s\n\t(tess.indexCount) = %i",
                "(!tess.indexCount)",
                tess.indexCount);
        if (backEndData->viewInfoCount)
            RB_Draw3DCommon();
        if (tess.indexCount)
            MyAssertHandler(
                ".\\rb_backend.cpp",
                3062,
                0,
                "%s\n\t(tess.indexCount) = %i",
                "(!tess.indexCount)",
                tess.indexCount);
        R_InitCmdBufSourceState(&gfxCmdBufSourceState, &gfxCmdBufInput, 0);
        gfxCmdBufSourceState.input.data = backEndData;
        memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
        memset((unsigned __int8 *)gfxCmdBufState.vertexShaderConstState, 0, sizeof(gfxCmdBufState.vertexShaderConstState));
        memset((unsigned __int8 *)gfxCmdBufState.pixelShaderConstState, 0, sizeof(gfxCmdBufState.pixelShaderConstState));
        R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
        R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
        RB_InitSceneViewport();
        if (backEndData->cmds)
            RB_ExecuteRenderCommandsLoop(backEndData->cmds);
        if (r_drawPrimHistogram->current.enabled)
            RB_DrawPrimHistogramOverlay();
        if (tess.indexCount)
            RB_EndTessSurface();
        memcpy(&gfxCmdBufState, &gfxCmdBufState, sizeof(gfxCmdBufState));
        if (gfxCmdBufState.prim.indexBuffer)
            R_ChangeIndices(&gfxCmdBufState.prim, 0);
        R_ClearAllStreamSources(&gfxCmdBufState.prim);
        if (g_primStats)
            MyAssertHandler(
                ".\\rb_backend.cpp",
                3100,
                0,
                "%s\n\t(g_primStats - g_viewStats->primStats) = %i",
                "(!g_primStats)",
                ((char *)g_primStats - (char *)g_viewStats) / 24);
        if (tess.indexCount)
            MyAssertHandler(
                ".\\rb_backend.cpp",
                3102,
                0,
                "%s\n\t(tess.indexCount) = %i",
                "(!tess.indexCount)",
                tess.indexCount);
        iassert( dx.device );
        iassert( dx.inScene );
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("dx.device->EndScene()\n");
            //hr = ((int(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *))dx.device->EndScene)(dx.device, dx.device);
            hr = dx.device->EndScene();
            if (hr < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v0 = R_ErrorDescription(hr);
                    Com_Error(ERR_FATAL, ".\\rb_backend.cpp (%i) dx.device->EndScene() failed: %s\n", 3107, v0);
                } while (alwaysfails);
            }
        } while (alwaysfails);
        dx.inScene = 0;
        if (!r_glob.isRenderingRemoteUpdate)
        {
            if (dx.gpuSync)
            {
                R_AcquireGpuFenceLock();
                RB_AdaptiveGpuSyncFinal();
                if (dx.gpuSync == 2)
                    dx.gpuSyncDelay = (unsigned __int64)(30.0 / msecPerRawTimerTick);
                R_InsertGpuFence();
                R_ReleaseGpuFenceLock();
            }
            else
            {
                dx.gpuSyncDelay = 0;
            }
        }
    }
}

void RB_RenderThreadIdle()
{
    if (sys_smp_allowed->current.enabled && r_smp_backend->current.enabled)
        R_ProcessWorkerCmdsWithTimeout(Sys_IsMainThreadReady, 1);
    else
        Sys_WaitForMainThread();
}

// positive sp value has been detected, the output may be wrong!
const void *data;
void __cdecl  RB_RenderThread(unsigned int threadContext)
{
    void *Value; // eax
    signed int wait; // [esp+34h] [ebp-8h]
    unsigned int start; // [esp+38h] [ebp-4h]

    iassert(threadContext == THREAD_CONTEXT_BACKEND);

    while (r_glob.haveThreadOwnership)
        NET_Sleep(1);

    while (1)
    {
        Value = Sys_GetValue(2);
        if (!_setjmp(*(jmp_buf *)Value))
            break;
        Profile_Recover(1);
        if (r_glob.isRenderingRemoteUpdate)
        {
            r_glob.isRenderingRemoteUpdate = 0;
            iassert(!r_glob.screenUpdateNotify);
            r_glob.screenUpdateNotify = 1;
            data = 0;
        }
        else if (data)
        {
            Com_ErrorAbort();
        }
    }
    Profile_Guard(1);
    PROF_SCOPED("RendererSleep");
    while (1)
    {
        while (1)
        {
            {
                PROF_SCOPED("WaitBackendEvent");
                KISAK_NULLSUB();
                R_ProcessWorkerCmdsWithTimeout(Sys_WaitBackendEvent, 1);
            }

            if (Sys_FinishRenderer())
            {
                data = Sys_RendererSleep();
                if (data)
                    RB_RenderCommandFrame((GfxBackEndData*)data);
                Sys_StopRenderer();
                //KISAK_NULLSUB();
                RB_RenderThreadIdle();
                Sys_StartRenderer();
            }
            if (r_glob.remoteScreenUpdateNesting)
            {
                if (!data)
                {
                    data = Sys_RendererSleep();
                    if (data)
                        RB_RenderCommandFrame((GfxBackEndData *)data);
                }
                iassert(!r_glob.screenUpdateNotify);
                r_glob.screenUpdateNotify = 1;
                iassert(!r_glob.isRenderingRemoteUpdate);
                r_glob.isRenderingRemoteUpdate = 1;
                do
                {
                    start = Sys_Milliseconds();
                    SCR_UpdateScreen();
                    wait = 33 - (Sys_Milliseconds() - start);
                    if (wait > 0)
                        NET_Sleep(wait);
                } while (r_glob.remoteScreenUpdateNesting);
                iassert(r_glob.isRenderingRemoteUpdate);
                r_glob.isRenderingRemoteUpdate = 0;
                iassert(!r_glob.screenUpdateNotify);
                r_glob.screenUpdateNotify = 1;
            }
            if (!data)
                break;
        LABEL_39:
            data = 0;
        }
        data = Sys_RendererSleep();
        if (data)
        {
            RB_RenderCommandFrame((GfxBackEndData *)data);
            goto LABEL_39;
        }
        KISAK_NULLSUB();
        R_ProcessWorkerCmdsWithTimeout(Sys_RendererReady, 0);
    }
}

int __cdecl RB_BackendTimeout()
{
    BOOL v1; // [esp+0h] [ebp-Ch]
    _BYTE v2[4]; // [esp+8h] [ebp-4h] BYREF

    if (dx.swapFence)
    {
        //v1 = dx.swapFence->GetData(dx.swapFence, v2, 4u, 1u) == 1;
        v1 = dx.swapFence->GetData(v2, 4, 1) == 1;
    }
    else
        v1 = 0;
    return !v1;
}

void __cdecl RB_RenderCommandFrame(const GfxBackEndData *data)
{
    unsigned int drawType; // [esp+28h] [ebp-8h]
    bool allowRendering; // [esp+2Fh] [ebp-1h]

    //Profile_EndInternal(0);
    LOBYTE(drawType) = 0;
    if (R_CheckLostDevice())
        allowRendering = g_disableRendering == 0;
    else
        allowRendering = 0;
    if (allowRendering)
    {
        KISAK_NULLSUB();
        RB_BeginFrame(data);
        RB_Draw3D();
        RB_CallExecuteRenderCommands();
        iassert( backEndData == data );
        drawType = backEndData->drawType;
        backEndData = 0;
    }
    Sys_RenderCompleted();
    {
        PROF_SCOPED("WaitRenderSwap");
        R_ProcessWorkerCmdsWithTimeout(RB_BackendTimeout, 1);
    }
    if (allowRendering)
    {
        KISAK_NULLSUB();
        RB_EndFrame(drawType);
    }
    //Profile_Begin(172);
}

void __cdecl RB_InitBackendGlobalStructs()
{
    memset((unsigned __int8 *)&backEnd, 0, sizeof(backEnd));
    RB_InitSceneViewport();
    RB_InitCodeImages();
}

void __cdecl RB_SetBspImages()
{
    if (rgp.world->skyImage && (rgp.world->skySamplerState & 7) == 0)
        MyAssertHandler(
            ".\\rb_backend.cpp",
            3505,
            0,
            "%s",
            "!rgp.world->skyImage || (rgp.world->skySamplerState & SAMPLER_FILTER_MASK)");
    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_OUTDOOR] = rgp.world->outdoorImage;
    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_SKY] = rgp.world->skyImage;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_SKY] = rgp.world->skySamplerState;
}

void __cdecl RB_BindDefaultImages()
{
    GfxCmdBufContext context; // [esp+0h] [ebp-10h]
    unsigned int samplerIndex; // [esp+8h] [ebp-8h]

    context.source = &gfxCmdBufSourceState;
    context.state = &gfxCmdBufState;
    for (samplerIndex = 0; samplerIndex < 0x10; ++samplerIndex)
        R_SetSampler(context, samplerIndex, 1u, rgp.whiteImage);
}

void __cdecl RB_InitCodeImages()
{
    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_BLACK] = rgp.blackImage;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_BLACK] = SAMPLER_FILTER_NEAREST;
    rg.codeImageNames[TEXTURE_SRC_CODE_BLACK] = 0;

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_WHITE] = rgp.whiteImage;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_WHITE] = SAMPLER_FILTER_NEAREST;
    rg.codeImageNames[TEXTURE_SRC_CODE_WHITE] = 0;

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_IDENTITY_NORMAL_MAP] = rgp.identityNormalMapImage;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_IDENTITY_NORMAL_MAP] = SAMPLER_FILTER_NEAREST;
    rg.codeImageNames[TEXTURE_SRC_CODE_IDENTITY_NORMAL_MAP] = 0;

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_MODEL_LIGHTING] = 0;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_MODEL_LIGHTING] = (SAMPLER_CLAMP_MASK | SAMPLER_FILTER_LINEAR);
    rg.codeImageNames[TEXTURE_SRC_CODE_MODEL_LIGHTING] = 0;

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_SHADOWCOOKIE] = gfxRenderTargets[R_RENDERTARGET_SHADOWCOOKIE].image;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_SHADOWCOOKIE] = (SAMPLER_CLAMP_V | SAMPLER_CLAMP_U | SAMPLER_FILTER_LINEAR);
    rg.codeImageNames[TEXTURE_SRC_CODE_SHADOWCOOKIE] = "shadowCookieSampler";

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_SHADOWMAP_SUN] = gfxRenderTargets[R_RENDERTARGET_SHADOWMAP_SUN].image;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_SHADOWMAP_SUN] = gfxMetrics.shadowmapSamplerState;
    rg.codeImageNames[TEXTURE_SRC_CODE_SHADOWMAP_SUN] = "shadowmapSamplerSun";

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_SHADOWMAP_SPOT] = 0;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_SHADOWMAP_SPOT] = gfxMetrics.shadowmapSamplerState;
    rg.codeImageNames[TEXTURE_SRC_CODE_SHADOWMAP_SPOT] = "shadowmapSamplerSpot";

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_FEEDBACK] = 0;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_FEEDBACK] = (SAMPLER_CLAMP_V | SAMPLER_CLAMP_U | SAMPLER_FILTER_LINEAR);
    rg.codeImageNames[TEXTURE_SRC_CODE_FEEDBACK] = "feedbackSampler";

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_RESOLVED_POST_SUN] = 0;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_RESOLVED_POST_SUN] = (SAMPLER_CLAMP_V | SAMPLER_CLAMP_U | SAMPLER_FILTER_LINEAR);
    rg.codeImageNames[TEXTURE_SRC_CODE_RESOLVED_POST_SUN] = 0;

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_RESOLVED_SCENE] = 0;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_RESOLVED_SCENE] = (SAMPLER_CLAMP_V | SAMPLER_CLAMP_U | SAMPLER_FILTER_LINEAR);
    rg.codeImageNames[TEXTURE_SRC_CODE_RESOLVED_SCENE] = 0;

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_POST_EFFECT_0] = gfxRenderTargets[R_RENDERTARGET_POST_EFFECT_0].image;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_POST_EFFECT_0] = (SAMPLER_CLAMP_V | SAMPLER_CLAMP_U | SAMPLER_FILTER_LINEAR);
    rg.codeImageNames[TEXTURE_SRC_CODE_POST_EFFECT_0] = "postEffect0";

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_POST_EFFECT_1] = gfxRenderTargets[R_RENDERTARGET_POST_EFFECT_1].image;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_POST_EFFECT_1] = (SAMPLER_CLAMP_V | SAMPLER_CLAMP_U | SAMPLER_FILTER_LINEAR);
    rg.codeImageNames[TEXTURE_SRC_CODE_POST_EFFECT_1] = "postEffect1";

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_SKY] = 0;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_SKY] = SAMPLER_FILTER_SHIFT;
    rg.codeImageNames[TEXTURE_SRC_CODE_SKY] = "sampler.sky";

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_LIGHT_ATTENUATION] = 0;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_LIGHT_ATTENUATION] = SAMPLER_FILTER_SHIFT;
    rg.codeImageNames[TEXTURE_SRC_CODE_LIGHT_ATTENUATION] = "attenuationSampler";

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_DYNAMIC_SHADOWS] = 0;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_DYNAMIC_SHADOWS] = (SAMPLER_CLAMP_V | SAMPLER_CLAMP_U | SAMPLER_FILTER_LINEAR);
    rg.codeImageNames[TEXTURE_SRC_CODE_DYNAMIC_SHADOWS] = 0;

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_OUTDOOR] = 0;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_OUTDOOR] = (SAMPLER_CLAMP_V | SAMPLER_CLAMP_U | SAMPLER_FILTER_LINEAR);
    rg.codeImageNames[TEXTURE_SRC_CODE_OUTDOOR] = 0;

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_FLOATZ] = gfxRenderTargets[R_RENDERTARGET_FLOAT_Z].image;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_FLOATZ] = (SAMPLER_CLAMP_V | SAMPLER_CLAMP_U | SAMPLER_FILTER_NEAREST);
    rg.codeImageNames[TEXTURE_SRC_CODE_FLOATZ] = 0;

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_Y] = 0;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_CINEMATIC_Y] = (SAMPLER_CLAMP_V | SAMPLER_CLAMP_U | SAMPLER_FILTER_LINEAR);
    rg.codeImageNames[TEXTURE_SRC_CODE_CINEMATIC_Y] = "cinematicY";

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_CR] = 0;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_CINEMATIC_CR] = (SAMPLER_CLAMP_V | SAMPLER_CLAMP_U | SAMPLER_FILTER_LINEAR);
    rg.codeImageNames[TEXTURE_SRC_CODE_CINEMATIC_CR] = "cinematicCr";

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_CB] = 0;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_CINEMATIC_CB] = (SAMPLER_CLAMP_V | SAMPLER_CLAMP_U | SAMPLER_FILTER_LINEAR);
    rg.codeImageNames[TEXTURE_SRC_CODE_CINEMATIC_CB] = "cinematicCb";

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_A] = 0;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_CINEMATIC_A] = (SAMPLER_CLAMP_V | SAMPLER_CLAMP_U | SAMPLER_FILTER_LINEAR);
    rg.codeImageNames[TEXTURE_SRC_CODE_CINEMATIC_A] = "cinematicA";
}

void __cdecl RB_RegisterBackendAssets()
{
    backEnd.debugFont = R_RegisterFont("fonts/smalldevfont", 1);
}

