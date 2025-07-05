#pragma once
#include <cstdint>
#include <xanim/xanim.h>

struct EntHandleInfo // sizeof=0x8
{                                       // ...
    void *handle;
    uint16_t next;              // ...
    uint16_t prev;              // ...
};
static_assert(sizeof(EntHandleInfo) == 0x8);

struct EntHandleList // sizeof=0x2
{                                       // ...
    uint16_t infoIndex;
};
static_assert(sizeof(EntHandleList) == 0x2);

struct EntHandle // sizeof=0x4
{                                       // ...
    uint16_t number;
    uint16_t infoIndex;

    static void setEnt(EntHandle *_this, gentity_s *ent);
    static gentity_s *ent(EntHandle *_this);
    static int32_t entnum(EntHandle *_this);
    static bool isDefined(EntHandle *_this);

    static void Init();
    static void Shutdown();
};
static_assert(sizeof(EntHandle) == 0x4);

void __cdecl EntHandleDissociate(gentity_s *ent);
void __cdecl EntHandleDissociateInternal(EntHandleList *entHandleList);
void __cdecl RemoveEntHandleInfo(EntHandleList *entHandleList, uint32_t oldInfoIndex);
uint32_t __cdecl AddEntHandleInfo(EntHandleList *entHandleList, void *handle);
