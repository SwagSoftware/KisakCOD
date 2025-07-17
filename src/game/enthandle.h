#pragma once

#include <universal/q_shared.h>

struct gentity_s;

struct EntHandleInfo // sizeof=0x8 // (SP/MP same)
{                                       // ...
    void *handle;
    uint16_t next;              // ...
    uint16_t prev;              // ...
};
static_assert(sizeof(EntHandleInfo) == 0x8);

struct EntHandleList // sizeof=0x2 // (SP/MP same)
{                                       // ...
    uint16_t infoIndex;
};
static_assert(sizeof(EntHandleList) == 0x2);

struct EntHandle // sizeof=0x4 // (SP/MP same)
{                                       // ...
    uint16_t number;
    uint16_t infoIndex;

    void setEnt(gentity_s *ent);
    gentity_s *ent();
    int32_t entnum();
    bool isDefined();

    static void Init();
    static void Shutdown();
};
static_assert(sizeof(EntHandle) == 0x4);

void __cdecl EntHandleDissociate(gentity_s *ent);
void __cdecl EntHandleDissociateInternal(EntHandleList *entHandleList);
void __cdecl RemoveEntHandleInfo(EntHandleList *entHandleList, uint32_t oldInfoIndex);
uint32_t __cdecl AddEntHandleInfo(EntHandleList *entHandleList, void *handle);


#ifdef KISAK_SP
struct sentient_s;

#define MAX_SENTIENTS 33

struct SentientHandle
{
    unsigned __int16 number;
    unsigned __int16 infoIndex;

    sentient_s *sentient();
    void setSentient(sentient_s *sentient);

    static void Init();
    bool isDefined();
};

void SentientHandleDissociate(sentient_s *sentient);
#endif