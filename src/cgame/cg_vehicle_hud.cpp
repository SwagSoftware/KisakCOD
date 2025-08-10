#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_vehicle_hud.h"
#include "cg_local.h"
#include <client/client.h>
#include "cg_main.h"
#include "cg_newdraw.h"

#include <xanim/xanim.h>
#include <script/scr_const.h>
#include "cg_ents.h"

const dvar_t *vehHudReticlePipOnAStickCenterCircle;
const dvar_t *vehHudReticlePipOnAStickMovingCircle;
const dvar_t *vehHudReticlePipOnAStickCenterCircleBuffer;
const dvar_t *vehHudReticlePipOnAStickMovingCircleBuffer;
const dvar_t *vehHudLineWidth;
const dvar_t *vehHudDrawPipOnStickWhenFreelooking;
const dvar_t *vehHudTargetSize;
const dvar_t *vehHudTargetScreenEdgeClampBufferLeft;
const dvar_t *vehHudTargetScreenEdgeClampBufferRight;
const dvar_t *vehHudTargetScreenEdgeClampBufferTop;
const dvar_t *vehHudTargetScreenEdgeClampBufferBottom;
const dvar_t *vehHudReticleBouncingDiamondSize;
const dvar_t *vehHudReticleBouncingRadius;
const dvar_t *vehHudReticleBouncingSpeed;

bool __cdecl ClampScreenPosToEdges(
    const float *localClientNum,
    Material *point,
    double padLeft,
    double padRight,
    double padTop,
    double padBottom,
    float *resultNormal,
    float *resultDist,
    int a9,
    int a10,
    float *a11,
    float *a12)
{
    ScreenPlacement *v20; // r30
    double v21; // fp0
    unsigned __int8 v22; // r27
    double v23; // fp13
    double v24; // fp27
    double v25; // fp28
    double v26; // fp11
    double v27; // fp0
    double v28; // fp13
    double v29; // fp8
    double v30; // fp9
    double v31; // fp7
    double v32; // fp12
    double v33; // fp0
    double v34; // fp12
    double v35; // fp11
    double v36; // fp26
    bool v37; // r11
    bool v38; // r29
    double v39; // fp13
    bool v40; // r28
    double v41; // fp0
    Material *v42; // r4
    const float *v43; // r3
    double v44; // fp0
    double v45; // fp0
    char v46; // r11
    double v47; // fp0
    char v48; // r10
    char v49; // r11
    double v50; // fp13
    float v52; // [sp+60h] [-90h] BYREF
    float v53; // [sp+64h] [-8Ch]
    float v54; // [sp+68h] [-88h] BYREF
    float v55; // [sp+6Ch] [-84h]
    const char *name; // [sp+70h] [-80h] BYREF
    float v57; // [sp+74h] [-7Ch]

    if (!a12)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_hudelem.cpp", 1070, 0, "%s", "resultDist");
    v20 = &scrPlaceView[(unsigned int)localClientNum];
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v21 = v20->realViewportSize[0];
    v22 = 0;
    v23 = *(float *)&point->info.name;
    v24 = (float)(v20->realViewportSize[1] * (float)0.5);
    v25 = (float)(v20->realViewportSize[0] * (float)0.5);
    name = point->info.name;
    v57 = *(float *)&point->info.gameFlags;
    v26 = (float)(v57 - (float)v24);
    *(float *)&point->info.gameFlags = v57 - (float)v24;
    v27 = (float)((float)v23 - (float)((float)v21 * (float)0.5));
    *(float *)&point->info.name = v27;
    v28 = (float)((float)(v20->realViewableMin[0] + (float)padLeft) - (float)v25);
    v29 = (float)((float)(v20->realViewableMin[1] + (float)padTop) - (float)v24);
    v30 = (float)((float)v25 - (float)((float)(v20->realViewportSize[0] - v20->realViewableMax[0]) + (float)padRight));
    v31 = (float)((float)v24 - (float)((float)(v20->realViewportSize[1] - v20->realViewableMax[1]) + (float)padBottom));
    if (v27 >= v28)
    {
        if (v27 <= v30)
            goto LABEL_10;
        v32 = (float)((float)((float)v25
            - (float)((float)(v20->realViewportSize[0] - v20->realViewableMax[0]) + (float)padRight))
            / (float)v27);
    }
    else
    {
        v32 = (float)((float)((float)(v20->realViewableMin[0] + (float)padLeft) - (float)v25) / (float)v27);
    }
    *(float *)&point->info.name = (float)v27 * (float)v32;
    v22 = 1;
    *(float *)&point->info.gameFlags = (float)v26 * (float)v32;
LABEL_10:
    v33 = *(float *)&point->info.gameFlags;
    if (v33 >= v29)
    {
        if (v33 <= v31)
            goto LABEL_15;
        v34 = (float)((float)v31 / *(float *)&point->info.gameFlags);
    }
    else
    {
        v34 = (float)((float)v29 / *(float *)&point->info.gameFlags);
    }
    v35 = (float)(*(float *)&point->info.name * (float)v34);
    *(float *)&point->info.gameFlags = *(float *)&point->info.gameFlags * (float)v34;
    *(float *)&point->info.name = v35;
    v22 = 1;
LABEL_15:
    if (!waypointOffscreenRoundedCorners->current.enabled)
        goto LABEL_40;
    v36 = (float)((float)((float)(v20->scaleVirtualToReal[1] + v20->scaleVirtualToReal[0])
        * waypointOffscreenCornerRadius->current.value)
        * (float)0.5);
    v37 = *(float *)&point->info.gameFlags < 0.0;
    v38 = *(float *)&point->info.name < 0.0;
    if (*(float *)&point->info.name >= 0.0)
        v39 = (float)((float)v30
            - (float)((float)((float)(v20->scaleVirtualToReal[1] + v20->scaleVirtualToReal[0])
                * waypointOffscreenCornerRadius->current.value)
                * (float)0.5));
    else
        v39 = (float)((float)((float)((float)(v20->scaleVirtualToReal[1] + v20->scaleVirtualToReal[0])
            * waypointOffscreenCornerRadius->current.value)
            * (float)0.5)
            + (float)v28);
    v40 = *(float *)&point->info.gameFlags < 0.0;
    v52 = v39;
    if (v37)
        v41 = (float)((float)v36 + (float)v29);
    else
        v41 = (float)((float)v31 - (float)v36);
    v53 = v41;
    if (waypointDebugDraw->current.enabled)
    {
        CL_DrawStretchPicPhysical(
            (float)((float)((float)v39 + (float)v25) - (float)1.0),
            (float)((float)((float)v41 + (float)v24) - (float)1.0),
            3.0,
            3.0,
            0.0,
            0.0,
            1.0,
            1.0,
            localClientNum,
            point);
        CL_DrawStretchPicPhysical(
            (float)((float)(*(float *)&point->info.name + (float)v25) - (float)1.0),
            (float)((float)(*(float *)&point->info.gameFlags + (float)v24) - (float)1.0),
            3.0,
            3.0,
            0.0,
            0.0,
            1.0,
            1.0,
            v43,
            v42);
        v41 = v53;
        v39 = v52;
    }
    v44 = (float)(*(float *)&point->info.gameFlags - (float)v41);
    v54 = *(float *)&point->info.name - (float)v39;
    v55 = v44;
    Vec2Normalize(&v54);
    v45 = v54;
    if (v38)
    {
        if (v45 >= 0.0)
            goto LABEL_29;
    LABEL_28:
        v46 = 1;
        goto LABEL_30;
    }
    if (v45 > 0.0)
        goto LABEL_28;
LABEL_29:
    v46 = 0;
LABEL_30:
    v47 = v55;
    v48 = v46;
    if (v40)
    {
        if (v47 >= 0.0)
            goto LABEL_35;
    }
    else if (v47 <= 0.0)
    {
    LABEL_35:
        v49 = 0;
        goto LABEL_36;
    }
    v49 = 1;
LABEL_36:
    if (v48 && v49 && Vec2Distance(&v52, (const float *)point) > v36)
    {
        v22 = 1;
        *(float *)&point->info.name = (float)(v54 * (float)v36) + v52;
        *(float *)&point->info.gameFlags = (float)(v55 * (float)v36) + v53;
    }
LABEL_40:
    v50 = (float)(*(float *)&point->info.gameFlags + (float)v24);
    *(float *)&point->info.name = *(float *)&point->info.name + (float)v25;
    *(float *)&point->info.gameFlags = v50;
    if (v22)
    {
        *a12 = Vec2Distance((const float *)point, (const float *)&name);
        *a11 = *(float *)&name - *(float *)&point->info.name;
        a11[1] = v57 - *(float *)&point->info.gameFlags;
        Vec2Normalize(a11);
    }
    //return (_cntlzw(v22) & 0x20) == 0;
    return v22 != 0;
}

void __cdecl CG_VehicleHudRegisterDvars()
{
    // KISAKTODO: grab descriptions from the assembly (enjoy!)
    vehHudReticlePipOnAStickCenterCircle = (dvar_t *)Dvar_RegisterFloat(
        "vehHudReticlePipOnAStickCenterCircle",
        8.0,
        0.0099999998,
        10000.0,
        0,
        0);
    vehHudReticlePipOnAStickMovingCircle = (dvar_t *)Dvar_RegisterFloat(
        "vehHudReticlePipOnAStickMovingCircle",
        32.0,
        0.0099999998,
        10000.0,
        0,
        0);
    vehHudReticlePipOnAStickCenterCircleBuffer = (dvar_t *)Dvar_RegisterFloat(
        "vehHudReticlePipOnAStickCenterCircleBuffer",
        0.25,
        -10000.0,
        10000.0,
        0,
        0);
    vehHudReticlePipOnAStickMovingCircleBuffer = (dvar_t *)Dvar_RegisterFloat(
        "vehHudReticlePipOnAStickMovingCircleBuffer",
        1.0,
        -10000.0,
        10000.0,
        0,
        0);
    vehHudLineWidth = Dvar_RegisterFloat("vehHudLineWidth", 8.0, 0.0099999998, 10000.0, 0, 0);
    vehHudDrawPipOnStickWhenFreelooking = (dvar_t *)Dvar_RegisterBool(
        "vehHudDrawPipOnStickWhenFreelooking",
        1,
        0,
        "Set to 0 to not draw the pip-on-a-stick reticle when the player is freelooking");
    vehHudTargetSize = Dvar_RegisterFloat("vehHudTargetSize", 30.0, 0.0099999998, 10000.0, 0, 0);
    vehHudTargetScreenEdgeClampBufferLeft = (dvar_t *)Dvar_RegisterFloat(
        "vehHudTargetScreenEdgeClampBufferLeft",
        0.0,
        -10000.0,
        10000.0,
        0,
        0);
    vehHudTargetScreenEdgeClampBufferRight = (dvar_t *)Dvar_RegisterFloat(
        "vehHudTargetScreenEdgeClampBufferRight",
        0.0,
        -10000.0,
        10000.0,
        0,
        0);
    vehHudTargetScreenEdgeClampBufferTop = Dvar_RegisterFloat(
        "vehHudTargetScreenEdgeClampBufferTop",
        0.0,
        -10000.0,
        10000.0,
        0,
        0);
    vehHudTargetScreenEdgeClampBufferBottom = (dvar_t *)Dvar_RegisterFloat(
        "vehHudTargetScreenEdgeClampBufferBottom",
        0.0,
        -10000.0,
        10000.0,
        0,
        0);
    vehHudReticleBouncingDiamondSize = Dvar_RegisterFloat(
        "vehHudReticleBouncingDiamondSize",
        24.0,
        0.0,
        1000000.0,
        0,
        0);
    vehHudReticleBouncingRadius = (dvar_t *)Dvar_RegisterFloat(
        "vehHudReticleBouncingRadius",
        80.0,
        0.0,
        1000000.0,
        0,
        0);
    vehHudReticleBouncingSpeed = (dvar_t *)Dvar_RegisterFloat(
        "vehHudReticleBouncingSpeed",
        400.0,
        0.0,
        1000000.0,
        0,
        0);
}

int __cdecl WorldDirToScreenPos(int localClientNum, const float *worldDir, float *outScreenPos)
{
    double v5; // fp28
    double v6; // fp30
    double v7; // fp29
    int result; // r3
    double v9; // fp0
    double v10; // fp13
    double v11; // fp0
    double v12; // fp13
    float v13; // [sp+50h] [-80h] BYREF
    float v14; // [sp+54h] [-7Ch]
    float v15; // [sp+58h] [-78h]
    float v16[6][3]; // [sp+60h] [-70h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    AnglesToAxis(cgArray[0].refdefViewAngles, v16);
    MatrixTransposeTransformVector(worldDir, (const mat3x3&)v16, &v13);
    v5 = (float)(cls.vidConfig.aspectRatioWindow * (float)480.0);
    if (v13 <= 0.0)
    {
        v9 = -v14;
        v10 = -v15;
        *outScreenPos = v9;
        outScreenPos[1] = v10;
        if (v9 == 0.0 && v10 == 0.0)
        {
            result = 0;
            outScreenPos[1] = 960.0;
        }
        else
        {
            if (I_fabs(v9) < v5)
            {
                do
                {
                    v11 = (float)(*outScreenPos * (float)v5);
                    *outScreenPos = *outScreenPos * (float)v5;
                    outScreenPos[1] = outScreenPos[1] * (float)v5;
                } while (I_fabs(v11) < v5);
            }
            if (I_fabs(outScreenPos[1]) < 480.0)
            {
                do
                {
                    v12 = (float)(outScreenPos[1] * (float)480.0);
                    *outScreenPos = *outScreenPos * (float)480.0;
                    outScreenPos[1] = v12;
                } while (I_fabs(v12) < 480.0);
            }
            return 0;
        }
    }
    else
    {
        v6 = (float)((float)((float)1.0 / v13) * v14);
        v7 = (float)((float)((float)1.0 / v13) * v15);
        if (cgArray[0].refdef.tanHalfFovX == 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp",
                78,
                0,
                "%s",
                "cgameGlob->refdef.tanHalfFovX");
        if (cgArray[0].refdef.tanHalfFovY == 0.0)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp",
                79,
                0,
                "%s",
                "cgameGlob->refdef.tanHalfFovY");
        result = 1;
        *outScreenPos = (float)((float)((float)v6 / cgArray[0].refdef.tanHalfFovX) * (float)v5) * (float)-0.5;
        outScreenPos[1] = (float)((float)v7 / cgArray[0].refdef.tanHalfFovY) * (float)-240.0;
    }
    return result;
}

static Clip_t ClampScreenPosToEdges_0(int localClientNum, float *point)
{
    int v4; // r10
    double value; // fp9
    double v6; // fp10
    double v7; // fp13
    double v8; // fp12
    double v9; // fp8
    double v10; // fp0
    double v11; // fp12
    double v12; // fp0
    Clip_t result; // r3
    double v14; // fp9
    double v15; // fp10
    double v16; // fp13
    double v17; // fp12
    double v18; // fp0
    double v19; // fp13
    double v20; // fp12

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    value = vehHudTargetScreenEdgeClampBufferTop->current.value;
    v6 = vehHudTargetScreenEdgeClampBufferBottom->current.value;
    v7 = scrPlaceView[localClientNum].virtualViewableMin[0];
    v8 = (float)(vehHudTargetScreenEdgeClampBufferLeft->current.value
        + (float)(scrPlaceView[localClientNum].scaleRealToVirtual[0] * scrPlaceView[localClientNum].subScreenLeft));
    v9 = (float)(vehHudTargetScreenEdgeClampBufferRight->current.value
        + (float)(scrPlaceView[localClientNum].scaleRealToVirtual[0] * scrPlaceView[localClientNum].subScreenLeft));
    if (v8 < v7)
        v8 = scrPlaceView[localClientNum].virtualViewableMin[0];
    if (v9 < v7)
        v9 = scrPlaceView[localClientNum].virtualViewableMin[0];
    v4 = localClientNum;
    v10 = scrPlaceView[localClientNum].virtualViewableMin[1];
    if (value < v10)
        value = scrPlaceView[v4].virtualViewableMin[1];
    if (v6 < v10)
        v6 = scrPlaceView[v4].virtualViewableMin[1];
    v11 = (float)((float)v8 - (float)((float)(cls.vidConfig.aspectRatioWindow * (float)480.0) * (float)0.5));
    v12 = *point;
    result = CLIP_NONE;
    v14 = (float)((float)value - (float)240.0);
    v15 = (float)((float)240.0 - (float)v6);
    if (v12 >= v11)
    {
        if (v12 <= (float)((float)((float)(cls.vidConfig.aspectRatioWindow * (float)480.0) * (float)0.5) - (float)v9))
            goto LABEL_16;
        v16 = (float)((float)((float)((float)(cls.vidConfig.aspectRatioWindow * (float)480.0) * (float)0.5) - (float)v9)
            / *point);
        result = CLIP_RIGHT;
    }
    else
    {
        v16 = (float)((float)v11 / *point);
        result = CLIP_LEFT;
    }
    v17 = point[1];
    *point = *point * (float)v16;
    point[1] = (float)v17 * (float)v16;
LABEL_16:
    v18 = point[1];
    if (v18 >= v14)
    {
        if (v18 <= v15)
            return result;
        v19 = (float)((float)v15 / point[1]);
        result = CLIP_BOTTOM;
    }
    else
    {
        v19 = (float)((float)v14 / point[1]);
        result = CLIP_TOP;
    }
    v20 = (float)(*point * (float)v19);
    point[1] = point[1] * (float)v19;
    *point = v20;
    return result;
}

void __cdecl CG_DrawVehicleTargets(int localClientNum, rectDef_s *rect, float *color, Material *defaultMaterial)
{
    float *v6; // r29
    centity_s *Entity; // r31
    double v8; // fp12
    Clip_t v9; // r31
    Material *v10; // r7
    const float *v11; // r6
    int v12; // r5
    int v13; // r4
    double value; // fp4
    double v15; // fp0
    double v16; // fp5
    float v17; // [sp+8h] [-178h]
    float v18; // [sp+10h] [-170h]
    float v19; // [sp+18h] [-168h]
    float v20; // [sp+20h] [-160h]
    float v21; // [sp+28h] [-158h]
    float v22; // [sp+30h] [-150h]
    float v23; // [sp+38h] [-148h]
    float v24; // [sp+40h] [-140h]
    float v25; // [sp+48h] [-138h]
    float v26; // [sp+50h] [-130h]
    float v27; // [sp+58h] [-128h]
    float v28; // [sp+60h] [-120h]
    float v29; // [sp+80h] [-100h] BYREF
    float v30; // [sp+84h] [-FCh]
    float v31; // [sp+88h] [-F8h]
    float v32; // [sp+90h] [-F0h] BYREF
    float v33; // [sp+94h] [-ECh]
    char v34[120]; // [sp+A0h] [-E0h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v6 = &cgArray[0].targets[0].offset[2];
    do
    {
        if (*((unsigned int *)v6 - 3) != ENTITYNUM_NONE && (((unsigned int)v6[3] & 2) == 0 || CG_JavelinADS(localClientNum)))
        {
            Entity = CG_GetEntity(localClientNum, *((unsigned int *)v6 - 3));
            if (!Entity)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp", 209, 0, "%s", "targetEnt");
            v29 = *(v6 - 2) + Entity->pose.origin[0];
            v30 = *(v6 - 1) + Entity->pose.origin[1];
            v8 = (float)(Entity->pose.origin[2] + *v6);
            v29 = v29 - cgArray[0].refdef.vieworg[0];
            v30 = v30 - cgArray[0].refdef.vieworg[1];
            v31 = (float)v8 - cgArray[0].refdef.vieworg[2];
            WorldDirToScreenPos(localClientNum, &v29, &v32);
            v9 = ClampScreenPosToEdges_0(localClientNum, &v32);
            if (v9 && (v13 = *((unsigned int *)v6 + 2), v13 != -1) || (v13 = *((unsigned int *)v6 + 1), v13 != -1))
            {
                if (CG_ServerMaterialName(localClientNum, v13, v34, 0x40u))
                    Material_RegisterHandle(v34, 7);
            }
            value = vehHudTargetSize->current.value;
            v15 = (float)(vehHudTargetSize->current.value * (float)0.5);
            if (v9 == CLIP_NONE || *((unsigned int *)v6 + 2) == -1)
            {
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    (float)(v32 - (float)v15),
                    (float)(v33 - (float)v15),
                    value,
                    value,
                    v13,
                    v12,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    v11,
                    v10,
                    rect->horzAlign,
                    rect->vertAlign,
                    v17,
                    v18,
                    v19,
                    v20,
                    v21,
                    v22,
                    v23,
                    v24,
                    v25,
                    v26,
                    v27,
                    v28);
            }
            else
            {
                v16 = 0.0;
                switch (v9)
                {
                case CLIP_BOTTOM:
                    v16 = 180.0;
                    break;
                case CLIP_RIGHT:
                    v16 = 90.0;
                    break;
                case CLIP_LEFT:
                    v16 = 270.0;
                    break;
                }
                CG_DrawRotatedPic(
                    &scrPlaceView[localClientNum],
                    (float)(v32 - (float)v15),
                    (float)(v33 - (float)v15),
                    value,
                    value,
                    v13,
                    v12,
                    v16,
                    v11,
                    v10);
            }
        }
        v6 += 7;
    } while ((int)v6 < (int)&cgArray[0].shellshock.loopEndTime);
}

void __cdecl CG_DrawJavelinTargets(int localClientNum, rectDef_s *rect, float *color, Material *defaultMaterial)
{
    const playerState_s *PredictedPlayerState; // r3
    const playerState_s *v7; // r30
    int weapLockedEntnum; // r4
    int v9; // r29
    targetInfo_t *v10; // r11
    centity_s *Entity; // r30
    float *offset; // r11
    double v13; // fp12
    double v14; // fp0
    double v15; // fp10
    double v16; // fp11
    const float *v17; // r6
    int v18; // r5
    int v19; // r4
    Material *v20; // r7
    const float *v21; // r6
    int v22; // r5
    int v23; // r4
    float v24; // [sp+8h] [-108h]
    float v25; // [sp+8h] [-108h]
    float v26; // [sp+10h] [-100h]
    float v27; // [sp+10h] [-100h]
    float v28; // [sp+18h] [-F8h]
    float v29; // [sp+18h] [-F8h]
    float v30; // [sp+20h] [-F0h]
    float v31; // [sp+20h] [-F0h]
    float v32; // [sp+28h] [-E8h]
    float v33; // [sp+28h] [-E8h]
    float v34; // [sp+30h] [-E0h]
    float v35; // [sp+30h] [-E0h]
    float v36; // [sp+38h] [-D8h]
    float v37; // [sp+38h] [-D8h]
    float v38; // [sp+40h] [-D0h]
    float v39; // [sp+40h] [-D0h]
    float v40; // [sp+48h] [-C8h]
    float v41; // [sp+48h] [-C8h]
    float v42; // [sp+50h] [-C0h]
    float v43; // [sp+50h] [-C0h]
    float v44; // [sp+58h] [-B8h]
    float v45; // [sp+58h] [-B8h]
    float v46; // [sp+60h] [-B0h]
    float v47; // [sp+60h] [-B0h]
    float v48; // [sp+80h] [-90h] BYREF
    float v49; // [sp+84h] [-8Ch]
    float v50; // [sp+88h] [-88h]
    float v51[6]; // [sp+90h] [-80h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    PredictedPlayerState = CG_GetPredictedPlayerState(localClientNum);
    v7 = PredictedPlayerState;
    if ((PredictedPlayerState->weapLockFlags & 2) != 0)
    {
        if (PredictedPlayerState->weapLockedEntnum == ENTITYNUM_NONE)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp",
                271,
                0,
                "%s",
                "ps->weapLockedEntnum != ENTITYNUM_NONE");
        weapLockedEntnum = v7->weapLockedEntnum;
        v9 = 0;
        v10 = &cgArray[0].targets[1];
        while (v10[-1].entNum != weapLockedEntnum)
        {
            if (v10->entNum == weapLockedEntnum)
            {
                ++v9;
                break;
            }
            if (v10[1].entNum == weapLockedEntnum)
            {
                v9 += 2;
                break;
            }
            if (v10[2].entNum == weapLockedEntnum)
            {
                v9 += 3;
                break;
            }
            v10 += 4;
            v9 += 4;
            if ((int)v10 >= (int)&cgArray[0].shellshock.hasSavedScreen)
                break;
        }
        if (v9 != 32)
        {
            Entity = CG_GetEntity(localClientNum, weapLockedEntnum);
            if (!Entity)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp", 283, 0, "%s", "targetEnt");
            offset = cgArray[0].targets[v9].offset;
            v13 = offset[1];
            v14 = (float)(*offset + Entity->pose.origin[0]);
            v15 = offset[2];
            v48 = *offset + Entity->pose.origin[0];
            v49 = Entity->pose.origin[1] + (float)v13;
            v16 = Entity->pose.origin[2];
            v48 = (float)v14 - cgArray[0].refdef.vieworg[0];
            v49 = v49 - cgArray[0].refdef.vieworg[1];
            v50 = (float)((float)v16 + (float)v15) - cgArray[0].refdef.vieworg[2];
            WorldDirToScreenPos(localClientNum, &v48, v51);
            if (ClampScreenPosToEdges_0(localClientNum, v51) == CLIP_NONE)
            {
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    v51[0],
                    -240.0,
                    2.0,
                    480.0,
                    v19,
                    v18,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    v17,
                    (Material *)0x82000000,
                    rect->horzAlign,
                    rect->vertAlign,
                    v24,
                    v26,
                    v28,
                    v30,
                    v32,
                    v34,
                    v36,
                    v38,
                    v40,
                    v42,
                    v44,
                    v46);
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    -320.0,
                    v51[1],
                    640.0,
                    2.0,
                    v23,
                    v22,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    v21,
                    v20,
                    rect->horzAlign,
                    rect->vertAlign,
                    v25,
                    v27,
                    v29,
                    v31,
                    v33,
                    v35,
                    v37,
                    v39,
                    v41,
                    v43,
                    v45,
                    v47);
            }
        }
    }
}

void CG_DrawPipOnAStickReticle(int localClientNum, rectDef_s *rect, float *color)
{
    centity_s *Entity; // r31
    DObj_s *ClientDObj; // r29
    WeaponDef *WeaponDef; // r25
    __int64 v7; // r11
    double v8; // fp12
    double v9; // fp0
    double v10; // fp28
    double v11; // fp27
    double v12; // fp26
    double v13; // fp25
    Material *v14; // r7
    const float *v15; // r6
    int v16; // r5
    int v17; // r4
    double v18; // fp1
    const dvar_t *v19; // r11
    double v20; // fp31
    double v21; // fp30
    Material *v22; // r7
    const float *v23; // r6
    int v24; // r5
    int v25; // r4
    float v26; // [sp+80h] [-F0h] BYREF
    float v27; // [sp+84h] [-ECh]
    __int64 v28; // [sp+88h] [-E8h] BYREF
    float v29; // [sp+90h] [-E0h] BYREF
    float v30; // [sp+94h] [-DCh]
    float v31; // [sp+98h] [-D8h] BYREF
    float v32; // [sp+9Ch] [-D4h]
    float v33; // [sp+A0h] [-D0h]
    float v34[9]; // [sp+B0h] [-C0h] BYREF
    float v35[21]; // [sp+D4h] [-9Ch] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    Entity = CG_GetEntity(localClientNum, cgArray[0].predictedPlayerState.viewlocked_entNum);
    ClientDObj = Com_GetClientDObj(Entity->nextState.number, 0);
    WeaponDef = BG_GetWeaponDef(Entity->nextState.weapon);
    if (ClientDObj
        && (vehHudDrawPipOnStickWhenFreelooking->current.value != 0.0
            || (cgArray[0].predictedPlayerState.eFlags & 0x40000) != 0))
    {
        if (CG_DObjGetWorldTagMatrix(&Entity->pose, ClientDObj, scr_const.tag_body, (float (*)[3])v34, v35))
        {
            WorldDirToScreenPos(localClientNum, v34, &v29);
            LODWORD(v7) = WeaponDef->iProjectileSpeed;
            HIDWORD(v7) = 108032;
            v28 = v7;
            v31 = v34[0] * (float)v7;
            v32 = (float)v7 * v34[1];
            v33 = v34[2] * (float)v7;
            v8 = (float)((float)((float)(Entity->currentState.pos.trDelta[1] - Entity->currentState.pos.trDelta[1])
                * cgArray[0].frameInterpolation)
                + Entity->currentState.pos.trDelta[1]);
            v9 = (float)((float)((float)(Entity->currentState.pos.trDelta[2] - Entity->currentState.pos.trDelta[2])
                * cgArray[0].frameInterpolation)
                + Entity->currentState.pos.trDelta[2]);
            v31 = (float)((float)((float)(Entity->currentState.pos.trDelta[0] - Entity->currentState.pos.trDelta[0])
                * cgArray[0].frameInterpolation)
                + Entity->currentState.pos.trDelta[0])
                + v31;
            v32 = (float)v8 + v32;
            v33 = (float)v9 + v33;
            if (WorldDirToScreenPos(localClientNum, &v31, (float *)&v28))
            {
                if (!cgMedia.vehCenterCircle)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp",
                        341,
                        0,
                        "%s",
                        "cgMedia.vehCenterCircle");
                if (!cgMedia.vehMovingCircle)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp",
                        342,
                        0,
                        "%s",
                        "cgMedia.vehMovingCircle");
                if (!cgMedia.vehHudLine)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp",
                        343,
                        0,
                        "%s",
                        "cgMedia.vehHudLine");
                v10 = *(float *)&v28;
                v11 = v29;
                v12 = *((float *)&v28 + 1);
                v13 = v30;
                v26 = *(float *)&v28 - v29;
                v27 = *((float *)&v28 + 1) - v30;
                v18 = Vec2Normalize(&v26);
                v19 = vehHudReticlePipOnAStickCenterCircle;
                v20 = (float)(vehHudReticlePipOnAStickMovingCircle->current.value * (float)0.5);
                v21 = (float)(vehHudReticlePipOnAStickCenterCircle->current.value * (float)0.5);
                if (v18 > (float)((float)((float)((float)(vehHudReticlePipOnAStickCenterCircle->current.value * (float)0.5)
                    + (float)(vehHudReticlePipOnAStickMovingCircle->current.value * (float)0.5))
                    - vehHudReticlePipOnAStickCenterCircleBuffer->current.value)
                    - vehHudReticlePipOnAStickMovingCircleBuffer->current.value))
                {
                    CG_Draw2DLine(
                        &scrPlaceView[localClientNum],
                        (float)((float)(v26
                            * (float)((float)(vehHudReticlePipOnAStickCenterCircle->current.value * (float)0.5)
                                - vehHudReticlePipOnAStickCenterCircleBuffer->current.value))
                            + (float)v11),
                        (float)((float)(v27
                            * (float)((float)(vehHudReticlePipOnAStickCenterCircle->current.value * (float)0.5)
                                - vehHudReticlePipOnAStickCenterCircleBuffer->current.value))
                            + (float)v13),
                        (float)((float)((float)(vehHudReticlePipOnAStickMovingCircleBuffer->current.value
                            - (float)(vehHudReticlePipOnAStickMovingCircle->current.value * (float)0.5))
                            * v26)
                            + (float)v10),
                        (float)((float)(v27
                            * (float)(vehHudReticlePipOnAStickMovingCircleBuffer->current.value
                                - (float)(vehHudReticlePipOnAStickMovingCircle->current.value * (float)0.5)))
                            + (float)v12),
                        vehHudLineWidth->current.value,
                        v17,
                        v16,
                        v15,
                        v14);
                    v19 = vehHudReticlePipOnAStickCenterCircle;
                }
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    (float)((float)v11 - (float)v21),
                    (float)((float)v13 - (float)v21),
                    v19->current.value,
                    v19->current.value,
                    v17,
                    v16,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    v15,
                    v14);
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    (float)((float)v10 - (float)v20),
                    (float)((float)v12 - (float)v20),
                    vehHudReticlePipOnAStickMovingCircle->current.value,
                    vehHudReticlePipOnAStickMovingCircle->current.value,
                    v25,
                    v24,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    v23,
                    v22);
            }
        }
    }
}

void __cdecl CG_InitVehicleReticle(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgArray[0].vehReticleLockOnEntNum = ENTITYNUM_NONE;
    cgArray[0].vehReticleOffset[0] = 0.0;
    cgArray[0].vehReticleOffset[1] = 0.0;
    cgArray[0].vehReticleVel[0] = 0.0;
    cgArray[0].vehReticleVel[1] = 0.0;
}

void __cdecl CG_ReticleStartLockOn(int localClientNum, int targetEntNum, int msecDuration)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgArray[0].vehReticleLockOnEntNum = targetEntNum;
    cgArray[0].vehReticleLockOnStartTime = cgArray[0].time;
    cgArray[0].vehReticleLockOnDuration = msecDuration;
}

int __cdecl CG_GetTargetPos(int localClientNum, int targetEntNum, float *outPos)
{
    int v6; // r30
    targetInfo_t *targets; // r11
    int result; // r3
    centity_s *Entity; // r31
    float *offset; // r11

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v6 = 0;
    targets = cgArray[0].targets;
    while (targets->entNum != targetEntNum)
    {
        ++targets;
        ++v6;
        if ((int)targets >= (int)&cgArray[0].shellshock)
            return 0;
    }
    Entity = CG_GetEntity(localClientNum, targetEntNum);
    if (!Entity)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp", 403, 0, "%s", "targetEnt");
    result = 1;
    offset = cgArray[0].targets[v6].offset;
    *outPos = Entity->pose.origin[0] + *offset;
    outPos[1] = Entity->pose.origin[1] + offset[1];
    outPos[2] = Entity->pose.origin[2] + offset[2];
    return result;
}

// local variable allocation has failed, the output may be wrong!
void __fastcall CG_DrawBouncingDiamond(int localClientNum, rectDef_s *rect, float *color)
{
    double value; // fp29
    int vehReticleLockOnEntNum; // r4
    centity_s *Entity; // r30
    double v8; // fp13
    __int64 v9; // r10 OVERLAPPED
    double v10; // fp28
    int v11; // r11
    double v16; // fp0
    double v21; // fp12
    double v22; // fp11
    double v23; // fp13
    double v24; // fp0
    double v25; // fp13
    double v26; // fp0
    unsigned __int8 v27; // r11
    double v28; // fp31
    double v29; // fp30
    double v31; // fp29
    long double v32; // fp2
    long double v33; // fp2
    double v34; // fp0
    double v35; // fp29
    long double v36; // fp2
    double v37; // fp10
    double v38; // fp11
    double v39; // fp12
    double v40; // fp9
    double v41; // fp11
    double v42; // fp10
    float v43[2]; // [sp+80h] [-90h] BYREF
    __int64 v44; // [sp+88h] [-88h]
    float v46; // [sp+98h] [-78h] BYREF
    float v47; // [sp+9Ch] [-74h]
    float v48; // [sp+A0h] [-70h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    value = vehHudReticleBouncingRadius->current.value;
    vehReticleLockOnEntNum = cgArray[0].vehReticleLockOnEntNum;
    if (cgArray[0].vehReticleLockOnEntNum == 2175)
    {
        v10 = 0.0;
        v21 = 0.0;
        v22 = 0.0;
    }
    else
    {
        if (CG_GetTargetPos(localClientNum, cgArray[0].vehReticleLockOnEntNum, &v46))
        {
            v8 = v48;
        }
        else
        {
            Entity = CG_GetEntity(localClientNum, cgArray[0].vehReticleLockOnEntNum);
            if (!Entity)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp", 442, 0, "%s", "targetEnt");
            v46 = Entity->pose.origin[0];
            v47 = Entity->pose.origin[1];
            v8 = Entity->pose.origin[2];
        }
        v46 = v46 - cgArray[0].refdef.vieworg[0];
        v47 = v47 - cgArray[0].refdef.vieworg[1];
        v48 = (float)v8 - cgArray[0].refdef.vieworg[2];
        WorldDirToScreenPos(localClientNum, &v46, v43);
        LODWORD(v9) = cgArray[0].vehReticleLockOnDuration;
        v10 = 0.0;
        if (cgArray[0].vehReticleLockOnDuration)
        {
            HIDWORD(v9) = cgArray[0].time;
            v44 = v9;
            v11 = cgArray[0].time - cgArray[0].vehReticleLockOnStartTime;
            v45 = *(__int64 *)((char *)&v9 - 4);
            _FP13 = (float)((float)((float)*(__int64 *)((char *)&v9 - 4) / (float)v9) - (float)1.0);
            _FP12 = -(float)((float)*(__int64 *)((char *)&v9 - 4) / (float)v9);
            __asm
            {
                fsel      f0, f13, f27, f0
                fsel      f13, f12, f28, f0
            }
        }
        else
        {
            _FP13 = 1.0;
        }
        value = (float)-(float)((float)((float)_FP13 * (float)value) - (float)value);
        v16 = (float)((float)_FP13 * (float)2.0);
        _FP13 = (float)((float)((float)_FP13 * (float)2.0) - (float)1.0);
        _FP10 = -v16;
        __asm
        {
            fsel      f0, f13, f27, f0
            fsel      f0, f10, f28, f0
        }
        v21 = (float)(v43[0] * (float)_FP0);
        v22 = (float)(v43[1] * (float)_FP0);
    }
    if (value == v10)
    {
        v23 = v21;
        v24 = v22;
        cgArray[0].vehReticleOffset[0] = v21;
    }
    else
    {
        v25 = cgArray[0].vehReticleVel[0];
        v26 = cgArray[0].vehReticleVel[1];
        if (cgArray[0].vehReticleVel[0] != v10 || (v27 = 1, v26 != v10))
            v27 = 0;
        v28 = (float)(cgArray[0].vehReticleOffset[1] - (float)v22);
        v29 = (float)(cgArray[0].vehReticleOffset[0] - (float)v21);
        HIDWORD(v30) = v27;
        if (v27
            || (float)((float)((float)(cgArray[0].vehReticleOffset[1] - (float)v22)
                * (float)(cgArray[0].vehReticleOffset[1] - (float)v22))
                + (float)((float)(cgArray[0].vehReticleOffset[0] - (float)v21)
                    * (float)(cgArray[0].vehReticleOffset[0] - (float)v21))) > (double)(float)((float)value
                        * (float)value))
        {
            v31 = (float)((float)(random() * (float)360.0) * (float)0.017453292);
            *(double *)&v32 = v31;
            v33 = sin(v32);
            v34 = *(double *)&v33;
            *(double *)&v33 = v31;
            v35 = (float)v34;
            v36 = cos(v33);
            cgArray[0].vehReticleVel[0] = v35;
            cgArray[0].vehReticleVel[1] = *(double *)&v36;
            v26 = (float)(vehHudReticleBouncingSpeed->current.value * (float)*(double *)&v36);
            v25 = (float)(vehHudReticleBouncingSpeed->current.value * (float)v35);
            v37 = -v26;
            v38 = v25;
            cgArray[0].vehReticleVel[0] = vehHudReticleBouncingSpeed->current.value * (float)v35;
            cgArray[0].vehReticleVel[1] = v26;
            v39 = (float)((float)((float)v28 * (float)v26) + (float)((float)v29 * (float)v25));
            v40 = (float)((float)((float)v28 * (float)v25) + (float)((float)v29 * (float)-v26));
            if (v40 < v39)
            {
                v25 = -v26;
                v26 = v38;
                v39 = v40;
                cgArray[0].vehReticleVel[0] = v37;
                cgArray[0].vehReticleVel[1] = v38;
            }
            v41 = -v38;
            if ((float)((float)((float)v28 * (float)v37) + (float)((float)v29 * (float)v41)) < v39)
            {
                v25 = v41;
                v26 = v37;
                v39 = (float)((float)((float)v28 * (float)v37) + (float)((float)v29 * (float)v41));
                cgArray[0].vehReticleVel[0] = v41;
                cgArray[0].vehReticleVel[1] = v37;
            }
            v42 = -v37;
            if ((float)((float)((float)v28 * (float)v41) + (float)((float)v29 * (float)v42)) < v39)
            {
                v25 = v42;
                v26 = v41;
                cgArray[0].vehReticleVel[0] = v42;
                cgArray[0].vehReticleVel[1] = v41;
            }
        }
        //LODWORD(v30) = cgArray[0].frametime;
        //v45 = v30;
        v23 = (float)((float)((float)v25 * (float)((float)cgArray[0].frametime * (float)0.001)) + cgArray[0].vehReticleOffset[0]);
        cgArray[0].vehReticleOffset[0] = v23;
        v24 = (float)((float)((float)v26 * (float)((float)cgArray[0].frametime * (float)0.001)) + cgArray[0].vehReticleOffset[1]);
    }
    cgArray[0].vehReticleOffset[1] = v24;
    CL_DrawStretchPic(
        &scrPlaceView[localClientNum],
        (float)((float)v23 - (float)(vehHudReticleBouncingDiamondSize->current.value * 0.5f)),
        (float)((float)v24 - (float)(vehHudReticleBouncingDiamondSize->current.value * 0.5f)),
        vehHudReticleBouncingDiamondSize->current.value,
        vehHudReticleBouncingDiamondSize->current.value,
        vehReticleLockOnEntNum,
        (int)color,
        v10,
        v10,
        1.0,
        1.0,
        a4,
        (Material *)(68 * localClientNum));
}

void __cdecl CG_DrawVehicleReticle(int localClientNum, rectDef_s *rect, float *color)
{
    centity_s *vehicle; // r30
    activeReticleType_t activeReticleType; // r11

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((cgArray[0].predictedPlayerState.eFlags & 0x20000) != 0
        && cgArray[0].predictedPlayerState.viewlocked_entNum != ENTITYNUM_NONE)
    {
        vehicle = CG_GetEntity(localClientNum, cgArray[0].predictedPlayerState.viewlocked_entNum);
        if (!vehicle)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp", 521, 0, "%s", "vehicle");
        if (vehicle->nextState.weapon)
        {
            activeReticleType = BG_GetWeaponDef(vehicle->nextState.weapon)->activeReticleType;
            if (activeReticleType == VEH_ACTIVE_RETICLE_PIP_ON_A_STICK)
            {
                CG_DrawPipOnAStickReticle(localClientNum, rect, color);
            }
            else if (activeReticleType == VEH_ACTIVE_RETICLE_BOUNCING_DIAMOND)
            {
                CG_DrawBouncingDiamond(localClientNum, rect, color);
            }
        }
    }
}

void __cdecl CG_TargetsChanged(int localClientNum, unsigned int num)
{
    const char *ConfigString; // r3
    unsigned int v5; // r31
    const char *v6; // r30
    bool v7; // zf
    targetInfo_t *v8; // r31
    const char *v9; // r3
    const char *v10; // r3
    const char *v11; // r3
    int v12; // r10
    const char *v13; // r3
    int v14; // r10
    const char *v15; // r3
    int v16; // r3

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    ConfigString = CL_GetConfigString(localClientNum, num);
    v5 = num - 27;
    v7 = num - 27 < 0x20;
    v6 = ConfigString;
    if (!v7)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp",
            552,
            0,
            "targetIndex doesn't index MAX_TARGETS\n\t%i not in [0, %i)",
            v5,
            32);
    v8 = &cgArray[0].targets[v5];
    if (*v6)
    {
        v9 = Info_ValueForKey(v6, "ent");
        if (*v9)
            v8->entNum = atol(v9);
        else
            v8->entNum = ENTITYNUM_NONE;
        v10 = Info_ValueForKey(v6, "offs");
        v8->offset[0] = 0.0;
        v8->offset[1] = 0.0;
        v8->offset[2] = 0.0;
        if (*v10)
            sscanf(v10, "%f %f %f", v8->offset, &v8->offset[1], &v8->offset[2]);
        v11 = Info_ValueForKey(v6, "mat");
        if (*v11)
            v12 = atol(v11);
        else
            v12 = -1;
        v8->materialIndex = v12;
        v13 = Info_ValueForKey(v6, "offmat");
        if (*v13)
            v14 = atol(v13);
        else
            v14 = -1;
        v8->offscreenMaterialIndex = v14;
        v15 = Info_ValueForKey(v6, "flags");
        if (*v15)
            v16 = atol(v15);
        else
            v16 = 0;
        v8->flags = v16;
    }
    else
    {
        v8->entNum = ENTITYNUM_NONE;
    }
}

