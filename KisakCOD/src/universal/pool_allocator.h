#pragma once

struct pooldata_t // sizeof=0x8
{                                       // ...
    void *firstFree;
    int activeCount;
};

struct freenode // sizeof=0x4
{
    freenode *next;
};

void __cdecl Pool_Init(char *pool, pooldata_t *pooldata, unsigned int itemSize, unsigned int itemCount);
freenode *__cdecl Pool_Alloc(pooldata_t *pooldata);
void __cdecl Pool_Free(freenode *data, pooldata_t *pooldata);
unsigned int __cdecl Pool_FreeCount(const pooldata_t *pooldata);
