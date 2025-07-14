#include "cg_local.h"
#include "cg_public.h"

#include <stringed/stringed_hooks.h>

#include <string.h>
#include <client_mp/client_mp.h>
#include <client/client.h>
#include <cgame_mp/cg_local_mp.h>
#include <EffectsCore/fx_system.h>
#include <universal/profile.h>

const float s_alignScale[4] = { 0.0, 0.5, 1.0, 0.0 }; // idb
float glowColor[4]; // KISAKTODO: check for duplicates more

const dvar_t *waypointOffscreenScaleSmallest;
const dvar_t *waypointPlayerOffsetStand;
const dvar_t *waypointTweakY;
const dvar_t *waypointOffscreenPointerHeight;
const dvar_t *waypointOffscreenPadTop;
const dvar_t *waypointIconHeight;
const dvar_t *waypointDistScaleRangeMin;
const dvar_t *waypointOffscreenRoundedCorners;
const dvar_t *waypointOffscreenPadBottom;
const dvar_t *waypointPlayerOffsetProne;
const dvar_t *waypointOffscreenPointerWidth;
const dvar_t *waypointIconWidth;
const dvar_t *waypointSplitscreenScale;
const dvar_t *waypointPlayerOffsetCrouch;
const dvar_t *waypointOffscreenPadRight;
const dvar_t *waypointOffscreenCornerRadius;
const dvar_t *waypointDebugDraw;
const dvar_t *waypointDistScaleRangeMax;
const dvar_t *waypointOffscreenScaleLength;
const dvar_t *waypointDistScaleSmallest;
const dvar_t *waypointOffscreenPadLeft;
const dvar_t *waypointOffscreenPointerDistance;
const dvar_t *waypointOffscreenDistanceThresholdAlpha;

const dvar_t *hudElemPausedBrightness;


void __cdecl CG_HudElemRegisterDvars()
{
    DvarLimits min; // [esp+4h] [ebp-10h]
    DvarLimits mina; // [esp+4h] [ebp-10h]
    DvarLimits minb; // [esp+4h] [ebp-10h]
    DvarLimits minc; // [esp+4h] [ebp-10h]
    DvarLimits mind; // [esp+4h] [ebp-10h]
    DvarLimits mine; // [esp+4h] [ebp-10h]
    DvarLimits minf; // [esp+4h] [ebp-10h]
    DvarLimits ming; // [esp+4h] [ebp-10h]
    DvarLimits minh; // [esp+4h] [ebp-10h]
    DvarLimits mini; // [esp+4h] [ebp-10h]
    DvarLimits minj; // [esp+4h] [ebp-10h]
    DvarLimits mink; // [esp+4h] [ebp-10h]
    DvarLimits minl; // [esp+4h] [ebp-10h]
    DvarLimits minm; // [esp+4h] [ebp-10h]
    DvarLimits minn; // [esp+4h] [ebp-10h]
    DvarLimits mino; // [esp+4h] [ebp-10h]
    DvarLimits minp; // [esp+4h] [ebp-10h]
    DvarLimits minq; // [esp+4h] [ebp-10h]
    DvarLimits minr; // [esp+4h] [ebp-10h]
    DvarLimits mins; // [esp+4h] [ebp-10h]
    DvarLimits mint; // [esp+4h] [ebp-10h]
    DvarLimits minu; // [esp+4h] [ebp-10h]

    waypointDebugDraw = Dvar_RegisterBool("waypointDebugDraw", 0, DVAR_NOFLAG, "");
    min.value.max = FLT_MAX;
    min.value.min = 1.1754944e-38f;
    waypointIconWidth = Dvar_RegisterFloat("waypointIconWidth", 36.0f, min, DVAR_NOFLAG, "Width of the offscreen pointer.");
    mina.value.max = FLT_MAX;
    mina.value.min = 1.1754944e-38f;
    waypointIconHeight = Dvar_RegisterFloat("waypointIconHeight", 36.0f, mina, DVAR_NOFLAG, "Height of the offscreen pointer.");
    minb.value.max = FLT_MAX;
    minb.value.min = 1.1754944e-38f;
    waypointOffscreenPointerWidth = Dvar_RegisterFloat(
        "waypointOffscreenPointerWidth",
        25.0f,
        minb,
        DVAR_NOFLAG,
        "Width of the offscreen pointer.");
    minc.value.max = FLT_MAX;
    minc.value.min = 1.1754944e-38f;
    waypointOffscreenPointerHeight = Dvar_RegisterFloat(
        "waypointOffscreenPointerHeight",
        12.0f,
        minc,
        DVAR_NOFLAG,
        "Height of the offscreen pointer.");
    mind.value.max = FLT_MAX;
    mind.value.min = 1.1754944e-38f;
    waypointOffscreenPointerDistance = Dvar_RegisterFloat(
        "waypointOffscreenPointerDistance",
        30.0f,
        mind,
        DVAR_NOFLAG,
        "Distance from the center of the offscreen objective icon to the center its arrow.");
    mine.value.max = FLT_MAX;
    mine.value.min = 0.0f;
    waypointOffscreenDistanceThresholdAlpha = Dvar_RegisterFloat(
        "waypointOffscreenDistanceThresholdAlpha",
        30.0f,
        mine,
        DVAR_NOFLAG,
        "Distance from the threshold over which offscreen objective icons lerp their alpha.");
    minf.value.max = FLT_MAX;
    minf.value.min = 0.0f;
    waypointOffscreenPadLeft = Dvar_RegisterFloat("waypointOffscreenPadLeft", 103.0f, minf, DVAR_NOFLAG, "Offset from the edge.");
    ming.value.max = FLT_MAX;
    ming.value.min = 0.0f;
    waypointOffscreenPadRight = Dvar_RegisterFloat("waypointOffscreenPadRight", 0.0f, ming, DVAR_NOFLAG, "Offset from the edge.");
    minh.value.max = FLT_MAX;
    minh.value.min = 0.0f;
    waypointOffscreenPadTop = Dvar_RegisterFloat("waypointOffscreenPadTop", 0.0f, minh, DVAR_NOFLAG, "Offset from the edge.");
    mini.value.max = FLT_MAX;
    mini.value.min = 0.0f;
    waypointOffscreenPadBottom = Dvar_RegisterFloat("waypointOffscreenPadBottom", 30.0f, mini, DVAR_NOFLAG, "Offset from the edge.");
    waypointOffscreenRoundedCorners = Dvar_RegisterBool(
        "waypointOffscreenRoundedCorners",
        1,
        DVAR_NOFLAG,
        "Off-screen icons take rounded corners when true.  90-degree corners when false.");
    minj.value.max = FLT_MAX;
    minj.value.min = 0.0f;
    waypointOffscreenCornerRadius = Dvar_RegisterFloat(
        "waypointOffscreenCornerRadius",
        105.0f,
        minj,
        DVAR_NOFLAG,
        "Size of the rounded corners.");
    mink.value.max = FLT_MAX;
    mink.value.min = 1.1754944e-38f;
    waypointOffscreenScaleLength = Dvar_RegisterFloat(
        "waypointOffscreenScaleLength",
        500.0f,
        mink,
        DVAR_NOFLAG,
        "How far the offscreen icon scale travels from full to smallest scale.");
    minl.value.max = FLT_MAX;
    minl.value.min = 0.0f;
    waypointOffscreenScaleSmallest = Dvar_RegisterFloat(
        "waypointOffscreenScaleSmallest",
        1.0f,
        minl,
        DVAR_NOFLAG,
        "Smallest scale that the offscreen effect uses.");
    minm.value.max = FLT_MAX;
    minm.value.min = 0.0f;
    waypointDistScaleRangeMin = Dvar_RegisterFloat(
        "waypointDistScaleRangeMin",
        1000.0f,
        minm,
        DVAR_NOFLAG,
        "Distance from player that icon distance scaling starts.");
    minn.value.max = FLT_MAX;
    minn.value.min = 0.0f;
    waypointDistScaleRangeMax = Dvar_RegisterFloat(
        "waypointDistScaleRangeMax",
        3000.0f,
        minn,
        DVAR_NOFLAG,
        "Distance from player that icon distance scaling ends.");
    mino.value.max = FLT_MAX;
    mino.value.min = 0.0f;
    waypointDistScaleSmallest = Dvar_RegisterFloat(
        "waypointDistScaleSmallest",
        0.80000001f,
        mino,
        DVAR_NOFLAG,
        "Smallest scale that the distance effect uses.");
    minp.value.max = FLT_MAX;
    minp.value.min = 0.1f;
    waypointSplitscreenScale = Dvar_RegisterFloat(
        "waypointSplitscreenScale",
        1.8f,
        minp,
        DVAR_NOFLAG,
        "Scale applied to waypoint icons in splitscreen views.");
    minq.value.max = FLT_MAX;
    minq.value.min = -FLT_MAX;
    waypointTweakY = Dvar_RegisterFloat("waypointTweakY", -17.0f, minq, 0, "");
    minr.value.max = 1.0f;
    minr.value.min = 0.0f;
    hudElemPausedBrightness = Dvar_RegisterFloat(
        "hudElemPausedBrightness",
        0.40000001f,
        minr,
        DVAR_CHEAT,
        "Brightness of the hudelems when the game is paused.");
    mins.value.max = FLT_MAX;
    mins.value.min = 0.0f;
    waypointPlayerOffsetProne = Dvar_RegisterFloat(
        "waypointPlayerOffsetProne",
        30.0f,
        mins,
        DVAR_CHEAT,
        "For waypoints pointing to players, how high to offset off of their origin when they are prone.");
    mint.value.max = FLT_MAX;
    mint.value.min = 0.0f;
    waypointPlayerOffsetCrouch = Dvar_RegisterFloat(
        "waypointPlayerOffsetCrouch",
        56.0f,
        mint,
        DVAR_CHEAT,
        "For waypoints pointing to players, how high to offset off of their origin when they are crouching.");
    minu.value.max = FLT_MAX;
    minu.value.min = 0.0f;
    waypointPlayerOffsetStand = Dvar_RegisterFloat(
        "waypointPlayerOffsetStand",
        74.0f,
        minu,
        DVAR_CHEAT,
        "For waypoints pointing to players, how high to offset off of their origin when they are standing.");
}

void __cdecl CG_TranslateHudElemMessage(
    int32_t localClientNum,
    const char *message,
    const char *messageType,
    char *hudElemString)
{
    const char *v4; // eax
    char *translatedString; // [esp+10h] [ebp-Ch]
    uint32_t stringLen; // [esp+14h] [ebp-8h] BYREF
    uint32_t searchPos; // [esp+18h] [ebp-4h] BYREF

    if (!message)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 274, 0, "%s", "message");
    if (!hudElemString)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 275, 0, "%s", "hudElemString");
    translatedString = SEH_LocalizeTextMessage(message, messageType, LOCMSG_SAFE);
    stringLen = strlen(translatedString);
    if (stringLen + 1 <= 0x100)
    {
        memcpy((uint8_t *)hudElemString, (uint8_t *)translatedString, stringLen);
        hudElemString[stringLen] = 0;
        searchPos = 0;
        while (ReplaceDirective(localClientNum, &searchPos, &stringLen, hudElemString))
            ;
    }
    else
    {
        v4 = va("Translated message too long to process: %s\n", message);
        Com_Error(ERR_DROP, v4);
    }
}

char __cdecl ReplaceDirective(int32_t localClientNum, uint32_t *searchPos, uint32_t *dstLen, char *dstString)
{
    const char *v4; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    int32_t directiveLen; // [esp+34h] [ebp-324h]
    const char *startTokenPos; // [esp+38h] [ebp-320h]
    int32_t newStringLen; // [esp+3Ch] [ebp-31Ch]
    uint32_t bindingLen; // [esp+40h] [ebp-318h]
    uint32_t endLen; // [esp+44h] [ebp-314h]
    char keyBinding[256]; // [esp+48h] [ebp-310h] BYREF
    uint8_t srcString[260]; // [esp+148h] [ebp-210h] BYREF
    int32_t beginLen; // [esp+24Ch] [ebp-10Ch]
    char directive[256]; // [esp+250h] [ebp-108h] BYREF
    const char *endTokenPos; // [esp+354h] [ebp-4h]
    char *dstStringa; // [esp+36Ch] [ebp+14h]
    uint8_t *dstStringb; // [esp+36Ch] [ebp+14h]

    if (!searchPos)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 199, 0, "%s", "searchPos");
    if (!dstLen)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 201, 0, "%s", "dstLen");
    if (*dstLen >= 0x100)
        MyAssertHandler(
            ".\\cgame\\cg_hudelem.cpp",
            202,
            0,
            "*dstLen doesn't index MAX_HUDELEM_TEXT_LEN\n\t%i not in [0, %i)",
            *dstLen,
            256);
    if (*searchPos > *dstLen)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 203, 0, "*searchPos <= *dstLen\n\t%i, %i", *searchPos, *dstLen);
    if (!dstString)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 204, 0, "%s", "dstString");
    if (*dstLen != strlen(dstString))
        MyAssertHandler(
            ".\\cgame\\cg_hudelem.cpp",
            205,
            0,
            "*dstLen == strlen( dstString )\n\t%i, %i",
            *dstLen,
            strlen(dstString));
    memcpy(srcString, (uint8_t *)dstString, *dstLen);
    srcString[*dstLen] = 0;
    v4 = strstr((const char*)&srcString[*searchPos], "[{");
    startTokenPos = (const char *)v4;
    if (!v4)
        return 0;
    v6 = strstr((const char*)v4, "}]");
    endTokenPos = v6;
    if (v6)
    {
        directiveLen = endTokenPos - startTokenPos - 2;
        if (directiveLen < 0)
            MyAssertHandler(
                ".\\cgame\\cg_hudelem.cpp",
                224,
                1,
                "%s\n\t(directiveLen) = %i",
                "(directiveLen >= 0)",
                directiveLen);
        if (directiveLen)
        {
            memcpy((uint8_t *)directive, (uint8_t *)startTokenPos + 2, directiveLen);
            directive[directiveLen] = 0;
            GetHudelemDirective(localClientNum, directive, keyBinding);
            bindingLen = &keyBinding[strlen(keyBinding) + 1] - &keyBinding[1];
            newStringLen = *dstLen - directiveLen + bindingLen - 4;
            if (newStringLen + 1 <= 256)
            {
                beginLen = startTokenPos - (const char *)srcString;
                dstStringa = &dstString[startTokenPos - (const char *)srcString];
                memcpy((uint8_t *)dstStringa, (uint8_t *)keyBinding, bindingLen);
                dstStringb = (uint8_t *)&dstStringa[bindingLen];
                endLen = newStringLen - beginLen - bindingLen;
                memcpy(dstStringb, (uint8_t *)endTokenPos + 2, endLen);
                dstStringb[endLen] = 0;
                *searchPos = bindingLen + beginLen;
                *dstLen = newStringLen;
                if (*searchPos > *dstLen)
                    MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 262, 1, "*searchPos <= *dstLen\n\t%i, %i", *searchPos, *dstLen);
                return 1;
            }
            else
            {
                v9 = va("String too long to add key binding: %s\n", dstString);
                Com_Error(ERR_DROP, v9);
                return 0;
            }
        }
        else
        {
            v8 = va("Directive empty in string '%s'", dstString);
            Com_Error(ERR_DROP, v8);
            return 0;
        }
    }
    else
    {
        v7 = va("No end token to match begin token in string '%s'", dstString);
        Com_Error(ERR_DROP, v7);
        return 0;
    }
}

void __cdecl GetHudelemDirective(int32_t localClientNum, char *directive, char *result)
{
    char *v3; // eax
    char arg0[256]; // [esp+0h] [ebp-208h] BYREF
    char name[260]; // [esp+100h] [ebp-108h] BYREF

    if (UI_GetKeyBindingLocalizedString(localClientNum, directive, result))
    {
        KISAK_NULLSUB();
    }
    else
    {
        ParseDirective(directive, name, arg0);
        if (I_stricmp(name, "FAKE_INTRO_SECONDS"))
        {
            v3 = UI_SafeTranslateString("KEY_UNBOUND");
            Com_sprintf(result, 0x100u, "%s(%s)", v3, directive);
        }
        else
        {
            DirectiveFakeIntroSeconds(localClientNum, arg0, result);
        }
    }
}

void __cdecl DirectiveFakeIntroSeconds(int32_t localClientNum, const char *arg0, char *result)
{
    int32_t fakeSeconds; // [esp+4h] [ebp-4h] BYREF

    fakeSeconds = 0;
    fakeSeconds = (int)strtol(arg0, NULL, 10);
    if ((unsigned int)fakeSeconds > 0x28)
    {
        fakeSeconds = 0;
        Com_PrintWarning(
            1,
            "Argument \"%s\" given for FAKE_INTRO_SECONDS is outside the acceptible range of (%d,%d).\n",
            arg0,
            0,
            40);
    }
    Com_sprintf(result, 4u, "%02d", fakeSeconds + CG_GetLocalClientGlobals(localClientNum)->time / 1000);
}

void __cdecl ParseDirective(char *directive, char *resultName, char *resultArg0)
{
    const char *v3; // eax
    const char *argpos; // [esp+4h] [ebp-4h]

    if (!directive)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 139, 0, "%s", "directive");
    v3 = strstr((char *)directive, (char*)":");
    argpos = v3;
    if (v3)
    {
        memcpy((uint8_t *)resultName, (uint8_t *)directive, v3 - directive);
        resultName[argpos - directive] = 0;
        I_strncpyz(resultArg0, (char *)argpos + 1, 256);
    }
    else
    {
        I_strncpyz(resultName, directive, 256);
        *resultArg0 = 0;
    }
}

void __cdecl CG_Draw2dHudElems(int32_t localClientNum, int32_t foreground)
{
    PROF_SCOPED("CG_Draw2dHudElems");

    bool v2; // [esp+7h] [ebp-100Dh]
    int32_t i; // [esp+8h] [ebp-100Ch]
    hudelem_s *elems[1025]; // [esp+Ch] [ebp-1008h] BYREF
    int32_t SortedHudElems; // [esp+1010h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    SortedHudElems = GetSortedHudElems(localClientNum, elems);

    if (SortedHudElems)
    {
        v2 = cgameGlob->nextSnap->ps.pm_type < PM_DEAD;
        for (i = 0; i < SortedHudElems; ++i)
        {
            if ((v2 || (elems[i]->flags & 2) == 0)
                && (!foreground || (elems[i]->flags & 1) != 0)
                && (foreground || (elems[i]->flags & 1) == 0)
                && ((elems[i]->flags & 4) == 0 || !UI_AnyMenuVisible(localClientNum)))
            {
                DrawSingleHudElem2d(localClientNum, elems[i]);
            }
        }
    }
}

void __cdecl DrawSingleHudElem2d(int32_t localClientNum, const hudelem_s *elem)
{
    char hudElemString[256]; // [esp+8h] [ebp-340h] BYREF
    cg_hudelem_t cghe; // [esp+108h] [ebp-240h] BYREF
    hudelem_color_t toColor; // [esp+344h] [ebp-4h] BYREF
    cg_s *cgameGlob; 

    if (elem->type == HE_TYPE_WAYPOINT)
    {
        DrawOffscreenViewableWaypoint(localClientNum, elem);
    }
    else
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        cghe.timeNow = cgameGlob->time;
        iassert(elem->fadeStartTime <= CG_GetLocalClientGlobals(localClientNum)->nextSnap->serverTime);
        BG_LerpHudColors(elem, cghe.timeNow, &toColor);

        if (toColor.a)
        {
            HudElemColorToVec4(&toColor, cghe.color);
            GetHudElemInfo(localClientNum, elem, &cghe, hudElemString);
            if (cghe.hudElemLabel[0])
            {
                DrawHudElemString(localClientNum, &scrPlaceView[localClientNum], cghe.hudElemLabel, elem, &cghe);
                cghe.x = cghe.x + cghe.labelWidth;
            }
            switch (elem->type)
            {
            case HE_TYPE_TEXT:
            case HE_TYPE_VALUE:
            case HE_TYPE_PLAYERNAME:
            case HE_TYPE_MAPNAME:
            case HE_TYPE_GAMETYPE:
            case HE_TYPE_TIMER_DOWN:
            case HE_TYPE_TIMER_UP:
            case HE_TYPE_TENTHS_TIMER_DOWN:
            case HE_TYPE_TENTHS_TIMER_UP:
                if (cghe.hudElemText[0])
                    DrawHudElemString(localClientNum, &scrPlaceView[localClientNum], cghe.hudElemText, elem, &cghe);
                break;
            case HE_TYPE_MATERIAL:
                DrawHudElemMaterial(localClientNum, elem, &cghe);
                break;
            case HE_TYPE_CLOCK_DOWN:
            case HE_TYPE_CLOCK_UP:
                DrawHudElemClock(localClientNum, elem, &cghe);
                break;
            default:
                if (!alwaysfails)
                    MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 1387, 0, "invalid case");
                break;
            }
        }
    }
}

void __cdecl GetHudElemInfo(int32_t localClientNum, const hudelem_s *elem, cg_hudelem_t *cghe, char *hudElemString)
{
    char *ConfigString; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    float v8; // [esp+10h] [ebp-30h]
    int32_t fontEnum; // [esp+28h] [ebp-18h]
    const ScreenPlacement *scrPlace; // [esp+2Ch] [ebp-14h]
    float baseFontScale; // [esp+30h] [ebp-10h]
    uint32_t namedClientIndex; // [esp+38h] [ebp-8h]

    scrPlace = &scrPlaceView[localClientNum];
    switch (elem->font)
    {
    case 0:
        goto $LN27_2;
    case 1:
        baseFontScale = 0.5;
        fontEnum = 4;
        break;
    case 2:
        baseFontScale = 0.33333334f;
        fontEnum = 5;
        break;
    case 3:
        baseFontScale = 0.25f;
        fontEnum = 6;
        break;
    case 4:
        baseFontScale = 0.25f;
        fontEnum = 2;
        break;
    case 5:
        baseFontScale = 0.25f;
        fontEnum = 3;
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 752, 0, "invalid case");
    $LN27_2:
        baseFontScale = 0.25f;
        fontEnum = 0;
        break;
    }
    v8 = baseFontScale * elem->fontScale;
    cghe->fontScale = scrPlace->scaleVirtualToReal[1] * v8;
    cghe->font = UI_GetFontHandle(scrPlace, fontEnum, cghe->fontScale);
    cghe->fontHeight = (float)UI_TextHeight(cghe->font, cghe->fontScale);
    cghe->hudElemLabel[0] = 0;
    if (elem->label)
        SafeTranslateHudElemString(localClientNum, elem->label, cghe->hudElemLabel);
    cghe->hudElemText[0] = 0;
    switch (elem->type)
    {
    case HE_TYPE_TEXT:
        if (elem->text)
            SafeTranslateHudElemString(localClientNum, elem->text, cghe->hudElemText);
        break;
    case HE_TYPE_VALUE:
        Com_sprintf(cghe->hudElemText, 0x100u, "%g", elem->value);
        break;
    case HE_TYPE_PLAYERNAME:
        namedClientIndex = (int)(elem->value);
        if (namedClientIndex < 0x40)
        {
            I_strncpyz(cghe->hudElemText, CG_GetLocalClientGlobals(localClientNum)->bgs.clientinfo[namedClientIndex].name, 256);
        }
        break;
    case HE_TYPE_MAPNAME:
        ConfigString = CL_GetConfigString(localClientNum, 0x11u);
        I_strncpyz(cghe->hudElemText, ConfigString, 256);
        break;
    case HE_TYPE_GAMETYPE:
        v5 = CL_GetConfigString(localClientNum, 0x12u);
        I_strncpyz(cghe->hudElemText, v5, 256);
        break;
    case HE_TYPE_TIMER_DOWN:
    case HE_TYPE_TIMER_UP:
        v6 = HudElemTimerString(elem, cghe->timeNow);
        CopyStringToHudElemString(v6, cghe->hudElemText);
        break;
    case HE_TYPE_TENTHS_TIMER_DOWN:
    case HE_TYPE_TENTHS_TIMER_UP:
        v7 = HudElemTenthsTimerString(elem, cghe->timeNow);
        CopyStringToHudElemString(v7, cghe->hudElemText);
        break;
    default:
        break;
    }
    if (cghe->hudElemLabel[0] && cghe->hudElemText[0])
        ConsolidateHudElemText(cghe, hudElemString);
    if (cghe->hudElemLabel[0])
        cghe->labelWidth = HudElemStringWidth(cghe->hudElemLabel, cghe);
    else
        cghe->labelWidth = 0.0f;
    if (cghe->hudElemText[0])
        cghe->textWidth = HudElemStringWidth(cghe->hudElemText, cghe);
    else
        cghe->textWidth = 0.0f;
    cghe->width = HudElemWidth(scrPlace, elem, cghe);
    cghe->height = HudElemHeight(scrPlace, elem, cghe);
    SetHudElemPos(&scrPlaceView[localClientNum], elem, cghe);
}

void __cdecl SafeTranslateHudElemString(int32_t localClientNum, int32_t index, char *hudElemString)
{
    char *ConfigString; // eax

    if (!hudElemString)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 301, 0, "%s", "hudElemString");
    if (index)
    {
        ConfigString = CL_GetConfigString(localClientNum, index + 309);
        CG_TranslateHudElemMessage(localClientNum, ConfigString, "hudelem string", hudElemString);
    }
}

double __cdecl HudElemStringWidth(const char *string, const cg_hudelem_t *cghe)
{
    double v4; // [esp+8h] [ebp-Ch]

    v4 = (double)UI_TextWidth(string, 0, cghe->font, cghe->fontScale);
    return (float)(v4 / CL_GetScreenAspectRatioDisplayPixel());
}

char *__cdecl HudElemTimerString(const hudelem_s *elem, int32_t timeNow)
{
    int32_t HudElemTime; // eax
    int32_t hours; // [esp+0h] [ebp-10h]
    int32_t seconds; // [esp+4h] [ebp-Ch]
    int32_t minutes; // [esp+8h] [ebp-8h]

    HudElemTime = GetHudElemTime(elem, timeNow);
    hours = HudElemTime / 1000 / 3600;
    minutes = HudElemTime / 1000 % 3600 / 60;
    seconds = HudElemTime / 1000 % 3600 % 60;
    if (hours)
        return va("%i:%02i:%02i", hours, minutes, seconds);
    else
        return va("%i:%02i", minutes, seconds);
}

int32_t __cdecl GetHudElemTime(const hudelem_s *elem, int32_t timeNow)
{
    int32_t result; // eax
    int32_t time; // [esp+4h] [ebp-4h]

    switch (elem->type)
    {
    case HE_TYPE_TIMER_DOWN:
        time = elem->time - timeNow + 999;
        goto LABEL_9;
    case HE_TYPE_TIMER_UP:
    case HE_TYPE_TENTHS_TIMER_UP:
    case HE_TYPE_CLOCK_UP:
        time = timeNow - elem->time;
        goto LABEL_9;
    case HE_TYPE_TENTHS_TIMER_DOWN:
        time = elem->time - timeNow + 99;
        goto LABEL_9;
    case HE_TYPE_CLOCK_DOWN:
        time = elem->time - timeNow;
    LABEL_9:
        if (time < 0)
            time = 0;
        result = time;
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 341, 0, "invalid case");
        result = 0;
        break;
    }
    return result;
}

char *__cdecl HudElemTenthsTimerString(const hudelem_s *elem, int32_t timeNow)
{
    int32_t HudElemTime; // eax
    int32_t hours; // [esp+0h] [ebp-14h]
    int32_t seconds; // [esp+4h] [ebp-10h]
    int32_t minutes; // [esp+8h] [ebp-Ch]
    int32_t tenths; // [esp+10h] [ebp-4h]

    HudElemTime = GetHudElemTime(elem, timeNow);
    hours = HudElemTime / 100 / 36000;
    minutes = HudElemTime / 100 % 36000 / 600;
    seconds = HudElemTime / 100 % 36000 % 600 / 10;
    tenths = HudElemTime / 100 % 36000 % 600 % 10;
    if (hours)
        return va("%i:%02i:%02i.%i", hours, minutes, seconds, tenths);
    else
        return va("%i:%02i.%i", minutes, seconds, tenths);
}

double __cdecl HudElemWidth(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe)
{
    double result; // st7
    float v4; // [esp+0h] [ebp-Ch]
    float v5; // [esp+4h] [ebp-8h]

    switch (elem->type)
    {
    case HE_TYPE_TEXT:
    case HE_TYPE_VALUE:
    case HE_TYPE_PLAYERNAME:
    case HE_TYPE_MAPNAME:
    case HE_TYPE_GAMETYPE:
    case HE_TYPE_TIMER_DOWN:
    case HE_TYPE_TIMER_UP:
    case HE_TYPE_TENTHS_TIMER_DOWN:
    case HE_TYPE_TENTHS_TIMER_UP:
        v5 = cghe->labelWidth + cghe->textWidth;
        result = v5;
        break;
    case HE_TYPE_MATERIAL:
    case HE_TYPE_CLOCK_DOWN:
    case HE_TYPE_CLOCK_UP:
        v4 = HudElemMaterialWidth(scrPlace, elem, cghe) + cghe->labelWidth;
        result = v4;
        break;
    case HE_TYPE_WAYPOINT:
        result = (double)elem->width;
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 494, 0, "invalid case");
        result = 0.0;
        break;
    }
    return result;
}

double __cdecl HudElemMaterialWidth(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe)
{
    float width; // [esp+1Ch] [ebp-10h]
    float lerp; // [esp+20h] [ebp-Ch]
    int32_t deltaTime; // [esp+24h] [ebp-8h]
    float fromWidth; // [esp+28h] [ebp-4h]

    width = HudElemMaterialSpecifiedWidth(scrPlace, elem->alignScreen, elem->width, cghe);
    if (elem->scaleTime <= 0)
        return width;
    deltaTime = cghe->timeNow - elem->scaleStartTime;
    if (deltaTime >= elem->scaleTime)
        return width;
    fromWidth = HudElemMaterialSpecifiedWidth(scrPlace, elem->fromAlignScreen, elem->fromWidth, cghe);
    if (deltaTime <= 0)
        return fromWidth;
    lerp = (double)deltaTime / (double)elem->scaleTime;
    if (lerp < 0.0 || lerp > 1.0)
        MyAssertHandler(
            ".\\cgame\\cg_hudelem.cpp",
            439,
            1,
            "lerp not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
            lerp,
            0.0,
            1.0);
    return (float)((width - fromWidth) * lerp + fromWidth);
}

double __cdecl HudElemMaterialSpecifiedWidth(
    const ScreenPlacement *scrPlace,
    char alignScreen,
    int32_t sizeVirtual,
    const cg_hudelem_t *cghe)
{
    if (!sizeVirtual)
        return cghe->fontHeight;
    if ((alignScreen & 0x38) == 0x20)
        return (float)(scrPlace->scaleVirtualToFull[0] * (double)sizeVirtual);
    else
        return (float)(scrPlace->scaleVirtualToReal[0] * (double)sizeVirtual);
}

double __cdecl HudElemHeight(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe)
{
    double result; // st7
    float height; // [esp+4h] [ebp-4h]

    switch (elem->type)
    {
    case HE_TYPE_TEXT:
    case HE_TYPE_VALUE:
    case HE_TYPE_PLAYERNAME:
    case HE_TYPE_MAPNAME:
    case HE_TYPE_GAMETYPE:
    case HE_TYPE_TIMER_DOWN:
    case HE_TYPE_TIMER_UP:
    case HE_TYPE_TENTHS_TIMER_DOWN:
    case HE_TYPE_TENTHS_TIMER_UP:
        height = cghe->fontHeight;
        goto LABEL_8;
    case HE_TYPE_MATERIAL:
    case HE_TYPE_CLOCK_DOWN:
    case HE_TYPE_CLOCK_UP:
        height = HudElemMaterialHeight(scrPlace, elem, cghe);
        goto LABEL_8;
    case HE_TYPE_WAYPOINT:
        height = (float)elem->height;
    LABEL_8:
        if (cghe != (const cg_hudelem_t *)-16 && cghe->fontHeight > (double)height)
            height = cghe->fontHeight;
        result = height;
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 530, 0, "invalid case");
        result = 0.0;
        break;
    }
    return result;
}

double __cdecl HudElemMaterialHeight(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe)
{
    float height; // [esp+1Ch] [ebp-10h]
    float lerp; // [esp+20h] [ebp-Ch]
    int32_t deltaTime; // [esp+24h] [ebp-8h]
    float fromHeight; // [esp+28h] [ebp-4h]

    height = HudElemMaterialSpecifiedHeight(scrPlace, elem->alignScreen, elem->height, cghe);
    if (elem->scaleTime <= 0)
        return height;
    deltaTime = cghe->timeNow - elem->scaleStartTime;
    if (deltaTime >= elem->scaleTime)
        return height;
    fromHeight = HudElemMaterialSpecifiedHeight(scrPlace, elem->fromAlignScreen, elem->fromHeight, cghe);
    if (deltaTime <= 0)
        return fromHeight;
    lerp = (double)deltaTime / (double)elem->scaleTime;
    if (lerp < 0.0 || lerp > 1.0)
        MyAssertHandler(
            ".\\cgame\\cg_hudelem.cpp",
            464,
            1,
            "lerp not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
            lerp,
            0.0,
            1.0);
    return (float)((height - fromHeight) * lerp + fromHeight);
}

double __cdecl HudElemMaterialSpecifiedHeight(
    const ScreenPlacement *scrPlace,
    char alignScreen,
    int32_t sizeVirtual,
    const cg_hudelem_t *cghe)
{
    if (!sizeVirtual)
        return cghe->fontHeight;
    if ((alignScreen & 7) == 4)
        return (float)(scrPlace->scaleVirtualToFull[1] * (double)sizeVirtual);
    else
        return (float)(scrPlace->scaleVirtualToReal[1] * (double)sizeVirtual);
}

void __cdecl SetHudElemPos(const ScreenPlacement *scrPlace, const hudelem_s *elem, cg_hudelem_t *cghe)
{
    float from[2]; // [esp+40h] [ebp-14h] BYREF
    float lerp; // [esp+48h] [ebp-Ch]
    float to[2]; // [esp+4Ch] [ebp-8h] BYREF

    if (!elem)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 632, 0, "%s", "elem");
    if (!cghe)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 633, 0, "%s", "cghe");
    lerp = HudElemMovementFrac(elem, cghe->timeNow);
    if (lerp == 1.0)
    {
        GetHudElemOrg(
            scrPlace,
            elem->alignOrg,
            elem->alignScreen,
            elem->x,
            elem->y,
            cghe->width,
            cghe->height,
            &cghe->x,
            &cghe->y);
        cghe->x = (float)(int)(cghe->x);
        cghe->y = (float)(int)(cghe->y);
    }
    else
    {
        GetHudElemOrg(
            scrPlace,
            elem->fromAlignOrg,
            elem->fromAlignScreen,
            elem->fromX,
            elem->fromY,
            cghe->width,
            cghe->height,
            from,
            &from[1]);
        GetHudElemOrg(scrPlace, elem->alignOrg, elem->alignScreen, elem->x, elem->y, cghe->width, cghe->height, to, &to[1]);
        cghe->x = (to[0] - from[0]) * lerp + from[0];
        cghe->y = (to[1] - from[1]) * lerp + from[1];
    }
}

void __cdecl GetHudElemOrg(
    const ScreenPlacement *scrPlace,
    int32_t alignOrg,
    int32_t alignScreen,
    float xVirtual,
    float yVirtual,
    float width,
    float height,
    float *orgX,
    float *orgY)
{
    float x; // [esp+18h] [ebp-8h]
    float y; // [esp+1Ch] [ebp-4h]

    if (!orgX)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 582, 0, "%s", "orgX");
    if (!orgY)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 583, 0, "%s", "orgY");
    x = ScrPlace_ApplyX(scrPlace, xVirtual, (alignScreen >> 3) & 7);
    *orgX = AlignHudElemX(alignOrg, x, width);
    y = ScrPlace_ApplyY(scrPlace, yVirtual, alignScreen & 7);
    *orgY = AlignHudElemY(alignOrg, y, height);
}

double __cdecl AlignHudElemX(int32_t alignOrg, float x, float width)
{
    uint32_t alignX; // [esp+4h] [ebp-4h]

    alignX = (alignOrg >> 2) & 3;
    if (alignX > 2)
        MyAssertHandler(
            ".\\cgame\\cg_hudelem.cpp",
            558,
            0,
            "%s\n\t(alignOrg) = %i",
            "(alignX == 0 || alignX == 1 || alignX == 2)",
            alignOrg);
    return (float)(x - width * s_alignScale[alignX]);
}

double __cdecl AlignHudElemY(int32_t alignOrg, float y, float height)
{
    int32_t alignY; // [esp+4h] [ebp-4h]

    alignY = alignOrg & 3;
    if ((alignOrg & 3) != 0 && alignY != 1 && alignY != 2)
        MyAssertHandler(
            ".\\cgame\\cg_hudelem.cpp",
            569,
            0,
            "%s\n\t(alignOrg) = %i",
            "(alignY == 0 || alignY == 1 || alignY == 2)",
            alignOrg);
    return (float)(y - height * s_alignScale[alignY]);
}

double __cdecl HudElemMovementFrac(const hudelem_s *elem, int32_t timeNow)
{
    int32_t time; // [esp+4h] [ebp-4h]

    if (elem->moveTime <= 0)
        return 1.0;
    time = timeNow - elem->moveStartTime;
    if (time <= 0)
        return 0.0;
    if (time >= elem->moveTime)
        return 1.0;
    return (float)((double)time / (double)elem->moveTime);
}

void __cdecl ConsolidateHudElemText(cg_hudelem_t *cghe, char *hudElemString)
{
    int32_t len; // [esp+8h] [ebp-Ch]
    int32_t textIndex; // [esp+Ch] [ebp-8h]
    int32_t labelIndex; // [esp+10h] [ebp-4h]

    len = 0;
    for (labelIndex = 0; len < 255 && cghe->hudElemLabel[labelIndex]; ++labelIndex)
    {
        if (cghe->hudElemLabel[labelIndex] == '&'
            && cghe->hudElemLabel[labelIndex + 1] == '&'
            && cghe->hudElemLabel[labelIndex + 2] == '1')
        {
            labelIndex += 3;
            break;
        }
        hudElemString[len++] = cghe->hudElemLabel[labelIndex];
    }
    for (textIndex = 0; len < 255 && cghe->hudElemText[textIndex]; ++textIndex)
        hudElemString[len++] = cghe->hudElemText[textIndex];
    while (len < 255 && cghe->hudElemLabel[labelIndex])
        hudElemString[len++] = cghe->hudElemLabel[labelIndex++];
    hudElemString[len] = 0;
    memcpy(cghe->hudElemText, hudElemString, sizeof(cghe->hudElemText));
    cghe->textWidth = HudElemStringWidth(cghe->hudElemText, cghe);
    cghe->hudElemLabel[0] = 0;
    cghe->labelWidth = 0.0;
}

void __cdecl CopyStringToHudElemString(char *string, char *hudElemString)
{
    int32_t v2; // ecx
    const char *v3; // eax
    int32_t stringLen; // [esp+10h] [ebp-4h]

    if (!string)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 718, 0, "%s", "string");
    if (!hudElemString)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 719, 0, "%s", "hudElemString");
    v2 = strlen(string);
    stringLen = v2;
    if (v2 < 256)
    {
        memcpy((uint8_t *)hudElemString, (uint8_t *)string, v2);
        hudElemString[stringLen] = 0;
    }
    else
    {
        v3 = va("Hud elem string too long, %s", string);
        Com_Error(ERR_DROP, v3);
    }
}

void __cdecl HudElemColorToVec4(const hudelem_color_t *hudElemColor, float *resultColor)
{
    if (!hudElemPausedBrightness)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 853, 0, "%s", "hudElemPausedBrightness");
    if (!cl_paused)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 854, 0, "%s", "cl_paused");
    *resultColor = (double)hudElemColor->r * 0.003921568859368563;
    resultColor[1] = (double)hudElemColor->g * 0.003921568859368563;
    resultColor[2] = (double)hudElemColor->b * 0.003921568859368563;
    resultColor[3] = (double)hudElemColor->a * 0.003921568859368563;
    if (cl_paused->current.integer)
        Vec3Scale(resultColor, hudElemPausedBrightness->current.value, resultColor);
    if (*resultColor < 0.0 || *resultColor > 1.000000953674316)
        MyAssertHandler(
            ".\\cgame\\cg_hudelem.cpp",
            864,
            0,
            "resultColor[0] not in [0.0f, 1.000001f]\n\t%g not in [%g, %g]",
            *resultColor,
            0.0,
            1.000000953674316);
    if (resultColor[1] < 0.0 || resultColor[1] > 1.000000953674316)
        MyAssertHandler(
            ".\\cgame\\cg_hudelem.cpp",
            865,
            0,
            "resultColor[1] not in [0.0f, 1.000001f]\n\t%g not in [%g, %g]",
            resultColor[1],
            0.0,
            1.000000953674316);
    if (resultColor[2] < 0.0 || resultColor[2] > 1.000000953674316)
        MyAssertHandler(
            ".\\cgame\\cg_hudelem.cpp",
            866,
            0,
            "resultColor[2] not in [0.0f, 1.000001f]\n\t%g not in [%g, %g]",
            resultColor[2],
            0.0,
            1.000000953674316);
    if (resultColor[3] < 0.0 || resultColor[3] > 1.000000953674316)
        MyAssertHandler(
            ".\\cgame\\cg_hudelem.cpp",
            867,
            0,
            "resultColor[3] not in [0.0f, 1.000001f]\n\t%g not in [%g, %g]",
            resultColor[3],
            0.0,
            1.000000953674316);
}

void __cdecl DrawHudElemString(
    uint32_t localClientNum,
    const ScreenPlacement *scrPlace,
    char *text,
    const hudelem_s *elem,
    cg_hudelem_t *cghe)
{
    float v5; // [esp+34h] [ebp-2Ch]
    float offsetY; // [esp+38h] [ebp-28h]
    float v7; // [esp+3Ch] [ebp-24h]
    int32_t strLength; // [esp+44h] [ebp-1Ch]
    float textScale; // [esp+48h] [ebp-18h]
    int32_t fxBirthTime; // [esp+4Ch] [ebp-14h]
    float y; // [esp+54h] [ebp-Ch]
    float scaleX; // [esp+58h] [ebp-8h]
    float dy; // [esp+5Ch] [ebp-4h]

    iassert(text);
    iassert(text[0]);
    iassert(elem);
    iassert(cghe);
    iassert(cghe->color[3]);

    offsetY = -(cghe->height - cghe->fontHeight);
    y = OffsetHudElemY(elem, cghe, offsetY);
    textScale = R_NormalizedTextScale(cghe->font, cghe->fontScale);
    v7 = textScale * scrPlace->scaleRealToVirtual[1];
    scaleX = scrPlace->scaleVirtualToReal[0] * v7;
    dy = cghe->fontHeight;
    fxBirthTime = elem->fxBirthTime;

    if (fxBirthTime && fxBirthTime > cghe->timeNow)
        fxBirthTime = cghe->timeNow;

    HudElemColorToVec4(&elem->glowColor, glowColor);

    if (fxBirthTime)
    {
        strLength = SEH_PrintStrlen(text);
        CL_PlayTextFXPulseSounds(
            localClientNum,
            cghe->timeNow,
            strLength,
            fxBirthTime,
            elem->fxLetterTime,
            elem->fxDecayStartTime,
            &CG_GetLocalClientGlobals(localClientNum)->hudElemSound[elem->soundID].lastPlayedTime);
    }

    v5 = y + dy;

    CL_DrawTextPhysicalWithEffects(
        text,
        0x7FFFFFFF,
        cghe->font,
        cghe->x,
        v5,
        scaleX,
        textScale,
        cghe->color,
        3,
        glowColor,
        cgMedia.textDecodeCharacters,
        cgMedia.textDecodeCharactersGlow,
        fxBirthTime,
        elem->fxLetterTime,
        elem->fxDecayStartTime,
        elem->fxDecayDuration);
}

double __cdecl OffsetHudElemY(const hudelem_s *elem, const cg_hudelem_t *cghe, float offsetY)
{
    float from; // [esp+18h] [ebp-Ch]
    float lerp; // [esp+1Ch] [ebp-8h]
    float to; // [esp+20h] [ebp-4h]

    lerp = HudElemMovementFrac(elem, cghe->timeNow);
    if (lerp == 1.0)
        return AlignHudElemY(elem->alignOrg, cghe->y, offsetY);
    from = AlignHudElemY(elem->fromAlignOrg, cghe->y, offsetY);
    to = AlignHudElemY(elem->alignOrg, cghe->y, offsetY);
    return (float)((from - to) * lerp + from);
}

void __cdecl DrawHudElemClock(int32_t localClientNum, const hudelem_s *elem, const cg_hudelem_t *cghe)
{
    float offsetY; // [esp+28h] [ebp-70h]
    float v4; // [esp+2Ch] [ebp-6Ch]
    float v5; // [esp+30h] [ebp-68h]
    float width; // [esp+34h] [ebp-64h]
    float height; // [esp+38h] [ebp-60h]
    float angle; // [esp+3Ch] [ebp-5Ch]
    char materialName[68]; // [esp+40h] [ebp-58h] BYREF
    Material *handMaterial; // [esp+88h] [ebp-10h]
    int32_t time; // [esp+8Ch] [ebp-Ch]
    Material *faceMaterial; // [esp+90h] [ebp-8h]
    float y; // [esp+94h] [ebp-4h]

    if (cghe->color[3] == 0.0)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 925, 0, "%s", "cghe->color[3]");
    if (CG_ServerMaterialName(localClientNum, elem->materialIndex, materialName, 0x3Au))
    {
        faceMaterial = Material_RegisterHandle(materialName, 7);
        I_strncat(materialName, 64, "needle");
        handMaterial = Material_RegisterHandle(materialName, 7);
        time = GetHudElemTime(elem, cghe->timeNow);
        if (elem->duration)
        {
            v5 = (double)time * 360.0 / (double)elem->duration;
            angle = AngleNormalize360(v5);
        }
        else
        {
            v4 = (double)time * 0.006000000052154064;
            angle = AngleNormalize360(v4);
        }
        width = HudElemMaterialWidth(&scrPlaceView[localClientNum], elem, cghe);
        height = HudElemMaterialHeight(&scrPlaceView[localClientNum], elem, cghe);
        offsetY = -(cghe->height - height);
        y = OffsetHudElemY(elem, cghe, offsetY);
        CL_DrawStretchPicPhysical(cghe->x, y, width, height, 0.0, 0.0, 1.0, 1.0, cghe->color, faceMaterial);
        CG_DrawRotatedPicPhysical(
            &scrPlaceView[localClientNum],
            cghe->x,
            y,
            width,
            height,
            angle,
            cghe->color,
            handMaterial);
    }
}

void __cdecl DrawHudElemMaterial(int32_t localClientNum, const hudelem_s *elem, cg_hudelem_t *cghe)
{
    float offsetY; // [esp+28h] [ebp-58h]
    Material *material; // [esp+2Ch] [ebp-54h]
    float width; // [esp+30h] [ebp-50h]
    float height; // [esp+34h] [ebp-4Ch]
    char materialName[64]; // [esp+38h] [ebp-48h] BYREF
    float y; // [esp+7Ch] [ebp-4h]

    if (cghe->color[3] == 0.0)
        MyAssertHandler(".\\cgame\\cg_hudelem.cpp", 956, 0, "%s", "cghe->color[3]");
    if (CG_ServerMaterialName(localClientNum, elem->materialIndex, materialName, 0x40u))
    {
        material = Material_RegisterHandle(materialName, 7);
        width = HudElemMaterialWidth(&scrPlaceView[localClientNum], elem, cghe);
        height = HudElemMaterialHeight(&scrPlaceView[localClientNum], elem, cghe);
        offsetY = -(cghe->height - height);
        y = OffsetHudElemY(elem, cghe, offsetY);
        CL_DrawStretchPicPhysical(cghe->x, y, width, height, 0.0, 0.0, 1.0, 1.0, cghe->color, material);
    }
}

void __cdecl DrawOffscreenViewableWaypoint(int32_t localClientNum, const hudelem_s *elem)
{
    double v2; // st7
    float v3; // [esp+2Ch] [ebp-118h]
    float v4; // [esp+30h] [ebp-114h]
    float x; // [esp+34h] [ebp-110h]
    float y; // [esp+38h] [ebp-10Ch]
    float v7; // [esp+3Ch] [ebp-108h]
    float v8; // [esp+44h] [ebp-100h]
    float v9; // [esp+50h] [ebp-F4h]
    float z; // [esp+54h] [ebp-F0h]
    float colorArrow[4]; // [esp+5Ch] [ebp-E8h] BYREF
    float shrinkDist; // [esp+6Ch] [ebp-D8h]
    float screenPosArrow[3]; // [esp+70h] [ebp-D4h]
    centity_s *cent; // [esp+7Ch] [ebp-C8h]
    float scaleVirtualToRealAvg; // [esp+80h] [ebp-C4h]
    float padTop; // [esp+84h] [ebp-C0h]
    float padBottom; // [esp+88h] [ebp-BCh]
    float iconWidth; // [esp+8Ch] [ebp-B8h]
    float pointerWidth; // [esp+90h] [ebp-B4h]
    float padding; // [esp+94h] [ebp-B0h]
    ScreenPlacement *scrPlace; // [esp+98h] [ebp-ACh]
    float clampedDist; // [esp+9Ch] [ebp-A8h] BYREF
    Material *material; // [esp+A0h] [ebp-A4h]
    float padRight; // [esp+A4h] [ebp-A0h]
    float iconHeight; // [esp+A8h] [ebp-9Ch]
    float angle; // [esp+ACh] [ebp-98h]
    float pointerHeight; // [esp+B0h] [ebp-94h]
    float clampedDir[2]; // [esp+B4h] [ebp-90h] BYREF
    char materialName[68]; // [esp+BCh] [ebp-88h] BYREF
    float distanceThresholdAlpha; // [esp+104h] [ebp-40h]
    float worldPos[3]; // [esp+108h] [ebp-3Ch] BYREF
    float tweak2dY; // [esp+114h] [ebp-30h]
    float padLeft; // [esp+118h] [ebp-2Ch]
    float screenPos[2]; // [esp+11Ch] [ebp-28h] BYREF
    float scale; // [esp+124h] [ebp-20h]
    hudelem_color_t toColor; // [esp+128h] [ebp-1Ch] BYREF
    float color[4]; // [esp+12Ch] [ebp-18h] BYREF
    bool didClamp; // [esp+13Fh] [ebp-5h]
    float pointerDistance; // [esp+140h] [ebp-4h]

    iassert(elem);
    iassert(elem->type == HE_TYPE_WAYPOINT);

    if (CG_ServerMaterialName(localClientNum, elem->offscreenMaterialIdx, materialName, 0x40u)
        && !CG_Flashbanged(localClientNum))
    {
        BG_LerpHudColors(elem, CG_GetLocalClientGlobals(localClientNum)->time, &toColor);
        if (toColor.a)
        {
            material = Material_RegisterHandle(materialName, 7);
            HudElemColorToVec4(&toColor, color);
            scrPlace = &scrPlaceView[localClientNum];
            scaleVirtualToRealAvg = (scrPlace->scaleVirtualToReal[0] + scrPlace->scaleVirtualToReal[1]) * 0.5;
            iconWidth = waypointIconWidth->current.value * scrPlace->scaleVirtualToReal[0];
            iconHeight = waypointIconHeight->current.value * scrPlace->scaleVirtualToReal[1];
            pointerWidth = waypointOffscreenPointerWidth->current.value * scrPlace->scaleVirtualToReal[0];
            pointerHeight = waypointOffscreenPointerHeight->current.value * scrPlace->scaleVirtualToReal[1];
            pointerDistance = waypointOffscreenPointerDistance->current.value * scaleVirtualToRealAvg;
            distanceThresholdAlpha = waypointOffscreenDistanceThresholdAlpha->current.value * scaleVirtualToRealAvg;
            if (distanceThresholdAlpha < 0.1f)
                distanceThresholdAlpha = 0.1f;
            padding = pointerHeight * 0.5f + pointerDistance;
            padLeft = waypointOffscreenPadLeft->current.value * scrPlace->scaleVirtualToReal[0] + padding;
            padRight = waypointOffscreenPadRight->current.value * scrPlace->scaleVirtualToReal[0] + padding;
            padTop = waypointOffscreenPadTop->current.value * scrPlace->scaleVirtualToReal[1] + padding;
            padBottom = waypointOffscreenPadBottom->current.value * scrPlace->scaleVirtualToReal[1] + padding;
            tweak2dY = waypointTweakY->current.value * scrPlace->scaleVirtualToReal[1];
            if (elem->targetEntNum == 1023)
            {
                v9 = elem->y;
                z = elem->z;
                worldPos[0] = elem->x;
                worldPos[1] = v9;
                worldPos[2] = z;
            LABEL_26:
                WorldPosToScreenPos(localClientNum, worldPos, screenPos);
                screenPos[1] = screenPos[1] + tweak2dY;
                didClamp = ClampScreenPosToEdges(
                    localClientNum,
                    screenPos,
                    padLeft,
                    padRight,
                    padTop,
                    padBottom,
                    clampedDir,
                    &clampedDist);
                if (didClamp && clampedDist > 0.0f)
                {
                    colorArrow[0] = color[0];
                    colorArrow[1] = color[1];
                    colorArrow[2] = color[2];
                    colorArrow[3] = color[3];
                    if (distanceThresholdAlpha > (double)clampedDist)
                        colorArrow[3] = clampedDist / distanceThresholdAlpha * colorArrow[3];
                    shrinkDist = waypointOffscreenScaleLength->current.value * scaleVirtualToRealAvg;
                    if (shrinkDist <= (double)clampedDist)
                    {
                        scale = waypointOffscreenScaleSmallest->current.value;
                    }
                    else
                    {
                        scale = clampedDist / shrinkDist;
                        scale = scale * waypointOffscreenScaleSmallest->current.value + (1.0 - scale) * 1.0;
                    }
                    iconWidth = iconWidth * scale;
                    iconHeight = iconHeight * scale;
                    v8 = -clampedDir[1];
                    v7 = atan2(clampedDir[0], v8);
                    angle = v7 * 180.0f / 3.141592741012573f;
                    screenPosArrow[0] = pointerDistance * clampedDir[0] + screenPos[0];
                    screenPosArrow[1] = pointerDistance * clampedDir[1] + screenPos[1];
                    y = screenPosArrow[1] - pointerHeight * 0.5f;
                    x = screenPosArrow[0] - pointerWidth * 0.5f;
                    CL_DrawStretchPicPhysicalRotateXY(
                        x,
                        y,
                        pointerWidth,
                        pointerHeight,
                        0.0f,
                        0.0f,
                        1.0f,
                        1.0f,
                        angle,
                        colorArrow,
                        cgMedia.offscreenObjectivePointer);
                }
                scale = GetScaleForDistance(localClientNum, worldPos);
                iconWidth = iconWidth * scale;
                iconHeight = iconHeight * scale;
                v4 = screenPos[1] - iconHeight * 0.5f;
                v3 = screenPos[0] - iconWidth * 0.5f;
                CL_DrawStretchPicPhysical(v3, v4, iconWidth, iconHeight, 0.0f, 0.0f, 1.0f, 1.0f, color, material);
                return;
            }
            screenPosArrow[2] = 1.4046605e-38f;
            if (elem->targetEntNum != CG_GetLocalClientGlobals(localClientNum)->predictedPlayerEntity.nextState.number)
            {
                cent = CG_GetEntity(localClientNum, elem->targetEntNum);
                if (!cent->nextValid)
                {
                    Com_PrintWarning(
                        1,
                        "DrawOffscreenViewableWaypoint(): targetEnt %i not in snapshot, may not be a network-broadcasting entity.",
                        elem->targetEntNum);
                    return;
                }
                worldPos[0] = cent->pose.origin[0];
                worldPos[1] = cent->pose.origin[1];
                worldPos[2] = cent->pose.origin[2];
                if (cent->nextState.eType == 1)
                {
                    if ((cent->currentState.eFlags & 8) != 0)
                    {
                        worldPos[2] = worldPos[2] + waypointPlayerOffsetProne->current.value;
                    }
                    else
                    {
                        if ((cent->currentState.eFlags & 4) != 0)
                            v2 = worldPos[2] + waypointPlayerOffsetCrouch->current.value;
                        else
                            v2 = worldPos[2] + waypointPlayerOffsetStand->current.value;
                        worldPos[2] = v2;
                    }
                }
                goto LABEL_26;
            }
        }
    }
}

char __cdecl WorldPosToScreenPos(int32_t localClientNum, const float *worldPos, float *outScreenPos)
{
    float v4; // [esp+0h] [ebp-64h]
    float v5; // [esp+4h] [ebp-60h]
    float v6; // [esp+8h] [ebp-5Ch]
    float v7; // [esp+Ch] [ebp-58h]
    float v8; // [esp+10h] [ebp-54h]
    float v9; // [esp+14h] [ebp-50h]
    float v10; // [esp+18h] [ebp-4Ch]
    float v11; // [esp+24h] [ebp-40h]
    ScreenPlacement *scrPlace; // [esp+40h] [ebp-24h]
    float projections[3]; // [esp+48h] [ebp-1Ch] BYREF
    const refdef_s *refdef; // [esp+54h] [ebp-10h]
    float x; // [esp+58h] [ebp-Ch]
    float y; // [esp+5Ch] [ebp-8h]
    float w; // [esp+60h] [ebp-4h]
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    refdef = &cgameGlob->refdef;
    scrPlace = &scrPlaceView[localClientNum];
    CG_GetViewAxisProjections(&cgameGlob->refdef, worldPos, projections);
    w = projections[0];
    if (projections[0] >= 0.0f)
    {
        x = projections[1] / refdef->tanHalfFovX;
        *outScreenPos = scrPlace->realViewportSize[0] * 0.5f * (1.0f - x / w);
        y = projections[2] / refdef->tanHalfFovY;
        outScreenPos[1] = scrPlace->realViewportSize[1] * 0.5f * (1.0f - y / w);
        return 1;
    }
    else
    {
        *outScreenPos = -projections[1];
        outScreenPos[1] = -projections[2];
        v9 = fabs(*outScreenPos);
        if (v9 >= EQUAL_EPSILON || (v8 = fabs(outScreenPos[1]), v8 >= EQUAL_EPSILON))
        {
            v7 = fabs(*outScreenPos);
            if (v7 < EQUAL_EPSILON)
                *outScreenPos = 0.001f;
            v6 = fabs(outScreenPos[1]);
            if (v6 < EQUAL_EPSILON)
                outScreenPos[1] = 0.001f;
            while (1)
            {
                v5 = fabs(*outScreenPos);
                if (scrPlace->realViewportSize[0] <= (double)v5)
                    break;
                v11 = scrPlace->realViewportSize[0];
                *outScreenPos = v11 * *outScreenPos;
                outScreenPos[1] = v11 * outScreenPos[1];
            }
            while (1)
            {
                v4 = fabs(outScreenPos[1]);
                if (scrPlace->realViewportSize[1] <= (double)v4)
                    break;
                v10 = scrPlace->realViewportSize[1];
                *outScreenPos = v10 * *outScreenPos;
                outScreenPos[1] = v10 * outScreenPos[1];
            }
        }
        else
        {
            outScreenPos[1] = scrPlace->realViewportSize[1] + scrPlace->realViewportSize[1];
        }
        return 0;
    }
}

bool __cdecl ClampScreenPosToEdges(
    int32_t localClientNum,
    float *point,
    float padLeft,
    float padRight,
    float padTop,
    float padBottom,
    float *resultNormal,
    float *resultDist)
{
    double v8; // st7
    double v9; // st7
    bool v11; // [esp+28h] [ebp-8Ch]
    bool v12; // [esp+2Ch] [ebp-88h]
    float v13; // [esp+30h] [ebp-84h]
    float v14; // [esp+34h] [ebp-80h]
    float x; // [esp+38h] [ebp-7Ch]
    float y; // [esp+3Ch] [ebp-78h]
    float v17; // [esp+58h] [ebp-5Ch]
    float v18; // [esp+5Ch] [ebp-58h]
    float v19; // [esp+60h] [ebp-54h]
    float v20; // [esp+64h] [ebp-50h]
    float dist; // [esp+68h] [ebp-4Ch]
    float dir[2]; // [esp+6Ch] [ebp-48h] BYREF
    bool top; // [esp+77h] [ebp-3Dh]
    float radius; // [esp+78h] [ebp-3Ch]
    bool left; // [esp+7Dh] [ebp-37h]
    bool horzQualify; // [esp+7Eh] [ebp-36h]
    bool vertQualify; // [esp+7Fh] [ebp-35h]
    float focus[2]; // [esp+80h] [ebp-34h] BYREF
    float borderTop; // [esp+88h] [ebp-2Ch]
    float borderBottom; // [esp+8Ch] [ebp-28h]
    ScreenPlacement *scrPlace; // [esp+90h] [ebp-24h]
    bool clamped; // [esp+97h] [ebp-1Dh]
    const cg_s *cgameGlob; // [esp+98h] [ebp-1Ch]
    float halfWidth; // [esp+9Ch] [ebp-18h]
    float borderRight; // [esp+A0h] [ebp-14h]
    float pointOriginal[2]; // [esp+A4h] [ebp-10h] BYREF
    float halfHeight; // [esp+ACh] [ebp-8h]
    float borderLeft; // [esp+B0h] [ebp-4h]

    iassert(resultDist);

    scrPlace = &scrPlaceView[localClientNum];
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    halfWidth = scrPlace->realViewportSize[0] * 0.5f;
    halfHeight = scrPlace->realViewportSize[1] * 0.5f;
    pointOriginal[0] = *point;
    pointOriginal[1] = point[1];
    *point = *point - halfWidth;
    point[1] = point[1] - halfHeight;
    borderLeft = scrPlace->realViewableMin[0] + padLeft - halfWidth;
    borderRight = halfWidth - (scrPlace->realViewportSize[0] - scrPlace->realViewableMax[0] + padRight);
    borderTop = scrPlace->realViewableMin[1] + padTop - halfHeight;
    borderBottom = halfHeight - (scrPlace->realViewportSize[1] - scrPlace->realViewableMax[1] + padBottom);
    clamped = 0;
    if (borderLeft <= (float)*point)
    {
        if (borderRight < (float)*point)
        {
            v19 = borderRight / *point;
            *point = v19 * *point;
            point[1] = v19 * point[1];
            clamped = 1;
        }
    }
    else
    {
        v20 = borderLeft / *point;
        *point = v20 * *point;
        point[1] = v20 * point[1];
        clamped = 1;
    }
    if (borderTop <= (float)point[1])
    {
        if (borderBottom < (float)point[1])
        {
            v17 = borderBottom / point[1];
            *point = v17 * *point;
            point[1] = v17 * point[1];
            clamped = 1;
        }
    }
    else
    {
        v18 = borderTop / point[1];
        *point = v18 * *point;
        point[1] = v18 * point[1];
        clamped = 1;
    }
    if (waypointOffscreenRoundedCorners->current.enabled)
    {
        radius = (scrPlace->scaleVirtualToReal[0] + scrPlace->scaleVirtualToReal[1])
            * waypointOffscreenCornerRadius->current.value
            * 0.5f;
        left = *point < 0.0f;
        top = point[1] < 0.0f;
        if (left)
            v8 = borderLeft + radius;
        else
            v8 = borderRight - radius;
        focus[0] = v8;
        if (top)
            v9 = borderTop + radius;
        else
            v9 = borderBottom - radius;
        focus[1] = v9;
        if (waypointDebugDraw->current.enabled)
        {
            y = focus[1] + halfHeight - 1.0f;
            x = focus[0] + halfWidth - 1.0f;
            CL_DrawStretchPicPhysical(x, y, 3.0f, 3.0f, 0.0f, 0.0f, 1.0f, 1.0f, colorYellow, cgMedia.whiteMaterial);
            v14 = point[1] + halfHeight - 1.0f;
            v13 = *point + halfWidth - 1.0f;
            CL_DrawStretchPicPhysical(v13, v14, 3.0f, 3.0f, 0.0f, 0.0f, 1.0f, 1.0f, colorGreen, cgMedia.whiteMaterial);
        }
        dir[0] = *point - focus[0];
        dir[1] = point[1] - focus[1];
        Vec2Normalize(dir);
        v12 = left && dir[0] < 0.0f || !left && dir[0] > 0.0f;
        horzQualify = v12;
        v11 = top && dir[1] < 0.0f || !top && dir[1] > 0.0f;
        vertQualify = v11;
        if (horzQualify && vertQualify)
        {
            dist = Vec2Distance(focus, point);
            if (radius < (float)dist)
            {
                *point = radius * dir[0] + focus[0];
                point[1] = radius * dir[1] + focus[1];
                clamped = 1;
            }
        }
    }
    *point = *point + halfWidth;
    point[1] = point[1] + halfHeight;
    if (clamped)
    {
        *resultDist = Vec2Distance(point, pointOriginal);
        *resultNormal = pointOriginal[0] - *point;
        resultNormal[1] = pointOriginal[1] - point[1];
        Vec2Normalize(resultNormal);
    }
    return clamped;
}

float __cdecl GetScaleForDistance(int32_t localClientNum, const float *worldPos)
{
    float diff[4]; // [esp+8h] [ebp-18h] BYREF
    float range; // [esp+18h] [ebp-8h]
    float dist3D; // [esp+1Ch] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    Vec3Sub(worldPos, cgameGlob->refdef.vieworg, diff);
    dist3D = Vec3Length(diff);
    if (waypointDistScaleRangeMin->current.value >= dist3D)
        return 1.0f;
    if (waypointDistScaleRangeMax->current.value <= dist3D)
        return waypointDistScaleSmallest->current.value;
    range = waypointDistScaleRangeMax->current.value - waypointDistScaleRangeMin->current.value;
    if (range <= 0.0f)
        range = 1.0f;
    range = (dist3D - waypointDistScaleRangeMin->current.value) / range;
    return range * waypointDistScaleSmallest->current.value + (1.0f - range) * 1.0f;
}

int32_t __cdecl GetSortedHudElems(int32_t localClientNum, hudelem_s **elems)
{
    playerState_s *ps; // [esp+4h] [ebp-8h]
    int32_t elemCount; // [esp+8h] [ebp-4h] BYREF
    const cg_s *clientGlob;

    clientGlob = CG_GetLocalClientGlobals(localClientNum);
    if (!clientGlob->nextSnap)
        return 0;
    ps = &clientGlob->nextSnap->ps;
    elemCount = 0;
    CopyInUseHudElems(elems, &elemCount, ps->hud.current, 31);
    CopyInUseHudElems(elems, &elemCount, ps->hud.archival, 31);
    qsort(elems, elemCount, 4u, compare_hudelems);
    return elemCount;
}

void __cdecl CopyInUseHudElems(hudelem_s **elems, int32_t *elemCount, hudelem_s *elemSrcArray, int32_t elemSrcArrayCount)
{
    int32_t i; // [esp+0h] [ebp-4h]

    for (i = 0; i < elemSrcArrayCount && elemSrcArray[i].type; ++i)
        elems[(*elemCount)++] = &elemSrcArray[i];
}

void __cdecl CG_AddDrawSurfsFor3dHudElems(int32_t localClientNum)
{
    int32_t i; // [esp+0h] [ebp-104h]
    hudelem_s *elems[62]; // [esp+4h] [ebp-100h] BYREF
    int32_t elemCount; // [esp+100h] [ebp-4h]

    if (CG_ShouldDrawHud(localClientNum))
    {
        elemCount = GetSortedHudElems(localClientNum, elems);
        if ((unsigned int)elemCount >= 0x3E)
            MyAssertHandler(
                ".\\cgame\\cg_hudelem.cpp",
                1547,
                0,
                "elemCount doesn't index ARRAY_COUNT( elems )\n\t%i not in [0, %i)",
                elemCount,
                62);
        for (i = 0; i < elemCount; ++i)
        {
            if (elems[i]->type == HE_TYPE_WAYPOINT)
                AddDrawSurfForHudElemWaypoint(localClientNum, elems[i]);
        }
    }
}

void AddDrawSurfForHudElemWaypoint(int32_t localClientNum, const hudelem_s *elem)
{
    FxSprite sprite; // [esp-94h] [ebp-A0h] BYREF
    float z; // [esp-70h] [ebp-7Ch]
    float y; // [esp-6Ch] [ebp-78h]
    float x; // [esp-68h] [ebp-74h]
    float v6; // [esp-64h] [ebp-70h]
    int32_t v7; // [esp-60h] [ebp-6Ch]
    float v8; // [esp-5Ch] [ebp-68h]
    Material *v9; // [esp-58h] [ebp-64h]
    char v10[68]; // [esp-54h] [ebp-60h] BYREF
    hudelem_color_t v11; // [esp-10h] [ebp-1Ch] BYREF
    int32_t time; // [esp-Ch] [ebp-18h]
    void *v13; // [esp+0h] [ebp-Ch]
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    time = cgameGlob->time;
    BG_LerpHudColors(elem, cgameGlob->time, &v11);
    if (v11.a)
    {
        if (!CG_ServerMaterialName(localClientNum, elem->offscreenMaterialIdx, v10, 0x40u))
        {
            if (CG_ServerMaterialName(localClientNum, elem->materialIndex, v10, 0x40u))
            {
                v9 = Material_RegisterHandle(v10, 7);
                v8 = HudElemWaypointHeight(localClientNum, elem);
                if (v8 != 0.0)
                {
                    if (elem->value <= 0.0)
                    {
                        v7 = 0;
                        v6 = v8;
                    }
                    else
                    {
                        v7 = 3;
                        v6 = v8 * 0.00430000014603138f;
                    }
                    x = elem->x;
                    y = elem->y;
                    z = elem->z;
                    sprite.pos[0] = x;
                    sprite.pos[1] = y;
                    sprite.pos[2] = z;
                    *(hudelem_color_t *)sprite.rgbaColor = v11;
                    sprite.flags = v7;
                    sprite.radius = v6;
                    sprite.minScreenRadius = 0.0f;
                    sprite.material = v9;
                    FX_SpriteAdd(&sprite);
                }
            }
        }
    }
}

float __cdecl HudElemWaypointHeight(int32_t localClientNum, const hudelem_s *elem)
{
    float height; // [esp+20h] [ebp-10h]
    float lerp; // [esp+24h] [ebp-Ch]
    int32_t deltaTime; // [esp+28h] [ebp-8h]
    float fromHeight; // [esp+2Ch] [ebp-4h]

    height = (float)elem->height;
    if (elem->scaleTime <= 0)
        return height;
    deltaTime = CG_GetLocalClientGlobals(localClientNum)->time - elem->scaleStartTime;
    if (deltaTime >= elem->scaleTime)
        return height;
    fromHeight = (float)elem->fromHeight;
    if (deltaTime <= 0)
        return fromHeight;
    lerp = (double)deltaTime / (double)elem->scaleTime;

    if (lerp < 0.0f || lerp > 1.0f)
        MyAssertHandler(
            ".\\cgame\\cg_hudelem.cpp",
            991,
            1,
            "lerp not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
            lerp,
            0.0f,
            1.0f);
    return (height - fromHeight) * lerp + fromHeight;
}

