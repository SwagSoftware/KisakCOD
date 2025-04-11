#include "r_cinematic.h"

#include <universal/q_shared.h>
#include <universal/assertive.h>
#include <qcommon/threads.h>
#include <universal/com_memory.h>
#include "r_init.h"
#include <msslib/mss.h>
#include <sound/snd_local.h>
#include <win32/win_local.h>
#include "rb_state.h"
#include "r_image.h"
#include <database/database.h>
#include <qcommon/com_fileaccess.h>

CinematicGlob cinematicGlob;
bool g_cinematicThreadInitialized;
CinematicThreadState g_cinematicThreadState;

int __cdecl CinematicHunk_Alloc(CinematicHunk* hunk, int size)
{
    const char* v2; // eax
    char* alloced; // [esp+0h] [ebp-4h]

    if (!hunk->base)
        MyAssertHandler(".\\r_cinematic.cpp", 366, 0, "%s", "hunk->base");
    if (!hunk->atFront)
        MyAssertHandler(".\\r_cinematic.cpp", 367, 0, "%s", "hunk->atFront");
    if (!hunk->atBack)
        MyAssertHandler(".\\r_cinematic.cpp", 368, 0, "%s", "hunk->atBack");
    if (!hunk->end)
        MyAssertHandler(".\\r_cinematic.cpp", 369, 0, "%s", "hunk->end");
    if (size < 0)
        MyAssertHandler(".\\r_cinematic.cpp", 370, 0, "%s", "size >= 0");
    alloced = (char*)hunk->atFront;
    hunk->atFront = &alloced[size];
    if (hunk->atFront <= hunk->atBack)
        return (int)alloced;
    if (!alwaysfails)
    {
        v2 = va("CinematicHunk_Alloc failed: 0x%08x 0x%08x 0x%08x\n", hunk->atFront, size, hunk->atBack);
        MyAssertHandler(".\\r_cinematic.cpp", 376, 0, v2);
    }
    return -1;
}

void R_Cinematic_RelinquishIO()
{
    if (!cinematicGlob.hasFileIO)
        MyAssertHandler(".\\r_cinematic.cpp", 1030, 0, "%s", "cinematicGlob.hasFileIO");
    if (!cinematicGlob.bink)
        MyAssertHandler(".\\r_cinematic.cpp", 1031, 0, "%s", "cinematicGlob.bink");
    BinkControlBackgroundIO(cinematicGlob.bink, 1);
    R_Cinematic_CheckBinkError();
    Sys_ResumeDatabaseThread(THREAD_OWNER_CINEMATICS);
    cinematicGlob.hasFileIO = 0;
}

void R_Cinematic_CheckBinkError()
{
    const char *v0; // eax
    const char *binkError; // [esp+0h] [ebp-4h]

    binkError = (const char *)BinkGetError();
    if (binkError)
    {
        if (*binkError)
        {
            v0 = va("BinkGetError(): \"%s\"", binkError);
            MyAssertHandler(".\\r_cinematic.cpp", 282, 0, "%s\n\t%s", "!binkError || binkError[0] == '\\0'", v0);
        }
    }
}

void __cdecl R_Cinematic_InitBinkVolumes()
{
    int v0; // [esp+0h] [ebp-50h]
    int v1; // [esp+28h] [ebp-28h]
    int volumes[8]; // [esp+30h] [ebp-20h] BYREF

    if ((int)(__int64)(cinematicGlob.playbackVolume * 32768.0) < 0x8000)
        v1 = (__int64)(cinematicGlob.playbackVolume * 32768.0);
    else
        v1 = 0x8000;
    if (v1 > 0)
        v0 = v1;
    else
        v0 = 0;
    memset(&volumes[2], 0, 24);
    volumes[0] = v0;
    volumes[1] = v0;
    BinkSetMixBinVolumes(cinematicGlob.bink, 0, 0, volumes, 8);
    R_Cinematic_CheckBinkError();
    volumes[0] = 0;
    volumes[1] = 0;
    memset(&volumes[3], 0, 20);
    volumes[2] = v0;
    BinkSetMixBinVolumes(cinematicGlob.bink, 1, 0, volumes, 8);
    R_Cinematic_CheckBinkError();
    memset(volumes, 0, 12);
    memset(&volumes[4], 0, 16);
    volumes[3] = v0;
    BinkSetMixBinVolumes(cinematicGlob.bink, 2, 0, volumes, 8);
    R_Cinematic_CheckBinkError();
    memset(volumes, 0, 16);
    volumes[6] = 0;
    volumes[7] = 0;
    volumes[4] = v0;
    volumes[5] = v0;
    BinkSetMixBinVolumes(cinematicGlob.bink, 3, 0, volumes, 8);
    R_Cinematic_CheckBinkError();
    memset(volumes, 0, 24);
    volumes[6] = v0;
    volumes[7] = v0;
    BinkSetMixBinVolumes(cinematicGlob.bink, 4, 0, volumes, 8);
    R_Cinematic_CheckBinkError();
}

void __cdecl R_Cinematic_Init()
{
    if (g_cinematicThreadInitialized && g_cinematicThreadState != CINEMATIC_THREAD_STATE_TO_HOST_BETWEEN_UPDATES)
        MyAssertHandler(
            ".\\r_cinematic.cpp",
            1874,
            0,
            "%s",
            "!g_cinematicThreadInitialized || g_cinematicThreadState == CINEMATIC_THREAD_STATE_TO_HOST_BETWEEN_UPDATES");
    memset((unsigned __int8 *)&cinematicGlob, 0, sizeof(cinematicGlob));
    cinematicGlob.activeImageFrame = -1;
    R_Cinematic_ReserveMemory();
    if (cinematicGlob.currentPaused)
        MyAssertHandler(".\\r_cinematic.cpp", 1881, 0, "%s", "cinematicGlob.currentPaused == CINEMATIC_NOT_PAUSED");
    if (cinematicGlob.targetPaused)
        MyAssertHandler(".\\r_cinematic.cpp", 1882, 0, "%s", "cinematicGlob.targetPaused == CINEMATIC_NOT_PAUSED");
    if (!g_cinematicThreadInitialized)
    {
        g_cinematicThreadInitialized = 1;
        g_cinematicThreadState = CINEMATIC_THREAD_STATE_TO_HOST_BETWEEN_UPDATES;
        Sys_SpawnCinematicsThread((void(__cdecl *)(unsigned int))R_Cinematic_Thread);
        CL_ResetStats_f();
        cinematicGlob.atHighPriority = 1;
    }
}

unsigned int *R_Cinematic_ReserveMemory()
{
    unsigned int *result; // eax

    result = (unsigned int*)Z_Malloc((int)&cls.globalServers[9796].adr.ipx[2], "R_Cinematic_ReserveMemory", 18);
    cinematicGlob.memPool = result;
    return result;
}

void __cdecl  R_Cinematic_Thread(unsigned int threadContext)
{
    if (threadContext != 4)
        MyAssertHandler(
            ".\\r_cinematic.cpp",
            1815,
            0,
            "threadContext == THREAD_CONTEXT_CINEMATIC\n\t%i, %i",
            threadContext,
            4);
    while (1)
    {
        R_CinematicThread_WaitForHostEvent();
        R_Cinematic_UpdateFrame_Core2();
        g_cinematicThreadState = CINEMATIC_THREAD_STATE_TO_HOST_BETWEEN_UPDATES;
        Sys_ResetCinematicsHostOutstandingRequestEvent();
        Sys_SetCinematicsThreadOutstandingRequestEvent();
    }
}

void R_Cinematic_UpdateFrame_Core2()
{
    bool localTargetChanged; // [esp+3h] [ebp-109h]
    char localTargetCinematic[256]; // [esp+4h] [ebp-108h] BYREF
    unsigned int localTargetPlaybackFlags; // [esp+108h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_CINEMATIC_TARGET_CHANGE);
    localTargetChanged = cinematicGlob.targetCinematicChanged;
    if (cinematicGlob.targetCinematicChanged)
    {
        I_strncpyz(localTargetCinematic, cinematicGlob.targetCinematicName, 256);
        cinematicGlob.targetCinematicChanged = 0;
        localTargetPlaybackFlags = cinematicGlob.playbackFlags;
    }
    else
    {
        localTargetCinematic[0] = 0;
        localTargetPlaybackFlags = 0;
    }
    Sys_LeaveCriticalSection(CRITSECT_CINEMATIC_TARGET_CHANGE);
    R_Cinematic_UpdateFrame_Core(localTargetChanged, localTargetCinematic, localTargetPlaybackFlags);
}

void __cdecl R_Cinematic_UpdateFrame_Core(
    bool localTargetChanged,
    char *localTargetCinematic,
    unsigned int localPlaybackFlags)
{
    bool isCinematicBeingPlayed; // [esp+3h] [ebp-1h]

    if (!localTargetChanged && localPlaybackFlags)
        MyAssertHandler(".\\r_cinematic.cpp", 1713, 0, "%s", "localTargetChanged || !localPlaybackFlags");
    if (!localTargetChanged && localTargetCinematic && *localTargetCinematic)
        MyAssertHandler(
            ".\\r_cinematic.cpp",
            1714,
            0,
            "%s",
            "localTargetChanged || !localTargetCinematic || !localTargetCinematic[0]");
    cinematicGlob.underrun = 0;
    if (localTargetChanged)
    {
        if (cinematicGlob.currentCinematicName[0])
            R_Cinematic_StopPlayback_Now();
        if (*localTargetCinematic && R_Cinematic_StartPlayback_Now(localTargetCinematic, localPlaybackFlags))
            I_strncpyz(cinematicGlob.currentCinematicName, localTargetCinematic, 256);
        else
            cinematicGlob.currentCinematicName[0] = 0;
    }
    isCinematicBeingPlayed = cinematicGlob.currentCinematicName[0] != 0;
    if (cinematicGlob.currentCinematicName[0])
    {
        cinematicGlob.framesStopped = 0;
    }
    else
    {
        if (cinematicGlob.activeImageFrame != -1)
            MyAssertHandler(
                ".\\r_cinematic.cpp",
                1733,
                0,
                "%s",
                "cinematicGlob.activeImageFrame == CINEMATIC_INVALID_IMAGE_FRAME");
        if (R_Cinematic_AreHunksOpen())
        {
            if (++cinematicGlob.framesStopped >= 5)
                R_Cinematic_HunksClose();
        }
    }
    if (isCinematicBeingPlayed)
    {
        if (!cinematicGlob.bink)
            MyAssertHandler(".\\r_cinematic.cpp", 1749, 0, "%s", "cinematicGlob.bink");
        if (!R_Cinematic_Advance())
            cinematicGlob.cinematicFinished = 1;
    }
}

char __cdecl R_Cinematic_AreHunksOpen()
{
    if (CinematicHunk_IsOpen(&cinematicGlob.masterHunk))
    {
        if (!CinematicHunk_IsOpen(&cinematicGlob.binkHunk))
            MyAssertHandler(".\\r_cinematic.cpp", 521, 0, "%s", "CinematicHunk_IsOpen( &cinematicGlob.binkHunk )");
        if (!CinematicHunk_IsOpen(&cinematicGlob.residentHunk))
            MyAssertHandler(".\\r_cinematic.cpp", 522, 0, "%s", "CinematicHunk_IsOpen( &cinematicGlob.residentHunk )");
        return 1;
    }
    else
    {
        if (CinematicHunk_IsOpen(&cinematicGlob.binkHunk))
            MyAssertHandler(".\\r_cinematic.cpp", 531, 0, "%s", "!CinematicHunk_IsOpen( &cinematicGlob.binkHunk )");
        if (CinematicHunk_IsOpen(&cinematicGlob.residentHunk))
            MyAssertHandler(".\\r_cinematic.cpp", 532, 0, "%s", "!CinematicHunk_IsOpen( &cinematicGlob.residentHunk )");
        return 0;
    }
}

char __cdecl CinematicHunk_IsOpen(CinematicHunk *hunk)
{
    if (hunk->base)
    {
        if (!hunk->atFront)
            MyAssertHandler(".\\r_cinematic.cpp", 317, 0, "%s", "hunk->atFront");
        if (!hunk->atBack)
            MyAssertHandler(".\\r_cinematic.cpp", 318, 0, "%s", "hunk->atBack");
        if (!hunk->end)
            MyAssertHandler(".\\r_cinematic.cpp", 319, 0, "%s", "hunk->end");
        return 1;
    }
    else
    {
        if (hunk->atFront)
            MyAssertHandler(".\\r_cinematic.cpp", 324, 0, "%s", "!hunk->atFront");
        if (hunk->atBack)
            MyAssertHandler(".\\r_cinematic.cpp", 325, 0, "%s", "!hunk->atBack");
        if (hunk->end)
            MyAssertHandler(".\\r_cinematic.cpp", 326, 0, "%s", "!hunk->end");
        return 0;
    }
}

void R_Cinematic_HunksClose()
{
    CinematicHunk_Close(&cinematicGlob.masterHunk);
    CinematicHunk_Close(&cinematicGlob.binkHunk);
    CinematicHunk_Close(&cinematicGlob.residentHunk);
}

void __cdecl CinematicHunk_Close(CinematicHunk *hunk)
{
    if (!hunk->base)
        MyAssertHandler(".\\r_cinematic.cpp", 302, 0, "%s", "hunk->base");
    if (!hunk->atFront)
        MyAssertHandler(".\\r_cinematic.cpp", 303, 0, "%s", "hunk->atFront");
    if (!hunk->atBack)
        MyAssertHandler(".\\r_cinematic.cpp", 304, 0, "%s", "hunk->atBack");
    if (!hunk->end)
        MyAssertHandler(".\\r_cinematic.cpp", 305, 0, "%s", "hunk->end");
    hunk->base = 0;
    hunk->atFront = 0;
    hunk->atBack = 0;
    hunk->end = 0;
}

char __cdecl R_Cinematic_Advance()
{
    CinematicEnum targetPaused; // [esp+A0h] [ebp-44h]
    BINKREALTIME binkRealtime; // [esp+A4h] [ebp-40h] BYREF
    unsigned int percentageFull; // [esp+DCh] [ebp-8h]
    int skipped; // [esp+E0h] [ebp-4h]

    //Profile_Begin(145);
    targetPaused = cinematicGlob.targetPaused;
    if (cinematicGlob.targetPaused > (unsigned int)CINEMATIC_PAUSED)
        MyAssertHandler(
            ".\\r_cinematic.cpp",
            1078,
            0,
            "%s\n\t(targetPaused) = %i",
            "(targetPaused == CINEMATIC_PAUSED || targetPaused == CINEMATIC_NOT_PAUSED)",
            cinematicGlob.targetPaused);
    if (cinematicGlob.currentPaused > (unsigned int)CINEMATIC_PAUSED)
        MyAssertHandler(
            ".\\r_cinematic.cpp",
            1079,
            0,
            "%s\n\t(cinematicGlob.currentPaused) = %i",
            "(cinematicGlob.currentPaused == CINEMATIC_PAUSED || cinematicGlob.currentPaused == CINEMATIC_NOT_PAUSED)",
            cinematicGlob.currentPaused);
    if (!cinematicGlob.bink)
        MyAssertHandler(".\\r_cinematic.cpp", 1081, 0, "%s", "cinematicGlob.bink");
    if (cinematicGlob.underrun)
        MyAssertHandler(".\\r_cinematic.cpp", 1083, 0, "%s", "!cinematicGlob.underrun");
    percentageFull = R_Cinematic_GetPercentageFull();
    if (percentageFull < 0x28 && (cinematicGlob.playbackFlags & 1) == 0)
    {
        cinematicGlob.underrun = 1;
        targetPaused = CINEMATIC_PAUSED;
    }
    if (targetPaused != cinematicGlob.currentPaused)
    {
        BinkPause(cinematicGlob.bink, targetPaused == CINEMATIC_PAUSED);
        R_Cinematic_CheckBinkError();
        cinematicGlob.currentPaused = targetPaused;
    }
    //Profile_Begin(148);
    if (BinkWait(cinematicGlob.bink) || cinematicGlob.underrun)
    {
        //Profile_EndInternal(0);
    }
    else
    {
        //Profile_EndInternal(0);
        Lock_Bink_textures(&cinematicGlob.binkTextureSet);
        //Profile_Begin(146);
        skipped = BinkDoFrame(cinematicGlob.bink);
        //Profile_EndInternal(0);
        if (!skipped)
        {
            if (cinematicGlob.binkTextureSet.bink_buffers.FrameNum > 1)
                MyAssertHandler(
                    ".\\r_cinematic.cpp",
                    1127,
                    0,
                    "%s\n\t(cinematicGlob.binkTextureSet.bink_buffers.FrameNum) = %i",
                    "(cinematicGlob.binkTextureSet.bink_buffers.FrameNum == 0 || cinematicGlob.binkTextureSet.bink_buffers.FrameNum == 1)",
                    cinematicGlob.binkTextureSet.bink_buffers.FrameNum);
            cinematicGlob.activeImageFrame = cinematicGlob.binkTextureSet.bink_buffers.FrameNum;
            cinematicGlob.activeImageFrameTextureSet = cinematicGlob.activeTextureSet;
        }
        Unlock_Bink_textures(dx.device, &cinematicGlob.binkTextureSet, cinematicGlob.bink);
        if ((cinematicGlob.playbackFlags & 2) != 0 || cinematicGlob.bink->FrameNum != cinematicGlob.bink->Frames)
        {
            //Profile_Begin(147);
            BinkNextFrame(cinematicGlob.bink);
            //Profile_EndInternal(0);
        }
    }
    R_Cinematic_CheckBinkError();
    if ((cinematicGlob.playbackFlags & 2) != 0 || cinematicGlob.bink->FrameNum != cinematicGlob.bink->Frames)
    {
        if (!cinematicGlob.bink)
            MyAssertHandler(".\\r_cinematic.cpp", 1167, 0, "%s", "cinematicGlob.bink");
        BinkGetRealtime(cinematicGlob.bink, &binkRealtime, 0);
        R_Cinematic_UpdateTimeInMsec(&binkRealtime);
        if ((cinematicGlob.playbackFlags & 8) != 0)
        {
            percentageFull = 100;
        }
        else if (binkRealtime.ReadBufferUsed < binkRealtime.ReadBufferSize)
        {
            percentageFull = 100 * binkRealtime.ReadBufferUsed / cinematicGlob.binkIOSize;
        }
        else
        {
            percentageFull = 100;
        }
        if (cinematicGlob.hasFileIO)
        {
            if (percentageFull > 0x5F)
                R_Cinematic_RelinquishIO();
        }
        else if (percentageFull < 0x32)
        {
            if ((cinematicGlob.playbackFlags & 8) != 0)
                MyAssertHandler(
                    ".\\r_cinematic.cpp",
                    1181,
                    0,
                    "%s",
                    "!(cinematicGlob.playbackFlags & CINEMATIC_PLAYBACKFLAGS_MEMORY_RESIDENT)");
            R_Cinematic_SeizeIO();
        }
        //Profile_EndInternal(0);
        return 1;
    }
    else
    {
        if (cinematicGlob.hasFileIO)
            R_Cinematic_RelinquishIO();
        //Profile_EndInternal(0);
        return 0;
    }
}

unsigned int __cdecl R_Cinematic_GetPercentageFull()
{
    BINKREALTIME binkRealtime; // [esp+4h] [ebp-38h] BYREF

    if (!cinematicGlob.bink)
        MyAssertHandler(".\\r_cinematic.cpp", 1009, 0, "%s", "cinematicGlob.bink");
    BinkGetRealtime(cinematicGlob.bink, &binkRealtime, 0);
    if ((cinematicGlob.playbackFlags & 8) != 0)
        return 100;
    if (binkRealtime.ReadBufferUsed < binkRealtime.ReadBufferSize)
        return 100 * binkRealtime.ReadBufferUsed / cinematicGlob.binkIOSize;
    return 100;
}

void R_Cinematic_SeizeIO()
{
    if (cinematicGlob.hasFileIO)
        MyAssertHandler(".\\r_cinematic.cpp", 1017, 0, "%s", "!cinematicGlob.hasFileIO");
    if (!cinematicGlob.bink)
        MyAssertHandler(".\\r_cinematic.cpp", 1018, 0, "%s", "cinematicGlob.bink");
    BinkControlBackgroundIO(cinematicGlob.bink, 2);
    R_Cinematic_CheckBinkError();
    Sys_SuspendDatabaseThread(THREAD_OWNER_CINEMATICS);
    cinematicGlob.hasFileIO = 1;
}

void __cdecl R_Cinematic_UpdateTimeInMsec(const BINKREALTIME *binkRealtime)
{
    const char *v1; // eax
    unsigned __int64 frameNum; // [esp+8h] [ebp-20h]
    unsigned __int64 frameRateDiv; // [esp+10h] [ebp-18h]
    unsigned __int64 timeInMsec; // [esp+18h] [ebp-10h]
    unsigned __int64 frameRate; // [esp+20h] [ebp-8h]

    frameNum = binkRealtime->FrameNum;
    if ((unsigned int)frameNum < 0x80000000)
    {
        frameRateDiv = binkRealtime->FrameRateDiv;
        frameRate = binkRealtime->FrameRate;
        timeInMsec = 1000 * frameNum * frameRateDiv / frameRate;
        cinematicGlob.timeInMsec = timeInMsec;
        if (timeInMsec != (unsigned int)timeInMsec)
        {
            v1 = va(
                "%08x:%08x, %08x:%08x, %08x:%08x, %08x:%08x",
                HIDWORD(timeInMsec),
                (unsigned int)timeInMsec,
                HIDWORD(frameNum),
                (unsigned int)frameNum,
                HIDWORD(frameRate),
                (unsigned int)frameRate,
                HIDWORD(frameRateDiv),
                (unsigned int)frameRateDiv);
            MyAssertHandler(".\\r_cinematic.cpp", 1063, 0, "%s\n\t%s", "cinematicGlob.timeInMsec == timeInMsec", v1);
        }
    }
    else
    {
        cinematicGlob.timeInMsec = 0;
    }
}

void R_Cinematic_StopPlayback_Now()
{
    cinematicGlob.activeImageFrame = -1;
    if (cinematicGlob.hasFileIO)
        R_Cinematic_RelinquishIO();
    BinkClose(cinematicGlob.bink);
    cinematicGlob.bink = 0;
    CinematicHunk_Reset(&cinematicGlob.binkHunk);
}

void __cdecl CinematicHunk_Reset(CinematicHunk *hunk)
{
    if (!hunk->base)
        MyAssertHandler(".\\r_cinematic.cpp", 334, 0, "%s", "hunk->base");
    if (!hunk->atFront)
        MyAssertHandler(".\\r_cinematic.cpp", 335, 0, "%s", "hunk->atFront");
    if (!hunk->atBack)
        MyAssertHandler(".\\r_cinematic.cpp", 336, 0, "%s", "hunk->atBack");
    if (!hunk->end)
        MyAssertHandler(".\\r_cinematic.cpp", 337, 0, "%s", "hunk->end");
    hunk->atFront = hunk->base;
    hunk->atBack = hunk->end;
}

char __cdecl R_Cinematic_StartPlayback_Now(const char *filename, unsigned int playbackFlags)
{
    _DIG_DRIVER *Driver; // eax
    unsigned int TrackIDsToPlay[5]; // [esp+1Ch] [ebp-9Ch] BYREF
    char errText[132]; // [esp+30h] [ebp-88h] BYREF

    TrackIDsToPlay[0] = 0;
    TrackIDsToPlay[1] = 1;
    TrackIDsToPlay[2] = 2;
    TrackIDsToPlay[3] = 3;
    TrackIDsToPlay[4] = 4;
    if ((playbackFlags & 4) == 0 || cinematicGlob.atHighPriority)
    {
        if ((playbackFlags & 4) == 0 && cinematicGlob.atHighPriority)
        {
            CL_ResetStats_f();
            cinematicGlob.atHighPriority = 0;
        }
    }
    else
    {
        CL_ResetStats_f();
        cinematicGlob.atHighPriority = 1;
    }
    if (cinematicGlob.bink)
        MyAssertHandler(".\\r_cinematic.cpp", 1623, 0, "%s", "!cinematicGlob.bink");
    cinematicGlob.activeImageFrame = -1;
    cinematicGlob.activeTextureSet ^= 1u;
    if (R_Cinematic_AreHunksOpen())
        R_Cinematic_HunksReset(cinematicGlob.activeTextureSet, playbackFlags);
    else
        R_Cinematic_HunksOpen(cinematicGlob.activeTextureSet, playbackFlags);
    if (!CinematicHunk_IsEmpty(&cinematicGlob.binkHunk))
        MyAssertHandler(".\\r_cinematic.cpp", 1632, 0, "%s", "CinematicHunk_IsEmpty( &cinematicGlob.binkHunk )");
    R_Cinematic_CheckBinkError();
    BinkSetMemory(R_Cinematic_Bink_Alloc, R_Cinematic_Bink_Free);
    R_Cinematic_CheckBinkError();
    Driver = MSS_GetDriver();
    BinkSetSoundSystem(BinkOpenMiles, (unsigned int)Driver);
    R_Cinematic_CheckBinkError();
    BinkSetSoundTrack(5, TrackIDsToPlay);
    R_Cinematic_CheckBinkError();
    cinematicGlob.timeInMsec = 0;
    errText[0] = 0;
    if (R_Cinematic_BinkOpen(filename, playbackFlags, errText, 0x80u)
        || (Com_PrintWarning(8, "R_Cinematic_BinkOpen '%s' failed: %s; trying default.\n", filename, errText),
            CinematicHunk_Reset(&cinematicGlob.binkHunk),
            errText[0] = 0,
            R_Cinematic_BinkOpen("default", playbackFlags, errText, 0x80u)))
    {
        R_Cinematic_CheckBinkError();
        if (!cinematicGlob.bink)
            MyAssertHandler(".\\r_cinematic.cpp", 1680, 0, "%s", "cinematicGlob.bink");
        R_Cinematic_InitBinkVolumes();
        memset((unsigned __int8 *)&cinematicGlob.binkTextureSet, 0, sizeof(cinematicGlob.binkTextureSet));
        BinkGetFrameBuffersInfo(cinematicGlob.bink, &cinematicGlob.binkTextureSet.bink_buffers);
        R_Cinematic_CheckBinkError();
        R_Cinematic_InitBinkTextures();
        BinkRegisterFrameBuffers(cinematicGlob.bink, &cinematicGlob.binkTextureSet.bink_buffers);
        R_Cinematic_CheckBinkError();
        cinematicGlob.currentPaused = CINEMATIC_NOT_PAUSED;
        return 1;
    }
    else
    {
        Com_PrintWarning(8, "R_Cinematic_BinkOpen '%s' failed: %s; not playing movie.\n", "default", errText);
        if (cinematicGlob.activeImageFrame != -1)
            MyAssertHandler(
                ".\\r_cinematic.cpp",
                1662,
                0,
                "%s",
                "cinematicGlob.activeImageFrame == CINEMATIC_INVALID_IMAGE_FRAME");
        CinematicHunk_Reset(&cinematicGlob.binkHunk);
        cinematicGlob.cinematicFinished = 1;
        RB_UnbindAllImages();
        gfxCmdBufInput.codeImages[22] = rgp.blackImage;
        gfxCmdBufInput.codeImages[23] = rgp.grayImage;
        gfxCmdBufInput.codeImages[24] = rgp.grayImage;
        gfxCmdBufInput.codeImages[25] = rgp.blackImage;
        if (!rgp.blackImage)
            MyAssertHandler(".\\r_cinematic.cpp", 1670, 0, "%s", "gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_Y]");
        if (!gfxCmdBufInput.codeImages[23])
            MyAssertHandler(".\\r_cinematic.cpp", 1671, 0, "%s", "gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_CR]");
        if (!gfxCmdBufInput.codeImages[24])
            MyAssertHandler(".\\r_cinematic.cpp", 1672, 0, "%s", "gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_CB]");
        if (!gfxCmdBufInput.codeImages[25])
            MyAssertHandler(".\\r_cinematic.cpp", 1673, 0, "%s", "gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_A]");
        return 0;
    }
}

bool __cdecl CinematicHunk_IsEmpty(CinematicHunk *hunk)
{
    if (!hunk->base)
        MyAssertHandler(".\\r_cinematic.cpp", 345, 0, "%s", "hunk->base");
    if (!hunk->atFront)
        MyAssertHandler(".\\r_cinematic.cpp", 346, 0, "%s", "hunk->atFront");
    if (!hunk->atBack)
        MyAssertHandler(".\\r_cinematic.cpp", 347, 0, "%s", "hunk->atBack");
    if (!hunk->end)
        MyAssertHandler(".\\r_cinematic.cpp", 348, 0, "%s", "hunk->end");
    return hunk->atFront == hunk->base && hunk->atBack == hunk->end;
}

void __cdecl R_Cinematic_HunksOpen(int activeTexture, char playbackFlags)
{
    CinematicHunk_Open(&cinematicGlob.masterHunk, (char *)cinematicGlob.memPool, (int)&cls.globalServers[9796].adr.ipx[2]);
    R_Cinematic_HunksAllocate(activeTexture, playbackFlags);
}

void __cdecl CinematicHunk_Open(CinematicHunk *hunk, char *memory, int size)
{
    if (hunk->base)
        MyAssertHandler(".\\r_cinematic.cpp", 289, 0, "%s", "!hunk->base");
    if (hunk->atFront)
        MyAssertHandler(".\\r_cinematic.cpp", 290, 0, "%s", "!hunk->atFront");
    if (hunk->atBack)
        MyAssertHandler(".\\r_cinematic.cpp", 291, 0, "%s", "!hunk->atBack");
    if (hunk->end)
        MyAssertHandler(".\\r_cinematic.cpp", 292, 0, "%s", "!hunk->end");
    hunk->base = memory;
    hunk->atFront = memory;
    hunk->end = &memory[size];
    hunk->atBack = hunk->end;
}

void __cdecl R_Cinematic_HunksAllocate(int activeTexture, char playbackFlags)
{
    char *residentBufferBase; // [esp+8h] [ebp-14h]
    char *newResidentBufferSize; // [esp+Ch] [ebp-10h]
    int newBinkBufferSize; // [esp+10h] [ebp-Ch]
    char *binkBufferBase; // [esp+14h] [ebp-8h]

    newBinkBufferSize = (int)cls.globalServers[-21346].adr.ip;
    if ((int)cls.globalServers[-21346].adr.ip <= 0)
        MyAssertHandler(
            ".\\r_cinematic.cpp",
            440,
            0,
            "%s\n\t(newBinkBufferSize) = %i",
            "(newBinkBufferSize > 0)",
            newBinkBufferSize);
    newResidentBufferSize = 0;
    if ((playbackFlags & 8) != 0 && (playbackFlags & 0x20) == 0)
    {
        newResidentBufferSize = &cls.globalServers[-31974].mapName[10];
        if ((int)&cls.globalServers[-31974].mapName[10] <= 0)
            MyAssertHandler(
                ".\\r_cinematic.cpp",
                446,
                0,
                "%s\n\t(newResidentBufferSize) = %i",
                "(newResidentBufferSize > 0)",
                newResidentBufferSize);
        newBinkBufferSize = 1572864;
    }
    binkBufferBase = (char *)CinematicHunk_Alloc(&cinematicGlob.masterHunk, 0x435800);
    residentBufferBase = (char *)CinematicHunk_Alloc(&cinematicGlob.masterHunk, (int)newResidentBufferSize);
    CinematicHunk_Open(&cinematicGlob.binkHunk, binkBufferBase, newBinkBufferSize);
    CinematicHunk_Open(&cinematicGlob.residentHunk, residentBufferBase, (int)newResidentBufferSize);
}

int __cdecl CinematicHunk_Alloc(CinematicHunk *hunk, int size)
{
    const char *v2; // eax
    char *alloced; // [esp+0h] [ebp-4h]

    if (!hunk->base)
        MyAssertHandler(".\\r_cinematic.cpp", 366, 0, "%s", "hunk->base");
    if (!hunk->atFront)
        MyAssertHandler(".\\r_cinematic.cpp", 367, 0, "%s", "hunk->atFront");
    if (!hunk->atBack)
        MyAssertHandler(".\\r_cinematic.cpp", 368, 0, "%s", "hunk->atBack");
    if (!hunk->end)
        MyAssertHandler(".\\r_cinematic.cpp", 369, 0, "%s", "hunk->end");
    if (size < 0)
        MyAssertHandler(".\\r_cinematic.cpp", 370, 0, "%s", "size >= 0");
    alloced = (char *)hunk->atFront;
    hunk->atFront = &alloced[size];
    if (hunk->atFront <= hunk->atBack)
        return (int)alloced;
    if (!alwaysfails)
    {
        v2 = va("CinematicHunk_Alloc failed: 0x%08x 0x%08x 0x%08x\n", hunk->atFront, size, hunk->atBack);
        MyAssertHandler(".\\r_cinematic.cpp", 376, 0, v2);
    }
    return -1;
}

void __cdecl R_Cinematic_HunksReset(int activeTexture, char playbackFlags)
{
    CinematicHunk_Close(&cinematicGlob.binkHunk);
    CinematicHunk_Close(&cinematicGlob.residentHunk);
    CinematicHunk_Reset(&cinematicGlob.masterHunk);
    R_Cinematic_HunksAllocate(activeTexture, playbackFlags);
}

static void __cdecl R_Cinematic_ReleaseImages(CinematicTextureSet *textureSet)
{
    int frameIter; // [esp+0h] [ebp-4h]

    for (frameIter = 0; frameIter != 2; ++frameIter)
    {
        Image_Release(&textureSet->imageY[frameIter]);
        Image_Release(&textureSet->imageCr[frameIter]);
        Image_Release(&textureSet->imageCb[frameIter]);
        Image_Release(&textureSet->imageA[frameIter]);
    }
    Image_Release(&textureSet->drawImageY);
    Image_Release(&textureSet->drawImageCr);
    Image_Release(&textureSet->drawImageCb);
    Image_Release(&textureSet->drawImageA);
}

IDirect3DTexture9 *__cdecl R_Cinematic_MakeBinkTexture_PC(
    GfxImage *image,
    unsigned int width,
    unsigned int height,
    int baseImageFlags)
{
    Image_Setup(image, width, height, 1, baseImageFlags | 3, D3DFMT_L8);
    return image->texture.map;
}

IDirect3DTexture9 *R_Cinematic_MakeBinkDrawTextures()
{
    IDirect3DTexture9 *result; // eax
    bool useAlpha; // [esp+Bh] [ebp-9h]
    int frameIter; // [esp+Ch] [ebp-8h]
    CinematicTextureSet *textureSet; // [esp+10h] [ebp-4h]

    useAlpha = 0;
    for (frameIter = 0; frameIter != 2; ++frameIter)
    {
        if (cinematicGlob.binkTextureSet.bink_buffers.Frames[frameIter].APlane.Allocate)
            useAlpha = 1;
    }
    textureSet = &cinematicGlob.textureSets[cinematicGlob.activeTextureSet];
    cinematicGlob.binkTextureSet.tex_draw.Ytexture = R_Cinematic_MakeBinkTexture_PC(
        &textureSet->drawImageY,
        cinematicGlob.binkTextureSet.bink_buffers.YABufferWidth,
        cinematicGlob.binkTextureSet.bink_buffers.YABufferHeight,
        0x10000);
    cinematicGlob.binkTextureSet.tex_draw.cRtexture = R_Cinematic_MakeBinkTexture_PC(
        &textureSet->drawImageCr,
        cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferWidth,
        cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferHeight,
        0x10000);
    cinematicGlob.binkTextureSet.tex_draw.cBtexture = R_Cinematic_MakeBinkTexture_PC(
        &textureSet->drawImageCb,
        cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferWidth,
        cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferHeight,
        0x10000);
    result = (IDirect3DTexture9 *)useAlpha;
    if (useAlpha)
    {
        result = R_Cinematic_MakeBinkTexture_PC(
            &textureSet->drawImageA,
            cinematicGlob.binkTextureSet.bink_buffers.YABufferWidth,
            cinematicGlob.binkTextureSet.bink_buffers.YABufferHeight,
            0x10000);
        cinematicGlob.binkTextureSet.tex_draw.Atexture = result;
    }
    else
    {
        cinematicGlob.binkTextureSet.tex_draw.Atexture = 0;
    }
    return result;
}

IDirect3DTexture9 *R_Cinematic_InitBinkTextures()
{
    BINKFRAMETEXTURES *textures; // [esp+40h] [ebp-10h]
    int frameIter; // [esp+48h] [ebp-8h]
    CinematicTextureSet *textureSet; // [esp+4Ch] [ebp-4h]

    g_cinematicThreadState = CINEMATIC_THREAD_STATE_TO_HOST_NEED_UNBIND_ALL_IMAGES;
    Sys_ResetCinematicsHostOutstandingRequestEvent();
    Sys_SetCinematicsThreadOutstandingRequestEvent();
    R_CinematicThread_WaitForHostEvent();
    textureSet = &cinematicGlob.textureSets[cinematicGlob.activeTextureSet];
    R_Cinematic_ReleaseImages(textureSet);
    if (cinematicGlob.binkTextureSet.bink_buffers.TotalFrames != 2)
        MyAssertHandler(
            ".\\r_cinematic.cpp",
            867,
            0,
            "%s",
            "cinematicGlob.binkTextureSet.bink_buffers.TotalFrames == CINEMATIC_IMAGES_REQUIRED");
    for (frameIter = 0; frameIter != 2; ++frameIter)
    {
        textures = &cinematicGlob.binkTextureSet.textures[frameIter];
        textures->Ytexture = R_Cinematic_MakeBinkTexture_PC(
            &textureSet->imageY[frameIter],
            cinematicGlob.binkTextureSet.bink_buffers.YABufferWidth,
            cinematicGlob.binkTextureSet.bink_buffers.YABufferHeight,
            0x40000);
        textures->cRtexture = R_Cinematic_MakeBinkTexture_PC(
            &textureSet->imageCr[frameIter],
            cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferWidth,
            cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferHeight,
            0x40000);
        textures->cBtexture = R_Cinematic_MakeBinkTexture_PC(
            &textureSet->imageCb[frameIter],
            cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferWidth,
            cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferHeight,
            0x40000);
        if (cinematicGlob.binkTextureSet.bink_buffers.Frames[frameIter].APlane.Allocate)
            textures->Atexture = R_Cinematic_MakeBinkTexture_PC(
                &textureSet->imageA[frameIter],
                cinematicGlob.binkTextureSet.bink_buffers.YABufferWidth,
                cinematicGlob.binkTextureSet.bink_buffers.YABufferHeight,
                0x40000);
        else
            textures->Atexture = 0;
    }
    return R_Cinematic_MakeBinkDrawTextures();
}

void* __stdcall R_Cinematic_Bink_Alloc(unsigned int bytes)
{
    return (void*)CinematicHunk_Alloc(&cinematicGlob.binkHunk, bytes);
}

void __stdcall R_Cinematic_Bink_Free(void *ptr)
{
    ;
}

bool __cdecl R_Cinematic_BinkOpen(
    const char *filename,
    unsigned int playbackFlags,
    char *errText,
    unsigned int errTextSize)
{
    char *cwd; // [esp+4h] [ebp-20Ch]
    char filepath[2][256]; // [esp+8h] [ebp-208h] BYREF

    cwd = Sys_Cwd();
    if (!cwd)
        MyAssertHandler(".\\r_cinematic.cpp", 1494, 0, "%s", "cwd");
    filepath[1][0] = 0;
    if ((playbackFlags & 0x20) != 0)
    {
        if ((playbackFlags & 8) == 0)
            MyAssertHandler(
                ".\\r_cinematic.cpp",
                1507,
                0,
                "%s\n\t(playbackFlags) = %i",
                "(playbackFlags & 0x00000008)",
                playbackFlags);
        _snprintf(filepath[0], 0x100u, "video/%s.%s", filename, "bik");
    }
    else
    {
        _snprintf(filepath[0], 0x100u, "%s\\main\\video\\%s.%s", cwd, filename, "bik");
        _snprintf(filepath[1], 0x100u, "%s\\raw\\video\\%s.%s", cwd, filename, "bik");
    }
    if (R_Cinematic_BinkOpenPath(filepath[0], playbackFlags, errText, errTextSize))
        return 1;
    return filepath[1][0] && R_Cinematic_BinkOpenPath(filepath[1], playbackFlags, errText, errTextSize);
}

char __cdecl R_Cinematic_BinkOpenPath(
    const char *filepath,
    char playbackFlags,
    char *errText,
    unsigned int errTextSize)
{
    const char *Error; // eax
    RawFile *rawfile; // [esp+5Ch] [ebp-Ch]
    const void *filledBuffer; // [esp+60h] [ebp-8h] BYREF
    unsigned int flags; // [esp+64h] [ebp-4h]

    if ((playbackFlags & 8) != 0)
    {
        if ((cinematicGlob.playbackFlags & 1) == 0)
        {
            g_cinematicThreadState = CINEMATIC_THREAD_STATE_TO_HOST_ENTERING_BINK;
            Sys_ResetCinematicsHostOutstandingRequestEvent();
            Sys_SetCinematicsThreadOutstandingRequestEvent();
            R_CinematicThread_WaitForHostEvent();
        }
        if ((playbackFlags & 0x20) != 0)
        {
            rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, filepath).rawfile;
            if (!rawfile)
            {
                if (errText)
                    _snprintf(errText, errTextSize, "Couldn't find rawfile '%s' in db", filepath);
                return 0;
            }
            filledBuffer = rawfile->buffer;
        }
        else if (!R_Cinematic_BinkOpenPath_MemoryResident(filepath, &filledBuffer, errText, errTextSize))
        {
            return 0;
        }
        flags = 0x4104400;
        cinematicGlob.bink = (BINK *)BinkOpen((const char*)filledBuffer, 0x4104400);
        if (!cinematicGlob.bink)
            CinematicHunk_Reset(&cinematicGlob.residentHunk);
        if ((cinematicGlob.playbackFlags & 1) == 0)
        {
            g_cinematicThreadState = CINEMATIC_THREAD_STATE_TO_HOST_EXITED_BINK;
            Sys_ResetCinematicsHostOutstandingRequestEvent();
            Sys_SetCinematicsThreadOutstandingRequestEvent();
            R_CinematicThread_WaitForHostEvent();
        }
    }
    else
    {
        if ((cinematicGlob.playbackFlags & 1) == 0)
        {
            g_cinematicThreadState = CINEMATIC_THREAD_STATE_TO_HOST_ENTERING_BINK;
            Sys_ResetCinematicsHostOutstandingRequestEvent();
            Sys_SetCinematicsThreadOutstandingRequestEvent();
            R_CinematicThread_WaitForHostEvent();
        }
        if (CinematicHunk_GetFreeSpace(&cinematicGlob.binkHunk) <= 1572864)
            MyAssertHandler(
                ".\\r_cinematic.cpp",
                1440,
                0,
                "%s",
                "CinematicHunk_GetFreeSpace( &cinematicGlob.binkHunk ) > BINK_MISC_BUFFER_SIZE");
        cinematicGlob.binkIOSize = CinematicHunk_GetFreeSpace(&cinematicGlob.binkHunk) - 1572864;
        BinkSetIOSize(cinematicGlob.binkIOSize);
        flags = 17843200;
        cinematicGlob.bink = (BINK *)BinkOpen(filepath, 17843200);
        R_CinematicThread_EndBinkAsync();
    }
    if (cinematicGlob.bink)
        return 1;
    if (errText)
    {
        Error = (const char *)BinkGetError();
        _snprintf(errText, errTextSize, "BinkOpen: %s", Error);
    }
    return 0;
}

int __cdecl CinematicHunk_GetFreeSpace(CinematicHunk *hunk)
{
    if (!hunk->base)
        MyAssertHandler(".\\r_cinematic.cpp", 355, 0, "%s", "hunk->base");
    if (!hunk->atFront)
        MyAssertHandler(".\\r_cinematic.cpp", 356, 0, "%s", "hunk->atFront");
    if (!hunk->atBack)
        MyAssertHandler(".\\r_cinematic.cpp", 357, 0, "%s", "hunk->atBack");
    if (!hunk->end)
        MyAssertHandler(".\\r_cinematic.cpp", 358, 0, "%s", "hunk->end");
    return (char *)hunk->atBack - (char *)hunk->atFront;
}

bool R_CinematicThread_EndBinkAsync()
{
    bool result; // eax

    result = cinematicGlob.playbackFlags & 1;
    if ((cinematicGlob.playbackFlags & 1) == 0)
    {
        g_cinematicThreadState = CINEMATIC_THREAD_STATE_TO_HOST_EXITED_BINK;
        Sys_ResetCinematicsHostOutstandingRequestEvent();
        Sys_SetCinematicsThreadOutstandingRequestEvent();
        return R_CinematicThread_WaitForHostEvent();
    }
    return result;
}

char __cdecl R_Cinematic_BinkOpenPath_MemoryResident(
    const char *filename,
    const void **outPtr,
    char *errText,
    unsigned int errTextSize)
{
    void *allocedBuffer; // [esp+Ch] [ebp-18h]
    iobuf *fileHandle; // [esp+10h] [ebp-14h]
    int fileSize; // [esp+14h] [ebp-10h]
    int numberOfBytesRead; // [esp+1Ch] [ebp-8h]
    int freeBufferSpace; // [esp+20h] [ebp-4h]

    fileHandle = fopen(filename, "rb");
    if (!fileHandle)
    {
        if (errText)
            _snprintf(errText, errTextSize, "Open failed");
        return 0;
    }
    fileSize = FileWrapper_GetFileSize(fileHandle);
    if (fileSize <= 0)
    {
        if (errText)
        {
            if (fileSize)
                _snprintf(errText, errTextSize, "GetFileSize failed (%i)", fileSize);
            else
                _snprintf(errText, errTextSize, "Zero file size");
        }
    LABEL_14:
        fclose(fileHandle);
        return 0;
    }
    freeBufferSpace = CinematicHunk_GetFreeSpace(&cinematicGlob.residentHunk);
    if (fileSize > freeBufferSpace)
    {
        if (errText)
            _snprintf(errText, errTextSize, "Cinematic too big (%i > %i)", fileSize, freeBufferSpace);
        goto LABEL_14;
    }
    allocedBuffer = (void *)CinematicHunk_Alloc(&cinematicGlob.residentHunk, fileSize);
    *outPtr = allocedBuffer;
    numberOfBytesRead = fread(allocedBuffer, 1u, fileSize, fileHandle);
    fclose(fileHandle);
    if (numberOfBytesRead == fileSize)
        return 1;
    if (errText)
        _snprintf(errText, errTextSize, "Read was short (wanted %i got %i)", fileSize, numberOfBytesRead);
    CinematicHunk_Reset(&cinematicGlob.residentHunk);
    return 0;
}

bool R_CinematicThread_WaitForHostEvent()
{
    bool result; // eax

    while (1)
    {
        result = Sys_WaitForCinematicsHostOutstandingRequestEventTimeout(0x10u);
        if (result)
            break;
        if (cinematicGlob.hasFileIO && R_Cinematic_GetPercentageFull() > 0x5F)
            R_Cinematic_RelinquishIO();
    }
    return result;
}

void __cdecl R_Cinematic_Shutdown()
{
    int setIter; // [esp+0h] [ebp-4h]

    R_Cinematic_StopPlayback();
    R_Cinematic_SyncNow();
    for (setIter = 0; setIter != 2; ++setIter)
        R_Cinematic_ReleaseImages(&cinematicGlob.textureSets[setIter]);
    Z_Free((char *)cinematicGlob.memPool, 18);
}

void __cdecl R_Cinematic_StartPlayback(char *name, unsigned int playbackFlags, float volume)
{
    Sys_EnterCriticalSection(CRITSECT_CINEMATIC);
    R_Cinematic_StartPlayback_Internal(name, playbackFlags, volume);
    Sys_LeaveCriticalSection(CRITSECT_CINEMATIC);
}

void __cdecl R_Cinematic_StartPlayback_Internal(char *name, unsigned int playbackFlags, float volume)
{
    Sys_EnterCriticalSection(CRITSECT_CINEMATIC_TARGET_CHANGE);
    I_strncpyz(cinematicGlob.targetCinematicName, name, 256);
    cinematicGlob.targetCinematicChanged = 1;
    cinematicGlob.cinematicFinished = 0;
    cinematicGlob.targetPaused = CINEMATIC_NOT_PAUSED;
    cinematicGlob.playbackFlags = playbackFlags;
    cinematicGlob.playbackVolume = volume;
    Sys_LeaveCriticalSection(CRITSECT_CINEMATIC_TARGET_CHANGE);
}

void __cdecl R_Cinematic_StartNextPlayback()
{
    Sys_EnterCriticalSection(CRITSECT_CINEMATIC);
    if (R_Cinematic_IsNextReady_Internal())
    {
        R_Cinematic_StartPlayback_Internal(
            cinematicGlob.nextCinematicName,
            cinematicGlob.nextCinematicPlaybackFlags,
            cinematicGlob.playbackVolume);
        cinematicGlob.nextCinematicName[0] = 0;
    }
    Sys_LeaveCriticalSection(CRITSECT_CINEMATIC);
}

bool __cdecl R_Cinematic_IsNextReady_Internal()
{
    return cinematicGlob.nextCinematicName[0] != 0;
}

void __cdecl R_Cinematic_StopPlayback()
{
    Sys_EnterCriticalSection(CRITSECT_CINEMATIC);
    Sys_EnterCriticalSection(CRITSECT_CINEMATIC_TARGET_CHANGE);
    cinematicGlob.targetCinematicName[0] = 0;
    cinematicGlob.targetCinematicChanged = 1;
    cinematicGlob.cinematicFinished = 0;
    Sys_LeaveCriticalSection(CRITSECT_CINEMATIC_TARGET_CHANGE);
    Sys_LeaveCriticalSection(CRITSECT_CINEMATIC);
}

void __cdecl R_Cinematic_UpdateFrame()
{
    bool v0; // [esp+0h] [ebp-44h]

    //Profile_Begin(144);
    Sys_EnterCriticalSection(CRITSECT_CINEMATIC);
    v0 = !cinematicGlob.fullSyncNextUpdate && (cinematicGlob.playbackFlags & 1) == 0;
    cinematicGlob.fullSyncNextUpdate = 0;
    if (g_cinematicThreadState == CINEMATIC_THREAD_STATE_FROM_HOST_GO)
        MyAssertHandler(
            ".\\r_cinematic.cpp",
            1984,
            0,
            "%s",
            "g_cinematicThreadState != CINEMATIC_THREAD_STATE_FROM_HOST_GO");
    if (R_Cinematic_ThreadFinish(v0))
    {
        g_cinematicThreadState = CINEMATIC_THREAD_STATE_FROM_HOST_GO;
        Sys_ResetCinematicsThreadOutstandingRequestEvent();
        Sys_SetCinematicsHostOutstandingRequestEvent();
        R_Cinematic_ThreadFinish(v0);
    }
    R_Cinematic_UpdateRendererImages();
    Sys_LeaveCriticalSection(CRITSECT_CINEMATIC);
    //Profile_EndInternal(0);
}

void R_Cinematic_UpdateRendererImages()
{
    if (cinematicGlob.activeImageFrame == -1)
    {
        RB_UnbindAllImages();
        gfxCmdBufInput.codeImages[22] = rgp.blackImage;
        gfxCmdBufInput.codeImages[23] = rgp.grayImage;
        gfxCmdBufInput.codeImages[24] = rgp.grayImage;
        gfxCmdBufInput.codeImages[25] = rgp.blackImage;
        if (!rgp.grayImage)
            MyAssertHandler(".\\r_cinematic.cpp", 974, 0, "%s", "gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_CR]");
        if (!gfxCmdBufInput.codeImages[24])
            MyAssertHandler(".\\r_cinematic.cpp", 975, 0, "%s", "gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_CB]");
        if (!gfxCmdBufInput.codeImages[25])
            MyAssertHandler(".\\r_cinematic.cpp", 976, 0, "%s", "gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_A]");
        if (!gfxCmdBufInput.codeImages[22])
            MyAssertHandler(".\\r_cinematic.cpp", 977, 0, "%s", "gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_Y]");
    }
    else
    {
        R_Cinematic_SetRendererImagesToFrame(cinematicGlob.activeImageFrame);
        if (!gfxCmdBufInput.codeImages[23])
            MyAssertHandler(".\\r_cinematic.cpp", 987, 0, "%s", "gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_CR]");
        if (!gfxCmdBufInput.codeImages[24])
            MyAssertHandler(".\\r_cinematic.cpp", 988, 0, "%s", "gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_CB]");
        if (!gfxCmdBufInput.codeImages[25])
            MyAssertHandler(".\\r_cinematic.cpp", 989, 0, "%s", "gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_A]");
        if (!gfxCmdBufInput.codeImages[22])
            MyAssertHandler(".\\r_cinematic.cpp", 990, 0, "%s", "gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_Y]");
    }
}

void __cdecl R_Cinematic_SetRendererImagesToFrame(int frameToSetTo)
{
    CinematicTextureSet *textureSet; // [esp+0h] [ebp-4h]

    textureSet = &cinematicGlob.textureSets[cinematicGlob.activeImageFrameTextureSet];
    if (frameToSetTo == -1)
        MyAssertHandler(".\\r_cinematic.cpp", 929, 0, "%s", "frameToSetTo != CINEMATIC_INVALID_IMAGE_FRAME");
    RB_UnbindAllImages();
    gfxCmdBufInput.codeImages[22] = &textureSet->drawImageY;
    gfxCmdBufInput.codeImages[23] = &textureSet->drawImageCr;
    gfxCmdBufInput.codeImages[24] = &textureSet->drawImageCb;
    if (textureSet->drawImageA.texture.basemap)
        gfxCmdBufInput.codeImages[25] = &textureSet->drawImageA;
    else
        gfxCmdBufInput.codeImages[25] = rgp.whiteImage;
}

char __cdecl R_Cinematic_ThreadFinish(bool midBinkIsOkay)
{
    do
    {
        while (2)
        {
            if (Sys_WaitForCinematicsThreadOutstandingRequestEventTimeout(1u))
            {
                switch (g_cinematicThreadState)
                {
                case CINEMATIC_THREAD_STATE_TO_HOST_BETWEEN_UPDATES:
                    return 1;
                case CINEMATIC_THREAD_STATE_TO_HOST_ENTERING_BINK:
                    g_cinematicThreadState = CINEMATIC_THREAD_STATE_FROM_HOST_GO_BINK;
                    goto LABEL_14;
                case CINEMATIC_THREAD_STATE_TO_HOST_EXITED_BINK:
                    g_cinematicThreadState = CINEMATIC_THREAD_STATE_FROM_HOST_GO;
                    goto LABEL_14;
                case CINEMATIC_THREAD_STATE_TO_HOST_NEED_UNBIND_ALL_IMAGES:
                    RB_UnbindAllImages();
                    g_cinematicThreadState = CINEMATIC_THREAD_STATE_FROM_HOST_GO;
                LABEL_14:
                    Sys_ResetCinematicsThreadOutstandingRequestEvent();
                    Sys_SetCinematicsHostOutstandingRequestEvent();
                    continue;
                default:
                    if (!alwaysfails)
                        MyAssertHandler(".\\r_cinematic.cpp", 1861, 0, "R_Cinematic_ThreadFinish: Can't happen.");
                    return 1;
                }
            }
            break;
        }
    } while (!midBinkIsOkay || g_cinematicThreadState != CINEMATIC_THREAD_STATE_FROM_HOST_GO_BINK);
    return 0;
}

void __cdecl R_Cinematic_SyncNow()
{
    cinematicGlob.fullSyncNextUpdate = 1;
    R_Cinematic_UpdateFrame();
}

void __cdecl R_Cinematic_DrawLetterbox_OptionalCinematic(bool drawCinematic, float letterboxAlpha)
{
    float rectY; // [esp+3Ch] [ebp-2Ch]
    float width; // [esp+44h] [ebp-24h]
    float height; // [esp+48h] [ebp-20h]
    float letterboxHalfHeight; // [esp+4Ch] [ebp-1Ch]
    float color[4]; // [esp+50h] [ebp-18h] BYREF
    float movieHeight; // [esp+60h] [ebp-8h]
    float aspectRatio; // [esp+64h] [ebp-4h]

    width = (float)vidConfig.displayWidth;
    height = (float)vidConfig.displayHeight;
    aspectRatio = vidConfig.aspectRatioDisplayPixel;
    movieHeight = width * vidConfig.aspectRatioDisplayPixel / (float)1.7777778;
    if (height < (double)movieHeight)
        movieHeight = (float)vidConfig.displayHeight;
    letterboxHalfHeight = (height - movieHeight) * 0.5;
    color[0] = 0.0;
    color[1] = 0.0;
    color[2] = 0.0;
    color[3] = letterboxAlpha;
    R_AddCmdDrawStretchPic(0.0, 0.0, width, letterboxHalfHeight, 0.0, 0.0, 1.0, 1.0, color, rgp.whiteMaterial);
    rectY = height - letterboxHalfHeight;
    R_AddCmdDrawStretchPic(0.0, rectY, width, letterboxHalfHeight, 0.0, 0.0, 1.0, 1.0, color, rgp.whiteMaterial);
    if (drawCinematic)
        R_AddCmdDrawStretchPic(
            0.0,
            letterboxHalfHeight,
            width,
            movieHeight,
            0.0,
            0.0,
            1.0,
            1.0,
            colorWhite,
            rgp.cinematicMaterial);
}

void __cdecl R_Cinematic_DrawStretchPic_Letterboxed()
{
    R_Cinematic_DrawLetterbox_OptionalCinematic(1, 1.0);
}

bool __cdecl R_Cinematic_IsFinished()
{
    return cinematicGlob.cinematicFinished;
}

bool __cdecl R_Cinematic_IsStarted()
{
    return !R_Cinematic_IsFinished() && cinematicGlob.currentCinematicName[0];
}

bool __cdecl R_Cinematic_IsNextReady()
{
    return R_Cinematic_IsNextReady_Internal();
}

bool __cdecl R_Cinematic_IsUnderrun()
{
    return cinematicGlob.underrun;
}

