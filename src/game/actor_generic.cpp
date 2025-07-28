#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_generic.h"
#include "g_main.h"

void __cdecl Actor_Generic_Finish(actor_s *self, ai_state_t eNextState)
{
    ;
}

void __cdecl Actor_Generic_Suspend(actor_s *self, ai_state_t eNextState)
{
    unsigned int stateLevel; // r7
    ai_state_t v5; // r8
    ai_state_t v6; // r8

    stateLevel = self->stateLevel;
    if (stateLevel >= 5)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_generic.cpp",
            25,
            0,
            "self->stateLevel doesn't index ARRAY_COUNT( self->eState )\n\t%i not in [0, %i)",
            stateLevel,
            5);
    v5 = self->eState[self->stateLevel];
    if (v5 <= AIS_INVALID || v5 >= AIS_COUNT)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_generic.cpp",
            26,
            0,
            "%s\n\t(self->eState[self->stateLevel]) = %i",
            "(self->eState[self->stateLevel] > AIS_INVALID && self->eState[self->stateLevel] < AIS_COUNT)",
            v5);
    v6 = self->eState[self->stateLevel];
    if (!AIFuncTable[self->species][v6].pfnFinish)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_generic.cpp",
            27,
            0,
            "%s\n\t(self->eState[self->stateLevel]) = %i",
            "(AIFuncTable[self->species][self->eState[self->stateLevel]].pfnFinish)",
            v6);
    AIFuncTable[self->species][self->eState[self->stateLevel]].pfnFinish(self, eNextState);
}

bool __cdecl Actor_Generic_Resume(actor_s *self, ai_state_t ePrevState)
{
    unsigned int stateLevel; // r7
    ai_state_t v5; // r8
    ai_state_t v6; // r8

    stateLevel = self->stateLevel;
    if (stateLevel >= 4)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_generic.cpp",
            42,
            0,
            "self->stateLevel doesn't index ARRAY_COUNT( self->eState ) - 1\n\t%i not in [0, %i)",
            stateLevel,
            4);
    v5 = self->eState[self->stateLevel];
    if (v5 <= AIS_INVALID || v5 >= AIS_COUNT)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_generic.cpp",
            43,
            0,
            "%s\n\t(self->eState[self->stateLevel]) = %i",
            "(self->eState[self->stateLevel] > AIS_INVALID && self->eState[self->stateLevel] < AIS_COUNT)",
            v5);
    v6 = self->eState[self->stateLevel];
    if (!AIFuncTable[self->species][v6].pfnStart)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_generic.cpp",
            44,
            0,
            "%s\n\t(self->eState[self->stateLevel]) = %i",
            "(AIFuncTable[self->species][self->eState[self->stateLevel]].pfnStart)",
            v6);
    return ((int(__cdecl *)(actor_s *, ai_state_t))AIFuncTable[self->species][self->eState[self->stateLevel]].pfnStart)(
        self,
        ePrevState);
}

void __cdecl Actor_Generic_Pain(
    actor_s *self,
    gentity_s *attacker,
    int iDamage,
    const float *vPoint,
    const int iMod,
    const float *vDir,
    const hitLocation_t hitLoc)
{
    ;
}

void __cdecl Actor_Generic_Touch(actor_s *self, gentity_s *pOther)
{
    int iHitEntnum; // r11

    iHitEntnum = self->Physics.iHitEntnum;
    if (pOther->s.number == iHitEntnum)
    {
        if (level.gentities[iHitEntnum].client)
            Actor_ClearPath(self);
    }
}

