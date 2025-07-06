#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

// sv_ccmds
void __fastcall SV_SetValuesFromSkill();
void SV_DifficultyEasy();
void SV_DifficultyMedium();
void SV_DifficultyHard();
void SV_DifficultyFu();
int __fastcall ReadSaveHeader(const char *filename, SaveHeader *header);
int __fastcall ExtractMapStringFromSaveGame(const char *filename, char *mapname);
void __fastcall ShowLoadErrorsSummary(const char *mapName, unsigned int count, int a3);
void __fastcall SV_ClearLoadGame();
void __fastcall SV_MapRestart(int savegame, int loadScripts);
int __fastcall CheckForSaveGame(char *mapname, char *filename);
int __fastcall SV_CheckLoadGame();
void __fastcall SV_RequestMapRestart(int loadScripts);
void __fastcall SV_FastRestart_f();
void __fastcall SV_MapRestart_f();
// attributes: thunk
void __fastcall SV_NextLevel_f();
void __fastcall SV_LoadGame_f();
void __fastcall SV_ForceSelectSaveDevice_f();
void __fastcall SV_SelectSaveDevice_f();
void __fastcall CheckSaveExists(const char *filename);
void __fastcall SV_LoadGameContinue_f();
// attributes: thunk
void __fastcall SV_ScriptUsage_f();
void SV_ScriptVarUsage_f_usage();
void SV_ScriptVarUsage_f();
void SV_Script//Profile_f();
void SV_ScriptBuiltin_f();
void SV_ScriptProfileReset_f();
// attributes: thunk
void __fastcall SV_ScriptProfileFile_f();
// attributes: thunk
void __fastcall SV_StringUsage_f();
void SV_SaveGame_f();
void __fastcall SV_SaveGameLastCommit_f();
void __fastcall SV_RemoveOperatorCommands();
void SV_Map_f();
void __fastcall SV_AddOperatorCommands();


// sv_client
void __fastcall SV_DirectConnect();
void __fastcall SV_SendClientGameState(client_t *client);
void __fastcall SV_SendGameState();
void __fastcall SV_ClientEnterWorld(client_t *client);
float __fastcall SV_FX_GetVisibility(const float *start, const float *end);
void __fastcall SV_ExecuteClientCommand(const char *s);
void __fastcall SV_ClientThink(usercmd_s *cmd);
// attributes: thunk
gentity_s *__fastcall SV_GetEntityState(int entnum);
void __fastcall SV_TrackPlayerDied();
void __fastcall SV_AddToPlayerScore(int amount);



// sv_demo
void __fastcall TRACK_sv_demo();
unsigned int __fastcall SV_GetHistoryIndex(server_demo_history_t *history);
int __fastcall SV_GetBufferIndex(unsigned __int8 *ptr);
void __fastcall SV_HistoryFree(unsigned __int8 *ptr, int size);
int __fastcall SV_HistoryAlloc(server_demo_history_t *history, unsigned __int8 **pData, int size);
int __fastcall SV_MsgAlloc(unsigned int maxsize);
void SV_CheckDemoSize();
bool __fastcall SV_DemoWrite(const void *buffer, unsigned int len, _iobuf *file);
int __fastcall SV_FindTimeSkipIndex(int time);
FileMarkSkip *__fastcall SV_FindMarkSkip(const char *name);
void __fastcall SV_TruncateHistoryTimeCache(int maxTime);
int SV_ClearHistoryMarkCache();
// attributes: thunk
void __fastcall SV_TruncateHistoryCache(int maxTime);
int SV_SetAutoSaveHistoryTime();
void __fastcall SV_ResetDemo();
_iobuf *SV_ClearHistoryCache();
void __fastcall SV_FreeDemoSaveBuf(server_demo_save_t *save);
void __fastcall SV_FreeHistoryData(server_demo_history_t *history);
void __fastcall SV_FreeHistory(server_demo_history_t **history);
void SV_FreeDemoMsg();
int __fastcall SV_WaitForSaveHistoryDone();
void __fastcall SV_ShutdownDemo();
int __fastcall SV_AddDemoSave(SaveGame *savehandle, server_demo_save_t *save, int createSave);
_iobuf *__fastcall SV_DemoOpenFile(const char *fileName);
void __fastcall SV_InitWriteDemo(int randomSeed);
void __fastcall SV_InitReadDemoSavegame(SaveGame **saveHandle);
int __fastcall SV_InitDemoSavegame(SaveGame **save);
bool __fastcall SV_IsDemoPlaying();
bool __fastcall SV_UsingDemoSave();
void __fastcall SV_RecordClientCommand(const char *s);
void __fastcall SV_RecordClientThink(usercmd_s *cmd);
void __fastcall SV_RecordFxVisibility(double visibility);
void __fastcall SV_RecordCheatsOk(int cheatsOk);
void __fastcall SV_RecordIsRecentlyLoaded(bool isRecentlyLoaded);
void __fastcall SV_Record_Dvar_GetVariantString(const char *buffer);
void __fastcall SV_RecordButtonPressed(int buttonPressed);
void __fastcall SV_GetFreeDemoName(const char *baseName, int demoCount, char *testDemoName);
void __fastcall SV_SaveDemoImmediate(SaveImmediate *save);
void __fastcall SV_WriteDemo(SaveGame *save);
void __fastcall SV_SaveDemo(const char *demoName, const char *description, unsigned __int32 saveType);
void __fastcall SV_AutoSaveDemo(const char *baseName, const char *description, int demoCount, bool force);
void SV_EnableAutoDemo();
void __fastcall SV_SaveDemo_f();
void SV_DemoRestart();
void __fastcall SV_DemoRestart_f();
int __fastcall SV_DemoHasMark();
void __fastcall SV_LoadDemo(SaveGame *save, void *fileHandle);
bool __fastcall SV_RecordingDemo();
int __fastcall SV_Demo_Dvar_Set(const char *var_name, const char *value);
int __fastcall SV_WriteDemoSaveBuf(server_demo_save_t *save);
bool __fastcall SV_WriteHistory(_iobuf *fileHistory, const server_demo_history_t *history);
void __fastcall SV_SaveHistoryTime(server_demo_history_t *history);
void __fastcall SV_SaveHistoryMark(const server_demo_history_t *history);
void __fastcall SV_SaveHistory(server_demo_history_t *history);
void __fastcall  SV_SaveHistoryLoop(unsigned int threadContext);
bool SV_InitHistorySaveThread();
void __fastcall SV_InitDemoSystem();
server_demo_history_t *__fastcall SV_DemoGetFreeBuffer();
int __fastcall SV_HistoryIsNew(server_demo_history_t *history);
void __fastcall SV_ClearInfrequentTimeMarks(server_demo_history_t *history);
server_demo_history_t *__fastcall SV_DemoGetBuffer();
server_demo_history_t *__fastcall SV_GetMarkHistory(const char *name);
int __fastcall SV_DemoSaveHistory(server_demo_history_t *history);
void __fastcall SV_DemoMark_f();
bool __fastcall SV_DemoRead(void *buffer, unsigned int len, _iobuf *file);
int __fastcall SV_DemoAllocRead(
    server_demo_history_t *history,
    unsigned __int8 **buffer,
    unsigned int len,
    _iobuf *file);
bool __fastcall SV_ReadHistory(_iobuf *fileHistory, server_demo_history_t *history);
bool __fastcall SV_DemoLoadHistory(_iobuf *fileHistory, int fileOffset);
bool __fastcall SV_LoadHistoryForTime(int time);
bool __fastcall SV_ActiveHistoryIsMark(const char *name);
int __fastcall SV_LoadHistoryForMark(const char *name);
void __fastcall SV_DemoGoto_f();
void __fastcall SV_DemoSetNextLevelTime(int time);
void __fastcall SV_DemoBack_f();
void __fastcall SV_DemoForward_f();
void __fastcall SV_DemoFullForward_f();
void __fastcall SV_DemoLive_f();
void __fastcall SV_DemoInfo_f(int a1, int a2, int a3);
int __fastcall SV_GetDemoStartTime();
int __fastcall SV_GetDemoEndTime();
int __fastcall SV_CheckAutoSaveHistory(int setTooSoon);
void SV_DoAutoSaveHistory();
void __fastcall SV_UpdateDemo();
void SV_DemoLive();
void __fastcall SV_EndDemo(bool error);
void SV_ReadNextDemoType();
bool __fastcall SV_InitReadDemo(int *randomSeed);
bool __fastcall SV_InitDemo(int *randomSeed);
bool __fastcall SV_ReadPacket(int framePos);
float __fastcall SV_DemoFxVisibility();
int __fastcall SV_DemoCheatsOk();
bool __fastcall SV_DemoIsRecentlyLoaded();
char *__fastcall SV_Demo_Dvar_GetVariantString();
int __fastcall SV_DemoButtonPressed();


// sv_main
void __fastcall TRACK_sv_main();
char *__fastcall SV_ExpandNewlines(char *in);
void __fastcall SV_DumpServerCommands(client_t *client);
void __fastcall AppendCommandsForInternalSave(const char *filename);
void __fastcall SV_InitiatePendingSave(
    const char *filename,
    const char *description,
    const char *screenshot,
    SaveType saveType,
    unsigned int commitLevel,
    PendingSave *pendingSave,
    bool suppressPlayerNotify);
int __fastcall SV_AddPendingSave(
    const char *filename,
    const char *description,
    const char *screenshot,
    SaveType saveType,
    unsigned int commitLevel,
    bool suppressPlayerNotify);
int __fastcall SV_ProcessPendingSave(PendingSave *pendingSave);
int __fastcall SV_ProcessPendingSaves();
void __fastcall SV_ClearPendingSaves();
int __fastcall SV_IsInternalSave(const char *filename);
void __fastcall SV_SetLastSaveName(const char *filename);
void __fastcall SV_AddServerCommand(client_t *client, const char *cmd);
// local variable allocation has failed, the output may be wrong!
void SV_SendServerCommand(
    client_t *cl,
    const char *fmt,
    __int64 a3,
    __int64 a4,
    __int64 a5,
    int a6,
    int a7,
    int a8,
    int a9,
    int a10,
    int a11,
    ...);
void __fastcall SV_SaveServerCommands(SaveGame *save);
void __fastcall SV_LoadServerCommands(SaveGame *save);
void __fastcall SV_PreFrame();
int __fastcall SV_RunFrame(ServerFrameExtent extent, int timeCap);
void SV_ProcessPostFrame();
void __fastcall SV_UpdatePerformanceFrame(int time);
bool __fastcall SV_CheckSkipTimeout();
int __fastcall SV_CheckStartServer();
int __fastcall SV_WaitStartServer();
void __fastcall  SV_ServerThread(unsigned int threadContext);
void __fastcall SV_InitServerThread();
void __fastcall SV_ExitAfterTime();
void SV_WakeServer();
void __fastcall SV_WaitServer();
void __fastcall SV_InitSnapshot();
void __fastcall SV_WaitSaveGame();
void __fastcall SV_BeginSaveGame();
void __fastcall SV_EndSaveGame();
int __fastcall SV_WaitServerSnapshot();
bool __fastcall SV_ReachedServerCommandThreshold();
void __fastcall SV_FrameInternal(int msec);
int __fastcall SV_GetPartialFrametime();
int __fastcall SV_ForwardFrame();
int __fastcall SV_ClientFrameRateFix(int msec);
int __fastcall SV_Frame(int msec);
bool __fastcall SV_SaveMemory_IsRecentlyLoaded();


// sv_snapshot
void __fastcall SV_WriteSnapshotToClient(client_t *client, msg_t *msg);
void __fastcall SV_UpdateServerCommandsToClient(client_t *client);
void __fastcall SV_AddEntToSnapshot(int entnum);
void __fastcall SV_AddEntitiesVisibleFromPoint(int clientNum);
void __fastcall SV_BuildClientSnapshot(client_t *client);
void __fastcall SV_SendMessageToClient(msg_t *msg, client_t *client);
void __fastcall SV_BuildAndSendClientSnapshot(client_t *client);
void __fastcall SV_SendClientMessages();
void __fastcall SV_WriteSnapshotToClientCmd(void *cmdData);
void __fastcall SV_ArchiveSnapshotCmd(void *cmdData);
