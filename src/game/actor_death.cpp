#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_death.h"

bool __cdecl Actor_Death_Start(actor_s *self, ai_state_t ePrevState)
{
    int health; // r8
    float *v4; // r6
    int v5; // r5
    int time; // r11
    gentity_s *ent; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_death.cpp", 20, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_death.cpp", 21, 0, "%s", "self->ent");
    health = self->ent->health;
    if (health > 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_death.cpp",
            22,
            0,
            "%s\n\t(self->ent->health) = %i",
            "(self->ent->health <= 0)",
            health);
    Actor_ClearKeepClaimedNode(self);
    Sentient_ClaimNode(self->sentient, 0);
    if (self->eAnimMode != AI_ANIM_NOPHYSICS
        && !self->ent->tagInfo
        && !BG_ActorIsProne(&self->ProneInfo, level.time)
        && !(unsigned __int8)Com_IsRagdollTrajectory(&self->ent->s.lerp.pos))
    {
        time = level.time;
        ent = self->ent;
        self->ProneInfo.fTorsoPitch = 0.0;
        self->ProneInfo.iProneTime = time;
        self->ProneInfo.fWaistPitch = 0.0;
        self->ProneInfo.prone = 1;
        self->ProneInfo.bCorpseOrientation = 1;
        self->ProneInfo.iProneTrans = 500;
        Actor_OrientCorpseToGround(ent, 0, v5, v4);
    }
    Actor_SetSubState(self, STATE_DEATH_PRECLEANUP);
    return 1;
}

void __cdecl Actor_Death_Cleanup(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_death.cpp", 53, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_death.cpp", 54, 0, "%s", "self->sentient");
    Actor_BroadcastTeamEvent(self->sentient, AI_EV_DEATH);
    Actor_Grenade_DropIfHeld(self);
    Sentient_Dissociate(self->sentient);
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_death.cpp", 63, 0, "%s", "self->sentient");
    if (self->Physics.bIsAlive)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_death.cpp", 64, 0, "%s", "!self->Physics.bIsAlive");
    Actor_ClearPath(self);
}

actor_think_result_t __cdecl Actor_Death_Think(actor_s *self)
{
    unsigned int v2; // r10

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_death.cpp", 79, 0, "%s", "self");
    v2 = 4 * (self->stateLevel + 8);
    self->pszDebugInfo = "death";
    if (*(gentity_s **)((char *)&self->ent + v2) == (gentity_s *)200 && level.time - self->iStateTime >= 800)
    {
        Actor_Death_Cleanup(self);
        Actor_SetSubState(self, STATE_DEATH_POSTCLEANUP);
    }
    if (self->pAnimScriptFunc != &g_animScriptTable[self->species]->death || Actor_IsAnimScriptAlive(self))
    {
        Actor_SetOrientMode(self, AI_ORIENT_DONT_CHANGE);
        Actor_AnimDeath(self);
        Actor_PostThink(self);
        if (level.time >= self->iStateTime + 500)
        {
            self->ent->r.contents = 67117056;
            SV_LinkEntity(self->ent);
        }
        return 0;
    }
    else
    {
        if (self->eSubState[self->stateLevel] == STATE_DEATH_PRECLEANUP)
            Actor_Death_Cleanup(self);
        return 2;
    }
}

