#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_vehicle_hud.h"
#include "cg_local.h"
#include <client/client.h>

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
    return (_cntlzw(v22) & 0x20) == 0;
}

void __cdecl CG_VehicleHudRegisterDvars()
{
    const char *v3; // r5
    unsigned __int16 v4; // r4
    const char *v5; // r5
    unsigned __int16 v6; // r4
    const char *v7; // r5
    unsigned __int16 v8; // r4
    const char *v9; // r5
    unsigned __int16 v10; // r4
    const char *v11; // r5
    unsigned __int16 v12; // r4
    const char *v13; // r5
    unsigned __int16 v14; // r4
    const char *v15; // r5
    unsigned __int16 v16; // r4
    const char *v17; // r5
    unsigned __int16 v18; // r4
    const char *v19; // r5
    unsigned __int16 v20; // r4
    const char *v21; // r5
    unsigned __int16 v22; // r4
    const char *v23; // r5
    unsigned __int16 v24; // r4
    const char *v25; // r5
    unsigned __int16 v26; // r4

    dword_827DD9BC = (int)Dvar_RegisterFloat("vehHudReticlePipOnAStickCenterCircle", 8.0, 0.0099999998, 10000.0, a2, a3);
    dword_827DD99C = (int)Dvar_RegisterFloat("vehHudReticlePipOnAStickMovingCircle", 32.0, 0.0099999998, 10000.0, v4, v3);
    dword_827DD9B4 = (int)Dvar_RegisterFloat(
        "vehHudReticlePipOnAStickCenterCircleBuffer",
        0.25,
        -10000.0,
        10000.0,
        v6,
        v5);
    dword_827DD9A0 = (int)Dvar_RegisterFloat("vehHudReticlePipOnAStickMovingCircleBuffer", 1.0, -10000.0, 10000.0, v8, v7);
    vehHudLineWidth = Dvar_RegisterFloat("vehHudLineWidth", 8.0, 0.0099999998, 10000.0, v10, v9);
    dword_827DD9B8 = (int)Dvar_RegisterBool(
        "vehHudDrawPipOnStickWhenFreelooking",
        1,
        0,
        "Set to 0 to not draw the pip-on-a-stick reticle when the player is freelooking");
    vehHudTargetSize = Dvar_RegisterFloat("vehHudTargetSize", 30.0, 0.0099999998, 10000.0, v12, v11);
    dword_827DD994 = (int)Dvar_RegisterFloat("vehHudTargetScreenEdgeClampBufferLeft", 0.0, -10000.0, 10000.0, v14, v13);
    dword_827DD998 = (int)Dvar_RegisterFloat("vehHudTargetScreenEdgeClampBufferRight", 0.0, -10000.0, 10000.0, v16, v15);
    vehHudTargetScreenEdgeClampBufferTop = Dvar_RegisterFloat(
        "vehHudTargetScreenEdgeClampBufferTop",
        0.0,
        -10000.0,
        10000.0,
        v18,
        v17);
    dword_827DD9A4 = (int)Dvar_RegisterFloat("vehHudTargetScreenEdgeClampBufferBottom", 0.0, -10000.0, 10000.0, v20, v19);
    vehHudReticleBouncingDiamondSize = Dvar_RegisterFloat(
        "vehHudReticleBouncingDiamondSize",
        24.0,
        0.0,
        1000000.0,
        v22,
        v21);
    dword_827DD9C4 = (int)Dvar_RegisterFloat("vehHudReticleBouncingRadius", 80.0, 0.0, 1000000.0, v24, v23);
    dword_827DD9C0 = (int)Dvar_RegisterFloat("vehHudReticleBouncingSpeed", 400.0, 0.0, 1000000.0, v26, v25);
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
    MatrixTransposeTransformVector(worldDir, v16, &v13);
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
        if (*((unsigned int *)v6 - 3) != 2175 && (((unsigned int)v6[3] & 2) == 0 || CG_JavelinADS(localClientNum)))
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
        if (PredictedPlayerState->weapLockedEntnum == 2175)
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

void __cdecl CG_DrawPipOnAStickReticle(int localClientNum, rectDef_s *rect, float *color)
{
    centity_s *Entity; // r31
    DObj_s *ClientDObj; // r29
    WeaponDef *WeaponDef; // r25
    __int64 v8; // r11
    double v9; // fp12
    double v10; // fp0
    double v11; // fp28
    double v12; // fp27
    double v13; // fp26
    double v14; // fp25
    Material *v15; // r7
    const float *v16; // r6
    int v17; // r5
    int v18; // r4
    double v19; // fp1
    int v20; // r11
    double v21; // fp31
    double v22; // fp30
    Material *v23; // r7
    const float *v24; // r6
    int v25; // r5
    int v26; // r4
    float v27; // [sp+8h] [-168h]
    float v28; // [sp+8h] [-168h]
    float v29; // [sp+10h] [-160h]
    float v30; // [sp+10h] [-160h]
    float v31; // [sp+18h] [-158h]
    float v32; // [sp+18h] [-158h]
    float v33; // [sp+20h] [-150h]
    float v34; // [sp+20h] [-150h]
    float v35; // [sp+28h] [-148h]
    float v36; // [sp+28h] [-148h]
    float v37; // [sp+30h] [-140h]
    float v38; // [sp+30h] [-140h]
    float v39; // [sp+38h] [-138h]
    float v40; // [sp+38h] [-138h]
    float v41; // [sp+40h] [-130h]
    float v42; // [sp+40h] [-130h]
    float v43; // [sp+48h] [-128h]
    float v44; // [sp+48h] [-128h]
    float v45; // [sp+50h] [-120h]
    float v46; // [sp+50h] [-120h]
    float v47; // [sp+58h] [-118h]
    float v48; // [sp+58h] [-118h]
    float v49; // [sp+60h] [-110h]
    float v50; // [sp+60h] [-110h]
    float v51; // [sp+80h] [-F0h] BYREF
    float v52; // [sp+84h] [-ECh]
    __int64 v53; // [sp+88h] [-E8h] BYREF
    float v54; // [sp+90h] [-E0h] BYREF
    float v55; // [sp+94h] [-DCh]
    float v56; // [sp+98h] [-D8h] BYREF
    float v57; // [sp+9Ch] [-D4h]
    float v58; // [sp+A0h] [-D0h]
    float v59[9]; // [sp+B0h] [-C0h] BYREF
    float v60[21]; // [sp+D4h] [-9Ch] BYREF

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
    if (ClientDObj && (*(float *)(dword_827DD9B8 + 12) != 0.0 || (cgArray[0].predictedPlayerState.eFlags & 0x40000) != 0))
    {
        if (CG_DObjGetWorldTagMatrix(&Entity->pose, ClientDObj, scr_const.tag_body, (float (*)[3])v59, v60))
        {
            WorldDirToScreenPos(localClientNum, v59, &v54);
            LODWORD(v8) = WeaponDef->iProjectileSpeed;
            HIDWORD(v8) = 108032;
            v53 = v8;
            v56 = v59[0] * (float)v8;
            v57 = (float)v8 * v59[1];
            v58 = v59[2] * (float)v8;
            v9 = (float)((float)((float)(Entity->currentState.pos.trDelta[1] - Entity->currentState.pos.trDelta[1])
                * cgArray[0].frameInterpolation)
                + Entity->currentState.pos.trDelta[1]);
            v10 = (float)((float)((float)(Entity->currentState.pos.trDelta[2] - Entity->currentState.pos.trDelta[2])
                * cgArray[0].frameInterpolation)
                + Entity->currentState.pos.trDelta[2]);
            v56 = (float)((float)((float)(Entity->currentState.pos.trDelta[0] - Entity->currentState.pos.trDelta[0])
                * cgArray[0].frameInterpolation)
                + Entity->currentState.pos.trDelta[0])
                + v56;
            v57 = (float)v9 + v57;
            v58 = (float)v10 + v58;
            if ((unsigned __int8)WorldDirToScreenPos(localClientNum, &v56, (float *)&v53))
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
                v11 = *(float *)&v53;
                v12 = v54;
                v13 = *((float *)&v53 + 1);
                v14 = v55;
                v51 = *(float *)&v53 - v54;
                v52 = *((float *)&v53 + 1) - v55;
                v19 = Vec2Normalize(&v51);
                v20 = dword_827DD9BC;
                v21 = (float)(*(float *)(dword_827DD99C + 12) * (float)0.5);
                v22 = (float)(*(float *)(dword_827DD9BC + 12) * (float)0.5);
                if (v19 > (float)((float)((float)((float)(*(float *)(dword_827DD9BC + 12) * (float)0.5)
                    + (float)(*(float *)(dword_827DD99C + 12) * (float)0.5))
                    - *(float *)(dword_827DD9B4 + 12))
                    - *(float *)(dword_827DD9A0 + 12)))
                {
                    CG_Draw2DLine(
                        &scrPlaceView[localClientNum],
                        (float)((float)(v51
                            * (float)((float)(*(float *)(dword_827DD9BC + 12) * (float)0.5)
                                - *(float *)(dword_827DD9B4 + 12)))
                            + (float)v12),
                        (float)((float)(v52
                            * (float)((float)(*(float *)(dword_827DD9BC + 12) * (float)0.5)
                                - *(float *)(dword_827DD9B4 + 12)))
                            + (float)v14),
                        (float)((float)((float)(*(float *)(dword_827DD9A0 + 12)
                            - (float)(*(float *)(dword_827DD99C + 12) * (float)0.5))
                            * v51)
                            + (float)v11),
                        (float)((float)(v52
                            * (float)(*(float *)(dword_827DD9A0 + 12)
                                - (float)(*(float *)(dword_827DD99C + 12) * (float)0.5)))
                            + (float)v13),
                        vehHudLineWidth->current.value,
                        v18,
                        v17,
                        v16,
                        v15);
                    v20 = dword_827DD9BC;
                }
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    (float)((float)v12 - (float)v22),
                    (float)((float)v14 - (float)v22),
                    *(float *)(v20 + 12),
                    *(float *)(v20 + 12),
                    v18,
                    v17,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    v16,
                    v15,
                    rect->horzAlign,
                    rect->vertAlign,
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
                    v47,
                    v49);
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    (float)((float)v11 - (float)v21),
                    (float)((float)v13 - (float)v21),
                    *(float *)(dword_827DD99C + 12),
                    *(float *)(dword_827DD99C + 12),
                    v26,
                    v25,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    v24,
                    v23,
                    rect->horzAlign,
                    rect->vertAlign,
                    v28,
                    v30,
                    v32,
                    v34,
                    v36,
                    v38,
                    v40,
                    v42,
                    v44,
                    v46,
                    v48,
                    v50);
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
    cgArray[0].vehReticleLockOnEntNum = 2175;
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
void __cdecl CG_DrawBouncingDiamond(int localClientNum, rectDef_s *rect, float *color, const float *a4)
{
    double v6; // fp29
    int vehReticleLockOnEntNum; // r4
    centity_s *Entity; // r30
    double v9; // fp13
    __int64 v10; // r10 OVERLAPPED
    double v11; // fp28
    int v12; // r11
    double v17; // fp0
    double v22; // fp12
    double v23; // fp11
    double v24; // fp13
    double v25; // fp0
    double v26; // fp13
    double v27; // fp0
    unsigned __int8 v28; // r11
    double v29; // fp31
    double v30; // fp30
    __int64 v31; // r11
    double v32; // fp29
    long double v33; // fp2
    long double v34; // fp2
    double v35; // fp0
    double v36; // fp29
    long double v37; // fp2
    double v38; // fp10
    double v39; // fp11
    double v40; // fp12
    double v41; // fp9
    double v42; // fp11
    double v43; // fp10
    float v44; // [sp+8h] [-108h]
    float v45; // [sp+10h] [-100h]
    float v46; // [sp+18h] [-F8h]
    float v47; // [sp+20h] [-F0h]
    float v48; // [sp+28h] [-E8h]
    float v49; // [sp+30h] [-E0h]
    float v50; // [sp+38h] [-D8h]
    float v51; // [sp+40h] [-D0h]
    float v52; // [sp+48h] [-C8h]
    float v53; // [sp+50h] [-C0h]
    float v54; // [sp+58h] [-B8h]
    float v55; // [sp+60h] [-B0h]
    float v56[2]; // [sp+80h] [-90h] BYREF
    __int64 v57; // [sp+88h] [-88h]
    __int64 v58; // [sp+90h] [-80h]
    float v59; // [sp+98h] [-78h] BYREF
    float v60; // [sp+9Ch] [-74h]
    float v61; // [sp+A0h] [-70h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v6 = *(float *)(dword_827DD9C4 + 12);
    vehReticleLockOnEntNum = cgArray[0].vehReticleLockOnEntNum;
    if (cgArray[0].vehReticleLockOnEntNum == 2175)
    {
        v11 = 0.0;
        v22 = 0.0;
        v23 = 0.0;
    }
    else
    {
        if ((unsigned __int8)CG_GetTargetPos(localClientNum, cgArray[0].vehReticleLockOnEntNum, &v59))
        {
            v9 = v61;
        }
        else
        {
            Entity = CG_GetEntity(localClientNum, cgArray[0].vehReticleLockOnEntNum);
            if (!Entity)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp", 442, 0, "%s", "targetEnt");
            v59 = Entity->pose.origin[0];
            v60 = Entity->pose.origin[1];
            v9 = Entity->pose.origin[2];
        }
        v59 = v59 - cgArray[0].refdef.vieworg[0];
        v60 = v60 - cgArray[0].refdef.vieworg[1];
        v61 = (float)v9 - cgArray[0].refdef.vieworg[2];
        WorldDirToScreenPos(localClientNum, &v59, v56);
        LODWORD(v10) = cgArray[0].vehReticleLockOnDuration;
        v11 = 0.0;
        if (cgArray[0].vehReticleLockOnDuration)
        {
            HIDWORD(v10) = cgArray[0].time;
            v57 = v10;
            v12 = cgArray[0].time - cgArray[0].vehReticleLockOnStartTime;
            v58 = *(__int64 *)((char *)&v10 - 4);
            _FP13 = (float)((float)((float)*(__int64 *)((char *)&v10 - 4) / (float)v10) - (float)1.0);
            _FP12 = -(float)((float)*(__int64 *)((char *)&v10 - 4) / (float)v10);
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
        v6 = (float)-(float)((float)((float)_FP13 * (float)v6) - (float)v6);
        v17 = (float)((float)_FP13 * (float)2.0);
        _FP13 = (float)((float)((float)_FP13 * (float)2.0) - (float)1.0);
        _FP10 = -v17;
        __asm
        {
            fsel      f0, f13, f27, f0
            fsel      f0, f10, f28, f0
        }
        v22 = (float)(v56[0] * (float)_FP0);
        v23 = (float)(v56[1] * (float)_FP0);
    }
    if (v6 == v11)
    {
        v24 = v22;
        v25 = v23;
        cgArray[0].vehReticleOffset[0] = v22;
    }
    else
    {
        v26 = cgArray[0].vehReticleVel[0];
        v27 = cgArray[0].vehReticleVel[1];
        if (cgArray[0].vehReticleVel[0] != v11 || (v28 = 1, v27 != v11))
            v28 = 0;
        v29 = (float)(cgArray[0].vehReticleOffset[1] - (float)v23);
        v30 = (float)(cgArray[0].vehReticleOffset[0] - (float)v22);
        HIDWORD(v31) = v28;
        if (v28
            || (float)((float)((float)(cgArray[0].vehReticleOffset[1] - (float)v23)
                * (float)(cgArray[0].vehReticleOffset[1] - (float)v23))
                + (float)((float)(cgArray[0].vehReticleOffset[0] - (float)v22)
                    * (float)(cgArray[0].vehReticleOffset[0] - (float)v22))) > (double)(float)((float)v6 * (float)v6))
        {
            v32 = (float)((float)(random() * (float)360.0) * (float)0.017453292);
            *(double *)&v33 = v32;
            v34 = sin(v33);
            v35 = *(double *)&v34;
            *(double *)&v34 = v32;
            v36 = (float)v35;
            v37 = cos(v34);
            cgArray[0].vehReticleVel[0] = v36;
            cgArray[0].vehReticleVel[1] = *(double *)&v37;
            v27 = (float)(*(float *)(dword_827DD9C0 + 12) * (float)*(double *)&v37);
            v26 = (float)(*(float *)(dword_827DD9C0 + 12) * (float)v36);
            v38 = -v27;
            v39 = v26;
            cgArray[0].vehReticleVel[0] = *(float *)(dword_827DD9C0 + 12) * (float)v36;
            cgArray[0].vehReticleVel[1] = v27;
            v40 = (float)((float)((float)v29 * (float)v27) + (float)((float)v30 * (float)v26));
            v41 = (float)((float)((float)v29 * (float)v26) + (float)((float)v30 * (float)-v27));
            if (v41 < v40)
            {
                v26 = -v27;
                v27 = v39;
                v40 = v41;
                cgArray[0].vehReticleVel[0] = v38;
                cgArray[0].vehReticleVel[1] = v39;
            }
            v42 = -v39;
            if ((float)((float)((float)v29 * (float)v38) + (float)((float)v30 * (float)v42)) < v40)
            {
                v26 = v42;
                v27 = v38;
                v40 = (float)((float)((float)v29 * (float)v38) + (float)((float)v30 * (float)v42));
                cgArray[0].vehReticleVel[0] = v42;
                cgArray[0].vehReticleVel[1] = v38;
            }
            v43 = -v38;
            if ((float)((float)((float)v29 * (float)v42) + (float)((float)v30 * (float)v43)) < v40)
            {
                v26 = v43;
                v27 = v42;
                cgArray[0].vehReticleVel[0] = v43;
                cgArray[0].vehReticleVel[1] = v42;
            }
        }
        LODWORD(v31) = cgArray[0].frametime;
        v58 = v31;
        v24 = (float)((float)((float)v26 * (float)((float)v31 * (float)0.001)) + cgArray[0].vehReticleOffset[0]);
        cgArray[0].vehReticleOffset[0] = v24;
        v25 = (float)((float)((float)v27 * (float)((float)v31 * (float)0.001)) + cgArray[0].vehReticleOffset[1]);
    }
    cgArray[0].vehReticleOffset[1] = v25;
    CL_DrawStretchPic(
        &scrPlaceView[localClientNum],
        (float)((float)v24 - (float)(vehHudReticleBouncingDiamondSize->current.value * (float)0.5)),
        (float)((float)v25 - (float)(vehHudReticleBouncingDiamondSize->current.value * (float)0.5)),
        vehHudReticleBouncingDiamondSize->current.value,
        vehHudReticleBouncingDiamondSize->current.value,
        vehReticleLockOnEntNum,
        (int)color,
        v11,
        v11,
        1.0,
        1.0,
        a4,
        (Material *)(68 * localClientNum),
        rect->horzAlign,
        rect->vertAlign,
        v44,
        v45,
        v46,
        v47,
        v48,
        v49,
        v50,
        v51,
        v52,
        v53,
        v54,
        v55);
}

void __cdecl CG_DrawVehicleReticle(int localClientNum, rectDef_s *rect, float *color)
{
    centity_s *Entity; // r30
    activeReticleType_t activeReticleType; // r11
    const float *v8; // r6

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((cgArray[0].predictedPlayerState.eFlags & 0x20000) != 0
        && cgArray[0].predictedPlayerState.viewlocked_entNum != 2175)
    {
        Entity = CG_GetEntity(localClientNum, cgArray[0].predictedPlayerState.viewlocked_entNum);
        if (!Entity)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_vehicle_hud.cpp", 521, 0, "%s", "vehicle");
        if (Entity->nextState.weapon)
        {
            activeReticleType = BG_GetWeaponDef(Entity->nextState.weapon)->activeReticleType;
            if (activeReticleType == VEH_ACTIVE_RETICLE_PIP_ON_A_STICK)
            {
                CG_DrawPipOnAStickReticle(localClientNum, rect, color);
            }
            else if (activeReticleType == VEH_ACTIVE_RETICLE_BOUNCING_DIAMOND)
            {
                CG_DrawBouncingDiamond(localClientNum, rect, color, v8);
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
            v8->entNum = 2175;
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
        v8->entNum = 2175;
    }
}

