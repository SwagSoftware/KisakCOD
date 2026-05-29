#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor.h"
#include "actor_animapi.h"
#include <script/scr_vm.h>
#include <script/scr_const.h>
#include "g_local.h"
#include "actor_orientation.h"
#include "g_scr_main.h"
#include "g_main.h"
#include "actor_cover.h"

void __cdecl Actor_InitAnim(actor_s *self)
{
    iassert(self->AnimScriptHandle == 0);
    iassert(self->pAnimScriptFunc == NULL);
    iassert(self->eAnimMode == AI_ANIM_UNKNOWN);
}

unsigned int __cdecl Actor_IsAnimScriptAlive(actor_s *self)
{
    unsigned int result; // r3

    result = self->AnimScriptHandle;
    if (result)
        return Scr_IsThreadAlive(result);
    return result;
}

void __cdecl Actor_KillAnimScript(actor_s *self)
{
    int AnimScriptHandle; // r11

    if (!Scr_IsSystemActive())
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp",
            47,
            0,
            "%s",
            "Scr_IsSystemActive( SCR_SYS_GAME )");

    AnimScriptHandle = self->AnimScriptHandle;
    self->pAnimScriptFunc = 0;
    if (AnimScriptHandle)
    {
        Scr_Notify(self->ent, scr_const.killanimscript, 0);
        Scr_FreeThread(self->AnimScriptHandle);
        self->AnimScriptHandle = 0;
        self->arrivalInfo.animscriptOverrideRunTo = 0;
    }
}

void __cdecl Actor_SetAnimScript(
    actor_s *self,
    scr_animscript_t *pAnimScriptFunc,
    ai_movemode_t moveMode,
    ai_animmode_t animMode)
{
    ai_animmode_t eScriptSetAnimMode; // r11
    gentity_s *ent; // r11

    iassert(self);
    iassert(pAnimScriptFunc);

    if (self->pAnimScriptFunc == pAnimScriptFunc)
    {
        eScriptSetAnimMode = self->eScriptSetAnimMode;
        if (eScriptSetAnimMode)
            self->eAnimMode = eScriptSetAnimMode;
        else
            self->eAnimMode = animMode;
        if (self->moveMode != moveMode)
        {
            self->moveMode = moveMode;
            Scr_Notify(self->ent, scr_const.movemode, 0);
        }
    }
    else
    {
        Scr_DecTime();
        Actor_KillAnimScript(self);
        Scr_RunCurrentThreads();
        Actor_ClearScriptOrient(self);
        ent = self->ent;
        self->bGrenadeTossValid = 0;
        self->eScriptSetAnimMode = AI_ANIM_UNKNOWN;
        self->safeToChangeScript = 1;
        ent->flags &= ~(FL_NO_AUTO_ANIM_UPDATE);
        Scr_SetString(&self->scriptState, 0);
        Scr_SetString(&self->stateChangeReason, 0);
        self->pAnimScriptFunc = pAnimScriptFunc;
        self->eAnimMode = animMode;
        self->moveMode = moveMode;
        self->pushable = 1;
        Actor_ClearKeepClaimedNode(self);
        self->AnimScriptHandle = Scr_ExecEntThread(self->ent, pAnimScriptFunc->func, 0);
        iassert(self->AnimScriptHandle);
        G_XAnimUpdateEnt(self->ent);
        Scr_IncTime();
    }
}

void __cdecl Actor_AnimMoveAway(actor_s *self, scr_animscript_t *pAnimScriptFunc)
{
    iassert(pAnimScriptFunc);
    
    if (Vec2LengthSq(self->Physics.vVelocity) >= 1.0f)
    {
        Actor_SetAnimScript(self, &g_animScriptTable[self->species]->move, AI_MOVE_WALK, AI_ANIM_MOVE_CODE);
    }
    else
    {
        Actor_SetAnimScript(self, pAnimScriptFunc, AI_MOVE_STOP_SOON, AI_ANIM_MOVE_CODE);
    }

    self->bUseGoalWeight = 0;
}

void __cdecl Actor_AnimStop(actor_s *self, scr_animscript_t *pAnimScriptFunc)
{
    iassert(pAnimScriptFunc);

    Actor_CheckCollisions(self);

    if (self->pCloseEnt.isDefined())
    {
        Actor_AnimMoveAway(self, pAnimScriptFunc);
    }
    else
    {
        iassert(pAnimScriptFunc);
        Actor_SetAnimScript(self, pAnimScriptFunc, AI_MOVE_STOP, AI_ANIM_MOVE_CODE);
        self->bUseGoalWeight = 0;
    }
}

void __cdecl Actor_AnimWalk(actor_s *self, scr_animscript_t *pAfterMoveAnimScriptFunc)
{
    iassert(Actor_HasPath(self));
    iassert(pAfterMoveAnimScriptFunc);

    if (self->Path.iPathEndTime && self->Path.iPathEndTime - level.time < 200)
    {
        Actor_SetAnimScript(self, pAfterMoveAnimScriptFunc, AI_MOVE_STOP_SOON, AI_ANIM_MOVE_CODE);
    }
    else
    {
        Actor_SetAnimScript(self, &g_animScriptTable[self->species]->move, AI_MOVE_WALK, AI_ANIM_MOVE_CODE);
    }

    self->bUseGoalWeight = 0;
}

scr_animscript_t *__cdecl Actor_GetStopAnim(actor_s *self)
{
    AISpecies species; // r10
    pathnode_t *pClaimedNode; // r30
    scr_animscript_t *result; // r3
    scr_animscript_t *v5; // [sp+50h] [-20h] BYREF

    species = self->species;
    if (species == AI_SPECIES_HUMAN)
    {
        pClaimedNode = self->sentient->pClaimedNode;
        if ((!self->fixedNode || pClaimedNode == self->codeGoal.node)
            && pClaimedNode
            && Actor_IsNearClaimedNode(self)
            && Actor_Cover_PickAttackScript(self, pClaimedNode, 0, &v5))
        {
            result = v5;
            if (v5)
                return result;
            if (!self->fixedNode)
                Actor_NodeClaimRevoked(self, 5000);
        }
        species = self->species;
    }
    return &g_animScriptTable[species]->stop;
}

void __cdecl Actor_AnimTryWalk(actor_s *self)
{
    if (self->pCloseEnt.isDefined())
    {
        Actor_AnimMoveAway(self, &g_animScriptTable[self->species]->stop);
    }
    else
    {
        iassert(Actor_HasPath(self));
        if (self->Path.iPathEndTime && self->Path.iPathEndTime - level.time < 200)
        {
            Actor_SetAnimScript(self, Actor_GetStopAnim(self), AI_MOVE_STOP_SOON, AI_ANIM_MOVE_CODE);
        }
        else
        {
            Actor_SetAnimScript(self, &g_animScriptTable[self->species]->move, AI_MOVE_WALK, AI_ANIM_MOVE_CODE);
        }

        self->bUseGoalWeight = 0;
    }
}

void __cdecl Actor_AnimRun(actor_s *self, scr_animscript_t *pAfterMoveAnimScriptFunc)
{
    iassert(Actor_HasPath(self));
    iassert(pAfterMoveAnimScriptFunc);

    if (self->Path.iPathEndTime && self->Path.iPathEndTime - level.time < 200)
    {
        Actor_SetAnimScript(self, pAfterMoveAnimScriptFunc, AI_MOVE_STOP_SOON, AI_ANIM_MOVE_CODE);
    }
    else
    {
        Actor_SetAnimScript(self, &g_animScriptTable[self->species]->move, AI_MOVE_RUN, AI_ANIM_MOVE_CODE);
    }

    self->bUseGoalWeight = 0;
}

void __cdecl Actor_AnimTryRun(actor_s *self)
{
    if (self->pCloseEnt.isDefined())
    {
        Actor_AnimMoveAway(self, &g_animScriptTable[self->species]->stop);
    }
    else
    {
        iassert(Actor_HasPath(self));
        if (self->Path.iPathEndTime && self->Path.iPathEndTime - level.time <= 200)
        {
            Actor_SetAnimScript(self, Actor_GetStopAnim(self), AI_MOVE_STOP_SOON, AI_ANIM_MOVE_CODE);
        }
        else
        {
            Actor_SetAnimScript(self, &g_animScriptTable[self->species]->move, AI_MOVE_RUN, AI_ANIM_MOVE_CODE);
        }

        self->bUseGoalWeight = 0;
    }
}

void __cdecl Actor_AnimCombat(actor_s *self)
{
    sentient_s *sentient; // r10
    bool fixedNode; // r11
    const pathnode_t *pClaimedNode; // r30
    AnimScriptList *v5; // r4
    int isDefined; // r3
    ai_animmode_t v7; // r6
    AnimScriptList *v8; // [sp+50h] [-30h] BYREF

    Actor_CheckCollisions(self);
    Actor_ClearPileUp(self);
    sentient = self->sentient;
    fixedNode = self->fixedNode;
    self->bUseGoalWeight = 0;
    pClaimedNode = sentient->pClaimedNode;
    if (fixedNode && pClaimedNode != self->codeGoal.node)
        pClaimedNode = 0;
    if (!fixedNode || self->exposedStartTime + 2000 < level.time)
    {
        if (pClaimedNode
            && self->species == AI_SPECIES_HUMAN
            && Actor_IsNearClaimedNode(self)
            && Actor_Cover_PickAttackScript(self, pClaimedNode, 0, (scr_animscript_t **)&v8))
        {
            v5 = v8;
            if (v8)
            {
            LABEL_17:
                v7 = AI_ANIM_MOVE_CODE;
                goto LABEL_18;
            }
            if (!self->fixedNode)
                Actor_NodeClaimRevoked(self, 5000);
        }
        if (self->fixedNode && (AnimScriptList *)self->pAnimScriptFunc != g_animScriptTable[self->species])
            self->exposedStartTime = level.time;
    }
    isDefined = self->pCloseEnt.isDefined();
    v7 = AI_ANIM_USE_BOTH_DELTAS;
    v5 = g_animScriptTable[self->species];
    if (isDefined)
        goto LABEL_17;
LABEL_18:
    Actor_SetAnimScript(self, &v5->combat, AI_MOVE_STOP, v7);
}

void __cdecl Actor_AnimPain(actor_s *self)
{
    Actor_SetAnimScript(self, &g_animScriptTable[self->species]->pain, AI_MOVE_STOP, AI_ANIM_USE_BOTH_DELTAS);
    self->bUseGoalWeight = 0;
}

void __cdecl Actor_AnimDeath(actor_s *self)
{
    ai_animmode_t eScriptSetAnimMode; // r6

    eScriptSetAnimMode = self->eScriptSetAnimMode;
    if (eScriptSetAnimMode == AI_ANIM_UNKNOWN)
        eScriptSetAnimMode = AI_ANIM_USE_BOTH_DELTAS;
    Actor_SetAnimScript(self, &g_animScriptTable[self->species]->death, AI_MOVE_STOP, eScriptSetAnimMode);
    self->bUseGoalWeight = 0;
}

void __cdecl Actor_AnimSpecific(actor_s *self, scr_animscript_t *func, ai_animmode_t eAnimMode, bool bUseGoalWeight)
{
    ai_animmode_t eScriptSetAnimMode; // r6

    iassert(func);
    eScriptSetAnimMode = self->eScriptSetAnimMode;
    if (eScriptSetAnimMode == AI_ANIM_UNKNOWN)
        eScriptSetAnimMode = eAnimMode;
    Actor_SetAnimScript(self, func, AI_MOVE_STOP, eScriptSetAnimMode);
    self->bUseGoalWeight = bUseGoalWeight;
}

void __cdecl Actor_AnimScripted(actor_s *self)
{
    Actor_SetAnimScript(self, &g_animScriptTable[self->species]->scripted, AI_MOVE_STOP, AI_ANIM_USE_BOTH_DELTAS);
    self->bUseGoalWeight = 1;
}

