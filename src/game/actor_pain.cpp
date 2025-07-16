#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_pain.h"

bool __cdecl Actor_InPain(const actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_pain.cpp", 14, 0, "%s", "self");
    return self->eState[self->stateLevel] == AIS_PAIN;
}

bool __cdecl Actor_Pain_Start(actor_s *self, ai_state_t ePrevState)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_pain.cpp", 28, 0, "%s", "self");
    self->iPainTime = level.time;
    Actor_ClearPath(self);
    Actor_AnimPain(self);
    Scr_Notify(self->ent, scr_const.pain, 0);
    return 1;
}

void __cdecl Actor_Pain_Finish(actor_s *self, ai_state_t eNextState)
{
    sentient_s *sentient; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_pain.cpp", 51, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_pain.cpp", 52, 0, "%s", "self->ent");
    sentient = self->sentient;
    if (sentient->lastAttacker)
    {
        Actor_BroadcastTeamEvent(sentient, 5);
        self->sentient->lastAttacker = 0;
    }
}

actor_think_result_t __cdecl Actor_Pain_Think(actor_s *self)
{
    int iPainTime; // r10
    int time; // r11
    sentient_s *sentient; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_pain.cpp", 71, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_pain.cpp", 72, 0, "%s", "self->ent");
    iPainTime = self->iPainTime;
    self->pszDebugInfo = "pain";
    time = level.time;
    if (level.time > iPainTime)
    {
        if (!Actor_IsAnimScriptAlive(self))
        {
            Actor_PopState(self);
            return 1;
        }
        time = level.time;
    }
    sentient = self->sentient;
    if (sentient->lastAttacker)
    {
        if (time - self->iPainTime >= 500)
        {
            Actor_BroadcastTeamEvent(sentient, 5);
            self->sentient->lastAttacker = 0;
        }
    }
    Actor_PreThink(self);
    Actor_SetOrientMode(self, AI_ORIENT_DONT_CHANGE);
    Actor_AnimPain(self);
    Actor_PostThink(self);
    return 0;
}

