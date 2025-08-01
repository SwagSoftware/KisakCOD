#pragma once
#include "actor_events.h"

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

struct AIEventListener
{
    int entIndex;
    unsigned int events;
};

void __cdecl TRACK_actor_event_listener();
void __cdecl Actor_EventListener_Init();
void __cdecl Actor_EventListener_SetCount(int listenerCount);
int __cdecl Actor_EventListener_GetCount();
int __cdecl Actor_FindEventFromString(unsigned __int16 eventString);
void __cdecl Actor_EventListener_Add(int entIndex, unsigned __int16 eventString);
void __cdecl RemoveSwapWithLast(unsigned int listenerIndex);
void __cdecl Actor_EventListener_Remove(int entIndex, unsigned __int16 eventString);
void __cdecl Actor_EventListener_RemoveEntity(int entIndex);
int __cdecl Actor_EventListener_Next(int index, int event, int teamFlags);
gentity_s *__cdecl Actor_EventListener_GetEntity(unsigned int index);
void __cdecl Actor_EventListener_NotifyToListener(
    gentity_s *listener,
    gentity_s *originator,
    ai_event_t event,
    const float *position);
void __cdecl Actor_DumpEventListners();
int __cdecl Actor_EventListener_First(int event, int teamFlags);

extern AIEventListener g_AIEVlisteners[32];