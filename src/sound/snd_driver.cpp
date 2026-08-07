#include <universal/q_shared.h>
#include "snd_local.h"
#include "snd_public.h"
#include <qcommon/mem_track.h>
#include <msslib/mss.h>
#include <qcommon/qcommon.h>
#include <universal/com_files.h>
#include <gfx_d3d/r_cinematic.h>
#include <universal/com_sndalias.h>
#include <universal/profile.h>

#ifdef KISAK_MP
#include <cgame_mp/cg_local_mp.h>
#elif KISAK_SP
#include <cgame/cg_main.h>
#endif

#ifdef KISAK_SOUND
// Declarations only - DR_WAV_IMPLEMENTATION/DR_MP3_IMPLEMENTATION are generated exactly
// once, in snd_driver_load_obj.cpp (see Phase 3).
#include <dr_libs/dr_wav.h>
#include <dr_libs/dr_mp3.h>
#include <AL/efx-presets.h>
#include <fstream>
#endif

#ifndef KISAK_SOUND
MssLocal milesGlob;
#else
AlLocal alGlob;
#endif

const dvar_t *snd_khz;
const dvar_t *snd_outputConfiguration;

void __cdecl TRACK_snd_driver()
{
#ifndef KISAK_SOUND
    track_static_alloc_internal(&milesGlob, 9936, "milesGlob", 13);
#else
    track_static_alloc_internal(&alGlob, sizeof(alGlob), "alGlob", 13);
#endif
}

bool __cdecl SND_IsMultiChannel()
{
#ifndef KISAK_SOUND
    return milesGlob.isMultiChannel;
#else
    return alGlob.isMultiChannel;
#endif
}

char __cdecl SND_InitDriver()
{
#if KISAK_DEDICATED
    return 0;
#else

    snd_khz = Dvar_RegisterInt("snd_khz", 44, (DvarLimits)0x2C0000000BLL, DVAR_ARCHIVE | DVAR_LATCH, "The game sound frequency.");
#ifndef KISAK_SOUND
    AIL_set_file_callbacks(MSS_FileOpenCallback, MSS_FileCloseCallback, MSS_FileSeekCallback, MSS_FileReadCallback);
    AIL_set_redist_directory("miles");
#endif
    snd_outputConfiguration = Dvar_RegisterEnum(
        "snd_outputConfiguration",
        snd_outputConfigurationStrings,
        0,
        DVAR_ARCHIVE | DVAR_LATCH,
        "Sound output configuration");
#ifndef KISAK_SOUND
    if (MSS_Startup())
    {
        MSS_open_digital_driver(11025, 2, 2);
        AIL_shutdown();
    }
#endif
    if (MSS_Startup())
    {
        if (MSS_Init())
        {
            MSS_InitChannels();
            MSS_InitEq();
            return 1;
        }
        else
        {
#ifndef KISAK_SOUND
            AIL_shutdown();
#endif
            MSS_ShutdownCleanup();
            MSS_InitFailed();
            return 0;
        }
    }
    else
    {
        MSS_InitFailed();
        return 0;
    }
#endif // KISAK_DEDICATED
}

void __cdecl SND_ShutdownDriver()
{
    R_Cinematic_StopPlayback();
    R_Cinematic_SyncNow();
#if KISAK_DEDICATED
    return;
#else

#ifndef KISAK_SOUND
    AIL_shutdown();
#endif // KISAK_SOUND
    MSS_ShutdownCleanup();

#endif // KISAK_DEDICATED
}

int __cdecl SND_GetDriverCPUPercentage()
{
#if KISAK_DEDICATED
    return 0;
#else

#ifndef KISAK_SOUND
    return AIL_digital_CPU_percent(milesGlob.driver);
#else
    // KISAK_SOUND TODO: no direct OpenAL equivalent to Miles' per-driver CPU usage stat.
    return 0;
#endif

#endif // KISAK_DEDICATED
}

void __cdecl SND_Set3DPosition(int index, const float *org)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels);

#ifndef KISAK_SOUND
    float v2; // [esp+0h] [ebp-28h]
    float delta[3]; // [esp+Ch] [ebp-1Ch] BYREF
    int listenerIndex; // [esp+18h] [ebp-10h]
    float transformed[3]; // [esp+1Ch] [ebp-Ch] BYREF

    listenerIndex = SND_GetListenerIndexNearestToOrigin(org);
    Vec3Sub(org, g_snd.listeners[listenerIndex].orient.origin, delta);
    MatrixTransposeTransformVector(delta, g_snd.listeners[listenerIndex].orient.axis, transformed);
    v2 = -transformed[1];
    AIL_set_sample_3D_position(
        milesGlob.handle_sample[index],
        v2,
        transformed[2],
        transformed[0]);
#else
    // Same listener-space transform as the Miles branch above (Miles' listener never moves
    // either - see the fixed AL listener setup in MSS_Init, snd_al.cpp). Only the axis swap
    // differs: Miles is left-handed X-right/Y-up/Z-forward (its call below sends
    // (-transformed[1], transformed[2], transformed[0]) = (right, up, forward)); OpenAL is
    // right-handed X-right/Y-up/Z-BACKWARD (-Z is forward, matching the AL_ORIENTATION set
    // in MSS_Init). Same X/Y, so only the forward component's sign flips.
    // NOT YET VERIFIED in-game (see WORK.md Phase 9) - this is a derivation from each SDK's
    // documented convention, not a tested one; confirm real left/right/front/back audio
    // before trusting it.
    float delta[3];
    int listenerIndex = SND_GetListenerIndexNearestToOrigin(org);
    Vec3Sub(org, g_snd.listeners[listenerIndex].orient.origin, delta);
    float transformed[3];
    MatrixTransposeTransformVector(delta, g_snd.listeners[listenerIndex].orient.axis, transformed);
    alSource3f(alGlob.source[index], AL_POSITION, -transformed[1], transformed[2], -transformed[0]);
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

#ifdef KISAK_SOUND
// Detaches and deletes the channel's currently-attached AL buffer, if any. Called before a
// channel starts playing a new sound (defensive - normally the channel allocator only hands
// out channels that were already stopped) and when a channel is explicitly stopped, since
// SND_StartAlias2D/3DSample generates a fresh buffer per play rather than caching one per
// SoundFile (see AlLocal::channelBuffer's comment in snd_local.h).
void SND_ReleaseChannelBuffer(int index)
{
    if (alGlob.channelBuffer[index])
    {
        alSourcei(alGlob.source[index], AL_BUFFER, 0);
        alDeleteBuffers(1, &alGlob.channelBuffer[index]);
        alGlob.channelBuffer[index] = 0;
    }
}

// Defined near SND_SetRoomtype (Phase 6) below; forward-declared here since
// SND_StartAlias2D/3DSample and SND_StartAliasStreamOnChannel call it before that point.
void SND_ApplyReverbSend(int index, const snd_alias_t *alias);

struct AlStreamState
{
    bool active;
    bool isMp3;
    bool looping;
    int fsHandle;
    drwav wav;
    drmp3 mp3;
    uint32_t channels;
    uint32_t sampleRate;
    // Frames decoded/queued since the current loop pass began (reset to the post-seek frame
    // index on start, reset again each time SND_FillStreamBuffers loops back to frame 0).
    // Needed because AL_SEC_OFFSET on a queued streaming source only reports the offset
    // *within the currently-playing buffer chunk*, not a running position across the whole
    // stream - there's no direct AL query for that, so SND_GetStreamChannelSaveInfo's
    // fraction is approximated from this instead (see its comment for the small, bounded
    // error this introduces: up to one buffer queue's worth of look-ahead).
    uint64_t framesQueued;
};

// Per-stream-channel decoder state. Indices 40-52 are the only ones ever used (matching
// g_snd's stream channel range), sized to 53 anyway for the same direct-index-equals-
// channel-index convention as AlLocal::source/channelBuffer. Kept file-static here rather
// than in AlLocal/snd_local.h so that widely-included header doesn't force every file that
// includes it (e.g. r_cinematic.cpp, which has nothing to do with streaming) to also pull
// in dr_wav.h/dr_mp3.h.
static AlStreamState g_streamState[53];

static const int AL_STREAM_BUFFER_COUNT = 4;
static const int AL_STREAM_BUFFER_FRAMES = 8192;

// Miles' MSS_File*Callback (snd_mss.cpp) bridges AIL's file I/O to FS_*; these do the same
// for dr_wav/dr_mp3, since neither library has a concept of a game virtual filesystem.
// Format extension is reachable from data (see WORK.md Phase 5 - Com_GetSoundFileName's
// filename comes straight from the asset CSV, not hardcoded to .wav), so both are wired up.
static size_t AL_StreamReadCallback(void *pUserData, void *pBufferOut, size_t bytesToRead)
{
    AlStreamState *stream = (AlStreamState *)pUserData;
    return FS_Read((uint8_t *)pBufferOut, (uint32_t)bytesToRead, stream->fsHandle);
}

// FS_Seek's zip-archive branch (com_files.cpp, used whenever a streamed sound is packed into an
// .iwd rather than sitting as a loose file on disk) doesn't implement standard SEEK_SET/CUR/END
// semantics - its three origin values are rotated by one relative to what every caller (including
// the original Miles snd_mss.cpp callback, and dr_wav/dr_mp3 here) assumes:
//   want SEEK_SET (absolute)         -> must pass origin 2 to FS_Seek
//   want SEEK_CUR (relative-to-here) -> must pass origin 0 to FS_Seek
//   want SEEK_END (relative-to-EOF)  -> must pass origin 1 to FS_Seek
// This is inherited unmodified from the original decompiled engine, so it isn't safe to fix inside
// FS_Seek itself without risking every other zip-file seek in the game (save data, Miles' own
// zip-streamed audio, etc.) - instead, rotate only here, and only for zip-backed handles; loose
// files go through a plain FS_FileSeek/fseek and are unaffected by this.
static int AL_FixupZipSeekOrigin(int fsHandle, int trueOrigin)
{
    static const int kZipOriginRotation[3] = { 2, 0, 1 }; // SEEK_SET->2, SEEK_CUR->0, SEEK_END->1
    return FS_IsFileInZip(fsHandle) ? kZipOriginRotation[trueOrigin] : trueOrigin;
}

static drwav_bool32 AL_WavSeekCallback(void *pUserData, int offset, drwav_seek_origin origin)
{
    AlStreamState *stream = (AlStreamState *)pUserData;
    FS_Seek(stream->fsHandle, offset, AL_FixupZipSeekOrigin(stream->fsHandle, (int)origin));
    return DRWAV_TRUE;
}

static drwav_bool32 AL_WavTellCallback(void *pUserData, drwav_int64 *pCursor)
{
    AlStreamState *stream = (AlStreamState *)pUserData;
    *pCursor = FS_FTell(stream->fsHandle);
    return DRWAV_TRUE;
}

static drmp3_bool32 AL_Mp3SeekCallback(void *pUserData, int offset, drmp3_seek_origin origin)
{
    AlStreamState *stream = (AlStreamState *)pUserData;
    FS_Seek(stream->fsHandle, offset, AL_FixupZipSeekOrigin(stream->fsHandle, (int)origin));
    return DRMP3_TRUE;
}

static drmp3_bool32 AL_Mp3TellCallback(void *pUserData, drmp3_int64 *pCursor)
{
    AlStreamState *stream = (AlStreamState *)pUserData;
    *pCursor = FS_FTell(stream->fsHandle);
    return DRMP3_TRUE;
}

// Stops, unqueues/deletes all buffers, closes the decoder and file handle for a stream
// channel. Safe to call on an already-inactive channel (no-op).
static void SND_ReleaseStreamChannel(int index)
{
    AlStreamState *stream = &g_streamState[index];
    if (!stream->active)
        return;

    alSourceStop(alGlob.source[index]);
    ALint queued = 0;
    alGetSourcei(alGlob.source[index], AL_BUFFERS_QUEUED, &queued);
    while (queued-- > 0)
    {
        ALuint buf;
        alSourceUnqueueBuffers(alGlob.source[index], 1, &buf);
        alDeleteBuffers(1, &buf);
    }
    alSourcei(alGlob.source[index], AL_BUFFER, 0);

    if (stream->isMp3)
        drmp3_uninit(&stream->mp3);
    else
        drwav_uninit(&stream->wav);
    FS_FCloseFile(stream->fsHandle);
    stream->active = false;
}

// Decodes and queues fresh PCM chunks to bring the channel's buffer queue back up to
// AL_STREAM_BUFFER_COUNT, first unqueuing/deleting any buffers that finished playing.
// Handles looping by seeking back to the start of the audio data when the decoder runs dry.
// Returns false once the stream has ended (not looping) with nothing left queued - the
// caller uses that to know when the channel should stop/free itself.
static bool SND_FillStreamBuffers(int index)
{
    AlStreamState *stream = &g_streamState[index];
    if (!stream->active)
        return false;

    ALuint source = alGlob.source[index];

    ALint processed = 0;
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
    while (processed-- > 0)
    {
        ALuint buf;
        alSourceUnqueueBuffers(source, 1, &buf);
        alDeleteBuffers(1, &buf);
    }

    ALint queued = 0;
    alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);

    // Reused across calls rather than stack-allocated (32KB) each time; safe because stream
    // channels are always filled one at a time, never interleaved, on a single thread.
    static int16_t chunk[AL_STREAM_BUFFER_FRAMES * 2]; // *2 for stereo worst case

    for (int i = queued; i < AL_STREAM_BUFFER_COUNT; ++i)
    {
        uint64_t framesRead = stream->isMp3
            ? drmp3_read_pcm_frames_s16(&stream->mp3, AL_STREAM_BUFFER_FRAMES, chunk)
            : drwav_read_pcm_frames_s16(&stream->wav, AL_STREAM_BUFFER_FRAMES, chunk);

        if (framesRead == 0)
        {
            if (!stream->looping)
                break;
            if (stream->isMp3)
                drmp3_seek_to_pcm_frame(&stream->mp3, 0);
            else
                drwav_seek_to_pcm_frame(&stream->wav, 0);
            framesRead = stream->isMp3
                ? drmp3_read_pcm_frames_s16(&stream->mp3, AL_STREAM_BUFFER_FRAMES, chunk)
                : drwav_read_pcm_frames_s16(&stream->wav, AL_STREAM_BUFFER_FRAMES, chunk);
            if (framesRead == 0)
                break; // zero-length file - avoid spinning forever
            stream->framesQueued = 0; // starting a fresh loop pass
        }

        ALuint buffer;
        alGenBuffers(1, &buffer);
        ALenum format = (stream->channels == 2) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
        alBufferData(buffer, format, chunk, (ALsizei)(framesRead * stream->channels * sizeof(int16_t)), stream->sampleRate);
        alSourceQueueBuffers(source, 1, &buffer);
        stream->framesQueued += framesRead;
        ++queued;
    }

    return queued > 0;
}
#endif

void __cdecl SND_Stop2DChannel(int index)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert((index >= 0 && index < 0 + g_snd.max_2D_channels));

#ifndef KISAK_SOUND
    AIL_end_sample(milesGlob.handle_sample[index]);
#else
    SND_ReleaseChannelBuffer(index);
#endif
    SND_ResetChannelInfo(index);
    SND_RemoveVoice(g_snd.chaninfo[index].entchannel);

#endif // KISAK_DEDICATED
}

void __cdecl SND_Pause2DChannel(int index)
{
#if KISAK_DEDICATED
    return;
#else

    iassert(index >= 0 && index < 0 + g_snd.max_2D_channels);

#ifndef KISAK_SOUND
    AIL_stop_sample(milesGlob.handle_sample[index]);
#else
    alSourcePause(alGlob.source[index]);
#endif // KISAK_SOUND
    g_snd.chaninfo[index].paused = 1;

#endif // KISAK_DEDICATED
}

void __cdecl SND_Unpause2DChannel(int index, int timeshift)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert(index >= 0 && index < 0 + g_snd.max_2D_channels);

    if (!g_snd.chaninfo[index].startDelay)
    {
#ifndef KISAK_SOUND
        AIL_resume_sample(milesGlob.handle_sample[index]);
#else
        alSourcePlay(alGlob.source[index]);
#endif // KISAK_SOUND
    }

    g_snd.chaninfo[index].soundFileInfo.endtime += timeshift;
    g_snd.chaninfo[index].startTime += timeshift;
    g_snd.chaninfo[index].paused = 0;

#endif // KISAK_DEDICATED
}

bool __cdecl SND_Is2DChannelFree(int index)
{
    iassert(index >= 0 && index < 0 + g_snd.max_2D_channels);

    return !g_snd.chaninfo[index].paused && !g_snd.chaninfo[index].startDelay && g_snd.chaninfo[index].alias0 == 0;
}

void __cdecl SND_Stop3DChannel(int index)
{
#ifdef KISAK_DEDICATED
    return;
#else
    iassert(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels);

#ifndef KISAK_SOUND
    AIL_end_sample(milesGlob.handle_sample[index]);
#else
    SND_ReleaseChannelBuffer(index);
#endif // KISAK_SOUND
    SND_ResetChannelInfo(index);
    SND_RemoveVoice(g_snd.chaninfo[index].entchannel);

#endif // KISAK_DEDICATED
}

void __cdecl SND_Pause3DChannel(int index)
{
    iassert(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels);
#ifdef KISAK_DEDICATED
    return;
#else

#ifndef KISAK_SOUND
    AIL_stop_sample(milesGlob.handle_sample[index]);
#else
    alSourcePause(alGlob.source[index]);
#endif
    g_snd.chaninfo[index].paused = 1;

#endif // KISAK_DEDICATED
}

void __cdecl SND_Unpause3DChannel(int index, int timeshift)
{
#ifdef KISAK_DEDICATED
    return;
#else
    iassert(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels);

    if (!g_snd.chaninfo[index].startDelay)
    {
#ifndef KISAK_SOUND
        AIL_resume_sample(milesGlob.handle_sample[index]);
#else
        alSourcePlay(alGlob.source[index]);
#endif
    }

    g_snd.chaninfo[index].soundFileInfo.endtime += timeshift;
    g_snd.chaninfo[index].startTime += timeshift;
    g_snd.chaninfo[index].paused = 0;
#endif // KISAK_DEDICATED
}

bool __cdecl SND_Is3DChannelFree(int index)
{
    iassert(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels);

    return !g_snd.chaninfo[index].paused && !g_snd.chaninfo[index].startDelay && g_snd.chaninfo[index].alias0 == 0;
}

void __cdecl SND_StopStreamChannel(int index)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert(index >= SND_FIRST_STREAM_CHANNEL && index < SND_FIRST_STREAM_CHANNEL + g_snd.max_stream_channels);

#ifndef KISAK_SOUND
    if (!milesGlob.handle_sample[index])
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            725,
            0,
            "%s",
            "milesGlob.handle_stream[index - SND_FIRST_STREAM_CHANNEL]");
    //if (!milesGlob.handle_sample[index]->)
    //    MyAssertHandler(
    //        ".\\win32\\snd_driver.cpp",
    //        726,
    //        0,
    //        "%s",
    //        "milesGlob.handle_stream[index - SND_FIRST_STREAM_CHANNEL]->samp");
    AIL_close_stream((HSTREAM)milesGlob.handle_sample[index]);
    milesGlob.handle_sample[index] = 0;
#else
    SND_ReleaseStreamChannel(index);
#endif // KISAK_SOUND
    SND_ResetChannelInfo(index);
    SND_RemoveVoice(g_snd.chaninfo[index].entchannel);

#endif // KISAK_DEDICATED
}

void __cdecl SND_PauseStreamChannel(int index)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert(index >= SND_FIRST_STREAM_CHANNEL && index < SND_FIRST_STREAM_CHANNEL + g_snd.max_stream_channels);

#ifndef KISAK_SOUND
    AIL_pause_stream((HSTREAM)milesGlob.handle_sample[index], 1);
#else
    alSourcePause(alGlob.source[index]);
#endif // KISAK_SOUND
    g_snd.chaninfo[index].paused = 1;

#endif // KISAK_DEDICATED
}

void __cdecl SND_UnpauseStreamChannel(int index, int timeshift)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert(index >= SND_FIRST_STREAM_CHANNEL && index < SND_FIRST_STREAM_CHANNEL + g_snd.max_stream_channels);

    if (!g_snd.chaninfo[index].startDelay)
    {
#ifndef KISAK_SOUND
        AIL_pause_stream((HSTREAM)milesGlob.handle_sample[index], 0);
#else
        alSourcePlay(alGlob.source[index]);
#endif // KISAK_SOUND
    }

    g_snd.chaninfo[index].soundFileInfo.endtime += timeshift;
    g_snd.chaninfo[index].startTime += timeshift;
    g_snd.chaninfo[index].paused = 0;
#endif // KISAK_DEDICATED
}

bool __cdecl SND_IsStreamChannelFree(int index)
{
#ifdef KIASK_DEDICATED
    return false;
#else

    iassert(index >= SND_FIRST_STREAM_CHANNEL && index < SND_FIRST_STREAM_CHANNEL + g_snd.max_stream_channels);

#ifndef KISAK_SOUND
    if (!milesGlob.handle_sample[index])
        return 1;

    if (g_snd.chaninfo[index].paused || g_snd.chaninfo[index].startDelay)
        return 0;

    return g_snd.chaninfo[index].alias0 == 0;
#else
    if (!g_streamState[index].active)
        return 1;

    if (g_snd.chaninfo[index].paused || g_snd.chaninfo[index].startDelay)
        return 0;

    return g_snd.chaninfo[index].alias0 == 0;
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

#ifdef KISAK_DEDICATED
void __cdecl SND_ApplyChannelMap(_SAMPLE* handle, const snd_alias_t* alias, int srcChannelCount)
{
    return;
}
#else

#ifndef KISAK_SOUND
void __cdecl SND_ApplyChannelMap(_SAMPLE *handle, const snd_alias_t *alias, int srcChannelCount)
{
    float v3; // [esp+0h] [ebp-60h]
    float v4; // [esp+4h] [ebp-5Ch]
    float v5; // [esp+8h] [ebp-58h]
    float v6; // [esp+Ch] [ebp-54h]
    MSS_SPEAKER src_list[18] = {
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER,
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER,
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER,
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER,
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER,
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER
    };
    MSS_SPEAKER dst_list[18] = { 
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, 
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER,
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, 
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, 
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, 
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER 
    };

    float outVolumes[18]; // [esp+10h] [ebp-50h] BYREF
    MSSChannelMap *channelMap; // [esp+58h] [ebp-8h]
    int i; // [esp+5Ch] [ebp-4h]

    iassert(handle);
    iassert(alias);

    channelMap = Com_GetSpeakerMap(alias->speakerMap, srcChannelCount);
    if (channelMap)
    {
        memset(outVolumes, 0, sizeof(outVolumes));
        for (i = 0; i < channelMap->speakerCount; ++i)
        {
            v5 = channelMap->speakers[i].levels[0];
            v6 = channelMap->speakers[i].levels[1];
            v4 = v5 - v6;
            if (v4 < 0.0)
                v3 = v6;
            else
                v3 = v5;
            outVolumes[i] = v3;
        }
        //AIL_set_sample_channel_levels(handle, outVolumes, channelMap->speakerCount);
        AIL_set_sample_channel_levels(handle, src_list, dst_list, outVolumes, channelMap->speakerCount);
    }
}
#else
// KISAK_SOUND TODO (Phase 4): per-speaker channel-level mapping. Note the existing Miles
// implementation above already has the real per-speaker src/dst mapping short-circuited
// (hardcoded MSS_SPEAKER_FRONT_CENTER arrays - see WORK.md Phase 4) - don't try to preserve
// behavior that isn't really there; OpenAL's own downmix is a reasonable default to start.
void __cdecl SND_ApplyChannelMap(ALuint handle, const snd_alias_t *alias, int srcChannelCount)
{
}
#endif

#endif // KISAK_DEDICATED

#if KISAK_DEDICATED
int __cdecl SND_StartAlias2DSample(SndStartAliasInfo* startAliasInfo, int* pChannel)
{
    return -1;
}
#else

#ifndef KISAK_SOUND
int __cdecl SND_StartAlias2DSample(SndStartAliasInfo *startAliasInfo, int *pChannel)
{
    float baseSlavePercentage; // [esp+4h] [ebp-ACh]
    double timescale; // [esp+10h] [ebp-A0h]
    _SAMPLE *handle; // [esp+90h] [ebp-20h]
    int total_msec; // [esp+94h] [ebp-1Ch] BYREF
    int start_msec; // [esp+98h] [ebp-18h]
    int playbackId; // [esp+9Ch] [ebp-14h]
    float realVolume; // [esp+A0h] [ebp-10h]
    MssSoundCOD4 *sound; // [esp+A4h] [ebp-Ch]
    int entchannel; // [esp+A8h] [ebp-8h]
    int index; // [esp+ACh] [ebp-4h]

    iassert(startAliasInfo->alias0);
    iassert(SNDALIASFLAGS_GET_TYPE(startAliasInfo->alias0->flags) == SAT_LOADED);
    iassert(startAliasInfo->alias0->soundFile);
    iassert(startAliasInfo->alias0->soundFile->type == SAT_LOADED);
    iassert(startAliasInfo->alias0->soundFile->u.loadSnd);
    iassert(startAliasInfo->alias0->soundFile->exists);
    iassert(startAliasInfo->alias1);
    iassert(SNDALIASFLAGS_GET_TYPE(startAliasInfo->alias1->flags) == SAT_LOADED);
    iassert(startAliasInfo->alias1->soundFile);
    iassert(startAliasInfo->alias1->soundFile->type == SAT_LOADED);
    iassert(startAliasInfo->alias1->soundFile->u.loadSnd);
    iassert(startAliasInfo->alias1->soundFile->exists);

    entchannel = SNDALIASFLAGS_GET_CHANNEL(startAliasInfo->alias0->flags);
    if (!SND_HasFreeVoice(entchannel))
        return -1;

    index = SND_FindFree2DChannel(startAliasInfo, entchannel);
    if (pChannel)
        *pChannel = index;

    if (index < 0)
        return -1;

    iassert(index >= 0 && index < 0 + g_snd.max_2D_channels);

    handle = milesGlob.handle_sample[index];
    sound = &startAliasInfo->alias0->soundFile->u.loadSnd->sound;

    {
        PROF_SCOPED("SND_init_sample");
        _AILSOUNDINFO info; // LWSS HACK: struct version conversion
        info.format = sound->info.format;
        info.data_ptr = sound->info.data_ptr;
        info.data_len = sound->info.data_len;
        info.rate = sound->info.rate;
        info.bits = sound->info.bits;
        info.channels = sound->info.channels;
        info.samples = sound->info.samples;
        info.block_size = sound->info.block_size;
        info.initial_ptr = sound->info.initial_ptr;
        info.channel_mask = ~0U; // NEW!

        //AIL_set_sample_info(handle, &sound->info);
        AIL_set_sample_info(handle, &info);
    }

    MSS_ApplyEqFilter(handle, entchannel);
    if (startAliasInfo->timescale)
    {
        timescale = g_snd.timescale;
        AIL_set_sample_playback_rate(handle, SnapFloatToInt((float)AIL_sample_playback_rate(handle) * startAliasInfo->pitch * timescale));
    }
    else
    {
        AIL_set_sample_playback_rate(handle, SnapFloatToInt((float)AIL_sample_playback_rate(handle) * startAliasInfo->pitch));
    }
    realVolume = startAliasInfo->volume
        * g_snd.volume
        * g_snd.channelvol->channelvol[SNDALIASFLAGS_GET_CHANNEL(startAliasInfo->alias0->flags)].volume;

    if (g_snd.slaveLerp != 0.0 && !startAliasInfo->master && (startAliasInfo->alias0->flags & 4) != 0)
        realVolume = SND_GetLerpedSlavePercentage(startAliasInfo->alias0->slavePercentage) * realVolume;

    SND_ApplyChannelMap(handle, startAliasInfo->alias0, sound->info.channels);
    SND_Set2DChannelVolume(index, realVolume);
    AIL_set_sample_loop_count(handle, (startAliasInfo->alias0->flags & 1) == 0);
    baseSlavePercentage = MSS_GetWetLevel(startAliasInfo->alias0);
    AIL_set_sample_reverb_levels(handle, MSS_GetDryLevel(), baseSlavePercentage);
    AIL_sample_ms_position(handle, &total_msec, 0);

    if (startAliasInfo->timeshift >= total_msec)
        return SND_SetPlaybackIdNotPlayed(index);

    if (startAliasInfo->fraction == 0.0)
    {
        if (startAliasInfo->timeshift)
        {
            start_msec = startAliasInfo->timeshift;
        }
        else if ((startAliasInfo->alias0->flags & 0x20) != 0)
        {
            start_msec = SnapFloatToInt(random() * (float)total_msec) & 0xFFFFFF80;
        }
        else
        {
            start_msec = 0;
        }
    }
    else
    {
        start_msec = SnapFloatToInt((float)total_msec * startAliasInfo->fraction);
    }
    if (start_msec)
        startAliasInfo->startDelay = 0;

    AIL_set_sample_ms_position(handle, start_msec);
    if (!startAliasInfo->startDelay
        && (!g_snd.paused || !g_snd.pauseSettings[(startAliasInfo->alias0->flags & 0x3F00) >> 8]))
    {
        AIL_resume_sample(handle);
    }

    total_msec += startAliasInfo->startDelay;
    if ((startAliasInfo->alias0->flags & 1) != 0)
        total_msec = 0;
    SND_SetChannelStartInfo(index, startAliasInfo);
    SND_SetSoundFileChannelInfo(index, sound->info.channels, sound->info.rate, total_msec, start_msec, SFLS_LOADED);
    playbackId = SND_AcquirePlaybackId(index, total_msec);

    if (playbackId != -1)
        SND_AddVoice(entchannel);

    return playbackId;
}
#else
int __cdecl SND_StartAlias2DSample(SndStartAliasInfo *startAliasInfo, int *pChannel)
{
    iassert(startAliasInfo->alias0);
    iassert(SNDALIASFLAGS_GET_TYPE(startAliasInfo->alias0->flags) == SAT_LOADED);
    iassert(startAliasInfo->alias0->soundFile);
    iassert(startAliasInfo->alias0->soundFile->type == SAT_LOADED);
    iassert(startAliasInfo->alias0->soundFile->u.loadSnd);
    iassert(startAliasInfo->alias0->soundFile->exists);
    iassert(startAliasInfo->alias1);
    iassert(SNDALIASFLAGS_GET_TYPE(startAliasInfo->alias1->flags) == SAT_LOADED);
    iassert(startAliasInfo->alias1->soundFile);
    iassert(startAliasInfo->alias1->soundFile->type == SAT_LOADED);
    iassert(startAliasInfo->alias1->soundFile->u.loadSnd);
    iassert(startAliasInfo->alias1->soundFile->exists);

    int entchannel = SNDALIASFLAGS_GET_CHANNEL(startAliasInfo->alias0->flags);
    if (!SND_HasFreeVoice(entchannel))
        return -1;

    int index = SND_FindFree2DChannel(startAliasInfo, entchannel);
    if (pChannel)
        *pChannel = index;

    if (index < 0)
        return -1;

    iassert(index >= 0 && index < 0 + g_snd.max_2D_channels);

    ALuint source = alGlob.source[index];
    MssSoundCOD4 *sound = &startAliasInfo->alias0->soundFile->u.loadSnd->sound;

    // dr_wav (Phase 3) always decoded loaded sounds to 16-bit PCM, so the format is always
    // mono/stereo 16-bit - no need to replicate MSS_DigitalFormatType's branching here.
    SND_ReleaseChannelBuffer(index);
    ALuint buffer;
    alGenBuffers(1, &buffer);
    ALenum format = (sound->info.channels == 2) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    alBufferData(buffer, format, sound->data, sound->info.data_len, sound->info.rate);
    alGlob.channelBuffer[index] = buffer;
    alSourcei(source, AL_BUFFER, buffer);

    MSS_ApplyEqFilter(source, entchannel);

    // AL_PITCH is already a ratio relative to the buffer's native (embedded) rate, unlike
    // Miles' AIL_set_sample_playback_rate which needed the current absolute rate multiplied
    // out by hand - so this is simpler than the Miles branch above, not just a translation.
    float pitch = startAliasInfo->timescale ? startAliasInfo->pitch * (float)g_snd.timescale : startAliasInfo->pitch;
    alSourcef(source, AL_PITCH, pitch);

    float realVolume = startAliasInfo->volume
        * g_snd.volume
        * g_snd.channelvol->channelvol[SNDALIASFLAGS_GET_CHANNEL(startAliasInfo->alias0->flags)].volume;

    if (g_snd.slaveLerp != 0.0 && !startAliasInfo->master && (startAliasInfo->alias0->flags & 4) != 0)
        realVolume = SND_GetLerpedSlavePercentage(startAliasInfo->alias0->slavePercentage) * realVolume;

    SND_ApplyChannelMap(source, startAliasInfo->alias0, sound->info.channels);
    SND_Set2DChannelVolume(index, realVolume);
    alSourcei(source, AL_LOOPING, (startAliasInfo->alias0->flags & 1) != 0 ? AL_TRUE : AL_FALSE); // matches AIL_set_sample_loop_count's polarity above: flags bit 0 set == looping
    SND_ApplyReverbSend(index, startAliasInfo->alias0);

    // Duration at the pitch-adjusted rate, not the buffer's native rate - matches Miles'
    // behavior above, which queries AIL_sample_ms_position() *after* setting the pitched
    // playback rate, so a pitched-up sound reports a proportionally shorter duration.
    float effectiveRate = sound->info.rate * pitch;
    int total_msec = effectiveRate > 0.0f ? (int)((int64_t)sound->info.samples * 1000 / effectiveRate) : 0;

    if (startAliasInfo->timeshift >= total_msec)
        return SND_SetPlaybackIdNotPlayed(index);

    int start_msec;
    if (startAliasInfo->fraction == 0.0)
    {
        if (startAliasInfo->timeshift)
        {
            start_msec = startAliasInfo->timeshift;
        }
        else if ((startAliasInfo->alias0->flags & 0x20) != 0)
        {
            start_msec = SnapFloatToInt(random() * (float)total_msec) & 0xFFFFFF80;
        }
        else
        {
            start_msec = 0;
        }
    }
    else
    {
        start_msec = SnapFloatToInt((float)total_msec * startAliasInfo->fraction);
    }
    if (start_msec)
        startAliasInfo->startDelay = 0;

    alSourcef(source, AL_SEC_OFFSET, start_msec / 1000.0f);
    if (!startAliasInfo->startDelay
        && (!g_snd.paused || !g_snd.pauseSettings[(startAliasInfo->alias0->flags & 0x3F00) >> 8]))
    {
        alSourcePlay(source);
    }

    total_msec += startAliasInfo->startDelay;
    if ((startAliasInfo->alias0->flags & 1) != 0)
        total_msec = 0;
    SND_SetChannelStartInfo(index, startAliasInfo);
    SND_SetSoundFileChannelInfo(index, sound->info.channels, sound->info.rate, total_msec, start_msec, SFLS_LOADED);
    int playbackId = SND_AcquirePlaybackId(index, total_msec);

    if (playbackId != -1)
        SND_AddVoice(entchannel);

    return playbackId;
}
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED

#ifdef KISAK_DEDICATED
void __cdecl SND_Apply3DSpatializationTweaks(_SAMPLE* handle, const snd_alias_t* alias)
{
    return;
}
#else

#ifndef KISAK_SOUND
void __cdecl SND_Apply3DSpatializationTweaks(_SAMPLE *handle, const snd_alias_t *alias)
{
    MSS_SPEAKER src_list[18] = {
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER,
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER,
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER,
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER,
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER,
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER
    };
    MSS_SPEAKER dst_list[18] = {
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER,
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER,
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER,
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER,
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER,
        MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER, MSS_SPEAKER_FRONT_CENTER
    };
    float outVolumes[19]; // [esp+0h] [ebp-58h] BYREF
    int index; // [esp+4Ch] [ebp-Ch]
    float notCenterPercentage; // [esp+50h] [ebp-8h]
    DWORD numChannels; // [esp+54h] [ebp-4h] BYREF

    iassert(handle);
    iassert(alias);

    if (SND_IsMultiChannel())
    {
        // LWSS ADD - get channel count
        numChannels = AIL_sample_channel_count(handle, NULL);
        // LWSS END
        //AIL_sample_channel_levels(handle, &numChannels);
        AIL_sample_channel_levels(handle, src_list, dst_list, outVolumes, numChannels);

        for (index = 0; index < numChannels; ++index)
            outVolumes[index] = 1.0;

        if (alias->centerPercentage != 0.0 && SND_IsMultiChannel())
        {
            notCenterPercentage = 1.0 - alias->centerPercentage;
            for (index = 0; index < numChannels; ++index)
                outVolumes[index] = outVolumes[index] * notCenterPercentage;
        }

        outVolumes[2] = alias->centerPercentage;
        outVolumes[3] = alias->lfePercentage;

        //AIL_set_sample_channel_levels(handle, outVolumes, numChannels);
        AIL_set_sample_channel_levels(handle, src_list, dst_list, outVolumes, numChannels);
    }
}
#else
// KISAK_SOUND TODO (Phase 4): see the note on SND_ApplyChannelMap's stub above - the
// existing Miles per-speaker mapping is already short-circuited, so this is low priority.
void __cdecl SND_Apply3DSpatializationTweaks(ALuint handle, const snd_alias_t *alias)
{
}
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED

#ifdef KISAK_DEDICATED
int __cdecl SND_StartAlias3DSample(SndStartAliasInfo* startAliasInfo, int* pChannel)
{
    return -1;
}
#else

#ifndef KISAK_SOUND
int __cdecl SND_StartAlias3DSample(SndStartAliasInfo *startAliasInfo, int *pChannel)
{
    double LerpedSlavePercentage; // st7
    float mindist; // [esp+4h] [ebp-108h]
    float maxdist; // [esp+8h] [ebp-104h]
    double timescale; // [esp+24h] [ebp-E8h]
    float diff[15]; // [esp+98h] [ebp-74h] BYREF
    _SAMPLE *handle; // [esp+D4h] [ebp-38h]
    int rate; // [esp+D8h] [ebp-34h]
    int total_msec; // [esp+DCh] [ebp-30h]
    int start_msec; // [esp+E0h] [ebp-2Ch]
    const float *listener; // [esp+E4h] [ebp-28h]
    float attenuation; // [esp+E8h] [ebp-24h]
    int playbackId; // [esp+ECh] [ebp-20h]
    float realVolume; // [esp+F0h] [ebp-1Ch]
    MssSoundCOD4 *sound; // [esp+F4h] [ebp-18h]
    float distance; // [esp+F8h] [ebp-14h]
    float distMin; // [esp+FCh] [ebp-10h]
    int entchannel; // [esp+100h] [ebp-Ch]
    int index; // [esp+104h] [ebp-8h]
    float distMax; // [esp+108h] [ebp-4h]

    iassert(startAliasInfo->alias0);
    iassert(SNDALIASFLAGS_GET_TYPE(startAliasInfo->alias0->flags) == SAT_LOADED);

    if (!startAliasInfo->alias0->soundFile)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 944, 0, "%s", "startAliasInfo->alias0->soundFile");
    if (startAliasInfo->alias0->soundFile->type != 1)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 945, 0, "%s", "startAliasInfo->alias0->soundFile->type == SAT_LOADED");
    if (!startAliasInfo->alias0->soundFile->u.loadSnd)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 946, 0, "%s", "startAliasInfo->alias0->soundFile->u.loadSnd");
    if (!startAliasInfo->alias0->soundFile->exists)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 947, 0, "%s", "startAliasInfo->alias0->soundFile->exists");
    if (!startAliasInfo->alias1)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 948, 0, "%s", "startAliasInfo->alias1");
    if ((startAliasInfo->alias1->flags & 0xC0) >> 6 != 1)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            949,
            0,
            "%s",
            "SNDALIASFLAGS_GET_TYPE( startAliasInfo->alias1->flags ) == SAT_LOADED");
    if (!startAliasInfo->alias1->soundFile)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 950, 0, "%s", "startAliasInfo->alias1->soundFile");
    if (startAliasInfo->alias1->soundFile->type != 1)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 951, 0, "%s", "startAliasInfo->alias1->soundFile->type == SAT_LOADED");
    if (!startAliasInfo->alias1->soundFile->u.loadSnd)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 952, 0, "%s", "startAliasInfo->alias1->soundFile->u.loadSnd");
    if (!startAliasInfo->alias1->soundFile->exists)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 953, 0, "%s", "startAliasInfo->alias1->soundFile->exists");
    entchannel = (startAliasInfo->alias0->flags & 0x3F00) >> 8;
    if (!SND_HasFreeVoice(entchannel))
        return -1;
    index = SND_FindFree3DChannel(startAliasInfo, entchannel);
    if (pChannel)
        *pChannel = index;
    if (index < 0)
        return -1;
    if (index < 8 || index >= g_snd.max_3D_channels + 8)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            965,
            0,
            "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)",
            index);
    handle = milesGlob.handle_sample[index];
    sound = &startAliasInfo->alias0->soundFile->u.loadSnd->sound;
    distMin = (1.0 - startAliasInfo->lerp) * startAliasInfo->alias0->distMin
        + startAliasInfo->alias1->distMin * startAliasInfo->lerp;
    distMax = (1.0 - startAliasInfo->lerp) * startAliasInfo->alias0->distMax
        + startAliasInfo->alias1->distMax * startAliasInfo->lerp;
    {
        PROF_SCOPED("SND_set_3d_sample_info");
        _AILSOUNDINFO info; // LWSS HACK: struct version conversion
        info.format = sound->info.format;
        info.data_ptr = sound->info.data_ptr;
        info.data_len = sound->info.data_len;
        info.rate = sound->info.rate;
        info.bits = sound->info.bits;
        info.channels = sound->info.channels;
        info.samples = sound->info.samples;
        info.block_size = sound->info.block_size;
        info.initial_ptr = sound->info.initial_ptr;
        info.channel_mask = ~0U; // NEW!

        //AIL_set_sample_info(handle, &sound->info);
        AIL_set_sample_info(handle, &info);
    }

    MSS_ApplyEqFilter(handle, entchannel);
    listener = g_snd.listeners[SND_GetListenerIndexNearestToOrigin(startAliasInfo->org)].orient.origin;
    Vec3Sub(listener, startAliasInfo->org, diff);
    distance = Vec3Length(diff);
    attenuation = SND_Attenuate(startAliasInfo->alias0->volumeFalloffCurve, distance, distMin, distMax);
    realVolume = startAliasInfo->volume
        * attenuation
        * g_snd.channelvol->channelvol[(startAliasInfo->alias0->flags & 0x3F00) >> 8].volume;
    realVolume = realVolume * g_snd.volume;
    if (g_snd.slaveLerp != 0.0 && !startAliasInfo->master && (startAliasInfo->alias0->flags & 4) != 0)
    {
        LerpedSlavePercentage = SND_GetLerpedSlavePercentage(startAliasInfo->alias0->slavePercentage);
        realVolume = LerpedSlavePercentage * realVolume;
    }
    SND_Apply3DSpatializationTweaks(handle, startAliasInfo->alias0);
    SND_Set3DChannelVolume(index, realVolume);
    //((void(__stdcall *)(uint32_t, uint32_t, uint32_t, uint32_t))AIL_set_sample_3D_distances)(
    //    handle,
    //    startAliasInfo->alias0->distMax,
    //    startAliasInfo->alias0->distMin,
    //    1);
    AIL_set_sample_3D_distances(handle, startAliasInfo->alias0->distMax, startAliasInfo->alias0->distMin, 1);
    if (startAliasInfo->timescale)
    {
        timescale = g_snd.timescale;
        rate = SnapFloatToInt((float)AIL_sample_playback_rate(handle) * startAliasInfo->pitch * timescale);
    }
    else
    {
        rate = SnapFloatToInt((float)AIL_sample_playback_rate(handle) * startAliasInfo->pitch);
    }
    AIL_set_sample_playback_rate(handle, rate);
    SND_Set3DPosition(index, startAliasInfo->org);
    AIL_set_sample_loop_count(handle, (startAliasInfo->alias0->flags & 1) == 0);
    maxdist = MSS_GetWetLevel(startAliasInfo->alias0);
    mindist = MSS_GetDryLevel();
    AIL_set_sample_reverb_levels(handle, mindist, maxdist);
    if (!rate)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1004, 0, "%s", "rate");
    if (startAliasInfo->timescale)
    {
        total_msec = SnapFloatToInt(g_snd.timescale * (float)(1000 * sound->info.samples) / (float)rate);
    }
    else
    {
        total_msec = 1000 * sound->info.samples / rate;
    }
    if (startAliasInfo->timeshift >= total_msec)
        return SND_SetPlaybackIdNotPlayed(index);
    if (startAliasInfo->fraction == 0.0)
    {
        if (startAliasInfo->timeshift)
        {
            start_msec = startAliasInfo->timeshift;
        }
        else if ((startAliasInfo->alias0->flags & 0x20) != 0)
        {
            start_msec = SnapFloatToInt(random() * (float)total_msec) & 0xFFFFFF80;
        }
        else
        {
            start_msec = 0;
        }
    }
    else
    {
        start_msec = SnapFloatToInt((float)total_msec * startAliasInfo->fraction);
    }
    if (start_msec)
        startAliasInfo->startDelay = 0;
    AIL_set_sample_ms_position(handle, SnapFloatToInt((float)start_msec / (float)total_msec * (float)sound->info.data_len));
    if (!startAliasInfo->startDelay
        && (!g_snd.paused || !g_snd.pauseSettings[(startAliasInfo->alias0->flags & 0x3F00) >> 8]))
    {
        AIL_resume_sample(handle);
    }
    total_msec += startAliasInfo->startDelay;
    if ((startAliasInfo->alias0->flags & 1) != 0)
        total_msec = 0;
    SND_SetChannelStartInfo(index, startAliasInfo);
    SND_SetSoundFileChannelInfo(index, sound->info.channels, sound->info.rate, total_msec, start_msec, SFLS_LOADED);
    playbackId = SND_AcquirePlaybackId(index, total_msec);
    if (playbackId != -1)
        SND_AddVoice(entchannel);
    return playbackId;
}
#else
int __cdecl SND_StartAlias3DSample(SndStartAliasInfo *startAliasInfo, int *pChannel)
{
    iassert(startAliasInfo->alias0);
    iassert(SNDALIASFLAGS_GET_TYPE(startAliasInfo->alias0->flags) == SAT_LOADED);
    iassert(startAliasInfo->alias0->soundFile);
    iassert(startAliasInfo->alias0->soundFile->type == SAT_LOADED);
    iassert(startAliasInfo->alias0->soundFile->u.loadSnd);
    iassert(startAliasInfo->alias0->soundFile->exists);
    iassert(startAliasInfo->alias1);
    iassert(SNDALIASFLAGS_GET_TYPE(startAliasInfo->alias1->flags) == SAT_LOADED);
    iassert(startAliasInfo->alias1->soundFile);
    iassert(startAliasInfo->alias1->soundFile->type == SAT_LOADED);
    iassert(startAliasInfo->alias1->soundFile->u.loadSnd);
    iassert(startAliasInfo->alias1->soundFile->exists);

    int entchannel = (startAliasInfo->alias0->flags & 0x3F00) >> 8;
    if (!SND_HasFreeVoice(entchannel))
        return -1;
    int index = SND_FindFree3DChannel(startAliasInfo, entchannel);
    if (pChannel)
        *pChannel = index;
    if (index < 0)
        return -1;
    iassert(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels);

    ALuint source = alGlob.source[index];
    MssSoundCOD4 *sound = &startAliasInfo->alias0->soundFile->u.loadSnd->sound;
    float distMin = (1.0f - startAliasInfo->lerp) * startAliasInfo->alias0->distMin
        + startAliasInfo->alias1->distMin * startAliasInfo->lerp;
    float distMax = (1.0f - startAliasInfo->lerp) * startAliasInfo->alias0->distMax
        + startAliasInfo->alias1->distMax * startAliasInfo->lerp;

    SND_ReleaseChannelBuffer(index);
    ALuint buffer;
    alGenBuffers(1, &buffer);
    ALenum format = (sound->info.channels == 2) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    alBufferData(buffer, format, sound->data, sound->info.data_len, sound->info.rate);
    alGlob.channelBuffer[index] = buffer;
    alSourcei(source, AL_BUFFER, buffer);

    MSS_ApplyEqFilter(source, entchannel);

    const float *listener = g_snd.listeners[SND_GetListenerIndexNearestToOrigin(startAliasInfo->org)].orient.origin;
    float diff[3];
    Vec3Sub(listener, startAliasInfo->org, diff);
    float distance = Vec3Length(diff);
    float attenuation = SND_Attenuate(startAliasInfo->alias0->volumeFalloffCurve, distance, distMin, distMax);
    float realVolume = startAliasInfo->volume
        * attenuation
        * g_snd.channelvol->channelvol[(startAliasInfo->alias0->flags & 0x3F00) >> 8].volume;
    realVolume = realVolume * g_snd.volume;
    if (g_snd.slaveLerp != 0.0 && !startAliasInfo->master && (startAliasInfo->alias0->flags & 4) != 0)
        realVolume = SND_GetLerpedSlavePercentage(startAliasInfo->alias0->slavePercentage) * realVolume;

    SND_Apply3DSpatializationTweaks(source, startAliasInfo->alias0);
    SND_Set3DChannelVolume(index, realVolume);
    // distMin/distMax feed OpenAL's automatic distance-attenuation model, which is disabled
    // (alDistanceModel(AL_NONE), see MSS_Init) since SND_Attenuate's curve above already
    // computes the final gain by hand - so unlike Miles' AIL_set_sample_3D_distances there's
    // nothing to set here at all.

    float pitch = startAliasInfo->timescale ? startAliasInfo->pitch * (float)g_snd.timescale : startAliasInfo->pitch;
    alSourcef(source, AL_PITCH, pitch);
    SND_Set3DPosition(index, startAliasInfo->org);
    alSourcei(source, AL_LOOPING, (startAliasInfo->alias0->flags & 1) != 0 ? AL_TRUE : AL_FALSE); // matches AIL_set_sample_loop_count's polarity above: flags bit 0 set == looping
    SND_ApplyReverbSend(index, startAliasInfo->alias0);

    // Duration at the pitch-adjusted rate - see the matching comment in
    // SND_StartAlias2DSample above.
    float effectiveRate = sound->info.rate * pitch;
    int total_msec = effectiveRate > 0.0f ? (int)((int64_t)sound->info.samples * 1000 / effectiveRate) : 0;

    if (startAliasInfo->timeshift >= total_msec)
        return SND_SetPlaybackIdNotPlayed(index);

    int start_msec;
    if (startAliasInfo->fraction == 0.0)
    {
        if (startAliasInfo->timeshift)
        {
            start_msec = startAliasInfo->timeshift;
        }
        else if ((startAliasInfo->alias0->flags & 0x20) != 0)
        {
            start_msec = SnapFloatToInt(random() * (float)total_msec) & 0xFFFFFF80;
        }
        else
        {
            start_msec = 0;
        }
    }
    else
    {
        start_msec = SnapFloatToInt((float)total_msec * startAliasInfo->fraction);
    }
    if (start_msec)
        startAliasInfo->startDelay = 0;

    alSourcef(source, AL_SEC_OFFSET, start_msec / 1000.0f);
    if (!startAliasInfo->startDelay
        && (!g_snd.paused || !g_snd.pauseSettings[(startAliasInfo->alias0->flags & 0x3F00) >> 8]))
    {
        alSourcePlay(source);
    }
    int totalMsecForChan = total_msec + startAliasInfo->startDelay;
    if ((startAliasInfo->alias0->flags & 1) != 0)
        totalMsecForChan = 0;
    SND_SetChannelStartInfo(index, startAliasInfo);
    SND_SetSoundFileChannelInfo(index, sound->info.channels, sound->info.rate, totalMsecForChan, start_msec, SFLS_LOADED);
    int playbackId = SND_AcquirePlaybackId(index, totalMsecForChan);
    if (playbackId != -1)
        SND_AddVoice(entchannel);
    return playbackId;
}
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED

void __cdecl SND_Set3DStreamPosition(int index, int listenerIndex, const float *org)
{
#ifdef KISAK_DEDICATED
    return;
#else
    iassert(index >= SND_FIRST_STREAM_CHANNEL && index < SND_FIRST_STREAM_CHANNEL + g_snd.max_stream_channels);

#ifndef KISAK_SOUND
    float v3; // [esp+0h] [ebp-28h]
    float delta[3]; // [esp+Ch] [ebp-1Ch] BYREF
    _SAMPLE *handle_sample; // [esp+18h] [ebp-10h]
    float transformed[3]; // [esp+1Ch] [ebp-Ch] BYREF

    Vec3Sub(org, g_snd.listeners[listenerIndex].orient.origin, delta);
    MatrixTransposeTransformVector(delta, g_snd.listeners[listenerIndex].orient.axis, transformed);
    handle_sample = AIL_stream_sample_handle((HSTREAM)milesGlob.handle_sample[index]);
    v3 = -transformed[1];
    AIL_set_sample_3D_position(handle_sample, v3, transformed[2], transformed[0]);
#else
    // Same derivation as SND_Set3DPosition above - see its comment for the axis-swap
    // reasoning (also not yet verified in-game, same caveat applies here).
    float delta[3];
    Vec3Sub(org, g_snd.listeners[listenerIndex].orient.origin, delta);
    float transformed[3];
    MatrixTransposeTransformVector(delta, g_snd.listeners[listenerIndex].orient.axis, transformed);
    alSource3f(alGlob.source[index], AL_POSITION, -transformed[1], transformed[2], -transformed[0]);
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

double __cdecl SND_GetStream3DVolumeFallOff(int index, int listenerIndex)
{
    float diff[3]; // [esp+10h] [ebp-24h] BYREF
    float maxdist; // [esp+1Ch] [ebp-18h]
    float dist; // [esp+20h] [ebp-14h]
    float lerp; // [esp+24h] [ebp-10h]
    const snd_alias_t *alias1; // [esp+28h] [ebp-Ch]
    const snd_alias_t *alias0; // [esp+2Ch] [ebp-8h]
    float mindist; // [esp+30h] [ebp-4h]

    iassert(index >= ((0 + 8) + 32) && index < g_snd.max_stream_channels + ((0 + 8) + 32));

    alias0 = g_snd.chaninfo[index].alias0;
    alias1 = g_snd.chaninfo[index].alias1;
    if (!SND_IsAliasChannel3D(SNDALIASFLAGS_GET_CHANNEL(alias0->flags)))
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            585,
            0,
            "%s",
            "SND_IsAliasChannel3D( SNDALIASFLAGS_GET_CHANNEL( alias0->flags ) )");
    Vec3Sub(g_snd.listeners[listenerIndex].orient.origin, g_snd.chaninfo[index].org, diff);
    dist = Vec3Length(diff);
    lerp = g_snd.chaninfo[index].lerp;
    mindist = (1.0 - lerp) * alias0->distMin + alias1->distMin * lerp;
    maxdist = (1.0 - lerp) * alias0->distMax + alias1->distMax * lerp;
    return SND_Attenuate(alias0->volumeFalloffCurve, dist, mindist, maxdist);
}

#if KISAK_DEDICATED
int __cdecl SND_StartAliasStreamOnChannel(SndStartAliasInfo* startAliasInfo, int index)
{
    // Dedicated does not need sound
    return -1;
}
#else
#ifndef KISAK_SOUND
int __cdecl SND_StartAliasStreamOnChannel(SndStartAliasInfo *startAliasInfo, int index)
{
    const char *error; // eax
    double LerpedSlavePercentage; // st7
    double Stream3DVolumeFallOff; // st7
    float baseSlavePercentage; // [esp+8h] [ebp-240h]
    float *org; // [esp+Ch] [ebp-23Ch]
    _SAMPLE *handle; // [esp+90h] [ebp-1B8h]
    int total_msec[2]; // [esp+94h] [ebp-1B4h] BYREF
    int start_msec; // [esp+9Ch] [ebp-1ACh]
    char filename[132]; // [esp+A0h] [ebp-1A8h] BYREF
    _SAMPLE *handle_sample; // [esp+124h] [ebp-124h]
    char realname[256]; // [esp+128h] [ebp-120h] BYREF
    int filetype; // [esp+22Ch] [ebp-1Ch] BYREF
    int playbackId; // [esp+230h] [ebp-18h]
    float realVolume; // [esp+234h] [ebp-14h]
    int srcChannelCount; // [esp+238h] [ebp-10h]
    int listenerIndex; // [esp+23Ch] [ebp-Ch]
    int entchannel; // [esp+240h] [ebp-8h]
    int baserate; // [esp+244h] [ebp-4h]

    iassert(startAliasInfo->alias0);
    iassert(SNDALIASFLAGS_GET_TYPE(startAliasInfo->alias0->flags) == SAT_STREAMED);
    iassert(startAliasInfo->alias1);
    iassert(SNDALIASFLAGS_GET_TYPE(startAliasInfo->alias1->flags) == SAT_STREAMED);
    iassert((index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels));

    bool fsInitialized = FS_Initialized();
    iassert(fsInitialized);

    entchannel = SNDALIASFLAGS_GET_CHANNEL(startAliasInfo->alias0->flags);

    if (!SND_HasFreeVoice(entchannel))
        return -1;

    if (startAliasInfo->alias0->soundFile->exists)
    {
        if (milesGlob.handle_sample[index])
        {
            AIL_close_stream((HSTREAM)milesGlob.handle_sample[index]);
            milesGlob.handle_sample[index] = 0;
        }
        Com_GetSoundFileName(startAliasInfo->alias0, filename, 128);
        Com_sprintf(realname, 0x100u, "sound/%s", filename);
        total_msec[1] = (int)realname;
        {
            PROF_SCOPED("SND_open_stream");
            handle = (_SAMPLE *)AIL_open_stream(milesGlob.driver, realname, 0);
        }
        if (handle)
        {
            milesGlob.handle_sample[index] = handle;
            handle_sample = (_SAMPLE *)AIL_stream_sample_handle((HSTREAM)handle);
            AIL_stream_info((HSTREAM)handle, 0, &filetype, 0, 0);
            srcChannelCount = ((filetype & 2) != 0) + 1;
            MSS_ApplyEqFilter(handle_sample, entchannel);
            baserate = AIL_sample_playback_rate(handle_sample);
            if (startAliasInfo->timescale)
            {
                AIL_set_sample_playback_rate(handle_sample, SnapFloatToInt(g_snd.timescale * ((float)baserate * startAliasInfo->pitch)));
            }
            else
            {
                AIL_set_sample_playback_rate(handle_sample, SnapFloatToInt((float)baserate * startAliasInfo->pitch));
            }
            realVolume = startAliasInfo->volume
                * g_snd.volume
                * g_snd.channelvol->channelvol[(startAliasInfo->alias0->flags & 0x3F00) >> 8].volume;
            if (g_snd.slaveLerp != 0.0 && !startAliasInfo->master && (startAliasInfo->alias0->flags & 4) != 0)
            {
                LerpedSlavePercentage = SND_GetLerpedSlavePercentage(startAliasInfo->alias0->slavePercentage);
                realVolume = LerpedSlavePercentage * realVolume;
            }
            AIL_set_stream_loop_count((HSTREAM)handle, (startAliasInfo->alias0->flags & 1) == 0);
            baseSlavePercentage = MSS_GetWetLevel(startAliasInfo->alias0);
            AIL_set_sample_reverb_levels(handle_sample, MSS_GetDryLevel(), baseSlavePercentage);
            AIL_stream_ms_position((HSTREAM)handle, total_msec, 0);
            if (startAliasInfo->timeshift < total_msec[0])
            {
                if (total_msec[0])
                {
                    if (startAliasInfo->fraction == 0.0)
                    {
                        if (startAliasInfo->timeshift)
                        {
                            start_msec = startAliasInfo->timeshift;
                        }
                        else if ((startAliasInfo->alias0->flags & 0x20) != 0)
                        {
                            start_msec = SnapFloatToInt(random() * (float)total_msec[0]) & 0xFFFFFF80;
                        }
                        else
                        {
                            start_msec = 0;
                        }
                    }
                    else
                    {
                        start_msec = SnapFloatToInt((float)total_msec[0] * startAliasInfo->fraction);
                    }
                    if (start_msec)
                        startAliasInfo->startDelay = 0;
                    AIL_set_stream_ms_position((HSTREAM)handle, start_msec);
                    if (!startAliasInfo->startDelay
                        && (!g_snd.paused || !g_snd.pauseSettings[(startAliasInfo->alias0->flags & 0x3F00) >> 8]))
                    {
                        AIL_pause_stream((HSTREAM)handle, 0);
                    }
                    total_msec[0] += startAliasInfo->startDelay;
                    if ((startAliasInfo->alias0->flags & 1) != 0)
                        total_msec[0] = 0;
                    org = g_snd.chaninfo[index].org;
                    *org = startAliasInfo->org[0];
                    org[1] = startAliasInfo->org[1];
                    org[2] = startAliasInfo->org[2];
                    SND_SetChannelStartInfo(index, startAliasInfo);
                    SND_SetSoundFileChannelInfo(index, srcChannelCount, baserate, total_msec[0], start_msec, SFLS_LOADED);
                    if (SND_IsAliasChannel3D((g_snd.chaninfo[index].alias0->flags & 0x3F00) >> 8))
                    {
                        SND_GetCurrent3DPosition(
                            g_snd.chaninfo[index].sndEnt,
                            g_snd.chaninfo[index].offset,
                            g_snd.chaninfo[index].org);
                        listenerIndex = SND_GetListenerIndexNearestToOrigin(g_snd.chaninfo[index].org);
                        SND_Set3DStreamPosition(index, listenerIndex, g_snd.chaninfo[index].org);
                        Stream3DVolumeFallOff = SND_GetStream3DVolumeFallOff(index, listenerIndex);
                        realVolume = Stream3DVolumeFallOff * realVolume;
                        //((void(__stdcall *)(uint32_t, uint32_t, uint32_t, uint32_t))AIL_set_sample_3D_distances)(
                        //    handle_sample,
                        //    startAliasInfo->alias0->distMax,
                        //    startAliasInfo->alias0->distMin,
                        //    1);
                        AIL_set_sample_3D_distances(handle_sample, startAliasInfo->alias0->distMax, startAliasInfo->alias0->distMin, 1);
                        SND_Apply3DSpatializationTweaks(handle_sample, startAliasInfo->alias0);
                    }
                    else
                    {
                        SND_ApplyChannelMap(handle_sample, startAliasInfo->alias0, srcChannelCount);
                    }
                    SND_SetStreamChannelVolume(index, realVolume);
                    playbackId = SND_AcquirePlaybackId(index, total_msec[0]);
                    if (playbackId != -1)
                        SND_AddVoice(entchannel);
                    return playbackId;
                }
                else
                {
                    Com_PrintError(1, "ERROR: Sound file '%s' is zero length, invalid\n", realname);
                    return SND_SetPlaybackIdNotPlayed(index);
                }
            }
            else
            {
                return SND_SetPlaybackIdNotPlayed(index);
            }
        }
        else
        {
            error = (const char *)AIL_last_error();
            Com_PrintError(
                9,
                "Couldn't play stream '%s' from alias '%s' - %s\n",
                realname,
                startAliasInfo->alias0->aliasName,
                error);
            return SND_SetPlaybackIdNotPlayed(index);
        }
    }
    else
    {
        Com_GetSoundFileName(startAliasInfo->alias0, filename, 128);
        Com_DPrintf(
            9,
            "Tried to play streamed sound '%s' from alias '%s', but it was not found at load time.\n",
            filename,
            startAliasInfo->alias0->aliasName);
        return SND_SetPlaybackIdNotPlayed(index);
    }
}
#else
int __cdecl SND_StartAliasStreamOnChannel(SndStartAliasInfo *startAliasInfo, int index)
{
    iassert(startAliasInfo->alias0);
    iassert(SNDALIASFLAGS_GET_TYPE(startAliasInfo->alias0->flags) == SAT_STREAMED);
    iassert(startAliasInfo->alias1);
    iassert(SNDALIASFLAGS_GET_TYPE(startAliasInfo->alias1->flags) == SAT_STREAMED);
    iassert((index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels));

    bool fsInitialized = FS_Initialized();
    iassert(fsInitialized);

    int entchannel = SNDALIASFLAGS_GET_CHANNEL(startAliasInfo->alias0->flags);
    if (!SND_HasFreeVoice(entchannel))
        return -1;

    char filename[128];
    Com_GetSoundFileName(startAliasInfo->alias0, filename, 128);

    if (!startAliasInfo->alias0->soundFile->exists)
    {
        Com_DPrintf(
            9,
            "Tried to play streamed sound '%s' from alias '%s', but it was not found at load time.\n",
            filename,
            startAliasInfo->alias0->aliasName);
        return SND_SetPlaybackIdNotPlayed(index);
    }

    char realname[256];
    Com_sprintf(realname, 0x100u, "sound/%s", filename);

    SND_ReleaseStreamChannel(index); // close any previous stream still open on this channel

    AlStreamState *stream = &g_streamState[index];
    memset(stream, 0, sizeof(*stream));
    
    uint32_t openResult = (FS_FOpenFileReadStream(realname, &stream->fsHandle) & 0x80000000) == 0;
    if (!openResult)
    {
        Com_PrintError(9, "Couldn't play stream '%s' from alias '%s' - file not found\n", realname, startAliasInfo->alias0->aliasName);
        return SND_SetPlaybackIdNotPlayed(index);
    }

    size_t nameLen = strlen(filename);
    stream->isMp3 = nameLen >= 4 && I_stricmp(filename + nameLen - 4, ".mp3") == 0;

    bool decoderOk = stream->isMp3
        ? drmp3_init(&stream->mp3, AL_StreamReadCallback, AL_Mp3SeekCallback, AL_Mp3TellCallback, NULL, stream, NULL)
        : drwav_init(&stream->wav, AL_StreamReadCallback, AL_WavSeekCallback, AL_WavTellCallback, stream, NULL);

    if (!decoderOk)
    {
        FS_FCloseFile(stream->fsHandle);
        Com_PrintError(9, "Couldn't play stream '%s' from alias '%s' - invalid or corrupted format\n", realname, startAliasInfo->alias0->aliasName);
        return SND_SetPlaybackIdNotPlayed(index);
    }

    stream->channels = stream->isMp3 ? stream->mp3.channels : stream->wav.channels;
    stream->sampleRate = stream->isMp3 ? stream->mp3.sampleRate : stream->wav.sampleRate;
    stream->looping = (startAliasInfo->alias0->flags & 1) != 0; // matches AIL_set_stream_loop_count's polarity above: flags bit 0 set == looping
    stream->active = true;

    int srcChannelCount = stream->channels;
    int baserate = stream->sampleRate;

    // dr_mp3 doesn't know its total frame count up front the way dr_wav does from the RIFF
    // header - drmp3_get_pcm_frame_count does a one-time scan to find it, paid once here at
    // stream start (same spirit as the one-time cost of opening/parsing the file at all).
    uint64_t totalFrames = stream->isMp3
        ? drmp3_get_pcm_frame_count(&stream->mp3)
        : stream->wav.totalPCMFrameCount;

    ALuint source = alGlob.source[index];
    MSS_ApplyEqFilter(source, entchannel);

    float pitch = startAliasInfo->timescale ? startAliasInfo->pitch * (float)g_snd.timescale : startAliasInfo->pitch;
    alSourcef(source, AL_PITCH, pitch);

    // Duration at the pitch-adjusted rate, not the buffer's native rate - see the matching
    // comment in SND_StartAlias2DSample. Computed after `pitch` above is known, unlike the
    // loaded-sound versions, since streaming has no separate "set sample info" step to hang
    // this off of.
    float effectiveRate = baserate * pitch;
    int total_msec = effectiveRate > 0.0f ? (int)((int64_t)totalFrames * 1000 / effectiveRate) : 0;

    float realVolume = startAliasInfo->volume
        * g_snd.volume
        * g_snd.channelvol->channelvol[(startAliasInfo->alias0->flags & 0x3F00) >> 8].volume;
    if (g_snd.slaveLerp != 0.0 && !startAliasInfo->master && (startAliasInfo->alias0->flags & 4) != 0)
        realVolume = SND_GetLerpedSlavePercentage(startAliasInfo->alias0->slavePercentage) * realVolume;

    alSourcei(source, AL_LOOPING, AL_FALSE); // looping is handled by SND_FillStreamBuffers re-queuing, not AL_LOOPING
    SND_ApplyReverbSend(index, startAliasInfo->alias0);

    if (startAliasInfo->timeshift >= total_msec && total_msec != 0)
    {
        SND_ReleaseStreamChannel(index);
        return SND_SetPlaybackIdNotPlayed(index);
    }

    if (total_msec == 0 && totalFrames == 0)
    {
        SND_ReleaseStreamChannel(index);
        Com_PrintError(1, "ERROR: Sound file '%s' is zero length, invalid\n", realname);
        return SND_SetPlaybackIdNotPlayed(index);
    }

    int start_msec;
    if (startAliasInfo->fraction == 0.0)
    {
        if (startAliasInfo->timeshift)
        {
            start_msec = startAliasInfo->timeshift;
        }
        else if ((startAliasInfo->alias0->flags & 0x20) != 0)
        {
            start_msec = SnapFloatToInt(random() * (float)total_msec) & 0xFFFFFF80;
        }
        else
        {
            start_msec = 0;
        }
    }
    else
    {
        start_msec = SnapFloatToInt((float)total_msec * startAliasInfo->fraction);
    }
    if (start_msec)
        startAliasInfo->startDelay = 0;

    if (start_msec > 0 && baserate > 0)
    {
        uint64_t targetFrame = (uint64_t)start_msec * baserate / 1000;
        if (stream->isMp3)
            drmp3_seek_to_pcm_frame(&stream->mp3, targetFrame);
        else
            drwav_seek_to_pcm_frame(&stream->wav, targetFrame);
        stream->framesQueued = targetFrame; // SND_FillStreamBuffers adds onto this below
    }

    SND_FillStreamBuffers(index);

    if (!startAliasInfo->startDelay
        && (!g_snd.paused || !g_snd.pauseSettings[(startAliasInfo->alias0->flags & 0x3F00) >> 8]))
    {
        alSourcePlay(source);
    }

    int totalMsecForChan = total_msec + startAliasInfo->startDelay;
    if ((startAliasInfo->alias0->flags & 1) != 0)
        totalMsecForChan = 0;

    float *org = g_snd.chaninfo[index].org;
    *org = startAliasInfo->org[0];
    org[1] = startAliasInfo->org[1];
    org[2] = startAliasInfo->org[2];
    SND_SetChannelStartInfo(index, startAliasInfo);
    SND_SetSoundFileChannelInfo(index, srcChannelCount, baserate, totalMsecForChan, start_msec, SFLS_LOADED);

    if (SND_IsAliasChannel3D((g_snd.chaninfo[index].alias0->flags & 0x3F00) >> 8))
    {
        SND_GetCurrent3DPosition(g_snd.chaninfo[index].sndEnt, g_snd.chaninfo[index].offset, g_snd.chaninfo[index].org);
        int listenerIndex = SND_GetListenerIndexNearestToOrigin(g_snd.chaninfo[index].org);
        SND_Set3DStreamPosition(index, listenerIndex, g_snd.chaninfo[index].org);
        realVolume = (float)SND_GetStream3DVolumeFallOff(index, listenerIndex) * realVolume;
    }
    else
    {
        SND_ApplyChannelMap(source, startAliasInfo->alias0, srcChannelCount);
    }
    SND_SetStreamChannelVolume(index, realVolume);

    int playbackId = SND_AcquirePlaybackId(index, totalMsecForChan);
    if (playbackId != -1)
        SND_AddVoice(entchannel);
    return playbackId;
}
#endif // KISAK_SOUND
#endif // KISAK_DEDICATED

#ifdef KISAK_SOUND
// snd_roomStrings[27] (snd.cpp) is the exact same 26-name, same-order I3DL2/EAX preset list
// as these - verified name-for-name, not assumed - so this is a mechanical 1:1 copy, not a
// design decision. SND_RoomtypeFromString (snd.cpp) maps a string to an index into this
// same list, so the two arrays must stay in lockstep.
static const EFXEAXREVERBPROPERTIES AL_RoomPresets[26] =
{
    EFX_REVERB_PRESET_GENERIC,
    EFX_REVERB_PRESET_PADDEDCELL,
    EFX_REVERB_PRESET_ROOM,
    EFX_REVERB_PRESET_BATHROOM,
    EFX_REVERB_PRESET_LIVINGROOM,
    EFX_REVERB_PRESET_STONEROOM,
    EFX_REVERB_PRESET_AUDITORIUM,
    EFX_REVERB_PRESET_CONCERTHALL,
    EFX_REVERB_PRESET_CAVE,
    EFX_REVERB_PRESET_ARENA,
    EFX_REVERB_PRESET_HANGAR,
    EFX_REVERB_PRESET_CARPETEDHALLWAY,
    EFX_REVERB_PRESET_HALLWAY,
    EFX_REVERB_PRESET_STONECORRIDOR,
    EFX_REVERB_PRESET_ALLEY,
    EFX_REVERB_PRESET_FOREST,
    EFX_REVERB_PRESET_CITY,
    EFX_REVERB_PRESET_MOUNTAINS,
    EFX_REVERB_PRESET_QUARRY,
    EFX_REVERB_PRESET_PLAIN,
    EFX_REVERB_PRESET_PARKINGLOT,
    EFX_REVERB_PRESET_SEWERPIPE,
    EFX_REVERB_PRESET_UNDERWATER,
    EFX_REVERB_PRESET_DRUGGED,
    EFX_REVERB_PRESET_DIZZY,
    EFX_REVERB_PRESET_PSYCHOTIC,
};

static void AL_ApplyReverbPreset(ALuint effect, const EFXEAXREVERBPROPERTIES &props)
{
    alEffectf(effect, AL_EAXREVERB_DENSITY, props.flDensity);
    alEffectf(effect, AL_EAXREVERB_DIFFUSION, props.flDiffusion);
    alEffectf(effect, AL_EAXREVERB_GAIN, props.flGain);
    alEffectf(effect, AL_EAXREVERB_GAINHF, props.flGainHF);
    alEffectf(effect, AL_EAXREVERB_GAINLF, props.flGainLF);
    alEffectf(effect, AL_EAXREVERB_DECAY_TIME, props.flDecayTime);
    alEffectf(effect, AL_EAXREVERB_DECAY_HFRATIO, props.flDecayHFRatio);
    alEffectf(effect, AL_EAXREVERB_DECAY_LFRATIO, props.flDecayLFRatio);
    alEffectf(effect, AL_EAXREVERB_REFLECTIONS_GAIN, props.flReflectionsGain);
    alEffectf(effect, AL_EAXREVERB_REFLECTIONS_DELAY, props.flReflectionsDelay);
    alEffectfv(effect, AL_EAXREVERB_REFLECTIONS_PAN, props.flReflectionsPan);
    alEffectf(effect, AL_EAXREVERB_LATE_REVERB_GAIN, props.flLateReverbGain);
    alEffectf(effect, AL_EAXREVERB_LATE_REVERB_DELAY, props.flLateReverbDelay);
    alEffectfv(effect, AL_EAXREVERB_LATE_REVERB_PAN, props.flLateReverbPan);
    alEffectf(effect, AL_EAXREVERB_ECHO_TIME, props.flEchoTime);
    alEffectf(effect, AL_EAXREVERB_ECHO_DEPTH, props.flEchoDepth);
    alEffectf(effect, AL_EAXREVERB_MODULATION_TIME, props.flModulationTime);
    alEffectf(effect, AL_EAXREVERB_MODULATION_DEPTH, props.flModulationDepth);
    alEffectf(effect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, props.flAirAbsorptionGainHF);
    alEffectf(effect, AL_EAXREVERB_HFREFERENCE, props.flHFReference);
    alEffectf(effect, AL_EAXREVERB_LFREFERENCE, props.flLFReference);
    alEffectf(effect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, props.flRoomRolloffFactor);
    alEffecti(effect, AL_EAXREVERB_DECAY_HFLIMIT, props.iDecayHFLimit);
}

// Sets the per-channel wet-send *gain* (AL_AUXILIARY_SEND_FILTER has no gain parameter of
// its own - see AlLocal::sendFilter's comment in snd_local.h for why a lowpass filter is
// used purely as a gain carrier here) and wires the channel's source to the global reverb
// aux slot. MSS_GetDryLevel() is hardcoded to 1.0f in the existing Miles code (dead code,
// see WORK.md Phase 6) so there's no dry-side control to port - the dry/direct path is left
// at its default (unfiltered, full AL_GAIN) which already matches "dry always 1.0".
void SND_ApplyReverbSend(int index, const snd_alias_t *alias)
{
    float wet = MSS_GetWetLevel(alias);
    alFilterf(alGlob.sendFilter[index], AL_LOWPASS_GAIN, wet);
    alFilterf(alGlob.sendFilter[index], AL_LOWPASS_GAINHF, 1.0f);
    alSource3i(alGlob.source[index], AL_AUXILIARY_SEND_FILTER, alGlob.auxSlot, 0, alGlob.sendFilter[index]);
}
#endif

void __cdecl SND_SetRoomtype(int roomtype)
{
#ifdef KISAK_DEDICATED
    return;
#else

#ifndef KISAK_SOUND
    AIL_set_room_type(milesGlob.driver, roomtype);
    AIL_set_digital_master_reverb_levels(milesGlob.driver, MSS_GetDryLevel(), MSS_GetWetLevel(0));
#else
    iassert(roomtype >= 0 && roomtype < ARRAY_COUNT(AL_RoomPresets));
    AL_ApplyReverbPreset(alGlob.reverbEffect, AL_RoomPresets[roomtype]);
    alAuxiliaryEffectSloti(alGlob.auxSlot, AL_EFFECTSLOT_EFFECT, alGlob.reverbEffect);
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

void __cdecl SND_UpdateEqs()
{
#ifdef KISAK_DEDICATED
    return;
#else

#ifndef KISAK_SOUND
    _SAMPLE *handle; // [esp+0h] [ebp-8h]
    int channelIndex; // [esp+4h] [ebp-4h]

    for (channelIndex = 0; channelIndex < 53; ++channelIndex)
    {
        handle = 0;
        if (channelIndex < 0 || channelIndex >= g_snd.max_2D_channels)
        {
            if (channelIndex < 8 || channelIndex >= g_snd.max_3D_channels + 8)
            {
                if (channelIndex >= 40 && channelIndex < g_snd.max_stream_channels + 40)
                {
                    if (SND_IsStreamChannelFree(channelIndex))
                        continue;

                    handle = (_SAMPLE *)AIL_stream_sample_handle((HSTREAM)milesGlob.handle_sample[channelIndex]);
                }
            }
            else
            {
                if (SND_Is3DChannelFree(channelIndex))
                    continue;

                handle = milesGlob.handle_sample[channelIndex];
            }
        }
        else
        {
            if (SND_Is2DChannelFree(channelIndex))
                continue;

            handle = milesGlob.handle_sample[channelIndex];
        }
        if (handle)
            MSS_ApplyEqFilter(handle, g_snd.chaninfo[channelIndex].entchannel);
    }
#else
    // Mirrors the Miles loop above, but MSS_ApplyEqFilter (snd_al.cpp) is a deliberate
    // no-op on this side - see its comment for why (no 1:1 EFX equivalent to Miles' 3-band
    // parametric EQ). Left calling it anyway rather than skipping this loop entirely, so
    // nothing needs to change here if MSS_ApplyEqFilter ever becomes real.
    for (int channelIndex = 0; channelIndex < 53; ++channelIndex)
    {
        MSS_ApplyEqFilter(alGlob.source[channelIndex], g_snd.chaninfo[channelIndex].entchannel);
    }
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

void __cdecl SND_SetEqParams(
    uint32_t entchannel,
    int eqIndex,
    uint32_t band,
    SND_EQTYPE type,
    float gain,
    float freq,
    float q)
{
    iassert(entchannel >= 0 && entchannel < 64);
    iassert(band >= 0 && band < 3);
    iassert(freq >= 0 && freq <= 20000);
    iassert(q > 0);

#ifndef KISAK_SOUND
    iassert((unsigned)eqIndex < ARRAY_COUNT(milesGlob.eq)); // LWSS ADD

    milesGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    milesGlob.eq[eqIndex].params[band][entchannel].gain = gain;
    milesGlob.eq[eqIndex].params[band][entchannel].freq = freq;
    milesGlob.eq[eqIndex].params[band][entchannel].q = q;
    milesGlob.eq[eqIndex].params[band][entchannel].type = type;
#else
    iassert((unsigned)eqIndex < ARRAY_COUNT(alGlob.eq));

    alGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    alGlob.eq[eqIndex].params[band][entchannel].gain = gain;
    alGlob.eq[eqIndex].params[band][entchannel].freq = freq;
    alGlob.eq[eqIndex].params[band][entchannel].q = q;
    alGlob.eq[eqIndex].params[band][entchannel].type = type;
#endif

#ifndef KISAK_XBOX
	SND_UpdateEqs();
#endif
}

void __cdecl SND_SetEqType(uint32_t entchannel, int eqIndex, uint32_t band, SND_EQTYPE type)
{
    iassert(entchannel >= 0 && entchannel < 64);
    iassert(band >= 0 && band < 3);

#ifndef KISAK_SOUND
    iassert((unsigned)eqIndex < ARRAY_COUNT(milesGlob.eq)); // LWSS ADD

    milesGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    milesGlob.eq[eqIndex].params[band][entchannel].type = type;
#else
    iassert((unsigned)eqIndex < ARRAY_COUNT(alGlob.eq));

    alGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    alGlob.eq[eqIndex].params[band][entchannel].type = type;
#endif
}

void __cdecl SND_SetEqFreq(uint32_t entchannel, int eqIndex, uint32_t band, float freq)
{
    iassert(entchannel >= 0 && entchannel < 64);
    iassert(band >= 0 && band < 3);
    iassert(freq >= 0 && freq <= 20000);

#ifndef KISAK_SOUND
    iassert((unsigned)eqIndex < ARRAY_COUNT(milesGlob.eq)); // LWSS ADD

    milesGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    milesGlob.eq[eqIndex].params[band][entchannel].freq = freq;
#else
    iassert((unsigned)eqIndex < ARRAY_COUNT(alGlob.eq));

    alGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    alGlob.eq[eqIndex].params[band][entchannel].freq = freq;
#endif
}

void __cdecl SND_SetEqGain(uint32_t entchannel, int eqIndex, uint32_t band, float gain)
{
    iassert(entchannel >= 0 && entchannel < 64);
    iassert(band >= 0 && band < 3);

#ifndef KISAK_SOUND
    iassert((unsigned)eqIndex < ARRAY_COUNT(milesGlob.eq)); // LWSS ADD
    milesGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    milesGlob.eq[eqIndex].params[band][entchannel].gain = gain;
#else
    iassert((unsigned)eqIndex < ARRAY_COUNT(alGlob.eq));
    alGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    alGlob.eq[eqIndex].params[band][entchannel].gain = gain;
#endif
}

void __cdecl SND_SetEqQ(uint32_t entchannel, int eqIndex, uint32_t band, float q)
{
    iassert(entchannel >= 0 && entchannel < 64);
    iassert(band >= 0 && band < 3);
    iassert(q > 0);

#ifndef KISAK_SOUND
    iassert((unsigned)eqIndex < ARRAY_COUNT(milesGlob.eq)); // LWSS ADD

    milesGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    milesGlob.eq[eqIndex].params[band][entchannel].q = q;
#else
    iassert((unsigned)eqIndex < ARRAY_COUNT(alGlob.eq));

    alGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    alGlob.eq[eqIndex].params[band][entchannel].q = q;
#endif

#ifndef KISAK_XBOX
	SND_UpdateEqs();
#endif
}

void __cdecl SND_DisableEq(uint32_t entchannel, int eqIndex, uint32_t band)
{
    iassert(entchannel >= 0 && entchannel < 64);
    iassert(band >= 0 && band < 3);

#ifndef KISAK_SOUND
    iassert((unsigned)eqIndex < ARRAY_COUNT(milesGlob.eq)); // LWSS ADD

    milesGlob.eq[eqIndex].params[band][entchannel].enabled = 0;
#else
    iassert((unsigned)eqIndex < ARRAY_COUNT(alGlob.eq));

    alGlob.eq[eqIndex].params[band][entchannel].enabled = 0;
#endif
}

void __cdecl SND_SaveEq(MemoryFile *memFile)
{
    int band; // [esp+0h] [ebp-Ch]
    int entchannel; // [esp+4h] [ebp-8h]
    int eqIndex; // [esp+8h] [ebp-4h]

    for (eqIndex = 0; eqIndex < 2; ++eqIndex)
    {
        for (band = 0; band < 3; ++band)
        {
            for (entchannel = 0; entchannel < 64; ++entchannel)
#ifndef KISAK_SOUND
                MemFile_WriteData(memFile, 20, &milesGlob.eq[eqIndex].params[band][entchannel]);
#else
                MemFile_WriteData(memFile, 20, &alGlob.eq[eqIndex].params[band][entchannel]);
#endif
        }
    }
}

void __cdecl SND_RestoreEq(MemoryFile *memFile)
{
    int band; // [esp+0h] [ebp-Ch]
    int entchannel; // [esp+4h] [ebp-8h]
    int eqIndex; // [esp+8h] [ebp-4h]

    for (eqIndex = 0; eqIndex < 2; ++eqIndex)
    {
        for (band = 0; band < 3; ++band)
        {
            for (entchannel = 0; entchannel < 64; ++entchannel)
#ifndef KISAK_SOUND
                MemFile_ReadData(memFile, 20, (uint8_t *)&milesGlob.eq[eqIndex].params[band][entchannel]);
#else
                MemFile_ReadData(memFile, 20, (uint8_t *)&alGlob.eq[eqIndex].params[band][entchannel]);
#endif
        }
    }
}

void __cdecl SND_PrintEqParams()
{
    float *v0; // edx
    snd_entchannel_info_t *channelName; // [esp+18h] [ebp-24h]
    int band; // [esp+1Ch] [ebp-20h]
    int entchannel; // [esp+20h] [ebp-1Ch]
    int eqIndex; // [esp+24h] [ebp-18h]

    Com_Printf(9, "Current EQ Settings\n---------------\n");
    for (entchannel = 0; entchannel < g_snd.entchannel_count; ++entchannel)
    {
        channelName = SND_GetEntChannelName(entchannel);
        Com_Printf(9, "+ %s\n", channelName->name);
        for (eqIndex = 0; eqIndex < 2; ++eqIndex)
        {
            for (band = 0; band < 3; ++band)
            {
#ifndef KISAK_SOUND
                v0 = (float *)&milesGlob.eq[eqIndex].params[band][entchannel];
#else
                v0 = (float *)&alGlob.eq[eqIndex].params[band][entchannel];
#endif
                if ((uint8_t)*((uint32_t *)v0 + 4))
                    Com_Printf(9, "\t%i %s %f Hz %f dB %f q\n", band, snd_eqTypeStrings[*(uint32_t *)v0], v0[2], v0[1], v0[3]);
            }
        }
    }
}

double __cdecl SND_Get2DChannelVolume(int index)
{
#ifdef KISAK_DEDICATED
    return 0;
#else

    iassert(index >= 0 && index < 0 + g_snd.max_2D_channels);

#ifndef KISAK_SOUND
    float right; // [esp+4h] [ebp-8h] BYREF
    float left; // [esp+8h] [ebp-4h] BYREF

    AIL_sample_volume_levels(milesGlob.handle_sample[index], &left, &right);

    if (g_snd.chaninfo[index].soundFileInfo.srcChannelCount == 2)
        return left;

    return (float)(left + right);
#else
    // Unlike Miles' separate left/right levels, AL_GAIN is a single scalar regardless of
    // channel count - no srcChannelCount branching needed on this side.
    ALfloat gain;
    alGetSourcef(alGlob.source[index], AL_GAIN, &gain);
    return gain;
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

void __cdecl SND_Set2DChannelVolume(int index, float volume)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert(index >= 0 && index < 0 + g_snd.max_2D_channels);

#ifndef KISAK_SOUND
    AIL_set_sample_volume_levels(milesGlob.handle_sample[index], volume, volume);
#else
    alSourcef(alGlob.source[index], AL_GAIN, volume);
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

double __cdecl SND_Get3DChannelVolume(int index)
{
#ifdef KISAK_DEDICATED
    return 0;
#else

    iassert(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels);

#ifndef KISAK_SOUND
    float right; // [esp+4h] [ebp-8h] BYREF
    float left; // [esp+8h] [ebp-4h] BYREF

    AIL_sample_volume_levels(milesGlob.handle_sample[index], &left, &right);
    if (g_snd.chaninfo[index].soundFileInfo.srcChannelCount == 2)
        return left;

    return (float)(left + right);
#else
    ALfloat gain;
    alGetSourcef(alGlob.source[index], AL_GAIN, &gain);
    return gain;
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

void __cdecl SND_Set3DChannelVolume(int index, float volume)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels);

#ifndef KISAK_SOUND
    float v2; // [esp+Ch] [ebp-4h]

    if (g_snd.chaninfo[index].soundFileInfo.srcChannelCount == 2)
    {
        AIL_set_sample_volume_levels(milesGlob.handle_sample[index], volume, volume);
    }
    else
    {
        v2 = volume * 0.5;
        AIL_set_sample_volume_levels(milesGlob.handle_sample[index], v2, v2);
    }
#else
    // Miles halves mono-source volume to compensate for its stereo-pair mixing; AL_GAIN
    // applies uniformly regardless of channel count, so no such compensation is needed.
    alSourcef(alGlob.source[index], AL_GAIN, volume);
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

double __cdecl SND_GetStreamChannelVolume(int index)
{
#ifdef KISAK_DEDICATED
    return 0;
#else

    iassert(index >= SND_FIRST_STREAM_CHANNEL && index < SND_FIRST_STREAM_CHANNEL + g_snd.max_stream_channels);

#ifndef KISAK_SOUND
    _SAMPLE *handle_sample; // [esp+4h] [ebp-Ch]
    float right; // [esp+8h] [ebp-8h] BYREF
    float left; // [esp+Ch] [ebp-4h] BYREF

    handle_sample = (_SAMPLE *)AIL_stream_sample_handle((HSTREAM)milesGlob.handle_sample[index]);
    AIL_sample_volume_levels(handle_sample, &left, &right);

    if (g_snd.chaninfo[index].soundFileInfo.srcChannelCount == 2
        || !SND_IsAliasChannel3D((g_snd.chaninfo[index].alias0->flags & 0x3F00) >> 8))
    {
        return left;
    }

    return (float)(left + right);
#else
    ALfloat gain;
    alGetSourcef(alGlob.source[index], AL_GAIN, &gain);
    return gain;
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

void __cdecl SND_SetStreamChannelVolume(int index, float volume)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert(index >= SND_FIRST_STREAM_CHANNEL && index < SND_FIRST_STREAM_CHANNEL + g_snd.max_stream_channels);

#ifndef KISAK_SOUND
    float v2; // [esp+Ch] [ebp-8h]
    _SAMPLE *handle_sample; // [esp+10h] [ebp-4h]

    handle_sample = (_SAMPLE *)AIL_stream_sample_handle((HSTREAM)milesGlob.handle_sample[index]);
    if (g_snd.chaninfo[index].soundFileInfo.srcChannelCount == 2
        || !SND_IsAliasChannel3D((g_snd.chaninfo[index].alias0->flags & 0x3F00) >> 8))
    {
        AIL_set_sample_volume_levels(handle_sample, volume, volume);
    }
    else
    {
        v2 = volume * 0.5;
        AIL_set_sample_volume_levels(handle_sample, v2, v2);
    }
#else
    alSourcef(alGlob.source[index], AL_GAIN, volume);
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

int __cdecl SND_Get2DChannelPlaybackRate(int index)
{
#ifdef KISAK_DEDICATED
    return 0;
#else

    iassert(index >= 0 && index < 0 + g_snd.max_2D_channels);

#ifndef KISAK_SOUND
    return AIL_sample_playback_rate(milesGlob.handle_sample[index]);
#else
    // AL_PITCH is a ratio, not an absolute rate like Miles' - convert against the buffer's
    // native rate, which SND_StartAlias2DSample already recorded via
    // SND_SetSoundFileChannelInfo at channel-start time (see snd_channel_info_t.
    // soundFileInfo.baserate), so no extra per-channel bookkeeping is needed for this.
    ALfloat pitch;
    alGetSourcef(alGlob.source[index], AL_PITCH, &pitch);
    return SnapFloatToInt(pitch * g_snd.chaninfo[index].soundFileInfo.baserate);
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

void __cdecl SND_Set2DChannelPlaybackRate(int index, int rate)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert(index >= 0 && index < 0 + g_snd.max_2D_channels);

#ifndef KISAK_SOUND
    AIL_set_sample_playback_rate(milesGlob.handle_sample[index], rate);
#else
    int baserate = g_snd.chaninfo[index].soundFileInfo.baserate;
    alSourcef(alGlob.source[index], AL_PITCH, baserate ? (float)rate / baserate : 1.0f);
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

int __cdecl SND_Get3DChannelPlaybackRate(int index)
{
#ifdef KISAK_DEDICATED
    return 0;
#else

    iassert(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels);

#ifndef KISAK_SOUND
    return AIL_sample_playback_rate(milesGlob.handle_sample[index]);
#else
    ALfloat pitch;
    alGetSourcef(alGlob.source[index], AL_PITCH, &pitch);
    return SnapFloatToInt(pitch * g_snd.chaninfo[index].soundFileInfo.baserate);
#endif // KISAK_SOUND

#endif // KSIAK_DEDICATED
}

void __cdecl SND_Set3DChannelPlaybackRate(int index, int rate)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels);

#ifndef KISAK_SOUND
    AIL_set_sample_playback_rate(milesGlob.handle_sample[index], rate);
#else
    int baserate = g_snd.chaninfo[index].soundFileInfo.baserate;
    alSourcef(alGlob.source[index], AL_PITCH, baserate ? (float)rate / baserate : 1.0f);
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

int __cdecl SND_GetStreamChannelPlaybackRate(int index)
{
#ifdef KISAK_DEDICATED
    return 0;
#else

    iassert(index >= SND_FIRST_STREAM_CHANNEL && index < SND_FIRST_STREAM_CHANNEL + g_snd.max_stream_channels);

#ifndef KISAK_SOUND
    _SAMPLE *handle_sample; // [esp+0h] [ebp-4h]

    handle_sample = (_SAMPLE *)AIL_stream_sample_handle((HSTREAM)milesGlob.handle_sample[index]);
    return AIL_sample_playback_rate(handle_sample);
#else
    // See SND_Get2DChannelPlaybackRate - same AL_PITCH-is-a-ratio conversion.
    ALfloat pitch;
    alGetSourcef(alGlob.source[index], AL_PITCH, &pitch);
    return SnapFloatToInt(pitch * g_snd.chaninfo[index].soundFileInfo.baserate);
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

void __cdecl SND_SetStreamChannelPlaybackRate(int index, int rate)
{
#ifdef KISAK_DEDICATED
    return;

#else

    iassert(index >= SND_FIRST_STREAM_CHANNEL && index < SND_FIRST_STREAM_CHANNEL + g_snd.max_stream_channels);

#ifndef KISAK_SOUND
    _SAMPLE *handle_sample; // [esp+0h] [ebp-4h]

    handle_sample = (_SAMPLE *)AIL_stream_sample_handle((HSTREAM)milesGlob.handle_sample[index]);
    AIL_set_sample_playback_rate(handle_sample, rate);
#else
    int baserate = g_snd.chaninfo[index].soundFileInfo.baserate;
    alSourcef(alGlob.source[index], AL_PITCH, baserate ? (float)rate / baserate : 1.0f);
#endif // KISAK_SOUND

#endif // KISAK_DEDICATEd
}

void __cdecl SND_Update2DChannelReverb(int index)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert(index >= 0 && index < 0 + g_snd.max_2D_channels);

#ifndef KISAK_SOUND
    AIL_set_sample_reverb_levels(milesGlob.handle_sample[index], MSS_GetDryLevel(), MSS_GetWetLevel(g_snd.chaninfo[index].alias0));
#else
    SND_ApplyReverbSend(index, g_snd.chaninfo[index].alias0);
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

void __cdecl SND_Update3DChannelReverb(int index)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels);

#ifndef KISAK_SOUND
    AIL_set_sample_reverb_levels(milesGlob.handle_sample[index], MSS_GetDryLevel(), MSS_GetWetLevel(g_snd.chaninfo[index].alias0));
#else
    SND_ApplyReverbSend(index, g_snd.chaninfo[index].alias0);
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

void __cdecl SND_UpdateStreamChannelReverb(int index)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert(index >= SND_FIRST_STREAM_CHANNEL && index < SND_FIRST_STREAM_CHANNEL + g_snd.max_stream_channels);

#ifndef KISAK_SOUND
    AIL_set_sample_reverb_levels(
        (_SAMPLE *)AIL_stream_sample_handle((HSTREAM)milesGlob.handle_sample[index]),
        MSS_GetDryLevel(),
        MSS_GetWetLevel(g_snd.chaninfo[index].alias0)
    );
#else
    SND_ApplyReverbSend(index, g_snd.chaninfo[index].alias0);
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

int __cdecl SND_Get2DChannelLength(int index)
{
#ifdef KISAK_DEDICATED
    return 0;
#else

    iassert(index >= 0 && index < 0 + g_snd.max_2D_channels);

#ifndef KISAK_SOUND
    int length; // [esp+0h] [ebp-4h] BYREF

    AIL_sample_ms_position(milesGlob.handle_sample[index], &length, 0);
    return length;
#else
    // SND_StartAlias2DSample already computed and stored the pitch-adjusted duration via
    // SND_AcquirePlaybackId at channel-start time - reuse it rather than re-deriving from
    // the buffer (which would also need AL_PITCH factored back in to match Miles' behavior
    // of reporting a pitch-adjusted length, same as the total_msec comment there explains).
    return g_snd.chaninfo[index].totalMsec;
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

int __cdecl SND_Get3DChannelLength(int index)
{
#ifdef KISAK_DEDICATED
    return 0;
#else

    iassert(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels);

#ifndef KISAK_SOUND
    int length; // [esp+0h] [ebp-4h] BYREF

    AIL_sample_ms_position(milesGlob.handle_sample[index], &length, 0);
    return length;
#else
    // See SND_Get2DChannelLength.
    return g_snd.chaninfo[index].totalMsec;
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

int __cdecl SND_GetStreamChannelLength(int index)
{
#ifdef KISAK_DEDICATED
    return 0;
#else

    iassert(index >= SND_FIRST_STREAM_CHANNEL && index < SND_FIRST_STREAM_CHANNEL + g_snd.max_stream_channels);

#ifndef KISAK_SOUND
    int length; // [esp+0h] [ebp-4h] BYREF

    AIL_stream_ms_position((HSTREAM)milesGlob.handle_sample[index], &length, 0);
    return length;
#else
    // See SND_Get2DChannelLength - reuses the duration SND_StartAliasStreamOnChannel
    // already computed via drwav's totalPCMFrameCount / dr_mp3's one-time frame-count scan.
    return g_snd.chaninfo[index].totalMsec;
#endif // KISAK_SOUND
#endif // KISAK_DEDICATED
}

void __cdecl SND_Get2DChannelSaveInfo(int index, snd_save_2D_sample_t *info)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert(index >= 0 && index < 0 + g_snd.max_2D_channels);

#ifndef KISAK_SOUND
    _SAMPLE *handle; // [esp+0h] [ebp-Ch]
    int offset; // [esp+4h] [ebp-8h] BYREF
    int length; // [esp+8h] [ebp-4h] BYREF

    handle = milesGlob.handle_sample[index];
    iassert(handle);

    AIL_sample_ms_position(handle, &length, &offset);
    info->fraction = (double)offset / (double)length;
    info->pitch = g_snd.chaninfo[index].pitch;
    AIL_sample_volume_pan(handle, &info->volume, 0);

    if (g_snd.volume == 0.0)
        info->volume = g_snd.chaninfo[index].basevolume;
    else
        info->volume = info->volume / g_snd.volume;
#else
    ALuint source = alGlob.source[index];
    ALfloat offsetSec = 0.0f;
    alGetSourcef(source, AL_SEC_OFFSET, &offsetSec);
    int totalMsec = g_snd.chaninfo[index].totalMsec;
    info->fraction = totalMsec > 0 ? (offsetSec * 1000.0f) / totalMsec : 0.0f;
    info->pitch = g_snd.chaninfo[index].pitch;

    ALfloat gain = 0.0f;
    alGetSourcef(source, AL_GAIN, &gain);
    if (g_snd.volume == 0.0)
        info->volume = g_snd.chaninfo[index].basevolume;
    else
        info->volume = gain / g_snd.volume;
    // info->pan intentionally left untouched, matching the Miles branch above (which also
    // never sets it - AIL_sample_volume_pan's third argument there is NULL/0).
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

void __cdecl SND_Set2DChannelFromSaveInfo(int index, snd_save_2D_sample_t *info)
{
    float volume; // [esp+4h] [ebp-4h]

    iassert(index >= 0 && index < 0 + g_snd.max_2D_channels);

    volume = info->volume * g_snd.volume;
    SND_Set2DChannelVolume(index, volume);
}

void __cdecl SND_Get3DChannelSaveInfo(int index, snd_save_3D_sample_t *info)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels);

#ifndef KISAK_SOUND
    _SAMPLE *handle; // [esp+0h] [ebp-Ch]
    int offset; // [esp+4h] [ebp-8h] BYREF
    int length; // [esp+8h] [ebp-4h] BYREF

    handle = milesGlob.handle_sample[index];
    iassert(handle);
    AIL_sample_ms_position(handle, &length, &offset);
    info->fraction = (double)offset / (double)length;
    info->pitch = g_snd.chaninfo[index].pitch;
    AIL_sample_volume_pan(handle, &info->volume, 0);

    if (g_snd.volume == 0.0)
        info->volume = g_snd.chaninfo[index].basevolume;
    else
        info->volume = info->volume / g_snd.volume;

    AIL_sample_3D_position(handle, info->org, &info->org[2], &info->org[1]);
#else
    ALuint source = alGlob.source[index];
    ALfloat offsetSec = 0.0f;
    alGetSourcef(source, AL_SEC_OFFSET, &offsetSec);
    int totalMsec = g_snd.chaninfo[index].totalMsec;
    info->fraction = totalMsec > 0 ? (offsetSec * 1000.0f) / totalMsec : 0.0f;
    info->pitch = g_snd.chaninfo[index].pitch;

    ALfloat gain = 0.0f;
    alGetSourcef(source, AL_GAIN, &gain);
    if (g_snd.volume == 0.0)
        info->volume = g_snd.chaninfo[index].basevolume;
    else
        info->volume = gain / g_snd.volume;

    // Miles' getter above reshuffles/negates its raw AIL_sample_3D_position output into a
    // specific save encoding that pairs with how its setter (SND_Set3DPosition) originally
    // transformed org[] on the way in. There's no 3D "SetFromSaveInfo" counterpart anywhere
    // in this codebase to require matching that exact byte layout on this side, so just
    // store the queried AL_POSITION directly in natural (x,y,z) order - self-consistent for
    // this backend, not byte-compatible with a Miles-format save.
    alGetSource3f(source, AL_POSITION, &info->org[0], &info->org[1], &info->org[2]);
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

void __cdecl SND_GetStreamChannelSaveInfo(int index, snd_save_stream_t *info)
{
#ifdef KISAK_DEDICATED
    return;
#else

    iassert(index >= SND_FIRST_STREAM_CHANNEL && index < SND_FIRST_STREAM_CHANNEL + g_snd.max_stream_channels);

#ifndef KISAK_SOUND
    int v2; // [esp+0h] [ebp-3Ch]
    double timescale; // [esp+8h] [ebp-34h]
    float *org; // [esp+14h] [ebp-28h]
    _STREAM *handle; // [esp+2Ch] [ebp-10h]
    _SAMPLE *handle_sample; // [esp+30h] [ebp-Ch]
    int offset; // [esp+34h] [ebp-8h] BYREF
    int length; // [esp+38h] [ebp-4h] BYREF

    handle = (_STREAM *)milesGlob.handle_sample[index];
    iassert(handle);
    handle_sample = (_SAMPLE *)AIL_stream_sample_handle((HSTREAM)milesGlob.handle_sample[index]);
    AIL_stream_ms_position(handle, &length, &offset);
    info->fraction = (double)offset / (double)length;
    if (g_snd.chaninfo[index].timescale)
    {
        timescale = g_snd.timescale;
        v2 = SnapFloatToInt((float)AIL_sample_playback_rate(handle_sample) / timescale);
    }
    else
    {
        v2 = AIL_sample_playback_rate(handle_sample);
    }
    info->rate = v2;
    info->basevolume = g_snd.chaninfo[index].basevolume;
    AIL_sample_volume_pan(handle_sample, &info->volume, 0);

    if (g_snd.volume == 0.0)
        info->volume = g_snd.chaninfo[index].basevolume;
    else
        info->volume = info->volume / g_snd.volume;

    org = g_snd.chaninfo[index].org;
    info->org[0] = *org;
    info->org[1] = org[1];
    info->org[2] = org[2];
#else
    // Unlike the 2D/3D loaded-sound getters above, AL_SEC_OFFSET can't be used for fraction
    // here: for a queued streaming source it only reports the offset *within the currently
    // playing buffer chunk*, not a running position across the whole stream. Use the
    // cumulative frame counter SND_FillStreamBuffers maintains instead (see AlStreamState::
    // framesQueued's comment - a small, bounded look-ahead error vs. true playback position,
    // not exact, but far closer than a chunk-relative offset would be).
    AlStreamState *stream = &g_streamState[index];
    int baserate = g_snd.chaninfo[index].soundFileInfo.baserate;
    int totalMsec = g_snd.chaninfo[index].totalMsec;
    if (totalMsec > 0 && baserate > 0)
    {
        uint64_t totalFrames = (uint64_t)totalMsec * baserate / 1000;
        info->fraction = totalFrames > 0 ? (float)stream->framesQueued / totalFrames : 0.0f;
    }
    else
    {
        info->fraction = 0.0f;
    }

    // See SND_GetStreamChannelPlaybackRate/SND_Get2DChannelPlaybackRate for the AL_PITCH-
    // is-a-ratio conversion; additionally strips the current global timescale back out
    // (mirroring the Miles branch above), so a restore under a *different* timescale
    // re-applies whatever timescale is active then rather than the one saved with.
    ALuint source = alGlob.source[index];
    ALfloat alPitch = 1.0f;
    alGetSourcef(source, AL_PITCH, &alPitch);
    int rate = SnapFloatToInt(alPitch * baserate);
    if (g_snd.chaninfo[index].timescale)
        rate = SnapFloatToInt((float)rate / g_snd.timescale);
    info->rate = rate;

    info->basevolume = g_snd.chaninfo[index].basevolume;
    ALfloat gain = 0.0f;
    alGetSourcef(source, AL_GAIN, &gain);
    if (g_snd.volume == 0.0)
        info->volume = g_snd.chaninfo[index].basevolume;
    else
        info->volume = gain / g_snd.volume;

    // g_snd.chaninfo[index].org is the plain world-space entity position tracked every
    // frame by SND_UpdateStreamChannel - already fully backend-agnostic, no AL query needed.
    float *org = g_snd.chaninfo[index].org;
    info->org[0] = org[0];
    info->org[1] = org[1];
    info->org[2] = org[2];
#endif // KISAK_SOUND

#endif // KISAK_DEDICATED
}

void __cdecl SND_SetStreamChannelFromSaveInfo(int index, snd_save_stream_t *info)
{
    float volume; // [esp+4h] [ebp-4h]

    iassert(index >= SND_FIRST_STREAM_CHANNEL && index < SND_FIRST_STREAM_CHANNEL + g_snd.max_stream_channels);

    volume = info->volume * g_snd.volume;
    SND_SetStreamChannelVolume(index, volume);
}

int __cdecl SND_GetSoundFileSize(uint32_t *pSoundFile)
{
    iassert(pSoundFile);

    return pSoundFile[2];
}

void __cdecl SND_DriverPostUpdate()
{
#ifndef KISAK_XBOX
    SND_UpdateEqs();
#endif
    KISAK_NULLSUB();
}

void __cdecl SND_Update2DChannel(int i, int frametime)
{
#if KISAK_DEDICATED
    return;
#else

    float v2; // [esp+4h] [ebp-18h]
    float volume; // [esp+8h] [ebp-14h]
    float volumea; // [esp+8h] [ebp-14h]
    const snd_alias_t *alias1; // [esp+Ch] [ebp-10h]
    const snd_alias_t *alias0; // [esp+10h] [ebp-Ch]
    snd_channel_info_t *chaninfo; // [esp+18h] [ebp-4h]

    iassert(i >= 0 && i < 0 + g_snd.max_2D_channels);

    chaninfo = &g_snd.chaninfo[i];
    if (!chaninfo->paused)
    {
        alias0 = chaninfo->alias0;
        alias1 = chaninfo->alias1;
        iassert(alias0);
        iassert(alias1);

        volume = chaninfo->basevolume;
#ifndef KISAK_SOUND
        bool finishedOrChaining = (!chaninfo->startDelay && AIL_sample_status(milesGlob.handle_sample[i]) == 2)
            || (alias0->chainAliasName && chaninfo->totalMsec + chaninfo->startTime - g_snd.time <= 0);
#else
        ALint state;
        alGetSourcei(alGlob.source[i], AL_SOURCE_STATE, &state);
        bool finishedOrChaining = (!chaninfo->startDelay && state == AL_STOPPED)
            || (alias0->chainAliasName && chaninfo->totalMsec + chaninfo->startTime - g_snd.time <= 0);
#endif
        if (finishedOrChaining)
        {
            SND_StopChannelAndPlayChainAlias(i);
        }
        else
        {
            if (g_snd.slaveLerp != 0.0 && !g_snd.chaninfo[i].master && (alias0->flags & 4) != 0)
                volume = SND_GetLerpedSlavePercentage(alias0->slavePercentage) * volume;
            iassert(SNDALIASFLAGS_GET_CHANNEL(alias0->flags) < 64);
            volumea = volume * g_snd.channelvol->channelvol[(alias0->flags & 0x3F00) >> 8].volume;
            v2 = volumea * g_snd.volume;
            SND_Set2DChannelVolume(i, v2);
            MSS_ResumeSample(i, frametime);
        }
    }

#endif // KISAK_DEDICATED
}

void __cdecl SND_Update3DChannel(int i, int frametime)
{
#ifdef KISAK_DEDICATED
    return;
#else

    double v2; // st7
    double LerpedSlavePercentage; // st7
    float v4; // [esp+Ch] [ebp-48h]
    float radius; // [esp+10h] [ebp-44h]
    snd_listener *a; // [esp+14h] [ebp-40h]
    float diff[3]; // [esp+1Ch] [ebp-38h] BYREF
    float volume; // [esp+28h] [ebp-2Ch]
    float lerp; // [esp+2Ch] [ebp-28h]
    const snd_alias_t *alias1; // [esp+30h] [ebp-24h]
    float distMin; // [esp+34h] [ebp-20h]
    const snd_alias_t *alias0; // [esp+38h] [ebp-1Ch]
    float org[3]; // [esp+3Ch] [ebp-18h] BYREF
    int timeleft; // [esp+48h] [ebp-Ch]
    float distMax; // [esp+4Ch] [ebp-8h]
    snd_channel_info_t *chaninfo; // [esp+50h] [ebp-4h]

    iassert(i >= (0 + 8) && i < (0 + 8) + g_snd.max_3D_channels);
    chaninfo = &g_snd.chaninfo[i];
    if (!chaninfo->paused)
    {
        alias0 = chaninfo->alias0;
        alias1 = chaninfo->alias1;
        iassert(alias0);
        iassert(alias1);
        lerp = chaninfo->lerp;
        volume = chaninfo->basevolume;
#ifndef KISAK_SOUND
        bool finishedOrChaining = (!chaninfo->startDelay && AIL_sample_status(milesGlob.handle_sample[i]) == 2)
            || ((timeleft = chaninfo->totalMsec + chaninfo->startTime - g_snd.time, alias0->chainAliasName) && timeleft <= 0);
#else
        timeleft = chaninfo->totalMsec + chaninfo->startTime - g_snd.time;
        ALint state;
        alGetSourcei(alGlob.source[i], AL_SOURCE_STATE, &state);
        bool finishedOrChaining = (!chaninfo->startDelay && state == AL_STOPPED)
            || (alias0->chainAliasName && timeleft <= 0);
#endif
        if (finishedOrChaining)
        {
            SND_StopChannelAndPlayChainAlias(i);
        }
        else
        {
            SND_GetCurrent3DPosition(g_snd.chaninfo[i].sndEnt, g_snd.chaninfo[i].offset, org);
            SND_Set3DPosition(i, org);
            distMin = (1.0 - lerp) * alias0->distMin + alias1->distMin * lerp;
            distMax = (1.0 - lerp) * alias0->distMax + alias1->distMax * lerp;
            a = &g_snd.listeners[SND_GetListenerIndexNearestToOrigin(org)];
            Vec3Sub(a->orient.origin, org, diff);
            radius = Vec3Length(diff);
            v2 = SND_Attenuate(alias0->volumeFalloffCurve, radius, distMin, distMax);
            volume = v2 * volume;
            if (g_snd.slaveLerp != 0.0 && !g_snd.chaninfo[i].master && (alias0->flags & 4) != 0)
            {
                LerpedSlavePercentage = SND_GetLerpedSlavePercentage(alias0->slavePercentage);
                volume = LerpedSlavePercentage * volume;
            }
            iassert(SNDALIASFLAGS_GET_CHANNEL(alias0->flags) < 64);
            volume = volume * g_snd.channelvol->channelvol[(alias0->flags & 0x3F00) >> 8].volume;
            v4 = volume * g_snd.volume;
            SND_Set3DChannelVolume(i, v4);
            MSS_ResumeSample(i, frametime);
        }
    }

#endif // KISAK_DEDICATED
}

void __cdecl SND_UpdateStreamChannel(int i, int frametime)
{
#ifdef KISAK_DEDICATED
    return;
#else

    int v2; // [esp+4h] [ebp-1Ch]
    float volume; // [esp+Ch] [ebp-14h]
    float volumea; // [esp+Ch] [ebp-14h]
    float volumeb; // [esp+Ch] [ebp-14h]
    int listenerIndex; // [esp+10h] [ebp-10h]
    const snd_alias_t *alias1; // [esp+14h] [ebp-Ch]
    const snd_alias_t *alias0; // [esp+18h] [ebp-8h]
    snd_channel_info_t *chaninfo; // [esp+1Ch] [ebp-4h]

    iassert(i >= ((0 + 8) + 32) && i < ((0 + 8) + 32) + g_snd.max_stream_channels);

    chaninfo = &g_snd.chaninfo[i];
    if (!chaninfo->paused && (i >= 45 || SND_UpdateBackgroundVolume(i - 40, frametime)))
    {
        alias0 = chaninfo->alias0;
        alias1 = chaninfo->alias1;
        iassert(alias0);
        iassert(alias1);
        volume = chaninfo->basevolume;
#ifndef KISAK_SOUND
        bool stillPlaying = g_snd.chaninfo[i].startDelay || AIL_stream_status((HSTREAM)milesGlob.handle_sample[i]) != 2;
#else
        // Refill/re-queue buffers *before* checking whether the source has actually
        // stopped, since topping up in time is exactly what prevents it from stopping -
        // SND_FillStreamBuffers returning true means there's still audio queued or being
        // decoded (including a fresh loop-back), independent of AL_SOURCE_STATE.
        bool hasMoreToPlay = SND_FillStreamBuffers(i);
        ALint state;
        alGetSourcei(alGlob.source[i], AL_SOURCE_STATE, &state);
        bool stillPlaying = g_snd.chaninfo[i].startDelay || hasMoreToPlay || state != AL_STOPPED;
#endif
        if (stillPlaying)
        {
            if (SND_IsAliasChannel3D(SNDALIASFLAGS_GET_CHANNEL(alias0->flags)))
            {
                SND_GetCurrent3DPosition(g_snd.chaninfo[i].sndEnt, g_snd.chaninfo[i].offset, g_snd.chaninfo[i].org);
                listenerIndex = SND_GetListenerIndexNearestToOrigin(g_snd.chaninfo[i].org);
                SND_Set3DStreamPosition(i, listenerIndex, g_snd.chaninfo[i].org);
                volume = SND_GetStream3DVolumeFallOff(i, listenerIndex) * volume;
            }
            if (g_snd.slaveLerp != 0.0 && !g_snd.chaninfo[i].master && (alias0->flags & 4) != 0)
                volume = SND_GetLerpedSlavePercentage(alias0->slavePercentage) * volume;

            iassert(SNDALIASFLAGS_GET_CHANNEL(alias0->flags) < 64);

            volumea = volume * g_snd.channelvol->channelvol[(alias0->flags & 0x3F00) >> 8].volume;
            volumeb = volumea * g_snd.volume;
            SND_SetStreamChannelVolume(i, volumeb);
            if (g_snd.chaninfo[i].startDelay)
            {
                if (g_snd.chaninfo[i].startDelay - frametime > 0)
                    v2 = g_snd.chaninfo[i].startDelay - frametime;
                else
                    v2 = 0;
                g_snd.chaninfo[i].startDelay = v2;
                if (!g_snd.chaninfo[i].startDelay)
                {
#ifndef KISAK_SOUND
                    AIL_pause_stream((HSTREAM)milesGlob.handle_sample[i], 0);
#else
                    alSourcePlay(alGlob.source[i]);
#endif
                }
            }
        }
        else
        {
            SND_StopChannelAndPlayChainAlias(i);
        }
    }

#endif // KISAK_DEDICATED
}

void __cdecl SND_SetHWND(HWND hwnd)
{
#ifdef KISAK_DEDICATED
    return;
#else
#ifndef KISAK_SOUND
    if (g_snd.Initialized2d)
        AIL_set_DirectSound_HWND(milesGlob.driver, hwnd);
#else
    // OpenAL has no window-handle dependency (no DirectSound backend to bind).
#endif // KISAK_SOUND
#endif // KISAK_DEDICATED
}

void __cdecl SND_SetData(MssSoundCOD4 *mssSound, void *srcData)
{
#if KISAK_DEDICATED
    return;
#else
    // KISAKTODO: double check MssSound struct usage here. It looks 'okay' at first glance

#ifndef KISAK_SOUND
    _AILMIXINFO mixinfo; // [esp+Ch] [ebp-80h] BYREF
    int digitalFormat; // [esp+88h] [ebp-4h]

    if (mssSound->info.rate > g_snd.playback_rate && mssSound->info.format != 17)
    {
        memset(&mixinfo, 0, sizeof(mixinfo));
        // LWSS Add: sound struct conversion
        mixinfo.Info.format = mssSound->info.format;
        mixinfo.Info.data_ptr = mssSound->info.data_ptr;
        mixinfo.Info.data_len = mssSound->info.data_len;
        mixinfo.Info.rate = mssSound->info.rate;
        mixinfo.Info.bits = mssSound->info.bits;
        mixinfo.Info.channels = mssSound->info.channels;
        mixinfo.Info.channel_mask = ~0U; // NEW!
        mixinfo.Info.samples = mssSound->info.samples;
        mixinfo.Info.block_size = mssSound->info.block_size;
        mixinfo.Info.initial_ptr = mssSound->info.initial_ptr;

        mixinfo.Info.data_ptr = srcData;
        mixinfo.Info.initial_ptr = srcData;
        while (mssSound->info.rate > g_snd.playback_rate)
        {
            //mssSound->info.rate >>= 1;
            mssSound->info.rate /= 2;
            //mssSound->info.samples >>= 1;
            mssSound->info.samples /= 2;
        }
        digitalFormat = MSS_DigitalFormatType(mssSound->info.format, mssSound->info.bits, mssSound->info.channels);
        mssSound->info.data_len = AIL_size_processed_digital_audio(mssSound->info.rate, digitalFormat, 1, &mixinfo);
        mssSound->data = MSS_Alloc(mssSound->info.data_len, mssSound->info.rate);
        AIL_process_digital_audio(
            mssSound->data,
            mssSound->info.data_len,
            mssSound->info.rate,
            mssSound->info.format,
            1,
            &mixinfo);
    }
    else
    {
        mssSound->data = MSS_Alloc(mssSound->info.data_len, mssSound->info.rate);
        Com_Memcpy(mssSound->data, srcData, mssSound->info.data_len);
    }
#else
    // dr_wav (see SND_LoadFromBuffer, snd_driver_load_obj.cpp) always decodes to 16-bit PCM
    // for us, so unlike the Miles branch above there's no ADPCM format to worry about here.
    if (mssSound->info.rate > g_snd.playback_rate)
    {
        // Resample down to g_snd.playback_rate via simple decimation (nearest-frame
        // resample), matching the halving loop in the Miles branch above. A real
        // low-pass-filtered resample would sound better, but this matches WORK.md Phase 3's
        // stated scope - revisit if downsampled loaded sounds turn out to sound too aliased.
        uint32_t srcFrameCount = mssSound->info.samples;
        uint32_t channels = mssSound->info.channels;
        uint32_t rate = mssSound->info.rate;
        uint32_t frameCount = srcFrameCount;

        while (rate > g_snd.playback_rate)
        {
            rate /= 2;
            frameCount /= 2;
        }

        uint32_t newDataLen = frameCount * channels * sizeof(int16_t);
        mssSound->data = MSS_Alloc(newDataLen, rate);

        const int16_t *src16 = (const int16_t *)srcData;
        int16_t *dst16 = (int16_t *)mssSound->data;
        for (uint32_t i = 0; i < frameCount; ++i)
        {
            uint32_t srcFrame = (uint32_t)((uint64_t)i * srcFrameCount / frameCount);
            for (uint32_t c = 0; c < channels; ++c)
                dst16[i * channels + c] = src16[srcFrame * channels + c];
        }

        mssSound->info.rate = rate;
        mssSound->info.samples = frameCount;
        mssSound->info.data_len = newDataLen;
    }
    else
    {
        mssSound->data = MSS_Alloc(mssSound->info.data_len, mssSound->info.rate);
        Com_Memcpy(mssSound->data, srcData, mssSound->info.data_len);
    }
#endif
    mssSound->info.data_ptr = mssSound->data;
    mssSound->info.initial_ptr = mssSound->data;
#endif // KISAK_DEDICATED
}

#ifdef KISAK_SP
void SND_SetEqLerp(double lerp)
{
    if (lerp < 0.0 || lerp > 1.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\xenon\\snd_driver.cpp",
            1740,
            0,
            "%s\n\t(lerp) = %g",
            HIDWORD(lerp),
            LODWORD(lerp));
#if KISAK_XBOX
    xaGlob.eqLerp = lerp;
#elif !defined(KISAK_SOUND)
	milesGlob.eqLerp = (float)lerp;
	SND_UpdateEqs();
#else
	alGlob.eqLerp = (float)lerp;
	SND_UpdateEqs();
#endif
}
#endif