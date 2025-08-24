#include "com_math.h"

#include <universal/assertive.h>
#include <universal/q_shared.h>
#include <qcommon/mem_track.h>
#include <math.h>
#include <xanim/dobj.h>
#include <stdlib.h>
#include "q_shared.h"

//Line 51773:  0006 : 0000bc58       float (*)[3] bytedirs        827bbc58     com_math.obj
//Line 53450 : 0006 : 0291d360       int marker_com_math      850cd360     com_math.obj

static unsigned int holdrand;

float bytedirs[162][3] =
{
  { -0.52573103f, 0.0f, 0.85065103f },
  { -0.44286299f, 0.238856f, 0.86418802f },
  { -0.29524201f, 0.0f, 0.955423f },
  { -0.309017f, 0.5f, 0.809017f },
  { -0.16246f, 0.26286599f, 0.951056f },
  { 0.0f, 0.0f, 1.0f },
  { 0.0f, 0.85065103f, 0.52573103f },
  { -0.14762101f, 0.71656698f, 0.68171799f },
  { 0.14762101f, 0.71656698f, 0.68171799f },
  { 0.0f, 0.52573103f, 0.85065103f },
  { 0.309017f, 0.5f, 0.809017f },
  { 0.52573103f, 0.0f, 0.85065103f },
  { 0.29524201f, 0.0f, 0.955423f },
  { 0.44286299f, 0.238856f, 0.86418802f },
  { 0.16246f, 0.26286599f, 0.951056f },
  { -0.68171799f, 0.14762101f, 0.71656698f },
  { -0.809017f, 0.309017f, 0.5f },
  { -0.58778501f, 0.42532501f, 0.688191f },
  { -0.85065103f, 0.52573103f, 0.0f },
  { -0.86418802f, 0.44286299f, 0.238856f },
  { -0.71656698f, 0.68171799f, 0.14762101f },
  { -0.688191f, 0.58778501f, 0.42532501f },
  { -0.5f, 0.809017f, 0.309017f },
  { -0.238856f, 0.86418802f, 0.44286299f },
  { -0.42532501f, 0.688191f, 0.58778501f },
  { -0.71656698f, 0.68171799f, -0.14762101f },
  { -0.5f, 0.809017f, -0.309017f },
  { -0.52573103f, 0.85065103f, 0.0f },
  { 0.0f, 0.85065103f, -0.52573103f },
  { -0.238856f, 0.86418802f, -0.44286299f },
  { 0.0f, 0.955423f, -0.29524201f },
  { -0.26286599f, 0.951056f, -0.16246f },
  { 0.0f, 1.0f, 0.0f },
  { 0.0f, 0.955423f, 0.29524201f },
  { -0.26286599f, 0.951056f, 0.16246f },
  { 0.238856f, 0.86418802f, 0.44286299f },
  { 0.26286599f, 0.951056f, 0.16246f },
  { 0.5f, 0.809017f, 0.309017f },
  { 0.238856f, 0.86418802f, -0.44286299f },
  { 0.26286599f, 0.951056f, -0.16246f },
  { 0.5f, 0.809017f, -0.309017f },
  { 0.85065103f, 0.52573103f, 0.0f },
  { 0.71656698f, 0.68171799f, 0.14762101f },
  { 0.71656698f, 0.68171799f, -0.14762101f },
  { 0.52573103f, 0.85065103f, 0.0f },
  { 0.42532501f, 0.688191f, 0.58778501f },
  { 0.86418802f, 0.44286299f, 0.238856f },
  { 0.688191f, 0.58778501f, 0.42532501f },
  { 0.809017f, 0.309017f, 0.5f },
  { 0.68171799f, 0.14762101f, 0.71656698f },
  { 0.58778501f, 0.42532501f, 0.688191f },
  { 0.955423f, 0.29524201f, 0.0f },
  { 1.0f, 0.0f, 0.0f },
  { 0.951056f, 0.16246f, 0.26286599f },
  { 0.85065103f, -0.52573103f, 0.0f },
  { 0.955423f, -0.29524201f, 0.0f },
  { 0.86418802f, -0.44286299f, 0.238856f },
  { 0.951056f, -0.16246f, 0.26286599f },
  { 0.809017f, -0.309017f, 0.5f },
  { 0.68171799f, -0.14762101f, 0.71656698f },
  { 0.85065103f, 0.0f, 0.52573103f },
  { 0.86418802f, 0.44286299f, -0.238856f },
  { 0.809017f, 0.309017f, -0.5f },
  { 0.951056f, 0.16246f, -0.26286599f },
  { 0.52573103f, 0.0f, -0.85065103f },
  { 0.68171799f, 0.14762101f, -0.71656698f },
  { 0.68171799f, -0.14762101f, -0.71656698f },
  { 0.85065103f, 0.0f, -0.52573103f },
  { 0.809017f, -0.309017f, -0.5f },
  { 0.86418802f, -0.44286299f, -0.238856f },
  { 0.951056f, -0.16246f, -0.26286599f },
  { 0.14762101f, 0.71656698f, -0.68171799f },
  { 0.309017f, 0.5f, -0.809017f },
  { 0.42532501f, 0.688191f, -0.58778501f },
  { 0.44286299f, 0.238856f, -0.86418802f },
  { 0.58778501f, 0.42532501f, -0.688191f },
  { 0.688191f, 0.58778501f, -0.42532501f },
  { -0.14762101f, 0.71656698f, -0.68171799f },
  { -0.309017f, 0.5f, -0.809017f },
  { 0.0f, 0.52573103f, -0.85065103f },
  { -0.52573103f, 0.0f, -0.85065103f },
  { -0.44286299f, 0.238856f, -0.86418802f },
  { -0.29524201f, 0.0f, -0.955423f },
  { -0.16246f, 0.26286599f, -0.951056f },
  { 0.0f, 0.0f, -1.0f },
  { 0.29524201f, 0.0f, -0.955423f },
  { 0.16246f, 0.26286599f, -0.951056f },
  { -0.44286299f, -0.238856f, -0.86418802f },
  { -0.309017f, -0.5f, -0.809017f },
  { -0.16246f, -0.26286599f, -0.951056f },
  { 0.0f, -0.85065103f, -0.52573103f },
  { -0.14762101f, -0.71656698f, -0.68171799f },
  { 0.14762101f, -0.71656698f, -0.68171799f },
  { 0.0f, -0.52573103f, -0.85065103f },
  { 0.309017f, -0.5f, -0.809017f },
  { 0.44286299f, -0.238856f, -0.86418802f },
  { 0.16246f, -0.26286599f, -0.951056f },
  { 0.238856f, -0.86418802f, -0.44286299f },
  { 0.5f, -0.809017f, -0.309017f },
  { 0.42532501f, -0.688191f, -0.58778501f },
  { 0.71656698f, -0.68171799f, -0.14762101f },
  { 0.688191f, -0.58778501f, -0.42532501f },
  { 0.58778501f, -0.42532501f, -0.688191f },
  { 0.0f, -0.955423f, -0.29524201f },
  { 0.0f, -1.0f, 0.0f },
  { 0.26286599f, -0.951056f, -0.16246f },
  { 0.0f, -0.85065103f, 0.52573103f },
  { 0.0f, -0.955423f, 0.29524201f },
  { 0.238856f, -0.86418802f, 0.44286299f },
  { 0.26286599f, -0.951056f, 0.16246f },
  { 0.5f, -0.809017f, 0.309017f },
  { 0.71656698f, -0.68171799f, 0.14762101f },
  { 0.52573103f, -0.85065103f, 0.0f },
  { -0.238856f, -0.86418802f, -0.44286299f },
  { -0.5f, -0.809017f, -0.309017f },
  { -0.26286599f, -0.951056f, -0.16246f },
  { -0.85065103f, -0.52573103f, 0.0f },
  { -0.71656698f, -0.68171799f, -0.14762101f },
  { -0.71656698f, -0.68171799f, 0.14762101f },
  { -0.52573103f, -0.85065103f, 0.0f },
  { -0.5f, -0.809017f, 0.309017f },
  { -0.238856f, -0.86418802f, 0.44286299f },
  { -0.26286599f, -0.951056f, 0.16246f },
  { -0.86418802f, -0.44286299f, 0.238856f },
  { -0.809017f, -0.309017f, 0.5f },
  { -0.688191f, -0.58778501f, 0.42532501f },
  { -0.68171799f, -0.14762101f, 0.71656698f },
  { -0.44286299f, -0.238856f, 0.86418802f },
  { -0.58778501f, -0.42532501f, 0.688191f },
  { -0.309017f, -0.5f, 0.809017f },
  { -0.14762101f, -0.71656698f, 0.68171799f },
  { -0.42532501f, -0.688191f, 0.58778501f },
  { -0.16246f, -0.26286599f, 0.951056f },
  { 0.44286299f, -0.238856f, 0.86418802f },
  { 0.16246f, -0.26286599f, 0.951056f },
  { 0.309017f, -0.5f, 0.809017f },
  { 0.14762101f, -0.71656698f, 0.68171799f },
  { 0.0f, -0.52573103f, 0.85065103f },
  { 0.42532501f, -0.688191f, 0.58778501f },
  { 0.58778501f, -0.42532501f, 0.688191f },
  { 0.688191f, -0.58778501f, 0.42532501f },
  { -0.955423f, 0.29524201f, 0.0f },
  { -0.951056f, 0.16246f, 0.26286599f },
  { -1.0f, 0.0f, 0.0f },
  { -0.85065103f, 0.0f, 0.52573103f },
  { -0.955423f, -0.29524201f, 0.0f },
  { -0.951056f, -0.16246f, 0.26286599f },
  { -0.86418802f, 0.44286299f, -0.238856f },
  { -0.951056f, 0.16246f, -0.26286599f },
  { -0.809017f, 0.309017f, -0.5f },
  { -0.86418802f, -0.44286299f, -0.238856f },
  { -0.951056f, -0.16246f, -0.26286599f },
  { -0.809017f, -0.309017f, -0.5f },
  { -0.68171799f, 0.14762101f, -0.71656698f },
  { -0.68171799f, -0.14762101f, -0.71656698f },
  { -0.85065103f, 0.0f, -0.52573103f },
  { -0.688191f, 0.58778501f, -0.42532501f },
  { -0.58778501f, 0.42532501f, -0.688191f },
  { -0.42532501f, 0.688191f, -0.58778501f },
  { -0.42532501f, -0.688191f, -0.58778501f },
  { -0.58778501f, -0.42532501f, -0.688191f },
  { -0.688191f, -0.58778501f, -0.42532501f }
};


float __cdecl AngleDelta(float a1, float a2)
{
    float v4; // [esp+Ch] [ebp-10h]
    float v5; // [esp+10h] [ebp-Ch]
    float v6; // [esp+14h] [ebp-8h]
    float v7; // [esp+18h] [ebp-4h]

    v6 = a1 - a2;
    v7 = v6 * 0.002777777845039964;
    v5 = v7 + 0.5;
    v4 = floor(v5);
    return ((v7 - v4) * 360.0);
}

void __cdecl TRACK_com_math()
{
    track_static_alloc_internal(bytedirs, 1944, "bytedirs", 10);
}

float __cdecl random()
{
    return (rand() / 32768.0);
}

float __cdecl crandom()
{
    return (random() * 2.0 - 1.0);
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

float __cdecl LinearTrack(float tgt, float cur, float rate, float deltaTime)
{
    float v4; // st7
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
    v9 = I_fabs(err);
    if (v9 <= EQUAL_EPSILON)
        return tgt;
    v8 = I_fabs(err);
    v7 = I_fabs(step);
    if (v7 > v8)
        return tgt;
    return (cur + step);
}

float __cdecl LinearTrackAngle(float tgt, float cur, float rate, float deltaTime)
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
    return ((v8 - v6) * 360.0);
}

float __cdecl DiffTrack(float tgt, float cur, float rate, float deltaTime)
{
    float v6; // [esp+4h] [ebp-18h]
    float v7; // [esp+8h] [ebp-14h]
    float v8; // [esp+Ch] [ebp-10h]
    float err; // [esp+14h] [ebp-8h]
    float step; // [esp+18h] [ebp-4h]

    err = tgt - cur;
    step = rate * err * deltaTime;
    v8 = I_fabs(err);
    if (v8 <= EQUAL_EPSILON)
        return tgt;
    v7 = I_fabs(err);
    v6 = I_fabs(step);
    if (v6 > v7)
        return tgt;
    return (cur + step);
}

float __cdecl DiffTrackAngle(float tgt, float cur, float rate, float deltaTime)
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
    return ((v8 - v6) * 360.0);
}

float __cdecl GraphGetValueFromFraction(int knotCount, const float (*knots)[2], float fraction)
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
    if (knots[knotCount - 1][0] != 1.0)
        MyAssertHandler(
            ".\\universal\\com_math.cpp",
            463,
            0,
            "%s\n\t(knots[knotCount - 1][0]) = %g",
            "(knots[knotCount - 1][0] == 1.0f)",
            knots[knotCount - 1][0]);
    for (knotIndex = 1; knotIndex < knotCount; ++knotIndex)
    {
        if (knots[knotIndex][0] >= fraction)
        {
            adjustedFrac = (fraction - knots[knotIndex - 1][0]) / (knots[knotIndex][0] - knots[knotIndex - 1][0]);
            if (adjustedFrac < 0.0 || adjustedFrac > 1.0)
                MyAssertHandler(
                    ".\\universal\\com_math.cpp",
                    471,
                    0,
                    "%s\n\t(adjustedFrac) = %g",
                    "(adjustedFrac >= 0.0f && adjustedFrac <= 1.0f)",
                    adjustedFrac);
            if (knots[knotIndex - 1][1] < 0.0 || knots[knotIndex - 1][1] > 1.0)
                MyAssertHandler(
                    ".\\universal\\com_math.cpp",
                    472,
                    0,
                    "%s\n\t(knots[knotIndex - 1][1]) = %g",
                    "(knots[knotIndex - 1][1] >= 0.0f && knots[knotIndex - 1][1] <= 1.0f)",
                    knots[knotIndex - 1][1]);
            if (knots[knotIndex][1] < 0.0 || knots[knotIndex][1] > 1.0)
                MyAssertHandler(
                    ".\\universal\\com_math.cpp",
                    473,
                    0,
                    "%s\n\t(knots[knotIndex][1]) = %g",
                    "(knots[knotIndex][1] >= 0.0f && knots[knotIndex][1] <= 1.0f)",
                    knots[knotIndex][1]);
            result = (knots[knotIndex][1] - knots[knotIndex - 1][1]) * adjustedFrac + knots[knotIndex - 1][1];
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

float __cdecl Q_acos(float c)
{
    float v2; // [esp+0h] [ebp-8h]

    v2 = acos(c);
    if (v2 > 3.141592741012573)
        return 3.1415927f;
    if (v2 >= -3.141592741012573)
        return v2;
    return 3.1415927f;
}

char __cdecl ClampChar(int i)
{
    if (i < -128)
        return CHAR_MIN;
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
        if (bestd < d)
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

float __cdecl Vec2Length(const vec2r v)
{
    float v3; // [esp+4h] [ebp-4h]

    v3 = v[1] * v[1] + *v * *v;
    return sqrtf(v3);
}

float __cdecl Vec2Distance(const vec2r v1, const vec2r v2)
{
    vec2 dir; // [esp+4h] [ebp-8h] BYREF
    dir[0] = v2[0] - v1[0];
    dir[1] = v2[1] - v1[1];
    return Vec2Length(dir);
}

float __cdecl Vec2DistanceSq(const vec2r p1, const vec2r p2)
{
    float v; // [esp+4h] [ebp-8h]
    float v_4; // [esp+8h] [ebp-4h]

    v = p2[0] - p1[0];
    v_4 = p2[1] - p1[1];
    return (v * v + v_4 * v_4);
}

void __cdecl Vec3ProjectionCoords(const float *dir, int *xCoord, int *yCoord)
{
    float dirSq[3]; // [esp+0h] [ebp-Ch] BYREF

    Vec3Mul(dir, dir, dirSq);
    if (dirSq[0] > dirSq[2] || dirSq[1] > dirSq[2])
    {
        if (dirSq[0] > dirSq[1] || dirSq[2] > dirSq[1])
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

float __cdecl Vec2Normalize(vec2r v)
{
    float v2; // [esp+0h] [ebp-14h]
    float v3; // [esp+4h] [ebp-10h]
    float ilength; // [esp+Ch] [ebp-8h]
    float length; // [esp+10h] [ebp-4h]

    length = v[0] * v[0] + v[1] * v[1];
    
    v3 = sqrt(length);
    if (-v3 < 0.0)
        v2 = v3;
    else
        v2 = 1.0;
    
    ilength = 1.0 / v2;
    v[0] = v[0] * ilength;
    v[1] = v[1] * ilength;
    return v3;
}

void __cdecl Vec3NormalizeFast(float *v)
{
    int32_t number; // [esp+0h] [ebp-1Ch]
    float invLength; // [esp+18h] [ebp-4h]

    *(float *)&number = Vec3LengthSq(v);
    invLength = I_rsqrt(number);
    v[0] = v[0] * invLength;
    v[1] = v[1] * invLength;
    v[2] = v[2] * invLength;
}

float __cdecl Vec3NormalizeTo(const vec3r v, vec3r out)
{
    float v3; // [esp+0h] [ebp-14h]
    float v4; // [esp+4h] [ebp-10h]
    float ilength; // [esp+Ch] [ebp-8h]
    float length; // [esp+10h] [ebp-4h]

    length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    v4 = sqrt(length);
    if (-v4 < 0.0)
        v3 = v4;
    else
        v3 = 1.0;
    ilength = 1.0 / v3;

    out[0] = v[0] * ilength;
    out[1] = v[1] * ilength;
    out[2] = v[2] * ilength;
    return v4;
}

float __cdecl Vec2NormalizeTo(const float *v, float *out)
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

void __cdecl Vec3Rotate(const vec3r in, const mat3x3& matrix, vec3r out)
{
    iassert(in != out);

    out[0] = in[0] * (matrix)[0][0] + in[1] * (matrix)[0][1] + in[2] * (matrix)[0][2];
    out[1] = in[0] * (matrix)[1][0] + in[1] * (matrix)[1][1] + in[2] * (matrix)[1][2];
    out[2] = in[0] * (matrix)[2][0] + in[1] * (matrix)[2][1] + in[2] * (matrix)[2][2];
}

void __cdecl Vec3RotateTranspose(const vec3r in, const mat3x3& matrix, vec3r out)
{
    iassert(in != out);

    out[0] = in[0] * (matrix)[0][0] + in[1] * (matrix)[1][0] + in[2] * (matrix)[2][0];
    out[1] = in[0] * (matrix)[0][1] + in[1] * (matrix)[1][1] + in[2] * (matrix)[2][1];
    out[2] = in[0] * (matrix)[0][2] + in[1] * (matrix)[1][2] + in[2] * (matrix)[2][2];
}

void __cdecl RotatePointAroundVector(float* dst, const float* dir, const float* point, float degrees)
{
    mat3x3 m; // [esp+1Ch] [ebp-E0h] BYREF
    mat3x3 rot; // [esp+74h] [ebp-88h] BYREF

    mat3x3 tmpmat; // [esp+B4h] [ebp-48h] BYREF
    mat3x3 im; // [esp+D8h] [ebp-24h] BYREF

    float rad; // [esp+40h] [ebp-BCh]
    float vr[3]; // [esp+44h] [ebp-B8h] BYREF
    
    
    float vf[3]; // [esp+98h] [ebp-64h] BYREF
    float vup[3]; // [esp+A4h] [ebp-58h] BYREF
    int i; // [esp+B0h] [ebp-4Ch]

    if (*dir == 0.0 && dir[1] == 0.0 && dir[2] == 0.0)
        MyAssertHandler(".\\universal\\com_math.cpp", 862, 0, "%s", "dir[0] || dir[1] || dir[2]");
    
    vf[0] = *dir;
    vf[1] = dir[1];
    vf[2] = dir[2];
    
    PerpendicularVector(dir, vr);
    Vec3Cross(vr, vf, vup);
    
    m[0][0] = vr[0];
    m[1][0] = vr[1];
    m[2][0] = vr[2];
    m[0][1] = vup[0];
    m[1][1] = vup[1];
    m[2][1] = vup[2];
    m[0][2] = vf[0];
    m[1][2] = vf[1];
    m[2][2] = vf[2];

    memcpy(im, m, sizeof(im));
    im[0][1] = vr[1];
    im[0][2] = vr[2];
    im[1][0] = vup[0];
    im[1][2] = vup[2];
    im[2][0] = vf[0];
    im[2][1] = vf[1];

    mat3x3 zrot{};

    zrot[2][2] = 1.0;
    zrot[1][1] = 1.0;
    zrot[0][0] = 1.0;
    rad = degrees * 0.01745329238474369;
    iassert(!isnan(rad));

    zrot[0][0] = cos(rad);
    zrot[0][1] = sin(rad);

    iassert(!isnan(zrot[0][1]));
    iassert(!isnan(zrot[0][0]));

    zrot[1][0] = -zrot[0][1];
    zrot[1][1] = zrot[0][0];
    
    MatrixMultiply(m, zrot, tmpmat);
    MatrixMultiply(tmpmat, im, rot);

    for (i = 0; i < 3; ++i)
        dst[i] = rot[i][0] * *point + rot[i][1] * point[1] + rot[i][2] * point[2];
}

void __cdecl Vec3Basis_RightHanded(const float *forward, float *left, float *up)
{
    PerpendicularVector(forward, up);
    Vec3Cross(up, forward, left);
}

void __cdecl Vec3AddScalar(const float* a, float s, float* sum)
{
    *sum = *a + s;
    sum[1] = a[1] + s;
    sum[2] = a[2] + s;
}

void __cdecl Vec3Sub(const float *a, const float *b, float *diff)
{
    diff[0] = a[0] - b[0];
    diff[1] = a[1] - b[1];
    diff[2] = a[2] - b[2];
}

float __cdecl vectoyaw(const float *vec)
{
    float v2; // [esp+0h] [ebp-14h]
    float v3; // [esp+4h] [ebp-10h]
    float yawa; // [esp+10h] [ebp-4h]

    if (vec[1] == 0.0 && *vec == 0.0)
    {
        return 0.0;
    }
    else
    {
        v3 = atan2(vec[1], *vec);
        yawa = v3 * 180.0 / 3.141592741012573;
        if (yawa < 0.0)
            v2 = 360.0;
        else
            v2 = 0.0;
        return (yawa + v2);
    }
}

float __cdecl vectosignedyaw(const float *vec)
{
    float v2; // [esp+0h] [ebp-10h]
    float yaw; // [esp+Ch] [ebp-4h]

    if (vec[1] == 0.0 && *vec == 0.0)
    {
        return 0.0;
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

float __cdecl vectopitch(const float *vec)
{
    float v2; // [esp+0h] [ebp-20h]
    float v3; // [esp+4h] [ebp-1Ch]
    float v4; // [esp+8h] [ebp-18h]
    float v6; // [esp+14h] [ebp-Ch]
    float pitcha; // [esp+1Ch] [ebp-4h]

    if (vec[1] == 0.0 && *vec == 0.0)
    {
        if (-vec[2] < 0.0)
            return 270.0;
        else
            return 90.0;
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
        return (pitcha + v2);
    }
}

float __cdecl vectosignedpitch(const float *vec)
{
    float v2; // [esp+0h] [ebp-1Ch]
    float v3; // [esp+4h] [ebp-18h]
    float v5; // [esp+10h] [ebp-Ch]

    if (vec[1] == 0.0 && *vec == 0.0)
    {
        if (-vec[2] < 0.0)
            return -90.0;
        else
            return 90.0;
    }
    else
    {
        v5 = vec[1] * vec[1] + *vec * *vec;
        v3 = sqrt(v5);
        v2 = atan2(vec[2], v3);
        return (v2 * -180.0 / 3.141592741012573);
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

void __cdecl PerpendicularVector(const float* src, float* dst)
{
    const char* v2; // eax
    float scale; // [esp+18h] [ebp-34h]
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
    pos = srcSq[0] > srcSq[1];
    if (srcSq[pos] > srcSq[2])
        pos = 2;
    d = -src[pos];
    Vec3Scale(src, d, dst);
    dst[pos] = dst[pos] + 1.0;
    Vec3Normalize(dst);
}

float __cdecl PointToBoxDistSq(const float *pt, const float *mins, const float *maxs)
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
    EPSILON = 0.000099999997f;
    v8 = dir1dir2 * dir1LenSq;
    v7 = I_fabs(v8);
    v6 = 0.000099999997 * v7;
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

mat3x3 identityMatrix33 = {
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1}
};

mat4x4 identityMatrix44 = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}
};

void __cdecl OrthographicMatrix(mat4x4 &mtx, float width, float height, float depth)
{
    if (!mtx)
        MyAssertHandler(".\\universal\\com_math.cpp", 2281, 0, "%s", "mtx");
    if (width == 0.0)
        MyAssertHandler(".\\universal\\com_math.cpp", 2282, 0, "%s", "width != 0");
    if (height == 0.0)
        MyAssertHandler(".\\universal\\com_math.cpp", 2283, 0, "%s", "height != 0");
    if (depth == 0.0)
        MyAssertHandler(".\\universal\\com_math.cpp", 2284, 0, "%s", "depth != 0");
    memset(&mtx, 0, sizeof(mat4x4));
    (mtx)[0][0] = 2.0 / width;
    (mtx)[1][1] = 2.0 / height;
    (mtx)[2][2] = 0.5 / depth;
    (mtx)[3][2] = 0.5;
    (mtx)[3][3] = 1.0;
}

void __cdecl MatrixIdentity33(mat3x3& out)
{
    iassert(out);

    // out = identityMatrix33;
    memcpy(&out, &identityMatrix33, sizeof(out));
}

void __cdecl MatrixIdentity44(mat4x4& out)
{
    iassert(out);
    memcpy(&out, &identityMatrix44, sizeof(out));
}

void __cdecl MatrixSet44(mat4x4 &out, const vec3r origin, const mat3x3 &axis, float scale)
{
    (out)[0][0] = (axis)[0][0] * scale;
    (out)[0][1] = (axis)[0][1] * scale;
    (out)[0][2] = (axis)[0][2] * scale;
    (out)[0][3] = 0.0;

    (out)[1][0] = (axis)[1][0] * scale;
    (out)[1][1] = (axis)[1][1] * scale;
    (out)[1][2] = (axis)[1][2] * scale;
    (out)[1][3] = 0.0;

    (out)[2][0] = (axis)[2][0] * scale;
    (out)[2][1] = (axis)[2][1] * scale;
    (out)[2][2] = (axis)[2][2] * scale;
    out[2][3] = 0.0;

    out[3][0] = origin[0];
    out[3][1] = origin[1];
    out[3][2] = origin[2];
    out[3][3] = 1.0;
}

void __cdecl MatrixMultiply(const mat3x3 &in1, const mat3x3 &in2, mat3x3 &out)
{
    iassert(&in1 != &out);
    iassert(&in2 != &out);

    out[0][0] = (in1)[0][0] * (in2)[0][0]
        + (in1)[0][1] * (in2)[1][0]
        + (in1)[0][2] * (in2)[2][0];
    (out)[0][1] = (in1)[0][0] * (in2)[0][1]
        + (in1)[0][1] * (in2)[1][1]
        + (in1)[0][2] * (in2)[2][1];
    (out)[0][2] = (in1)[0][0] * (in2)[0][2]
        + (in1)[0][1] * (in2)[1][2]
        + (in1)[0][2] * (in2)[2][2];
    (out)[1][0] = (in1)[1][0] * (in2)[0][0]
        + (in1)[1][1] * (in2)[1][0]
        + (in1)[1][2] * (in2)[2][0];
    (out)[1][1] = (in1)[1][0] * (in2)[0][1]
        + (in1)[1][1] * (in2)[1][1]
        + (in1)[1][2] * (in2)[2][1];
    (out)[1][2] = (in1)[1][0] * (in2)[0][2]
        + (in1)[1][1] * (in2)[1][2]
        + (in1)[1][2] * (in2)[2][2];
    (out)[2][0] = (in1)[2][0] * (in2)[0][0]
        + (in1)[2][1] * (in2)[1][0]
        + (in1)[2][2] * (in2)[2][0];
    (out)[2][1] = (in1)[2][0] * (in2)[0][1]
        + (in1)[2][1] * (in2)[1][1]
        + (in1)[2][2] * (in2)[2][1];
    (out)[2][2] = (in1)[2][0] * (in2)[0][2]
        + (in1)[2][1] * (in2)[1][2]
        + (in1)[2][2] * (in2)[2][2];
}

// NOTE: this is not literally a 4x3 matrix multiplication since that does not work
void __cdecl MatrixMultiply43(const mat4x3 &in1, const mat4x3 &in2, mat4x3 &out)
{
    iassert(&in1 != &out);
    iassert(&in2 != &out);

    (out)[0][0] = (in1)[0][0] * (in2)[0][0]
        + (float)(in1)[0][1] * (float)(in2)[1][0]
        + (float)(in1)[0][2] * (float)(in2)[2][0];
    (out)[1][0] = (float)(in1)[1][0] * (in2)[0][0]
        + (float)(in1)[1][1] * (float)(in2)[1][0]
        + (float)(in1)[1][2] * (float)(in2)[2][0];
    (out)[2][0] = (float)(in1)[2][0] * (in2)[0][0]
        + (float)(in1)[2][1] * (float)(in2)[1][0]
        + (float)(in1)[2][2] * (float)(in2)[2][0];
    (out)[0][1] = (in1)[0][0] * (float)(in2)[0][1]
        + (float)(in1)[0][1] * (float)(in2)[1][1]
        + (float)(in1)[0][2] * (float)(in2)[2][1];
    (out)[1][1] = (float)(in1)[1][0] * (float)(in2)[0][1]
        + (float)(in1)[1][1] * (float)(in2)[1][1]
        + (float)(in1)[1][2] * (float)(in2)[2][1];
    (out)[2][1] = (float)(in1)[2][0] * (float)(in2)[0][1]
        + (float)(in1)[2][1] * (float)(in2)[1][1]
        + (float)(in1)[2][2] * (float)(in2)[2][1];
    (out)[0][2] = (in1)[0][0] * (float)(in2)[0][2]
        + (float)(in1)[0][1] * (float)(in2)[1][2]
        + (float)(in1)[0][2] * (float)(in2)[2][2];
    (out)[1][2] = (float)(in1)[1][0] * (float)(in2)[0][2]
        + (float)(in1)[1][1] * (float)(in2)[1][2]
        + (float)(in1)[1][2] * (float)(in2)[2][2];
    (out)[2][2] = (float)(in1)[2][0] * (float)(in2)[0][2]
        + (float)(in1)[2][1] * (float)(in2)[1][2]
        + (float)(in1)[2][2] * (float)(in2)[2][2];
    (out)[3][0] = (float)(in1)[3][0] * (in2)[0][0]
        + (float)(in1)[3][1] * (float)(in2)[1][0]
        + (float)(in1)[3][2] * (float)(in2)[2][0]
        + (float)(in2)[3][0];
    (out)[3][1] = (float)(in1)[3][0] * (float)(in2)[0][1]
        + (float)(in1)[3][1] * (float)(in2)[1][1]
        + (float)(in1)[3][2] * (float)(in2)[2][1]
        + (float)(in2)[3][1];
    (out)[3][2] = (float)(in1)[3][0] * (float)(in2)[0][2]
        + (float)(in1)[3][1] * (float)(in2)[1][2]
        + (float)(in1)[3][2] * (float)(in2)[2][2]
        + (float)(in2)[3][2];
}

void __cdecl MatrixMultiply44(const mat4x4 &in1, const mat4x4 &in2, mat4x4 &out)
{
    iassert(&in1 != &out);
    iassert(&in2 != &out);

    (out)[0][0] = (in1)[0][0] * (in2)[0][0]
        + (in1)[0][1] * (in2)[1][0]
        + (in1)[0][2] * (in2)[2][0]
        + (in1)[0][3] * (in2)[3][0];
    (out)[0][1] = (in1)[0][0] * (in2)[0][1]
        + (in1)[0][1] * (in2)[1][1]
        + (in1)[0][2] * (in2)[2][1]
        + (in1)[0][3] * (in2)[3][1];
    (out)[0][2] = (in1)[0][0] * (in2)[0][2]
        + (in1)[0][1] * (in2)[1][2]
        + (in1)[0][2] * (in2)[2][2]
        + (in1)[0][3] * (in2)[3][2];
    (out)[0][3] = (in1)[0][0] * (in2)[0][3]
        + (in1)[0][1] * (in2)[1][3]
        + (in1)[0][2] * (in2)[2][3]
        + (in1)[0][3] * (in2)[3][3];
    (out)[1][0] = (in1)[1][0] * (in2)[0][0]
        + (in1)[1][1] * (in2)[1][0]
        + (in1)[1][2] * (in2)[2][0]
        + (in1)[1][3] * (in2)[3][0];
    (out)[1][1] = (in1)[1][0] * (in2)[0][1]
        + (in1)[1][1] * (in2)[1][1]
        + (in1)[1][2] * (in2)[2][1]
        + (in1)[1][3] * (in2)[3][1];
    (out)[1][2] = (in1)[1][0] * (in2)[0][2]
        + (in1)[1][1] * (in2)[1][2]
        + (in1)[1][2] * (in2)[2][2]
        + (in1)[1][3] * (in2)[3][2];
    (out)[1][3] = (in1)[1][0] * (in2)[0][3]
        + (in1)[1][1] * (in2)[1][3]
        + (in1)[1][2] * (in2)[2][3]
        + (in1)[1][3] * (in2)[3][3];
    (out)[2][0] = (in1)[2][0] * (in2)[0][0]
        + (in1)[2][1] * (in2)[1][0]
        + (in1)[2][2] * (in2)[2][0]
        + (in1)[2][3] * (in2)[3][0];
    (out)[2][1] = (in1)[2][0] * (in2)[0][1]
        + (in1)[2][1] * (in2)[1][1]
        + (in1)[2][2] * (in2)[2][1]
        + (in1)[2][3] * (in2)[3][1];
    (out)[2][2] = (in1)[2][0] * (in2)[0][2]
        + (in1)[2][1] * (in2)[1][2]
        + (in1)[2][2] * (in2)[2][2]
        + (in1)[2][3] * (in2)[3][2];
    (out)[2][3] = (in1)[2][0] * (in2)[0][3]
        + (in1)[2][1] * (in2)[1][3]
        + (in1)[2][2] * (in2)[2][3]
        + (in1)[2][3] * (in2)[3][3];
    (out)[3][0] = (in1)[3][0] * (in2)[0][0]
        + (in1)[3][1] * (in2)[1][0]
        + (in1)[3][2] * (in2)[2][0]
        + (in1)[3][3] * (in2)[3][0];
    (out)[3][1] = (in1)[3][0] * (in2)[0][1]
        + (in1)[3][1] * (in2)[1][1]
        + (in1)[3][2] * (in2)[2][1]
        + (in1)[3][3] * (in2)[3][1];
    (out)[3][2] = (in1)[3][0] * (in2)[0][2]
        + (in1)[3][1] * (in2)[1][2]
        + (in1)[3][2] * (in2)[2][2]
        + (in1)[3][3] * (in2)[3][2];
    (out)[3][3] = (in1)[3][0] * (in2)[0][3]
        + (in1)[3][1] * (in2)[1][3]
        + (in1)[3][2] * (in2)[2][3]
        + (in1)[3][3] * (in2)[3][3];
}

void __cdecl MatrixTranspose(const mat3x3& in, mat3x3& out)
{
    iassert(in != out);

    (out)[0][0] = (in)[0][0];
    (out)[0][1] = (in)[1][0];
    (out)[0][2] = (in)[2][0];
    (out)[1][0] = (in)[0][1];
    (out)[1][1] = (in)[1][1];
    (out)[1][2] = (in)[2][1];
    (out)[2][0] = (in)[0][2];
    (out)[2][1] = (in)[1][2];
    (out)[2][2] = (in)[2][2];
}

void __cdecl MatrixTranspose44(const mat4x4 &in, mat4x4 &out)
{
    iassert(in != out);

    (out)[0][0] = (in)[0][0];
    (out)[0][1] = (in)[1][0];
    (out)[0][2] = (in)[2][0];
    (out)[0][3] = (in)[3][0];
    (out)[1][0] = (in)[0][1];
    (out)[1][1] = (in)[1][1];
    (out)[1][2] = (in)[2][1];
    (out)[1][3] = (in)[3][1];
    (out)[2][0] = (in)[0][2];
    (out)[2][1] = (in)[1][2];
    (out)[2][2] = (in)[2][2];
    (out)[2][3] = (in)[3][2];
    (out)[3][0] = (in)[0][3];
    (out)[3][1] = (in)[1][3];
    (out)[3][2] = (in)[2][3];
    (out)[3][3] = (in)[3][3];
}

void __cdecl MatrixTransformVector(const vec3r in1, const mat3x3& in2, vec3r out)
{
    iassert(in1 != out);

    (out)[0] = (in1)[0] * (in2)[0][0] + (in1)[1] * (in2)[1][0] + (in1)[2] * (in2)[2][0];
    (out)[1] = (in1)[0] * (in2)[0][1] + (in1)[1] * (in2)[1][1] + (in1)[2] * (in2)[2][1];
    (out)[2] = (in1)[0] * (in2)[0][2] + (in1)[1] * (in2)[1][2] + (in1)[2] * (in2)[2][2];
}

void __cdecl MatrixInverseOrthogonal43(const mat4x3& in, mat4x3& out)
{
    vec3 negated; // [esp+0h] [ebp-Ch] BYREF

    MatrixTranspose((const mat3x3 &)in, (mat3x3 &)out);
    Vec3Sub(vec3_origin, (in)[3], negated);
    MatrixTransformVector(negated, (const mat3x3 &) out, out[3]);
}

void __cdecl MatrixInverse44(const mat4x4 &mat, mat4x4& dst)
{
    float src[16]; // [esp+0h] [ebp-78h]
    float tmp[12]; // [esp+44h] [ebp-34h]

    float det; // [esp+40h] [ebp-38h]
    int i; // [esp+74h] [ebp-4h]

    iassert(mat != dst);

    for (i = 0; i < 4; ++i)
    {
        src[i] = (mat)[i][0];
        src[i + 4] = (mat)[i][1];
        src[i + 8] = (mat)[i][2];
        src[i + 12] = (mat)[i][3];
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
    (dst)[0][0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
    (dst)[0][0] = (dst)[0][0] - (tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7]);
    (dst)[0][1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
    (dst)[0][1] = (dst)[0][1] - (tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7]);
    (dst)[0][2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
    (dst)[0][2] = (dst)[0][2] - (tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7]);
    (dst)[0][3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
    (dst)[0][3] = (dst)[0][3] - (tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6]);
    (dst)[1][0] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
    (dst)[1][0] = (dst)[1][0] - (tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3]);
    (dst)[1][1] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
    (dst)[1][1] = (dst)[1][1] - (tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3]);
    (dst)[1][2] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
    (dst)[1][2] = (dst)[1][2] - (tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3]);
    (dst)[1][3] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
    (dst)[1][3] = (dst)[1][3] - (tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2]);
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
    (dst)[2][0] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
    (dst)[2][0] = (dst)[2][0] - (tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15]);
    (dst)[2][1] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
    (dst)[2][1] = (dst)[2][1] - (tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15]);
    (dst)[2][2] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
    (dst)[2][2] = (dst)[2][2] - (tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15]);
    (dst)[2][3] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
    (dst)[2][3] = (dst)[2][3] - (tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14]);
    (dst)[3][0] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
    (dst)[3][0] = (dst)[3][0] - (tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10]);
    (dst)[3][1] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
    (dst)[3][1] = (dst)[3][1] - (tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8]);
    (dst)[3][2] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
    (dst)[3][2] = (dst)[3][2] - (tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9]);
    (dst)[3][3] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
    (dst)[3][3] = (dst)[3][3] - (tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8]);
    det = src[0] * (dst)[0][0] + src[1] * (dst)[0][1] + src[2] * (dst)[0][2] + src[3] * (dst)[0][3];
    
    iassert(det != 0.0);

    det = 1.0 / det;
    for (i = 0; i < 16; ++i)
        (dst)[0][i] = (dst)[0][i] * det;
}

void __cdecl MatrixTransformVector44(const vec4r vec, const mat4x4 &mat, vec4r out)
{
    iassert(vec != out);
    out[0] = vec[0] * (mat)[0][0] + vec[1] * (mat)[1][0] + vec[2] * (mat)[2][0] + vec[3] * (mat)[3][0];
    out[1] = vec[0] * (mat)[0][1] + vec[1] * (mat)[1][1] + vec[2] * (mat)[2][1] + vec[3] * (mat)[3][1];
    out[2] = vec[0] * (mat)[0][2] + vec[1] * (mat)[1][2] + vec[2] * (mat)[2][2] + vec[3] * (mat)[3][2];
    out[3] = vec[0] * (mat)[0][3] + vec[1] * (mat)[1][3] + vec[2] * (mat)[2][3] + vec[3] * (mat)[3][3];
}

void __cdecl InvMatrixTransformVectorQuatTrans(const float *in, const DObjAnimMat *mat, float *out)
{
    float temp[3]; // [esp+4Ch] [ebp-30h]
    float axis[3][3]; // [esp+58h] [ebp-24h] BYREF

    temp[0] = in[0] - mat->trans[0];
    temp[1] = in[1] - mat->trans[1];
    temp[2] = in[2] - mat->trans[2];
    ConvertQuatToMat(mat, axis);
    out[0] = (float)((float)(temp[0] * axis[0][0]) + (float)(temp[1] * axis[0][1])) + (float)(temp[2] * axis[0][2]);
    out[1] = (float)((float)(temp[0] * axis[1][0]) + (float)(temp[1] * axis[1][1])) + (float)(temp[2] * axis[1][2]);
    out[2] = (float)((float)(temp[0] * axis[2][0]) + (float)(temp[1] * axis[2][1])) + (float)(temp[2] * axis[2][2]);
}

void __cdecl MatrixTransposeTransformVector(const vec3r in1, const mat3x3& in2, vec3r out)
{
    iassert(in1 != out);

    out[0] = in1[0] * in2[0][0] + in1[1] * in2[0][1] + in1[2] * in2[0][2];
    out[1] = in1[0] * in2[1][0] + in1[1] * in2[1][1] + in1[2] * in2[1][2];
    out[2] = in1[0] * in2[2][0] + in1[1] * in2[2][1] + in1[2] * in2[2][2];
}

void __cdecl MatrixTransformVector43(const vec3r in1, const mat4x3 &in2, vec3r out)
{
    iassert(in1 != out);

    out[0] = in1[0] * (in2)[0][0] + in1[1] * (in2)[1][0] + in1[2] * (in2)[2][0] + (in2)[3][0];
    out[1] = in1[0] * (in2)[0][1] + in1[1] * (in2)[1][1] + in1[2] * (in2)[2][1] + (in2)[3][1];
    out[2] = in1[0] * (in2)[0][2] + in1[1] * (in2)[1][2] + in1[2] * (in2)[2][2] + (in2)[3][2];
}

void __cdecl MatrixTransposeTransformVector43(const vec3r in1, const mat4x3 &in2, vec3r out)
{
    iassert(in1 != out);
    float temp[3]; // [esp+0h] [ebp-Ch] BYREF

    Vec3Sub(in1, (in2)[3], temp);

    out[0] = (in2)[0][0] * temp[0] + (in2)[0][1] * temp[1] + (in2)[0][2] * temp[2];
    out[1] = (in2)[1][0] * temp[0] + (in2)[1][1] * temp[1] + (in2)[1][2] * temp[2];
    out[2] = (in2)[2][0] * temp[0] + (in2)[2][1] * temp[1] + (in2)[2][2] * temp[2];
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

void __cdecl QuatToAxis(const float *quat, mat3x3 &axis)
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

    (axis)[0][0] = 1.0 - (yya + zza);
    (axis)[0][1] = xy + zw;
    (axis)[0][2] = xz - yw;
    (axis)[1][0] = xy - zw;
    (axis)[1][1] = 1.0 - (xxa + zza);
    (axis)[1][2] = yz + xw;
    (axis)[2][0] = xz + yw;
    (axis)[2][1] = yz - xw;
    (axis)[2][2] = 1.0 - (xxa + yya);
}

void __cdecl UnitQuatToAxis(const float *quat, mat3x3 &axis)
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

    (axis)[0][0] = 1.0 - (yy + zz);
    (axis)[0][1] = xy + zw;
    (axis)[0][2] = xz - yw;
    (axis)[1][0] = xy - zw;
    (axis)[1][1] = 1.0 - (xx + zz);
    (axis)[1][2] = yz + xw;
    (axis)[2][0] = xz + yw;
    (axis)[2][1] = yz - xw;
    (axis)[2][2] = 1.0 - (xx + yy);
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
    float v4; // st6
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

void __cdecl QuatMultiply(const float* in1, const float* in2, float* out)
{
    iassert(in1 != out);
    iassert(in2 != out);

    out[0] = in1[0] * in2[3] + in1[3] * *in2 + in1[2] * in2[1] - in1[1] * in2[2];
    out[1] = in1[1] * in2[3] - in1[2] * *in2 + in1[3] * in2[1] + *in1 * in2[2];
    out[2] = in1[2] * in2[3] + in1[1] * *in2 - *in1 * in2[1] + in1[3] * in2[2];
    out[3] = in1[3] * in2[3] - *in1 * *in2 - in1[1] * in2[1] - in1[2] * in2[2];
}

void QuatInverse(const float *in, float *out)
{
    out[0] = -in[0];
    out[1] = -in[1];
    out[2] = -in[2];
    out[3] = in[3];
}

float __cdecl RotationToYaw(const float *rot)
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
    return (v2 * 57.2957763671875);
}

void __cdecl AnglesSubtract(float *v1, float *v2, float *v3)
{
    *v3 = AngleDelta(*v1, *v2);
    v3[1] = AngleDelta(v1[1], v2[1]);
    v3[2] = AngleDelta(v1[2], v2[2]);
}

float __cdecl AngleNormalize360(float angle)
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
        return ((scaledAngle - v3) * 360.0);
    else
        return (result - 360.0);
}

float AngleNormalize180(float angle)
{
    angle = fmodf(angle + 180.0f, 360.0f);
    if (angle < 0.0f)
        angle += 360.0f;
    return angle - 180.0f;
}

float AngleSubtract(float a1, float a2)
{
    float delta = fmodf(a1 - a2, 360.0f);
    if (delta < 0.0f)
        delta += 360.0f;
    return delta;
}

float __cdecl RadiusFromBounds(const float *mins, const float *maxs)
{
    return sqrt(RadiusFromBoundsSq(mins, maxs));
}

float __cdecl RadiusFromBounds2D(const float *mins, const float *maxs)
{
    float v4; // [esp+4h] [ebp-4h]

    v4 = RadiusFromBounds2DSq(mins, maxs);
    return sqrt(v4);
}

float __cdecl RadiusFromBoundsSq(const float *mins, const float *maxs)
{
    float v3; // [esp+0h] [ebp-2Ch]
    float v4; // [esp+4h] [ebp-28h]
    float v5; // [esp+8h] [ebp-24h]
    int i; // [esp+18h] [ebp-14h]
    float corner[3]; // [esp+1Ch] [ebp-10h] BYREF
    float a; // [esp+28h] [ebp-4h]

    for (i = 0; i < 3; ++i)
    {
        v5 = I_fabs(mins[i]);
        a = v5;
        v4 = I_fabs(maxs[i]);
        if (v4 >= v5)
            v3 = v4;
        else
            v3 = a;
        corner[i] = v3;
    }
    return Vec3LengthSq(corner);
}

float __cdecl RadiusFromBounds2DSq(const float *mins, const float *maxs)
{
    float v4; // [esp+4h] [ebp-28h]
    float v5; // [esp+8h] [ebp-24h]
    float v6; // [esp+Ch] [ebp-20h]
    int i; // [esp+1Ch] [ebp-10h]
    float corner[2]; // [esp+20h] [ebp-Ch]
    float a; // [esp+28h] [ebp-4h]

    for (i = 0; i < 2; ++i)
    {
        v6 = I_fabs(mins[i]);
        a = v6;
        v5 = I_fabs(maxs[i]);
        if (v5 >= v6)
            v4 = v5;
        else
            v4 = a;
        corner[i] = v4;
    }
    return (corner[1] * corner[1] + corner[0] * corner[0]);
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

    if (*mins > *maxs)
        MyAssertHandler(".\\universal\\com_math.cpp", 2455, 0, "%s", "maxs[0] >= mins[0]");
    if (mins[1] > maxs[1])
        MyAssertHandler(".\\universal\\com_math.cpp", 2456, 0, "%s", "maxs[1] >= mins[1]");
    if (mins[2] > maxs[2])
        MyAssertHandler(".\\universal\\com_math.cpp", 2457, 0, "%s", "maxs[2] >= mins[2]");
    Vec3Sub(maxs, mins, size);
    v3 = size[0] - size[1];
    if (v3 < 0.0)
        v2 = size[1];
    else
        v2 = size[0];
    if (size[2] < v2)
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

    if (*mins > *maxs)
        MyAssertHandler(".\\universal\\com_math.cpp", 2477, 0, "maxs[0] >= mins[0]\n\t%g, %g", *maxs, *mins);
    if (mins[1] > maxs[1])
        MyAssertHandler(".\\universal\\com_math.cpp", 2478, 0, "maxs[1] >= mins[1]\n\t%g, %g", maxs[1], mins[1]);
    if (mins[2] > maxs[2])
        MyAssertHandler(".\\universal\\com_math.cpp", 2479, 0, "maxs[2] >= mins[2]\n\t%g, %g", maxs[2], mins[2]);
    Vec3Sub(maxs, mins, size);
    if (size[2] < size[0])
    {
        diff = (size[0] - size[2]) * 0.5;
        *mins = *mins + diff;
        *maxs = *maxs - diff;
    }
    if (size[2] < size[1])
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
    if (*mins > *v)
        *mins = *v;
    if (*maxs < *v)
        *maxs = *v;
    if (mins[1] > v[1])
        mins[1] = v[1];
    if (maxs[1] < v[1])
        maxs[1] = v[1];
    if (mins[2] > v[2])
        mins[2] = v[2];
    if (maxs[2] < v[2])
        maxs[2] = v[2];
}

void __cdecl AddPointToBounds2D(const float *v, float *mins, float *maxs)
{
    if (mins[0] > v[0])
        mins[0] = v[0];
    if (maxs[0] < v[0])
        maxs[0] = v[0];
    if (mins[1] > v[1])
        mins[1] = v[1];
    if (maxs[1] < v[1])
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
    if (*mins > *v || *maxs < *v)
        return 0;
    if (mins[1] > v[1] || maxs[1] < v[1])
        return 0;
    return mins[2] <= v[2] && maxs[2] >= v[2];
}

bool __cdecl BoundsOverlap(const float *mins0, const float *maxs0, const float *mins1, const float *maxs1)
{
    if (*maxs1 < *mins0 || *maxs0 < *mins1)
        return 0;
    if (maxs1[1] < mins0[1] || maxs0[1] < mins1[1])
        return 0;
    return maxs1[2] >= mins0[2] && maxs0[2] >= mins1[2];
}

void __cdecl ExpandBounds(const float *addedmins, const float *addedmaxs, float *mins, float *maxs)
{
    if (*addedmins < *mins)
        *mins = *addedmins;
    if (*addedmaxs > *maxs)
        *maxs = *addedmaxs;
    if (addedmins[1] < mins[1])
        mins[1] = addedmins[1];
    if (addedmaxs[1] > maxs[1])
        maxs[1] = addedmaxs[1];
    if (addedmins[2] < mins[2])
        mins[2] = addedmins[2];
    if (addedmaxs[2] > maxs[2])
        maxs[2] = addedmaxs[2];
}

void __cdecl AxisClear(mat3x3 &axis)
{
    (axis)[0][0] = 1.0;
    (axis)[0][1] = 0.0;
    (axis)[0][2] = 0.0;
    
    (axis)[1][0] = 0.0;
    (axis)[1][1] = 1.0;
    (axis)[1][2] = 0.0;

    (axis)[2][0] = 0.0;
    (axis)[2][1] = 0.0;
    (axis)[2][2] = 1.0;
}

void __cdecl AxisTranspose(const mat3x3& in, mat3x3& out)
{
    iassert(&in != &out);

    (out)[0][0] = (in)[0][0];
    (out)[0][1] = (in)[1][0];
    (out)[0][2] = (in)[2][0];
    
    (out)[1][0] = (in)[0][1];
    (out)[1][1] = (in)[1][1];
    (out)[1][2] = (in)[2][1];

    (out)[2][0] = (in)[0][2];
    (out)[2][1] = (in)[1][2];
    (out)[2][2] = (in)[2][2];
}

void __cdecl AxisTransformVec3(const mat3x3 &axes, const float *vec, float *out)
{
    out[0] = *vec * (axes)[0][0] + vec[1] * (axes)[1][0] + vec[2] * (axes)[2][0];
    out[1] = *vec * (axes)[0][1] + vec[1] * (axes)[1][1] + vec[2] * (axes)[2][1];
    out[2] = *vec * (axes)[0][2] + vec[1] * (axes)[1][2] + vec[2] * (axes)[2][2];
}

void __cdecl YawToAxis(float yaw, mat3x3 &axis)
{
    float right[3]; // [esp+Ch] [ebp-Ch] BYREF

    YawVectors(yaw, axis[0], right);
    (axis)[2][0] = 0.0;
    (axis)[2][1] = 0.0;
    (axis)[2][2] = 1.0;
    Vec3Sub(vec3_origin, right, axis[1]);
}

void __cdecl AxisToAngles(const mat3x3 &axis, vec3r angles)
{
    float v2; // [esp+0h] [ebp-38h]
    float rad; // [esp+18h] [ebp-20h]
    float rada; // [esp+18h] [ebp-20h]
    float right[3]; // [esp+1Ch] [ebp-1Ch] BYREF
    float temp; // [esp+28h] [ebp-10h]
    float pitch; // [esp+2Ch] [ebp-Ch]
    float fCos; // [esp+30h] [ebp-8h]
    float fSin; // [esp+34h] [ebp-4h]

    vectoangles(axis[0], angles);
    right[0] = axis[1][0];
    right[1] = axis[1][1];
    right[2] = axis[1][2];
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
    float invDeterminant; // [esp+0h] [ebp-28h]
    float determinant; // [esp+8h] [ebp-20h]

    determinant = (plane[1][1] * plane[2][2] - plane[2][1] * plane[1][2]) * **plane
        + (plane[2][1] * (*plane)[2] - (*plane)[1] * plane[2][2]) * *plane[1]
        + ((*plane)[1] * plane[1][2] - plane[1][1] * (*plane)[2]) * *plane[2];
    if (I_fabs(determinant) < EQUAL_EPSILON)
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
    float v5; // [esp+4h] [ebp-64h]
    float v6; // [esp+Ch] [ebp-5Ch]
    float v7; // [esp+10h] [ebp-58h]
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
        rounded = (int)(v13) * snapGrid;
        v12 = rounded - xyz[axis];
        v9 = I_fabs(v12);
        if (snapEpsilon <= v9)
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
            v6 = I_fabs(v11);
            snapError = v6;
            if (v6 > maxSnapError)
                maxSnapError = snapError;
            v5 = planes[planeIndex][3];
            v10 = Vec3Dot(planes[planeIndex], xyz) - v5;
            v4 = I_fabs(v10);
            baseError = v4;
            if (v4 > maxBaseError)
                maxBaseError = baseError;
        }
        if (maxBaseError > maxSnapError)
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
        edgeNormal = (*verts)[3 * vertIndex + y] - (*verts)[3 * vertIndexPrev + y];
        edgeNormal_4 = (*verts)[3 * vertIndexPrev + x] - (*verts)[3 * vertIndex + x];
        pointDelta = point[x] - (*verts)[3 * vertIndexPrev + x];
        pointDelta_4 = point[y] - (*verts)[3 * vertIndexPrev + y];
        projectionDist = edgeNormal_4 * pointDelta_4 + edgeNormal * pointDelta;
        if (projectionDist < 0.0)
            return 0;
        vertIndexPrev = vertIndex;
    }
    return 1;
}

int __cdecl PlaneFromPoints(float *plane, const float *v0, const float *v1, const float *v2)
{
    float v5; // st7
    float v6; // st7
    float v7; // [esp+0h] [ebp-34h]
    float v8; // [esp+4h] [ebp-30h]
    float v9; // [esp+Ch] [ebp-28h]
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

void __cdecl ProjectPointOnPlane(const float *const f1, const float *const normal, float *const result)
{
    const char *v3; // eax
    double v4; // [esp+18h] [ebp-14h]
    float d; // [esp+28h] [ebp-4h]

    if (!Vec3IsNormalized(normal))
    {
        v4 = Vec3Length(normal);
        v3 = va("(%g %g %g) len %g", *normal, normal[1], normal[2], v4);
        MyAssertHandler(".\\universal\\com_math.cpp", 3139, 0, "%s\n\t%s", "Vec3IsNormalized( normal )", v3);
    }
    d = -Vec3Dot(normal, f1);
    Vec3Mad(f1, d, normal, result);
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
            if (minsSqrd <= maxsSqrd)
                v5 = minsSqrd;
            else
                v5 = maxsSqrd;
            total = total + v5;
        }
    }
    return fogOpaqueDistSqrd < total;
}

float __cdecl Q_rint(float in)
{
    float v3; // [esp+Ch] [ebp-4h]

    v3 = in + 0.5;
    return floor(v3);
}

float __cdecl ColorNormalize(const float* in, float *out)
{
    float max; // [esp+8h] [ebp-8h]
    float scale; // [esp+Ch] [ebp-4h]

    max = in[0];
    if (max < in[1])
        max = in[1];
    if (max < in[2])
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

float __cdecl PitchForYawOnNormal(float fYaw, const float *normal)
{
    float v4; // [esp+10h] [ebp-14h]
    float forward[3]; // [esp+18h] [ebp-Ch] BYREF

    iassert(normal[0] != 0.0 || normal[1] != 0.0 || normal[2] != 0.0);

    YawVectors(fYaw, forward, 0);
    if (normal[2] == 0.0)
        return 270.0;
    forward[2] = (*normal * forward[0] + normal[1] * forward[1]) / normal[2];
    v4 = atan(forward[2]);
    return (v4 * 180.0 / 3.141592741012573);
}

void __cdecl NearestPitchAndYawOnPlane(const float* angles, const float* normal, float* result)
{
    float projected[3]; // [esp+0h] [ebp-18h] BYREF
    float forward[3]; // [esp+Ch] [ebp-Ch] BYREF

    iassert(normal[0] != 0.0 || normal[1] != 0.0 || normal[2] != 0.0);

    AngleVectors(angles, forward, 0, 0);
    ProjectPointOnPlane(forward, normal, projected);
    vectoangles(projected, result);
}

void __cdecl Rand_Init(int seed)
{
    holdrand = seed;
}

float __cdecl flrand(float min, float max)
{
    float result; // [esp+8h] [ebp-4h]

    holdrand = 214013 * holdrand + 2531011;
    result = (holdrand >> 17);
    return ((max - min) * result / 32768.0 + min);
}

int __cdecl irand(int min, int max)
{
    holdrand = 214013 * holdrand + 2531011;
    return (((holdrand >> 17) * (__int64)(max - min)) >> 15) + min;
}

void __cdecl MatrixTransformVectorQuatTransEquals(const DObjAnimMat *in, float *inout)
{
    float temp; // [esp+48h] [ebp-2Ch]
    float temp_4; // [esp+4Ch] [ebp-28h]
    float axis[3][3]; // [esp+50h] [ebp-24h] BYREF

    ConvertQuatToMat(in, axis);
    temp =     (inout[0] * axis[0][0]) + (inout[1] * axis[1][0]) + (inout[2] * axis[2][0]) + in->trans[0];
    temp_4 =   (inout[0] * axis[0][1]) + (inout[1] * axis[1][1]) + (inout[2] * axis[2][1]) + in->trans[1];
    inout[2] = (inout[0] * axis[0][2]) + (inout[1] * axis[1][2]) + (inout[2] * axis[2][2]) + in->trans[2];
    inout[0] = temp;
    inout[1] = temp_4;
}

void __cdecl QuatMultiplyEquals(const float *in, float *inout)
{
    float temp[3];

    temp[0] = (float)((float)((float)(inout[1] * in[3]) - (float)(inout[2] * in[0])) + (float)(inout[3] * in[1])) + (float)(inout[0] * in[2]);
    temp[1] = (float)((float)((float)(inout[2] * in[3]) + (float)(inout[1] * in[0])) - (float)(inout[0] * in[1])) + (float)(inout[3] * in[2]);
    temp[2] = (float)((float)((float)(inout[3] * in[3]) - (float)(inout[0] * in[0])) - (float)(inout[1] * in[1])) - (float)(inout[2] * in[2]);
    inout[0] = (float)((float)((float)(inout[0] * in[3]) + (float)(inout[3] * in[0])) + (float)(inout[2] * in[1])) - (float)(inout[1] * in[2]);
    inout[1] = temp[0];
    inout[2] = temp[1];
    inout[3] = temp[2];
}

void __cdecl ConvertQuatToInverseMat(const DObjAnimMat *mat, float (*axis)[3])
{
    float scaledQuat[3]; // [esp+28h] [ebp-20h]

    iassert(!IS_NAN(mat->quat[0]) && !IS_NAN(mat->quat[1]) && !IS_NAN(mat->quat[2]) && !IS_NAN(mat->quat[3]));
    iassert(!IS_NAN(mat->transWeight));

    float transWeight = mat->transWeight;

    Vec3Scale(mat->quat, transWeight, scaledQuat);

    float xx = scaledQuat[0] * mat->quat[0];
    float xy = scaledQuat[0] * mat->quat[1];
    float xz = scaledQuat[0] * mat->quat[2];
    float xw = scaledQuat[0] * mat->quat[3];
    float yy = scaledQuat[1] * mat->quat[1];
    float yz = scaledQuat[1] * mat->quat[2];
    float yw = scaledQuat[1] * mat->quat[3];
    float zz = scaledQuat[2] * mat->quat[2];
    float zw = scaledQuat[2] * mat->quat[3];

    (*axis)[0] = 1.0 - (yy + zz);
    (*axis)[1] = xy - zw;
    (*axis)[2] = xz + yw;
    (*axis)[3] = xy + zw;
    (*axis)[4] = 1.0 - (xx + zz);
    (*axis)[5] = yz - xw;
    (*axis)[6] = xz - yw;
    (*axis)[7] = yz + xw;
    (*axis)[8] = 1.0 - (xx + yy);

    (*axis)[9] =  -(mat->trans[0] * (*axis)[0] + mat->trans[1] * (*axis)[3] + mat->trans[2] * (*axis)[6]);
    (*axis)[10] = -(mat->trans[0] * (*axis)[1] + mat->trans[1] * (*axis)[4] + mat->trans[2] * (*axis)[7]);
    (*axis)[11] = -(mat->trans[0] * (*axis)[2] + mat->trans[1] * (*axis)[5] + mat->trans[2] * (*axis)[8]);
}

void __cdecl ConvertQuatToMat(const DObjAnimMat *mat, float (*axis)[3])
{
    float scaledQuat[3]; // [esp+28h] [ebp-20h]

    float yy; // [esp+18h] [ebp-30h]
    float xy; // [esp+1Ch] [ebp-2Ch]
    float zz; // [esp+20h] [ebp-28h]
    float zw; // [esp+24h] [ebp-24h]
    float yw; // [esp+34h] [ebp-14h]
    float xz; // [esp+38h] [ebp-10h]
    float yz; // [esp+3Ch] [ebp-Ch]
    float xx; // [esp+40h] [ebp-8h]
    float xw; // [esp+44h] [ebp-4h]

    iassert(!IS_NAN((mat->quat[0])));
    iassert(!IS_NAN((mat->quat[1])));
    iassert(!IS_NAN((mat->quat[2])));
    iassert(!IS_NAN((mat->quat[3])));
    iassert(!IS_NAN((mat->transWeight)));

    Vec3Scale(mat->quat, mat->transWeight, scaledQuat);

    xx = scaledQuat[0] * mat->quat[0];
    xy = scaledQuat[0] * mat->quat[1];
    xz = scaledQuat[0] * mat->quat[2];
    xw = scaledQuat[0] * mat->quat[3];
    yy = scaledQuat[1] * mat->quat[1];
    yz = scaledQuat[1] * mat->quat[2];
    yw = scaledQuat[1] * mat->quat[3];
    zz = scaledQuat[2] * mat->quat[2];
    zw = scaledQuat[2] * mat->quat[3];
    (*axis)[0] = 1.0 - (float)(yy + zz);
    (*axis)[1] = xy + zw;
    (*axis)[2] = xz - yw;
    (*axis)[3] = xy - zw;
    (*axis)[4] = 1.0 - (float)(xx + zz);
    (*axis)[5] = yz + xw;
    (*axis)[6] = xz + yw;
    (*axis)[7] = yz - xw;
    (*axis)[8] = 1.0 - (float)(xx + yy);
}

void __cdecl MatrixTransformVectorQuatTrans(const vec3r in, const DObjAnimMat *mat, vec3r out)
{
    float axis[3][3];

    ConvertQuatToMat(mat, axis);

    out[0] = (in[0] * axis[0][0]) + (in[1] * axis[1][0]) + (in[2] * axis[2][0]) + mat->trans[0];
    out[1] = (in[0] * axis[0][1]) + (in[1] * axis[1][1]) + (in[2] * axis[2][1]) + mat->trans[1];
    out[2] = (in[0] * axis[0][2]) + (in[1] * axis[1][2]) + (in[2] * axis[2][2]) + mat->trans[2];
}

void __cdecl AxisToQuat(const float (*mat)[3], float *out)
{
    float v2; // [esp+8h] [ebp-50h]
    float invLength; // [esp+Ch] [ebp-4Ch]
    float test[4][4]; // [esp+10h] [ebp-48h] BYREF
    int best; // [esp+50h] [ebp-8h]
    float testSizeSq; // [esp+54h] [ebp-4h]

    test[0][0] = (mat)[1][2] - (mat)[2][1];
    test[0][1] = (mat)[2][0] - (mat)[0][2];
    test[0][2] = (mat)[0][1] - (mat)[1][0];
    test[0][3] = (mat)[0][0] + (mat)[1][1] + (mat)[2][2] + 1.0;
    testSizeSq = Vec4LengthSq(test[0]);
    if (testSizeSq < 1.0)
    {
        test[1][0] = (mat)[2][0] + (mat)[0][2];
        test[1][1] = (mat)[2][1] + (mat)[1][2];
        test[1][2] = (mat)[2][2] - (mat)[1][1] - (mat)[0][0] + 1.0;
        test[1][3] = test[0][2];
        testSizeSq = Vec4LengthSq(test[1]);
        if (testSizeSq < 1.0)
        {
            test[2][0] = (mat)[0][0] - (mat)[1][1] - (mat)[2][2] + 1.0;
            test[2][1] = (mat)[1][0] + (mat)[0][1];
            test[2][2] = test[1][0];
            test[2][3] = test[0][0];
            testSizeSq = Vec4LengthSq(test[2]);
            if (testSizeSq < 1.0)
            {
                test[3][0] = test[2][1];
                test[3][1] = (mat)[1][1] - (mat)[0][0] - (mat)[2][2] + 1.0;
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
    
    iassert(testSizeSq != 0.0);

    v2 = sqrt(testSizeSq);
    invLength = 1.0 / v2;
    Vec4Scale(test[best], invLength, out);
}

void __cdecl AxisToSignedAngles(const float (*axis)[3], float *angles)
{
    float v2; // [esp+0h] [ebp-38h]
    float rad; // [esp+18h] [ebp-20h]
    float rada; // [esp+18h] [ebp-20h]
    float right[3]; // [esp+1Ch] [ebp-1Ch] BYREF
    float temp; // [esp+28h] [ebp-10h]
    float pitch; // [esp+2Ch] [ebp-Ch]
    float fCos; // [esp+30h] [ebp-8h]
    float fSin; // [esp+34h] [ebp-4h]

    vectosignedangles((const float *)axis, angles);
    right[0] = (*axis)[3];
    right[1] = (*axis)[4];
    right[2] = (*axis)[5];
    //rad = COERCE_FLOAT(*((_DWORD *)angles + 1) ^ _mask__NegFloat_) * 0.017453292;
    rad = (float)((float)-angles[1] * (float)0.017453292);
    fCos = cos(rad);
    fSin = sin(rad);
    temp = (float)(fCos * right[0]) - (float)(fSin * right[1]);
    right[1] = (float)(fSin * right[0]) + (float)(fCos * right[1]);
    //rada = COERCE_FLOAT(*(_DWORD *)angles ^ _mask__NegFloat_) * 0.017453292;
    rada = (float)((float)-*angles * (float)0.017453292);
    fCos = cos(rada);
    fSin = sin(rada);
    right[0] = (float)(fSin * right[2]) + (float)(fCos * temp);
    right[2] = (float)(fCos * right[2]) - (float)(fSin * temp);
    pitch = vectosignedpitch(right);
    if (right[1] >= 0.0)
    {
        //*((_DWORD *)angles + 2) = LODWORD(pitch) ^ _mask__NegFloat_;
        angles[2] = -pitch;
    }
    else
    {
        if (pitch >= 0.0)
        {
            //v2 = FLOAT_N180_0;
            v2 = -180.0f;
        }
        else
        {
            //v2 = FLOAT_180_0;
            v2 = 180.0f;
        }
        angles[2] = pitch + v2;
    }
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
    v9 = I_fabs(v17);
    scaledSepDist = scaledSepDist - v9;
    v16 = scaledSepAxis[1] * boxHalfSize[1];
    v8 = I_fabs(v16);
    scaledSepDist = scaledSepDist - v8;
    v15 = scaledSepAxis[2] * boxHalfSize[2];
    v7 = I_fabs(v15);
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
    v13 = I_fabs(v19);
    v18 = v13 - *boxHalfSize;
    v12 = v18 - 0.0;
    if (v12 < 0.0)
        v11 = 0.0;
    else
        v11 = v13 - *boxHalfSize;
    distFromBoxToMid[0] = v11;
    v17 = sphereOrg[1] - boxCenter[1];
    v10 = I_fabs(v17);
    v16 = v10 - boxHalfSize[1];
    v9 = v16 - 0.0;
    if (v9 < 0.0)
        v8 = 0.0;
    else
        v8 = v10 - boxHalfSize[1];
    distFromBoxToMid[1] = v8;
    v15 = sphereOrg[2] - boxCenter[2];
    v7 = I_fabs(v15);
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
    v24 = I_fabs(deltaMid[0]);
    v30 = v24 - *boxHalfSize;
    v23 = v30 - 0.0;
    if (v23 < 0.0)
        v22 = 0.0;
    else
        v22 = v24 - *boxHalfSize;
    distFromBoxToMid[0] = v22;
    v21 = I_fabs(deltaMid[1]);
    v29 = v21 - boxHalfSize[1];
    v20 = v29 - 0.0;
    if (v20 < 0.0)
        v19 = 0.0;
    else
        v19 = v21 - boxHalfSize[1];
    distFromBoxToMid[1] = v19;
    v18 = I_fabs(deltaMid[2]);
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
    v10 = I_fabs(v27);
    scaledSepDist = scaledSepDist - v10;
    v26 = scaledSepAxis[1] * boxHalfSize[1];
    v9 = I_fabs(v26);
    scaledSepDist = scaledSepDist - v9;
    v25 = scaledSepAxis[2] * boxHalfSize[2];
    v8 = I_fabs(v25);
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
    if (radius <= dist)
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

void __cdecl AxisCopy(const mat3x3 &in, mat3x3 &out)
{
    memcpy(out, in, sizeof(mat3x3));
}


bool __cdecl Vec4IsNormalized(const float* v)
{
    float v2; // [esp+4h] [ebp-8h]
    float v3; // [esp+8h] [ebp-4h]

    v3 = Vec4LengthSq(v) - 1.0;
    v2 = I_fabs(v3);
    return v2 < 0.002f;
}

void __cdecl Vec4Copy(const float *from, float *to)
{
    *to = *from;
    to[1] = from[1];
    to[2] = from[2];
    to[3] = from[3];
}

bool __cdecl Vec4Compare(const float *a, const float *b)
{
    return *b == *a && b[1] == a[1] && b[2] == a[2] && b[3] == a[3];
}

void __cdecl Vec4Set(float *v, float x, float y, float z, float w)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
    v[3] = w;
}

void __cdecl Vec3MadMad(
    const float *start,
    float scale0,
    const float *dir0,
    float scale1,
    const float *dir1,
    float *result)
{
    *result = scale0 * *dir0 + *start + scale1 * *dir1;
    result[1] = scale0 * dir0[1] + start[1] + scale1 * dir1[1];
    result[2] = scale0 * dir0[2] + start[2] + scale1 * dir1[2];
}

float __cdecl Vec3Normalize(float* v)
{
    float v2; // [esp+0h] [ebp-14h]
    float v3; // [esp+4h] [ebp-10h]
    float ilength; // [esp+Ch] [ebp-8h]
    float length; // [esp+10h] [ebp-4h]

    length = *v * *v + v[1] * v[1] + v[2] * v[2];
    v3 = sqrt(length);
    if (-v3 < 0.0)
        v2 = v3;
    else
        v2 = 1.0;
    ilength = 1.0 / v2;
    *v = *v * ilength;
    v[1] = v[1] * ilength;
    v[2] = v[2] * ilength;
    return v3;
}

void __cdecl Vec3Mul(const float* a, const float* b, float* product)
{
    *product = *a * *b;
    product[1] = a[1] * b[1];
    product[2] = a[2] * b[2];
}

void __cdecl Vec3Negate(const float *from, float *to)
{
    to[0] = -from[0];
    to[1] = -from[1];
    to[2] = -from[2];
}

void __cdecl Vec3Lerp(const float* start, const float* end, float fraction, float* endpos)
{
    *endpos = (*end - *start) * fraction + *start;
    endpos[1] = (end[1] - start[1]) * fraction + start[1];
    endpos[2] = (end[2] - start[2]) * fraction + start[2];
}

void __cdecl Vec4Lerp(const float* from, const float* to, float frac, float* result)
{
    *result = (*to - *from) * frac + *from;
    result[1] = (to[1] - from[1]) * frac + from[1];
    result[2] = (to[2] - from[2]) * frac + from[2];
    result[3] = (to[3] - from[3]) * frac + from[3];
}

float __cdecl Vec4Length(const float *v)
{
    float v3; // [esp+4h] [ebp-4h]

    v3 = v[3] * v[3] + v[2] * v[2] + v[1] * v[1] + *v * *v;
    return sqrt(v3);
}

void __cdecl Vec4Scale(const float* v, float scale, float* result)
{
    *result = scale * *v;
    result[1] = scale * v[1];
    result[2] = scale * v[2];
    result[3] = scale * v[3];
}

void __cdecl Vec4Add(const float *a, const float *b, float *sum)
{
    *sum = *a + *b;
    sum[1] = a[1] + b[1];
    sum[2] = a[2] + b[2];
    sum[3] = a[3] + b[3];
}

void __cdecl Vec4Sub(const float *a, const float *b, float *diff)
{
    *diff = *a - *b;
    diff[1] = a[1] - b[1];
    diff[2] = a[2] - b[2];
    diff[3] = a[3] - b[3];
}

void __cdecl Vec4Mad(const float *start, float scale, const float *dir, float *result)
{
    *result = scale * *dir + *start;
    result[1] = scale * dir[1] + start[1];
    result[2] = scale * dir[2] + start[2];
    result[3] = scale * dir[3] + start[3];
}

void __cdecl Vec4MadMad(
    const float *start,
    float scale0,
    const float *dir0,
    float scale1,
    const float *dir1,
    float *result)
{
    result[0] = scale0 * dir0[0] + start[0] + scale1 * dir1[0];
    result[1] = scale0 * dir0[1] + start[1] + scale1 * dir1[1];
    result[2] = scale0 * dir0[2] + start[2] + scale1 * dir1[2];
    result[3] = scale0 * dir0[3] + start[3] + scale1 * dir1[3];
}

void __cdecl Vec3Mad(const float *start, float scale, const float *dir, float *result)
{
    result[0] = scale * dir[0] + start[0];
    result[1] = scale * dir[1] + start[1];
    result[2] = scale * dir[2] + start[2];
}

void __cdecl Vec3Accum(const float *subTotal, const float *weight, const float *added, float *total)
{
    total[0] = weight[0] * added[0] + subTotal[0];
    total[1] = weight[1] * added[1] + subTotal[1];
    total[2] = weight[2] * added[2] + subTotal[2];
}

void __cdecl Vec3Cross(const vec3r v0, const vec3r v1, vec3r cross)
{
    iassert(v0 != cross);
    iassert(v1 != cross);
    cross[0] = v0[1] * v1[2] - v0[2] * v1[1];
    cross[1] = v0[2] * v1[0] - v0[0] * v1[2];
    cross[2] = v0[0] * v1[1] - v0[1] * v1[0];
}

void __cdecl Vec3Copy(const vec3r from, vec3r to)
{
    to[0] = from[0];
    to[1] = from[1];
    to[2] = from[2];
}

float __cdecl Vec3Length(const vec3r v)
{
    return sqrtf((v[2] * v[2]) + (v[1] * v[1]) + (v[0] * v[0]));
}

float __cdecl Vec3Dot(const vec3r a, const vec3r b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float __cdecl Vec4Dot(const float *a, const float *b)
{
    return (float)(*a * *b + a[1] * b[1] + a[2] * b[2] + a[3] * b[3]);
}

void __cdecl Vec4Mul(const float *a, const float *b, float *product)
{
    *product = *a * *b;
    product[1] = a[1] * b[1];
    product[2] = a[2] * b[2];
    product[3] = a[3] * b[3];
}

void __cdecl MatrixForViewer(mat4x4 &mtx, const vec3r origin, const mat3x3 &axis)
{
    if (!mtx)
        MyAssertHandler(".\\universal\\com_math.cpp", 2302, 0, "%s", "mtx");
    if (!origin)
        MyAssertHandler(".\\universal\\com_math.cpp", 2303, 0, "%s", "origin");
    if (!axis)
        MyAssertHandler(".\\universal\\com_math.cpp", 2304, 0, "%s", "axis");
    (mtx)[0][0] = -(axis)[1][0];
    (mtx)[1][0] = -(axis)[1][1];
    (mtx)[2][0] = -(axis)[1][2];
    (mtx)[3][0] = -(*origin * (mtx)[0][0] + origin[1] * (mtx)[1][0] + origin[2] * (mtx)[2][0]);
    (mtx)[0][1] = (axis)[2][0];
    (mtx)[1][1] = (axis)[2][1];
    (mtx)[2][1] = (axis)[2][2];
    (mtx)[3][1] = -(*origin * (mtx)[0][1] + origin[1] * (mtx)[1][1] + origin[2] * (mtx)[2][1]);
    (mtx)[0][2] = (axis)[0][0];
    (mtx)[1][2] = (axis)[0][1];
    (mtx)[2][2] = (axis)[0][2];
    (mtx)[3][2] = -(*origin * (mtx)[0][2] + origin[1] * (mtx)[1][2] + origin[2] * (mtx)[2][2]);
    (mtx)[0][3] = 0.0;
    (mtx)[1][3] = 0.0;
    (mtx)[2][3] = 0.0;
    (mtx)[3][3] = 1.0;
}

void __cdecl InfinitePerspectiveMatrix(float (*mtx)[4], float tanHalfFovX, float tanHalfFovY, float zNear)
{
    if (!mtx)
        MyAssertHandler(".\\universal\\com_math.cpp", 2251, 0, "%s", "mtx");
    if (zNear <= 0.0)
        MyAssertHandler(".\\universal\\com_math.cpp", 2252, 0, "%s", "zNear > 0");
    memset((unsigned __int8 *)mtx, 0, sizeof(mat4x4));
    (*mtx)[0] = 0.99951172f / tanHalfFovX;
    (*mtx)[5] = 0.99951172f / tanHalfFovY;
    (*mtx)[10] = 0.99951172f;
    (*mtx)[11] = 1.0f;
    (*mtx)[14] = -zNear * 0.99951172f;
}

void __cdecl ClearBounds(float *mins, float *maxs)
{
    *mins = 131072.0f;
    mins[1] = 131072.0f;
    mins[2] = 131072.0f;
    *maxs = -131072.0f;
    maxs[1] = -131072.0f;
    maxs[2] = -131072.0f;
}

float __cdecl Vec4LengthSq(const float *v)
{
    return (v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
}

float __cdecl Vec3LengthSq(const float* v)
{
    return (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void __cdecl Vec3Scale(const float *v, float scale, float *result)
{
    result[0] = scale * v[0];
    result[1] = scale * v[1];
    result[2] = scale * v[2];
}

void __cdecl Vec3Add(const float *a, const float *b, float *sum)
{
    sum[0] = a[0] + b[0];
    sum[1] = a[1] + b[1];
    sum[2] = a[2] + b[2];
}

void __cdecl Vec3Avg(const float *a, const float *b, float *sum)
{
    *sum = (*a + *b) * 0.5;
    sum[1] = (a[1] + b[1]) * 0.5;
    sum[2] = (a[2] + b[2]) * 0.5;
}


void __cdecl Vec3ScaleMad(float scale0, const float *dir0, float scale1, const float *dir1, float *result)
{
    *result = scale0 * *dir0 + scale1 * *dir1;
    result[1] = scale0 * dir0[1] + scale1 * dir1[1];
    result[2] = scale0 * dir0[2] + scale1 * dir1[2];
}

bool __cdecl Vec3IsNormalized(const float *v)
{
    float v2; // [esp+4h] [ebp-8h]
    float v3; // [esp+8h] [ebp-4h]

    v3 = Vec3LengthSq(v) - 1.0;
    v2 = I_fabs(v3);
    return v2 < 0.002f;
}

void __cdecl QuatMultiplyReverseInverse(const float *in1, const float *in2, float *out)
{
    out[0] = (in2[0] * in1[3]) - (in2[3] * *in1) - (in2[2] * in1[1]) + (in2[1] * in1[2]);
    out[1] = (in2[1] * in1[3]) + (in2[2] * *in1) - (in2[3] * in1[1]) - (in2[0] * in1[2]);
    out[2] = (in2[2] * in1[3]) - (in2[1] * *in1) + (in2[0] * in1[1]) - (in2[3] * in1[2]);
    out[3] = (in2[3] * in1[3]) + (in2[0] * *in1) + (in2[1] * in1[1]) + (in2[2] * in1[2]);
}

void __cdecl QuatMultiplyReverseEquals(const float *in, float *inout)
{
    float temp[3]; // [esp+4h] [ebp-Ch]

    temp[0] =  (in[1] * inout[3]) - (in[2] * *inout) + (in[3] * inout[1]) + (in[0] * inout[2]);
    temp[1] =  (in[2] * inout[3]) + (in[1] * *inout) - (in[0] * inout[1]) + (in[3] * inout[2]);
    temp[2] =  (in[3] * inout[3]) - (in[0] * *inout) - (in[1] * inout[1]) - (in[2] * inout[2]);
    inout[0] = (in[0] * inout[3]) + (in[3] * *inout) + (in[2] * inout[1]) - (in[1] * inout[2]);
    inout[1] = temp[0];
    inout[2] = temp[1];
    inout[3] = temp[2];
}

void __cdecl QuatMultiplyInverse(const float *in1, const float *in2, float *out)
{
    out[0] = -in1[0] * in2[3]
        + in1[3] * in2[0]
        - in1[2] * in2[1]
        + in1[1] * in2[2];

    out[1] = -in1[1] * in2[3]
        + in1[2] * in2[0]
        + in1[3] * in2[1]
        - in1[0] * in2[2];

    out[2] = -in1[2] * in2[3]
        - in1[1] * in2[0]
        + in1[0] * in2[1]
        + in1[3] * in2[2];

    out[3] = in1[3] * in2[3]
        + in1[0] * in2[0]
        + in1[1] * in2[1]
        + in1[2] * in2[2];
}

void __cdecl R_TransformSkelMat(const float *origin, const DObjSkelMat *mat, float *out)
{
    *out = (float)((float)(*origin * mat->axis[0][0])
        + (float)((float)(mat->axis[2][0] * origin[2]) + (float)(mat->axis[1][0] * origin[1])))
        + mat->origin[0];
    out[1] = (float)((float)(mat->axis[1][1] * origin[1])
        + (float)((float)(mat->axis[0][1] * *origin) + (float)(mat->axis[2][1] * origin[2])))
        + mat->origin[1];
    out[2] = (float)((float)(mat->axis[1][2] * origin[1])
        + (float)((float)(mat->axis[0][2] * *origin) + (float)(mat->axis[2][2] * origin[2])))
        + mat->origin[2];
}

void __cdecl ConvertQuatToSkelMat(const DObjAnimMat *const mat, DObjSkelMat *skelMat)
{
    float scaledQuat[3];

    float yy;
    float xy;
    float zz;
    float zw;
    float yw;
    float xz;
    float yz;
    float xx;
    float xw;

    iassert((!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])));
    iassert((!IS_NAN(mat->transWeight)));

    Vec3Scale(mat->quat, mat->transWeight, scaledQuat);

    xx = scaledQuat[0] * mat->quat[0];
    xy = scaledQuat[0] * mat->quat[1];
    xz = scaledQuat[0] * mat->quat[2];
    xw = scaledQuat[0] * mat->quat[3];

    yy = scaledQuat[1] * mat->quat[1];
    yz = scaledQuat[1] * mat->quat[2];
    yw = scaledQuat[1] * mat->quat[3];

    zz = scaledQuat[2] * mat->quat[2];
    zw = scaledQuat[2] * mat->quat[3];

    skelMat->axis[0][0] = 1.0f - (float)(yy + zz);
    skelMat->axis[0][1] = xy + zw;
    skelMat->axis[0][2] = xz - yw;
    skelMat->axis[0][3] = 0.0f;

    skelMat->axis[1][0] = xy - zw;
    skelMat->axis[1][1] = 1.0f - (float)(xx + zz);
    skelMat->axis[1][2] = yz + xw;
    skelMat->axis[1][3] = 0.0f;

    skelMat->axis[2][0] = xz + yw;
    skelMat->axis[2][1] = yz - xw;
    skelMat->axis[2][2] = 1.0f - (float)(xx + yy);
    skelMat->axis[2][3] = 0.0f;

    skelMat->origin[0] = mat->trans[0];
    skelMat->origin[1] = mat->trans[1];
    skelMat->origin[2] = mat->trans[2];
    skelMat->origin[3] = 1.0f;
}

void __cdecl ConvertQuatToInverseSkelMat(const DObjAnimMat *const mat, DObjSkelMat *skelMat)
{
    float scaledQuat[3];

    float yy;
    float xy;
    float zz;
    float zw;
    float yw;
    float xz;
    float yz;
    float xx;
    float xw;

    iassert((!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])));
    iassert((!IS_NAN(mat->transWeight)));

    Vec3Scale(mat->quat, mat->transWeight, scaledQuat);

    xx = scaledQuat[0] * mat->quat[0];
    xy = scaledQuat[0] * mat->quat[1];
    xz = scaledQuat[0] * mat->quat[2];
    xw = scaledQuat[0] * mat->quat[3];

    yy = scaledQuat[1] * mat->quat[1];
    yz = scaledQuat[1] * mat->quat[2];
    yw = scaledQuat[1] * mat->quat[3];

    zz = scaledQuat[2] * mat->quat[2];
    zw = scaledQuat[2] * mat->quat[3];

    skelMat->axis[0][0] = 1.0f - (yy + zz);
    skelMat->axis[0][1] = xy - zw;
    skelMat->axis[0][2] = xz + yw;
    skelMat->axis[0][3] = 0.0f;

    skelMat->axis[1][0] = xy + zw;
    skelMat->axis[1][1] = 1.0f - (xx + zz);
    skelMat->axis[1][2] = yz - xw;
    skelMat->axis[1][3] = 0.0f;

    skelMat->axis[2][0] = xz - yw;
    skelMat->axis[2][1] = yz + xw;
    skelMat->axis[2][2] = 1.0f - (xx + yy);
    skelMat->axis[2][3] = 0.0f;

    skelMat->origin[0] = -((mat->trans[0] * skelMat->axis[0][0]) + (mat->trans[1] * skelMat->axis[1][0]) + (mat->trans[2] * skelMat->axis[2][0]));// ^_mask__NegFloat_;
    skelMat->origin[1] = -((mat->trans[0] * skelMat->axis[0][1]) + (mat->trans[1] * skelMat->axis[1][1]) + (mat->trans[2] * skelMat->axis[2][1]));// ^_mask__NegFloat_;
    skelMat->origin[2] = -((mat->trans[0] * skelMat->axis[0][2]) + (mat->trans[1] * skelMat->axis[1][2]) + (mat->trans[2] * skelMat->axis[2][2]));// ^_mask__NegFloat_;
    skelMat->origin[3] = 1.0f;
}

void __cdecl FinitePerspectiveMatrix(float (*mtx)[4], float tanHalfFovX, float tanHalfFovY, float zNear, float zFar)
{
    if (!mtx)
        MyAssertHandler(".\\universal\\com_math.cpp", 2217, 0, "%s", "mtx");
    if (zNear <= 0.0)
        MyAssertHandler(".\\universal\\com_math.cpp", 2218, 0, "zNear > 0.0f\n\t%g, %g", zNear, 0.0);
    if (zNear >= (double)zFar)
        MyAssertHandler(".\\universal\\com_math.cpp", 2219, 0, "zFar > zNear\n\t%g, %g", zFar, zNear);
    memset((unsigned __int8 *)mtx, 0, 0x40u);
    (*mtx)[0] = 1.0 / tanHalfFovX;
    (*mtx)[5] = 1.0 / tanHalfFovY;
    (*mtx)[10] = -zFar / (zNear - zFar);
    (*mtx)[11] = 1.0;
    (*mtx)[14] = zNear * zFar / (zNear - zFar);
}

// KISAKTODO: double check this function's logic
float LerpAngle(float from, float to, float frac)
{
    float delta = fmodf(to - from + 540.0f, 360.0f) - 180.0f;
    return from + delta * frac;
}

float PointToLineDistSq2D(const float *point, const float *start, const float *end)
{
    float dx = end[0] - start[0];
    float dy = end[1] - start[1];

    float px = point[0] - start[0];
    float py = point[1] - start[1];

    float segDot = dx * dx + dy * dy;

    if (segDot == 0.0f)
    {
        iassert(0);
        return 0.0f;
    }

    float proj = -(px * dx + py * dy) / segDot;

    float projX = dx * proj + px;
    float projY = dy * proj + py;

    float distSq = projX * projX + projY * projY;
    return distSq;
}

// aislop
int IsPosInsideArc(
    const float *pos,
    float posRadius,
    const float *arcOrigin,
    float arcRadius,
    float arcAngle0,
    float arcAngle1,
    float arcHalfHeight)
{
    iassert(pos);
    iassert(arcOrigin);

    float dx = pos[0] - arcOrigin[0];
    float dy = pos[1] - arcOrigin[1];
    float horizontalDist = sqrtf(dx * dx + dy * dy);

    float zDiff = pos[2] - arcOrigin[2];
    float verticalLowerBound = arcOrigin[2] - arcHalfHeight;
    float verticalUpperBound = arcOrigin[2] + arcHalfHeight;

    float effectiveDist = horizontalDist - (float)posRadius;

    if ((effectiveDist * effectiveDist <= arcRadius * arcRadius) &&
        (pos[2] >= verticalLowerBound) &&
        (pos[2] <= verticalUpperBound))
    {
        //float invLen = 1.0f / fmaxf(horizontalDist, 1e-6f);  // prevent divide by zero
        float invLen = 1.0f / horizontalDist;

        float dir[3] = 
        {
            dx * invLen,
            dy * invLen,
            zDiff  // unused in yaw calculation, but preserved in vector format
        };

        float yaw = vectoyaw(dir);
        float normYaw = AngleNormalize360(yaw);

        if (arcAngle0 >= arcAngle1) 
        {
            if (normYaw < arcAngle1 || normYaw > arcAngle0)
                return 1;
        }
        else 
        {
            if (normYaw < arcAngle1 && normYaw > arcAngle0)
                return 1;
        }
    }

    return 0;
}

// aislop
void ProjectPointOntoVector(const float *point, const float *start, const float *end, float *vProj)
{
    float dir[3] = 
    {
        end[0] - start[0],
        end[1] - start[1],
        end[2] - start[2]
    };

    float lengthSq = dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2];
    if (lengthSq == 0.0f) 
    {
        // Degenerate line (start == end), projection is just the start point
        vProj[0] = start[0];
        vProj[1] = start[1];
        vProj[2] = start[2];
        return;
    }

    float toPoint[3] = 
    {
        point[0] - start[0],
        point[1] - start[1],
        point[2] - start[2]
    };

    // Project point onto the line direction
    float dot = toPoint[0] * dir[0] + toPoint[1] * dir[1] + toPoint[2] * dir[2];
    float t = dot / lengthSq;

    vProj[0] = start[0] + t * dir[0];
    vProj[1] = start[1] + t * dir[1];
    vProj[2] = start[2] + t * dir[2];
}

float Q_fabs(float f) 
{
    int tmp = *(int *)&f;
    tmp &= 0x7FFFFFFF;
    return *(float *)&tmp;
}

void vectosignedangles(const float *vec, float *angles)
{
    float yaw, pitch;

    // Handle the special case where horizontal components are zero
    if (vec[0] == 0.0f && vec[1] == 0.0f) {
        yaw = 0.0f;
        pitch = (vec[2] < 0.0f) ? -90.0f : 90.0f;
    }
    else {
        // Compute yaw (angle in the XY plane)
        yaw = RAD2DEG(atan2f(vec[1], vec[0]));

        // Compute horizontal distance (magnitude in the XY plane)
        float xyLen = sqrtf(vec[0] * vec[0] + vec[1] * vec[1]);

        // Compute pitch (elevation from horizontal plane)
        pitch = RAD2DEG(atan2f(vec[2], xyLen));
    }

    angles[0] = pitch;  // X = pitch
    angles[1] = yaw;    // Y = yaw
    angles[2] = 0.0f;   // Z = roll (unused in this function)
}

// KISAKTODO: Feel like it could be improved
void FastSinCos(float radians, float *s, float *c)
{
    *s = sinf(radians);
    *c = cosf(radians);
}

// aislop
void MatrixRotationX(float mat[3][3], float degree)
{
    float radians = degree * (3.14159265f / 180.0f);
    float s = sinf(radians);
    float c = cosf(radians);

    mat[0][0] = 1.0f; mat[0][1] = 0.0f; mat[0][2] = 0.0f;
    mat[1][0] = 0.0f; mat[1][1] = c; mat[1][2] = -s;
    mat[2][0] = 0.0f; mat[2][1] = s; mat[2][2] = c;
}


// aislop
void MatrixRotationY(float mat[3][3], float degree)
{
    float radians = degree * (3.14159265f / 180.0f);
    float s = sinf(radians);
    float c = cosf(radians);

    mat[0][0] = c;  mat[0][1] = 0.0f; mat[0][2] = -s;
    mat[1][0] = 0.0f; mat[1][1] = 1.0f; mat[1][2] = 0.0f;
    mat[2][0] = s;  mat[2][1] = 0.0f; mat[2][2] = c;
}

// aislop
void MatrixRotationZ(float mat[3][3], float degree)
{
    float radians = degree * (3.14159265f / 180.0f);
    float s = sinf(radians);
    float c = cosf(radians);

    mat[0][0] = c; mat[0][1] = -s; mat[0][2] = 0.0f;
    mat[1][0] = s; mat[1][1] = c; mat[1][2] = 0.0f;
    mat[2][0] = 0.0f; mat[2][1] = 0.0f; mat[2][2] = 1.0f;
}

void Vec3Basis_LeftHanded(const float *forward, float *right, float *up)
{
    PerpendicularVector(forward, up);
    Vec3Cross(forward, up, right);
}

float Vec3DistanceSq(const float *p1, const float *p2)
{
    return (p2[2] - p1[2]) * (p2[2] - p1[2]) 
         + (p2[1] - p1[1]) * (p2[1] - p1[1]) 
         + (p2[0] - p1[0]) * (p2[0] - p1[0]);
}