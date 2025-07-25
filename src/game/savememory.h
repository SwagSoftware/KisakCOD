#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_save.h"

unsigned int __cdecl Com_BlockChecksum32(const void *buffer, unsigned int length);
void __cdecl TRACK_save_memory();
SaveGame *__cdecl SaveMemory_GetMemoryFile(SaveGame *save);
SaveGame *__cdecl SaveMemory_GetSaveHandle(unsigned int type);
void __cdecl SaveMemory_ClearSaveGame(SaveGame *saveGame, bool isUsingGlobalBuffer);
void *SaveMemory_ResetGameBuffers();
void __cdecl SaveMemory_InitializeSaveSystem();
void __cdecl SaveMemory_ShutdownSaveSystem();
void __cdecl SaveMemory_ClearDemoSave();
void __cdecl SaveMemory_AllocateTempMemory(SaveGame *save, int size, void *buffer);
void __cdecl SaveMemory_AllocateHeapMemory(SaveGame *save, unsigned int size);
void __cdecl SaveMemory_FreeMemory(SaveGame *save);
int __cdecl SaveMemory_GenerateSaveId();
// attributes: thunk
void __cdecl SaveMemory_StartSegment(SaveGame *save, int index);
// attributes: thunk
void __cdecl SaveMemory_MoveToSegment(SaveGame *save, int index);
void __cdecl SaveMemory_InitializeGameSave(SaveGame *save);
void __cdecl SaveMemory_InitializeDemoSave(SaveGame *save);
void __cdecl SaveMemory_FinalizeSave(SaveGame *save);
void __cdecl SaveMemory_InitializeLoad(SaveGame *save, int size);
void __cdecl SaveMemory_FinalizeLoad(SaveGame *save);
void __cdecl SaveMemory_FinalizeSaveCommit(SaveGame *save);
bool __cdecl SaveMemory_IsSuccessful(SaveGame *save);
int __cdecl SaveMemory_IsSaving(SaveGame *save);
bool __cdecl SaveMemory_IsLoading(SaveGame *save);
bool __cdecl SaveMemory_IsWaitingForCommit(SaveGame *save);
unsigned __int8 *__cdecl SaveMemory_GetBodyBuffer(SaveGame *save);
unsigned int __cdecl SaveMemory_CalculateChecksum(SaveGame *save);
void __cdecl SaveMemory_InitializeLoadFromBuffer(SaveGame *save, unsigned __int8 *buffer, int length);
void __cdecl SaveMemory_SaveWrite(const void *buffer, int len, SaveGame *save);
void __cdecl SaveMemory_SetBuffer(void *buffer, int len, SaveGame *save);
void __cdecl SaveMemory_LoadRead(void *buffer, int size, SaveGame *save);
int __cdecl SaveMemory_GetTotalLoadSize(SaveGame *save);
// local variable allocation has failed, the output may be wrong!
void __cdecl SaveMemory_CreateHeader(
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
const SaveHeader *__cdecl SaveMemory_GetHeader(SaveGame *save);
void *__cdecl SaveMemory_ReadLoadFromDevice(
    const char *filename,
    int checksum,
    int useLoadedSourceFiles,
    SaveGame **save);
bool __cdecl SaveMemory_IsRecentlyLoaded();
int __cdecl SaveMemory_IsCommittedSaveAvailable(const char *filename, int checksum);
bool __cdecl SaveMemory_IsCurrentCommittedSaveValid();
int __cdecl SaveMemory_CommitSave(SaveGame *save, int saveId);
void __cdecl SaveMemory_RollbackSave(SaveGame *save);
SaveGame *__cdecl SaveMemory_GetLastCommittedSave();
int __cdecl SaveMemory_WriteSaveToDevice(SaveGame *save);
bool __cdecl SaveMemory_IsWrittenToDevice(SaveGame *save);
int __cdecl SaveMemory_ForceCommitSave(SaveGame *save);
bool __cdecl SaveMemory_IsCommitForced();
void __cdecl SaveMemory_ClearForcedCommitFlag();
void __cdecl SaveMemory_FinalizeSaveToDisk(SaveGame *save);
void __cdecl SaveMemory_CleanupSaveMemory();
