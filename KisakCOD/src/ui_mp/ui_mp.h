#pragma once

#include <qcommon/qcommon.h>
#include <qcommon/net_chan_mp.h>

#include <gfx_d3d/r_font.h>

#include <xanim/xanim.h>

enum UILocalVarType : __int32
{                                       // ...
    UILOCALVAR_INT = 0x0,
    UILOCALVAR_FLOAT = 0x1,
    UILOCALVAR_STRING = 0x2,
};
enum uiMenuCommand_t : __int32
{                                       // ...
    UIMENU_NONE = 0x0,
    UIMENU_MAIN = 0x1,
    UIMENU_INGAME = 0x2,
    UIMENU_NEED_CD = 0x3,
    UIMENU_BAD_CD_KEY = 0x4,
    UIMENU_PREGAME = 0x5,
    UIMENU_POSTGAME = 0x6,
    UIMENU_WM_QUICKMESSAGE = 0x7,
    UIMENU_WM_AUTOUPDATE = 0x8,
    UIMENU_SCRIPT_POPUP = 0x9,
    UIMENU_SCOREBOARD = 0xA,
    UIMENU_ENDOFGAME = 0xB,
};
struct ScreenPlacement // sizeof=0x44
{                                       // ...
    float scaleVirtualToReal[2];        // ...
    float scaleVirtualToFull[2];        // ...
    float scaleRealToVirtual[2];
    float virtualViewableMin[2];
    float virtualViewableMax[2];
    float realViewportSize[2];
    float realViewableMin[2];
    float realViewableMax[2];
    float subScreenLeft;
};
union UILocalVar_u // sizeof=0x4
{                                       // ...
    UILocalVar_u()
    {
        integer = 0;
    }
    UILocalVar_u(int i)
    {
        integer = i;
    }
    UILocalVar_u(float f)
    {
        value = f;
    }
    UILocalVar_u(const char* str)
    {
        string = str;
    }
    int integer;
    float value;
    const char* string;
};
struct UILocalVar // sizeof=0xC
{                                       // ...
    UILocalVarType type;
    const char *name;
    UILocalVar_u u;
};
struct UILocalVarContext // sizeof=0xC00
{                                       // ...
    UILocalVar table[256];
};

struct UiContext_cursor // sizeof=0x8
{                                       // ...
    float x;
    float y;
};
struct PlayerProfileStatus // sizeof=0x104
{                                       // ...
    int sortDir;
    int displayProfile[64];
};
struct UiContext // sizeof=0x1678
{                                       // ...
    int localClientNum;
    float bias;
    int realTime;
    int frameTime;
    UiContext_cursor cursor;
    int isCursorVisible;
    int screenWidth;
    int screenHeight;
    float screenAspect;
    float FPS;
    float blurRadiusOut;
    menuDef_t *Menus[640];
    int menuCount;
    menuDef_t *menuStack[16];
    int openMenuCount;
    UILocalVarContext localVars;        // ...
};

struct uiInfo_s // sizeof=0x24B0
{
    UiContext uiDC;
    int myTeamCount;
    int playerRefresh;
    int playerIndex;
    int playerProfileCount;
    const char *playerProfileName[64];
    PlayerProfileStatus playerProfileStatus;
    int timeIndex;
    int previousTimes[4];
    uiMenuCommand_t currentMenuType;
    bool allowScriptMenuResponse;
    char findPlayerName[1024];
    char foundPlayerServerAddresses[16][64];
    char foundPlayerServerNames[16][64];
    // padding byte
    // padding byte
    // padding byte
    int numFoundPlayerServers;
    int nextFindPlayerRefresh;
};

struct ConversionArguments // sizeof=0x28
{                                       // ...
    int argCount;                       // ...
    const char *args[9];                // ...
};
struct CachedAssets_t // sizeof=0x48
{                                       // ...
    Material *scrollBarArrowUp;         // ...
    Material *scrollBarArrowDown;       // ...
    Material *scrollBarArrowLeft;       // ...
    Material *scrollBarArrowRight;      // ...
    Material *scrollBar;                // ...
    Material *scrollBarThumb;           // ...
    Material *sliderBar;                // ...
    Material *sliderThumb;              // ...
    Material *whiteMaterial;            // ...
    Material *cursor;                   // ...
    Font_s *bigFont;                    // ...
    Font_s *smallFont;                  // ...
    Font_s *consoleFont;                // ...
    Font_s *boldFont;                   // ...
    Font_s *textFont;                   // ...
    Font_s *extraBigFont;               // ...
    Font_s *objectiveFont;              // ...
    snd_alias_list_t *itemFocusSound;
};
struct gameTypeInfo // sizeof=0x8
{                                       // ...
    const char *gameType;               // ...
    const char *gameTypeName;           // ...
};
struct mapInfo // sizeof=0xA0
{                                       // ...
    const char *mapName;                // ...
    const char *mapLoadName;            // ...
    const char *imageName;              // ...
    const char *opponentName;
    int teamMembers;
    int typeBits;                       // ...
    int timeToBeat[32];
    Material *levelShot;                // ...
    int active;                         // ...
};
struct modInfo_t // sizeof=0x8
{                                       // ...
    const char *modName;                // ...
    const char *modDescr;               // ...
};
struct serverStatus_s // sizeof=0x2028
{                                       // ...
    char string[8192];                  // ...
    netadr_t address;                   // ...
    int time;
    int startTime;                      // ...
    int pending;
    int print;
    int retrieved;                      // ...
};
struct serverStatusInfo_t // sizeof=0xD04
{                                       // ...
    char address[64];
    const char *lines[128][4];          // ...
    char text[1024];
    char pings[192];
    int numLines;                       // ...
};
struct pendingServer_t // sizeof=0x8C
{                                       // ...
    char adrstr[64];
    char name[64];                      // ...
    int startTime;                      // ...
    int serverNum;
    int valid;                          // ...
};
struct pendingServerStatus_t // sizeof=0x8C4
{                                       // ...
    int num;                            // ...
    pendingServer_t server[16];         // ...
};
struct sharedUiInfo_t // sizeof=0x1C5B0
{                                       // ...
    CachedAssets_t assets;              // ...
    int playerCount;                    // ...
    char playerNames[64][32];           // ...
    char teamNames[64][32];
    int playerClientNums[64];           // ...
    int numGameTypes;                   // ...
    gameTypeInfo gameTypes[32];         // ...
    int numCustomGameTypes;             // ...
    gameTypeInfo customGameTypes[32];
    char customGameTypeCancelState[2048];
    int numJoinGameTypes;               // ...
    gameTypeInfo joinGameTypes[32];     // ...
    int mapCount;                       // ...
    mapInfo mapList[128];               // ...
    Material *serverHardwareIconList[10]; // ...
    modInfo_t modList[64];              // ...
    int modCount;                       // ...
    int modIndex;                       // ...
    serverStatus_s serverStatus;        // ...
    _BYTE gap8EB4[73968];               // ...
    char serverStatusAddress[64];       // ...
    serverStatusInfo_t serverStatusInfo; // ...
    int nextServerStatusRefresh;        // ...
    pendingServerStatus_t pendingServerStatus; // ...
};

UILocalVarContext *__cdecl UI_GetLocalVarsContext(int localClientNum);
void __cdecl TRACK_ui_main();
void __cdecl UI_DrawSides(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float w,
    float h,
    int horzAlign,
    int vertAlign,
    float size,
    const float *color);
void __cdecl UI_DrawTopBottom(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float w,
    float h,
    int horzAlign,
    int vertAlign,
    float size,
    const float *color);
void __cdecl UI_DrawRect(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float width,
    float height,
    int horzAlign,
    int vertAlign,
    float size,
    const float *color);
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
    const float *loColor);
int __cdecl UI_TextWidth(const char *text, int maxChars, Font_s *font, float scale);
int __cdecl UI_TextHeight(Font_s *font, float scale);
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
    int style);
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
    bool cinematic);
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
    int style);
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
    char cursor);
Font_s *__cdecl UI_GetFontHandle(const ScreenPlacement *scrPlace, int fontEnum, float scale);
void __cdecl UI_UpdateTime(int localClientNum, int realtime);
void __cdecl UI_DrawBuildNumber(int localClientNum);
void __cdecl UI_Refresh(int localClientNum);
void __cdecl UI_Shutdown(int localClientNum);
char *__cdecl GetMenuBuffer(char *filename);
XModelPiece *__cdecl GetMenuBuffer_FastFile(const char *filename);
int __cdecl Load_ScriptMenu(int localClientNum, char *pszMenu, int imageTrack);
MenuList *__cdecl Load_ScriptMenuInternal(char *pszMenu, int imageTrack);
char *__cdecl UI_GetMapDisplayName(const char *pszMap);
char *__cdecl UI_GetMapDisplayNameFromPartialLoadNameMatch(const char *mapName, int *mapLoadNameLen);
char *__cdecl UI_GetGameTypeDisplayName(const char *pszGameType);
int __cdecl UI_OwnerDrawWidth(int ownerDraw, Font_s *font, float scale);
void __cdecl UI_DrawMapLevelshot(int localClientNum);
void __cdecl UI_LoadIngameMenus(int localClientNum);
void __cdecl UI_SetMap(char *mapname, char *gametype);
int __cdecl UI_GetTalkerClientNum(int localClientNum, int num);
void __cdecl ProfLoad_DrawOverlay(rectDef_s *rect);
char __cdecl UI_DrawRecordLevel(int localClientNum, rectDef_s *rect);
void __cdecl UI_DrawGameType(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    int textStyle);
void __cdecl UI_DrawNetGameType(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    int textStyle);
void __cdecl UI_DrawJoinGameType(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    int textStyle);
void __cdecl UI_DrawNetFilter(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    int textStyle);
void __cdecl UI_DrawNetSource(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    int textStyle);
void __cdecl UI_DrawMapPreview(int localClientNum, const rectDef_s *rect, float scale, const float *color);
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
    char textAlignMode);
void __cdecl UI_DrawServerRefreshDate(
    int localClientNum,
    rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    int textStyle);
void __cdecl UI_DrawKeyBindStatus(
    int localClientNum,
    rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    int textStyle);
void __cdecl UI_DrawLocalTalking(int localClientNum, const rectDef_s *rect, const float *color);
void __cdecl UI_DrawTalkerNum(
    int localClientNum,
    int num,
    rectDef_s *rect,
    Font_s *font,
    float *color,
    float textScale,
    int style);
bool __cdecl UI_OwnerDrawVisible(__int16 flags);
int __cdecl UI_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key);
int __cdecl UI_CheckExecKey(int localClientNum, int key);
void __cdecl UI_RunMenuScript(int localClientNum, char **args, const char *actualScript);
void __cdecl UI_ServersSort(int column, int force);
int __cdecl UI_ServersQsortCompare(unsigned int *arg1, unsigned int *arg2);
void UI_VerifyLanguage();
void __cdecl UI_UpdateDisplayServers(uiInfo_s *uiInfo);
char __cdecl UI_GetOpenOrCloseMenuOnDvarArgs(
    const char **args,
    const char *cmd,
    char *dvarName,
    int dvarNameLen,
    char *testValue,
    int testValueLen,
    char *menuName,
    int menuNameLen);
void __cdecl UI_OpenMenuOnDvar(
    uiInfo_s *uiInfo,
    const char *cmd,
    const char *menuName,
    const char *dvarName,
    const char *testValue);
bool __cdecl UI_DvarValueTest(const char *cmd, const char *dvarName, const char *testValue, bool wantMatch);
void __cdecl UI_CloseMenuOnDvar(
    uiInfo_s *uiInfo,
    const char *cmd,
    const char *menuName,
    const char *dvarName,
    const char *testValue);
void __cdecl UI_BuildServerDisplayList(uiInfo_s *uiInfo, int force);
void __cdecl UI_BinaryServerInsertion(unsigned int num);
void __cdecl UI_InsertServerIntoDisplayList(unsigned int num, int position);
int UI_ClearDisplayedServers();
void __cdecl UI_BuildServerStatus(uiInfo_s *uiInfo, int force);
int __cdecl UI_FeederCount(int localClientNum, float feederID);
void __cdecl UI_BuildPlayerList(int localClientNum);
char *__cdecl UI_FeederItemText(
    int localClientNum,
    itemDef_s *item,
    const float feederID,
    int index,
    unsigned int column,
    Material **handle);
Material *__cdecl UI_FeederItemImage(float feederID, int index);
void __cdecl UI_FeederItemColor(
    int localClientNum,
    itemDef_s *item,
    float feederID,
    int index,
    int column,
    float *color);
void __cdecl UI_OverrideCursorPos(int localClientNum, itemDef_s *item);
void __cdecl UI_FeederSelection(int localClientNum, float feederID, int index);
void UI_GetGameTypesList();
void UI_GetGameTypesList_FastFile();
void __cdecl UI_Pause(int localClientNum, int b);
void __cdecl UI_OpenMenu_f();
void __cdecl UI_ListMenus_f();
void __cdecl CL_SelectStringTableEntryInDvar_f();
void __cdecl UI_CloseMenu_f();
void __cdecl UI_Init(int localClientNum);
const dvar_s *UI_RegisterDvars();
Font_s *UI_AssetCache();
void __cdecl UI_KeyEvent(int localClientNum, int key, int down);
int __cdecl UI_GetActiveMenu(int localClientNum);
const char *__cdecl UI_GetTopActiveMenuName(int localClientNum);
int __cdecl UI_SetActiveMenu(int localClientNum, int menu);
int __cdecl UI_IsFullscreen(int localClientNum);
void __cdecl UI_DrawConnectScreen(int localClientNum);
void __cdecl Text_PaintCenter(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    Font_s *const font,
    float scale,
    const float *color,
    char *text,
    const float *glowColor);
void __cdecl Text_PaintCenterWithDots(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    Font_s *const font,
    float scale,
    const float *color,
    const char *text,
    const float *glowColor);
double __cdecl UI_GetBlurRadius(int localClientNum);
void UI_StopServerRefresh();
void __cdecl UI_DoServerRefresh(uiInfo_s *uiInfo);
void __cdecl UI_StartServerRefresh(int localClientNum, int full);
void __cdecl UI_UpdatePendingPings(uiInfo_s *uiInfo);
char *__cdecl UI_SafeTranslateString(const char *reference);
bool __cdecl UI_AnyMenuVisible(int localClientNum);
char *__cdecl UI_ReplaceConversionString(char *sourceString, const char *replaceString);
char *__cdecl UI_ReplaceConversionInt(char *sourceString, int replaceInt);
void __cdecl UI_ReplaceConversions(
    char *sourceString,
    ConversionArguments *arguments,
    char *outputString,
    int outputStringSize);
void __cdecl UI_CloseAll(int localClientNum);
void __cdecl UI_CloseFocusedMenu(int localClientNum);
int __cdecl UI_Popup(int localClientNum, const char *menu);
int __cdecl UI_PopupScriptMenu(int localClientNum, const char *menuName, bool useMouse);
void __cdecl UI_ClosePopupScriptMenu(int localClientNum, bool allowResponse);
char __cdecl UI_AllowScriptMenuResponse(int localClientNum);
void __cdecl UI_CloseInGameMenu(int localClientNum);
void __cdecl UI_CloseAllMenus(int localClientNum);
bool __cdecl Menu_IsMenuOpenAndVisible(int localClientNum, const char *menuName);
bool __cdecl UI_ShouldDrawCrosshair();

extern const dvar_t *ui_showList;
extern const dvar_t *ui_customClassName;
extern const dvar_t *ui_mapname;
extern const dvar_t *ui_netSource;
extern const dvar_t *ui_showMenuOnly;
extern const dvar_t *ui_bigFont;
extern const dvar_t *ui_cinematicsTimestamp;
extern const dvar_t *ui_connectScreenTextGlowColor;
extern const dvar_t *ui_selectedPlayer;
extern const dvar_t *ui_extraBigFont;
extern const dvar_t *ui_drawCrosshair;
extern const dvar_t *ui_buildSize;
extern const dvar_t *ui_allow_classchange;
extern const dvar_t *ui_hud_hardcore;
extern const dvar_t *ui_gametype;
extern const dvar_t *uiscript_debug;
extern const dvar_t *ui_currentMap;
extern const dvar_t *vehDriverViewHeightMin;
extern const dvar_t *ui_playerPartyColor;
extern const dvar_t *ui_allow_teamchange;
extern const dvar_t *ui_buildLocation;
extern const dvar_t *ui_smallFont;
extern const dvar_t *ui_customModeEditName;
extern const dvar_t *ui_uav_axis;
extern const dvar_t *ui_serverStatusTimeOut;
extern const dvar_t *vehDriverViewHeightMax;
extern const dvar_t *ui_uav_allies;
extern const dvar_t *ui_uav_client;
extern const dvar_t *ui_borderLowLightScale;
extern const dvar_t *ui_partyFull;
extern const dvar_t *ui_customModeName;

extern sharedUiInfo_t sharedUiInfo;