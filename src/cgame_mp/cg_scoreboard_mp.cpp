#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "cg_local_mp.h"
#include "cg_public_mp.h"
#include <client_mp/client_mp.h>
#include <database/database.h>
#include <ui_mp/ui_mp.h>
#include <stringed/stringed_hooks.h>
#include <client/client.h>
#include <ui/ui.h>


const dvar_t *cg_scoreboardMyColor;
const dvar_t *cg_scoreboardScrollStep;
const dvar_t *cg_ScoresPing_LowColor;
const dvar_t *cg_scoreboardHeight;
const dvar_t *cg_scoreboardFont;
const dvar_t *cg_scoreboardItemHeight;
const dvar_t *cg_ScoresPing_Interval;
const dvar_t *cg_scoreboardTextOffset;
const dvar_t *cg_ScoresPing_MedColor;
const dvar_t *cg_ScoresPing_HighColor;
const dvar_t *cg_scoreboardPingGraph;
const dvar_t *cg_ScoresPing_BgColor;
const dvar_t *cg_scoreboardPingHeight;
const dvar_t *cg_scoreboardPingWidth;
const dvar_t *cg_ScoresPing_MaxBars;
const dvar_t *cg_scoreboardHeaderFontScale;
const dvar_t *cg_scoreboardPingText;
const dvar_t *cg_scoreboardRankFontScale;
const dvar_t *cg_scoreboardWidth;
const dvar_t *cg_scoreboardBannerHeight;

const listColumnInfo_t columnInfo[8] =
{
  { LCT_RANK_ICON, 0.07f, "", 0 },
  { LCT_STATUS_ICON, 0.050000001f, "", 2 },
  { LCT_NAME, 0.43000001f, "", 0 },
  { LCT_TALKING_ICON, 0.050000001f, "", 0 },
  { LCT_SCORE, 0.1f, "CGAME_SB_SCORE", 2 },
  { LCT_KILLS, 0.1f, "CGAME_SB_KILLS", 2 },
  { LCT_ASSISTS, 0.1f, "CGAME_SB_ASSISTS", 2 },
  { LCT_DEATHS, 0.1f, "CGAME_SB_DEATHS", 2 }
}; // idb
const listColumnInfo_t columnInfoWithPing[9] =
{
  { LCT_RANK_ICON, 0.050000001f, "", 0 },
  { LCT_STATUS_ICON, 0.050000001f, "", 2 },
  { LCT_NAME, 0.34999999f, "", 0 },
  { LCT_TALKING_ICON, 0.050000001f, "", 0 },
  { LCT_SCORE, 0.1f, "CGAME_SB_SCORE", 2 },
  { LCT_KILLS, 0.1f, "CGAME_SB_KILLS", 2 },
  { LCT_ASSISTS, 0.1f, "CGAME_SB_ASSISTS", 2 },
  { LCT_DEATHS, 0.1f, "CGAME_SB_DEATHS", 2 },
  { LCT_PING, 0.1f, "CGAME_SB_PING", 2 }
}; // idb

void __cdecl UpdateScores(int localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cgameGlob->scoresRequestTime + 2000 < cgameGlob->time)
    {
        cgameGlob->scoresRequestTime = cgameGlob->time;
        CL_AddReliableCommand(localClientNum, "score");
    }
}

const score_t *__cdecl UI_GetOurClientScore(int localClientNum)
{
    return GetClientScore(localClientNum, CG_GetLocalClientGlobals(localClientNum)->clientNum);
}

const score_t *__cdecl GetClientScore(int localClientNum, int clientNum)
{
    int scoreNum; // [esp+4h] [ebp-8h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    for (scoreNum = 0; scoreNum < cgameGlob->numScores; ++scoreNum)
    {
        if (cgameGlob->bgs.clientinfo[cgameGlob->scores[scoreNum].client].infoValid
            && cgameGlob->scores[scoreNum].client == clientNum)
        {
            return &cgameGlob->scores[scoreNum];
        }
    }
    return 0;
}

const score_t *__cdecl UI_GetScoreAtRank(int localClientNum, int rank)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (rank < 1 || rank > cgameGlob->numScores)
        return 0;

    if (cgameGlob->bgs.clientinfo[cgameGlob->teamScores[10 * rank + 2]].infoValid)
        return (const score_t *)&cgameGlob->teamScores[10 * rank + 2];

    return 0;
}

char *__cdecl CG_GetGametypeDescription(int localClientNum)
{
    return SEH_LocalizeTextMessage(CG_GetLocalClientGlobals(localClientNum)->objectiveText, "game objective display", LOCMSG_SAFE);
}

char __cdecl CG_DrawScoreboard_GetTeamColorIndex(int team, int localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    bcassert(cgameGlob->clientNum, MAX_CLIENTS);

    if (team != 2 && team != 1)
        return 55;

    if (cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team != TEAM_ALLIES
        && cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team != TEAM_AXIS)
    {
        return 55;
    }

    if (cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team == team)
        return 56;

    return 57;
}

int __cdecl CG_DrawScoreboard(int localClientNum)
{
    const float *fadeColor; // [esp+4h] [ebp-Ch]
    float fade; // [esp+8h] [ebp-8h]
    cg_s *cgameGlob;

    if (cg_paused->current.integer)
        return 0;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (CG_IsScoreboardDisplayed(localClientNum))
    {
        fade = 1.0;
    }
    else
    {
        fadeColor = CG_FadeColor(cgameGlob->time, cgameGlob->scoreFadeTime, 100, 100);
        if (!fadeColor)
            return 0;
        fade = fadeColor[3];
    }
    UpdateScores(localClientNum);
    if (cgameGlob->scoresTop <= 0)
        CenterViewOnClient(localClientNum);
    iassert(cgameGlob->scoresTop > 0);
    CG_DrawScoreboard_Backdrop(localClientNum, fade);
    CG_DrawScoreboard_ScoresList(localClientNum, fade);
    return 1;
}

char szServerIPAddress[128];
char *__cdecl CL_GetServerIPAddress()
{
    __int16 v1; // ax
    clientConnection_t *clc; // [esp+0h] [ebp-4h]

    if (clientUIActives[0].connectionState >= CA_CONNECTED)
    {
        clc = CL_GetLocalClientConnection(0);
        v1 = BigShort(clc->serverAddress.port);
        Com_sprintf(
            szServerIPAddress,
            0x80u,
            "%i.%i.%i.%i:%i",
            clc->serverAddress.ip[0],
            clc->serverAddress.ip[1],
            clc->serverAddress.ip[2],
            clc->serverAddress.ip[3],
            v1);
    }
    else
    {
        memset(szServerIPAddress, 0, sizeof(szServerIPAddress));
    }
    return szServerIPAddress;
}

void __cdecl CG_DrawBackdropServerInfo(int localClientNum, float alpha)
{
    int v2; // esi
    double v3; // [esp+28h] [ebp-50h]
    float v4; // [esp+34h] [ebp-44h]
    float v5; // [esp+38h] [ebp-40h]
    float value; // [esp+3Ch] [ebp-3Ch]
    Font_s *footerFont; // [esp+40h] [ebp-38h]
    ScreenPlacement *scrPlace; // [esp+44h] [ebp-34h]
    char *serverName; // [esp+50h] [ebp-28h]
    char *serverIP; // [esp+54h] [ebp-24h]
    float x; // [esp+5Ch] [ebp-1Ch]
    float xa; // [esp+5Ch] [ebp-1Ch]
    float xb; // [esp+5Ch] [ebp-1Ch]
    float y; // [esp+60h] [ebp-18h]
    float color[4]; // [esp+64h] [ebp-14h] BYREF
    float fontScale; // [esp+74h] [ebp-4h]
    cgs_t *cgs;

    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = alpha;

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    serverName = cgs->szHostName;
    serverIP = CL_GetServerIPAddress();
    if (!I_stricmp(serverIP, "0.0.0.0:0"))
        serverIP = UI_SafeTranslateString("CGAME_LISTENSERVER");
    fontScale = CG_BannerScoreboardScaleMultiplier() * 0.3499999940395355;
    scrPlace = &scrPlaceView[localClientNum];
    do
    {
        footerFont = UI_GetFontHandle(scrPlace, cg_scoreboardFont->current.integer, fontScale);
        value = cg_scoreboardWidth->current.value;
        v2 = UI_TextWidth(serverName, 0, footerFont, fontScale);
        if (value - 6.0 - 8.0 >= (double)(v2 + UI_TextWidth(serverIP, 0, footerFont, fontScale) + 4))
            break;
        fontScale = fontScale - 0.009999999776482582;
    } while (fontScale > 0.07500000298023224);
    v5 = cg_scoreboardHeight->current.value;
    v3 = CG_BackdropTop() + v5 - 3.0 - 2.0 - 14.0 + 14.0;
    y = v3 - (double)(14 - UI_TextHeight(footerFont, fontScale)) * 0.5;
    x = CG_BackdropLeft(localClientNum) + 3.0 + 2.0 + 4.0;
    UI_DrawText(scrPlace, serverName, 0x7FFFFFFF, footerFont, x, y, 1, 0, fontScale, color, 3);
    v4 = cg_scoreboardWidth->current.value;
    xa = CG_BackdropLeft(localClientNum) + v4 - 3.0 - 2.0 - 4.0;
    xb = xa - (double)(UI_TextWidth(serverIP, 0, footerFont, fontScale) + 4);
    UI_DrawText(scrPlace, serverIP, 0x7FFFFFFF, footerFont, xb, y, 1, 0, fontScale, color, 3);
}

void __cdecl CG_DrawScoreboard_Backdrop(int localClientNum, float alpha)
{
    CG_BackdropLeft(localClientNum);
    CG_BackdropTop();
    CG_DrawBackdropServerInfo(localClientNum, alpha);
}

double __cdecl CG_BackdropLeft(int localClientNum)
{
    float v3; // [esp+4h] [ebp-14h]
    float v4; // [esp+8h] [ebp-10h]
    float v5; // [esp+10h] [ebp-8h]

    v5 = scrPlaceView[localClientNum].virtualViewableMax[0] - scrPlaceView[localClientNum].virtualViewableMin[0];
    v4 = (v5 - cg_scoreboardWidth->current.value) / 2.0;
    v3 = 0.0 - v4;
    if (v3 < 0.0)
        return (float)((v5 - cg_scoreboardWidth->current.value) / 2.0);
    else
        return (float)0.0;
}

double __cdecl CG_BackdropTop()
{
    float v2; // [esp+4h] [ebp-Ch]
    float v3; // [esp+8h] [ebp-8h]

    v3 = (480.0 - cg_scoreboardHeight->current.value) / 2.0;
    v2 = 0.0 - v3;
    if (v2 < 0.0)
        return (float)((480.0 - cg_scoreboardHeight->current.value) / 2.0);
    else
        return (float)0.0;
}

double __cdecl CG_BannerScoreboardScaleMultiplier()
{
    return 1.0;
}

void __cdecl CG_DrawScoreboard_ScoresList(int localClientNum, float alpha)
{
    float v2; // [esp+10h] [ebp-54h]
    double v3; // [esp+14h] [ebp-50h]
    float h; // [esp+1Ch] [ebp-48h]
    double v5; // [esp+20h] [ebp-44h]
    double v6; // [esp+28h] [ebp-3Ch]
    double integer; // [esp+30h] [ebp-34h]
    float scrollbarTop; // [esp+38h] [ebp-2Ch]
    team_t team; // [esp+3Ch] [ebp-28h]
    int teama; // [esp+3Ch] [ebp-28h]
    float listWidth; // [esp+44h] [ebp-20h]
    float yb; // [esp+48h] [ebp-1Ch]
    float yc; // [esp+48h] [ebp-1Ch]
    float y; // [esp+48h] [ebp-1Ch]
    float ya; // [esp+48h] [ebp-1Ch]
    float yd; // [esp+48h] [ebp-1Ch]
    float ye; // [esp+48h] [ebp-1Ch]
    float yf; // [esp+48h] [ebp-1Ch]
    float color[5]; // [esp+4Ch] [ebp-18h] BYREF
    int drawLine; // [esp+60h] [ebp-4h] BYREF
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cgameGlob->numScores)
    {
        iassert(cgameGlob->scoresTop <= CG_ScoreboardTotalLines(localClientNum));
        cgameGlob->scoresOffBottom = 0;
        yb = CG_BackdropTop() + 3.0 + 2.0 + 24.0 + 1.0;
        color[0] = 1.0;
        color[1] = 1.0;
        color[2] = 1.0;
        color[3] = alpha;
        listWidth = cg_scoreboardWidth->current.value - 6.0 - 4.0 - 8.0;
        integer = (double)cg_scoreboardItemHeight->current.integer;
        yc = CG_BannerScoreboardScaleMultiplier() * integer + 4.0 + yb;
        y = yc + 15.0;
        v6 = (double)cg_scoreboardBannerHeight->current.integer;
        scrollbarTop = CG_BannerScoreboardScaleMultiplier() * v6 + y;
        if (cgameGlob->scoresTop <= 1)
        {
            v3 = (double)cg_scoreboardBannerHeight->current.integer;
            v2 = CG_BannerScoreboardScaleMultiplier() * v3;
            CG_DrawScoreboard_ListColumnHeaders(localClientNum, color, y, v2, listWidth);
        }
        else
        {
            v5 = (double)cg_scoreboardBannerHeight->current.integer;
            h = CG_BannerScoreboardScaleMultiplier() * v5;
            y = CG_DrawScoreboard_ListColumnHeaders(localClientNum, color, y, h, listWidth);
        }
        color[4] = y;
        drawLine = 1;
        if (cgameGlob->teamPlayers[1] || cgameGlob->teamPlayers[2])
        {
            team = cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team;
            if (team != TEAM_AXIS && team != TEAM_ALLIES)
                team = TEAM_ALLIES;
            ya = CG_DrawTeamOfClientScore(localClientNum, color, y, team, listWidth, &drawLine);
            if (team == TEAM_AXIS)
                teama = 2;
            else
                teama = 1;
            yd = ya + 4.0;
            ye = CG_DrawTeamOfClientScore(localClientNum, color, yd, teama, listWidth, &drawLine);
            y = ye + 4.0;
        }
        if (cgameGlob->teamPlayers[0])
        {
            yf = CG_DrawTeamOfClientScore(localClientNum, color, y, 0, listWidth, &drawLine);
            y = yf + 4.0;
        }
        if (cgameGlob->teamPlayers[3])
            CG_DrawTeamOfClientScore(localClientNum, color, y, 3, listWidth, &drawLine);
        cgameGlob->scoresBottom = drawLine - 1;
        CG_DrawScrollbar(localClientNum, color, scrollbarTop);
    }
}

double __cdecl CG_DrawScoreboard_ListColumnHeaders(
    int localClientNum,
    const float *color,
    float y,
    float h,
    float listWidth)
{
    float v7; // [esp+20h] [ebp-40h]
    float v8; // [esp+24h] [ebp-3Ch]
    float v9; // [esp+28h] [ebp-38h]
    float v10; // [esp+30h] [ebp-30h]
    double v11; // [esp+34h] [ebp-2Ch]
    float scale; // [esp+3Ch] [ebp-24h]
    char *translation; // [esp+40h] [ebp-20h]
    const listColumnInfo_t *info; // [esp+44h] [ebp-1Ch] BYREF
    Font_s *font; // [esp+48h] [ebp-18h]
    const ScreenPlacement *scrPlace; // [esp+4Ch] [ebp-14h]
    float xAdj; // [esp+50h] [ebp-10h]
    int i; // [esp+54h] [ebp-Ch]
    float x; // [esp+58h] [ebp-8h]
    int fieldCount; // [esp+5Ch] [ebp-4h] BYREF

    scrPlace = &scrPlaceView[localClientNum];
    scale = CG_BannerScoreboardScaleMultiplier() * cg_scoreboardHeaderFontScale->current.value * 0.8500000238418579;
    font = UI_GetFontHandle(scrPlace, cg_scoreboardFont->current.integer, scale);
    x = CG_BackdropLeft(localClientNum) + 3.0 + 2.0 + 4.0;
    CG_GetScoreboardInfo(&info, &fieldCount);
    for (i = 0; i < fieldCount; ++i)
    {
        if (*info[i].pszName)
        {
            translation = UI_SafeTranslateString((char *)info[i].pszName);
            v11 = info[i].fWidth * listWidth;
            v10 = CG_BannerScoreboardScaleMultiplier() * cg_scoreboardHeaderFontScale->current.value * 0.8500000238418579;
            xAdj = (v11 - (double)UI_TextWidth(translation, 0, font, v10)) * 0.5;
            v9 = CG_BannerScoreboardScaleMultiplier() * cg_scoreboardHeaderFontScale->current.value * 0.8500000238418579;
            v8 = y + h;
            v7 = x + xAdj;
            UI_DrawText(scrPlace, translation, 0x7FFFFFFF, font, v7, v8, 1, 0, v9, color, 3);
        }
        x = info[i].fWidth * listWidth + x;
    }
    return (float)(y + h + 4.0);
}

void __cdecl CG_GetScoreboardInfo(const listColumnInfo_t **colInfo, int *numFields)
{
    if (cg_scoreboardPingText->current.enabled)
    {
        *colInfo = columnInfoWithPing;
        *numFields = 9;
    }
    else
    {
        *colInfo = columnInfo;
        *numFields = 8;
    }
}

int __cdecl CG_ScoreboardTotalLines(int localClientNum)
{
    int total; // [esp+4h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    total = cgameGlob->numScores;
    if (cgameGlob->teamPlayers[0])
        ++total;
    if (cgameGlob->teamPlayers[1])
        ++total;
    if (cgameGlob->teamPlayers[2])
        ++total;
    if (cgameGlob->teamPlayers[3])
        ++total;
    return total;
}

double __cdecl CG_DrawTeamOfClientScore(
    int localClientNum,
    const float *color,
    float y,
    int team,
    float listWidth,
    int *drawLine)
{
    float lineHeight; // [esp+14h] [ebp-38h]
    double v8; // [esp+18h] [ebp-34h]
    float h; // [esp+20h] [ebp-2Ch]
    double integer; // [esp+24h] [ebp-28h]
    float teamColor[4]; // [esp+34h] [ebp-18h] BYREF
    int i; // [esp+44h] [ebp-8h]
    const score_t *score; // [esp+48h] [ebp-4h]
    float ya; // [esp+5Ch] [ebp+10h]
    float yb; // [esp+5Ch] [ebp+10h]

    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    score = cgameGlob->scores;
    integer = (double)cg_scoreboardBannerHeight->current.integer;
    h = CG_BannerScoreboardScaleMultiplier() * integer;
    ya = CG_DrawScoreboard_ListBanner(localClientNum, color, y, listWidth, h, team, drawLine);
    i = 0;
    while (i < cgameGlob->numScores)
    {
        if (score->client >= 0x40u)
            MyAssertHandler(
                ".\\cgame_mp\\cg_scoreboard_mp.cpp",
                1037,
                0,
                "score->client doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                score->client,
                64);
        if (cgameGlob->bgs.clientinfo[score->client].infoValid && score->team == team)
        {
            v8 = (double)cg_scoreboardItemHeight->current.integer;
            lineHeight = CG_BannerScoreboardScaleMultiplier() * v8;
            if (CG_CheckDrawScoreboardLine(localClientNum, drawLine, ya, lineHeight))
            {
                CG_TeamColor(team, "g_ScoresColor", teamColor);
                teamColor[3] = color[3];
                yb = CG_DrawClientScore(localClientNum, teamColor, ya, score, listWidth);
                ya = yb + 4.0;
            }
        }
        ++i;
        ++score;
    }
    return ya;
}

int __cdecl CG_CheckDrawScoreboardLine(int localClientNum, int *drawLine, float y, float lineHeight)
{
    float value; // [esp+8h] [ebp-8h]

    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cgameGlob->scoresOffBottom)
        return 0;

    if (*drawLine >= cgameGlob->scoresTop)
    {
        value = cg_scoreboardHeight->current.value;
        if (CG_BackdropTop() + value - 3.0 - 2.0 - 14.0 - 1.0 >= y + lineHeight)
        {
            ++*drawLine;
            return 1;
        }
        else
        {
            cgameGlob->scoresOffBottom = 1;
            return 0;
        }
    }
    else
    {
        ++*drawLine;
        return 0;
    }
}

double __cdecl CG_DrawScoreboard_ListBanner(
    int localClientNum,
    const float *color,
    float y,
    float w,
    float h,
    int team,
    int *piDrawLine)
{
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    float v12; // [esp+28h] [ebp-40h]
    float v13; // [esp+30h] [ebp-38h]
    float v14; // [esp+34h] [ebp-34h]
    float scale; // [esp+38h] [ebp-30h]
    float v16; // [esp+3Ch] [ebp-2Ch]
    float v17; // [esp+44h] [ebp-24h]
    Font_s *bannerFont; // [esp+48h] [ebp-20h]
    const char *teamName; // [esp+4Ch] [ebp-1Ch]
    const char *teamNamea; // [esp+4Ch] [ebp-1Ch]
    ScreenPlacement *scrPlace; // [esp+50h] [ebp-18h]
    Material *material; // [esp+58h] [ebp-10h]
    char *shaderName; // [esp+5Ch] [ebp-Ch]
    char *displayString; // [esp+60h] [ebp-8h]
    char *displayStringa; // [esp+60h] [ebp-8h]
    float x; // [esp+64h] [ebp-4h]
    float xa; // [esp+64h] [ebp-4h]

    if (!CG_CheckDrawScoreboardLine(localClientNum, piDrawLine, y, h))
        return y;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    scrPlace = &scrPlaceView[localClientNum];
    v17 = CG_BannerScoreboardScaleMultiplier() * 0.3499999940395355;
    bannerFont = UI_GetFontHandle(scrPlace, cg_scoreboardFont->current.integer, v17);
    if (team)
    {
        if (team == 1)
        {
            shaderName = (char *)Dvar_GetString("g_TeamIcon_Axis");
            teamName = Dvar_GetString("g_TeamName_Axis");
            v8 = SEH_LocalizeTextMessage(teamName, "scoreboard team name", LOCMSG_SAFE);
            displayString = va("%s", v8);
        }
        else if (team == 2)
        {
            shaderName = (char *)Dvar_GetString("g_TeamIcon_Allies");
            teamNamea = Dvar_GetString("g_TeamName_Allies");
            v9 = SEH_LocalizeTextMessage(teamNamea, "scoreboard team name", LOCMSG_SAFE);
            displayString = va("%s", v9);
        }
        else
        {
            if (team != 3)
                MyAssertHandler(
                    ".\\cgame_mp\\cg_scoreboard_mp.cpp",
                    666,
                    0,
                    "%s\n\t(team) = %i",
                    "(team == TEAM_SPECTATOR)",
                    team);
            shaderName = (char *)Dvar_GetString("g_TeamIcon_Spectator");
            v10 = SEH_LocalizeTextMessage("CGAME_SPECTATORS", "scoreboard team name", LOCMSG_SAFE);
            displayString = va("%s", v10);
        }
    }
    else
    {
        shaderName = (char *)Dvar_GetString("g_TeamIcon_Free");
        displayString = (char *)"";
    }
    x = CG_BackdropLeft(localClientNum) + 3.0 + 2.0 + 4.0;
    material = Material_RegisterHandle(shaderName, 7);
    if (!Material_IsDefault(material))
    {
        v16 = CG_BackdropLeft(localClientNum) + 3.0 + 2.0 + 4.0;
        UI_DrawHandlePic(scrPlace, v16, y, h, h, 1, 0, color, material);
        x = h + 8.0 + x;
    }
    scale = CG_BannerScoreboardScaleMultiplier() * 0.3499999940395355;
    v14 = y + h;
    UI_DrawText(scrPlace, displayString, 0x7FFFFFFF, bannerFont, x, v14, 1, 0, scale, color, 3);
    v13 = CG_BannerScoreboardScaleMultiplier() * 0.3499999940395355;
    xa = (double)(UI_TextWidth(displayString, 0x7FFFFFFF, bannerFont, v13) + 8) + x;
    displayStringa = va("( %i )", CG_GetLocalClientGlobals(localClientNum)->teamPlayers[team]);
    v12 = CG_BannerScoreboardScaleMultiplier() * 0.3499999940395355;
    UI_DrawText(scrPlace, displayStringa, 0x7FFFFFFF, bannerFont, xa, v14, 1, 0, v12, color, 3);
    return (float)(y + h + 4.0);
}

double __cdecl CG_DrawClientScore(
    int localClientNum,
    const float *color,
    float y,
    const score_t *score,
    float listWidth)
{
    float width; // [esp+8h] [ebp-134h]
    int iAlignment; // [esp+Ch] [ebp-130h]
    Font_s *v8; // [esp+10h] [ebp-12Ch]
    float value; // [esp+14h] [ebp-128h]
    Material *scale; // [esp+1Ch] [ebp-120h]
    Material *scalea; // [esp+1Ch] [ebp-120h]
    Material *scaleb; // [esp+1Ch] [ebp-120h]
    double v14; // [esp+24h] [ebp-118h]
    float v15; // [esp+2Ch] [ebp-110h]
    float v16; // [esp+30h] [ebp-10Ch]
    double v17; // [esp+34h] [ebp-108h]
    float v18; // [esp+3Ch] [ebp-100h]
    float v19; // [esp+40h] [ebp-FCh]
    float v20; // [esp+44h] [ebp-F8h]
    float v21; // [esp+48h] [ebp-F4h]
    float v22; // [esp+4Ch] [ebp-F0h]
    float v23; // [esp+50h] [ebp-ECh]
    float v24; // [esp+54h] [ebp-E8h]
    double v25; // [esp+58h] [ebp-E4h]
    float v26; // [esp+60h] [ebp-DCh]
    double v27; // [esp+64h] [ebp-D8h]
    float v28; // [esp+6Ch] [ebp-D0h]
    double v29; // [esp+70h] [ebp-CCh]
    double v30; // [esp+78h] [ebp-C4h]
    float v31; // [esp+80h] [ebp-BCh]
    float v32; // [esp+88h] [ebp-B4h]
    float v33; // [esp+8Ch] [ebp-B0h]
    double v34; // [esp+90h] [ebp-ACh]
    float v35; // [esp+98h] [ebp-A4h]
    double v36; // [esp+9Ch] [ebp-A0h]
    float v37; // [esp+A4h] [ebp-98h]
    double v38; // [esp+A8h] [ebp-94h]
    float v39; // [esp+B0h] [ebp-8Ch]
    float v40; // [esp+B4h] [ebp-88h]
    double v41; // [esp+B8h] [ebp-84h]
    float v42; // [esp+C0h] [ebp-7Ch]
    double v43; // [esp+C4h] [ebp-78h]
    float v44; // [esp+CCh] [ebp-70h]
    double v45; // [esp+D0h] [ebp-6Ch]
    float v46; // [esp+DCh] [ebp-60h]
    double integer; // [esp+E0h] [ebp-5Ch]
    const listColumnInfo_t *info; // [esp+E8h] [ebp-54h] BYREF
    const ScreenPlacement *scrPlace; // [esp+ECh] [ebp-50h]
    cg_s *cgameGlob; // [esp+F0h] [ebp-4Ch]
    const char *string; // [esp+F4h] [ebp-48h]
    Material *material; // [esp+F8h] [ebp-44h]
    float xAdj; // [esp+FCh] [ebp-40h]
    float textColor[4]; // [esp+100h] [ebp-3Ch] BYREF
    Font_s *listFont; // [esp+110h] [ebp-2Ch]
    float backColor[4]; // [esp+114h] [ebp-28h] BYREF
    clientInfo_t *ci; // [esp+124h] [ebp-18h]
    int i; // [esp+128h] [ebp-14h]
    float x; // [esp+12Ch] [ebp-10h]
    float h; // [esp+130h] [ebp-Ch]
    int fieldCount; // [esp+134h] [ebp-8h] BYREF
    float w; // [esp+138h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    bcassert(score->client, MAX_CLIENTS);

    ci = &cgameGlob->bgs.clientinfo[score->client];
    if (!ci->infoValid)
        return y;
    scrPlace = &scrPlaceView[localClientNum];
    CG_GetScoreboardInfo(&info, &fieldCount);
    x = CG_BackdropLeft(localClientNum) + 3.0 + 2.0 + 4.0;
    integer = (double)cg_scoreboardItemHeight->current.integer;
    h = CG_BannerScoreboardScaleMultiplier() * integer;
    material = Material_RegisterHandle("white", 7);
    backColor[0] = *color;
    backColor[1] = color[1];
    backColor[2] = color[2];
    backColor[3] = color[3] * 0.5;
    UI_DrawHandlePic(scrPlace, x, y, listWidth, h, 1, 0, backColor, material);
    v46 = CG_BannerScoreboardScaleMultiplier() * 0.3499999940395355;
    listFont = UI_GetFontHandle(scrPlace, cg_scoreboardFont->current.integer, v46);
    x = CG_BackdropLeft(localClientNum) + 3.0 + 2.0 + 4.0;
    if (score->client == cgameGlob->clientNum)
    {
        Dvar_GetUnpackedColor(cg_scoreboardMyColor, textColor);
    }
    else
    {
        textColor[0] = 1.0;
        textColor[1] = 1.0;
        textColor[2] = 1.0;
    }
    textColor[3] = color[3];
    for (i = 0; i < fieldCount; ++i)
    {
        w = info[i].fWidth * listWidth;
        switch (info[i].type)
        {
        case LCT_NAME:
            string = ci->name;
            v23 = CG_BannerScoreboardScaleMultiplier() * 0.3499999940395355;
            DrawListString(localClientNum, (char *)string, x, y, w, info[i].iAlignment, listFont, v23, 3, textColor);
            break;
        case LCT_SCORE:
            if (score->team != 3)
            {
                string = va("%i", score->score);
                v22 = CG_BannerScoreboardScaleMultiplier() * 0.3499999940395355;
                DrawListString(localClientNum, (char *)string, x, y, w, info[i].iAlignment, listFont, v22, 3, textColor);
            }
            break;
        case LCT_DEATHS:
            if (score->team != 3)
            {
                string = va("%i", score->deaths);
                v21 = CG_BannerScoreboardScaleMultiplier() * 0.3499999940395355;
                DrawListString(localClientNum, (char *)string, x, y, w, info[i].iAlignment, listFont, v21, 3, textColor);
            }
            break;
        case LCT_PING:
            string = va("%i", score->ping);
            v18 = CG_BannerScoreboardScaleMultiplier() * 0.3499999940395355;
            DrawListString(localClientNum, (char *)string, x, y, w, info[i].iAlignment, listFont, v18, 3, textColor);
            break;
        case LCT_STATUS_ICON:
            if (score->hStatusIcon)
            {
                v45 = (double)cg_scoreboardItemHeight->current.integer;
                v44 = CG_BannerScoreboardScaleMultiplier() * v45;
                xAdj = CalcXAdj(info[i].iAlignment, w, v44);
                backColor[0] = 1.0;
                backColor[1] = 1.0;
                backColor[2] = 1.0;
                backColor[3] = color[3];
                scale = score->hStatusIcon;
                v43 = (double)cg_scoreboardItemHeight->current.integer;
                v42 = CG_BannerScoreboardScaleMultiplier() * v43;
                v41 = (double)cg_scoreboardItemHeight->current.integer;
                v40 = CG_BannerScoreboardScaleMultiplier() * v41;
                v39 = x + xAdj;
                UI_DrawHandlePic(scrPlace, v39, y, v40, v42, 1, 0, backColor, scale);
            }
            break;
        case LCT_TALKING_ICON:
            if (CL_IsPlayerMuted(localClientNum, score->client))
            {
                material = Material_RegisterHandle("voice_off", 7);
            }
            else if (CL_IsPlayerTalking(localClientNum, score->client))
            {
                material = Material_RegisterHandle("voice_on", 7);
            }
            else
            {
                material = 0;
            }
            if (material)
            {
                backColor[0] = 1.0;
                backColor[1] = 1.0;
                backColor[2] = 1.0;
                backColor[3] = color[3];
                scaleb = material;
                v27 = (double)cg_scoreboardItemHeight->current.integer;
                v26 = CG_BannerScoreboardScaleMultiplier() * v27;
                v25 = (double)cg_scoreboardItemHeight->current.integer;
                v24 = CG_BannerScoreboardScaleMultiplier() * v25;
                UI_DrawHandlePic(scrPlace, x, y, v24, v26, 1, 0, backColor, scaleb);
            }
            break;
        case LCT_KILLS:
            if (score->team != 3)
            {
                string = va("%i", score->kills);
                v20 = CG_BannerScoreboardScaleMultiplier() * 0.3499999940395355;
                DrawListString(localClientNum, (char *)string, x, y, w, info[i].iAlignment, listFont, v20, 3, textColor);
            }
            break;
        case LCT_RANK_ICON:
            if (score->hRankIcon)
            {
                v38 = (double)cg_scoreboardItemHeight->current.integer;
                v37 = CG_BannerScoreboardScaleMultiplier() * v38;
                xAdj = CalcXAdj(info[i].iAlignment, w, v37);
                backColor[0] = 1.0;
                backColor[1] = 1.0;
                backColor[2] = 1.0;
                backColor[3] = color[3];
                scalea = score->hRankIcon;
                v36 = (double)cg_scoreboardItemHeight->current.integer;
                v35 = CG_BannerScoreboardScaleMultiplier() * v36;
                v34 = (double)cg_scoreboardItemHeight->current.integer;
                v33 = CG_BannerScoreboardScaleMultiplier() * v34;
                v32 = x + xAdj;
                UI_DrawHandlePic(scrPlace, v32, y, v33, v35, 1, 0, backColor, scalea);
                string = CL_GetRankData(score->rank, MP_RANKTABLE_DISPLAYLEVEL);
                value = cg_scoreboardRankFontScale->current.value;
                v8 = listFont;
                iAlignment = info[i].iAlignment;
                width = w;
                v31 = (double)UI_TextHeight(listFont, value) * 0.25 + y;
                v30 = x + xAdj;
                v29 = (double)cg_scoreboardItemHeight->current.integer;
                v28 = CG_BannerScoreboardScaleMultiplier() * v29 + v30;
                DrawListString(localClientNum, (char *)string, v28, v31, width, iAlignment, v8, value, 3, colorWhite);
            }
            break;
        case LCT_ASSISTS:
            if (score->team != 3)
            {
                string = va("%i", score->assists);
                v19 = CG_BannerScoreboardScaleMultiplier() * 0.3499999940395355;
                DrawListString(localClientNum, (char *)string, x, y, w, info[i].iAlignment, listFont, v19, 3, textColor);
            }
            break;
        default:
            break;
        }
        x = x + w;
    }
    if (cg_scoreboardPingGraph->current.enabled)
    {
        v17 = (double)cg_scoreboardItemHeight->current.integer;
        v16 = CG_BannerScoreboardScaleMultiplier() * v17;
        v15 = listWidth * cg_scoreboardPingWidth->current.value;
        CG_DrawClientPing(localClientNum, score->ping, x, y, v15, v16);
    }
    v14 = (double)cg_scoreboardItemHeight->current.integer;
    return (float)(CG_BannerScoreboardScaleMultiplier() * v14 + y);
}

double __cdecl CalcXAdj(int align, float maxw, float w)
{
    if (align == 1)
    {
        return (float)((maxw - w) / 2.0);
    }
    else if (align == 2)
    {
        return (float)(maxw - w);
    }
    else
    {
        if (align)
            MyAssertHandler(".\\cgame_mp\\cg_scoreboard_mp.cpp", 698, 0, "%s", "align == UI_LEFT");
        return 0.0;
    }
}

void __cdecl DrawListString(
    int localClientNum,
    char *string,
    float x,
    float y,
    float width,
    int alignment,
    Font_s *font,
    float scale,
    int style,
    const float *color)
{
    float maxw_4; // [esp+18h] [ebp-38h]
    float v11; // [esp+1Ch] [ebp-34h]
    float v12; // [esp+20h] [ebp-30h]
    double v13; // [esp+28h] [ebp-28h]
    double integer; // [esp+30h] [ebp-20h]
    double value; // [esp+38h] [ebp-18h]
    float xAdj; // [esp+4Ch] [ebp-4h]

    if (string)
    {
        while (width < (double)UI_TextWidth(string, 0x7FFFFFFF, font, scale))
            scale = scale - 0.02500000037252903;
        if (scale <= 0.0)
            MyAssertHandler(
                ".\\cgame_mp\\cg_scoreboard_mp.cpp",
                717,
                0,
                "%s\n\t(scale) = %i",
                "(scale > 0)",
                //(unsigned int)COERCE_UNSIGNED_INT64(scale));
                (unsigned int)(unsigned __int64)(scale));
        if (scale < 0.2000000029802322)
            style = 0;
        maxw_4 = (float)UI_TextWidth(string, 0x7FFFFFFF, font, scale);
        xAdj = CalcXAdj(alignment, width, maxw_4);
        if (alignment == 2)
            xAdj = xAdj - 4.0;
        value = cg_scoreboardTextOffset->current.value;
        integer = (double)cg_scoreboardItemHeight->current.integer;
        v13 = CG_BannerScoreboardScaleMultiplier() * integer;
        v12 = ((double)UI_TextHeight(font, scale) + v13) * value + y;
        v11 = x + xAdj;
        UI_DrawText(&scrPlaceView[localClientNum], string, 0x7FFFFFFF, font, v11, v12, 1, 0, scale, color, style);
    }
}

void __cdecl CG_DrawClientPing(int localClientNum, int ping, float x, float y, float maxWidth, float maxHeight)
{
    float v6; // [esp+20h] [ebp-74h]
    int v7; // [esp+30h] [ebp-64h]
    float v8; // [esp+34h] [ebp-60h]
    float v9; // [esp+38h] [ebp-5Ch]
    int maxBars; // [esp+44h] [ebp-50h]
    ScreenPlacement *scrPlace; // [esp+48h] [ebp-4Ch]
    Material *materiala; // [esp+4Ch] [ebp-48h]
    Material *material; // [esp+4Ch] [ebp-48h]
    int bar; // [esp+50h] [ebp-44h]
    float lerp; // [esp+54h] [ebp-40h]
    float endColor[4]; // [esp+58h] [ebp-3Ch] BYREF
    int interval; // [esp+68h] [ebp-2Ch]
    float startColor[4]; // [esp+6Ch] [ebp-28h] BYREF
    float color[4]; // [esp+7Ch] [ebp-18h] BYREF
    float h; // [esp+8Ch] [ebp-8h]
    float w; // [esp+90h] [ebp-4h]
    float xa; // [esp+A4h] [ebp+10h]

    scrPlace = &scrPlaceView[localClientNum];
    materiala = Material_RegisterHandle("white", 7);
    Dvar_GetUnpackedColorByName("cg_ScoresPing_BgColor", color);
    v9 = maxWidth + 2.0;
    v8 = x + 8.0 - 1.0;
    UI_DrawHandlePic(scrPlace, v8, y, v9, maxHeight, 1, 0, color, materiala);
    maxBars = Dvar_GetInt("cg_ScoresPing_MaxBars");
    interval = Dvar_GetInt("cg_ScoresPing_Interval");
    if (interval <= 0)
        MyAssertHandler(".\\cgame_mp\\cg_scoreboard_mp.cpp", 757, 0, "%s\n\t(interval) = %i", "(interval > 0)", interval);
    material = Material_RegisterHandle("white", 7);
    if (maxBars - ping / interval < 1)
        v7 = 1;
    else
        v7 = maxBars - ping / interval;
    if (v7 >= maxBars / 2)
    {
        Dvar_GetUnpackedColorByName("cg_ScoresPing_MedColor", startColor);
        Dvar_GetUnpackedColorByName("cg_ScoresPing_LowColor", endColor);
        lerp = (double)(v7 - maxBars / 2) / (double)(maxBars / 2);
    }
    else
    {
        Dvar_GetUnpackedColorByName("cg_ScoresPing_HighColor", startColor);
        Dvar_GetUnpackedColorByName("cg_ScoresPing_MedColor", endColor);
        lerp = (double)v7 / (double)(maxBars / 2);
    }
    Vec4Lerp(startColor, endColor, lerp, color);
    xa = x + 8.0;
    w = maxWidth / (double)maxBars - 1.0;
    if (w < 1.0)
        w = 1.0;
    for (bar = 1; bar <= v7; ++bar)
    {
        h = maxHeight * cg_scoreboardPingHeight->current.value * (double)bar / (double)maxBars;
        v6 = y + maxHeight - h;
        UI_DrawHandlePic(scrPlace, xa, v6, w, h, 1, 0, color, material);
        xa = w + 1.0 + xa;
    }
}

void __cdecl CG_DrawScrollbar(int localClientNum, const float *color, float top)
{
    float v3; // [esp+28h] [ebp-44h]
    float v4; // [esp+2Ch] [ebp-40h]
    float v5; // [esp+30h] [ebp-3Ch]
    float v6; // [esp+34h] [ebp-38h]
    float value; // [esp+38h] [ebp-34h]
    ScreenPlacement *scrPlace; // [esp+3Ch] [ebp-30h]
    Material *material; // [esp+44h] [ebp-28h]
    Material *materiala; // [esp+44h] [ebp-28h]
    Material *materialb; // [esp+44h] [ebp-28h]
    Material *materialc; // [esp+44h] [ebp-28h]
    Material *materiald; // [esp+44h] [ebp-28h]
    Material *materiale; // [esp+44h] [ebp-28h]
    int totalLines; // [esp+48h] [ebp-24h]
    float barColor[4]; // [esp+4Ch] [ebp-20h] BYREF
    float x; // [esp+5Ch] [ebp-10h]
    float y; // [esp+60h] [ebp-Ch]
    float h; // [esp+64h] [ebp-8h]
    float w; // [esp+68h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cgameGlob->scoresTop > 1 || cgameGlob->scoresOffBottom)
    {
        totalLines = CG_ScoreboardTotalLines(localClientNum);
        scrPlace = &scrPlaceView[localClientNum];
        barColor[0] = *color;
        barColor[1] = color[1];
        barColor[2] = color[2];
        material = Material_RegisterHandle("black", 7);
        barColor[3] = color[3] * 0.5;
        value = cg_scoreboardWidth->current.value;
        x = CG_BackdropLeft(localClientNum)
            + 3.0
            + 2.0
            + 4.0
            + (value - 6.0 - 4.0 - 8.0) * (cg_scoreboardPingWidth->current.value + 1.0)
            + 8.0
            + 4.0;
        y = top;
        w = 8.0;
        v6 = cg_scoreboardHeight->current.value;
        h = CG_BackdropTop() + v6 - 3.0 - 2.0 - 14.0 - 1.0 - top - 1.0;
        UI_DrawHandlePic(scrPlace, x, top, 8.0, h, 1, 0, barColor, material);
        x = x + 1.0;
        w = w - 2.0;
        y = y + 1.0;
        h = h - 2.0;
        if (totalLines)
        {
            y = (double)(cgameGlob->scoresTop - 1) / (double)totalLines * h + y;
            h = (double)(cgameGlob->scoresBottom - cgameGlob->scoresTop + 1) / (double)totalLines * h;
        }
        materiala = Material_RegisterHandle("white", 7);
        barColor[3] = color[3] * 0.25;
        UI_DrawHandlePic(scrPlace, x, y, w, h, 1, 0, barColor, materiala);
        barColor[3] = color[3];
        if (cgameGlob->scoresTop > 1)
        {
            materialb = Material_RegisterHandle("hudscoreboardscroll_uparrow", 7);
            v5 = cg_scoreboardWidth->current.value;
            x = CG_BackdropLeft(localClientNum)
                + 3.0
                + 2.0
                + 4.0
                + (v5 - 6.0 - 4.0 - 8.0) * (cg_scoreboardPingWidth->current.value + 1.0)
                + 8.0
                + 8.0
                + 8.0
                + 2.0
                + 0.0;
            y = top;
            w = 16.0;
            h = 16.0;
            UI_DrawHandlePic(scrPlace, x, top, 16.0, 16.0, 1, 0, barColor, materialb);
            materialc = Material_RegisterHandle("hudscoreboardscroll_upkey", 7);
            x = x - 0.0;
            y = y + 18.0;
            w = 16.0;
            h = 16.0;
            UI_DrawHandlePic(scrPlace, x, y, 16.0, 16.0, 1, 0, barColor, materialc);
        }
        if (cgameGlob->scoresOffBottom)
        {
            materiald = Material_RegisterHandle("hudscoreboardscroll_downarrow", 7);
            v4 = cg_scoreboardWidth->current.value;
            x = CG_BackdropLeft(localClientNum)
                + 3.0
                + 2.0
                + 4.0
                + (v4 - 6.0 - 4.0 - 8.0) * (cg_scoreboardPingWidth->current.value + 1.0)
                + 8.0
                + 8.0
                + 8.0
                + 2.0
                + 0.0;
            v3 = cg_scoreboardHeight->current.value;
            y = CG_BackdropTop() + v3 - 3.0 - 2.0 - 14.0 - 1.0 - 1.0 - 16.0;
            w = 16.0;
            h = 16.0;
            UI_DrawHandlePic(scrPlace, x, y, 16.0, 16.0, 1, 0, barColor, materiald);
            materiale = Material_RegisterHandle("hudscoreboardscroll_downkey", 7);
            x = x - 0.0;
            y = y - 18.0;
            w = 16.0;
            h = 16.0;
            UI_DrawHandlePic(scrPlace, x, y, 16.0, 16.0, 1, 0, barColor, materiale);
        }
    }
}

void __cdecl CenterViewOnClient(int localClientNum)
{
    double v1; // [esp+0h] [ebp-54h]
    double v2; // [esp+8h] [ebp-4Ch]
    double v3; // [esp+10h] [ebp-44h]
    double v4; // [esp+18h] [ebp-3Ch]
    double integer; // [esp+20h] [ebp-34h]
    double v6; // [esp+28h] [ebp-2Ch]
    double v7; // [esp+30h] [ebp-24h]
    float value; // [esp+38h] [ebp-1Ch]
    int clientLine; // [esp+3Ch] [ebp-18h]
    team_t team; // [esp+40h] [ebp-14h]
    int viewmax; // [esp+48h] [ebp-Ch]
    int i; // [esp+4Ch] [ebp-8h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    value = cg_scoreboardHeight->current.value;
    v7 = CG_BackdropTop() + value - 3.0 - 2.0 - 14.0;
    v6 = v7 - (CG_BackdropTop() + 3.0 + 2.0 + 24.0 + 1.0) - 1.0;
    integer = (double)cg_scoreboardBannerHeight->current.integer;
    v4 = v6 - CG_BannerScoreboardScaleMultiplier() * integer;
    v3 = (double)cg_scoreboardItemHeight->current.integer;
    v2 = v4 - CG_BannerScoreboardScaleMultiplier() * v3 - 4.0;
    v1 = (double)cg_scoreboardItemHeight->current.integer;
    viewmax = (int)(v2 / (CG_BannerScoreboardScaleMultiplier() * v1 + 4.0));
    team = cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team;
    clientLine = 1;
    if (team == TEAM_SPECTATOR)
    {
        cgameGlob->scoresTop = 1;
    }
    else
    {
        for (i = 0; i < cgameGlob->numScores; ++i)
        {
            if (cgameGlob->scores[i].team == team)
            {
                if (cgameGlob->scores[i].client == cgameGlob->clientNum)
                    break;
                ++clientLine;
            }
        }
        if (clientLine > viewmax)
            cgameGlob->scoresTop = clientLine - viewmax / 2 + 1;
        else
            cgameGlob->scoresTop = 1;
    }
}

int __cdecl CG_IsScoreboardDisplayed(int localClientNum)
{
    return CG_GetLocalClientGlobals(localClientNum)->showScores;
}

void __cdecl CG_ScrollScoreboardUp(cg_s *cgameGlob)
{
    if (cgameGlob->scoresTop > 1)
    {
        cgameGlob->scoresTop -= cg_scoreboardScrollStep->current.integer;
        if (cgameGlob->scoresTop < 2)
            cgameGlob->scoresTop = 1;
    }
}

void __cdecl CG_ScrollScoreboardDown(cg_s *cgameGlob)
{
    int totalLines; // [esp+0h] [ebp-4h]

    if (cgameGlob->scoresOffBottom)
    {
        totalLines = CG_ScoreboardTotalLines(cgameGlob->localClientNum);
        cgameGlob->scoresTop += cg_scoreboardScrollStep->current.integer;
        if (cgameGlob->scoresTop > totalLines)
            cgameGlob->scoresTop = totalLines;
    }
}

void __cdecl CG_RegisterScoreboardDvars()
{
    DvarLimits b; // [esp+8h] [ebp-10h]
    DvarLimits ba; // [esp+8h] [ebp-10h]
    DvarLimits bb; // [esp+8h] [ebp-10h]
    DvarLimits bc; // [esp+8h] [ebp-10h]
    DvarLimits bd; // [esp+8h] [ebp-10h]
    DvarLimits be; // [esp+8h] [ebp-10h]
    DvarLimits bf; // [esp+8h] [ebp-10h]

    cg_ScoresPing_MaxBars = Dvar_RegisterInt(
        "cg_ScoresPing_MaxBars",
        4,
        (DvarLimits)0xA00000001LL,
        DVAR_ARCHIVE,
        "Number of bars to show in ping graph");
    cg_ScoresPing_Interval = Dvar_RegisterInt(
        "cg_ScoresPing_Interval",
        100,
        (DvarLimits)0x1F400000001LL,
        DVAR_ARCHIVE,
        "Number of milliseconds each bar represents");
    cg_ScoresPing_HighColor = Dvar_RegisterColor(
        "cg_ScoresPing_HighColor",
        0.80000001f,
        0.0f,
        0.0f,
        1.0f,
        DVAR_ARCHIVE,
        "Color for high ping");
    cg_ScoresPing_MedColor = Dvar_RegisterColor(
        "cg_ScoresPing_MedColor",
        0.80000001f,
        0.80000001f,
        0.0f,
        1.0f,
        DVAR_ARCHIVE,
        "Color for medium ping");
    cg_ScoresPing_LowColor = Dvar_RegisterColor("cg_ScoresPing_LowColor", 0.0, 0.75, 0.0, 1.0, DVAR_ARCHIVE, "Color for low ping");
    cg_ScoresPing_BgColor = Dvar_RegisterColor(
        "cg_ScoresPing_BgColor",
        0.25f,
        0.25f,
        0.25f,
        0.5f,
        DVAR_ARCHIVE,
        "Background color of ping");
    cg_scoreboardScrollStep = Dvar_RegisterInt(
        "cg_scoreboardScrollStep",
        3,
        (DvarLimits)0x800000001LL,
        DVAR_NOFLAG,
        "Scroll step amount for the scoreboard");
    cg_scoreboardBannerHeight = Dvar_RegisterInt(
        "cg_scoreboardBannerHeight",
        35,
        (DvarLimits)0x6400000001LL,
        DVAR_NOFLAG,
        "Banner height of the scoreboard");
    cg_scoreboardItemHeight = Dvar_RegisterInt(
        "cg_scoreboardItemHeight",
        18,
        (DvarLimits)0x3E800000001LL,
        DVAR_NOFLAG,
        "Item height of each item");
    b.value.max = 1.0f;
    b.value.min = 0.0f;
    cg_scoreboardPingWidth = Dvar_RegisterFloat(
        "cg_scoreboardPingWidth",
        0.035999998f,
        b,
        DVAR_NOFLAG,
        "Width of the ping graph as a % of the scoreboard");
    ba.value.max = 1.0f;
    ba.value.min = 0.0f;
    cg_scoreboardPingHeight = Dvar_RegisterFloat(
        "cg_scoreboardPingHeight",
        0.69999999f,
        ba,
        DVAR_NOFLAG,
        "Height of the ping graph as a % of the scoreboard row height");
    bb.value.max = FLT_MAX;
    bb.value.min = 0.0f;
    cg_scoreboardWidth = Dvar_RegisterFloat("cg_scoreboardWidth", 500.0f, bb, DVAR_NOFLAG, "Width of the scoreboard");
    bc.value.max = FLT_MAX;
    bc.value.min = 0.0f;
    cg_scoreboardHeight = Dvar_RegisterFloat("cg_scoreboardHeight", 435.0f, bc, DVAR_NOFLAG, "Height of the scoreboard");
    cg_scoreboardMyColor = Dvar_RegisterColor(
        "cg_scoreboardMyColor",
        1.0f,
        0.80000001f,
        0.40000001f,
        1.0f,
        DVAR_NOFLAG,
        "The local player's font color when shown in scoreboard");
    bd.value.max = FLT_MAX;
    bd.value.min = 0.0f;
    cg_scoreboardTextOffset = Dvar_RegisterFloat("cg_scoreboardTextOffset", 0.5f, bd, DVAR_NOFLAG, "Scoreboard text offset");
    cg_scoreboardFont = Dvar_RegisterInt(
        "cg_scoreboardFont",
        0,
        (DvarLimits)0x600000000LL,
        DVAR_NOFLAG,
        "Scoreboard font enum ( see menudefinition.h )");
    be.value.max = FLT_MAX;
    be.value.min = 0.0f;
    cg_scoreboardHeaderFontScale = Dvar_RegisterFloat(
        "cg_scoreboardHeaderFontScale",
        0.34999999f,
        be,
        DVAR_NOFLAG,
        "Scoreboard header font scale");
    cg_scoreboardPingText = Dvar_RegisterBool("cg_scoreboardPingText", 1, DVAR_NOFLAG, "Whether to show numeric ping value");
    cg_scoreboardPingGraph = Dvar_RegisterBool("cg_scoreboardPingGraph", 0, DVAR_NOFLAG, "Whether to show graphical ping");
    bf.value.max = FLT_MAX;
    bf.value.min = 0.0f;
    cg_scoreboardRankFontScale = Dvar_RegisterFloat("cg_scoreboardRankFontScale", 0.25f, bf, DVAR_NOFLAG, "Scale of rank font");
}

void __cdecl CG_RegisterScoreboardGraphics()
{
    Material_RegisterHandle("white", 7);
    Material_RegisterHandle("white", 7);
    Material_RegisterHandle("black", 7);
    Material_RegisterHandle("white", 7);
    Material_RegisterHandle("white", 7);
    Material_RegisterHandle("black", 7);
    Material_RegisterHandle("hudscoreboardscroll_uparrow", 7);
    Material_RegisterHandle("hudscoreboardscroll_upkey", 7);
    Material_RegisterHandle("hudscoreboardscroll_downarrow", 7);
    Material_RegisterHandle("hudscoreboardscroll_downkey", 7);
    Material_RegisterHandle("voice_on", 7);
    Material_RegisterHandle("voice_off", 7);
}

bool __cdecl Scoreboard_HandleInput(int localClientNum, int key)
{
    bool result; // al

    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    
    switch (key)
    {
    case 163:
    case 190:
    case 205:
        CG_ScrollScoreboardDown(cgameGlob);
        result = 1;
        break;
    case 164:
    case 184:
    case 206:
        CG_ScrollScoreboardUp(cgameGlob);
        result = 1;
        break;
    default:
        result = 0;
        break;
    }
    return result;
}

