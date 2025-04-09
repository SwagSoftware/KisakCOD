#include "ui_mp.h"
#include <ui/ui.h>
#include <qcommon/cmd.h>
#include <qcommon/mem_track.h>
#include <client_mp/client_mp.h>
#include <gfx_d3d/r_rendercmds.h>
#include <universal/q_parse.h>
#include <client/client.h>
#include <buildnumber_mp.h>
#include <stringed/stringed_hooks.h>
#include <database/database.h>
#include <universal/com_files.h>
#include <universal/com_sndalias.h>

#include <gfx_d3d/r_dvars.h>
#include <win32/win_local.h>
#include <qcommon/com_playerprofile.h>

enum sscType_t : __int32
{                                       // ...
    SSC_STRING = 0x0,
    SSC_YESNO = 0x1,
    SSC_GAMETYPE = 0x2,
    SSC_MAPNAME = 0x3,
};

struct serverFilter_s // sizeof=0x8
{                                       // ...
    const char *description;            // ...
    const char *basedir;                // ...
};

struct serverStatusDvar_t // sizeof=0xC
{
    const char *name;
    const char *altName;
    sscType_t type;
};


int tleEstimates[80] =
{
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60,
  60
}; // idb

serverStatusDvar_t serverStatusDvars[26] =
{
  { "sv_hostname", "@EXE_SV_INFO_SERVERNAME", SSC_STRING },
  { "address", "@EXE_SV_INFO_ADDRESS", SSC_STRING },
  { "pswrd", "@EXE_SV_INFO_PASSWORD", SSC_YESNO },
  { "gamename", "@EXE_SV_INFO_GAMENAME", SSC_STRING },
  { "g_gametype", "@EXE_SV_INFO_GAMETYPE", SSC_GAMETYPE },
  { "sv_pure", "@EXE_SV_INFO_PURE", SSC_YESNO },
  { "mapname", "@EXE_SV_INFO_MAP", SSC_MAPNAME },
  { "shortversion", "@EXE_SV_INFO_VERSION", SSC_STRING },
  { "protocol", "@EXE_SV_INFO_PROTOCOL", SSC_STRING },
  { "sv_maxping", "@EXE_SV_INFO_MAXPING", SSC_STRING },
  { "sv_minping", "@EXE_SV_INFO_MINPING", SSC_STRING },
  { "sv_maxrate", "@EXE_SV_INFO_MAXRATE", SSC_STRING },
  { "sv_floodprotect", "@EXE_SV_INFO_FLOODPROTECT", SSC_YESNO },
  { "sv_allowanonymous", "@EXE_SV_INFO_ALLOWANON", SSC_STRING },
  { "sv_maxclients", "@EXE_SV_INFO_MAXCLIENTS", SSC_STRING },
  { "sv_privateclients", "@EXE_SV_INFO_PRIVATECLIENTS", SSC_STRING },
  { "scr_friendlyFire", "@EXE_SV_INFO_FRIENDLY_FIRE", SSC_STRING },
  { "fs_game", "@EXE_SV_INFO_MOD", SSC_STRING },
  { "mod", "@MENU_MODS", SSC_YESNO },
  { "scr_killcam", "@EXE_SV_INFO_KILLCAM", SSC_YESNO },
  { "g_antilag", "@EXE_SV_INFO_ANTILAG", SSC_YESNO },
  { "g_compassShowEnemies", "@EXE_SV_INFO_COMPASS_ENEMIES", SSC_YESNO },
  { "sv_voice", "@EXE_SV_INFO_VOICE", SSC_YESNO },
  { "sv_punkbuster", "@MPUI_PUNKBUSTER", SSC_YESNO },
  { "sv_disableClientConsole", "@EXE_SV_INFO_CLIENT_CONSOLE", SSC_YESNO },
  { NULL, NULL, SSC_STRING }
}; // idb

const char *netSources[3] = { "EXE_LOCAL", "EXE_INTERNET", "EXE_FAVORITES" }; // idb
const serverFilter_s serverFilters[1] = { { "EXE_ALL", "" }}; // idb

const dvar_t *ui_showList;
const dvar_t *ui_customClassName;
const dvar_t *ui_mapname;
const dvar_t *ui_netSource;
const dvar_t *ui_showMenuOnly;
const dvar_t *ui_bigFont;
const dvar_t *ui_cinematicsTimestamp;
const dvar_t *ui_connectScreenTextGlowColor;
const dvar_t *ui_selectedPlayer;
const dvar_t *ui_extraBigFont;
const dvar_t *ui_drawCrosshair;
const dvar_t *ui_buildSize;
const dvar_t *ui_allow_classchange;
const dvar_t *ui_hud_hardcore;
const dvar_t *ui_gametype;
const dvar_t *uiscript_debug;
const dvar_t *ui_currentMap;
const dvar_t *vehDriverViewHeightMin;
const dvar_t *ui_playerPartyColor;
const dvar_t *ui_allow_teamchange;
const dvar_t *ui_buildLocation;
const dvar_t *ui_smallFont;
const dvar_t *ui_customModeEditName;
const dvar_t *ui_uav_axis;
const dvar_t *ui_serverStatusTimeOut;
const dvar_t *vehDriverViewHeightMax;
const dvar_t *ui_uav_allies;
const dvar_t *ui_uav_client;
const dvar_t *ui_borderLowLightScale;
const dvar_t *ui_partyFull;
const dvar_t *ui_customModeName;

const dvar_t *ui_joinGameType;
const dvar_t *ui_netGameTypeName;
const dvar_t *ui_dedicated;
const dvar_t *ui_currentNetMap;
const dvar_t *ui_browserShowFull;
const dvar_t *ui_browserShowEmpty;
const dvar_t *ui_browserShowPassword;
const dvar_t *ui_browserShowPure;
const dvar_t *ui_browserMod;
const dvar_t *ui_browserShowDedicated;
const dvar_t *ui_browserFriendlyfire;

const dvar_t *ui_browserKillcam;
const dvar_t *ui_browserShowPunkBuster;
const dvar_t *ui_playerProfileCount;
const dvar_t *ui_playerProfileSelected;
const dvar_t *ui_playerProfileNameNew;
const dvar_t *ui_showEndOfGame;
const dvar_t *ui_playerProfileNameNew;
const dvar_t *ui_playerProfileNameNew;
const dvar_t *ui_playerProfileNameNew;

const dvar_t *ui_netGameType;

LegacyHacks legacyHacks;

char g_mapname[64];
char g_gametype[64];
int ui_serverFilterType;
bool g_ingameMenusLoaded[1];

uiInfo_s uiInfoArray; // On PC this array is just [1].
sharedUiInfo_t sharedUiInfo;

const char *MonthAbbrev[12] =
{
  "EXE_MONTH_ABV_JANUARY",
  "EXE_MONTH_ABV_FEBRUARY",
  "EXE_MONTH_ABV_MARCH",
  "EXE_MONTH_ABV_APRIL",
  "EXE_MONTH_ABV_MAY",
  "EXE_MONTH_ABV_JUN",
  "EXE_MONTH_ABV_JULY",
  "EXE_MONTH_ABV_AUGUST",
  "EXE_MONTH_ABV_SEPTEMBER",
  "EXE_MONTH_ABV_OCTOBER",
  "EXE_MONTH_ABV_NOVEMBER",
  "EXE_MONTH_ABV_DECEMBER"
}; // idb

char menuBuf2[32768];

UILocalVarContext *__cdecl UI_GetLocalVarsContext(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return &uiInfoArray.uiDC.localVars;
}

void __cdecl TRACK_ui_main()
{
    track_static_alloc_internal(&sharedUiInfo, 116144, "sharedUiInfo", 34);
    track_static_alloc_internal(&uiInfoArray, 9392, "uiInfoArray", 34);
    track_static_alloc_internal(MonthAbbrev, 48, "MonthAbbrev", 34);
    track_static_alloc_internal(menuBuf2, 0x8000, "menuBuf2", 34);
}

void __cdecl UI_DrawSides(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float w,
    float h,
    int horzAlign,
    int vertAlign,
    float size,
    const float *color)
{
    float v9; // [esp+30h] [ebp-4h]

    CL_DrawStretchPic(
        scrPlace,
        x,
        y,
        size,
        h,
        horzAlign,
        vertAlign,
        0.0,
        0.0,
        0.0,
        0.0,
        color,
        sharedUiInfo.assets.whiteMaterial);
    v9 = x + w - size;
    CL_DrawStretchPic(
        scrPlace,
        v9,
        y,
        size,
        h,
        horzAlign,
        vertAlign,
        0.0,
        0.0,
        0.0,
        0.0,
        color,
        sharedUiInfo.assets.whiteMaterial);
}

void __cdecl UI_DrawTopBottom(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float w,
    float h,
    int horzAlign,
    int vertAlign,
    float size,
    const float *color)
{
    float v9; // [esp+30h] [ebp-4h]

    CL_DrawStretchPic(
        scrPlace,
        x,
        y,
        w,
        size,
        horzAlign,
        vertAlign,
        0.0,
        0.0,
        0.0,
        0.0,
        color,
        sharedUiInfo.assets.whiteMaterial);
    v9 = y + h - size;
    CL_DrawStretchPic(
        scrPlace,
        x,
        v9,
        w,
        size,
        horzAlign,
        vertAlign,
        0.0,
        0.0,
        0.0,
        0.0,
        color,
        sharedUiInfo.assets.whiteMaterial);
}

void __cdecl UI_DrawRect(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float width,
    float height,
    int horzAlign,
    int vertAlign,
    float size,
    const float *color)
{
    float v9; // [esp+20h] [ebp-8h]
    float h; // [esp+24h] [ebp-4h]

    UI_DrawTopBottom(scrPlace, x, y, width, height, horzAlign, vertAlign, size, color);
    h = height - (size + size);
    v9 = y + size;
    UI_DrawSides(scrPlace, x, v9, width, h, horzAlign, vertAlign, size, color);
}

void __cdecl UI_DrawHighlightRect(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float w,
    float h,
    int horzAlign,
    int vertAlign,
    float size,
    const float *hiColor,
    const float *loColor)
{
    float v10; // [esp+8h] [ebp-D8h]
    float v11; // [esp+10h] [ebp-D0h]
    float v12; // [esp+14h] [ebp-CCh]
    float v13; // [esp+20h] [ebp-C0h]
    float v14; // [esp+24h] [ebp-BCh]
    float v15; // [esp+30h] [ebp-B0h]
    float v16; // [esp+3Ch] [ebp-A4h]
    float v17; // [esp+48h] [ebp-98h]
    float v18; // [esp+4Ch] [ebp-94h]
    float v19; // [esp+54h] [ebp-8Ch]
    float v20; // [esp+60h] [ebp-80h]
    float v21; // [esp+6Ch] [ebp-74h]
    float v22; // [esp+74h] [ebp-6Ch]
    float v23; // [esp+78h] [ebp-68h]
    float v24; // [esp+88h] [ebp-58h]
    float v25; // [esp+94h] [ebp-4Ch]
    float v26; // [esp+98h] [ebp-48h]
    float v27; // [esp+A0h] [ebp-40h]
    float v28; // [esp+B0h] [ebp-30h] BYREF
    float dummyY; // [esp+B4h] [ebp-2Ch] BYREF
    float verts[4][2]; // [esp+B8h] [ebp-28h] BYREF
    float dummyX; // [esp+D8h] [ebp-8h] BYREF
    float dy; // [esp+DCh] [ebp-4h] BYREF

    ScrPlace_ApplyRect(scrPlace, &x, &y, &w, &h, horzAlign, vertAlign);
    dummyX = 0.0;
    dummyY = 0.0;
    v28 = size;
    dy = size;
    ScrPlace_ApplyRect(scrPlace, &dummyX, &dummyY, &v28, &dy, horzAlign, vertAlign);
    verts[0][0] = x;
    verts[0][1] = y;
    v27 = w + x;
    verts[1][0] = v27;
    verts[1][1] = y;
    v25 = w + x - v28;
    v26 = dy + y;
    verts[2][0] = v25;
    verts[2][1] = v26;
    v24 = v28 + x;
    verts[3][0] = v24;
    verts[3][1] = v26;
    R_AddCmdDrawQuadPic(verts, hiColor, sharedUiInfo.assets.whiteMaterial);
    verts[0][0] = x;
    verts[0][1] = y;
    v22 = v28 + x;
    v23 = dy + y;
    verts[1][0] = v22;
    verts[1][1] = v23;
    v21 = h + y - dy;
    verts[2][0] = v22;
    verts[2][1] = v21;
    v20 = h + y;
    verts[3][0] = x;
    verts[3][1] = v20;
    R_AddCmdDrawQuadPic(verts, hiColor, sharedUiInfo.assets.whiteMaterial);
    v19 = h + y;
    verts[0][0] = x;
    verts[0][1] = v19;
    v17 = v28 + x;
    v18 = h + y - dy;
    verts[1][0] = v17;
    verts[1][1] = v18;
    v16 = w + x - v28;
    verts[2][0] = v16;
    verts[2][1] = v18;
    v15 = w + x;
    verts[3][0] = v15;
    verts[3][1] = v19;
    R_AddCmdDrawQuadPic(verts, loColor, sharedUiInfo.assets.whiteMaterial);
    v14 = w + x;
    verts[0][0] = v14;
    verts[0][1] = y;
    v13 = h + y;
    verts[1][0] = v14;
    verts[1][1] = v13;
    v11 = w + x - v28;
    v12 = h + y - dy;
    verts[2][0] = v11;
    verts[2][1] = v12;
    v10 = dy + y;
    verts[3][0] = v11;
    verts[3][1] = v10;
    R_AddCmdDrawQuadPic(verts, loColor, sharedUiInfo.assets.whiteMaterial);
}

int __cdecl UI_TextWidth(const char *text, int maxChars, Font_s *font, float scale)
{
    float actualScale; // [esp+8h] [ebp-4h]

    actualScale = R_NormalizedTextScale(font, scale);
    return (int)((double)R_TextWidth(text, maxChars, font) * actualScale);
}

int __cdecl UI_TextHeight(Font_s *font, float scale)
{
    float actualScale; // [esp+8h] [ebp-4h]

    actualScale = R_NormalizedTextScale(font, scale);
    return (int)((double)R_TextHeight(font) * actualScale);
}

void __cdecl UI_DrawText(
    const ScreenPlacement *scrPlace,
    const char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    int horzAlign,
    int vertAlign,
    float scale,
    const float *color,
    int style)
{
    float v11; // [esp+18h] [ebp-18h]
    float v12; // [esp+1Ch] [ebp-14h]
    float v13; // [esp+20h] [ebp-10h]
    float v14; // [esp+24h] [ebp-Ch]
    float xScale; // [esp+28h] [ebp-8h] BYREF
    float yScale; // [esp+2Ch] [ebp-4h] BYREF

    xScale = R_NormalizedTextScale(font, scale);
    yScale = xScale;
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    v14 = x + 0.5;
    v12 = floor(v14);
    x = v12;
    v13 = y + 0.5;
    v11 = floor(v13);
    y = v11;
    CL_DrawTextPhysical(text, maxChars, font, x, v11, xScale, yScale, color, style);
}

void __cdecl UI_DrawTextWithGlow(
    const ScreenPlacement *scrPlace,
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    int horzAlign,
    int vertAlign,
    float scale,
    const float *color,
    int style,
    const float *glowColor,
    bool subtitle,
    bool cinematic)
{
    float v14; // [esp+34h] [ebp-18h]
    float v15; // [esp+38h] [ebp-14h]
    float v16; // [esp+3Ch] [ebp-10h]
    float v17; // [esp+40h] [ebp-Ch]
    float xScale; // [esp+44h] [ebp-8h] BYREF
    float yScale; // [esp+48h] [ebp-4h] BYREF

    xScale = R_NormalizedTextScale(font, scale);
    yScale = xScale;
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    v17 = x + 0.5;
    v15 = floor(v17);
    x = v15;
    v16 = y + 0.5;
    v14 = floor(v16);
    y = v14;
    if (subtitle)
        R_AddCmdDrawTextSubtitle(text, maxChars, font, x, y, xScale, yScale, 0.0, color, style, glowColor, cinematic);
    else
        CL_DrawTextPhysicalWithEffects(
            text,
            maxChars,
            font,
            x,
            y,
            xScale,
            yScale,
            color,
            style,
            glowColor,
            0,
            0,
            0,
            0,
            0,
            0);
}

void __cdecl UI_DrawTextNoSnap(
    const ScreenPlacement *scrPlace,
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    int horzAlign,
    int vertAlign,
    float scale,
    const float *color,
    int style)
{
    float xScale; // [esp+18h] [ebp-8h] BYREF
    float yScale; // [esp+1Ch] [ebp-4h] BYREF

    xScale = R_NormalizedTextScale(font, scale);
    yScale = xScale;
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    CL_DrawTextPhysical(text, maxChars, font, x, y, xScale, yScale, color, style);
}

void __cdecl UI_DrawTextWithCursor(
    const ScreenPlacement *scrPlace,
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    int horzAlign,
    int vertAlign,
    float scale,
    const float *color,
    int style,
    int cursorPos,
    char cursor)
{
    float v13; // [esp+20h] [ebp-18h]
    float v14; // [esp+24h] [ebp-14h]
    float v15; // [esp+28h] [ebp-10h]
    float v16; // [esp+2Ch] [ebp-Ch]
    float xScale; // [esp+30h] [ebp-8h] BYREF
    float yScale; // [esp+34h] [ebp-4h] BYREF

    xScale = R_NormalizedTextScale(font, scale);
    yScale = xScale;
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    v16 = x + 0.5;
    v14 = floor(v16);
    x = v14;
    v15 = y + 0.5;
    v13 = floor(v15);
    y = v13;
    CL_DrawTextPhysicalWithCursor(text, maxChars, font, x, v13, xScale, yScale, color, style, cursorPos, cursor);
}

Font_s *__cdecl UI_GetFontHandle(const ScreenPlacement *scrPlace, int fontEnum, float scale)
{
    float scalea; // [esp+10h] [ebp+10h]

    switch (fontEnum)
    {
    case 2:
        return sharedUiInfo.assets.bigFont;
    case 3:
        return sharedUiInfo.assets.smallFont;
    case 5:
        return sharedUiInfo.assets.consoleFont;
    case 6:
        return sharedUiInfo.assets.objectiveFont;
    }
    scalea = scrPlace->scaleVirtualToReal[1] * scale;
    if (fontEnum == 4)
    {
        if (ui_smallFont->current.value < (double)scalea)
        {
            if (ui_bigFont->current.value > (double)scalea)
                return sharedUiInfo.assets.textFont;
            else
                return sharedUiInfo.assets.boldFont;
        }
        else
        {
            return sharedUiInfo.assets.smallFont;
        }
    }
    else if (ui_smallFont->current.value < (double)scalea)
    {
        if (ui_extraBigFont->current.value > (double)scalea)
        {
            if (ui_bigFont->current.value > (double)scalea)
                return sharedUiInfo.assets.textFont;
            else
                return sharedUiInfo.assets.bigFont;
        }
        else
        {
            return sharedUiInfo.assets.extraBigFont;
        }
    }
    else
    {
        return sharedUiInfo.assets.smallFont;
    }
}

void __cdecl UI_UpdateTime(int localClientNum, int realtime)
{
    int frameTimeIndex; // [esp+4h] [ebp-Ch]
    int frameTimeTotal; // [esp+Ch] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    uiInfoArray.uiDC.frameTime = realtime - uiInfoArray.uiDC.realTime;
    uiInfoArray.uiDC.realTime = realtime;
    uiInfoArray.previousTimes[uiInfoArray.timeIndex++ % 4] = uiInfoArray.uiDC.frameTime;
    if (uiInfoArray.timeIndex > 4)
    {
        frameTimeTotal = 0;
        for (frameTimeIndex = 0; frameTimeIndex < 4; ++frameTimeIndex)
            frameTimeTotal += uiInfoArray.previousTimes[frameTimeIndex];
        if (!frameTimeTotal)
            frameTimeTotal = 1;
        uiInfoArray.uiDC.FPS = (float)(4000 / frameTimeTotal);
    }
}

void __cdecl UI_DrawBuildNumber(int localClientNum)
{
    int BuildNumberAsInt; // eax
    char *v2; // eax
    Font_s *scale; // [esp+0h] [ebp-20h]
    float x; // [esp+4h] [ebp-1Ch]
    float y; // [esp+8h] [ebp-18h]
    float value; // [esp+14h] [ebp-Ch]

    value = ui_buildSize->current.value;
    y = ui_buildLocation->current.vector[1];
    x = ui_buildLocation->current.value;
    scale = UI_GetFontHandle(&scrPlaceView[localClientNum], 0, value);
    BuildNumberAsInt = getBuildNumberAsInt();
    v2 = va("%s.%i", "1.0", BuildNumberAsInt);
    UI_DrawText(&scrPlaceView[localClientNum], v2, 64, scale, x, y, 3, 0, value, colorMdGrey, 0);
}

int __cdecl LAN_GetServerStatus(char *serverAddress, char *serverStatus, int maxLen)
{
    return CL_ServerStatus(serverAddress, serverStatus, maxLen);
}

void __cdecl UI_SortServerStatusInfo(serverStatusInfo_t *info)
{
    const char *v1; // eax
    int j; // [esp+18h] [ebp-14h]
    const char *tmp1; // [esp+1Ch] [ebp-10h]
    const char *tmp2; // [esp+20h] [ebp-Ch]
    int index; // [esp+24h] [ebp-8h]
    int i; // [esp+28h] [ebp-4h]

    index = 0;
    for (i = 0; serverStatusDvars[i].name; ++i)
    {
        for (j = 0; j < info->numLines; ++j)
        {
            if (info->lines[j][1] && !*info->lines[j][1] && !I_stricmp(serverStatusDvars[i].name, info->lines[j][0]))
            {
                tmp1 = info->lines[index][0];
                tmp2 = info->lines[index][3];
                info->lines[index][0] = info->lines[j][0];
                info->lines[index][3] = info->lines[j][3];
                info->lines[j][0] = tmp1;
                info->lines[j][3] = tmp2;
                if (strlen(serverStatusDvars[i].altName))
                    info->lines[index][0] = serverStatusDvars[i].altName;
                switch (serverStatusDvars[i].type)
                {
                case SSC_STRING:
                    break;
                case SSC_YESNO:
                    if (atoi(info->lines[index][3]))
                        info->lines[index][3] = "@EXE_YES";
                    else
                        info->lines[index][3] = "@EXE_NO";
                    break;
                case SSC_GAMETYPE:
                    info->lines[index][3] = UI_GetGameTypeDisplayName(info->lines[index][3]);
                    break;
                case SSC_MAPNAME:
                    info->lines[index][3] = UI_GetMapDisplayName(info->lines[index][3]);
                    break;
                default:
                    if (!alwaysfails)
                    {
                        v1 = va("unknown server status dvar type: %i", serverStatusDvars[i].type);
                        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 3684, 0, v1);
                    }
                    break;
                }
                ++index;
            }
        }
    }
}

int __cdecl UI_GetServerStatusInfo(char *serverAddress, serverStatusInfo_t *info)
{
    char *v3; // eax
    char *v4; // eax
    _BYTE *v5; // eax
    _BYTE *v6; // eax
    _BYTE *v7; // eax
    char *ping; // [esp+10h] [ebp-18h]
    int len; // [esp+18h] [ebp-10h]
    int i; // [esp+1Ch] [ebp-Ch]
    char *p; // [esp+20h] [ebp-8h]
    char *pa; // [esp+20h] [ebp-8h]
    char *score; // [esp+24h] [ebp-4h]

    if (info)
    {
        memset((unsigned __int8 *)info, 0, sizeof(serverStatusInfo_t));
        if (LAN_GetServerStatus(serverAddress, info->text, 1024))
        {
            I_strncpyz(info->address, serverAddress, 64);
            p = info->text;
            info->numLines = 0;
            info->lines[info->numLines][0] = "address";
            info->lines[info->numLines][1] = "";
            info->lines[info->numLines][2] = "";
            info->lines[info->numLines++][3] = (const char *)info;
            do
            {
                if (!p)
                    break;
                if (!*p)
                    break;
                strchr(p, 0x5Cu);
                p = v3;
                if (!v3)
                    break;
                *v3 = 0;
                p = v3 + 1;
                if (v3[1] == 92)
                    break;
                info->lines[info->numLines][0] = p;
                info->lines[info->numLines][1] = "";
                info->lines[info->numLines][2] = "";
                strchr(p, 0x5Cu);
                p = v4;
                if (!v4)
                    break;
                *v4 = 0;
                p = v4 + 1;
                info->lines[info->numLines++][3] = v4 + 1;
            } while (info->numLines < 128);
            if (info->numLines < 125)
            {
                info->lines[info->numLines][0] = "";
                info->lines[info->numLines][1] = "";
                info->lines[info->numLines][2] = "";
                info->lines[info->numLines++][3] = "";
                info->lines[info->numLines][0] = "@EXE_SV_INFO_NUM";
                info->lines[info->numLines][1] = "@EXE_SV_INFO_SCORE";
                info->lines[info->numLines][2] = "@EXE_SV_INFO_PING";
                info->lines[info->numLines++][3] = "@EXE_SV_INFO_NAME";
                i = 0;
                len = 0;
                while (p && *p)
                {
                    if (*p == 92)
                        *p++ = 0;
                    if (!p)
                        break;
                    score = p;
                    strchr(p, 0x20u);
                    if (!v5)
                        break;
                    *v5 = 0;
                    ping = (char*)(v5 + 1);
                    strchr((char*)(v5 + 1), 0x20u);
                    if (!v6)
                        break;
                    *v6 = 0;
                    pa = (char*)(v6 + 1);
                    Com_sprintf(&info->pings[len], 192 - len, "%d", i);
                    info->lines[info->numLines][0] = &info->pings[len];
                    len += strlen(&info->pings[len]) + 1;
                    info->lines[info->numLines][1] = score;
                    info->lines[info->numLines][2] = ping;
                    info->lines[info->numLines][3] = pa;
                    if (++info->numLines >= 128)
                        break;
                    strchr(pa, 0x5Cu);
                    if (!v7)
                        break;
                    *v7 = 0;
                    p = (char*)(v7 + 1);
                    ++i;
                }
            }
            UI_SortServerStatusInfo(info);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        LAN_GetServerStatus(serverAddress, 0, 0);
        return 0;
    }
}

int numFound;
int numTimeOuts;
uiInfo_s *UI_BuildFindPlayerList()
{
    uiInfo_s *result; // eax
    const char *v1; // eax
    int j; // [esp+4h] [ebp-1144h]
    serverStatusInfo_t info; // [esp+8h] [ebp-1140h] BYREF
    uiInfo_s *uiInfo; // [esp+D14h] [ebp-434h]
    char dest[36]; // [esp+D18h] [ebp-430h] BYREF
    int i; // [esp+D3Ch] [ebp-40Ch]
    char buf[1028]; // [esp+D40h] [ebp-408h] BYREF

    uiInfo = &uiInfoArray;
    result = &uiInfoArray;
    if (uiInfoArray.nextFindPlayerRefresh)
    {
        result = (uiInfo_s *)uiInfo->nextFindPlayerRefresh;
        if ((int)result <= uiInfo->uiDC.realTime)
        {
            UI_UpdateDisplayServers(uiInfo);
            for (i = 0; i < 16; ++i)
            {
                if (sharedUiInfo.pendingServerStatus.server[i].valid
                    && UI_GetServerStatusInfo(sharedUiInfo.pendingServerStatus.server[i].adrstr, &info))
                {
                    ++numFound;
                    for (j = 0; j < info.numLines; ++j)
                    {
                        if (*(_DWORD *)&info.text[16 * j - 2040])
                        {
                            if (**(_BYTE **)&info.text[16 * j - 2040])
                            {
                                I_strncpyz(dest, *(char **)&info.text[16 * j - 2036], 34);
                                I_CleanStr(dest);
                                if (stristr(dest, uiInfo->findPlayerName))
                                {
                                    if (uiInfo->numFoundPlayerServers >= 15)
                                    {
                                        sharedUiInfo.pendingServerStatus.num = *(_DWORD *)&sharedUiInfo.gap8EB4[72900];
                                    }
                                    else
                                    {
                                        I_strncpyz(
                                            &uiInfo->findPlayerName[64 * uiInfo->numFoundPlayerServers + 960],
                                            sharedUiInfo.pendingServerStatus.server[i].adrstr,
                                            64);
                                        I_strncpyz(
                                            uiInfo->foundPlayerServerAddresses[uiInfo->numFoundPlayerServers + 15],
                                            sharedUiInfo.pendingServerStatus.server[i].name,
                                            64);
                                        ++uiInfo->numFoundPlayerServers;
                                    }
                                }
                            }
                        }
                    }
                    Com_sprintf(
                        uiInfo->foundPlayerServerAddresses[uiInfo->numFoundPlayerServers + 15],
                        0x40u,
                        "searching %d/%d...",
                        sharedUiInfo.pendingServerStatus.num,
                        numFound);
                    sharedUiInfo.pendingServerStatus.server[i].valid = 0;
                }
                if (!sharedUiInfo.pendingServerStatus.server[i].valid
                    || sharedUiInfo.pendingServerStatus.server[i].startTime < uiInfo->uiDC.realTime
                    - ui_serverStatusTimeOut->current.integer)
                {
                    if (sharedUiInfo.pendingServerStatus.server[i].valid)
                        ++numTimeOuts;
                    UI_GetServerStatusInfo(sharedUiInfo.pendingServerStatus.server[i].adrstr, 0);
                    sharedUiInfo.pendingServerStatus.server[i].valid = 0;
                    UI_UpdateDisplayServers(uiInfo);
                    if (sharedUiInfo.pendingServerStatus.num < *(int *)&sharedUiInfo.gap8EB4[72900])
                    {
                        sharedUiInfo.pendingServerStatus.server[i].startTime = uiInfo->uiDC.realTime;
                        LAN_GetServerAddressString(
                            ui_netSource->current.integer,
                            *(_DWORD *)&sharedUiInfo.gap8EB4[4 * sharedUiInfo.pendingServerStatus.num - 7100],
                            sharedUiInfo.pendingServerStatus.server[i].adrstr,
                            64);
                        LAN_GetServerInfo(
                            ui_netSource->current.integer,
                            *(_DWORD *)&sharedUiInfo.gap8EB4[4 * sharedUiInfo.pendingServerStatus.num - 7100],
                            buf,
                            1024);
                        v1 = Info_ValueForKey(buf, "hostname");
                        I_strncpyz(sharedUiInfo.pendingServerStatus.server[i].name, v1, 64);
                        sharedUiInfo.pendingServerStatus.server[i].valid = 1;
                        Com_sprintf(
                            uiInfo->foundPlayerServerAddresses[uiInfo->numFoundPlayerServers + 15],
                            0x40u,
                            "searching %d/%d...",
                            ++sharedUiInfo.pendingServerStatus.num,
                            numFound);
                    }
                }
            }
            for (i = 0; i < 16 && !sharedUiInfo.pendingServerStatus.server[i].valid; ++i)
                ;
            if (i >= 16)
            {
                if (uiInfo->numFoundPlayerServers)
                {
                    if (uiInfo->numFoundPlayerServers == 2)
                        result = (uiInfo_s *)Com_sprintf(
                            uiInfo->foundPlayerServerAddresses[uiInfo->numFoundPlayerServers + 15],
                            0x40u,
                            "%d server%s found with player %s",
                            uiInfo->numFoundPlayerServers - 1,
                            "",
                            uiInfo->findPlayerName);
                    else
                        result = (uiInfo_s *)Com_sprintf(
                            uiInfo->foundPlayerServerAddresses[uiInfo->numFoundPlayerServers + 15],
                            0x40u,
                            "%d server%s found with player %s",
                            uiInfo->numFoundPlayerServers - 1,
                            "s",
                            uiInfo->findPlayerName);
                }
                else
                {
                    result = (uiInfo_s *)Com_sprintf(
                        uiInfo->foundPlayerServerAddresses[uiInfo->numFoundPlayerServers + 15],
                        0x40u,
                        "no servers found");
                }
                uiInfo->nextFindPlayerRefresh = 0;
            }
            else
            {
                result = uiInfo;
                uiInfo->nextFindPlayerRefresh = uiInfo->uiDC.realTime + 25;
            }
        }
    }
    return result;
}

void __cdecl UI_Refresh(int localClientNum)
{
    float x; // [esp+20h] [ebp-18h]
    float y; // [esp+24h] [ebp-14h]
    float h; // [esp+30h] [ebp-8h]
    float w; // [esp+34h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (Menu_Count(&uiInfoArray.uiDC) > 0)
    {
        Menu_PaintAll(&uiInfoArray.uiDC);
        UI_DoServerRefresh(&uiInfoArray);
        UI_BuildServerStatus(&uiInfoArray, 0);
        UI_BuildFindPlayerList();
        if (CL_AllLocalClientsDisconnected())
        {
            if (localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\ui_mp\\../client_mp/client_mp.h",
                    1112,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum);
            if (clientUIActives[0].connectionState == CA_DISCONNECTED)
                UI_DrawBuildNumber(localClientNum);
        }
        if (uiInfoArray.uiDC.isCursorVisible)
        {
            if (!Dvar_GetBool("cl_bypassMouseInput") && UI_GetActiveMenu(localClientNum) != UIMENU_SCOREBOARD)
            {
                w = scrPlaceFull.scaleVirtualToReal[0] * 32.0 / scrPlaceFull.scaleVirtualToFull[0];
                h = scrPlaceFull.scaleVirtualToReal[1] * 32.0 / scrPlaceFull.scaleVirtualToFull[1];
                y = uiInfoArray.uiDC.cursor.y - h * 0.5;
                x = uiInfoArray.uiDC.cursor.x - w * 0.5;
                UI_DrawHandlePic(&scrPlaceView[localClientNum], x, y, w, h, 4, 4, 0, sharedUiInfo.assets.cursor);
            }
        }
    }
    if (!Menu_GetFocused(&uiInfoArray.uiDC))
    {
        if (Key_IsCatcherActive(localClientNum, 16))
            Key_RemoveCatcher(localClientNum, -17);
    }
}

void __cdecl LAN_SaveServersToCache()
{
    int version; // [esp+8h] [ebp-10h] BYREF
    int size; // [esp+Ch] [ebp-Ch] BYREF
    int fileOut; // [esp+10h] [ebp-8h]
    int i; // [esp+14h] [ebp-4h]

    fileOut = FS_SV_FOpenFileWrite("servercache.dat");
    if (fileOut)
    {
        version = 1;
        FS_Write((char *)&version, 4u, fileOut);
        for (i = cls.numglobalservers - 1; i >= 0; --i)
        {
            if (cls.globalServers[i].requestCount >= 3u)
                goto LABEL_8;
            if (!i)
                break;
            if (!NET_CompareAdrSigned(&cls.globalServers[i].adr, (netadr_t *)&cls.localServers[i + 127].adr.port))
                LABEL_8:
            memcpy(&cls.globalServers[i], &cls.globalServers[--cls.numglobalservers], sizeof(cls.globalServers[i]));
        }
        CL_SortGlobalServers();
        FS_Write((char *)&cls.numglobalservers, 4u, fileOut);
        FS_Write((char *)&cls.numfavoriteservers, 4u, fileOut);
        size = 2978944;
        FS_Write((char *)&size, 4u, fileOut);
        FS_Write((char *)cls.globalServers, 0x2D2A80u, fileOut);
        FS_Write((char *)cls.favoriteServers, 0x4A00u, fileOut);
        FS_FCloseFile(fileOut);
    }
}

void __cdecl UI_Shutdown(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    Menus_CloseAll(&uiInfoArray.uiDC);
    sharedUiInfo.assets.whiteMaterial = 0;
    UILocalVar_Shutdown(&uiInfoArray.uiDC.localVars);
    if (!useFastFile->current.enabled)
        Menus_FreeAllMemory(&uiInfoArray.uiDC);
    LAN_SaveServersToCache();
}

char *__cdecl GetMenuBuffer_LoadObj(char *filename)
{
    int len; // [esp+0h] [ebp-8h]
    int f; // [esp+4h] [ebp-4h] BYREF

    len = FS_FOpenFileByMode(filename, &f, FS_READ);
    if (f)
    {
        if (len < 0x8000)
        {
            FS_Read((unsigned __int8 *)menuBuf2, len, f);
            menuBuf2[len] = 0;
            FS_FCloseFile(f);
            return menuBuf2;
        }
        else
        {
            Com_PrintError(13, "menu file too large: %s is %i, max allowed is %i", filename, len, 0x8000);
            FS_FCloseFile(f);
            return 0;
        }
    }
    else
    {
        Com_PrintError(13, "menu file not found: %s, using default\n", filename);
        return 0;
    }
}

char *__cdecl GetMenuBuffer(char *filename)
{
    if (useFastFile->current.enabled)
        return (char *)GetMenuBuffer_FastFile(filename);
    else
        return GetMenuBuffer_LoadObj(filename);
}

XModelPiece *__cdecl GetMenuBuffer_FastFile(const char *filename)
{
    RawFile *rawfile; // [esp+4h] [ebp-4h]

    rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, filename).rawfile;
    if (rawfile)
        return (XModelPiece *)rawfile->buffer;
    Com_PrintError(13, "menu file not found: %s, using default\n", filename);
    return 0;
}

int __cdecl Load_ScriptMenu(int localClientNum, char *pszMenu, int imageTrack)
{
    MenuList *menuList; // [esp+4h] [ebp-4h]

    menuList = Load_ScriptMenuInternal(pszMenu, imageTrack);
    if (!menuList)
        return 0;
    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    UI_AddMenuList(&uiInfoArray.uiDC, menuList);
    return 1;
}

MenuList *__cdecl Load_ScriptMenuInternal(char *pszMenu, int imageTrack)
{
    char szMenuFile[260]; // [esp+0h] [ebp-108h] BYREF

    strcpy(szMenuFile, "ui_mp/scriptmenus/");
    I_strncat(szMenuFile, 256, pszMenu);
    I_strncat(szMenuFile, 256, ".menu");
    return UI_LoadMenu(szMenuFile, imageTrack);
}

char *__cdecl UI_GetMapDisplayName(const char *pszMap)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < sharedUiInfo.mapCount; ++i)
    {
        if (!I_stricmp(pszMap, (const char *)sharedUiInfo.serverHardwareIconList[40 * i - 5119]))
            return UI_SafeTranslateString((char *)sharedUiInfo.mapList[i].mapName);
    }
    return (char *)pszMap;
}

char *__cdecl UI_GetMapDisplayNameFromPartialLoadNameMatch(const char *mapName, int *mapLoadNameLen)
{
    int i; // [esp+14h] [ebp-4h]

    if (!mapLoadNameLen)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 1043, 0, "%s", "mapLoadNameLen");
    for (i = 0; i < sharedUiInfo.mapCount; ++i)
    {
        *mapLoadNameLen = strlen((const char *)sharedUiInfo.serverHardwareIconList[40 * i - 5119]);
        if (!I_strnicmp(mapName, (const char *)sharedUiInfo.serverHardwareIconList[40 * i - 5119], *mapLoadNameLen))
            return UI_SafeTranslateString((char *)sharedUiInfo.mapList[i].mapName);
    }
    return 0;
}

char *__cdecl UI_GetGameTypeDisplayName(const char *pszGameType)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < sharedUiInfo.numGameTypes; ++i)
    {
        if (!I_stricmp(pszGameType, sharedUiInfo.gameTypes[i].gameType))
            return UI_SafeTranslateString((char *)sharedUiInfo.gameTypes[i].gameTypeName);
    }
    return (char *)pszGameType;
}

int __cdecl UI_OwnerDrawWidth(int ownerDraw, Font_s *font, float scale)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *s; // [esp+8h] [ebp-4h]

    s = 0;
    switch (ownerDraw)
    {
    case 205:
        s = sharedUiInfo.gameTypes[ui_gametype->current.integer].gameTypeName;
        break;
    case 220:
        if (ui_netSource->current.integer > sharedUiInfo.numJoinGameTypes)
            Dvar_SetInt((dvar_s*)ui_netSource, 0);
        v3 = va("EXE_NETSOURCE_%s", netSources[ui_netSource->current.integer]);
        s = SEH_LocalizeTextMessage(v3, "net source", LOCMSG_SAFE);
        break;
    case 222:
        if ((unsigned int)ui_serverFilterType >= 2)
            ui_serverFilterType = 0;
        v4 = va("EXE_SERVERFILTER_%s", serverFilters[ui_serverFilterType].description);
        s = SEH_LocalizeTextMessage(v4, "server filter", LOCMSG_SAFE);
        break;
    case 247:
        v5 = va("ui_lastServerRefresh_%i", ui_netSource->current.integer);
        s = Dvar_GetVariantString(v5);
        break;
    case 250:
        if (Display_KeyBindPending())
            s = UI_SafeTranslateString("EXE_KEYWAIT");
        else
            s = UI_SafeTranslateString("EXE_KEYCHANGE");
        break;
    default:
        break;
    }
    if (s)
        return UI_TextWidth(s, 0, font, scale);
    else
        return 0;
}

void __cdecl UI_DrawMapLevelshot(int localClientNum)
{
    menuDef_t *menu; // [esp+28h] [ebp-8h]
    menuDef_t *menua; // [esp+28h] [ebp-8h]

    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    menu = Menus_FindByName(&uiInfoArray.uiDC, "pregame_loaderror_mp");
    if (!menu || !Menus_MenuIsInStack(&uiInfoArray.uiDC, menu))
    {
        if (g_mapname[0])
        {
            if (useFastFile->current.enabled)
                menua = DB_FindXAssetHeader(ASSET_TYPE_MENU, "connect").menu;
            else
                menua = Menus_FindByName(&uiInfoArray.uiDC, "connect");
        }
        else
        {
            menua = 0;
        }
        if (menua)
        {
            if (!Menus_MenuIsInStack(&uiInfoArray.uiDC, menua))
                Menus_Open(&uiInfoArray.uiDC, menua);
            uiInfoArray.uiDC.blurRadiusOut = 0.0;
            Window_SetDynamicFlags(localClientNum, &menua->window, 16388);
            Menu_Paint(&uiInfoArray.uiDC, menua);
        }
        else
        {
            UI_FillRect(&scrPlaceView[localClientNum], 0.0, 0.0, 640.0, 480.0, 0, 0, colorBlack);
        }
    }
}

void __cdecl UI_LoadIngameMenus(int localClientNum)
{
    MenuList *menuList; // [esp+4h] [ebp-4h]

    if (!g_ingameMenusLoaded[localClientNum])
    {
        g_ingameMenusLoaded[localClientNum] = 1;
        menuList = UI_LoadMenus((char*)"ui_mp/ingame.txt", 3);
        if (localClientNum)
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                332,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        UI_AddMenuList(&uiInfoArray.uiDC, menuList);
    }
}

void __cdecl UI_ParseMenuMaterial(const char *key, char *value)
{
    Material *material; // [esp+0h] [ebp-4Ch]
    char name[68]; // [esp+4h] [ebp-48h] BYREF

    material = Material_RegisterHandle(value, 3);
    Com_sprintf(name, 0x40u, "$%s", key);
    I_strlwr(name);
    Material_Duplicate(material, name);
}

void __cdecl UI_MapLoadInfo(const char *filename)
{
    const char *parse; // [esp+14h] [ebp-118h] BYREF
    int tokenLen; // [esp+18h] [ebp-114h]
    char key[256]; // [esp+1Ch] [ebp-110h] BYREF
    const char *token; // [esp+120h] [ebp-Ch]
    char *loadfile; // [esp+124h] [ebp-8h] BYREF
    const char *value; // [esp+128h] [ebp-4h]

    if (*filename)
    {
        if (FS_ReadFile(filename, (void **)&loadfile) >= 0)
        {
            parse = loadfile;
            Com_BeginParseSession(filename);
            Com_SetCSV(1);
            while (1)
            {
                token = (const char *)Com_Parse(&parse);
                if (!*token)
                    break;
                tokenLen = strlen(token) + 1;
                if ((unsigned int)tokenLen >= 0x100)
                {
                    Com_EndParseSession();
                    Com_Error(ERR_DROP, "key '%s' is %i > %i characters long", key, tokenLen - 1, 255);
                }
                memcpy((unsigned __int8 *)key, (unsigned __int8 *)token, tokenLen);
                value = (const char *)Com_ParseOnLine(&parse);
                if (!*value)
                {
                    Com_EndParseSession();
                    Com_Error(ERR_DROP, "key '%s' missing value in '%s'\n", key, filename);
                    break;
                }
                UI_ParseMenuMaterial(key, (char *)value);
            }
            Com_EndParseSession();
            FS_FreeFile(loadfile);
        }
        else
        {
            Com_PrintWarning(13, "WARNING: Could not find '%s'.\n", filename);
        }
    }
}

void __cdecl UI_SetMap(char *mapname, char *gametype)
{
    const char *v2; // eax

    I_strncpyz(g_mapname, mapname, 64);
    I_strncpyz(g_gametype, gametype, 64);
    if (!useFastFile->current.enabled)
    {
        if (g_mapname[0])
        {
            v2 = va("maps/mp/%s.csv", g_mapname);
            UI_MapLoadInfo(v2);
        }
    }
}

int __cdecl UI_GetTalkerClientNum(int localClientNum, int num)
{
    int client; // [esp+0h] [ebp-8h]
    int talker; // [esp+4h] [ebp-4h]

    talker = 0;
    for (client = 0; client < 64; ++client)
    {
        if (!CL_IsClientLocal(client) && CL_IsPlayerTalking(localClientNum, client))
        {
            if (talker == num)
                return client;
            ++talker;
        }
    }
    return -1;
}

const float PROFLOAD_FONT_SCALE = 0.36f;
const float PROFLOAD_TEXT_COLOR[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
const float PROFLOAD_BACKGROUND_COLOR[4] = { 0.0f, 0.0f, 0.0f, 0.8f };
long double msecPerRawTimerTick;

void __cdecl ProfLoad_DrawOverlay(rectDef_s *rect)
{
    int fileReadCount; // [esp+7Ch] [ebp-128h]
    Font_s *profileFont; // [esp+88h] [ebp-11Ch]
    int fileOpenCount; // [esp+8Ch] [ebp-118h]
    char line[256]; // [esp+94h] [ebp-110h] BYREF
    float y; // [esp+198h] [ebp-Ch]
    MapProfileElement *fileOpenElement; // [esp+19Ch] [ebp-8h]
    int fileSeekCount; // [esp+1A0h] [ebp-4h]

    if (com_profileLoading->current.enabled)
    {
        y = 395.0;
        profileFont = UI_GetFontHandle(&scrPlaceFull, 0, 0.36000001);
        UI_FillRect(
            &scrPlaceFull,
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            PROFLOAD_BACKGROUND_COLOR);
        ProfLoad_DrawTree();
        fileOpenElement = mapLoadProfile.elements;
        fileOpenCount = mapLoadProfile.elementAccessCount[0];
        fileSeekCount = mapLoadProfile.elementAccessCount[1];
        fileReadCount = mapLoadProfile.elementAccessCount[2];
        sprintf(
            line,
            "Total Load Time: %5.4f",
            (double)((double)(mapLoadProfile.ticksFinish - mapLoadProfile.ticksStart)
                * msecPerRawTimerTick
                * 0.001000000047497451));
        UI_DrawText(
            &scrPlaceFull,
            line,
            256,
            profileFont,
            60.0,
            y,
            rect->horzAlign,
            rect->vertAlign,
            0.36000001,
            PROFLOAD_TEXT_COLOR,
            0);
        sprintf(
            line,
            "File Open Time: %5.4f, Accessed %d times",
            (double)((double)fileOpenElement->ticksTotal * msecPerRawTimerTick * 0.001000000047497451),
            fileOpenCount);
        UI_DrawText(
            &scrPlaceFull,
            line,
            256,
            profileFont,
            250.0,
            y,
            rect->horzAlign,
            rect->vertAlign,
            0.36000001,
            PROFLOAD_TEXT_COLOR,
            0);
        y = (double)15 + y;
        sprintf(
            line,
            "Profiled Time: %5.4f",
            (double)((double)mapLoadProfile.ticksProfiled * msecPerRawTimerTick * 0.001000000047497451));
        UI_DrawText(
            &scrPlaceFull,
            line,
            256,
            profileFont,
            60.0,
            y,
            rect->horzAlign,
            rect->vertAlign,
            0.36000001,
            PROFLOAD_TEXT_COLOR,
            0);
        sprintf(
            line,
            "File Seek Time: %5.4f, Accessed %d times",
            (double)((double)mapLoadProfile.elements[1].ticksTotal * msecPerRawTimerTick * 0.001000000047497451),
            fileSeekCount);
        UI_DrawText(
            &scrPlaceFull,
            line,
            256,
            profileFont,
            250.0,
            y,
            rect->horzAlign,
            rect->vertAlign,
            0.36000001,
            PROFLOAD_TEXT_COLOR,
            0);
        y = (double)15 + y;
        sprintf(
            line,
            "Unprofiled Time: %5.4f",
            (double)((double)(mapLoadProfile.ticksFinish - mapLoadProfile.ticksStart - mapLoadProfile.ticksProfiled)
                * msecPerRawTimerTick
                * 0.001000000047497451));
        UI_DrawText(
            &scrPlaceFull,
            line,
            256,
            profileFont,
            60.0,
            y,
            rect->horzAlign,
            rect->vertAlign,
            0.36000001,
            PROFLOAD_TEXT_COLOR,
            0);
        sprintf(
            line,
            "File Read Time: %5.4f, Accessed %d times",
            (double)((double)mapLoadProfile.elements[2].ticksTotal * msecPerRawTimerTick * 0.001000000047497451),
            fileReadCount);
        UI_DrawText(
            &scrPlaceFull,
            line,
            256,
            profileFont,
            250.0,
            y,
            rect->horzAlign,
            rect->vertAlign,
            0.36000001,
            PROFLOAD_TEXT_COLOR,
            0);
    }
}

char __cdecl UI_DrawRecordLevel(int localClientNum, rectDef_s *rect)
{
    float v3; // [esp+1Ch] [ebp-30h]
    float v4; // [esp+20h] [ebp-2Ch]
    float v5; // [esp+24h] [ebp-28h]
    float v6; // [esp+28h] [ebp-24h]
    float v7; // [esp+2Ch] [ebp-20h]
    float v8; // [esp+30h] [ebp-1Ch]
    float x; // [esp+34h] [ebp-18h]
    float y; // [esp+38h] [ebp-14h]
    float height; // [esp+3Ch] [ebp-10h]
    ScreenPlacement *scrPlace; // [esp+40h] [ebp-Ch]
    float size; // [esp+44h] [ebp-8h]
    float curLevel; // [esp+48h] [ebp-4h]

    scrPlace = &scrPlaceView[localClientNum];
    curLevel = Voice_GetVoiceLevel();
    if (curLevel > 0.0)
    {
        size = rect->w * curLevel - 4.0;
        if (size > 0.0)
        {
            height = rect->h - 4.0;
            y = rect->y + 2.0;
            x = rect->x + 2.0;
            UI_FillRect(scrPlace, x, y, size, height, 0, 0, colorWhite);
        }
    }
    UI_FillRect(scrPlace, rect->x, rect->y, rect->w, 1.0, 0, 0, colorWhite);
    v8 = rect->y + rect->h - 1.0;
    UI_FillRect(scrPlace, rect->x, v8, rect->w, 1.0, 0, 0, colorWhite);
    v7 = rect->h - 2.0;
    v6 = rect->y + 1.0;
    UI_FillRect(scrPlace, rect->x, v6, 1.0, v7, 0, 0, colorWhite);
    v5 = rect->h - 2.0;
    v4 = rect->y + 1.0;
    v3 = rect->x + rect->w - 1.0;
    UI_FillRect(scrPlace, v3, v4, 1.0, v5, 0, 0, colorWhite);
    return 1;
}

void __cdecl UI_DrawGameType(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    int textStyle)
{
    char *v6; // eax

    if (*sharedUiInfo.gameTypes[ui_gametype->current.integer].gameTypeName)
        v6 = UI_SafeTranslateString((char *)sharedUiInfo.gameTypes[ui_gametype->current.integer].gameTypeName);
    else
        v6 = UI_SafeTranslateString("EXE_ALL");
    UI_DrawText(
        &scrPlaceView[localClientNum],
        v6,
        0x7FFFFFFF,
        font,
        rect->x,
        rect->y,
        rect->horzAlign,
        rect->vertAlign,
        scale,
        color,
        textStyle);
}

void __cdecl UI_DrawNetGameType(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    int textStyle)
{
    char *v6; // eax

    if (ui_netGameType->current.integer > sharedUiInfo.numGameTypes)
    {
        Dvar_SetInt((dvar_s *)ui_netGameType, 0);
        Dvar_SetString((dvar_s *)ui_netGameTypeName, (char *)sharedUiInfo.gameTypes[0].gameType);
    }
    if (*sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameTypeName)
        v6 = UI_SafeTranslateString((char *)sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameTypeName);
    else
        v6 = UI_SafeTranslateString("EXE_ALL");
    UI_DrawText(
        &scrPlaceView[localClientNum],
        v6,
        0x7FFFFFFF,
        font,
        rect->x,
        rect->y,
        rect->horzAlign,
        rect->vertAlign,
        scale,
        color,
        textStyle);
}

void __cdecl UI_DrawJoinGameType(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    int textStyle)
{
    char *v6; // eax

    if (ui_joinGameType->current.integer > sharedUiInfo.numJoinGameTypes)
        Dvar_SetInt((dvar_s *)ui_joinGameType, 0);
    if (*sharedUiInfo.joinGameTypes[ui_joinGameType->current.integer].gameTypeName)
        v6 = UI_SafeTranslateString((char *)sharedUiInfo.joinGameTypes[ui_joinGameType->current.integer].gameTypeName);
    else
        v6 = UI_SafeTranslateString("EXE_ALL");
    UI_DrawText(
        &scrPlaceView[localClientNum],
        v6,
        0x7FFFFFFF,
        font,
        rect->x,
        rect->y,
        rect->horzAlign,
        rect->vertAlign,
        scale,
        color,
        textStyle);
}

void __cdecl UI_DrawNetFilter(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    int textStyle)
{
    const char *v6; // eax
    char *pszTeanslation; // [esp+1Ch] [ebp-4h]

    if ((unsigned int)ui_serverFilterType >= 2)
        ui_serverFilterType = 0;
    v6 = va("EXE_SERVERFILTER_%s", serverFilters[ui_serverFilterType].description);
    pszTeanslation = SEH_LocalizeTextMessage(v6, "server filter", LOCMSG_SAFE);
    UI_DrawText(
        &scrPlaceView[localClientNum],
        pszTeanslation,
        0x7FFFFFFF,
        font,
        rect->x,
        rect->y,
        rect->horzAlign,
        rect->vertAlign,
        scale,
        color,
        textStyle);
}

void __cdecl UI_DrawNetSource(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    int textStyle)
{
    const char *v6; // eax
    char *translation; // [esp+1Ch] [ebp-4h]

    v6 = va("EXE_NETSOURCE_%s", netSources[ui_netSource->current.integer]);
    translation = SEH_LocalizeTextMessage(v6, "net source", LOCMSG_SAFE);
    UI_DrawText(
        &scrPlaceView[localClientNum],
        translation,
        0x7FFFFFFF,
        font,
        rect->x,
        rect->y,
        rect->horzAlign,
        rect->vertAlign,
        scale,
        color,
        textStyle);
}

void __cdecl UI_DrawMapPreview(int localClientNum, const rectDef_s *rect, float scale, const float *color)
{
    Material *mtl; // [esp+20h] [ebp-4h]

    mtl = UI_GetLevelShot(ui_currentNetMap->current.integer);
    UI_DrawHandlePic(
        &scrPlaceView[localClientNum],
        rect->x,
        rect->y,
        rect->w,
        rect->h,
        rect->horzAlign,
        rect->vertAlign,
        color,
        mtl);
}


void __cdecl UI_OwnerDraw(
    int localClientNum,
    float x,
    float y,
    float w,
    float h,
    int horzAlign,
    int vertAlign,
    float text_x,
    float text_y,
    int ownerDraw,
    int ownerDrawFlags,
    int align,
    float special,
    Font_s *font,
    float scale,
    float *color,
    Material *material,
    int textStyle,
    rectDef_s parentRect,
    char textAlignMode)
{
    rectDef_s rect; // [esp+4Ch] [ebp-18h] BYREF

    if (CL_IsCgameInitialized(localClientNum))
        CG_OwnerDraw(
            localClientNum,
            parentRect,
            x,
            y,
            w,
            h,
            horzAlign,
            vertAlign,
            text_x,
            text_y,
            ownerDraw,
            ownerDrawFlags,
            align,
            special,
            font,
            scale,
            color,
            material,
            textStyle,
            textAlignMode);
    rect.x = x + text_x;
    rect.y = y + text_y;
    rect.w = w;
    rect.h = h;
    rect.horzAlign = horzAlign;
    rect.vertAlign = vertAlign;
    switch (ownerDraw)
    {
    case 205:
        UI_DrawGameType(localClientNum, &rect, font, scale, color, textStyle);
        break;
    case 220:
        UI_DrawNetSource(localClientNum, &rect, font, scale, color, textStyle);
        break;
    case 222:
        UI_DrawNetFilter(localClientNum, &rect, font, scale, color, textStyle);
        break;
    case 245:
        UI_DrawNetGameType(localClientNum, &rect, font, scale, color, textStyle);
        break;
    case 247:
        UI_DrawServerRefreshDate(localClientNum, &rect, font, scale, color, textStyle);
        break;
    case 250:
        UI_DrawKeyBindStatus(localClientNum, &rect, font, scale, color, textStyle);
        break;
    case 253:
        UI_DrawJoinGameType(localClientNum, &rect, font, scale, color, textStyle);
        break;
    case 254:
        UI_DrawMapPreview(localClientNum, &rect, scale, color);
        break;
    case 264:
        ProfLoad_DrawOverlay(&rect);
        break;
    case 265:
        UI_DrawRecordLevel(localClientNum, &rect);
        break;
    case 266:
        UI_DrawLocalTalking(localClientNum, &rect, color);
        break;
    case 267:
    case 268:
    case 269:
    case 270:
        UI_DrawTalkerNum(localClientNum, ownerDraw - 267, &rect, font, color, scale, textStyle);
        break;
    default:
        return;
    }
}

void __cdecl UI_DrawServerRefreshDate(
    int localClientNum,
    rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    int textStyle)
{
    const char *v6; // eax
    char *VariantString; // eax
    float t; // [esp+1Ch] [ebp-80h]
    float v9; // [esp+20h] [ebp-7Ch]
    float v10; // [esp+28h] [ebp-74h]
    char buff[64]; // [esp+2Ch] [ebp-70h] BYREF
    float lowLight[4]; // [esp+70h] [ebp-2Ch] BYREF
    float newColor[5]; // [esp+80h] [ebp-1Ch] BYREF
    const char *string; // [esp+94h] [ebp-8h]
    int serverCount; // [esp+98h] [ebp-4h]

    if (*(_DWORD *)&sharedUiInfo.serverStatus.string[1124])
    {
        lowLight[0] = *color * 0.800000011920929;
        lowLight[1] = color[1] * 0.800000011920929;
        lowLight[2] = color[2] * 0.800000011920929;
        lowLight[3] = color[3] * 0.800000011920929;
        if (localClientNum)
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                332,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        newColor[4] = 1.117259e-30;
        v10 = (float)(uiInfoArray.uiDC.realTime / 75);
        v9 = sin(v10);
        t = v9 * 0.5 + 0.5;
        LerpColor(color, lowLight, newColor, t);
        if (LAN_WaitServerResponse(ui_netSource->current.integer))
        {
            string = UI_SafeTranslateString("EXE_WAITINGFORMASTERSERVERRESPONSE");
        }
        else
        {
            serverCount = LAN_GetServerCount(ui_netSource->current.integer);
            string = UI_SafeTranslateString("EXE_GETTINGINFOFORSERVERS");
            string = UI_ReplaceConversionInt((char *)string, serverCount);
        }
        UI_DrawText(
            &scrPlaceView[localClientNum],
            (char *)string,
            0x7FFFFFFF,
            font,
            rect->x,
            rect->y,
            rect->horzAlign,
            rect->vertAlign,
            scale,
            newColor,
            textStyle);
    }
    else
    {
        v6 = va("ui_lastServerRefresh_%i", ui_netSource->current.integer);
        VariantString = (char *)Dvar_GetVariantString(v6);
        I_strncpyz(buff, VariantString, 64);
        string = UI_SafeTranslateString("EXE_REFRESHTIME");
        string = UI_ReplaceConversionString((char *)string, buff);
        UI_DrawText(
            &scrPlaceView[localClientNum],
            (char *)string,
            0x7FFFFFFF,
            font,
            rect->x,
            rect->y,
            rect->horzAlign,
            rect->vertAlign,
            scale,
            color,
            textStyle);
    }
}

void __cdecl UI_DrawKeyBindStatus(
    int localClientNum,
    rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    int textStyle)
{
    char *v6; // eax
    float x; // [esp+0h] [ebp-24h]
    float y; // [esp+4h] [ebp-20h]
    int horzAlign; // [esp+8h] [ebp-1Ch]
    int vertAlign; // [esp+Ch] [ebp-18h]
    const char *v11; // [esp+1Ch] [ebp-8h]

    if (Display_KeyBindPending())
        v11 = "EXE_KEYWAIT";
    else
        v11 = "EXE_KEYCHANGE";
    vertAlign = rect->vertAlign;
    horzAlign = rect->horzAlign;
    y = rect->y;
    x = rect->x;
    v6 = UI_SafeTranslateString(v11);
    UI_DrawText(&scrPlaceView[localClientNum], v6, 0x7FFFFFFF, font, x, y, horzAlign, vertAlign, scale, color, textStyle);
}

void __cdecl UI_DrawLocalTalking(int localClientNum, const rectDef_s *rect, const float *color)
{
    Material *material; // [esp+20h] [ebp-4h]

    if (sv_voice->current.enabled && cl_voice->current.enabled && IN_IsTalkKeyHeld())
    {
        material = Material_RegisterHandle("voice_on", 7);
        UI_DrawHandlePic(
            &scrPlaceView[localClientNum],
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            color,
            material);
    }
}

void __cdecl UI_DrawTalkerNum(
    int localClientNum,
    int num,
    rectDef_s *rect,
    Font_s *font,
    float *color,
    float textScale,
    int style)
{
    float v7; // [esp+20h] [ebp-40h]
    float v8; // [esp+24h] [ebp-3Ch]
    int client; // [esp+28h] [ebp-38h]
    Material *material; // [esp+2Ch] [ebp-34h]
    char name[40]; // [esp+30h] [ebp-30h] BYREF
    int textHeight; // [esp+5Ch] [ebp-4h]

    client = UI_GetTalkerClientNum(localClientNum, num);
    if (client >= 0)
    {
        if (CL_GetClientName(localClientNum, client, name, 38))
        {
            material = Material_RegisterHandle("voice_on", 7);
            textHeight = UI_TextHeight(font, textScale);
            UI_DrawHandlePic(
                &scrPlaceView[localClientNum],
                rect->x,
                rect->y,
                rect->w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                color,
                material);
            v8 = (double)textHeight + rect->y + (rect->h - (double)textHeight) / 2.0;
            v7 = rect->x + rect->w + 2.0;
            UI_DrawText(
                &scrPlaceView[localClientNum],
                name,
                32,
                font,
                v7,
                v8,
                rect->horzAlign,
                rect->vertAlign,
                textScale,
                color,
                style);
        }
        else
        {
            Com_PrintWarning(13, "Unable to find client %i but they are talking\n", client);
        }
    }
}

void UI_CreatePlayerProfile()
{
    char name[32]; // [esp+14h] [ebp-2Ch] BYREF
    int curSelected; // [esp+38h] [ebp-8h]
    int profileIndex; // [esp+3Ch] [ebp-4h]

    if (strlen(ui_playerProfileNameNew->current.string))
    {
        I_strncpyz(name, (char *)ui_playerProfileNameNew->current.integer, 32);
        Dvar_SetString((dvar_s *)ui_playerProfileNameNew, (char *)"");
        if (uiInfoArray.playerProfileCount == 64)
        {
            Menus_OpenByName(&uiInfoArray.uiDC, "profile_create_too_many_popmenu");
        }
        else
        {
            for (profileIndex = 0; profileIndex < uiInfoArray.playerProfileCount; ++profileIndex)
            {
                if (!I_stricmp(name, uiInfoArray.playerProfileName[profileIndex]))
                {
                    Menus_OpenByName(&uiInfoArray.uiDC, "profile_exists_popmenu");
                    return;
                }
            }
            if (Com_NewPlayerProfile(name))
            {
                uiInfoArray.playerProfileName[uiInfoArray.playerProfileCount++] = String_Alloc(name);
                UI_SortPlayerProfiles(0);
                Dvar_SetInt(ui_playerProfileCount, uiInfoArray.playerProfileCount);
                curSelected = UI_GetPlayerProfileListIndexFromName(name);
                if ((unsigned int)curSelected >= uiInfoArray.playerProfileCount)
                    MyAssertHandler(
                        ".\\ui_mp\\ui_main_mp.cpp",
                        2241,
                        0,
                        "curSelected doesn't index uiInfo->playerProfileCount\n\t%i not in [0, %i)",
                        curSelected,
                        uiInfoArray.playerProfileCount);
                UI_SelectPlayerProfileIndex(curSelected);
            }
            else
            {
                Menus_OpenByName(&uiInfoArray.uiDC, "profile_create_fail_popmenu");
            }
        }
    }
}

void UI_AddPlayerProfiles()
{
    const char *v0; // eax
    const char **profileList; // [esp+0h] [ebp-10h]
    int profileCount; // [esp+4h] [ebp-Ch] BYREF
    uiInfo_s *uiInfo; // [esp+8h] [ebp-8h]
    int profileIndex; // [esp+Ch] [ebp-4h]

    uiInfo = &uiInfoArray;
    uiInfoArray.playerProfileCount = 0;
    uiInfo->playerProfileStatus.sortDir = 1;
    profileList = FS_ListFiles("profiles", "/", FS_LIST_ALL, &profileCount);
    for (profileIndex = 0; profileIndex < profileCount; ++profileIndex)
    {
        v0 = String_Alloc(profileList[profileIndex]);
        uiInfo->playerProfileName[uiInfo->playerProfileCount++] = v0;
    }
    FS_FreeFileList(profileList);
    UI_SortPlayerProfiles(0);
    Dvar_SetInt(ui_playerProfileCount, uiInfo->playerProfileCount);
}

bool __cdecl UI_OwnerDrawVisible(__int16 flags)
{
    bool vis; // [esp+0h] [ebp-4h]

    vis = 1;
    if ((flags & 4) != 0)
        vis = ui_netSource->current.integer == 2;
    if ((flags & 0x1000) != 0 && ui_netSource->current.integer == 2)
        return 0;
    return vis;
}

int __cdecl UI_GameType_HandleKey(int flags, float *special, int key, int resetMap)
{
    int oldCount; // [esp+0h] [ebp-8h]
    int nextGameType; // [esp+4h] [ebp-4h]

    if (key != 200 && key != 201 && key != 13 && key != 191)
        return 0;
    oldCount = UI_MapCountByGameType();
    if (key != 201)
    {
        nextGameType = ui_gametype->current.integer + 1;
        if (nextGameType >= sharedUiInfo.numGameTypes)
            goto LABEL_7;
        if (ui_gametype->current.integer == 1)
            nextGameType = 3;
    LABEL_14:
        Dvar_SetInt((dvar_s*)ui_gametype, nextGameType);
        goto LABEL_15;
    }
    nextGameType = ui_gametype->current.integer - 1;
    if (ui_gametype->current.integer != 3)
    {
        if (nextGameType < 2)
            nextGameType = sharedUiInfo.numGameTypes - 1;
        goto LABEL_14;
    }
LABEL_7:
    Dvar_SetInt((dvar_s *)ui_gametype, 1);
LABEL_15:
    if (resetMap)
    {
        if (oldCount != UI_MapCountByGameType())
            Dvar_SetInt((dvar_s *)ui_currentMap, 0);
    }
    return 1;
}

int __cdecl UI_NetSource_HandleKey(int flags, float *special, int key)
{
    int integer; // [esp+0h] [ebp-Ch]
    int nextNetSource; // [esp+8h] [ebp-4h]

    if (key != 200 && key != 201 && key != 13 && key != 191)
        return 0;
    if (key == 201)
    {
        if (ui_netSource->current.integer)
            integer = ui_netSource->current.integer;
        else
            integer = 3;
        Dvar_SetInt(ui_netSource, integer - 1);
    }
    else
    {
        nextNetSource = ui_netSource->current.integer + 1;
        if (ui_netSource->current.integer == 2)
            nextNetSource = 0;
        Dvar_SetInt(ui_netSource, nextNetSource);
    }
    if (ui_netSource->current.integer != 1)
        UI_StartServerRefresh(0, 1);
    UI_BuildServerDisplayList(&uiInfoArray, 1);
    return 1;
}

int __cdecl UI_NetFilter_HandleKey(int flags, float *special, int key)
{
    if (key != 200 && key != 201 && key != 13 && key != 191)
        return 0;
    if (key == 201)
        --ui_serverFilterType;
    else
        ++ui_serverFilterType;
    if (ui_serverFilterType < 1)
    {
        if (ui_serverFilterType < 0)
            ui_serverFilterType = 0;
    }
    else
    {
        ui_serverFilterType = 0;
    }
    UI_BuildServerDisplayList(&uiInfoArray, 1);
    return 1;
}

BOOL __cdecl UI_IsMapActive(int mapIndex)
{
    if (mapIndex < 0 || mapIndex >= sharedUiInfo.mapCount)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            3117,
            0,
            "%s\n\t(mapIndex) = %i",
            "(mapIndex >= 0 && mapIndex < sharedUiInfo.mapCount)",
            mapIndex);
    return sharedUiInfo.serverHardwareIconList[40 * mapIndex - 5081] != 0;
}

void __cdecl UI_SelectListIndexForMapIndex(int mapIndex)
{
    int listIndex; // [esp+0h] [ebp-4h]

    listIndex = UI_GetListIndexFromMapIndex(mapIndex);
    Menu_SetFeederSelection(&uiInfoArray.uiDC, 0, 4, listIndex, "createserver_maps");
}

void UI_SelectFirstActiveMap()
{
    int mapIndex; // [esp+4h] [ebp-4h]

    for (mapIndex = 0; mapIndex < sharedUiInfo.mapCount; ++mapIndex)
    {
        if (sharedUiInfo.serverHardwareIconList[40 * mapIndex - 5081])
        {
            Menu_SetFeederSelection(&uiInfoArray.uiDC, 0, 4, 0, "createserver_maps");
            Dvar_SetInt(ui_currentNetMap, mapIndex);
            return;
        }
    }
    Com_Error(ERR_FATAL, "No active maps found for gametype '%s'.", ui_netGameTypeName->current.string);
}

void __cdecl UI_SelectCurrentMap(int localClientNum)
{
    const char *v1; // eax
    int iCount; // [esp+0h] [ebp-C60h]
    char *info; // [esp+4h] [ebp-C5Ch]
    char szMap[68]; // [esp+8h] [ebp-C58h] BYREF
    int i; // [esp+4Ch] [ebp-C14h]
    uiClientState_s cstate; // [esp+50h] [ebp-C10h] BYREF

    CL_GetClientState(localClientNum, &cstate);
    if (cstate.connState == CA_ACTIVE)
    {
        info = CL_GetConfigString(localClientNum, 0);
        if (*info)
        {
            v1 = Info_ValueForKey(info, "mapname");
            I_strncpyz(szMap, v1, 64);
            iCount = 0;
            for (i = 0; i < sharedUiInfo.mapCount; ++i)
            {
                if (sharedUiInfo.serverHardwareIconList[40 * i - 5081])
                {
                    if (!I_stricmp(szMap, sharedUiInfo.mapList[i].mapName))
                    {
                        Menu_SetFeederSelection(&uiInfoArray.uiDC, 0, 4, iCount, "createserver_maps");
                        return;
                    }
                    ++iCount;
                }
            }
        }
    }
}

int __cdecl UI_NetGameType_HandleKey(int flags, float *special, int key)
{
    int integer; // [esp+0h] [ebp-8h]
    int nextNetGameType; // [esp+4h] [ebp-4h]

    if (key != 200 && key != 201 && key != 13 && key != 191)
        return 0;
    if (key == 201)
    {
        if (ui_netGameType->current.integer)
            integer = ui_netGameType->current.integer;
        else
            integer = sharedUiInfo.numGameTypes;
        Dvar_SetInt(ui_netGameType, integer - 1);
    }
    else
    {
        nextNetGameType = ui_netGameType->current.integer + 1;
        if (nextNetGameType == sharedUiInfo.numGameTypes)
            nextNetGameType = 0;
        Dvar_SetInt(ui_netGameType, nextNetGameType);
    }
    Dvar_SetString((dvar_s *)ui_netGameTypeName, (char *)sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameType);
    Dvar_SetStringByName("g_gametype", (char *)sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameType);
    UI_MapCountByGameType();
    if (UI_IsMapActive(ui_currentNetMap->current.integer))
        UI_SelectListIndexForMapIndex(ui_currentNetMap->current.integer);
    else
        UI_SelectFirstActiveMap();
    UI_SelectCurrentMap(0);
    return 1;
}

int __cdecl UI_JoinGameType_HandleKey(int flags, float *special, int key)
{
    int integer; // [esp+0h] [ebp-8h]
    int nextJoinGameType; // [esp+4h] [ebp-4h]

    if (key != 200 && key != 201 && key != 13 && key != 191)
        return 0;
    if (key == 201)
    {
        if (ui_joinGameType->current.integer)
            integer = ui_joinGameType->current.integer;
        else
            integer = sharedUiInfo.numJoinGameTypes;
        Dvar_SetInt(ui_joinGameType, integer - 1);
    }
    else
    {
        nextJoinGameType = ui_joinGameType->current.integer + 1;
        if (nextJoinGameType == sharedUiInfo.numJoinGameTypes)
            nextJoinGameType = 0;
        Dvar_SetInt(ui_joinGameType, nextJoinGameType);
    }
    UI_BuildServerDisplayList(&uiInfoArray, 1);
    return 1;
}

int __cdecl UI_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key)
{
    switch (ownerDraw)
    {
    case 205:
        return UI_GameType_HandleKey(flags, special, key, 1);
    case 220:
        UI_NetSource_HandleKey(flags, special, key);
        return 0;
    case 222:
        UI_NetFilter_HandleKey(flags, special, key);
        return 0;
    case 245:
        return UI_NetGameType_HandleKey(flags, special, key);
    case 253:
        return UI_JoinGameType_HandleKey(flags, special, key);
    default:
        return 0;
    }
}

int g_editingField;
int __cdecl UI_CheckExecKey(int localClientNum, int key)
{
    menuDef_t *menu; // [esp+4h] [ebp-8h]
    ItemKeyHandler *handler; // [esp+8h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    menu = Menu_GetFocused(&uiInfoArray.uiDC);
    if (g_editingField)
        return 1;
    if (key > 256)
        return 0;
    if (!menu)
        return 0;
    for (handler = menu->onKey; handler; handler = handler->next)
    {
        if (handler->key == key)
            return 1;
    }
    return 0;
}

void __cdecl UI_LoadPlayerProfile(int localClientNum)
{
    if (!ui_playerProfileSelected)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 2289, 0, "%s", "ui_playerProfileSelected");
    if (*(_BYTE *)ui_playerProfileSelected->current.integer)
        Com_ChangePlayerProfile(localClientNum, (char *)ui_playerProfileSelected->current.integer);
}

void __cdecl UI_Update(const char *name)
{
    char *value; // eax
    char *VariantString; // eax
    int rate; // [esp+4h] [ebp-4h]

    if (I_stricmp(name, "ui_SetName"))
    {
        if (I_stricmp(name, "ui_GetName"))
        {
            if (I_stricmp(name, "ui_setRate"))
            {
                if (!I_stricmp(name, "ui_mousePitch"))
                {
                    if (Dvar_GetBool(name))
                        Dvar_SetFloatByName("m_pitch", -0.022);
                    else
                        Dvar_SetFloatByName("m_pitch", 0.022);
                }
            }
            else
            {
                rate = Dvar_GetInt("rate");
                if (rate < 5000)
                {
                    Dvar_SetIntByName("cl_maxpackets", 15);
                    if (rate < 4000)
                        Dvar_SetIntByName("cl_packetdup", 1);
                    else
                        Dvar_SetIntByName("cl_packetdup", 2);
                }
                else
                {
                    Dvar_SetIntByName("cl_maxpackets", 30);
                    Dvar_SetIntByName("cl_packetdup", 1);
                }
            }
        }
        else
        {
            VariantString = (char *)Dvar_GetVariantString("name");
            Dvar_SetStringByName("ui_Name", VariantString);
        }
    }
    else
    {
        value = (char *)Dvar_GetVariantString("ui_Name");
        Dvar_SetStringByName("name", value);
    }
}

void UI_SelectActivePlayerProfile()
{
    int selIndex; // [esp+0h] [ebp-4h]

    selIndex = UI_GetPlayerProfileListIndexFromName(com_playerProfile->current.string);
    if (selIndex >= 0 && selIndex < uiInfoArray.playerProfileCount)
        UI_SelectPlayerProfileIndex(selIndex);
}

int __cdecl LAN_AddServer(int source, char *name, char *address)
{
    const char *v3; // eax
    int i; // [esp+8h] [ebp-24h]
    int *count; // [esp+Ch] [ebp-20h]
    serverInfo_t *servers; // [esp+10h] [ebp-1Ch]
    netadr_t adr; // [esp+14h] [ebp-18h] BYREF

    servers = 0;
    count = 0;
    if (source == 2)
    {
        count = &cls.numfavoriteservers;
        servers = cls.favoriteServers;
    }
    else if (!alwaysfails)
    {
        v3 = va("Unhandled source %i in LAN_AddServer\n", source);
        MyAssertHandler(".\\client_mp\\cl_ui_pc_mp.cpp", 41, 1, v3);
    }
    if (!servers || *count >= 128)
        return -1;
    if (!NET_StringToAdr(address, &adr))
        return -2;
    for (i = 0; i < *count && !NET_CompareAdr(servers[i].adr, adr); ++i)
        ;
    if (i < *count)
        return 0;
    servers[*count].adr = adr;
    I_strncpyz(servers[*count].hostName, name, 32);
    servers[(*count)++].dirty = 1;
    if (source == 1)
        CL_SortGlobalServers();
    return 1;
}

void __cdecl UI_AddServerToFavoritesList(char *pszName, char *pszAddress)
{
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    int res; // [esp+20h] [ebp-4h]

    if (strlen(pszName))
    {
        if (strlen(pszAddress))
        {
            res = LAN_AddServer(2, pszName, pszAddress);
            if (res)
            {
                if (res == -1)
                {
                    v5 = UI_SafeTranslateString("EXE_FAVORITELISTFULL");
                    Com_Printf(13, "%s\n", v5);
                    Dvar_SetStringByName("ui_favorite_message", "@EXE_FAVORITELISTFULL");
                }
                else if (res == -2)
                {
                    v6 = UI_SafeTranslateString("EXE_BADSERVERADDRESS");
                    Com_Printf(13, "%s\n", v6);
                    Dvar_SetStringByName("ui_favorite_message", "@EXE_BADSERVERADDRESS");
                }
                else
                {
                    v7 = UI_SafeTranslateString("EXE_FAVORITEADDED");
                    v8 = va("%s\n", v7);
                    Com_Printf(13, v8, pszAddress);
                    Dvar_SetStringByName("ui_favorite_message", "@EXE_FAVORITEADDED");
                }
            }
            else
            {
                v4 = UI_SafeTranslateString("EXE_FAVORITEINLIST");
                Com_Printf(13, "%s\n", v4);
                Dvar_SetStringByName("ui_favorite_message", "@EXE_FAVORITEINLIST");
            }
        }
        else
        {
            v3 = UI_SafeTranslateString("EXE_FAVORITEADDRESSEMPTY");
            Com_Printf(13, "%s\n", v3);
            Dvar_SetStringByName("ui_favorite_message", "@EXE_FAVORITEADDRESSEMPTY");
        }
    }
    else
    {
        v2 = UI_SafeTranslateString("EXE_FAVORITENAMEEMPTY");
        Com_Printf(13, "%s\n", v2);
        Dvar_SetStringByName("ui_favorite_message", "@EXE_FAVORITENAMEEMPTY");
    }
}

void __cdecl LAN_RemoveServer(int source, char *addr)
{
    const char *v2; // eax
    int j; // [esp+4h] [ebp-28h]
    netadr_t comp; // [esp+8h] [ebp-24h] BYREF
    int i; // [esp+20h] [ebp-Ch]
    int *count; // [esp+24h] [ebp-8h]
    serverInfo_t *servers; // [esp+28h] [ebp-4h]

    servers = 0;
    count = 0;
    if (source == 2)
    {
        count = &cls.numfavoriteservers;
        servers = cls.favoriteServers;
    }
    else if (!alwaysfails)
    {
        v2 = va("Unhandled source %i in LAN_RemoveServer\n", source);
        MyAssertHandler(".\\client_mp\\cl_ui_pc_mp.cpp", 89, 1, v2);
    }
    if (servers)
    {
        NET_StringToAdr(addr, &comp);
        for (i = 0; i < *count; ++i)
        {
            if (NET_CompareAdr(comp, servers[i].adr))
            {
                for (j = i; j < *count - 1; ++j)
                    Com_Memcpy((char *)&servers[j], (char *)&servers[j + 1], 148);
                --*count;
                return;
            }
        }
    }
}

int __cdecl UI_GetPlayerProfileListIndexFromName(const char *name)
{
    unsigned int nameIndex; // [esp+4h] [ebp-8h]
    int profileIndex; // [esp+8h] [ebp-4h]

    if (!name)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 2090, 0, "%s", "name");
    for (profileIndex = 0; profileIndex < uiInfoArray.playerProfileCount; ++profileIndex)
    {
        nameIndex = uiInfoArray.playerProfileStatus.displayProfile[profileIndex];
        if (nameIndex >= uiInfoArray.playerProfileCount)
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                2096,
                0,
                "nameIndex doesn't index uiInfo->playerProfileCount\n\t%i not in [0, %i)",
                nameIndex,
                uiInfoArray.playerProfileCount);
        if (!I_stricmp(name, uiInfoArray.playerProfileName[nameIndex]))
            return profileIndex;
    }
    return -1;
}

const char *UI_LoadMods()
{
    const char *result; // eax
    int numdirs; // [esp+20h] [ebp-818h]
    const char *dirptr; // [esp+24h] [ebp-814h]
    char dirlist[2048]; // [esp+28h] [ebp-810h] BYREF
    char *descptr; // [esp+82Ch] [ebp-Ch]
    int i; // [esp+830h] [ebp-8h]
    int dirlen; // [esp+834h] [ebp-4h]

    sharedUiInfo.modCount = 0;
    sharedUiInfo.modIndex = 0;
    numdirs = FS_GetFileList("$modlist", "", FS_LIST_ALL, dirlist, 2048);
    dirptr = dirlist;
    for (i = 0; ; ++i)
    {
        result = (const char *)i;
        if (i >= numdirs)
            break;
        dirlen = strlen(dirptr) + 1;
        descptr = (char *)&dirptr[dirlen];
        sharedUiInfo.modList[sharedUiInfo.modCount].modName = String_Alloc(dirptr);
        sharedUiInfo.modList[sharedUiInfo.modCount].modDescr = String_Alloc(descptr);
        result = &dirptr[strlen(descptr) + 1 + dirlen];
        dirptr = result;
        if (++sharedUiInfo.modCount >= 64)
            break;
    }
    return result;
}

int __cdecl UI_PlayerProfilesQsortCompare(_DWORD *arg1, _DWORD *arg2)
{
    int result; // [esp+0h] [ebp-10h]

    if (!arg1)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 2111, 0, "%s", "arg1");
    if (!arg2)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 2112, 0, "%s", "arg2");
    if (*arg1 == *arg2)
        return 0;
    result = I_stricmp(uiInfoArray.playerProfileName[*arg1], uiInfoArray.playerProfileName[*arg2]);
    if (uiInfoArray.playerProfileStatus.sortDir)
        return result;
    else
        return -result;
}

void __cdecl UI_SelectPlayerProfileIndex(int index)
{
    int menuIndex; // [esp+8h] [ebp-4h]

    for (menuIndex = uiInfoArray.uiDC.openMenuCount - 1; menuIndex >= 0; --menuIndex)
    {
        if (Window_IsVisible(0, &uiInfoArray.uiDC.menuStack[menuIndex]->window))
            Menu_SetFeederSelection(&uiInfoArray.uiDC, uiInfoArray.uiDC.menuStack[menuIndex], 24, index, 0);
    }
}

void __cdecl UI_SortPlayerProfiles(int selectIndex)
{
    int profileIndex; // [esp+4h] [ebp-4h]

    if (uiInfoArray.playerProfileCount)
    {
        for (profileIndex = 0; profileIndex < uiInfoArray.playerProfileCount; ++profileIndex)
            uiInfoArray.playerProfileStatus.displayProfile[profileIndex] = profileIndex;
        qsort(
            uiInfoArray.playerProfileStatus.displayProfile,
            uiInfoArray.playerProfileCount,
            4u,
            (int(__cdecl *)(const void *, const void *))UI_PlayerProfilesQsortCompare);
        UI_SelectPlayerProfileIndex(selectIndex);
    }
}

void UI_DeletePlayerProfile()
{
    const char *v0; // eax
    unsigned int curSelected; // [esp+8h] [ebp-8h]
    unsigned int nameIndex; // [esp+Ch] [ebp-4h]

    if (uiInfoArray.playerProfileCount)
    {
        if (!ui_playerProfileSelected)
            MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 2257, 0, "%s", "ui_playerProfileSelected");
        if (Com_DeletePlayerProfile(ui_playerProfileSelected->current.string))
        {
            curSelected = UI_GetPlayerProfileListIndexFromName(ui_playerProfileSelected->current.string);
            if (curSelected >= uiInfoArray.playerProfileCount)
                MyAssertHandler(
                    ".\\ui_mp\\ui_main_mp.cpp",
                    2265,
                    0,
                    "curSelected doesn't index uiInfo->playerProfileCount\n\t%i not in [0, %i)",
                    curSelected,
                    uiInfoArray.playerProfileCount);
            nameIndex = uiInfoArray.playerProfileStatus.displayProfile[curSelected];
            if (nameIndex >= uiInfoArray.playerProfileCount)
                MyAssertHandler(
                    ".\\ui_mp\\ui_main_mp.cpp",
                    2268,
                    0,
                    "nameIndex doesn't index uiInfo->playerProfileCount\n\t%i not in [0, %i)",
                    nameIndex,
                    uiInfoArray.playerProfileCount);
            if (I_stricmp(uiInfoArray.playerProfileName[nameIndex], ui_playerProfileSelected->current.string))
            {
                v0 = va("%s != %s", uiInfoArray.playerProfileName[nameIndex], ui_playerProfileSelected->current.string);
                MyAssertHandler(
                    ".\\ui_mp\\ui_main_mp.cpp",
                    2269,
                    0,
                    "%s\n\t%s",
                    "!I_stricmp( uiInfo->playerProfileName[nameIndex], ui_playerProfileSelected->current.string )",
                    v0);
            }
            if (--uiInfoArray.playerProfileCount)
            {
                uiInfoArray.playerProfileName[nameIndex] = uiInfoArray.playerProfileName[uiInfoArray.playerProfileCount];
                if (curSelected == uiInfoArray.playerProfileCount)
                    --curSelected;
                UI_SortPlayerProfiles(curSelected);
            }
            else
            {
                Dvar_SetString((dvar_s *)ui_playerProfileSelected, (char *)"");
            }
            Dvar_SetInt(ui_playerProfileCount, uiInfoArray.playerProfileCount);
        }
        else
        {
            Menus_OpenByName(&uiInfoArray.uiDC, "profile_delete_fail_popmenu");
        }
    }
}

void __cdecl LAN_GetServerAddressString(int source, unsigned int n, char *buf, int buflen)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax

    if (source)
    {
        if (source == 1)
        {
            if ((n & 0x80000000) == 0 && (int)n < cls.numglobalservers)
            {
                v5 = NET_AdrToString(cls.globalServers[n].adr);
                I_strncpyz(buf, v5, buflen);
                return;
            }
        }
        else if (source == 2 && n < 0x80)
        {
            v6 = NET_AdrToString(cls.favoriteServers[n].adr);
            I_strncpyz(buf, v6, buflen);
            return;
        }
    }
    else if (n < 0x80)
    {
        v4 = NET_AdrToString(cls.localServers[n].adr);
        I_strncpyz(buf, v4, buflen);
        return;
    }
    *buf = 0;
}

void UI_SelectCurrentGameType()
{
    const char *currType; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    currType = Dvar_GetString("g_gametype");
    for (i = 0; i < sharedUiInfo.numGameTypes; ++i)
    {
        if (!I_stricmp(currType, sharedUiInfo.gameTypes[i].gameType))
        {
            Dvar_SetInt(ui_netGameType, i);
            Dvar_SetString((dvar_s *)ui_netGameTypeName, (char *)sharedUiInfo.gameTypes[i].gameType);
            return;
        }
    }
}

void __cdecl UI_RunMenuScript(int localClientNum, char **args, const char *actualScript)
{
    char *v3; // eax
    const char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    const char *v14; // eax
    const dvar_s *Var; // eax
    const char *v16; // eax
    int v17; // eax
    const char *v18; // eax
    const char *v19; // eax
    const char *v20; // eax
    const char *v21; // eax
    const char *v22; // eax
    char *VariantString; // eax
    int v24; // eax
    int Int; // eax
    unsigned int ClientNumForPlayerListNum; // eax
    char v27[128]; // [esp+3Ch] [ebp-29F0h] BYREF
    char v28[256]; // [esp+BCh] [ebp-2970h] BYREF
    char v29[128]; // [esp+1BCh] [ebp-2870h] BYREF
    char menuName[128]; // [esp+23Ch] [ebp-27F0h] BYREF
    char testValue[256]; // [esp+2BCh] [ebp-2770h] BYREF
    char dvarName[132]; // [esp+3BCh] [ebp-2670h] BYREF
    int status; // [esp+440h] [ebp-25ECh] BYREF
    char name[1024]; // [esp+444h] [ebp-25E8h] BYREF
    char v35[32]; // [esp+844h] [ebp-21E8h] BYREF
    char v36[32]; // [esp+864h] [ebp-21C8h] BYREF
    char addr; // [esp+884h] [ebp-21A8h] BYREF
    _BYTE v38[3]; // [esp+885h] [ebp-21A7h] BYREF
    char v39[1024]; // [esp+8A4h] [ebp-2188h] BYREF
    char pszAddress[32]; // [esp+CA4h] [ebp-1D88h] BYREF
    char pszName[32]; // [esp+CC4h] [ebp-1D68h] BYREF
    char s[1028]; // [esp+CE4h] [ebp-1D48h] BYREF
    int i; // [esp+10E8h] [ebp-1944h] BYREF
    char v44[1028]; // [esp+10ECh] [ebp-1940h] BYREF
    int ServerPunkBuster; // [esp+14F0h] [ebp-153Ch]
    char value[264]; // [esp+14F4h] [ebp-1538h] BYREF
    char key[1024]; // [esp+15FCh] [ebp-1430h] BYREF
    char checksum[1028]; // [esp+19FCh] [ebp-1030h] BYREF
    char dest[20]; // [esp+1E00h] [ebp-C2Ch] BYREF
    char buf; // [esp+1E14h] [ebp-C18h] BYREF
    _BYTE v51[3]; // [esp+1E15h] [ebp-C17h] BYREF
    char src[4]; // [esp+1E18h] [ebp-C14h] BYREF
    char v53[4]; // [esp+1E1Ch] [ebp-C10h] BYREF
    char v54[1012]; // [esp+1E20h] [ebp-C0Ch] BYREF
    char buf2[1030]; // [esp+2214h] [ebp-818h] BYREF
    bool v56; // [esp+261Ah] [ebp-412h]
    bool Bool; // [esp+261Bh] [ebp-411h]
    UiContext *dc; // [esp+261Ch] [ebp-410h]
    char out[1028]; // [esp+2624h] [ebp-408h] BYREF

    if (String_Parse((const char **)args, out, 1024))
    {
        if (localClientNum)
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                332,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        dc = (UiContext *)&uiInfoArray;
        if (I_stricmp(out, "StartServer"))
        {
            if (I_stricmp(out, "getCDKey"))
            {
                if (I_stricmp(out, "verifyCDKey"))
                {
                    if (I_stricmp(out, "loadArenas"))
                    {
                        if (I_stricmp(out, "loadGameInfo"))
                        {
                            if (I_stricmp(out, "LoadMods"))
                            {
                                if (I_stricmp(out, "voteTypeMap"))
                                {
                                    if (I_stricmp(out, "voteMap"))
                                    {
                                        if (I_stricmp(out, "voteGame"))
                                        {
                                            if (I_stricmp(out, "clearError"))
                                            {
                                                if (I_stricmp(out, "RefreshServers"))
                                                {
                                                    if (I_stricmp(out, "RefreshFilter"))
                                                    {
                                                        if (I_stricmp(out, "addPlayerProfiles"))
                                                        {
                                                            if (I_stricmp(out, "sortPlayerProfiles"))
                                                            {
                                                                if (I_stricmp(out, "selectActivePlayerProfile"))
                                                                {
                                                                    if (I_stricmp(out, "createPlayerProfile"))
                                                                    {
                                                                        if (I_stricmp(out, "deletePlayerProfile"))
                                                                        {
                                                                            if (I_stricmp(out, "loadPlayerProfile"))
                                                                            {
                                                                                if (I_stricmp(out, "RunMod"))
                                                                                {
                                                                                    if (I_stricmp(out, "ClearMods"))
                                                                                    {
                                                                                        if (I_stricmp(out, "closeJoin"))
                                                                                        {
                                                                                            if (I_stricmp(out, "StopRefresh"))
                                                                                            {
                                                                                                if (I_stricmp(out, "ServerStatus"))
                                                                                                {
                                                                                                    if (I_stricmp(out, "UpdateFilter"))
                                                                                                    {
                                                                                                        if (I_stricmp(out, "JoinServer"))
                                                                                                        {
                                                                                                            if (I_stricmp(out, "Quit"))
                                                                                                            {
                                                                                                                if (I_stricmp(out, "Controls"))
                                                                                                                {
                                                                                                                    if (I_stricmp(out, "Leave"))
                                                                                                                    {
                                                                                                                        if (I_stricmp(out, "ServerSort"))
                                                                                                                        {
                                                                                                                            if (I_stricmp(out, "closeingame"))
                                                                                                                            {
                                                                                                                                if (I_stricmp(out, "voteKick"))
                                                                                                                                {
                                                                                                                                    if (I_stricmp(out, "voteTempBan"))
                                                                                                                                    {
                                                                                                                                        if (I_stricmp(out, "addFavorite"))
                                                                                                                                        {
                                                                                                                                            if (I_stricmp(out, "deleteFavorite"))
                                                                                                                                            {
                                                                                                                                                if (I_stricmp(out, "createFavorite"))
                                                                                                                                                {
                                                                                                                                                    if (I_stricmp(out, "update"))
                                                                                                                                                    {
                                                                                                                                                        if (I_stricmp(out, "setPbClStatus"))
                                                                                                                                                        {
                                                                                                                                                            if (I_stricmp(out, "startSingleplayer"))
                                                                                                                                                            {
                                                                                                                                                                if (I_stricmp(out, "getLanguage"))
                                                                                                                                                                {
                                                                                                                                                                    if (I_stricmp(out, "verifyLanguage"))
                                                                                                                                                                    {
                                                                                                                                                                        if (I_stricmp(
                                                                                                                                                                            out,
                                                                                                                                                                            "updateLanguage"))
                                                                                                                                                                        {
                                                                                                                                                                            if (I_stricmp(out, "mutePlayer"))
                                                                                                                                                                            {
                                                                                                                                                                                if (I_stricmp(
                                                                                                                                                                                    out,
                                                                                                                                                                                    "openMenuOnDvar")
                                                                                                                                                                                    && I_stricmp(
                                                                                                                                                                                        out,
                                                                                                                                                                                        "openMenuOnDvarNot"))
                                                                                                                                                                                {
                                                                                                                                                                                    if (I_stricmp(
                                                                                                                                                                                        out,
                                                                                                                                                                                        "closeMenuOnDvar")
                                                                                                                                                                                        && I_stricmp(
                                                                                                                                                                                            out,
                                                                                                                                                                                            "closeMenuOnDvarNot"))
                                                                                                                                                                                    {
                                                                                                                                                                                        if (I_stricmp(
                                                                                                                                                                                            out,
                                                                                                                                                                                            "setRecommended"))
                                                                                                                                                                                        {
                                                                                                                                                                                            if (I_stricmp(
                                                                                                                                                                                                out,
                                                                                                                                                                                                "clearLoadErrorsSummary"))
                                                                                                                                                                                            {
                                                                                                                                                                                                Com_Printf(
                                                                                                                                                                                                    13,
                                                                                                                                                                                                    "unknown UI script %s in block:\n%s\n",
                                                                                                                                                                                                    out,
                                                                                                                                                                                                    actualScript);
                                                                                                                                                                                            }
                                                                                                                                                                                            else
                                                                                                                                                                                            {
                                                                                                                                                                                                Menus_CloseAll(dc);
                                                                                                                                                                                            }
                                                                                                                                                                                        }
                                                                                                                                                                                        else
                                                                                                                                                                                        {
                                                                                                                                                                                            Com_SetRecommended(
                                                                                                                                                                                                localClientNum,
                                                                                                                                                                                                1);
                                                                                                                                                                                        }
                                                                                                                                                                                    }
                                                                                                                                                                                    else if (UI_GetOpenOrCloseMenuOnDvarArgs((const char **)args, out, v29, 128, v28, 256, v27, 128))
                                                                                                                                                                                    {
                                                                                                                                                                                        UI_CloseMenuOnDvar(
                                                                                                                                                                                            (uiInfo_s *)dc,
                                                                                                                                                                                            out,
                                                                                                                                                                                            v27,
                                                                                                                                                                                            v29,
                                                                                                                                                                                            v28);
                                                                                                                                                                                    }
                                                                                                                                                                                }
                                                                                                                                                                                else if (UI_GetOpenOrCloseMenuOnDvarArgs(
                                                                                                                                                                                    (const char **)args,
                                                                                                                                                                                    out,
                                                                                                                                                                                    dvarName,
                                                                                                                                                                                    128,
                                                                                                                                                                                    testValue,
                                                                                                                                                                                    256,
                                                                                                                                                                                    menuName,
                                                                                                                                                                                    128))
                                                                                                                                                                                {
                                                                                                                                                                                    UI_OpenMenuOnDvar(
                                                                                                                                                                                        (uiInfo_s *)dc,
                                                                                                                                                                                        out,
                                                                                                                                                                                        menuName,
                                                                                                                                                                                        dvarName,
                                                                                                                                                                                        testValue);
                                                                                                                                                                                }
                                                                                                                                                                            }
                                                                                                                                                                            else if (dc[1].realTime >= 0
                                                                                                                                                                                && dc[1].realTime < sharedUiInfo.playerCount)
                                                                                                                                                                            {
                                                                                                                                                                                ClientNumForPlayerListNum = UI_GetClientNumForPlayerListNum(dc[1].realTime);
                                                                                                                                                                                CL_MutePlayer(
                                                                                                                                                                                    localClientNum,
                                                                                                                                                                                    ClientNumForPlayerListNum);
                                                                                                                                                                            }
                                                                                                                                                                        }
                                                                                                                                                                        else
                                                                                                                                                                        {
                                                                                                                                                                            Int = Dvar_GetInt("ui_language");
                                                                                                                                                                            Dvar_SetIntByName(
                                                                                                                                                                                "loc_language",
                                                                                                                                                                                Int);
                                                                                                                                                                            UI_VerifyLanguage();
                                                                                                                                                                            Cbuf_AddText(
                                                                                                                                                                                localClientNum,
                                                                                                                                                                                "vid_restart\n");
                                                                                                                                                                        }
                                                                                                                                                                    }
                                                                                                                                                                    else
                                                                                                                                                                    {
                                                                                                                                                                        UI_VerifyLanguage();
                                                                                                                                                                    }
                                                                                                                                                                }
                                                                                                                                                                else
                                                                                                                                                                {
                                                                                                                                                                    v24 = Dvar_GetInt("loc_language");
                                                                                                                                                                    Dvar_SetIntByName("ui_language", v24);
                                                                                                                                                                    UI_VerifyLanguage();
                                                                                                                                                                }
                                                                                                                                                            }
                                                                                                                                                            else
                                                                                                                                                            {
                                                                                                                                                                Cbuf_AddText(
                                                                                                                                                                    localClientNum,
                                                                                                                                                                    "startSingleplayer\n");
                                                                                                                                                            }
                                                                                                                                                        }
                                                                                                                                                        else if (Int_Parse(
                                                                                                                                                            (const char **)args,
                                                                                                                                                            &status))
                                                                                                                                                        {
                                                                                                                                                            CLUI_SetPbClStatus(status);
                                                                                                                                                        }
                                                                                                                                                    }
                                                                                                                                                    else if (String_Parse(
                                                                                                                                                        (const char **)args,
                                                                                                                                                        name,
                                                                                                                                                        1024))
                                                                                                                                                    {
                                                                                                                                                        UI_Update(name);
                                                                                                                                                    }
                                                                                                                                                }
                                                                                                                                                else if (ui_netSource->current.integer == 2)
                                                                                                                                                {
                                                                                                                                                    v35[0] = 0;
                                                                                                                                                    v36[0] = 0;
                                                                                                                                                    I_strncpyz(v36, "default", 32);
                                                                                                                                                    VariantString = (char *)Dvar_GetVariantString("ui_favoriteAddress");
                                                                                                                                                    I_strncpyz(v35, VariantString, 32);
                                                                                                                                                    UI_AddServerToFavoritesList(v36, v35);
                                                                                                                                                    UI_StartServerRefresh(localClientNum, 1);
                                                                                                                                                }
                                                                                                                                            }
                                                                                                                                            else if (ui_netSource->current.integer == 2
                                                                                                                                                && *(int *)&sharedUiInfo.serverStatus.string[1128] >= 0
                                                                                                                                                && *(int *)&sharedUiInfo.serverStatus.string[1128] < *(int *)&sharedUiInfo.gap8EB4[72900])
                                                                                                                                            {
                                                                                                                                                UI_UpdateDisplayServers((uiInfo_s *)dc);
                                                                                                                                                LAN_GetServerInfo(
                                                                                                                                                    ui_netSource->current.integer,
                                                                                                                                                    *(_DWORD *)&sharedUiInfo.gap8EB4[4 * *(_DWORD *)&sharedUiInfo.serverStatus.string[1128] - 7100],
                                                                                                                                                    v39,
                                                                                                                                                    1024);
                                                                                                                                                addr = 0;
                                                                                                                                                v22 = Info_ValueForKey(v39, "addr");
                                                                                                                                                I_strncpyz(&addr, v22, 32);
                                                                                                                                                if (&v38[strlen(&addr)] != v38)
                                                                                                                                                    LAN_RemoveServer(2, &addr);
                                                                                                                                            }
                                                                                                                                        }
                                                                                                                                        else if (ui_netSource->current.integer != 2)
                                                                                                                                        {
                                                                                                                                            pszAddress[0] = 0;
                                                                                                                                            pszName[0] = 0;
                                                                                                                                            UI_UpdateDisplayServers((uiInfo_s *)dc);
                                                                                                                                            if (*(int *)&sharedUiInfo.serverStatus.string[1128] >= 0
                                                                                                                                                && *(int *)&sharedUiInfo.serverStatus.string[1128] < *(int *)&sharedUiInfo.gap8EB4[72900])
                                                                                                                                            {
                                                                                                                                                LAN_GetServerInfo(
                                                                                                                                                    ui_netSource->current.integer,
                                                                                                                                                    *(_DWORD *)&sharedUiInfo.gap8EB4[4 * *(_DWORD *)&sharedUiInfo.serverStatus.string[1128] - 7100],
                                                                                                                                                    s,
                                                                                                                                                    1024);
                                                                                                                                                v20 = Info_ValueForKey(s, "hostname");
                                                                                                                                                I_strncpyz(pszName, v20, 32);
                                                                                                                                                v21 = Info_ValueForKey(s, "addr");
                                                                                                                                                I_strncpyz(pszAddress, v21, 32);
                                                                                                                                            }
                                                                                                                                            UI_AddServerToFavoritesList(pszName, pszAddress);
                                                                                                                                        }
                                                                                                                                    }
                                                                                                                                    else if (dc[1].realTime >= 0
                                                                                                                                        && dc[1].realTime < sharedUiInfo.playerCount)
                                                                                                                                    {
                                                                                                                                        v19 = va(
                                                                                                                                            "callvote tempBanUser \"%s\"\n",
                                                                                                                                            sharedUiInfo.playerNames[dc[1].realTime]);
                                                                                                                                        Cbuf_AddText(localClientNum, v19);
                                                                                                                                    }
                                                                                                                                }
                                                                                                                                else if (dc[1].realTime >= 0
                                                                                                                                    && dc[1].realTime < sharedUiInfo.playerCount)
                                                                                                                                {
                                                                                                                                    v18 = va(
                                                                                                                                        "callvote kick \"%s\"\n",
                                                                                                                                        sharedUiInfo.playerNames[dc[1].realTime]);
                                                                                                                                    Cbuf_AddText(localClientNum, v18);
                                                                                                                                }
                                                                                                                            }
                                                                                                                            else
                                                                                                                            {
                                                                                                                                Key_RemoveCatcher(localClientNum, -17);
                                                                                                                                Key_ClearStates(localClientNum);
                                                                                                                                Dvar_SetIntByName("cl_paused", 0);
                                                                                                                                Menus_CloseAll(dc);
                                                                                                                            }
                                                                                                                        }
                                                                                                                        else if (Int_Parse((const char **)args, &i))
                                                                                                                        {
                                                                                                                            if (i == *(_DWORD *)&sharedUiInfo.serverStatus.string[1112])
                                                                                                                                *(_DWORD *)&sharedUiInfo.serverStatus.string[1116] = *(_DWORD *)&sharedUiInfo.serverStatus.string[1116] == 0;
                                                                                                                            UI_ServersSort(i, 1);
                                                                                                                        }
                                                                                                                    }
                                                                                                                    else
                                                                                                                    {
                                                                                                                        Cbuf_AddText(localClientNum, "disconnect\n");
                                                                                                                        Key_SetCatcher(localClientNum, 16);
                                                                                                                        Menus_CloseAll(dc);
                                                                                                                        Menus_OpenByName(dc, "main");
                                                                                                                    }
                                                                                                                }
                                                                                                                else
                                                                                                                {
                                                                                                                    Dvar_SetIntByName("cl_paused", 1);
                                                                                                                    Key_SetCatcher(localClientNum, 16);
                                                                                                                    Menus_CloseAll(dc);
                                                                                                                    Menus_OpenByName(dc, "setup_menu2");
                                                                                                                }
                                                                                                            }
                                                                                                            else
                                                                                                            {
                                                                                                                v17 = CL_ControllerIndexFromClientNum(localClientNum);
                                                                                                                Cmd_ExecuteSingleCommand(localClientNum, v17, (char*)"quit");
                                                                                                            }
                                                                                                        }
                                                                                                        else
                                                                                                        {
                                                                                                            Dvar_SetBoolByName("cg_thirdPerson", 0);
                                                                                                            UI_UpdateDisplayServers((uiInfo_s *)dc);
                                                                                                            // LWSS: Remove punkbuster crap
                                                                                                            //ServerPunkBuster = LAN_GetServerPunkBuster(
                                                                                                            //    ui_netSource->current.integer,
                                                                                                            //    *(_DWORD *)&sharedUiInfo.gap8EB4[4 * *(_DWORD *)&sharedUiInfo.serverStatus.string[1128] - 7100]);
                                                                                                            //if (ServerPunkBuster != 1 || Dvar_GetBool("cl_punkbuster"))
                                                                                                            //{
                                                                                                            //    if (*(int *)&sharedUiInfo.serverStatus.string[1128] >= 0
                                                                                                            //        && *(int *)&sharedUiInfo.serverStatus.string[1128] < *(int *)&sharedUiInfo.gap8EB4[72900])
                                                                                                            //    {
                                                                                                            //        LAN_GetServerAddressString(
                                                                                                            //            ui_netSource->current.integer,
                                                                                                            //            *(_DWORD *)&sharedUiInfo.gap8EB4[4
                                                                                                            //            * *(_DWORD *)&sharedUiInfo.serverStatus.string[1128]
                                                                                                            //            - 7100],
                                                                                                            //            v44,
                                                                                                            //            1024);
                                                                                                            //        v16 = va("connect %s\n", v44);
                                                                                                            //        Cbuf_AddText(localClientNum, v16);
                                                                                                            //    }
                                                                                                            //}
                                                                                                            //else
                                                                                                            //{
                                                                                                                Menus_OpenByName(dc, "joinpb_popmenu");
                                                                                                            //}
                                                                                                        }
                                                                                                    }
                                                                                                    else
                                                                                                    {
                                                                                                        if (!ui_netSource->current.integer)
                                                                                                            UI_StartServerRefresh(localClientNum, 1);
                                                                                                        UI_BuildServerDisplayList((uiInfo_s *)dc, 1);
                                                                                                        UI_FeederSelection(localClientNum, 2.0, 0);
                                                                                                    }
                                                                                                }
                                                                                                else
                                                                                                {
                                                                                                    UI_UpdateDisplayServers(&uiInfoArray);
                                                                                                    if (*(int *)&sharedUiInfo.serverStatus.string[1128] >= 0
                                                                                                        && *(int *)&sharedUiInfo.serverStatus.string[1128] < *(int *)&sharedUiInfo.gap8EB4[72900])
                                                                                                    {
                                                                                                        LAN_GetServerAddressString(
                                                                                                            ui_netSource->current.integer,
                                                                                                            *(_DWORD *)&sharedUiInfo.gap8EB4[4
                                                                                                            * *(_DWORD *)&sharedUiInfo.serverStatus.string[1128]
                                                                                                            - 7100],
                                                                                                            sharedUiInfo.serverStatusAddress,
                                                                                                            64);
                                                                                                        UI_BuildServerStatus(&uiInfoArray, 1);
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                            else
                                                                                            {
                                                                                                UI_StopServerRefresh();
                                                                                                *(_DWORD *)&sharedUiInfo.gap8EB4[72912] = 0;
                                                                                                sharedUiInfo.nextServerStatusRefresh = 0;
                                                                                                dc[1].localVars.table[79].u.integer = 0;
                                                                                            }
                                                                                        }
                                                                                        else if (*(_DWORD *)&sharedUiInfo.serverStatus.string[1124])
                                                                                        {
                                                                                            UI_StopServerRefresh();
                                                                                            *(_DWORD *)&sharedUiInfo.gap8EB4[72912] = 0;
                                                                                            sharedUiInfo.nextServerStatusRefresh = 0;
                                                                                            dc[1].localVars.table[79].u.integer = 0;
                                                                                            UI_BuildServerDisplayList((uiInfo_s *)dc, 1);
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            Menus_CloseByName(dc, "joinserver");
                                                                                            Menus_OpenByName(dc, "main");
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        if (useFastFile->current.enabled)
                                                                                            DB_SyncXAssets();
                                                                                        Var = Dvar_FindVar("fs_game");
                                                                                        Dvar_Reset(Var, DVAR_SOURCE_INTERNAL);
                                                                                        Cbuf_AddText(localClientNum, "vid_restart\n");
                                                                                    }
                                                                                }
                                                                                else
                                                                                {
                                                                                    if (sharedUiInfo.modIndex >= 0x40u)
                                                                                        MyAssertHandler(
                                                                                            ".\\ui_mp\\ui_main_mp.cpp",
                                                                                            2780,
                                                                                            0,
                                                                                            "sharedUiInfo.modIndex doesn't index MAX_MODS\n\t%i not in [0, %i)",
                                                                                            sharedUiInfo.modIndex,
                                                                                            64);
                                                                                    if (sharedUiInfo.modList[sharedUiInfo.modIndex].modName)
                                                                                    {
                                                                                        Com_sprintf(
                                                                                            value,
                                                                                            0x104u,
                                                                                            "%s/%s",
                                                                                            "mods",
                                                                                            sharedUiInfo.modList[sharedUiInfo.modIndex].modName);
                                                                                        if (useFastFile->current.enabled)
                                                                                            DB_SyncXAssets();
                                                                                        Dvar_SetStringByName("fs_game", value);
                                                                                        Cbuf_AddText(localClientNum, "vid_restart\n");
                                                                                    }
                                                                                }
                                                                            }
                                                                            else
                                                                            {
                                                                                UI_LoadPlayerProfile(localClientNum);
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            UI_DeletePlayerProfile();
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        UI_CreatePlayerProfile();
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    UI_SelectActivePlayerProfile();
                                                                }
                                                            }
                                                            else
                                                            {
                                                                dc[1].Menus[56] = (menuDef_t *)(dc[1].Menus[56] == 0);
                                                                UI_SortPlayerProfiles(0);
                                                            }
                                                        }
                                                        else
                                                        {
                                                            UI_AddPlayerProfiles();
                                                        }
                                                    }
                                                    else
                                                    {
                                                        UI_StartServerRefresh(localClientNum, 0);
                                                        UI_BuildServerDisplayList((uiInfo_s *)dc, 1);
                                                    }
                                                }
                                                else
                                                {
                                                    UI_StartServerRefresh(localClientNum, 1);
                                                    UI_BuildServerDisplayList((uiInfo_s *)dc, 1);
                                                }
                                            }
                                            else
                                            {
                                                Dvar_SetStringByName("com_errorMessage", (char *)"");
                                                Dvar_SetBoolByName("com_isNotice", 0);
                                                if (localClientNum)
                                                    MyAssertHandler(
                                                        "c:\\trees\\cod3\\src\\ui_mp\\../client_mp/client_mp.h",
                                                        1112,
                                                        0,
                                                        "%s\n\t(localClientNum) = %i",
                                                        "(localClientNum == 0)",
                                                        localClientNum);
                                                if (clientUIActives[0].connectionState > CA_DISCONNECTED)
                                                    Key_RemoveCatcher(localClientNum, -17);
                                            }
                                        }
                                        else
                                        {
                                            v14 = va(
                                                "callvote g_gametype %s\n",
                                                sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameType);
                                            Cbuf_AddText(localClientNum, v14);
                                        }
                                    }
                                    else if (ui_currentNetMap->current.integer >= 0
                                        && ui_currentNetMap->current.integer < sharedUiInfo.mapCount)
                                    {
                                        v13 = va(
                                            "callvote map %s\n",
                                            (const char *)sharedUiInfo.serverHardwareIconList[40 * ui_currentNetMap->current.integer
                                            - 5119]);
                                        Cbuf_AddText(localClientNum, v13);
                                    }
                                }
                                else
                                {
                                    v12 = va(
                                        "callvote typemap %s %s\n",
                                        sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameType,
                                        (const char *)sharedUiInfo.serverHardwareIconList[40 * ui_currentNetMap->current.integer
                                        - 5119]);
                                    Cbuf_AddText(localClientNum, v12);
                                }
                            }
                            else
                            {
                                UI_LoadMods();
                            }
                        }
                        else
                        {
                            UI_GetGameTypesList();
                        }
                    }
                    else
                    {
                        UI_LoadArenas();
                        UI_SelectCurrentGameType();
                        UI_MapCountByGameType();
                        Menu_SetFeederSelection(dc, 0, 4, 0, "createserver_maps");
                        UI_SelectCurrentMap(localClientNum);
                    }
                }
                else
                {
                    key[0] = 0;
                    v5 = (char *)Dvar_GetVariantString("cdkey1");
                    I_strncat(key, 1024, v5);
                    v6 = (char *)Dvar_GetVariantString("cdkey2");
                    I_strncat(key, 1024, v6);
                    v7 = (char *)Dvar_GetVariantString("cdkey3");
                    I_strncat(key, 1024, v7);
                    v8 = (char *)Dvar_GetVariantString("cdkey4");
                    I_strncat(key, 1024, v8);
                    checksum[0] = 0;
                    v9 = (char *)Dvar_GetVariantString("cdkey5");
                    I_strncat(checksum, 1024, v9);
                    if (CL_CDKeyValidate(key, checksum))
                    {
                        v10 = UI_SafeTranslateString("EXE_CDKEYVALID");
                        Dvar_SetStringByName("ui_cdkeyvalid", v10);
                        CLUI_SetCDKey(key, checksum);
                    }
                    else
                    {
                        v11 = UI_SafeTranslateString("EXE_CDKEYINVALID");
                        Dvar_SetStringByName("ui_cdkeyvalid", v11);
                    }
                }
            }
            else
            {
                CLUI_GetCDKey(&buf, 17, buf2, 5);
                Dvar_SetStringByName("cdkey1", (char *)"");
                Dvar_SetStringByName("cdkey2", (char *)"");
                Dvar_SetStringByName("cdkey3", (char *)"");
                Dvar_SetStringByName("cdkey4", (char *)"");
                Dvar_SetStringByName("cdkey5", (char *)"");
                if (&v51[strlen(&buf)] - v51 == 16)
                {
                    I_strncpyz(dest, &buf, 5);
                    Dvar_SetStringByName("cdkey1", dest);
                    I_strncpyz(dest, src, 5);
                    Dvar_SetStringByName("cdkey2", dest);
                    I_strncpyz(dest, v53, 5);
                    Dvar_SetStringByName("cdkey3", dest);
                    I_strncpyz(dest, v54, 5);
                    Dvar_SetStringByName("cdkey4", dest);
                }
                if (strlen(buf2) == 4)
                {
                    I_strncpyz(dest, buf2, 5);
                    Dvar_SetStringByName("cdkey5", dest);
                }
            }
        }
        else if (ui_dedicated->current.integer
            || (Bool = Dvar_GetBool("sv_punkbuster"), v56 = Dvar_GetBool("cl_punkbuster"), !Bool)
            || v56)
        {
            Dvar_SetBoolByName("cg_thirdPerson", 0);
            v3 = va("%i", ui_dedicated->current.integer);
            Dvar_SetFromStringByNameFromSource("dedicated", v3, DVAR_SOURCE_EXTERNAL);
            Dvar_SetStringByName("g_gametype", (char *)sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameType);
            v4 = va(
                "wait ; wait ; map %s\n",
                (const char *)sharedUiInfo.serverHardwareIconList[40 * ui_currentNetMap->current.integer - 5119]);
            Cbuf_AddText(localClientNum, v4);
        }
        else
        {
            Menus_OpenByName(dc, "startpb_popmenu");
        }
    }
}

void __cdecl UI_ServersSort(int column, int force)
{
    if (force || *(unsigned int *)&sharedUiInfo.serverStatus.string[1112] != column)
    {
        *(unsigned int *)&sharedUiInfo.serverStatus.string[1112] = column;
        qsort(
            &sharedUiInfo.serverStatus.string[1132],
            *(unsigned int *)&sharedUiInfo.gap8EB4[72900],
            4u,
            (int(__cdecl *)(const void *, const void *))UI_ServersQsortCompare);
    }
}

int __cdecl UI_ServersQsortCompare(unsigned int *arg1, unsigned int *arg2)
{
    return LAN_CompareServers(
        ui_netSource->current.integer,
        *(int *)&sharedUiInfo.serverStatus.string[1112],
        *(int *)&sharedUiInfo.serverStatus.string[1116],
        *arg1,
        *arg2);
}

void UI_VerifyLanguage()
{
    int newLanguage; // [esp+0h] [ebp-Ch]
    int verifiedLanguage; // [esp+4h] [ebp-8h]
    int oldLanguage; // [esp+8h] [ebp-4h]

    oldLanguage = Dvar_GetInt("loc_language");
    newLanguage = Dvar_GetInt("ui_language");
    verifiedLanguage = SEH_VerifyLanguageSelection(newLanguage);
    if (verifiedLanguage != newLanguage)
        Dvar_SetIntByName("ui_language", verifiedLanguage);
    if (newLanguage == oldLanguage)
        Dvar_SetBoolByName("ui_languagechanged", 0);
    else
        Dvar_SetBoolByName("ui_languagechanged", 1);
}

void __cdecl UI_UpdateDisplayServers(uiInfo_s *uiInfo)
{
    int serverCount; // [esp+0h] [ebp-4h]

    serverCount = LAN_GetServerCount(ui_netSource->current.integer);
    if (*(unsigned int *)&sharedUiInfo.gap8EB4[72904] != serverCount)
    {
        *(unsigned int *)&sharedUiInfo.gap8EB4[72904] = serverCount;
        if (*(unsigned int *)&sharedUiInfo.gap8EB4[72900])
        {
            *(unsigned int *)&sharedUiInfo.serverStatus.string[1128] = -1;
            UI_BuildServerDisplayList(uiInfo, 1);
        }
    }
}

char __cdecl UI_GetOpenOrCloseMenuOnDvarArgs(
    const char **args,
    const char *cmd,
    char *dvarName,
    int dvarNameLen,
    char *testValue,
    int testValueLen,
    char *menuName,
    int menuNameLen)
{
    if (String_Parse(args, dvarName, dvarNameLen))
    {
        if (String_Parse(args, testValue, testValueLen))
        {
            if (String_Parse(args, menuName, menuNameLen))
            {
                return 1;
            }
            else
            {
                Com_Printf(13, "%s: invalid menu name.\n", cmd);
                return 0;
            }
        }
        else
        {
            Com_Printf(13, "%s: invalid test value.\n", cmd);
            return 0;
        }
    }
    else
    {
        Com_Printf(13, "%s: invalid dvar name.\n", cmd);
        return 0;
    }
}

void __cdecl UI_OpenMenuOnDvar(
    uiInfo_s *uiInfo,
    const char *cmd,
    const char *menuName,
    const char *dvarName,
    const char *testValue)
{
    bool wantMatch; // [esp+3h] [ebp-1h]

    if (!cmd)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 2570, 0, "%s", "cmd");
    if (I_stricmp(cmd, "openMenuOnDvar") && I_stricmp(cmd, "openMenuOnDvarNot"))
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            2571,
            0,
            "%s\n\t(cmd) = %s",
            "(!I_stricmp( cmd, \"openMenuOnDvar\" ) || !I_stricmp( cmd, \"openMenuOnDvarNot\" ))",
            cmd);
    if (!menuName)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 2572, 0, "%s", "menuName");
    wantMatch = I_stricmp(cmd, "openMenuOnDvar") == 0;
    if (UI_DvarValueTest(cmd, dvarName, testValue, wantMatch))
        Menus_OpenByName(&uiInfo->uiDC, menuName);
}

bool __cdecl UI_DvarValueTest(const char *cmd, const char *dvarName, const char *testValue, bool wantMatch)
{
    const char *dvarValue; // [esp+8h] [ebp-4h]

    if (!cmd)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 2549, 0, "%s", "cmd");
    if (!dvarName)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 2550, 0, "%s", "dvarName");
    if (!testValue)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 2551, 0, "%s", "testValue");
    if (Dvar_FindVar(dvarName))
    {
        dvarValue = Dvar_GetVariantString(dvarName);
        return (I_stricmp(testValue, dvarValue) == 0) == wantMatch;
    }
    else
    {
        Com_Printf(13, "%s: cannot find dvar %s\n", cmd, dvarName);
        return 0;
    }
}

void __cdecl UI_CloseMenuOnDvar(
    uiInfo_s *uiInfo,
    const char *cmd,
    const char *menuName,
    const char *dvarName,
    const char *testValue)
{
    bool wantMatch; // [esp+3h] [ebp-1h]

    if (!cmd)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 2584, 0, "%s", "cmd");
    if (I_stricmp(cmd, "closeMenuOnDvar") && I_stricmp(cmd, "closeMenuOnDvarNot"))
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            2585,
            0,
            "%s\n\t(cmd) = %s",
            "(!I_stricmp( cmd, \"closeMenuOnDvar\" ) || !I_stricmp( cmd, \"closeMenuOnDvarNot\" ))",
            cmd);
    if (!menuName)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 2586, 0, "%s", "menuName");
    wantMatch = I_stricmp(cmd, "closeMenuOnDvar") == 0;
    if (UI_DvarValueTest(cmd, dvarName, testValue, wantMatch))
        Menus_CloseByName(&uiInfo->uiDC, menuName);
}

void __cdecl UI_RemoveServerFromDisplayList(int num)
{
    int j; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < *(int *)&sharedUiInfo.gap8EB4[72900]; ++i)
    {
        if (*(_DWORD *)&sharedUiInfo.gap8EB4[4 * i - 7100] == num)
        {
            --*(_DWORD *)&sharedUiInfo.gap8EB4[72900];
            for (j = i; j < *(int *)&sharedUiInfo.gap8EB4[72900]; ++j)
                *(_DWORD *)&sharedUiInfo.gap8EB4[4 * j - 7100] = *(_DWORD *)&sharedUiInfo.gap8EB4[4 * j - 7096];
            return;
        }
    }
}

int numclean;
void __cdecl UI_BuildServerDisplayList(uiInfo_s *uiInfo, int force)
{
    char *String; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    const char *v14; // eax
    const char *v15; // eax
    const char *gameType; // [esp-4h] [ebp-458h]
    const char *basedir; // [esp-4h] [ebp-458h]
    char v18; // [esp+13h] [ebp-441h]
    _BYTE *v19; // [esp+18h] [ebp-43Ch]
    char *v20; // [esp+1Ch] [ebp-438h]
    int hardware; // [esp+30h] [ebp-424h]
    char info[1024]; // [esp+34h] [ebp-420h] BYREF
    int ping; // [esp+438h] [ebp-1Ch]
    int maxClients; // [esp+43Ch] [ebp-18h]
    int len; // [esp+440h] [ebp-14h]
    int dirty; // [esp+444h] [ebp-10h]
    int i; // [esp+448h] [ebp-Ch]
    int clients; // [esp+44Ch] [ebp-8h]
    int count; // [esp+450h] [ebp-4h]

    if (force || uiInfo->uiDC.realTime > *(int *)&sharedUiInfo.gap8EB4[72912])
    {
        if (force == 2)
            force = 0;
        String = (char *)Dvar_GetString("cl_motdString");
        I_strncpyz((char*)&sharedUiInfo.gap8EB4[72944], String, 1024);
        len = strlen((char*)&sharedUiInfo.gap8EB4[72944]);
        if (!len)
        {
            v3 = UI_SafeTranslateString("EXE_COD_MULTIPLAYER");
            v20 = va("%s - %s", v3, "1.0");
            v19 = &sharedUiInfo.gap8EB4[72944];
            do
            {
                v18 = *v20;
                *v19++ = *v20++;
            } while (v18);
            len = strlen((char *)&sharedUiInfo.gap8EB4[72944]);
        }
        if (len != *(unsigned int *)&sharedUiInfo.gap8EB4[72920])
        {
            *(unsigned int *)&sharedUiInfo.gap8EB4[72920] = len;
            *(unsigned int *)&sharedUiInfo.gap8EB4[72924] = -1;
        }
        if (force)
        {
            numclean = 0;
            UI_ClearDisplayedServers();
            if (*(int *)&sharedUiInfo.serverStatus.string[1128] >= 0)
                Menu_SetFeederSelection(&uiInfo->uiDC, 0, 2, 0, 0);
            LAN_MarkServerDirty(ui_netSource->current.integer, 0xFFFFFFFF, 1u);
        }
        count = LAN_GetServerCount(ui_netSource->current.integer);
        if (!LAN_WaitServerResponse(ui_netSource->current.integer) && (ui_netSource->current.integer || count))
        {
            UI_ServersSort(*(int *)&sharedUiInfo.serverStatus.string[1112], 1);
            dirty = 0;
            for (i = 0; i < count; ++i)
            {
                if (LAN_ServerIsDirty(ui_netSource->current.integer, i))
                {
                    dirty = 1;
                    ping = LAN_GetServerPing(ui_netSource->current.integer, i);
                    if (ping > 0 || ui_netSource->current.integer == 2)
                    {
                        LAN_GetServerInfo(ui_netSource->current.integer, i, info, 1024);
                        v4 = Info_ValueForKey(info, "clients");
                        clients = atoi(v4);
                        *(unsigned int *)&sharedUiInfo.gap8EB4[72908] += clients;
                        v5 = Info_ValueForKey(info, "addr");
                        if (!I_strnicmp(v5, "000.000.000.000", 15) || !ui_browserShowEmpty->current.enabled && !clients)
                            goto LABEL_55;
                        if (!ui_browserShowFull->current.enabled)
                        {
                            v6 = Info_ValueForKey(info, "sv_maxclients");
                            maxClients = atoi(v6);
                            if (clients == maxClients)
                                goto LABEL_55;
                        }
                        if (ui_browserShowPassword->current.integer >= 0)
                        {
                            v7 = Info_ValueForKey(info, "pswrd");
                            if (atoi(v7) != ui_browserShowPassword->current.integer)
                                goto LABEL_55;
                        }
                        if (ui_browserShowPure->current.enabled)
                        {
                            v8 = Info_ValueForKey(info, "pure");
                            if (!atoi(v8))
                                goto LABEL_55;
                        }
                        if (ui_browserShowDedicated->current.enabled)
                        {
                            v9 = Info_ValueForKey(info, "hw");
                            hardware = atoi(v9);
                            if (hardware != 1 && hardware != 2 && hardware != 3)
                                goto LABEL_55;
                        }
                        if (ui_browserMod->current.integer >= 0)
                        {
                            v10 = Info_ValueForKey(info, "mod");
                            if (atoi(v10) != ui_browserMod->current.integer)
                                goto LABEL_55;
                        }
                        if ((ui_browserFriendlyfire->current.integer < 0
                            || (v11 = Info_ValueForKey(info, "ff"), atoi(v11) == ui_browserFriendlyfire->current.integer))
                            && (ui_browserKillcam->current.integer < 0
                                || (v12 = Info_ValueForKey(info, "kc"), atoi(v12) == ui_browserKillcam->current.integer))
                            && (ui_browserShowPunkBuster->current.integer < 0
                                || (v13 = Info_ValueForKey(info, "pb"), atoi(v13) == ui_browserShowPunkBuster->current.integer))
                            && (!*sharedUiInfo.joinGameTypes[ui_joinGameType->current.integer].gameTypeName
                                || (gameType = sharedUiInfo.joinGameTypes[ui_joinGameType->current.integer].gameType,
                                    v14 = Info_ValueForKey(info, "gametype"),
                                    !I_stricmp(v14, gameType)))
                            && (ui_serverFilterType <= 0
                                || (basedir = serverFilters[ui_serverFilterType].basedir,
                                    v15 = Info_ValueForKey(info, "game"),
                                    !I_stricmp(v15, basedir))))
                        {
                            if (ui_netSource->current.integer == 2)
                                UI_RemoveServerFromDisplayList(i);
                            UI_BinaryServerInsertion(i);
                            if (ping > 0)
                            {
                                LAN_MarkServerDirty(ui_netSource->current.integer, i, 0);
                                ++numclean;
                            }
                        }
                        else
                        {
                        LABEL_55:
                            LAN_MarkServerDirty(ui_netSource->current.integer, i, 0);
                        }
                    }
                }
            }
            *(unsigned int *)&sharedUiInfo.serverStatus.string[1104] = uiInfo->uiDC.realTime;
        }
        else
        {
            UI_ClearDisplayedServers();
            *(unsigned int *)&sharedUiInfo.gap8EB4[72912] = uiInfo->uiDC.realTime + 500;
        }
    }
}

void __cdecl UI_BinaryServerInsertion(unsigned int num)
{
    int offset; // [esp+0h] [ebp-10h]
    int len; // [esp+4h] [ebp-Ch]
    int res; // [esp+8h] [ebp-8h]
    int mid; // [esp+Ch] [ebp-4h]

    len = *(unsigned int *)&sharedUiInfo.gap8EB4[72900];
    mid = *(unsigned int *)&sharedUiInfo.gap8EB4[72900];
    offset = 0;
    res = 0;
    while (mid > 0)
    {
        mid = len >> 1;
        res = LAN_CompareServers(
            ui_netSource->current.integer,
            *(int *)&sharedUiInfo.serverStatus.string[1112],
            *(int *)&sharedUiInfo.serverStatus.string[1116],
            num,
            *(unsigned int *)&sharedUiInfo.gap8EB4[4 * (len >> 1) - 7100 + 4 * offset]);
        if (res != -LAN_CompareServers(
            ui_netSource->current.integer,
            *(int *)&sharedUiInfo.serverStatus.string[1112],
            *(int *)&sharedUiInfo.serverStatus.string[1116],
            *(unsigned int *)&sharedUiInfo.gap8EB4[4 * (len >> 1) - 7100 + 4 * offset],
            num))
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                3320,
                0,
                "%s",
                "res == -LAN_CompareServers( ui_netSource->current.integer, sharedUiInfo.serverStatus.sortKey, sharedUiInfo.serve"
                "rStatus.sortDir, sharedUiInfo.serverStatus.displayServers[offset + mid], num )");
        if (!res)
        {
            UI_InsertServerIntoDisplayList(num, mid + offset);
            return;
        }
        if (res > 0)
            offset += mid;
        len -= mid;
    }
    if (res > 0)
        ++offset;
    UI_InsertServerIntoDisplayList(num, offset);
}

void __cdecl UI_InsertServerIntoDisplayList(unsigned int num, int position)
{
    int i; // [esp+0h] [ebp-8h]
    int res; // [esp+4h] [ebp-4h]
    int resa; // [esp+4h] [ebp-4h]

    if (position < 0)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 3238, 0, "%s", "position >= 0");
    if (position < *(int *)&sharedUiInfo.gap8EB4[72900])
    {
        res = LAN_CompareServers(
            ui_netSource->current.integer,
            *(int *)&sharedUiInfo.serverStatus.string[1112],
            *(int *)&sharedUiInfo.serverStatus.string[1116],
            num,
            *(unsigned int *)&sharedUiInfo.gap8EB4[4 * position - 7100]);
        if (res != -LAN_CompareServers(
            ui_netSource->current.integer,
            *(int *)&sharedUiInfo.serverStatus.string[1112],
            *(int *)&sharedUiInfo.serverStatus.string[1116],
            *(unsigned int *)&sharedUiInfo.gap8EB4[4 * position - 7100],
            num))
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                3243,
                0,
                "%s",
                "res == -LAN_CompareServers( ui_netSource->current.integer, sharedUiInfo.serverStatus.sortKey, sharedUiInfo.serve"
                "rStatus.sortDir, sharedUiInfo.serverStatus.displayServers[position], num )");
        if (res > 0)
            MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 3244, 0, "%s", "res <= 0");
    }
    if (position > 0)
    {
        resa = LAN_CompareServers(
            ui_netSource->current.integer,
            *(int *)&sharedUiInfo.serverStatus.string[1112],
            *(int *)&sharedUiInfo.serverStatus.string[1116],
            num,
            *(unsigned int *)&sharedUiInfo.gap8EB4[4 * position - 7104]);
        if (resa != -LAN_CompareServers(
            ui_netSource->current.integer,
            *(int *)&sharedUiInfo.serverStatus.string[1112],
            *(int *)&sharedUiInfo.serverStatus.string[1116],
            *(unsigned int *)&sharedUiInfo.gap8EB4[4 * position - 7104],
            num))
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                3249,
                0,
                "%s",
                "res == -LAN_CompareServers( ui_netSource->current.integer, sharedUiInfo.serverStatus.sortKey, sharedUiInfo.serve"
                "rStatus.sortDir, sharedUiInfo.serverStatus.displayServers[position - 1], num )");
        if (resa < 0)
            MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 3250, 0, "%s", "res >= 0");
    }
    if (position >= 0 && position <= *(int *)&sharedUiInfo.gap8EB4[72900])
    {
        if (position <= *(int *)&sharedUiInfo.serverStatus.string[1128] && *(unsigned int *)&sharedUiInfo.gap8EB4[72900])
            ++*(unsigned int *)&sharedUiInfo.serverStatus.string[1128];
        for (i = ++ * (unsigned int *)&sharedUiInfo.gap8EB4[72900]; i > position; --i)
            *(unsigned int *)&sharedUiInfo.gap8EB4[4 * i - 7100] = *(unsigned int *)&sharedUiInfo.gap8EB4[4 * i - 7104];
        *(unsigned int *)&sharedUiInfo.gap8EB4[4 * position - 7100] = num;
    }
}

int UI_ClearDisplayedServers()
{
    int result; // eax

    *(unsigned int *)&sharedUiInfo.gap8EB4[72900] = 0;
    *(unsigned int *)&sharedUiInfo.gap8EB4[72908] = 0;
    result = LAN_GetServerCount(ui_netSource->current.integer);
    *(unsigned int *)&sharedUiInfo.gap8EB4[72904] = result;
    return result;
}

void __cdecl UI_BuildServerStatus(uiInfo_s *uiInfo, int force)
{
    if (!uiInfo->nextFindPlayerRefresh)
    {
        if (force)
        {
            Menu_SetFeederSelection(&uiInfo->uiDC, 0, 13, 0, 0);
            sharedUiInfo.serverStatusInfo.numLines = 0;
            LAN_GetServerStatus(0, 0, 0);
        }
        else if (!sharedUiInfo.nextServerStatusRefresh || sharedUiInfo.nextServerStatusRefresh > uiInfo->uiDC.realTime)
        {
            return;
        }
        UI_UpdateDisplayServers(uiInfo);
        if (*(int *)&sharedUiInfo.serverStatus.string[1128] >= 0
            && *(int *)&sharedUiInfo.serverStatus.string[1128] <= *(int *)&sharedUiInfo.gap8EB4[72900]
            && *(unsigned int *)&sharedUiInfo.gap8EB4[72900])
        {
            if (UI_GetServerStatusInfo(sharedUiInfo.serverStatusAddress, &sharedUiInfo.serverStatusInfo))
            {
                sharedUiInfo.nextServerStatusRefresh = 0;
                UI_GetServerStatusInfo(sharedUiInfo.serverStatusAddress, 0);
            }
            else
            {
                sharedUiInfo.nextServerStatusRefresh = uiInfo->uiDC.realTime + 500;
            }
        }
    }
}

int __cdecl UI_MapCountByGameType()
{
    int c; // [esp+0h] [ebp-Ch]
    int game; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    game = ui_netGameType->current.integer;
    c = 0;
    for (i = 0; i < sharedUiInfo.mapCount; ++i)
    {
        sharedUiInfo.serverHardwareIconList[40 * i - 5081] = 0;
        if (((int)sharedUiInfo.serverHardwareIconList[40 * i - 5115] & (1 << game)) != 0)
        {
            ++c;
            sharedUiInfo.serverHardwareIconList[40 * i - 5081] = (Material *)1;
        }
    }
    return c;
}

int __cdecl UI_FeederCount(int localClientNum, float feederID)
{
    if (feederID == 4.0)
        return UI_MapCountByGameType();
    if (feederID == 9.0)
        return sharedUiInfo.modCount;
    if (feederID == 2.0)
    {
        if (localClientNum)
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                332,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        UI_UpdateDisplayServers(&uiInfoArray);
        return *(_DWORD *)&sharedUiInfo.gap8EB4[72900];
    }
    else if (feederID == 13.0)
    {
        return sharedUiInfo.serverStatusInfo.numLines;
    }
    else if (feederID == 7.0)
    {
        if (localClientNum)
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                332,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if (uiInfoArray.uiDC.realTime > uiInfoArray.playerRefresh)
        {
            uiInfoArray.playerRefresh = uiInfoArray.uiDC.realTime + 3000;
            UI_BuildPlayerList(localClientNum);
        }
        return sharedUiInfo.playerCount;
    }
    else if (feederID == 39.0)
    {
        return 0;
    }
    else if (feederID == 20.0)
    {
        if (localClientNum)
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                332,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if (uiInfoArray.uiDC.realTime > uiInfoArray.playerRefresh)
        {
            uiInfoArray.playerRefresh = uiInfoArray.uiDC.realTime + 3000;
            UI_BuildPlayerList(localClientNum);
        }
        return sharedUiInfo.playerCount;
    }
    else if (feederID == 24.0)
    {
        if (localClientNum)
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                332,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        return uiInfoArray.playerProfileCount;
    }
    else if (feederID == 29.0)
    {
        return sharedUiInfo.numCustomGameTypes;
    }
    else
    {
        return 0;
    }
}

void __cdecl UI_BuildPlayerList(int localClientNum)
{
    const char *v1; // eax
    char *info; // [esp+0h] [ebp-C44h]
    uiClientState_s state; // [esp+4h] [ebp-C40h] BYREF
    char szName[40]; // [esp+C10h] [ebp-34h] BYREF
    int n; // [esp+C3Ch] [ebp-8h]
    int count; // [esp+C40h] [ebp-4h]

    CL_GetClientState(localClientNum, &state);
    info = CL_GetConfigString(localClientNum, 0);
    v1 = Info_ValueForKey(info, "sv_maxclients");
    count = atoi(v1);
    memset((unsigned __int8 *)sharedUiInfo.playerClientNums, 0xFFu, sizeof(sharedUiInfo.playerClientNums));
    sharedUiInfo.playerCount = 0;
    for (n = 0; n < count; ++n)
    {
        if (CL_GetClientName(localClientNum, n, szName, 38))
        {
            sharedUiInfo.playerClientNums[sharedUiInfo.playerCount] = n;
            I_strncpyz(sharedUiInfo.playerNames[sharedUiInfo.playerCount], szName, 32);
            I_CleanStr(sharedUiInfo.playerNames[sharedUiInfo.playerCount]);
            ++sharedUiInfo.playerCount;
        }
    }
}

int __cdecl UI_GetClientNumForPlayerListNum(int playerListIndex)
{
    if (sharedUiInfo.playerClientNums[playerListIndex] >= 0x40u)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            1437,
            0,
            "%s\n\t(sharedUiInfo.playerClientNums[playerListIndex]) = %i",
            "(sharedUiInfo.playerClientNums[playerListIndex] >= 0 && sharedUiInfo.playerClientNums[playerListIndex] < 64)",
            sharedUiInfo.playerClientNums[playerListIndex]);
    return sharedUiInfo.playerClientNums[playerListIndex];
}

char info[1024];
int lastColumn;
int lastTime;
char clientBuff[38];

const char *__cdecl UI_FeederItemText(
    int localClientNum,
    itemDef_s *item,
    const float feederID,
    int index,
    unsigned int column,
    Material **handle)
{
    const char *result; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    const char *v14; // eax
    const char *v15; // eax
    const char *v16; // eax
    const char *v17; // eax
    unsigned int ClientNumForPlayerListNum; // eax
    const char *v19; // [esp-4h] [ebp-18h]
    unsigned int hardware; // [esp+4h] [ebp-10h]
    int ping; // [esp+8h] [ebp-Ch]
    int actual; // [esp+Ch] [ebp-8h] BYREF
    uiInfo_s *uiInfo; // [esp+10h] [ebp-4h]

    *handle = 0;
    if (feederID == 4.0)
        return UI_SelectedMap(index, &actual);
    if (feederID != 2.0)
    {
        if (feederID == 13.0)
        {
            if (index >= 0 && index < sharedUiInfo.serverStatusInfo.numLines && column < 4)
            {
                if (*sharedUiInfo.serverStatusInfo.lines[index][column] == 64)
                    return UI_SafeTranslateString((char *)sharedUiInfo.serverStatusInfo.lines[index][column] + 1);
                else
                    return (char *)sharedUiInfo.serverStatusInfo.lines[index][column];
            }
        }
        else if (feederID == 7.0)
        {
            if (index >= 0 && index < sharedUiInfo.playerCount)
                return sharedUiInfo.playerNames[index];
        }
        else if (feederID == 9.0)
        {
            if (index >= 0 && index < sharedUiInfo.modCount)
            {
                if (sharedUiInfo.modList[index].modDescr && *sharedUiInfo.modList[index].modDescr)
                    return (char *)sharedUiInfo.modList[index].modDescr;
                else
                    return (char *)sharedUiInfo.modList[index].modName;
            }
        }
        else
        {
            if (feederID == 39.0)
                return (char *)"";
            if (feederID == 20.0 && index >= 0 && index < sharedUiInfo.playerCount)
            {
                if (column == 1)
                    return sharedUiInfo.playerNames[index];
                ClientNumForPlayerListNum = UI_GetClientNumForPlayerListNum(index);
                if (CL_IsPlayerMuted(localClientNum, ClientNumForPlayerListNum))
                    return UI_SafeTranslateString("MP_MUTED");
            }
        }
        goto LABEL_73;
    }
    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    uiInfo = (uiInfo_s *)&uiInfoArray;
    UI_UpdateDisplayServers((uiInfo_s *)&uiInfoArray);
    if (index < 0 || index >= *(int *)&sharedUiInfo.gap8EB4[72900])
    {
    LABEL_73:
        if (feederID != 24.0)
            return (char *)"";
        if (localClientNum)
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                332,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        uiInfo = (uiInfo_s *)&uiInfoArray;
        if (index >= 0 && index < uiInfo->playerProfileCount)
            return (char *)uiInfo->playerProfileName[uiInfo->playerProfileStatus.displayProfile[index]];
        else
            return (char *)"";
    }
    if (lastColumn != column || lastTime > uiInfo->uiDC.realTime + 5000)
    {
        LAN_GetServerInfo(ui_netSource->current.integer, *(unsigned int *)&sharedUiInfo.gap8EB4[4 * index - 7100], info, 1024);
        lastColumn = column;
        lastTime = uiInfo->uiDC.realTime;
    }
    v7 = Info_ValueForKey(info, "ping");
    ping = atoi(v7);
    switch (column)
    {
    case 0u:
        v8 = Info_ValueForKey(info, "pswrd");
        if (atoi(v8))
            result = "X";
        else
            result = (char *)"";
        break;
    case 1u:
        v9 = Info_ValueForKey(info, "hw");
        hardware = atoi(v9);
        if (hardware <= 9)
            *handle = sharedUiInfo.serverHardwareIconList[hardware];
        result = (char *)"";
        break;
    case 2u:
        if (ping > 0)
        {
            v14 = Info_ValueForKey(info, "hostname");
            I_strncpyz(clientBuff, v14, 38);
            result = clientBuff;
        }
        else
        {
            result = Info_ValueForKey(info, "addr");
        }
        break;
    case 3u:
        v15 = Info_ValueForKey(info, "mapname");
        result = UI_GetMapDisplayName(v15);
        break;
    case 4u:
        v19 = Info_ValueForKey(info, "sv_maxclients");
        v16 = Info_ValueForKey(info, "clients");
        Com_sprintf(clientBuff, 0x26u, "%s (%s)", v16, v19);
        result = clientBuff;
        break;
    case 5u:
        if (Info_ValueForKey(info, "gametype") && *Info_ValueForKey(info, "gametype"))
        {
            v17 = Info_ValueForKey(info, "gametype");
            result = UI_GetGameTypeDisplayName(v17);
        }
        else
        {
            result = "?";
        }
        break;
    case 6u:
        v12 = Info_ValueForKey(info, "voice");
        if (atoi(v12))
            result = "X";
        else
            result = (char *)"";
        break;
    case 7u:
        v10 = Info_ValueForKey(info, "pure");
        if (atoi(v10))
            result = "X";
        else
            result = (char *)"";
        break;
    case 8u:
        v11 = Info_ValueForKey(info, "mod");
        if (atoi(v11))
            result = (char *)"";
        else
            result = "X";
        break;
    case 9u:
        v13 = Info_ValueForKey(info, "pb");
        if (atoi(v13))
            result = "X";
        else
            result = (char *)"";
        break;
    case 0xAu:
        if (ping > 0)
            result = Info_ValueForKey(info, "ping");
        else
            result = "...";
        break;
    default:
        goto LABEL_73;
    }
    return result;
}

Material *__cdecl UI_GetLevelShot(int index)
{
    if (index < 0 || index >= sharedUiInfo.mapCount)
        index = 0;
    if (!sharedUiInfo.serverHardwareIconList[40 * index - 5082])
        sharedUiInfo.serverHardwareIconList[40 * index - 5082] = Material_RegisterHandle(
            (char *)sharedUiInfo.serverHardwareIconList[40 * index - 5118],
            3);
    return sharedUiInfo.serverHardwareIconList[40 * index - 5082];
}

Material *__cdecl UI_FeederItemImage(float feederID, int index)
{
    int actual; // [esp+0h] [ebp-4h] BYREF

    if (feederID != 4.0)
        return 0;
    UI_SelectedMap(index, &actual);
    return UI_GetLevelShot(actual);
}

void __cdecl UI_FeederItemColor(
    int localClientNum,
    itemDef_s *item,
    float feederID,
    int index,
    int column,
    float *color)
{
    listBoxDef_s *listPtr; // [esp+10h] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    if (!listPtr)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 4704, 0, "%s", "listPtr");
    if (CL_GetLocalClientActiveCount())
    {
        *color = item->window.foreColor[0];
        color[1] = item->window.foreColor[1];
        color[2] = item->window.foreColor[2];
        color[3] = item->window.foreColor[3];
    }
    else
    {
        *color = listPtr->disableColor[0];
        color[1] = listPtr->disableColor[1];
        color[2] = listPtr->disableColor[2];
        color[3] = listPtr->disableColor[3];
    }
}

int __cdecl UI_GetListIndexFromMapIndex(int testMapIndex)
{
    int listIndex; // [esp+0h] [ebp-8h]
    int mapIndex; // [esp+4h] [ebp-4h]

    if (testMapIndex < 0 || testMapIndex >= sharedUiInfo.mapCount)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            4153,
            0,
            "%s\n\t(testMapIndex) = %i",
            "(testMapIndex >= 0 && testMapIndex < sharedUiInfo.mapCount)",
            testMapIndex);
    listIndex = 0;
    for (mapIndex = 0; mapIndex < sharedUiInfo.mapCount; ++mapIndex)
    {
        if (sharedUiInfo.serverHardwareIconList[40 * mapIndex - 5081])
        {
            if (mapIndex == testMapIndex)
                return listIndex;
            ++listIndex;
        }
    }
    return 0;
}

void __cdecl UI_OverrideCursorPos(int localClientNum, itemDef_s *item)
{
    int v2; // [esp+4h] [ebp-1Ch]
    int v3; // [esp+Ch] [ebp-14h]
    int max; // [esp+10h] [ebp-10h]
    int delta; // [esp+14h] [ebp-Ch]
    listBoxDef_s *listPtr; // [esp+18h] [ebp-8h]

    if (item->special == 4.0)
    {
        item->cursorPos[localClientNum] = UI_GetListIndexFromMapIndex(ui_currentNetMap->current.integer);
    }
    else if (item->special != 39.0 && item->special == 2.0)
    {
        listPtr = item->typeData.listBox;
        if (listPtr->endPos[localClientNum])
        {
            if (*(int *)&sharedUiInfo.serverStatus.string[1128] >= 0
                && item->cursorPos[localClientNum] >= listPtr->startPos[localClientNum]
                && item->cursorPos[localClientNum] <= listPtr->endPos[localClientNum])
            {
                delta = *(unsigned int *)&sharedUiInfo.serverStatus.string[1128] - item->cursorPos[localClientNum];
                max = Item_ListBox_MaxScroll(localClientNum, item);
                if (delta + listPtr->startPos[localClientNum] < max)
                    v3 = delta + listPtr->startPos[localClientNum];
                else
                    v3 = max;
                if (v3 > 0)
                    v2 = v3;
                else
                    v2 = 0;
                listPtr->startPos[localClientNum] = v2;
                listPtr->endPos[localClientNum] += delta;
                item->cursorPos[localClientNum] = *(unsigned int *)&sharedUiInfo.serverStatus.string[1128];
            }
        }
        else
        {
            item->cursorPos[localClientNum] = -1;
        }
    }
}

char *__cdecl UI_SelectedMap(int index, int *actual)
{
    int c; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    c = 0;
    *actual = 0;
    for (i = 0; i < sharedUiInfo.mapCount; ++i)
    {
        if (sharedUiInfo.serverHardwareIconList[40 * i - 5081])
        {
            if (c == index)
            {
                *actual = i;
                return UI_SafeTranslateString((char *)sharedUiInfo.mapList[i].mapName);
            }
            ++c;
        }
    }
    return (char *)"";
}

char info_0[1024];
void __cdecl UI_FeederSelection(int localClientNum, float feederID, int index)
{
    int actual; // [esp+8h] [ebp-8h] BYREF
    uiInfo_s *uiInfo; // [esp+Ch] [ebp-4h]

    if (index < 0)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 4915, 0, "%s", "index >= 0");
    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    uiInfo = (uiInfo_s *)&uiInfoArray;
    if (feederID == 4.0)
    {
        UI_SelectedMap(index, &actual);
        Dvar_SetInt((dvar_s *)ui_currentMap, actual);
        Dvar_SetInt((dvar_s *)ui_currentNetMap, actual);
    }
    else if (feederID == 2.0)
    {
        if (*(int *)&sharedUiInfo.gap8EB4[72900] > 0)
            *(unsigned int *)&sharedUiInfo.serverStatus.string[1128] = index;
        LAN_GetServerInfo(ui_netSource->current.integer, *(unsigned int *)&sharedUiInfo.gap8EB4[4 * index - 7100], info_0, 1024);
    }
    else if (feederID == 7.0)
    {
        uiInfo->playerIndex = index;
    }
    else if (feederID != 39.0)
    {
        if (feederID == 9.0)
        {
            sharedUiInfo.modIndex = index;
        }
        else if (feederID == 20.0)
        {
            uiInfo->playerIndex = index;
        }
        else if (feederID == 24.0 && index >= 0 && index < uiInfo->playerProfileCount)
        {
            Dvar_SetString(
                (dvar_s *)ui_playerProfileSelected,
                (char *)uiInfo->playerProfileName[uiInfo->playerProfileStatus.displayProfile[index]]);
        }
    }
}

void UI_GetGameTypesList_LoadObj()
{
    char *v0; // eax
    unsigned int v1; // [esp+0h] [ebp-1030h]
    char *p; // [esp+10h] [ebp-1020h]
    char *data_p; // [esp+18h] [ebp-1018h] BYREF
    char *v4; // [esp+1Ch] [ebp-1014h]
    char listbuf[4096]; // [esp+20h] [ebp-1010h] BYREF
    int i; // [esp+1024h] [ebp-Ch]
    char *MenuBuffer; // [esp+1028h] [ebp-8h]
    int FileList; // [esp+102Ch] [ebp-4h]

    FileList = FS_GetFileList("maps/mp/gametypes", "gsc", FS_LIST_PURE_ONLY, listbuf, 4096);
    p = listbuf;
    for (i = 0; i < FileList; ++i)
    {
        v1 = strlen(p);
        if (*p == 95)
        {
            p += v1 + 1;
        }
        else
        {
            if (!I_stricmp(&p[v1 - 4], ".gsc"))
                p[v1 - 4] = 0;
            if (sharedUiInfo.numGameTypes == 32 || sharedUiInfo.numJoinGameTypes == 32)
            {
                Com_Printf(13, "Too many game type scripts found! Only loading the first %i\n", 31);
                return;
            }
            sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameType = String_Alloc(p);
            sharedUiInfo.joinGameTypes[sharedUiInfo.numJoinGameTypes].gameType = sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameType;
            v0 = va("maps/mp/gametypes/%s.txt", p);
            MenuBuffer = GetMenuBuffer(v0);
            data_p = MenuBuffer;
            if (MenuBuffer)
            {
                v4 = (char *)Com_Parse((const char **)&data_p);
                sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameTypeName = String_Alloc(v4);
            }
            else
            {
                sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameTypeName = sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameType;
            }
            sharedUiInfo.joinGameTypes[sharedUiInfo.numJoinGameTypes++].gameTypeName = sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes++].gameTypeName;
            p += v1 + 1;
        }
    }
}

void UI_GetGameTypesList()
{
    sharedUiInfo.numGameTypes = 0;
    sharedUiInfo.numCustomGameTypes = 0;
    sharedUiInfo.numJoinGameTypes = 0;
    sharedUiInfo.joinGameTypes[0].gameType = String_Alloc("All");
    sharedUiInfo.joinGameTypes[sharedUiInfo.numJoinGameTypes++].gameTypeName = "";
    if (useFastFile->current.enabled)
        ((void(__cdecl *)(void (*)()))UI_GetGameTypesList_FastFile)(UI_GetGameTypesList_FastFile);
    else
        ((void(__cdecl *)(void (*)()))UI_GetGameTypesList_LoadObj)(UI_GetGameTypesList_LoadObj);
    if (!sharedUiInfo.numGameTypes)
        Com_Error(ERR_FATAL, "No game type scripts found in maps/mp/gametypes folder");
}

void UI_GetGameTypesList_FastFile()
{
    char *v0; // eax
    parseInfo_t *pszFileName; // [esp+4h] [ebp-18h]
    const char *pBuffParse; // [esp+8h] [ebp-14h] BYREF
    const char *pToken; // [esp+Ch] [ebp-10h]
    RawFile *gametypesFile; // [esp+10h] [ebp-Ch]
    char *pBuff; // [esp+14h] [ebp-8h]
    const char *gametypesBuf; // [esp+18h] [ebp-4h] BYREF

    gametypesFile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, "maps/mp/gametypes/_gametypes.txt").rawfile;
    if (gametypesFile)
    {
        gametypesBuf = gametypesFile->buffer;
        while (1)
        {
            pszFileName = Com_Parse(&gametypesBuf);
            if (!gametypesBuf)
                break;
            if (sharedUiInfo.numGameTypes == 32 || sharedUiInfo.numJoinGameTypes == 32)
            {
                Com_Printf(13, "Too many game type scripts found! Only loading the first %i\n", 31);
                return;
            }
            sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameType = String_Alloc(pszFileName->token);
            sharedUiInfo.joinGameTypes[sharedUiInfo.numJoinGameTypes].gameType = sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameType;
            v0 = va("maps/mp/gametypes/%s.txt", pszFileName->token);
            pBuff = GetMenuBuffer(v0);
            pBuffParse = pBuff;
            if (pBuff)
            {
                pToken = (const char *)Com_Parse(&pBuffParse);
                sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameTypeName = String_Alloc(pToken);
            }
            else
            {
                sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameTypeName = sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes].gameType;
            }
            sharedUiInfo.joinGameTypes[sharedUiInfo.numJoinGameTypes++].gameTypeName = sharedUiInfo.gameTypes[sharedUiInfo.numGameTypes++].gameTypeName;
        }
    }
}

void __cdecl UI_Pause(int localClientNum, int b)
{
    if (b)
    {
        Dvar_SetIntByName("cl_paused", 1);
        Key_SetCatcher(localClientNum, 16);
    }
    else
    {
        Key_RemoveCatcher(localClientNum, -17);
        Key_ClearStates(localClientNum);
        Dvar_SetIntByName("cl_paused", 0);
    }
}

void __cdecl UI_OpenMenu_f()
{
    char name[68]; // [esp+4h] [ebp-48h] BYREF

    Cmd_ArgsBuffer(1, name, 64);
    Menus_OpenByName(&uiInfoArray.uiDC, name);
}

void __cdecl UI_ListMenus_f()
{
    Menus_PrintAllLoadedMenus(&uiInfoArray.uiDC);
}

void __cdecl CL_SelectStringTableEntryInDvar_f()
{
    const char *v0; // eax
    unsigned int v1; // eax
    const char *v2; // eax
    int v3; // eax
    const char *v4; // eax
    char *ColumnValueForRow; // [esp-4h] [ebp-18h]
    double rowCount; // [esp+4h] [ebp-10h]
    const StringTable *table; // [esp+Ch] [ebp-8h] BYREF
    int row; // [esp+10h] [ebp-4h]

    if (!r_reflectionProbeGenerate->current.enabled)
    {
        if (Cmd_Argc() >= 4)
        {
            v0 = Cmd_Argv(1);
            StringTable_GetAsset(v0, (XAssetHeader *)&table);
            v1 = Sys_Milliseconds();
            srand(v1);
            rowCount = (double)table->rowCount;
            row = (int)((double)rand() * rowCount / 32767.0);
            v2 = Cmd_Argv(2);
            v3 = atoi(v2);
            ColumnValueForRow = (char *)StringTable_GetColumnValueForRow(table, row, v3);
            v4 = Cmd_Argv(3);
            Dvar_SetStringByName(v4, ColumnValueForRow);
        }
        else
        {
            Com_Printf(16, "usage: selectStringTableEntryInDvar <tableFileName> <columnNum> <dvarName>");
        }
    }
}

void __cdecl UI_CloseMenu_f()
{
    char name[68]; // [esp+4h] [ebp-48h] BYREF

    Cmd_ArgsBuffer(1, name, 64);
    Menus_CloseByName(&uiInfoArray.uiDC, name);
}

void __cdecl UI_LoadSoundAliases()
{
    Com_LoadSoundAliases("menu", "all_mp", SASYS_UI);
}

BOOL __cdecl LAN_LoadCachedServersInternal(int fileIn)
{
    int version; // [esp+0h] [ebp-8h] BYREF
    int size; // [esp+4h] [ebp-4h] BYREF

    if (FS_Read((unsigned __int8 *)&version, 4u, fileIn) != 4)
        return 0;
    if (version != 1)
        return 0;
    if (FS_Read((unsigned __int8 *)&cls.numglobalservers, 4u, fileIn) != 4)
        return 0;
    if (cls.numglobalservers >= 0x4E20u)
        return 0;
    if (FS_Read((unsigned __int8 *)&cls.numfavoriteservers, 4u, fileIn) != 4)
        return 0;
    if (cls.numfavoriteservers >= 0x80u)
        return 0;
    if (FS_Read((unsigned __int8 *)&size, 4u, fileIn) != 4)
        return 0;
    if (size != 2978944)
        return 0;
    if (FS_Read((unsigned __int8 *)cls.globalServers, 0x2D2A80u, fileIn) == 2960000)
        return FS_Read((unsigned __int8 *)cls.favoriteServers, 0x4A00u, fileIn) == 18944;
    return 0;
}

void __cdecl LAN_LoadCachedServers()
{
    int fileIn; // [esp+0h] [ebp-8h] BYREF
    int success; // [esp+4h] [ebp-4h]

    if (FS_SV_FOpenFileRead("servercache.dat", &fileIn)
        && (success = LAN_LoadCachedServersInternal(fileIn), FS_FCloseFile(fileIn), success))
    {
        CL_SortGlobalServers();
    }
    else
    {
        cls.numglobalservers = 0;
        cls.numfavoriteservers = 0;
    }
}

void __cdecl UI_Init(int localClientNum)
{
    const char *v1; // eax
    DvarLimits v2; // [esp-10h] [ebp-24h]
    bool v3; // [esp+0h] [ebp-14h]
    MenuList *menuList; // [esp+10h] [ebp-4h]
    MenuList *menuLista; // [esp+10h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    uiInfoArray.uiDC.localClientNum = localClientNum;
    g_ingameMenusLoaded[localClientNum] = 0;
    if (useFastFile->current.enabled)
        DB_ResetZoneSize(0);
    if (!useFastFile->current.enabled)
        UI_LoadSoundAliases();
    UI_RegisterDvars();
    uiInfoArray.allowScriptMenuResponse = 1;
    String_Init();
    Menu_Setup(&uiInfoArray.uiDC);
    CL_GetScreenDimensions(&uiInfoArray.uiDC.screenWidth, &uiInfoArray.uiDC.screenHeight, &uiInfoArray.uiDC.screenAspect);
    if (480 * uiInfoArray.uiDC.screenWidth <= 640 * uiInfoArray.uiDC.screenHeight)
        uiInfoArray.uiDC.bias = 0.0;
    else
        uiInfoArray.uiDC.bias = ((double)uiInfoArray.uiDC.screenWidth
            - (double)uiInfoArray.uiDC.screenHeight * 1.333333373069763)
        * 0.5;
    Sys_Milliseconds();
    UI_GetGameTypesList();
    if (sharedUiInfo.numGameTypes > 0x20u)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            6138,
            0,
            "%s",
            "sharedUiInfo.numGameTypes <= ARRAY_COUNT( sharedUiInfo.gameTypes )");
    v2.integer.max = sharedUiInfo.numGameTypes - 1;
    v2.enumeration.stringCount = 0;
    ui_netGameType = Dvar_RegisterInt("ui_netGametype", 0, v2, 1u, "Game type");
    UI_LoadArenas();
    if (useFastFile->current.enabled)
    {
        menuList = UI_LoadMenus((char*)"ui_mp/code.txt", 3);
        UI_AddMenuList(&uiInfoArray.uiDC, menuList);
    }
    if (!g_mapname[0] || !useFastFile->current.enabled)
    {
        menuLista = UI_LoadMenus((char *)"ui_mp/menus.txt", 3);
        UI_AddMenuList(&uiInfoArray.uiDC, menuLista);
    }
    if (g_mapname[0] && !useFastFile->current.enabled)
    {
        v1 = va("maps/mp/%s.csv", g_mapname);
        UI_MapLoadInfo(v1);
    }
    UI_AssetCache();
    Menus_CloseAll(&uiInfoArray.uiDC);
    sharedUiInfo.serverHardwareIconList[0] = Material_RegisterHandle("server_hardware_unknown", 3);
    sharedUiInfo.serverHardwareIconList[1] = Material_RegisterHandle("server_hardware_linux_dedicated", 3);
    sharedUiInfo.serverHardwareIconList[2] = Material_RegisterHandle("server_hardware_win_dedicated", 3);
    sharedUiInfo.serverHardwareIconList[3] = Material_RegisterHandle("server_hardware_mac_dedicated", 3);
    sharedUiInfo.serverHardwareIconList[6] = Material_RegisterHandle("server_hardware_win_listen", 3);
    sharedUiInfo.serverHardwareIconList[7] = Material_RegisterHandle("server_hardware_mac_listen", 3);
    LAN_LoadCachedServers(); // cl_ui_xenon_mp.obj
    UI_ServersSort(10, 0);
    v3 = Dvar_GetFloat("m_pitch") < 0.0;
    Dvar_SetBoolByName("ui_mousePitch", v3);
    if (ui_netGameType->current.integer >= 0x20u)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            6201,
            0,
            "%s\n\t(ui_netGameType->current.integer) = %i",
            "(ui_netGameType->current.integer >= 0 && ui_netGameType->current.integer < (sizeof( sharedUiInfo.gameTypes ) / (si"
            "zeof( sharedUiInfo.gameTypes[0] ) * (sizeof( sharedUiInfo.gameTypes ) != 4 || sizeof( sharedUiInfo.gameTypes[0] ) <= 4))))",
            ui_netGameType->current.integer);
    Dvar_SetString((dvar_s *)ui_netGameTypeName, (char *)sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameType);
    Dvar_RegisterBool("ui_multiplayer", 1, 0x40u, "True if the game is multiplayer");
    uiscript_debug = Dvar_RegisterInt(
        "uiscript_debug",
        0,
        (DvarLimits)0x200000000LL,
        0,
        "spam debug info for the ui script");
}

const dvar_s *UI_RegisterDvars()
{
    const dvar_s *result; // eax
    DvarLimits min; // [esp+10h] [ebp-10h]
    DvarLimits mina; // [esp+10h] [ebp-10h]
    DvarLimits minb; // [esp+10h] [ebp-10h]
    DvarLimits minc; // [esp+10h] [ebp-10h]
    DvarLimits mind; // [esp+10h] [ebp-10h]
    DvarLimits mine; // [esp+10h] [ebp-10h]
    DvarLimits minf; // [esp+10h] [ebp-10h]

    ui_customModeName = Dvar_RegisterString("ui_customModeName", (char *)"", 0, "Custom game mode name");
    ui_customModeEditName = Dvar_RegisterString(
        "ui_customModeEditName",
        (char *)"",
        0,
        "Name to give the currently edited custom game mode when editing is complete");
    ui_customClassName = Dvar_RegisterString("ui_customClassName", (char *)"", 0, "Custom Class name");
    Dvar_RegisterBool("g_allowvote", 1, 1u, 0);
    Dvar_RegisterBool("cg_brass", 1, 1u, 0);
    Dvar_RegisterBool("fx_marks", 1, 1u, 0);
    Dvar_RegisterString("server1", (char *)"", 1u, "Server display");
    Dvar_RegisterString("server2", (char *)"", 1u, "Server display");
    Dvar_RegisterString("server3", (char *)"", 1u, "Server display");
    Dvar_RegisterString("server4", (char *)"", 1u, "Server display");
    Dvar_RegisterString("server5", (char *)"", 1u, "Server display");
    Dvar_RegisterString("server6", (char *)"", 1u, "Server display");
    Dvar_RegisterString("server7", (char *)"", 1u, "Server display");
    Dvar_RegisterString("server8", (char *)"", 1u, "Server display");
    Dvar_RegisterString("server9", (char *)"", 1u, "Server display");
    Dvar_RegisterString("server10", (char *)"", 1u, "Server display");
    Dvar_RegisterString("server11", (char *)"", 1u, "Server display");
    Dvar_RegisterString("server12", (char *)"", 1u, "Server display");
    Dvar_RegisterString("server13", (char *)"", 1u, "Server display");
    Dvar_RegisterString("server14", (char *)"", 1u, "Server display");
    Dvar_RegisterString("server15", (char *)"", 1u, "Server display");
    Dvar_RegisterString("server16", (char *)"", 1u, "Server display");
    ui_netSource = Dvar_RegisterInt(
        "ui_netSource",
        1,
        (DvarLimits)0x200000000LL,
        1u,
        "The network source where:\n  0:Local\n  1:Internet\n  2:Favourites");
    min.value.max = 1.0;
    min.value.min = 0.0;
    ui_smallFont = Dvar_RegisterFloat("ui_smallFont", 0.25, min, 1u, "Small font scale");
    mina.value.max = 1.0;
    mina.value.min = 0.0;
    ui_bigFont = Dvar_RegisterFloat("ui_bigFont", 0.40000001, mina, 1u, "Big font scale");
    minb.value.max = 1.0;
    minb.value.min = 0.0;
    ui_extraBigFont = Dvar_RegisterFloat("ui_extraBigFont", 0.55000001, minb, 1u, "Extra big font scale");
    ui_currentMap = Dvar_RegisterInt("ui_currentMap", 0, (DvarLimits)0x7FFFFFFF00000000LL, 1u, "Current map index");
    ui_gametype = Dvar_RegisterInt("ui_gametype", 3, (DvarLimits)0x7FFFFFFF00000000LL, 1u, "Game type");
    ui_joinGameType = Dvar_RegisterInt("ui_joinGametype", 0, (DvarLimits)0x7FFFFFFF00000000LL, 1u, "Game join type");
    ui_netGameTypeName = Dvar_RegisterString("ui_netGametypeName", (char *)"", 1u, "Displayed game type name");
    ui_dedicated = Dvar_RegisterInt(
        "ui_dedicated",
        0,
        (DvarLimits)0x200000000LL,
        1u,
        "True if this is a dedicated server");
    ui_currentNetMap = Dvar_RegisterInt(
        "ui_currentNetMap",
        0,
        (DvarLimits)0x7FFFFFFF00000000LL,
        1u,
        "Currently running map");
    ui_browserShowFull = Dvar_RegisterBool("ui_browserShowFull", 1, 1u, "Show full servers");
    ui_browserShowEmpty = Dvar_RegisterBool("ui_browserShowEmpty", 1, 1u, "Show empty servers");
    ui_browserShowPassword = Dvar_RegisterInt(
        "ui_browserShowPassword",
        -1,
        (DvarLimits)0x1FFFFFFFFLL,
        1u,
        "Show servers that are password protected");
    ui_browserShowPure = Dvar_RegisterBool("ui_browserShowPure", 1, 1u, "Show pure servers only");
    ui_browserMod = Dvar_RegisterInt("ui_browserMod", 0, (DvarLimits)0x1FFFFFFFFLL, 1u, "UI Mod value");
    ui_browserShowDedicated = Dvar_RegisterBool("ui_browserShowDedicated", 0, 1u, "Show dedicated servers only");
    ui_browserFriendlyfire = Dvar_RegisterInt(
        "ui_browserFriendlyfire",
        -1,
        (DvarLimits)0x7FFFFFFF80000000LL,
        1u,
        "Friendly fire is active");
    ui_browserKillcam = Dvar_RegisterInt(
        "ui_browserKillcam",
        -1,
        (DvarLimits)0x7FFFFFFF80000000LL,
        1u,
        "Kill cam is active");
    ui_serverStatusTimeOut = Dvar_RegisterInt(
        "ui_serverStatusTimeOut",
        7000,
        (DvarLimits)0x7FFFFFFF00000000LL,
        1u,
        "Time in milliseconds before a server status request times out");
    ui_browserShowPunkBuster = Dvar_RegisterInt(
        "ui_browserShowPunkBuster",
        -1,
        (DvarLimits)0x7FFFFFFF80000000LL,
        1u,
        "Only show PunkBuster servers?");
    ui_playerProfileCount = Dvar_RegisterInt(
        "ui_playerProfileCount",
        0,
        (DvarLimits)0x7FFFFFFF80000000LL,
        0x40u,
        "Number of player profiles");
    ui_playerProfileSelected = Dvar_RegisterString(
        "ui_playerProfileSelected",
        (char *)"",
        0x40u,
        "Selected player profile name");
    ui_playerProfileNameNew = Dvar_RegisterString(
        "ui_playerProfileNameNew",
        (char *)"",
        0,
        "New player profile name");
    minc.value.max = 10000.0;
    minc.value.min = -10000.0;
    ui_buildLocation = Dvar_RegisterVec2("ui_buildLocation", -100.0, 52.0, minc, 0, "Where to draw the build number");
    mind.value.max = 1.0;
    mind.value.min = 0.0;
    ui_buildSize = Dvar_RegisterFloat("ui_buildSize", 0.30000001, mind, 0, "Font size to use for the build number");
    ui_showList = Dvar_RegisterBool("ui_showList", 0, 0x80u, "Show onscreen list of currently visible menus");
    ui_showMenuOnly = Dvar_RegisterString(
        "ui_showMenuOnly",
        (char *)"",
        0,
        "If set, only menus using this name will draw.");
    ui_showEndOfGame = Dvar_RegisterBool("ui_showEndOfGame", 0, 0, "Currently showing the end of game menu.");
    mine.value.max = 1.0;
    mine.value.min = 0.0;
    ui_borderLowLightScale = Dvar_RegisterFloat(
        "ui_borderLowLightScale",
        0.60000002,
        mine,
        0,
        "Scales the border color for the lowlight color on certain UI borders");
    ui_cinematicsTimestamp = Dvar_RegisterBool(
        "ui_cinematicsTimestamp",
        0,
        0,
        "Shows cinematics timestamp on subtitle UI elements.");
    minf.value.max = 1.0;
    minf.value.min = 0.0;
    ui_connectScreenTextGlowColor = Dvar_RegisterVec4(
        "ui_connectScreenTextGlowColor",
        0.30000001,
        0.60000002,
        0.30000001,
        1.0,
        minf,
        0,
        "Glow color applied to the mode and map name strings on the connect screen.");
    ui_drawCrosshair = Dvar_RegisterBool("ui_drawCrosshair", 1, 1u, "Whether to draw crosshairs.");
    ui_hud_hardcore = Dvar_RegisterBool(
        "ui_hud_hardcore",
        0,
        0x80u,
        "Whether the HUD should be suppressed for hardcore mode");
    ui_uav_allies = Dvar_RegisterBool("ui_uav_allies", 0, 0x80u, "Whether the UI should show UAV to allies");
    ui_uav_axis = Dvar_RegisterBool("ui_uav_axis", 0, 0x80u, "Whether the UI should show UAV to axis");
    ui_uav_client = Dvar_RegisterBool("ui_uav_client", 0, 0x80u, "Whether the UI should show UAV to this client");
    ui_allow_classchange = Dvar_RegisterBool(
        "ui_allow_classchange",
        0,
        0x80u,
        "Whether the UI should allow changing class");
    result = Dvar_RegisterBool("ui_allow_teamchange", 0, 0x80u, "Whether the UI should allow changing team");
    ui_allow_teamchange = result;
    return result;
}

Font_s *UI_AssetCache()
{
    Font_s *result; // eax

    sharedUiInfo.assets.whiteMaterial = Material_RegisterHandle("white", 3);
    sharedUiInfo.assets.scrollBar = Material_RegisterHandle("ui_scrollbar", 3);
    sharedUiInfo.assets.scrollBarArrowDown = Material_RegisterHandle("ui_scrollbar_arrow_dwn_a", 3);
    sharedUiInfo.assets.scrollBarArrowUp = Material_RegisterHandle("ui_scrollbar_arrow_up_a", 3);
    sharedUiInfo.assets.scrollBarArrowLeft = Material_RegisterHandle("ui_scrollbar_arrow_left", 3);
    sharedUiInfo.assets.scrollBarArrowRight = Material_RegisterHandle("ui_scrollbar_arrow_right", 3);
    sharedUiInfo.assets.scrollBarThumb = Material_RegisterHandle("ui_scrollbar_thumb", 3);
    sharedUiInfo.assets.sliderBar = Material_RegisterHandle("ui_slider2", 3);
    sharedUiInfo.assets.sliderThumb = Material_RegisterHandle("ui_sliderbutt_1", 3);
    sharedUiInfo.assets.cursor = Material_RegisterHandle("ui_cursor", 0);
    sharedUiInfo.assets.bigFont = CL_RegisterFont("fonts/bigfont", 0);
    sharedUiInfo.assets.smallFont = CL_RegisterFont("fonts/smallfont", 0);
    sharedUiInfo.assets.consoleFont = CL_RegisterFont("fonts/consolefont", 0);
    sharedUiInfo.assets.boldFont = CL_RegisterFont("fonts/boldfont", 0);
    sharedUiInfo.assets.textFont = CL_RegisterFont("fonts/normalfont", 0);
    sharedUiInfo.assets.extraBigFont = CL_RegisterFont("fonts/extrabigfont", 0);
    result = CL_RegisterFont("fonts/objectivefont", 0);
    sharedUiInfo.assets.objectiveFont = result;
    return result;
}

int bypassKeyClear;
void __cdecl UI_KeyEvent(int localClientNum, int key, int down)
{
    menuDef_t *menu; // [esp+8h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (Menu_Count(&uiInfoArray.uiDC))
    {
        menu = Menu_GetFocused(&uiInfoArray.uiDC);
        if (!menu)
            goto LABEL_25;
        if (Dvar_GetBool("cl_bypassMouseInput") || UI_GetActiveMenu(localClientNum) == UIMENU_SCOREBOARD)
            bypassKeyClear = 1;
        if (key == 27 && down && !Menus_AnyFullScreenVisible(&uiInfoArray.uiDC) && !menu->onESC)
            Menus_CloseAll(&uiInfoArray.uiDC);
        if (Key_IsCatcherActive(uiInfoArray.uiDC.localClientNum, 16))
            Menu_HandleKey(&uiInfoArray.uiDC, menu, key, down);
        if (!Menu_GetFocused(&uiInfoArray.uiDC))
        {
        LABEL_25:
            if (Key_IsCatcherActive(uiInfoArray.uiDC.localClientNum, 16))
            {
                Key_RemoveCatcher(localClientNum, -17);
                if (!bypassKeyClear)
                    Key_ClearStates(localClientNum);
                bypassKeyClear = 0;
                Dvar_SetIntByName("cl_paused", 0);
            }
        }
    }
}

uiMenuCommand_t __cdecl UI_GetActiveMenu(int localClientNum)
{
  if ( localClientNum )
    MyAssertHandler(
      ".\\ui_mp\\ui_main_mp.cpp",
      332,
      0,
      "%s\n\t(localClientNum) = %i",
      "(localClientNum == 0)",
      localClientNum);
  return uiInfoArray.currentMenuType;
}
const char *__cdecl UI_GetTopActiveMenuName(int localClientNum)
{
    int topMenuStackIndex; // [esp+4h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    topMenuStackIndex = uiInfoArray.uiDC.openMenuCount - 1;
    if (topMenuStackIndex < 0 || topMenuStackIndex >= uiInfoArray.uiDC.menuCount)
        return 0;
    if (!uiInfoArray.uiDC.menuStack[topMenuStackIndex])
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 6325, 0, "%s", "uiInfo->uiDC.menuStack[topMenuStackIndex]");
    return uiInfoArray.uiDC.menuStack[topMenuStackIndex]->window.name;
}

int __cdecl UI_SetActiveMenu(int localClientNum, uiMenuCommand_t menu)
{
    int result; // eax
    const char *v3; // eax
    const char *buf; // [esp+40h] [ebp-4h]
    const char *bufa; // [esp+40h] [ebp-4h]
    const char *bufb; // [esp+40h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (Menu_Count(&uiInfoArray.uiDC) <= 0)
        return 0;
    if (menu == UIMENU_SCRIPT_POPUP)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 6348, 0, "%s", "menu != UIMENU_SCRIPT_POPUP");
    uiInfoArray.currentMenuType = menu;
    switch (menu)
    {
    case UIMENU_NONE:
        Key_RemoveCatcher(localClientNum, -17);
        Dvar_SetIntByName("cl_paused", 0);
        Menus_CloseAll(&uiInfoArray.uiDC);
        result = 1;
        break;
    case UIMENU_MAIN:
        if (localClientNum)
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                6361,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        Key_SetCatcher(localClientNum, 16);
        Menus_OpenByName(&uiInfoArray.uiDC, "main");
        buf = Dvar_GetString("com_errorMessage");
        if (strlen(buf))
        {
            if (I_stricmp(buf, ";"))
                Menus_OpenByName(&uiInfoArray.uiDC, "error_popmenu");
        }
        SND_FadeAllSounds(1.0, 1000);
        result = 1;
        break;
    case UIMENU_INGAME:
        Key_SetCatcher(localClientNum, 16);
        Menus_CloseAll(&uiInfoArray.uiDC);
        v3 = CG_ScriptMainMenu(uiInfoArray.uiDC.localClientNum);
        if (!Menus_OpenByName(&uiInfoArray.uiDC, v3))
            Menus_OpenByName(&uiInfoArray.uiDC, "main");
        result = 1;
        break;
    case UIMENU_NEED_CD:
        Key_SetCatcher(localClientNum, 16);
        Menus_OpenByName(&uiInfoArray.uiDC, "needcd");
        result = 1;
        break;
    case UIMENU_BAD_CD_KEY:
        Key_SetCatcher(localClientNum, 16);
        Menus_OpenByName(&uiInfoArray.uiDC, "badcd");
        result = 1;
        break;
    case UIMENU_PREGAME:
        if (!*Dvar_GetString("com_errorMessage"))
            MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 6577, 0, "%s", "buf[0]");
        Key_SetCatcher(localClientNum, 16);
        Menus_CloseAll(&uiInfoArray.uiDC);
        Menus_OpenByName(&uiInfoArray.uiDC, "pregame_loaderror_mp");
        result = 1;
        break;
    case UIMENU_WM_QUICKMESSAGE:
        uiInfoArray.uiDC.cursor.x = 639.0;
        uiInfoArray.uiDC.cursor.y = 479.0;
        UI_SetSystemCursorPos(&uiInfoArray.uiDC, 639.0, 479.0);
        Key_SetCatcher(localClientNum, 16);
        CL_SetDisplayHUDWithKeycatchUI(uiInfoArray.uiDC.localClientNum, 1);
        Menus_CloseAll(&uiInfoArray.uiDC);
        Menus_OpenByName(&uiInfoArray.uiDC, "quickmessage");
        result = 1;
        break;
    case UIMENU_WM_AUTOUPDATE:
        Menus_OpenByName(&uiInfoArray.uiDC, "autoupdate");
        result = 1;
        break;
    case UIMENU_SCOREBOARD:
        Key_SetCatcher(localClientNum, 16);
        Menus_CloseAll(&uiInfoArray.uiDC);
        Menus_OpenByName(&uiInfoArray.uiDC, "scoreboard");
        bufa = Dvar_GetString("com_errorMessage");
        if (strlen(bufa) && I_stricmp(bufa, ";"))
            Menus_OpenByName(&uiInfoArray.uiDC, "error_popmenu");
        result = 1;
        break;
    case UIMENU_ENDOFGAME:
        Key_SetCatcher(localClientNum, 16);
        Menus_CloseAll(&uiInfoArray.uiDC);
        Menus_OpenByName(&uiInfoArray.uiDC, "endofgame");
        bufb = Dvar_GetString("com_errorMessage");
        if (strlen(bufb) && I_stricmp(bufb, ";"))
            Menus_OpenByName(&uiInfoArray.uiDC, "error_popmenu");
        result = 1;
        break;
    default:
        result = 0;
        break;
    }
    return result;
}

int __cdecl UI_IsFullscreen(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return Menus_AnyFullScreenVisible(&uiInfoArray.uiDC);
}

void __cdecl UI_ReadableSize(char *buf, unsigned int bufsize, int value)
{
    char *v3; // eax
    char *v4; // eax
    char *v5; // [esp-4h] [ebp-50h]
    char *v6; // [esp-4h] [ebp-50h]

    if (value <= 0x40000000)
    {
        if (value <= 0x100000)
        {
            if (value <= 1024)
            {
                v4 = UI_SafeTranslateString("EXE_BYTES");
                Com_sprintf(buf, bufsize, "%d %s", value, v4);
            }
            else
            {
                v3 = UI_SafeTranslateString("EXE_KILOBYTE");
                Com_sprintf(buf, bufsize, "%d %s", value / 1024, v3);
            }
        }
        else
        {
            Com_sprintf(buf, bufsize, "%d", value / 0x100000);
            v6 = UI_SafeTranslateString("EXE_MEGABYTE");
            Com_sprintf(
                &buf[strlen(buf)],
                bufsize - strlen(buf),
                ".%02d %s",
                (int)(100 * (value & 0x800FFFFF)) / 0x100000,
                v6);
        }
    }
    else
    {
        Com_sprintf(buf, bufsize, "%d", value / 0x40000000);
        v5 = UI_SafeTranslateString("EXE_GIGABYTE");
        Com_sprintf(
            &buf[strlen(buf)],
            bufsize - strlen(buf),
            ".%02d %s",
            (int)(100 * (value & 0xBFFFFFFF)) / 0x40000000,
            v5);
    }
}

void __cdecl UI_PrintTime(char *buf, unsigned int bufsize, int time)
{
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // [esp-4h] [ebp-4h]
    char *v7; // [esp-4h] [ebp-4h]

    if (time <= 3600)
    {
        if (time <= 60)
        {
            v5 = UI_SafeTranslateString("EXE_SECONDS");
            Com_sprintf(buf, bufsize, "%d %s", time, v5);
        }
        else
        {
            v7 = UI_SafeTranslateString("EXE_SECONDS");
            v4 = UI_SafeTranslateString("EXE_MINUTES");
            Com_sprintf(buf, bufsize, "%d %s %d %s", time / 60, v4, time % 60, v7);
        }
    }
    else
    {
        v6 = UI_SafeTranslateString("EXE_MINUTES");
        v3 = UI_SafeTranslateString("EXE_HOURS");
        Com_sprintf(buf, bufsize, "%d %s %d %s", time / 3600, v3, time % 3600 / 60, v6);
    }
}

int tleIndex;
void __cdecl UI_DisplayDownloadInfo(char *downloadName, float centerPoint, float yStart, Font_s *font, float scale)
{
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    char *v11; // eax
    char *v12; // eax
    char *v13; // eax
    char *v14; // eax
    char *v15; // eax
    float x; // [esp+0h] [ebp-1CCh]
    float v17; // [esp+8h] [ebp-1C4h]
    float v18; // [esp+8h] [ebp-1C4h]
    float v19; // [esp+8h] [ebp-1C4h]
    char *v20; // [esp+14h] [ebp-1B8h]
    char *v21; // [esp+14h] [ebp-1B8h]
    char *v22; // [esp+14h] [ebp-1B8h]
    int v23; // [esp+18h] [ebp-1B4h]
    int v24; // [esp+18h] [ebp-1B4h]
    float v25; // [esp+20h] [ebp-1ACh]
    float v26; // [esp+24h] [ebp-1A8h]
    float v27; // [esp+28h] [ebp-1A4h]
    float v28; // [esp+2Ch] [ebp-1A0h]
    float v29; // [esp+30h] [ebp-19Ch]
    float v30; // [esp+34h] [ebp-198h]
    float v31; // [esp+38h] [ebp-194h]
    float v32; // [esp+3Ch] [ebp-190h]
    float v33; // [esp+40h] [ebp-18Ch]
    float v34; // [esp+44h] [ebp-188h]
    float v35; // [esp+48h] [ebp-184h]
    float v36; // [esp+4Ch] [ebp-180h]
    float v37; // [esp+50h] [ebp-17Ch]
    float v38; // [esp+54h] [ebp-178h]
    float v39; // [esp+5Ch] [ebp-170h]
    float v40; // [esp+64h] [ebp-168h]
    float v41; // [esp+68h] [ebp-164h]
    float y; // [esp+6Ch] [ebp-160h]
    int i; // [esp+74h] [ebp-158h]
    int timeleft; // [esp+78h] [ebp-154h]
    char dlTimeBuf[68]; // [esp+7Ch] [ebp-150h] BYREF
    int downloadTime; // [esp+C0h] [ebp-10Ch]
    char xferRateBuf[64]; // [esp+C4h] [ebp-108h] BYREF
    int firstColumn; // [esp+104h] [ebp-C8h]
    uiInfo_s *uiInfo; // [esp+108h] [ebp-C4h]
    int percent; // [esp+10Ch] [ebp-C0h]
    int width; // [esp+110h] [ebp-BCh]
    float secondColumn; // [esp+114h] [ebp-B8h]
    int downloadCount; // [esp+118h] [ebp-B4h]
    char totalSizeBuf[68]; // [esp+11Ch] [ebp-B0h] BYREF
    float maxSecondColumnWidth; // [esp+160h] [ebp-6Ch]
    float fileNameScale; // [esp+164h] [ebp-68h]
    const char *s; // [esp+168h] [ebp-64h]
    char dlSizeBuf[68]; // [esp+16Ch] [ebp-60h] BYREF
    float color[4]; // [esp+1B4h] [ebp-18h] BYREF
    int downloadSize; // [esp+1C4h] [ebp-8h]
    int xferRate; // [esp+1C8h] [ebp-4h]

    firstColumn = 24;
    secondColumn = 200.0;
    maxSecondColumnWidth = 630.0 - (float)200.0;
    downloadSize = legacyHacks.cl_downloadSize;
    downloadCount = legacyHacks.cl_downloadCount;
    downloadTime = legacyHacks.cl_downloadTime;
    color[0] = 0.0;
    color[1] = 0.0;
    color[2] = 0.0;
    color[3] = 0.2;
    y = yStart + 184.0;
    UI_FillRect(&scrPlaceFull, 0.0, y, 640.0, 85.0, 0, 0, color);
    v41 = yStart + 185.0;
    UI_FillRect(&scrPlaceFull, 0.0, v41, 640.0, 83.0, 0, 0, color);
    v40 = yStart + 186.0;
    UI_FillRect(&scrPlaceFull, 0.0, v40, 640.0, 81.0, 0, 0, color);
    if (downloadSize > 0)
    {
        color[0] = 0.0;
        color[1] = 1.0;
        color[2] = 0.0;
        color[3] = 0.15000001;
        width = (int)((double)downloadCount / (double)downloadSize * 640.0);
        v17 = (float)(width + 2);
        v39 = yStart + 164.0;
        UI_FillRect(&scrPlaceFull, 0.0, v39, v17, 5.0, 0, 0, color);
        v18 = (float)(width + 1);
        v38 = yStart + 165.0;
        UI_FillRect(&scrPlaceFull, 0.0, v38, v18, 3.0, 0, 0, color);
        v19 = (float)width;
        v37 = yStart + 166.0;
        UI_FillRect(&scrPlaceFull, 0.0, v37, v19, 1.0, 0, 0, color);
    }
    v36 = yStart + 210.0;
    v5 = UI_SafeTranslateString("EXE_DOWNLOADING");
    UI_DrawText(&scrPlaceFull, v5, 64, font, 24.0, v36, 0, 0, scale, colorLtGrey, 0);
    v35 = yStart + 235.0;
    v6 = UI_SafeTranslateString("EXE_EST_TIME_LEFT");
    UI_DrawText(&scrPlaceFull, v6, 64, font, 24.0, v35, 0, 0, scale, colorLtGrey, 0);
    v34 = yStart + 260.0;
    v7 = UI_SafeTranslateString("EXE_TRANS_RATE");
    UI_DrawText(&scrPlaceFull, v7, 64, font, 24.0, v34, 0, 0, scale, colorLtGrey, 0);
    width = UI_TextWidth(downloadName, 0, font, scale);
    if (maxSecondColumnWidth >= (double)width)
    {
        fileNameScale = scale;
    }
    else
    {
        fileNameScale = scale * maxSecondColumnWidth / (double)width;
        if (fileNameScale <= 0.2000000029802322)
            v33 = 0.2;
        else
            v33 = fileNameScale;
        fileNameScale = v33;
    }
    v32 = yStart + 210.0;
    UI_DrawText(&scrPlaceFull, downloadName, 0x7FFFFFFF, font, secondColumn, v32, 0, 0, fileNameScale, colorLtGrey, 0);
    UI_ReadableSize(dlSizeBuf, 0x40u, downloadCount);
    UI_ReadableSize(totalSizeBuf, 0x40u, downloadSize);
    if (downloadSize <= 0)
        percent = 0;
    else
        percent = (int)((double)downloadCount * 100.0 / (double)downloadSize);
    if (downloadCount >= 4096 && downloadTime)
    {
        uiInfo = &uiInfoArray;
        if ((uiInfoArray.uiDC.realTime - downloadTime) / 1000)
            xferRate = downloadCount / ((uiInfo->uiDC.realTime - downloadTime) / 1000);
        else
            xferRate = 0;
        UI_ReadableSize(xferRateBuf, 0x40u, xferRate);
        if (downloadSize && xferRate)
        {
            timeleft = 0;
            tleEstimates[tleIndex++] = downloadSize / xferRate
                - downloadSize / xferRate * (downloadCount / 1024) / (downloadSize / 1024);
            if (tleIndex >= 80)
                tleIndex = 0;
            for (i = 0; i < 80; ++i)
                timeleft += tleEstimates[i];
            UI_PrintTime(dlTimeBuf, 0x40u, timeleft / 80);
            v29 = yStart + 235.0;
            UI_DrawText(&scrPlaceFull, dlTimeBuf, 0x7FFFFFFF, font, secondColumn, v29, 0, 0, scale, colorLtGrey, 3);
            v23 = percent;
            v21 = UI_SafeTranslateString("EXE_COPIED");
            v10 = UI_SafeTranslateString("EXE_OF");
            s = va("%s %s %s %s (%d%%)", dlSizeBuf, v10, totalSizeBuf, v21, v23);
            v28 = yStart + 320.0;
            Text_PaintCenter(&scrPlaceFull, centerPoint, v28, font, scale, colorLtGrey, (char *)s, 0);
        }
        else
        {
            v11 = UI_SafeTranslateString("EXE_ESTIMATING");
            v27 = yStart + 235.0;
            Text_PaintCenter(&scrPlaceFull, centerPoint, v27, font, scale, colorLtGrey, v11, 0);
            if (downloadSize)
            {
                v24 = percent;
                v22 = UI_SafeTranslateString("EXE_COPIED");
                v12 = UI_SafeTranslateString("EXE_OF");
                s = va("%s %s %s %s (%d%%)", dlSizeBuf, v12, totalSizeBuf, v22, v24);
            }
            else
            {
                v13 = UI_SafeTranslateString("EXE_COPIED");
                s = va("(%s %s)", dlSizeBuf, v13);
            }
            v26 = yStart + 320.0;
            Text_PaintCenter(&scrPlaceFull, centerPoint, v26, font, scale, colorLtGrey, (char *)s, 0);
        }
        if (xferRate)
        {
            v25 = yStart + 260.0;
            x = secondColumn;
            v14 = UI_SafeTranslateString("EXE_SECONDS");
            v15 = va("%s/%s", xferRateBuf, v14);
            UI_DrawText(&scrPlaceFull, v15, 0x7FFFFFFF, font, x, v25, 0, 0, scale, colorLtGrey, 3);
        }
    }
    else
    {
        v8 = UI_SafeTranslateString("EXE_ESTIMATING");
        v31 = yStart + 235.0;
        Text_PaintCenter(&scrPlaceFull, centerPoint, v31, font, scale, colorLtGrey, v8, 0);
        v20 = UI_SafeTranslateString("EXE_COPIED");
        v9 = UI_SafeTranslateString("EXE_OF");
        s = va("%s %s %s %s (%d%%)", dlSizeBuf, v9, totalSizeBuf, v20, percent);
        v30 = yStart + 340.0;
        Text_PaintCenter(&scrPlaceFull, centerPoint, v30, font, scale, colorLtGrey, (char *)s, 0);
    }
}

void __cdecl UI_DrawConnectScreen(int localClientNum)
{
    char *v1; // eax
    char *v2; // eax
    char *MapDisplayName; // eax
    char *v4; // eax
    char *v5; // eax
    const char *String; // [esp+18h] [ebp-DC8h]
    const DvarValue *p_current; // [esp+18h] [ebp-DC8h]
    signed int v8; // [esp+20h] [ebp-DC0h]
    float y; // [esp+30h] [ebp-DB0h]
    char v10; // [esp+37h] [ebp-DA9h]
    char *v11; // [esp+3Ch] [ebp-DA4h]
    char *v12; // [esp+40h] [ebp-DA0h]
    char v13; // [esp+47h] [ebp-D99h]
    char *v14; // [esp+4Ch] [ebp-D94h]
    char *v15; // [esp+50h] [ebp-D90h]
    Font_s *FontHandle; // [esp+54h] [ebp-D8Ch]
    Font_s *downloadDisplayFont; // [esp+58h] [ebp-D88h]
    float yPrint; // [esp+60h] [ebp-D80h]
    Font_s *font; // [esp+64h] [ebp-D7Ch]
    char *translation; // [esp+68h] [ebp-D78h]
    bool displayConnectionInfo; // [esp+6Fh] [ebp-D71h]
    float yStart; // [esp+74h] [ebp-D6Ch]
    float centerPoint; // [esp+78h] [ebp-D68h]
    int index; // [esp+80h] [ebp-D60h]
    char *s; // [esp+84h] [ebp-D5Ch]
    char *sa; // [esp+84h] [ebp-D5Ch]
    float scale; // [esp+88h] [ebp-D58h]
    int i; // [esp+8Ch] [ebp-D54h]
    char ps[60]; // [esp+90h] [ebp-D50h] BYREF
    float headerYPos; // [esp+CCh] [ebp-D14h]
    uiClientState_s cstate; // [esp+D0h] [ebp-D10h] BYREF
    char text[256]; // [esp+CD8h] [ebp-108h] BYREF
    int neednewline; // [esp+DDCh] [ebp-4h]

    yStart = 89.0;
    displayConnectionInfo = 1;
    headerYPos = 55.0;
    CG_DrawInformation(localClientNum);
    centerPoint = 320.0;
    scale = 0.5;
    font = UI_GetFontHandle(&scrPlaceFull, 6, 0.5);
    if (!font)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 6876, 0, "%s", "font");
    CL_GetClientState(localClientNum, &cstate);
    if (cls.wwwDlInProgress && legacyHacks.cl_downloadName[0])
    {
        downloadDisplayFont = UI_GetFontHandle(&scrPlaceFull, 1, scale);
        UI_DisplayDownloadInfo(legacyHacks.cl_downloadName, centerPoint, yStart, downloadDisplayFont, scale);
    }
    else
    {
        if (I_stricmp(cstate.servername, "Auto-Updater"))
        {
            if (g_mapname[0])
            {
                translation = UI_GetGameTypeDisplayName(g_gametype);
                Text_PaintCenter(
                    &scrPlaceFull,
                    centerPoint,
                    yStart,
                    font,
                    scale,
                    colorWhite,
                    translation,
                    &ui_connectScreenTextGlowColor->current.value);
                p_current = &ui_connectScreenTextGlowColor->current;
                MapDisplayName = UI_GetMapDisplayName(g_mapname);
                y = yStart + (float)30.0;
                Text_PaintCenter(&scrPlaceFull, centerPoint, y, font, scale, colorWhite, MapDisplayName, &p_current->value);
                displayConnectionInfo = 0;
            }
        }
        else
        {
            if (legacyHacks.cl_downloadName[0])
            {
                String = Dvar_GetString("cl_updateversion");
                v1 = UI_SafeTranslateString("EXE_DOWNLOADINGUPDATE");
                v15 = UI_ReplaceConversionString(v1, String);
                v14 = text;
                do
                {
                    v13 = *v15;
                    *v14++ = *v15++;
                } while (v13);
            }
            else
            {
                v2 = UI_SafeTranslateString("EXE_CONNECTINGTO");
                v12 = UI_ReplaceConversionString(v2, cstate.servername);
                v11 = text;
                do
                {
                    v10 = *v12;
                    *v11++ = *v12++;
                } while (v10);
            }
            Text_PaintCenter(&scrPlaceFull, centerPoint, headerYPos, font, scale, colorWhite, text, 0);
            displayConnectionInfo = 0;
        }
        if (cstate.connState < CA_CONNECTED)
        {
            index = 0;
            yPrint = yStart + 210.0;
            neednewline = 0;
            s = UI_SafeTranslateString(cstate.messageString);
            v8 = strlen(s);
            for (i = 0; i < v8; ++i)
            {
                ps[index] = s[i];
                if (index > 40 && i > 0)
                    neednewline = 1;
                if (index >= 58 || i == v8 - 1 || neednewline && s[i] == 32)
                {
                    ps[index + 1] = 0;
                    Text_PaintCenter(&scrPlaceFull, centerPoint, yPrint, font, scale, colorYellow, ps, 0);
                    neednewline = 0;
                    yPrint = yPrint + 22.0;
                    index = -1;
                }
                ++index;
            }
        }
        if (!I_stricmp(cstate.servername, "localhost"))
            displayConnectionInfo = 0;
        if (DB_GetLoadedFraction() == 1.0)
            displayConnectionInfo = 1;
        sa = 0;
        switch (cstate.connState)
        {
        case CA_CONNECTING:
            if (displayConnectionInfo)
            {
                v4 = UI_SafeTranslateString("EXE_AWAITINGCONNECTION");
                sa = UI_ReplaceConversionInt(v4, cstate.connectPacketCount);
            }
            goto LABEL_49;
        case CA_CHALLENGING:
            if (displayConnectionInfo)
            {
                v5 = UI_SafeTranslateString("EXE_AWAITINGCHALLENGE");
                sa = UI_ReplaceConversionInt(v5, cstate.connectPacketCount);
            }
            goto LABEL_49;
        case CA_CONNECTED:
            if (legacyHacks.cl_downloadName[0])
            {
                FontHandle = UI_GetFontHandle(&scrPlaceFull, 1, scale);
                UI_DisplayDownloadInfo(legacyHacks.cl_downloadName, centerPoint, yStart, FontHandle, scale);
            }
            else
            {
                if (displayConnectionInfo)
                    sa = UI_SafeTranslateString("EXE_AWAITINGGAMESTATE");
            LABEL_49:
                if (sa)
                    Text_PaintCenterWithDots(&scrPlaceFull, centerPoint, 145.0, font, scale, colorWhite, sa, 0);
            }
            break;
        case CA_SENDINGSTATS:
            if (displayConnectionInfo)
                sa = UI_SafeTranslateString("EXE_UPLOADINGSTATS");
            goto LABEL_49;
        case CA_LOADING:
        case CA_PRIMED:
            if (displayConnectionInfo)
                sa = UI_SafeTranslateString("EXE_AWAITINGHOST");
            goto LABEL_49;
        default:
            return;
        }
    }
}

void __cdecl Text_PaintCenter(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    Font_s *const font,
    float scale,
    const float *color,
    char *text,
    const float *glowColor)
{
    float v8; // [esp+28h] [ebp-Ch]

    v8 = x - (double)(UI_TextWidth(text, 0, font, scale) / 2);
    UI_DrawTextWithGlow(scrPlace, text, 0x7FFFFFFF, font, v8, y, 0, 0, scale, color, 6, glowColor, 0, 0);
}

void __cdecl Text_PaintCenterWithDots(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    Font_s *const font,
    float scale,
    const float *color,
    const char *text,
    const float *glowColor)
{
    char *v8; // eax
    float v9; // [esp+28h] [ebp-18h]
    int v10; // [esp+30h] [ebp-10h]
    int width; // [esp+34h] [ebp-Ch]
    const char *dots; // [esp+3Ch] [ebp-4h]

    v10 = ((int)Sys_Milliseconds() / 500) & 3;
    switch (v10)
    {
    case 1:
        dots = ".  ";
        break;
    case 2:
        dots = ".. ";
        break;
    case 3:
        dots = "...";
        break;
    default:
        dots = "   ";
        break;
    }
    width = UI_TextWidth(text, 0, font, scale);
    v8 = va("%s%s", text, dots);
    v9 = x - (double)(width / 2);
    UI_DrawTextWithGlow(scrPlace, v8, 0x7FFFFFFF, font, v9, y, 0, 0, scale, color, 6, glowColor, 0, 0);
}

double __cdecl UI_GetBlurRadius(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (!&uiInfoArray)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 7029, 0, "%s", "uiInfo");
    return uiInfoArray.uiDC.blurRadiusOut;
}

void UI_StopServerRefresh()
{
    int count; // [esp+0h] [ebp-4h]

    if (*(unsigned int *)&sharedUiInfo.serverStatus.string[1124])
    {
        *(unsigned int *)&sharedUiInfo.serverStatus.string[1124] = 0;
        Com_Printf(
            13,
            "%d servers listed in browser with %d players.\n",
            *(unsigned int *)&sharedUiInfo.gap8EB4[72900],
            *(unsigned int *)&sharedUiInfo.gap8EB4[72908]);
        count = LAN_GetServerCount(ui_netSource->current.integer);
        if (count - *(unsigned int *)&sharedUiInfo.gap8EB4[72900] > 0)
            Com_Printf(
                13,
                "%d servers not listed (filtered out by game browser settings)\n",
                count - *(unsigned int *)&sharedUiInfo.gap8EB4[72900]);
    }
}

void __cdecl UI_DoServerRefresh(uiInfo_s *uiInfo)
{
    bool wait; // [esp+0h] [ebp-4h]

    wait = 0;
    if (*(unsigned int *)&sharedUiInfo.serverStatus.string[1124])
    {
        if (ui_netSource->current.integer != 2)
        {
            if (ui_netSource->current.integer)
                wait = LAN_WaitServerResponse(ui_netSource->current.integer) != 0;
            else
                wait = LAN_GetServerCount(ui_netSource->current.integer) == 0;
        }
        if (uiInfo->uiDC.realTime >= *(int *)&sharedUiInfo.serverStatus.string[1104] || !wait)
        {
            UI_UpdateDisplayServers(uiInfo);
            if (LAN_UpdateDirtyPings((netsrc_t)uiInfo->uiDC.localClientNum, ui_netSource->current.unsignedInt))
            {
                *(unsigned int *)&sharedUiInfo.serverStatus.string[1104] = uiInfo->uiDC.realTime + 1000;
            }
            else if (!wait)
            {
                UI_BuildServerDisplayList(uiInfo, 2);
                UI_StopServerRefresh();
            }
            UI_BuildServerDisplayList(uiInfo, 0);
        }
    }
}

void __cdecl UI_StartServerRefresh(int localClientNum, int full)
{
    char *v2; // eax
    char *v3; // eax
    int v4; // eax
    int v5; // eax
    int tm_mday; // [esp-10h] [ebp-6Ch]
    int v7; // [esp-Ch] [ebp-68h]
    int tm_hour; // [esp-8h] [ebp-64h]
    int tm_min; // [esp-4h] [ebp-60h]
    char *v10; // [esp-4h] [ebp-60h]
    qtime_s q; // [esp+14h] [ebp-48h] BYREF
    char dvarName[24]; // [esp+38h] [ebp-24h] BYREF
    const char *ptr; // [esp+54h] [ebp-8h]
    int i; // [esp+58h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    Com_RealTime(&q);
    _snprintf(dvarName, 0x18u, "ui_lastServerRefresh_%i", ui_netSource->current.integer);
    tm_min = q.tm_min;
    tm_hour = q.tm_hour;
    v7 = q.tm_year + 1900;
    tm_mday = q.tm_mday;
    v2 = UI_SafeTranslateString((char *)MonthAbbrev[q.tm_mon]);
    v3 = va("%s %i, %i   %i:%02i", v2, tm_mday, v7, tm_hour, tm_min);
    Dvar_SetStringByName(dvarName, v3);
    if (full)
    {
        *(_DWORD *)&sharedUiInfo.serverStatus.string[1124] = 1;
        *(_DWORD *)&sharedUiInfo.gap8EB4[72912] = uiInfoArray.uiDC.realTime + 1000;
        UI_ClearDisplayedServers();
        LAN_MarkServerDirty(ui_netSource->current.integer, 0xFFFFFFFF, 1u);
        LAN_ResetPings(ui_netSource->current.integer);
        if (ui_netSource->current.integer)
        {
            *(_DWORD *)&sharedUiInfo.serverStatus.string[1104] = uiInfoArray.uiDC.realTime + 5000;
            if (ui_netSource->current.integer == 1)
            {
                i = 0;
                ptr = Dvar_GetVariantString("debug_protocol");
                if (strlen(ptr))
                    v10 = va("globalservers %d %s full empty\n", i, ptr);
                else
                    v10 = va("globalservers %d %d full empty\n", i, 1);
                v5 = CL_ControllerIndexFromClientNum(localClientNum);
                Cmd_ExecuteSingleCommand(localClientNum, v5, v10);
            }
        }
        else
        {
            v4 = CL_ControllerIndexFromClientNum(localClientNum);
            Cmd_ExecuteSingleCommand(localClientNum, v4, (char*)"localservers\n");
            *(_DWORD *)&sharedUiInfo.serverStatus.string[1104] = uiInfoArray.uiDC.realTime + 1000;
        }
    }
    else
    {
        UI_UpdatePendingPings(&uiInfoArray);
    }
}

void __cdecl UI_UpdatePendingPings(uiInfo_s *uiInfo)
{
    LAN_ResetPings(ui_netSource->current.integer);
    *(unsigned int *)&sharedUiInfo.serverStatus.string[1124] = 1;
    *(unsigned int *)&sharedUiInfo.serverStatus.string[1104] = uiInfo->uiDC.realTime + 1000;
}

char errorString[1024];
char *__cdecl UI_SafeTranslateString(const char *reference)
{
    char v2; // [esp+3h] [ebp-11h]
    char *v3; // [esp+8h] [ebp-Ch]
    const char *v4; // [esp+Ch] [ebp-8h]
    const char *translation; // [esp+10h] [ebp-4h]

    if (*reference == 21)
    {
        ++reference;
        translation = 0;
    }
    else
    {
        translation = SEH_StringEd_GetString(reference);
    }
    if (!translation)
    {
        if (loc_warnings->current.enabled)
        {
            if (loc_warningsAsErrors->current.enabled)
                Com_Error(ERR_LOCALIZATION, "Could not translate string \"%s\"", reference);
            else
                Com_PrintWarning(13, "WARNING: Could not translate string \"%s\"\n", reference);
            strcpy(errorString, "^1UNLOCALIZED(^7");
            I_strncat(errorString, 1024, reference);
            I_strncat(errorString, 1024, "^1)^7");
        }
        else
        {
            v4 = reference;
            v3 = errorString;
            do
            {
                v2 = *v4;
                *v3++ = *v4++;
            } while (v2);
        }
        return errorString;
    }
    return (char *)translation;
}

BOOL __cdecl UI_AnyMenuVisible(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return uiInfoArray.uiDC.openMenuCount != 0;
}

char *__cdecl UI_ReplaceConversionString(char *sourceString, const char *replaceString)
{
    char outputString[1028]; // [esp+0h] [ebp-430h] BYREF
    ConversionArguments convArgs; // [esp+408h] [ebp-28h] BYREF

    memset(&convArgs.args[1], 0, 32);
    convArgs.argCount = 1;
    convArgs.args[0] = replaceString;
    UI_ReplaceConversions(sourceString, &convArgs, outputString, 1024);
    return va(outputString);
}

char *__cdecl UI_ReplaceConversionInt(char *sourceString, int replaceInt)
{
    char outputString[1028]; // [esp+0h] [ebp-450h] BYREF
    char tempString[32]; // [esp+404h] [ebp-4Ch] BYREF
    ConversionArguments convArgs; // [esp+428h] [ebp-28h] BYREF

    memset(&convArgs, 0, sizeof(convArgs));
    sprintf(tempString, "%d", replaceInt);
    convArgs.argCount = 1;
    convArgs.args[0] = tempString;
    UI_ReplaceConversions(sourceString, &convArgs, outputString, 1024);
    return va(outputString);
}

void __cdecl UI_ReplaceConversions(
    char *sourceString,
    ConversionArguments *arguments,
    char *outputString,
    int outputStringSize)
{
    int v4; // eax
    int v5; // edx
    signed int v6; // [esp+0h] [ebp-38h]
    int argIndex; // [esp+24h] [ebp-14h]
    int argStringIndex; // [esp+28h] [ebp-10h]
    int index; // [esp+2Ch] [ebp-Ch]
    int outputStringCounter; // [esp+30h] [ebp-8h]
    int sourceStringLength; // [esp+34h] [ebp-4h]

    if (!sourceString)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 7349, 0, "%s", "sourceString");
    strstr((unsigned __int8 *)sourceString, (unsigned __int8 *)"&&");
    if (v4)
    {
        if (!arguments)
            MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 7357, 0, "%s", "arguments");
        if (arguments->argCount > 9)
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                7358,
                0,
                "%s\n\t(arguments->argCount) = %i",
                "(arguments->argCount <= 9)",
                arguments->argCount);
        v5 = strlen(sourceString);
        sourceStringLength = v5;
        if (v5 <= 0)
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                7362,
                0,
                "%s\n\t(sourceStringLength) = %i",
                "(sourceStringLength > 0)",
                v5);
        memset((unsigned __int8 *)outputString, 0, outputStringSize);
        outputStringCounter = 0;
        index = 0;
        while (index < sourceStringLength)
        {
            if (!strncmp(&sourceString[index], "&&", 2u) && isdigit(sourceString[index + 2]))
            {
                argIndex = sourceString[index + 2] - 49;
                if (argIndex < 0 || argIndex >= arguments->argCount)
                    MyAssertHandler(
                        ".\\ui_mp\\ui_main_mp.cpp",
                        7376,
                        0,
                        "%s\n\t(argIndex) = %i",
                        "(argIndex >= 0 && argIndex < arguments->argCount)",
                        argIndex);
                if (argIndex >= 9)
                    MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 7377, 0, "%s\n\t(argIndex) = %i", "(argIndex < 9)", argIndex);
                if (!arguments->args[argIndex])
                    MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 7379, 0, "%s", "arguments->args[argIndex]");
                v6 = strlen(arguments->args[argIndex]);
                for (argStringIndex = 0; argStringIndex < v6; ++argStringIndex)
                    outputString[outputStringCounter++] = arguments->args[argIndex][argStringIndex];
                index += 3;
            }
            else
            {
                outputString[outputStringCounter++] = sourceString[index++];
            }
        }
        CL_ResetStats_f();
    }
    else
    {
        I_strncpyz(outputString, sourceString, outputStringSize);
    }
}

void __cdecl UI_CloseAll(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    Menus_CloseAll(&uiInfoArray.uiDC);
    UI_SetActiveMenu(localClientNum, 0);
}

void __cdecl UI_CloseFocusedMenu(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (Menu_Count(&uiInfoArray.uiDC) > 0)
    {
        if (Menu_GetFocused(&uiInfoArray.uiDC))
        {
            if (!Menus_AnyFullScreenVisible(&uiInfoArray.uiDC))
                Menus_CloseAll(&uiInfoArray.uiDC);
        }
        else if (Key_IsCatcherActive(localClientNum, 16))
        {
            Key_RemoveCatcher(localClientNum, -17);
        }
    }
}

int __cdecl UI_Popup(int localClientNum, const char *menu)
{
    if (!CL_AllowPopup(localClientNum) || UI_IsFullscreen(localClientNum))
        return 0;
    if (I_stricmp(menu, "UIMENU_WM_QUICKMESSAGE"))
    {
        if (!I_stricmp(menu, "UIMENU_WM_AUTOUPDATE"))
            UI_SetActiveMenu(localClientNum, 8);
    }
    else
    {
        UI_SetActiveMenu(localClientNum, 7);
    }
    return 1;
}

void __cdecl UI_SetSystemCursorPos(UiContext *dc, float x, float y)
{
    tagPOINT X; // [esp+0h] [ebp-28h]
    float v4; // [esp+8h] [ebp-20h]
    float v5; // [esp+Ch] [ebp-1Ch]
    float v6; // [esp+10h] [ebp-18h]
    float v7; // [esp+14h] [ebp-14h]
    float v8; // [esp+18h] [ebp-10h]
    float v9; // [esp+1Ch] [ebp-Ch]

    v9 = x * scrPlaceFull.scaleVirtualToFull[0];
    v7 = v9 + 0.5;
    v6 = floor(v7);
    v8 = y * scrPlaceFull.scaleVirtualToFull[1];
    v5 = v8 + 0.5;
    v4 = floor(v5);
    X.y = (int)v4;
    X.x = (int)v6;
    CL_SetCursorPos(X);
}

int __cdecl UI_PopupScriptMenu(int localClientNum, const char *menuName, bool useMouse)
{
    menuDef_t *pFocus; // [esp+Ch] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    pFocus = Menu_GetFocused(&uiInfoArray.uiDC);
    if (pFocus && uiInfoArray.currentMenuType != UIMENU_SCRIPT_POPUP && uiInfoArray.currentMenuType != UIMENU_SCOREBOARD)
        return 0;
    if (!pFocus || I_stricmp(pFocus->window.name, menuName))
    {
        uiInfoArray.currentMenuType = UIMENU_SCRIPT_POPUP;
        if (!useMouse)
        {
            uiInfoArray.uiDC.cursor.x = 639.0;
            uiInfoArray.uiDC.cursor.y = 479.0;
            UI_SetSystemCursorPos(&uiInfoArray.uiDC, 639.0, 479.0);
        }
        Key_SetCatcher(localClientNum, 16);
        Menus_CloseAll(&uiInfoArray.uiDC);
        Menus_OpenByName(&uiInfoArray.uiDC, menuName);
    }
    return 1;
}

void __cdecl UI_ClosePopupScriptMenu(int localClientNum, bool allowResponse)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (uiInfoArray.currentMenuType == UIMENU_SCRIPT_POPUP)
    {
        uiInfoArray.allowScriptMenuResponse = allowResponse;
        UI_CloseFocusedMenu(localClientNum);
        uiInfoArray.allowScriptMenuResponse = 1;
    }
}

bool __cdecl UI_AllowScriptMenuResponse(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return uiInfoArray.allowScriptMenuResponse;
}

void __cdecl UI_CloseInGameMenu(int localClientNum)
{
    if (!UI_IsFullscreen(localClientNum) && UI_GetActiveMenu(localClientNum) == 2)
        UI_CloseFocusedMenu(localClientNum);
}

void __cdecl UI_CloseAllMenus(int localClientNum)
{
    UI_CloseAll(localClientNum);
}

bool __cdecl Menu_IsMenuOpenAndVisible(int localClientNum, const char *menuName)
{
    menuDef_t *menu; // [esp+4h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    menu = Menus_FindByName(&uiInfoArray.uiDC, menuName);
    if (!menu)
        return 0;
    if (Menus_MenuIsInStack(&uiInfoArray.uiDC, menu))
        return Menu_IsVisible(&uiInfoArray.uiDC, menu) != 0;
    return 0;
}

bool __cdecl UI_ShouldDrawCrosshair()
{
    return ui_drawCrosshair->current.enabled;
}

