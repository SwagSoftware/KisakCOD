#pragma once

struct PhysicalMemoryAllocation // sizeof=0x8
{                                       // ...
    const char *name;                   // ...
    unsigned int pos;                   // ...
};
struct PhysicalMemoryPrim // sizeof=0x10C
{                                       // ...
    const char *allocName;
    unsigned int allocListCount;        // ...
    unsigned int pos;                   // ...
    PhysicalMemoryAllocation allocList[32]; // ...
};
struct PhysicalMemory // sizeof=0x21C
{                                       // ...
    unsigned __int8 *buf;
    PhysicalMemoryPrim prim[2];         // ...
};

void __cdecl PMem_Init();
void __cdecl PMem_DumpMemStats();
void __cdecl PMem_InitPhysicalMemory(PhysicalMemory *pmem, unsigned __int8 *memory, unsigned int memorySize);
void __cdecl PMem_BeginAlloc(const char *name, unsigned int allocType);
void __cdecl PMem_BeginAllocInPrim(PhysicalMemoryPrim *prim, const char *name);
void __cdecl PMem_EndAlloc(const char *name, unsigned int allocType);
void __cdecl PMem_EndAllocInPrim(PhysicalMemoryPrim *prim, const char *name);
void __cdecl PMem_Free(const char *name, unsigned int allocType);
void __cdecl PMem_FreeInPrim(PhysicalMemoryPrim *prim, const char *name);
void __cdecl PMem_FreeIndex(PhysicalMemoryPrim *prim, unsigned int allocIndex);
int __cdecl PMem_GetOverAllocatedSize();
unsigned __int8 *__cdecl PMem_Alloc(
    unsigned int size,
    unsigned int alignment,
    unsigned int type,
    unsigned int allocType);
unsigned int __cdecl PMem_GetFreeAmount();
void __cdecl PMem_DumpMemStats();
