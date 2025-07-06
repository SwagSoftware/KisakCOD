#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_state.h"

void __fastcall TRACK_actor_state()
{
    track_static_alloc_internal((void *)g_eSimplificationRules, 64, "g_eSimplificationRules", 5);
    track_static_alloc_internal((void *)g_eSupercedingStates, 64, "g_eSupercedingStates", 5);
    track_static_alloc_internal((void *)g_eSupercededStates, 64, "g_eSupercededStates", 5);
}

void __fastcall Actor_SetDefaultState(actor_s *actor)
{
    if (actor->stateLevel)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp", 110, 0, "%s", "actor->stateLevel == 0");
    if (actor->simulatedStateLevel)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
            111,
            0,
            "%s",
            "actor->simulatedStateLevel == 0");
    if (actor->transitionCount)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp", 112, 0, "%s", "actor->transitionCount == 0");
    actor->eState[0] = AIS_EXPOSED;
    actor->eSimulatedState[0] = AIS_EXPOSED;
}

int __fastcall Actor_StartState(actor_s *self, ai_state_t eStartedState)
{
    if (!self->stateLevel)
        self->iStateTime = level.time;
    if (!AIFuncTable[self->species][eStartedState].pfnStart)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
            131,
            0,
            "%s\n\t(eStartedState) = %i",
            "(AIFuncTable[self->species][eStartedState].pfnStart)",
            eStartedState);
    self->eState[self->stateLevel] = eStartedState;
    return ((int(__fastcall *)(actor_s *, ai_state_t))AIFuncTable[self->species][eStartedState].pfnStart)(
        self,
        self->eState[self->stateLevel]);
}

void __fastcall Actor_FinishState(actor_s *self, ai_state_t eNextState)
{
    ai_state_t v4; // r8
    ai_state_t v5; // r29

    v4 = self->eState[self->stateLevel];
    v5 = v4;
    if (!AIFuncTable[self->species][v4].pfnFinish)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
            150,
            0,
            "%s\n\t(eCurState) = %i",
            "(AIFuncTable[self->species][eCurState].pfnFinish)",
            v4);
    AIFuncTable[self->species][v5].pfnFinish(self, eNextState);
}

void __fastcall Actor_SuspendState(actor_s *self, ai_state_t eNextState)
{
    ai_state_t v4; // r8
    ai_state_t v5; // r29

    v4 = self->eState[self->stateLevel];
    v5 = v4;
    if (!AIFuncTable[self->species][v4].pfnSuspend)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
            168,
            0,
            "%s\n\t(eCurState) = %i",
            "(AIFuncTable[self->species][eCurState].pfnSuspend)",
            v4);
    AIFuncTable[self->species][v5].pfnSuspend(self, eNextState);
}

int __fastcall Actor_GetNextPopedState(actor_s *self)
{
    unsigned int stateLevel; // r11

    stateLevel = self->stateLevel;
    if (stateLevel)
        return *((unsigned int *)&self->species + stateLevel);
    else
        return 1;
}

int __fastcall Actor_ResumeState(actor_s *self, ai_state_t ePrevState)
{
    ai_state_t v4; // r8
    ai_state_t v5; // r29
    unsigned int stateLevel; // r11

    v4 = self->eState[self->stateLevel];
    v5 = v4;
    if (!AIFuncTable[self->species][v4].pfnResume)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
            205,
            0,
            "%s\n\t(eCurState) = %i",
            "(AIFuncTable[self->species][eCurState].pfnResume)",
            v4);
    if (AIFuncTable[self->species][v5].pfnResume(self, ePrevState))
        return 1;
    stateLevel = self->stateLevel;
    if (stateLevel)
        Actor_FinishState(self, *((ai_state_t *)&self->species + stateLevel));
    else
        Actor_FinishState(self, AIS_EXPOSED);
    return 0;
}

void __fastcall Actor_StartDefaultState(actor_s *self)
{
    if (!(unsigned __int8)Actor_StartState(self, AIS_EXPOSED))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp", 228, 0, "%s", "startSuccess");
}

void __fastcall Actor_ResumePopedState(actor_s *self, ai_state_t state)
{
    unsigned int stateLevel; // r11
    char v4; // r3

    stateLevel = self->stateLevel;
    while (stateLevel)
    {
        self->stateLevel = stateLevel - 1;
        v4 = Actor_ResumeState(self, state);
        stateLevel = self->stateLevel;
        state = self->eState[stateLevel];
        if (v4)
            return;
    }
    if (!(unsigned __int8)Actor_StartState(self, AIS_EXPOSED))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp", 228, 0, "%s", "startSuccess");
}

void __fastcall Actor_ThinkStateTransitions(actor_s *self)
{
    unsigned int v2; // r19
    ai_state_t *p_eState; // r29
    unsigned int stateLevel; // r7
    ai_state_t v5; // r8
    int v6; // r4
    const char *v7; // r3
    unsigned int v8; // r9
    unsigned int v9; // r10
    ai_state_t *eSimulatedState; // r11

    if (!self->transitionCount)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp", 268, 0, "%s", "self->transitionCount > 0");
    v2 = 0;
    if (self->transitionCount)
    {
        p_eState = &self->StateTransitions[0].eState;
        do
        {
            stateLevel = self->stateLevel;
            if (stateLevel >= 5)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
                    275,
                    0,
                    "self->stateLevel doesn't index ARRAY_COUNT( self->eState )\n\t%i not in [0, %i)",
                    stateLevel,
                    5);
            v5 = self->eState[self->stateLevel];
            if (v5 <= AIS_INVALID || v5 >= AIS_COUNT)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
                    276,
                    0,
                    "%s\n\t(self->eState[self->stateLevel]) = %i",
                    "(self->eState[self->stateLevel] > AIS_INVALID && self->eState[self->stateLevel] < AIS_COUNT)",
                    v5);
            v6 = *((unsigned int *)p_eState - 1);
            switch (v6)
            {
            case 1:
                Actor_FinishState(self, *p_eState);
                if (!(unsigned __int8)Actor_StartState(self, *p_eState)
                    && !(unsigned __int8)Actor_StartState(self, AIS_EXPOSED))
                {
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp", 228, 0, "%s", "startSuccess");
                }
                break;
            case 2:
                if (self->stateLevel + 1 >= 5)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
                        288,
                        0,
                        "%s",
                        "self->stateLevel + 1 < ARRAY_COUNT( self->eState )");
                Actor_SuspendState(self, *p_eState);
                ++self->stateLevel;
                if (!(unsigned __int8)Actor_StartState(self, *p_eState))
                    Actor_ResumePopedState(self, *p_eState);
                break;
            case 3:
                if (!self->stateLevel)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
                        297,
                        0,
                        "%s",
                        "self->stateLevel > 0");
                Actor_FinishState(self, *((ai_state_t *)&self->species + self->stateLevel));
                Actor_ResumePopedState(self, self->eState[self->stateLevel]);
                break;
            default:
                if (!alwaysfails)
                {
                    v7 = va("invalid state transition %i", v6);
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp", 303, 0, v7);
                }
                break;
            }
            ++v2;
            p_eState += 2;
        } while (v2 < self->transitionCount);
    }
    v8 = self->stateLevel;
    v9 = 0;
    eSimulatedState = self->eSimulatedState;
    self->transitionCount = 0;
    self->simulatedStateLevel = v8;
    do
    {
        ++v9;
        *eSimulatedState = *(eSimulatedState - 36);
        ++eSimulatedState;
    } while (v9 <= self->stateLevel);
}

void __fastcall Actor_SetSubState(actor_s *self, ai_substate_t eSubState)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp", 328, 0, "%s", "self");
    self->eSubState[self->stateLevel] = eSubState;
}

int __fastcall Actor_IsStateOnStack(const actor_s *self, ai_state_t eState)
{
    int v2; // r10
    ai_state_t *i; // r11

    v2 = 0;
    for (i = self->eSimulatedState; *i != eState; ++i)
    {
        if (++v2 > self->simulatedStateLevel)
            return 0;
    }
    return 1;
}

int __fastcall Actor_PendingTransitionTo(actor_s *self, ai_state_t eState)
{
    unsigned int transitionCount; // r8
    int v3; // r10
    ai_transition_cmd_t *i; // r11

    transitionCount = self->transitionCount;
    v3 = 0;
    if (!transitionCount)
        return 0;
    for (i = self->StateTransitions;
        i->eState != eState || i->eTransition != AIS_TRANSITION_SET && i->eTransition != AIS_TRANSITION_PUSH;
        ++i)
    {
        if (++v3 >= transitionCount)
            return 0;
    }
    return 1;
}

void __fastcall Actor_SimplifyStateTransitions(actor_s *self)
{
    unsigned int transitionCount; // r11
    int v3; // r10
    unsigned int v4; // r30
    unsigned int v5; // r29
    ai_substate_t v6; // r11
    unsigned int v7; // r11

    if (self->transitionCount >= 2)
    {
        while (1)
        {
            transitionCount = self->transitionCount;
            v3 = 8 * (transitionCount + 6);
            v4 = *(unsigned int *)((char *)&self->ent + v3);
            v5 = *(&self->iStateTime + 2 * transitionCount);
            if (v4 >= 4)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
                    399,
                    0,
                    "eCmd1 doesn't index ARRAY_COUNT( g_eSimplificationRules )\n\t%i not in [0, %i)",
                    *(gentity_s **)((char *)&self->ent + v3),
                    4);
            if (v5 >= 4)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
                    400,
                    0,
                    "eCmd2 doesn't index ARRAY_COUNT( g_eSimplificationRules[0] )\n\t%i not in [0, %i)",
                    v5,
                    4);
            if (!self->Physics.bIsAlive)
                break;
            v6 = g_eSimplificationRules[v4][v5];
            if (v6 != -1)
            {
                if (v6)
                {
                    self->eSubState[2 * self->transitionCount + 4] = v6;
                    self->eSubState[2 * self->transitionCount + 5] = *(&self->preThinkTime + 2 * self->transitionCount);
                    v7 = self->transitionCount - 1;
                }
                else
                {
                    v7 = self->transitionCount - 2;
                }
                self->transitionCount = v7;
                if (v7 >= 2)
                    continue;
            }
            return;
        }
        Com_PrintWarning(18, "WARNING: ignoring AI state transition on entnum %d (actor is dead)\n", self->ent->s.number);
        self->transitionCount = 1;
    }
}

int __fastcall Actor_AllowedToPushState(actor_s *self, int eState)
{
    unsigned int simulatedStateLevel; // r5
    unsigned int v5; // r6
    ai_state_t *v6; // r9
    unsigned int v7; // r10
    const ai_state_t *v8; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp", 439, 0, "%s", "self");
    if (eState < 7 || eState > 10)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
            440,
            0,
            "%s",
            "eState >= AIS_PUSHABLE_FIRST && eState <= AIS_PUSHABLE_LAST");
    simulatedStateLevel = self->simulatedStateLevel;
    v5 = 1;
    if (simulatedStateLevel)
    {
        v6 = &self->eSimulatedState[1];
        while (2)
        {
            v7 = 0;
            v8 = g_eSupercedingStates[eState - 7];
            do
            {
                if (*v6 == *v8)
                    return 0;
                ++v7;
                ++v8;
            } while (v7 < 4);
            ++v5;
            ++v6;
            if (v5 <= simulatedStateLevel)
                continue;
            break;
        }
    }
    return 1;
}

// attributes: thunk
void __fastcall Actor_OnStateChange(actor_s *self)
{
    Actor_ClearArrivalPos(self);
}

void __fastcall Actor_SetState(actor_s *self, ai_state_t eState)
{
    unsigned int transitionCount; // r7
    unsigned int simulatedStateLevel; // r4
    const char *v6; // r3
    int v7; // [sp+8h] [-B8h]
    int v8; // [sp+Ch] [-B4h]
    int v9; // [sp+10h] [-B0h]
    int v10; // [sp+14h] [-ACh]
    int v11; // [sp+18h] [-A8h]

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp", 508, 0, "%s", "self");
    transitionCount = self->transitionCount;
    if (transitionCount >= 0xB)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
            509,
            0,
            "self->transitionCount doesn't index ARRAY_COUNT( self->StateTransitions )\n\t%i not in [0, %i)",
            transitionCount,
            11);
    simulatedStateLevel = self->simulatedStateLevel;
    if (simulatedStateLevel)
    {
        v6 = va(
            "s level %i stack %i %i %i %i %i, level %i stack %i %i %i %i %i",
            simulatedStateLevel,
            self->eSimulatedState[0],
            self->eSimulatedState[1],
            self->eSimulatedState[2],
            self->eSimulatedState[3],
            self->eSimulatedState[4],
            self->stateLevel,
            v7,
            v8,
            v9,
            v10,
            v11);
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
            510,
            0,
            "%s\n\t%s",
            "self->simulatedStateLevel == 0",
            v6);
    }
    self->StateTransitions[self->transitionCount].eTransition = AIS_TRANSITION_SET;
    self->StateTransitions[self->transitionCount++].eState = eState;
    Actor_SimplifyStateTransitions(self);
    self->eSimulatedState[self->simulatedStateLevel] = eState;
    Actor_ClearArrivalPos(self);
}

void __fastcall Actor_PopState(actor_s *self)
{
    unsigned int transitionCount; // r7

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp", 569, 0, "%s", "self");
    transitionCount = self->transitionCount;
    if (transitionCount >= 0xB)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
            570,
            0,
            "self->transitionCount doesn't index ARRAY_COUNT( self->StateTransitions )\n\t%i not in [0, %i)",
            transitionCount,
            11);
    if (!self->simulatedStateLevel)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
            571,
            0,
            "%s",
            "self->simulatedStateLevel > 0");
    self->StateTransitions[self->transitionCount].eTransition = AIS_TRANSITION_POP;
    self->StateTransitions[self->transitionCount++].eState = AIS_INVALID;
    Actor_SimplifyStateTransitions(self);
    --self->simulatedStateLevel;
}

void __fastcall Actor_ForceState(actor_s *self, ai_state_t eState)
{
    unsigned int v4; // r11

    if (self->stateLevel)
    {
        do
        {
            if (!AIFuncTable[self->species][self->eState[self->stateLevel]].pfnFinish)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
                    595,
                    0,
                    "%s",
                    "AIFuncTable[self->species][self->eState[self->stateLevel]].pfnFinish");
            AIFuncTable[self->species][self->eState[self->stateLevel]].pfnFinish(self, eState);
            v4 = self->stateLevel - 1;
            self->stateLevel = v4;
        } while (v4);
    }
    self->simulatedStateLevel = 0;
    self->transitionCount = 0;
    Actor_SetState(self, eState);
}

void __fastcall Actor_PrepareToPushState(actor_s *self, int eState)
{
    unsigned int simulatedStateLevel; // r6
    unsigned int v5; // r31
    ai_state_t *v6; // r9
    unsigned int v7; // r10
    const ai_state_t *v8; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp", 465, 0, "%s", "self");
    if (eState < 7 || eState > 10)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
            466,
            0,
            "%s",
            "eState >= AIS_PUSHABLE_FIRST && eState <= AIS_PUSHABLE_LAST");
    simulatedStateLevel = self->simulatedStateLevel;
    v5 = 1;
    if (simulatedStateLevel)
    {
        v6 = &self->eSimulatedState[1];
        while (2)
        {
            v7 = 0;
            v8 = g_eSupercededStates[eState - 7];
            do
            {
                if (*v6 == *v8)
                {
                    do
                        Actor_PopState(self);
                    while (v5 <= self->simulatedStateLevel);
                    return;
                }
                ++v7;
                ++v8;
            } while (v7 < 4);
            ++v5;
            ++v6;
            if (v5 <= simulatedStateLevel)
                continue;
            break;
        }
    }
}

int __fastcall Actor_PushState(actor_s *self, ai_state_t eState)
{
    int result; // r3
    unsigned int simulatedStateLevel; // r7
    unsigned int v6; // r10

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp", 535, 0, "%s", "self");
    if (eState < AIS_PAIN || eState > AIS_NEGOTIATION)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
            536,
            0,
            "eState not in [AIS_PUSHABLE_FIRST, AIS_PUSHABLE_LAST]\n\t%i not in [%i, %i]",
            eState,
            7,
            10);
    result = Actor_AllowedToPushState(self, eState);
    if (result)
    {
        Actor_PrepareToPushState(self, eState);
        simulatedStateLevel = self->simulatedStateLevel;
        if (simulatedStateLevel >= 4)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_state.cpp",
                543,
                0,
                "self->simulatedStateLevel doesn't index ARRAY_COUNT( self->eState ) - 1\n\t%i not in [0, %i)",
                simulatedStateLevel,
                4);
        self->StateTransitions[self->transitionCount].eTransition = AIS_TRANSITION_PUSH;
        self->StateTransitions[self->transitionCount++].eState = eState;
        Actor_SimplifyStateTransitions(self);
        v6 = 4 * (self->simulatedStateLevel + 40);
        ++self->simulatedStateLevel;
        *(gentity_s **)((char *)&self->ent + v6) = (gentity_s *)eState;
        Actor_ClearArrivalPos(self);
        return 1;
    }
    return result;
}

