#include "com_math.h"

#include <universal/assertive.h>
#include <qcommon/mem_track.h>
#include <math.h>
#include <stdlib.h>
#include "q_shared.h"

//Line 51773:  0006 : 0000bc58       float (*)[3] bytedirs        827bbc58     com_math.obj
//Line 53450 : 0006 : 0291d360       int marker_com_math      850cd360     com_math.obj

float bytedirs[162][3];

double __cdecl AngleDelta(float a1, float a2)
{
    float v4; // [esp+Ch] [ebp-10h]
    float v5; // [esp+10h] [ebp-Ch]
    float v6; // [esp+14h] [ebp-8h]
    float v7; // [esp+18h] [ebp-4h]

    v6 = a1 - a2;
    v7 = v6 * 0.002777777845039964;
    v5 = v7 + 0.5;
    v4 = floor(v5);
    return (float)((v7 - v4) * 360.0);
}

void __cdecl TRACK_com_math()
{
    track_static_alloc_internal(bytedirs, 1944, "bytedirs", 10);
}

double __cdecl random()
{
    return (float)((double)rand() / 32768.0);
}

double __cdecl crandom()
{
    return (float)(random() * 2.0 - 1.0);
}

void __cdecl GaussianRandom(float *f0, float *f1)
{
    float v2; // [esp+0h] [ebp-1Ch]
    float v3; // [esp+4h] [ebp-18h]
    float v4; // [esp+8h] [ebp-14h]
    float x; // [esp+10h] [ebp-Ch]
    float y; // [esp+14h] [ebp-8h]
    float w; // [esp+18h] [ebp-4h]

    if (!f0)
        MyAssertHandler(".\\universal\\com_math.cpp", 167, 0, "%s", "f0");
    if (!f1)
        MyAssertHandler(".\\universal\\com_math.cpp", 168, 0, "%s", "f1");
    do
    {
        x = crandom();
        y = crandom();
        w = x * x + y * y;
    } while (w > 1.0);
    v4 = log(w);
    v3 = v4 * -2.0 / w;
    v2 = sqrt(v3);
    *f0 = x * v2;
    *f1 = y * v2;
}

unsigned int __cdecl RandWithSeed(int *seed)
{
    *seed = 1103515245 * *seed + 12345;
    return *seed / 0x10000 % 0x8000u;
}

void __cdecl PointInCircleFromUniformDeviates(float radiusDeviate, float yawDeviate, float *point)
{
    float v3; // [esp+0h] [ebp-20h]
    float yaw; // [esp+14h] [ebp-Ch]
    float sinYaw; // [esp+18h] [ebp-8h]
    float cosYaw; // [esp+1Ch] [ebp-4h]

    v3 = sqrt(radiusDeviate);
    yaw = yawDeviate * 6.283185482025146;
    cosYaw = cos(yaw);
    sinYaw = sin(yaw);
    *point = v3 * cosYaw;
    point[1] = v3 * sinYaw;
}

double __cdecl LinearTrack(float tgt, float cur, float rate, float deltaTime)
{
    double v4; // st7
    float v7; // [esp+4h] [ebp-18h]
    float v8; // [esp+8h] [ebp-14h]
    float v9; // [esp+Ch] [ebp-10h]
    float err; // [esp+14h] [ebp-8h]
    float step; // [esp+18h] [ebp-4h]

    err = tgt - cur;
    if (err <= 0.0)
        v4 = -rate * deltaTime;
    else
        v4 = rate * deltaTime;
    step = v4;
    v9 = fabs(err);
    if (v9 <= 0.001000000047497451)
        return tgt;
    v8 = fabs(err);
    v7 = fabs(step);
    if (v7 > (double)v8)
        return tgt;
    return (float)(cur + step);
}

double __cdecl LinearTrackAngle(float tgt, float cur, float rate, float deltaTime)
{
    float v6; // [esp+14h] [ebp-10h]
    float v7; // [esp+18h] [ebp-Ch]
    float v8; // [esp+1Ch] [ebp-8h]
    float angle; // [esp+20h] [ebp-4h]

    while (tgt - cur > 180.0)
        tgt = tgt - 360.0;
    while (tgt - cur < -180.0)
        tgt = tgt + 360.0;
    angle = LinearTrack(tgt, cur, rate, deltaTime);
    v8 = angle * 0.002777777845039964;
    v7 = v8 + 0.5;
    v6 = floor(v7);
    return (float)((v8 - v6) * 360.0);
}

double __cdecl DiffTrack(float tgt, float cur, float rate, float deltaTime)
{
    float v6; // [esp+4h] [ebp-18h]
    float v7; // [esp+8h] [ebp-14h]
    float v8; // [esp+Ch] [ebp-10h]
    float err; // [esp+14h] [ebp-8h]
    float step; // [esp+18h] [ebp-4h]

    err = tgt - cur;
    step = rate * err * deltaTime;
    v8 = fabs(err);
    if (v8 <= 0.001000000047497451)
        return tgt;
    v7 = fabs(err);
    v6 = fabs(step);
    if (v6 > (double)v7)
        return tgt;
    return (float)(cur + step);
}

double __cdecl DiffTrackAngle(float tgt, float cur, float rate, float deltaTime)
{
    float v6; // [esp+14h] [ebp-10h]
    float v7; // [esp+18h] [ebp-Ch]
    float v8; // [esp+1Ch] [ebp-8h]
    float angle; // [esp+20h] [ebp-4h]

    while (tgt - cur > 180.0)
        tgt = tgt - 360.0;
    while (tgt - cur < -180.0)
        tgt = tgt + 360.0;
    angle = DiffTrack(tgt, cur, rate, deltaTime);
    v8 = angle * 0.002777777845039964;
    v7 = v8 + 0.5;
    v6 = floor(v7);
    return (float)((v8 - v6) * 360.0);
}

double __cdecl GraphGetValueFromFraction(int knotCount, const float (*knots)[2], float fraction)
{
    float result; // [esp+8h] [ebp-Ch]
    float adjustedFrac; // [esp+Ch] [ebp-8h]
    int knotIndex; // [esp+10h] [ebp-4h]

    result = -1.0;
    if (!knots)
        MyAssertHandler(".\\universal\\com_math.cpp", 460, 0, "%s", "knots");
    if (knotCount < 2)
        MyAssertHandler(".\\universal\\com_math.cpp", 461, 0, "%s\n\t(knotCount) = %i", "(knotCount >= 2)", knotCount);
    if (fraction < 0.0 || fraction > 1.0)
        MyAssertHandler(
            ".\\universal\\com_math.cpp",
            462,
            0,
            "%s\n\t(fraction) = %g",
            "(fraction >= 0.0f && fraction <= 1.0f)",
            fraction);
    if ((*knots)[2 * knotCount - 2] != 1.0)
        MyAssertHandler(
            ".\\universal\\com_math.cpp",
            463,
            0,
            "%s\n\t(knots[knotCount - 1][0]) = %g",
            "(knots[knotCount - 1][0] == 1.0f)",
            (*knots)[2 * knotCount - 2]);
    for (knotIndex = 1; knotIndex < knotCount; ++knotIndex)
    {
        if ((*knots)[2 * knotIndex] >= (double)fraction)
        {
            adjustedFrac = (fraction - (float)(*knots)[2 * knotIndex - 2])
                / ((float)(*knots)[2 * knotIndex] - (float)(*knots)[2 * knotIndex - 2]);
            if (adjustedFrac < 0.0 || adjustedFrac > 1.0)
                MyAssertHandler(
                    ".\\universal\\com_math.cpp",
                    471,
                    0,
                    "%s\n\t(adjustedFrac) = %g",
                    "(adjustedFrac >= 0.0f && adjustedFrac <= 1.0f)",
                    adjustedFrac);
            if ((*knots)[2 * knotIndex - 1] < 0.0 || (*knots)[2 * knotIndex - 1] > 1.0)
                MyAssertHandler(
                    ".\\universal\\com_math.cpp",
                    472,
                    0,
                    "%s\n\t(knots[knotIndex - 1][1]) = %g",
                    "(knots[knotIndex - 1][1] >= 0.0f && knots[knotIndex - 1][1] <= 1.0f)",
                    (*knots)[2 * knotIndex - 1]);
            if ((*knots)[2 * knotIndex + 1] < 0.0 || (*knots)[2 * knotIndex + 1] > 1.0)
                MyAssertHandler(
                    ".\\universal\\com_math.cpp",
                    473,
                    0,
                    "%s\n\t(knots[knotIndex][1]) = %g",
                    "(knots[knotIndex][1] >= 0.0f && knots[knotIndex][1] <= 1.0f)",
                    (*knots)[2 * knotIndex + 1]);
            result = ((float)(*knots)[2 * knotIndex + 1] - (float)(*knots)[2 * knotIndex - 1]) * adjustedFrac
                + (float)(*knots)[2 * knotIndex - 1];
            break;
        }
    }
    if (result < 0.0 || result > 1.0)
        MyAssertHandler(
            ".\\universal\\com_math.cpp",
            480,
            0,
            "%s\n\t(result) = %g",
            "(result >= 0.0f && result <= 1.0f)",
            result);
    return result;
}

double __cdecl Q_acos(float c)
{
    float v2; // [esp+0h] [ebp-8h]

    v2 = acos(c);
    if (v2 > 3.141592741012573)
        return 3.1415927;
    if (v2 >= -3.141592741012573)
        return v2;
    return 3.1415927;
}

char __cdecl ClampChar(int i)
{
    if (i < -128)
        return 0x80;
    if (i <= 127)
        return i;
    return 127;
}

unsigned __int8 __cdecl DirToByte(const float *dir)
{
    float d; // [esp+0h] [ebp-Ch]
    unsigned __int8 best; // [esp+6h] [ebp-6h]
    unsigned __int8 i; // [esp+7h] [ebp-5h]
    float bestd; // [esp+8h] [ebp-4h]

    if (!dir)
        return 0;
    bestd = 0.0;
    best = 0;
    for (i = 0; i < 0xA2u; ++i)
    {
        d = Vec3Dot(dir, bytedirs[i]);
        if (bestd < (double)d)
        {
            bestd = d;
            best = i;
        }
    }
    return best;
}

void __cdecl ByteToDir(unsigned int b, float *dir)
{
    float *v2; // ecx

    if (b < 0xA2)
    {
        *dir = bytedirs[b][0];
        v2 = bytedirs[b];
        dir[1] = v2[1];
        dir[2] = v2[2];
    }
    else
    {
        *dir = 0.0;
        dir[1] = 0.0;
        dir[2] = 0.0;
    }
}

int __cdecl VecNCompareCustomEpsilon(const float *v0, const float *v1, float epsilon, int coordCount)
{
    float v5; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < coordCount; ++i)
    {
        v5 = epsilon * epsilon;
        if (v5 < (v0[i] - v1[i]) * (v0[i] - v1[i]))
            return 0;
    }
    return 1;
}

double __cdecl Vec2Distance(const float *v1, const float *v2)
{
    float dir[2]; // [esp+4h] [ebp-8h] BYREF

    dir[0] = *v2 - *v1;
    dir[1] = v2[1] - v1[1];
    return Vec2Length(dir);
}

double __cdecl Vec2DistanceSq(const float *p1, const float *p2)
{
    float v; // [esp+4h] [ebp-8h]
    float v_4; // [esp+8h] [ebp-4h]

    v = *p2 - *p1;
    v_4 = p2[1] - p1[1];
    return (float)(v * v + v_4 * v_4);
}

void __cdecl Vec3ProjectionCoords(const float *dir, int *xCoord, int *yCoord)
{
    float dirSq[3]; // [esp+0h] [ebp-Ch] BYREF

    Vec3Mul(dir, dir, dirSq);
    if (dirSq[0] > (double)dirSq[2] || dirSq[1] > (double)dirSq[2])
    {
        if (dirSq[0] > (double)dirSq[1] || dirSq[2] > (double)dirSq[1])
        {
            if (*dir <= 0.0)
            {
                *xCoord = 2;
                *yCoord = 1;
            }
            else
            {
                *xCoord = 1;
                *yCoord = 2;
            }
        }
        else if (dir[1] <= 0.0)
        {
            *xCoord = 0;
            *yCoord = 2;
        }
        else
        {
            *xCoord = 2;
            *yCoord = 0;
        }
    }
    else if (dir[2] <= 0.0)
    {
        *xCoord = 1;
        *yCoord = 0;
    }
    else
    {
        *xCoord = 0;
        *yCoord = 1;
    }
}

double __cdecl Vec2Normalize(float *v)
{
    float v2; // [esp+0h] [ebp-14h]
    float v3; // [esp+4h] [ebp-10h]
    float ilength; // [esp+Ch] [ebp-8h]
    float length; // [esp+10h] [ebp-4h]

    length = *v * *v + v[1] * v[1];
    v3 = sqrt(length);
    if (-v3 < 0.0)
        v2 = v3;
    else
        v2 = 1.0;
    ilength = 1.0 / v2;
    *v = *v * ilength;
    v[1] = v[1] * ilength;
    return v3;
}

double __cdecl Vec3NormalizeTo(const float *v, float *out)
{
    float v3; // [esp+0h] [ebp-14h]
    float v4; // [esp+4h] [ebp-10h]
    float ilength; // [esp+Ch] [ebp-8h]
    float length; // [esp+10h] [ebp-4h]

    length = *v * *v + v[1] * v[1] + v[2] * v[2];
    v4 = sqrt(length);
    if (-v4 < 0.0)
        v3 = v4;
    else
        v3 = 1.0;
    ilength = 1.0 / v3;
    *out = *v * ilength;
    out[1] = v[1] * ilength;
    out[2] = v[2] * ilength;
    return v4;
}

double __cdecl Vec2NormalizeTo(const float *v, float *out)
{
    float v3; // [esp+0h] [ebp-14h]
    float v4; // [esp+4h] [ebp-10h]
    float ilength; // [esp+Ch] [ebp-8h]
    float length; // [esp+10h] [ebp-4h]

    length = *v * *v + v[1] * v[1];
    v4 = sqrt(length);
    if (-v4 < 0.0)
        v3 = v4;
    else
        v3 = 1.0;
    ilength = 1.0 / v3;
    *out = *v * ilength;
    out[1] = v[1] * ilength;
    return v4;
}

void __cdecl Vec3Rotate(const float *in, const float (*matrix)[3], float *out)
{
    if (in == out)
        MyAssertHandler(".\\universal\\com_math.cpp", 828, 0, "%s", "in != out");
    *out = *in * (*matrix)[0] + in[1] * (float)(*matrix)[1] + in[2] * (float)(*matrix)[2];
    out[1] = *in * (float)(*matrix)[3] + in[1] * (float)(*matrix)[4] + in[2] * (float)(*matrix)[5];
    out[2] = *in * (float)(*matrix)[6] + in[1] * (float)(*matrix)[7] + in[2] * (float)(*matrix)[8];
}

void __cdecl Vec3RotateTranspose(const float *in, const float (*matrix)[3], float *out)
{
    if (in == out)
        MyAssertHandler(".\\universal\\com_math.cpp", 837, 0, "%s", "in != out");
    *out = *in * (*matrix)[0] + in[1] * (float)(*matrix)[3] + in[2] * (float)(*matrix)[6];
    out[1] = *in * (float)(*matrix)[1] + in[1] * (float)(*matrix)[4] + in[2] * (float)(*matrix)[7];
    out[2] = *in * (float)(*matrix)[2] + in[1] * (float)(*matrix)[5] + in[2] * (float)(*matrix)[8];
}

void __cdecl RotatePointAroundVector(float *dst, const float *dir, const float *point, float degrees)
{
    float m[3][3]; // [esp+1Ch] [ebp-E0h] BYREF
    float rad; // [esp+40h] [ebp-BCh]
    _QWORD vr[3]; // [esp+44h] [ebp-B8h] BYREF
    float zrot_12; // [esp+5Ch] [ebp-A0h]
    float zrot_16; // [esp+60h] [ebp-9Ch]
    int zrot_20; // [esp+64h] [ebp-98h]
    __int64 zrot_24; // [esp+68h] [ebp-94h]
    float zrot_32; // [esp+70h] [ebp-8Ch]
    float rot[3][3]; // [esp+74h] [ebp-88h] BYREF
    float vf[3]; // [esp+98h] [ebp-64h] BYREF
    float vup[3]; // [esp+A4h] [ebp-58h] BYREF
    int i; // [esp+B0h] [ebp-4Ch]
    float tmpmat[3][3]; // [esp+B4h] [ebp-48h] BYREF
    float im[3][3]; // [esp+D8h] [ebp-24h] BYREF

    if (*dir == 0.0 && dir[1] == 0.0 && dir[2] == 0.0)
        MyAssertHandler(".\\universal\\com_math.cpp", 862, 0, "%s", "dir[0] || dir[1] || dir[2]");
    vf[0] = *dir;
    vf[1] = dir[1];
    vf[2] = dir[2];
    PerpendicularVector(dir, (float *)vr);
    Vec3Cross((const float *)vr, vf, vup);
    m[0][0] = *(float *)vr;
    m[1][0] = *((float *)vr + 1);
    m[2][0] = *(float *)&vr[1];
    m[0][1] = vup[0];
    m[1][1] = vup[1];
    m[2][1] = vup[2];
    m[0][2] = vf[0];
    m[1][2] = vf[1];
    m[2][2] = vf[2];
    memcpy(im, m, sizeof(im));
    im[0][1] = *((float *)vr + 1);
    im[0][2] = *(float *)&vr[1];
    im[1][0] = vup[0];
    im[1][2] = vup[2];
    im[2][0] = vf[0];
    im[2][1] = vf[1];
    vr[2] = 0;
    zrot_12 = 0.0;
    zrot_20 = 0;
    zrot_24 = 0;
    zrot_32 = 1.0;
    zrot_16 = 1.0;
    *((float *)&vr[1] + 1) = 1.0;
    rad = degrees * 0.01745329238474369;
    if ((LOunsigned int(rad) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\universal\\com_math.cpp", 897, 0, "%s", "!IS_NAN(rad)");
    *((float *)&vr[1] + 1) = cos(rad);
    *(float *)&vr[2] = sin(rad);
    if ((vr[2] & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\universal\\com_math.cpp", 901, 0, "%s", "!IS_NAN(zrot[0][1])");
    if ((HIunsigned int(vr[1]) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\universal\\com_math.cpp", 902, 0, "%s", "!IS_NAN(zrot[0][0])");
    zrot_12 = -*(float *)&vr[2];
    zrot_16 = *((float *)&vr[1] + 1);
    MatrixMultiply(m, (const float (*)[3])((char *)&vr[1] + 4), tmpmat);
    MatrixMultiply(tmpmat, im, rot);
    for (i = 0; i < 3; ++i)
        dst[i] = rot[i][0] * *point + rot[i][1] * point[1] + rot[i][2] * point[2];
}

void __cdecl Vec3Basis_RightHanded(const float *forward, float *left, float *up)
{
    PerpendicularVector(forward, up);
    Vec3Cross(up, forward, left);
}

double __cdecl vectoyaw(const float *vec)
{
    float v2; // [esp+0h] [ebp-14h]
    float v3; // [esp+4h] [ebp-10h]
    float yawa; // [esp+10h] [ebp-4h]

    if (vec[1] == 0.0 && *vec == 0.0)
    {
        return (float)0.0;
    }
    else
    {
        v3 = atan2(vec[1], *vec);
        yawa = v3 * 180.0 / 3.141592741012573;
        if (yawa < 0.0)
            v2 = 360.0;
        else
            v2 = 0.0;
        return (float)(yawa + v2);
    }
}

double __cdecl vectosignedyaw(const float *vec)
{
    float v2; // [esp+0h] [ebp-10h]
    float yaw; // [esp+Ch] [ebp-4h]

    if (vec[1] == 0.0 && *vec == 0.0)
    {
        return (float)0.0;
    }
    else
    {
        v2 = atan2(vec[1], *vec);
        yaw = v2 * 180.0 / 3.141592741012573;
        if (yaw < -180.0)
            MyAssertHandler(".\\universal\\com_math.cpp", 993, 0, "%s", "yaw >= -180");
        if (yaw > 180.0)
            MyAssertHandler(".\\universal\\com_math.cpp", 994, 0, "%s", "yaw <= 180");
    }
    return yaw;
}

double __cdecl vectopitch(const float *vec)
{
    float v2; // [esp+0h] [ebp-20h]
    float v3; // [esp+4h] [ebp-1Ch]
    float v4; // [esp+8h] [ebp-18h]
    float v6; // [esp+14h] [ebp-Ch]
    float pitcha; // [esp+1Ch] [ebp-4h]

    if (vec[1] == 0.0 && *vec == 0.0)
    {
        if (-vec[2] < 0.0)
            return (float)270.0;
        else
            return (float)90.0;
    }
    else
    {
        v6 = vec[1] * vec[1] + *vec * *vec;
        v4 = sqrt(v6);
        v3 = atan2(vec[2], v4);
        pitcha = v3 * -180.0 / 3.141592741012573;
        if (pitcha < 0.0)
            v2 = 360.0;
        else
            v2 = 0.0;
        return (float)(pitcha + v2);
    }
}

double __cdecl vectosignedpitch(const float *vec)
{
    float v2; // [esp+0h] [ebp-1Ch]
    float v3; // [esp+4h] [ebp-18h]
    float v5; // [esp+10h] [ebp-Ch]

    if (vec[1] == 0.0 && *vec == 0.0)
    {
        if (-vec[2] < 0.0)
            return (float)-90.0;
        else
            return (float)90.0;
    }
    else
    {
        v5 = vec[1] * vec[1] + *vec * *vec;
        v3 = sqrt(v5);
        v2 = atan2(vec[2], v3);
        return (float)(v2 * -180.0 / 3.141592741012573);
    }
}

void __cdecl vectoangles(const float *vec, float *angles)
{
    float v2; // [esp+0h] [ebp-34h]
    float v3; // [esp+4h] [ebp-30h]
    float v4; // [esp+8h] [ebp-2Ch]
    float v5; // [esp+Ch] [ebp-28h]
    float v6; // [esp+10h] [ebp-24h]
    float v7; // [esp+14h] [ebp-20h]
    float v8; // [esp+1Ch] [ebp-18h]
    float yaw; // [esp+28h] [ebp-Ch]
    float yawa; // [esp+28h] [ebp-Ch]
    float pitch; // [esp+30h] [ebp-4h]
    float pitcha; // [esp+30h] [ebp-4h]

    if (vec[1] == 0.0 && *vec == 0.0)
    {
        yaw = 0.0;
        if (-vec[2] < 0.0)
            v7 = 270.0;
        else
            v7 = 90.0;
        pitch = v7;
    }
    else
    {
        v6 = atan2(vec[1], *vec);
        yawa = v6 * 180.0 / 3.141592741012573;
        if (yawa < 0.0)
            v5 = 360.0;
        else
            v5 = 0.0;
        yaw = yawa + v5;
        v8 = vec[1] * vec[1] + *vec * *vec;
        v4 = sqrt(v8);
        v3 = atan2(vec[2], v4);
        pitcha = v3 * -180.0 / 3.141592741012573;
        if (pitcha < 0.0)
            v2 = 360.0;
        else
            v2 = 0.0;
        pitch = pitcha + v2;
    }
    *angles = pitch;
    angles[1] = yaw;
    angles[2] = 0.0;
}

void __cdecl UnitQuatToAngles(const float *quat, float *angles)
{
    float axis[3][3]; // [esp+0h] [ebp-24h] BYREF

    UnitQuatToAxis(quat, axis);
    AxisToAngles(axis, angles);
}

void __cdecl YawVectors(float yaw, float *forward, float *right)
{
    float cy; // [esp+8h] [ebp-Ch]
    float angle; // [esp+Ch] [ebp-8h]
    float sy; // [esp+10h] [ebp-4h]

    angle = yaw * 0.01745329238474369;
    cy = cos(angle);
    sy = sin(angle);
    if (forward)
    {
        *forward = cy;
        forward[1] = sy;
        forward[2] = 0.0;
    }
    if (right)
    {
        *right = sy;
        right[1] = -cy;
        right[2] = 0.0;
    }
}

void __cdecl YawVectors2D(float yaw, float *forward, float *right)
{
    float cy; // [esp+8h] [ebp-Ch]
    float angle; // [esp+Ch] [ebp-8h]
    float sy; // [esp+10h] [ebp-4h]

    angle = yaw * 0.01745329238474369;
    cy = cos(angle);
    sy = sin(angle);
    if (forward)
    {
        *forward = cy;
        forward[1] = sy;
    }
    if (right)
    {
        *right = sy;
        right[1] = -cy;
    }
}

void __cdecl PerpendicularVector(const float *src, float *dst)
{
    const char *v2; // eax
    double scale; // [esp+18h] [ebp-34h]
    int pos; // [esp+38h] [ebp-14h]
    float d; // [esp+3Ch] [ebp-10h]
    float srcSq[3]; // [esp+40h] [ebp-Ch]

    if (!Vec3IsNormalized(src))
    {
        scale = Vec3Length(src);
        v2 = va("(%g %g %g) len %g", *src, src[1], src[2], scale);
        MyAssertHandler(".\\universal\\com_math.cpp", 1184, 0, "%s\n\t%s", "Vec3IsNormalized( src )", v2);
    }
    srcSq[0] = *src * *src;
    srcSq[1] = src[1] * src[1];
    srcSq[2] = src[2] * src[2];
    pos = srcSq[0] > (double)srcSq[1];
    if (srcSq[pos] > (double)srcSq[2])
        pos = 2;
    d = -src[pos];
    Vec3Scale(src, d, dst);
    dst[pos] = dst[pos] + 1.0;
    Vec3Normalize(dst);
}

double __cdecl PointToBoxDistSq(const float *pt, const float *mins, const float *maxs)
{
    float delta; // [esp+0h] [ebp-Ch]
    float deltaa; // [esp+0h] [ebp-Ch]
    int axis; // [esp+4h] [ebp-8h]
    float distSq; // [esp+8h] [ebp-4h]

    distSq = 0.0;
    for (axis = 0; axis < 3; ++axis)
    {
        delta = mins[axis] - pt[axis];
        if (delta <= 0.0)
        {
            deltaa = pt[axis] - maxs[axis];
            if (deltaa > 0.0)
                distSq = deltaa * deltaa + distSq;
        }
        else
        {
            distSq = delta * delta + distSq;
        }
    }
    return distSq;
}

void __cdecl ClosestApproachOfTwoLines(
    const float *p1,
    const float *dir1,
    const float *p2,
    const float *dir2,
    float *s,
    float *t)
{
    float v6; // [esp+0h] [ebp-3Ch]
    float v7; // [esp+4h] [ebp-38h]
    float v8; // [esp+8h] [ebp-34h]
    float diff[3]; // [esp+Ch] [ebp-30h] BYREF
    float invDet; // [esp+18h] [ebp-24h]
    float dir2LenSq; // [esp+1Ch] [ebp-20h]
    float diffDiff; // [esp+20h] [ebp-1Ch]
    float dir1dir2; // [esp+24h] [ebp-18h]
    float dir1LenSq; // [esp+28h] [ebp-14h]
    float det; // [esp+2Ch] [ebp-10h]
    float dir1Diff; // [esp+30h] [ebp-Ch]
    float dir2Diff; // [esp+34h] [ebp-8h]
    float EPSILON; // [esp+38h] [ebp-4h]

    Vec3Sub(p1, p2, diff);
    dir1LenSq = Vec3LengthSq(dir1);
    dir2LenSq = Vec3LengthSq(dir2);
    dir1dir2 = -Vec3Dot(dir1, dir2);
    dir1Diff = Vec3Dot(dir1, diff);
    diffDiff = Vec3Dot(diff, diff);
    det = dir1LenSq * dir2LenSq - dir1dir2 * dir1dir2;
    EPSILON = 0.000099999997;
    v8 = dir1dir2 * dir1LenSq;
    v7 = fabs(v8);
    v6 = (float)0.000099999997 * v7;
    if (v6 >= det * det)
    {
        if (dir1LenSq <= 0.000009999999747378752)
            MyAssertHandler(".\\universal\\com_math.cpp", 1399, 0, "%s", "dir1LenSq > 0.00001f");
        *s = -dir1Diff / dir1LenSq;
        *t = 0.0;
    }
    else
    {
        dir2Diff = -Vec3Dot(dir2, diff);
        invDet = 1.0 / det;
        *s = (dir1dir2 * dir2Diff - dir2LenSq * dir1Diff) * invDet;
        *t = (dir1dir2 * dir1Diff - dir1LenSq * dir2Diff) * invDet;
    }
}

void __cdecl MatrixIdentity33(float (*out)[3])
{
    if (!out)
        MyAssertHandler(".\\universal\\com_math.cpp", 1419, 0, "%s", "out");
    (*out)[0] = 0.0;
    (*out)[1] = 0.0;
    (*out)[2] = 0.0;
    (*out)[3] = 0.0;
    (*out)[4] = 0.0;
    (*out)[5] = 0.0;
    (*out)[6] = 0.0;
    (*out)[7] = 0.0;
    (*out)[8] = 0.0;
    (*out)[0] = 1.0;
    (*out)[4] = 1.0;
    (*out)[8] = 1.0;
}

void __cdecl MatrixIdentity44(float (*out)[4])
{
    if (!out)
        MyAssertHandler(".\\universal\\com_math.cpp", 1429, 0, "%s", "out");
    memcpy(out, identityMatrix44, 0x40u);
}

void __cdecl MatrixSet44(float (*out)[4], const float *origin, const float (*axis)[3], float scale)
{
    (*out)[0] = (*axis)[0] * scale;
    (*out)[1] = (float)(*axis)[1] * scale;
    (*out)[2] = (float)(*axis)[2] * scale;
    (*out)[3] = 0.0;
    (*out)[4] = (float)(*axis)[3] * scale;
    (*out)[5] = (float)(*axis)[4] * scale;
    (*out)[6] = (float)(*axis)[5] * scale;
    (*out)[7] = 0.0;
    (*out)[8] = (float)(*axis)[6] * scale;
    (*out)[9] = (float)(*axis)[7] * scale;
    (*out)[10] = (float)(*axis)[8] * scale;
    (*out)[11] = 0.0;
    (*out)[12] = *origin;
    (*out)[13] = origin[1];
    (*out)[14] = origin[2];
    (*out)[15] = 1.0;
}

void __cdecl MatrixMultiply(const float (*in1)[3], const float (*in2)[3], float (*out)[3])
{
    (*out)[0] = (*in1)[0] * (*in2)[0] + (float)(*in1)[1] * (float)(*in2)[3] + (float)(*in1)[2] * (float)(*in2)[6];
    (*out)[1] = (*in1)[0] * (float)(*in2)[1] + (float)(*in1)[1] * (float)(*in2)[4] + (float)(*in1)[2] * (float)(*in2)[7];
    (*out)[2] = (*in1)[0] * (float)(*in2)[2] + (float)(*in1)[1] * (float)(*in2)[5] + (float)(*in1)[2] * (float)(*in2)[8];
    (*out)[3] = (float)(*in1)[3] * (*in2)[0] + (float)(*in1)[4] * (float)(*in2)[3] + (float)(*in1)[5] * (float)(*in2)[6];
    (*out)[4] = (float)(*in1)[3] * (float)(*in2)[1]
        + (float)(*in1)[4] * (float)(*in2)[4]
        + (float)(*in1)[5] * (float)(*in2)[7];
    (*out)[5] = (float)(*in1)[3] * (float)(*in2)[2]
        + (float)(*in1)[4] * (float)(*in2)[5]
        + (float)(*in1)[5] * (float)(*in2)[8];
    (*out)[6] = (float)(*in1)[6] * (*in2)[0] + (float)(*in1)[7] * (float)(*in2)[3] + (float)(*in1)[8] * (float)(*in2)[6];
    (*out)[7] = (float)(*in1)[6] * (float)(*in2)[1]
        + (float)(*in1)[7] * (float)(*in2)[4]
        + (float)(*in1)[8] * (float)(*in2)[7];
    (*out)[8] = (float)(*in1)[6] * (float)(*in2)[2]
        + (float)(*in1)[7] * (float)(*in2)[5]
        + (float)(*in1)[8] * (float)(*in2)[8];
}

void __cdecl MatrixMultiply43(const float (*in1)[3], const float (*in2)[3], float (*out)[3])
{
    if (in1 == out)
        MyAssertHandler(".\\universal\\com_math.cpp", 1545, 0, "%s", "in1 != out");
    if (in2 == out)
        MyAssertHandler(".\\universal\\com_math.cpp", 1546, 0, "%s", "in2 != out");
    (*out)[0] = (*in1)[0] * (*in2)[0] + (float)(*in1)[1] * (float)(*in2)[3] + (float)(*in1)[2] * (float)(*in2)[6];
    (*out)[3] = (float)(*in1)[3] * (*in2)[0] + (float)(*in1)[4] * (float)(*in2)[3] + (float)(*in1)[5] * (float)(*in2)[6];
    (*out)[6] = (float)(*in1)[6] * (*in2)[0] + (float)(*in1)[7] * (float)(*in2)[3] + (float)(*in1)[8] * (float)(*in2)[6];
    (*out)[1] = (*in1)[0] * (float)(*in2)[1] + (float)(*in1)[1] * (float)(*in2)[4] + (float)(*in1)[2] * (float)(*in2)[7];
    (*out)[4] = (float)(*in1)[3] * (float)(*in2)[1]
        + (float)(*in1)[4] * (float)(*in2)[4]
        + (float)(*in1)[5] * (float)(*in2)[7];
    (*out)[7] = (float)(*in1)[6] * (float)(*in2)[1]
        + (float)(*in1)[7] * (float)(*in2)[4]
        + (float)(*in1)[8] * (float)(*in2)[7];
    (*out)[2] = (*in1)[0] * (float)(*in2)[2] + (float)(*in1)[1] * (float)(*in2)[5] + (float)(*in1)[2] * (float)(*in2)[8];
    (*out)[5] = (float)(*in1)[3] * (float)(*in2)[2]
        + (float)(*in1)[4] * (float)(*in2)[5]
        + (float)(*in1)[5] * (float)(*in2)[8];
    (*out)[8] = (float)(*in1)[6] * (float)(*in2)[2]
        + (float)(*in1)[7] * (float)(*in2)[5]
        + (float)(*in1)[8] * (float)(*in2)[8];
    (*out)[9] = (float)(*in1)[9] * (*in2)[0]
        + (float)(*in1)[10] * (float)(*in2)[3]
        + (float)(*in1)[11] * (float)(*in2)[6]
        + (float)(*in2)[9];
    (*out)[10] = (float)(*in1)[9] * (float)(*in2)[1]
        + (float)(*in1)[10] * (float)(*in2)[4]
        + (float)(*in1)[11] * (float)(*in2)[7]
        + (float)(*in2)[10];
    (*out)[11] = (float)(*in1)[9] * (float)(*in2)[2]
        + (float)(*in1)[10] * (float)(*in2)[5]
        + (float)(*in1)[11] * (float)(*in2)[8]
        + (float)(*in2)[11];
}

void __cdecl MatrixMultiply44(const float (*in1)[4], const float (*in2)[4], float (*out)[4])
{
    if (in1 == out)
        MyAssertHandler(".\\universal\\com_math.cpp", 1573, 0, "%s", "in1 != out");
    if (in2 == out)
        MyAssertHandler(".\\universal\\com_math.cpp", 1574, 0, "%s", "in2 != out");
    (*out)[0] = (*in1)[0] * (*in2)[0]
        + (float)(*in1)[1] * (float)(*in2)[4]
        + (float)(*in1)[2] * (float)(*in2)[8]
        + (float)(*in1)[3] * (float)(*in2)[12];
    (*out)[1] = (*in1)[0] * (float)(*in2)[1]
        + (float)(*in1)[1] * (float)(*in2)[5]
        + (float)(*in1)[2] * (float)(*in2)[9]
        + (float)(*in1)[3] * (float)(*in2)[13];
    (*out)[2] = (*in1)[0] * (float)(*in2)[2]
        + (float)(*in1)[1] * (float)(*in2)[6]
        + (float)(*in1)[2] * (float)(*in2)[10]
        + (float)(*in1)[3] * (float)(*in2)[14];
    (*out)[3] = (*in1)[0] * (float)(*in2)[3]
        + (float)(*in1)[1] * (float)(*in2)[7]
        + (float)(*in1)[2] * (float)(*in2)[11]
        + (float)(*in1)[3] * (float)(*in2)[15];
    (*out)[4] = (float)(*in1)[4] * (*in2)[0]
        + (float)(*in1)[5] * (float)(*in2)[4]
        + (float)(*in1)[6] * (float)(*in2)[8]
        + (float)(*in1)[7] * (float)(*in2)[12];
    (*out)[5] = (float)(*in1)[4] * (float)(*in2)[1]
        + (float)(*in1)[5] * (float)(*in2)[5]
        + (float)(*in1)[6] * (float)(*in2)[9]
        + (float)(*in1)[7] * (float)(*in2)[13];
    (*out)[6] = (float)(*in1)[4] * (float)(*in2)[2]
        + (float)(*in1)[5] * (float)(*in2)[6]
        + (float)(*in1)[6] * (float)(*in2)[10]
        + (float)(*in1)[7] * (float)(*in2)[14];
    (*out)[7] = (float)(*in1)[4] * (float)(*in2)[3]
        + (float)(*in1)[5] * (float)(*in2)[7]
        + (float)(*in1)[6] * (float)(*in2)[11]
        + (float)(*in1)[7] * (float)(*in2)[15];
    (*out)[8] = (float)(*in1)[8] * (*in2)[0]
        + (float)(*in1)[9] * (float)(*in2)[4]
        + (float)(*in1)[10] * (float)(*in2)[8]
        + (float)(*in1)[11] * (float)(*in2)[12];
    (*out)[9] = (float)(*in1)[8] * (float)(*in2)[1]
        + (float)(*in1)[9] * (float)(*in2)[5]
        + (float)(*in1)[10] * (float)(*in2)[9]
        + (float)(*in1)[11] * (float)(*in2)[13];
    (*out)[10] = (float)(*in1)[8] * (float)(*in2)[2]
        + (float)(*in1)[9] * (float)(*in2)[6]
        + (float)(*in1)[10] * (float)(*in2)[10]
        + (float)(*in1)[11] * (float)(*in2)[14];
    (*out)[11] = (float)(*in1)[8] * (float)(*in2)[3]
        + (float)(*in1)[9] * (float)(*in2)[7]
        + (float)(*in1)[10] * (float)(*in2)[11]
        + (float)(*in1)[11] * (float)(*in2)[15];
    (*out)[12] = (float)(*in1)[12] * (*in2)[0]
        + (float)(*in1)[13] * (float)(*in2)[4]
        + (float)(*in1)[14] * (float)(*in2)[8]
        + (float)(*in1)[15] * (float)(*in2)[12];
    (*out)[13] = (float)(*in1)[12] * (float)(*in2)[1]
        + (float)(*in1)[13] * (float)(*in2)[5]
        + (float)(*in1)[14] * (float)(*in2)[9]
        + (float)(*in1)[15] * (float)(*in2)[13];
    (*out)[14] = (float)(*in1)[12] * (float)(*in2)[2]
        + (float)(*in1)[13] * (float)(*in2)[6]
        + (float)(*in1)[14] * (float)(*in2)[10]
        + (float)(*in1)[15] * (float)(*in2)[14];
    (*out)[15] = (float)(*in1)[12] * (float)(*in2)[3]
        + (float)(*in1)[13] * (float)(*in2)[7]
        + (float)(*in1)[14] * (float)(*in2)[11]
        + (float)(*in1)[15] * (float)(*in2)[15];
}

void __cdecl MatrixTranspose(const float (*in)[3], float (*out)[3])
{
    if (in == out)
        MyAssertHandler(".\\universal\\com_math.cpp", 1605, 0, "%s", "in != out");
    (*out)[0] = (*in)[0];
    (*out)[1] = (*in)[3];
    (*out)[2] = (*in)[6];
    (*out)[3] = (*in)[1];
    (*out)[4] = (*in)[4];
    (*out)[5] = (*in)[7];
    (*out)[6] = (*in)[2];
    (*out)[7] = (*in)[5];
    (*out)[8] = (*in)[8];
}

void __cdecl MatrixTranspose44(const float *in, float *out)
{
    if (in == out)
        MyAssertHandler(".\\universal\\com_math.cpp", 1621, 0, "%s", "in != out");
    *out = *in;
    out[1] = in[4];
    out[2] = in[8];
    out[3] = in[12];
    out[4] = in[1];
    out[5] = in[5];
    out[6] = in[9];
    out[7] = in[13];
    out[8] = in[2];
    out[9] = in[6];
    out[10] = in[10];
    out[11] = in[14];
    out[12] = in[3];
    out[13] = in[7];
    out[14] = in[11];
    out[15] = in[15];
}

void __cdecl MatrixInverseOrthogonal43(const float (*in)[3], float (*out)[3])
{
    float origin[3]; // [esp+0h] [ebp-Ch] BYREF

    MatrixTranspose(in, out);
    Vec3Sub(vec3_origin, &(*in)[9], origin);
    MatrixTransformVector(origin, out, &(*out)[9]);
}

void __cdecl MatrixInverse44(const float *mat, float *dst)
{
    float src[16]; // [esp+0h] [ebp-78h]
    float det; // [esp+40h] [ebp-38h]
    float tmp[12]; // [esp+44h] [ebp-34h]
    int i; // [esp+74h] [ebp-4h]

    if (mat == dst)
        MyAssertHandler(".\\universal\\com_math.cpp", 1702, 0, "%s", "mat != dst");
    for (i = 0; i < 4; ++i)
    {
        src[i] = mat[4 * i];
        src[i + 4] = mat[4 * i + 1];
        src[i + 8] = mat[4 * i + 2];
        src[i + 12] = mat[4 * i + 3];
    }
    tmp[0] = src[10] * src[15];
    tmp[1] = src[11] * src[14];
    tmp[2] = src[9] * src[15];
    tmp[3] = src[11] * src[13];
    tmp[4] = src[9] * src[14];
    tmp[5] = src[10] * src[13];
    tmp[6] = src[8] * src[15];
    tmp[7] = src[11] * src[12];
    tmp[8] = src[8] * src[14];
    tmp[9] = src[10] * src[12];
    tmp[10] = src[8] * src[13];
    tmp[11] = src[9] * src[12];
    *dst = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
    *dst = *dst - (tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7]);
    dst[1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
    dst[1] = dst[1] - (tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7]);
    dst[2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
    dst[2] = dst[2] - (tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7]);
    dst[3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
    dst[3] = dst[3] - (tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6]);
    dst[4] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
    dst[4] = dst[4] - (tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3]);
    dst[5] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
    dst[5] = dst[5] - (tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3]);
    dst[6] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
    dst[6] = dst[6] - (tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3]);
    dst[7] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
    dst[7] = dst[7] - (tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2]);
    tmp[0] = src[2] * src[7];
    tmp[1] = src[3] * src[6];
    tmp[2] = src[1] * src[7];
    tmp[3] = src[3] * src[5];
    tmp[4] = src[1] * src[6];
    tmp[5] = src[2] * src[5];
    tmp[6] = src[0] * src[7];
    tmp[7] = src[3] * src[4];
    tmp[8] = src[0] * src[6];
    tmp[9] = src[2] * src[4];
    tmp[10] = src[0] * src[5];
    tmp[11] = src[1] * src[4];
    dst[8] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
    dst[8] = dst[8] - (tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15]);
    dst[9] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
    dst[9] = dst[9] - (tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15]);
    dst[10] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
    dst[10] = dst[10] - (tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15]);
    dst[11] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
    dst[11] = dst[11] - (tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14]);
    dst[12] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
    dst[12] = dst[12] - (tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10]);
    dst[13] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
    dst[13] = dst[13] - (tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8]);
    dst[14] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
    dst[14] = dst[14] - (tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9]);
    dst[15] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
    dst[15] = dst[15] - (tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8]);
    det = src[0] * *dst + src[1] * dst[1] + src[2] * dst[2] + src[3] * dst[3];
    if (det == 0.0)
        MyAssertHandler(".\\universal\\com_math.cpp", 1775, 0, "%s", "det");
    det = 1.0 / det;
    for (i = 0; i < 16; ++i)
        dst[i] = dst[i] * det;
}

void __cdecl MatrixTransformVector44(const float *vec, const float (*mat)[4], float *out)
{
    if (vec == out)
        MyAssertHandler(".\\universal\\com_math.cpp", 1789, 0, "%s", "vec != out");
    *out = *vec * (*mat)[0] + vec[1] * (float)(*mat)[4] + vec[2] * (float)(*mat)[8] + vec[3] * (float)(*mat)[12];
    out[1] = *vec * (float)(*mat)[1] + vec[1] * (float)(*mat)[5] + vec[2] * (float)(*mat)[9] + vec[3] * (float)(*mat)[13];
    out[2] = *vec * (float)(*mat)[2] + vec[1] * (float)(*mat)[6] + vec[2] * (float)(*mat)[10] + vec[3] * (float)(*mat)[14];
    out[3] = *vec * (float)(*mat)[3] + vec[1] * (float)(*mat)[7] + vec[2] * (float)(*mat)[11] + vec[3] * (float)(*mat)[15];
}

void __cdecl MatrixTransposeTransformVector(const float *in1, const float (*in2)[3], float *out)
{
    if (in1 == out)
        MyAssertHandler(".\\universal\\com_math.cpp", 1804, 0, "%s", "in1 != out");
    *out = *in1 * (*in2)[0] + in1[1] * (float)(*in2)[1] + in1[2] * (float)(*in2)[2];
    out[1] = *in1 * (float)(*in2)[3] + in1[1] * (float)(*in2)[4] + in1[2] * (float)(*in2)[5];
    out[2] = *in1 * (float)(*in2)[6] + in1[1] * (float)(*in2)[7] + in1[2] * (float)(*in2)[8];
}

void __cdecl MatrixTransformVector43(const float *in1, const float (*in2)[3], float *out)
{
    if (in1 == out)
        MyAssertHandler(".\\universal\\com_math.cpp", 1818, 0, "%s", "in1 != out");
    *out = *in1 * (*in2)[0] + in1[1] * (float)(*in2)[3] + in1[2] * (float)(*in2)[6] + (float)(*in2)[9];
    out[1] = *in1 * (float)(*in2)[1] + in1[1] * (float)(*in2)[4] + in1[2] * (float)(*in2)[7] + (float)(*in2)[10];
    out[2] = *in1 * (float)(*in2)[2] + in1[1] * (float)(*in2)[5] + in1[2] * (float)(*in2)[8] + (float)(*in2)[11];
}

void __cdecl MatrixTransposeTransformVector43(float *in1, const float (*in2)[3], float *out)
{
    float temp[3]; // [esp+0h] [ebp-Ch] BYREF

    if (in1 == out)
        MyAssertHandler(".\\universal\\com_math.cpp", 1834, 0, "%s", "in1 != out");
    Vec3Sub(in1, &(*in2)[9], temp);
    *out = (*in2)[0] * temp[0] + (float)(*in2)[1] * temp[1] + (float)(*in2)[2] * temp[2];
    out[1] = (float)(*in2)[3] * temp[0] + (float)(*in2)[4] * temp[1] + (float)(*in2)[5] * temp[2];
    out[2] = (float)(*in2)[6] * temp[0] + (float)(*in2)[7] * temp[1] + (float)(*in2)[8] * temp[2];
}

void __cdecl VectorAngleMultiply(float *vec, float angle)
{
    float v2; // [esp+8h] [ebp-10h]
    float temp; // [esp+Ch] [ebp-Ch]
    float x; // [esp+10h] [ebp-8h]
    float y; // [esp+14h] [ebp-4h]

    v2 = angle * 0.01745329238474369;
    x = cos(v2);
    y = sin(v2);
    temp = *vec * x - vec[1] * y;
    vec[1] = vec[1] * x + *vec * y;
    *vec = temp;
}

void __cdecl QuatToAxis(const float *quat, float (*axis)[3])
{
    float yy; // [esp+0h] [ebp-38h]
    float yya; // [esp+0h] [ebp-38h]
    float ww; // [esp+4h] [ebp-34h]
    float xy; // [esp+8h] [ebp-30h]
    float zz; // [esp+Ch] [ebp-2Ch]
    float zza; // [esp+Ch] [ebp-2Ch]
    float zw; // [esp+10h] [ebp-28h]
    float scaledX; // [esp+14h] [ebp-24h]
    float scaledY; // [esp+18h] [ebp-20h]
    float yw; // [esp+1Ch] [ebp-1Ch]
    float xz; // [esp+20h] [ebp-18h]
    float yz; // [esp+24h] [ebp-14h]
    float xx; // [esp+28h] [ebp-10h]
    float xxa; // [esp+28h] [ebp-10h]
    float xw; // [esp+2Ch] [ebp-Ch]
    float scale; // [esp+30h] [ebp-8h]
    float magSqr; // [esp+34h] [ebp-4h]

    xx = *quat * *quat;
    yy = quat[1] * quat[1];
    zz = quat[2] * quat[2];
    ww = quat[3] * quat[3];
    magSqr = xx + yy + zz + ww;
    if (magSqr <= 0.0)
        MyAssertHandler(".\\universal\\com_math.cpp", 1910, 0, "%s", "magSqr > 0.0f");
    scale = 2.0 / magSqr;
    xxa = xx * scale;
    yya = yy * scale;
    zza = zz * scale;
    scaledX = scale * *quat;
    xy = scaledX * quat[1];
    xz = scaledX * quat[2];
    xw = scaledX * quat[3];
    scaledY = scale * quat[1];
    yz = scaledY * quat[2];
    yw = scaledY * quat[3];
    zw = scale * quat[2] * quat[3];
    (*axis)[0] = 1.0 - (yya + zza);
    (*axis)[1] = xy + zw;
    (*axis)[2] = xz - yw;
    (*axis)[3] = xy - zw;
    (*axis)[4] = 1.0 - (xxa + zza);
    (*axis)[5] = yz + xw;
    (*axis)[6] = xz + yw;
    (*axis)[7] = yz - xw;
    (*axis)[8] = 1.0 - (xxa + yya);
}

void __cdecl UnitQuatToAxis(const float *quat, float (*axis)[3])
{
    const char *v2; // eax
    float yy; // [esp+24h] [ebp-30h]
    float xy; // [esp+28h] [ebp-2Ch]
    float scaledZ; // [esp+2Ch] [ebp-28h]
    float zz; // [esp+30h] [ebp-24h]
    float zw; // [esp+34h] [ebp-20h]
    float scaledX; // [esp+38h] [ebp-1Ch]
    float scaledY; // [esp+3Ch] [ebp-18h]
    float yw; // [esp+40h] [ebp-14h]
    float xz; // [esp+44h] [ebp-10h]
    float yz; // [esp+48h] [ebp-Ch]
    float xx; // [esp+4Ch] [ebp-8h]
    float xw; // [esp+50h] [ebp-4h]

    if (!Vec4IsNormalized(quat))
    {
        v2 = va("%g %g %g %g", *quat, quat[1], quat[2], quat[3]);
        MyAssertHandler(".\\universal\\com_math.cpp", 1948, 0, "%s\n\t%s", "Vec4IsNormalized( quat )", v2);
    }
    scaledX = *quat + *quat;
    xx = scaledX * *quat;
    xy = scaledX * quat[1];
    xz = scaledX * quat[2];
    xw = scaledX * quat[3];
    scaledY = quat[1] + quat[1];
    yy = scaledY * quat[1];
    yz = scaledY * quat[2];
    yw = scaledY * quat[3];
    scaledZ = quat[2] + quat[2];
    zw = scaledZ * quat[3];
    zz = scaledZ * quat[2];
    (*axis)[0] = 1.0 - (yy + zz);
    (*axis)[1] = xy + zw;
    (*axis)[2] = xz - yw;
    (*axis)[3] = xy - zw;
    (*axis)[4] = 1.0 - (xx + zz);
    (*axis)[5] = yz + xw;
    (*axis)[6] = xz + yw;
    (*axis)[7] = yz - xw;
    (*axis)[8] = 1.0 - (xx + yy);
}

void __cdecl UnitQuatToForward(const float *quat, float *forward)
{
    const char *v2; // eax

    if (!Vec4IsNormalized(quat))
    {
        v2 = va("%g %g %g %g", *quat, quat[1], quat[2], quat[3]);
        MyAssertHandler(".\\universal\\com_math.cpp", 1981, 0, "%s\n\t%s", "Vec4IsNormalized( quat )", v2);
    }
    *forward = 1.0 - (quat[1] * quat[1] + quat[2] * quat[2]) * 2.0;
    forward[1] = (*quat * quat[1] + quat[2] * quat[3]) * 2.0;
    forward[2] = (*quat * quat[2] - quat[1] * quat[3]) * 2.0;
}

void __cdecl QuatSlerp(const float *from, const float *to, float frac, float *result)
{
    double v4; // st6
    float v5; // [esp+0h] [ebp-30h]
    float v6; // [esp+4h] [ebp-2Ch]
    float v7; // [esp+8h] [ebp-28h]
    float v8; // [esp+Ch] [ebp-24h]
    float v9; // [esp+10h] [ebp-20h]
    float v10; // [esp+14h] [ebp-1Ch]
    float scaleTo; // [esp+20h] [ebp-10h]
    float scaleFrom; // [esp+24h] [ebp-Ch]
    float dot; // [esp+28h] [ebp-8h]
    bool dotWasNeg; // [esp+2Fh] [ebp-1h]

    dot = Vec4Dot(from, to);
    if (dot >= 0.0)
    {
        dotWasNeg = 0;
    }
    else
    {
        dotWasNeg = 1;
        dot = dot * -1.0;
    }
    if (dot <= 0.94999999)
    {
        v10 = acos(dot);
        v9 = sin(v10);
        v8 = 1.0 - v10 * frac;
        v7 = sin(v8);
        scaleFrom = v7 / v9;
        v6 = v10 * frac;
        v5 = sin(v6);
        scaleTo = v5 / v9;
    }
    else
    {
        scaleFrom = 1.0 - frac;
        scaleTo = frac;
    }
    if (dotWasNeg)
    {
        *result = scaleFrom * *from + scaleTo * *to * -1.0;
        result[1] = scaleFrom * from[1] + scaleTo * to[1] * -1.0;
        result[2] = scaleFrom * from[2] + scaleTo * to[2] * -1.0;
        v4 = scaleTo * to[3] * -1.0;
    }
    else
    {
        *result = scaleFrom * *from + scaleTo * *to;
        result[1] = scaleFrom * from[1] + scaleTo * to[1];
        result[2] = scaleFrom * from[2] + scaleTo * to[2];
        v4 = scaleTo * to[3];
    }
    result[3] = scaleFrom * from[3] + v4;
}

void __cdecl QuatMultiply(const float *in1, const float *in2, float *out)
{
    if (in1 == out)
        MyAssertHandler("c:\\trees\\cod3\\src\\universal\\com_math.h", 732, 0, "%s", "in1 != out");
    if (in2 == out)
        MyAssertHandler("c:\\trees\\cod3\\src\\universal\\com_math.h", 733, 0, "%s", "in2 != out");
    *out = *in1 * in2[3] + in1[3] * *in2 + in1[2] * in2[1] - in1[1] * in2[2];
    out[1] = in1[1] * in2[3] - in1[2] * *in2 + in1[3] * in2[1] + *in1 * in2[2];
    out[2] = in1[2] * in2[3] + in1[1] * *in2 - *in1 * in2[1] + in1[3] * in2[2];
    out[3] = in1[3] * in2[3] - *in1 * *in2 - in1[1] * in2[1] - in1[2] * in2[2];
}

double __cdecl RotationToYaw(const float *rot)
{
    float v2; // [esp+0h] [ebp-18h]
    float v3; // [esp+4h] [ebp-14h]
    float v4; // [esp+8h] [ebp-10h]
    float r; // [esp+Ch] [ebp-Ch]
    float ra; // [esp+Ch] [ebp-Ch]
    float zz; // [esp+14h] [ebp-4h]

    zz = *rot * *rot;
    r = rot[1] * rot[1] + zz;
    if (r == 0.0)
        MyAssertHandler(".\\universal\\com_math.cpp", 2120, 0, "%s", "r");
    ra = 2.0 / r;
    v4 = ra * (rot[1] * *rot);
    v3 = 1.0 - ra * zz;
    v2 = atan2(v4, v3);
    return (float)(v2 * 57.2957763671875);
}

void __cdecl AnglesSubtract(float *v1, float *v2, float *v3)
{
    *v3 = AngleDelta(*v1, *v2);
    v3[1] = AngleDelta(v1[1], v2[1]);
    v3[2] = AngleDelta(v1[2], v2[2]);
}

double __cdecl AngleNormalize360(float angle)
{
    float v3; // [esp+Ch] [ebp-18h]
    float scaledAngle; // [esp+18h] [ebp-Ch]
    float result; // [esp+1Ch] [ebp-8h]
    float result2; // [esp+20h] [ebp-4h]

    scaledAngle = angle * 0.002777777845039964;
    v3 = floor(scaledAngle);
    result = (scaledAngle - v3) * 360.0;
    result2 = result - 360.0;
    if (result2 < 0.0)
        return (float)((scaledAngle - v3) * 360.0);
    else
        return (float)(result - 360.0);
}

double __cdecl RadiusFromBounds(const float *mins, const float *maxs)
{
    float v4; // [esp+4h] [ebp-4h]

    v4 = RadiusFromBoundsSq(mins, maxs);
    return (float)sqrt(v4);
}

double __cdecl RadiusFromBounds2D(const float *mins, const float *maxs)
{
    float v4; // [esp+4h] [ebp-4h]

    v4 = RadiusFromBounds2DSq(mins, maxs);
    return (float)sqrt(v4);
}

double __cdecl RadiusFromBoundsSq(const float *mins, const float *maxs)
{
    float v3; // [esp+0h] [ebp-2Ch]
    float v4; // [esp+4h] [ebp-28h]
    float v5; // [esp+8h] [ebp-24h]
    int i; // [esp+18h] [ebp-14h]
    float corner[3]; // [esp+1Ch] [ebp-10h] BYREF
    float a; // [esp+28h] [ebp-4h]

    for (i = 0; i < 3; ++i)
    {
        v5 = fabs(mins[i]);
        a = v5;
        v4 = fabs(maxs[i]);
        if (v4 >= (double)v5)
            v3 = v4;
        else
            v3 = a;
        corner[i] = v3;
    }
    return Vec3LengthSq(corner);
}

double __cdecl RadiusFromBounds2DSq(const float *mins, const float *maxs)
{
    float v4; // [esp+4h] [ebp-28h]
    float v5; // [esp+8h] [ebp-24h]
    float v6; // [esp+Ch] [ebp-20h]
    int i; // [esp+1Ch] [ebp-10h]
    float corner[2]; // [esp+20h] [ebp-Ch]
    float a; // [esp+28h] [ebp-4h]

    for (i = 0; i < 2; ++i)
    {
        v6 = fabs(mins[i]);
        a = v6;
        v5 = fabs(maxs[i]);
        if (v5 >= (double)v6)
            v4 = v5;
        else
            v4 = a;
        corner[i] = v4;
    }
    return (float)(corner[1] * corner[1] + corner[0] * corner[0]);
}

void __cdecl ExtendBounds(float *mins, float *maxs, const float *offset)
{
    if (*offset <= 0.0)
        *mins = *mins + *offset;
    else
        *maxs = *maxs + *offset;
    if (offset[1] <= 0.0)
        mins[1] = mins[1] + offset[1];
    else
        maxs[1] = maxs[1] + offset[1];
    if (offset[2] <= 0.0)
        mins[2] = mins[2] + offset[2];
    else
        maxs[2] = maxs[2] + offset[2];
}

void __cdecl ExpandBoundsToWidth(float *mins, float *maxs)
{
    float v2; // [esp+0h] [ebp-1Ch]
    float v3; // [esp+4h] [ebp-18h]
    float diff; // [esp+8h] [ebp-14h]
    float size[3]; // [esp+10h] [ebp-Ch] BYREF

    if (*mins > (double)*maxs)
        MyAssertHandler(".\\universal\\com_math.cpp", 2455, 0, "%s", "maxs[0] >= mins[0]");
    if (mins[1] > (double)maxs[1])
        MyAssertHandler(".\\universal\\com_math.cpp", 2456, 0, "%s", "maxs[1] >= mins[1]");
    if (mins[2] > (double)maxs[2])
        MyAssertHandler(".\\universal\\com_math.cpp", 2457, 0, "%s", "maxs[2] >= mins[2]");
    Vec3Sub(maxs, mins, size);
    v3 = size[0] - size[1];
    if (v3 < 0.0)
        v2 = size[1];
    else
        v2 = size[0];
    if (size[2] < (double)v2)
    {
        diff = (v2 - size[2]) * 0.5;
        mins[2] = mins[2] - diff;
        maxs[2] = maxs[2] + diff;
    }
}

void __cdecl ShrinkBoundsToHeight(float *mins, float *maxs)
{
    float diff; // [esp+10h] [ebp-10h]
    float diffa; // [esp+10h] [ebp-10h]
    float size[3]; // [esp+14h] [ebp-Ch] BYREF

    if (*mins > (double)*maxs)
        MyAssertHandler(".\\universal\\com_math.cpp", 2477, 0, "maxs[0] >= mins[0]\n\t%g, %g", *maxs, *mins);
    if (mins[1] > (double)maxs[1])
        MyAssertHandler(".\\universal\\com_math.cpp", 2478, 0, "maxs[1] >= mins[1]\n\t%g, %g", maxs[1], mins[1]);
    if (mins[2] > (double)maxs[2])
        MyAssertHandler(".\\universal\\com_math.cpp", 2479, 0, "maxs[2] >= mins[2]\n\t%g, %g", maxs[2], mins[2]);
    Vec3Sub(maxs, mins, size);
    if (size[2] < (double)size[0])
    {
        diff = (size[0] - size[2]) * 0.5;
        *mins = *mins + diff;
        *maxs = *maxs - diff;
    }
    if (size[2] < (double)size[1])
    {
        diffa = (size[1] - size[2]) * 0.5;
        mins[1] = mins[1] + diffa;
        maxs[1] = maxs[1] - diffa;
    }
}

void __cdecl ClearBounds2D(float *mins, float *maxs)
{
    *mins = 131072.0;
    mins[1] = 131072.0;
    *maxs = -131072.0;
    maxs[1] = -131072.0;
}

void __cdecl AddPointToBounds(const float *v, float *mins, float *maxs)
{
    if (*mins > (double)*v)
        *mins = *v;
    if (*maxs < (double)*v)
        *maxs = *v;
    if (mins[1] > (double)v[1])
        mins[1] = v[1];
    if (maxs[1] < (double)v[1])
        maxs[1] = v[1];
    if (mins[2] > (double)v[2])
        mins[2] = v[2];
    if (maxs[2] < (double)v[2])
        maxs[2] = v[2];
}

void __cdecl AddPointToBounds2D(const float *v, float *mins, float *maxs)
{
    if (*mins > (double)*v)
        *mins = *v;
    if (*maxs < (double)*v)
        *maxs = *v;
    if (mins[1] > (double)v[1])
        mins[1] = v[1];
    if (maxs[1] < (double)v[1])
        maxs[1] = v[1];
}

bool __cdecl PointInBounds(const float *v, const float *mins, const float *maxs)
{
    if (!v)
        MyAssertHandler(".\\universal\\com_math.cpp", 2560, 0, "%s", "v");
    if (!mins)
        MyAssertHandler(".\\universal\\com_math.cpp", 2561, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\universal\\com_math.cpp", 2562, 0, "%s", "maxs");
    if (*mins > (double)*v || *maxs < (double)*v)
        return 0;
    if (mins[1] > (double)v[1] || maxs[1] < (double)v[1])
        return 0;
    return mins[2] <= (double)v[2] && maxs[2] >= (double)v[2];
}

bool __cdecl BoundsOverlap(const float *mins0, const float *maxs0, const float *mins1, const float *maxs1)
{
    if (*maxs1 < (double)*mins0 || *maxs0 < (double)*mins1)
        return 0;
    if (maxs1[1] < (double)mins0[1] || maxs0[1] < (double)mins1[1])
        return 0;
    return maxs1[2] >= (double)mins0[2] && maxs0[2] >= (double)mins1[2];
}

void __cdecl ExpandBounds(const float *addedmins, const float *addedmaxs, float *mins, float *maxs)
{
    if (*addedmins < (double)*mins)
        *mins = *addedmins;
    if (*addedmaxs > (double)*maxs)
        *maxs = *addedmaxs;
    if (addedmins[1] < (double)mins[1])
        mins[1] = addedmins[1];
    if (addedmaxs[1] > (double)maxs[1])
        maxs[1] = addedmaxs[1];
    if (addedmins[2] < (double)mins[2])
        mins[2] = addedmins[2];
    if (addedmaxs[2] > (double)maxs[2])
        maxs[2] = addedmaxs[2];
}

void __cdecl AxisClear(float (*axis)[3])
{
    (*axis)[0] = 1.0;
    (*axis)[1] = 0.0;
    (*axis)[2] = 0.0;
    (*axis)[3] = 0.0;
    (*axis)[4] = 1.0;
    (*axis)[5] = 0.0;
    (*axis)[6] = 0.0;
    (*axis)[7] = 0.0;
    (*axis)[8] = 1.0;
}

void __cdecl AxisTranspose(const float (*in)[3], float (*out)[3])
{
    if (in == out)
        MyAssertHandler(".\\universal\\com_math.cpp", 2732, 0, "%s", "in != out");
    (*out)[0] = (*in)[0];
    (*out)[1] = (*in)[3];
    (*out)[2] = (*in)[6];
    (*out)[3] = (*in)[1];
    (*out)[4] = (*in)[4];
    (*out)[5] = (*in)[7];
    (*out)[6] = (*in)[2];
    (*out)[7] = (*in)[5];
    (*out)[8] = (*in)[8];
}

void __cdecl AxisTransformVec3(const float (*axes)[3], const float *vec, float *out)
{
    *out = *vec * (*axes)[0] + vec[1] * (float)(*axes)[3] + vec[2] * (float)(*axes)[6];
    out[1] = *vec * (float)(*axes)[1] + vec[1] * (float)(*axes)[4] + vec[2] * (float)(*axes)[7];
    out[2] = *vec * (float)(*axes)[2] + vec[1] * (float)(*axes)[5] + vec[2] * (float)(*axes)[8];
}

void __cdecl YawToAxis(float yaw, float (*axis)[3])
{
    float right[3]; // [esp+Ch] [ebp-Ch] BYREF

    YawVectors(yaw, (float *)axis, right);
    (*axis)[6] = 0.0;
    (*axis)[7] = 0.0;
    (*axis)[8] = 1.0;
    Vec3Sub(vec3_origin, right, &(*axis)[3]);
}

void __cdecl AxisToAngles(const float (*axis)[3], float *angles)
{
    float v2; // [esp+0h] [ebp-38h]
    float rad; // [esp+18h] [ebp-20h]
    float rada; // [esp+18h] [ebp-20h]
    float right[3]; // [esp+1Ch] [ebp-1Ch] BYREF
    float temp; // [esp+28h] [ebp-10h]
    float pitch; // [esp+2Ch] [ebp-Ch]
    float fCos; // [esp+30h] [ebp-8h]
    float fSin; // [esp+34h] [ebp-4h]

    vectoangles((const float *)axis, angles);
    right[0] = (*axis)[3];
    right[1] = (*axis)[4];
    right[2] = (*axis)[5];
    rad = -angles[1] * 0.01745329238474369;
    fCos = cos(rad);
    fSin = sin(rad);
    temp = fCos * right[0] - fSin * right[1];
    right[1] = fSin * right[0] + fCos * right[1];
    rada = -*angles * 0.01745329238474369;
    fCos = cos(rada);
    fSin = sin(rada);
    right[0] = fSin * right[2] + fCos * temp;
    right[2] = fCos * right[2] - fSin * temp;
    pitch = vectosignedpitch(right);
    if (right[1] >= 0.0)
    {
        angles[2] = -pitch;
    }
    else
    {
        if (pitch >= 0.0)
            v2 = -180.0;
        else
            v2 = 180.0;
        angles[2] = pitch + v2;
    }
}

int __cdecl IntersectPlanes(const float **plane, float *xyz)
{
    double invDeterminant; // [esp+0h] [ebp-28h]
    double determinant; // [esp+8h] [ebp-20h]

    determinant = (plane[1][1] * plane[2][2] - plane[2][1] * plane[1][2]) * **plane
        + (plane[2][1] * (*plane)[2] - (*plane)[1] * plane[2][2]) * *plane[1]
        + ((*plane)[1] * plane[1][2] - plane[1][1] * (*plane)[2]) * *plane[2];
    if (fabs(determinant) < 0.001000000047497451)
        return 0;
    invDeterminant = 1.0 / determinant;
    *xyz = ((plane[1][1] * plane[2][2] - plane[2][1] * plane[1][2]) * (*plane)[3]
        + (plane[2][1] * (*plane)[2] - (*plane)[1] * plane[2][2]) * plane[1][3]
        + ((*plane)[1] * plane[1][2] - plane[1][1] * (*plane)[2]) * plane[2][3])
        * invDeterminant;
    xyz[1] = ((plane[1][2] * *plane[2] - plane[2][2] * *plane[1]) * (*plane)[3]
        + (plane[2][2] * **plane - (*plane)[2] * *plane[2]) * plane[1][3]
        + ((*plane)[2] * *plane[1] - plane[1][2] * **plane) * plane[2][3])
        * invDeterminant;
    xyz[2] = ((*plane[1] * plane[2][1] - *plane[2] * plane[1][1]) * (*plane)[3]
        + (*plane[2] * (*plane)[1] - **plane * plane[2][1]) * plane[1][3]
        + (**plane * plane[1][1] - *plane[1] * (*plane)[1]) * plane[2][3])
        * invDeterminant;
    return 1;
}

void __cdecl SnapPointToIntersectingPlanes(const float **planes, float *xyz, float snapGrid, float snapEpsilon)
{
    float v4; // [esp+0h] [ebp-68h]
    double v5; // [esp+4h] [ebp-64h]
    float v6; // [esp+Ch] [ebp-5Ch]
    double v7; // [esp+10h] [ebp-58h]
    float v9; // [esp+1Ch] [ebp-4Ch]
    float v10; // [esp+20h] [ebp-48h]
    float v11; // [esp+24h] [ebp-44h]
    float v12; // [esp+28h] [ebp-40h]
    float v13; // [esp+30h] [ebp-38h]
    float snapped[3]; // [esp+40h] [ebp-28h] BYREF
    float baseError; // [esp+4Ch] [ebp-1Ch]
    float maxBaseError; // [esp+50h] [ebp-18h]
    float snapError; // [esp+54h] [ebp-14h]
    float maxSnapError; // [esp+58h] [ebp-10h]
    float rounded; // [esp+5Ch] [ebp-Ch]
    int axis; // [esp+60h] [ebp-8h]
    int planeIndex; // [esp+64h] [ebp-4h]

    for (axis = 0; axis < 3; ++axis)
    {
        v13 = xyz[axis] / snapGrid;
        rounded = (double)(int)(v13 + 9.313225746154785e-10) * snapGrid;
        v12 = rounded - xyz[axis];
        v9 = fabs(v12);
        if (snapEpsilon <= (double)v9)
            snapped[axis] = xyz[axis];
        else
            snapped[axis] = rounded;
    }
    if (*xyz != snapped[0] || xyz[1] != snapped[1] || xyz[2] != snapped[2])
    {
        maxSnapError = 0.0;
        maxBaseError = snapEpsilon;
        for (planeIndex = 0; planeIndex < 3; ++planeIndex)
        {
            v7 = planes[planeIndex][3];
            v11 = Vec3Dot(planes[planeIndex], snapped) - v7;
            v6 = fabs(v11);
            snapError = v6;
            if (v6 > (double)maxSnapError)
                maxSnapError = snapError;
            v5 = planes[planeIndex][3];
            v10 = Vec3Dot(planes[planeIndex], xyz) - v5;
            v4 = fabs(v10);
            baseError = v4;
            if (v4 > (double)maxBaseError)
                maxBaseError = baseError;
        }
        if (maxBaseError > (double)maxSnapError)
        {
            *xyz = snapped[0];
            xyz[1] = snapped[1];
            xyz[2] = snapped[2];
        }
    }
}

int __cdecl ProjectedWindingContainsCoplanarPoint(
    const float (*verts)[3],
    int vertCount,
    int x,
    int y,
    const float *point)
{
    float projectionDist; // [esp+0h] [ebp-1Ch]
    float edgeNormal; // [esp+4h] [ebp-18h]
    float edgeNormal_4; // [esp+8h] [ebp-14h]
    int vertIndex; // [esp+Ch] [ebp-10h]
    int vertIndexPrev; // [esp+10h] [ebp-Ch]
    float pointDelta; // [esp+14h] [ebp-8h]
    float pointDelta_4; // [esp+18h] [ebp-4h]

    vertIndexPrev = vertCount - 1;
    for (vertIndex = 0; vertIndex < vertCount; ++vertIndex)
    {
        edgeNormal = (float)(*verts)[3 * vertIndex + y] - (float)(*verts)[3 * vertIndexPrev + y];
        edgeNormal_4 = (float)(*verts)[3 * vertIndexPrev + x] - (float)(*verts)[3 * vertIndex + x];
        pointDelta = point[x] - (float)(*verts)[3 * vertIndexPrev + x];
        pointDelta_4 = point[y] - (float)(*verts)[3 * vertIndexPrev + y];
        projectionDist = edgeNormal_4 * pointDelta_4 + edgeNormal * pointDelta;
        if (projectionDist < 0.0)
            return 0;
        vertIndexPrev = vertIndex;
    }
    return 1;
}

int __cdecl PlaneFromPoints(float *plane, const float *v0, const float *v1, const float *v2)
{
    double v5; // st7
    double v6; // st7
    float v7; // [esp+0h] [ebp-34h]
    double v8; // [esp+4h] [ebp-30h]
    double v9; // [esp+Ch] [ebp-28h]
    float v2_v0[3]; // [esp+14h] [ebp-20h] BYREF
    float v1_v0[3]; // [esp+20h] [ebp-14h] BYREF
    float length; // [esp+2Ch] [ebp-8h]
    float lengthSq; // [esp+30h] [ebp-4h]

    Vec3Sub(v1, v0, v1_v0);
    Vec3Sub(v2, v0, v2_v0);
    Vec3Cross(v2_v0, v1_v0, plane);
    lengthSq = Vec3LengthSq(plane);
    if (lengthSq >= 2.0)
        goto LABEL_7;
    if (lengthSq == 0.0)
        return 0;
    v9 = Vec3LengthSq(v1_v0);
    v5 = Vec3LengthSq(v2_v0);
    if (lengthSq <= v5 * v9 * 0.00000100000011116208)
    {
        Vec3Sub(v2, v1, v1_v0);
        Vec3Sub(v0, v1, v2_v0);
        Vec3Cross(v2_v0, v1_v0, plane);
        v8 = Vec3LengthSq(v1_v0);
        v6 = Vec3LengthSq(v2_v0);
        if (lengthSq <= v6 * v8 * 0.00000100000011116208)
            return 0;
    }
LABEL_7:
    v7 = sqrt(lengthSq);
    length = v7;
    *plane = *plane / v7;
    plane[1] = plane[1] / length;
    plane[2] = plane[2] / length;
    plane[3] = Vec3Dot(v0, plane);
    return 1;
}

void __cdecl ProjectPointOnPlane(const float *p, __int64 normal)
{
    const char *v2; // eax
    double v3; // [esp+18h] [ebp-14h]
    float d; // [esp+28h] [ebp-4h]

    if (!Vec3IsNormalized((const float *)normal))
    {
        v3 = Vec3Length((const float *)normal);
        v2 = va("(%g %g %g) len %g", *(float *)normal, *(float *)(normal + 4), *(float *)(normal + 8), v3);
        MyAssertHandler(".\\universal\\com_math.cpp", 3139, 0, "%s\n\t%s", "Vec3IsNormalized( normal )", v2);
    }
    d = -Vec3Dot((const float *)normal, p);
    Vec3Mad(p, d, (const float *)normal, (float *)HIunsigned int(normal));
}

void __cdecl SetPlaneSignbits(cplane_s *out)
{
    int j; // [esp+0h] [ebp-8h]
    unsigned __int8 bits; // [esp+7h] [ebp-1h]

    bits = 0;
    for (j = 0; j < 3; ++j)
    {
        if (out->normal[j] < 0.0)
            bits |= 1 << j;
    }
    out->signbits = bits;
}

bool __cdecl BoxDistSqrdExceeds(const float *absmin, const float *absmax, const float *org, float fogOpaqueDistSqrd)
{
    float v5; // [esp+4h] [ebp-2Ch]
    float minsSqrd; // [esp+8h] [ebp-28h]
    float mins[3]; // [esp+Ch] [ebp-24h] BYREF
    float total; // [esp+18h] [ebp-18h]
    float maxs[3]; // [esp+1Ch] [ebp-14h] BYREF
    int i; // [esp+28h] [ebp-8h]
    float maxsSqrd; // [esp+2Ch] [ebp-4h]

    Vec3Sub(absmin, org, mins);
    Vec3Sub(absmax, org, maxs);
    total = 0.0;
    for (i = 0; i < 3; ++i)
    {
        if (mins[i] * maxs[i] > 0.0)
        {
            minsSqrd = mins[i] * mins[i];
            maxsSqrd = maxs[i] * maxs[i];
            if (minsSqrd <= (double)maxsSqrd)
                v5 = minsSqrd;
            else
                v5 = maxsSqrd;
            total = total + v5;
        }
    }
    return fogOpaqueDistSqrd < (double)total;
}

double __cdecl Q_rint(float in)
{
    float v3; // [esp+Ch] [ebp-4h]

    v3 = in + 0.5;
    return (float)floor(v3);
}

double __cdecl ColorNormalize(float *in, float *out)
{
    float max; // [esp+8h] [ebp-8h]
    float scale; // [esp+Ch] [ebp-4h]

    max = *in;
    if (max < (double)in[1])
        max = in[1];
    if (max < (double)in[2])
        max = in[2];
    if (max == 0.0)
    {
        out[2] = 1.0;
        out[1] = 1.0;
        *out = 1.0;
        return 0.0;
    }
    else
    {
        scale = 1.0 / max;
        Vec3Scale(in, scale, out);
        return max;
    }
}

double __cdecl PitchForYawOnNormal(float fYaw, const float *normal)
{
    float v4; // [esp+10h] [ebp-14h]
    float forward[3]; // [esp+18h] [ebp-Ch] BYREF

    if (*normal == 0.0 && normal[1] == 0.0 && normal[2] == 0.0)
        MyAssertHandler(".\\universal\\com_math.cpp", 3693, 0, "%s", "normal[0] || normal[1] || normal[2]");
    YawVectors(fYaw, forward, 0);
    if (normal[2] == 0.0)
        return 270.0;
    forward[2] = (*normal * forward[0] + normal[1] * forward[1]) / normal[2];
    v4 = atan(forward[2]);
    return (float)(v4 * 180.0 / 3.141592741012573);
}

void __cdecl NearestPitchAndYawOnPlane(const float *angles, const float *normal, float *result)
{
    float projected[3]; // [esp+0h] [ebp-18h] BYREF
    float forward[3]; // [esp+Ch] [ebp-Ch] BYREF

    if (*normal == 0.0 && normal[1] == 0.0 && normal[2] == 0.0)
        MyAssertHandler(".\\universal\\com_math.cpp", 3724, 0, "%s", "normal[0] || normal[1] || normal[2]");
    AngleVectors(angles, forward, 0, 0);
    ProjectPointOnPlane(forward, __SPAIR64__(projected, (unsigned int)normal));
    vectoangles(projected, result);
}

void __cdecl Rand_Init(int seed)
{
    holdrand = seed;
}

double __cdecl flrand(float min, float max)
{
    float result; // [esp+8h] [ebp-4h]

    holdrand = 214013 * holdrand + 2531011;
    result = (float)(holdrand >> 17);
    return (float)((max - min) * result / 32768.0 + min);
}

int __cdecl irand(int min, int max)
{
    holdrand = 214013 * holdrand + 2531011;
    return (((holdrand >> 17) * (__int64)(max - min)) >> 15) + min;
}

void __cdecl MatrixTransformVectorQuatTrans(const float *in, const DObjAnimMat *mat, float *out)
{
    float v3; // [esp+1Ch] [ebp-54h]
    float v4; // [esp+20h] [ebp-50h]
    float v5; // [esp+24h] [ebp-4Ch]
    float v6; // [esp+28h] [ebp-48h]
    float result; // [esp+2Ch] [ebp-44h] BYREF
    float v8; // [esp+30h] [ebp-40h]
    float v9; // [esp+34h] [ebp-3Ch]
    float v10; // [esp+38h] [ebp-38h]
    float v11; // [esp+3Ch] [ebp-34h]
    float v12; // [esp+40h] [ebp-30h]
    float v13; // [esp+44h] [ebp-2Ch]
    float v14; // [esp+48h] [ebp-28h]
    float v15; // [esp+4Ch] [ebp-24h]
    float v16; // [esp+50h] [ebp-20h]
    float v17; // [esp+54h] [ebp-1Ch]
    float v18; // [esp+58h] [ebp-18h]
    float v19; // [esp+5Ch] [ebp-14h]
    float v20; // [esp+60h] [ebp-10h]
    float v21; // [esp+64h] [ebp-Ch]
    float v22; // [esp+68h] [ebp-8h]
    float v23; // [esp+6Ch] [ebp-4h]

    if ((COERCE_UNSIGNED_INT(mat->quat[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mat->quat[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mat->quat[2]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mat->quat[3]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\universal\\../xanim/xanim_public.h",
            432,
            0,
            "%s",
            "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
    }
    if ((COERCE_UNSIGNED_INT(mat->transWeight) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\universal\\../xanim/xanim_public.h",
            433,
            0,
            "%s",
            "!IS_NAN(mat->transWeight)");
    Vec3Scale(mat->quat, mat->transWeight, &result);
    v13 = result * mat->quat[0];
    v4 = result * mat->quat[1];
    v11 = result * mat->quat[2];
    v14 = result * mat->quat[3];
    v3 = v8 * mat->quat[1];
    v12 = v8 * mat->quat[2];
    v10 = v8 * mat->quat[3];
    v5 = v9 * mat->quat[2];
    v6 = v9 * mat->quat[3];
    v15 = 1.0 - (v3 + v5);
    v16 = v4 + v6;
    v17 = v11 - v10;
    v18 = v4 - v6;
    v19 = 1.0 - (v13 + v5);
    v20 = v12 + v14;
    v21 = v11 + v10;
    v22 = v12 - v14;
    v23 = 1.0 - (v13 + v3);
    *out = *in * v15 + in[1] * v18 + in[2] * v21 + mat->trans[0];
    out[1] = *in * v16 + in[1] * v19 + in[2] * v22 + mat->trans[1];
    out[2] = *in * v17 + in[1] * v20 + in[2] * v23 + mat->trans[2];
}

void __cdecl AxisToQuat(const float (*mat)[3], float *out)
{
    float v2; // [esp+8h] [ebp-50h]
    float invLength; // [esp+Ch] [ebp-4Ch]
    float test[4][4]; // [esp+10h] [ebp-48h] BYREF
    int best; // [esp+50h] [ebp-8h]
    float testSizeSq; // [esp+54h] [ebp-4h]

    test[0][0] = (float)(*mat)[5] - (float)(*mat)[7];
    test[0][1] = (float)(*mat)[6] - (float)(*mat)[2];
    test[0][2] = (float)(*mat)[1] - (float)(*mat)[3];
    test[0][3] = (*mat)[0] + (float)(*mat)[4] + (float)(*mat)[8] + 1.0;
    testSizeSq = Vec4LengthSq(test[0]);
    if (testSizeSq < 1.0)
    {
        test[1][0] = (float)(*mat)[6] + (float)(*mat)[2];
        test[1][1] = (float)(*mat)[7] + (float)(*mat)[5];
        test[1][2] = (float)(*mat)[8] - (float)(*mat)[4] - (*mat)[0] + 1.0;
        test[1][3] = test[0][2];
        testSizeSq = Vec4LengthSq(test[1]);
        if (testSizeSq < 1.0)
        {
            test[2][0] = (*mat)[0] - (float)(*mat)[4] - (float)(*mat)[8] + 1.0;
            test[2][1] = (float)(*mat)[3] + (float)(*mat)[1];
            test[2][2] = test[1][0];
            test[2][3] = test[0][0];
            testSizeSq = Vec4LengthSq(test[2]);
            if (testSizeSq < 1.0)
            {
                test[3][0] = test[2][1];
                test[3][1] = (float)(*mat)[4] - (*mat)[0] - (float)(*mat)[8] + 1.0;
                test[3][2] = test[1][1];
                test[3][3] = test[0][1];
                testSizeSq = Vec4LengthSq(test[3]);
                if (testSizeSq < 1.0)
                    MyAssertHandler(
                        ".\\universal\\com_math.cpp",
                        3832,
                        0,
                        "%s\n\t(testSizeSq) = %g",
                        "(testSizeSq >= 1.0f)",
                        testSizeSq);
                best = 3;
            }
            else
            {
                best = 2;
            }
        }
        else
        {
            best = 1;
        }
    }
    else
    {
        best = 0;
    }
    if (testSizeSq == 0.0)
        MyAssertHandler(".\\universal\\com_math.cpp", 3837, 0, "%s", "testSizeSq");
    v2 = sqrt(testSizeSq);
    invLength = 1.0 / v2;
    Vec4Scale(test[best], invLength, out);
}

void __cdecl QuatLerp(const float *qa, const float *qb, float frac, float *out)
{
    float dot; // [esp+8h] [ebp-4h]

    dot = Vec4Dot(qa, qb);
    if (dot < 0.0)
    {
        *out = -*qb;
        out[1] = -qb[1];
        out[2] = -qb[2];
        out[3] = -qb[3];
        Vec4Lerp(qa, out, frac, out);
    }
    else
    {
        Vec4Lerp(qa, qb, frac, out);
    }
}

bool __cdecl CullBoxFromCone(
    const float *coneOrg,
    const float *coneDir,
    float cosHalfFov,
    const float *boxCenter,
    const float *boxHalfSize)
{
    float v6; // [esp+0h] [ebp-88h]
    float v7; // [esp+Ch] [ebp-7Ch]
    float v8; // [esp+10h] [ebp-78h]
    float v9; // [esp+14h] [ebp-74h]
    float v10; // [esp+18h] [ebp-70h]
    float v11; // [esp+24h] [ebp-64h]
    float v12; // [esp+28h] [ebp-60h]
    float v13; // [esp+2Ch] [ebp-5Ch]
    float v14; // [esp+30h] [ebp-58h]
    float v15; // [esp+34h] [ebp-54h]
    float v16; // [esp+38h] [ebp-50h]
    float v17; // [esp+3Ch] [ebp-4Ch]
    float cosHalfFovSq; // [esp+40h] [ebp-48h]
    float scaledSepAxis[3]; // [esp+44h] [ebp-44h] BYREF
    float dist; // [esp+50h] [ebp-38h]
    float deltaMid[3]; // [esp+54h] [ebp-34h] BYREF
    float sinHalfFovSq; // [esp+60h] [ebp-28h]
    float scaledSepDist; // [esp+64h] [ebp-24h]
    float perpendicular[3]; // [esp+68h] [ebp-20h] BYREF
    float scale; // [esp+74h] [ebp-14h]
    float perpLenSq; // [esp+78h] [ebp-10h]
    float farCorner[3]; // [esp+7Ch] [ebp-Ch] BYREF

    if (cosHalfFov < 0.0)
        MyAssertHandler(".\\universal\\com_math.cpp", 3960, 0, "%s", "cosHalfFov >= 0.0f");
    Vec3Sub(boxCenter, coneOrg, deltaMid);
    if (*coneDir < 0.0)
        v14 = -1.0;
    else
        v14 = 1.0;
    farCorner[0] = deltaMid[0] - *boxHalfSize * v14;
    if (coneDir[1] < 0.0)
        v13 = -1.0;
    else
        v13 = 1.0;
    farCorner[1] = deltaMid[1] - boxHalfSize[1] * v13;
    if (coneDir[2] < 0.0)
        v12 = -1.0;
    else
        v12 = 1.0;
    farCorner[2] = deltaMid[2] - boxHalfSize[2] * v12;
    dist = Vec3Dot(farCorner, coneDir);
    if (dist >= 0.0)
        return 1;
    v6 = -dist;
    Vec3Mad(farCorner, v6, coneDir, perpendicular);
    perpLenSq = Vec3LengthSq(perpendicular);
    cosHalfFovSq = cosHalfFov * cosHalfFov;
    sinHalfFovSq = 1.0 - cosHalfFovSq;
    if (dist * dist * sinHalfFovSq >= perpLenSq * cosHalfFovSq)
        return 0;
    v11 = perpLenSq * sinHalfFovSq;
    v10 = sqrt(v11);
    scale = cosHalfFov / v10;
    Vec3Mad(coneDir, scale, perpendicular, scaledSepAxis);
    scaledSepDist = Vec3Dot(scaledSepAxis, deltaMid);
    v17 = scaledSepAxis[0] * *boxHalfSize;
    v9 = fabs(v17);
    scaledSepDist = scaledSepDist - v9;
    v16 = scaledSepAxis[1] * boxHalfSize[1];
    v8 = fabs(v16);
    scaledSepDist = scaledSepDist - v8;
    v15 = scaledSepAxis[2] * boxHalfSize[2];
    v7 = fabs(v15);
    scaledSepDist = scaledSepDist - v7;
    return scaledSepDist >= 0.0;
}

bool __cdecl CullBoxFromSphere(const float *sphereOrg, float radius, const float *boxCenter, const float *boxHalfSize)
{
    float v5; // [esp+0h] [ebp-54h]
    float v6; // [esp+4h] [ebp-50h]
    float v7; // [esp+8h] [ebp-4Ch]
    float v8; // [esp+Ch] [ebp-48h]
    float v9; // [esp+10h] [ebp-44h]
    float v10; // [esp+14h] [ebp-40h]
    float v11; // [esp+18h] [ebp-3Ch]
    float v12; // [esp+1Ch] [ebp-38h]
    float v13; // [esp+20h] [ebp-34h]
    float v14; // [esp+24h] [ebp-30h]
    float v15; // [esp+2Ch] [ebp-28h]
    float v16; // [esp+30h] [ebp-24h]
    float v17; // [esp+38h] [ebp-1Ch]
    float v18; // [esp+3Ch] [ebp-18h]
    float v19; // [esp+44h] [ebp-10h]
    float distFromBoxToMid[3]; // [esp+48h] [ebp-Ch] BYREF

    v19 = *sphereOrg - *boxCenter;
    v13 = fabs(v19);
    v18 = v13 - *boxHalfSize;
    v12 = v18 - 0.0;
    if (v12 < 0.0)
        v11 = 0.0;
    else
        v11 = v13 - *boxHalfSize;
    distFromBoxToMid[0] = v11;
    v17 = sphereOrg[1] - boxCenter[1];
    v10 = fabs(v17);
    v16 = v10 - boxHalfSize[1];
    v9 = v16 - 0.0;
    if (v9 < 0.0)
        v8 = 0.0;
    else
        v8 = v10 - boxHalfSize[1];
    distFromBoxToMid[1] = v8;
    v15 = sphereOrg[2] - boxCenter[2];
    v7 = fabs(v15);
    v14 = v7 - boxHalfSize[2];
    v6 = v14 - 0.0;
    if (v6 < 0.0)
        v5 = 0.0;
    else
        v5 = v7 - boxHalfSize[2];
    distFromBoxToMid[2] = v5;
    return radius * radius < Vec3LengthSq(distFromBoxToMid);
}

bool __cdecl CullBoxFromConicSectionOfSphere(
    const float *coneOrg,
    const float *coneDir,
    float cosHalfFov,
    float radius,
    const float *boxCenter,
    const float *boxHalfSize)
{
    float v7; // [esp+0h] [ebp-DCh]
    float v8; // [esp+Ch] [ebp-D0h]
    float v9; // [esp+10h] [ebp-CCh]
    float v10; // [esp+14h] [ebp-C8h]
    float v11; // [esp+18h] [ebp-C4h]
    float v12; // [esp+24h] [ebp-B8h]
    float v13; // [esp+28h] [ebp-B4h]
    float v14; // [esp+2Ch] [ebp-B0h]
    float v15; // [esp+30h] [ebp-ACh]
    float v16; // [esp+34h] [ebp-A8h]
    float v17; // [esp+38h] [ebp-A4h]
    float v18; // [esp+3Ch] [ebp-A0h]
    float v19; // [esp+40h] [ebp-9Ch]
    float v20; // [esp+44h] [ebp-98h]
    float v21; // [esp+48h] [ebp-94h]
    float v22; // [esp+4Ch] [ebp-90h]
    float v23; // [esp+50h] [ebp-8Ch]
    float v24; // [esp+54h] [ebp-88h]
    float v25; // [esp+58h] [ebp-84h]
    float v26; // [esp+5Ch] [ebp-80h]
    float v27; // [esp+60h] [ebp-7Ch]
    float v28; // [esp+64h] [ebp-78h]
    float v29; // [esp+70h] [ebp-6Ch]
    float v30; // [esp+7Ch] [ebp-60h]
    float cosHalfFovSq; // [esp+88h] [ebp-54h]
    float scaledSepAxis[3]; // [esp+8Ch] [ebp-50h] BYREF
    float dist; // [esp+98h] [ebp-44h]
    float deltaMid[3]; // [esp+9Ch] [ebp-40h] BYREF
    float sinHalfFovSq; // [esp+A8h] [ebp-34h]
    float scaledSepDist; // [esp+ACh] [ebp-30h]
    float distFromBoxToMid[3]; // [esp+B0h] [ebp-2Ch] BYREF
    float perpendicular[3]; // [esp+BCh] [ebp-20h] BYREF
    float scale; // [esp+C8h] [ebp-14h]
    float perpLenSq; // [esp+CCh] [ebp-10h]
    float farCorner[3]; // [esp+D0h] [ebp-Ch] BYREF

    if (cosHalfFov < 0.0)
        MyAssertHandler(".\\universal\\com_math.cpp", 4035, 0, "%s", "cosHalfFov >= 0.0f");
    Vec3Sub(boxCenter, coneOrg, deltaMid);
    v24 = fabs(deltaMid[0]);
    v30 = v24 - *boxHalfSize;
    v23 = v30 - 0.0;
    if (v23 < 0.0)
        v22 = 0.0;
    else
        v22 = v24 - *boxHalfSize;
    distFromBoxToMid[0] = v22;
    v21 = fabs(deltaMid[1]);
    v29 = v21 - boxHalfSize[1];
    v20 = v29 - 0.0;
    if (v20 < 0.0)
        v19 = 0.0;
    else
        v19 = v21 - boxHalfSize[1];
    distFromBoxToMid[1] = v19;
    v18 = fabs(deltaMid[2]);
    v28 = v18 - boxHalfSize[2];
    v17 = v28 - 0.0;
    if (v17 < 0.0)
        v16 = 0.0;
    else
        v16 = v18 - boxHalfSize[2];
    distFromBoxToMid[2] = v16;
    if (radius * radius < Vec3LengthSq(distFromBoxToMid))
        return 1;
    if (*coneDir < 0.0)
        v15 = -1.0;
    else
        v15 = 1.0;
    farCorner[0] = deltaMid[0] - *boxHalfSize * v15;
    if (coneDir[1] < 0.0)
        v14 = -1.0;
    else
        v14 = 1.0;
    farCorner[1] = deltaMid[1] - boxHalfSize[1] * v14;
    if (coneDir[2] < 0.0)
        v13 = -1.0;
    else
        v13 = 1.0;
    farCorner[2] = deltaMid[2] - boxHalfSize[2] * v13;
    dist = Vec3Dot(farCorner, coneDir);
    if (dist >= 0.0)
        return 1;
    v7 = -dist;
    Vec3Mad(farCorner, v7, coneDir, perpendicular);
    perpLenSq = Vec3LengthSq(perpendicular);
    cosHalfFovSq = cosHalfFov * cosHalfFov;
    sinHalfFovSq = 1.0 - cosHalfFovSq;
    if (dist * dist * sinHalfFovSq >= perpLenSq * cosHalfFovSq)
        return 0;
    v12 = perpLenSq * sinHalfFovSq;
    v11 = sqrt(v12);
    scale = cosHalfFov / v11;
    Vec3Mad(coneDir, scale, perpendicular, scaledSepAxis);
    scaledSepDist = Vec3Dot(scaledSepAxis, deltaMid);
    v27 = scaledSepAxis[0] * *boxHalfSize;
    v10 = fabs(v27);
    scaledSepDist = scaledSepDist - v10;
    v26 = scaledSepAxis[1] * boxHalfSize[1];
    v9 = fabs(v26);
    scaledSepDist = scaledSepDist - v9;
    v25 = scaledSepAxis[2] * boxHalfSize[2];
    v8 = fabs(v25);
    scaledSepDist = scaledSepDist - v8;
    return scaledSepDist >= 0.0;
}

bool __cdecl CullSphereFromCone(
    const float *coneOrg,
    const float *coneDir,
    float cosHalfFov,
    const float *sphereCenter,
    float radius)
{
    float scale; // [esp+0h] [ebp-3Ch]
    float v7; // [esp+Ch] [ebp-30h]
    float cosHalfFovSq; // [esp+10h] [ebp-2Ch]
    float delta[3]; // [esp+14h] [ebp-28h] BYREF
    float dist; // [esp+20h] [ebp-1Ch]
    float discriminant; // [esp+24h] [ebp-18h]
    float sinHalfFovSq; // [esp+28h] [ebp-14h]
    float perpendicular[3]; // [esp+2Ch] [ebp-10h] BYREF
    float perpLenSq; // [esp+38h] [ebp-4h]

    if (cosHalfFov < 0.0)
        MyAssertHandler(".\\universal\\com_math.cpp", 4098, 0, "%s", "cosHalfFov >= 0.0f");
    Vec3Sub(sphereCenter, coneOrg, delta);
    dist = Vec3Dot(delta, coneDir);
    if (radius <= (double)dist)
        return 1;
    scale = -dist;
    Vec3Mad(delta, scale, coneDir, perpendicular);
    perpLenSq = Vec3LengthSq(perpendicular);
    cosHalfFovSq = cosHalfFov * cosHalfFov;
    sinHalfFovSq = 1.0 - cosHalfFovSq;
    v7 = sqrt(sinHalfFovSq);
    discriminant = v7 * dist - radius;
    return discriminant * discriminant <= perpLenSq * cosHalfFovSq;
}

void __cdecl AxisCopy(const float (*in)[3], float (*out)[3])
{
    (*out)[0] = (*in)[0];
    (*out)[1] = (*in)[1];
    (*out)[2] = (*in)[2];
    (*out)[3] = (*in)[3];
    (*out)[4] = (*in)[4];
    (*out)[5] = (*in)[5];
    (*out)[6] = (*in)[6];
    (*out)[7] = (*in)[7];
    (*out)[8] = (*in)[8];
}

double __cdecl log(double X)
{
    int v1; // eax
    bool v2; // zf
    char v4; // [esp+0h] [ebp-8h]

    if (__use_sse2_mathfcns)
    {
        v1 = _mm_getcsr() & 0x1F80;
        v2 = v1 == 8064;
        if (v1 == 8064)
            v2 = (v4 & 0x7F) == 127;
        if (v2)
            JUMPOUT(0x8453A8);
    }
    JUMPOUT(0x8353CF);
}

