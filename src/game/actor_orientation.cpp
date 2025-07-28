#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_orientation.h"
#include "g_main.h"
#include <universal/com_math.h>
#include "actor_threat.h"
#include "actor_senses.h"

void __cdecl Actor_SetDesiredLookAngles(ai_orient_t *pOrient, double fPitch, double fYaw)
{
    if (!pOrient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 24, 0, "%s", "pOrient");
    pOrient->fDesiredLookPitch = AngleNormalize360(fPitch);
    pOrient->fDesiredLookYaw = AngleNormalize360(fYaw);
}

void __cdecl Actor_SetDesiredBodyAngle(ai_orient_t *pOrient, double fAngle)
{
    if (!pOrient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 41, 0, "%s", "pOrient");
    pOrient->fDesiredBodyYaw = AngleNormalize360(fAngle);
}

void __cdecl Actor_SetDesiredAngles(ai_orient_t *pOrient, double fPitch, double fYaw)
{
    Actor_SetDesiredLookAngles(pOrient, fPitch, fYaw);
    if (!pOrient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 41, 0, "%s", "pOrient");
    pOrient->fDesiredBodyYaw = AngleNormalize360(fYaw);
}

void __cdecl Actor_SetLookAngles(actor_s *self, double fPitch, double fYaw)
{
    double v6; // fp1
    double v7; // fp1
    float v8[4]; // [sp+50h] [-30h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 70, 0, "%s", "self");
    v6 = AngleNormalize360(fPitch);
    self->fLookPitch = v6;
    v8[0] = v6;
    v7 = AngleNormalize360(fYaw);
    self->fLookYaw = v7;
    v8[1] = v7;
    v8[2] = 0.0;
    AngleVectors(v8, self->vLookForward, self->vLookRight, self->vLookUp);
}

void __cdecl Actor_SetBodyAngle(actor_s *self, double fAngle)
{
    gentity_s *ent; // r29

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 94, 0, "%s", "self");
    ent = self->ent;
    if (!ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 97, 0, "%s", "ent");
    ent->r.currentAngles[0] = 0.0;
    ent->r.currentAngles[1] = AngleNormalize360(fAngle);
    ent->r.currentAngles[2] = 0.0;
}

void __cdecl Actor_ChangeAngles(actor_s *self, double fPitch, double fYaw)
{
    ai_orient_t *p_CodeOrient; // r30
    double v7; // fp29
    double v8; // fp29

    p_CodeOrient = &self->CodeOrient;
    v7 = (float)(self->CodeOrient.fDesiredBodyYaw + (float)fYaw);
    Actor_SetDesiredLookAngles(&self->CodeOrient, (float)(self->CodeOrient.fDesiredLookPitch + (float)fPitch), v7);
    if (!p_CodeOrient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 41, 0, "%s", "pOrient");
    p_CodeOrient->fDesiredBodyYaw = AngleNormalize360(v7);
    v8 = (float)(self->ScriptOrient.fDesiredBodyYaw + (float)fYaw);
    Actor_SetDesiredLookAngles(&self->ScriptOrient, (float)(self->ScriptOrient.fDesiredLookPitch + (float)fPitch), v8);
    if (self == (actor_s *)-280)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 41, 0, "%s", "pOrient");
    self->ScriptOrient.fDesiredBodyYaw = AngleNormalize360(v8);
    Actor_SetBodyAngle(self, (float)(self->ent->r.currentAngles[1] + (float)fYaw));
    Actor_SetLookAngles(self, (float)(self->fLookPitch + (float)fPitch), (float)(self->fLookYaw + (float)fYaw));
}

void __cdecl Actor_UpdateLookAngles(actor_s *self)
{
    ai_orient_t *p_ScriptOrient; // r30
    double v4; // fp31
    long double v5; // fp2
    double v6; // fp0
    double v7; // fp25
    double v8; // fp31
    long double v9; // fp2
    double v10; // fp0

    double a2;

    p_ScriptOrient = &self->ScriptOrient;
    if (self->ScriptOrient.eMode == AI_ORIENT_INVALID)
        p_ScriptOrient = &self->CodeOrient;
    v4 = (float)((float)(p_ScriptOrient->fDesiredLookPitch - self->fLookPitch) * (float)0.0027777778);
    a2 = (float)((float)((float)(p_ScriptOrient->fDesiredLookPitch - self->fLookPitch) * (float)0.0027777778) + (float)0.5);
    v5 = floor(a2);
    v6 = (float)((float)((float)v4 - (float)*(double *)&v5) * (float)360.0);
    if (v6 <= 40.0)
    {
        if (v6 < -40.0)
            v6 = -40.0;
    }
    else
    {
        v6 = 40.0;
    }
    v7 = (float)(self->fLookPitch + (float)v6);
    v8 = (float)((float)(p_ScriptOrient->fDesiredLookYaw - self->fLookYaw) * (float)0.0027777778);
    *(double *)&v5 = (float)((float)((float)(p_ScriptOrient->fDesiredLookYaw - self->fLookYaw) * (float)0.0027777778)
        + (float)0.5);
    v9 = floor(v5);
    v10 = (float)((float)((float)v8 - (float)*(double *)&v9) * (float)360.0);
    if (v10 <= 40.0)
    {
        if (v10 < -40.0)
            v10 = -40.0;
    }
    else
    {
        v10 = 40.0;
    }
    Actor_SetLookAngles(self, v7, (float)(self->fLookYaw + (float)v10));
}

void __cdecl Actor_UpdateBodyAngle(actor_s *self)
{
    double fDesiredBodyYaw; // fp0
    gentity_s *ent; // r11
    double v5; // fp31
    long double v6; // fp2
    double v7; // fp31
    double v8; // fp13
    double v9; // fp0

    double a2;

    if (self->ScriptOrient.eMode)
        fDesiredBodyYaw = self->ScriptOrient.fDesiredBodyYaw;
    else
        fDesiredBodyYaw = self->CodeOrient.fDesiredBodyYaw;
    ent = self->ent;
    self->fDesiredBodyYaw = fDesiredBodyYaw;
    v5 = (float)((float)((float)fDesiredBodyYaw - ent->r.currentAngles[1]) * (float)0.0027777778);
    a2 = (float)((float)((float)((float)fDesiredBodyYaw - ent->r.currentAngles[1]) * (float)0.0027777778) + (float)0.5);
    v6 = floor(a2);
    v7 = (float)((float)((float)v5 - (float)*(double *)&v6) * (float)360.0);
    if (BG_ActorIsProne(&self->ProneInfo, level.time))
        v8 = 0.035999998;
    else
        v8 = 0.30000001;
    v9 = (float)((float)v8 * (float)50.0);
    if (v7 > v9 || (v9 = (float)((float)v8 * (float)-50.0), v7 < v9))
        v7 = v9;
    Actor_SetBodyAngle(self, (float)(self->ent->r.currentAngles[1] + (float)v7));
}

void __cdecl Actor_FaceVector(ai_orient_t *pOrient, const float *v)
{
    double v4; // fp31
    float v5; // [sp+50h] [-40h] BYREF
    float v6; // [sp+54h] [-3Ch]

    if (*v == 0.0 && v[1] == 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 207, 0, "%s", "v[0] || v[1]");
    vectoangles(v, &v5);
    v4 = v6;
    Actor_SetDesiredLookAngles(pOrient, v5, v6);
    if (!pOrient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 41, 0, "%s", "pOrient");
    pOrient->fDesiredBodyYaw = AngleNormalize360(v4);
}

void __cdecl Actor_FaceMotion(actor_s *self, ai_orient_t *pOrient)
{
    float *v4; // r3
    float v5; // [sp+50h] [-40h] BYREF
    float v6; // [sp+54h] [-3Ch]
    float v7[4]; // [sp+60h] [-30h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 226, 0, "%s", "self");
    if (Actor_HasPath(self) && self->eAnimMode == AI_ANIM_MOVE_CODE)
    {
        if (self->moveMode
            && Path_CompleteLookahead(&self->Path)
            && self->Path.fLookaheadDist < 60.0
            && (float)((float)(self->vLookForward[1] * self->Path.lookaheadDir[1])
                + (float)(self->Path.lookaheadDir[0] * self->vLookForward[0])) < 0.0
            || (Actor_GetMoveHistoryAverage(self, &v5), v5 == 0.0) && v6 == 0.0
            || !self->moveMode
            && (Vec2Normalize(&v5),
                YawVectors(pOrient->fDesiredBodyYaw, v4, v7),
                (float)((float)(v7[0] * v5) + (float)(v7[1] * v6)) >= 0.89999998))
        {
            Actor_SetDesiredAngles(pOrient, self->ent->r.currentAngles[0], self->ent->r.currentAngles[1]);
        }
        else
        {
            Actor_FaceVector(pOrient, &v5);
        }
    }
}

void __cdecl Actor_SetAnglesToLikelyEnemyPath(actor_s *self)
{
    const pathnode_t *faceLikelyEnemyPathNode; // r10
    gentity_s *ent; // r11
    float v4; // [sp+50h] [-30h] BYREF
    float v5; // [sp+54h] [-2Ch]
    float v6; // [sp+58h] [-28h]

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 265, 0, "%s", "self");
    if (!self->faceLikelyEnemyPathNode)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp",
            266,
            0,
            "%s",
            "self->faceLikelyEnemyPathNode");
    faceLikelyEnemyPathNode = self->faceLikelyEnemyPathNode;
    ent = self->ent;
    v4 = faceLikelyEnemyPathNode->constant.vOrigin[0] - self->ent->r.currentOrigin[0];
    v5 = faceLikelyEnemyPathNode->constant.vOrigin[1] - ent->r.currentOrigin[1];
    v6 = faceLikelyEnemyPathNode->constant.vOrigin[2] - ent->r.currentOrigin[2];
    if ((float)((float)(v5 * v5) + (float)(v4 * v4)) < 1.0)
    {
        self->anglesToLikelyEnemyPath[0] = ent->r.currentAngles[0];
        self->anglesToLikelyEnemyPath[1] = ent->r.currentAngles[1];
        self->anglesToLikelyEnemyPath[2] = ent->r.currentAngles[2];
    }
    else
    {
        vectoangles(&v4, self->anglesToLikelyEnemyPath);
    }
}

const pathnode_t *__cdecl Actor_GetAnglesToLikelyEnemyPath(actor_s *self)
{
    sentient_s *sentient; // r11
    const pathnode_t *result; // r3
    team_t v4; // r30
    const pathnode_t *v5; // r3
    sentient_s *v6; // r29
    double v7; // fp31
    int v8; // r27
    sentient_s *v9; // r30
    double v10; // fp0
    double v11; // fp13
    double v12; // fp12
    double v13; // fp0
    int v14; // r11
    sentient_info_t *v15; // r5
    int v16; // r11
    float v17[4]; // [sp+50h] [-50h] BYREF

    sentient = self->sentient;
    if (sentient->bIgnoreAll)
        return 0;
    v4 = Sentient_EnemyTeam(sentient->eTeam);
    if (v4 == TEAM_FREE)
        return 0;
    if (!self->faceLikelyEnemyPathNode)
    {
        if (self->faceLikelyEnemyPathNeedRecalculateTime <= level.time)
            goto LABEL_10;
        return 0;
    }
    if (self->faceLikelyEnemyPathNeedCheckTime <= level.time)
    {
        if (self->faceLikelyEnemyPathNeedRecalculateTime > level.time)
        {
            v5 = Sentient_NearestNode(self->sentient);
            if (Path_NodesVisible(v5, self->faceLikelyEnemyPathNode))
            {
                Actor_SetAnglesToLikelyEnemyPath(self);
            LABEL_25:
                self->faceLikelyEnemyPathNeedCheckTime = level.time + 500;
                return (const pathnode_t *)1;
            }
        }
    LABEL_10:
        v6 = 0;
        v7 = FLT_MAX;
        self->faceLikelyEnemyPathNode = 0;
        v8 = 1 << v4;
        v9 = Sentient_FirstSentient(1 << v4);
        if (!v9)
            goto LABEL_18;
        do
        {
            if (!Actor_CheckIgnore(self->sentient, v9))
            {
                Sentient_GetOrigin(v9, v17);
                v10 = (float)(self->ent->r.currentOrigin[0] - v17[0]);
                v11 = (float)(self->ent->r.currentOrigin[2] - v17[2]);
                v12 = (float)(self->ent->r.currentOrigin[1] - v17[1]);
                v13 = (float)((float)((float)v12 * (float)v12)
                    + (float)((float)((float)v10 * (float)v10) + (float)((float)v11 * (float)v11)));
                if (v9->ent->client)
                    v13 = (float)((float)v13 * (float)0.25);
                if (v7 > v13)
                {
                    v7 = v13;
                    v6 = v9;
                }
            }
            v9 = Sentient_NextSentient(v9, v8);
        } while (v9);
        if (!v6)
        {
        LABEL_18:
            result = 0;
            v14 = level.time + 500;
            self->faceLikelyEnemyPathNeedRecalculateTime = level.time + 500;
            self->faceLikelyEnemyPathNeedCheckTime = v14;
            return result;
        }
        if (v7 <= (float)(v6->maxVisibleDist * v6->maxVisibleDist))
            v15 = 0;
        else
            v15 = &self->sentientInfo[v6 - level.sentients];
        result = Path_FindFacingNode(self->sentient, v6, v15);
        if (!result)
        {
            v16 = level.time + 3000;
            self->faceLikelyEnemyPathNeedRecalculateTime = level.time + 3000;
            self->faceLikelyEnemyPathNeedCheckTime = v16;
            return result;
        }
        self->faceLikelyEnemyPathNode = result;
        Actor_SetAnglesToLikelyEnemyPath(self);
        self->faceLikelyEnemyPathNeedRecalculateTime = level.time + 3000;
        goto LABEL_25;
    }
    return (const pathnode_t *)1;
}

void __cdecl Actor_FaceLikelyEnemyPath(actor_s *self, ai_orient_t *pOrient)
{
    sentient_s *sentient; // r11
    const pathnode_t *pClaimedNode; // r30

    if (self->faceLikelyEnemyPathNeedCheckTime <= level.time)
    {
        if ((unsigned __int8)Actor_GetAnglesToLikelyEnemyPath(self))
        {
            Actor_SetDesiredAngles(pOrient, self->anglesToLikelyEnemyPath[0], self->anglesToLikelyEnemyPath[1]);
        }
        else
        {
            sentient = self->sentient;
            pClaimedNode = sentient->pClaimedNode;
            if (pClaimedNode && Path_IsValidClaimNode(sentient->pClaimedNode))
            {
                if (Actor_PointNearNode(self->ent->r.currentOrigin, pClaimedNode))
                    Actor_SetDesiredAngles(pOrient, 0.0, pClaimedNode->constant.fAngle);
            }
        }
    }
}

// local variable allocation has failed, the output may be wrong!
void __cdecl Actor_FaceEnemy(actor_s *self, ai_orient_t *pOrient)
{
    actor_s *v4; // r3
    bool v5; // zf
    sentient_s *TargetSentient; // r3
    sentient_s *v7; // r28
    sentient_info_t *v8; // r30
    gentity_s *ent; // r11
    double v10; // fp0
    double v11; // fp13
    double v12; // fp12
    __int64 v13; // r9 OVERLAPPED
    int iPathTime; // r10
    const pathnode_t *FacingNode; // r3
    gentity_s *v16; // r11
    gentity_s *v17; // r11
    float v18; // [sp+60h] [-40h] BYREF
    float v19; // [sp+64h] [-3Ch]
    float v20; // [sp+68h] [-38h]

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 439, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 440, 0, "%s", "self->sentient");
    v5 = Actor_GetTargetEntity(self) != 0;
    v4 = self;
    if (!v5)
        goto LABEL_6;
    TargetSentient = Actor_GetTargetSentient(self);
    v7 = TargetSentient;
    if (!TargetSentient)
        goto LABEL_19;
    v8 = &self->sentientInfo[TargetSentient - level.sentients];
    if (v8->VisCache.bVisible)
        goto LABEL_19;
    if (self->species)
    {
        ent = self->ent;
        v10 = (float)(self->sentientInfo[TargetSentient - level.sentients].vLastKnownPos[0] - self->ent->r.currentOrigin[0]);
        v18 = self->sentientInfo[TargetSentient - level.sentients].vLastKnownPos[0] - self->ent->r.currentOrigin[0];
        v11 = (float)(self->sentientInfo[TargetSentient - level.sentients].vLastKnownPos[1] - ent->r.currentOrigin[1]);
        v19 = self->sentientInfo[TargetSentient - level.sentients].vLastKnownPos[1] - ent->r.currentOrigin[1];
        v12 = (float)(self->sentientInfo[TargetSentient - level.sentients].vLastKnownPos[2] - ent->r.currentOrigin[2]);
        goto LABEL_20;
    }
    v4 = self;
    if (!v8->VisCache.iLastVisTime)
    {
    LABEL_6:
        Actor_FaceLikelyEnemyPath(v4, pOrient);
        return;
    }
    if (Actor_HasPath(self))
    {
        iPathTime = self->Path.iPathTime;
        LODWORD(v13) = level.time;
        if ((float)((float)*(__int64 *)((char *)&v13 - 4) + (float)300.0) < (double)(float)v13
            && level.time - v8->VisCache.iLastVisTime > 500)
        {
            FacingNode = Path_FindFacingNode(self->sentient, v7, v8);
            if (FacingNode)
            {
                v16 = self->ent;
                v10 = (float)(FacingNode->constant.vOrigin[0] - self->ent->r.currentOrigin[0]);
                v18 = FacingNode->constant.vOrigin[0] - self->ent->r.currentOrigin[0];
                v11 = (float)(FacingNode->constant.vOrigin[1] - v16->r.currentOrigin[1]);
                v19 = FacingNode->constant.vOrigin[1] - v16->r.currentOrigin[1];
                v12 = (float)(FacingNode->constant.vOrigin[2] - v16->r.currentOrigin[2]);
                goto LABEL_20;
            }
        }
    LABEL_19:
        Actor_GetTargetPosition(self, &v18);
        v17 = self->ent;
        v10 = (float)(v18 - self->ent->r.currentOrigin[0]);
        v18 = v18 - self->ent->r.currentOrigin[0];
        v11 = (float)(v19 - v17->r.currentOrigin[1]);
        v19 = v19 - v17->r.currentOrigin[1];
        v12 = (float)(v20 - v17->r.currentOrigin[2]);
    LABEL_20:
        v20 = v12;
        if ((float)((float)((float)v10 * (float)v10) + (float)((float)v11 * (float)v11)) >= 1.0)
            Actor_FaceVector(pOrient, &v18);
        return;
    }
    if (level.time - v8->VisCache.iLastVisTime >= 10000)
        Actor_FaceLikelyEnemyPath(self, pOrient);
}

// aislop
int Actor_FaceGoodShootPos(actor_s *self, ai_orient_t *pOrient)
{
    if (!self->goodShootPosValid)
    {
        return 0;
    }

    float *origin = self->ent->r.currentOrigin;

    float dx = self->goodShootPos[0] - origin[0];
    float dy = self->goodShootPos[1] - origin[1];
    float dz = self->goodShootPos[2] - origin[2];

    float distSq = dx * dx + dy * dy + dz * dz;

    if (distSq == 0.0f)
    {
        return 0;
    }

    float invLen = 1.0f / sqrtf(distSq);

    float dir[3] =
    {
        dx * invLen,
        dy * invLen,
        dz * invLen
    };

    if (!Path_MayFaceEnemy(&self->Path, dir, origin))
    {
        return 0;
    }

    Actor_FaceVector(pOrient, dir);
    return 1;
}

// aislop
void Actor_FaceEnemyOrMotion(actor_s *self, ai_orient_t *pOrient)
{
    iassert(self);
    iassert(pOrient);

    // If actor is not moving or doesn't have a path, face enemy directly
    if (!Actor_HasPath(self) ||
        (self->Physics.vVelocity[0] == 0.0f && self->Physics.vVelocity[1] == 0.0f))
    {
        Actor_FaceEnemy(self, pOrient);
        return;
    }

    // Check whether there's an enemy target, it's visible, and within range
    if (!Actor_GetTargetEntity(self) || !Actor_CanSeeEnemy(self))
    {
        Actor_FaceMotion(self, pOrient);
        return;
    }

    float targetPos[3];
    Actor_GetTargetPosition(self, targetPos);

    float *origin = self->ent->r.currentOrigin;

    float dx = targetPos[0] - origin[0];
    float dy = targetPos[1] - origin[1];
    float dz = targetPos[2] - origin[2];

    float distSq = dx * dx + dy * dy + dz * dz;

    // If target is farther than 350 units (squared: 122500), just face motion
    if (distSq > 122500.0f)
    {
        Actor_FaceMotion(self, pOrient);
        return;
    }

    float length = sqrtf(distSq);

    if (length == 0.0f)
    {
        Actor_FaceMotion(self, pOrient);
        return;
    }

    float invLen = 1.0f / length;

    float dir[3] =
    {
        dx * invLen,
        dy * invLen,
        dz * invLen
    };

    if (!Path_MayFaceEnemy(&self->Path, dir, origin))
    {
        Actor_FaceMotion(self, pOrient);
        return;
    }

    Actor_FaceEnemy(self, pOrient);
}


// aislop
void Actor_FaceEnemyOrMotionSidestep(actor_s *self, ai_orient_t *pOrient)
{
    iassert(self);
    iassert(pOrient);

    // If actor is not moving or has no path, face enemy directly
    if (!Actor_HasPath(self) ||
        (self->Physics.vVelocity[0] == 0.0f && self->Physics.vVelocity[1] == 0.0f))
    {
        Actor_FaceEnemy(self, pOrient);
        return;
    }

    // Otherwise, face in direction of motion toward target (if valid)
    float targetPos[2];
    float targetZ;
    Actor_GetTargetPosition(self, targetPos);
    targetZ = *(float *)&targetPos[1]; // assuming 3rd value returned into stack after v16

    float *currentOrigin = self->ent->r.currentOrigin;

    float dx = targetPos[0] - currentOrigin[0];
    float dy = targetPos[1] - currentOrigin[1];
    float dz = targetZ - currentOrigin[2];

    float lengthSq = dx * dx + dy * dy + dz * dz;
    if (lengthSq == 0.0f)
    {
        Actor_FaceMotion(self, pOrient);
        return;
    }

    float invLength = 1.0f / sqrtf(lengthSq);
    float dir[3] =
    {
        dx * invLength,
        dy * invLength,
        dz * invLength
    };

    if (!Path_MayFaceEnemy(&self->Path, dir, currentOrigin))
    {
        Actor_FaceMotion(self, pOrient);
        return;
    }

    Actor_FaceEnemy(self, pOrient);
}

void __cdecl Actor_DecideOrientation(actor_s *self)
{
    ai_orient_mode_t eMode; // r6
    ai_orient_t *p_ScriptOrient; // r30
    sentient_s *sentient; // r11
    pathnode_t *pClaimedNode; // r31
    const char *v6; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 632, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 633, 0, "%s", "self->sentient");
    eMode = self->ScriptOrient.eMode;
    p_ScriptOrient = &self->ScriptOrient;
    if (eMode == AI_ORIENT_INVALID)
        p_ScriptOrient = &self->CodeOrient;
    switch (p_ScriptOrient->eMode)
    {
    case AI_ORIENT_DONT_CHANGE:
        return;
    case AI_ORIENT_TO_MOTION:
        Actor_FaceMotion(self, p_ScriptOrient);
        break;
    case AI_ORIENT_TO_ENEMY:
        Actor_FaceEnemy(self, p_ScriptOrient);
        break;
    case AI_ORIENT_TO_ENEMY_OR_MOTION:
        Actor_FaceEnemyOrMotion(self, p_ScriptOrient);
        break;
    case AI_ORIENT_TO_ENEMY_OR_MOTION_SIDESTEP:
        Actor_FaceEnemyOrMotionSidestep(self, p_ScriptOrient);
        break;
    case AI_ORIENT_TO_GOAL:
        sentient = self->sentient;
        pClaimedNode = sentient->pClaimedNode;
        if (pClaimedNode)
        {
            if (Path_IsValidClaimNode(sentient->pClaimedNode))
                Actor_SetDesiredAngles(p_ScriptOrient, 0.0, pClaimedNode->constant.fAngle);
        }
        break;
    default:
        if (!alwaysfails)
        {
            v6 = va("invalid orient mode %i (code = %i, script = %i)", p_ScriptOrient->eMode, self->CodeOrient.eMode, eMode);
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 668, 0, v6);
        }
        break;
    }
}

void __cdecl Actor_SetOrientMode(actor_s *self, ai_orient_mode_t eMode)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp", 683, 0, "%s", "self");
    if (eMode <= AI_ORIENT_INVALID || eMode >= AI_ORIENT_COUNT)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_orientation.cpp",
            684,
            0,
            "%s",
            "eMode > AI_ORIENT_INVALID && eMode < AI_ORIENT_COUNT");
    self->CodeOrient.eMode = eMode;
}

void __cdecl Actor_ClearScriptOrient(actor_s *self)
{
    ai_orient_t *p_CodeOrient; // r11
    ai_orient_mode_t v2; // r9
    float v3; // r8
    float v4; // r7
    float v5; // r11
    ai_orient_mode_t eMode; // r9
    float fDesiredLookPitch; // r8
    float fDesiredLookYaw; // r7
    float fDesiredBodyYaw; // r6

    p_CodeOrient = &self->CodeOrient;
    if (self->ScriptOrient.eMode)
    {
        eMode = self->ScriptOrient.eMode;
        fDesiredLookPitch = self->ScriptOrient.fDesiredLookPitch;
        fDesiredLookYaw = self->ScriptOrient.fDesiredLookYaw;
        fDesiredBodyYaw = self->ScriptOrient.fDesiredBodyYaw;
        self->ScriptOrient.eMode = AI_ORIENT_INVALID;
        p_CodeOrient->eMode = eMode;
        self->CodeOrient.fDesiredLookPitch = fDesiredLookPitch;
        self->CodeOrient.fDesiredLookYaw = fDesiredLookYaw;
        self->CodeOrient.fDesiredBodyYaw = fDesiredBodyYaw;
    }
    else
    {
        v2 = p_CodeOrient->eMode;
        v3 = self->CodeOrient.fDesiredLookPitch;
        v4 = self->CodeOrient.fDesiredLookYaw;
        v5 = self->CodeOrient.fDesiredBodyYaw;
        self->ScriptOrient.eMode = v2;
        self->ScriptOrient.fDesiredLookPitch = v3;
        self->ScriptOrient.fDesiredLookYaw = v4;
        self->ScriptOrient.fDesiredBodyYaw = v5;
        self->ScriptOrient.eMode = AI_ORIENT_INVALID;
    }
}

