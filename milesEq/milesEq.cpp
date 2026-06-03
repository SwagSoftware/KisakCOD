//############################################################################
//##                                                                        ##
//##  milesEq.cpp -- "3 Band Parm Eq" pipeline filter for Miles 7.2e        ##
//##                                                                        ##
//##  Clean re-implementation of the CoD3 custom EQ filter (originally      ##
//##  c:\trees\cod3\cod3src\milesEq\) against the Miles Sound System 7.2e   ##
//##  SDK, so the pipeline-filter ABI matches the 7.2e mss32.dll KisakCOD   ##
//##  ships. The legacy milesEq.flt was built for an older Miles; under the ##
//##  7.2e mixer it walked off the sample buffer -> AV inside milesEq.flt   ##
//##  (crash on coup with EQ'd streamed sounds).                            ##
//##                                                                        ##
//##  Registers provider "3 Band Parm Eq" exposing per-band sample          ##
//##  properties (N = 0..2), exactly as KisakCOD MSS_ApplyEqFilter /        ##
//##  SND_SetEqParams drive them:                                           ##
//##      "Enable N" (int)  "Type N" (int, SND_EQTYPE)                       ##
//##      "Freq N"  (float) "Gain N" (float, dB) "Q N" (float)              ##
//##                                                                        ##
//##  Each band is a typed biquad (RBJ Audio-EQ Cookbook):                  ##
//##      Type 0 LowPass  1 HighPass  2 LowShelf  3 HighShelf  4 Bell       ##
//##                                                                        ##
//##  Build as a standalone .flt (exports RIB_Main): milesEq.cpp + the SDK  ##
//##  ribdll.c + radlibc.c, link mss32.lib, /Gz (stdcall), milesEq.def.     ##
//############################################################################

#include "mss.h"
#include "imssapi.h"

#define FILTER_NAME "3 Band Parm Eq"
#define N_BANDS     3

// SND_EQTYPE (KisakCOD snd_public.h)
enum
{
   EQ_LOWPASS   = 0,
   EQ_HIGHPASS  = 1,
   EQ_LOWSHELF  = 2,
   EQ_HIGHSHELF = 3,
   EQ_BELL      = 4
};

//
// Attribute tokens. The first three are required by common.inl's
// PROVIDER_property; the rest are our per-band sample properties, laid out
// 5-per-band in the fixed order Enable, Type, Freq, Gain, Q.
//
enum PROP
{
   _FX_PROVIDER_FLAGS,
   _FX_SAMPLE_PROPERTIES,
   _FX_N_SAMPLE_PROPERTIES,

   _FX_BAND_BASE,                                       // first per-band token
   _FX_BAND_LAST = _FX_BAND_BASE + (N_BANDS * 5) - 1
};

#define PROP_FIELD_ENABLE 0
#define PROP_FIELD_TYPE   1
#define PROP_FIELD_FREQ   2
#define PROP_FIELD_GAIN   3
#define PROP_FIELD_Q      4
#define PROP_ID(band,field) ( _FX_BAND_BASE + (band) * 5 + (field) )

//
// One DRIVERSTATE per HDIGDRIVER (generic).
//
struct DRIVERSTATE
{
   HDIGDRIVER dig;
};

//
// One biquad per band: parameters, normalized coefficients (a0 == 1), and
// Direct-Form-I history.
//
struct BAND
{
   S32 enabled;
   S32 type;
   F32 freq;
   F32 gain;            // dB
   F32 q;

   F32 b0, b1, b2, a1, a2;
   F32 x1, x2, y1, y2;
};

//
// One SAMPLESTATE per HSAMPLE.
//
struct SAMPLESTATE
{
   HSAMPLE          sample;
   DRIVERSTATE FAR *driver;

   S32  calculated_rate;
   BAND band[N_BANDS];
};

#include "common.inl"   // FLT_startup/shutdown/open_driver/close_driver,
                        // FLTSMP_open_sample/close_sample, PROVIDER_property,
                        // FX_CLIPRANGE, F_PI, _FX_DENORMVAL ...

//############################################################################
//#  Compute one band's normalized biquad coefficients (RBJ cookbook).       #
//############################################################################

static void CalcBand( SAMPLESTATE FAR *SS, S32 n, S32 playback_rate )
{
   BAND FAR *b = &SS->band[n];

   F32 Fs = (F32) playback_rate;
   if ( Fs < 1.0F ) Fs = 1.0F;

   F32 f0 = b->freq;
   FX_CLIPRANGE( f0, 20.0F, Fs * 0.5F - 1.0F );

   F32 Q = b->q;
   if ( Q < 0.01F ) Q = 0.01F;

   F32 w0    = 2.0F * F_PI * f0 / Fs;
   F32 cw    = (F32) AIL_cos( w0 );
   F32 sw    = (F32) AIL_sin( w0 );
   F32 alpha = sw / ( 2.0F * Q );

   F32 A   = (F32) AIL_pow( 10.0F, b->gain / 40.0F );     // sqrt of linear gain
   F32 sqA = (F32) AIL_pow( A, 0.5F );
   F32 ta  = 2.0F * sqA * alpha;

   F32 b0, b1, b2, a0, a1, a2;

   switch ( b->type )
      {
      case EQ_LOWPASS:
         b0 = ( 1.0F - cw ) * 0.5F;  b1 = 1.0F - cw;  b2 = ( 1.0F - cw ) * 0.5F;
         a0 = 1.0F + alpha;          a1 = -2.0F * cw; a2 = 1.0F - alpha;
         break;

      case EQ_HIGHPASS:
         b0 = ( 1.0F + cw ) * 0.5F;  b1 = -( 1.0F + cw ); b2 = ( 1.0F + cw ) * 0.5F;
         a0 = 1.0F + alpha;          a1 = -2.0F * cw;     a2 = 1.0F - alpha;
         break;

      case EQ_LOWSHELF:
         b0 =        A * ( ( A + 1.0F ) - ( A - 1.0F ) * cw + ta );
         b1 = 2.0F * A * ( ( A - 1.0F ) - ( A + 1.0F ) * cw );
         b2 =        A * ( ( A + 1.0F ) - ( A - 1.0F ) * cw - ta );
         a0 =            ( A + 1.0F ) + ( A - 1.0F ) * cw + ta;
         a1 =  -2.0F *  ( ( A - 1.0F ) + ( A + 1.0F ) * cw );
         a2 =            ( A + 1.0F ) + ( A - 1.0F ) * cw - ta;
         break;

      case EQ_HIGHSHELF:
         b0 =         A * ( ( A + 1.0F ) + ( A - 1.0F ) * cw + ta );
         b1 = -2.0F * A * ( ( A - 1.0F ) + ( A + 1.0F ) * cw );
         b2 =         A * ( ( A + 1.0F ) + ( A - 1.0F ) * cw - ta );
         a0 =             ( A + 1.0F ) - ( A - 1.0F ) * cw + ta;
         a1 =   2.0F *  ( ( A - 1.0F ) - ( A + 1.0F ) * cw );
         a2 =             ( A + 1.0F ) - ( A - 1.0F ) * cw - ta;
         break;

      case EQ_BELL:
      default:
         b0 = 1.0F + alpha * A;  b1 = -2.0F * cw;  b2 = 1.0F - alpha * A;
         a0 = 1.0F + alpha / A;  a1 = -2.0F * cw;  a2 = 1.0F - alpha / A;
         break;
      }

   F32 inv = 1.0F / a0;
   b->b0 = b0 * inv;  b->b1 = b1 * inv;  b->b2 = b2 * inv;
   b->a1 = a1 * inv;  b->a2 = a2 * inv;
}

static void init_sample( SAMPLESTATE FAR *SS )
{
   S32 rate = SS->driver->dig->DMA_rate;

   for ( S32 n = 0; n < N_BANDS; n++ )
      {
      BAND FAR *b = &SS->band[n];
      b->enabled = 0;
      b->type    = EQ_LOWPASS;
      b->freq    = 20000.0F;            // matches KisakCOD MSS_InitEq defaults
      b->gain    = 0.0F;
      b->q       = 1.0F;
      b->x1 = b->x2 = b->y1 = b->y2 = 0.0F;
      CalcBand( SS, n, rate );
      }

   SS->calculated_rate = rate;
}

static void close_sample( SAMPLESTATE FAR *SS )
{
   (void) SS;
}

//############################################################################
//#  Process one channel of mono 16-bit samples through the enabled bands.   #
//#                                                                          #
//#  Miles calls this once per channel per mix chunk. As in the SDK Parmeq   #
//#  sample, biquad history is per-band (not per-channel); for game EQ the   #
//#  cross-channel coupling on stereo content is inaudible.                  #
//############################################################################

static void AILCALL FLTSMP_sample_process( HSAMPLESTATE state,
                                           S16 FAR * MSSRESTRICT source_buffer,
                                           S16 FAR * MSSRESTRICT dest_buffer,
                                           S32       n_samples,
                                           S32       dest_playback_rate,
                                           S32       channel )
{
   (void) channel;

   SAMPLESTATE FAR *SSp = (SAMPLESTATE FAR *) state;
   SAMPLESTATE SS;
   AIL_memcpy( &SS, SSp, sizeof( SS ) );

   // Fast path: no band enabled -> straight copy.
   S32 any = 0;
   for ( S32 n = 0; n < N_BANDS; n++ ) any |= SS.band[n].enabled;
   if ( !any )
      {
      if ( source_buffer != dest_buffer )
         AIL_memcpy( dest_buffer, source_buffer, n_samples * 2 );
      return;
      }

   if ( dest_playback_rate != SS.calculated_rate )
      {
      for ( S32 n = 0; n < N_BANDS; n++ ) CalcBand( &SS, n, dest_playback_rate );
      SS.calculated_rate = dest_playback_rate;
      }

   for ( S32 i = 0; i < n_samples; i++ )
      {
      F32 sig = (F32)(S16) LE_SWAP16( source_buffer );
      ++source_buffer;

      for ( S32 n = 0; n < N_BANDS; n++ )
         {
         BAND FAR *b = &SS.band[n];
         if ( !b->enabled ) continue;

         F32 in  = sig;
         F32 out = b->b0 * in + b->b1 * b->x1 + b->b2 * b->x2
                              - b->a1 * b->y1 - b->a2 * b->y2;

         b->x2 = b->x1;  b->x1 = in;
         b->y2 = b->y1;  b->y1 = out + _FX_DENORMVAL;
         sig = out;
         }

      S32 tmp;
      WRITE_MONO_SAMPLE( dest_buffer, sig, tmp );
      }

   AIL_memcpy( SSp, &SS, sizeof( *SSp ) );
}

//############################################################################
//#  Get / set a per-band sample property (returns previous value).          #
//############################################################################

static S32 AILCALL FLTSMP_sample_property( HSAMPLESTATE state, HPROPERTY property,
                                           void FAR       *before_value,
                                           void const FAR *new_value,
                                           void FAR       *after_value )
{
   SAMPLESTATE FAR *SS = (SAMPLESTATE FAR *) state;

   if ( (S32) property < _FX_BAND_BASE || (S32) property > _FX_BAND_LAST )
      return 0;

   S32 rel   = (S32) property - _FX_BAND_BASE;
   S32 n     = rel / 5;
   S32 field = rel % 5;
   BAND FAR *b = &SS->band[n];

   switch ( field )
      {
      case PROP_FIELD_ENABLE:
         if ( before_value ) *(S32 FAR*) before_value = b->enabled;
         if ( new_value )    b->enabled = ( *(const S32 FAR*) new_value ) ? 1 : 0;
         if ( after_value )  *(S32 FAR*) after_value = b->enabled;
         return 1;

      case PROP_FIELD_TYPE:
         if ( before_value ) *(S32 FAR*) before_value = b->type;
         if ( new_value )
            {
            S32 t = *(const S32 FAR*) new_value;
            FX_CLIPRANGE( t, EQ_LOWPASS, EQ_BELL );
            b->type = t;
            CalcBand( SS, n, SS->calculated_rate );
            }
         if ( after_value )  *(S32 FAR*) after_value = b->type;
         return 1;

      case PROP_FIELD_FREQ:
         if ( before_value ) *(F32 FAR*) before_value = b->freq;
         if ( new_value )
            {
            b->freq = *(const F32 FAR*) new_value;
            FX_CLIPRANGE( b->freq, 20.0F, ((F32) SS->driver->dig->DMA_rate) * 0.5F - 1.0F );
            CalcBand( SS, n, SS->calculated_rate );
            }
         if ( after_value )  *(F32 FAR*) after_value = b->freq;
         return 1;

      case PROP_FIELD_GAIN:
         if ( before_value ) *(F32 FAR*) before_value = b->gain;
         if ( new_value )
            {
            b->gain = *(const F32 FAR*) new_value;
            FX_CLIPRANGE( b->gain, -24.0F, 24.0F );
            CalcBand( SS, n, SS->calculated_rate );
            }
         if ( after_value )  *(F32 FAR*) after_value = b->gain;
         return 1;

      case PROP_FIELD_Q:
         if ( before_value ) *(F32 FAR*) before_value = b->q;
         if ( new_value )
            {
            b->q = *(const F32 FAR*) new_value;
            FX_CLIPRANGE( b->q, 0.01F, 100.0F );
            CalcBand( SS, n, SS->calculated_rate );
            }
         if ( after_value )  *(F32 FAR*) after_value = b->q;
         return 1;
      }

   return 0;
}

//############################################################################
//#  Provider registration entry. Miles' redist-dir auto-loader finds the     #
//#  provider via GetProcAddress("_RIB_Main@8") -- the DECORATED __stdcall     #
//#  name. (Verified: stock mssdsp.flt and the old CoD3 milesEq.flt both       #
//#  export exactly `_RIB_Main@8`, NOT undecorated `RIB_Main`.) Exporting the  #
//#  undecorated name (e.g. via a .def) makes LoadLibrary succeed but the      #
//#  auto-loader's GetProcAddress("_RIB_Main@8") return NULL, so Miles         #
//#  rejects/unloads the filter and it never reaches AIL_enumerate_filters.    #
//#                                                                            #
//#  Fix: __declspec(dllexport) on this extern "C" __stdcall fn (AILEXPORT ==  #
//#  __stdcall on Win32) exports the decorated `_RIB_Main@8` -- no .def needed.#
//#  mss.h declared it DXDEC=dllimport, so MSVC emits a benign C4273 here and  #
//#  the dllexport wins. We deliberately do NOT define __RADINDLL__ (that      #
//#  would flip every AIL_* declaration to dllexport as well).                 #
//############################################################################

__declspec(dllexport) S32 AILEXPORT RIB_Main( HPROVIDER provider_handle, U32 up_down )
{
   const RIB_INTERFACE_ENTRY FLT1[] =
      {
      REG_FN( PROVIDER_property ),
      REG_PR( "Name",     PROVIDER_NAME,     (RIB_DATA_SUBTYPE)( RIB_STRING | RIB_READONLY ) ),
      REG_PR( "Version",  PROVIDER_VERSION,  (RIB_DATA_SUBTYPE)( RIB_HEX    | RIB_READONLY ) ),
      REG_PR( "Flags",    _FX_PROVIDER_FLAGS,(RIB_DATA_SUBTYPE)( RIB_HEX    | RIB_READONLY ) ),
      };

   const RIB_INTERFACE_ENTRY FLT2[] =
      {
      REG_FN( FLT_startup ),
      REG_FN( FLT_error ),
      REG_FN( FLT_shutdown ),
      REG_FN( FLT_open_driver ),
      };

   const RIB_INTERFACE_ENTRY FLT3[] =
      {
      REG_FN( FLT_close_driver ),
      REG_FN( FLT_premix_process ),
      REG_FN( FLT_postmix_process ),
      };

   const RIB_INTERFACE_ENTRY FLTSMP1[] =
      {
      REG_FN( FLTSMP_open_sample ),
      REG_FN( FLTSMP_close_sample ),
      REG_FN( FLTSMP_sample_process ),
      REG_FN( FLTSMP_sample_property ),
      };

   const RIB_INTERFACE_ENTRY FLTSMP2[] =
      {
      REG_PR( "Enable 0", PROP_ID(0,PROP_FIELD_ENABLE), RIB_DEC   ),
      REG_PR( "Type 0",   PROP_ID(0,PROP_FIELD_TYPE),   RIB_DEC   ),
      REG_PR( "Freq 0",   PROP_ID(0,PROP_FIELD_FREQ),   RIB_FLOAT ),
      REG_PR( "Gain 0",   PROP_ID(0,PROP_FIELD_GAIN),   RIB_FLOAT ),
      REG_PR( "Q 0",      PROP_ID(0,PROP_FIELD_Q),      RIB_FLOAT ),

      REG_PR( "Enable 1", PROP_ID(1,PROP_FIELD_ENABLE), RIB_DEC   ),
      REG_PR( "Type 1",   PROP_ID(1,PROP_FIELD_TYPE),   RIB_DEC   ),
      REG_PR( "Freq 1",   PROP_ID(1,PROP_FIELD_FREQ),   RIB_FLOAT ),
      REG_PR( "Gain 1",   PROP_ID(1,PROP_FIELD_GAIN),   RIB_FLOAT ),
      REG_PR( "Q 1",      PROP_ID(1,PROP_FIELD_Q),      RIB_FLOAT ),

      REG_PR( "Enable 2", PROP_ID(2,PROP_FIELD_ENABLE), RIB_DEC   ),
      REG_PR( "Type 2",   PROP_ID(2,PROP_FIELD_TYPE),   RIB_DEC   ),
      REG_PR( "Freq 2",   PROP_ID(2,PROP_FIELD_FREQ),   RIB_FLOAT ),
      REG_PR( "Gain 2",   PROP_ID(2,PROP_FIELD_GAIN),   RIB_FLOAT ),
      REG_PR( "Q 2",      PROP_ID(2,PROP_FIELD_Q),      RIB_FLOAT ),
      };

   if ( up_down )
      {
      RIB_register( provider_handle, "MSS pipeline filter",             FLT1 );
      RIB_register( provider_handle, "MSS pipeline filter",             FLT2 );
      RIB_register( provider_handle, "MSS pipeline filter",             FLT3 );
      RIB_register( provider_handle, "Pipeline filter sample services", FLTSMP1 );
      RIB_register( provider_handle, "Pipeline filter sample services", FLTSMP2 );
      }
   else
      {
      RIB_unregister_all( provider_handle );
      }

   return TRUE;
}
