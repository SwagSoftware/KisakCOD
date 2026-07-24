#include "snd_local.h"
#include "snd_public.h"
#include <qcommon/qcommon.h>
#include <portaudio.h>
#include <cstring>
#include <cmath>
#include <Windows.h>
#include <universal/com_memory.h>

PaLocal paGlob;
const dvar_t* snd_khz;
const dvar_t* snd_outputConfiguration;

static int PA_AudioCallback(
    const void* input, void* output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    (void)input; (void)timeInfo; (void)statusFlags;
    float* out = (float*)output;
    PaLocal* pa = (PaLocal*)userData;

    memset(out, 0, frameCount * PA_OUTPUT_CHANNELS * sizeof(float));

    for (int i = 0; i < 53; i++) {
        PaChannelState* ch = &pa->channels[i];
        if (!ch->playing || ch->paused) continue;

        if (ch->stream) {
            // --- Streaming channel: drain the ring buffer ---
            PaStreamState* s = ch->stream;
            for (unsigned long f = 0; f < frameCount; f++) {
                if ((s->writePos - s->readPos) <= 0) {
                    if (s->stopThread)
                        ch->playing = false;  // fill thread exhausted, stream done
                    break;
                }
                int r = s->readPos % PA_RING_FRAMES;
                out[f * 2 + 0] += s->ring[r * 2 + 0] * ch->gain * ch->panL;
                out[f * 2 + 1] += s->ring[r * 2 + 1] * ch->gain * ch->panR;
                s->readPos++;
            }
        }
        else {
            // --- Loaded sound: resample from pcmData into output ---
            // Advance pcmPos by (pitch * srcRate / outputRate) per output frame
            float step = ch->pitch * ((float)ch->srcRate / (float)PA_OUTPUT_RATE);

            for (unsigned long f = 0; f < frameCount; f++) {
                int pos0 = (int)ch->pcmPos;
                if (pos0 >= ch->pcmFrames) {
                    ch->playing = false;
                    break;
                }

                // Linear interpolation between adjacent frames
                int   pos1 = pos0 + 1 < ch->pcmFrames ? pos0 + 1 : pos0;
                float frac = ch->pcmPos - (float)pos0;

                if (ch->srcChannels == 2) {
                    float l0 = ch->pcmData[pos0 * 2 + 0] / 32768.0f;
                    float r0 = ch->pcmData[pos0 * 2 + 1] / 32768.0f;
                    float l1 = ch->pcmData[pos1 * 2 + 0] / 32768.0f;
                    float r1 = ch->pcmData[pos1 * 2 + 1] / 32768.0f;
                    out[f * 2 + 0] += (l0 + frac * (l1 - l0)) * ch->gain * ch->panL;
                    out[f * 2 + 1] += (r0 + frac * (r1 - r0)) * ch->gain * ch->panR;
                }
                else {
                    float s0 = ch->pcmData[pos0] / 32768.0f;
                    float s1 = ch->pcmData[pos1] / 32768.0f;
                    float s = s0 + frac * (s1 - s0);
                    out[f * 2 + 0] += s * ch->gain * ch->panL;
                    out[f * 2 + 1] += s * ch->gain * ch->panR;
                }

                ch->pcmPos += step;
                if ((int)ch->pcmPos >= ch->pcmFrames) {
                    if (ch->looping)
                        ch->pcmPos = fmodf(ch->pcmPos, (float)ch->pcmFrames);
                    else {
                        ch->playing = false;
                        break;
                    }
                }
            }
        }
    }

    // Clamp � summing many channels can exceed 1.0
    for (unsigned long s = 0; s < frameCount * PA_OUTPUT_CHANNELS; s++) {
        if (out[s] > 1.0f) out[s] = 1.0f;
        if (out[s] < -1.0f) out[s] = -1.0f;
    }

    return paContinue;
}

static void PA_StreamFillThread(int param) {
    int si = (int)(intptr_t)param;
    PaStreamState* s = &paGlob.streamStates[si];

    const int CHUNK = 1024;            // frames to decode per iteration
    int16_t tmp[CHUNK * 2];            // max stereo

    while (!s->stopThread) {
        int avail = PA_RING_FRAMES - (s->writePos - s->readPos);
        if (avail < CHUNK) 
        { 
            Sleep(5); 
            continue; 
        }

        s->lock.lock();
        drmp3_uint64 got = 0;
        if (s->drType == DR_TYPE_WAV)
            got = drwav_read_pcm_frames_s16(&s->dr.wav, CHUNK, tmp);
        else if (s->drType == DR_TYPE_MP3)
            got = drmp3_read_pcm_frames_s16(&s->dr.mp3, CHUNK, tmp);

        if (got == 0 && s->looping) 
        {
            if (s->drType == DR_TYPE_WAV)
            {
                drwav_seek_to_pcm_frame(&s->dr.wav, 0);
                got = drwav_read_pcm_frames_s16(&s->dr.wav, CHUNK, tmp);
            }
            else if (s->drType == DR_TYPE_MP3)
            {
                drmp3_seek_to_pcm_frame(&s->dr.mp3, 0);
                got = drmp3_read_pcm_frames_s16(&s->dr.mp3, CHUNK, tmp);
            }
        }
        s->lock.unlock();

        if (got == 0) { s->stopThread = true; break; }

        // int ch = (int)s->wav.channels;
        int32_t ch = s->drType == DR_TYPE_WAV ? s->dr.wav.channels : s->dr.mp3.channels;
        for (drmp3_uint64 f = 0; f < got; f++) {
            int w = s->writePos % PA_RING_FRAMES;
            if (ch == 2) {
                s->ring[w * 2 + 0] = tmp[f * 2 + 0] / 32768.0f;
                s->ring[w * 2 + 1] = tmp[f * 2 + 1] / 32768.0f;
            }
            else {
                float mono = tmp[f] / 32768.0f;
                s->ring[w * 2 + 0] = mono;
                s->ring[w * 2 + 1] = mono;
            }
            s->writePos++;
        }
    }
    /*return 0;*/
}

void PA_StartStreamFillThread(int si) {
    PaStreamState* s = &paGlob.streamStates[si];
    s->writePos   = 0;
    s->readPos    = 0;
    s->stopThread = false;
    s->active     = true;
    s->fillThread = std::thread(PA_StreamFillThread, si); //CreateThread(NULL, 0, PA_StreamFillThread, (LPVOID)(intptr_t)si, 0, NULL);
}

void PA_StopStreamFillThread(int si) {
    PaStreamState* s = &paGlob.streamStates[si];
    if (s->active) {
        s->stopThread = true;
        // WaitForSingleObject(s->fillThread, 2000);
        if (s->fillThread.joinable())
            s->fillThread.join();

        if (s->drType == DR_TYPE_WAV)
            drwav_uninit(&s->dr.wav);
        else if (s->drType == DR_TYPE_MP3)
            drmp3_uninit(&s->dr.mp3);

        s->drType == DR_TYPE_NONE;

        if (s->data)
            Z_Free(s->data, 15);
        s->data = nullptr;
        s->dataLength = 0;

        s->active = false;
    }
}

bool PA_Startup() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        Com_PrintError(9, "PortAudio Pa_Initialize failed: %s\n", Pa_GetErrorText(err));
        return false;
    }
    return true;
}

bool PA_Init() {
    PaStreamParameters out;
    out.device = Pa_GetDefaultOutputDevice();
    out.channelCount = PA_OUTPUT_CHANNELS;
    out.sampleFormat = paFloat32;
    out.suggestedLatency = Pa_GetDeviceInfo(out.device)->defaultLowOutputLatency;
    out.hostApiSpecificStreamInfo = NULL;

    PaError err = Pa_OpenStream(
        &paGlob.stream,
        NULL, &out,
        PA_OUTPUT_RATE,
        paFramesPerBufferUnspecified,
        paNoFlag,
        PA_AudioCallback,
        &paGlob);

    if (err != paNoError) {
        Com_PrintError(9, "PortAudio Pa_OpenStream failed: %s\n", Pa_GetErrorText(err));
        return false;
    }

    err = Pa_StartStream(paGlob.stream);
    if (err != paNoError) {
        Com_PrintError(9, "PortAudio Pa_StartStream failed: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(paGlob.stream);
        return false;
    }

    g_snd.Initialized2d = 1; 
    g_snd.Initialized3d = 1;
    g_snd.max_2D_channels = 8;
    g_snd.max_3D_channels = 32;
    g_snd.max_stream_channels = 13;
    g_snd.playback_rate = PA_OUTPUT_RATE;
    g_snd.timescale = 1.0f;

    for (int i = 0; i < 13; i++) {
        paGlob.channels[40 + i].stream = &paGlob.streamStates[i];
    }

    Com_Printf(9, "PortAudio: %s � device: %s\n",
        Pa_GetVersionText(),
        Pa_GetDeviceInfo(out.device)->name);
    return true;
}

void PA_InitChannels() {
    g_snd.ambient_track = 1;
}

void PA_InitEq() {
    for (int ei = 0; ei < 2; ei++)
        for (int b = 0; b < 3; b++)
            for (int c = 0; c < 64; c++) {
                SndEqParams& p = paGlob.eq[ei].params[b][c];
                p.enabled = 0;
                p.freq = 20000.0f;
                p.gain = 1.0f;
                p.q = 1.0f;
                p.type = SND_EQTYPE_FIRST;
            }
}

void PA_ShutdownCleanup() {
    for (int i = 0; i < 13; i++) {
        PaStreamState* s = &paGlob.streamStates[i];
        if (s->active) {
            s->stopThread = true;
            if (s->fillThread.joinable())
                s->fillThread.join();

            if (s->drType == DR_TYPE_WAV)
                drwav_uninit(&s->dr.wav);
            else if (s->drType == DR_TYPE_MP3)
                drmp3_uninit(&s->dr.mp3);

            s->drType == DR_TYPE_NONE;

            if (s->data)
                Z_Free(s->data, 15);
            s->data = nullptr;
            s->dataLength = 0;
        }
    }
    if (paGlob.stream) {
        Pa_StopStream(paGlob.stream);
        Pa_CloseStream(paGlob.stream);
    }
    Pa_Terminate();
    memset(&paGlob, 0, sizeof(paGlob));
}
