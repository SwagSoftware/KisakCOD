#include "database.h"

#include <universal/physicalmemory.h>

int g_block_mem_type[9];
char *g_block_mem_name[9];

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
        if (zoneMem->blocks[blockIndex].size)
            MyAssertHandler(
                ".\\database\\db_memory.cpp",
                91,
                0,
                "zoneMem->blocks[blockIndex].size == 0\n\t%i, %i",
                zoneMem->blocks[blockIndex].size,
                0);
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
    if (zoneMem->vertexBuffer)
        MyAssertHandler(".\\database\\db_memory.cpp", 104, 0, "%s", "zoneMem->vertexBuffer == NULL");
    if (zoneMem->lockedVertexData)
        MyAssertHandler(".\\database\\db_memory.cpp", 105, 0, "%s", "zoneMem->lockedVertexData == NULL");
    if (zoneMem->blocks[7].size)
        zoneMem->lockedVertexData = (unsigned __int8 *)R_AllocStaticVertexBuffer(
            (IDirect3DVertexBuffer9 **)&zoneMem->vertexBuffer,
            zoneMem->blocks[7].size);
    if (zoneMem->indexBuffer)
        MyAssertHandler(".\\database\\db_memory.cpp", 110, 0, "%s", "zoneMem->indexBuffer == NULL");
    if (zoneMem->lockedIndexData)
        MyAssertHandler(".\\database\\db_memory.cpp", 111, 0, "%s", "zoneMem->lockedIndexData == NULL");
    if (zoneMem->blocks[8].size)
        zoneMem->lockedIndexData = (unsigned __int8 *)R_AllocStaticIndexBuffer(
            (IDirect3DIndexBuffer9 **)&zoneMem->indexBuffer,
            zoneMem->blocks[8].size);
}

unsigned __int8 *__cdecl DB_MemAlloc(unsigned int size, unsigned int type, unsigned int allocType)
{
    if (type <= 1)
        return PMem_Alloc(size + 15, 0x1000u, 4u, allocType);
    if (type != 2)
        MyAssertHandler(".\\database\\db_memory.cpp", 73, 0, "%s", "type == DM_MEMORY_PHYSICAL");
    return PMem_Alloc(size, 0x1000u, 0x404u, allocType);
}

