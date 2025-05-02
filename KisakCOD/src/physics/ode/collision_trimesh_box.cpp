/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001-2003 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/


/*************************************************************************
 *                                                                       *
 * Triangle-box collider by Alen Ladavac and Vedran Klanac.              *
 * Ported to ODE by Oskari Nyman.                                        *
 *                                                                       *
 *************************************************************************/

#include <ode/collision.h>
#include <ode/matrix.h>
#include <ode/rotation.h>
#include <ode/odemath.h>
#include "collision_util.h"

#define TRIMESH_INTERNAL
#include "collision_trimesh_internal.h"

// LWSS ADD 
#include "collision_trimesh_KISAK.h"

#include <Windows.h>
#include <universal/com_math.h>
#include <universal/q_shared.h>
#include "collision_kernel.h"

struct collData_t // sizeof=0xC4
{                                       // ...
    float mHullBoxRot[12];              // ...
    float vHullBoxPos[4];               // ...
    float vBoxHalfSize[4];              // ...
    float vBestNormal[4];               // ...
    float fBestDepth;                   // ...
    int iBestAxis;
    float triangleEdge0[4];
    float triangleEdge1[4];
    float triangleEdge2[4];
    float triangleNormal[4];
    int iFlags;                         // ...
    dContactGeom *ContactGeoms;         // ...
    int iStride;                        // ...
    dxGeom *Geom1;
    dxGeom *Geom2;
    int ctContacts;                     // ...
    const float *bodyCenter;            // ...
};

char __cdecl cldTestFace(
    collData_t *tbData,
    float fp0,
    float fp1,
    float fp2,
    float fR,
    float fD,
    float *vNormal,
    int iAxis)
{
    const char *v9; // eax
    float fMin; // [esp+1Ch] [ebp-14h]
    float fDepthMin; // [esp+20h] [ebp-10h]
    float fDepthMax; // [esp+24h] [ebp-Ch]
    float fDepth; // [esp+28h] [ebp-8h]
    float fMax; // [esp+2Ch] [ebp-4h]

    if (*vNormal == 0.0 && vNormal[1] == 0.0 && vNormal[2] == 0.0)
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            167,
            0,
            "%s",
            "vNormal[0] != 0 || vNormal[1] != 0 || vNormal[2] != 0");
    if (fp1 <= fp0)
    {
        if (fp2 <= fp1)
            fMin = fp2;
        else
            fMin = fp1;
    }
    else if (fp2 <= fp0)
    {
        fMin = fp2;
    }
    else
    {
        fMin = fp0;
    }
    if (fp1 >= fp0)
    {
        if (fp2 >= fp1)
            fMax = fp2;
        else
            fMax = fp1;
    }
    else if (fp2 >= fp0)
    {
        fMax = fp2;
    }
    else
    {
        fMax = fp0;
    }
    fDepthMin = fR - fMin;
    if (fDepthMin < 0.0)
        return 0;
    fDepthMax = fMax + fR;
    if (fDepthMax < 0.0)
        return 0;
    if (Vec3Dot(vNormal, tbData->triangleNormal) <= 0.0)
    {
        fDepth = fR - fMin;
    }
    else
    {
        fDepth = fMax + fR;
        *vNormal = -*vNormal;
        vNormal[1] = -vNormal[1];
        vNormal[2] = -vNormal[2];
    }
    if (tbData->fBestDepth > fDepth)
    {
        tbData->vBestNormal[0] = *vNormal;
        tbData->vBestNormal[1] = vNormal[1];
        tbData->vBestNormal[2] = vNormal[2];
        if (!Vec3IsNormalized(tbData->vBestNormal))
        {
            v9 = va("normal: (%f, %f, %f)", tbData->vBestNormal[0], tbData->vBestNormal[1], tbData->vBestNormal[2]);
            MyAssertHandler(
                ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
                231,
                0,
                "%s\n\t%s",
                "Vec3IsNormalized( tbData->vBestNormal )",
                v9);
        }
        tbData->iBestAxis = iAxis;
        tbData->fBestDepth = fDepth;
    }
    return 1;
}

char __cdecl cldTestNormal(collData_t *tbData, dReal fp0, dReal fR, float *vNormal, int iAxis)
{
    const char *v6; // eax
    float fOneOverLength; // [esp+34h] [ebp-Ch]
    float fDepth; // [esp+38h] [ebp-8h]
    float fDeptha; // [esp+38h] [ebp-8h]
    float fLength; // [esp+3Ch] [ebp-4h]

    if ((LODWORD(fp0) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\physics\\ode\\src\\collision_trimesh_box.cpp", 121, 0, "%s", "!IS_NAN(fp0)");
    if ((LODWORD(fR) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\physics\\ode\\src\\collision_trimesh_box.cpp", 122, 0, "%s", "!IS_NAN(fR)");
    if (!vNormal)
        MyAssertHandler(".\\physics\\ode\\src\\collision_trimesh_box.cpp", 123, 0, "%s", "vNormal");
    if ((COERCE_UNSIGNED_INT(*vNormal) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vNormal[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vNormal[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            124,
            0,
            "%s",
            "!IS_NAN((vNormal)[0]) && !IS_NAN((vNormal)[1]) && !IS_NAN((vNormal)[2])");
    }
    fDepth = fR + fp0;
    if (fDepth >= 3.402823466385289e38)
        MyAssertHandler(".\\physics\\ode\\src\\collision_trimesh_box.cpp", 128, 0, "%s", "fDepth < MAXVALUE");
    if (fDepth < 0.0)
        return 0;
    fLength = Vec3Length(vNormal);
    if (fLength <= 0.0)
        MyAssertHandler(".\\physics\\ode\\src\\collision_trimesh_box.cpp", 139, 0, "%s", "fLength > 0");
    fOneOverLength = 1.0 / fLength;
    fDeptha = fDepth * fOneOverLength;
    if (tbData->fBestDepth > fDeptha)
    {
        tbData->vBestNormal[0] = -*vNormal * fOneOverLength;
        tbData->vBestNormal[1] = -vNormal[1] * fOneOverLength;
        tbData->vBestNormal[2] = -vNormal[2] * fOneOverLength;
        if (!Vec3IsNormalized(tbData->vBestNormal))
        {
            v6 = va("normal: (%f, %f, %f)", tbData->vBestNormal[0], tbData->vBestNormal[1], tbData->vBestNormal[2]);
            MyAssertHandler(
                ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
                151,
                0,
                "%s\n\t%s",
                "Vec3IsNormalized( tbData->vBestNormal )",
                v6);
        }
        tbData->iBestAxis = iAxis;
        tbData->fBestDepth = fDeptha;
    }
    return 1;
}

char __cdecl cldTestEdge(collData_t *tbData, float fp0, float fp1, float fR, float fD, float *vNormal, int iAxis)
{
    const char *v8; // eax
    float fOneOverLength; // [esp+20h] [ebp-1Ch]
    float fMin; // [esp+24h] [ebp-18h]
    float fDepthMin; // [esp+28h] [ebp-14h]
    float fDepthMax; // [esp+2Ch] [ebp-10h]
    float fDepth; // [esp+30h] [ebp-Ch]
    float fDeptha; // [esp+30h] [ebp-Ch]
    float fLength; // [esp+34h] [ebp-8h]
    float fMax; // [esp+38h] [ebp-4h]

    if (fp1 <= fp0)
    {
        fMin = fp1;
        fMax = fp0;
    }
    else
    {
        fMin = fp0;
        fMax = fp1;
    }
    fDepthMin = fR - fMin;
    fDepthMax = fMax + fR;
    if (fDepthMin < 0.0 || fDepthMax < 0.0)
        return 0;
    if (fDepthMax >= fDepthMin)
    {
        fDepth = fR - fMin;
    }
    else
    {
        fDepth = fMax + fR;
        *vNormal = -*vNormal;
        vNormal[1] = -vNormal[1];
        vNormal[2] = -vNormal[2];
    }
    fLength = Vec3Length(vNormal);
    if (fLength > 0.0)
    {
        fOneOverLength = 1.0 / fLength;
        fDeptha = fDepth * fOneOverLength;
        if (tbData->fBestDepth > fDeptha * 1.5)
        {
            tbData->vBestNormal[0] = *vNormal * fOneOverLength;
            tbData->vBestNormal[1] = vNormal[1] * fOneOverLength;
            tbData->vBestNormal[2] = vNormal[2] * fOneOverLength;
            if (!Vec3IsNormalized(tbData->vBestNormal))
            {
                v8 = va("normal: (%f, %f, %f)", tbData->vBestNormal[0], tbData->vBestNormal[1], tbData->vBestNormal[2]);
                MyAssertHandler(
                    ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
                    303,
                    0,
                    "%s\n\t%s",
                    "Vec3IsNormalized( tbData->vBestNormal )",
                    v8);
            }
            tbData->iBestAxis = iAxis;
            tbData->fBestDepth = fDeptha;
        }
    }
    return 1;
}

char __cdecl cldTestSeparatingAxes(collData_t *tbData, const dVector3 *v0, const dVector3 *v1, const dVector3 *v2)
{
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    double v9; // st7
    double v10; // st7
    const char *v11; // eax
    double v12; // st7
    double v13; // st7
    const char *v14; // eax
    double v15; // st7
    double v16; // st7
    const char *v17; // eax
    double v18; // st7
    const char *v19; // eax
    double v20; // st7
    const char *v21; // eax
    double v22; // st7
    double v23; // st7
    const char *v24; // eax
    double v25; // st7
    const char *v26; // eax
    double v27; // st7
    const char *v28; // eax
    double v29; // st7
    double v30; // st7
    const char *v31; // eax
    double v32; // st7
    const char *v33; // eax
    double v34; // st7
    const char *v35; // eax
    double v36; // st7
    double v37; // st7
    const char *v38; // eax
    float v39; // [esp+1Ch] [ebp-1E8h]
    float v40; // [esp+20h] [ebp-1E4h]
    float v41; // [esp+24h] [ebp-1E0h]
    float v42; // [esp+28h] [ebp-1DCh]
    float v43; // [esp+2Ch] [ebp-1D8h]
    float v44; // [esp+30h] [ebp-1D4h]
    float v45; // [esp+34h] [ebp-1D0h]
    float v46; // [esp+38h] [ebp-1CCh]
    float v47; // [esp+3Ch] [ebp-1C8h]
    float v48; // [esp+40h] [ebp-1C4h]
    float v49; // [esp+44h] [ebp-1C0h]
    float v50; // [esp+48h] [ebp-1BCh]
    float v51; // [esp+4Ch] [ebp-1B8h]
    float v52; // [esp+50h] [ebp-1B4h]
    float v53; // [esp+54h] [ebp-1B0h]
    float v54; // [esp+58h] [ebp-1ACh]
    float v55; // [esp+5Ch] [ebp-1A8h]
    float v56; // [esp+60h] [ebp-1A4h]
    float v57; // [esp+64h] [ebp-1A0h]
    float v58; // [esp+68h] [ebp-19Ch]
    float v59; // [esp+6Ch] [ebp-198h]
    double v60; // [esp+70h] [ebp-194h]
    float v61; // [esp+80h] [ebp-184h]
    float v62; // [esp+88h] [ebp-17Ch]
    float v63; // [esp+94h] [ebp-170h]
    float v64; // [esp+9Ch] [ebp-168h]
    float v65; // [esp+A8h] [ebp-15Ch]
    float v66; // [esp+B0h] [ebp-154h]
    float v67; // [esp+BCh] [ebp-148h]
    float v68; // [esp+C4h] [ebp-140h]
    float v69; // [esp+D0h] [ebp-134h]
    float v70; // [esp+D8h] [ebp-12Ch]
    float v71; // [esp+E4h] [ebp-120h]
    float v72; // [esp+ECh] [ebp-118h]
    float v73; // [esp+F8h] [ebp-10Ch]
    float v74; // [esp+100h] [ebp-104h]
    float v75; // [esp+10Ch] [ebp-F8h]
    float v76; // [esp+114h] [ebp-F0h]
    float v77; // [esp+120h] [ebp-E4h]
    float v78; // [esp+128h] [ebp-DCh]
    float v79; // [esp+140h] [ebp-C4h]
    float v80; // [esp+148h] [ebp-BCh]
    float v81; // [esp+150h] [ebp-B4h]
    float boxAxis0[4]; // [esp+198h] [ebp-6Ch] BYREF
    float fp0; // [esp+1A8h] [ebp-5Ch]
    float boxAxis1[4]; // [esp+1ACh] [ebp-58h] BYREF
    float fR; // [esp+1BCh] [ebp-48h]
    float fD; // [esp+1C0h] [ebp-44h]
    float triNormalMag; // [esp+1C4h] [ebp-40h]
    float boxAxis2[4]; // [esp+1C8h] [ebp-3Ch] BYREF
    float tempNormal[4]; // [esp+1D8h] [ebp-2Ch] BYREF
    float fp1; // [esp+1E8h] [ebp-1Ch]
    float bodyDist; // [esp+1ECh] [ebp-18h]
    float vecToV0[4]; // [esp+1F0h] [ebp-14h] BYREF
    float fp2; // [esp+200h] [ebp-4h]

    if (!v0)
        MyAssertHandler(".\\physics\\ode\\src\\collision_trimesh_box.cpp", 371, 0, "%s", "&v0");
    if (!v1)
        MyAssertHandler(".\\physics\\ode\\src\\collision_trimesh_box.cpp", 372, 0, "%s", "&v1");
    if (!v2)
        MyAssertHandler(".\\physics\\ode\\src\\collision_trimesh_box.cpp", 373, 0, "%s", "&v2");
    if ((COERCE_UNSIGNED_INT((*v0)[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((*v0)[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((*v0)[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            374,
            0,
            "%s",
            "!IS_NAN((v0)[0]) && !IS_NAN((v0)[1]) && !IS_NAN((v0)[2])");
    }
    if ((COERCE_UNSIGNED_INT((*v1)[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((*v1)[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((*v1)[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            375,
            0,
            "%s",
            "!IS_NAN((v1)[0]) && !IS_NAN((v1)[1]) && !IS_NAN((v1)[2])");
    }
    if ((COERCE_UNSIGNED_INT((*v2)[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((*v2)[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((*v2)[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            376,
            0,
            "%s",
            "!IS_NAN((v2)[0]) && !IS_NAN((v2)[1]) && !IS_NAN((v2)[2])");
    }
    tbData->iBestAxis = 0;
    tbData->fBestDepth = 3.4028235e38;
    tbData->vBestNormal[0] = 0.0;
    tbData->vBestNormal[1] = 0.0;
    tbData->vBestNormal[2] = 0.0;
    Vec3Sub((const float*)v1, (const float *)v0, tbData->triangleEdge0);
    if (Vec3LengthSq(tbData->triangleEdge0) < 0.009999999776482582)
        return 0;
    Vec3Sub((const float *)v2, (const float *)v0, tbData->triangleEdge1);
    if (Vec3LengthSq(tbData->triangleEdge1) < 0.009999999776482582)
        return 0;
    Vec3Sub(tbData->triangleEdge1, tbData->triangleEdge0, tbData->triangleEdge2);
    if (Vec3LengthSq(tbData->triangleEdge2) < 0.009999999776482582)
        return 0;
    tbData->triangleNormal[0] = tbData->triangleEdge0[1] * tbData->triangleEdge1[2]
        - tbData->triangleEdge0[2] * tbData->triangleEdge1[1];
    tbData->triangleNormal[1] = tbData->triangleEdge0[2] * tbData->triangleEdge1[0]
        - tbData->triangleEdge0[0] * tbData->triangleEdge1[2];
    tbData->triangleNormal[2] = tbData->triangleEdge0[0] * tbData->triangleEdge1[1]
        - tbData->triangleEdge0[1] * tbData->triangleEdge1[0];
    triNormalMag = Vec3Normalize(tbData->triangleNormal);
    if (triNormalMag < 0.009999999776482582)
        return 0;
    boxAxis0[0] = tbData->mHullBoxRot[0];
    boxAxis0[1] = tbData->mHullBoxRot[4];
    boxAxis0[2] = tbData->mHullBoxRot[8];
    boxAxis1[0] = tbData->mHullBoxRot[1];
    boxAxis1[1] = tbData->mHullBoxRot[5];
    boxAxis1[2] = tbData->mHullBoxRot[9];
    boxAxis2[0] = tbData->mHullBoxRot[2];
    boxAxis2[1] = tbData->mHullBoxRot[6];
    boxAxis2[2] = tbData->mHullBoxRot[10];
    if (!Vec3IsNormalized(boxAxis0))
    {
        v5 = va("normal: (%f, %f, %f)", boxAxis0[0], boxAxis0[1], boxAxis0[2]);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            404,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( boxAxis0 )",
            v5);
    }
    if (!Vec3IsNormalized(boxAxis1))
    {
        v6 = va("normal: (%f, %f, %f)", boxAxis1[0], boxAxis1[1], boxAxis1[2]);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            405,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( boxAxis1 )",
            v6);
    }
    if (!Vec3IsNormalized(boxAxis2))
    {
        v7 = va("normal: (%f, %f, %f)", boxAxis2[0], boxAxis2[1], boxAxis2[2]);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            406,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( boxAxis2 )",
            v7);
    }
    Vec3Sub((const float*)v0, tbData->vHullBoxPos, vecToV0);
    v60 = Vec3Dot(tbData->triangleNormal, tbData->bodyCenter);
    bodyDist = v60 - Vec3Dot(tbData->triangleNormal, (const float*)v0);
    if (Vec3Dot(vecToV0, tbData->triangleNormal) >= 0.0 && bodyDist < 0.0)
        return 0;
    tempNormal[0] = tbData->triangleNormal[0];
    tempNormal[1] = tbData->triangleNormal[1];
    tempNormal[2] = tbData->triangleNormal[2];
    fp0 = Vec3Dot(tempNormal, vecToV0);
    fp1 = fp0;
    fp2 = fp0;
    v81 = Vec3Dot(tbData->triangleNormal, boxAxis0);
    v59 = fabs(v81);
    v80 = Vec3Dot(tbData->triangleNormal, boxAxis1);
    v58 = fabs(v80);
    v79 = Vec3Dot(tbData->triangleNormal, boxAxis2);
    v57 = fabs(v79);
    fR = v59 * tbData->vBoxHalfSize[0] + v58 * tbData->vBoxHalfSize[1] + v57 * tbData->vBoxHalfSize[2];
    if (!cldTestNormal(tbData, fp0, fR, tempNormal, 1))
        return 0;
    if (!Vec3IsNormalized(tbData->vBestNormal))
    {
        v8 = va(
            "normal: (%f, %f, %f) bestaxis: %i",
            tbData->vBestNormal[0],
            tbData->vBestNormal[1],
            tbData->vBestNormal[2],
            tbData->iBestAxis);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            430,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( tbData->vBestNormal )",
            v8);
    }
    tempNormal[0] = boxAxis0[0];
    tempNormal[1] = boxAxis0[1];
    tempNormal[2] = boxAxis0[2];
    fD = Vec3Dot(tempNormal, tbData->triangleNormal);
    fp0 = Vec3Dot(tempNormal, vecToV0);
    v9 = Vec3Dot(boxAxis0, tbData->triangleEdge0);
    fp1 = v9 + fp0;
    v10 = Vec3Dot(boxAxis0, tbData->triangleEdge1);
    fp2 = v10 + fp0;
    fR = tbData->vBoxHalfSize[0];
    if (!cldTestFace(tbData, fp0, fp1, fp2, fR, fD, tempNormal, 2))
        return 0;
    if (!Vec3IsNormalized(tbData->vBestNormal))
    {
        v11 = va(
            "normal: (%f, %f, %f) bestaxis: %i",
            tbData->vBestNormal[0],
            tbData->vBestNormal[1],
            tbData->vBestNormal[2],
            tbData->iBestAxis);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            449,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( tbData->vBestNormal )",
            v11);
    }
    tempNormal[0] = boxAxis1[0];
    tempNormal[1] = boxAxis1[1];
    tempNormal[2] = boxAxis1[2];
    fD = Vec3Dot(tempNormal, tbData->triangleNormal);
    fp0 = Vec3Dot(tempNormal, vecToV0);
    v12 = Vec3Dot(boxAxis1, tbData->triangleEdge0);
    fp1 = v12 + fp0;
    v13 = Vec3Dot(boxAxis1, tbData->triangleEdge1);
    fp2 = v13 + fp0;
    fR = tbData->vBoxHalfSize[1];
    if (!cldTestFace(tbData, fp0, fp1, fp2, fR, fD, tempNormal, 3))
        return 0;
    if (!Vec3IsNormalized(tbData->vBestNormal))
    {
        v14 = va(
            "normal: (%f, %f, %f) bestaxis: %i",
            tbData->vBestNormal[0],
            tbData->vBestNormal[1],
            tbData->vBestNormal[2],
            tbData->iBestAxis);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            467,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( tbData->vBestNormal )",
            v14);
    }
    tempNormal[0] = boxAxis2[0];
    tempNormal[1] = boxAxis2[1];
    tempNormal[2] = boxAxis2[2];
    fD = Vec3Dot(tempNormal, tbData->triangleNormal);
    fp0 = Vec3Dot(tempNormal, vecToV0);
    v15 = Vec3Dot(boxAxis2, tbData->triangleEdge0);
    fp1 = v15 + fp0;
    v16 = Vec3Dot(boxAxis2, tbData->triangleEdge1);
    fp2 = v16 + fp0;
    fR = tbData->vBoxHalfSize[2];
    if (!cldTestFace(tbData, fp0, fp1, fp2, fR, fD, tempNormal, 4))
        return 0;
    if (!Vec3IsNormalized(tbData->vBestNormal))
    {
        v17 = va(
            "normal: (%f, %f, %f) bestaxis: %i",
            tbData->vBestNormal[0],
            tbData->vBestNormal[1],
            tbData->vBestNormal[2],
            tbData->iBestAxis);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            485,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( tbData->vBestNormal )",
            v17);
    }
    tempNormal[0] = boxAxis0[1] * tbData->triangleEdge0[2] - boxAxis0[2] * tbData->triangleEdge0[1];
    tempNormal[1] = boxAxis0[2] * tbData->triangleEdge0[0] - boxAxis0[0] * tbData->triangleEdge0[2];
    tempNormal[2] = boxAxis0[0] * tbData->triangleEdge0[1] - boxAxis0[1] * tbData->triangleEdge0[0];
    fD = Vec3Dot(tempNormal, tbData->triangleNormal);
    fp0 = Vec3Dot(tempNormal, vecToV0);
    fp1 = fp0;
    v18 = Vec3Dot(boxAxis0, tbData->triangleNormal);
    fp2 = v18 * triNormalMag + fp0;
    v78 = Vec3Dot(boxAxis2, tbData->triangleEdge0);
    v56 = fabs(v78);
    v77 = Vec3Dot(boxAxis1, tbData->triangleEdge0);
    v55 = fabs(v77);
    fR = v56 * tbData->vBoxHalfSize[1] + v55 * tbData->vBoxHalfSize[2];
    if (!cldTestEdge(tbData, fp1, fp2, fR, fD, tempNormal, 5))
        return 0;
    if (!Vec3IsNormalized(tbData->vBestNormal))
    {
        v19 = va(
            "normal: (%f, %f, %f) bestaxis: %i",
            tbData->vBestNormal[0],
            tbData->vBestNormal[1],
            tbData->vBestNormal[2],
            tbData->iBestAxis);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            504,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( tbData->vBestNormal )",
            v19);
    }
    tempNormal[0] = boxAxis0[1] * tbData->triangleEdge1[2] - boxAxis0[2] * tbData->triangleEdge1[1];
    tempNormal[1] = boxAxis0[2] * tbData->triangleEdge1[0] - boxAxis0[0] * tbData->triangleEdge1[2];
    tempNormal[2] = boxAxis0[0] * tbData->triangleEdge1[1] - boxAxis0[1] * tbData->triangleEdge1[0];
    fD = Vec3Dot(tempNormal, tbData->triangleNormal);
    fp0 = Vec3Dot(tempNormal, vecToV0);
    v20 = Vec3Dot(boxAxis0, tbData->triangleNormal);
    fp1 = fp0 - v20 * triNormalMag;
    fp2 = fp0;
    v76 = Vec3Dot(boxAxis2, tbData->triangleEdge1);
    v54 = fabs(v76);
    v75 = Vec3Dot(boxAxis1, tbData->triangleEdge1);
    v53 = fabs(v75);
    fR = v54 * tbData->vBoxHalfSize[1] + v53 * tbData->vBoxHalfSize[2];
    if (!cldTestEdge(tbData, fp0, fp1, fR, fD, tempNormal, 6))
        return 0;
    if (!Vec3IsNormalized(tbData->vBestNormal))
    {
        v21 = va(
            "normal: (%f, %f, %f) bestaxis: %i",
            tbData->vBestNormal[0],
            tbData->vBestNormal[1],
            tbData->vBestNormal[2],
            tbData->iBestAxis);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            521,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( tbData->vBestNormal )",
            v21);
    }
    tempNormal[0] = boxAxis0[1] * tbData->triangleEdge2[2] - boxAxis0[2] * tbData->triangleEdge2[1];
    tempNormal[1] = boxAxis0[2] * tbData->triangleEdge2[0] - boxAxis0[0] * tbData->triangleEdge2[2];
    tempNormal[2] = boxAxis0[0] * tbData->triangleEdge2[1] - boxAxis0[1] * tbData->triangleEdge2[0];
    fD = Vec3Dot(tempNormal, tbData->triangleNormal);
    fp0 = Vec3Dot(tempNormal, vecToV0);
    v22 = Vec3Dot(boxAxis0, tbData->triangleNormal);
    fp1 = fp0 - v22 * triNormalMag;
    v23 = Vec3Dot(boxAxis0, tbData->triangleNormal);
    fp2 = fp0 - v23 * triNormalMag;
    v74 = Vec3Dot(boxAxis2, tbData->triangleEdge2);
    v52 = fabs(v74);
    v73 = Vec3Dot(boxAxis1, tbData->triangleEdge2);
    v51 = fabs(v73);
    fR = v52 * tbData->vBoxHalfSize[1] + v51 * tbData->vBoxHalfSize[2];
    if (!cldTestEdge(tbData, fp0, fp1, fR, fD, tempNormal, 7))
        return 0;
    if (!Vec3IsNormalized(tbData->vBestNormal))
    {
        v24 = va(
            "normal: (%f, %f, %f) bestaxis: %i",
            tbData->vBestNormal[0],
            tbData->vBestNormal[1],
            tbData->vBestNormal[2],
            tbData->iBestAxis);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            538,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( tbData->vBestNormal )",
            v24);
    }
    tempNormal[0] = boxAxis1[1] * tbData->triangleEdge0[2] - boxAxis1[2] * tbData->triangleEdge0[1];
    tempNormal[1] = boxAxis1[2] * tbData->triangleEdge0[0] - boxAxis1[0] * tbData->triangleEdge0[2];
    tempNormal[2] = boxAxis1[0] * tbData->triangleEdge0[1] - boxAxis1[1] * tbData->triangleEdge0[0];
    fD = Vec3Dot(tempNormal, tbData->triangleNormal);
    fp0 = Vec3Dot(tempNormal, vecToV0);
    fp1 = fp0;
    v25 = Vec3Dot(boxAxis1, tbData->triangleNormal);
    fp2 = v25 * triNormalMag + fp0;
    v72 = Vec3Dot(boxAxis2, tbData->triangleEdge0);
    v50 = fabs(v72);
    v71 = Vec3Dot(boxAxis0, tbData->triangleEdge0);
    v49 = fabs(v71);
    fR = v50 * tbData->vBoxHalfSize[0] + v49 * tbData->vBoxHalfSize[2];
    if (!cldTestEdge(tbData, fp0, fp2, fR, fD, tempNormal, 8))
        return 0;
    if (!Vec3IsNormalized(tbData->vBestNormal))
    {
        v26 = va(
            "normal: (%f, %f, %f) bestaxis: %i",
            tbData->vBestNormal[0],
            tbData->vBestNormal[1],
            tbData->vBestNormal[2],
            tbData->iBestAxis);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            556,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( tbData->vBestNormal )",
            v26);
    }
    tempNormal[0] = boxAxis1[1] * tbData->triangleEdge1[2] - boxAxis1[2] * tbData->triangleEdge1[1];
    tempNormal[1] = boxAxis1[2] * tbData->triangleEdge1[0] - boxAxis1[0] * tbData->triangleEdge1[2];
    tempNormal[2] = boxAxis1[0] * tbData->triangleEdge1[1] - boxAxis1[1] * tbData->triangleEdge1[0];
    fD = Vec3Dot(tempNormal, tbData->triangleNormal);
    fp0 = Vec3Dot(tempNormal, vecToV0);
    v27 = Vec3Dot(boxAxis1, tbData->triangleNormal);
    fp1 = fp0 - v27 * triNormalMag;
    fp2 = fp0;
    v70 = Vec3Dot(boxAxis2, tbData->triangleEdge1);
    v48 = fabs(v70);
    v69 = Vec3Dot(boxAxis0, tbData->triangleEdge1);
    v47 = fabs(v69);
    fR = v48 * tbData->vBoxHalfSize[0] + v47 * tbData->vBoxHalfSize[2];
    if (!cldTestEdge(tbData, fp0, fp1, fR, fD, tempNormal, 9))
        return 0;
    if (!Vec3IsNormalized(tbData->vBestNormal))
    {
        v28 = va(
            "normal: (%f, %f, %f) bestaxis: %i",
            tbData->vBestNormal[0],
            tbData->vBestNormal[1],
            tbData->vBestNormal[2],
            tbData->iBestAxis);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            574,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( tbData->vBestNormal )",
            v28);
    }
    tempNormal[0] = boxAxis1[1] * tbData->triangleEdge2[2] - boxAxis1[2] * tbData->triangleEdge2[1];
    tempNormal[1] = boxAxis1[2] * tbData->triangleEdge2[0] - boxAxis1[0] * tbData->triangleEdge2[2];
    tempNormal[2] = boxAxis1[0] * tbData->triangleEdge2[1] - boxAxis1[1] * tbData->triangleEdge2[0];
    fD = Vec3Dot(tempNormal, tbData->triangleNormal);
    fp0 = Vec3Dot(tempNormal, vecToV0);
    v29 = Vec3Dot(boxAxis1, tbData->triangleNormal);
    fp1 = fp0 - v29 * triNormalMag;
    v30 = Vec3Dot(boxAxis1, tbData->triangleNormal);
    fp2 = fp0 - v30 * triNormalMag;
    v68 = Vec3Dot(boxAxis2, tbData->triangleEdge2);
    v46 = fabs(v68);
    v67 = Vec3Dot(boxAxis0, tbData->triangleEdge2);
    v45 = fabs(v67);
    fR = v46 * tbData->vBoxHalfSize[0] + v45 * tbData->vBoxHalfSize[2];
    if (!cldTestEdge(tbData, fp0, fp1, fR, fD, tempNormal, 10))
        return 0;
    if (!Vec3IsNormalized(tbData->vBestNormal))
    {
        v31 = va(
            "normal: (%f, %f, %f) bestaxis: %i",
            tbData->vBestNormal[0],
            tbData->vBestNormal[1],
            tbData->vBestNormal[2],
            tbData->iBestAxis);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            592,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( tbData->vBestNormal )",
            v31);
    }
    tempNormal[0] = boxAxis2[1] * tbData->triangleEdge0[2] - boxAxis2[2] * tbData->triangleEdge0[1];
    tempNormal[1] = boxAxis2[2] * tbData->triangleEdge0[0] - boxAxis2[0] * tbData->triangleEdge0[2];
    tempNormal[2] = boxAxis2[0] * tbData->triangleEdge0[1] - boxAxis2[1] * tbData->triangleEdge0[0];
    fD = Vec3Dot(tempNormal, tbData->triangleNormal);
    fp0 = Vec3Dot(tempNormal, vecToV0);
    fp1 = fp0;
    v32 = Vec3Dot(boxAxis2, tbData->triangleNormal);
    fp2 = v32 * triNormalMag + fp0;
    v66 = Vec3Dot(boxAxis1, tbData->triangleEdge0);
    v44 = fabs(v66);
    v65 = Vec3Dot(boxAxis0, tbData->triangleEdge0);
    v43 = fabs(v65);
    fR = v44 * tbData->vBoxHalfSize[0] + v43 * tbData->vBoxHalfSize[1];
    if (!cldTestEdge(tbData, fp0, fp2, fR, fD, tempNormal, 11))
        return 0;
    if (!Vec3IsNormalized(tbData->vBestNormal))
    {
        v33 = va(
            "normal: (%f, %f, %f) bestaxis: %i",
            tbData->vBestNormal[0],
            tbData->vBestNormal[1],
            tbData->vBestNormal[2],
            tbData->iBestAxis);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            610,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( tbData->vBestNormal )",
            v33);
    }
    tempNormal[0] = boxAxis2[1] * tbData->triangleEdge1[2] - boxAxis2[2] * tbData->triangleEdge1[1];
    tempNormal[1] = boxAxis2[2] * tbData->triangleEdge1[0] - boxAxis2[0] * tbData->triangleEdge1[2];
    tempNormal[2] = boxAxis2[0] * tbData->triangleEdge1[1] - boxAxis2[1] * tbData->triangleEdge1[0];
    fD = Vec3Dot(tempNormal, tbData->triangleNormal);
    fp0 = Vec3Dot(tempNormal, vecToV0);
    v34 = Vec3Dot(boxAxis2, tbData->triangleNormal);
    fp1 = fp0 - v34 * triNormalMag;
    fp2 = fp0;
    v64 = Vec3Dot(boxAxis1, tbData->triangleEdge1);
    v42 = fabs(v64);
    v63 = Vec3Dot(boxAxis0, tbData->triangleEdge1);
    v41 = fabs(v63);
    fR = v42 * tbData->vBoxHalfSize[0] + v41 * tbData->vBoxHalfSize[1];
    if (!cldTestEdge(tbData, fp0, fp1, fR, fD, tempNormal, 12))
        return 0;
    if (!Vec3IsNormalized(tbData->vBestNormal))
    {
        v35 = va(
            "normal: (%f, %f, %f) bestaxis: %i",
            tbData->vBestNormal[0],
            tbData->vBestNormal[1],
            tbData->vBestNormal[2],
            tbData->iBestAxis);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            628,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( tbData->vBestNormal )",
            v35);
    }
    tempNormal[0] = boxAxis2[1] * tbData->triangleEdge2[2] - boxAxis2[2] * tbData->triangleEdge2[1];
    tempNormal[1] = boxAxis2[2] * tbData->triangleEdge2[0] - boxAxis2[0] * tbData->triangleEdge2[2];
    tempNormal[2] = boxAxis2[0] * tbData->triangleEdge2[1] - boxAxis2[1] * tbData->triangleEdge2[0];
    fD = Vec3Dot(tempNormal, tbData->triangleNormal);
    fp0 = Vec3Dot(tempNormal, vecToV0);
    v36 = Vec3Dot(boxAxis2, tbData->triangleNormal);
    fp1 = fp0 - v36 * triNormalMag;
    v37 = Vec3Dot(boxAxis2, tbData->triangleNormal);
    fp2 = fp0 - v37 * triNormalMag;
    v62 = Vec3Dot(boxAxis1, tbData->triangleEdge2);
    v40 = fabs(v62);
    v61 = Vec3Dot(boxAxis0, tbData->triangleEdge2);
    v39 = fabs(v61);
    fR = v40 * tbData->vBoxHalfSize[0] + v39 * tbData->vBoxHalfSize[1];
    if (!cldTestEdge(tbData, fp0, fp1, fR, fD, tempNormal, 13))
        return 0;
    if (!Vec3IsNormalized(tbData->vBestNormal))
    {
        v38 = va(
            "normal: (%f, %f, %f) bestaxis: %i",
            tbData->vBestNormal[0],
            tbData->vBestNormal[1],
            tbData->vBestNormal[2],
            tbData->iBestAxis);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            646,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( tbData->vBestNormal )",
            v38);
    }
    return 1;
}

void __cdecl cldClipPolyToPlane(
    collData_t *tbData,
    dVector3 avArrayIn[],
    int ctIn,
    dVector3 avArrayOut[],
    int *ctOut,
    const dVector4 *plPlane)
{
    float vIntersectionPoint; // [esp+4h] [ebp-20h]
    float vIntersectionPoint_4; // [esp+8h] [ebp-1Ch]
    float vIntersectionPoint_8; // [esp+Ch] [ebp-18h]
    float fDistance0; // [esp+14h] [ebp-10h]
    float fDistance1; // [esp+18h] [ebp-Ch]
    int i1; // [esp+1Ch] [ebp-8h]
    int i0; // [esp+20h] [ebp-4h]

    *ctOut = 0;
    i0 = ctIn - 1;
    for (i1 = 0; i1 < ctIn; ++i1)
    {
        fDistance0 = (*plPlane)[0] * (*avArrayIn)[4 * i0]
            + (*plPlane)[1] * (*avArrayIn)[4 * i0 + 1]
            + (*plPlane)[2] * (*avArrayIn)[4 * i0 + 2]
            + (*plPlane)[3];
        fDistance1 = (*plPlane)[0] * (*avArrayIn)[4 * i1]
            + (*plPlane)[1] * (*avArrayIn)[4 * i1 + 1]
            + (*plPlane)[2] * (*avArrayIn)[4 * i1 + 2]
            + (*plPlane)[3];
        if (fDistance0 >= 0.0)
        {
            (*avArrayOut)[4 * *ctOut] = (*avArrayIn)[4 * i0];
            (*avArrayOut)[4 * *ctOut + 1] = (*avArrayIn)[4 * i0 + 1];
            (*avArrayOut)[4 * (*ctOut)++ + 2] = (*avArrayIn)[4 * i0 + 2];
        }
        if (fDistance0 > 0.0 && fDistance1 < 0.0 || fDistance0 < 0.0 && fDistance1 > 0.0)
        {
            vIntersectionPoint = (*avArrayIn)[4 * i0]
                - ((*avArrayIn)[4 * i0] - (*avArrayIn)[4 * i1]) * fDistance0 / (fDistance0 - fDistance1);
            vIntersectionPoint_4 = (*avArrayIn)[4 * i0 + 1]
                - ((*avArrayIn)[4 * i0 + 1] - (*avArrayIn)[4 * i1 + 1])
                * fDistance0
                / (fDistance0 - fDistance1);
            vIntersectionPoint_8 = (*avArrayIn)[4 * i0 + 2]
                - ((*avArrayIn)[4 * i0 + 2] - (*avArrayIn)[4 * i1 + 2])
                * fDistance0
                / (fDistance0 - fDistance1);
            (*avArrayOut)[4 * *ctOut] = vIntersectionPoint;
            (*avArrayOut)[4 * *ctOut + 1] = vIntersectionPoint_4;
            (*avArrayOut)[4 * (*ctOut)++ + 2] = vIntersectionPoint_8;
        }
        i0 = i1;
    }
}

char __cdecl cldClosestPointOnTwoLines(
    float *vPoint1,
    float *vLenVec1,
    float *vPoint2,
    float *vLenVec2,
    float *fvalue1,
    float *fvalue2)
{
    float fd; // [esp+0h] [ebp-20h]
    float fda; // [esp+0h] [ebp-20h]
    float vp[4]; // [esp+4h] [ebp-1Ch] BYREF
    float fq2; // [esp+14h] [ebp-Ch]
    float fuaub; // [esp+18h] [ebp-8h]
    float fq1; // [esp+1Ch] [ebp-4h]

    Vec3Sub(vPoint2, vPoint1, vp);
    fuaub = Vec3Dot(vLenVec1, vLenVec2);
    fq1 = Vec3Dot(vLenVec1, vp);
    fq2 = -Vec3Dot(vLenVec2, vp);
    fd = 1.0 - fuaub * fuaub;
    if (fd <= 0.0)
    {
        *fvalue1 = 0.0;
        *fvalue2 = 0.0;
        return 0;
    }
    else
    {
        fda = 1.0 / fd;
        *fvalue1 = (fuaub * fq2 + fq1) * fda;
        *fvalue2 = (fuaub * fq1 + fq2) * fda;
        return 1;
    }
}

void __cdecl cldClipping(collData_t *tbData, const dVector3 *v0, const dVector3 *v1, const dVector3 *v2)
{
    const char *v4; // eax
    double v5; // st7
    const char *v6; // eax
    const char *v7; // eax
    double v8; // st7
    const char *v9; // eax
    const char *v10; // eax
    float v11; // [esp+18h] [ebp-574h]
    float v12; // [esp+1Ch] [ebp-570h]
    float v13; // [esp+20h] [ebp-56Ch]
    float v14; // [esp+24h] [ebp-568h]
    int v15; // [esp+50h] [ebp-53Ch]
    unsigned int v16; // [esp+54h] [ebp-538h]
    dContactGeom *v17; // [esp+58h] [ebp-534h]
    int v18; // [esp+5Ch] [ebp-530h]
    char *v19; // [esp+94h] [ebp-4F8h]
    int iStride; // [esp+9Ch] [ebp-4F0h]
    unsigned int ctContacts; // [esp+A0h] [ebp-4ECh]
    dContactGeom *ContactGeoms; // [esp+A4h] [ebp-4E8h]
    int iFlags; // [esp+A8h] [ebp-4E4h]
    float *normal; // [esp+D0h] [ebp-4BCh]
    int v25; // [esp+D8h] [ebp-4B4h]
    unsigned int v26; // [esp+DCh] [ebp-4B0h]
    dContactGeom *v27; // [esp+E0h] [ebp-4ACh]
    int v28; // [esp+E4h] [ebp-4A8h]
    float *v29; // [esp+F8h] [ebp-494h]
    float v30; // [esp+FCh] [ebp-490h]
    float sum[4]; // [esp+100h] [ebp-48Ch] BYREF
    int m; // [esp+110h] [ebp-47Ch]
    int i_1; // [esp+114h] [ebp-478h]
    int x; // [esp+118h] [ebp-474h]
    float mTransposed[12]; // [esp+11Ch] [ebp-470h]
    float vAbsNormal[4]; // [esp+14Ch] [ebp-440h]
    float a[4]; // [esp+15Ch] [ebp-430h] BYREF
    dVector3 avArrayOut[9]; // [esp+16Ch] [ebp-420h] BYREF
    int iTempCnt2; // [esp+200h] [ebp-38Ch] BYREF
    dVector4 plPlane; // [esp+204h] [ebp-388h] BYREF
    float v41[4]; // [esp+214h] [ebp-378h] BYREF
    int iB2; // [esp+224h] [ebp-368h]
    float v43; // [esp+228h] [ebp-364h]
    float v44; // [esp+22Ch] [ebp-360h]
    float v45; // [esp+230h] [ebp-35Ch]
    float vNr[4]; // [esp+238h] [ebp-354h]
    int iTempCnt1; // [esp+248h] [ebp-344h] BYREF
    float vTemp2[4]; // [esp+24Ch] [ebp-340h] BYREF
    float vRotCol2[4]; // [esp+25Ch] [ebp-330h]
    int iB0; // [esp+26Ch] [ebp-320h]
    int iB1; // [esp+270h] [ebp-31Ch]
    float vCenter[4]; // [esp+274h] [ebp-318h]
    float avArrayIn[16]; // [esp+284h] [ebp-308h] BYREF
    dVector3 avTempArray1_1[9]; // [esp+2C4h] [ebp-2C8h] BYREF
    float *v55; // [esp+358h] [ebp-234h]
    float v56; // [esp+35Ch] [ebp-230h]
    float v57[4]; // [esp+360h] [ebp-22Ch] BYREF
    int k; // [esp+370h] [ebp-21Ch]
    int j; // [esp+374h] [ebp-218h]
    int v60; // [esp+378h] [ebp-214h]
    float v61; // [esp+37Ch] [ebp-210h]
    float v62; // [esp+380h] [ebp-20Ch]
    float v63; // [esp+384h] [ebp-208h]
    dVector3 avTempArray2[9]; // [esp+38Ch] [ebp-200h] BYREF
    int ctIn; // [esp+41Ch] [ebp-170h] BYREF
    dVector4 plane = { 0.0f, 0.0f, 0.0f, 0.0f }; // [esp+420h] [ebp-16Ch] BYREF
    float v67[4]; // [esp+430h] [ebp-15Ch] BYREF
    int ctOut; // [esp+440h] [ebp-14Ch] BYREF
    int sneed; // [esp+444h] [ebp-148h]
    int v70; // [esp+448h] [ebp-144h]
    float avPoints[3][4]; // [esp+44Ch] [ebp-140h] BYREF
    dVector3 avTempArray1[9]; // [esp+47Ch] [ebp-110h] BYREF
    float v73[4]; // [esp+510h] [ebp-7Ch] BYREF
    float fSign; // [esp+520h] [ebp-6Ch]
    int i; // [esp+524h] [ebp-68h]
    float vub[4]; // [esp+528h] [ebp-64h] BYREF
    float vPa[4]; // [esp+538h] [ebp-54h] BYREF
    float fParam2; // [esp+548h] [ebp-44h] BYREF
    int iEdge; // [esp+54Ch] [ebp-40h]
    dContactGeom *Contact; // [esp+550h] [ebp-3Ch]
    float vua[4]; // [esp+554h] [ebp-38h] BYREF
    float fParam1; // [esp+564h] [ebp-28h] BYREF
    float vPb[8]; // [esp+568h] [ebp-24h] BYREF
    int col; // [esp+588h] [ebp-4h]

    if (tbData->iBestAxis <= 4)
    {
        if (tbData->iBestAxis >= 2 && tbData->iBestAxis <= 4)
        {
            v67[0] = tbData->vBestNormal[0];
            v67[1] = tbData->vBestNormal[1];
            v67[2] = tbData->vBestNormal[2];
            v60 = tbData->iBestAxis - 2;
            if (v60)
            {
                sneed = 0;
                if (v60 == 1)
                    v70 = 2;
                else
                    v70 = 1;
            }
            else
            {
                sneed = 1;
                v70 = 2;
            }
            Vec3Sub((const float*)v0, tbData->vHullBoxPos, avPoints[0]);
            Vec3Sub((const float*)v1, tbData->vHullBoxPos, avPoints[1]);
            Vec3Sub((const float*)v2, tbData->vHullBoxPos, avPoints[2]);
            for (j = 0; j < 9; ++j)
            {
                avTempArray1[j][0] = 0.0;
                avTempArray1[j][1] = 0.0;
                avTempArray1[j][2] = 0.0;
                avTempArray2[j][0] = 0.0;
                avTempArray2[j][1] = 0.0;
                avTempArray2[j][2] = 0.0;
            }
            v61 = -v67[0];
            v62 = -v67[1];
            v63 = -v67[2];
            plane[0] = v61;
            plane[1] = v62;
            plane[2] = v63;
            plane[3] = tbData->vBoxHalfSize[v60];
            cldClipPolyToPlane(tbData, avPoints, 3, avTempArray1, &ctOut, &plane);
            v61 = tbData->mHullBoxRot[sneed];
            v62 = tbData->mHullBoxRot[sneed + 4];
            v63 = tbData->mHullBoxRot[sneed + 8];
            plane[0] = v61;
            plane[1] = v62;
            plane[2] = v63;
            plane[3] = tbData->vBoxHalfSize[sneed];
            cldClipPolyToPlane(tbData, avTempArray1, ctOut, avTempArray2, &ctIn, &plane);
            v61 = tbData->mHullBoxRot[sneed];
            v62 = tbData->mHullBoxRot[sneed + 4];
            v63 = tbData->mHullBoxRot[sneed + 8];
            v61 = -v61;
            v62 = -v62;
            v63 = -v63;
            plane[0] = v61;
            plane[1] = v62;
            plane[2] = v63;
            plane[3] = tbData->vBoxHalfSize[sneed];
            cldClipPolyToPlane(tbData, avTempArray2, ctIn, avTempArray1, &ctOut, &plane);
            v61 = tbData->mHullBoxRot[v70];
            v62 = tbData->mHullBoxRot[v70 + 4];
            v63 = tbData->mHullBoxRot[v70 + 8];
            plane[0] = v61;
            plane[1] = v62;
            plane[2] = v63;
            plane[3] = tbData->vBoxHalfSize[v70];
            cldClipPolyToPlane(tbData, avTempArray1, ctOut, avTempArray2, &ctIn, &plane);
            v61 = tbData->mHullBoxRot[v70];
            v62 = tbData->mHullBoxRot[v70 + 4];
            v63 = tbData->mHullBoxRot[v70 + 8];
            v61 = -v61;
            v62 = -v62;
            v63 = -v63;
            plane[0] = v61;
            plane[1] = v62;
            plane[2] = v63;
            plane[3] = tbData->vBoxHalfSize[v70];
            cldClipPolyToPlane(tbData, avTempArray2, ctIn, avTempArray1, &ctOut, &plane);
            for (k = 0; k < ctOut; ++k)
            {
                v5 = Vec3Dot(v67, avTempArray1[k]);
                v56 = v5 - tbData->vBoxHalfSize[v60];
                if (v56 > 0.0)
                    v56 = 0.0;
                Vec3Add(avTempArray1[k], tbData->vHullBoxPos, v57);
                if (tbData->ctContacts >= tbData->iFlags)
                    break;
                iStride = tbData->iStride;
                ctContacts = tbData->ctContacts;
                ContactGeoms = tbData->ContactGeoms;
                iFlags = tbData->iFlags;
                if (ctContacts >= iFlags)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\physics\\ode\\src\\collision_trimesh_internal.h",
                        47,
                        0,
                        "Index doesn't index Flags & 0x0ffff\n\t%i not in [0, %i)",
                        ctContacts,
                        iFlags);
                v55 = (ContactGeoms->pos + iStride * ctContacts);
                v55[8] = -v56;
                v19 = (char*)(v55 + 4);
                v55[4] = tbData->vBestNormal[0];
                *(v19 + 1) = tbData->vBestNormal[1];
                *(v19 + 2) = tbData->vBestNormal[2];
                *v55 = v57[0];
                v55[1] = v57[1];
                v55[2] = v57[2];
                //v55[9] = *&tbData->Geom1;
                v55[9] = (float)(tbData->Geom1->type);
                //v55[10] = *&tbData->Geom2;
                v55[10] = (float)(tbData->Geom2->type);
                if ((COERCE_UNSIGNED_INT(v55[4]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(v55[5]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(v55[6]) & 0x7F800000) == 0x7F800000)
                {
                    MyAssertHandler(
                        ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
                        892,
                        0,
                        "%s",
                        "!IS_NAN((Contact->normal)[0]) && !IS_NAN((Contact->normal)[1]) && !IS_NAN((Contact->normal)[2])");
                }
                if (!Vec3IsNormalized(v55 + 4))
                {
                    v6 = va("normal: (%f, %f, %f)", v55[4], v55[5], v55[6]);
                    MyAssertHandler(
                        ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
                        893,
                        0,
                        "%s\n\t%s",
                        "Vec3IsNormalized( Contact->normal )",
                        v6);
                }
                if ((COERCE_UNSIGNED_INT(*v55) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(v55[1]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(v55[2]) & 0x7F800000) == 0x7F800000)
                {
                    MyAssertHandler(
                        ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
                        894,
                        0,
                        "%s",
                        "!IS_NAN((Contact->pos)[0]) && !IS_NAN((Contact->pos)[1]) && !IS_NAN((Contact->pos)[2])");
                }
                if ((COERCE_UNSIGNED_INT(v55[8]) & 0x7F800000) == 0x7F800000)
                    MyAssertHandler(".\\physics\\ode\\src\\collision_trimesh_box.cpp", 895, 0, "%s", "!IS_NAN(Contact->depth)");
                if (Vec3Dot((v55 + 4), tbData->triangleNormal) > 0.0 && !alwaysfails)
                    MyAssertHandler(
                        ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
                        897,
                        0,
                        "Contact normal must be against the triangle normal");
                ++tbData->ctContacts;
            }
        }
    }
    else
    {
        vPa[0] = tbData->vHullBoxPos[0];
        vPa[1] = tbData->vHullBoxPos[1];
        vPa[2] = tbData->vHullBoxPos[2];
        for (i = 0; i < 3; ++i)
        {
            v73[0] = tbData->mHullBoxRot[i];
            v73[1] = tbData->mHullBoxRot[i + 4];
            v73[2] = tbData->mHullBoxRot[i + 8];
            if (Vec3Dot(tbData->vBestNormal, v73) <= 0.0)
                v14 = -1.0;
            else
                v14 = 1.0;
            fSign = v14;
            vPa[0] = v14 * tbData->vBoxHalfSize[i] * v73[0] + vPa[0];
            vPa[1] = v14 * tbData->vBoxHalfSize[i] * v73[1] + vPa[1];
            vPa[2] = v14 * tbData->vBoxHalfSize[i] * v73[2] + vPa[2];
        }
        iEdge = (tbData->iBestAxis - 5) % 3;
        if (iEdge)
        {
            if (iEdge == 1)
            {
                vPb[0] = (*v2)[0];
                vPb[1] = (*v2)[1];
                vPb[2] = (*v2)[2];
                vub[0] = tbData->triangleEdge1[0];
                vub[1] = tbData->triangleEdge1[1];
                vub[2] = tbData->triangleEdge1[2];
            }
            else
            {
                vPb[0] = (*v1)[0];
                vPb[1] = (*v1)[1];
                vPb[2] = (*v1)[2];
                vub[0] = tbData->triangleEdge2[0];
                vub[1] = tbData->triangleEdge2[1];
                vub[2] = tbData->triangleEdge2[2];
            }
        }
        else
        {
            vPb[0] = (*v0)[0];
            vPb[1] = (*v0)[1];
            vPb[2] = (*v0)[2];
            vub[0] = tbData->triangleEdge0[0];
            vub[1] = tbData->triangleEdge0[1];
            vub[2] = tbData->triangleEdge0[2];
        }
        dNormalize3(vub);
        col = (tbData->iBestAxis - 5) / 3;
        vua[0] = tbData->mHullBoxRot[col];
        vua[1] = tbData->mHullBoxRot[col + 4];
        vua[2] = tbData->mHullBoxRot[col + 8];
        cldClosestPointOnTwoLines(vPa, vua, vPb, vub, &fParam1, &fParam2);
        vPa[0] = vua[0] * fParam1 + vPa[0];
        vPa[1] = vua[1] * fParam1 + vPa[1];
        vPa[2] = vua[2] * fParam1 + vPa[2];
        vPb[0] = vub[0] * fParam2 + vPb[0];
        vPb[1] = vub[1] * fParam2 + vPb[1];
        vPb[2] = vub[2] * fParam2 + vPb[2];
        Vec3Add(vPa, vPb, &vPb[4]);
        vPb[4] = vPb[4] * 0.5;
        vPb[5] = vPb[5] * 0.5;
        vPb[6] = vPb[6] * 0.5;
        v25 = tbData->iStride;
        v26 = tbData->ctContacts;
        v27 = tbData->ContactGeoms;
        v28 = tbData->iFlags;
        if (v26 >= v28)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\physics\\ode\\src\\collision_trimesh_internal.h",
                47,
                0,
                "Index doesn't index Flags & 0x0ffff\n\t%i not in [0, %i)",
                v26,
                v28);
        Contact = (v27 + v25 * v26);
        Contact->depth = tbData->fBestDepth;
        normal = Contact->normal;
        Contact->normal[0] = tbData->vBestNormal[0];
        normal[1] = tbData->vBestNormal[1];
        normal[2] = tbData->vBestNormal[2];
        Contact->pos[0] = vPb[4];
        Contact->pos[1] = vPb[5];
        Contact->pos[2] = vPb[6];
        Contact->g1 = tbData->Geom1;
        Contact->g2 = tbData->Geom2;
        if ((COERCE_UNSIGNED_INT(Contact->normal[0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(Contact->normal[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(Contact->normal[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
                761,
                0,
                "%s",
                "!IS_NAN((Contact->normal)[0]) && !IS_NAN((Contact->normal)[1]) && !IS_NAN((Contact->normal)[2])");
        }
        if (!Vec3IsNormalized(Contact->normal))
        {
            v4 = va("normal: (%f, %f, %f)", Contact->normal[0], Contact->normal[1], Contact->normal[2]);
            MyAssertHandler(
                ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
                762,
                0,
                "%s\n\t%s",
                "Vec3IsNormalized( Contact->normal )",
                v4);
        }
        if ((COERCE_UNSIGNED_INT(Contact->pos[0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(Contact->pos[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(Contact->pos[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
                763,
                0,
                "%s",
                "!IS_NAN((Contact->pos)[0]) && !IS_NAN((Contact->pos)[1]) && !IS_NAN((Contact->pos)[2])");
        }
        if ((COERCE_UNSIGNED_INT(Contact->depth) & 0x7F800000) == 0x7F800000)
            MyAssertHandler(".\\physics\\ode\\src\\collision_trimesh_box.cpp", 764, 0, "%s", "!IS_NAN(Contact->depth)");
        ++tbData->ctContacts;
    }
    if (!Vec3IsNormalized(tbData->triangleNormal))
    {
        v7 = va("normal: (%f, %f, %f)", tbData->triangleNormal[0], tbData->triangleNormal[1], tbData->triangleNormal[2]);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            915,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( tbData->triangleNormal )",
            v7);
    }
    v41[0] = tbData->triangleNormal[0];
    v41[1] = tbData->triangleNormal[1];
    v41[2] = tbData->triangleNormal[2];
    mTransposed[0] = tbData->mHullBoxRot[0];
    mTransposed[1] = tbData->mHullBoxRot[4];
    mTransposed[2] = tbData->mHullBoxRot[8];
    mTransposed[4] = tbData->mHullBoxRot[1];
    mTransposed[5] = tbData->mHullBoxRot[5];
    mTransposed[6] = tbData->mHullBoxRot[9];
    mTransposed[8] = tbData->mHullBoxRot[2];
    mTransposed[9] = tbData->mHullBoxRot[6];
    mTransposed[10] = tbData->mHullBoxRot[10];
    vNr[0] = mTransposed[0] * v41[0] + mTransposed[1] * v41[1] + mTransposed[2] * v41[2];
    vNr[1] = mTransposed[4] * v41[0] + mTransposed[5] * v41[1] + mTransposed[6] * v41[2];
    vNr[2] = mTransposed[8] * v41[0] + mTransposed[9] * v41[1] + mTransposed[10] * v41[2];
    v13 = fabs(vNr[0]);
    vAbsNormal[0] = v13;
    v12 = fabs(vNr[1]);
    vAbsNormal[1] = v12;
    v11 = fabs(vNr[2]);
    vAbsNormal[2] = v11;
    if (v13 >= v12)
    {
        if (vAbsNormal[2] >= vAbsNormal[0])
        {
            iB1 = 0;
            iB2 = 1;
            iB0 = 2;
        }
        else
        {
            iB0 = 0;
            iB1 = 1;
            iB2 = 2;
        }
    }
    else
    {
        iB1 = 0;
        if (vAbsNormal[2] >= vAbsNormal[1])
        {
            iB2 = 1;
            iB0 = 2;
        }
        else
        {
            iB0 = 1;
            iB2 = 2;
        }
    }
    v43 = tbData->mHullBoxRot[iB0];
    v44 = tbData->mHullBoxRot[iB0 + 4];
    v45 = tbData->mHullBoxRot[iB0 + 8];
    if (vNr[iB0] <= 0.0)
    {
        vCenter[0] = tbData->vHullBoxPos[0] - (*v0)[0] + tbData->vBoxHalfSize[iB0] * v43;
        vCenter[1] = tbData->vHullBoxPos[1] - (*v0)[1] + tbData->vBoxHalfSize[iB0] * v44;
        v8 = tbData->vHullBoxPos[2] - (*v0)[2] + tbData->vBoxHalfSize[iB0] * v45;
    }
    else
    {
        vCenter[0] = tbData->vHullBoxPos[0] - (*v0)[0] - tbData->vBoxHalfSize[iB0] * v43;
        vCenter[1] = tbData->vHullBoxPos[1] - (*v0)[1] - tbData->vBoxHalfSize[iB0] * v44;
        v8 = tbData->vHullBoxPos[2] - (*v0)[2] - tbData->vBoxHalfSize[iB0] * v45;
    }
    vCenter[2] = v8;
    v43 = tbData->mHullBoxRot[iB1];
    v44 = tbData->mHullBoxRot[iB1 + 4];
    v45 = tbData->mHullBoxRot[iB1 + 8];
    vRotCol2[0] = tbData->mHullBoxRot[iB2];
    vRotCol2[1] = tbData->mHullBoxRot[iB2 + 4];
    vRotCol2[2] = tbData->mHullBoxRot[iB2 + 8];
    for (x = 0; x < 3; ++x)
    {
        avArrayIn[x] = tbData->vBoxHalfSize[iB1] * *(&v43 + x) + vCenter[x] - tbData->vBoxHalfSize[iB2] * vRotCol2[x];
        avArrayIn[x + 4] = vCenter[x] - tbData->vBoxHalfSize[iB1] * *(&v43 + x) - tbData->vBoxHalfSize[iB2] * vRotCol2[x];
        avArrayIn[x + 8] = vCenter[x] - tbData->vBoxHalfSize[iB1] * *(&v43 + x) + tbData->vBoxHalfSize[iB2] * vRotCol2[x];
        avArrayIn[x + 12] = tbData->vBoxHalfSize[iB1] * *(&v43 + x) + vCenter[x] + tbData->vBoxHalfSize[iB2] * vRotCol2[x];
    }
    iTempCnt1 = 0;
    iTempCnt2 = 0;
    for (i_1 = 0; i_1 < 9; ++i_1)
    {
        avTempArray1_1[i_1][0] = 0.0;
        avTempArray1_1[i_1][1] = 0.0;
        avTempArray1_1[i_1][2] = 0.0;
        avArrayOut[i_1][0] = 0.0;
        avArrayOut[i_1][1] = 0.0;
        avArrayOut[i_1][2] = 0.0;
    }
    a[0] = -tbData->triangleNormal[0];
    a[1] = -tbData->triangleNormal[1];
    a[2] = -tbData->triangleNormal[2];
    dNormalize3(a);
    if (!Vec3IsNormalized(a))
    {
        v9 = va("normal: (%f, %f, %f)", a[0], a[1], a[2]);
        MyAssertHandler(
            ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
            1032,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( vTemp )",
            v9);
    }
    plPlane[0] = a[0];
    plPlane[1] = a[1];
    plPlane[2] = a[2];
    plPlane[3] = 0.0;
    cldClipPolyToPlane(tbData, (dVector3*)avArrayIn, 4, avTempArray1_1, &iTempCnt1, &plPlane);
    Vec3Sub((const float*)v1, (const float*)v0, vTemp2);
    a[0] = tbData->triangleNormal[1] * vTemp2[2] - tbData->triangleNormal[2] * vTemp2[1];
    a[1] = tbData->triangleNormal[2] * vTemp2[0] - tbData->triangleNormal[0] * vTemp2[2];
    a[2] = tbData->triangleNormal[0] * vTemp2[1] - tbData->triangleNormal[1] * vTemp2[0];
    dNormalize3(a);
    plPlane[0] = a[0];
    plPlane[1] = a[1];
    plPlane[2] = a[2];
    plPlane[3] = 0.0;
    cldClipPolyToPlane(tbData, avTempArray1_1, iTempCnt1, avArrayOut, &iTempCnt2, &plPlane);
    Vec3Sub((const float *)v2, (const float *)v1, vTemp2);
    a[0] = tbData->triangleNormal[1] * vTemp2[2] - tbData->triangleNormal[2] * vTemp2[1];
    a[1] = tbData->triangleNormal[2] * vTemp2[0] - tbData->triangleNormal[0] * vTemp2[2];
    a[2] = tbData->triangleNormal[0] * vTemp2[1] - tbData->triangleNormal[1] * vTemp2[0];
    dNormalize3(a);
    Vec3Sub((const float *)v0, (const float *)v2, vTemp2);
    plPlane[0] = a[0];
    plPlane[1] = a[1];
    plPlane[2] = a[2];
    plPlane[3] = Vec3Dot(vTemp2, a);
    cldClipPolyToPlane(tbData, avArrayOut, iTempCnt2, avTempArray1_1, &iTempCnt1, &plPlane);
    Vec3Sub((const float *)v0, (const float *)v2, vTemp2);
    a[0] = tbData->triangleNormal[1] * vTemp2[2] - tbData->triangleNormal[2] * vTemp2[1];
    a[1] = tbData->triangleNormal[2] * vTemp2[0] - tbData->triangleNormal[0] * vTemp2[2];
    a[2] = tbData->triangleNormal[0] * vTemp2[1] - tbData->triangleNormal[1] * vTemp2[0];
    dNormalize3(a);
    plPlane[0] = a[0];
    plPlane[1] = a[1];
    plPlane[2] = a[2];
    plPlane[3] = 0.0;
    cldClipPolyToPlane(tbData, avTempArray1_1, iTempCnt1, avArrayOut, &iTempCnt2, &plPlane);
    for (m = 0; m < iTempCnt2; ++m)
    {
        v30 = Vec3Dot(v41, avArrayOut[m]);
        if (v30 > 0.0)
            v30 = 0.0;
        Vec3Add(avArrayOut[m], (const float *)v0, sum);
        if (tbData->ctContacts >= tbData->iFlags)
            break;
        v15 = tbData->iStride;
        v16 = tbData->ctContacts;
        v17 = tbData->ContactGeoms;
        v18 = tbData->iFlags;
        if (v16 >= v18)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\physics\\ode\\src\\collision_trimesh_internal.h",
                47,
                0,
                "Index doesn't index Flags & 0x0ffff\n\t%i not in [0, %i)",
                v16,
                v18);
        v29 = (v17->pos + v15 * v16);
        v29[8] = -v30;
        v29[4] = -tbData->triangleNormal[0];
        v29[5] = -tbData->triangleNormal[1];
        v29[6] = -tbData->triangleNormal[2];
        *v29 = sum[0];
        v29[1] = sum[1];
        v29[2] = sum[2];
        //v29[9] = *&tbData->Geom1;
        v29[9] = (float)(tbData->Geom1->type);
        //v29[10] = *&tbData->Geom2;
        v29[10] = (float)(tbData->Geom2->type);
        if ((COERCE_UNSIGNED_INT(v29[4]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(v29[5]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(v29[6]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
                1094,
                0,
                "%s",
                "!IS_NAN((Contact->normal)[0]) && !IS_NAN((Contact->normal)[1]) && !IS_NAN((Contact->normal)[2])");
        }
        if (!Vec3IsNormalized(v29 + 4))
        {
            v10 = va("normal: (%f, %f, %f)", v29[4], v29[5], v29[6]);
            MyAssertHandler(
                ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
                1095,
                0,
                "%s\n\t%s",
                "Vec3IsNormalized( Contact->normal )",
                v10);
        }
        if ((COERCE_UNSIGNED_INT(*v29) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(v29[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(v29[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
                1096,
                0,
                "%s",
                "!IS_NAN((Contact->pos)[0]) && !IS_NAN((Contact->pos)[1]) && !IS_NAN((Contact->pos)[2])");
        }
        if ((COERCE_UNSIGNED_INT(v29[8]) & 0x7F800000) == 0x7F800000)
            MyAssertHandler(".\\physics\\ode\\src\\collision_trimesh_box.cpp", 1097, 0, "%s", "!IS_NAN(Contact->depth)");
        if (Vec3Dot((v29 + 4), tbData->triangleNormal) > 0.0 && !alwaysfails)
            MyAssertHandler(
                ".\\physics\\ode\\src\\collision_trimesh_box.cpp",
                1099,
                0,
                "Contact normal must be against the triangle normal");
        ++tbData->ctContacts;
    }
}

// test one mesh triangle on intersection with given box
static void _cldTestOneTriangle(collData_t *tbData, const dVector3 &v0, const dVector3 &v1, const dVector3 &v2)//, void *pvUser)
{
    // do intersection test and find best separating axis
    if (!cldTestSeparatingAxes(tbData, &v0, &v1, &v2)) {
        // if not found do nothing
        return;
    }

    //// if best separation axis is not found
    //if (iBestAxis == 0) {
    //    // this should not happen (we should already exit in that case)
    //    //dMessage (0, "best separation axis not found");
    //    // do nothing
    //    return;
    //}

    iassert(tbData->iBestAxis != 0);
    iassert(Vec3IsNormalized(tbData->vBestNormal));

    cldClipping(tbData, &v0, &v1, &v2);
}


int __cdecl dCollideBoxTriangleList(
    const unsigned __int16 *indices,
    const float (*verts)[3],
    int triCount,
    const float *boxR,
    const float *boxPos,
    const float *boxHalfExtents,
    const float *bodyCenter,
    int Flags,
    dContactGeom *Contacts,
    int Stride)
{
    const float *v11; // [esp+4h] [ebp-118h]
    const float *v12; // [esp+Ch] [ebp-110h]
    const float *v13; // [esp+10h] [ebp-10Ch]
    collData_t tbData; // [esp+14h] [ebp-108h] BYREF
    float dv[3][4]; // [esp+DCh] [ebp-40h] BYREF
    int i; // [esp+10Ch] [ebp-10h]
    const float (*mRotBox)[12]; // [esp+110h] [ebp-Ch]
    const unsigned __int16 *pIndices; // [esp+114h] [ebp-8h]
    const float (*vPosBox)[4]; // [esp+118h] [ebp-4h]

    mRotBox = (const float(*)[12])boxR;
    vPosBox = (const float(*)[4])boxPos;
    tbData.mHullBoxRot[0] = *boxR;
    tbData.mHullBoxRot[1] = boxR[1];
    tbData.mHullBoxRot[2] = boxR[2];
    tbData.mHullBoxRot[3] = boxR[3];
    tbData.mHullBoxRot[4] = boxR[4];
    tbData.mHullBoxRot[5] = boxR[5];
    tbData.mHullBoxRot[6] = boxR[6];
    tbData.mHullBoxRot[7] = boxR[7];
    tbData.mHullBoxRot[8] = boxR[8];
    tbData.mHullBoxRot[9] = boxR[9];
    tbData.mHullBoxRot[10] = boxR[10];
    tbData.mHullBoxRot[11] = boxR[11];
    tbData.vHullBoxPos[0] = *boxPos;
    tbData.vHullBoxPos[1] = boxPos[1];
    tbData.vHullBoxPos[2] = boxPos[2];
    tbData.vBoxHalfSize[0] = *boxHalfExtents;
    tbData.vBoxHalfSize[1] = boxHalfExtents[1];
    tbData.vBoxHalfSize[2] = boxHalfExtents[2];
    tbData.ctContacts = 0;
    tbData.iStride = Stride;
    tbData.iFlags = Flags;
    tbData.ContactGeoms = Contacts;
    tbData.fBestDepth = 3.4028235e38;
    tbData.vBestNormal[0] = 0.0;
    tbData.vBestNormal[1] = 0.0;
    tbData.vBestNormal[2] = 0.0;
    tbData.bodyCenter = bodyCenter;
    pIndices = indices;
    for (i = 0; i < triCount && tbData.ctContacts < LOWORD(tbData.iFlags); ++i)
    {
        v13 = &(*verts)[3 * *pIndices];
        dv[0][0] = *v13;
        dv[0][1] = v13[1];
        dv[0][2] = v13[2];
        dv[0][3] = 0.0;
        v12 = &(*verts)[3 * *++pIndices];
        dv[1][0] = *v12;
        dv[1][1] = v12[1];
        dv[1][2] = v12[2];
        dv[1][3] = 0.0;
        v11 = &(*verts)[3 * *++pIndices];
        dv[2][0] = *v11;
        dv[2][1] = v11[1];
        dv[2][2] = v11[2];
        dv[2][3] = 0.0;
        ++pIndices;
        _cldTestOneTriangle(&tbData, dv[2], dv[1], *dv);
    }
    return tbData.ctContacts;
}

// LWSS END


// largest number, double or float
#if defined(dSINGLE)
  #define MAXVALUE FLT_MAX
#else
  #define MAXVALUE DBL_MAX
#endif


// dVector3
// r=a-b
#define SUBTRACT(a,b,r) do{ \
  (r)[0]=(a)[0] - (b)[0]; \
  (r)[1]=(a)[1] - (b)[1]; \
  (r)[2]=(a)[2] - (b)[2]; }while(0)


// dVector3
// a=b
#define SET(a,b) do{ \
  (a)[0]=(b)[0]; \
  (a)[1]=(b)[1]; \
  (a)[2]=(b)[2]; }while(0)


// dMatrix3
// a=b
#define SETM(a,b) do{ \
  (a)[0]=(b)[0]; \
  (a)[1]=(b)[1]; \
  (a)[2]=(b)[2]; \
  (a)[3]=(b)[3]; \
  (a)[4]=(b)[4]; \
  (a)[5]=(b)[5]; \
  (a)[6]=(b)[6]; \
  (a)[7]=(b)[7]; \
  (a)[8]=(b)[8]; \
  (a)[9]=(b)[9]; \
  (a)[10]=(b)[10]; \
                     (a)[11]=(b)[11]; }while(0)


// dVector3
// r=a+b
#define ADD(a,b,r) do{ \
  (r)[0]=(a)[0] + (b)[0]; \
  (r)[1]=(a)[1] + (b)[1]; \
  (r)[2]=(a)[2] + (b)[2]; }while(0)


// dMatrix3, int, dVector3
// v=column a from m
#define GETCOL(m,a,v) do{ \
  (v)[0]=(m)[(a)+0]; \
  (v)[1]=(m)[(a)+4]; \
  (v)[2]=(m)[(a)+8]; }while(0)


// dVector4, dVector3
// distance between plane p and point v
#define POINTDISTANCE(p,v) \
  ( p[0]*v[0] + p[1]*v[1] + p[2]*v[2] + p[3] )


// dVector4, dVector3, dReal
// construct plane from normal and d
#define CONSTRUCTPLANE(plane,normal,d) do{ \
  plane[0]=normal[0];\
  plane[1]=normal[1];\
  plane[2]=normal[2];\
  plane[3]=d; }while(0)


// dVector3
// length of vector a
#define LENGTHOF(a) \
  dSqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2])


// box data
static dMatrix3 mHullBoxRot;
static dVector3 vHullBoxPos;
static dVector3 vBoxHalfSize;

// mesh data
static dVector3   vHullDstPos;

// global collider data
static dVector3 vBestNormal;
static dReal    fBestDepth;
static int    iBestAxis = 0;
static int    iExitAxis = 0;
static dVector3 vE0, vE1, vE2, vN;

// global info for contact creation
static int iFlags;
static dContactGeom *ContactGeoms;
static int iStride;
static dxGeom *Geom1;
static dxGeom *Geom2;
static int ctContacts = 0;



// Test normal of mesh face as separating axis for intersection
// LWSS: Changed
#if 0
static BOOL _cldTestNormal( dReal fp0, dReal fR, dVector3 vNormal, int iAxis ) 
{
  // calculate overlapping interval of box and triangle
  dReal fDepth = fR+fp0;
  
  // if we do not overlap
  if ( fDepth<0 ) { 
    // do nothing
    return FALSE;
  }

  // calculate normal's length
  dReal fLength = LENGTHOF(vNormal);
  // if long enough
  if ( fLength > 0.0f ) {

    dReal fOneOverLength = 1.0f/fLength;
    // normalize depth
    fDepth = fDepth*fOneOverLength;

    // get minimum depth
    if (fDepth<fBestDepth) {
      vBestNormal[0] = -vNormal[0]*fOneOverLength;
      vBestNormal[1] = -vNormal[1]*fOneOverLength;
      vBestNormal[2] = -vNormal[2]*fOneOverLength;
      iBestAxis = iAxis;
      //dAASSERT(fDepth>=0);
      fBestDepth = fDepth;
    }

  }

  return TRUE;
}
#endif




// Test box axis as separating axis 
#if 0 // LWSS: Changed
static BOOL _cldTestFace( dReal fp0, dReal fp1, dReal fp2, dReal fR, dReal fD, 
                          dVector3 vNormal, int iAxis ) 
{
  dReal fMin, fMax;

  // find min of triangle interval 
  if ( fp0 < fp1 ) {
    if ( fp0 < fp2 ) {
      fMin = fp0;
    } else {
      fMin = fp2;
    }
  } else {
    if( fp1 < fp2 ) {
      fMin = fp1; 
    } else {
      fMin = fp2;
    }
  }

  // find max of triangle interval 
  if ( fp0 > fp1 ) {
    if ( fp0 > fp2 ) {
      fMax = fp0;
    } else {
      fMax = fp2;
    }
  } else {
    if( fp1 > fp2 ) {
      fMax = fp1; 
    } else {
      fMax = fp2;
    }
  }

  // calculate minimum and maximum depth
  dReal fDepthMin = fR - fMin;
  dReal fDepthMax = fMax + fR;

  // if we dont't have overlapping interval
  if ( fDepthMin < 0 || fDepthMax < 0 ) {
    // do nothing
    return FALSE;
  }

  dReal fDepth = 0;

  // if greater depth is on negative side 
  if ( fDepthMin > fDepthMax ) {
    // use smaller depth (one from positive side)
    fDepth = fDepthMax;
    // flip normal direction
    vNormal[0] = -vNormal[0];
    vNormal[1] = -vNormal[1];
    vNormal[2] = -vNormal[2];
    fD = -fD;
  // if greater depth is on positive side 
  } else {
    // use smaller depth (one from negative side)
    fDepth = fDepthMin;   
  }

  
  // if lower depth than best found so far 
  if (fDepth<fBestDepth) {
    // remember current axis as best axis
    vBestNormal[0]  = vNormal[0];
    vBestNormal[1]  = vNormal[1];
    vBestNormal[2]  = vNormal[2];
    iBestAxis    = iAxis;
    //dAASSERT(fDepth>=0);
    fBestDepth   = fDepth;
  }

  return TRUE;
}
#endif

// Test cross products of box axis and triangle edges as separating axis
#if 0 // LWSS: Changed
static BOOL _cldTestEdge( dReal fp0, dReal fp1, dReal fR, dReal fD, 
                          dVector3 vNormal, int iAxis ) 
{
  dReal fMin, fMax;

  // calculate min and max interval values  
  if ( fp0 < fp1 ) {
    fMin = fp0;
    fMax = fp1;
  } else {
    fMin = fp1;
    fMax = fp0;    
  }

  // check if we overlapp
  dReal fDepthMin = fR - fMin;
  dReal fDepthMax = fMax + fR;

  // if we don't overlapp
  if ( fDepthMin < 0 || fDepthMax < 0 ) {
    // do nothing
    return FALSE;
  }

  dReal fDepth;
  

  // if greater depth is on negative side 
  if ( fDepthMin > fDepthMax ) {
    // use smaller depth (one from positive side)
    fDepth = fDepthMax;
    // flip normal direction
    vNormal[0] = -vNormal[0];
    vNormal[1] = -vNormal[1];
    vNormal[2] = -vNormal[2];
    fD = -fD;
  // if greater depth is on positive side 
  } else {
    // use smaller depth (one from negative side)
    fDepth = fDepthMin;   
  }

  // calculate normal's length
  dReal fLength = LENGTHOF(vNormal);

  // if long enough
  if ( fLength > 0.0f ) {

    // normalize depth
    dReal fOneOverLength = 1.0f/fLength;
    fDepth = fDepth*fOneOverLength;
    fD*=fOneOverLength;
    

    // if lower depth than best found so far (favor face over edges)
    if (fDepth*1.5f<fBestDepth) {
      // remember current axis as best axis
      vBestNormal[0]  = vNormal[0]*fOneOverLength;
      vBestNormal[1]  = vNormal[1]*fOneOverLength;
      vBestNormal[2]  = vNormal[2]*fOneOverLength;
      iBestAxis    = iAxis;
      //dAASSERT(fDepth>=0);
      fBestDepth   = fDepth;
    }
  }

  return TRUE;
}
#endif




// clip polygon with plane and generate new polygon points
#if 0 // LWSS: Changed
static void _cldClipPolyToPlane( dVector3 avArrayIn[], int ctIn, 
                      dVector3 avArrayOut[], int &ctOut, 
                      const dVector4 &plPlane )
{
  // start with no output points
  ctOut = 0;

  int i0 = ctIn-1;

  // for each edge in input polygon
  for (int i1=0; i1<ctIn; i0=i1, i1++) {
  

    // calculate distance of edge points to plane
    dReal fDistance0 = POINTDISTANCE( plPlane ,avArrayIn[i0] );
    dReal fDistance1 = POINTDISTANCE( plPlane ,avArrayIn[i1] );


    // if first point is in front of plane
    if( fDistance0 >= 0 ) {
      // emit point
      avArrayOut[ctOut][0] = avArrayIn[i0][0];
      avArrayOut[ctOut][1] = avArrayIn[i0][1];
      avArrayOut[ctOut][2] = avArrayIn[i0][2];
      ctOut++;
    }

    // if points are on different sides
    if( (fDistance0 > 0 && fDistance1 < 0) || ( fDistance0 < 0 && fDistance1 > 0) ) {

      // find intersection point of edge and plane
      dVector3 vIntersectionPoint;
      vIntersectionPoint[0]= avArrayIn[i0][0] - (avArrayIn[i0][0]-avArrayIn[i1][0])*fDistance0/(fDistance0-fDistance1);
      vIntersectionPoint[1]= avArrayIn[i0][1] - (avArrayIn[i0][1]-avArrayIn[i1][1])*fDistance0/(fDistance0-fDistance1);
      vIntersectionPoint[2]= avArrayIn[i0][2] - (avArrayIn[i0][2]-avArrayIn[i1][2])*fDistance0/(fDistance0-fDistance1);

      // emit intersection point
      avArrayOut[ctOut][0] = vIntersectionPoint[0];
      avArrayOut[ctOut][1] = vIntersectionPoint[1];
      avArrayOut[ctOut][2] = vIntersectionPoint[2];
      ctOut++;
    }
  }

}
#endif



#if 0 // LWSS: Changed 
static BOOL _cldTestSeparatingAxes(const dVector3 &v0, const dVector3 &v1, const dVector3 &v2) {
  // reset best axis
  iBestAxis = 0;
  iExitAxis = -1;
  fBestDepth = MAXVALUE;

  // calculate edges
  SUBTRACT(v1,v0,vE0);
  SUBTRACT(v2,v0,vE1);
  SUBTRACT(vE1,vE0,vE2);

  // calculate poly normal
  dCROSS(vN,=,vE0,vE1);

  // extract box axes as vectors
  dVector3 vA0,vA1,vA2;
  GETCOL(mHullBoxRot,0,vA0);
  GETCOL(mHullBoxRot,1,vA1);
  GETCOL(mHullBoxRot,2,vA2);

  // box halfsizes
  dReal fa0 = vBoxHalfSize[0];
  dReal fa1 = vBoxHalfSize[1];
  dReal fa2 = vBoxHalfSize[2];

  // calculate relative position between box and triangle
  dVector3 vD;
  SUBTRACT(v0,vHullBoxPos,vD);

  // calculate length of face normal
  dReal fNLen = LENGTHOF( vN );

  dVector3 vL;
  dReal fp0, fp1, fp2, fR, fD;

  // Test separating axes for intersection
  // ************************************************
  // Axis 1 - Triangle Normal 
  SET(vL,vN);
  fp0  = dDOT(vL,vD);
  fp1  = fp0;
  fp2  = fp0;
  fR=fa0*dFabs( dDOT(vN,vA0) ) + fa1 * dFabs( dDOT(vN,vA1) ) + fa2 * dFabs( dDOT(vN,vA2) );


  if( !_cldTestNormal( fp0, fR, vL, 1) ) { 
    iExitAxis=1;
    return FALSE; 
  } 
 
  // ************************************************

  // Test Faces
  // ************************************************
  // Axis 2 - Box X-Axis
  SET(vL,vA0);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 + dDOT(vA0,vE0);
  fp2 = fp0 + dDOT(vA0,vE1);
  fR  = fa0;


  if( !_cldTestFace( fp0, fp1, fp2, fR, fD, vL, 2) ) { 
    iExitAxis=2;
    return FALSE; 
  }
  // ************************************************

  // ************************************************
  // Axis 3 - Box Y-Axis
  SET(vL,vA1);
  fD = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 + dDOT(vA1,vE0);
  fp2 = fp0 + dDOT(vA1,vE1);
  fR  = fa1;


  if( !_cldTestFace( fp0, fp1, fp2, fR, fD, vL, 3) ) { 
    iExitAxis=3;
    return FALSE; 
  }

  // ************************************************

  // ************************************************
  // Axis 4 - Box Z-Axis
  SET(vL,vA2);
  fD = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 + dDOT(vA2,vE0);
  fp2 = fp0 + dDOT(vA2,vE1);
  fR  = fa2;


  if( !_cldTestFace( fp0, fp1, fp2, fR, fD, vL, 4) ) { 
    iExitAxis=4;
    return FALSE; 
  }

  // ************************************************

  // Test Edges
  // ************************************************
  // Axis 5 - Box X-Axis cross Edge0
  dCROSS(vL,=,vA0,vE0);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0;
  fp2 = fp0 + dDOT(vA0,vN);
  fR  = fa1 * dFabs(dDOT(vA2,vE0)) + fa2 * dFabs(dDOT(vA1,vE0));


  if( !_cldTestEdge( fp1, fp2, fR, fD, vL, 5) ) { 
    iExitAxis=5;
    return FALSE; 
  }
  // ************************************************

  // ************************************************
  // Axis 6 - Box X-Axis cross Edge1
  dCROSS(vL,=,vA0,vE1);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 - dDOT(vA0,vN);
  fp2 = fp0;
  fR  = fa1 * dFabs(dDOT(vA2,vE1)) + fa2 * dFabs(dDOT(vA1,vE1));


  if( !_cldTestEdge( fp0, fp1, fR, fD, vL, 6) ) { 
    iExitAxis=6;
    return FALSE; 
  }
  // ************************************************

  // ************************************************
  // Axis 7 - Box X-Axis cross Edge2
  dCROSS(vL,=,vA0,vE2);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 - dDOT(vA0,vN);
  fp2 = fp0 - dDOT(vA0,vN);
  fR  = fa1 * dFabs(dDOT(vA2,vE2)) + fa2 * dFabs(dDOT(vA1,vE2));


  if( !_cldTestEdge( fp0, fp1, fR, fD, vL, 7) ) { 
    iExitAxis=7;
    return FALSE; 
  }

  // ************************************************

  // ************************************************
  // Axis 8 - Box Y-Axis cross Edge0
  dCROSS(vL,=,vA1,vE0);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0;
  fp2 = fp0 + dDOT(vA1,vN);
  fR  = fa0 * dFabs(dDOT(vA2,vE0)) + fa2 * dFabs(dDOT(vA0,vE0));


  if( !_cldTestEdge( fp0, fp2, fR, fD, vL, 8) ) { 
    iExitAxis=8;
    return FALSE; 
  }

  // ************************************************

  // ************************************************
  // Axis 9 - Box Y-Axis cross Edge1
  dCROSS(vL,=,vA1,vE1);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 - dDOT(vA1,vN);
  fp2 = fp0;
  fR  = fa0 * dFabs(dDOT(vA2,vE1)) + fa2 * dFabs(dDOT(vA0,vE1));


  if( !_cldTestEdge( fp0, fp1, fR, fD, vL, 9) ) { 
    iExitAxis=9;
    return FALSE; 
  }

  // ************************************************

  // ************************************************
  // Axis 10 - Box Y-Axis cross Edge2
  dCROSS(vL,=,vA1,vE2);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 - dDOT(vA1,vN);
  fp2 = fp0 - dDOT(vA1,vN);
  fR  = fa0 * dFabs(dDOT(vA2,vE2)) + fa2 * dFabs(dDOT(vA0,vE2));


  if( !_cldTestEdge( fp0, fp1, fR, fD, vL, 10) ) { 
    iExitAxis=10;
    return FALSE; 
  }

  // ************************************************

  // ************************************************
  // Axis 11 - Box Z-Axis cross Edge0
  dCROSS(vL,=,vA2,vE0);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0;
  fp2 = fp0 + dDOT(vA2,vN);
  fR  = fa0 * dFabs(dDOT(vA1,vE0)) + fa1 * dFabs(dDOT(vA0,vE0));


  if( !_cldTestEdge( fp0, fp2, fR, fD, vL, 11) ) { 
    iExitAxis=11;
    return FALSE; 
  }
  // ************************************************

  // ************************************************
  // Axis 12 - Box Z-Axis cross Edge1
  dCROSS(vL,=,vA2,vE1);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 - dDOT(vA2,vN);
  fp2 = fp0;
  fR  = fa0 * dFabs(dDOT(vA1,vE1)) + fa1 * dFabs(dDOT(vA0,vE1));


  if( !_cldTestEdge( fp0, fp1, fR, fD, vL, 12) ) { 
    iExitAxis=12;
    return FALSE; 
  }
  // ************************************************

  // ************************************************
  // Axis 13 - Box Z-Axis cross Edge2
  dCROSS(vL,=,vA2,vE2);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 - dDOT(vA2,vN);
  fp2 = fp0 - dDOT(vA2,vN);
  fR  = fa0 * dFabs(dDOT(vA1,vE2)) + fa1 * dFabs(dDOT(vA0,vE2));


  if( !_cldTestEdge( fp0, fp1, fR, fD, vL, 13) ) { 
    iExitAxis=13;
    return FALSE; 
  }
 
  // ************************************************
  return TRUE; 
}
#endif

// find two closest points on two lines
#if 0 // LWSS: Changed quite a bit (Only used by cldClipping()
static BOOL _cldClosestPointOnTwoLines( dVector3 vPoint1, dVector3 vLenVec1, 
                                        dVector3 vPoint2, dVector3 vLenVec2, 
                                        dReal &fvalue1, dReal &fvalue2) 
{
  // calulate denominator
  dVector3 vp;
  SUBTRACT(vPoint2,vPoint1,vp);
  dReal fuaub  = dDOT(vLenVec1,vLenVec2);
  dReal fq1    = dDOT(vLenVec1,vp);
  dReal fq2    = -dDOT(vLenVec2,vp);
  dReal fd     = 1.0f - fuaub * fuaub;
  
  // if denominator is positive
  if (fd > 0.0f) {
    // calculate points of closest approach
    fd = 1.0f/fd;
    fvalue1 = (fq1 + fuaub*fq2)*fd;
    fvalue2 = (fuaub*fq1 + fq2)*fd;
    return TRUE;
  // otherwise  
  } else {
    // lines are parallel
    fvalue1 = 0.0f;
    fvalue2 = 0.0f;
    return FALSE;
  }

}
#endif

// clip and generate contacts
#if 0 // LWSS: has completely changed
static void _cldClipping(const dVector3 &v0, const dVector3 &v1, const dVector3 &v2) {

  // if we have edge/edge intersection
  if ( iBestAxis > 4 ) {

    dVector3 vub,vPb,vPa;

    SET(vPa,vHullBoxPos);

    // calculate point on box edge
    for( int i=0; i<3; i++) {
      dVector3 vRotCol;
      GETCOL(mHullBoxRot,i,vRotCol);
      dReal fSign = dDOT(vBestNormal,vRotCol) > 0 ? 1.0f : -1.0f;

      vPa[0] += fSign * vBoxHalfSize[i] * vRotCol[0];
      vPa[1] += fSign * vBoxHalfSize[i] * vRotCol[1];
      vPa[2] += fSign * vBoxHalfSize[i] * vRotCol[2];
    }

    int iEdge = (iBestAxis-5)%3;

    // decide which edge is on triangle
    if ( iEdge == 0 ) {
      SET(vPb,v0);
      SET(vub,vE0);
    } else if ( iEdge == 1) {
      SET(vPb,v2);
      SET(vub,vE1);
    } else {
      SET(vPb,v1);
      SET(vub,vE2);
    }
    

    // setup direction parameter for face edge
    dNormalize3(vub);

    dReal fParam1, fParam2;

    // setup direction parameter for box edge
    dVector3 vua;
    int col=(iBestAxis-5)/3;
    GETCOL(mHullBoxRot,col,vua);

    // find two closest points on both edges
    _cldClosestPointOnTwoLines( vPa, vua, vPb, vub, fParam1, fParam2 );
    vPa[0] += vua[0]*fParam1;
    vPa[1] += vua[1]*fParam1;
    vPa[2] += vua[2]*fParam1;

    vPb[0] += vub[0]*fParam2; 
    vPb[1] += vub[1]*fParam2; 
    vPb[2] += vub[2]*fParam2; 

    // calculate collision point
    dVector3 vPntTmp;
    ADD(vPa,vPb,vPntTmp);

    vPntTmp[0]*=0.5f;
    vPntTmp[1]*=0.5f;
    vPntTmp[2]*=0.5f;

    // generate contact point between two closest points
    dContactGeom* Contact = SAFECONTACT(iFlags, ContactGeoms, ctContacts, iStride);
    Contact->depth = fBestDepth;
    SET(Contact->normal,vBestNormal);
    SET(Contact->pos,vPntTmp);
    Contact->g1 = Geom1;
    Contact->g2 = Geom2;
    ctContacts++;


  // if triangle is the referent face then clip box to triangle face
  } else if ( iBestAxis == 1 ) {
    
    
    dVector3 vNormal2;
    vNormal2[0]=-vBestNormal[0];
    vNormal2[1]=-vBestNormal[1];
    vNormal2[2]=-vBestNormal[2];

    
    // vNr is normal in box frame, pointing from triangle to box
    dMatrix3 mTransposed;
    mTransposed[0*4+0]=mHullBoxRot[0*4+0];
    mTransposed[0*4+1]=mHullBoxRot[1*4+0];
    mTransposed[0*4+2]=mHullBoxRot[2*4+0];

    mTransposed[1*4+0]=mHullBoxRot[0*4+1];
    mTransposed[1*4+1]=mHullBoxRot[1*4+1];
    mTransposed[1*4+2]=mHullBoxRot[2*4+1];

    mTransposed[2*4+0]=mHullBoxRot[0*4+2];
    mTransposed[2*4+1]=mHullBoxRot[1*4+2];
    mTransposed[2*4+2]=mHullBoxRot[2*4+2];

    dVector3 vNr;
    vNr[0]=mTransposed[0*4+0]*vNormal2[0]+  mTransposed[0*4+1]*vNormal2[1]+  mTransposed[0*4+2]*vNormal2[2];
    vNr[1]=mTransposed[1*4+0]*vNormal2[0]+  mTransposed[1*4+1]*vNormal2[1]+  mTransposed[1*4+2]*vNormal2[2];
    vNr[2]=mTransposed[2*4+0]*vNormal2[0]+  mTransposed[2*4+1]*vNormal2[1]+  mTransposed[2*4+2]*vNormal2[2];
  

    dVector3 vAbsNormal;
    vAbsNormal[0] = dFabs( vNr[0] );
    vAbsNormal[1] = dFabs( vNr[1] );
    vAbsNormal[2] = dFabs( vNr[2] );

    // get closest face from box
    int iB0, iB1, iB2;
    if (vAbsNormal[1] > vAbsNormal[0]) {
      if (vAbsNormal[1] > vAbsNormal[2]) {
        iB1 = 0;  iB0 = 1;  iB2 = 2;
      } else {
        iB1 = 0;  iB2 = 1;  iB0 = 2;
      }
    } else {

      if (vAbsNormal[0] > vAbsNormal[2]) {
        iB0 = 0;  iB1 = 1;  iB2 = 2;
      } else {
        iB1 = 0;  iB2 = 1;  iB0 = 2;
      }
    }

    // Here find center of box face we are going to project
    dVector3 vCenter;
    dVector3 vRotCol;
    GETCOL(mHullBoxRot,iB0,vRotCol);
    
    if (vNr[iB0] > 0) {
        vCenter[0] = vHullBoxPos[0] - v0[0] - vBoxHalfSize[iB0] * vRotCol[0];
      vCenter[1] = vHullBoxPos[1] - v0[1] - vBoxHalfSize[iB0] * vRotCol[1];
      vCenter[2] = vHullBoxPos[2] - v0[2] - vBoxHalfSize[iB0] * vRotCol[2];
    } else {
      vCenter[0] = vHullBoxPos[0] - v0[0] + vBoxHalfSize[iB0] * vRotCol[0];
      vCenter[1] = vHullBoxPos[1] - v0[1] + vBoxHalfSize[iB0] * vRotCol[1];
      vCenter[2] = vHullBoxPos[2] - v0[2] + vBoxHalfSize[iB0] * vRotCol[2];
    }  

    // Here find 4 corner points of box
    dVector3 avPoints[4];

    dVector3 vRotCol2;
    GETCOL(mHullBoxRot,iB1,vRotCol);
    GETCOL(mHullBoxRot,iB2,vRotCol2);

    for(int x=0;x<3;x++) {
        avPoints[0][x] = vCenter[x] + (vBoxHalfSize[iB1] * vRotCol[x]) - (vBoxHalfSize[iB2] * vRotCol2[x]);
        avPoints[1][x] = vCenter[x] - (vBoxHalfSize[iB1] * vRotCol[x]) - (vBoxHalfSize[iB2] * vRotCol2[x]);
        avPoints[2][x] = vCenter[x] - (vBoxHalfSize[iB1] * vRotCol[x]) + (vBoxHalfSize[iB2] * vRotCol2[x]);
        avPoints[3][x] = vCenter[x] + (vBoxHalfSize[iB1] * vRotCol[x]) + (vBoxHalfSize[iB2] * vRotCol2[x]);
    }


    // clip Box face with 4 planes of triangle (1 face plane, 3 egde planes)
    dVector3 avTempArray1[9];
    dVector3 avTempArray2[9];
    dVector4 plPlane;

    int iTempCnt1=0;
    int iTempCnt2=0;

    // zeroify vectors - necessary?
    for(int i=0; i<9; i++) {
      avTempArray1[i][0]=0;
      avTempArray1[i][1]=0;
      avTempArray1[i][2]=0;

      avTempArray2[i][0]=0;
      avTempArray2[i][1]=0;
      avTempArray2[i][2]=0;
    }


    // Normal plane
    dVector3 vTemp;
    vTemp[0]=-vN[0];
    vTemp[1]=-vN[1];
    vTemp[2]=-vN[2];
    dNormalize3(vTemp);
    CONSTRUCTPLANE(plPlane,vTemp,0);

    _cldClipPolyToPlane( avPoints, 4, avTempArray1, iTempCnt1, plPlane  );
    

    // Plane p0
    dVector3 vTemp2;
    SUBTRACT(v1,v0,vTemp2);
    dCROSS(vTemp,=,vN,vTemp2);
    dNormalize3(vTemp);
    CONSTRUCTPLANE(plPlane,vTemp,0);

    _cldClipPolyToPlane( avTempArray1, iTempCnt1, avTempArray2, iTempCnt2, plPlane  );


    // Plane p1
    SUBTRACT(v2,v1,vTemp2);
    dCROSS(vTemp,=,vN,vTemp2);
    dNormalize3(vTemp);
    SUBTRACT(v0,v2,vTemp2);
    CONSTRUCTPLANE(plPlane,vTemp,dDOT(vTemp2,vTemp));

    _cldClipPolyToPlane( avTempArray2, iTempCnt2, avTempArray1, iTempCnt1, plPlane  );


    // Plane p2
    SUBTRACT(v0,v2,vTemp2);
    dCROSS(vTemp,=,vN,vTemp2);
    dNormalize3(vTemp);
    CONSTRUCTPLANE(plPlane,vTemp,0);

    _cldClipPolyToPlane( avTempArray1, iTempCnt1, avTempArray2, iTempCnt2, plPlane  );


    // END of clipping polygons



    // for each generated contact point
    for ( int i=0; i<iTempCnt2; i++ ) {
      // calculate depth
      dReal fTempDepth = dDOT(vNormal2,avTempArray2[i]);

      // clamp depth to zero
      if (fTempDepth > 0) {
        fTempDepth = 0;
      }

      dVector3 vPntTmp;
      ADD(avTempArray2[i],v0,vPntTmp);

      if(ctContacts<(iFlags & NUMC_MASK)) {
          dContactGeom* Contact = SAFECONTACT(iFlags, ContactGeoms, ctContacts, iStride);

          Contact->depth = -fTempDepth;
          SET(Contact->normal,vBestNormal);
          SET(Contact->pos,vPntTmp);
          Contact->g1 = Geom1;
          Contact->g2 = Geom2;
          ctContacts++;
      } else {
          break;
      }
    }

    //dAASSERT(ctContacts>0);

  // if box face is the referent face, then clip triangle on box face
  } else { // 2 <= if iBestAxis <= 4
    
    // get normal of box face
    dVector3 vNormal2;
    SET(vNormal2,vBestNormal);

    // get indices of box axes in correct order
    int iA0,iA1,iA2;
    iA0 = iBestAxis-2;
    if ( iA0 == 0 ) {
      iA1 = 1; iA2 = 2;
    } else if ( iA0 == 1 ) {
      iA1 = 0; iA2 = 2;
    } else {
      iA1 = 0; iA2 = 1;
    }

    dVector3 avPoints[3];
    // calculate triangle vertices in box frame
    SUBTRACT(v0,vHullBoxPos,avPoints[0]);
    SUBTRACT(v1,vHullBoxPos,avPoints[1]);
    SUBTRACT(v2,vHullBoxPos,avPoints[2]);

    // CLIP Polygons
    // define temp data for clipping
    dVector3 avTempArray1[9];
    dVector3 avTempArray2[9];
    
    int iTempCnt1, iTempCnt2;

    // zeroify vectors - necessary?
    for(int i=0; i<9; i++) {
      avTempArray1[i][0]=0;
      avTempArray1[i][1]=0;
      avTempArray1[i][2]=0;

      avTempArray2[i][0]=0;
      avTempArray2[i][1]=0;
      avTempArray2[i][2]=0;
    }

    // clip triangle with 5 box planes (1 face plane, 4 edge planes)

    dVector4 plPlane;

    // Normal plane
    dVector3 vTemp;
    vTemp[0]=-vNormal2[0];
    vTemp[1]=-vNormal2[1];
    vTemp[2]=-vNormal2[2];
    CONSTRUCTPLANE(plPlane,vTemp,vBoxHalfSize[iA0]);

    _cldClipPolyToPlane( avPoints, 3, avTempArray1, iTempCnt1, plPlane );
    

    // Plane p0
    GETCOL(mHullBoxRot,iA1,vTemp);
    CONSTRUCTPLANE(plPlane,vTemp,vBoxHalfSize[iA1]);

    _cldClipPolyToPlane( avTempArray1, iTempCnt1, avTempArray2, iTempCnt2, plPlane );


    // Plane p1
    GETCOL(mHullBoxRot,iA1,vTemp);
    vTemp[0]=-vTemp[0];
    vTemp[1]=-vTemp[1];
    vTemp[2]=-vTemp[2];
    CONSTRUCTPLANE(plPlane,vTemp,vBoxHalfSize[iA1]);

    _cldClipPolyToPlane( avTempArray2, iTempCnt2, avTempArray1, iTempCnt1, plPlane );


    // Plane p2
    GETCOL(mHullBoxRot,iA2,vTemp);
    CONSTRUCTPLANE(plPlane,vTemp,vBoxHalfSize[iA2]);

    _cldClipPolyToPlane( avTempArray1, iTempCnt1, avTempArray2, iTempCnt2, plPlane );


    // Plane p3
    GETCOL(mHullBoxRot,iA2,vTemp);
    vTemp[0]=-vTemp[0];
    vTemp[1]=-vTemp[1];
    vTemp[2]=-vTemp[2];
    CONSTRUCTPLANE(plPlane,vTemp,vBoxHalfSize[iA2]);

    _cldClipPolyToPlane( avTempArray2, iTempCnt2, avTempArray1, iTempCnt1, plPlane );


    // for each generated contact point
    for ( int i=0; i<iTempCnt1; i++ ) {
      // calculate depth
      dReal fTempDepth = dDOT(vNormal2,avTempArray1[i])-vBoxHalfSize[iA0];
      
      // clamp depth to zero
      if (fTempDepth > 0) {
        fTempDepth = 0;
      }
    
      // generate contact data
      dVector3 vPntTmp;
      ADD(avTempArray1[i],vHullBoxPos,vPntTmp);

      if(ctContacts<(iFlags & NUMC_MASK)) {
          dContactGeom* Contact = SAFECONTACT(iFlags, ContactGeoms, ctContacts, iStride);

          Contact->depth = -fTempDepth;
          SET(Contact->normal,vBestNormal);
          SET(Contact->pos,vPntTmp);
          Contact->g1 = Geom1;
          Contact->g2 = Geom2;
          ctContacts++;
      } else {
          break;
      }
    }

    //dAASSERT(ctContacts>0);
  }
  
}
#endif

// LWSS: Remove
#if 0
// box to mesh collider
int dCollideBTL(dxGeom* g1, dxGeom* BoxGeom, int Flags, dContactGeom* Contacts, int Stride){

  dxTriMesh* TriMesh = (dxTriMesh*)g1;


  // get source hull position, orientation and half size
  const dMatrix3& mRotBox=*(const dMatrix3*)dGeomGetRotation(BoxGeom);
  const dVector3& vPosBox=*(const dVector3*)dGeomGetPosition(BoxGeom);

  // to global
  SETM(mHullBoxRot,mRotBox);
  SET(vHullBoxPos,vPosBox);

  dGeomBoxGetLengths(BoxGeom, vBoxHalfSize);
  vBoxHalfSize[0] *= 0.5f;
  vBoxHalfSize[1] *= 0.5f;
  vBoxHalfSize[2] *= 0.5f;



  // get destination hull position and orientation
  const dMatrix3& mRotMesh=*(const dMatrix3*)dGeomGetRotation(TriMesh);
  const dVector3& vPosMesh=*(const dVector3*)dGeomGetPosition(TriMesh);

  // to global
  SET(vHullDstPos,vPosMesh);



  // global info for contact creation
  ctContacts = 0;
  iStride=Stride;
  iFlags=Flags;
  ContactGeoms=Contacts;
  Geom1=TriMesh;
  Geom2=BoxGeom;

 

  // reset stuff
  fBestDepth = MAXVALUE;
  vBestNormal[0]=0;
  vBestNormal[1]=0;
  vBestNormal[2]=0;

  OBBCollider& Collider = TriMesh->_OBBCollider;




  // Make OBB
  OBB Box;
  Box.mCenter.x = vPosBox[0];
  Box.mCenter.y = vPosBox[1];
  Box.mCenter.z = vPosBox[2];


  Box.mExtents.x = vBoxHalfSize[0];
  Box.mExtents.y = vBoxHalfSize[1];
  Box.mExtents.z = vBoxHalfSize[2];

  Box.mRot.m[0][0] = mRotBox[0];
  Box.mRot.m[1][0] = mRotBox[1];
  Box.mRot.m[2][0] = mRotBox[2];

  Box.mRot.m[0][1] = mRotBox[4];
  Box.mRot.m[1][1] = mRotBox[5];
  Box.mRot.m[2][1] = mRotBox[6];

  Box.mRot.m[0][2] = mRotBox[8];
  Box.mRot.m[1][2] = mRotBox[9];
  Box.mRot.m[2][2] = mRotBox[10];

  Matrix4x4 amatrix;
  Matrix4x4 BoxMatrix = MakeMatrix(vPosBox, mRotBox, amatrix);

  Matrix4x4 InvBoxMatrix;
  InvertPRMatrix(InvBoxMatrix, BoxMatrix);

  // TC results
  if (TriMesh->doBoxTC) {
	dxTriMesh::BoxTC* BoxTC = 0;
	for (int i = 0; i < TriMesh->BoxTCCache.size(); i++){
		if (TriMesh->BoxTCCache[i].Geom == BoxGeom){
			BoxTC = &TriMesh->BoxTCCache[i];
			break;
		}
	}
	if (!BoxTC){
		TriMesh->BoxTCCache.push(dxTriMesh::BoxTC());

		BoxTC = &TriMesh->BoxTCCache[TriMesh->BoxTCCache.size() - 1];
		BoxTC->Geom = BoxGeom;
		BoxTC->FatCoeff = 1.0f;
	}

	// Intersect
	Collider.SetTemporalCoherence(true);
	Collider.Collide(*BoxTC, Box, TriMesh->Data->BVTree, null, &MakeMatrix(vPosMesh, mRotMesh, amatrix));
  }
  else {
		Collider.SetTemporalCoherence(false);
		Collider.Collide(dxTriMesh::defaultBoxCache, Box, TriMesh->Data->BVTree, null, 
						 &MakeMatrix(vPosMesh, mRotMesh, amatrix));	
	}
	    
  // Retrieve data
  int TriCount = Collider.GetNbTouchedPrimitives();
  const int* Triangles = (const int*)Collider.GetTouchedPrimitives();

  if (TriCount != 0){
      if (TriMesh->ArrayCallback != null){
         TriMesh->ArrayCallback(TriMesh, BoxGeom, Triangles, TriCount);
    }
    
    //int OutTriCount = 0;
    
    // loop through all intersecting triangles
    for (int i = 0; i < TriCount; i++){
        if(ctContacts>=(iFlags & NUMC_MASK)) {
            break;
        }

        
        const int& Triint = Triangles[i];
        if (!Callback(TriMesh, BoxGeom, Triint)) continue;


        dVector3 dv[3];
        FetchTriangle(TriMesh, Triint, vPosMesh, mRotMesh, dv);


        // test this triangle
        _cldTestOneTriangle(dv[0],dv[1],dv[2]);
    }
  }


  return ctContacts;
}
#endif