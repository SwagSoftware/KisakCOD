#include "cg_local_mp.h"
#include "cg_public_mp.h"
#include <script/scr_const.h>
#include <gfx_d3d/r_scene.h>
#include <xanim/dobj_utils.h>
#include <EffectsCore/fx_system.h>

//struct vehicleEffects(*)[8] vehEffects 8284e650     cg_vehicles_mp.obj

const unsigned __int16 *wheelTags[4] =
{
  (const unsigned __int16 *)&scr_const.tag_wheel_front_left,
  (const unsigned __int16 *)&scr_const.tag_wheel_front_right,
  (const unsigned __int16 *)&scr_const.tag_wheel_back_left,
  (const unsigned __int16 *)&scr_const.tag_wheel_back_right
}; // idb

const dvar_t *vehDebugClient;
const dvar_t *heli_barrelSlowdown;
const dvar_t *vehDriverViewFocusRange;
const dvar_t *heli_barrelMaxVelocity;
const dvar_t *vehDriverViewDist;
const dvar_t *heli_barrelRotation;

void __cdecl CG_VehRegisterDvars()
{
    DvarLimits min; // [esp+4h] [ebp-10h]
    DvarLimits mina; // [esp+4h] [ebp-10h]
    DvarLimits minb; // [esp+4h] [ebp-10h]
    DvarLimits minc; // [esp+4h] [ebp-10h]
    DvarLimits mind; // [esp+4h] [ebp-10h]

    vehDebugClient = Dvar_RegisterBool("vehDebugClient", 0, 0x80u, "Turn on debug information for vehicles");
    min.value.max = 1000.0;
    min.value.min = 1.0;
    vehDriverViewDist = Dvar_RegisterFloat(
        "vehDriverViewDist",
        300.0,
        min,
        0x80u,
        "How far away the driver's view is from the focus point");
    mina.value.max = 1000.0;
    mina.value.min = 0.0;
    vehDriverViewFocusRange = Dvar_RegisterFloat(
        "vehDriverViewFocusRange",
        50.0,
        mina,
        0x80u,
        "How far the driver's view focus will travel vertically");
    minb.value.max = 360.0;
    minb.value.min = -360.0;
    heli_barrelRotation = Dvar_RegisterFloat(
        "heli_barrelRotation",
        70.0,
        minb,
        0,
        "How much to rotate the turret barrel when a helicopter fires");
    minc.value.max = 3.4028235e38;
    minc.value.min = -360.0;
    heli_barrelMaxVelocity = Dvar_RegisterFloat("heli_barrelMaxVelocity", 1250.0, minc, 0, "");
    mind.value.max = 3.4028235e38;
    mind.value.min = -360.0;
    heli_barrelSlowdown = Dvar_RegisterFloat("heli_barrelSlowdown", 360.0, mind, 0, "");
}

DObj_s *__cdecl GetVehicleEntDObj(int localClientNum, centity_s *centVeh)
{
    if (!centVeh->nextValid)
        MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 99, 0, "%s", "centVeh->nextValid");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1083,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return CG_PreProcess_GetDObj(
        localClientNum,
        centVeh->nextState.number,
        centVeh->nextState.eType,
        cgsArray[0].gameModels[centVeh->nextState.un2.hintString]);
}

void __cdecl CG_VehGunnerPOV(int localClientNum, float *resultOrigin, float *resultAngles)
{
    clientInfo_t *ci; // [esp+4h] [ebp-28h]
    float tagMtx[3][3]; // [esp+8h] [ebp-24h] BYREF

    ci = ClientInfoForLocalClient(localClientNum);
    if (!ci)
        MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 169, 0, "%s", "ci");
    if (ci->attachedVehEntNum == 1023)
        MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 170, 0, "%s", "ci->attachedVehEntNum != ENTITYNUM_NONE");
    GetTagMatrix(localClientNum, ci->attachedVehEntNum, scr_const.tag_gunner_pov, tagMtx, resultOrigin);
    AxisToAngles(tagMtx, resultAngles);
}

clientInfo_t *__cdecl ClientInfoForLocalClient(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].predictedPlayerState.clientNum >= 0x40u)
        MyAssertHandler(
            ".\\cgame_mp\\cg_vehicles_mp.cpp",
            71,
            0,
            "ps->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            cgArray[0].predictedPlayerState.clientNum,
            64);
    return &cgArray[0].bgs.clientinfo[cgArray[0].predictedPlayerState.clientNum];
}

void __cdecl GetTagMatrix(
    int localClientNum,
    unsigned int vehEntNum,
    unsigned __int16 tagName,
    //float (*resultTagMat)[3],
    mat3x3& resultTagMat,
    float *resultOrigin)
{
    centity_s *centVeh; // [esp+0h] [ebp-8h]
    DObj_s *objVeh; // [esp+4h] [ebp-4h]

    centVeh = CG_GetEntity(localClientNum, vehEntNum);
    if (centVeh->nextValid)
    {
        objVeh = GetVehicleEntDObj(localClientNum, centVeh);
        if (objVeh)
        {
            CG_DObjGetWorldTagMatrix(&centVeh->pose, objVeh, tagName, resultTagMat, resultOrigin);
        }
        else
        {
            Com_PrintWarning(14, "GetTagMatrix() unable to get vehicle DObj.\n");
            MatrixIdentity33(resultTagMat);
            *resultOrigin = 0.0;
            resultOrigin[1] = 0.0;
            resultOrigin[2] = 0.0;
        }
    }
    else
    {
        Com_PrintWarning(14, "Vehicle being used by a player was not included in latest snapshot.\n");
        MatrixIdentity33(resultTagMat);
        *resultOrigin = 0.0;
        resultOrigin[1] = 0.0;
        resultOrigin[2] = 0.0;
    }
}

bool __cdecl CG_VehLocalClientUsingVehicle(int localClientNum)
{
    clientInfo_t *ci; // [esp+0h] [ebp-4h]

    ci = ClientInfoForLocalClient(localClientNum);
    if (!ci)
        MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 193, 0, "%s", "ci");
    return ci->attachedVehEntNum != 1023;
}

bool __cdecl CG_VehLocalClientDriving(int localClientNum)
{
    clientInfo_t *ci; // [esp+0h] [ebp-4h]

    ci = ClientInfoForLocalClient(localClientNum);
    if (!ci)
        MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 210, 0, "%s", "ci");
    return ci->attachedVehEntNum != 1023 && ci->attachedVehSlotIndex == 0;
}

bool __cdecl CG_VehEntityUsingVehicle(int localClientNum, unsigned int entNum)
{
    clientInfo_t *ci; // [esp+0h] [ebp-4h]

    ci = ClientInfoForEntity(localClientNum, entNum);
    return ci && ci->attachedVehEntNum != 1023;
}

clientInfo_t *__cdecl ClientInfoForEntity(int localClientNum, unsigned int entNum)
{
    centity_s *cent; // [esp+0h] [ebp-4h]

    cent = CG_GetEntity(localClientNum, entNum);
    if (cent->nextState.eType != 1)
        return 0;
    if (cent->nextState.eType >= 17)
        return 0;
    if (cent->nextState.clientNum >= 0x40u)
        MyAssertHandler(
            ".\\cgame_mp\\cg_vehicles_mp.cpp",
            88,
            0,
            "cent->nextState.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            cent->nextState.clientNum,
            64);
    return &bgs->clientinfo[cent->nextState.clientNum];
}

int __cdecl CG_VehLocalClientVehicleSlot(int localClientNum)
{
    clientInfo_t *ci; // [esp+0h] [ebp-4h]

    ci = ClientInfoForLocalClient(localClientNum);
    if (!ci)
        MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 258, 0, "%s", "ci");
    if (ci->attachedVehEntNum == 1023)
        MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 259, 0, "%s", "ci->attachedVehEntNum != ENTITYNUM_NONE");
    return ci->attachedVehSlotIndex;
}

int __cdecl CG_VehPlayerVehicleSlot(int localClientNum, unsigned int entNum)
{
    clientInfo_t *ci; // [esp+0h] [ebp-4h]

    ci = ClientInfoForEntity(localClientNum, entNum);
    if (!ci)
        MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 270, 0, "%s", "ci");
    if (ci->attachedVehEntNum == 1023)
        MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 271, 0, "%s", "ci->attachedVehEntNum != ENTITYNUM_NONE");
    return ci->attachedVehSlotIndex;
}

void __cdecl CG_VehSeatTransformForPlayer(
    int localClientNum,
    unsigned int entNum,
    float *resultOrigin,
    float *resultAngles)
{
    clientInfo_t *ci; // [esp+0h] [ebp-8h]
    centity_s *centPlayer; // [esp+4h] [ebp-4h]

    ci = ClientInfoForEntity(localClientNum, entNum);
    centPlayer = CG_GetEntity(localClientNum, entNum);
    if (centPlayer->nextState.eType != 1)
        MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 285, 0, "%s", "centPlayer->nextState.eType == ET_PLAYER");
    if (centPlayer->nextState.eType >= 17)
        MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 286, 0, "%s", "centPlayer->nextState.eType < ET_EVENTS");
    SeatTransformForClientInfo(localClientNum, ci, resultOrigin, resultAngles);
}

void __cdecl SeatTransformForClientInfo(int localClientNum, clientInfo_t *ci, float *resultOrigin, float *resultAngles)
{
    if (!ci)
        MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 181, 0, "%s", "ci");
    if (ci->attachedVehEntNum == 1023)
        MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 182, 0, "%s", "ci->attachedVehEntNum != ENTITYNUM_NONE");
    SeatTransformForSlot(localClientNum, ci->attachedVehEntNum, ci->attachedVehSlotIndex, resultOrigin, resultAngles);
}

void __cdecl SeatTransformForSlot(
    int localClientNum,
    unsigned int vehEntNum,
    unsigned int vehSlotIdx,
    float *resultOrigin,
    float *resultAngles)
{
    unsigned __int16 tagName; // [esp+0h] [ebp-34h]
    float tagOrigin[3]; // [esp+4h] [ebp-30h] BYREF
    //float tagMtx[3][3]; // [esp+10h] [ebp-24h] BYREF
    mat3x3 tagMtx;

    tagName = BG_VehiclesGetSlotTagName(vehSlotIdx);
    GetTagMatrix(localClientNum, vehEntNum, tagName, tagMtx, tagOrigin);
    if (resultAngles)
        AxisToAngles(tagMtx, resultAngles);
    if (resultOrigin)
    {
        *resultOrigin = tagOrigin[0];
        resultOrigin[1] = tagOrigin[1];
        resultOrigin[2] = tagOrigin[2];
        if (vehSlotIdx <= 1)
            resultOrigin[2] = resultOrigin[2] + -35.0;
    }
}

void __cdecl CG_VehSeatOriginForLocalClient(int localClientNum, float *result)
{
    clientInfo_t *ci; // [esp+0h] [ebp-4h]

    if (!result)
        MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 297, 0, "%s", "result");
    ci = ClientInfoForLocalClient(localClientNum);
    SeatTransformForClientInfo(localClientNum, ci, result, 0);
}

double __cdecl Veh_GetTurretBarrelRoll(int localClientNum, centity_s *cent)
{
    int entityNum; // [esp+0h] [ebp-10h]
    vehicleEffects *vehFx; // [esp+8h] [ebp-8h]
    int msecs; // [esp+Ch] [ebp-4h]

    entityNum = CG_GetEntityIndex(localClientNum, cent);
    vehFx = VehicleGetFxInfo(localClientNum, entityNum);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    msecs = cgArray[0].time - vehFx->lastBarrelUpdateTime;
    vehFx->lastBarrelUpdateTime = cgArray[0].time;
    vehFx->barrelPos = (double)msecs / 1000.0 * vehFx->barrelVelocity + vehFx->barrelPos;
    if (vehFx->barrelPos > 360.0)
        vehFx->barrelPos = vehFx->barrelPos - 360.0;
    vehFx->barrelVelocity = vehFx->barrelVelocity - (double)msecs * heli_barrelSlowdown->current.value / 1000.0;
    if (vehFx->barrelVelocity < 0.0)
        vehFx->barrelVelocity = 0.0;
    return vehFx->barrelPos;
}

int __cdecl CG_GetEntityIndex(int localClientNum, const centity_s *cent)
{
    if (cent->nextState.number != (cent - cg_entitiesArray[localClientNum]) % 1024)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1125,
            0,
            "cent->nextState.number == (cent - &cg_entitiesArray[localClientNum][0]) % MAX_GENTITIES\n\t%i, %i",
            cent->nextState.number,
            (cent - cg_entitiesArray[localClientNum]) % 1024);
    return cent->nextState.number;
}

vehicleEffects vehEffects[1][8];
vehicleEffects *__cdecl VehicleGetFxInfo(int localClientNum, int entityNum)
{
    vehicleEffects *v3; // edx
    int veh; // [esp+4h] [ebp-8h]
    int veha; // [esp+4h] [ebp-8h]
    int vehb; // [esp+4h] [ebp-8h]
    int oldest; // [esp+8h] [ebp-4h]

    for (veh = 0; veh < 8 && vehEffects[localClientNum][veh].active; ++veh)
    {
        if (vehEffects[localClientNum][veh].entityNum == entityNum)
        {
            vehEffects[localClientNum][veh].lastAccessed = Sys_Milliseconds();
            return &vehEffects[localClientNum][veh];
        }
    }
    for (veha = 0; veha < 8 && vehEffects[localClientNum][veha].active; ++veha)
        ;
    if (veha >= 8)
    {
        oldest = 0;
        for (vehb = 1; vehb < 8; ++vehb)
        {
            if (vehEffects[localClientNum][vehb].lastAccessed < vehEffects[localClientNum][oldest].lastAccessed)
                oldest = vehb;
        }
        veha = oldest;
    }
    v3 = &vehEffects[localClientNum][veha];
    *(_DWORD *)&v3->active = 0;
    v3->lastAccessed = 0;
    v3->entityNum = 0;
    v3->nextDustFx = 0;
    v3->nextSmokeFx = 0;
    *(_DWORD *)&v3->soundPlaying = 0;
    v3->barrelVelocity = 0.0;
    v3->barrelPos = 0.0;
    v3->lastBarrelUpdateTime = 0;
    *(_DWORD *)&v3->tag_engine_left = 0;
    v3->active = 1;
    vehEffects[localClientNum][veha].lastAccessed = Sys_Milliseconds();
    vehEffects[localClientNum][veha].entityNum = entityNum;
    return &vehEffects[localClientNum][veha];
}

void __cdecl Veh_IncTurretBarrelRoll(int localClientNum, int entityNum, float rotation)
{
    float v3; // [esp+0h] [ebp-14h]
    float v4; // [esp+4h] [ebp-10h]
    float v5; // [esp+8h] [ebp-Ch]
    float value; // [esp+Ch] [ebp-8h]
    vehicleEffects *vehFx; // [esp+10h] [ebp-4h]

    vehFx = VehicleGetFxInfo(localClientNum, entityNum);
    v5 = rotation + vehFx->barrelVelocity;
    value = heli_barrelMaxVelocity->current.value;
    v4 = value - v5;
    if (v4 < 0.0)
        v3 = value;
    else
        v3 = rotation + vehFx->barrelVelocity;
    vehFx->barrelVelocity = v3;
}

void __cdecl CG_VehProcessEntity(int localClientNum, centity_s *cent)
{
    DObj_s *obj; // [esp+10h] [ebp-78h]
    vehfx_t fxInfo; // [esp+18h] [ebp-70h] BYREF
    const cgs_t *cgs; // [esp+68h] [ebp-20h]
    int time; // [esp+6Ch] [ebp-1Ch]
    LerpEntityState *p_currentState; // [esp+70h] [ebp-18h]
    float lightingOrigin[3]; // [esp+74h] [ebp-14h] BYREF
    float materialTime; // [esp+80h] [ebp-8h]
    entityState_s *ns; // [esp+84h] [ebp-4h]

    p_currentState = &cent->currentState;
    ns = &cent->nextState;
    if ((cent->nextState.lerp.eFlags & 0x20) == 0 && cent->nextValid)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                1083,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        cgs = cgsArray;
        obj = GetVehicleEntDObj(localClientNum, cent);
        if (obj)
        {
            SetupPoseControllers(localClientNum, obj, cent, &fxInfo);
            VehicleFXTest(localClientNum, obj, cent, &fxInfo);
            lightingOrigin[0] = cent->pose.origin[0];
            lightingOrigin[1] = cent->pose.origin[1];
            lightingOrigin[2] = cent->pose.origin[2];
            lightingOrigin[2] = lightingOrigin[2] + 32.0;
            if (localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                    1071,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum);
            if (p_currentState->u.vehicle.materialTime < 0)
            {
                materialTime = 0.0;
            }
            else
            {
                time = p_currentState->u.vehicle.materialTime
                    + (int)((double)(ns->lerp.u.vehicle.materialTime - p_currentState->u.vehicle.materialTime)
                        * cgArray[0].frameInterpolation);
                materialTime = (double)(cgArray[0].time - time) * 0.001000000047497451;
            }
            R_AddDObjToScene(obj, &cent->pose, ns->number, 4u, lightingOrigin, materialTime);
        }
    }
}

void __cdecl SetupPoseControllers(int localClientNum, DObj_s *obj, centity_s *cent, vehfx_t *fxInfo)
{
    const XModel *Model; // eax
    unsigned __int16 v5; // ax
    float scale; // [esp+0h] [ebp-1E8h]
    float v7; // [esp+Ch] [ebp-1DCh]
    float v8; // [esp+10h] [ebp-1D8h]
    float v9; // [esp+14h] [ebp-1D4h]
    float v10; // [esp+18h] [ebp-1D0h]
    double v11; // [esp+1Ch] [ebp-1CCh]
    float v12; // [esp+24h] [ebp-1C4h]
    float v13; // [esp+28h] [ebp-1C0h]
    float v14; // [esp+2Ch] [ebp-1BCh]
    float v15; // [esp+30h] [ebp-1B8h]
    float v16; // [esp+34h] [ebp-1B4h]
    float v17; // [esp+38h] [ebp-1B0h]
    double v18; // [esp+3Ch] [ebp-1ACh]
    float v19; // [esp+44h] [ebp-1A4h]
    float v20; // [esp+48h] [ebp-1A0h]
    float v21; // [esp+4Ch] [ebp-19Ch]
    float v22; // [esp+50h] [ebp-198h]
    float v23; // [esp+54h] [ebp-194h]
    float v24; // [esp+58h] [ebp-190h]
    double v25; // [esp+5Ch] [ebp-18Ch]
    float v26; // [esp+64h] [ebp-184h]
    float v27; // [esp+68h] [ebp-180h]
    float v28; // [esp+6Ch] [ebp-17Ch]
    float v29; // [esp+70h] [ebp-178h]
    float v30; // [esp+74h] [ebp-174h]
    float v31; // [esp+78h] [ebp-170h]
    double v32; // [esp+7Ch] [ebp-16Ch]
    float v33; // [esp+84h] [ebp-164h]
    float v34; // [esp+88h] [ebp-160h]
    float v35; // [esp+8Ch] [ebp-15Ch]
    float v36; // [esp+90h] [ebp-158h]
    float v37; // [esp+94h] [ebp-154h]
    float v38; // [esp+98h] [ebp-150h]
    double frameInterpolation; // [esp+9Ch] [ebp-14Ch]
    float v40; // [esp+A4h] [ebp-144h]
    float v41; // [esp+A8h] [ebp-140h]
    float v42; // [esp+B4h] [ebp-134h]
    float v43; // [esp+B8h] [ebp-130h]
    float v44; // [esp+C4h] [ebp-124h]
    float v45; // [esp+C8h] [ebp-120h]
    float gunYaw; // [esp+CCh] [ebp-11Ch]
    float v47; // [esp+D8h] [ebp-110h]
    float v48; // [esp+DCh] [ebp-10Ch]
    float cosHalfFovInner; // [esp+E0h] [ebp-108h]
    float v50; // [esp+ECh] [ebp-FCh]
    float v51; // [esp+F0h] [ebp-F8h]
    float v52; // [esp+F4h] [ebp-F4h]
    float v53; // [esp+100h] [ebp-E8h]
    float v54; // [esp+104h] [ebp-E4h]
    float v55; // [esp+108h] [ebp-E0h]
    float v56; // [esp+114h] [ebp-D4h]
    float suspTravel; // [esp+124h] [ebp-C4h]
    trace_t trace; // [esp+128h] [ebp-C0h] BYREF
    int tireIdx; // [esp+154h] [ebp-94h]
    LerpEntityState *p_currentState; // [esp+158h] [ebp-90h]
    const DObjAnimMat *boneMtxList; // [esp+15Ch] [ebp-8Ch]
    float wheelPos[3]; // [esp+160h] [ebp-88h] BYREF
    float axis[8][3]; // [esp+16Ch] [ebp-7Ch] BYREF
    const entityState_s *ns; // [esp+1CCh] [ebp-1Ch]
    float traceStart[3]; // [esp+1D0h] [ebp-18h] BYREF
    float traceEnd[3]; // [esp+1DCh] [ebp-Ch] BYREF

    if (!obj)
        MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 421, 0, "%s", "obj");
    if (cent->nextState.eType != 14 && cent->nextState.eType != 12)
        MyAssertHandler(
            ".\\cgame_mp\\cg_vehicles_mp.cpp",
            422,
            0,
            "%s",
            "cent->nextState.eType == ET_VEHICLE || cent->nextState.eType == ET_HELICOPTER");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    p_currentState = &cent->currentState;
    ns = &cent->nextState;
    fxInfo->soundEnabled = 0;
    for (tireIdx = 0; tireIdx < 4; ++tireIdx)
        fxInfo->tireActive[tireIdx] = 0;
    v55 = p_currentState->u.turret.gunAngles[0];
    v41 = ns->lerp.u.turret.gunAngles[0] - v55;
    v56 = v41 * 0.002777777845039964;
    v40 = v56 + 0.5;
    frameInterpolation = cgArray[0].frameInterpolation;
    v38 = floor(v40);
    v37 = (v56 - v38) * 360.0;
    v54 = v37 * frameInterpolation + v55;
    v36 = v54 * 182.0444488525391 + 0.5;
    v35 = floor(v36);
    cent->pose.vehicle.pitch = (int)v35;
    v52 = p_currentState->u.turret.gunAngles[1];
    v34 = ns->lerp.u.turret.gunAngles[1] - v52;
    v53 = v34 * 0.002777777845039964;
    v33 = v53 + 0.5;
    v32 = cgArray[0].frameInterpolation;
    v31 = floor(v33);
    v30 = (v53 - v31) * 360.0;
    v51 = v30 * v32 + v52;
    v29 = v51 * 182.0444488525391 + 0.5;
    v28 = floor(v29);
    cent->pose.vehicle.roll = (int)v28;
    cosHalfFovInner = p_currentState->u.primaryLight.cosHalfFovInner;
    v27 = ns->lerp.u.primaryLight.cosHalfFovInner - cosHalfFovInner;
    v50 = v27 * 0.002777777845039964;
    v26 = v50 + 0.5;
    v25 = cgArray[0].frameInterpolation;
    v24 = floor(v26);
    v23 = (v50 - v24) * 360.0;
    v48 = v23 * v25 + cosHalfFovInner;
    v22 = v48 * 182.0444488525391 + 0.5;
    v21 = floor(v22);
    cent->pose.vehicle.barrelPitch = (int)v21;
    gunYaw = p_currentState->u.vehicle.gunYaw;
    v20 = ns->lerp.u.vehicle.gunYaw - gunYaw;
    v47 = v20 * 0.002777777845039964;
    v19 = v47 + 0.5;
    v18 = cgArray[0].frameInterpolation;
    v17 = floor(v19);
    v16 = (v47 - v17) * 360.0;
    v45 = v16 * v18 + gunYaw;
    v15 = v45 * 182.0444488525391 + 0.5;
    v14 = floor(v15);
    cent->pose.vehicle.yaw = (int)v14;
    cent->pose.turret.barrelPitch = Veh_GetTurretBarrelRoll(localClientNum, cent);
    v43 = p_currentState->u.turret.gunAngles[2];
    v13 = ns->lerp.u.turret.gunAngles[2] - v43;
    v44 = v13 * 0.002777777845039964;
    v12 = v44 + 0.5;
    v11 = cgArray[0].frameInterpolation;
    v10 = floor(v12);
    v9 = (v44 - v10) * 360.0;
    v42 = v9 * v11 + v43;
    v8 = v42 * 182.0444488525391 + 0.5;
    v7 = floor(v8);
    cent->pose.vehicle.steerYaw = (int)v7;
    cent->pose.vehicle.time = (double)ns->time2 * 0.001000000047497451;
    DObjGetBoneIndex(obj, scr_const.tag_body, &cent->pose.vehicle.tag_body);
    DObjGetBoneIndex(obj, scr_const.tag_turret, &cent->pose.vehicle.tag_turret);
    DObjGetBoneIndex(obj, scr_const.tag_barrel, &cent->pose.vehicle.tag_barrel);
    if (cent->pose.cullIn == 2)
    {
        CG_DObjGetWorldTagMatrix(&cent->pose, obj, scr_const.tag_origin, (float (*)[3])axis[4], fxInfo->soundEngineOrigin);
        fxInfo->soundEnabled = 1;
        suspTravel = cent->pose.vehicle.time;
        AnglesToAxis(cent->pose.angles, axis);
        axis[3][0] = cent->pose.origin[0];
        axis[3][1] = cent->pose.origin[1];
        axis[3][2] = cent->pose.origin[2];
        Model = DObjGetModel(obj, 0);
        boneMtxList = XModelGetBasePose(Model);
        if (*wheelTags[0] != scr_const.tag_wheel_front_left)
            MyAssertHandler(
                ".\\cgame_mp\\cg_vehicles_mp.cpp",
                466,
                0,
                "%s",
                "*wheelTags[TAG_WHEEL_FRONT_LEFT] == scr_const.tag_wheel_front_left");
        if (*wheelTags[1] != scr_const.tag_wheel_front_right)
            MyAssertHandler(
                ".\\cgame_mp\\cg_vehicles_mp.cpp",
                467,
                0,
                "%s",
                "*wheelTags[TAG_WHEEL_FRONT_RIGHT] == scr_const.tag_wheel_front_right");
        for (tireIdx = 0; tireIdx < 4; ++tireIdx)
        {
            if (DObjGetBoneIndex(obj, *wheelTags[tireIdx], &cent->pose.vehicle.wheelBoneIndex[tireIdx]))
            {
                MatrixTransformVector43(boneMtxList[cent->pose.vehicle.wheelBoneIndex[tireIdx]].trans, axis, wheelPos);
                Vec3Mad(wheelPos, 40.0, axis[2], traceStart);
                scale = -suspTravel;
                Vec3Mad(wheelPos, scale, axis[2], traceEnd);
                CG_TraceCapsule(&trace, traceStart, (float *)vec3_origin, (float *)vec3_origin, traceEnd, ns->number, 529);
                v5 = CompressUnit(trace.fraction);
                cent->pose.vehicle.wheelFraction[tireIdx] = v5;
                if (tireIdx == cgArray[0].vehicleFrame % 4)
                {
                    fxInfo->tireActive[tireIdx] = 1;
                    Vec3Lerp(traceStart, traceEnd, trace.fraction, fxInfo->tireGroundPoint[tireIdx]);
                    fxInfo->tireGroundSurfType[tireIdx] = (trace.surfaceFlags & 0x1F00000) >> 20;
                }
            }
        }
    }
    else
    {
        for (tireIdx = 0; tireIdx < 4; ++tireIdx)
            cent->pose.vehicle.wheelBoneIndex[tireIdx] = -2;
    }
}

unsigned __int16 __cdecl CompressUnit(float unit)
{
    if (unit < 0.0 || unit > 1.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\bgame\\../cgame_mp/cg_pose_mp.h",
            102,
            0,
            "%s\n\t(unit) = %g",
            "(unit >= 0.0f && unit <= 1.0f)",
            unit);
    return (int)(unit * 65535.0 + 0.5);
}

void __cdecl VehicleFXTest(int localClientNum, const DObj_s *obj, centity_s *cent, vehfx_t *fxInfo)
{
    char *v4; // eax
    const char *v5; // eax
    entityState_s_un1 v6; // [esp+18h] [ebp-10Ch]
    float v7; // [esp+1Ch] [ebp-108h]
    float lerp; // [esp+20h] [ebp-104h]
    float v9; // [esp+24h] [ebp-100h]
    float v10; // [esp+28h] [ebp-FCh]
    float v11; // [esp+2Ch] [ebp-F8h]
    float diff[3]; // [esp+3Ch] [ebp-E8h] BYREF
    float v13; // [esp+48h] [ebp-DCh]
    float v14; // [esp+4Ch] [ebp-D8h]
    float v15; // [esp+50h] [ebp-D4h]
    int v16; // [esp+54h] [ebp-D0h]
    int startMsec; // [esp+58h] [ebp-CCh]
    const snd_alias_t *idleAlias0; // [esp+60h] [ebp-C4h]
    SndEntHandle sndEnt; // [esp+64h] [ebp-C0h]
    float sndLerp; // [esp+68h] [ebp-BCh]
    const snd_alias_t *idleAlias1; // [esp+6Ch] [ebp-B8h]
    const snd_alias_t *engineAlias1; // [esp+70h] [ebp-B4h]
    const snd_alias_t *engineAlias0; // [esp+74h] [ebp-B0h]
    bool result; // [esp+7Ah] [ebp-AAh]
    unsigned __int8 boneIndex; // [esp+7Bh] [ebp-A9h]
    int entityNum; // [esp+7Ch] [ebp-A8h]
    const cg_s *cgameGlob; // [esp+80h] [ebp-A4h]
    float mins[3]; // [esp+84h] [ebp-A0h] BYREF
    float dist; // [esp+90h] [ebp-94h]
    float end[3]; // [esp+94h] [ebp-90h] BYREF
    trace_t trace; // [esp+A0h] [ebp-84h] BYREF
    float maxs[3]; // [esp+CCh] [ebp-58h] BYREF
    int tireIdx; // [esp+D8h] [ebp-4Ch]
    vehicleEffects *vehFx; // [esp+DCh] [ebp-48h]
    float speed; // [esp+E0h] [ebp-44h]
    float groundpos[3]; // [esp+E4h] [ebp-40h] BYREF
    int nextDustInc; // [esp+F0h] [ebp-34h]
    const FxEffectDef *fx; // [esp+F4h] [ebp-30h]
    float axis[3][3]; // [esp+F8h] [ebp-2Ch] BYREF
    int tag; // [esp+11Ch] [ebp-8h]
    const entityState_s *ns; // [esp+120h] [ebp-4h]

    if (!fxInfo)
        MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 545, 0, "%s", "fxInfo");
    ns = &cent->nextState;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgameGlob = cgArray;
    speed = GetSpeed(localClientNum, cent);
    if (cent->nextState.eType == 14)
    {
        if (speed >= 0.050000001 && speed <= 0.60000002)
        {
            for (tireIdx = 0; tireIdx < 4; ++tireIdx)
            {
                if (tireIdx == cgameGlob->vehicleFrame % 4
                    && fxInfo->tireActive[tireIdx]
                    && fxInfo->tireGroundSurfType[tireIdx])
                {
                    fx = cgMedia.fxVehicleTireDust;
                    MatrixIdentity33(axis);
                    if (localClientNum)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                            1071,
                            0,
                            "%s\n\t(localClientNum) = %i",
                            "(localClientNum == 0)",
                            localClientNum);
                    startMsec = cgArray[0].time;
                    FX_PlayOrientedEffect(localClientNum, fx, cgArray[0].time, fxInfo->tireGroundPoint[tireIdx], axis);
                    if (vehDebugClient->current.enabled)
                    {
                        v4 = va("#%i", fxInfo->tireGroundSurfType[tireIdx]);
                        CG_DebugStarWithText(fxInfo->tireGroundPoint[tireIdx], colorWhite, colorWhite, v4, 1.0, 4);
                    }
                }
            }
        }
        if (fxInfo->soundEnabled)
            result = CG_ShouldPlaySoundOnLocalClient();
        else
            result = 0;
        if (result)
        {
            engineAlias0 = CL_PickSoundAlias("hummer_engine_low");
            engineAlias1 = CL_PickSoundAlias("hummer_engine_high");
            idleAlias0 = CL_PickSoundAlias("hummer_idle_low");
            idleAlias1 = CL_PickSoundAlias("hummer_idle_high");
            if (engineAlias0)
            {
                if (engineAlias1 && idleAlias0 && idleAlias1)
                {
                    sndLerp = speed / 0.80000001;
                    v11 = 0.0 - sndLerp;
                    if (v11 < 0.0)
                        v10 = sndLerp;
                    else
                        v10 = 0.0;
                    sndLerp = v10;
                    v9 = v10 - 1.0;
                    if (v9 < 0.0)
                        lerp = sndLerp;
                    else
                        lerp = 1.0;
                    sndLerp = lerp;
                    v16 = 0;
                    sndEnt.field.entIndex = ns->number;
                    SND_PlayBlendedSoundAliases(
                        engineAlias0,
                        engineAlias1,
                        lerp,
                        1.0,
                        sndEnt,
                        fxInfo->soundEngineOrigin,
                        0,
                        SASYS_CGAME);
                    v7 = 1.0 - lerp;
                    SND_PlayBlendedSoundAliases(
                        idleAlias0,
                        idleAlias1,
                        v7,
                        1.0,
                        sndEnt,
                        fxInfo->soundEngineOrigin,
                        0,
                        SASYS_CGAME);
                }
            }
        }
        return;
    }
    if (cent->nextState.eType == 12)
    {
        entityNum = CG_GetEntityIndex(localClientNum, cent);
        vehFx = VehicleGetFxInfo(localClientNum, entityNum);
        if (vehFx->nextDustFx <= (signed int)Sys_Milliseconds())
        {
            mins[0] = -0.5;
            mins[1] = -0.5;
            mins[2] = -0.5;
            maxs[0] = 0.5;
            maxs[1] = 0.5;
            maxs[2] = 0.5;
            v13 = cent->currentState.pos.trBase[0];
            v14 = cent->currentState.pos.trBase[1];
            v15 = cent->currentState.pos.trBase[2] - 1200.0;
            end[0] = v13;
            end[1] = v14;
            end[2] = v15;
            CG_TraceCapsule(&trace, cent->currentState.pos.trBase, mins, maxs, end, entityNum, 2097);
            nextDustInc = 1000;
            if (trace.fraction < 1.0)
            {
                Vec3Lerp(cent->currentState.pos.trBase, end, trace.fraction, groundpos);
                Vec3Sub(groundpos, cent->currentState.pos.trBase, diff);
                dist = Vec3Length(diff);
                if (dist < 1200.0)
                {
                    nextDustInc = (int)(((dist - 350.0) / 850.0 * 0.1000000089406967 + 0.05000000074505806) * 1000.0);
                    axis[0][0] = 0.0;
                    axis[0][1] = 0.0;
                    axis[0][2] = 1.0;
                    Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                    if ((FxMarksSystem *)(trace.surfaceFlags & 0x1F00000) == (FxMarksSystem *)&fx_marksSystemPool[0].pointGroups[930].pointGroup.points[0].xyz[2])
                        fx = cgMedia.heliWaterEffect;
                    else
                        fx = cgMedia.heliDustEffect;
                    if (localClientNum)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                            1071,
                            0,
                            "%s\n\t(localClientNum) = %i",
                            "(localClientNum == 0)",
                            localClientNum);
                    FX_PlayOrientedEffect(localClientNum, fx, cgArray[0].time, groundpos, axis);
                }
            }
            vehFx->nextDustFx = nextDustInc + Sys_Milliseconds();
        }
        if (cent->nextState.un1.scale != 3 && vehFx->nextSmokeFx <= (signed int)Sys_Milliseconds())
        {
            v6.scale = (int)cent->nextState.un1;
            switch (v6.scale)
            {
            case 0:
                fx = cgMedia.helicopterOnFire;
                tag = scr_const.tag_engine_left;
                DObjGetBoneIndex(obj, scr_const.tag_engine_left, &vehFx->tag_engine_left);
                boneIndex = vehFx->tag_engine_left;
                goto LABEL_59;
            case 1:
                fx = cgMedia.helicopterHeavySmoke;
                tag = scr_const.tag_engine_left;
                DObjGetBoneIndex(obj, scr_const.tag_engine_left, &vehFx->tag_engine_left);
                boneIndex = vehFx->tag_engine_left;
                goto LABEL_59;
            case 2:
                fx = cgMedia.helicopterLightSmoke;
                tag = scr_const.tag_engine_right;
                DObjGetBoneIndex(obj, scr_const.tag_engine_right, &vehFx->tag_engine_right);
                boneIndex = vehFx->tag_engine_right;
            LABEL_59:
                if (localClientNum)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
                        1071,
                        0,
                        "%s\n\t(localClientNum) = %i",
                        "(localClientNum == 0)",
                        localClientNum);
                FX_PlayBoltedEffect(localClientNum, fx, cgArray[0].time, cent->nextState.number, boneIndex);
                vehFx->nextSmokeFx = Sys_Milliseconds() + 50;
                return;
            }
            if (!alwaysfails)
            {
                v5 = va("Unhandled helicopter stage %i", cent->nextState.un1.scale);
                MyAssertHandler(".\\cgame_mp\\cg_vehicles_mp.cpp", 698, 0, v5);
            }
        }
    }
}

double __cdecl GetSpeed(int localClientNum, centity_s *cent)
{
    int serverTimeDelta; // [esp+Ch] [ebp-1Ch]
    float posDelta[3]; // [esp+10h] [ebp-18h] BYREF
    float len; // [esp+1Ch] [ebp-Ch]
    LerpEntityState *p_currentState; // [esp+20h] [ebp-8h]
    entityState_s *ns; // [esp+24h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    p_currentState = &cent->currentState;
    ns = &cent->nextState;
    serverTimeDelta = cgArray[0].nextSnap->serverTime - cgArray[0].snap->serverTime;
    Vec3Sub(cent->nextState.lerp.pos.trBase, cent->currentState.pos.trBase, posDelta);
    len = Vec3Length(posDelta);
    if ((double)serverTimeDelta <= 0.0)
        return 0.0;
    return (float)(len / (double)serverTimeDelta);
}

void __cdecl CG_VehSphereCoordsToPos(float sphereDistance, float sphereYaw, float sphereAltitude, float *result)
{
    float v4; // [esp+8h] [ebp-20h]
    float v5; // [esp+14h] [ebp-14h]
    float altitudeSin; // [esp+18h] [ebp-10h]
    float altitudeCos; // [esp+1Ch] [ebp-Ch]
    float yawSin; // [esp+20h] [ebp-8h]
    float yawCos; // [esp+24h] [ebp-4h]

    v5 = (90.0 - sphereAltitude) * 0.01745329238474369;
    altitudeCos = cos(v5);
    altitudeSin = sin(v5);
    v4 = (sphereYaw - 90.0) * 0.01745329238474369;
    yawCos = cos(v4);
    yawSin = sin(v4);
    *result = sphereDistance * yawCos * altitudeSin;
    result[1] = sphereDistance * yawSin * altitudeSin;
    result[2] = sphereDistance * altitudeCos;
}

void __cdecl CG_Veh_Init()
{
    memset((unsigned __int8 *)vehEffects, 0, sizeof(vehEffects));
}

