#include "game_public.h"
#include <client/client.h>
#include <cgame/cg_public.h>


void __cdecl G_DebugLine(const float *start, const float *end, const float *color, int depthTest)
{
#ifndef DEDICATED
    CL_AddDebugLine(start, end, color, depthTest, 0, 1);
#endif
}

void __cdecl G_DebugLineWithDuration(
    const float *start,
    const float *end,
    const float *color,
    int depthTest,
    int duration)
{
#ifndef DEDICATED
    CL_AddDebugLine(start, end, color, depthTest, duration, 1);
#endif
}

void __cdecl G_DebugStar(const float *point, const float *color)
{
#ifndef DEDICATED
    CL_AddDebugStar(point, color, 0, 1);
#endif
}

void __cdecl G_DebugStarWithText(
    const float *point,
    const float *starColor,
    const float *textColor,
    char *string,
    float fontsize)
{
#ifndef DEDICATED
    CL_AddDebugStarWithText(point, starColor, textColor, string, fontsize, 0, 1);
#endif
}

void __cdecl G_DebugBox(
    const float *origin,
    const float *mins,
    const float *maxs,
    float yaw,
    const float *color,
    int depthTest,
    int duration)
{
#ifndef DEDICATED
    float v7; // [esp+0h] [ebp-94h]
    float v8; // [esp+10h] [ebp-84h]
    unsigned int j; // [esp+14h] [ebp-80h]
    float rotated; // [esp+18h] [ebp-7Ch]
    float rotated_4; // [esp+1Ch] [ebp-78h]
    unsigned int i; // [esp+24h] [ebp-70h]
    unsigned int ia; // [esp+24h] [ebp-70h]
    float fCos; // [esp+28h] [ebp-6Ch]
    float v[25]; // [esp+2Ch] [ebp-68h] BYREF
    float fSin; // [esp+90h] [ebp-4h]

    v8 = yaw * 0.01745329238474369;
    fCos = cos(v8);
    fSin = sin(v8);
    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 3; ++j)
        {
            if ((i & (1 << j)) != 0)
                v7 = maxs[j];
            else
                v7 = mins[j];
            v[3 * i + j] = v7;
        }
        rotated = v[3 * i] * fCos - v[3 * i + 1] * fSin;
        rotated_4 = v[3 * i] * fSin + v[3 * i + 1] * fCos;
        v[3 * i] = rotated;
        v[3 * i + 1] = rotated_4;
        Vec3Add(&v[3 * i], origin, &v[3 * i]);
    }
    for (ia = 0; ia < 0xC; ++ia)
        G_DebugLineWithDuration(&v[3 * iEdgePairs[ia][0]], &v[3 * iEdgePairs[ia][1]], color, depthTest, duration);
#endif
}

void __cdecl G_DebugCircle(
    const float *center,
    float radius,
    const float *color,
    int depthTest,
    int onGround,
    int duration)
{
#ifndef DEDICATED
    float eyepos[3]; // [esp+18h] [ebp-18h] BYREF
    float dir[3]; // [esp+24h] [ebp-Ch] BYREF

    if (onGround)
    {
        dir[0] = 0.0;
        dir[1] = 0.0;
        dir[2] = 1.0;
    }
    else
    {
        eyepos[0] = level.clients->ps.origin[0];
        eyepos[1] = level.clients->ps.origin[1];
        eyepos[2] = level.clients->ps.origin[2];
        eyepos[2] = eyepos[2] + level.clients->ps.viewHeightCurrent;
        Vec3Sub(center, eyepos, dir);
    }
    G_DebugCircleEx(center, radius, dir, color, depthTest, duration);
#endif
}

void __cdecl G_DebugCircleEx(
    const float *center,
    float radius,
    const float *dir,
    const float *color,
    int depthTest,
    int duration)
{
#ifndef DEDICATED
    float fAngle; // [esp+1Ch] [ebp-F4h]
    float fCos; // [esp+20h] [ebp-F0h]
    float fCosa; // [esp+20h] [ebp-F0h]
    float fSin; // [esp+24h] [ebp-ECh]
    float fSina; // [esp+24h] [ebp-ECh]
    float normal[3]; // [esp+28h] [ebp-E8h] BYREF
    float right[3]; // [esp+34h] [ebp-DCh] BYREF
    float up[3]; // [esp+40h] [ebp-D0h] BYREF
    unsigned int i; // [esp+4Ch] [ebp-C4h]
    float v[16][3]; // [esp+50h] [ebp-C0h] BYREF

    Vec3NormalizeTo(dir, normal);
    PerpendicularVector(normal, right);
    Vec3Cross(normal, right, up);
    for (i = 0; i < 0x10; ++i)
    {
        fAngle = (double)i * 0.3926990926265717;
        fCos = cos(fAngle);
        fSin = sin(fAngle);
        fSina = fSin * radius;
        fCosa = fCos * radius;
        Vec3Mad(center, fSina, up, v[i]);
        Vec3Mad(v[i], fCosa, right, v[i]);
    }
    for (i = 0; i < 0x10; ++i)
        G_DebugLineWithDuration(v[i], v[(i + 1) % 0x10], color, depthTest, duration);
#endif
}

