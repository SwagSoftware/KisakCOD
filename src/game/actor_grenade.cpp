#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_grenade.h"
#include "g_main.h"
#include "g_local.h"

#include <universal/com_math.h>
#include <xanim/xanim.h>
#include <script/scr_const.h>
#include "actor_orientation.h"
#include "actor_team_move.h"
#include "actor_cover.h"
#include "actor_state.h"
#include "actor_threat.h"
#include "actor_events.h"
#include <script/scr_vm.h>

float g_vGrenadeHint[512][3]{ 0.0f };
float g_vRefPos[3]{ 0.0f };

void __cdecl TRACK_actor_grenade()
{
    track_static_alloc_internal(g_vGrenadeHint, 6144, "g_vGrenadeHint", 5);
    track_static_alloc_internal(g_vRefPos, 12, "g_vRefPos", 5);
}

void __cdecl SP_info_grenade_hint(gentity_s *ent)
{
    unsigned int grenadeHintCount; // r11
    float *v3; // r10
    double v4; // fp0

    grenadeHintCount = level.grenadeHintCount;
    if (level.grenadeHintCount >= 0x200)
    {
        Com_Error(ERR_DROP, "MAX_GRENADE_HINTS (%i) exceeded", 512);
        grenadeHintCount = level.grenadeHintCount;
    }
    v3 = g_vGrenadeHint[grenadeHintCount];
    *v3 = ent->r.currentOrigin[0];
    v3[1] = ent->r.currentOrigin[1];
    v4 = ent->r.currentOrigin[2];
    level.grenadeHintCount = grenadeHintCount + 1;
    v3[2] = v4;
    G_FreeEntity(ent);
}

bool __cdecl Actor_Grenade_IsValidTrajectory(
    actor_s *self,
    const float *vFrom,
    float *vVelocity,
    const float *vGoal)
{
    double v8; // fp12
    double v9; // fp31
    double v10; // fp24
    double v11; // fp29
    double v12; // fp30
    double v13; // fp0
    double v14; // fp30
    int v15; // r4
    double v16; // fp30
    double v17; // fp12
    double v18; // fp27
    double v19; // fp12
    double v20; // fp11
    double v21; // fp28
    double v22; // fp13
    gentity_s *ent; // r11
    gentity_s *v24; // r8
    double v25; // fp0
    double v27; // fp31
    double v28; // fp0
    gentity_s *v29; // r11
    int EntityHitId; // r11
    double v31; // fp0
    double v32; // fp13
    double v33; // fp12
    double v34; // fp0
    sentient_s *sentient; // r31
    int v36; // [sp+50h] [-130h] BYREF
    float v37; // [sp+54h] [-12Ch]
    float v38; // [sp+58h] [-128h] BYREF
    float v39; // [sp+5Ch] [-124h]
    float v40; // [sp+60h] [-120h]
    float v41; // [sp+68h] [-118h] BYREF
    float v42; // [sp+6Ch] [-114h]
    float v43; // [sp+70h] [-110h]
    float v44[2]; // [sp+78h] [-108h] BYREF
    float v45; // [sp+80h] [-100h]
    float v46[4]; // [sp+88h] [-F8h] BYREF
    float v47[6]; // [sp+98h] [-E8h] BYREF
    trace_t v48[2]; // [sp+B0h] [-D0h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 81, 0, "%s", "self");
    if (!vFrom)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 82, 0, "%s", "vFrom");
    if (!vVelocity)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 83, 0, "%s", "vVelocity");
    if (!vGoal)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 84, 0, "%s", "vGoal");
    if (g_gravity->current.value <= 0.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            85,
            0,
            "%s",
            "g_gravity->current.value > 0");
    v37 = *vFrom;
    if ((LODWORD(v37) & 0x7F800000) == 0x7F800000
        || (v37 = vFrom[1], (LODWORD(v37) & 0x7F800000) == 0x7F800000)
        || (v37 = vFrom[2], (LODWORD(v37) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            106,
            0,
            "%s",
            "!IS_NAN((vFrom)[0]) && !IS_NAN((vFrom)[1]) && !IS_NAN((vFrom)[2])");
    }
    v37 = *vVelocity;
    if ((LODWORD(v37) & 0x7F800000) == 0x7F800000
        || (v37 = vVelocity[1], (LODWORD(v37) & 0x7F800000) == 0x7F800000)
        || (v37 = vVelocity[2], (LODWORD(v37) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            107,
            0,
            "%s",
            "!IS_NAN((vVelocity)[0]) && !IS_NAN((vVelocity)[1]) && !IS_NAN((vVelocity)[2])");
    }
    v37 = *vGoal;
    if ((LODWORD(v37) & 0x7F800000) == 0x7F800000
        || (v37 = vGoal[1], (LODWORD(v37) & 0x7F800000) == 0x7F800000)
        || (v37 = vGoal[2], (LODWORD(v37) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            108,
            0,
            "%s",
            "!IS_NAN((vGoal)[0]) && !IS_NAN((vGoal)[1]) && !IS_NAN((vGoal)[2])");
    }
    v8 = vVelocity[2];
    v47[0] = -1.0;
    v47[1] = -1.0;
    v47[2] = -1.0;
    v44[0] = 1.0;
    v44[1] = 1.0;
    v45 = 1.0;
    if (v8 <= 0.0)
    {
        v9 = 0.0;
        v11 = 0.0;
        v16 = (float)((float)((float)((float)(vFrom[2] - vGoal[2]) * g_gravity->current.value) * (float)2.0)
            + (float)(vVelocity[2] * vVelocity[2]));
        v37 = (float)((float)((float)(vFrom[2] - vGoal[2]) * g_gravity->current.value) * (float)2.0)
            + (float)(vVelocity[2] * vVelocity[2]);
        if ((LODWORD(v37) & 0x7F800000) == 0x7F800000)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 142, 0, "%s", "!IS_NAN(tDiscriminant)");
        if (v16 <= 0.0)
            return 0;
        v17 = sqrtf(v16);
        v10 = 0.5;
        v14 = (float)((float)((float)((float)v17 + vVelocity[2]) / g_gravity->current.value) * (float)0.5);
        v37 = (float)((float)((float)v17 + vVelocity[2]) / g_gravity->current.value) * (float)0.5;
        if ((LODWORD(v37) & 0x7F800000) != 0x7F800000)
            goto LABEL_39;
        v15 = 148;
    }
    else
    {
        v9 = (float)((float)((float)1.0 / g_gravity->current.value) * (float)v8);
        v10 = 0.5;
        v11 = (float)((float)((float)((float)1.0 / g_gravity->current.value) * (float)v8) * (float)0.5);
        v12 = (float)((float)((float)((float)(vFrom[2] - vGoal[2]) * (float)((float)1.0 / g_gravity->current.value))
            * (float)2.0)
            + (float)((float)v9 * (float)v9));
        if ((COERCE_UNSIGNED_INT((float)((float)1.0 / g_gravity->current.value) * (float)v8) & 0x7F800000) == 0x7F800000)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 126, 0, "%s", "!IS_NAN(tApex)");
        v37 = v11;
        if ((LODWORD(v37) & 0x7F800000) == 0x7F800000)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 127, 0, "%s", "!IS_NAN(tMidNear)");
        v37 = v12;
        if ((LODWORD(v37) & 0x7F800000) == 0x7F800000)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 128, 0, "%s", "!IS_NAN(tDiscriminant)");
        if (v12 <= 0.0)
            return 0;
        v13 = sqrtf(v12);
        v14 = (float)((float)v13 * (float)0.5);
        v37 = (float)v13 * (float)0.5;
        if ((LODWORD(v37) & 0x7F800000) != 0x7F800000)
            goto LABEL_39;
        v15 = 134;
    }
    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", v15, 0, "%s", "!IS_NAN(tHalfFarSegment)");
LABEL_39:
    v18 = (float)((float)v14 + (float)v9);
    v19 = *vFrom;
    if (I_fabs((float)((float)((float)((float)((float)((float)v14 + (float)v9) + (float)v14) * *vVelocity) + *vFrom)
        - *vGoal)) <= 0.1)
    {
        v20 = vFrom[1];
        if (I_fabs((float)((float)((float)((float)((float)((float)v14 + (float)v9) + (float)v14) * vVelocity[1]) + vFrom[1])
            - vGoal[1])) <= 0.1)
        {
            v36 = 0;
            if (v9 <= 0.0)
            {
                v40 = vFrom[2];
                v38 = v19;
                v39 = v20;
            }
            else
            {
                v21 = (float)((float)((float)(g_gravity->current.value * (float)v11) * (float)v11) * (float)0.125);
                v37 = (float)((float)(g_gravity->current.value * (float)v11) * (float)v11) * (float)0.125;
                if ((LODWORD(v37) & 0x7F800000) == 0x7F800000)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
                        164,
                        0,
                        "%s",
                        "!IS_NAN(fMaxErrorNear)");
                v22 = (float)((float)(vVelocity[1] * (float)v11) + vFrom[1]);
                v46[0] = (float)(*vVelocity * (float)v11) + *vFrom;
                v45 = (float)v21 + (float)1.0;
                v46[1] = v22;
                ent = self->ent;
                v46[2] = (float)((float)((float)(g_gravity->current.value * (float)v11) * (float)v11) * (float)1.5) + vFrom[2];
                SV_SightTrace(&v36, vFrom, v47, v44, v46, ent->s.number, ENTITYNUM_NONE, 42004625);
                if (g_drawGrenadeHints->current.integer > 0)
                    G_DebugLineWithDuration(vFrom, v46, colorCyan, 1, 200);
                if (v36)
                    return 0;
                v24 = self->ent;
                v25 = (float)((float)(vVelocity[1] * (float)v9) + vFrom[1]);
                v38 = (float)(*vVelocity * (float)v9) + *vFrom;
                v39 = v25;
                v40 = (float)((float)((float)(g_gravity->current.value * (float)v9) * (float)v9) * (float)v10) + vFrom[2];
                SV_SightTrace(&v36, v46, v47, v44, &v38, v24->s.number, ENTITYNUM_NONE, 42004625);
                if (g_drawGrenadeHints->current.integer > 0)
                    G_DebugLineWithDuration(v46, &v38, colorCyan, 1, 200);
                if (v36)
                    return 0;
            }
            v27 = (float)((float)((float)(g_gravity->current.value * (float)v14) * (float)v14) * (float)0.125);
            v37 = (float)((float)(g_gravity->current.value * (float)v14) * (float)v14) * (float)0.125;
            if ((LODWORD(v37) & 0x7F800000) == 0x7F800000)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 207, 0, "%s", "!IS_NAN(fMaxErrorFar)");
            v28 = (float)((float)(*vVelocity * (float)v18) + *vFrom);
            v45 = (float)v27 + (float)1.0;
            v29 = self->ent;
            v42 = (float)(vVelocity[1] * (float)v18) + vFrom[1];
            v41 = v28;
            v43 = (float)((float)-(float)((float)((float)(g_gravity->current.value * (float)v18) * (float)v10) - vVelocity[2])
                * (float)v18)
                + vFrom[2];
            SV_SightTrace(&v36, &v38, v47, v44, &v41, v29->s.number, ENTITYNUM_NONE, 42004625);
            if (g_drawGrenadeHints->current.integer > 0)
                G_DebugLineWithDuration(&v38, &v41, colorCyan, 1, 200);
            if (!v36)
            {
                G_TraceCapsule(v48, &v41, v47, v44, vGoal, self->ent->s.number, 42004625);
                if (g_drawGrenadeHints->current.integer > 0)
                    G_DebugLineWithDuration(&v41, vGoal, colorCyan, 1, 200);
                if (v48[0].fraction == 1.0)
                    return 1;
                EntityHitId = Trace_GetEntityHitId(v48);
                if (EntityHitId == ENTITYNUM_WORLD)
                {
                    v31 = (float)((float)((float)((float)(vGoal[1] - v42) * v48[0].fraction) + v42) - vGoal[1]);
                    v32 = (float)((float)((float)((float)(vGoal[2] - v43) * v48[0].fraction) + v43) - vGoal[2]);
                    v33 = (float)((float)((float)((float)(*vGoal - v41) * v48[0].fraction) + v41) - *vGoal);
                    v34 = (float)((float)((float)v33 * (float)v33)
                        + (float)((float)((float)v32 * (float)v32) + (float)((float)v31 * (float)v31)));
                    if (v34 >= 16.0)
                        return v34 < 900.0 && v48[0].normal[2] > v10;
                }
                else
                {
                    sentient = level.gentities[EntityHitId].sentient;
                    if (!sentient || sentient->eTeam != Sentient_EnemyTeam(self->sentient->eTeam))
                        return 0;
                }
                return 1;
            }
        }
    }
    return 0;
}

void __cdecl Actor_Grenade_GetTossFromPosition(
    actor_s *self,
    const float *vStandPos,
    const float *vOffset,
    float *vFrom)
{
    float v8[4]; // [sp+50h] [-70h] BYREF
    float v9[8][3]; // [sp+60h] [-60h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 277, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 278, 0, "%s", "self->ent");
    AnglesToAxis(self->ent->r.currentAngles, v9);
    MatrixTransformVector(vOffset, (const mat3x3&)v9, v8);
    *vFrom = *vStandPos + v8[0];
    vFrom[1] = vStandPos[1] + v8[1];
    vFrom[2] = vStandPos[2] + v8[2];
}

void __cdecl Actor_Grenade_GetTossPositions(
    const float *vFrom,
    float *vTargetPos,
    float *vLand,
    unsigned int grenadeWPID)
{
    WeaponDef *weapDef; // r28
    double v8; // fp12
    int v9; // r11
    int v10; // r10
    int v11; // r11
    double v12; // fp11
    double v13; // fp0
    float v14; // [sp+50h] [-80h] BYREF
    float v15; // [sp+54h] [-7Ch]
    float v16; // [sp+58h] [-78h]
    float v17[4]; // [sp+60h] [-70h] BYREF
    trace_t v18; // [sp+70h] [-60h] BYREF

    weapDef = BG_GetWeaponDef(grenadeWPID);
    if (!weapDef)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 306, 0, "%s", "weapDef");
    v8 = vTargetPos[2];
    v14 = *vTargetPos;
    v17[0] = v14;
    v15 = vTargetPos[1];
    v16 = (float)v8 + (float)1.0;
    v17[1] = v15;
    v17[2] = (float)v8 - (float)1.0;
    G_MissileTrace(&v18, &v14, v17, ENTITYNUM_NONE, 2065);

    if (v18.fraction == 1.0)
        v9 = 0;
    else
        v9 = (v18.surfaceFlags >> 20) & 0x1F;

    v10 = 4 * ((unsigned __int8)v9 + 368);
    v11 = 4 * ((unsigned __int8)v9 + 397);
    v12 = (float)((float)(vTargetPos[1] - vFrom[1])
        * (float)((float)1.0
            - (float)((float)((float)(*(float *)((char *)&weapDef->szInternalName + v11)
                - *(float *)((char *)&weapDef->szInternalName + v10))
                * (float)0.93970001)
                + *(float *)((char *)&weapDef->szInternalName + v10))));
    v13 = (float)((float)(vTargetPos[2] - vFrom[2])
        * (float)((float)1.0
            - (float)((float)((float)(*(float *)((char *)&weapDef->szInternalName + v11)
                - *(float *)((char *)&weapDef->szInternalName + v10))
                * (float)0.93970001)
                + *(float *)((char *)&weapDef->szInternalName + v10))));
    *vLand = *vFrom
        + (float)((float)(*vTargetPos - *vFrom)
            * (float)((float)1.0
                - (float)((float)((float)(*(float *)((char *)&weapDef->szInternalName + v11)
                    - *(float *)((char *)&weapDef->szInternalName + v10))
                    * (float)0.93970001)
                    + *(float *)((char *)&weapDef->szInternalName + v10))));
    vLand[1] = vFrom[1] + (float)v12;
    vLand[2] = vFrom[2] + (float)v13;
}

int __cdecl Actor_Grenade_GetTossPositionsFromHints(
    const float *vFrom,
    const float *vTargetPos,
    const unsigned int method,
    float *vLand)
{
    long double v7; // fp2
    long double v8; // fp2
    double v9; // fp29
    int v10; // r26
    unsigned int v11; // r30
    double v12; // fp30
    float *v13; // r31
    double v14; // fp0
    double v15; // fp31
    float *v16; // r11
    float v18; // [sp+50h] [-90h] BYREF
    float v19; // [sp+54h] [-8Ch]
    float v20; // [sp+60h] [-80h] BYREF
    float v21; // [sp+64h] [-7Ch]

    //Profile_Begin(351);
    *(double *)&v7 = (float)((float)((float)(ai_debugGrenadeHintArc->current.value * (float)0.5) + (float)180.0)
        * (float)0.017453292);
    v8 = cos(v7);
    v9 = (float)*(double *)&v8;
    if (!level.grenadeHintCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            358,
            0,
            "%s",
            "level.grenadeHintCount > 0");
    v10 = -1;
    v11 = 0;
    v12 = FLT_MAX;
    if (!level.grenadeHintCount)
        goto LABEL_13;
    v13 = &g_vGrenadeHint[0][1];
    do
    {
        if ((float)(v13[1] + 10.0) >= (double)vTargetPos[2])
        {
            v14 = (float)(*v13 - vTargetPos[1]);
            v18 = *(v13 - 1) - *vTargetPos;
            v19 = v14;
            v15 = Vec2Normalize(&v18);
            if (v15 <= 192.0)
            {
                v20 = *(v13 - 1) - *vFrom;
                v21 = *v13 - vFrom[1];
                Vec2Normalize(&v20);
                if ((float)((float)(v20 * v18) + (float)(v21 * v19)) <= v9 && v15 < v12)
                {
                    v10 = v11;
                    v12 = v15;
                }
            }
        }
        ++v11;
        v13 += 3;
    } while (v11 < level.grenadeHintCount);
    if (v10 == -1)
    {
    LABEL_13:
        //Profile_EndInternal(0);
        return 0;
    }
    else
    {
        v16 = g_vGrenadeHint[v10];
        *vLand = *v16;
        vLand[1] = v16[1];
        vLand[2] = v16[2];
        //Profile_EndInternal(0);
        return 1;
    }
}

bool __cdecl Actor_Grenade_CheckMaximumEnergyToss(
    actor_s *self,
    float *vFrom,
    float *vLand,
    int bLob,
    float *vVelOut)
{
    double v10; // fp29
    double v11; // fp28
    double v12; // fp30
    double v13; // fp11
    double v15; // fp0
    double v16; // fp13
    double v17; // fp13
    double v18; // fp13
    double v19; // fp31
    const dvar_s *v20; // r11
    double v21; // fp0
    float v22; // [sp+50h] [-90h]

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 413, 0, "%s", "self");
    if (!vFrom)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 414, 0, "%s", "vFrom");
    if (!vLand)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 415, 0, "%s", "vLand");
    if (g_gravity->current.value <= 0.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            423,
            0,
            "%s",
            "g_gravity->current.value > 0");
    if ((COERCE_UNSIGNED_INT(*vFrom) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vFrom[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vFrom[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            425,
            0,
            "%s",
            "!IS_NAN((vFrom)[0]) && !IS_NAN((vFrom)[1]) && !IS_NAN((vFrom)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*vLand) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vLand[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vLand[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            426,
            0,
            "%s",
            "!IS_NAN((vLand)[0]) && !IS_NAN((vLand)[1]) && !IS_NAN((vLand)[2])");
    }
    v10 = (float)(*vLand - *vFrom);
    v11 = (float)(vLand[1] - vFrom[1]);
    v12 = (float)(vLand[2] - vFrom[2]);
    v13 = 1.0;
    if ((float)((float)((float)v12 * (float)v12)
        + (float)((float)((float)v11 * (float)v11) + (float)((float)(*vLand - *vFrom) * (float)(*vLand - *vFrom)))) < 1.0)
        return 0;
    v15 = (float)-(float)((float)(g_gravity->current.value * (float)(vLand[2] - vFrom[2])) - (float)810000.0);
    v16 = (float)((float)((float)v15 * (float)v15)
        - (float)((float)(g_gravity->current.value * g_gravity->current.value)
            * (float)((float)((float)v12 * (float)v12)
                + (float)((float)((float)v11 * (float)v11)
                    + (float)((float)(*vLand - *vFrom) * (float)(*vLand - *vFrom))))));
    if (v16 < 0.0)
        return 0;
    v17 = sqrtf(v16);
    if (bLob)
        v18 = (float)((float)v17
            - (float)((float)(g_gravity->current.value * (float)(vLand[2] - vFrom[2])) - (float)810000.0));
    else
        v18 = (float)((float)-(float)((float)(g_gravity->current.value * (float)(vLand[2] - vFrom[2])) - (float)810000.0)
            - (float)v17);
    v19 = (float)((float)sqrtf((float)((float)v18 * 2.0)) / g_gravity->current.value);
    if (v19 == 0.0)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 444, 0, "%s", "fTotalTime != 0.0f");
        v13 = 1.0;
    }
    v20 = g_gravity;
    *vVelOut = (float)((float)v13 / (float)v19) * (float)v10;
    vVelOut[1] = (float)((float)v13 / (float)v19) * (float)v11;
    v21 = (float)((float)((float)(v20->current.value * (float)v19) * (float)0.5)
        + (float)((float)((float)v13 / (float)v19) * (float)v12));
    vVelOut[2] = (float)((float)(v20->current.value * (float)v19) * (float)0.5)
        + (float)((float)((float)v13 / (float)v19) * (float)v12);
    if ((COERCE_UNSIGNED_INT((float)((float)v13 / (float)v19) * (float)v10) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((float)((float)v13 / (float)v19) * (float)v11) & 0x7F800000) == 0x7F800000
        || (v22 = v21, (LODWORD(v22) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            450,
            0,
            "%s",
            "!IS_NAN((vVelOut)[0]) && !IS_NAN((vVelOut)[1]) && !IS_NAN((vVelOut)[2])");
    }
    return Actor_Grenade_IsValidTrajectory(self, vFrom, vVelOut, vLand);
}

bool __cdecl Actor_Grenade_CheckInfiniteEnergyToss(actor_s *self, float *vFrom, float *vLand, float *vVelOut)
{
    double v8; // fp30
    double v9; // fp29
    double v10; // fp28
    double v11; // fp31
    double v12; // fp31
    const dvar_s *v13; // r11
    double v14; // fp0
    float v16; // [sp+50h] [-80h]
    float v17; // [sp+50h] [-80h]

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 459, 0, "%s", "self");
    if (!vFrom)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 460, 0, "%s", "vFrom");
    if (!vLand)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 461, 0, "%s", "vLand");
    if (g_gravity->current.value <= 0.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            468,
            0,
            "%s",
            "g_gravity->current.value > 0");
    if ((COERCE_UNSIGNED_INT(*vFrom) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vFrom[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vFrom[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            470,
            0,
            "%s",
            "!IS_NAN((vFrom)[0]) && !IS_NAN((vFrom)[1]) && !IS_NAN((vFrom)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*vLand) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vLand[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vLand[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            471,
            0,
            "%s",
            "!IS_NAN((vLand)[0]) && !IS_NAN((vLand)[1]) && !IS_NAN((vLand)[2])");
    }
    v8 = (float)(vLand[1] - vFrom[1]);
    v9 = (float)(vLand[2] - vFrom[2]);
    v10 = (float)(*vLand - *vFrom);
    v11 = (float)((float)((float)(*vLand - *vFrom) * (float)(*vLand - *vFrom))
        + (float)((float)((float)v9 * (float)v9) + (float)((float)v8 * (float)v8)));
    if (v11 <= 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 475, 0, "%s", "fTotalDistSqrd > 0");
    v12 = (float)((float)sqrtf((float)((float)((float)sqrtf(v11) * g_gravity->current.value) * (float)2.0))
        / g_gravity->current.value);
    if (v12 == 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 480, 0, "%s", "fTotalTime != 0.0f");
    v13 = g_gravity;
    v16 = (float)((float)1.0 / (float)v12) * (float)v10;
    *vVelOut = v16;
    vVelOut[1] = (float)((float)1.0 / (float)v12) * (float)v8;
    v14 = (float)((float)((float)(v13->current.value * (float)v12) * (float)0.5)
        + (float)((float)((float)1.0 / (float)v12) * (float)v9));
    vVelOut[2] = (float)((float)(v13->current.value * (float)v12) * (float)0.5)
        + (float)((float)((float)1.0 / (float)v12) * (float)v9);
    if ((LODWORD(v16) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((float)((float)1.0 / (float)v12) * (float)v8) & 0x7F800000) == 0x7F800000
        || (v17 = v14, (LODWORD(v17) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            486,
            0,
            "%s",
            "!IS_NAN((vVelOut)[0]) && !IS_NAN((vVelOut)[1]) && !IS_NAN((vVelOut)[2])");
    }
    return Actor_Grenade_IsValidTrajectory(self, vFrom, vVelOut, vLand);
}

bool __cdecl Actor_Grenade_CheckMinimumEnergyToss(actor_s *self, float *vFrom, float *vLand, float *vVelOut)
{
    double v8; // fp29
    double v9; // fp28
    double v10; // fp30
    double v11; // fp31
    double v13; // fp31
    const dvar_s *v14; // r11
    double v15; // fp0
    float v16; // [sp+50h] [-80h]
    float v17; // [sp+50h] [-80h]

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 500, 0, "%s", "self");
    if (!vFrom)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 501, 0, "%s", "vFrom");
    if (!vLand)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 502, 0, "%s", "vLand");
    if (g_gravity->current.value <= 0.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            509,
            0,
            "%s",
            "g_gravity->current.value > 0");
    if ((COERCE_UNSIGNED_INT(*vFrom) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vFrom[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vFrom[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            511,
            0,
            "%s",
            "!IS_NAN((vFrom)[0]) && !IS_NAN((vFrom)[1]) && !IS_NAN((vFrom)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*vLand) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vLand[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vLand[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            512,
            0,
            "%s",
            "!IS_NAN((vLand)[0]) && !IS_NAN((vLand)[1]) && !IS_NAN((vLand)[2])");
    }
    v8 = (float)(*vLand - *vFrom);
    v9 = (float)(vLand[1] - vFrom[1]);
    v10 = (float)(vLand[2] - vFrom[2]);
    v11 = sqrtf((float)((float)((float)v10 * (float)v10)
        + (float)((float)((float)v9 * (float)v9)
            + (float)((float)(*vLand - *vFrom) * (float)(*vLand - *vFrom)))));
    if (v11 <= 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 516, 0, "%s", "fTotalDist > 0");
    if ((float)((float)((float)v10 + (float)v11) * g_gravity->current.value) > 810000.0)
        return 0;
    v13 = sqrtf((float)((float)((float)v11 / g_gravity->current.value) * (float)2.0));
    if (v13 == 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 524, 0, "%s", "fTotalTime != 0.0f");
    v14 = g_gravity;
    v16 = (float)((float)1.0 / (float)v13) * (float)v8;
    *vVelOut = v16;
    vVelOut[1] = (float)((float)1.0 / (float)v13) * (float)v9;
    v15 = (float)((float)((float)(v14->current.value * (float)v13) * (float)0.5)
        + (float)((float)((float)1.0 / (float)v13) * (float)v10));
    vVelOut[2] = (float)((float)(v14->current.value * (float)v13) * (float)0.5)
        + (float)((float)((float)1.0 / (float)v13) * (float)v10);
    if ((LODWORD(v16) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT((float)((float)1.0 / (float)v13) * (float)v9) & 0x7F800000) == 0x7F800000
        || (v17 = v15, (LODWORD(v17) & 0x7F800000) == 0x7F800000))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            530,
            0,
            "%s",
            "!IS_NAN((vVelOut)[0]) && !IS_NAN((vVelOut)[1]) && !IS_NAN((vVelOut)[2])");
    }
    return Actor_Grenade_IsValidTrajectory(self, vFrom, vVelOut, vLand);
}

int __cdecl Actor_Grenade_CheckGrenadeHintToss(actor_s *self, float *vFrom, float *vLand, float *vVelOut)
{
    unsigned int grenadeHintCount; // r8
    unsigned int v9; // r29
    double v10; // fp23
    double v11; // fp22
    double v12; // fp25
    double v13; // fp30
    float *v14; // r31
    double v15; // fp0
    double v16; // fp12
    double v17; // fp0
    double v18; // fp1
    double v19; // fp29
    double v20; // fp11
    double v21; // fp10
    double v22; // fp12
    double v23; // fp13
    double v24; // fp26
    double v25; // fp28
    double v26; // fp31
    double v27; // fp0
    double v28; // fp12
    float v30; // [sp+58h] [-E8h] BYREF
    float v31; // [sp+5Ch] [-E4h]
    float v32; // [sp+60h] [-E0h]
    float v33[28]; // [sp+68h] [-D8h] BYREF

    //Profile_Begin(352);
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 547, 0, "%s", "self");
    if (!vFrom)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 548, 0, "%s", "vFrom");
    if (!vLand)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 549, 0, "%s", "vLand");
    if (g_gravity->current.value <= 0.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            566,
            0,
            "%s",
            "g_gravity->current.value > 0");
    if ((COERCE_UNSIGNED_INT(*vFrom) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vFrom[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vFrom[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            568,
            0,
            "%s",
            "!IS_NAN((vFrom)[0]) && !IS_NAN((vFrom)[1]) && !IS_NAN((vFrom)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*vLand) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vLand[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(vLand[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            569,
            0,
            "%s",
            "!IS_NAN((vLand)[0]) && !IS_NAN((vLand)[1]) && !IS_NAN((vLand)[2])");
    }
    grenadeHintCount = level.grenadeHintCount;
    if (!level.grenadeHintCount)
    {
    LABEL_30:
        //Profile_EndInternal(0);
        return 0;
    }
    v9 = 0;
    v10 = (float)(*vLand - *vFrom);
    v11 = (float)(vLand[1] - vFrom[1]);
    v12 = (float)(vLand[2] - vFrom[2]);
    v13 = sqrtf((float)((float)((float)(*vLand - *vFrom) * (float)(*vLand - *vFrom)) + (float)((float)v11 * (float)v11)));
    v14 = &g_vGrenadeHint[0][2];
    while (1)
    {
        v15 = (float)(*(v14 - 2) - *vFrom);
        v31 = *(v14 - 1) - vFrom[1];
        v16 = *v14;
        v30 = v15;
        v17 = (float)((float)((float)((float)v16 - vFrom[2]) * (float)v12)
            + (float)((float)((float)v15 * (float)v10) + (float)(v31 * (float)v11)));
        v32 = (float)v16 - vFrom[2];
        if (v17 >= 0.0)
            break;
    LABEL_29:
        ++v9;
        v14 += 3;
        if (v9 >= grenadeHintCount)
            goto LABEL_30;
    }
    v18 = Vec2Normalize(&v30);
    v19 = v18;
    if (v18 >= v13)
        goto LABEL_28;
    if (v18 == 0.0)
        goto LABEL_28;
    v33[2] = vLand[2];
    v20 = *vLand;
    v33[0] = (float)(v30 * (float)v13) + *vFrom;
    v21 = vLand[1];
    v22 = v32;
    v23 = (float)((float)(v31 * (float)v13) + vFrom[1]);
    v33[1] = (float)(v31 * (float)v13) + vFrom[1];
    v32 = 0.0;
    if ((float)((float)((float)((float)v21 - (float)v23) * (float)((float)v21 - (float)v23))
        + (float)((float)((float)v20 - v33[0]) * (float)((float)v20 - v33[0]))) > 36864.0)
        goto LABEL_28;
    v24 = (float)((float)v22 * (float)v13);
    if ((float)((float)((float)v18 * (float)v12) - (float)((float)v22 * (float)v13)) >= 0.0)
        goto LABEL_28;
    v25 = (float)((float)(g_gravity->current.value
        / (float)((float)((float)v18 * (float)v12) - (float)((float)v22 * (float)v13)))
        * (float)-0.5);
    v26 = sqrtf((float)((float)((float)((float)((float)v13 - (float)v18)
        * (float)((float)(g_gravity->current.value
            / (float)((float)((float)v18 * (float)v12)
                - (float)((float)v22 * (float)v13)))
            * (float)-0.5))
        * (float)v18)
        * (float)v13));
    if (v26 == 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 605, 0, "%s", "fVelHorz != 0.0f");
    v27 = (float)((float)((float)((float)((float)v24 * (float)v13) - (float)((float)((float)v19 * (float)v19) * (float)v12))
        / (float)v26)
        * (float)v25);
    if ((float)((float)((float)v27 * (float)v27) + (float)((float)v26 * (float)v26)) > 810000.0
        || (v28 = (float)(v31 * (float)v26),
            *vVelOut = v30 * (float)v26,
            vVelOut[1] = v28,
            vVelOut[2] = (float)((float)((float)((float)v24 * (float)v13)
                - (float)((float)((float)v19 * (float)v19) * (float)v12))
                / (float)v26)
            * (float)v25,
            !Actor_Grenade_IsValidTrajectory(self, vFrom, vVelOut, v33)))
    {
    LABEL_28:
        grenadeHintCount = level.grenadeHintCount;
        goto LABEL_29;
    }
    //Profile_EndInternal(0);
    return 1;
}

int __cdecl compare_desperate_hints(float *pe0, float *pe1)
{
    double v2; // fp0
    double v3; // fp13

    v2 = (float)((float)((float)(pe0[1] - g_vRefPos[1]) * (float)(pe0[1] - g_vRefPos[1]))
        + (float)((float)((float)(*pe0 - g_vRefPos[0]) * (float)(*pe0 - g_vRefPos[0]))
            + (float)((float)(pe0[2] - g_vRefPos[2]) * (float)(pe0[2] - g_vRefPos[2]))));
    v3 = (float)((float)((float)(pe1[1] - g_vRefPos[1]) * (float)(pe1[1] - g_vRefPos[1]))
        + (float)((float)((float)(pe1[2] - g_vRefPos[2]) * (float)(pe1[2] - g_vRefPos[2]))
            + (float)((float)(*pe1 - g_vRefPos[0]) * (float)(*pe1 - g_vRefPos[0]))));
    if (v2 >= v3)
        return v2 > v3;
    else
        return -1;
}

int __cdecl Actor_Grenade_CheckDesperateToss(actor_s *self, float *vFrom, float *vVelOut)
{
    unsigned int v6; // r30
    float *v7; // r31

    //Profile_Begin(353);
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 664, 0, "%s", "self");
    if (!vFrom)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 665, 0, "%s", "vFrom");
    if (g_gravity->current.value <= 0.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
            669,
            0,
            "%s",
            "g_gravity->current.value > 0");
    if (level.grenadeHintCount
        && (g_vRefPos[0] = *vFrom,
            g_vRefPos[1] = vFrom[1],
            g_vRefPos[2] = vFrom[2],
            qsort(
                g_vGrenadeHint,
                level.grenadeHintCount,
                0xCu,
                (int(__cdecl *)(const void *, const void *))compare_desperate_hints),
            v6 = 0,
            level.grenadeHintCount))
    {
        v7 = g_vGrenadeHint[0];
        while (!Actor_Grenade_CheckMaximumEnergyToss(self, vFrom, v7, 0, vVelOut))
        {
            ++v6;
            v7 += 3;
            if (v6 >= level.grenadeHintCount)
                goto LABEL_12;
        }
        //Profile_EndInternal(0);
        return 1;
    }
    else
    {
    LABEL_12:
        //Profile_EndInternal(0);
        return 0;
    }
}

bool __cdecl Actor_GrenadeLauncher_CheckPos(
    actor_s *self,
    const float *vStandPos,
    const float *vOffset,
    const float *vTargetPos,
    double speed,
    float *vPosOut,
    float *vVelOut,
    float *a8)
{
    long double v16; // fp2
    double v17; // fp11
    long double v18; // fp2
    float *v19; // r3
    double v20; // fp13
    double v21; // fp0
    float v22; // [sp+50h] [-60h]
    float v23; // [sp+54h] [-5Ch] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 856, 0, "%s", "self");
    if (!vStandPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 857, 0, "%s", "vStandPos");
    if (!vTargetPos)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 858, 0, "%s", "vTargetPos");
    if (!vVelOut)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 859, 0, "%s", "vPosOut");
    if (!a8)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 860, 0, "%s", "vVelOut");
    if (g_gravity->current.value <= 0.0)
        return 0;
    if (vOffset)
    {
        Actor_Grenade_GetTossFromPosition(self, vStandPos, vOffset, vVelOut);
    }
    else
    {
        *vVelOut = *vStandPos;
        vVelOut[1] = vStandPos[1];
        vVelOut[2] = vStandPos[2];
    }
    *(double *)&v16 = Vec2Distance(vVelOut, vTargetPos);
    if (*(double *)&v16 == 0.0)
        return 0;
    v17 = (float)((float)((float)((float)((float)((float)(vTargetPos[2] - vVelOut[2])
        / (float)((float)speed * (float)speed))
        * (float)((float)*(double *)&v16 * (float)*(double *)&v16))
        * g_gravity->current.value)
        * (float)2.0)
        + (float)((float)((float)*(double *)&v16 * (float)*(double *)&v16)
            - (float)((float)((float)((float)((float)((float)*(double *)&v16 * (float)*(double *)&v16)
                * (float)((float)*(double *)&v16 * (float)*(double *)&v16))
                * g_gravity->current.value)
                * g_gravity->current.value)
                / (float)((float)((float)speed * (float)speed) * (float)((float)speed * (float)speed)))));
    if (v17 < 0.0)
        return 0;
    *(double *)&v16 = (float)((float)((float)((float)sqrtf(v17)
        - (float)((float)*(double *)&v16 * (float)*(double *)&v16))
        * (float)((float)speed * (float)speed))
        / (float)((float)((float)*(double *)&v16 * (float)*(double *)&v16) * g_gravity->current.value));
    v18 = atan(v16);
    FastSinCos((float)*(double *)&v18, v19, &v23);
    *a8 = *vTargetPos - *vVelOut;
    a8[1] = vTargetPos[1] - vVelOut[1];
    Vec2Normalize(a8);
    v20 = (float)(*a8 * v22);
    v21 = (float)(a8[1] * v22);
    a8[2] = v23 * (float)speed;
    *a8 = (float)v20 * (float)speed;
    a8[1] = (float)v21 * (float)speed;
    return Actor_Grenade_IsValidTrajectory(self, vVelOut, a8, vTargetPos);
}

int Actor_Grenade_IsSafeTarget(actor_s *self, const float *vTargetPos, unsigned int iWeapID)
{
    WeaponDef *weapDef; // r28
    float explosionCutoff; // fp31
    int v9; // r30
    sentient_s *Sentient; // r31
    float v12; // [sp+58h] [-48h] BYREF
    float v13; // [sp+5Ch] [-44h]
    float v14; // [sp+60h] [-40h]

    iassert(self);
    iassert(self->sentient);
    weapDef = BG_GetWeaponDef(iWeapID);
    iassert(weapDef);

    explosionCutoff = (float)((float)weapDef->iExplosionRadius * (float)1.1);
    v9 = ~(1 << Sentient_EnemyTeam(self->sentient->eTeam));
    Sentient = Sentient_FirstSentient(v9);
    if (!Sentient)
        return 1;
    while (1)
    {
        Sentient_GetOrigin(Sentient, &v12);
        if ((float)((float)((float)(vTargetPos[1] - v13) * (float)(vTargetPos[1] - v13))
            + (float)((float)((float)(vTargetPos[2] - v14) * (float)(vTargetPos[2] - v14))
                + (float)((float)(*vTargetPos - v12) * (float)(*vTargetPos - v12)))) <= explosionCutoff)
            break;
        Sentient = Sentient_NextSentient(Sentient, v9);
        if (!Sentient)
            return 1;
    }
    return 0;
}

void __cdecl Actor_PredictGrenadeLandPos(gentity_s *pGrenade)
{
    int clipmask; // r30
    int nextthink; // r10
    int v4; // r3
    double v5; // fp13
    double v6; // fp12
    int v7; // [sp+50h] [-30h] BYREF
    float v8[4]; // [sp+58h] [-28h] BYREF

    if (!pGrenade)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 967, 0, "%s", "pGrenade");
    if (pGrenade->mover.decelTime == 0.0 && pGrenade->mover.aDecelTime == 0.0 && pGrenade->mover.speed == 0.0)
    {
        clipmask = pGrenade->clipmask;
        nextthink = pGrenade->nextthink;
        pGrenade->clipmask = clipmask & 0xFDFF3FFF;
        v4 = G_PredictMissile(pGrenade, nextthink - level.time, v8, 1, &v7);
        pGrenade->item[1].ammoCount = v7;
        if (v4)
        {
            v5 = v8[1];
            v6 = v8[2];
            pGrenade->mover.decelTime = v8[0];
            pGrenade->mover.aDecelTime = v5;
            pGrenade->mover.speed = v6;
        }
        else
        {
            pGrenade->mover.decelTime = pGrenade->r.currentOrigin[0];
            pGrenade->mover.aDecelTime = pGrenade->r.currentOrigin[1];
            pGrenade->mover.speed = pGrenade->r.currentOrigin[2];
        }
        pGrenade->clipmask = clipmask;
    }
}

bool __cdecl Actor_Grenade_IsPointSafe(actor_s *self, const float *vPoint)
{
    EntHandle *p_pGrenade; // r31
    gentity_s *v5; // r3
    WeaponDef *WeaponDef; // r27
    __int64 v7; // r10
    double v9; // fp29
    gentity_s *v10; // r11
    EntHandle *v11; // r3
    const float *v12; // r31
    gentity_s *v13; // r3
    bool v14; // r8
    float *v15; // r7

    p_pGrenade = &self->pGrenade;

    iassert(self->pGrenade.isDefined());
    v5 = p_pGrenade->ent();
    WeaponDef = BG_GetWeaponDef(v5->s.weapon);
    if (!WeaponDef)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 1051, 0, "%s", "weapDef");
    if (WeaponDef->projExplosion == WEAPPROJEXP_FLASHBANG)
        return 1;
    LODWORD(v7) = WeaponDef->iExplosionRadius;
    v9 = (float)((float)v7 * (float)1.1);
    v10 = p_pGrenade->ent();
    v11 = p_pGrenade;
    v12 = (const float *)&v10->missile.predictLandPos;
    v13 = v11->ent();
    return G_CanRadiusDamageFromPos(self->ent, vPoint, v13, v12, v9, 1.0, v15, 0.0, v14, 0) == 0;
}

float __cdecl Actor_Grenade_EscapePlane(actor_s *self, float *normal)
{
    double v4; // fp31
    gentity_s *TargetEntity; // r31
    gentity_s *ent; // r11
    const float *v7; // r4
    gentity_s *v8; // r3
    double v10; // fp13
    int integer; // r10
    int number; // r11
    double v13; // fp1

    v4 = 0.0;
    iassert(self);
    iassert(self->pGrenade.isDefined());
    iassert(normal);
    
    TargetEntity = Actor_GetTargetEntity(self);
    if (TargetEntity)
    {
        ent = self->ent;
        *normal = TargetEntity->r.currentOrigin[0] - self->ent->r.currentOrigin[0];
        normal[1] = TargetEntity->r.currentOrigin[1] - ent->r.currentOrigin[1];
        v8 = self->pGrenade.ent();
        float *v9 = (float)((float)(*normal * *normal) + (float)(normal[1] * normal[1])) 
            >= (double)(float)((float)(*normal * (float)(TargetEntity->r.currentOrigin[0] - v8->mover.decelTime)) + (float)(normal[1] * (float)(TargetEntity->r.currentOrigin[1] - v8->mover.aDecelTime)))
            ? self->pGrenade.ent()->missile.predictLandPos
            : self->ent->r.currentOrigin;
        v10 = normal[1];
        v4 = (float)((float)((float)sqrtf((float)((float)(*normal * *normal) + (float)(normal[1] * normal[1])))
            * (float)15.0)
            + (float)((float)(v9[1] * normal[1]) + (float)(*v9 * *normal)));
        if ((float)((float)(v9[1] * normal[1]) + (float)(*v9 * *normal)) > v4)
        {
            *normal = -*normal;
            normal[1] = -v10;
            v4 = -v4;
        }
        integer = ai_debugEntIndex->current.integer;
        number = self->ent->s.number;
        if (integer != number)
            goto LABEL_16;
        if (ai_showSuppression->current.integer > 0)
        {
        LABEL_17:
            G_DebugPlane(normal, v4, v7, self->ent->r.currentOrigin, 100.0, (int)colorOrange, (int)self->ent);
            goto LABEL_18;
        }
        if (integer != number)
        {
        LABEL_16:
            if (ai_showSuppression->current.integer == number)
                goto LABEL_17;
        }
    }
LABEL_18:
    v13 = v4;
    return *((float *)&v13 + 1);
}

void __cdecl Actor_Grenade_GetPickupPos(actor_s *self, const float *enemyPos, float *vGrenadePickupPos)
{
    gentity_s *v6; // r3
    double v7; // fp13
    gentity_s *v8; // r3
    double v9; // fp12
    double v10; // fp11
    float v11; // [sp+50h] [-40h] BYREF
    float v12; // [sp+54h] [-3Ch]
    float v13; // [sp+58h] [-38h]

    iassert(self);
    iassert(enemyPos);

    v6 = self->pGrenade.ent();
    v7 = enemyPos[1];
    v11 = v6->mover.decelTime - *enemyPos;
    v12 = v6->mover.aDecelTime - (float)v7;
    Vec2Normalize(&v11);
    v13 = 0.0;
    v8 = self->pGrenade.ent();
    v9 = v12;
    v10 = v13;
    *vGrenadePickupPos = (float)(v11 * (float)29.5) + v8->mover.decelTime;
    vGrenadePickupPos[1] = (float)((float)v9 * (float)29.5) + v8->mover.aDecelTime;
    vGrenadePickupPos[2] = (float)((float)v10 * (float)29.5) + v8->mover.speed;
}

bool __cdecl Actor_Grenade_ShouldIgnore(actor_s *self, gentity_s *grenade)
{
    gentity_s *ent; // r11
    double v5; // fp31
    sentient_s *sentient; // r30
    double v8; // fp0
    double v9; // fp13
    double v10; // fp12
    float v11; // [sp+50h] [-40h] BYREF
    float v12; // [sp+54h] [-3Ch]
    
    iassert(self);
    iassert(self->sentient);
    iassert(grenade);

    if (!Actor_HasPath(self) || (unsigned __int8)Actor_PointAtGoal(self->ent->r.currentOrigin, &self->scriptGoal))
        return 0;

    ent = self->ent;
    v11 = grenade->mover.decelTime - self->ent->r.currentOrigin[0];
    v12 = grenade->mover.aDecelTime - ent->r.currentOrigin[1];
    Vec2Normalize(&v11);
    v5 = (float)((float)(self->Path.lookaheadDir[1] * v12) + (float)(self->Path.lookaheadDir[0] * v11));
    if (v5 < 0.0)
        return 1;
    if (grenade->parent.isDefined())
    {
        if (grenade->parent.ent()->sentient)
        {
            sentient = grenade->parent.ent()->sentient;
            if (sentient->eTeam != Sentient_EnemyTeam(self->sentient->eTeam))
                return 0;
        }
    }

    if (v5 <= 0.7f)
        return 0;

    v8 = (float)(self->ent->r.currentOrigin[0] - grenade->mover.decelTime);
    v9 = (float)(self->ent->r.currentOrigin[2] - grenade->mover.speed);
    v10 = (float)(self->ent->r.currentOrigin[1] - grenade->mover.aDecelTime);
    return (float)((float)((float)v10 * (float)v10)
        + (float)((float)((float)v8 * (float)v8) + (float)((float)v9 * (float)v9))) > 100.0;
}

int __cdecl Actor_IsAwareOfGrenade(actor_s *self)
{
    int result; // r3
    __int64 v3; // r10

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 1515, 0, "%s", "self");
    if (self->grenadeAwareness == 0.0)
        return 0;
    if (!Actor_GetTargetEntity(self))
        return 1;
    LODWORD(v3) = G_rand();
    result = 0;
    if ((float)v3 <= (double)(float)(self->grenadeAwareness * (float)32767.0))
        return 1;
    return result;
}

void __cdecl Actor_GrenadePing(actor_s *self, gentity_s *pGrenade)
{
    unsigned int stateLevel; // r11
    gentity_s *v5; // r3
    sentient_s *sentient; // r30
    float v7[16]; // [sp+50h] [-40h] BYREF

    iassert(self);
    iassert(pGrenade);

    if (!self->flashBanged)
    {
        stateLevel = self->stateLevel;
        if (self->eState[stateLevel] != AIS_GRENADE_RESPONSE || self->eSubState[stateLevel] != STATE_GRENADE_THROWBACK)
        {
            if (!self->pGrenade.isDefined()
                || (v5 = self->pGrenade.ent(), v5->mover.decelTime == 0.0)
                && v5->mover.aDecelTime == 0.0
                && v5->mover.speed == 0.0)
            {
                if ((unsigned __int8)Actor_IsAwareOfGrenade(self))
                {
                    self->pGrenade.setEnt(pGrenade);
                    if (pGrenade->parent.isDefined())
                    {
                        if (pGrenade->parent.ent()->sentient)
                        {
                            sentient = pGrenade->parent.ent()->sentient;
                            if (sentient->eTeam == Sentient_EnemyTeam(self->sentient->eTeam))
                            {
                                Scr_AddEntity(pGrenade);
                                Scr_Notify(self->ent, scr_const.grenadedanger, 1u);
                            }
                        }
                    }
                    Actor_PredictGrenadeLandPos(pGrenade);
                    if (Actor_Grenade_ShouldIgnore(self, pGrenade)
                        || (Sentient_GetOrigin(self->sentient, v7), Actor_Grenade_IsPointSafe(self, v7)))
                    {
                        self->pGrenade.setEnt(NULL);
                    }
                }
            }
        }
    }
}

void __cdecl Actor_DissociateGrenade(gentity_s *pGrenade)
{
    EntHandleDissociate(pGrenade);
    pGrenade->activator = 0;
}

void __cdecl Actor_Grenade_Attach(actor_s *self)
{
    gentity_s *v2; // r28
    WeaponDef *weapDef; // r30
    const char *Name; // r30
    unsigned __int8 v5; // r11

    iassert(self);
    iassert(self->pGrenade.isDefined());
    v2 = self->pGrenade.ent();
    weapDef = BG_GetWeaponDef(v2->s.weapon);
    iassert(weapDef);
    Name = XModelGetName(weapDef->worldModel[v2->s.weaponModel]);
    G_EntDetach(self->ent, Name, scr_const.grenade_return_hand_tag);
    G_EntAttach(self->ent, Name, scr_const.grenade_return_hand_tag, 0);
    if (self->vGrenadeTossVel[0] != 0.0 || self->vGrenadeTossVel[1] != 0.0)
        Actor_FaceVector(&self->CodeOrient, self->vGrenadeTossVel);
    Actor_SetOrientMode(self, AI_ORIENT_DONT_CHANGE);
    v5 = v2->r.svFlags | 1;
    v2->s.lerp.pos.trType = TR_STATIONARY;
    v2->r.svFlags = v5;
    v2->parent.setEnt(self->ent);
}

void __cdecl Actor_Grenade_Detach(actor_s *self)
{
    gentity_s *v2; // r31
    WeaponDef *WeaponDef; // r30
    const char *Name; // r3
    unsigned __int8 svFlags; // r10
    float v6; // [sp+50h] [-40h] BYREF
    float v7; // [sp+54h] [-3Ch]
    float v8; // [sp+58h] [-38h]

    iassert(self);
    iassert(self->pGrenade.isDefined());
    
    v2 = self->pGrenade.ent();
    WeaponDef = BG_GetWeaponDef(v2->s.weapon);
    if (!WeaponDef)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 1693, 0, "%s", "weapDef");
    Name = XModelGetName(WeaponDef->worldModel[v2->s.weaponModel]);
    G_EntDetach(self->ent, Name, scr_const.grenade_return_hand_tag);
    G_DObjGetWorldTagPos_CheckTagExists(self->ent, scr_const.grenade_return_hand_tag, &v6);
    v2->r.currentOrigin[0] = v6;
    v2->r.currentOrigin[1] = v7;
    v2->r.currentOrigin[2] = v8;
    v2->s.lerp.pos.trBase[0] = v6;
    v2->s.lerp.pos.trBase[1] = v7;
    v2->s.lerp.pos.trBase[2] = v8;
    v2->s.lerp.pos.trDelta[0] = 0.0;
    v2->s.lerp.pos.trDelta[1] = 0.0;
    v2->s.lerp.pos.trDelta[2] = 0.0;
    svFlags = v2->r.svFlags;
    v2->activator = 0;
    v2->r.svFlags = svFlags & 0xFE;
    v2->mover.decelTime = 0.0;
    v2->mover.aDecelTime = 0.0;
    v2->mover.speed = 0.0;
}

int __cdecl Actor_Grenade_InActorHands(gentity_s *grenade)
{
    if (!grenade)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 1745, 0, "%s", "grenade");
    return grenade->r.svFlags & 1;
}

bool __cdecl Actor_Grenade_Resume(actor_s *self, ai_state_t ePrevState)
{
    int result; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 1810, 0, "%s", "self");
    if (!self->pGrenade.isDefined())
        return 0;
    result = 1;
    if (!self->bGrenadeTossValid)
        return 0;
    return result;
}

void __cdecl Actor_Grenade_Cower(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 1828, 0, "%s", "self");
    Actor_SetOrientMode(self, AI_ORIENT_DONT_CHANGE);
    Actor_AnimStop(self, &g_animScriptTable[self->species]->grenade_cower);
}

void __cdecl Actor_Grenade_Combat(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 1844, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 1845, 0, "%s", "self->sentient");
    if (Actor_GetTargetEntity(self))
    {
        Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY);
        Actor_AnimCombat(self);
    }
    else
    {
        Actor_SetSubState(self, STATE_GRENADE_COWER);
        Actor_Grenade_Cower(self);
    }
}

void __cdecl Actor_Grenade_CoverAttack(actor_s *self)
{
    iassert(self);
    iassert(self->sentient);
    iassert(self->pAnimScriptFunc);

    if (Actor_GetTargetEntity(self))
    {
        Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY);
        Actor_AnimSpecific(self, self->pAttackScriptFunc, AI_ANIM_USE_BOTH_DELTAS, 1);
    }
    else
    {
        Actor_SetSubState(self, STATE_GRENADE_COWER);
        Actor_Grenade_Cower(self);
    }
}

void __cdecl Actor_Grenade_Flee(actor_s *self)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 1893, 0, "%s", "self");
    if (Actor_HasPath(self))
    {
        Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY_OR_MOTION);
        Actor_TeamMoveBlockedClear(self);
        Actor_MoveAlongPathWithTeam(self, 1, 0, 0);
    }
    else if (Actor_Grenade_IsPointSafe(self, self->ent->r.currentOrigin))
    {
        Actor_SetSubState(self, STATE_GRENADE_COMBAT);
        Actor_Grenade_Combat(self);
    }
    else
    {
        Actor_SetSubState(self, STATE_GRENADE_COWER);
        Actor_Grenade_Cower(self);
    }
}

void __cdecl Actor_Grenade_TakeCover(actor_s *self)
{
    pathnode_t *pClaimedNode; // r4

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 1926, 0, "%s", "self");
    if (Actor_HasPath(self))
    {
        Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY_OR_MOTION);
        Actor_TeamMoveBlockedClear(self);
        Actor_MoveAlongPathWithTeam(self, 1, 0, 0);
    }
    else
    {
        pClaimedNode = self->sentient->pClaimedNode;
        if (pClaimedNode
            && (Actor_Cover_PickAttackScript(self, pClaimedNode, 1, &self->pAttackScriptFunc), self->pAttackScriptFunc)
            && Actor_Grenade_IsPointSafe(self, self->ent->r.currentOrigin))
        {
            Actor_SetSubState(self, STATE_GRENADE_COVERATTACK);
            Actor_Grenade_CoverAttack(self);
        }
        else
        {
            Actor_SetSubState(self, STATE_GRENADE_FLEE);
            Actor_Grenade_Flee(self);
        }
    }
}

actor_think_result_t __cdecl Actor_Grenade_ThrowBack(actor_s *self)
{
    gentity_s *v2; // r3
    double v3; // fp31
    double v4; // fp30
    double v5; // fp29
    const char *v6; // r3

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 1961, 0, "%s", "self");
    Actor_ClearPileUp(self);
    Actor_SetAnimScript(self, &g_animScriptTable[self->species]->grenade_return_throw, 0, AI_ANIM_MOVE_CODE);
    self->bUseGoalWeight = 1;
    if (Actor_IsAnimScriptAlive(self))
    {
        Actor_UpdateOriginAndAngles(self);
        return ACTOR_THINK_DONE;
    }
    else
    {
        Actor_SetState(self, AIS_EXPOSED);
        if (self->pGrenade.isDefined() && self->pGrenade.ent()->activator == self->ent)
        {
            v2 = self->pGrenade.ent();
            if (Actor_Grenade_InActorHands(v2))
            {
                v3 = self->ent->r.currentOrigin[2];
                v4 = self->ent->r.currentOrigin[1];
                v5 = self->ent->r.currentOrigin[0];
                SL_ConvertToString(self->ent->classname);
                v6 = va(
                    "ai classname '%s' at (%g %g %g) finished the grenade return script without throwing a grenade",
                    (const char *)HIDWORD(v5),
                    v5,
                    v4,
                    v3);
                Scr_Error(v6);
            }
        }
        return ACTOR_THINK_REPEAT;
    }
}

void __cdecl G_DrawGrenadeHints()
{
    gentity_s *gentities; // r9
    unsigned int v6; // r25
    unsigned int grenadeHintCount; // r10
    double v8; // fp30
    double v9; // fp29
    double v10; // fp28
    float *v11; // r31
    double v12; // fp26
    float *v13; // r30
    double v14; // fp0
    double v15; // fp13
    double v16; // fp12
    const float *v17; // r4
    double v18; // fp0
    double v19; // fp13
    double v22; // fp11
    double v23; // fp12
    double v24; // fp12
    double v25; // fp13
    double v26; // fp7
    double v27; // fp11
    double v28; // fp12
    __int64 v29; // [sp+50h] [-C0h] BYREF
    float v30; // [sp+58h] [-B8h]
    float v31; // [sp+60h] [-B0h] BYREF
    float v32; // [sp+64h] [-ACh]
    float v33; // [sp+68h] [-A8h]
    float v34[4]; // [sp+70h] [-A0h] BYREF
    float v35[4]; // [sp+80h] [-90h] BYREF
    float v36; // [sp+90h] [-80h] BYREF
    float v37; // [sp+94h] [-7Ch]
    float v38; // [sp+98h] [-78h]

    if (g_drawGrenadeHints->current.integer > 0)
    {
#if 0 // KISAKTODO
        gentities = level.gentities;
        if (level.gentities->client)
        {
            v6 = 0;
            v34[0] = 0.0;
            v34[1] = 0.0;
            grenadeHintCount = level.grenadeHintCount;
            v8 = level.clients->ps.origin[0];
            v9 = (float)(level.clients->ps.viewHeightCurrent + level.clients->ps.origin[2]);
            v10 = level.clients->ps.origin[1];
            v34[2] = 1.0;
            if (level.grenadeHintCount)
            {
                LODWORD(a5) = a5 * a5;
                v29 = a5;
                v11 = &g_vGrenadeHint[0][2];
                v12 = (float)a5;
                do
                {
                    v13 = v11 - 2;
                    v14 = (float)(gentities->r.currentOrigin[1] - *(v11 - 1));
                    v15 = (float)(gentities->r.currentOrigin[2] - *v11);
                    v16 = (float)(gentities->r.currentOrigin[0] - *(v11 - 2));
                    if ((float)((float)((float)v16 * (float)v16)
                        + (float)((float)((float)v15 * (float)v15) + (float)((float)v14 * (float)v14))) <= v12)
                    {
                        G_DebugCircle(v11 - 2, 8.0, a2, (int)colorRed, 1, 0);
                        G_DebugCircle(v11 - 2, 192.0, v17, (int)colorOrange, 1, 0);
                        v18 = (float)(*(v11 - 1) - (float)v10);
                        v19 = (float)(*v11 - (float)v9);

                        // aislop
                        //_FP10 = -sqrtf((float)((float)((float)(*v13 - (float)v8) * (float)(*v13 - (float)v8))
                        //    + (float)((float)((float)(*v11 - (float)v9) * (float)(*v11 - (float)v9))
                        //        + (float)((float)v18 * (float)v18))));
                        //__asm { fsel      f11, f10, f31, f11 }
                        //v22 = (float)((float)1.0 / (float)_FP11);

                        {
                            float dx = *v13 - (float)v8;
                            float dy = *v11 - (float)v9;
                            float dz = (float)v18;

                            float distance = sqrtf(dx * dx + dy * dy + dz * dz);
                            if (distance <= 0.0f)
                                distance = FLT_MIN; // Avoid division by zero or negative sqrt result

                            v22 = 1.0f / distance;
                        }

                        v23 = (float)((float)v22 * (float)(*v13 - (float)v8));
                        v35[1] = (float)(*(v11 - 1) - (float)v10) * (float)v22;
                        v35[0] = v23;
                        v35[2] = (float)v19 * (float)v22;
                        Vec3Cross(v35, v34, &v36);
                        v24 = *(v11 - 1);
                        v25 = (float)-(float)((float)(v36 * 192.0) - *v13);
                        v26 = (float)((float)(v37 * 192.0) + *(v11 - 1));
                        v31 = (float)(v36 * 192.0) + *v13;
                        v27 = *v11;
                        v32 = v26;
                        *(float *)&v29 = v25;
                        v33 = (float)(v38 * (float)192.0) + (float)v27;
                        *((float *)&v29 + 1) = -(float)((float)(v37 * (float)192.0) - (float)v24);
                        v30 = -(float)((float)(v38 * (float)192.0) - (float)v27);
                        G_DebugLine(&v31, (const float *)&v29, colorOrange, 1);
                        v28 = *v11;
                        v31 = *v13;
                        *(float *)&v29 = v31;
                        v32 = *(v11 - 1);
                        v33 = (float)v28 + (float)192.0;
                        *((float *)&v29 + 1) = v32;
                        v30 = (float)v28 - (float)192.0;
                        G_DebugLine(&v31, (const float *)&v29, colorOrange, 1);
                        grenadeHintCount = level.grenadeHintCount;
                        gentities = level.gentities;
                    }
                    ++v6;
                    v11 += 3;
                } while (v6 < grenadeHintCount);
            }
        }
#endif
    }
}

static const float ACTOR_GRENADE_HINT_DEVIATION_RADIUS = 15.0f;

bool __cdecl Actor_Grenade_CheckTossPos(
    actor_s *self,
    float *vStandPos,
    float *vOffset,
    float *vTargetPos,
    unsigned int method,
    float *vPosOut,
    float *vVelOut,
    double randomRange,
    int bRechecking)
{
    double v38; // fp0
    double v39; // fp1
    double v40; // fp0
    double v41; // fp2
    char TossPositionsFromHints; // r29
    const float *v43; // r5
    bool v44; // r3
    float *v45; // r7
    int v46; // r6
    bool v47; // r30
    const float *v48; // r4
    const float *v49; // r4
    const char *v50; // r3
    const char *v51; // r3
    float v52; // [sp+58h] [-D8h] BYREF
    float v53; // [sp+5Ch] [-D4h]
    float v54; // [sp+60h] [-D0h]
    float v55; // [sp+68h] [-C8h] BYREF
    float v56; // [sp+6Ch] [-C4h]
    float v57; // [sp+70h] [-C0h]
    float v58; // [sp+78h] [-B8h] BYREF
    float v59; // [sp+7Ch] [-B4h]
    float v60; // [sp+80h] [-B0h]
    trace_t v61; // [sp+90h] [-A0h] BYREF

    iassert(self);
    iassert(vStandPos);
    iassert(method);

    iassert(!IS_NAN((vStandPos)[0]) && !IS_NAN((vStandPos)[1]) && !IS_NAN((vStandPos)[2]));
    iassert(!IS_NAN((vOffset)[0]) && !IS_NAN((vOffset)[1]) && !IS_NAN((vOffset)[2]));
    iassert(!IS_NAN((vTargetPos)[0]) && !IS_NAN((vTargetPos)[1]) && !IS_NAN((vTargetPos)[2]));
    
    if (g_gravity->current.value <= 0.0
        || self->pGrenade.isDefined() && !bRechecking
        || !Actor_Grenade_IsSafeTarget(self, vTargetPos, self->iGrenadeWeaponIndex))
    {
        return 0;
    }
    v56 = vTargetPos[1];
    v38 = *vTargetPos;
    v57 = vTargetPos[2];
    v55 = v38;
    if (randomRange != 0.0)
    {
        v39 = G_flrand((float)((float)v38 - (float)randomRange), (float)((float)v38 + (float)randomRange));
        v40 = vTargetPos[1];
        v41 = (float)(vTargetPos[1] + (float)randomRange);
        v55 = v39;
        v56 = G_flrand((float)((float)v40 - (float)randomRange), v41);
    }
    Actor_Grenade_GetTossFromPosition(self, vStandPos, vOffset, vPosOut);
    G_MissileTrace(&v61, vPosOut, &v55, ENTITYNUM_NONE, 2065);
    TossPositionsFromHints = 0;
    if (v61.fraction == 1.0)
    {
        Actor_Grenade_GetTossPositions(vPosOut, &v55, &v52, self->iGrenadeWeaponIndex);
        if (g_drawGrenadeHints->current.integer <= 0)
            goto LABEL_37;
        v43 = colorRed;
    }
    else
    {
        if (level.grenadeHintCount)
            TossPositionsFromHints = Actor_Grenade_GetTossPositionsFromHints(vPosOut, vTargetPos, method, &v58);
        if (TossPositionsFromHints)
        {
            v52 = G_flrand(
                (float)(v58 - ACTOR_GRENADE_HINT_DEVIATION_RADIUS),
                (float)(ACTOR_GRENADE_HINT_DEVIATION_RADIUS + v58));
            v53 = G_flrand(
                (float)(v59 - ACTOR_GRENADE_HINT_DEVIATION_RADIUS),
                (float)(ACTOR_GRENADE_HINT_DEVIATION_RADIUS + v59));
            v54 = G_flrand(
                (float)(v60 - ACTOR_GRENADE_HINT_DEVIATION_RADIUS),
                (float)(ACTOR_GRENADE_HINT_DEVIATION_RADIUS + v60));
            if (g_drawGrenadeHints->current.integer <= 0)
                goto LABEL_37;
            v43 = colorCyan;
        }
        else
        {
            v52 = v55;
            v53 = v56;
            v54 = v57;
            if (g_drawGrenadeHints->current.integer <= 0)
                goto LABEL_37;
            v43 = colorBlue;
        }
    }
    G_DebugLineWithDuration(vPosOut, &v52, v43, 0, 100);
LABEL_37:
    if ((float)((float)((float)(v53 - vPosOut[1]) * (float)(v53 - vPosOut[1]))
        + (float)((float)((float)(v52 - *vPosOut) * (float)(v52 - *vPosOut))
            + (float)((float)(v54 - vPosOut[2]) * (float)(v54 - vPosOut[2])))) == 0.0)
        return 0;
    if (method == scr_const.min_energy)
    {
        v44 = Actor_Grenade_CheckMinimumEnergyToss(self, vPosOut, &v52, vVelOut);
        goto LABEL_43;
    }
    if (method == scr_const.min_time)
    {
        v45 = vVelOut;
        v46 = 0;
    LABEL_42:
        v44 = Actor_Grenade_CheckMaximumEnergyToss(self, vPosOut, &v52, v46, v45);
        goto LABEL_43;
    }
    if (method == scr_const.max_time)
    {
        v45 = vVelOut;
        v46 = 1;
        goto LABEL_42;
    }
    if (method != scr_const.infinite_energy)
    {
        v50 = SL_ConvertToString(method);
        v51 = va("checkGrenadeThrow: method must be 'min energy', 'min time', or 'max time' - value passed in was %s", v50);
        Scr_Error(v51);
        return 0;
    }
    v44 = Actor_Grenade_CheckInfiniteEnergyToss(self, vPosOut, &v52, vVelOut);
LABEL_43:
    v47 = v44;
    if (v44 && g_drawGrenadeHints->current.integer > 0)
    {
        G_DebugLineWithDuration(vPosOut, &v55, colorWhite, 0, 100);
        G_DebugCircle(&v52, 3.0, v48, (int)colorWhite, 1, 0);
        if (TossPositionsFromHints)
            G_DebugCircle(&v58, 6.0, v49, (int)colorCyan, 1, 0);
    }
    return v47;
}

bool __cdecl Actor_Grenade_AttemptReturnTo(
    actor_s *self,
    float *vFrom,
    float *vEnemyPos,
    float *vLand,
    float *vVelocity)
{
    float offset[3]; // [sp+60h] [-60h] BYREF

    offset[0] = 0.0;
    offset[1] = 0.0;
    offset[2] = 40.0;

    iassert(self);
    iassert(vEnemyPos);

    Scr_SetString(&self->GrenadeTossMethod, scr_const.min_energy);

    if (Actor_Grenade_CheckTossPos(
        self,
        vFrom,
        offset,
        vEnemyPos,
        scr_const.min_energy,
        vLand,
        vVelocity,
        0.0,
        1))
    {
        return 1;
    }
    Scr_SetString(&self->GrenadeTossMethod, scr_const.min_time);
    if (Actor_Grenade_CheckTossPos(
        self,
        vFrom,
        offset,
        vEnemyPos,
        scr_const.min_time,
        vLand,
        vVelocity,
        0.0,
        1))
    {
        return 1;
    }
    Scr_SetString(&self->GrenadeTossMethod, scr_const.max_time);
    return Actor_Grenade_CheckTossPos(
        self,
        vFrom,
        offset,
        vEnemyPos,
        scr_const.max_time,
        vLand,
        vVelocity,
        0.0,
        1);
}

int __cdecl Actor_Grenade_AttemptReturn(actor_s *self)
{
    gentity_s *v2; // r26
    EntHandle *p_parent; // r27
    sentient_s *sentient; // r28
    WeaponDef *WeaponDef; // r28
    float *currentOrigin; // r3
    gentity_s *v7; // r3
    bool v8; // r29
    const sentient_s *TargetSentient; // r3
    gentity_s *v10; // r3
    gentity_s *v11; // r3
    double v12; // fp0
    double v13; // fp13
    double v14; // fp12
    double v15; // fp11
    double v16; // fp10
    double v17; // fp9
    double v18; // fp13
    double v19; // fp12
    float decelTime; // [sp+50h] [-90h] BYREF
    float aDecelTime; // [sp+54h] [-8Ch]
    float speed; // [sp+58h] [-88h]
    float v24[4]; // [sp+60h] [-80h] BYREF
    float v25[4]; // [sp+70h] [-70h] BYREF
    float v26[4]; // [sp+80h] [-60h] BYREF
    float v27[20]; // [sp+90h] [-50h] BYREF

    iassert(self);
    iassert(self->pGrenade.isDefined());
    iassert(self->sentient);
    v2 = self->pGrenade.ent();
    p_parent = &v2->parent;
    if (!v2->parent.isDefined())
        return 0;
    if (!p_parent->ent()->sentient)
        return 0;
    sentient = p_parent->ent()->sentient;
    if (sentient->eTeam != Sentient_EnemyTeam(self->sentient->eTeam))
        return 0;
    WeaponDef = BG_GetWeaponDef(v2->s.weapon);
    if (!WeaponDef)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 1314, 0, "%s", "weapDef");
    if (WeaponDef->weapType == WEAPTYPE_PROJECTILE)
        return 0;
    currentOrigin = self->ent->r.currentOrigin;
    decelTime = v2->mover.decelTime;
    aDecelTime = v2->mover.aDecelTime;
    speed = v2->mover.speed;
    if (Vec2Distance(currentOrigin, (const float *)&v2->missile.predictLandPos) > 150.0)
        return 0;
    v7 = p_parent->ent();
    Sentient_GetOrigin(v7->sentient, v24);
    v8 = Actor_PointAt(self->ent->r.currentOrigin, (const float *)&v2->missile.predictLandPos);
    if (v8)
    {
        v25[0] = v2->mover.decelTime;
        v25[1] = v2->mover.aDecelTime;
        v25[2] = v2->mover.speed;
    }
    else
    {
        Actor_Grenade_GetPickupPos(self, v24, v25);
    }
    if (!Actor_Grenade_AttemptReturnTo(self, &decelTime, v24, v27, v26))
    {
        TargetSentient = Actor_GetTargetSentient(self);
        if (TargetSentient
            && (Sentient_GetOrigin(TargetSentient, v24), Actor_Grenade_AttemptReturnTo(self, &decelTime, v24, v27, v26)))
        {
            if (!v8)
                Actor_Grenade_GetPickupPos(self, v24, v25);
        }
        else
        {
            self->bGrenadeTargetValid = 0;
            if (!Actor_Grenade_CheckDesperateToss(self, &decelTime, v26))
                return 0;
        }
    }
    Actor_FindPath(self, v25, 0, 1);
    if (!Actor_HasPath(self))
        return 0;
    if (self->goalPosChanged)
        Scr_Notify(self->ent, scr_const.goal_changed, 0);
    if (!Actor_GetTargetEntity(self))
    {
        v10 = p_parent->ent();
        Actor_GetPerfectInfo(self, v10->sentient);
        v11 = p_parent->ent();
        Sentient_SetEnemy(self->sentient, v11, 1);
    }
    v12 = decelTime;
    v13 = aDecelTime;
    v14 = speed;
    v15 = v26[0];
    v16 = v26[1];
    self->bGrenadeTossValid = 1;
    v17 = v26[2];
    self->vGrenadeTossPos[0] = v12;
    self->vGrenadeTossPos[1] = v13;
    self->vGrenadeTossPos[2] = v14;
    self->vGrenadeTossVel[0] = v15;
    self->vGrenadeTossVel[1] = v16;
    self->vGrenadeTossVel[2] = v17;
    if (self->bGrenadeTargetValid)
    {
        v18 = v24[1];
        v19 = v24[2];
        self->vGrenadeTargetPos[0] = v24[0];
        self->vGrenadeTargetPos[1] = v18;
        self->vGrenadeTargetPos[2] = v19;
    }
    if (v2->activator != self->ent)
    {
        v2->nextthink += 1000;
        v2->activator = self->ent;
        Actor_SetSubState(self, STATE_GRENADE_ACQUIRE);
    }
    return 1;
}

void __cdecl Actor_Grenade_DropIfHeld(actor_s *self)
{
    gentity_s *v2; // r31
    gentity_s *v3; // r3

    iassert(self);

    if (self->pGrenade.isDefined() && self->pGrenade.ent()->activator == self->ent)
    {
        v2 = self->pGrenade.ent();
        if (Actor_Grenade_InActorHands(v2))
        {
            Actor_Grenade_Detach(self);
            G_InitGrenadeEntity(self->ent, v2);
            G_InitGrenadeMovement(v2, v2->r.currentOrigin, vec3_origin, 1);
            v3 = self->pGrenade.ent();
            SV_LinkEntity(v3);
        }
        self->pGrenade.ent()->activator = 0;
    }
}

void __cdecl Actor_Grenade_Finish(actor_s *self, ai_state_t eNextState)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 1781, 0, "%s", "self");
    Actor_Grenade_DropIfHeld(self);
}

void __cdecl Actor_Grenade_Suspend(actor_s *self, ai_state_t eNextState)
{
    iassert(self);
    Actor_Grenade_DropIfHeld(self);
}

bool __cdecl Actor_Grenade_CheckToss(
    actor_s *self,
    float *vStandPos,
    float *vOffset,
    unsigned int method,
    float *vPosOut,
    float *vVelOut,
    double randomRange,
    int bRechecking)
{
    const sentient_s *TargetSentient; // r30
    double v19; // fp0
    double v20; // fp13
    double v21; // fp12
    int v22; // r10
    int v23; // [sp+8h] [-E8h]
    int v24; // [sp+Ch] [-E4h]
    int v25; // [sp+10h] [-E0h]
    int v26; // [sp+14h] [-DCh]
    int v27; // [sp+18h] [-D8h]
    int v28; // [sp+1Ch] [-D4h]
    int v29; // [sp+20h] [-D0h]
    int v30; // [sp+24h] [-CCh]
    int v31; // [sp+28h] [-C8h]
    int v32; // [sp+2Ch] [-C4h]
    int v33; // [sp+30h] [-C0h]
    int v34; // [sp+34h] [-BCh]
    int v35; // [sp+38h] [-B8h]
    int v36; // [sp+3Ch] [-B4h]
    int v37; // [sp+40h] [-B0h]
    int v38; // [sp+44h] [-ACh]
    int v39; // [sp+48h] [-A8h]
    int v40; // [sp+4Ch] [-A4h]
    int v41; // [sp+50h] [-A0h]
    float v42[4]; // [sp+60h] [-90h] BYREF
    float v43[4]; // [sp+70h] [-80h] BYREF
    float v44[6]; // [sp+80h] [-70h] BYREF

    iassert(self);
    iassert(self->sentient);
    iassert(vStandPos);

    TargetSentient = Actor_GetTargetSentient(self);

    if (!TargetSentient || g_gravity->current.value <= 0.0 || self->pGrenade.isDefined() && !bRechecking)
        return 0;

    Sentient_GetOrigin(TargetSentient, v42);
    Sentient_GetVelocity(TargetSentient, v43);
    v19 = (float)(v43[0] + v42[0]);
    v20 = (float)(v43[1] + v42[1]);
    v21 = (float)(v43[2] + v42[2]);
    self->vGrenadeTargetPos[0] = v43[0] + v42[0];
    self->vGrenadeTargetPos[1] = v20;
    self->vGrenadeTargetPos[2] = v21;
    self->bGrenadeTargetValid = 1;
    v44[0] = v19;
    v44[1] = v20;
    v44[2] = v21;
    Scr_SetString(&self->GrenadeTossMethod, method);
    return Actor_Grenade_CheckTossPos(
        self,
        vStandPos,
        vOffset,
        v44,
        method,
        vPosOut,
        vVelOut,
        randomRange,
        bRechecking);
}

void __cdecl Actor_Grenade_AttemptEscape(actor_s *self, int bForceAbortPath)
{
    gentity_s *v4; // r3
    gentity_s *v5; // r3
    gentity_s *v6; // r3
    char v7; // r11
    char v8; // r26
    gentity_s *v9; // r3
    gentity_s *v10; // r3
    gentity_s *v11; // r3
    gentity_s *v12; // r30
    sentient_s *sentient; // r3
    sentient_s *v14; // r4
    gentity_s *v15; // r3
    WeaponDef *weapDef; // r30
    __int64 v17; // r11
    double v18; // fp31
    gentity_s *v19; // r3
    ai_substate_t v20; // r4
    gentity_s *v21; // r3
    pathnode_t *CoverFromPoint; // r3
    pathnode_t *v23; // r30
    double v24; // fp30
    gentity_s *v25; // r3
    float *v27; // r5
    gentity_s *v28; // r3
    float *v30; // r5
    gentity_s *v31; // r3
    int v32; // r5
    bool HasPath; // r3
    float v34[2]; // [sp+50h] [-60h] BYREF
    __int64 v35; // [sp+58h] [-58h]

    iassert(self);
    iassert(self->pGrenade.isDefined());
    iassert(self->sentient);
    v4 = self->pGrenade.ent();
    if (!v4->parent.isDefined()
        || (v5 = self->pGrenade.ent(), v6 = v5->parent.ent(), v7 = 1, v6 != self->ent))
    {
        v7 = 0;
    }
    v8 = v7;
    Actor_Grenade_DropIfHeld(self);
    if (!Actor_GetTargetEntity(self))
    {
        v9 = self->pGrenade.ent();
        if (v9->parent.isDefined())
        {
            v10 = self->pGrenade.ent();
            v11 = v10->parent.ent();
            v12 = v11;
            if (v11)
            {
                sentient = v11->sentient;
                if (sentient)
                {
                    v14 = self->sentient;
                    if (sentient != v14 && sentient->eTeam != v14->eTeam && !Actor_CheckIgnore(sentient, v14))
                    {
                        Actor_GetPerfectInfo(self, v12->sentient);
                        Sentient_SetEnemy(self->sentient, v12, 1);
                    }
                }
            }
        }
    }
    v15 = self->pGrenade.ent();
    weapDef = BG_GetWeaponDef(v15->s.weapon);
    iassert(weapDef);
    LODWORD(v17) = weapDef->iExplosionRadius;
    v35 = v17;
    v18 = (float)((float)v17 * (float)1.1);
    if (!bForceAbortPath)
    {
        if (Actor_HasPath(self))
        {
            v19 = self->pGrenade.ent();
            if (!Path_EncroachesPoint2D(
                &self->Path,
                self->ent->r.currentOrigin,
                v19->missile.predictLandPos,
                (float)((float)v18 * (float)v18)))
                goto LABEL_26;
        }
    }
    if (!v8)
    {
        v21 = self->pGrenade.ent();
        CoverFromPoint = Actor_Cover_FindCoverFromPoint(self, v21->missile.predictLandPos, v18);
        v23 = CoverFromPoint;
        if (CoverFromPoint && Actor_FindPathToNode(self, CoverFromPoint, 1))
        {
            Actor_ClearKeepClaimedNode(self);
            Sentient_ClaimNode(self->sentient, v23);
            v20 = STATE_GRENADE_TAKECOVER;
            goto LABEL_36;
        }
        goto LABEL_30;
    }
    Actor_FindPathToGoal(self);
    if (!Actor_HasPath(self)
        || (self->pGrenade.ent(), !Actor_Grenade_IsPointSafe(self, self->Path.vFinalGoal)))
    {
    LABEL_30:
        v24 = Actor_Grenade_EscapePlane(self, v34);
        v25 = self->pGrenade.ent();
        Actor_FindPathAwayNotCrossPlanes(self, v25->missile.predictLandPos, v18, v27, v24, v34, 1);
        if (Actor_HasPath(self))
        {
            Actor_SetSubState(self, STATE_GRENADE_FLEE);
            return;
        }
        if (!Actor_IsSuppressed(self)
            || (v28 = self->pGrenade.ent(),
                Actor_FindPathAwayNotCrossPlanes(self, v28->missile.predictLandPos, v18, v30, v24, v34, 0),
                !Actor_HasPath(self)))
        {
            v31 = self->pGrenade.ent();
            Actor_FindPathAway(self, v31->missile.predictLandPos, v18, v32);
            HasPath = Actor_HasPath(self);
            v20 = STATE_GRENADE_FLEE;
            if (!HasPath)
                v20 = STATE_GRENADE_COWER;
            goto LABEL_36;
        }
    }
LABEL_26:
    v20 = STATE_GRENADE_FLEE;
LABEL_36:
    Actor_SetSubState(self, v20);
}

void __cdecl Actor_Grenade_DecideResponse(actor_s *self)
{
    iassert(self);
    iassert(self->pGrenade.isDefined());
    iassert(self->sentient);

    if (self->pGrenade.ent()->activator
        || self->bGrenadeTossValid
        || self->pGrenade.ent()->missile.thrownBack
        || !Actor_Grenade_AttemptReturn(self))
    {
        Actor_Grenade_AttemptEscape(self, 0);
    }
}

int __cdecl Actor_Grenade_ReevaluateResponse(actor_s *self, ActorGrenadeReevaluateMode reevaluateMode)
{
    gentity_s *v4; // r3
    gentity_s *ent; // r30

    iassert(self);
    iassert(self->pGrenade.isDefined());
    
    if (reevaluateMode
        || (v4 = self->pGrenade.ent(), Vec2DistanceSq(self->ent->r.currentOrigin, v4->r.currentOrigin) > 22500.0)
        || !Actor_Grenade_AttemptReturn(self))
    {
        ent = self->ent;
        if (self->pGrenade.ent()->activator == ent && Actor_Grenade_IsPointSafe(self, ent->r.currentOrigin))
        {
            self->pGrenade.ent()->activator = 0;
            self->pGrenade.setEnt(NULL);
            Actor_SetState(self, AIS_EXPOSED);
            return 1;
        }
        Actor_Grenade_AttemptEscape(self, 1);
    }
    return 0;
}

void __cdecl Actor_GrenadeBounced(gentity_s *pGrenade, gentity_s *pHitEnt)
{
    actor_s *i; // r31

    if ((pHitEnt->r.contents & 0x200C000) != 0)
    {
        pGrenade->mover.decelTime = 0.0;
        pGrenade->mover.aDecelTime = 0.0;
        pGrenade->mover.speed = 0.0;
        Actor_PredictGrenadeLandPos(pGrenade);
        for (i = Actor_FirstActor(-1); i; i = Actor_NextActor(i, -1))
        {
            if (i->pGrenade.isDefined()
                && i->pGrenade.ent() == pGrenade
                && i->eSimulatedState[i->simulatedStateLevel] == AIS_GRENADE_RESPONSE)
            {
                Actor_Grenade_ReevaluateResponse(i, REEVALUATE_ATTEMPT_RETURN);
            }
        }
    }
}

bool __cdecl Actor_Grenade_Start(actor_s *self, ai_state_t ePrevState)
{
    int result; // r3

    iassert(self);

    result = self->pGrenade.isDefined();
    if (result)
    {
        Actor_Grenade_DecideResponse(self);
        if (self->eSubState[self->stateLevel] < STATE_GRENADE_FLEE)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
                1767,
                0,
                "%s",
                "self->eSubState[self->stateLevel] >= STATE_GRENADE_FLEE");
        return 1;
    }
    return result;
}

actor_think_result_t __cdecl Actor_Grenade_Acquire(actor_s *self)
{
    actor_s *pOther; // r28
    gentity_s *v3; // r30
    gentity_s *ent; // r30
    gentity_s *v6; // r3
    float v7; // [sp+50h] [-40h] BYREF
    float v8; // [sp+54h] [-3Ch]
    float v9; // [sp+58h] [-38h]

    iassert(self);
    iassert(self->pGrenade.isDefined());

    if (self->pPileUpEnt)
    {
        Actor_Grenade_AttemptEscape(self, 1);
        pOther = self->pPileUpEnt->actor;

        iassert(pOther != self);

        if (pOther)
        {
            if (pOther->pGrenade.isDefined())
            {
                v3 = pOther->pGrenade.ent();
                if (v3 == self->pGrenade.ent() && pOther->eState[pOther->stateLevel] == AIS_GRENADE_RESPONSE)
                    Actor_Grenade_ReevaluateResponse(pOther, REEVALUATE_ATTEMPT_RETURN);
            }
        }
        if (self->eSubState[self->stateLevel] == STATE_GRENADE_ACQUIRE)
            Actor_SetSubState(self, STATE_GRENADE_COWER);
        return ACTOR_THINK_REPEAT;
    }
    if (self->Physics.iHitEntnum != ENTITYNUM_NONE)
    {
        Actor_Grenade_AttemptEscape(self, 1);
        if (self->eSubState[self->stateLevel] == STATE_GRENADE_ACQUIRE)
        {
            Actor_SetSubState(self, STATE_GRENADE_COWER);
            return ACTOR_THINK_REPEAT;
        }
        return ACTOR_THINK_REPEAT;
    }
    if (Actor_HasPath(self))
    {
        Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY_OR_MOTION);
        Actor_AnimRun(self, &g_animScriptTable[self->species]->combat);
    }
    else
    {
        if (Vec2DistanceSq(self->ent->r.currentOrigin, self->Path.vFinalGoal) > 100.0)
        {
            if ((unsigned __int8)Actor_Grenade_ReevaluateResponse(self, REEVALUATE_NO_RETURN))
                return ACTOR_THINK_REPEAT;
            if (self->eSubState[self->stateLevel] == STATE_GRENADE_ACQUIRE)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
                    2038,
                    0,
                    "%s",
                    "self->eSubState[self->stateLevel] != STATE_GRENADE_ACQUIRE");
                return ACTOR_THINK_DONE;
            }
            return ACTOR_THINK_DONE;
        }
        if (I_fabs((float)(self->ent->r.currentOrigin[2] - self->Path.vFinalGoal[2])) > 32.0)
        {
            if ((unsigned __int8)Actor_Grenade_ReevaluateResponse(self, REEVALUATE_NO_RETURN))
                return ACTOR_THINK_REPEAT;
            if (self->eSubState[self->stateLevel] == STATE_GRENADE_ACQUIRE)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp",
                    2049,
                    0,
                    "%s",
                    "self->eSubState[self->stateLevel] != STATE_GRENADE_ACQUIRE");
        }
        ent = self->ent;
        v6 = self->pGrenade.ent();
        v7 = v6->mover.decelTime - ent->r.currentOrigin[0];
        v8 = v6->mover.aDecelTime - ent->r.currentOrigin[1];
        v9 = v6->mover.speed - ent->r.currentOrigin[2];
        if ((float)((float)(v7 * v7) + (float)(v8 * v8)) >= 1.0)
            Actor_FaceVector(&self->CodeOrient, &v7);
        Actor_SetOrientMode(self, AI_ORIENT_DONT_CHANGE);
        self->ScriptOrient.eMode = AI_ORIENT_INVALID;
        if (self->pGrenade.ent()->item[1].ammoCount <= level.time)
        {
            self->pGrenade.ent()->missile.thrownBack = 1;
            Actor_SetSubState(self, STATE_GRENADE_THROWBACK);
            return Actor_Grenade_ThrowBack(self);
        }
    }
    Actor_PostThink(self);
    return ACTOR_THINK_DONE;
}

actor_think_result_t __cdecl Actor_Grenade_Think(actor_s *self)
{
    actor_think_result_t result; // r3
    ai_substate_t v3; // r11
    ai_substate_t v4; // r4
    const char *v5; // r3

    iassert(self);

    if (!self->pGrenade.isDefined() || self->flashBanged)
    {
        Actor_SetState(self, AIS_EXPOSED);
        return ACTOR_THINK_REPEAT;
    }
    else
    {
        Actor_PreThink(self);
        v3 = self->eSubState[self->stateLevel];
        if ((v3 == STATE_GRENADE_TAKECOVER || v3 == STATE_GRENADE_COVERATTACK) && !self->sentient->pClaimedNode)
            Actor_Grenade_AttemptEscape(self, 1);
        v4 = self->eSubState[self->stateLevel];
        switch (v4)
        {
        case STATE_GRENADE_FLEE:
            self->pszDebugInfo = "grenade_flee";
            Actor_Grenade_Flee(self);
            goto LABEL_20;
        case STATE_GRENADE_TAKECOVER:
            self->pszDebugInfo = "grenade_takecover";
            Actor_Grenade_TakeCover(self);
            goto LABEL_20;
        case STATE_GRENADE_COWER:
            self->pszDebugInfo = "grenade_cower";
            Actor_Grenade_Cower(self);
            goto LABEL_20;
        case STATE_GRENADE_COMBAT:
            self->pszDebugInfo = "grenade_combat";
            Actor_Grenade_Combat(self);
            goto LABEL_20;
        case STATE_GRENADE_COVERATTACK:
            self->pszDebugInfo = "grenade_coverattack";
            Actor_Grenade_CoverAttack(self);
            goto LABEL_20;
        case STATE_GRENADE_ACQUIRE:
            self->pszDebugInfo = "grenade_acquire";
            return Actor_Grenade_Acquire(self);
        case STATE_GRENADE_THROWBACK:
            self->pszDebugInfo = "grenade_throwback";
            return Actor_Grenade_ThrowBack(self);
        default:
            if (!alwaysfails)
            {
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_grenade.cpp", 2152, 0, va("unhandled grenade state %i", v4));
            }
        LABEL_20:
            Actor_PostThink(self);
            result = ACTOR_THINK_DONE;
            break;
        }
    }
    return result;
}

