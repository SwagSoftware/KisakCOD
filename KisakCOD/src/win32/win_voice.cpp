#include "win_local.h"

static unsigned int __cdecl mixerGetRecordLevel(char *SrcName)
{
    int v2; // eax
    tagMIXERCONTROLA mxc; // [esp+0h] [ebp-188h] BYREF
    unsigned int jj; // [esp+98h] [ebp-F0h]
    tagMIXERLINECONTROLSA mxlc; // [esp+9Ch] [ebp-ECh] BYREF
    HMIXER__ *phmx; // [esp+B8h] [ebp-D0h] BYREF
    unsigned int ii; // [esp+BCh] [ebp-CCh]
    tagMIXERLINEA mixerline; // [esp+C0h] [ebp-C8h] BYREF
    tMIXERCONTROLDETAILS_UNSIGNED newSetting; // [esp+16Ch] [ebp-1Ch] BYREF
    tMIXERCONTROLDETAILS mxcd; // [esp+170h] [ebp-18h] BYREF

    if (!waveInGetNumDevs())
        return -1;
    if (!mixerGetNumDevs())
        return -1;
    if (mixerOpen(&phmx, 0, 0, 0, 0))
        return -1;
    mixerline.cbStruct = 168;
    mixerline.dwComponentType = 7;
    mixerGetLineInfoA((HMIXEROBJ)phmx, &mixerline, 3u);
    jj = mixerline.cConnections;
    for (ii = 0; ii < jj; ++ii)
    {
        mixerline.dwSource = ii;
        mixerGetLineInfoA((HMIXEROBJ)phmx, &mixerline, 1u);
        strstr(mixerline.szName, SrcName);
        if (v2)
        {
            mxlc.cbStruct = 24;
            mxlc.dwLineID = mixerline.dwLineID;
            mxlc.dwControlID = 1342373889;
            mxlc.cControls = 1;
            mxlc.cbmxctrl = 148;
            mxlc.pamxctrl = &mxc;
            if (!mixerGetLineControlsA((HMIXEROBJ)phmx, &mxlc, 2u))
            {
                mxcd.cbStruct = 24;
                mxcd.cChannels = 1;
                mxcd.cbDetails = 4;
                mxcd.paDetails = &newSetting;
                mxcd.cMultipleItems = 0;
                mxcd.dwControlID = mxc.dwControlID;
                mixerGetControlDetailsA((HMIXEROBJ)phmx, &mxcd, 0);
                mixerClose(phmx);
                return newSetting.dwValue;
            }
            break;
        }
    }
    mixerClose(phmx);
    return -1;
}

bool __cdecl Voice_SendVoiceData()
{
    if (!sv_voice->current.enabled || !cl_voice->current.enabled || Dvar_GetInt("rate") < 5000)
        return 0;

    return clientUIActives[0].connectionState == CA_ACTIVE
        && (cl_talking->current.enabled || IN_IsTalkKeyHeld() || cl_voiceCommunication.voicePacketCount);
}

bool __cdecl Voice_Init()
{
    DvarLimits min; // [esp+4h] [ebp-30h]
    DvarLimits mina; // [esp+4h] [ebp-30h]
    HWND__ *handle; // [esp+2Ch] [ebp-8h]
    int client; // [esp+30h] [ebp-4h]

    winvoice_mic_mute = Dvar_RegisterBool("winvoice_mic_mute", 1, 1u, "Mute the microphone");
    min.value.max = 65535.0;
    min.value.min = 0.0;
    winvoice_mic_reclevel = Dvar_RegisterFloat("winvoice_mic_reclevel", 65535.0, min, 1u, "Microphone recording level");
    winvoice_save_voice = Dvar_RegisterBool("winvoice_save_voice", 0, 1u, "Write voice data to a file");
    mina.value.max = 2.0;
    mina.value.min = 0.25;
    winvoice_mic_scaler = Dvar_RegisterFloat("winvoice_mic_scaler", 1.0, mina, 1u, "Microphone scaler value");
    mixerGetRecordSource(old_rec_source);
    mixerSetRecordSource("Mic");
    mic_old_reclevel = mixerGetRecordLevel("Mic");
    mic_current_reclevel = (unsigned __int16)(int)winvoice_mic_reclevel->current.value;
    mixerSetRecordLevel("Mic", (int)winvoice_mic_reclevel->current.value);
    mixerSetMicrophoneMute(winvoice_mic_mute->current.color[0]);
    g_current_bandwidth_setting = 0;
    handle = GetDesktopWindow();
    Record_Init();
    Sound_Init(handle);
    Encode_Init(g_current_bandwidth_setting);
    Decode_Init(g_current_bandwidth_setting);
    g_voice_initialized = 1;
    memset((unsigned __int8 *)s_clientTalkTime, 0, sizeof(s_clientTalkTime));
    for (client = 0; client < 64; ++client)
        s_clientSamples[client] = Sound_NewSample();
    return 0;
}

void __cdecl Voice_Shutdown()
{
    int client; // [esp+0h] [ebp-4h]

    for (client = 0; client < 64; ++client)
        Sound_DestroySample(s_clientSamples[client]);
    Voice_StopRecording();
    Record_Shutdown();
    Encode_Shutdown();
    Decode_Shutdown();
    Sound_Shutdown();
    mixerSetMicrophoneMute(1u);
    mixerSetRecordLevel("Mic", mic_old_reclevel);
    mixerSetRecordSource(old_rec_source);
    g_voice_initialized = 0;
}

double __cdecl Voice_GetVoiceLevel()
{
    int ii; // [esp+0h] [ebp-8h]
    float avgLvl; // [esp+4h] [ebp-4h]

    if (!g_voice_initialized)
        return 0.0;
    levelSamples[sampleCount % 6] = voice_current_voicelevel / 32767.0 / 6.0;
    ++sampleCount;
    avgLvl = 0.0;
    for (ii = 0; ii < 6; ++ii)
        avgLvl = avgLvl + levelSamples[ii];
    return avgLvl;
}

void __cdecl Voice_Playback()
{
    int client; // [esp+24h] [ebp-4h]

    if (g_voice_initialized)
    {
        if (mic_current_reclevel != (unsigned __int16)(int)winvoice_mic_reclevel->current.value)
        {
            mic_current_reclevel = (unsigned __int16)(int)winvoice_mic_reclevel->current.value;
            mixerSetRecordLevel("Mic", (int)winvoice_mic_reclevel->current.value);
        }
        for (client = 0; client < 64; ++client)
            Sound_SampleFrame(s_clientSamples[client]);
        Sound_Frame();
    }
}

int __cdecl Voice_GetLocalVoiceData()
{
    if (!g_voice_initialized)
        return 0;
    if (!recording)
        Voice_StartRecording();
    if (winvoice_mic_scaler->current.value != voice_current_scaler)
    {
        voice_current_scaler = winvoice_mic_scaler->current.value;
        if (voice_current_scaler >= 0.5)
        {
            if (voice_current_scaler > 1.5)
                voice_current_scaler = 1.5;
        }
        else
        {
            voice_current_scaler = 0.5;
        }
    }
    Record_Frame();
    return 0;
}

void __cdecl Voice_IncomingVoiceData(unsigned __int8 talker, unsigned __int8 *data, int packetDataSize)
{
    int v3; // [esp+0h] [ebp-201Ch]
    iobuf *stream; // [esp+8h] [ebp-2014h]
    int v5; // [esp+10h] [ebp-200Ch]
    __int16 out[4096]; // [esp+14h] [ebp-2008h] BYREF
    unsigned int data_len; // [esp+2018h] [ebp-4h]

    if (!playing)
        playing = 1;
    data_len = 0;
    v5 = 0;
    if (talker >= 0x40u)
        MyAssertHandler(".\\win32\\win_voice.cpp", 207, 0, "%s\n\t(talker) = %i", "(talker >= 0 && talker < 64)", talker);
    s_clientTalkTime[talker] = Sys_Milliseconds();
    while (v5 < packetDataSize)
    {
        if (packetDataSize - v5 < g_frame_size)
            v3 = packetDataSize - v5;
        else
            v3 = g_frame_size;
        data_len = Decode_Sample((char *)&data[v5], v3, out, g_frame_size);
        if ((int)data_len > 0)
            Sound_UpdateSample(s_clientSamples[talker], (char *)out, data_len);
        v5 += v3;
    }
    if (winvoice_save_voice->current.enabled)
    {
        stream = fopen("voice.wav", "ab");
        if (stream)
        {
            fwrite(out, data_len, 1u, stream);
            fclose(stream);
        }
    }
}

bool __cdecl Voice_IsClientTalking(unsigned int clientNum)
{
    if (clientNum >= 0x40)
        MyAssertHandler(
            ".\\win32\\win_voice.cpp",
            236,
            0,
            "%s\n\t(clientNum) = %i",
            "(clientNum >= 0 && clientNum < 64)",
            clientNum);
    return (int)(Sys_Milliseconds() - s_clientTalkTime[clientNum]) < 300;
}

char __cdecl Voice_StartRecording()
{
    if (!recording)
    {
        currentRecordingSample = Record_NewSample();
        Record_Start(currentRecordingSample);
        recording = 1;
        ++count;
    }
    return 1;
}

char __cdecl Voice_StopRecording()
{
    if (!recording)
        return 0;
    Record_Stop(currentRecordingSample);
    Record_DestroySample(currentRecordingSample);
    recording = 0;
    return 1;
}

