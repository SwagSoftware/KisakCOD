#pragma once

#include <d3d9.h>
#include <binklib/bink.h>
#include <binklib/binktextures.h>
#include "r_material.h"

enum CinematicEnum : __int32
{                                       // ...
    CINEMATIC_NOT_PAUSED = 0x0,
    CINEMATIC_PAUSED = 0x1,
};

enum CinematicThreadState : __int32
{
    CINEMATIC_THREAD_STATE_FROM_HOST_GO = 0x0,
    CINEMATIC_THREAD_STATE_FROM_HOST_GO_BINK = 0x1,
    CINEMATIC_THREAD_STATE_TO_HOST_BETWEEN_UPDATES = 0x2,
    CINEMATIC_THREAD_STATE_TO_HOST_ENTERING_BINK = 0x3,
    CINEMATIC_THREAD_STATE_TO_HOST_EXITED_BINK = 0x4,
    CINEMATIC_THREAD_STATE_TO_HOST_NEED_UNBIND_ALL_IMAGES = 0x5,
};

struct CinematicHunk // sizeof=0x10
{                                       // ...
    void* base;
    void* atFront;
    void* atBack;
    void* end;
};

struct CinematicTextureSet // sizeof=0x1B0
{                                       // ...
    GfxImage imageY[2];
    GfxImage imageCr[2];
    GfxImage imageCb[2];
    GfxImage imageA[2];
    GfxImage drawImageY;
    GfxImage drawImageCr;
    GfxImage drawImageCb;
    GfxImage drawImageA;
};

struct __declspec(align(4)) CinematicGlob // sizeof=0x7AC
{                                       // ...
    char currentCinematicName[256];     // ...
    char targetCinematicName[256];      // ...
    char nextCinematicName[256];        // ...
    unsigned int nextCinematicPlaybackFlags; // ...
    unsigned int playbackFlags;         // ...
    bool targetCinematicChanged;        // ...
    bool cinematicFinished;             // ...
    // padding byte
    // padding byte
    unsigned int timeInMsec;            // ...
    unsigned int binkIOSize;            // ...
    volatile bool fullSyncNextUpdate;   // ...
    bool playbackStarted;
    bool hasFileIO;                     // ...
    bool usingAlpha;
    bool atHighPriority;                // ...
    // padding byte
    // padding byte
    // padding byte
    BINK *bink;                         // ...
    BINKTEXTURESET binkTextureSet;      // ...
    CinematicHunk masterHunk;           // ...
    CinematicHunk binkHunk;             // ...
    CinematicHunk residentHunk;         // ...
    int activeImageFrame;               // ...
    int framesStopped;                  // ...
    CinematicEnum currentPaused;        // ...
    CinematicEnum targetPaused;         // ...
    CinematicTextureSet textureSets[2]; // ...
    int activeTextureSet;               // ...
    int activeImageFrameTextureSet;     // ...
    void *memPool;                      // ...
    float playbackVolume;               // ...
    bool underrun;                      // ...
    // padding byte
    // padding byte
    // padding byte
};

void R_Cinematic_RelinquishIO();
void R_Cinematic_CheckBinkError();
void __cdecl R_Cinematic_InitBinkVolumes();
void __cdecl R_Cinematic_Init();
unsigned int *R_Cinematic_ReserveMemory();
void __cdecl  R_Cinematic_Thread(unsigned int threadContext);
void R_Cinematic_UpdateFrame_Core2();
void __cdecl R_Cinematic_UpdateFrame_Core(
    bool localTargetChanged,
    char *localTargetCinematic,
    unsigned int localPlaybackFlags);
char __cdecl R_Cinematic_AreHunksOpen();
char __cdecl CinematicHunk_IsOpen(CinematicHunk *hunk);
void R_Cinematic_HunksClose();
void __cdecl CinematicHunk_Close(CinematicHunk *hunk);
char __cdecl R_Cinematic_Advance();
unsigned int __cdecl R_Cinematic_GetPercentageFull();
void R_Cinematic_SeizeIO();
void __cdecl R_Cinematic_UpdateTimeInMsec(const BINKREALTIME *binkRealtime);
void R_Cinematic_StopPlayback_Now();
void __cdecl CinematicHunk_Reset(CinematicHunk *hunk);
char __cdecl R_Cinematic_StartPlayback_Now(const char *filename, unsigned int playbackFlags);
bool __cdecl CinematicHunk_IsEmpty(CinematicHunk *hunk);
void __cdecl R_Cinematic_HunksOpen(int activeTexture, char playbackFlags);
void __cdecl CinematicHunk_Open(CinematicHunk *hunk, char *memory, int size);
void __cdecl R_Cinematic_HunksAllocate(int activeTexture, char playbackFlags);
int __cdecl CinematicHunk_Alloc(CinematicHunk *hunk, int size);
void __cdecl R_Cinematic_HunksReset(int activeTexture, char playbackFlags);
IDirect3DTexture9 *R_Cinematic_InitBinkTextures();
void* __stdcall R_Cinematic_Bink_Alloc(unsigned int bytes);
void __stdcall R_Cinematic_Bink_Free(void *ptr);
bool __cdecl R_Cinematic_BinkOpen(
    const char *filename,
    unsigned int playbackFlags,
    char *errText,
    unsigned int errTextSize);
char __cdecl R_Cinematic_BinkOpenPath(
    const char *filepath,
    char playbackFlags,
    char *errText,
    unsigned int errTextSize);
int __cdecl CinematicHunk_GetFreeSpace(CinematicHunk *hunk);
bool R_CinematicThread_EndBinkAsync();
char __cdecl R_Cinematic_BinkOpenPath_MemoryResident(
    const char *filename,
    const void **outPtr,
    char *errText,
    unsigned int errTextSize);
bool R_CinematicThread_WaitForHostEvent();
void __cdecl R_Cinematic_Shutdown();
void __cdecl R_Cinematic_StartPlayback(char *name, unsigned int playbackFlags, float volume);
void __cdecl R_Cinematic_StartPlayback_Internal(char *name, unsigned int playbackFlags, float volume);
void __cdecl R_Cinematic_StartNextPlayback();
bool __cdecl R_Cinematic_IsNextReady_Internal();
void __cdecl R_Cinematic_StopPlayback();
void __cdecl R_Cinematic_UpdateFrame();
void R_Cinematic_UpdateRendererImages();
void __cdecl R_Cinematic_SetRendererImagesToFrame(int frameToSetTo);
char __cdecl R_Cinematic_ThreadFinish(bool midBinkIsOkay);
void __cdecl R_Cinematic_SyncNow();
void __cdecl R_Cinematic_DrawStretchPic_Letterboxed();
bool __cdecl R_Cinematic_IsFinished();
bool __cdecl R_Cinematic_IsStarted();
bool __cdecl R_Cinematic_IsNextReady();
bool __cdecl R_Cinematic_IsUnderrun();
