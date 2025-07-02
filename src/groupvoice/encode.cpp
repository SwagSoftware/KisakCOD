#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "directsound.h"

#include <speex/speex.h>
#include <qcommon/qcommon.h>
#include <server_mp/server_mp.h>

void *g_encoder;
SpeexBits encodeBits;
int g_speex_initialized;
int g_current_bandwidth_setting;
int g_frame_size;

void __cdecl Encode_SetOptions(int frequency, int quality)
{
    int yes = 1;
    speex_encoder_ctl(g_encoder, SPEEX_SET_SAMPLING_RATE, &frequency);
    speex_encoder_ctl(g_encoder, SPEEX_SET_QUALITY, &quality);
    speex_encoder_ctl(g_encoder, SPEEX_GET_FRAME_SIZE, &g_frame_size);
    speex_encoder_ctl(g_encoder, SPEEX_SET_VAD, &yes);
    speex_encoder_ctl(g_encoder, SPEEX_SET_DTX, &yes);
    g_encoder_quality = quality;
    g_encoder_samplerate = frequency;
}

bool __cdecl Encode_Init(int bandwidthEnum)
{
    void *v1; // eax

    if (bandwidthEnum)
    {
        if (bandwidthEnum == 1)
        {
            v1 = speex_encoder_init(&speex_wb_mode);
        }
        else
        {
            if (bandwidthEnum != 2)
            {
                Com_Printf(9, "Unknown bandwidth mode %i\n", bandwidthEnum);
                return 0;
            }
            v1 = speex_encoder_init(&speex_uwb_mode);
        }
    }
    else
    {
        v1 = speex_encoder_init(&speex_nb_mode);
    }
    g_encoder = v1;
    speex_bits_init(&encodeBits);
    g_speex_initialized = 1;
    Encode_SetOptions(g_encoder_samplerate, g_encoder_quality);
    speex_encoder_ctl(g_encoder, SPEEX_GET_FRAME_SIZE, &g_frame_size);
    g_current_bandwidth_setting = bandwidthEnum;
    return g_encoder != 0;
}

char __cdecl Encode_Shutdown()
{
    if (g_encoder)
        speex_encoder_destroy(g_encoder);
    g_encoder = 0;
    if (g_speex_initialized)
    {
        speex_bits_destroy(&encodeBits);
        g_speex_initialized = 0;
    }
    return 1;
}

int __cdecl Encode_Sample(__int16 *buffer_in, char *buffer_out, int maxLength)
{
    if (sv_voiceQuality->current.integer != g_encoder_quality)
    {
        g_encoder_quality = sv_voiceQuality->current.integer;
        speex_encoder_ctl(g_encoder, SPEEX_SET_QUALITY, &g_encoder_quality);
    }
    speex_bits_reset(&encodeBits);
    speex_encode_int(g_encoder, buffer_in, &encodeBits);
    return speex_bits_write(&encodeBits, buffer_out, maxLength);
}

int __cdecl Encode_GetFrameSize()
{
    return g_frame_size;
}

