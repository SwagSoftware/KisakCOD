#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "cg_local_mp.h"
#include "cg_public_mp.h"
#include <qcommon/mem_track.h>
#include <client/client.h>
#include <xanim/xmodel.h>

#include <EffectsCore/fx_system.h>

#include <game_mp/g_main_mp.h>

#include <script/scr_const.h>
#include <script/scr_vm.h>

#include <stringed/stringed_hooks.h>
#include <universal/profile.h>

extern const dvar_t *ui_showEndOfGame;

struct CenterPrint // sizeof=0x408
{                                       // ...
    int32_t time;                           // ...
    char text[1024];
    int32_t priority;
};

menuDef_t * menuScoreboard;
CenterPrint s_centerPrint[1];
OverheadFade overheadFade[64];

void __cdecl TRACK_cg_draw()
{
    track_static_alloc_internal(s_centerPrint, 1032, "s_centerPrint", 9);
    track_static_alloc_internal(overheadFade, 768, "overheadFade", 10);
}

void __cdecl CG_PriorityCenterPrint(int32_t localClientNum, const char* str, int32_t priority)
{
    CenterPrint* centerPrint; // [esp+4h] [ebp-10Ch]
    char hudElemString[260]; // [esp+8h] [ebp-108h] BYREF

    const cg_s *cgameGlob;

    centerPrint = &s_centerPrint[localClientNum];
    if (!centerPrint->time || priority >= centerPrint->priority)
    {
        CG_TranslateHudElemMessage(localClientNum, str, "Center Print", hudElemString);
        I_strncpyz(centerPrint->text, hudElemString, 256);
        centerPrint->priority = priority;

        cgameGlob = CG_GetLocalClientGlobals(0);

        centerPrint->time = cgameGlob->time;
    }
}

void __cdecl CG_ClearCenterPrint(int32_t localClientNum)
{
    s_centerPrint[localClientNum].time = 0;
}
void __cdecl CG_DrawCenterString(
    int32_t localClientNum,
    const rectDef_s* rect,
    Font_s* font,
    float fontscale,
    float* color,
    int32_t textStyle)
{
    float v6; // [esp+24h] [ebp-20h]
    CenterPrint* centerPrint; // [esp+34h] [ebp-10h]
    float* fadeColor; // [esp+38h] [ebp-Ch]
    int32_t time; // [esp+3Ch] [ebp-8h]
    float x; // [esp+40h] [ebp-4h]

    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(0);

    time = cgameGlob->time;
    centerPrint = &s_centerPrint[localClientNum];
    if (centerPrint->time > cgameGlob->time)
        centerPrint->time = 0;
    if (centerPrint->time)
    {
        fadeColor = CG_FadeColor(time, centerPrint->time, (int)(cg_centertime->current.value * 1000.0), 100);
        if (fadeColor)
        {
            Vec4Mul(color, fadeColor, color);
            x = rect->x - (double)(int)(UI_TextWidth(centerPrint->text, 0, font, fontscale) * 0.5f);
            UI_DrawText(
                &scrPlaceView[localClientNum],
                centerPrint->text,
                0x7FFFFFFF,
                font,
                x,
                rect->y,
                rect->horzAlign,
                rect->vertAlign,
                fontscale,
                color,
                textStyle);
        }
        else
        {
            centerPrint->time = 0;
            centerPrint->priority = 0;
        }
    }
}

void __cdecl CG_ClearOverheadFade()
{
    memset((uint8_t *)overheadFade, 0, sizeof(overheadFade));
}

void __cdecl CG_Draw2D(int32_t localClientNum)
{
    bool drawHud; // [esp+37h] [ebp-Dh]
    int32_t isScoreboardVisible; // [esp+38h] [ebp-Ch]
    int32_t chatOverScoreboard; // [esp+3Ch] [ebp-8h]
    playerState_s *ps; // [esp+40h] [ebp-4h]

    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(0);

    if (cgameGlob->cubemapShot == CUBEMAPSHOT_NONE && cg_draw2D->current.enabled)
    {
        if (debugOverlay->current.integer == 1)
        {
            DrawViewmodelInfo(localClientNum);
        }
        else if (net_showprofile->current.integer)
        {
            CG_DrawSnapshotAnalysis(localClientNum);
            CG_DrawPingAnalysis(localClientNum);
            CG_DrawSnapshotEntityAnalysis(localClientNum);
        }
        else
        {
            drawHud = CG_ShouldDrawHud(localClientNum);
            ps = &cgameGlob->nextSnap->ps;
            if (cgameGlob->nextSnap->ps.pm_type == PM_INTERMISSION)
            {
                DrawIntermission(localClientNum);
                CG_DrawSay(localClientNum);
            }
            else
            {
                Dvar_SetBool(ui_showEndOfGame, 0);
                CG_CompassUpdateActors(localClientNum);
                chatOverScoreboard = CG_IsScoreboardDisplayed(localClientNum);
                if (ps->pm_type == PM_SPECTATOR)
                {
                    if (drawHud)
                    {
                        CG_ScanForCrosshairEntity(localClientNum);
                        CG_UpdatePlayerNames(localClientNum);
                        if (!chatOverScoreboard)
                            CG_DrawChatMessages(localClientNum);
                        CG_Draw2dHudElems(localClientNum, 0);
                        Menu_PaintAll(&cgDC[localClientNum]);
                        CG_Draw2dHudElems(localClientNum, 1);
                    }
                }
                else
                {
                    CG_DrawNightVisionOverlay(localClientNum);
                    CG_ScanForCrosshairEntity(localClientNum);
                    if (ps->pm_type < PM_DEAD)
                        CG_DrawCrosshair(localClientNum);
                    if (drawHud)
                    {
                        if (ps->pm_type < PM_DEAD)
                            CG_UpdatePlayerNames(localClientNum);
                        if (!chatOverScoreboard)
                            CG_DrawChatMessages(localClientNum);
                    }
                    CG_CheckTimedMenus(localClientNum);
                    if (drawHud)
                    {
                        CG_Draw2dHudElems(localClientNum, 0);
                        Menu_PaintAll(&cgDC[localClientNum]);
                    }
                    CG_Draw2dHudElems(localClientNum, 1);
                }
                isScoreboardVisible = CG_DrawScoreboard(localClientNum);
                if (drawHud)
                {
                    if (chatOverScoreboard)
                        CG_DrawChatMessages(localClientNum);
                    CG_DrawFlashDamage(cgameGlob);
                    CG_DrawDamageDirectionIndicators(localClientNum);
                    CG_DrawGrenadeIndicators(localClientNum);
                    if (!isScoreboardVisible && cg_drawSpectatorMessages->current.enabled)
                    {
                        CG_DrawSpectatorMessage(localClientNum);
                        CG_DrawFollow(localClientNum);
                    }
                    CG_DrawVote(localClientNum);
                    CG_DrawLagometer(localClientNum);
                    CG_DrawSnapshotAnalysis(localClientNum);
                    CG_DrawPingAnalysis(localClientNum);
                    CG_DrawSnapshotEntityAnalysis(localClientNum);
                    {
                        PROF_SCOPED("DebugOverlays");
                        CG_DrawDebugOverlays(localClientNum);
                    }
                    if (!isScoreboardVisible)
                    {
                        CG_DrawMiniConsole(localClientNum);
                        CG_DrawErrorMessages(localClientNum);
                    }
                    CG_DrawSay(localClientNum);
                }
            }
        }
    }
}

void __cdecl CG_DrawChatMessages(int32_t localClientNum)
{
    int32_t v1; // edx
    const dvar_s *v2; // [esp+30h] [ebp-58h]
    Font_s *font; // [esp+34h] [ebp-54h]
    ScreenPlacement *scrPlace; // [esp+38h] [ebp-50h]
    int32_t cgameTimeNow; // [esp+40h] [ebp-48h]
    float fontWidth; // [esp+44h] [ebp-44h]
    float alphapercent; // [esp+48h] [ebp-40h]
    float alphapercenta; // [esp+48h] [ebp-40h]
    char *msg; // [esp+4Ch] [ebp-3Ch]
    int32_t hudChatY; // [esp+50h] [ebp-38h]
    int32_t hudChatX; // [esp+58h] [ebp-30h]
    int32_t i; // [esp+5Ch] [ebp-2Ch]
    float fontHeight; // [esp+60h] [ebp-28h]
    float x; // [esp+64h] [ebp-24h]
    float y; // [esp+68h] [ebp-20h]
    float ya; // [esp+68h] [ebp-20h]
    float color[4]; // [esp+6Ch] [ebp-1Ch] BYREF
    float fontScale; // [esp+7Ch] [ebp-Ch]
    int32_t chatHeight; // [esp+80h] [ebp-8h]
    float w; // [esp+84h] [ebp-4h]
    cgs_t *cgamestaticGlob;
    cg_s *cgameGlob;

    PROF_SCOPED("CG_DrawChatMessages");

    iassert(cg_hudChatPosition);

    chatHeight = cg_chatHeight->current.integer;
    if (chatHeight)
    {
        if (CG_IsScoreboardDisplayed(localClientNum))
            v2 = cg_hudChatIntermissionPosition;
        else
            v2 = cg_hudChatPosition;
        hudChatX = (int)v2->current.value;
        hudChatY = (int)v2->current.vector[1];
        cgamestaticGlob = CG_GetLocalClientStaticGlobals(localClientNum);
        fontHeight = cgamestaticGlob->viewHeight <= 768 ? 16.0 : 10.0;
        fontWidth = cgamestaticGlob->viewWidth <= 768 ? 12.0 : 8.0;
        fontScale = fontHeight / 48.0;
        if (cgamestaticGlob->teamLastChatPos != cgamestaticGlob->teamChatPos)
        {
            cgameGlob = CG_GetLocalClientGlobals(localClientNum);
            cgameTimeNow = cgameGlob->time;
            if (cgameGlob->time - cgamestaticGlob->teamChatMsgTimes[cgamestaticGlob->teamLastChatPos % chatHeight] > cg_chatTime->current.integer)
                ++cgamestaticGlob->teamLastChatPos;
            scrPlace = &scrPlaceView[localClientNum];
            font = UI_GetFontHandle(scrPlace, 0, fontScale);
            for (i = cgamestaticGlob->teamChatPos - 1; ; --i)
            {
                if (i < cgamestaticGlob->teamLastChatPos)
                    return;
                alphapercent = (double)cg_chatTime->current.integer
                    - (double)(cgameTimeNow - cgamestaticGlob->teamChatMsgTimes[i % chatHeight]);
                if (alphapercent <= 200.0)
                {
                    alphapercenta = alphapercent / 200.0;
                    if (alphapercenta <= 0.0)
                        continue;
                }
                else
                {
                    alphapercenta = 1.0;
                }
                v1 = i % chatHeight;
                msg = cgamestaticGlob->teamChatMsgs[v1];
                if ((cgs_t *)((char *)cgsArray + v1 * 160) == (cgs_t *)-3728 // KISAKTODO: something to do with cgs->teamChatMsgs
                    || *msg != '^'
                    || !cgamestaticGlob->teamChatMsgs[v1][1]
                    || cgamestaticGlob->teamChatMsgs[v1][1] == '^'
                    || cgamestaticGlob->teamChatMsgs[v1][1] < '0'
                    || cgamestaticGlob->teamChatMsgs[v1][1] > '9')
                {
                    color[0] = 1.0;
                    color[1] = 1.0;
                    color[2] = 1.0;
                }
                else
                {
                    CL_LookupColor(localClientNum, cgamestaticGlob->teamChatMsgs[v1][1], color);
                }
                Vec3Scale(color, 0.25, color);
                color[3] = alphapercenta * 0.6000000238418579;
                y = (double)hudChatY - (double)(cgamestaticGlob->teamChatPos - i) * fontHeight;
                w = (double)UI_TextWidth(msg, 0, font, fontScale) + fontWidth * 3.0;
                UI_DrawHandlePic(scrPlace, 0.0, y, w, fontHeight, 1, 1, color, cgMedia.teamStatusBar);
                color[3] = alphapercenta;
                x = (float)hudChatX;
                ya = (double)hudChatY - (double)(cgamestaticGlob->teamChatPos - i) * fontHeight + fontHeight - 1.0;
                color[0] = 1.0;
                color[1] = 1.0;
                color[2] = 1.0;
                UI_DrawText(scrPlace, msg, 0x7FFFFFFF, font, x, ya, 1, 1, fontScale, color, 3);
            }
        }
    }
}

void __cdecl CG_ScanForCrosshairEntity(int32_t localClientNum)
{
    centity_s *Entity; // eax
    float v2; // [esp+Ch] [ebp-84h]
    float diff[3]; // [esp+18h] [ebp-78h] BYREF
    float fCheckDist; // [esp+24h] [ebp-6Ch]
    int32_t fadeOutTime; // [esp+28h] [ebp-68h]
    team_t team; // [esp+2Ch] [ebp-64h]
    float contactEnd[3]; // [esp+30h] [ebp-60h] BYREF
    cg_s *cgameGlob; // [esp+3Ch] [ebp-54h]
    float start[3]; // [esp+40h] [ebp-50h] BYREF
    float end[3]; // [esp+4Ch] [ebp-44h] BYREF
    float vis; // [esp+58h] [ebp-38h]
    trace_t trace; // [esp+5Ch] [ebp-34h] BYREF
    WeaponDef *weapDef; // [esp+88h] [ebp-8h]
    uint16_t hitEntId; // [esp+8Ch] [ebp-4h]

    PROF_SCOPED("CG_ScanForCrosshairEntity");

    fCheckDist = 8192.0f;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    iassert(cgameGlob->nextSnap);
    cgameGlob->predictedPlayerState.weapFlags &= 0xFFFFFFE7;

    if (!CG_Flashbanged(localClientNum))
    {
        start[0] = cgameGlob->refdef.vieworg[0];
        start[1] = cgameGlob->refdef.vieworg[1];
        start[2] = cgameGlob->refdef.vieworg[2];
        Vec3Mad(start, fCheckDist, cgameGlob->refdef.viewaxis[0], end);
        CG_TraceCapsule(
            &trace,
            start,
            (float *)vec3_origin,
            (float *)vec3_origin,
            end,
            cgameGlob->nextSnap->ps.clientNum,
            0x2803001);
        hitEntId = Trace_GetEntityHitId(&trace);
        if (hitEntId < 0x40u)
        {
            bcassert(cgameGlob->nextSnap->ps.clientNum, MAX_CLIENTS);
            fadeOutTime = cg_friendlyNameFadeOut->current.integer;
            team = cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team;
            if (team != TEAM_SPECTATOR)
            {
                bcassert(cgameGlob->crosshairClientNum, MAX_CLIENTS);
                if (cgameGlob->bgs.clientinfo[cgameGlob->crosshairClientNum].team == team && team)
                {
                    cgameGlob->predictedPlayerState.weapFlags |= 8u;
                }
                else
                {
                    Entity = CG_GetEntity(localClientNum, hitEntId);
                    Vec3Sub(Entity->pose.origin, start, diff);
                    weapDef = BG_GetWeaponDef(cgameGlob->nextSnap->ps.weapon);
                    v2 = weapDef->enemyCrosshairRange * weapDef->enemyCrosshairRange;
                    if (v2 < Vec3LengthSq(diff))
                        return;
                    cgameGlob->predictedPlayerState.weapFlags |= 0x10u;
                    fadeOutTime = cg_enemyNameFadeOut->current.integer;
                }
            }
            Vec3Lerp(start, end, trace.fraction, contactEnd);
            vis = FX_GetClientVisibility(localClientNum, start, contactEnd);
            if (vis >= 0.2000000029802322)
            {
                if (cgameGlob->crosshairClientNum != hitEntId
                    || cgameGlob->time - cgameGlob->crosshairClientLastTime > fadeOutTime)
                {
                    cgameGlob->crosshairClientNum = hitEntId;
                    cgameGlob->crosshairClientStartTime = cgameGlob->time;
                }
                cgameGlob->crosshairClientLastTime = cgameGlob->time;
            }
        }
    }
}

void __cdecl CG_CheckTimedMenus(int32_t localClientNum)
{
    cg_s *cgameGlob;

    PROF_SCOPED("CG_CheckTimedMenus");

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cgameGlob->voiceTime && cgameGlob->time - cgameGlob->voiceTime > 2500)
    {
        Menus_CloseByName(&cgDC[localClientNum], "voiceMenu");
        cgameGlob->voiceTime = 0;
    }
    CG_CheckForPlayerInput(localClientNum);
    CG_CheckHudHealthDisplay(localClientNum);
    CG_CheckHudAmmoDisplay(localClientNum);
    CG_CheckHudCompassDisplay(localClientNum);
    CG_CheckHudStanceDisplay(localClientNum);
    CG_CheckHudOffHandDisplay(localClientNum);
    CG_CheckHudObjectiveDisplay(localClientNum);
    CG_CheckHudSprintDisplay(localClientNum);
}

void __cdecl CG_CheckForPlayerInput(int32_t localClientNum)
{
    usercmd_s v1; // [esp-40h] [ebp-9Ch] BYREF
    usercmd_s v2; // [esp-20h] [ebp-7Ch] BYREF
    usercmd_s oldCmd; // [esp+8h] [ebp-54h] BYREF
    int32_t oldCmdIndex; // [esp+28h] [ebp-34h]
    usercmd_s newCmd; // [esp+2Ch] [ebp-30h] BYREF
    int32_t newInput; // [esp+50h] [ebp-Ch]
    int32_t changedButtons; // [esp+54h] [ebp-8h]
    int32_t newCmdIndex; // [esp+58h] [ebp-4h]

    newCmdIndex = CL_GetCurrentCmdNumber(localClientNum);
    if (newCmdIndex > 1)
    {
        oldCmdIndex = newCmdIndex - 1;
        CL_GetUserCmd(localClientNum, newCmdIndex - 1, &oldCmd);
        CL_GetUserCmd(localClientNum, newCmdIndex, &newCmd);
        changedButtons = newCmd.buttons ^ oldCmd.buttons;
        memcpy(&v2, &newCmd, sizeof(v2));
        memcpy(&v1, &oldCmd, sizeof(v1));
        newInput = CG_CheckPlayerMovement(v1, v2);
        if (CG_CheckPlayerWeaponUsage(localClientNum, newCmd.buttons))
            newInput = 1;
        if (CG_CheckPlayerOffHandUsage(localClientNum, newCmd.buttons))
            newInput = 1;
        if (CG_CheckPlayerStanceChange(localClientNum, newCmd.buttons, changedButtons))
            newInput = 1;
        if (!newInput)
            newInput = CG_CheckPlayerMiscInput(changedButtons) != 0;
        if (newInput)
            CG_MenuShowNotify(localClientNum, 2);
    }
}

bool __cdecl CG_CheckPlayerMovement(usercmd_s oldCmd, usercmd_s newCmd)
{
    if (memcmp(oldCmd.angles, newCmd.angles, 0xCu))
        return 1;
    return newCmd.forwardmove || newCmd.rightmove;
}

int32_t __cdecl CG_CheckPlayerStanceChange(int32_t localClientNum, __int16 newButtons, __int16 changedButtons)
{
    if ((changedButtons & 0x1300) != 0)
    {
        CG_MenuShowNotify(localClientNum, 3);
        return 1;
    }
    else
    {
        if ((newButtons & 0x1300) != 0)
            CG_MenuShowNotify(localClientNum, 3);
        return 0;
    }
}

int32_t __cdecl CG_CheckPlayerWeaponUsage(int32_t localClientNum, char buttons)
{
    if (!CG_CheckPlayerFireNonTurret(localClientNum, buttons) && !CG_CheckPlayerTryReload(localClientNum, buttons))
        return 0;
    CG_MenuShowNotify(localClientNum, 1);
    return 1;
}

bool __cdecl CG_CheckPlayerTryReload(int32_t localClientNum, char buttons)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if ((buttons & 0x30) == 0)
        return 0;

    return (cgameGlob->predictedPlayerState.pm_flags & 4) == 0 && (cgameGlob->predictedPlayerState.eFlags & 0x300) == 0;
}

bool __cdecl CG_CheckPlayerFireNonTurret(int32_t localClientNum, char buttons)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if ((buttons & 1) == 0)
        return 0;

    return (cgameGlob->predictedPlayerState.eFlags & 0x300) == 0;
}

int32_t __cdecl CG_CheckPlayerOffHandUsage(int32_t localClientNum, __int16 buttons)
{
    if ((buttons & 0xC000) == 0)
        return 0;
    CG_MenuShowNotify(localClientNum, 4);
    return 1;
}

uint32_t __cdecl CG_CheckPlayerMiscInput(int32_t buttons)
{
    return buttons & 0xFFFFECFF;
}

void __cdecl CG_CheckHudHealthDisplay(int32_t localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (hud_health_startpulse_injured->current.value <= CG_CalcPlayerHealth(&cgameGlob->nextSnap->ps))
    {
        if (hud_fade_healthbar->current.value != 0.0
            && cgameGlob->healthFadeTime
            && hud_fade_healthbar->current.value * 1000.0 < (double)(cgameGlob->time - cgameGlob->healthFadeTime))
        {
            if (CL_GetLocalClientActiveCount() == 1)
                Menus_HideByName(&cgDC[localClientNum], "Health");
            else
                Menus_HideByName(&cgDC[localClientNum], "Health_mp");
            cgameGlob->healthFadeTime = 0;
        }
    }
    else
    {
        CG_MenuShowNotify(localClientNum, 0);
    }
}

void __cdecl CG_CheckHudAmmoDisplay(int32_t localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (CG_CheckPlayerForLowAmmo(cgameGlob) || CG_CheckPlayerForLowClip(cgameGlob))
        CG_MenuShowNotify(localClientNum, 1);
    if (hud_fade_ammodisplay->current.value != 0.0
        && cgameGlob->ammoFadeTime
        && hud_fade_ammodisplay->current.value * 1000.0 < (double)(cgameGlob->time - cgameGlob->ammoFadeTime))
    {
        if (CL_GetLocalClientActiveCount() == 1)
        {
            Menus_HideByName(&cgDC[localClientNum], "weaponinfo");
            Menus_HideByName(&cgDC[localClientNum], "weaponinfo_lowdef");
        }
        else
        {
            Menus_HideByName(&cgDC[localClientNum], "weaponinfo_mp");
        }
        cgameGlob->ammoFadeTime = 0;
    }
}

void __cdecl CG_CheckHudCompassDisplay(int32_t localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (hud_fade_compass->current.value != 0.0
        && cgameGlob->compassFadeTime
        && hud_fade_compass->current.value * 1000.0 < (double)(cgameGlob->time - cgameGlob->compassFadeTime))
    {
        if (CL_GetLocalClientActiveCount() == 1)
            Menus_HideByName(&cgDC[localClientNum], "Compass");
        else
            Menus_HideByName(&cgDC[localClientNum], "Compass_mp");
        cgameGlob->compassFadeTime = 0;
    }
}

void __cdecl CG_CheckHudStanceDisplay(int32_t localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if ((cgameGlob->nextSnap->ps.eFlags & 8) != 0 && (cgameGlob->nextSnap->ps.eFlags & 0x100) != 0
        || (cgameGlob->nextSnap->ps.eFlags & 4) != 0 && (cgameGlob->nextSnap->ps.eFlags & 0x200) != 0)
    {
        CG_MenuShowNotify(localClientNum, 3);
    }
    if (hud_fade_stance->current.value != 0.0
        && cgameGlob->stanceFadeTime
        && hud_fade_stance->current.value * 1000.0 < (double)(cgameGlob->time - cgameGlob->stanceFadeTime))
    {
        if (CL_GetLocalClientActiveCount() == 1)
            Menus_HideByName(&cgDC[localClientNum], "stance");
        else
            Menus_HideByName(&cgDC[localClientNum], "stance_mp");
        cgameGlob->stanceFadeTime = 0;
    }
}

void __cdecl CG_CheckHudSprintDisplay(int32_t localClientNum)
{
    int32_t maxSprintTime; // [esp+Ch] [ebp-Ch]
    playerState_s* ps; // [esp+10h] [ebp-8h]

    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    ps = &cgameGlob->nextSnap->ps;
    if (cgameGlob->nextSnap->ps.pm_type != PM_DEAD)
    {
        maxSprintTime = BG_GetMaxSprintTime(ps);
        if (PM_GetSprintLeft(ps, cgameGlob->time) < maxSprintTime)
            CG_MenuShowNotify(localClientNum, 6);
    }
    if (ps->pm_type != PM_DEAD
        && cgameGlob->predictedPlayerState.sprintState.lastSprintStart > cgameGlob->predictedPlayerState.sprintState.lastSprintEnd)
    {
        CG_MenuShowNotify(localClientNum, 6);
    }
    if (hud_fade_sprint->current.value != 0.0
        && cgameGlob->sprintFadeTime
        && hud_fade_stance->current.value * 1000.0 < (double)(cgameGlob->time - cgameGlob->sprintFadeTime))
    {
        if (CL_GetLocalClientActiveCount() == 1)
            Menus_HideByName(&cgDC[localClientNum], "sprintMeter");
        else
            Menus_HideByName(&cgDC[localClientNum], "sprintMeter_mp");
        cgameGlob->sprintFadeTime = 0;
    }
}

void __cdecl CG_CheckHudOffHandDisplay(int32_t localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (hud_fade_offhand->current.value != 0.0
        && cgameGlob->offhandFadeTime
        && hud_fade_offhand->current.value * 1000.0 < (double)(cgameGlob->time - cgameGlob->offhandFadeTime))
    {
        if (CL_GetLocalClientActiveCount() == 1)
            Menus_HideByName(&cgDC[localClientNum], "offhandinfo");
        else
            Menus_HideByName(&cgDC[localClientNum], "offhandinfo_mp");
        cgameGlob->offhandFadeTime = 0;
    }
}

void __cdecl CG_CheckHudObjectiveDisplay(int32_t localClientNum)
{
    if (CG_IsScoreboardDisplayed(localClientNum))
    {
        cg_s *cgameGlob;

        cgameGlob = CG_GetLocalClientGlobals(localClientNum);

        if (cgameGlob->time - cgameGlob->scoreFadeTime > 100)
            Menus_HideByName(&cgDC[localClientNum], "objectiveinfo");
    }
}

void __cdecl CG_DrawMiniConsole(int32_t localClientNum)
{
    if (cg_minicon->current.enabled)
        Con_DrawMiniConsole(localClientNum, 2, 4, 1.0);
}

void __cdecl CG_DrawErrorMessages(int32_t localClientNum)
{
    Con_DrawErrors(localClientNum, 2, 300, 1.0);
}

void __cdecl CG_DrawSay(int32_t localClientNum)
{
    if (!cg_hudSayPosition)
        MyAssertHandler(".\\cgame_mp\\cg_draw_mp.cpp", 1092, 0, "%s", "cg_hudSayPosition");
    Con_DrawSay(localClientNum, (int)cg_hudSayPosition->current.value, (int)cg_hudSayPosition->current.vector[1] + 24);
}

void __cdecl CG_DrawVote(int32_t localClientNum)
{
    char *v1; // eax
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    int32_t v5; // [esp+Ch] [ebp-344h]
    int32_t voteYes; // [esp+10h] [ebp-340h]
    char *v7; // [esp+10h] [ebp-340h]
    int32_t v8; // [esp+14h] [ebp-33Ch]
    char *v9; // [esp+14h] [ebp-33Ch]
    const char *scale; // [esp+18h] [ebp-338h]
    int32_t scalea; // [esp+18h] [ebp-338h]
    int32_t scaleb; // [esp+18h] [ebp-338h]
    Font_s *font; // [esp+1Ch] [ebp-334h]
    char szVoteYes[256]; // [esp+20h] [ebp-330h] BYREF
    const ScreenPlacement *scrPlace; // [esp+120h] [ebp-230h]
    const cg_s *cgameGlob; // [esp+124h] [ebp-22Ch]
    int32_t iNumKeys; // [esp+128h] [ebp-228h]
    const cgs_t *cgs; // [esp+12Ch] [ebp-224h]
    int32_t sec; // [esp+130h] [ebp-220h]
    const char *s; // [esp+134h] [ebp-21Ch]
    char szVoteNo[260]; // [esp+138h] [ebp-218h] BYREF
    float fontHeight; // [esp+23Ch] [ebp-114h]
    char binding[256]; // [esp+240h] [ebp-110h] BYREF
    float x; // [esp+344h] [ebp-Ch]
    float y; // [esp+348h] [ebp-8h]
    float fontScale; // [esp+34Ch] [ebp-4h]

    iassert(cg_hudVotePosition);

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);

    if (cgs->voteTime)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        scrPlace = &scrPlaceView[localClientNum];
        if (cgs->viewHeight <= 768)
            fontHeight = 16.0;
        else
            fontHeight = 10.0;
        fontScale = fontHeight / 48.0;
        font = UI_GetFontHandle(scrPlace, 0, fontScale);
        x = cg_hudVotePosition->current.value;
        y = cg_hudVotePosition->current.vector[1] + fontHeight;
        iNumKeys = UI_GetKeyBindingLocalizedString(localClientNum, "vote yes", binding);
        if (iNumKeys)
            I_strncpyz(szVoteYes, binding, 256);
        else
            I_strncpyz(szVoteYes, "vote yes", 256);
        iNumKeys = UI_GetKeyBindingLocalizedString(localClientNum, "vote no", binding);
        if (iNumKeys)
            I_strncpyz(szVoteNo, binding, 256);
        else
            I_strncpyz(szVoteNo, "vote no", 256);
        sec = (cgs->voteTime - cgameGlob->time) / 1000;
        if (sec < 0)
            sec = 0;
        scale = cgs->voteString;
        v8 = sec;
        if ((cgameGlob->nextSnap->ps.eFlags & 0x100000) != 0)
        {
            v1 = UI_SafeTranslateString("CGAME_VOTE");
            s = va("%s(%i):%s", v1, v8, scale);
            UI_DrawText(scrPlace, s, 0x7FFFFFFF, font, x, y, 1, 1, fontScale, colorYellow, 3);
            y = y + fontHeight;
            scalea = cgs->voteNo;
            v9 = UI_SafeTranslateString("CGAME_NO");
            voteYes = cgs->voteYes;
            v2 = UI_SafeTranslateString("CGAME_YES");
            s = va("%s:%i, %s:%i", v2, voteYes, v9, scalea);
        }
        else
        {
            v3 = UI_SafeTranslateString("CGAME_VOTE");
            s = va("%s(%i):%s", v3, v8, scale);
            UI_DrawText(scrPlace, s, 0x7FFFFFFF, font, x, y, 1, 1, fontScale, colorYellow, 3);
            y = y + fontHeight;
            scaleb = cgs->voteNo;
            v7 = UI_SafeTranslateString("CGAME_NO");
            v5 = cgs->voteYes;
            v4 = UI_SafeTranslateString("CGAME_YES");
            s = va("%s(%s):%i, %s(%s):%i", v4, szVoteYes, v5, v7, szVoteNo, scaleb);
        }
        UI_DrawText(scrPlace, s, 0x7FFFFFFF, font, x, y, 1, 1, fontScale, colorYellow, 3);
    }
}

void __cdecl DrawIntermission(int32_t localClientNum)
{
    if (UI_GetActiveMenu(localClientNum) == 9 && UI_GetTopActiveMenuName(localClientNum))
    {
        CG_DrawScoreboard(localClientNum);
        CG_DrawChatMessages(localClientNum);
    }
    else
    {
        cg_s *cgameGlob;

        cgameGlob = CG_GetLocalClientGlobals(localClientNum);

        if (ui_showEndOfGame->current.enabled)
        {
            if (UI_GetActiveMenu(localClientNum) != 11)
                UI_SetActiveMenu(localClientNum, (uiMenuCommand_t)11);
        }
        else
        {
            cgameGlob->showScores = 1;
            cgameGlob->scoreFadeTime = cgameGlob->time;
            if (UI_GetActiveMenu(localClientNum) != 10 && !ui_showEndOfGame->current.enabled)
                UI_SetActiveMenu(localClientNum, (uiMenuCommand_t)10);
            if (UI_GetActiveMenu(localClientNum) == 10)
                CG_DrawScoreboard(localClientNum);
        }
        CG_DrawChatMessages(localClientNum);
    }
}

const char *__cdecl CG_GetBoundSpectatorCommand(int32_t localClientNum, const char **choices, int32_t choiceCount)
{
    int32_t i; // [esp+0h] [ebp-10Ch]
    char binding[260]; // [esp+4h] [ebp-108h] BYREF

    for (i = 0; i < choiceCount; ++i)
    {
        iassert(choices[i]);
        if (UI_GetKeyBindingLocalizedString(localClientNum, choices[i], binding))
            return choices[i];
    }
    iassert(choiceCount > 0);

    return *choices;
}

void __cdecl CG_DrawSpectatorMessage(int32_t localClientNum)
{
    const char *BoundSpectatorCommand; // eax
    const char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    char *str; // [esp+20h] [ebp-180h]
    Font_s *font; // [esp+24h] [ebp-17Ch]
    float lineHeight; // [esp+2Ch] [ebp-174h]
    const char *followPrev[4]; // [esp+34h] [ebp-16Ch] BYREF
    const cg_s *cgameGlob; // [esp+44h] [ebp-15Ch]
    int32_t vertAlign; // [esp+48h] [ebp-158h]
    const char *followStop[2]; // [esp+4Ch] [ebp-154h] BYREF
    int32_t lineNum; // [esp+54h] [ebp-14Ch]
    const char *commands[5]; // [esp+58h] [ebp-148h]
    int32_t horzAlign; // [esp+6Ch] [ebp-134h]
    const char *messages[5]; // [esp+70h] [ebp-130h]
    int32_t i; // [esp+84h] [ebp-11Ch]
    char binding[260]; // [esp+88h] [ebp-118h] BYREF
    float x; // [esp+190h] [ebp-10h]
    float y; // [esp+194h] [ebp-Ch]
    float fontScale; // [esp+198h] [ebp-8h]
    const playerState_s *ps; // [esp+19Ch] [ebp-4h]

    followPrev[0] = "+toggleads_throw";
    followPrev[1] = "+speed_throw";
    followPrev[2] = "+speed";
    followPrev[3] = "toggleads";
    followStop[0] = "+melee";
    followStop[1] = "+melee_breath";
    if (cg_descriptiveText->current.enabled && !Key_IsCatcherActive(localClientNum, 16))
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        iassert(cgameGlob->nextSnap);
        ps = &cgameGlob->nextSnap->ps;
        if ((ps->otherFlags & 0x18) != 0)
        {
            fontScale = 0.20833333f;
            font = UI_GetFontHandle(&scrPlaceView[localClientNum], 0, 0.20833333f);
            lineNum = 0;
            lineHeight = (float)UI_TextHeight(font, 0.20833333f);
            x = 240.0f;
            y = 436.0f - (lineHeight + lineHeight);
            horzAlign = 0;
            vertAlign = 0;
            if ((ps->otherFlags & 8) != 0)
            {
                commands[lineNum] = "+attack";
                messages[lineNum++] = "PLATFORM_FOLLOWNEXTPLAYER";
                BoundSpectatorCommand = CG_GetBoundSpectatorCommand(localClientNum, followPrev, 4);
                commands[lineNum] = BoundSpectatorCommand;
                messages[lineNum++] = "PLATFORM_FOLLOWPREVIOUSPLAYER";
            }
            if ((ps->otherFlags & 0x10) != 0)
            {
                v2 = CG_GetBoundSpectatorCommand(localClientNum, followStop, 2);
                commands[lineNum] = v2;
                messages[lineNum++] = "PLATFORM_FOLLOWSTOP";
            }
            if (lineNum > 5)
                MyAssertHandler(".\\cgame_mp\\cg_draw_mp.cpp", 1383, 0, "%s", "lineNum <= MAX_SPECTATOR_MSG_LINES");
            for (i = 0; i < lineNum; ++i)
            {
                if (!UI_GetKeyBindingLocalizedString(localClientNum, commands[i], binding))
                {
                    v3 = UI_SafeTranslateString("KEY_UNBOUND");
                    I_strncpyz(binding, v3, 256);
                }
                v4 = UI_SafeTranslateString((char *)messages[i]);
                str = UI_ReplaceConversionString(v4, binding);
                UI_DrawText(
                    &scrPlaceView[localClientNum],
                    str,
                    0x7FFFFFFF,
                    font,
                    x,
                    y,
                    horzAlign,
                    vertAlign,
                    fontScale,
                    colorWhite,
                    3);
                y = y + lineHeight;
            }
        }
    }
}

int32_t __cdecl CG_DrawFollow(int32_t localClientNum)
{
    Font_s* font; // [esp+24h] [ebp-4Ch]
    ScreenPlacement* scrPlace; // [esp+28h] [ebp-48h]
    char* followingString; // [esp+30h] [ebp-40h]
    char clientName[40]; // [esp+34h] [ebp-3Ch] BYREF
    float scale; // [esp+60h] [ebp-10h]
    float x; // [esp+64h] [ebp-Ch]
    float y; // [esp+68h] [ebp-8h]
    const playerState_s* ps; // [esp+6Ch] [ebp-4h]

    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    ps = &cgameGlob->nextSnap->ps;
    if ((ps->otherFlags & 2) == 0)
        return 0;
    if (cgameGlob->inKillCam)
        return 0;
    if (ps->clientNum >= 0x40u)
        MyAssertHandler(
            ".\\cgame_mp\\cg_draw_mp.cpp",
            1419,
            0,
            "ps->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            ps->clientNum,
            64);
    if (!CL_GetClientName(localClientNum, ps->clientNum, clientName, 38))
        Com_sprintf(clientName, 0x26u, "?");
    followingString = SEH_LocalizeTextMessage("CGAME_FOLLOWING", "spectator follow string", LOCMSG_SAFE);
    scale = 0.33333334f;
    if (G_ExitAfterConnectPaths())
        scale = scale * 1.5;
    scrPlace = &scrPlaceView[localClientNum];
    font = UI_GetFontHandle(scrPlace, 6, 0.33333334f);
    x = (float)-UI_TextWidth(followingString, 0, font, scale) * 0.5f;
    y = 20.0f;
    UI_DrawText(scrPlace, followingString, 0x7FFFFFFF, font, x, 20.0f, 7, 1, scale, colorWhite, 3);
    x = (float)-UI_TextWidth(clientName, 0, font, scale) * 0.5f;
    y = 36.0f;
    UI_DrawText(scrPlace, clientName, 0x7FFFFFFF, font, x, 36.0f, 7, 1, scale, colorWhite, 3);
    return 1;
}

void __cdecl CG_UpdatePlayerNames(int32_t localClientNum)
{
    PROF_SCOPED("CG_UpdatePlayerNames");

    CG_DrawCrosshairNames(localClientNum);
    CG_DrawFriendlyNames(localClientNum);
}

void __cdecl CG_DrawFriendlyNames(int32_t localClientNum)
{
    bool v1; // [esp+4h] [ebp-2Ch]
    snapshot_s *v2; // [esp+8h] [ebp-28h]
    int32_t entityIndex; // [esp+Ch] [ebp-24h]
    team_t team; // [esp+10h] [ebp-20h]
    bool flashed; // [esp+17h] [ebp-19h]
    snapshot_s *nextSnap; // [esp+1Ch] [ebp-14h]
    centity_s *cent; // [esp+20h] [ebp-10h]
    float alpha; // [esp+2Ch] [ebp-4h]

    if (cg_drawFriendlyNames->current.enabled)
    {
        cg_s *cgameGlob;

        cgameGlob = CG_GetLocalClientGlobals(localClientNum);

        team = cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team;
        nextSnap = cgameGlob->nextSnap;
        flashed = CG_Flashbanged(localClientNum);
        for (entityIndex = 0; entityIndex < nextSnap->numEntities; ++entityIndex)
        {
            cent = CG_GetEntity(localClientNum, nextSnap->entities[entityIndex].number);
            if (cent->nextState.eType == ET_PLAYER)
            {
                v2 = cgameGlob->nextSnap;
                v1 = (v2->ps.otherFlags & 6) != 0 && cent->nextState.number == v2->ps.clientNum;
                if ((!v1 || cgameGlob->renderingThirdPerson)
                    && team
                    && (team == TEAM_SPECTATOR || team == cgameGlob->bgs.clientinfo[cent->nextState.clientNum].team))
                {
                    if (!flashed && CG_CanSeeFriendlyHead(localClientNum, cent))
                    {
                        if (!overheadFade[cent->nextState.clientNum].visible)
                        {
                            overheadFade[cent->nextState.clientNum].visible = 1;
                            overheadFade[cent->nextState.clientNum].startTime = cgameGlob->time;
                        }
                    }
                    else
                    {
                        overheadFade[cent->nextState.clientNum].visible = 0;
                    }
                    if (overheadFade[cent->nextState.clientNum].visible)
                        overheadFade[cent->nextState.clientNum].lastTime = cgameGlob->time;
                    alpha = CG_FadeCrosshairNameAlpha(
                        cgameGlob->time,
                        overheadFade[cent->nextState.clientNum].startTime,
                        overheadFade[cent->nextState.clientNum].lastTime,
                        cg_friendlyNameFadeIn->current.integer,
                        cg_friendlyNameFadeOut->current.integer);
                    CG_DrawOverheadNames(localClientNum, cent, alpha);
                }
            }
        }
    }
}

void __cdecl CG_DrawOverheadNames(int32_t localClientNum, const centity_s *cent, float alpha)
{
    float v3; // [esp+34h] [ebp-108h]
    int32_t v4; // [esp+38h] [ebp-104h]
    float v5; // [esp+3Ch] [ebp-100h]
    int32_t v6; // [esp+40h] [ebp-FCh]
    double v7; // [esp+44h] [ebp-F8h]
    float v8; // [esp+4Ch] [ebp-F0h]
    int32_t v9; // [esp+50h] [ebp-ECh]
    float v10; // [esp+54h] [ebp-E8h]
    float v11; // [esp+58h] [ebp-E4h]
    int32_t v12; // [esp+5Ch] [ebp-E0h]
    double v13; // [esp+60h] [ebp-DCh]
    float v14; // [esp+68h] [ebp-D4h]
    float v15; // [esp+6Ch] [ebp-D0h]
    float v16; // [esp+70h] [ebp-CCh]
    float v17; // [esp+74h] [ebp-C8h]
    float v18; // [esp+78h] [ebp-C4h]
    float v19; // [esp+7Ch] [ebp-C0h]
    float diff[5]; // [esp+80h] [ebp-BCh] BYREF
    float iconSize; // [esp+94h] [ebp-A8h]
    float textSize; // [esp+98h] [ebp-A4h]
    float rankSize; // [esp+9Ch] [ebp-A0h]
    float distance; // [esp+A0h] [ebp-9Ch]
    float distFrac; // [esp+A4h] [ebp-98h]
    Font_s *font; // [esp+A8h] [ebp-94h]
    DObj_s *obj; // [esp+ACh] [ebp-90h]
    float origin[3]; // [esp+B0h] [ebp-8Ch] BYREF
    const cg_s *cgameGlob; // [esp+BCh] [ebp-80h]
    Material *material; // [esp+C0h] [ebp-7Ch] BYREF
    int32_t rank; // [esp+C4h] [ebp-78h]
    float distanceScale; // [esp+C8h] [ebp-74h]
    float viewPos[3]; // [esp+CCh] [ebp-70h] BYREF
    char textBuffer[40]; // [esp+D8h] [ebp-64h] BYREF
    float glow[4]; // [esp+104h] [ebp-38h] BYREF
    float scale; // [esp+114h] [ebp-28h]
    const playerState_s *ps; // [esp+118h] [ebp-24h]
    float color[4]; // [esp+11Ch] [ebp-20h] BYREF
    float x; // [esp+12Ch] [ebp-10h] BYREF
    float y; // [esp+130h] [ebp-Ch] BYREF
    const char *text; // [esp+134h] [ebp-8h]
    float distanceSq; // [esp+138h] [ebp-4h]

    if (alpha > EQUAL_EPSILON)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        ps = &cgameGlob->nextSnap->ps;
        font = UI_GetFontHandle(0, cg_overheadNamesFont->current.integer, 1.0);
        obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
        if (obj && CG_DObjGetWorldTagPos(&cent->pose, obj, scr_const.j_head, origin))
        {
            origin[2] = origin[2] + 10.0;
        }
        else
        {
            origin[0] = cent->pose.origin[0];
            origin[1] = cent->pose.origin[1];
            origin[2] = cent->pose.origin[2];
            origin[2] = origin[2] + 82.0;
        }
        if (CL_GetClientName(localClientNum, cent->nextState.clientNum, textBuffer, 38))
        {
            text = textBuffer;
            CG_RelativeTeamColor(cent->nextState.clientNum, "g_TeamColor", color, localClientNum);
            color[3] = alpha;
            if (CG_CalcNamePosition(localClientNum, origin, &x, &y))
            {
                viewPos[0] = cgameGlob->refdef.vieworg[0];
                viewPos[1] = cgameGlob->refdef.vieworg[1];
                viewPos[2] = cgameGlob->refdef.vieworg[2];
                Vec3Sub(origin, viewPos, diff);
                distanceSq = Vec3LengthSq(diff);
                v17 = cg_overheadNamesNearDist->current.value * cg_overheadNamesNearDist->current.value;
                if (distanceSq >= (double)v17)
                {
                    v16 = cg_overheadNamesFarDist->current.value * cg_overheadNamesFarDist->current.value;
                    if (distanceSq <= (double)v16)
                    {
                        v15 = sqrt(distanceSq);
                        distance = v15;
                        distFrac = (v15 - cg_overheadNamesNearDist->current.value)
                            / (cg_overheadNamesFarDist->current.value - cg_overheadNamesNearDist->current.value);
                        distanceScale = distFrac * cg_overheadNamesFarScale->current.value + 1.0 - distFrac;
                    }
                    else
                    {
                        distanceScale = cg_overheadNamesFarScale->current.value;
                    }
                }
                else
                {
                    distanceScale = 1.0;
                }
                v14 = cg_overheadNamesSize->current.value * distanceScale;
                scale = R_NormalizedTextScale(font, v14);
                v13 = scale * 0.5;
                v12 = R_TextWidth(text, 32, font);
                x = x - (double)v12 * v13;
                v19 = x + 0.5;
                v11 = floor(v19);
                x = v11;
                v18 = y + 0.5;
                v10 = floor(v18);
                y = v10;
                Dvar_GetUnpackedColor(cg_overheadNamesGlow, glow);
                glow[3] = glow[3] * alpha;
                CL_DrawTextPhysicalWithEffects((char *)text, 32, font, x, y, scale, scale, color, 3, glow, 0, 0, 0, 0, 0, 0);
                rank = cgameGlob->bgs.clientinfo[cent->nextState.clientNum].rank;
                CL_GetRankIcon(rank, cgameGlob->bgs.clientinfo[cent->nextState.clientNum].prestige, &material);
                if (material)
                {
                    v9 = R_TextHeight(font);
                    textSize = (double)v9 * scale;
                    iconSize = cg_overheadIconSize->current.value * textSize;
                    v8 = cg_overheadRankSize->current.value * distanceScale;
                    scale = R_NormalizedTextScale(font, v8);
                    text = CL_GetRankData(rank, MP_RANKTABLE_DISPLAYLEVEL);
                    v7 = distanceScale * 2.0 + iconSize;
                    v6 = R_TextWidth(text, 32, font);
                    x = x - ((double)v6 * scale + v7);
                    color[0] = 1.0;
                    color[1] = 1.0;
                    color[2] = 1.0;
                    color[3] = alpha;
                    v5 = y - (iconSize + textSize) * 0.5;
                    CL_DrawStretchPicPhysical(x, v5, iconSize, iconSize, 0.0, 0.0, 1.0, 1.0, color, material);
                    v4 = R_TextHeight(font);
                    rankSize = (double)v4 * scale;
                    x = x + iconSize;
                    v3 = rankSize * 0.25 + y;
                    CL_DrawTextPhysical((char *)text, 3, font, x, v3, scale, scale, color, 3);
                }
            }
        }
        else
        {
            Com_PrintError(14, "Unable to get name for client num: %i\n", cent->nextState.clientNum);
        }
    }
}

char __cdecl CG_CalcNamePosition(int32_t localClientNum, float *origin, float *xOut, float *yOut)
{
    ScreenPlacement *scrPlace; // [esp+0h] [ebp-24h]
    float projections[3]; // [esp+8h] [ebp-1Ch] BYREF
    const refdef_s *refdef; // [esp+14h] [ebp-10h]
    float x; // [esp+18h] [ebp-Ch]
    float y; // [esp+1Ch] [ebp-8h]
    float w; // [esp+20h] [ebp-4h]

    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    refdef = &cgameGlob->refdef;
    scrPlace = &scrPlaceView[localClientNum];
    CG_GetViewAxisProjections(&cgameGlob->refdef, origin, projections);
    w = projections[0];
    if (projections[0] < 0.0)
        return 0;
    x = projections[1] / refdef->tanHalfFovX;
    *xOut = scrPlace->realViewportSize[0] * 0.5 * (1.0 - x / w);
    y = projections[2] / refdef->tanHalfFovY;
    *yOut = scrPlace->realViewportSize[1] * 0.5 * (1.0 - y / w);
    return 1;
}

double __cdecl CG_FadeCrosshairNameAlpha(int32_t time, int32_t startMsec, int32_t lastMsec, int32_t fadeInMsec, int32_t fadeOutMsec)
{
    int32_t timeSinceLastOver; // [esp+8h] [ebp-4h]

    timeSinceLastOver = time - lastMsec;
    if (time - lastMsec >= fadeOutMsec)
        return 0.0;
    if (lastMsec - startMsec < fadeInMsec)
        return 0.0;
    if (timeSinceLastOver >= fadeOutMsec)
        return 1.0;
    return (float)((double)(fadeOutMsec - timeSinceLastOver) * 1.0 / (double)fadeOutMsec);
}

bool __cdecl CG_CanSeeFriendlyHead(int32_t localClientNum, const centity_s *cent)
{
    float v3; // [esp+8h] [ebp-8Ch]
    float v4; // [esp+Ch] [ebp-88h]
    float diff[3]; // [esp+10h] [ebp-84h] BYREF
    DObj_s *obj; // [esp+24h] [ebp-70h]
    const cg_s *cgameGlob; // [esp+28h] [ebp-6Ch]
    float contactEnd[3]; // [esp+2Ch] [ebp-68h] BYREF
    float start[3]; // [esp+38h] [ebp-5Ch] BYREF
    float end[3]; // [esp+44h] [ebp-50h] BYREF
    float eyeDelta[3]; // [esp+50h] [ebp-44h] BYREF
    float vis; // [esp+5Ch] [ebp-38h]
    trace_t trace; // [esp+60h] [ebp-34h] BYREF
    const playerState_s *ps; // [esp+8Ch] [ebp-8h]
    uint16_t hitEntId; // [esp+90h] [ebp-4h]

    if (cg_drawThroughWalls->current.enabled)
        return 1;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    ps = &cgameGlob->nextSnap->ps;
    start[0] = cgameGlob->refdef.vieworg[0];
    start[1] = cgameGlob->refdef.vieworg[1];
    start[2] = cgameGlob->refdef.vieworg[2];
    obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
    if (!obj || !CG_DObjGetWorldTagPos(&cent->pose, obj, scr_const.j_head, end))
    {
        end[0] = cent->pose.origin[0];
        end[1] = cent->pose.origin[1];
        end[2] = cent->pose.origin[2];
    }
    Vec3Sub(end, cgameGlob->refdef.vieworg, eyeDelta);
    if (Vec3Dot(eyeDelta, cgameGlob->refdef.viewaxis[0]) < 0.0)
        return 0;
    Vec3Sub(end, start, diff);
    v4 = Vec3LengthSq(diff);
    v3 = cg_overheadNamesMaxDist->current.value * cg_overheadNamesMaxDist->current.value;
    if (v4 > (double)v3)
        return 0;
    CG_TraceCapsule(&trace, start, (float *)vec3_origin, (float *)vec3_origin, end, ps->clientNum, 0x2803001);
    hitEntId = Trace_GetEntityHitId(&trace);
    if (hitEntId != ENTITYNUM_NONE && hitEntId != cent->nextState.clientNum)
        return 0;
    Vec3Lerp(start, end, trace.fraction, contactEnd);
    vis = FX_GetClientVisibility(localClientNum, start, contactEnd);
    return vis >= 0.2000000029802322;
}

void __cdecl CG_DrawCrosshairNames(int32_t localClientNum)
{
    int32_t entityIndex; // [esp+4h] [ebp-1Ch]
    team_t myTeam; // [esp+Ch] [ebp-14h]
    snapshot_s* nextSnap; // [esp+10h] [ebp-10h]
    centity_s* cent; // [esp+14h] [ebp-Ch]
    float alpha; // [esp+1Ch] [ebp-4h]
    cg_s *cgameGlob;

    if (cg_drawCrosshairNames->current.enabled)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);

        if (!cgameGlob->renderingThirdPerson && cgameGlob->crosshairClientNum <= 64)
        {
            bcassert(cgameGlob->nextSnap->ps.clientNum, MAX_CLIENTS);
            if (cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].infoValid)
            {
                bcassert(cgameGlob->crosshairClientNum, MAX_CLIENTS);
                if (cgameGlob->bgs.clientinfo[cgameGlob->crosshairClientNum].infoValid)
                {
                    nextSnap = cgameGlob->nextSnap;
                    for (entityIndex = 0; entityIndex < nextSnap->numEntities; ++entityIndex)
                    {
                        cent = CG_GetEntity(localClientNum, nextSnap->entities[entityIndex].number);
                        if (cent->nextState.eType == ET_PLAYER && cent->nextState.clientNum == cgameGlob->crosshairClientNum)
                        {
                            myTeam = cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team;
                            if (myTeam == TEAM_SPECTATOR
                                || myTeam && myTeam == cgameGlob->bgs.clientinfo[cgameGlob->crosshairClientNum].team)
                            {
                                alpha = CG_FadeCrosshairNameAlpha(
                                    cgameGlob->time,
                                    cgameGlob->crosshairClientStartTime,
                                    cgameGlob->crosshairClientLastTime,
                                    cg_friendlyNameFadeIn->current.integer,
                                    cg_friendlyNameFadeOut->current.integer);
                            }
                            else
                            {
                                alpha = CG_FadeCrosshairNameAlpha(
                                    cgameGlob->time,
                                    cgameGlob->crosshairClientStartTime,
                                    cgameGlob->crosshairClientLastTime,
                                    cg_enemyNameFadeIn->current.integer,
                                    cg_enemyNameFadeOut->current.integer);
                            }
                            CG_DrawOverheadNames(localClientNum, cent, alpha);
                            return;
                        }
                    }
                }
            }
        }
    }
}

void __cdecl DrawViewmodelInfo(int32_t localClientNum)
{
    const char *v1; // [esp+2Ch] [ebp-84Ch]
    const char *v2; // [esp+30h] [ebp-848h]
    const char *v3; // [esp+34h] [ebp-844h]
    const char *v4; // [esp+38h] [ebp-840h]
    const char *v5; // [esp+3Ch] [ebp-83Ch]
    const char *name; // [esp+40h] [ebp-838h]
    weaponInfo_s *weapInfo; // [esp+44h] [ebp-834h]
    char buffer[2052]; // [esp+48h] [ebp-830h] BYREF
    Font_s *font; // [esp+850h] [ebp-28h]
    const ScreenPlacement *scrPlace; // [esp+854h] [ebp-24h]
    XModel *weaponMdl; // [esp+858h] [ebp-20h]
    const cg_s *cgameGlob; // [esp+85Ch] [ebp-1Ch]
    const char *weaponMdlName; // [esp+860h] [ebp-18h]
    int32_t len; // [esp+864h] [ebp-14h]
    int32_t weaponIndex; // [esp+868h] [ebp-10h]
    float fov; // [esp+86Ch] [ebp-Ch]
    WeaponDef *weapDef; // [esp+870h] [ebp-8h]
    const playerState_s *ps; // [esp+874h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    ps = &cgameGlob->predictedPlayerState;
    weaponIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    scrPlace = &scrPlaceView[localClientNum];
    font = UI_GetFontHandle(scrPlace, 6, 0.25);
    if (weaponIndex > 0)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1095,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        weapInfo = &cg_weaponsArray[0][weaponIndex];
        weapDef = BG_GetWeaponDef(weaponIndex);
        weaponMdl = weapDef->gunXModel[cgameGlob->predictedPlayerState.weaponmodels[weaponIndex]];
        if (weaponMdl)
            name = weaponMdl->name;
        else
            name = 0;
        weaponMdlName = name;
        fov = CG_GetViewFov(localClientNum);
        if (weapInfo->knifeModel)
            v5 = weapInfo->knifeModel->name;
        else
            v5 = "none";
        if (weapInfo->rocketModel)
            v4 = weapInfo->rocketModel->name;
        else
            v4 = "none";
        if (weapInfo->gogglesModel)
            v3 = weapInfo->gogglesModel->name;
        else
            v3 = "none";
        if (weapInfo->handModel)
            v2 = weapInfo->handModel->name;
        else
            v2 = "none";
        if (weaponMdlName)
            v1 = weaponMdlName;
        else
            v1 = "none";
        Com_sprintf(
            buffer,
            0x800u,
            "^6%s\n"
            "^7Weapon: ^2%s^7 - ^5%s\n"
            "^7Hands: ^5%s\n"
            "^7Goggles: ^5%s\n"
            "^7Rocket: ^5%s\n"
            "^7Knife: ^5%s\n"
            "^7ADS: ^5%.2f ^7-^5 %.0f^7fov\n"
            "^7---Anims---\n"
            "^3",
            WeaponStateNames[ps->weaponstate],
            weapDef->szInternalName,
            v1,
            v2,
            v3,
            v4,
            v5,
            ps->fWeaponPosFrac,
            fov);
        len = &buffer[strlen(buffer) + 1] - &buffer[1];
        DObjDisplayAnimToBuffer(weapInfo->viewModelDObj, "", &buffer[len], 2048 - len);
        UI_DrawText(scrPlace, buffer, 2048, font, 0.0, 20.0, 1, 1, 0.25, colorWhite, 3);
    }
}

void __cdecl CG_DrawActive(int32_t localClientNum)
{
    float angles[3]; // [esp+8h] [ebp-10h] BYREF
    float fovSensitivityScale; // [esp+14h] [ebp-4h]

    cg_s *cgameGlob; // [esp+0h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    fovSensitivityScale = cgameGlob->zoomSensitivity;
    if (cgameGlob->shellshock.sensitivity != 0.0)
        fovSensitivityScale = fovSensitivityScale * cgameGlob->shellshock.sensitivity;

    CL_SetFOVSensitivityScale(localClientNum, fovSensitivityScale);
    Vec3Add(cgameGlob->kickAngles, cgameGlob->offsetAngles, angles);
    CL_SetUserCmdAimValues(localClientNum, angles);
    BG_AssertOffhandIndexOrNone(cgameGlob->equippedOffHand);
    CL_SetUserCmdWeapons(localClientNum, cgameGlob->weaponSelect, cgameGlob->equippedOffHand);
    CL_SetExtraButtons(localClientNum, cgameGlob->extraButtons);
    cgameGlob->extraButtons = 0;
    CL_RenderScene(&cgameGlob->refdef);
}

void __cdecl CG_AddSceneTracerBeams(int32_t localClientNum)
{
    CG_AddLocalEntityTracerBeams(localClientNum);
}

void __cdecl CG_GenerateSceneVerts(int32_t localClientNum)
{
    CG_AddAllPlayerSpriteDrawSurfs(localClientNum);
    CG_AddDrawSurfsFor3dHudElems(localClientNum);
}

void __cdecl CG_GetViewAxisProjections(const refdef_s *refdef, const float *worldPoint, float *projections)
{
    float eyeDelta[3]; // [esp+0h] [ebp-10h] BYREF
    int32_t i; // [esp+Ch] [ebp-4h]

    Vec3Sub(worldPoint, refdef->vieworg, eyeDelta);
    for (i = 0; i < 3; ++i)
    {
        projections[i] = Vec3Dot(eyeDelta, refdef->viewaxis[i]);
    }
}

