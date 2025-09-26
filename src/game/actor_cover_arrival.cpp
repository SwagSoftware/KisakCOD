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
    wNegotiationStartNode = wPathLen - 3;

    // aislop
    //_FP13 = (float)((float)16384.0 - (float)v5);
    //__asm { fsel      f7, f13, f0, f1 }

    float diff = 16384.0f - (float)v5;
    float _FP7 = (diff >= 0.0f) ? 1.0f : 0.0f;

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
    pathnode_t *arrivalNode; // r25
    double goalHeight; // fp0
    int iTraceMask; // r26
    int wPathLen; // r10
    int ptItr; // r30
    float traceEnd[3]; // [sp+50h] [-C0h] BYREF
    float traceStart[4]; // [sp+70h] [-A0h] BYREF
    trace_t traceresults; // [sp+80h] [-90h] BYREF

    iassert(Actor_HasPath(self));
    p_Path = &self->Path;
    if (Path_HasNegotiationNode(&self->Path))
    {
        arrivalNode = Path_GetNegotiationNode(&self->Path);
        iassert(arrivalNode);
        traceEnd[0] = arrivalNode->constant.vOrigin[0];
        traceEnd[1] = arrivalNode->constant.vOrigin[1];
        goalHeight = arrivalNode->constant.vOrigin[2];
    }
    else
    {
        arrivalNode = self->sentient->pClaimedNode;
        iassert(arrivalNode);
        traceEnd[0] = self->Path.vFinalGoal[0];
        traceEnd[1] = self->Path.vFinalGoal[1];
        goalHeight = self->Path.vFinalGoal[2];
    }
    iTraceMask = self->Physics.iTraceMask;
    wPathLen = self->Path.wPathLen;
    traceEnd[2] = goalHeight + 18.0f;

    ptItr = self->Path.wNegotiationStartNode + 1;

    if (ptItr < wPathLen)
    {
        do
        {
            float *origPoint = p_Path->pts[ptItr].vOrigPoint;

            if (ptItr > self->Path.wNegotiationStartNode + 1
                && (((traceEnd[1] - origPoint[1]) * (traceEnd[1] - origPoint[1]))
                    + ((traceEnd[0] - origPoint[0]) * (traceEnd[0] - origPoint[0]))) > 250000.0f)
            {
                break;
            }
            if (((arrivalNode->constant.forward[1] * (traceEnd[1] - origPoint[1]))
                + (arrivalNode->constant.forward[0] * (traceEnd[0] - origPoint[0]))) >= 0.0)
            {
                traceStart[0] = origPoint[0];
                traceStart[1] = origPoint[1];
                traceStart[2] = origPoint[2] + 18.0f;
                G_TraceCapsule(&traceresults, traceStart, vec3_origin, vec3_origin, traceEnd, self->ent->s.number, iTraceMask);
                if (traceresults.allsolid || traceresults.fraction < 1.0)
                    break;
            }
            ++ptItr;
        } while (ptItr < self->Path.wPathLen);
    }

    if (ptItr != self->Path.wNegotiationStartNode + 1)
    {
        pathpoint_t *pt = &p_Path->pts[ptItr];
        float vec[3];
        vec[0] = traceEnd[0] - pt[-1].vOrigPoint[0];
        vec[1] = traceEnd[1] - pt[-1].vOrigPoint[1];
        Vec2Normalize(vec);
        vec[2] = 0.0;
        Scr_AddVector(vec);
        Scr_Notify(self->ent, scr_const.corner_approach, 1);
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

