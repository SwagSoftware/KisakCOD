#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

// KISAKTODO: Cleanup SP UI after 'client/` is done

#include "ui.h"

const dvar_t *ui_showList;
const dvar_t *ui_isSaving;
const dvar_t *ui_startupActiveController;
const dvar_t *ui_skipMainLockout;
const dvar_t *ui_useSuggestedWeapons;
const dvar_t *ui_saveMessageMinTime;
const dvar_t *ui_showMenuOnly;
const dvar_t *ui_bigFont;
const dvar_t *ui_cinematicsTimestamp;
const dvar_t *ui_mousePitch;
const dvar_t *ui_extraBigFont;
const dvar_t *ui_nextMission;
const dvar_t *uiscript_debug;
const dvar_t *ui_autoContinue;
const dvar_t *ui_smallFont;
const dvar_t *ui_hideMap;
const dvar_t *ui_savegame;
const dvar_t *ui_drawCrosshairNames;
const dvar_t *ui_borderLowLightScale;
const dvar_t *ui_campaign;

static char g_mapname[64];
static char g_gametype[64];
static int ui_serverFilterType;
static bool g_ingameMenusLoaded;

uiInfo_s uiInfo;
sharedUiInfo_t sharedUiInfo;
SaveTimeGlob ui_saveTimeGlob;


UILocalVarContext *__cdecl UI_GetLocalVarsContext(int localClientNum)
{
    return &uiInfo.uiDC.localVars;
}

const dvar_s *UI_RegisterDvars()
{
    double Float; // fp1
    const char *v1; // r5
    unsigned __int16 v2; // r4
    const char *v3; // r5
    unsigned __int16 v4; // r4
    const char *v5; // r5
    unsigned __int16 v6; // r4
    const char *v7; // r5
    unsigned __int16 v8; // r4
    const char *v9; // r5
    unsigned __int16 v10; // r4
    const dvar_s *result; // r3

    Dvar_RegisterBool("cg_brass", 1, 1u, 0);
    Dvar_RegisterBool("fx_marks", 1, 1u, 0);
    Float = Dvar_GetFloat("m_pitch");
    Dvar_RegisterBool("ui_mousePitch", Float < 0.0, 0x201u, "Invert mouse pitch");
    ui_smallFont = Dvar_RegisterFloat("ui_smallFont", 0.25, 0.0, 1.0, v2, v1);
    ui_bigFont = Dvar_RegisterFloat("ui_bigFont", 0.4f, 0.0, 1.0, v4, v3);
    ui_extraBigFont = Dvar_RegisterFloat("ui_extraBigFont", 0.55000001, 0.0, 1.0, v6, v5);
    ui_campaign = Dvar_RegisterString("ui_campaign", "american", 0x1000u, "Current campaign");
    ui_nextMission = Dvar_RegisterInt("ui_nextMission", 0, 0, 3, 0x1000u, "Next mission");
    ui_savegame = Dvar_RegisterString("ui_savegame", "", 0, "Save game name");
    ui_autoContinue = Dvar_RegisterBool(
        "ui_autoContinue",
        0,
        0,
        "Automatically 'click to continue' after loading a level");
    ui_showList = Dvar_RegisterBool("ui_showList", 0, 0x80u, "Show list of currently visible menus");
    ui_showMenuOnly = Dvar_RegisterString(
        "ui_showMenuOnly",
        "",
        0,
        "If set, only menus using this name will draw.");
    ui_hideMap = Dvar_RegisterBool(
        "ui_hideMap",
        0,
        0x1000u,
        "Meant to be set by script and referenced by menu files to determine if minimap should be drawn.");
    ui_isSaving = Dvar_RegisterBool("ui_isSaving", 0, 0x40u, "True if the game is currently saving");
    ui_saveMessageMinTime = Dvar_RegisterFloat("ui_saveMessageMinTime", 1.0, 0.0, 3.0, v8, v7);
    ui_borderLowLightScale = Dvar_RegisterFloat("ui_borderLowLightScale", 0.6f, 0.0, 1.0, v10, v9);
    result = Dvar_RegisterBool("ui_cinematicsTimestamp", 0, 0, "Shows cinematics timestamp on subtitle UI elements.");
    ui_cinematicsTimestamp = result;
    return result;
}

void __cdecl TRACK_ui_main()
{
    track_static_alloc_internal(&uiInfo, 37664, "uiInfo", 34);
}

Font_s *UI_AssetCache()
{
    Font_s *result; // r3

    sharedUiInfo.assets.whiteMaterial = Material_RegisterHandle("white", 3);
    sharedUiInfo.assets.scrollBar = Material_RegisterHandle("ui_scrollbar", 3);
    sharedUiInfo.assets.scrollBarArrowDown = Material_RegisterHandle("ui_scrollbar_arrow_dwn_a", 3);
    sharedUiInfo.assets.scrollBarArrowUp = Material_RegisterHandle("ui_scrollbar_arrow_up_a", 3);
    sharedUiInfo.assets.scrollBarArrowLeft = Material_RegisterHandle("ui_scrollbar_arrow_left", 3);
    sharedUiInfo.assets.scrollBarArrowRight = Material_RegisterHandle("ui_scrollbar_arrow_right", 3);
    sharedUiInfo.assets.scrollBarThumb = Material_RegisterHandle("ui_scrollbar_thumb", 3);
    sharedUiInfo.assets.sliderBar = Material_RegisterHandle("ui_slider2", 3);
    sharedUiInfo.assets.sliderThumb = Material_RegisterHandle("ui_sliderbutt_1", 3);
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

void __cdecl UI_DrawSides(
    const ScreenPlacement *scrPlace,
    double x,
    double y,
    double w,
    double h,
    int horzAlign,
    int vertAlign,
    double size,
    const float *color,
    Material *a10)
{
    Material *v16; // r7
    const float *v17; // r6
    int v18; // r5
    int v19; // r4

    CL_DrawStretchPic(scrPlace, x, y, size, h, horzAlign, vertAlign, 0.0, 0.0, 0.0, 0.0, color, a10);
    CL_DrawStretchPic(
        scrPlace,
        (float)((float)((float)x + (float)w) - (float)size),
        y,
        size,
        h,
        v19,
        v18,
        0.0,
        0.0,
        0.0,
        0.0,
        v17,
        v16);
}

void __cdecl UI_DrawTopBottom(
    const ScreenPlacement *scrPlace,
    double x,
    double y,
    double w,
    double h,
    int horzAlign,
    int vertAlign,
    double size,
    const float *color,
    Material *a10)
{
    Material *v16; // r7
    const float *v17; // r6
    int v18; // r5
    int v19; // r4

    CL_DrawStretchPic(scrPlace, x, y, w, size, horzAlign, vertAlign, 0.0, 0.0, 0.0, 0.0, color, a10);
    CL_DrawStretchPic(
        scrPlace,
        x,
        (float)((float)((float)y + (float)h) - (float)size),
        w,
        size,
        v19,
        v18,
        0.0,
        0.0,
        0.0,
        0.0,
        v17,
        v16);
}

void __cdecl UI_DrawRect(
    const ScreenPlacement *scrPlace,
    double x,
    double y,
    double width,
    double height,
    int horzAlign,
    int vertAlign,
    double size,
    const float *color,
    Material *a10)
{
    Material *v16; // r7
    const float *v17; // r6
    int v18; // r5
    int v19; // r4

    UI_DrawTopBottom(scrPlace, x, y, width, height, horzAlign, vertAlign, size, color, a10);
    UI_DrawSides(
        scrPlace,
        x,
        (float)((float)y + (float)size),
        width,
        (float)-(float)((float)((float)size * (float)2.0) - (float)height),
        v19,
        v18,
        size,
        v17,
        v16);
}

void __cdecl UI_DrawHighlightRect(
    const ScreenPlacement *scrPlace,
    double x,
    double y,
    double w,
    double h,
    int horzAlign,
    int vertAlign,
    double size,
    const float *hiColor,
    const float *loColor,
    int a11,
    int a12,
    double a13,
    double a14,
    double a15,
    int a16,
    float a17,
    float a18,
    float a19,
    float a20,
    float a21,
    float a22,
    float a23,
    float a24,
    float a25,
    float a26,
    float a27,
    float a28,
    float a29,
    float a30,
    float a31,
    float a32,
    float a33,
    float a34,
    float a35,
    const float *a36,
    float a37,
    const float *a38)
{
    const float *v41; // r30
    const float *v42; // r30
    float v43; // [sp+50h] [-60h] BYREF
    float v44; // [sp+54h] [-5Ch] BYREF
    float v45; // [sp+58h] [-58h] BYREF
    float v46; // [sp+5Ch] [-54h] BYREF
    float v47[2]; // [sp+60h] [-50h] BYREF
    float v48; // [sp+68h] [-48h]
    float v49; // [sp+6Ch] [-44h]
    float v50; // [sp+70h] [-40h]
    float v51; // [sp+74h] [-3Ch]
    float v52; // [sp+78h] [-38h]
    float v53; // [sp+7Ch] [-34h]

    a22 = x;
    a24 = y;
    a26 = w;
    a28 = h;
    v44 = size;
    v43 = size;
    ScrPlace_ApplyRect(scrPlace, &a22, &a24, &a26, &a28, a11, a12);
    v46 = 0.0;
    v45 = 0.0;
    ScrPlace_ApplyRect(scrPlace, &v46, &v45, &v44, &v43, a11, a12);
    v47[1] = a24;
    v49 = a24;
    v41 = a36;
    v47[0] = a22;
    v52 = v44 + a22;
    v48 = a22 + a26;
    v51 = v43 + a24;
    v53 = v51;
    v50 = v48 - v44;
    R_AddCmdDrawQuadPic((const float (*)[2])v47, a36, sharedUiInfo.assets.whiteMaterial);
    v47[0] = a22;
    v47[1] = a24;
    v52 = a22;
    v49 = v43 + a24;
    v48 = v44 + a22;
    v50 = v48;
    v53 = a24 + a28;
    v51 = v53 - v43;
    R_AddCmdDrawQuadPic((const float (*)[2])v47, v41, sharedUiInfo.assets.whiteMaterial);
    v42 = a38;
    v47[0] = a22;
    v48 = v44 + a22;
    v47[1] = a24 + a28;
    v50 = (float)(a22 + a26) - v44;
    v52 = a22 + a26;
    v53 = v47[1];
    v49 = v47[1] - v43;
    v51 = v49;
    R_AddCmdDrawQuadPic((const float (*)[2])v47, a38, sharedUiInfo.assets.whiteMaterial);
    v47[1] = a24;
    v53 = v43 + a24;
    v47[0] = a22 + a26;
    v48 = v47[0];
    v49 = a24 + a28;
    v50 = v47[0] - v44;
    v52 = v50;
    v51 = v49 - v43;
    R_AddCmdDrawQuadPic((const float (*)[2])v47, v42, sharedUiInfo.assets.whiteMaterial);
}

int __cdecl UI_TextWidth(const char *text, int maxChars, Font_s *font, double scale)
{
    double v7; // fp31
    __int64 v8; // r11
    long double v9; // fp2
    long double v10; // fp2

    v7 = R_NormalizedTextScale(font, scale);
    LODWORD(v8) = R_TextWidth(text, maxChars, font);
    *(double *)&v9 = (float)((float)((float)v8 * (float)v7) + (float)0.5);
    v10 = floor(v9);
    return (int)(float)*(double *)&v10;
}

int __cdecl UI_TextHeight(Font_s *font, double scale)
{
    double v3; // fp31
    __int64 v4; // r11
    long double v5; // fp2
    long double v6; // fp2

    v3 = R_NormalizedTextScale(font, scale);
    LODWORD(v4) = R_TextHeight(font);
    *(double *)&v5 = (float)((float)((float)v4 * (float)v3) + (float)0.5);
    v6 = floor(v5);
    return (int)(float)*(double *)&v6;
}

void __cdecl UI_DrawText(
    const ScreenPlacement *scrPlace,
    const char *text,
    int maxChars,
    Font_s *font,
    double x,
    double y,
    int horzAlign,
    int vertAlign,
    double scale,
    const float *color,
    int style,
    double a12,
    double a13,
    double a14,
    double a15,
    double a16,
    float a17,
    float a18,
    float a19,
    float a20,
    float a21,
    float a22,
    float a23,
    float a24,
    float a25,
    float a26,
    float a27,
    float a28,
    float a29,
    float a30)
{
    long double v36; // fp2
    long double v37; // fp2
    long double v38; // fp2
    int v39; // r7
    const float *v40; // r6
    float v41; // [sp+60h] [-50h] BYREF
    float v42[3]; // [sp+64h] [-4Ch] BYREF

    a28 = x;
    a30 = y;
    v42[0] = R_NormalizedTextScale(font, scale);
    v41 = v42[0];
    ScrPlace_ApplyRect(scrPlace, &a28, &a30, v42, &v41, (int)color, style);
    *(double *)&v36 = (float)(a28 + (float)0.5);
    v37 = floor(v36);
    a28 = *(double *)&v37;
    *(double *)&v37 = (float)(a30 + (float)0.5);
    v38 = floor(v37);
    a30 = *(double *)&v38;
    CL_DrawTextPhysical(text, maxChars, font, a28, (float)*(double *)&v38, v42[0], v41, v40, v39);
}

void __cdecl UI_DrawTextWithGlow(
    const ScreenPlacement *scrPlace,
    const char *text,
    int maxChars,
    Font_s *font,
    double x,
    double y,
    int horzAlign,
    int vertAlign,
    double scale,
    const float *color,
    int style,
    double a12,
    double a13,
    double a14,
    double a15,
    double a16,
    const float *glowColor,
    bool subtitle,
    bool cinematic,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    float a28,
    int a29,
    float a30,
    int a31,
    int a32,
    int a33,
    int a34,
    int a35,
    int a36,
    int a37,
    Material *a38,
    int a39,
    int a40,
    int a41,
    int a42,
    int a43,
    char a44)
{
    long double v50; // fp2
    long double v51; // fp2
    Material *v52; // r9
    const float *v53; // r8
    int v54; // r7
    const float *v55; // r6
    long double v56; // fp2
    double v57; // fp2
    double v58; // fp4
    double v59; // fp3
    double v60; // fp1
    int v61; // [sp+8h] [-D8h]
    int v62; // [sp+Ch] [-D4h]
    int v63; // [sp+10h] [-D0h]
    int v64; // [sp+14h] [-CCh]
    float v65; // [sp+90h] [-50h] BYREF
    float v66[3]; // [sp+94h] [-4Ch] BYREF

    a28 = x;
    a30 = y;
    v66[0] = R_NormalizedTextScale(font, scale);
    v65 = v66[0];
    ScrPlace_ApplyRect(scrPlace, &a28, &a30, v66, &v65, (int)color, style);
    *(double *)&v50 = (float)(a28 + (float)0.5);
    v51 = floor(v50);
    a28 = *(double *)&v51;
    *(double *)&v51 = (float)(a30 + (float)0.5);
    v56 = floor(v51);
    v57 = (float)*(double *)&v56;
    v58 = v65;
    v59 = v66[0];
    a30 = *(double *)&v56;
    v60 = a28;
    if (a44)
        R_AddCmdDrawTextSubtitle(text, maxChars, font, v60, v57, v59, v58, 0.0, v55, v54, v53, (bool)v52);
    else
        CL_DrawTextPhysicalWithEffects(
            text,
            maxChars,
            font,
            v60,
            v57,
            v59,
            v58,
            v55,
            v54,
            v53,
            v52,
            a38,
            v61,
            v62,
            v63,
            v64);
}

void __cdecl UI_DrawTextNoSnap(
    const ScreenPlacement *scrPlace,
    const char *text,
    int maxChars,
    Font_s *font,
    double x,
    double y,
    int horzAlign,
    int vertAlign,
    double scale,
    const float *color,
    int style,
    double a12,
    double a13,
    double a14,
    double a15,
    double a16,
    float a17,
    float a18,
    float a19,
    float a20,
    float a21,
    float a22,
    float a23,
    float a24,
    float a25,
    float a26,
    float a27,
    float a28,
    float a29,
    float a30)
{
    int v36; // r7
    const float *v37; // r6
    float v38; // [sp+60h] [-40h] BYREF
    float v39[15]; // [sp+64h] [-3Ch] BYREF

    a28 = x;
    a30 = y;
    v39[0] = R_NormalizedTextScale(font, scale);
    v38 = v39[0];
    ScrPlace_ApplyRect(scrPlace, &a28, &a30, v39, &v38, (int)color, style);
    CL_DrawTextPhysical(text, maxChars, font, a28, a30, v39[0], v38, v37, v36);
}

void __cdecl UI_DrawTextWithCursor(
    const ScreenPlacement *scrPlace,
    const char *text,
    int maxChars,
    Font_s *font,
    double x,
    double y,
    int horzAlign,
    int vertAlign,
    double scale,
    const float *color,
    int style,
    double a12,
    double a13,
    double a14,
    double a15,
    double a16,
    int cursorPos,
    char cursor,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    float a28,
    int a29,
    float a30)
{
    long double v36; // fp2
    long double v37; // fp2
    long double v38; // fp2
    char v39; // r9
    int v40; // r8
    int v41; // r7
    const float *v42; // r6
    float v43; // [sp+70h] [-50h] BYREF
    float v44[3]; // [sp+74h] [-4Ch] BYREF

    a28 = x;
    a30 = y;
    v44[0] = R_NormalizedTextScale(font, scale);
    v43 = v44[0];
    ScrPlace_ApplyRect(scrPlace, &a28, &a30, v44, &v43, (int)color, style);
    *(double *)&v36 = (float)(a28 + (float)0.5);
    v37 = floor(v36);
    a28 = *(double *)&v37;
    *(double *)&v37 = (float)(a30 + (float)0.5);
    v38 = floor(v37);
    a30 = *(double *)&v38;
    CL_DrawTextPhysicalWithCursor(text, maxChars, font, a28, (float)*(double *)&v38, v44[0], v43, v42, v41, v40, v39);
}

Font_s *__cdecl UI_GetFontHandle(const ScreenPlacement *scrPlace, int fontEnum, double scale)
{
    double value; // fp0

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
    if (R_IsHiDef())
        scale = (float)((float)scale * (float)1.5);
    value = ui_smallFont->current.value;
    if (fontEnum == 4)
    {
        if (scale > value)
        {
            if (scale >= ui_bigFont->current.value)
                return sharedUiInfo.assets.boldFont;
            return sharedUiInfo.assets.textFont;
        }
        return sharedUiInfo.assets.smallFont;
    }
    if (scale <= value)
        return sharedUiInfo.assets.smallFont;
    if (scale >= ui_extraBigFont->current.value)
        return sharedUiInfo.assets.extraBigFont;
    if (scale >= ui_bigFont->current.value)
        return sharedUiInfo.assets.bigFont;
    return sharedUiInfo.assets.textFont;
}

void UI_UpdateSaveUI()
{
    int v0; // r3
    int v1; // r3
    int v2; // [sp+50h] [-20h]

    if (!ui_saveTimeGlob.hasfirstFrameShown)
    {
        v0 = Sys_Milliseconds();
        Com_Printf(13, "Save Message First Frame Shown: %i\n", v0);
        ui_saveTimeGlob.hasfirstFrameShown = 1;
    }
    if (ui_saveTimeGlob.callWrite)
    {
        Cbuf_AddText(0, "savegame_lastcommit\n");
        ui_saveTimeGlob.callWrite = 0;
    }
    if (ui_saveTimeGlob.isSaving)
    {
        v2 = (int)(float)(ui_saveMessageMinTime->current.value * (float)1000.0);
        if (Sys_Milliseconds() - ui_saveTimeGlob.saveTime > v2 && !SaveDevice_IsAccessingDevice())
        {
            if (!ui_saveTimeGlob.saveMenuName)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp", 477, 0, "%s", "ui_saveTimeGlob.saveMenuName");
            ui_saveTimeGlob.saveTime = 0;
            ui_saveTimeGlob.isSaving = 0;
            Dvar_SetBool(ui_isSaving, 0);
            Menus_CloseByName(&uiInfo.uiDC, ui_saveTimeGlob.saveMenuName);
            v1 = Sys_Milliseconds();
            Com_Printf(13, "Save Message Last Frame Shown: %i\n", v1);
        }
    }
}

void __cdecl UI_UpdateTime(int realtime)
{
    int v1; // r9
    signed int v2; // r10
    __int64 v3; // r10

    v1 = realtime - uiInfo.uiDC.realTime;
    uiInfo.uiDC.frameTime = realtime - uiInfo.uiDC.realTime;
    uiInfo.uiDC.realTime = realtime;
    uiInfo.previousTimes[uiInfo.timeIndex++ % 4] = v1;
    if (uiInfo.timeIndex > 4)
    {
        v2 = uiInfo.previousTimes[0] + uiInfo.previousTimes[1] + uiInfo.previousTimes[2] + uiInfo.previousTimes[3];
        if (!v2)
            v2 = 1;
        HIDWORD(v3) = 4000;
        __twllei(v2, 0);
        LODWORD(v3) = 4000 / v2;
        uiInfo.uiDC.FPS = (float)v3;
    }
}

void __cdecl UI_Shutdown()
{
    Menus_CloseAll(&uiInfo.uiDC);
    UILocalVar_Shutdown(&uiInfo.uiDC.localVars);
    Cmd_RemoveCommand("openmenu");
    Cmd_RemoveCommand("closemenu");
}

MenuList *__cdecl Load_ScriptMenuInternal(const char *pszMenu, int imageTrack)
{
    char v4[264]; // [sp+50h] [-120h] BYREF

    strcpy(v4, "ui/scriptmenus/");
    I_strncat(v4, 256, pszMenu);
    I_strncat(v4, 256, ".menu");
    return UI_LoadMenu(v4, imageTrack);
}

MenuList *__cdecl Load_ScriptMenu(const char *pszMenu, int imageTrack)
{
    MenuList *result; // r3

    result = Load_ScriptMenuInternal(pszMenu, imageTrack);
    if (result)
    {
        UI_AddMenuList(&uiInfo.uiDC, result);
        return (MenuList *)1;
    }
    return result;
}

int __cdecl UI_SavegameIndexFromFilename(const char *filename)
{
    int v2; // r29
    int *i; // r31

    if (!filename)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp", 635, 0, "%s", "filename");
    v2 = 0;
    if (uiInfo.savegameCount <= 0)
        return -1;
    for (i = uiInfo.savegameStatus.displaySavegames; I_stricmp(filename, uiInfo.savegameList[*i].savegameFile); ++i)
    {
        if (++v2 >= uiInfo.savegameCount)
            return -1;
    }
    return v2;
}

int __cdecl UI_SavegameIndexFromFilename2(const char *filename)
{
    int v1; // r3

    v1 = UI_SavegameIndexFromFilename(filename);
    if (v1 < 0)
        return -1;
    else
        return uiInfo.savegameStatus.displaySavegames[v1];
}

void __cdecl UI_DrawSaveGameShot(rectDef_s *rect, double scale, float *color)
{
    int v4; // r3
    int v5; // r30
    Material *v6; // r7
    const float *v7; // r6
    int v8; // r5
    int v9; // r4
    Material *sshotImage; // r3
    int v11; // r30

    v4 = UI_SavegameIndexFromFilename(uiInfo.savegameName);
    if (v4 >= 0 && (v5 = uiInfo.savegameStatus.displaySavegames[v4], v5 >= 0))
    {
        sshotImage = uiInfo.sshotImage;
        if (uiInfo.sshotImage && uiInfo.savegameList[v5].imageName)
        {
            if (!I_strnicmp(uiInfo.savegameList[v5].imageName, uiInfo.sshotImageName, 64))
                goto LABEL_14;
            sshotImage = uiInfo.sshotImage;
        }
        v11 = v5 << 6;
        if (*(const char **)((char *)&uiInfo.savegameList[0].imageName + v11))
        {
            sshotImage = Material_RegisterRawImage(*(const char **)((char *)&uiInfo.savegameList[0].imageName + v11), 3);
            uiInfo.sshotImage = sshotImage;
        }
        if (!*(const char **)((char *)&uiInfo.savegameList[0].imageName + v11) || !sshotImage)
            uiInfo.sshotImage = Material_RegisterHandle("unknownsave", 3);
        I_strncpyz(uiInfo.sshotImageName, *(const char **)((char *)&uiInfo.savegameList[0].imageName + v11), 64);
    }
    else
    {
        Material_RegisterHandle("unknownsave", 3);
    }
LABEL_14:
    UI_DrawHandlePic(&scrPlaceFull, rect->x, rect->y, rect->w, rect->h, v9, v8, v7, v6);
}

void UI_DrawCinematic()
{
    R_Cinematic_DrawStretchPic_Letterboxed(0);
}

void __cdecl UI_LoadIngameMenus()
{
    MenuList *Menus; // r3

    iassert(!g_ingameMenusLoaded);
    g_ingameMenusLoaded = 1;
    Menus = UI_LoadMenus((char*)"ui/ingame.txt", 3);
    UI_AddMenuList(&uiInfo.uiDC, Menus);
}

void __cdecl UI_SetMap(const char *mapname)
{
    I_strncpyz(g_mapname, mapname, 64);
}

int __cdecl UI_OwnerDrawVisible(int flags)
{
    return 1;
}

int __cdecl UI_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key)
{
    return 0;
}

int __cdecl UI_CompareTimes(qtime_s *tm1, qtime_s *tm2)
{
    int tm_year; // r9
    int v4; // r10
    int result; // r3
    int tm_yday; // r10
    int v7; // r9
    int tm_hour; // r10
    int v9; // r9
    int tm_min; // r10
    int v11; // r9

    tm_year = tm2->tm_year;
    v4 = tm1->tm_year;
    result = v4 - tm_year;
    if (v4 == tm_year)
    {
        tm_yday = tm1->tm_yday;
        v7 = tm2->tm_yday;
        result = tm_yday - v7;
        if (tm_yday == v7)
        {
            tm_hour = tm1->tm_hour;
            v9 = tm2->tm_hour;
            result = tm_hour - v9;
            if (tm_hour == v9)
            {
                tm_min = tm1->tm_min;
                v11 = tm2->tm_min;
                result = tm_min - v11;
                if (tm_min == v11)
                    return tm1->tm_sec - tm2->tm_sec;
            }
        }
    }
    return result;
}

int __cdecl UI_SavegamesQsortCompare(unsigned int *arg1, unsigned int *arg2)
{
    int result; // r3
    SavegameInfo *v3; // r10
    SavegameInfo *v4; // r9

    if (*arg1 == *arg2)
        return 0;
    v3 = &uiInfo.savegameList[*arg1];
    v4 = &uiInfo.savegameList[*arg2];
    if (uiInfo.savegameStatus.sortKey)
    {
        if (uiInfo.savegameStatus.sortKey == 1)
            result = UI_CompareTimes(&v4->tm, &v3->tm);
        else
            result = 0;
    }
    else
    {
        result = I_stricmp(v4->savegameName, v3->savegameName);
    }
    if (!uiInfo.savegameStatus.sortDir)
        return -result;
    return result;
}

void __cdecl UI_Update(const char *name)
{
    const char *v2; // r3
    const char *String; // r3
    double v4; // fp1

    if (I_stricmp(name, "ui_SetName"))
    {
        if (I_stricmp(name, "ui_GetName"))
        {
            if (!I_stricmp(name, "ui_mousePitch"))
            {
                if (Dvar_GetBool(name))
                    v4 = -0.022;
                else
                    v4 = 0.022;
                Dvar_SetFloatByName("m_pitch", v4);
            }
        }
        else
        {
            String = Dvar_GetString("name");
            Dvar_SetStringByName("ui_Name", String);
        }
    }
    else
    {
        v2 = Dvar_GetString("ui_Name");
        Dvar_SetStringByName("name", v2);
    }
}

void UI_SaveComplete()
{
    int v0; // r3
    const char *v1; // r3

    if (SaveDevice_IsSaveSuccessful())
    {
        if (Live_HasAcceptedInvitation())
        {
            v0 = CL_ControllerIndexFromClientNum(0);
            Live_ConfirmAcceptInvitation(v0);
        }
        else
        {
            if (ui_saveTimeGlob.hasExecOnSuccess)
            {
                v1 = va("%s\n", ui_saveTimeGlob.execOnSuccess);
                Cbuf_AddText(0, v1);
            }
            memset(ui_saveTimeGlob.execOnSuccess, 0, sizeof(ui_saveTimeGlob.execOnSuccess));
            ui_saveTimeGlob.hasExecOnSuccess = 0;
        }
    }
    else if (Live_HasAcceptedInvitation())
    {
        Live_DeclineInvitation();
    }
}

void *UI_SaveRevert()
{
    void *result; // r3

    if (Live_HasAcceptedInvitation())
        Live_DeclineInvitation();
    result = memset(ui_saveTimeGlob.execOnSuccess, 0, sizeof(ui_saveTimeGlob.execOnSuccess));
    ui_saveTimeGlob.hasExecOnSuccess = 0;
    return result;
}

void UI_VerifyLanguage()
{
    int Int; // r29
    int v1; // r31
    int v2; // r4

    Int = Dvar_GetInt("loc_language");
    v1 = Dvar_GetInt("ui_language");
    v2 = SEH_VerifyLanguageSelection(v1);
    if (v2 != v1)
        Dvar_SetIntByName("ui_language", v2);
    Dvar_SetBoolByName("ui_languagechanged", v1 != Int);
}

int __cdecl UI_GetOpenOrCloseMenuOnDvarArgs(
    const char **args,
    const char *cmd,
    char *dvarName,
    char *testValue,
    char *menuName)
{
    if (String_Parse(args, dvarName, 1024))
    {
        if (String_Parse(args, testValue, 1024))
        {
            if (String_Parse(args, menuName, 1024))
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

bool __cdecl UI_DvarValueTest(const char *cmd, const char *dvarName, const char *testValue, bool wantMatch)
{
    const char *VariantString; // r3

    if (!cmd)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp", 1609, 0, "%s", "cmd");
    if (!dvarName)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp", 1610, 0, "%s", "dvarName");
    if (!testValue)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp", 1611, 0, "%s", "testValue");
    if (Dvar_FindVar(dvarName))
    {
        VariantString = Dvar_GetVariantString(dvarName);
        return (__PAIR64__(wantMatch, I_stricmp(testValue, VariantString)) - 1) >> 32 == 0;
    }
    else
    {
        Com_Printf(13, "%s: cannot find dvar %s\n", cmd, dvarName);
        return 0;
    }
}

void __cdecl UI_OpenMenuOnDvar(const char *cmd, const char *menuName, const char *dvarName, const char *testValue)
{
    int v8; // r3

    if (!cmd)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp", 1630, 0, "%s", "cmd");
    if (I_stricmp(cmd, "openMenuOnDvar") && I_stricmp(cmd, "openMenuOnDvarNot"))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp",
            1631,
            0,
            "%s\n\t(cmd) = %s",
            "(!I_stricmp( cmd, \"openMenuOnDvar\" ) || !I_stricmp( cmd, \"openMenuOnDvarNot\" ))",
            cmd);
    if (!menuName)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp", 1632, 0, "%s", "menuName");
    v8 = I_stricmp(cmd, "openMenuOnDvar");
    if (UI_DvarValueTest(cmd, dvarName, testValue, v8 == 0))
        Menus_OpenByName(&uiInfo.uiDC, menuName);
}

void __cdecl UI_CloseMenuOnDvar(const char *cmd, const char *menuName, const char *dvarName, const char *testValue)
{
    int v8; // r3

    if (!cmd)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp", 1644, 0, "%s", "cmd");
    if (I_stricmp(cmd, "closeMenuOnDvar") && I_stricmp(cmd, "closeMenuOnDvarNot"))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp",
            1645,
            0,
            "%s\n\t(cmd) = %s",
            "(!I_stricmp( cmd, \"closeMenuOnDvar\" ) || !I_stricmp( cmd, \"closeMenuOnDvarNot\" ))",
            cmd);
    if (!menuName)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp", 1646, 0, "%s", "menuName");
    v8 = I_stricmp(cmd, "closeMenuOnDvar");
    if (UI_DvarValueTest(cmd, dvarName, testValue, v8 == 0))
        Menus_CloseByName(&uiInfo.uiDC, menuName);
}

bool __cdecl UI_AutoContinue()
{
    return CL_TimeDemoPlaying() || ui_autoContinue->current.color[0] != 0;
}

void __cdecl UI_OverrideCursorPos(int localClientNum, itemDef_s *item)
{
    listBoxDef_s *ListBoxDef; // r29
    int v5; // r3

    if (item->special == 30.0)
    {
        ListBoxDef = Item_GetListBoxDef(item);
        if (!ListBoxDef)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp", 1942, 0, "%s", "listPtr");
        v5 = Item_ListBox_Viewmax(item);
        LB_OverrideCursorPos(v5, &item->cursorPos[localClientNum], &ListBoxDef->startPos[localClientNum]);
    }
}

int __cdecl UI_FeederCount(int localClientNum, double feederID)
{
    if (feederID == 30.0)
        return LB_FeederCount(localClientNum);
    else
        return 0;
}

char *__cdecl UI_FeederItemText(
    int localClientNum,
    itemDef_s *item,
    double feederID,
    int index,
    int column,
    Material **handle,
    Material **a7)
{
    *a7 = 0;
    if (feederID == 30.0)
        return (char *)LB_FeederItemText(localClientNum, column, (int)handle, a7);
    else
        return byte_82003CDD;
}

int __cdecl UI_FeederItemEnabled(int localClientNum, double feederID, int index)
{
    return 1;
}

Material *__cdecl UI_FeederItemImage(double feederID, int index)
{
    return 0;
}

void __cdecl UI_FeederItemColor(
    int localClientNum,
    itemDef_s *item,
    double feederID,
    int index,
    int column,
    float *color,
    float *a7)
{
    *a7 = item->window.foreColor[0];
    a7[1] = item->window.foreColor[1];
    a7[2] = item->window.foreColor[2];
    a7[3] = item->window.foreColor[3];
}

void __cdecl UI_FeederSelection(int localClientNum, double feederID, int index, int a4)
{
    if (feederID == 30.0)
        LB_FeederSelection(localClientNum, a4);
}

const char *__cdecl UI_GetSavegameInfo()
{
    return SEH_LocalizeTextMessage(uiInfo.savegameInfo, "Savegame Description Text", LOCMSG_SAFE);
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

int UI_OpenMenu_f()
{
    char v1[72]; // [sp+50h] [-50h] BYREF

    Cmd_ArgsBuffer(1, v1, 64);
    return Menus_OpenByName(&uiInfo.uiDC, v1);
}

void UI_CloseMenu_f()
{
    char v0[72]; // [sp+50h] [-50h] BYREF

    Cmd_ArgsBuffer(1, v0, 64);
    Menus_CloseByName(&uiInfo.uiDC, v0);
}

void __cdecl UI_OpenMenu(int localClientNum, const char *menuName)
{
    Menus_OpenByName(&uiInfo.uiDC, menuName);
}

void __cdecl UI_CloseMenu(int localClientNum, const char *menuName)
{
    Menus_CloseByName(&uiInfo.uiDC, menuName);
}

// local variable allocation has failed, the output may be wrong!
void __cdecl UI_Init()
{
    __int64 v0; // r10 OVERLAPPED
    int v1; // r8
    double v2; // fp0
    MenuList *Menus; // r3
    MenuList *v4; // r3

    memset(&uiInfo, 0, sizeof(uiInfo));
    uiInfo.uiDC.localClientNum = 0;
    g_currentMenuType = UIMENU_NONE;
    g_ingameMenusLoaded = 0;
    UI_RegisterDvars();
    uiInfo.allowScriptMenuResponse = 1;
    Cmd_AddCommandInternal("openmenu", (void(__cdecl *)())UI_OpenMenu_f, &UI_OpenMenu_f_VAR);
    Cmd_AddCommandInternal("closemenu", UI_CloseMenu_f, &UI_CloseMenu_f_VAR);
    String_Init();
    Menu_Setup(&uiInfo.uiDC);
    CL_GetScreenDimensions(&uiInfo.uiDC.screenWidth, &uiInfo.uiDC.screenHeight, &uiInfo.uiDC.screenAspect);
    LODWORD(v0) = uiInfo.uiDC.screenWidth;
    if (480 * uiInfo.uiDC.screenWidth <= 640 * uiInfo.uiDC.screenHeight)
    {
        v2 = 0.0;
    }
    else
    {
        HIDWORD(v0) = uiInfo.uiDC.screenHeight;
        v1 = 480 * uiInfo.uiDC.screenWidth;
        v2 = (float)((float)-(float)((float)((float)*(__int64 *)((char *)&v0 + 4) * (float)1.3333334) - (float)v0)
            * (float)0.5);
    }
    uiInfo.uiDC.bias = v2;
    Sys_Milliseconds();
    Menus = UI_LoadMenus("ui/code.txt", 3);
    UI_AddMenuList(&uiInfo.uiDC, Menus);
    memset(&ui_saveTimeGlob, 0, sizeof(ui_saveTimeGlob));
    Dvar_SetBool(ui_isSaving, 0);
    if (!g_mapname[0])
    {
        v4 = UI_LoadMenus("ui/menus.txt", 3);
        UI_AddMenuList(&uiInfo.uiDC, v4);
    }
    UI_AssetCache();
    Menus_CloseAll(&uiInfo.uiDC);
    Dvar_RegisterBool("ui_multiplayer", 0, 0x40u, "True if the game is multiplayer");
    uiscript_debug = Dvar_RegisterInt("uiscript_debug", 0, 0, 2, 0, "spam debug info for the ui script");
}

void __cdecl UI_KeyEvent(int localClientNum, int key, int down)
{
    menuDef_t *Focused; // r30

    if (Menu_Count(&uiInfo.uiDC))
    {
        Focused = Menu_GetFocused(&uiInfo.uiDC);
        if (!Focused)
            goto LABEL_10;
        if (key != 2 || !down || Menus_AnyFullScreenVisible(&uiInfo.uiDC) || Focused->onESC)
            Menu_HandleKey(&uiInfo.uiDC, Focused, key, down);
        else
            Menus_CloseAll(&uiInfo.uiDC);
        if (!Menu_GetFocused(&uiInfo.uiDC))
        {
        LABEL_10:
            Key_RemoveCatcher(localClientNum, -17);
            Key_ClearStates(localClientNum);
            if (!CL_SkipRendering())
                Dvar_SetIntByName("cl_paused", 0);
        }
    }
}

uiMenuCommand_t __cdecl UI_GetActiveMenu(int localClientNum)
{
    return g_currentMenuType;
}

const char *__cdecl UI_GetTopActiveMenuName(int localClientNum)
{
    int v1; // r10
    int v2; // r31

    v1 = uiInfo.uiDC.openMenuCount - 1;
    if (uiInfo.uiDC.openMenuCount - 1 < 0 || v1 >= uiInfo.uiDC.menuCount)
        return 0;
    v2 = v1;
    if (!uiInfo.uiDC.menuStack[v1])
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp",
            2356,
            0,
            "%s",
            "uiInfo.uiDC.menuStack[topMenuStackIndex]");
    return uiInfo.uiDC.menuStack[v2]->window.name;
}

void __cdecl UI_ShowAcceptInviteWarning()
{
    Dvar_SetIntByName("cl_paused", 1);
    Key_SetCatcher(0, 16);
    Menus_OpenByName(&uiInfo.uiDC, "sp_acceptinvite_warning");
}

void __cdecl UI_ShowReadingSaveDevicePopup()
{
    Key_SetCatcher(0, 16);
    Menus_OpenByName(&uiInfo.uiDC, "readingsavedevice");
}

void __cdecl UI_HideReadingSaveDevicePopup()
{
    Menus_CloseByName(&uiInfo.uiDC, "readingsavedevice");
}

int __cdecl UI_IsFullscreen()
{
    return Menus_AnyFullScreenVisible(&uiInfo.uiDC);
}

float __cdecl UI_GetBlurRadius()
{
    double blurRadiusOut; // fp1

    blurRadiusOut = uiInfo.uiDC.blurRadiusOut;
    return *((float *)&blurRadiusOut + 1);
}

char *__cdecl UI_SafeTranslateString(const char *reference)
{
    const char *v1; // r30
    char *result; // r3
    char *v3; // r10
    const char *v4; // r11
    int v5; // ctr
    const char *v6; // r11
    int v7; // r10

    v1 = reference;
    if (*reference == 21)
    {
        v1 = reference + 1;
    }
    else
    {
        result = (char *)SEH_StringEd_GetString(reference);
        if (result)
            return result;
    }
    if (loc_warnings->current.enabled)
    {
        if (loc_warningsAsErrors->current.enabled)
            Com_Error(ERR_LOCALIZATION, "Could not translate string \"%s\"", v1);
        else
            Com_PrintWarning(13, "WARNING: Could not translate string \"%s\"\n", v1);
        v3 = errorString;
        v4 = "^1UNLOCALIZED(^7";
        v5 = 17;
        do
        {
            *v3++ = *v4++;
            --v5;
        } while (v5);
        I_strncat(errorString, 1024, v1);
        I_strncat(errorString, 1024, "^1)^7");
    }
    else
    {
        v6 = v1;
        do
        {
            v7 = *(unsigned __int8 *)v6;
            (v6++)[errorString - v1] = v7;
        } while (v7);
    }
    return errorString;
}

int __cdecl UI_AnyFullScreenMenuVisible(int localClientNum)
{
    return Menus_AnyFullScreenVisible(&uiInfo.uiDC);
}

bool __cdecl UI_AnyMenuVisible(int localClientNum)
{
    return uiInfo.uiDC.openMenuCount != 0;
}

void __cdecl UI_FilterStringForButtonAnimation(char *str, unsigned int strMaxSize)
{
    unsigned int v4; // r11
    int v5; // r10

    if (Sys_Milliseconds() % 1000 > 800)
    {
        v4 = 0;
        if (*str)
        {
            do
            {
                if (v4 >= strMaxSize)
                    break;
                v5 = str[v4];
                if (v5 == 16)
                {
                    str[v4] = -68;
                }
                else if (v5 == 17)
                {
                    str[v4] = -67;
                }
                ++v4;
            } while (str[v4]);
        }
    }
}

void __cdecl UI_ReplaceConversions(
    const char *sourceString,
    ConversionArguments *arguments,
    char *outputString,
    size_t outputStringSize)
{
    const char *v8; // r11
    int v10; // r11
    int v11; // r20
    int v12; // r29
    int v13; // r28
    char *v14; // r30
    int v15; // r31
    int v16; // r31
    unsigned __int8 *v17; // r11
    unsigned __int8 *v19; // r10
    int v20; // r11
    int v21; // r10
    char v22; // r9

    if (!sourceString)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp", 2701, 0, "%s", "sourceString");
    if (strstr(sourceString, "&&"))
    {
        if (!arguments)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp", 2709, 0, "%s", "arguments");
        if (arguments->argCount > 9)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp",
                2710,
                0,
                "%s\n\t(arguments->argCount) = %i",
                "(arguments->argCount <= 9)",
                arguments->argCount);
        v8 = sourceString;
        while (*(unsigned __int8 *)v8++)
            ;
        v10 = v8 - sourceString - 1;
        v11 = v10;
        if (v10 <= 0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp",
                2714,
                0,
                "%s\n\t(sourceStringLength) = %i",
                "(sourceStringLength > 0)",
                v10);
        memset(outputString, 0, outputStringSize);
        v12 = 0;
        v13 = 0;
        while (v13 < v11)
        {
            v14 = (char *)&sourceString[v13];
            if (strncmp(&sourceString[v13], "&&", 2u) || !isdigit(sourceString[v13 + 2]))
            {
                ++v13;
                outputString[v12++] = *v14;
            }
            else
            {
                v15 = sourceString[v13 + 2] - 49;
                if (v15 < 0 || v15 >= arguments->argCount)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp",
                        2728,
                        0,
                        "%s\n\t(argIndex) = %i",
                        "(argIndex >= 0 && argIndex < arguments->argCount)",
                        v15);
                if (v15 >= 9)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp",
                        2729,
                        0,
                        "%s\n\t(argIndex) = %i",
                        "(argIndex < 9)",
                        v15);
                v16 = 4 * (v15 + 1);
                if (!*(int *)((char *)&arguments->argCount + v16))
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp", 2731, 0, "%s", "arguments->args[argIndex]");
                v17 = *(unsigned __int8 **)((char *)&arguments->argCount + v16);
                while (*v17++)
                    ;
                v19 = &v17[-*(int *)((char *)&arguments->argCount + v16)];
                v20 = 0;
                v21 = (int)(v19 - 1);
                if (v21 > 0)
                {
                    do
                    {
                        v22 = *(_BYTE *)(*(int *)((char *)&arguments->argCount + v16) + v20++);
                        outputString[v12++] = v22;
                    } while (v20 < v21);
                }
                v13 += 3;
            }
        }
        UI_FilterStringForButtonAnimation(outputString, outputStringSize);
    }
    else
    {
        I_strncpyz(outputString, sourceString, outputStringSize);
    }
}

void __cdecl UI_CloseFocusedMenu()
{
    if (Menu_Count(&uiInfo.uiDC) > 0 && Menu_GetFocused(&uiInfo.uiDC) && !Menus_AnyFullScreenVisible(&uiInfo.uiDC))
        Menus_CloseAll(&uiInfo.uiDC);
}

bool __cdecl Menu_IsMenuOpenAndVisible(const int localClientNum, const char *menuName)
{
    menuDef_t *v2; // r3
    menuDef_t *v3; // r30

    v2 = Menus_FindByName(&uiInfo.uiDC, menuName);
    v3 = v2;
    return v2 && Menus_MenuIsInStack(&uiInfo.uiDC, v2) && Menu_IsVisible(&uiInfo.uiDC, v3);
}

int __cdecl UI_PopupScriptMenu(const char *menuName, bool useMouse)
{
    menuDef_t *Focused; // r3
    double v6; // fp0

    Focused = Menu_GetFocused(&uiInfo.uiDC);
    if (!Focused)
        goto LABEL_5;
    if (g_currentMenuType != UIMENU_SCRIPT_POPUP)
        return 0;
    if (I_stricmp(Focused->window.name, menuName))
    {
    LABEL_5:
        g_currentMenuType = UIMENU_SCRIPT_POPUP;
        if (useMouse)
        {
            uiInfo.uiDC.cursor.x = 320.0;
            v6 = 240.0;
        }
        else
        {
            uiInfo.uiDC.cursor.x = 639.0;
            v6 = 479.0;
        }
        uiInfo.uiDC.cursor.y = v6;
        Key_SetCatcher(0, 16);
        Menus_CloseAll(&uiInfo.uiDC);
        Menus_OpenByName(&uiInfo.uiDC, menuName);
    }
    return 1;
}

void __cdecl UI_ClosePopupScriptMenu(int localClientNum, bool allowResponse)
{
    if (g_currentMenuType == UIMENU_SCRIPT_POPUP)
    {
        uiInfo.allowScriptMenuResponse = allowResponse;
        UI_CloseFocusedMenu();
        uiInfo.allowScriptMenuResponse = 1;
    }
}

bool __cdecl UI_AllowScriptMenuResponse(int localClientNum)
{
    return uiInfo.allowScriptMenuResponse;
}

void UI_PlayerStart()
{
    if (g_currentMenuType != UIMENU_PREGAME && g_currentMenuType != UIMENU_MAIN)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp",
            516,
            0,
            "%s\n\t(UI_GetActiveMenu( 0 )) = %i",
            "(UI_GetActiveMenu( 0 ) == UIMENU_PREGAME || UI_GetActiveMenu( 0 ) == UIMENU_MAIN)",
            g_currentMenuType);
    CL_SetSkipRendering(0);
    UI_SetActiveMenu(0, 0);
    if (R_Cinematic_IsNextReady())
        R_Cinematic_StartNextPlayback();
    else
        R_Cinematic_StopPlayback();
}

void __cdecl UI_Refresh()
{
    UI_UpdateSaveUI();
    if (Menu_Count(&uiInfo.uiDC) > 0)
    {
        Menu_PaintAll(&uiInfo.uiDC);
        if (g_currentMenuType == UIMENU_PREGAME)
        {
            if (Menu_IsMenuOpenAndVisible(0, "pregame"))
            {
                if (R_Cinematic_IsFinished())
                    UI_PlayerStart();
            }
        }
    }
}

int __cdecl UI_OwnerDrawWidth(int ownerDraw, Font_s *font, double scale)
{
    const char *v5; // r3
    char *v6; // r3

    if (ownerDraw == 250
        && (!Display_KeyBindPending() ? (v5 = "EXE_KEYCHANGE") : (v5 = "EXE_KEYWAIT"), (v6 = UI_SafeTranslateString(v5)) != 0))
    {
        return UI_TextWidth(v6, 0, font, scale);
    }
    else
    {
        return 0;
    }
}

void __cdecl UI_DrawKeyBindStatus(rectDef_s *rect, Font_s *font, double scale, float *color, int textStyle)
{
    const char *v8; // r3
    int vertAlign; // r27
    const float *horzAlign; // r26
    double y; // fp30
    double x; // fp29
    char *v13; // r3
    int v14; // r8
    int v15; // r7
    double v16; // fp8
    double v17; // fp7
    double v18; // fp6
    double v19; // fp5
    double v20; // fp4
    float v21; // [sp+8h] [-B8h]
    float v22; // [sp+10h] [-B0h]
    float v23; // [sp+18h] [-A8h]
    float v24; // [sp+20h] [-A0h]
    float v25; // [sp+28h] [-98h]
    float v26; // [sp+30h] [-90h]
    float v27; // [sp+38h] [-88h]
    float v28; // [sp+40h] [-80h]
    float v29; // [sp+48h] [-78h]
    float v30; // [sp+50h] [-70h]
    float v31; // [sp+58h] [-68h]
    float v32; // [sp+60h] [-60h]
    float v33; // [sp+68h] [-58h]
    float v34; // [sp+70h] [-50h]

    if (Display_KeyBindPending())
        v8 = "EXE_KEYWAIT";
    else
        v8 = "EXE_KEYCHANGE";
    vertAlign = rect->vertAlign;
    horzAlign = (const float *)rect->horzAlign;
    y = rect->y;
    x = rect->x;
    v13 = UI_SafeTranslateString(v8);
    UI_DrawText(
        &scrPlaceFull,
        v13,
        0x7FFFFFFF,
        font,
        x,
        y,
        v15,
        v14,
        scale,
        horzAlign,
        vertAlign,
        v20,
        v19,
        v18,
        v17,
        v16,
        v21,
        v22,
        v23,
        v24,
        v25,
        v26,
        v27,
        v28,
        v29,
        v30,
        v31,
        v32,
        v33,
        v34);
}

void __cdecl UI_DrawLoggedInUserName(rectDef_s *rect, Font_s *font, double scale, float *color, int textStyle)
{
    int v8; // r8
    int v9; // r7
    const char *LocalClientName; // r4
    double v11; // fp8
    double v12; // fp7
    double v13; // fp6
    double v14; // fp5
    double v15; // fp4
    int vertAlign; // r27
    double y; // fp30
    const float *horzAlign; // r26
    double x; // fp29
    char *v20; // r3
    int v21; // r8
    int v22; // r7
    double v23; // fp8
    double v24; // fp7
    double v25; // fp6
    double v26; // fp5
    double v27; // fp4
    float v28; // [sp+8h] [-B8h]
    float v29; // [sp+10h] [-B0h]
    float v30; // [sp+18h] [-A8h]
    float v31; // [sp+20h] [-A0h]
    float v32; // [sp+28h] [-98h]
    float v33; // [sp+30h] [-90h]
    float v34; // [sp+38h] [-88h]
    float v35; // [sp+40h] [-80h]
    float v36; // [sp+48h] [-78h]
    float v37; // [sp+50h] [-70h]
    float v38; // [sp+58h] [-68h]
    float v39; // [sp+60h] [-60h]
    float v40; // [sp+68h] [-58h]
    float v41; // [sp+70h] [-50h]

    if (!cl_multi_gamepads_enabled)
    {
        LocalClientName = Live_GetLocalClientName(cl_controller_in_use);
        if (*LocalClientName)
        {
            UI_DrawText(
                &scrPlaceFull,
                LocalClientName,
                0x7FFFFFFF,
                font,
                rect->x,
                rect->y,
                v9,
                v8,
                scale,
                (const float *)rect->horzAlign,
                rect->vertAlign,
                v15,
                v14,
                v13,
                v12,
                v11,
                v28,
                v29,
                v30,
                v31,
                v32,
                v33,
                v34,
                v35,
                v36,
                v37,
                v38,
                v39,
                v40,
                v41);
        }
        else
        {
            vertAlign = rect->vertAlign;
            y = rect->y;
            horzAlign = (const float *)rect->horzAlign;
            x = rect->x;
            v20 = UI_SafeTranslateString("XBOXLIVE_NOTSIGNEDIN");
            UI_DrawText(
                &scrPlaceFull,
                v20,
                0x7FFFFFFF,
                font,
                x,
                y,
                v22,
                v21,
                scale,
                horzAlign,
                vertAlign,
                v27,
                v26,
                v25,
                v24,
                v23,
                v28,
                v29,
                v30,
                v31,
                v32,
                v33,
                v34,
                v35,
                v36,
                v37,
                v38,
                v39,
                v40,
                v41);
        }
    }
}

void __cdecl UI_SavegameSort(int column, int force)
{
    int v2; // r6

    if (force || uiInfo.savegameStatus.sortKey != column)
    {
        uiInfo.savegameStatus.sortKey = column;
        if (uiInfo.savegameCount)
        {
            qsort(
                uiInfo.savegameStatus.displaySavegames,
                uiInfo.savegameCount,
                4u,
                (int(__cdecl *)(const void *, const void *))UI_SavegamesQsortCompare);
            if (uiInfo.savegameName[0])
            {
                v2 = UI_SavegameIndexFromFilename(uiInfo.savegameName);
                if (v2 < 0)
                    return;
            }
            else
            {
                v2 = 0;
            }
            Menu_SetFeederSelection(&uiInfo.uiDC, 0, 16, v2, 0);
        }
        else
        {
            Dvar_SetString(ui_savegame, byte_82003CDD);
            uiInfo.savegameName[0] = 0;
            strcpy(uiInfo.savegameInfo, "EXE_NOSAVEGAMES");
        }
    }
}

void __cdecl UI_RunMenuScript(int localClientNum, const char **args, const char *actualScript)
{
    int v6; // r31
    char *v7; // r11
    int v8; // ctr
    const char *VariantString; // r3
    int v10; // r3
    int Int; // r3
    char v12[16]; // [sp+50h] [-14B0h] BYREF
    char v13[1024]; // [sp+60h] [-14A0h] BYREF
    char v14[128]; // [sp+460h] [-10A0h] BYREF
    char v15[32]; // [sp+4E0h] [-1020h] BYREF
    char v16[1024]; // [sp+8E0h] [-C20h] BYREF
    char v17[1024]; // [sp+CE0h] [-820h] BYREF
    char v18[1056]; // [sp+10E0h] [-420h] BYREF

    if (!String_Parse(args, v13, 1024))
        return;
    if (!I_stricmp(v13, "clearError"))
    {
        Dvar_SetStringByName("com_errorMessage", byte_82003CDD);
        Dvar_SetBoolByName("com_isNotice", 0);
        return;
    }
    if (!I_stricmp(v13, "loadMissionDifficultyOffset"))
    {
        if (String_Parse(args, v15, 1024))
        {
            v6 = atol(v15);
            v7 = v14;
            v8 = 16;
            do
            {
                *(_QWORD *)v7 = 0x1000000000LL;
                v7 += 8;
                --v8;
            } while (v8);
            VariantString = Dvar_GetVariantString("mis_difficulty");
            I_strncpyz(v14, VariantString, 128);
            if (v6 >= 128)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp",
                    1700,
                    0,
                    "%s",
                    "offset < static_cast< int >( sizeof( missionDifficulty ) )");
            I_strncpyz(v12, &v14[v6], 2);
            Dvar_SetStringByName("ui_level", v12);
        }
        return;
    }
    if (!I_stricmp(v13, "playerstart"))
    {
        UI_PlayerStart();
        return;
    }
    if (!I_stricmp(v13, "Quit"))
    {
        v10 = CL_ControllerIndexFromClientNum(0);
        Cmd_ExecuteSingleCommand(0, v10, "quit");
        return;
    }
    if (!I_stricmp(v13, "Controls"))
    {
        Dvar_SetIntByName("cl_paused", 1);
        Key_SetCatcher(0, 16);
        Menus_CloseAll(&uiInfo.uiDC);
        Menus_OpenByName(&uiInfo.uiDC, "setup_menu2");
        return;
    }
    if (!I_stricmp(v13, "Leave"))
    {
        Cbuf_AddText(0, "disconnect\n");
        Key_SetCatcher(0, 16);
        Menus_CloseAll(&uiInfo.uiDC);
        Menus_OpenByName(&uiInfo.uiDC, "main");
        return;
    }
    if (!I_stricmp(v13, "closeingame"))
    {
        Key_RemoveCatcher(localClientNum, -17);
        Key_ClearStates(localClientNum);
        Dvar_SetIntByName("cl_paused", 0);
        Menus_CloseAll(&uiInfo.uiDC);
        return;
    }
    if (!I_stricmp(v13, "update"))
    {
        if (String_Parse(args, v15, 1024))
            UI_Update(v15);
        return;
    }
    if (!I_stricmp(v13, "startSingleplayer") || !I_stricmp(v13, "startMultiplayer"))
    {
        Cbuf_AddText(0, "startMultiplayer\n");
        return;
    }
    if (!I_stricmp(v13, "getLanguage"))
    {
        Int = Dvar_GetInt("loc_language");
        Dvar_SetIntByName("ui_language", Int);
    LABEL_29:
        UI_VerifyLanguage();
        return;
    }
    if (!I_stricmp(v13, "verifyLanguage"))
        goto LABEL_29;
    if (I_stricmp(v13, "saveComplete"))
    {
        if (I_stricmp(v13, "saveRevert"))
        {
            if (I_stricmp(v13, "openMenuOnDvar") && I_stricmp(v13, "openMenuOnDvarNot"))
            {
                if (I_stricmp(v13, "closeMenuOnDvar") && I_stricmp(v13, "closeMenuOnDvarNot"))
                {
                    if (I_stricmp(v13, "RefreshLeaderboards"))
                    {
                        if (I_stricmp(v13, "ViewGamerCard"))
                            Com_Printf(13, "unknown UI script %s in block:\n%s\n", v13, actualScript);
                        else
                            LB_OnSelect(localClientNum);
                    }
                    else
                    {
                        LB_ForceRefresh();
                    }
                }
                else if ((unsigned __int8)UI_GetOpenOrCloseMenuOnDvarArgs(args, v13, v16, v17, v18))
                {
                    UI_CloseMenuOnDvar(v13, v18, v16, v17);
                }
            }
            else if ((unsigned __int8)UI_GetOpenOrCloseMenuOnDvarArgs(args, v13, v16, v17, v18))
            {
                UI_OpenMenuOnDvar(v13, v18, v16, v17);
            }
        }
        else
        {
            UI_SaveRevert();
        }
    }
    else
    {
        UI_SaveComplete();
    }
}

int __cdecl UI_SetActiveMenu(int localClientNum, uiMenuCommand_t menu)
{
    int result; // r3
    uiMenuCommand_t v4; // r11
    const char *String; // r3

    if (Menu_Count(&uiInfo.uiDC) <= 0)
        return 0;
    if (menu == UIMENU_BRIEFING)
    {
        if (g_currentMenuType == UIMENU_BRIEFING)
            return 0;
    }
    else if (menu == UIMENU_SCRIPT_POPUP)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp", 2416, 0, "%s", "menu != UIMENU_SCRIPT_POPUP");
    }
    v4 = g_currentMenuType;
    g_currentMenuType = menu;
    switch (menu)
    {
    case UIMENU_NONE:
        Key_RemoveCatcher(0, -17);
        Key_ClearStates(0);
        Dvar_SetIntByName("cl_paused", 0);
        Menus_CloseAll(&uiInfo.uiDC);
        if (CL_SkipRendering())
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp", 2429, 0, "%s", "!CL_SkipRendering()");
        goto LABEL_10;
    case UIMENU_MAIN:
        Key_SetCatcher(0, 16);
        Menus_OpenByName(&uiInfo.uiDC, "main");
        String = Dvar_GetString("com_errorMessage");
        if (*String)
        {
            Menus_OpenByName(&uiInfo.uiDC, "error_popmenu");
            CL_StopControllerRumbles();
        }
        SND_FadeAllSounds(1.0, (int)String);
        return 1;
    case UIMENU_INGAME:
        if (v4 == UIMENU_CONTROLLERREMOVED)
        {
        LABEL_10:
            result = 1;
        }
        else
        {
            Dvar_SetIntByName("cl_paused", 1);
            Key_SetCatcher(0, 16);
            Menus_OpenByName(&uiInfo.uiDC, "pausedmenu");
            result = 1;
        }
        break;
    case UIMENU_PREGAME:
        if (UI_AutoContinue())
        {
            UI_PlayerStart();
            result = 1;
        }
        else
        {
            Dvar_SetIntByName("cl_paused", 1);
            Key_SetCatcher(0, 16);
            Menus_CloseAll(&uiInfo.uiDC);
            if (*Dvar_GetString("com_errorMessage"))
                Menus_OpenByName(&uiInfo.uiDC, "pregame_loaderror");
            else
                Menus_OpenByName(&uiInfo.uiDC, "pregame");
            result = 1;
        }
        break;
    case UIMENU_POSTGAME:
        Key_SetCatcher(0, 16);
        Menus_CloseAll(&uiInfo.uiDC);
        Menus_OpenByName(&uiInfo.uiDC, "endofgame");
        result = 1;
        break;
    case UIMENU_BRIEFING:
        Menus_CloseAll(&uiInfo.uiDC);
        Menus_OpenByName(&uiInfo.uiDC, "briefing");
        result = 1;
        break;
    case UIMENU_VICTORYSCREEN:
        uiInfo.uiDC.cursor.x = 320.0;
        uiInfo.uiDC.cursor.y = 448.0;
        Key_SetCatcher(0, 16);
        Menus_OpenByName(&uiInfo.uiDC, "victoryscreen");
        result = 1;
        break;
    case UIMENU_SAVEERROR:
        uiInfo.uiDC.cursor.x = 320.0;
        uiInfo.uiDC.cursor.y = 448.0;
        Dvar_SetIntByName("cl_paused", 1);
        Key_SetCatcher(0, 16);
        Menus_OpenByName(&uiInfo.uiDC, "savegame_error");
        result = 1;
        break;
    case UIMENU_SAVE_LOADING:
        if (SaveMemory_IsRecentlyLoaded())
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\ui\\ui_main.cpp",
                2480,
                0,
                "%s",
                "!SaveMemory_IsRecentlyLoaded()");
        Menus_OpenByName(&uiInfo.uiDC, "savegameloading");
        result = 1;
        break;
    case UIMENU_CONTROLLERREMOVED:
        Dvar_SetIntByName("cl_paused", 1);
        Key_SetCatcher(0, 16);
        if (!Menu_GetFocused(&uiInfo.uiDC) || g_currentMenuType == UIMENU_NONE)
            Menus_OpenByName(&uiInfo.uiDC, "pausedmenu");
        result = 1;
        break;
    default:
        return 0;
    }
    return result;
}

void __cdecl UI_DrawConnectScreen()
{
    if (Menu_Count(&uiInfo.uiDC) > 0 && g_currentMenuType != UIMENU_BRIEFING)
    {
        g_currentMenuType = UIMENU_BRIEFING;
        Menus_CloseAll(&uiInfo.uiDC);
        Menus_OpenByName(&uiInfo.uiDC, "briefing");
    }
    SCR_UpdateLoadScreen();
}

char *__cdecl UI_ReplaceConversionString(const char *sourceString, const char *replaceString)
{
    __int64 v2; // r10
    ConversionArguments v4; // [sp+50h] [-440h] BYREF
    char v5[1032]; // [sp+80h] [-410h] BYREF

    LODWORD(v2) = 0;
    HIDWORD(v2) = replaceString;
    *(_QWORD *)&v4.args[1] = v2;
    *(_QWORD *)&v4.args[3] = v2;
    *(_QWORD *)&v4.args[5] = v2;
    *(_QWORD *)&v4.args[7] = v2;
    v4.args[0] = replaceString;
    v4.argCount = 1;
    UI_ReplaceConversions(sourceString, &v4, v5, 0x400u);
    return va(v5);
}

char *__cdecl UI_ReplaceConversionInt(const char *sourceString, int replaceInt)
{
    __int64 v2; // r10
    ConversionArguments v5; // [sp+50h] [-460h] BYREF
    char v6[32]; // [sp+80h] [-430h] BYREF
    char v7[1024]; // [sp+A0h] [-410h] BYREF

    LODWORD(v2) = 0;
    HIDWORD(v2) = 0x82000000;
    *(_QWORD *)&v5.args[1] = v2;
    *(_QWORD *)&v5.args[3] = v2;
    *(_QWORD *)&v5.args[5] = v2;
    *(_QWORD *)&v5.args[7] = v2;
    sprintf_0(v6, "%d", replaceInt);
    v5.argCount = 1;
    v5.args[0] = v6;
    UI_ReplaceConversions(sourceString, &v5, v7, 0x400u);
    return va(v7);
}

char *__cdecl UI_ReplaceConversionInts(
    const char *sourceString,
    int numInts,
    char *replaceInts,
    int a4,
    int a5,
    int a6,
    __int64 a7)
{
    const char **args; // r30
    int v9; // r28
    char *v10; // r31
    int v11; // r29
    ConversionArguments v13; // [sp+50h] [-670h] BYREF
    char v14; // [sp+80h] [-640h] BYREF
    char v15[1088]; // [sp+280h] [-440h] BYREF

    LODWORD(a7) = 0;
    v13.args[0] = 0;
    *(_QWORD *)&v13.args[1] = a7;
    *(_QWORD *)&v13.args[3] = a7;
    *(_QWORD *)&v13.args[5] = a7;
    *(_QWORD *)&v13.args[7] = a7;
    v13.argCount = numInts;
    if (numInts > 0)
    {
        args = v13.args;
        v9 = replaceInts - (char *)v13.args;
        v10 = &v14;
        v11 = numInts;
        do
        {
            sprintf_0(v10, "%d", *(const char **)((char *)args + v9));
            --v11;
            *args = v10;
            v10 += 32;
            ++args;
        } while (v11);
    }
    UI_ReplaceConversions(sourceString, &v13, v15, 0x400u);
    return va(v15);
}

int __cdecl UI_Popup(int localClientNum, const char *menu)
{
    const char *v3; // r30

    v3 = "briefing";
    if (I_stricmp(menu, "briefing"))
    {
        if (!CL_IsLocalClientInGame(localClientNum))
            return 1;
        if (CL_TimeDemoPlaying())
            return 1;
        v3 = "victoryscreen";
        if (I_stricmp(menu, "victoryscreen") || Menu_Count(&uiInfo.uiDC) <= 0)
            return 1;
        uiInfo.uiDC.cursor.x = 320.0;
        g_currentMenuType = UIMENU_VICTORYSCREEN;
        uiInfo.uiDC.cursor.y = 448.0;
        Key_SetCatcher(0, 16);
    }
    else
    {
        if (Menu_Count(&uiInfo.uiDC) <= 0 || g_currentMenuType == UIMENU_BRIEFING)
            return 0;
        g_currentMenuType = UIMENU_BRIEFING;
        Menus_CloseAll(&uiInfo.uiDC);
    }
    Menus_OpenByName(&uiInfo.uiDC, v3);
    return 1;
}

void __cdecl UI_DrawLoggedInUser(rectDef_s *rect, Font_s *font, double scale, float *color, int textStyle)
{
    const char *LocalClientName; // r27
    int vertAlign; // r27
    double y; // fp30
    const float *horzAlign; // r26
    double x; // fp29
    char *v13; // r3
    int v14; // r8
    int v15; // r7
    double v16; // fp8
    double v17; // fp7
    double v18; // fp6
    double v19; // fp5
    double v20; // fp4
    char *v21; // r3
    char *v22; // r3
    int v23; // r8
    int v24; // r7
    double v25; // fp8
    double v26; // fp7
    double v27; // fp6
    double v28; // fp5
    double v29; // fp4
    float v30; // [sp+8h] [-B8h]
    float v31; // [sp+10h] [-B0h]
    float v32; // [sp+18h] [-A8h]
    float v33; // [sp+20h] [-A0h]
    float v34; // [sp+28h] [-98h]
    float v35; // [sp+30h] [-90h]
    float v36; // [sp+38h] [-88h]
    float v37; // [sp+40h] [-80h]
    float v38; // [sp+48h] [-78h]
    float v39; // [sp+50h] [-70h]
    float v40; // [sp+58h] [-68h]
    float v41; // [sp+60h] [-60h]
    float v42; // [sp+68h] [-58h]
    float v43; // [sp+70h] [-50h]

    if (!cl_multi_gamepads_enabled)
    {
        LocalClientName = Live_GetLocalClientName(cl_controller_in_use);
        if (*LocalClientName)
        {
            v21 = UI_SafeTranslateString("XBOXLIVE_SIGNEDINAS");
            v22 = UI_ReplaceConversionString(v21, LocalClientName);
            UI_DrawText(
                &scrPlaceFull,
                v22,
                0x7FFFFFFF,
                font,
                rect->x,
                rect->y,
                v24,
                v23,
                scale,
                (const float *)rect->horzAlign,
                rect->vertAlign,
                v29,
                v28,
                v27,
                v26,
                v25,
                v30,
                v31,
                v32,
                v33,
                v34,
                v35,
                v36,
                v37,
                v38,
                v39,
                v40,
                v41,
                v42,
                v43);
        }
        else
        {
            vertAlign = rect->vertAlign;
            y = rect->y;
            horzAlign = (const float *)rect->horzAlign;
            x = rect->x;
            v13 = UI_SafeTranslateString("XBOXLIVE_NOTSIGNEDIN");
            UI_DrawText(
                &scrPlaceFull,
                v13,
                0x7FFFFFFF,
                font,
                x,
                y,
                v15,
                v14,
                scale,
                horzAlign,
                vertAlign,
                v20,
                v19,
                v18,
                v17,
                v16,
                v30,
                v31,
                v32,
                v33,
                v34,
                v35,
                v36,
                v37,
                v38,
                v39,
                v40,
                v41,
                v42,
                v43);
        }
    }
}

void __cdecl UI_OwnerDraw(
    int localClientNum,
    double x,
    double y,
    double w,
    double h,
    int horzAlign,
    int vertAlign,
    double text_x,
    double text_y,
    int ownerDraw,
    int ownerDrawFlags,
    float *align,
    double special,
    Font_s *font,
    double scale,
    float *color,
    Material *material,
    int textStyle,
    rectDef_s *parentRect,
    int textAlignMode,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28,
    int a29,
    int a30,
    int a31,
    int a32,
    int a33,
    int a34,
    int a35,
    int a36,
    int a37,
    int a38,
    int a39,
    int a40,
    int a41,
    int a42,
    int a43,
    int a44,
    int a45,
    Font_s *ownerDraw_0,
    int a47,
    int ownerDrawFlags_0,
    int a49,
    int align_0,
    int a51,
    int a52,
    int a53,
    Font_s *font_0,
    int a55,
    int a56,
    rectDef_s *a57,
    float *color_0,
    int a59,
    Material *materiala)
{
    Font_s *v70; // r9
    int v71; // r8
    int v72; // r7
    int v73; // r6
    float *v74; // r5
    float *v75; // r4
    Material *v76; // [sp+8h] [-148h]
    int v77; // [sp+Ch] [-144h]
    int v78; // [sp+10h] [-140h]
    rectDef_s v79[3]; // [sp+C0h] [-90h] BYREF

    if (CL_IsCgameInitialized(localClientNum))
        CG_OwnerDraw(
            a57,
            x,
            y,
            w,
            h,
            a59,
            (int)materiala,
            text_x,
            text_y,
            v73,
            v72,
            v71,
            special,
            v70,
            scale,
            align,
            v76,
            v77,
            v78);
    v79[0].x = (float)x + (float)text_x;
    v79[0].y = (float)y + (float)text_y;
    v79[0].horzAlign = (int)align;
    v79[0].w = w;
    v79[0].vertAlign = (int)font;
    v79[0].h = h;
    switch (a38)
    {
    case 250:
        UI_DrawKeyBindStatus(v79, ownerDraw_0, scale, v74, align_0);
        break;
    case 258:
        UI_DrawSaveGameShot(v79, scale, v75);
        break;
    case 264:
        ProfLoad_DrawOverlay(v79);
        break;
    case 272:
        UI_DrawLoggedInUser(v79, ownerDraw_0, scale, v74, align_0);
        break;
    case 276:
        UI_DrawLoggedInUserName(v79, ownerDraw_0, scale, v74, align_0);
        break;
    case 277:
        R_Cinematic_DrawStretchPic_Letterboxed(0);
        break;
    default:
        return;
    }
}

