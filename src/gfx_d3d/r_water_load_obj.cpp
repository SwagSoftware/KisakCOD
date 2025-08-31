#include "r_water.h"
#include <qcommon/qcommon.h>
#include <universal/q_shared.h>
#include "r_image.h"



int sceneWaterMapSetupsCount;
water_t sceneWaterMapSetups[16];

BOOL __cdecl R_WatersEquivalent(const water_t *w0, const water_t *w1)
{
    float v3; // [esp+0h] [ebp-4Ch]
    float v4; // [esp+4h] [ebp-48h]
    float v5; // [esp+8h] [ebp-44h]
    float v6; // [esp+Ch] [ebp-40h]
    float v7; // [esp+10h] [ebp-3Ch]
    float v8; // [esp+14h] [ebp-38h]
    float v9; // [esp+18h] [ebp-34h]
    float v10; // [esp+1Ch] [ebp-30h]
    float v11; // [esp+20h] [ebp-2Ch]
    float v12; // [esp+28h] [ebp-24h]
    float v13; // [esp+30h] [ebp-1Ch]
    float v14; // [esp+3Ch] [ebp-10h]
    float v15; // [esp+40h] [ebp-Ch]
    float v16; // [esp+44h] [ebp-8h]
    float fDirAngleCos; // [esp+48h] [ebp-4h]

    iassert( w0 );
    iassert( w1 );
    if (w0->M != w1->M || w0->N != w1->N)
        return 0;
    if (w1->Lx != w0->Lx || w1->Lz != w0->Lz)
        return 0;
    v16 = w0->amplitude - w1->amplitude;
    v10 = I_fabs(v16);
    if (v10 > EQUAL_EPSILON)
        return 0;
    v15 = w0->gravity - w1->gravity;
    v9 = I_fabs(v15);
    if (v9 > 0.1)
        return 0;
    v14 = w0->windvel - w1->windvel;
    v8 = I_fabs(v14);
    if (v8 > 0.1)
        return 0;
    v13 = w0->winddir[0] * w1->winddir[0] + w0->winddir[1] * w1->winddir[1];
    v12 = w0->winddir[0] * w0->winddir[0] + w0->winddir[1] * w0->winddir[1];
    v11 = w1->winddir[0] * w1->winddir[0] + w1->winddir[1] * w1->winddir[1];
    v7 = v11 * v12;
    v6 = sqrt(v7);
    v5 = v6 + 1.0e-10;
    fDirAngleCos = v13 / v5;
    v4 = fDirAngleCos - 1.0;
    v3 = I_fabs(v4);
    return v3 <= EQUAL_EPSILON;
}

void __cdecl R_ShutdownLoadWater()
{
    sceneWaterMapSetupsCount = 0;
}

void __cdecl R_PickWaterFrequencies(water_t *water)
{
    float v1; // [esp+0h] [ebp-68h]
    float v2; // [esp+4h] [ebp-64h]
    float v3; // [esp+8h] [ebp-60h]
    float v4; // [esp+Ch] [ebp-5Ch]
    float v5; // [esp+10h] [ebp-58h]
    float v6; // [esp+14h] [ebp-54h]
    int m; // [esp+28h] [ebp-40h]
    float m_scale; // [esp+2Ch] [ebp-3Ch]
    float Ph; // [esp+30h] [ebp-38h]
    float kz; // [esp+34h] [ebp-34h]
    float windfactor; // [esp+38h] [ebp-30h]
    float L_sqrd; // [esp+3Ch] [ebp-2Ch]
    float K_sqrd; // [esp+40h] [ebp-28h]
    float windvelsqrd; // [esp+44h] [ebp-24h]
    float n_scale; // [esp+48h] [ebp-20h]
    int n; // [esp+4Ch] [ebp-1Ch]
    float scale; // [esp+50h] [ebp-18h]
    int i; // [esp+54h] [ebp-14h]
    float kx; // [esp+58h] [ebp-10h]
    complex_s E; // [esp+5Ch] [ebp-Ch] BYREF
    float w_sqrd; // [esp+64h] [ebp-4h]

    iassert( water );
    windvelsqrd = water->windvel * water->windvel;
    L_sqrd = windvelsqrd * windvelsqrd / water->gravity;
    n_scale = 6.283185482025146 / ((double)water->N * water->Lx);
    m_scale = 6.283185482025146 / ((double)water->M * water->Lz);
    i = 0;
    for (n = -water->N / 2; n < water->N / 2; ++n)
    {
        kx = (double)n * n_scale;
        for (m = -water->M / 2; m < water->M / 2; ++m)
        {
            kz = (double)m * m_scale;
            GaussianRandom(&E.real, &E.imag);
            K_sqrd = kx * kx + kz * kz;
            v6 = sqrt(K_sqrd);
            w_sqrd = water->gravity * v6;
            windfactor = water->winddir[0] * kx + water->winddir[1] * kz;
            if (windfactor > 0.0)
            {
                v5 = -1.0 / (L_sqrd * K_sqrd);
                v4 = exp(v5);
                Ph = water->amplitude * v4 / (K_sqrd * K_sqrd * K_sqrd) * (windfactor * windfactor);
                v3 = Ph * 0.5;
                v2 = sqrt(v3);
                scale = v2 * water->amplitude;
                water->H0[i].real = E.real * scale;
                water->H0[i].imag = E.imag * scale;
                v1 = sqrt(w_sqrd);
                water->wTerm[i] = v1;
            }
            else
            {
                water->H0[i].real = 0.0;
                water->H0[i].imag = 0.0;
                water->wTerm[i] = 0.0;
            }
            ++i;
        }
    }
}

GfxImage *__cdecl R_CreateWaterMap(char *name, unsigned __int16 imageWidth, unsigned __int16 imageHeight)
{
    GfxImage *image; // [esp+0h] [ebp-4h]

    image = Image_Alloc(name, 5u, 0xBu, 9u);
    iassert( image );
    image->width = imageWidth;
    image->height = imageHeight;
    Image_BuildWaterMap(image);
    return image;
}

void __cdecl R_CreateWaterSetup(const water_t *source, int waterMapSetupIndex, water_t *destination)
{
    char *v3; // eax
    unsigned __int16 M; // [esp-8h] [ebp-18h]
    unsigned __int16 N; // [esp-4h] [ebp-14h]
    GfxImage *image; // [esp+8h] [ebp-8h]
    int elementCount; // [esp+Ch] [ebp-4h]

    iassert( source );
    iassert( destination );
    elementCount = source->N * source->M;
    memcpy(destination, source, sizeof(water_t));
    destination->H0 = (complex_s *)Material_Alloc(8 * elementCount);
    destination->wTerm = (float *)Material_Alloc(4 * elementCount);
    R_PickWaterFrequencies(destination);
    N = source->N;
    M = source->M;
    v3 = va("watersetup%i", waterMapSetupIndex);
    image = R_CreateWaterMap(v3, M, N);
    iassert( image );
    destination->image = image;
}

water_t *__cdecl R_LoadWaterSetup(const water_t *water)
{
    int waterMapSetupIndex; // [esp+0h] [ebp-4h]

    iassert( IsPowerOf2( water->N ) );
    iassert( IsPowerOf2( water->M ) );
    if (water->M < 4 || water->M > 64)
        MyAssertHandler(
            ".\\r_water_load_obj.cpp",
            143,
            0,
            "water->M not in [MIN_WATER_SIZE, MAX_WATER_SIZE]\n\t%i not in [%i, %i]",
            water->M,
            4,
            64);
    if (water->N < 4 || water->N > 64)
        MyAssertHandler(
            ".\\r_water_load_obj.cpp",
            144,
            0,
            "water->N not in [MIN_WATER_SIZE, MAX_WATER_SIZE]\n\t%i not in [%i, %i]",
            water->N,
            4,
            64);
    iassert( water->Lx > 0 );
    iassert( water->Lz > 0 );
    iassert( water->gravity > 0 );
    iassert( water->windvel > 0 );
    iassert( water->winddir[0] || water->winddir[1] );
    iassert( water->amplitude > 0 );
    for (waterMapSetupIndex = 0; waterMapSetupIndex < sceneWaterMapSetupsCount; ++waterMapSetupIndex)
    {
        if (R_WatersEquivalent(&sceneWaterMapSetups[waterMapSetupIndex], water))
            return &sceneWaterMapSetups[waterMapSetupIndex];
    }
    if (waterMapSetupIndex == 16)
    {
        Com_PrintError(8, "ERROR: map uses more than %i waterMap textures\n", 16);
        return 0;
    }
    else
    {
        R_CreateWaterSetup(water, waterMapSetupIndex, &sceneWaterMapSetups[waterMapSetupIndex]);
        ++sceneWaterMapSetupsCount;
        return &sceneWaterMapSetups[waterMapSetupIndex];
    }
}

void __cdecl R_InitLoadWater()
{
    iassert( sceneWaterMapSetupsCount == 0 );
}