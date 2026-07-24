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

AlLocal alGlob;

void MSS_InitFailed()
{
    if (Dvar_GetInt("r_vc_compile") != 2)
        Com_Printf(9, "OpenAL sound system initialization failed\n");
}

// KISAK_SOUND TODO (Phase 2 - driver init): open the ALC device/context, size g_snd's
// channel counts/playback rate, and allocate alGlob.source[53]. Mirrors MSS_Init +
// MSS_open_digital_driver in snd_mss.cpp.
char __cdecl MSS_Init()
{
    Com_PrintError(9, "ERROR: OpenAL sound system not yet implemented (KISAK_SOUND).\n");
    return 0;
}

// KISAK_SOUND TODO (Phase 2): alGenSources for all 53 channels. Mirrors MSS_InitChannels.
void MSS_InitChannels()
{
}

// KISAK_SOUND TODO (Phase 7 - EQ): set up the EFX filter/aux-slot state. Mirrors MSS_InitEq.
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

// KISAK_SOUND TODO (Phase 2): alcOpenDevice/alcCreateContext. Mirrors MSS_Startup.
bool __cdecl MSS_Startup()
{
    return false;
}

// KISAK_SOUND TODO (Phase 2): alcDestroyContext/alcCloseDevice. Mirrors MSS_ShutdownCleanup.
void MSS_ShutdownCleanup()
{
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

// KISAK_SOUND TODO (Phase 7 - EQ): apply alGlob.eq[][entchannel] via an EFX filter on the
// given source. Mirrors MSS_ApplyEqFilter.
void __cdecl MSS_ApplyEqFilter(ALuint source, int entchannel)
{
}

// KISAK_SOUND TODO (Phase 4 - playback): resume the source once its start delay elapses.
// Mirrors MSS_ResumeSample.
void __cdecl MSS_ResumeSample(int i, int frametime)
{
    if (g_snd.chaninfo[i].startDelay)
    {
        int remaining = g_snd.chaninfo[i].startDelay - frametime;
        g_snd.chaninfo[i].startDelay = (remaining > 0) ? remaining : 0;
        if (!g_snd.chaninfo[i].startDelay)
        {
            // KISAK_SOUND TODO: alSourcePlay(alGlob.source[i]);
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
