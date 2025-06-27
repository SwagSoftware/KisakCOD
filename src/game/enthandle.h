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


struct EntHandle // sizeof=0x4
{                                       // ...
    unsigned __int16 number;
    unsigned __int16 infoIndex;

    static void setEnt(EntHandle *_this, gentity_s *ent);
    static gentity_s *ent(EntHandle *_this);
    static int entnum(EntHandle *_this);
    static bool isDefined(EntHandle *_this);

    static void Init();
    static void Shutdown();
};

void __cdecl EntHandleDissociate(gentity_s *ent);
void __cdecl EntHandleDissociateInternal(EntHandleList *entHandleList);
void __cdecl RemoveEntHandleInfo(EntHandleList *entHandleList, unsigned int oldInfoIndex);
unsigned int __cdecl AddEntHandleInfo(EntHandleList *entHandleList, void *handle);
