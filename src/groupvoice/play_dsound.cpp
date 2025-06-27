#include "directsound.h"
#include <universal/assertive.h>
#include <qcommon/qcommon.h>

bool dsoundplay_initialized;

LPDIRECTSOUND8 lpds;

int __cdecl DSound_GetBytesLeft(dsound_sample_t *sample)
{
    return sample->bytesBuffered;
}

unsigned int __cdecl DSound_UpdateSample(dsound_sample_t *sample, char *data, signed int data_len)
{
    signed int v4; // [esp+8h] [ebp-2Ch]
    HRESULT hr; // [esp+10h] [ebp-24h]
    HRESULT hra; // [esp+10h] [ebp-24h]
    unsigned int dataOffset; // [esp+18h] [ebp-1Ch]
    int bytesLeft; // [esp+20h] [ebp-14h]
    void *pLock1; // [esp+24h] [ebp-10h] BYREF
    DWORD dwLockLen2; // [esp+28h] [ebp-Ch] BYREF
    void *pLock2; // [esp+2Ch] [ebp-8h] BYREF
    DWORD dwLockLen1; // [esp+30h] [ebp-4h] BYREF

    if (!dsoundplay_initialized)
        return -1;
    if (!data_len)
        return 0;
    bytesLeft = DSound_GetBytesLeft(sample);
    if (bytesLeft > sample->currentBufferLength)
        MyAssertHandler(".\\groupvoice\\play_dsound.cpp", 97, 0, "%s", "bytesLeft <= (int)sample->currentBufferLength");
    if (data_len < (sample->dwBufferSize - bytesLeft))
        v4 = data_len;
    else
        v4 = sample->dwBufferSize - bytesLeft;
    if (!v4)
        return 0;
    if (v4 <= 0)
        MyAssertHandler(".\\groupvoice\\play_dsound.cpp", 106, 0, "%s", "bytesToUpload > 0");
    if (v4 >= sample->dwBufferSize)
        MyAssertHandler(".\\groupvoice\\play_dsound.cpp", 107, 0, "%s", "bytesToUpload < (int)sample->dwBufferSize");
    hr = sample->DSB->Lock(sample->currentOffset, v4, &pLock1, &dwLockLen1, &pLock2, &dwLockLen2, 0);
    if (hr >= 0)
    {
        dataOffset = 0;
        if (pLock1)
        {
            memcpy(pLock1, data, dwLockLen1);
            dataOffset = dwLockLen1;
        }
        if (pLock2)
        {
            memcpy(pLock2, &data[dataOffset], dwLockLen2);
            dataOffset += dwLockLen2;
        }
        hra = sample->DSB->Unlock(pLock1, dwLockLen1, pLock2, dwLockLen2);
        if (hra >= 0)
        {
            if (sample->currentBufferLength > sample->dwBufferSize)
                MyAssertHandler(
                    ".\\groupvoice\\play_dsound.cpp",
                    175,
                    0,
                    "%s",
                    "sample->currentBufferLength <= sample->dwBufferSize");
            if (sample->currentBufferLength < sample->dwBufferSize)
            {
                sample->currentBufferLength += dataOffset;
                if (sample->currentBufferLength > sample->dwBufferSize)
                    sample->currentBufferLength = sample->dwBufferSize;
            }
            sample->bytesBuffered += dataOffset;
            if (sample->bytesBuffered > sample->currentBufferLength)
                MyAssertHandler(
                    ".\\groupvoice\\play_dsound.cpp",
                    187,
                    0,
                    "%s",
                    "sample->bytesBuffered <= (int)sample->currentBufferLength");
            if (sample->playing)
                sample->lastOffset = sample->currentOffset;
            sample->currentOffset = (dataOffset + sample->currentOffset) % sample->dwBufferSize;
            if (v4 + bytesLeft < DSound_GetBytesLeft(sample))
                MyAssertHandler(".\\groupvoice\\play_dsound.cpp", 196, 0, "%s", "bytesLeft + bytesToUpload >= tempBytesLeft");
            return dwLockLen2 + dwLockLen1;
        }
        else
        {
            if (hra == -2147024809)
            {
                Com_Printf(9, "DSERR_INVALIDPARAM\n");
            }
            else if (hra == -2005401550)
            {
                Com_Printf(9, "DSERR_INVALIDCALL\n");
            }
            Com_Printf(9, "Error trying to unlock sample!\n");
            return -1;
        }
    }
    else
    {
        if (hr > -2005401530)
        {
            if (hr == -2005401450)
                Com_Printf(9, "DSERR_BUFFERLOST\n");
        }
        else
        {
            switch (hr)
            {
            case -2005401530:
                Com_Printf(9, "DSERR_PRIOLEVELNEEDED\n");
                break;
            case -2147024809:
                Com_Printf(9, "DSERR_INVALIDPARAM\n");
                Com_Printf(9, " Offset : %i, length: %i\n", sample->currentOffset, v4);
                break;
            case -2005401550:
                Com_Printf(9, "DSERR_INVALIDCALL\n");
                break;
            }
        }
        Com_Printf(9, "Error trying to lock sample!\n");
        return -1;
    }
}

void __cdecl DSound_AdjustSamplePlayback(dsound_sample_t *sample, int bytesLeft)
{
    unsigned __int8 playMode; // [esp+20h] [ebp-8h]

    if (bytesLeft <= 0)
        MyAssertHandler(".\\groupvoice\\play_dsound.cpp", 207, 0, "%s", "bytesLeft > 0");
    if (bytesLeft >= MIN_COMFORTABLE_BUFFER_AMOUNT)
    {
        if (bytesLeft <= MAX_COMFORTABLE_BUFFER_AMOUNT)
        {
            if (!sample->playMode && bytesLeft <= COMFORTABLE_BUFFER_AMOUNT
                || sample->playMode == 1 && bytesLeft >= COMFORTABLE_BUFFER_AMOUNT)
            {
                sample->playMode = 2;
            }
        }
        else
        {
            sample->playMode = 0;
        }
    }
    else
    {
        sample->playMode = 1;
    }
    playMode = sample->playMode;
    if (playMode)
    {
        if (playMode == 1)
            sample->DSB->SetFrequency((sample->frequency - sample->frequency * maxScalingDownMultiplier));
        else
            sample->DSB->SetFrequency(sample->frequency);
    }
    else
    {
        sample->DSB->SetFrequency((sample->frequency + sample->frequency * maxScalingUpMultiplier));
    }
}

bool __cdecl DSound_BufferUnderrunOccurred(dsound_sample_t *sample)
{
    return sample->bytesBuffered <= 0;
}

void __cdecl DSound_HandleBufferUnderrun(dsound_sample_t *sample)
{
    DSound_GetBytesLeft(sample);
    sample->DSB->Stop();
    sample->currentBufferLength = 0;
    sample->currentOffset = 0;
    sample->lastOffset = 0;
    sample->lastPlayPos = 0;
    if (sample->stopPosition)
        sample->stopPosition -= sample->currentOffset;
    else
        sample->stopPosition = 0;
    sample->DSB->SetCurrentPosition(sample->currentOffset);
    sample->lastOffset = sample->currentOffset;
    sample->playing = 0;
    sample->bytesBuffered = 0;
}

void __cdecl DSound_SampleFrame(dsound_sample_t *sample)
{
    DWORD dwWritePos; // [esp+4h] [ebp-14h] BYREF
    HRESULT hr; // [esp+8h] [ebp-10h]
    int bytesLeft; // [esp+Ch] [ebp-Ch]
    DWORD dwPlayPos; // [esp+10h] [ebp-8h] BYREF
    int bytesPlayed; // [esp+14h] [ebp-4h]

    bytesLeft = DSound_GetBytesLeft(sample);
    if (!sample->playing && bytesLeft >= COMFORTABLE_BUFFER_AMOUNT)
    {
        hr = sample->DSB->Play(0, 0, 1u);
        if (hr < 0)
        {
            Com_Printf(9, "Error: Failed to play DirectSound play buffer (%i)!\n", hr);
            return;
        }
        hr = sample->DSB->SetCurrentPosition(sample->lastPlayPos);
        if (hr < 0)
            Com_Printf(9, "Error: Failed to set current position to %i when playing sound buffer!\n", sample->lastOffset);
        sample->playing = 1;
        sample->playMode = 2;
    }
    if (sample->playing)
    {
        hr = sample->DSB->GetCurrentPosition(&dwPlayPos, &dwWritePos);
        if (hr < 0)
            Com_Printf(9, "Error: Failed to get cursor positions \n");
        dwWritePos = sample->currentOffset;
        if (sample->stopPosition < 0)
        {
            if (DSound_BufferUnderrunOccurred(sample))
                goto LABEL_22;
        }
        else if (sample->lastPlayPos > dwPlayPos)
        {
            if (sample->stopPosition >= sample->lastPlayPos || sample->stopPosition <= dwPlayPos)
                goto LABEL_15;
        }
        else if (sample->lastPlayPos < sample->stopPosition && dwPlayPos >= sample->stopPosition)
        {
        LABEL_15:
            DSound_StopSample(sample);
            return;
        }
        DSound_AdjustSamplePlayback(sample, bytesLeft);
        bytesPlayed = dwPlayPos - sample->lastPlayPos;
        if (bytesPlayed < 0)
            bytesPlayed = dwPlayPos + sample->currentBufferLength - sample->lastPlayPos;
        if (bytesPlayed < 0)
            MyAssertHandler(".\\groupvoice\\play_dsound.cpp", 368, 0, "%s", "bytesPlayed >= 0");
        sample->bytesBuffered -= bytesPlayed;
        if (!DSound_BufferUnderrunOccurred(sample))
        {
            sample->lastPlayPos = dwPlayPos;
            return;
        }
    LABEL_22:
        DSound_HandleBufferUnderrun(sample);
    }
}

HRESULT __cdecl CreateBasicBuffer(
    IDirectSound8 *lpDirectSound,
    IDirectSoundBuffer **ppDsb,
    unsigned int sampleRate,
    unsigned __int16 channels,
    unsigned int bufferSize)
{
    HRESULT hr; // [esp+0h] [ebp-40h]
    _DSBUFFERDESC dsbdesc; // [esp+4h] [ebp-3Ch] BYREF
    tWAVEFORMATEX wfx; // [esp+2Ch] [ebp-14h] BYREF

    wfx.cbSize = 0;
    wfx.wFormatTag = 1;
    wfx.nChannels = channels;
    wfx.wBitsPerSample = 16;
    wfx.nSamplesPerSec = sampleRate;
    wfx.nBlockAlign = 16 * channels / 8;
    wfx.nAvgBytesPerSec = sampleRate * wfx.nBlockAlign;
    dsbdesc.dwReserved = 0;
    memset(&dsbdesc.guid3DAlgorithm, 0, sizeof(dsbdesc.guid3DAlgorithm));
    dsbdesc.dwSize = 36;
    dsbdesc.dwFlags = 33000;
    dsbdesc.dwBufferBytes = bufferSize;
    dsbdesc.lpwfxFormat = &wfx;
    hr = (lpds->CreateSoundBuffer)(&dsbdesc, ppDsb, 0);
    if (hr < 0)
        Com_Printf(9, "Failed to create sound buffer!\n");
    return hr;
}

dsound_sample_t *__cdecl DSound_NewSample()
{
    dsound_sample_t *sample; // [esp+4h] [ebp-4h]

    if (!dsoundplay_initialized)
        return 0;
    sample = DSOUNDRecord_NewSample();
    sample->dwBufferSize = g_sound_playBufferSize;
    if (CreateBasicBuffer(lpds, &sample->DSB, sample->frequency, sample->channels, g_sound_playBufferSize) >= 0)
        return sample;
    Com_Printf(9, "Error: Failed to create DirectSound play buffer\n");
    sample->DSB->Release();
    sample->DSB = 0;
    return 0;
}

char __cdecl DSound_StopSample(dsound_sample_t *sample)
{
    if (!dsoundplay_initialized)
        return 0;
    if (!sample->playing)
        return 0;
    if (sample->DSB)
        sample->DSB->Stop();
    sample->stopPosition = -1;
    sample->channel = -1;
    sample->playing = 0;
    if (sample->DSB->SetCurrentPosition(0) < 0)
        Com_Printf(9, "Error: Failed to set current position to %i when playing sound buffer!\n", sample->lastOffset);
    return 1;
}

int __cdecl DSound_Init(bool callDsoundInit, HWND__ *handle)
{
    if (dsoundplay_initialized)
        return 1;
    if (DirectSoundCreate8(0, &lpds, 0) >= 0)
    {
        if (lpds->SetCooperativeLevel(handle, 2u) >= 0)
        {
            dsoundplay_initialized = 1;
            return 1;
        }
        else
        {
            Com_Printf(9, "Error: [play] failure when calling SetCooperativeLevel()\n");
            return 0;
        }
    }
    else
    {
        Com_Printf(9, "ERROR: [play] failed to initialize DirectSound\n");
        return 0;
    }
}

void __cdecl DSound_Shutdown()
{
    if (dsoundplay_initialized)
    {
        lpds->Release();
        dsoundplay_initialized = 0;
    }
}

