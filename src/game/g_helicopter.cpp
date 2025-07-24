#include "game_public.h"
#include <script/scr_vm.h>
#include <game_mp/g_public_mp.h>
#include <script/scr_const.h>
#include <game_mp/g_utils_mp.h>


void __cdecl VEH_CheckForPredictedCrash(gentity_s *ent)
{
    vehicle_physic_t *phys; // [esp+Ch] [ebp-40h]
    trace_t trace; // [esp+14h] [ebp-38h] BYREF
    float targetPos[3]; // [esp+40h] [ebp-Ch] BYREF

    if (!ent)
        MyAssertHandler(".\\game\\g_helicopter.cpp", 448, 0, "%s", "ent");
    if (!ent->scr_vehicle)
        MyAssertHandler(".\\game\\g_helicopter.cpp", 449, 0, "%s", "ent->scr_vehicle");
    phys = &ent->scr_vehicle->phys;
    if (vehHelicopterLookaheadTime->current.value != 0.0
        && (ent->scr_vehicle->phys.vel[0] != 0.0
            || ent->scr_vehicle->phys.vel[1] != 0.0
            || ent->scr_vehicle->phys.vel[2] != 0.0))
    {
        Vec3Mad(
            ent->scr_vehicle->phys.origin,
            vehHelicopterLookaheadTime->current.value,
            ent->scr_vehicle->phys.vel,
            targetPos);
        G_TraceCapsule(&trace, phys->origin, phys->mins, phys->maxs, targetPos, ent->s.number, ent->clipmask);
        if (trace.fraction < 1.0)
        {
            Scr_AddVector(trace.normal);
            Scr_AddVector(phys->vel);
            Scr_Notify(ent, scr_const.veh_predictedcollision, 2u);
        }
    }
}

void __cdecl VEH_UpdateClientChopper(gentity_s *ent)
{
    bool v1; // eax
    float v3; // [esp+14h] [ebp-D0h]
    float v4; // [esp+18h] [ebp-CCh]
    float v5; // [esp+44h] [ebp-A0h]
    float v6; // [esp+48h] [ebp-9Ch]
    vehicle_physic_t *phys; // [esp+4Ch] [ebp-98h]
    scr_vehicle_s *veh; // [esp+54h] [ebp-90h]
    char move[7]; // [esp+58h] [ebp-8Ch] BYREF
    bool bumped; // [esp+5Fh] [ebp-85h]
    float rotAccel[3]; // [esp+60h] [ebp-84h] BYREF
    float startPos[3]; // [esp+6Ch] [ebp-78h] BYREF
    float collision[3]; // [esp+78h] [ebp-6Ch] BYREF
    float bodyAccel[3]; // [esp+84h] [ebp-60h] BYREF
    float worldAccel[3]; // [esp+90h] [ebp-54h] BYREF
    float axis[4][3]; // [esp+9Ch] [ebp-48h] BYREF
    float yawAngles[3]; // [esp+CCh] [ebp-18h] BYREF
    float startVel[3]; // [esp+D8h] [ebp-Ch] BYREF

    move[0] = 0;
    move[1] = 0;
    move[2] = 0;
    move[3] = 0;
    if (!ent)
        MyAssertHandler(".\\game\\g_helicopter.cpp", 491, 0, "%s", "ent");
    if (!ent->scr_vehicle)
        MyAssertHandler(".\\game\\g_helicopter.cpp", 492, 0, "%s", "ent->scr_vehicle");
    veh = ent->scr_vehicle;
    phys = &veh->phys;
    VEH_GetVehicleInfo(veh->infoIdx);
    HELI_CalcAccel(ent, move, bodyAccel, rotAccel);
    veh->phys.rotVel[1] = rotAccel[1] * 0.05000000074505806 + veh->phys.rotVel[1];
    v5 = veh->phys.rotVel[1] * 0.05000000074505806 + veh->phys.prevAngles[1];
    v6 = v5 * 0.002777777845039964;
    v4 = v6 + 0.5;
    v3 = floor(v4);
    veh->phys.angles[1] = (v6 - v3) * 360.0;
    veh->phys.angles[0] = rotAccel[0];
    veh->phys.angles[2] = rotAccel[2];
    if ((COERCE_UNSIGNED_INT(veh->phys.angles[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(veh->phys.angles[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(veh->phys.angles[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game\\g_helicopter.cpp",
            577,
            0,
            "%s",
            "!IS_NAN((phys->angles)[0]) && !IS_NAN((phys->angles)[1]) && !IS_NAN((phys->angles)[2])");
    }
    yawAngles[0] = 0.0f;
    yawAngles[1] = veh->phys.angles[1];
    yawAngles[2] = 0.0f;
    AngleVectors(yawAngles, axis[0], axis[1], axis[2]);
    axis[3][0] = 0.0f;
    axis[3][1] = 0.0f;
    axis[3][2] = 0.0f;
    MatrixTransformVector(bodyAccel, *(const mat3x3*)axis, worldAccel);
    if (vehHelicopterSoftCollisions->current.enabled)
        HELI_SoftenCollisions(ent, worldAccel);
    Vec3Mad(veh->phys.vel, 0.050000001f, worldAccel, veh->phys.vel);
    if (0.0 != veh->phys.vel[0] || 0.0 != veh->phys.vel[1] || 0.0 != veh->phys.vel[2])
    {
        startVel[0] = veh->phys.vel[0];
        startVel[1] = veh->phys.vel[1];
        startVel[2] = veh->phys.vel[2];
        startPos[0] = phys->origin[0];
        startPos[1] = veh->phys.origin[1];
        startPos[2] = veh->phys.origin[2];
        VEH_ClearGround();
        v1 = VEH_SlideMove(ent, 0);
        bumped = v1;
        if (v1)
        {
            Vec3Mad(startPos, 0.050000001f, startVel, collision);
            Vec3Sub(phys->origin, collision, collision);
            if (Vec3Normalize(collision) > 0.0)
            {
                Scr_AddVector(collision);
                Scr_AddVector(startVel);
                Scr_Notify(ent, scr_const.veh_collision, 2u);
            }
        }
    }
    VEH_CheckForPredictedCrash(ent);
    MatrixTransposeTransformVector43(veh->phys.vel, axis, veh->phys.bodyVel);
    veh->speed = Vec3Length(veh->phys.vel);
    if (veh->speed < 0.0f)
        MyAssertHandler(".\\game\\g_helicopter.cpp", 622, 0, "%s", "veh->speed >= 0.0f");
    if (move[2] > 0)
    {
        veh->idleSndLerp = DiffTrack(0.0, veh->idleSndLerp, 4.0f, 0.050000001f);
        veh->engineSndLerp = DiffTrack(1.0, veh->engineSndLerp, 4.0f, 0.050000001f);
    }
    else
    {
        veh->idleSndLerp = DiffTrack(1.0, veh->idleSndLerp, 4.0f, 0.050000001f);
        veh->engineSndLerp = DiffTrack(0.0, veh->engineSndLerp, 4.0f, 0.050000001f);
    }
}

void __cdecl HELI_CalcAccel(gentity_s *ent, char *move, float *bodyAccel, float *rotAccel)
{
    float scale; // [esp+4h] [ebp-29Ch]
    float v5; // [esp+10h] [ebp-290h]
    float v6; // [esp+14h] [ebp-28Ch]
    float v7; // [esp+18h] [ebp-288h]
    float v8; // [esp+1Ch] [ebp-284h]
    float v9; // [esp+20h] [ebp-280h]
    float v10; // [esp+24h] [ebp-27Ch]
    float v11; // [esp+28h] [ebp-278h]
    float v12; // [esp+2Ch] [ebp-274h]
    float v13; // [esp+30h] [ebp-270h]
    float v14; // [esp+34h] [ebp-26Ch]
    float v15; // [esp+38h] [ebp-268h]
    float v16; // [esp+3Ch] [ebp-264h]
    float tgt; // [esp+40h] [ebp-260h]
    float v18; // [esp+44h] [ebp-25Ch]
    float v19; // [esp+48h] [ebp-258h]
    float v20; // [esp+4Ch] [ebp-254h]
    float v21; // [esp+50h] [ebp-250h]
    float v22; // [esp+54h] [ebp-24Ch]
    float v23; // [esp+58h] [ebp-248h]
    float v24; // [esp+5Ch] [ebp-244h]
    float v25; // [esp+60h] [ebp-240h]
    float v26; // [esp+64h] [ebp-23Ch]
    float v27; // [esp+68h] [ebp-238h]
    float v28; // [esp+6Ch] [ebp-234h]
    float v29; // [esp+70h] [ebp-230h]
    float v30; // [esp+74h] [ebp-22Ch]
    float v31; // [esp+78h] [ebp-228h]
    float v32; // [esp+7Ch] [ebp-224h]
    float rate; // [esp+80h] [ebp-220h]
    float *worldTilt; // [esp+A4h] [ebp-1FCh]
    float *v35; // [esp+A8h] [ebp-1F8h]
    float *worldTiltVel; // [esp+B0h] [ebp-1F0h]
    float *v37; // [esp+B4h] [ebp-1ECh]
    float v38; // [esp+C0h] [ebp-1E0h]
    float v39; // [esp+C4h] [ebp-1DCh]
    float v40; // [esp+CCh] [ebp-1D4h]
    float v41; // [esp+D4h] [ebp-1CCh]
    float v42; // [esp+DCh] [ebp-1C4h]
    float v43; // [esp+E0h] [ebp-1C0h]
    float v44; // [esp+E4h] [ebp-1BCh]
    float v45; // [esp+E8h] [ebp-1B8h]
    float v46; // [esp+ECh] [ebp-1B4h]
    float v47; // [esp+F0h] [ebp-1B0h]
    float v48; // [esp+F4h] [ebp-1ACh]
    float v49; // [esp+F8h] [ebp-1A8h]
    float v50; // [esp+FCh] [ebp-1A4h]
    float v51; // [esp+100h] [ebp-1A0h]
    float v52; // [esp+104h] [ebp-19Ch]
    float v53; // [esp+108h] [ebp-198h]
    float v54; // [esp+10Ch] [ebp-194h]
    float v55; // [esp+110h] [ebp-190h]
    float v56; // [esp+114h] [ebp-18Ch]
    float v57; // [esp+118h] [ebp-188h]
    float v58; // [esp+11Ch] [ebp-184h]
    float v59; // [esp+120h] [ebp-180h]
    float v60; // [esp+124h] [ebp-17Ch]
    float v61; // [esp+128h] [ebp-178h]
    float v62; // [esp+12Ch] [ebp-174h]
    float v63; // [esp+130h] [ebp-170h]
    float value; // [esp+138h] [ebp-168h]
    float v65; // [esp+13Ch] [ebp-164h]
    float v66; // [esp+144h] [ebp-15Ch]
    float v67; // [esp+148h] [ebp-158h]
    float v68; // [esp+150h] [ebp-150h]
    float frac; // [esp+16Ch] [ebp-134h]
    float velScale; // [esp+170h] [ebp-130h]
    float velScalea; // [esp+170h] [ebp-130h]
    float yawScale; // [esp+174h] [ebp-12Ch]
    float noYawAngles[3]; // [esp+178h] [ebp-128h] BYREF
    float speedParallel; // [esp+184h] [ebp-11Ch]
    vehicle_physic_t *phys; // [esp+188h] [ebp-118h]
    vehicle_info_t *info; // [esp+18Ch] [ebp-114h]
    float tgtSpeed; // [esp+190h] [ebp-110h]
    float velParallel[3]; // [esp+194h] [ebp-10Ch] BYREF
    float deltaTilt[2]; // [esp+1A0h] [ebp-100h]
    scr_vehicle_s *veh; // [esp+1A8h] [ebp-F8h]
    float tgtDir[3]; // [esp+1ACh] [ebp-F4h] BYREF
    float targetTilt[3]; // [esp+1B8h] [ebp-E8h] BYREF
    float tgtVel[3]; // [esp+1C4h] [ebp-DCh] BYREF
    float controllerFrac[4]; // [esp+1D0h] [ebp-D0h] BYREF
    float oldTiltVel[2]; // [esp+1E0h] [ebp-C0h]
    float speedSq; // [esp+1E8h] [ebp-B8h]
    float tiltAccel[2]; // [esp+1ECh] [ebp-B4h]
    float tgtYawVel; // [esp+1F4h] [ebp-ACh]
    float track[4]; // [esp+1F8h] [ebp-A8h]
    float decel[2]; // [esp+208h] [ebp-98h]
    float maxSpeed[3]; // [esp+210h] [ebp-90h] BYREF
    float nextState; // [esp+21Ch] [ebp-84h]
    float bodyMat[4][3]; // [esp+220h] [ebp-80h] BYREF
    float newAccel[2]; // [esp+250h] [ebp-50h]
    float upVec[3]; // [esp+258h] [ebp-48h] BYREF
    int axis; // [esp+264h] [ebp-3Ch]
    float yawAngles[3]; // [esp+268h] [ebp-38h] BYREF
    float maxAccel[3]; // [esp+274h] [ebp-2Ch]
    float worldTargetTilt[3]; // [esp+280h] [ebp-20h] BYREF
    float velOrthogonal[3]; // [esp+28Ch] [ebp-14h] BYREF
    float newDecel[2]; // [esp+298h] [ebp-8h]

    veh = ent->scr_vehicle;
    phys = &veh->phys;
    info = VEH_GetVehicleInfo(veh->infoIdx);
    if (veh->joltTime <= 0.0)
    {
        v67 = vehHelicopterMaxSpeed->current.value * 17.6;
        v68 = vehHelicopterMaxSpeedVertical->current.value * 17.6;
        maxSpeed[0] = v67;
        maxSpeed[1] = v67;
        maxSpeed[2] = v68;
        v65 = vehHelicopterMaxAccel->current.value * 17.6;
        v66 = vehHelicopterMaxAccelVertical->current.value * 17.6;
        maxAccel[0] = v65;
        maxAccel[1] = v65;
        maxAccel[2] = v66;
    }
    else
    {
        maxSpeed[0] = veh->joltSpeed;
        maxSpeed[1] = maxSpeed[0];
        maxSpeed[2] = maxSpeed[0];
        maxAccel[0] = veh->joltDecel;
        maxAccel[1] = maxAccel[0];
        maxAccel[2] = maxAccel[0];
    }
    HELI_CmdScale(move, controllerFrac);
    for (axis = 0; axis < 3; ++axis)
    {
        if (maxSpeed[axis] == 0.0)
            track[axis] = 1.0;
        else
            track[axis] = maxAccel[axis] / maxSpeed[axis];
    }
    Vec3Mul(maxSpeed, controllerFrac, tgtVel);
    noYawAngles[1] = 0.0;
    noYawAngles[0] = phys->prevAngles[0];
    noYawAngles[2] = phys->prevAngles[2];
    AngleVectors(noYawAngles, 0, 0, upVec);
    tgtVel[0] = tgtVel[2] * upVec[0] + tgtVel[0];
    tgtVel[1] = tgtVel[2] * upVec[1] + tgtVel[1];
    tgtVel[2] = tgtVel[2] * upVec[2];
    tgtSpeed = Vec3NormalizeTo(tgtVel, tgtDir);
    speedParallel = Vec3Dot(phys->bodyVel, tgtDir);
    scale = -speedParallel;
    Vec3Mad(phys->bodyVel, scale, tgtDir, velOrthogonal);
    Vec3Scale(tgtDir, speedParallel, velParallel);
    value = vehHelicopterDecelerationSide->current.value;
    decel[0] = vehHelicopterDecelerationFwd->current.value;
    decel[1] = value;
    newAccel[0] = 0.0;
    newAccel[1] = 0.0;
    newDecel[0] = 0.0;
    newDecel[1] = 0.0;
    for (axis = 0; axis < 2; ++axis)
    {
        rate = decel[axis] * track[axis];
        nextState = DiffTrack(0.0, velOrthogonal[axis], rate, 0.050000001f);
        newDecel[axis] = (nextState - velOrthogonal[axis]) / 0.05000000074505806f;
        if (speedParallel >= tgtSpeed)
        {
            v32 = decel[axis] * track[axis];
            nextState = DiffTrack(tgtVel[axis], velParallel[axis], v32, 0.050000001f);
            newDecel[axis] = (nextState - velParallel[axis]) / 0.05000000074505806f + newDecel[axis];
        }
        else
        {
            nextState = DiffTrack(tgtVel[axis], velParallel[axis], track[axis], 0.050000001f);
            newAccel[axis] = (nextState - velParallel[axis]) / 0.05000000074505806f + newAccel[axis];
        }
        bodyAccel[axis] = newDecel[axis] + newAccel[axis];
        v60 = bodyAccel[axis];
        v62 = maxAccel[axis];
        v31 = v60 - v62;
        if (v31 < 0.0)
            v63 = v60;
        else
            v63 = v62;
        v61 = -maxAccel[axis];
        v30 = v61 - v60;
        if (v30 < 0.0)
            v29 = v63;
        else
            v29 = -maxAccel[axis];
        bodyAccel[axis] = v29;
        v56 = newDecel[axis];
        v58 = maxAccel[axis];
        v28 = v56 - v58;
        if (v28 < 0.0)
            v59 = v56;
        else
            v59 = v58;
        v57 = -maxAccel[axis];
        v27 = v57 - v56;
        if (v27 < 0.0)
            v26 = v59;
        else
            v26 = -maxAccel[axis];
        newDecel[axis] = v26;
        v52 = newAccel[axis];
        v54 = maxAccel[axis];
        v25 = v52 - v54;
        if (v25 < 0.0)
            v55 = v52;
        else
            v55 = v54;
        v53 = -maxAccel[axis];
        v24 = v53 - v52;
        if (v24 < 0.0)
            v23 = v55;
        else
            v23 = -maxAccel[axis];
        newAccel[axis] = v23;
    }
    nextState = DiffTrack(tgtVel[2], phys->bodyVel[2], track[2], 0.050000001f);
    bodyAccel[2] = (nextState - phys->bodyVel[2]) / 0.05000000074505806f;
    v49 = bodyAccel[2];
    v22 = v49 - maxAccel[2];
    if (v22 < 0.0)
        v51 = v49;
    else
        v51 = maxAccel[2];
    v50 = -maxAccel[2];
    v21 = v50 - v49;
    if (v21 < 0.0f)
        v20 = v51;
    else
        v20 = -maxAccel[2];
    bodyAccel[2] = v20;
    if (vehHelicopterMaxYawRate->current.value <= 0.0f)
        MyAssertHandler(".\\game\\g_helicopter.cpp", 209, 0, "%s", "vehHelicopterMaxYawRate->current.value > 0.0f");
    track[3] = vehHelicopterMaxYawAccel->current.value / vehHelicopterMaxYawRate->current.value;
    tgtYawVel = vehHelicopterMaxYawRate->current.value * controllerFrac[3];
    tgtYawVel = tgtYawVel - controllerFrac[0] * controllerFrac[1] * vehHelicopterYawOnLeftStick->current.value;
    v47 = vehHelicopterMaxYawRate->current.value;
    v19 = tgtYawVel - v47;
    if (v19 < 0.0f)
        v48 = tgtYawVel;
    else
        v48 = v47;
    v46 = -vehHelicopterMaxYawRate->current.value;
    v18 = v46 - tgtYawVel;
    if (v18 < 0.0f)
        tgt = v48;
    else
        tgt = -vehHelicopterMaxYawRate->current.value;
    tgtYawVel = tgt;
    nextState = DiffTrack(tgt, phys->rotVel[1], track[3], 0.050000001f);
    rotAccel[1] = (nextState - phys->rotVel[1]) / 0.05000000074505806f;
    v42 = rotAccel[1];
    v44 = vehHelicopterMaxYawAccel->current.value;
    v16 = v42 - v44;
    if (v16 < 0.0f)
        v45 = v42;
    else
        v45 = v44;
    v43 = -vehHelicopterMaxYawAccel->current.value;
    v15 = v43 - v42;
    if (v15 < 0.0f)
        v14 = v45;
    else
        v14 = -vehHelicopterMaxYawAccel->current.value;
    rotAccel[1] = v14;
    for (axis = 0; axis < 2; ++axis)
    {
        if (maxSpeed[axis] <= 0.0f)
        {
            if (vehHelicopterMaxSpeed->current.value <= 0.0f)
                MyAssertHandler(".\\game\\g_helicopter.cpp", 227, 0, "%s", "vehHelicopterMaxSpeed->current.value > 0.0f");
            maxSpeed[axis] = 17.6f * vehHelicopterMaxSpeed->current.value;
        }
        if (maxAccel[axis] <= 0.0f)
        {
            if (vehHelicopterMaxAccel->current.value <= 0.0f)
                MyAssertHandler(".\\game\\g_helicopter.cpp", 232, 0, "%s", "vehHelicopterMaxAccel->current.value > 0.0f");
            maxAccel[axis] = 17.6f * vehHelicopterMaxAccel->current.value;
        }
    }
    yawAngles[0] = 0.0f;
    yawAngles[1] = phys->angles[1];
    yawAngles[2] = 0.0f;
    AngleVectors(yawAngles, bodyMat[0], bodyMat[1], bodyMat[2]);
    bodyMat[3][0] = 0.0f;
    bodyMat[3][1] = 0.0f;
    bodyMat[3][2] = 0.0f;
    targetTilt[0] = phys->bodyVel[0] / maxSpeed[0] * vehHelicopterTiltFromVelocity->current.value;
    targetTilt[0] = tgtVel[0] / maxSpeed[0] * vehHelicopterTiltFromControllerAxes->current.value + targetTilt[0];
    targetTilt[0] = newAccel[0] / maxAccel[0] * vehHelicopterTiltFromAcceleration->current.value + targetTilt[0];
    targetTilt[0] = newDecel[0] / maxAccel[0] * vehHelicopterTiltFromDeceleration->current.value + targetTilt[0];
    v13 = targetTilt[0] - 1.0f;
    if (v13 < 0.0f)
        v41 = targetTilt[0];
    else
        v41 = 1.0f;
    v12 = -1.0f - targetTilt[0];
    if (v12 < 0.0f)
        v11 = v41;
    else
        v11 = -1.0f;
    targetTilt[0] = v11;
    targetTilt[1] = phys->bodyVel[1] / maxSpeed[1] * vehHelicopterTiltFromVelocity->current.value;
    targetTilt[1] = tgtVel[1] / maxSpeed[1] * vehHelicopterTiltFromControllerAxes->current.value + targetTilt[1];
    targetTilt[1] = newAccel[1] / maxAccel[1] * vehHelicopterTiltFromAcceleration->current.value + targetTilt[1];
    targetTilt[1] = newDecel[1] / maxAccel[1] * vehHelicopterTiltFromDeceleration->current.value + targetTilt[1];
    if (phys->bodyVel[0] > 0.0)
    {
        v10 = I_fabs(phys->rotVel[1]);
        if (v10 > 0.0)
        {
            if (maxSpeed[0] <= 0.0f)
                MyAssertHandler(".\\game\\g_helicopter.cpp", 259, 0, "%s", "maxSpeed[0] > 0");
            velScale = phys->bodyVel[0] / maxSpeed[0];
            if (vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt->current.value <= (double)velScale)
            {
                velScalea = 1.0f;
            }
            else
            {
                if (vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt->current.value <= 0.0f)
                    MyAssertHandler(
                        ".\\game\\g_helicopter.cpp",
                        263,
                        1,
                        "%s",
                        "vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt->current.value > 0");
                velScalea = velScale / vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt->current.value;
            }
            if (vehHelicopterMaxYawRate->current.value <= 0.0)
                MyAssertHandler(".\\game\\g_helicopter.cpp", 269, 0, "%s", "vehHelicopterMaxYawRate->current.value > 0.0f");
            yawScale = -phys->rotVel[1] / vehHelicopterMaxYawRate->current.value;
            targetTilt[1] = vehHelicopterTiltFromFwdAndYaw->current.value * velScalea * yawScale + targetTilt[1];
        }
    }
    v9 = targetTilt[1] - 1.0f;
    if (v9 < 0.0f)
        v40 = targetTilt[1];
    else
        v40 = 1.0f;
    v8 = -1.0f - targetTilt[1];
    if (v8 < 0.0f)
        v7 = v40;
    else
        v7 = -1.0f;
    targetTilt[1] = v7;
    targetTilt[2] = 0.0f;
    MatrixTransformVector(targetTilt, *(const mat3x3*)bodyMat, worldTargetTilt);
    deltaTilt[0] = worldTargetTilt[0] - phys->worldTilt[0];
    deltaTilt[1] = worldTargetTilt[1] - phys->worldTilt[1];
    if (vehHelicopterTiltMomentum->current.value == 0.0f)
        MyAssertHandler(".\\game\\g_helicopter.cpp", 281, 0, "%s", "vehHelicopterTiltMomentum->current.value");
    v39 = vehHelicopterTiltSpeed->current.value / vehHelicopterTiltMomentum->current.value;
    tiltAccel[0] = v39 * deltaTilt[0];
    tiltAccel[1] = v39 * deltaTilt[1];
    v38 = -1.0f / vehHelicopterTiltMomentum->current.value;
    tiltAccel[0] = v38 * phys->worldTiltVel[0] + tiltAccel[0];
    tiltAccel[1] = v38 * phys->worldTiltVel[1] + tiltAccel[1];
    oldTiltVel[0] = phys->worldTiltVel[0];
    oldTiltVel[1] = phys->worldTiltVel[1];
    worldTiltVel = phys->worldTiltVel;
    v37 = phys->worldTiltVel;
    phys->worldTiltVel[0] = 0.050000001f * tiltAccel[0] + phys->worldTiltVel[0];
    worldTiltVel[1] = 0.050000001f * tiltAccel[1] + v37[1];
    oldTiltVel[0] = (oldTiltVel[0] + phys->worldTiltVel[0]) * 0.5;
    oldTiltVel[1] = (oldTiltVel[1] + phys->worldTiltVel[1]) * 0.5;
    worldTilt = phys->worldTilt;
    v35 = phys->worldTilt;
    phys->worldTilt[0] = 0.050000001f * oldTiltVel[0] + phys->worldTilt[0];
    worldTilt[1] = 0.050000001f * oldTiltVel[1] + v35[1];
    for (axis = 0; axis < 2; ++axis)
    {
        if (phys->worldTilt[axis] <= 1.0f)
        {
            if (phys->worldTilt[axis] < -1.0f)
            {
                phys->worldTilt[axis] = -1.0f;
                if (phys->worldTiltVel[axis] < 0.0f)
                    phys->worldTiltVel[axis] = 0.0f;
            }
        }
        else
        {
            phys->worldTilt[axis] = 1.0f;
            if (phys->worldTiltVel[axis] > 0.0f)
                phys->worldTiltVel[axis] = 0.0f;
        }
    }
    MatrixTransposeTransformVector43(phys->worldTilt, bodyMat, targetTilt);
    *rotAccel = targetTilt[0] * vehHelicopterMaxPitch->current.value;
    rotAccel[2] = targetTilt[1] * vehHelicopterMaxRoll->current.value;
    speedSq = phys->bodyVel[1] * phys->bodyVel[1] + phys->bodyVel[0] * phys->bodyVel[0];
    v6 = vehHelicopterHoverSpeedThreshold->current.value * vehHelicopterHoverSpeedThreshold->current.value;
    if (speedSq < (float)v6)
    {
        if (vehHelicopterHoverSpeedThreshold->current.value <= 0.0)
            MyAssertHandler(".\\game\\g_helicopter.cpp", 316, 0, "%s", "vehHelicopterHoverSpeedThreshold->current.value > 0");
        v5 = sqrt(speedSq);
        frac = (vehHelicopterHoverSpeedThreshold->current.value - v5) / vehHelicopterHoverSpeedThreshold->current.value;
        HELI_UpdateJitter(&veh->jitter);
        *rotAccel = frac * veh->jitter.jitterPos[0] + *rotAccel;
        rotAccel[2] = frac * veh->jitter.jitterPos[2] + rotAccel[2];
    }
    if ((COERCE_UNSIGNED_INT(*bodyAccel) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(bodyAccel[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(bodyAccel[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game\\g_helicopter.cpp",
            325,
            0,
            "%s",
            "!IS_NAN((bodyAccel)[0]) && !IS_NAN((bodyAccel)[1]) && !IS_NAN((bodyAccel)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*rotAccel) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(rotAccel[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(rotAccel[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game\\g_helicopter.cpp",
            326,
            0,
            "%s",
            "!IS_NAN((rotAccel)[0]) && !IS_NAN((rotAccel)[1]) && !IS_NAN((rotAccel)[2])");
    }
}

void __cdecl HELI_CmdScale(char *move, float *outFracs)
{
    float v2; // [esp+0h] [ebp-5Ch]
    float v3; // [esp+4h] [ebp-58h]
    float v4; // [esp+8h] [ebp-54h]
    float v5; // [esp+Ch] [ebp-50h]
    float v6; // [esp+10h] [ebp-4Ch]
    float v7; // [esp+14h] [ebp-48h]
    float v8; // [esp+40h] [ebp-1Ch]
    int max; // [esp+44h] [ebp-18h]
    float scale; // [esp+54h] [ebp-8h]
    float scalea; // [esp+54h] [ebp-8h]
    float scaleb; // [esp+54h] [ebp-8h]
    int axis; // [esp+58h] [ebp-4h]

    if (!move)
        MyAssertHandler(".\\game\\g_helicopter.cpp", 19, 0, "%s", "move");
    if (!outFracs)
        MyAssertHandler(".\\game\\g_helicopter.cpp", 20, 0, "%s", "outFracs");
    for (axis = 0; axis < 4; ++axis)
        outFracs[axis] = (float)move[axis] / 127.0f;
    if (*move || move[1])
    {
        v8 = (float)(move[1] * move[1] + *move * *move);
        v7 = sqrt(v8);
        max = abs8(*move);
        if (abs8(move[1]) > max)
            max = abs8(move[1]);
        if (max)
        {
            scale = (float)max / v7;
            *outFracs = scale * *outFracs;
            outFracs[1] = scale * outFracs[1];
        }
        v6 = I_fabs(outFracs[1]);
        if (vehHelicopterStrafeDeadzone->current.value > (float)v6)
            outFracs[1] = 0.0;
        if (vehHelicopterScaleMovement->current.enabled)
        {
            v5 = I_fabs(*outFracs);
            v4 = I_fabs(outFracs[1]);
            if (v5 > 1.0)
                MyAssertHandler(".\\game\\g_helicopter.cpp", 49, 0, "%s", "absAxis[0] <= 1.0f");
            if (v4 > 1.0)
                MyAssertHandler(".\\game\\g_helicopter.cpp", 50, 0, "%s", "absAxis[1] <= 1.0f");
            if (v4 >= (float)v5)
            {
                scaleb = 1.0f - (v4 - v5);
                *outFracs = *outFracs * scaleb;
            }
            else
            {
                scalea = 1.0f - (v5 - v4);
                outFracs[1] = outFracs[1] * scalea;
            }
        }
    }
    v3 = I_fabs(outFracs[2]);
    if (vehHelicopterRightStickDeadzone->current.value > (float)v3)
        outFracs[2] = 0.0f;
    v2 = I_fabs(outFracs[3]);
    if (vehHelicopterRightStickDeadzone->current.value > (float)v2)
        outFracs[3] = 0.0f;
}

void __cdecl HELI_UpdateJitter(VehicleJitter *jitter)
{
    double v1; // st7
    double v2; // st7
    float min; // [esp+8h] [ebp-20h]
    float v4; // [esp+10h] [ebp-18h]
    int jitterDelay; // [esp+14h] [ebp-14h]
    float newOffset[3]; // [esp+18h] [ebp-10h] BYREF
    int i; // [esp+24h] [ebp-4h]

    if (jitter->jitterPeriodMin || jitter->jitterPeriodMax)
    {
        if (level.time > jitter->jitterEndTime)
        {
            if ((double)jitter->jitterPeriodMin < 0.05000000074505806)
                jitter->jitterPeriodMin = 50;
            jitterDelay = G_irand(jitter->jitterPeriodMin, jitter->jitterPeriodMax);
            jitter->jitterEndTime = jitterDelay + level.time;
            for (i = 0; i < 3; ++i)
            {
                if (jitter->jitterOffsetRange[i] != 0.0)
                {
                    min = -jitter->jitterOffsetRange[i];
                    v1 = G_flrand(min, jitter->jitterOffsetRange[i]);
                    newOffset[i] = v1;
                }
            }
            Vec3Sub(newOffset, jitter->jitterAccel, jitter->jitterDeltaAccel);
            if (jitterDelay <= 0)
                MyAssertHandler(".\\game\\g_helicopter.cpp", 95, 0, "%s", "jitterDelay > 0");
            v4 = 50.0 / (double)jitterDelay;
            Vec3Scale(jitter->jitterDeltaAccel, v4, jitter->jitterDeltaAccel);
        }
        Vec3Add(jitter->jitterAccel, jitter->jitterDeltaAccel, jitter->jitterAccel);
        for (i = 0; i < 3; ++i)
        {
            v2 = DiffTrack(
                jitter->jitterAccel[i],
                jitter->jitterPos[i],
                vehHelicopterJitterJerkyness->current.value,
                0.050000001f);
            jitter->jitterPos[i] = v2;
        }
    }
}

void __cdecl HELI_SoftenCollisions(gentity_s *ent, float *worldAccel)
{
    double v2; // st7
    float v3; // [esp+0h] [ebp-78h]
    float v4; // [esp+Ch] [ebp-6Ch]
    float scale; // [esp+10h] [ebp-68h]
    bool clipped; // [esp+1Ch] [ebp-5Ch]
    float errorAccel; // [esp+24h] [ebp-54h]
    scr_vehicle_s *veh; // [esp+28h] [ebp-50h]
    float clippedPos[3]; // [esp+2Ch] [ebp-4Ch] BYREF
    float velChange[3]; // [esp+38h] [ebp-40h] BYREF
    float error[3]; // [esp+44h] [ebp-34h] BYREF
    float wishVel[3]; // [esp+50h] [ebp-28h] BYREF
    float targetPos[3]; // [esp+5Ch] [ebp-1Ch] BYREF
    float errorMagSqr; // [esp+68h] [ebp-10h]
    float oldVel[3]; // [esp+6Ch] [ebp-Ch]

    if (!ent)
        MyAssertHandler(".\\game\\g_helicopter.cpp", 400, 0, "%s", "ent");
    if (!ent->scr_vehicle)
        MyAssertHandler(".\\game\\g_helicopter.cpp", 401, 0, "%s", "ent->scr_vehicle");
    veh = ent->scr_vehicle;
    if (veh->phys.vel[0] != 0.0f || veh->phys.vel[1] != 0.0f || veh->phys.vel[2] != 0.0f)
    {
        oldVel[0] = veh->phys.vel[0];
        oldVel[1] = veh->phys.vel[1];
        oldVel[2] = veh->phys.vel[2];
        scale = vehHelicopterLookaheadTime->current.value / 0.05000000074505806f;
        Vec3Scale(veh->phys.vel, scale, veh->phys.vel);
        Vec3Mad(veh->phys.origin, 0.050000001f, veh->phys.vel, targetPos);
        clipped = VEH_TestSlideMove(ent, clippedPos);
        veh->phys.vel[0] = oldVel[0];
        veh->phys.vel[1] = oldVel[1];
        veh->phys.vel[2] = oldVel[2];
        if (clipped)
        {
            Vec3Sub(targetPos, clippedPos, error);
            errorMagSqr = Vec3LengthSq(error);
            if (errorMagSqr >= 1.0f)
            {
                v2 = Vec3Dot(worldAccel, error);
                errorAccel = v2 / errorMagSqr;
                if (errorAccel > 0.0f)
                {
                    v3 = -errorAccel;
                    Vec3Mad(worldAccel, v3, error, worldAccel);
                }
                Vec3Sub(clippedPos, veh->phys.origin, wishVel);
                if (vehHelicopterLookaheadTime->current.value <= 0.0f)
                    MyAssertHandler(".\\game\\g_helicopter.cpp", 434, 0, "%s", "vehHelicopterLookaheadTime->current.value > 0");
                v4 = 1.0 / vehHelicopterLookaheadTime->current.value;
                Vec3Scale(wishVel, v4, wishVel);
                Vec3Sub(wishVel, veh->phys.vel, velChange);
                Vec3Mad(worldAccel, 20.0f, velChange, worldAccel);
            }
        }
    }
}

bool __cdecl VEH_TestSlideMove(gentity_s *ent, float *outPos)
{
    bool result; // eax
    scr_vehicle_s *veh; // [esp+10h] [ebp-1Ch]
    float startOrigin; // [esp+14h] [ebp-18h]
    float startOrigin_4; // [esp+18h] [ebp-14h]
    float startOrigin_8; // [esp+1Ch] [ebp-10h]
    float startVel; // [esp+20h] [ebp-Ch]
    float startVel_4; // [esp+24h] [ebp-8h]
    float startVel_8; // [esp+28h] [ebp-4h]

    veh = ent->scr_vehicle;
    startOrigin = veh->phys.origin[0];
    startOrigin_4 = veh->phys.origin[1];
    startOrigin_8 = veh->phys.origin[2];
    startVel = veh->phys.vel[0];
    startVel_4 = veh->phys.vel[1];
    startVel_8 = veh->phys.vel[2];
    result = VEH_SlideMove(ent, 0);
    *outPos = veh->phys.origin[0];
    outPos[1] = veh->phys.origin[1];
    outPos[2] = veh->phys.origin[2];
    veh->phys.origin[0] = startOrigin;
    veh->phys.origin[1] = startOrigin_4;
    veh->phys.origin[2] = startOrigin_8;
    veh->phys.vel[0] = startVel;
    veh->phys.vel[1] = startVel_4;
    veh->phys.vel[2] = startVel_8;
    return result;
}

