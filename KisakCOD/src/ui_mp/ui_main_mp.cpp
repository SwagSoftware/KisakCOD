#include "ui_mp.h"
#include <ui/ui.h>
#include <qcommon/cmd.h>
#include <qcommon/mem_track.h>
#include <client_mp/client_mp.h>
#include <gfx_d3d/r_rendercmds.h>
#include <universal/q_parse.h>

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

int ui_serverFilterType;

UiContext uiInfoArray;
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
    return &uiInfoArray.localVars;
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
    uiInfoArray.frameTime = realtime - uiInfoArray.realTime;
    uiInfoArray.realTime = realtime;
    *((unsigned int *)&uiInfoArray + dword_DB561AC[0]++ % 4 + 1572) = uiInfoArray.frameTime;
    if (dword_DB561AC[0] > 4)
    {
        frameTimeTotal = 0;
        for (frameTimeIndex = 0; frameTimeIndex < 4; ++frameTimeIndex)
            frameTimeTotal += *((unsigned int *)&uiInfoArray + frameTimeIndex + 1572);
        if (!frameTimeTotal)
            frameTimeTotal = 1;
        uiInfoArray.FPS = (float)(4000 / frameTimeTotal);
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
    if (Menu_Count(&uiInfoArray) > 0)
    {
        Menu_PaintAll(&uiInfoArray);
        UI_DoServerRefresh((uiInfo_s *)&uiInfoArray);
        UI_BuildServerStatus((uiInfo_s *)&uiInfoArray, 0);
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
            if (!MEMORY[0xE7A7CC][0])
                UI_DrawBuildNumber(localClientNum);
        }
        if (uiInfoArray.isCursorVisible)
        {
            if (!Dvar_GetBool("cl_bypassMouseInput") && UI_GetActiveMenu(localClientNum) != 10)
            {
                w = scrPlaceFull.scaleVirtualToReal[0] * 32.0 / scrPlaceFull.scaleVirtualToFull[0];
                h = scrPlaceFull.scaleVirtualToReal[1] * 32.0 / scrPlaceFull.scaleVirtualToFull[1];
                y = uiInfoArray.cursor.y - h * 0.5;
                x = uiInfoArray.cursor.x - w * 0.5;
                UI_DrawHandlePic(&scrPlaceView[localClientNum], x, y, w, h, 4, 4, 0, sharedUiInfo.assets.cursor);
            }
        }
    }
    if (!Menu_GetFocused(&uiInfoArray))
    {
        if (Key_IsCatcherActive(localClientNum, 16))
            Key_RemoveCatcher(localClientNum, -17);
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
    Menus_CloseAll(&uiInfoArray);
    sharedUiInfo.assets.whiteMaterial = 0;
    UILocalVar_Shutdown(&uiInfoArray.localVars);
    if (!useFastFile->current.enabled)
        Menus_FreeAllMemory(&uiInfoArray);
    LAN_SaveServersToCache();
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
    UI_AddMenuList(&uiInfoArray, menuList);
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
            Dvar_SetInt((dvar_s *)ui_netSource, 0);
        v3 = va(aExeNetsource, netSources[ui_netSource->current.integer]);
        s = SEH_LocalizeTextMessage(v3, "net source", LOCMSG_SAFE);
        break;
    case 222:
        if ((unsigned int)ui_serverFilterType >= 2)
            ui_serverFilterType = 0;
        v4 = va(aExeServerfilte, serverFilters[ui_serverFilterType].description);
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
    menu = Menus_FindByName(&uiInfoArray, "pregame_loaderror_mp");
    if (!menu || !Menus_MenuIsInStack(&uiInfoArray, menu))
    {
        if (g_mapname[0])
        {
            if (useFastFile->current.enabled)
                menua = DB_FindXAssetHeader(ASSET_TYPE_MENU, "connect").menu;
            else
                menua = Menus_FindByName(&uiInfoArray, "connect");
        }
        else
        {
            menua = 0;
        }
        if (menua)
        {
            if (!Menus_MenuIsInStack(&uiInfoArray, menua))
                Menus_Open(&uiInfoArray, menua);
            uiInfoArray.blurRadiusOut = 0.0;
            Window_SetDynamicFlags(localClientNum, &menua->window, 16388);
            Menu_Paint(&uiInfoArray, menua);
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
        menuList = UI_LoadMenus("ui_mp/ingame.txt", 3);
        if (localClientNum)
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                332,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        UI_AddMenuList(&uiInfoArray, menuList);
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
void __cdecl ProfLoad_DrawOverlay(rectDef_s *rect)
{
    MapProfileEntry *v1; // eax
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
        ProfLoad_DrawTree(v1);
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
    v6 = va(aExeServerfilte, serverFilters[ui_serverFilterType].description);
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

    v6 = va(aExeNetsource, netSources[ui_netSource->current.integer]);
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

    if (*(unsigned int *)&sharedUiInfo.serverStatus.string[1124])
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
        LODWORD(newColor[4]) = &uiInfoArray;
        v10 = (float)(uiInfoArray.realTime / 75);
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
    char *v11; // [esp+1Ch] [ebp-8h]

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
    menu = Menu_GetFocused(&uiInfoArray);
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
    dvar_s *Var; // eax
    const char *v16; // eax
    int v17; // eax
    const char *v18; // eax
    const char *v19; // eax
    char *v20; // eax
    char *v21; // eax
    char *v22; // eax
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
        dc = &uiInfoArray;
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
                                                                                                                                                    *(unsigned int *)&sharedUiInfo.gap8EB4[4 * *(unsigned int *)&sharedUiInfo.serverStatus.string[1128] - 7100],
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
                                                                                                                                                    *(unsigned int *)&sharedUiInfo.gap8EB4[4 * *(unsigned int *)&sharedUiInfo.serverStatus.string[1128] - 7100],
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
                                                                                                                            if (i == *(unsigned int *)&sharedUiInfo.serverStatus.string[1112])
                                                                                                                                *(unsigned int *)&sharedUiInfo.serverStatus.string[1116] = *(unsigned int *)&sharedUiInfo.serverStatus.string[1116] == 0;
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
                                                                                                                Cmd_ExecuteSingleCommand(localClientNum, v17, aQuit_1);
                                                                                                            }
                                                                                                        }
                                                                                                        else
                                                                                                        {
                                                                                                            Dvar_SetBoolByName("cg_thirdPerson", 0);
                                                                                                            UI_UpdateDisplayServers((uiInfo_s *)dc);
                                                                                                            ServerPunkBuster = LAN_GetServerPunkBuster(
                                                                                                                ui_netSource->current.integer,
                                                                                                                *(unsigned int *)&sharedUiInfo.gap8EB4[4 * *(unsigned int *)&sharedUiInfo.serverStatus.string[1128] - 7100]);
                                                                                                            if (ServerPunkBuster != 1 || Dvar_GetBool("cl_punkbuster"))
                                                                                                            {
                                                                                                                if (*(int *)&sharedUiInfo.serverStatus.string[1128] >= 0
                                                                                                                    && *(int *)&sharedUiInfo.serverStatus.string[1128] < *(int *)&sharedUiInfo.gap8EB4[72900])
                                                                                                                {
                                                                                                                    LAN_GetServerAddressString(
                                                                                                                        ui_netSource->current.integer,
                                                                                                                        *(unsigned int *)&sharedUiInfo.gap8EB4[4
                                                                                                                        * *(unsigned int *)&sharedUiInfo.serverStatus.string[1128]
                                                                                                                        - 7100],
                                                                                                                        v44,
                                                                                                                        1024);
                                                                                                                    v16 = va("connect %s\n", v44);
                                                                                                                    Cbuf_AddText(localClientNum, v16);
                                                                                                                }
                                                                                                            }
                                                                                                            else
                                                                                                            {
                                                                                                                Menus_OpenByName(dc, "joinpb_popmenu");
                                                                                                            }
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
                                                                                                    UI_UpdateDisplayServers((uiInfo_s *)&uiInfoArray);
                                                                                                    if (*(int *)&sharedUiInfo.serverStatus.string[1128] >= 0
                                                                                                        && *(int *)&sharedUiInfo.serverStatus.string[1128] < *(int *)&sharedUiInfo.gap8EB4[72900])
                                                                                                    {
                                                                                                        LAN_GetServerAddressString(
                                                                                                            ui_netSource->current.integer,
                                                                                                            *(unsigned int *)&sharedUiInfo.gap8EB4[4
                                                                                                            * *(unsigned int *)&sharedUiInfo.serverStatus.string[1128]
                                                                                                            - 7100],
                                                                                                            sharedUiInfo.serverStatusAddress,
                                                                                                            64);
                                                                                                        UI_BuildServerStatus((uiInfo_s *)&uiInfoArray, 1);
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                            else
                                                                                            {
                                                                                                UI_StopServerRefresh();
                                                                                                *(unsigned int *)&sharedUiInfo.gap8EB4[72912] = 0;
                                                                                                sharedUiInfo.nextServerStatusRefresh = 0;
                                                                                                dc[1].localVars.table[79].u.integer = 0;
                                                                                            }
                                                                                        }
                                                                                        else if (*(unsigned int *)&sharedUiInfo.serverStatus.string[1124])
                                                                                        {
                                                                                            UI_StopServerRefresh();
                                                                                            *(unsigned int *)&sharedUiInfo.gap8EB4[72912] = 0;
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
                                                                                        Var = (dvar_s *)_Dvar_FindVar("fs_game");
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
                                                Dvar_SetStringByName("com_errorMessage", (char *)&String);
                                                Dvar_SetBoolByName("com_isNotice", 0);
                                                if (localClientNum)
                                                    MyAssertHandler(
                                                        "c:\\trees\\cod3\\src\\ui_mp\\../client_mp/client_mp.h",
                                                        1112,
                                                        0,
                                                        "%s\n\t(localClientNum) = %i",
                                                        "(localClientNum == 0)",
                                                        localClientNum);
                                                if (MEMORY[0xE7A7CC][0] > 0)
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
                Dvar_SetStringByName("cdkey1", (char *)&String);
                Dvar_SetStringByName("cdkey2", (char *)&String);
                Dvar_SetStringByName("cdkey3", (char *)&String);
                Dvar_SetStringByName("cdkey4", (char *)&String);
                Dvar_SetStringByName("cdkey5", (char *)&String);
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
    if (_Dvar_FindVar(dvarName))
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

void __cdecl UI_BuildServerDisplayList(uiInfo_s *uiInfo, int force)
{
    char *String; // eax
    char *v3; // eax
    char *v4; // eax
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
        I_strncpyz(&sharedUiInfo.gap8EB4[72944], String, 1024);
        len = strlen(&sharedUiInfo.gap8EB4[72944]);
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
            len = strlen(&sharedUiInfo.gap8EB4[72944]);
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
        UI_UpdateDisplayServers((uiInfo_s *)&uiInfoArray);
        return *(unsigned int *)&sharedUiInfo.gap8EB4[72900];
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
        if (uiInfoArray.realTime > unk_DB55F9C)
        {
            unk_DB55F9C = uiInfoArray.realTime + 3000;
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
        if (uiInfoArray.realTime > unk_DB55F9C)
        {
            unk_DB55F9C = uiInfoArray.realTime + 3000;
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
        return dword_DB55FA4;
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
    char *v1; // eax
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

char *__cdecl UI_FeederItemText(
    int localClientNum,
    itemDef_s *item,
    const float feederID,
    int index,
    unsigned int column,
    Material **handle)
{
    char *result; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    char *v11; // eax
    char *v12; // eax
    char *v13; // eax
    char *v14; // eax
    char *v15; // eax
    char *v16; // eax
    char *v17; // eax
    unsigned int ClientNumForPlayerListNum; // eax
    char *v19; // [esp-4h] [ebp-18h]
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
        Com_Error(ERR_FATAL, &byte_8B6CD0);
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
    Menus_OpenByName(&uiInfoArray, name);
}

void __cdecl UI_ListMenus_f()
{
    Menus_PrintAllLoadedMenus(&uiInfoArray);
}

void __cdecl CL_SelectStringTableEntryInDvar_f()
{
    const char *v0; // eax
    unsigned intv1; // eax
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
    Menus_CloseByName(&uiInfoArray, name);
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
    uiInfoArray.localClientNum = localClientNum;
    g_ingameMenusLoaded[localClientNum] = 0;
    if (useFastFile->current.enabled)
        DB_ResetZoneSize(0);
    if (!useFastFile->current.enabled)
        UI_LoadSoundAliases();
    UI_RegisterDvars();
    byte_DB561C4 = 1;
    String_Init();
    Menu_Setup(&uiInfoArray);
    CL_GetScreenDimensions(&uiInfoArray.screenWidth, &uiInfoArray.screenHeight, &uiInfoArray.screenAspect);
    if (480 * uiInfoArray.screenWidth <= 640 * uiInfoArray.screenHeight)
        uiInfoArray.bias = 0.0;
    else
        uiInfoArray.bias = ((double)uiInfoArray.screenWidth - (double)uiInfoArray.screenHeight * 1.333333373069763) * 0.5;
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
        menuList = UI_LoadMenus("ui_mp/code.txt", 3);
        UI_AddMenuList(&uiInfoArray, menuList);
    }
    if (!g_mapname[0] || !useFastFile->current.enabled)
    {
        menuLista = UI_LoadMenus("ui_mp/menus.txt", 3);
        UI_AddMenuList(&uiInfoArray, menuLista);
    }
    if (g_mapname[0] && !useFastFile->current.enabled)
    {
        v1 = va("maps/mp/%s.csv", g_mapname);
        UI_MapLoadInfo(v1);
    }
    UI_AssetCache();
    Menus_CloseAll(&uiInfoArray);
    sharedUiInfo.serverHardwareIconList[0] = Material_RegisterHandle("server_hardware_unknown", 3);
    sharedUiInfo.serverHardwareIconList[1] = Material_RegisterHandle("server_hardware_linux_dedicated", 3);
    sharedUiInfo.serverHardwareIconList[2] = Material_RegisterHandle("server_hardware_win_dedicated", 3);
    sharedUiInfo.serverHardwareIconList[3] = Material_RegisterHandle("server_hardware_mac_dedicated", 3);
    sharedUiInfo.serverHardwareIconList[6] = Material_RegisterHandle("server_hardware_win_listen", 3);
    sharedUiInfo.serverHardwareIconList[7] = Material_RegisterHandle("server_hardware_mac_listen", 3);
    LAN_LoadCachedServers();
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
    if (Menu_Count(&uiInfoArray))
    {
        menu = Menu_GetFocused(&uiInfoArray);
        if (!menu)
            goto LABEL_25;
        if (Dvar_GetBool("cl_bypassMouseInput") || UI_GetActiveMenu(localClientNum) == 10)
            bypassKeyClear = 1;
        if (key == 27 && down && !Menus_AnyFullScreenVisible(&uiInfoArray) && !menu->onESC)
            Menus_CloseAll(&uiInfoArray);
        if (Key_IsCatcherActive(uiInfoArray.localClientNum, 16))
            Menu_HandleKey(&uiInfoArray, menu, key, down);
        if (!Menu_GetFocused(&uiInfoArray))
        {
        LABEL_25:
            if (Key_IsCatcherActive(uiInfoArray.localClientNum, 16))
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

int __cdecl UI_GetActiveMenu(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return dword_DB561C0;
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
    topMenuStackIndex = uiInfoArray.openMenuCount - 1;
    if (topMenuStackIndex < 0 || topMenuStackIndex >= uiInfoArray.menuCount)
        return 0;
    if (!uiInfoArray.menuStack[topMenuStackIndex])
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 6325, 0, "%s", "uiInfo->uiDC.menuStack[topMenuStackIndex]");
    return uiInfoArray.menuStack[topMenuStackIndex]->window.name;
}

int __cdecl UI_SetActiveMenu(int localClientNum, int menu)
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
    if (Menu_Count(&uiInfoArray) <= 0)
        return 0;
    if (menu == 9)
        MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 6348, 0, "%s", "menu != UIMENU_SCRIPT_POPUP");
    dword_DB561C0 = menu;
    switch (menu)
    {
    case 0:
        Key_RemoveCatcher(localClientNum, -17);
        Dvar_SetIntByName("cl_paused", 0);
        Menus_CloseAll(&uiInfoArray);
        result = 1;
        break;
    case 1:
        if (localClientNum)
            MyAssertHandler(
                ".\\ui_mp\\ui_main_mp.cpp",
                6361,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        Key_SetCatcher(localClientNum, 16);
        Menus_OpenByName(&uiInfoArray, "main");
        buf = Dvar_GetString("com_errorMessage");
        if (strlen(buf))
        {
            if (I_stricmp(buf, ";"))
                Menus_OpenByName(&uiInfoArray, "error_popmenu");
        }
        SND_FadeAllSounds(1.0, 1000);
        result = 1;
        break;
    case 2:
        Key_SetCatcher(localClientNum, 16);
        Menus_CloseAll(&uiInfoArray);
        v3 = CG_ScriptMainMenu(uiInfoArray.localClientNum);
        if (!Menus_OpenByName(&uiInfoArray, v3))
            Menus_OpenByName(&uiInfoArray, "main");
        result = 1;
        break;
    case 3:
        Key_SetCatcher(localClientNum, 16);
        Menus_OpenByName(&uiInfoArray, "needcd");
        result = 1;
        break;
    case 4:
        Key_SetCatcher(localClientNum, 16);
        Menus_OpenByName(&uiInfoArray, "badcd");
        result = 1;
        break;
    case 5:
        if (!*Dvar_GetString("com_errorMessage"))
            MyAssertHandler(".\\ui_mp\\ui_main_mp.cpp", 6577, 0, "%s", "buf[0]");
        Key_SetCatcher(localClientNum, 16);
        Menus_CloseAll(&uiInfoArray);
        Menus_OpenByName(&uiInfoArray, "pregame_loaderror_mp");
        result = 1;
        break;
    case 7:
        uiInfoArray.cursor.x = 639.0;
        uiInfoArray.cursor.y = 479.0;
        UI_SetSystemCursorPos(&uiInfoArray, 639.0, 479.0);
        Key_SetCatcher(localClientNum, 16);
        CL_SetDisplayHUDWithKeycatchUI(uiInfoArray.localClientNum, 1);
        Menus_CloseAll(&uiInfoArray);
        Menus_OpenByName(&uiInfoArray, "quickmessage");
        result = 1;
        break;
    case 8:
        Menus_OpenByName(&uiInfoArray, "autoupdate");
        result = 1;
        break;
    case 10:
        Key_SetCatcher(localClientNum, 16);
        Menus_CloseAll(&uiInfoArray);
        Menus_OpenByName(&uiInfoArray, "scoreboard");
        bufa = Dvar_GetString("com_errorMessage");
        if (strlen(bufa) && I_stricmp(bufa, ";"))
            Menus_OpenByName(&uiInfoArray, "error_popmenu");
        result = 1;
        break;
    case 11:
        Key_SetCatcher(localClientNum, 16);
        Menus_CloseAll(&uiInfoArray);
        Menus_OpenByName(&uiInfoArray, "endofgame");
        bufb = Dvar_GetString("com_errorMessage");
        if (strlen(bufb) && I_stricmp(bufb, ";"))
            Menus_OpenByName(&uiInfoArray, "error_popmenu");
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
    return Menus_AnyFullScreenVisible(&uiInfoArray);
}

void __cdecl UI_DrawConnectScreen(int localClientNum)
{
    char *v1; // eax
    char *v2; // eax
    char *MapDisplayName; // eax
    char *v4; // eax
    char *v5; // eax
    const char *String; // [esp+18h] [ebp-DC8h]
    DvarValue *p_current; // [esp+18h] [ebp-DC8h]
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
    return uiInfoArray.blurRadiusOut;
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
        *(unsigned int *)&sharedUiInfo.serverStatus.string[1124] = 1;
        *(unsigned int *)&sharedUiInfo.gap8EB4[72912] = uiInfoArray.realTime + 1000;
        UI_ClearDisplayedServers();
        LAN_MarkServerDirty(ui_netSource->current.integer, 0xFFFFFFFF, 1u);
        LAN_ResetPings(ui_netSource->current.integer);
        if (ui_netSource->current.integer)
        {
            *(unsigned int *)&sharedUiInfo.serverStatus.string[1104] = uiInfoArray.realTime + 5000;
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
            Cmd_ExecuteSingleCommand(localClientNum, v4, "localservers\n");
            *(unsigned int *)&sharedUiInfo.serverStatus.string[1104] = uiInfoArray.realTime + 1000;
        }
    }
    else
    {
        UI_UpdatePendingPings((uiInfo_s *)&uiInfoArray);
    }
}

void __cdecl UI_UpdatePendingPings(uiInfo_s *uiInfo)
{
    LAN_ResetPings(ui_netSource->current.integer);
    *(unsigned int *)&sharedUiInfo.serverStatus.string[1124] = 1;
    *(unsigned int *)&sharedUiInfo.serverStatus.string[1104] = uiInfo->uiDC.realTime + 1000;
}

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

bool __cdecl UI_AnyMenuVisible(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return uiInfoArray.openMenuCount != 0;
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
    strstr((unsigned __int8 *)sourceString, asc_861D24);
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
            if (!strncmp(&sourceString[index], asc_861D24, 2u) && isdigit(sourceString[index + 2]))
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
    Menus_CloseAll(&uiInfoArray);
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
    if (Menu_Count(&uiInfoArray) > 0)
    {
        if (Menu_GetFocused(&uiInfoArray))
        {
            if (!Menus_AnyFullScreenVisible(&uiInfoArray))
                Menus_CloseAll(&uiInfoArray);
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
    pFocus = Menu_GetFocused(&uiInfoArray);
    if (pFocus && dword_DB561C0 != 9 && dword_DB561C0 != 10)
        return 0;
    if (!pFocus || I_stricmp(pFocus->window.name, menuName))
    {
        dword_DB561C0 = 9;
        if (!useMouse)
        {
            uiInfoArray.cursor.x = 639.0;
            uiInfoArray.cursor.y = 479.0;
            UI_SetSystemCursorPos(&uiInfoArray, 639.0, 479.0);
        }
        Key_SetCatcher(localClientNum, 16);
        Menus_CloseAll(&uiInfoArray);
        Menus_OpenByName(&uiInfoArray, menuName);
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
    if (dword_DB561C0 == 9)
    {
        byte_DB561C4 = allowResponse;
        UI_CloseFocusedMenu(localClientNum);
        byte_DB561C4 = 1;
    }
}

char __cdecl UI_AllowScriptMenuResponse(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\ui_mp\\ui_main_mp.cpp",
            332,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return byte_DB561C4;
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
    menu = Menus_FindByName(&uiInfoArray, menuName);
    if (!menu)
        return 0;
    if (Menus_MenuIsInStack(&uiInfoArray, menu))
        return Menu_IsVisible(&uiInfoArray, menu) != 0;
    return 0;
}

bool __cdecl UI_ShouldDrawCrosshair()
{
    return ui_drawCrosshair->current.enabled;
}

