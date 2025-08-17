#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include <xanim/xanim.h>
#include <physics/ode/objects.h>
#include <bgame/bg_local.h>

#include <ui/ui_shared.h>

#undef DrawText // Horrible OS

// mission
void Campaign_UnlockAll();
void Campaign_RegisterDvars();

extern const dvar_t *mis_cheat;
extern const dvar_t *mis_01;
extern const dvar_t *mis_difficulty;

// ui_main
struct __declspec(align(4)) SaveTimeGlob
{
    bool isSaving;
    bool callWrite;
    int saveTime;
    bool hasfirstFrameShown;
    const char *saveMenuName;
    char execOnSuccess[256];
    bool hasExecOnSuccess;
};
struct SavegameInfo
{
    const char *savegameFile;
    const char *savegameName;
    const char *imageName;
    const char *mapName;
    const char *savegameInfoText;
    const char *time;
    const char *date;
    qtime_s tm;
};
struct savegameStatus_s
{
    int sortKey;
    int sortDir;
    int displaySavegames[256];
};
struct uiInfo_s
{
    UiContext uiDC;
    SavegameInfo savegameList[512];
    int savegameCount;
    savegameStatus_s savegameStatus;

    int timeIndex;
    int previousTimes[4];
    bool allowScriptMenuResponse;
    char savegameName[64];
    char savegameInfo[256];
    Material *sshotImage;
    char sshotImageName[64];
};
struct sharedUiInfo_t // sizeof=0x1C5B0
{                                       // ...
    CachedAssets_t assets;              // ...
};

// LWSS: one of the main differences is that SP doesn't have `localClientNum` because there is no splitscreen
void UI_AssetCache();

Font_s *__cdecl UI_GetFontHandle(const ScreenPlacement *scrPlace, int fontEnum, float scale);
void UI_UpdateSaveUI();
void __cdecl UI_UpdateTime(int realtime);
void __cdecl UI_Shutdown();
MenuList *__cdecl Load_ScriptMenuInternal(const char *pszMenu, int imageTrack);
MenuList *__cdecl Load_ScriptMenu(const char *pszMenu, int imageTrack);
int __cdecl UI_SavegameIndexFromFilename(const char *filename);
int __cdecl UI_SavegameIndexFromFilename2(const char *filename);
void __cdecl UI_DrawSaveGameShot(rectDef_s *rect, double scale, float *color);
void UI_DrawCinematic();
void __cdecl UI_LoadIngameMenus();
void __cdecl UI_SetMap(const char *mapname);
int __cdecl UI_OwnerDrawVisible(int flags);
int __cdecl UI_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key);
int __cdecl UI_CompareTimes(qtime_s *tm1, qtime_s *tm2);
int __cdecl UI_SavegamesQsortCompare(unsigned int *arg1, unsigned int *arg2);
void __cdecl UI_Update(const char *name);
void UI_SaveComplete();
void *UI_SaveRevert();
void UI_VerifyLanguage();
int __cdecl UI_GetOpenOrCloseMenuOnDvarArgs(
    const char **args,
    const char *cmd,
    char *dvarName,
    char *testValue,
    char *menuName);
bool __cdecl UI_DvarValueTest(const char *cmd, const char *dvarName, const char *testValue, bool wantMatch);
void __cdecl UI_OpenMenuOnDvar(const char *cmd, const char *menuName, const char *dvarName, const char *testValue);
void __cdecl UI_CloseMenuOnDvar(const char *cmd, const char *menuName, const char *dvarName, const char *testValue);
bool __cdecl UI_AutoContinue();
int __cdecl UI_FeederItemEnabled(int localClientNum, double feederID, int index);
Material *__cdecl UI_FeederItemImage(double feederID, int index);
const char *__cdecl UI_GetSavegameInfo();
int UI_OpenMenu_f();
void UI_CloseMenu_f();
void __cdecl UI_OpenMenu(int localClientNum, const char *menuName);
void __cdecl UI_CloseMenu(int localClientNum, const char *menuName);
void __cdecl UI_Init();
void __cdecl UI_ShowAcceptInviteWarning();
void __cdecl UI_ShowReadingSaveDevicePopup();
void __cdecl UI_HideReadingSaveDevicePopup();
int __cdecl UI_IsFullscreen();
float __cdecl UI_GetBlurRadius();
char *__cdecl UI_SafeTranslateString(const char *reference);
int __cdecl UI_AnyFullScreenMenuVisible(int localClientNum);
void __cdecl UI_FilterStringForButtonAnimation(char *str, unsigned int strMaxSize);
void __cdecl UI_ReplaceConversions(
    const char *sourceString,
    ConversionArguments *arguments,
    char *outputString,
    size_t outputStringSize);
void __cdecl UI_CloseFocusedMenu();
int __cdecl UI_PopupScriptMenu(const char *menuName, bool useMouse);
void UI_PlayerStart();
void __cdecl UI_Refresh();
int __cdecl UI_OwnerDrawWidth(int ownerDraw, Font_s *font, double scale);
void __cdecl UI_DrawKeyBindStatus(
    int localClientNum,
    rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    int textStyle);
void __cdecl UI_DrawLoggedInUserName(rectDef_s *rect, Font_s *font, double scale, float *color, int textStyle);
void __cdecl UI_SavegameSort(int column, int force);
void __cdecl UI_DrawConnectScreen();
char *__cdecl UI_ReplaceConversionString(const char *sourceString, const char *replaceString);
char *__cdecl UI_ReplaceConversionInt(const char *sourceString, int replaceInt);
char *__cdecl UI_ReplaceConversionInts(
    const char *sourceString,
    int numInts,
    char *replaceInts,
    int a4,
    int a5,
    int a6,
    __int64 a7);
int __cdecl UI_Popup(int localClientNum, const char *menu);
void __cdecl UI_DrawLoggedInUser(rectDef_s *rect, Font_s *font, double scale, float *color, int textStyle);
void __cdecl UI_MouseEvent(int localClientNum, int x, int y);

extern const dvar_t *ui_showList;
extern const dvar_t *ui_isSaving;
extern const dvar_t *ui_startupActiveController;
extern const dvar_t *ui_skipMainLockout;
extern const dvar_t *ui_useSuggestedWeapons;
extern const dvar_t *ui_saveMessageMinTime;
extern const dvar_t *ui_showMenuOnly;
extern const dvar_t *ui_bigFont;
extern const dvar_t *ui_cinematicsTimestamp;
extern const dvar_t *ui_mousePitch;
extern const dvar_t *ui_extraBigFont;
extern const dvar_t *ui_nextMission;
extern const dvar_t *uiscript_debug;
extern const dvar_t *ui_autoContinue;
extern const dvar_t *ui_smallFont;
extern const dvar_t *ui_hideMap;
extern const dvar_t *ui_savegame;
extern const dvar_t *ui_drawCrosshairNames;
extern const dvar_t *ui_borderLowLightScale;
extern const dvar_t *ui_campaign;

extern sharedUiInfo_t sharedUiInfo;
extern SaveTimeGlob ui_saveTimeGlob;