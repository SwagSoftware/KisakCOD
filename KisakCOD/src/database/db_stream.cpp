#include "database.h"

unsigned int g_streamDelayIndex;
XBlock * g_streamBlocks;
unsigned __int8 *g_streamPosArray[9];
StreamDelayInfo g_streamDelayArray[4096];
unsigned int g_streamPosIndex;
StreamPosInfo g_streamPosStack[64];
XZoneMemory *g_streamZoneMem;
unsigned __int8 *g_streamPos;
unsigned int g_streamPosStackIndex;

void __cdecl DB_InitStreams(XZoneMemory *zoneMem)
{
    int i; // [esp+0h] [ebp-4h]

    g_streamZoneMem = zoneMem;
    g_streamPos = zoneMem->blocks[0].data;
    g_streamPosIndex = 0;
    g_streamDelayIndex = 0;
    g_streamPosStackIndex = 0;
    for (i = 0; i < 9; ++i)
        g_streamPosArray[i] = zoneMem->blocks[i].data;
}

void __cdecl DB_PushStreamPos(unsigned int index)
{
    if (index >= 9)
        MyAssertHandler(".\\database\\db_stream.cpp", 82, 0, "%s", "index < ARRAY_COUNT( g_streamPosArray )");
    if (g_streamPosIndex >= 9)
        MyAssertHandler(".\\database\\db_stream.cpp", 83, 0, "%s", "g_streamPosIndex < ARRAY_COUNT( g_streamPosArray )");
    if (g_streamPosStackIndex >= 0x40)
        MyAssertHandler(
            ".\\database\\db_stream.cpp",
            84,
            0,
            "%s",
            "g_streamPosStackIndex < ARRAY_COUNT( g_streamPosStack )");
    g_streamPosStack[g_streamPosStackIndex].index = g_streamPosIndex;
    DB_SetStreamIndex(index);

    g_streamPosStack[g_streamPosStackIndex++].pos = g_streamPos;

    // lmao compiler???
    //*(&g_streamPosIndex + 2 * g_streamPosStackIndex) = (unsigned int)g_streamPos;
    // == (char*)&g_streamPosIndex + 8 * g_streamPosStackIndex
    // == (char*)&g_streamPosIndex + 8 + 8 * (g_streamPosStackIndex - 1)
    // == (char*)&g_streamPosStack + 8 * (g_streamPosStackIndex - 1)
    // == &g_streamPosStack[g_streamPosStackIndex - 1].pos
}

void __cdecl DB_CloneStreamData(unsigned __int8 *destStart)
{
    if (destStart)
        memcpy(
            &destStart[g_streamPosArray[g_streamPosIndex] - g_streamZoneMem->blocks[g_streamPosIndex].data],
            g_streamPosArray[g_streamPosIndex],
            g_streamPos - g_streamPosArray[g_streamPosIndex]);
}

void __cdecl DB_SetStreamIndex(unsigned int index)
{
    if (index != g_streamPosIndex)
    {
        if (g_streamPosIndex == 7)
        {
            DB_CloneStreamData(g_streamZoneMem->lockedVertexData);
        }
        else if (g_streamPosIndex == 8)
        {
            DB_CloneStreamData(g_streamZoneMem->lockedIndexData);
        }
        iassert(index < arr_cnt(g_streamPosArray));
        g_streamPosArray[g_streamPosIndex] = g_streamPos;
        g_streamPosIndex = index;
        g_streamPos = g_streamPosArray[index];
    }
}

void __cdecl DB_PopStreamPos()
{
    vassert(g_streamPosStackIndex > 0, "(g_streamPosStackIndex = %d)", g_streamPosStackIndex);
    --g_streamPosStackIndex;
    if (!g_streamPosIndex)
        g_streamPos = g_streamPosStack[g_streamPosStackIndex].pos;
    DB_SetStreamIndex(g_streamPosStack[g_streamPosStackIndex].index);
}

unsigned __int8 *__cdecl DB_GetStreamPos()
{
    return g_streamPos;
}

unsigned __int8 *__cdecl DB_AllocStreamPos(int alignment)
{
    g_streamPos = (unsigned __int8 *)(~alignment & (unsigned int)&g_streamPos[alignment]);
    return g_streamPos;
}

void __cdecl DB_IncStreamPos(int size)
{
    iassert(g_streamPos);
    if (&g_streamPos[size] > &g_streamZoneMem->blocks[g_streamPosIndex].data[g_streamZoneMem->blocks[g_streamPosIndex].size])
        MyAssertHandler(
            ".\\database\\db_stream.cpp",
            135,
            0,
            "%s",
            "g_streamPos + size <= g_streamZoneMem->blocks[g_streamPosIndex].data + g_streamZoneMem->blocks[g_streamPosIndex].size");
    g_streamPos += size;
}

const void **__cdecl DB_InsertPointer()
{
    const void **pData; // [esp+0h] [ebp-4h]

    DB_PushStreamPos(4u);
    pData = (const void **)DB_AllocStreamPos(3);
    DB_IncStreamPos(4);
    DB_PopStreamPos();
    return pData;
}

