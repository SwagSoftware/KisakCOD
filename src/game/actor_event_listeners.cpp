#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_event_listeners.h"

#include <universal/q_shared.h>
#include <qcommon/mem_track.h>

#include <script/scr_const.h>
#include <qcommon/qcommon.h>
#include "g_local.h"
#include <script/scr_vm.h>
#include "g_main.h"

AIEventListener g_AIEVlisteners[32];

unsigned __int16 *g_AIEV_scrConst_table[23] =
{
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  &scr_const.explode,
  &scr_const.grenadedanger,
  &scr_const.grenadedanger,
  &scr_const.gunshot,
  &scr_const.silenced_shot,
  NULL,
  NULL,
  &scr_const.bulletwhizby,
  &scr_const.projectile_impact,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

int g_listenerCount;

void __cdecl TRACK_actor_event_listener()
{
    track_static_alloc_internal(g_AIEVlisteners, 256, "g_AIEVlisteners", 5);
    track_static_alloc_internal(g_AIEV_scrConst_table, 92, "g_AIEV_scrConst_table", 5);
}

void __cdecl Actor_EventListener_Init()
{
    unsigned int *p_events; // r11

    g_listenerCount = 0;
    p_events = &g_AIEVlisteners[0].events;
    do
    {
        *(p_events - 1) = 2175;
        *p_events = 0;
        p_events += 2;
    } while ((int)p_events < (int)&g_AIEVlisteners[32]);
}

void __cdecl Actor_EventListener_SetCount(int listenerCount)
{
    g_listenerCount = listenerCount;
}

int __cdecl Actor_EventListener_GetCount()
{
    return g_listenerCount;
}

int __cdecl Actor_FindEventFromString(unsigned __int16 eventString)
{
    int v1; // r9
    unsigned __int16 **v2; // r11
    const char *v3; // r3
    const char *v4; // r3

    v1 = 0;
    v2 = g_AIEV_scrConst_table;
    while (!*v2 || **v2 != eventString)
    {
        ++v2;
        ++v1;
        if ((int)v2 >= (int)&g_AIEV_scrConst_table[23])
        {
            v3 = SL_ConvertToString(eventString);
            v4 = va("Unable to find AI event for [%s]", v3);
            Scr_Error(v4);
            return 0;
        }
    }
    return v1;
}

void __cdecl Actor_EventListener_Add(int entIndex, unsigned __int16 eventString)
{
    int EventFromString; // r29
    int v4; // r11
    AIEventListener *v5; // r8
    const char *v6; // r3
    unsigned int *p_events; // r3

    EventFromString = Actor_FindEventFromString(eventString);
    if (EventFromString >= 23)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_event_listeners.cpp",
            125,
            0,
            "%s",
            "event < AI_EV_NUM_EVENTS");
    if (EventFromString)
    {
        v4 = 0;
        if (g_listenerCount <= 0)
        {
        LABEL_8:
            if (g_listenerCount < 32)
            {
                p_events = &g_AIEVlisteners[g_listenerCount].events;
                g_AIEVlisteners[g_listenerCount].entIndex = entIndex;
                Com_BitSetAssert(p_events, EventFromString, 0xFFFFFFF);
                ++g_listenerCount;
            }
            else
            {
                v6 = va("Max listeners exceeded; entity id: %d\n", entIndex);
                Scr_Error(v6);
            }
        }
        else
        {
            v5 = g_AIEVlisteners;
            while (v5->entIndex != entIndex)
            {
                ++v4;
                ++v5;
                if (v4 >= g_listenerCount)
                    goto LABEL_8;
            }
            Com_BitSetAssert(&g_AIEVlisteners[v4].events, EventFromString, 0xFFFFFFF);
        }
    }
}

void __cdecl RemoveSwapWithLast(unsigned int listenerIndex)
{
    unsigned int v2; // r7
    int v3; // r10
    unsigned int v5; // r8
    unsigned int v6; // r10

    if (listenerIndex >= 0x20)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_event_listeners.cpp",
            161,
            0,
            "listenerIndex doesn't index MAX_AI_EV_LISTENERS\n\t%i not in [0, %i)",
            listenerIndex,
            32);
    v2 = listenerIndex;
    v3 = g_listenerCount - 1;
    if ((int)listenerIndex < --g_listenerCount)
    {
        v5 = listenerIndex;
        v2 = v3;
        g_AIEVlisteners[v5].entIndex = g_AIEVlisteners[v3].entIndex;
        g_AIEVlisteners[v5].events = g_AIEVlisteners[v3].events;
    }
    v6 = v2;
    g_AIEVlisteners[v6].entIndex = 2175;
    g_AIEVlisteners[v6].events = 0;
}

void __cdecl Actor_EventListener_Remove(int entIndex, unsigned __int16 eventString)
{
    int EventFromString; // r4
    unsigned int v4; // r31
    AIEventListener *i; // r11

    EventFromString = Actor_FindEventFromString(eventString);
    v4 = 0;
    if (g_listenerCount > 0)
    {
        for (i = g_AIEVlisteners; i->entIndex != entIndex; ++i)
        {
            if ((int)++v4 >= g_listenerCount)
                return;
        }
        Com_BitClearAssert(&g_AIEVlisteners[v4].events, EventFromString, 0xFFFFFFF);
        if (!g_AIEVlisteners[v4].events)
            RemoveSwapWithLast(v4);
    }
}

void __cdecl Actor_EventListener_RemoveEntity(int entIndex)
{
    unsigned int v1; // r10
    AIEventListener *i; // r11

    v1 = 0;
    if (g_listenerCount > 0)
    {
        for (i = g_AIEVlisteners; i->entIndex != entIndex; ++i)
        {
            if ((int)++v1 >= g_listenerCount)
                return;
        }
        RemoveSwapWithLast(v1);
    }
}

int __cdecl Actor_EventListener_Next(int index, int event, int teamFlags)
{
    int v3; // r30
    AIEventListener *i; // r31
    sentient_s *sentient; // r11

    v3 = index + 1;
    if (index + 1 >= g_listenerCount)
        return -1;
    for (i = &g_AIEVlisteners[v3]; ; ++i)
    {
        if (Com_BitCheckAssert(&i->events, event, 0xFFFFFFF))
        {
            sentient = g_entities[i->entIndex].sentient;
            if (!sentient || ((1 << sentient->eTeam) & teamFlags) != 0)
                break;
        }
        if (++v3 >= g_listenerCount)
            return -1;
    }
    return v3;
}

gentity_s *__cdecl Actor_EventListener_GetEntity(unsigned int index)
{
    if (index >= 0x20)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_event_listeners.cpp",
            272,
            0,
            "index doesn't index MAX_AI_EV_LISTENERS\n\t%i not in [0, %i)",
            index,
            32);
    return &g_entities[g_AIEVlisteners[index].entIndex];
}

void __cdecl Actor_EventListener_NotifyToListener(
    gentity_s *listener,
    gentity_s *originator,
    ai_event_t event,
    const float *position)
{
    if (!listener)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_event_listeners.cpp", 287, 0, "%s", "listener");
    if (listener->r.inuse)
    {
        Scr_AddVector(position);
        if (originator)
            Scr_AddEntity(originator);
        else
            Scr_AddUndefined();
        if (!g_AIEV_scrConst_table[event])
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_event_listeners.cpp",
                299,
                0,
                "%s",
                "g_AIEV_scrConst_table[event]");
        Scr_Notify(listener, *g_AIEV_scrConst_table[event], 2u);
    }
}

void __cdecl Actor_DumpEventListners()
{
    int v0; // r30
    AIEventListener *v1; // r31
    unsigned int events; // r23
    const char *EntityTypeName; // r3

    Com_Printf(18, "AIEventListners: %d, level time: %d\n", g_listenerCount, level.time);
    Com_Printf(18, "==================\n", g_listenerCount);
    v0 = 0;
    if (g_listenerCount > 0)
    {
        v1 = g_AIEVlisteners;
        do
        {
            if (v1->entIndex >= 2176)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_event_listeners.cpp",
                    313,
                    0,
                    "%s",
                    "g_AIEVlisteners[i].entIndex < MAX_GENTITIES");
            events = v1->events;
            EntityTypeName = G_GetEntityTypeName(&g_entities[v1->entIndex]);
            Com_Printf(18, "%d entity: %04d (%s), events: %x\n", v0++, v1->entIndex, EntityTypeName, events);
            ++v1;
        } while (v0 < g_listenerCount);
    }
}

int __cdecl Actor_EventListener_First(int event, int teamFlags)
{
    return Actor_EventListener_Next(-1, event, teamFlags);
}

