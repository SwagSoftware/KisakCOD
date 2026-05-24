#pragma once
#include <cstdint>

struct pooldata_t // sizeof=0x8
{                                       // ...
    void *firstFree;
    int activeCount;
};

struct freenode // sizeof=0x4
{
    freenode *next;
};

void __cdecl Pool_Init(char *pool, pooldata_t *pooldata, uint32_t itemSize, uint32_t itemCount);
freenode *__cdecl Pool_Alloc(pooldata_t *pooldata);

void __cdecl Pool_Free(freenode *data, pooldata_t *pooldata);
uint32_t __cdecl Pool_FreeCount(const pooldata_t *pooldata);

#define INIT_STATIC_POOL(array, pooldata) Pool_Init((char*)array, pooldata, sizeof((array)[0]), sizeof(array) / sizeof((array)[0]))