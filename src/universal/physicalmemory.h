#pragma once
#include <cstdint>

struct PhysicalMemoryAllocation // sizeof=0x8
{                                       // ...
    const char *name;                   // ...
    uint32_t pos;                   // ...
};
struct PhysicalMemoryPrim // sizeof=0x10C
{                                       // ...
    const char *allocName;
    uint32_t allocListCount;        // ...
    uint32_t pos;                   // ...
    PhysicalMemoryAllocation allocList[32]; // ...
};
struct PhysicalMemory // sizeof=0x21C
{                                       // ...
    uint8_t *buf;
    PhysicalMemoryPrim prim[2];         // ...
};

void __cdecl PMem_Init();
void __cdecl PMem_DumpMemStats();
void __cdecl PMem_InitPhysicalMemory(PhysicalMemory *pmem, uint8_t *memory, uint32_t memorySize);
void __cdecl PMem_BeginAlloc(const char *name, uint32_t allocType);
void __cdecl PMem_BeginAllocInPrim(PhysicalMemoryPrim *prim, const char *name);
void __cdecl PMem_EndAlloc(const char *name, uint32_t allocType);
void __cdecl PMem_EndAllocInPrim(PhysicalMemoryPrim *prim, const char *name);
void __cdecl PMem_Free(const char *name, uint32_t allocType);
void __cdecl PMem_FreeInPrim(PhysicalMemoryPrim *prim, const char *name);
void __cdecl PMem_FreeIndex(PhysicalMemoryPrim *prim, uint32_t allocIndex);
int __cdecl PMem_GetOverAllocatedSize();
uint8_t *__cdecl PMem_Alloc(
    uint32_t size,
    uint32_t alignment,
    uint32_t type,
    uint32_t allocType);
uint32_t __cdecl PMem_GetFreeAmount();
void __cdecl PMem_DumpMemStats();
