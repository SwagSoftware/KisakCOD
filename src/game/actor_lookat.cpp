#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_lookat.h"
#include <xanim/xanim.h>
#include "g_main.h"
#include "g_local.h"

void __cdecl Actor_InitLookAt(actor_s *self)
{
    self->lookAtInfo.bDoLookAt = 0;
    self->lookAtInfo.vLookAtPos[0] = 0.0;
    self->lookAtInfo.vLookAtPos[1] = 0.0;
    self->lookAtInfo.vLookAtPos[2] = 0.0;
    self->lookAtInfo.bLookAtSetup = 0;
    self->lookAtInfo.fLookAtTurnAngle = 0.0;
    self->lookAtInfo.fLookAtTurnSpeed = 0.0;
    self->lookAtInfo.fLookAtTurnAccel = 120.0;
    self->lookAtInfo.fLookAtAnimYawLimit = 90.0;
    self->lookAtInfo.fLookAtYawLimit = 90.0;
}

void __cdecl Actor_SetLookAtAnimNodes(
    actor_s *self,
    unsigned __int16 animStraight,
    unsigned __int16 animLeft,
    unsigned __int16 animRight)
{
    XAnimTree_s *ActorAnimTree; // r3
    XAnimTree_s *v9; // r29
    unsigned int animLookAtStraight; // r30
    DObj_s *ServerDObj; // r3
    int v12; // r7
    unsigned int v13; // r6
    unsigned int v14; // r5
    int time; // r11

    ActorAnimTree = G_GetActorAnimTree(self);
    self->lookAtInfo.animLookAtStraight = animStraight;
    self->lookAtInfo.animLookAtLeft = animLeft;
    v9 = ActorAnimTree;
    self->lookAtInfo.animLookAtRight = animRight;
    XAnimClearTreeGoalWeights(ActorAnimTree, animLeft, 0.0);
    XAnimClearTreeGoalWeights(v9, self->lookAtInfo.animLookAtRight, 0.0);
    animLookAtStraight = self->lookAtInfo.animLookAtStraight;
    ServerDObj = Com_GetServerDObj(self->ent->s.number);
    XAnimSetCompleteGoalWeight(ServerDObj, animLookAtStraight, 1.0, 0.0, 1.0, v14, v13, v12);
    self->lookAtInfo.fLookAtAnimBlendRate = 0.0;
    self->lookAtInfo.fLookAtLimitBlendRate = 0.0;
    time = level.time;
    self->lookAtInfo.bLookAtSetup = 1;
    self->lookAtInfo.iLookAtBlendEndTime = time;
}

void __cdecl Actor_SetLookAt(actor_s *self, float *vPosition, double fTurnAccel)
{
    double fLookAtTurnAccel; // fp13

    self->lookAtInfo.vLookAtPos[0] = *vPosition;
    self->lookAtInfo.vLookAtPos[1] = vPosition[1];
    self->lookAtInfo.vLookAtPos[2] = vPosition[2];
    if (fTurnAccel > 20.0)
        self->lookAtInfo.fLookAtTurnAccel = fTurnAccel;
    fLookAtTurnAccel = self->lookAtInfo.fLookAtTurnAccel;
    self->lookAtInfo.bDoLookAt = 1;
    if (fLookAtTurnAccel < 20.0)
        self->lookAtInfo.fLookAtTurnAccel = 20.0;
}

float __cdecl Actor_CurrentLookAtAnimYawMax(actor_s *self)
{
    __int64 v1; // r11
    double v2; // fp1
    double v3; // fp0

    HIDWORD(v1) = self->lookAtInfo.iLookAtBlendEndTime;
    LODWORD(v1) = HIDWORD(v1) - level.time;
    if (HIDWORD(v1) - level.time < 0)
        LODWORD(v1) = 0;
    v2 = 0.0;
    v3 = (float)-(float)((float)(self->lookAtInfo.fLookAtAnimBlendRate * (float)v1) - self->lookAtInfo.fLookAtAnimYawLimit);
    if (v3 >= 0.0)
    {
        v2 = 180.0;
        if (v3 <= 180.0)
            v2 = (float)-(float)((float)(self->lookAtInfo.fLookAtAnimBlendRate * (float)v1)
                - self->lookAtInfo.fLookAtAnimYawLimit);
    }
    return *((float *)&v2 + 1);
}

float __cdecl Actor_CurrentLookAtYawMax(actor_s *self)
{
    __int64 v1; // r11
    double v2; // fp1
    double v3; // fp0

    HIDWORD(v1) = self->lookAtInfo.iLookAtBlendEndTime;
    LODWORD(v1) = HIDWORD(v1) - level.time;
    if (HIDWORD(v1) - level.time < 0)
        LODWORD(v1) = 0;
    v2 = 0.0;
    v3 = (float)-(float)((float)(self->lookAtInfo.fLookAtLimitBlendRate * (float)v1) - self->lookAtInfo.fLookAtYawLimit);
    if (v3 >= 0.0)
    {
        v2 = 180.0;
        if (v3 <= 180.0)
            v2 = (float)-(float)((float)(self->lookAtInfo.fLookAtLimitBlendRate * (float)v1) - self->lookAtInfo.fLookAtYawLimit);
    }
    return *((float *)&v2 + 1);
}

void __cdecl Actor_SetLookAtYawLimits(actor_s *self, double fAnimYawLimit, double fYawLimit, double fBlendTime)
{
    __int64 v4; // r11
    double v5; // fp0
    double v6; // fp13

    if (fAnimYawLimit != self->lookAtInfo.fLookAtAnimYawLimit || fYawLimit != self->lookAtInfo.fLookAtYawLimit)
    {
        HIDWORD(v4) = self->lookAtInfo.iLookAtBlendEndTime;
        LODWORD(v4) = HIDWORD(v4) - level.time;
        if (HIDWORD(v4) - level.time < 0)
            LODWORD(v4) = 0;
        v5 = (float)-(float)((float)((float)v4 * self->lookAtInfo.fLookAtAnimBlendRate)
            - self->lookAtInfo.fLookAtAnimYawLimit);
        if (v5 >= 0.0)
        {
            if (v5 > 180.0)
                v5 = 180.0;
        }
        else
        {
            v5 = 0.0;
        }
        self->lookAtInfo.fLookAtAnimYawLimit = fAnimYawLimit;
        LODWORD(v4) = HIDWORD(v4) - level.time;
        if (HIDWORD(v4) - level.time < 0)
            LODWORD(v4) = 0;
        v6 = (float)-(float)((float)((float)v4 * self->lookAtInfo.fLookAtLimitBlendRate) - self->lookAtInfo.fLookAtYawLimit);
        if (v6 >= 0.0)
        {
            if (v6 > 180.0)
                v6 = 180.0;
        }
        else
        {
            v6 = 0.0;
        }
        self->lookAtInfo.fLookAtYawLimit = fYawLimit;
        self->lookAtInfo.iLookAtBlendEndTime = level.time - (int)(float)((float)fBlendTime * (float)-1000.0);
        if (fBlendTime == 0.0)
        {
            self->lookAtInfo.fLookAtAnimBlendRate = 9999.0;
            self->lookAtInfo.fLookAtLimitBlendRate = 9999.0;
        }
        else
        {
            self->lookAtInfo.fLookAtAnimBlendRate = (float)((float)0.001 / (float)fBlendTime)
                * (float)((float)fAnimYawLimit - (float)v5);
            self->lookAtInfo.fLookAtLimitBlendRate = (float)((float)fYawLimit - (float)v6)
                * (float)((float)0.001 / (float)fBlendTime);
        }
    }
}

void __cdecl Actor_StopLookAt(actor_s *self, double fTurnAccel)
{
    self->lookAtInfo.bDoLookAt = 0;
    if (fTurnAccel > 20.0)
        self->lookAtInfo.fLookAtTurnAccel = fTurnAccel;
}

void __cdecl Actor_UpdateLookAt(actor_s *self)
{
    int v2; // r30
    double v3; // fp31
    double v4; // fp13
    double v5; // fp1
    double fLookAtTurnAngle; // fp9
    double v7; // fp13
    double fLookAtTurnSpeed; // fp0
    double v9; // fp0
    double v10; // fp0
    double v11; // fp0
    const XAnimTree_s *EntAnimTree; // r30
    double v13; // fp31
    DObj_s *ServerDObj; // r29
    int v15; // r7
    unsigned int v16; // r6
    unsigned int v17; // r5
    double v18; // fp1
    double v19; // fp30
    int v20; // r7
    unsigned int v21; // r6
    unsigned int v22; // r5
    double v23; // fp1
    unsigned int animLookAtRight; // r4
    int v25; // r7
    unsigned int v26; // r6
    unsigned int v27; // r5
    double Weight; // fp1
    double v29; // fp31
    int v30; // r7
    unsigned int v31; // r6
    unsigned int v32; // r5
    double v33; // fp1
    float v34[4]; // [sp+50h] [-70h] BYREF
    float v35[4]; // [sp+60h] [-60h] BYREF
    float v36[12]; // [sp+70h] [-50h] BYREF

    v2 = 0;
    if (self->lookAtInfo.bLookAtSetup
        && (self->lookAtInfo.bDoLookAt
            && self->lookAtInfo.fLookAtAnimYawLimit != 0.0
            && self->lookAtInfo.fLookAtYawLimit != 0.0
            || self->lookAtInfo.fLookAtTurnAngle != 0.0))
    {
        v3 = Actor_CurrentLookAtYawMax(self);
        Sentient_GetEyePosition(self->sentient, v34);
        v4 = (float)(self->lookAtInfo.vLookAtPos[1] - v34[1]);
        v35[0] = self->lookAtInfo.vLookAtPos[0] - v34[0];
        v35[1] = v4;
        v35[2] = self->lookAtInfo.vLookAtPos[2] - v34[2];
        vectoangles(v35, v36);
        if (!self->lookAtInfo.bDoLookAt
            || self->lookAtInfo.fLookAtAnimYawLimit == 0.0
            || self->lookAtInfo.fLookAtYawLimit == 0.0)
        {
            v5 = 0.0;
        }
        else
        {
            v5 = AngleSubtract(self->ent->r.currentAngles[1], v36[1]);
            if (v5 >= -v3)
            {
                if (v5 > v3)
                    v5 = v3;
            }
            else
            {
                v5 = -v3;
            }
        }
        fLookAtTurnAngle = self->lookAtInfo.fLookAtTurnAngle;
        v7 = (float)((float)v5 - self->lookAtInfo.fLookAtTurnAngle);
        //if (__fabs(v7) < 1.0)
        if (fabs(v7) < 1.0)
            goto LABEL_40;
        fLookAtTurnSpeed = self->lookAtInfo.fLookAtTurnSpeed;
        if (v5 >= 0.0)
        {
            if (v5 != 0.0)
                goto LABEL_19;
            if (v7 >= 0.0)
                goto LABEL_26;
        }
        fLookAtTurnSpeed = (float)(self->lookAtInfo.fLookAtTurnSpeed * (float)-1.0);
        v2 = 1;
        v5 = (float)((float)v5 * (float)-1.0);
        v7 = (float)((float)v7 * (float)-1.0);
    LABEL_19:
        if (v7 < 0.0)
        {
            if ((float)((float)fLookAtTurnSpeed * (float)-0.1) > v7)
            {
                v9 = (float)-(float)((float)(self->lookAtInfo.fLookAtTurnAccel * (float)0.050000001) - (float)fLookAtTurnSpeed);
            }
            else
            {
                v9 = (float)((float)fLookAtTurnSpeed * (float)1.5);
                if (v9 <= -10.0)
                {
                    if ((float)((float)v9 * (float)-0.1) > v7)
                        v9 = (float)((float)((float)v7 * (float)-10.0) - (float)0.1);
                }
                else
                {
                    v9 = -10.0;
                }
            }
            goto LABEL_32;
        }
    LABEL_26:
        if ((float)((float)fLookAtTurnSpeed * (float)0.1) < v7)
        {
            v9 = (float)((float)(self->lookAtInfo.fLookAtTurnAccel * (float)0.050000001) + (float)fLookAtTurnSpeed);
        }
        else
        {
            v9 = (float)-(float)((float)((float)fLookAtTurnSpeed * (float)0.5) - (float)fLookAtTurnSpeed);
            if (v9 >= 10.0)
            {
                if ((float)((float)v9 * (float)0.1) < v7)
                    v9 = (float)((float)((float)v7 * (float)10.0) + (float)0.1);
            }
            else
            {
                v9 = 10.0;
            }
        }
    LABEL_32:
        if (v9 > (float)(self->lookAtInfo.fLookAtTurnAccel * (float)0.22))
            v9 = (float)(self->lookAtInfo.fLookAtTurnAccel * (float)0.22);
        if (v2)
        {
            v9 = (float)((float)v9 * (float)-1.0);
            v5 = (float)((float)v5 * (float)-1.0);
        }
        self->lookAtInfo.fLookAtTurnSpeed = v9;
        if (fLookAtTurnAngle >= v5)
        {
            v11 = (float)((float)((float)v9 * (float)0.050000001) + self->lookAtInfo.fLookAtTurnAngle);
            self->lookAtInfo.fLookAtTurnAngle = v11;
            if (v11 > v5)
                goto LABEL_41;
        }
        else
        {
            v10 = (float)((float)((float)v9 * (float)0.050000001) + (float)fLookAtTurnAngle);
            self->lookAtInfo.fLookAtTurnAngle = v10;
            if (v10 < v5)
                goto LABEL_41;
        }
    LABEL_40:
        self->lookAtInfo.fLookAtTurnAngle = v5;
        self->lookAtInfo.fLookAtTurnSpeed = 0.0;
    LABEL_41:
        EntAnimTree = G_GetEntAnimTree(self->ent);
        if (!EntAnimTree)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_lookat.cpp", 317, 0, "%s", "pAnimTree");
        v13 = (float)((float)(self->lookAtInfo.fLookAtTurnAngle / Actor_CurrentLookAtAnimYawMax(self)) * (float)0.5);
        if (v13 <= 1.0)
        {
            if (v13 < -1.0)
                v13 = -1.0;
        }
        else
        {
            v13 = 1.0;
        }
        ServerDObj = Com_GetServerDObj(self->ent->s.number);
        if (v13 <= 0.0)
        {
            v13 = (float)((float)v13 * (float)-1.0);
            Weight = XAnimGetWeight(EntAnimTree, self->lookAtInfo.animLookAtLeft);
            v19 = 0.075000003;
            if (v13 != Weight)
                XAnimSetCompleteGoalWeight(
                    ServerDObj,
                    self->lookAtInfo.animLookAtLeft,
                    v13,
                    //(float)((float)0.075000003 / (float)__fabs((float)((float)v13 - (float)Weight))),
                    (float)((float)0.075f / (float)fabs((float)((float)v13 - (float)Weight))),
                    1.0,
                    v27,
                    v26,
                    v25);
            v23 = XAnimGetWeight(EntAnimTree, self->lookAtInfo.animLookAtRight);
            if (v23 == 0.0)
                goto LABEL_57;
            animLookAtRight = self->lookAtInfo.animLookAtRight;
        }
        else
        {
            v18 = XAnimGetWeight(EntAnimTree, self->lookAtInfo.animLookAtRight);
            v19 = 0.075000003;
            if (v13 != v18)
                XAnimSetCompleteGoalWeight(
                    ServerDObj,
                    self->lookAtInfo.animLookAtRight,
                    v13,
                    //(float)((float)0.075000003 / (float)__fabs((float)((float)v13 - (float)v18))),
                    (float)((float)0.075f / (float)fabs((float)((float)v13 - (float)v18))),
                    1.0,
                    v17,
                    v16,
                    v15);
            v23 = XAnimGetWeight(EntAnimTree, self->lookAtInfo.animLookAtLeft);
            if (v23 == 0.0)
                goto LABEL_57;
            animLookAtRight = self->lookAtInfo.animLookAtLeft;
        }
        XAnimSetCompleteGoalWeight(ServerDObj, animLookAtRight, 0.0, (float)((float)v19 / (float)v23), 1.0, v22, v21, v20);
    LABEL_57:
        v29 = (float)((float)1.0 - (float)v13);
        v33 = XAnimGetWeight(EntAnimTree, self->lookAtInfo.animLookAtStraight);
        if (v29 != v33)
            XAnimSetCompleteGoalWeight(
                ServerDObj,
                self->lookAtInfo.animLookAtStraight,
                v29,
                //(float)((float)v19 / (float)__fabs((float)((float)v29 - (float)v33))),
                (float)((float)v19 / (float)fabs((float)((float)v29 - (float)v33))),
                1.0,
                v32,
                v31,
                v30);
    }
}

