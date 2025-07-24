#include "ragdoll.h"

//float *unitQuaternion     827c055c     ragdoll_quat.obj

void __cdecl Ragdoll_QuatMul(const float *qa, const float *qb, float *dest)
{
    float w0; // [esp+0h] [ebp-8h]
    float w1; // [esp+4h] [ebp-4h]

    if (qa == dest || qb == dest)
        MyAssertHandler(".\\ragdoll\\ragdoll_quat.cpp", 24, 0, "%s", "qa != dest && qb != dest");
    w0 = qa[3];
    w1 = qb[3];
    *dest = w0 * *qb + w1 * *qa + qa[1] * qb[2] - qa[2] * qb[1];
    dest[1] = w0 * qb[1] + w1 * qa[1] + qa[2] * *qb - *qa * qb[2];
    dest[2] = w0 * qb[2] + w1 * qa[2] + *qa * qb[1] - qa[1] * *qb;
    dest[3] = w0 * w1 - *qa * *qb - qa[1] * qb[1] - qa[2] * qb[2];
}

void __cdecl Ragdoll_QuatMulInvSecond(const float *qa, const float *qb, float *dest)
{
    float qinv[4]; // [esp+0h] [ebp-10h] BYREF

    Ragdoll_QuatConjugate(qb, qinv);
    Ragdoll_QuatMul(qa, qinv, dest);
}

void __cdecl Ragdoll_QuatConjugate(const float *src, float *dest)
{
    *dest = -*src;
    dest[1] = -src[1];
    dest[2] = -src[2];
    dest[3] = src[3];
}

void __cdecl Ragdoll_QuatInverse(const float *src, float *dest)
{
    const char *v2; // eax

    if (!Vec4IsNormalized(src))
    {
        v2 = va("%g %g %g %g", *src, src[1], src[2], src[3]);
        MyAssertHandler(".\\ragdoll\\ragdoll_quat.cpp", 94, 0, "%s\n\t%s", "Vec4IsNormalized( src )", v2);
    }
    *dest = -*src;
    dest[1] = -src[1];
    dest[2] = -src[2];
    dest[3] = src[3];
}

void __cdecl Ragdoll_QuatPointRotate(const float *p, const float *q, float *dest)
{
    float tmp0[4]; // [esp+0h] [ebp-40h] BYREF
    float qp[4]; // [esp+10h] [ebp-30h] BYREF
    float tmp1[4]; // [esp+20h] [ebp-20h] BYREF
    float qInv[4]; // [esp+30h] [ebp-10h] BYREF

    qp[0] = *p;
    qp[1] = p[1];
    qp[2] = p[2];
    qp[3] = 0.0;
    Ragdoll_QuatInverse(q, qInv);
    Ragdoll_QuatMul(q, qInv, tmp0);
    Ragdoll_QuatMul(tmp0, qp, tmp1);
    *dest = tmp1[0];
    dest[1] = tmp1[1];
    dest[2] = tmp1[2];
}

void __cdecl Ragdoll_QuatNormalize(float *quat)
{
    Vec4Normalize(quat);
}

void __cdecl Ragdoll_Mat33ToQuat(const float (*axis)[3], float *quat)
{
    double v2; // st7
    float v3; // [esp+0h] [ebp-24h]
    float v4; // [esp+4h] [ebp-20h]
    float v5; // [esp+8h] [ebp-1Ch]
    float v6; // [esp+Ch] [ebp-18h]
    float v7; // [esp+10h] [ebp-14h]
    float v8; // [esp+14h] [ebp-10h]
    float v9; // [esp+18h] [ebp-Ch]
    float trace; // [esp+1Ch] [ebp-8h]
    float s; // [esp+20h] [ebp-4h]
    float sa; // [esp+20h] [ebp-4h]
    float sb; // [esp+20h] [ebp-4h]
    float sc; // [esp+20h] [ebp-4h]

    trace = (*axis)[0] + (float)(*axis)[4] + (float)(*axis)[8] + 1.0;
    if (trace <= 0.000001)
    {
        if ((*axis)[4] >= (double)(*axis)[0] || (*axis)[8] >= (double)(*axis)[0])
        {
            if ((*axis)[8] >= (double)(*axis)[4])
            {
                v7 = (float)(*axis)[8] + 1.0 - (*axis)[0] - (float)(*axis)[4];
                v3 = sqrt(v7);
                sc = v3 * 2.0;
                *quat = ((float)(*axis)[2] + (float)(*axis)[6]) / sc;
                quat[1] = ((float)(*axis)[5] + (float)(*axis)[7]) / sc;
                quat[2] = sc * 0.25;
                v2 = ((float)(*axis)[1] - (float)(*axis)[3]) / sc;
            }
            else
            {
                v8 = (float)(*axis)[4] + 1.0 - (*axis)[0] - (float)(*axis)[8];
                v4 = sqrt(v8);
                sb = v4 * 2.0;
                *quat = ((float)(*axis)[1] + (float)(*axis)[3]) / sb;
                quat[1] = sb * 0.25;
                quat[2] = ((float)(*axis)[5] + (float)(*axis)[7]) / sb;
                v2 = ((float)(*axis)[2] - (float)(*axis)[6]) / sb;
            }
            quat[3] = v2;
        }
        else
        {
            v9 = (*axis)[0] + 1.0 - (float)(*axis)[4] - (float)(*axis)[8];
            v5 = sqrt(v9);
            sa = v5 * 2.0;
            *quat = sa * 0.25;
            quat[1] = ((float)(*axis)[1] + (float)(*axis)[3]) / sa;
            quat[2] = ((float)(*axis)[2] + (float)(*axis)[6]) / sa;
            quat[3] = ((float)(*axis)[5] - (float)(*axis)[7]) / sa;
        }
    }
    else
    {
        v6 = sqrt(trace);
        s = 0.5 / v6;
        *quat = ((float)(*axis)[7] - (float)(*axis)[5]) * s;
        quat[1] = ((float)(*axis)[2] - (float)(*axis)[6]) * s;
        quat[2] = ((float)(*axis)[3] - (float)(*axis)[1]) * s;
        quat[3] = 0.25 / s;
    }
}

void __cdecl Ragdoll_QuatToAxisAngle(const float *quat, float *axisAngle)
{
    float v2; // [esp+4h] [ebp-18h]
    float v3; // [esp+8h] [ebp-14h]
    float angleMag; // [esp+Ch] [ebp-10h]
    float halfTheta; // [esp+14h] [ebp-8h]

    halfTheta = Q_acos(quat[3]);
    v3 = sin(halfTheta);
    v2 = I_fabs(v3);
    if (v2 <= 0.000001)
    {
        *axisAngle = 0.0;
        axisAngle[1] = 0.0;
        axisAngle[2] = 0.0;
    }
    else
    {
        angleMag = halfTheta * 2.0 / v3;
        *axisAngle = *quat * angleMag;
        axisAngle[1] = quat[1] * angleMag;
        axisAngle[2] = quat[2] * angleMag;
    }
}

void __cdecl Ragdoll_QuatLerp(const float *qa, const float *qb, float t, float *dest)
{
    Vec4Lerp(qa, qb, t, dest);
    Vec4Normalize(dest);
}

