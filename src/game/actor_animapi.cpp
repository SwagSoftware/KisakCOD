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
    if (self->AnimScriptHandle)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp",
            16,
            0,
            "%s",
            "self->AnimScriptHandle == 0");
    if (self->pAnimScriptFunc)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp",
            17,
            0,
            "%s",
            "self->pAnimScriptFunc == NULL");
    if (self->eAnimMode)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp",
            18,
            0,
            "%s",
            "self->eAnimMode == AI_ANIM_UNKNOWN");
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
    unsigned __int8 moveMode,
    ai_animmode_t animMode)
{
    ai_animmode_t eScriptSetAnimMode; // r11
    gentity_s *ent; // r11
    unsigned __int16 v10; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp", 72, 0, "%s", "self");
    if (!pAnimScriptFunc)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp", 73, 0, "%s", "pAnimScriptFunc");
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
        v10 = Scr_ExecEntThread(self->ent, pAnimScriptFunc->func, 0);
        self->AnimScriptHandle = v10;
        if (!v10)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp", 117, 0, "%s", "self->AnimScriptHandle");
        G_XAnimUpdateEnt(self->ent);
        Scr_IncTime();
    }
}

void __cdecl Actor_AnimMoveAway(actor_s *self, scr_animscript_t *pAnimScriptFunc)
{
    unsigned __int8 v4; // r5
    scr_animscript_t *p_move; // r4

    if (!pAnimScriptFunc)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp", 132, 0, "%s", "pAnimScriptFunc");
    if ((float)((float)(self->Physics.vVelocity[0] * self->Physics.vVelocity[0])
        + (float)(self->Physics.vVelocity[1] * self->Physics.vVelocity[1])) >= 1.0)
    {
        v4 = 2;
        p_move = &g_animScriptTable[self->species]->move;
    }
    else
    {
        v4 = 1;
        p_move = pAnimScriptFunc;
    }
    Actor_SetAnimScript(self, p_move, v4, AI_ANIM_MOVE_CODE);
    self->bUseGoalWeight = 0;
}

void __cdecl Actor_AnimStop(actor_s *self, scr_animscript_t *pAnimScriptFunc)
{
    if (!pAnimScriptFunc)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp", 150, 0, "%s", "pAnimScriptFunc");
    Actor_CheckCollisions(self);
    if (self->pCloseEnt.isDefined())
    {
        Actor_AnimMoveAway(self, pAnimScriptFunc);
    }
    else
    {
        if (!pAnimScriptFunc)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp", 156, 0, "%s", "pAnimScriptFunc");
        Actor_SetAnimScript(self, pAnimScriptFunc, 0, AI_ANIM_MOVE_CODE);
        self->bUseGoalWeight = 0;
    }
}

void __cdecl Actor_AnimWalk(actor_s *self, scr_animscript_t *pAfterMoveAnimScriptFunc)
{
    int iPathEndTime; // r10
    unsigned __int8 v5; // r5
    scr_animscript_t *p_move; // r4

    if (!Actor_HasPath(self))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp", 175, 0, "%s", "Actor_HasPath( self )");
    if (!pAfterMoveAnimScriptFunc)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp", 176, 0, "%s", "pAfterMoveAnimScriptFunc");
    iPathEndTime = self->Path.iPathEndTime;
    if (iPathEndTime && iPathEndTime - level.time < 200)
    {
        v5 = 1;
        p_move = pAfterMoveAnimScriptFunc;
    }
    else
    {
        v5 = 2;
        p_move = &g_animScriptTable[self->species]->move;
    }
    Actor_SetAnimScript(self, p_move, v5, AI_ANIM_MOVE_CODE);
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
    int iPathEndTime; // r10
    scr_animscript_t *StopAnim; // r4
    unsigned __int8 v4; // r5

    if (self->pCloseEnt.isDefined())
    {
        Actor_AnimMoveAway(self, &g_animScriptTable[self->species]->stop);
    }
    else
    {
        if (!Actor_HasPath(self))
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp", 235, 0, "%s", "Actor_HasPath( self )");
        iPathEndTime = self->Path.iPathEndTime;
        if (iPathEndTime && iPathEndTime - level.time < 200)
        {
            StopAnim = Actor_GetStopAnim(self);
            v4 = 1;
        }
        else
        {
            v4 = 2;
            StopAnim = &g_animScriptTable[self->species]->move;
        }
        Actor_SetAnimScript(self, StopAnim, v4, AI_ANIM_MOVE_CODE);
        self->bUseGoalWeight = 0;
    }
}

void __cdecl Actor_AnimRun(actor_s *self, scr_animscript_t *pAfterMoveAnimScriptFunc)
{
    int iPathEndTime; // r10
    unsigned __int8 v5; // r5
    scr_animscript_t *p_move; // r4

    if (!Actor_HasPath(self))
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp", 258, 0, "%s", "Actor_HasPath( self )");
    if (!pAfterMoveAnimScriptFunc)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp", 259, 0, "%s", "pAfterMoveAnimScriptFunc");
    iPathEndTime = self->Path.iPathEndTime;
    if (iPathEndTime && iPathEndTime - level.time < 200)
    {
        v5 = 1;
        p_move = pAfterMoveAnimScriptFunc;
    }
    else
    {
        v5 = 3;
        p_move = &g_animScriptTable[self->species]->move;
    }
    Actor_SetAnimScript(self, p_move, v5, AI_ANIM_MOVE_CODE);
    self->bUseGoalWeight = 0;
}

void __cdecl Actor_AnimTryRun(actor_s *self)
{
    int iPathEndTime; // r10
    scr_animscript_t *StopAnim; // r4
    unsigned __int8 v4; // r5

    if (self->pCloseEnt.isDefined())
    {
        Actor_AnimMoveAway(self, &g_animScriptTable[self->species]->stop);
    }
    else
    {
        if (!Actor_HasPath(self))
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp", 280, 0, "%s", "Actor_HasPath( self )");
        iPathEndTime = self->Path.iPathEndTime;
        if (iPathEndTime && iPathEndTime - level.time <= 200)
        {
            StopAnim = Actor_GetStopAnim(self);
            v4 = 1;
        }
        else
        {
            v4 = 3;
            StopAnim = &g_animScriptTable[self->species]->move;
        }
        Actor_SetAnimScript(self, StopAnim, v4, AI_ANIM_MOVE_CODE);
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
    Actor_SetAnimScript(self, &v5->combat, 0, v7);
}

void __cdecl Actor_AnimPain(actor_s *self)
{
    Actor_SetAnimScript(self, &g_animScriptTable[self->species]->pain, 0, AI_ANIM_USE_BOTH_DELTAS);
    self->bUseGoalWeight = 0;
}

void __cdecl Actor_AnimDeath(actor_s *self)
{
    ai_animmode_t eScriptSetAnimMode; // r6

    eScriptSetAnimMode = self->eScriptSetAnimMode;
    if (eScriptSetAnimMode == AI_ANIM_UNKNOWN)
        eScriptSetAnimMode = AI_ANIM_USE_BOTH_DELTAS;
    Actor_SetAnimScript(self, &g_animScriptTable[self->species]->death, 0, eScriptSetAnimMode);
    self->bUseGoalWeight = 0;
}

void __cdecl Actor_AnimSpecific(actor_s *self, scr_animscript_t *func, ai_animmode_t eAnimMode, bool bUseGoalWeight)
{
    ai_animmode_t eScriptSetAnimMode; // r6

    if (!func)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_animapi.cpp", 389, 0, "%s", "func");
    eScriptSetAnimMode = self->eScriptSetAnimMode;
    if (eScriptSetAnimMode == AI_ANIM_UNKNOWN)
        eScriptSetAnimMode = eAnimMode;
    Actor_SetAnimScript(self, func, 0, eScriptSetAnimMode);
    self->bUseGoalWeight = bUseGoalWeight;
}

void __cdecl Actor_AnimScripted(actor_s *self)
{
    Actor_SetAnimScript(self, &g_animScriptTable[self->species]->scripted, 0, AI_ANIM_USE_BOTH_DELTAS);
    self->bUseGoalWeight = 1;
}

