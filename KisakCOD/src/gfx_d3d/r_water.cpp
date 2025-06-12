#include "r_water.h"
#include <qcommon/mem_track.h>
#include "r_image.h"
#include <universal/fft.h>
#include "r_dvars.h"
#include <qcommon/qcommon.h>
#include <universal/profile.h>

WaterGlob waterGlob;
WaterGlobStatic waterGlobStatic;

//unsigned int const *const g_selectByteFromInt__uint4 820e3810     gfx_d3d : r_water.obj
long volatile g_waterLock;//          85b3ac80     gfx_d3d : r_water.obj
//struct __vector4 const g_selectByteFromInt 85b45900     gfx_d3d : r_water.obj


void __cdecl TRACK_r_water()
{
    track_static_alloc_internal(&waterGlob, 36864, "waterGlob", 18);
}

void __cdecl R_UploadWaterTextureInternal(water_t **data)
{
    water_t *water; // [esp+68h] [ebp-4h]

    Profile_Begin(119);
    water = *data;
    Profile_Begin(122);
    WaterFrequenciesAtTime(waterGlob.H, water, water->writable.floatTime);
    Profile_EndInternal(0);
    Profile_Begin(123);
    WaterAmplitudesFromFrequencies(waterGlob.H, water);
    Profile_EndInternal(0);
    Profile_Begin(124);
    WaterPixelsFromAmplitudes((GfxColor *)waterGlob.pixels, waterGlob.H, water);
    Profile_EndInternal(0);
    Profile_Begin(127);
    GenerateMipMaps(D3DFMT_L8, waterGlob.pixels, water);
    Profile_EndInternal(0);
    Profile_EndInternal(0);
    InterlockedExchangeAdd(&g_waterLock, -1);
}

void __cdecl WaterFrequenciesAtTime(complex_s *H, const water_t *water, float t)
{
    __int64 v3; // [esp+0h] [ebp-44h]
    float sinReal; // [esp+28h] [ebp-1Ch]
    int vecKCount; // [esp+2Ch] [ebp-18h]
    int vecKIndex; // [esp+30h] [ebp-14h]
    float *wTerm; // [esp+34h] [ebp-10h]
    complex_s *H0; // [esp+38h] [ebp-Ch]
    float sinImag; // [esp+3Ch] [ebp-8h]
    float ta; // [esp+54h] [ebp+10h]

    if (!H)
        MyAssertHandler(".\\r_water.cpp", 154, 0, "%s", "H");
    if (!water)
        MyAssertHandler(".\\r_water.cpp", 155, 0, "%s", "water");
    ta = t * 162.9746551513672;
    vecKCount = water->N * water->M;
    if (vecKCount > 4096)
        MyAssertHandler(".\\r_water.cpp", 160, 0, "%s\n\t(vecKCount) = %i", "(vecKCount <= (64 * 64))", vecKCount);
    vecKIndex = 0;
    wTerm = water->wTerm;
    H0 = water->H0;
    while (vecKIndex < vecKCount)
    {
        if (*(unsigned int *)wTerm)
        {
            v3 = (__int64)(*wTerm * ta);
            sinReal = waterGlobStatic.sinTable[((v3 & 0x3FF) + 255) & 0x3FF];
            sinImag = waterGlobStatic.sinTable[v3 & 0x3FF];
            iassert(!IS_NAN(H0->real));
            iassert(!IS_NAN(H0->imag));
            iassert(!IS_NAN(sinReal));
            iassert(!IS_NAN(sinImag));

            H[vecKIndex].real = H0->real * sinReal;
            H[vecKIndex].imag = H0->imag * sinImag;

            iassert(!IS_NAN(H[vecKIndex].real));
            iassert(!IS_NAN(H[vecKIndex].imag));
        }
        else
        {
            H[vecKIndex].real = 0.0;
            H[vecKIndex].imag = 0.0;
        }
        ++vecKIndex;
        ++wTerm;
        ++H0;
    }
}

void __cdecl WaterAmplitudesFromFrequencies(complex_s *H, const water_t *water)
{
    int fftIndex; // [esp+0h] [ebp-Ch]
    int fftIndexa; // [esp+0h] [ebp-Ch]
    unsigned int log2_m; // [esp+4h] [ebp-8h]
    int waterIndex; // [esp+8h] [ebp-4h]
    int waterIndexa; // [esp+8h] [ebp-4h]

    if (!H)
        MyAssertHandler(".\\r_water.cpp", 215, 0, "%s", "H");
    if (!water)
        MyAssertHandler(".\\r_water.cpp", 216, 0, "%s", "water");
    if (water->M != water->N)
        MyAssertHandler(".\\r_water.cpp", 217, 0, "%s", "water->M == water->N");
    for (log2_m = 0; water->M != 1 << log2_m; ++log2_m)
        ;
    for (waterIndex = 0; waterIndex < water->N; ++waterIndex)
    {
        fftIndex = water->M * waterIndex;
        if (fftIndex < 0)
            MyAssertHandler(".\\r_water.cpp", 227, 0, "%s", "fftIndex >= 0");
        if (fftIndex >= 4096)
            MyAssertHandler(".\\r_water.cpp", 228, 0, "%s", "fftIndex < HCOUNT");
        FFT(&H[fftIndex], log2_m, waterGlobStatic.fftBitswap, waterGlobStatic.fftTrigTable);
    }
    TransposeArray(H, water->M);
    for (waterIndexa = 0; waterIndexa < water->M; ++waterIndexa)
    {
        fftIndexa = water->N * waterIndexa;
        if (fftIndexa < 0)
            MyAssertHandler(".\\r_water.cpp", 239, 0, "%s", "fftIndex >= 0");
        if (fftIndexa >= 4096)
            MyAssertHandler(".\\r_water.cpp", 240, 0, "%s", "fftIndex < HCOUNT");
        FFT(&H[fftIndexa], log2_m, waterGlobStatic.fftBitswap, waterGlobStatic.fftTrigTable);
    }
    TransposeArray(H, water->M);
}

void __cdecl TransposeArray(complex_s *H, unsigned int M)
{
    unsigned int v2; // edx
    float real; // ecx
    float imag; // edx
    unsigned int v5; // eax
    unsigned int j; // [esp+4h] [ebp-10h]
    complex_s temp; // [esp+8h] [ebp-Ch]
    unsigned int i; // [esp+10h] [ebp-4h]

    for (i = 0; i < M; ++i)
    {
        for (j = 0; j < i; ++j)
        {
            temp = H[j + M * i];
            v2 = i + M * j;
            real = H[v2].real;
            imag = H[v2].imag;
            v5 = j + M * i;
            H[v5].real = real;
            H[v5].imag = imag;
            H[i + M * j] = temp;
        }
    }
}

void __cdecl WaterPixelsFromAmplitudes(GfxColor *pixels, complex_s *H, const water_t *water)
{
    float v3; // [esp+Ch] [ebp-9Ch]
    float v4; // [esp+10h] [ebp-98h]
    float v5; // [esp+20h] [ebp-88h]
    float v6; // [esp+24h] [ebp-84h]
    float v7; // [esp+34h] [ebp-74h]
    float v8; // [esp+38h] [ebp-70h]
    float v9; // [esp+48h] [ebp-60h]
    float v10; // [esp+4Ch] [ebp-5Ch]
    float v11; // [esp+50h] [ebp-58h]
    float v12; // [esp+54h] [ebp-54h]
    float v13; // [esp+58h] [ebp-50h]
    float v14; // [esp+5Ch] [ebp-4Ch]
    float v15; // [esp+68h] [ebp-40h]
    float v16; // [esp+6Ch] [ebp-3Ch]
    float v17; // [esp+70h] [ebp-38h]
    float v18; // [esp+74h] [ebp-34h]
    float v19; // [esp+78h] [ebp-30h]
    float v20; // [esp+7Ch] [ebp-2Ch]
    float v21; // [esp+80h] [ebp-28h]
    float v22; // [esp+84h] [ebp-24h]
    unsigned int ixy; // [esp+98h] [ebp-10h]
    unsigned int ixya; // [esp+98h] [ebp-10h]
    float dz; // [esp+9Ch] [ebp-Ch]
    GfxColor color; // [esp+A0h] [ebp-8h]
    unsigned int count; // [esp+A4h] [ebp-4h]

    if (!pixels)
        MyAssertHandler(".\\r_water.cpp", 257, 0, "%s", "pixels");
    if (!H)
        MyAssertHandler(".\\r_water.cpp", 258, 0, "%s", "H");
    if (water->M != water->N)
        MyAssertHandler(".\\r_water.cpp", 259, 0, "%s", "water->M == water->N");
    count = water->N * water->M;
    if ((count & 3) != 0)
        MyAssertHandler(".\\r_water.cpp", 262, 0, "%s\n\t(count) = %i", "(!(count & 3))", count);
    dz = 1.0 / (double)count;
    ixy = 0;
    while (ixy < count)
    {
        v22 = H[ixy].imag * H[ixy].imag + H[ixy].real * H[ixy].real;
        v14 = sqrt(v22);
        ixya = ixy + 1;
        v21 = H[ixya].imag * H[ixya].imag + H[ixya].real * H[ixya].real;
        v13 = sqrt(v21);
        ++ixya;
        v20 = H[ixya].imag * H[ixya].imag + H[ixya].real * H[ixya].real;
        v12 = sqrt(v20);
        ++ixya;
        v19 = H[ixya].imag * H[ixya].imag + H[ixya].real * H[ixya].real;
        v11 = sqrt(v19);
        ixy = ixya + 1;
        v18 = dz * v14;
        v10 = v18 - 1.0;
        if (v10 < 0.0)
            v9 = dz * v14;
        else
            v9 = 1.0;
        v17 = dz * v13;
        v8 = v17 - 1.0;
        if (v8 < 0.0)
            v7 = dz * v13;
        else
            v7 = 1.0;
        v16 = dz * v12;
        v6 = v16 - 1.0;
        if (v6 < 0.0)
            v5 = dz * v12;
        else
            v5 = 1.0;
        v15 = dz * v11;
        v4 = v15 - 1.0;
        if (v4 < 0.0)
            v3 = dz * v11;
        else
            v3 = 1.0;
        color.array[3] = (int)(v3 * 255.9989929199219);
        color.array[0] = (int)(v9 * 255.9989929199219);
        color.array[1] = (int)(v7 * 255.9989929199219);
        color.array[2] = (int)(v5 * 255.9989929199219);
        pixels->packed = color.packed;
        ++pixels;
    }
}

void __cdecl GenerateMipMaps(_D3DFORMAT format, unsigned __int8 *pixels, water_t *water)
{
    int srcWidth; // [esp+44h] [ebp-8h]
    unsigned int mipIndex; // [esp+48h] [ebp-4h]

    if (!pixels)
        MyAssertHandler(".\\r_water.cpp", 592, 0, "%s", "pixels");
    if (!water)
        MyAssertHandler(".\\r_water.cpp", 593, 0, "%s", "water");
    if (water->M != water->N)
        MyAssertHandler(".\\r_water.cpp", 594, 0, "%s", "water->M == water->N");
    Profile_Begin(126);
    Image_UploadData(water->image, D3DFMT_L8, D3DCUBEMAP_FACE_POSITIVE_X, 0, waterGlob.pixels);
    Profile_EndInternal(0);
    srcWidth = water->M;
    mipIndex = 1;
    while (srcWidth > 1)
    {
        R_DownsampleMipMapBilinear(pixels, srcWidth, srcWidth, 1, pixels);
        Profile_Begin(126);
        Image_UploadData(water->image, format, D3DCUBEMAP_FACE_POSITIVE_X, mipIndex, pixels);
        Profile_EndInternal(0);
        srcWidth >>= 1;
        ++mipIndex;
    }
}

void __cdecl R_UploadWaterTexture(water_t *water, float floatTime)
{
    if (!water)
        MyAssertHandler(".\\r_water.cpp", 703, 0, "%s", "water");
    if (floatTime != water->writable.floatTime)
    {
        water->writable.floatTime = floatTime;
        R_UploadWaterTextureInternal(&water);
    }
}

void __cdecl R_InitWater()
{
    float v0; // [esp+0h] [ebp-Ch]
    float v1; // [esp+4h] [ebp-8h]
    int tableIndex; // [esp+8h] [ebp-4h]

    for (tableIndex = 0; tableIndex < 1024; ++tableIndex)
    {
        v1 = (double)tableIndex * 0.3515625 * 0.01745329238474369;
        v0 = sin(v1);
        waterGlobStatic.sinTable[tableIndex] = v0;
    }
    FFT_Init(waterGlobStatic.fftBitswap, waterGlobStatic.fftTrigTable);
}

void __cdecl Load_PicmipWater(water_t **waterRef)
{
    complex_s *H0; // edx
    float real; // ecx
    float imag; // edx
    complex_s *v4; // eax
    int v5; // [esp+4h] [ebp-2Ch]
    int v6; // [esp+8h] [ebp-28h]
    int m; // [esp+14h] [ebp-1Ch]
    int srcIndex; // [esp+18h] [ebp-18h]
    int downsample; // [esp+1Ch] [ebp-14h]
    int n; // [esp+24h] [ebp-Ch]
    int dstIndex; // [esp+2Ch] [ebp-4h]

    if ((*waterRef)->M >> r_picmip_water->current.integer < 4)
        v6 = 4;
    else
        v6 = (*waterRef)->M >> r_picmip_water->current.integer;
    if ((*waterRef)->N >> r_picmip_water->current.integer < 4)
        v5 = 4;
    else
        v5 = (*waterRef)->N >> r_picmip_water->current.integer;
    if (v6 != (*waterRef)->M || v5 != (*waterRef)->N)
    {
        if ((*waterRef)->M != (*waterRef)->N)
            MyAssertHandler(".\\r_water.cpp", 746, 0, "%s", "(*waterRef)->M == (*waterRef)->N");
        if (v6 != v5)
            MyAssertHandler(".\\r_water.cpp", 747, 0, "%s", "M == N");
        downsample = (*waterRef)->M / v6;
        (*waterRef)->M = v6;
        (*waterRef)->N = v5;
        srcIndex = 0;
        dstIndex = 0;
        for (m = 0; m < v6; ++m)
        {
            for (n = 0; n < v5; ++n)
            {
                H0 = (*waterRef)->H0;
                real = H0[srcIndex].real;
                imag = H0[srcIndex].imag;
                v4 = (*waterRef)->H0;
                v4[dstIndex].real = real;
                v4[dstIndex++].imag = imag;
                srcIndex += downsample;
            }
            srcIndex += v6 * downsample * (downsample - 1);
        }
    }
}

