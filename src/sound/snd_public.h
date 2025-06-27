#pragma once

#include <qcommon/qcommon.h>
#include <msslib/mss.h>
#include <gfx_d3d/fxprimitives.h>
#include <universal/memfile.h>

// snd

static const char *snd_eqTypeStrings[6] = { "lowpass", "highpass", "lowshelf", "highshelf", "bell", NULL }; // idb

enum SND_EQTYPE : __int32
{                                       // ...
    SND_EQTYPE_FIRST = 0x0,
    SND_EQTYPE_LOWPASS = 0x0,
    SND_EQTYPE_HIGHPASS = 0x1,
    SND_EQTYPE_LOWSHELF = 0x2,
    SND_EQTYPE_HIGHSHELF = 0x3,
    SND_EQTYPE_BELL = 0x4,
    SND_EQTYPE_LAST = 0x4,
    SND_EQTYPE_COUNT = 0x5,
    SND_EQTYPE_INVALID = 0x5,
};
enum snd_overlay_type_t : __int32
{                                       // ...
    SND_OVERLAY_NONE = 0x0,
    SND_OVERLAY_3D = 0x1,
    SND_OVERLAY_STREAM = 0x2,
    SND_OVERLAY_2D = 0x3,
};
enum snd_stopsounds_arg_t : __int32
{                                       // ...
    SND_STOP_ALL = 0x0,
    SND_KEEP_REVERB = 0x1,
    SND_KEEP_MUSIC = 0x2,
    SND_KEEP_AMBIENT = 0x4,
    SND_KEEP_MUSIC_AND_AMBIENT = 0x6,
    SND_STOP_STREAMED = 0x8,
    SND_KEEP_CHANNEL_VOLUMES = 0x10,
};
enum snd_alias_system_t : __int32
{                                       // ...
    SASYS_UI = 0x0,
    SASYS_CGAME = 0x1,
    SASYS_GAME = 0x2,
    SASYS_COUNT = 0x3,
};
enum SndLengthId : __int32
{                                       // ...
    SndLengthNotify_Script = 0x0,
    SndLengthNotify_Subtitle = 0x1,
    SndLengthNotifyCount = 0x2,
};
enum SndFileLoadingState : __int32
{                                       // ...
    SFLS_UNLOADED = 0x0,
    SFLS_LOADING = 0x1,
    SFLS_LOADED = 0x2,
};
struct snd_listener // sizeof=0x38
{                                       // ...
    orientation_t orient;               // ...
    int clientNum;                      // ...
    bool active;                        // ...
    // padding byte
    // padding byte
    // padding byte
};
// LWSS HACK: We use a slightly different version of MSS that has a +4 bigger struct.
 struct _AILSOUNDINFO_COD4 // sizeof=0x24
{                                       // ...
    int format;
    const void *data_ptr;               // ...
    unsigned int data_len;              // ...
    unsigned int rate;
    int bits;
    int channels;
    unsigned int samples;
    unsigned int block_size;
    const void *initial_ptr;            // ...
};
struct MssSoundCOD4 // sizeof=0x28
{
    _AILSOUNDINFO_COD4 info;
    unsigned __int8 *data;
};
// LWSS END
//struct MssSound // sizeof=0x2C
//{                                       // ...
//    _AILSOUNDINFO info;
//    unsigned __int8 *data;
//};
struct LoadedSound // sizeof=0x2C
{                                       // ...
    const char *name;
    MssSoundCOD4 sound;
};
struct StreamFileNameRaw // sizeof=0x8
{                                       // ...
    const char *dir;
    const char *name;
};
union StreamFileInfo // sizeof=0x8
{                                       // ...
    StreamFileNameRaw raw;
};
struct StreamFileName // sizeof=0x8
{                                       // ...
    StreamFileInfo info;
};
struct StreamedSound // sizeof=0x8
{                                       // ...
    StreamFileName filename;
};
union SoundFileRef // sizeof=0x8
{                                       // ...
    LoadedSound *loadSnd;
    StreamedSound streamSnd;
};
struct SoundFile // sizeof=0xC
{
    unsigned __int8 type;
    unsigned __int8 exists;
    // padding byte
    // padding byte
    SoundFileRef u;
};
struct SndCurve // sizeof=0x48
{                                       // ...
    const char *filename;               // ...
    int knotCount;                      // ...
    float knots[8][2];                  // ...
};
struct MSSSpeakerLevels // sizeof=0x10
{                                       // ...
    int speaker;
    int numLevels;
    float levels[2];
};
struct MSSChannelMap // sizeof=0x64
{                                       // ...
    int speakerCount;
    MSSSpeakerLevels speakers[6];
};
struct SpeakerMap // sizeof=0x198
{                                       // ...
    bool isDefault;
    // padding byte
    // padding byte
    // padding byte
    const char *name;                   // ...
    MSSChannelMap channelMaps[2][2];
};
struct SpeakerMapInfo // sizeof=0x1D8
{                                       // ...
    char name[64];
    SpeakerMap speakerMap;              // ...
};
const struct snd_alias_t // sizeof=0x5C
{
    const char *aliasName;
    const char *subtitle;
    const char *secondaryAliasName;
    const char *chainAliasName;
    SoundFile *soundFile;
    int sequence;
    float volMin;
    float volMax;
    float pitchMin;
    float pitchMax;
    float distMin;
    float distMax;
    int flags;
    float slavePercentage;
    float probability;
    float lfePercentage;
    float centerPercentage;
    int startDelay;
    SndCurve *volumeFalloffCurve;
    float envelopMin;
    float envelopMax;
    float envelopPercentage;
    SpeakerMap *speakerMap;
};
struct snd_alias_list_t // sizeof=0xC
{                                       // ...
    const char *aliasName;              // ...
    snd_alias_t *head;                  // ...
    int count;                          // ...
};
struct snd_entchannel_info_t // sizeof=0x50
{                                       // ...
    char name[64];
    int priority;                       // ...
    bool is3d;                          // ...
    bool isRestricted;                  // ...
    bool isPausable;                    // ...
    // padding byte
    int maxVoices;                      // ...
    int voiceCount;                     // ...
};

struct SndEntHandle_s // sizeof=0x4
{                                       // ...
    unsigned int entIndex;
};
union SndEntHandle // sizeof=0x4
{                                       // ...
    SndEntHandle()
    {
        handle = -1;
    }
    SndEntHandle(int entIndex)
    {
        field.entIndex = entIndex;
    }
    SndEntHandle_s field;
    int handle;
};
struct SndStartAliasInfo // sizeof=0x38
{                                       // ...
    const snd_alias_t *alias0;          // ...
    const snd_alias_t *alias1;          // ...
    float lerp;                         // ...
    SndEntHandle sndEnt;                // ...
    float org[3];                       // ...
    float volume;                       // ...
    float pitch;                        // ...
    int timeshift;                      // ...
    float fraction;                     // ...
    int startDelay;                     // ...
    bool master;                        // ...
    bool timescale;                     // ...
    // padding byte
    // padding byte
    snd_alias_system_t system;          // ...
};
struct SndFileSpecificChannelInfo // sizeof=0x10
{                                       // ...
    SndFileLoadingState loadingState;   // ...
    int srcChannelCount;                // ...
    int baserate;                       // ...
    int endtime;                        // ...
};
struct sndLengthNotifyInfo // sizeof=0x24
{                                       // ...
    SndLengthId id[4];
    void *data[4];
    int count;
};
struct snd_channel_info_t // sizeof=0x8C
{                                       // ...
    SndFileSpecificChannelInfo soundFileInfo; // ...
    SndEntHandle sndEnt;                // ...
    int entchannel;                     // ...
    int startDelay;                     // ...
    int startTime;                      // ...
    int looptime;                       // ...
    int totalMsec;                      // ...
    int playbackId;                     // ...
    sndLengthNotifyInfo lengthNotifyInfo; // ...
    float basevolume;                   // ...
    float pitch;
    const snd_alias_t *alias0;          // ...
    const snd_alias_t *alias1;          // ...
    int saveIndex0;
    int saveIndex1;
    float lerp;                         // ...
    float org[3];                       // ...
    float offset[3];                    // ...
    bool paused;                        // ...
    bool master;                        // ...
    bool timescale;                     // ...
    // padding byte
    snd_alias_system_t system;          // ...
};
struct snd_volume_info_t // sizeof=0xC
{                                       // ...
    float volume;                       // ...
    float goalvolume;                   // ...
    float goalrate;                     // ...
};
struct snd_overlay_info_t // sizeof=0x110
{                                       // ...
    char pszSampleName[128];            // ...
    char aliasName[64];                 // ...
    char entchannel[64];                // ...
    float fBaseVolume;                  // ...
    float fCurVolume;                   // ...
    int dist;                           // ...
    float fPitch;                       // ...
};

struct snd_local_t_restore// sizeof=0x4008
{                                       // ...
    unsigned __int8 buffer[16384];
    int size;                           // ...
    bool compress;                      // ...
    // padding byte
    // padding byte
    // padding byte
};
struct snd_channelvolgroup // sizeof=0x304
{                                       // ...
    snd_volume_info_t channelvol[64];
    bool active;                        // ...
    // padding byte
    // padding byte
    // padding byte
};
struct snd_background_info_t // sizeof=0x8
{                                       // ...
    float goalvolume;                   // ...
    float goalrate;                     // ...
};
struct snd_enveffect // sizeof=0x20
{                                       // ...
    int roomtype;
    float drylevel;
    float drygoal;
    float dryrate;
    float wetlevel;
    float wetgoal;
    float wetrate;
    bool active;                        // ...
    // padding byte
    // padding byte
    // padding byte
};
struct snd_amplifier // sizeof=0x18
{                                       // ...
    snd_listener *listener;             // ...
    int minRadius;
    int maxRadius;                      // ...
    float falloffExp;
    float minVol;
    float maxVol;
};
struct snd_physics_info // sizeof=0x10
{                                       // ...
    snd_alias_list_t *aliasList;        // ...
    float org[3];                       // ...
};
struct snd_physics // sizeof=0x204
{                                       // ...
    snd_physics_info info[32];          // ...
    int count;                          // ...
};
struct snd_local_t // sizeof=0x7EF8
{                                       // ...
    bool Initialized2d;                 // ...
    bool Initialized3d;                 // ...
    bool paused;                        // ...
    // padding byte
    int playbackIdCounter;              // ...
    unsigned int playback_rate;         // ...
    int playback_channels;              // ...
    float timescale;                    // ...
    int pausetime;                      // ...
    int cpu;                            // ...
    snd_local_t_restore restore; // ...
    float volume;                       // ...
    snd_volume_info_t mastervol;        // ...
    snd_channelvolgroup channelVolGroups[4]; // ...
    snd_channelvolgroup *channelvol;    // ...
    snd_background_info_t background[5]; // ...
    int ambient_track;                  // ...
    float slaveLerp;                    // ...
    snd_enveffect envEffects[3];        // ...
    snd_enveffect *effect;              // ...
    bool defaultPauseSettings[64];      // ...
    bool pauseSettings[64];             // ...
    snd_listener listeners[2];          // ...
    int time;                           // ...
    int looptime;                       // ...
    snd_amplifier amplifier;            // ...
    snd_entchannel_info_t entchaninfo[64]; // ...
    int entchannel_count;               // ...
    snd_channel_info_t chaninfo[53];    // ...
    int max_2D_channels;                // ...
    int max_3D_channels;                // ...
    int max_stream_channels;            // ...
};

void __cdecl TRACK_snd();
void __cdecl SND_DebugAliasPrint(bool condition, const snd_alias_t *alias, const char *msg);
int __cdecl SND_GetEntChannelCount();
bool __cdecl SND_IsStreamChannelLoading(int index);
bool __cdecl SND_HasFreeVoice(int entchannel);
void __cdecl SND_AddVoice(int entchannel);
void __cdecl SND_RemoveVoice(int entchannel);
int __cdecl SND_GetPriority(int entchannel);
bool __cdecl SND_IsRestricted(int entchannel);
bool __cdecl SND_IsAliasChannel3D(int entchannel);
bool __cdecl SND_IsPausable(int entchannel);
snd_entchannel_info_t *__cdecl SND_GetEntChannelName(int entchannel);
int __cdecl SND_GetEntChannelFromName(const char *channelName);
char __cdecl SND_ValidateEnvEffectsPriorityValue(const char *priorityName, int *priority);
void __cdecl SND_SetEnvironmentEffects_f();
int __cdecl SND_RoomtypeFromString(const char *string);
void __cdecl SND_DeactivateEnvironmentEffects_f();
void __cdecl SND_SetEq_f();
SND_EQTYPE __cdecl SND_EqTypeFromString(const char *typeString);
char __cdecl SND_ParseChannelAndBand_f(int *entchannel, int *eqIndex, int *band);
void __cdecl SND_SetEqFreq_f();
void __cdecl SND_SetEqType_f();
void __cdecl SND_SetEqGain_f();
void __cdecl SND_SetEqQ_f();
void __cdecl SND_DeactivateEq_f();
char __cdecl SND_AnyActiveListeners();
int __cdecl SND_GetListenerIndexNearestToOrigin(const float *origin);
void __cdecl SND_DisconnectListener(int localClientNum);
void __cdecl SND_SetListener(int localClientNum, int clientNum, const float *origin, const float (*axis)[3]);
void __cdecl SND_SaveListeners(snd_listener *listeners);
void __cdecl SND_RestoreListeners(snd_listener *listeners);
int __cdecl SND_SetPlaybackIdNotPlayed(unsigned int index);
int __cdecl SND_AcquirePlaybackId(unsigned int index, int totalMsec);
char __cdecl SND_AddLengthNotify(int playbackId, const snd_alias_t *lengthNotifyData, SndLengthId id);
void __cdecl DoLengthNotify(int msec, const snd_alias_t *lengthNotifyData, SndLengthId id);
char __cdecl SND_GetKnownLength(int playbackId, int *msec);
double __cdecl SND_GetLerpedSlavePercentage(float baseSlavePercentage);
double __cdecl SND_Attenuate(SndCurve *volumeFalloffCurve, float radius, float mindist, float maxdist);
void __cdecl SND_GetCurrent3DPosition(SndEntHandle sndEnt, float *offset, float *pos_out);
void __cdecl SND_ResetChannelInfo(int index);
void __cdecl SND_SetChannelStartInfo(unsigned int index, SndStartAliasInfo *SndStartAliasInfo);
void __cdecl SND_SetSoundFileChannelInfo(
    unsigned int index,
    int srcChannelCount,
    int baserate,
    int total_msec,
    int start_msec,
    SndFileLoadingState loadingState);
int __cdecl SND_FindFree2DChannel(SndStartAliasInfo *startAliasInfo, int entchannel);
int __cdecl SND_FindReplaceableChannel(
    SndStartAliasInfo *startAliasInfo,
    int entchannel,
    unsigned int first,
    int count);
int __cdecl SND_FindFree3DChannel(SndStartAliasInfo *startAliasInfo, int entchannel);
void __cdecl DB_SaveSounds();
void __cdecl SND_Archive(snd_channel_info_t *chaninfo);
void __cdecl DB_LoadSounds();
void __cdecl SND_Unarchive(snd_channel_info_t *chaninfo);
void __cdecl SND_StopSoundAliasOnEnt(SndEntHandle sndEnt, const char *aliasName);
void __cdecl StopSoundAliasesOnEnt(SndEntHandle sndEnt, const char *aliasName);
void __cdecl SND_StopSoundsOnEnt(SndEntHandle sndEnt);
void __cdecl SND_InitFXSounds();
void __cdecl SND_AddPlayFXSoundAlias(snd_alias_t *alias, SndEntHandle sndEnt, const float *origin);
void __cdecl Snd_AssertAliasValid(snd_alias_t *alias);
void __cdecl SND_PlayFXSounds();
int __cdecl SND_PlaySoundAlias(
    const snd_alias_t *alias,
    SndEntHandle sndEnt,
    const float *org,
    int timeshift,
    snd_alias_system_t system);
int __cdecl SND_PlaySoundAlias_Internal(
    const snd_alias_t *alias0,
    const snd_alias_t *alias1,
    float lerp,
    float volumeScale,
    SndEntHandle sndEnt,
    const float *org,
    int *pChannel,
    int timeshift,
    bool treatAsMaster,
    bool useTimescale,
    snd_alias_system_t system);
void __cdecl SND_StopEntityChannel(SndEntHandle sndEnt, int entchannel);
int __cdecl SND_StartAliasSample(SndStartAliasInfo *startAliasInfo, int *pChannel);
int __cdecl SND_StartAliasStream(SndStartAliasInfo *startAliasInfo, int *pChannel);
int __cdecl SND_FindFreeStreamChannel(SndStartAliasInfo *startAliasInfo, int entchannel);
void __cdecl SND_ChoosePitchAndVolume(
    const snd_alias_t *alias0,
    const snd_alias_t *alias1,
    float lerp,
    float volumeScale,
    float *volume,
    float *pitch);
char __cdecl SND_ContinueLoopingSound(
    const snd_alias_t *alias0,
    const snd_alias_t *alias1,
    float lerp,
    float volumeScale,
    SndEntHandle sndEnt,
    const float *org,
    int *pChannel);
void __cdecl SND_ContinueLoopingSound_Internal(
    unsigned int chanIndex,
    float lerp,
    float volumeScale,
    int *pChannel,
    void(__cdecl *setPlaybackRateFunc)(int, int));
bool __cdecl SND_IsNullSoundFile(const SoundFile *soundFile);
int __cdecl SND_PlaySoundAliasAsMaster(
    const snd_alias_t *alias,
    SndEntHandle sndEnt,
    const float *org,
    int timeshift,
    snd_alias_system_t system);
int __cdecl SND_PlayBlendedSoundAliases(
    const snd_alias_t *alias0,
    const snd_alias_t *alias1,
    float lerp,
    float volumeScale,
    SndEntHandle sndEnt,
    const float *org,
    int timeshift,
    snd_alias_system_t system);
char __cdecl SND_ValidateSoundAliasBlend(const snd_alias_t *alias0, const snd_alias_t *alias1, bool bReport);
int __cdecl SND_PlayLocalSoundAlias(unsigned int localClientNum, const snd_alias_t *alias, snd_alias_system_t system);
int __cdecl SND_PlayLocalSoundAliasByName(
    unsigned int localClientNum,
    const char *aliasname,
    snd_alias_system_t system);
void __cdecl SND_ResetPauseSettingsToDefaults();
void __cdecl SND_PlayMusicAlias(
    int localClientNum,
    const snd_alias_t *alias,
    bool useTimescale,
    snd_alias_system_t system);
void __cdecl SND_StartBackground(
    int localClientNum,
    unsigned int track,
    const snd_alias_t *alias,
    int fadetime,
    float fraction,
    bool useTimescale,
    snd_alias_system_t system);
void SND_UpdatePause();
int SND_PauseSounds();
void SND_UnpauseSounds();
void __cdecl SND_StopMusic(int fadetime);
void __cdecl SND_StopBackground(unsigned int track, int fadetime);
void __cdecl SND_PlayAmbientAlias(
    int localClientNum,
    const snd_alias_t *alias,
    int fadetime,
    snd_alias_system_t system);
void __cdecl SND_StopAmbient(int localClientNum, int fadetime);
void __cdecl SND_FadeAllSounds(float volume, int fadetime);
void __cdecl SND_SetChannelVolumes(int priority, const float *channelvolume, int fademsec);
void __cdecl SND_DeactivateChannelVolumes(int priority, int fademsec);
void __cdecl SND_UpdateLoopingSounds();
char __cdecl SND_UpdateBackgroundVolume(unsigned int track, int frametime);
void __cdecl SND_SetEnvironmentEffects(
    int priority,
    const char *roomstring,
    float drylevel,
    float wetlevel,
    int fademsec);
void __cdecl SND_DeactivateEnvironmentEffects(int priority, int fademsec);
void __cdecl SND_UpdateReverbs();
void __cdecl SND_DeactivateAllEq(int eqIndex);
void __cdecl SND_DeactivateChannelEq(const char *channelName, int eqIndex);
void __cdecl SND_DeactivateEq(const char *channelName, int eqIndex, unsigned int band);
void __cdecl SND_Update();
void __cdecl SND_UpdateMasterVolumes(int frametime);
void __cdecl SND_UpdateVolume(snd_volume_info_t *volinfo, int frametime);
void __cdecl SND_UpdateAllChannels(int frametime);
void __cdecl SND_UpdateSlaveLerp(int frametime);
bool __cdecl SND_Is2DChannelPlaying(int index);
bool __cdecl SND_Is3DChannelPlaying(int index);
bool __cdecl SND_IsStreamChannelPlaying(int index);
void __cdecl SND_UpdateRoomEffects(int frametime);
void SND_UpdateTimeScale();
void __cdecl DebugDrawWorldSounds(int debugDrawStyle);
void __cdecl DebugDrawWorldSound3D(
    unsigned int idx,
    int debugDrawStyle,
    int *offsets,
    int *closestId,
    float *closestIdDotProd);
double __cdecl FontSizeForDistance(float distance);
void SND_UpdatePhysics();
bool __cdecl SND_ShouldGiveCpuWarning();
void __cdecl SND_StopSounds(snd_stopsounds_arg_t which);
void __cdecl SND_Init();
void __cdecl SND_PlayLocal_f();
void __cdecl RelativeToListener(const snd_listener *listener, float yaw, float pitch, float dist, float *result);
void SND_InitEntChannels();
void __cdecl SND_ParseEntChannelFile(const char *buffer);
char __cdecl SND_BooleanFromString(const char *value, const char *trueValue, const char *falseValue, bool defaultValue);
void __cdecl SND_Shutdown();
void __cdecl SND_ShutdownChannels();
void __cdecl SND_ErrorCleanup();
void __cdecl SND_Save(struct MemoryFile *memFile);
void __cdecl SND_Save3DChannel(int chanIndex, struct MemoryFile *memFile);
void __cdecl SND_SaveSoundAlias(const snd_alias_t *alias, struct MemoryFile *memFile);
void __cdecl SND_SaveChanInfo(const snd_channel_info_t *chaninfo, struct MemoryFile *memFile);
void __cdecl SND_SaveLengthNotifyInfo(const sndLengthNotifyInfo *info, struct MemoryFile *memFile);
void __cdecl SND_Save2DChannel(int chanIndex, struct MemoryFile *memFile);
void __cdecl SND_SaveStreamChannel(int chanIndex, struct MemoryFile *memFile);
void __cdecl SND_Restore(struct MemoryFile *memFile);
char __cdecl SND_Restore3DChannel(struct MemoryFile *memFile);
snd_alias_t *__cdecl SND_RestoreSoundAlias(struct MemoryFile *memFile);
void __cdecl SND_RestoreChanInfo(snd_channel_info_t *chaninfo, struct MemoryFile *memFile);
void __cdecl SND_RestoreLengthNotifyInfo(struct MemoryFile *memFile, sndLengthNotifyInfo *info);
char __cdecl SND_Restore2DChannel(struct MemoryFile *memFile);
char __cdecl SND_RestoreStreamChannel(int channel, struct MemoryFile *memFile);
int __cdecl SND_GetSoundOverlay(snd_overlay_type_t type, snd_overlay_info_t *info, int maxcount, int *cpu);
int __cdecl SND_GetSoundOverlay2D(snd_overlay_info_t *info, int maxcount);
int __cdecl SND_GetSoundOverlay3D(snd_overlay_info_t *info, int maxcount);
int __cdecl SND_GetSoundOverlayStream(snd_overlay_info_t *info, int maxcount);
void __cdecl SND_StopChannelAndPlayChainAlias(unsigned int chanId);
void __cdecl StopChannel(int chanId);
void __cdecl SND_AddPhysicsSound(snd_alias_list_t *aliasList, float *org);
double __cdecl SND_GetVolumeNormalized();
void __cdecl SND_SetHWND(HWND hwnd);
void __cdecl SND_SetData(MssSoundCOD4 *mssSound, void *srcData);

// snd_driver_load_obj
struct LoadedSound *__cdecl SND_LoadSoundFile(const char *name);

extern const dvar_t *snd_cinematicVolumeScale;
extern const dvar_t *snd_enable3D;
extern const dvar_t *snd_enableEq;
extern const dvar_t *snd_debugReplace;
extern const dvar_t *snd_debugAlias;
extern const dvar_t *snd_enable2D;
extern const dvar_t *snd_khz;
extern const dvar_t *snd_draw3D;
extern const dvar_t *snd_volume;
extern const dvar_t *snd_errorOnMissing;
extern const dvar_t *snd_drawEqEnts;
extern const dvar_t *snd_enableReverb;
extern const dvar_t *snd_bits;
extern const dvar_t *snd_slaveFadeTime;
extern const dvar_t *snd_enableStream;
extern const dvar_t *snd_drawEqChannels;
extern const dvar_t *snd_levelFadeTime;
extern const dvar_t *snd_touchStreamFilesOnLoad;