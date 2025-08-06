#pragma once
#include <qcommon/ent.h>

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif
#include <game/enthandle.h>
#include <qcommon/net_chan.h>
#include <client/client.h>

#include <bgame/bg_local.h>

enum clientConnected_t : __int32
{
    CON_DISCONNECTED = 0x0,
    CON_CONNECTING = 0x1,
    CON_CONNECTED = 0x2,
};

struct clientPersistent_t
{
    clientConnected_t connected;
    usercmd_s cmd;
    usercmd_s oldcmd;
    int maxHealth;
    float moveSpeedScaleMultiplier;
};

struct gclient_s
{
    playerState_s ps;
    clientPersistent_t pers;
    int noclip;
    int ufo;
    int bFrozen;
    int buttons;
    int oldbuttons;
    int latched_buttons;
    int buttonsSinceLastFrame;
    float fGunPitch;
    float fGunYaw;
    float fGunXOfs;
    float fGunYOfs;
    float fGunZOfs;
    int damage_blood;
    float damage_from[3];
    int damage_fromWorld;
    int respawnTime;
    float currentAimSpreadScale;
    gentity_s *pHitHitEnt;
    EntHandle pLookatEnt;
    float prevLinkedInvQuat[4];
    bool prevLinkAnglesSet;
    bool link_rotationMovesEyePos;
    bool link_doCollision;
    bool link_useTagAnglesForViewAngles;
    bool linkAnglesLocked;
    float linkAnglesFrac;
    float linkAnglesMinClamp[2];
    float linkAnglesMaxClamp[2];
    int inControlTime;
    int lastTouchTime;
    EntHandle useHoldEntity;
    int useHoldTime;
    int useButtonDone;
    int bDisableAutoPickup;
    int invulnerableExpireTime;
    bool invulnerableActivated;
    bool invulnerableEnabled;
    bool playerMoved;
    float playerLOSCheckPos[2];
    float playerLOSCheckDir[2];
    int playerLOSPosTime;
    int playerADSTargetTime;
    unsigned int lastWeapon;
    bool previouslyFiring;
    bool previouslyUsingNightVision;
    int groundTiltEntNum;
};

struct client_t
{
    int state;
    serverCommands_s reliableCommands;
    usercmd_s lastUsercmd;
    gentity_s *gentity;
    playerState_s frames[1];
    netchan_t netchan;
};

struct server_demo_save_t
{
    unsigned __int8 *buf;
    int bufLen;
};

struct server_demo_history_t
{
    bool manual;
    int time;
    char name[64];
    server_demo_save_t save;
    unsigned __int8 *cmBuf;
    int cmBufLen;
    unsigned __int8 *freeEntBuf;
    int freeEntBufLen;
    int randomSeed;
    int nextFramePos;
    usercmd_s lastUsercmd;
    int msgBit;
    int msgReadcount;
    int readType;
};

struct SaveImmediate
{
    void *f;
};

struct FileSkip
{
    int time;
    int fileEndOffset;
};

struct FileMarkSkip
{
    char name[64];
    int fileOffset;
};

enum SaveType : __int32
{
    SAVE_TYPE_INTERNAL = 0x0,
    SAVE_TYPE_AUTOSAVE = 0x1,
    SAVE_TYPE_CONSOLE = 0x2,
};

struct __declspec(align(4)) PendingSave
{
    char filename[64];
    char description[256];
    char screenShotName[64];
    int saveId;
    SaveType saveType;
    unsigned int commitLevel;
    bool suppressPlayerNotify;
};

enum ServerFrameExtent : __int32
{
    SV_FRAME_DO_ALL = 0x0,
    SV_FRAME_DO_SMOOTHING = 0x1,
};

// sv_ccmds
void __cdecl SV_SetValuesFromSkill();
void SV_DifficultyEasy();
void SV_DifficultyMedium();
void SV_DifficultyHard();
void SV_DifficultyFu();
int __cdecl ReadSaveHeader(const char *filename, SaveHeader *header);
int __cdecl ExtractMapStringFromSaveGame(const char *filename, char *mapname);
void __cdecl ShowLoadErrorsSummary(const char *mapName, unsigned int count, int a3);
void __cdecl SV_ClearLoadGame();
void __cdecl SV_MapRestart(int savegame, int loadScripts);
int __cdecl CheckForSaveGame(char *mapname, char *filename);
int __cdecl SV_CheckLoadGame();
void __cdecl SV_RequestMapRestart(int loadScripts);
void __cdecl SV_FastRestart_f();
void __cdecl SV_MapRestart_f();
void __cdecl SV_NextLevel_f();
void __cdecl SV_LoadGame_f();
void __cdecl SV_ForceSelectSaveDevice_f();
void __cdecl SV_SelectSaveDevice_f();
void __cdecl CheckSaveExists(const char *filename);
void __cdecl SV_LoadGameContinue_f();
void __cdecl SV_ScriptUsage_f();
void SV_ScriptVarUsage_f_usage();
void SV_ScriptVarUsage_f();
void SV_ScriptProfile_f();
void SV_ScriptBuiltin_f();
void SV_ScriptProfileReset_f();
void __cdecl SV_ScriptProfileFile_f();
void __cdecl SV_StringUsage_f();
void SV_SaveGame_f();
void __cdecl SV_SaveGameLastCommit_f();
void __cdecl SV_RemoveOperatorCommands();
void SV_Map_f();
void __cdecl SV_AddOperatorCommands();


// sv_client
void __cdecl SV_DirectConnect();
void __cdecl SV_SendClientGameState(client_t *client);
void __cdecl SV_SendGameState();
void __cdecl SV_ClientEnterWorld(client_t *client);
float __cdecl SV_FX_GetVisibility(const float *start, const float *end);
void __cdecl SV_ExecuteClientCommand(const char *s);
void __cdecl SV_ClientThink(usercmd_s *cmd);
gentity_s *__cdecl SV_GetEntityState(int entnum);
void __cdecl SV_TrackPlayerDied();
void __cdecl SV_AddToPlayerScore(int amount);



// sv_demo
struct SaveGame;

void __cdecl TRACK_sv_demo();
unsigned int __cdecl SV_GetHistoryIndex(server_demo_history_t *history);
int __cdecl SV_GetBufferIndex(unsigned __int8 *ptr);
void __cdecl SV_HistoryFree(unsigned __int8 *ptr, int size);
int __cdecl SV_HistoryAlloc(server_demo_history_t *history, unsigned __int8 **pData, int size);
int __cdecl SV_MsgAlloc(unsigned int maxsize);
void SV_CheckDemoSize();
bool __cdecl SV_DemoWrite(const void *buffer, unsigned int len, _iobuf *file);
int __cdecl SV_FindTimeSkipIndex(int time);
FileMarkSkip *__cdecl SV_FindMarkSkip(const char *name);
void __cdecl SV_TruncateHistoryTimeCache(int maxTime);
int SV_ClearHistoryMarkCache();
// attributes: thunk
void __cdecl SV_TruncateHistoryCache(int maxTime);
int SV_SetAutoSaveHistoryTime();
void __cdecl SV_ResetDemo();
_iobuf *SV_ClearHistoryCache();
void __cdecl SV_FreeDemoSaveBuf(server_demo_save_t *save);
void __cdecl SV_FreeHistoryData(server_demo_history_t *history);
void __cdecl SV_FreeHistory(server_demo_history_t **history);
void SV_FreeDemoMsg();
int __cdecl SV_WaitForSaveHistoryDone();
void __cdecl SV_ShutdownDemo();
int __cdecl SV_AddDemoSave(SaveGame *savehandle, server_demo_save_t *save, int createSave);
_iobuf *__cdecl SV_DemoOpenFile(const char *fileName);
void __cdecl SV_InitWriteDemo(int randomSeed);
void __cdecl SV_InitReadDemoSavegame(SaveGame **saveHandle);
int __cdecl SV_InitDemoSavegame(SaveGame **save);
bool __cdecl SV_IsDemoPlaying();
bool __cdecl SV_UsingDemoSave();
void __cdecl SV_RecordClientCommand(const char *s);
void __cdecl SV_RecordClientThink(usercmd_s *cmd);
void __cdecl SV_RecordFxVisibility(double visibility);
void __cdecl SV_RecordCheatsOk(int cheatsOk);
void __cdecl SV_RecordIsRecentlyLoaded(bool isRecentlyLoaded);
void __cdecl SV_Record_Dvar_GetVariantString(const char *buffer);
void __cdecl SV_RecordButtonPressed(int buttonPressed);
void __cdecl SV_GetFreeDemoName(const char *baseName, int demoCount, char *testDemoName);
void __cdecl SV_SaveDemoImmediate(SaveImmediate *save);
void __cdecl SV_WriteDemo(SaveGame *save);
void __cdecl SV_SaveDemo(const char *demoName, const char *description, unsigned __int32 saveType);
void __cdecl SV_AutoSaveDemo(const char *baseName, const char *description, int demoCount, bool force);
void SV_EnableAutoDemo();
void __cdecl SV_SaveDemo_f();
void SV_DemoRestart();
void __cdecl SV_DemoRestart_f();
int __cdecl SV_DemoHasMark();
void __cdecl SV_LoadDemo(SaveGame *save, void *fileHandle);
bool __cdecl SV_RecordingDemo();
int __cdecl SV_Demo_Dvar_Set(const char *var_name, const char *value);
int __cdecl SV_WriteDemoSaveBuf(server_demo_save_t *save);
bool __cdecl SV_WriteHistory(_iobuf *fileHistory, const server_demo_history_t *history);
void __cdecl SV_SaveHistoryTime(server_demo_history_t *history);
void __cdecl SV_SaveHistoryMark(const server_demo_history_t *history);
void __cdecl SV_SaveHistory(server_demo_history_t *history);
void __cdecl  SV_SaveHistoryLoop(unsigned int threadContext);
bool SV_InitHistorySaveThread();
void __cdecl SV_InitDemoSystem();
server_demo_history_t *__cdecl SV_DemoGetFreeBuffer();
int __cdecl SV_HistoryIsNew(server_demo_history_t *history);
void __cdecl SV_ClearInfrequentTimeMarks(server_demo_history_t *history);
server_demo_history_t *__cdecl SV_DemoGetBuffer();
server_demo_history_t *__cdecl SV_GetMarkHistory(const char *name);
int __cdecl SV_DemoSaveHistory(server_demo_history_t *history);
void __cdecl SV_DemoMark_f();
bool __cdecl SV_DemoRead(void *buffer, unsigned int len, _iobuf *file);
int __cdecl SV_DemoAllocRead(
    server_demo_history_t *history,
    unsigned __int8 **buffer,
    unsigned int len,
    _iobuf *file);
bool __cdecl SV_ReadHistory(_iobuf *fileHistory, server_demo_history_t *history);
bool __cdecl SV_DemoLoadHistory(_iobuf *fileHistory, int fileOffset);
bool __cdecl SV_LoadHistoryForTime(int time);
bool __cdecl SV_ActiveHistoryIsMark(const char *name);
int __cdecl SV_LoadHistoryForMark(const char *name);
void __cdecl SV_DemoGoto_f();
void __cdecl SV_DemoSetNextLevelTime(int time);
void __cdecl SV_DemoBack_f();
void __cdecl SV_DemoForward_f();
void __cdecl SV_DemoFullForward_f();
void __cdecl SV_DemoLive_f();
void __cdecl SV_DemoInfo_f();
int __cdecl SV_GetDemoStartTime();
int __cdecl SV_GetDemoEndTime();
int __cdecl SV_CheckAutoSaveHistory(int setTooSoon);
void SV_DoAutoSaveHistory();
void __cdecl SV_UpdateDemo();
void SV_DemoLive();
void __cdecl SV_EndDemo(bool error);
void SV_ReadNextDemoType();
bool __cdecl SV_InitReadDemo(int *randomSeed);
bool __cdecl SV_InitDemo(int *randomSeed);
bool __cdecl SV_ReadPacket(int framePos);
float __cdecl SV_DemoFxVisibility();
int __cdecl SV_DemoCheatsOk();
bool __cdecl SV_DemoIsRecentlyLoaded();
const char *__cdecl SV_Demo_Dvar_GetVariantString();
int __cdecl SV_DemoButtonPressed();


// sv_main
enum serverState_t : __int32
{
    SS_DEAD = 0x0,
    SS_LOADING = 0x1,
    SS_GAME = 0x2,
};
struct svEntity_s
{
    unsigned __int16 worldSector;
    unsigned __int16 nextEntityInWorldSector;
    int linkcontents;
    float linkmin[2];
    float linkmax[2];
};
struct server_demo_t
{
    int startTime;
    int endTime;
    bool nextLevelplaying;
    server_demo_history_t *nextLevelSave;
    int nextLevelTime;
    bool changed;
    bool recording;
    bool playing;
    int nextFramePos;
    int readType;
    int forwardMsec;
    msg_t msg;
    server_demo_save_t save;
    bool startLive;
    int autoSaveTime;
};
struct snapshotEntityNumbers_t
{
    int numSnapshotEntities;
    int snapshotEntities[2048];
};

#define MAX_CONFIGSTRINGS 2815
struct server_t
{
    serverState_t state;
    int timeResidual;
    bool clearTimeResidual;
    int pendingSnapshot;
    volatile int restartServerThread;
    volatile int requestSaveGame;
    volatile int savingGame;
    bool smp;
    int waitSnapshotTime;
    volatile int serverExecTime;
    volatile int serverFrameTime;
    volatile int serverFrameTimeMin;
    volatile int serverFrameTimeMax;
    int inFrame;
    int clientMessageTimeout;
    int partialFrametime;
    int nextFrameTime;
    cmodel_t *models[512];
    unsigned __int16 emptyConfigString;
    unsigned __int16 configstrings[MAX_CONFIGSTRINGS];
    svEntity_s svEntities[2176];
    gentity_s *gentities;
    int gentitySize;
    int num_entities;
    playerState_s *gameClients;
    int gameClientSize;
    int checksum;
    server_demo_t demo;
    int levelTime;
    int skelTimeStamp;
    unsigned int skelMemPos;
    int previousTime;
    int previousTimeIndex;
    int previousTotalTimes[10];
    int previousErrorTimes[10];
    snapshotEntityNumbers_t entityNumbers;
    char cmd[1024];
    char cmd2[1024];
    char cmd3[1024];
    char cmd4[1024];
};

struct serverStatic_t
{
    int initialized;
    int snapFlagServerBit;
    client_t *clients;
    int numSnapshotEntities;
    int nextSnapshotEntities;
    netadr_t authorizeAddress;
    int playerDeaths;
    int playerScore;
};

struct __declspec(align(4)) PendingSaveList
{
    PendingSave pendingSaves[3];
    volatile int count;
    bool isAutoSaving;
};

void __cdecl TRACK_sv_main();
char *__cdecl SV_ExpandNewlines(char *in);
void __cdecl SV_DumpServerCommands(client_t *client);
void __cdecl AppendCommandsForInternalSave(const char *filename);
void __cdecl SV_InitiatePendingSave(
    const char *filename,
    const char *description,
    const char *screenshot,
    SaveType saveType,
    unsigned int commitLevel,
    PendingSave *pendingSave,
    bool suppressPlayerNotify);
int __cdecl SV_AddPendingSave(
    const char *filename,
    const char *description,
    const char *screenshot,
    SaveType saveType,
    unsigned int commitLevel,
    bool suppressPlayerNotify);
int __cdecl SV_ProcessPendingSave(PendingSave *pendingSave);
int __cdecl SV_ProcessPendingSaves();
void __cdecl SV_ClearPendingSaves();
int __cdecl SV_IsInternalSave(const char *filename);
void __cdecl SV_SetLastSaveName(const char *filename);
void __cdecl SV_AddServerCommand(client_t *client, const char *cmd);
void SV_SendServerCommand(client_t *cl, const char *fmt, ...);
void __cdecl SV_SaveServerCommands(SaveGame *save);
void __cdecl SV_LoadServerCommands(SaveGame *save);
void __cdecl SV_PreFrame();
int __cdecl SV_RunFrame(ServerFrameExtent extent, int timeCap);
void SV_ProcessPostFrame();
void __cdecl SV_UpdatePerformanceFrame(int time);
bool __cdecl SV_CheckSkipTimeout();
int __cdecl SV_CheckStartServer();
int __cdecl SV_WaitStartServer();
void __cdecl  SV_ServerThread(unsigned int threadContext);
void __cdecl SV_InitServerThread();
void __cdecl SV_ExitAfterTime();
void SV_WakeServer();
void __cdecl SV_WaitServer();
void __cdecl SV_InitSnapshot();
void __cdecl SV_WaitSaveGame();
void __cdecl SV_BeginSaveGame();
void __cdecl SV_EndSaveGame();
int __cdecl SV_WaitServerSnapshot();
bool __cdecl SV_ReachedServerCommandThreshold();
void __cdecl SV_FrameInternal(int msec);
int __cdecl SV_GetPartialFrametime();
int __cdecl SV_ForwardFrame();
int __cdecl SV_ClientFrameRateFix(int msec);
int __cdecl SV_Frame(int msec);
bool __cdecl SV_SaveMemory_IsRecentlyLoaded();

extern server_t sv;
extern serverStatic_t svs;

extern int com_time;
extern int com_inServerFrame;

extern const dvar_t *sv_lastSaveGame;
extern const dvar_t *sv_smp;
extern const dvar_t *sv_player_damageMultiplier;
extern const dvar_t *sv_player_maxhealth;
extern const dvar_t *sv_saveOnStartMap;
extern const dvar_t *sv_gameskill;
extern const dvar_t *sv_mapname;
extern const dvar_t *sv_saveDeviceAvailable;
extern const dvar_t *sv_cheats;
extern const dvar_t *player_healthEasy;
extern const dvar_t *player_healthHard;
extern const dvar_t *sv_player_deathInvulnerableTime;
extern const dvar_t *runForTime;
extern const dvar_t *sv_saveGameSuccess;
extern const dvar_t *sv_saveGameAvailable;
extern const dvar_t *sv_saveGameNotReadable;
extern const dvar_t *replay_autosave;
extern const dvar_t *player_healthMedium;
extern const dvar_t *player_healthFu;
extern const dvar_t *replay_asserts;

inline int SV_GetCheckSum()
{
    return sv.checksum;
}


// sv_snapshot
void __cdecl SV_WriteSnapshotToClient(client_t *client, msg_t *msg);
void __cdecl SV_UpdateServerCommandsToClient(client_t *client);
void __cdecl SV_AddEntToSnapshot(int entnum);
void __cdecl SV_AddEntitiesVisibleFromPoint(int clientNum);
void __cdecl SV_BuildClientSnapshot(client_t *client);
void __cdecl SV_SendMessageToClient(msg_t *msg, client_t *client);
void __cdecl SV_BuildAndSendClientSnapshot(client_t *client);
void __cdecl SV_SendClientMessages();
void __cdecl SV_WriteSnapshotToClientCmd(void *cmdData);
void __cdecl SV_ArchiveSnapshotCmd(void *cmdData);
