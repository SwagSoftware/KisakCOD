#pragma once
#include <xanim/xanim.h>

struct EntHandleInfo // sizeof=0x8
{                                       // ...
    void *handle;
    unsigned __int16 next;              // ...
    unsigned __int16 prev;              // ...
};
struct EntHandleList // sizeof=0x2
{                                       // ...
    unsigned __int16 infoIndex;
};

void __cdecl EntHandleDissociate(gentity_s *ent);
void __cdecl EntHandleDissociateInternal(EntHandleList *entHandleList);
void __cdecl RemoveEntHandleInfo(EntHandleList *entHandleList, unsigned int oldInfoIndex);
unsigned int __cdecl AddEntHandleInfo(EntHandleList *entHandleList, void *handle);
