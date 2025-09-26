#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_navigation.h"
#include <universal/com_math.h>
#include "g_main.h"
#include "game_public.h"
#include "actor_senses.h"
#include <server/sv_game.h>
#include <universal/profile.h>

struct CustomSearchInfo_FindCloseNode
{
    float goalPos[3];
    pathnode_t *closestNode;
    float closestDistSq;

    float EvaluateHeuristic(pathnode_t *pSuccessor, const float *vGoalPos)
    {
        return Vec2Distance(pSuccessor->constant.vOrigin, vGoalPos);
    }

    bool IgnoreNode(pathnode_t *pNode)
    {
        float v3; // [esp+Ch] [ebp-Ch]
        float heightDist; // [esp+10h] [ebp-8h]
        float distSq; // [esp+14h] [ebp-4h]

        v3 = this->goalPos[1] - pNode->constant.vOrigin[1];
        distSq = (float)((float)(this->goalPos[0] - pNode->constant.vOrigin[0])
            * (float)(this->goalPos[0] - pNode->constant.vOrigin[0]))
            + (float)(v3 * v3);
        if (distSq >= this->closestDistSq)
            return 1;
        heightDist = pNode->constant.vOrigin[2] - this->goalPos[2];
        if ((float)(heightDist * heightDist) > 6400.0)
            return 1;
        this->closestDistSq = distSq;
        this->closestNode = pNode;
        return 0;
    }
};

struct CustomSearchInfo_FindPath
{
    pathnode_t *m_pNodeTo;
    float startPos[3];
    float negotiationOverlapCost;

    float EvaluateHeuristic(pathnode_t *pSuccessor, const float *vGoalPos)
    {
        float v[2]; // [esp+18h] [ebp-Ch] BYREF
        float dist; // [esp+20h] [ebp-4h]

        v[0] = vGoalPos[0] - pSuccessor->constant.vOrigin[0];
        v[1] = vGoalPos[1] - pSuccessor->constant.vOrigin[1];
        dist = Vec2Length(v);
        dist = (float)((float)pSuccessor->dynamic.userCount * this->negotiationOverlapCost) + dist;
        if (pSuccessor->constant.minUseDistSq > 1.0 && pSuccessor->constant.minUseDistSq > Vec3DistanceSq(pSuccessor->constant.vOrigin, this->startPos))
        {
            return (float)(dist + this->negotiationOverlapCost);
        }
        return dist;
    }
};
/* 10044 */
struct CustomSearchInfo_FindPathWithWidth
{
    pathnode_t *m_pNodeTo;
    float width;
    float perp[2];

    float EvaluateHeuristic(pathnode_t *pSuccessor, const float *vGoalPos)
    {
        double v5; // fp31
        double v6; // fp30
        double v8; // fp1

        // KISAKTODO: better logic
        float a4 = (float)((float)fabsf((float)((float)((float)(this->perp[1]
            * (float)(pSuccessor->constant.vOrigin[1] - vGoalPos[1]))
            + (float)((float)(pSuccessor->constant.vOrigin[0] - *vGoalPos)
                * this->perp[0]))
            - this->width))
            * (float)0.0069077001);

        v5 = (float)(pSuccessor->constant.vOrigin[0] - vGoalPos[0]);
        v6 = (float)(pSuccessor->constant.vOrigin[1] - vGoalPos[1]);
        float v7 = exp(a4);

        pSuccessor->transient.costFactor = v7;

        v8 = (float)((float)sqrtf((float)((float)((float)v6 * (float)v6) + (float)((float)v5 * (float)v5))) * (float)*(double *)&v7);
        return v8;

        //double v4; // xmm0_8
        //long double v6; // [esp-18h] [ebp-3Ch]
        //float v7; // [esp+8h] [ebp-1Ch] BYREF
        //float dist; // [esp+Ch] [ebp-18h]
        //float *vOrigin; // [esp+10h] [ebp-14h]
        //float delta[3]; // [esp+14h] [ebp-10h]
        //float retaddr; // [esp+24h] [ebp+0h]
        //
        //delta[1] = a2;
        //delta[2] = retaddr;
        //LODWORD(delta[0]) = this;
        //vOrigin = pSuccessor->constant.vOrigin;
        //v7 = pSuccessor->constant.vOrigin[0] - vGoalPos[0];
        //dist = pSuccessor->constant.vOrigin[1] - vGoalPos[1];
        //v4 = (float)(COERCE_FLOAT(
        //    COERCE_UNSIGNED_INT((float)((float)(this->perp[0] * v7) 
        //        + (float)(this->perp[1] * dist)) - this->width)
        //    & _mask__AbsFloat_)
        //    * 0.0069077001);
        ////__libm_sse2_exp(v6);
        //exp(v6);
        ////*(float *)&v4 = v4;
        //pSuccessor->transient.costFactor = *(float *)&v4;
        //return Vec2Length(&v7) * *(float *)&v4;
    }
};

/* 10045 */
struct  CustomSearchInfo_FindPathNotCrossPlanes : CustomSearchInfo_FindPath
{
    int m_iPlaneCount;
    float (*m_vNormal)[2];
    float *m_fDist;

    // Inherits EvaluateHeuristic()

    bool IgnoreNode(pathnode_t *pNode)
    {
        int i; // [esp+Ch] [ebp-4h]

        for (i = 0; i < this->m_iPlaneCount; ++i)
        {
            if ((float)((float)(pNode->constant.vOrigin[0] * this->m_vNormal[i][0]) + (float)(pNode->constant.vOrigin[1] * this->m_vNormal[i][1])) > this->m_fDist[i])
                return 1;
        }
        return 0;
    }
};

/* 10046 */
struct  CustomSearchInfo_FindPathAway
{
    float m_vAwayFromPos[3];
    float m_fDistAway;
    float m_fDistAwaySqrd;
    float m_fInitialDistAwaySq;
    float m_fBestScore;
    pathnode_t *m_pBestNode;

    float EvaluateHeuristic(pathnode_t *pSuccessor, const float *vGoalPos)
    {
        float diff[3];
        Vec3Sub(pSuccessor->constant.vOrigin, this->m_vAwayFromPos, diff);
        return this->m_fDistAway - (Vec3Length(diff));
    }
};

/* 10047 */
struct  CustomSearchInfo_FindPathAwayNotCrossPlanes : CustomSearchInfo_FindPathAway
{
    int m_iPlaneCount;
    float (*m_vNormal)[2];
    float *m_fDist;

    bool IgnoreNode(pathnode_t *pNode)
    {
        int planeCount; // r8
        int v3; // r9
        float *fDist; // r10
        float *i; // r11

        planeCount = this->m_iPlaneCount;
        v3 = 0;
        if (planeCount <= 0)
            return 0;
        fDist = this->m_fDist;
        for (i = (float *)this->m_vNormal;
            (float)((float)(i[1] * pNode->constant.vOrigin[1]) + (float)(pNode->constant.vOrigin[0] * *i)) <= (double)*fDist;
            i += 2)
        {
            ++v3;
            ++fDist;
            if (v3 >= planeCount)
                return 0;
        }
        return 1;
    }
};

/* 10048 */
struct CustomSearchInfo_FindPathWithLOS
{
    pathnode_t *m_pNodeTo;
    float m_fWithinDistSqrd;
    float startPos[3];
    float negotiationOverlapCost;

    float EvaluateHeuristic(pathnode_t *pSuccessor, const float *vGoalPos)
    {
        float v[2]; // [esp+18h] [ebp-Ch] BYREF
        float dist; // [esp+20h] [ebp-4h]

        v[0] = vGoalPos[0] - pSuccessor->constant.vOrigin[0];
        v[1] = vGoalPos[1] - pSuccessor->constant.vOrigin[1];
        dist = Vec2Length(v);
        dist = (float)((float)pSuccessor->dynamic.userCount * this->negotiationOverlapCost) + dist;
        if (pSuccessor->constant.minUseDistSq > 1.0
            && pSuccessor->constant.minUseDistSq > Vec3DistanceSq(pSuccessor->constant.vOrigin, this->startPos))
        {
            return (float)(dist + this->negotiationOverlapCost);
        }
        return dist;
    }
};

/* 10049 */
struct  CustomSearchInfo_FindPathInCylinderWithLOS : CustomSearchInfo_FindPathWithLOS
{
    const actor_goal_s *goal;

    float EvaluateHeuristic(pathnode_t *pSuccessor, const float *vGoalPos)
    {
        float v[2]; // [esp+18h] [ebp-Ch] BYREF
        float dist; // [esp+20h] [ebp-4h]

        v[0] = vGoalPos[0] - pSuccessor->constant.vOrigin[0];
        v[1] = vGoalPos[1] - pSuccessor->constant.vOrigin[1];
        dist = Vec2Length(v);
        dist = (float)((float)pSuccessor->dynamic.userCount * this->negotiationOverlapCost) + dist;
        if (pSuccessor->constant.minUseDistSq > 1.0 && pSuccessor->constant.minUseDistSq > Vec3DistanceSq(pSuccessor->constant.vOrigin, this->startPos))
        {
            return (float)(dist + this->negotiationOverlapCost);
        }
        return dist;
    }
};

/* 10050 */
struct  CustomSearchInfo_FindPathInCylinderWithLOSNotCrossPlanes : CustomSearchInfo_FindPathWithLOS
{
    const actor_goal_s *goal;
    int m_iPlaneCount;
    float (*m_vNormal)[2];
    float *m_fDist;

    // inherits EvaluateHeuristic()

    float EvaluateHeuristic(pathnode_t *pSuccessor, const float *vGoalPos)
    {
        float v[2]; // [esp+18h] [ebp-Ch] BYREF
        float dist; // [esp+20h] [ebp-4h]

        v[0] = vGoalPos[0] - pSuccessor->constant.vOrigin[0];
        v[1] = vGoalPos[1] - pSuccessor->constant.vOrigin[1];
        dist = Vec2Length(v);
        dist = (float)((float)pSuccessor->dynamic.userCount * this->negotiationOverlapCost) + dist;
        if (pSuccessor->constant.minUseDistSq > 1.0 && pSuccessor->constant.minUseDistSq > Vec3DistanceSq(pSuccessor->constant.vOrigin, this->startPos))
        {
            return (float)(dist + this->negotiationOverlapCost);
        }
        return dist;
    }

    bool IgnoreNode(pathnode_t *pNode)
    {
        float *vOrigin; // r30
        int planeCount; // r8
        int v6; // r9
        float *fDist; // r10
        float *i; // r11

        vOrigin = pNode->constant.vOrigin;
        if (!Actor_PointAtGoal(pNode->constant.vOrigin, this->goal))
            return 1;
        planeCount = this->m_iPlaneCount;
        v6 = 0;
        if (planeCount > 0)
        {
            fDist = this->m_fDist;
            for (i = (float *)this->m_vNormal;
                (float)((float)(i[1] * vOrigin[1]) + (float)(*vOrigin * *i)) <= (double)*fDist;
                i += 2)
            {
                ++v6;
                ++fDist;
                if (v6 >= planeCount)
                    return 0;
            }
            return 1;
        }
        return 0;
    }
};

/* 10051 */
struct  CustomSearchInfo_FindPathFromInCylinder : CustomSearchInfo_FindPath
{
    float m_vOrigin[2];
    float m_fRadiusSqrd;
    float m_fHalfHeightSqrd;

    // inherits EvaluateHeuristic()
};

/* 10052 */
struct  CustomSearchInfo_FindPathFromInCylinderNotCrossPlanes : CustomSearchInfo_FindPath
{
    float m_vOrigin[2];
    float m_fRadiusSqrd;
    float m_fHalfHeightSqrd;
    int m_iPlaneCount;
    float (*m_vNormal)[2];
    float *m_fDist;

    // inherits EvaluateHeuristic()

    bool IgnoreNode(pathnode_t *pNode)
    {
        double v2; // fp0
        double v4; // fp0
        double v5; // fp13
        int planeCount; // r8
        int v7; // r9
        float *fDist; // r10
        float *i; // r11

        v2 = (float)(pNode->constant.vOrigin[2] - this->m_fRadiusSqrd);
        if ((float)((float)v2 * (float)v2) > (double)this->m_fHalfHeightSqrd)
            return 1;
        v4 = (float)(pNode->constant.vOrigin[0] - this->m_vOrigin[0]);
        v5 = (float)(pNode->constant.vOrigin[1] - this->m_vOrigin[1]);
        if ((float)((float)((float)v5 * (float)v5) + (float)((float)v4 * (float)v4)) > (double)this->m_fRadiusSqrd)
            return 1;
        planeCount = this->m_iPlaneCount;
        v7 = 0;
        if (planeCount > 0)
        {
            fDist = this->m_fDist;
            for (i = (float *)this->m_vNormal;
                (float)((float)(i[1] * pNode->constant.vOrigin[1]) + (float)(pNode->constant.vOrigin[0] * *i)) <= (double)*fDist;
                i += 2)
            {
                ++v7;
                ++fDist;
                if (v7 >= planeCount)
                    return 0;
            }
            return 1;
        }
        return 0;
    }
};

/* 10053 */
struct CustomSearchInfo_CouldAttack
{
    pathnode_t *m_pNodeTo;
    pathnode_t *attackNode;

    float EvaluateHeuristic(pathnode_t *pSuccessor, const float *vGoalPos)
    {
        return 0.0f;
    }
};

/* 10054 */
struct  CustomSearchInfo_FindPathClosestPossible
{
    float m_fBestScore;
    pathnode_t *m_pBestNode;
    pathnode_t *m_pNodeTo;
    float negotiationOverlapCost;

    float EvaluateHeuristic(pathnode_t *pSuccessor, const float *vGoalPos)
    {
        float v[2]; // [esp+Ch] [ebp-Ch] BYREF
        float dist; // [esp+14h] [ebp-4h]

        v[0] = vGoalPos[0] - pSuccessor->constant.vOrigin[0];
        v[1] = vGoalPos[1] - pSuccessor->constant.vOrigin[1];
        dist = Vec2Length(v);
        return (float)((float)((float)pSuccessor->dynamic.userCount * this->negotiationOverlapCost) + dist);
    }
};

path_t pathBackup;

bool __cdecl Path_IsPathStanceNode(const pathnode_t *node)
{
    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.h", 166, 0, "%s", "node");
    return ((1 << node->constant.type) & 0xC1FFC) == 0;
}

float __cdecl Path_GetPathDir(float *delta, const float *vFrom, const float *vTo)
{
    double v4; // fp31
    double v5; // fp12
    double v6; // fp1
    float v8; // [sp+50h] [-40h]

    v8 = *vTo - *vFrom;
    *delta = v8;
    delta[1] = vTo[1] - vFrom[1];
    if ((LODWORD(v8) & 0x7F800000) == 0x7F800000)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 56, 0, "%s", "!IS_NAN(delta[0])");
    if ((COERCE_UNSIGNED_INT(delta[1]) & 0x7F800000) == 0x7F800000)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 57, 0, "%s", "!IS_NAN(delta[1])");
    if (*delta == 0.0 && delta[1] == 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 58, 0, "%s", "delta[0] || delta[1]");
    v4 = sqrtf((float)((float)(delta[1] * delta[1]) + (float)(*delta * *delta)));
    if (v4 <= 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 60, 0, "%s", "fDist > 0");
    v5 = delta[1];
    v6 = v4;
    *delta = *delta * (float)((float)1.0 / (float)v4);
    delta[1] = (float)v5 * (float)((float)1.0 / (float)v4);
    return *((float *)&v6 + 1);
}

pathnode_t *__cdecl Path_GetNegotiationNode(const path_t *pPath)
{
    if (pPath->wNegotiationStartNode <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            74,
            0,
            "%s",
            "pPath->wNegotiationStartNode > 0");
    return Path_ConvertIndexToNode(pPath->pts[pPath->wNegotiationStartNode].iNodeNum);
}

void __cdecl Path_IncrementNodeUserCount(path_t *pPath)
{
    pathnode_t *NegotiationNode; // r31

    NegotiationNode = Path_GetNegotiationNode(pPath);
    if (!NegotiationNode)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 92, 0, "%s", "negotiationNode");
    ++NegotiationNode->dynamic.userCount;
}

void __cdecl Path_DecrementNodeUserCount(path_t *pPath)
{
    pathnode_t *NegotiationNode; // r29

    NegotiationNode = Path_GetNegotiationNode(pPath);
    if (!NegotiationNode)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 109, 0, "%s", "negotiationNode");
    if (NegotiationNode->dynamic.userCount <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            110,
            0,
            "%s",
            "negotiationNode->dynamic.userCount > 0");
    --NegotiationNode->dynamic.userCount;
}

void __cdecl Path_Backup(const path_t *path)
{
    if (!path)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 123, 0, "%s", "path");
    Com_Memcpy(&pathBackup, path, 996);
}

float __cdecl Path_GetDistToPathSegment(const float *vStartPos, const pathpoint_t *pt)
{
    double v4; // fp31
    double v5; // fp30
    double v6; // fp1

    if (!pt)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 156, 0, "%s", "pt");
    v4 = (float)(pt->vOrigPoint[0] - *vStartPos);
    v5 = (float)(pt->vOrigPoint[1] - vStartPos[1]);
    if (pt->fDir2D[0] == 0.0 && pt->fDir2D[1] == 0.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            159,
            0,
            "%s",
            "pt->fDir2D[0] || pt->fDir2D[1]");
    v6 = I_fabs((float)((float)(pt->fDir2D[0] * (float)v5) - (float)(pt->fDir2D[1] * (float)v4)));
    return *((float *)&v6 + 1);
}

void __cdecl Path_AddTrimmedAmount(path_t *pPath, const float *vStartPos)
{
    int v4; // r27
    float *vOrigPoint; // r31
    double v6; // fp27
    double v7; // fp29
    double v8; // fp26
    float *v9; // r29
    double v10; // fp31
    double v11; // fp30
    double v12; // fp0
    double v13; // fp31
    pathpoint_t *v14; // r31
    double DistToPathSegment; // fp31
    float fOrigLength; // fp1
    float fCurrLength; // fp2
    double v18; // fp31

    v4 = pPath->wPathLen - 1;
    if ((unsigned __int16)pPath->wNegotiationStartNode >= 0x8000u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            186,
            0,
            "%s",
            "pPath->wNegotiationStartNode >= 0");
    if (v4 != -1 && pPath->wNegotiationStartNode > v4)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            187,
            0,
            "%s",
            "i == -1 || pPath->wNegotiationStartNode <= i");
    if (v4 > pPath->wNegotiationStartNode)
    {
        vOrigPoint = pPath->pts[v4].vOrigPoint;
        v6 = Vec2Distance(vOrigPoint - 7, vStartPos);
        v7 = 0.0;
        v8 = 0.0;
        if (v4 >= pPath->wOrigPathLen)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                197,
                0,
                "%s",
                "i < pPath->wOrigPathLen");
        do
        {
            v9 = vOrigPoint;
            v10 = (float)(*vOrigPoint - *vStartPos);
            v11 = (float)(vOrigPoint[1] - vStartPos[1]);
            v12 = sqrtf((float)((float)((float)(*vOrigPoint - *vStartPos) * (float)(*vOrigPoint - *vStartPos))
                + (float)((float)v11 * (float)v11)));
            if (v12 <= v6)
            {
                v6 = v12;
                v8 = v7;
            }
            ++v4;
            vOrigPoint += 7;
            if (v4 >= pPath->wOrigPathLen)
                break;
            if (v9[3] == 0.0 && v9[4] == 0.0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    214,
                    0,
                    "%s",
                    "pt->fDir2D[0] || pt->fDir2D[1]");
            v13 = I_fabs((float)((float)((float)v11 * v9[3]) - (float)(v9[4] * (float)v10)));
            if (v9[5] <= 0.0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    216,
                    0,
                    "%s",
                    "pt->fOrigLength > 0");
            v7 = (float)((float)((float)v13 * v9[5]) + (float)v7);
        } while (v7 <= 32768.0);
        v14 = &pPath->pts[pPath->wPathLen - 2];
        DistToPathSegment = Path_GetDistToPathSegment(vStartPos, v14);
        if (v14->fOrigLength <= 0.0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 223, 0, "%s", "pt->fOrigLength > 0");
        if (pPath->fCurrLength <= 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                224,
                0,
                "%s",
                "pPath->fCurrLength > 0");
        fOrigLength = v14->fOrigLength;
        fCurrLength = pPath->fCurrLength;
        if (fOrigLength < fCurrLength)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                225,
                0,
                (const char *)HIDWORD(fOrigLength),
                LODWORD(fOrigLength),
                LODWORD(fCurrLength));
        v18 = (float)((float)((float)(v14->fOrigLength - pPath->fCurrLength) * (float)DistToPathSegment) + (float)v8);
        if (v18 < 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                228,
                0,
                "%s\n\t(closestAmount) = %g",
                HIDWORD(v18),
                LODWORD(v18));
        pPath->fLookaheadAmount = (float)v18 + pPath->fLookaheadAmount;
    }
}

void __cdecl Path_SubtractTrimmedAmount(path_t *pPath, const float *vStartPos)
{
    int wPathLen; // r11
    pathpoint_t *v5; // r30
    double DistToPathSegment; // fp31
    float fOrigLength; // fp1
    float fCurrLength; // fp2
    int v9; // r29
    double v10; // fp31
    float *p_fOrigLength; // r30
    double v12; // fp30
    double v13; // fp0

    wPathLen = pPath->wPathLen;
    if (wPathLen != pPath->wOrigPathLen)
    {
        if (wPathLen <= 1)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 253, 0, "%s", "pPath->wPathLen > 1");
        v5 = &pPath->pts[pPath->wPathLen - 2];
        DistToPathSegment = Path_GetDistToPathSegment(vStartPos, v5);
        if (v5->fOrigLength <= 0.0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 257, 0, "%s", "pt->fOrigLength > 0");
        if (pPath->fCurrLength <= 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                258,
                0,
                "%s",
                "pPath->fCurrLength > 0");
        fOrigLength = v5->fOrigLength;
        fCurrLength = pPath->fCurrLength;
        if (fOrigLength < fCurrLength)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                259,
                0,
                (const char *)HIDWORD(fOrigLength),
                LODWORD(fOrigLength),
                LODWORD(fCurrLength));
        v9 = pPath->wPathLen - 1;
        v10 = (float)((float)(v5->fOrigLength - pPath->fCurrLength) * (float)DistToPathSegment);
        if (v9 < pPath->wOrigPathLen - 1)
        {
            p_fOrigLength = &pPath->pts[v9].fOrigLength;
            do
            {
                v12 = Path_GetDistToPathSegment(vStartPos, (const pathpoint_t *)(p_fOrigLength - 5));
                if (*p_fOrigLength <= 0.0)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                        267,
                        0,
                        "%s",
                        "pt->fOrigLength > 0");
                ++v9;
                v10 = (float)((float)(*p_fOrigLength * (float)v12) + (float)v10);
                p_fOrigLength += 7;
            } while (v9 < pPath->wOrigPathLen - 1);
        }
        v13 = (float)(pPath->fLookaheadAmount - (float)v10);
        pPath->fLookaheadAmount = pPath->fLookaheadAmount - (float)v10;
        if (v13 < 64.0)
            pPath->fLookaheadAmount = 64.0;
    }
}

void __cdecl Path_BeginTrim(path_t *pPath, path_trim_t *pTrim)
{
    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2018, 0, "%s", "pPath");
    if (!pTrim)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2019, 0, "%s", "pTrim");
    if (pPath->wPathLen)
    {
        pTrim->iDelta = -2;
        pTrim->iIndex = pPath->wPathLen + 1;
    }
}

void __cdecl Path_Begin(path_t *pPath)
{
    memset(pPath, 0, sizeof(path_t));
    pPath->fLookaheadAmount = 32768.0;
    pPath->wDodgeEntity = ENTITYNUM_NONE;
}

void __cdecl Path_Clear(path_t *pPath)
{
    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2142, 0, "%s", "pPath");
    if (pPath->wNegotiationStartNode > 0)
    {
        Path_DecrementNodeUserCount(pPath);
        pPath->wNegotiationStartNode = 0;
    }
    pPath->wPathLen = 0;
    pPath->wOrigPathLen = 0;
}

bool __cdecl Path_Exists(const path_t *pPath)
{
    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2164, 0, "%s", "pPath");
    return pPath->wPathLen > 0;
}

int __cdecl Path_CompleteLookahead(const path_t *pPath)
{
    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2178, 0, "%s", "pPath");
    if (pPath->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2179, 0, "%s", "pPath->wPathLen > 0");
    return pPath->flags & 1;
}

unsigned int __cdecl Path_AttemptedCompleteLookahead(const path_t *pPath)
{
    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2194, 0, "%s", "pPath");
    if (pPath->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2195, 0, "%s", "pPath->wPathLen > 0");
    return ((unsigned int)pPath->flags >> 6) & 1;
}

bool __cdecl Path_UsesObstacleNegotiation(const path_t *pPath)
{
    int wPathLen; // r11
    float fCurrLength; // fp1
    double v4; // fp2
    const char *v5; // r3

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2210, 0, "%s", "pPath");
    if (pPath->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2211, 0, "%s", "pPath->wPathLen > 0");
    if ((unsigned __int16)pPath->wNegotiationStartNode >= 0x8000u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2212,
            0,
            "%s",
            "pPath->wNegotiationStartNode >= 0");
    if (pPath->wNegotiationStartNode > pPath->lookaheadNextNode)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2213,
            0,
            "%s",
            "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode");
    if (pPath->lookaheadNextNode >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2214,
            0,
            "%s",
            "pPath->lookaheadNextNode < pPath->wPathLen");
    wPathLen = pPath->wPathLen;
    if (wPathLen > 1)
    {
        fCurrLength = pPath->fCurrLength;
        v4 = *((float *)&pPath->pts[wPathLen - 1] - 2);
        if (fCurrLength > v4)
        {
            v5 = va((const char *)HIDWORD(fCurrLength), LODWORD(fCurrLength), LODWORD(v4));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2215,
                0,
                "%s\n\t%s",
                "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                v5);
        }
    }
    return pPath->wNegotiationStartNode > 0;
}

bool __cdecl Path_HasNegotiationNode(const path_t *path)
{
    if (!path)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2164, 0, "%s", "pPath");
    return path->wPathLen > 0 && path->wNegotiationStartNode > 0;
}

unsigned int __cdecl Path_AllowsObstacleNegotiation(const path_t *pPath)
{
    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2246, 0, "%s", "pPath");
    if (pPath->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2247, 0, "%s", "pPath->wPathLen > 0");
    return ((unsigned int)pPath->flags >> 4) & 1;
}

void __cdecl Path_GetObstacleNegotiationScript(const path_t *pPath, scr_animscript_t *animscript)
{
    int wPathLen; // r11
    float fCurrLength; // fp1
    double v6; // fp2
    const char *v7; // r3
    pathnode_t *v8; // r3

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2264, 0, "%s", "pPath");
    if (pPath->wNegotiationStartNode <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2265,
            0,
            "%s",
            "pPath->wNegotiationStartNode > 0");
    if (pPath->wNegotiationStartNode >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2266,
            0,
            "%s",
            "pPath->wNegotiationStartNode < pPath->wPathLen");
    if (pPath->pts[pPath->wNegotiationStartNode].iNodeNum < 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2267,
            0,
            "%s",
            "pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0");
    wPathLen = pPath->wPathLen;
    if (wPathLen > 1)
    {
        fCurrLength = pPath->fCurrLength;
        v6 = *((float *)&pPath->pts[wPathLen - 1] - 2);
        if (fCurrLength > v6)
        {
            v7 = va((const char *)HIDWORD(fCurrLength), LODWORD(fCurrLength), LODWORD(v6));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2268,
                0,
                "%s\n\t%s",
                "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                v7);
        }
    }
    if (!animscript)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2269, 0, "%s", "animscript");
    v8 = Path_ConvertIndexToNode(pPath->pts[pPath->wNegotiationStartNode].iNodeNum);
    animscript->func = v8->constant.animscriptfunc;
    Scr_SetString(&animscript->name, v8->constant.animscript);
}

int __cdecl Path_NeedsReevaluation(const path_t *pPath)
{
    unsigned __int8 v2; // r11

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2286, 0, "%s", "pPath");
    if (pPath->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2289, 0, "%s", "pPath->wPathLen > 0");
    if ((pPath->flags & 4) == 0)
        return 0;
    v2 = 1;
    if (pPath->wPathLen > 8)
        return 0;
    return v2;
}

int __cdecl Path_EncroachesPoint2D(path_t *pPath, const float *vStart, const float *vPoint, float fMinDistSqrd)
{
    double v8; // fp0
    double v9; // fp13
    double v10; // fp12
    double v11; // fp12
    float fLookaheadDistToNextNode; // fp28
    float *vOrigPoint; // r30
    double v15; // fp30
    double v16; // fp31
    double v17; // fp0
    double v18; // fp29
    double v19; // fp13
    double v20; // fp0
    int v21; // r30
    float *v22; // r31
    double v23; // fp30
    double v24; // fp29
    double v25; // fp0
    double v26; // fp28
    double v27; // fp0
    double v28; // fp31
    double v29; // fp13
    double v30; // fp0

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2310, 0, "%s", "pPath");
    if (pPath->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2311, 0, "%s", "pPath->wPathLen > 0");
    if (!vPoint)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2312, 0, "%s", "vPoint");
    if (fMinDistSqrd < 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2313, 0, "%s", "fMinDistSqrd >= 0");
    if (pPath->lookaheadNextNode >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2314,
            0,
            "%s",
            "pPath->lookaheadNextNode < pPath->wPathLen");
    v10 = (float)((float)(pPath->lookaheadDir[0]
        * (float)(vPoint[1] - (float)((float)(pPath->lookaheadDir[1] * pPath->fLookaheadDist) + vStart[1])))
        - (float)(pPath->lookaheadDir[1]
            * (float)(*vPoint - (float)((float)(pPath->lookaheadDir[0] * pPath->fLookaheadDist) + *vStart))));
    if ((float)((float)v10 * (float)v10) < fMinDistSqrd)
    {
        v11 = (float)((float)(pPath->lookaheadDir[1]
            * (float)(vPoint[1]
                - (float)((float)(pPath->lookaheadDir[1] * pPath->fLookaheadDist) + vStart[1])))
            + (float)(pPath->lookaheadDir[0]
                * (float)(*vPoint - (float)((float)(pPath->lookaheadDir[0] * pPath->fLookaheadDist) + *vStart))));
        if (v11 >= 0.0)
        {
            if (v11 <= pPath->fLookaheadDist)
                return 1;
            if (Vec2DistanceSq(vPoint, vStart) < fMinDistSqrd)
                return 1;
        }
        else
        {
            v9 = (float)(vPoint[1] - (float)((float)(pPath->lookaheadDir[1] * pPath->fLookaheadDist) + vStart[1]));
            v8 = (float)(*vPoint - (float)((float)(pPath->lookaheadDir[0] * pPath->fLookaheadDist) + *vStart));
            if ((float)((float)((float)v8 * (float)v8) + (float)((float)v9 * (float)v9)) < fMinDistSqrd)
                return 1;
        }
    }
    fLookaheadDistToNextNode = pPath->fLookaheadDistToNextNode;
    if (fLookaheadDistToNextNode != 0.0)
    {
        if (pPath->lookaheadNextNode >= pPath->wPathLen - 1)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2349,
                0,
                "%s",
                "pPath->lookaheadNextNode < pPath->wPathLen - 1");
        vOrigPoint = pPath->pts[pPath->lookaheadNextNode].vOrigPoint;
        v15 = (float)(vPoint[1] - vOrigPoint[1]);
        v16 = (float)(*vPoint - *vOrigPoint);
        if (vOrigPoint[3] == 0.0 && vOrigPoint[4] == 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2360,
                0,
                "%s",
                "pPath->pts[i].fDir2D[0] || pPath->pts[i].fDir2D[1]");
        v17 = (float)((float)(vOrigPoint[3] * (float)v15) - (float)(vOrigPoint[4] * (float)v16));
        if ((float)((float)v17 * (float)v17) < fMinDistSqrd)
        {
            v18 = (float)((float)(vOrigPoint[4] * (float)v15) + (float)(vOrigPoint[3] * (float)v16));
            if (v18 >= 0.0)
            {
                if (fLookaheadDistToNextNode <= 0.0)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2374, 0, "%s", "fLength > 0");
                if (v18 <= fLookaheadDistToNextNode)
                    return 1;
                v19 = (float)((float)(vOrigPoint[4] * (float)fLookaheadDistToNextNode) + (float)v15);
                v20 = (float)((float)(vOrigPoint[3] * (float)fLookaheadDistToNextNode) + (float)v16);
                if ((float)((float)((float)v20 * (float)v20) + (float)((float)v19 * (float)v19)) < fMinDistSqrd)
                    return 1;
            }
            else if ((float)((float)((float)v16 * (float)v16) + (float)((float)v15 * (float)v15)) < fMinDistSqrd)
            {
                return 1;
            }
        }
    }
    v21 = pPath->lookaheadNextNode - 1;
    if (v21 >= 0)
    {
        v22 = &pPath->pts[v21].fDir2D[1];
        do
        {
            v23 = (float)(*vPoint - *(v22 - 4));
            v24 = (float)(vPoint[1] - *(v22 - 3));
            if (*(v22 - 1) == 0.0 && *v22 == 0.0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    2397,
                    0,
                    "%s",
                    "pPath->pts[i].fDir2D[0] || pPath->pts[i].fDir2D[1]");
            v25 = (float)((float)(*(v22 - 1) * (float)v24) - (float)(*v22 * (float)v23));
            if ((float)((float)v25 * (float)v25) < fMinDistSqrd)
            {
                v26 = (float)((float)(*v22 * (float)v24) + (float)(*(v22 - 1) * (float)v23));
                if (v26 >= 0.0)
                {
                    v28 = v22[1];
                    if (v28 <= 0.0)
                        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2413, 0, "%s", "fLength > 0");
                    if (v26 <= v28)
                        return 1;
                    v29 = (float)((float)(*v22 * (float)v28) + (float)v24);
                    v30 = (float)((float)(*(v22 - 1) * (float)v28) + (float)v23);
                    v27 = (float)((float)((float)v30 * (float)v30) + (float)((float)v29 * (float)v29));
                }
                else
                {
                    v27 = (float)((float)((float)v23 * (float)v23) + (float)((float)v24 * (float)v24));
                }
                if (v27 < fMinDistSqrd)
                    return 1;
            }
            --v21;
            v22 -= 7;
        } while (v21 >= 0);
    }
    return 0;
}

int __cdecl Path_DistanceGreaterThan(path_t *pPath, float fDist)
{
    int wPathLen; // r11
    float fCurrLength; // fp1
    double v6; // fp2
    const char *v7; // r3
    double v8; // fp31
    int lookaheadNextNode; // r11
    int v11; // r31
    float *i; // r30

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2449, 0, "%s", "pPath");
    if (pPath->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2450, 0, "%s", "pPath->wPathLen > 0");
    if ((unsigned __int16)pPath->wNegotiationStartNode >= 0x8000u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2451,
            0,
            "%s",
            "pPath->wNegotiationStartNode >= 0");
    if (pPath->wNegotiationStartNode > pPath->lookaheadNextNode)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2452,
            0,
            "%s",
            "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode");
    if (pPath->lookaheadNextNode >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2453,
            0,
            "%s",
            "pPath->lookaheadNextNode < pPath->wPathLen");
    wPathLen = pPath->wPathLen;
    if (wPathLen > 1)
    {
        fCurrLength = pPath->fCurrLength;
        v6 = *((float *)&pPath->pts[wPathLen - 1] - 2);
        if (fCurrLength > v6)
        {
            v7 = va((const char *)HIDWORD(fCurrLength), LODWORD(fCurrLength), LODWORD(v6));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2454,
                0,
                "%s\n\t%s",
                "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                v7);
        }
    }
    v8 = (float)(pPath->fLookaheadDistToNextNode + pPath->fLookaheadDist);
    if (pPath->wNegotiationStartNode)
        v8 = (float)((float)(pPath->fLookaheadDistToNextNode + pPath->fLookaheadDist) + (float)128.0);
    if (v8 > fDist)
        return 1;
    lookaheadNextNode = pPath->lookaheadNextNode;
    if (pPath->lookaheadNextNode)
    {
        v11 = lookaheadNextNode - 1;
        if (lookaheadNextNode - 1 == pPath->wPathLen - 2)
        {
            if (pPath->fCurrLength < 0.0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    2470,
                    0,
                    "%s",
                    "pPath->fCurrLength >= 0");
            v8 = (float)(pPath->fCurrLength + (float)v8);
            if (v8 > fDist)
                return 1;
            --v11;
        }
        if (v11 >= 0)
        {
            for (i = &pPath->pts[v11].fOrigLength; ; i -= 7)
            {
                if (*i <= 0.0)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                        2479,
                        0,
                        "%s",
                        "pPath->pts[i].fOrigLength > 0");
                v8 = (float)((float)v8 + *i);
                if (v8 > fDist)
                    break;
                if (--v11 < 0)
                    return 0;
            }
            return 1;
        }
    }
    return 0;
}

void __cdecl Path_ReduceLookaheadAmount(path_t *pPath, double maxLookaheadAmountIfReduce)
{
    int flags; // r10
    double v3; // fp0
    double v4; // fp0

    flags = pPath->flags;
    if ((flags & 2) != 0)
        v3 = 0.75;
    else
        v3 = 0.5625;
    v4 = (float)((float)maxLookaheadAmountIfReduce * (float)v3);
    pPath->fLookaheadAmount = v4;
    pPath->flags = flags & 0xFFFFFDFC;
    if (v4 < 0.001)
        pPath->fLookaheadAmount = 0.001;
}

bool __cdecl Path_FailedLookahead(path_t *pPath)
{
    return pPath->fLookaheadAmount <= 0.001;
}

void __cdecl Path_IncreaseLookaheadAmount(path_t *pPath)
{
    //pPath->numReductions = 0;
    //++pPath->numIncreases;

    pPath->fLookaheadAmount *= 1.1764705f;

    //if (ai_useBetterLookahead->current.enabled && !zombiemode->current.enabled)
    //{
    //    v3 = _Pow_int<float>(momentumFactor, pPath->numIncreases - 1);
    //    pPath->fLookaheadAmount = pPath->fLookaheadAmount * v3;
    //}

    pPath->fLookaheadAmount += 6.4f;

    if (pPath->fLookaheadAmount > 65536.0f)
        pPath->fLookaheadAmount = 65536.0f;

    if ((pPath->flags & 2) != 0)
        pPath->flags |= 0x200;
    else
        pPath->flags |= 2;
}

static const float PREDICTION_TRACE_MIN[3] = { -15.0, -15.0, 0.0 };
static const float PREDICTION_TRACE_MAX[3] = { 15.0, 15.0, 72.0 };

bool __cdecl Path_PredictionTrace(
    float *vStartPos,
    float *vEndPos,
    int entityIgnore,
    int mask,
    float *vTraceEndPos,
    double stepheight,
    int allowStartSolid)
{
    actor_s *actor; // r11
    float vSource[3]; // [sp+50h] [-F0h] BYREF
    float vDest[3]; // [sp+70h] [-D0h] BYREF
    float traceMin[3]; // [sp+80h] [-C0h] BYREF
    trace_t trace; // [sp+90h] [-B0h] BYREF
    float vDown[3];
    int hitEntId;


    PROF_SCOPED("pathpredictiontrace");

    traceMin[0] = PREDICTION_TRACE_MIN[0];
    traceMin[1] = PREDICTION_TRACE_MIN[1];
    traceMin[2] = stepheight + PREDICTION_TRACE_MIN[2];

    iassert(vStartPos);
    iassert(vEndPos);


    vDest[0] = vEndPos[0];
    vDest[1] = vEndPos[1];
    vDest[2] = vStartPos[2]; // this a bug?

    vSource[0] = vStartPos[0];
    vSource[1] = vStartPos[1];
    vSource[2] = vStartPos[2];

    while (1)
    {

        G_TraceCapsule(&trace, vSource, traceMin, PREDICTION_TRACE_MAX, vDest, entityIgnore, mask);
        Vec3Lerp(vSource, vDest, trace.fraction, vTraceEndPos);

        if (trace.fraction < 0.0001) // blops fix
        {
            break;
        }

        if (trace.startsolid && !allowStartSolid)
        {
            break;
        }

        if (!trace.allsolid && trace.fraction == 1.0f)
        {
            vSource[0] = vTraceEndPos[0];
            vSource[1] = vTraceEndPos[1];

            vDown[0] = vTraceEndPos[0];
            vDown[1] = vTraceEndPos[1];
            vDown[2] = vSource[2] - 48.0f;

            G_TraceCapsule(&trace, vSource, traceMin, PREDICTION_TRACE_MAX, vDown, entityIgnore, mask);
            Vec3Lerp(vSource, vDown, trace.fraction, vTraceEndPos);

            if (vTraceEndPos[2] < vSource[2] || trace.fraction == 1.0 || trace.normal[2] >= 0.7f)
            {
                vTraceEndPos[2] += stepheight;
                return I_fabs(((vTraceEndPos[2] + stepheight) - vEndPos[2])) < 48.0f;
            }
            else
            {
                return 0;
            }
        }

        hitEntId = Trace_GetEntityHitId(&trace);
        if (!level.gentities[hitEntId].sentient)
        {
            break;
        }

        actor = level.gentities[hitEntId].actor;
        if (!actor)
        {
            return 0;
        }

        if ((actor->eAnimMode != AI_ANIM_MOVE_CODE || !actor->moveMode) 
            && (!actor->pPileUpActor || actor->pPileUpActor->ent->s.number != entityIgnore))
        {
            return 0;
        }

        mask &= ~0x4000u;

 LABEL_45:
        vSource[0] = vTraceEndPos[0];
        vSource[1] = vTraceEndPos[1];
        vDown[0] = vTraceEndPos[0];
        vDown[1] = vTraceEndPos[1];
        vDown[2] = vSource[2] - 48.0f;
        G_TraceCapsule(&trace, vSource, traceMin, PREDICTION_TRACE_MAX, vDown, entityIgnore, mask);
        Vec3Lerp(vSource, vDown, trace.fraction, vTraceEndPos);
        if (vTraceEndPos[2] >= vSource[2] && trace.fraction != 1.0 && trace.normal[2] < 0.7f)
        {
            return 0;
        }
        vSource[2] = vTraceEndPos[2] + stepheight;
        vDest[2] = vSource[2];
    }

    if (Vec2DistanceSq(vTraceEndPos, vSource) >= 225.0f)
    {
        goto LABEL_45;
    }

    return 0;
}

int __cdecl Path_IsTrimmed(path_t *pPath)
{
    int wPathLen; // r11
    float fCurrLength; // fp1
    double v4; // fp2
    const char *v5; // r3
    int v6; // r10
    int result; // r3
    int *v8; // r11

    if (!pPath->wOrigPathLen)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2782, 0, "%s", "pPath->wOrigPathLen");
    if (!pPath->wPathLen)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2783, 0, "%s", "pPath->wPathLen");
    if ((unsigned __int16)pPath->wNegotiationStartNode >= 0x8000u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2784,
            0,
            "%s",
            "pPath->wNegotiationStartNode >= 0");
    if (pPath->wNegotiationStartNode >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2785,
            0,
            "%s",
            "pPath->wNegotiationStartNode < pPath->wPathLen");
    wPathLen = pPath->wPathLen;
    if (wPathLen > 1)
    {
        fCurrLength = pPath->fCurrLength;
        v4 = *((float *)&pPath->pts[wPathLen - 1] - 2);
        if (fCurrLength > v4)
        {
            v5 = va((const char *)HIDWORD(fCurrLength), LODWORD(fCurrLength), LODWORD(v4));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2786,
                0,
                "%s\n\t%s",
                "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                v5);
        }
    }
    v6 = pPath->wPathLen;
    if (v6 - 1 <= pPath->wNegotiationStartNode)
        return 0;
    v8 = (int *)&pPath->pts[v6];
    if (*(v8 - 1) < 0 || *(v8 - 8) < 0 || (pPath->flags & 0x20) != 0 || pPath->fLookaheadAmount <= 0.001)
        return 0;
    if (v6 != pPath->wOrigPathLen)
        return 1;
    if (pPath->vCurrPoint[0] != *((float *)v8 - 7))
        return 1;
    if (pPath->vCurrPoint[1] != *((float *)v8 - 6))
        return 1;
    result = 0;
    if (pPath->vCurrPoint[2] != *((float *)v8 - 5))
        return 1;
    return result;
}

void __cdecl Path_RemoveCompletedPathPoints(path_t *pPath, __int16 pathPointIndex)
{
    int wDodgeCount; // r10
    int v5; // r11
    double v6; // fp1
    double v7; // fp2
    const char *v8; // r3
    int *v9; // r11
    int v10; // r4
    __int16 wPathLen; // r9
    __int16 v12; // r10
    int v13; // r10
    int v14; // r11
    float fCurrLength; // fp1
    double v16; // fp2
    const char *v17; // r3
    int *v18; // r11

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2820, 0, "%s", "pPath");
    wDodgeCount = pPath->wDodgeCount;
    v5 = (__int16)(pathPointIndex + 2);
    if (wDodgeCount >= 0)
    {
        wPathLen = pPath->wPathLen;
        pPath->wPathLen = v5;
        v12 = wDodgeCount - wPathLen;
        pPath->wDodgeCount = v12;
        v14 = (__int16)(v12 + v5);
        v13 = (__int16)(pathPointIndex + 2);
        pPath->wDodgeCount = v14;
        if (v14 < 0)
            pPath->wDodgeCount = 0;
        if (v13 > 1)
        {
            fCurrLength = pPath->fCurrLength;
            v16 = *((float *)&pPath->pts[v13 - 1] - 2);
            if (fCurrLength > v16)
            {
                v17 = va((const char *)HIDWORD(fCurrLength), LODWORD(fCurrLength), LODWORD(v16));
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    2837,
                    0,
                    "%s\n\t%s",
                    "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                    v17);
            }
        }
        if (pPath->wPathLen)
        {
            if (pPath->wNegotiationStartNode)
            {
                v18 = (int *)&pPath->pts[pPath->wNegotiationStartNode];
                if (v18[6] < 0 || *(v18 - 1) < 0)
                {
                    v10 = 2838;
                    goto LABEL_22;
                }
            }
        }
    }
    else
    {
        pPath->wPathLen = v5;
        if (v5 > 1)
        {
            v6 = pPath->fCurrLength;
            v7 = *((float *)&pPath->pts[v5 - 1] - 2);
            if (v6 > v7)
            {
                v8 = va((const char *)HIDWORD(v6), LODWORD(v6), LODWORD(v7));
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    2825,
                    0,
                    "%s\n\t%s",
                    "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                    v8);
            }
        }
        if (pPath->wPathLen)
        {
            if (pPath->wNegotiationStartNode)
            {
                v9 = (int *)&pPath->pts[pPath->wNegotiationStartNode];
                if (v9[6] < 0 || *(v9 - 1) < 0)
                {
                    v10 = 2826;
                LABEL_22:
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                        v10,
                        0,
                        "%s",
                        "!pPath->wPathLen || !pPath->wNegotiationStartNode || (pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0"
                        " && pPath->pts[pPath->wNegotiationStartNode - 1].iNodeNum >= 0)");
                }
            }
        }
    }
}

void __cdecl Path_TrimCompletedPath(path_t *pPath, const float *vStartPos)
{
    int wPathLen; // r11
    float fCurrLength; // fp1
    double v6; // fp2
    const char *v7; // r3
    int v8; // r10
    int v9; // r28
    float *i; // r27
    double v11; // fp28
    pathpoint_t *vCurrPoint; // r25
    const char *v13; // r3
    float fraction; // fp29
    const char *v15; // r3
    const char *v16; // r3
    double v17; // fp31
    __int16 v18; // r4
    double v19; // fp13
    double v20; // fp12
    double v21; // fp1
    double v22; // fp2
    float d2; // [sp+50h] [-70h]
    float d1; // [sp+54h] [-6Ch]
    float v25; // [sp+54h] [-6Ch]

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2865, 0, "%s", "pPath");
    if (pPath->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2866, 0, "%s", "pPath->wPathLen > 0");
    if ((unsigned __int16)pPath->wNegotiationStartNode >= 0x8000u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2867,
            0,
            "%s",
            "pPath->wNegotiationStartNode >= 0");
    if (pPath->wNegotiationStartNode >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2868,
            0,
            "%s",
            "pPath->wNegotiationStartNode < pPath->wPathLen");
    wPathLen = pPath->wPathLen;
    if (wPathLen > 1)
    {
        fCurrLength = pPath->fCurrLength;
        v6 = *((float *)&pPath->pts[wPathLen - 1] - 2);
        if (fCurrLength > v6)
        {
            v7 = va((const char *)HIDWORD(fCurrLength), LODWORD(fCurrLength), LODWORD(v6));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2869,
                0,
                "%s\n\t%s",
                "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                v7);
        }
    }
    v8 = 1;
    v9 = pPath->wPathLen - 2;
    if (v9 >= pPath->wNegotiationStartNode)
    {
        for (i = pPath->pts[v9].vOrigPoint; ; i -= 7)
        {
            d2 = (float)(pPath->lookaheadDir[1] * (float)(i[1] - vStartPos[1]))
                + (float)(pPath->lookaheadDir[0] * (float)(*i - *vStartPos));
            if (d2 > 0.0)
                break;
            --v9;
            v8 = 0;
            if (v9 < pPath->wNegotiationStartNode)
                return;
        }
        if (v8)
        {
            v11 = pPath->fCurrLength;
            vCurrPoint = (pathpoint_t *)pPath->vCurrPoint;
        }
        else
        {
            v11 = i[5];
            vCurrPoint = &pPath->pts[v9 + 1];
        }
        d1 = (float)((float)(pPath->lookaheadDir[1] * (float)(vCurrPoint->vOrigPoint[1] - vStartPos[1]))
            + (float)((float)(vCurrPoint->vOrigPoint[0] - *vStartPos) * pPath->lookaheadDir[0]))
            - (float)0.000099999997;
        iassert(d1);
        iassert(d2);
        iassert(d1 - d2 < 0);
        fraction = (float)(d1 / (float)(d1 - d2));
        iassert(fraction >= 0);
        iassert(fraction <= 1.f);
        
        v25 = v11;
        if ((LODWORD(v25) & 0x7F800000) == 0x7F800000)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2907, 0, "%s", "!IS_NAN(fLength)");
        if (v11 <= 0.0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2908, 0, "%s", "fLength > 0");
        v17 = (float)((float)fraction * (float)v11);
        if (v17 > v11)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2911, 0, "%s", "dist <= fLength");
        if (v17 < v11)
        {
            if (i[3] == 0.0 && i[4] == 0.0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    2927,
                    0,
                    "%s",
                    "pt->fDir2D[0] || pt->fDir2D[1]");
            if (v17 < 0.0)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2928, 0, "%s", "dist >= 0");
            pPath->vCurrPoint[0] = (float)(i[3] * (float)((float)fraction * (float)v11)) + vCurrPoint->vOrigPoint[0];
            pPath->vCurrPoint[1] = (float)(i[4] * (float)((float)fraction * (float)v11)) + vCurrPoint->vOrigPoint[1];
            v19 = vCurrPoint->vOrigPoint[2];
            v20 = i[2];
            pPath->fCurrLength = (float)v11 - (float)((float)fraction * (float)v11);
            pPath->vCurrPoint[2] = (float)((float)((float)v20 - (float)v19) * (float)fraction) + (float)v19;
            if ((COERCE_UNSIGNED_INT((float)v11 - (float)((float)fraction * (float)v11)) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    2935,
                    0,
                    "%s",
                    "!IS_NAN(pPath->fCurrLength)");
            if (pPath->fCurrLength <= 0.0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    2936,
                    0,
                    "%s",
                    "pPath->fCurrLength > 0");
            v21 = i[5];
            v22 = pPath->fCurrLength;
            if (v21 < v22)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    2937,
                    0,
                    (const char *)HIDWORD(v21),
                    LODWORD(v21),
                    LODWORD(v22));
            v18 = v9;
        }
        else
        {
            if (v9 <= pPath->wNegotiationStartNode)
                return;
            v18 = v9 - 1;
            pPath->vCurrPoint[0] = *i;
            pPath->vCurrPoint[1] = i[1];
            pPath->vCurrPoint[2] = i[2];
            pPath->fCurrLength = pPath->pts[v9 - 1].fOrigLength;
        }
        Path_RemoveCompletedPathPoints(pPath, v18);
    }
}

void __cdecl Path_BacktrackCompletedPath(path_t *pPath, const float *vStartPos)
{
    int v4; // r22
    float *vOrigPoint; // r29
    float *v6; // r23
    double v7; // fp26
    double v8; // fp31
    double v9; // fp30
    float fCurrLength; // fp1
    double v11; // fp2
    double v12; // fp25
    double v13; // fp27
    float *v14; // r28
    double v15; // fp30
    double v16; // fp29
    double v17; // fp31
    double v18; // fp31
    double v19; // fp30
    __int16 wOrigPathLen; // r11
    double v21; // fp1
    double v22; // fp2
    const char *v23; // r3
    int *v24; // r11
    double v25; // fp0
    double v26; // fp29
    const char *v27; // r3
    const char *v28; // r3
    double v29; // fp31
    double v30; // fp1
    double v31; // fp2
    int v32; // r11
    double v33; // fp1
    double v34; // fp2
    const char *v35; // r3
    int *v36; // r11
    double v37; // fp0
    double v38; // fp31
    double v39; // fp1
    double v40; // fp2
    const char *v41; // r3
    int *v42; // r11
    float v43; // [sp+50h] [-C0h]
    float v44; // [sp+50h] [-C0h]

    v4 = pPath->wPathLen - 1;
    if (v4 <= pPath->wNegotiationStartNode)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2970,
            0,
            "%s",
            "i > pPath->wNegotiationStartNode");
    vOrigPoint = pPath->pts[v4].vOrigPoint;
    v6 = vOrigPoint - 7;
    v7 = (float)(pPath->fLookaheadAmount * (float)0.17647055);
    v8 = (float)(*(vOrigPoint - 7) - *vStartPos);
    v9 = (float)(*(vOrigPoint - 6) - vStartPos[1]);
    if (*(vOrigPoint - 4) == 0.0 && v6[4] == 0.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2976,
            0,
            "%s",
            "nextPt->fDir2D[0] || nextPt->fDir2D[1]");
    fCurrLength = pPath->fCurrLength;
    v11 = v6[5];
    v12 = I_fabs((float)((float)(v6[3] * (float)v9) - (float)(v6[4] * (float)v8)));
    v13 = -(float)(pPath->fCurrLength * (float)v12);
    if (fCurrLength > v11)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2981,
            0,
            (const char *)HIDWORD(fCurrLength),
            LODWORD(fCurrLength),
            LODWORD(v11));
    while (1)
    {
        if (v4 >= pPath->wOrigPathLen)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2985,
                0,
                "%s",
                "i < pPath->wOrigPathLen");
        v14 = vOrigPoint;
        v15 = (float)(*vOrigPoint - *vStartPos);
        v16 = (float)(vOrigPoint[1] - vStartPos[1]);
        if ((float)((float)((float)(*vOrigPoint - *vStartPos) * (float)(*vOrigPoint - *vStartPos))
            + (float)((float)v16 * (float)v16)) > 65536.0)
        {
            pPath->flags |= 0x20u;
            goto LABEL_91;
        }
        v17 = (float)((float)(pPath->lookaheadDir[1] * (float)(vOrigPoint[1] - vStartPos[1]))
            + (float)(pPath->lookaheadDir[0] * (float)(*vOrigPoint - *vStartPos)));
        if (v17 <= 0.0)
            break;
        if (v6[5] <= 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3000,
                0,
                "%s",
                "nextPt->fOrigLength > 0");
        v18 = (float)(v6[5] * (float)v12);
        v13 = (float)((float)(v6[5] * (float)v12) + (float)v13);
        if (v13 >= v7)
        {
            if ((float)((float)v13 - (float)(v6[5] * (float)v12)) >= v7)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    3006,
                    0,
                    "%s",
                    "amount - amountInc < largestAmount");
            if (v18 <= 0.0)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3007, 0, "%s", "amountInc > 0");
            v19 = (float)((float)((float)v13 - (float)v7) / (float)v18);
            if (v19 < 0.0)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3009, 0, "%s", "fraction >= 0");
            if (v19 > 1.0)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3010, 0, "%s", "fraction <= 1.f");
            v13 = (float)((float)v13 - (float)v18);
            goto handleFraction;
        }
        if (v6[5] <= 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3016,
                0,
                "%s",
                "nextPt->fOrigLength > 0");
        ++v4;
        vOrigPoint += 7;
        if (v4 > pPath->wOrigPathLen)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3019,
                0,
                "%s",
                "i <= pPath->wOrigPathLen");
        if (v4 >= pPath->wOrigPathLen)
        {
            v43 = v6[5];
            pPath->fCurrLength = v43;
            if ((LODWORD(v43) & 0x7F800000) == 0x7F800000)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    3023,
                    0,
                    "%s",
                    "!IS_NAN(pPath->fCurrLength)");
            if (pPath->fCurrLength <= 0.0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    3024,
                    0,
                    "%s",
                    "pPath->fCurrLength > 0");
            pPath->vCurrPoint[0] = *v14;
            pPath->vCurrPoint[1] = v14[1];
            pPath->vCurrPoint[2] = v14[2];
            wOrigPathLen = pPath->wOrigPathLen;
            pPath->wPathLen = wOrigPathLen;
            if (wOrigPathLen > 1)
            {
                v21 = pPath->fCurrLength;
                v22 = *((float *)&pPath->pts[wOrigPathLen - 1] - 2);
                if (v21 > v22)
                {
                    v23 = va((const char *)HIDWORD(v21), LODWORD(v21), LODWORD(v22));
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                        3028,
                        0,
                        "%s\n\t%s",
                        "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                        v23);
                }
            }
            if (Path_HasNegotiationNode(pPath))
            {
                v24 = (int *)&pPath->pts[pPath->wNegotiationStartNode];
                if (v24[6] < 0 || *(v24 - 1) < 0)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                        3029,
                        0,
                        "%s",
                        "!Path_HasNegotiationNode( pPath ) || (pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0 && pPath->pts[p"
                        "Path->wNegotiationStartNode - 1].iNodeNum >= 0)");
            }
            if (v13 > 0.0)
            {
                if ((pPath->flags & 2) == 0)
                    v13 = (float)((float)v13 * (float)0.75);
            LABEL_48:
                v25 = (float)(pPath->fLookaheadAmount + (float)v13);
                goto LABEL_111;
            }
            return;
        }
        v6 = v14;
        if (v14[3] == 0.0 && v14[4] == 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3044,
                0,
                "%s",
                "nextPt->fDir2D[0] || nextPt->fDir2D[1]");
        v12 = I_fabs((float)((float)(v14[3] * (float)v16) - (float)(v14[4] * (float)v15)));
    }
    v26 = (float)((float)(pPath->lookaheadDir[1] * (float)(v6[1] - vStartPos[1]))
        + (float)(pPath->lookaheadDir[0] * (float)(*v6 - *vStartPos)));
    if (v26 <= 0.0)
    {
        if (v17 > 0.0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3114, 0, "%s", "d1 <= 0");
    LABEL_91:
        pPath->vCurrPoint[0] = *v6;
        pPath->vCurrPoint[1] = v6[1];
        pPath->vCurrPoint[2] = v6[2];
        if ((COERCE_UNSIGNED_INT(v6[5]) & 0x7F800000) == 0x7F800000)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3120,
                0,
                "%s",
                "!IS_NAN(nextPt->fOrigLength)");
        if (v6[5] <= 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3121,
                0,
                "%s",
                "nextPt->fOrigLength > 0");
        if (v4 <= pPath->wNegotiationStartNode)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3123,
                0,
                "%s",
                "i > pPath->wNegotiationStartNode");
        if (v4 <= pPath->wNegotiationStartNode + 1)
            v37 = 0.0;
        else
            v37 = *((float *)&pPath->pts[v4 - 1] - 2);
        pPath->fCurrLength = v37;
        v38 = (float)((float)(v6[5] * (float)v12) + (float)v13);
        pPath->wPathLen = v4;
        if ((__int16)v4 > 1)
        {
            v39 = pPath->fCurrLength;
            v40 = *((float *)&pPath->pts[(__int16)v4 - 1] - 2);
            if (v39 > v40)
            {
                v41 = va((const char *)HIDWORD(v39), LODWORD(v39), LODWORD(v40));
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    3128,
                    0,
                    "%s\n\t%s",
                    "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                    v41);
            }
        }
        if (Path_HasNegotiationNode(pPath))
        {
            v42 = (int *)&pPath->pts[pPath->wNegotiationStartNode];
            if (v42[6] < 0 || *(v42 - 1) < 0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    3129,
                    0,
                    "%s",
                    "!Path_HasNegotiationNode( pPath ) || (pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0 && pPath->pts[pPa"
                    "th->wNegotiationStartNode - 1].iNodeNum >= 0)");
        }
        if (v38 > 0.0)
        {
            if ((pPath->flags & 2) == 0)
                v38 = (float)((float)v38 * (float)0.75);
            v25 = (float)(pPath->fLookaheadAmount + (float)v38);
            goto LABEL_111;
        }
        return;
    }
    if (v17 > 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3054, 0, "%s", "d1 <= 0");
    if ((float)((float)v17 - (float)v26) == 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3055, 0, "%s", "d1 - d2");
    v19 = (float)((float)v17 / (float)((float)v17 - (float)v26));
    if (v19 < 0.0)
    {
        v27 = va("i: %d, d1: %f, d2: %f, fraction: %f", HIDWORD(v17), LODWORD(v17), LODWORD(v26), LODWORD(v19));
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3059,
            0,
            "%s\n\t%s",
            "fraction >= 0",
            v27);
    }
    if (v19 > 1.0)
    {
        v28 = va("i: %d: %d, d1: %f, d2: %f, fraction: %f", HIDWORD(v17), LODWORD(v17), LODWORD(v26), LODWORD(v19));
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3062,
            0,
            "%s\n\t%s",
            "fraction <= 1.f",
            v28);
    }
handleFraction:
    if (v6[5] <= 0.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3065,
            0,
            "%s",
            "nextPt->fOrigLength > 0");
    v29 = (float)(v6[5] * (float)v19);
    if (v29 > v6[5])
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3068,
            0,
            "%s",
            "dist <= nextPt->fOrigLength");
    if (v29 < v6[5])
    {
        if (v29 < 0.0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3083, 0, "%s", "dist >= 0");
        if (v6[5] <= 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3084,
                0,
                "%s",
                "nextPt->fOrigLength > 0");
        v44 = v6[5] - (float)v29;
        pPath->fCurrLength = v44;
        if ((LODWORD(v44) & 0x7F800000) == 0x7F800000)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3086,
                0,
                "%s",
                "!IS_NAN(pPath->fCurrLength)");
        if (pPath->fCurrLength <= 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3087,
                0,
                "%s",
                "pPath->fCurrLength > 0");
        v30 = v6[5];
        v31 = pPath->fCurrLength;
        if (v30 < v31)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3088,
                0,
                (const char *)HIDWORD(v30),
                LODWORD(v30),
                LODWORD(v31));
        v13 = (float)((float)(pPath->fCurrLength * (float)v12) + (float)v13);
        if (v6[3] == 0.0 && v6[4] == 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3091,
                0,
                "%s",
                "nextPt->fDir2D[0] || nextPt->fDir2D[1]");
        pPath->vCurrPoint[0] = (float)(v6[3] * (float)v29) + *vOrigPoint;
        pPath->vCurrPoint[1] = (float)(v6[4] * (float)v29) + vOrigPoint[1];
        pPath->vCurrPoint[2] = (float)((float)(v6[2] - vOrigPoint[2]) * (float)v19) + vOrigPoint[2];
    }
    else
    {
        if (--v4 <= pPath->wNegotiationStartNode)
            return;
        pPath->vCurrPoint[0] = *v6;
        pPath->vCurrPoint[1] = v6[1];
        pPath->vCurrPoint[2] = v6[2];
        pPath->fCurrLength = pPath->pts[v4 - 1].fOrigLength;
    }
    v32 = (__int16)(v4 + 1);
    pPath->wPathLen = v32;
    if (v32 > 1)
    {
        v33 = pPath->fCurrLength;
        v34 = *((float *)&pPath->pts[v32 - 1] - 2);
        if (v33 > v34)
        {
            v35 = va((const char *)HIDWORD(v33), LODWORD(v33), LODWORD(v34));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3098,
                0,
                "%s\n\t%s",
                "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                v35);
        }
    }
    if (Path_HasNegotiationNode(pPath))
    {
        v36 = (int *)&pPath->pts[pPath->wNegotiationStartNode];
        if (v36[6] < 0 || *(v36 - 1) < 0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3099,
                0,
                "%s",
                "!Path_HasNegotiationNode( pPath ) || (pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0 && pPath->pts[pPath"
                "->wNegotiationStartNode - 1].iNodeNum >= 0)");
    }
    if (v13 > 0.0)
    {
        if ((pPath->flags & 2) != 0)
            goto LABEL_48;
        v25 = (float)(pPath->fLookaheadAmount + (float)((float)v13 * (float)0.75));
    LABEL_111:
        pPath->fLookaheadAmount = v25;
        if (v25 > 65536.0)
            pPath->fLookaheadAmount = 65536.0;
    }
}

void __cdecl PathCalcLookahead_CheckMinLookaheadNodes(path_t *pPath, const pathpoint_t *pt, int currentNode)
{
    int wPathLen; // r11
    float *vOrigPoint; // r31
    float v7; // [sp+50h] [-40h] BYREF
    float v8; // [sp+54h] [-3Ch]

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3199, 0, "%s", "pPath");
    wPathLen = pPath->wPathLen;
    if (currentNode + 3 == wPathLen)
    {
        if (currentNode >= wPathLen - 2)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3204,
                0,
                "%s",
                "currentNode < pPath->wPathLen - 2");
        vOrigPoint = pPath->pts[currentNode].vOrigPoint;
        v7 = *(vOrigPoint - 7) - vOrigPoint[7];
        v8 = *(vOrigPoint - 6) - vOrigPoint[8];
        Vec2Normalize(&v7);
        if ((float)((float)(*(vOrigPoint - 3) * v8) + (float)(*(vOrigPoint - 4) * v7)) >= 0.866
            && (float)((float)(*(vOrigPoint - 3) * vOrigPoint[4]) + (float)(*(vOrigPoint - 4) * vOrigPoint[3])) >= 0.17299999)
        {
            ++pPath->minLookAheadNodes;
        }
    }
}

int Path_GetForwardStartPos(path_t *pPath, const float *vStartPos, float *vForwardStartPos)
{
    if (pPath->wPathLen <= 1 || (pPath->flags & 1) != 0)
        return 0;

    float lookaheadDistance = 60.0f - pPath->fLookaheadDist;

    if (lookaheadDistance < 0.0f)
        lookaheadDistance = 0.0f;

    vForwardStartPos[0] = pPath->lookaheadDir[0] * lookaheadDistance + vStartPos[0];
    vForwardStartPos[1] = pPath->lookaheadDir[1] * lookaheadDistance + vStartPos[1];

    return 1;
}

void __cdecl Path_UpdateForwardLookahead_IncompletePath(
    path_t *pPath,
    const pathpoint_t *pt,
    const float *vForwardStartPos,
    double area,
    double height)
{
    double v10; // fp31
    float v11; // [sp+50h] [-50h]

    v11 = height;
    if ((LODWORD(v11) & 0x7F800000) == 0x7F800000)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3336, 0, "%s", "!IS_NAN(height)");
    if (height == 0.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3337,
            0,
            "%s\n\t(height) = %g",
            HIDWORD(height),
            LODWORD(height));
    v10 = (float)((float)area / (float)height);
    if (pt->fDir2D[0] == 0.0 && pt->fDir2D[1] == 0.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3340,
            0,
            "%s",
            "pt->fDir2D[0] || pt->fDir2D[1]");
    pPath->forwardLookaheadDir2D[0] = (float)(pt->fDir2D[0] * (float)-v10) + pt->vOrigPoint[0];
    pPath->forwardLookaheadDir2D[1] = (float)(pt->fDir2D[1] * (float)-v10) + pt->vOrigPoint[1];
    pPath->forwardLookaheadDir2D[0] = pPath->forwardLookaheadDir2D[0] - *vForwardStartPos;
    pPath->forwardLookaheadDir2D[1] = pPath->forwardLookaheadDir2D[1] - vForwardStartPos[1];
    Vec2Normalize(pPath->forwardLookaheadDir2D);
}

void __cdecl Path_UpdateForwardLookahead(path_t *pPath, const float *vStartPos)
{
    int wPathLen; // r11
    float fCurrLength; // fp1
    double v6; // fp2
    const char *v7; // r3
    double v8; // fp27
    double v9; // fp26
    int v10; // r11
    __int16 wNegotiationStartNode; // r8
    int v12; // r31
    const pathpoint_t *v13; // r28
    pathpoint_t *v14; // r25
    double v15; // fp0
    double v16; // fp13
    float fLength; // fp31
    float fraction; // fp28
    const char *v19; // r3
    const char *v20; // r3
    double lookaheadAmount; // fp30
    double DistToPathSegment; // fp2
    double v23; // fp31
    float *p_fOrigLength; // r29
    double v25; // fp1
    double v26; // fp2
    float *vOrigPoint; // r11
    float d2; // [sp+50h] [-80h]
    float d1; // [sp+54h] [-7Ch]
    float v30; // [sp+58h] [-78h] BYREF
    float v31; // [sp+5Ch] [-74h]

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3371, 0, "%s", "pPath");
    if (pPath->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3372, 0, "%s", "pPath->wPathLen > 0");
    if ((unsigned __int16)pPath->wNegotiationStartNode >= 0x8000u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3373,
            0,
            "%s",
            "pPath->wNegotiationStartNode >= 0");
    if (pPath->wNegotiationStartNode > pPath->lookaheadNextNode)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3374,
            0,
            "%s",
            "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode");
    if (pPath->lookaheadNextNode >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3375,
            0,
            "%s",
            "pPath->lookaheadNextNode < pPath->wPathLen");
    wPathLen = pPath->wPathLen;
    if (wPathLen > 1)
    {
        fCurrLength = pPath->fCurrLength;
        v6 = *((float *)&pPath->pts[wPathLen - 1] - 2);
        if (fCurrLength > v6)
        {
            v7 = va((const char *)HIDWORD(fCurrLength), LODWORD(fCurrLength), LODWORD(v6));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3376,
                0,
                "%s\n\t%s",
                "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                v7);
        }
    }
    if ((pPath->flags & 0x200) != 0
        && Path_GetForwardStartPos(pPath, vStartPos, &v30)
        && (v8 = v30,
            v9 = v31,
            (float)((float)(pPath->lookaheadDir[1] * (float)(pPath->vCurrPoint[1] - v31))
                + (float)(pPath->lookaheadDir[0] * (float)(pPath->vCurrPoint[0] - v30))) <= 0.0)
        && (v10 = 1,
            wNegotiationStartNode = pPath->wNegotiationStartNode,
            v12 = pPath->wPathLen - 2,
            v12 >= wNegotiationStartNode))
    {
        v13 = &pPath->pts[v12];
        while (1)
        {
            d2 = (float)(pPath->lookaheadDir[0] * (float)(v13->vOrigPoint[0] - v30))
                + (float)(pPath->lookaheadDir[1] * (float)(v13->vOrigPoint[1] - v31));
            if (d2 > 0.0)
                break;
            --v12;
            --v13;
            v10 = 0;
            if (v12 < wNegotiationStartNode)
                goto LABEL_21;
        }
        v14 = &pPath->pts[v12];
        if (v10)
        {
            fLength = pPath->fCurrLength;
            v15 = pPath->vCurrPoint[0];
            v16 = pPath->vCurrPoint[1];
        }
        else
        {
            v15 = v14[1].vOrigPoint[0];
            v16 = v14[1].vOrigPoint[1];
            fLength = v13->fOrigLength;
        }
        d1 = (float)(pPath->lookaheadDir[1] * (float)((float)v16 - v31))
            + (float)(pPath->lookaheadDir[0] * (float)((float)v15 - v30));

        iassert(d1);
        iassert(d2);
        iassert(d1 - d2);
        
        fraction = (float)(d1 / (float)(d1 - d2));
        iassert(fraction >= 0);
        iassert(fraction <= 1.f);
        
        iassert(fLength);
        
        lookaheadAmount = pPath->fLookaheadAmount;
        iassert(lookaheadAmount);
        DistToPathSegment = Path_GetDistToPathSegment(&v30, v13);
        v23 = (float)((float)((float)fLength - (float)((float)(d1 / (float)(d1 - d2)) * (float)fLength))
            * (float)DistToPathSegment);
        if (v23 < lookaheadAmount)
        {
            if (v12 <= pPath->wNegotiationStartNode)
            {
            LABEL_47:
                vOrigPoint = pPath->pts[pPath->wNegotiationStartNode].vOrigPoint;
                pPath->forwardLookaheadDir2D[0] = *vOrigPoint - (float)v8;
                pPath->forwardLookaheadDir2D[1] = vOrigPoint[1] - (float)v9;
                Vec2Normalize(pPath->forwardLookaheadDir2D);
            }
            else
            {
                p_fOrigLength = &v14->fOrigLength;
                while (1)
                {
                    p_fOrigLength -= 7;
                    --v12;
                    v25 = *p_fOrigLength;
                    if (v25 <= 0.0)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                            3445,
                            0,
                            "%s\n\t(pt->fOrigLength) = %g",
                            HIDWORD(v25),
                            LODWORD(v25));
                    v26 = Path_GetDistToPathSegment(&v30, (const pathpoint_t *)(p_fOrigLength - 5));
                    v23 = (float)((float)(*p_fOrigLength * (float)v26) + (float)v23);
                    if (v23 >= lookaheadAmount)
                        break;
                    if (v12 <= pPath->wNegotiationStartNode)
                        goto LABEL_47;
                }
                Path_UpdateForwardLookahead_IncompletePath(
                    pPath,
                    (const pathpoint_t *)(p_fOrigLength - 5),
                    &v30,
                    (float)((float)v23 - (float)lookaheadAmount),
                    v26);
            }
        }
        else
        {
            Path_UpdateForwardLookahead_IncompletePath(
                pPath,
                v13,
                &v30,
                (float)((float)v23 - (float)lookaheadAmount),
                DistToPathSegment);
        }
    }
    else
    {
    LABEL_21:
        pPath->forwardLookaheadDir2D[0] = pPath->lookaheadDir[0];
        pPath->forwardLookaheadDir2D[1] = pPath->lookaheadDir[1];
    }
}

void __cdecl Path_DebugDraw(path_t *pPath, float *vStartPos, int bDrawLookahead)
{
    float fLookaheadDist; // fp0
    double v6; // fp10
    double v7; // fp13
    double v8; // fp9
    double v9; // fp7
    double v10; // fp11
    __int16 wPathLen; // r11
    int v12; // r29
    double v13; // fp0
    int v14; // r30
    int wNegotiationStartNode; // r11
    double v16; // fp0
    const float *v17; // r5
    float *v18; // r29
    int v19; // r11
    const float *v20; // r5
    float v21; // [sp+50h] [-60h] BYREF
    float v22; // [sp+54h] [-5Ch]
    float v23; // [sp+58h] [-58h]
    float v24; // [sp+60h] [-50h] BYREF
    float v25; // [sp+64h] [-4Ch]
    float v26; // [sp+68h] [-48h]

    if (pPath->wPathLen)
    {
        if (bDrawLookahead)
        {
            fLookaheadDist = pPath->fLookaheadDist;
            v6 = (float)(pPath->lookaheadDir[2] * pPath->fLookaheadDist);
            v7 = vStartPos[2];
            v8 = pPath->lookaheadDir[0];
            v9 = pPath->lookaheadDir[1];
            v10 = vStartPos[1];
            v24 = *vStartPos;
            v25 = v10;
            v21 = v24 + (float)((float)v8 * (float)fLookaheadDist);
            v22 = (float)v10 + (float)((float)v9 * (float)fLookaheadDist);
            v26 = (float)v7 + (float)16.0;
            v23 = (float)((float)v7 + (float)v6) + (float)16.0;
            G_DebugLine(&v24, &v21, colorRed, 0);
        }
        wPathLen = pPath->wPathLen;
        v26 = vStartPos[2] + (float)16.0;
        v12 = wPathLen;
        v13 = vStartPos[1];
        v24 = *vStartPos;
        v25 = v13;
        if (!wPathLen)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3615, 0, "%s", "i");
        v14 = v12 - 1;
        wNegotiationStartNode = pPath->wNegotiationStartNode;
        v23 = pPath->vCurrPoint[2] + (float)16.0;
        v16 = pPath->vCurrPoint[1];
        v21 = pPath->vCurrPoint[0];
        v22 = v16;
        v17 = colorBlue;
        if (v12 - 1 == wNegotiationStartNode - 1)
            v17 = colorCyan;
        G_DebugLine(&v24, &v21, v17, 0);
        v24 = v21;
        v25 = v22;
        v26 = v23;
        if (v12 != 1)
        {
            v18 = &pPath->pts[v14].vOrigPoint[2];
            do
            {
                v18 -= 7;
                --v14;
                v19 = pPath->wNegotiationStartNode - 1;
                v21 = *(v18 - 2);
                v22 = *(v18 - 1);
                v20 = colorBlue;
                v23 = *v18 + (float)16.0;
                if (v14 == v19)
                    v20 = colorCyan;
                G_DebugLine(&v24, &v21, v20, 0);
                v24 = v21;
                v25 = v22;
                v26 = v23;
            } while (v14);
        }
    }
}

bool __cdecl Path_WithinApproxDist(path_t *pPath, double checkDist)
{
    int lookaheadNextNode; // r11
    double v5; // fp31
    int v7; // r31
    float *i; // r30

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3654, 0, "%s", "pPath");
    if (pPath->wPathLen)
    {
        if ((unsigned __int16)pPath->wNegotiationStartNode >= 0x8000u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3659,
                0,
                "%s",
                "pPath->wNegotiationStartNode >= 0");
        if (pPath->wNegotiationStartNode > pPath->lookaheadNextNode)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3660,
                0,
                "%s",
                "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode");
        if (pPath->lookaheadNextNode >= pPath->wPathLen)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3661,
                0,
                "%s",
                "pPath->lookaheadNextNode < pPath->wPathLen");
        lookaheadNextNode = pPath->lookaheadNextNode;
        v5 = (float)(pPath->fLookaheadDistToNextNode + pPath->fLookaheadDist);
        if (lookaheadNextNode == pPath->wNegotiationStartNode)
            return v5 < checkDist;
        v7 = lookaheadNextNode - 1;
        if (lookaheadNextNode - 1 == pPath->wPathLen - 2)
        {
            if (pPath->fCurrLength <= 0.0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    3672,
                    0,
                    "%s",
                    "pPath->fCurrLength > 0");
            --v7;
            v5 = (float)(pPath->fCurrLength + (float)v5);
        }
        if (v5 >= checkDist)
            return 0;
        if (v7 >= pPath->wNegotiationStartNode)
        {
            for (i = &pPath->pts[v7].fOrigLength; ; i -= 7)
            {
                if (*i <= 0.0)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                        3682,
                        0,
                        "%s",
                        "pPath->pts[i].fOrigLength > 0");
                v5 = (float)((float)v5 + *i);
                if (v5 >= checkDist)
                    break;
                if (--v7 < pPath->wNegotiationStartNode)
                    return 1;
            }
            return 0;
        }
    }
    return 1;
}

ai_stance_e __cdecl Path_AllowedStancesForPath(path_t *pPath)
{
    int wPathLen; // r11
    float fCurrLength; // fp1
    double v4; // fp2
    const char *v5; // r3
    int v6; // r31
    float *vOrigPoint; // r11
    int *p_iNodeNum; // r28
    pathnode_t *v9; // r29
    int iNodeNum; // r3
    const pathnode_t *v11; // r3
    int *v13; // r31
    const pathnode_t *v14; // r3
    pathnode_t *v15; // r3

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3703, 0, "%s", "pPath");
    if (pPath->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3704, 0, "%s", "pPath->wPathLen > 0");
    if ((unsigned __int16)pPath->wNegotiationStartNode >= 0x8000u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3705,
            0,
            "%s",
            "pPath->wNegotiationStartNode >= 0");
    if (pPath->wNegotiationStartNode > pPath->lookaheadNextNode)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3706,
            0,
            "%s",
            "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode");
    if (pPath->lookaheadNextNode >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3707,
            0,
            "%s",
            "pPath->lookaheadNextNode < pPath->wPathLen");
    wPathLen = pPath->wPathLen;
    if (wPathLen > 1)
    {
        fCurrLength = pPath->fCurrLength;
        v4 = *((float *)&pPath->pts[wPathLen - 1] - 2);
        if (fCurrLength > v4)
        {
            v5 = va((const char *)HIDWORD(fCurrLength), LODWORD(fCurrLength), LODWORD(v4));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3708,
                0,
                "%s\n\t%s",
                "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                v5);
        }
    }
    v6 = pPath->wPathLen - 1;
    if (v6 < pPath->wNegotiationStartNode)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3711,
            0,
            "%s",
            "i >= pPath->wNegotiationStartNode");
    vOrigPoint = pPath->pts[v6].vOrigPoint;
    if (pPath->vCurrPoint[0] != *vOrigPoint
        || pPath->vCurrPoint[1] != vOrigPoint[1]
        || pPath->vCurrPoint[2] != vOrigPoint[2])
    {
        --v6;
    }
    if (v6 >= pPath->wNegotiationStartNode)
    {
        p_iNodeNum = &pPath->pts[v6].iNodeNum;
        do
        {
            if (*p_iNodeNum >= 0)
            {
                v9 = Path_ConvertIndexToNode(*p_iNodeNum);
                if (!v9)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\pathnode.h", 166, 0, "%s", "node");
                if (((1 << v9->constant.type) & 0xC1FFC) == 0)
                    break;
            }
            --v6;
            p_iNodeNum -= 7;
        } while (v6 >= pPath->wNegotiationStartNode);
    }
    if (v6 < pPath->wNegotiationStartNode)
    {
        v6 = pPath->wPathLen - 1;
        iNodeNum = pPath->pts[v6].iNodeNum;
        if (iNodeNum < 0)
            return STANCE_ANY;
        v11 = Path_ConvertIndexToNode(iNodeNum);
        if (!Path_IsPathStanceNode(v11))
            return STANCE_ANY;
    }
    v13 = (int *)&pPath->pts[v6];
    if (v13[6] < 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3726,
            0,
            "%s",
            "pPath->pts[i].iNodeNum >= 0");
    v14 = Path_ConvertIndexToNode(v13[6]);
    if (!Path_IsPathStanceNode(v14))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3727,
            0,
            "%s",
            "Path_IsPathStanceNode( Path_ConvertIndexToNode( pPath->pts[i].iNodeNum ) )");
    v15 = Path_ConvertIndexToNode(v13[6]);
    return (ai_stance_e)Path_AllowedStancesForNode(v15);
}

void __cdecl Path_DodgeDrawRaisedLine(float *start, float *end, const float *color)
{
    double v3; // fp12
    float v4[4]; // [sp+50h] [-30h] BYREF
    float v5[6]; // [sp+60h] [-20h] BYREF

    if (ai_showDodge->current.enabled)
    {
        v4[0] = *start;
        v4[1] = start[1];
        v5[0] = *end;
        v3 = end[2];
        v5[1] = end[1];
        v4[2] = start[2] + (float)50.0;
        v5[2] = (float)v3 + (float)50.0;
        G_DebugLineWithDuration(v4, v5, color, 0, 25);
    }
}

int __cdecl Path_MayFaceEnemy(path_t *pPath, float *vEnemyDir, float *vOrg)
{
    int wPathLen; // r11
    float fCurrLength; // fp1
    double v7; // fp2
    const char *v8; // r3
    int result; // r3
    __int64 v10; // r11

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 4034, 0, "%s", "pPath");
    if (pPath->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 4035, 0, "%s", "pPath->wPathLen > 0");
    if (!vEnemyDir)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 4036, 0, "%s", "vEnemyDir");
    if ((unsigned __int16)pPath->wNegotiationStartNode >= 0x8000u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            4037,
            0,
            "%s",
            "pPath->wNegotiationStartNode >= 0");
    if (pPath->wNegotiationStartNode > pPath->lookaheadNextNode)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            4038,
            0,
            "%s",
            "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode");
    if (pPath->lookaheadNextNode >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            4039,
            0,
            "%s",
            "pPath->lookaheadNextNode < pPath->wPathLen");
    wPathLen = pPath->wPathLen;
    if (wPathLen > 1)
    {
        fCurrLength = pPath->fCurrLength;
        v7 = *((float *)&pPath->pts[wPathLen - 1] - 2);
        if (fCurrLength > v7)
        {
            v8 = va((const char *)HIDWORD(fCurrLength), LODWORD(fCurrLength), LODWORD(v7));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                4040,
                0,
                "%s\n\t%s",
                "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                v8);
        }
    }
    if (level.time - pPath->iPathTime < 300)
        return 1;
    if (!Path_DistanceGreaterThan(pPath, 128.0))
        return 1;
    result = 0;
    if (pPath->fLookaheadDist >= (double)(float)-(float)((float)((float)((float)((float)((float)(pPath->lookaheadDir[1]
        * vEnemyDir[1])
        + (float)(*vEnemyDir
            * pPath->lookaheadDir[0]))
        + (float)1.0)
        * (float)0.5)
        * (float)48.0)
        - (float)72.0))
    {
        HIDWORD(v10) = pPath->iPathTime;
        LODWORD(v10) = level.time - HIDWORD(v10);
        if ((float)v10 < (double)(float)((float)((float)((float)((float)((float)(pPath->lookaheadDir[1] * vEnemyDir[1])
            + (float)(*vEnemyDir * pPath->lookaheadDir[0]))
            + (float)1.0)
            * (float)0.5)
            * (float)3500.0)
            + (float)1500.0))
            return 1;
    }
    return result;
}

void __cdecl Path_Restore(path_t *path)
{
    if (!path)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 135, 0, "%s", "path");
    if (path->wPathLen)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 136, 0, "%s", "path->wPathLen == 0");
    Com_Memcpy(path, &pathBackup, 996);
    if (Path_HasNegotiationNode(path))
        Path_IncrementNodeUserCount(path);
}

template <typename T, bool USE_IGNORE = false, bool CHECK_NODETO = true>
static bool Path_AStarAlgorithm(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    pathnode_t *pNodeFrom,
    const float *vGoalPos,
    int bIncludeGoalInPath,
    int bAllowNegotiationLinks,
    T *custom)
{
    pathnode_t *nodeTo; // r8
    pathnode_t *pCurrent; // r30
    int v19; // r28
    int i; // r29
    pathlink_s *v21; // r11
    pathnode_t *pSuccessor; // r31
    double v23; // fp0
    pathnode_t *pPrevOpen; // r11
    pathnode_t *v25; // r11
    double fApproxTotalCost; // fp0
    pathnode_t *pInsert; // r11
    pathnode_t *next; // r10
    int Path; // r31
    pathnode_t topParent;
    pathnode_t *pNextOpen; // [sp+B8h] [-A8h]

    //Profile_Begin(231);
    PROF_SCOPED("Path_AStarAlgorithm");
    iassert(vGoalPos || !bIncludeGoalInPath);
    bcassert((unsigned int)eTeam, ARRAY_COUNT(((pathlink_s *)0)->ubBadPlaceCount));

    if (vGoalPos)
    {
        g_pathAttemptGoalPos[0] = vGoalPos[0];
        g_pathAttemptGoalPos[1] = vGoalPos[1];
        g_pathAttemptGoalPos[2] = vGoalPos[2];
    }
    else
    {
        g_pathAttemptGoalPos[0] = 0.0;
        g_pathAttemptGoalPos[1] = 0.0;
        g_pathAttemptGoalPos[2] = 0.0;
    }

    pNodeFrom->transient.fCost = 0.0;
    pNodeFrom->transient.iSearchFrame = ++level.iSearchFrame;
    pNodeFrom->transient.pParent = &topParent;
    pNodeFrom->transient.pNextOpen = 0;
    pNodeFrom->transient.pPrevOpen = &topParent;

    topParent.transient.pNextOpen = pNodeFrom;

LABEL_12:
    if (!topParent.transient.pNextOpen)
    {
        return 0;
    }

    pCurrent = topParent.transient.pNextOpen;

    bool nodeToCheck = true;

    if constexpr (CHECK_NODETO)
    {
        nodeToCheck = (pCurrent != custom->m_pNodeTo);
    }

    if (nodeToCheck)
    {
        topParent.transient.pNextOpen = topParent.transient.pNextOpen->transient.pNextOpen;
        if (topParent.transient.pNextOpen)
        {
            topParent.transient.pNextOpen->transient.pPrevOpen = &topParent;
        }

        for (i = 0; ; i++)
        {
            if (i >= pCurrent->dynamic.wLinkCount)
            {
                pCurrent->transient.pPrevOpen = 0;
                goto LABEL_12;
            }

            if constexpr (USE_IGNORE)
            {
                if (custom->IgnoreNode(pSuccessor))
                {
                    continue;
                }
            }

            if (/*ignorebadplaces || */ !pCurrent->constant.Links[i].ubBadPlaceCount[eTeam])
            {
                pSuccessor = Path_ConvertIndexToNode(pCurrent->constant.Links[i].nodeNum);
                iassert(pSuccessor != pCurrent);

                if ((pCurrent->constant.type != NODE_NEGOTIATION_BEGIN
                        || pSuccessor->constant.type != NODE_NEGOTIATION_END
                        || !pCurrent->dynamic.wOverlapCount && !pSuccessor->dynamic.wOverlapCount))
                {
                    float fCost;
                    if (pSuccessor->transient.iSearchFrame == level.iSearchFrame)
                    {
                        fCost = (float)(pCurrent->constant.Links[i].fDist * 1.0) + pCurrent->transient.fCost;
                        if (fCost >= pSuccessor->transient.fCost)
                            continue;
                        if (pSuccessor->transient.pPrevOpen)
                        {
                            pSuccessor->transient.pPrevOpen->transient.pNextOpen = pSuccessor->transient.pNextOpen;
                            if (pSuccessor->transient.pNextOpen)
                                pSuccessor->transient.pNextOpen->transient.pPrevOpen = pSuccessor->transient.pPrevOpen;
                        }
                    }
                    else
                    {
                        pSuccessor->transient.iSearchFrame = level.iSearchFrame;
                        //v10 = CustomSearchInfo_FindPath::EvaluateHeuristic(custom, pSuccessor, vGoalPos);
                        pSuccessor->transient.fHeuristic = custom->EvaluateHeuristic(pSuccessor, vGoalPos);
                        fCost = (float)(pCurrent->constant.Links[i].fDist * 1.0) + pCurrent->transient.fCost;
                    }
                    pSuccessor->transient.pParent = pCurrent;
                    pSuccessor->transient.fCost = fCost;
                    fApproxTotalCost = pSuccessor->transient.fCost + pSuccessor->transient.fHeuristic;

                    for (pInsert = &topParent;
                        pInsert->transient.pNextOpen
                        && (float)(pInsert->transient.pNextOpen->transient.fCost
                            + pInsert->transient.pNextOpen->transient.fHeuristic) < fApproxTotalCost;
                        pInsert = pInsert->transient.pNextOpen)
                    {
                        ;
                    }

                    iassert(pInsert);

                    pSuccessor->transient.pPrevOpen = pInsert;
                    pSuccessor->transient.pNextOpen = pInsert->transient.pNextOpen;
                    pInsert->transient.pNextOpen = pSuccessor;
                    if (pSuccessor->transient.pNextOpen)
                        pSuccessor->transient.pNextOpen->transient.pPrevOpen = pSuccessor;
                }
            }
        }
    }

    if (pPath)
    {
        int success = Path_GeneratePath(
            pPath,
            eTeam,
            (float*)vStartPos,
            (float*)vGoalPos,
            pNodeFrom,
            topParent.transient.pNextOpen,
            bIncludeGoalInPath,
            bAllowNegotiationLinks);

        iassert(pPath->wPathLen <= pPath->wOrigPathLen);
        iassert(success || !pPath->wOrigPathLen);

        return success;
    }
    else
    {
        return 1;
    }
}

bool __cdecl Path_FindPathFromTo(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vStartPos,
    pathnode_t *pNodeTo,
    const float *vGoalPos,
    bool bAllowNegotiationLinks)
{
    CustomSearchInfo_FindPath info; // [sp+50h] [-70h] BYREF

    info.negotiationOverlapCost = ai_pathNegotiationOverlapCost->current.value;
    
    iassert(pNodeFrom);
    iassert(pNodeTo);
    iassert((pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0);
    iassert((pNodeTo->constant.spawnflags & PNF_DONTLINK) == 0);

    info.startPos[0] = vStartPos[0];
    info.startPos[1] = vStartPos[1];
    info.startPos[2] = vStartPos[2];
    info.m_pNodeTo = pNodeTo;

    return Path_AStarAlgorithm<CustomSearchInfo_FindPath>(
        pPath,
        eTeam,
        vStartPos,
        pNodeFrom,
        vGoalPos,
        1,
        bAllowNegotiationLinks,
        &info);
}

void __cdecl Path_TrimLastNodes(path_t *pPath, const int iNodeCount, bool bMaintainGoalPos)
{
    int wPathLen; // r11
    float fCurrLength; // fp1
    double v8; // fp2
    const char *v9; // r3
    __int16 v10; // r9
    int wDodgeCount; // r11
    int v12; // r11
    int v13; // r11
    int wNegotiationStartNode; // r10
    __int16 lookaheadNextNode; // r9
    unsigned int v16; // r11
    int v17; // r10
    int v18; // r11
    double v19; // fp1
    double v20; // fp2
    const char *v21; // r3
    int *v22; // r11

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 1896, 0, "%s", "pPath");
    if (iNodeCount < 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 1897, 0, "%s", "iNodeCount >= 0");
    if (iNodeCount >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            1898,
            0,
            "%s",
            "iNodeCount < pPath->wPathLen");
    if ((unsigned __int16)pPath->wNegotiationStartNode >= 0x8000u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            1899,
            0,
            "%s",
            "pPath->wNegotiationStartNode >= 0");
    if (pPath->wNegotiationStartNode > pPath->lookaheadNextNode)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            1900,
            0,
            "%s",
            "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode");
    if (pPath->lookaheadNextNode >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            1901,
            0,
            "%s",
            "pPath->lookaheadNextNode < pPath->wPathLen");
    if (pPath->fLookaheadDistToNextNode > (double)pPath->pts[pPath->lookaheadNextNode].fOrigLength)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            1902,
            0,
            "%s",
            "pPath->fLookaheadDistToNextNode <= pPath->pts[pPath->lookaheadNextNode].fOrigLength");
    wPathLen = pPath->wPathLen;
    if (wPathLen > 1)
    {
        fCurrLength = pPath->fCurrLength;
        v8 = *((float *)&pPath->pts[wPathLen - 1] - 2);
        if (fCurrLength > v8)
        {
            v9 = va((const char *)HIDWORD(fCurrLength), LODWORD(fCurrLength), LODWORD(v8));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                1903,
                0,
                "%s\n\t%s",
                "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                v9);
        }
    }
    if (iNodeCount)
    {
        v10 = pPath->wOrigPathLen - iNodeCount;
        wDodgeCount = pPath->wDodgeCount;
        pPath->wPathLen -= iNodeCount;
        pPath->wOrigPathLen = v10;
        if (wDodgeCount >= 0)
        {
            v12 = (__int16)(wDodgeCount - iNodeCount);
            pPath->wDodgeCount = v12;
            if (v12 < 0)
                pPath->wDodgeCount = 0;
        }
        v13 = (__int16)(pPath->lookaheadNextNode - iNodeCount);
        pPath->lookaheadNextNode = v13;
        if (v13 < 0)
        {
            pPath->fLookaheadDistToNextNode = 0.0;
            pPath->lookaheadNextNode = 0;
        }
        wNegotiationStartNode = pPath->wNegotiationStartNode;
        if (wNegotiationStartNode - iNodeCount > 0)
        {
            pPath->wNegotiationStartNode = wNegotiationStartNode - iNodeCount;
        }
        else
        {
            if (wNegotiationStartNode > 0)
                Path_DecrementNodeUserCount(pPath);
            pPath->wNegotiationStartNode = 0;
        }
        if (pPath->wPathLen <= 0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 1938, 0, "%s", "pPath->wPathLen > 0");
        if (pPath->wPathLen <= 1)
        {
            if (pPath->wNegotiationStartNode)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    1946,
                    0,
                    "%s",
                    "!pPath->wNegotiationStartNode");
            pPath->pts[0].vOrigPoint[0] = pPath->vCurrPoint[0];
            pPath->pts[0].vOrigPoint[1] = pPath->vCurrPoint[1];
            pPath->pts[0].vOrigPoint[2] = pPath->vCurrPoint[2];
            pPath->wOrigPathLen = 1;
            pPath->fCurrLength = 0.0;
            pPath->pts[0].iNodeNum = -1;
            pPath->pts[0].fOrigLength = 0.0;
            pPath->pts[0].fDir2D[0] = 0.0;
            pPath->pts[0].fDir2D[1] = 0.0;
        }
        else
        {
            memmove(pPath, &pPath->pts[iNodeCount], 28 * pPath->wOrigPathLen);
        }
        if (!bMaintainGoalPos)
        {
            pPath->vFinalGoal[0] = pPath->pts[0].vOrigPoint[0];
            pPath->vFinalGoal[1] = pPath->pts[0].vOrigPoint[1];
            pPath->vFinalGoal[2] = pPath->pts[0].vOrigPoint[2];
        }
        lookaheadNextNode = pPath->lookaheadNextNode;
        v16 = pPath->flags & 0xFFFFFFFB;
        v17 = pPath->wNegotiationStartNode;
        pPath->iPathEndTime = 0;
        pPath->flags = v16;
        if (v17 > lookaheadNextNode)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                1964,
                0,
                "%s",
                "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode");
        if (pPath->lookaheadNextNode >= pPath->wPathLen)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                1965,
                0,
                "%s",
                "pPath->lookaheadNextNode < pPath->wPathLen");
        if (pPath->fLookaheadDistToNextNode > (double)pPath->pts[pPath->lookaheadNextNode].fOrigLength)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                1966,
                0,
                "%s",
                "pPath->fLookaheadDistToNextNode <= pPath->pts[pPath->lookaheadNextNode].fOrigLength");
        v18 = pPath->wPathLen;
        if (v18 > 1)
        {
            v19 = pPath->fCurrLength;
            v20 = *((float *)&pPath->pts[v18 - 1] - 2);
            if (v19 > v20)
            {
                v21 = va((const char *)HIDWORD(v19), LODWORD(v19), LODWORD(v20));
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    1967,
                    0,
                    "%s\n\t%s",
                    "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                    v21);
            }
        }
        if (Path_HasNegotiationNode(pPath))
        {
            v22 = (int *)&pPath->pts[pPath->wNegotiationStartNode];
            if (v22[6] < 0 || *(v22 - 1) < 0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    1968,
                    0,
                    "%s",
                    "!Path_HasNegotiationNode( pPath ) || (pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0 && pPath->pts[pPa"
                    "th->wNegotiationStartNode - 1].iNodeNum >= 0)");
        }
    }
}

int __cdecl Path_ClipToGoal(path_t *pPath, const actor_goal_s *goal)
{
    int v5; // r31
    const float *i; // r30

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 1985, 0, "%s", "pPath");
    if ((unsigned __int16)pPath->wPathLen >= 0x8000u)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 1986, 0, "%s", "pPath->wPathLen >= 0");
    if (pPath->wPathLen)
    {
        if (!(unsigned __int8)Actor_PointAtGoal(pPath->vCurrPoint, goal))
            return 0;
        v5 = pPath->wPathLen - 2;
        if (v5 >= 0)
        {
            for (i = pPath->pts[v5].vOrigPoint; (unsigned __int8)Actor_PointAtGoal(i, goal); i -= 7)
            {
                if (--v5 < 0)
                    return 1;
            }
            Path_TrimLastNodes(pPath, v5 + 1, 0);
        }
    }
    return 1;
}

int __cdecl Path_TrimToSeePoint(
    path_t *pPath,
    path_trim_t *pTrim,
    actor_s *pActor,
    float fMaxDistSqrd,
    int iIgnoreEntityNum,
    const float *vPoint)
{
    int i; // r31
    int v14; // r6
    bool v15; // r28
    float *vOrigPoint; // r11
    double v17; // fp0
    double v18; // fp13
    bool CanSeePointFrom; // r3
    int v20; // r11
    int result; // r3
    float v22; // [sp+50h] [-80h] BYREF
    float v23; // [sp+54h] [-7Ch]
    float v24; // [sp+58h] [-78h]
    float v25; // [sp+60h] [-70h] BYREF
    float v26; // [sp+64h] [-6Ch]
    float v27; // [sp+68h] [-68h]

    iassert(pPath);
    iassert(pTrim);
    iassert(pActor);
    iassert(vPoint);

    if (pPath->wPathLen <= 0)
        return 0;

    iassert(pTrim->iDelta == -2 || pTrim->iDelta == -1 || pTrim->iDelta == +1);

    i = pTrim->iIndex + pTrim->iDelta;
    iassert(i >= 0);

    if (i >= pPath->wPathLen)
        return 0;

    Actor_GetEyeOffset(pActor, &v22);
    v15 = pPath->wPathLen - 1 == i;
    if (pPath->wPathLen - 1 == i)
    {
        v18 = (float)(pPath->vCurrPoint[1] + v23);
        v25 = v22 + pPath->vCurrPoint[0];
        v17 = pPath->vCurrPoint[2];
        v26 = v18;
    }
    else
    {
        vOrigPoint = pPath->pts[i].vOrigPoint;
        v25 = *vOrigPoint + v22;
        v26 = vOrigPoint[1] + v23;
        v17 = vOrigPoint[2];
    }
    v27 = (float)v17 + v24;
    CanSeePointFrom = Actor_CanSeePointFrom(pActor, &v25, vPoint, fMaxDistSqrd, v14);
    v20 = pTrim->iDelta;
    if (CanSeePointFrom)
    {
        if (v20 == -2 && !v15)
        {
            pTrim->iIndex = i;
            result = 1;
            pTrim->iDelta = 1;
            return result;
        }
        Path_TrimLastNodes(pPath, i, 0);
        return 0;
    }
    if (v20 == 1)
    {
        Path_TrimLastNodes(pPath, pTrim->iIndex, 0);
        return 0;
    }
    else if (i)
    {
        if (v20 != -2)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2109, 0, "%s", "pTrim->iDelta == -2");
        pTrim->iIndex = i;
        if (i == 1)
            pTrim->iDelta = -1;
        return 1;
    }
    else
    {
        Actor_ClearPath(pActor);
        return 0;
    }
}

PredictionTraceResult __cdecl Path_PredictionTraceCheckForEntities(
    float *vStartPos,
    float *vEndPos,
    int *entities,
    int entityCount,
    int entityIgnore,
    int mask,
    float *vTraceEndPos)
{
    int v13; // r28
    gentity_s *v15; // r3
    actor_s *actor; // r11
    PredictionTraceResult result; // r3
    float mins[3]; // [sp+50h] [-80h] BYREF
    float end[3]; // [sp+60h] [-70h] BYREF

    end[0] = vEndPos[0];
    end[1] = vEndPos[1];
    end[2] = vStartPos[2];

    mins[0] = -15.0f;
    mins[1] = -15.0f;
    mins[2] = 0.0f;

    v13 = 0;

    if (entityCount <= 0)
        return (PredictionTraceResult)!Path_PredictionTrace(vStartPos, vEndPos, entityIgnore, mask, vTraceEndPos, 18.0, 1);

    while (1)
    {
        v15 = SV_GentityNum(*entities);
        if (v15 && (actor = v15->actor) != 0 && actor->Physics.prone)
            mins[2] = 10.0f;
        else
            mins[2] = 18.0f;

        if (SV_SightTraceToEntity(vStartPos, mins, (float*)PREDICTION_TRACE_MAX, end, *entities, -1))
            break;
        ++v13;
        ++entities;
        if (v13 >= entityCount)
            return (PredictionTraceResult)!Path_PredictionTrace(vStartPos, vEndPos, entityIgnore, mask, vTraceEndPos, 18.0, 1);
    }
    result = PTR_HIT_ENTITY;
    vTraceEndPos[0] = vStartPos[0];
    vTraceEndPos[1] = vStartPos[1];
    vTraceEndPos[2] = vStartPos[2];
    return result;
}

bool __cdecl Path_LookaheadPredictionTrace(path_t *pPath, float *vStartPos, float *vEndPos)
{
    int mask; // r6
    int wDodgeEntity; // r11
    int checkentities; // [sp+50h] [-20h] BYREF
    float endpos[4]; // [sp+58h] [-18h] BYREF

    mask = 8519697;
    if (pPath->wDodgeCount)
    {
        wDodgeEntity = pPath->wDodgeEntity;
        if (wDodgeEntity != ENTITYNUM_NONE)
        {
            if (level.gentities[wDodgeEntity].actor)
            {
                checkentities = pPath->wDodgeEntity;
                return Path_PredictionTraceCheckForEntities(vStartPos, vEndPos, &checkentities, 1, ENTITYNUM_NONE, 8519697, endpos) == PTR_SUCCESS;
            }
            mask = 42074129;
        }
    }
    return Path_PredictionTrace(vStartPos, vEndPos, ENTITYNUM_NONE, mask, endpos, 18.0, 1);
}

void __cdecl Path_UpdateLookaheadAmount(
    path_t *pPath,
    float *vStartPos,
    float *vLookaheadPos,
    int bReduceLookaheadAmount,
    double dist,
    int lookaheadNextNode,
    double maxLookaheadAmountIfReduce)
{
    int v15; // r11
    int flags; // r10
    double v17; // fp0
    double v18; // fp0
    int v19; // r9
    int v20; // r11
    float fOrigLength; // fp0
    int v22; // r4
    double v23; // fp1
    double v24; // fp0
    int wPathLen; // r11
    float fCurrLength; // fp1
    double v28; // fp2
    const char *v29; // r3

    if (pPath->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2716, 0, "%s", "pPath->wPathLen > 0");
    v15 = pPath->wPathLen - 1;
    if (pPath->lookaheadNextNode >= v15)
    {
        pPath->fLookaheadDistToNextNode = 0.0;
        pPath->lookaheadNextNode = v15;
    }
    if (bReduceLookaheadAmount)
    {
        flags = pPath->flags;
        if ((flags & 2) != 0)
            v17 = 0.75;
        else
            v17 = 0.5625;
        v18 = (float)((float)maxLookaheadAmountIfReduce * (float)v17);
        pPath->fLookaheadAmount = v18;
        if (v18 < 0.001)
            pPath->fLookaheadAmount = 0.001;
        pPath->flags = flags & 0xFFFFFDFC;
    LABEL_28:
        pPath->lookaheadDir[0] = *vLookaheadPos - *vStartPos;
        pPath->lookaheadDir[1] = vLookaheadPos[1] - vStartPos[1];
        pPath->lookaheadDir[2] = vLookaheadPos[2] - vStartPos[2];
        v23 = Vec2Normalize(pPath->lookaheadDir);
        pPath->fLookaheadDist = v23;
        if (v23 == 0.0)
            v24 = 0.0;
        else
            v24 = (float)(pPath->lookaheadDir[2] / (float)v23);
        pPath->lookaheadDir[2] = v24;
        pPath->fLookaheadDistToNextNode = dist;
        pPath->lookaheadNextNode = lookaheadNextNode;

        iassert(pPath->wNegotiationStartNode <= pPath->lookaheadNextNode);
        iassert(pPath->lookaheadNextNode < pPath->wPathLen);
        iassert(pPath->pts[pPath->lookaheadNextNode].fOrigLength > 0);
        iassert(pPath->fLookaheadDistToNextNode <= pPath->pts[pPath->lookaheadNextNode].fOrigLength);

        wPathLen = pPath->wPathLen;
        if (wPathLen > 1)
        {
            fCurrLength = pPath->fCurrLength;
            v28 = *((float *)&pPath->pts[wPathLen - 1] - 2);
            if (fCurrLength > v28)
            {
                v29 = va((const char *)HIDWORD(fCurrLength), LODWORD(fCurrLength), LODWORD(v28));
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    2762,
                    0,
                    "%s\n\t%s",
                    "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                    v29);
            }
        }

        iassert(!pPath->fLookaheadDistToNextNode || (pPath->lookaheadNextNode < pPath->wPathLen - 1));
        iassert(pPath->lookaheadNextNode < pPath->wPathLen);

        if (pPath->fLookaheadDistToNextNode != 0.0 && pPath->lookaheadNextNode >= pPath->wPathLen - 1)
        {
            v22 = 2765;
            goto LABEL_50;
        }
        return;
    }
    if (Path_LookaheadPredictionTrace(pPath, vStartPos, vLookaheadPos))
    {
        Path_IncreaseLookaheadAmount(pPath);
        goto LABEL_28;
    }
    iassert(pPath->lookaheadNextNode >= 0);
    v19 = pPath->wPathLen;
    v20 = pPath->lookaheadNextNode;
    if (v20 == v19 - 2)
        fOrigLength = pPath->fCurrLength;
    else
        fOrigLength = pPath->pts[v20].fOrigLength;
    if ((pPath->flags & 2) == 0 || v20 >= v19 || pPath->fLookaheadDistToNextNode > fOrigLength)
    {
        Path_ReduceLookaheadAmount(pPath, maxLookaheadAmountIfReduce);
        goto LABEL_28;
    }
    Path_ReduceLookaheadAmount(pPath, maxLookaheadAmountIfReduce);
    iassert(pPath->lookaheadNextNode < pPath->wPathLen);
    if (pPath->fLookaheadDistToNextNode != 0.0 && pPath->lookaheadNextNode >= pPath->wPathLen - 1)
    {
        v22 = 2739;
    LABEL_50:
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            v22,
            0,
            "%s",
            "!pPath->fLookaheadDistToNextNode || (pPath->lookaheadNextNode < pPath->wPathLen - 1)");
    }
}

void __cdecl Path_CalcLookahead_Completed(
    path_t *pPath,
    float *vStartPos,
    int bReduceLookaheadAmount,
    double totalArea)
{
    float *vLookaheadPos;

    iassert(pPath);
    iassert(pPath->wNegotiationStartNode < pPath->wPathLen); // blops add

    pPath->flags |= 0x41u;
    if (pPath->wPathLen - 1 == pPath->wNegotiationStartNode)
        vLookaheadPos = pPath->vCurrPoint;
    else
        vLookaheadPos = pPath->pts[pPath->wNegotiationStartNode].vOrigPoint;

    Path_UpdateLookaheadAmount(
        pPath,
        vStartPos,
        vLookaheadPos,
        bReduceLookaheadAmount,
        0.0,
        pPath->wNegotiationStartNode,
        totalArea);

    pPath->fLookaheadAmount = fmaxf(pPath->fLookaheadAmount, fmaxf(totalArea, 32768.0f));
}

void __cdecl Path_CalcLookahead(path_t *pPath, float *vStartPos, int bReduceLookaheadAmount)
{
    int wPathLen; // r11
    float fCurrLength; // fp1
    double v10; // fp2
    const char *v11; // r3
    float fLookaheadAmount; // fp28
    double v13; // fp30
    int v14; // r27
    int i; // r29
    const pathpoint_t *pt; // r31
    double DistToPathSegment; // fp29
    float fLength; // fp31
    double v19; // fp2
    double dist; // fp30
    const float *vOrigPoint; // r11
    double v22; // fp0
    double v23; // fp0
    double v24; // fp13
    float lookaheadPos[16]; // [sp+60h] [-A0h] BYREF

    iassert(pPath);
    iassert(pPath->wPathLen > 0);
    iassert(pPath->wNegotiationStartNode >= 0);
    iassert(pPath->wNegotiationStartNode < pPath->wPathLen);

    wPathLen = pPath->wPathLen;
    if (wPathLen > 1)
    {
        fCurrLength = pPath->fCurrLength;
        v10 = *((float *)&pPath->pts[wPathLen - 1] - 2);
        if (fCurrLength > v10)
        {
            v11 = va((const char *)HIDWORD(fCurrLength), LODWORD(fCurrLength), LODWORD(v10));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3241,
                0,
                "%s\n\t%s",
                "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                v11);
        }
    }
    fLookaheadAmount = pPath->fLookaheadAmount;
    v13 = 0.0;
    if ((COERCE_UNSIGNED_INT(pPath->fLookaheadAmount) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3246,
            0,
            "%s",
            "!IS_NAN(lookaheadAmount)");
    if (fLookaheadAmount <= 0.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3247,
            0,
            "%s\n\t(lookaheadAmount) = %g",
            HIDWORD(fLookaheadAmount),
            LODWORD(fLookaheadAmount));
    v14 = 1;
    i = pPath->wPathLen - 2;
    if (i < pPath->wNegotiationStartNode)
    {
    LABEL_32:
        Path_CalcLookahead_Completed(pPath, vStartPos, bReduceLookaheadAmount, v13);
    }
    else
    {
        pt = &pPath->pts[i];
        while (1)
        {
            iassert(!IS_NAN((pt->vOrigPoint)[0]) && !IS_NAN((pt->vOrigPoint)[1]) && !IS_NAN((pt->vOrigPoint)[2]));
            
            DistToPathSegment = Path_GetDistToPathSegment(vStartPos, pt);
            if (v14)
                fLength = pPath->fCurrLength;
            else
                fLength = pt->fOrigLength;
            iassert(fLength > 0);
            v13 = (float)((float)((float)fLength * (float)DistToPathSegment) + (float)v13);
            if (pPath->minLookAheadNodes == 2)
                PathCalcLookahead_CheckMinLookaheadNodes(pPath, pt, i);
            if (pPath->minLookAheadNodes + i <= pPath->wPathLen - 2 && v13 >= fLookaheadAmount)
                break;
            --i;
            --pt;
            v14 = 0;
            if (i < pPath->wNegotiationStartNode)
                goto LABEL_32;
        }
        v19 = pt->fOrigLength;
        dist = (float)((float)((float)v13 - (float)fLookaheadAmount) / (float)DistToPathSegment);
        if (fLength > v19)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3281,
                0,
                (const char *)HIDWORD(fLength),
                LODWORD(fLength),
                LODWORD(v19));
        if (dist > fLength)
            dist = fLength;
        vOrigPoint = pt[1].vOrigPoint;
        v22 = (float)((float)(pt->fDir2D[1] * (float)-dist) + pt->vOrigPoint[1]);
        lookaheadPos[0] = (float)((float)-dist * pt->fDir2D[0]) + pt->vOrigPoint[0];
        lookaheadPos[1] = v22;
        if (v14)
            vOrigPoint = pPath->vCurrPoint;
        v23 = pt->vOrigPoint[2];
        v24 = (float)(pt->vOrigPoint[2] - vOrigPoint[2]);
        pPath->flags &= 0xFFFFFFBE;
        lookaheadPos[2] = -(float)((float)((float)((float)v24 / (float)fLength) * (float)dist) - (float)v23);
        Path_UpdateLookaheadAmount(pPath, vStartPos, lookaheadPos, bReduceLookaheadAmount, dist, i, fLookaheadAmount);
    }
}

void __cdecl Path_CheckNodeCountForDodge(path_t *pPath, int numNeeded, pathpoint_t **pt, int *startIndex)
{
    int v7; // r11
    int v8; // r31

    v7 = *startIndex + numNeeded;
    v8 = v7 - 31;
    if (v7 - 31 > 0)
    {
        Path_TrimLastNodes(pPath, v7 - 31, 0);
        *pt -= v8;
        *startIndex -= v8;
        if ((unsigned __int16)pPath->wNegotiationStartNode >= 0x8000u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3743,
                0,
                "%s",
                "pPath->wNegotiationStartNode >= 0");
        if (pPath->wNegotiationStartNode > *startIndex)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3744,
                0,
                "%s",
                "pPath->wNegotiationStartNode <= *startIndex");
    }
}

void __cdecl Path_TrimToBadPlaceLink(path_t *pPath, team_t eTeam)
{
    int v4; // r30
    int *i; // r31
    int v6; // r4

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 4092, 0, "%s", "pPath");
    v4 = pPath->wPathLen - 2;
    if (v4 >= 0)
    {
        for (i = &pPath->pts[v4 + 1].iNodeNum; ; i -= 7)
        {
            v6 = *(i - 7);
            if (v6 >= 0 && *i >= 0 && Path_IsBadPlaceLink(*i, v6, eTeam))
                break;
            if (--v4 < 0)
                return;
        }
        Path_TrimLastNodes(pPath, v4, 1);
    }
}

int __cdecl Path_FindPath(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    bool bAllowNegotiationLinks)
{
    pathnode_t *v10; // r30
    int *v11; // r6
    pathnode_t *v13; // r5
    _BYTE v14[16]; // [sp+50h] [-350h] BYREF
    pathsort_t v15[64]; // [sp+60h] [-340h] BYREF

    v10 = Path_NearestNode(vGoalPos, v15, -2, 192.0, (int *)vGoalPos, (int)v14, (nearestNodeHeightCheck)64);
    if (v10 && (v13 = Path_NearestNode(vStartPos, v15, -2, 192.0, v11, (int)v14, (nearestNodeHeightCheck)64)) != 0)
        return Path_FindPathFromTo(pPath, eTeam, v13, vStartPos, v10, vGoalPos, bAllowNegotiationLinks);
    else
        return 0;
}

pathnode_t *__cdecl Path_FindPathFrom(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    const float *vGoalPos,
    bool bAllowNegotiationLinks)
{
    pathnode_t *result; // r3
    _BYTE v13[16]; // [sp+50h] [-350h] BYREF
    pathsort_t v14[69]; // [sp+60h] [-340h] BYREF

    result = Path_NearestNode(vGoalPos, v14, -2, 192.0, (int *)vStartPos, (int)v13, (nearestNodeHeightCheck)64);
    if (result)
        return (pathnode_t *)Path_FindPathFromTo(
            pPath,
            eTeam,
            pNodeFrom,
            vStartPos,
            result,
            vGoalPos,
            bAllowNegotiationLinks);
    return result;
}

void __cdecl Path_UpdateLookahead(
    path_t *pPath,
    float *vStartPos,
    bool bReduceLookaheadAmount,
    bool bTrimAmount,
    bool bAllowBacktrack)
{
    int v10; // r7
    int v11; // r6
    int wPathLen; // r11
    float fCurrLength; // fp1
    double v14; // fp2
    const char *v15; // r3
    int wNegotiationStartNode; // r10
    int v17; // r11
    char v18; // r3
    unsigned int v19; // r11
    int v20; // r7
    int v21; // r6
    double v22; // fp1
    double v23; // fp0
    int flags; // r11
    __int16 v25; // r10

    //Profile_Begin(233);
    if ((unsigned __int16)pPath->wNegotiationStartNode >= 0x8000u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3529,
            0,
            "%s",
            "pPath->wNegotiationStartNode >= 0");
    if (pPath->wPathLen <= pPath->wNegotiationStartNode)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3530,
            0,
            "%s",
            "pPath->wPathLen > pPath->wNegotiationStartNode");
    wPathLen = pPath->wPathLen;
    if (wPathLen > 1)
    {
        fCurrLength = pPath->fCurrLength;
        v14 = *((float *)&pPath->pts[wPathLen - 1] - 2);
        if (fCurrLength > v14)
        {
            v15 = va((const char *)HIDWORD(fCurrLength), LODWORD(fCurrLength), LODWORD(v14));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3531,
                0,
                "%s\n\t%s",
                "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                v15);
        }
    }
    wNegotiationStartNode = pPath->wNegotiationStartNode;
    v17 = pPath->wPathLen - 1;
    pPath->flags &= ~0x20u;
    if (wNegotiationStartNode == v17)
    {
        pPath->lookaheadDir[0] = pPath->vCurrPoint[0] - *vStartPos;
        pPath->lookaheadDir[1] = pPath->vCurrPoint[1] - vStartPos[1];
        pPath->lookaheadDir[2] = pPath->vCurrPoint[2] - vStartPos[2];
        v22 = Vec2Normalize(pPath->lookaheadDir);
        pPath->fLookaheadDist = v22;
        if (v22 == 0.0)
            v23 = 0.0;
        else
            v23 = (float)(pPath->lookaheadDir[2] / (float)v22);
        flags = pPath->flags;
        pPath->lookaheadDir[2] = v23;
        v25 = pPath->wNegotiationStartNode;
        pPath->fLookaheadDistToNextNode = 0.0;
        pPath->lookaheadNextNode = v25;
        pPath->flags = flags | 0x41;
        goto LABEL_25;
    }
    if ((float)((float)(pPath->lookaheadDir[1] * (float)(pPath->vCurrPoint[1] - vStartPos[1]))
        + (float)((float)(pPath->vCurrPoint[0] - *vStartPos) * pPath->lookaheadDir[0])) > 0.0)
    {
        if (bAllowBacktrack && pPath->fLookaheadAmount >= 64.0)
            Path_BacktrackCompletedPath(pPath, vStartPos);
    LABEL_20:
        Path_CalcLookahead(pPath, vStartPos, bReduceLookaheadAmount);
        goto LABEL_25;
    }
    v18 = Path_NeedsReevaluation(pPath);
    v19 = pPath->flags;
    if (v18)
    {
        Path_FindPath(pPath, pPath->eTeam, vStartPos, pPath->vFinalGoal, (v19 >> 4) & 1);
        //Profile_EndInternal(0);
        return;
    }
    if ((v19 & 2) != 0 || pPath->fLookaheadAmount <= 16384.0)
        Path_TrimCompletedPath(pPath, vStartPos);
    if (!bTrimAmount)
        goto LABEL_20;
    Path_SubtractTrimmedAmount(pPath, vStartPos);
    Path_CalcLookahead(pPath, vStartPos, bReduceLookaheadAmount);
LABEL_25:
    Path_UpdateForwardLookahead(pPath, vStartPos);
    //Profile_EndInternal(0);
}

void __cdecl Path_SetLookaheadToStart(path_t *pPath, float *vStartPos, int bTrimAmount)
{
    float *lookaheadDir; // r3
    double v7; // fp0
    double v8; // fp1
    double v9; // fp0
    __int16 wPathLen; // r11

    v7 = (float)(pPath->vCurrPoint[0] - *vStartPos);
    lookaheadDir = pPath->lookaheadDir;
    *lookaheadDir = v7;
    lookaheadDir[1] = pPath->vCurrPoint[1] - vStartPos[1];
    lookaheadDir[2] = pPath->vCurrPoint[2] - vStartPos[2];
    v8 = Vec2Normalize(lookaheadDir);
    pPath->fLookaheadDist = v8;
    if (v8 == 0.0)
        v9 = 0.0;
    else
        v9 = (float)(pPath->lookaheadDir[2] / (float)v8);
    wPathLen = pPath->wPathLen;
    pPath->lookaheadDir[2] = v9;
    pPath->fLookaheadDistToNextNode = 0.0;
    pPath->lookaheadNextNode = wPathLen - 1;
    Path_UpdateLookahead(pPath, vStartPos, 0, bTrimAmount, 1);
}

void __cdecl Path_TransferLookahead(path_t *pPath, float *vStartPos)
{
    int wPathLen; // r11
    float fCurrLength; // fp1
    float amount; // fp21
    double totalArea; // fp23
    float vDir[2]; // v10, v11
    double fLength; // fp1
    double closestTotalArea; // fp22
    int i; // r29
    int bAtStart; // r10
    double bestForwardDot; // fp24
    double prevDot; // fp25
    float *fDir2D; // r31
    float vStartDir[3];

    iassert(pPath);
    iassert(pPath->wOrigPathLen == pPath->wPathLen);
    iassert(pPath->wNegotiationStartNode >= 0);
    iassert(pPath->wNegotiationStartNode < pPath->wPathLen);
    iassert(pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength);

    if (pPath->fLookaheadDist == 0.0)
    {
        Path_SetLookaheadToStart(pPath, vStartPos, 0);
        return;
    }

    amount = pPath->fLookaheadAmount;
    totalArea = 0.0;

    iassert(amount);

    vDir[0] = pPath->lookaheadDir[0];
    vDir[1] = pPath->lookaheadDir[1];

    iassert(vDir[0] || vDir[1]);

    vStartDir[0] = pPath->vCurrPoint[0] - vStartPos[0];
    vStartDir[1] = pPath->vCurrPoint[1] - vStartPos[1];

    fLength = Vec2Normalize(vStartDir);
    closestTotalArea = 0.0;
    i = pPath->wPathLen - 2;
    bAtStart = 1;

    prevDot = ((vDir[0] * vStartDir[1]) - (vDir[1] * vStartDir[0])) * fLength;
    bestForwardDot = (vDir[0] * vStartDir[0]) + (vDir[1] * vStartDir[1]);

    if (i < pPath->wNegotiationStartNode)
    {
        Path_SetLookaheadToStart(pPath, vStartPos, 0);
        return;
    }

    fDir2D = pPath->pts[i].fDir2D;

    float offset[2]; // v33, v34

    i = pPath->wPathLen - 2;

    while (i >= pPath->wNegotiationStartNode)
    {
        pathpoint_t *pt = &pPath->pts[i];
        offset[0] = pt->vOrigPoint[0] - vStartPos[0];
        offset[1] = pt->vOrigPoint[1] - vStartPos[1];

        if (bAtStart)
            fCurrLength = pPath->fCurrLength;
        else
            fCurrLength = fDir2D[2];

        fLength = fCurrLength;

        iassert(pt->fDir2D[0] || pt->fDir2D[1]);

        float height = (pt->fDir2D[0] * offset[1]) - (pt->fDir2D[1] * offset[0]);
        bool bInFront = (((pt->fDir2D[0] * vDir[1]) - (pt->fDir2D[1] * vDir[0])) * height) > 0.0;
        height = I_fabs(height);

        float dot = (vDir[0] * offset[1]) - (vDir[1] * offset[0]);

        if (bInFront && (float)(dot * prevDot) < 0.0)
        {
            Path_UpdateLookahead(pPath, vStartPos, 0, 1, 1);
            return;
        }

        iassert(!IS_NAN(fLength));
        iassert(fLength > 0);

        totalArea = ((height * fLength) + totalArea);

        if (totalArea >= amount)
        {
            iassert(!IS_NAN(height));
            iassert(height);
            iassert(!IS_NAN(fLength));
            iassert(fLength > 0);
            
            float dist = (totalArea - amount) / height;

            iassert(pt->fDir2D[0] || pt->fDir2D[1]);

            offset[0] = (-dist * pt->fDir2D[0]) + pt->vOrigPoint[0];
            offset[1] = (-dist * pt->fDir2D[1]) + pt->vOrigPoint[1];
            Vec2Normalize(offset);

            float forwardDot = (vDir[0] * offset[0]) + (vDir[1] * offset[1]);
            if (bestForwardDot > forwardDot)
            {
                Path_SetLookaheadToStart(pPath, vStartPos, 1);
                if ((pPath->flags & 2) == 0 && pPath->fLookaheadAmount > closestTotalArea)
                {
                    if (closestTotalArea < 64.0)
                        closestTotalArea = 64.0f;
                    pPath->fLookaheadAmount = closestTotalArea;
                }
                return;
            }

            Path_SetLookaheadToStart(pPath, vStartPos, 0);
            return;
        }

        prevDot = dot;
        Vec2Normalize(offset);
        float forwardDot = (vDir[0] * offset[0]) + (vDir[1] * offset[1]);
        if (forwardDot >= bestForwardDot)
        {
            bestForwardDot = forwardDot;
            closestTotalArea = totalArea;
        }
        --i;
        bAtStart = 0;
    }

    if (closestTotalArea == totalArea)
    {
        Path_SetLookaheadToStart(pPath, vStartPos, 0);
        return;
    }

    Path_SetLookaheadToStart(pPath, vStartPos, 1);
    if ((pPath->flags & 2) == 0 && pPath->fLookaheadAmount > closestTotalArea)
    {
        if (closestTotalArea < 64.0)
            closestTotalArea = 64.0f;
        pPath->fLookaheadAmount = closestTotalArea;
    }
}

int __cdecl Path_GeneratePath(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    float *vGoalPos,
    pathnode_t *pNodeFrom,
    pathnode_t *pNodeTo,
    bool bIncludeGoalPos,
    bool bAllowNegotiationLinks)
{
    int v16; // r25
    int wPathLen; // r11
    int flags; // r20
    double v19; // fp0
    int v20; // r24
    pathnode_t *pParent; // r11
    pathnode_t *v22; // r10
    bool v23; // zf
    int v24; // r27
    int v26; // r28
    float *v27; // r29
    int wNegotiationStartNode; // r11
    pathnode_t *v29; // r30
    pathpoint_t *v30; // r30
    float *fDir2D; // r29
    int v32; // r28
    float fOrigLength; // fp0
    float fLookaheadAmount; // fp0
    double v35; // fp0
    int v36; // r11
    float fCurrLength; // fp1
    double v38; // fp2
    const char *v39; // r3
    int *v40; // r11

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 454, 0, "%s", "pPath");
    if (!pNodeFrom)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 455, 0, "%s", "pNodeFrom");
    if (!pNodeTo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 456, 0, "%s", "pNodeTo");
    if (!vGoalPos && bIncludeGoalPos)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            457,
            0,
            "%s",
            "vGoalPos || !bIncludeGoalPos");
    Path_AddTrimmedAmount(pPath, vStartPos);
    v16 = 0;
    if (!bIncludeGoalPos)
    {
        pPath->pts[0].vOrigPoint[0] = pNodeTo->constant.vOrigin[0];
        pPath->pts[0].vOrigPoint[1] = pNodeTo->constant.vOrigin[1];
        pPath->pts[0].vOrigPoint[2] = pNodeTo->constant.vOrigin[2];
    LABEL_17:
        pPath->pts[0].iNodeNum = Path_ConvertNodeToIndex(pNodeTo);
        goto LABEL_18;
    }
    if (!vGoalPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 471, 0, "%s", "vGoalPos");
    pPath->pts[0].vOrigPoint[0] = *vGoalPos;
    pPath->pts[0].vOrigPoint[1] = vGoalPos[1];
    pPath->pts[0].vOrigPoint[2] = vGoalPos[2];
    if (*vGoalPos == pNodeTo->constant.vOrigin[0] && vGoalPos[1] == pNodeTo->constant.vOrigin[1])
        goto LABEL_17;
    v16 = 1;
    pPath->pts[0].iNodeNum = -1;
LABEL_18:
    wPathLen = (unsigned __int16)pPath->wPathLen;
    flags = pPath->flags;
    pPath->flags = 0;
    if (!wPathLen
        || pPath->pts[0].vOrigPoint[0] != pPath->vFinalGoal[0]
        || pPath->pts[0].vOrigPoint[1] != pPath->vFinalGoal[1]
        || pPath->pts[0].vOrigPoint[2] != pPath->vFinalGoal[2])
    {
        v19 = pPath->pts[0].vOrigPoint[0];
        if (v19 != pPath->vFinalGoal[0] || pPath->pts[0].vOrigPoint[1] != pPath->vFinalGoal[1])
            pPath->iPathEndTime = 0;
        pPath->vFinalGoal[0] = v19;
        pPath->vFinalGoal[1] = pPath->pts[0].vOrigPoint[1];
        pPath->vFinalGoal[2] = pPath->pts[0].vOrigPoint[2];
    }
    Path_Clear(pPath);
    v20 = 0;
    pPath->wDodgeCount = 0;
    if (pNodeTo)
    {
        pParent = pNodeTo->transient.pParent;
        ++v16;
        if (pNodeTo != pNodeFrom)
        {
            do
            {
                if (pParent->constant.type == NODE_NEGOTIATION_BEGIN
                    && pNodeTo->constant.type == NODE_NEGOTIATION_END
                    && pParent->constant.target == pNodeTo->constant.targetname)
                {
                    if (!bAllowNegotiationLinks)
                        return 0;
                    v20 = v16;
                }
                v22 = pParent->transient.pParent;
                ++v16;
                pParent->transient.pParent = pNodeTo;
                pNodeTo = pParent;
                v23 = pParent != pNodeFrom;
                pParent = v22;
            } while (v23);
        }
    }
    if (v16 <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 540, 0, "%s", "iTotal > 0");
    v24 = v16 - 32;
    if (v16 - 32 <= 0)
    {
        if (v16 <= 0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 552, 0, "%s", "iTotal > 0");
    }
    else
    {
        v20 -= v24;
        v16 = 32;
        pPath->flags |= 4u;
        if (v20 < 0)
        {
            //LOWORD(v20) = 0;
            v20 = 0;
        }
    }
    v26 = v16 - 1;
    if (v16 - 1 > 0)
    {
        v27 = &pPath->pts[v26].vOrigPoint[2];
        do
        {
            *(v27 - 2) = pNodeFrom->constant.vOrigin[0];
            *(v27 - 1) = pNodeFrom->constant.vOrigin[1];
            *v27 = pNodeFrom->constant.vOrigin[2];
            --v26;
            *((unsigned int *)v27 + 4) = Path_ConvertNodeToIndex(pNodeFrom);
            pNodeFrom = pNodeFrom->transient.pParent;
            v27 -= 7;
        } while (v26 > 0);
    }
    if (v24 > 0)
    {
        pPath->pts[0].vOrigPoint[0] = pNodeFrom->constant.vOrigin[0];
        pPath->pts[0].vOrigPoint[1] = pNodeFrom->constant.vOrigin[1];
        pPath->pts[0].vOrigPoint[2] = pNodeFrom->constant.vOrigin[2];
        pPath->pts[0].iNodeNum = Path_ConvertNodeToIndex(pNodeFrom);
    }
    if (pPath->wNegotiationStartNode)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            566,
            0,
            "%s",
            "pPath->wNegotiationStartNode == 0");
    pPath->wNegotiationStartNode = v20;
    if ((v20 & 0x8000u) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            568,
            0,
            "%s",
            "pPath->wNegotiationStartNode >= 0");
    wNegotiationStartNode = pPath->wNegotiationStartNode;
    if (wNegotiationStartNode > 0)
    {
        v29 = Path_ConvertIndexToNode(pPath->pts[wNegotiationStartNode].iNodeNum);
        if (!v29)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 92, 0, "%s", "negotiationNode");
        ++v29->dynamic.userCount;
    }
    v30 = &pPath->pts[v16];
    v30[-1].fOrigLength = 0.0;
    v30[-1].fDir2D[0] = 0.0;
    v30[-1].fDir2D[1] = 0.0;
    pPath->vCurrPoint[0] = v30[-1].vOrigPoint[0];
    pPath->vCurrPoint[1] = v30[-1].vOrigPoint[1];
    pPath->vCurrPoint[2] = v30[-1].vOrigPoint[2];
    if (v16 - 1 > 0)
    {
        fDir2D = pPath->pts[0].fDir2D;
        v32 = v16 - 1;
        do
        {
            --v32;
            fDir2D[2] = Path_GetPathDir(fDir2D, fDir2D + 4, fDir2D - 3);
            fDir2D += 7;
        } while (v32);
    }
    if (v16 <= 1)
        fOrigLength = 0.0;
    else
        fOrigLength = v30[-2].fOrigLength;
    pPath->fCurrLength = fOrigLength;
    if (v16 <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 585, 0, "%s", "iTotal > 0");
    if (v16 > 32)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            586,
            0,
            "%s",
            "iTotal <= PATH_MAX_POINTS");
    pPath->wPathLen = v16;
    pPath->wOrigPathLen = v16;
    if (bAllowNegotiationLinks)
        pPath->flags |= 0x10u;
    pPath->eTeam = eTeam;
    fLookaheadAmount = pPath->fLookaheadAmount;
    pPath->iPathTime = level.time;
    if (fLookaheadAmount != 0.0)
    {
        if ((flags & 0x180) != 0)
        {
            if ((flags & 0x80) != 0)
            {
                pPath->minLookAheadNodes = 0;
                v35 = 32768.0;
            }
            else
            {
                pPath->minLookAheadNodes = 2;
                v35 = 4096.0;
            }
            pPath->fLookaheadAmount = v35;
            pPath->lookaheadDir[0] = 0.0;
            pPath->lookaheadDir[1] = 0.0;
            pPath->lookaheadDir[2] = 0.0;
            Path_UpdateLookahead(pPath, vStartPos, 0, 0, 1);
            pPath->minLookAheadNodes = 0;
        }
        else
        {
            Path_TransferLookahead(pPath, vStartPos);
        }
        if (pPath->wNegotiationStartNode > pPath->lookaheadNextNode)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                620,
                0,
                "%s",
                "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode");
        if (pPath->lookaheadNextNode >= pPath->wPathLen)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                621,
                0,
                "%s",
                "pPath->lookaheadNextNode < pPath->wPathLen");
        if (pPath->fLookaheadDistToNextNode > (double)pPath->pts[pPath->lookaheadNextNode].fOrigLength)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                622,
                0,
                "%s",
                "pPath->fLookaheadDistToNextNode <= pPath->pts[pPath->lookaheadNextNode].fOrigLength");
        v36 = pPath->wPathLen;
        if (v36 > 1)
        {
            fCurrLength = pPath->fCurrLength;
            v38 = *((float *)&pPath->pts[v36 - 1] - 2);
            if (fCurrLength > v38)
            {
                v39 = va((const char *)HIDWORD(fCurrLength), LODWORD(fCurrLength), LODWORD(v38));
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    623,
                    0,
                    "%s\n\t%s",
                    "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                    v39);
            }
        }
        if (pPath->wNegotiationStartNode)
        {
            v40 = (int *)&pPath->pts[pPath->wNegotiationStartNode];
            if (v40[6] < 0 || *(v40 - 1) < 0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    624,
                    0,
                    "%s",
                    "!pPath->wNegotiationStartNode || (pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0 && pPath->pts[pPath->"
                    "wNegotiationStartNode - 1].iNodeNum >= 0)");
        }
        if (pPath->fLookaheadDistToNextNode != 0.0 && pPath->lookaheadNextNode >= pPath->wPathLen - 1)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                625,
                0,
                "%s",
                "!pPath->fLookaheadDistToNextNode || (pPath->lookaheadNextNode < pPath->wPathLen - 1)");
    }
    return 1;
}

void __cdecl Path_UpdateLookahead_NonCodeMove(path_t *pPath, const float *vPrevPos, float *vStartPos)
{
    double v6; // fp13
    int wNegotiationStartNode; // r9
    int v8; // r11
    float *vOrigPoint; // r10

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3477, 0, "%s", "pPath");
    if (!vStartPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3478, 0, "%s", "vStartPos");
    if (!vPrevPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3479, 0, "%s", "vPrevPos");
    v6 = (float)(vStartPos[1] - vPrevPos[1]);
    if ((float)((float)((float)(*vStartPos - *vPrevPos) * (float)(*vStartPos - *vPrevPos))
        + (float)((float)v6 * (float)v6)) != 0.0)
    {
        wNegotiationStartNode = pPath->wNegotiationStartNode;
        v8 = pPath->wPathLen - 2;
        if (v8 >= wNegotiationStartNode)
        {
            vOrigPoint = pPath->pts[v8].vOrigPoint;
            while ((float)((float)((float)(*vOrigPoint - *vPrevPos) * (float)(*vStartPos - *vPrevPos))
                + (float)((float)(vOrigPoint[1] - vPrevPos[1]) * (float)(vStartPos[1] - vPrevPos[1]))) < 0.0)
            {
                --v8;
                vOrigPoint -= 7;
                if (v8 < pPath->wNegotiationStartNode)
                    goto LABEL_16;
            }
            if ((float)((float)((float)(*vOrigPoint - *vStartPos) * (float)(*vStartPos - *vPrevPos))
                + (float)((float)(vOrigPoint[1] - vStartPos[1]) * (float)(vStartPos[1] - vPrevPos[1]))) <= 0.0
                && v8 > wNegotiationStartNode)
            {
                pPath->vCurrPoint[0] = *vOrigPoint;
                pPath->vCurrPoint[1] = vOrigPoint[1];
                pPath->vCurrPoint[2] = vOrigPoint[2];
                pPath->fCurrLength = pPath->pts[v8 - 1].fOrigLength;
                Path_RemoveCompletedPathPoints(pPath, v8 - 1);
            }
        }
    }
LABEL_16:

    // aislop
    //_FP12 = (float)(pPath->fLookaheadAmount - (float)1024.0);
    //__asm { fsel      f0, f12, f0, f13 }
    //pPath->fLookaheadAmount = _FP0;

    {
        float lookaheadAmount = pPath->fLookaheadAmount - 1024.0f;
        pPath->fLookaheadAmount = fmaxf(lookaheadAmount, 0.0f);
    }


    Path_UpdateLookahead(pPath, vStartPos, 0, 0, 0);
}

int __cdecl Path_AttemptDodge(
    path_t *pPath,
    float *vOrg,
    float *vDodgeStart,
    float *vDodgeEnd,
    int startIndex,
    int *entities,
    int entityCount,
    int entityIgnore,
    int mask,
    int bCheckLookahead)
{
    int maxIndex; // r27
    pathpoint_t *point; // r31
    __int16 lookaheadNextNode; // r11
    unsigned int wNegotiationStartNode; // r10
    PredictionTraceResult result; // r3
    float dist; // fp31
    pathpoint_t *pt; // [sp+50h] [-100h] BYREF
    float vDelta[2]; // [sp+58h] [-F8h] BYREF
    float vNewDodgeEnd[3]; // [sp+60h] [-F0h] BYREF
    float vNewDodgeStart[3]; // [sp+70h] [-E0h] BYREF
    float vEnd[3]; // [sp+80h] [-D0h] BYREF
    float vTraceEndPos[3]; // [sp+90h] [-C0h] BYREF
    float pathDir;

    //a20 = startIndex;
    iassert(pPath);
    iassert(pPath->wPathLen > 0);
    iassert(!(vDodgeStart[0] == vDodgeEnd[0] && vDodgeStart[1] == vDodgeEnd[1]));
    iassert(startIndex <= pPath->lookaheadNextNode);
    iassert(pPath->lookaheadNextNode < pPath->wPathLen);
    
    //v38 = a28;
    Path_PredictionTraceCheckForEntities(vOrg, vDodgeStart, entities, entityCount, entityIgnore, mask, vNewDodgeStart);
    Path_PredictionTraceCheckForEntities(vNewDodgeStart, vDodgeEnd, entities, entityCount, entityIgnore, mask, vNewDodgeEnd);

    pt = &pPath->pts[startIndex];

    if (ai_showDodge->current.enabled)
    {
        G_DebugLineWithDuration(vDodgeStart, vNewDodgeStart, colorYellow, 0, 25);
        G_DebugLineWithDuration(vNewDodgeStart, vNewDodgeEnd, colorYellow, 0, 25);
        G_DebugLineWithDuration(vNewDodgeEnd, pPath->pts[startIndex].vOrigPoint, colorYellow, 0, 25);
    }

    if (startIndex < pPath->wNegotiationStartNode)
        goto LABEL_103;

    if (Path_PredictionTraceCheckForEntities(
        vNewDodgeEnd,
        pt->vOrigPoint,
        entities,
        entityCount,
        entityIgnore,
        mask,
        vTraceEndPos))
    {
        maxIndex = pPath->wPathLen - 2;
        if (pPath->wDodgeCount > 0)
            maxIndex -= pPath->wDodgeCount;

        if (startIndex >= maxIndex)
            goto LABEL_103;

        point = &pPath->pts[++startIndex + 1];
        do
        {
            ++point;
            ++startIndex;
        } while (Vec2DistanceSq(point->vOrigPoint, vOrg) >= 1406.25 && startIndex < maxIndex); // KISAKTODO: check startIndex here

        //a20 = startingIndex;
        //v44 = ai_showDodge->current.color[0];

        pt = &pPath->pts[startIndex];

        if (ai_showDodge->current.enabled)
            G_DebugLineWithDuration(vNewDodgeEnd, pPath->pts[startIndex].vOrigPoint, colorMagenta, 0, 25);

        if (Path_PredictionTraceCheckForEntities(
            vNewDodgeEnd,
            pt->vOrigPoint,
            entities,
            entityCount,
            entityIgnore,
            mask,
            vTraceEndPos))
        {
        LABEL_103: // fail_2
            if (bCheckLookahead)
            {
                vDelta[0] = vNewDodgeEnd[0] - vNewDodgeStart[0];
                vDelta[1] = vNewDodgeEnd[1] = vNewDodgeStart[1];

                if (Vec2Normalize(vDelta) > 15.0f)
                {
                    vNewDodgeEnd[0] = (float)(-15.0 * vDelta[0]) + vNewDodgeEnd[0];
                    vNewDodgeEnd[1] = (float)(-15.0 * vDelta[1]) + vNewDodgeEnd[1];
                }
                else
                {
                    vNewDodgeEnd[0] = vNewDodgeStart[0];
                    vNewDodgeEnd[1] = vNewDodgeStart[1];
                    vNewDodgeEnd[2] = vNewDodgeStart[2];
                }

                lookaheadNextNode = pPath->lookaheadNextNode;
                wNegotiationStartNode = (unsigned __int16)pPath->wNegotiationStartNode;

                //v48 = lookaheadNextNode;
                //a20 = lookaheadNextNode;
                iassert(pPath->wNegotiationStartNode >= 0);
                iassert(pPath->wNegotiationStartNode <= startIndex);
                iassert(startIndex < pPath->wPathLen - 1);

                vEnd[0] = (float)(pPath->lookaheadDir[0] * pPath->fLookaheadDist) + vOrg[0];
                vEnd[1] = (float)((float)(pPath->lookaheadDir[1] * pPath->fLookaheadDist) + vOrg[1]);
                vEnd[2] = pPath->pts[startIndex].vOrigPoint[2];

                pt = &pPath->pts[startIndex];

                if (ai_showDodge->current.enabled)
                    G_DebugLineWithDuration(vNewDodgeEnd, vEnd, colorRed, 0, 25);

                result = Path_PredictionTraceCheckForEntities(vNewDodgeEnd, vEnd, entities, entityCount, entityIgnore, mask, vTraceEndPos);

                if (result != PTR_HIT_WORLD)
                {
                    if (result == PTR_HIT_ENTITY)
                    {
                        vEnd[0] = (float)((float)((float)((float)(pPath->lookaheadDir[1] * (float)(vNewDodgeEnd[1] - vOrg[1]))
                            + (float)(pPath->lookaheadDir[0] * (float)(vNewDodgeEnd[0] - vOrg[0])))
                            + (float)15.0)
                            * pPath->lookaheadDir[0])
                            + *vOrg;
                        vEnd[1] = (float)((float)(pPath->lookaheadDir[1]
                            * (float)((float)((float)(pPath->lookaheadDir[1] * (float)(vNewDodgeEnd[1] - vOrg[1]))
                                + (float)(pPath->lookaheadDir[0] * (float)(vNewDodgeEnd[0] - *vOrg)))
                                + (float)15.0))
                            + vOrg[1]);

                        if (ai_showDodge->current.enabled)
                            G_DebugLineWithDuration(vNewDodgeEnd, vEnd, colorCyan, 0, 25);

                        if (Path_PredictionTraceCheckForEntities(vNewDodgeEnd, vEnd, entities, entityCount, entityIgnore, mask, vTraceEndPos) == PTR_SUCCESS)
                        {
                            iassert(pPath->wNegotiationStartNode <= startIndex);

                            if (ai_showDodge->current.enabled)
                                G_DebugLineWithDuration(vEnd, pPath->pts[startIndex].vOrigPoint, colorCyan, 0, 25);

                            if (Path_PredictionTraceCheckForEntities(
                                vEnd,
                                pPath->pts[startIndex].vOrigPoint,
                                entities,
                                entityCount,
                                entityIgnore,
                                mask,
                                vTraceEndPos) == PTR_SUCCESS)
                            {
                                Path_CheckNodeCountForDodge(pPath, 3, &pt, &startIndex);
                                //v52 = pt[0];
                                pt->fOrigLength = Path_GetPathDir(pt->fDir2D, vEnd, pt->vOrigPoint);
                                //v53 = (unsigned int)pt[0];
                                //v52->fOrigLength = *(float *)pt;
                                iassert(!IS_NAN(pt->fOrigLength));

                                pt++;
                                startIndex++;
                                //v54 = (path_t *)&v52[1];
                                //v55 = a20;
                                //v54->pts[0].vOrigPoint[0] = vEnd;
                                //v56 = v55 + 1;
                                //v54->pts[0].vOrigPoint[1] = v89;
                                //v54->pts[0].vOrigPoint[2] = v90;
                                //v54->pts[0].iNodeNum = -1;
                                pt->vOrigPoint[0] = vEnd[0];
                                pt->vOrigPoint[1] = vEnd[1];
                                pt->vOrigPoint[2] = vEnd[2];
                                pt->iNodeNum = -1;
                                //Path_DodgeDrawRaisedLine(&v54[-1].fCurrLength, (float *)v54, colorBlue);
                                Path_DodgeDrawRaisedLine(pt[-1].vOrigPoint, pt->vOrigPoint, colorBlue);
                                goto done;
                            }
                        }
                    }
                    else
                    {
                        if (ai_showDodge->current.enabled)
                            G_DebugLineWithDuration(vEnd, pPath->pts[startIndex].vOrigPoint, colorMdGrey, 0, 25);

                        if (Path_PredictionTraceCheckForEntities(
                            vEnd,
                            pt->vOrigPoint,
                            entities,
                            entityCount,
                            entityIgnore,
                            mask,
                            vTraceEndPos) == PTR_SUCCESS)
                        {
                            Path_CheckNodeCountForDodge(pPath, 3, &pt, &startIndex);
                            dist = pPath->fLookaheadDistToNextNode;
                            iassert(dist > 0);
                            //v58 = pt;
                            //pt[0] = (pathpoint_t *)LODWORD(pt[0]->fOrigLength);
                            iassert(!IS_NAN(pt->fOrigLength));
                            iassert(pt->fOrigLength > 0);
                            iassert(pt->fDir2D[0] || pt->fDir2D[1]);
                            iassert(dist <= pt->fOrigLength);

                            //v54 = (path_t *)&v58[1];
                            //v58[1].vOrigPoint[0] = -(float)((float)(v58->fDir2D[0] * (float)dist) - v58->vOrigPoint[0]);
                            //v58[1].vOrigPoint[1] = -(float)((float)(v58->fDir2D[1] * (float)dist) - v58->vOrigPoint[1]);
                            //v59 = v58->vOrigPoint[2];
                            //v60 = (float)(v58->vOrigPoint[2] - v58[1].vOrigPoint[2]);
                            //fOrigLength = v58->fOrigLength;

                            pt[1].vOrigPoint[0] = pt->vOrigPoint[0] - (float)(pt->fDir2D[0] * dist);
                            pt[1].vOrigPoint[1] = pt->vOrigPoint[1] - (float)(pt->fDir2D[1] * dist);
                            pt[1].vOrigPoint[2] = pt->vOrigPoint[2] - (float)((float)((float)(pt->vOrigPoint[2] - pt[1].vOrigPoint[2]) * dist) / pt->fOrigLength);

                            //*(float *)pt = dist;
                            pt->fOrigLength = dist;
                            //v62 = ((int)pt[0] & 0x7F800000) == 2139095040;
                            //v58[1].vOrigPoint[2] = -(float)((float)((float)((float)v60 / (float)fOrigLength)
                            //    * (float)dist)
                            //    - (float)v59);
                            //v58->fOrigLength = dist;
                            iassert(!IS_NAN(pt->fOrigLength));
                            iassert(pt->fOrigLength > 0);

                            pt++;
                            //v63 = a20;
                            //vOrigPoint = v58[1].vOrigPoint;
                            //v65 = (float *)v58;
                            pPath->lookaheadNextNode = ++startIndex;
                            pPath->fLookaheadDistToNextNode = 0.0;
                            //v56 = v63 + 1;
                            //v54->pts[0].iNodeNum = -1;
                            pt->iNodeNum = -1;
                            Path_DodgeDrawRaisedLine(pt[-1].vOrigPoint, pt->vOrigPoint, colorGreen);
                            goto done;
                        }
                    }
                }
            }
            return 0;
        }
    }
    Path_CheckNodeCountForDodge(pPath, 2, &pt, &startIndex);
    //v56 = a20;
    //v54 = (path_t *)pt[0];
done:
    iassert(pt == &pPath->pts[startIndex]);
    //v66 = vNewDodgeEnd;
    pPath->wDodgeCount = 0;
    //if (v54->pts[0].vOrigPoint[0] != v66 || v54->pts[0].vOrigPoint[1] != v83)
    if (pt->vOrigPoint[0] != vNewDodgeEnd[0] || pt->vOrigPoint[1] != vNewDodgeEnd[1])
    {
        //pt->fOrigLength = Path_GetPathDir(v54->pts[0].fDir2D, &vNewDodgeEnd, (const float *)v54);
        pt->fOrigLength = Path_GetPathDir(pt->fDir2D, vNewDodgeEnd, pt->vOrigPoint);
        //v67 = (unsigned int)pt[0];
        //v54->pts[0].fOrigLength = *(float *)pt;
        iassert(!IS_NAN(pt->fOrigLength));
        //v54 = (path_t *)((char *)v54 + 28);
        //v68 = vNewDodgeEnd;
        //LOWORD(v56) = v56 + 1;
        pt++;
        startIndex++;
        ++pPath->wDodgeCount;
        //v54->pts[0].vOrigPoint[0] = v68;
        //v54->pts[0].vOrigPoint[1] = v83;
        //v54->pts[0].vOrigPoint[2] = v84;
        pt->vOrigPoint[0] = vNewDodgeEnd[0];
        pt->vOrigPoint[1] = vNewDodgeEnd[1];
        pt->vOrigPoint[2] = vNewDodgeEnd[2];
        //v54->pts[0].iNodeNum = -1;
        pt->iNodeNum = -1;
        //Path_DodgeDrawRaisedLine(&v54[-1].fCurrLength, (float *)v54, colorLtGreen);
        Path_DodgeDrawRaisedLine(pt[-1].vOrigPoint, pt->vOrigPoint, colorLtGreen);
    }
    //if (vNewDodgeStart != vNewDodgeEnd || v86 != v83)
    if (vNewDodgeStart[0] != vNewDodgeEnd[0] || vNewDodgeStart[1] != vNewDodgeEnd[1])
    {
        //pt->fOrigLength = Path_GetPathDir(v54->pts[0].fDir2D, &vNewDodgeStart, &vNewDodgeEnd);
        pt->fOrigLength = Path_GetPathDir(pt->fDir2D, vNewDodgeStart, vNewDodgeEnd);
        //v69 = (unsigned int)pt[0];
        //v54->pts[0].fOrigLength = *(float *)pt;
        iassert(!IS_NAN(pt->fOrigLength));
        //v54 = (path_t *)((char *)v54 + 28);
        //v70 = vNewDodgeStart;
        //LOWORD(v56) = v56 + 1;
        pt++;
        startIndex++;
        ++pPath->wDodgeCount;
        //v54->pts[0].vOrigPoint[0] = v70;
        //v54->pts[0].vOrigPoint[1] = v86;
        //v54->pts[0].vOrigPoint[2] = v87;
        //v54->pts[0].iNodeNum = -1;
        //Path_DodgeDrawRaisedLine(&v54[-1].fCurrLength, (float *)v54, colorMdYellow);
        pt->vOrigPoint[0] = vNewDodgeStart[0];
        pt->vOrigPoint[1] = vNewDodgeStart[1];
        pt->vOrigPoint[2] = vNewDodgeStart[2];
        pt->iNodeNum = -1;
        Path_DodgeDrawRaisedLine(pt[-1].vOrigPoint, pt->vOrigPoint, colorMdYellow);
    }
    //v71 = vNewDodgeStart;
    pPath->fCurrLength = pt[-1].fOrigLength;
    pPath->vCurrPoint[0] = vNewDodgeStart[0];
    pPath->vCurrPoint[1] = vNewDodgeStart[1];
    pPath->vCurrPoint[2] = vNewDodgeStart[2];
    pt->fOrigLength = 0.0f;
    pt->fDir2D[0] = 0.0f;
    pt->fDir2D[1] = 0.0f;
    //flags = pPath->flags;
    pPath->flags &= 0xFFFFFDBC;
    pPath->wPathLen = startIndex + 1;
    pPath->wOrigPathLen = pPath->wPathLen;
    Path_TransferLookahead(pPath, vOrg);

    iassert(pPath->wNegotiationStartNode <= pPath->lookaheadNextNode);
    iassert(pPath->lookaheadNextNode < pPath->wPathLen);
    iassert(pPath->fLookaheadDistToNextNode <= pPath->pts[pPath->lookaheadNextNode].fOrigLength);

    iassert(pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength);
    iassert(!pPath->wPathLen || !pPath->wNegotiationStartNode || (pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0 && pPath->pts[pPath->wNegotiationStartNode - 1].iNodeNum >= 0));
    iassert(!pPath->fLookaheadDistToNextNode || (pPath->lookaheadNextNode < pPath->wPathLen - 1));

    return 1;
}

pathnode_t *__cdecl Path_FindCloseNode(
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vGoalPos,
    bool bAllowNegotiationLinks)
{
    CustomSearchInfo_FindCloseNode info; // [sp+50h] [-50h] BYREF

    iassert(pNodeFrom);
    iassert((pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0);
    iassert(vGoalPos);

    info.goalPos[0] = vGoalPos[0];
    info.goalPos[1] = vGoalPos[1];
    info.goalPos[2] = vGoalPos[2];
    info.closestNode = pNodeFrom;
    info.closestDistSq = Vec2DistanceSq(pNodeFrom->constant.vOrigin, vGoalPos);

    Path_AStarAlgorithm<CustomSearchInfo_FindCloseNode, true, false>(
        0,
        eTeam,
        pNodeFrom->constant.vOrigin,
        pNodeFrom,
        vGoalPos,
        1,
        bAllowNegotiationLinks,
        &info);
    return info.closestNode;
}

bool __cdecl Path_FindPathFromToWithWidth(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vStartPos,
    pathnode_t *pNodeTo,
    const float *vGoalPos,
    bool bAllowNegotiationLinks,
    double width,
    float *perp)
{
    CustomSearchInfo_FindPathWithWidth info; // [sp+50h] [-70h] BYREF

    iassert(pNodeFrom);
    iassert(pNodeTo);
    iassert((pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0);
    iassert((pNodeTo->constant.spawnflags & PNF_DONTLINK) == 0);
   
    info.width = width;
    info.m_pNodeTo = pNodeTo;
    info.perp[0] = perp[0];
    info.perp[1] = perp[1];

    pNodeFrom->transient.costFactor = 0.0f;

    return Path_AStarAlgorithm<CustomSearchInfo_FindPathWithWidth>(
        pPath,
        eTeam,
        vStartPos,
        pNodeFrom,
        vGoalPos,
        1,
        bAllowNegotiationLinks,
        &info);
}

bool __cdecl Path_FindPathFromToNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vStartPos,
    pathnode_t *pNodeTo,
    const float *vGoalPos,
    float (*vNormal)[2],
    float *fDist,
    int iPlaneCount,
    bool bAllowNegotiationLinks)
{
    CustomSearchInfo_FindPathNotCrossPlanes info; // [sp+50h] [-80h] BYREF

    info.negotiationOverlapCost = ai_pathNegotiationOverlapCost->current.value;

    iassert(pNodeFrom);
    iassert(pNodeTo);
    iassert((pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0);
    iassert((pNodeTo->constant.spawnflags & PNF_DONTLINK) == 0);

    info.m_pNodeTo = pNodeTo;
    info.m_iPlaneCount = iPlaneCount;
    info.m_vNormal = vNormal;
    info.m_fDist = fDist;

    info.startPos[0] = vStartPos[0];
    info.startPos[1] = vStartPos[1];
    info.startPos[2] = vStartPos[2];

    if (info.IgnoreNode(pNodeFrom) || info.IgnoreNode(pNodeTo))
    {
        return 0;
    }
    else
    {
        return Path_AStarAlgorithm<CustomSearchInfo_FindPathNotCrossPlanes>(
            pPath,
            eTeam,
            vStartPos,
            pNodeFrom,
            vGoalPos,
            1,
            bAllowNegotiationLinks,
            &info);
    }
}

bool __cdecl Path_FindPathFromAway(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    float *vAwayFromPos,
    float fDistAway,
    bool bAllowNegotiationLinks)
{
    pathnode_t *m_pBestNode; // r29
    int success; // r31
    CustomSearchInfo_FindPathAway info; // [sp+50h] [-70h] BYREF

    iassert(pNodeFrom);

    info.m_fBestScore = -1.0;
    info.m_pBestNode = 0;
    info.m_fDistAway = fDistAway;
    info.m_fDistAwaySqrd = (float)fDistAway * (float)fDistAway;
    info.m_vAwayFromPos[0] = vAwayFromPos[0];
    info.m_vAwayFromPos[1] = vAwayFromPos[1];
    info.m_vAwayFromPos[2] = vAwayFromPos[2];
    info.m_fInitialDistAwaySq = (float)((float)(vAwayFromPos[1] - vStartPos[1]) * (float)(vAwayFromPos[1] - vStartPos[1]))
        + (float)((float)((float)(info.m_vAwayFromPos[0] - (float)vStartPos[0])
            * (float)(info.m_vAwayFromPos[0] - (float)vStartPos[0]))
            + (float)((float)(vAwayFromPos[2] - vStartPos[2]) * (float)(vAwayFromPos[2] - vStartPos[2])));

    if (Path_AStarAlgorithm<CustomSearchInfo_FindPathAway, false, false>(pPath, eTeam, vStartPos, pNodeFrom, 0, 0, bAllowNegotiationLinks, &info))
        return 1;

    m_pBestNode = info.m_pBestNode;
    iassert(info.m_pBestNode);

    if (m_pBestNode == pNodeFrom)
        return 0;

    if (!pPath)
        return 1;

    //Profile_Begin(231);
    success = Path_GeneratePath(pPath, eTeam, vStartPos, 0, pNodeFrom, m_pBestNode, 0, bAllowNegotiationLinks);

    iassert(pPath->wPathLen <= pPath->wOrigPathLen);
    iassert(success || !pPath->wOrigPathLen);

    //Profile_EndInternal(0);
    return success;
}

bool __cdecl Path_FindPathFromAwayNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    float *vAwayFromPos,
    float fDistAway,
    float (*vNormal)[2],
    float *fDist,
    int iPlaneCount,
    bool bAllowNegotiationLinks)
{
    pathnode_t *m_pBestNode; // r30
    int success; // r31
    CustomSearchInfo_FindPathAwayNotCrossPlanes info; // [sp+50h] [-90h] BYREF

    iassert(pNodeFrom);
    info.m_fBestScore = -1.0;
    info.m_pBestNode = 0;
    info.m_vAwayFromPos[0] = vAwayFromPos[0];
    info.m_vAwayFromPos[1] = vAwayFromPos[1];
    info.m_vAwayFromPos[2] = vAwayFromPos[2];
    info.m_vNormal = vNormal;
    info.m_fDist = fDist;
    info.m_fDistAway = fDistAway;
    info.m_iPlaneCount = iPlaneCount;
    info.m_fDistAwaySqrd = fDistAway * fDistAway;

    if (info.IgnoreNode(pNodeFrom))
        return 0;

    if (Path_AStarAlgorithm<CustomSearchInfo_FindPathAwayNotCrossPlanes, false, false>(
        pPath,
        eTeam,
        vStartPos,
        pNodeFrom,
        0,
        0,
        bAllowNegotiationLinks,
        &info))
    {
        return 1;
    }

    m_pBestNode = info.m_pBestNode;
    iassert(info.m_pBestNode);

    if (m_pBestNode == pNodeFrom || Path_NodesVisible(m_pBestNode, pNodeFrom))
        return 0;

    if (!pPath)
        return 1;

    //Profile_Begin(231);
    success = Path_GeneratePath(pPath, eTeam, vStartPos, 0, pNodeFrom, m_pBestNode, 0, bAllowNegotiationLinks);
    iassert(pPath->wPathLen <= pPath->wOrigPathLen);
    iassert(success || !pPath->wOrigPathLen);
    //Profile_EndInternal(0);
    return success;
}

bool __cdecl Path_FindPathInCylinderWithLOS(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    const actor_goal_s *goal,
    float fWithinDistSqrd,
    bool bAllowNegotiationLinks)
{
    pathnode_t *pNodeFrom; // r6
    CustomSearchInfo_FindPathInCylinderWithLOS info; // [sp+60h] [-370h] BYREF
    pathsort_t nodes[64]; // [sp+80h] [-350h] BYREF
    int nodeCount;

    info.negotiationOverlapCost = ai_pathNegotiationOverlapCost->current.value;
    info.m_pNodeTo = Path_NearestNode(vGoalPos, nodes, -2, 192.0, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);

    if (!info.m_pNodeTo)
        return NULL;

    pNodeFrom = Path_NearestNode(vStartPos, nodes, -2, 192.0, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);

    if (!pNodeFrom)
        return NULL;

    info.startPos[0] = vStartPos[0];
    info.startPos[1] = vStartPos[1];
    info.startPos[2] = vStartPos[2];
    info.m_fWithinDistSqrd = fWithinDistSqrd;
    info.goal = goal;

    return Path_AStarAlgorithm<CustomSearchInfo_FindPathInCylinderWithLOS>(
        pPath,
        eTeam,
        vStartPos,
        pNodeFrom,
        vGoalPos,
        0,
        bAllowNegotiationLinks,
        &info);
}

bool __cdecl Path_FindPathInCylinderWithLOSNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    const actor_goal_s *goal,
    float fWithinDistSqrd,
    float (*vNormal)[2],
    float *fDist,
    int iPlaneCount,
    bool bAllowNegotiationLinks)
{
    pathnode_t *pNodeFrom; // r6
    CustomSearchInfo_FindPathInCylinderWithLOSNotCrossPlanes info; // [sp+70h] [-390h] BYREF
    pathsort_t nodes[64]; // [sp+A0h] [-360h] BYREF
    int nodeCount;

    info.negotiationOverlapCost = ai_pathNegotiationOverlapCost->current.value;
    info.m_pNodeTo = Path_NearestNode(vGoalPos, nodes, -2, 192.0, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);

    if (!info.m_pNodeTo)
        return NULL;

    pNodeFrom = Path_NearestNodeNotCrossPlanes(
        vStartPos,
        nodes,
        -2,
        192.0,
        vNormal,
        fDist,
        iPlaneCount,
        &nodeCount,
        64,
        NEAREST_NODE_DO_HEIGHT_CHECK
    );

    if (!pNodeFrom)
        return NULL;

    info.startPos[0] = vStartPos[0];
    info.startPos[1] = vStartPos[1];
    info.startPos[2] = vStartPos[2];
    info.goal = goal;
    info.m_fWithinDistSqrd = fWithinDistSqrd;
    info.m_iPlaneCount = iPlaneCount;
    info.m_vNormal = vNormal;
    info.m_fDist = fDist;

    return Path_AStarAlgorithm<CustomSearchInfo_FindPathInCylinderWithLOSNotCrossPlanes>(
        pPath,
        eTeam,
        vStartPos,
        pNodeFrom,
        vGoalPos,
        false,
        bAllowNegotiationLinks,
        &info);
}

bool __cdecl Path_FindPathFromInCylinder(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    const float *vGoalPos,
    float *vOrigin,
    float fRadiusSqrd,
    float fHalfHeightSqrd,
    bool bAllowNegotiationLinks)
{
    pathnode_t *pNodeTo; // r3
    CustomSearchInfo_FindPathFromInCylinder info; // [sp+50h] [-380h] BYREF
    pathsort_t nodes[64]; // [sp+80h] [-350h] BYREF
    int nodeCount;

    info.negotiationOverlapCost = ai_pathNegotiationOverlapCost->current.value;
    iassert((pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0);
    pNodeTo = Path_NearestNode(vGoalPos, nodes, -2, 192.0, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);

    if (pNodeTo)
    {
        info.m_vOrigin[0] = vOrigin[0];
        info.m_vOrigin[1] = vOrigin[1];

        info.startPos[0] = vStartPos[0];
        info.startPos[1] = vStartPos[1];
        info.startPos[2] = vStartPos[2];

        info.m_pNodeTo = pNodeTo;
        info.m_fRadiusSqrd = fRadiusSqrd;
        info.m_fHalfHeightSqrd = fHalfHeightSqrd;

        return Path_AStarAlgorithm<CustomSearchInfo_FindPathFromInCylinder>(
            pPath,
            eTeam,
            vStartPos,
            pNodeFrom,
            vGoalPos,
            1,
            bAllowNegotiationLinks,
            &info);
    }

    return NULL;
}

int __cdecl Path_FindPathFromInCylinderNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vStartPos,
    const float *vGoalPos,
    float *vOrigin,
    float fRadiusSqrd,
    float fHalfHeightSqrd,
    float (*vNormal)[2],
    float *fDist,
    int iPlaneCount,
    bool bAllowNegotiationLinks)
{
    pathnode_t *pNodeTo; // r11
    nearestNodeHeightCheck v47; // [sp+8h] [-3F8h]
    CustomSearchInfo_FindPathFromInCylinderNotCrossPlanes info; // [sp+60h] [-3A0h] BYREF
    _BYTE v49[16]; // [sp+90h] [-370h] BYREF
    pathsort_t nodes[64]; // [sp+A0h] [-360h] BYREF
    int nodeCount;

    info.negotiationOverlapCost = ai_pathNegotiationOverlapCost->current.value;
    iassert((pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0);

    pNodeTo = Path_NearestNodeNotCrossPlanes(vGoalPos, nodes, -2, 192.0, vNormal, fDist, iPlaneCount, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);

    if (!pNodeTo)
        return 0;

    info.m_vOrigin[0] = vOrigin[0];
    info.m_vOrigin[1] = vOrigin[1];
    info.m_pNodeTo = pNodeTo;
    info.m_iPlaneCount = iPlaneCount;
    info.m_vNormal = vNormal;
    info.m_fDist = fDist;
    info.startPos[0] = vStartPos[0];
    info.startPos[1] = vStartPos[1];
    info.startPos[2] = vStartPos[2];
    info.m_fRadiusSqrd = fRadiusSqrd;
    info.m_fHalfHeightSqrd = fHalfHeightSqrd;

    if (info.IgnoreNode(pNodeFrom))
        return 0;
    else
        return Path_AStarAlgorithm<CustomSearchInfo_FindPathFromInCylinderNotCrossPlanes>(
            pPath,
            eTeam,
            vStartPos,
            pNodeFrom,
            vGoalPos,
            1,
            bAllowNegotiationLinks,
            &info);
}

const pathnode_t *__cdecl Path_FindFacingNode(sentient_s *pSelf, sentient_s *pOther, sentient_info_t *pInfo)
{
    pathnode_t *v6; // r29
    const pathnode_t *result; // r3
    pathnode_t *pLastKnownNode; // r31
    bool v9; // zf
    CustomSearchInfo_CouldAttack v10; // [sp+50h] [-60h] BYREF
    float v11[4]; // [sp+58h] [-58h] BYREF
    float v12[18]; // [sp+68h] [-48h] BYREF

    v6 = Sentient_NearestNode(pSelf);
    v10.m_pNodeTo = v6;
    if (!v6)
        return 0;
    if (!pInfo || (pLastKnownNode = pInfo->pLastKnownNode) == 0)
    {
        pLastKnownNode = Sentient_NearestNode(pOther);
        if (!pLastKnownNode)
            return 0;
    }
    if (Path_NodesVisible(pLastKnownNode, v6))
        return pLastKnownNode;
    Sentient_GetOrigin(pSelf, v12);
    if (pInfo)
    {
        v11[0] = pInfo->vLastKnownPos[0];
        v11[1] = pInfo->vLastKnownPos[1];
        v11[2] = pInfo->vLastKnownPos[2];
    }
    else
    {
        Sentient_GetOrigin(pOther, v11);
    }
    v9 = Path_AStarAlgorithm<CustomSearchInfo_CouldAttack>(0, pOther->eTeam, v11, pLastKnownNode, v12, 0, 1, &v10) == 0;
    result = 0;
    if (!v9)
        return v10.attackNode;
    return result;
}

bool __cdecl Path_FindPathGetCloseAsPossible(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    pathnode_t *pNodeTo,
    const float *vGoalPos,
    bool bAllowNegotiationLinks)
{
    CustomSearchInfo_FindPathClosestPossible info; // [sp+50h] [-60h] BYREF

    iassert(pNodeTo);
    iassert(pNodeFrom);
    iassert((pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0);

    info.negotiationOverlapCost = ai_pathNegotiationOverlapCost->current.value;
    info.m_fBestScore = FLT_MAX;
    info.m_pBestNode = 0;
    info.m_pNodeTo = pNodeTo;

    if (Path_AStarAlgorithm<CustomSearchInfo_FindPathClosestPossible>(
        pPath,
        eTeam,
        vStartPos,
        pNodeFrom,
        vGoalPos,
        1,
        bAllowNegotiationLinks,
        &info))
    {
        return 1;
    }

    iassert(info.m_pBestNode);

    //Profile_Begin(231);
    if (Path_GeneratePath(
        pPath,
        eTeam,
        vStartPos,
        info.m_pBestNode->constant.vOrigin,
        pNodeFrom,
        info.m_pBestNode,
        1,
        bAllowNegotiationLinks))
    {
        iassert(pPath->wPathLen <= pPath->wOrigPathLen);
        //Profile_EndInternal(0);
        return 1;
    }

    //Profile_EndInternal(0);
    return 0;
}

bool __cdecl Path_FindPathWithWidth(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    bool bAllowNegotiationLinks,
    double width,
    float *perp)
{
    pathnode_t *pNodeTo; // r30
    pathnode_t *pNodeFrom; // r5
    pathsort_t nodes[64]; // [sp+70h] [-350h] BYREF
    int nodeCount;

    pNodeTo = Path_NearestNode(vGoalPos, nodes, -2, 192.0, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);

    if (pNodeTo && (pNodeFrom = Path_NearestNode(vStartPos, nodes, -2, 192.0, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK)))
        return Path_FindPathFromToWithWidth(
            pPath,
            eTeam,
            pNodeFrom,
            vStartPos,
            pNodeTo,
            vGoalPos,
            bAllowNegotiationLinks,
            width,
            perp);
    else
        return 0;
}

bool __cdecl Path_FindPathNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    float (*vNormal)[2],
    float *fDist,
    int iPlaneCount,
    bool bAllowNegotiationLinks)
{
    pathnode_t *pNodeTo; // r26
    pathnode_t *pNodeFrom; // r5
    pathsort_t nodes[64]; // [sp+70h] [-370h] BYREF
    int nodeCount;

    pNodeTo = Path_NearestNodeNotCrossPlanes(
        vGoalPos,
        nodes,
        -2,
        192.0,
        vNormal,
        fDist,
        iPlaneCount,
        &nodeCount,
        64,
        NEAREST_NODE_DO_HEIGHT_CHECK
    );

    if (pNodeTo && (pNodeFrom = Path_NearestNodeNotCrossPlanes(
            vStartPos,
            nodes,
            -2,
            192.0,
            vNormal,
            fDist,
            iPlaneCount,
            &nodeCount,
            64,
            NEAREST_NODE_DO_HEIGHT_CHECK)))
    {
        return Path_FindPathFromToNotCrossPlanes(
            pPath,
            eTeam,
            pNodeFrom,
            vStartPos,
            pNodeTo,
            vGoalPos,
            vNormal,
            fDist,
            iPlaneCount,
            bAllowNegotiationLinks);
    }
    else
    {
        return NULL;
    }
}

bool __cdecl Path_FindPathFromNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vStartPos,
    const float *vGoalPos,
    float (*vNormal)[2],
    float *fDist,
    int iPlaneCount,
    bool bAllowNegotiationLinks)
{
    pathnode_t *nodeTo; // r3
    int nodeCount; // [sp+60h] [-360h] BYREF
    pathsort_t nodes[64]; // [sp+70h] [-350h] BYREF

    nodeTo = Path_NearestNodeNotCrossPlanes(
        vGoalPos,
        nodes,
        -2,
        192.0,
        vNormal,
        fDist,
        iPlaneCount,
        &nodeCount,
        64,
        NEAREST_NODE_DO_HEIGHT_CHECK);

    if (nodeTo)
    {
        return Path_FindPathFromToNotCrossPlanes(
            pPath,
            eTeam,
            pNodeFrom,
            vStartPos,
            nodeTo,
            vGoalPos,
            vNormal,
            fDist,
            iPlaneCount,
            bAllowNegotiationLinks);
    }

    return NULL;
}

pathnode_t *__cdecl Path_FindPathAway(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    float *vAwayFromPos,
    float fDistAway,
    bool bAllowNegotiationLinks)
{
    int v13; // r8
    pathnode_t *result; // r3
    _BYTE v15[16]; // [sp+50h] [-350h] BYREF
    pathsort_t nodes[64]; // [sp+60h] [-340h] BYREF

    result = Path_NearestNode(vStartPos, nodes, -2, 192.0, (int *)vAwayFromPos, (int)v15, (nearestNodeHeightCheck)64);
    if (result)
        return (pathnode_t *)Path_FindPathFromAway(pPath, eTeam, result, vStartPos, vAwayFromPos, fDistAway, bAllowNegotiationLinks);
    return result;
}

pathnode_t *__cdecl Path_FindPathAwayNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    float *vAwayFromPos,
    float fDistAway,
    float (*vNormal)[2],
    float *fDist,
    int iPlaneCount,
    bool bAllowNegotiationLinks)
{
    float *v37; // r8
    pathnode_t *pNodeTo; // r3
    nearestNodeHeightCheck v39; // [sp+8h] [-3B8h]
    int v60[16]; // [sp+60h] [-360h] BYREF
    int nodeCount;
    pathsort_t nodes[64]; // [sp+70h] [-350h] BYREF

    pNodeTo = Path_NearestNodeNotCrossPlanes(
        vStartPos,
        nodes,
        -2,
        192.0,
        0,
        fDist,
        iPlaneCount,
        &nodeCount,
        64,
        NEAREST_NODE_DO_HEIGHT_CHECK);

    if (pNodeTo)
        return (pathnode_t *)Path_FindPathFromAwayNotCrossPlanes(
            pPath,
            eTeam,
            pNodeTo,
            vStartPos,
            vAwayFromPos,
            fDistAway,
            (float (*)[2])v37,
            fDist,
            iPlaneCount,
            bAllowNegotiationLinks);
    return pNodeTo;
}

