#include "database.h"

#include <universal/physicalmemory.h>
#include <qcommon/qcommon.h>
#include <gfx_d3d/r_buffers.h>

int g_block_mem_type[9] =
{ 0, 1, 1, 2, 1, 1, 2, 2, 2 };

const char *g_block_mem_name[9] =
{
  "temp",
  "runtime",
  "large_runtime",
  "physical_runtime",
  "virtual",
  "large",
  "physical",
  "vertex",
  "index"
};

void __cdecl DB_RecoverGeometryBuffers(XZoneMemory *zoneMem)
{
#ifndef DEDICATED
    unsigned __int8 *lockedData; // [esp+4h] [ebp-4h]
    unsigned __int8 *lockedDataa; // [esp+4h] [ebp-4h]

    if (zoneMem->blocks[7].size)
    {
        iassert(zoneMem->blocks[7].data);
        iassert(zoneMem->vertexBuffer == NULL);
        lockedData = (unsigned __int8 *)R_AllocStaticVertexBuffer((IDirect3DVertexBuffer9 **)&zoneMem->vertexBuffer, zoneMem->blocks[7].size);
        memcpy(lockedData, zoneMem->blocks[7].data, zoneMem->blocks[7].size);
        if (zoneMem->lockedVertexData)
            zoneMem->lockedVertexData = lockedData;
        else
            R_FinishStaticVertexBuffer((IDirect3DVertexBuffer9 *)zoneMem->vertexBuffer);
    }
    if (zoneMem->blocks[8].size)
    {
        iassert(zoneMem->blocks[8].data);
        iassert(zoneMem->indexBuffer == NULL);
        lockedDataa = (unsigned __int8 *)R_AllocStaticIndexBuffer((IDirect3DIndexBuffer9 **)&zoneMem->indexBuffer, zoneMem->blocks[8].size);
        memcpy(lockedDataa, zoneMem->blocks[8].data, zoneMem->blocks[8].size);
        if (zoneMem->lockedIndexData)
            zoneMem->lockedIndexData = lockedDataa;
        else
            R_FinishStaticIndexBuffer((IDirect3DIndexBuffer9 *)zoneMem->indexBuffer);
    }
#endif
}

void __cdecl DB_ReleaseGeometryBuffers(XZoneMemory *zoneMem)
{
#ifndef DEDICATED
    IDirect3DVertexBuffer9 *vb; // [esp+0h] [ebp-8h]
    IDirect3DIndexBuffer9 *ib; // [esp+4h] [ebp-4h]

    if (zoneMem->vertexBuffer)
    {
        vb = (IDirect3DVertexBuffer9 *)zoneMem->vertexBuffer;
        if (zoneMem->lockedVertexData)
            R_UnlockVertexBuffer(vb);
        R_FreeStaticVertexBuffer(vb);
        zoneMem->vertexBuffer = 0;
    }
    if (zoneMem->indexBuffer)
    {
        ib = (IDirect3DIndexBuffer9 *)zoneMem->indexBuffer;
        if (zoneMem->lockedIndexData)
            R_UnlockIndexBuffer(ib);
        R_FreeStaticIndexBuffer(ib);
        zoneMem->indexBuffer = 0;
    }
#endif
}

void __cdecl DB_AllocXZoneMemory(
    unsigned int *blockSize,
    const char *filename,
    XZoneMemory *zoneMem,
    unsigned int allocType)
{
    int OverAllocatedSize; // [esp+20h] [ebp-14h]
    unsigned int blockIndex; // [esp+28h] [ebp-Ch]
    unsigned __int8 *buf; // [esp+2Ch] [ebp-8h]
    unsigned int size; // [esp+30h] [ebp-4h]

    for (blockIndex = 0; blockIndex < 9; ++blockIndex)
    {
        iassert(zoneMem->blocks[blockIndex].size == 0);
        size = blockSize[blockIndex];
        if (size)
        {
            buf = DB_MemAlloc(size, g_block_mem_type[blockIndex], allocType);
            if (!buf)
            {
                OverAllocatedSize = PMem_GetOverAllocatedSize();
                Com_Error(
                    ERR_DROP,
                    "Could not allocate %.2f MB of type '%s' for zone '%s' needed an additional %.2f MB",
                    (double)size * 0.00000095367431640625,
                    g_block_mem_name[blockIndex],
                    filename,
                    (double)OverAllocatedSize * 0.00000095367431640625);
            }
            zoneMem->blocks[blockIndex].size = size;
            zoneMem->blocks[blockIndex].data = buf;
        }
    }
#ifndef DEDICATED
    if (zoneMem->vertexBuffer)
        MyAssertHandler(".\\database\\db_memory.cpp", 104, 0, "%s", "zoneMem->vertexBuffer == NULL");
    if (zoneMem->lockedVertexData)
        MyAssertHandler(".\\database\\db_memory.cpp", 105, 0, "%s", "zoneMem->lockedVertexData == NULL");
    if (zoneMem->blocks[7].size)
        zoneMem->lockedVertexData = (unsigned __int8 *)R_AllocStaticVertexBuffer((IDirect3DVertexBuffer9 **)&zoneMem->vertexBuffer, zoneMem->blocks[7].size);
    if (zoneMem->indexBuffer)
        MyAssertHandler(".\\database\\db_memory.cpp", 110, 0, "%s", "zoneMem->indexBuffer == NULL");
    if (zoneMem->lockedIndexData)
        MyAssertHandler(".\\database\\db_memory.cpp", 111, 0, "%s", "zoneMem->lockedIndexData == NULL");
    if (zoneMem->blocks[8].size)
        zoneMem->lockedIndexData = (unsigned __int8 *)R_AllocStaticIndexBuffer((IDirect3DIndexBuffer9 **)&zoneMem->indexBuffer, zoneMem->blocks[8].size);
#endif
}

unsigned __int8 *__cdecl DB_MemAlloc(unsigned int size, unsigned int type, unsigned int allocType)
{
    if (type <= 1)
        return PMem_Alloc(size + 15, 0x1000u, 4, allocType);
    iassert(type == DM_MEMORY_PHYSICAL);
    return PMem_Alloc(size, 0x1000u, 0x404u, allocType);
}

