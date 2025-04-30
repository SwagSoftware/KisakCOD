#include "snd_local.h"
#include "snd_public.h"
#include <universal/com_files.h>
#include <universal/com_memory.h>

LoadedSound *__cdecl SND_LoadFromBuffer(void *buffer, const char *soundName)
{
    _AILSOUNDINFO info; // [esp+8h] [ebp-28h] BYREF
    LoadedSound *loadSnd; // [esp+2Ch] [ebp-4h]

    if (!buffer)
        MyAssertHandler(".\\win32\\snd_driver_load_obj.cpp", 134, 0, "%s", "buffer");
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

LoadedSound *__cdecl SND_LoadSoundFile(const char *name)
{
    void *buffer; // [esp+4h] [ebp-10Ch] BYREF
    char realname[256]; // [esp+8h] [ebp-108h] BYREF
    LoadedSound *loadSnd; // [esp+10Ch] [ebp-4h]

    if (useFastFile->current.enabled)
        MyAssertHandler(".\\win32\\snd_driver_load_obj.cpp", 175, 0, "%s", "IsObjFileLoad()");
    if (!name)
        MyAssertHandler(".\\win32\\snd_driver_load_obj.cpp", 176, 0, "%s", "name");
    Com_sprintf(realname, 0x100u, "sound/%s", name);
    if (FS_ReadFile(realname, &buffer) >= 0)
    {
        loadSnd = SND_LoadFromBuffer(buffer, name);
        FS_FreeFile((char*)buffer);
        return loadSnd;
    }
    else
    {
        Com_PrintError(1, "ERROR: Sound file '%s' not found\n", realname);
        return 0;
    }
}