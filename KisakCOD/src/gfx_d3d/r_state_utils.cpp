#include "r_utils.h"
#include <universal/com_memory.h>
#include <aim_assist/aim_assist.h>
#include "r_state.h"
#include "r_dvars.h"

void __cdecl Byte4PackVertexColor(const float *from, unsigned __int8 *to)
{
    unsigned __int8 v2; // [esp+0h] [ebp-90h]
    unsigned __int8 v3; // [esp+4h] [ebp-8Ch]
    unsigned __int8 v4; // [esp+8h] [ebp-88h]
    unsigned __int8 v5; // [esp+Ch] [ebp-84h]
    int v6; // [esp+18h] [ebp-78h]
    float v7; // [esp+20h] [ebp-70h]
    int v8; // [esp+38h] [ebp-58h]
    float v9; // [esp+40h] [ebp-50h]
    int v10; // [esp+58h] [ebp-38h]
    float v11; // [esp+60h] [ebp-30h]
    int v12; // [esp+78h] [ebp-18h]
    float v13; // [esp+80h] [ebp-10h]

    v13 = *from * 255.0;
    if ((int)(v13 + 9.313225746154785e-10) < 255)
        v12 = (int)(v13 + 9.313225746154785e-10);
    else
        v12 = 255;
    if (v12 > 0)
        v5 = v12;
    else
        v5 = 0;
    to[2] = v5;
    v11 = from[1] * 255.0;
    if ((int)(v11 + 9.313225746154785e-10) < 255)
        v10 = (int)(v11 + 9.313225746154785e-10);
    else
        v10 = 255;
    if (v10 > 0)
        v4 = v10;
    else
        v4 = 0;
    to[1] = v4;
    v9 = from[2] * 255.0;
    if ((int)(v9 + 9.313225746154785e-10) < 255)
        v8 = (int)(v9 + 9.313225746154785e-10);
    else
        v8 = 255;
    if (v8 > 0)
        v3 = v8;
    else
        v3 = 0;
    *to = v3;
    v7 = from[3] * 255.0;
    if ((int)(v7 + 9.313225746154785e-10) < 255)
        v6 = (int)(v7 + 9.313225746154785e-10);
    else
        v6 = 255;
    if (v6 > 0)
        v2 = v6;
    else
        v2 = 0;
    to[3] = v2;
}

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

void __cdecl R_ConvertColorToBytes(const float *colorFloat, unsigned __int8 *colorBytes)
{
    if (colorFloat)
        Byte4PackVertexColor(colorFloat, colorBytes);
    else
        *(unsigned int *)colorBytes = -1;
}

int __cdecl R_PickMaterial(
    int traceMask,
    const float *org,
    const float *dir,
    char *name,
    char *surfaceFlags,
    char *contents,
    unsigned int charLimit)
{
    int v8; // ecx
    char v9; // [esp+4Fh] [ebp-55h]
    const char *material; // [esp+58h] [ebp-4Ch]
    float end[3]; // [esp+5Ch] [ebp-48h] BYREF
    trace_t trace; // [esp+68h] [ebp-3Ch] BYREF
    int contentsLen; // [esp+94h] [ebp-10h]
    int index; // [esp+98h] [ebp-Ch]
    int surfaceFlagsLen; // [esp+9Ch] [ebp-8h]
    int i; // [esp+A0h] [ebp-4h]

    Vec3Mad(org, 262144.0, dir, end);
    CM_BoxTrace(&trace, org, end, vec3_origin, vec3_origin, 0, traceMask);
    if (trace.startsolid || trace.allsolid || trace.fraction == 1.0 || !trace.material)
        return 0;
    material = trace.material;
    do
    {
        v9 = *material;
        *name++ = *material++;
    } while (v9);
    *surfaceFlags = 0;
    surfaceFlags[charLimit - 1] = 0;
    surfaceFlagsLen = 0;
    *contents = 0;
    contents[charLimit - 1] = 0;
    contentsLen = 0;
    v8 = (trace.surfaceFlags & 0x1F00000) >> 20;
    index = (unsigned __int8)v8;
    if ((_BYTE)v8 && index < 29)
        strncpy((unsigned __int8 *)surfaceFlags, (unsigned __int8 *)infoParms[index - 1].name, charLimit);
    else
        strncpy((unsigned __int8 *)surfaceFlags, "^1default^7", charLimit);
    if (surfaceFlags[charLimit - 1])
        return 0;
    surfaceFlagsLen = strlen(surfaceFlags);
    if ((trace.contents & 1) != 0)
        strncpy((unsigned __int8 *)contents, "solid", charLimit);
    else
        strncpy((unsigned __int8 *)contents, "^3nonsolid^7", charLimit);
    if (contents[charLimit - 1])
        return 0;
    contentsLen = strlen(contents);
    for (i = 28; infoParms[i].name; ++i)
    {
        if ((trace.surfaceFlags & MEMORY[0x94F4B0][5 * i]) != 0)
        {
            surfaceFlags[surfaceFlagsLen++] = 32;
            strncpy(
                (unsigned __int8 *)&surfaceFlags[surfaceFlagsLen],
                (unsigned __int8 *)infoParms[i].name,
                charLimit - surfaceFlagsLen);
            if (surfaceFlags[charLimit - 1])
                return 0;
            surfaceFlagsLen += strlen(&surfaceFlags[surfaceFlagsLen]);
        }
        if ((trace.contents & MEMORY[0x94F4B4][5 * i]) != 0)
        {
            contents[contentsLen++] = 32;
            strncpy((unsigned __int8 *)&contents[contentsLen], (unsigned __int8 *)infoParms[i].name, charLimit - contentsLen);
            if (contents[charLimit - 1])
                return 0;
            contentsLen += strlen(&contents[contentsLen]);
        }
    }
    return 1;
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
    v5 = fabs(cosIncidentAngle);
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


void __cdecl R_SetShadowLookupMatrix(GfxCmdBufSourceState *source, const GfxMatrix *matrix)
{
    ++source->matrixVersions[6];
    memcpy(&source->shadowLookupMatrix, matrix, sizeof(source->shadowLookupMatrix));
}

void __cdecl R_Set2D(GfxCmdBufSourceState *source)
{
    GfxViewport viewport; // [esp+4h] [ebp-10h] BYREF
    int savedregs; // [esp+14h] [ebp+0h] BYREF

    if (source->viewMode != VIEW_MODE_2D)
    {
        source->viewMode = VIEW_MODE_2D;
        source->viewportIsDirty = 1;
        source->eyeOffset[0] = 0.0;
        source->eyeOffset[1] = 0.0;
        source->eyeOffset[2] = 0.0;
        source->eyeOffset[3] = 1.0;
        R_GetViewport(source, &viewport);
        R_CmdBufSet2D((GfxViewParms *)&savedregs, source, &viewport);
    }
}

void __usercall R_CmdBufSet2D(GfxViewParms *a1@<ebp>, GfxCmdBufSourceState *source, GfxViewport *viewport)
{
    float v3[16]; // [esp-8h] [ebp-9Ch] BYREF
    GfxMatrix identity_52; // [esp+38h] [ebp-5Ch] BYREF
    GfxViewParms *transform_56; // [esp+7Ch] [ebp-18h]
    float transform_60; // [esp+80h] [ebp-14h]
    float v7; // [esp+84h] [ebp-10h]
    GfxViewParms *viewParms; // [esp+88h] [ebp-Ch]
    float invHeight; // [esp+8Ch] [ebp-8h]
    float retaddr; // [esp+94h] [ebp+0h]

    viewParms = a1;
    invHeight = retaddr;
    if (viewport->width <= 0)
        MyAssertHandler(
            ".\\r_state_utils.cpp",
            168,
            0,
            "%s\n\t(viewport->width) = %i",
            "(viewport->width > 0)",
            viewport->width);
    if (viewport->height <= 0)
        MyAssertHandler(
            ".\\r_state_utils.cpp",
            169,
            0,
            "%s\n\t(viewport->height) = %i",
            "(viewport->height > 0)",
            viewport->height);
    v7 = 1.0 / (double)viewport->width;
    transform_60 = 1.0 / (double)viewport->height;
    transform_56 = &source->viewParms;
    memset((unsigned __int8 *)&identity_52, 0, sizeof(identity_52));
    identity_52.m[0][0] = v7 * 2.0;
    identity_52.m[1][1] = transform_60 * -2.0;
    identity_52.m[3][0] = -1.0 - v7;
    identity_52.m[3][1] = transform_60 + 1.0;
    identity_52.m[3][2] = 1.0;
    identity_52.m[3][3] = 1.0;
    R_MatrixIdentity44((float (*)[4])v3);
    R_MatrixIdentity44(transform_56->viewMatrix.m);
    qmemcpy(&transform_56->projectionMatrix, &identity_52, sizeof(transform_56->projectionMatrix));
    qmemcpy(&transform_56->viewProjectionMatrix, &identity_52, sizeof(transform_56->viewProjectionMatrix));
    memset(
        (unsigned __int8 *)&transform_56->inverseViewProjectionMatrix,
        0,
        sizeof(transform_56->inverseViewProjectionMatrix));
    ++source->matrixVersions[1];
    ++source->matrixVersions[2];
    ++source->matrixVersions[4];
    qmemcpy(R_GetActiveWorldMatrix(source), v3, 0x40u);
}

GfxCmdBufSourceState *__cdecl R_GetActiveWorldMatrix(GfxCmdBufSourceState *source)
{
    R_WorldMatrixChanged(source);
    return source;
}