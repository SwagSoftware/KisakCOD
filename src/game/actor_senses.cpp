#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_senses.h"
#include "g_main.h"
#include <universal/com_math.h>
#include <server/server.h>
#include "pathnode.h"
#include "actor.h"
#include "sentient.h"
#include "g_local.h"
#include <script/scr_const.h>
#include "actor_turret.h"
#include "turret.h"

// aislop
int Actor_SightTrace(actor_s *self, const float *start, const float *end, int passEntNum)
{
    float traceStart[3];
    int traceResult[2];
    const float *traceEnd = end;

    ++self->iTraceCount;

    gentity_s *ent = self->ent;
    int entNum = ent->s.number;
    int traceFlags = 0;

    if (self->ignoreCloseFoliage)
    {
        float dx = end[0] - start[0];
        float dy = end[1] - start[1];
        float dz = end[2] - start[2];

        float distSq = dx * dx + dy * dy + dz * dz;
        float dist = sqrtf(distSq);

        if (dist == 0.0f)
            return 0;

        float invDist = 1.0f / dist;

        float dirX = dx * invDist;
        float dirY = dy * invDist;
        float dirZ = dz * invDist;

        float foliageIgnoreDist = ai_foliageIngoreDist->current.value;

        traceStart[0] = start[0] + dirX * foliageIgnoreDist;
        traceStart[1] = start[1] + dirY * foliageIgnoreDist;
        traceStart[2] = start[2] + dirZ * foliageIgnoreDist;

        // Use adjusted start position
        traceEnd = end;
        traceFlags = 41998339;  // Includes foliage
        SV_SightTrace(traceResult, traceStart, vec3_origin, vec3_origin, end, entNum, passEntNum, traceFlags);

        if (traceResult[0])
            return 0;

        // Now continue with adjusted start point
        traceStart[0] = start[0] + dirX * foliageIgnoreDist;
        traceStart[1] = start[1] + dirY * foliageIgnoreDist;
        traceStart[2] = start[2] + dirZ * foliageIgnoreDist;

        traceFlags = 41998337;  // Without foliage
    }
    else
    {
        float dx = end[0] - start[0];
        float dy = end[1] - start[1];
        float dz = end[2] - start[2];

        float distSq = dx * dx + dy * dy + dz * dz;
        float threshold = ai_foliageIngoreDist->current.value;
        float thresholdSq = threshold * threshold;

        traceFlags = (distSq >= thresholdSq) ? 41998339 : 41998337;
        traceStart[0] = start[0];
        traceStart[1] = start[1];
        traceStart[2] = start[2];
    }

    SV_SightTrace(traceResult, traceStart, vec3_origin, vec3_origin, traceEnd, entNum, passEntNum, traceFlags);

    if (traceResult[0] || SV_FX_GetVisibility(start, end) < 0.2f)
        return 0;

    return 1;
}

int __cdecl Actor_CanSeePointFrom(
    actor_s *self,
    const float *vStart,
    const float *vEnd,
    double fMaxDistSqrd,
    int ignoreEntityNum)
{
    double v11; // fp13
    double v12; // fp12
    double v13; // fp0

    if (fMaxDistSqrd < 0.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp",
            100,
            0,
            "%s\n\t(fMaxDistSqrd) = %g",
            "(fMaxDistSqrd >= 0)",
            fMaxDistSqrd);
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 101, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 102, 0, "%s", "self->ent");
    if (fMaxDistSqrd == 0.0)
        return Actor_SightTrace(self, vStart, vEnd, ignoreEntityNum);
    v11 = (float)(vEnd[2] - vStart[2]);
    v12 = (float)(vEnd[1] - vStart[1]);
    v13 = (float)((float)((float)v12 * (float)v12)
        + (float)((float)((float)(*vEnd - *vStart) * (float)(*vEnd - *vStart)) + (float)((float)v11 * (float)v11)));
    if (v13 <= fMaxDistSqrd && v13 <= level.fFogOpaqueDistSqrd)
        return Actor_SightTrace(self, vStart, vEnd, ignoreEntityNum);
    else
        return 0;
}

int __cdecl Actor_CanSeeEnemyViaClaimedNode(actor_s *self)
{
    pathnode_t *pClaimedNode; // r30
    sentient_s *TargetSentient; // r31
    const pathnode_t *v4; // r4
    int v5; // r3
    unsigned __int8 v6; // r11

    pClaimedNode = self->sentient->pClaimedNode;
    if (!pClaimedNode)
        return 0;
    TargetSentient = Actor_GetTargetSentient(self);
    if (!TargetSentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 503, 0, "%s", "enemy");
    v4 = Sentient_NearestNode(TargetSentient);
    if (!v4)
        return 0;
    v5 = Path_ExpandedNodeVisible(pClaimedNode, v4);
    v6 = 1;
    if (!v5)
        return 0;
    return v6;
}

sentient_s *__cdecl Actor_KnowAboutEnemy(actor_s *self, int hadPath)
{
    sentient_s *result; // r3
    sentient_s *v5; // r30
    int lastKnownPosTime; // r11
    unsigned __int8 v7; // r11
    bool v8; // zf

    result = Actor_GetTargetSentient(self);
    v5 = result;
    if (result)
    {
        if (hadPath || !(unsigned __int8)Actor_CanSeeEnemyViaClaimedNode(self))
        {
            lastKnownPosTime = self->sentientInfo[v5 - level.sentients].lastKnownPosTime;
            if (!lastKnownPosTime)
                return 0;
            v8 = level.time - lastKnownPosTime < 10000;
            v7 = 1;
            if (!v8)
                return 0;
            return (sentient_s *)v7;
        }
        else
        {
            return (sentient_s *)1;
        }
    }
    return result;
}

int __cdecl Actor_CanShootFrom(actor_s *self, const float *vTarget, const float *vFrom)
{
    double v6; // fp13
    double v7; // fp12
    unsigned __int16 EntityHitId; // r31
    sentient_s *sentient; // r11
    unsigned __int8 v11; // r11
    bool v12; // zf
    trace_t v13[2]; // [sp+50h] [-60h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 572, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 573, 0, "%s", "self->ent");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 574, 0, "%s", "self->sentient");
    v6 = (float)(vFrom[2] - vTarget[2]);
    v7 = (float)(vFrom[1] - vTarget[1]);
    if ((float)((float)((float)v7 * (float)v7)
        + (float)((float)((float)(*vFrom - *vTarget) * (float)(*vFrom - *vTarget)) + (float)((float)v6 * (float)v6))) > (double)level.fFogOpaqueDistSqrd)
        return 0;
    G_TraceCapsule(v13, vFrom, vec3_origin, vec3_origin, vTarget, self->ent->s.number, 33605729);
    if (v13[0].fraction == 1.0)
        return 1;
    EntityHitId = Trace_GetEntityHitId(v13);
    if (self->sentient->targetEnt.isDefined())
    {
        if (self->sentient->targetEnt.entnum() == EntityHitId)
            return 1;
    }
    sentient = g_entities[EntityHitId].sentient;
    if (!sentient)
        return 0;
    v12 = sentient->eTeam != self->sentient->eTeam;
    v11 = 1;
    if (!v12)
        return 0;
    return v11;
}

int __cdecl compare_sentient_sort(unsigned int *pe1, unsigned int *pe2)
{
    return pe2[1] - pe1[1];
}

void __cdecl Actor_UpdateLastKnownPos(actor_s *self, sentient_s *other)
{
    sentient_s *sentients; // r11
    char *v5; // r11
    char *v6; // r31

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 743, 0, "%s", "self");
    if (!other)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 744, 0, "%s", "other");
    sentients = level.sentients;
    if (other < level.sentients || other >= &level.sentients[33])
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp",
            745,
            0,
            "%s",
            "other >= level.sentients && other < level.sentients + MAX_SENTIENTS");
        sentients = level.sentients;
    }
    if (other != &sentients[other - sentients])
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp",
            746,
            0,
            "%s",
            "other == level.sentients + (other - level.sentients)");
        sentients = level.sentients;
    }
    v5 = (char *)self + 40 * (other - sentients);
    v6 = v5 + 2100;
    *((unsigned int *)v5 + 529) = level.time;
    Sentient_GetOrigin(other, (float *)v5 + 531);
    *((unsigned int *)v6 + 9) = (unsigned int)other->pNearestNode;
}

void __cdecl Actor_UpdateLastEnemySightPos(actor_s *self)
{
    const sentient_s *TargetSentient; // r3
    char *v3; // r11

    TargetSentient = Actor_GetTargetSentient(self);
    if (TargetSentient)
    {
        v3 = (char *)self + 40 * (TargetSentient - level.sentients);
        if (v3[2100])
        {
            if (*((unsigned int *)v3 + 527) == level.time)
            {
                self->lastEnemySightPosValid = 1;
                Sentient_GetEyePosition(TargetSentient, self->lastEnemySightPos);
            }
        }
    }
}

void __cdecl Actor_UpdateEyeInformation(actor_s *self)
{
    gentity_s *ent; // r3
    double v3; // fp13
    double v4; // fp12
    double v5; // fp11
    double v6; // fp10
    gentity_s *v7; // r11
    double v8; // fp13
    float v9[20]; // [sp+50h] [-50h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 795, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 796, 0, "%s", "self->ent");
    if (self->eyeInfo.time != level.time)
    {
        //Profile_Begin(229);
        ent = self->ent;
        self->eyeInfo.time = level.time;
        if (G_DObjGetWorldTagMatrix(ent, scr_const.tag_eye, (float (*)[3])v9))
        {
            v3 = v9[10];
            v4 = v9[11];
            v5 = v9[0];
            v6 = v9[1];
            self->eyeInfo.pos[0] = v9[9];
            self->eyeInfo.pos[1] = v3;
            self->eyeInfo.pos[2] = v4;
            self->eyeInfo.dir[0] = v5;
            self->eyeInfo.dir[1] = v6;
            Vec2Normalize(self->eyeInfo.dir);
            self->eyeInfo.dir[2] = 0.0;
        }
        else
        {
            Com_Printf(18, "Actor_UpdateEyeInformation: Actor dobj doesn't have TAG_EYE.\n");
            v7 = self->ent;
            self->eyeInfo.pos[0] = self->ent->r.currentOrigin[0];
            self->eyeInfo.pos[1] = v7->r.currentOrigin[1];
            v8 = v7->r.currentOrigin[2];
            self->eyeInfo.pos[2] = v7->r.currentOrigin[2];
            self->eyeInfo.pos[2] = (float)64.0 + (float)v8;
            self->eyeInfo.dir[0] = self->vLookForward[0];
            self->eyeInfo.dir[1] = self->vLookForward[1];
            self->eyeInfo.dir[2] = self->vLookForward[2];
        }
        //Profile_EndInternal(0);
    }
}

void __cdecl Actor_GetEyePosition(actor_s *self, float *vEyePosOut)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 843, 0, "%s", "self");
    if (!vEyePosOut)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 844, 0, "%s", "vEyePosOut");
    Actor_UpdateEyeInformation(self);
    *vEyePosOut = self->eyeInfo.pos[0];
    vEyePosOut[1] = self->eyeInfo.pos[1];
    vEyePosOut[2] = self->eyeInfo.pos[2];
}

void __cdecl Actor_GetDebugEyePosition(actor_s *self, float *vEyePosOut)
{
    gentity_s *ent; // r8
    int v5; // r9
    float *v6; // r10
    float *absmax; // r11
    double v8; // fp0

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 870, 0, "%s", "self");
    if (!vEyePosOut)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 871, 0, "%s", "vEyePosOut");
    *vEyePosOut = self->eyeInfo.pos[0];
    ent = self->ent;
    v5 = 0;
    vEyePosOut[1] = self->eyeInfo.pos[1];
    v6 = vEyePosOut;
    absmax = ent->r.absmax;
    vEyePosOut[2] = self->eyeInfo.pos[2];
    while (1)
    {
        v8 = *v6;
        if (v8 < (float)(*(absmax - 3) - (float)32.0) || v8 >(float)(*absmax + (float)32.0))
            break;
        ++v5;
        ++v6;
        ++absmax;
        if (v5 >= 3)
            return;
    }
    *vEyePosOut = ent->r.currentOrigin[0];
    vEyePosOut[1] = ent->r.currentOrigin[1];
    vEyePosOut[2] = ent->r.currentOrigin[2] + (float)56.0;
}

void __cdecl Actor_GetEyeDirection(actor_s *self, float *vEyeDir)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 896, 0, "%s", "self");
    if (!vEyeDir)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 897, 0, "%s", "vEyeDir");
    Actor_UpdateEyeInformation(self);
    *vEyeDir = self->eyeInfo.dir[0];
    vEyeDir[1] = self->eyeInfo.dir[1];
    vEyeDir[2] = self->eyeInfo.dir[2];
}

void __cdecl Actor_GetEyeOffset(actor_s *self, float *vEyePosOut)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 913, 0, "%s", "self");
    if (!vEyePosOut)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 914, 0, "%s", "vEyePosOut");
    *vEyePosOut = 0.0;
    vEyePosOut[1] = 0.0;
    vEyePosOut[2] = 64.0;
}

int __cdecl Actor_GetMuzzleInfo(actor_s *self, float *vOrigin, float *vForward)
{
    double v7; // fp0
    double v8; // fp13
    double v9; // fp12
    double v10; // fp11
    double v11; // fp10
    double v12; // fp9
    float v13[28]; // [sp+50h] [-70h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 931, 0, "%s", "self");
    if (self->muzzleInfo.time != level.time)
    {
        //Profile_Begin(238);
        if (!G_DObjGetWorldTagMatrix(self->ent, scr_const.tag_flash, (float (*)[3])v13))
        {
            //Profile_EndInternal(0);
            return 0;
        }
        v7 = v13[9];
        v8 = v13[10];
        v9 = v13[11];
        v10 = v13[0];
        v11 = v13[1];
        self->muzzleInfo.time = level.time;
        v12 = v13[2];
        self->muzzleInfo.pos[0] = v7;
        self->muzzleInfo.pos[1] = v8;
        self->muzzleInfo.pos[2] = v9;
        self->muzzleInfo.dir[0] = v10;
        self->muzzleInfo.dir[1] = v11;
        self->muzzleInfo.dir[2] = v12;
        //Profile_EndInternal(0);
    }
    if (!vOrigin)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 950, 0, "%s", "vOrigin");
    *vOrigin = self->muzzleInfo.pos[0];
    vOrigin[1] = self->muzzleInfo.pos[1];
    vOrigin[2] = self->muzzleInfo.pos[2];
    if (vForward)
    {
        *vForward = self->muzzleInfo.dir[0];
        vForward[1] = self->muzzleInfo.dir[1];
        vForward[2] = self->muzzleInfo.dir[2];
    }
    return 1;
}

bool __cdecl PointInFovAndRange(
    actor_s *self,
    const float *vEyePos,
    const float *vPoint,
    double fovDot,
    double fMaxDistSqrd)
{
    double v6; // fp31
    double v7; // fp30
    double v8; // fp29
    double v9; // fp27
    bool result; // r3
    double v11; // fp0
    float v12[18]; // [sp+50h] [-50h] BYREF

    v6 = (float)(*vPoint - *vEyePos);
    v7 = (float)(vPoint[2] - vEyePos[2]);
    v8 = (float)(vPoint[1] - vEyePos[1]);
    v9 = (float)((float)((float)v8 * (float)v8)
        + (float)((float)((float)(*vPoint - *vEyePos) * (float)(*vPoint - *vEyePos))
            + (float)((float)v7 * (float)v7)));
    result = 0;
    if (fMaxDistSqrd == 0.0 || v9 <= fMaxDistSqrd && v9 <= level.fFogOpaqueDistSqrd)
    {
        if (fovDot == 0.0)
            return 1;
        Actor_GetEyeDirection(self, v12);
        v11 = (float)((float)(v12[1] * (float)v8) + (float)((float)((float)v6 * v12[0]) + (float)(v12[2] * (float)v7)));
        if (v11 >= 0.0
            && (float)((float)v11 * (float)v11) >= (double)(float)((float)((float)v9 * (float)fovDot) * (float)fovDot))
        {
            return 1;
        }
    }
    return result;
}

int __cdecl Actor_CanSeePointExInternal(
    actor_s *self,
    const float *vPoint,
    double fovDot,
    double fMaxDistSqrd,
    int ignoreEntityNum,
    float *vViewPos)
{
    int result; // r3

    iassert(fovDot >= 0);
    iassert(fMaxDistSqrd >= 0.0);

    float eyePos[3];
    Actor_GetEyePosition(self, eyePos);

    result = PointInFovAndRange(self, eyePos, vPoint, fovDot, fMaxDistSqrd);
    if (result)
        return Actor_SightTrace(self, eyePos, vPoint, ignoreEntityNum); // KISAKTODO double check args
    return result;
}

int __cdecl Actor_CanSeePointEx(
    actor_s *self,
    const float *vPoint,
    double fovDot,
    double fMaxDistSqrd,
    int ignoreEntityNum)
{
    float *v12; // r6
    int v13; // r5
    float v15[2]; // [sp+50h] [-60h] BYREF
    float v16[6]; // [sp+58h] [-58h] BYREF

    iassert(fovDot >= 0);
    iassert(fMaxDistSqrd >= 0.0);
    
    if (!Actor_IsUsingTurret(self) || !turret_CanTargetPoint(self->pTurret, vPoint, v16, v15))
        Actor_GetEyePosition(self, v16);

    return Actor_CanSeePointExInternal(self, vPoint, fovDot, fMaxDistSqrd, ignoreEntityNum, v16);// KISAKTODO double check args
}

void __cdecl Actor_UpdateVisCache(actor_s *self, const gentity_s *ent, sentient_info_t *pInfo, bool bVisible)
{
    bool v8; // r31
    int number; // r5

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 265, 0, "%s", "self");
    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 266, 0, "%s", "ent");
    if (!pInfo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 267, 0, "%s", "pInfo");
    v8 = pInfo->VisCache.bVisible;
    VisCache_Update(&pInfo->VisCache, bVisible);
    if (bVisible)
    {
        if (!v8)
        {
            number = self->ent->s.number;
            if (g_dumpAIEvents->current.integer == number)
                Com_Printf(18, "%d ^3 visible^7:  entity^5 %d ^7at time^5 %d\n", number, ent->s.number, level.time);
            if (ent == Actor_GetTargetEntity(self))
                Scr_Notify(self->ent, scr_const.enemy_visible, 0);
        }
        if (!Actor_IsUsingTurret(self))
            pInfo->attackTime = 0;
        Actor_UpdateLastKnownPos(self, ent->sentient);
    }
}

// aislop
int Actor_CanSeeEntityEx(actor_s *self, const gentity_s *ent, double fovDot, double fMaxDistSqrd)
{
    iassert(self);
    iassert(self->sentient);
    iassert(ent);
    iassert(fovDot);
    iassert(fMaxDistSqrd);

    float eyePosTarget[4];
    float eyePosSelf[14];
    float dummyOut[2];
    sentient_s *targetSentient = ent->sentient;
    sentient_info_t *sentientInfo = NULL;

    if (targetSentient)
    {
        sentientInfo = &self->sentientInfo[targetSentient - level.sentients];
        Sentient_GetEyePosition(targetSentient, eyePosTarget);

        if (Actor_IsUsingTurret(self))
        {
            if (turret_CanTargetPoint(self->pTurret, eyePosTarget, eyePosSelf, dummyOut) ||
                turret_CanTargetSentient(self->pTurret, targetSentient, eyePosTarget, eyePosSelf, dummyOut))
            {
                // can target; skip further checks
                goto can_see_check;
            }

            if (level.time - sentientInfo->lastKnownPosTime >= 1000 &&
                Vec2DistanceSq(eyePosTarget, self->ent->r.currentOrigin) >= 262144.0f)
            {
                return 0;
            }
        }

        Actor_GetEyePosition(self, eyePosSelf);

    can_see_check:
        double adjustedFovDot = fovDot;
        if (Actor_GetTargetSentient(self) == targetSentient ||
            (self->pFavoriteEnemy.isDefined() && self->pFavoriteEnemy.sentient() == targetSentient))
        {
            adjustedFovDot = 0.0;
        }

        float targetMaxDistSq = targetSentient->maxVisibleDist * targetSentient->maxVisibleDist;
        float effectiveMaxDistSq = (float)fMaxDistSqrd > targetMaxDistSq ? (float)fMaxDistSqrd : targetMaxDistSq;

        unsigned char canSee = Actor_CanSeePointExInternal(
            self, eyePosTarget, adjustedFovDot, effectiveMaxDistSq, ent->s.number, eyePosSelf);

        if (canSee)
        {
            if (fovDot < self->fovDot || fMaxDistSqrd > self->fMaxSightDistSqrd)
                ; // fall through
        }
        else if (fovDot > self->fovDot || fMaxDistSqrd < self->fMaxSightDistSqrd)
        {
            ; // fall through
        }

        if (targetSentient)
            Actor_UpdateVisCache(self, ent, sentientInfo, canSee);

        if (!canSee)
            return 0;
    }
    else
    {
        if (!G_DObjGetWorldTagPos(ent, scr_const.tag_eye, eyePosTarget))
            G_EntityCentroid(ent, eyePosTarget);

        unsigned char canSee = Actor_CanSeePointEx(self, eyePosTarget, fovDot, fMaxDistSqrd, ent->s.number);
        if (canSee)
        {
            if (fovDot < self->fovDot || fMaxDistSqrd > self->fMaxSightDistSqrd)
                ; // fall through
        }
        else if (fovDot > self->fovDot || fMaxDistSqrd < self->fMaxSightDistSqrd)
        {
            ; // fall through
        }

        if (!canSee)
            return 0;
    }

    if (ent->actor && !Actor_IsUsingTurret(self) && !Actor_IsUsingTurret(ent->actor))
    {
        if (!ent->sentient)
            MyAssertHandler(__FILE__, __LINE__, 0, "%s", "ent->sentient");

        float selfMaxDistSq = self->sentient->maxVisibleDist * self->sentient->maxVisibleDist;
        float actorEffectiveMaxDistSq = ent->actor->fMaxSightDistSqrd > selfMaxDistSq ?
            ent->actor->fMaxSightDistSqrd : selfMaxDistSq;

        int reciprocalCanSee = PointInFovAndRange(
            ent->actor, eyePosTarget, eyePosSelf,
            ent->actor->fovDot, actorEffectiveMaxDistSq
        );

        Actor_UpdateVisCache(
            ent->actor, self->ent,
            &ent->actor->sentientInfo[self->sentient - level.sentients],
            reciprocalCanSee
        );
    }

    return 1;
}

int __cdecl Actor_CanSeeSentientEx(
    actor_s *self,
    sentient_s *sentient,
    double fovDot,
    double fMaxDistSqrd,
    int iMaxLatency,
    int a6,
    int a7)
{
    char *v12; // r9
    int v13; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 470, 0, "%s", "self");
    if (!sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 471, 0, "%s", "sentient");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 472, 0, "%s", "self->sentient");
    if (self->sentient == sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 473, 0, "%s", "self->sentient != sentient");
    v12 = (char *)self + 40 * (sentient - level.sentients);
    v13 = *((unsigned int *)v12 + 526);
    if (v13 && v13 + a7 >= level.time && v12[2100] && fovDot <= self->fovDot && fMaxDistSqrd <= self->fMaxSightDistSqrd)
        return 1;
    else
        return Actor_CanSeeEntityEx(self, sentient->ent, fovDot, fMaxDistSqrd);
}

int __cdecl Actor_CanShootEnemy(actor_s *self)
{
    int result; // r3
    float v3[4]; // [sp+50h] [-40h] BYREF
    float v4[12]; // [sp+60h] [-30h] BYREF

    iassert(self);
    iassert(self->ent);
    iassert(self->sentient);
    iassert(self->sentient->targetEnt.isDefined());

    Actor_GetTargetLookPosition(self, v4);
    result = Actor_GetMuzzleInfo(self, v3, 0);
    if (result)
        return Actor_CanShootFrom(self, v4, v3);

    return result;
}

int __cdecl Actor_CanSeePoint(actor_s *self, const float *vPoint)
{
    return Actor_CanSeePointEx(self, vPoint, self->fovDot, self->fMaxSightDistSqrd, ENTITYNUM_NONE);
}

int __cdecl Actor_CanSeeEntityPoint(actor_s *self, const float *vPoint, const gentity_s *ent)
{
    return Actor_CanSeePointEx(self, vPoint, self->fovDot, self->fMaxSightDistSqrd, (int)ent);
}

int __cdecl Actor_CanSeeEntity(actor_s *self, const gentity_s *ent)
{
    return Actor_CanSeeEntityEx(self, ent, self->fovDot, self->fMaxSightDistSqrd);
}

int __cdecl Actor_CanSeeSentient(actor_s *self, sentient_s *sentient, int iMaxLatency)
{
    char *v3; // r9
    int v4; // r11

    v3 = (char *)self + 40 * (sentient - level.sentients);
    v4 = *((unsigned int *)v3 + 526);
    if (v4 && v4 + iMaxLatency >= level.time)
        return (unsigned __int8)v3[2100];
    else
        return Actor_CanSeeEntityEx(self, sentient->ent, self->fovDot, self->fMaxSightDistSqrd);
}

int __cdecl Actor_CanSeeEnemy(actor_s *self)
{
    sentient_s *TargetSentient; // r4
    const gentity_s *v4; // r3

    iassert(self);
    iassert(self->sentient);
    iassert(self->sentient->targetEnt.isDefined());

    TargetSentient = Actor_GetTargetSentient(self);
    if (TargetSentient)
        return Actor_CanSeeSentient(self, TargetSentient, 250);

    v4 = self->sentient->targetEnt.ent();
    return Actor_CanSeeEntityEx(self, v4, self->fovDot, self->fMaxSightDistSqrd);
}

int __cdecl Actor_CanSeeEnemyExtended(actor_s *self, int useClaimedNode)
{
    sentient_s *TargetSentient; // r30
    int iLastVisTime; // r11
    unsigned __int8 v7; // r11
    bool v8; // zf
    const gentity_s *v9; // r3

    iassert(self);
    iassert(self->sentient);
    iassert(self->sentient->targetEnt.isDefined());

    TargetSentient = Actor_GetTargetSentient(self);
    if (TargetSentient)
    {
        if (useClaimedNode && (unsigned __int8)Actor_CanSeeEnemyViaClaimedNode(self))
        {
            return 1;
        }
        else
        {
            iLastVisTime = self->sentientInfo[TargetSentient - level.sentients].VisCache.iLastVisTime;
            if (!iLastVisTime)
                return 0;
            v8 = level.time - iLastVisTime < 10000;
            v7 = 1;
            if (!v8)
                return 0;
            return v7;
        }
    }
    else
    {
        v9 = self->sentient->targetEnt.ent();
        return Actor_CanSeeEntityEx(self, v9, self->fovDot, self->fMaxSightDistSqrd);
    }
}

void __cdecl Actor_UpdateSight(actor_s *self)
{
    team_t v2; // r3
    signed int v3; // r25
    int v4; // r18
    sentient_s *Sentient; // r29
    float *v6; // r30
    gentity_s *ent; // r11
    double v8; // fp31
    int v9; // r31
    double v10; // fp1
    __int64 v11; // r11
    int iTraceCount; // r29
    signed int v13; // r30
    const gentity_s ***v14; // r31
    int iLastUpdateTime; // r11
    float v16; // [sp+50h] [-1B0h] BYREF
    float v17; // [sp+54h] [-1ACh]
    float v18; // [sp+58h] [-1A8h]
    float v19; // [sp+5Ch] [-1A4h]
    __int64 v20; // [sp+60h] [-1A0h]
    _BYTE v21[264]; // [sp+70h] [-190h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 658, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 659, 0, "%s", "self->sentient");
    //Profile_Begin(230);
    v2 = Sentient_EnemyTeam(self->sentient->eTeam);
    if (v2)
    {
        v3 = 0;
        v4 = 1 << v2;
        Sentient = Sentient_FirstSentient(1 << v2);
        if (Sentient)
        {
            v6 = (float *)v21;
            do
            {
                if (!Sentient->ent)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 691, 0, "%s", "sentient->ent");
                if (Sentient->ent->s.number == self->ent->s.number)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp",
                        692,
                        0,
                        "%s",
                        "sentient->ent->s.number != self->ent->s.number");
                Sentient_GetOrigin(Sentient, &v16);
                ent = self->ent;
                v16 = v16 - self->ent->r.currentOrigin[0];
                v17 = v17 - ent->r.currentOrigin[1];
                v8 = (float)((float)(v16 * v16)
                    + (float)((float)((float)(v18 - ent->r.currentOrigin[2]) * (float)(v18 - ent->r.currentOrigin[2]))
                        + (float)(v17 * v17)));
                v18 = v18 - ent->r.currentOrigin[2];
                v19 = v8;
                if ((LODWORD(v19) & 0x7F800000) == 0x7F800000)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_senses.cpp", 698, 0, "%s", "!IS_NAN(fDistSqrd)");
                if (v8 != 0.0)
                {
                    v9 = level.time - self->sentientInfo[Sentient - level.sentients].VisCache.iLastUpdateTime - 100;
                    v10 = I_rsqrt(v8);
                    *(unsigned int *)v6 = (unsigned int)Sentient;
                    ++v3;
                    LODWORD(v11) = v9 & ~(v9 >> 31);
                    v20 = v11;
                    v6[1] = (float)v10 * (float)v11;
                    v6 += 2;
                }
                Sentient = Sentient_NextSentient(Sentient, v4);
            } while (Sentient);
            if (v3 > 1)
                qsort(v21, v3, 8u, (int(__cdecl *)(const void *, const void *))compare_sentient_sort);
        }
        iTraceCount = self->iTraceCount;
        v13 = 0;
        if (v3 > 0)
        {
            v14 = (const gentity_s ***)v21;
            do
            {
                iLastUpdateTime = self->sentientInfo[((char *)*v14 - (char *)level.sentients) / 116].VisCache.iLastUpdateTime;
                if (!iLastUpdateTime || iLastUpdateTime < level.time)
                    Actor_CanSeeEntityEx(self, **v14, self->fovDot, self->fMaxSightDistSqrd);
                if (self->iTraceCount != iTraceCount)
                    break;
                ++v13;
                v14 += 2;
            } while (v13 < v3);
        }
    }
    //Profile_EndInternal(0);
}

