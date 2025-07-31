#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "server.h"
#include <qcommon/com_fileaccess.h>
#include <game/g_local.h>
#include <qcommon/threads.h>
#include <game/savememory.h>
#include <win32/win_local.h>
#include <universal/com_files.h>
#include <client/cl_demo.h>
#include <game/g_save.h>
#include <qcommon/msg.h>
#include <qcommon/com_playerprofile.h>
#include <qcommon/cmd.h>
#include <client/cl_input.h>

unsigned __int8 g_buf[2][3145728];
unsigned __int8 g_msgBuf[10485760];
FileSkip g_fileSkips[3600]{ 0 };
FileMarkSkip g_fileMarkSkips[50];
server_demo_history_t g_historyBuffers[2];
int g_bufSize[2];
int g_numFileSkips;
int g_numFileMarkSkips;
FILE *g_fileMarkHistory;
FILE *g_fileTimeHistory;
server_demo_history_t *g_history;
bool g_savingHistory;
server_demo_history_t *volatile g_historySaving;

void __cdecl TRACK_sv_demo()
{
    track_static_alloc_internal(g_buf, 6291456, "g_buf", 0);
    track_static_alloc_internal(g_msgBuf, 10485760, "g_msgBuf", 0);
    track_static_alloc_internal(g_fileSkips, 28800, "g_fileSkips", 0);
    track_static_alloc_internal(g_fileMarkSkips, 3400, "g_fileMarkSkips", 0);
    track_static_alloc_internal(g_historyBuffers, 344, "g_historyBuffers", 0);
}

unsigned int __cdecl SV_GetHistoryIndex(server_demo_history_t *history)
{
    if (history == &g_historyBuffers[1])
        return 1;

    iassert(history == &g_historyBuffers[0]);
    return 0;
}

int __cdecl SV_GetBufferIndex(unsigned __int8 *ptr)
{
    int v1; // r8
    int v2; // r10
    unsigned __int8 *v3; // r11
    const char *v4; // r3

    v1 = 0;
    v2 = 0;
    v3 = g_buf[1];
    do
    {
        if (ptr >= v3 - 3145728 && ptr < v3)
            return v1;
        v2 += 3145728;
        ++v1;
        v3 += 3145728;
    } while (v2 < 6291456);
    if (!alwaysfails)
    {
        v4 = va("SV_GetBufferIndex: ptr 0x%x isn't in either of the history buffers.\n", ptr);
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 178, 0, v4);
    }
    return 0;
}

void __cdecl SV_HistoryFree(unsigned __int8 *ptr, int size)
{
    int BufferIndex; // r3
    int v5; // r10

    BufferIndex = SV_GetBufferIndex(ptr);
    v5 = g_bufSize[BufferIndex] - size;
    g_bufSize[BufferIndex] = v5;
    if (ptr != &g_buf[BufferIndex][v5])
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp",
            190,
            0,
            "%s",
            "ptr == g_buf[bufferIndex] + g_bufSize[bufferIndex]");
}

int __cdecl SV_HistoryAlloc(server_demo_history_t *history, unsigned __int8 **pData, int size)
{
    unsigned int HistoryIndex; // r3
    unsigned int v7; // r28
    int v8; // r11
    int v9; // r31
    unsigned __int8 *v10; // r3
    int result; // r3

    if (size <= 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp",
            199,
            0,
            "%s\n\t(size) = %i",
            "(size > 0)",
            size);
    if (!history)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 200, 0, "%s", "history");
    HistoryIndex = SV_GetHistoryIndex(history);
    v7 = HistoryIndex;
    v8 = g_bufSize[HistoryIndex];
    v9 = v8 + size;
    if ((unsigned int)(v8 + size) > 0x300000)
    {
        Com_PrintError(1, "SV_HistoryAlloc failed. Needed %d more memory\n", v9 - 3145728);
        return 0;
    }
    else
    {
        v10 = &g_buf[HistoryIndex][v8];
        *pData = v10;
        memset(v10, 0, size);
        result = 1;
        g_bufSize[v7] = v9;
    }
    return result;
}

int __cdecl SV_MsgAlloc(unsigned int maxsize)
{
    if (sv.demo.msg.data)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 271, 0, "%s", "!sv.demo.msg.data");
    if (maxsize > 0xA00000)
        return 0;
    sv.demo.msg.data = g_msgBuf;
    sv.demo.msg.maxsize = 10485760;
    memset(g_msgBuf, 0, sizeof(g_msgBuf));
    return 1;
}

void SV_CheckDemoSize()
{
    sv.demo.changed = 1;
    if (sv.demo.msg.data)
    {
        if (sv.demo.msg.data != g_msgBuf)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp",
                290,
                0,
                "%s",
                "sv.demo.msg.data == g_msgBuf");
        if (sv.demo.msg.maxsize != 10485760)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp",
                291,
                0,
                "%s",
                "sv.demo.msg.maxsize == SERVER_DEMO_MESSAGE_MAXSIZE");
    }
    else
    {
        SV_MsgAlloc(0);
    }
}

bool __cdecl SV_DemoWrite(const void *buffer, unsigned int len, _iobuf *file)
{
    return FS_FileWrite(buffer, len, file) == len;
}

int __cdecl SV_FindTimeSkipIndex(int time)
{
    int v1; // r11
    FileSkip *v2; // r10

    v1 = 0;
    if (g_numFileSkips > 0)
    {
        v2 = g_fileSkips;
        do
        {
            if (v2->time > time)
                break;
            ++v1;
            ++v2;
        } while (v1 < g_numFileSkips);
    }
    return v1 - 1;
}

FileMarkSkip *__cdecl SV_FindMarkSkip(const char *name)
{
    int v2; // r31
    FileMarkSkip *i; // r30

    v2 = 0;
    if (g_numFileMarkSkips <= 0)
        return 0;
    for (i = g_fileMarkSkips; I_stricmp(i->name, name); ++i)
    {
        if (++v2 >= g_numFileMarkSkips)
            return 0;
    }
    return &g_fileMarkSkips[v2];
}

void __cdecl SV_TruncateHistoryTimeCache(int maxTime)
{
    int v1; // r11
    FileSkip *v2; // r10
    bool v3; // cr58
    int fileEndOffset; // r4

    v1 = 0;
    if (g_numFileSkips > 0)
    {
        v2 = g_fileSkips;
        do
        {
            if (v2->time > maxTime)
                break;
            ++v1;
            ++v2;
        } while (v1 < g_numFileSkips);
    }
    g_numFileSkips = v1;
    v3 = v1 == 0;
    if (v1 < 0)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 504, 1, "%s", "g_numFileSkips >= 0");
        v1 = g_numFileSkips;
        v3 = g_numFileSkips == 0;
    }
    if (v3)
        fileEndOffset = 0;
    else
        fileEndOffset = g_fileSkips[v1 - 1].fileEndOffset;
    FS_FileSeek(g_fileTimeHistory, fileEndOffset, 2);
}

int SV_ClearHistoryMarkCache()
{
    g_numFileMarkSkips = 0;
    return FS_FileSeek(g_fileMarkHistory, 0, 2);
}

// attributes: thunk
void __cdecl SV_TruncateHistoryCache(int maxTime)
{
    SV_TruncateHistoryTimeCache(maxTime);
}

int SV_SetAutoSaveHistoryTime()
{
    int result; // r3

    result = G_GetTime();
    sv.demo.autoSaveTime = result;
    return result;
}

void __cdecl SV_ResetDemo()
{
    sv.demo.recording = 0;
    sv.demo.playing = 0;
    sv.demo.forwardMsec = 0;
    sv.demo.startLive = 0;
}

_iobuf *SV_ClearHistoryCache()
{
    _iobuf *result; // r3

    g_numFileSkips = 0;
    if (g_fileTimeHistory)
        FS_FileSeek(g_fileTimeHistory, 0, 2);
    result = g_fileMarkHistory;
    g_numFileMarkSkips = 0;
    if (g_fileMarkHistory)
        return (_iobuf *)FS_FileSeek(g_fileMarkHistory, 0, 2);
    return result;
}

void __cdecl SV_FreeDemoSaveBuf(server_demo_save_t *save)
{
    unsigned __int8 *buf; // r3

    buf = save->buf;
    if (buf)
    {
        SV_HistoryFree(buf, save->bufLen);
        save->buf = 0;
        save->bufLen = 0;
    }
}

void __cdecl SV_FreeHistoryData(server_demo_history_t *history)
{
    unsigned __int8 *freeEntBuf; // r3
    unsigned __int8 *cmBuf; // r3
    unsigned __int8 *buf; // r3

    if (!history)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 717, 0, "%s", "history");
    freeEntBuf = history->freeEntBuf;
    if (freeEntBuf)
    {
        SV_HistoryFree(freeEntBuf, history->freeEntBufLen);
        history->freeEntBuf = 0;
        history->freeEntBufLen = 0;
    }
    cmBuf = history->cmBuf;
    if (cmBuf)
    {
        SV_HistoryFree(cmBuf, history->cmBufLen);
        history->cmBuf = 0;
        history->cmBufLen = 0;
    }
    buf = history->save.buf;
    if (buf)
    {
        SV_HistoryFree(buf, history->save.bufLen);
        history->save.buf = 0;
        history->save.bufLen = 0;
    }
}

void __cdecl SV_FreeHistory(server_demo_history_t **history)
{
    server_demo_history_t *v2; // r3

    v2 = *history;
    if (v2)
    {
        SV_FreeHistoryData(v2);
        *history = 0;
    }
}

void SV_FreeDemoMsg()
{
    sv.demo.changed = 0;
    if (g_history)
    {
        SV_FreeHistoryData(g_history);
        g_history = 0;
    }
    if (sv.demo.msg.data)
    {
        sv.demo.msg.data = 0;
        sv.demo.msg.maxsize = 0;
    }
}

int __cdecl SV_WaitForSaveHistoryDone()
{
    if (!g_savingHistory)
        return 1;
    if (Sys_WaitForSaveHistoryDone())
    {
        g_savingHistory = 0;
        return 1;
    }
    if (g_history)
    {
        SV_FreeHistoryData(g_history);
        g_history = 0;
    }
    return 0;
}

void __cdecl SV_ShutdownDemo()
{
    SV_FreeDemoMsg();
    if (g_history)
    {
        SV_FreeHistoryData(g_history);
        g_history = 0;
    }
    if ((unsigned __int8)SV_WaitForSaveHistoryDone())
    {
        if (sv.demo.save.buf)
        {
            SV_HistoryFree(sv.demo.save.buf, sv.demo.save.bufLen);
            sv.demo.save.buf = 0;
            sv.demo.save.bufLen = 0;
        }
    }
}

int __cdecl SV_AddDemoSave(SaveGame *savehandle, server_demo_save_t *save, int createSave)
{
    server_demo_history_t *v6; // r26
    SaveGame *v7; // r31
    SaveGame *MemoryFile; // r3
    SaveGame *v9; // r3
    SaveGame *v10; // r3
    bool IsSuccessful; // zf
    SaveGame *v13; // r3
    int v14; // r30
    unsigned __int8 *v15; // r29
    SaveGame *v16; // r3
    int v17; // r4
    unsigned __int8 *v18; // r5
    SaveGame *v19; // r3
    int v20; // r30
    SaveGame *v21; // r3
    int v22; // r29
    unsigned __int8 *buf; // r26
    SaveGame *v24; // r3
    unsigned __int8 *v25; // r30

    if (g_history)
    {
        v6 = g_history;
    }
    else
    {
        if (g_historySaving)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 830, 0, "%s", "!g_historySaving");
        v6 = g_historyBuffers;
    }
    if (save->buf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 837, 0, "%s", "!save->buf");
    v7 = SaveMemory_GetSaveHandle(1u);
    if (!v7)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 841, 0, "%s", "demohandle");
    if (savehandle == v7)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 842, 0, "%s", "savehandle != demohandle");
    SaveMemory_InitializeDemoSave(v7);
    MemoryFile = SaveMemory_GetMemoryFile(v7);
    Dvar_SaveDvars(&MemoryFile->memFile, 4u);
    v9 = SaveMemory_GetMemoryFile(v7);
    Scr_SaveSource(&v9->memFile);
    if (savehandle)
    {
        SaveMemory_StartSegment(v7, -1);
        IsSuccessful = !SaveMemory_IsSuccessful(v7);
        v10 = v7;
        if (!IsSuccessful)
        {
            v19 = SaveMemory_GetMemoryFile(v7);
            v20 = MemFile_CopySegments(&v19->memFile, 0, 0);
            v21 = SaveMemory_GetMemoryFile(savehandle);
            v22 = v20 + MemFile_CopySegments(&v21->memFile, 1, 0);
            IsSuccessful = SV_HistoryAlloc(v6, &save->buf, v22) == 0;
            v10 = v7;
            if (!IsSuccessful)
            {
                buf = save->buf;
                save->bufLen = v22;
                v24 = SaveMemory_GetMemoryFile(v7);
                MemFile_CopySegments(&v24->memFile, 0, buf);
                v25 = &save->buf[v20];
                v16 = SaveMemory_GetMemoryFile(savehandle);
                v17 = 1;
                v18 = v25;
                goto LABEL_22;
            }
        }
    LABEL_16:
        SaveMemory_FinalizeSave(v10);
        return 0;
    }
    if (createSave)
        G_SaveState(0, v7);
    SaveMemory_StartSegment(v7, -1);
    IsSuccessful = SaveMemory_IsSuccessful(v7);
    v10 = v7;
    if (!IsSuccessful)
        goto LABEL_16;
    v13 = SaveMemory_GetMemoryFile(v7);
    v14 = MemFile_CopySegments(&v13->memFile, 0, 0);
    IsSuccessful = SV_HistoryAlloc(v6, &save->buf, v14) == 0;
    v10 = v7;
    if (IsSuccessful)
        goto LABEL_16;
    v15 = save->buf;
    save->bufLen = v14;
    v16 = SaveMemory_GetMemoryFile(v7);
    v17 = 0;
    v18 = v15;
LABEL_22:
    MemFile_CopySegments(&v16->memFile, v17, v18);
    SaveMemory_FinalizeSave(v7);
    return 1;
}

_iobuf *__cdecl SV_DemoOpenFile(const char *fileName)
{
    const char *v2; // r3
    char v4[288]; // [sp+50h] [-120h] BYREF

    v2 = Sys_DefaultInstallPath();
    FS_BuildOSPath(v2, "", fileName, v4);
    if (!FS_CreatePath(v4))
        return FS_FileOpenWriteReadBinary(v4);
    Com_PrintError(1, "Failed to create path '%s'\n", v4);
    return 0;
}

void __cdecl SV_InitWriteDemo(int randomSeed)
{
    ProfLoad_Begin("SV_InitWriteDemo");
    if (sv.demo.nextLevelplaying)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1030, 0, "%s", "!sv.demo.nextLevelplaying");
    SV_FreeDemoMsg();
    if (CL_DemoPlaying())
    {
        sv.demo.recording = 0;
        CM_LinkWorld();
        G_UpdateAllEntities();
        ProfLoad_End();
    }
    else
    {
        sv.demo.startTime = G_GetTime();
        sv.demo.recording = 1;
        if (sv.demo.msg.data)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1045, 0, "%s", "!sv.demo.msg.data");
        MSG_Init(&sv.demo.msg, 0, 0);
        SV_CheckDemoSize();
        MSG_WriteLong(&sv.demo.msg, randomSeed);
        SV_ClearHistoryCache();
        CM_LinkWorld();
        G_UpdateAllEntities();
        ProfLoad_End();
    }
}

void __cdecl SV_InitReadDemoSavegame(SaveGame **saveHandle)
{
    server_demo_save_t *p_save; // r28
    SaveGame *v3; // r31
    SaveGame *MemoryFile; // r3
    SaveGame *v5; // r3
    SaveGame *v6; // r3

    if (!saveHandle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1074, 0, "%s", "saveHandle");
    if (!sv.demo.nextLevelplaying)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1075, 0, "%s", "sv.demo.nextLevelplaying");
    if (sv.demo.nextLevelSave)
        p_save = &sv.demo.nextLevelSave->save;
    else
        p_save = &sv.demo.save;
    if (*saveHandle)
    {
        SV_TruncateHistoryTimeCache(0);
        g_numFileMarkSkips = 0;
        FS_FileSeek(g_fileMarkHistory, 0, 2);
    }
    if (!p_save->buf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1088, 0, "%s", "save->buf");
    if (*saveHandle)
        SaveMemory_FinalizeLoad(*saveHandle);
    v3 = SaveMemory_GetSaveHandle(1u);
    SaveMemory_InitializeLoadFromBuffer(v3, p_save->buf, p_save->bufLen);
    MemoryFile = SaveMemory_GetMemoryFile(v3);
    Dvar_LoadDvars(&MemoryFile->memFile);
    v5 = SaveMemory_GetMemoryFile(v3);
    Scr_SkipSource(&v5->memFile, 0);
    v6 = SaveMemory_GetMemoryFile(v3);
    if (MemFile_AtEnd(&v6->memFile))
    {
        SaveMemory_MoveToSegment(v3, -1);
        SaveMemory_FinalizeLoad(v3);
        *saveHandle = 0;
    }
    else
    {
        *saveHandle = v3;
    }
}

int __cdecl SV_InitDemoSavegame(SaveGame **save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1222, 0, "%s", "save");
    if (sv.demo.nextLevelplaying)
    {
        SV_InitReadDemoSavegame(save);
    }
    else
    {
        sv.demo.nextLevelSave = 0;
        if (sv.demo.save.buf)
        {
            SV_HistoryFree(sv.demo.save.buf, sv.demo.save.bufLen);
            sv.demo.save.buf = 0;
            sv.demo.save.bufLen = 0;
        }
        SV_AddDemoSave(*save, &sv.demo.save, 0);
    }
    return 1;
}

bool __cdecl SV_IsDemoPlaying()
{
    return sv.demo.playing;
}

bool __cdecl SV_UsingDemoSave()
{
    return sv.demo.nextLevelSave != 0;
}

void __cdecl SV_RecordClientCommand(const char *s)
{
    int FramePos; // r3

    if (sv.demo.recording)
    {
        SV_CheckDemoSize();
        MSG_WriteByte(&sv.demo.msg, 2);
        FramePos = G_GetFramePos();
        MSG_WriteLong(&sv.demo.msg, FramePos);
        MSG_WriteString(&sv.demo.msg, (char*)s);
    }
}

void __cdecl SV_RecordClientThink(usercmd_s *cmd)
{
    int FramePos; // r3

    if (sv.demo.recording)
    {
        SV_CheckDemoSize();
        MSG_WriteByte(&sv.demo.msg, 0);
        FramePos = G_GetFramePos();
        MSG_WriteLong(&sv.demo.msg, FramePos);
        MSG_WriteDeltaUsercmd(&sv.demo.msg, &svs.clients->lastUsercmd, cmd);
    }
}

void __cdecl SV_RecordFxVisibility(double visibility)
{
    if (sv.demo.recording)
    {
        SV_CheckDemoSize();
        MSG_WriteByte(&sv.demo.msg, 1);
        MSG_WriteFloat(&sv.demo.msg, visibility);
    }
}

void __cdecl SV_RecordCheatsOk(int cheatsOk)
{
    if (sv.demo.recording)
    {
        SV_CheckDemoSize();
        MSG_WriteByte(&sv.demo.msg, 7);
        MSG_WriteLong(&sv.demo.msg, cheatsOk);
    }
}

void __cdecl SV_RecordIsRecentlyLoaded(bool isRecentlyLoaded)
{
    if (sv.demo.recording)
    {
        SV_CheckDemoSize();
        MSG_WriteByte(&sv.demo.msg, 10);
        MSG_WriteByte(&sv.demo.msg, isRecentlyLoaded);
    }
}

void __cdecl SV_Record_Dvar_GetVariantString(const char *buffer)
{
    if (sv.demo.recording)
    {
        SV_CheckDemoSize();
        MSG_WriteByte(&sv.demo.msg, 3);
        MSG_WriteString(&sv.demo.msg, (char*)buffer);
    }
}

void __cdecl SV_RecordButtonPressed(int buttonPressed)
{
    if (sv.demo.recording)
    {
        SV_CheckDemoSize();
        MSG_WriteByte(&sv.demo.msg, 8);
        MSG_WriteByte(&sv.demo.msg, buttonPressed);
    }
}

void __cdecl SV_GetFreeDemoName(const char *baseName, int demoCount, char *testDemoName)
{
    int i; // r30
    int Remote; // r3
    int v8; // r27
    void *v9[4]; // [sp+50h] [-150h] BYREF
    char v10[320]; // [sp+60h] [-140h] BYREF

    for (i = 1; !demoCount || i < demoCount; ++i)
    {
        Com_sprintf(testDemoName, 64, "%s%i", baseName, i);
        Com_BuildPlayerProfilePath(v10, 256, "save/%s.svg", testDemoName);
        if (FS_IsUsingRemotePCSharing())
            Remote = FS_FOpenFileReadRemote(v10, 0, v9);
        else
            Remote = FS_FOpenFileReadCurrentThread(v10, v9);
        v8 = Remote;
        FS_FCloseFile(v9[0]);
        if (v8 <= 0)
            goto LABEL_10;
    }
    i = 0;
LABEL_10:
    if (demoCount)
    {
        __twllei(demoCount, 0);
        __twlgei(demoCount & ~(__ROL4__(i + 1, 1) - 1), 0xFFFFFFFF);
        Com_sprintf(testDemoName, 64, "%s%i", baseName, (i + 1) % demoCount);
        Com_BuildPlayerProfilePath(v10, 256, "save/%s.svg", testDemoName);
        //if (FS_IsUsingRemotePCSharing())
        //    FS_DeleteRemote(v10);
        //else
            FS_Delete(v10);
        Com_sprintf(testDemoName, 64, "%s%i", baseName, i);
    }
}

void __cdecl SV_SaveDemoImmediate(SaveImmediate *save)
{
    if (!sv.demo.msg.data)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1689, 0, "%s", "sv.demo.msg.data");
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1690, 0, "%s", "save");
    SaveMemory_SaveWriteImmediate(&sv.demo, 4, save);
    SaveMemory_SaveWriteImmediate(&sv.demo.endTime, 4, save);
    SaveMemory_SaveWriteImmediate(&sv.demo.msg.cursize, 4, save);
    SaveMemory_SaveWriteImmediate(sv.demo.msg.data, sv.demo.msg.cursize, save);
}

void __cdecl SV_WriteDemo(SaveGame *save)
{
    if (!sv.demo.save.bufLen)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1707, 0, "%s", "sv.demo.save.bufLen");
    if (!sv.demo.save.buf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1708, 0, "%s", "sv.demo.save.buf");
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1709, 0, "%s", "save");
    SaveMemory_SetBuffer(sv.demo.save.buf, sv.demo.save.bufLen, save);
}

void __cdecl SV_SaveDemo(const char *demoName, const char *description, unsigned __int32 saveType)
{
    const char *v3; // r29
    SaveGame *SaveHandle; // r31
    SaveGame *v7; // r3
    bool v8; // zf
    SaveGame *v9; // [sp+8h] [-108h]
    int v10; // [sp+Ch] [-104h]
    int v11; // [sp+10h] [-100h]
    int v12; // [sp+14h] [-FCh]
    int v13; // [sp+18h] [-F8h]
    int v14; // [sp+1Ch] [-F4h]
    int v15; // [sp+20h] [-F0h]
    int v16; // [sp+24h] [-ECh]
    int v17; // [sp+28h] [-E8h]
    int v18; // [sp+2Ch] [-E4h]
    int v19; // [sp+30h] [-E0h]
    int v20; // [sp+34h] [-DCh]
    int v21; // [sp+38h] [-D8h]
    int v22; // [sp+3Ch] [-D4h]
    int v23; // [sp+40h] [-D0h]
    int v24; // [sp+44h] [-CCh]
    int v25; // [sp+48h] [-C8h]
    int v26; // [sp+4Ch] [-C4h]
    int v27; // [sp+50h] [-C0h]
    char v28[64]; // [sp+60h] [-B0h] BYREF
    char v29[112]; // [sp+A0h] [-70h] BYREF

    v3 = demoName;
    if (!sv.demo.msg.data)
    {
        Com_Printf(15, "No replay.\n");
        return;
    }
    if (!sv.demo.save.bufLen)
    {
        Com_Printf(15, "Replay start failed to save.\n");
        return;
    }
    sv.demo.changed = 0;
    if (!demoName)
    {
        SV_GetFreeDemoName("replay", 50, v28);
        v3 = v28;
    }
    SaveHandle = SaveMemory_GetSaveHandle(1u);
    if (!SaveHandle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1750, 0, "%s", "save");
    SaveMemory_InitializeDemoSave(SaveHandle);
    SaveMemory_StartSegment(SaveHandle, -1);
    v8 = !SaveMemory_IsSuccessful(SaveHandle);
    v7 = SaveHandle;
    if (v8)
        goto LABEL_12;
    SV_WriteDemo(SaveHandle);
    if (!BuildCleanSavePath(v29, 0x40u, v3, (SaveType)saveType))
    {
        v7 = SaveHandle;
    LABEL_12:
        SaveMemory_FinalizeSave(v7);
        return;
    }
    SaveMemory_CreateHeader(
        v29,
        description,
        0,
        sv.checksum,
        1,
        0,
        saveType,
        0,
        v9,
        v10,
        v11,
        v12,
        v13,
        v14,
        v15,
        v16,
        v17,
        v18,
        v19,
        v20,
        v21,
        v22,
        v23,
        v24,
        v25,
        v26,
        v27,
        (int)SaveHandle);
    MemCard_SetUseDevDrive(1);
    SaveMemory_FinalizeSaveToDisk(SaveHandle);
    MemCard_SetUseDevDrive(0);
}

void __cdecl SV_AutoSaveDemo(const char *baseName, const char *description, int demoCount, bool force)
{
    SaveGame *SaveHandle; // r3
    char v9[112]; // [sp+50h] [-70h] BYREF

    if (!Sys_IsDatabaseThread() && sv.state == SS_GAME && sv.demo.msg.data && (force || sv.demo.changed))
    {
        SaveHandle = SaveMemory_GetSaveHandle(1u);
        if (!(unsigned __int8)SaveMemory_IsSaving(SaveHandle))
        {
            SV_GetFreeDemoName(baseName, demoCount, v9);
            SV_SaveDemo(v9, description, 0);
        }
    }
}

void SV_EnableAutoDemo()
{
    if (replay_time && replay_autosave)
    {
        Dvar_SetBool(replay_time, 1);
        if (replay_autosave->current.integer > 2)
            Dvar_SetInt(replay_autosave, 2);
    }
}

void __cdecl SV_SaveDemo_f()
{
    int nesting; // r7
    const char *v1; // r3

    if (replay_time)
    {
        if (replay_autosave)
        {
            Dvar_SetBool(replay_time, 1);
            if (replay_autosave->current.integer > 2)
                Dvar_SetInt(replay_autosave, 2);
        }
    }
    nesting = sv_cmd_args.nesting;
    if (sv_cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\server\\../qcommon/cmd.h",
            167,
            0,
            "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            sv_cmd_args.nesting,
            8);
        nesting = sv_cmd_args.nesting;
    }
    if (sv_cmd_args.argc[nesting] <= 2)
    {
        if (SV_Cmd_Argc() == 2)
            v1 = SV_Cmd_Argv(1);
        else
            v1 = 0;
        SV_SaveDemo(v1, 0, 2u);
    }
    else
    {
        Com_Printf(0, "replay_save <name>\n");
    }
}

void SV_DemoRestart()
{
    if (!sv.demo.msg.data)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1863, 0, "%s", "sv.demo.msg.data");
    sv.demo.recording = 0;
    sv.demo.playing = 0;
    sv.demo.forwardMsec = 0;
    sv.demo.startLive = 0;
    sv.demo.nextLevelplaying = 1;
    SV_RequestMapRestart(0);
}

void __cdecl SV_DemoRestart_f()
{
    if (replay_time)
    {
        if (replay_autosave)
        {
            Dvar_SetBool(replay_time, 1);
            if (replay_autosave->current.integer > 2)
                Dvar_SetInt(replay_autosave, 2);
        }
    }
    if (sv.demo.msg.data)
    {
        sv.demo.nextLevelSave = 0;
        SV_DemoRestart();
    }
    else
    {
        Com_Printf(0, "No replay.\n");
    }
}

int __cdecl SV_DemoHasMark()
{
    unsigned __int8 v0; // r11

    if (!sv.demo.playing)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1899, 0, "%s", "sv.demo.playing");
    if (!g_history)
        return 0;
    v0 = 1;
    if (!g_history->manual)
        return 0;
    return v0;
}

void __cdecl SV_LoadDemo(SaveGame *save, void *fileHandle)
{
    const SaveHeader *Header; // r30
    int bodySize; // r5
    unsigned __int8 *buf; // r31
    SaveGame *MemoryFile; // r3
    unsigned int v8; // [sp+50h] [-40h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1914, 0, "%s", "save");
    sv.demo.recording = 0;
    sv.demo.playing = 0;
    sv.demo.forwardMsec = 0;
    sv.demo.startLive = 0;
    SV_FreeDemoMsg();
    sv.demo.nextLevelplaying = 1;
    sv.demo.nextLevelSave = 0;
    sv.demo.nextLevelTime = 0;
    ReadFromDevice(&sv.demo, 4, fileHandle);
    ReadFromDevice(&sv.demo.endTime, 4, fileHandle);
    ReadFromDevice(&v8, 4, fileHandle);
    if (sv.demo.msg.data)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1927, 0, "%s", "!sv.demo.msg.data");
    if (!(unsigned __int8)SV_MsgAlloc(v8))
        Sys_OutOfMemErrorInternal("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1929);
    MSG_Init(&sv.demo.msg, sv.demo.msg.data, sv.demo.msg.maxsize);
    sv.demo.msg.cursize = v8;
    ReadFromDevice(sv.demo.msg.data, v8, fileHandle);
    if (sv.demo.save.buf)
    {
        SV_HistoryFree(sv.demo.save.buf, sv.demo.save.bufLen);
        sv.demo.save.buf = 0;
        sv.demo.save.bufLen = 0;
    }
    Header = SaveMemory_GetHeader(save);
    if (!Header->bodySize)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1938, 0, "%s", "header->bodySize");
    bodySize = Header->bodySize;
    sv.demo.save.bufLen = bodySize;
    if (sv.demo.save.buf)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1942, 0, "%s", "!sv.demo.save.buf");
        bodySize = sv.demo.save.bufLen;
    }
    if (!SV_HistoryAlloc(g_historyBuffers, &sv.demo.save.buf, bodySize))
        Sys_OutOfMemErrorInternal("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1944);
    buf = sv.demo.save.buf;
    MemoryFile = SaveMemory_GetMemoryFile(save);
    MemFile_CopySegments(&MemoryFile->memFile, 0, buf);
}

void Cmd_Echo_f()
{
    int i; // r28
    int nesting; // r7
    const char *v2; // r3

    for (i = 1; ; ++i)
    {
        nesting = sv_cmd_args.nesting;
        if (sv_cmd_args.nesting >= 8u)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\qcommon\\cmd.h",
                167,
                0,
                "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
                sv_cmd_args.nesting,
                8);
            nesting = sv_cmd_args.nesting;
        }
        if (i >= sv_cmd_args.argc[nesting])
            break;
        v2 = SV_Cmd_Argv(i);
        Com_Printf(0, "^3%s ", v2);
    }
    Com_Printf(0, "\n");
    if (SV_RecordingDemo())
    {
        if (sv_cmd_args.nesting >= 8u)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\qcommon\\cmd.h",
                203,
                0,
                "sv_cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
                sv_cmd_args.nesting,
                8);
        Con_CloseConsole(0);
    }
}

bool __cdecl SV_RecordingDemo()
{
    return sv.demo.recording;
}

int __cdecl SV_Demo_Dvar_Set(const char *var_name, const char *value)
{
    int FramePos; // r3

    if (sv.demo.playing)
        return 0;
    if (sv.demo.recording)
    {
        SV_CheckDemoSize();
        MSG_WriteByte(&sv.demo.msg, 6);
        FramePos = G_GetFramePos();
        MSG_WriteLong(&sv.demo.msg, FramePos);
        MSG_WriteString(&sv.demo.msg, (char*)var_name);
        MSG_WriteString(&sv.demo.msg, (char*)value);
    }
    return 1;
}

int __cdecl SV_WriteDemoSaveBuf(server_demo_save_t *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1994, 0, "%s", "save");
    if (save->buf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1995, 0, "%s", "!save->buf");
    save->buf = 0;
    save->bufLen = 0;
    return SV_AddDemoSave(0, save, 1);
}

bool __cdecl SV_WriteHistory(_iobuf *fileHistory, const server_demo_history_t *history)
{
    unsigned int bufLen; // r30
    unsigned int cmBufLen; // r30
    unsigned int freeEntBufLen; // r30

    if (!history)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2005, 0, "%s", "history");
    FS_FileSeek(fileHistory, 0, 0);
    if (FS_FileWrite(history, 0xACu, fileHistory) != 172)
        return 0;
    bufLen = history->save.bufLen;
    if (bufLen != FS_FileWrite(history->save.buf, bufLen, fileHistory))
        return 0;
    cmBufLen = history->cmBufLen;
    if (cmBufLen != FS_FileWrite(history->cmBuf, cmBufLen, fileHistory))
        return 0;
    freeEntBufLen = history->freeEntBufLen;
    return FS_FileWrite(history->freeEntBuf, freeEntBufLen, fileHistory) == freeEntBufLen;
}

void __cdecl SV_SaveHistoryTime(server_demo_history_t *history)
{
    _iobuf *v2; // r3
    _iobuf *v3; // r3
    int v4; // r3
    int v5; // r11

    if (!history)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2025, 0, "%s", "history");
    if (history->manual)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2026, 0, "%s", "!history->manual");
    v2 = g_fileTimeHistory;
    if (g_fileTimeHistory)
    {
        if (g_numFileSkips == 3600)
        {
            Com_PrintWarning(15, "Failed to save demo history.  Disabling autosave.\n");
            Dvar_SetInt(replay_autosave, 0);
        }
        else
        {
            if (g_numFileSkips && g_fileSkips[g_numFileSkips - 1].time >= history->time)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp",
                    2041,
                    0,
                    "%s",
                    "g_numFileSkips == 0 || g_fileSkips[g_numFileSkips - 1].time < history->time");
                v2 = g_fileTimeHistory;
            }
            if (!SV_WriteHistory(v2, history))
                Com_PrintError(1, "Failed to save demo history.\n");
            v3 = g_fileTimeHistory;
            g_fileSkips[g_numFileSkips].time = history->time;
            v4 = FS_FileTell(v3);
            v5 = g_numFileSkips + 1;
            g_fileSkips[g_numFileSkips].fileEndOffset = v4;
            g_numFileSkips = v5;
        }
    }
    else
    {
        Com_PrintError(1, "Failed to open demo cache file.\n");
    }
}

void __cdecl SV_SaveHistoryMark(const server_demo_history_t *history)
{
    FileMarkSkip *MarkSkip; // r31
    int v3; // r11
    _iobuf *v4; // r3

    if (!history)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2057, 0, "%s", "history");
    if (!history->manual)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2058, 0, "%s", "history->manual");
    if (!g_fileMarkHistory)
    {
        Com_PrintError(1, "Failed to open demo cache file.\n");
        return;
    }
    MarkSkip = SV_FindMarkSkip(history->name);
    if (!MarkSkip)
    {
        if (g_numFileMarkSkips == 50)
        {
            Com_PrintError(1, "Out of mark slots.\n");
            return;
        }
        MarkSkip = &g_fileMarkSkips[g_numFileMarkSkips];
        I_strncpyz(MarkSkip->name, history->name, 64);
        ++g_numFileMarkSkips;
    }
    v3 = FS_FileTell(g_fileMarkHistory);
    v4 = g_fileMarkHistory;
    MarkSkip->fileOffset = v3;
    if (!SV_WriteHistory(v4, history))
    {
        Com_PrintError(1, "Failed to save demo history.\n");
        MarkSkip->name[0] = 0;
    }
}

void __cdecl SV_SaveHistory(server_demo_history_t *history)
{
    //Profile_Begin(406);
    if (history->manual)
        SV_SaveHistoryMark(history);
    else
        SV_SaveHistoryTime(history);
    //Profile_EndInternal(0);
}

void __cdecl  SV_SaveHistoryLoop(unsigned int threadContext)
{
    server_demo_history_t *volatile v1; // r31

    if (threadContext != 11)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp",
            2105,
            0,
            "threadContext == THREAD_CONTEXT_SERVER_DEMO\n\t%i, %i",
            threadContext,
            11);
    while (1)
    {
        Sys_WaitForSaveHistory();
        //__lwsync();
        if (!g_historySaving)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2114, 0, "%s", "g_historySaving");
        v1 = g_historySaving;
        //Profile_Begin(406);
        if (v1->manual)
            SV_SaveHistoryMark(v1);
        else
            SV_SaveHistoryTime(v1);
        //Profile_EndInternal(0);
        SV_FreeHistoryData(g_historySaving);
        g_historySaving = 0;
        //__lwsync();
        Sys_SetSaveHistoryDoneEvent();
    }
}

bool SV_InitHistorySaveThread()
{
    return Sys_SpawnServerDemoThread((void(__cdecl *)(unsigned int))SV_SaveHistoryLoop);
}

void __cdecl SV_InitDemoSystem()
{
    if (g_fileTimeHistory)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2136, 0, "%s", "!g_fileTimeHistory");
    if (g_fileMarkHistory)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2137, 0, "%s", "!g_fileMarkHistory");
    g_fileTimeHistory = SV_DemoOpenFile("timeHistory.cache");
    g_fileMarkHistory = SV_DemoOpenFile("markHistory.cache");
    Sys_SpawnServerDemoThread((void(__cdecl *)(unsigned int))SV_SaveHistoryLoop);
}

server_demo_history_t *__cdecl SV_DemoGetFreeBuffer()
{
    server_demo_history_t *result; // r3

    result = g_historyBuffers;
    if (g_historySaving == g_historyBuffers)
        return &g_historyBuffers[1];
    return result;
}

int __cdecl SV_HistoryIsNew(server_demo_history_t *history)
{
    unsigned __int8 v1; // r11

    if (!g_numFileSkips)
        return 1;
    if (history->manual)
        return 1;
    v1 = 0;
    if (history->time > g_fileSkips[g_numFileSkips - 1].time)
        return 1;
    return v1;
}

void __cdecl SV_ClearInfrequentTimeMarks(server_demo_history_t *history)
{
    int v2; // r11
    char v3; // r10
    int v4; // r9
    FileSkip *v5; // r10
    int v6; // r31
    int time; // r3

    if (history)
    {
        v2 = g_numFileSkips;
        if (!g_numFileSkips || history->manual || (v3 = 0, history->time > g_fileSkips[g_numFileSkips - 1].time))
            v3 = 1;
        if (!v3)
        {
            v4 = 0;
            if (g_numFileSkips > 0)
            {
                v5 = g_fileSkips;
                do
                {
                    if (v5->time > history->time)
                        break;
                    ++v4;
                    ++v5;
                } while (v4 < g_numFileSkips);
            }
            v6 = v4 - 1;
            if (v4 - 1 < -1 || v6 >= g_numFileSkips)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp",
                    2189,
                    0,
                    "%s\n\t(fileSkipIndex) = %i",
                    "(fileSkipIndex >= -1 && fileSkipIndex < g_numFileSkips)",
                    v4 - 1);
                v2 = g_numFileSkips;
            }
            if (v6 != v2 - 1)
            {
                time = history->time;
                if (g_fileSkips[v6 + 1].time > 1500 * replay_autosave->current.integer + time)
                    SV_TruncateHistoryTimeCache(time);
            }
        }
    }
}

server_demo_history_t *__cdecl SV_DemoGetBuffer()
{
    server_demo_history_t *v0; // r10
    char v1; // r11
    server_demo_history_t *result; // r3
    server_demo_history_t *v3; // r3

    SV_ClearInfrequentTimeMarks(g_history);
    v0 = g_history;
    if (g_history)
    {
        if (!g_numFileSkips || g_history->manual || (v1 = 0, g_history->time > g_fileSkips[g_numFileSkips - 1].time))
            v1 = 1;
        if (v1)
        {
            if (g_savingHistory)
            {
                //__lwsync();
                if (g_historySaving)
                {
                    if (!replay_autosave)
                        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2213, 0, "%s", "replay_autosave");
                    Com_PrintError(1, "Stalling for previous demo history to save.\n");
                    Com_PrintError(
                        1,
                        "You may want to increase replay_autosave(%i) to avoid hitches.\n",
                        replay_autosave->current.integer);
                }
                if (!Sys_WaitForSaveHistoryDone())
                {
                    Com_PrintError(1, "Demo history save time out.\n");
                    result = 0;
                    g_history = 0;
                    return result;
                }
                g_savingHistory = 0;
                if (g_historySaving)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2227, 0, "%s", "!g_historySaving");
                v0 = g_history;
            }
            if (g_historySaving)
            {
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2230, 0, "%s", "!g_historySaving");
                v0 = g_history;
            }
            g_historySaving = v0;
            //__lwsync();
            g_savingHistory = 1;
            Sys_SetSaveHistoryEvent();
        }
    }
    v3 = &g_historyBuffers[1];
    if (g_historySaving != g_historyBuffers)
        v3 = g_historyBuffers;
    g_history = v3;
    SV_FreeHistoryData(v3);
    memset(g_history, 0, sizeof(server_demo_history_t));
    g_history->time = G_GetTime();
    return g_history;
}

server_demo_history_t *__cdecl SV_GetMarkHistory(const char *name)
{
    if (!SV_DemoGetBuffer() || !(unsigned __int8)SV_WaitForSaveHistoryDone())
        return 0;
    g_history->manual = 1;
    I_strncpyz(g_history->name, name, 64);
    return g_history;
}

int __cdecl SV_DemoSaveHistory(server_demo_history_t *history)
{
    server_demo_save_t *p_save; // r27
    bool playing; // r11
    int v4; // r11
    int v5; // r3
    int v7; // r3

    p_save = &history->save;
    if (history->save.buf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2279, 0, "%s", "!history->save.buf");
    if (history->cmBuf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2280, 0, "%s", "!history->cmBuf");
    if (history->freeEntBuf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2281, 0, "%s", "!history->freeEntBuf");
    //Profile_Begin(405);
    playing = sv.demo.playing;
    if (sv.demo.playing)
    {
        if (sv.demo.recording)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2287, 0, "%s", "!sv.demo.recording");
        history->nextFramePos = sv.demo.nextFramePos;
        history->readType = sv.demo.readType;
        v4 = 60060;
    }
    else
    {
        if (!sv.demo.recording)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2298, 0, "%s", "sv.demo.recording");
            playing = sv.demo.playing;
        }
        if (playing)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2299, 0, "%s", "!sv.demo.playing");
        history->nextFramePos = G_GetFramePos();
        history->readType = 9;
        v4 = 60052;
    }
    history->msgReadcount = *(serverState_t *)((char *)&sv.state + v4);
    history->msgBit = sv.demo.msg.bit;
    history->randomSeed = G_GetRandomSeed();
    memcpy(&history->lastUsercmd, &svs.clients->lastUsercmd, sizeof(history->lastUsercmd));
    if (SV_WriteDemoSaveBuf(p_save)
        && (v5 = CM_SaveWorld(0), history->cmBufLen = v5, SV_HistoryAlloc(history, &history->cmBuf, v5))
        && (CM_SaveWorld(history->cmBuf),
            v7 = G_SaveFreeEntities(0),
            history->freeEntBufLen = v7,
            SV_HistoryAlloc(history, &history->freeEntBuf, v7)))
    {
        G_SaveFreeEntities(history->freeEntBuf);
        //Profile_EndInternal(0);
        return 1;
    }
    else
    {
        //Profile_EndInternal(0);
        return 0;
    }
}

void __cdecl SV_DemoMark_f()
{
    const char *v0; // r3

    if (replay_time)
    {
        if (replay_autosave)
        {
            Dvar_SetBool(replay_time, 1);
            if (replay_autosave->current.integer > 2)
                Dvar_SetInt(replay_autosave, 2);
        }
    }
    if (sv.demo.msg.data)
    {
        if (sv.demo.playing || sv.demo.recording)
        {
            if (SV_Cmd_Argc() <= 1)
                v0 = "";
            else
                v0 = SV_Cmd_Argv(1);
            if (SV_GetMarkHistory(v0))
            {
                SV_FreeHistoryData(g_history);
                if (!SV_DemoSaveHistory(g_history))
                {
                    if (g_history)
                    {
                        SV_FreeHistoryData(g_history);
                        g_history = 0;
                    }
                }
            }
            else
            {
                Com_Printf(0, "Mark failed because previous save hasn't finished.\n");
            }
        }
        else
        {
            Com_Printf(0, "Past end of replay.\n");
        }
    }
    else
    {
        Com_Printf(0, "No replay.\n");
    }
}

bool __cdecl SV_DemoRead(void *buffer, unsigned int len, _iobuf *file)
{
    return FS_FileRead(buffer, len, file) == len;
}

int __cdecl SV_DemoAllocRead(
    server_demo_history_t *history,
    unsigned __int8 **buffer,
    unsigned int len,
    _iobuf *file)
{
    int result; // r3

    result = SV_HistoryAlloc(history, buffer, len);
    if (result)
        return FS_FileRead(*buffer, len, file) == len;
    return result;
}

bool __cdecl SV_ReadHistory(_iobuf *fileHistory, server_demo_history_t *history)
{
    int bufLen; // r31
    int cmBufLen; // r31
    int freeEntBufLen; // r31
    bool v10; // r11

    if (!fileHistory)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2399, 0, "%s", "fileHistory");
    if (!history)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2400, 0, "%s", "history");
    if (FS_FileRead(history, 0xACu, fileHistory) != 172)
        return 0;
    bufLen = history->save.bufLen;
    if (!SV_HistoryAlloc(history, &history->save.buf, bufLen)
        || FS_FileRead(history->save.buf, bufLen, fileHistory) != bufLen)
    {
        return 0;
    }
    cmBufLen = history->cmBufLen;
    if (!SV_HistoryAlloc(history, &history->cmBuf, cmBufLen)
        || FS_FileRead(history->cmBuf, cmBufLen, fileHistory) != cmBufLen)
    {
        return 0;
    }
    freeEntBufLen = history->freeEntBufLen;
    if (SV_HistoryAlloc(history, &history->freeEntBuf, freeEntBufLen))
        v10 = FS_FileRead(history->freeEntBuf, freeEntBufLen, fileHistory) == freeEntBufLen;
    else
        v10 = 0;
    return (_cntlzw(v10) & 0x20) == 0;
}

bool __cdecl SV_DemoLoadHistory(_iobuf *fileHistory, int fileOffset)
{
    if (!fileHistory)
        return 0;
    while (g_historySaving)
        ;
    if (FS_FileSeek(fileHistory, fileOffset, 2))
    {
        Com_PrintError(1, "SV_DemoLoadHistory: seek failed\n");
        return 0;
    }
    SV_FreeHistoryData(g_history);
    return SV_ReadHistory(fileHistory, g_history);
}

bool __cdecl SV_LoadHistoryForTime(int time)
{
    int v2; // r11
    FileSkip *v3; // r10
    int v4; // r31

    if (!SV_DemoGetBuffer() || !(unsigned __int8)SV_WaitForSaveHistoryDone())
        return 0;
    v2 = 0;
    if (g_numFileSkips > 0)
    {
        v3 = g_fileSkips;
        do
        {
            if (v3->time > time)
                break;
            ++v2;
            ++v3;
        } while (v2 < g_numFileSkips);
    }
    v4 = v2 - 1;
    if (!v2)
    {
        if (g_history)
        {
            SV_FreeHistoryData(g_history);
            g_history = 0;
        }
        return 0;
    }
    if (v4 >= g_numFileSkips)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp",
            2459,
            0,
            "%s",
            "fileSkipIndex < g_numFileSkips");
    if (v4)
        return SV_DemoLoadHistory(g_fileTimeHistory, g_fileSkips[v4 - 1].fileEndOffset);
    else
        return SV_DemoLoadHistory(g_fileTimeHistory, 0);
}

bool __cdecl SV_ActiveHistoryIsMark(const char *name)
{
    return g_history && g_history->manual && I_stricmp(g_history->name, name) == 0;
}

int __cdecl SV_LoadHistoryForMark(const char *name)
{
    char v2; // r11
    FileMarkSkip *MarkSkip; // r11
    server_demo_history_t *v5; // r11

    v2 = (char)g_history;
    if (g_history)
        v2 = g_history->manual && I_stricmp(g_history->name, name) == 0;
    if (!v2)
    {
        if (!SV_DemoGetBuffer())
            return 0;
        MarkSkip = SV_FindMarkSkip(name);
        if (!MarkSkip)
        {
            g_history = 0;
            return 0;
        }
        if (!SV_DemoLoadHistory(g_fileMarkHistory, MarkSkip->fileOffset))
            return 0;
        v5 = g_history;
        if (!g_history->save.buf)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2504, 1, "%s", "g_history->save.buf");
            v5 = g_history;
        }
        if (!v5->cmBuf)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2505, 1, "%s", "g_history->cmBuf");
            v5 = g_history;
        }
        if (!v5->freeEntBuf)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2506, 1, "%s", "g_history->freeEntBuf");
            v5 = g_history;
        }
        if (!v5->manual)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2507, 1, "%s", "g_history->manual");
    }
    return 1;
}

void __cdecl SV_DemoGoto_f()
{
    const char *v0; // r3

    if (replay_time)
    {
        if (replay_autosave)
        {
            Dvar_SetBool(replay_time, 1);
            if (replay_autosave->current.integer > 2)
                Dvar_SetInt(replay_autosave, 2);
        }
    }
    if (sv.demo.msg.data)
    {
        if (SV_Cmd_Argc() <= 1)
            v0 = "";
        else
            v0 = SV_Cmd_Argv(1);
        if ((unsigned __int8)SV_LoadHistoryForMark(v0))
        {
            sv.demo.nextLevelSave = g_history;
            SV_DemoRestart();
        }
        else
        {
            Com_Printf(0, "Mark not found.\n");
        }
    }
    else
    {
        Com_Printf(0, "No replay.\n");
    }
}

void __cdecl SV_DemoSetNextLevelTime(int time)
{
    int v1; // r11

    if (sv.demo.nextLevelSave)
    {
        v1 = sv.demo.nextLevelSave->time;
        if (time <= v1 + 2500)
            goto LABEL_5;
    }
    if (time < 2500)
    {
        v1 = 0;
    LABEL_5:
        sv.demo.nextLevelTime = v1;
        return;
    }
    sv.demo.nextLevelTime = time;
}

void __cdecl SV_DemoBack_f()
{
    int v0; // r31
    int v1; // r31
    const char *v2; // r3
    long double v3; // fp2

    if (replay_time && replay_autosave)
    {
        Dvar_SetBool(replay_time, 1);
        if (replay_autosave->current.integer > 2)
            Dvar_SetInt(replay_autosave, 2);
    }
    if (!sv.demo.msg.data)
    {
        Com_Printf(0, "No replay.\n");
        return;
    }
    if (sv.demo.forwardMsec)
    {
        Com_Printf(0, "replay_back ignored.\n");
        return;
    }
    if (SV_Cmd_Argc() <= 1)
    {
        v0 = 50;
    LABEL_11:
        v1 = G_GetTime() - v0;
        SV_LoadHistoryForTime(v1);
        sv.demo.nextLevelSave = g_history;
        SV_DemoRestart();
        SV_DemoSetNextLevelTime(v1);
        return;
    }
    v2 = SV_Cmd_Argv(1);
    v3 = atof(v2);
    v0 = (int)(*(double *)&v3 * 1000.0);
    if (v0 > 0)
        goto LABEL_11;
    Com_Printf(0, "bad value\n");
}

void __cdecl SV_DemoForward_f()
{
    const char *v0; // r3
    long double v1; // fp2
    int v2; // r28
    int v3; // r30
    server_demo_history_t *v4; // r11
    int Time; // r3

    if (replay_time && replay_autosave)
    {
        Dvar_SetBool(replay_time, 1);
        if (replay_autosave->current.integer > 2)
            Dvar_SetInt(replay_autosave, 2);
    }
    if (!sv.demo.playing)
    {
        Com_Printf(0, "Not playing replay.\n");
        return;
    }
    if (sv.demo.forwardMsec)
    {
        Com_Printf(0, "replay_forward ignored.\n");
        return;
    }
    if (SV_Cmd_Argc() <= 1)
    {
        sv.demo.forwardMsec = 50;
        return;
    }
    v0 = SV_Cmd_Argv(1);
    v1 = atof(v0);
    v2 = (int)(*(double *)&v1 * 1000.0);
    if (v2 <= 0)
    {
        Com_Printf(0, "bad value\n");
        return;
    }
    v3 = G_GetTime() + v2;
    SV_LoadHistoryForTime(v3);
    if (g_history)
        goto LABEL_16;
    if (G_GetTime() <= 4000)
    {
    LABEL_17:
        CL_SetSkipRendering(1);
        sv.demo.forwardMsec = v2;
        return;
    }
    v4 = g_history;
    if (g_history)
    {
    LABEL_16:
        Time = G_GetTime();
        v4 = g_history;
        if (g_history->time - Time <= 4000)
            goto LABEL_17;
    }
    sv.demo.nextLevelSave = v4;
    SV_DemoRestart();
    SV_DemoSetNextLevelTime(v3);
}

void __cdecl SV_DemoFullForward_f()
{
    int v0; // r11
    const char *v1; // r3
    long double v2; // fp2

    if (replay_time && replay_autosave)
    {
        Dvar_SetBool(replay_time, 1);
        if (replay_autosave->current.integer > 2)
            Dvar_SetInt(replay_autosave, 2);
    }
    if (!sv.demo.playing)
    {
        Com_Printf(0, "Not playing replay.\n");
        return;
    }
    if (sv.demo.forwardMsec)
    {
        Com_Printf(0, "replay_forward_full ignored.\n");
        return;
    }
    if (SV_Cmd_Argc() <= 1)
    {
        v0 = 50;
    LABEL_11:
        sv.demo.forwardMsec = v0;
        return;
    }
    v1 = SV_Cmd_Argv(1);
    v2 = atof(v1);
    v0 = (int)(*(double *)&v2 * 1000.0);
    if (v0 > 0)
        goto LABEL_11;
    Com_Printf(0, "bad value\n");
}

void __cdecl SV_DemoLive_f()
{
    if (replay_time)
    {
        if (replay_autosave)
        {
            Dvar_SetBool(replay_time, 1);
            if (replay_autosave->current.integer > 2)
                Dvar_SetInt(replay_autosave, 2);
        }
    }
    if (sv.demo.playing)
        sv.demo.startLive = 1;
    else
        Com_Printf(0, "Not playing replay.\n");
}

void __cdecl SV_DemoInfo_f(int a1, int a2, int a3)
{
    FileSkip *v3; // r31
    int v4; // r29
    FileMarkSkip *v5; // r30
    int i; // r30

    v3 = g_fileSkips;
    if (g_numFileMarkSkips > 0)
    {
        if (!g_fileMarkHistory)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2726, 0, "%s", "g_fileMarkHistory");
        Com_Printf(0, "Named Marks(%d):\n", a3);
        v4 = 0;
        if (g_numFileMarkSkips > 0)
        {
            v5 = g_fileMarkSkips;
            do
            {
                Com_Printf(0, "\t'%s':%d\n", v5->name, v5->fileOffset);
                ++v4;
                ++v5;
            } while (v4 < g_numFileMarkSkips);
        }
    }
    if (g_numFileSkips > 0)
    {
        if (!g_fileTimeHistory)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2738, 0, "%s", "g_fileTimeHistory");
        Com_Printf(0, "Time Marks(%d):\n", a3);
        for (i = 0; i < g_numFileSkips; ++v3)
        {
            Com_Printf(0, "\t%d:%d\n", v3->time, v3->fileEndOffset);
            ++i;
        }
    }
}

int __cdecl SV_GetDemoStartTime()
{
    return sv.demo.startTime;
}

int __cdecl SV_GetDemoEndTime()
{
    return sv.demo.endTime;
}

int __cdecl SV_CheckAutoSaveHistory(int setTooSoon)
{
    const dvar_s *v2; // r11
    int v4; // r11

    v2 = replay_autosave;
    if (!replay_autosave)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 2780, 0, "%s", "replay_autosave");
        v2 = replay_autosave;
    }
    if (!v2->current.integer || !sv.demo.playing && !sv.demo.recording)
        return 0;
    v4 = G_GetTime() - sv.demo.autoSaveTime;
    if (v4 >= 0 && v4 <= 1000 * replay_autosave->current.integer)
        return 0;
    if (setTooSoon)
        sv.demo.autoSaveTime = G_GetTime();
    return 1;
}

void SV_DoAutoSaveHistory()
{
    if (SV_CheckAutoSaveHistory(1))
    {
        if (SV_DemoGetBuffer())
        {
            g_history->manual = 0;
            SV_FreeHistoryData(g_history);
            if (!SV_DemoSaveHistory(g_history))
            {
                if (g_history)
                {
                    SV_FreeHistoryData(g_history);
                    g_history = 0;
                }
            }
        }
        else
        {
            Com_PrintError(1, "Replay autosave failed because previous save hasn't finish.\n");
        }
    }
}

void __cdecl SV_UpdateDemo()
{
    if (sv.demo.recording)
        sv.demo.endTime = G_GetTime();
    SV_DoAutoSaveHistory();
}

void SV_DemoLive()
{
    int Time; // r3

    MSG_Truncate(&sv.demo.msg);
    if (g_history)
    {
        SV_FreeHistoryData(g_history);
        g_history = 0;
    }
    Time = G_GetTime();
    SV_TruncateHistoryTimeCache(Time);
}

void __cdecl SV_EndDemo(bool error)
{
    int Time; // r3

    if (!sv.demo.playing)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 561, 0, "%s", "sv.demo.playing");
    if (sv.demo.recording)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 562, 0, "%s", "!sv.demo.recording");
    if (error)
    {
        Com_Printf(15, "Aborted replay due to inconsistency.\n");
        Dvar_SetInt(cl_paused, 1);
    }
    else
    {
        Com_Printf(15, "End of replay.\n");
        sv.demo.recording = 1;
        if (sv.demo.startLive)
        {
            SV_DemoLive();
        }
        else
        {
            Time = G_GetTime();
            SV_TruncateHistoryTimeCache(Time);
        }
    }
    sv.demo.playing = 0;
    --sv.demo.nextFramePos;
    sv.demo.forwardMsec = 0;
    sv.clearTimeResidual = 1;
    CL_SetSkipRendering(0);
    sv.demo.startLive = 0;
    if (g_history)
    {
        SV_FreeHistoryData(g_history);
        g_history = 0;
    }
}

void SV_ReadNextDemoType()
{
    int Byte; // r3

    if (!sv.demo.playing)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 634, 0, "%s", "sv.demo.playing");
    if (sv.demo.recording)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 635, 0, "%s", "!sv.demo.recording");
    if (sv.demo.startLive)
    {
        SV_EndDemo(0);
    }
    else
    {
        Byte = MSG_ReadByte(&sv.demo.msg);
        sv.demo.readType = Byte;
        switch (Byte)
        {
        case 0:
        case 2:
        case 6:
            sv.demo.nextFramePos = MSG_ReadLong(&sv.demo.msg);
            break;
        case 1:
        case 3:
        case 4:
        case 5:
        case 7:
        case 8:
        case 10:
            --sv.demo.nextFramePos;
            break;
        default:
            SV_EndDemo(Byte != -1);
            break;
        }
    }
}

bool __cdecl SV_InitReadDemo(int *randomSeed)
{
    server_demo_history_t *nextLevelSave; // r11
    server_demo_history_t *v3; // r11
    int v4; // r11

    ProfLoad_Begin("SV_InitReadDemo");
    if (!randomSeed)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1118, 0, "%s", "randomSeed");
    if (!sv.demo.nextLevelplaying)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1120, 0, "%s", "sv.demo.nextLevelplaying");
    sv.demo.nextLevelplaying = 0;
    sv.demo.playing = 1;
    MSG_BeginReading(&sv.demo.msg);
    nextLevelSave = sv.demo.nextLevelSave;
    if (sv.demo.nextLevelSave)
    {
        if (!sv.demo.nextLevelSave->freeEntBuf)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp",
                1142,
                0,
                "%s",
                "sv.demo.nextLevelSave->freeEntBuf");
            nextLevelSave = sv.demo.nextLevelSave;
        }
        G_LoadFreeEntities(nextLevelSave->freeEntBuf);
        v3 = sv.demo.nextLevelSave;
        if (!sv.demo.nextLevelSave->cmBuf)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp",
                1145,
                0,
                "%s",
                "sv.demo.nextLevelSave->cmBuf");
            v3 = sv.demo.nextLevelSave;
        }
        CM_LoadWorld(v3->cmBuf);
        G_UpdateAllEntities();
        CM_UnlockTree();
        *randomSeed = sv.demo.nextLevelSave->randomSeed;
        memcpy(&svs.clients->lastUsercmd, &sv.demo.nextLevelSave->lastUsercmd, sizeof(svs.clients->lastUsercmd));
        sv.demo.nextFramePos = sv.demo.nextLevelSave->nextFramePos;
        sv.demo.msg.readcount = sv.demo.nextLevelSave->msgReadcount;
        sv.demo.msg.bit = sv.demo.nextLevelSave->msgBit;
        sv.demo.readType = sv.demo.nextLevelSave->readType;
        if (sv.demo.readType == 9)
            SV_ReadNextDemoType();
    }
    else
    {
        *randomSeed = MSG_ReadLong(&sv.demo.msg);
        sv.demo.nextFramePos = 0;
        SV_ReadNextDemoType();
        CM_LinkWorld();
        G_UpdateAllEntities();
    }
    if (sv.demo.nextLevelTime)
    {
        v4 = sv.demo.nextLevelTime - G_GetTime();
        sv.demo.forwardMsec = v4;
        if (v4 < 0)
        {
            v4 = 0;
            sv.demo.forwardMsec = 0;
        }
        sv.demo.nextLevelTime = 0;
        if (v4)
            CL_SetSkipRendering(1);
    }
    ProfLoad_End();
    return sv.demo.playing;
}

bool __cdecl SV_InitDemo(int *randomSeed)
{
    sv.demo.autoSaveTime = G_GetTime();
    if (sv.demo.nextLevelplaying)
        return SV_InitReadDemo(randomSeed);
    SV_InitWriteDemo(*randomSeed);
    return 0;
}

bool __cdecl SV_ReadPacket(int framePos)
{
    bool result; // r3
    const dvar_s *Var; // r30
    usercmd_s v4; // [sp+50h] [-480h] BYREF
    char v5[1088]; // [sp+90h] [-440h] BYREF

    result = sv.demo.playing;
    if (sv.demo.playing && framePos == sv.demo.nextFramePos)
    {
        while (1)
        {
            if (!result)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1290, 0, "%s", "sv.demo.playing");
            if (!sv.demo.readType)
            {
                MSG_ReadDeltaUsercmd(&sv.demo.msg, &svs.clients->lastUsercmd, &v4);
                SV_ReadNextDemoType();
                SV_ClientThink(&v4);
                CL_ClearClientThinkPacket();
                goto LABEL_18;
            }
            if (sv.demo.readType == 2)
                break;
            if (sv.demo.readType != 6)
                goto LABEL_15;
            if (!MSG_ReadString(&sv.demo.msg, v5, 1024))
                goto LABEL_15;
            Var = Dvar_FindVar(v5);
            if (!Var || !MSG_ReadString(&sv.demo.msg, v5, 1024))
                goto LABEL_15;
            SV_ReadNextDemoType();
            Dvar_SetFromString((dvar_s*)Var, v5);
        LABEL_18:
            result = sv.demo.playing;
            if (framePos != sv.demo.nextFramePos)
                return result;
        }
        if (MSG_ReadString(&sv.demo.msg, v5, 1024))
        {
            SV_ReadNextDemoType();
            SV_ExecuteClientCommand(v5);
            CL_ClearClientCommandPacket();
            goto LABEL_18;
        }
    LABEL_15:
        SV_EndDemo(1);
        goto LABEL_18;
    }
    return result;
}

float __cdecl SV_DemoFxVisibility()
{
    double v0; // fp1
    double Float; // fp31

    if (!sv.demo.playing)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1425, 0, "%s", "sv.demo.playing");
    if (sv.demo.readType == 1)
    {
        Float = MSG_ReadFloat(&sv.demo.msg);
        SV_ReadNextDemoType();
        v0 = Float;
    }
    else
    {
        SV_EndDemo(1);
        v0 = 0.0;
    }
    return *((float *)&v0 + 1);
}

int __cdecl SV_DemoCheatsOk()
{
    int Long; // r31

    if (!sv.demo.playing)
        return 0;
    if (sv.demo.readType != 7)
    {
        SV_EndDemo(1);
        return 0;
    }
    Long = MSG_ReadLong(&sv.demo.msg);
    SV_ReadNextDemoType();
    return Long;
}

bool __cdecl SV_DemoIsRecentlyLoaded()
{
    bool v1; // r31

    if (!sv.demo.playing)
        return 0;
    if (sv.demo.readType != 10)
    {
        SV_EndDemo(1);
        return 0;
    }
    v1 = MSG_ReadByte(&sv.demo.msg) != 0;
    SV_ReadNextDemoType();
    return v1;
}

char *__cdecl SV_Demo_Dvar_GetVariantString()
{
    char v1[1032]; // [sp+50h] [-420h] BYREF

    if (!sv.demo.playing)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1552, 0, "%s", "sv.demo.playing");
    if (sv.demo.readType == 3 && MSG_ReadString(&sv.demo.msg, v1, 1024))
    {
        SV_ReadNextDemoType();
        return va("%s", v1);
    }
    else
    {
        SV_EndDemo(1);
        return "";
    }
}

int __cdecl SV_DemoButtonPressed()
{
    int Byte; // r31

    if (!sv.demo.playing)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_demo.cpp", 1601, 0, "%s", "sv.demo.playing");
    if (sv.demo.readType == 8)
    {
        Byte = MSG_ReadByte(&sv.demo.msg);
        SV_ReadNextDemoType();
        return Byte;
    }
    else
    {
        SV_EndDemo(1);
        return 0;
    }
}

