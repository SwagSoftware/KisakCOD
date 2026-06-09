#include "com_math.h"

#include <math.h> //sin(), cos()
#include "q_shared.h"

// guessing this is in its own file because they have some fancy XBOX ifdef with handmade ASM

void __cdecl AngleVectors(const float *angles, float *forward, float *right, float *up)
{
    float		angle;
    float		sr, sp, sy, cr, cp, cy;

    angle = angles[YAW] * (M_PI * 2 / 360.0);
    cy = cos(angle);
    sy = sin(angle);
    angle = angles[PITCH] * (M_PI * 2 / 360.0);
    sp = sin(angle);
    cp = cos(angle);

	if (forward)
	{
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;
	}
	if (right || up)
	{
		angle = angles[ROLL] * (M_PI*2 / 360.0);
		sr = sin(angle);
		cr = cos(angle);
		if (right)
		{
			right[0] = (-sr*sp*cy + cr*sy);
			right[1] = (-sr*sp*sy + -cr*cy);
			right[2] = -sr*cp;
		}
		if (up)
		{
			up[0] = (cr*sp*cy + sr*sy);
			up[1] = (cr*sp*sy + -sr*cy);
			up[2] = cr*cp;
		}
	}
}

void __cdecl AnglesToAxis(const float *angles, float axis[3][3])
{
    // This is basically AngleVectors() and then a right subtract, 
    // but it's been optimized to not calculate and then negate right in this function
    float		angle;
    /*static*/ float		sr, sp, sy, cr, cp, cy;

    angle = angles[YAW] * (M_PI * 2 / 360.0);
    cy = cos(angle);
    sy = sin(angle);
    angle = angles[PITCH] * (M_PI * 2 / 360.0);
    cp = cos(angle);
    sp = sin(angle);

    //forward
    axis[0][0] = cp*cy;
    axis[0][1] = cp*sy;
    axis[0][2] = -sp;

    angle = angles[ROLL] * (M_PI * 2 / 360.0);
    cr = cos(angle);
    sr = sin(angle);
    //right (negated)
    axis[1][0] = sr*sp*cy + -sy*cr;
    axis[1][1] = sr*sp*sy + cr*cy;
    axis[1][2] = sr*cp;
    //up
    axis[2][0] = cr*sp*cy + -sr*-sy;
    axis[2][1] = cr*sp*sy + -sr*cy;
    axis[2][2] = cr*cp;
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

