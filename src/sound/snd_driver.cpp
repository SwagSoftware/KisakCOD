#include "snd_local.h"
#include "snd_public.h"
#include <qcommon/mem_track.h>
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

void __cdecl TRACK_snd_driver()
{
    track_static_alloc_internal(&paGlob, sizeof(paGlob), "paGlob", 13);
}

bool __cdecl SND_IsMultiChannel()
{
    return paGlob.isMultiChannel;
}

char __cdecl SND_InitDriver()
{
    snd_khz = Dvar_RegisterInt("snd_khz", 44, (DvarLimits)0x2C0000000BLL,
        DVAR_ARCHIVE | DVAR_LATCH, "The game sound frequency.");
    snd_outputConfiguration = Dvar_RegisterEnum("snd_outputConfiguration",
        snd_outputConfigurationStrings, 0, DVAR_ARCHIVE | DVAR_LATCH,
        "Sound output configuration");

    if (!PA_Startup())
        return 0;

    if (!PA_Init())
    {
        PA_ShutdownCleanup();
        return 0;
    }

    PA_InitChannels();
    PA_InitEq();

    return 1;
}

void __cdecl SND_ShutdownDriver()
{
    R_Cinematic_StopPlayback();
    R_Cinematic_SyncNow();
    PA_ShutdownCleanup();
}

int __cdecl SND_GetDriverCPUPercentage()
{
    return 0;
}

void __cdecl SND_Set3DPosition(int index, const float *org)
{
    float delta[3] = { 0 };
    float transformed[3] = { 0 };

    int32_t li = SND_GetListenerIndexNearestToOrigin(org);
    Vec3Sub(org, g_snd.listeners[li].orient.origin, delta);
    MatrixTransposeTransformVector(delta, g_snd.listeners[li].orient.axis, transformed);

    float distMax = (g_snd.chaninfo[index].alias0)
        ? g_snd.chaninfo[index].alias0->distMax : 2000.0f;
    float pan = (distMax > 0.0f) ? (transformed[1] / distMax) : 0.0f;
    pan = (pan < -1.0f) ? -1.0f : (pan > 1.0f) ? 1.0f : pan;

    paGlob.channels[index].panL = sqrtf(0.5f * (1.0f - pan));
    paGlob.channels[index].panR = sqrtf(0.5f * (1.0f + pan));
}

void __cdecl SND_Stop2DChannel(int index)
{
    paGlob.channels[index].playing = false;
    paGlob.channels[index].pcmPos = 0.0f;
    SND_ResetChannelInfo(index);
    SND_RemoveVoice(g_snd.chaninfo[index].entchannel);
}

void __cdecl SND_Pause2DChannel(int index)
{
    paGlob.channels[index].paused = true;
    g_snd.chaninfo[index].paused = 1;
}

void __cdecl SND_Unpause2DChannel(int index, int timeshift)
{
    if (!g_snd.chaninfo[index].startDelay)
        paGlob.channels[index].paused = false;

    g_snd.chaninfo[index].soundFileInfo.endtime += timeshift;
    g_snd.chaninfo[index].startTime += timeshift;
    g_snd.chaninfo[index].paused = 0;
}

bool __cdecl SND_Is2DChannelFree(int index)
{
    return !paGlob.channels[index].playing
        && !g_snd.chaninfo[index].paused
        && !g_snd.chaninfo[index].startDelay
        && g_snd.chaninfo[index].alias0 == NULL;
}

void __cdecl SND_Stop3DChannel(int index)
{
    paGlob.channels[index].playing = false;
    paGlob.channels[index].pcmPos = 0.0f;
    SND_ResetChannelInfo(index);
    SND_RemoveVoice(g_snd.chaninfo[index].entchannel);
}

void __cdecl SND_Pause3DChannel(int index)
{
    paGlob.channels[index].paused = true;
    g_snd.chaninfo[index].paused = 1;
}

void __cdecl SND_Unpause3DChannel(int index, int timeshift)
{
    if (!g_snd.chaninfo[index].startDelay)
        paGlob.channels[index].paused = false;

    g_snd.chaninfo[index].soundFileInfo.endtime += timeshift;
    g_snd.chaninfo[index].startTime += timeshift;
    g_snd.chaninfo[index].paused = 0;
}

bool __cdecl SND_Is3DChannelFree(int index)
{
    return !paGlob.channels[index].playing
        && !g_snd.chaninfo[index].paused
        && !g_snd.chaninfo[index].startDelay
        && g_snd.chaninfo[index].alias0 == NULL;
}

void __cdecl SND_StopStreamChannel(int index)
{
    int si = index - 40;
    if (si >= 0 && si < 13)
        PA_StopStreamFillThread(si);
    paGlob.channels[index].playing = false;
    paGlob.channels[index].pcmPos = 0.0f;
    SND_ResetChannelInfo(index);
    SND_RemoveVoice(g_snd.chaninfo[index].entchannel);
}

void __cdecl SND_PauseStreamChannel(int index)
{
    paGlob.channels[index].paused = true;
    g_snd.chaninfo[index].paused = 1;
}

void __cdecl SND_UnpauseStreamChannel(int index, int timeshift)
{
    if (!g_snd.chaninfo[index].startDelay)
        paGlob.channels[index].paused = false;

    g_snd.chaninfo[index].soundFileInfo.endtime += timeshift;
    g_snd.chaninfo[index].startTime += timeshift;
    g_snd.chaninfo[index].paused = 0;
}

bool __cdecl SND_IsStreamChannelFree(int index)
{
    return !paGlob.channels[index].playing
        && !g_snd.chaninfo[index].paused
        && !g_snd.chaninfo[index].startDelay
        && g_snd.chaninfo[index].alias0 == NULL;
}

void __cdecl SND_ApplyChannelMap(_SAMPLE *handle, const snd_alias_t *alias, int srcChannelCount)
{
    // No MSS speaker mapping in PortAudio path; panning handled via SND_Set3DPosition
}

int __cdecl SND_StartAlias2DSample(SndStartAliasInfo *startAliasInfo, int *pChannel)
{
    int total_msec;
    int start_msec;
    int playbackId;
    float realVolume;
    MssSoundCOD4 *sound;
    int entchannel;
    int index;

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

    entchannel = (startAliasInfo->alias0->flags & 0x3F00) >> 8;
    if (!SND_HasFreeVoice(entchannel))
        return -1;
    index = SND_FindFree2DChannel(startAliasInfo, entchannel);
    if (pChannel)
        *pChannel = index;
    if (index < 0)
        return -1;

    iassert((index >= 0 && index < 0 + g_snd.max_2D_channels));

    sound = &startAliasInfo->alias0->soundFile->u.loadSnd->sound;

    PaChannelState *ch = &paGlob.channels[index];
    ch->pcmData     = (int16_t *)sound->info.data_ptr;
    ch->pcmFrames   = sound->info.samples;
    ch->srcRate     = sound->info.rate;
    ch->srcChannels = sound->info.channels;
    ch->looping     = (startAliasInfo->alias0->flags & 1) != 0;
    ch->panL        = sqrtf(0.5f);
    ch->panR        = sqrtf(0.5f);
    ch->stream      = NULL;

    float pitchScale = startAliasInfo->timescale ? (float)g_snd.timescale : 1.0f;
    ch->pitch = startAliasInfo->pitch * pitchScale;

    realVolume = startAliasInfo->volume
        * g_snd.volume
        * g_snd.channelvol->channelvol[entchannel].volume;
    if (g_snd.slaveLerp != 0.0 && !startAliasInfo->master && (startAliasInfo->alias0->flags & 4) != 0)
        realVolume = (float)(SND_GetLerpedSlavePercentage(startAliasInfo->alias0->slavePercentage) * realVolume);
    ch->gain = realVolume;

    total_msec = (ch->srcRate > 0) ? (ch->pcmFrames * 1000 / ch->srcRate) : 0;

    if (startAliasInfo->timeshift >= total_msec)
        return SND_SetPlaybackIdNotPlayed(index);

    if (startAliasInfo->fraction == 0.0)
    {
        if (startAliasInfo->timeshift)
            start_msec = startAliasInfo->timeshift;
        else if ((startAliasInfo->alias0->flags & 0x20) != 0)
            start_msec = SnapFloatToInt(random() * (float)total_msec) & 0xFFFFFF80;
        else
            start_msec = 0;
    }
    else
    {
        start_msec = SnapFloatToInt((float)total_msec * startAliasInfo->fraction);
    }

    if (start_msec)
        startAliasInfo->startDelay = 0;

    ch->pcmPos  = (ch->srcRate > 0) ? ((float)start_msec * ch->srcRate / 1000.0f) : 0.0f;
    ch->playing = true;
    ch->paused  = (startAliasInfo->startDelay > 0)
        || (g_snd.paused != 0 && g_snd.pauseSettings[entchannel] != 0);

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

void __cdecl SND_Apply3DSpatializationTweaks(_SAMPLE *handle, const snd_alias_t *alias)
{
    // No multi-channel tweaks in PortAudio path; pan set via SND_Set3DPosition
}

int __cdecl SND_StartAlias3DSample(SndStartAliasInfo *startAliasInfo, int *pChannel)
{
    double LerpedSlavePercentage;
    float diff[15];
    int total_msec;
    int start_msec;
    const float *listener;
    float attenuation;
    int playbackId;
    float realVolume;
    MssSoundCOD4 *sound;
    float distance;
    float distMin;
    int entchannel;
    int index;
    float distMax;

    if (!startAliasInfo->alias0)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 942, 0, "%s", "startAliasInfo->alias0");
    if ((startAliasInfo->alias0->flags & 0xC0) >> 6 != 1)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 943, 0, "%s",
            "SNDALIASFLAGS_GET_TYPE( startAliasInfo->alias0->flags ) == SAT_LOADED");
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
        MyAssertHandler(".\\win32\\snd_driver.cpp", 949, 0, "%s",
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
        MyAssertHandler(".\\win32\\snd_driver.cpp", 965, 0, "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)", index);

    sound = &startAliasInfo->alias0->soundFile->u.loadSnd->sound;
    distMin = (1.0f - startAliasInfo->lerp) * startAliasInfo->alias0->distMin
        + startAliasInfo->alias1->distMin * startAliasInfo->lerp;
    distMax = (1.0f - startAliasInfo->lerp) * startAliasInfo->alias0->distMax
        + startAliasInfo->alias1->distMax * startAliasInfo->lerp;

    listener = g_snd.listeners[SND_GetListenerIndexNearestToOrigin(startAliasInfo->org)].orient.origin;
    Vec3Sub(listener, startAliasInfo->org, diff);
    distance = Vec3Length(diff);
    attenuation = (float)SND_Attenuate(startAliasInfo->alias0->volumeFalloffCurve, distance, distMin, distMax);
    realVolume = startAliasInfo->volume
        * attenuation
        * g_snd.channelvol->channelvol[entchannel].volume;
    realVolume = realVolume * g_snd.volume;
    if (g_snd.slaveLerp != 0.0 && !startAliasInfo->master && (startAliasInfo->alias0->flags & 4) != 0)
    {
        LerpedSlavePercentage = SND_GetLerpedSlavePercentage(startAliasInfo->alias0->slavePercentage);
        realVolume = (float)(LerpedSlavePercentage * realVolume);
    }

    PaChannelState *ch = &paGlob.channels[index];
    ch->pcmData     = (int16_t *)sound->info.data_ptr;
    ch->pcmFrames   = sound->info.samples;
    ch->srcRate     = sound->info.rate;
    ch->srcChannels = sound->info.channels;
    ch->gain        = realVolume;
    ch->looping     = (startAliasInfo->alias0->flags & 1) != 0;
    ch->stream      = NULL;

    float pitchScale = startAliasInfo->timescale ? (float)g_snd.timescale : 1.0f;
    ch->pitch = startAliasInfo->pitch * pitchScale;

    SND_Set3DPosition(index, startAliasInfo->org);

    total_msec = (ch->srcRate > 0) ? (ch->pcmFrames * 1000 / ch->srcRate) : 0;

    if (startAliasInfo->timeshift >= total_msec)
        return SND_SetPlaybackIdNotPlayed(index);

    if (startAliasInfo->fraction == 0.0)
    {
        if (startAliasInfo->timeshift)
            start_msec = startAliasInfo->timeshift;
        else if ((startAliasInfo->alias0->flags & 0x20) != 0)
            start_msec = SnapFloatToInt(random() * (float)total_msec) & 0xFFFFFF80;
        else
            start_msec = 0;
    }
    else
    {
        start_msec = SnapFloatToInt((float)total_msec * startAliasInfo->fraction);
    }

    if (start_msec)
        startAliasInfo->startDelay = 0;

    ch->pcmPos  = (ch->srcRate > 0) ? ((float)start_msec * ch->srcRate / 1000.0f) : 0.0f;
    ch->playing = true;
    ch->paused  = (startAliasInfo->startDelay > 0)
        || (g_snd.paused != 0 && g_snd.pauseSettings[entchannel] != 0);

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

void __cdecl SND_Set3DStreamPosition(int index, int listenerIndex, const float *org)
{
    float delta[3];
    float transformed[3];

    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            619,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);

    Vec3Sub(org, g_snd.listeners[listenerIndex].orient.origin, delta);
    MatrixTransposeTransformVector(delta, g_snd.listeners[listenerIndex].orient.axis, transformed);

    float distMax = (g_snd.chaninfo[index].alias0)
        ? g_snd.chaninfo[index].alias0->distMax : 2000.0f;
    float pan = (distMax > 0.0f) ? (transformed[1] / distMax) : 0.0f;
    pan = (pan < -1.0f) ? -1.0f : (pan > 1.0f) ? 1.0f : pan;
    paGlob.channels[index].panL = sqrtf(0.5f * (1.0f - pan));
    paGlob.channels[index].panR = sqrtf(0.5f * (1.0f + pan));
}

double __cdecl SND_GetStream3DVolumeFallOff(int index, int listenerIndex)
{
    float diff[3];
    float maxdist;
    float dist;
    float lerp;
    const snd_alias_t *alias1;
    const snd_alias_t *alias0;
    float mindist;

    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            581,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < g_snd.max_stream_channels + ((0 + 8) + 32))",
            index);
    alias0 = g_snd.chaninfo[index].alias0;
    alias1 = g_snd.chaninfo[index].alias1;
    if (!SND_IsAliasChannel3D((alias0->flags & 0x3F00) >> 8))
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            585,
            0,
            "%s",
            "SND_IsAliasChannel3D( SNDALIASFLAGS_GET_CHANNEL( alias0->flags ) )");
    Vec3Sub(g_snd.listeners[listenerIndex].orient.origin, g_snd.chaninfo[index].org, diff);
    dist = Vec3Length(diff);
    lerp = g_snd.chaninfo[index].lerp;
    mindist = (1.0f - lerp) * alias0->distMin + alias1->distMin * lerp;
    maxdist = (1.0f - lerp) * alias0->distMax + alias1->distMax * lerp;
    return SND_Attenuate(alias0->volumeFalloffCurve, dist, mindist, maxdist);
}

int __cdecl SND_StartAliasStreamOnChannel(SndStartAliasInfo *startAliasInfo, int index)
{
    double LerpedSlavePercentage;
    double Stream3DVolumeFallOff;
    float *org;
    char filename[132];
    char realname[256];
    int start_msec;
    int listenerIndex;
    int playbackId;
    float realVolume;
    int srcChannelCount;
    int entchannel;

    iassert(startAliasInfo->alias0);
    iassert(SNDALIASFLAGS_GET_TYPE(startAliasInfo->alias0->flags) == SAT_STREAMED);
    iassert(startAliasInfo->alias1);
    iassert(SNDALIASFLAGS_GET_TYPE(startAliasInfo->alias1->flags) == SAT_STREAMED);
    iassert((index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels));
    iassert(FS_Initialized());

    entchannel = (startAliasInfo->alias0->flags & 0x3F00) >> 8;
    if (!SND_HasFreeVoice(entchannel))
        return -1;

    if (!startAliasInfo->alias0->soundFile->exists)
    {
        Com_GetSoundFileName(startAliasInfo->alias0, filename, 128);
        Com_DPrintf(9,
            "Tried to play streamed sound '%s' from alias '%s', but it was not found at load time.\n",
            filename, startAliasInfo->alias0->aliasName);
        return SND_SetPlaybackIdNotPlayed(index);
    }

    Com_GetSoundFileName(startAliasInfo->alias0, filename, 128);
    Com_sprintf(realname, 0x100u, "sound/%s", filename);

    // Stop any existing stream on this channel before opening a new one
    PA_StopStreamFillThread(index - 40);

    PaStreamState *st = &paGlob.streamStates[index - 40];

    {
        PROF_SCOPED("SND_open_stream");
        if (!drmp3_init_file(&st->wav, realname, NULL))
        {
            Com_PrintError(9, "Couldn't open stream '%s' from alias '%s'\n",
                realname, startAliasInfo->alias0->aliasName);
            return SND_SetPlaybackIdNotPlayed(index);
        }
    }

    srcChannelCount = (int)st->wav.channels;
    uint32_t baserate = st->wav.sampleRate;

    float pitchScale = startAliasInfo->timescale ? (float)g_snd.timescale : 1.0f;
    paGlob.channels[index].pitch   = startAliasInfo->pitch * pitchScale;
    paGlob.channels[index].srcRate = (int)baserate;

    realVolume = startAliasInfo->volume
        * g_snd.volume
        * g_snd.channelvol->channelvol[entchannel].volume;
    if (g_snd.slaveLerp != 0.0 && !startAliasInfo->master && (startAliasInfo->alias0->flags & 4) != 0)
    {
        LerpedSlavePercentage = SND_GetLerpedSlavePercentage(startAliasInfo->alias0->slavePercentage);
        realVolume = (float)(LerpedSlavePercentage * realVolume);
    }

    st->looping = (startAliasInfo->alias0->flags & 1) != 0;
    paGlob.channels[index].looping = st->looping;

    drwav_uint64 totalFrames = st->wav.totalPCMFrameCount;
    int total_msec = (baserate > 0) ? (int)(totalFrames * 1000 / baserate) : 0;

    if (startAliasInfo->timeshift >= total_msec)
    {
        drmp3_uninit(&st->wav);
        return SND_SetPlaybackIdNotPlayed(index);
    }

    if (!total_msec)
    {
        Com_PrintError(1, "ERROR: Sound file '%s' is zero length, invalid\n", realname);
        drmp3_uninit(&st->wav);
        return SND_SetPlaybackIdNotPlayed(index);
    }

    if (startAliasInfo->fraction == 0.0)
    {
        if (startAliasInfo->timeshift)
            start_msec = startAliasInfo->timeshift;
        else if ((startAliasInfo->alias0->flags & 0x20) != 0)
            start_msec = SnapFloatToInt(random() * (float)total_msec) & 0xFFFFFF80;
        else
            start_msec = 0;
    }
    else
    {
        start_msec = SnapFloatToInt((float)total_msec * startAliasInfo->fraction);
    }

    if (start_msec)
        startAliasInfo->startDelay = 0;

    if (start_msec && baserate > 0)
    {
        drmp3_uint64 startFrame = (drmp3_uint64)((float)start_msec * baserate / 1000.0f);
        drmp3_seek_to_pcm_frame(&st->wav, startFrame);
    }

    total_msec += startAliasInfo->startDelay;
    if ((startAliasInfo->alias0->flags & 1) != 0)
        total_msec = 0;

    org = g_snd.chaninfo[index].org;
    *org   = startAliasInfo->org[0];
    org[1] = startAliasInfo->org[1];
    org[2] = startAliasInfo->org[2];

    SND_SetChannelStartInfo(index, startAliasInfo);
    SND_SetSoundFileChannelInfo(index, srcChannelCount, baserate, total_msec, start_msec, SFLS_LOADED);

    if (SND_IsAliasChannel3D((g_snd.chaninfo[index].alias0->flags & 0x3F00) >> 8))
    {
        SND_GetCurrent3DPosition(
            g_snd.chaninfo[index].sndEnt,
            g_snd.chaninfo[index].offset,
            g_snd.chaninfo[index].org);
        listenerIndex = SND_GetListenerIndexNearestToOrigin(g_snd.chaninfo[index].org);
        SND_Set3DStreamPosition(index, listenerIndex, g_snd.chaninfo[index].org);
        Stream3DVolumeFallOff = SND_GetStream3DVolumeFallOff(index, listenerIndex);
        realVolume = (float)(Stream3DVolumeFallOff * realVolume);
    }
    else
    {
        paGlob.channels[index].panL = sqrtf(0.5f);
        paGlob.channels[index].panR = sqrtf(0.5f);
    }

    SND_SetStreamChannelVolume(index, realVolume);

    paGlob.channels[index].pcmData = NULL;
    paGlob.channels[index].stream  = st;
    paGlob.channels[index].playing = true;
    paGlob.channels[index].paused  = (startAliasInfo->startDelay > 0)
        || (g_snd.paused != 0 && g_snd.pauseSettings[entchannel] != 0);

    PA_StartStreamFillThread(index - 40);

    playbackId = SND_AcquirePlaybackId(index, total_msec);
    if (playbackId != -1)
        SND_AddVoice(entchannel);
    return playbackId;
}

void __cdecl SND_SetRoomtype(int roomtype)
{
    // No reverb/roomtype in PortAudio path
}

void __cdecl SND_UpdateEqs()
{
    // No software EQ filtering in PortAudio path
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
    iassert((unsigned)eqIndex < ARRAY_COUNT(paGlob.eq));

    paGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    paGlob.eq[eqIndex].params[band][entchannel].gain = gain;
    paGlob.eq[eqIndex].params[band][entchannel].freq = freq;
    paGlob.eq[eqIndex].params[band][entchannel].q = q;
    paGlob.eq[eqIndex].params[band][entchannel].type = type;
}

void __cdecl SND_SetEqType(uint32_t entchannel, int eqIndex, uint32_t band, SND_EQTYPE type)
{
    if (entchannel >= 0x40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1264,
            0,
            "%s\n\t(entchannel) = %i",
            "(entchannel >= 0 && entchannel < 64)",
            entchannel);
    if (band > 2)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1265, 0, "%s\n\t(band) = %i", "(band >= 0 && band < 3)", band);
    iassert((unsigned)eqIndex < ARRAY_COUNT(paGlob.eq));
    paGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    paGlob.eq[eqIndex].params[band][entchannel].type = type;
}

void __cdecl SND_SetEqFreq(uint32_t entchannel, int eqIndex, uint32_t band, float freq)
{
    if (entchannel >= 0x40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1276,
            0,
            "%s\n\t(entchannel) = %i",
            "(entchannel >= 0 && entchannel < 64)",
            entchannel);
    if (band > 2)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1277, 0, "%s\n\t(band) = %i", "(band >= 0 && band < 3)", band);
    if (freq < 0.0 || freq > 20000.0)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1278, 0, "%s\n\t(freq) = %g", "(freq >= 0 && freq <= 20000)", freq);
    iassert((unsigned)eqIndex < ARRAY_COUNT(paGlob.eq));
    paGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    paGlob.eq[eqIndex].params[band][entchannel].freq = freq;
}

void __cdecl SND_SetEqGain(uint32_t entchannel, int eqIndex, uint32_t band, float gain)
{
    if (entchannel >= 0x40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1289,
            0,
            "%s\n\t(entchannel) = %i",
            "(entchannel >= 0 && entchannel < 64)",
            entchannel);
    if (band > 2)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1290, 0, "%s\n\t(band) = %i", "(band >= 0 && band < 3)", band);
    iassert((unsigned)eqIndex < ARRAY_COUNT(paGlob.eq));
    paGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    paGlob.eq[eqIndex].params[band][entchannel].gain = gain;
}

void __cdecl SND_SetEqQ(uint32_t entchannel, int eqIndex, uint32_t band, float q)
{
    if (entchannel >= 0x40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1303,
            0,
            "%s\n\t(entchannel) = %i",
            "(entchannel >= 0 && entchannel < 64)",
            entchannel);
    if (band > 2)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1304, 0, "%s\n\t(band) = %i", "(band >= 0 && band < 3)", band);
    if (q <= 0.0)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1305, 0, "%s\n\t(q) = %g", "(q > 0)", q);
    iassert((unsigned)eqIndex < ARRAY_COUNT(paGlob.eq));
    paGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    paGlob.eq[eqIndex].params[band][entchannel].q = q;
}

void __cdecl SND_DisableEq(uint32_t entchannel, int eqIndex, uint32_t band)
{
    if (entchannel >= 0x40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1316,
            0,
            "%s\n\t(entchannel) = %i",
            "(entchannel >= 0 && entchannel < 64)",
            entchannel);
    if (band > 2)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1317, 0, "%s\n\t(band) = %i", "(band >= 0 && band < 3)", band);
    iassert((unsigned)eqIndex < ARRAY_COUNT(paGlob.eq));
    paGlob.eq[eqIndex].params[band][entchannel].enabled = 0;
}

void __cdecl SND_SaveEq(MemoryFile *memFile)
{
    int band;
    int entchannel;
    int eqIndex;

    for (eqIndex = 0; eqIndex < 2; ++eqIndex)
        for (band = 0; band < 3; ++band)
            for (entchannel = 0; entchannel < 64; ++entchannel)
                MemFile_WriteData(memFile, 20, &paGlob.eq[eqIndex].params[band][entchannel]);
}

void __cdecl SND_RestoreEq(MemoryFile *memFile)
{
    int band;
    int entchannel;
    int eqIndex;

    for (eqIndex = 0; eqIndex < 2; ++eqIndex)
        for (band = 0; band < 3; ++band)
            for (entchannel = 0; entchannel < 64; ++entchannel)
                MemFile_ReadData(memFile, 20, (uint8_t *)&paGlob.eq[eqIndex].params[band][entchannel]);
}

void __cdecl SND_PrintEqParams()
{
    float *v0;
    snd_entchannel_info_t *channelName;
    int band;
    int entchannel;
    int eqIndex;

    Com_Printf(9, "Current EQ Settings\n---------------\n");
    for (entchannel = 0; entchannel < g_snd.entchannel_count; ++entchannel)
    {
        channelName = SND_GetEntChannelName(entchannel);
        Com_Printf(9, "+ %s\n", channelName->name);
        for (eqIndex = 0; eqIndex < 2; ++eqIndex)
        {
            for (band = 0; band < 3; ++band)
            {
                v0 = (float *)&paGlob.eq[eqIndex].params[band][entchannel];
                if ((uint8_t)*((uint32_t *)v0 + 4))
                    Com_Printf(9, "\t%i %s %f Hz %f dB %f q\n", band, snd_eqTypeStrings[*(uint32_t *)v0], v0[2], v0[1], v0[3]);
            }
        }
    }
}

double __cdecl SND_Get2DChannelVolume(int index)
{
    return paGlob.channels[index].gain;
}

void __cdecl SND_Set2DChannelVolume(int index, float volume)
{
    paGlob.channels[index].gain = volume;
}

double __cdecl SND_Get3DChannelVolume(int index)
{
    return paGlob.channels[index].gain;
}

void __cdecl SND_Set3DChannelVolume(int index, float volume)
{
    paGlob.channels[index].gain = volume;
}

double __cdecl SND_GetStreamChannelVolume(int index)
{
    return paGlob.channels[index].gain;
}

void __cdecl SND_SetStreamChannelVolume(int index, float volume)
{
    paGlob.channels[index].gain = volume;
}

int __cdecl SND_Get2DChannelPlaybackRate(int index)
{
    return (int)(paGlob.channels[index].pitch * paGlob.channels[index].srcRate);
}

void __cdecl SND_Set2DChannelPlaybackRate(int index, int rate)
{
    if (paGlob.channels[index].srcRate > 0)
        paGlob.channels[index].pitch = (float)rate / (float)paGlob.channels[index].srcRate;
}

int __cdecl SND_Get3DChannelPlaybackRate(int index)
{
    return (int)(paGlob.channels[index].pitch * paGlob.channels[index].srcRate);
}

void __cdecl SND_Set3DChannelPlaybackRate(int index, int rate)
{
    if (paGlob.channels[index].srcRate > 0)
        paGlob.channels[index].pitch = (float)rate / (float)paGlob.channels[index].srcRate;
}

int __cdecl SND_GetStreamChannelPlaybackRate(int index)
{
    return (int)(paGlob.channels[index].pitch * paGlob.channels[index].srcRate);
}

void __cdecl SND_SetStreamChannelPlaybackRate(int index, int rate)
{
    if (paGlob.channels[index].srcRate > 0)
        paGlob.channels[index].pitch = (float)rate / (float)paGlob.channels[index].srcRate;
}

void __cdecl SND_Update2DChannelReverb(int index)
{
    // No reverb in PortAudio path
}

void __cdecl SND_Update3DChannelReverb(int index)
{
    // No reverb in PortAudio path
}

void __cdecl SND_UpdateStreamChannelReverb(int index)
{
    // No reverb in PortAudio path
}

int __cdecl SND_Get2DChannelLength(int index)
{
    if (index < 0 || index >= g_snd.max_2D_channels)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1567, 0, "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)", index);
    PaChannelState *ch = &paGlob.channels[index];
    return (ch->srcRate > 0) ? (ch->pcmFrames * 1000 / ch->srcRate) : 0;
}

int __cdecl SND_Get3DChannelLength(int index)
{
    if (index < 8 || index >= g_snd.max_3D_channels + 8)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1578, 0, "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)", index);
    PaChannelState *ch = &paGlob.channels[index];
    return (ch->srcRate > 0) ? (ch->pcmFrames * 1000 / ch->srcRate) : 0;
}

int __cdecl SND_GetStreamChannelLength(int index)
{
    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1590, 0, "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)", index);
    PaStreamState *st = &paGlob.streamStates[index - 40];
    if (!st->active || st->wav.sampleRate == 0)
        return 0;
    return (int)(st->wav.totalPCMFrameCount * 1000 / st->wav.sampleRate);
}

void __cdecl SND_Get2DChannelSaveInfo(int index, snd_save_2D_sample_t *info)
{
    if (index < 0 || index >= g_snd.max_2D_channels)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1603, 0, "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)", index);
    PaChannelState *ch = &paGlob.channels[index];
    info->fraction = (ch->pcmFrames > 0) ? (ch->pcmPos / (float)ch->pcmFrames) : 0.0f;
    info->pitch    = g_snd.chaninfo[index].pitch;
    info->pan      = 0.0f;
    if (g_snd.volume == 0.0f)
        info->volume = g_snd.chaninfo[index].basevolume;
    else
        info->volume = ch->gain / g_snd.volume;
}

void __cdecl SND_Set2DChannelFromSaveInfo(int index, snd_save_2D_sample_t *info)
{
    if (index < 0 || index >= g_snd.max_2D_channels)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1620, 0, "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)", index);
    float volume = info->volume * g_snd.volume;
    SND_Set2DChannelVolume(index, volume);
}

void __cdecl SND_Get3DChannelSaveInfo(int index, snd_save_3D_sample_t *info)
{
    if (index < 8 || index >= g_snd.max_3D_channels + 8)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1632, 0, "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)", index);
    PaChannelState *ch = &paGlob.channels[index];
    info->fraction = (ch->pcmFrames > 0) ? (ch->pcmPos / (float)ch->pcmFrames) : 0.0f;
    info->pitch    = g_snd.chaninfo[index].pitch;
    if (g_snd.volume == 0.0f)
        info->volume = g_snd.chaninfo[index].basevolume;
    else
        info->volume = ch->gain / g_snd.volume;
    info->org[0] = g_snd.chaninfo[index].org[0];
    info->org[1] = g_snd.chaninfo[index].org[1];
    info->org[2] = g_snd.chaninfo[index].org[2];
}

void __cdecl SND_GetStreamChannelSaveInfo(int index, snd_save_stream_t *info)
{
    float *org;

    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1656,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);

    PaStreamState *st = &paGlob.streamStates[index - 40];
    if (st->active && st->wav.totalPCMFrameCount > 0)
        info->fraction = (float)((double)st->readPos / (double)st->wav.totalPCMFrameCount);
    else
        info->fraction = 0.0f;

    info->rate       = (int)(paGlob.channels[index].pitch * (float)st->wav.sampleRate);
    info->basevolume = g_snd.chaninfo[index].basevolume;
    info->pan        = 0.0f;
    if (g_snd.volume == 0.0f)
        info->volume = g_snd.chaninfo[index].basevolume;
    else
        info->volume = paGlob.channels[index].gain / g_snd.volume;
    org = g_snd.chaninfo[index].org;
    info->org[0] = *org;
    info->org[1] = org[1];
    info->org[2] = org[2];
}

void __cdecl SND_SetStreamChannelFromSaveInfo(int index, snd_save_stream_t *info)
{
    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1676,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
    float volume = info->volume * g_snd.volume;
    SND_SetStreamChannelVolume(index, volume);
}

int __cdecl SND_GetSoundFileSize(uint32_t *pSoundFile)
{
    if (!pSoundFile)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1705, 0, "%s", "pSoundFile");
    return pSoundFile[2];
}

void __cdecl SND_DriverPostUpdate()
{
    SND_UpdateEqs();
    KISAK_NULLSUB();
}

void __cdecl SND_Update2DChannel(int i, int frametime)
{
    float v2;
    float volume;
    float volumea;
    const snd_alias_t *alias1;
    const snd_alias_t *alias0;
    snd_channel_info_t *chaninfo;

    if (i < 0 || i >= g_snd.max_2D_channels)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1745,
            0,
            "%s\n\t(i) = %i",
            "(i >= 0 && i < 0 + g_snd.max_2D_channels)",
            i);
    chaninfo = &g_snd.chaninfo[i];
    if (!chaninfo->paused)
    {
        alias0 = chaninfo->alias0;
        alias1 = chaninfo->alias1;
        if (!alias0)
            MyAssertHandler(".\\win32\\snd_driver.cpp", 1753, 0, "%s", "alias0");
        if (!alias1)
            MyAssertHandler(".\\win32\\snd_driver.cpp", 1754, 0, "%s", "alias1");
        volume = chaninfo->basevolume;
        if ((!chaninfo->startDelay && !paGlob.channels[i].playing)
            || (alias0->chainAliasName && chaninfo->totalMsec + chaninfo->startTime - g_snd.time <= 0))
        {
            SND_StopChannelAndPlayChainAlias(i);
        }
        else
        {
            if (g_snd.slaveLerp != 0.0 && !g_snd.chaninfo[i].master && (alias0->flags & 4) != 0)
                volume = (float)(SND_GetLerpedSlavePercentage(alias0->slavePercentage) * volume);
            if ((alias0->flags & 0x3F00) >> 8 >= 64)
                MyAssertHandler(
                    ".\\win32\\snd_driver.cpp",
                    1773,
                    0,
                    "%s\n\t((((alias0->flags) & (((1 << 6) - 1) << ((6 + 2)))) >> ((6 + 2)))) = %i",
                    "((((alias0->flags) & (((1 << 6) - 1) << ((6 + 2)))) >> ((6 + 2))) >= 0 && (((alias0->flags) & (((1 << 6) - 1) "
                    "<< ((6 + 2)))) >> ((6 + 2))) < 64)",
                    (alias0->flags & 0x3F00) >> 8);
            volumea = volume * g_snd.channelvol->channelvol[(alias0->flags & 0x3F00) >> 8].volume;
            v2 = volumea * g_snd.volume;
            SND_Set2DChannelVolume(i, v2);

            if (chaninfo->startDelay)
            {
                int newDelay = chaninfo->startDelay - frametime;
                chaninfo->startDelay = (newDelay > 0) ? newDelay : 0;
                if (!chaninfo->startDelay
                    && (!g_snd.paused || !g_snd.pauseSettings[(alias0->flags & 0x3F00) >> 8]))
                    paGlob.channels[i].paused = false;
            }
        }
    }
}

void __cdecl SND_Update3DChannel(int i, int frametime)
{
    double v2;
    double LerpedSlavePercentage;
    float v4;
    float radius;
    snd_listener *a;
    float diff[3];
    float volume;
    float lerp;
    const snd_alias_t *alias1;
    float distMin;
    const snd_alias_t *alias0;
    float org[3];
    int timeleft;
    float distMax;
    snd_channel_info_t *chaninfo;

    if (i < 8 || i >= g_snd.max_3D_channels + 8)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1793,
            0,
            "%s\n\t(i) = %i",
            "(i >= (0 + 8) && i < (0 + 8) + g_snd.max_3D_channels)",
            i);
    chaninfo = &g_snd.chaninfo[i];
    if (!chaninfo->paused)
    {
        alias0 = chaninfo->alias0;
        alias1 = chaninfo->alias1;
        if (!alias0)
            MyAssertHandler(".\\win32\\snd_driver.cpp", 1802, 0, "%s", "alias0");
        if (!alias1)
            MyAssertHandler(".\\win32\\snd_driver.cpp", 1803, 0, "%s", "alias1");
        lerp = chaninfo->lerp;
        volume = chaninfo->basevolume;
        if ((!chaninfo->startDelay && !paGlob.channels[i].playing)
            || (timeleft = chaninfo->totalMsec + chaninfo->startTime - g_snd.time, alias0->chainAliasName) && timeleft <= 0)
        {
            SND_StopChannelAndPlayChainAlias(i);
        }
        else
        {
            SND_GetCurrent3DPosition(g_snd.chaninfo[i].sndEnt, g_snd.chaninfo[i].offset, org);
            SND_Set3DPosition(i, org);
            distMin = (1.0f - lerp) * alias0->distMin + alias1->distMin * lerp;
            distMax = (1.0f - lerp) * alias0->distMax + alias1->distMax * lerp;
            a = &g_snd.listeners[SND_GetListenerIndexNearestToOrigin(org)];
            Vec3Sub(a->orient.origin, org, diff);
            radius = Vec3Length(diff);
            v2 = SND_Attenuate(alias0->volumeFalloffCurve, radius, distMin, distMax);
            volume = (float)(v2 * volume);
            if (g_snd.slaveLerp != 0.0 && !g_snd.chaninfo[i].master && (alias0->flags & 4) != 0)
            {
                LerpedSlavePercentage = SND_GetLerpedSlavePercentage(alias0->slavePercentage);
                volume = (float)(LerpedSlavePercentage * volume);
            }
            if ((alias0->flags & 0x3F00) >> 8 >= 64)
                MyAssertHandler(
                    ".\\win32\\snd_driver.cpp",
                    1830,
                    0,
                    "%s\n\t((((alias0->flags) & (((1 << 6) - 1) << ((6 + 2)))) >> ((6 + 2)))) = %i",
                    "((((alias0->flags) & (((1 << 6) - 1) << ((6 + 2)))) >> ((6 + 2))) >= 0 && (((alias0->flags) & (((1 << 6) - 1) "
                    "<< ((6 + 2)))) >> ((6 + 2))) < 64)",
                    (alias0->flags & 0x3F00) >> 8);
            volume = volume * g_snd.channelvol->channelvol[(alias0->flags & 0x3F00) >> 8].volume;
            v4 = volume * g_snd.volume;
            SND_Set3DChannelVolume(i, v4);

            if (chaninfo->startDelay)
            {
                int newDelay = chaninfo->startDelay - frametime;
                chaninfo->startDelay = (newDelay > 0) ? newDelay : 0;
                if (!chaninfo->startDelay
                    && (!g_snd.paused || !g_snd.pauseSettings[(alias0->flags & 0x3F00) >> 8]))
                    paGlob.channels[i].paused = false;
            }
        }
    }
}

void __cdecl SND_UpdateStreamChannel(int i, int frametime)
{
    int v2;
    float volume;
    float volumea;
    float volumeb;
    int listenerIndex;
    const snd_alias_t *alias1;
    const snd_alias_t *alias0;
    snd_channel_info_t *chaninfo;

    if (i < 40 || i >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1846,
            0,
            "%s\n\t(i) = %i",
            "(i >= ((0 + 8) + 32) && i < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            i);
    chaninfo = &g_snd.chaninfo[i];
    if (!chaninfo->paused && (i >= 45 || SND_UpdateBackgroundVolume(i - 40, frametime)))
    {
        alias0 = chaninfo->alias0;
        alias1 = chaninfo->alias1;
        if (!alias0)
            MyAssertHandler(".\\win32\\snd_driver.cpp", 1860, 0, "%s", "alias0");
        if (!alias1)
            MyAssertHandler(".\\win32\\snd_driver.cpp", 1861, 0, "%s", "alias1");
        volume = chaninfo->basevolume;
        if (chaninfo->startDelay || paGlob.channels[i].playing)
        {
            if (SND_IsAliasChannel3D((alias0->flags & 0x3F00) >> 8))
            {
                SND_GetCurrent3DPosition(g_snd.chaninfo[i].sndEnt, g_snd.chaninfo[i].offset, g_snd.chaninfo[i].org);
                listenerIndex = SND_GetListenerIndexNearestToOrigin(g_snd.chaninfo[i].org);
                SND_Set3DStreamPosition(i, listenerIndex, g_snd.chaninfo[i].org);
                volume = (float)(SND_GetStream3DVolumeFallOff(i, listenerIndex) * volume);
            }
            if (g_snd.slaveLerp != 0.0 && !g_snd.chaninfo[i].master && (alias0->flags & 4) != 0)
                volume = (float)(SND_GetLerpedSlavePercentage(alias0->slavePercentage) * volume);
            if ((alias0->flags & 0x3F00) >> 8 >= 64)
                MyAssertHandler(
                    ".\\win32\\snd_driver.cpp",
                    1882,
                    0,
                    "%s\n\t((((alias0->flags) & (((1 << 6) - 1) << ((6 + 2)))) >> ((6 + 2)))) = %i",
                    "((((alias0->flags) & (((1 << 6) - 1) << ((6 + 2)))) >> ((6 + 2))) >= 0 && (((alias0->flags) & (((1 << 6) - 1) "
                    "<< ((6 + 2)))) >> ((6 + 2))) < 64)",
                    (alias0->flags & 0x3F00) >> 8);
            volumea = volume * g_snd.channelvol->channelvol[(alias0->flags & 0x3F00) >> 8].volume;
            volumeb = volumea * g_snd.volume;
            SND_SetStreamChannelVolume(i, volumeb);
            if (chaninfo->startDelay)
            {
                if (chaninfo->startDelay - frametime > 0)
                    v2 = chaninfo->startDelay - frametime;
                else
                    v2 = 0;
                chaninfo->startDelay = v2;
                if (!chaninfo->startDelay
                    && (!g_snd.paused || !g_snd.pauseSettings[(alias0->flags & 0x3F00) >> 8]))
                    paGlob.channels[i].paused = false;
            }
        }
        else
        {
            SND_StopChannelAndPlayChainAlias(i);
        }
    }
}


#ifdef KISAK_SP
void SND_SetEqLerp(double lerp)
{
#if KISAK_XBOX
    if (lerp < 0.0 || lerp > 1.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\xenon\\snd_driver.cpp",
            1740,
            0,
            "%s\n\t(lerp) = %g",
            HIDWORD(lerp),
            LODWORD(lerp));
    xaGlob.eqLerp = lerp;
#endif
}
#endif