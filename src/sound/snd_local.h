#pragma once

#ifndef KISAK_SOUND
#include <msslib/mss.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
// efx.h only declares its functions as directly-linkable (rather than just LPALGENEFFECTS-
// style function-pointer typedefs meant for dynamic alGetProcAddress loading) when this is
// defined first. Since we statically link openal-soft ourselves and know EFX is compiled
// in, direct linkage is simpler than the usual portable-extension-loading dance.
#define AL_ALEXT_PROTOTYPES
#include <AL/efx.h>
#endif
#include "snd_public.h"

static const char *snd_outputConfigurationStrings[6] = { "Windows default", "Mono", "Stereo", "4 speakers", "5.1 speakers", NULL }; // idb

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

#ifndef KISAK_SOUND
// Miles' file-callback bridge (see MSS_File*Callback in snd_mss.cpp). The OpenAL path reads
// directly via FS_Read when refilling stream buffers, so it has no equivalent bookkeeping.
struct MssFileHandle // sizeof=0x9C
{                                       // ...
    uint32_t id;
    MssFileHandle *next;
    int handle;
    char fileName[128];
    uint32_t hashCode;
    int offset;
    int fileOffset;
    int fileLength;
};
#endif

struct SndEqParams // sizeof=0x14
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

struct snd_eqoverlay_info_t // sizeof=0x1C
{                                       // ...
    SndEqParams *params[2][3];
    float lerp;                         // ...
};

struct MssEqInfo // sizeof=0xF00
{                                       // ...
    SndEqParams params[3][64];
};

#ifndef KISAK_SOUND
typedef struct _SAMPLE FAR *HSAMPLE;           // Handle to sample

struct MssLocal // sizeof=0x26D0
{                                       // ...
    _DIG_DRIVER *driver;                // ...
    HSAMPLE handle_sample[40];         // ...
    _STREAM *handle_stream[13];
    MssEqInfo eq[2];                    // ...
    uint32_t eqFilter;              // ...
#ifndef KISAK_XBOX
	float eqLerp;
#endif

    MssFileHandle fileHandle[13];
    MssFileHandle *freeFileHandle;
    bool isMultiChannel;                // ...
    // padding byte
    // padding byte
    // padding byte
};
#else
// OpenAL equivalent of MssLocal. Unlike Miles (which split 2D/3D "samples" from "streams"
// into separate handle types/arrays), OpenAL sources are uniform, so a single array covers
// all 53 channels (indices match g_snd.chaninfo[] exactly: 0-7 2D, 8-39 3D, 40-52 stream).
struct AlLocal
{
    ALCdevice *device;
    ALCcontext *context;
    ALuint source[53];
    // Per-channel AL buffer, generated fresh each time SND_StartAlias2D/3DSample plays a
    // "loaded" sound and deleted when the channel is stopped/reused. Simpler than caching
    // one buffer per SoundFile (which would need a lifetime-safe cache key working for both
    // the raw-CSV load path and fastfile-preloaded LoadedSounds - see WORK.md Phase 4), at
    // the cost of re-uploading PCM to the driver on every replay of the same sound.
    ALuint channelBuffer[53];

    MssEqInfo eq[2];                    // same EQ band data as Miles; DSP application TBD
    ALuint eqFilter;                    // reserved for a future EFX filter object (unused for now)
#ifndef KISAK_XBOX
    float eqLerp;
#endif

    ALuint auxSlot;                     // global reverb auxiliary effect slot
    ALuint reverbEffect;                // global reverb effect object (EAXREVERB, one room preset active at a time)
    // Per-channel AL_FILTER_LOWPASS object, allocated once at init (unlike channelBuffer,
    // its params just get updated in place, not recreated per-play). Used purely as a
    // per-source wet-send *gain* carrier for AL_AUXILIARY_SEND_FILTER (AL_LOWPASS_GAIN set
    // to the desired send level, AL_LOWPASS_GAINHF left neutral at 1.0) - the standard EFX
    // technique for per-source send level, since AL_AUXILIARY_SEND_FILTER has no gain
    // parameter of its own. See SND_ApplyReverbSend in snd_driver.cpp.
    ALuint sendFilter[53];
    bool isMultiChannel;
};
#endif

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
    uint32_t entchannel,
    int eqIndex,
    uint32_t band,
    SND_EQTYPE type,
    float gain,
    float freq,
    float q);
void __cdecl SND_SetEqType(uint32_t entchannel, int eqIndex, uint32_t band, SND_EQTYPE type);
void __cdecl SND_SetEqFreq(uint32_t entchannel, int eqIndex, uint32_t band, float freq);
void __cdecl SND_SetEqGain(uint32_t entchannel, int eqIndex, uint32_t band, float gain);
void __cdecl SND_SetEqQ(uint32_t entchannel, int eqIndex, uint32_t band, float q);
void __cdecl SND_DisableEq(uint32_t entchannel, int eqIndex, uint32_t band);
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
int __cdecl SND_GetSoundFileSize(uint32_t *pSoundFile);
void __cdecl SND_DriverPostUpdate();
void __cdecl SND_Update2DChannel(int i, int frametime);
void __cdecl SND_Update3DChannel(int i, int frametime);
void __cdecl SND_UpdateStreamChannel(int i, int frametime);

#ifdef KISAK_SP
void SND_SetEqLerp(double lerp);
#endif



// snd_mss (!KISAK_SOUND: implemented in snd_mss.cpp against Miles)
// snd_al  ( KISAK_SOUND: implemented in snd_al.cpp against OpenAL)
//
// Function names keep their historical MSS_ prefix even on the OpenAL side, so that shared
// callers (snd.cpp, snd_driver.cpp) can call them without their own #ifdef KISAK_SOUND -
// only one of snd_mss.cpp/snd_al.cpp is compiled for a given build, and it provides the body.
#ifndef KISAK_SOUND
// Miles routes all its file I/O (including stream reads) through these callbacks, bridged
// to FS_* in snd_mss.cpp. OpenAL has no equivalent hook; its streaming path (added in a later
// phase) calls FS_Read directly when refilling buffers, so these have no OpenAL counterpart.
uint32_t __stdcall MSS_FileOpenCallback(const MSS_FILE *pszFilename, UINTa *phFileHandle);
void __stdcall MSS_FileCloseCallback(UINTa hFileHandle);
int __stdcall MSS_FileSeekCallback(UINTa hFileHandle, int offset, uint32_t type);
uint32_t __stdcall MSS_FileReadCallback(UINTa hFileHandle, void *pBuffer, uint32_t bytes);

_DIG_DRIVER *__cdecl MSS_open_digital_driver(int hertz, int bits, int channels);
#endif
void MSS_InitFailed();
char __cdecl MSS_Init();
void MSS_InitChannels();
void MSS_InitEq();
bool __cdecl MSS_Startup();
void MSS_ShutdownCleanup();
float MSS_GetDryLevel();
float MSS_GetWetLevel(const snd_alias_t *pAlias);
#ifndef KISAK_SOUND
void __cdecl MSS_ApplyEqFilter(_SAMPLE *s, int entchannel);
#else
void __cdecl MSS_ApplyEqFilter(ALuint source, int entchannel);
#endif
void __cdecl MSS_ResumeSample(int i, int frametime);
#ifndef KISAK_SOUND
_DIG_DRIVER *__cdecl MSS_GetDriver();
#endif
int __cdecl MSS_DigitalFormatType(int waveFormat, int bits, int channels);
uint8_t *__cdecl MSS_Alloc(uint32_t bytes, uint32_t rate);
uint8_t *__cdecl MSS_Alloc_LoadObj(uint32_t bytes, uint32_t rate);
uint32_t *__cdecl MSS_Alloc_FastFile(int bytes);


#ifndef KISAK_SOUND
extern MssLocal milesGlob;
#else
extern AlLocal alGlob;
#endif
extern snd_local_t g_snd;

extern const dvar_t *snd_khz;
extern const dvar_t *snd_outputConfiguration;