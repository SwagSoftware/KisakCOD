#pragma once
#include "bg_public.h"

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

struct actor_prone_info_s
{
    bool bCorpseOrientation;
    bool orientPitch;
    bool prone;
    int iProneTime;
    int iProneTrans;
    float fBodyHeight;
    //$A899A4A44C693354E5CF33C9EDFF92AE ___u6;
    union
    {
        float fTorsoPitch;
        float fBodyPitch;
    };
    //$8F7A1F2A0E788339D3BE9A175DA5EAEF ___u7;
    union
    {
        float fWaistPitch;
        float fBodyRoll;
    };
};

int __cdecl BG_ActorIsProne(actor_prone_info_s *pInfo, int iCurrentTime);
int __cdecl BG_ActorGoalIsProne(actor_prone_info_s *pInfo);
// local variable allocation has failed, the output may be wrong!
float __cdecl BG_GetActorProneFraction(actor_prone_info_s *pInfo, int iCurrentTime);
