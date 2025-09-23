#include "game_public.h"
#include <server/sv_world.h>
#include <script/scr_const.h>
#include <server/sv_game.h>
#include <script/scr_vm.h>

#ifdef KISAK_MP
#include <game_mp/g_utils_mp.h>
#elif KISAK_SP
#include "g_main.h"
#include "g_local.h"
#include "g_public.h"
#endif


void __cdecl Reached_ScriptMover(gentity_s *pEnt)
{
    float v1; // [esp+18h] [ebp-24h]
    float v2; // [esp+1Ch] [ebp-20h]
    float v3; // [esp+20h] [ebp-1Ch]
    float v4; // [esp+24h] [ebp-18h]
    float v5; // [esp+2Ch] [ebp-10h]
    float v6; // [esp+34h] [ebp-8h]
    int bMoveFinished; // [esp+38h] [ebp-4h]
    int bMoveFinisheda; // [esp+38h] [ebp-4h]

    if (pEnt->s.lerp.pos.trType)
    {
        if (pEnt->s.lerp.pos.trDuration + pEnt->s.lerp.pos.trTime <= level.time)
        {
            bMoveFinished = ScriptMover_UpdateMove(
                &pEnt->s.lerp.pos,
                pEnt->r.currentOrigin,
                pEnt->mover.speed,
                pEnt->mover.midTime,
                pEnt->mover.decelTime,
                pEnt->mover.pos1,
                pEnt->mover.pos2,
                pEnt->mover.pos3);
            BG_EvaluateTrajectory(&pEnt->s.lerp.pos, level.time, pEnt->r.currentOrigin);
            SV_LinkEntity(pEnt);
            if (bMoveFinished)
                Scr_Notify(pEnt, scr_const.movedone, 0);
        }
    }
    if (pEnt->s.lerp.apos.trType && pEnt->s.lerp.apos.trDuration + pEnt->s.lerp.apos.trTime <= level.time)
    {
        bMoveFinisheda = ScriptMover_UpdateMove(
            &pEnt->s.lerp.apos,
            pEnt->r.currentAngles,
            pEnt->mover.aSpeed,
            pEnt->mover.aMidTime,
            pEnt->mover.aDecelTime,
            pEnt->mover.apos1,
            pEnt->mover.apos2,
            pEnt->mover.apos3);
        BG_EvaluateTrajectory(&pEnt->s.lerp.apos, level.time, pEnt->r.currentAngles);
        SV_LinkEntity(pEnt);
        if (bMoveFinisheda)
        {
            v6 = pEnt->r.currentAngles[0] * 0.002777777845039964;
            v4 = v6 + 0.5;
            v3 = floor(v4);
            pEnt->r.currentAngles[0] = (v6 - v3) * 360.0;
            pEnt->r.currentAngles[1] = AngleNormalize360(pEnt->r.currentAngles[1]);
            v5 = pEnt->r.currentAngles[2] * 0.002777777845039964;
            v2 = v5 + 0.5;
            v1 = floor(v2);
            pEnt->r.currentAngles[2] = (v5 - v1) * 360.0;
            Scr_Notify(pEnt, scr_const.rotatedone, 0);
        }
    }
}

int __cdecl ScriptMover_UpdateMove(
    trajectory_t *pTr,
    float *vCurrPos,
    float fSpeed,
    float fMidTime,
    float fDecelTime,
    const float *vPos1,
    const float *vPos2,
    const float *vPos3)
{
    float fDelta; // [esp+3Ch] [ebp-14h]
    float vMove[3]; // [esp+40h] [ebp-10h] BYREF
    int trDuration; // [esp+4Ch] [ebp-4h]

    trDuration = (int)(fMidTime * 1000.0);
    if (pTr->trType == TR_ACCELERATE && trDuration > 0)
    {
        pTr->trTime = level.time;
        pTr->trDuration = trDuration;
        pTr->trBase[0] = *vPos1;
        pTr->trBase[1] = vPos1[1];
        pTr->trBase[2] = vPos1[2];
        Vec3Sub(vPos2, vPos1, vMove);
        if (!trDuration)
            MyAssertHandler(".\\game\\g_scr_mover.cpp", 38, 0, "%s", "trDuration");
        fDelta = 1000.0 / (double)trDuration;
        Vec3Scale(vMove, fDelta, pTr->trDelta);
        if ((COERCE_UNSIGNED_INT(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\game\\g_scr_mover.cpp",
                41,
                0,
                "%s",
                "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])");
        }
        pTr->trType = TR_LINEAR_STOP;
        return 0;
    }
    if ((pTr->trType == TR_ACCELERATE && trDuration <= 0 || pTr->trType == TR_LINEAR_STOP) && fDecelTime > 0.0)
    {
        pTr->trTime = level.time;
        pTr->trDuration = (int)(fDecelTime * 1000.0);
        pTr->trBase[0] = *vPos2;
        pTr->trBase[1] = vPos2[1];
        pTr->trBase[2] = vPos2[2];
        Vec3Sub(vPos3, vPos2, vMove);
        Vec3Normalize(vMove);
        Vec3Scale(vMove, fSpeed, vMove);
        pTr->trDelta[0] = vMove[0];
        pTr->trDelta[1] = vMove[1];
        pTr->trDelta[2] = vMove[2];
        if ((COERCE_UNSIGNED_INT(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\game\\g_scr_mover.cpp",
                55,
                0,
                "%s",
                "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])");
        }
        pTr->trType = TR_DECELERATE;
        return 0;
    }
    if (pTr->trType == TR_GRAVITY || pTr->trType == TR_RAGDOLL_GRAVITY)
    {
        BG_EvaluateTrajectory(pTr, level.time, pTr->trBase);
    }
    else
    {
        pTr->trBase[0] = *vPos3;
        pTr->trBase[1] = vPos3[1];
        pTr->trBase[2] = vPos3[2];
    }
    pTr->trTime = level.time;
    pTr->trType = TR_STATIONARY;
    return 1;
}

void __cdecl InitScriptMover(gentity_s *pSelf)
{
    pSelf->r.svFlags = 0;
    pSelf->handler = ENT_HANDLER_SCRIPT_MOVER;
    pSelf->s.eType = ET_SCRIPTMOVER;
    pSelf->s.lerp.pos.trBase[0] = pSelf->r.currentOrigin[0];
    pSelf->s.lerp.pos.trBase[1] = pSelf->r.currentOrigin[1];
    pSelf->s.lerp.pos.trBase[2] = pSelf->r.currentOrigin[2];
    pSelf->s.lerp.pos.trType = TR_STATIONARY;
    pSelf->s.lerp.apos.trBase[0] = pSelf->r.currentAngles[0];
    pSelf->s.lerp.apos.trBase[1] = pSelf->r.currentAngles[1];
    pSelf->s.lerp.apos.trBase[2] = pSelf->r.currentAngles[2];
    pSelf->s.lerp.apos.trType = TR_STATIONARY;

    pSelf->flags |= FL_SUPPORTS_LINKTO;
}

void __cdecl SP_script_brushmodel(gentity_s *self)
{
    if (SV_SetBrushModel(self))
    {
        InitScriptMover(self);
        SV_LinkEntity(self);
    }
    else
    {
        Com_PrintError(
            1,
            "Killing script_brushmodel at (%f %f %f) because the brush model is invalid.\n",
            self->s.lerp.pos.trBase[0],
            self->s.lerp.pos.trBase[1],
            self->s.lerp.pos.trBase[2]);
        G_FreeEntity(self);
    }
}

void __cdecl SP_script_model(gentity_s *pSelf)
{
    G_DObjUpdate(pSelf);
    InitScriptMover(pSelf);
    pSelf->r.svFlags |= 4u;
    pSelf->r.contents = 8320;
    SV_LinkEntity(pSelf);
}

void __cdecl SP_script_origin(gentity_s *pSelf)
{
    InitScriptMover(pSelf);
    pSelf->r.contents = 0;
    SV_LinkEntity(pSelf);
    pSelf->r.svFlags |= 1u;
}

void __cdecl ScriptEntCmdGetCommandTimes(float *pfTotalTime, float *pfAccelTime, float *pfDecelTime)
{
    float fTotalTimeRoundedUp; // [esp+0h] [ebp-8h]
    int iNumParms; // [esp+4h] [ebp-4h]

    *pfTotalTime = Scr_GetFloat(1);
    if (*pfTotalTime <= 0.0)
        Scr_ParamError(1u, "total time must be positive");
    if (*pfTotalTime < EQUAL_EPSILON)
        *pfTotalTime = 0.001f;
    iNumParms = Scr_GetNumParam();
    if (iNumParms <= 2)
    {
        *pfAccelTime = 0.0f;
        *pfDecelTime = 0.0f;
    }
    else
    {
        *pfAccelTime = Scr_GetFloat(2);
        if (*pfAccelTime < 0.0f)
            Scr_ParamError(2u, "accel time must be nonnegative");
        if (iNumParms <= 3)
        {
            *pfDecelTime = 0.0f;
        }
        else
        {
            *pfDecelTime = Scr_GetFloat(3);
            if (*pfDecelTime < 0.0f)
                Scr_ParamError(3u, "decel time must be nonnegative");
        }
    }
    if (*pfTotalTime < *pfAccelTime + *pfDecelTime)
    {
        fTotalTimeRoundedUp = *pfTotalTime * 1.000000476837158f;
        if (fTotalTimeRoundedUp >= *pfAccelTime + *pfDecelTime)
            *pfTotalTime = fTotalTimeRoundedUp;
        else
            Scr_Error("accel time plus decel time is greater than total time");
    }
}

void __cdecl ScriptEntCmd_MoveTo(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+Ch] [ebp-1Ch]
    float fTotalTime; // [esp+10h] [ebp-18h] BYREF
    float fAccelTime; // [esp+14h] [ebp-14h] BYREF
    float fDecelTime; // [esp+18h] [ebp-10h] BYREF
    float vPos[3]; // [esp+1Ch] [ebp-Ch] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        
        pSelf = &g_entities[entref.entnum];
        if (pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light)
        {
            v1 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    Scr_GetVector(0, vPos);
    ScriptEntCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
    ScriptMover_Move(pSelf, vPos, fTotalTime, fAccelTime, fDecelTime);
}

void __cdecl ScriptMover_Move(gentity_s *pEnt, const float *vPos, float fTotalTime, float fAccelTime, float fDecelTime)
{
    float origin[3]; // [esp+2Ch] [ebp-Ch] BYREF

    origin[0] = pEnt->r.currentOrigin[0];
    origin[1] = pEnt->r.currentOrigin[1];
    origin[2] = pEnt->r.currentOrigin[2];
    ScriptMover_SetupMove(
        &pEnt->s.lerp.pos,
        vPos,
        fTotalTime,
        fAccelTime,
        fDecelTime,
        origin,
        &pEnt->mover.speed,
        &pEnt->mover.midTime,
        &pEnt->mover.aDecelTime,
        //(float *)&pEnt->u30,
        pEnt->mover.pos1,
        pEnt->mover.pos2,
        pEnt->mover.pos3);
    SV_LinkEntity(pEnt);
}

void __cdecl ScriptMover_SetupMove(
    trajectory_t *pTr,
    const float *vPos,
    float fTotalTime,
    float fAccelTime,
    float fDecelTime,
    float *vCurrPos,
    float *pfSpeed,
    float *pfMidTime,
    float *pfDecelTime,
    float *vPos1,
    float *vPos2,
    float *vPos3)
{
    float fDist; // [esp+58h] [ebp-20h]
    float fDelta; // [esp+5Ch] [ebp-1Ch]
    float fDeltaa; // [esp+5Ch] [ebp-1Ch]
    float vMaxSpeed[3]; // [esp+60h] [ebp-18h] BYREF
    float vMove[3]; // [esp+6Ch] [ebp-Ch] BYREF

    Vec3Sub(vPos, vCurrPos, vMove);
    if (pTr->trType)
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
    if (fAccelTime == 0.0 && fDecelTime == 0.0f)
    {
        pTr->trTime = level.time;
        pTr->trDuration = (int)(fTotalTime * 1000.0f);
        *pfMidTime = fTotalTime;
        *pfDecelTime = 0.0;
        *vPos3 = *vPos;
        vPos3[1] = vPos[1];
        vPos3[2] = vPos[2];
        pTr->trBase[0] = *vCurrPos;
        pTr->trBase[1] = vCurrPos[1];
        pTr->trBase[2] = vCurrPos[2];
        if (!pTr->trDuration)
            MyAssertHandler(".\\game\\g_scr_mover.cpp", 144, 0, "%s", "pTr->trDuration");
        fDelta = 1000.0 / (double)pTr->trDuration;
        Vec3Scale(vMove, fDelta, pTr->trDelta);
        if ((COERCE_UNSIGNED_INT(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\game\\g_scr_mover.cpp",
                147,
                0,
                "%s",
                "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])");
        }
        pTr->trType = TR_LINEAR_STOP;
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
    }
    else
    {
        *pfMidTime = fTotalTime - fAccelTime - fDecelTime;
        *pfDecelTime = fDecelTime;
        fDist = Vec3Length(vMove);
        if (fTotalTime * 2.0 - fAccelTime - fDecelTime == 0.0)
            MyAssertHandler(".\\game\\g_scr_mover.cpp", 159, 0, "%s", "(2.0f * fTotalTime) - fAccelTime - fDecelTime");
        *pfSpeed = fDist * 2.0 / (fTotalTime * 2.0 - fAccelTime - fDecelTime);
        Vec3NormalizeTo(vMove, vMaxSpeed);
        Vec3Scale(vMaxSpeed, *pfSpeed, vMaxSpeed);
        if (fAccelTime == 0.0)
        {
            *vPos1 = *vCurrPos;
            vPos1[1] = vCurrPos[1];
            vPos1[2] = vCurrPos[2];
            if (*pfMidTime == 0.0)
            {
                pTr->trTime = level.time;
                pTr->trDuration = (int)(*pfDecelTime * 1000.0f);
                pTr->trBase[0] = *vCurrPos;
                pTr->trBase[1] = vCurrPos[1];
                pTr->trBase[2] = vCurrPos[2];
                pTr->trDelta[0] = vMaxSpeed[0];
                pTr->trDelta[1] = vMaxSpeed[1];
                pTr->trDelta[2] = vMaxSpeed[2];
                if ((COERCE_UNSIGNED_INT(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
                {
                    MyAssertHandler(
                        ".\\game\\g_scr_mover.cpp",
                        202,
                        0,
                        "%s",
                        "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])");
                }
                pTr->trType = TR_DECELERATE;
            }
            else
            {
                pTr->trTime = level.time;
                pTr->trDuration = (int)(*pfMidTime * 1000.0f);
                pTr->trBase[0] = *vCurrPos;
                pTr->trBase[1] = vCurrPos[1];
                pTr->trBase[2] = vCurrPos[2];
                Vec3Scale(vMaxSpeed, *pfMidTime, vMove);
                if (!pTr->trDuration)
                    MyAssertHandler(".\\game\\g_scr_mover.cpp", 188, 0, "%s", "pTr->trDuration");
                fDeltaa = 1000.0f / (float)pTr->trDuration;
                Vec3Scale(vMove, fDeltaa, pTr->trDelta);
                if ((COERCE_UNSIGNED_INT(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
                {
                    MyAssertHandler(
                        ".\\game\\g_scr_mover.cpp",
                        191,
                        0,
                        "%s",
                        "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])");
                }
                pTr->trType = TR_LINEAR_STOP;
            }
        }
        else
        {
            pTr->trTime = level.time;
            pTr->trDuration = (int)(fAccelTime * 1000.0f);
            pTr->trBase[0] = *vCurrPos;
            pTr->trBase[1] = vCurrPos[1];
            pTr->trBase[2] = vCurrPos[2];
            pTr->trDelta[0] = vMaxSpeed[0];
            pTr->trDelta[1] = vMaxSpeed[1];
            pTr->trDelta[2] = vMaxSpeed[2];
            if ((COERCE_UNSIGNED_INT(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\game\\g_scr_mover.cpp",
                    171,
                    0,
                    "%s",
                    "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])");
            }
            pTr->trType = TR_ACCELERATE;
            BG_EvaluateTrajectory(pTr, pTr->trDuration + level.time, vPos1);
        }
        Vec3Mad(vPos1, *pfMidTime, vMaxSpeed, vPos2);
        *vPos3 = *vPos;
        vPos3[1] = vPos[1];
        vPos3[2] = vPos[2];
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
    }
}

void __cdecl ScriptEntCmd_GravityMove(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    float velocity[3]; // [esp+24h] [ebp-14h] BYREF
    gentity_s *pSelf; // [esp+30h] [ebp-8h]
    float fTotalTime; // [esp+34h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        pSelf = &g_entities[entref.entnum];
        if (pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light)
        {
            v1 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    Scr_GetVector(0, velocity);
    if ((LODWORD(velocity[0]) & 0x7F800000) == 0x7F800000
        || (LODWORD(velocity[1]) & 0x7F800000) == 0x7F800000
        || (LODWORD(velocity[2]) & 0x7F800000) == 0x7F800000)
    {
        v2 = va("invalid velocity parameter in movegravity command: %f %f %f", velocity[0], velocity[1], velocity[2]);
        Scr_Error(v2);
    }
    fTotalTime = Scr_GetFloat(1);
    ScriptMover_GravityMove(pSelf, velocity, fTotalTime);
}

void __cdecl ScriptMover_GravityMove(gentity_s *mover, float *velocity, float totalTime)
{
    trajectory_t *trajectory; // [esp+24h] [ebp-4h]

    if (!mover)
        MyAssertHandler(".\\game\\g_scr_mover.cpp", 356, 0, "%s", "mover");
    if ((COERCE_UNSIGNED_INT(*velocity) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(velocity[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(velocity[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game\\g_scr_mover.cpp",
            357,
            0,
            "%s",
            "!IS_NAN((velocity)[0]) && !IS_NAN((velocity)[1]) && !IS_NAN((velocity)[2])");
    }
    trajectory = &mover->s.lerp.pos;
    mover->s.lerp.pos.trTime = level.time;
    mover->s.lerp.pos.trDuration = (int)(totalTime * 1000.0f);
    mover->s.lerp.pos.trBase[0] = mover->r.currentOrigin[0];
    mover->s.lerp.pos.trBase[1] = mover->r.currentOrigin[1];
    mover->s.lerp.pos.trBase[2] = mover->r.currentOrigin[2];
    mover->s.lerp.pos.trDelta[0] = *velocity;
    mover->s.lerp.pos.trDelta[1] = velocity[1];
    mover->s.lerp.pos.trDelta[2] = velocity[2];
    if ((COERCE_UNSIGNED_INT(mover->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mover->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mover->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\game\\g_scr_mover.cpp",
            367,
            0,
            "%s",
            "!IS_NAN((trajectory->trDelta)[0]) && !IS_NAN((trajectory->trDelta)[1]) && !IS_NAN((trajectory->trDelta)[2])");
    }
    trajectory->trType = TR_GRAVITY;
    BG_EvaluateTrajectory(trajectory, level.time, mover->r.currentOrigin);
    SV_LinkEntity(mover);
}

void __cdecl ScriptEnt_MoveAxis(scr_entref_t entref, int iAxis)
{
    const char *v2; // eax
    gentity_s *pSelf; // [esp+10h] [ebp-20h]
    float fTotalTime; // [esp+14h] [ebp-1Ch] BYREF
    float fAccelTime; // [esp+18h] [ebp-18h] BYREF
    float fDecelTime; // [esp+1Ch] [ebp-14h] BYREF
    float vPos[3]; // [esp+20h] [ebp-10h] BYREF
    float fMove; // [esp+2Ch] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        pSelf = &g_entities[entref.entnum];
        if (pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light)
        {
            v2 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v2);
        }
    }
    fMove = Scr_GetFloat(0);
    ScriptEntCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
    vPos[0] = pSelf->r.currentOrigin[0];
    vPos[1] = pSelf->r.currentOrigin[1];
    vPos[2] = pSelf->r.currentOrigin[2];
    vPos[iAxis] = vPos[iAxis] + fMove;
    ScriptMover_Move(pSelf, vPos, fTotalTime, fAccelTime, fDecelTime);
}

void __cdecl ScriptEntCmd_MoveX(scr_entref_t entref)
{
    ScriptEnt_MoveAxis(entref, 0);
}

void __cdecl ScriptEntCmd_MoveY(scr_entref_t entref)
{
    ScriptEnt_MoveAxis(entref, 1);
}

void __cdecl ScriptEntCmd_MoveZ(scr_entref_t entref)
{
    ScriptEnt_MoveAxis(entref, 2);
}

void __cdecl ScriptEntCmd_RotateTo(scr_entref_t entref)
{
    const char *v1; // eax
    double v2; // st7
    gentity_s *pSelf; // [esp+10h] [ebp-2Ch]
    float fTotalTime; // [esp+14h] [ebp-28h] BYREF
    float fAccelTime; // [esp+18h] [ebp-24h] BYREF
    float vDest[3]; // [esp+1Ch] [ebp-20h] BYREF
    float fDecelTime; // [esp+28h] [ebp-14h] BYREF
    float vRot[3]; // [esp+2Ch] [ebp-10h] BYREF
    int i; // [esp+38h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        pSelf = &g_entities[entref.entnum];
        if (pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light)
        {
            v1 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    Scr_GetVector(0, vDest);
    ScriptEntCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
    for (i = 0; i < 3; ++i)
    {
        v2 = AngleDelta(vDest[i], pSelf->r.currentAngles[i]);
        vRot[i] = v2 + pSelf->r.currentAngles[i];
    }
    ScriptMover_Rotate(pSelf, vRot, fTotalTime, fAccelTime, fDecelTime);
}

void __cdecl ScriptMover_Rotate(
    gentity_s *pEnt,
    const float *vRot,
    float fTotalTime,
    float fAccelTime,
    float fDecelTime)
{
    float angles[3]; // [esp+2Ch] [ebp-Ch] BYREF

    angles[0] = pEnt->r.currentAngles[0];
    angles[1] = pEnt->r.currentAngles[1];
    angles[2] = pEnt->r.currentAngles[2];
    ScriptMover_SetupMove(
        &pEnt->s.lerp.apos,
        vRot,
        fTotalTime,
        fAccelTime,
        fDecelTime,
        angles,
        &pEnt->mover.aSpeed,
        &pEnt->mover.aMidTime,
        &pEnt->mover.aDecelTime,
        pEnt->mover.apos1,
        pEnt->mover.apos2,
        pEnt->mover.apos3);
    SV_LinkEntity(pEnt);
}

void __cdecl ScriptEntCmd_DevAddPitch(scr_entref_t entref)
{
    ScriptEnt_DevAddRotate(entref, 1u);
}

void __cdecl ScriptEnt_DevAddRotate(scr_entref_t entref, unsigned int iAxis)
{
    const char *v2; // eax
    float v3; // [esp+14h] [ebp-60h]
    int i; // [esp+18h] [ebp-5Ch]
    gentity_s *pSelf; // [esp+1Ch] [ebp-58h]
    float axisOut[3][3]; // [esp+20h] [ebp-54h] BYREF
    float fDelta; // [esp+44h] [ebp-30h]
    float axisIn[3][3]; // [esp+48h] [ebp-2Ch] BYREF
    float fCos; // [esp+6Ch] [ebp-8h]
    float fSin; // [esp+70h] [ebp-4h]

    if (iAxis > 2)
        MyAssertHandler(".\\game\\g_scr_mover.cpp", 733, 0, "%s", "iAxis >= 0 && iAxis < 3");
    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        pSelf = &g_entities[entref.entnum];
        if (pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light)
        {
            v2 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v2);
        }
    }
    if (Scr_GetNumParam() == 1)
    {
        fDelta = Scr_GetFloat(0);
        v3 = fDelta * 0.01745329238474369;
        fCos = cos(v3);
        fSin = sin(v3);
        AnglesToAxis(pSelf->r.currentAngles, axisIn);
        for (i = 0; i < 3; ++i)
        {
            axisOut[iAxis][i] = axisIn[iAxis][i];
            axisOut[(int)(iAxis + 1) % 3][i] = axisIn[(int)(iAxis + 1) % 3][i] * fCos + axisIn[(int)(iAxis + 2) % 3][i] * fSin;
            axisOut[(int)(iAxis + 2) % 3][i] = axisIn[(int)(iAxis + 2) % 3][i] * fCos - axisIn[(int)(iAxis + 1) % 3][i] * fSin;
        }
        AxisToAngles(axisOut, pSelf->r.currentAngles);
        pSelf->s.lerp.apos.trType = TR_STATIONARY;
        pSelf->s.lerp.apos.trBase[0] = pSelf->r.currentAngles[0];
        pSelf->s.lerp.apos.trBase[1] = pSelf->r.currentAngles[1];
        pSelf->s.lerp.apos.trBase[2] = pSelf->r.currentAngles[2];
        SV_LinkEntity(pSelf);
    }
    else
    {
        Scr_Error("ScriptEnt_ProtoAddRotate: expect exactly one parameter.");
    }
}

void __cdecl ScriptEntCmd_DevAddYaw(scr_entref_t entref)
{
    ScriptEnt_DevAddRotate(entref, 2u);
}

void __cdecl ScriptEntCmd_DevAddRoll(scr_entref_t entref)
{
    ScriptEnt_DevAddRotate(entref, 0);
}

void __cdecl ScriptEnt_RotateAxis(scr_entref_t entref, int iAxis)
{
    const char *v2; // eax
    gentity_s *pSelf; // [esp+10h] [ebp-20h]
    float fTotalTime; // [esp+14h] [ebp-1Ch] BYREF
    float fAccelTime; // [esp+18h] [ebp-18h] BYREF
    float fDecelTime; // [esp+1Ch] [ebp-14h] BYREF
    float vRot[3]; // [esp+20h] [ebp-10h] BYREF
    float fMove; // [esp+2Ch] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        pSelf = &g_entities[entref.entnum];
        if (pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light)
        {
            v2 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v2);
        }
    }
    fMove = Scr_GetFloat(0);
    ScriptEntCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
    vRot[0] = pSelf->r.currentAngles[0];
    vRot[1] = pSelf->r.currentAngles[1];
    vRot[2] = pSelf->r.currentAngles[2];
    vRot[iAxis] = vRot[iAxis] + fMove;
    ScriptMover_Rotate(pSelf, vRot, fTotalTime, fAccelTime, fDecelTime);
}

void __cdecl ScriptEntCmd_RotatePitch(scr_entref_t entref)
{
    ScriptEnt_RotateAxis(entref, 0);
}

void __cdecl ScriptEntCmd_RotateYaw(scr_entref_t entref)
{
    ScriptEnt_RotateAxis(entref, 1);
}

void __cdecl ScriptEntCmd_RotateRoll(scr_entref_t entref)
{
    ScriptEnt_RotateAxis(entref, 2);
}

void __cdecl ScriptEntCmd_Vibrate(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+28h] [ebp-5Ch]
    float impulseVector[3]; // [esp+2Ch] [ebp-58h] BYREF
    float amplitude; // [esp+38h] [ebp-4Ch]
    float scaledImpulseVector[3]; // [esp+3Ch] [ebp-48h] BYREF
    const char *error; // [esp+48h] [ebp-3Ch]
    float time; // [esp+4Ch] [ebp-38h]
    float vibrationAngles[3]; // [esp+50h] [ebp-34h]
    float period; // [esp+5Ch] [ebp-28h]
    float axis[3][3]; // [esp+60h] [ebp-24h] BYREF

    error = "illegal call to vibrate()\n";
    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        pSelf = &g_entities[entref.entnum];
        if (pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light)
        {
            v1 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (Scr_GetNumParam() == 4)
    {
        Scr_GetVector(0, impulseVector);
        amplitude = Scr_GetFloat(1);
        period = Scr_GetFloat(2);
        time = Scr_GetFloat(3);
        Vec3Normalize(impulseVector);
        Vec3Scale(impulseVector, amplitude, scaledImpulseVector);
        AnglesToAxis(pSelf->r.currentAngles, axis);
        vibrationAngles[0] = Vec3Dot(axis[0], scaledImpulseVector);
        vibrationAngles[2] = -Vec3Dot(axis[1], scaledImpulseVector);
        vibrationAngles[1] = 0.0;
        pSelf->mover.apos3[0] = pSelf->r.currentAngles[0];
        pSelf->mover.apos3[1] = pSelf->r.currentAngles[1];
        pSelf->mover.apos3[2] = pSelf->r.currentAngles[2];
        pSelf->s.lerp.apos.trDuration = (int)(period * 1000.0);
        pSelf->s.lerp.apos.trTime = level.time + (int)(time * 1000.0);
        pSelf->s.lerp.apos.trBase[0] = pSelf->r.currentAngles[0];
        pSelf->s.lerp.apos.trBase[1] = pSelf->r.currentAngles[1];
        pSelf->s.lerp.apos.trBase[2] = pSelf->r.currentAngles[2];
        pSelf->s.lerp.apos.trDelta[0] = vibrationAngles[0];
        pSelf->s.lerp.apos.trDelta[1] = vibrationAngles[1];
        pSelf->s.lerp.apos.trDelta[2] = vibrationAngles[2];
        pSelf->s.lerp.apos.trType = TR_SINE;
    }
    else
    {
        Scr_Error(error);
    }
}

void __cdecl ScriptEntCmd_RotateVelocity(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+Ch] [ebp-1Ch]
    float fTotalTime; // [esp+10h] [ebp-18h] BYREF
    float fAccelTime; // [esp+14h] [ebp-14h] BYREF
    float fDecelTime; // [esp+18h] [ebp-10h] BYREF
    float vSpeed[3]; // [esp+1Ch] [ebp-Ch] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        pSelf = &g_entities[entref.entnum];
        if (pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light)
        {
            v1 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    Scr_GetVector(0, vSpeed);
    ScriptEntCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
    ScriptMover_RotateSpeed(pSelf, vSpeed, fTotalTime, fAccelTime, fDecelTime);
}

void __cdecl ScriptMover_RotateSpeed(
    gentity_s *pEnt,
    const float *vRotSpeed,
    float fTotalTime,
    float fAccelTime,
    float fDecelTime)
{
    ScriptMover_SetupMoveSpeed(
        &pEnt->s.lerp.apos,
        vRotSpeed,
        fTotalTime,
        fAccelTime,
        fDecelTime,
        pEnt->r.currentAngles,
        &pEnt->mover.aSpeed,
        &pEnt->mover.aMidTime,
        &pEnt->mover.aDecelTime,
        pEnt->mover.apos1,
        pEnt->mover.apos2,
        pEnt->mover.apos3);
    SV_LinkEntity(pEnt);
}

void __cdecl ScriptMover_SetupMoveSpeed(
    trajectory_t *pTr,
    const float *vSpeed,
    float fTotalTime,
    float fAccelTime,
    float fDecelTime,
    float *vCurrPos,
    float *pfSpeed,
    float *pfMidTime,
    float *pfDecelTime,
    float *vPos1,
    float *vPos2,
    float *vPos3)
{
    trajectory_t tr; // [esp+6Ch] [ebp-24h] BYREF

    if (pTr->trType)
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
    if (fAccelTime == 0.0f && fDecelTime == 0.0f)
    {
        pTr->trTime = level.time;
        pTr->trDuration = (int)(fTotalTime * 1000.0f);
        *pfMidTime = fTotalTime;
        *pfDecelTime = 0.0f;
        pTr->trBase[0] = *vCurrPos;
        pTr->trBase[1] = vCurrPos[1];
        pTr->trBase[2] = vCurrPos[2];
        pTr->trDelta[0] = *vSpeed;
        pTr->trDelta[1] = vSpeed[1];
        pTr->trDelta[2] = vSpeed[2];
        if ((COERCE_UNSIGNED_INT(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
            || (COERCE_UNSIGNED_INT(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
        {
            MyAssertHandler(
                ".\\game\\g_scr_mover.cpp",
                260,
                0,
                "%s",
                "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])");
        }
        pTr->trType = TR_LINEAR_STOP;
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
        BG_EvaluateTrajectory(pTr, pTr->trDuration + level.time, vPos3);
    }
    else
    {
        *pfMidTime = fTotalTime - fAccelTime - fDecelTime;
        *pfDecelTime = fDecelTime;
        *pfSpeed = Vec3Length(vSpeed);
        if (fAccelTime == 0.0f)
        {
            *vPos1 = *vCurrPos;
            vPos1[1] = vCurrPos[1];
            vPos1[2] = vCurrPos[2];
            if (*pfMidTime == 0.0f)
            {
                pTr->trTime = level.time;
                pTr->trDuration = (int)(*pfDecelTime * 1000.0f);
                pTr->trBase[0] = *vCurrPos;
                pTr->trBase[1] = vCurrPos[1];
                pTr->trBase[2] = vCurrPos[2];
                pTr->trDelta[0] = *vSpeed;
                pTr->trDelta[1] = vSpeed[1];
                pTr->trDelta[2] = vSpeed[2];
                if ((COERCE_UNSIGNED_INT(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
                {
                    MyAssertHandler(
                        ".\\game\\g_scr_mover.cpp",
                        309,
                        0,
                        "%s",
                        "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])");
                }
                pTr->trType = TR_DECELERATE;
            }
            else
            {
                pTr->trTime = level.time;
                pTr->trDuration = (int)(*pfMidTime * 1000.0f);
                pTr->trBase[0] = *vCurrPos;
                pTr->trBase[1] = vCurrPos[1];
                pTr->trBase[2] = vCurrPos[2];
                pTr->trDelta[0] = *vSpeed;
                pTr->trDelta[1] = vSpeed[1];
                pTr->trDelta[2] = vSpeed[2];
                if ((COERCE_UNSIGNED_INT(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
                {
                    MyAssertHandler(
                        ".\\game\\g_scr_mover.cpp",
                        299,
                        0,
                        "%s",
                        "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])");
                }
                pTr->trType = TR_LINEAR_STOP;
            }
        }
        else
        {
            pTr->trTime = level.time;
            pTr->trDuration = (int)(fAccelTime * 1000.0f);
            pTr->trBase[0] = *vCurrPos;
            pTr->trBase[1] = vCurrPos[1];
            pTr->trBase[2] = vCurrPos[2];
            pTr->trDelta[0] = *vSpeed;
            pTr->trDelta[1] = vSpeed[1];
            pTr->trDelta[2] = vSpeed[2];
            if ((COERCE_UNSIGNED_INT(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                || (COERCE_UNSIGNED_INT(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\game\\g_scr_mover.cpp",
                    282,
                    0,
                    "%s",
                    "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])");
            }
            pTr->trType = TR_ACCELERATE;
            BG_EvaluateTrajectory(pTr, pTr->trDuration + level.time, vPos1);
        }
        Vec3Mad(vPos1, *pfMidTime, vSpeed, vPos2);
        if (*pfDecelTime == 0.0f)
        {
            *vPos3 = *vPos2;
            vPos3[1] = vPos2[1];
            vPos3[2] = vPos2[2];
        }
        else
        {
            tr.trType = TR_DECELERATE;
            tr.trTime = level.time;
            tr.trDuration = (int)(*pfDecelTime * 1000.0f);
            tr.trBase[0] = *vPos2;
            tr.trBase[1] = vPos2[1];
            tr.trBase[2] = vPos2[2];
            tr.trDelta[0] = *vSpeed;
            tr.trDelta[1] = vSpeed[1];
            tr.trDelta[2] = vSpeed[2];
            if ((LODWORD(tr.trDelta[0]) & 0x7F800000) == 0x7F800000
                || (LODWORD(tr.trDelta[1]) & 0x7F800000) == 0x7F800000
                || (LODWORD(tr.trDelta[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\game\\g_scr_mover.cpp",
                    327,
                    0,
                    "%s",
                    "!IS_NAN((tr.trDelta)[0]) && !IS_NAN((tr.trDelta)[1]) && !IS_NAN((tr.trDelta)[2])");
            }
            BG_EvaluateTrajectory(&tr, tr.trDuration + level.time, vPos3);
        }
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
    }
}

void __cdecl ScriptEntCmd_SetCanDamage(scr_entref_t entref)
{
    gentity_s *pSelf; // [esp+0h] [ebp-Ch]

    if (Scr_GetNumParam() == 1)
    {
        if (entref.classnum)
        {
            Scr_ObjectError("not an entity");
            pSelf = 0;
        }
        else
        {
            iassert(entref.entnum < MAX_GENTITIES);
            pSelf = &g_entities[entref.entnum];
        }
        pSelf->takedamage = Scr_GetInt(0);
    }
    else
    {
        Scr_Error("illegal call to setcandamage()\n");
    }
}

void __cdecl ScriptEntCmd_PhysicsLaunch(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-1Ch]
    float contact_point[3]; // [esp+4h] [ebp-18h] BYREF
    float initial_force[3]; // [esp+10h] [ebp-Ch] BYREF

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        pSelf = &g_entities[entref.entnum];
        if (pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light)
        {
            v1 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (Scr_GetNumParam() == 2)
    {
        Scr_GetVector(0, contact_point);
        Scr_GetVector(1u, initial_force);
    }
    else
    {
        contact_point[0] = 0.0f;
        contact_point[1] = 0.0f;
        contact_point[2] = 0.0f;
        initial_force[0] = 0.0f;
        initial_force[1] = 0.0f;
        initial_force[2] = 0.0f;
    }
    ScriptMover_SetupPhysicsLaunch(&pSelf->s.lerp.pos, &pSelf->s.lerp.apos, contact_point, initial_force);
    pSelf->r.contents = 0;
    pSelf->takedamage = 0;
}

void __cdecl ScriptMover_SetupPhysicsLaunch(
    trajectory_t *pTr,
    trajectory_t *paTr,
    const float *contact_point,
    const float *initial_force)
{
    float currPos[3]; // [esp+10h] [ebp-18h] BYREF
    float currApos[3]; // [esp+1Ch] [ebp-Ch] BYREF

    if (pTr->trType == TR_PHYSICS)
        Scr_Error("physicslaunch called more than once for the same entity.");
    pTr->trTime = level.time;
    pTr->trDuration = 0x7FFFFFFF;
    BG_EvaluateTrajectory(pTr, level.time, currPos);
    BG_EvaluateTrajectory(paTr, level.time, currApos);
    pTr->trBase[0] = currPos[0];
    pTr->trBase[1] = currPos[1];
    pTr->trBase[2] = currPos[2];
    paTr->trBase[0] = currApos[0];
    paTr->trBase[1] = currApos[1];
    paTr->trBase[2] = currApos[2];
    pTr->trDelta[0] = *contact_point;
    pTr->trDelta[1] = contact_point[1];
    pTr->trDelta[2] = contact_point[2];
    paTr->trDelta[0] = *initial_force;
    paTr->trDelta[1] = initial_force[1];
    paTr->trDelta[2] = initial_force[2];
    pTr->trType = TR_PHYSICS;
    paTr->trType = TR_PHYSICS;
}

void __cdecl ScriptEntCmd_Solid(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        pSelf = &g_entities[entref.entnum];
        if (pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light)
        {
            v1 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (pSelf->classname == scr_const.script_origin)
    {
        Com_DPrintf(23, "cannot use the solid/notsolid commands on a script_origin entity( number %i )\n", pSelf->s.number);
    }
    else
    {
        if (pSelf->classname == scr_const.script_model)
        {
            pSelf->r.contents = 8320;
        }
        else
        {
            pSelf->r.contents = 1;
            pSelf->s.lerp.eFlags &= ~1u;
        }
        SV_LinkEntity(pSelf);
    }
}

void __cdecl ScriptEntCmd_NotSolid(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *pSelf; // [esp+0h] [ebp-4h]

    if (entref.classnum)
    {
        Scr_ObjectError("not an entity");
        pSelf = 0;
    }
    else
    {
        iassert(entref.entnum < MAX_GENTITIES);
        pSelf = &g_entities[entref.entnum];
        if (pSelf->classname != scr_const.script_brushmodel
            && pSelf->classname != scr_const.script_model
            && pSelf->classname != scr_const.script_origin
            && pSelf->classname != scr_const.light)
        {
            v1 = va("entity %i is not a script_brushmodel, script_model, script_origin, or light", entref.entnum);
            Scr_ObjectError(v1);
        }
    }
    if (pSelf->classname == scr_const.script_origin)
    {
        Com_DPrintf(23, "cannot use the solid/notsolid commands on a script_origin entity( number %i )\n", pSelf->s.number);
    }
    else
    {
        pSelf->r.contents = 0;
        if (pSelf->classname != scr_const.script_model)
            pSelf->s.lerp.eFlags |= 1u;
        SV_LinkEntity(pSelf);
    }
}

const BuiltinMethodDef methods_1[18] =
{
  { "moveto", &ScriptEntCmd_MoveTo, 0 },
  { "movex", &ScriptEntCmd_MoveX, 0 },
  { "movey", &ScriptEntCmd_MoveY, 0 },
  { "movez", &ScriptEntCmd_MoveZ, 0 },
  { "movegravity", &ScriptEntCmd_GravityMove, 0 },
  { "rotateto", &ScriptEntCmd_RotateTo, 0 },
  { "rotatepitch", &ScriptEntCmd_RotatePitch, 0 },
  { "rotateyaw", &ScriptEntCmd_RotateYaw, 0 },
  { "rotateroll", &ScriptEntCmd_RotateRoll, 0 },
  { "devaddpitch", &ScriptEntCmd_DevAddPitch, 1 },
  { "devaddyaw", &ScriptEntCmd_DevAddYaw, 1 },
  { "devaddroll", &ScriptEntCmd_DevAddRoll, 1 },
  { "vibrate", &ScriptEntCmd_Vibrate, 0 },
  { "rotatevelocity", &ScriptEntCmd_RotateVelocity, 0 },
  { "solid", &ScriptEntCmd_Solid, 0 },
  { "notsolid", &ScriptEntCmd_NotSolid, 0 },
  { "setcandamage", &ScriptEntCmd_SetCanDamage, 0 },
  { "physicslaunch", &ScriptEntCmd_PhysicsLaunch, 0 }
}; // idb

void(__cdecl *__cdecl ScriptEnt_GetMethod(const char **pName))(scr_entref_t)
{
    unsigned int i; // [esp+18h] [ebp-4h]

    for (i = 0; i < 0x12; ++i)
    {
        if (!strcmp(*pName, methods_1[i].actionString))
        {
            *pName = methods_1[i].actionString;
            return methods_1[i].actionFunc;
        }
    }
    return 0;
}

