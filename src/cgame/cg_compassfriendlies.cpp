#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_compassfriendlies.h"
#include <game/actor.h>
#include "cg_actors.h"
#include "cg_public.h"
#include "cg_main.h"
#include "cg_local.h"

CompassActor s_compassActors[1][32];
CompassVehicle s_compassVehicles[1][64];

void __cdecl TRACK_cg_compassfriendlies()
{
    track_static_alloc_internal(s_compassActors, 768, "s_compassActors", 9);
    track_static_alloc_internal(s_compassVehicles, 1280, "s_compassVehicles", 9);
}

void __cdecl CG_ClearCompassPingData()
{
    memset(s_compassActors, 0, sizeof(s_compassActors));
    memset(s_compassVehicles, 0, sizeof(s_compassVehicles));
}

void __cdecl CG_CompassAddWeaponPingInfo(int localClientNum, centity_s *cent, float *origin, int msec)
{
    int eType; // r11
    unsigned int v9; // r7
    CompassActor *v10; // r31
    int LocalClientTime; // r3
    bool v12; // cr58
    char v13; // r10
    unsigned int v14; // [sp+50h] [-40h] BYREF
    int team; // [sp+54h] [-3Ch] BYREF

    eType = cent->nextState.eType;
    if (eType != 16)
    {
        if (eType != 14)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_compassfriendlies.cpp",
                61,
                0,
                "%s",
                "cent->nextState.eType == ET_ACTOR");
        G_BypassForCG_GetClientActorIndexAndTeam(cent->nextState.number, (int *)&v14, &team);
        if (team != 3 && team != 4)
        {
            if (team != 2 && team != 1)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_compassfriendlies.cpp",
                    66,
                    0,
                    "%s",
                    "actorTeam == TEAM_ALLIES || actorTeam == TEAM_AXIS");
            if (localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_compassfriendlies.cpp",
                    68,
                    0,
                    "localClientNum doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1);
            v9 = v14;
            if (v14 >= 0x20)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_compassfriendlies.cpp",
                    69,
                    0,
                    "actorIndex doesn't index MAX_COMPASS_ACTORS\n\t%i not in [0, %i)",
                    v14,
                    32);
                v9 = v14;
            }
            v10 = &s_compassActors[localClientNum][v9];
            LocalClientTime = CG_GetLocalClientTime(localClientNum);
            v12 = team == 1;
            //v13 = _cntlzw(v15 - 1);
            v10->beginFadeTime = LocalClientTime + msec;
            //v10->enemy = (v13 & 0x20) != 0;
            v10->enemy = team == TEAM_BAD;
            if (v12)
            {
                v10->lastPos[0] = *origin;
                v10->lastPos[1] = origin[1];
            }
        }
    }
}

void __cdecl CG_CompassApplyPointerRadiusScale(float *radiusScale)
{
    if (!radiusScale)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_compassfriendlies.cpp", 81, 0, "%s", "radiusScale");
    *radiusScale = (float)(compassSize->current.value * *radiusScale) * (float)55.0;
}

void __cdecl CG_CalcCompassPointerRadius(float *radius, double dist)
{
    const dvar_s *v4; // r11
    double v5; // fp0
    double v6; // fp0

    if (!radius)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_compassfriendlies.cpp", 89, 0, "%s", "radius");
    v4 = compassMinRadius;
    v5 = (float)((float)((float)dist - compassMinRange->current.value)
        / (float)(compassMaxRange->current.value - compassMinRange->current.value));
    *radius = (float)((float)dist - compassMinRange->current.value)
        / (float)(compassMaxRange->current.value - compassMinRange->current.value);
    v6 = (float)((float)((float)1.0 - v4->current.value) * (float)v5);
    *radius = v6;
    *radius = v4->current.value + (float)v6;
    CG_CompassApplyPointerRadiusScale(radius);
}

void __cdecl CG_CompassUpdateActorInfo(int localClientNum, int entityIndex)
{
    centity_s *Entity; // r31
    int ClientActorFriendlyIndex; // r3
    CompassActor *v6; // r11

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (!cgArray[0].nextSnap)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_compassfriendlies.cpp",
            107,
            0,
            "%s",
            "cgameGlob->nextSnap");
    Entity = CG_GetEntity(localClientNum, entityIndex);
    if (Entity->nextState.eType != 14)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_compassfriendlies.cpp",
            109,
            0,
            "%s",
            "cent->nextState.eType == ET_ACTOR");
    ClientActorFriendlyIndex = G_BypassForCG_GetClientActorFriendlyIndex(entityIndex);
    if (ClientActorFriendlyIndex >= 0)
    {
        v6 = &s_compassActors[localClientNum][ClientActorFriendlyIndex];
        v6->lastUpdate = cgArray[0].time;
        v6->lastPos[0] = Entity->pose.origin[0];
        v6->lastPos[1] = Entity->pose.origin[1];
        v6->lastYaw = Entity->pose.angles[1];
    }
}

CompassVehicle *__cdecl Compass_GetVehicle(int localClientNum, int entityNum)
{
    unsigned int v2; // r5
    int v3; // r11
    CompassVehicle *v4; // r10
    int v5; // r11
    int *p_lastUpdate; // r10
    CompassVehicle *result; // r3
    int v8; // r31
    int v9; // r6
    int v10; // r11
    int v11; // r8
    int *v12; // r10

    v2 = 1280 * localClientNum;
    v3 = 0;
    v4 = s_compassVehicles[localClientNum];
    do
    {
        if (v4->entityNum == entityNum)
            return &s_compassVehicles[localClientNum][v3];
        ++v3;
        ++v4;
    } while (v3 < 64);
    v5 = 0;
    p_lastUpdate = &s_compassVehicles[v2 / 0x500][1].lastUpdate;
    while (*(p_lastUpdate - 5))
    {
        if (!*p_lastUpdate)
        {
            ++v5;
            break;
        }
        if (!p_lastUpdate[5])
        {
            v5 += 2;
            break;
        }
        if (!p_lastUpdate[10])
        {
            v5 += 3;
            break;
        }
        v5 += 4;
        p_lastUpdate += 20;
        if (v5 >= 64)
            goto LABEL_16;
    }
    if (v5 < 64)
        goto LABEL_25;
LABEL_16:
    v8 = 0;
    v9 = 1;
    v10 = 0;
    v11 = 60;
    v12 = &s_compassVehicles[v2 / 0x500][2].lastUpdate;
    do
    {
        if (*(v12 - 5) < *(int *)((char *)&s_compassVehicles[0][0].lastUpdate + v10 + v2))
        {
            v8 = v9;
            v10 = v11 - 40;
        }
        if (*v12 < *(int *)((char *)&s_compassVehicles[0][0].lastUpdate + v10 + v2))
        {
            v8 = v9 + 1;
            v10 = v11 - 20;
        }
        if (v12[5] < *(int *)((char *)&s_compassVehicles[0][0].lastUpdate + v10 + v2))
        {
            v8 = v9 + 2;
            v10 = v11;
        }
        v11 += 60;
        v9 += 3;
        v12 += 15;
    } while (v11 < 1320);
    v5 = v8;
LABEL_25:
    result = &s_compassVehicles[localClientNum][v5];
    result->entityNum = 0;
    result->lastUpdate = 0;
    result->lastPos[0] = 0.0;
    result->lastPos[1] = 0.0;
    result->lastYaw = 0.0;
    result->entityNum = entityNum;
    return result;
}

void __cdecl CG_CompassUpdateVehicleInfo(int localClientNum, int entityIndex)
{
    centity_s *Entity; // r31
    CompassVehicle *Vehicle; // r3

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (!cgArray[0].nextSnap)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_compassfriendlies.cpp",
            165,
            0,
            "%s",
            "cgameGlob->nextSnap");
    Entity = CG_GetEntity(localClientNum, entityIndex);
    if (Entity->nextState.eType != 11)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_compassfriendlies.cpp",
            167,
            0,
            "%s",
            "cent->nextState.eType == ET_VEHICLE");
    Vehicle = Compass_GetVehicle(localClientNum, entityIndex);
    Vehicle->lastUpdate = cgArray[0].time;
    *(double *)Vehicle->lastPos = *(double *)Entity->pose.origin;
    Vehicle->lastYaw = Entity->pose.angles[1];
}

void __cdecl CG_CompassDrawActors(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    float *color)
{
    double v11; // fp24
    double v12; // fp27
    double v13; // fp26
    int v14; // r19
    CompassActor *v15; // r31
    int v16; // r11
    Material *v17; // r7
    const float *v18; // r6
    int v19; // r5
    int v20; // r4
    double v21; // fp1
    int beginFadeTime; // r11
    double v23; // fp31
    Material *compassping_friendlyfiring; // r30
    double v25; // fp0
    double v26; // fp0
    __int64 v27; // r11
    const char *v28; // r3
    int v29; // r8
    int v30; // r7
    double v31; // fp8
    double v32; // fp7
    double v33; // fp6
    double v34; // fp5
    int v35; // r30
    int *p_beginFadeTime; // r31
    int v37; // r11
    double value; // fp0
    __int64 v39; // r10
    Material *v40; // r7
    const float *v41; // r6
    int v42; // r5
    int v43; // r4
    int v44; // r11
    double v45; // fp1
    double v46; // fp2
    double v47; // fp0
    __int64 v48; // r11
    float v49; // [sp+8h] [-1B8h]
    float v50; // [sp+10h] [-1B0h]
    double lastYaw; // [sp+18h] [-1A8h]
    float v52; // [sp+28h] [-198h]
    float v53; // [sp+30h] [-190h]
    float v54; // [sp+38h] [-188h]
    float v55; // [sp+40h] [-180h]
    float v56; // [sp+48h] [-178h]
    float v57; // [sp+50h] [-170h]
    float v58; // [sp+58h] [-168h]
    float v59; // [sp+60h] [-160h]
    float v60; // [sp+68h] [-158h]
    float v61; // [sp+70h] [-150h] BYREF
    float v62; // [sp+74h] [-14Ch] BYREF
    float v63; // [sp+78h] [-148h]
    const char *v64; // [sp+7Ch] [-144h]
    float v65; // [sp+80h] [-140h] BYREF
    float v66; // [sp+84h] [-13Ch]
    float v67[2]; // [sp+88h] [-138h] BYREF
    unsigned __int64 v68; // [sp+90h] [-130h]
    unsigned __int64 v69; // [sp+98h] [-128h]
    unsigned __int64 v70; // [sp+A0h] [-120h]
    float v71; // [sp+A8h] [-118h]
    float v72; // [sp+B0h] [-110h]
    float v73; // [sp+B4h] [-10Ch]
    float v74; // [sp+B8h] [-108h]
    float v75; // [sp+BCh] [-104h]
    rectDef_s v76[4]; // [sp+C0h] [-100h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v64 = "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_compassfriendlies.cpp";
    if (!cgArray[0].nextSnap)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_compassfriendlies.cpp",
            196,
            0,
            "%s",
            "cgameGlob->nextSnap");
    v11 = CG_FadeCompass(localClientNum, cgArray[0].compassFadeTime, compassType);
    if (v11 != 0.0)
    {
        CG_CompassUpYawVector(cgArray, v67);
        CG_CompassCalcDimensions(compassType, cgArray, parentRect, rect, &v76[0].x, &v76[0].y, &v76[0].w, &v76[0].h);
        v72 = *color;
        v73 = color[1];
        v74 = color[2];
        v75 = color[3];
        v12 = (float)((float)(v76[0].w * (float)0.5) + v76[0].x);
        v13 = (float)((float)(v76[0].h * (float)0.5) + v76[0].y);
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_compassfriendlies.cpp",
                211,
                0,
                "localClientNum doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                localClientNum,
                1);
        v14 = 32;
        v15 = s_compassActors[localClientNum];
        v63 = -1.741201e-37;
        do
        {
            if (v15->lastUpdate > cgArray[0].time)
                v15->lastUpdate = 0;
            if (v15->lastUpdate >= cgArray[0].time - 800
                && (!CG_WorldPosToCompass(compassType, cgArray, v76, v67, cgArray[0].refdef.vieworg, v15->lastPos, 0, &v65)
                    || *(_BYTE *)(*(unsigned int *)(LODWORD(v63) - 17812) + 12)))
            {
                CalcCompassFriendlySize(compassType, &v61, &v62);
                v65 = v65 - (float)((float)(v61 * (float)0.5) - (float)v12);
                v66 = v66 - (float)((float)(v62 * (float)0.5) - (float)v13);
                if (compassType || !compassRotation->current.enabled)
                    v16 = 171408;
                else
                    v16 = 170872;
                v21 = AngleNormalize360((float)(*(float *)((char *)&cgArray[0].clientNum + v16) - v15->lastYaw));
                beginFadeTime = v15->beginFadeTime;
                v23 = v21;
                compassping_friendlyfiring = 0;
                v25 = (float)(compassSoundPingFadeTime->current.value * (float)1000.0);
                v68 = __PAIR64__((unsigned int)compassSoundPingFadeTime, cgArray[0].time);
                v69 = __PAIR64__((unsigned int)compassSoundPingFadeTime, beginFadeTime);
                if ((float)((float)__SPAIR64__((unsigned int)compassSoundPingFadeTime, beginFadeTime) + (float)v25) < (double)(float)__SPAIR64__((unsigned int)compassSoundPingFadeTime, cgArray[0].time))
                {
                    v26 = v75;
                }
                else if (beginFadeTime < cgArray[0].time)
                {
                    compassping_friendlyfiring = cgMedia.compassping_friendlyfiring;
                    HIDWORD(v27) = cgArray[0].time;
                    LODWORD(v27) = cgArray[0].time - beginFadeTime;
                    v70 = v27;
                    v26 = (float)((float)1.0 - (float)((float)v27 / (float)v25));
                    v75 = v26;
                }
                else
                {
                    v26 = 1.0;
                    compassping_friendlyfiring = cgMedia.compassping_friendlyfiring;
                    v75 = 1.0;
                }
                if (v11 < color[3])
                    color[3] = v11;
                if (v11 < v26)
                {
                    v26 = v11;
                    v75 = v11;
                }
                if (!compassping_friendlyfiring || v26 != 1.0)
                    CG_DrawRotatedPic(&scrPlaceView[localClientNum], v65, v66, v61, v62, v20, v19, v21, v18, v17);
                if (compassping_friendlyfiring)
                    CG_DrawRotatedPic(&scrPlaceView[localClientNum], v65, v66, v61, v62, v20, v19, v23, v18, v17);
                if (compassDebug->current.enabled)
                {
                    lastYaw = v15->lastYaw;
                    v28 = va((const char *)HIDWORD(lastYaw), LODWORD(lastYaw), LODWORD(v23));
                    CL_DrawText(
                        &scrPlaceView[localClientNum],
                        v28,
                        0x7FFFFFFF,
                        cgMedia.smallDevFont,
                        v65,
                        (float)(v66 + (float)15.0),
                        v30,
                        v29,
                        0.69999999,
                        0.69999999,
                        (const float *)rect->horzAlign,
                        rect->vertAlign,
                        v34,
                        v33,
                        v32,
                        v31,
                        v49,
                        v50,
                        *(float *)&lastYaw,
                        *(float *)&v23,
                        v52,
                        v53,
                        v54,
                        v55,
                        v56,
                        v57,
                        v58,
                        v59,
                        v60,
                        v61,
                        v63,
                        v65,
                        v67[0],
                        *(float *)&v68,
                        *(float *)&v69,
                        *(float *)&v70,
                        v71,
                        v72);
                }
            }
            --v14;
            ++v15;
        } while (v14);
        if (localClientNum)
            MyAssertHandler(
                v64,
                269,
                0,
                "localClientNum doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                localClientNum,
                1);
        v35 = 32;
        p_beginFadeTime = &s_compassActors[localClientNum][0].beginFadeTime;
        do
        {
            if (*((_BYTE *)p_beginFadeTime + 4))
            {
                v37 = *p_beginFadeTime;
                value = compassSoundPingFadeTime->current.value;
                HIDWORD(v39) = (uint32)compassSoundPingFadeTime;
                LODWORD(v39) = *p_beginFadeTime;
                v70 = __PAIR64__((unsigned int)compassSoundPingFadeTime, cgArray[0].time);
                v69 = v39;
                if ((float)((float)((float)value * (float)1000.0) + (float)v39) >= (double)(float)__SPAIR64__(
                    (unsigned int)compassSoundPingFadeTime,
                    cgArray[0].time)
                    && v37
                    && (!CG_WorldPosToCompass(
                        compassType,
                        cgArray,
                        v76,
                        v67,
                        cgArray[0].refdef.vieworg,
                        (const float *)p_beginFadeTime - 3,
                        0,
                        &v65)
                        || *(_BYTE *)(*(unsigned int *)(LODWORD(v63) - 17812) + 12)))
                {
                    CalcCompassFriendlySize(compassType, &v61, &v62);
                    v44 = *p_beginFadeTime;
                    v45 = (float)(v65 - (float)((float)(v61 * (float)0.5) - (float)v12));
                    v46 = (float)(v66 - (float)((float)(v62 * (float)0.5) - (float)v13));
                    v65 = v65 - (float)((float)(v61 * (float)0.5) - (float)v12);
                    v66 = v66 - (float)((float)(v62 * (float)0.5) - (float)v13);
                    if (v44 < cgArray[0].time)
                    {
                        LODWORD(v48) = cgArray[0].time - v44;
                        HIDWORD(v48) = v48;
                        v47 = (float)((float)1.0
                            - (float)((float)v48 / (float)(compassSoundPingFadeTime->current.value * (float)1000.0)));
                        v68 = v48;
                    }
                    else
                    {
                        v47 = 1.0;
                    }
                    v75 = v47;
                    if (v11 < v47)
                        v75 = v11;
                    UI_DrawHandlePic(&scrPlaceView[localClientNum], v45, v46, v61, v62, v43, v42, v41, v40);
                }
            }
            --v35;
            p_beginFadeTime += 6;
        } while (v35);
    }
}

void __cdecl CG_CompassDrawVehicles(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    float *color,
    unsigned __int8 vehicleCompassType)
{
    double v13; // fp26
    double v14; // fp28
    double v15; // fp27
    int v16; // r19
    CompassVehicle *v17; // r30
    double value; // fp13
    const dvar_s *v19; // r11
    double v20; // fp31
    double v21; // fp30
    double v22; // fp0
    int v23; // r11
    Material *v24; // r7
    const float *v25; // r6
    int v26; // r5
    int v27; // r4
    double v28; // fp1
    float v29; // [sp+60h] [-100h] BYREF
    float v30; // [sp+64h] [-FCh]
    float v31[5]; // [sp+68h] [-F8h] BYREF
    float v32; // [sp+7Ch] [-E4h]
    rectDef_s v33[3]; // [sp+80h] [-E0h] BYREF

    if (compassType)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_compassfriendlies.cpp",
            318,
            0,
            "%s",
            "compassType == COMPASS_TYPE_PARTIAL");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (!cgArray[0].nextSnap)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_compassfriendlies.cpp",
            321,
            0,
            "%s",
            "cgameGlob->nextSnap");
    v13 = CG_FadeCompass(localClientNum, cgArray[0].compassFadeTime, compassType);
    if (v13 != 0.0)
    {
        CG_CompassCalcDimensions(compassType, cgArray, parentRect, rect, &v33[0].x, &v33[0].y, &v33[0].w, &v33[0].h);
        v31[2] = *color;
        v31[3] = color[1];
        v31[4] = color[2];
        v32 = color[3];
        v14 = (float)((float)(v33[0].w * (float)0.5) + v33[0].x);
        v15 = (float)((float)(v33[0].h * (float)0.5) + v33[0].y);
        CG_CompassUpYawVector(cgArray, v31);
        v16 = 64;
        v17 = s_compassVehicles[localClientNum];
        do
        {
            if (v17->lastUpdate > cgArray[0].time)
                v17->lastUpdate = 0;
            if (v17->lastUpdate >= cgArray[0].time - 800
                && CG_GetEntity(localClientNum, v17->entityNum)->nextState.un1.scale == vehicleCompassType
                && (!CG_WorldPosToCompass(compassType, cgArray, v33, v31, cgArray[0].refdef.vieworg, v17->lastPos, 0, &v29)
                    || compassClampIcons->current.enabled))
            {
                if (vehicleCompassType == 1)
                {
                    value = compassIconTankWidth->current.value;
                    v19 = compassIconTankHeight;
                }
                else
                {
                    value = compassIconOtherVehWidth->current.value;
                    v19 = compassIconOtherVehHeight;
                }
                v20 = (float)(compassSize->current.value * (float)value);
                v21 = (float)(compassSize->current.value * v19->current.value);
                v22 = (float)-(float)((float)((float)(compassSize->current.value * (float)value) * (float)0.5) - (float)v14);
                v30 = v30 - (float)((float)((float)(compassSize->current.value * v19->current.value) * (float)0.5) - (float)v15);
                v29 = (float)v22 + v29;
                if (compassType || !compassRotation->current.enabled)
                    v23 = 171408;
                else
                    v23 = 170872;
                v28 = AngleNormalize360((float)(*(float *)((char *)&cgArray[0].clientNum + v23) - v17->lastYaw));
                if (v13 < v32)
                    v32 = v13;
                CG_DrawRotatedPic(&scrPlaceView[localClientNum], v29, v30, v20, v21, v27, v26, v28, v25, v24);
            }
            --v16;
            ++v17;
        } while (v16);
    }
}

