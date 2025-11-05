#include "r_pretess.h"
#include <qcommon/threads.h>
#include <universal/q_shared.h>
#include "r_scene.h"
#include "r_buffers.h"
#include "r_drawsurf.h"
#include "rb_tess.h"
#include "r_draw_staticmodel.h"
#include <universal/profile.h>


void __cdecl R_InitDrawSurfListInfo(GfxDrawSurfListInfo *info)
{
    iassert(Sys_IsMainThread());

    info->drawSurfs = 0;
    info->drawSurfCount = 0;
    info->baseTechType = TECHNIQUE_DEPTH_PREPASS;
    info->viewInfo = 0;
    info->viewOrigin[0] = 0.0f;
    info->viewOrigin[1] = 0.0f;
    info->viewOrigin[2] = 0.0f;
    info->viewOrigin[3] = 0.0f;
    info->light = 0;
    info->cameraView = 0;

    iassert(!info->light);
}

void __cdecl R_EmitDrawSurfList(GfxDrawSurf *drawSurfs, unsigned int drawSurfCount)
{
    int newDrawSurfCount; // [esp+4Ch] [ebp-4h]

    PROF_SCOPED("R_EmitDrawSurfList");

    iassert(drawSurfs);

    // LWSS ADD from blops
    if (!drawSurfCount)
    {
        return; // I had this hit once (ever!) while dying
    }
    // LWSS END

    newDrawSurfCount = drawSurfCount + frontEndDataOut->drawSurfCount;

    if (newDrawSurfCount <= 0x8000)
    {
        Com_Memcpy(&frontEndDataOut->drawSurfs[frontEndDataOut->drawSurfCount], drawSurfs, 8 * drawSurfCount);
        frontEndDataOut->drawSurfCount = newDrawSurfCount;
    }
    else
    {
        R_WarnOncePerFrame(R_WARN_MAX_DRAWSURFS);
    }
}

void __cdecl R_MergeAndEmitDrawSurfLists(DrawSurfType firstStage, int stageCount)
{
    unsigned int v2; // eax
    signed int v3; // [esp+0h] [ebp-164h]
    unsigned int srcStageIndex; // [esp+38h] [ebp-12Ch]
    int freeDrawSurfCount; // [esp+3Ch] [ebp-128h]
    unsigned int stageIndex; // [esp+40h] [ebp-124h]
    signed int primarySortKey; // [esp+48h] [ebp-11Ch]
    GfxDrawSurf *drawSurfs[DRAW_SURF_TYPE_COUNT]; // [esp+4Ch] [ebp-118h]
    unsigned int dstStageIndex; // [esp+D8h] [ebp-8Ch]
    unsigned int drawSurfCount[DRAW_SURF_TYPE_COUNT]; // [esp+DCh] [ebp-88h]
    
    iassert(stageCount >= 1 && stageCount <= DRAW_SURF_TYPE_COUNT);

    freeDrawSurfCount = 0x8000 - frontEndDataOut->drawSurfCount;
    if (freeDrawSurfCount > 0)
    {
        dstStageIndex = 0;
        for (srcStageIndex = 0; srcStageIndex < stageCount; ++srcStageIndex)
        {
            stageIndex = srcStageIndex + firstStage;
            if (scene.drawSurfCount[srcStageIndex + firstStage] > freeDrawSurfCount)
            {
                scene.drawSurfCount[stageIndex] = freeDrawSurfCount;
                R_WarnOncePerFrame(R_WARN_MAX_DRAWSURFS);
            }
            if (scene.drawSurfCount[stageIndex])
            {
                freeDrawSurfCount -= scene.drawSurfCount[stageIndex];
                drawSurfCount[dstStageIndex] = scene.drawSurfCount[stageIndex];
                drawSurfs[dstStageIndex++] = scene.drawSurfs[stageIndex];
            }
        }
        while (dstStageIndex)
        {
            stageCount = dstStageIndex;
            if (dstStageIndex == 1)
            {
                R_EmitDrawSurfList(drawSurfs[0], drawSurfCount[0]);
                return;
            }
            primarySortKey = drawSurfs[0]->fields.primarySortKey;
            for (stageIndex = 1; stageIndex < dstStageIndex; ++stageIndex)
            {
                if (drawSurfs[stageIndex]->fields.primarySortKey < primarySortKey)
                    v3 = drawSurfs[stageIndex]->fields.primarySortKey;
                else
                    v3 = primarySortKey;

                primarySortKey = v3;
            }
            dstStageIndex = 0;
            for (srcStageIndex = 0; srcStageIndex < stageCount; ++srcStageIndex)
            {
                v2 = R_EmitDrawSurfListForKey(drawSurfs[srcStageIndex], drawSurfCount[srcStageIndex], primarySortKey); // KISAKTODO: change to blops style
                drawSurfs[dstStageIndex] = &drawSurfs[srcStageIndex][v2];
                drawSurfCount[dstStageIndex] = drawSurfCount[srcStageIndex] - v2;
                dstStageIndex += drawSurfCount[dstStageIndex] != 0;
            }
        }
    }
}

unsigned int __cdecl R_EmitDrawSurfListForKey(
    const GfxDrawSurf *drawSurfs,
    unsigned int drawSurfCount,
    unsigned int primarySortKey)
{
    unsigned int usedCount; // [esp+44h] [ebp-14h]
    GfxDrawSurf drawSurf; // [esp+48h] [ebp-10h]
    GfxDrawSurf *outDrawSurf; // [esp+54h] [ebp-4h]

    PROF_SCOPED("R_EmitDrawSurfList");

    iassert(drawSurfs);
    iassert(drawSurfCount);
    iassert(frontEndDataOut->drawSurfCount + drawSurfCount <= MAX_DRAWSURFS);

    outDrawSurf = &frontEndDataOut->drawSurfs[frontEndDataOut->drawSurfCount];
    usedCount = 0;
    do
    {
        drawSurf = drawSurfs[usedCount];
        if (drawSurf.fields.primarySortKey != primarySortKey)
            break;
        outDrawSurf[usedCount] = drawSurf;
        usedCount++;
    } while (usedCount < drawSurfCount);

    frontEndDataOut->drawSurfCount += usedCount;
    return usedCount;
}


unsigned __int16 *__cdecl R_AllocPreTessIndices(int count)
{
    unsigned __int16 *indices; // [esp+0h] [ebp-4h]

    iassert( gfxBuf.preTessIndexBuffer->indices != NULL );
    iassert( count );
    if (count + gfxBuf.preTessIndexBuffer->used > gfxBuf.preTessIndexBuffer->total)
        return 0;
    indices = &gfxBuf.preTessIndexBuffer->indices[gfxBuf.preTessIndexBuffer->used];
    gfxBuf.preTessIndexBuffer->used += count;
    return indices;
}

void __cdecl R_EndPreTess()
{
    iassert( gfxBuf.preTessIndexBuffer->indices != NULL );
    R_UnlockIndexBuffer(gfxBuf.preTessIndexBuffer->buffer);
    gfxBuf.preTessIndexBuffer->indices = 0;
}

void __cdecl R_BeginPreTess()
{
    iassert( gfxBuf.preTessIndexBuffer->indices == NULL );
    gfxBuf.preTessIndexBuffer->indices = (unsigned __int16 *)R_LockIndexBuffer(
        gfxBuf.preTessIndexBuffer->buffer,
        0,
        2 * gfxBuf.preTessIndexBuffer->total,
        0x2000);
    gfxBuf.preTessIndexBuffer->used = 0;
}

int __cdecl R_ReadBspPreTessDrawSurfs(
    GfxReadCmdBuf *cmdBuf,
    const GfxBspPreTessDrawSurf **list,
    unsigned int *count,
    unsigned int *baseIndex)
{
    *count = R_ReadPrimDrawSurfInt(cmdBuf);
    if (!*count)
        return 0;
    *baseIndex = R_ReadPrimDrawSurfInt(cmdBuf);
    *list = (const GfxBspPreTessDrawSurf *)R_ReadPrimDrawSurfData(cmdBuf, *count);
    return 1;
}