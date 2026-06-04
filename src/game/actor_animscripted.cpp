#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_animscripted.h"
#include "g_local.h"
#include <script/scr_memorytree.h>
#include "actor_state.h"
#include "actor_orientation.h"
#include <script/scr_const.h>

bool __cdecl Actor_ScriptedAnim_Start(actor_s *self, ai_state_t ePrevState)
{
    return 1;
}

void __cdecl Actor_ScriptedAnim_Finish(actor_s *self, ai_state_t eNextState)
{
    gentity_s *ent; // r31

    ent = self->ent;
    iassert(ent);
    if (ent->scripted)
    {
        MT_Free((unsigned char*)ent->scripted, sizeof(animscripted_s));
        ent->scripted = NULL;
    }
}

actor_think_result_t __cdecl Actor_ScriptedAnim_Think(actor_s *self)
{
    int keepNodeDuringScriptedAnim; // r10
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
        iassert(self->eSimulatedState[self->simulatedStateLevel] == AIS_SCRIPTEDANIM || self->eState[self->stateLevel] == AIS_DEATH);

        if (self->eSimulatedState[self->simulatedStateLevel] != AIS_SCRIPTEDANIM)
            return ACTOR_THINK_REPEAT;

        Actor_PopState(self);
        return ACTOR_THINK_REPEAT;
    }

    ent = self->ent;
    iassert(ent);
    G_Animscripted_Think(ent);

    if (!ent->scripted)
    {
        Actor_PopState(self);
        return ACTOR_THINK_REPEAT;
    }

    Actor_PreThink(self);
    Actor_SetDesiredAngles(&self->CodeOrient, ent->r.currentAngles[0], ent->r.currentAngles[1]);

    Vec3Clear(self->Physics.vVelocity);
    Vec3Clear(self->Physics.vWishDelta);

    return ACTOR_THINK_DONE;
}

bool __cdecl Actor_CustomAnim_Start(actor_s *self, ai_state_t ePrevState)
{
    Scr_Notify(self->ent, scr_const.begin_custom_anim, 0);
    return 1;
}

actor_think_result_t __cdecl Actor_CustomAnim_Think(actor_s *self)
{
    scr_animscript_t *p_AnimScriptSpecific; // r28
    int keepNodeDuringScriptedAnim; // r10

    p_AnimScriptSpecific = &self->AnimScriptSpecific;

    iassert(self->AnimScriptSpecific.func);

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
        iassert(self->ent);
        Actor_PreThink(self);
        iassert(self->eAnimMode != AI_ANIM_UNKNOWN);
        Actor_UpdateOriginAndAngles(self);
        return ACTOR_THINK_DONE;
    }
    else
    {
        Actor_PopState(self);
        return ACTOR_THINK_REPEAT;
    }
}

