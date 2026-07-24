#pragma once
#include <portaudio.h>
extern "C"
{
    #include <dr_libs/dr_wav.h>
    #include <dr_libs/dr_mp3.h>
};
#include "snd_public.h"

#include <thread>
#include <mutex>

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

extern snd_local_t g_snd;

extern const dvar_t *snd_khz;
extern const dvar_t *snd_outputConfiguration;

// PortAudio
enum {
    PA_OUTPUT_RATE = 48000,
    PA_OUTPUT_CHANNELS = 2,
    PA_RING_FRAMES = 8192 // per-stream ring buffer size in frames
};

typedef enum _DR_TYPE : int32_t {
    DR_TYPE_NONE = 0,
    DR_TYPE_WAV,
    DR_TYPE_MP3,
    DR_TYPE_COUNT
} DR_TYPE;

struct PaStreamState {
    float    ring[PA_RING_FRAMES * PA_OUTPUT_CHANNELS];  // stereo float, pre-converted
    volatile int writePos;      // advanced by fill thread (in frames, never wraps)
    volatile int readPos;       // advanced by audio callback (in frames, never wraps)
    DR_TYPE drType;
    union {
        drmp3 mp3;
        drwav wav;
    } dr;
    void* data;
    uint32_t dataLength;
    bool     looping;
    volatile bool active;
    volatile bool stopThread;
    std::thread   fillThread;
    std::mutex lock;
};


struct PaChannelState {
    // Loaded PCM (channels 0-39)
    int16_t* pcmData;           // decoded s16 samples; NULL for streaming channels
    int      pcmFrames;         // total sample frames
    float    pcmPos;            // fractional read position (for resampling)
    int      srcRate;           // original sample rate (e.g. 22050)
    int      srcChannels;       // 1 = mono, 2 = stereo

    // Playback state � volatile because the audio callback reads on another thread
    volatile float gain;
    volatile float pitch;       // speed ratio: 1.0 = normal, 2.0 = double speed
    volatile float panL;        // left  output gain  (0.0 � 1.0)
    volatile float panR;        // right output gain  (0.0 � 1.0)
    volatile bool  looping;
    volatile bool  playing;
    volatile bool  paused;

    // Streaming (channels 40-52 only; NULL for loaded channels)
    PaStreamState* stream;
};

struct PaLocal {
    PaStream* stream;                // the single PortAudio output stream
    PaChannelState  channels[53];          // 0-7: 2D, 8-39: 3D, 40-52: streamed
    PaStreamState   streamStates[13];      // backing store for stream channels
    MssEqInfo       eq[2];                 // EQ param storage � unchanged
    bool            isMultiChannel;
};

extern PaLocal paGlob;
extern snd_local_t g_snd;
extern const dvar_t* snd_khz;
extern const dvar_t* snd_outputConfiguration;

// snd_pa
bool PA_Startup();
bool PA_Init();
void PA_InitChannels();
void PA_InitEq();
void PA_ShutdownCleanup();
void PA_StartStreamFillThread(int si);
void PA_StopStreamFillThread(int si);

// Assert Defines
#define SNDALIASFLAGS_GET_TYPE(x) (x & 0xC0) >> 6

// Helper Functions
