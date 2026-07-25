#include <universal/q_shared.h>
#include "snd_local.h"
#include "snd_public.h"
#include <universal/com_files.h>
#include <universal/com_memory.h>

#ifdef KISAK_SOUND
// Single-header decoders (public domain / MIT-0), vendored in deps/dr_libs. This is the
// one translation unit that generates their implementations. dr_mp3's implementation is
// compiled in now so it's ready, but nothing calls it yet - "loaded" sounds are always WAV
// per the original Miles code's own MSS_DigitalFormatType format check (snd_al.cpp); dr_mp3
// is reserved for the streaming path (Phase 5) in case any streamed content is actually MP3.
#define DR_WAV_IMPLEMENTATION
#include <dr_libs/dr_wav.h>
#define DR_MP3_IMPLEMENTATION
#include <dr_libs/dr_mp3.h>
#endif

#ifndef KISAK_SOUND
LoadedSound *__cdecl SND_LoadFromBuffer(void *buffer, const char *soundName)
{
    _AILSOUNDINFO info; // [esp+8h] [ebp-28h] BYREF
    LoadedSound *loadSnd; // [esp+2Ch] [ebp-4h]

    iassert(buffer);

    if (AIL_WAV_info(buffer, &info))
    {
        if (info.data_len)
        {
            loadSnd = (LoadedSound*)Hunk_Alloc(0x2Cu, "SND_LoadFromBuffer", 15);
            loadSnd->name = soundName;
            qmemcpy(&loadSnd->sound, &info, 0x24u);
            SND_SetData(&loadSnd->sound, (void*)info.data_ptr);
            return loadSnd;
        }
        else
        {
            Com_PrintError(1, "ERROR: Sound file '%s' is zero length, invalid\n", soundName);
            return 0;
        }
    }
    else
    {
        Com_PrintError(1, "ERROR: Sound file '%s' is in an invalid or corrupted format\n", soundName);
        return 0;
    }
}
#else
// bufferSize is new vs. the Miles branch above: AIL_WAV_info trusts the RIFF header's own
// declared chunk sizes and never needed the total buffer length, but drwav_init_memory
// bounds-checks against it for safety. SND_LoadFromBuffer has no callers outside this file
// (see SND_LoadSoundFile below), so extending its signature is safe.
LoadedSound *__cdecl SND_LoadFromBuffer(void *buffer, uint32_t bufferSize, const char *soundName)
{
    iassert(buffer);

    drwav wav;
    if (!drwav_init_memory(&wav, buffer, bufferSize, NULL))
    {
        Com_PrintError(1, "ERROR: Sound file '%s' is in an invalid or corrupted format\n", soundName);
        return 0;
    }

    if (wav.totalPCMFrameCount == 0)
    {
        drwav_uninit(&wav);
        Com_PrintError(1, "ERROR: Sound file '%s' is zero length, invalid\n", soundName);
        return 0;
    }

    // MSS_DigitalFormatType (snd_al.cpp) only ever recognized 1 (PCM) or 17 (IMA-ADPCM)
    // source formats; dr_wav decodes both of those (and everything else it supports) to
    // plain PCM for us, so we always end up with 16-bit PCM regardless of the source
    // format/bit depth - no ADPCM-vs-PCM branching needed on this side at all.
    uint32_t channels = wav.channels;
    uint32_t sampleRate = wav.sampleRate;
    uint32_t frameCount = (uint32_t)wav.totalPCMFrameCount;
    uint32_t dataLen = frameCount * channels * sizeof(drwav_int16);

    drwav_int16 *pcm = (drwav_int16 *)Z_Malloc(dataLen, "SND_LoadFromBuffer_temp", 15);
    drwav_uint64 framesRead = drwav_read_pcm_frames_s16(&wav, frameCount, pcm);
    drwav_uninit(&wav);

    LoadedSound *loadSnd = (LoadedSound *)Hunk_Alloc(0x2Cu, "SND_LoadFromBuffer", 15);
    loadSnd->name = soundName;
    loadSnd->sound.info.format = 1; // PCM
    loadSnd->sound.info.data_ptr = NULL; // filled in by SND_SetData below
    loadSnd->sound.info.data_len = (uint32_t)(framesRead * channels * sizeof(drwav_int16));
    loadSnd->sound.info.rate = sampleRate;
    loadSnd->sound.info.bits = 16;
    loadSnd->sound.info.channels = channels;
    loadSnd->sound.info.samples = (uint32_t)framesRead;
    loadSnd->sound.info.block_size = channels * sizeof(drwav_int16);
    loadSnd->sound.info.initial_ptr = NULL;

    SND_SetData(&loadSnd->sound, pcm);
    Z_Free(pcm, 15);

    return loadSnd;
}
#endif

LoadedSound *__cdecl SND_LoadSoundFile(const char *name)
{
    void *buffer; // [esp+4h] [ebp-10Ch] BYREF
    char realname[256]; // [esp+8h] [ebp-108h] BYREF
    LoadedSound *loadSnd; // [esp+10Ch] [ebp-4h]

    if (IsFastFileLoad())
        MyAssertHandler(".\\win32\\snd_driver_load_obj.cpp", 175, 0, "%s", "IsObjFileLoad()");

    iassert(name);

    Com_sprintf(realname, 0x100u, "sound/%s", name);
    int fileLen = FS_ReadFile(realname, &buffer);
    if (fileLen >= 0)
    {
#ifndef KISAK_SOUND
        loadSnd = SND_LoadFromBuffer(buffer, name);
#else
        loadSnd = SND_LoadFromBuffer(buffer, (uint32_t)fileLen, name);
#endif
        FS_FreeFile((char*)buffer);
        return loadSnd;
    }
    else
    {
        Com_PrintError(1, "ERROR: Sound file '%s' not found\n", realname);
        return 0;
    }
}
