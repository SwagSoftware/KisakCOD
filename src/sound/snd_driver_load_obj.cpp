#include "snd_local.h"
#include "snd_public.h"
#include <universal/com_files.h>
#include <universal/com_memory.h>

LoadedSound *__cdecl SND_LoadFromBuffer(void *buffer, int bufSize, const char *soundName)
{
    drmp3 wav;
    LoadedSound *loadSnd;

    iassert(buffer);

    if (!drmp3_init_memory(&wav, buffer, (size_t)bufSize, NULL))
    {
        Com_PrintError(1, "ERROR: Sound file '%s' is in an invalid or corrupted format\n", soundName);
        return NULL;
    }

    if (wav.totalPCMFrameCount == 0)
    {
        drmp3_uninit(&wav);
        Com_PrintError(1, "ERROR: Sound file '%s' is zero length, invalid\n", soundName);
        return NULL;
    }

    int dataLen = (int)(wav.totalPCMFrameCount * wav.channels * sizeof(int16_t));

    loadSnd = (LoadedSound *)Hunk_Alloc(0x2Cu, "SND_LoadFromBuffer", 15);
    loadSnd->name = soundName;
    loadSnd->sound.info.format     = 1;  // PCM
    loadSnd->sound.info.rate       = (int)wav.sampleRate;
    loadSnd->sound.info.bits       = 16; // always s16 after decode
    loadSnd->sound.info.channels   = (int)wav.channels;
    loadSnd->sound.info.samples    = (int)wav.totalPCMFrameCount;
    loadSnd->sound.info.data_len   = dataLen;
    loadSnd->sound.info.block_size = (int)(wav.channels * sizeof(int16_t));

    loadSnd->sound.data = (uint8_t *)Z_Malloc(dataLen, "SND_LoadFromBuffer", 15);
    drmp3_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, (int16_t *)loadSnd->sound.data);
    loadSnd->sound.info.data_ptr    = loadSnd->sound.data;
    loadSnd->sound.info.initial_ptr = loadSnd->sound.data;

    drmp3_uninit(&wav);
    return loadSnd;
}

LoadedSound *__cdecl SND_LoadSoundFile(const char *name)
{
    void *buffer;
    char realname[256];
    LoadedSound *loadSnd;
    int fileSize;

    if (IsFastFileLoad())
        MyAssertHandler(".\\win32\\snd_driver_load_obj.cpp", 175, 0, "%s", "IsObjFileLoad()");

    iassert(name);

    Com_sprintf(realname, 0x100u, "sound/%s", name);
    fileSize = FS_ReadFile(realname, &buffer);
    if (fileSize >= 0)
    {
        loadSnd = SND_LoadFromBuffer(buffer, fileSize, name);
        FS_FreeFile((char *)buffer);
        return loadSnd;
    }
    else
    {
        Com_PrintError(1, "ERROR: Sound file '%s' not found\n", realname);
        return NULL;
    }
}