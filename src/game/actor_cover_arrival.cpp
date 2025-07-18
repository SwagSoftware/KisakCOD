#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_cover_arrival.h"
#include "g_local.h"
#include "actor_state.h"
#include "actor_orientation.h"
#include <script/scr_const.h>
#include <script/scr_vm.h>

bool __cdecl Actor_CheckCoverLeave(actor_s *self, const float *exitPos)
{
    double v5; // fp1
    int wPathLen; // r11
    int wNegotiationStartNode; // r8
    int v10; // r7
    int v11; // r6
    int v12; // r11
    float *i; // r9
    double v14; // fp13

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover_arrival.cpp", 25, 0, "%s", "self");
    if (!Actor_HasPath(self) || !self->sentient->pPrevClaimedNode)
        return 0;
    v5 = Vec2DistanceSq(self->ent->r.currentOrigin, exitPos);
    wPathLen = self->Path.wPathLen;
    _FP13 = (float)((float)16384.0 - (float)v5);
    wNegotiationStartNode = wPathLen - 3;
    __asm { fsel      f7, f13, f0, f1 }
    if (wPathLen - 3 < self->Path.wNegotiationStartNode)
        wNegotiationStartNode = self->Path.wNegotiationStartNode;
    v10 = wPathLen - 2;
    v11 = 0;
    v12 = wPathLen - 2;
    if (v12 >= wNegotiationStartNode)
    {
        for (i = self->Path.pts[v12].vOrigPoint;
            (float)((float)((float)(*i - self->ent->r.currentOrigin[0]) * (float)(*exitPos - self->ent->r.currentOrigin[0]))
                + (float)((float)(i[1] - self->ent->r.currentOrigin[1])
                    * (float)(exitPos[1] - self->ent->r.currentOrigin[1]))) >= 0.0
            || ++v11 <= 1;
            i -= 7)
        {
            v14 = (float)(i[1] - self->ent->r.currentOrigin[1]);
            if ((float)((float)((float)v14 * (float)v14)
                + (float)((float)(*i - self->ent->r.currentOrigin[0]) * (float)(*i - self->ent->r.currentOrigin[0]))) > _FP7)
            {
                if (v12 != v10)
                    return 1;
                return v11 == 0;
            }
            if (--v12 < wNegotiationStartNode)
                return 1;
        }
        return 0;
    }
    return 1;
}

int __cdecl Actor_CheckCoverApproach(actor_s *self)
{
    int result; // r3
    sentient_s *sentient; // r11
    float *pClaimedNode; // r30

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover_arrival.cpp", 77, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover_arrival.cpp", 78, 0, "%s", "self->sentient");
    if (self->arrivalInfo.animscriptOverrideRunTo)
        return 0;
    if (Path_HasNegotiationNode(&self->Path))
        return 1;
    sentient = self->sentient;
    pClaimedNode = (float *)sentient->pClaimedNode;
    if (!pClaimedNode)
        return 0;
    if (!Path_IsCoverNode(sentient->pClaimedNode))
        return 0;
    if (!Actor_HasPath(self))
        return 0;
    if (self->Path.vFinalGoal[0] != pClaimedNode[5])
        return 0;
    if (self->Path.vFinalGoal[1] != pClaimedNode[6])
        return 0;
    result = 1;
    if (self->Path.vFinalGoal[2] != pClaimedNode[7])
        return 0;
    return result;
}

void __cdecl Actor_CoverApproachNotify(actor_s *self)
{
    path_t *p_Path; // r29
    pathnode_t *NegotiationNode; // r25
    double v4; // fp0
    int iTraceMask; // r26
    int wPathLen; // r10
    int v7; // r30
    float *v8; // r31
    gentity_s *ent; // r11
    pathpoint_t *v10; // r11
    float v11; // [sp+50h] [-C0h] BYREF
    float v12; // [sp+54h] [-BCh]
    float v13; // [sp+58h] [-B8h]
    float v14[4]; // [sp+60h] [-B0h] BYREF
    float v15[4]; // [sp+70h] [-A0h] BYREF
    trace_t v16; // [sp+80h] [-90h] BYREF

    if (!Actor_HasPath(self))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover_arrival.cpp",
            113,
            0,
            "%s",
            "Actor_HasPath( self )");
    p_Path = &self->Path;
    if (Path_HasNegotiationNode(&self->Path))
    {
        NegotiationNode = Path_GetNegotiationNode(&self->Path);
        if (!NegotiationNode)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover_arrival.cpp", 120, 0, "%s", "arrivalNode");
        v11 = NegotiationNode->constant.vOrigin[0];
        v12 = NegotiationNode->constant.vOrigin[1];
        v4 = NegotiationNode->constant.vOrigin[2];
    }
    else
    {
        NegotiationNode = self->sentient->pClaimedNode;
        if (!NegotiationNode)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_cover_arrival.cpp", 126, 0, "%s", "arrivalNode");
        v11 = self->Path.vFinalGoal[0];
        v12 = self->Path.vFinalGoal[1];
        v4 = self->Path.vFinalGoal[2];
    }
    iTraceMask = self->Physics.iTraceMask;
    wPathLen = self->Path.wPathLen;
    v7 = self->Path.wNegotiationStartNode + 1;
    v13 = (float)v4 + (float)18.0;
    if (v7 < wPathLen)
    {
        v8 = &p_Path->pts[v7].vOrigPoint[1];
        do
        {
            if (v7 > self->Path.wNegotiationStartNode + 1
                && (float)((float)((float)(v12 - *v8) * (float)(v12 - *v8))
                    + (float)((float)(v11 - *(v8 - 1)) * (float)(v11 - *(v8 - 1)))) > 250000.0)
            {
                break;
            }
            if ((float)((float)(NegotiationNode->constant.forward[1] * (float)(v12 - *v8))
                + (float)(NegotiationNode->constant.forward[0] * (float)(v11 - *(v8 - 1)))) >= 0.0)
            {
                v15[0] = *(v8 - 1);
                ent = self->ent;
                v15[1] = *v8;
                v15[2] = v8[1] + (float)18.0;
                G_TraceCapsule(&v16, v15, vec3_origin, vec3_origin, &v11, ent->s.number, iTraceMask);
                if (v16.allsolid || v16.fraction < 1.0)
                    break;
            }
            ++v7;
            v8 += 7;
        } while (v7 < self->Path.wPathLen);
    }
    if (v7 != self->Path.wNegotiationStartNode + 1)
    {
        v10 = &p_Path->pts[v7];
        v14[0] = v11 - v10[-1].vOrigPoint[0];
        v14[1] = v12 - v10[-1].vOrigPoint[1];
        Vec2Normalize(v14);
        v14[2] = 0.0;
        Scr_AddVector(v14);
        Scr_Notify(self->ent, scr_const.corner_approach, 1u);
    }
}

bool __cdecl Actor_CoverArrival_Start(actor_s *self, ai_state_t ePrevState)
{
    int result; // r3

    Actor_SetOrientMode(self, AI_ORIENT_DONT_CHANGE);
    result = 1;
    self->eAnimMode = AI_ANIM_USE_BOTH_DELTAS_ZONLY_PHYSICS;
    self->bUseGoalWeight = 1;
    return result;
}

bool __cdecl Actor_CoverArrival_Resume(actor_s *self, ai_state_t ePrevState)
{
    return false;
}

actor_think_result_t __cdecl Actor_CoverArrival_Think(actor_s *self)
{
    scr_animscript_t *StopAnim; // r3

    iassert(self->bUseGoalWeight);
    Actor_SetAnimScript(self, &g_animScriptTable[self->species]->cover_arrival, 0, self->eAnimMode);
    self->pushable = 0;
    if (Actor_IsAnimScriptAlive(self))
    {
        self->pszDebugInfo = "cover_arrival";
        Actor_PreThink(self);
        G_ReduceOriginError(self->ent->r.currentOrigin, self->arrivalInfo.animscriptOverrideOriginError, 0.25);
        Actor_UpdateOriginAndAngles(self);
        return ACTOR_THINK_DONE;
    }
    else
    {
        Actor_SetState(self, AIS_EXPOSED);
        StopAnim = Actor_GetStopAnim(self);
        Actor_SetAnimScript(self, StopAnim, 0, AI_ANIM_MOVE_CODE);
        Actor_ClearPath(self);
        return ACTOR_THINK_REPEAT;
    }
}

