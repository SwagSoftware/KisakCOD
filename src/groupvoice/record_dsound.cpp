#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "directsound.h"
#include <qcommon/qcommon.h>

dsound_sample_t *s_recordingSamplePtr;
dsound_sample_t *g_current_sample;
dsound_sample_t s_recordingSamples[65];
bool g_recording_initialized;
bool g_currently_recording;

LPDIRECTSOUNDCAPTURE g_pDSCaptureInstance;


int(__cdecl *current_audioCallback)(audioSample_t *);

void __cdecl DSOUNDRecord_UpdateSample(dsound_sample_t *pRecSample)
{
    int lLockSize; // [esp+8h] [ebp-48h]
    unsigned int lLockSizea; // [esp+8h] [ebp-48h]
    int lLockSizeb; // [esp+8h] [ebp-48h]
    HRESULT hr; // [esp+Ch] [ebp-44h]
    HRESULT hra; // [esp+Ch] [ebp-44h]
    void *pLock1; // [esp+14h] [ebp-3Ch] BYREF
    DWORD dwCapturePos; // [esp+18h] [ebp-38h] BYREF
    audioSample_t audioSample; // [esp+1Ch] [ebp-34h] BYREF
    DWORD dwLockLen2; // [esp+3Ch] [ebp-14h] BYREF
    int iChannels; // [esp+40h] [ebp-10h]
    void *pLock2; // [esp+44h] [ebp-Ch] BYREF
    DWORD dwReadPos; // [esp+48h] [ebp-8h] BYREF
    DWORD dwLockLen1; // [esp+4Ch] [ebp-4h] BYREF

    if (g_recording_initialized)
    {
        if (pRecSample->DSCB->GetCurrentPosition(&dwCapturePos, &dwReadPos) < 0)
            Com_PrintError(9, "Error: Failed to get cursor positions \n");
        if (dwReadPos >= pRecSample->dwCaptureOffset)
            lLockSize = dwReadPos - pRecSample->dwCaptureOffset;
        else
            lLockSize = dwReadPos + pRecSample->dwBufferSize - pRecSample->dwCaptureOffset;
        lLockSizea = lLockSize - lLockSize % 4;
        if (lLockSizea)
        {
            hr = pRecSample->DSCB->Lock(
                pRecSample->dwCaptureOffset,
                lLockSizea,
                &pLock1,
                &dwLockLen1,
                &pLock2,
                &dwLockLen2,
                0);
            if (hr >= 0)
            {
                audioSample.stereo = g_sound_channels > 1;
                if (g_sound_channels <= 1)
                    iChannels = 1;
                else
                    iChannels = 2;
                audioSample.bytesPerSample = 2;
                audioSample.frequency = g_sound_recordFrequency;
                audioSample.sampleOffset = 0;
                audioSample.channels = g_sound_channels;
                lLockSizeb = dwLockLen2 + dwLockLen1;
                if (pLock1)
                {
                    audioSample.lengthInSamples = dwLockLen1 / (2 * iChannels);
                    audioSample.lengthInBytes = dwLockLen1;
                    audioSample.buffer = (unsigned char *)pLock1;
                    current_audioCallback(&audioSample);
                }
                if (pLock2)
                {
                    audioSample.lengthInSamples = dwLockLen2 / (2 * iChannels);
                    audioSample.lengthInBytes = dwLockLen2;
                    audioSample.buffer = (unsigned char*)pLock2;
                    current_audioCallback(&audioSample);
                }
                pRecSample->dwCaptureOffset += lLockSizeb;
                pRecSample->dwCaptureOffset %= pRecSample->dwBufferSize;
                hra = pRecSample->DSCB->Unlock(pLock1, dwLockLen1, pLock2, dwLockLen2);
                if (hra < 0)
                {
                    if (hra == -2147024809)
                    {
                        Com_Printf(9, "DSERR_INVALIDPARAM\n");
                        Com_Printf(9, " Offset : %i\n", pRecSample->dwCaptureOffset);
                    }
                    else if (hra == -2005401550)
                    {
                        Com_Printf(9, "DSERR_INVALIDCALL\n");
                    }
                    Com_PrintError(9, "Error trying to unlock sample!\n");
                }
            }
            else
            {
                if (hr == -2147024809)
                {
                    Com_Printf(9, "DSERR_INVALIDPARAM\n");
                    Com_Printf(9, " Offset : %i, length: %i\n", pRecSample->dwCaptureOffset, lLockSizea);
                }
                else if (hr == -2005401550)
                {
                    Com_Printf(9, "DSERR_INVALIDCALL\n");
                }
                Com_PrintError(9, "Error trying to lock sample!\n");
            }
        }
    }
}

void __cdecl DSOUNDRecord_Frame()
{
    if (g_recording_initialized && g_currently_recording)
    {
        if (g_current_sample)
            DSOUNDRecord_UpdateSample(g_current_sample);
    }
}

dsound_sample_t *__cdecl DSOUNDRecord_NewSample()
{
    dsound_sample_t *pRecSample; // [esp+0h] [ebp-4h]

    if (!g_recording_initialized)
        return 0;
    if (s_recordingSamplePtr - s_recordingSamples > 65)
        MyAssertHandler(
            ".\\groupvoice\\record_dsound.cpp",
            197,
            0,
            "%s",
            "s_recordingSamplePtr - s_recordingSamples <= MAX_CLIENTS + 1");
    pRecSample = s_recordingSamplePtr++;
    memset(pRecSample, 0, sizeof(dsound_sample_t));
    pRecSample->frequency = g_sound_recordFrequency;
    pRecSample->volume = g_sound_recordVolume;
    pRecSample->pan = 128;
    pRecSample->channels = g_sound_channels;
    pRecSample->stopPosition = -1;
    pRecSample->playMode = 2;
    return pRecSample;
}

int __cdecl DSOUNDRecord_DestroySample(dsound_sample_t *pRecSample)
{
    if (!g_recording_initialized)
        return 0;
    if (pRecSample->DSB)
    {
        pRecSample->DSB->Release();
        pRecSample->DSB = 0;
    }
    return 1;
}

HRESULT __cdecl DSOUNDRecord_Start(dsound_sample_t *pRecSample)
{
    HRESULT hr; // [esp+4h] [ebp-34h]
    HRESULT hra; // [esp+4h] [ebp-34h]
    _DSCBUFFERDESC dscbd; // [esp+8h] [ebp-30h] BYREF
    tWAVEFORMATEX wfx; // [esp+24h] [ebp-14h] BYREF

    if (!g_recording_initialized)
        return -1;
    if (!pRecSample)
        return -1;
    wfx.wFormatTag = 1;
    wfx.nChannels = g_sound_channels;
    wfx.wBitsPerSample = 16;
    wfx.nSamplesPerSec = g_sound_recordFrequency;
    wfx.nAvgBytesPerSec = 2 * g_sound_channels * g_sound_recordFrequency;
    wfx.nBlockAlign = 2 * g_sound_channels;
    wfx.cbSize = 0;
    dscbd.dwSize = 28;
    dscbd.dwFlags = 0;
    dscbd.dwBufferBytes = g_sound_recordBufferSize;
    dscbd.dwReserved = 0;
    dscbd.lpwfxFormat = &wfx;
    dscbd.dwFXCount = 0;
    dscbd.lpDSCFXDesc = 0;
    if (pRecSample->DSCB
        || (hr = (g_pDSCaptureInstance->CreateCaptureBuffer)(
            &dscbd,
            &pRecSample->DSCB,
            0),
            hr >= 0))
    {
        pRecSample->dwCaptureOffset = 0;
        pRecSample->dwBufferSize = dscbd.dwBufferBytes;
        hra = pRecSample->DSCB->Start(1u);
        if (hra < 0)
        {
            Com_PrintError(9, "error: Unable to Read to Buffer\n");
            return -1;
        }
        else
        {
            g_current_sample = pRecSample;
            g_currently_recording = 1;
            return hra;
        }
    }
    else
    {
        if (hr > -2005401500)
        {
            if (hr == -2005401480)
            {
                Com_Printf(9, "NODRIVER \n");
            }
            else if (hr == -2005401430)
            {
                Com_Printf(9, "UNINITIALIZED \n");
            }
        }
        else
        {
            switch (hr)
            {
            case -2005401500:
                Com_Printf(9, "BADFORMAT\n");
                break;
            case -2147467259:
                Com_Printf(9, "GENERIC\n");
                break;
            case -2147024882:
                Com_Printf(9, "OUTOFMEMORY \n");
                break;
            case -2147024809:
                Com_Printf(9, "INVALIDPARAM\n");
                break;
            }
        }
        Com_PrintError(9, "error: Failed to allocate Capture Buffer \n");
        return -1;
    }
}

HRESULT __cdecl DSOUNDRecord_Stop(dsound_sample_t *pRecSample)
{
    HRESULT hr; // [esp+0h] [ebp-4h]

    if (!g_recording_initialized)
        return -1;
    if (!pRecSample->DSCB)
        return -1;
    if (pRecSample->DSCB->Stop() < 0)
    {
        Com_PrintError(9, "Error stopping recording buffer\n");
    }
    else
    {
        g_current_sample = 0;
        g_currently_recording = 0;
    }
    hr = pRecSample->DSCB->Release();
    if (hr < 0)
        Com_PrintError(9, "Error releasing recording buffer\n");
    else
        pRecSample->DSCB = 0;
    return hr;
}

int __cdecl DSOUNDRecord_Init(bool bCallDsoundInit)
{
    HRESULT hr; // [esp+0h] [ebp-4h]

    s_recordingSamplePtr = s_recordingSamples;
    g_recording_initialized = 0;
    if (bCallDsoundInit && (hr = DirectSoundCaptureCreate(0, &g_pDSCaptureInstance, 0), hr < 0))
    {
        Com_PrintError(9, "Error initializing direct sound instance!  %s\n", hr);
        return 0;
    }
    else
    {
        g_recording_initialized = 1;
        return 1;
    }
}

void __cdecl DSOUNDRecord_Shutdown()
{
    HRESULT hr; // [esp+0h] [ebp-4h]

    hr = 0;
    if (g_pDSCaptureInstance)
        hr = g_pDSCaptureInstance->Release();
    g_pDSCaptureInstance = 0;
    if (hr < 0)
        Com_PrintError(9, "Error releasing direct sound instance!  %s\n", hr);
}

void __cdecl Record_SetRecordingCallback(int(__cdecl *new_audioCallback)(audioSample_t *))
{
    current_audioCallback = new_audioCallback;
}