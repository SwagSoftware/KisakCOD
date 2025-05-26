#include "snd_local.h"
#include "snd_public.h"
#include <universal/com_files.h>
#include <qcommon/qcommon.h>
#include <universal/com_memory.h>

unsigned int __stdcall MSS_FileOpenCallback(const MSS_FILE *pszFilename, UINTa *phFileHandle)
{
    return (FS_FOpenFileReadStream(pszFilename, (int *)phFileHandle) & 0x80000000) == 0;
}
void __stdcall MSS_FileCloseCallback(UINTa hFileHandle)
{
    FS_FCloseFile(hFileHandle);
}
int __stdcall MSS_FileSeekCallback(UINTa hFileHandle, int offset, unsigned int type)
{
    if (type)
    {
        if (type == 1)
        {
            FS_Seek(hFileHandle, offset, 0);
        }
        else
        {
            if (type != 2)
                return 0;
            FS_Seek(hFileHandle, offset, 1);
        }
    }
    else
    {
        FS_Seek(hFileHandle, offset, 2);
    }
    return FS_FTell(hFileHandle);
}
unsigned int __stdcall MSS_FileReadCallback(UINTa hFileHandle, void *pBuffer, unsigned int bytes)
{
    return FS_Read((unsigned char *)pBuffer, bytes, hFileHandle);
}

_DIG_DRIVER *__cdecl MSS_open_digital_driver(int hertz, int bits, int channels)
{
  bool v4; // [esp+0h] [ebp-10h]
  int outputChannels; // [esp+4h] [ebp-Ch] BYREF
  _DIG_DRIVER *driver; // [esp+8h] [ebp-8h]
  MSS_MC_SPEC mcSpec; // [esp+Ch] [ebp-4h] BYREF

  AIL_set_preference(1, 53);
  driver = (_DIG_DRIVER *)AIL_open_digital_driver(hertz, bits, channels, 0);
  if ( driver )
  {
    AIL_speaker_configuration(driver, 0, &outputChannels, 0, &mcSpec);
    v4 = outputChannels > 2 || !outputChannels;
    milesGlob.isMultiChannel = v4;
    if ( channels == 16 && mcSpec == MSS_MC_STEREO )
    {
      AIL_shutdown();
      if ( !MSS_Startup() )
      {
        MSS_InitFailed();
        return 0;
      }
      AIL_set_preference(1, 53);
      return (_DIG_DRIVER *)AIL_open_digital_driver(hertz, bits, 32, 0);
    }
  }
  return driver;
}

void MSS_InitFailed()
{
  if ( Dvar_GetInt("r_vc_compile") != 2 )
    Com_Printf(9, "Miles sound system initialization failed\n");
}

MSS_MC_SPEC mss_spec[5] =
{
    MSS_MC_USE_SYSTEM_CONFIG,
    MSS_MC_MONO,
    MSS_MC_HEADPHONES,
    MSS_MC_40_DISCRETE,
    MSS_MC_51_DISCRETE
};

char __cdecl MSS_Init()
{
  const char *error; // eax
  int integer; // [esp+4h] [ebp-Ch]
  int hertz; // [esp+8h] [ebp-8h]

  integer = snd_khz->current.integer;
  if ( integer == 11 )
  {
    hertz = 11025;
  }
  else
  {
    if ( integer != 22 )
    {
      if ( integer == 44 )
      {
        hertz = 44100;
        goto LABEL_8;
      }
      Com_Printf(9, "invalid value %i for snd_khz, using 22 khz instead\n", snd_khz->current.integer);
    }
    hertz = 22050;
  }
LABEL_8:
  Com_Printf(
    9,
    "Attempting %i kHz %i bit [%s] sound\n",
    hertz / 1000,
    16,
    snd_outputConfigurationStrings[snd_outputConfiguration->current.integer]);
  milesGlob.driver = MSS_open_digital_driver(hertz, 16, mss_spec[snd_outputConfiguration->current.integer]);
  if ( milesGlob.driver )
  {
    AIL_set_3D_distance_factor(milesGlob.driver, 0.0254);
    AIL_set_3D_rolloff_factor(milesGlob.driver, 0.0);
    AIL_set_speaker_configuration(milesGlob.driver, 0, 0, 3.0);
    g_snd.Initialized2d = 1;
    g_snd.Initialized3d = 1;
    g_snd.max_2D_channels = 8;
    g_snd.max_3D_channels = 32;
    g_snd.max_stream_channels = 13;
    g_snd.playback_rate = hertz + hertz / 2;
    if ( g_snd.playback_rate >= 0xAC44 )
      g_snd.playback_rate = 0x7FFFFFFF;
    g_snd.playback_channels = (mss_spec[snd_outputConfiguration->current.integer] != MSS_MC_MONO) + 1;
    g_snd.timescale = 1.0;
    return 1;
  }
  else
  {
    error = (const char *)AIL_last_error();
    Com_PrintError(9, "ERROR: Couldn't initialize digital driver: %s\n", error);
    return 0;
  }
}

void MSS_InitChannels()
{
  int i; // [esp+0h] [ebp-4h]

  for ( i = 0; i < g_snd.max_3D_channels + g_snd.max_2D_channels; ++i )
  {
    milesGlob.handle_sample[i] = (_SAMPLE *)AIL_allocate_sample_handle(milesGlob.driver);
    if ( !milesGlob.handle_sample[i] )
      Com_Error(ERR_DROP, "MILES sound sample allocation failed on channel %i", i + 1);
    //AIL_init_sample(milesGlob.handle_sample[i], 1, 0);
    AIL_init_sample(milesGlob.handle_sample[i], 1);
  }
  g_snd.ambient_track = 1;
}

void MSS_InitEq()
{
  int channelIndex; // [esp+4h] [ebp-10h]
  int band; // [esp+8h] [ebp-Ch]
  SndEqParams *params; // [esp+Ch] [ebp-8h]
  int eqIndex; // [esp+10h] [ebp-4h]

  milesGlob.eqFilter = 0;
  for ( eqIndex = 0; eqIndex < 2; ++eqIndex )
  {
    for ( band = 0; band < 3; ++band )
    {
      for ( channelIndex = 0; channelIndex < 64; ++channelIndex )
      {
        params = &milesGlob.eq[eqIndex].params[band][channelIndex];
        params->enabled = 0;
        params->freq = 20000.0;
        params->gain = 1.0;
        params->q = 1.0;
        params->type = SND_EQTYPE_FIRST;
      }
    }
  }
  // LWSS ADD
  HPROENUM itr = HPROENUM_FIRST;
  HPROVIDER provider;
  char *filterName = NULL;
  int filterCount = 0;
  while (AIL_enumerate_filters(&itr, &provider, &filterName))
  {
      Com_Printf(19, "[%d]Found filter (%s)\n", filterCount, filterName);
      filterCount++;
  }
  // LWSS END

  if ( AIL_find_filter("Parametric EQ Filter", &milesGlob.eqFilter) )
  {
    AIL_open_filter(milesGlob.eqFilter, milesGlob.driver);
  }
  else
  {
    milesGlob.eqFilter = 0;
    Com_PrintError(9, "ERROR: unable to load eq filter.\n");
  }
}

bool __cdecl MSS_Startup()
{
  return AIL_startup() != 0;
}

void MSS_ShutdownCleanup()
{
  //Com_ClearMemTrack();
  memset((unsigned __int8 *)&milesGlob, 0, sizeof(milesGlob));
}

double __cdecl MSS_GetWetLevel(const snd_alias_t *pAlias)
{
  if ( g_snd.effect->wetlevel < 0.0 || g_snd.effect->wetlevel > 1.0 )
    MyAssertHandler(
      ".\\win32\\snd_driver.cpp",
      188,
      0,
      "%s\n\t(g_snd.effect->wetlevel) = %g",
      "(g_snd.effect->wetlevel >= 0 && g_snd.effect->wetlevel <= 1)",
      g_snd.effect->wetlevel);
  if ( !pAlias )
    return g_snd.effect->wetlevel;
  if ( !snd_enableReverb->current.enabled || (pAlias->flags & 0x10) != 0 )
    return (float)0.0;
  else
    return g_snd.effect->wetlevel;
}

const char *MSS_EQ_ENABLED[3] = { "Enable 0", "Enable 1", "Enable 2" }; // idb
const char *MSS_EQ_FREQ[3] = { "Freq 0", "Freq 1", "Freq 2" }; // idb
const char *MSS_EQ_TYPE[3] = { "Type 0", "Type 1", "Type 2" }; // idb
const char *MSS_EQ_GAIN[3] = { "Gain 0", "Gain 1", "Gain 2" }; // idb
const char *MSS_EQ_Q[3] = { "Q 0", "Q 1", "Q 2" }; // idb

void __cdecl MSS_ApplyEqFilter(_SAMPLE *s, int entchannel)
{
  int enabled; // [esp+0h] [ebp-14h] BYREF
  int band; // [esp+4h] [ebp-10h]
  SAMPLESTAGE stage; // [esp+8h] [ebp-Ch]
  SndEqParams *params; // [esp+Ch] [ebp-8h]
  int eqIndex; // [esp+10h] [ebp-4h]
  
  if ( snd_enableEq->current.enabled && milesGlob.eqFilter )
  {
    AIL_set_sample_processor(s, SP_FILTER, milesGlob.eqFilter);
    AIL_set_sample_processor(s, SP_FILTER_1, milesGlob.eqFilter);
    stage = SP_FILTER;
    for (eqIndex = 0; eqIndex < 2; ++eqIndex)
    {
        for (band = 0; band < 3; ++band)
        {
            params = &milesGlob.eq[eqIndex].params[band][entchannel];
            AIL_sample_stage_property(s, stage, MSS_EQ_ENABLED[band], -1, 0, &enabled, 0);
            if (enabled)
            {
                AIL_sample_stage_property(s, stage, MSS_EQ_TYPE[band], -1, 0, params, 0);
                AIL_sample_stage_property(s, stage, MSS_EQ_FREQ[band], -1, 0, &params->freq, 0);
                AIL_sample_stage_property(s, stage, MSS_EQ_GAIN[band], -1, 0, &params->gain, 0);
                AIL_sample_stage_property(s, stage, MSS_EQ_Q[band],    -1, 0, &params->q, 0);
            }
        }
        stage = SP_FILTER_1;
    }
  }
}

void __cdecl MSS_ResumeSample(int i, int frametime)
{
  int v2; // [esp+0h] [ebp-8h]

  if ( g_snd.chaninfo[i].startDelay )
  {
    if ( g_snd.chaninfo[i].startDelay - frametime > 0 )
      v2 = g_snd.chaninfo[i].startDelay - frametime;
    else
      v2 = 0;
    g_snd.chaninfo[i].startDelay = v2;
    if ( !g_snd.chaninfo[i].startDelay )
      AIL_resume_sample(milesGlob.handle_sample[i]);
  }
}

_DIG_DRIVER *__cdecl MSS_GetDriver()
{
  return milesGlob.driver;
}

int __cdecl MSS_DigitalFormatType(int waveFormat, int bits, int channels)
{
  int digitalFormat; // [esp+0h] [ebp-4h]

  if ( waveFormat != 1 && waveFormat != 17 )
    Com_Error(ERR_FATAL, "unknown wave format %i", waveFormat);
  if ( channels != 1 && channels != 2 )
    Com_Error(ERR_FATAL, "Sound has %i channels; only 1 or 2 channels are supported.\n", channels);
  if ( bits != 8 && bits != 16 )
    Com_Error(ERR_FATAL, "Sound uses %i bits per channel; only 8 or 16 bit channels are supported.\n", bits);
  digitalFormat = 0;
  if ( waveFormat == 17 )
    digitalFormat = 4;
  if ( bits == 16 )
    digitalFormat |= 1u;
  if ( channels == 2 )
    return digitalFormat | 2;
  return digitalFormat;
}

unsigned __int8 *__cdecl MSS_Alloc(unsigned int bytes, unsigned int rate)
{
  if ( useFastFile->current.enabled )
    return (unsigned __int8 *)((int (__cdecl *)(unsigned int, unsigned int))MSS_Alloc_FastFile)(bytes, rate);
  else
    return MSS_Alloc_LoadObj(bytes, rate);
}

unsigned __int8 *__cdecl MSS_Alloc_LoadObj(unsigned int bytes, unsigned int rate)
{
  int min_Spec_bytes; // [esp+0h] [ebp-4h]

  min_Spec_bytes = bytes;
  while ( rate > 0x4099 )
  {
    rate >>= 1;
    min_Spec_bytes /= 2;
  }
  //track_hunk_alloc((min_Spec_bytes + 31) & 0xFFFFFFE0, 0x7FFFFFFF, "MSS_Alloc", 16);
  return Hunk_Alloc(bytes, "MSS_Alloc", 15);
}

unsigned int *__cdecl MSS_Alloc_FastFile(int bytes)
{
  return (unsigned int *)Z_Malloc(bytes, "MSS_Alloc", 15);
}

