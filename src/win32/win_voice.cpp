#include "win_local.h"
#include <server_mp/server_mp.h>
#include <groupvoice/directsound.h>

const dvar_t *winvoice_mic_mute;
const dvar_t *winvoice_mic_reclevel;
const dvar_t *winvoice_save_voice;
const dvar_t *winvoice_mic_scaler;

char old_rec_source[256];
int mic_old_reclevel;
int mic_current_reclevel;
int g_voice_initialized;
int s_clientTalkTime[64];
bool recording;
float voice_current_scaler;

float levelSamples[6];
int sampleCount;
float voice_current_voicelevel;
bool playing;
int count;

dsound_sample_t *currentRecordingSample;
dsound_sample_t *s_clientSamples[64];

static unsigned int __cdecl mixerGetRecordLevel(char *SrcName)
{
    const char *v2; // eax
    tagMIXERCONTROLA mxc; // [esp+0h] [ebp-188h] BYREF
    unsigned int jj; // [esp+98h] [ebp-F0h]
    tagMIXERLINECONTROLSA mxlc; // [esp+9Ch] [ebp-ECh] BYREF
    HMIXER__ *phmx; // [esp+B8h] [ebp-D0h] BYREF
    unsigned int ii; // [esp+BCh] [ebp-CCh]
    tagMIXERLINEA mixerline{ 0 }; // [esp+C0h] [ebp-C8h] BYREF
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
    MMRESULT res = mixerGetLineInfoA((HMIXEROBJ)phmx, &mixerline, 3u); // KISAKTODO: this fails with ret: 0x400 - idk why
    jj = mixerline.cConnections;
    for (ii = 0; ii < jj; ++ii)
    {
        mixerline.dwSource = ii;
        mixerGetLineInfoA((HMIXEROBJ)phmx, &mixerline, 1u);
        v2 = strstr(mixerline.szName, SrcName);
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

int __cdecl mixerSetRecordSource(char *SrcName)
{
    const char *v2; // eax
    int jj; // [esp+0h] [ebp-100h]
    tagMIXERLINECONTROLSA mxlc; // [esp+4h] [ebp-FCh] BYREF
    unsigned int err; // [esp+1Ch] [ebp-E4h]
    int iNumChannels; // [esp+20h] [ebp-E0h]
    int iMultipleItems; // [esp+24h] [ebp-DCh]
    HMIXER mixerHandle; // [esp+28h] [ebp-D8h] BYREF
    tMIXERCONTROLDETAILS_BOOLEAN *lpListBool; // [esp+2Ch] [ebp-D4h]
    int ii; // [esp+30h] [ebp-D0h]
    tagMIXERCONTROLDETAILS_LISTTEXTA *lpListText; // [esp+34h] [ebp-CCh]
    tagMIXERLINEA mixerline; // [esp+38h] [ebp-C8h] BYREF
    tMIXERCONTROLDETAILS mxcd; // [esp+E4h] [ebp-1Ch] BYREF
    tagMIXERCONTROLA *lpmxc; // [esp+FCh] [ebp-4h]

    if (!waveInGetNumDevs())
        return 0;
    if (!mixerGetNumDevs())
        return 0;
    if (mixerOpen(&mixerHandle, 0, 0, 0, 0))
        return 0;

    // LWSS: Winapi sucks
    HMIXEROBJ phmx = (HMIXEROBJ)mixerHandle;

    lpmxc = 0;
    lpListText = 0;
    lpListBool = 0;
    mixerline.cbStruct = 168;
    mixerline.dwComponentType = 7;
    if (mixerGetLineInfoA(phmx, &mixerline, 3u) != MMSYSERR_NOERROR)
    {
        mixerClose(mixerHandle); // KISAKTODO: this fails always for some reason.
        return 0;
    }
    lpmxc = (tagMIXERCONTROLA*)calloc(148 * mixerline.cControls, 1u);
    mxlc.cbStruct = 24;
    mxlc.dwLineID = mixerline.dwLineID;
    mxlc.dwControlID = 0;
    mxlc.cControls = mixerline.cControls;
    mxlc.cbmxctrl = 148;
    mxlc.pamxctrl = lpmxc;
    err = mixerGetLineControlsA(phmx, &mxlc, 0);
    if (!err)
    {
        for (ii = 0; ii < mixerline.cControls; ++ii)
        {
            if ((lpmxc[ii].dwControlType & 0xF0000000) == 0x70000000)
            {
                iNumChannels = mixerline.cChannels;
                iMultipleItems = 0;
                if ((lpmxc[ii].fdwControl & 1) != 0)
                    iNumChannels = 1;
                if ((lpmxc[ii].fdwControl & 2) != 0)
                    iMultipleItems = lpmxc[ii].cMultipleItems;
                lpListText = (tagMIXERCONTROLDETAILS_LISTTEXTA*)calloc(72 * iMultipleItems * iNumChannels, 1u);
                mxcd.cbStruct = 24;
                mxcd.dwControlID = lpmxc[ii].dwControlID;
                mxcd.cChannels = iNumChannels;
                mxcd.cMultipleItems = iMultipleItems;
                mxcd.cbDetails = 72;
                mxcd.paDetails = lpListText;
                err = mixerGetControlDetailsA(phmx, &mxcd, 1u);
                if (!err)
                {
                    lpListBool = (tMIXERCONTROLDETAILS_BOOLEAN*)calloc(4 * iMultipleItems * iNumChannels, 1u);
                    mxcd.cbDetails = 4;
                    mxcd.paDetails = lpListBool;
                    err = mixerGetControlDetailsA(phmx, &mxcd, 0);
                    if (!err)
                    {
                        for (jj = 0; jj < iMultipleItems; ++jj)
                        {
                            v2 = strstr(lpListText[jj].szName, SrcName);
                            if (v2)
                                lpListBool[jj].fValue = 1;
                            else
                                lpListBool[jj].fValue = 0;
                        }
                        err = mixerSetControlDetails(phmx, &mxcd, 0);
                        if (lpmxc)
                            free(lpmxc);
                        lpmxc = 0;
                        if (lpListText)
                            free(lpListText);
                        lpListText = 0;
                        if (lpListBool)
                            free(lpListBool);
                        lpListBool = 0;
                        if (!err)
                            return 1;
                    }
                }
            }
        }
    }
    if (lpmxc)
        free(lpmxc);
    lpmxc = 0;
    if (lpListText)
        free(lpListText);
    lpListText = 0;
    if (lpListBool)
        free(lpListBool);

    mixerClose(mixerHandle); // LWSS ADD

    return 0;
}

int __cdecl mixerGetRecordSource(char *srcName)
{
    int jj; // [esp+4h] [ebp-100h]
    tagMIXERLINECONTROLSA mxlc; // [esp+8h] [ebp-FCh] BYREF
    unsigned int err; // [esp+20h] [ebp-E4h]
    int iNumChannels; // [esp+24h] [ebp-E0h]
    int iMultipleItems; // [esp+28h] [ebp-DCh]
    tMIXERCONTROLDETAILS_BOOLEAN *lpListBool; // [esp+30h] [ebp-D4h]
    int ii; // [esp+34h] [ebp-D0h]
    tagMIXERCONTROLDETAILS_LISTTEXTA *lpListText; // [esp+38h] [ebp-CCh]
    tagMIXERLINEA mixerline; // [esp+3Ch] [ebp-C8h] BYREF
    tMIXERCONTROLDETAILS mxcd; // [esp+E8h] [ebp-1Ch] BYREF
    tagMIXERCONTROLA *lpmxc; // [esp+100h] [ebp-4h]

    HMIXER mixerHandle; // [esp+2Ch] [ebp-D8h] BYREF

    if (!waveInGetNumDevs())
        return 0;
    if (!mixerGetNumDevs())
        return 0;
    if (mixerOpen(&mixerHandle, 0, 0, 0, 0))
        return 0;
    
    // LWSS: Winapi sucks
    HMIXEROBJ phmx = (HMIXEROBJ)mixerHandle;

    lpmxc = 0;
    lpListText = 0;
    lpListBool = 0;
    mixerline.cbStruct = 168;
    mixerline.dwComponentType = 7;
    if (mixerGetLineInfoA(phmx, &mixerline, 3u) != MMSYSERR_NOERROR)
    {
        mixerClose(mixerHandle); // KISAKTODO: this fails always for some reason.
        return 0;
    }
    lpmxc = (tagMIXERCONTROLA*)calloc(148 * mixerline.cControls, 1u);
    mxlc.cbStruct = 24;
    mxlc.dwLineID = mixerline.dwLineID;
    mxlc.dwControlID = 0;
    mxlc.cControls = mixerline.cControls;
    mxlc.cbmxctrl = 148;
    mxlc.pamxctrl = lpmxc;
    err = mixerGetLineControlsA(phmx, &mxlc, 0);
    if (!err)
    {
        for (ii = 0; ii < mixerline.cControls; ++ii)
        {
            if ((lpmxc[ii].dwControlType & 0xF0000000) == 0x70000000)
            {
                iNumChannels = mixerline.cChannels;
                iMultipleItems = 0;
                if ((lpmxc[ii].fdwControl & 1) != 0)
                    iNumChannels = 1;
                if ((lpmxc[ii].fdwControl & 2) != 0)
                    iMultipleItems = lpmxc[ii].cMultipleItems;
                lpListText = (tagMIXERCONTROLDETAILS_LISTTEXTA*)calloc(72 * iMultipleItems * iNumChannels, 1u);
                mxcd.cbStruct = 24;
                mxcd.dwControlID = lpmxc[ii].dwControlID;
                mxcd.cChannels = iNumChannels;
                mxcd.cMultipleItems = iMultipleItems;
                mxcd.cbDetails = 72;
                mxcd.paDetails = lpListText;
                err = mixerGetControlDetailsA(phmx, &mxcd, 1u);
                if (!err)
                {
                    lpListBool = (tMIXERCONTROLDETAILS_BOOLEAN*)calloc(4 * iMultipleItems * iNumChannels, 1u);
                    mxcd.cbDetails = 4;
                    mxcd.paDetails = lpListBool;
                    err = mixerGetControlDetailsA(phmx, &mxcd, 0);
                    if (!err)
                    {
                        for (jj = 0; jj < iMultipleItems; ++jj)
                        {
                            if (lpListBool[jj].fValue == 1)
                                strncpy(srcName, lpListText[jj].szName, 0xFFu);
                        }
                        if (lpmxc)
                            free(lpmxc);
                        lpmxc = 0;
                        if (lpListText)
                            free(lpListText);
                        lpListText = 0;
                        if (lpListBool)
                            free(lpListBool);
                        lpListBool = 0;
                        if (!err)
                            return 1;
                    }
                }
            }
        }
    }
    if (lpmxc)
        free(lpmxc);
    lpmxc = 0;
    if (lpListText)
        free(lpListText);
    lpListText = 0;
    if (lpListBool)
        free(lpListBool);

    mixerClose(mixerHandle); // LWSS ADD

    return 0;
}

int __cdecl mixerSetRecordLevel(char *SrcName, unsigned __int16 newLevel)
{
    const char *v3; // eax
    tagMIXERCONTROLA mxc; // [esp+0h] [ebp-188h] BYREF
    unsigned int jj; // [esp+98h] [ebp-F0h]
    tagMIXERLINECONTROLSA mxlc; // [esp+9Ch] [ebp-ECh] BYREF
    unsigned int err; // [esp+B4h] [ebp-D4h]
    HMIXER mixerHandle; // [esp+B8h] [ebp-D0h] BYREF
    unsigned int ii; // [esp+BCh] [ebp-CCh]
    tagMIXERLINEA mixerline; // [esp+C0h] [ebp-C8h] BYREF
    tMIXERCONTROLDETAILS_UNSIGNED newSetting; // [esp+16Ch] [ebp-1Ch] BYREF
    tMIXERCONTROLDETAILS mxcd; // [esp+170h] [ebp-18h] BYREF

    if (!waveInGetNumDevs())
        return 0;
    if (!mixerGetNumDevs())
        return 0;
    if (mixerOpen(&mixerHandle, 0, 0, 0, 0))
        return 0;

    // LWSS: Winapi sucks
    HMIXEROBJ phmx = (HMIXEROBJ)mixerHandle;

    mixerline.cbStruct = 168;
    mixerline.dwComponentType = 7;
    if (mixerGetLineInfoA(phmx, &mixerline, 3u))
        return 0;
    jj = mixerline.cConnections;
    for (ii = 0; ii < jj; ++ii)
    {
        mixerline.dwSource = ii;
        if (!mixerGetLineInfoA(phmx, &mixerline, 1u))
        {
            v3 = strstr(mixerline.szName, SrcName);
            if (v3)
            {
                mxlc.cbStruct = 24;
                mxlc.dwLineID = mixerline.dwLineID;
                mxlc.dwControlID = 1342373889;
                mxlc.cControls = 1;
                mxlc.cbmxctrl = 148;
                mxlc.pamxctrl = &mxc;
                err = mixerGetLineControlsA(phmx, &mxlc, 2u);
                if (!err)
                {
                    mxcd.cbStruct = 24;
                    mxcd.cChannels = 1;
                    mxcd.cbDetails = 4;
                    mxcd.paDetails = &newSetting;
                    mxcd.cMultipleItems = 0;
                    mxcd.dwControlID = mxc.dwControlID;
                    err = mixerGetControlDetailsA(phmx, &mxcd, 0);
                    if (!err)
                    {
                        newSetting.dwValue = newLevel;
                        mixerSetControlDetails(phmx, &mxcd, 0);
                    }
                }
                break;
            }
        }
    }
    mixerClose(mixerHandle);
    return 0;
}

int __cdecl mixerSetMicrophoneMute(unsigned __int8 bMute)
{
    const char *v2; // eax
    tagMIXERCONTROLA mxc; // [esp+0h] [ebp-188h] BYREF
    unsigned int jj; // [esp+98h] [ebp-F0h]
    tagMIXERLINECONTROLSA mxlc; // [esp+9Ch] [ebp-ECh] BYREF
    unsigned int err; // [esp+B4h] [ebp-D4h]
    HMIXER mixerHandle; // [esp+B8h] [ebp-D0h] BYREF
    unsigned int ii; // [esp+BCh] [ebp-CCh]
    tagMIXERLINEA mixerline; // [esp+C0h] [ebp-C8h] BYREF
    tMIXERCONTROLDETAILS_BOOLEAN newSetting; // [esp+16Ch] [ebp-1Ch] BYREF
    tMIXERCONTROLDETAILS mxcd; // [esp+170h] [ebp-18h] BYREF

    if (!waveInGetNumDevs())
        return 0;
    if (!mixerGetNumDevs())
        return 0;
    if (mixerOpen(&mixerHandle, 0, 0, 0, 0))
        return 0;

    // LWSS: Winapi sucks
    HMIXEROBJ phmx = (HMIXEROBJ)mixerHandle;

    mixerline.cbStruct = 168;
    mixerline.dwComponentType = 4;
    err = mixerGetLineInfoA(phmx, &mixerline, 3u);
    if (!err)
    {
        jj = mixerline.cConnections;
        for (ii = 0; ii < jj; ++ii)
        {
            mixerline.dwSource = ii;
            mixerGetLineInfoA(phmx, &mixerline, 1u);
            v2 = strstr(mixerline.szName, "Mic");
            if (v2)
            {
                mxlc.cbStruct = 24;
                mxlc.dwLineID = mixerline.dwLineID;
                mxlc.dwControlID = 536936450;
                mxlc.cControls = 1;
                mxlc.cbmxctrl = 148;
                mxlc.pamxctrl = &mxc;
                err = mixerGetLineControlsA(phmx, &mxlc, 2u);
                if (!err)
                {
                    mxcd.cbStruct = 24;
                    mxcd.cChannels = 1;
                    mxcd.cbDetails = 4;
                    mxcd.paDetails = &newSetting;
                    mxcd.cMultipleItems = 0;
                    mxcd.dwControlID = mxc.dwControlID;
                    err = mixerGetControlDetailsA(phmx, &mxcd, 0);
                    if (!err)
                    {
                        newSetting.fValue = bMute;
                        err = mixerSetControlDetails(phmx, &mxcd, 0);
                    }
                }
                break;
            }
        }
    }
    mixerClose(mixerHandle);
    return 0;
}

int __cdecl Client_SendVoiceData(int bytes, char *enc_buffer)
{
    if (bytes > 0)
    {
        memcpy(cl_voiceCommunication.voicePackets[cl_voiceCommunication.voicePacketCount].data, enc_buffer, bytes);
        cl_voiceCommunication.voicePackets[cl_voiceCommunication.voicePacketCount++].dataSize = bytes;
        CL_VoiceTransmit(0);
    }
    return bytes;
}

int samples_in_partial_audio_buffer;
__int16 partial_audio_buffer[640];
char enc_buffer[4096];
int enc_buffer_pos;
int __cdecl Record_QueueAudioDataForEncoding(audioSample_t *sample)
{
    __int64 v1; // rax
    int v3; // [esp+0h] [ebp-2030h]
    int v4; // [esp+10h] [ebp-2020h]
    int v5; // [esp+14h] [ebp-201Ch]
    int bytes; // [esp+18h] [ebp-2018h]
    _WORD dst[4098]; // [esp+20h] [ebp-2010h] BYREF
    int i; // [esp+2028h] [ebp-8h]
    int FrameSize; // [esp+202Ch] [ebp-4h]

    voice_current_voicelevel = 0.0;
    if (sample->bytesPerSample == 2 && sample->lengthInSamples > 0)
    {
        for (i = 0; i < sample->lengthInSamples; ++i)
        {
            *(_WORD*)&sample->buffer[2 * i] = (int)(*(__int16*)&sample->buffer[2 * i] * voice_current_scaler);
            v1 = *(__int16*)&sample->buffer[2 * i];
            voice_current_voicelevel = ((HIDWORD(v1) ^ v1) - HIDWORD(v1)) + voice_current_voicelevel;
        }
        voice_current_voicelevel = voice_current_voicelevel / sample->lengthInSamples;
    }
    if (!Voice_SendVoiceData())
        return 0;
    if (IN_IsTalkKeyHeld())
    {
        FrameSize = Encode_GetFrameSize();
        sample->sampleOffset = 0;
        v4 = 0;
        if (FrameSize > 0)
        {
            if (sample->lengthInBytes > 0)
            {
                if (sample->sampleOffset >= 0)
                {
                    while (FrameSize <= samples_in_partial_audio_buffer + sample->lengthInSamples - sample->sampleOffset)
                    {
                        v5 = 0;
                        if (samples_in_partial_audio_buffer)
                        {
                            if (samples_in_partial_audio_buffer < FrameSize - 4)
                                v3 = samples_in_partial_audio_buffer;
                            else
                                v3 = FrameSize - 4;
                            samples_in_partial_audio_buffer = v3;
                            if (sample->bytesPerSample * v3 > 0x2000)
                                MyAssertHandler(
                                    ".\\groupvoice\\record.cpp",
                                    117,
                                    0,
                                    "%s",
                                    "2*ENC_BUFFER_SIZE >= samples_copied * sample->bytesPerSample");
                            if (sample->bytesPerSample * v3 > 1280)
                                MyAssertHandler(
                                    ".\\groupvoice\\record.cpp",
                                    118,
                                    0,
                                    "%s",
                                    "2*AUDIO_BUFFER_SIZE >= samples_copied * sample->bytesPerSample");
                            memcpy(dst, partial_audio_buffer, sample->bytesPerSample * v3);
                            v5 = v3;
                            samples_in_partial_audio_buffer -= v3;
                        }
                        memcpy(
                            &dst[v5],
                            &sample->buffer[sample->bytesPerSample * sample->sampleOffset],
                            sample->bytesPerSample * (FrameSize - v5));
                        sample->sampleOffset += FrameSize - v5;
                        bytes = Encode_Sample((short*)dst, &enc_buffer[enc_buffer_pos], 4096 - enc_buffer_pos);
                        v4 += bytes;
                        Client_SendVoiceData(bytes, &enc_buffer[enc_buffer_pos]);
                    }
                    if (sample->sampleOffset < sample->lengthInSamples)
                    {
                        memcpy(
                            &partial_audio_buffer[samples_in_partial_audio_buffer],
                            &sample->buffer[sample->bytesPerSample * sample->sampleOffset],
                            sample->bytesPerSample * (sample->lengthInSamples - sample->sampleOffset));
                        samples_in_partial_audio_buffer += sample->lengthInSamples - sample->sampleOffset;
                    }
                    return v4;
                }
                else
                {
                    Com_Printf(9, "Invalid sample offset of %i\n", sample->sampleOffset);
                    return 0;
                }
            }
            else
            {
                Com_Printf(9, "Invalid sample length of %i samples\n", sample->lengthInSamples);
                return 0;
            }
        }
        else
        {
            Com_Printf(9, "Invalid encode frame size of %i\n", FrameSize);
            return 0;
        }
    }
    else
    {
        CL_VoiceTransmit(0);
        return 0;
    }
}

int __cdecl Record_AudioCallback(audioSample_t *sample)
{
    return Record_QueueAudioDataForEncoding(sample);
}

int __cdecl Record_Init()
{
    Record_SetRecordingCallback(Record_AudioCallback);
    return DSOUNDRecord_Init(1);
}

int __cdecl Sound_Init(HWND__ *handle)
{
    return DSound_Init(1, handle);
}

dsound_sample_t *__cdecl Sound_NewSample()
{
    return DSound_NewSample();
}

bool __cdecl Voice_Init()
{
    DvarLimits min; // [esp+4h] [ebp-30h]
    DvarLimits mina; // [esp+4h] [ebp-30h]
    HWND__ *handle; // [esp+2Ch] [ebp-8h]
    int client; // [esp+30h] [ebp-4h]

    winvoice_mic_mute = Dvar_RegisterBool("winvoice_mic_mute", 1, DVAR_ARCHIVE, "Mute the microphone");
    min.value.max = 65535.0;
    min.value.min = 0.0;
    winvoice_mic_reclevel = Dvar_RegisterFloat("winvoice_mic_reclevel", 65535.0, min, DVAR_ARCHIVE, "Microphone recording level");
    winvoice_save_voice = Dvar_RegisterBool("winvoice_save_voice", 0, DVAR_ARCHIVE, "Write voice data to a file");
    mina.value.max = 2.0;
    mina.value.min = 0.25;
    winvoice_mic_scaler = Dvar_RegisterFloat("winvoice_mic_scaler", 1.0, mina, DVAR_ARCHIVE, "Microphone scaler value");
    mixerGetRecordSource(old_rec_source);
    mixerSetRecordSource((char*)"Mic");
    mic_old_reclevel = mixerGetRecordLevel((char*)"Mic");
    mic_current_reclevel = (unsigned __int16)(int)winvoice_mic_reclevel->current.value;
    mixerSetRecordLevel((char*)"Mic", (int)winvoice_mic_reclevel->current.value);
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

int __cdecl Record_DestroySample(dsound_sample_t *sample);

int __cdecl Sound_DestroySample(struct dsound_sample_t *sample)
{
    return Record_DestroySample(sample);
}

void __cdecl Record_Shutdown()
{
    Encode_Shutdown();
    DSOUNDRecord_Shutdown();
}

void __cdecl Sound_Shutdown()
{
    DSound_Shutdown();
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
    mixerSetRecordLevel((char*)"Mic", mic_old_reclevel);
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

void __cdecl Sound_SampleFrame(dsound_sample_t *sample)
{
    DSound_SampleFrame(sample);
}

void __cdecl Sound_Frame()
{
    KISAK_NULLSUB();
}

void __cdecl Voice_Playback()
{
    int client; // [esp+24h] [ebp-4h]

    if (g_voice_initialized)
    {
        if (mic_current_reclevel != (unsigned __int16)(int)winvoice_mic_reclevel->current.value)
        {
            mic_current_reclevel = (unsigned __int16)(int)winvoice_mic_reclevel->current.value;
            mixerSetRecordLevel((char*)"Mic", (int)winvoice_mic_reclevel->current.value);
        }
        for (client = 0; client < 64; ++client)
            Sound_SampleFrame(s_clientSamples[client]);
        Sound_Frame();
    }
}

void __cdecl Record_Frame()
{
    DSOUNDRecord_Frame();
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

unsigned int __cdecl Sound_UpdateSample(dsound_sample_t *sample, char *data, unsigned int data_len)
{
    return DSound_UpdateSample(sample, data, data_len);
}

void __cdecl Voice_IncomingVoiceData(unsigned __int8 talker, unsigned __int8 *data, int packetDataSize)
{
    int v3; // [esp+0h] [ebp-201Ch]
    FILE *stream; // [esp+8h] [ebp-2014h]
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

dsound_sample_t *__cdecl Record_NewSample()
{
    return DSOUNDRecord_NewSample();
}

HRESULT __cdecl Record_Start(dsound_sample_t *sample)
{
    return DSOUNDRecord_Start(sample);
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

HRESULT __cdecl Record_Stop(dsound_sample_t *sample)
{
    return DSOUNDRecord_Stop(sample);
}

int __cdecl Record_DestroySample(dsound_sample_t *sample)
{
    return DSOUNDRecord_DestroySample(sample);
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

