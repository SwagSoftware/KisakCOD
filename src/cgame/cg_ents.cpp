#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_ents.h"
#include <qcommon/com_bsp.h>
#include <gfx_d3d/r_scene.h>
#include "cg_actors.h"
#include <xanim/dobj_utils.h>
#include <EffectsCore/fx_system.h>
#include <win32/win_local.h>
#include <physics/phys_local.h>
#include <game/savememory.h>
#include <ragdoll/ragdoll.h>
#include <gfx_d3d/r_workercmds_common.h>
#include "cg_main.h"
#include <script/scr_const.h>
#include "cg_local.h"
#include <gfx_d3d/r_model.h>
#include "cg_pose.h"

void __cdecl LocalConvertQuatToMat(const DObjAnimMat *mat, float (*axis)[3])
{
    double v4; // fp5
    double v5; // fp12
    double v6; // fp4
    double v7; // fp13
    double v8; // fp0
    double v9; // fp11
    double v10; // fp10
    double v11; // fp8
    double v12; // fp9

    if ((COERCE_UNSIGNED_INT(mat->quat[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mat->quat[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mat->quat[2]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(mat->quat[3]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../xanim/xanim_public.h",
            434,
            0,
            "%s",
            "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
    }
    if ((COERCE_UNSIGNED_INT(mat->transWeight) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../xanim/xanim_public.h",
            435,
            0,
            "%s",
            "!IS_NAN(mat->transWeight)");
    v4 = (float)(mat->quat[2] * (float)(mat->quat[0] * mat->transWeight));
    v5 = (float)(mat->quat[1] * (float)(mat->quat[0] * mat->transWeight));
    v6 = (float)(mat->quat[2] * (float)(mat->quat[1] * mat->transWeight));
    v7 = (float)(mat->quat[3] * (float)(mat->quat[0] * mat->transWeight));
    v8 = (float)(mat->quat[3] * (float)(mat->quat[1] * mat->transWeight));
    v9 = (float)(mat->quat[3] * (float)(mat->quat[2] * mat->transWeight));
    v10 = (float)((float)(mat->quat[2] * (float)(mat->quat[2] * mat->transWeight))
        + (float)(mat->quat[1] * (float)(mat->quat[1] * mat->transWeight)));
    v11 = (float)((float)(mat->quat[2] * (float)(mat->quat[2] * mat->transWeight))
        + (float)(mat->quat[0] * (float)(mat->quat[0] * mat->transWeight)));
    v12 = (float)((float)(mat->quat[1] * (float)(mat->quat[1] * mat->transWeight))
        + (float)(mat->quat[0] * (float)(mat->quat[0] * mat->transWeight)));
    (*axis)[5] = (float)(mat->quat[2] * (float)(mat->quat[1] * mat->transWeight))
        + (float)(mat->quat[3] * (float)(mat->quat[0] * mat->transWeight));
    (*axis)[2] = (float)v4 - (float)v8;
    (*axis)[6] = (float)v8 + (float)v4;
    (*axis)[3] = (float)v5 - (float)v9;
    (*axis)[0] = (float)1.0 - (float)v10;
    (*axis)[1] = (float)v9 + (float)v5;
    (*axis)[4] = (float)1.0 - (float)v11;
    (*axis)[7] = (float)v6 - (float)v7;
    (*axis)[8] = (float)1.0 - (float)v12;
}

unsigned int __cdecl Com_GetPrimaryLightCount()
{
    if (!comWorld.isInUse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/com_bsp_api.h", 23, 0, "%s", "comWorld.isInUse");
    return comWorld.primaryLightCount;
}

const ComPrimaryLight *__cdecl Com_GetPrimaryLight(unsigned int primaryLightIndex)
{
    if (!comWorld.isInUse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/com_bsp_api.h", 31, 0, "%s", "comWorld.isInUse");
    if (primaryLightIndex >= comWorld.primaryLightCount)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/com_bsp_api.h",
            32,
            0,
            "primaryLightIndex doesn't index comWorld.primaryLightCount\n\t%i not in [0, %i)",
            primaryLightIndex,
            comWorld.primaryLightCount);
    return &comWorld.primaryLights[primaryLightIndex];
}

void __cdecl CG_LockLightingOrigin(centity_s *cent, float *lightingOrigin)
{
    if ((cent->nextState.lerp.eFlags & 0x400) != 0)
    {
        if (cent->lightingOrigin[0] == 0.0 && cent->lightingOrigin[1] == 0.0 && cent->lightingOrigin[2] == 0.0)
        {
            cent->lightingOrigin[0] = cent->pose.origin[0];
            cent->lightingOrigin[1] = cent->pose.origin[1];
            cent->lightingOrigin[2] = cent->pose.origin[2];
        }
        *lightingOrigin = cent->lightingOrigin[0];
        lightingOrigin[1] = cent->lightingOrigin[1];
        lightingOrigin[2] = cent->lightingOrigin[2];
    }
    cent->lightingOrigin[0] = 0.0;
    cent->lightingOrigin[1] = 0.0;
    cent->lightingOrigin[2] = 0.0;
    *lightingOrigin = cent->pose.origin[0];
    lightingOrigin[1] = cent->pose.origin[1];
    lightingOrigin[2] = cent->pose.origin[2] + (float)4.0;
}

int __cdecl CG_GetRenderFlagForRefEntity(__int16 eFlags)
{
    unsigned int v1; // r11
    int v2; // r10
    int result; // r3
    bool v4; // zf

    v1 = eFlags & 0xE000;
    v2 = 0;
    if (v1 > 0x6000)
    {
        if (v1 == 0x8000)
        {
            v2 = 2048;
        }
        else if (v1 == 40960)
        {
            v2 = 256;
        }
    }
    else if ((eFlags & 0xE000) == 0x6000)
    {
        v2 = 1536;
    }
    else if (v1 == 0x2000)
    {
        v2 = 512;
    }
    else if (v1 == 0x4000)
    {
        v2 = 1024;
    }
    v4 = (eFlags & 0x80) == 0;
    result = v2 | 8;
    if (v4)
        return v2;
    return result;
}

void __cdecl CG_General(int localClientNum, centity_s *cent)
{
    entityState_s *p_nextState; // r31
    unsigned int RenderFlagForRefEntity; // r3
    const DObj_s *v5; // r9
    float *v6; // r7
    double v7; // fp1
    float v8[6]; // [sp+50h] [-30h] BYREF

    p_nextState = &cent->nextState;
    if ((cent->nextState.lerp.eFlags & 0x20) == 0)
    {
        if (Com_GetClientDObj(cent->nextState.number, 0))
        {
            CG_LockLightingOrigin(cent, v8);
            RenderFlagForRefEntity = CG_GetRenderFlagForRefEntity(p_nextState->lerp.eFlags);
            R_AddDObjToScene(v5, &cent->pose, p_nextState->number, RenderFlagForRefEntity, v6, v7);
        }
    }
}

void __cdecl CG_Item(centity_s *cent)
{
    entityState_s *p_nextState; // r31
    int v3; // r10
    unsigned __int8 v4; // r28
    unsigned int v5; // r29
    WeaponDef *WeaponDef; // r30
    unsigned int RenderFlagForRefEntity; // r3
    const DObj_s *v8; // r9
    float *v9; // r7
    double v10; // fp1

    p_nextState = &cent->nextState;
    if (*(unsigned __int16 *)cent->nextState.index >= 0x800u)
        Com_Error(ERR_DROP, "Bad item index %i on entity", cent->nextState.index);
    if ((p_nextState->lerp.eFlags & 0x20) == 0)
    {
        v3 = *(unsigned __int16 *)p_nextState->index;
        v4 = (unsigned int)v3 >> 7;
        v5 = v3 - (v3 >> 7 << 7);
        WeaponDef = BG_GetWeaponDef(v5);
        if (!WeaponDef)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 166, 0, "%s", "weapDef");
        if (!WeaponDef->worldModel[v4])
            Com_Error(ERR_DROP, "Bad item/weap index"); //KISAKTODO: "XModel loaded for item index %i weap index %i model %i (%s)", *(unsigned __int16 *)p_nextState->index, v5);
        if (Com_GetClientDObj(p_nextState->number, 0))
        {
            RenderFlagForRefEntity = CG_GetRenderFlagForRefEntity(p_nextState->lerp.eFlags);
            R_AddDObjToScene(v8, &cent->pose, p_nextState->number, RenderFlagForRefEntity, v9, v10);
        }
    }
}

void __cdecl CG_AddEntityLoopSound(int localClientNum, const centity_s *cent)
{
    float *BrushModel; // r3
    double v5; // fp10
    unsigned int v6; // r4
    double v7; // fp9
    double v8; // fp12
    double v9; // fp0
    const char *ConfigString; // r3
    const float *origin; // r5
    float v12[6]; // [sp+50h] [-30h] BYREF

    if (cent->nextState.solid == 0xFFFFFF)
    {
        BrushModel = (float *)R_GetBrushModel(*(unsigned __int16 *)cent->nextState.index);
        v5 = cent->pose.origin[1];
        v6 = cent->nextState.loopSound + 1667;
        v7 = cent->pose.origin[2];
        v8 = (float)((float)(BrushModel[7] + BrushModel[10]) * (float)0.5);
        v9 = (float)((float)(BrushModel[8] + BrushModel[11]) * (float)0.5);
        v12[0] = cent->pose.origin[0] + (float)((float)(BrushModel[9] + BrushModel[6]) * (float)0.5);
        v12[1] = (float)v5 + (float)v8;
        v12[2] = (float)v7 + (float)v9;
        ConfigString = CL_GetConfigString(localClientNum, v6);
        origin = v12;
    }
    else
    {
        ConfigString = CL_GetConfigString(localClientNum, cent->nextState.loopSound + 1667);
        origin = cent->pose.origin;
    }
    CG_PlaySoundAliasByName(localClientNum, cent->nextState.number, origin, ConfigString);
}

void __cdecl CG_EntityEffects(int localClientNum, centity_s *cent)
{
    if (cent->nextState.loopSound)
        CG_AddEntityLoopSound(localClientNum, cent);
}

void __cdecl CG_mg42_PreControllers(int localClientNum, const DObj_s *obj, centity_s *cent)
{
    bool v5; // r11

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((cgArray[0].predictedPlayerState.eFlags & 0x300) == 0
        || (v5 = 1, cgArray[0].predictedPlayerState.viewlocked_entNum != cent->nextState.number))
    {
        v5 = 0;
    }
    cent->pose.turret.playerUsing = v5;
    if (v5)
    {
        cent->pose.cullIn = 0;
        cent->pose.actor.proneType = (int)cgArray[0].refdefViewAngles;
    }
    else
    {
        cent->pose.turret.angles.pitch = LerpAngle(
            cent->currentState.u.turret.gunAngles[0],
            cent->nextState.lerp.u.turret.gunAngles[0],
            cgArray[0].frameInterpolation);
        cent->pose.actor.pitch = LerpAngle(
            cent->currentState.u.turret.gunAngles[1],
            cent->nextState.lerp.u.turret.gunAngles[1],
            cgArray[0].frameInterpolation);
        cent->pose.actor.roll = LerpAngle(
            cent->currentState.u.turret.gunAngles[2],
            cent->nextState.lerp.u.turret.gunAngles[2],
            cgArray[0].frameInterpolation);
    }
    DObjGetBoneIndex(obj, scr_const.tag_aim, &cent->pose.turret.tag_aim);
    DObjGetBoneIndex(obj, scr_const.tag_aim_animated, &cent->pose.turret.tag_aim_animated);
    DObjGetBoneIndex(obj, scr_const.tag_flash, &cent->pose.turret.tag_flash);
}

void __cdecl CG_mg42(int localClientNum, centity_s *cent)
{
    entityState_s *p_nextState; // r30
    const DObj_s *ClientDObj; // r3
    const DObj_s *v6; // r28
    int RenderFlagForRefEntity; // r3
    float *v8; // r7
    double v9; // fp1

    p_nextState = &cent->nextState;
    if ((cent->nextState.lerp.eFlags & 0x20) == 0)
    {
        ClientDObj = Com_GetClientDObj(cent->nextState.number, 0);
        v6 = ClientDObj;
        if (ClientDObj)
        {
            CG_mg42_PreControllers(localClientNum, ClientDObj, cent);
            if (!CG_PlayerUsingScopedTurret(localClientNum)
                || CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState.viewlocked_entNum != p_nextState->number)
            {
                RenderFlagForRefEntity = CG_GetRenderFlagForRefEntity(p_nextState->lerp.eFlags);
                R_AddDObjToScene(v6, &cent->pose, p_nextState->number, RenderFlagForRefEntity | 4, v8, v9);
            }
        }
    }
}

bool __cdecl JavelinSoftLaunch(WeaponDef *weapDef, cg_s *cgameGlob, entityState_s *s1)
{
    return weapDef->guidedMissileType == MISSILE_GUIDANCE_JAVELIN
        && cgameGlob->time - s1->lerp.u.missile.launchTime <= weapDef->projIgnitionDelay;
}

void __cdecl CG_Missile(int localClientNum, centity_s *cent)
{
    WeaponDef *WeaponDef; // r27
    snd_alias_list_t *projectileSound; // r6
    const DObj_s *ClientDObj; // r25
    const FxEffectDef *projTrailEffect; // r4
    const FxEffectDef *projIgnitionEffect; // r4
    snd_alias_list_t *projIgnitionSound; // r6
    unsigned int RenderFlagForRefEntity; // r3
    float *v12; // r7
    double v13; // fp1

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((cent->nextState.lerp.eFlags & 0x20) == 0
        && cent->nextState.lerp.u.missile.launchTime <= CG_GetLocalClientTime(localClientNum))
    {
        if (cent->nextState.weapon >= BG_GetNumWeapons())
            cent->nextState.weapon = 0;
        CG_GetLocalClientWeaponInfo(localClientNum, cent->nextState.weapon);
        WeaponDef = BG_GetWeaponDef(cent->nextState.weapon);
        projectileSound = WeaponDef->projectileSound;
        if (projectileSound)
            CG_PlaySoundAlias(localClientNum, cent->nextState.number, cent->pose.origin, projectileSound);
        ClientDObj = Com_GetClientDObj(cent->nextState.number, 0);
        if (ClientDObj)
        {
            if (!cent->bTrailMade
                && (WeaponDef->guidedMissileType != MISSILE_GUIDANCE_JAVELIN
                    || cgArray[0].time - cent->nextState.lerp.u.missile.launchTime > WeaponDef->projIgnitionDelay))
            {
                cent->bTrailMade = 1;
                projTrailEffect = WeaponDef->projTrailEffect;
                if (projTrailEffect)
                    CG_PlayBoltedEffect(localClientNum, projTrailEffect, cent->nextState.number, scr_const.tag_fx);
                projIgnitionEffect = WeaponDef->projIgnitionEffect;
                if (projIgnitionEffect)
                    CG_PlayBoltedEffect(localClientNum, projIgnitionEffect, cent->nextState.number, scr_const.tag_fx);
                projIgnitionSound = WeaponDef->projIgnitionSound;
                if (projIgnitionSound)
                    CG_PlaySoundAlias(localClientNum, cent->nextState.number, cent->pose.origin, projIgnitionSound);
            }
            RenderFlagForRefEntity = CG_GetRenderFlagForRefEntity(cent->nextState.lerp.eFlags);
            R_AddDObjToScene(ClientDObj, &cent->pose, cent->nextState.number, RenderFlagForRefEntity, v12, v13);
            CG_AddHudGrenade(cgArray, cent);
        }
    }
}

static float g_entMoveTolVec[3] = { 16.0, 16.0, 16.0 };

// KISAKTODO: Remove this stupid function
bool __cdecl CG_VecLessThan(float *a, float *b)
{
    float v3; // [esp+4h] [ebp-1Ch]
    float v4; // [esp+8h] [ebp-18h]
    float v5; // [esp+Ch] [ebp-14h]
    float v6; // [esp+10h] [ebp-10h]
    float v7; // [esp+14h] [ebp-Ch]
    float v8; // [esp+18h] [ebp-8h]
    float v9; // [esp+1Ch] [ebp-4h]

    v8 = *a - *b;
    v9 = a[1] - b[1];
    v5 = v8 - v9;
    if (v5 < 0.0)
        v6 = a[1] - b[1];
    else
        v6 = *a - *b;
    v7 = a[2] - b[2];
    v4 = v6 - v7;
    if (v4 < 0.0)
        v3 = a[2] - b[2];
    else
        v3 = v6;
    return v3 <= 0.0;
}

void __cdecl CG_UpdateBModelWorldBounds(unsigned int localClientNum, centity_s *cent, int forceFilter)
{
#if 0
    GfxBrushModel *BrushModel; // r30
    double v25; // fp7
    double v26; // fp6
    double v27; // fp8
    double v36; // fp11
    double v37; // fp13
    double v38; // fp0
    float v41; // [sp+50h] [-A0h] BYREF
    float v42; // [sp+54h] [-9Ch]
    float v43; // [sp+58h] [-98h]
    float v44; // [sp+60h] [-90h] BYREF
    float v45; // [sp+64h] [-8Ch]
    float v46; // [sp+68h] [-88h]
    float v47[3]; // [sp+70h] [-80h] BYREF
    _BYTE v48[12]; // [sp+7Ch] [-74h] BYREF
    _BYTE v49[104]; // [sp+88h] [-68h] BYREF

    _R12 = -80;
    __asm { stvx128   v126, r1, r12 }
    _R12 = -64;
    __asm { stvx128   v127, r1, r12 }
    BrushModel = R_GetBrushModel(*(unsigned __int16 *)cent->nextState.index);
    _R31 = 16;
    _R11 = BrushModel->bounds[0];
    _R10 = BrushModel->bounds[1];
    __asm
    {
        lvrx      v0, r31, r11
        lvlx      v13, r0, r11
        vor128    v127, v13, v0
        lvrx      v12, r31, r10
        lvlx      v0, r0, r10
        vor128    v126, v0, v12
    }
    AnglesToAxis(cent->pose.angles, (float (*)[3])v47);
    __asm { vspltw128 v5, v126, 1 }
    __asm { vspltw128 v8, v126, 0 }
    _R10 = &g_zero;
    __asm { vspltw128 v9, v127, 0 }
    _R11 = &g_keepXYZ;
    __asm { vspltw128 v3, v126, 2 }
    _R9 = v47;
    _R8 = v47;
    _R7 = v48;
    __asm { lvx128    v13, r0, r10 }
    _R10 = v48;
    __asm { lvx128    v0, r0, r11 }
    _R11 = cent->pose.origin;
    __asm { lvrx      v12, r31, r9 }
    _R9 = v49;
    __asm { lvlx      v11, r0, r8 }
    _R8 = v49;
    __asm
    {
        vor       v12, v11, v12
        lvrx      v11, r31, r7
        lvlx      v10, r0, r10
    }
    __asm
    {
        vor       v11, v10, v11
        lvlx      v4, r0, r11
        lvrx      v6, r31, r11
    }
    _R11 = &v41;
    __asm
    {
        lvlx      v7, r0, r8
        vor       v6, v4, v6
        lvrx      v10, r31, r9
        vand      v12, v12, v0
        vor       v10, v7, v10
        vspltw128 v7, v127, 1
        vand      v11, v11, v0
        vand      v6, v6, v0
        vand      v10, v10, v0
        vmr       v0, v7
        vcmpgtfp  v4, v13, v11
        vmr       v7, v5
        vcmpgtfp  v5, v13, v12
        vcmpgtfp  v2, v13, v10
        vspltw128 v13, v127, 2
        vsel      v1, v9, v8, v5
        vsel      v9, v8, v9, v5
        vsel      v5, v0, v7, v4
        vsel      v0, v7, v0, v4
        vmaddfp   v8, v1, v6, v12
        vmaddfp   v12, v9, v6, v12
        vmaddfp   v8, v5, v8, v11
        vmaddfp   v12, v0, v12, v11
        vsel      v0, v13, v3, v2
        vsel      v13, v3, v13, v2
        vmaddfp   v0, v0, v8, v10
        vmaddfp   v13, v13, v12, v10
        stvx128   v0, r0, r11
    }
    _R11 = &v44;
    v25 = v42;
    v26 = v41;
    __asm { stvx128   v13, r0, r11 }
    v27 = v44;
    if (forceFilter)
    {
        v36 = v43;
        v38 = v46;
        v37 = v45;
        goto LABEL_6;
    }
    _FP11 = (float)((float)(BrushModel->writable.mins[0] - v41) - (float)(BrushModel->writable.mins[1] - v42));
    __asm { fsel      f0, f11, f13, f0 }
    _FP13 = (float)((float)_FP0 - (float)(BrushModel->writable.mins[2] - v43));
    __asm { fsel      f13, f13, f0, f12 }
    if (_FP13 > 0.0)
        goto LABEL_4;
    _FP4 = (float)((float)(v44 - BrushModel->writable.maxs[0]) - (float)(v45 - BrushModel->writable.maxs[1]));
    __asm { fsel      f13, f4, f12, f13 }
    _FP12 = (float)((float)_FP13 - (float)(v46 - BrushModel->writable.maxs[2]));
    __asm { fsel      f13, f12, f13, f11 }
    if (_FP13 > 0.0)
    {
    LABEL_4:
        v26 = (float)(v41 - g_entMoveTolVec[0]);
        v25 = (float)(v42 - g_entMoveTolVec[1]);
        v27 = (float)(v44 + g_entMoveTolVec[0]);
        v36 = (float)(v43 - g_entMoveTolVec[2]);
        v37 = (float)(v45 + g_entMoveTolVec[1]);
        v38 = (float)(v46 + g_entMoveTolVec[2]);
    LABEL_6:
        BrushModel->writable.mins[0] = v26;
        BrushModel->writable.mins[1] = v25;
        BrushModel->writable.mins[2] = v36;
        BrushModel->writable.maxs[0] = v27;
        BrushModel->writable.maxs[1] = v37;
        BrushModel->writable.maxs[2] = v38;
        R_LinkBModelEntity(localClientNum, cent->nextState.number, BrushModel);
    }
    _R0 = -80;
    __asm { lvx128    v126, r1, r0 }
    _R0 = -64;
    __asm { lvx128    v127, r1, r0 }
#endif
    // TODO(mrsteyk): re-decompiled, check validity!

   //float* v4; // [esp-10h] [ebp-1F0h]
    float maxs[3]; // [esp-Ch] [ebp-1ECh] BYREF
    float v6; // [esp+0h] [ebp-1E0h]
    float mins[3]; // [esp+4h] [ebp-1DCh] BYREF
    float v8[8]; // [esp+10h] [ebp-1D0h]
    float v9; // [esp+30h] [ebp-1B0h]
    float v10; // [esp+34h] [ebp-1ACh]
    float v11; // [esp+38h] [ebp-1A8h]
    float4 rotatedBounds[2]; // [esp+3Ch] [ebp-1A4h] BYREF
    int32_t v13; // [esp+5Ch] [ebp-184h]
    __int64 v14; // [esp+60h] [ebp-180h]
    int32_t v15; // [esp+68h] [ebp-178h]
    int32_t v16; // [esp+6Ch] [ebp-174h]
    int32_t v17; // [esp+70h] [ebp-170h]
    int32_t v18; // [esp+74h] [ebp-16Ch]
    int32_t v19; // [esp+78h] [ebp-168h]
    __int64 v20; // [esp+7Ch] [ebp-164h]
    int32_t v21; // [esp+84h] [ebp-15Ch]
    int32_t v22; // [esp+88h] [ebp-158h]
    int32_t v23; // [esp+8Ch] [ebp-154h]
    float *v24; // [esp+90h] [ebp-150h]
    __int64 v25; // [esp+94h] [ebp-14Ch]
    int32_t v26; // [esp+9Ch] [ebp-144h]
    int32_t v27; // [esp+A0h] [ebp-140h]
    __int64 v28; // [esp+A4h] [ebp-13Ch]
    int32_t v29; // [esp+ACh] [ebp-134h]
    int32_t v30; // [esp+B0h] [ebp-130h]
    int32_t v31; // [esp+B4h] [ebp-12Ch]
    int32_t v32; // [esp+B8h] [ebp-128h]
    int32_t v33; // [esp+BCh] [ebp-124h]
    __int64 v34; // [esp+C0h] [ebp-120h]
    int32_t v35; // [esp+C8h] [ebp-118h]
    int32_t v36; // [esp+CCh] [ebp-114h]
    int32_t v37; // [esp+D0h] [ebp-110h]
    float *v38; // [esp+D4h] [ebp-10Ch]
    __int64 v39; // [esp+D8h] [ebp-108h]
    int32_t v40; // [esp+E0h] [ebp-100h]
    int32_t v41; // [esp+E4h] [ebp-FCh]
    __int64 v42; // [esp+E8h] [ebp-F8h]
    int32_t v43; // [esp+F0h] [ebp-F0h]
    int32_t v44; // [esp+F4h] [ebp-ECh]
    int32_t v45; // [esp+F8h] [ebp-E8h]
    int32_t v46; // [esp+FCh] [ebp-E4h]
    int32_t v47; // [esp+100h] [ebp-E0h]
    __int64 v48; // [esp+104h] [ebp-DCh]
    int32_t v49; // [esp+10Ch] [ebp-D4h]
    int32_t v50; // [esp+110h] [ebp-D0h]
    int32_t v51; // [esp+114h] [ebp-CCh]
    __int64 v52; // [esp+118h] [ebp-C8h]
    float v53; // [esp+120h] [ebp-C0h]
    float v54; // [esp+124h] [ebp-BCh]
    float *v55; // [esp+128h] [ebp-B8h]
    __int64 v56; // [esp+12Ch] [ebp-B4h]
    float v57; // [esp+134h] [ebp-ACh]
    float v58; // [esp+138h] [ebp-A8h]
    float v59; // [esp+13Ch] [ebp-A4h]
    float v60; // [esp+140h] [ebp-A0h]
    float v61; // [esp+144h] [ebp-9Ch]
    float v62; // [esp+148h] [ebp-98h]
    float v63; // [esp+14Ch] [ebp-94h]
    float v64; // [esp+150h] [ebp-90h]
    float v65; // [esp+154h] [ebp-8Ch]
    float v66; // [esp+158h] [ebp-88h]
    float *v67; // [esp+15Ch] [ebp-84h]
    float v68; // [esp+160h] [ebp-80h]
    float v69; // [esp+164h] [ebp-7Ch]
    float v70; // [esp+168h] [ebp-78h]
    float v71; // [esp+16Ch] [ebp-74h]
    float *v72; // [esp+170h] [ebp-70h]
    float v73; // [esp+174h] [ebp-6Ch]
    float v74; // [esp+178h] [ebp-68h]
    float v75; // [esp+17Ch] [ebp-64h]
    float v76; // [esp+180h] [ebp-60h]
    float *origin; // [esp+184h] [ebp-5Ch]
    float v78[3][3]; // [esp+188h] [ebp-58h] BYREF
    float axis_24[4]; // [esp+1ACh] [ebp-34h]
    float bounds_4[3]; // [esp+1BCh] [ebp-24h] BYREF
    int32_t v81; // [esp+1C8h] [ebp-18h]
    GfxBrushModel *brush; // [esp+1CCh] [ebp-14h]
    //int32_t bounds_28; // [esp+1D4h] [ebp-Ch]
    //GfxBrushModel* bmodel; // [esp+1D8h] [ebp-8h]
    //GfxBrushModel* retaddr; // [esp+1E0h] [ebp+0h]

    //bounds_28 = a1;
    //bmodel = retaddr;
    //brush = R_GetBrushModel(cent->nextState.index.brushmodel);
    brush = R_GetBrushModel(*(unsigned __int16 *)cent->nextState.index);
    axis_24[0] = brush->bounds[0][0];
    axis_24[1] = brush->bounds[0][1];
    axis_24[2] = brush->bounds[0][2];
    axis_24[3] = brush->bounds[1][0];
    bounds_4[0] = brush->bounds[1][0];
    bounds_4[1] = brush->bounds[1][1];
    bounds_4[2] = brush->bounds[1][2];
    v81 = *(_DWORD *)&brush->surfaceCount;
    AnglesToAxis(cent->pose.angles, v78);
    origin = cent->pose.origin;
    v73 = v78[0][0];
    v74 = v78[0][1];
    v75 = v78[0][2];
    v76 = 0.0f;
    v72 = v78[1];
    v68 = v78[1][0];
    v69 = v78[1][1];
    v70 = v78[1][2];
    v71 = 0.0f;
    v67 = v78[2];
    v63 = v78[2][0];
    v64 = v78[2][1];
    v65 = v78[2][2];
    v66 = 0.0f;
    v59 = cent->pose.origin[0];
    v60 = cent->pose.origin[1];
    v61 = cent->pose.origin[2];
    v62 = 0.0f;
    *(float *)&v56 = axis_24[0];
    *((float *)&v56 + 1) = axis_24[0];
    v57 = axis_24[0];
    v58 = axis_24[0];
    v55 = bounds_4;
    *(float *)&v52 = bounds_4[0];
    *((float *)&v52 + 1) = bounds_4[0];
    v53 = bounds_4[0];
    v54 = bounds_4[0];
    if (v78[0][0] >= 0.0)
        v51 = 0;
    else
        v51 = -1;
    LODWORD(v48) = v51;
    if (v74 >= 0.0)
        v47 = 0;
    else
        v47 = -1;
    HIDWORD(v48) = v47;
    if (v75 >= 0.0)
        v46 = 0;
    else
        v46 = -1;
    v49 = v46;
    if (v76 >= 0.0)
        v45 = 0;
    else
        v45 = -1;
    v50 = v45;
    v42 = v52 & v48 | v56 & ~v48;
    v43 = LODWORD(v53) & v49 | LODWORD(v57) & ~v49;
    v44 = LODWORD(v54) & v50 | LODWORD(v58) & ~v50;
    v39 = v56 & v48 | v52 & ~v48;
    v40 = LODWORD(v57) & v49 | LODWORD(v53) & ~v49;
    v41 = LODWORD(v58) & v50 | LODWORD(v54) & ~v50;
    *(float *)&v56 = axis_24[1];
    *((float *)&v56 + 1) = axis_24[1];
    v57 = axis_24[1];
    v58 = axis_24[1];
    v38 = bounds_4;
    *(float *)&v52 = bounds_4[1];
    *((float *)&v52 + 1) = bounds_4[1];
    v53 = bounds_4[1];
    v54 = bounds_4[1];
    if (v68 >= 0.0f)
        v37 = 0;
    else
        v37 = -1;
    LODWORD(v34) = v37;
    if (v69 >= 0.0f)
        v33 = 0;
    else
        v33 = -1;
    HIDWORD(v34) = v33;
    if (v70 >= 0.0f)
        v32 = 0;
    else
        v32 = -1;
    v35 = v32;
    if (v71 >= 0.0f)
        v31 = 0;
    else
        v31 = -1;
    v36 = v31;
    v28 = v52 & v34 | v56 & ~v34;
    v29 = LODWORD(v53) & v35 | LODWORD(v57) & ~v35;
    v30 = LODWORD(v54) & v36 | LODWORD(v58) & ~v36;
    v25 = v56 & v34 | v52 & ~v34;
    v26 = LODWORD(v57) & v35 | LODWORD(v53) & ~v35;
    v27 = LODWORD(v58) & v36 | LODWORD(v54) & ~v36;
    *(float *)&v56 = axis_24[2];
    *((float *)&v56 + 1) = axis_24[2];
    v57 = axis_24[2];
    v58 = axis_24[2];
    v24 = bounds_4;
    *(float *)&v52 = bounds_4[2];
    *((float *)&v52 + 1) = bounds_4[2];
    v53 = bounds_4[2];
    v54 = bounds_4[2];
    if (v63 >= 0.0f)
        v23 = 0;
    else
        v23 = -1;
    LODWORD(v20) = v23;
    if (v64 >= 0.0f)
        v19 = 0;
    else
        v19 = -1;
    HIDWORD(v20) = v19;
    if (v65 >= 0.0f)
        v18 = 0;
    else
        v18 = -1;
    v21 = v18;
    if (v66 >= 0.0f)
        v17 = 0;
    else
        v17 = -1;
    v22 = v17;
    v14 = v52 & v20 | v56 & ~v20;
    v15 = LODWORD(v53) & v21 | LODWORD(v57) & ~v21;
    v16 = LODWORD(v54) & v22 | LODWORD(v58) & ~v22;
    *(_QWORD *)&rotatedBounds[1].unitVec[1].packed = v56 & v20 | v52 & ~v20;
    rotatedBounds[1].u[3] = LODWORD(v57) & v21 | LODWORD(v53) & ~v21;
    v13 = LODWORD(v58) & v22 | LODWORD(v54) & ~v22;
    v9 = *(float *)&v42 * v73 + v59;
    v10 = *((float *)&v42 + 1) * v74 + v60;
    v11 = *(float *)&v43 * v75 + v61;
    rotatedBounds[0].v[0] = *(float *)&v44 * v76 + v62;
    v9 = *(float *)&v28 * v68 + v9;
    v10 = *((float *)&v28 + 1) * v69 + v10;
    v11 = *(float *)&v29 * v70 + v11;
    rotatedBounds[0].v[0] = *(float *)&v30 * v71 + rotatedBounds[0].v[0];
    v9 = *(float *)&v14 * v63 + v9;
    v10 = *((float *)&v14 + 1) * v64 + v10;
    v11 = *(float *)&v15 * v65 + v11;
    rotatedBounds[0].v[0] = *(float *)&v16 * v66 + rotatedBounds[0].v[0];
    LODWORD(v8[7]) = (uintptr_t)&rotatedBounds[0].v[1];
    rotatedBounds[0].v[1] = *(float *)&v39 * v73 + v59;
    rotatedBounds[0].v[2] = *((float *)&v39 + 1) * v74 + v60;
    rotatedBounds[0].v[3] = *(float *)&v40 * v75 + v61;
    rotatedBounds[1].v[0] = *(float *)&v41 * v76 + v62;
    LODWORD(v8[6]) = (uintptr_t)&rotatedBounds[0].v[1];
    LODWORD(v8[5]) = (uintptr_t)&rotatedBounds[0].v[1];
    rotatedBounds[0].v[1] = *(float *)&v25 * v68 + rotatedBounds[0].v[1];
    rotatedBounds[0].v[2] = *((float *)&v25 + 1) * v69 + rotatedBounds[0].v[2];
    rotatedBounds[0].v[3] = *(float *)&v26 * v70 + rotatedBounds[0].v[3];
    rotatedBounds[1].v[0] = *(float *)&v27 * v71 + rotatedBounds[1].v[0];
    LODWORD(v8[4]) = (uintptr_t)&rotatedBounds[0].v[1];
    LODWORD(v8[3]) = (uintptr_t)&rotatedBounds[0].v[1];
    rotatedBounds[0].v[1] = rotatedBounds[1].v[1] * v63 + rotatedBounds[0].v[1];
    rotatedBounds[0].v[2] = rotatedBounds[1].v[2] * v64 + rotatedBounds[0].v[2];
    rotatedBounds[0].v[3] = rotatedBounds[1].v[3] * v65 + rotatedBounds[0].v[3];
    rotatedBounds[1].v[0] = *(float *)&v13 * v66 + rotatedBounds[1].v[0];
    mins[0] = v9;
    mins[1] = v10;
    mins[2] = v11;
    v8[0] = rotatedBounds[0].v[0];
    *(_QWORD *)maxs = *(_QWORD *)&rotatedBounds[0].unitVec[1].packed;
    maxs[2] = rotatedBounds[0].v[3];
    v6 = rotatedBounds[1].v[0];
    if (forceFilter)
        goto LABEL_41;
    if (!CG_VecLessThan(brush->writable.mins, mins) || !CG_VecLessThan(maxs, brush->writable.maxs))
    {
        Vec3Sub(mins, g_entMoveTolVec, mins);
        Vec3Add(maxs, g_entMoveTolVec, maxs);
    LABEL_41:
        brush->writable.mins[0] = mins[0];
        brush->writable.mins[1] = mins[1];
        brush->writable.mins[2] = mins[2];
        brush->writable.maxs[0] = maxs[0];
        brush->writable.maxs[1] = maxs[1];
        brush->writable.maxs[2] = maxs[2];
        R_LinkBModelEntity(localClientNum, cent->nextState.number, brush);
    }
}

void __cdecl CG_ScriptMover(int localClientNum, centity_s *cent)
{
    entityState_s *p_nextState; // r31
    DObj_s *ClientDObj; // r29
    unsigned int RenderFlagForRefEntity; // r3
    float *v7; // r7
    double v8; // fp1
    cg_s *LocalClientGlobals; // r3
    unsigned int number; // r29
    const GfxBrushModel *BrushModel; // r3
    float v12; // [sp+50h] [-40h] BYREF

    p_nextState = &cent->nextState;
    if ((cent->nextState.lerp.eFlags & 0x20) == 0)
    {
        if (cent->nextState.solid == 0xFFFFFF)
        {
            number = cent->nextState.number;
            BrushModel = R_GetBrushModel(*(unsigned __int16 *)cent->nextState.index);
            R_AddBrushModelToSceneFromAngles(BrushModel, cent->pose.origin, cent->pose.angles, number);
        }
        else
        {
            ClientDObj = Com_GetClientDObj(cent->nextState.number, 0);
            if (ClientDObj)
            {
                if ((CG_LockLightingOrigin(cent, &v12),
                    RenderFlagForRefEntity = CG_GetRenderFlagForRefEntity(p_nextState->lerp.eFlags),
                    R_AddDObjToScene(ClientDObj, &cent->pose, p_nextState->number, RenderFlagForRefEntity, v7, v8),
                    CG_LookingThroughNightVision(localClientNum))
                    && (p_nextState->lerp.eFlags & 0x4000) != 0
                    || cg_laserForceOn->current.enabled)
                {
                    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
                    CG_Laser_Add(cent, ClientDObj, &cent->pose, LocalClientGlobals->refdef.viewOffset, LASER_OWNER_NON_PLAYER);
                }
            }
        }
    }
}

void __cdecl CG_AdjustPositionForMover(
    int localClientNum,
    float *in,
    int moverNum,
    int fromTime,
    int toTime,
    float *out,
    float *outDeltaAngles)
{
    centity_s *Entity; // r3
    centity_s *v13; // r27
    const trajectory_t *p_pos; // r28
    const trajectory_t *p_apos; // r27
    double v16; // fp13
    double v17; // fp12
    double v18; // fp11
    double v19; // fp10
    double v20; // fp7
    double v21; // fp9
    double v22; // fp9
    float v23[4]; // [sp+50h] [-80h] BYREF
    float v24[4]; // [sp+60h] [-70h] BYREF
    float v25[4]; // [sp+70h] [-60h] BYREF
    float v26[20]; // [sp+80h] [-50h] BYREF

    if (outDeltaAngles)
    {
        *outDeltaAngles = 0.0;
        outDeltaAngles[1] = 0.0;
        outDeltaAngles[2] = 0.0;
    }
    if ((unsigned int)(moverNum - 1) <= 0x87C
        && (Entity = CG_GetEntity(localClientNum, moverNum), v13 = Entity, Entity->nextState.eType == 5))
    {
        p_pos = &Entity->currentState.pos;
        BG_EvaluateTrajectory(&Entity->currentState.pos, fromTime, v23);
        p_apos = &v13->currentState.apos;
        BG_EvaluateTrajectory(p_apos, fromTime, v25);
        BG_EvaluateTrajectory(p_pos, toTime, v24);
        BG_EvaluateTrajectory(p_apos, toTime, v26);
        v16 = (float)(v24[1] - v23[1]);
        v17 = (float)(v24[2] - v23[2]);
        v18 = (float)(v26[0] - v25[0]);
        v19 = (float)(v26[1] - v25[1]);
        v20 = v26[2];
        *out = *in + (float)(v24[0] - v23[0]);
        out[1] = in[1] + (float)v16;
        v21 = v25[2];
        out[2] = in[2] + (float)v17;
        v22 = (float)((float)v20 - (float)v21);
        if (outDeltaAngles)
        {
            *outDeltaAngles = v18;
            outDeltaAngles[1] = v19;
            outDeltaAngles[2] = v22;
        }
    }
    else
    {
        *out = *in;
        out[1] = in[1];
        out[2] = in[2];
    }
}

void __cdecl CG_SetFrameInterpolation(int localClientNum)
{
    snapshot_s *nextSnap; // r11
    __int64 v2; // r11
    __int64 v3; // r10
    __int64 v4; // [sp+50h] [-30h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (!cgArray[0].snap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 487, 0, "%s", "cgameGlob->snap");
    nextSnap = cgArray[0].nextSnap;
    if (!cgArray[0].nextSnap)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 488, 0, "%s", "cgameGlob->nextSnap");
        nextSnap = cgArray[0].nextSnap;
    }
    HIDWORD(v2) = cgArray[0].snap->serverTime;
    LODWORD(v2) = nextSnap->serverTime - HIDWORD(v2);
    if ((unsigned int)v2)
    {
        v4 = v2;
        HIDWORD(v3) = cgArray[0].time;
        LODWORD(v3) = cgArray[0].time - HIDWORD(v2);
        cgArray[0].frameInterpolation = (float)v3 / (float)v4;
        if (cgArray[0].frameInterpolation < 0.0)
            cgArray[0].frameInterpolation = 0.0;
    }
    else
    {
        cgArray[0].frameInterpolation = 0.0;
    }
}

void __cdecl CG_DObjUpdateInfo(const cg_s *cgameGlob, DObj_s *obj, bool notify)
{
    __int64 v3; // r11

    HIDWORD(v3) = 108040;
    LODWORD(v3) = cgameGlob->animFrametime;
    DObjUpdateClientInfo(obj, (float)((float)v3 * (float)0.001), (bool)obj);
}

centity_s *__cdecl CG_GetPose(int localClientNum, int handle)
{
    if ((unsigned int)handle >= 0x900)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
            755,
            0,
            "%s\n\t(handle) = %i",
            "(handle >= 0 && handle < (((2176)) + 128))",
            handle);
    if (handle < 2176)
        return CG_GetEntity(localClientNum, handle);
    if (handle - 2176 >= 128)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
            763,
            0,
            "%s\n\t(handle) = %i",
            "(handle >= ((2176)) && handle - ((2176)) < 128)",
            handle);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return (centity_s *)&cgArray[0].viewModelPose;
}

unsigned __int16 *g_wheelTags[6] =
{
  &scr_const.tag_wheel_front_left,
  &scr_const.tag_wheel_front_right,
  &scr_const.tag_wheel_back_left,
  &scr_const.tag_wheel_back_right,
  &scr_const.tag_wheel_middle_left,
  &scr_const.tag_wheel_middle_right,
};

void __cdecl CG_Vehicle_PreControllers(int localClientNum, const DObj_s *obj, centity_s *cent)
{
    long double v6; // fp2
    long double v7; // fp2
    long double v8; // fp2
    long double v9; // fp2
    long double v10; // fp2
    long double v11; // fp2
    long double v12; // fp2
    long double v13; // fp2
    long double v14; // fp2
    long double v15; // fp2
    __int64 v16; // r11
    unsigned __int8 *v17; // r11
    int v18; // ctr
    double height; // fp30
    const XModel *Model; // r3
    const DObjAnimMat *BasePose; // r24
    unsigned __int16 **v22; // r27
    unsigned __int8 *wheelBoneIndex; // r30
    CEntFx *v24; // r29
    int v25; // r25
    int number; // r8
    float v27[4]; // [sp+58h] [-F8h] BYREF
    float v28[4]; // [sp+68h] [-E8h] BYREF
    float v29; // [sp+78h] [-D8h] BYREF
    float v30; // [sp+7Ch] [-D4h]
    float v31; // [sp+80h] [-D0h]
    float v32[2][3]; // [sp+90h] [-C0h] BYREF
    float v33; // [sp+A8h] [-A8h]
    float v34; // [sp+ACh] [-A4h]
    float v35; // [sp+B0h] [-A0h]
    float v36; // [sp+B4h] [-9Ch]
    float v37; // [sp+B8h] [-98h]
    float v38; // [sp+BCh] [-94h]
    trace_t v39; // [sp+C0h] [-90h] BYREF

    if (!obj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 800, 0, "%s", "obj");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    *(double *)&v6 = (float)((float)(LerpAngle(
        cent->currentState.u.turret.gunAngles[0],
        cent->nextState.lerp.u.turret.gunAngles[0],
        cgArray[0].frameInterpolation)
        * (float)182.04445)
        + (float)0.5);
    v7 = floor(v6);
    cent->pose.vehicle.pitch = (int)*(double *)&v7;
    *(double *)&v8 = (float)((float)(LerpAngle(
        cent->currentState.u.turret.gunAngles[1],
        cent->nextState.lerp.u.turret.gunAngles[1],
        cgArray[0].frameInterpolation)
        * (float)182.04445)
        + (float)0.5);
    v9 = floor(v8);
    cent->pose.vehicle.roll = (int)*(double *)&v9;
    *(double *)&v10 = (float)((float)(LerpAngle(
        cent->currentState.u.primaryLight.cosHalfFovInner,
        cent->nextState.lerp.u.primaryLight.cosHalfFovInner,
        cgArray[0].frameInterpolation)
        * (float)182.04445)
        + (float)0.5);
    v11 = floor(v10);
    cent->pose.vehicle.barrelPitch = (int)*(double *)&v11;
    *(double *)&v12 = (float)((float)(LerpAngle(
        cent->currentState.u.vehicle.gunYaw,
        cent->nextState.lerp.u.vehicle.gunYaw,
        cgArray[0].frameInterpolation)
        * (float)182.04445)
        + (float)0.5);
    v13 = floor(v12);
    cent->pose.vehicle.yaw = (int)*(double *)&v13;
    *(double *)&v14 = (float)((float)(LerpAngle(
        cent->currentState.u.turret.gunAngles[2],
        cent->nextState.lerp.u.turret.gunAngles[2],
        cgArray[0].frameInterpolation)
        * (float)182.04445)
        + (float)0.5);
    v15 = floor(v14);
    cent->pose.vehicle.steerYaw = (int)*(double *)&v15;
    LODWORD(v16) = cent->nextState.time2;
    cent->pose.actor.height = (float)v16 * (float)0.001;
    DObjGetBoneIndex(obj, scr_const.tag_body, &cent->pose.vehicle.tag_body);
    DObjGetBoneIndex(obj, scr_const.tag_turret, &cent->pose.vehicle.tag_turret);
    DObjGetBoneIndex(obj, scr_const.tag_barrel, &cent->pose.vehicle.tag_barrel);
    if (cent->pose.cullIn == 2)
    {
        height = cent->pose.actor.height;
        AnglesToAxis(cent->pose.angles, v32);
        v36 = cent->pose.origin[0];
        v37 = cent->pose.origin[1];
        v38 = cent->pose.origin[2];
        Model = DObjGetModel(obj, 0);
        BasePose = XModelGetBasePose(Model);
        if (*g_wheelTags[0] != scr_const.tag_wheel_front_left)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
                835,
                0,
                "%s",
                "*g_wheelTags[TAG_WHEEL_FRONT_LEFT] == scr_const.tag_wheel_front_left");
        if (*g_wheelTags[1] != scr_const.tag_wheel_front_right)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
                836,
                0,
                "%s",
                "*g_wheelTags[TAG_WHEEL_FRONT_RIGHT] == scr_const.tag_wheel_front_right");
        v22 = g_wheelTags;
        wheelBoneIndex = cent->pose.vehicle.wheelBoneIndex;
        v24 = &cent->pose.fx + 2;
        v25 = 6;
        do
        {
            if (DObjGetBoneIndex(obj, **v22, wheelBoneIndex))
            {
                MatrixTransformVector43((const float *)((char *)BasePose->trans + __ROL4__(*wheelBoneIndex, 5)), v32, &v29);
                v27[0] = (float)(v33 * (float)40.0) + v29;
                v27[1] = (float)(v34 * (float)40.0) + v30;
                number = cent->nextState.number;
                v28[0] = (float)(v33 * (float)-height) + v29;
                v27[2] = (float)(v35 * (float)40.0) + v31;
                v28[1] = (float)(v34 * (float)-height) + v30;
                v28[2] = (float)(v35 * (float)-height) + v31;
                CG_TraceCapsule(&v39, v27, vec3_origin, vec3_origin, v28, number, 529);
                HIWORD(v24->triggerTime) = CompressUnit(v39.fraction);
            }
            --v25;
            v24 = (CEntFx *)((char *)v24 + 2);
            ++v22;
            ++wheelBoneIndex;
        } while (v25);
    }
    else
    {
        v17 = cent->pose.vehicle.wheelBoneIndex;
        v18 = 6;
        do
        {
            *v17++ = -2;
            --v18;
        } while (v18);
    }
}

void __cdecl CG_Vehicle(int localClientNum, centity_s *cent)
{
    entityState_s *p_nextState; // r30
    const DObj_s *ClientDObj; // r3
    const DObj_s *v6; // r29
    int RenderFlagForRefEntity; // r3
    float *v8; // r7
    double v9; // fp1

    p_nextState = &cent->nextState;
    if ((cent->nextState.lerp.eFlags & 0x20) == 0)
    {
        ClientDObj = Com_GetClientDObj(cent->nextState.number, 0);
        v6 = ClientDObj;
        if (ClientDObj)
        {
            CG_Vehicle_PreControllers(localClientNum, ClientDObj, cent);
            CG_GetLocalClientGlobals(localClientNum);
            RenderFlagForRefEntity = CG_GetRenderFlagForRefEntity(p_nextState->lerp.eFlags);
            R_AddDObjToScene(v6, &cent->pose, p_nextState->number, RenderFlagForRefEntity | 4, v8, v9);
            if (p_nextState->eType == 11)
                CG_CompassUpdateVehicleInfo(localClientNum, p_nextState->number);
        }
    }
}

void __cdecl CG_SoundBlend(int localClientNum, centity_s *cent)
{
    entityState_s *p_nextState; // r31
    unsigned int v5; // r11
    const char *ConfigString; // r3
    const snd_alias_t *v7; // r28
    const char *v8; // r3
    snd_alias_t *v9; // r3
    const snd_alias_t *v10; // r27
    const float *v11; // r6
    int v12; // r5
    cg_s *LocalClientGlobals; // r3
    double v14; // fp31
    double v15; // fp30

    p_nextState = &cent->nextState;
    v5 = cent->nextState.eventParms[0];
    if (v5)
    {
        if (cent->nextState.eventParms[1])
        {
            ConfigString = CL_GetConfigString(localClientNum, v5 + 1667);
            v7 = CL_PickSoundAlias(ConfigString);
            v8 = CL_GetConfigString(localClientNum, p_nextState->eventParms[1] + 1667);
            v9 = CL_PickSoundAlias(v8);
            v10 = v9;
            if (v7)
            {
                if (v9)
                {
                    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
                    v14 = (float)((float)((float)(p_nextState->lerp.u.turret.gunAngles[0]
                        - cent->currentState.u.turret.gunAngles[0])
                        * LocalClientGlobals->frameInterpolation)
                        + cent->currentState.u.turret.gunAngles[0]);
                    v15 = (float)((float)((float)(p_nextState->lerp.u.turret.gunAngles[1]
                        - cent->currentState.u.turret.gunAngles[1])
                        * LocalClientGlobals->frameInterpolation)
                        + cent->currentState.u.turret.gunAngles[1]);
                    if (v14 < 0.0 || v14 > 1.0)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
                            926,
                            0,
                            "%s",
                            "(lerp >= 0.0f) && (lerp <= 1.0f)");
                    SND_PlayBlendedSoundAliases(
                        v7,
                        v10,
                        v14,
                        v15,
                        v12,
                        v11,
                        p_nextState->number,
                        (snd_alias_system_t)cent->pose.origin);
                }
            }
        }
    }
}

FxEffect *__cdecl CG_StartFx(int localClientNum, centity_s *cent, int startAtTime)
{
    int scale; // r31
    const FxEffectDef *v7; // r31
    float v9[8][3]; // [sp+50h] [-60h] BYREF

    AnglesToAxis(cent->nextState.lerp.apos.trBase, v9);
    scale = cent->nextState.un1.scale;
    if (!cent->nextState.un1.scale || cent->nextState.un1.scale > 0x63u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
            943,
            0,
            "fxId not in [1, MAX_EFFECT_NAMES - 1]\n\t%i not in [%i, %i]",
            cent->nextState.un1.scale,
            1,
            99);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            917,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v7 = cgsArray[0].fxs[scale];
    if (!v7)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 946, 0, "%s", "fxDef");
    return FX_SpawnOrientedEffect(localClientNum, v7, startAtTime, cent->pose.origin, v9, 0x87Fu);
}

void __cdecl CG_Fx(int localClientNum, centity_s *cent)
{
    FxEffect *effect; // r4
    FxEffect *started; // r3

    if (cent->pose.actor.proneType != cent->nextState.time2)
    {
        effect = cent->pose.fx.effect;
        if (effect)
            FX_ThroughWithEffect(localClientNum, effect);
        cent->pose.actor.proneType = 0;
        cent->pose.fx.effect = 0;
        started = CG_StartFx(localClientNum, cent, cent->nextState.time2);
        cent->pose.fx.effect = started;
        if (started)
            cent->pose.actor.proneType = cent->nextState.time2;
    }
}

void __cdecl CG_LoopFx(int localClientNum, centity_s *cent)
{
    int period; // r28
    FxEffect *started; // r3
    int v6; // r10

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cent->nextState.lerp.u.turret.gunAngles[0] == 0.0
        || (float)((float)((float)(cent->pose.origin[1] - cgArray[0].predictedPlayerState.origin[1])
            * (float)(cent->pose.origin[1] - cgArray[0].predictedPlayerState.origin[1]))
            + (float)((float)((float)(cent->pose.origin[0] - cgArray[0].predictedPlayerState.origin[0])
                * (float)(cent->pose.origin[0] - cgArray[0].predictedPlayerState.origin[0]))
                + (float)((float)(cent->pose.origin[2] - cgArray[0].predictedPlayerState.origin[2])
                    * (float)(cent->pose.origin[2] - cgArray[0].predictedPlayerState.origin[2])))) < (double)(float)(cent->nextState.lerp.u.turret.gunAngles[0] * cent->nextState.lerp.u.turret.gunAngles[0]))
    {
        period = cent->nextState.lerp.u.loopFx.period;
        if (period <= 0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 981, 0, "%s", "period > 0");
        if (!cent->pose.fx.effect)
        {
            started = CG_StartFx(localClientNum, cent, cgArray[0].time);
            cent->pose.fx.effect = started;
            if (!started)
                return;
            cent->pose.actor.proneType = cgArray[0].time + period;
        }
        if (cgArray[0].time >= cent->pose.actor.proneType)
        {
            do
            {
                FX_RetriggerEffect(localClientNum, cent->pose.fx.effect, cent->pose.actor.proneType);
                v6 = cent->pose.actor.proneType + period;
                cent->pose.actor.proneType = v6;
            } while (cgArray[0].time >= v6);
        }
    }
}

void __cdecl CG_ClampPrimaryLightOrigin(GfxLight *light, const ComPrimaryLight *refLight)
{
    double v2; // fp13
    double v3; // fp12
    double v4; // fp11
    double v5; // fp0
    double v6; // fp0

    v2 = (float)(light->origin[1] - refLight->origin[1]);
    v3 = (float)(light->origin[2] - refLight->origin[2]);
    v4 = (float)(light->origin[0] - refLight->origin[0]);
    v5 = (float)((float)((float)v4 * (float)v4) + (float)((float)((float)v3 * (float)v3) + (float)((float)v2 * (float)v2)));
    if (v5 >= (float)(refLight->translationLimit * refLight->translationLimit))
    {
        v6 = (float)(refLight->translationLimit / (float)__fsqrts(v5));
        light->origin[0] = (float)((float)v6 * (float)(light->origin[0] - refLight->origin[0])) + refLight->origin[0];
        light->origin[1] = (float)((float)v6 * (float)v2) + refLight->origin[1];
        light->origin[2] = (float)((float)v3 * (float)v6) + refLight->origin[2];
    }
}

void __cdecl CG_ClampPrimaryLightDir(GfxLight *light, const ComPrimaryLight *refLight)
{
    double rotationLimit; // fp12
    double v5; // fp11
    double v6; // fp0
    double v7; // fp10
    double v8; // fp9
    const char *v9; // r3
    double v10; // [sp+30h] [-40h]

    rotationLimit = refLight->rotationLimit;
    v5 = (float)((float)(light->dir[0] * refLight->dir[0])
        + (float)((float)(light->dir[2] * refLight->dir[2]) + (float)(light->dir[1] * refLight->dir[1])));
    if (v5 < rotationLimit)
    {
        v6 = __fsqrts((float)((float)-(float)((float)(refLight->rotationLimit * refLight->rotationLimit) - (float)1.0)
            / (float)-(float)((float)((float)v5 * (float)v5) - (float)1.0)));
        v7 = (float)((float)((float)(refLight->dir[1] * (float)-v5) + light->dir[1]) * (float)v6);
        v8 = (float)((float)((float)(refLight->dir[2] * (float)-v5) + light->dir[2]) * (float)v6);
        light->dir[0] = (float)((float)((float)(refLight->dir[0] * (float)-v5) + light->dir[0]) * (float)v6)
            + (float)(refLight->rotationLimit * refLight->dir[0]);
        light->dir[1] = (float)(refLight->dir[1] * (float)rotationLimit) + (float)v7;
        light->dir[2] = (float)(refLight->dir[2] * (float)rotationLimit) + (float)v8;
        if (__fabs((float)((float)((float)(light->dir[2] * light->dir[2])
            + (float)((float)(light->dir[0] * light->dir[0]) + (float)(light->dir[1] * light->dir[1])))
            - (float)1.0)) >= 0.0020000001)
        {
            v10 = __fsqrts((float)((float)(light->dir[2] * light->dir[2])
                + (float)((float)(light->dir[0] * light->dir[0]) + (float)(light->dir[1] * light->dir[1]))));
            v9 = va(
                (const char *)(const char *)HIDWORD(COERCE_UNSIGNED_INT64(light->dir[0])),
                (unsigned int)COERCE_UNSIGNED_INT64(light->dir[0]),
                (unsigned int)COERCE_UNSIGNED_INT64(light->dir[1]),
                HIDWORD(v10),
                LODWORD(v10));
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
                1030,
                0,
                "%s\n\t%s",
                "Vec3IsNormalized( light->dir )",
                v9);
        }
        if (__fabs((float)((float)((float)(light->dir[0] * refLight->dir[0])
            + (float)((float)(light->dir[2] * refLight->dir[2])
                + (float)(light->dir[1] * refLight->dir[1])))
            - refLight->rotationLimit)) > 0.001)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
                1031,
                0,
                "%s",
                "I_fabs( Vec3Dot( light->dir, refLight->dir ) - refLight->rotationLimit ) <= 0.001f");
    }
}

// local variable allocation has failed, the output may be wrong!
void __cdecl CG_PrimaryLight(int localClientNum, centity_s *cent)
{
    unsigned int PrimaryLightCount; // r3
    unsigned int v5; // r3
    LerpEntityState *p_currentState; // r29
    LerpEntityState *p_lerp; // r30
    GfxLight *v8; // r31
    const ComPrimaryLight *PrimaryLight; // r3
    __int128 v10; // r11
    double v11; // fp13
    __int64 v12; // r8 OVERLAPPED
    double v13; // fp12
    __int64 v14; // r6
    const ComPrimaryLight *v15; // r26
    double frameInterpolation; // fp11
    double v17; // fp0
    double v18; // fp13
    double cosHalfFovOuter; // fp1
    int v20; // r11
    int v21; // r9
    double cosHalfFovInner; // fp0
    const char *v23; // r3
    float v24[4]; // [sp+80h] [-60h] BYREF

    if (cent->nextState.eType != 9)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
            1046,
            0,
            "%s",
            "cent->nextState.eType == ET_PRIMARY_LIGHT");
    if (!*(_WORD *)cent->nextState.index)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
            1047,
            0,
            "%s",
            "cent->nextState.index.primaryLight != PRIMARY_LIGHT_NONE");
    if (!comWorld.isInUse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\../qcommon/com_bsp_api.h", 23, 0, "%s", "comWorld.isInUse");
    if (*(unsigned __int16 *)cent->nextState.index >= comWorld.primaryLightCount)
    {
        PrimaryLightCount = Com_GetPrimaryLightCount();
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
            1048,
            0,
            "cent->nextState.index.primaryLight doesn't index Com_GetPrimaryLightCount()\n\t%i not in [0, %i)",
            *(unsigned __int16 *)cent->nextState.index,
            PrimaryLightCount);
    }
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v5 = *(unsigned __int16 *)cent->nextState.index;
    p_currentState = &cent->currentState;
    p_lerp = &cent->nextState.lerp;
    v8 = (GfxLight *)((char *)cgArray[0].refdef.primaryLights + __ROL4__(v5, 6));
    PrimaryLight = Com_GetPrimaryLight(v5);
    DWORD2(v10) = p_currentState->u.primaryLight.colorAndExp[1];
    LODWORD(v12) = p_lerp->u.primaryLight.colorAndExp[0];
    v11 = p_currentState->u.turret.gunAngles[1];
    DWORD1(v10) = p_currentState->u.primaryLight.colorAndExp[2];
    HIDWORD(v12) = p_lerp->u.primaryLight.colorAndExp[1];
    v13 = p_lerp->u.turret.gunAngles[1];
    LODWORD(v14) = p_lerp->u.primaryLight.colorAndExp[2];
    v15 = PrimaryLight;
    frameInterpolation = cgArray[0].frameInterpolation;
    LODWORD(v10) = p_currentState->u.primaryLight.colorAndExp[0];
    v17 = (float)(p_currentState->u.turret.gunAngles[1] * (float)((float)(__int64)v10 * (float)0.0039215689));
    v8->color[0] = (float)((float)((float)(p_lerp->u.turret.gunAngles[1] * (float)((float)v12 * (float)0.0039215689))
        - (float)v17)
        * cgArray[0].frameInterpolation)
        + (float)v17;
    v8->color[1] = (float)((float)((float)((float)((float)*(__int64 *)((char *)&v12 + 4) * (float)0.0039215689)
        * (float)v13)
        - (float)((float)((float)*(__int64 *)((char *)&v10 + 4) * (float)0.0039215689)
            * (float)v11))
        * (float)frameInterpolation)
        + (float)((float)((float)*(__int64 *)((char *)&v10 + 4) * (float)0.0039215689) * (float)v11);
    v18 = (float)((float)((float)*(__int64 *)((char *)&v12 - 4) * (float)0.0039215689) * (float)v11);
    v8->color[2] = (float)((float)((float)((float)((float)v14 * (float)0.0039215689) * (float)v13) - (float)v18)
        * (float)frameInterpolation)
        + (float)v18;
    if (PrimaryLight->rotationLimit < 1.0)
    {
        BG_EvaluateTrajectory(&p_lerp->apos, cgArray[0].time, v24);
        AngleVectors(v24, v8->dir, 0, 0);
        v8->dir[0] = -v8->dir[0];
        v8->dir[1] = -v8->dir[1];
        v8->dir[2] = -v8->dir[2];
        if (v15->rotationLimit > -1.0)
            CG_ClampPrimaryLightDir(v8, v15);
    }
    if (v15->translationLimit > 0.0)
    {
        BG_EvaluateTrajectory(&p_lerp->pos, cgArray[0].time, v8->origin);
        CG_ClampPrimaryLightOrigin(v8, v15);
    }
    v8->radius = (float)((float)(p_lerp->u.turret.gunAngles[2] - p_currentState->u.turret.gunAngles[2])
        * cgArray[0].frameInterpolation)
        + p_currentState->u.turret.gunAngles[2];
    v8->cosHalfFovOuter = (float)((float)(p_lerp->u.primaryLight.cosHalfFovOuter
        - p_currentState->u.primaryLight.cosHalfFovOuter)
        * cgArray[0].frameInterpolation)
        + p_currentState->u.primaryLight.cosHalfFovOuter;
    cosHalfFovOuter = v8->cosHalfFovOuter;
    v8->cosHalfFovInner = (float)((float)(p_lerp->u.primaryLight.cosHalfFovInner
        - p_currentState->u.primaryLight.cosHalfFovInner)
        * cgArray[0].frameInterpolation)
        + p_currentState->u.primaryLight.cosHalfFovInner;
    v20 = p_currentState->u.primaryLight.colorAndExp[3];
    v21 = p_lerp->u.primaryLight.colorAndExp[3] - v20;
    v8->exponent = (int)(float)((float)*(__int64 *)((char *)&v12 - 4) * cgArray[0].frameInterpolation) + v20;
    if (cosHalfFovOuter <= 0.0
        || (cosHalfFovInner = v8->cosHalfFovInner, cosHalfFovOuter >= cosHalfFovInner)
        || cosHalfFovInner > 1.0)
    {
        v23 = va(
            (const char *)HIDWORD(cosHalfFovOuter),
            LODWORD(cosHalfFovOuter),
            (unsigned int)COERCE_UNSIGNED_INT64(v8->cosHalfFovInner));
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
            1083,
            0,
            "%s\n\t%s",
            "0.0f < light->cosHalfFovOuter && light->cosHalfFovOuter < light->cosHalfFovInner && light->cosHalfFovInner <= 1.0f",
            v23);
    }
}

void __cdecl CG_InterpolateEntityOrigin(const cg_s *cgameGlob, centity_s *cent)
{
    double frameInterpolation; // fp31
    double v5; // fp13
    double v6; // fp10
    double v7; // fp12
    double v8; // fp9
    float v9; // [sp+50h] [-40h] BYREF
    float v10; // [sp+54h] [-3Ch]
    float v11; // [sp+58h] [-38h]
    float v12[4]; // [sp+60h] [-30h] BYREF

    if (!cgameGlob->nextSnap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1101, 0, "%s", "cgameGlob->nextSnap");
    frameInterpolation = cgameGlob->frameInterpolation;
    BG_EvaluateTrajectory(&cent->currentState.pos, cgameGlob->snap->serverTime, &v9);
    BG_EvaluateTrajectory(&cent->nextState.lerp.pos, cgameGlob->nextSnap->serverTime, v12);
    v5 = v10;
    v6 = (float)(v12[1] - v10);
    v7 = v11;
    v8 = (float)(v12[2] - v11);
    cent->pose.origin[0] = (float)((float)(v12[0] - v9) * (float)frameInterpolation) + v9;
    cent->pose.origin[1] = (float)((float)v6 * (float)frameInterpolation) + (float)v5;
    cent->pose.origin[2] = (float)((float)v8 * (float)frameInterpolation) + (float)v7;
}

void __cdecl CG_InterpolateEntityAngles(const cg_s *cgameGlob, centity_s *cent)
{
    double frameInterpolation; // fp31
    float v5[4]; // [sp+50h] [-40h] BYREF
    float v6[4]; // [sp+60h] [-30h] BYREF

    if (!cgameGlob->nextSnap)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1121, 0, "%s", "cgameGlob->nextSnap");
    frameInterpolation = cgameGlob->frameInterpolation;
    BG_EvaluateTrajectory(&cent->currentState.apos, cgameGlob->snap->serverTime, v6);
    BG_EvaluateTrajectory(&cent->nextState.lerp.apos, cgameGlob->nextSnap->serverTime, v5);
    cent->pose.angles[0] = LerpAngle(v6[0], v5[0], frameInterpolation);
    cent->pose.angles[1] = LerpAngle(v6[1], v5[1], frameInterpolation);
    cent->pose.angles[2] = LerpAngle(v6[2], v5[2], frameInterpolation);
}

void __cdecl CG_CreatePhysicsObject(int localClientNum, centity_s *cent)
{
    const DObj_s *ClientDObj; // r29
    PhysPreset *PhysPreset; // r28
    const char *v6; // r3
    int v7; // r30
    const char *Name; // r3
    double v9; // fp0
    double v10; // fp1
    double v13; // fp13
    float v14[4]; // [sp+50h] [-70h] BYREF
    float v15[4]; // [sp+60h] [-60h] BYREF
    float v16[4]; // [sp+70h] [-50h] BYREF
    float v17[16]; // [sp+80h] [-40h] BYREF

    v14[0] = 0.0;
    v14[1] = 0.0;
    v14[2] = 0.0;
    v15[0] = cent->currentState.pos.trBase[0];
    v15[1] = cent->currentState.pos.trBase[1];
    v15[2] = cent->currentState.pos.trBase[2];
    AnglesToQuat(cent->currentState.apos.trBase, v17);
    ClientDObj = Com_GetClientDObj(cent->nextState.number, localClientNum);
    if (!ClientDObj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1151, 0, "%s", "obj");
    PhysPreset = DObjGetPhysPreset(ClientDObj);
    if (PhysPreset)
    {
        Sys_EnterCriticalSection(CRITSECT_PHYSICS);
        v7 = Phys_ObjCreate(PHYS_WORLD_FX, v15, v17, v14, PhysPreset);
        if (v7)
        {
            DObjPhysicsSetCollisionFromXModel(ClientDObj, PHYS_WORLD_FX, v7);
            v9 = cent->currentState.apos.trDelta[2];
            v10 = __fsqrts((float)((float)(cent->currentState.apos.trDelta[0] * cent->currentState.apos.trDelta[0])
                + (float)((float)(cent->currentState.apos.trDelta[2] * cent->currentState.apos.trDelta[2])
                    + (float)(cent->currentState.apos.trDelta[1] * cent->currentState.apos.trDelta[1]))));
            _FP10 = -v10;
            __asm { fsel      f10, f10, f11, f1 }
            v13 = (float)(cent->currentState.apos.trDelta[1] * (float)((float)1.0 / (float)_FP10));
            v16[0] = (float)((float)1.0 / (float)_FP10) * cent->currentState.apos.trDelta[0];
            v16[1] = v13;
            v16[2] = (float)v9 * (float)((float)1.0 / (float)_FP10);
            Phys_ObjBulletImpact(PHYS_WORLD_FX, v7, cent->currentState.pos.trDelta, v16, v10, PhysPreset->bulletForceScale);
            Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
            cent->pose.physObjId = v7;
        }
        else
        {
            Name = DObjGetName(ClientDObj);
            Com_PrintWarning(1, "Failed to create physics object for '%s'.\n", Name);
            cent->pose.physObjId = -1;
            Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
        }
    }
    else
    {
        cent->pose.physObjId = -1;
        v6 = DObjGetName(ClientDObj);
        Com_PrintWarning(1, "Failed to create physics object for '%s'.  No physics preset.\n", v6);
    }
}

void __cdecl CG_UpdatePhysicsPose(centity_s *cent)
{
    int physObjId; // r11
    float v3[4]; // [sp+50h] [-20h] BYREF

    physObjId = cent->pose.physObjId;
    if (!physObjId || physObjId == -1)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
            1195,
            0,
            "%s",
            "cent->pose.physObjId != PHYS_OBJ_ID_NULL && cent->pose.physObjId != PHYS_OBJ_ID_DEAD");
    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    Phys_ObjGetInterpolatedState(PHYS_WORLD_FX, cent->pose.physObjId, cent->pose.origin, v3);
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
    UnitQuatToAngles(v3, cent->pose.angles);
}

int __cdecl CG_ExpiredLaunch(int localClientNum, centity_s *cent)
{
    int result; // r3

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cent->pose.physObjId || cgArray[0].time <= cent->currentState.pos.trTime + 1000)
        return 0;
    result = 1;
    cent->pose.physObjId = -1;
    return result;
}

void __cdecl CG_CalcEntityPhysicsPositions(int localClientNum, centity_s *cent)
{
    if (!cent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1226, 0, "%s", "cent");
    if (cent->currentState.pos.trType != TR_PHYSICS || cent->currentState.apos.trType != TR_PHYSICS)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
            1227,
            0,
            "%s",
            "cent->currentState.pos.trType == TR_PHYSICS && cent->currentState.apos.trType == TR_PHYSICS");
    if (Com_GetClientDObj(cent->nextState.number, localClientNum)
        && !(unsigned __int8)CG_ExpiredLaunch(localClientNum, cent))
    {
        if (!cent->pose.physObjId)
            CG_CreatePhysicsObject(localClientNum, cent);
        if (cent->pose.physObjId == -1)
        {
            cent->pose.origin[0] = cent->currentState.pos.trBase[0];
            cent->pose.origin[1] = cent->currentState.pos.trBase[1];
            cent->pose.origin[2] = cent->currentState.pos.trBase[2];
            cent->pose.angles[0] = cent->currentState.apos.trBase[0];
            cent->pose.angles[1] = cent->currentState.apos.trBase[1];
            cent->pose.angles[2] = cent->currentState.apos.trBase[2];
        }
        else
        {
            CG_UpdatePhysicsPose(cent);
        }
    }
}

void __cdecl CG_SaveEntityPhysics(centity_s *cent, SaveGame *save)
{
    const DObj_s *ClientDObj; // r28
    int physObjId; // r11
    char v6; // r11
    bool v7; // zf
    MemoryFile *MemoryFile; // r3
    const char *Name; // r29
    MemoryFile *v10; // r3
    char v11; // [sp+50h] [-40h] BYREF

    if (!cent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1254, 0, "%s", "cent");
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1255, 0, "%s", "save");
    ClientDObj = Com_GetClientDObj(cent->nextState.number, 0);
    if (!ClientDObj || (physObjId = cent->pose.physObjId) == 0 || (v7 = physObjId != -1, v6 = 1, !v7))
        v6 = 0;
    v11 = v6;
    SaveMemory_SaveWrite(&v11, 1, save);
    if (v11)
    {
        MemoryFile = SaveMemory_GetMemoryFile(save);
        Phys_ObjSave(cent->pose.physObjId, MemoryFile);
        if (!ClientDObj)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1268, 0, "%s", "obj");
        Name = DObjGetName(ClientDObj);
        if (!Name)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1270, 0, "%s", "modelName");
        v10 = SaveMemory_GetMemoryFile(save);
        MemFile_WriteCString(v10, Name);
    }
}

void __cdecl CG_LoadEntityPhysics(centity_s *cent, SaveGame *save)
{
    MemoryFile *MemoryFile; // r3
    MemoryFile *v5; // r3
    const char *CString; // r3
    const XModel *v7; // r29
    char v8; // [sp+50h] [-30h] BYREF

    if (!cent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1280, 0, "%s", "cent");
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1281, 0, "%s", "save");
    v8 = 0;
    SaveMemory_LoadRead(&v8, 1, save);
    if (v8)
    {
        MemoryFile = SaveMemory_GetMemoryFile(save);
        cent->pose.physObjId = Phys_ObjLoad(PHYS_WORLD_FX, MemoryFile);
        v5 = SaveMemory_GetMemoryFile(save);
        CString = MemFile_ReadCString(v5);
        v7 = R_RegisterModel(CString);
        if (!v7)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1295, 0, "%s", "xmodel");
        Phys_ObjSetCollisionFromXModel(v7, PHYS_WORLD_FX, cent->pose.physObjId);
    }
}

void __cdecl CG_CreateRagdollObject(int localClientNum, centity_s *cent)
{
    cent->pose.ragdollHandle = Ragdoll_CreateRagdollForDObj(localClientNum, 0, cent->nextState.number, 0, 1);
    cent->pose.isRagdoll = 1;
}

void __cdecl CG_UpdateRagdollPose(centity_s *cent)
{
    if (!cent->pose.ragdollHandle)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
            1314,
            0,
            "%s",
            "cent->pose.ragdollHandle != RAGDOLL_INVALID");
    Ragdoll_GetRootOrigin(cent->pose.ragdollHandle, cent->pose.origin);
}

void __cdecl CG_CalcEntityRagdollPositions(int localClientNum, centity_s *cent)
{
    if (!cent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1321, 0, "%s", "cent");
    if (!(unsigned __int8)Com_IsRagdollTrajectory(&cent->currentState.pos)
        && !(unsigned __int8)Com_IsRagdollTrajectory(&cent->currentState.apos))
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
            1322,
            0,
            "%s",
            "Com_IsRagdollTrajectory( &cent->currentState.pos ) || Com_IsRagdollTrajectory( &cent->currentState.apos )");
    }
    if (!cent->pose.ragdollHandle)
    {
        cent->pose.ragdollHandle = Ragdoll_CreateRagdollForDObj(localClientNum, 0, cent->nextState.number, 0, 1);
        cent->pose.isRagdoll = 1;
    }
    if (cent->pose.ragdollHandle)
        CG_UpdateRagdollPose(cent);
}

void __cdecl CG_CalcEntityLerpPositions(int localClientNum, centity_s *cent)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cent->currentState.pos.trType == TR_PHYSICS)
    {
        CG_CalcEntityPhysicsPositions(localClientNum, cent);
    }
    else if ((unsigned __int8)Com_IsRagdollTrajectory(&cent->currentState.pos))
    {
        CG_InterpolateEntityOrigin(cgArray, cent);
        CG_CalcEntityRagdollPositions(localClientNum, cent);
    }
    else
    {
        CG_InterpolateEntityOrigin(cgArray, cent);
        if (cent->currentState.pos.trType != TR_INTERPOLATE && cent != &cgArray[0].predictedPlayerEntity)
            CG_AdjustPositionForMover(
                localClientNum,
                cent->pose.origin,
                cent->nextState.groundEntityNum,
                cgArray[0].snap->serverTime,
                cgArray[0].time,
                cent->pose.origin,
                0);
        if (cent->currentState.apos.trType == TR_INTERPOLATE)
            CG_InterpolateEntityAngles(cgArray, cent);
        else
            BG_EvaluateTrajectory(&cent->currentState.apos, cgArray[0].time, cent->pose.angles);
    }
}

void __cdecl CG_DObjCalcBone(const cpose_t *pose, DObj_s *obj, int boneIndex)
{
    const DObj_s *v6; // r3
    bool v7; // zf
    int v8[16]; // [sp+50h] [-40h] BYREF

    if (!obj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1384, 0, "%s", "obj");
    if (!pose)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1385, 0, "%s", "pose");
    DObjLock(obj);
    v7 = CL_DObjCreateSkelForBone(obj, boneIndex) != 0;
    v6 = obj;
    if (!v7)
    {
        DObjGetHierarchyBits(obj, boneIndex, v8);
        CG_DoControllers(pose, obj, v8);
        DObjCalcSkel(obj, v8);
        v6 = obj;
    }
    DObjUnlock((DObj_s *)v6);
}

void __cdecl CG_DrawEntEqDebug(const centity_s *cent)
{
    int eType; // r3
    int number; // r30
    const char *EntityTypeName; // r3
    int v5; // r7
    const char *v6; // r6
    const float *v7; // r5

    if (snd_drawEqEnts->current.enabled)
    {
        eType = cent->nextState.eType;
        switch (cent->nextState.eType)
        {
        case 3u:
        case 6u:
        case 0xAu:
        case 0xEu:
            number = cent->nextState.number;
            EntityTypeName = BG_GetEntityTypeName(eType);
            v6 = va("%s: #%i", EntityTypeName, number);
            if ((cent->currentState.eFlags & 0x200000) != 0)
                v7 = colorRed;
            else
                v7 = colorLtGreen;
            CG_DebugStarWithText(cent->pose.origin, v7, v7, v6, 1.0, v5);
            break;
        default:
            return;
        }
    }
}

void __cdecl CG_ClearUnion(int localClientNum, centity_s *cent)
{
    FxEffect *effect; // r4
    $51809EA76892896F64281DFB626CE797 *v4; // r11
    int v5; // ctr

    switch (cent->pose.eTypeUnion)
    {
    case 7u:
    case 8u:
        effect = cent->pose.fx.effect;
        if (effect)
            FX_ThroughWithEffect(localClientNum, effect);
        cent->pose.actor.proneType = 0;
        cent->pose.fx.effect = 0;
        break;
    case 0xAu:
        cent->pose.actor.proneType = 0;
        *(_QWORD *)&cent->pose.fx.effect = 0;
        cent->pose.actor.height = 0.0;
        break;
    case 0xBu:
    case 0xDu:
        v4 = &cent->pose.44;
        v5 = 10;
        do
        {
            v4->actor.proneType = 0;
            v4 = ($51809EA76892896F64281DFB626CE797 *)((char *)v4 + 4);
            --v5;
        } while (v5);
        break;
    case 0xEu:
    case 0x10u:
        cent->pose.actor.proneType = 0;
        *(_QWORD *)&cent->pose.fx.effect = 0;
        cent->pose.actor.height = 0.0;
        break;
    default:
        break;
    }
    cent->pose.eTypeUnion = 0;
}

void __cdecl CG_SetUnionType(int localClientNum, centity_s *cent)
{
    unsigned __int8 eType; // r11

    eType = cent->nextState.eType;
    switch (eType)
    {
    case 7u:
    case 8u:
    case 0xAu:
    case 0xBu:
    case 0xDu:
    case 0xEu:
    case 0x10u:
        break;
    default:
        eType = 0;
        break;
    }
    cent->pose.eTypeUnion = eType;
}

void __cdecl CG_UpdatePoseUnion(int localClientNum, centity_s *cent)
{
    unsigned __int8 eType; // r11

    CG_ClearUnion(localClientNum, cent);
    eType = cent->nextState.eType;
    switch (eType)
    {
    case 7u:
    case 8u:
    case 0xAu:
    case 0xBu:
    case 0xDu:
    case 0xEu:
    case 0x10u:
        break;
    default:
        eType = 0;
        break;
    }
    cent->pose.eTypeUnion = eType;
}

void __cdecl CG_ProcessEntity(int localClientNum, centity_s *cent)
{
    const float *v4; // r6
    int v5; // r4
    int eType; // r5

    if (cent->nextState.loopSound)
        CG_AddEntityLoopSound(localClientNum, cent);
    if (cent->nextState.eType != cent->pose.eTypeUnion)
        CG_UpdatePoseUnion(localClientNum, cent);
    CG_DrawEntEqDebug(cent);
    eType = cent->nextState.eType;
    switch (cent->nextState.eType)
    {
    case 0u:
        CG_General(localClientNum, cent);
        break;
    case 1u:
    case 4u:
        return;
    case 2u:
        CG_Item(cent);
        break;
    case 3u:
        CG_Missile(localClientNum, cent);
        break;
    case 5u:
        CG_ScriptMover(localClientNum, cent);
        break;
    case 6u:
        CG_SoundBlend(localClientNum, cent);
        break;
    case 7u:
        CG_Fx(localClientNum, cent);
        break;
    case 8u:
        CG_LoopFx(localClientNum, cent);
        break;
    case 9u:
        CG_PrimaryLight(localClientNum, cent);
        break;
    case 0xAu:
        CG_mg42(localClientNum, cent);
        break;
    case 0xBu:
    case 0xDu:
        CG_Vehicle(localClientNum, cent);
        break;
    case 0xEu:
    case 0x10u:
        CG_Actor(localClientNum, cent);
        break;
    case 0xFu:
        CG_ActorSpawner(cent, v5, eType, v4);
        break;
    default:
        Com_Error(ERR_DROP, "Bad entity type %i", eType);
        break;
    }
}

void __cdecl CG_SaveEntityFX(centity_s *cent, SaveGame *save)
{
    int eType; // r11
    int v5; // r11
    bool v6; // zf
    FxEffect *effect; // r4
    int v8; // [sp+50h] [-20h] BYREF
    int ClientEffectIndex; // [sp+54h] [-1Ch] BYREF

    eType = cent->nextState.eType;
    if (eType == 7 || (v6 = eType != 8, v5 = 0, !v6))
        v5 = 1;
    v8 = v5;
    SaveMemory_SaveWrite(&v8, 4, save);
    if (v8)
    {
        SaveMemory_SaveWrite(&cent->pose.44, 4, save);
        effect = cent->pose.fx.effect;
        if (effect)
            ClientEffectIndex = FX_GetClientEffectIndex(0, effect);
        else
            ClientEffectIndex = -1;
        SaveMemory_SaveWrite(&ClientEffectIndex, 4, save);
    }
}

void __cdecl CG_LoadEntityFX(centity_s *cent, SaveGame *save)
{
    int v4; // [sp+50h] [-20h] BYREF
    int v5; // [sp+54h] [-1Ch] BYREF

    SaveMemory_LoadRead(&v4, 4, save);
    if (v4)
    {
        SaveMemory_LoadRead(&cent->pose.44, 4, save);
        SaveMemory_LoadRead(&v5, 4, save);
        if (v5 == -1)
            cent->pose.fx.effect = 0;
        else
            cent->pose.fx.effect = FX_GetClientEffectByIndex(0, v5);
    }
}

void __cdecl CG_SaveEntity(unsigned int entnum, SaveGame *save)
{
    centity_s *v4; // r31

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1628, 0, "%s", "save");
    if (entnum >= 0x880)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
            1629,
            0,
            "entnum doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
            entnum,
            2176);
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            932,
            0,
            "entityIndex doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
            entnum,
            2176);
    }
    v4 = &cg_entitiesArray[0][entnum];
    if (!v4->nextValid)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1632, 0, "%s", "cent->nextValid");
    SaveMemory_SaveWrite(&v4->previousEventSequence, 4, save);
    CG_SaveEntityFX(v4, save);
    CG_SaveEntityPhysics(v4, save);
}

void __cdecl CG_LoadEntity(unsigned int entnum, SaveGame *save)
{
    centity_s *v4; // r31

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1648, 0, "%s", "save");
    if (entnum >= 0x880)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp",
            1649,
            0,
            "entnum doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
            entnum,
            2176);
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            932,
            0,
            "entityIndex doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
            entnum,
            2176);
    }
    v4 = &cg_entitiesArray[0][entnum];
    SaveMemory_LoadRead(&v4->previousEventSequence, 4, save);
    CG_LoadEntityFX(v4, save);
    CG_LoadEntityPhysics(v4, save);
}

void __cdecl CG_SaveEntities(SaveGame *save)
{
    int v1; // r7
    int v3; // r31
    unsigned int v4[16]; // [sp+50h] [-40h] BYREF

    v1 = 0;
    v4[0] = 0;
    do
    {
        v3 = v1;
        if ((unsigned int)v1 >= 0x880)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
                932,
                0,
                "entityIndex doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
                v1,
                2176);
            v1 = v4[0];
        }
        if (cg_entitiesArray[0][v3].nextValid)
        {
            SaveMemory_SaveWrite(v4, 4, save);
            CG_SaveEntity(v4[0], save);
            v1 = v4[0];
        }
        v4[0] = ++v1;
    } while (v1 < 2176);
    v4[0] = -1;
    SaveMemory_SaveWrite(v4, 4, save);
}

void __cdecl CG_LoadEntities(SaveGame *save)
{
    signed int i; // r5
    unsigned int v3[2]; // [sp+50h] [-20h] BYREF

    SaveMemory_LoadRead(v3, 4, save);
    for (i = v3[0]; v3[0] >= 0; i = v3[0])
    {
        if (i >= 2176)
        {
            Com_Error(ERR_DROP, "G_LoadGame: entitynum out of range (%i, MAX = %i)", i, 2176);
            i = v3[0];
        }
        CG_LoadEntity(i, save);
        SaveMemory_LoadRead(v3, 4, save);
    }
}

void __cdecl CG_GetPoseOrigin(const cpose_t *pose, float *origin)
{
    if (!pose)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1699, 0, "%s", "pose");
    *origin = pose->origin[0];
    origin[1] = pose->origin[1];
    origin[2] = pose->origin[2];
}

void __cdecl CG_GetPoseAngles(const cpose_t *pose, float *angles)
{
    if (!pose)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1706, 0, "%s", "pose");
    *angles = pose->angles[0];
    angles[1] = pose->angles[1];
    angles[2] = pose->angles[2];
}

float *__cdecl CG_GetEntityOrigin(int localClientNum, int entnum)
{
    return CG_GetEntity(localClientNum, entnum)->pose.origin;
}

void __cdecl CG_GetPoseLightingHandle(const cpose_t *pose)
{
    ;
}

void __cdecl CG_PredictiveSkinCEntity(GfxSceneEntity *sceneEnt)
{
    GfxSceneEntityInfo v2; // r11
    int cullIn; // r10

    if (!sceneEnt)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 1730, 0, "%s", "sceneEnt");
    if (!cl_freemove->current.integer)
    {
        v2.pose = (const cpose_t *)sceneEnt->info;
        cullIn = v2.pose->cullIn;
        if (cullIn == 1)
        {
            v2.pose->cullIn = 0;
            R_UpdateXModelBoundsDelayed(sceneEnt);
        }
        else if (cullIn == 2)
        {
            v2.pose->cullIn = 0;
            R_SkinGfxEntityDelayed(sceneEnt);
        }
    }
}

void __cdecl CG_AddPacketEntity(unsigned int localClientNum, unsigned int entnum)
{
    centity_s *Entity; // r3
    centity_s *v5; // r27
    float *origin; // r31
    double v7; // fp28
    double v8; // fp27
    double v9; // fp31
    double v10; // fp30
    double v11; // fp29
    double v12; // fp26
    char v13; // r11
    unsigned __int8 v14; // r28
    char v15; // r9
    float *v16; // r11
    double v17; // fp0
    double v18; // fp13
    const DObj_s *ClientDObj; // r3
    double Radius; // fp1
    int v21; // r11

    Entity = CG_GetEntity(localClientNum, entnum);
    v5 = Entity;
    origin = Entity->pose.origin;
    v7 = Entity->pose.angles[0];
    v8 = Entity->pose.angles[1];
    v9 = Entity->pose.origin[0];
    v10 = Entity->pose.origin[1];
    v11 = Entity->pose.origin[2];
    v12 = Entity->pose.angles[2];
    if (Entity->nextState.eType == 5 && Entity->nextState.solid == 0xFFFFFF)
    {
        CG_CalcEntityLerpPositions(localClientNum, Entity);
        if (*origin != v9
            || origin[1] != v10
            || origin[2] != v11
            || v5->pose.angles[0] != v7
            || v5->pose.angles[1] != v8
            || (v13 = 0, v5->pose.angles[2] != v12))
        {
            v13 = 1;
        }
        v14 = v13;
        if (v13)
            CG_UpdateBModelWorldBounds(localClientNum, v5, 0);
    }
    else
    {
        CG_CalcEntityLerpPositions(localClientNum, Entity);
        if (*origin != v9
            || origin[1] != v10
            || origin[2] != v11
            || v5->pose.angles[0] != v7
            || v5->pose.angles[1] != v8
            || (v15 = 0, v5->pose.angles[2] != v12))
        {
            v15 = 1;
        }
        v14 = v15;
        v16 = cg_entityOriginArray[localClientNum][entnum];
        v17 = (float)(v16[1] - origin[1]);
        v18 = (float)(v16[2] - origin[2]);
        if ((float)((float)((float)(*v16 - *origin) * (float)(*v16 - *origin))
            + (float)((float)((float)v18 * (float)v18) + (float)((float)v17 * (float)v17))) > 256.0)
        {
            *v16 = *origin;
            v16[1] = origin[1];
            v16[2] = origin[2];
            ClientDObj = Com_GetClientDObj(entnum, localClientNum);
            if (ClientDObj)
            {
                Radius = DObjGetRadius(ClientDObj);
                R_LinkDObjEntity(localClientNum, entnum, origin, (float)((float)Radius + (float)16.0));
            }
        }
    }
    if (v5->pose.physObjId == -1)
    {
        if (CG_IsEntityLinked(localClientNum, entnum))
            CG_UnlinkEntity(localClientNum, entnum);
    }
    else
    {
        if (CG_IsEntityLinked(localClientNum, entnum))
            v21 = v14;
        else
            v21 = CG_EntityNeedsLinked(localClientNum, entnum);
        if (v21)
            CG_LinkEntity(localClientNum, entnum);
        CG_ProcessEntity(localClientNum, v5);
    }
}

int __cdecl CG_AddPacketEntities(int localClientNum)
{
    int viewlocked_entNum; // r24
    int v3; // r25
    snapshot_s *nextSnap; // r11
    int v5; // r29
    int v6; // r30
    unsigned int v7; // r31
    centity_s *Entity; // r4

    //PIXBeginNamedEvent_Copy_NoVarArgs(0xFFFFFFFF, "add packet ents");
    //Profile_Begin(325);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgArray[0].rumbleScale = 0.0;
    if ((cgArray[0].predictedPlayerState.eFlags & 0x300) != 0)
        viewlocked_entNum = cgArray[0].predictedPlayerState.viewlocked_entNum;
    else
        viewlocked_entNum = 2175;
    v3 = 0;
    nextSnap = cgArray[0].nextSnap;
    v5 = 0;
    if (cgArray[0].nextSnap->numEntities > 0)
    {
        v6 = 45796;
        do
        {
            v7 = *(int *)((char *)&nextSnap->snapFlags + v6);
            Entity = CG_GetEntity(localClientNum, v7);
            if (Entity->nextState.eType < 0x11u)
            {
                if (v7 == viewlocked_entNum)
                {
                    v3 = 1;
                    CG_CalcEntityLerpPositions(localClientNum, Entity);
                }
                else
                {
                    CG_AddPacketEntity(localClientNum, v7);
                }
            }
            nextSnap = cgArray[0].nextSnap;
            ++v5;
            v6 += 4;
        } while (v5 < cgArray[0].nextSnap->numEntities);
    }
    //Profile_EndInternal(0);
    //PIXEndNamedEvent();
    return v3;
}

DObjAnimMat *__cdecl CG_DObjGetLocalBoneMatrix(const cpose_t *pose, DObj_s *obj, int boneIndex)
{
    DObjAnimMat *RotTransArray; // r3

    if (!obj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 640, 0, "%s", "obj");
    //Profile_Begin(319);
    CG_DObjCalcBone(pose, obj, boneIndex);
    //Profile_EndInternal(0);
    RotTransArray = DObjGetRotTransArray(obj);
    if (RotTransArray)
        return &RotTransArray[boneIndex];
    else
        return 0;
}

DObjAnimMat *__cdecl CG_DObjGetLocalTagMatrix(const cpose_t *pose, DObj_s *obj, unsigned int tagName)
{
    DObjAnimMat *result; // r3
    unsigned __int8 v7; // [sp+50h] [-30h] BYREF

    if (!obj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 661, 0, "%s", "obj");
    v7 = -2;
    result = (DObjAnimMat *)DObjGetBoneIndex(obj, tagName, &v7);
    if (result)
        return CG_DObjGetLocalBoneMatrix(pose, obj, v7);
    return result;
}

DObjAnimMat *__cdecl CG_DObjGetWorldBoneMatrix(
    const cpose_t *pose,
    DObj_s *obj,
    int boneIndex,
    float (*tagMat)[3],
    float *origin)
{
    DObjAnimMat *result; // r3
    float *v11; // r31

    if (!obj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 682, 0, "%s", "obj");
    if (!pose)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 683, 0, "%s", "pose");
    result = CG_DObjGetLocalBoneMatrix(pose, obj, boneIndex);
    v11 = (float *)result;
    if (result)
    {
        LocalConvertQuatToMat(result, tagMat);
        result = (DObjAnimMat *)1;
        *origin = v11[4] + cgArray[0].refdef.viewOffset[0];
        origin[1] = v11[5] + cgArray[0].refdef.viewOffset[1];
        origin[2] = v11[6] + cgArray[0].refdef.viewOffset[2];
    }
    return result;
}

DObjAnimMat *__cdecl CG_DObjGetWorldTagMatrix(
    const cpose_t *pose,
    DObj_s *obj,
    unsigned int tagName,
    float (*tagMat)[3],
    float *origin)
{
    DObjAnimMat *result; // r3
    float *v11; // r31

    if (!obj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 707, 0, "%s", "obj");
    if (!pose)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 708, 0, "%s", "pose");
    result = CG_DObjGetLocalTagMatrix(pose, obj, tagName);
    v11 = (float *)result;
    if (result)
    {
        LocalConvertQuatToMat(result, tagMat);
        result = (DObjAnimMat *)1;
        *origin = v11[4] + cgArray[0].refdef.viewOffset[0];
        origin[1] = v11[5] + cgArray[0].refdef.viewOffset[1];
        origin[2] = v11[6] + cgArray[0].refdef.viewOffset[2];
    }
    return result;
}

int __cdecl CG_DObjGetWorldTagPos(const cpose_t *pose, DObj_s *obj, unsigned int tagName, float *pos)
{
    int result; // r3
    float *v9; // r11

    if (!obj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 732, 0, "%s", "obj");
    if (!pose)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_ents.cpp", 733, 0, "%s", "pose");
    result = (int)CG_DObjGetLocalTagMatrix(pose, obj, tagName);
    v9 = (float *)result;
    if (result)
    {
        result = 1;
        *pos = v9[4] + cgArray[0].refdef.viewOffset[0];
        pos[1] = v9[5] + cgArray[0].refdef.viewOffset[1];
        pos[2] = v9[6] + cgArray[0].refdef.viewOffset[2];
    }
    return result;
}

