#ifdef KISAK_OPENAL
// OpenAL implementation of the snd_driver/snd_mss interface declared in snd_local.h.
// Mirrors snd_mss.cpp function-for-function; only one of the two files is compiled for a
// given build (see scripts/common_files.cmake), selected by KISAK_SOUND. See snd_local.h
// for why these keep their historical MSS_ prefix despite backing onto OpenAL here.
#include <universal/q_shared.h>
#include "snd_local.h"
#include "snd_public.h"
#include <universal/com_files.h>
#include <qcommon/qcommon.h>
#include <universal/com_memory.h>
#include <math.h>

// alGlob is declared extern in snd_local.h; its one definition lives in snd_driver.cpp
// (KISAK_SOUND branch), matching where milesGlob's Miles-side definition already lives.

void MSS_InitFailed()
{
    if (Dvar_GetInt("r_vc_compile") != 2)
        Com_Printf(CON_CHANNEL_SOUND, "OpenAL sound system initialization failed\n");
}

// Mirrors MSS_Init + MSS_open_digital_driver in snd_mss.cpp. Unlike Miles (which opens a
// throwaway probe driver first to detect multichannel capability, then reopens for real -
// see MSS_open_digital_driver's comments), MSS_Startup already does an equivalent
// open-then-close sanity probe, so this just opens the real device/context once.
char __cdecl MSS_Init()
{
    int integer = snd_khz->current.integer;
    int hertz;
    if (integer == 11)
    {
        hertz = 11025;
    }
    else if (integer == 44)
    {
        hertz = 44100;
    }
    else
    {
        if (integer != 22)
            Com_Printf(CON_CHANNEL_SOUND, "invalid value %i for snd_khz, using 22 khz instead\n", snd_khz->current.integer);
        hertz = 22050;
    }

    Com_Printf(
        CON_CHANNEL_SOUND,
        "Attempting %i kHz %i bit [%s] sound\n",
        hertz / 1000,
        16,
        snd_outputConfigurationStrings[snd_outputConfiguration->current.integer]);

    alGlob.device = alcOpenDevice(NULL);
    if (!alGlob.device)
    {
        Com_PrintError(CON_CHANNEL_SOUND, "ERROR: Couldn't open OpenAL device\n");
        return 0;
    }

    ALCint attrs[] = { ALC_FREQUENCY, hertz, 0 };
    alGlob.context = alcCreateContext(alGlob.device, attrs);
    if (!alGlob.context || !alcMakeContextCurrent(alGlob.context))
    {
        Com_PrintError(CON_CHANNEL_SOUND, "ERROR: Couldn't create OpenAL context\n");
        if (alGlob.context)
            alcDestroyContext(alGlob.context);
        alcCloseDevice(alGlob.device);
        alGlob.device = NULL;
        alGlob.context = NULL;
        return 0;
    }

    // Distance falloff is computed entirely by SND_Attenuate's curve and baked directly
    // into AL_GAIN (see the Phase 4 playback functions); disable OpenAL's own automatic
    // distance attenuation so it isn't applied a second time. Mirrors Miles'
    // AIL_set_3D_rolloff_factor(driver, 0.0f) in the original MSS_Init.
    alDistanceModel(AL_NONE);

    // Miles' listener never moves either - SND_Set3DPosition (snd_driver.cpp) always
    // pre-transforms each source's world position into listener-space before handing it
    // to the driver. Fix the AL listener at the origin with identity orientation once,
    // and never touch it again; this is also why split-screen's 2 listeners already work
    // today despite only one physical AL listener existing.
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    ALfloat orientation[6] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
    alListenerfv(AL_ORIENTATION, orientation);

    // One global reverb bus, matching Miles' own model (a single active room type + a
    // per-sample dry/wet send level - see SND_SetRoomtype/SND_ApplyReverbSend in
    // snd_driver.cpp, Phase 6). The effect's actual EAXREVERB properties get filled in by
    // SND_SetRoomtype whenever the room changes; just allocate the objects here.
    alGenAuxiliaryEffectSlots(1, &alGlob.auxSlot);
    alGenEffects(1, &alGlob.reverbEffect);
    alEffecti(alGlob.reverbEffect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);

    g_snd.Initialized2d = 1;
    g_snd.Initialized3d = 1;
    g_snd.max_2D_channels = 8;
    g_snd.max_3D_channels = 32;
    g_snd.max_stream_channels = 13;
    g_snd.playback_rate = hertz + hertz / 2;
    if (g_snd.playback_rate >= 0xAC44)
        g_snd.playback_rate = 0x7FFFFFFF;
    // KISAK_SOUND TODO (Phase 4): derive from snd_outputConfiguration once speaker-config
    // mapping is ported; Miles derives this from its opened mss_spec[] channel count.
    g_snd.playback_channels = 2;
    g_snd.timescale = 1.0;
    return 1;
}

// Unlike Miles (which splits 40 pre-allocated 2D/3D "sample" handles from lazily-opened
// stream handles), OpenAL sources are uniform - allocate all 53 up front.
void MSS_InitChannels()
{
    int totalChannels = g_snd.max_2D_channels + g_snd.max_3D_channels + g_snd.max_stream_channels;
    alGenSources(totalChannels, alGlob.source);
    for (int i = 0; i < totalChannels; ++i)
    {
        if (!alGlob.source[i])
            Com_Error(ERR_DROP, "OpenAL source allocation failed on channel %i", i + 1);
    }

    // Per-channel wet-send gain carrier (see AlLocal::sendFilter's comment in snd_local.h).
    // Allocated once here, params updated in place per-channel by SND_ApplyReverbSend.
    alGenFilters(totalChannels, alGlob.sendFilter);
    for (int i = 0; i < totalChannels; ++i)
        alFilteri(alGlob.sendFilter[i], AL_FILTER_TYPE, AL_FILTER_LOWPASS);

    g_snd.ambient_track = SND_TRACK_AMBIENT_PRIMARY_0;
}

// Mirrors MSS_InitEq. Only the shared EQ *data* (alGlob.eq[][]) gets initialized here -
// see MSS_ApplyEqFilter below for why alGlob.eqFilter is deliberately left at 0/unallocated
// rather than an EFX filter object (Phase 7: no working EQ application on this side).
void MSS_InitEq()
{
    alGlob.eqFilter = 0;
#ifndef KISAK_XBOX
    alGlob.eqLerp = 1.0f;
#endif

    for (int eqIndex = 0; eqIndex < 2; ++eqIndex)
    {
        for (int band = 0; band < 3; ++band)
        {
            for (int channelIndex = 0; channelIndex < 64; ++channelIndex)
            {
                SndEqParams *params = &alGlob.eq[eqIndex].params[band][channelIndex];
                params->enabled = 0;
                params->freq = 20000.0f;
                params->gain = 1.0f;
                params->q = 1.0f;
                params->type = SND_EQTYPE_FIRST;
            }
        }
    }
}

// Mirrors MSS_Startup (AIL_startup). Miles' AIL_startup is a global one-time SDK init with
// no device involved yet; OpenAL has no equivalent 2-stage init, so do an equivalent
// open-then-close sanity probe here instead - the real device open happens in MSS_Init.
bool __cdecl MSS_Startup()
{
    ALCdevice *probe = alcOpenDevice(NULL);
    if (!probe)
        return false;
    alcCloseDevice(probe);
    return true;
}

// Mirrors MSS_ShutdownCleanup. Also frees the sources/filters MSS_InitChannels allocated
// and the reverb objects MSS_Init allocated - a gap left over from Phase 2/6 that had gone
// unnoticed since nothing previously exercised a full init-then-shutdown cycle.
void MSS_ShutdownCleanup()
{
    if (alGlob.context)
    {
        int totalChannels = g_snd.max_2D_channels + g_snd.max_3D_channels + g_snd.max_stream_channels;
        if (totalChannels > 0)
        {
            alDeleteSources(totalChannels, alGlob.source);
            alDeleteFilters(totalChannels, alGlob.sendFilter);
        }
        alDeleteAuxiliaryEffectSlots(1, &alGlob.auxSlot);
        alDeleteEffects(1, &alGlob.reverbEffect);

        alcMakeContextCurrent(NULL);
        alcDestroyContext(alGlob.context);
    }
    if (alGlob.device)
        alcCloseDevice(alGlob.device);
    memset(&alGlob, 0, sizeof(alGlob));
}

float MSS_GetDryLevel()
{
    return 1.0f;
    //return g_snd.effect->drylevel;
}

float MSS_GetWetLevel(const snd_alias_t *pAlias)
{
    iassert(g_snd.effect->wetlevel >= 0 && g_snd.effect->wetlevel <= 1);

    if (!pAlias)
        return g_snd.effect->wetlevel;

    if (!snd_enableReverb->current.enabled || (pAlias->flags & 0x10) != 0)
        return 0.0f;
    else
        return g_snd.effect->wetlevel;
}

// Mirrors MSS_ApplyEqFilter. Left as a genuine no-op: Miles does true 3-band parametric EQ
// per entchannel (up to 2 EQ slots x 3 bands each, arbitrary type/freq/gain/Q per band -
// see the Miles branch in snd_mss.cpp), and there's no 1:1 equivalent in OpenAL-Soft's EFX,
// which only exposes single-band lowpass/highpass/bandpass filters. Real parity would need
// custom biquad DSP via the AL_SOFT_callback_buffer extension (alBufferCallbackSOFT) -
// genuine DSP work, budgeted separately if ever needed (see WORK.md Phase 7). Not a
// regression from what players get today either way: snd_enableEq already defaults off in
// the existing Miles code due to a documented crash bug (snd.cpp, SND_Init).
//
// Sketch of a crude single-band approximation, if this is ever revisited - uses only band 0
// of EQ slot 0 (ignoring slot 1 entirely, which only matters for the Xbox-only crossfade
// path anyway, and bands 1/2), and only handles the two band types EFX has a filter for.
// LOWSHELF/HIGHSHELF/BELL have no EFX equivalent at all. This is illustrative only - not
// wired up, not tested, and alGlob.eqFilter is deliberately left unallocated (0) above in
// MSS_InitEq since nothing here uses it while this stays commented out.
/*
void __cdecl MSS_ApplyEqFilter(ALuint source, int entchannel)
{
    if (!snd_enableEq->current.enabled)
    {
        alSourcei(source, AL_DIRECT_FILTER, AL_FILTER_NULL);
        return;
    }

    SndEqParams *params = &alGlob.eq[0].params[0][entchannel];
    if (!params->enabled)
    {
        alSourcei(source, AL_DIRECT_FILTER, AL_FILTER_NULL);
        return;
    }

    switch (params->type)
    {
    case SND_EQTYPE_LOWPASS:
        alFilteri(alGlob.eqFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
        alFilterf(alGlob.eqFilter, AL_LOWPASS_GAIN, params->gain);
        alFilterf(alGlob.eqFilter, AL_LOWPASS_GAINHF, 1.0f - (params->freq / 20000.0f));
        alSourcei(source, AL_DIRECT_FILTER, alGlob.eqFilter);
        break;
    case SND_EQTYPE_HIGHPASS:
        alFilteri(alGlob.eqFilter, AL_FILTER_TYPE, AL_FILTER_HIGHPASS);
        alFilterf(alGlob.eqFilter, AL_HIGHPASS_GAIN, params->gain);
        alFilterf(alGlob.eqFilter, AL_HIGHPASS_GAINLF, params->freq / 20000.0f);
        alSourcei(source, AL_DIRECT_FILTER, alGlob.eqFilter);
        break;
    default:
        // SND_EQTYPE_LOWSHELF/HIGHSHELF/BELL: no EFX filter type covers these.
        alSourcei(source, AL_DIRECT_FILTER, AL_FILTER_NULL);
        break;
    }
}
*/
void __cdecl MSS_ApplyEqFilter(ALuint source, int entchannel)
{
}

// Mirrors MSS_ResumeSample.
void __cdecl MSS_ResumeSample(int i, int frametime)
{
    if (g_snd.chaninfo[i].startDelay)
    {
        int remaining = g_snd.chaninfo[i].startDelay - frametime;
        g_snd.chaninfo[i].startDelay = (remaining > 0) ? remaining : 0;
        if (!g_snd.chaninfo[i].startDelay)
        {
            alSourcePlay(alGlob.source[i]);
        }
    }
}

int __cdecl MSS_DigitalFormatType(int waveFormat, int bits, int channels)
{
    int digitalFormat;

    if (waveFormat != 1 && waveFormat != 17)
        Com_Error(ERR_FATAL, "unknown wave format %i", waveFormat);
    if (channels != 1 && channels != 2)
        Com_Error(ERR_FATAL, "Sound has %i channels; only 1 or 2 channels are supported.\n", channels);
    if (bits != 8 && bits != 16)
        Com_Error(ERR_FATAL, "Sound uses %i bits per channel; only 8 or 16 bit channels are supported.\n", bits);
    digitalFormat = 0;
    if (waveFormat == 17)
        digitalFormat = 4;
    if (bits == 16)
        digitalFormat |= 1u;
    if (channels == 2)
        return digitalFormat | 2;
    return digitalFormat;
}

uint8_t *__cdecl MSS_Alloc(uint32_t bytes, uint32_t rate)
{
    if (IsFastFileLoad())
        return (uint8_t *)((int(__cdecl *)(uint32_t, uint32_t))MSS_Alloc_FastFile)(bytes, rate);
    else
        return MSS_Alloc_LoadObj(bytes, rate);
}

uint8_t *__cdecl MSS_Alloc_LoadObj(uint32_t bytes, uint32_t rate)
{
    int min_Spec_bytes = bytes;
    while (rate > 0x4099)
    {
        rate >>= 1;
        min_Spec_bytes /= 2;
    }
    return Hunk_Alloc(bytes, "MSS_Alloc", 15);
}

uint32_t *__cdecl MSS_Alloc_FastFile(int bytes)
{
    return (uint32_t *)Z_Malloc(bytes, "MSS_Alloc", 15);
}

#endif