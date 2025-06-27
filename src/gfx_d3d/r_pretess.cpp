#include "r_pretess.h"
#include <qcommon/threads.h>
#include <universal/q_shared.h>
#include "r_scene.h"
#include "r_buffers.h"
#include "rb_tess.h"
#include "r_draw_staticmodel.h"
#include <universal/profile.h>


void __cdecl R_InitDrawSurfListInfo(GfxDrawSurfListInfo *info)
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\r_pretess.cpp", 74, 0, "%s", "Sys_IsMainThread()");
    info->drawSurfs = 0;
    info->drawSurfCount = 0;
    info->baseTechType = TECHNIQUE_DEPTH_PREPASS;
    info->viewInfo = 0;
    info->viewOrigin[0] = 0.0;
    info->viewOrigin[1] = 0.0;
    info->viewOrigin[2] = 0.0;
    info->viewOrigin[3] = 0.0;
    info->light = 0;
    info->cameraView = 0;
    if (info->light)
        MyAssertHandler(".\\r_pretess.cpp", 77, 0, "%s", "!info->light");
}

void __cdecl R_EmitDrawSurfList(GfxDrawSurf *drawSurfs, unsigned int drawSurfCount)
{
    int newDrawSurfCount; // [esp+4Ch] [ebp-4h]

    PROF_SCOPED("R_EmitDrawSurfList");

    iassert(drawSurfs);

    newDrawSurfCount = drawSurfCount + frontEndDataOut->drawSurfCount;

    if (newDrawSurfCount <= 0x8000)
    {
        Com_Memcpy(
            (char *)&frontEndDataOut->drawSurfs[frontEndDataOut->drawSurfCount],
            (char *)drawSurfs,
            8 * drawSurfCount);
        frontEndDataOut->drawSurfCount = newDrawSurfCount;
    }
    else
    {
        R_WarnOncePerFrame(R_WARN_MAX_DRAWSURFS);
    }
}

void __cdecl R_MergeAndEmitDrawSurfLists(unsigned int firstStage, unsigned int stageCount)
{
    unsigned int v2; // eax
    signed int v3; // [esp+0h] [ebp-164h]
    unsigned int srcStageIndex; // [esp+38h] [ebp-12Ch]
    unsigned int srcStageIndexa; // [esp+38h] [ebp-12Ch]
    int freeDrawSurfCount; // [esp+3Ch] [ebp-128h]
    unsigned int stageIndex; // [esp+40h] [ebp-124h]
    unsigned int stageIndexa; // [esp+40h] [ebp-124h]
    signed int primarySortKey; // [esp+48h] [ebp-11Ch]
    GfxDrawSurf *drawSurfs[34]; // [esp+4Ch] [ebp-118h]
    unsigned int dstStageIndex; // [esp+D8h] [ebp-8Ch]
    unsigned int drawSurfCount[34]; // [esp+DCh] [ebp-88h]
    unsigned int stageCounta; // [esp+170h] [ebp+Ch]

    if (!stageCount || stageCount > 0x22)
        MyAssertHandler(
            ".\\r_pretess.cpp",
            214,
            0,
            "stageCount not in [1, DRAW_SURF_TYPE_COUNT]\n\t%i not in [%i, %i]",
            stageCount,
            1,
            34);
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
            stageCounta = dstStageIndex;
            if (dstStageIndex == 1)
            {
                {
                    PROF_SCOPED("R_EmitDrawSurfList");
                    Com_Memcpy(
                        (char *)&frontEndDataOut->drawSurfs[frontEndDataOut->drawSurfCount],
                        (char *)drawSurfs[0],
                        8 * drawSurfCount[0]);
                    frontEndDataOut->drawSurfCount += drawSurfCount[0];
                }
                return;
            }
            primarySortKey = drawSurfs[0]->fields.primarySortKey;
            for (stageIndexa = 1; stageIndexa < dstStageIndex; ++stageIndexa)
            {
                //if ((int)((drawSurfs[stageIndexa]->packed >> 54) & 0x3F) < primarySortKey)
                if (drawSurfs[stageIndexa]->fields.primarySortKey < primarySortKey)
                    v3 = drawSurfs[stageIndexa]->fields.primarySortKey;
                else
                    v3 = primarySortKey;

                primarySortKey = v3;
            }
            dstStageIndex = 0;
            for (srcStageIndexa = 0; srcStageIndexa < stageCounta; ++srcStageIndexa)
            {
                v2 = R_EmitDrawSurfListForKey(drawSurfs[srcStageIndexa], drawSurfCount[srcStageIndexa], primarySortKey);
                drawSurfs[dstStageIndex] = &drawSurfs[srcStageIndexa][v2];
                drawSurfCount[dstStageIndex] = drawSurfCount[srcStageIndexa] - v2;
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

    if (!drawSurfs)
        MyAssertHandler(".\\r_pretess.cpp", 138, 0, "%s", "drawSurfs");
    if (!drawSurfCount)
        MyAssertHandler(".\\r_pretess.cpp", 139, 0, "%s", "drawSurfCount");
    if (drawSurfCount + frontEndDataOut->drawSurfCount > 0x8000)
        MyAssertHandler(".\\r_pretess.cpp", 140, 0, "%s", "frontEndDataOut->drawSurfCount + drawSurfCount <= MAX_DRAWSURFS");
    outDrawSurf = &frontEndDataOut->drawSurfs[frontEndDataOut->drawSurfCount];
    usedCount = 0;
    do
    {
        drawSurf = drawSurfs[usedCount];
        if (drawSurf.fields.primarySortKey != primarySortKey)
            break;
        outDrawSurf[usedCount++] = drawSurf;
    } while (usedCount < drawSurfCount);

    frontEndDataOut->drawSurfCount += usedCount;
    return usedCount;
}


unsigned __int16 *__cdecl R_AllocPreTessIndices(int count)
{
    unsigned __int16 *indices; // [esp+0h] [ebp-4h]

    if (!gfxBuf.preTessIndexBuffer->indices)
        MyAssertHandler(".\\r_pretess.cpp", 304, 0, "%s", "gfxBuf.preTessIndexBuffer->indices != NULL");
    if (!count)
        MyAssertHandler(".\\r_pretess.cpp", 305, 0, "%s", "count");
    if (count + gfxBuf.preTessIndexBuffer->used > gfxBuf.preTessIndexBuffer->total)
        return 0;
    indices = &gfxBuf.preTessIndexBuffer->indices[gfxBuf.preTessIndexBuffer->used];
    gfxBuf.preTessIndexBuffer->used += count;
    return indices;
}

void __cdecl R_EndPreTess()
{
    if (!gfxBuf.preTessIndexBuffer->indices)
        MyAssertHandler(".\\r_pretess.cpp", 292, 0, "%s", "gfxBuf.preTessIndexBuffer->indices != NULL");
    R_UnlockIndexBuffer(gfxBuf.preTessIndexBuffer->buffer);
    gfxBuf.preTessIndexBuffer->indices = 0;
}

void __cdecl R_BeginPreTess()
{
    if (gfxBuf.preTessIndexBuffer->indices)
        MyAssertHandler(".\\r_pretess.cpp", 280, 0, "%s", "gfxBuf.preTessIndexBuffer->indices == NULL");
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