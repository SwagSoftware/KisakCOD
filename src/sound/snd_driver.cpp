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

MssLocal milesGlob;

const dvar_t *snd_khz;
const dvar_t *snd_outputConfiguration;

void __cdecl TRACK_snd_driver()
{
    track_static_alloc_internal(&milesGlob, 9936, "milesGlob", 13);
}

bool __cdecl SND_IsMultiChannel()
{
    return milesGlob.isMultiChannel;
}

char __cdecl SND_InitDriver()
{
    snd_khz = Dvar_RegisterInt("snd_khz", 44, (DvarLimits)0x2C0000000BLL, DVAR_ARCHIVE | DVAR_LATCH, "The game sound frequency.");
    AIL_set_file_callbacks(MSS_FileOpenCallback, MSS_FileCloseCallback, MSS_FileSeekCallback, MSS_FileReadCallback);
    AIL_set_redist_directory("miles");
    snd_outputConfiguration = Dvar_RegisterEnum(
        "snd_outputConfiguration",
        snd_outputConfigurationStrings,
        0,
        DVAR_ARCHIVE | DVAR_LATCH,
        "Sound output configuration");
    if (MSS_Startup())
    {
        MSS_open_digital_driver(11025, 2, 2);
        AIL_shutdown();
    }
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
            AIL_shutdown();
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
}

void __cdecl SND_ShutdownDriver()
{
    R_Cinematic_StopPlayback();
    R_Cinematic_SyncNow();
    AIL_shutdown();
    MSS_ShutdownCleanup();
}

int __cdecl SND_GetDriverCPUPercentage()
{
    return AIL_digital_CPU_percent(milesGlob.driver);
}

void __cdecl SND_Set3DPosition(int index, const float *org)
{
    float v2; // [esp+0h] [ebp-28h]
    float delta[3]; // [esp+Ch] [ebp-1Ch] BYREF
    int listenerIndex; // [esp+18h] [ebp-10h]
    float transformed[3]; // [esp+1Ch] [ebp-Ch] BYREF

    if (index < 8 || index >= g_snd.max_3D_channels + 8)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            602,
            0,
            "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)",
            index);
    listenerIndex = SND_GetListenerIndexNearestToOrigin(org);
    Vec3Sub(org, g_snd.listeners[listenerIndex].orient.origin, delta);
    MatrixTransposeTransformVector(delta, g_snd.listeners[listenerIndex].orient.axis, transformed);
    v2 = -transformed[1];
    AIL_set_sample_3D_position(
        milesGlob.handle_sample[index],
        v2,
        transformed[2],
        transformed[0]);
}

void __cdecl SND_Stop2DChannel(int index)
{
    if (index < 0 || index >= g_snd.max_2D_channels)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            642,
            0,
            "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)",
            index);
    AIL_end_sample(milesGlob.handle_sample[index]);
    SND_ResetChannelInfo(index);
    SND_RemoveVoice(g_snd.chaninfo[index].entchannel);
}

void __cdecl SND_Pause2DChannel(int index)
{
    if (index < 0 || index >= g_snd.max_2D_channels)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            653,
            0,
            "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)",
            index);
    AIL_stop_sample(milesGlob.handle_sample[index]);
    g_snd.chaninfo[index].paused = 1;
}

void __cdecl SND_Unpause2DChannel(int index, int timeshift)
{
    if (index < 0 || index >= g_snd.max_2D_channels)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            662,
            0,
            "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)",
            index);
    if (!g_snd.chaninfo[index].startDelay)
        AIL_resume_sample(milesGlob.handle_sample[index]);
    g_snd.chaninfo[index].soundFileInfo.endtime += timeshift;
    g_snd.chaninfo[index].startTime += timeshift;
    g_snd.chaninfo[index].paused = 0;
}

bool __cdecl SND_Is2DChannelFree(int index)
{
    if (index < 0 || index >= g_snd.max_2D_channels)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            674,
            0,
            "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)",
            index);
    return !g_snd.chaninfo[index].paused && !g_snd.chaninfo[index].startDelay && g_snd.chaninfo[index].alias0 == 0;
}

void __cdecl SND_Stop3DChannel(int index)
{
    if (index < 8 || index >= g_snd.max_3D_channels + 8)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            683,
            0,
            "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)",
            index);
    AIL_end_sample(milesGlob.handle_sample[index]);
    SND_ResetChannelInfo(index);
    SND_RemoveVoice(g_snd.chaninfo[index].entchannel);
}

void __cdecl SND_Pause3DChannel(int index)
{
    if (index < 8 || index >= g_snd.max_3D_channels + 8)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            694,
            0,
            "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)",
            index);
    AIL_stop_sample(milesGlob.handle_sample[index]);
    g_snd.chaninfo[index].paused = 1;
}

void __cdecl SND_Unpause3DChannel(int index, int timeshift)
{
    if (index < 8 || index >= g_snd.max_3D_channels + 8)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            703,
            0,
            "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)",
            index);
    if (!g_snd.chaninfo[index].startDelay)
        AIL_resume_sample(milesGlob.handle_sample[index]);
    g_snd.chaninfo[index].soundFileInfo.endtime += timeshift;
    g_snd.chaninfo[index].startTime += timeshift;
    g_snd.chaninfo[index].paused = 0;
}

bool __cdecl SND_Is3DChannelFree(int index)
{
    if (index < 8 || index >= g_snd.max_3D_channels + 8)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            715,
            0,
            "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)",
            index);
    return !g_snd.chaninfo[index].paused && !g_snd.chaninfo[index].startDelay && g_snd.chaninfo[index].alias0 == 0;
}

void __cdecl SND_StopStreamChannel(int index)
{
    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            724,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
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
    SND_ResetChannelInfo(index);
    SND_RemoveVoice(g_snd.chaninfo[index].entchannel);
}

void __cdecl SND_PauseStreamChannel(int index)
{
    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            737,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
    AIL_pause_stream((HSTREAM)milesGlob.handle_sample[index], 1);
    g_snd.chaninfo[index].paused = 1;
}

void __cdecl SND_UnpauseStreamChannel(int index, int timeshift)
{
    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            745,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
    if (!g_snd.chaninfo[index].startDelay)
        AIL_pause_stream((HSTREAM)milesGlob.handle_sample[index], 0);
    g_snd.chaninfo[index].soundFileInfo.endtime += timeshift;
    g_snd.chaninfo[index].startTime += timeshift;
    g_snd.chaninfo[index].paused = 0;
}

bool __cdecl SND_IsStreamChannelFree(int index)
{
    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            756,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
    if (!milesGlob.handle_sample[index])
        return 1;
    if (g_snd.chaninfo[index].paused || g_snd.chaninfo[index].startDelay)
        return 0;
    return g_snd.chaninfo[index].alias0 == 0;
}

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

    if (!handle)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 771, 0, "%s", "handle");
    if (!alias)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 772, 0, "%s", "alias");
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

int __cdecl SND_StartAlias2DSample(SndStartAliasInfo *startAliasInfo, int *pChannel)
{
    float v3; // [esp+0h] [ebp-B0h]
    float baseSlavePercentage; // [esp+4h] [ebp-ACh]
    double timescale; // [esp+10h] [ebp-A0h]
    float v6; // [esp+1Ch] [ebp-94h]
    float v7; // [esp+2Ch] [ebp-84h]
    float v8; // [esp+3Ch] [ebp-74h]
    float v9; // [esp+4Ch] [ebp-64h]
    _SAMPLE *handle; // [esp+90h] [ebp-20h]
    int total_msec; // [esp+94h] [ebp-1Ch] BYREF
    int start_msec; // [esp+98h] [ebp-18h]
    int playbackId; // [esp+9Ch] [ebp-14h]
    float realVolume; // [esp+A0h] [ebp-10h]
    MssSoundCOD4 *sound; // [esp+A4h] [ebp-Ch]
    int entchannel; // [esp+A8h] [ebp-8h]
    int index; // [esp+ACh] [ebp-4h]

    if (!startAliasInfo->alias0)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 835, 0, "%s", "startAliasInfo->alias0");
    if ((startAliasInfo->alias0->flags & 0xC0) >> 6 != 1)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            836,
            0,
            "%s",
            "SNDALIASFLAGS_GET_TYPE( startAliasInfo->alias0->flags ) == SAT_LOADED");
    if (!startAliasInfo->alias0->soundFile)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 837, 0, "%s", "startAliasInfo->alias0->soundFile");
    if (startAliasInfo->alias0->soundFile->type != 1)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 838, 0, "%s", "startAliasInfo->alias0->soundFile->type == SAT_LOADED");
    if (!startAliasInfo->alias0->soundFile->u.loadSnd)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 839, 0, "%s", "startAliasInfo->alias0->soundFile->u.loadSnd");
    if (!startAliasInfo->alias0->soundFile->exists)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 840, 0, "%s", "startAliasInfo->alias0->soundFile->exists");
    if (!startAliasInfo->alias1)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 841, 0, "%s", "startAliasInfo->alias1");
    if ((startAliasInfo->alias1->flags & 0xC0) >> 6 != 1)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            842,
            0,
            "%s",
            "SNDALIASFLAGS_GET_TYPE( startAliasInfo->alias1->flags ) == SAT_LOADED");
    if (!startAliasInfo->alias1->soundFile)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 843, 0, "%s", "startAliasInfo->alias1->soundFile");
    if (startAliasInfo->alias1->soundFile->type != 1)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 844, 0, "%s", "startAliasInfo->alias1->soundFile->type == SAT_LOADED");
    if (!startAliasInfo->alias1->soundFile->u.loadSnd)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 845, 0, "%s", "startAliasInfo->alias1->soundFile->u.loadSnd");
    if (!startAliasInfo->alias1->soundFile->exists)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 846, 0, "%s", "startAliasInfo->alias1->soundFile->exists");
    entchannel = (startAliasInfo->alias0->flags & 0x3F00) >> 8;
    if (!SND_HasFreeVoice(entchannel))
        return -1;
    index = SND_FindFree2DChannel(startAliasInfo, entchannel);
    if (pChannel)
        *pChannel = index;
    if (index < 0)
        return -1;
    if (index >= g_snd.max_2D_channels)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            858,
            0,
            "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)",
            index);
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
        AIL_set_sample_playback_rate(handle, (int)((float)AIL_sample_playback_rate(handle) * startAliasInfo->pitch * timescale));
    }
    else
    {
        AIL_set_sample_playback_rate(handle, (int)((float)AIL_sample_playback_rate(handle) * startAliasInfo->pitch));
    }
    realVolume = startAliasInfo->volume
        * g_snd.volume
        * g_snd.channelvol->channelvol[(startAliasInfo->alias0->flags & 0x3F00) >> 8].volume;
    if (g_snd.slaveLerp != 0.0 && !startAliasInfo->master && (startAliasInfo->alias0->flags & 4) != 0)
        realVolume = SND_GetLerpedSlavePercentage(startAliasInfo->alias0->slavePercentage) * realVolume;
    SND_ApplyChannelMap(handle, startAliasInfo->alias0, sound->info.channels);
    SND_Set2DChannelVolume(index, realVolume);
    AIL_set_sample_loop_count(handle, (startAliasInfo->alias0->flags & 1) == 0);
    baseSlavePercentage = MSS_GetWetLevel(startAliasInfo->alias0);
    v3 = CG_BannerScoreboardScaleMultiplier();
    AIL_set_sample_reverb_levels(handle, v3, baseSlavePercentage);
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
            start_msec = (int)(random() * (float)total_msec) & 0xFFFFFF80;
        }
        else
        {
            start_msec = 0;
        }
    }
    else
    {
        start_msec = (int)((float)total_msec * startAliasInfo->fraction);
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

    if (!handle)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 797, 0, "%s", "handle");
    if (!alias)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 798, 0, "%s", "alias");
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

int __cdecl SND_StartAlias3DSample(SndStartAliasInfo *startAliasInfo, int *pChannel)
{
    double LerpedSlavePercentage; // st7
    float mindist; // [esp+4h] [ebp-108h]
    float maxdist; // [esp+8h] [ebp-104h]
    double timescale; // [esp+24h] [ebp-E8h]
    float v7; // [esp+30h] [ebp-DCh]
    float v8; // [esp+44h] [ebp-C8h]
    float v9; // [esp+54h] [ebp-B8h]
    float v10; // [esp+64h] [ebp-A8h]
    float v11; // [esp+74h] [ebp-98h]
    float v12; // [esp+84h] [ebp-88h]
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

    if (!startAliasInfo->alias0)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 942, 0, "%s", "startAliasInfo->alias0");
    if ((startAliasInfo->alias0->flags & 0xC0) >> 6 != 1)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            943,
            0,
            "%s",
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
    //((void(__stdcall *)(unsigned int, unsigned int, unsigned int, unsigned int))AIL_set_sample_3D_distances)(
    //    handle,
    //    startAliasInfo->alias0->distMax,
    //    startAliasInfo->alias0->distMin,
    //    1);
    AIL_set_sample_3D_distances(handle, startAliasInfo->alias0->distMax, startAliasInfo->alias0->distMin, 1);
    if (startAliasInfo->timescale)
    {
        timescale = g_snd.timescale;
        rate = (int)((float)AIL_sample_playback_rate(handle) * startAliasInfo->pitch * timescale);
    }
    else
    {
        rate = (int)((float)AIL_sample_playback_rate(handle) * startAliasInfo->pitch);
    }
    AIL_set_sample_playback_rate(handle, rate);
    SND_Set3DPosition(index, startAliasInfo->org);
    AIL_set_sample_loop_count(handle, (startAliasInfo->alias0->flags & 1) == 0);
    maxdist = MSS_GetWetLevel(startAliasInfo->alias0);
    mindist = CG_BannerScoreboardScaleMultiplier();
    AIL_set_sample_reverb_levels(handle, mindist, maxdist);
    if (!rate)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1004, 0, "%s", "rate");
    if (startAliasInfo->timescale)
    {
        total_msec = (int)(g_snd.timescale * (float)(1000 * sound->info.samples) / (float)rate);
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
            start_msec = (int)(random() * (float)total_msec) & 0xFFFFFF80;
        }
        else
        {
            start_msec = 0;
        }
    }
    else
    {
        start_msec = (int)((float)total_msec * startAliasInfo->fraction);
    }
    if (start_msec)
        startAliasInfo->startDelay = 0;
    AIL_set_sample_ms_position(handle, (int)((float)start_msec / (float)total_msec * (float)sound->info.data_len));
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

void __cdecl SND_Set3DStreamPosition(int index, int listenerIndex, const float *org)
{
    float v3; // [esp+0h] [ebp-28h]
    float delta[3]; // [esp+Ch] [ebp-1Ch] BYREF
    _SAMPLE *handle_sample; // [esp+18h] [ebp-10h]
    float transformed[3]; // [esp+1Ch] [ebp-Ch] BYREF

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
    handle_sample = AIL_stream_sample_handle((HSTREAM)milesGlob.handle_sample[index]);
    v3 = -transformed[1];
    AIL_set_sample_3D_position(handle_sample, v3, transformed[2], transformed[0]);
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
    mindist = (1.0 - lerp) * alias0->distMin + alias1->distMin * lerp;
    maxdist = (1.0 - lerp) * alias0->distMax + alias1->distMax * lerp;
    return SND_Attenuate(alias0->volumeFalloffCurve, dist, mindist, maxdist);
}


int __cdecl SND_StartAliasStreamOnChannel(SndStartAliasInfo *startAliasInfo, int index)
{
    const char *error; // eax
    double LerpedSlavePercentage; // st7
    double Stream3DVolumeFallOff; // st7
    float v6; // [esp+4h] [ebp-244h]
    float baseSlavePercentage; // [esp+8h] [ebp-240h]
    float *org; // [esp+Ch] [ebp-23Ch]
    float v9; // [esp+18h] [ebp-230h]
    float v10; // [esp+28h] [ebp-220h]
    float v11; // [esp+3Ch] [ebp-20Ch]
    float v12; // [esp+50h] [ebp-1F8h]
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

    if (!startAliasInfo->alias0)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1060, 0, "%s", "startAliasInfo->alias0");
    if ((startAliasInfo->alias0->flags & 0xC0) >> 6 != 2)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1061,
            0,
            "%s",
            "SNDALIASFLAGS_GET_TYPE( startAliasInfo->alias0->flags ) == SAT_STREAMED");
    if (!startAliasInfo->alias1)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1062, 0, "%s", "startAliasInfo->alias1");
    if ((startAliasInfo->alias1->flags & 0xC0) >> 6 != 2)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1063,
            0,
            "%s",
            "SNDALIASFLAGS_GET_TYPE( startAliasInfo->alias1->flags ) == SAT_STREAMED");
    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1064,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
    if (!FS_Initialized())
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1067, 0, "%s", "FS_Initialized()");
    entchannel = (startAliasInfo->alias0->flags & 0x3F00) >> 8;
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
                AIL_set_sample_playback_rate(handle_sample, (int)(g_snd.timescale * ((float)baserate * startAliasInfo->pitch)));
            }
            else
            {
                AIL_set_sample_playback_rate(handle_sample, (int)((float)baserate * startAliasInfo->pitch));
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
            v6 = CG_BannerScoreboardScaleMultiplier();
            AIL_set_sample_reverb_levels(handle_sample, v6, baseSlavePercentage);
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
                            start_msec = (int)(random() * (float)total_msec[0]) & 0xFFFFFF80;
                        }
                        else
                        {
                            start_msec = 0;
                        }
                    }
                    else
                    {
                        start_msec = (int)((float)total_msec[0] * startAliasInfo->fraction);
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
                        //((void(__stdcall *)(unsigned int, unsigned int, unsigned int, unsigned int))AIL_set_sample_3D_distances)(
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

void __cdecl SND_SetRoomtype(int roomtype)
{
    float v1; // [esp+0h] [ebp-8h]
    float WetLevel; // [esp+4h] [ebp-4h]

    AIL_set_room_type(milesGlob.driver, roomtype);
    WetLevel = MSS_GetWetLevel(0);
    v1 = CG_BannerScoreboardScaleMultiplier();
    AIL_set_digital_master_reverb_levels(milesGlob.driver, v1, WetLevel);
}

void __cdecl SND_UpdateEqs()
{
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
}

void __cdecl SND_SetEqParams(
    unsigned int entchannel,
    int eqIndex,
    unsigned int band,
    SND_EQTYPE type,
    float gain,
    float freq,
    float q)
{
    if (entchannel >= 0x40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1247,
            0,
            "%s\n\t(entchannel) = %i",
            "(entchannel >= 0 && entchannel < 64)",
            entchannel);
    if (band > 2)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1248, 0, "%s\n\t(band) = %i", "(band >= 0 && band < 3)", band);
    if (freq < 0.0 || freq > 20000.0)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1249, 0, "%s\n\t(freq) = %g", "(freq >= 0 && freq <= 20000)", freq);
    if (q <= 0.0)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1250, 0, "%s\n\t(q) = %g", "(q > 0)", q);
    milesGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    milesGlob.eq[eqIndex].params[band][entchannel].gain = gain;
    milesGlob.eq[eqIndex].params[band][entchannel].freq = freq;
    milesGlob.eq[eqIndex].params[band][entchannel].q = q;
    milesGlob.eq[eqIndex].params[band][entchannel].type = type;
}

void __cdecl SND_SetEqType(unsigned int entchannel, int eqIndex, unsigned int band, SND_EQTYPE type)
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
    milesGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    milesGlob.eq[eqIndex].params[band][entchannel].type = type;
}

void __cdecl SND_SetEqFreq(unsigned int entchannel, int eqIndex, unsigned int band, float freq)
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
    milesGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    milesGlob.eq[eqIndex].params[band][entchannel].freq = freq;
}

void __cdecl SND_SetEqGain(unsigned int entchannel, int eqIndex, unsigned int band, float gain)
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
    milesGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    milesGlob.eq[eqIndex].params[band][entchannel].gain = gain;
}

void __cdecl SND_SetEqQ(unsigned int entchannel, int eqIndex, unsigned int band, float q)
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
    milesGlob.eq[eqIndex].params[band][entchannel].enabled = 1;
    milesGlob.eq[eqIndex].params[band][entchannel].q = q;
}

void __cdecl SND_DisableEq(unsigned int entchannel, int eqIndex, unsigned int band)
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
    milesGlob.eq[eqIndex].params[band][entchannel].enabled = 0;
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
                MemFile_WriteData(memFile, 20, &milesGlob.eq[eqIndex].params[band][entchannel]);
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
                MemFile_ReadData(memFile, 20, (unsigned __int8 *)&milesGlob.eq[eqIndex].params[band][entchannel]);
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
                v0 = (float *)&milesGlob.eq[eqIndex].params[band][entchannel];
                if ((unsigned __int8)*((unsigned int *)v0 + 4))
                    Com_Printf(9, "\t%i %s %f Hz %f dB %f q\n", band, snd_eqTypeStrings[*(unsigned int *)v0], v0[2], v0[1], v0[3]);
            }
        }
    }
}

double __cdecl SND_Get2DChannelVolume(int index)
{
    float right; // [esp+4h] [ebp-8h] BYREF
    float left; // [esp+8h] [ebp-4h] BYREF

    if (index < 0 || index >= g_snd.max_2D_channels)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1399,
            0,
            "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)",
            index);
    AIL_sample_volume_levels(milesGlob.handle_sample[index], &left, &right);
    if (g_snd.chaninfo[index].soundFileInfo.srcChannelCount == 2)
        return left;
    return (float)(left + right);
}

void __cdecl SND_Set2DChannelVolume(int index, float volume)
{
    if (index < 0 || index >= g_snd.max_2D_channels)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1412,
            0,
            "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)",
            index);
    AIL_set_sample_volume_levels(milesGlob.handle_sample[index], volume, volume);
}

double __cdecl SND_Get3DChannelVolume(int index)
{
    float right; // [esp+4h] [ebp-8h] BYREF
    float left; // [esp+8h] [ebp-4h] BYREF

    if (index < 8 || index >= g_snd.max_3D_channels + 8)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1423,
            0,
            "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)",
            index);
    AIL_sample_volume_levels(milesGlob.handle_sample[index], &left, &right);
    if (g_snd.chaninfo[index].soundFileInfo.srcChannelCount == 2)
        return left;
    return (float)(left + right);
}

void __cdecl SND_Set3DChannelVolume(int index, float volume)
{
    float v2; // [esp+Ch] [ebp-4h]

    if (index < 8 || index >= g_snd.max_3D_channels + 8)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1436,
            0,
            "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)",
            index);
    if (g_snd.chaninfo[index].soundFileInfo.srcChannelCount == 2)
    {
        AIL_set_sample_volume_levels(milesGlob.handle_sample[index], volume, volume);
    }
    else
    {
        v2 = volume * 0.5;
        AIL_set_sample_volume_levels(milesGlob.handle_sample[index], v2, v2);
    }
}

double __cdecl SND_GetStreamChannelVolume(int index)
{
    _SAMPLE *handle_sample; // [esp+4h] [ebp-Ch]
    float right; // [esp+8h] [ebp-8h] BYREF
    float left; // [esp+Ch] [ebp-4h] BYREF

    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1452,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
    handle_sample = (_SAMPLE *)AIL_stream_sample_handle((HSTREAM)milesGlob.handle_sample[index]);
    AIL_sample_volume_levels(handle_sample, &left, &right);
    if (g_snd.chaninfo[index].soundFileInfo.srcChannelCount == 2
        || !SND_IsAliasChannel3D((g_snd.chaninfo[index].alias0->flags & 0x3F00) >> 8))
    {
        return left;
    }
    return (float)(left + right);
}

void __cdecl SND_SetStreamChannelVolume(int index, float volume)
{
    float v2; // [esp+Ch] [ebp-8h]
    _SAMPLE *handle_sample; // [esp+10h] [ebp-4h]

    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1469,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
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
}

int __cdecl SND_Get2DChannelPlaybackRate(int index)
{
    if (index < 0 || index >= g_snd.max_2D_channels)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1482,
            0,
            "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)",
            index);
    return AIL_sample_playback_rate(milesGlob.handle_sample[index]);
}

void __cdecl SND_Set2DChannelPlaybackRate(int index, int rate)
{
    if (index < 0 || index >= g_snd.max_2D_channels)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1489,
            0,
            "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)",
            index);
    AIL_set_sample_playback_rate(milesGlob.handle_sample[index], rate);
}

int __cdecl SND_Get3DChannelPlaybackRate(int index)
{
    if (index < 8 || index >= g_snd.max_3D_channels + 8)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1496,
            0,
            "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)",
            index);
    return AIL_sample_playback_rate(milesGlob.handle_sample[index]);
}

void __cdecl SND_Set3DChannelPlaybackRate(int index, int rate)
{
    if (index < 8 || index >= g_snd.max_3D_channels + 8)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1504,
            0,
            "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)",
            index);
    AIL_set_sample_playback_rate(milesGlob.handle_sample[index], rate);
}

int __cdecl SND_GetStreamChannelPlaybackRate(int index)
{
    _SAMPLE *handle_sample; // [esp+0h] [ebp-4h]

    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1514,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
    handle_sample = (_SAMPLE *)AIL_stream_sample_handle((HSTREAM)milesGlob.handle_sample[index]);
    return AIL_sample_playback_rate(handle_sample);
}

void __cdecl SND_SetStreamChannelPlaybackRate(int index, int rate)
{
    _SAMPLE *handle_sample; // [esp+0h] [ebp-4h]

    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1526,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
    handle_sample = (_SAMPLE *)AIL_stream_sample_handle((HSTREAM)milesGlob.handle_sample[index]);
    AIL_set_sample_playback_rate(handle_sample, rate);
}

void __cdecl SND_Update2DChannelReverb(int index)
{
    float v1; // [esp+0h] [ebp-8h]
    float WetLevel; // [esp+4h] [ebp-4h]

    if (index < 0 || index >= g_snd.max_2D_channels)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1536,
            0,
            "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)",
            index);
    WetLevel = MSS_GetWetLevel(g_snd.chaninfo[index].alias0);
    v1 = CG_BannerScoreboardScaleMultiplier();
    AIL_set_sample_reverb_levels(milesGlob.handle_sample[index], v1, WetLevel);
}

void __cdecl SND_Update3DChannelReverb(int index)
{
    float v1; // [esp+0h] [ebp-8h]
    float WetLevel; // [esp+4h] [ebp-4h]

    if (index < 8 || index >= g_snd.max_3D_channels + 8)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1544,
            0,
            "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)",
            index);
    WetLevel = MSS_GetWetLevel(g_snd.chaninfo[index].alias0);
    v1 = CG_BannerScoreboardScaleMultiplier();
    AIL_set_sample_reverb_levels(milesGlob.handle_sample[index], v1, WetLevel);
}

void __cdecl SND_UpdateStreamChannelReverb(int index)
{
    float v1; // [esp+0h] [ebp-Ch]
    float WetLevel; // [esp+4h] [ebp-8h]
    _SAMPLE *handle_sample; // [esp+8h] [ebp-4h]

    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1555,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
    handle_sample = (_SAMPLE *)AIL_stream_sample_handle((HSTREAM)milesGlob.handle_sample[index]);
    WetLevel = MSS_GetWetLevel(g_snd.chaninfo[index].alias0);
    v1 = CG_BannerScoreboardScaleMultiplier();
    AIL_set_sample_reverb_levels(handle_sample, v1, WetLevel);
}

int __cdecl SND_Get2DChannelLength(int index)
{
    int length; // [esp+0h] [ebp-4h] BYREF

    if (index < 0 || index >= g_snd.max_2D_channels)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1567,
            0,
            "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)",
            index);
    AIL_sample_ms_position(milesGlob.handle_sample[index], &length, 0);
    return length;
}

int __cdecl SND_Get3DChannelLength(int index)
{
    int length; // [esp+0h] [ebp-4h] BYREF

    if (index < 8 || index >= g_snd.max_3D_channels + 8)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1578,
            0,
            "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)",
            index);
    AIL_sample_ms_position(milesGlob.handle_sample[index], &length, 0);
    return length;
}

int __cdecl SND_GetStreamChannelLength(int index)
{
    int length; // [esp+0h] [ebp-4h] BYREF

    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1590,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
    AIL_stream_ms_position((HSTREAM)milesGlob.handle_sample[index], &length, 0);
    return length;
}

void __cdecl SND_Get2DChannelSaveInfo(int index, snd_save_2D_sample_t *info)
{
    _SAMPLE *handle; // [esp+0h] [ebp-Ch]
    int offset; // [esp+4h] [ebp-8h] BYREF
    int length; // [esp+8h] [ebp-4h] BYREF

    if (index < 0 || index >= g_snd.max_2D_channels)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1603,
            0,
            "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)",
            index);
    handle = milesGlob.handle_sample[index];
    if (!handle)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1605, 0, "%s", "handle");
    AIL_sample_ms_position(handle, &length, &offset);
    info->fraction = (double)offset / (double)length;
    info->pitch = g_snd.chaninfo[index].pitch;
    AIL_sample_volume_pan(handle, &info->volume, 0);
    if (g_snd.volume == 0.0)
        info->volume = g_snd.chaninfo[index].basevolume;
    else
        info->volume = info->volume / g_snd.volume;
}

void __cdecl SND_Set2DChannelFromSaveInfo(int index, snd_save_2D_sample_t *info)
{
    float volume; // [esp+4h] [ebp-4h]

    if (index < 0 || index >= g_snd.max_2D_channels)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1620,
            0,
            "%s\n\t(index) = %i",
            "(index >= 0 && index < 0 + g_snd.max_2D_channels)",
            index);
    volume = info->volume * g_snd.volume;
    SND_Set2DChannelVolume(index, volume);
}

void __cdecl SND_Get3DChannelSaveInfo(int index, snd_save_3D_sample_t *info)
{
    _SAMPLE *handle; // [esp+0h] [ebp-Ch]
    int offset; // [esp+4h] [ebp-8h] BYREF
    int length; // [esp+8h] [ebp-4h] BYREF

    if (index < 8 || index >= g_snd.max_3D_channels + 8)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1632,
            0,
            "%s\n\t(index) = %i",
            "(index >= (0 + 8) && index < (0 + 8) + g_snd.max_3D_channels)",
            index);
    handle = milesGlob.handle_sample[index];
    if (!handle)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1635, 0, "%s", "handle");
    AIL_sample_ms_position(handle, &length, &offset);
    info->fraction = (double)offset / (double)length;
    info->pitch = g_snd.chaninfo[index].pitch;
    AIL_sample_volume_pan(handle, &info->volume, 0);
    if (g_snd.volume == 0.0)
        info->volume = g_snd.chaninfo[index].basevolume;
    else
        info->volume = info->volume / g_snd.volume;
    AIL_sample_3D_position(handle, info->org, &info->org[2], &info->org[1]);
}

void __cdecl SND_GetStreamChannelSaveInfo(int index, snd_save_stream_t *info)
{
    int v2; // [esp+0h] [ebp-3Ch]
    double timescale; // [esp+8h] [ebp-34h]
    float *org; // [esp+14h] [ebp-28h]
    float v5; // [esp+1Ch] [ebp-20h]
    _STREAM *handle; // [esp+2Ch] [ebp-10h]
    _SAMPLE *handle_sample; // [esp+30h] [ebp-Ch]
    int offset; // [esp+34h] [ebp-8h] BYREF
    int length; // [esp+38h] [ebp-4h] BYREF

    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1656,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
    handle = (_STREAM *)milesGlob.handle_sample[index];
    if (!handle)
        MyAssertHandler(".\\win32\\snd_driver.cpp", 1658, 0, "%s", "handle");
    handle_sample = (_SAMPLE *)AIL_stream_sample_handle((HSTREAM)milesGlob.handle_sample[index]);
    AIL_stream_ms_position(handle, &length, &offset);
    info->fraction = (double)offset / (double)length;
    if (g_snd.chaninfo[index].timescale)
    {
        timescale = g_snd.timescale;
        v2 = (int)((float)AIL_sample_playback_rate(handle_sample) / timescale);
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
}

void __cdecl SND_SetStreamChannelFromSaveInfo(int index, snd_save_stream_t *info)
{
    float volume; // [esp+4h] [ebp-4h]

    if (index < 40 || index >= g_snd.max_stream_channels + 40)
        MyAssertHandler(
            ".\\win32\\snd_driver.cpp",
            1676,
            0,
            "%s\n\t(index) = %i",
            "(index >= ((0 + 8) + 32) && index < ((0 + 8) + 32) + g_snd.max_stream_channels)",
            index);
    volume = info->volume * g_snd.volume;
    SND_SetStreamChannelVolume(index, volume);
}

int __cdecl SND_GetSoundFileSize(unsigned int *pSoundFile)
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
    float v2; // [esp+4h] [ebp-18h]
    float volume; // [esp+8h] [ebp-14h]
    float volumea; // [esp+8h] [ebp-14h]
    const snd_alias_t *alias1; // [esp+Ch] [ebp-10h]
    const snd_alias_t *alias0; // [esp+10h] [ebp-Ch]
    snd_channel_info_t *chaninfo; // [esp+18h] [ebp-4h]

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
        if (!chaninfo->startDelay && AIL_sample_status(milesGlob.handle_sample[i]) == 2
            || alias0->chainAliasName && chaninfo->totalMsec + chaninfo->startTime - g_snd.time <= 0)
        {
            SND_StopChannelAndPlayChainAlias(i);
        }
        else
        {
            if (g_snd.slaveLerp != 0.0 && !g_snd.chaninfo[i].master && (alias0->flags & 4) != 0)
                volume = SND_GetLerpedSlavePercentage(alias0->slavePercentage) * volume;
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
            MSS_ResumeSample(i, frametime);
        }
    }
}

void __cdecl SND_Update3DChannel(int i, int frametime)
{
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
        if (!chaninfo->startDelay && AIL_sample_status(milesGlob.handle_sample[i]) == 2
            || (timeleft = chaninfo->totalMsec + chaninfo->startTime - g_snd.time, alias0->chainAliasName) && timeleft <= 0)
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
            MSS_ResumeSample(i, frametime);
        }
    }
}

void __cdecl SND_UpdateStreamChannel(int i, int frametime)
{
    int v2; // [esp+4h] [ebp-1Ch]
    float volume; // [esp+Ch] [ebp-14h]
    float volumea; // [esp+Ch] [ebp-14h]
    float volumeb; // [esp+Ch] [ebp-14h]
    int listenerIndex; // [esp+10h] [ebp-10h]
    const snd_alias_t *alias1; // [esp+14h] [ebp-Ch]
    const snd_alias_t *alias0; // [esp+18h] [ebp-8h]
    snd_channel_info_t *chaninfo; // [esp+1Ch] [ebp-4h]

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
        if (g_snd.chaninfo[i].startDelay || AIL_stream_status((HSTREAM)milesGlob.handle_sample[i]) != 2)
        {
            if (SND_IsAliasChannel3D((alias0->flags & 0x3F00) >> 8))
            {
                SND_GetCurrent3DPosition(g_snd.chaninfo[i].sndEnt, g_snd.chaninfo[i].offset, g_snd.chaninfo[i].org);
                listenerIndex = SND_GetListenerIndexNearestToOrigin(g_snd.chaninfo[i].org);
                SND_Set3DStreamPosition(i, listenerIndex, g_snd.chaninfo[i].org);
                volume = SND_GetStream3DVolumeFallOff(i, listenerIndex) * volume;
            }
            if (g_snd.slaveLerp != 0.0 && !g_snd.chaninfo[i].master && (alias0->flags & 4) != 0)
                volume = SND_GetLerpedSlavePercentage(alias0->slavePercentage) * volume;
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
            if (g_snd.chaninfo[i].startDelay)
            {
                if (g_snd.chaninfo[i].startDelay - frametime > 0)
                    v2 = g_snd.chaninfo[i].startDelay - frametime;
                else
                    v2 = 0;
                g_snd.chaninfo[i].startDelay = v2;
                if (!g_snd.chaninfo[i].startDelay)
                    AIL_pause_stream((HSTREAM)milesGlob.handle_sample[i], 0);
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