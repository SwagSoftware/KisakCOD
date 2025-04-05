#pragma once

#include <msslib/mss.h>
#include "snd_public.h"

struct snd_save_2D_sample_t // sizeof=0x10
{                                       // ...
    float fraction;                     // ...
    float pitch;                        // ...
    float volume;
    float pan;
};

struct snd_save_3D_sample_t // sizeof=0x18
{                                       // ...
    float fraction;                     // ...
    float pitch;                        // ...
    float volume;
    float org[3];                       // ...
};

struct snd_save_stream_t // sizeof=0x20
{                                       // ...
    float fraction;                     // ...
    int rate;                           // ...
    float basevolume;                   // ...
    float volume;
    float pan;
    float org[3];                       // ...
};

struct MssFileHandle // sizeof=0x9C
{                                       // ...
    unsigned int id;
    MssFileHandle *next;
    int handle;
    char fileName[128];
    unsigned int hashCode;
    int offset;
    int fileOffset;
    int fileLength;
};

struct __declspec(align(4)) SndEqParams // sizeof=0x14
{                                       // ...
    SND_EQTYPE type;                    // ...
    float gain;                         // ...
    float freq;                         // ...
    float q;                            // ...
    bool enabled;                       // ...
    // padding byte
    // padding byte
    // padding byte
};

struct MssEqInfo // sizeof=0xF00
{                                       // ...
    SndEqParams params[3][64];
};

struct __declspec(align(4)) MssLocal // sizeof=0x26D0
{                                       // ...
    _DIG_DRIVER *driver;                // ...
    _SAMPLE *handle_sample[40];         // ...
    _STREAM *handle_stream[13];
    MssEqInfo eq[2];                    // ...
    unsigned int eqFilter;              // ...
    MssFileHandle fileHandle[13];
    MssFileHandle *freeFileHandle;
    bool isMultiChannel;                // ...
    // padding byte
    // padding byte
    // padding byte
};

// snd_driver
void __cdecl TRACK_snd_driver();
bool __cdecl SND_IsMultiChannel();
char __cdecl SND_InitDriver();
void __cdecl SND_ShutdownDriver();
int __cdecl SND_GetDriverCPUPercentage();
void __cdecl SND_Set3DPosition(int index, const float *org);
void __cdecl SND_Stop2DChannel(int index);
void __cdecl SND_Pause2DChannel(int index);
void __cdecl SND_Unpause2DChannel(int index, int timeshift);
bool __cdecl SND_Is2DChannelFree(int index);
void __cdecl SND_Stop3DChannel(int index);
void __cdecl SND_Pause3DChannel(int index);
void __cdecl SND_Unpause3DChannel(int index, int timeshift);
bool __cdecl SND_Is3DChannelFree(int index);
void __cdecl SND_StopStreamChannel(int index);
void __cdecl SND_PauseStreamChannel(int index);
void __cdecl SND_UnpauseStreamChannel(int index, int timeshift);
bool __cdecl SND_IsStreamChannelFree(int index);
int __cdecl SND_StartAlias2DSample(SndStartAliasInfo *startAliasInfo, int *pChannel);
int __cdecl SND_StartAlias3DSample(SndStartAliasInfo *startAliasInfo, int *pChannel);
int __cdecl SND_StartAliasStreamOnChannel(SndStartAliasInfo *startAliasInfo, int index);
void __cdecl SND_SetRoomtype(int roomtype);
void __cdecl SND_UpdateEqs();
void __cdecl SND_SetEqParams(
    unsigned int entchannel,
    int eqIndex,
    unsigned int band,
    SND_EQTYPE type,
    float gain,
    float freq,
    float q);
void __cdecl SND_SetEqType(unsigned int entchannel, int eqIndex, unsigned int band, SND_EQTYPE type);
void __cdecl SND_SetEqFreq(unsigned int entchannel, int eqIndex, unsigned int band, float freq);
void __cdecl SND_SetEqGain(unsigned int entchannel, int eqIndex, unsigned int band, float gain);
void __cdecl SND_SetEqQ(unsigned int entchannel, int eqIndex, unsigned int band, float q);
void __cdecl SND_DisableEq(unsigned int entchannel, int eqIndex, unsigned int band);
void __cdecl SND_SaveEq(MemoryFile *memFile);
void __cdecl SND_RestoreEq(MemoryFile *memFile);
void __cdecl SND_PrintEqParams();
double __cdecl SND_Get2DChannelVolume(int index);
void __cdecl SND_Set2DChannelVolume(int index, float volume);
double __cdecl SND_Get3DChannelVolume(int index);
void __cdecl SND_Set3DChannelVolume(int index, float volume);
double __cdecl SND_GetStreamChannelVolume(int index);
void __cdecl SND_SetStreamChannelVolume(int index, float volume);
int __cdecl SND_Get2DChannelPlaybackRate(int index);
void __cdecl SND_Set2DChannelPlaybackRate(int index, int rate);
int __cdecl SND_Get3DChannelPlaybackRate(int index);
void __cdecl SND_Set3DChannelPlaybackRate(int index, int rate);
int __cdecl SND_GetStreamChannelPlaybackRate(int index);
void __cdecl SND_SetStreamChannelPlaybackRate(int index, int rate);
void __cdecl SND_Update2DChannelReverb(int index);
void __cdecl SND_Update3DChannelReverb(int index);
void __cdecl SND_UpdateStreamChannelReverb(int index);
int __cdecl SND_Get2DChannelLength(int index);
int __cdecl SND_Get3DChannelLength(int index);
int __cdecl SND_GetStreamChannelLength(int index);
void __cdecl SND_Get2DChannelSaveInfo(int index, snd_save_2D_sample_t *info);
void __cdecl SND_Set2DChannelFromSaveInfo(int index, snd_save_2D_sample_t *info);
void __cdecl SND_Get3DChannelSaveInfo(int index, snd_save_3D_sample_t *info);
void __cdecl SND_GetStreamChannelSaveInfo(int index, snd_save_stream_t *info);
void __cdecl SND_SetStreamChannelFromSaveInfo(int index, snd_save_stream_t *info);
int __cdecl SND_GetSoundFileSize(unsigned int *pSoundFile);
void __cdecl SND_DriverPostUpdate();
void __cdecl SND_Update2DChannel(int i, int frametime);
void __cdecl SND_Update3DChannel(int i, int frametime);
void __cdecl SND_UpdateStreamChannel(int i, int frametime);




// snd_mss
unsigned int __stdcall MSS_FileOpenCallback(const MSS_FILE *pszFilename, unsigned int *phFileHandle);
void __stdcall MSS_FileCloseCallback(unsigned int hFileHandle);
int __stdcall MSS_FileSeekCallback(unsigned int hFileHandle, int offset, unsigned int type);
unsigned int __stdcall MSS_FileReadCallback(unsigned int hFileHandle, void *pBuffer, unsigned int bytes);

_DIG_DRIVER *__cdecl MSS_open_digital_driver(int hertz, int bits, int channels);
void MSS_InitFailed();
char __cdecl MSS_Init();
void MSS_InitChannels();
void MSS_InitEq();
bool __cdecl MSS_Startup();
void MSS_ShutdownCleanup();
double __cdecl MSS_GetWetLevel(const snd_alias_t *pAlias);
void __cdecl MSS_ApplyEqFilter(_SAMPLE *s, int entchannel);
void __cdecl MSS_ResumeSample(int i, int frametime);
_DIG_DRIVER *__cdecl MSS_GetDriver();
int __cdecl MSS_DigitalFormatType(int waveFormat, int bits, int channels);
unsigned __int8 *__cdecl MSS_Alloc(unsigned int bytes, unsigned int rate);
unsigned __int8 *__cdecl MSS_Alloc_LoadObj(unsigned int bytes, unsigned int rate);
unsigned int *__cdecl MSS_Alloc_FastFile(int bytes);


extern MssLocal milesGlob;
extern snd_local_t g_snd;

extern const dvar_t *snd_khz;
extern const dvar_t *snd_outputConfiguration;