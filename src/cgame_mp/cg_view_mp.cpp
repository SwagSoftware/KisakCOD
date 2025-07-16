#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "cg_local_mp.h"
#include "cg_public_mp.h"

#include <client_mp/client_mp.h>

#include <EffectsCore/fx_system.h>

#include <qcommon/mem_track.h>
#include <qcommon/cmd.h>

#include <script/scr_const.h>
#include <gfx_d3d/r_dpvs.h>
#include <qcommon/threads.h>
#include <client/client.h>
#include <aim_assist/aim_assist.h>
#include <gfx_d3d/r_workercmds_common.h>
#include <universal/profile.h>
#include <game_mp/g_main_mp.h>

TestEffect s_testEffect[1];
ClientViewParams clientViewParamsArray[1][1];

const dvar_t *cg_heliKillCamDist;
const dvar_t *cg_heliKillCamFov;
const dvar_t *cg_heliKillCamZDist;
const dvar_t *cg_heliKillCamNearBlur;
const dvar_t *cg_heliKillCamFarBlur;
const dvar_t *cg_heliKillCamFarBlurStart;
const dvar_t *cg_heliKillCamFarBlurDist;
const dvar_t *cg_heliKillCamNearBlurStart;
const dvar_t *cg_heliKillCamNearBlurEnd;
const dvar_t *cg_airstrikeKillCamFov;
const dvar_t *cg_airstrikeKillCamDist;
const dvar_t *cg_airstrikeKillCamCloseXYDist;
const dvar_t *cg_airstrikeKillCamCloseZDist;
const dvar_t *cg_airstrikeKillCamNearBlur;
const dvar_t *cg_airstrikeKillCamFarBlur;
const dvar_t *cg_airstrikeKillCamFarBlurStart;
const dvar_t *cg_airstrikeKillCamFarBlurDist;
const dvar_t *cg_airstrikeKillCamNearBlurStart;
const dvar_t *cg_airstrikeKillCamNearBlurEnd;

void __cdecl TRACK_cg_view()
{
    track_static_alloc_internal(s_testEffect, 84, "s_testEffect", 9);
    track_static_alloc_internal(clientViewParamsArray, 16, "clientViewParamsArray", 10);
}

void __cdecl CG_FxSetTestPosition()
{
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(0);

    if (cgameGlob->nextSnap)
    {
        Vec3Mad(cgameGlob->refdef.vieworg, 100.0, cgameGlob->refdef.viewaxis[0], s_testEffect[0].pos);
        Com_Printf(
            21,
            "\n\nFX Testing position set to: (%f, %f, %f)\n\n",
            s_testEffect[0].pos[0],
            s_testEffect[0].pos[1],
            s_testEffect[0].pos[2]);
    }
}

void __cdecl CG_FxTest()
{
    char *v0; // eax
    const char *v1; // eax

    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(0);

    if (cgameGlob->nextSnap)
    {
        if (Cmd_Argc() < 2)
            Com_Printf(21, "Must supply filename from base path.  Optional restart time.\n");
        v0 = (char *)Cmd_Argv(1);
        I_strncpyz(s_testEffect[0].name, v0, 64);
        if (I_strncmp(s_testEffect[0].name, "fx/", 3))
        {
            Com_Printf(21, "Spawning Fx %s\n", s_testEffect[0].name);
            CG_PlayTestFx(0);
            if (Cmd_Argc() == 3)
            {
                v1 = Cmd_Argv(2);
                s_testEffect[0].respawnTime = (int)(atof(v1) * 1000.0);
            }
            else
            {
                s_testEffect[0].respawnTime = 0;
            }
        }
        else
        {
            Com_PrintError(1, "Fx path [%s] must not inclue \"fx/\" \n", s_testEffect[0].name);
        }
    }
}

void __cdecl CG_PlayTestFx(int32_t localClientNum)
{
    TestEffect *testEffect; // [esp+8h] [ebp-30h]
    const FxEffectDef *fxDef; // [esp+Ch] [ebp-2Ch]
    int32_t time; // [esp+10h] [ebp-28h]
    float axis[3][3]; // [esp+14h] [ebp-24h] BYREF
    const cg_s *cgameGlob;


    testEffect = &s_testEffect[localClientNum];
    fxDef = FX_Register(testEffect->name);
    axis[0][0] = 0.0;
    axis[0][1] = 0.0;
    axis[0][2] = 1.0;
    axis[1][0] = 1.0;
    axis[1][1] = 0.0;
    axis[1][2] = 0.0;
    axis[2][0] = 0.0;
    axis[2][1] = 1.0;
    axis[2][2] = 0.0;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    time = cgameGlob->time;
    FX_PlayOrientedEffect(localClientNum, fxDef, cgameGlob->time, testEffect->pos, axis);
    testEffect->time = time;
}

double __cdecl CG_GetViewFov(int32_t localClientNum)
{
    float v3; // [esp+Ch] [ebp-2Ch]
    float value; // [esp+10h] [ebp-28h]
    float weaponFov; // [esp+18h] [ebp-20h]
    float posLerp; // [esp+1Ch] [ebp-1Ch]
    int32_t weapIndex; // [esp+24h] [ebp-14h]
    WeaponDef *weapDef; // [esp+2Ch] [ebp-Ch]
    float zoomFrac; // [esp+30h] [ebp-8h]
    float viewFov; // [esp+34h] [ebp-4h]
    float viewFova; // [esp+34h] [ebp-4h]
    const cg_s *cgameGlob;

    iassert(cg_fov);
    iassert(cg_fovScale);
    iassert(cg_fovMin);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    weapDef = BG_GetWeaponDef(weapIndex);
    if (cgameGlob->predictedPlayerState.pm_type == PM_INTERMISSION)
    {
        viewFov = 90.0;
    }
    else
    {
        viewFov = cg_fov->current.value;
        if (viewFov < 1.0 || viewFov > 160.0)
            MyAssertHandler(
                ".\\cgame_mp\\cg_view_mp.cpp",
                539,
                1,
                "%s\n\t(viewFov) = %g",
                "(viewFov >= 1 && viewFov <= 160)",
                viewFov);
        if (BG_IsAimDownSightWeapon(weapIndex))
        {
            posLerp = cgameGlob->predictedPlayerState.fWeaponPosFrac;
            weaponFov = weapDef->fAdsZoomFov;
            if (posLerp == 1.0)
            {
                viewFov = weaponFov;
            }
            else if (posLerp != 0.0)
            {
                if (cgameGlob->playerEntity.bPositionToADS)
                {
                    zoomFrac = posLerp - (1.0 - weapDef->fAdsZoomInFrac);
                    if (zoomFrac > 0.0)
                        zoomFrac = zoomFrac / weapDef->fAdsZoomInFrac;
                }
                else
                {
                    zoomFrac = posLerp - (1.0 - weapDef->fAdsZoomOutFrac);
                    if (zoomFrac > 0.0)
                        zoomFrac = zoomFrac / weapDef->fAdsZoomOutFrac;
                }
                if (zoomFrac > 0.0)
                    viewFov = viewFov - (viewFov - weaponFov) * zoomFrac;
            }
        }
    }
    if ((cgameGlob->predictedPlayerState.eFlags & 0x300) != 0)
        viewFov = 55.0;
    viewFova = viewFov * cg_fovScale->current.value;
    value = cg_fovMin->current.value;
    v3 = value - viewFova;
    if (v3 < 0.0)
        return viewFova;
    else
        return value;
}

void __cdecl CG_ViewRegisterDvars()
{
    DvarLimits min; // [esp+4h] [ebp-10h]
    DvarLimits mina; // [esp+4h] [ebp-10h]
    DvarLimits minb; // [esp+4h] [ebp-10h]
    DvarLimits minc; // [esp+4h] [ebp-10h]
    DvarLimits mind; // [esp+4h] [ebp-10h]
    DvarLimits mine; // [esp+4h] [ebp-10h]
    DvarLimits minf; // [esp+4h] [ebp-10h]
    DvarLimits ming; // [esp+4h] [ebp-10h]
    DvarLimits minh; // [esp+4h] [ebp-10h]
    DvarLimits mini; // [esp+4h] [ebp-10h]
    DvarLimits minj; // [esp+4h] [ebp-10h]
    DvarLimits mink; // [esp+4h] [ebp-10h]
    DvarLimits minl; // [esp+4h] [ebp-10h]
    DvarLimits minm; // [esp+4h] [ebp-10h]
    DvarLimits minn; // [esp+4h] [ebp-10h]
    DvarLimits mino; // [esp+4h] [ebp-10h]
    DvarLimits minp; // [esp+4h] [ebp-10h]
    DvarLimits minq; // [esp+4h] [ebp-10h]
    DvarLimits minr; // [esp+4h] [ebp-10h]

    min.value.max = FLT_MAX;
    min.value.min = 0.0;
    cg_heliKillCamDist = Dvar_RegisterFloat(
        "cg_heliKillCamDist",
        1000.0,
        min,
        DVAR_CHEAT,
        "Helicopter kill camera distance from helicopter.");
    mina.value.max = 160.0f;
    mina.value.min = 0.1f;
    cg_heliKillCamFov = Dvar_RegisterFloat(
        "cg_heliKillCamFov",
        15.0f,
        mina,
        DVAR_CHEAT,
        "Helicopter kill camera field of view.");
    minb.value.max = FLT_MAX;
    minb.value.min = 0.0f;
    cg_heliKillCamZDist = Dvar_RegisterFloat(
        "cg_heliKillCamZDist",
        50.0f,
        minb,
        DVAR_CHEAT,
        "Helicopter kill camera distance above the helicopter.");
    minc.value.max = 10.0f;
    minc.value.min = 4.0f;
    cg_heliKillCamNearBlur = Dvar_RegisterFloat(
        "cg_heliKillCamNearBlur",
        4.0f,
        minc,
        DVAR_CHEAT,
        "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
    mind.value.max = 10.0f;
    mind.value.min = 0.0f;
    cg_heliKillCamFarBlur = Dvar_RegisterFloat(
        "cg_heliKillCamFarBlur",
        2.0f,
        mind,
        DVAR_CHEAT,
        "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
    mine.value.max = FLT_MAX;
    mine.value.min = 0.0f;
    cg_heliKillCamFarBlurStart = Dvar_RegisterFloat(
        "cg_heliKillCamFarBlurStart",
        100.0f,
        mine,
        DVAR_CHEAT,
        "Helicopter kill camera distance above the helicopter.");
    minf.value.max = FLT_MAX;
    minf.value.min = 0.0f;
    cg_heliKillCamFarBlurDist = Dvar_RegisterFloat(
        "cg_heliKillCamFarBlurDist",
        300.0f,
        minf,
        DVAR_CHEAT,
        "Helicopter kill camera distance above the helicopter.");
    ming.value.max = FLT_MAX;
    ming.value.min = 0.0f;
    cg_heliKillCamNearBlurStart = Dvar_RegisterFloat(
        "cg_heliKillCamNearBlurStart",
        0.0f,
        ming,
        DVAR_CHEAT,
        "Helicopter kill camera distance above the helicopter.");
    minh.value.max = 10000.0f;
    minh.value.min = 0.0f;
    cg_heliKillCamNearBlurEnd = Dvar_RegisterFloat(
        "cg_heliKillCamNearBlurEnd",
        100.0f,
        minh,
        DVAR_CHEAT,
        "Helicopter kill camera distance above the helicopter.");
    mini.value.max = 160.0f;
    mini.value.min = 0.1f;
    cg_airstrikeKillCamFov = Dvar_RegisterFloat(
        "cg_airstrikeKillCamFov",
        80.0f,
        mini,
        DVAR_CHEAT,
        "Airstrike kill camera field of view.");
    minj.value.max = FLT_MAX;
    minj.value.min = 0.0f;
    cg_airstrikeKillCamDist = Dvar_RegisterFloat(
        "cg_airstrikeKillCamDist",
        200.0f,
        minj,
        DVAR_CHEAT,
        "Airstrike kill camera distance.");
    mink.value.max = FLT_MAX;
    mink.value.min = 0.0f;
    cg_airstrikeKillCamCloseXYDist = Dvar_RegisterFloat(
        "cg_airstrikeKillCamCloseXYDist",
        24.0f,
        mink,
        DVAR_CHEAT,
        "Airstrike kill camera closest distance in front of the bomb.");
    minl.value.max = FLT_MAX;
    minl.value.min = 0.0f;
    cg_airstrikeKillCamCloseZDist = Dvar_RegisterFloat(
        "cg_airstrikeKillCamCloseZDist",
        24.0f,
        minl,
        DVAR_CHEAT,
        "Airstrike kill camera closest distance above the target.");
    minm.value.max = 10.0f;
    minm.value.min = 4.0f;
    cg_airstrikeKillCamNearBlur = Dvar_RegisterFloat(
        "cg_airstrikeKillCamNearBlur",
        4.0f,
        minm,
        DVAR_CHEAT,
        "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
    minn.value.max = 10.0f;
    minn.value.min = 0.0f;
    cg_airstrikeKillCamFarBlur = Dvar_RegisterFloat(
        "cg_airstrikeKillCamFarBlur",
        2.0f,
        minn,
        DVAR_CHEAT,
        "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
    mino.value.max = FLT_MAX;
    mino.value.min = 0.0f;
    cg_airstrikeKillCamFarBlurStart = Dvar_RegisterFloat(
        "cg_airstrikeKillCamFarBlurStart",
        100.0f,
        mino,
        DVAR_CHEAT,
        "Airstrike kill camera distance above the airplane.");
    minp.value.max = FLT_MAX;
    minp.value.min = 0.0f;
    cg_airstrikeKillCamFarBlurDist = Dvar_RegisterFloat(
        "cg_airstrikeKillCamFarBlurDist",
        300.0f,
        minp,
        DVAR_CHEAT,
        "Airstrike kill camera distance above the airplane.");
    minq.value.max = FLT_MAX;
    minq.value.min = 0.0f;
    cg_airstrikeKillCamNearBlurStart = Dvar_RegisterFloat(
        "cg_airstrikeKillCamNearBlurStart",
        0.0f,
        minq,
        DVAR_CHEAT,
        "Airstrike kill camera distance above the airplane.");
    minr.value.max = 10000.0f;
    minr.value.min = 0.0f;
    cg_airstrikeKillCamNearBlurEnd = Dvar_RegisterFloat(
        "cg_airstrikeKillCamNearBlurEnd",
        100.0f,
        minr,
        DVAR_CHEAT,
        "Airstrike kill camera distance above the airplane.");
}

void __cdecl CG_UpdateHelicopterKillCam(int32_t localClientNum)
{
    float scale; // [esp+0h] [ebp-70h]
    float *v2; // [esp+18h] [ebp-58h]
    float *v3; // [esp+1Ch] [ebp-54h]
    float delta[3]; // [esp+3Ch] [ebp-34h] BYREF
    float origin[3]; // [esp+48h] [ebp-28h] BYREF
    float right[3]; // [esp+58h] [ebp-18h] BYREF
    float distance; // [esp+64h] [ebp-Ch]
    centity_s *centHelicopter; // [esp+68h] [ebp-8h]
    centity_s *centTarget; // [esp+6Ch] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cgameGlob->predictedPlayerState.killCamEntity == 1023)
        MyAssertHandler(
            ".\\cgame_mp\\cg_view_mp.cpp",
            941,
            0,
            "%s",
            "cgameGlob->predictedPlayerState.killCamEntity != ENTITYNUM_NONE");
    if (!cgameGlob->inKillCam)
        MyAssertHandler(".\\cgame_mp\\cg_view_mp.cpp", 942, 0, "%s", "cgameGlob->inKillCam");
    centHelicopter = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.killCamEntity);
    if (!centHelicopter->nextValid)
        MyAssertHandler(".\\cgame_mp\\cg_view_mp.cpp", 945, 0, "%s", "centHelicopter->nextValid");
    if (centHelicopter->pose.eType != 12)
        MyAssertHandler(".\\cgame_mp\\cg_view_mp.cpp", 946, 0, "%s", "centHelicopter->pose.eType == ET_HELICOPTER");
    centTarget = CG_GetEntity(localClientNum, cgameGlob->clientNum);
    if (centTarget->pose.eType != 1)
        MyAssertHandler(".\\cgame_mp\\cg_view_mp.cpp", 949, 0, "%s", "centTarget->pose.eType == ET_PLAYER");
    origin[0] = centHelicopter->pose.origin[0];
    origin[1] = centHelicopter->pose.origin[1];
    origin[2] = centHelicopter->pose.origin[2];
    origin[2] = origin[2] + cg_heliKillCamZDist->current.value;
    Vec3Sub(centTarget->pose.origin, origin, delta);
    distance = Vec3Normalize(delta);
    if (distance <= 0.0)
        MyAssertHandler(".\\cgame_mp\\cg_view_mp.cpp", 955, 0, "%s", "distance > 0.0f");
    Vec3Cross(up, delta, right);
    Vec3Normalize(right);
    v3 = cgameGlob->refdef.viewaxis[1];
    cgameGlob->refdef.viewaxis[1][0] = right[0];
    v3[1] = right[1];
    v3[2] = right[2];
    v2 = cgameGlob->refdef.viewaxis[0];
    cgameGlob->refdef.viewaxis[0][0] = delta[0];
    v2[1] = delta[1];
    v2[2] = delta[2];
    Vec3Cross(cgameGlob->refdef.viewaxis[0], cgameGlob->refdef.viewaxis[1], cgameGlob->refdef.viewaxis[2]);
    Vec3Normalize(cgameGlob->refdef.viewaxis[2]);
    scale = -cg_heliKillCamDist->current.value;
    Vec3Mad(origin, scale, delta, origin);
    Vec3Copy(origin, cgameGlob->refdef.vieworg);
    CG_UpdateHelicopterKillCamDof(distance, &cgameGlob->refdef.dof);
    AxisToAngles(cgameGlob->refdef.viewaxis, cgameGlob->refdefViewAngles);
    CG_UpdateFov(localClientNum, cg_heliKillCamFov->current.value);
}

void __cdecl CG_UpdateFov(int32_t localClientNum, float fov_x)
{
    float v2; // [esp+0h] [ebp-1Ch]
    float v3; // [esp+4h] [ebp-18h]
    float dxDz; // [esp+8h] [ebp-14h]
    float dyDz; // [esp+18h] [ebp-4h]
    cg_s *cgameGlob;
    cgs_t *cgs;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);

    v3 = fov_x * 0.01745329238474369 * 0.5;
    v2 = tan(v3);
    dyDz = v2 * 0.75;
    dxDz = dyDz * cgs->viewAspect;
    cgameGlob->refdef.tanHalfFovX = dxDz;
    cgameGlob->refdef.tanHalfFovY = dyDz;
    cgameGlob->zoomSensitivity = v2 / 0.6370702385902405;
}

void __cdecl CG_UpdateHelicopterKillCamDof(float distance, GfxDepthOfField *dof)
{
    dof->nearBlur = cg_heliKillCamNearBlur->current.value;
    dof->farBlur = cg_heliKillCamFarBlur->current.value;
    dof->nearStart = cg_heliKillCamNearBlurStart->current.value;
    dof->nearEnd = cg_heliKillCamDist->current.value + distance - cg_heliKillCamNearBlurEnd->current.value;
    dof->farStart = cg_heliKillCamDist->current.value + distance + cg_heliKillCamFarBlurStart->current.value;
    dof->farEnd = cg_heliKillCamDist->current.value
        + distance
        + cg_heliKillCamFarBlurStart->current.value
        + cg_heliKillCamFarBlurDist->current.value;
}

void __cdecl CG_UpdateAirstrikeKillCam(int32_t localClientNum)
{
    float scale; // [esp+0h] [ebp-60h]
    centity_s* centBomb; // [esp+34h] [ebp-2Ch]
    float bombOrigin[3]; // [esp+38h] [ebp-28h] BYREF
    float delta[3]; // [esp+44h] [ebp-1Ch] BYREF
    cg_s* cgameGlob; // [esp+50h] [ebp-10h]
    float projectedDistance; // [esp+54h] [ebp-Ch]
    float distance; // [esp+58h] [ebp-8h]
    centity_s* centTarget; // [esp+5Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    iassert(cgameGlob->predictedPlayerState.killCamEntity != ENTITYNUM_NONE);
    iassert(cgameGlob->inKillCam);
    centBomb = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.killCamEntity);
    iassert(centBomb->nextValid);
    centTarget = CG_GetEntity(localClientNum, cgameGlob->clientNum);
    iassert(centTarget->pose.eType == ET_PLAYER);
    bombOrigin[0] = centBomb->pose.origin[0];
    bombOrigin[1] = centBomb->pose.origin[1];
    bombOrigin[2] = centBomb->pose.origin[2];
    AnglesToAxis(centBomb->pose.angles, cgameGlob->refdef.viewaxis);
    Vec3Sub(centTarget->pose.origin, bombOrigin, delta);
    if (bombOrigin[2] < centTarget->pose.origin[2] + cg_airstrikeKillCamCloseZDist->current.value)
        bombOrigin[2] = centTarget->pose.origin[2] + cg_airstrikeKillCamCloseZDist->current.value;
    Vec3Sub(centTarget->pose.origin, bombOrigin, delta);
    projectedDistance = Vec3Dot(delta, cgameGlob->refdef.viewaxis[0]);
    if (projectedDistance < 0.0)
    {
        delta[0] = 0.0;
        delta[1] = 0.0;
    }
    Vec3Normalize(delta);
    Vec3Cross(delta, cgameGlob->refdef.viewaxis[1], cgameGlob->refdef.viewaxis[2]);
    Vec3Normalize(cgameGlob->refdef.viewaxis[2]);
    Vec3Cross(cgameGlob->refdef.viewaxis[1], cgameGlob->refdef.viewaxis[2], cgameGlob->refdef.viewaxis[0]);
    Vec3Normalize(cgameGlob->refdef.viewaxis[0]);
    scale = -cg_airstrikeKillCamDist->current.value;
    Vec3Mad(bombOrigin, scale, cgameGlob->refdef.viewaxis[0], cgameGlob->refdef.vieworg);
    Vec3Sub(centTarget->pose.origin, cgameGlob->refdef.vieworg, delta);
    distance = Vec3Normalize(delta);
    CG_UpdateAirstrikeKillCamDof(distance, &cgameGlob->refdef.dof);
    AxisToAngles(*(const mat3x3*)cgameGlob->refdef.viewaxis, cgameGlob->refdefViewAngles);
    CG_ShakeCamera(localClientNum);
    CG_PerturbCamera(cgameGlob);
    CG_UpdateFov(localClientNum, cg_airstrikeKillCamFov->current.value);
}

void __cdecl CG_UpdateAirstrikeKillCamDof(float distance, GfxDepthOfField *dof)
{
    dof->nearBlur = cg_airstrikeKillCamNearBlur->current.value;
    dof->farBlur = cg_airstrikeKillCamFarBlur->current.value;
    dof->nearStart = cg_airstrikeKillCamNearBlurStart->current.value;
    dof->nearEnd = distance - cg_airstrikeKillCamNearBlurEnd->current.value;
    dof->farStart = distance + cg_airstrikeKillCamFarBlurStart->current.value;
    dof->farEnd = distance
        + cg_airstrikeKillCamFarBlurStart->current.value
        + cg_airstrikeKillCamFarBlurDist->current.value;
}

void __cdecl CG_InitView(int32_t localClientNum)
{
    float zfar; // [esp+0h] [ebp-8h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    CG_UpdateThirdPerson(localClientNum);
    CG_UpdateViewOffset(localClientNum);
    CG_PredictPlayerState(localClientNum);
    CG_UpdateViewWeaponAnim(localClientNum);
    CG_CalcViewValues(localClientNum);
    FX_SetNextUpdateTime(localClientNum, cgameGlob->time);
    zfar = R_GetFarPlaneDist();
    FX_SetNextUpdateCamera(localClientNum, &cgameGlob->refdef, zfar);
}

void __cdecl CG_CalcViewValues(int32_t localClientNum)
{
    float v1; // [esp+Ch] [ebp-44h]
    float v2; // [esp+38h] [ebp-18h]
    float f; // [esp+40h] [ebp-10h]
    float uiBlurRadius; // [esp+48h] [ebp-8h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    cgameGlob->refdef.zNear = 0.0;
    cgameGlob->refdef.time = cgameGlob->time;
    cgameGlob->refdef.localClientNum = localClientNum;
    uiBlurRadius = CL_GetMenuBlurRadius(localClientNum);
    v2 = uiBlurRadius * uiBlurRadius + cgDC[localClientNum].blurRadiusOut * cgDC[localClientNum].blurRadiusOut;
    v1 = sqrt(v2);
    cgameGlob->refdef.blurRadius = v1;
    CG_VisionSetApplyToRefdef(localClientNum);
    if (cgameGlob->cubemapShot)
    {
        CG_CalcCubemapViewValues(cgameGlob);
    }
    else
    {
        CG_CalcVrect(localClientNum);
        if (cgameGlob->predictedPlayerState.pm_type == PM_INTERMISSION)
        {
            cgameGlob->refdef.vieworg[0] = cgameGlob->predictedPlayerState.origin[0];
            cgameGlob->refdef.vieworg[1] = cgameGlob->predictedPlayerState.origin[1];
            cgameGlob->refdef.vieworg[2] = cgameGlob->predictedPlayerState.origin[2];
            cgameGlob->refdefViewAngles[0] = cgameGlob->predictedPlayerState.viewangles[0];
            cgameGlob->refdefViewAngles[1] = cgameGlob->predictedPlayerState.viewangles[1];
            cgameGlob->refdefViewAngles[2] = cgameGlob->predictedPlayerState.viewangles[2];
            AnglesToAxis(cgameGlob->refdefViewAngles, cgameGlob->refdef.viewaxis);
            CG_CalcFov(localClientNum);
        }
        else if (CG_VehLocalClientUsingVehicle(localClientNum))
        {
            CalcViewValuesVehicle(localClientNum);
        }
        else if (CG_HelicopterKillCamEnabled(localClientNum))
        {
            CG_UpdateHelicopterKillCam(localClientNum);
        }
        else if (CG_AirstrikeKillCamEnabled(localClientNum))
        {
            CG_UpdateAirstrikeKillCam(localClientNum);
        }
        else
        {
            cgameGlob->fBobCycle = BG_GetBobCycle(&cgameGlob->predictedPlayerState);
            cgameGlob->xyspeed = BG_GetSpeed(&cgameGlob->predictedPlayerState, cgameGlob->time);
            cgameGlob->refdef.vieworg[0] = cgameGlob->predictedPlayerState.origin[0];
            cgameGlob->refdef.vieworg[1] = cgameGlob->predictedPlayerState.origin[1];
            cgameGlob->refdef.vieworg[2] = cgameGlob->predictedPlayerState.origin[2];
            if (!cgameGlob->playerTeleported
                && (!cgameGlob->nextSnap->ps.pm_type
                    || cgameGlob->nextSnap->ps.pm_type == PM_NOCLIP
                    || cgameGlob->nextSnap->ps.pm_type == PM_UFO))
            {
                CG_SmoothCameraZ(cgameGlob);
            }
            cgameGlob->lastVieworg[0] = cgameGlob->refdef.vieworg[0];
            cgameGlob->lastVieworg[1] = cgameGlob->refdef.vieworg[1];
            cgameGlob->lastVieworg[2] = cgameGlob->refdef.vieworg[2];
            cgameGlob->refdefViewAngles[0] = cgameGlob->predictedPlayerState.viewangles[0];
            cgameGlob->refdefViewAngles[1] = cgameGlob->predictedPlayerState.viewangles[1];
            cgameGlob->refdefViewAngles[2] = cgameGlob->predictedPlayerState.viewangles[2];
            if (cg_errorDecay->current.value > 0.0)
            {
                f = (cg_errorDecay->current.value - (double)(cgameGlob->time - cgameGlob->predictedErrorTime))
                    / cg_errorDecay->current.value;
                if (f <= 0.0 || f >= 1.0)
                    cgameGlob->predictedErrorTime = 0;
                else
                    Vec3Mad(cgameGlob->refdef.vieworg, f, cgameGlob->predictedError, cgameGlob->refdef.vieworg);
            }
            CG_CalcTurretViewValues(localClientNum);
            if (!cgameGlob->renderingThirdPerson)
                CG_OffsetFirstPersonView(cgameGlob);
            CG_ShakeCamera(localClientNum);
            AnglesToAxis(cgameGlob->refdefViewAngles, cgameGlob->refdef.viewaxis);
            CG_ApplyViewAnimation(localClientNum);
            if (cgameGlob->renderingThirdPerson)
                CG_OffsetThirdPersonView(cgameGlob);
            CG_PerturbCamera(cgameGlob);
            CG_CalcFov(localClientNum);
        }
    }
}

void __cdecl CG_OffsetThirdPersonView(cg_s *cgameGlob)
{
    float scale; // [esp+0h] [ebp-80h]
    float v2; // [esp+Ch] [ebp-74h]
    float v3; // [esp+10h] [ebp-70h]
    float v4; // [esp+18h] [ebp-68h]
    float right[3]; // [esp+28h] [ebp-58h] BYREF
    float view[3]; // [esp+34h] [ebp-4Ch] BYREF
    float forward[3]; // [esp+40h] [ebp-40h] BYREF
    float focusDist; // [esp+4Ch] [ebp-34h]
    float focusAngles[3]; // [esp+50h] [ebp-30h] BYREF
    float up[3]; // [esp+5Ch] [ebp-24h] BYREF
    float viewAngles[3]; // [esp+68h] [ebp-18h] BYREF
    float focusPoint[3]; // [esp+74h] [ebp-Ch] BYREF

    cgameGlob->refdef.vieworg[2] = cgameGlob->refdef.vieworg[2] + cgameGlob->predictedPlayerState.viewHeightCurrent;
    viewAngles[0] = cgameGlob->refdefViewAngles[0];
    viewAngles[1] = cgameGlob->refdefViewAngles[1];
    viewAngles[2] = cgameGlob->refdefViewAngles[2];
    focusAngles[0] = cgameGlob->refdefViewAngles[0];
    focusAngles[1] = cgameGlob->refdefViewAngles[1];
    focusAngles[2] = cgameGlob->refdefViewAngles[2];
    if (cgameGlob->predictedPlayerState.pm_type >= PM_DEAD)
    {
        focusAngles[1] = (float)cgameGlob->predictedPlayerState.stats[1];
        viewAngles[1] = (float)cgameGlob->predictedPlayerState.stats[1];
    }
    if (focusAngles[0] > 45.0)
        focusAngles[0] = 45.0;
    AngleVectors(focusAngles, forward, 0, 0);
    Vec3Mad(cgameGlob->refdef.vieworg, 512.0, forward, focusPoint);
    view[0] = cgameGlob->refdef.vieworg[0];
    view[1] = cgameGlob->refdef.vieworg[1];
    view[2] = cgameGlob->refdef.vieworg[2];
    view[2] = view[2] + 8.0;
    viewAngles[0] = viewAngles[0] * 0.5;
    viewAngles[1] = viewAngles[1] - cg_thirdPersonAngle->current.value;
    AngleVectors(viewAngles, forward, right, up);
    scale = -cg_thirdPersonRange->current.value;
    Vec3Mad(view, scale, forward, view);
    ThirdPersonViewTrace(cgameGlob, cgameGlob->refdef.vieworg, view, 2065, cgameGlob->refdef.vieworg);
    Vec3Sub(focusPoint, cgameGlob->refdef.vieworg, focusPoint);
    v4 = focusPoint[1] * focusPoint[1] + focusPoint[0] * focusPoint[0];
    v3 = sqrt(v4);
    focusDist = v3;
    if (v3 < 1.0)
        focusDist = 1.0;
    v2 = atan2(focusPoint[2], focusDist);
    viewAngles[0] = v2 * -57.2957763671875;
    AnglesToAxis(viewAngles, cgameGlob->refdef.viewaxis);
}

const float MYMINS[3] = { -4.0f, -4.0f, -4.0f };
const float MYMAXS[3] = { 4.0f, 4.0f, 4.0f };
void __cdecl ThirdPersonViewTrace(cg_s *cgameGlob, float *start, float *end, int32_t contentMask, float *result)
{
    float testEnd[3]; // [esp+8h] [ebp-38h] BYREF
    trace_t trace; // [esp+14h] [ebp-2Ch] BYREF

    CG_TraceCapsule(
        &trace,
        start,
        (float *)MYMINS,
        (float *)MYMAXS,
        end,
        cgameGlob->predictedPlayerState.clientNum,
        contentMask);
    if (trace.fraction == 1.0)
    {
        *result = *end;
        result[1] = end[1];
        result[2] = end[2];
    }
    else
    {
        Vec3Lerp(start, end, trace.fraction, testEnd);
        testEnd[2] = (1.0 - trace.fraction) * 32.0 + testEnd[2];
        CG_TraceCapsule(
            &trace,
            start,
            (float *)MYMINS,
            (float *)MYMAXS,
            testEnd,
            cgameGlob->predictedPlayerState.clientNum,
            contentMask);
        Vec3Lerp(start, testEnd, trace.fraction, result);
    }
}

void __cdecl CG_CalcVrect(int32_t localClientNum)
{
    cg_s *cgameGlob;
    const cgs_t *cgs;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    
    cgameGlob->refdef.x = cgs->viewX;
    cgameGlob->refdef.y = cgs->viewY;
    cgameGlob->refdef.width = cgs->viewWidth;
    cgameGlob->refdef.height = cgs->viewHeight;
    cgameGlob->refdef.useScissorViewport = 0;
}

void __cdecl CG_SmoothCameraZ(cg_s *cgameGlob)
{
    float diff; // [esp+0h] [ebp-14h]
    int32_t timeSinceStart; // [esp+4h] [ebp-10h]
    int32_t smoothingDuration; // [esp+Ch] [ebp-8h]
    float lerp; // [esp+10h] [ebp-4h]

    if (cgameGlob->stepViewChange != 0.0 && cgameGlob->time - cgameGlob->stepViewStart >= 0)
    {
        timeSinceStart = cgameGlob->time - cgameGlob->stepViewStart;
        smoothingDuration = (int)(cg_viewZSmoothingTime->current.value * 1000.0);
        if (timeSinceStart < smoothingDuration)
        {
            if (timeSinceStart >= 0)
                lerp = (double)timeSinceStart * 1.0 / (double)smoothingDuration;
            else
                lerp = 0.0;
        }
        else
        {
            lerp = 1.0;
        }
        diff = (1.0 - lerp) * cgameGlob->stepViewChange;
        cgameGlob->refdef.vieworg[2] = cgameGlob->refdef.vieworg[2] - diff;
    }
}

void __cdecl CG_OffsetFirstPersonView(cg_s *cgameGlob)
{
    int32_t v1; // [esp+14h] [ebp-54h]
    float deltaa; // [esp+1Ch] [ebp-4Ch]
    float delta; // [esp+1Ch] [ebp-4Ch]
    float deltab; // [esp+1Ch] [ebp-4Ch]
    float vRight[3]; // [esp+24h] [ebp-44h] BYREF
    float angles[3]; // [esp+30h] [ebp-38h] BYREF
    float f; // [esp+3Ch] [ebp-2Ch]
    float deltaB; // [esp+40h] [ebp-28h]
    viewState_t vs; // [esp+44h] [ebp-24h] BYREF

    iassert(cgameGlob->nextSnap->ps.pm_type < PM_DEAD);

    if (cgameGlob->nextSnap->ps.pm_type != PM_INTERMISSION && (cgameGlob->predictedPlayerState.eFlags & 0x300) == 0)
    {
        vs.ps = &cgameGlob->predictedPlayerState;
        if (cgameGlob->damageTime)
            v1 = cgameGlob->damageTime - vs.ps->deltaTime;
        else
            v1 = 0;
        vs.damageTime = v1;
        vs.time = cgameGlob->time - vs.ps->deltaTime;
        vs.v_dmg_pitch = cgameGlob->v_dmg_pitch;
        vs.v_dmg_roll = cgameGlob->v_dmg_roll;
        vs.xyspeed = cgameGlob->xyspeed;
        vs.frametime = (double)cgameGlob->frametime * EQUAL_EPSILON;
        vs.fLastIdleFactor = cgameGlob->playerEntity.fLastIdleFactor;
        vs.weapIdleTime = &cgameGlob->weapIdleTime;
        BG_CalculateViewAngles(&vs, angles);
        Vec3Add(cgameGlob->refdefViewAngles, angles, cgameGlob->refdefViewAngles);
        cgameGlob->refdef.vieworg[2] = cgameGlob->refdef.vieworg[2] + cgameGlob->predictedPlayerState.viewHeightCurrent;
        deltaa = BG_GetVerticalBobFactor(
            &cgameGlob->predictedPlayerState,
            cgameGlob->fBobCycle,
            cgameGlob->xyspeed,
            bg_bobMax->current.value);
        cgameGlob->refdef.vieworg[2] = cgameGlob->refdef.vieworg[2] + deltaa;
        deltaB = BG_GetHorizontalBobFactor(
            &cgameGlob->predictedPlayerState,
            cgameGlob->fBobCycle,
            cgameGlob->xyspeed,
            bg_bobMax->current.value);
        AngleVectors(cgameGlob->refdefViewAngles, 0, vRight, 0);
        Vec3Mad(cgameGlob->refdef.vieworg, deltaB, vRight, cgameGlob->refdef.vieworg);
        delta = (float)(cgameGlob->time - cgameGlob->landTime);
        if (delta <= 0.0 || delta >= 150.0)
        {
            if (delta > 0.0 && delta < 450.0)
            {
                deltab = delta - 150.0;
                f = 1.0 - deltab / 300.0;
                cgameGlob->refdef.vieworg[2] = cgameGlob->landChange * f + cgameGlob->refdef.vieworg[2];
            }
        }
        else
        {
            f = delta / 150.0;
            cgameGlob->refdef.vieworg[2] = cgameGlob->landChange * f + cgameGlob->refdef.vieworg[2];
        }
        AddLeanToPosition(
            cgameGlob->refdef.vieworg,
            cgameGlob->refdefViewAngles[1],
            cgameGlob->predictedPlayerState.leanf,
            16.0,
            20.0);
        if (cgameGlob->refdef.vieworg[2] < cgameGlob->predictedPlayerState.origin[2] + 8.0)
            cgameGlob->refdef.vieworg[2] = cgameGlob->predictedPlayerState.origin[2] + 8.0;
    }
}

void __cdecl CG_CalcFov(int32_t localClientNum)
{
    float fov_x; // [esp+4h] [ebp-4h]

    fov_x = CG_GetViewFov(localClientNum);
    CG_UpdateFov(localClientNum, fov_x);
}

void __cdecl CG_CalcCubemapViewValues(cg_s *cgameGlob)
{
    cgameGlob->refdef.x = 0;
    cgameGlob->refdef.y = 0;
    cgameGlob->refdef.width = cgameGlob->cubemapSize + 2;
    cgameGlob->refdef.height = cgameGlob->cubemapSize + 2;
    cgameGlob->refdef.tanHalfFovX = (double)(cgameGlob->cubemapSize + 2) / (double)cgameGlob->cubemapSize;
    cgameGlob->refdef.tanHalfFovY = cgameGlob->refdef.tanHalfFovX;
    cgameGlob->refdef.vieworg[0] = cgameGlob->predictedPlayerState.origin[0];
    cgameGlob->refdef.vieworg[1] = cgameGlob->predictedPlayerState.origin[1];
    cgameGlob->refdef.vieworg[2] = cgameGlob->predictedPlayerState.origin[2];
    cgameGlob->refdef.vieworg[2] = cgameGlob->refdef.vieworg[2] + cgameGlob->predictedPlayerState.viewHeightCurrent;
    switch (cgameGlob->cubemapShot)
    {
    case CUBEMAPSHOT_RIGHT:
        cgameGlob->refdef.viewaxis[0][0] = 1.0;
        cgameGlob->refdef.viewaxis[0][1] = 0.0;
        cgameGlob->refdef.viewaxis[0][2] = 0.0;
        cgameGlob->refdef.viewaxis[1][0] = 0.0;
        cgameGlob->refdef.viewaxis[1][1] = 1.0;
        cgameGlob->refdef.viewaxis[1][2] = 0.0;
        cgameGlob->refdef.viewaxis[2][0] = 0.0;
        cgameGlob->refdef.viewaxis[2][1] = 0.0;
        cgameGlob->refdef.viewaxis[2][2] = 1.0;
        break;
    case CUBEMAPSHOT_LEFT:
        cgameGlob->refdef.viewaxis[0][0] = -1.0;
        cgameGlob->refdef.viewaxis[0][1] = 0.0;
        cgameGlob->refdef.viewaxis[0][2] = 0.0;
        cgameGlob->refdef.viewaxis[1][0] = 0.0;
        cgameGlob->refdef.viewaxis[1][1] = -1.0;
        cgameGlob->refdef.viewaxis[1][2] = 0.0;
        cgameGlob->refdef.viewaxis[2][0] = 0.0;
        cgameGlob->refdef.viewaxis[2][1] = 0.0;
        cgameGlob->refdef.viewaxis[2][2] = 1.0;
        break;
    case CUBEMAPSHOT_BACK:
        cgameGlob->refdef.viewaxis[0][0] = 0.0;
        cgameGlob->refdef.viewaxis[0][1] = 1.0;
        cgameGlob->refdef.viewaxis[0][2] = 0.0;
        cgameGlob->refdef.viewaxis[1][0] = -1.0;
        cgameGlob->refdef.viewaxis[1][1] = 0.0;
        cgameGlob->refdef.viewaxis[1][2] = 0.0;
        cgameGlob->refdef.viewaxis[2][0] = 0.0;
        cgameGlob->refdef.viewaxis[2][1] = 0.0;
        cgameGlob->refdef.viewaxis[2][2] = 1.0;
        break;
    case CUBEMAPSHOT_FRONT:
        cgameGlob->refdef.viewaxis[0][0] = 0.0;
        cgameGlob->refdef.viewaxis[0][1] = -1.0;
        cgameGlob->refdef.viewaxis[0][2] = 0.0;
        cgameGlob->refdef.viewaxis[1][0] = 1.0;
        cgameGlob->refdef.viewaxis[1][1] = 0.0;
        cgameGlob->refdef.viewaxis[1][2] = 0.0;
        cgameGlob->refdef.viewaxis[2][0] = 0.0;
        cgameGlob->refdef.viewaxis[2][1] = 0.0;
        cgameGlob->refdef.viewaxis[2][2] = 1.0;
        break;
    case CUBEMAPSHOT_UP:
        cgameGlob->refdef.viewaxis[0][0] = 0.0;
        cgameGlob->refdef.viewaxis[0][1] = 0.0;
        cgameGlob->refdef.viewaxis[0][2] = 1.0;
        cgameGlob->refdef.viewaxis[1][0] = 0.0;
        cgameGlob->refdef.viewaxis[1][1] = 1.0;
        cgameGlob->refdef.viewaxis[1][2] = 0.0;
        cgameGlob->refdef.viewaxis[2][0] = -1.0;
        cgameGlob->refdef.viewaxis[2][1] = 0.0;
        cgameGlob->refdef.viewaxis[2][2] = 0.0;
        break;
    case CUBEMAPSHOT_DOWN:
        cgameGlob->refdef.viewaxis[0][0] = 0.0;
        cgameGlob->refdef.viewaxis[0][1] = 0.0;
        cgameGlob->refdef.viewaxis[0][2] = -1.0;
        cgameGlob->refdef.viewaxis[1][0] = 0.0;
        cgameGlob->refdef.viewaxis[1][1] = 1.0;
        cgameGlob->refdef.viewaxis[1][2] = 0.0;
        cgameGlob->refdef.viewaxis[2][0] = 1.0;
        cgameGlob->refdef.viewaxis[2][1] = 0.0;
        cgameGlob->refdef.viewaxis[2][2] = 0.0;
        break;
    default:
        return;
    }
}

void __cdecl CG_CalcTurretViewValues(int32_t localClientNum)
{
    double v1; // [esp+0h] [ebp-20h]
    double v2; // [esp+8h] [ebp-18h]
    DObj_s *obj; // [esp+10h] [ebp-10h]
    centity_s *cent; // [esp+18h] [ebp-8h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    
    if ((cgameGlob->predictedPlayerState.eFlags & 0x300) != 0)
    {
        if (cgameGlob->predictedPlayerState.viewlocked == PLAYERVIEWLOCK_NONE)
            MyAssertHandler(".\\cgame_mp\\cg_view_mp.cpp", 701, 0, "%s", "ps->viewlocked");
        if (cgameGlob->predictedPlayerState.viewlocked_entNum == 1023)
            MyAssertHandler(".\\cgame_mp\\cg_view_mp.cpp", 702, 0, "%s", "ps->viewlocked_entNum != ENTITYNUM_NONE");
        cent = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.viewlocked_entNum);
        obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
        if (obj)
        {
            if (!CG_DObjGetWorldTagPos(&cent->pose, obj, scr_const.tag_player, cgameGlob->refdef.vieworg))
                Com_Error(ERR_DROP, "Turret has no bone: tag_player");
            if (cgameGlob->predictedPlayerState.viewlocked == PLAYERVIEWLOCK_WEAPONJITTER && !cgameGlob->renderingThirdPerson)
            {
                v2 = crandom();
                cgameGlob->refdefViewAngles[0] = BG_GetWeaponDef(cent->nextState.weapon)->vertViewJitter * v2
                    + cgameGlob->refdefViewAngles[0];
                v1 = crandom();
                cgameGlob->refdefViewAngles[1] = BG_GetWeaponDef(cent->nextState.weapon)->horizViewJitter * v1
                    + cgameGlob->refdefViewAngles[1];
            }
        }
    }
}

void __cdecl CG_ApplyViewAnimation(int32_t localClientNum)
{
    weaponInfo_s* weapInfo; // [esp+20h] [ebp-10h]
    int32_t weaponIndex; // [esp+28h] [ebp-8h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cgameGlob->predictedPlayerState.pm_type != PM_SPECTATOR
        && cgameGlob->predictedPlayerState.pm_type != PM_INTERMISSION
        && (cgameGlob->predictedPlayerState.eFlags & 0x300) == 0
        && !cgameGlob->renderingThirdPerson)
    {
        weaponIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
        if (weaponIndex > 0)
        {
            vassert(localClientNum == 0, "localClientNum = %d", localClientNum);
            weapInfo = &cg_weaponsArray[0][weaponIndex];
            if (weapInfo->viewModelDObj)
            {
                cgameGlob->viewModelAxis[0][0] = cgameGlob->refdef.viewaxis[0][0];
                cgameGlob->viewModelAxis[0][1] = cgameGlob->refdef.viewaxis[0][1];
                cgameGlob->viewModelAxis[0][2] = cgameGlob->refdef.viewaxis[0][2];
                cgameGlob->viewModelAxis[1][0] = cgameGlob->refdef.viewaxis[1][0];
                cgameGlob->viewModelAxis[1][1] = cgameGlob->refdef.viewaxis[1][1];
                cgameGlob->viewModelAxis[1][2] = cgameGlob->refdef.viewaxis[1][2];
                cgameGlob->viewModelAxis[2][0] = cgameGlob->refdef.viewaxis[2][0];
                cgameGlob->viewModelAxis[2][1] = cgameGlob->refdef.viewaxis[2][1];
                cgameGlob->viewModelAxis[2][2] = cgameGlob->refdef.viewaxis[2][2];
                cgameGlob->viewModelAxis[3][0] = cgameGlob->refdef.vieworg[0];
                cgameGlob->viewModelAxis[3][1] = cgameGlob->refdef.vieworg[1];
                cgameGlob->viewModelAxis[3][2] = cgameGlob->refdef.vieworg[2];
                CG_UpdateViewModelPose(weapInfo->viewModelDObj, localClientNum);
                if (CG_DObjGetWorldTagMatrix(
                    &cgameGlob->viewModelPose,
                    weapInfo->viewModelDObj,
                    scr_const.tag_camera,
                    cgameGlob->refdef.viewaxis,
                    cgameGlob->refdef.vieworg))
                {
                    AxisToAngles(*(const mat3x3*)cgameGlob->refdef.viewaxis, cgameGlob->refdefViewAngles);
                }
            }
        }
    }
}

void __cdecl CalcViewValuesVehicle(int32_t localClientNum)
{
    int32_t slot; // [esp+4h] [ebp-4h]

    if (!CG_VehLocalClientUsingVehicle(localClientNum))
        MyAssertHandler(".\\cgame_mp\\cg_view_mp.cpp", 852, 0, "%s", "CG_VehLocalClientUsingVehicle( localClientNum )");
    slot = CG_VehLocalClientVehicleSlot(localClientNum);
    if (slot)
    {
        if (slot == 1)
        {
            CalcViewValuesVehiclePassenger(localClientNum);
        }
        else
        {
            iassert(slot == VEHICLE_RIDESLOT_GUNNER);
            CalcViewValuesVehicleGunner(localClientNum);
        }
    }
    else
    {
        CalcViewValuesVehicleDriver(localClientNum);
    }
}

const float TEMP_OFFSET[3] = { 0.0f, 0.0f, 55.0f };
void __cdecl CalcViewValuesVehicleDriver(int32_t localClientNum)
{
    float v1; // [esp+10h] [ebp-4Ch]
    float v2; // [esp+14h] [ebp-48h]
    float v3; // [esp+18h] [ebp-44h]
    float v4; // [esp+2Ch] [ebp-30h]
    clientActive_t *LocalClientGlobals; // [esp+34h] [ebp-28h]
    float tmpVect[3]; // [esp+3Ch] [ebp-20h] BYREF
    float pitch; // [esp+48h] [ebp-14h]
    playerState_s *ps; // [esp+4Ch] [ebp-10h]
    float focusPoint[3]; // [esp+50h] [ebp-Ch] BYREF
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    focusPoint[0] = cgameGlob->predictedPlayerState.origin[0];
    focusPoint[1] = cgameGlob->predictedPlayerState.origin[1];
    focusPoint[2] = cgameGlob->predictedPlayerState.origin[2];
    Vec3Add(focusPoint, TEMP_OFFSET, focusPoint);
    v4 = LocalClientGlobals->vehicleViewPitch * 0.002777777845039964;
    v3 = v4 + 0.5;
    v2 = floor(v3);
    pitch = (v4 - v2) * 360.0;
    v1 = fabs(pitch);
    pitch = v1;
    if (vehDriverViewHeightMax->current.value == 0.0)
        MyAssertHandler(".\\cgame_mp\\cg_view_mp.cpp", 791, 0, "%s", "vehDriverViewHeightMax->current.value != 0");
    focusPoint[2] = (vehDriverViewHeightMax->current.value - pitch)
        / vehDriverViewHeightMax->current.value
        * vehDriverViewFocusRange->current.value
        + focusPoint[2];
    if (vehDebugClient->current.enabled)
        CG_DebugStar(focusPoint, colorBlue, 0);
    CG_VehSphereCoordsToPos(
        vehDriverViewDist->current.value,
        LocalClientGlobals->vehicleViewYaw,
        LocalClientGlobals->vehicleViewPitch,
        tmpVect);
    Vec3Add(tmpVect, focusPoint, cgameGlob->refdef.vieworg);
    ThirdPersonViewTrace(cgameGlob, focusPoint, cgameGlob->refdef.vieworg, 2065, cgameGlob->refdef.vieworg);
    Vec3Sub(focusPoint, cgameGlob->refdef.vieworg, tmpVect);
    Vec3Normalize(tmpVect);
    vectoangles(tmpVect, cgameGlob->refdefViewAngles);
    AnglesToAxis(cgameGlob->refdefViewAngles, cgameGlob->refdef.viewaxis);
    CG_CalcFov(localClientNum);
}

void CalcViewValuesVehiclePassenger(int32_t localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    CG_VehSeatOriginForLocalClient(localClientNum, cgameGlob->predictedPlayerState.origin);
    cgameGlob->refdef.vieworg[0] = cgameGlob->predictedPlayerState.origin[0];
    cgameGlob->refdef.vieworg[1] = cgameGlob->predictedPlayerState.origin[1];
    cgameGlob->refdef.vieworg[2] = cgameGlob->predictedPlayerState.origin[2];
    cgameGlob->refdefViewAngles[0] = cgameGlob->predictedPlayerState.viewangles[0];
    cgameGlob->refdefViewAngles[1] = cgameGlob->predictedPlayerState.viewangles[1];
    cgameGlob->refdefViewAngles[2] = cgameGlob->predictedPlayerState.viewangles[2];
    CG_OffsetFirstPersonView(cgameGlob);
    AnglesToAxis(cgameGlob->refdefViewAngles, cgameGlob->refdef.viewaxis);
    CG_ApplyViewAnimation(localClientNum);
    CG_PerturbCamera(cgameGlob);
    CG_CalcFov(localClientNum);
}

void CalcViewValuesVehicleGunner(int32_t localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    CG_VehGunnerPOV(localClientNum, cgameGlob->refdef.vieworg, cgameGlob->refdefViewAngles);
    AnglesToAxis(cgameGlob->refdefViewAngles, cgameGlob->refdef.viewaxis);
    CG_ApplyViewAnimation(localClientNum);
    CG_PerturbCamera(cgameGlob);
    CG_CalcFov(localClientNum);
}

bool __cdecl CG_HelicopterKillCamEnabled(int32_t localClientNum)
{
    centity_s* cent; // [esp+4h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if (!cgameGlob->inKillCam)
        return 0;
    if (cgameGlob->predictedPlayerState.killCamEntity == 1023)
        return 0;
    cent = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.killCamEntity);
    return cent->nextValid && cent->pose.eType == 12;
}

bool __cdecl CG_AirstrikeKillCamEnabled(int32_t localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if (!cgameGlob->inKillCam)
        return 0;
    if (cgameGlob->predictedPlayerState.killCamEntity == 1023)
        return 0;
    return CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.killCamEntity)->nextValid;
}

void __cdecl CG_UpdateThirdPerson(int32_t localClientNum)
{
    bool v1; // [esp+0h] [ebp-8h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    v1 = cg_thirdPerson->current.enabled || cgameGlob->nextSnap->ps.pm_type >= PM_DEAD;
    cgameGlob->renderingThirdPerson = v1;
    if (CG_VehLocalClientDriving(localClientNum))
        cgameGlob->renderingThirdPerson = 1;
    if (CG_KillCamEntityEnabled(localClientNum))
        cgameGlob->renderingThirdPerson = 1;
}

bool __cdecl CG_KillCamEntityEnabled(int32_t localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (!cgameGlob->inKillCam)
        return 0;
    if (cgameGlob->predictedPlayerState.killCamEntity == 1023)
        return 0;
    return CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.killCamEntity)->nextValid;
}

const ClientViewParams *__cdecl CG_GetLocalClientViewParams(int32_t localClientNum)
{
    int32_t activeClientCountArrayIndex; // [esp+0h] [ebp-8h]
    int32_t activeClientIndex; // [esp+4h] [ebp-4h]

    activeClientIndex = CL_LocalActiveIndexFromClientNum(localClientNum);
    activeClientCountArrayIndex = CL_GetLocalClientActiveCount() - 1;
    if (activeClientIndex)
        MyAssertHandler(
            ".\\cgame_mp\\cg_view_mp.cpp",
            1323,
            0,
            "activeClientIndex doesn't index ARRAY_COUNT( clientViewParamsArray[0] )\n\t%i not in [0, %i)",
            activeClientIndex,
            1);
    if (activeClientCountArrayIndex)
        MyAssertHandler(
            ".\\cgame_mp\\cg_view_mp.cpp",
            1324,
            0,
            "activeClientCountArrayIndex doesn't index ARRAY_COUNT( clientViewParamsArray )\n\t%i not in [0, %i)",
            activeClientCountArrayIndex,
            1);
    if (activeClientIndex > activeClientCountArrayIndex)
        MyAssertHandler(
            ".\\cgame_mp\\cg_view_mp.cpp",
            1325,
            0,
            "activeClientIndex <= activeClientCountArrayIndex\n\t%i, %i",
            activeClientIndex,
            activeClientCountArrayIndex);
    return &clientViewParamsArray[activeClientCountArrayIndex][activeClientIndex];
}

void __cdecl CG_UpdateViewOffset(int32_t localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (CG_KillCamEntityEnabled(localClientNum))
    {
        CG_UpdateKillCamEntityViewOffset(localClientNum);
    }
    else
    {
        Vec3Lerp(
            cgameGlob->snap->ps.origin,
            cgameGlob->nextSnap->ps.origin,
            cgameGlob->frameInterpolation,
            cgameGlob->refdef.viewOffset);
        cgameGlob->refdef.viewOffset[2] = cgameGlob->refdef.viewOffset[2] + cgameGlob->nextSnap->ps.viewHeightCurrent;
    }
    CL_ResetSkeletonCache(localClientNum);
}

void __cdecl CG_UpdateKillCamEntityViewOffset(int32_t localClientNum)
{
    centity_s* cent; // [esp+Ch] [ebp-4h]

    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    iassert(cgameGlob->predictedPlayerState.killCamEntity != ENTITYNUM_NONE);
    iassert(cgameGlob->inKillCam);

    cent = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.killCamEntity);
    iassert(cent->nextValid);
    cgameGlob->refdef.viewOffset[0] = cent->pose.origin[0];
    cgameGlob->refdef.viewOffset[1] = cent->pose.origin[1];
    cgameGlob->refdef.viewOffset[2] = cent->pose.origin[2];
}

void __cdecl CL_SyncGpu(int(__cdecl *WorkCallback)(uint64_t))
{
    R_SyncGpu(WorkCallback);
}

int32_t __cdecl CG_DrawActiveFrame(
    int32_t localClientNum,
    int32_t serverTime,
    DemoType demoType,
    CubemapShot cubemapShot,
    int32_t cubemapSize,
    int32_t renderScreen)
{
    shellshock_parms_t* ShellshockParms; // eax
    uint32_t NumWeapons; // eax
    int32_t tanHalfFovX; // [esp+0h] [ebp-74h]
    int32_t zfar; // [esp+4h] [ebp-70h]
    float zfara; // [esp+4h] [ebp-70h]
    uint32_t weapIdx; // [esp+4Ch] [ebp-28h]
    int32_t i; // [esp+50h] [ebp-24h]
    DObj_s* obj; // [esp+54h] [ebp-20h]
    FxCmd fxUpdateCmd; // [esp+5Ch] [ebp-18h] BYREF
    int32_t viewlocked_entNum; // [esp+68h] [ebp-Ch]
    const cgs_t* cgs; // [esp+6Ch] [ebp-8h]
    int32_t prevState; // [esp+70h] [ebp-4h]
    cg_s *cgameGlob;

    prevState = 0;
    iassert(Sys_IsMainThread());
    
    R_ClearScene(localClientNum);
    FX_BeginUpdate(localClientNum);
    CG_SetCollWorldLocalClientNum(localClientNum);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    cgameGlob->oldTime = cgameGlob->time;
    cgameGlob->time = serverTime;
    cgameGlob->bgs.time = cgameGlob->time;
    cgameGlob->demoType = demoType;
    cgameGlob->cubemapShot = cubemapShot;
    cgameGlob->cubemapSize = cubemapSize;
    cgameGlob->renderScreen = renderScreen;
    cgameGlob->frametime = cgameGlob->time - cgameGlob->oldTime;
    if (cgameGlob->frametime < 0)
    {
        FX_RewindTo(localClientNum, cgameGlob->time);
        cgameGlob->frametime = 0;
        cgameGlob->oldTime = cgameGlob->time;
    }
    CG_AddLagometerFrameInfo(cgameGlob);
    cgameGlob->bgs.frametime = cgameGlob->frametime;
    if (bgs)
        MyAssertHandler(".\\cgame_mp\\cg_view_mp.cpp", 1703, 0, "%s\n\t(bgs) = %p", "(bgs == 0)", bgs);
    if (cgameGlob->isLoading)
        return 0;
    bgs = &cgameGlob->bgs;
    if (cgameGlob->snap)
        prevState = cgameGlob->snap->ps.pm_type;
    CG_ProcessSnapshots(localClientNum);
    if (cgameGlob->renderScreen)
    {
        if (cgameGlob->nextSnap && (cgameGlob->nextSnap->snapFlags & 2) == 0)
        {
            if (CL_IsServerLoadingMap())
            {
                if (bgs != &cgameGlob->bgs)
                    MyAssertHandler(".\\cgame_mp\\cg_view_mp.cpp", 1740, 0, "%s\n\t(bgs) = %p", "(bgs == &cgameGlob->bgs)", bgs);
                bgs = 0;
                return 0;
            }
            else
            {
                CL_SetWaitingOnServerToLoadMap(localClientNum, 0);
                if (bgs != &cgameGlob->bgs)
                    MyAssertHandler(".\\cgame_mp\\cg_view_mp.cpp", 1747, 0, "%s\n\t(bgs) = %p", "(bgs == &cgameGlob->bgs)", bgs);
                if (!cgameGlob->snap)
                    MyAssertHandler(".\\cgame_mp\\cg_view_mp.cpp", 1749, 0, "%s", "cgameGlob->snap");
                if (!cgameGlob->nextSnap)
                    MyAssertHandler(".\\cgame_mp\\cg_view_mp.cpp", 1750, 0, "%s", "cgameGlob->nextSnap");
                CG_VisionSetsUpdate(localClientNum);
                CG_UpdateViewOffset(localClientNum);
                cgameGlob->refdef.vieworg[0] = cgameGlob->refdef.viewOffset[0];
                cgameGlob->refdef.vieworg[1] = cgameGlob->refdef.viewOffset[1];
                cgameGlob->refdef.vieworg[2] = cgameGlob->refdef.viewOffset[2];
                cgameGlob->refdef.time = cgameGlob->time;
                R_SetLodOrigin(&cgameGlob->refdef);
                FX_SetNextUpdateTime(localClientNum, cgameGlob->time);
                FX_FillUpdateCmd(localClientNum, &fxUpdateCmd);
                R_UpdateNonDependentEffects(&fxUpdateCmd);
                cgs = CG_GetLocalClientStaticGlobals(localClientNum);
                if (cgameGlob->snap->ps.shellshockIndex)
                {
                    zfar = cgameGlob->snap->ps.shellshockDuration;
                    tanHalfFovX = cgameGlob->snap->ps.shellshockTime;
                    ShellshockParms = BG_GetShellshockParms(cgameGlob->snap->ps.shellshockIndex);
                }
                else
                {
                    zfar = cgameGlob->testShock.duration;
                    tanHalfFovX = cgameGlob->testShock.time;
                    ShellshockParms = BG_GetShellshockParms(0);
                }
                CG_StartShellShock(cgameGlob, ShellshockParms, tanHalfFovX, zfar);
                CG_UpdateShellShock(
                    localClientNum,
                    cgameGlob->shellshock.parms,
                    cgameGlob->shellshock.startTime,
                    cgameGlob->shellshock.duration);
                CG_UpdateThirdPerson(localClientNum);
                CG_ClearHudGrenades();
                CG_UpdateEntInfo(localClientNum);
                AimTarget_ClearTargetList(localClientNum);
                if (CG_AddPacketEntities(localClientNum))
                    viewlocked_entNum = cgameGlob->predictedPlayerState.viewlocked_entNum;
                else
                    viewlocked_entNum = 1023;
                if (!cgameGlob->predictedPlayerState.locationSelectionInfo
                    || (cgameGlob->predictedPlayerState.otherFlags & 2) != 0)
                {
                    if (Key_IsCatcherActive(localClientNum, 8))
                        Key_RemoveCatcher(localClientNum, -9);
                    cgameGlob->selectedLocation[0] = 0.5;
                    cgameGlob->selectedLocation[1] = 0.5;
                }
                else if (!Key_IsCatcherActive(localClientNum, 8))
                {
                    Key_AddCatcher(localClientNum, 8);
                    cgameGlob->selectedLocation[0] = 0.5;
                    cgameGlob->selectedLocation[1] = 0.5;
                }
                if ((cgameGlob->nextSnap->ps.otherFlags & 4) != 0)
                {
                    CG_KickAngles(cgameGlob);
                }
                else
                {
                    cgameGlob->kickAVel[0] = 0.0;
                    cgameGlob->kickAVel[1] = 0.0;
                    cgameGlob->kickAVel[2] = 0.0;
                    cgameGlob->kickAngles[0] = 0.0;
                    cgameGlob->kickAngles[1] = 0.0;
                    cgameGlob->kickAngles[2] = 0.0;
                }
                CL_SyncGpu(0);
                CL_Input(localClientNum);
                KISAK_NULLSUB();
                CG_PredictPlayerState(localClientNum);
                KISAK_NULLSUB();
                CG_UpdateViewWeaponAnim(localClientNum);
                KISAK_NULLSUB();
                CG_CalcViewValues(localClientNum);
                KISAK_NULLSUB();
                zfara = R_GetFarPlaneDist();
                FX_SetNextUpdateCamera(localClientNum, &cgameGlob->refdef, zfara);
                R_UpdateSpotLightEffect(&fxUpdateCmd);
                SND_SetListener(
                    localClientNum,
                    cgameGlob->nextSnap->ps.clientNum,
                    cgameGlob->refdef.vieworg,
                    cgameGlob->refdef.viewaxis);
                CG_AddViewWeapon(localClientNum);
                CG_UpdateTestFX(localClientNum);
                if ((cgameGlob->nextSnap->ps.otherFlags & 6) != 0)
                {
                    obj = Com_GetClientDObj(cgameGlob->nextSnap->ps.clientNum, localClientNum);
                    if (obj)
                    {
                        CG_DObjUpdateInfo(cgameGlob, obj, 1);
                        CG_ProcessClientNoteTracks(cgameGlob, cgameGlob->nextSnap->ps.clientNum);
                    }
                    CG_CalcEntityLerpPositions(localClientNum, &cgameGlob->predictedPlayerEntity);
                    for (i = 0; i < 6; ++i)
                        cgameGlob->predictedPlayerEntity.pose.player.tag[i] = -2;
                    CG_ProcessEntity(localClientNum, &cgameGlob->predictedPlayerEntity);
                }
                if (viewlocked_entNum != 1023)
                    CG_AddPacketEntity(localClientNum, viewlocked_entNum);
                GetCeilingHeight(cgameGlob);
                if (!localClientNum)
                    DumpAnims(0);
                KISAK_NULLSUB();
                R_UpdateRemainingEffects(&fxUpdateCmd);
                KISAK_NULLSUB();
                AimTarget_UpdateClientTargets(localClientNum);
                AimAssist_UpdateScreenTargets(
                    localClientNum,
                    cgameGlob->refdef.vieworg,
                    cgameGlob->refdefViewAngles,
                    cgameGlob->refdef.tanHalfFovX,
                    cgameGlob->refdef.tanHalfFovY);
                CG_UpdateSceneDepthOfField(localClientNum);
                KISAK_NULLSUB();
                R_AddCmdProjectionSet2D();
                DrawShellshockBlend(localClientNum);
                CG_CompassIncreaseRadarTime(localClientNum);
                {
                    PROF_SCOPED("CG_Draw2D");
                    CG_Draw2D(localClientNum);
                }
                if (cgameGlob->weaponSelect >= BG_GetNumWeapons())
                {
                    NumWeapons = BG_GetNumWeapons();
                    Com_PrintWarning(
                        17,
                        "WARNING: Invalid weaponSelect setting %i (out of range 0 - %i)\n",
                        cgameGlob->weaponSelect,
                        NumWeapons - 1);
                    cgameGlob->weaponSelect = 0;
                    for (weapIdx = 1; weapIdx < BG_GetNumWeapons(); ++weapIdx)
                    {
                        if (Com_BitCheckAssert(cgameGlob->predictedPlayerState.weapons, weapIdx, 16))
                        {
                            cgameGlob->weaponSelect = weapIdx;
                            break;
                        }
                    }
                }
                CG_DrawActive(localClientNum);
                iassert(bgs == &cgameGlob->bgs);
                bgs = 0;
                return 1;
            }
        }
        else
        {
            iassert(bgs == &cgameGlob->bgs);
            bgs = 0;
            return 0;
        }
    }
    else
    {
        iassert(bgs == &cgameGlob->bgs);
        bgs = 0;
        return 0;
    }
}

void __cdecl CG_UpdateTestFX(int32_t localClientNum)
{
    if (s_testEffect[localClientNum].respawnTime >= 1)
    {
        if (CG_GetLocalClientGlobals(0)->time > s_testEffect[localClientNum].respawnTime + s_testEffect[localClientNum].time)
            CG_PlayTestFx(localClientNum);
    }
}

void __cdecl CG_KickAngles(cg_s *cgameGlob)
{
    double v1; // st7
    float v2; // [esp+0h] [ebp-38h]
    float v3; // [esp+4h] [ebp-34h]
    float v4; // [esp+8h] [ebp-30h]
    int32_t v5; // [esp+Ch] [ebp-2Ch]
    float kickChange; // [esp+14h] [ebp-24h]
    int32_t t; // [esp+18h] [ebp-20h]
    float idealCenterSpeed; // [esp+20h] [ebp-18h]
    int32_t weapIndex; // [esp+24h] [ebp-14h]
    int32_t i; // [esp+28h] [ebp-10h]
    WeaponDef *weapDef; // [esp+2Ch] [ebp-Ch]
    float ft; // [esp+34h] [ebp-4h]

    weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    weapDef = BG_GetWeaponDef(weapIndex);
    for (t = cgameGlob->frametime; t > 0; t -= 5)
    {
        if (t <= 5)
            v5 = t;
        else
            v5 = 5;
        ft = (double)v5 * EQUAL_EPSILON;
        for (i = 0; i < 3; ++i)
        {
            if (cgameGlob->kickAVel[i] != 0.0 || cgameGlob->kickAngles[i] != 0.0)
            {
                if (cgameGlob->kickAngles[i] != 0.0)
                {
                    v4 = cgameGlob->kickAngles[i] <= 0.0 ? 1.0 : -1.0;
                    if (v4 != 0.0)
                    {
                        if (weapIndex)
                        {
                            if (cgameGlob->predictedPlayerState.fWeaponPosFrac <= 0.5)
                                v1 = v4 * weapDef->fHipViewKickCenterSpeed;
                            else
                                v1 = v4 * weapDef->fAdsViewKickCenterSpeed;
                            idealCenterSpeed = v1;
                        }
                        else
                        {
                            idealCenterSpeed = v4 * (float)2400.0;
                        }
                        cgameGlob->kickAVel[i] = idealCenterSpeed * ft + cgameGlob->kickAVel[i];
                    }
                }
                kickChange = cgameGlob->kickAVel[i] * ft;
                if (cgameGlob->kickAngles[i] * kickChange < 0.0)
                    kickChange = kickChange * 0.05999999865889549;
                if ((cgameGlob->kickAngles[i] + kickChange) * cgameGlob->kickAngles[i] < 0.0)
                {
                    cgameGlob->kickAngles[i] = 0.0;
                    cgameGlob->kickAVel[i] = 0.0;
                }
                else
                {
                    cgameGlob->kickAngles[i] = cgameGlob->kickAngles[i] + kickChange;
                    if (cgameGlob->kickAngles[i] == 0.0)
                    {
                        cgameGlob->kickAVel[i] = 0.0;
                    }
                    else
                    {
                        v3 = fabs(cgameGlob->kickAngles[i]);
                        if (v3 > 10.0)
                        {
                            if (cgameGlob->kickAngles[i] <= 0.0)
                                v2 = -10.0;
                            else
                                v2 = 10.0;
                            cgameGlob->kickAngles[i] = v2;
                            cgameGlob->kickAVel[i] = 0.0;
                        }
                    }
                }
            }
        }
    }
}

void __cdecl CG_UpdateEntInfo(int32_t localClientNum)
{
    DObj_s* obj; // [esp+30h] [ebp-10h]
    int32_t num; // [esp+38h] [ebp-8h]
    uint32_t entnum; // [esp+3Ch] [ebp-4h]

    KISAK_NULLSUB();
    PROF_SCOPED("CG_UpdateEntInfo");

    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    for (num = 0; num < cgameGlob->nextSnap->numEntities; ++num)
    {
        entnum = cgameGlob->nextSnap->entities[num].number;
        obj = Com_GetClientDObj(entnum, localClientNum);
        if (obj)
        {
            CG_DObjUpdateInfo(cgameGlob, obj, 1);
            CG_ProcessClientNoteTracks(cgameGlob, entnum);
        }
    }
}

void __cdecl GetCeilingHeight(cg_s *cgameGlob)
{
    trace_t result; // [esp+Ch] [ebp-3Ch] BYREF
    float endPos[4]; // [esp+38h] [ebp-10h] BYREF

    endPos[3] = 1024.0;
    endPos[0] = cgameGlob->predictedPlayerState.origin[0];
    endPos[1] = cgameGlob->predictedPlayerState.origin[1];
    endPos[2] = cgameGlob->predictedPlayerState.origin[2];
    endPos[2] = endPos[2] + (float)1024.0;
    CG_TraceCapsule(
        &result,
        cgameGlob->predictedPlayerState.origin,
        (float *)playerMins,
        (float *)playerMaxs,
        endPos,
        1023,
        1);
    if (result.fraction < 1.0)
    {
        Vec3Lerp(cgameGlob->predictedPlayerState.origin, endPos, result.fraction, endPos);
        cgameGlob->heightToCeiling = endPos[2] - cgameGlob->predictedPlayerState.origin[2];
    }
    else
    {
        cgameGlob->heightToCeiling = FLT_MAX;
    }
}

void __cdecl DumpAnims(int32_t localClientNum)
{
    const DObj_s *obj; // [esp+0h] [ebp-4h]

    if (cg_dumpAnims->current.integer < -1 || cg_dumpAnims->current.integer >= 1024)
        MyAssertHandler(
            ".\\cgame_mp\\cg_view_mp.cpp",
            1407,
            0,
            "%s\n\t(cg_dumpAnims->current.integer) = %i",
            "(cg_dumpAnims->current.integer >= -1 && cg_dumpAnims->current.integer < (1<<10))",
            cg_dumpAnims->current.integer);
    if (cg_dumpAnims->current.integer >= 0 && !cg_paused->current.integer)
    {
        obj = Com_GetClientDObj(cg_dumpAnims->current.unsignedInt, localClientNum);
        if (obj)
            DObjDisplayAnim(obj, "client:\n");
    }
}

void __cdecl DrawShellshockBlend(int32_t localClientNum)
{
    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cg_drawShellshock->current.enabled)
    {
        if (cgameGlob->shellshock.parms->screenBlend.type)
        {
            iassert(cgameGlob->shellshock.parms->screenBlend.type == SHELLSHOCK_VIEWTYPE_FLASHED);
            CG_DrawShellShockSavedScreenBlendFlashed(
                localClientNum,
                cgameGlob->shellshock.parms,
                cgameGlob->shellshock.startTime,
                cgameGlob->shellshock.duration);
        }
        else
        {
            CG_DrawShellShockSavedScreenBlendBlurred(
                localClientNum,
                cgameGlob->shellshock.parms,
                cgameGlob->shellshock.startTime,
                cgameGlob->shellshock.duration);
        }
    }
}

void __cdecl CG_UpdateSceneDepthOfField(int32_t localClientNum)
{
    playerState_s* ps; // [esp+Ch] [ebp-4h]

    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    ps = &cgameGlob->snap->ps;
    if (!CG_KillCamEntityEnabled(localClientNum))
    {
        if (!G_ExitAfterConnectPaths()
            && ps->dofNearStart == 0.0
            && ps->dofNearEnd == 0.0
            && ps->dofFarStart == 0.0
            && ps->dofFarEnd == 0.0)
        {
            CG_UpdateAdsDof(localClientNum, &cgameGlob->refdef.dof);
            iassert(cgameGlob->refdef.dof.nearBlur >= 4.0f);
        }
        else
        {
            cgameGlob->refdef.dof.nearStart = ps->dofNearStart;
            cgameGlob->refdef.dof.nearEnd = ps->dofNearEnd;
            cgameGlob->refdef.dof.farStart = ps->dofFarStart;
            cgameGlob->refdef.dof.farEnd = ps->dofFarEnd;
            cgameGlob->refdef.dof.nearBlur = ps->dofNearBlur;
            cgameGlob->refdef.dof.farBlur = ps->dofFarBlur;
        }
    }
}

void __cdecl CG_UpdateAdsDof(int32_t localClientNum, GfxDepthOfField *dof)
{
    uint32_t ScreenTargetEntity; // eax
    float v[4]; // [esp+2Ch] [ebp-8Ch] BYREF
    float diff[3]; // [esp+3Ch] [ebp-7Ch] BYREF
    float nearStart; // [esp+48h] [ebp-70h]
    float dt; // [esp+4Ch] [ebp-6Ch]
    int32_t targetCount; // [esp+50h] [ebp-68h]
    cg_s *cgameGlob; // [esp+54h] [ebp-64h]
    float nearEnd; // [esp+58h] [ebp-60h]
    centity_s *cent; // [esp+5Ch] [ebp-5Ch]
    float nearBlur; // [esp+60h] [ebp-58h]
    float farStart; // [esp+64h] [ebp-54h]
    int32_t targetIndex; // [esp+68h] [ebp-50h]
    trace_t trace; // [esp+6Ch] [ebp-4Ch] BYREF
    float targetDist; // [esp+98h] [ebp-20h]
    float traceDist; // [esp+9Ch] [ebp-1Ch]
    float farBlur; // [esp+A0h] [ebp-18h]
    playerState_s *ps; // [esp+A4h] [ebp-14h]
    float traceEnd[3]; // [esp+A8h] [ebp-10h] BYREF
    float farEnd; // [esp+B4h] [ebp-4h]

    iassert(dof);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    ps = &cgameGlob->predictedPlayerState;
    if (cgameGlob->predictedPlayerState.fWeaponPosFrac == 0.0f && ps->pm_type < PM_DEAD)
    {
        dof->nearStart = 0.0f;
        dof->nearEnd = 0.0f;
        dof->farStart = 5000.0f;
        dof->farEnd = 5000.0f;
        dof->nearBlur = 6.0f;
        dof->farBlur = 0.0f;
    }
    else
    {
        nearEnd = 10000.0f;
        farStart = -1.0f;
        targetCount = AimAssist_GetScreenTargetCount(localClientNum);
        for (targetIndex = 0; targetIndex < targetCount; ++targetIndex)
        {
            ScreenTargetEntity = AimAssist_GetScreenTargetEntity(localClientNum, targetIndex);
            cent = CG_GetEntity(localClientNum, ScreenTargetEntity);
            Vec3Sub(ps->origin, cent->pose.origin, diff);
            targetDist = Vec3Length(diff);
            if (nearEnd > targetDist - 30.0f)
                nearEnd = targetDist - 30.0f;
            if (farStart < targetDist + 30.0f)
                farStart = targetDist + 30.0f;
        }
        if (farStart >= (float)nearEnd)
        {
            if (nearEnd >= 50.0f)
            {
                if (nearEnd > 512.0f)
                    nearEnd = 512.0f;
            }
            else
            {
                nearEnd = 50.0f;
            }
            if (farStart <= 2500.0f)
            {
                if (farStart < 1000.0f)
                    farStart = 1000.0f;
            }
            else
            {
                farStart = 2500.0f;
            }
        }
        else
        {
            nearEnd = 256.0f;
            farStart = 2500.0f;
        }
        Vec3Mad(cgameGlob->refdef.vieworg, 8192.0f, cgameGlob->refdef.viewaxis[0], traceEnd);
        CG_TraceCapsule(
            &trace,
            cgameGlob->refdef.vieworg,
            (float *)vec3_origin,
            (float *)vec3_origin,
            traceEnd,
            ps->clientNum,
            0x806C31);
        Vec3Lerp(cgameGlob->refdef.vieworg, traceEnd, trace.fraction, traceEnd);
        Vec3Sub(traceEnd, cgameGlob->refdef.vieworg, v);
        traceDist = Vec3Length(v);
        if (traceDist < (float)nearEnd)
            nearEnd = traceDist - 30.0f;
        if (nearEnd < 1.0f)
            nearEnd = 1.0f;
        if (traceDist > (float)farStart)
            farStart = traceDist;
        nearStart = 1.0f;
        farEnd = farStart * 4.0f;
        nearBlur = 6.0f;
        farBlur = 0.0f;
        dt = (float)cgameGlob->frametime * EQUAL_EPSILON;
        if (ps->fWeaponPosFrac == 1.0 || ps->pm_type >= PM_DEAD)
        {
            dof->nearStart = CG_UpdateAdsDofValue(dof->nearStart, nearStart, 50.0f, dt);
            dof->nearEnd = CG_UpdateAdsDofValue(dof->nearEnd, nearEnd, 50.0f, dt);
            dof->farStart = CG_UpdateAdsDofValue(dof->farStart, farStart, 400.0f, dt);
            dof->farEnd = CG_UpdateAdsDofValue(dof->farEnd, farEnd, 400.0f, dt);
            dof->nearBlur = CG_UpdateAdsDofValue(dof->nearBlur, nearBlur, 0.1f, dt);
            dof->farBlur = CG_UpdateAdsDofValue(dof->farBlur, farBlur, 0.1f, dt);
        }
        else
        {
            dof->nearStart = ps->fWeaponPosFrac * nearStart + (1.0f - ps->fWeaponPosFrac) * 0.0f;
            dof->nearEnd = ps->fWeaponPosFrac * nearEnd + (1.0f - ps->fWeaponPosFrac) * 0.0f;
            dof->farStart = ps->fWeaponPosFrac * farStart + (1.0f - ps->fWeaponPosFrac) * 5000.0f;
            dof->farEnd = ps->fWeaponPosFrac * farEnd + (1.0f - ps->fWeaponPosFrac) * 5000.0f;
            dof->nearBlur = ps->fWeaponPosFrac * nearBlur + (1.0f - ps->fWeaponPosFrac) * 6.0f;
            dof->farBlur = ps->fWeaponPosFrac * farBlur + (1.0f - ps->fWeaponPosFrac) * 0.0f;
        }
    }
}

double __cdecl CG_UpdateAdsDofValue(float currentValue, float targetValue, float maxChange, float dt)
{
    float changeVal; // [esp+0h] [ebp-4h]
    float changeVala; // [esp+0h] [ebp-4h]
    float maxChangea; // [esp+14h] [ebp+10h]

    maxChangea = maxChange / 0.05000000074505806 * dt;
    if (targetValue >= (double)currentValue)
    {
        if (targetValue > (double)currentValue)
        {
            changeVala = (targetValue - currentValue) * 0.5;
            if (maxChangea >= (double)changeVala)
            {
                if (changeVala < 1.0)
                    changeVala = 1.0;
            }
            else
            {
                changeVala = maxChangea;
            }
            if (targetValue >= currentValue + changeVala)
                return (float)(currentValue + changeVala);
            else
                return targetValue;
        }
    }
    else
    {
        changeVal = (currentValue - targetValue) * 0.5;
        if (maxChangea >= (double)changeVal)
        {
            if (changeVal < 1.0)
                changeVal = 1.0;
        }
        else
        {
            changeVal = maxChangea;
        }
        if (targetValue <= currentValue - changeVal)
            return (float)(currentValue - changeVal);
        else
            return targetValue;
    }
    return currentValue;
}

