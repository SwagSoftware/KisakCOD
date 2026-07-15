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
            colorYellow,
            cgMedia.whiteMaterial);
        CL_DrawStretchPicPhysical(
            (float)((float)(*(float *)&point->info.name + (float)v25) - (float)1.0),
            (float)((float)(*(float *)&point->info.gameFlags + (float)v24) - (float)1.0),
            3.0,
            3.0,
            0.0,
            0.0,
            1.0,
            1.0,
            colorGreen,
            cgMedia.whiteMaterial);
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
    float transformed[3]; // [sp+50h] [-80h] BYREF
    //float v14; // [sp+54h] [-7Ch]
    //float v15; // [sp+58h] [-78h]
    float axis[6][3]; // [sp+60h] [-70h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    AnglesToAxis(cgArray[0].refdefViewAngles, axis);
    MatrixTransposeTransformVector(worldDir, (const mat3x3&)axis, transformed);
    v5 = (float)(cls.vidConfig.aspectRatioWindow * (float)480.0);
    if (transformed[0] <= 0.0)
    {
        v9 = -transformed[1];
        v10 = -transformed[2];
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
        v6 = (float)((float)((float)1.0 / transformed[0]) * transformed[1]);
        v7 = (float)((float)((float)1.0 / transformed[0]) * transformed[2]);
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
    int targetIndex;

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);

    for (targetIndex = 0; targetIndex < ARRAY_COUNT(cgArray[0].targets); ++targetIndex)
    {
        targetInfo_t *target = &cgArray[0].targets[targetIndex];
        centity_s *targetEnt;
        Material *material;
        Clip_t clip;
        int materialIndex;
        float worldDir[3];
        float screenPos[2];
        float halfSize;
        float angle;
        char materialName[64];

        if (target->entNum == ENTITYNUM_NONE)
            continue;
        if ((target->flags & 2) != 0 && !CG_JavelinADS(localClientNum))
            continue;

        targetEnt = CG_GetEntity(localClientNum, target->entNum);
        if (!targetEnt)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp", 209, 0, "%s", "targetEnt");

        worldDir[0] = targetEnt->pose.origin[0] + target->offset[0] - cgArray[0].refdef.vieworg[0];
        worldDir[1] = targetEnt->pose.origin[1] + target->offset[1] - cgArray[0].refdef.vieworg[1];
        worldDir[2] = targetEnt->pose.origin[2] + target->offset[2] - cgArray[0].refdef.vieworg[2];
        WorldDirToScreenPos(localClientNum, worldDir, screenPos);
        clip = ClampScreenPosToEdges_0(localClientNum, screenPos);

        if (clip != CLIP_NONE && target->offscreenMaterialIndex != -1)
            materialIndex = target->offscreenMaterialIndex;
        else
            materialIndex = target->materialIndex;

        material = defaultMaterial;
        if (materialIndex != -1 && CG_ServerMaterialName(localClientNum, materialIndex, materialName, sizeof(materialName)))
            material = Material_RegisterHandle(materialName, 7);

        halfSize = vehHudTargetSize->current.value * 0.5f;
        if (clip == CLIP_NONE || target->offscreenMaterialIndex == -1)
        {
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                screenPos[0] - halfSize,
                screenPos[1] - halfSize,
                vehHudTargetSize->current.value,
                vehHudTargetSize->current.value,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                color,
                material);
            continue;
        }

        angle = 0.0f;
        switch (clip)
        {
        case CLIP_BOTTOM:
            angle = 180.0f;
            break;
        case CLIP_RIGHT:
            angle = 90.0f;
            break;
        case CLIP_LEFT:
            angle = 270.0f;
            break;
        }

        CG_DrawRotatedPic(
            &scrPlaceView[localClientNum],
            screenPos[0] - halfSize,
            screenPos[1] - halfSize,
            vehHudTargetSize->current.value,
            vehHudTargetSize->current.value,
            rect->horzAlign,
            rect->vertAlign,
            angle,
            color,
            material);
    }
}

void __cdecl CG_DrawJavelinTargets(int localClientNum, rectDef_s *rect, float *color, Material *defaultMaterial)
{
    const playerState_s *ps;
    centity_s *targetEnt;
    int targetIndex;
    float worldDir[3];
    float screenPos[2];

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);

    ps = CG_GetPredictedPlayerState(localClientNum);
    if ((ps->weapLockFlags & 2) == 0)
        return;

    if (ps->weapLockedEntnum == ENTITYNUM_NONE)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp",
            271,
            0,
            "%s",
            "ps->weapLockedEntnum != ENTITYNUM_NONE");

    for (targetIndex = 0; targetIndex < ARRAY_COUNT(cgArray[0].targets); ++targetIndex)
    {
        if (cgArray[0].targets[targetIndex].entNum == ps->weapLockedEntnum)
            break;
    }
    if (targetIndex == ARRAY_COUNT(cgArray[0].targets))
        return;

    targetEnt = CG_GetEntity(localClientNum, ps->weapLockedEntnum);
    if (!targetEnt)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp", 283, 0, "%s", "targetEnt");

    worldDir[0] = targetEnt->pose.origin[0] + cgArray[0].targets[targetIndex].offset[0] - cgArray[0].refdef.vieworg[0];
    worldDir[1] = targetEnt->pose.origin[1] + cgArray[0].targets[targetIndex].offset[1] - cgArray[0].refdef.vieworg[1];
    worldDir[2] = targetEnt->pose.origin[2] + cgArray[0].targets[targetIndex].offset[2] - cgArray[0].refdef.vieworg[2];
    WorldDirToScreenPos(localClientNum, worldDir, screenPos);
    if (ClampScreenPosToEdges_0(localClientNum, screenPos) != CLIP_NONE)
        return;

    CL_DrawStretchPic(
        &scrPlaceView[localClientNum],
        screenPos[0],
        -240.0,
        2.0,
        480.0,
        rect->horzAlign,
        rect->vertAlign,
        0.0,
        0.0,
        1.0,
        1.0,
        color,
        defaultMaterial);
    CL_DrawStretchPic(
        &scrPlaceView[localClientNum],
        -320.0,
        screenPos[1],
        640.0,
        2.0,
        rect->horzAlign,
        rect->vertAlign,
        0.0,
        0.0,
        1.0,
        1.0,
        color,
        defaultMaterial);
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
    float screenPos[2]; // [sp+90h] [-E0h] BYREF
    //float v30; // [sp+94h] [-DCh]
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
            WorldDirToScreenPos(localClientNum, v34, screenPos);
            float projectileSpeed = (float)WeaponDef->iProjectileSpeed;
            v31 = v34[0] * projectileSpeed;
            v32 = projectileSpeed * v34[1];
            v33 = v34[2] * projectileSpeed;
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
                v11 = screenPos[0];
                v12 = *((float *)&v28 + 1);
                v13 = screenPos[1];
                v26 = *(float *)&v28 - screenPos[0];
                v27 = *((float *)&v28 + 1) - screenPos[1];
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
                        rect->horzAlign,
                        rect->vertAlign,
                        color,
                        cgMedia.vehHudLine);
                    v19 = vehHudReticlePipOnAStickCenterCircle;
                }
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    (float)((float)v11 - (float)v21),
                    (float)((float)v13 - (float)v21),
                    v19->current.value,
                    v19->current.value,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    color,
                    cgMedia.vehCenterCircle); // might be the below material (vehMovingCircle) instead
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    (float)((float)v10 - (float)v20),
                    (float)((float)v12 - (float)v20),
                    vehHudReticlePipOnAStickMovingCircle->current.value,
                    vehHudReticlePipOnAStickMovingCircle->current.value,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    color,
                    cgMedia.vehMovingCircle);
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

// aislop
void CG_DrawBouncingDiamond(int localClientNum, rectDef_s *rect, float *color)
{
    if (localClientNum != 0) {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    }

    float bounceRadius = vehHudReticleBouncingRadius->current.value;
    int targetEntNum = cgArray[0].vehReticleLockOnEntNum;

    float screenX = 0.0f, screenY = 0.0f;

    if (targetEntNum != 2175) {
        float worldPos[3];

        if (!CG_GetTargetPos(localClientNum, targetEntNum, &worldPos[0])) {
            centity_s *ent = CG_GetEntity(localClientNum, targetEntNum);
            if (!ent) {
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp", 442, 0, "%s", "targetEnt");
            }
            worldPos[0] = ent->pose.origin[0];
            worldPos[1] = ent->pose.origin[1];
            worldPos[2] = ent->pose.origin[2];
        }

        // Convert to view space
        worldPos[0] -= cgArray[0].refdef.vieworg[0];
        worldPos[1] -= cgArray[0].refdef.vieworg[1];
        worldPos[2] -= cgArray[0].refdef.vieworg[2];

        float screenPos[2];
        WorldDirToScreenPos(localClientNum, worldPos, screenPos);

        float lockDuration = (float)cgArray[0].vehReticleLockOnDuration;
        float interp = 1.0f;

        if (lockDuration > 0.0f) {
            int timeElapsed = cgArray[0].time - cgArray[0].vehReticleLockOnStartTime;
            float progress = (float)timeElapsed / lockDuration;
            interp = (progress > 1.0f) ? 1.0f : -progress + 1.0f;
        }

        // Apply interpolated position
        float bounceScale = (1.0f - interp) * bounceRadius;

        screenX = screenPos[0] * bounceScale;
        screenY = screenPos[1] * bounceScale;
    }

    float x, y;
    if (bounceRadius == 0.0f) {
        x = screenX;
        y = screenY;
        cgArray[0].vehReticleOffset[0] = x;
    }
    else {
        float offsetX = cgArray[0].vehReticleOffset[0];
        float offsetY = cgArray[0].vehReticleOffset[1];
        float velX = cgArray[0].vehReticleVel[0];
        float velY = cgArray[0].vehReticleVel[1];

        bool reset = (velX == 0.0f && velY == 0.0f);

        float dx = offsetX - screenX;
        float dy = offsetY - screenY;
        float distSqr = dx * dx + dy * dy;

        if (reset || distSqr > bounceRadius * bounceRadius) {
            float angle = random() * 360.0f * (float)(M_PI / 180.0);
            float sinA = sinf(angle);
            float cosA = cosf(angle);

            velX = vehHudReticleBouncingSpeed->current.value * sinA;
            velY = vehHudReticleBouncingSpeed->current.value * cosA;

            float dot1 = dx * velY + dy * velX;
            float dot2 = dx * velX - dy * velY;

            if (dot2 < dot1) {
                velX = -velY;
                velY = velX;
            }

            float negVelX = -velX;
            float dot3 = dx * negVelX + dy * -velY;

            if (dot3 < dot1) {
                velX = -velY;
                velY = -velX;
            }

            cgArray[0].vehReticleVel[0] = velX;
            cgArray[0].vehReticleVel[1] = velY;
        }

        float deltaTime = cgArray[0].frametime * 0.001f;

        x = offsetX + velX * deltaTime;
        y = offsetY + velY * deltaTime;

        cgArray[0].vehReticleOffset[0] = x;
        cgArray[0].vehReticleOffset[1] = y;
    }

    float size = vehHudReticleBouncingDiamondSize->current.value;
    float halfSize = size * 0.5f;

    CL_DrawStretchPic(
        &scrPlaceView[localClientNum],
        x - halfSize,
        y - halfSize,
        size,
        size,
        rect->horzAlign,
        rect->vertAlign,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        color,
        cgMedia.vehBouncingDiamondReticle);
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

