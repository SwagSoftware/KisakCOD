#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

unsigned int __fastcall Com_BlockChecksum32(const void *buffer, unsigned int length);
void __fastcall TRACK_save_memory();
SaveGame *__fastcall SaveMemory_GetMemoryFile(SaveGame *save);
SaveGame *__fastcall SaveMemory_GetSaveHandle(unsigned int type);
void __fastcall SaveMemory_ClearSaveGame(SaveGame *saveGame, bool isUsingGlobalBuffer);
void *SaveMemory_ResetGameBuffers();
void __fastcall SaveMemory_InitializeSaveSystem();
void __fastcall SaveMemory_ShutdownSaveSystem();
void __fastcall SaveMemory_ClearDemoSave();
void __fastcall SaveMemory_AllocateTempMemory(SaveGame *save, int size, void *buffer);
void __fastcall SaveMemory_AllocateHeapMemory(SaveGame *save, unsigned int size);
void __fastcall SaveMemory_FreeMemory(SaveGame *save);
int __fastcall SaveMemory_GenerateSaveId();
// attributes: thunk
void __fastcall SaveMemory_StartSegment(SaveGame *save, int index);
// attributes: thunk
void __fastcall SaveMemory_MoveToSegment(SaveGame *save, int index);
void __fastcall SaveMemory_InitializeGameSave(SaveGame *save);
void __fastcall SaveMemory_InitializeDemoSave(SaveGame *save);
void __fastcall SaveMemory_FinalizeSave(SaveGame *save);
void __fastcall SaveMemory_InitializeLoad(SaveGame *save, int size);
void __fastcall SaveMemory_FinalizeLoad(SaveGame *save);
void __fastcall SaveMemory_FinalizeSaveCommit(SaveGame *save);
bool __fastcall SaveMemory_IsSuccessful(SaveGame *save);
int __fastcall SaveMemory_IsSaving(SaveGame *save);
bool __fastcall SaveMemory_IsLoading(SaveGame *save);
bool __fastcall SaveMemory_IsWaitingForCommit(SaveGame *save);
unsigned __int8 *__fastcall SaveMemory_GetBodyBuffer(SaveGame *save);
unsigned int __fastcall SaveMemory_CalculateChecksum(SaveGame *save);
void __fastcall SaveMemory_InitializeLoadFromBuffer(SaveGame *save, unsigned __int8 *buffer, int length);
void __fastcall SaveMemory_SaveWrite(const void *buffer, int len, SaveGame *save);
void __fastcall SaveMemory_SetBuffer(void *buffer, int len, SaveGame *save);
void __fastcall SaveMemory_LoadRead(void *buffer, int size, SaveGame *save);
int __fastcall SaveMemory_GetTotalLoadSize(SaveGame *save);
// local variable allocation has failed, the output may be wrong!
void __fastcall SaveMemory_CreateHeader(
    const char *cleanUserName,
    const char *description,
    const char *screenshot,
    int checksum,
    bool demoPlayback,
    bool suppressPlayerNotify,
    unsigned int saveType,
    int saveId,
    SaveGame *save,
    int a10,
    int a11,
    int a12,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28);
const SaveHeader *__fastcall SaveMemory_GetHeader(SaveGame *save);
void *__fastcall SaveMemory_ReadLoadFromDevice(
    const char *filename,
    int checksum,
    int useLoadedSourceFiles,
    SaveGame **save);
bool __fastcall SaveMemory_IsRecentlyLoaded();
int __fastcall SaveMemory_IsCommittedSaveAvailable(const char *filename, int checksum);
bool __fastcall SaveMemory_IsCurrentCommittedSaveValid();
int __fastcall SaveMemory_CommitSave(SaveGame *save, int saveId);
void __fastcall SaveMemory_RollbackSave(SaveGame *save);
SaveGame *__fastcall SaveMemory_GetLastCommittedSave();
int __fastcall SaveMemory_WriteSaveToDevice(SaveGame *save);
bool __fastcall SaveMemory_IsWrittenToDevice(SaveGame *save);
int __fastcall SaveMemory_ForceCommitSave(SaveGame *save);
bool __fastcall SaveMemory_IsCommitForced();
void __fastcall SaveMemory_ClearForcedCommitFlag();
void __fastcall SaveMemory_FinalizeSaveToDisk(SaveGame *save);
void __fastcall SaveMemory_CleanupSaveMemory();
