#include "com_math.h"

#include <math.h> //sin(), cos()

void __cdecl AngleVectors(const float *angles, float *forward, float *right, float *up)
{
    float cy; // [esp+18h] [ebp-1Ch]
    float angle; // [esp+1Ch] [ebp-18h]
    float anglea; // [esp+1Ch] [ebp-18h]
    float angleb; // [esp+1Ch] [ebp-18h]
    float sr; // [esp+20h] [ebp-14h]
    float v9; // [esp+24h] [ebp-10h]
    float cr; // [esp+28h] [ebp-Ch]
    float cp; // [esp+2Ch] [ebp-8h]
    float sy; // [esp+30h] [ebp-4h]

    angle = angles[1] * 0.01745329238474369;
    cy = cos(angle);
    sy = sin(angle);
    anglea = *angles * 0.01745329238474369;
    cp = cos(anglea);
    v9 = sin(anglea);
    if (forward)
    {
        *forward = cp * cy;
        forward[1] = cp * sy;
        forward[2] = -v9;
    }
    if (right || up)
    {
        angleb = angles[2] * 0.01745329238474369;
        cr = cos(angleb);
        sr = sin(angleb);
        if (right)
        {
            *right = sr * -1.0 * v9 * cy + cr * -1.0 * -sy;
            right[1] = sr * -1.0 * v9 * sy + cr * -1.0 * cy;
            right[2] = sr * -1.0 * cp;
        }
        if (up)
        {
            *up = cr * v9 * cy + -sr * -sy;
            up[1] = cr * v9 * sy + -sr * cy;
            up[2] = cr * cp;
        }
    }
}

void __cdecl AnglesToAxis(const float *angles, float axis[3][3])
{
    float cy; // [esp+18h] [ebp-1Ch]
    float angle; // [esp+1Ch] [ebp-18h]
    float anglea; // [esp+1Ch] [ebp-18h]
    float angleb; // [esp+1Ch] [ebp-18h]
    float sr; // [esp+20h] [ebp-14h]
    float v7; // [esp+24h] [ebp-10h]
    float cr; // [esp+28h] [ebp-Ch]
    float cp; // [esp+2Ch] [ebp-8h]
    float sy; // [esp+30h] [ebp-4h]

    angle = angles[1] * 0.01745329238474369;
    cy = cos(angle);
    sy = sin(angle);
    anglea = *angles * 0.01745329238474369;
    cp = cos(anglea);
    v7 = sin(anglea);
    axis[0][0] = cp * cy;
    axis[0][1] = cp * sy;
    axis[0][2] = -v7;
    angleb = angles[2] * 0.01745329238474369;
    cr = cos(angleb);
    sr = sin(angleb);
    axis[1][0] = sr * v7 * cy + -sy * cr;
    axis[1][1] = sr * v7 * sy + cr * cy;
    axis[1][2] = sr * cp;
    axis[2][0] = cr * v7 * cy + -sr * -sy;
    axis[2][1] = cr * v7 * sy + -sr * cy;
    axis[2][2] = cr * cp;
}

float __cdecl Vec4Normalize(float *v)
{
    float v2; // [esp+0h] [ebp-Ch]
    float ilength; // [esp+4h] [ebp-8h]
    float length; // [esp+8h] [ebp-4h]

    length = *v * *v + v[1] * v[1] + v[2] * v[2] + v[3] * v[3];
    v2 = sqrt(length);
    if (v2 != 0.0)
    {
        ilength = 1.0 / v2;
        *v = *v * ilength;
        v[1] = v[1] * ilength;
        v[2] = v[2] * ilength;
        v[3] = v[3] * ilength;
    }
    return v2;
}

void __cdecl AnglesToQuat(const float *angles, float *quat)
{
    float axis[3][3]; // [esp+0h] [ebp-24h] BYREF

    AnglesToAxis(angles, axis);
    AxisToQuat(axis, quat); // LWSS: in com_math.cpp
}

