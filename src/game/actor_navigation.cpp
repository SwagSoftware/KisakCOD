#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_navigation.h"
#include <universal/com_math.h>

//   int const __cdecl Path_AStarAlgorithm<class CustomSearchInfo_FindCloseNode>(struct path_t *, enum team_t, float const *const, struct pathnode_t *, float const *const, int, int, class CustomSearchInfo_FindCloseNode &) 82202a88 f   actor_navigation.obj
//   int const __cdecl Path_AStarAlgorithm<class CustomSearchInfo_FindPathWithWidth>(struct path_t *, enum team_t, float const *const, struct pathnode_t *, float const *const, int, int, class CustomSearchInfo_FindPathWithWidth &) 82202dd0 f   actor_navigation.obj
//   int const __cdecl Path_AStarAlgorithm<class CustomSearchInfo_FindPathNotCrossPlanes>(struct path_t *, enum team_t, float const *const, struct pathnode_t *, float const *const, int, int, class CustomSearchInfo_FindPathNotCrossPlanes &) 82203188 f   actor_navigation.obj
//   int const __cdecl Path_AStarAlgorithm<class CustomSearchInfo_FindPathAway>(struct path_t *, enum team_t, float const *const, struct pathnode_t *, float const *const, int, int, class CustomSearchInfo_FindPathAway &) 822035a0 f   actor_navigation.obj
//   int const __cdecl Path_AStarAlgorithm<class CustomSearchInfo_FindPathAwayNotCrossPlanes>(struct path_t *, enum team_t, float const *const, struct pathnode_t *, float const *const, int, int, class CustomSearchInfo_FindPathAwayNotCrossPlanes &) 822039d8 f   actor_navigation.obj
//   int const __cdecl Path_AStarAlgorithm<class CustomSearchInfo_FindPathInCylinderWithLOS>(struct path_t *, enum team_t, float const *const, struct pathnode_t *, float const *const, int, int, class CustomSearchInfo_FindPathInCylinderWithLOS &) 82203e78 f   actor_navigation.obj
//   int const __cdecl Path_AStarAlgorithm<class CustomSearchInfo_FindPathInCylinderWithLOSNotCrossPlanes>(struct path_t *, enum team_t, float const *const, struct pathnode_t *, float const *const, int, int, class CustomSearchInfo_FindPathInCylinderWithLOSNotCrossPlanes &) 82204298 f   actor_navigation.obj
//   int const __cdecl Path_AStarAlgorithm<class CustomSearchInfo_FindPathFromInCylinder>(struct path_t *, enum team_t, float const *const, struct pathnode_t *, float const *const, int, int, class CustomSearchInfo_FindPathFromInCylinder &) 822046b8 f   actor_navigation.obj
//   int const __cdecl Path_AStarAlgorithm<class CustomSearchInfo_FindPathFromInCylinderNotCrossPlanes>(struct path_t *, enum team_t, float const *const, struct pathnode_t *, float const *const, int, int, class CustomSearchInfo_FindPathFromInCylinderNotCrossPlanes &) 82204ac0 f   actor_navigation.obj
//   int const __cdecl Path_AStarAlgorithm<class CustomSearchInfo_CouldAttack>(struct path_t *, enum team_t, float const *const, struct pathnode_t *, float const *const, int, int, class CustomSearchInfo_CouldAttack &) 82204e88 f   actor_navigation.obj
//   int const __cdecl Path_AStarAlgorithm<class CustomSearchInfo_FindPathClosestPossible>
//   int const __cdecl Path_AStarAlgorithm<class CustomSearchInfo_FindPath>(struct path_t *, enum team_t, float const *const, struct pathnode_t *, float const *const, int, int, class CustomSearchInfo_FindPath &) 821fef98 f   actor_navigation.obj

// Line 26290:  0004 : 000f6960       public: float const __cdecl CustomSearchInfo_FindPath::EvaluateHeuristic(struct pathnode_t *, float const *const) 82206960 f i actor_navigation.obj
// Line 26291 : 0004 : 000f6a00       public: float const __cdecl CustomSearchInfo_FindPathWithWidth::EvaluateHeuristic(struct pathnode_t *, float const *const) 82206a00 f i actor_navigation.obj
// Line 26292 : 0004 : 000f6a98       public: BOOL const __cdecl CustomSearchInfo_FindPathNotCrossPlanes::IgnoreNode(struct pathnode_t *) 82206a98 f i actor_navigation.obj
// Line 26293 : 0004 : 000f6af8       public: BOOL const __cdecl CustomSearchInfo_FindPathAwayNotCrossPlanes::IgnoreNode(struct pathnode_t *) 82206af8 f i actor_navigation.obj
// Line 26294 : 0004 : 000f6b58       public: float const __cdecl CustomSearchInfo_FindPathWithLOS::EvaluateHeuristic(struct pathnode_t *, float const *const) 82206b58 f i actor_navigation.obj
// Line 26295 : 0004 : 000f6bf8       public: BOOL const __cdecl CustomSearchInfo_FindPathInCylinderWithLOSNotCrossPlanes::IgnoreNode(struct pathnode_t *) 82206bf8 f i actor_navigation.obj
// Line 26296 : 0004 : 000f6ca0       public: BOOL const __cdecl CustomSearchInfo_FindPathFromInCylinderNotCrossPlanes::IgnoreNode(struct pathnode_t *) 82206ca0 f i actor_navigation.obj

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
    v4 = __fsqrts((float)((float)(delta[1] * delta[1]) + (float)(*delta * *delta)));
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
    v6 = __fabs((float)((float)(pt->fDir2D[0] * (float)v5) - (float)(pt->fDir2D[1] * (float)v4)));
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
    double fOrigLength; // fp1
    double fCurrLength; // fp2
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
            v12 = __fsqrts((float)((float)((float)(*vOrigPoint - *vStartPos) * (float)(*vOrigPoint - *vStartPos))
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
            v13 = __fabs((float)((float)((float)v11 * v9[3]) - (float)(v9[4] * (float)v10)));
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
    double fOrigLength; // fp1
    double fCurrLength; // fp2
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
    pPath->wDodgeEntity = 2175;
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
    double fCurrLength; // fp1
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
    double fCurrLength; // fp1
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

int __cdecl Path_EncroachesPoint2D(path_t *pPath, const float *vStart, const float *vPoint, double fMinDistSqrd)
{
    double v8; // fp0
    double v9; // fp13
    double v10; // fp12
    double v11; // fp12
    double fLookaheadDistToNextNode; // fp28
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

int __cdecl Path_DistanceGreaterThan(path_t *pPath, double fDist)
{
    int wPathLen; // r11
    double fCurrLength; // fp1
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
    double v1; // fp0
    int flags; // r11
    int v3; // r11

    v1 = (float)((float)(pPath->fLookaheadAmount * (float)1.1764705) + (float)6.4000001);
    pPath->fLookaheadAmount = (float)(pPath->fLookaheadAmount * (float)1.1764705) + (float)6.4000001;
    if (v1 > 65536.0)
        pPath->fLookaheadAmount = 65536.0;
    flags = pPath->flags;
    if ((flags & 2) != 0)
        v3 = flags | 0x200;
    else
        v3 = flags | 2;
    pPath->flags = v3;
}

bool __cdecl Path_PredictionTrace(
    float *vStartPos,
    float *vEndPos,
    int entityIgnore,
    int mask,
    float *vTraceEndPos,
    double stepheight,
    int allowStartSolid,
    int a8)
{
    double v15; // fp0
    double v16; // fp12
    bool v17; // cr58
    double v18; // fp13
    double v19; // fp10
    double v20; // fp8
    double fraction; // fp0
    double v22; // fp11
    double v23; // fp12
    gentity_s *v24; // r11
    actor_s *actor; // r11
    actor_s *pPileUpActor; // r11
    double v27; // fp0
    double v28; // fp13
    double v29; // fp11
    double v30; // fp9
    double v31; // fp10
    double v32; // fp0
    double v33; // fp8
    double v34; // fp12
    double v35; // fp12
    double v36; // fp11
    double v37; // fp9
    double v38; // fp10
    double v39; // fp0
    double v40; // fp8
    double v41; // fp13
    float v43; // [sp+50h] [-F0h] BYREF
    float v44; // [sp+54h] [-ECh]
    float v45; // [sp+58h] [-E8h]
    float v46; // [sp+60h] [-E0h] BYREF
    float v47; // [sp+64h] [-DCh]
    float v48; // [sp+68h] [-D8h]
    float v49; // [sp+70h] [-D0h] BYREF
    float v50; // [sp+74h] [-CCh]
    float v51; // [sp+78h] [-C8h]
    float v52[4]; // [sp+80h] [-C0h] BYREF
    trace_t v53[2]; // [sp+90h] [-B0h] BYREF

    v52[2] = stepheight;
    v52[0] = -15.0;
    v52[1] = -15.0;
    if (!vStartPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2610, 0, "%s", "vStartPos");
    if (!vEndPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2611, 0, "%s", "vEndPos");
    v49 = *vEndPos;
    v50 = vEndPos[1];
    v43 = *vStartPos;
    v15 = vStartPos[2];
    v44 = vStartPos[1];
    while (1)
    {
        v45 = v15;
        v51 = v15;
        G_TraceCapsule(v53, &v43, v52, PREDICTION_TRACE_MAX, &v49, entityIgnore, mask);
        v16 = v43;
        v17 = !v53[0].startsolid;
        v18 = v45;
        v19 = (float)(v49 - v43);
        v20 = (float)(v51 - v45);
        fraction = v53[0].fraction;
        v22 = (float)((float)((float)(v50 - v44) * v53[0].fraction) + v44);
        vTraceEndPos[1] = (float)((float)(v50 - v44) * v53[0].fraction) + v44;
        v23 = (float)((float)((float)v19 * (float)fraction) + (float)v16);
        *vTraceEndPos = v23;
        vTraceEndPos[2] = (float)((float)v20 * (float)fraction) + (float)v18;
        if (!v17 && !a8)
            break;
        if (!v53[0].allsolid && fraction == 1.0)
        {
            v46 = *vTraceEndPos;
            v47 = vTraceEndPos[1];
            v43 = v23;
            v44 = v22;
            v48 = (float)v18 - (float)72.0;
            G_TraceCapsule(v53, &v43, v52, PREDICTION_TRACE_MAX, &v46, entityIgnore, mask);
            v35 = v45;
            v36 = v43;
            v37 = (float)(v46 - v43);
            v38 = v44;
            v39 = v53[0].fraction;
            v40 = (float)(v47 - v44);
            v41 = (float)((float)((float)(v48 - v45) * v53[0].fraction) + v45);
            vTraceEndPos[2] = (float)((float)(v48 - v45) * v53[0].fraction) + v45;
            *vTraceEndPos = (float)((float)v37 * (float)v39) + (float)v36;
            vTraceEndPos[1] = (float)((float)v40 * (float)v39) + (float)v38;
            if (v41 < v35 || v39 == 1.0 || v53[0].normal[2] >= 0.69999999)
            {
                vTraceEndPos[2] = (float)v41 + (float)stepheight;
                return __fabs((float)((float)((float)v41 + (float)stepheight) - vEndPos[2])) < 72.0;
            }
            return 0;
        }
        v24 = &level.gentities[Trace_GetEntityHitId(v53)];
        if (v24->sentient)
        {
            actor = v24->actor;
            if (!actor)
                return 0;
            if (actor->eAnimMode != AI_ANIM_MOVE_CODE || !actor->moveMode)
            {
                pPileUpActor = actor->pPileUpActor;
                if (!pPileUpActor || pPileUpActor->ent->s.number != entityIgnore)
                    return 0;
            }
            mask &= ~0x4000u;
        }
        else if (Vec2DistanceSq(vTraceEndPos, &v43) < 225.0)
        {
            return 0;
        }
        v43 = *vTraceEndPos;
        v46 = v43;
        v27 = vTraceEndPos[1];
        v44 = vTraceEndPos[1];
        v48 = v45 - (float)72.0;
        v47 = v27;
        G_TraceCapsule(v53, &v43, v52, PREDICTION_TRACE_MAX, &v46, entityIgnore, mask);
        v28 = v45;
        v29 = v43;
        v30 = (float)(v46 - v43);
        v31 = v44;
        v32 = v53[0].fraction;
        v33 = (float)(v47 - v44);
        v34 = (float)((float)((float)(v48 - v45) * v53[0].fraction) + v45);
        vTraceEndPos[2] = (float)((float)(v48 - v45) * v53[0].fraction) + v45;
        *vTraceEndPos = (float)((float)v30 * (float)v32) + (float)v29;
        vTraceEndPos[1] = (float)((float)v33 * (float)v32) + (float)v31;
        if (v34 >= v28 && v32 != 1.0 && v53[0].normal[2] < 0.69999999)
            return 0;
        v15 = (float)((float)v34 + (float)stepheight);
    }
    return 0;
}

int __cdecl Path_IsTrimmed(path_t *pPath)
{
    int wPathLen; // r11
    double fCurrLength; // fp1
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
    double fCurrLength; // fp1
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
    double fCurrLength; // fp1
    double v6; // fp2
    const char *v7; // r3
    int v8; // r10
    int v9; // r28
    float *i; // r27
    double v11; // fp28
    pathpoint_t *vCurrPoint; // r25
    const char *v13; // r3
    double v14; // fp29
    const char *v15; // r3
    const char *v16; // r3
    double v17; // fp31
    __int16 v18; // r4
    double v19; // fp13
    double v20; // fp12
    double v21; // fp1
    double v22; // fp2
    float v23; // [sp+50h] [-70h]
    float v24; // [sp+54h] [-6Ch]
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
            v23 = (float)(pPath->lookaheadDir[1] * (float)(i[1] - vStartPos[1]))
                + (float)(pPath->lookaheadDir[0] * (float)(*i - *vStartPos));
            if (v23 > 0.0)
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
        v24 = (float)((float)(pPath->lookaheadDir[1] * (float)(vCurrPoint->vOrigPoint[1] - vStartPos[1]))
            + (float)((float)(vCurrPoint->vOrigPoint[0] - *vStartPos) * pPath->lookaheadDir[0]))
            - (float)0.000099999997;
        if (v24 > 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2896,
                0,
                "%s\n\t(d1) = %g",
                (unsigned int)HIDWORD(COERCE_UNSIGNED_INT64(v24)),
                (unsigned int)COERCE_UNSIGNED_INT64(v24));
        if (v23 <= 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2897,
                0,
                "%s\n\t(d2) = %g",
                (unsigned int)HIDWORD(COERCE_UNSIGNED_INT64(v23)),
                (unsigned int)COERCE_UNSIGNED_INT64(v23));
        if ((float)(v24 - v23) >= 0.0)
        {
            v13 = va(
                (const char *)(const char *)HIDWORD(COERCE_UNSIGNED_INT64(v24)),
                (unsigned int)COERCE_UNSIGNED_INT64(v24),
                (unsigned int)COERCE_UNSIGNED_INT64(v23));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2898,
                0,
                "%s\n\t%s",
                "d1 - d2 < 0",
                v13);
        }
        v14 = (float)(v24 / (float)(v24 - v23));
        if (v14 < 0.0)
        {
            v15 = va(
                "i: %d, pPath->wPathLen: %d, d1: %f, d2: %f, fraction: %f",
                v9,
                v24,
                (unsigned int)COERCE_UNSIGNED_INT64(v23),
                LODWORD(v14));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2902,
                0,
                "%s\n\t%s",
                "fraction >= 0",
                v15);
        }
        if (v14 > 1.0)
        {
            v16 = va(
                "i: %d, pPath->wPathLen: %d, d1: %f, d2: %f, fraction: %f",
                v9,
                v24,
                (unsigned int)COERCE_UNSIGNED_INT64(v23),
                LODWORD(v14));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2905,
                0,
                "%s\n\t%s",
                "fraction <= 1.f",
                v16);
        }
        v25 = v11;
        if ((LODWORD(v25) & 0x7F800000) == 0x7F800000)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2907, 0, "%s", "!IS_NAN(fLength)");
        if (v11 <= 0.0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2908, 0, "%s", "fLength > 0");
        v17 = (float)((float)v14 * (float)v11);
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
            pPath->vCurrPoint[0] = (float)(i[3] * (float)((float)v14 * (float)v11)) + vCurrPoint->vOrigPoint[0];
            pPath->vCurrPoint[1] = (float)(i[4] * (float)((float)v14 * (float)v11)) + vCurrPoint->vOrigPoint[1];
            v19 = vCurrPoint->vOrigPoint[2];
            v20 = i[2];
            pPath->fCurrLength = (float)v11 - (float)((float)v14 * (float)v11);
            pPath->vCurrPoint[2] = (float)((float)((float)v20 - (float)v19) * (float)v14) + (float)v19;
            if ((COERCE_UNSIGNED_INT((float)v11 - (float)((float)v14 * (float)v11)) & 0x7F800000) == 0x7F800000)
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
    double fCurrLength; // fp1
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
    v12 = __fabs((float)((float)(v6[3] * (float)v9) - (float)(v6[4] * (float)v8)));
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
        v12 = __fabs((float)((float)(v14[3] * (float)v16) - (float)(v14[4] * (float)v15)));
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

int __cdecl Path_GetForwardStartPos(path_t *pPath, const float *vStartPos, float *vForwardStartPos)
{
    int result; // r3

    if (pPath->wPathLen <= 1 || (pPath->flags & 1) != 0)
        return 0;
    result = 1;
    _FP11 = (float)((float)60.0 - pPath->fLookaheadDist);
    __asm { fsel      f0, f11, f0, f13 }
    *vForwardStartPos = (float)(pPath->lookaheadDir[0] * (float)_FP0) + *vStartPos;
    vForwardStartPos[1] = (float)(pPath->lookaheadDir[1] * (float)_FP0) + vStartPos[1];
    return result;
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
    double fCurrLength; // fp1
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
    double fOrigLength; // fp31
    double v18; // fp28
    const char *v19; // r3
    const char *v20; // r3
    double fLookaheadAmount; // fp30
    double DistToPathSegment; // fp2
    double v23; // fp31
    float *p_fOrigLength; // r29
    double v25; // fp1
    double v26; // fp2
    float *vOrigPoint; // r11
    float v28; // [sp+50h] [-80h]
    float v29; // [sp+54h] [-7Ch]
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
            v28 = (float)(pPath->lookaheadDir[0] * (float)(v13->vOrigPoint[0] - v30))
                + (float)(pPath->lookaheadDir[1] * (float)(v13->vOrigPoint[1] - v31));
            if (v28 > 0.0)
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
            fOrigLength = pPath->fCurrLength;
            v15 = pPath->vCurrPoint[0];
            v16 = pPath->vCurrPoint[1];
        }
        else
        {
            v15 = v14[1].vOrigPoint[0];
            v16 = v14[1].vOrigPoint[1];
            fOrigLength = v13->fOrigLength;
        }
        v29 = (float)(pPath->lookaheadDir[1] * (float)((float)v16 - v31))
            + (float)(pPath->lookaheadDir[0] * (float)((float)v15 - v30));
        if (v29 > 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3415,
                0,
                "%s\n\t(d1) = %g",
                (unsigned int)HIDWORD(COERCE_UNSIGNED_INT64(v29)),
                (unsigned int)COERCE_UNSIGNED_INT64(v29));
        if (v28 <= 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3416,
                0,
                "%s\n\t(d2) = %g",
                (unsigned int)HIDWORD(COERCE_UNSIGNED_INT64(v28)),
                (unsigned int)COERCE_UNSIGNED_INT64(v28));
        if ((float)(v29 - v28) == 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3417,
                0,
                "%s\n\t(d1 - d2) = %g",
                (unsigned int)HIDWORD(COERCE_UNSIGNED_INT64((float)(v29 - v28))),
                (unsigned int)COERCE_UNSIGNED_INT64((float)(v29 - v28)));
        v18 = (float)(v29 / (float)(v29 - v28));
        if (v18 < 0.0)
        {
            v19 = va(
                "i: %d, pPath->wPathLen: %d, d1: %f, d2: %f, fraction: %f",
                v12,
                v29,
                (unsigned int)COERCE_UNSIGNED_INT64(v28),
                LODWORD(v18));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3421,
                0,
                "%s\n\t%s",
                "fraction >= 0",
                v19);
        }
        if (v18 > 1.0)
        {
            v20 = va(
                "i: %d, pPath->wPathLen: %d, d1: %f, d2: %f, fraction: %f",
                v12,
                v29,
                (unsigned int)COERCE_UNSIGNED_INT64(v28),
                LODWORD(v18));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3424,
                0,
                "%s\n\t%s",
                "fraction <= 1.f",
                v20);
        }
        if (fOrigLength <= 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3425,
                0,
                "%s\n\t(fLength) = %g",
                HIDWORD(fOrigLength),
                LODWORD(fOrigLength));
        fLookaheadAmount = pPath->fLookaheadAmount;
        if (fLookaheadAmount <= 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3430,
                0,
                "%s\n\t(lookaheadAmount) = %g",
                HIDWORD(fLookaheadAmount),
                LODWORD(fLookaheadAmount));
        DistToPathSegment = Path_GetDistToPathSegment(&v30, v13);
        v23 = (float)((float)((float)fOrigLength - (float)((float)(v29 / (float)(v29 - v28)) * (float)fOrigLength))
            * (float)DistToPathSegment);
        if (v23 < fLookaheadAmount)
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
                    if (v23 >= fLookaheadAmount)
                        break;
                    if (v12 <= pPath->wNegotiationStartNode)
                        goto LABEL_47;
                }
                Path_UpdateForwardLookahead_IncompletePath(
                    pPath,
                    (const pathpoint_t *)(p_fOrigLength - 5),
                    &v30,
                    (float)((float)v23 - (float)fLookaheadAmount),
                    v26);
            }
        }
        else
        {
            Path_UpdateForwardLookahead_IncompletePath(
                pPath,
                v13,
                &v30,
                (float)((float)v23 - (float)fLookaheadAmount),
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
    double fLookaheadDist; // fp0
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
    double fCurrLength; // fp1
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
            return 7;
        v11 = Path_ConvertIndexToNode(iNodeNum);
        if (!Path_IsPathStanceNode(v11))
            return 7;
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
    return Path_AllowedStancesForNode(v15);
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
    double fCurrLength; // fp1
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

int __cdecl Path_FindPathFromTo(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vStartPos,
    pathnode_t *pNodeTo,
    const float *vGoalPos,
    int bAllowNegotiationLinks)
{
    CustomSearchInfo_FindPath v15; // [sp+50h] [-70h] BYREF

    v15.negotiationOverlapCost = ai_pathNegotiationOverlapCost->current.value;
    if (!pNodeFrom)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 958, 0, "%s", "pNodeFrom");
    if (!pNodeTo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 959, 0, "%s", "pNodeTo");
    if ((pNodeFrom->constant.spawnflags & 1) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            960,
            0,
            "%s",
            "(pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0");
    if ((pNodeTo->constant.spawnflags & 1) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            961,
            0,
            "%s",
            "(pNodeTo->constant.spawnflags & PNF_DONTLINK) == 0");
    v15.startPos[0] = *vStartPos;
    v15.startPos[1] = vStartPos[1];
    v15.startPos[2] = vStartPos[2];
    v15.m_pNodeTo = pNodeTo;
    return Path_AStarAlgorithm<CustomSearchInfo_FindPath>(
        pPath,
        eTeam,
        vStartPos,
        pNodeFrom,
        vGoalPos,
        1,
        bAllowNegotiationLinks,
        &v15);
}

void __cdecl Path_TrimLastNodes(path_t *pPath, const int iNodeCount, bool bMaintainGoalPos)
{
    int wPathLen; // r11
    double fCurrLength; // fp1
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
    double fMaxDistSqrd,
    int iIgnoreEntityNum,
    const float *vPoint,
    const float *a7)
{
    int iDelta; // r11
    int v13; // r31
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

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2040, 0, "%s", "pPath");
    if (!pTrim)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2041, 0, "%s", "pTrim");
    if (!pActor)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2042, 0, "%s", "pActor");
    if (!a7)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2043, 0, "%s", "vPoint");
    if (pPath->wPathLen <= 0)
        return 0;
    iDelta = pTrim->iDelta;
    if (iDelta != -2 && iDelta != -1 && iDelta != 1)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2053,
            0,
            "%s",
            "pTrim->iDelta == -2 || pTrim->iDelta == -1 || pTrim->iDelta == +1");
    v13 = pTrim->iIndex + pTrim->iDelta;
    if (v13 < 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2056, 0, "%s", "i >= 0");
    if (v13 >= pPath->wPathLen)
        return 0;
    Actor_GetEyeOffset(pActor, &v22);
    v15 = pPath->wPathLen - 1 == v13;
    if (pPath->wPathLen - 1 == v13)
    {
        v18 = (float)(pPath->vCurrPoint[1] + v23);
        v25 = v22 + pPath->vCurrPoint[0];
        v17 = pPath->vCurrPoint[2];
        v26 = v18;
    }
    else
    {
        vOrigPoint = pPath->pts[v13].vOrigPoint;
        v25 = *vOrigPoint + v22;
        v26 = vOrigPoint[1] + v23;
        v17 = vOrigPoint[2];
    }
    v27 = (float)v17 + v24;
    CanSeePointFrom = Actor_CanSeePointFrom(pActor, &v25, a7, fMaxDistSqrd, v14);
    v20 = pTrim->iDelta;
    if (CanSeePointFrom)
    {
        if (v20 == -2 && !v15)
        {
            pTrim->iIndex = v13;
            result = 1;
            pTrim->iDelta = 1;
            return result;
        }
        Path_TrimLastNodes(pPath, v13, 0);
        return 0;
    }
    if (v20 == 1)
    {
        Path_TrimLastNodes(pPath, pTrim->iIndex, 0);
        return 0;
    }
    else if (v13)
    {
        if (v20 != -2)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 2109, 0, "%s", "pTrim->iDelta == -2");
        pTrim->iIndex = v13;
        if (v13 == 1)
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
    double v17; // fp0
    int v18; // r7
    PredictionTraceResult result; // r3
    float v20[2]; // [sp+50h] [-80h] BYREF
    float v21; // [sp+58h] [-78h]
    float v22[4]; // [sp+60h] [-70h] BYREF

    v22[0] = *vEndPos;
    v20[0] = -15.0;
    v13 = 0;
    v20[1] = -15.0;
    v22[2] = vStartPos[2];
    v22[1] = vEndPos[1];
    v21 = 0.0;
    if (entityCount <= 0)
        return !Path_PredictionTrace(vStartPos, vEndPos, entityIgnore, mask, vTraceEndPos, 18.0, mask, 1);
    while (1)
    {
        v15 = SV_GentityNum(*entities);
        if (v15 && (actor = v15->actor) != 0 && actor->Physics.prone)
            v17 = 10.0;
        else
            v17 = 18.0;
        v18 = *entities;
        v21 = v17;
        if (SV_SightTraceToEntity(vStartPos, v20, PREDICTION_TRACE_MAX, v22, v18, -1))
            break;
        ++v13;
        ++entities;
        if (v13 >= entityCount)
            return !Path_PredictionTrace(vStartPos, vEndPos, entityIgnore, mask, vTraceEndPos, 18.0, mask, 1);
    }
    result = PTR_HIT_ENTITY;
    *vTraceEndPos = *vStartPos;
    vTraceEndPos[1] = vStartPos[1];
    vTraceEndPos[2] = vStartPos[2];
    return result;
}

bool __cdecl Path_LookaheadPredictionTrace(path_t *pPath, float *vStartPos, float *vEndPos, int a4, int a5, int a6)
{
    int v6; // r6
    int wDodgeEntity; // r11
    int v9; // [sp+50h] [-20h] BYREF
    float v10[4]; // [sp+58h] [-18h] BYREF

    v6 = 8519697;
    if (pPath->wDodgeCount)
    {
        wDodgeEntity = pPath->wDodgeEntity;
        if (wDodgeEntity != 2175)
        {
            if (level.gentities[wDodgeEntity].actor)
            {
                v9 = pPath->wDodgeEntity;
                return Path_PredictionTraceCheckForEntities(vStartPos, vEndPos, &v9, 1, 2175, 8519697, v10) == PTR_SUCCESS;
            }
            v6 = 42074129;
        }
    }
    return Path_PredictionTrace(vStartPos, vEndPos, 2175, v6, v10, 18.0, a6, 1);
}

void __cdecl Path_UpdateLookaheadAmount(
    path_t *pPath,
    float *vStartPos,
    float *vLookaheadPos,
    int bReduceLookaheadAmount,
    double dist,
    int lookaheadNextNode,
    double maxLookaheadAmountIfReduce,
    __int16 a8)
{
    int v15; // r11
    int flags; // r10
    double v17; // fp0
    double v18; // fp0
    int v19; // r9
    int v20; // r11
    double fOrigLength; // fp0
    int v22; // r4
    double v23; // fp1
    double v24; // fp0
    __int16 wNegotiationStartNode; // r10
    int wPathLen; // r11
    double fCurrLength; // fp1
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
        wNegotiationStartNode = pPath->wNegotiationStartNode;
        pPath->lookaheadDir[2] = v24;
        pPath->fLookaheadDistToNextNode = dist;
        pPath->lookaheadNextNode = a8;
        if (wNegotiationStartNode > a8)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2758,
                0,
                "%s",
                "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode");
        if (pPath->lookaheadNextNode >= pPath->wPathLen)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2759,
                0,
                "%s",
                "pPath->lookaheadNextNode < pPath->wPathLen");
        if (pPath->pts[pPath->lookaheadNextNode].fOrigLength <= 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2760,
                0,
                "%s",
                "pPath->pts[pPath->lookaheadNextNode].fOrigLength > 0");
        if (pPath->fLookaheadDistToNextNode > (double)pPath->pts[pPath->lookaheadNextNode].fOrigLength)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2761,
                0,
                "%s",
                "pPath->fLookaheadDistToNextNode <= pPath->pts[pPath->lookaheadNextNode].fOrigLength");
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
        if (pPath->fLookaheadDistToNextNode != 0.0 && pPath->lookaheadNextNode >= pPath->wPathLen - 1)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2763,
                0,
                "%s",
                "!pPath->fLookaheadDistToNextNode || (pPath->lookaheadNextNode < pPath->wPathLen - 1)");
        if (pPath->lookaheadNextNode >= pPath->wPathLen)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                2764,
                0,
                "%s",
                "pPath->lookaheadNextNode < pPath->wPathLen");
        if (pPath->fLookaheadDistToNextNode != 0.0 && pPath->lookaheadNextNode >= pPath->wPathLen - 1)
        {
            v22 = 2765;
            goto LABEL_50;
        }
        return;
    }
    if (Path_LookaheadPredictionTrace(pPath, vStartPos, vLookaheadPos, bReduceLookaheadAmount, lookaheadNextNode, a8))
    {
        Path_IncreaseLookaheadAmount(pPath);
        goto LABEL_28;
    }
    if ((unsigned __int16)pPath->lookaheadNextNode >= 0x8000u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2732,
            0,
            "%s",
            "pPath->lookaheadNextNode >= 0");
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
    if (pPath->lookaheadNextNode >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            2738,
            0,
            "%s",
            "pPath->lookaheadNextNode < pPath->wPathLen");
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
    double totalArea,
    int a5,
    int a6)
{
    int wNegotiationStartNode; // r8
    int v11; // r11
    float *vCurrPoint; // r5

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3156, 0, "%s", "pPath");
    wNegotiationStartNode = pPath->wNegotiationStartNode;
    v11 = pPath->wPathLen - 1;
    pPath->flags |= 0x41u;
    if (v11 == wNegotiationStartNode)
        vCurrPoint = pPath->vCurrPoint;
    else
        vCurrPoint = pPath->pts[wNegotiationStartNode].vOrigPoint;
    Path_UpdateLookaheadAmount(
        pPath,
        vStartPos,
        vCurrPoint,
        bReduceLookaheadAmount,
        0.0,
        a6,
        totalArea,
        wNegotiationStartNode);
    _FP12 = (float)((float)totalArea - (float)32768.0);
    __asm { fsel      f0, f12, f31, f0 }
    _FP12 = (float)((float)_FP0 - pPath->fLookaheadAmount);
    __asm { fsel      f0, f12, f13, f0 }
    pPath->fLookaheadAmount = _FP0;
}

void __cdecl Path_CalcLookahead(path_t *pPath, float *vStartPos, int bReduceLookaheadAmount, int a4, int a5)
{
    int wPathLen; // r11
    double fCurrLength; // fp1
    double v10; // fp2
    const char *v11; // r3
    double fLookaheadAmount; // fp28
    double v13; // fp30
    int v14; // r27
    int v15; // r29
    const pathpoint_t *v16; // r31
    double DistToPathSegment; // fp29
    double fOrigLength; // fp31
    double v19; // fp2
    double v20; // fp30
    float *vOrigPoint; // r11
    double v22; // fp0
    double v23; // fp0
    double v24; // fp13
    float v25[16]; // [sp+60h] [-A0h] BYREF

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3237, 0, "%s", "pPath");
    if (pPath->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3238, 0, "%s", "pPath->wPathLen > 0");
    if ((unsigned __int16)pPath->wNegotiationStartNode >= 0x8000u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3239,
            0,
            "%s",
            "pPath->wNegotiationStartNode >= 0");
    if (pPath->wNegotiationStartNode >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3240,
            0,
            "%s",
            "pPath->wNegotiationStartNode < pPath->wPathLen");
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
    v15 = pPath->wPathLen - 2;
    if (v15 < pPath->wNegotiationStartNode)
    {
    LABEL_32:
        Path_CalcLookahead_Completed(pPath, vStartPos, bReduceLookaheadAmount, v13, a4, a5);
    }
    else
    {
        v16 = &pPath->pts[v15];
        while (1)
        {
            if ((COERCE_UNSIGNED_INT(v16->vOrigPoint[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(v16->vOrigPoint[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(v16->vOrigPoint[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    3259,
                    0,
                    "%s",
                    "!IS_NAN((pt->vOrigPoint)[0]) && !IS_NAN((pt->vOrigPoint)[1]) && !IS_NAN((pt->vOrigPoint)[2])");
            }
            DistToPathSegment = Path_GetDistToPathSegment(vStartPos, v16);
            if (v14)
                fOrigLength = pPath->fCurrLength;
            else
                fOrigLength = v16->fOrigLength;
            if (fOrigLength <= 0.0)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3263, 0, "%s", "fLength > 0");
            v13 = (float)((float)((float)fOrigLength * (float)DistToPathSegment) + (float)v13);
            if (pPath->minLookAheadNodes == 2)
                PathCalcLookahead_CheckMinLookaheadNodes(pPath, v16, v15);
            if (pPath->minLookAheadNodes + v15 <= pPath->wPathLen - 2 && v13 >= fLookaheadAmount)
                break;
            --v15;
            --v16;
            v14 = 0;
            if (v15 < pPath->wNegotiationStartNode)
                goto LABEL_32;
        }
        v19 = v16->fOrigLength;
        v20 = (float)((float)((float)v13 - (float)fLookaheadAmount) / (float)DistToPathSegment);
        if (fOrigLength > v19)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3281,
                0,
                (const char *)HIDWORD(fOrigLength),
                LODWORD(fOrigLength),
                LODWORD(v19));
        if (v20 > fOrigLength)
            v20 = fOrigLength;
        vOrigPoint = v16[1].vOrigPoint;
        v22 = (float)((float)(v16->fDir2D[1] * (float)-v20) + v16->vOrigPoint[1]);
        v25[0] = (float)((float)-v20 * v16->fDir2D[0]) + v16->vOrigPoint[0];
        v25[1] = v22;
        if (v14)
            vOrigPoint = pPath->vCurrPoint;
        v23 = v16->vOrigPoint[2];
        v24 = (float)(v16->vOrigPoint[2] - vOrigPoint[2]);
        pPath->flags &= 0xFFFFFFBE;
        v25[2] = -(float)((float)((float)((float)v24 / (float)fOrigLength) * (float)v20) - (float)v23);
        Path_UpdateLookaheadAmount(pPath, vStartPos, v25, bReduceLookaheadAmount, v20, a5, fLookaheadAmount, v15);
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
    int bAllowNegotiationLinks)
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
    int bAllowNegotiationLinks)
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
    int bReduceLookaheadAmount,
    int bTrimAmount,
    int bAllowBacktrack)
{
    int v10; // r7
    int v11; // r6
    int wPathLen; // r11
    double fCurrLength; // fp1
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
        Path_CalcLookahead(pPath, vStartPos, bReduceLookaheadAmount, v11, v10);
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
    Path_CalcLookahead(pPath, vStartPos, bReduceLookaheadAmount, v21, v20);
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
    double fCurrLength; // fp1
    double v6; // fp2
    const char *v7; // r3
    double fLookaheadAmount; // fp21
    double v9; // fp23
    double v10; // fp28
    double v11; // fp30
    double v12; // fp0
    double v13; // fp1
    double v14; // fp22
    int v15; // r29
    int v16; // r10
    double v17; // fp24
    double v18; // fp25
    float *fDir2D; // r31
    float *v20; // r26
    double v21; // fp11
    double v22; // fp12
    double v23; // fp26
    double v24; // fp29
    double v25; // fp31
    double v26; // fp31
    double v27; // fp0
    float *v28; // r4
    path_t *v29; // r3
    float v30; // [sp+50h] [-D0h]
    float v31; // [sp+50h] [-D0h]
    float v32; // [sp+50h] [-D0h]
    float v33; // [sp+58h] [-C8h] BYREF
    float v34; // [sp+5Ch] [-C4h]
    float v35; // [sp+60h] [-C0h] BYREF
    float v36; // [sp+64h] [-BCh]

    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 325, 0, "%s", "pPath");
    if (pPath->wOrigPathLen != pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            326,
            0,
            "%s",
            "pPath->wOrigPathLen == pPath->wPathLen");
    if ((unsigned __int16)pPath->wNegotiationStartNode >= 0x8000u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            327,
            0,
            "%s",
            "pPath->wNegotiationStartNode >= 0");
    if (pPath->wNegotiationStartNode >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            328,
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
                329,
                0,
                "%s\n\t%s",
                "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                v7);
        }
    }
    if (pPath->fLookaheadDist == 0.0)
        goto LABEL_59;
    fLookaheadAmount = pPath->fLookaheadAmount;
    v9 = 0.0;
    if (fLookaheadAmount <= 0.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            339,
            0,
            "%s\n\t(amount) = %g",
            HIDWORD(fLookaheadAmount),
            LODWORD(fLookaheadAmount));
    v10 = pPath->lookaheadDir[0];
    v11 = pPath->lookaheadDir[1];
    if (v10 == 0.0 && v11 == 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 341, 0, "%s", "vDir[0] || vDir[1]");
    v12 = (float)(pPath->vCurrPoint[1] - vStartPos[1]);
    v35 = pPath->vCurrPoint[0] - *vStartPos;
    v36 = v12;
    v13 = Vec2Normalize(&v35);
    v14 = 0.0;
    v15 = pPath->wPathLen - 2;
    v16 = 1;
    v17 = (float)((float)(v36 * (float)v11) + (float)(v35 * (float)v10));
    v18 = (float)((float)((float)(v36 * (float)v10) - (float)(v35 * (float)v11)) * (float)v13);
    if (v15 < pPath->wNegotiationStartNode)
    {
    LABEL_59:
        v28 = vStartPos;
        v29 = pPath;
    LABEL_60:
        Path_SetLookaheadToStart(v29, v28, 0);
    }
    else
    {
        fDir2D = pPath->pts[v15].fDir2D;
        while (1)
        {
            v20 = fDir2D - 3;
            v21 = (float)(*(fDir2D - 3) - *vStartPos);
            v33 = *(fDir2D - 3) - *vStartPos;
            v22 = (float)(*(fDir2D - 2) - vStartPos[1]);
            v34 = *(fDir2D - 2) - vStartPos[1];
            if (v16)
                v23 = pPath->fCurrLength;
            else
                v23 = fDir2D[2];
            if (*fDir2D == 0.0 && fDir2D[1] == 0.0)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    356,
                    0,
                    "%s",
                    "pt->fDir2D[0] || pt->fDir2D[1]");
                v22 = v34;
                v21 = v33;
            }
            v24 = __fabs((float)((float)(*fDir2D * (float)v22) - (float)(fDir2D[1] * (float)v21)));
            v25 = (float)((float)((float)v22 * (float)v10) - (float)((float)v21 * (float)v11));
            if ((float)((float)((float)(*fDir2D * (float)v11) - (float)(fDir2D[1] * (float)v10))
                * (float)((float)(*fDir2D * (float)v22) - (float)(fDir2D[1] * (float)v21))) > 0.0
                && (float)((float)((float)((float)v22 * (float)v10) - (float)((float)v21 * (float)v11)) * (float)v18) < 0.0)
            {
                Path_UpdateLookahead(pPath, vStartPos, 0, 1, 1);
                return;
            }
            v30 = v23;
            if ((LODWORD(v30) & 0x7F800000) == 0x7F800000)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 367, 0, "%s", "!IS_NAN(fLength)");
            if (v23 <= 0.0)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 368, 0, "%s", "fLength > 0");
            v9 = (float)((float)((float)v24 * (float)v23) + (float)v9);
            if (v9 >= fLookaheadAmount)
                break;
            v18 = v25;
            Vec2Normalize(&v33);
            if ((float)((float)(v34 * (float)v11) + (float)(v33 * (float)v10)) >= v17)
            {
                v17 = (float)((float)(v34 * (float)v11) + (float)(v33 * (float)v10));
                v14 = v9;
            }
            --v15;
            fDir2D -= 7;
            v16 = 0;
            if (v15 < pPath->wNegotiationStartNode)
            {
                if (v14 == v9)
                    goto LABEL_59;
                Path_SetLookaheadToStart(pPath, vStartPos, 1);
                if ((pPath->flags & 2) == 0 && pPath->fLookaheadAmount > v14)
                {
                    if (v14 < 64.0)
                        v14 = 64.0;
                    goto LABEL_41;
                }
                return;
            }
        }
        v31 = v24;
        if ((LODWORD(v31) & 0x7F800000) == 0x7F800000)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 372, 0, "%s", "!IS_NAN(height)");
        if (v24 == 0.0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 373, 0, "%s", "height");
        v32 = v23;
        if ((LODWORD(v32) & 0x7F800000) == 0x7F800000)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 374, 0, "%s", "!IS_NAN(fLength)");
        if (v23 <= 0.0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 375, 0, "%s", "fLength > 0");
        v26 = (float)((float)((float)v9 - (float)fLookaheadAmount) / (float)v24);
        if (*fDir2D == 0.0 && v20[4] == 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                379,
                0,
                "%s",
                "pt->fDir2D[0] || pt->fDir2D[1]");
        v27 = (float)((float)((float)(v20[4] * (float)-v26) + v20[1]) - vStartPos[1]);
        v33 = (float)((float)(*fDir2D * (float)-v26) + *(fDir2D - 3)) - *vStartPos;
        v34 = v27;
        Vec2Normalize(&v33);
        v28 = vStartPos;
        v29 = pPath;
        if ((float)((float)(v34 * (float)v11) + (float)(v33 * (float)v10)) >= v17)
            goto LABEL_60;
        Path_SetLookaheadToStart(pPath, vStartPos, 1);
        if ((pPath->flags & 2) == 0 && pPath->fLookaheadAmount > v14)
        {
            if (v14 >= 64.0)
            {
            LABEL_41:
                pPath->fLookaheadAmount = v14;
                return;
            }
            pPath->fLookaheadAmount = 64.0;
        }
    }
}

int __cdecl Path_GeneratePath(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    float *vGoalPos,
    pathnode_t *pNodeFrom,
    pathnode_t *pNodeTo,
    int bIncludeGoalPos,
    int bAllowNegotiationLinks)
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
    double fOrigLength; // fp0
    double fLookaheadAmount; // fp0
    double v35; // fp0
    int v36; // r11
    double fCurrLength; // fp1
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
            LOWORD(v20) = 0;
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
    _FP12 = (float)(pPath->fLookaheadAmount - (float)1024.0);
    __asm { fsel      f0, f12, f0, f13 }
    pPath->fLookaheadAmount = _FP0;
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
    int bCheckLookahead,
    int a11,
    int a12,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28,
    int a29,
    int a30)
{
    int v30; // r28
    int v38; // r24
    int v39; // r11
    int wDodgeCount; // r11
    int v41; // r27
    int v42; // r29
    pathpoint_t *v43; // r31
    int v44; // r10
    __int16 lookaheadNextNode; // r11
    unsigned int wNegotiationStartNode; // r10
    int v48; // r28
    double v49; // fp0
    PredictionTraceResult v50; // r3
    double v51; // fp0
    pathpoint_t *v52; // r31
    unsigned int v53; // r11
    path_t *v54; // r31
    int v55; // r11
    int v56; // r29
    double fLookaheadDistToNextNode; // fp31
    pathpoint_t *v58; // r29
    double v59; // fp0
    double v60; // fp13
    double fOrigLength; // fp12
    bool v62; // cr58
    int v63; // r11
    float *vOrigPoint; // r4
    float *v65; // r3
    double v66; // fp13
    unsigned int v67; // r11
    double v68; // fp0
    unsigned int v69; // r11
    double v70; // fp0
    double v71; // fp13
    double v72; // fp0
    int flags; // r10
    int wPathLen; // r11
    double fCurrLength; // fp1
    double v76; // fp2
    const char *v77; // r3
    int *v78; // r11
    pathpoint_t *v79[2]; // [sp+50h] [-100h] BYREF
    float v80; // [sp+58h] [-F8h] BYREF
    float v81; // [sp+5Ch] [-F4h]
    float v82; // [sp+60h] [-F0h] BYREF
    float v83; // [sp+64h] [-ECh]
    float v84; // [sp+68h] [-E8h]
    float v85; // [sp+70h] [-E0h] BYREF
    float v86; // [sp+74h] [-DCh]
    float v87; // [sp+78h] [-D8h]
    float v88; // [sp+80h] [-D0h] BYREF
    float v89; // [sp+84h] [-CCh]
    float v90; // [sp+88h] [-C8h]
    float v91[6]; // [sp+90h] [-C0h] BYREF

    v30 = startIndex;
    a20 = startIndex;
    if (!pPath)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3789, 0, "%s", "pPath");
    if (pPath->wPathLen <= 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3790, 0, "%s", "pPath->wPathLen > 0");
    if (*vDodgeStart == *vDodgeEnd && vDodgeStart[1] == vDodgeEnd[1])
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3791,
            0,
            "%s",
            "!(vDodgeStart[0] == vDodgeEnd[0] && vDodgeStart[1] == vDodgeEnd[1])");
    if (v30 > pPath->lookaheadNextNode)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3792,
            0,
            "%s",
            "startIndex <= pPath->lookaheadNextNode");
    if (pPath->lookaheadNextNode >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3793,
            0,
            "%s",
            "pPath->lookaheadNextNode < pPath->wPathLen");
    v38 = a28;
    Path_PredictionTraceCheckForEntities(vOrg, vDodgeStart, entities, entityCount, entityIgnore, a28, &v85);
    Path_PredictionTraceCheckForEntities(&v85, vDodgeEnd, entities, entityCount, entityIgnore, v38, &v82);
    v39 = ai_showDodge->current.color[0];
    v79[0] = &pPath->pts[v30];
    if (v39)
    {
        G_DebugLineWithDuration(vDodgeStart, &v85, colorYellow, 0, 25);
        G_DebugLineWithDuration(&v85, &v82, colorYellow, 0, 25);
        G_DebugLineWithDuration(&v82, pPath->pts[v30].vOrigPoint, colorYellow, 0, 25);
    }
    if (v30 < pPath->wNegotiationStartNode)
        goto LABEL_103;
    if (Path_PredictionTraceCheckForEntities(
        &v82,
        pPath->pts[v30].vOrigPoint,
        entities,
        entityCount,
        entityIgnore,
        v38,
        v91))
    {
        wDodgeCount = pPath->wDodgeCount;
        v41 = pPath->wPathLen - 2;
        if (wDodgeCount > 0)
            v41 -= wDodgeCount;
        if (v30 >= v41)
            goto LABEL_103;
        v42 = v30 + 1;
        v43 = &pPath->pts[v30 + 1];
        do
        {
            ++v43;
            ++v30;
            ++v42;
        } while (Vec2DistanceSq(v43->vOrigPoint, vOrg) >= 1406.25 && v42 < v41);
        a20 = v30;
        v44 = ai_showDodge->current.color[0];
        v79[0] = &pPath->pts[v30];
        if (v44)
            G_DebugLineWithDuration(&v82, pPath->pts[v30].vOrigPoint, colorMagenta, 0, 25);
        if (Path_PredictionTraceCheckForEntities(
            &v82,
            pPath->pts[v30].vOrigPoint,
            entities,
            entityCount,
            entityIgnore,
            v38,
            v91))
        {
        LABEL_103:
            if (a30)
            {
                v80 = v82 - v85;
                v81 = v83 - v86;
                if (Vec2Normalize(&v80) > 15.0)
                {
                    v82 = -(float)((float)(v80 * (float)15.0) - v82);
                    v83 = -(float)((float)(v81 * (float)15.0) - v83);
                }
                else
                {
                    v82 = v85;
                    v83 = v86;
                    v84 = v87;
                }
                lookaheadNextNode = pPath->lookaheadNextNode;
                wNegotiationStartNode = (unsigned __int16)pPath->wNegotiationStartNode;
                v48 = lookaheadNextNode;
                a20 = lookaheadNextNode;
                if (wNegotiationStartNode >= 0x8000)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                        3858,
                        0,
                        "%s",
                        "pPath->wNegotiationStartNode >= 0");
                if (pPath->wNegotiationStartNode > v48)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                        3859,
                        0,
                        "%s",
                        "pPath->wNegotiationStartNode <= startIndex");
                if (v48 >= pPath->wPathLen - 1)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                        3860,
                        0,
                        "%s",
                        "startIndex < pPath->wPathLen - 1");
                v49 = (float)((float)(pPath->lookaheadDir[1] * pPath->fLookaheadDist) + vOrg[1]);
                v88 = (float)(pPath->lookaheadDir[0] * pPath->fLookaheadDist) + *vOrg;
                v89 = v49;
                v90 = pPath->pts[v48].vOrigPoint[2];
                v79[0] = &pPath->pts[v48];
                if (ai_showDodge->current.enabled)
                    G_DebugLineWithDuration(&v82, &v88, colorRed, 0, 25);
                v50 = Path_PredictionTraceCheckForEntities(&v82, &v88, entities, entityCount, entityIgnore, v38, v91);
                if (v50 != PTR_HIT_WORLD)
                {
                    if (v50 == PTR_HIT_ENTITY)
                    {
                        v51 = (float)((float)(pPath->lookaheadDir[1]
                            * (float)((float)((float)(pPath->lookaheadDir[1] * (float)(v83 - vOrg[1]))
                                + (float)(pPath->lookaheadDir[0] * (float)(v82 - *vOrg)))
                                + (float)15.0))
                            + vOrg[1]);
                        v88 = (float)((float)((float)((float)(pPath->lookaheadDir[1] * (float)(v83 - vOrg[1]))
                            + (float)(pPath->lookaheadDir[0] * (float)(v82 - *vOrg)))
                            + (float)15.0)
                            * pPath->lookaheadDir[0])
                            + *vOrg;
                        v89 = v51;
                        if (ai_showDodge->current.enabled)
                            G_DebugLineWithDuration(&v82, &v88, colorCyan, 0, 25);
                        if (Path_PredictionTraceCheckForEntities(&v82, &v88, entities, entityCount, entityIgnore, v38, v91) == PTR_SUCCESS)
                        {
                            if (pPath->wNegotiationStartNode > v48)
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                                    3891,
                                    0,
                                    "%s",
                                    "pPath->wNegotiationStartNode <= startIndex");
                            if (ai_showDodge->current.enabled)
                                G_DebugLineWithDuration(&v88, pPath->pts[v48].vOrigPoint, colorCyan, 0, 25);
                            if (Path_PredictionTraceCheckForEntities(
                                &v88,
                                pPath->pts[v48].vOrigPoint,
                                entities,
                                entityCount,
                                entityIgnore,
                                v38,
                                v91) == PTR_SUCCESS)
                            {
                                Path_CheckNodeCountForDodge(pPath, 3, v79, &a20);
                                v52 = v79[0];
                                *(float *)v79 = Path_GetPathDir(v79[0]->fDir2D, &v88, v79[0]->vOrigPoint);
                                v53 = (unsigned int)v79[0];
                                v52->fOrigLength = *(float *)v79;
                                if ((v53 & 0x7F800000) == 0x7F800000)
                                    MyAssertHandler(
                                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                                        3904,
                                        0,
                                        "%s",
                                        "!IS_NAN(pt->fOrigLength)");
                                v54 = (path_t *)&v52[1];
                                v55 = a20;
                                v54->pts[0].vOrigPoint[0] = v88;
                                v56 = v55 + 1;
                                v54->pts[0].vOrigPoint[1] = v89;
                                v54->pts[0].vOrigPoint[2] = v90;
                                v54->pts[0].iNodeNum = -1;
                                Path_DodgeDrawRaisedLine(&v54[-1].fCurrLength, (float *)v54, colorBlue);
                                goto done;
                            }
                        }
                    }
                    else
                    {
                        if (ai_showDodge->current.enabled)
                            G_DebugLineWithDuration(&v88, pPath->pts[v48].vOrigPoint, colorMdGrey, 0, 25);
                        if (Path_PredictionTraceCheckForEntities(
                            &v88,
                            pPath->pts[v48].vOrigPoint,
                            entities,
                            entityCount,
                            entityIgnore,
                            v38,
                            v91) == PTR_SUCCESS)
                        {
                            Path_CheckNodeCountForDodge(pPath, 3, v79, &a20);
                            fLookaheadDistToNextNode = pPath->fLookaheadDistToNextNode;
                            if (fLookaheadDistToNextNode <= 0.0)
                                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 3930, 0, "%s", "dist > 0");
                            v58 = v79[0];
                            v79[0] = (pathpoint_t *)LODWORD(v79[0]->fOrigLength);
                            if (((int)v79[0] & 0x7F800000) == 0x7F800000)
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                                    3932,
                                    0,
                                    "%s",
                                    "!IS_NAN(pt->fOrigLength)");
                            if (v58->fOrigLength <= 0.0)
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                                    3933,
                                    0,
                                    "%s",
                                    "pt->fOrigLength > 0");
                            if (v58->fDir2D[0] == 0.0 && v58->fDir2D[1] == 0.0)
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                                    3934,
                                    0,
                                    "%s",
                                    "pt->fDir2D[0] || pt->fDir2D[1]");
                            if (fLookaheadDistToNextNode > v58->fOrigLength)
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                                    3935,
                                    0,
                                    "%s",
                                    "dist <= pt->fOrigLength");
                            v54 = (path_t *)&v58[1];
                            v58[1].vOrigPoint[0] = -(float)((float)(v58->fDir2D[0] * (float)fLookaheadDistToNextNode)
                                - v58->vOrigPoint[0]);
                            v58[1].vOrigPoint[1] = -(float)((float)(v58->fDir2D[1] * (float)fLookaheadDistToNextNode)
                                - v58->vOrigPoint[1]);
                            v59 = v58->vOrigPoint[2];
                            v60 = (float)(v58->vOrigPoint[2] - v58[1].vOrigPoint[2]);
                            fOrigLength = v58->fOrigLength;
                            *(float *)v79 = fLookaheadDistToNextNode;
                            v62 = ((int)v79[0] & 0x7F800000) == 2139095040;
                            v58[1].vOrigPoint[2] = -(float)((float)((float)((float)v60 / (float)fOrigLength)
                                * (float)fLookaheadDistToNextNode)
                                - (float)v59);
                            v58->fOrigLength = fLookaheadDistToNextNode;
                            if (v62)
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                                    3942,
                                    0,
                                    "%s",
                                    "!IS_NAN(pt->fOrigLength)");
                            if (v58->fOrigLength <= 0.0)
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                                    3943,
                                    0,
                                    "%s",
                                    "pt->fOrigLength > 0");
                            v63 = a20;
                            vOrigPoint = v58[1].vOrigPoint;
                            v65 = (float *)v58;
                            pPath->fLookaheadDistToNextNode = 0.0;
                            v56 = v63 + 1;
                            pPath->lookaheadNextNode = v63 + 1;
                            v54->pts[0].iNodeNum = -1;
                            Path_DodgeDrawRaisedLine(v65, vOrigPoint, colorGreen);
                            goto done;
                        }
                    }
                }
            }
            return 0;
        }
    }
    Path_CheckNodeCountForDodge(pPath, 2, v79, &a20);
    v56 = a20;
    v54 = (path_t *)v79[0];
done:
    if (v54 != (path_t *)&pPath->pts[v56])
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            3959,
            0,
            "%s",
            "pt == &pPath->pts[startIndex]");
    v66 = v82;
    pPath->wDodgeCount = 0;
    if (v54->pts[0].vOrigPoint[0] != v66 || v54->pts[0].vOrigPoint[1] != v83)
    {
        *(float *)v79 = Path_GetPathDir(v54->pts[0].fDir2D, &v82, (const float *)v54);
        v67 = (unsigned int)v79[0];
        v54->pts[0].fOrigLength = *(float *)v79;
        if ((v67 & 0x7F800000) == 0x7F800000)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3965,
                0,
                "%s",
                "!IS_NAN(pt->fOrigLength)");
        v54 = (path_t *)((char *)v54 + 28);
        v68 = v82;
        LOWORD(v56) = v56 + 1;
        ++pPath->wDodgeCount;
        v54->pts[0].vOrigPoint[0] = v68;
        v54->pts[0].vOrigPoint[1] = v83;
        v54->pts[0].vOrigPoint[2] = v84;
        v54->pts[0].iNodeNum = -1;
        Path_DodgeDrawRaisedLine(&v54[-1].fCurrLength, (float *)v54, colorLtGreen);
    }
    if (v85 != v82 || v86 != v83)
    {
        *(float *)v79 = Path_GetPathDir(v54->pts[0].fDir2D, &v85, &v82);
        v69 = (unsigned int)v79[0];
        v54->pts[0].fOrigLength = *(float *)v79;
        if ((v69 & 0x7F800000) == 0x7F800000)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                3982,
                0,
                "%s",
                "!IS_NAN(pt->fOrigLength)");
        v54 = (path_t *)((char *)v54 + 28);
        v70 = v85;
        LOWORD(v56) = v56 + 1;
        ++pPath->wDodgeCount;
        v54->pts[0].vOrigPoint[0] = v70;
        v54->pts[0].vOrigPoint[1] = v86;
        v54->pts[0].vOrigPoint[2] = v87;
        v54->pts[0].iNodeNum = -1;
        Path_DodgeDrawRaisedLine(&v54[-1].fCurrLength, (float *)v54, colorMdYellow);
    }
    v71 = v85;
    pPath->fCurrLength = v54[-1].pathEndAnimDistSq;
    pPath->vCurrPoint[0] = v71;
    v72 = v87;
    pPath->vCurrPoint[1] = v86;
    pPath->vCurrPoint[2] = v72;
    v54->pts[0].fOrigLength = 0.0;
    v54->pts[0].fDir2D[0] = 0.0;
    v54->pts[0].fDir2D[1] = 0.0;
    flags = pPath->flags;
    pPath->wPathLen = v56 + 1;
    pPath->wOrigPathLen = v56 + 1;
    pPath->flags = flags & 0xFFFFFDBC;
    Path_TransferLookahead(pPath, vOrg);
    if (pPath->wNegotiationStartNode > pPath->lookaheadNextNode)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            4010,
            0,
            "%s",
            "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode");
    if (pPath->lookaheadNextNode >= pPath->wPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            4011,
            0,
            "%s",
            "pPath->lookaheadNextNode < pPath->wPathLen");
    if (pPath->fLookaheadDistToNextNode > (double)pPath->pts[pPath->lookaheadNextNode].fOrigLength)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            4012,
            0,
            "%s",
            "pPath->fLookaheadDistToNextNode <= pPath->pts[pPath->lookaheadNextNode].fOrigLength");
    wPathLen = pPath->wPathLen;
    if (wPathLen > 1)
    {
        fCurrLength = pPath->fCurrLength;
        v76 = *((float *)&pPath->pts[wPathLen - 1] - 2);
        if (fCurrLength > v76)
        {
            v77 = va((const char *)HIDWORD(fCurrLength), LODWORD(fCurrLength), LODWORD(v76));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                4013,
                0,
                "%s\n\t%s",
                "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                v77);
        }
    }
    if (pPath->wPathLen)
    {
        if (pPath->wNegotiationStartNode)
        {
            v78 = (int *)&pPath->pts[pPath->wNegotiationStartNode];
            if (v78[6] < 0 || *(v78 - 1) < 0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                    4014,
                    0,
                    "%s",
                    "!pPath->wPathLen || !pPath->wNegotiationStartNode || (pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0 &"
                    "& pPath->pts[pPath->wNegotiationStartNode - 1].iNodeNum >= 0)");
        }
    }
    if (pPath->fLookaheadDistToNextNode != 0.0 && pPath->lookaheadNextNode >= pPath->wPathLen - 1)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            4015,
            0,
            "%s",
            "!pPath->fLookaheadDistToNextNode || (pPath->lookaheadNextNode < pPath->wPathLen - 1)");
    return 1;
}

pathnode_t *__cdecl Path_FindCloseNode(
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vGoalPos,
    int bAllowNegotiationLinks)
{
    CustomSearchInfo_FindCloseNode v9; // [sp+50h] [-50h] BYREF

    if (!pNodeFrom)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 827, 0, "%s", "pNodeFrom");
    if ((pNodeFrom->constant.spawnflags & 1) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            828,
            0,
            "%s",
            "(pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0");
    if (!vGoalPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 829, 0, "%s", "vGoalPos");
    v9.goalPos[0] = *vGoalPos;
    v9.goalPos[1] = vGoalPos[1];
    v9.closestNode = pNodeFrom;
    v9.goalPos[2] = vGoalPos[2];
    v9.closestDistSq = Vec2DistanceSq(pNodeFrom->constant.vOrigin, vGoalPos);
    Path_AStarAlgorithm<CustomSearchInfo_FindCloseNode>(
        0,
        eTeam,
        pNodeFrom->constant.vOrigin,
        pNodeFrom,
        vGoalPos,
        1,
        bAllowNegotiationLinks,
        &v9);
    return v9.closestNode;
}

int __cdecl Path_FindPathFromToWithWidth(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vStartPos,
    pathnode_t *pNodeTo,
    const float *vGoalPos,
    int bAllowNegotiationLinks,
    double width,
    float *perp,
    int a10,
    int a11,
    int a12,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28,
    float *a29)
{
    CustomSearchInfo_FindPathWithWidth v38; // [sp+50h] [-70h] BYREF

    if (!pNodeFrom)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 1058, 0, "%s", "pNodeFrom");
    if (!pNodeTo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 1059, 0, "%s", "pNodeTo");
    if ((pNodeFrom->constant.spawnflags & 1) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            1060,
            0,
            "%s",
            "(pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0");
    if ((pNodeTo->constant.spawnflags & 1) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            1061,
            0,
            "%s",
            "(pNodeTo->constant.spawnflags & PNF_DONTLINK) == 0");
    v38.width = width;
    v38.m_pNodeTo = pNodeTo;
    v38.perp[0] = *a29;
    v38.perp[1] = a29[1];
    pNodeFrom->transient.costFactor = 0.0;
    return Path_AStarAlgorithm<CustomSearchInfo_FindPathWithWidth>(
        pPath,
        eTeam,
        vStartPos,
        pNodeFrom,
        vGoalPos,
        1,
        bAllowNegotiationLinks,
        &v38);
}

int __cdecl Path_FindPathFromToNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vStartPos,
    pathnode_t *pNodeTo,
    const float *vGoalPos,
    float (*vNormal)[2],
    float *fDist,
    int iPlaneCount,
    int bAllowNegotiationLinks,
    int a11,
    int a12,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28,
    int a29,
    int a30)
{
    CustomSearchInfo_FindPathNotCrossPlanes v39[4]; // [sp+50h] [-80h] BYREF

    v39[0].negotiationOverlapCost = ai_pathNegotiationOverlapCost->current.value;
    if (!pNodeFrom)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 1168, 0, "%s", "pNodeFrom");
    if (!pNodeTo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 1169, 0, "%s", "pNodeTo");
    if ((pNodeFrom->constant.spawnflags & 1) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            1170,
            0,
            "%s",
            "(pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0");
    if ((pNodeTo->constant.spawnflags & 1) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            1171,
            0,
            "%s",
            "(pNodeTo->constant.spawnflags & PNF_DONTLINK) == 0");
    v39[0].startPos[0] = *vStartPos;
    v39[0].startPos[1] = vStartPos[1];
    v39[0].m_pNodeTo = pNodeTo;
    v39[0].startPos[2] = vStartPos[2];
    v39[0].m_vNormal = vNormal;
    v39[0].m_iPlaneCount = a28;
    v39[0].m_fDist = fDist;
    if (CustomSearchInfo_FindPathNotCrossPlanes::IgnoreNode(v39, pNodeFrom)
        || CustomSearchInfo_FindPathNotCrossPlanes::IgnoreNode(v39, pNodeTo))
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
            a30,
            v39);
    }
}

int __cdecl Path_FindPathFromAway(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    float *vAwayFromPos,
    double fDistAway,
    int bAllowNegotiationLinks,
    int a8)
{
    double v15; // fp13
    double v16; // fp10
    double v17; // fp11
    double v18; // fp12
    double v19; // fp9
    pathnode_t *m_pBestNode; // r29
    int Path; // r31
    CustomSearchInfo_FindPathAway v23; // [sp+50h] [-70h] BYREF

    v23.m_fBestScore = -1.0;
    v23.m_pBestNode = 0;
    if (!pNodeFrom)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 1276, 0, "%s", "pNodeFrom");
    v15 = vAwayFromPos[2];
    v16 = (float)(vAwayFromPos[2] - vStartPos[2]);
    v17 = *vStartPos;
    v23.m_vAwayFromPos[0] = *vAwayFromPos;
    v18 = vAwayFromPos[1];
    v19 = (float)(vAwayFromPos[1] - vStartPos[1]);
    v23.m_fDistAway = fDistAway;
    v23.m_fDistAwaySqrd = (float)fDistAway * (float)fDistAway;
    v23.m_vAwayFromPos[1] = v18;
    v23.m_vAwayFromPos[2] = v15;
    v23.m_fInitialDistAwaySq = (float)((float)v19 * (float)v19)
        + (float)((float)((float)(v23.m_vAwayFromPos[0] - (float)v17)
            * (float)(v23.m_vAwayFromPos[0] - (float)v17))
            + (float)((float)v16 * (float)v16));
    if (Path_AStarAlgorithm<CustomSearchInfo_FindPathAway>(pPath, eTeam, vStartPos, pNodeFrom, 0, 0, a8, &v23))
        return 1;
    m_pBestNode = v23.m_pBestNode;
    if (!v23.m_pBestNode)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 1287, 0, "%s", "info.m_pBestNode");
    if (m_pBestNode == pNodeFrom)
        return 0;
    if (!pPath)
        return 1;
    //Profile_Begin(231);
    Path = Path_GeneratePath(pPath, eTeam, vStartPos, 0, pNodeFrom, m_pBestNode, 0, a8);
    if (pPath->wPathLen > pPath->wOrigPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            1298,
            0,
            "%s",
            "pPath->wPathLen <= pPath->wOrigPathLen");
    if (!Path)
    {
        if (pPath->wOrigPathLen)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                1299,
                0,
                "%s",
                "success || !pPath->wOrigPathLen");
    }
    //Profile_EndInternal(0);
    return Path;
}

int __cdecl Path_FindPathFromAwayNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    float *vAwayFromPos,
    double fDistAway,
    float (*vNormal)[2],
    float *fDist,
    float *iPlaneCount,
    int bAllowNegotiationLinks,
    int a11,
    int a12,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28,
    int a29,
    int a30,
    int a31)
{
    pathnode_t *m_pBestNode; // r30
    int Path; // r31
    CustomSearchInfo_FindPathAwayNotCrossPlanes v42; // [sp+50h] [-90h] BYREF

    v42.m_fBestScore = -1.0;
    v42.m_pBestNode = 0;
    if (!pNodeFrom)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 1370, 0, "%s", "pNodeFrom");
    v42.m_vAwayFromPos[0] = *vAwayFromPos;
    v42.m_vAwayFromPos[1] = vAwayFromPos[1];
    v42.m_vNormal = (float (*)[2])fDist;
    v42.m_vAwayFromPos[2] = vAwayFromPos[2];
    v42.m_fDist = iPlaneCount;
    v42.m_fDistAway = fDistAway;
    v42.m_iPlaneCount = a29;
    v42.m_fDistAwaySqrd = (float)fDistAway * (float)fDistAway;
    if (CustomSearchInfo_FindPathAwayNotCrossPlanes::IgnoreNode(&v42, pNodeFrom))
        return 0;
    if (Path_AStarAlgorithm<CustomSearchInfo_FindPathAwayNotCrossPlanes>(
        pPath,
        eTeam,
        vStartPos,
        pNodeFrom,
        0,
        0,
        a31,
        &v42))
    {
        return 1;
    }
    m_pBestNode = v42.m_pBestNode;
    if (!v42.m_pBestNode)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 1385, 0, "%s", "info.m_pBestNode");
    if (m_pBestNode == pNodeFrom || Path_NodesVisible(m_pBestNode, pNodeFrom))
        return 0;
    if (!pPath)
        return 1;
    //Profile_Begin(231);
    Path = Path_GeneratePath(pPath, eTeam, vStartPos, 0, pNodeFrom, m_pBestNode, 0, a31);
    if (pPath->wPathLen > pPath->wOrigPathLen)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            1399,
            0,
            "%s",
            "pPath->wPathLen <= pPath->wOrigPathLen");
    if (!Path)
    {
        if (pPath->wOrigPathLen)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                1400,
                0,
                "%s",
                "success || !pPath->wOrigPathLen");
    }
    //Profile_EndInternal(0);
    return Path;
}

int __cdecl Path_FindPathInCylinderWithLOS(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    const actor_goal_s *goal,
    double fWithinDistSqrd,
    int bAllowNegotiationLinks,
    int a8)
{
    int *v15; // r6
    pathnode_t *v17; // r6
    _BYTE v18[16]; // [sp+50h] [-380h] BYREF
    CustomSearchInfo_FindPathInCylinderWithLOS v19; // [sp+60h] [-370h] BYREF
    pathsort_t v20[64]; // [sp+80h] [-350h] BYREF

    v19.negotiationOverlapCost = ai_pathNegotiationOverlapCost->current.value;
    v19.m_pNodeTo = Path_NearestNode(vGoalPos, v20, -2, 192.0, (int *)vGoalPos, (int)v18, (nearestNodeHeightCheck)64);
    if (!v19.m_pNodeTo)
        return 0;
    v17 = Path_NearestNode(vStartPos, v20, -2, 192.0, v15, (int)v18, (nearestNodeHeightCheck)64);
    if (!v17)
        return 0;
    v19.startPos[0] = *vStartPos;
    v19.startPos[1] = vStartPos[1];
    v19.startPos[2] = vStartPos[2];
    v19.m_fWithinDistSqrd = fWithinDistSqrd;
    v19.goal = goal;
    return Path_AStarAlgorithm<CustomSearchInfo_FindPathInCylinderWithLOS>(
        pPath,
        eTeam,
        vStartPos,
        v17,
        vGoalPos,
        0,
        a8,
        &v19);
}

int __cdecl Path_FindPathInCylinderWithLOSNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    const actor_goal_s *goal,
    double fWithinDistSqrd,
    float (*vNormal)[2],
    float *fDist,
    float *iPlaneCount,
    int bAllowNegotiationLinks,
    int a11,
    int a12,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28,
    int *a29,
    int a30,
    int a31)
{
    pathnode_t *v40; // r6
    nearestNodeHeightCheck v41; // [sp+8h] [-3F8h]
    _BYTE v42[16]; // [sp+60h] [-3A0h] BYREF
    CustomSearchInfo_FindPathInCylinderWithLOSNotCrossPlanes v43; // [sp+70h] [-390h] BYREF
    pathsort_t v44[64]; // [sp+A0h] [-360h] BYREF

    v43.negotiationOverlapCost = ai_pathNegotiationOverlapCost->current.value;
    v43.m_pNodeTo = Path_NearestNode(vGoalPos, v44, -2, 192.0, (int *)vGoalPos, (int)v42, (nearestNodeHeightCheck)64);
    if (!v43.m_pNodeTo)
        return 0;
    v40 = Path_NearestNodeNotCrossPlanes(
        vStartPos,
        v44,
        -2,
        192.0,
        (float (*)[2])0x40,
        fDist,
        (int)iPlaneCount,
        a29,
        (int)v42,
        v41);
    if (!v40)
        return 0;
    v43.startPos[0] = *vStartPos;
    v43.startPos[1] = vStartPos[1];
    v43.startPos[2] = vStartPos[2];
    v43.goal = goal;
    v43.m_fWithinDistSqrd = fWithinDistSqrd;
    v43.m_iPlaneCount = (int)a29;
    v43.m_vNormal = (float (*)[2])fDist;
    v43.m_fDist = iPlaneCount;
    return Path_AStarAlgorithm<CustomSearchInfo_FindPathInCylinderWithLOSNotCrossPlanes>(
        pPath,
        eTeam,
        vStartPos,
        v40,
        vGoalPos,
        0,
        a31,
        &v43);
}

pathnode_t *__cdecl Path_FindPathFromInCylinder(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    const float *vGoalPos,
    float *vOrigin,
    double fRadiusSqrd,
    double fHalfHeightSqrd,
    int bAllowNegotiationLinks,
    int a10,
    int a11,
    int a12,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28,
    int a29,
    int a30)
{
    unsigned __int16 spawnflags; // r10
    pathnode_t *result; // r3
    CustomSearchInfo_FindPathFromInCylinder v40; // [sp+50h] [-380h] BYREF
    _BYTE v41[12]; // [sp+74h] [-35Ch] BYREF
    pathsort_t v42[64]; // [sp+80h] [-350h] BYREF

    spawnflags = pNodeFrom->constant.spawnflags;
    v40.negotiationOverlapCost = ai_pathNegotiationOverlapCost->current.value;
    if ((spawnflags & 1) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            1611,
            0,
            "%s",
            "(pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0");
    result = Path_NearestNode(vGoalPos, v42, -2, 192.0, (int *)vStartPos, (int)v41, (nearestNodeHeightCheck)64);
    if (result)
    {
        v40.m_vOrigin[0] = *vOrigin;
        v40.m_vOrigin[1] = vOrigin[1];
        v40.startPos[0] = *vStartPos;
        v40.startPos[1] = vStartPos[1];
        v40.startPos[2] = vStartPos[2];
        v40.m_pNodeTo = result;
        v40.m_fRadiusSqrd = fRadiusSqrd;
        v40.m_fHalfHeightSqrd = fHalfHeightSqrd;
        return (pathnode_t *)Path_AStarAlgorithm<CustomSearchInfo_FindPathFromInCylinder>(
            pPath,
            eTeam,
            vStartPos,
            pNodeFrom,
            vGoalPos,
            1,
            a30,
            &v40);
    }
    return result;
}

int __cdecl Path_FindPathFromInCylinderNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vStartPos,
    const float *vGoalPos,
    float *vOrigin,
    double fRadiusSqrd,
    double fHalfHeightSqrd,
    float (*vNormal)[2],
    float *fDist,
    int iPlaneCount,
    int bAllowNegotiationLinks,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28,
    int a29,
    float *a30,
    int a31,
    float *a32,
    int a33,
    int *a34,
    int a35,
    int a36)
{
    unsigned __int16 spawnflags; // r10
    pathnode_t *v45; // r11
    nearestNodeHeightCheck v47; // [sp+8h] [-3F8h]
    CustomSearchInfo_FindPathFromInCylinderNotCrossPlanes v48; // [sp+60h] [-3A0h] BYREF
    _BYTE v49[16]; // [sp+90h] [-370h] BYREF
    pathsort_t v50[64]; // [sp+A0h] [-360h] BYREF

    spawnflags = pNodeFrom->constant.spawnflags;
    v48.negotiationOverlapCost = ai_pathNegotiationOverlapCost->current.value;
    if ((spawnflags & 1) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            1682,
            0,
            "%s",
            "(pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0");
    v45 = Path_NearestNodeNotCrossPlanes(vGoalPos, v50, -2, 192.0, 0, a30, (int)a32, a34, (int)v49, v47);
    if (!v45)
        return 0;
    v48.m_vOrigin[0] = *vOrigin;
    v48.m_vOrigin[1] = vOrigin[1];
    v48.m_pNodeTo = v45;
    v48.m_iPlaneCount = (int)a34;
    v48.startPos[0] = *vStartPos;
    v48.m_vNormal = (float (*)[2])a30;
    v48.m_fDist = a32;
    v48.startPos[1] = vStartPos[1];
    v48.startPos[2] = vStartPos[2];
    v48.m_fRadiusSqrd = fRadiusSqrd;
    v48.m_fHalfHeightSqrd = fHalfHeightSqrd;
    if (CustomSearchInfo_FindPathFromInCylinderNotCrossPlanes::IgnoreNode(&v48, pNodeFrom))
        return 0;
    else
        return Path_AStarAlgorithm<CustomSearchInfo_FindPathFromInCylinderNotCrossPlanes>(
            pPath,
            eTeam,
            vStartPos,
            pNodeFrom,
            vGoalPos,
            1,
            a36,
            &v48);
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

int __cdecl Path_FindPathGetCloseAsPossible(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    float *vStartPos,
    pathnode_t *pNodeTo,
    const float *vGoalPos,
    int bAllowNegotiationLinks)
{
    pathnode_t *m_pBestNode; // r31
    CustomSearchInfo_FindPathClosestPossible v16; // [sp+50h] [-60h] BYREF

    v16.negotiationOverlapCost = ai_pathNegotiationOverlapCost->current.value;
    v16.m_fBestScore = 3.4028235e38;
    v16.m_pBestNode = 0;
    if (!pNodeTo)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 1862, 0, "%s", "pNodeTo");
    if (!pNodeFrom)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 1863, 0, "%s", "pNodeFrom");
    if ((pNodeFrom->constant.spawnflags & 1) != 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
            1864,
            0,
            "%s",
            "(pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0");
    v16.m_pNodeTo = pNodeTo;
    if (Path_AStarAlgorithm<CustomSearchInfo_FindPathClosestPossible>(
        pPath,
        eTeam,
        vStartPos,
        pNodeFrom,
        vGoalPos,
        1,
        bAllowNegotiationLinks,
        &v16))
    {
        return 1;
    }
    m_pBestNode = v16.m_pBestNode;
    if (!v16.m_pBestNode)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp", 1872, 0, "%s", "info.m_pBestNode");
    //Profile_Begin(231);
    if (Path_GeneratePath(
        pPath,
        eTeam,
        vStartPos,
        m_pBestNode->constant.vOrigin,
        pNodeFrom,
        m_pBestNode,
        1,
        bAllowNegotiationLinks))
    {
        if (pPath->wPathLen > pPath->wOrigPathLen)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_navigation.cpp",
                1877,
                0,
                "%s",
                "pPath->wPathLen <= pPath->wOrigPathLen");
        //Profile_EndInternal(0);
        return 1;
    }
    //Profile_EndInternal(0);
    return 0;
}

int __cdecl Path_FindPathWithWidth(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    int bAllowNegotiationLinks,
    double width,
    float *perp,
    float *a8)
{
    pathnode_t *v15; // r30
    int *v16; // r6
    float *v18; // r10
    pathnode_t *v19; // r5
    int v20; // [sp+8h] [-3B8h]
    int v21; // [sp+Ch] [-3B4h]
    int v22; // [sp+10h] [-3B0h]
    int v23; // [sp+14h] [-3ACh]
    int v24; // [sp+18h] [-3A8h]
    int v25; // [sp+1Ch] [-3A4h]
    int v26; // [sp+20h] [-3A0h]
    int v27; // [sp+24h] [-39Ch]
    int v28; // [sp+28h] [-398h]
    int v29; // [sp+2Ch] [-394h]
    int v30; // [sp+30h] [-390h]
    int v31; // [sp+34h] [-38Ch]
    int v32; // [sp+38h] [-388h]
    int v33; // [sp+3Ch] [-384h]
    int v34; // [sp+40h] [-380h]
    int v35; // [sp+44h] [-37Ch]
    int v36; // [sp+48h] [-378h]
    int v37; // [sp+4Ch] [-374h]
    int v38; // [sp+50h] [-370h]
    _BYTE v39[16]; // [sp+60h] [-360h] BYREF
    pathsort_t v40[64]; // [sp+70h] [-350h] BYREF

    v15 = Path_NearestNode(vGoalPos, v40, -2, 192.0, (int *)vGoalPos, (int)v39, (nearestNodeHeightCheck)64);
    if (v15 && (v19 = Path_NearestNode(vStartPos, v40, -2, 192.0, v16, (int)v39, (nearestNodeHeightCheck)64)) != 0)
        return Path_FindPathFromToWithWidth(
            pPath,
            eTeam,
            v19,
            vStartPos,
            v15,
            vGoalPos,
            bAllowNegotiationLinks,
            width,
            v18,
            v20,
            v21,
            v22,
            v23,
            v24,
            v25,
            v26,
            v27,
            v28,
            v29,
            v30,
            v31,
            v32,
            v33,
            v34,
            v35,
            v36,
            v37,
            v38,
            a8);
    else
        return 0;
}

int __cdecl Path_FindPathNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    float *vGoalPos,
    float (*vNormal)[2],
    float *fDist,
    int *iPlaneCount,
    int bAllowNegotiationLinks)
{
    pathnode_t *v16; // r26
    float *v17; // r6
    pathnode_t *v19; // r5
    nearestNodeHeightCheck v20; // [sp+8h] [-3D8h]
    nearestNodeHeightCheck v21; // [sp+8h] [-3D8h]
    int v22; // [sp+8h] [-3D8h]
    int v23; // [sp+Ch] [-3D4h]
    int v24; // [sp+10h] [-3D0h]
    int v25; // [sp+14h] [-3CCh]
    int v26; // [sp+18h] [-3C8h]
    int v27; // [sp+1Ch] [-3C4h]
    int v28; // [sp+20h] [-3C0h]
    int v29; // [sp+24h] [-3BCh]
    int v30; // [sp+28h] [-3B8h]
    int v31; // [sp+2Ch] [-3B4h]
    int v32; // [sp+30h] [-3B0h]
    int v33; // [sp+34h] [-3ACh]
    int v34; // [sp+38h] [-3A8h]
    int v35; // [sp+3Ch] [-3A4h]
    int v36; // [sp+40h] [-3A0h]
    int v37; // [sp+44h] [-39Ch]
    int v38; // [sp+48h] [-398h]
    int v39; // [sp+4Ch] [-394h]
    int v40; // [sp+50h] [-390h]
    int v41; // [sp+58h] [-388h]
    _BYTE v42[16]; // [sp+60h] [-380h] BYREF
    pathsort_t v43[64]; // [sp+70h] [-370h] BYREF

    v16 = Path_NearestNodeNotCrossPlanes(
        vGoalPos,
        v43,
        -2,
        192.0,
        (float (*)[2])vGoalPos,
        (float *)vNormal,
        (int)fDist,
        iPlaneCount,
        (int)v42,
        v20);
    if (v16
        && (v19 = Path_NearestNodeNotCrossPlanes(
            vStartPos,
            v43,
            -2,
            192.0,
            (float (*)[2])v17,
            (float *)vNormal,
            (int)fDist,
            iPlaneCount,
            (int)v42,
            v21)) != 0)
    {
        return Path_FindPathFromToNotCrossPlanes(
            pPath,
            eTeam,
            v19,
            vStartPos,
            v16,
            vGoalPos,
            vNormal,
            fDist,
            v22,
            v23,
            v24,
            v25,
            v26,
            v27,
            v28,
            v29,
            v30,
            v31,
            v32,
            v33,
            v34,
            v35,
            v36,
            v37,
            v38,
            v39,
            v40,
            (int)iPlaneCount,
            v41,
            bAllowNegotiationLinks);
    }
    else
    {
        return 0;
    }
}

pathnode_t *__cdecl Path_FindPathFromNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    pathnode_t *pNodeFrom,
    const float *vStartPos,
    const float *vGoalPos,
    float (*vNormal)[2],
    float *fDist,
    int *iPlaneCount,
    int bAllowNegotiationLinks,
    int a10,
    int a11,
    int a12,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28)
{
    pathnode_t *result; // r3
    nearestNodeHeightCheck v37; // [sp+8h] [-3B8h]
    int v38; // [sp+8h] [-3B8h]
    int v39; // [sp+Ch] [-3B4h]
    int v40; // [sp+10h] [-3B0h]
    int v41; // [sp+14h] [-3ACh]
    int v42; // [sp+18h] [-3A8h]
    int v43; // [sp+1Ch] [-3A4h]
    int v44; // [sp+20h] [-3A0h]
    int v45; // [sp+24h] [-39Ch]
    int v46; // [sp+28h] [-398h]
    int v47; // [sp+2Ch] [-394h]
    int v48; // [sp+30h] [-390h]
    int v49; // [sp+34h] [-38Ch]
    int v50; // [sp+38h] [-388h]
    int v51; // [sp+3Ch] [-384h]
    int v52; // [sp+40h] [-380h]
    int v53; // [sp+44h] [-37Ch]
    int v54; // [sp+48h] [-378h]
    int v55; // [sp+4Ch] [-374h]
    int v56; // [sp+50h] [-370h]
    int v57; // [sp+58h] [-368h]
    _BYTE v58[16]; // [sp+60h] [-360h] BYREF
    pathsort_t v59[70]; // [sp+70h] [-350h] BYREF

    result = Path_NearestNodeNotCrossPlanes(
        vGoalPos,
        v59,
        -2,
        192.0,
        0,
        (float *)vNormal,
        (int)fDist,
        iPlaneCount,
        (int)v58,
        v37);
    if (result)
        return (pathnode_t *)Path_FindPathFromToNotCrossPlanes(
            pPath,
            eTeam,
            pNodeFrom,
            vStartPos,
            result,
            vGoalPos,
            vNormal,
            fDist,
            v38,
            v39,
            v40,
            v41,
            v42,
            v43,
            v44,
            v45,
            v46,
            v47,
            v48,
            v49,
            v50,
            v51,
            v52,
            v53,
            v54,
            v55,
            v56,
            (int)iPlaneCount,
            v57,
            a28);
    return result;
}

pathnode_t *__cdecl Path_FindPathAway(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    float *vAwayFromPos,
    double fDistAway,
    int bAllowNegotiationLinks,
    int a7)
{
    int v13; // r8
    pathnode_t *result; // r3
    _BYTE v15[16]; // [sp+50h] [-350h] BYREF
    pathsort_t v16[64]; // [sp+60h] [-340h] BYREF

    result = Path_NearestNode(vStartPos, v16, -2, 192.0, (int *)vAwayFromPos, (int)v15, (nearestNodeHeightCheck)64);
    if (result)
        return (pathnode_t *)Path_FindPathFromAway(pPath, eTeam, result, vStartPos, vAwayFromPos, fDistAway, v13, a7);
    return result;
}

pathnode_t *__cdecl Path_FindPathAwayNotCrossPlanes(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    float *vAwayFromPos,
    double fDistAway,
    float (*vNormal)[2],
    float *fDist,
    float *iPlaneCount,
    int *bAllowNegotiationLinks,
    int a10,
    int a11,
    int a12,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28,
    int a29)
{
    float *v37; // r8
    pathnode_t *result; // r3
    nearestNodeHeightCheck v39; // [sp+8h] [-3B8h]
    int v40; // [sp+8h] [-3B8h]
    int v41; // [sp+Ch] [-3B4h]
    int v42; // [sp+10h] [-3B0h]
    int v43; // [sp+14h] [-3ACh]
    int v44; // [sp+18h] [-3A8h]
    int v45; // [sp+1Ch] [-3A4h]
    int v46; // [sp+20h] [-3A0h]
    int v47; // [sp+24h] [-39Ch]
    int v48; // [sp+28h] [-398h]
    int v49; // [sp+2Ch] [-394h]
    int v50; // [sp+30h] [-390h]
    int v51; // [sp+34h] [-38Ch]
    int v52; // [sp+38h] [-388h]
    int v53; // [sp+3Ch] [-384h]
    int v54; // [sp+40h] [-380h]
    int v55; // [sp+44h] [-37Ch]
    int v56; // [sp+48h] [-378h]
    int v57; // [sp+4Ch] [-374h]
    int v58; // [sp+50h] [-370h]
    int v59; // [sp+58h] [-368h]
    _BYTE v60[16]; // [sp+60h] [-360h] BYREF
    pathsort_t v61[64]; // [sp+70h] [-350h] BYREF

    result = Path_NearestNodeNotCrossPlanes(
        vStartPos,
        v61,
        -2,
        192.0,
        0,
        fDist,
        (int)iPlaneCount,
        bAllowNegotiationLinks,
        (int)v60,
        v39);
    if (result)
        return (pathnode_t *)Path_FindPathFromAwayNotCrossPlanes(
            pPath,
            eTeam,
            result,
            vStartPos,
            vAwayFromPos,
            fDistAway,
            (float (*)[2])v37,
            fDist,
            iPlaneCount,
            v40,
            v41,
            v42,
            v43,
            v44,
            v45,
            v46,
            v47,
            v48,
            v49,
            v50,
            v51,
            v52,
            v53,
            v54,
            v55,
            v56,
            v57,
            v58,
            (int)bAllowNegotiationLinks,
            v59,
            a29);
    return result;
}

