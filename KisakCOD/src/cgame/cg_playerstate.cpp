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
    if (!MEMORY[0x98F458])
        MyAssertHandler(".\\cgame\\cg_playerstate.cpp", 108, 0, "%s", "cgameGlob->snap");
    memcpy(MEMORY[0x9D5574], (unsigned __int8 *)(MEMORY[0x98F458] + 12), 0x2F64u);
    MEMORY[0x9DF71C][34] = MEMORY[0x9D565C];
    MEMORY[0x9DF71C][35] = MEMORY[0x9D5560];
    MEMORY[0x9DF71C][38] = MEMORY[0x9D5654];
    MEMORY[0x9DF71C][9] = 0;
    MEMORY[0x9DF71C][10] = 0;
    MEMORY[0x9DF71C][29] = 0;
    MEMORY[0x9D8748][4122] = 0.0;
    MEMORY[0x9D8748][4123] = 0.0;
    MEMORY[0x9D8748][4124] = 0.0;
    MEMORY[0x9D8748][4125] = 0.0;
    MEMORY[0x9D8748][4126] = 0.0;
    MEMORY[0x9D8748][4127] = 0.0;
    MEMORY[0x9D8748][4128] = 0.0;
    MEMORY[0x9D8748][4129] = 0.0;
    MEMORY[0x9D8748][4130] = 0.0;
    *(float *)&MEMORY[0x9DF71C][78] = 0.0;
    *(float *)&MEMORY[0x9DF71C][79] = 0.0;
    *(float *)&MEMORY[0x9DF71C][80] = 0.0;
    *(float *)&MEMORY[0x9DF71C][75] = 0.0;
    *(float *)&MEMORY[0x9DF71C][76] = 0.0;
    *(float *)&MEMORY[0x9DF71C][77] = 0.0;
    *(float *)&MEMORY[0x9DF71C][74] = 0.0;
    memset(MEMORY[0x9D86B4], 0, 0x30u);
    MEMORY[0x9DF71C][63] = 0;
    *(float *)&MEMORY[0x9DF71C][71] = 0.0;
    *(float *)&MEMORY[0x9DF71C][72] = 0.0;
    *(float *)&MEMORY[0x9DF71C][89] = 0.0;
    *(float *)&MEMORY[0x9DF71C][90] = 0.0;
    *(float *)&MEMORY[0x9DF71C][91] = 0.0;
    *(float *)&MEMORY[0x9DF71C][92] = 0.0;
    *(float *)&MEMORY[0x9DF71C][93] = 0.0;
    *(float *)&MEMORY[0x9DF71C][94] = 0.0;
    memset((unsigned __int8 *)&MEMORY[0x9DF71C][39], 0, 0x60u);
    CG_ClearCameraShakes(localClientNum);
    MEMORY[0x9D86E8] = 0.0;
    MEMORY[0x9D86EC] = 0.0;
    MEMORY[0x9D86F0] = 0.0;
    MEMORY[0x9E06EC] = 0;
    CL_SetStance(localClientNum, CL_STANCE_STAND);
    CL_SetADS(localClientNum, 0);
    CG_SetEquippedOffHand(localClientNum, MEMORY[0x9D5654]);
    CG_ResetLowHealthOverlay(cgArray);
    MEMORY[0x9D86FC] = 3.4028235e38;
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

