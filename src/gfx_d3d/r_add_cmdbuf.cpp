#include "r_bsp.h"
#include "r_rendercmds.h"


void __cdecl R_InitDelayedCmdBuf(GfxDelayedCmdBuf *delayedCmdBuf)
{
    delayedCmdBuf->primDrawSurfPos = -1;
    delayedCmdBuf->primDrawSurfSize = 0;
    *(unsigned int *)&delayedCmdBuf->drawSurfKey.fields = -1;
    HIDWORD(delayedCmdBuf->drawSurfKey.packed) = -1;
}

void __cdecl R_EndCmdBuf(GfxDelayedCmdBuf *delayedCmdBuf)
{
    if ((HIDWORD(delayedCmdBuf->drawSurfKey.packed) & *(unsigned int *)&delayedCmdBuf->drawSurfKey.fields) != -1)
    {
        if (!delayedCmdBuf->primDrawSurfSize)
            MyAssertHandler(
                ".\\r_add_cmdbuf.cpp",
                32,
                0,
                "%s\n\t(delayedCmdBuf->primDrawSurfSize) = %i",
                "(delayedCmdBuf->primDrawSurfSize > 0)",
                delayedCmdBuf->primDrawSurfSize);
        *(unsigned int *)&delayedCmdBuf->drawSurfKey.fields = -1;
        HIDWORD(delayedCmdBuf->drawSurfKey.packed) = -1;
        frontEndDataOut->primDrawSurfsBuf[delayedCmdBuf->primDrawSurfPos++] = 0;
        --delayedCmdBuf->primDrawSurfSize;
    }
}

int __cdecl R_AllocDrawSurf(
    GfxDelayedCmdBuf *delayedCmdBuf,
    GfxDrawSurf drawSurf,
    GfxDrawSurfList *drawSurfList,
    unsigned int size)
{
    unsigned int primDrawSurfPos; // [esp+10h] [ebp-4h]

    if (size >= 0x10000)
        MyAssertHandler(".\\r_add_cmdbuf.cpp", 51, 0, "%s\n\t(size) = %i", "(size < (128 * 512))", size);
    if (delayedCmdBuf->drawSurfKey.packed != drawSurf.packed)
        R_EndCmdBuf(delayedCmdBuf);
    if (delayedCmdBuf->primDrawSurfSize > size)
    {
        primDrawSurfPos = delayedCmdBuf->primDrawSurfPos;
    }
    else
    {
        R_EndCmdBuf(delayedCmdBuf);
        primDrawSurfPos = InterlockedExchangeAdd(&frontEndDataOut->primDrawSurfPos, 512);
        if (primDrawSurfPos >= 0x10000)
        {
            delayedCmdBuf->primDrawSurfSize = 0;
            R_WarnOncePerFrame(R_WARN_PRIM_DRAW_SURF_BUFFER_SIZE);
            return 0;
        }
        delayedCmdBuf->primDrawSurfPos = primDrawSurfPos;
        delayedCmdBuf->primDrawSurfSize = 512;
    }
    if (delayedCmdBuf->drawSurfKey.packed == drawSurf.packed)
        return 1;
    if (drawSurfList->current < drawSurfList->end)
    {
        delayedCmdBuf->drawSurfKey = drawSurf;
        if (primDrawSurfPos >= 0x10000)
            MyAssertHandler(
                ".\\r_add_cmdbuf.cpp",
                117,
                0,
                "primDrawSurfPos doesn't index 1 << MTL_SORT_OBJECT_ID_BITS\n\t%i not in [0, %i)",
                primDrawSurfPos,
                0x10000);
        *(unsigned int *)&drawSurf.fields = (unsigned __int16)primDrawSurfPos | *(unsigned int *)&drawSurf.fields & 0xFFFF0000;
        drawSurfList->current->fields = drawSurf.fields;
        ++drawSurfList->current;
        return 1;
    }
    else
    {
        R_WarnOncePerFrame(R_WARN_MAX_SCENE_DRAWSURFS, "R_AllocDrawSurf");
        return 0;
    }
}

void __cdecl R_WritePrimDrawSurfInt(GfxDelayedCmdBuf *delayedCmdBuf, unsigned int value)
{
    if (!delayedCmdBuf->primDrawSurfSize)
        MyAssertHandler(".\\r_add_cmdbuf.cpp", 144, 0, "%s", "delayedCmdBuf->primDrawSurfSize");
    if (delayedCmdBuf->primDrawSurfPos < 0)
        MyAssertHandler(
            ".\\r_add_cmdbuf.cpp",
            145,
            0,
            "%s\n\t(delayedCmdBuf->primDrawSurfPos) = %i",
            "(delayedCmdBuf->primDrawSurfPos >= 0)",
            delayedCmdBuf->primDrawSurfPos);
    --delayedCmdBuf->primDrawSurfSize;
    frontEndDataOut->primDrawSurfsBuf[delayedCmdBuf->primDrawSurfPos++] = value;
}

void __cdecl R_WritePrimDrawSurfData(GfxDelayedCmdBuf *delayedCmdBuf, unsigned __int8 *data, unsigned int count)
{
    if (delayedCmdBuf->primDrawSurfSize < count)
        MyAssertHandler(
            ".\\r_add_cmdbuf.cpp",
            162,
            0,
            "%s\n\t(delayedCmdBuf->primDrawSurfSize) = %i",
            "(delayedCmdBuf->primDrawSurfSize >= count)",
            delayedCmdBuf->primDrawSurfSize);
    if (delayedCmdBuf->primDrawSurfPos < 0)
        MyAssertHandler(".\\r_add_cmdbuf.cpp", 163, 0, "%s", "delayedCmdBuf->primDrawSurfPos >= 0");
    delayedCmdBuf->primDrawSurfSize -= count;
    memcpy((unsigned __int8 *)&frontEndDataOut->primDrawSurfsBuf[delayedCmdBuf->primDrawSurfPos], data, 4 * count);
    delayedCmdBuf->primDrawSurfPos += count;
}

