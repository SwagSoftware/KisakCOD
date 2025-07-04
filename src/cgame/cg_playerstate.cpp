#include "cg_local.h"
#include "cg_public.h"

#include <cgame_mp/cg_local_mp.h>

#include <client_mp/client_mp.h>

void __cdecl CG_Respawn(int32_t localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    iassert(cgameGlob->snap);

    memcpy(
        (uint8_t*)&cgameGlob->predictedPlayerState,
        (uint8_t*)&cgameGlob->snap->ps,
        sizeof(cgameGlob->predictedPlayerState));
    cgameGlob->weaponSelect = cgameGlob->predictedPlayerState.weapon;
    cgameGlob->weaponSelectTime = cgameGlob->time;
    cgameGlob->equippedOffHand = cgameGlob->predictedPlayerState.offHandIndex;
    cgameGlob->cursorHintIcon = 0;
    cgameGlob->cursorHintTime = 0;
    cgameGlob->proneBlockedEndTime = 0;
    cgameGlob->swayViewAngles[0] = 0.0;
    cgameGlob->swayViewAngles[1] = 0.0;
    cgameGlob->swayViewAngles[2] = 0.0;
    cgameGlob->swayAngles[0] = 0.0;
    cgameGlob->swayAngles[1] = 0.0;
    cgameGlob->swayAngles[2] = 0.0;
    cgameGlob->swayOffset[0] = 0.0;
    cgameGlob->swayOffset[1] = 0.0;
    cgameGlob->swayOffset[2] = 0.0;
    cgameGlob->kickAngles[0] = 0.0;
    cgameGlob->kickAngles[1] = 0.0;
    cgameGlob->kickAngles[2] = 0.0;
    cgameGlob->kickAVel[0] = 0.0;
    cgameGlob->kickAVel[1] = 0.0;
    cgameGlob->kickAVel[2] = 0.0;
    cgameGlob->xyspeed = 0.0;
    memset((uint8_t*)&cgameGlob->playerEntity, 0, sizeof(cgameGlob->playerEntity));
    cgameGlob->damageTime = 0;
    cgameGlob->v_dmg_pitch = 0.0;
    cgameGlob->v_dmg_roll = 0.0;
    cgameGlob->vGunOffset[0] = 0.0;
    cgameGlob->vGunOffset[1] = 0.0;
    cgameGlob->vGunOffset[2] = 0.0;
    cgameGlob->vGunSpeed[0] = 0.0;
    cgameGlob->vGunSpeed[1] = 0.0;
    cgameGlob->vGunSpeed[2] = 0.0;
    memset((uint8_t*)cgameGlob->viewDamage, 0, sizeof(cgameGlob->viewDamage));
    CG_ClearCameraShakes(localClientNum);
    cgameGlob->predictedError[0] = 0.0;
    cgameGlob->predictedError[1] = 0.0;
    cgameGlob->predictedError[2] = 0.0;
    cgameGlob->adsViewErrorDone = 0;
    CL_SetStance(localClientNum, CL_STANCE_STAND);
    CL_SetADS(localClientNum, 0);
    CG_SetEquippedOffHand(localClientNum, cgameGlob->predictedPlayerState.offHandIndex);
    CG_ResetLowHealthOverlay(cgameGlob);
    cgameGlob->heightToCeiling = FLT_MAX;
    CG_HoldBreathInit(cgameGlob);
}

int32_t __cdecl CG_TransitionPlayerState(int32_t localClientNum, playerState_s *ps, const transPlayerState_t *ops)
{
    if (ps->damageEvent != ops->damageEvent && ps->damageCount)
        CG_DamageFeedback(localClientNum, ps->damageYaw, ps->damagePitch, ps->damageCount);
    return CG_CheckPlayerstateEvents(localClientNum, ps, ops);
}

void __cdecl CG_DamageFeedback(int32_t localClientNum, int32_t yawByte, int32_t pitchByte, int32_t damage)
{
    double v4; // st7
    float angle; // [esp+8h] [ebp-3Ch]
    float kick; // [esp+Ch] [ebp-38h]
    float dir[3]; // [esp+10h] [ebp-34h] BYREF
    int32_t slot; // [esp+1Ch] [ebp-28h]
    cg_s *cgameGlob; // [esp+20h] [ebp-24h]
    float yaw; // [esp+24h] [ebp-20h]
    float forwardFrac; // [esp+28h] [ebp-1Ch]
    float sideFrac; // [esp+2Ch] [ebp-18h]
    float angles[3]; // [esp+30h] [ebp-14h] BYREF
    float pitch; // [esp+3Ch] [ebp-8h]
    int32_t i; // [esp+40h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
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

int32_t __cdecl CG_CheckPlayerstateEvents(int32_t localClientNum, playerState_s* ps, const transPlayerState_t* ops)
{
    int32_t v4; // [esp+4h] [ebp-18h]
    int32_t event; // [esp+8h] [ebp-14h]
    int32_t i; // [esp+14h] [ebp-8h]
    int32_t eventSequence; // [esp+18h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

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
            cgameGlob->predictedPlayerEntity.nextState.eventParm = LOBYTE(ps->eventParms[i & 3]);
            CG_EntityEvent(localClientNum, &cgameGlob->predictedPlayerEntity, event);
        }
    }
    return eventSequence;
}
