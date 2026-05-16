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

float __cdecl BG_GetActorProneFraction(actor_prone_info_s *pInfo, int iCurrentTime)
{
    if (!pInfo->prone)
        return 0.0f;

    int trans = pInfo->iProneTrans;
    if (!trans)
        return 1.0f;

    int startTime = pInfo->iProneTime;
    int elapsed = iCurrentTime - startTime;

    if (trans < 0)
    {
        // standing up (prone -> stand)
        if (startTime - trans >= iCurrentTime)
            return 1.0f - (float)elapsed / (float)(-trans);
        pInfo->prone = 0;
        return 0.0f;
    }

    // going prone (stand -> prone)
    if (startTime + trans >= iCurrentTime)
        return (float)elapsed / (float)trans;
    pInfo->iProneTrans = 0;
    return 1.0f;
}

