#include "physicalmemory.h"

#include <Windows.h>
#include "assertive.h"
#include <qcommon/mem_track.h>
#include "q_shared.h"
#include <qcommon/qcommon.h>

PhysicalMemory g_mem;
int g_overAllocatedSize;

void __cdecl PMem_Init()
{
    unsigned __int8 *memory; // [esp+0h] [ebp-4h]

    memory = (unsigned __int8 *)VirtualAlloc(0, 0x8000000u, 0x1000u, 4u);
    PMem_InitPhysicalMemory(&g_mem, memory, 0x8000000u);
}

void __cdecl PMem_InitPhysicalMemory(PhysicalMemory *pmem, unsigned __int8 *memory, unsigned int memorySize)
{
    if (!pmem)
        MyAssertHandler(".\\universal\\physicalmemory.cpp", 277, 0, "%s", "pmem");
    if (!memory)
        MyAssertHandler(".\\universal\\physicalmemory.cpp", 278, 0, "%s", "memory");
    memset((unsigned __int8 *)pmem, 0, sizeof(PhysicalMemory));
    pmem->buf = memory;
    pmem->prim[1].pos = memorySize;
}

void __cdecl PMem_BeginAlloc(const char *name, unsigned int allocType)
{
    if (allocType >= 2)
        MyAssertHandler(
            ".\\universal\\physicalmemory.cpp",
            350,
            0,
            "allocType doesn't index PHYS_ALLOC_COUNT\n\t%i not in [0, %i)",
            allocType,
            2);
    PMem_BeginAllocInPrim(&g_mem.prim[allocType], name);
}

void __cdecl PMem_BeginAllocInPrim(PhysicalMemoryPrim *prim, const char *name)
{
    PhysicalMemoryAllocation *allocEntry; // [esp+0h] [ebp-4h]

    if (prim->allocName)
        MyAssertHandler(".\\universal\\physicalmemory.cpp", 332, 0, "%s", "!prim->allocName");
    if (prim->allocListCount >= 0x20)
        MyAssertHandler(".\\universal\\physicalmemory.cpp", 333, 0, "%s", "prim->allocListCount < MAX_PHYSICAL_ALLOCATIONS");
    prim->allocName = name;
    allocEntry = &prim->allocList[prim->allocListCount++];
    allocEntry->name = name;
    allocEntry->pos = prim->pos;
}

void __cdecl PMem_EndAlloc(const char *name, unsigned int allocType)
{
    if (allocType >= 2)
        MyAssertHandler(
            ".\\universal\\physicalmemory.cpp",
            378,
            0,
            "allocType doesn't index PHYS_ALLOC_COUNT\n\t%i not in [0, %i)",
            allocType,
            2);
    PMem_EndAllocInPrim(&g_mem.prim[allocType], name);
}

void __cdecl PMem_EndAllocInPrim(PhysicalMemoryPrim *prim, const char *name)
{
    __int64 v2; // rax

    if (prim->allocName != name)
        MyAssertHandler(".\\universal\\physicalmemory.cpp", 364, 0, "%s", "prim->allocName == name");
    prim->allocName = 0;
    if (!prim->allocListCount)
        MyAssertHandler(".\\universal\\physicalmemory.cpp", 368, 0, "%s", "prim->allocListCount > 0");
    v2 = prim->pos - *(&prim->pos + 2 * prim->allocListCount);
    //track_physical_alloc((HIunsigned int(v2) ^ v2) - HIunsigned int(v2), name, 10);
}

void __cdecl PMem_Free(const char *name, unsigned int allocType)
{
    if (allocType >= 2)
        MyAssertHandler(
            ".\\universal\\physicalmemory.cpp",
            454,
            0,
            "allocType doesn't index PHYS_ALLOC_COUNT\n\t%i not in [0, %i)",
            allocType,
            2);
    PMem_FreeInPrim(&g_mem.prim[allocType], name);
}

void __cdecl PMem_FreeInPrim(PhysicalMemoryPrim *prim, const char *name)
{
    unsigned int allocIndex; // [esp+0h] [ebp-8h]

    for (allocIndex = 0; allocIndex < prim->allocListCount; ++allocIndex)
    {
        if (prim->allocList[allocIndex].name == name)
        {
            PMem_FreeIndex(prim, allocIndex);
            return;
        }
    }
}

void __cdecl PMem_FreeIndex(PhysicalMemoryPrim *prim, unsigned int allocIndex)
{
    __int64 v2; // rax
    const char *v3; // eax
    __int64 v4; // rax
    PhysicalMemoryAllocation *allocEntry; // [esp+0h] [ebp-Ch]
    const char *name; // [esp+4h] [ebp-8h]

    if (prim->allocName)
        MyAssertHandler(".\\universal\\physicalmemory.cpp", 394, 0, "%s", "!prim->allocName");
    allocEntry = &prim->allocList[allocIndex];
    name = allocEntry->name;
    if (!allocEntry->name)
        MyAssertHandler(".\\universal\\physicalmemory.cpp", 400, 0, "%s", "name");
    allocEntry->name = 0;
    if (allocIndex == prim->allocListCount - 1)
    {
        v4 = prim->pos - prim->allocList[allocIndex].pos;
        //track_physical_alloc(HIunsigned int(v4) - (HIunsigned int(v4) ^ v4), name, 10);
        do
        {
            prim->pos = allocEntry->pos;
            if (!prim->allocListCount)
                MyAssertHandler(".\\universal\\physicalmemory.cpp", 424, 0, "%s", "prim->allocListCount");
            if (!--prim->allocListCount)
                break;
            allocEntry = (PhysicalMemoryAllocation *)(&prim->allocListCount + 2 * prim->allocListCount);
        } while (!allocEntry->name);
    }
    else
    {
        if (allocIndex + 1 >= prim->allocListCount)
            MyAssertHandler(".\\universal\\physicalmemory.cpp", 408, 0, "%s", "allocIndex + 1 < prim->allocListCount");
        v2 = prim->allocList[allocIndex + 1].pos - prim->allocList[allocIndex].pos;
        //track_physical_alloc(HIunsigned int(v2) - (HIunsigned int(v2) ^ v2), name, 10);
        if (!alwaysfails)
        {
            v3 = va("freeing '%s' caused a memory hole\n", name);
            MyAssertHandler(".\\universal\\physicalmemory.cpp", 411, 0, v3);
        }
    }
}

int __cdecl PMem_GetOverAllocatedSize()
{
    return g_overAllocatedSize;
}

unsigned __int8 *__cdecl PMem_Alloc(
    unsigned int size,
    unsigned int alignment,
    unsigned int type,
    unsigned int allocType)
{
    PhysicalMemoryPrim *prim; // [esp+10h] [ebp-Ch]
    unsigned int lowPos; // [esp+14h] [ebp-8h]
    unsigned int alignmenta; // [esp+28h] [ebp+Ch]

    prim = &g_mem.prim[allocType];
    if (!prim->allocName)
        MyAssertHandler(".\\universal\\physicalmemory.cpp", 536, 0, "%s", "prim->allocName");
    if (!size)
        MyAssertHandler(".\\universal\\physicalmemory.cpp", 537, 0, "%s", "size");
    if (!alignment)
        MyAssertHandler(".\\universal\\physicalmemory.cpp", 539, 0, "%s", "alignment");
    alignmenta = alignment - 1;
    if (allocType)
    {
        if (allocType != 1)
            MyAssertHandler(".\\universal\\physicalmemory.cpp", 633, 0, "%s", "allocType == PHYS_ALLOC_HIGH");
        lowPos = ~alignmenta & (g_mem.prim[allocType].pos - size);
        g_overAllocatedSize = g_mem.prim[0].pos - lowPos;
        if (g_overAllocatedSize > 0)
            Sys_OutOfMemErrorInternal(".\\universal\\physicalmemory.cpp", 641);
        g_mem.prim[allocType].pos = lowPos;
    }
    else
    {
        lowPos = ~alignmenta & (alignmenta + prim->pos);
        g_overAllocatedSize = size + lowPos - g_mem.prim[1].pos;
        if (g_overAllocatedSize > 0)
        {
            Com_PrintError(16, "Need %i more bytes of ram for alloc to succeed\n", g_overAllocatedSize);
            Sys_OutOfMemErrorInternal(".\\universal\\physicalmemory.cpp", 608);
        }
        g_mem.prim[allocType].pos = size + lowPos;
    }
    return &g_mem.buf[lowPos];
}

unsigned int __cdecl PMem_GetFreeAmount()
{
    return g_mem.prim[1].pos - g_mem.prim[0].pos;
}

void __cdecl PMem_DumpMemStats()
{
    double v0; // st7
    int FreeAmount; // eax
    double v2; // st7
    double v3; // st7
    signed int j; // [esp+8h] [ebp-14h]
    unsigned int i; // [esp+Ch] [ebp-10h]
    unsigned int top; // [esp+14h] [ebp-8h]
    unsigned int bottom; // [esp+18h] [ebp-4h]

    for (i = 0; i < g_mem.prim[1].allocListCount; ++i)
    {
        if (i == g_mem.prim[1].allocListCount - 1)
            bottom = g_mem.prim[1].pos;
        else
            bottom = g_mem.prim[1].allocList[i + 1].pos;
        v0 = ConvertToMB(g_mem.prim[1].allocList[i].pos - bottom);
        Com_Printf(16, "%-18.18s %5.1f\n", g_mem.prim[1].allocList[i].name, v0);
    }
    FreeAmount = PMem_GetFreeAmount();
    v2 = ConvertToMB(FreeAmount);
    Com_Printf(16, "free physical      %5.1f\n", v2);
    top = g_mem.prim[0].pos;
    for (j = g_mem.prim[0].allocListCount - 1; j >= 0; --j)
    {
        v3 = ConvertToMB(top - g_mem.prim[0].allocList[j].pos);
        Com_Printf(16, "%-18.18s %5.1f\n", g_mem.prim[0].allocList[j].name, v3);
        top = g_mem.prim[0].allocList[j].pos;
    }
    Com_Printf(16, "------------------------\n");
}

double __cdecl ConvertToMB(int bytes)
{
    return (float)((double)bytes / 1048576.0);
}

