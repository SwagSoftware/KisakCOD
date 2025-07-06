#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __fastcall TRACK_actor_event_listener();
void __fastcall Actor_EventListener_Init();
void __fastcall Actor_EventListener_SetCount(int listenerCount);
int __fastcall Actor_EventListener_GetCount();
int __fastcall Actor_FindEventFromString(unsigned __int16 eventString);
void __fastcall Actor_EventListener_Add(int entIndex, unsigned __int16 eventString);
void __fastcall RemoveSwapWithLast(unsigned int listenerIndex);
void __fastcall Actor_EventListener_Remove(int entIndex, unsigned __int16 eventString);
void __fastcall Actor_EventListener_RemoveEntity(int entIndex);
int __fastcall Actor_EventListener_Next(int index, int event, int teamFlags);
gentity_s *__fastcall Actor_EventListener_GetEntity(unsigned int index);
void __fastcall Actor_EventListener_NotifyToListener(
    gentity_s *listener,
    gentity_s *originator,
    ai_event_t event,
    const float *position);
void __fastcall Actor_DumpEventListners();
int __fastcall Actor_EventListener_First(int event, int teamFlags);
