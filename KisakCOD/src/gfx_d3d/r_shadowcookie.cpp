#include "r_shadowcookie.h"
#include "r_pretess.h"
#include "r_scene.h"
#include <qcommon/mem_track.h>

#include <algorithm>
#include <xanim/dobj.h>
#include "r_dvars.h"
#include "r_dpvs.h"
#include "r_model_pose.h"
#include "r_model.h"
#include "r_drawsurf.h"
#include <cgame/cg_local.h>
#include "r_sunshadow.h"
#include "r_marks.h"

struct ShadowReceiverCallback // sizeof=0x4
{                                       // ...
    unsigned __int8 *surfaceVisData;    // ...
};
struct ShadowCookieGlob // sizeof=0x8
{                                       // ...
    float weightCap;                    // ...
    int lastTime;                       // ...
};

ShadowCookieGlob shadowCookieGlob;

bool __cdecl R_SortBspShadowReceiverSurfaces(GfxSurface *surface0, GfxSurface *surface1)
{
    return surface0 < surface1;
}

void __cdecl R_EmitShadowCookieSurfs(GfxViewInfo *viewInfo)
{
    float *v1; // [esp+4h] [ebp-38h]
    float *dir; // [esp+Ch] [ebp-30h]
    ShadowCookie *cookie; // [esp+14h] [ebp-28h]
    int firstCasterDrawSurf; // [esp+1Ch] [ebp-20h]
    unsigned int cookieIndex; // [esp+20h] [ebp-1Ch]
    int firstReceiverDrawSurf; // [esp+24h] [ebp-18h]
    unsigned int casterDrawSurfCount; // [esp+2Ch] [ebp-10h]
    GfxDrawSurf *casterDrawSurfs; // [esp+34h] [ebp-8h]

    if (frontEndDataOut->sunLight.type != 1)
        MyAssertHandler(".\\r_shadowcookie.cpp", 308, 0, "%s", "frontEndDataOut->sunLight.type == GFX_LIGHT_TYPE_DIR");
    for (cookieIndex = 0; cookieIndex < viewInfo->shadowCookieList.cookieCount; ++cookieIndex)
    {
        cookie = &viewInfo->shadowCookieList.cookies[cookieIndex];
        casterDrawSurfs = (GfxDrawSurf *)cookie->casterInfo.drawSurfs;
        casterDrawSurfCount = cookie->casterInfo.drawSurfCount;
        R_InitDrawSurfListInfo(&cookie->casterInfo);
        cookie->casterInfo.baseTechType = TECHNIQUE_SHADOWCOOKIE_CASTER;
        cookie->casterInfo.viewInfo = viewInfo;
        dir = frontEndDataOut->sunLight.dir;
        cookie->casterInfo.viewOrigin[0] = frontEndDataOut->sunLight.dir[0];
        cookie->casterInfo.viewOrigin[1] = dir[1];
        cookie->casterInfo.viewOrigin[2] = dir[2];
        cookie->casterInfo.viewOrigin[3] = 0.0;
        if (cookie->casterInfo.cameraView)
            MyAssertHandler(".\\r_shadowcookie.cpp", 325, 0, "%s", "!casterInfo->cameraView");
        R_InitDrawSurfListInfo(&cookie->receiverInfo);
        cookie->receiverInfo.baseTechType = TECHNIQUE_SHADOWCOOKIE_RECEIVER;
        cookie->receiverInfo.viewInfo = viewInfo;
        v1 = frontEndDataOut->sunLight.dir;
        cookie->receiverInfo.viewOrigin[0] = frontEndDataOut->sunLight.dir[0];
        cookie->receiverInfo.viewOrigin[1] = v1[1];
        cookie->receiverInfo.viewOrigin[2] = v1[2];
        cookie->receiverInfo.viewOrigin[3] = 0.0;
        if (cookie->receiverInfo.cameraView)
            MyAssertHandler(".\\r_shadowcookie.cpp", 333, 0, "%s", "!receiverInfo->cameraView");
        firstCasterDrawSurf = frontEndDataOut->drawSurfCount;
        R_EmitDrawSurfList(casterDrawSurfs, casterDrawSurfCount);
        cookie->casterInfo.drawSurfs = &frontEndDataOut->drawSurfs[firstCasterDrawSurf];
        cookie->casterInfo.drawSurfCount = frontEndDataOut->drawSurfCount - firstCasterDrawSurf;
        firstReceiverDrawSurf = frontEndDataOut->drawSurfCount;
        R_EmitDrawSurfList(scene.cookie[cookieIndex].drawSurfs, scene.cookie[cookieIndex].drawSurfCount);
        cookie->receiverInfo.drawSurfs = &frontEndDataOut->drawSurfs[firstReceiverDrawSurf];
        cookie->receiverInfo.drawSurfCount = frontEndDataOut->drawSurfCount - firstReceiverDrawSurf;
    }
}

void __cdecl R_GenerateShadowCookiesCmd(ShadowCookieCmd *cmd)
{
    R_GenerateShadowCookies(cmd->localClientNum, cmd->viewParmsDpvs, cmd->viewParmsDraw, cmd->shadowCookieList);
}


void __cdecl R_PopulateCandidates(const GfxViewParms *viewParmsDraw, ShadowCandidate *candidates)
{
    double Radius; // st7
    float diff[3]; // [esp+4h] [ebp-38h] BYREF
    ShadowCandidate *worstCandidate; // [esp+10h] [ebp-2Ch]
    float entityMajorAxisLength; // [esp+14h] [ebp-28h]
    unsigned int sceneEntCount; // [esp+18h] [ebp-24h]
    int shadowHint; // [esp+1Ch] [ebp-20h]
    GfxEntity *gfxEnt; // [esp+20h] [ebp-1Ch]
    unsigned int sceneEntIndex; // [esp+24h] [ebp-18h]
    GfxSceneEntity *sceneEnt; // [esp+28h] [ebp-14h]
    unsigned int gfxEntIndex; // [esp+2Ch] [ebp-10h]
    unsigned int candidateIter; // [esp+30h] [ebp-Ch]
    float entityDistance; // [esp+34h] [ebp-8h]
    float entityWeight; // [esp+38h] [ebp-4h]

    for (candidateIter = 0; candidateIter != 24; ++candidateIter)
    {
        candidates[candidateIter].sceneEntIndex = -1;
        candidates[candidateIter].weight = 100000000.0;
    }
    worstCandidate = candidates;
    sceneEntCount = scene.sceneDObjCount;
    for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
    {
        sceneEnt = &scene.sceneDObj[sceneEntIndex];
        gfxEntIndex = sceneEnt->gfxEntIndex;
        if (gfxEntIndex)
        {
            gfxEnt = &frontEndDataOut->gfxEnts[gfxEntIndex];
            if ((gfxEnt->renderFxFlags & 2) != 0)
                continue;
            shadowHint = gfxEnt->renderFxFlags & 0xE00;
            if (shadowHint == 512)
                continue;
        }
        else
        {
            shadowHint = 0;
        }
        if (sceneEnt->cull.state != 4)
        {
            Radius = DObjGetRadius(sceneEnt->obj);
            entityMajorAxisLength = Radius + Radius;
            if (entityMajorAxisLength != 0.0)
            {
                Vec3Sub(sceneEnt->placement.base.origin, viewParmsDraw->origin, diff);
                entityDistance = Vec3Length(diff);
                entityWeight = entityDistance / entityMajorAxisLength;
                if (entityWeight < 100000000.0)
                {
                    if (shadowHint && shadowHint != 2048 && shadowHint != 1536 && shadowHint != 1024)
                        MyAssertHandler(
                            ".\\r_shadowcookie.cpp",
                            407,
                            0,
                            "%s\n\t(shadowHint) = %i",
                            "(shadowHint == (0 << (9)) || shadowHint == (4 << (9)) || shadowHint == (3 << (9)) || shadowHint == (2 << (9)))",
                            shadowHint);
                    switch (shadowHint)
                    {
                    case 2048:
                        entityWeight = 0.0;
                        break;
                    case 1536:
                        entityWeight = entityWeight * 0.25;
                        break;
                    case 1024:
                        entityWeight = entityWeight * 4.0;
                        break;
                    }
                    if (!worstCandidate)
                        MyAssertHandler(".\\r_shadowcookie.cpp", 416, 0, "%s", "worstCandidate");
                    if (worstCandidate->weight >= (double)entityWeight)
                    {
                        worstCandidate->sceneEntIndex = sceneEntIndex;
                        worstCandidate->weight = entityWeight;
                        worstCandidate = candidates;
                        for (candidateIter = 1; candidateIter != 24; ++candidateIter)
                        {
                            if (worstCandidate->weight < (double)candidates[candidateIter].weight)
                                worstCandidate = &candidates[candidateIter];
                        }
                    }
                }
            }
        }
    }
}

bool __cdecl R_ShadowCandidatePred(const ShadowCandidate &a, const ShadowCandidate &b)
{
    return b.weight > a.weight;
}

const float shadowFrustumSidePlanes[5][4] =
{
  { -1.0, 0.0, 0.0, 1.0 },
  { 1.0, 0.0, 0.0, 1.0 },
  { 0.0, -1.0, 0.0, 1.0 },
  { 0.0, 1.0, 0.0, 1.0 },
  { 0.0, 0.0, 1.0, 0.0 }
}; // idb

void __cdecl R_GenerateShadowCookies(
    int localClientNum,
    const GfxViewParms *viewParmsDpvs,
    const GfxViewParms *viewParmsDraw,
    ShadowCookieList *shadowCookieList)
{
    ShadowCookie *cookie; // [esp+78h] [ebp-CCh]
    ShadowCandidate candidates[24]; // [esp+7Ch] [ebp-C8h] BYREF
    ShadowCandidate cookieIndex; // [esp+13Ch] [ebp-8h] BYREF

    CL_ResetStats_f();
    //Profile_Begin(150);
    R_PopulateCandidates(viewParmsDraw, &candidates[0]);

    //std::_Sort<ShadowCandidate *, int, bool(__cdecl *)(ShadowCandidate const &, ShadowCandidate const &)>(
    //    candidates,
    //    &cookieIndex,
    //    24,
    //    R_ShadowCandidatePred);
    std::sort(&candidates[0], &candidates[23], R_ShadowCandidatePred);

    R_AddCasters(localClientNum, viewParmsDraw, candidates, shadowCookieList);
    //Profile_EndInternal(0);
    LODWORD(cookieIndex.weight) = shadowCookieList->cookieCount;
    if (LODWORD(cookieIndex.weight))
    {
        for (cookieIndex.sceneEntIndex = 0;
            cookieIndex.sceneEntIndex < LODWORD(cookieIndex.weight);
            ++cookieIndex.sceneEntIndex)
        {
            cookie = &shadowCookieList->cookies[cookieIndex.sceneEntIndex];
            scene.cookie[cookieIndex.sceneEntIndex].drawSurfCount = 0;
            R_FrustumClipPlanes(
                &cookie->shadowViewParms->viewMatrix,
                shadowFrustumSidePlanes,
                5,
                scene.cookie[cookieIndex.sceneEntIndex].planes);
        }
        R_GenerateBspShadowReceivers(shadowCookieList);
        R_GenerateSceneEntShadowReceivers(shadowCookieList);
    }
}

void __cdecl R_AddCasters(
    int localClientNum,
    const GfxViewParms *viewParmsDraw,
    ShadowCandidate *candidates,
    ShadowCookieList *shadowCookieList)
{
    unsigned int unsignedInt; // [esp+8h] [ebp-38h]
    float timeDeltaSeconds; // [esp+18h] [ebp-28h]
    int localClientCount; // [esp+1Ch] [ebp-24h]
    float fadePoint; // [esp+20h] [ebp-20h]
    float fadeVal; // [esp+24h] [ebp-1Ch]
    signed int sceneEntIndex; // [esp+28h] [ebp-18h]
    int moveWeightCap; // [esp+2Ch] [ebp-14h]
    int candidateIter; // [esp+30h] [ebp-10h]
    float fadeScale; // [esp+38h] [ebp-8h]
    float entityWeight; // [esp+3Ch] [ebp-4h]

    CL_ResetStats_f();
    shadowCookieList->cookieCount = 0;
    moveWeightCap = 0;
    fadePoint = (1.0 - sc_fadeRange->current.value) * shadowCookieGlob.weightCap;
    fadeScale = 1.0 / (sc_fadeRange->current.value * shadowCookieGlob.weightCap);
    localClientCount = CL_GetLocalClientActiveCount();
    if (localClientCount <= 0)
        MyAssertHandler(".\\r_shadowcookie.cpp", 457, 0, "%s", "localClientCount > 0");
    for (candidateIter = 0; ; ++candidateIter)
    {
        if (candidateIter == 24 / localClientCount)
        {
            moveWeightCap = -1;
            goto LABEL_24;
        }
        sceneEntIndex = candidates[candidateIter].sceneEntIndex;
        if (sceneEntIndex < 0)
            goto LABEL_24;
        entityWeight = candidates[candidateIter].weight;
        if (entityWeight >= 100000000.0)
            MyAssertHandler(
                ".\\r_shadowcookie.cpp",
                480,
                0,
                "%s\n\t(entityWeight) = %g",
                "(entityWeight < 100000000.0f)",
                entityWeight);
        if (shadowCookieGlob.weightCap < (double)entityWeight)
            break;
        if (fadePoint >= (double)entityWeight || entityWeight == 0.0)
        {
            R_AddShadowCookie(localClientNum, viewParmsDraw, sceneEntIndex, 1.0, shadowCookieList);
        }
        else
        {
            fadeVal = (shadowCookieGlob.weightCap - entityWeight) * fadeScale;
            R_AddShadowCookie(localClientNum, viewParmsDraw, sceneEntIndex, fadeVal, shadowCookieList);
        }
    }
    if (!sc_wantCount)
        MyAssertHandler(".\\r_shadowcookie.cpp", 487, 0, "%s", "sc_wantCount");
    //if (!LODWORD(r_lightTweakSunDirection.vector[2]))
    if (!sc_wantCountMargin)
        MyAssertHandler(".\\r_shadowcookie.cpp", 488, 0, "%s", "sc_wantCountMargin");
    if (candidateIter <= sc_wantCountMargin->current.integer + 12 + sc_wantCount->current.integer)
    {
        //if (candidateIter < sc_wantCount->current.integer - *(unsigned int *)(LODWORD(r_lightTweakSunDirection.vector[2]) + 12))
        if (candidateIter < sc_wantCount->current.integer - sc_wantCountMargin->current.integer + 12)
            moveWeightCap = 1;
    }
    else
    {
        moveWeightCap = -1;
    }
LABEL_24:
    if (shadowCookieGlob.lastTime == -1)
        timeDeltaSeconds = 0.0;
    else
        timeDeltaSeconds = (double)(scene.def.time - shadowCookieGlob.lastTime) * EQUAL_EPSILON;
    if (timeDeltaSeconds < 0.0)
        timeDeltaSeconds = 0.0;
    shadowCookieGlob.lastTime = scene.def.time;
    if (moveWeightCap <= 0)
    {
        if (moveWeightCap < 0)
            shadowCookieGlob.weightCap = shadowCookieGlob.weightCap - sc_shadowInRate->current.value * timeDeltaSeconds;
    }
    else
    {
        shadowCookieGlob.weightCap = sc_shadowOutRate->current.value * timeDeltaSeconds + shadowCookieGlob.weightCap;
    }
    if (shadowCookieGlob.weightCap < 0.00009999999747378752)
        shadowCookieGlob.weightCap = 0.000099999997;
    if (sc_debugReceiverCount->current.integer < (signed int)shadowCookieList->cookieCount)
        unsignedInt = sc_debugReceiverCount->current.unsignedInt;
    else
        unsignedInt = shadowCookieList->cookieCount;
    shadowCookieList->cookieCount = unsignedInt;
}

void __cdecl R_AddShadowCookie(
    int localClientNum,
    const GfxViewParms *viewParms,
    unsigned int sceneEntIndex,
    float fade,
    ShadowCookieList *cookieList)
{
    float *boxMax; // [esp+54h] [ebp-5Ch]
    float *maxs; // [esp+58h] [ebp-58h]
    float *boxMin; // [esp+5Ch] [ebp-54h]
    float *mins; // [esp+60h] [ebp-50h]
    int firstDrawSurf; // [esp+78h] [ebp-38h]
    const DObj_s *obj; // [esp+7Ch] [ebp-34h] BYREF
    ShadowCookie *cookie; // [esp+80h] [ebp-30h]
    float span[3]; // [esp+84h] [ebp-2Ch] BYREF
    GfxSceneEntity *localSceneEnt; // [esp+90h] [ebp-20h] BYREF
    GfxDrawSurf *newCasterDrawSurfs; // [esp+94h] [ebp-1Ch]
    int endDrawSurf; // [esp+98h] [ebp-18h]
    int dimIter; // [esp+9Ch] [ebp-14h]
    GfxSceneEntity *sceneEnt; // [esp+A0h] [ebp-10h]
    int casterDrawSurfCount; // [esp+A4h] [ebp-Ch]
    GfxDrawSurf *casterDrawSurfs; // [esp+A8h] [ebp-8h]
    GfxDrawSurf *lastDrawSurf; // [esp+ACh] [ebp-4h]
    int savedregs; // [esp+B0h] [ebp+0h] BYREF

    //Profile_Begin(152);
    sceneEnt = &scene.sceneDObj[sceneEntIndex];
    if (R_UpdateSceneEntBounds(sceneEnt, &localSceneEnt, &obj, 1))
    {
        if (cookieList->cookieCount >= 0x18)
            MyAssertHandler(
                ".\\r_shadowcookie.cpp",
                231,
                0,
                "%s\n\t(cookieList->cookieCount) = %i",
                "(cookieList->cookieCount < 24)",
                cookieList->cookieCount);
        Vec3Sub(sceneEnt->cull.maxs, sceneEnt->cull.mins, span);
        if (Vec3LengthSq(span) <= 0.0)
            MyAssertHandler(".\\r_shadowcookie.cpp", 236, 0, "%s", "Vec3LengthSq( span ) > 0.0f");
        cookie = &cookieList->cookies[cookieList->cookieCount++];
        cookie->sceneEntIndex = sceneEntIndex;
        cookie->shadowViewParms = R_AllocViewParms();
        R_GenerateShadowCookieViewParms(sceneEnt->cull.mins, sceneEnt->cull.maxs, cookie->shadowViewParms);
        cookie->fade = fade;
        memcpy(cookie, &cookie->shadowViewParms->viewProjectionMatrix, 0x40u);
        cookie->shadowLookupMatrix.m[0][1] = cookie->shadowLookupMatrix.m[0][1] * -1.0;
        cookie->shadowLookupMatrix.m[1][1] = cookie->shadowLookupMatrix.m[1][1] * -1.0;
        cookie->shadowLookupMatrix.m[2][1] = cookie->shadowLookupMatrix.m[2][1] * -1.0;
        cookie->shadowLookupMatrix.m[3][1] = cookie->shadowLookupMatrix.m[3][1] * -1.0;
        for (dimIter = 0; dimIter != 2; ++dimIter)
        {
            cookie->shadowLookupMatrix.m[0][dimIter] = cookie->shadowLookupMatrix.m[0][dimIter] * 0.5;
            cookie->shadowLookupMatrix.m[1][dimIter] = cookie->shadowLookupMatrix.m[1][dimIter] * 0.5;
            cookie->shadowLookupMatrix.m[2][dimIter] = cookie->shadowLookupMatrix.m[2][dimIter] * 0.5;
            cookie->shadowLookupMatrix.m[3][dimIter] = cookie->shadowLookupMatrix.m[3][dimIter] * 0.5;
            cookie->shadowLookupMatrix.m[3][dimIter] = cookie->shadowLookupMatrix.m[3][dimIter] + 0.5;
        }
        boxMin = cookie->boxMin;
        mins = sceneEnt->cull.mins;
        cookie->boxMin[0] = sceneEnt->cull.mins[0];
        boxMin[1] = mins[1];
        boxMin[2] = mins[2];
        boxMax = cookie->boxMax;
        maxs = sceneEnt->cull.maxs;
        cookie->boxMax[0] = sceneEnt->cull.maxs[0];
        boxMax[1] = maxs[1];
        boxMax[2] = maxs[2];
        firstDrawSurf = scene.drawSurfCount[33];
        casterDrawSurfs = &scene.drawSurfs[33][scene.drawSurfCount[33]];
        cookie->casterInfo.drawSurfs = casterDrawSurfs;
        lastDrawSurf = &scene.drawSurfs[33][scene.maxDrawSurfCount[33]];
        if (cookieList->cookieCount <= sc_debugCasterCount->current.integer)
        {
            rg.debugViewParms = viewParms;
            //Profile_Begin(155);
            R_SkinSceneEnt(sceneEnt);
            //Profile_EndInternal(0);
            newCasterDrawSurfs = R_AddDObjSurfaces(sceneEnt, TECHNIQUE_SHADOWCOOKIE_CASTER, casterDrawSurfs, lastDrawSurf);
            casterDrawSurfCount = newCasterDrawSurfs - casterDrawSurfs;
            scene.drawSurfCount[33] += casterDrawSurfCount;
            R_SortDrawSurfs(casterDrawSurfs, casterDrawSurfCount);
        }
        endDrawSurf = scene.drawSurfCount[33];
        cookie->casterInfo.drawSurfCount = scene.drawSurfCount[33] - firstDrawSurf;
        cookie->receiverInfo.drawSurfs = &scene.drawSurfs[33][endDrawSurf];
        //Profile_EndInternal(0);
    }
    else
    {
        if (localSceneEnt)
            CG_UsedDObjCalcPose(localSceneEnt->info.pose);
        //Profile_EndInternal(0);
    }
}

void __cdecl R_GenerateShadowCookieViewParms(float *modelMin, float *modelMax, GfxViewParms *shadowViewParms)
{
    float width; // [esp+10h] [ebp-90h]
    float v4; // [esp+14h] [ebp-8Ch]
    float v5; // [esp+18h] [ebp-88h]
    float v6; // [esp+1Ch] [ebp-84h]
    float projected; // [esp+38h] [ebp-68h]
    float span; // [esp+40h] [ebp-60h]
    int outDimIter; // [esp+44h] [ebp-5Ch]
    int dimIter; // [esp+48h] [ebp-58h]
    int dimItera; // [esp+48h] [ebp-58h]
    float projectedMax[3]; // [esp+4Ch] [ebp-54h]
    int cornerIter; // [esp+58h] [ebp-48h]
    float projectedMin[3]; // [esp+5Ch] [ebp-44h]
    float scale; // [esp+68h] [ebp-38h]
    float corner[3]; // [esp+6Ch] [ebp-34h] BYREF
    float sunAxes[3][3]; // [esp+78h] [ebp-28h] BYREF
    float inverseLength; // [esp+9Ch] [ebp-4h]

    if (!modelMin)
        MyAssertHandler(".\\r_shadowcookie.cpp", 92, 0, "%s", "modelMin");
    if (!modelMax)
        MyAssertHandler(".\\r_shadowcookie.cpp", 93, 0, "%s", "modelMax");
    if (!shadowViewParms)
        MyAssertHandler(".\\r_shadowcookie.cpp", 94, 0, "%s", "shadowViewParms");
    R_GetSunAxes(sunAxes);
    sunAxes[1][0] = -sunAxes[1][0];
    sunAxes[1][1] = -sunAxes[1][1];
    sunAxes[1][2] = -sunAxes[1][2];
    for (cornerIter = 0; cornerIter != 8; ++cornerIter)
    {
        if ((cornerIter & 1) != 0)
            v6 = *modelMax;
        else
            v6 = *modelMin;
        corner[0] = v6;
        if ((cornerIter & 2) != 0)
            v5 = modelMax[1];
        else
            v5 = modelMin[1];
        corner[1] = v5;
        if ((cornerIter & 4) != 0)
            v4 = modelMax[2];
        else
            v4 = modelMin[2];
        corner[2] = v4;
        for (dimIter = 0; dimIter != 3; ++dimIter)
        {
            projected = Vec3Dot(corner, sunAxes[dimIter]);
            if (!cornerIter || projectedMax[dimIter] < (double)projected)
                projectedMax[dimIter] = projected;
            if (!cornerIter || projectedMin[dimIter] > (double)projected)
                projectedMin[dimIter] = projected;
        }
    }
    for (dimItera = 0; dimItera != 3; ++dimItera)
    {
        scale = 1.0;
        span = projectedMax[dimItera] - projectedMin[dimItera];
        scale = 2.0 / span;
        scale = scale * 0.9375;
        outDimIter = (dimItera + 2) % 3;
        if (outDimIter == 2)
        {
            shadowViewParms->viewMatrix.m[0][2] = sunAxes[dimItera][0];
            shadowViewParms->viewMatrix.m[1][2] = sunAxes[dimItera][1];
            shadowViewParms->viewMatrix.m[2][2] = sunAxes[dimItera][2];
            shadowViewParms->viewMatrix.m[3][2] = (projectedMin[dimItera] + projectedMax[dimItera]) * -0.5;
            if (sc_length->current.value <= 0.0)
                MyAssertHandler(".\\r_shadowcookie.cpp", 142, 0, "%s", "sc_length->current.value > 0");
            inverseLength = 1.0 / sc_length->current.value;
            shadowViewParms->viewMatrix.m[0][2] = shadowViewParms->viewMatrix.m[0][2] * inverseLength;
            shadowViewParms->viewMatrix.m[1][2] = shadowViewParms->viewMatrix.m[1][2] * inverseLength;
            shadowViewParms->viewMatrix.m[2][2] = shadowViewParms->viewMatrix.m[2][2] * inverseLength;
            shadowViewParms->viewMatrix.m[3][2] = shadowViewParms->viewMatrix.m[3][2] * inverseLength;
        }
        else
        {
            shadowViewParms->viewMatrix.m[0][outDimIter] = sunAxes[dimItera][0] * scale;
            shadowViewParms->viewMatrix.m[1][outDimIter] = sunAxes[dimItera][1] * scale;
            shadowViewParms->viewMatrix.m[2][outDimIter] = sunAxes[dimItera][2] * scale;
            shadowViewParms->viewMatrix.m[3][outDimIter] = -scale * 0.5 * (projectedMin[dimItera] + projectedMax[dimItera]);
        }
    }
    shadowViewParms->viewMatrix.m[0][3] = 0.0;
    shadowViewParms->viewMatrix.m[1][3] = 0.0;
    shadowViewParms->viewMatrix.m[2][3] = 0.0;
    shadowViewParms->viewMatrix.m[3][3] = 1.0;
    shadowViewParms->axis[0][0] = sunAxes[0][0];
    shadowViewParms->axis[0][1] = sunAxes[0][1];
    shadowViewParms->axis[0][2] = sunAxes[0][2];
    shadowViewParms->axis[1][0] = sunAxes[1][0];
    shadowViewParms->axis[1][1] = sunAxes[1][1];
    shadowViewParms->axis[1][2] = sunAxes[1][2];
    shadowViewParms->axis[2][0] = sunAxes[2][0];
    shadowViewParms->axis[2][1] = sunAxes[2][1];
    shadowViewParms->axis[2][2] = sunAxes[2][2];
    shadowViewParms->origin[0] = -sunAxes[0][0];
    shadowViewParms->origin[1] = -sunAxes[0][1];
    shadowViewParms->origin[2] = -sunAxes[0][2];
    shadowViewParms->origin[3] = 0.0;
    width = (float)1.0158731 * 2.0;
    OrthographicMatrix(shadowViewParms->projectionMatrix.m, width, width, 1.0);
    shadowViewParms->depthHackNearClip = shadowViewParms->projectionMatrix.m[3][2];
    R_SetupViewProjectionMatrices(shadowViewParms);
}

void __cdecl R_GenerateBspShadowReceivers(ShadowCookieList *shadowCookieList)
{
    unsigned __int16 triSurfList[2]; // [esp+A0h] [ebp-64h] BYREF
    ShadowCookie *cookie; // [esp+A4h] [ebp-60h]
    unsigned int surfIndex; // [esp+A8h] [ebp-5Ch]
    float start[3]; // [esp+ACh] [ebp-58h] BYREF
    float end[3]; // [esp+B8h] [ebp-4Ch] BYREF
    float radius; // [esp+C4h] [ebp-40h]
    ShadowReceiverCallback shadowReceiverCallback; // [esp+C8h] [ebp-3Ch] BYREF
    int cookieIndex; // [esp+CCh] [ebp-38h]
    int cookieCount; // [esp+D0h] [ebp-34h]
    const GfxSceneEntity *sceneEnt; // [esp+D4h] [ebp-30h]
    GfxSurface **surfaces; // [esp+D8h] [ebp-2Ch]
    GfxDrawSurf *drawSurfs; // [esp+DCh] [ebp-28h]
    GfxDrawSurf *surfaceMaterials; // [esp+E0h] [ebp-24h]
    unsigned int listSurfIndex; // [esp+E4h] [ebp-20h]
    unsigned int cookieDrawSurfCount; // [esp+E8h] [ebp-1Ch]
    GfxBspDrawSurfData surfData; // [esp+ECh] [ebp-18h] BYREF
    int savedregs; // [esp+104h] [ebp+0h] BYREF

    if (!rgp.world)
        MyAssertHandler(".\\r_shadowcookie.cpp", 583, 0, "%s", "rgp.world");
    cookieCount = shadowCookieList->cookieCount;
    if (!cookieCount)
        MyAssertHandler(".\\r_shadowcookie.cpp", 586, 0, "%s", "cookieCount");
    shadowReceiverCallback.surfaceVisData = rgp.world->dpvs.surfaceVisData[0];
    //Profile_Begin(153);
    R_InitBspDrawSurf(&surfData);
    surfaceMaterials = rgp.world->dpvs.surfaceMaterials;
    for (cookieIndex = 0; cookieIndex < cookieCount; ++cookieIndex)
    {
        cookie = &shadowCookieList->cookies[cookieIndex];
        sceneEnt = &scene.sceneDObj[cookie->sceneEntIndex];
        start[0] = sceneEnt->placement.base.origin[0];
        start[1] = sceneEnt->placement.base.origin[1];
        start[2] = sceneEnt->placement.base.origin[2];
        Vec3Mad(start, 10000.0, cookie->shadowViewParms->axis[0], end);
        radius = DObjGetRadius(sceneEnt->obj);
        drawSurfs = scene.cookie[cookieIndex].drawSurfs;
        surfaces = (GfxSurface **)&drawSurfs[128];
        cookieDrawSurfCount = R_CylinderSurfaces(
            start,
            end,
            radius,
            scene.cookie[cookieIndex].planes,
            5u,
            (int(__cdecl *)(int, void *))R_AllowBspShadowReceiver,
            &shadowReceiverCallback,
            (GfxSurface **)&drawSurfs[128],
            0x100u);
        if (cookieDrawSurfCount)
        {
            surfData.drawSurfList.current = drawSurfs;
            surfData.drawSurfList.end = (GfxDrawSurf *)&scene.cookie[cookieIndex + 1];
            //std::_Sort<int *, int, bool(__cdecl *)(int, int)>(
            //    (const GfxStaticModelDrawInst **)surfaces,
            //    (const GfxStaticModelDrawInst **)&surfaces[cookieDrawSurfCount],
            //    (int)(4 * cookieDrawSurfCount) >> 2,
            //    (bool(__cdecl *)(const GfxStaticModelDrawInst *, const GfxStaticModelDrawInst *))R_SortBspShadowReceiverSurfaces);
            std::sort(surfaces, surfaces + cookieDrawSurfCount, R_SortBspShadowReceiverSurfaces);
            for (listSurfIndex = 0; listSurfIndex < cookieDrawSurfCount; ++listSurfIndex)
            {
                if (listSurfIndex >= rgp.world->surfaceCount)
                    MyAssertHandler(
                        ".\\r_shadowcookie.cpp",
                        620,
                        0,
                        "listSurfIndex doesn't index rgp.world->surfaceCount\n\t%i not in [0, %i)",
                        listSurfIndex,
                        rgp.world->surfaceCount);
                surfIndex = surfaces[listSurfIndex] - rgp.world->dpvs.surfaces;
                triSurfList[0] = surfIndex;
                R_AddBspDrawSurfs(surfaceMaterials[surfIndex], (unsigned __int8 *)triSurfList, 1u, &surfData);
            }
            R_EndCmdBuf(&surfData.delayedCmdBuf);
            scene.cookie[cookieIndex].drawSurfCount = surfData.drawSurfList.current - scene.cookie[cookieIndex].drawSurfs;
        }
    }
    //Profile_EndInternal(0);
}

bool __cdecl R_AllowBspShadowReceiver(int surfIndex, unsigned int *shadowReceiverCallbackAsVoid)
{
    return *(_BYTE *)(*shadowReceiverCallbackAsVoid + surfIndex)
        && Material_GetTechnique(rgp.world->dpvs.surfaces[surfIndex].material, TECHNIQUE_SHADOWCOOKIE_RECEIVER) != 0;
}

void __cdecl R_GenerateSceneEntShadowReceivers(ShadowCookieList *shadowCookieList)
{
    volatile unsigned int sceneEntCount; // [esp+48h] [ebp-28h]
    unsigned int cookieIndex; // [esp+50h] [ebp-20h]
    unsigned int sceneEntIndex; // [esp+54h] [ebp-1Ch]
    GfxSceneEntity *sceneEnt; // [esp+58h] [ebp-18h]
    unsigned int cookieCount; // [esp+5Ch] [ebp-14h]
    volatile int cookieDrawSurfCount; // [esp+68h] [ebp-8h]

    //Profile_Begin(153);
    sceneEntCount = scene.sceneDObjCount;
    cookieCount = shadowCookieList->cookieCount;
    if (!cookieCount)
        MyAssertHandler(".\\r_shadowcookie.cpp", 660, 0, "%s", "cookieCount");
    for (sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex)
    {
        if (scene.sceneDObjVisData[0][sceneEntIndex] == 1)
        {
            sceneEnt = &scene.sceneDObj[sceneEntIndex];
            if (sceneEnt->cull.state < 2)
                MyAssertHandler(
                    ".\\r_shadowcookie.cpp",
                    668,
                    0,
                    "sceneEnt->cull.state >= CULL_STATE_BOUNDED\n\t%i, %i",
                    sceneEnt->cull.state,
                    2);
            if (sceneEnt->cull.state == 4)
                MyAssertHandler(".\\r_shadowcookie.cpp", 669, 0, "%s", "sceneEnt->cull.state != CULL_STATE");
            if (sceneEnt->gfxEntIndex && (frontEndDataOut->gfxEnts[sceneEnt->gfxEntIndex].renderFxFlags & 0x100) != 0)
            {
                for (cookieIndex = 0; cookieIndex < cookieCount; ++cookieIndex)
                {
                    if (sceneEntIndex != shadowCookieList->cookies[cookieIndex].sceneEntIndex
                        && !R_OutsideOfShadowFrustumPlanes(scene.cookie[cookieIndex].planes, sceneEnt->cull.mins))
                    {
                        cookieDrawSurfCount = scene.cookie[cookieIndex].drawSurfCount;
                        scene.cookie[cookieIndex].drawSurfCount = R_AddDObjSurfaces(
                            sceneEnt,
                            TECHNIQUE_SHADOWCOOKIE_RECEIVER,
                            &scene.cookie[cookieIndex].drawSurfs[cookieDrawSurfCount],
                            (GfxDrawSurf *)&scene.cookie[cookieIndex + 1])
                            - &scene.cookie[cookieIndex].drawSurfs[cookieDrawSurfCount]
                            + cookieDrawSurfCount;
                    }
                }
            }
        }
    }
    //Profile_EndInternal(0);
}

char __cdecl R_OutsideOfShadowFrustumPlanes(const DpvsPlane *planes, const float *minmax)
{
    int plane; // [esp+8h] [ebp-4h]

    for (plane = 0; plane != 5; ++plane)
    {
        if (*(const float *)((char *)minmax + planes[plane].side[0]) * planes[plane].coeffs[0]
            + planes[plane].coeffs[3]
            + *(const float *)((char *)minmax + planes[plane].side[1]) * planes[plane].coeffs[1]
            + *(const float *)((char *)minmax + planes[plane].side[2]) * planes[plane].coeffs[2] <= 0.0)
            return 1;
    }
    return 0;
}

void __cdecl R_ResetShadowCookies()
{
    shadowCookieGlob.weightCap = 10.0;
    shadowCookieGlob.lastTime = -1;
}

