#include "bg_actor_prone.h"

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

int __cdecl BG_ActorIsProne(actor_prone_info_s *pInfo, int iCurrentTime)
{
    int iProneTrans; // r10
    int iProneTime; // r9

    if (!pInfo->prone || pInfo->bCorpseOrientation || pInfo->orientPitch)
        return 0;
    iProneTrans = pInfo->iProneTrans;
    if (iProneTrans)
    {
        iProneTime = pInfo->iProneTime;
        if (iProneTrans >= 0)
        {
            if (iProneTime + iProneTrans < iCurrentTime)
                pInfo->iProneTrans = 0;
        }
        else if (iProneTime - iProneTrans < iCurrentTime)
        {
            pInfo->prone = 0;
            return 0;
        }
    }
    return 1;
}

int __cdecl BG_ActorGoalIsProne(actor_prone_info_s *pInfo)
{
    int iProneTrans; // r11
    int result; // r3

    if (!pInfo->prone)
        return 0;
    if (pInfo->bCorpseOrientation)
        return 0;
    if (pInfo->orientPitch)
        return 0;
    iProneTrans = pInfo->iProneTrans;
    result = 1;
    if (iProneTrans < 0)
        return 0;
    return result;
}

// local variable allocation has failed, the output may be wrong!
float __cdecl BG_GetActorProneFraction(actor_prone_info_s *pInfo, int iCurrentTime)
{
    __int64 v2; // r11 OVERLAPPED
    int v3; // r9 OVERLAPPED
    double v4; // fp1
    int v5; // r9 OVERLAPPED

    if (!pInfo->prone)
    {
    LABEL_6:
        v4 = 0.0;
        return *((float *)&v4 + 1);
    }
    LODWORD(v2) = pInfo->iProneTrans;
    if (!(unsigned int)v2)
    {
        v4 = 1.0;
        return *((float *)&v4 + 1);
    }
    HIDWORD(v2) = pInfo->iProneTime;
    if ((int)v2 < 0)
    {
        v3 = HIDWORD(v2) - v2;
        if (HIDWORD(v2) - (int)v2 >= iCurrentTime)
        {
            LODWORD(v2) = -(int)v2;
            HIDWORD(v2) = iCurrentTime - HIDWORD(v2);
            v4 = (float)((float)1.0 - (float)((float)*(__int64 *)((char *)&v2 + 4) / (float)v2));
            return *((float *)&v4 + 1);
        }
        pInfo->prone = 0;
        goto LABEL_6;
    }
    v5 = HIDWORD(v2) + v2;
    if (HIDWORD(v2) + (int)v2 >= iCurrentTime)
    {
        HIDWORD(v2) = iCurrentTime - HIDWORD(v2);
        v4 = (float)((float)*(__int64 *)((char *)&v2 + 4) / (float)v2);
    }
    else
    {
        pInfo->iProneTrans = 0;
        v4 = 1.0;
    }
    return *((float *)&v4 + 1);
}

