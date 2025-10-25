#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "savememory.h"
#include <qcommon/com_bsp.h>
#include <buildnumber.h>
#include "g_main.h"
#include <bgame/bg_public.h>
#include <server/server.h>
#include <server/sv_game.h>

struct __declspec(align(4)) SaveMemoryGlob
{
    SaveGame *committedGameSave;
    SaveGame *currentGameSave;
    SaveGame game0;
    SaveGame game1;
    SaveGame demo;
    unsigned __int8 buffer0[1572864];
    unsigned __int8 buffer1[1572864];
    unsigned __int8 buffer2[1572864];
    int recentLoadTime;
    bool isCommitForced;
};

SaveMemoryGlob saveMemoryGlob;
int g_saveId;

unsigned int __cdecl Com_BlockChecksum32(const void *buffer, unsigned int length)
{
    return Com_BlockChecksumKey32((const unsigned char*)buffer, length, 0);
}

void __cdecl TRACK_save_memory()
{
    track_static_alloc_internal(&saveMemoryGlob, 4722040, "saveMemoryGlob", 10);
}

MemoryFile * SaveMemory_GetMemoryFile(SaveGame *save)
{
    iassert(save);
    iassert(!save->isDirectWriteActive);

    return (MemoryFile*)save;
}

SaveGame *__cdecl SaveMemory_GetSaveHandle(unsigned int type)
{
    if (!type)
        return saveMemoryGlob.currentGameSave;
    if (type == 1)
        return &saveMemoryGlob.demo;
    if (type < 3)
        return saveMemoryGlob.committedGameSave;
    if (!alwaysfails)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 174, 0, "unreachable");
    return 0;
}

void __cdecl SaveMemory_ClearSaveGame(SaveGame *saveGame, bool isUsingGlobalBuffer)
{
    memset(saveGame, 0, sizeof(SaveGame));
    saveGame->isUsingGlobalBuffer = isUsingGlobalBuffer;
}

void *SaveMemory_ResetGameBuffers()
{
    void *result; // r3

    memset(&saveMemoryGlob.game0, 0, sizeof(saveMemoryGlob.game0));
    saveMemoryGlob.game0.isUsingGlobalBuffer = 1;
    result = memset(&saveMemoryGlob.game1, 0, sizeof(saveMemoryGlob.game1));
    saveMemoryGlob.game1.isUsingGlobalBuffer = 1;
    saveMemoryGlob.game0.memFile.buffer = saveMemoryGlob.buffer0;
    saveMemoryGlob.game1.memFile.buffer = saveMemoryGlob.buffer1;
    saveMemoryGlob.game0.memFile.bufferSize = 1572864;
    saveMemoryGlob.game1.memFile.bufferSize = 1572864;
    saveMemoryGlob.committedGameSave = &saveMemoryGlob.game0;
    saveMemoryGlob.currentGameSave = &saveMemoryGlob.game1;
    return result;
}

void __cdecl SaveMemory_InitializeSaveSystem()
{
    if (saveMemoryGlob.committedGameSave)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp",
            226,
            0,
            "%s",
            "!saveMemoryGlob.committedGameSave");
    if (saveMemoryGlob.currentGameSave)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp",
            227,
            0,
            "%s",
            "!saveMemoryGlob.currentGameSave");
    memset(&saveMemoryGlob.game0, 0, sizeof(saveMemoryGlob.game0));
    saveMemoryGlob.game0.isUsingGlobalBuffer = 1;
    memset(&saveMemoryGlob.game1, 0, sizeof(saveMemoryGlob.game1));
    saveMemoryGlob.game1.isUsingGlobalBuffer = 1;
    saveMemoryGlob.game0.memFile.buffer = saveMemoryGlob.buffer0;
    saveMemoryGlob.game1.memFile.buffer = saveMemoryGlob.buffer1;
    saveMemoryGlob.game0.memFile.bufferSize = 1572864;
    saveMemoryGlob.game1.memFile.bufferSize = 1572864;
    saveMemoryGlob.committedGameSave = &saveMemoryGlob.game0;
    saveMemoryGlob.currentGameSave = &saveMemoryGlob.game1;
    saveMemoryGlob.recentLoadTime = 0;
}

void __cdecl SaveMemory_ShutdownSaveSystem()
{
    saveMemoryGlob.committedGameSave = 0;
    saveMemoryGlob.currentGameSave = 0;
}

void __cdecl SaveMemory_ClearDemoSave()
{
    memset(&saveMemoryGlob.demo, 0, sizeof(saveMemoryGlob.demo));
    saveMemoryGlob.demo.isUsingGlobalBuffer = 0;
}

void __cdecl SaveMemory_AllocateTempMemory(SaveGame *save, int size, void *buffer)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 256, 0, "%s", "save");
    if (save->saveState)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 257, 0, "%s", "save->saveState == MEMCLEAR");
    save->saveState = SAVING;
    MemFile_InitForWriting(&save->memFile, size, (byte*)buffer, 0, 0);
    if (!save->memFile.buffer)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 262, 0, "%s", "save->memFile.buffer");
}

void __cdecl SaveMemory_AllocateHeapMemory(SaveGame *save, unsigned int size)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 273, 0, "%s", "save");
    if (save->isUsingGlobalBuffer)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 274, 0, "%s", "!save->isUsingGlobalBuffer");
    if (size >= 0x180000)
        Com_Error(ERR_DROP, "SaveMemory_AllocateHeapMemory: failed to alloc %d bytes", size);
    save->memFile.buffer = saveMemoryGlob.buffer2;
}

void __cdecl SaveMemory_FreeMemory(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 292, 0, "%s", "save");
    if (save->isUsingGlobalBuffer)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 293, 0, "%s", "!save->isUsingGlobalBuffer");
    if (save->memFile.buffer)
    {
        MemFile_Shutdown(&save->memFile);
        save->saveState = MEMCLEAR;
    }
}

int __cdecl SaveMemory_GenerateSaveId()
{
    return ++g_saveId;
}

// attributes: thunk
void __cdecl SaveMemory_StartSegment(SaveGame *save, int index)
{
    MemFile_StartSegment(&save->memFile, index);
}

// attributes: thunk
void __cdecl SaveMemory_MoveToSegment(SaveGame *save, int index)
{
    MemFile_MoveToSegment(&save->memFile, index);
}

void __cdecl SaveMemory_InitializeGameSave(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 336, 0, "%s", "save");
    if (!save->isUsingGlobalBuffer)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 337, 0, "%s", "save->isUsingGlobalBuffer");
    MemFile_InitForWriting(&save->memFile, 1572864, save->memFile.buffer, 1, 0);
    save->saveState = SAVING;
    saveMemoryGlob.recentLoadTime = 0;
}

void __cdecl SaveMemory_InitializeDemoSave(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 359, 0, "%s", "save");
    if (save->isUsingGlobalBuffer)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 360, 0, "%s", "!save->isUsingGlobalBuffer");
    SaveMemory_FreeMemory(save);
    SaveMemory_AllocateTempMemory(save, 1572864, saveMemoryGlob.buffer2);
    if (save->saveState != SAVING)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 363, 1, "%s", "save->saveState == SAVING");
}

void __cdecl SaveMemory_FinalizeSave(SaveGame *save)
{
    bool isUsingGlobalBuffer; // r10

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 375, 0, "%s", "save");
    if (save->saveState != SAVING)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 376, 0, "%s", "save->saveState == SAVING");
    isUsingGlobalBuffer = save->isUsingGlobalBuffer;
    save->saveState = AWAITING_COMMIT;
    if (!isUsingGlobalBuffer)
        SaveMemory_FreeMemory(save);
}

void __cdecl SaveMemory_InitializeLoad(SaveGame *save, int size)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 403, 0, "%s", "save");
    MemFile_InitForReading(&save->memFile, size, save->memFile.buffer, 0);
    save->saveState = LOADING;
}

void __cdecl SaveMemory_FinalizeLoad(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 418, 0, "%s", "save");
    if (save->saveState != LOADING)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 419, 0, "%s", "save->saveState == LOADING");
    if (save->isUsingGlobalBuffer)
    {
        save->saveState = COMMITTED;
        saveMemoryGlob.recentLoadTime = Sys_Milliseconds();
    }
    else
    {
        SaveMemory_FreeMemory(save);
    }
}

void __cdecl SaveMemory_FinalizeSaveCommit(SaveGame *save)
{
    SaveBufferState saveState; // r11

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 437, 0, "%s", "save");
    saveState = save->saveState;
    if (saveState != AWAITING_COMMIT && saveState != COMMITTED)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp",
            438,
            0,
            "%s",
            "save->saveState == AWAITING_COMMIT || save->saveState == COMMITTED");
    save->saveState = COMMITTED;
}

bool __cdecl SaveMemory_IsSuccessful(SaveGame *save)
{
    return !save->memFile.memoryOverflow;
}

int __cdecl SaveMemory_IsSaving(SaveGame *save)
{
    SaveBufferState saveState; // r11
    unsigned __int8 v3; // r11
    bool v4; // zf

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 473, 0, "%s", "save");
    saveState = save->saveState;
    if (saveState == SAVING)
        return 1;
    v4 = saveState != AWAITING_COMMIT;
    v3 = 0;
    if (!v4)
        return 1;
    return v3;
}

bool __cdecl SaveMemory_IsLoading(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 485, 0, "%s", "save");
    return save->saveState == LOADING;
}

bool __cdecl SaveMemory_IsWaitingForCommit(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 497, 0, "%s", "save");
    return save->saveState == AWAITING_COMMIT;
}

unsigned __int8 *__cdecl SaveMemory_GetBodyBuffer(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 511, 0, "%s", "save");
    return save->memFile.buffer;
}

unsigned int __cdecl SaveMemory_CalculateChecksum(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 521, 0, "%s", "save");
    if (!save->memFile.buffer)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 522, 0, "%s", "save->memFile.buffer");
    return Com_BlockChecksumKey32(save->memFile.buffer, save->header.bodySize, 0);
}

void __cdecl SaveMemory_InitializeLoadFromBuffer(SaveGame *save, unsigned __int8 *buffer, int length)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 534, 0, "%s", "save");
    MemFile_InitForReading(&save->memFile, length, buffer, 0);
    save->header.bodySize = length;
    save->header.saveCheckSum = SaveMemory_CalculateChecksum(save);
    save->saveState = LOADING;
}

void __cdecl SaveMemory_SaveWrite(const void *buffer, int len, SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 551, 0, "%s", "save");
    if (save->saveState != SAVING)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 552, 0, "%s", "save->saveState == SAVING");
    if (save->isDirectWriteActive)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp",
            553,
            0,
            "%s",
            "save->isDirectWriteActive == false");
    MemFile_WriteData(&save->memFile, len, buffer);
}

void __cdecl SaveMemory_SetBuffer(void *buffer, int len, SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 567, 0, "%s", "save");
    if (save->isUsingGlobalBuffer)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 568, 0, "%s", "!save->isUsingGlobalBuffer");
    SaveMemory_FreeMemory(save);
    if (save->saveState)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 572, 0, "%s", "save->saveState == MEMCLEAR");
    if (save->isDirectWriteActive)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp",
            573,
            0,
            "%s",
            "save->isDirectWriteActive == false");
    save->saveState = SAVING;
    MemFile_CommonInit(&save->memFile, len, (byte*)buffer, 0, 1);
}

void __cdecl SaveMemory_LoadRead(void *buffer, int size, SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 590, 0, "%s", "save");
    if (save->saveState != LOADING)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 591, 0, "%s", "save->saveState == LOADING");
    MemFile_ReadData(&save->memFile, size, (byte *)buffer);
}

int __cdecl SaveMemory_GetTotalLoadSize(SaveGame *save)
{
    SaveBufferState saveState; // r8

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 604, 0, "%s", "save");
    saveState = save->saveState;
    if (saveState != LOADING)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp",
            605,
            0,
            "%s\n\t(save->saveState) = %i",
            "(save->saveState == LOADING)",
            saveState);
    return save->memFile.bufferSize;
}

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
    SaveGame *save)
{
    const dvar_s *v36; // r3
    const char *BuildNumber; // r3
    unsigned int bytesUsed; // r4
    unsigned __int8 *buffer; // r3
    const dvar_s *v40; // r3
    __int64 v41; // r11 OVERLAPPED
    _DWORD *v42; // r9
    int v43; // r11
    _DWORD v44[2]; // [sp+50h] [-70h] BYREF
    __int64 v45; // [sp+58h] [-68h]

    iassert(cleanUserName);
    iassert(save);
    iassert(save->saveState == SAVING);

    memset(&save->header, 0, sizeof(save->header));
    save->suppressPlayerNotify = suppressPlayerNotify;
    save->header.demoPlayback = demoPlayback;
    save->header.saveVersion = 287;
    //save->header.internalSave = (_cntlzw(saveType) & 0x20) != 0;
    save->header.internalSave = saveType != 0;
    v36 = Dvar_RegisterString("mapname", "", 0x44u, "The current map name");
    save->header.saveId = saveId;
    I_strncpyz(save->header.mapName, v36->current.string, 256);
    BuildNumber = getBuildNumber();
    I_strncpyz(save->header.buildNumber, BuildNumber, 128);
    bytesUsed = save->memFile.bytesUsed;
    buffer = save->memFile.buffer;
    save->header.gameCheckSum = checksum;
    save->header.saveCheckSum = Com_BlockChecksumKey32(buffer, bytesUsed, 0);
    Scr_GetChecksum((unsigned int*)save->header.scrCheckSum);
    v40 = Dvar_RegisterString("ui_campaign", "american", 0x1000u, "The current campaign");
    I_strncpyz(save->header.campaign, v40->current.string, 256);
    if (screenshot)
        I_strncpyz(save->header.screenShotName, screenshot, 64);
    else
        save->header.screenShotName[0] = 0;
    I_strncpyz(save->header.filename, cleanUserName, 64);
    if (description)
        I_strncpyz(save->header.description, description, 256);
    else
        save->header.description[0] = 0;
    if (g_entities[0].health
        && (LODWORD(v41) = g_entities[0].client->pers.maxHealth, (_DWORD)v41)
        && (HIDWORD(v41) = 100 * g_entities[0].health,
            v45 = v41,
            v42 = v44,
            v44[1] = 100 * g_entities[0].health,
            v44[0] = (int)(float)((float)*(__int64 *)((char *)&v41 + 4) / (float)v41),
            v43 = v44[0],
            v44[0] >= 1))
    {
        if (v44[0] > 100)
            v43 = 100;
    }
    else
    {
        v43 = 1;
    }
    save->header.health = v43;
    save->header.skill = sv_gameskill->current.integer;
    Com_RealTime(&save->header.time);
    save->header.bodySize = (int32_t)MemFile_CopySegments(&save->memFile, 0, 0);
    save->isWrittenToDevice = 0;
}

const SaveHeader *__cdecl SaveMemory_GetHeader(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 693, 0, "%s", "save");
    return &save->header;
}

void *__cdecl SaveMemory_ReadLoadFromDevice(
    const char *filename,
    int checksum,
    int useLoadedSourceFiles,
    SaveGame **save)
{
    // KISAKSAVE
#if 0
    __int64 v7; // r10
    __int64 v8; // r8
    __int64 v9; // r6
    __int64 v10; // r10
    __int64 v11; // r8
    __int64 v12; // r8
    __int64 v13; // r10
    __int64 v14; // r6
    __int64 v15; // r8
    __int64 v16; // r10
    __int64 v17; // r8
    __int64 v18; // r10
    int v19; // r3
    __int64 v20; // r6
    __int64 v21; // r10
    __int64 v22; // r8
    const dvar_s *v23; // r3
    __int64 v24; // r6
    __int64 v25; // r10
    __int64 v26; // r8
    int v28; // [sp+8h] [-4E8h]
    int v29; // [sp+Ch] [-4E4h]
    int v30; // [sp+10h] [-4E0h]
    int v31; // [sp+14h] [-4DCh]
    int v32; // [sp+18h] [-4D8h]
    int v33; // [sp+1Ch] [-4D4h]
    int v34; // [sp+20h] [-4D0h]
    int v35; // [sp+24h] [-4CCh]
    void *v36; // [sp+50h] [-4A0h] BYREF
    unsigned int v37[292]; // [sp+60h] [-490h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 706, 0, "%s", "save");
    memset(&saveMemoryGlob.game0, 0, sizeof(saveMemoryGlob.game0));
    saveMemoryGlob.game0.isUsingGlobalBuffer = 1;
    memset(&saveMemoryGlob.game1, 0, sizeof(saveMemoryGlob.game1));
    saveMemoryGlob.game1.isUsingGlobalBuffer = 1;
    saveMemoryGlob.game0.memFile.buffer = saveMemoryGlob.buffer0;
    saveMemoryGlob.game1.memFile.buffer = saveMemoryGlob.buffer1;
    saveMemoryGlob.game0.memFile.bufferSize = 1572864;
    saveMemoryGlob.game1.memFile.bufferSize = 1572864;
    saveMemoryGlob.committedGameSave = &saveMemoryGlob.game0;
    saveMemoryGlob.currentGameSave = &saveMemoryGlob.game1;
    HIBYTE(v37[4]) = 1;
    if (OpenDevice(filename, &v36) < 0)
    {
        CloseDevice(v36);
        G_SaveError(
            ERR_DROP,
            SAVE_ERROR_MISSING_DEVICE,
            __SPAIR64__(&unk_82042DB4, (unsigned int)filename),
            v8,
            v7,
            v28,
            v29,
            v30,
            v31,
            v32,
            v33,
            v34,
            v35);
    }
    if (ReadFromDevice(v37, 1108, v36) != 1108)
    {
        CloseDevice(v36);
        HIDWORD(v9) = &unk_82042D9C;
        G_SaveError(ERR_DROP, SAVE_ERROR_MISSING_DEVICE, v9, v11, v10, v28, v29, v30, v31, v32, v33, v34, v35);
    }
    if (v37[0] != 287)
    {
        CloseDevice(v36);
        HIDWORD(v12) = v37[0];
        HIDWORD(v13) = &v37[72];
        HIDWORD(v14) = &unk_82042D58;
        LODWORD(v12) = 287;
        LODWORD(v14) = &v37[248];
        G_SaveError(ERR_DROP, SAVE_ERROR_CORRUPT_SAVE, v14, v12, v13, v28, v29, v30, v31, v32, v33, v34, v35);
    }
    if (v37[276] > 1572864)
    {
        CloseDevice(v36);
        HIDWORD(v15) = v37[276];
        LODWORD(v15) = 1572864;
        G_SaveError(
            ERR_DROP,
            SAVE_ERROR_CORRUPT_SAVE,
            __SPAIR64__(&unk_82042CF0, (unsigned int)filename),
            v15,
            v16,
            v28,
            v29,
            v30,
            v31,
            v32,
            v33,
            v34,
            v35);
    }
    if (BYTE1(v37[266]))
    {
        *save = &saveMemoryGlob.demo;
        SaveMemory_FreeMemory(&saveMemoryGlob.demo);
        SaveMemory_AllocateHeapMemory(*save, v37[276]);
    }
    else
    {
        if (v37[276] > 1572864)
        {
            CloseDevice(v36);
            HIDWORD(v17) = v37[276];
            LODWORD(v17) = 1572864;
            G_SaveError(
                ERR_DROP,
                SAVE_ERROR_CORRUPT_SAVE,
                __SPAIR64__(&unk_82042C78, (unsigned int)filename),
                v17,
                v18,
                v28,
                v29,
                v30,
                v31,
                v32,
                v33,
                v34,
                v35);
        }
        *save = saveMemoryGlob.committedGameSave;
    }
    v19 = ReadFromDevice((*save)->memFile.buffer, v37[276], v36);
    if (v19 != v37[276])
    {
        CloseDevice(v36);
        HIDWORD(v20) = &unk_82042D9C;
        G_SaveError(ERR_DROP, SAVE_ERROR_MISSING_DEVICE, v20, v22, v21, v28, v29, v30, v31, v32, v33, v34, v35);
    }
    v23 = Dvar_RegisterString("ui_campaign", "american", 0x1000u, "The current campaign");
    Dvar_SetString(v23, (const char *)&v37[104]);
    Com_Printf(10, "Save game build number: %s\n", (const char *)&v37[72]);
    if (v37[1] != checksum)
    {
        CloseDevice(v36);
        LODWORD(v24) = &v37[72];
        HIDWORD(v24) = &unk_82042C18;
        G_SaveError(ERR_DROP, SAVE_ERROR_CORRUPT_SAVE, v24, v26, v25, v28, v29, v30, v31, v32, v33, v34, v35);
    }
    (*save)->isWrittenToDevice = 0;
    memcpy(&(*save)->header, v37, sizeof((*save)->header));
    return v36;
#else
    return NULL;
#endif
}

bool __cdecl SaveMemory_IsRecentlyLoaded()
{
    return (unsigned int)(Sys_Milliseconds() - saveMemoryGlob.recentLoadTime) < 0x7D0;
}

int __cdecl SaveMemory_IsCommittedSaveAvailable(const char *filename, int checksum)
{
    const SaveHeader *Header; // r3
    const SaveHeader *v5; // r31
    const char *v6; // r11
    int result; // r3

    if (g_useDevSaveArea)
        return 0;
    if (!saveMemoryGlob.committedGameSave)
        return 0;
    Header = SaveMemory_GetHeader(saveMemoryGlob.committedGameSave);
    v5 = Header;
    if (!Header->filename[0])
        return 0;
    v6 = Header->filename;
    while (*v6++)
        ;
    if (strncmp(filename, Header->filename, v6 - Header->filename - 1))
        return 0;
    result = 1;
    if (checksum != v5->gameCheckSum)
        return 0;
    return result;
}

bool __cdecl SaveMemory_IsCurrentCommittedSaveValid()
{
    return saveMemoryGlob.committedGameSave && saveMemoryGlob.committedGameSave->saveState == COMMITTED;
}

int __cdecl SaveMemory_CommitSave(SaveGame *save, int saveId)
{
    SaveGame *committedGameSave; // r10

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 497, 0, "%s", "save");
    if (save->saveState == AWAITING_COMMIT)
    {
        if (!save->isUsingGlobalBuffer)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 833, 0, "%s", "save->isUsingGlobalBuffer");
        if (!save->memFile.buffer)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 834, 0, "%s", "save->memFile.buffer");
        if (save != saveMemoryGlob.currentGameSave)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp",
                835,
                0,
                "%s",
                "save == saveMemoryGlob.currentGameSave");
        if (!saveMemoryGlob.committedGameSave)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp",
                836,
                0,
                "%s",
                "saveMemoryGlob.committedGameSave");
        if (!saveMemoryGlob.currentGameSave)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp",
                837,
                0,
                "%s",
                "saveMemoryGlob.currentGameSave");
        committedGameSave = saveMemoryGlob.committedGameSave;
        saveMemoryGlob.committedGameSave = saveMemoryGlob.currentGameSave;
        saveMemoryGlob.currentGameSave = committedGameSave;
        saveMemoryGlob.committedGameSave->saveState = COMMITTED;
        if (save == (SaveGame *)-1024)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 846, 0, "%s", "save->header.filename");
        SV_SetLastSaveName(save->header.filename);
        if (!save->suppressPlayerNotify)
            SV_GameSendServerCommand(-1, "reachedcheckpoint");
        return 1;
    }
    else
    {
        Com_Printf(10, "Attempting to commit an invalid save buffer\n");
        return 0;
    }
}

void __cdecl SaveMemory_RollbackSave(SaveGame *save)
{
    if (!save)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 860, 0, "%s", "save");
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 497, 0, "%s", "save");
    }
    if (save->saveState == AWAITING_COMMIT)
    {
        if (!save->isUsingGlobalBuffer)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 865, 0, "%s", "save->isUsingGlobalBuffer");
        if (!save->memFile.buffer)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 866, 0, "%s", "save->memFile.buffer");
        if (save != saveMemoryGlob.currentGameSave)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp",
                867,
                0,
                "%s",
                "save == saveMemoryGlob.currentGameSave");
        save->saveState = MEMCLEAR;
    }
}

SaveGame *__cdecl SaveMemory_GetLastCommittedSave()
{
    return saveMemoryGlob.committedGameSave;
}

int __cdecl SaveMemory_WriteSaveToDevice(SaveGame *save)
{
    SaveBufferState saveState; // r11
    int v4; // r29
    int v5; // r30
    int v6; // r3

    iassert(save);
    saveState = save->saveState;
    if (saveState == AWAITING_COMMIT || saveState == COMMITTED)
    {
        if (save->memFile.bufferSize != save->header.bodySize)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp",
                894,
                0,
                "%s",
                "save->memFile.bufferSize == saveHeader->bodySize");
        if (!save->memFile.buffer)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 896, 0, "%s", "save->memFile.buffer");
        if (save->isWrittenToDevice)
            Com_Printf(10, "** Save was already written to a device; it is being saved again.  This is not an error.\n");
        v4 = Sys_Milliseconds();
        //v5 = WriteSaveToDevice(save->memFile.buffer, &save->header, save->suppressPlayerNotify); // KISAKSAVE
        v6 = Sys_Milliseconds();
        Com_Printf(10, "time to write: %i  ms\n", v6 - v4);
        //if (!v5)
        //    save->isWrittenToDevice = 1;
        SaveMemory_FinalizeSaveCommit(save);
        //return v5;
        return 0;
    }
    else
    {
        Com_Printf(10, "Attempting to write an invalid save buffer\n");
        return 0;
    }
}

bool __cdecl SaveMemory_IsWrittenToDevice(SaveGame *save)
{
    return save->isWrittenToDevice;
}

int __cdecl SaveMemory_ForceCommitSave(SaveGame *save)
{
    saveMemoryGlob.isCommitForced = 1;
    return SaveMemory_CommitSave(save, save->header.saveId);
}

bool __cdecl SaveMemory_IsCommitForced()
{
    return saveMemoryGlob.isCommitForced;
}

void __cdecl SaveMemory_ClearForcedCommitFlag()
{
    saveMemoryGlob.isCommitForced = 0;
}

void __cdecl SaveMemory_FinalizeSaveToDisk(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 386, 0, "%s", "save");
    if (save->saveState != SAVING)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\savememory.cpp", 387, 0, "%s", "save->saveState == SAVING");
    save->saveState = AWAITING_COMMIT;
    SaveMemory_WriteSaveToDevice(save);
    if (!save->isUsingGlobalBuffer)
        SaveMemory_FreeMemory(save);
}

void __cdecl SaveMemory_CleanupSaveMemory()
{
    if (saveMemoryGlob.currentGameSave)
        SaveMemory_RollbackSave(saveMemoryGlob.currentGameSave);
    if (saveMemoryGlob.demo.saveState)
        SaveMemory_FreeMemory(&saveMemoryGlob.demo);
}

