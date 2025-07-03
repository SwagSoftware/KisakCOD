#pragma once

#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include <qcommon/qcommon.h>
#include <qcommon/net_chan_mp.h>

#include <xanim/xanim.h>

#include <ui/ui_shared.h>

struct LegacyHacks // sizeof=0x4C
{                                       // ...
    int cl_downloadSize;                // ...
    int cl_downloadCount;               // ...
    int cl_downloadTime;                // ...
    char cl_downloadName[64];           // ...
};

struct PlayerProfileStatus // sizeof=0x104
{                                       // ...
    int sortDir;
    int displayProfile[64];
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
    _BYTE gap8EB4[73968];               // ... // KISAKTODO: clean this thing up
    char serverStatusAddress[64];       // ...
    serverStatusInfo_t serverStatusInfo; // ...
    int nextServerStatusRefresh;        // ...
    pendingServerStatus_t pendingServerStatus; // ...
};



XModelPiece *__cdecl GetMenuBuffer_FastFile(const char *filename);
MenuList *__cdecl Load_ScriptMenuInternal(char *pszMenu, int imageTrack);
char *__cdecl UI_GetMapDisplayName(const char *pszMap);
char *__cdecl UI_GetMapDisplayNameFromPartialLoadNameMatch(const char *mapName, int *mapLoadNameLen);
char *__cdecl UI_GetGameTypeDisplayName(const char *pszGameType);
int __cdecl UI_OwnerDrawWidth(int ownerDraw, Font_s *font, float scale);
bool __cdecl UI_OwnerDrawVisible(__int16 flags);
int __cdecl UI_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key);

void __cdecl UI_UpdateTime(int localClientNum, int realtime);
void __cdecl UI_DrawBuildNumber(int localClientNum);
void __cdecl UI_Refresh(int localClientNum);
void __cdecl UI_Shutdown(int localClientNum);
char *__cdecl GetMenuBuffer(char *filename);
int __cdecl Load_ScriptMenu(int localClientNum, char *pszMenu, int imageTrack);
void __cdecl UI_DrawMapLevelshot(int localClientNum);
void __cdecl UI_LoadIngameMenus(int localClientNum);
void __cdecl UI_SetMap(char *mapname, char *gametype);
int __cdecl UI_GetTalkerClientNum(int localClientNum, int num);
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

int __cdecl UI_CheckExecKey(int localClientNum, int key);
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
void __cdecl UI_BuildPlayerList(int localClientNum);
Material *__cdecl UI_FeederItemImage(float feederID, int index);

void UI_GetGameTypesList();
void UI_GetGameTypesList_FastFile();
void __cdecl UI_OpenMenu_f();
void __cdecl UI_ListMenus_f();
void __cdecl CL_SelectStringTableEntryInDvar_f();
void __cdecl UI_CloseMenu_f();
void __cdecl UI_Init(int localClientNum);
Font_s *UI_AssetCache();
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
void __cdecl UI_UpdatePendingPings(uiInfo_s *uiInfo);
char *__cdecl UI_SafeTranslateString(const char *reference);
char *__cdecl UI_ReplaceConversionString(char *sourceString, const char *replaceString);
char *__cdecl UI_ReplaceConversionInt(char *sourceString, int replaceInt);
void __cdecl UI_ReplaceConversions(
    char *sourceString,
    ConversionArguments *arguments,
    char *outputString,
    int outputStringSize);
void __cdecl UI_CloseAll(int localClientNum);
void __cdecl UI_CloseFocusedMenu(int localClientNum);
int __cdecl UI_PopupScriptMenu(int localClientNum, const char *menuName, bool useMouse);
void __cdecl UI_CloseInGameMenu(int localClientNum);
void __cdecl UI_CloseAllMenus(int localClientNum);
bool __cdecl UI_ShouldDrawCrosshair();
void __cdecl UI_MouseEvent(int localClientNum, int x, int y);

void __cdecl UI_StartServerRefresh(int localClientNum, int full);

extern const dvar_t *ui_customClassName;
extern const dvar_t *ui_mapname;
extern const dvar_t *ui_netSource;
extern const dvar_t *ui_connectScreenTextGlowColor;
extern const dvar_t *ui_selectedPlayer;
extern const dvar_t *ui_drawCrosshair;
extern const dvar_t *ui_buildSize;
extern const dvar_t *ui_allow_classchange;
extern const dvar_t *ui_hud_hardcore;
extern const dvar_t *ui_gametype;
extern const dvar_t *ui_currentMap;
extern const dvar_t *vehDriverViewHeightMin;
extern const dvar_t *ui_playerPartyColor;
extern const dvar_t *ui_allow_teamchange;
extern const dvar_t *ui_buildLocation;
extern const dvar_t *ui_customModeEditName;
extern const dvar_t *ui_uav_axis;
extern const dvar_t *ui_serverStatusTimeOut;
extern const dvar_t *vehDriverViewHeightMax;
extern const dvar_t *ui_uav_allies;
extern const dvar_t *ui_uav_client;
extern const dvar_t *ui_partyFull;
extern const dvar_t *ui_customModeName;

extern sharedUiInfo_t sharedUiInfo; // struct Differs from SP

extern LegacyHacks legacyHacks;


// ui_gameinfo_mp
int __cdecl UI_ParseInfos(const char *buf, int max, char **infos);
void __cdecl UI_LoadArenas();
void UI_LoadArenasFromFile();
void UI_LoadArenasFromFile_FastFile();
