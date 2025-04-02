#include "cg_local.h"
#include "cg_public.h"

void __cdecl CG_Respawn(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (!cgArray[0].snap)
        MyAssertHandler(".\\cgame\\cg_playerstate.cpp", 108, 0, "%s", "cgameGlob->snap");
    memcpy(
        (unsigned __int8*)&cgArray[0].predictedPlayerState,
        (unsigned __int8*)&cgArray[0].snap->ps,
        sizeof(cgArray[0].predictedPlayerState));
    cgArray[0].weaponSelect = cgArray[0].predictedPlayerState.weapon;
    cgArray[0].weaponSelectTime = cgArray[0].time;
    cgArray[0].equippedOffHand = cgArray[0].predictedPlayerState.offHandIndex;
    cgArray[0].cursorHintIcon = 0;
    cgArray[0].cursorHintTime = 0;
    cgArray[0].proneBlockedEndTime = 0;
    cgArray[0].swayViewAngles[0] = 0.0;
    cgArray[0].swayViewAngles[1] = 0.0;
    cgArray[0].swayViewAngles[2] = 0.0;
    cgArray[0].swayAngles[0] = 0.0;
    cgArray[0].swayAngles[1] = 0.0;
    cgArray[0].swayAngles[2] = 0.0;
    cgArray[0].swayOffset[0] = 0.0;
    cgArray[0].swayOffset[1] = 0.0;
    cgArray[0].swayOffset[2] = 0.0;
    cgArray[0].kickAngles[0] = 0.0;
    cgArray[0].kickAngles[1] = 0.0;
    cgArray[0].kickAngles[2] = 0.0;
    cgArray[0].kickAVel[0] = 0.0;
    cgArray[0].kickAVel[1] = 0.0;
    cgArray[0].kickAVel[2] = 0.0;
    cgArray[0].xyspeed = 0.0;
    memset((unsigned __int8*)&cgArray[0].playerEntity, 0, sizeof(cgArray[0].playerEntity));
    cgArray[0].damageTime = 0;
    cgArray[0].v_dmg_pitch = 0.0;
    cgArray[0].v_dmg_roll = 0.0;
    cgArray[0].vGunOffset[0] = 0.0;
    cgArray[0].vGunOffset[1] = 0.0;
    cgArray[0].vGunOffset[2] = 0.0;
    cgArray[0].vGunSpeed[0] = 0.0;
    cgArray[0].vGunSpeed[1] = 0.0;
    cgArray[0].vGunSpeed[2] = 0.0;
    memset((unsigned __int8*)cgArray[0].viewDamage, 0, sizeof(cgArray[0].viewDamage));
    CG_ClearCameraShakes(localClientNum);
    cgArray[0].predictedError[0] = 0.0;
    cgArray[0].predictedError[1] = 0.0;
    cgArray[0].predictedError[2] = 0.0;
    cgArray[0].adsViewErrorDone = 0;
    CL_SetStance(localClientNum, CL_STANCE_STAND);
    CL_SetADS(localClientNum, 0);
    CG_SetEquippedOffHand(localClientNum, cgArray[0].predictedPlayerState.offHandIndex);
    CG_ResetLowHealthOverlay(cgArray);
    cgArray[0].heightToCeiling = 3.4028235e38;
    CG_HoldBreathInit(cgArray);
}

int __cdecl CG_TransitionPlayerState(int localClientNum, playerState_s *ps, const transPlayerState_t *ops)
{
    if (ps->damageEvent != ops->damageEvent && ps->damageCount)
        CG_DamageFeedback(localClientNum, ps->damageYaw, ps->damagePitch, ps->damageCount);
    return CG_CheckPlayerstateEvents(localClientNum, ps, ops);
}

void __cdecl CG_DamageFeedback(int localClientNum, int yawByte, int pitchByte, int damage)
{
    double v4; // st7
    float angle; // [esp+8h] [ebp-3Ch]
    float kick; // [esp+Ch] [ebp-38h]
    float dir[3]; // [esp+10h] [ebp-34h] BYREF
    int slot; // [esp+1Ch] [ebp-28h]
    cg_s *cgameGlob; // [esp+20h] [ebp-24h]
    float yaw; // [esp+24h] [ebp-20h]
    float forwardFrac; // [esp+28h] [ebp-1Ch]
    float sideFrac; // [esp+2Ch] [ebp-18h]
    float angles[3]; // [esp+30h] [ebp-14h] BYREF
    float pitch; // [esp+3Ch] [ebp-8h]
    int i; // [esp+40h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgameGlob = cgArray;
    kick = (double)damage * bg_viewKickScale->current.value;
    if (bg_viewKickMin->current.value <= (double)kick)
    {
        if (bg_viewKickMax->current.value < (double)kick)
            kick = bg_viewKickMax->current.value;
    }
    else
    {
        kick = bg_viewKickMin->current.value;
    }
    if (yawByte == 255 && pitchByte == 255)
    {
        cgameGlob->v_dmg_roll = 0.0;
        cgameGlob->v_dmg_pitch = -kick;
    }
    else
    {
        pitch = (double)pitchByte / 255.0 * 360.0;
        yaw = (double)yawByte / 255.0 * 360.0;
        angles[0] = pitch;
        angles[1] = yaw;
        angles[2] = 0.0;
        AngleVectors(angles, dir, 0, 0);
        sideFrac = Vec3Dot(dir, cgameGlob->refdef.viewaxis[1]);
        forwardFrac = Vec3Dot(dir, cgameGlob->refdef.viewaxis[0]);
        cgameGlob->v_dmg_roll = -kick * sideFrac;
        cgameGlob->v_dmg_pitch = kick * forwardFrac;
        slot = 0;
        for (i = 1; i < 8; ++i)
        {
            if (cgameGlob->viewDamage[i].time < cgameGlob->viewDamage[slot].time)
                slot = i;
        }
        cgameGlob->viewDamage[slot].time = cgameGlob->snap->serverTime;
        cgameGlob->viewDamage[slot].duration = cg_hudDamageIconTime->current.integer;
        angle = (random() - 0.5) * 20.0 + yaw;
        v4 = AngleNormalize360(angle);
        cgameGlob->viewDamage[slot].yaw = v4;
    }
    cgameGlob->v_dmg_time = cgameGlob->time + 500;
    cgameGlob->damageTime = cgameGlob->snap->serverTime;
    CG_MenuShowNotify(localClientNum, 0);
}

int __cdecl CG_CheckPlayerstateEvents(int localClientNum, playerState_s *ps, const transPlayerState_t *ops)
{
    int v4; // [esp+4h] [ebp-18h]
    int event; // [esp+8h] [ebp-14h]
    int i; // [esp+14h] [ebp-8h]
    int eventSequence; // [esp+18h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v4 = ops->eventSequence;
    if (v4 <= ps->eventSequence + 64)
        eventSequence = ops->eventSequence;
    else
        eventSequence = v4 - 256;
    for (i = ps->eventSequence - 4; i < ps->eventSequence; ++i)
    {
        if (i >= eventSequence || i > eventSequence - 4 && ps->events[i & 3] != ops->events[i & 3])
        {
            event = ps->events[i & 3];
            MEMORY[0x9D8640] = LOBYTE(ps->eventParms[i & 3]);
            CG_EntityEvent(localClientNum, (centity_s *)&MEMORY[0x9D84D8], event);
        }
    }
    return eventSequence;
}

