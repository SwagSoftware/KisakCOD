#include "r_utils.h"
#include <universal/com_memory.h>
#include <aim_assist/aim_assist.h>
#include "r_dvars.h"
#include <universal/surfaceflags.h>

unsigned int __cdecl R_HashAssetName(const char *name)
{
    const char *pos; // [esp+0h] [ebp-8h]
    unsigned int hash; // [esp+4h] [ebp-4h]

    hash = 0;
    for (pos = name; *pos; ++pos)
    {
        if (*pos >= 65 && *pos <= 90)
            MyAssertHandler(".\\r_utils.cpp", 42, 0, "%s\n\t(name) = %s", "(*pos < 'A' || *pos > 'Z')", name);
        if (*pos == 92)
            MyAssertHandler(".\\r_utils.cpp", 43, 0, "%s\n\t(name) = %s", "(*pos != '\\\\' || *pos == '/')", name);
        hash = *pos ^ (33 * hash);
    }
    return hash;
}

unsigned int __cdecl R_HashString(const char *string)
{
    unsigned int hash; // [esp+4h] [ebp-4h]

    hash = 0;
    while (*string)
        hash = (*string++ | 0x20) ^ (33 * hash);
    return hash;
}

char *__cdecl R_AllocGlobalVariable(unsigned int bytes, const char *name)
{
    return Z_VirtualAlloc(bytes, name, 18);
}

char __cdecl R_CullPointAndRadius(const float *pt, float radius, const DpvsPlane *clipPlanes, int clipPlaneCount)
{
    float dist; // [esp+0h] [ebp-8h]
    int planeIndex; // [esp+4h] [ebp-4h]

    for (planeIndex = 0; planeIndex < clipPlaneCount; ++planeIndex)
    {
        dist = Vec3Dot(pt, clipPlanes[planeIndex].coeffs) + clipPlanes[planeIndex].coeffs[3];
        if (dist < -radius)
            return 1;
    }
    return 0;
}

void __cdecl R_ConvertColorToBytes(const float *colorFloat, unsigned int *colorBytes)
{
    if (colorFloat)
        Byte4PackVertexColor(colorFloat, (unsigned char*)colorBytes);
    else
        *colorBytes = -1;
}

double __cdecl FresnelTerm(float n0, float n1, float cosIncidentAngle)
{
    float v4; // [esp+10h] [ebp-5Ch]
    float v5; // [esp+14h] [ebp-58h]
    long double sinTransmissionAngle; // [esp+1Ch] [ebp-50h]
    double sinRatio; // [esp+24h] [ebp-48h]
    double tanSum; // [esp+2Ch] [ebp-40h]
    long double tanRatio; // [esp+3Ch] [ebp-30h]
    double incidentAngle; // [esp+44h] [ebp-28h]
    double transmissionAngle; // [esp+4Ch] [ebp-20h]
    double sinSum; // [esp+54h] [ebp-18h]
    float refraction; // [esp+60h] [ebp-Ch]

    if (cosIncidentAngle < -1.0)
        MyAssertHandler(
            ".\\r_utils.cpp",
            213,
            1,
            "%s\n\t(cosIncidentAngle) = %g",
            "(cosIncidentAngle >= -1)",
            cosIncidentAngle);
    if (cosIncidentAngle > 1.0)
        MyAssertHandler(
            ".\\r_utils.cpp",
            214,
            1,
            "%s\n\t(cosIncidentAngle) = %g",
            "(cosIncidentAngle <= 1)",
            cosIncidentAngle);
    v5 = I_fabs(cosIncidentAngle);
    v4 = acos(v5);
    incidentAngle = v4;
    sinTransmissionAngle = sin(v4) * (n0 / n1);
    if (sinTransmissionAngle <= 1.0)
    {
        if (sinTransmissionAngle < -1.0)
            sinTransmissionAngle = -1.0;
    }
    else
    {
        sinTransmissionAngle = 1.0;
    }
    transmissionAngle = asin(sinTransmissionAngle);
    sinSum = sin(incidentAngle + transmissionAngle);
    if (0.0 == sinSum)
        MyAssertHandler(".\\r_utils.cpp", 227, 1, "%s\n\t(sinSum) = %g", "(sinSum != 0)", sinSum);
    sinRatio = sin(incidentAngle - transmissionAngle) / sinSum;
    tanSum = tan(incidentAngle + transmissionAngle);
    if (0.0 == tanSum)
        MyAssertHandler(".\\r_utils.cpp", 233, 1, "%s\n\t(tanSum) = %g", "(tanSum != 0)", tanSum);
    tanRatio = tan(incidentAngle - transmissionAngle) / tanSum;
    refraction = (sinRatio * sinRatio + tanRatio * tanRatio) * 0.5;
    if (refraction < 0.0)
        return 0.0;
    if (refraction <= 1.0)
        return refraction;
    return 1.0;
}

char __cdecl R_GetClearColor(float *unpackedRgba)
{
    if (r_clear->current.integer && (r_clear->current.integer != 1 || developer->current.integer))
    {
        if (r_clear->current.integer != 4 || rg.fogSettings[2].density == 0.0)
        {
            if (r_clear->current.integer == 3 || (Sys_Milliseconds() & 0x200) == 0)
            {
                Byte4UnpackRgba((const unsigned __int8 *)&r_clearColor->current, unpackedRgba);
                unpackedRgba[3] = 1.0;
                return 1;
            }
            else
            {
                Byte4UnpackRgba((const unsigned __int8 *)&r_clearColor2->current, unpackedRgba);
                unpackedRgba[3] = 1.0;
                return 1;
            }
        }
        else
        {
            Byte4UnpackBgra((const unsigned __int8 *)&rg.fogSettings[2].color, unpackedRgba);
            unpackedRgba[3] = 1.0;
            return 1;
        }
    }
    else
    {
        *unpackedRgba = 0.0;
        unpackedRgba[1] = 0.0;
        unpackedRgba[2] = 0.0;
        unpackedRgba[3] = 0.0;
        return 0;
    }
}

void __cdecl Byte4UnpackBgra(const unsigned __int8 *from, float *to)
{
    *to = (double)from[2] * 0.003921568859368563;
    to[1] = (double)from[1] * 0.003921568859368563;
    to[2] = (double)*from * 0.003921568859368563;
    to[3] = (double)from[3] * 0.003921568859368563;
}

