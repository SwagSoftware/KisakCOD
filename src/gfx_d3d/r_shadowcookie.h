#pragma once

#include "r_gfx.h"
#include "r_rendercmds.h"

struct ShadowCandidate // sizeof=0x8
{                                       // ...
    int sceneEntIndex;                  // ...
    float weight;                       // ...
};

void __cdecl R_EmitShadowCookieSurfs(GfxViewInfo *viewInfo);
void __cdecl R_GenerateShadowCookiesCmd(ShadowCookieCmd *cmd);
void __cdecl R_GenerateShadowCookies(
    int localClientNum,
    const GfxViewParms *viewParmsDpvs,
    const GfxViewParms *viewParmsDraw,
    ShadowCookieList *shadowCookieList);
bool __cdecl R_ShadowCandidatePred(const ShadowCandidate &a, const ShadowCandidate &b);
void __cdecl R_PopulateCandidates(const GfxViewParms *viewParmsDraw, ShadowCandidate *candidates);
void __cdecl R_AddCasters(
    int localClientNum,
    const GfxViewParms *viewParmsDraw,
    ShadowCandidate *candidates,
    ShadowCookieList *shadowCookieList);
void __cdecl R_AddShadowCookie(
    int localClientNum,
    const GfxViewParms *viewParms,
    unsigned int sceneEntIndex,
    float fade,
    ShadowCookieList *cookieList);
void __cdecl R_GenerateShadowCookieViewParms(float *modelMin, float *modelMax, GfxViewParms *shadowViewParms);
void __cdecl R_GenerateBspShadowReceivers(ShadowCookieList *shadowCookieList);
bool __cdecl R_AllowBspShadowReceiver(int surfIndex, unsigned int *shadowReceiverCallbackAsVoid);
void __cdecl R_GenerateSceneEntShadowReceivers(ShadowCookieList *shadowCookieList);
char __cdecl R_OutsideOfShadowFrustumPlanes(const DpvsPlane *planes, const float *minmax);
void __cdecl R_ResetShadowCookies();
