#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_animscripted.h"

int __cdecl Actor_ScriptedAnim_Start(actor_s *self, ai_state_t ePrevState)
{
    return 1;
}

void __cdecl Actor_ScriptedAnim_Finish(actor_s *self, ai_state_t eNextState)
{
    gentity_s *ent; // r31
    animscripted_s *scripted; // r3

    ent = self->ent;
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_animscripted.cpp", 31, 0, "%s", "ent");
    scripted = ent->scripted;
    if (scripted)
    {
        MT_Free(scripted, 96);
        ent->scripted = 0;
    }
}

actor_think_result_t __cdecl Actor_ScriptedAnim_Think(actor_s *self)
{
    int keepNodeDuringScriptedAnim; // r10
    actor_s *v3; // r3
    actor_think_result_t result; // r3
    gentity_s *ent; // r30

    keepNodeDuringScriptedAnim = self->keepNodeDuringScriptedAnim;
    self->pszDebugInfo = "animscripted";
    if (!keepNodeDuringScriptedAnim)
    {
        Actor_ClearKeepClaimedNode(self);
        Sentient_ClaimNode(self->sentient, 0);
    }
    Actor_ClearPath(self);
    Actor_AnimScripted(self);
    self->pushable = 0;
    if (!Actor_IsAnimScriptAlive(self))
    {
        if (self->eSimulatedState[self->simulatedStateLevel] != AIS_SCRIPTEDANIM
            && self->eState[self->stateLevel] != AIS_DEATH)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_animscripted.cpp",
                66,
                0,
                "%s",
                "self->eSimulatedState[self->simulatedStateLevel] == AIS_SCRIPTEDANIM || self->eState[self->stateLevel] == AIS_DEATH");
        }
        if (self->eSimulatedState[self->simulatedStateLevel] != AIS_SCRIPTEDANIM)
            return 1;
        v3 = self;
    LABEL_9:
        Actor_PopState(v3);
        return 1;
    }
    ent = self->ent;
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_animscripted.cpp", 73, 0, "%s", "ent");
    G_Animscripted_Think(ent);
    v3 = self;
    if (!ent->scripted)
        goto LABEL_9;
    Actor_PreThink(self);
    Actor_SetDesiredAngles(&self->CodeOrient, ent->r.currentAngles[0], ent->r.currentAngles[1]);
    result = ACTOR_THINK;
    self->Physics.vVelocity[0] = 0.0;
    self->Physics.vVelocity[1] = 0.0;
    self->Physics.vVelocity[2] = 0.0;
    self->Physics.vWishDelta[0] = 0.0;
    self->Physics.vWishDelta[1] = 0.0;
    self->Physics.vWishDelta[2] = 0.0;
    return result;
}

int __cdecl Actor_CustomAnim_Start(actor_s *self, ai_state_t ePrevState)
{
    Scr_Notify(self->ent, scr_const.begin_custom_anim, 0);
    return 1;
}

int __cdecl Actor_CustomAnim_Think(actor_s *self)
{
    scr_animscript_t *p_AnimScriptSpecific; // r28
    int keepNodeDuringScriptedAnim; // r10

    p_AnimScriptSpecific = &self->AnimScriptSpecific;
    if (!self->AnimScriptSpecific.func)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_animscripted.cpp",
            118,
            0,
            "%s",
            "self->AnimScriptSpecific.func");
    keepNodeDuringScriptedAnim = self->keepNodeDuringScriptedAnim;
    self->pszDebugInfo = "animcustom";
    if (!keepNodeDuringScriptedAnim)
    {
        Actor_ClearKeepClaimedNode(self);
        Sentient_ClaimNode(self->sentient, 0);
    }
    Actor_ClearPath(self);
    Actor_AnimSpecific(self, p_AnimScriptSpecific, AI_ANIM_USE_BOTH_DELTAS, 1);
    if (Actor_IsAnimScriptAlive(self))
    {
        if (!self->ent)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_animscripted.cpp", 137, 0, "%s", "ent");
        Actor_PreThink(self);
        if (self->eAnimMode == AI_ANIM_UNKNOWN)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_animscripted.cpp",
                141,
                0,
                "%s",
                "self->eAnimMode != AI_ANIM_UNKNOWN");
        Actor_UpdateOriginAndAngles(self);
        return 0;
    }
    else
    {
        Actor_PopState(self);
        return 1;
    }
}

