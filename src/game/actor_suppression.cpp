#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_suppression.h"
#include "game_public.h"

void __cdecl DebugDrawSuppression(actor_s *self, const float *a2, int a3, int a4, int a5)
{
    int *p_movementOnly; // r31
    int v7; // r29
    const float *v8; // r6

    // LWSS: this decomp sucks
    //p_movementOnly = &self->Suppressant[0].movementOnly;
    //v7 = 4;
    //do
    //{
    //    if (*(p_movementOnly - 5))
    //    {
    //        v8 = colorCyan;
    //        if (!*p_movementOnly)
    //            v8 = colorRed;
    //        G_DebugPlane(
    //            (const float *)p_movementOnly - 3,
    //            *((float *)p_movementOnly - 1),
    //            a2,
    //            self->ent->r.currentOrigin,
    //            100.0,
    //            (int)v8,
    //            a5);
    //    }
    //    --v7;
    //    p_movementOnly += 6;
    //} while (v7);

    for (int i = 0; i < 4; i++)
    {
        ai_suppression_t *pSupp = &self->Suppressant[i];
        if (pSupp->iTime)
        {
            const float *color = colorCyan;
            if (!pSupp->movementOnly)
            {
                color = colorRed;
            }

            G_DebugPlane(pSupp->clipPlane, pSupp->clipPlane[2], self->ent->r.currentOrigin, color, 100.0,  )
        }
    }
}

int __cdecl Actor_PickNewSuppressantEntry(actor_s *self, sentient_s *pSuppressor)
{
    int result; // r3
    int v4; // r10
    char *v5; // r9
    ai_suppression_t *Suppressant; // r11
    int v7; // r7

    v4 = 0;
    v5 = 0;
    Suppressant = self->Suppressant;
    v7 = -3468 - (unsigned int)self;
    result = 0;
    while (Suppressant->pSuppressor != pSuppressor)
    {
        if (Suppressant->iTime < *(int *)((char *)&self->Suppressant[0].iTime + (unsigned int)v5))
        {
            result = v4;
            v5 = (char *)Suppressant + v7;
        }
        ++v4;
        ++Suppressant;
        if (v4 >= 4)
            return result;
    }
    return v4;
}

int __cdecl Actor_NearCoverNode(actor_s *self)
{
    int result; // r3
    const pathnode_t *v3; // r3
    const pathnode_t *v4; // r31
    bool v5; // zf

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 60, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 61, 0, "%s", "self->sentient");
    if (Actor_NearClaimNode(self, 32.0))
        return 1;
    v3 = Sentient_NearestNode(self->sentient);
    v4 = v3;
    if (!v3)
        return 0;
    if (!Path_IsCoverNode(v3))
        return 0;
    v5 = Actor_PointNearPoint(self->ent->r.currentOrigin, v4->constant.vOrigin, 32.0);
    result = 1;
    if (!v5)
        return 0;
    return result;
}

void __cdecl Actor_BulletWhizbyNotify(actor_s *self, sentient_s *pSuppressor)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 85, 0, "%s", "self");
    if (!pSuppressor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 86, 0, "%s", "pSuppressor");
    Scr_AddEntity(pSuppressor->ent);
    Scr_Notify(self->ent, scr_const.bulletwhizby, 1u);
}

void __cdecl Actor_AddSuppressionLine(
    actor_s *self,
    sentient_s *pSuppressor,
    const float *vStart,
    const float *vEnd)
{
    double v8; // fp13
    double v9; // fp12
    double v10; // fp11
    char *v13; // r11
    double v14; // fp0
    double v15; // fp13
    double v16; // fp12
    float v17[20]; // [sp+50h] [-50h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 102, 0, "%s", "self");
    if (!pSuppressor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 103, 0, "%s", "pSuppressor");
    if (!vStart)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 104, 0, "%s", "vStart");
    if (!vEnd)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 105, 0, "%s", "vEnd");
    if (self->sentient->eTeam != pSuppressor->eTeam)
    {
        _FP12 = (float)((float)1.0 - (float)(self->suppressionMeter + (float)0.15000001));
        __asm { fsel      f0, f12, f0, f13 }
        self->suppressionMeter = _FP0;
        if (!(unsigned __int8)Actor_NearCoverNode(self))
            goto LABEL_29;
        goto LABEL_18;
    }
    if (pSuppressor->ent->client)
        goto LABEL_18;
    if (!self->ignoreSuppression)
    {
        if (!pSuppressor->ent->actor)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp",
                120,
                0,
                "%s",
                "pSuppressor->ent->actor");
        if (!(unsigned __int8)Actor_IsMoving(pSuppressor->ent->actor))
        {
            v8 = (float)(*vStart - self->ent->r.currentOrigin[0]);
            v9 = (float)(vStart[2] - self->ent->r.currentOrigin[2]);
            v10 = (float)(vStart[1] - self->ent->r.currentOrigin[1]);
            if ((float)((float)((float)v10 * (float)v10)
                + (float)((float)((float)v9 * (float)v9) + (float)((float)v8 * (float)v8))) <= (double)(float)(ai_friendlySuppressionDist->current.value * ai_friendlySuppressionDist->current.value))
            {
            LABEL_18:
                v13 = (char *)self + 24 * Actor_PickNewSuppressantEntry(self, pSuppressor);
                *((unsigned int *)v13 + 867) = level.time;
                *((unsigned int *)v13 + 868) = pSuppressor;
                v14 = (float)(vStart[1] - vEnd[1]);
                v15 = (float)(*vEnd - *vStart);
                v16 = (float)((float)(vStart[1] * (float)(*vEnd - *vStart)) + (float)(*vStart * (float)(vStart[1] - vEnd[1])));
                if ((float)((float)(self->ent->r.currentOrigin[1] * (float)(*vEnd - *vStart))
                    + (float)(self->ent->r.currentOrigin[0] * (float)(vStart[1] - vEnd[1]))) > v16)
                {
                    v14 = -v14;
                    v15 = -v15;
                    v16 = -v16;
                }
                *((float *)v13 + 869) = v14;
                *((float *)v13 + 870) = v15;
                *((float *)v13 + 871) = v16;
                if (self->sentient->eTeam == pSuppressor->eTeam)
                {
                    *((unsigned int *)v13 + 872) = 1;
                    if (self->eState[self->stateLevel] == AIS_EXPOSED && Actor_HasPath(self))
                    {
                        ProjectPointOntoVector(self->ent->r.currentOrigin, vStart, vEnd, v17);
                        if ((float)((float)(self->Path.lookaheadDir[1] * (float)(v17[1] - self->ent->r.currentOrigin[1]))
                            + (float)(self->Path.lookaheadDir[0] * (float)(v17[0] - self->ent->r.currentOrigin[0]))) > 0.0)
                            Actor_ClearPath(self);
                    }
                    return;
                }
                if (!self->ignoreSuppression)
                {
                    *((unsigned int *)v13 + 872) = 0;
                    if (!self->suppressionStartTime)
                        self->suppressionStartTime = level.time;
                    Scr_AddEntity(pSuppressor->ent);
                    Scr_Notify(self->ent, scr_const.suppression, 1u);
                    return;
                }
            LABEL_29:
                Actor_BulletWhizbyNotify(self, pSuppressor);
            }
        }
    }
}

void __cdecl Actor_ClearSuppressant(ai_suppression_t *suppressant)
{
    if (!suppressant)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 199, 0, "%s", "suppressant");
    suppressant->iTime = 0;
    suppressant->pSuppressor = 0;
    suppressant->movementOnly = 0;
}

void __cdecl Actor_DecaySuppressionLines(actor_s *self, const float *a2, int a3, int a4, int a5)
{
    bool v6; // r21
    char v7; // r24
    ai_suppression_t *Suppressant; // r31
    int v11; // r27
    int integer; // r11
    const dvar_s *v13; // r11
    int v14; // r9
    int number; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 215, 0, "%s", "self");
    v6 = self->suppressionStartTime > 0;
    v7 = 0;
    if (self->ignoreSuppression)
    {
        _FP0 = 0.0;
    }
    else
    {
        _FP12 = -(float)(self->suppressionMeter - (float)0.0099999998);
        __asm { fsel      f0, f12, f13, f0 }
    }
    self->suppressionMeter = _FP0;
    Suppressant = self->Suppressant;
    v11 = 4;
    do
    {
        if (Suppressant->iTime > 0)
        {
            if (Suppressant->movementOnly)
            {
                integer = ai_friendlyFireBlockDuration->current.integer;
            }
            else
            {
                integer = 0;
                if (!self->ignoreSuppression)
                    integer = self->suppressionDuration;
            }
            if (level.time - Suppressant->iTime < integer)
            {
                if (!Suppressant->movementOnly)
                    v7 = 1;
            }
            else
            {
                if (!Suppressant)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 199, 0, "%s", "suppressant");
                Suppressant->iTime = 0;
                Suppressant->pSuppressor = 0;
                Suppressant->movementOnly = 0;
            }
        }
        --v11;
        ++Suppressant;
    } while (v11);
    if (v6 && !v7)
        Scr_Notify(self->ent, scr_const.suppression_end, 0);
    if (!v7)
        self->suppressionStartTime = 0;
    v13 = ai_debugEntIndex;
    if (ai_debugEntIndex->current.integer == self->ent->s.number && ai_showSuppression->current.integer > 0)
    {
        DebugDrawSuppression(self, a2, a3, a4, a5);
        v13 = ai_debugEntIndex;
    }
    v14 = v13->current.integer;
    number = self->ent->s.number;
    if (v14 != number && ai_showSuppression->current.integer == number)
        DebugDrawSuppression(self, a2, a3, a4, a5);
}

void __cdecl Actor_DissociateSuppressor(actor_s *self, sentient_s *pSuppressor)
{
    bool v4; // r22
    char v5; // r26
    ai_suppression_t *Suppressant; // r31
    int v7; // r29
    int *p_movementOnly; // r30

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 276, 0, "%s", "self");
    if (!pSuppressor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 277, 0, "%s", "pSuppressor");
    v4 = self->suppressionStartTime > 0;
    v5 = 0;
    Suppressant = self->Suppressant;
    v7 = 4;
    do
    {
        p_movementOnly = &Suppressant->movementOnly;
        if (Suppressant->pSuppressor == pSuppressor)
        {
            if (!Suppressant)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 199, 0, "%s", "suppressant");
            Suppressant->iTime = 0;
            Suppressant->pSuppressor = 0;
            *p_movementOnly = 0;
        }
        else if (Suppressant->iTime > 0 && !*p_movementOnly)
        {
            v5 = 1;
        }
        --v7;
        ++Suppressant;
    } while (v7);
    if (v4 && !v5)
        Scr_Notify(self->ent, scr_const.suppression_end, 0);
    if (!v5)
        self->suppressionStartTime = 0;
}

int __cdecl Actor_IsSuppressedInAnyway(actor_s *self)
{
    int v3; // r11
    ai_suppression_t *i; // r10

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 333, 0, "%s", "self");
    if (self->suppressionStartTime <= 0)
    {
        v3 = 0;
        for (i = self->Suppressant; !i->iTime; ++i)
        {
            if (++v3 >= 4)
                return 0;
        }
    }
    return 1;
}

bool __cdecl Actor_IsSuppressed(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 359, 0, "%s", "self");
    return self->suppressionStartTime > 0;
}

int __cdecl Actor_IsMoveSuppressed(actor_s *self)
{
    int v2; // r10
    int *i; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 374, 0, "%s", "self");
    v2 = 0;
    for (i = &self->Suppressant[0].movementOnly; !*(i - 5) || !*i; i += 6)
    {
        if (++v2 >= 4)
            return 0;
    }
    return 1;
}

int __cdecl Actor_IsSuppressionWaiting(actor_s *self)
{
    int v2; // r9
    ai_suppression_t *i; // r10

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 397, 0, "%s", "self");
    if (self->ignoreSuppression)
        return 0;
    v2 = 0;
    for (i = self->Suppressant; !i->iTime || i->movementOnly || level.time - i->iTime >= self->suppressionWait; ++i)
    {
        if (++v2 >= 4)
            return 0;
    }
    return 1;
}

int __cdecl Actor_GetSuppressionPlanes(actor_s *self, float (*vNormalOut)[2], float *fDistOut)
{
    int result; // r3
    float *v7; // r11
    float *v8; // r11
    float *v9; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 428, 0, "%s", "self");
    result = 0;
    if (self->Suppressant[0].iTime)
    {
        result = 1;
        (*vNormalOut)[0] = self->Suppressant[0].clipPlane[0];
        (*vNormalOut)[1] = self->Suppressant[0].clipPlane[1];
        *fDistOut = self->Suppressant[0].clipPlane[2];
    }
    if (self->Suppressant[1].iTime)
    {
        v7 = &(*vNormalOut)[2 * result];
        *v7 = self->Suppressant[1].clipPlane[0];
        v7[1] = self->Suppressant[1].clipPlane[1];
        fDistOut[result++] = self->Suppressant[1].clipPlane[2];
    }
    if (self->Suppressant[2].iTime)
    {
        v8 = &(*vNormalOut)[2 * result];
        *v8 = self->Suppressant[2].clipPlane[0];
        v8[1] = self->Suppressant[2].clipPlane[1];
        fDistOut[result++] = self->Suppressant[2].clipPlane[2];
    }
    if (self->Suppressant[3].iTime)
    {
        v9 = &(*vNormalOut)[2 * result];
        *v9 = self->Suppressant[3].clipPlane[0];
        v9[1] = self->Suppressant[3].clipPlane[1];
        fDistOut[result++] = self->Suppressant[3].clipPlane[2];
    }
    return result;
}

int __cdecl Actor_GetMoveOnlySuppressionPlanes(actor_s *self, float (*vNormalOut)[2], float *fDistOut)
{
    int result; // r3
    float *v7; // r11
    float *v8; // r11
    float *v9; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 457, 0, "%s", "self");
    result = 0;
    if (self->Suppressant[0].iTime && self->Suppressant[0].movementOnly)
    {
        result = 1;
        (*vNormalOut)[0] = self->Suppressant[0].clipPlane[0];
        (*vNormalOut)[1] = self->Suppressant[0].clipPlane[1];
        *fDistOut = self->Suppressant[0].clipPlane[2];
    }
    if (self->Suppressant[1].iTime && self->Suppressant[1].movementOnly)
    {
        v7 = &(*vNormalOut)[2 * result];
        *v7 = self->Suppressant[1].clipPlane[0];
        v7[1] = self->Suppressant[1].clipPlane[1];
        fDistOut[result++] = self->Suppressant[1].clipPlane[2];
    }
    if (self->Suppressant[2].iTime && self->Suppressant[2].movementOnly)
    {
        v8 = &(*vNormalOut)[2 * result];
        *v8 = self->Suppressant[2].clipPlane[0];
        v8[1] = self->Suppressant[2].clipPlane[1];
        fDistOut[result++] = self->Suppressant[2].clipPlane[2];
    }
    if (self->Suppressant[3].iTime)
    {
        if (self->Suppressant[3].movementOnly)
        {
            v9 = &(*vNormalOut)[2 * result];
            *v9 = self->Suppressant[3].clipPlane[0];
            v9[1] = self->Suppressant[3].clipPlane[1];
            fDistOut[result++] = self->Suppressant[3].clipPlane[2];
        }
    }
    return result;
}

void __cdecl Actor_ClearAllSuppressionFromEnemySentient(sentient_s *pSuppressor)
{
    int v2; // r26
    actor_s *i; // r31

    if (!pSuppressor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_suppression.cpp", 311, 0, "%s", "pSuppressor");
    v2 = 1 << Sentient_EnemyTeam(pSuppressor->eTeam);
    for (i = Actor_FirstActor(v2); i; i = Actor_NextActor(i, v2))
    {
        if (i->suppressionStartTime > 0)
            Actor_DissociateSuppressor(i, pSuppressor);
    }
}

